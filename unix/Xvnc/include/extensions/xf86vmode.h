/* $XFree86: xc/include/extensions/xf86vmode.h,v 3.20.2.1 1997/07/13 14:44:57 dawes Exp $ */
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
/* $XConsortium: xf86vmode.h /main/9 1996/10/26 21:38:11 kaleb $ */

/* THIS IS NOT AN X CONSORTIUM STANDARD */

#ifndef _XF86VIDMODE_H_
#define _XF86VIDMODE_H_

#include <X11/Xfuncproto.h>
#include <X11/Xmd.h>

#define X_XF86VidModeQueryVersion	0
#define X_XF86VidModeGetModeLine	1
#define X_XF86VidModeModModeLine	2
#define X_XF86VidModeSwitchMode		3
#define X_XF86VidModeGetMonitor		4
#define X_XF86VidModeLockModeSwitch	5
#define X_XF86VidModeGetAllModeLines	6
#define X_XF86VidModeAddModeLine	7
#define X_XF86VidModeDeleteModeLine	8
#define X_XF86VidModeValidateModeLine	9
#define X_XF86VidModeSwitchToMode	10
#define X_XF86VidModeGetViewPort	11
#define X_XF86VidModeSetViewPort	12

#ifdef XF86VIDMODE_EVENTS
#define XF86VidModeNotify		0
#define XF86VidModeNumberEvents		(XF86VidModeNotify + 1)

#define XF86VidModeNotifyMask		0x00000001

#define XF86VidModeNonEvent		0
#define XF86VidModeModeChange		1
#else
#define XF86VidModeNumberEvents		0
#endif

#define XF86VidModeBadClock		0
#define XF86VidModeBadHTimings		1
#define XF86VidModeBadVTimings		2
#define XF86VidModeModeUnsuitable	3
#define XF86VidModeExtensionDisabled	4
#define XF86VidModeClientNotLocal	5
#define XF86VidModeZoomLocked		6
#define XF86VidModeNumberErrors		(XF86VidModeZoomLocked + 1)

#ifndef _XF86VIDMODE_SERVER_

typedef struct {
    unsigned short	hdisplay;
    unsigned short	hsyncstart;
    unsigned short	hsyncend;
    unsigned short	htotal;
    unsigned short	vdisplay;
    unsigned short	vsyncstart;
    unsigned short	vsyncend;
    unsigned short	vtotal;
    unsigned int	flags;
    int			privsize;
#if defined(__cplusplus) || defined(c_plusplus)
    /* private is a C++ reserved word */
    INT32		*c_private;
#else
    INT32		*private;
#endif
} XF86VidModeModeLine;

typedef struct {
    unsigned int	dotclock;
    unsigned short	hdisplay;
    unsigned short	hsyncstart;
    unsigned short	hsyncend;
    unsigned short	htotal;
    unsigned short	vdisplay;
    unsigned short	vsyncstart;
    unsigned short	vsyncend;
    unsigned short	vtotal;
    unsigned int	flags;
    int			privsize;
#if defined(__cplusplus) || defined(c_plusplus)
    /* private is a C++ reserved word */
    INT32		*c_private;
#else
    INT32		*private;
#endif
} XF86VidModeModeInfo;

typedef struct {
    float		hi;
    float		lo;
} XF86VidModeSyncRange;

typedef struct {
    char*			vendor;
    char*			model;
    float			EMPTY;
    unsigned char		nhsync;
    XF86VidModeSyncRange*	hsync;
    unsigned char		nvsync;
    XF86VidModeSyncRange*	vsync;
} XF86VidModeMonitor;
    
typedef struct {
    int type;			/* of event */
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;		/* true if this came from a SendEvent req */
    Display *display;		/* Display the event was read from */
    Window root;		/* root window of event screen */
    int state;			/* What happened */
    int kind;			/* What happened */
    Bool forced;		/* extents of new region */
    Time time;			/* event timestamp */
} XF86VidModeNotifyEvent;

#define XF86VidModeSelectNextMode(disp, scr) \
	XF86VidModeSwitchMode(disp, scr, 1)
#define XF86VidModeSelectPrevMode(disp, scr) \
	XF86VidModeSwitchMode(disp, scr, -1)

_XFUNCPROTOBEGIN

Bool XF86VidModeQueryVersion(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    int*		/* majorVersion */,
    int*		/* minorVersion */
#endif
);

Bool XF86VidModeQueryExtension(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    int*		/* event_base */,
    int*		/* error_base */
#endif
);

Bool XF86VidModeGetModeLine(
#if NeedFunctionPrototypes
    Display*			/* dpy */,
    int				/* screen */,
    int*			/* dotclock */,
    XF86VidModeModeLine*	/* modeline */
#endif
);

Bool XF86VidModeGetAllModeLines(
#if NeedFunctionPrototypes
    Display*			/* dpy */,
    int				/* screen */,
    int*			/* modecount */,
    XF86VidModeModeInfo***	/* modelinesPtr */
#endif
);

Bool XF86VidModeAddModeLine(
#if NeedFunctionPrototypes
    Display*			/* dpy */,
    int				/* screen */,
    XF86VidModeModeInfo*	/* new modeline */,
    XF86VidModeModeInfo*	/* after modeline */
#endif
);

Bool XF86VidModeDeleteModeLine(
#if NeedFunctionPrototypes
    Display*			/* dpy */,
    int				/* screen */,
    XF86VidModeModeInfo*	/* modeline */
#endif
);

Bool XF86VidModeModModeLine(
#if NeedFunctionPrototypes
    Display*			/* dpy */,
    int				/* screen */,
    XF86VidModeModeLine*	/* modeline */
#endif
);

Status XF86VidModeValidateModeLine(
#if NeedFunctionPrototypes
    Display*			/* dpy */,
    int				/* screen */,
    XF86VidModeModeInfo*	/* modeline */
#endif
);

Bool XF86VidModeSwitchMode(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    int			/* screen */,
    int			/* zoom */
#endif
);

Bool XF86VidModeSwitchToMode(
#if NeedFunctionPrototypes
    Display*			/* dpy */,
    int				/* screen */,
    XF86VidModeModeInfo*	/* modeline */
#endif
);

Bool XF86VidModeLockModeSwitch(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    int			/* screen */,
    int			/* lock */
#endif
);

Bool XF86VidModeGetMonitor(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    int			/* screen */,
    XF86VidModeMonitor*	/* monitor */
#endif
);

Bool XF86VidModeGetViewPort(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    int			/* screen */,
    int*		/* x return */,
    int*		/* y return */
#endif
);

Bool XF86VidModeSetViewPort(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    int			/* screen */,
    int			/* x */,
    int			/* y */
#endif
);

_XFUNCPROTOEND

#endif

#endif
