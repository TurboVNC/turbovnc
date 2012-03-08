/* $XConsortium: XTest.h,v 1.9 94/04/17 20:11:18 rws Exp $ */
/* $XFree86: xc/include/extensions/XTest.h,v 3.0 1996/12/12 09:12:53 dawes Exp $ */
/*

Copyright (c) 1992  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/

#ifndef _XTEST_H_
#define _XTEST_H_

#include <X11/Xfuncproto.h>

#define X_XTestGetVersion	0
#define X_XTestCompareCursor	1
#define X_XTestFakeInput	2
#define X_XTestGrabControl	3

#define XTestNumberEvents	0

#define XTestNumberErrors	0

#define XTestMajorVersion	2
#define XTestMinorVersion	2

#define XTestExtensionName	"XTEST"

#ifndef _XTEST_SERVER_

#include <X11/extensions/XInput.h>

_XFUNCPROTOBEGIN

Bool XTestQueryExtension(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    int*		/* event_basep */,
    int*		/* error_basep */,
    int*		/* majorp */,
    int*		/* minorp */
#endif
);

Bool XTestCompareCursorWithWindow(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Window		/* window */,
    Cursor		/* cursor */
#endif
);

Bool XTestCompareCurrentCursorWithWindow(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Window		/* window */
#endif
);

extern int XTestFakeKeyEvent(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    unsigned int	/* keycode */,
    Bool		/* is_press */,
    unsigned long	/* delay */
#endif
);

extern int XTestFakeButtonEvent(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    unsigned int	/* button */,
    Bool		/* is_press */,
    unsigned long	/* delay */
#endif
);

extern int XTestFakeMotionEvent(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    int			/* screen */,
    int			/* x */,
    int			/* y */,
    unsigned long	/* delay */
#endif
);

extern int XTestFakeRelativeMotionEvent(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    int			/* x */,
    int			/* y */,
    unsigned long	/* delay */
#endif
);

extern int XTestFakeDeviceKeyEvent(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    XDevice*		/* dev */,
    unsigned int	/* keycode */,
    Bool		/* is_press */,
    int*		/* axes */,
    int			/* n_axes */,
    unsigned long	/* delay */
#endif
);

extern int XTestFakeDeviceButtonEvent(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    XDevice*		/* dev */,
    unsigned int	/* button */,
    Bool		/* is_press */,
    int*		/* axes */,
    int			/* n_axes */,
    unsigned long	/* delay */
#endif
);

extern int XTestFakeProximityEvent(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    XDevice*		/* dev */,
    Bool		/* in_prox */,
    int*		/* axes */,
    int			/* n_axes */,
    unsigned long	/* delay */
#endif
);

extern int XTestFakeDeviceMotionEvent(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    XDevice*		/* dev */,
    Bool		/* is_relative */,
    int			/* first_axis */,
    int*		/* axes */,
    int			/* n_axes */,
    unsigned long	/* delay */
#endif
);

extern int XTestGrabControl(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Bool		/* impervious */
#endif
);

void XTestSetGContextOfGC(
#if NeedFunctionPrototypes
    GC			/* gc */,
    GContext		/* gid */
#endif
);

void XTestSetVisualIDOfVisual(
#if NeedFunctionPrototypes
    Visual*		/* visual */,
    VisualID		/* visualid */
#endif
);

Status XTestDiscard(
#if NeedFunctionPrototypes
    Display*		/* dpy */
#endif
);

_XFUNCPROTOEND

#endif /* _XTEST_SERVER_ */

#endif
