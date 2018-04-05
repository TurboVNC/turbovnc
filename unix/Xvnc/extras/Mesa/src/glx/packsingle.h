#ifndef __GLX_packsingle_h__
#define __GLX_packsingle_h__

/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice including the dates of first publication and
 * either this permission notice or a reference to
 * http://oss.sgi.com/projects/FreeB/
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * SILICON GRAPHICS, INC. BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of Silicon Graphics, Inc.
 * shall not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization from
 * Silicon Graphics, Inc.
 */

#include "packrender.h"

/*
** The macros in this header convert wire protocol data types to the client
** machine's native data types.  The header is part of the porting layer of
** the client library, and it is intended that hardware vendors will rewrite
** this header to suit their own machines.
*/

/*
** Dummy define to make the GetReqExtra macro happy.  The value is not
** used, but instead the code in __GLX_SINGLE_BEGIN issues its own store
** to req->reqType with the proper code (our extension code).
*/
#define X_GLXSingle 0

/* Declare common variables used during a single command */
#define __GLX_SINGLE_DECLARE_VARIABLES()         \
   struct glx_context *gc = __glXGetCurrentContext();  \
   GLubyte *pc, *pixelHeaderPC;                  \
   GLuint compsize, cmdlen;                      \
   Display *dpy = gc->currentDpy;                \
   xGLXSingleReq *req

#define __GLX_SINGLE_LOAD_VARIABLES()           \
   pc = gc->pc;                                 \
   /* Muffle compilers */                       \
   pixelHeaderPC = 0;  (void)pixelHeaderPC;     \
   compsize = 0;       (void)compsize;          \
   cmdlen = 0;         (void)cmdlen

/* Start a single command */
#define __GLX_SINGLE_BEGIN(opcode,bytes)        \
   if (dpy) {                                   \
   (void) __glXFlushRenderBuffer(gc, pc);       \
   LockDisplay(dpy);                            \
   GetReqExtra(GLXSingle,bytes,req);            \
   req->reqType = gc->majorOpcode;              \
   req->glxCode = opcode;                       \
   req->contextTag = gc->currentContextTag;     \
   pc = ((GLubyte *)(req) + sz_xGLXSingleReq)

/* End a single command */
#define __GLX_SINGLE_END()       \
   UnlockDisplay(dpy);           \
   SyncHandle();                 \
   }

/* Store data to sending for a single command */
#define __GLX_SINGLE_PUT_CHAR(offset,a)         \
   *((INT8 *) (pc + offset)) = a

#define __GLX_SINGLE_PUT_SHORT(offset,a)        \
   *((INT16 *) (pc + offset)) = a

#define __GLX_SINGLE_PUT_LONG(offset,a)         \
   *((INT32 *) (pc + offset)) = a

#define __GLX_SINGLE_PUT_FLOAT(offset,a)        \
   *((FLOAT32 *) (pc + offset)) = a

/* Read support macros */
#define __GLX_SINGLE_READ_XREPLY()                    \
   (void) _XReply(dpy, (xReply*) &reply, 0, False)

#define __GLX_SINGLE_GET_RETVAL(a,cast)         \
   a = (cast) reply.retval

#define __GLX_SINGLE_GET_SIZE(a)                \
   a = (GLint) reply.size

#define __GLX_SINGLE_GET_CHAR(p)                \
   memcpy((p), &reply.pad3, 1);

#define __GLX_SINGLE_GET_SHORT(p)               \
   memcpy((p), &reply.pad3, 2);

#define __GLX_SINGLE_GET_LONG(p)                \
   memcpy((p), &reply.pad3, 4);

#define __GLX_SINGLE_GET_FLOAT(p)               \
   memcpy((p), &reply.pad3, 4);

#define __GLX_SINGLE_GET_DOUBLE(p)              \
   memcpy((p), &reply.pad3, 8);

/* Get an array of typed data */
#define __GLX_SINGLE_GET_VOID_ARRAY(a,alen)     \
   {                                            \
      GLint slop = alen*__GLX_SIZE_INT8 & 3;    \
      _XRead(dpy,(char *)a,alen*__GLX_SIZE_INT8);  \
      if (slop) _XEatData(dpy,4-slop);             \
   }

#define __GLX_SINGLE_GET_CHAR_ARRAY(a,alen)     \
   {                                            \
      GLint slop = alen*__GLX_SIZE_INT8 & 3;    \
      _XRead(dpy,(char *)a,alen*__GLX_SIZE_INT8);  \
      if (slop) _XEatData(dpy,4-slop);             \
   }


#define __GLX_SINGLE_GET_SHORT_ARRAY(a,alen)    \
   {                                            \
      GLint slop = (alen*__GLX_SIZE_INT16) & 3;    \
      _XRead(dpy,(char *)a,alen*__GLX_SIZE_INT16); \
      if (slop) _XEatData(dpy,4-slop);             \
   }

#define __GLX_SINGLE_GET_LONG_ARRAY(a,alen)        \
   _XRead(dpy,(char *)a,alen*__GLX_SIZE_INT32);

#define __GLX_SINGLE_GET_FLOAT_ARRAY(a,alen)       \
   _XRead(dpy,(char *)a,alen*__GLX_SIZE_FLOAT32);

#define __GLX_SINGLE_GET_DOUBLE_ARRAY(a,alen)      \
   _XRead(dpy,(char *)a,alen*__GLX_SIZE_FLOAT64);

#endif /* !__GLX_packsingle_h__ */
