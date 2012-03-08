/************************************************************

Copyright (c) 1989  X Consortium

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

********************************************************/

/* $XConsortium: shape.h,v 1.17 94/04/17 20:11:23 rws Exp $ */

#ifndef _SHAPE_H_
#define _SHAPE_H_

#include <X11/Xfuncproto.h>

#define X_ShapeQueryVersion		0
#define X_ShapeRectangles		1
#define X_ShapeMask			2
#define X_ShapeCombine			3
#define X_ShapeOffset			4
#define X_ShapeQueryExtents		5
#define X_ShapeSelectInput		6
#define X_ShapeInputSelected		7
#define X_ShapeGetRectangles		8

#define ShapeSet			0
#define ShapeUnion			1
#define ShapeIntersect			2
#define ShapeSubtract			3
#define ShapeInvert			4

#define ShapeBounding			0
#define ShapeClip			1

#define ShapeNotifyMask			(1L << 0)
#define ShapeNotify			0

#define ShapeNumberEvents		(ShapeNotify + 1)

#ifndef _SHAPE_SERVER_
typedef struct {
    int	type;		    /* of event */
    unsigned long serial;   /* # of last request processed by server */
    Bool send_event;	    /* true if this came frome a SendEvent request */
    Display *display;	    /* Display the event was read from */
    Window window;	    /* window of event */
    int kind;		    /* ShapeBounding or ShapeClip */
    int x, y;		    /* extents of new region */
    unsigned width, height;
    Time time;		    /* server timestamp when region changed */
    Bool shaped;	    /* true if the region exists */
} XShapeEvent;

_XFUNCPROTOBEGIN

extern Bool XShapeQueryExtension (
#if NeedFunctionPrototypes
    Display*	/* display */,
    int*	/* event_base */,
    int*	/* error_base */
#endif
);

extern Status XShapeQueryVersion (
#if NeedFunctionPrototypes
    Display*	/* display */,
    int*	/* major_version */,
    int*	/* minor_version */
#endif
);

extern void XShapeCombineRegion (
#if NeedFunctionPrototypes
    Display*	/* display */,
    Window	/* dest */,
    int		/* dest_kind */,
    int		/* x_off */,
    int		/* y_off */,
    Region	/* region */,
    int		/* op */
#endif
);

extern void XShapeCombineRectangles (
#if NeedFunctionPrototypes
    Display*	/* display */,
    Window	/* dest */,
    int		/* dest_kind */,
    int		/* x_off */,
    int		/* y_off */,
    XRectangle*	/* rectangles */,
    int		/* n_rects */,
    int		/* op */,
    int		/* ordering */
#endif
);

extern void XShapeCombineMask (
#if NeedFunctionPrototypes
    Display*	/* display */,
    Window	/* dest */,
    int		/* dest_kind */,
    int		/* x_off */,
    int		/* y_off */,
    Pixmap	/* src */,
    int		/* op */
#endif
);

extern void XShapeCombineShape (
#if NeedFunctionPrototypes
    Display*	/* display */,
    Window	/* dest */,
    int		/* dest_kind */,
    int		/* x_off */,
    int		/* y_off */,
    Window	/* src */,
    int		/* src_kind */,
    int		/* op */
#endif
);

extern void XShapeOffsetShape (
#if NeedFunctionPrototypes
    Display*	/* display */,
    Window	/* dest */,
    int		/* dest_kind */,
    int		/* x_off */,
    int		/* y_off */
#endif
);

extern Status XShapeQueryExtents (
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* window */,
    Bool*		/* bounding_shaped */,
    int*		/* x_bounding */,
    int*		/* y_bounding */,
    unsigned int*	/* w_bounding */,
    unsigned int*	/* h_bounding */,
    Bool*		/* clip_shaped */,
    int*		/* x_clip */,
    int*		/* y_clip */,
    unsigned int*	/* w_clip */,
    unsigned int*	/* h_clip */
#endif
);

extern void XShapeSelectInput (
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* window */,
    unsigned long	/* mask */
#endif
);

extern unsigned long XShapeInputSelected (
#if NeedFunctionPrototypes
    Display*	/* display */,
    Window	/* window */
#endif
);

extern XRectangle *XShapeGetRectangles (
#if NeedFunctionPrototypes
    Display*	/* display */,
    Window	/* window */,
    int		/* kind */,
    int*	/* count */,
    int*	/* ordering */
#endif
);

_XFUNCPROTOEND

#endif /* _SHAPE_SERVER_ */

#endif /* _SHAPE_H_ */
