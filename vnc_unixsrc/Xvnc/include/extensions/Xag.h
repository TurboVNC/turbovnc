/*
Copyright (c) 1996  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.
*/
/* $XConsortium: Xag.h /main/2 1996/11/12 12:18:22 swick $ */

#ifndef _XAG_H_
#define _XAG_H_

#include <X11/Xfuncproto.h>

#define X_XagQueryVersion		0
#define X_XagCreate			1
#define X_XagDestroy			2
#define X_XagGetAttr			3
#define X_XagQuery			4
#define X_XagCreateAssoc		5
#define X_XagDestroyAssoc		6

#define XagBadAppGroup			0
#define XagNumberErrors			(XagBadAppGroup + 1)

#define XagNsingleScreen		0
#define XagNdefaultRoot			1
#define XagNrootVisual			2
#define XagNdefaultColormap		3
#define XagNblackPixel			4
#define XagNwhitePixel			5
#define XagNappGroupLeader		6

#ifndef _XAG_SERVER_

#if NeedVarargsPrototypes
#include <stdarg.h>
#else
#include <varargs.h>
#endif

_XFUNCPROTOBEGIN

typedef XID XAppGroup;

Bool XagQueryVersion(
#if NeedFunctionPrototypes
    Display*			/* dpy */,
    int*			/* major_version */,
    int*			/* minor_version */
#endif
);

Status XagCreateEmbeddedApplicationGroup(
#if NeedFunctionPrototypes
    Display*			/* dpy */,
    VisualID			/* root_visual */,
    Colormap			/* default_colormap */,
    unsigned long		/* black_pixel */,
    unsigned long		/* white_pixel */,
    XAppGroup*			/* app_group_return */
#endif
);

Status XagCreateNonembeddedApplicationGroup(
#if NeedFunctionPrototypes
    Display*			/* dpy */,
    XAppGroup*			/* app_group_return */
#endif
);

Status XagDestroyApplicationGroup(
#if NeedFunctionPrototypes
    Display*			/* dpy */,
    XAppGroup			/* app_group */
#endif
);

Status XagGetApplicationGroupAttributes(
#if NeedVarargsPrototypes
    Display*			/* dpy */,
    XAppGroup			/* app_group */,
    ...
#endif
);

Status XagQueryApplicationGroup(
#if NeedFunctionPrototypes
    Display*			/* dpy */,
    XID				/* resource_base */,
    XAppGroup*			/* app_group_ret */
#endif
);

Status XagCreateAssociation(
#if NeedFunctionPrototypes
    Display*			/* dpy */,
    Window*			/* window_ret */,
    void*			/* system_window */
#endif
);

Status XagDestroyAssociation(
#if NeedFunctionPrototypes
    Display*			/* dpy */,
    Window			/* window */
#endif
);

_XFUNCPROTOEND

#endif /* _XAG_SERVER_ */

#endif /* _XAG_H_ */

