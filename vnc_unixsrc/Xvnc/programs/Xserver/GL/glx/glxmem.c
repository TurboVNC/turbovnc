/* $XFree86: xc/programs/Xserver/GL/glx/glxmem.c,v 1.6 2001/10/31 22:50:27 tsi Exp $ */
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
** Implementation of a buffer in main memory
*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "glxserver.h"
#include "glxmem.h"
#include "glxext.h"
#include "GL/internal/glcore.h"

/* don't want to include glmath.h */
extern GLuint __glFloorLog2(GLuint);

/* ---------------------------------------------------------- */

#define	BUF_ALIGN 32	/* x86 cache alignment (used for assembly paths) */
#define	BUF_ALIGN_MASK	(BUF_ALIGN-1)

static GLboolean
Resize(__GLdrawableBuffer *buf, 
       GLint x, GLint y, GLuint width, GLuint height,
       __GLdrawablePrivate *glPriv, GLuint bufferMask)
{
    GLuint newSize;
    void *ubase;
    GLint pixelWidth;
    GLint alignedWidth;

    /*
    ** Note: 
    **	buf->handle : unaligned base
    **  buf->base   : aligned base
    */

    pixelWidth = BUF_ALIGN / buf->elementSize;
    alignedWidth = (width & ~(pixelWidth-1)) + pixelWidth;

    newSize = alignedWidth * height * buf->elementSize;

    /*
    ** Only allocate buffer space for the SGI core.
    ** Mesa and Aqua handle their own buffer allocations.
    */
#if defined(__GL_BUFFER_SIZE_TRACKS_WINDOW)
    if (__glXCoreType() == GL_CORE_SGI) {
#else
    if (newSize > buf->size && __glXCoreType() == GL_CORE_SGI) {
#endif
	if (buf->handle) {
	    ubase = (*glPriv->realloc)(buf->handle, newSize + BUF_ALIGN_MASK);
	    if (ubase == NULL) {
		return GL_FALSE;
	    }
	} else {
	    ubase = (*glPriv->malloc)(newSize + BUF_ALIGN_MASK);
	    if (ubase == NULL) {
		return GL_FALSE;
	    }
	}
	buf->size = newSize;

	buf->handle = ubase;
	buf->base = (void *)(((size_t)ubase + BUF_ALIGN_MASK) &
			     (unsigned int) ~BUF_ALIGN_MASK);
	assert(((size_t)buf->base % BUF_ALIGN) == 0);
    }

    buf->width = width;
    buf->height = height;
    buf->byteWidth = alignedWidth * buf->elementSize;
    buf->outerWidth = alignedWidth;

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

static void
Free(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    if (buf->handle) {
	(*glPriv->free)(buf->handle);
	buf->handle = NULL;
    }
}


void
__glXInitMem(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLint bits)
{
    buf->width = buf->height = 0;	/* to be filled during Update */
    buf->depth = bits;
    buf->size = 0;
    buf->handle = buf->base = NULL;	/* to be filled during Update */
    buf->byteWidth = 0;
    buf->elementSize = ((bits - 1) / 8) + 1;
    buf->elementSizeLog2 = __glFloorLog2(buf->elementSize);

    buf->resize = Resize;
    buf->lock = Lock;
    buf->unlock = Unlock;
    buf->fill = NULL;
    buf->free = Free;
}
