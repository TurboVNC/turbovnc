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

#define NEED_REPLIES
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "glxserver.h"
#include "unpack.h"
#include "g_disptab.h"
#include "g_disptab_EXT.h"

void __glXDisp_PolygonStipple(GLbyte *pc)
{
    __GLXpixelHeader *hdr = (__GLXpixelHeader *) pc;

    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) hdr->rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, (GLint) hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, (GLint) hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) hdr->alignment);

    glPolygonStipple((GLubyte *)(hdr+1));
}

void __glXDisp_Bitmap(GLbyte *pc)
{
    __GLXdispatchBitmapHeader *hdr = (__GLXdispatchBitmapHeader *) pc;

    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) hdr->rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, (GLint) hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, (GLint) hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) hdr->alignment);

    glBitmap((GLsizei) hdr->width,
	     (GLsizei) hdr->height,
	     (GLfloat) hdr->xorig,
	     (GLfloat) hdr->yorig,
	     (GLfloat) hdr->xmove,
	     (GLfloat) hdr->ymove,
	     (GLubyte *)(hdr+1));
}

void __glXDisp_TexImage1D(GLbyte *pc)
{
    __GLXdispatchTexImageHeader *hdr = (__GLXdispatchTexImageHeader *) pc;

    glPixelStorei(GL_UNPACK_SWAP_BYTES, hdr->swapBytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) hdr->rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, (GLint) hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, (GLint) hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) hdr->alignment);

    glTexImage1D(hdr->target,
		 (GLint) hdr->level,
		 (GLint) hdr->components,
		 (GLsizei) hdr->width,
		 (GLint) hdr->border,
		 hdr->format,
		 hdr->type,
		 (GLvoid *)(hdr+1));
}

void __glXDisp_TexImage2D(GLbyte *pc)
{
    __GLXdispatchTexImageHeader *hdr = (__GLXdispatchTexImageHeader *) pc;

    glPixelStorei(GL_UNPACK_SWAP_BYTES, hdr->swapBytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) hdr->rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, (GLint) hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, (GLint) hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) hdr->alignment);

    glTexImage2D(hdr->target,
		 (GLint) hdr->level,
		 (GLint) hdr->components,
		 (GLsizei) hdr->width,
		 (GLsizei) hdr->height,
		 (GLint) hdr->border,
		 hdr->format,
		 hdr->type,
		 (GLvoid *)(hdr+1));
}

void __glXDisp_TexImage3D(GLbyte *pc)
{
    __GLXdispatchTexImage3DHeader *hdr = (__GLXdispatchTexImage3DHeader *) pc;

    glPixelStorei(GL_UNPACK_SWAP_BYTES, hdr->swapBytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, hdr->rowLength);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, hdr->imageHeight);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_IMAGES, hdr->skipImages);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, hdr->alignment);

    glTexImage3D(hdr->target, hdr->level, hdr->internalformat, hdr->width,
		 hdr->height, hdr->depth, hdr->border, hdr->format, hdr->type,
		 (GLvoid *)(hdr+1));
}

void __glXDisp_DrawPixels(GLbyte *pc)
{
    __GLXdispatchDrawPixelsHeader *hdr = (__GLXdispatchDrawPixelsHeader *) pc;

    glPixelStorei(GL_UNPACK_SWAP_BYTES, hdr->swapBytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) hdr->rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, (GLint) hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, (GLint) hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) hdr->alignment);

    glDrawPixels((GLsizei) hdr->width,
		 (GLsizei) hdr->height,
		 hdr->format,
		 hdr->type,
		 (GLvoid *)(hdr+1));
}

void __glXDisp_TexSubImage1D(GLbyte *pc)
{
    __GLXdispatchTexSubImageHeader *hdr = (__GLXdispatchTexSubImageHeader *) pc;

    glPixelStorei(GL_UNPACK_SWAP_BYTES, hdr->swapBytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) hdr->rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, (GLint) hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, (GLint) hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) hdr->alignment);

    glTexSubImage1D(hdr->target,
		    (GLint) hdr->level,
		    (GLint) hdr->xoffset,
		    (GLsizei) hdr->width,
		    hdr->format,
		    hdr->type,
		    (GLvoid *)(hdr+1));
}

void __glXDisp_TexSubImage2D(GLbyte *pc)
{
    __GLXdispatchTexSubImageHeader *hdr = (__GLXdispatchTexSubImageHeader *) pc;

    glPixelStorei(GL_UNPACK_SWAP_BYTES, hdr->swapBytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) hdr->rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, (GLint) hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, (GLint) hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) hdr->alignment);

    glTexSubImage2D(hdr->target,
		    (GLint) hdr->level,
		    (GLint) hdr->xoffset,
		    (GLint) hdr->yoffset,
		    (GLsizei) hdr->width,
		    (GLsizei) hdr->height,
		    hdr->format,
		    hdr->type,
		    (GLvoid *)(hdr+1));
}

void __glXDisp_TexSubImage3D(GLbyte *pc)
{
    __GLXdispatchTexSubImage3DHeader *hdr =
				(__GLXdispatchTexSubImage3DHeader *) pc;

    glPixelStorei(GL_UNPACK_SWAP_BYTES, hdr->swapBytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, hdr->rowLength);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, hdr->imageHeight);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_IMAGES, hdr->skipImages);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, hdr->alignment);

    glTexSubImage3D(hdr->target, hdr->level, hdr->xoffset, hdr->yoffset,
		       hdr->zoffset, hdr->width, hdr->height, hdr->depth,
		       hdr->format, hdr->type, (GLvoid *)(hdr+1));
}

void __glXDisp_ColorTable(GLbyte *pc)
{
   __GLXdispatchColorTableHeader *hdr =
				(__GLXdispatchColorTableHeader *) pc;

    glPixelStorei(GL_UNPACK_SWAP_BYTES, hdr->swapBytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, hdr->rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, hdr->alignment);

    glColorTable(hdr->target, hdr->internalformat,
		 hdr->width, hdr->format, hdr->type,
		 (GLvoid *)(hdr+1));
}

void __glXDisp_ColorSubTable(GLbyte *pc)
{
   __GLXdispatchColorSubTableHeader *hdr =
				(__GLXdispatchColorSubTableHeader *) pc;

    glPixelStorei(GL_UNPACK_SWAP_BYTES, hdr->swapBytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, hdr->rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, hdr->alignment);

    glColorSubTable(hdr->target, hdr->start, hdr->count, hdr->format,
		    hdr->type, (GLvoid *)(hdr+1));
}

void __glXDisp_ConvolutionFilter1D(GLbyte *pc)
{
   __GLXdispatchConvolutionFilterHeader *hdr =
				(__GLXdispatchConvolutionFilterHeader *) pc;

    glPixelStorei(GL_UNPACK_SWAP_BYTES, hdr->swapBytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, hdr->rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, hdr->alignment);

    glConvolutionFilter1D(hdr->target, hdr->internalformat,
		 hdr->width, hdr->format, hdr->type,
		 (GLvoid *)(hdr+1));
}

void __glXDisp_ConvolutionFilter2D(GLbyte *pc)
{
   __GLXdispatchConvolutionFilterHeader *hdr =
				 (__GLXdispatchConvolutionFilterHeader *) pc;

    glPixelStorei(GL_UNPACK_SWAP_BYTES, hdr->swapBytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, hdr->rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, hdr->alignment);

    glConvolutionFilter2D(hdr->target, hdr->internalformat,
		 hdr->width, hdr->height, hdr->format, hdr->type,
		 (GLvoid *)(hdr+1));
}

void __glXDisp_SeparableFilter2D(GLbyte *pc)
{
   __GLXdispatchConvolutionFilterHeader *hdr =
				(__GLXdispatchConvolutionFilterHeader *) pc;
    GLint hdrlen, image1len;

    hdrlen = __GLX_PAD(__GLX_CONV_FILT_CMD_DISPATCH_HDR_SIZE);

    glPixelStorei(GL_UNPACK_SWAP_BYTES, hdr->swapBytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, hdr->rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, hdr->alignment);

    /* XXX check this usage - internal code called
    ** a version without the packing parameters
    */
    image1len = __glXImageSize(hdr->format, hdr->type, 0, hdr->width, 1, 1,
			       0, hdr->rowLength, 0, hdr->skipRows,
			       hdr->alignment);
    image1len = __GLX_PAD(image1len);

    glSeparableFilter2D(hdr->target, hdr->internalformat,
		 hdr->width, hdr->height, hdr->format, hdr->type,
		 ((GLubyte *)hdr+hdrlen), ((GLubyte *)hdr+hdrlen+image1len));
}
