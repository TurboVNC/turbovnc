/* Copyright (C) 2011, 2013-2015, 2017-2018, 2021 D. R. Commander.
 *                                                All Rights Reserved.
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

enum VncParamType { VNC_BOOL = 0, VNC_BOOL_AUTO, VNC_DOUBLE, VNC_INT,
                    VNC_SUBSAMP };

typedef struct _VncParam {
  const char *name;
  void *ptr;
  enum VncParamType type;
  double min, max;
  const char *desc;
} VncParam;

#define NUM_PARAMS 9

VncParam params[NUM_PARAMS] =
{
  { "AlwaysShared", (void *)&rfbAlwaysShared, VNC_BOOL, 0.0, 1.0,
    "Always treat new connections as shared" },
  { "Disconnect", (void *)&rfbDisconnect, VNC_BOOL, 0.0, 1.0,
    "Disconnect existing viewers when a new non-shared connection comes in, "
    "rather than refusing the new connection" },
  { "NeverShared", (void *)&rfbNeverShared, VNC_BOOL, 0.0, 1.0,
    "Never treat new connections as shared" },
  { "ALR", (void *)&rfbAutoLosslessRefresh, VNC_DOUBLE, 0.0, 3600.0,
    "Automatic lossless refresh timeout (0.0 = ALR disabled)" },
  { "ALRAll", (void *)&rfbALRAll, VNC_BOOL, 0.0, 1.0,
    "True = Make all screen regions eligible for ALR; False = Make only "
    "regions affected by PutImage operations eligible for ALR" },
  { "ALRQual", (void *)&rfbALRQualityLevel, VNC_INT, -1.0, 100.0,
    "Automatic lossless refresh JPEG quality (-1 = send ALR using "
    "mathematically lossless image rather than JPEG image)" },
  { "ALRSamp", (void *)&rfbALRSubsampLevel, VNC_SUBSAMP, 0.0, 3.0,
    "Automatic lossless refresh JPEG chroma subsampling (1x, 2x, 4x, or "
    "gray)" },
  { "Interframe", (void *)&rfbInterframe, VNC_BOOL_AUTO, -1.0, 1.0,
    "Use interframe comparison (Auto = determined by compression level)" },
  { "Profile", (void *)&rfbProfile, VNC_BOOL, 0.0, 1.0,
    "Enable profiling" }

};


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


static int ProcVncExtSetParam(ClientPtr client)
{
  xVncExtSetParamReply rep;
  int i;
  char *name, *str, *value;

  REQUEST(xVncExtSetParamReq);
  REQUEST_FIXED_SIZE(xVncExtSetParamReq, stuff->paramLen);
  str = (char *)rfbAlloc(stuff->paramLen + 1);
  strncpy(str, (char *)&stuff[1], stuff->paramLen);
  str[stuff->paramLen] = 0;

  rep.type = X_Reply;
  rep.length = 0;
  rep.sequenceNumber = client->sequence;
  rep.success = 0;

  name = str;
  value = strstr(str, "=");
  if (value) *value++ = '\0';

  if (value) {
    for (i = 0; i < NUM_PARAMS; i++) {
      if (!strcasecmp(name, params[i].name)) {
        switch (params[i].type) {
          case VNC_BOOL:
            if (!strcmp(value, "1") || !strncasecmp(value, "on", 2) ||
                toupper(value[0]) == 'T' || toupper(value[0]) == 'Y') {
              *(Bool *)params[i].ptr = TRUE;
              rep.success = 1;
            } else if (!strcmp(value, "0") || !strncasecmp(value, "of", 2) ||
                       toupper(value[0]) == 'F' || toupper(value[0]) == 'N') {
              *(Bool *)params[i].ptr = FALSE;
              rep.success = 1;
            }
            break;
          case VNC_BOOL_AUTO:
            if (!strcmp(value, "-1") || toupper(value[0]) == 'A') {
              *(int *)params[i].ptr = -1;
              rep.success = 1;
            } else if (!strcmp(value, "1") || !strncasecmp(value, "on", 2) ||
                       toupper(value[0]) == 'T' || toupper(value[0]) == 'Y') {
              *(int *)params[i].ptr = 1;
              rep.success = 1;
            } else if (!strcmp(value, "0") || !strncasecmp(value, "of", 2) ||
                       toupper(value[0]) == 'F' || toupper(value[0]) == 'N') {
              *(int *)params[i].ptr = 0;
              rep.success = 1;
            }
            break;
          case VNC_DOUBLE:
          {
            double tmp;
            if (sscanf(value, "%lf", &tmp) == 1 && tmp >= params[i].min &&
                tmp <= params[i].max) {
              *(double *)params[i].ptr = tmp;
              rep.success = 1;
            }
            break;
          }
          case VNC_INT:
          {
            int tmp;
            if (sscanf(value, "%d", &tmp) == 1 && tmp >= (int)params[i].min &&
                tmp <= (int)params[i].max) {
              *(int *)params[i].ptr = tmp;
              rep.success = 1;
            }
            break;
          }
          case VNC_SUBSAMP:
          {
            int s;
            for (s = 0; s < TVNC_SAMPOPT; s++) {
              if (toupper(value[0]) == toupper(subsampStr[s][0]) ||
                  (s == TVNC_GRAY && value[0] == '0')) {
                *(int *)params[i].ptr = s;
                rep.success = 1;
                break;
              }
            }
            break;
          }
        }
      }
    }
  }

  if (client->swapped) {
    swaps(&rep.sequenceNumber);
    swapl(&rep.length);
  }
  WriteToClient(client, sizeof(xVncExtSetParamReply), (char *)&rep);
  free(str);
  return (client->noClientException);
}


static int SProcVncExtSetParam(ClientPtr client)
{
  REQUEST(xVncExtSetParamReq);
  swaps(&stuff->length);
  REQUEST_AT_LEAST_SIZE(xVncExtSetParamReq);
  return ProcVncExtSetParam(client);
}


static int ProcVncExtGetParam(ClientPtr client)
{
  xVncExtGetParamReply rep;
  int i, len = 0;
  char *str, temps[80], *value = NULL;

  REQUEST(xVncExtGetParamReq);
  REQUEST_FIXED_SIZE(xVncExtGetParamReq, stuff->paramLen);
  str = (char *)rfbAlloc(stuff->paramLen + 1);
  strncpy(str, (char *)&stuff[1], stuff->paramLen);
  str[stuff->paramLen] = 0;

  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;
  rep.success = 0;

  #define BOOL_STR(var) ((var) ? "True" : "False")
  #define BOOL_AUTO_STR(var) ((var) < 0 ? "Auto" : BOOL_STR(var))

  for (i = 0; i < NUM_PARAMS; i++) {
    if (!strcasecmp(str, params[i].name)) {
      switch (params[i].type) {
        case VNC_BOOL:
          snprintf(temps, 80, "%s", BOOL_STR(*(Bool *)params[i].ptr));
          break;
        case VNC_BOOL_AUTO:
          snprintf(temps, 80, "%s", BOOL_AUTO_STR(*(int *)params[i].ptr));
          break;
        case VNC_DOUBLE:
          snprintf(temps, 80, "%f", *(double *)params[i].ptr);
          break;
        case VNC_INT:
          snprintf(temps, 80, "%d", *(int *)params[i].ptr);
          break;
        case VNC_SUBSAMP:
          snprintf(temps, 80, "%s", subsampStr[*(int *)params[i].ptr]);
          break;
      }
      value = temps;
      rep.success = 1;
      len = strlen(value);
    }
  }

  rep.length = (len + 3) >> 2;
  rep.valueLen = len;
  if (client->swapped) {
    swaps(&rep.sequenceNumber);
    swapl(&rep.length);
    swaps(&rep.valueLen);
  }
  WriteToClient(client, sizeof(xVncExtGetParamReply), (char *)&rep);
  if (value)
    WriteToClient(client, len, value);
  free(str);
  return client->noClientException;
}


static int SProcVncExtGetParam(ClientPtr client)
{
  REQUEST(xVncExtGetParamReq);
  swaps(&stuff->length);
  REQUEST_AT_LEAST_SIZE(xVncExtGetParamReq);
  return ProcVncExtGetParam(client);
}


static int ProcVncExtGetParamDesc(ClientPtr client)
{
  xVncExtGetParamDescReply rep;
  int i, len = 0;
  const char *desc = NULL;
  char *str;

  REQUEST(xVncExtGetParamDescReq);
  REQUEST_FIXED_SIZE(xVncExtGetParamDescReq, stuff->paramLen);
  str = (char *)rfbAlloc(stuff->paramLen + 1);
  strncpy(str, (char *)&stuff[1], stuff->paramLen);
  str[stuff->paramLen] = 0;

  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;
  rep.success = 0;

  for (i = 0; i < NUM_PARAMS; i++) {
    if (!strcasecmp(str, params[i].name)) {
      desc = params[i].desc;
      rep.success = 1;
      len = strlen(desc);
    }
  }

  rep.length = (len + 3) >> 2;
  rep.descLen = len;
  if (client->swapped) {
    swaps(&rep.sequenceNumber);
    swapl(&rep.length);
    swaps(&rep.descLen);
  }
  WriteToClient(client, sizeof(xVncExtGetParamDescReply), (char *)&rep);
  if (desc)
    WriteToClient(client, len, (char *)desc);
  free(str);
  return client->noClientException;
}


static int SProcVncExtGetParamDesc(ClientPtr client)
{
  REQUEST(xVncExtGetParamDescReq);
  swaps(&stuff->length);
  REQUEST_AT_LEAST_SIZE(xVncExtGetParamDescReq);
  return ProcVncExtGetParamDesc(client);
}


static int ProcVncExtListParams(ClientPtr client)
{
  xVncExtListParamsReply rep;
  int i, len = 0;
  char *ptr, *str;

  REQUEST_SIZE_MATCH(xVncExtListParamsReq);

  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;

  for (i = 0; i < NUM_PARAMS; i++)
    len += strlen(params[i].name) + 1;
  str = (char *)rfbAlloc(len);
  ptr = str;
  for (i = 0; i < NUM_PARAMS; i++) {
    *ptr++ = strlen(params[i].name);
    memcpy(ptr, params[i].name, strlen(params[i].name));
    ptr += strlen(params[i].name);
  }

  rep.length = (len + 3) >> 2;
  rep.nParams = NUM_PARAMS;
  if (client->swapped) {
    swaps(&rep.sequenceNumber);
    swapl(&rep.length);
    swaps(&rep.nParams);
  }
  WriteToClient(client, sizeof(xVncExtListParamsReply), (char *)&rep);
  WriteToClient(client, len, str);
  free(str);
  return client->noClientException;
}


static int SProcVncExtListParams(ClientPtr client)
{
  REQUEST(xVncExtListParamsReq);
  swaps(&stuff->length);
  REQUEST_SIZE_MATCH(xVncExtListParamsReq);
  return ProcVncExtListParams(client);
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
  xVncExtConnectReply rep;

  REQUEST(xVncExtConnectReq);
  REQUEST_FIXED_SIZE(xVncExtConnectReq, stuff->strLen);
  str = (char *)rfbAlloc(stuff->strLen + 1);
  strncpy(str, (char *)&stuff[1], stuff->strLen);
  str[stuff->strLen] = 0;

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
    case X_VncExtSetParam:
      return ProcVncExtSetParam(client);
    case X_VncExtGetParam:
      return ProcVncExtGetParam(client);
    case X_VncExtGetParamDesc:
      return ProcVncExtGetParamDesc(client);
    case X_VncExtListParams:
      return ProcVncExtListParams(client);
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
    case X_VncExtSetParam:
      return SProcVncExtSetParam(client);
    case X_VncExtGetParam:
      return SProcVncExtGetParam(client);
    case X_VncExtGetParamDesc:
      return SProcVncExtGetParamDesc(client);
    case X_VncExtListParams:
      return SProcVncExtListParams(client);
    case X_VncExtSelectInput:
      return SProcVncExtSelectInput(client);
    case X_VncExtConnect:
      return SProcVncExtConnect(client);
    default:
      return BadRequest;
  }
}
