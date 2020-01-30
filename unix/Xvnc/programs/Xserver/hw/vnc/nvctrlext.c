/* Copyright (C) 2015, 2020 D. R. Commander.  All Rights Reserved.
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
 * NV-CONTROL extension implementation (receives requests from libXNVCtrl and
 * marshals them to a 3D-enabled X server)
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include <errno.h>

#include <X11/X.h>
#include <X11/Xfuncproto.h>
#include "misc.h"
#include "extnsionst.h"
#include "dixstruct.h"
#include <X11/Xlib.h>
#include "NVCtrlLib.h"
#include "nv_control.h"

extern void rfbLog(char *format, ...);
extern int rfbPort;
static int ProcNVCTRLDispatch(ClientPtr client);
static int SProcNVCTRLDispatch(ClientPtr client);

void nvCtrlExtensionInit(void);

static unsigned long nvCtrlGeneration = 0;

char *nvCtrlDisplay = NULL;
Bool noNVCTRLExtension = True;
int error_code;


#define SWAP(index_a, index_b)  \
  temp = ((char *)ptr)[index_a];  \
  ((char *)ptr)[index_a] = ((char *)ptr)[index_b];  \
  ((char *)ptr)[index_b] = temp;  \

static inline void swap_float(float *ptr)
{
  char temp;
  SWAP(0, 3);
  SWAP(1, 2);
}


#define X_INIT()  \
  XErrorHandler oldHandler = NULL;  \
  Display *dpy = NULL;  \
  char *tmp = NULL;  \
  if (!nvCtrlDisplay)  \
    FatalError("NV-CONTROL ERROR:\nNo 3D X server specified");  \
  if ((tmp = strchr(nvCtrlDisplay, ':')) != NULL) {  \
    if (strlen(tmp) > 1) {  \
      int displayNumber = atoi(tmp + 1);  \
      if (displayNumber + 5900 == rfbPort) {  \
        rfbLog("NV-CONTROL ERROR:\nCannot redirect NV-CONTROL requests to the same X display\n");  \
        return BadRequest;  \
      }  \
    }  \
  }  \
  if ((dpy = XOpenDisplay(nvCtrlDisplay)) == NULL) {  \
    rfbLog("NV-CONTROL X11 ERROR: Could not open X display %s\n",  \
           nvCtrlDisplay);  \
    return BadRequest;  \
  }  \
  error_code = Success;  \
  oldHandler = XSetErrorHandler(xhandler);

#define X_DEINIT()  \
  if (oldHandler && oldHandler != xhandler)  \
    XSetErrorHandler(oldHandler);  \
  XCloseDisplay(dpy);  \
  if (error_code != Success)  \
    return error_code;


static int xhandler(Display *dpy, XErrorEvent *xe)
{
  char errstr[256];

  errstr[0] = 0;
  XGetErrorText(dpy, xe->error_code, errstr, 255);
  rfbLog("NV-CONTROL X11 ERROR: %s (XID: 0x%.8x)\n", errstr, xe->resourceid);
  error_code = xe->error_code;
  return 0;
}


static void nvCtrlResetProc(ExtensionEntry *extEntry)
{
}


void nvCtrlExtensionInit(void)
{
  ExtensionEntry *ext;

  if (nvCtrlGeneration == serverGeneration) {
    rfbLog("nvCtrlExtensionInit() called twice in same generation?\n");
    return;
  }
  nvCtrlGeneration = serverGeneration;

  ext = AddExtension(NV_CONTROL_NAME, NV_CONTROL_EVENTS, NV_CONTROL_ERRORS,
                     ProcNVCTRLDispatch, SProcNVCTRLDispatch, nvCtrlResetProc,
                     StandardMinorOpcode);
  if (!ext) {
    ErrorF("nvCtrlExtensionInit(): AddExtension failed\n");
    return;
  }

  rfbLog("NV-CONTROL extension running!\n");
}


static int ProcNVCTRLQueryExtension(ClientPtr client)
{
  int major = 0, minor = 0;
  xnvCtrlQueryExtensionReply rep;

  X_INIT();

  REQUEST_SIZE_MATCH(xnvCtrlQueryExtensionReq);

  if (!XNVCTRLQueryVersion(dpy, &major, &minor))
    error_code = BadAccess;  /* BadAccess causes _XReply() on the client end to
                                return 0 without calling the X error handler */
  X_DEINIT();

  rep.type = X_Reply;
  rep.length = 0;
  rep.sequenceNumber = client->sequence;
  rep.major = major;
  rep.minor = minor;
  if (client->swapped) {
    swaps(&rep.sequenceNumber);
    swaps(&rep.major);
    swaps(&rep.minor);
  }
  WriteToClient(client, sizeof(xnvCtrlQueryExtensionReply), (char *)&rep);
  return client->noClientException;
}

static int SProcNVCTRLQueryExtension(ClientPtr client)
{
  REQUEST(xnvCtrlQueryExtensionReq);
  swaps(&stuff->length);
  REQUEST_SIZE_MATCH(xnvCtrlQueryExtensionReq);
  return ProcNVCTRLQueryExtension(client);
}


static int ProcNVCTRLIsNv(ClientPtr client)
{
  xnvCtrlIsNvReply rep;

  X_INIT();

  REQUEST_SIZE_MATCH(xnvCtrlIsNvReq);

  rep.isnv = XNVCTRLIsNvScreen(dpy, DefaultScreen(dpy));
  X_DEINIT();

  rep.type = X_Reply;
  rep.length = 0;
  rep.sequenceNumber = client->sequence;
  if (client->swapped) {
    swaps(&rep.sequenceNumber);
    swapl(&rep.isnv);
  }
  WriteToClient(client, sizeof(xnvCtrlIsNvReply), (char *)&rep);
  return client->noClientException;
}

static int SProcNVCTRLIsNv(ClientPtr client)
{
  REQUEST(xnvCtrlIsNvReq);
  swaps(&stuff->length);
  REQUEST_SIZE_MATCH(xnvCtrlIsNvReq);
  swapl(&stuff->screen);
  return ProcNVCTRLIsNv(client);
}


static int ProcNVCTRLQueryTargetCount(ClientPtr client)
{
  xnvCtrlQueryTargetCountReply rep;
  int count = 0;
  REQUEST(xnvCtrlQueryTargetCountReq);

  X_INIT();

  REQUEST_SIZE_MATCH(xnvCtrlQueryTargetCountReq);

  if (!XNVCTRLQueryTargetCount(dpy, stuff->target_type, &count))
    error_code = BadAccess;  /* BadAccess causes _XReply() on the client end to
                                return 0 without calling the X error handler */
  X_DEINIT();

  rep.type = X_Reply;
  rep.length = 0;
  rep.sequenceNumber = client->sequence;
  rep.count = (CARD32)count;
  if (client->swapped) {
    swaps(&rep.sequenceNumber);
    swapl(&rep.count);
  }
  WriteToClient(client, sizeof(xnvCtrlQueryTargetCountReply), (char *)&rep);
  return client->noClientException;
}

static int SProcNVCTRLQueryTargetCount(ClientPtr client)
{
  REQUEST(xnvCtrlQueryTargetCountReq);
  swaps(&stuff->length);
  REQUEST_SIZE_MATCH(xnvCtrlQueryTargetCountReq);
  swapl(&stuff->target_type);
  return ProcNVCTRLQueryTargetCount(client);
}


static int ProcNVCTRLSetAttribute(ClientPtr client)
{
  REQUEST(xnvCtrlSetAttributeReq);

  X_INIT();

  REQUEST_SIZE_MATCH(xnvCtrlSetAttributeReq);

  XNVCTRLSetTargetAttribute(dpy, stuff->target_type, stuff->target_id,
                            stuff->display_mask, stuff->attribute,
                            stuff->value);
  X_DEINIT();

  return Success;
}

static int SProcNVCTRLSetAttribute(ClientPtr client)
{
  REQUEST(xnvCtrlSetAttributeReq);
  swaps(&stuff->length);
  REQUEST_SIZE_MATCH(xnvCtrlSetAttributeReq);
  swaps(&stuff->target_id);
  swaps(&stuff->target_type);
  swapl(&stuff->display_mask);
  swapl(&stuff->attribute);
  swapl(&stuff->value);
  return ProcNVCTRLSetAttribute(client);
}


static int ProcNVCTRLSetAttributeAndGetStatus(ClientPtr client)
{
  xnvCtrlSetAttributeAndGetStatusReply rep;
  REQUEST(xnvCtrlSetAttributeAndGetStatusReq);

  X_INIT();

  REQUEST_SIZE_MATCH(xnvCtrlSetAttributeAndGetStatusReq);

  rep.flags =
    XNVCTRLSetTargetAttributeAndGetStatus(dpy, stuff->target_type,
                                          stuff->target_id,
                                          stuff->display_mask,
                                          stuff->attribute, stuff->value);
  X_DEINIT();

  rep.type = X_Reply;
  rep.length = 0;
  rep.sequenceNumber = client->sequence;
  if (client->swapped) {
    swaps(&rep.sequenceNumber);
    swapl(&rep.flags);
  }
  WriteToClient(client, sizeof(xnvCtrlSetAttributeAndGetStatusReply),
                (char *)&rep);
  return client->noClientException;
}

static int SProcNVCTRLSetAttributeAndGetStatus(ClientPtr client)
{
  REQUEST(xnvCtrlSetAttributeAndGetStatusReq);
  swaps(&stuff->length);
  REQUEST_SIZE_MATCH(xnvCtrlSetAttributeAndGetStatusReq);
  swaps(&stuff->target_id);
  swaps(&stuff->target_type);
  swapl(&stuff->display_mask);
  swapl(&stuff->attribute);
  swapl(&stuff->value);
  return ProcNVCTRLSetAttributeAndGetStatus(client);
}


static int ProcNVCTRLQueryAttribute(ClientPtr client)
{
  xnvCtrlQueryAttributeReply rep;
  int value = 0;
  REQUEST(xnvCtrlQueryAttributeReq);

  X_INIT();

  REQUEST_SIZE_MATCH(xnvCtrlQueryAttributeReq);

  rep.flags =
    XNVCTRLQueryTargetAttribute(dpy, stuff->target_type, stuff->target_id,
                                stuff->display_mask, stuff->attribute,
                                &value);
  X_DEINIT();

  rep.type = X_Reply;
  rep.length = 0;
  rep.value = value;
  rep.sequenceNumber = client->sequence;
  if (client->swapped) {
    swaps(&rep.sequenceNumber);
    swapl(&rep.flags);
    swapl(&rep.value);
  }
  WriteToClient(client, sizeof(xnvCtrlQueryAttributeReply), (char *)&rep);
  return client->noClientException;
}

static int SProcNVCTRLQueryAttribute(ClientPtr client)
{
  REQUEST(xnvCtrlQueryAttributeReq);
  swaps(&stuff->length);
  REQUEST_SIZE_MATCH(xnvCtrlQueryAttributeReq);
  swaps(&stuff->target_id);
  swaps(&stuff->target_type);
  swapl(&stuff->display_mask);
  swapl(&stuff->attribute);
  return ProcNVCTRLQueryAttribute(client);
}


static int ProcNVCTRLQueryAttribute64(ClientPtr client)
{
  xnvCtrlQueryAttribute64Reply rep;
  REQUEST(xnvCtrlQueryAttributeReq);

  X_INIT();

  REQUEST_SIZE_MATCH(xnvCtrlQueryAttributeReq);

  rep.flags =
    XNVCTRLQueryTargetAttribute64(dpy, stuff->target_type, stuff->target_id,
                                  stuff->display_mask, stuff->attribute,
                                  &rep.value_64);
  X_DEINIT();

  rep.type = X_Reply;
  rep.length = 0;
  rep.sequenceNumber = client->sequence;
  if (client->swapped) {
    swaps(&rep.sequenceNumber);
    swapl(&rep.flags);
    rep.value_64 = bswap_64(rep.value_64);
  }
  WriteToClient(client, sizeof(xnvCtrlQueryAttribute64Reply), (char *)&rep);
  return client->noClientException;
}

static int SProcNVCTRLQueryAttribute64(ClientPtr client)
{
  REQUEST(xnvCtrlQueryAttributeReq);
  swaps(&stuff->length);
  REQUEST_SIZE_MATCH(xnvCtrlQueryAttributeReq);
  swaps(&stuff->target_id);
  swaps(&stuff->target_type);
  swapl(&stuff->display_mask);
  swapl(&stuff->attribute);
  return ProcNVCTRLQueryAttribute64(client);
}


static int ProcNVCTRLQueryStringAttribute(ClientPtr client)
{
  xnvCtrlQueryStringAttributeReply rep;
  char *ptr = NULL;
  REQUEST(xnvCtrlQueryStringAttributeReq);

  X_INIT();

  REQUEST_SIZE_MATCH(xnvCtrlQueryStringAttributeReq);

  rep.flags =
    XNVCTRLQueryTargetStringAttribute(dpy, stuff->target_type,
                                      stuff->target_id, stuff->display_mask,
                                      stuff->attribute, &ptr);
  if (error_code != Success)
    free(ptr);
  X_DEINIT();

  rep.type = X_Reply;
  rep.n = ptr ? strlen(ptr) + 1 : 0;
  rep.length = (rep.n + 3) >> 2;
  rep.sequenceNumber = client->sequence;
  if (client->swapped) {
    swaps(&rep.sequenceNumber);
    swapl(&rep.length);
    swapl(&rep.flags);
    swapl(&rep.n);
  }
  WriteToClient(client, sizeof(xnvCtrlQueryStringAttributeReply),
                (char *)&rep);
  if (ptr) {
    if (client->noClientException == Success)
      WriteToClient(client, rep.n, ptr);
    free(ptr);
  }
  return client->noClientException;
}

static int SProcNVCTRLQueryStringAttribute(ClientPtr client)
{
  REQUEST(xnvCtrlQueryStringAttributeReq);
  swaps(&stuff->length);
  REQUEST_SIZE_MATCH(xnvCtrlQueryStringAttributeReq);
  swaps(&stuff->target_id);
  swaps(&stuff->target_type);
  swapl(&stuff->display_mask);
  swapl(&stuff->attribute);
  return ProcNVCTRLQueryStringAttribute(client);
}


static int ProcNVCTRLSetStringAttribute(ClientPtr client)
{
  xnvCtrlSetStringAttributeReply rep;
  REQUEST(xnvCtrlSetStringAttributeReq);

  X_INIT();

  REQUEST_FIXED_SIZE(xnvCtrlSetStringAttributeReq, stuff->num_bytes);

  rep.flags =
    XNVCTRLSetTargetStringAttribute(dpy, stuff->target_type, stuff->target_id,
                                    stuff->display_mask, stuff->attribute,
                                    (char *)&stuff[1]);
  X_DEINIT();

  rep.type = X_Reply;
  rep.length = 0;
  rep.sequenceNumber = client->sequence;
  if (client->swapped) {
    swaps(&rep.sequenceNumber);
    swapl(&rep.length);
    swapl(&rep.flags);
  }
  WriteToClient(client, sizeof(xnvCtrlSetStringAttributeReply), (char *)&rep);
  return client->noClientException;
}

static int SProcNVCTRLSetStringAttribute(ClientPtr client)
{
  REQUEST(xnvCtrlSetStringAttributeReq);
  swaps(&stuff->length);
  REQUEST_SIZE_MATCH(xnvCtrlSetStringAttributeReq);
  swaps(&stuff->target_id);
  swaps(&stuff->target_type);
  swapl(&stuff->display_mask);
  swapl(&stuff->attribute);
  swapl(&stuff->num_bytes);
  return ProcNVCTRLSetStringAttribute(client);
}


static int ProcNVCTRLQueryValidAttributeValues(ClientPtr client)
{
  xnvCtrlQueryValidAttributeValuesReply rep;
  NVCTRLAttributeValidValuesRec values;
  REQUEST(xnvCtrlQueryValidAttributeValuesReq);

  X_INIT();

  REQUEST_SIZE_MATCH(xnvCtrlQueryValidAttributeValuesReq);

  if (stuff->nvReqType == X_nvCtrlQueryValidStringAttributeValues)
    rep.flags =
      XNVCTRLQueryValidTargetStringAttributeValues(dpy, stuff->target_type,
                                                   stuff->target_id,
                                                   stuff->display_mask,
                                                   stuff->attribute, &values);
  else
    rep.flags =
      XNVCTRLQueryValidTargetAttributeValues(dpy, stuff->target_type,
                                             stuff->target_id,
                                             stuff->display_mask,
                                             stuff->attribute, &values);
  X_DEINIT();

  rep.type = X_Reply;
  rep.length = 0;
  rep.sequenceNumber = client->sequence;
  rep.attr_type = values.type;
  rep.min = values.u.range.min;
  rep.max = values.u.range.max;
  rep.bits = values.u.bits.ints;
  rep.perms = values.permissions;
  if (client->swapped) {
    swaps(&rep.sequenceNumber);
    swapl(&rep.flags);
    swapl(&rep.attr_type);
    swapl(&rep.min);
    swapl(&rep.max);
    swapl(&rep.bits);
    swapl(&rep.perms);
  }
  WriteToClient(client, sizeof(xnvCtrlQueryValidAttributeValuesReply),
                (char *)&rep);
  return client->noClientException;
}

static int SProcNVCTRLQueryValidAttributeValues(ClientPtr client)
{
  REQUEST(xnvCtrlQueryValidAttributeValuesReq);
  swaps(&stuff->length);
  REQUEST_SIZE_MATCH(xnvCtrlQueryValidAttributeValuesReq);
  swaps(&stuff->target_id);
  swaps(&stuff->target_type);
  swapl(&stuff->display_mask);
  swapl(&stuff->attribute);
  return ProcNVCTRLQueryValidAttributeValues(client);
}


static int ProcNVCTRLQueryValidAttributeValues64(ClientPtr client)
{
  xnvCtrlQueryValidAttributeValues64Reply rep;
  NVCTRLAttributeValidValuesRec values;
  REQUEST(xnvCtrlQueryValidAttributeValuesReq);

  X_INIT();

  REQUEST_SIZE_MATCH(xnvCtrlQueryValidAttributeValuesReq);

  rep.flags =
    XNVCTRLQueryValidTargetAttributeValues(dpy, stuff->target_type,
                                           stuff->target_id,
                                           stuff->display_mask,
                                           stuff->attribute, &values);
  X_DEINIT();

  rep.type = X_Reply;
  rep.length = sz_xnvCtrlQueryValidAttributeValues64Reply_extra;
  rep.sequenceNumber = client->sequence;
  rep.attr_type = values.type;
  rep.min_64 = values.u.range.min;
  rep.max_64 = values.u.range.max;
  rep.bits_64 = values.u.bits.ints;
  rep.perms = values.permissions;
  if (client->swapped) {
    swaps(&rep.sequenceNumber);
    swapl(&rep.length);
    swapl(&rep.flags);
    swapl(&rep.attr_type);
    rep.min_64 = bswap_64(rep.min_64);
    rep.max_64 = bswap_64(rep.max_64);
    rep.bits_64 = bswap_64(rep.bits_64);
    swapl(&rep.perms);
  }
  WriteToClient(client, sizeof(xnvCtrlQueryValidAttributeValues64Reply),
                (char *)&rep);
  return client->noClientException;
}

static int SProcNVCTRLQueryValidAttributeValues64(ClientPtr client)
{
  REQUEST(xnvCtrlQueryValidAttributeValuesReq);
  swaps(&stuff->length);
  REQUEST_SIZE_MATCH(xnvCtrlQueryValidAttributeValuesReq);
  swaps(&stuff->target_id);
  swaps(&stuff->target_type);
  swapl(&stuff->display_mask);
  swapl(&stuff->attribute);
  return ProcNVCTRLQueryValidAttributeValues64(client);
}


static int ProcNVCTRLQueryAttributePermissions(ClientPtr client, int reqType)
{
  xnvCtrlQueryAttributePermissionsReply rep;
  NVCTRLAttributePermissionsRec permissions;
  REQUEST(xnvCtrlQueryAttributePermissionsReq);

  X_INIT();

  REQUEST_SIZE_MATCH(xnvCtrlQueryAttributePermissionsReq);

  memset(&permissions, 0, sizeof(NVCTRLAttributePermissionsRec));

  switch (reqType) {
    case X_nvCtrlQueryAttributePermissions:
      rep.flags =
        XNVCTRLQueryAttributePermissions(dpy, stuff->attribute, &permissions);
      break;
    case X_nvCtrlQueryStringAttributePermissions:
      rep.flags =
        XNVCTRLQueryStringAttributePermissions(dpy, stuff->attribute,
                                               &permissions);
      break;
    case X_nvCtrlQueryBinaryDataAttributePermissions:
      rep.flags =
        XNVCTRLQueryBinaryDataAttributePermissions(dpy, stuff->attribute,
                                                   &permissions);
      break;
    case X_nvCtrlQueryStringOperationAttributePermissions:
      rep.flags =
        XNVCTRLQueryStringOperationAttributePermissions(dpy, stuff->attribute,
                                                        &permissions);
      break;
    default:
      return BadRequest;
  }

  X_DEINIT();

  rep.type = X_Reply;
  rep.length = 0;
  rep.sequenceNumber = client->sequence;
  rep.attr_type = permissions.type;
  rep.perms = permissions.permissions;
  if (client->swapped) {
    swaps(&rep.sequenceNumber);
    swapl(&rep.flags);
    swapl(&rep.attr_type);
    swapl(&rep.perms);
  }
  WriteToClient(client, sz_xnvCtrlQueryAttributePermissionsReply,
                (char *)&rep);
  return client->noClientException;
}

static int SProcNVCTRLQueryAttributePermissions(ClientPtr client)
{
  REQUEST(xnvCtrlQueryAttributePermissionsReq);
  swaps(&stuff->length);
  REQUEST_SIZE_MATCH(xnvCtrlQueryAttributePermissionsReq);
  swapl(&stuff->attribute);
  return ProcNVCTRLQueryAttributePermissions(client, stuff->nvReqType);
}


static int ProcNVCTRLSetGvoColorConversion(ClientPtr client)
{
  float colorMatrix[3][3], colorOffset[3], colorScale[3];
  REQUEST(xnvCtrlSetGvoColorConversionReq);

  X_INIT();

  REQUEST_SIZE_MATCH(xnvCtrlSetGvoColorConversionReq);

  colorMatrix[0][0] = stuff->cscMatrix_y_r;
  colorMatrix[0][1] = stuff->cscMatrix_y_g;
  colorMatrix[0][2] = stuff->cscMatrix_y_b;

  colorMatrix[1][0] = stuff->cscMatrix_cr_r;
  colorMatrix[1][1] = stuff->cscMatrix_cr_g;
  colorMatrix[1][2] = stuff->cscMatrix_cr_b;

  colorMatrix[2][0] = stuff->cscMatrix_cb_r;
  colorMatrix[2][1] = stuff->cscMatrix_cb_g;
  colorMatrix[2][2] = stuff->cscMatrix_cb_b;

  colorOffset[0] = stuff->cscOffset_y;
  colorOffset[1] = stuff->cscOffset_cr;
  colorOffset[2] = stuff->cscOffset_cb;

  colorScale[0] = stuff->cscScale_y;
  colorScale[1] = stuff->cscScale_cr;
  colorScale[2] = stuff->cscScale_cb;

  XNVCTRLSetGvoColorConversion(dpy, DefaultScreen(dpy), colorMatrix,
                               colorOffset, colorScale);

  X_DEINIT();

  return Success;
}

static int SProcNVCTRLSetGvoColorConversion(ClientPtr client)
{
  REQUEST(xnvCtrlSetGvoColorConversionReq);
  swaps(&stuff->length);
  REQUEST_SIZE_MATCH(xnvCtrlSetGvoColorConversionReq);
  swap_float(&stuff->cscMatrix_y_r);
  swap_float(&stuff->cscMatrix_y_g);
  swap_float(&stuff->cscMatrix_y_b);
  swap_float(&stuff->cscMatrix_cr_r);
  swap_float(&stuff->cscMatrix_cr_g);
  swap_float(&stuff->cscMatrix_cr_b);
  swap_float(&stuff->cscMatrix_cb_r);
  swap_float(&stuff->cscMatrix_cb_g);
  swap_float(&stuff->cscMatrix_cb_b);
  swap_float(&stuff->cscOffset_y);
  swap_float(&stuff->cscOffset_cr);
  swap_float(&stuff->cscOffset_cb);
  swap_float(&stuff->cscScale_y);
  swap_float(&stuff->cscScale_cr);
  swap_float(&stuff->cscScale_cb);
  return ProcNVCTRLSetGvoColorConversion(client);
}


static int ProcNVCTRLQueryGvoColorConversion(ClientPtr client)
{
  xnvCtrlQueryGvoColorConversionReply rep;
  float colorMatrix[3][3], colorOffset[3], colorScale[3];

  X_INIT();

  REQUEST_SIZE_MATCH(xnvCtrlQueryGvoColorConversionReq);

  if (!XNVCTRLQueryGvoColorConversion(dpy, DefaultScreen(dpy), colorMatrix,
                                      colorOffset, colorScale))
    error_code = BadAccess;  /* BadAccess causes _XReply() on the client end to
                                return 0 without calling the X error handler */
  X_DEINIT();

  rep.type = X_Reply;
  rep.length = 0;
  rep.sequenceNumber = client->sequence;
  if (client->swapped)
    swaps(&rep.sequenceNumber);
  WriteToClient(client, sizeof(xnvCtrlQueryGvoColorConversionReply),
                (char *)&rep);
  if (client->noClientException != Success)
    return client->noClientException;

  WriteToClient(client, 36, (char *)colorMatrix);
  if (client->noClientException != Success)
    return client->noClientException;
  WriteToClient(client, 12, (char *)colorOffset);
  if (client->noClientException != Success)
    return client->noClientException;
  WriteToClient(client, 12, (char *)colorScale);

  return client->noClientException;
}

static int SProcNVCTRLQueryGvoColorConversion(ClientPtr client)
{
  REQUEST(xnvCtrlQueryGvoColorConversionReq);
  swaps(&stuff->length);
  REQUEST_SIZE_MATCH(xnvCtrlQueryGvoColorConversionReq);
  return ProcNVCTRLQueryGvoColorConversion(client);
}


static int ProcNVCTRLQueryBinaryData(ClientPtr client)
{
  xnvCtrlQueryBinaryDataReply rep;
  unsigned char *ptr = NULL;
  int len = 0;
  REQUEST(xnvCtrlQueryBinaryDataReq);

  X_INIT();

  REQUEST_SIZE_MATCH(xnvCtrlQueryBinaryDataReq);

  rep.flags =
    XNVCTRLQueryTargetBinaryData(dpy, stuff->target_type, stuff->target_id,
                                 stuff->display_mask, stuff->attribute, &ptr,
                                 &len);
  if (error_code != Success)
    free(ptr);
  X_DEINIT();

  rep.type = X_Reply;
  rep.n = ptr ? len : 0;
  rep.length = (rep.n + 3) >> 2;
  rep.sequenceNumber = client->sequence;
  if (client->swapped) {
    swaps(&rep.sequenceNumber);
    swapl(&rep.length);
    swapl(&rep.flags);
    swapl(&rep.n);
  }
  WriteToClient(client, sizeof(xnvCtrlQueryBinaryDataReply), (char *)&rep);
  if (ptr) {
    if (client->noClientException == Success)
      WriteToClient(client, rep.n, ptr);
    free(ptr);
  }
  return client->noClientException;
}

static int SProcNVCTRLQueryBinaryData(ClientPtr client)
{
  REQUEST(xnvCtrlQueryBinaryDataReq);
  swaps(&stuff->length);
  REQUEST_SIZE_MATCH(xnvCtrlQueryBinaryDataReq);
  swaps(&stuff->target_id);
  swaps(&stuff->target_type);
  swapl(&stuff->display_mask);
  swapl(&stuff->attribute);
  return ProcNVCTRLQueryBinaryData(client);
}


static int ProcNVCTRLStringOperation(ClientPtr client)
{
  xnvCtrlStringOperationReply rep;
  char *ptr = NULL;
  REQUEST(xnvCtrlStringOperationReq);

  X_INIT();

  REQUEST_FIXED_SIZE(xnvCtrlStringOperationReq, stuff->num_bytes);

  rep.ret =
    XNVCTRLStringOperation(dpy, stuff->target_type, stuff->target_id,
                           stuff->display_mask, stuff->attribute,
                           (char *)&stuff[1], &ptr);
  if (error_code != Success)
    free(ptr);
  X_DEINIT();

  rep.type = X_Reply;
  rep.num_bytes = ptr ? strlen(ptr) + 1 : 0;
  rep.length = (rep.num_bytes + 3) >> 2;
  rep.sequenceNumber = client->sequence;
  if (client->swapped) {
    swaps(&rep.sequenceNumber);
    swapl(&rep.length);
    swapl(&rep.ret);
    swapl(&rep.num_bytes);
  }
  WriteToClient(client, sizeof(xnvCtrlStringOperationReply), (char *)&rep);
  if (ptr) {
    if (client->noClientException == Success)
      WriteToClient(client, rep.num_bytes, ptr);
    free(ptr);
  }
  return client->noClientException;
}

static int SProcNVCTRLStringOperation(ClientPtr client)
{
  REQUEST(xnvCtrlStringOperationReq);
  swaps(&stuff->length);
  REQUEST_SIZE_MATCH(xnvCtrlStringOperationReq);
  swaps(&stuff->target_id);
  swaps(&stuff->target_type);
  swapl(&stuff->display_mask);
  swapl(&stuff->attribute);
  swapl(&stuff->num_bytes);
  return ProcNVCTRLStringOperation(client);
}


static int ProcNVCTRLDispatch(ClientPtr client)
{
  REQUEST(xReq);
  switch (stuff->data) {
    case X_nvCtrlQueryExtension:
      return ProcNVCTRLQueryExtension(client);
    case X_nvCtrlIsNv:
      return ProcNVCTRLIsNv(client);
    case X_nvCtrlQueryTargetCount:
      return ProcNVCTRLQueryTargetCount(client);
    case X_nvCtrlSetAttribute:
      return ProcNVCTRLSetAttribute(client);
    case X_nvCtrlSetAttributeAndGetStatus:
      return ProcNVCTRLSetAttributeAndGetStatus(client);
    case X_nvCtrlQueryAttribute:
      return ProcNVCTRLQueryAttribute(client);
    case X_nvCtrlQueryAttribute64:
      return ProcNVCTRLQueryAttribute64(client);
    case X_nvCtrlQueryStringAttribute:
      return ProcNVCTRLQueryStringAttribute(client);
    case X_nvCtrlSetStringAttribute:
      return ProcNVCTRLSetStringAttribute(client);
    case X_nvCtrlQueryValidAttributeValues:
      return ProcNVCTRLQueryValidAttributeValues(client);
    case X_nvCtrlQueryValidAttributeValues64:
      return ProcNVCTRLQueryValidAttributeValues64(client);
    case X_nvCtrlQueryAttributePermissions:
    case X_nvCtrlQueryStringAttributePermissions:
    case X_nvCtrlQueryBinaryDataAttributePermissions:
    case X_nvCtrlQueryStringOperationAttributePermissions:
      return ProcNVCTRLQueryAttributePermissions(client, stuff->data);
    case X_nvCtrlSetGvoColorConversion:
      return ProcNVCTRLSetGvoColorConversion(client);
    case X_nvCtrlQueryGvoColorConversion:
      return ProcNVCTRLQueryGvoColorConversion(client);
    case X_nvCtrlQueryBinaryData:
      return ProcNVCTRLQueryBinaryData(client);
    case X_nvCtrlStringOperation:
      return ProcNVCTRLStringOperation(client);
    default:
      return BadRequest;
  }
}


static int SProcNVCTRLDispatch(ClientPtr client)
{
  REQUEST(xReq);
  switch (stuff->data) {
    case X_nvCtrlQueryExtension:
      return SProcNVCTRLQueryExtension(client);
    case X_nvCtrlIsNv:
      return SProcNVCTRLIsNv(client);
    case X_nvCtrlQueryTargetCount:
      return SProcNVCTRLQueryTargetCount(client);
    case X_nvCtrlSetAttribute:
      return SProcNVCTRLSetAttribute(client);
    case X_nvCtrlSetAttributeAndGetStatus:
      return SProcNVCTRLSetAttributeAndGetStatus(client);
    case X_nvCtrlQueryAttribute:
      return SProcNVCTRLQueryAttribute(client);
    case X_nvCtrlQueryAttribute64:
      return SProcNVCTRLQueryAttribute64(client);
    case X_nvCtrlQueryStringAttribute:
      return SProcNVCTRLQueryStringAttribute(client);
    case X_nvCtrlSetStringAttribute:
      return SProcNVCTRLSetStringAttribute(client);
    case X_nvCtrlQueryValidAttributeValues:
      return SProcNVCTRLQueryValidAttributeValues(client);
    case X_nvCtrlQueryValidAttributeValues64:
      return SProcNVCTRLQueryValidAttributeValues64(client);
    case X_nvCtrlQueryAttributePermissions:
    case X_nvCtrlQueryStringAttributePermissions:
    case X_nvCtrlQueryBinaryDataAttributePermissions:
    case X_nvCtrlQueryStringOperationAttributePermissions:
      return SProcNVCTRLQueryAttributePermissions(client);
    case X_nvCtrlSetGvoColorConversion:
      return SProcNVCTRLSetGvoColorConversion(client);
    case X_nvCtrlQueryGvoColorConversion:
      return SProcNVCTRLQueryGvoColorConversion(client);
    case X_nvCtrlQueryBinaryData:
      return SProcNVCTRLQueryBinaryData(client);
    case X_nvCtrlStringOperation:
      return SProcNVCTRLStringOperation(client);
    default:
      return BadRequest;
  }
}
