/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2011, 2013-2015, 2017-2018 D. R. Commander.
 *                                          All Rights Reserved.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

/*
 * VNC Extension implementation (basically a C port of the relevant portions of
 * vncExtInit.cc from the RealVNC code base
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include <errno.h>

#include <X11/X.h>
#include "misc.h"
#include "extnsionst.h"
#include "rfb.h"
#define _VNCEXT_SERVER_
#define _VNCEXT_PROTO_
#include "vncExt.h"

#undef max
#undef min

static void vncResetProc(ExtensionEntry *extEntry);
static void vncClientStateChange(CallbackListPtr *, pointer, pointer);
static int ProcVncExtDispatch(ClientPtr client);
static int SProcVncExtDispatch(ClientPtr client);

static unsigned long vncExtGeneration = 0;

static struct _VncInputSelect *vncInputSelectHead = 0;
typedef struct _VncInputSelect {
  ClientPtr client;
  Window window;
  int mask;
  struct _VncInputSelect *next;
} VncInputSelect;

static int vncErrorBase = 0;
static int vncEventBase = 0;


void vncExtensionInit(void)
{
  ExtensionEntry *extEntry;

  if (vncExtGeneration == serverGeneration) {
    rfbLog("vncExtensionInit() called twice in same generation?\n");
    return;
  }
  vncExtGeneration = serverGeneration;

  extEntry = AddExtension(VNCEXTNAME, VncExtNumberEvents, VncExtNumberErrors,
                          ProcVncExtDispatch, SProcVncExtDispatch,
                          vncResetProc, StandardMinorOpcode);
  if (!extEntry) {
    ErrorF("vncExtensionInit(): AddExtension failed\n");
    return;
  }

  vncErrorBase = extEntry->errorBase;
  vncEventBase = extEntry->eventBase;

  vncSelectionInit();

  rfbLog("VNC extension running!\n");

  if (!AddCallback(&ClientStateCallback, vncClientStateChange, 0))
    FatalError("Add ClientStateCallback failed");
}


static void vncResetProc(ExtensionEntry *extEntry)
{
}


static void vncClientStateChange(CallbackListPtr *callbacks, pointer data,
                                 pointer p)
{
  VncInputSelect *cur;
  ClientPtr client = ((NewClientInfoRec *)p)->client;

  if (client->clientState == ClientStateGone) {
    VncInputSelect **nextPtr = &vncInputSelectHead;
    for (cur = vncInputSelectHead; cur; cur = *nextPtr) {
      if (cur->client == client) {
        *nextPtr = cur->next;
        free(cur);
        continue;
      }
      nextPtr = &cur->next;
    }
  }
}


static int ProcVncExtSelectInput(ClientPtr client)
{
  VncInputSelect **nextPtr = &vncInputSelectHead;
  VncInputSelect *cur;

  REQUEST(xVncExtSelectInputReq);
  REQUEST_SIZE_MATCH(xVncExtSelectInputReq);
  for (cur = vncInputSelectHead; cur; cur = *nextPtr) {
    if (cur->client == client && cur->window == stuff->window) {
      cur->mask = stuff->mask;
      if (!cur->mask) {
        *nextPtr = cur->next;
        free(cur);
      }
      break;
    }
    nextPtr = &cur->next;
  }
  if (!cur) {
    cur = (VncInputSelect *)rfbAlloc(sizeof(VncInputSelect));
    cur->client = client;
    cur->window = stuff->window;
    cur->mask = stuff->mask;
    cur->next = vncInputSelectHead;
    vncInputSelectHead = cur;
  }
  return client->noClientException;
}


static int SProcVncExtSelectInput(ClientPtr client)
{
  REQUEST(xVncExtSelectInputReq);
  swaps(&stuff->length);
  REQUEST_SIZE_MATCH(xVncExtSelectInputReq);
  swapl(&stuff->window);
  swapl(&stuff->mask);
  return ProcVncExtSelectInput(client);
}


static int ProcVncExtConnect(ClientPtr client)
{
  char *str;

  REQUEST(xVncExtConnectReq);
  REQUEST_FIXED_SIZE(xVncExtConnectReq, stuff->strLen);
  str = (char *)rfbAlloc(stuff->strLen + 1);
  strncpy(str, (char *)&stuff[1], stuff->strLen);
  str[stuff->strLen] = 0;

  xVncExtConnectReply rep;
  rep.success = 0;
  if (stuff->strLen == 0) {
    rfbClientPtr cl, nextCl;
    for (cl = rfbClientHead; cl; cl = nextCl) {
      nextCl = cl->next;
      if (cl->reverseConnection) {
        rfbCloseClient(cl);
        rep.success = 1;
      }
    }
  } else {
    int port = 5500, id = -1, i;
    for (i = 0; i < stuff->strLen; i++) {
      if (str[i] == ':') {
        port = atoi(&str[i + 1]);
        str[i] = 0;
        break;
      }
    }
    for (i = 0; i < stuff->strLen; i++) {
      if (str[i] == '#') {
        id = atoi(&str[i + 1]);
        str[i] = 0;
        break;
      }
    }
    if (!rfbReverseConnection(str, port, id))
      rfbLog("Could not initiate reverse connection to %s:%d\n", str, port);
    else rep.success = 1;
  }

  rep.type = X_Reply;
  rep.length = 0;
  rep.sequenceNumber = client->sequence;
  if (client->swapped) {
    swaps(&rep.sequenceNumber);
    swapl(&rep.length);
  }
  WriteToClient(client, sizeof(xVncExtConnectReply), (char *)&rep);
  free(str);
  return client->noClientException;
}


static int SProcVncExtConnect(ClientPtr client)
{
  REQUEST(xVncExtConnectReq);
  swaps(&stuff->length);
  REQUEST_AT_LEAST_SIZE(xVncExtConnectReq);
  return ProcVncExtConnect(client);
}


static int ProcVncExtDispatch(ClientPtr client)
{
  REQUEST(xReq);
  switch (stuff->data) {
    case X_VncExtSelectInput:
      return ProcVncExtSelectInput(client);
    case X_VncExtConnect:
      return ProcVncExtConnect(client);
    default:
      return BadRequest;
  }
}


static int SProcVncExtDispatch(ClientPtr client)
{
  REQUEST(xReq);
  switch (stuff->data) {
    case X_VncExtSelectInput:
      return SProcVncExtSelectInput(client);
    case X_VncExtConnect:
      return SProcVncExtConnect(client);
    default:
      return BadRequest;
  }
}
