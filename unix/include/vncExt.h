/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
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
#ifndef _VNCEXT_H_
#define _VNCEXT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define X_VncExtSetParam 0
#define X_VncExtGetParam 1
#define X_VncExtGetParamDesc 2
#define X_VncExtListParams 3
#define X_VncExtSelectInput 6
#define X_VncExtConnect 7

#define VncExtNumberEvents 3
#define VncExtNumberErrors 0

#ifndef _VNCEXT_SERVER_

Bool XVncExtQueryExtension(Display* dpy, int* event_basep, int* error_basep);
Bool XVncExtSetParam(Display* dpy, const char* param);
Bool XVncExtGetParam(Display* dpy, const char* param, char** value, int* len);
char* XVncExtGetParamDesc(Display* dpy, const char* param);
char** XVncExtListParams(Display* dpy, int* nParams);
void XVncExtFreeParamList(char** list);
Bool XVncExtConnect(Display* dpy, const char* hostAndPort);

#endif

#ifdef _VNCEXT_PROTO_

#define VNCEXTNAME "VNC-EXTENSION"

typedef struct {
  CARD8 reqType;       /* always VncExtReqCode */
  CARD8 vncExtReqType; /* always VncExtSetParam */
  CARD16 length B16;
  CARD8 paramLen;
  CARD8 pad0;
  CARD16 pad1 B16;
} xVncExtSetParamReq;
#define sz_xVncExtSetParamReq 8

typedef struct {
 BYTE type; /* X_Reply */
 BYTE success;
 CARD16 sequenceNumber B16;
 CARD32 length B32;
 CARD32 pad0 B32;
 CARD32 pad1 B32;
 CARD32 pad2 B32;
 CARD32 pad3 B32;
 CARD32 pad4 B32;
 CARD32 pad5 B32;
} xVncExtSetParamReply;
#define sz_xVncExtSetParamReply 32


typedef struct {
  CARD8 reqType;       /* always VncExtReqCode */
  CARD8 vncExtReqType; /* always VncExtGetParam */
  CARD16 length B16;
  CARD8 paramLen;
  CARD8 pad0;
  CARD16 pad1 B16;
} xVncExtGetParamReq;
#define sz_xVncExtGetParamReq 8

typedef struct {
 BYTE type; /* X_Reply */
 BYTE success;
 CARD16 sequenceNumber B16;
 CARD32 length B32;
 CARD16 valueLen B16;
 CARD16 pad0 B16;
 CARD32 pad1 B32;
 CARD32 pad2 B32;
 CARD32 pad3 B32;
 CARD32 pad4 B32;
 CARD32 pad5 B32;
} xVncExtGetParamReply;
#define sz_xVncExtGetParamReply 32


typedef struct {
  CARD8 reqType;       /* always VncExtReqCode */
  CARD8 vncExtReqType; /* always VncExtGetParamDesc */
  CARD16 length B16;
  CARD8 paramLen;
  CARD8 pad0;
  CARD16 pad1 B16;
} xVncExtGetParamDescReq;
#define sz_xVncExtGetParamDescReq 8

typedef struct {
 BYTE type; /* X_Reply */
 BYTE success;
 CARD16 sequenceNumber B16;
 CARD32 length B32;
 CARD16 descLen B16;
 CARD16 pad0 B16;
 CARD32 pad1 B32;
 CARD32 pad2 B32;
 CARD32 pad3 B32;
 CARD32 pad4 B32;
 CARD32 pad5 B32;
} xVncExtGetParamDescReply;
#define sz_xVncExtGetParamDescReply 32


typedef struct {
  CARD8 reqType;       /* always VncExtReqCode */
  CARD8 vncExtReqType; /* always VncExtListParams */
  CARD16 length B16;
} xVncExtListParamsReq;
#define sz_xVncExtListParamsReq 4

typedef struct {
 BYTE type; /* X_Reply */
 BYTE pad0;
 CARD16 sequenceNumber B16;
 CARD32 length B32;
 CARD16 nParams B16;
 CARD16 pad1 B16;
 CARD32 pad2 B32;
 CARD32 pad3 B32;
 CARD32 pad4 B32;
 CARD32 pad5 B32;
 CARD32 pad6 B32;
} xVncExtListParamsReply;
#define sz_xVncExtListParamsReply 32


typedef struct {
  CARD8 reqType;       /* always VncExtReqCode */
  CARD8 vncExtReqType; /* always VncExtSelectInput */
  CARD16 length B16;
  CARD32 window B32;
  CARD32 mask B32;
} xVncExtSelectInputReq;
#define sz_xVncExtSelectInputReq 12


typedef struct {
  CARD8 reqType;       /* always VncExtReqCode */
  CARD8 vncExtReqType; /* always VncExtConnect */
  CARD16 length B16;
  CARD8 strLen;
  CARD8 pad0;
  CARD16 pad1 B16;
} xVncExtConnectReq;
#define sz_xVncExtConnectReq 8

typedef struct {
 BYTE type; /* X_Reply */
 BYTE success;
 CARD16 sequenceNumber B16;
 CARD32 length B32;
 CARD32 pad0 B32;
 CARD32 pad1 B32;
 CARD32 pad2 B32;
 CARD32 pad3 B32;
 CARD32 pad4 B32;
 CARD32 pad5 B32;
} xVncExtConnectReply;
#define sz_xVncExtConnectReply 32

#endif

#ifdef __cplusplus
}
#endif

#endif
