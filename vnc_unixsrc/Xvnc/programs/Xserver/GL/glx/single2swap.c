/* $XFree86: xc/programs/Xserver/GL/glx/single2swap.c,v 1.7 2002/01/14 22:47:08 tsi Exp $ */
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
#include "glxutil.h"
#include "glxext.h"
#include "unpack.h"
#include "g_disptab.h"
#include "GL/glx_ansic.h"

int __glXDispSwap_FeedbackBuffer(__GLXclientState *cl, GLbyte *pc)
{
    GLsizei size;
    GLenum type;
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLXcontext *cx;
    int error;

    __GLX_SWAP_INT(&((xGLXSingleReq *)pc)->contextTag);
    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    pc += __GLX_SINGLE_HDR_SIZE;
    __GLX_SWAP_INT(pc+0);
    __GLX_SWAP_INT(pc+4);
    size = *(GLsizei *)(pc+0);
    type = *(GLenum *)(pc+4);
    if (cx->feedbackBufSize < size) {
	cx->feedbackBuf = (GLfloat *) __glXRealloc(cx->feedbackBuf,
						   (size_t) size 
						   * __GLX_SIZE_FLOAT32);
	if (!cx->feedbackBuf) {
	    cl->client->errorValue = size;
	    return BadAlloc;
	}
	cx->feedbackBufSize = size;
    }
    glFeedbackBuffer(size, type, cx->feedbackBuf);
    __GLX_NOTE_UNFLUSHED_CMDS(cx);
    return Success;
}

int __glXDispSwap_SelectBuffer(__GLXclientState *cl, GLbyte *pc)
{
    __GLXcontext *cx;
    GLsizei size;
    __GLX_DECLARE_SWAP_VARIABLES;
    int error;

    __GLX_SWAP_INT(&((xGLXSingleReq *)pc)->contextTag);
    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    pc += __GLX_SINGLE_HDR_SIZE;
    __GLX_SWAP_INT(pc+0);
    size = *(GLsizei *)(pc+0);
    if (cx->selectBufSize < size) {
	cx->selectBuf = (GLuint *) __glXRealloc(cx->selectBuf,
						(size_t) size 
						* __GLX_SIZE_CARD32);
	if (!cx->selectBuf) {
	    cl->client->errorValue = size;
	    return BadAlloc;
	}
	cx->selectBufSize = size;
    }
    glSelectBuffer(size, cx->selectBuf);
    __GLX_NOTE_UNFLUSHED_CMDS(cx);
    return Success;
}

int __glXDispSwap_RenderMode(__GLXclientState *cl, GLbyte *pc)
{
    ClientPtr client;
    __GLXcontext *cx;
    xGLXRenderModeReply reply;
    GLint nitems=0, retBytes=0, retval, newModeCheck;
    GLubyte *retBuffer = NULL;
    GLenum newMode;
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_DECLARE_SWAP_ARRAY_VARIABLES;
    int error;

    __GLX_SWAP_INT(&((xGLXSingleReq *)pc)->contextTag);
    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    pc += __GLX_SINGLE_HDR_SIZE;
    __GLX_SWAP_INT(pc);
    newMode = *(GLenum*) pc;
    retval = glRenderMode(newMode);

    /* Check that render mode worked */
    glGetIntegerv(GL_RENDER_MODE, &newModeCheck);
    if (newModeCheck != newMode) {
	/* Render mode change failed.  Bail */
	newMode = newModeCheck;
	goto noChangeAllowed;
    }

    /*
    ** Render mode might have still failed if we get here.  But in this
    ** case we can't really tell, nor does it matter.  If it did fail, it
    ** will return 0, and thus we won't send any data across the wire.
    */

    switch (cx->renderMode) {
      case GL_RENDER:
	cx->renderMode = newMode;
	break;
      case GL_FEEDBACK:
	if (retval < 0) {
	    /* Overflow happened. Copy the entire buffer */
	    nitems = cx->feedbackBufSize;
	} else {
	    nitems = retval;
	}
	retBytes = nitems * __GLX_SIZE_FLOAT32;
	retBuffer = (GLubyte*) cx->feedbackBuf;
	__GLX_SWAP_FLOAT_ARRAY((GLbyte *)retBuffer, nitems);
	cx->renderMode = newMode;
	break;
      case GL_SELECT:
	if (retval < 0) {
	    /* Overflow happened.  Copy the entire buffer */
	    nitems = cx->selectBufSize;
	} else {
	    GLuint *bp = cx->selectBuf;
	    GLint i;

	    /*
	    ** Figure out how many bytes of data need to be sent.  Parse
	    ** the selection buffer to determine this fact as the
	    ** return value is the number of hits, not the number of
	    ** items in the buffer.
	    */
	    nitems = 0;
	    i = retval;
	    while (--i >= 0) {
		GLuint n;

		/* Parse select data for this hit */
		n = *bp;
		bp += 3 + n;
	    }
	    nitems = bp - cx->selectBuf;
	}
	retBytes = nitems * __GLX_SIZE_CARD32;
	retBuffer = (GLubyte*) cx->selectBuf;
	__GLX_SWAP_INT_ARRAY((GLbyte *)retBuffer, nitems);
	cx->renderMode = newMode;
	break;
    }

    /*
    ** First reply is the number of elements returned in the feedback or
    ** selection array, as per the API for glRenderMode itself.
    */
  noChangeAllowed:;
    client = cl->client;
    reply.length = nitems;
    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;
    reply.retval = retval;
    reply.size = nitems;
    reply.newMode = newMode;
    __GLX_SWAP_SHORT(&reply.sequenceNumber);
    __GLX_SWAP_INT(&reply.length);
    __GLX_SWAP_INT(&reply.retval);
    __GLX_SWAP_INT(&reply.size);
    __GLX_SWAP_INT(&reply.newMode);
    WriteToClient(client, sz_xGLXRenderModeReply, (char *)&reply);
    if (retBytes) {
	WriteToClient(client, retBytes, (char *)retBuffer);
    }
    return Success;
}

int __glXDispSwap_Flush(__GLXclientState *cl, GLbyte *pc)
{
	__GLXcontext *cx;
	int error;
	__GLX_DECLARE_SWAP_VARIABLES;

	__GLX_SWAP_INT(&((xGLXSingleReq *)pc)->contextTag);
	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}

	glFlush();
	__GLX_NOTE_FLUSHED_CMDS(cx);
	return Success;
}

int __glXDispSwap_Finish(__GLXclientState *cl, GLbyte *pc)
{
    __GLXcontext *cx;
    ClientPtr client;
    int error;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT(&((xGLXSingleReq *)pc)->contextTag);
    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    /* Do a local glFinish */
    glFinish();
    __GLX_NOTE_FLUSHED_CMDS(cx);

    /* Send empty reply packet to indicate finish is finished */
    client = cl->client;
    __GLX_BEGIN_REPLY(0);
    __GLX_PUT_RETVAL(0);
    __GLX_SWAP_REPLY_HEADER();
    __GLX_SEND_HEADER();

    return Success;
}

int __glXDispSwap_GetString(__GLXclientState *cl, GLbyte *pc)
{
    return DoGetString(cl, pc, GL_TRUE);
}

int __glXDispSwap_GetClipPlane(__GLXclientState *cl, GLbyte *pc)
{
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    GLdouble answer[4];
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_DECLARE_SWAP_ARRAY_VARIABLES;

    __GLX_SWAP_INT(&((xGLXSingleReq *)pc)->contextTag);
    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }
    pc += __GLX_SINGLE_HDR_SIZE;
    __GLX_SWAP_INT(pc + 0);

    __glXClearErrorOccured();
    glGetClipPlane(*(GLenum   *)(pc + 0), answer);
    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SWAP_REPLY_HEADER();
	__GLX_SEND_HEADER();
    } else {
	__GLX_SWAP_DOUBLE_ARRAY((GLbyte *)answer, 4);
	__GLX_BEGIN_REPLY(32);
	__GLX_SWAP_REPLY_HEADER();
	__GLX_SEND_HEADER();
	__GLX_SEND_DOUBLE_ARRAY(4);
    }
    return Success;
}
