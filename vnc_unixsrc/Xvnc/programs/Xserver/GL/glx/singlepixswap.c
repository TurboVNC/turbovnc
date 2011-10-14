/* $XFree86: xc/programs/Xserver/GL/glx/singlepixswap.c,v 1.5 2001/03/21 16:29:37 dawes Exp $ */
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
#include "glxext.h"
#include "singlesize.h"
#include "unpack.h"
#include "g_disptab.h"
#include "g_disptab_EXT.h"

int __glXDispSwap_ReadPixels(__GLXclientState *cl, GLbyte *pc)
{
    GLsizei width, height;
    GLenum format, type;
    GLboolean swapBytes, lsbFirst;
    GLint compsize;
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    char *answer, answerBuffer[200];

    __GLX_SWAP_INT(&((xGLXSingleReq *)pc)->contextTag);
    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    pc += __GLX_SINGLE_HDR_SIZE;
    __GLX_SWAP_INT(pc+0);
    __GLX_SWAP_INT(pc+4);
    __GLX_SWAP_INT(pc+8);
    __GLX_SWAP_INT(pc+12);
    __GLX_SWAP_INT(pc+16);
    __GLX_SWAP_INT(pc+20);

    width = *(GLsizei *)(pc + 8);
    height = *(GLsizei *)(pc + 12);
    format = *(GLenum *)(pc + 16);
    type = *(GLenum *)(pc + 20);
    swapBytes = *(GLboolean *)(pc + 24);
    lsbFirst = *(GLboolean *)(pc + 25);
    compsize = __glReadPixels_size(format,type,width,height);
    if (compsize < 0) compsize = 0;

    glPixelStorei(GL_PACK_SWAP_BYTES, !swapBytes);
    glPixelStorei(GL_PACK_LSB_FIRST, lsbFirst);
    __GLX_GET_ANSWER_BUFFER(answer,cl,compsize,1);
    __glXClearErrorOccured();
    glReadPixels(
		 *(GLint    *)(pc + 0),
		 *(GLint    *)(pc + 4),
		 *(GLsizei  *)(pc + 8),
		 *(GLsizei  *)(pc + 12),
		 *(GLenum   *)(pc + 16),
		 *(GLenum   *)(pc + 20),
		 answer
		 );

    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SWAP_REPLY_HEADER();
	__GLX_SEND_HEADER();
    } else {
	__GLX_BEGIN_REPLY(compsize);
	__GLX_SWAP_REPLY_HEADER();
	__GLX_SEND_HEADER();
	__GLX_SEND_VOID_ARRAY(compsize);
    }
    return Success;
}

int __glXDispSwap_GetTexImage(__GLXclientState *cl, GLbyte *pc)
{
    GLint level, compsize;
    GLenum format, type, target;
    GLboolean swapBytes;
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    char *answer, answerBuffer[200];
    GLint width=0, height=0, depth=1;

    __GLX_SWAP_INT(&((xGLXSingleReq *)pc)->contextTag);
    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    pc += __GLX_SINGLE_HDR_SIZE;
    __GLX_SWAP_INT(pc+0);
    __GLX_SWAP_INT(pc+4);
    __GLX_SWAP_INT(pc+8);
    __GLX_SWAP_INT(pc+12);

    level = *(GLint *)(pc + 4);
    format = *(GLenum *)(pc + 8);
    type = *(GLenum *)(pc + 12);
    target = *(GLenum *)(pc + 0);
    swapBytes = *(GLboolean *)(pc + 16);

    glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, &height);
    if ( target == GL_TEXTURE_3D) {
	glGetTexLevelParameteriv(target, level, GL_TEXTURE_DEPTH, &depth);
    }
    /*
     * The three queries above might fail if we're in a state where queries
     * are illegal, but then width, height, and depth would still be zero anyway.
     */
    compsize = __glGetTexImage_size(target,level,format,type,width,height,depth);
    if (compsize < 0) compsize = 0;

    glPixelStorei(GL_PACK_SWAP_BYTES, !swapBytes);
    __GLX_GET_ANSWER_BUFFER(answer,cl,compsize,1);
    __glXClearErrorOccured();
    glGetTexImage(
		  *(GLenum   *)(pc + 0),
		  *(GLint    *)(pc + 4),
		  *(GLenum   *)(pc + 8),
		  *(GLenum   *)(pc + 12),
		  answer
		  );

    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SWAP_REPLY_HEADER();
	__GLX_SEND_HEADER();
    } else {
	__GLX_BEGIN_REPLY(compsize);
	__GLX_SWAP_REPLY_HEADER();
	__GLX_SWAP_INT(&width);
	__GLX_SWAP_INT(&height);
	__GLX_SWAP_INT(&depth);
	((xGLXGetTexImageReply *)&__glXReply)->width = width;
	((xGLXGetTexImageReply *)&__glXReply)->height = height;
	((xGLXGetTexImageReply *)&__glXReply)->depth = depth;
	__GLX_SEND_HEADER();
	__GLX_SEND_VOID_ARRAY(compsize);
    }
    return Success;
}

int __glXDispSwap_GetPolygonStipple(__GLXclientState *cl, GLbyte *pc)
{
    GLboolean lsbFirst;
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    GLubyte answerBuffer[200];
    char *answer;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT(&((xGLXSingleReq *)pc)->contextTag);
    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }
    pc += __GLX_SINGLE_HDR_SIZE;
    lsbFirst = *(GLboolean *)(pc + 0);

    glPixelStorei(GL_PACK_LSB_FIRST, lsbFirst);
    __GLX_GET_ANSWER_BUFFER(answer,cl,128,1);

    __glXClearErrorOccured();
    glGetPolygonStipple(
			(GLubyte  *) answer
			);
    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SWAP_REPLY_HEADER();
	__GLX_SEND_HEADER();
    } else {
	__GLX_BEGIN_REPLY(128);
	__GLX_SWAP_REPLY_HEADER();
	__GLX_SEND_HEADER();
	__GLX_SEND_BYTE_ARRAY(128);
    }
    return Success;
}

int __glXDispSwap_GetSeparableFilter(__GLXclientState *cl, GLbyte *pc)
{
    GLint compsize, compsize2;
    GLenum format, type, target;
    GLboolean swapBytes;
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    __GLX_DECLARE_SWAP_VARIABLES;
    char *answer, answerBuffer[200];
    GLint width=0, height=0;

    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    pc += __GLX_SINGLE_HDR_SIZE;
    __GLX_SWAP_INT(pc+0);
    __GLX_SWAP_INT(pc+4);
    __GLX_SWAP_INT(pc+8);

    format = *(GLenum *)(pc + 4);
    type = *(GLenum *)(pc + 8);
    target = *(GLenum *)(pc + 0);
    swapBytes = *(GLboolean *)(pc + 12);

    /* target must be SEPARABLE_2D, however I guess we can let the GL
       barf on this one.... */

    glGetConvolutionParameteriv(target, GL_CONVOLUTION_WIDTH, &width);
    glGetConvolutionParameteriv(target, GL_CONVOLUTION_HEIGHT, &height);
    /*
     * The two queries above might fail if we're in a state where queries
     * are illegal, but then width and height would still be zero anyway.
     */
    compsize = __glGetTexImage_size(target,1,format,type,width,1,1);
    compsize2 = __glGetTexImage_size(target,1,format,type,height,1,1);

    if (compsize < 0) compsize = 0;
    if (compsize2 < 0) compsize2 = 0;
    compsize = __GLX_PAD(compsize);
    compsize2 = __GLX_PAD(compsize2);

    glPixelStorei(GL_PACK_SWAP_BYTES, !swapBytes);
    __GLX_GET_ANSWER_BUFFER(answer,cl,compsize + compsize2,1);
    __glXClearErrorOccured();
    glGetSeparableFilter(
		  *(GLenum   *)(pc + 0),
		  *(GLenum   *)(pc + 4),
		  *(GLenum   *)(pc + 8),
		  answer,
		  answer + compsize,
		  NULL
		  );

    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SWAP_REPLY_HEADER();
    } else {
	__GLX_BEGIN_REPLY(compsize + compsize2);
	__GLX_SWAP_REPLY_HEADER();
	__GLX_SWAP_INT(&width);
	__GLX_SWAP_INT(&height);
	((xGLXGetSeparableFilterReply *)&__glXReply)->width = width;
	((xGLXGetSeparableFilterReply *)&__glXReply)->height = height;
	__GLX_SEND_VOID_ARRAY(compsize + compsize2);
    }

    return Success;
}

int __glXDispSwap_GetConvolutionFilter(__GLXclientState *cl, GLbyte *pc)
{
    GLint compsize;
    GLenum format, type, target;
    GLboolean swapBytes;
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    __GLX_DECLARE_SWAP_VARIABLES;
    char *answer, answerBuffer[200];
    GLint width=0, height=0;

    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    pc += __GLX_SINGLE_HDR_SIZE;
    __GLX_SWAP_INT(pc+0);
    __GLX_SWAP_INT(pc+4);
    __GLX_SWAP_INT(pc+8);

    format = *(GLenum *)(pc + 4);
    type = *(GLenum *)(pc + 8);
    target = *(GLenum *)(pc + 0);
    swapBytes = *(GLboolean *)(pc + 12);

    glGetConvolutionParameteriv(target, GL_CONVOLUTION_WIDTH, &width);
    if (target == GL_CONVOLUTION_2D) {
        height = 1;
    } else {
	glGetConvolutionParameteriv(target, GL_CONVOLUTION_HEIGHT, &height);
    }
    /*
     * The two queries above might fail if we're in a state where queries
     * are illegal, but then width and height would still be zero anyway.
     */
    compsize = __glGetTexImage_size(target,1,format,type,width,height,1);
    if (compsize < 0) compsize = 0;

    glPixelStorei(GL_PACK_SWAP_BYTES, !swapBytes);
    __GLX_GET_ANSWER_BUFFER(answer,cl,compsize,1);
    __glXClearErrorOccured();
    glGetConvolutionFilter(
		  *(GLenum   *)(pc + 0),
		  *(GLenum   *)(pc + 4),
		  *(GLenum   *)(pc + 8),
		  answer
		  );

    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SWAP_REPLY_HEADER();
    } else {
	__GLX_BEGIN_REPLY(compsize);
	__GLX_SWAP_REPLY_HEADER();
	__GLX_SWAP_INT(&width);
	__GLX_SWAP_INT(&height);
	((xGLXGetConvolutionFilterReply *)&__glXReply)->width = width;
	((xGLXGetConvolutionFilterReply *)&__glXReply)->height = height;
	__GLX_SEND_VOID_ARRAY(compsize);
    }

    return Success;
}

int __glXDispSwap_GetHistogram(__GLXclientState *cl, GLbyte *pc)
{
    GLint compsize;
    GLenum format, type, target;
    GLboolean swapBytes, reset;
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    __GLX_DECLARE_SWAP_VARIABLES;
    char *answer, answerBuffer[200];
    GLint width=0;

    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    pc += __GLX_SINGLE_HDR_SIZE;
    __GLX_SWAP_INT(pc+0);
    __GLX_SWAP_INT(pc+4);
    __GLX_SWAP_INT(pc+8);

    format = *(GLenum *)(pc + 4);
    type = *(GLenum *)(pc + 8);
    target = *(GLenum *)(pc + 0);
    swapBytes = *(GLboolean *)(pc + 12);
    reset = *(GLboolean *)(pc + 13);

    glGetHistogramParameteriv(target, GL_HISTOGRAM_WIDTH, &width);
    /*
     * The one query above might fail if we're in a state where queries
     * are illegal, but then width would still be zero anyway.
     */
    compsize = __glGetTexImage_size(target,1,format,type,width,1,1);
    if (compsize < 0) compsize = 0;

    glPixelStorei(GL_PACK_SWAP_BYTES, !swapBytes);
    __GLX_GET_ANSWER_BUFFER(answer,cl,compsize,1);
    __glXClearErrorOccured();
    glGetHistogram( target, reset, format, type, answer);

    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SWAP_REPLY_HEADER();
    } else {
	__GLX_BEGIN_REPLY(compsize);
	__GLX_SWAP_REPLY_HEADER();
	__GLX_SWAP_INT(&width);
	((xGLXGetHistogramReply *)&__glXReply)->width = width;
	__GLX_SEND_VOID_ARRAY(compsize);
    }

    return Success;
}

int __glXDispSwap_GetMinmax(__GLXclientState *cl, GLbyte *pc)
{
    GLint compsize;
    GLenum format, type, target;
    GLboolean swapBytes, reset;
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    __GLX_DECLARE_SWAP_VARIABLES;
    char *answer, answerBuffer[200];

    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    pc += __GLX_SINGLE_HDR_SIZE;
    __GLX_SWAP_INT(pc+0);
    __GLX_SWAP_INT(pc+4);
    __GLX_SWAP_INT(pc+8);

    format = *(GLenum *)(pc + 4);
    type = *(GLenum *)(pc + 8);
    target = *(GLenum *)(pc + 0);
    swapBytes = *(GLboolean *)(pc + 12);
    reset = *(GLboolean *)(pc + 13);

    compsize = __glGetTexImage_size(target,1,format,type,2,1,1);
    if (compsize < 0) compsize = 0;

    glPixelStorei(GL_PACK_SWAP_BYTES, !swapBytes);
    __GLX_GET_ANSWER_BUFFER(answer,cl,compsize,1);
    __glXClearErrorOccured();
    glGetMinmax( target, reset, format, type, answer);

    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SWAP_REPLY_HEADER();
    } else {
	__GLX_BEGIN_REPLY(compsize);
	__GLX_SWAP_REPLY_HEADER();
	__GLX_SEND_VOID_ARRAY(compsize);
    }

    return Success;
}

int __glXDispSwap_GetColorTable(__GLXclientState *cl, GLbyte *pc)
{
    GLint compsize;
    GLenum format, type, target;
    GLboolean swapBytes;
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    __GLX_DECLARE_SWAP_VARIABLES;
    char *answer, answerBuffer[200];
    GLint width=0;

    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    pc += __GLX_SINGLE_HDR_SIZE;
    __GLX_SWAP_INT(pc+0);
    __GLX_SWAP_INT(pc+4);
    __GLX_SWAP_INT(pc+8);

    format = *(GLenum *)(pc + 4);
    type = *(GLenum *)(pc + 8);
    target = *(GLenum *)(pc + 0);
    swapBytes = *(GLboolean *)(pc + 12);

    glGetColorTableParameteriv(target, GL_COLOR_TABLE_WIDTH, &width);
    /*
     * The one query above might fail if we're in a state where queries
     * are illegal, but then width would still be zero anyway.
     */
    compsize = __glGetTexImage_size(target,1,format,type,width,1,1);
    if (compsize < 0) compsize = 0;

    glPixelStorei(GL_PACK_SWAP_BYTES, !swapBytes);
    __GLX_GET_ANSWER_BUFFER(answer,cl,compsize,1);
    __glXClearErrorOccured();
    glGetColorTable(
		  *(GLenum   *)(pc + 0),
		  *(GLenum   *)(pc + 4),
		  *(GLenum   *)(pc + 8),
		  answer
		  );

    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SWAP_REPLY_HEADER();
    } else {
	__GLX_BEGIN_REPLY(compsize);
	__GLX_SWAP_REPLY_HEADER();
	__GLX_SWAP_INT(&width);
	((xGLXGetColorTableReply *)&__glXReply)->width = width;
	__GLX_SEND_VOID_ARRAY(compsize);
    }

    return Success;
}
