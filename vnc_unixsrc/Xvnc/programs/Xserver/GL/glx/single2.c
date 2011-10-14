/* $XFree86: xc/programs/Xserver/GL/glx/single2.c,v 1.8 2004/02/10 22:54:15 alanh Exp $ */
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

int __glXDisp_FeedbackBuffer(__GLXclientState *cl, GLbyte *pc)
{
    GLsizei size;
    GLenum type;
    __GLXcontext *cx;
    int error;

    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    pc += __GLX_SINGLE_HDR_SIZE;
    size = *(GLsizei *)(pc+0);
    type = *(GLenum *)(pc+4);
    if (cx->feedbackBufSize < size) {
	cx->feedbackBuf = (GLfloat *) __glXRealloc(cx->feedbackBuf,
						   (size_t)size 
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

int __glXDisp_SelectBuffer(__GLXclientState *cl, GLbyte *pc)
{
    __GLXcontext *cx;
    GLsizei size;
    int error;

    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    pc += __GLX_SINGLE_HDR_SIZE;
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

int __glXDisp_RenderMode(__GLXclientState *cl, GLbyte *pc)
{
    ClientPtr client;
    xGLXRenderModeReply reply;
    __GLXcontext *cx;
    GLint nitems=0, retBytes=0, retval, newModeCheck;
    GLubyte *retBuffer = NULL;
    GLenum newMode;
    int error;

    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    pc += __GLX_SINGLE_HDR_SIZE;
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
    WriteToClient(client, sz_xGLXRenderModeReply, (char *)&reply);
    if (retBytes) {
	WriteToClient(client, retBytes, (char *)retBuffer);
    }
    return Success;
}

int __glXDisp_Flush(__GLXclientState *cl, GLbyte *pc)
{
	__GLXcontext *cx;
	int error;

	cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
	if (!cx) {
		return error;
	}

	glFlush();
	__GLX_NOTE_FLUSHED_CMDS(cx);
	return Success;
}

int __glXDisp_Finish(__GLXclientState *cl, GLbyte *pc)
{
    __GLXcontext *cx;
    ClientPtr client;
    int error;

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
    __GLX_SEND_HEADER();
    return Success;
}

#define SEPARATOR " "

char *__glXcombine_strings(const char *cext_string, const char *sext_string)
{
   size_t clen, slen;
   char *combo_string, *token, *s1;
   const char *s2, *end;

   /* safeguard to prevent potentially fatal errors in the string functions */
   if (!cext_string)
      cext_string = "";
   if (!sext_string)
      sext_string = "";

   /*
   ** String can't be longer than min(cstring, sstring)
   ** pull tokens out of shortest string
   ** include space in combo_string for final separator and null terminator
   */
   if ( (clen = __glXStrlen(cext_string)) > (slen = __glXStrlen(sext_string)) ) {
	combo_string = (char *) __glXMalloc(slen + 2);
	s1 = (char *) __glXMalloc(slen + 2); __glXStrcpy(s1, sext_string);
	s2 = cext_string;
   } else {
	combo_string = (char *) __glXMalloc(clen + 2);
	s1 = (char *) __glXMalloc(clen + 2); __glXStrcpy(s1, cext_string);
	s2 = sext_string;
   }
   if (!combo_string || !s1) {
	if (combo_string) __glXFree(combo_string);
	if (s1) __glXFree(s1);
	return NULL;
   }
   combo_string[0] = '\0';

   /* Get first extension token */
   token = __glXStrtok( s1, SEPARATOR);
   while ( token != NULL ) {

	/*
	** if token in second string then save it
	** beware of extension names which are prefixes of other extension names
	*/
	const char *p = s2;
	end = p + __glXStrlen(p);
	while (p < end) {
	    size_t n = __glXStrcspn(p, SEPARATOR);
	    if ((__glXStrlen(token) == n) && (__glXStrncmp(token, p, n) == 0)) {
		combo_string = __glXStrcat( combo_string, token);
		combo_string = __glXStrcat( combo_string, SEPARATOR);
	    }
	    p += (n + 1);
	}

	/* Get next extension token */
	token = __glXStrtok( NULL, SEPARATOR);
   }
   __glXFree(s1);
   return combo_string;
}

int DoGetString(__GLXclientState *cl, GLbyte *pc, GLboolean need_swap)
{
    ClientPtr client;
    __GLXcontext *cx;
    GLenum name;
    const char *string;
    __GLX_DECLARE_SWAP_VARIABLES;
    int error;
    char *buf = NULL, *buf1 = NULL;
    GLint length = 0;

    /* If the client has the opposite byte order, swap the contextTag and
     * the name.
     */
    if ( need_swap ) {
	__GLX_SWAP_INT(pc + 4);
	__GLX_SWAP_INT(pc + __GLX_SINGLE_HDR_SIZE);
    }

    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    pc += __GLX_SINGLE_HDR_SIZE;
    name = *(GLenum *)(pc + 0);
    string = (const char *)glGetString(name);
    client = cl->client;

    /*
    ** Restrict extensions to those that are supported by both the
    ** implementation and the connection.  That is, return the
    ** intersection of client, server, and core extension strings.
    */
    if (name == GL_EXTENSIONS) {
	buf1 = __glXcombine_strings(string,
				      cl->GLClientextensions);
	buf = __glXcombine_strings(buf1,
				      cx->pGlxScreen->GLextensions);
	if (buf1 != NULL) {
	    __glXFree(buf1);
	}
	string = buf;
    }
    else if ( name == GL_VERSION ) {
	if ( atof( string ) > atof( GLServerVersion ) ) {
	    buf = __glXMalloc( __glXStrlen( string ) 
			       + __glXStrlen( GLServerVersion )
			       + 4 );
	    if ( buf == NULL ) {
		string = GLServerVersion;
	    }
	    else {
		__glXSprintf( buf, "%s (%s)", GLServerVersion, string );
		string = buf;
	    }
	}
    }
    if (string) {
	length = __glXStrlen((const char *) string) + 1;
    }

    __GLX_BEGIN_REPLY(length);
    __GLX_PUT_SIZE(length);

    if ( need_swap ) {
	__GLX_SWAP_REPLY_SIZE();
	__GLX_SWAP_REPLY_HEADER();
    }

    __GLX_SEND_HEADER();
    WriteToClient(client, length, (char *) string); 
    if (buf != NULL) {
	__glXFree(buf);
    }
    return Success;
}

int __glXDisp_GetString(__GLXclientState *cl, GLbyte *pc)
{
    return DoGetString(cl, pc, GL_FALSE);
}

int __glXDisp_GetClipPlane(__GLXclientState *cl, GLbyte *pc)
{
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    GLdouble answer[4];

    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }
    pc += __GLX_SINGLE_HDR_SIZE;

    __glXClearErrorOccured();
    glGetClipPlane(*(GLenum   *)(pc + 0), answer);
    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SEND_HEADER();
    } else {
	__GLX_BEGIN_REPLY(32);
	__GLX_SEND_HEADER();
	__GLX_SEND_DOUBLE_ARRAY(4);
    }
    return Success;
}

