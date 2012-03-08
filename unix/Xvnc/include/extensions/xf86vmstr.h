/* $XFree86: xc/include/extensions/xf86vmstr.h,v 3.18.2.1 1997/05/25 14:13:37 dawes Exp $ */
/*

Copyright (c) 1995  Kaleb S. KEITHLEY

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL Kaleb S. KEITHLEY BE LIABLE FOR ANY CLAIM, DAMAGES 
OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Kaleb S. KEITHLEY 
shall not be used in advertising or otherwise to promote the sale, use 
or other dealings in this Software without prior written authorization
from Kaleb S. KEITHLEY

*/
/* $XConsortium: xf86vmstr.h /main/10 1996/10/26 21:38:17 kaleb $ */

/* THIS IS NOT AN X CONSORTIUM STANDARD */

#ifndef _XF86VIDMODESTR_H_
#define _XF86VIDMODESTR_H_

#include "xf86vmode.h"

#define XF86VIDMODENAME "XFree86-VidModeExtension"

#define XF86VIDMODE_MAJOR_VERSION	0	/* current version numbers */
#define XF86VIDMODE_MINOR_VERSION	8
/*
 * major version 0 == uses parameter-to-wire functions in XFree86 libXxf86vm.
 * major version 1 == uses parameter-to-wire functions hard-coded in xvidtune
 *                    client.
 */

typedef struct _XF86VidModeQueryVersion {
    CARD8	reqType;		/* always XF86VidModeReqCode */
    CARD8	xf86vidmodeReqType;	/* always X_XF86VidModeQueryVersion */
    CARD16	length B16;
} xXF86VidModeQueryVersionReq;
#define sz_xXF86VidModeQueryVersionReq	4

typedef struct {
    BYTE	type;			/* X_Reply */
    BOOL	pad1;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD16	majorVersion B16;	/* major version of XF86VidMode */
    CARD16	minorVersion B16;	/* minor version of XF86VidMode */
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
} xXF86VidModeQueryVersionReply;
#define sz_xXF86VidModeQueryVersionReply	32

typedef struct _XF86VidModeGetModeLine {
    CARD8	reqType;		/* always XF86VidModeReqCode */
    CARD8	xf86vidmodeReqType;
    CARD16	length B16;
    CARD16	screen B16;
    CARD16	pad B16;
} xXF86VidModeGetModeLineReq,
  xXF86VidModeGetAllModeLinesReq,
  xXF86VidModeGetMonitorReq,
  xXF86VidModeGetViewPortReq;
#define sz_xXF86VidModeGetModeLineReq		8
#define sz_xXF86VidModeGetAllModeLinesReq	8
#define sz_xXF86VidModeGetMonitorReq		8
#define sz_xXF86VidModeGetViewPortReq		8

typedef struct {
    BYTE	type;			/* X_Reply */
    BOOL	pad1;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	dotclock B32;
    CARD16	hdisplay B16;
    CARD16	hsyncstart B16;
    CARD16	hsyncend B16;
    CARD16	htotal B16;
    CARD16	vdisplay B16;
    CARD16	vsyncstart B16;
    CARD16	vsyncend B16;
    CARD16	vtotal B16;
    CARD32	flags B32;
    CARD32	privsize B32;
} xXF86VidModeGetModeLineReply;
#define sz_xXF86VidModeGetModeLineReply	36

typedef struct {
    CARD32	dotclock B32;
    CARD16	hdisplay B16;
    CARD16	hsyncstart B16;
    CARD16	hsyncend B16;
    CARD16	htotal B16;
    CARD16	vdisplay B16;
    CARD16	vsyncstart B16;
    CARD16	vsyncend B16;
    CARD16	vtotal B16;
    CARD32	flags B32;
    CARD32	privsize B32;
} xXF86VidModeModeInfo;

typedef struct {
    BYTE	type;			/* X_Reply */
    BOOL	pad1;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	modecount B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
} xXF86VidModeGetAllModeLinesReply;
#define sz_xXF86VidModeGetAllModeLinesReply	32

typedef struct _XF86VidModeAddModeLine {
    CARD8	reqType;		/* always XF86VidModeReqCode */
    CARD8	xf86vidmodeReqType;	/* always X_XF86VidModeAddMode */
    CARD16	length B16;
    CARD32	screen B32;		/* could be CARD16 but need the pad */
    CARD32	dotclock B32;
    CARD16	hdisplay B16;
    CARD16	hsyncstart B16;
    CARD16	hsyncend B16;
    CARD16	htotal B16;
    CARD16	vdisplay B16;
    CARD16	vsyncstart B16;
    CARD16	vsyncend B16;
    CARD16	vtotal B16;
    CARD32	flags B32;
    CARD32	privsize B32;
    CARD32	after_dotclock B32;
    CARD16	after_hdisplay B16;
    CARD16	after_hsyncstart B16;
    CARD16	after_hsyncend B16;
    CARD16	after_htotal B16;
    CARD16	after_vdisplay B16;
    CARD16	after_vsyncstart B16;
    CARD16	after_vsyncend B16;
    CARD16	after_vtotal B16;
    CARD32	after_flags B32;
} xXF86VidModeAddModeLineReq;
#define sz_xXF86VidModeAddModeLineReq	60

typedef struct _XF86VidModeModModeLine {
    CARD8	reqType;		/* always XF86VidModeReqCode */
    CARD8	xf86vidmodeReqType;	/* always X_XF86VidModeModModeLine */
    CARD16	length B16;
    CARD32	screen B32;		/* could be CARD16 but need the pad */
    CARD16	hdisplay B16;
    CARD16	hsyncstart B16;
    CARD16	hsyncend B16;
    CARD16	htotal B16;
    CARD16	vdisplay B16;
    CARD16	vsyncstart B16;
    CARD16	vsyncend B16;
    CARD16	vtotal B16;
    CARD32	flags B32;
    CARD32	privsize B32;
} xXF86VidModeModModeLineReq;
#define sz_xXF86VidModeModModeLineReq	32

typedef struct _XF86VidModeValidateModeLine {
    CARD8	reqType;		/* always XF86VidModeReqCode */
    CARD8	xf86vidmodeReqType;
    CARD16	length B16;
    CARD32	screen B32;		/* could be CARD16 but need the pad */
    CARD32	dotclock B32;
    CARD16	hdisplay B16;
    CARD16	hsyncstart B16;
    CARD16	hsyncend B16;
    CARD16	htotal B16;
    CARD16	vdisplay B16;
    CARD16	vsyncstart B16;
    CARD16	vsyncend B16;
    CARD16	vtotal B16;
    CARD32	flags B32;
    CARD32	privsize B32;
} xXF86VidModeDeleteModeLineReq,
  xXF86VidModeValidateModeLineReq,
  xXF86VidModeSwitchToModeReq;
#define sz_xXF86VidModeDeleteModeLineReq	36
#define sz_xXF86VidModeValidateModeLineReq	36
#define sz_xXF86VidModeSwitchToModeReq		36

typedef struct _XF86VidModeSwitchMode {
    CARD8	reqType;		/* always XF86VidModeReqCode */
    CARD8	xf86vidmodeReqType;	/* always X_XF86VidModeSwitchMode */
    CARD16	length B16;
    CARD16	screen B16;
    CARD16	zoom B16;
} xXF86VidModeSwitchModeReq;
#define sz_xXF86VidModeSwitchModeReq	8

typedef struct _XF86VidModeLockModeSwitch {
    CARD8	reqType;		/* always XF86VidModeReqCode */
    CARD8	xf86vidmodeReqType;	/* always X_XF86VidModeLockModeSwitch */
    CARD16	length B16;
    CARD16	screen B16;
    CARD16	lock B16;
} xXF86VidModeLockModeSwitchReq;
#define sz_xXF86VidModeLockModeSwitchReq	8

typedef struct {
    BYTE	type;			/* X_Reply */
    BOOL	pad1;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	status B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
} xXF86VidModeValidateModeLineReply;
#define sz_xXF86VidModeValidateModeLineReply	32

typedef struct {
    BYTE	type;			/* X_Reply */
    BOOL	pad1;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD8	vendorLength;
    CARD8	modelLength;
    CARD8	nhsync;
    CARD8	nvsync;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
} xXF86VidModeGetMonitorReply;
#define sz_xXF86VidModeGetMonitorReply	32

typedef struct {
    BYTE	type;
    BOOL	pad1;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	x B32;
    CARD32	y B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
} xXF86VidModeGetViewPortReply;
#define sz_xXF86VidModeGetViewPortReply	32

typedef struct _XF86VidModeSetViewPort {
    CARD8	reqType;		/* always VidModeReqCode */
    CARD8	xf86vidmodeReqType;	/* always X_XF86VidModeSetViewPort */
    CARD16	length B16;
    CARD16	screen B16;
    CARD16	pad B16;
    CARD32      x B32;
    CARD32	y B32;
} xXF86VidModeSetViewPortReq;
#define sz_xXF86VidModeSetViewPortReq	16

#endif /* _XF86VIDMODESTR_H_ */
