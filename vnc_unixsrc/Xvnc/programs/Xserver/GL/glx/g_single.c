/* $XFree86: xc/programs/Xserver/GL/glx/g_single.c,v 1.5tsi Exp $ */
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

int __glXDisp_NewList(__GLXclientState *cl, GLbyte *pc)
{
	__GLXcontext *cx;
	int error;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;

	glNewList( 
		*(GLuint   *)(pc + 0),
		*(GLenum   *)(pc + 4)
	);
	__GLX_NOTE_UNFLUSHED_CMDS(cx);
	return Success;
}

int __glXDisp_EndList(__GLXclientState *cl, GLbyte *pc)
{
	__GLXcontext *cx;
	int error;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}

	glEndList( 
	);
	return Success;
}

int __glXDisp_DeleteLists(__GLXclientState *cl, GLbyte *pc)
{
	__GLXcontext *cx;
	int error;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;

	glDeleteLists( 
		*(GLuint   *)(pc + 0),
		*(GLsizei  *)(pc + 4)
	);
	__GLX_NOTE_UNFLUSHED_CMDS(cx);
	return Success;
}

int __glXDisp_GenLists(__GLXclientState *cl, GLbyte *pc)
{
	GLuint retval;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;

	retval =
	glGenLists( 
		*(GLsizei  *)(pc + 0)
	);
	__GLX_PUT_RETVAL(retval);
	__GLX_BEGIN_REPLY(0);
	__GLX_SEND_HEADER();
	return Success;
}

int __glXDisp_PixelStoref(__GLXclientState *cl, GLbyte *pc)
{
	__GLXcontext *cx;
	int error;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;

	glPixelStoref( 
		*(GLenum   *)(pc + 0),
		*(GLfloat  *)(pc + 4)
	);
	__GLX_NOTE_UNFLUSHED_CMDS(cx);
	return Success;
}

int __glXDisp_PixelStorei(__GLXclientState *cl, GLbyte *pc)
{
	__GLXcontext *cx;
	int error;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;

	glPixelStorei( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4)
	);
	__GLX_NOTE_UNFLUSHED_CMDS(cx);
	return Success;
}

int __glXDisp_GetBooleanv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLboolean answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 0);
	compsize = __glGetBooleanv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize,1);
	__glXClearErrorOccured();
	glGetBooleanv( 
		*(GLenum   *)(pc + 0),
		(GLboolean *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_BYTE();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_BYTE_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetDoublev(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLdouble answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 0);
	compsize = __glGetDoublev_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*8,8);
	__glXClearErrorOccured();
	glGetDoublev( 
		*(GLenum   *)(pc + 0),
		(GLdouble *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_DOUBLE();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*8);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_DOUBLE_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetError(__GLXclientState *cl, GLbyte *pc)
{
	GLenum retval;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}

	retval =
	glGetError( 
	);
	__GLX_PUT_RETVAL(retval);
	__GLX_BEGIN_REPLY(0);
	__GLX_SEND_HEADER();
	return Success;
}

int __glXDisp_GetFloatv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLfloat answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 0);
	compsize = __glGetFloatv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetFloatv( 
		*(GLenum   *)(pc + 0),
		(GLfloat  *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_FLOAT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_FLOAT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetIntegerv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLint answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 0);
	compsize = __glGetIntegerv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetIntegerv( 
		*(GLenum   *)(pc + 0),
		(GLint    *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_INT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_INT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetLightfv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLfloat answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 4);
	compsize = __glGetLightfv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetLightfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_FLOAT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_FLOAT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetLightiv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLint answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 4);
	compsize = __glGetLightiv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetLightiv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_INT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_INT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetMapdv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum target;
	GLenum query;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLdouble answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	target = *(GLenum *)(pc + 0);
	query = *(GLenum *)(pc + 4);
	compsize = __glGetMapdv_size(target,query);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*8,8);
	__glXClearErrorOccured();
	glGetMapdv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLdouble *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_DOUBLE();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*8);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_DOUBLE_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetMapfv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum target;
	GLenum query;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLfloat answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	target = *(GLenum *)(pc + 0);
	query = *(GLenum *)(pc + 4);
	compsize = __glGetMapfv_size(target,query);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetMapfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_FLOAT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_FLOAT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetMapiv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum target;
	GLenum query;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLint answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	target = *(GLenum *)(pc + 0);
	query = *(GLenum *)(pc + 4);
	compsize = __glGetMapiv_size(target,query);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetMapiv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_INT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_INT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetMaterialfv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLfloat answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 4);
	compsize = __glGetMaterialfv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetMaterialfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_FLOAT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_FLOAT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetMaterialiv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLint answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 4);
	compsize = __glGetMaterialiv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetMaterialiv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_INT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_INT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetPixelMapfv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum map;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLfloat answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	map = *(GLenum *)(pc + 0);
	compsize = __glGetPixelMapfv_size(map);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetPixelMapfv( 
		*(GLenum   *)(pc + 0),
		(GLfloat  *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_FLOAT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_FLOAT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetPixelMapuiv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum map;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLuint answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	map = *(GLenum *)(pc + 0);
	compsize = __glGetPixelMapuiv_size(map);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetPixelMapuiv( 
		*(GLenum   *)(pc + 0),
		(GLuint   *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_INT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_INT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetPixelMapusv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum map;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLushort answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	map = *(GLenum *)(pc + 0);
	compsize = __glGetPixelMapusv_size(map);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*2,2);
	__glXClearErrorOccured();
	glGetPixelMapusv( 
		*(GLenum   *)(pc + 0),
		(GLushort *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_SHORT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*2);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_SHORT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetTexEnvfv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLfloat answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 4);
	compsize = __glGetTexEnvfv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetTexEnvfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_FLOAT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_FLOAT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetTexEnviv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLint answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 4);
	compsize = __glGetTexEnviv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetTexEnviv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_INT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_INT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetTexGendv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLdouble answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 4);
	compsize = __glGetTexGendv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*8,8);
	__glXClearErrorOccured();
	glGetTexGendv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLdouble *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_DOUBLE();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*8);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_DOUBLE_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetTexGenfv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLfloat answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 4);
	compsize = __glGetTexGenfv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetTexGenfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_FLOAT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_FLOAT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetTexGeniv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLint answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 4);
	compsize = __glGetTexGeniv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetTexGeniv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_INT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_INT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetTexParameterfv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLfloat answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 4);
	compsize = __glGetTexParameterfv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetTexParameterfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_FLOAT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_FLOAT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetTexParameteriv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLint answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 4);
	compsize = __glGetTexParameteriv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetTexParameteriv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_INT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_INT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetTexLevelParameterfv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLfloat answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 8);
	compsize = __glGetTexLevelParameterfv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetTexLevelParameterfv( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLenum   *)(pc + 8),
		(GLfloat  *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_FLOAT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_FLOAT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetTexLevelParameteriv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLint answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 8);
	compsize = __glGetTexLevelParameteriv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetTexLevelParameteriv( 
		*(GLenum   *)(pc + 0),
		*(GLint    *)(pc + 4),
		*(GLenum   *)(pc + 8),
		(GLint    *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_INT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_INT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_IsEnabled(__GLXclientState *cl, GLbyte *pc)
{
	GLboolean retval;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;

	retval =
	glIsEnabled( 
		*(GLenum   *)(pc + 0)
	);
	__GLX_PUT_RETVAL(retval);
	__GLX_BEGIN_REPLY(0);
	__GLX_SEND_HEADER();
	return Success;
}

int __glXDisp_IsList(__GLXclientState *cl, GLbyte *pc)
{
	GLboolean retval;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;

	retval =
	glIsList( 
		*(GLuint   *)(pc + 0)
	);
	__GLX_PUT_RETVAL(retval);
	__GLX_BEGIN_REPLY(0);
	__GLX_SEND_HEADER();
	return Success;
}

int __glXDisp_AreTexturesResident(__GLXclientState *cl, GLbyte *pc)
{
	GLsizei n;
	GLboolean retval;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLboolean answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	n = *(GLsizei *)(pc + 0);

	__GLX_GET_ANSWER_BUFFER(answer,cl,n,1);
	retval =
	glAreTexturesResident( 
		*(GLsizei  *)(pc + 0),
		(GLuint   *)(pc + 4),
		(GLboolean *) answer
	);
	__GLX_PUT_RETVAL(retval);
	__GLX_BEGIN_REPLY(n);
	__GLX_SEND_HEADER();
	__GLX_SEND_BYTE_ARRAY(n);
	return Success;
}

int __glXDisp_DeleteTextures(__GLXclientState *cl, GLbyte *pc)
{
	__GLXcontext *cx;
	int error;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;

	glDeleteTextures( 
		*(GLsizei  *)(pc + 0),
		(GLuint   *)(pc + 4)
	);
	return Success;
}

int __glXDisp_GenTextures(__GLXclientState *cl, GLbyte *pc)
{
	GLsizei n;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLuint answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	n = *(GLsizei *)(pc + 0);

	__GLX_GET_ANSWER_BUFFER(answer,cl,n*4,4);
	glGenTextures( 
		*(GLsizei  *)(pc + 0),
		(GLuint   *) answer
	);
	__GLX_BEGIN_REPLY(n*4);
	__GLX_SEND_HEADER();
	__GLX_SEND_INT_ARRAY(n);
	return Success;
}

int __glXDisp_IsTexture(__GLXclientState *cl, GLbyte *pc)
{
	GLboolean retval;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;

	retval =
	glIsTexture( 
		*(GLuint   *)(pc + 0)
	);
	__GLX_PUT_RETVAL(retval);
	__GLX_BEGIN_REPLY(0);
	__GLX_SEND_HEADER();
	return Success;
}

int __glXDisp_GetColorTableParameterfv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLfloat answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 4);
	compsize = __glGetColorTableParameterfv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetColorTableParameterfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_FLOAT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_FLOAT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetColorTableParameteriv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLint answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 4);
	compsize = __glGetColorTableParameteriv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetColorTableParameteriv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_INT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_INT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetConvolutionParameterfv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLfloat answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 4);
	compsize = __glGetConvolutionParameterfv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetConvolutionParameterfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_FLOAT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_FLOAT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetConvolutionParameteriv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLint answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 4);
	compsize = __glGetConvolutionParameteriv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetConvolutionParameteriv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_INT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_INT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetHistogramParameterfv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLfloat answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 4);
	compsize = __glGetHistogramParameterfv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetHistogramParameterfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_FLOAT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_FLOAT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetHistogramParameteriv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLint answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 4);
	compsize = __glGetHistogramParameteriv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetHistogramParameteriv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_INT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_INT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetMinmaxParameterfv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLfloat answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 4);
	compsize = __glGetMinmaxParameterfv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetMinmaxParameterfv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLfloat  *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_FLOAT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_FLOAT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_GetMinmaxParameteriv(__GLXclientState *cl, GLbyte *pc)
{
	GLenum pname;
	GLint compsize;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLint answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_SINGLE_HDR_SIZE;
	pname = *(GLenum *)(pc + 4);
	compsize = __glGetMinmaxParameteriv_size(pname);
	if (compsize < 0) compsize = 0;

	__GLX_GET_ANSWER_BUFFER(answer,cl,compsize*4,4);
	__glXClearErrorOccured();
	glGetMinmaxParameteriv( 
		*(GLenum   *)(pc + 0),
		*(GLenum   *)(pc + 4),
		(GLint    *) answer
	);
	if (__glXErrorOccured()) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(0);
	    __GLX_SEND_HEADER();
	} else if (compsize == 1) {
	    __GLX_BEGIN_REPLY(0);
	    __GLX_PUT_SIZE(1);
	    __GLX_PUT_INT();
	    __GLX_SEND_HEADER();
	} else {
	    __GLX_BEGIN_REPLY(compsize*4);
	    __GLX_PUT_SIZE(compsize);
	    __GLX_SEND_HEADER();
	    __GLX_SEND_INT_ARRAY(compsize);
	}
	return Success;
}

int __glXDisp_AreTexturesResidentEXT(__GLXclientState *cl, GLbyte *pc)
{
	GLsizei n;
	GLboolean retval;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLboolean answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_VENDPRIV_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_VENDPRIV_HDR_SIZE;
	n = *(GLsizei *)(pc + 0);

	__GLX_GET_ANSWER_BUFFER(answer,cl,n,1);
	retval =
	glAreTexturesResidentEXT( 
		*(GLsizei  *)(pc + 0),
		(GLuint   *)(pc + 4),
		(GLboolean *) answer
	);
	__GLX_PUT_RETVAL(retval);
	__GLX_BEGIN_REPLY(n);
	__GLX_SEND_HEADER();
	__GLX_SEND_BYTE_ARRAY(n);
	return Success;
}

int __glXDisp_DeleteTexturesEXT(__GLXclientState *cl, GLbyte *pc)
{
	__GLXcontext *cx;
	int error;

	cx = __glXForceCurrent(cl, __GLX_GET_VENDPRIV_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_VENDPRIV_HDR_SIZE;

	glDeleteTexturesEXT( 
		*(GLsizei  *)(pc + 0),
		(GLuint   *)(pc + 4)
	);
	return Success;
}

int __glXDisp_GenTexturesEXT(__GLXclientState *cl, GLbyte *pc)
{
	GLsizei n;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;
	GLuint answerBuffer[200];
	char *answer;

	cx = __glXForceCurrent(cl, __GLX_GET_VENDPRIV_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_VENDPRIV_HDR_SIZE;
	n = *(GLsizei *)(pc + 0);

	__GLX_GET_ANSWER_BUFFER(answer,cl,n*4,4);
	glGenTexturesEXT( 
		*(GLsizei  *)(pc + 0),
		(GLuint   *) answer
	);
	__GLX_BEGIN_REPLY(n*4);
	__GLX_SEND_HEADER();
	__GLX_SEND_INT_ARRAY(n);
	return Success;
}

int __glXDisp_IsTextureEXT(__GLXclientState *cl, GLbyte *pc)
{
	GLboolean retval;
	__GLXcontext *cx;
	ClientPtr client = cl->client;
	int error;

	cx = __glXForceCurrent(cl, __GLX_GET_VENDPRIV_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}
	pc += __GLX_VENDPRIV_HDR_SIZE;

	retval =
	glIsTextureEXT( 
		*(GLuint   *)(pc + 0)
	);
	__GLX_PUT_RETVAL(retval);
	__GLX_BEGIN_REPLY(0);
	__GLX_SEND_HEADER();
	return Success;
}

