/* $XFree86: xc/programs/Xserver/GL/glx/glxbuf.c,v 1.6 2001/03/25 05:32:01 tsi Exp $ */
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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "glxserver.h"
#include "glxutil.h"
#include "glxbuf.h"
#include "glxfb.h"
#include "glxmem.h"
#include "glxpix.h"

void
__glXFBInitDrawable(__GLXdrawablePrivate *glxPriv, __GLcontextModes *modes)
{
    __GLdrawablePrivate *glPriv;
    GLint rgbBits;
    GLint accumBits;

    glPriv = &glxPriv->glPriv;
    rgbBits = modes->rgbBits;
    accumBits = modes->accumRedBits + modes->accumGreenBits +
	modes->accumBlueBits + modes->accumAlphaBits;

#if defined(__GL_ALIGNED_BUFFERS)
    /* initialize pixel alignments (for more details see context.h) */
    glPriv->xAlignment = 1;
    glPriv->yAlignment = 1;
#endif

    glxPriv->swapBuffers = __glXFBMemSwapBuffers;

    glPriv->yInverted = GL_TRUE;	/* Y is upside-down */

    if (modes->doubleBufferMode) {
	if (modes->colorIndexMode) {
	    __glXInitFB(&glPriv->frontBuffer, glPriv, modes->indexBits);
	    __glXInitMem(&glPriv->backBuffer, glPriv, modes->indexBits);
	} else {
	    __glXInitFB(&glPriv->frontBuffer, glPriv, rgbBits);
	    __glXInitMem(&glPriv->backBuffer, glPriv, rgbBits);
	}
    } else {
	if (modes->colorIndexMode) {
	    __glXInitFB(&glPriv->frontBuffer, glPriv, modes->indexBits);
	} else {
	    __glXInitFB(&glPriv->frontBuffer, glPriv, rgbBits);
	}
    }

#if defined(__GL_MAX_AUXBUFFERS) && (__GL_MAX_AUXBUFFERS > 0)
    if (modes->maxAuxBuffers > 0) {
	GLint i;

	for (i=0; i < modes->maxAuxBuffers; i++) {
	    if (modes->colorIndexMode) {
		__glXInitMem(&glPriv->auxBuffer[i], glPriv, modes->indexBits);
	    } else {
		__glXInitMem(&glPriv->auxBuffer[i], glPriv, rgbBits);
	    }
	}
    }
#endif

    if (modes->haveAccumBuffer) {
	__glXInitMem(&glPriv->accumBuffer, glPriv, accumBits);
    }
    if (modes->haveDepthBuffer) {
	__glXInitMem(&glPriv->depthBuffer, glPriv, modes->depthBits);
    }
    if (modes->haveStencilBuffer) {
	__glXInitMem(&glPriv->stencilBuffer, glPriv, modes->stencilBits);
    }
}

void
__glXPixInitDrawable(__GLXdrawablePrivate *glxPriv, __GLcontextModes *modes)
{
    __GLdrawablePrivate *glPriv;
    GLint rgbBits;
    GLint accumBits;

    assert(glxPriv->pGlxPixmap);

    glPriv = &glxPriv->glPriv;
    rgbBits = modes->rgbBits;
    accumBits = modes->accumRedBits + modes->accumGreenBits +
	modes->accumBlueBits + modes->accumAlphaBits;

#if defined(__GL_ALIGNED_BUFFERS)
    /* initialize pixel alignments (for more details see context.h) */
    glPriv->xAlignment = 1;
    glPriv->yAlignment = 1;
#endif

    glxPriv->swapBuffers = (GLboolean (*)(__GLXdrawablePrivate *))__glXNop;

    glPriv->yInverted = GL_FALSE;

    if (modes->doubleBufferMode) {
	if (modes->colorIndexMode) {
	    __glXInitPix(&glPriv->frontBuffer, glPriv, rgbBits,
			 glxPriv->drawId, glxPriv->pGlxPixmap);
	    __glXInitMem(&glPriv->backBuffer, glPriv, modes->indexBits);
	} else {
	    __glXInitPix(&glPriv->frontBuffer, glPriv, rgbBits,
			 glxPriv->drawId, glxPriv->pGlxPixmap);
	    __glXInitMem(&glPriv->backBuffer, glPriv, rgbBits);
	}
    } else {
	if (modes->colorIndexMode) {
	    __glXInitPix(&glPriv->frontBuffer, glPriv, rgbBits, 
			 glxPriv->drawId, glxPriv->pGlxPixmap);
	} else {
	    __glXInitPix(&glPriv->frontBuffer, glPriv, rgbBits,
			 glxPriv->drawId, glxPriv->pGlxPixmap);
	}
    }

#if defined(__GL_MAX_AUXBUFFERS) && (__GL_MAX_AUXBUFFERS > 0)
    if (modes->maxAuxBuffers > 0) {
	GLint i;

	for (i=0; i < modes->maxAuxBuffers; i++) {
	    if (modes->colorIndexMode) {
		__glXInitMem(&glPriv->auxBuffer[i], glPriv, modes->indexBits);
	    } else {
		__glXInitMem(&glPriv->auxBuffer[i], glPriv, rgbBits);
	    }
	}
    }
#endif

    if (modes->haveAccumBuffer) {
	__glXInitMem(&glPriv->accumBuffer, glPriv, accumBits);
    }
    if (modes->haveDepthBuffer) {
	__glXInitMem(&glPriv->depthBuffer, glPriv, modes->depthBits);
    }
    if (modes->haveStencilBuffer) {
	__glXInitMem(&glPriv->stencilBuffer, glPriv, modes->stencilBits);
    }
}


#define __GLX_SET_ACCEL_BUFFER_MASK(bm) \
    if (status == GL_FALSE) return GL_FALSE; \
    if (status == GL_TRUE) accelBufferMask |= bm; \
    /* for __GL_BUFFER_FALLBACK don't do anything */

GLboolean
__glXResizeBuffers(__GLdrawablePrivate *glPriv,
		   GLint x, GLint y, GLuint width, GLuint height)
{
    __GLcontextModes *modes;
    __GLdrawableRegion *glRegion;
    GLboolean status;
    GLuint accelBufferMask;

    modes = glPriv->modes;
    accelBufferMask = 0;

    status = (*glPriv->frontBuffer.resize)(&glPriv->frontBuffer,
					   x, y, width, height, glPriv,
					   __GL_FRONT_BUFFER_MASK);
    __GLX_SET_ACCEL_BUFFER_MASK(__GL_FRONT_BUFFER_MASK);

    if (modes->doubleBufferMode) {
	status = (*glPriv->backBuffer.resize)(&glPriv->backBuffer,
					      x, y, width, height, glPriv,
					      __GL_BACK_BUFFER_MASK);
	__GLX_SET_ACCEL_BUFFER_MASK(__GL_BACK_BUFFER_MASK);
    }

#if defined(__GL_MAX_AUXBUFFERS) && (__GL_MAX_AUXBUFFERS > 0)
    if (modes->maxAuxBuffers > 0) {
	GLint i;

	for (i=0; i < modes->maxAuxBuffers; i++) {
	    status = (*glPriv->auxBuffers[i].resize)(&glPriv->auxBuffer[i],
						     x, y, width, height, 
						     glPriv,
						     __GL_AUX_BUFFER_MASK(i));
	    __GLX_SET_ACCEL_BUFFER_MASK(__GL_AUX_BUFFER_MASK(i));
	}
    }
#endif

    if (modes->haveAccumBuffer) {
	status = (*glPriv->accumBuffer.resize)(&glPriv->accumBuffer,
					       x, y, width, height, glPriv,
					       __GL_ACCUM_BUFFER_MASK);
	__GLX_SET_ACCEL_BUFFER_MASK(__GL_ACCUM_BUFFER_MASK);
    }

    if (modes->haveDepthBuffer) {
	status = (*glPriv->depthBuffer.resize)(&glPriv->depthBuffer,
					       x, y, width, height, glPriv,
					       __GL_DEPTH_BUFFER_MASK);
	__GLX_SET_ACCEL_BUFFER_MASK(__GL_DEPTH_BUFFER_MASK);
    }

    if (modes->haveStencilBuffer) {
	status = (*glPriv->stencilBuffer.resize)(&glPriv->stencilBuffer,
						 x, y, width, height, glPriv,
						 __GL_STENCIL_BUFFER_MASK);
	__GLX_SET_ACCEL_BUFFER_MASK(__GL_STENCIL_BUFFER_MASK);
    }

    glPriv->accelBufferMask = accelBufferMask;

    /* finally, update the ownership region */
    glRegion = &glPriv->ownershipRegion;
    glRegion->numRects = 1;
    glRegion->rects[0].x0 = 0;
    glRegion->rects[0].y0 = 0;
    glRegion->rects[0].x1 = width;
    glRegion->rects[0].y1 = height;

    return GL_TRUE;
}

void
__glXFreeBuffers(__GLXdrawablePrivate *glxPriv)
{
    __GLdrawablePrivate *glPriv = &glxPriv->glPriv;
#if defined(__GL_MAX_AUXBUFFERS) && (__GL_MAX_AUXBUFFERS > 0)
    __GLcontextModes *modes = glPriv->modes;
#endif

    if (glPriv->frontBuffer.free) {
	(*glPriv->frontBuffer.free)(&glPriv->frontBuffer, glPriv);
    }
    if (glPriv->backBuffer.free) {
	(*glPriv->backBuffer.free)(&glPriv->backBuffer, glPriv);
    }

#if defined(__GL_MAX_AUXBUFFERS) && (__GL_MAX_AUXBUFFERS > 0)
    if (modes->maxAuxBuffers > 0) {
	GLint i;

	for (i=0; i < modes->maxAuxBuffers; i++) {
	    if (glPriv->auxBuffer[i].free) {
		(*glPriv->auxBuffer[i].free)(&glPriv->auxBuffer[i], glPriv);
	    }
	}
    }
#endif

    if (glPriv->accumBuffer.free) {
	(*glPriv->accumBuffer.free)(&glPriv->accumBuffer, glPriv);
    }

    if (glPriv->depthBuffer.free) {
	(*glPriv->depthBuffer.free)(&glPriv->depthBuffer, glPriv);
    }

    if (glPriv->stencilBuffer.free) {
	(*glPriv->stencilBuffer.free)(&glPriv->stencilBuffer, glPriv);
    }
}
