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

/* THIS IS NOT AN X CONSORTIUM STANDARD */

/* $XConsortium: XShm.h,v 1.7 94/09/22 21:25:20 dpw Exp $ */




/* $XFree86: xc/include/extensions/XShm.h,v 1.2.2.1 1997/08/04 02:10:32 dawes Exp $ */

#ifndef _XSHM_H_
#define _XSHM_H_

#include <X11/Xfuncproto.h>

#define X_ShmQueryVersion		0
#define X_ShmAttach			1
#define X_ShmDetach			2
#define X_ShmPutImage			3
#define X_ShmGetImage			4
#define X_ShmCreatePixmap		5

#define ShmCompletion			0
#define ShmNumberEvents			(ShmCompletion + 1)

#define BadShmSeg			0
#define ShmNumberErrors			(BadShmSeg + 1)

typedef unsigned long ShmSeg;

#ifndef _XSHM_SERVER_
typedef struct {
    int	type;		    /* of event */
    unsigned long serial;   /* # of last request processed by server */
    Bool send_event;	    /* true if this came frome a SendEvent request */
    Display *display;	    /* Display the event was read from */
    Drawable drawable;	    /* drawable of request */
    int major_code;	    /* ShmReqCode */
    int minor_code;	    /* X_ShmPutImage */
    ShmSeg shmseg;	    /* the ShmSeg used in the request */
    unsigned long offset;   /* the offset into ShmSeg used in the request */
} XShmCompletionEvent;

typedef struct {
    ShmSeg shmseg;	/* resource id */
    int shmid;		/* kernel id */
    char *shmaddr;	/* address in client */
    Bool readOnly;	/* how the server should attach it */
} XShmSegmentInfo;

_XFUNCPROTOBEGIN

Bool XShmQueryExtension(
#if NeedFunctionPrototypes
    Display*            /* dpy */
#endif
);

Bool XShmQueryVersion(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    int*		/* majorVersion */,
    int*		/* minorVersion */,
    Bool*		/* sharedPixmaps */
#endif
);

int XShmPixmapFormat(
#if NeedFunctionPrototypes
    Display*		/* dpy */
#endif
);

Status XShmAttach(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    XShmSegmentInfo*	/* shminfo */
#endif
);

Status XShmDetach(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    XShmSegmentInfo*	/* shminfo */
#endif
);

Status XShmPutImage(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Drawable		/* d */,
    GC			/* gc */,
    XImage*		/* image */,
    int			/* src_x */,
    int			/* src_y */,
    int			/* dst_x */,
    int			/* dst_y */,
    unsigned int	/* src_width */,
    unsigned int	/* src_height */,
    Bool		/* send_event */
#endif
);

Status XShmGetImage(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Drawable		/* d */,
    XImage*		/* image */,
    int			/* x */,
    int			/* y */,
    unsigned long	/* plane_mask */
#endif
);

XImage *XShmCreateImage(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Visual*		/* visual */,
    unsigned int	/* depth */,
    int			/* format */,
    char*		/* data */,
    XShmSegmentInfo*	/* shminfo */,
    unsigned int	/* width */,
    unsigned int	/* height */
#endif
);

Pixmap XShmCreatePixmap(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Drawable		/* d */,
    char*		/* data */,
    XShmSegmentInfo*	/* shminfo */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    unsigned int	/* depth */
#endif
);

_XFUNCPROTOEND

#endif

#endif
