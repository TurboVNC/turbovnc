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

#ifndef _SHMSTR_H_
#define _SHMSTR_H_

/* $XConsortium: shmstr.h,v 1.9 94/04/17 20:11:25 rws Exp $ */
/* $XFree86: xc/include/extensions/shmstr.h,v 3.0 1996/05/06 05:52:42 dawes Exp $ */

#include "XShm.h"

#define ShmSeg CARD32
#define Drawable CARD32
#define VisualID CARD32
#define GContext CARD32
#define Pixmap CARD32

#define SHMNAME "MIT-SHM"

#define SHM_MAJOR_VERSION	1	/* current version numbers */
#define SHM_MINOR_VERSION	1

#ifdef _XSHM_SERVER_
#if NeedFunctionPrototypes
#define XSHM_PUT_IMAGE_ARGS \
    DrawablePtr		/* dst */, \
    GCPtr		/* pGC */, \
    int			/* depth */, \
    unsigned int	/* format */, \
    int			/* w */, \
    int			/* h */, \
    int			/* sx */, \
    int			/* sy */, \
    int			/* sw */, \
    int			/* sh */, \
    int			/* dx */, \
    int			/* dy */, \
    char *		/* data */
#else
#define XSHM_PUT_IMAGE_ARGS /* nothing */
#endif

#if NeedFunctionPrototypes
#define XSHM_CREATE_PIXMAP_ARGS \
    ScreenPtr	/* pScreen */, \
    int		/* width */, \
    int		/* height */, \
    int		/* depth */, \
    char *	/* addr */
#else
#define XSHM_CREATE_PIXMAP_ARGS /* nothing */
#endif

typedef struct _ShmFuncs {
    PixmapPtr	(* CreatePixmap)(XSHM_CREATE_PIXMAP_ARGS);
    void	(* PutImage)(XSHM_PUT_IMAGE_ARGS);
} ShmFuncs, *ShmFuncsPtr;
#endif

typedef struct _ShmQueryVersion {
    CARD8	reqType;		/* always ShmReqCode */
    CARD8	shmReqType;		/* always X_ShmQueryVersion */
    CARD16	length B16;
} xShmQueryVersionReq;
#define sz_xShmQueryVersionReq	4

typedef struct {
    BYTE	type;			/* X_Reply */
    BOOL	sharedPixmaps;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD16	majorVersion B16;	/* major version of SHM protocol */
    CARD16	minorVersion B16;	/* minor version of SHM protocol */
    CARD16	uid B16;
    CARD16	gid B16;
    CARD8	pixmapFormat;
    CARD8	pad0;
    CARD16	pad1 B16;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
} xShmQueryVersionReply;
#define sz_xShmQueryVersionReply	32

typedef struct _ShmAttach {
    CARD8	reqType;	/* always ShmReqCode */
    CARD8	shmReqType;	/* always X_ShmAttach */
    CARD16	length B16;
    ShmSeg	shmseg B32;
    CARD32	shmid B32;
    BOOL	readOnly;
    BYTE	pad0;
    CARD16	pad1 B16;
} xShmAttachReq;
#define sz_xShmAttachReq	16

typedef struct _ShmDetach {
    CARD8	reqType;	/* always ShmReqCode */
    CARD8	shmReqType;	/* always X_ShmDetach */
    CARD16	length B16;
    ShmSeg	shmseg B32;
} xShmDetachReq;
#define sz_xShmDetachReq	8

typedef struct _ShmPutImage {
    CARD8	reqType;	/* always ShmReqCode */
    CARD8	shmReqType;	/* always X_ShmPutImage */
    CARD16	length B16;
    Drawable	drawable B32;
    GContext	gc B32;
    CARD16	totalWidth B16;
    CARD16	totalHeight B16;
    CARD16	srcX B16;
    CARD16	srcY B16;
    CARD16	srcWidth B16;
    CARD16	srcHeight B16;
    INT16	dstX B16;
    INT16	dstY B16;
    CARD8	depth;
    CARD8	format;
    CARD8	sendEvent;
    CARD8	bpad;
    ShmSeg	shmseg B32;
    CARD32	offset B32;
} xShmPutImageReq;    
#define sz_xShmPutImageReq	40

typedef struct _ShmGetImage {
    CARD8	reqType;	/* always ShmReqCode */
    CARD8	shmReqType;	/* always X_ShmGetImage */
    CARD16	length B16;
    Drawable	drawable B32;
    INT16	x B16;
    INT16	y B16;
    CARD16	width B16;
    CARD16	height B16;
    CARD32	planeMask B32;
    CARD8	format;
    CARD8	pad0;
    CARD8	pad1;
    CARD8	pad2;
    ShmSeg	shmseg B32;
    CARD32	offset B32;
} xShmGetImageReq;    
#define sz_xShmGetImageReq	32

typedef struct _ShmGetImageReply {
    BYTE	type;  /* X_Reply */
    CARD8	depth;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    VisualID	visual B32;
    CARD32	size B32;
    CARD32	pad0 B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
} xShmGetImageReply;
#define sz_xShmGetImageReply	32

typedef struct _ShmCreatePixmap {
    CARD8	reqType;	/* always ShmReqCode */
    CARD8	shmReqType;	/* always X_ShmCreatePixmap */
    CARD16	length B16;
    Pixmap	pid B32;
    Drawable	drawable B32;
    CARD16	width B16;
    CARD16	height B16;
    CARD8	depth;
    CARD8	pad0;
    CARD8	pad1;
    CARD8	pad2;
    ShmSeg	shmseg B32;
    CARD32	offset B32;
} xShmCreatePixmapReq;
#define sz_xShmCreatePixmapReq 28

typedef struct _ShmCompletion {
    BYTE	type;		/* always eventBase + ShmCompletion */
    BYTE	bpad0;
    CARD16	sequenceNumber B16;
    Drawable	drawable B32;
    CARD16	minorEvent B16;
    BYTE	majorEvent;
    BYTE	bpad1;
    ShmSeg	shmseg B32;
    CARD32	offset B32;
    CARD32	pad0 B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
} xShmCompletionEvent;
#define sz_xShmCompletionEvent	32

#undef ShmSeg
#undef Drawable
#undef VisualID
#undef GContext
#undef Pixmap

#endif /* _SHMSTR_H_ */
