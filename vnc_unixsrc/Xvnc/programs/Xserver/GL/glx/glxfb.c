/* $XFree86$ */
/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
** 
** http://oss.sgi.com/projects/FreeB
** 
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
** 
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
** 
** Additional Notice Provisions: The application programming interfaces
** established by SGI in conjunction with the Original Code are The
** OpenGL(R) Graphics System: A Specification (Version 1.2.1), released
** April 1, 1999; The OpenGL(R) Graphics System Utility Library (Version
** 1.3), released November 4, 1998; and OpenGL(R) Graphics with the X
** Window System(R) (Version 1.3), released October 19, 1998. This software
** was created using the OpenGL(R) version 1.2.1 Sample Implementation
** published by SGI, but has not been independently verified as being
** compliant with the OpenGL(R) version 1.2.1 Specification.
**
*/

/*
** An implementation of a buffer which is part of the front buffer
*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "glxserver.h"
#include "glxutil.h"
#include "glxfb.h"

#include <gcstruct.h>

/* so we don't include glmath.h */
extern GLuint __glFloorLog2(GLuint);

typedef struct __GLFBbufferInfoRec {
    GCPtr	pGC;
} __GLFBbufferInfo;

extern PixmapPtr __glXPrivPixGetPtr(__GLdrawableBuffer *);

/* ---------------------------------------------------------- */

static GLboolean
Resize(__GLdrawableBuffer *buf, 
       GLint x, GLint y, GLuint width, GLuint height,
       __GLdrawablePrivate *glPriv, GLuint bufferMask)
{
    buf->width = width;
    buf->height = height;
    buf->byteWidth = width * buf->elementSize;
    buf->outerWidth = width;

    return GL_TRUE;
}

static void
Lock(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
}

static void
Unlock(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
}

/*
** Do a swap buffer with
** a memory surface as a back buffer
** a FB surface as a front buffer
*/
GLboolean
__glXFBMemSwapBuffers(__GLXdrawablePrivate *glxPriv)
{
    __GLdrawablePrivate *glPriv = &glxPriv->glPriv;
    __GLdrawableBuffer *front = &glPriv->frontBuffer;
    __GLdrawableBuffer *back = &glPriv->backBuffer;
    __GLFBbufferInfo *bufferInfo;
    GCPtr pGC;
    GLint width, height, depth, pad;
    GLubyte *buf;

    bufferInfo = (__GLFBbufferInfo *) front->other;
    pGC = bufferInfo->pGC;

    width = back->width;
    height = back->height;
    depth = back->depth;
    buf = back->base;
    pad = back->outerWidth - back->width;	/* back buffer padding */
    /* adjust buffer padding. X wants left, GL has right */
    buf -= pad;

    ValidateGC(glxPriv->pDraw, pGC);
    (*pGC->ops->PutImage)(glxPriv->pDraw, pGC,
			 depth,
			 0, 0, width, height,
			 pad, ZPixmap,
			 (char *)buf);

    return GL_TRUE;
}

static void
Free(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    __GLFBbufferInfo *bufferInfo;

    bufferInfo = (__GLFBbufferInfo *) buf->other;

    if (bufferInfo->pGC) {
	FreeScratchGC(bufferInfo->pGC);
    }

    __glXFree(bufferInfo);
    buf->other = NULL;
}

/*
** function to return the X GC of this buffer (to be used by DDX)
*/
GCPtr __glXFBGetGC(__GLdrawableBuffer *buf)
{
    __GLFBbufferInfo *bufferInfo;

    bufferInfo = (__GLFBbufferInfo *) buf->other;

    if (bufferInfo) {
	return bufferInfo->pGC;
    } else {
	return NULL;
    }
}


void
__glXInitFB(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLint bits)
{
    __GLFBbufferInfo *bufferInfo;
    __GLXdrawablePrivate *glxPriv = (__GLXdrawablePrivate *) glPriv->other;
    GCPtr pGC;

    buf->depth = bits;
    buf->width = buf->height = 0;	/* to be filled during Update */
    buf->handle = buf->base = NULL;	/* to be filled during Update */
    buf->size = 0;
    buf->byteWidth = 0;
    buf->elementSize = ((bits-1) / 8) + 1;
    buf->elementSizeLog2 = __glFloorLog2(buf->elementSize);

    buf->resize = Resize;
    buf->lock = Lock;
    buf->unlock = Unlock;
    buf->fill = NULL;
    buf->free = Free;

    /* allocate local information */
    bufferInfo = (__GLFBbufferInfo *) __glXMalloc(sizeof(__GLFBbufferInfo));
    buf->other = (void *) bufferInfo;

    pGC = CreateScratchGC(glxPriv->pDraw->pScreen,
			  glxPriv->pDraw->depth);
    bufferInfo->pGC = pGC;
    (*pGC->funcs->ChangeClip)(pGC, CT_NONE, NULL, 0);
}
