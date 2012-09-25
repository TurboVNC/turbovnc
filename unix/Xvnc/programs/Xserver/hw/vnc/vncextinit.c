/* Copyright (C) 2011 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
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

#include <stdio.h>
#include <errno.h>

#define NEED_EVENTS
#include <X11/X.h>
#include "misc.h"
#include "extnsionst.h"
#include "selection.h"
#include "rfb.h"
#define _VNCEXT_SERVER_
#define _VNCEXT_PROTO_
#include "vncExt.h"

#undef max
#undef min

static void vncResetProc(ExtensionEntry* extEntry);
static void vncClientStateChange(CallbackListPtr*, pointer, pointer);
static void SendSelectionChangeEvent(Atom selection);
static int ProcVncExtDispatch(ClientPtr client);
static int SProcVncExtDispatch(ClientPtr client);
static void vncSelectionCallback(CallbackListPtr *callbacks, pointer data,
                                 pointer args);

static unsigned long vncExtGeneration = 0;

static char* clientCutText = 0;
static int clientCutTextLen = 0;

static struct _VncInputSelect* vncInputSelectHead = 0;
typedef struct _VncInputSelect {
  ClientPtr client;
  Window window;
  int mask;
  struct _VncInputSelect* next;
} VncInputSelect;

static int vncErrorBase = 0;
static int vncEventBase = 0;

void vncExtensionInit(void)
{
  ExtensionEntry* extEntry;

  if (vncExtGeneration == serverGeneration) {
    rfbLog("vncExtensionInit() called twice in same generation?\n");
    return;
  }
  vncExtGeneration = serverGeneration;

  extEntry
    = AddExtension(VNCEXTNAME, VncExtNumberEvents, VncExtNumberErrors,
                   ProcVncExtDispatch, SProcVncExtDispatch, vncResetProc,
                   StandardMinorOpcode);
  if (!extEntry) {
    ErrorF("vncExtensionInit(): AddExtension failed\n");
    return;
  }

  vncErrorBase = extEntry->errorBase;
  vncEventBase = extEntry->eventBase;

  rfbLog("VNC extension running!\n");

  if (!AddCallback(&ClientStateCallback, vncClientStateChange, 0)) {
    FatalError("Add ClientStateCallback failed\n");
  }

  if (!AddCallback(&SelectionCallback, vncSelectionCallback, 0)) {
    FatalError("Add SelectionCallback failed\n");
  }
}

static void vncResetProc(ExtensionEntry* extEntry)
{
}

static void vncSelectionCallback(CallbackListPtr *callbacks, pointer data, pointer args)
{
  SelectionInfoRec *info = (SelectionInfoRec *) args;
  Selection *selection = info->selection;

  SendSelectionChangeEvent(selection->selection);
}

static void vncClientStateChange(CallbackListPtr *callbacks, pointer data, pointer p)
{
  VncInputSelect *cur;
  ClientPtr client = ((NewClientInfoRec*)p)->client;
  if (client->clientState == ClientStateGone) {
    VncInputSelect** nextPtr = &vncInputSelectHead;
    for (cur = vncInputSelectHead; cur; cur = *nextPtr) {
      if (cur->client == client) {
        *nextPtr = cur->next;
        xfree (cur);
        continue;
      }
      nextPtr = &cur->next;
    }
  }
}

void vncClientCutText(const char* str, int len)
{
  VncInputSelect *cur;
  xVncExtClientCutTextNotifyEvent ev;
  if (clientCutText) xfree (clientCutText);
  clientCutText = (char *)malloc(len);
  if (!clientCutText)
    FatalError("vncClientCutText(): Memory allocation failure\n");
  memcpy(clientCutText, str, len);
  clientCutTextLen = len;
  ev.type = vncEventBase + VncExtClientCutTextNotify;
  for (cur = vncInputSelectHead; cur; cur = cur->next) {
    if (cur->mask & VncExtClientCutTextMask) {
      ev.sequenceNumber = cur->client->sequence;
      ev.window = cur->window;
      ev.time = GetTimeInMillis();
      if (cur->client->swapped) {
        int n;
        swaps(&ev.sequenceNumber, n);
        swapl(&ev.window, n);
        swapl(&ev.time, n);
      }
      WriteToClient(cur->client, sizeof(xVncExtClientCutTextNotifyEvent),
                    (char *)&ev);
    }
  }
}

static void SendSelectionChangeEvent(Atom selection)
{
  VncInputSelect *cur;
  xVncExtSelectionChangeNotifyEvent ev;
  ev.type = vncEventBase + VncExtSelectionChangeNotify;
  for (cur = vncInputSelectHead; cur; cur = cur->next) {
    if (cur->mask & VncExtSelectionChangeMask) {
      ev.sequenceNumber = cur->client->sequence;
      ev.window = cur->window;
      ev.selection = selection;
      if (cur->client->swapped) {
        int n;
        swaps(&ev.sequenceNumber, n);
        swapl(&ev.window, n);
        swapl(&ev.selection, n);
      }
      WriteToClient(cur->client, sizeof(xVncExtSelectionChangeNotifyEvent),
                    (char *)&ev);
    }
  }
}

static int ProcVncExtSetServerCutText(ClientPtr client)
{
  char *str;
  REQUEST(xVncExtSetServerCutTextReq);
  REQUEST_FIXED_SIZE(xVncExtSetServerCutTextReq, stuff->textLen);
  str = (char *)xalloc(stuff->textLen + 1);
  if (!str)
    FatalError("ProcVncExtSetServerCutText(): Memory allocation failure\n");
  strncpy(str, (char*)&stuff[1], stuff->textLen);
  str[stuff->textLen] = 0;
  rfbSendServerCutText(str, stuff->textLen);
  xfree (str);
  return (client->noClientException);
}

static int SProcVncExtSetServerCutText(ClientPtr client)
{
  register char n;
  REQUEST(xVncExtSetServerCutTextReq);
  swaps(&stuff->length, n);
  REQUEST_AT_LEAST_SIZE(xVncExtSetServerCutTextReq);
  swapl(&stuff->textLen, n);
  return ProcVncExtSetServerCutText(client);
}

static int ProcVncExtGetClientCutText(ClientPtr client)
{
  xVncExtGetClientCutTextReply rep;
  int n;

  REQUEST(xVncExtGetClientCutTextReq);
  REQUEST_SIZE_MATCH(xVncExtGetClientCutTextReq);

  rep.type = X_Reply;
  rep.length = (clientCutTextLen + 3) >> 2;
  rep.sequenceNumber = client->sequence;
  rep.textLen = clientCutTextLen;
  if (client->swapped) {
    swaps(&rep.sequenceNumber, n);
    swapl(&rep.length, n);
    swapl(&rep.textLen, n);
  }
  WriteToClient(client, sizeof(xVncExtGetClientCutTextReply), (char *)&rep);
  if (clientCutText)
    WriteToClient(client, clientCutTextLen, clientCutText);
  return (client->noClientException);
}

static int SProcVncExtGetClientCutText(ClientPtr client)
{
  register char n;
  REQUEST(xVncExtGetClientCutTextReq);
  swaps(&stuff->length, n);
  REQUEST_SIZE_MATCH(xVncExtGetClientCutTextReq);
  return ProcVncExtGetClientCutText(client);
}

static int ProcVncExtSelectInput(ClientPtr client)
{
  VncInputSelect** nextPtr = &vncInputSelectHead;
  VncInputSelect* cur;
  REQUEST(xVncExtSelectInputReq);
  REQUEST_SIZE_MATCH(xVncExtSelectInputReq);
  for (cur = vncInputSelectHead; cur; cur = *nextPtr) {
    if (cur->client == client && cur->window == stuff->window) {
      cur->mask = stuff->mask;
      if (!cur->mask) {
        *nextPtr = cur->next;
        xfree (cur);
      }
      break;
    }
    nextPtr = &cur->next;
  }
  if (!cur) {
    cur = (VncInputSelect *)xalloc(sizeof(VncInputSelect));
    if (!cur)
      FatalError("ProcVncExtSelectInput(): Memory allocation failure\n");
    cur->client = client;
    cur->window = stuff->window;
    cur->mask = stuff->mask;
    cur->next = vncInputSelectHead;
    vncInputSelectHead = cur;
  }
  return (client->noClientException);
}

static int SProcVncExtSelectInput(ClientPtr client)
{
  register char n;
  REQUEST(xVncExtSelectInputReq);
  swaps(&stuff->length, n);
  REQUEST_SIZE_MATCH(xVncExtSelectInputReq);
  swapl(&stuff->window, n);
  swapl(&stuff->mask, n);
  return ProcVncExtSelectInput(client);
}

static int ProcVncExtDispatch(ClientPtr client)
{
  REQUEST(xReq);
  switch (stuff->data) {
  case X_VncExtSetServerCutText:
    return ProcVncExtSetServerCutText(client);
  case X_VncExtGetClientCutText:
    return ProcVncExtGetClientCutText(client);
  case X_VncExtSelectInput:
    return ProcVncExtSelectInput(client);
  default:
    return BadRequest;
  }
}

static int SProcVncExtDispatch(ClientPtr client)
{
  REQUEST(xReq);
  switch (stuff->data) {
  case X_VncExtSetServerCutText:
    return SProcVncExtSetServerCutText(client);
  case X_VncExtGetClientCutText:
    return SProcVncExtGetClientCutText(client);
  case X_VncExtSelectInput:
    return SProcVncExtSelectInput(client);
  default:
    return BadRequest;
  }
}
