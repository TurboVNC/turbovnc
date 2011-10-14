/* $XFree86: xc/programs/Xserver/GL/glx/g_renderswap.c,v 1.8 2004/01/28 18:11:50 alanh Exp $ */
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
** Additional Notice Provisions: This software was created using the
** OpenGL(R) version 1.2.1 Sample Implementation published by SGI, but has
** not been independently verified as being compliant with the OpenGL(R)
** version 1.2.1 Specification.
*/

#define NEED_REPLIES
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "glxserver.h"
#include "glxext.h"
#include "g_disptab.h"
#include "g_disptab_EXT.h"
#include "unpack.h"
#include "impsize.h"
#include "singlesize.h"

void __glXDispSwap_CallList(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glCallList( 
		*(GLuint   *)(pc + 0)
	);
}

void __glXDispSwap_ListBase(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glListBase( 
		*(GLuint   *)(pc + 0)
	);
}

void __glXDispSwap_Begin(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glBegin( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDispSwap_Color3bv(GLbyte *pc)
{
	glColor3bv( 
		(GLbyte   *)(pc + 0)
	);
}

void __glXDispSwap_Color3dv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 24);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 3);

	glColor3dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_Color3fv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_FLOAT_ARRAY(pc + 0, 3);

	glColor3fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_Color3iv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT_ARRAY(pc + 0, 3);

	glColor3iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDispSwap_Color3sv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_SHORT_ARRAY(pc + 0, 3);

	glColor3sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDispSwap_Color3ubv(GLbyte *pc)
{
	glColor3ubv( 
		(GLubyte  *)(pc + 0)
	);
}

void __glXDispSwap_Color3uiv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT_ARRAY(pc + 0, 3);

	glColor3uiv( 
		(GLuint   *)(pc + 0)
	);
}

void __glXDispSwap_Color3usv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_SHORT_ARRAY(pc + 0, 3);

	glColor3usv( 
		(GLushort *)(pc + 0)
	);
}

void __glXDispSwap_Color4bv(GLbyte *pc)
{
	glColor4bv( 
		(GLbyte   *)(pc + 0)
	);
}

void __glXDispSwap_Color4dv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;


#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 32);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 4);

	glColor4dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_Color4fv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_FLOAT_ARRAY(pc + 0, 4);

	glColor4fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_Color4iv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT_ARRAY(pc + 0, 4);

	glColor4iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDispSwap_Color4sv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_SHORT_ARRAY(pc + 0, 4);

	glColor4sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDispSwap_Color4ubv(GLbyte *pc)
{
	glColor4ubv( 
		(GLubyte  *)(pc + 0)
	);
}

void __glXDispSwap_Color4uiv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT_ARRAY(pc + 0, 4);

	glColor4uiv( 
		(GLuint   *)(pc + 0)
	);
}

void __glXDispSwap_Color4usv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_SHORT_ARRAY(pc + 0, 4);

	glColor4usv( 
		(GLushort *)(pc + 0)
	);
}

void __glXDispSwap_EdgeFlagv(GLbyte *pc)
{
	glEdgeFlagv( 
		(GLboolean *)(pc + 0)
	);
}

void __glXDispSwap_End(GLbyte *pc)
{
	glEnd( 
	);
}

void __glXDispSwap_Indexdv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 8);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 1);

	glIndexdv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_Indexfv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_FLOAT_ARRAY(pc + 0, 1);

	glIndexfv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_Indexiv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT_ARRAY(pc + 0, 1);

	glIndexiv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDispSwap_Indexsv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_SHORT_ARRAY(pc + 0, 1);

	glIndexsv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDispSwap_Normal3bv(GLbyte *pc)
{
	glNormal3bv( 
		(GLbyte   *)(pc + 0)
	);
}

void __glXDispSwap_Normal3dv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 24);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 3);

	glNormal3dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_Normal3fv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_FLOAT_ARRAY(pc + 0, 3);

	glNormal3fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_Normal3iv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT_ARRAY(pc + 0, 3);

	glNormal3iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDispSwap_Normal3sv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_SHORT_ARRAY(pc + 0, 3);

	glNormal3sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDispSwap_RasterPos2dv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 16);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 2);

	glRasterPos2dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_RasterPos2fv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_FLOAT_ARRAY(pc + 0, 2);

	glRasterPos2fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_RasterPos2iv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT_ARRAY(pc + 0, 2);

	glRasterPos2iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDispSwap_RasterPos2sv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_SHORT_ARRAY(pc + 0, 2);

	glRasterPos2sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDispSwap_RasterPos3dv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 24);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 3);

	glRasterPos3dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_RasterPos3fv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_FLOAT_ARRAY(pc + 0, 3);

	glRasterPos3fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_RasterPos3iv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT_ARRAY(pc + 0, 3);

	glRasterPos3iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDispSwap_RasterPos3sv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_SHORT_ARRAY(pc + 0, 3);

	glRasterPos3sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDispSwap_RasterPos4dv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 32);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 4);

	glRasterPos4dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_RasterPos4fv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_FLOAT_ARRAY(pc + 0, 4);

	glRasterPos4fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_RasterPos4iv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT_ARRAY(pc + 0, 4);

	glRasterPos4iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDispSwap_RasterPos4sv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_SHORT_ARRAY(pc + 0, 4);

	glRasterPos4sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDispSwap_Rectdv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 32);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 2);
	__GLX_SWAP_DOUBLE_ARRAY(pc + 16, 2);

	glRectdv( 
		(GLdouble *)(pc + 0),
		(GLdouble *)(pc + 16)
	);
}

void __glXDispSwap_Rectfv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_FLOAT_ARRAY(pc + 0, 2);
	__GLX_SWAP_FLOAT_ARRAY(pc + 8, 2);

	glRectfv( 
		(GLfloat  *)(pc + 0),
		(GLfloat  *)(pc + 8)
	);
}

void __glXDispSwap_Rectiv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT_ARRAY(pc + 0, 2);
	__GLX_SWAP_INT_ARRAY(pc + 8, 2);

	glRectiv( 
		(GLint    *)(pc + 0),
		(GLint    *)(pc + 8)
	);
}

void __glXDispSwap_Rectsv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_SHORT_ARRAY(pc + 0, 2);
	__GLX_SWAP_SHORT_ARRAY(pc + 4, 2);

	glRectsv( 
		(GLshort  *)(pc + 0),
		(GLshort  *)(pc + 4)
	);
}

void __glXDispSwap_TexCoord1dv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 8);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 1);

	glTexCoord1dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_TexCoord1fv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_FLOAT_ARRAY(pc + 0, 1);

	glTexCoord1fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_TexCoord1iv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT_ARRAY(pc + 0, 1);

	glTexCoord1iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDispSwap_TexCoord1sv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_SHORT_ARRAY(pc + 0, 1);

	glTexCoord1sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDispSwap_TexCoord2dv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 16);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 2);

	glTexCoord2dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_TexCoord2fv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_FLOAT_ARRAY(pc + 0, 2);

	glTexCoord2fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_TexCoord2iv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT_ARRAY(pc + 0, 2);

	glTexCoord2iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDispSwap_TexCoord2sv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_SHORT_ARRAY(pc + 0, 2);

	glTexCoord2sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDispSwap_TexCoord3dv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 24);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 3);

	glTexCoord3dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_TexCoord3fv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_FLOAT_ARRAY(pc + 0, 3);

	glTexCoord3fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_TexCoord3iv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT_ARRAY(pc + 0, 3);

	glTexCoord3iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDispSwap_TexCoord3sv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_SHORT_ARRAY(pc + 0, 3);

	glTexCoord3sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDispSwap_TexCoord4dv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 32);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 4);

	glTexCoord4dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_TexCoord4fv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_FLOAT_ARRAY(pc + 0, 4);

	glTexCoord4fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_TexCoord4iv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT_ARRAY(pc + 0, 4);

	glTexCoord4iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDispSwap_TexCoord4sv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_SHORT_ARRAY(pc + 0, 4);

	glTexCoord4sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDispSwap_Vertex2dv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 16);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 2);

	glVertex2dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_Vertex2fv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_FLOAT_ARRAY(pc + 0, 2);

	glVertex2fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_Vertex2iv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT_ARRAY(pc + 0, 2);

	glVertex2iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDispSwap_Vertex2sv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_SHORT_ARRAY(pc + 0, 2);

	glVertex2sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDispSwap_Vertex3dv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 24);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 3);

	glVertex3dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_Vertex3fv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_FLOAT_ARRAY(pc + 0, 3);

	glVertex3fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_Vertex3iv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT_ARRAY(pc + 0, 3);

	glVertex3iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDispSwap_Vertex3sv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_SHORT_ARRAY(pc + 0, 3);

	glVertex3sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDispSwap_Vertex4dv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 32);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 4);

	glVertex4dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_Vertex4fv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_FLOAT_ARRAY(pc + 0, 4);

	glVertex4fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_Vertex4iv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT_ARRAY(pc + 0, 4);

	glVertex4iv( 
		(GLint    *)(pc + 0)
	);
}

void __glXDispSwap_Vertex4sv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_SHORT_ARRAY(pc + 0, 4);

	glVertex4sv( 
		(GLshort  *)(pc + 0)
	);
}

void __glXDispSwap_ClipPlane(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 36);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_INT(pc + 32);
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 4);

	glClipPlane( 
		*(GLenum   *)(pc + 32),
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_ColorMaterial(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);

	glColorMaterial( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4)
	);
}

void __glXDispSwap_CullFace(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glCullFace( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDispSwap_Fogf(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT(pc + 4);

	glFogf( 
		*(GLenum   *)(pc + 0),
		*(GLfloat  *)(pc + 4)
	);
}

void __glXDispSwap_Fogfv(GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	pname = *(GLenum *)(pc + 0);
	compsize = __glFogfv_size(pname);
	if (compsize < 0) compsize = 0;
	__GLX_SWAP_FLOAT_ARRAY(pc + 4, compsize);

	glFogfv( 
		*(GLenum   *)(pc + 0),
		(GLfloat  *)(pc + 4)
	);
}

void __glXDispSwap_Fogi(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);

	glFogi( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4)
	);
}

void __glXDispSwap_Fogiv(GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	pname = *(GLenum *)(pc + 0);
	compsize = __glFogiv_size(pname);
	if (compsize < 0) compsize = 0;
	__GLX_SWAP_INT_ARRAY(pc + 4, compsize);

	glFogiv( 
		*(GLenum   *)(pc + 0),
		(GLint    *)(pc + 4)
	);
}

void __glXDispSwap_FrontFace(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glFrontFace( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDispSwap_Hint(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);

	glHint( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4)
	);
}

void __glXDispSwap_Lightf(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_FLOAT(pc + 8);

	glLightf( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLfloat  *)(pc + 8)
	);
}

void __glXDispSwap_Lightfv(GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 4);
	pname = *(GLenum *)(pc + 4);
	compsize = __glLightfv_size(pname);
	if (compsize < 0) compsize = 0;
	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT_ARRAY(pc + 8, compsize);

	glLightfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *)(pc + 8)
	);
}

void __glXDispSwap_Lighti(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);

	glLighti( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLint    *)(pc + 8)
	);
}

void __glXDispSwap_Lightiv(GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 4);
	pname = *(GLenum *)(pc + 4);
	compsize = __glLightiv_size(pname);
	if (compsize < 0) compsize = 0;
	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT_ARRAY(pc + 8, compsize);

	glLightiv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *)(pc + 8)
	);
}

void __glXDispSwap_LightModelf(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT(pc + 4);

	glLightModelf( 
		*(GLenum   *)(pc + 0),
		*(GLfloat  *)(pc + 4)
	);
}

void __glXDispSwap_LightModelfv(GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	pname = *(GLenum *)(pc + 0);
	compsize = __glLightModelfv_size(pname);
	if (compsize < 0) compsize = 0;
	__GLX_SWAP_FLOAT_ARRAY(pc + 4, compsize);

	glLightModelfv( 
		*(GLenum   *)(pc + 0),
		(GLfloat  *)(pc + 4)
	);
}

void __glXDispSwap_LightModeli(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);

	glLightModeli( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4)
	);
}

void __glXDispSwap_LightModeliv(GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	pname = *(GLenum *)(pc + 0);
	compsize = __glLightModeliv_size(pname);
	if (compsize < 0) compsize = 0;
	__GLX_SWAP_INT_ARRAY(pc + 4, compsize);

	glLightModeliv( 
		*(GLenum   *)(pc + 0),
		(GLint    *)(pc + 4)
	);
}

void __glXDispSwap_LineStipple(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_SHORT(pc + 4);

	glLineStipple( 
		*(GLint    *)(pc + 0),
		*(GLushort *)(pc + 4)
	);
}

void __glXDispSwap_LineWidth(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_FLOAT(pc + 0);

	glLineWidth( 
		*(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_Materialf(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_FLOAT(pc + 8);

	glMaterialf( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLfloat  *)(pc + 8)
	);
}

void __glXDispSwap_Materialfv(GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 4);
	pname = *(GLenum *)(pc + 4);
	compsize = __glMaterialfv_size(pname);
	if (compsize < 0) compsize = 0;
	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT_ARRAY(pc + 8, compsize);

	glMaterialfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *)(pc + 8)
	);
}

void __glXDispSwap_Materiali(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);

	glMateriali( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLint    *)(pc + 8)
	);
}

void __glXDispSwap_Materialiv(GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 4);
	pname = *(GLenum *)(pc + 4);
	compsize = __glMaterialiv_size(pname);
	if (compsize < 0) compsize = 0;
	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT_ARRAY(pc + 8, compsize);

	glMaterialiv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *)(pc + 8)
	);
}

void __glXDispSwap_PointSize(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_FLOAT(pc + 0);

	glPointSize( 
		*(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_PolygonMode(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);

	glPolygonMode( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4)
	);
}

void __glXDispSwap_Scissor(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);
	__GLX_SWAP_INT(pc + 12);

	glScissor( 
		*(GLint    *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLsizei  *)(pc + 8),
		*(GLsizei  *)(pc + 12)
	);
}

void __glXDispSwap_ShadeModel(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glShadeModel( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDispSwap_TexParameterf(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_FLOAT(pc + 8);

	glTexParameterf( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLfloat  *)(pc + 8)
	);
}

void __glXDispSwap_TexParameterfv(GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 4);
	pname = *(GLenum *)(pc + 4);
	compsize = __glTexParameterfv_size(pname);
	if (compsize < 0) compsize = 0;
	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT_ARRAY(pc + 8, compsize);

	glTexParameterfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *)(pc + 8)
	);
}

void __glXDispSwap_TexParameteri(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);

	glTexParameteri( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLint    *)(pc + 8)
	);
}

void __glXDispSwap_TexParameteriv(GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 4);
	pname = *(GLenum *)(pc + 4);
	compsize = __glTexParameteriv_size(pname);
	if (compsize < 0) compsize = 0;
	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT_ARRAY(pc + 8, compsize);

	glTexParameteriv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *)(pc + 8)
	);
}

void __glXDispSwap_TexEnvf(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_FLOAT(pc + 8);

	glTexEnvf( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLfloat  *)(pc + 8)
	);
}

void __glXDispSwap_TexEnvfv(GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 4);
	pname = *(GLenum *)(pc + 4);
	compsize = __glTexEnvfv_size(pname);
	if (compsize < 0) compsize = 0;
	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT_ARRAY(pc + 8, compsize);

	glTexEnvfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *)(pc + 8)
	);
}

void __glXDispSwap_TexEnvi(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);

	glTexEnvi( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLint    *)(pc + 8)
	);
}

void __glXDispSwap_TexEnviv(GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 4);
	pname = *(GLenum *)(pc + 4);
	compsize = __glTexEnviv_size(pname);
	if (compsize < 0) compsize = 0;
	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT_ARRAY(pc + 8, compsize);

	glTexEnviv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *)(pc + 8)
	);
}

void __glXDispSwap_TexGend(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;


#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 16);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_INT(pc + 8);
	__GLX_SWAP_INT(pc + 12);
	__GLX_SWAP_DOUBLE(pc + 0);

	glTexGend( 
		*(GLenum   *)(pc + 8),
		*(GLenum   *)(pc + 12),
		*(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_TexGendv(GLbyte *pc)
{
	GLenum pname;
#ifdef __GLX_ALIGN64
	GLint cmdlen;
#endif
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 4);
	pname = *(GLenum *)(pc + 4);
	compsize = __glTexGendv_size(pname);
	if (compsize < 0) compsize = 0;

#ifdef __GLX_ALIGN64
	cmdlen = __GLX_PAD(8+compsize*8);
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, cmdlen);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_DOUBLE_ARRAY(pc + 8, compsize);

	glTexGendv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLdouble *)(pc + 8)
	);
}

void __glXDispSwap_TexGenf(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_FLOAT(pc + 8);

	glTexGenf( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLfloat  *)(pc + 8)
	);
}

void __glXDispSwap_TexGenfv(GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 4);
	pname = *(GLenum *)(pc + 4);
	compsize = __glTexGenfv_size(pname);
	if (compsize < 0) compsize = 0;
	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT_ARRAY(pc + 8, compsize);

	glTexGenfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *)(pc + 8)
	);
}

void __glXDispSwap_TexGeni(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);

	glTexGeni( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLint    *)(pc + 8)
	);
}

void __glXDispSwap_TexGeniv(GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 4);
	pname = *(GLenum *)(pc + 4);
	compsize = __glTexGeniv_size(pname);
	if (compsize < 0) compsize = 0;
	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT_ARRAY(pc + 8, compsize);

	glTexGeniv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *)(pc + 8)
	);
}

void __glXDispSwap_InitNames(GLbyte *pc)
{
	glInitNames( 
	);
}

void __glXDispSwap_LoadName(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glLoadName( 
		*(GLuint   *)(pc + 0)
	);
}

void __glXDispSwap_PassThrough(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_FLOAT(pc + 0);

	glPassThrough( 
		*(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_PopName(GLbyte *pc)
{
	glPopName( 
	);
}

void __glXDispSwap_PushName(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glPushName( 
		*(GLuint   *)(pc + 0)
	);
}

void __glXDispSwap_DrawBuffer(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glDrawBuffer( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDispSwap_Clear(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glClear( 
		*(GLbitfield *)(pc + 0)
	);
}

void __glXDispSwap_ClearAccum(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_FLOAT(pc + 0);
	__GLX_SWAP_FLOAT(pc + 4);
	__GLX_SWAP_FLOAT(pc + 8);
	__GLX_SWAP_FLOAT(pc + 12);

	glClearAccum( 
		*(GLfloat  *)(pc + 0),
		*(GLfloat  *)(pc + 4),
		*(GLfloat  *)(pc + 8),
		*(GLfloat  *)(pc + 12)
	);
}

void __glXDispSwap_ClearIndex(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_FLOAT(pc + 0);

	glClearIndex( 
		*(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_ClearColor(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_FLOAT(pc + 0);
	__GLX_SWAP_FLOAT(pc + 4);
	__GLX_SWAP_FLOAT(pc + 8);
	__GLX_SWAP_FLOAT(pc + 12);

	glClearColor( 
		*(GLclampf *)(pc + 0),
		*(GLclampf *)(pc + 4),
		*(GLclampf *)(pc + 8),
		*(GLclampf *)(pc + 12)
	);
}

void __glXDispSwap_ClearStencil(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glClearStencil( 
		*(GLint    *)(pc + 0)
	);
}

void __glXDispSwap_ClearDepth(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;


#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 8);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE(pc + 0);

	glClearDepth( 
		*(GLclampd *)(pc + 0)
	);
}

void __glXDispSwap_StencilMask(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glStencilMask( 
		*(GLuint   *)(pc + 0)
	);
}

void __glXDispSwap_ColorMask(GLbyte *pc)
{
	glColorMask( 
		*(GLboolean *)(pc + 0),
		*(GLboolean *)(pc + 1),
		*(GLboolean *)(pc + 2),
		*(GLboolean *)(pc + 3)
	);
}

void __glXDispSwap_DepthMask(GLbyte *pc)
{
	glDepthMask( 
		*(GLboolean *)(pc + 0)
	);
}

void __glXDispSwap_IndexMask(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glIndexMask( 
		*(GLuint   *)(pc + 0)
	);
}

void __glXDispSwap_Accum(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT(pc + 4);

	glAccum( 
		*(GLenum   *)(pc + 0),
		*(GLfloat  *)(pc + 4)
	);
}

void __glXDispSwap_Disable(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glDisable( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDispSwap_Enable(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glEnable( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDispSwap_PopAttrib(GLbyte *pc)
{
	glPopAttrib( 
	);
}

void __glXDispSwap_PushAttrib(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glPushAttrib( 
		*(GLbitfield *)(pc + 0)
	);
}

void __glXDispSwap_MapGrid1d(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;


#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 20);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_INT(pc + 16);
	__GLX_SWAP_DOUBLE(pc + 0);
	__GLX_SWAP_DOUBLE(pc + 8);

	glMapGrid1d( 
		*(GLint    *)(pc + 16),
		*(GLdouble *)(pc + 0),
		*(GLdouble *)(pc + 8)
	);
}

void __glXDispSwap_MapGrid1f(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT(pc + 4);
	__GLX_SWAP_FLOAT(pc + 8);

	glMapGrid1f( 
		*(GLint    *)(pc + 0),
		*(GLfloat  *)(pc + 4),
		*(GLfloat  *)(pc + 8)
	);
}

void __glXDispSwap_MapGrid2d(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;


#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 40);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_INT(pc + 32);
	__GLX_SWAP_DOUBLE(pc + 0);
	__GLX_SWAP_DOUBLE(pc + 8);
	__GLX_SWAP_INT(pc + 36);
	__GLX_SWAP_DOUBLE(pc + 16);
	__GLX_SWAP_DOUBLE(pc + 24);

	glMapGrid2d( 
		*(GLint    *)(pc + 32),
		*(GLdouble *)(pc + 0),
		*(GLdouble *)(pc + 8),
		*(GLint    *)(pc + 36),
		*(GLdouble *)(pc + 16),
		*(GLdouble *)(pc + 24)
	);
}

void __glXDispSwap_MapGrid2f(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT(pc + 4);
	__GLX_SWAP_FLOAT(pc + 8);
	__GLX_SWAP_INT(pc + 12);
	__GLX_SWAP_FLOAT(pc + 16);
	__GLX_SWAP_FLOAT(pc + 20);

	glMapGrid2f( 
		*(GLint    *)(pc + 0),
		*(GLfloat  *)(pc + 4),
		*(GLfloat  *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLfloat  *)(pc + 16),
		*(GLfloat  *)(pc + 20)
	);
}

void __glXDispSwap_EvalCoord1dv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 8);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 1);

	glEvalCoord1dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_EvalCoord1fv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_FLOAT_ARRAY(pc + 0, 1);

	glEvalCoord1fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_EvalCoord2dv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 16);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 2);

	glEvalCoord2dv( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_EvalCoord2fv(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_FLOAT_ARRAY(pc + 0, 2);

	glEvalCoord2fv( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_EvalMesh1(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);

	glEvalMesh1( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLint    *)(pc + 8)
	);
}

void __glXDispSwap_EvalPoint1(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glEvalPoint1( 
		*(GLint    *)(pc + 0)
	);
}

void __glXDispSwap_EvalMesh2(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);
	__GLX_SWAP_INT(pc + 12);
	__GLX_SWAP_INT(pc + 16);

	glEvalMesh2( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLint    *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLint    *)(pc + 16)
	);
}

void __glXDispSwap_EvalPoint2(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);

	glEvalPoint2( 
		*(GLint    *)(pc + 0),
		*(GLint    *)(pc + 4)
	);
}

void __glXDispSwap_AlphaFunc(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT(pc + 4);

	glAlphaFunc( 
		*(GLenum   *)(pc + 0),
		*(GLclampf *)(pc + 4)
	);
}

void __glXDispSwap_BlendFunc(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);

	glBlendFunc( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4)
	);
}

void __glXDispSwap_LogicOp(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glLogicOp( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDispSwap_StencilFunc(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);

	glStencilFunc( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLuint   *)(pc + 8)
	);
}

void __glXDispSwap_StencilOp(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);

	glStencilOp( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLenum   *)(pc + 8)
	);
}

void __glXDispSwap_DepthFunc(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glDepthFunc( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDispSwap_PixelZoom(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_FLOAT(pc + 0);
	__GLX_SWAP_FLOAT(pc + 4);

	glPixelZoom( 
		*(GLfloat  *)(pc + 0),
		*(GLfloat  *)(pc + 4)
	);
}

void __glXDispSwap_PixelTransferf(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT(pc + 4);

	glPixelTransferf( 
		*(GLenum   *)(pc + 0),
		*(GLfloat  *)(pc + 4)
	);
}

void __glXDispSwap_PixelTransferi(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);

	glPixelTransferi( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4)
	);
}

void __glXDispSwap_PixelMapfv(GLbyte *pc)
{
	GLint mapsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 4);
	mapsize = *(GLint *)(pc + 4);
	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT_ARRAY(pc + 8, mapsize);

	glPixelMapfv( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		(GLfloat  *)(pc + 8)
	);
}

void __glXDispSwap_PixelMapuiv(GLbyte *pc)
{
	GLint mapsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 4);
	mapsize = *(GLint *)(pc + 4);
	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT_ARRAY(pc + 8, mapsize);

	glPixelMapuiv( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		(GLuint   *)(pc + 8)
	);
}

void __glXDispSwap_PixelMapusv(GLbyte *pc)
{
	GLint mapsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 4);
	mapsize = *(GLint *)(pc + 4);
	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_SHORT_ARRAY(pc + 8, mapsize);

	glPixelMapusv( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		(GLushort *)(pc + 8)
	);
}

void __glXDispSwap_ReadBuffer(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glReadBuffer( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDispSwap_CopyPixels(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);
	__GLX_SWAP_INT(pc + 12);
	__GLX_SWAP_INT(pc + 16);

	glCopyPixels( 
		*(GLint    *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLsizei  *)(pc + 8),
		*(GLsizei  *)(pc + 12),
		*(GLenum   *)(pc + 16)
	);
}

void __glXDispSwap_DepthRange(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;


#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 16);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE(pc + 0);
	__GLX_SWAP_DOUBLE(pc + 8);

	glDepthRange( 
		*(GLclampd *)(pc + 0),
		*(GLclampd *)(pc + 8)
	);
}

void __glXDispSwap_Frustum(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;


#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 48);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE(pc + 0);
	__GLX_SWAP_DOUBLE(pc + 8);
	__GLX_SWAP_DOUBLE(pc + 16);
	__GLX_SWAP_DOUBLE(pc + 24);
	__GLX_SWAP_DOUBLE(pc + 32);
	__GLX_SWAP_DOUBLE(pc + 40);

	glFrustum( 
		*(GLdouble *)(pc + 0),
		*(GLdouble *)(pc + 8),
		*(GLdouble *)(pc + 16),
		*(GLdouble *)(pc + 24),
		*(GLdouble *)(pc + 32),
		*(GLdouble *)(pc + 40)
	);
}

void __glXDispSwap_LoadIdentity(GLbyte *pc)
{
	glLoadIdentity( 
	);
}

void __glXDispSwap_LoadMatrixf(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_FLOAT_ARRAY(pc + 0, 16);

	glLoadMatrixf( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_LoadMatrixd(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 128);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 16);

	glLoadMatrixd( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_MatrixMode(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glMatrixMode( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDispSwap_MultMatrixf(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_FLOAT_ARRAY(pc + 0, 16);

	glMultMatrixf( 
		(GLfloat  *)(pc + 0)
	);
}

void __glXDispSwap_MultMatrixd(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 128);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 16);

	glMultMatrixd( 
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_Ortho(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;


#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 48);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE(pc + 0);
	__GLX_SWAP_DOUBLE(pc + 8);
	__GLX_SWAP_DOUBLE(pc + 16);
	__GLX_SWAP_DOUBLE(pc + 24);
	__GLX_SWAP_DOUBLE(pc + 32);
	__GLX_SWAP_DOUBLE(pc + 40);

	glOrtho( 
		*(GLdouble *)(pc + 0),
		*(GLdouble *)(pc + 8),
		*(GLdouble *)(pc + 16),
		*(GLdouble *)(pc + 24),
		*(GLdouble *)(pc + 32),
		*(GLdouble *)(pc + 40)
	);
}

void __glXDispSwap_PopMatrix(GLbyte *pc)
{
	glPopMatrix( 
	);
}

void __glXDispSwap_PushMatrix(GLbyte *pc)
{
	glPushMatrix( 
	);
}

void __glXDispSwap_Rotated(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;


#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 32);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE(pc + 0);
	__GLX_SWAP_DOUBLE(pc + 8);
	__GLX_SWAP_DOUBLE(pc + 16);
	__GLX_SWAP_DOUBLE(pc + 24);

	glRotated( 
		*(GLdouble *)(pc + 0),
		*(GLdouble *)(pc + 8),
		*(GLdouble *)(pc + 16),
		*(GLdouble *)(pc + 24)
	);
}

void __glXDispSwap_Rotatef(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_FLOAT(pc + 0);
	__GLX_SWAP_FLOAT(pc + 4);
	__GLX_SWAP_FLOAT(pc + 8);
	__GLX_SWAP_FLOAT(pc + 12);

	glRotatef( 
		*(GLfloat  *)(pc + 0),
		*(GLfloat  *)(pc + 4),
		*(GLfloat  *)(pc + 8),
		*(GLfloat  *)(pc + 12)
	);
}

void __glXDispSwap_Scaled(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;


#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 24);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE(pc + 0);
	__GLX_SWAP_DOUBLE(pc + 8);
	__GLX_SWAP_DOUBLE(pc + 16);

	glScaled( 
		*(GLdouble *)(pc + 0),
		*(GLdouble *)(pc + 8),
		*(GLdouble *)(pc + 16)
	);
}

void __glXDispSwap_Scalef(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_FLOAT(pc + 0);
	__GLX_SWAP_FLOAT(pc + 4);
	__GLX_SWAP_FLOAT(pc + 8);

	glScalef( 
		*(GLfloat  *)(pc + 0),
		*(GLfloat  *)(pc + 4),
		*(GLfloat  *)(pc + 8)
	);
}

void __glXDispSwap_Translated(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;


#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 24);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_DOUBLE(pc + 0);
	__GLX_SWAP_DOUBLE(pc + 8);
	__GLX_SWAP_DOUBLE(pc + 16);

	glTranslated( 
		*(GLdouble *)(pc + 0),
		*(GLdouble *)(pc + 8),
		*(GLdouble *)(pc + 16)
	);
}

void __glXDispSwap_Translatef(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_FLOAT(pc + 0);
	__GLX_SWAP_FLOAT(pc + 4);
	__GLX_SWAP_FLOAT(pc + 8);

	glTranslatef( 
		*(GLfloat  *)(pc + 0),
		*(GLfloat  *)(pc + 4),
		*(GLfloat  *)(pc + 8)
	);
}

void __glXDispSwap_Viewport(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);
	__GLX_SWAP_INT(pc + 12);

	glViewport( 
		*(GLint    *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLsizei  *)(pc + 8),
		*(GLsizei  *)(pc + 12)
	);
}

void __glXDispSwap_PolygonOffset(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_FLOAT(pc + 0);
	__GLX_SWAP_FLOAT(pc + 4);

	glPolygonOffset( 
		*(GLfloat  *)(pc + 0),
		*(GLfloat  *)(pc + 4)
	);
}

void __glXDispSwap_CopyTexImage1D(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);
	__GLX_SWAP_INT(pc + 12);
	__GLX_SWAP_INT(pc + 16);
	__GLX_SWAP_INT(pc + 20);
	__GLX_SWAP_INT(pc + 24);

	glCopyTexImage1D( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLenum   *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLint    *)(pc + 16),
		*(GLsizei  *)(pc + 20),
		*(GLint    *)(pc + 24)
	);
}

void __glXDispSwap_CopyTexImage2D(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);
	__GLX_SWAP_INT(pc + 12);
	__GLX_SWAP_INT(pc + 16);
	__GLX_SWAP_INT(pc + 20);
	__GLX_SWAP_INT(pc + 24);
	__GLX_SWAP_INT(pc + 28);

	glCopyTexImage2D( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLenum   *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLint    *)(pc + 16),
		*(GLsizei  *)(pc + 20),
		*(GLsizei  *)(pc + 24),
		*(GLint    *)(pc + 28)
	);
}

void __glXDispSwap_CopyTexSubImage1D(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);
	__GLX_SWAP_INT(pc + 12);
	__GLX_SWAP_INT(pc + 16);
	__GLX_SWAP_INT(pc + 20);

	glCopyTexSubImage1D( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLint    *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLint    *)(pc + 16),
		*(GLsizei  *)(pc + 20)
	);
}

void __glXDispSwap_CopyTexSubImage2D(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);
	__GLX_SWAP_INT(pc + 12);
	__GLX_SWAP_INT(pc + 16);
	__GLX_SWAP_INT(pc + 20);
	__GLX_SWAP_INT(pc + 24);
	__GLX_SWAP_INT(pc + 28);

	glCopyTexSubImage2D( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLint    *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLint    *)(pc + 16),
		*(GLint    *)(pc + 20),
		*(GLsizei  *)(pc + 24),
		*(GLsizei  *)(pc + 28)
	);
}

void __glXDispSwap_BindTexture(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);

	glBindTexture( 
		*(GLenum   *)(pc + 0),
		*(GLuint   *)(pc + 4)
	);
}

void __glXDispSwap_PrioritizeTextures(GLbyte *pc)
{
	GLsizei n;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	n = *(GLsizei *)(pc + 0);
	__GLX_SWAP_INT_ARRAY(pc + 4, n);
	__GLX_SWAP_FLOAT_ARRAY(pc + 4+n*4, n);

	glPrioritizeTextures( 
		*(GLsizei  *)(pc + 0),
		(GLuint   *)(pc + 4),
		(GLclampf *)(pc + 4+n*4)
	);
}

void __glXDispSwap_Indexubv(GLbyte *pc)
{
	glIndexubv( 
		(GLubyte  *)(pc + 0)
	);
}

void __glXDispSwap_BlendColor(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_FLOAT(pc + 0);
	__GLX_SWAP_FLOAT(pc + 4);
	__GLX_SWAP_FLOAT(pc + 8);
	__GLX_SWAP_FLOAT(pc + 12);

	glBlendColor( 
		*(GLclampf *)(pc + 0),
		*(GLclampf *)(pc + 4),
		*(GLclampf *)(pc + 8),
		*(GLclampf *)(pc + 12)
	);
}

void __glXDispSwap_BlendEquation(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glBlendEquation( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDispSwap_ColorTableParameterfv(GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 4);
	pname = *(GLenum *)(pc + 4);
	compsize = __glColorTableParameterfv_size(pname);
	if (compsize < 0) compsize = 0;
	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT_ARRAY(pc + 8, compsize);

	glColorTableParameterfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *)(pc + 8)
	);
}

void __glXDispSwap_ColorTableParameteriv(GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 4);
	pname = *(GLenum *)(pc + 4);
	compsize = __glColorTableParameteriv_size(pname);
	if (compsize < 0) compsize = 0;
	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT_ARRAY(pc + 8, compsize);

	glColorTableParameteriv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *)(pc + 8)
	);
}

void __glXDispSwap_CopyColorTable(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);
	__GLX_SWAP_INT(pc + 12);
	__GLX_SWAP_INT(pc + 16);

	glCopyColorTable( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLint    *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLsizei  *)(pc + 16)
	);
}

void __glXDispSwap_CopyColorSubTable(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);
	__GLX_SWAP_INT(pc + 12);
	__GLX_SWAP_INT(pc + 16);

	glCopyColorSubTable( 
		*(GLenum   *)(pc + 0),
		*(GLsizei  *)(pc + 4),
		*(GLint    *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLsizei  *)(pc + 16)
	);
}

void __glXDispSwap_ConvolutionParameterf(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_FLOAT(pc + 8);

	glConvolutionParameterf( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLfloat  *)(pc + 8)
	);
}

void __glXDispSwap_ConvolutionParameterfv(GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 4);
	pname = *(GLenum *)(pc + 4);
	compsize = __glConvolutionParameterfv_size(pname);
	if (compsize < 0) compsize = 0;
	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT_ARRAY(pc + 8, compsize);

	glConvolutionParameterfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *)(pc + 8)
	);
}

void __glXDispSwap_ConvolutionParameteri(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);

	glConvolutionParameteri( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLint    *)(pc + 8)
	);
}

void __glXDispSwap_ConvolutionParameteriv(GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 4);
	pname = *(GLenum *)(pc + 4);
	compsize = __glConvolutionParameteriv_size(pname);
	if (compsize < 0) compsize = 0;
	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT_ARRAY(pc + 8, compsize);

	glConvolutionParameteriv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *)(pc + 8)
	);
}

void __glXDispSwap_CopyConvolutionFilter1D(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);
	__GLX_SWAP_INT(pc + 12);
	__GLX_SWAP_INT(pc + 16);

	glCopyConvolutionFilter1D( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLint    *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLsizei  *)(pc + 16)
	);
}

void __glXDispSwap_CopyConvolutionFilter2D(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);
	__GLX_SWAP_INT(pc + 12);
	__GLX_SWAP_INT(pc + 16);
	__GLX_SWAP_INT(pc + 20);

	glCopyConvolutionFilter2D( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLint    *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLsizei  *)(pc + 16),
		*(GLsizei  *)(pc + 20)
	);
}

void __glXDispSwap_Histogram(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);

	glHistogram( 
		*(GLenum   *)(pc + 0),
		*(GLsizei  *)(pc + 4),
		*(GLenum   *)(pc + 8),
		*(GLboolean *)(pc + 12)
	);
}

void __glXDispSwap_Minmax(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);

	glMinmax( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		*(GLboolean *)(pc + 8)
	);
}

void __glXDispSwap_ResetHistogram(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glResetHistogram( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDispSwap_ResetMinmax(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glResetMinmax( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDispSwap_CopyTexSubImage3D(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + 8);
	__GLX_SWAP_INT(pc + 12);
	__GLX_SWAP_INT(pc + 16);
	__GLX_SWAP_INT(pc + 20);
	__GLX_SWAP_INT(pc + 24);
	__GLX_SWAP_INT(pc + 28);
	__GLX_SWAP_INT(pc + 32);

	glCopyTexSubImage3D( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLint    *)(pc + 8),
		*(GLint    *)(pc + 12),
		*(GLint    *)(pc + 16),
		*(GLint    *)(pc + 20),
		*(GLint    *)(pc + 24),
		*(GLsizei  *)(pc + 28),
		*(GLsizei  *)(pc + 32)
	);
}

void __glXDispSwap_ActiveTextureARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glActiveTextureARB( 
		*(GLenum   *)(pc + 0)
	);
}

void __glXDispSwap_MultiTexCoord1dvARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 12);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_INT(pc + 8);
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 1);

	glMultiTexCoord1dvARB( 
		*(GLenum   *)(pc + 8),
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_MultiTexCoord1fvARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT_ARRAY(pc + 4, 1);

	glMultiTexCoord1fvARB( 
		*(GLenum   *)(pc + 0),
		(GLfloat  *)(pc + 4)
	);
}

void __glXDispSwap_MultiTexCoord1ivARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT_ARRAY(pc + 4, 1);

	glMultiTexCoord1ivARB( 
		*(GLenum   *)(pc + 0),
		(GLint    *)(pc + 4)
	);
}

void __glXDispSwap_MultiTexCoord1svARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_SHORT_ARRAY(pc + 4, 1);

	glMultiTexCoord1svARB( 
		*(GLenum   *)(pc + 0),
		(GLshort  *)(pc + 4)
	);
}

void __glXDispSwap_MultiTexCoord2dvARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 20);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_INT(pc + 16);
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 2);

	glMultiTexCoord2dvARB( 
		*(GLenum   *)(pc + 16),
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_MultiTexCoord2fvARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT_ARRAY(pc + 4, 2);

	glMultiTexCoord2fvARB( 
		*(GLenum   *)(pc + 0),
		(GLfloat  *)(pc + 4)
	);
}

void __glXDispSwap_MultiTexCoord2ivARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT_ARRAY(pc + 4, 2);

	glMultiTexCoord2ivARB( 
		*(GLenum   *)(pc + 0),
		(GLint    *)(pc + 4)
	);
}

void __glXDispSwap_MultiTexCoord2svARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_SHORT_ARRAY(pc + 4, 2);

	glMultiTexCoord2svARB( 
		*(GLenum   *)(pc + 0),
		(GLshort  *)(pc + 4)
	);
}

void __glXDispSwap_MultiTexCoord3dvARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 28);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_INT(pc + 24);
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 3);

	glMultiTexCoord3dvARB( 
		*(GLenum   *)(pc + 24),
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_MultiTexCoord3fvARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT_ARRAY(pc + 4, 3);

	glMultiTexCoord3fvARB( 
		*(GLenum   *)(pc + 0),
		(GLfloat  *)(pc + 4)
	);
}

void __glXDispSwap_MultiTexCoord3ivARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT_ARRAY(pc + 4, 3);

	glMultiTexCoord3ivARB( 
		*(GLenum   *)(pc + 0),
		(GLint    *)(pc + 4)
	);
}

void __glXDispSwap_MultiTexCoord3svARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_SHORT_ARRAY(pc + 4, 3);

	glMultiTexCoord3svARB( 
		*(GLenum   *)(pc + 0),
		(GLshort  *)(pc + 4)
	);
}

void __glXDispSwap_MultiTexCoord4dvARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

#ifdef __GLX_ALIGN64
	if ((unsigned long)(pc) & 7) {
	    __GLX_MEM_COPY(pc-4, pc, 36);
	    pc -= 4;
	}
#endif
	__GLX_SWAP_INT(pc + 32);
	__GLX_SWAP_DOUBLE_ARRAY(pc + 0, 4);

	glMultiTexCoord4dvARB( 
		*(GLenum   *)(pc + 32),
		(GLdouble *)(pc + 0)
	);
}

void __glXDispSwap_MultiTexCoord4fvARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT_ARRAY(pc + 4, 4);

	glMultiTexCoord4fvARB( 
		*(GLenum   *)(pc + 0),
		(GLfloat  *)(pc + 4)
	);
}

void __glXDispSwap_MultiTexCoord4ivARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_INT_ARRAY(pc + 4, 4);

	glMultiTexCoord4ivARB( 
		*(GLenum   *)(pc + 0),
		(GLint    *)(pc + 4)
	);
}

void __glXDispSwap_MultiTexCoord4svARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_SHORT_ARRAY(pc + 4, 4);

	glMultiTexCoord4svARB( 
		*(GLenum   *)(pc + 0),
		(GLshort  *)(pc + 4)
	);
}


/*
 * Extensions
 */

#ifndef MISSING_GL_EXTS

void __glXDispSwap_PointParameterfARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_SWAP_INT(pc + 0);
	__GLX_SWAP_FLOAT(pc + 4);
	glPointParameterfARB(
		*(GLenum *)(pc + 0),
		*(GLfloat *)(pc + 4)
	);
}

void __glXDispSwap_PointParameterfvARB(GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_DECLARE_SWAP_ARRAY_VARIABLES;

	__GLX_SWAP_INT(pc + 0);
	pname = *(GLenum *)(pc + 0);
	compsize = __glPointParameterfvEXT_size(pname);
	if (compsize < 0) compsize = 0;
	__GLX_SWAP_FLOAT_ARRAY(pc + 4, compsize);

	glPointParameterfvARB(
		*(GLenum *)(pc + 0),
		(GLfloat *)(pc + 4)
	);
}

void __glXDispSwap_ActiveStencilFaceEXT(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(pc + 0);

	glActiveStencilFaceEXT(
		*(GLenum *)(pc + 0)
	);
}

void __glXDispSwap_WindowPos3fARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_SWAP_FLOAT(pc + 0);
	__GLX_SWAP_FLOAT(pc + 4);
	__GLX_SWAP_FLOAT(pc + 8);
	glWindowPos3fARB(
		*(GLfloat *)(pc + 0),
		*(GLfloat *)(pc + 4),
		*(GLfloat *)(pc + 8)
	);
}
#endif /* !MISSING_GL_EXTS */

void __glXDispSwap_SampleCoverageARB(GLbyte *pc)
{
	__GLX_DECLARE_SWAP_VARIABLES;
	__GLX_SWAP_FLOAT(pc + 0);
	__GLX_SWAP_INT(pc + 4);

	glSampleCoverageARB(
		*(GLfloat *)(pc + 0),
		*(GLboolean *)(pc + 4)
	);
}
