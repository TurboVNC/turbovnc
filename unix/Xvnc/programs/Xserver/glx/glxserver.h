#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifndef _GLX_server_h_
#define _GLX_server_h_

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

#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/Xmd.h>
#include <misc.h>
#include <dixstruct.h>
#include <pixmapstr.h>
#include <gcstruct.h>
#include <extnsionst.h>
#include <resource.h>
#include <scrnintstr.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glxproto.h>

/*
** GLX resources.
*/
typedef XID GLXContextID;
typedef XID GLXPixmap;
typedef XID GLXDrawable;

typedef struct __GLXclientStateRec __GLXclientState;
typedef struct __GLXdrawable __GLXdrawable;
typedef struct __GLXcontext __GLXcontext;

#include "glxscreens.h"
#include "glxdrawable.h"
#include "glxcontext.h"

#ifndef True
#define True 1
#endif
#ifndef False
#define False 0
#endif

extern __GLXscreen *glxGetScreen(ScreenPtr pScreen);
extern __GLXclientState *glxGetClient(ClientPtr pClient);

/************************************************************************/

void GlxSetVisualConfigs(int nconfigs, void *configs, void **privates);

void __glXScreenInitVisuals(__GLXscreen * screen);

/*
** The last context used (from the server's persective) is cached.
*/
extern __GLXcontext *__glXForceCurrent(__GLXclientState *, GLXContextTag,
                                       int *);

int __glXError(int error);

/************************************************************************/

typedef struct __GLXprovider __GLXprovider;
struct __GLXprovider {
    __GLXscreen *(*screenProbe) (ScreenPtr pScreen);
    const char *name;
    __GLXprovider *next;
};
extern __GLXprovider __glXDRISWRastProvider;

void GlxPushProvider(__GLXprovider * provider);

enum {
    GLX_MINIMAL_VISUALS,
    GLX_TYPICAL_VISUALS,
    GLX_ALL_VISUALS
};

void __glXsetEnterLeaveServerFuncs(void (*enter) (GLboolean),
                                   void (*leave) (GLboolean));
void __glXenterServer(GLboolean rendering);
void __glXleaveServer(GLboolean rendering);

void glxSuspendClients(void);
void glxResumeClients(void);

typedef void (*glx_func_ptr)(void);
typedef glx_func_ptr (*glx_gpa_proc)(const char *);
void __glXsetGetProcAddress(glx_gpa_proc get_proc_address);
void *__glGetProcAddress(const char *);

void
__glXsendSwapEvent(__GLXdrawable *drawable, int type, CARD64 ust,
                   CARD64 msc, CARD32 sbc);

#if PRESENT
void
__glXregisterPresentCompleteNotify(void);
#endif

/*
** State kept per client.
*/
struct __GLXclientStateRec {
    /*
     ** Whether this structure is currently being used to support a client.
     */
    Bool inUse;

    /*
     ** Buffer for returned data.
     */
    GLbyte *returnBuf;
    GLint returnBufSize;

    /*
     ** Keep track of large rendering commands, which span multiple requests.
     */
    GLint largeCmdBytesSoFar;   /* bytes received so far        */
    GLint largeCmdBytesTotal;   /* total bytes expected         */
    GLint largeCmdRequestsSoFar;        /* requests received so far     */
    GLint largeCmdRequestsTotal;        /* total requests expected      */
    GLbyte *largeCmdBuf;
    GLint largeCmdBufSize;

    /* Back pointer to X client record */
    ClientPtr client;

    char *GLClientextensions;
};

/************************************************************************/

/*
** Dispatch tables.
*/
typedef void (*__GLXdispatchRenderProcPtr) (GLbyte *);
typedef int (*__GLXdispatchSingleProcPtr) (__GLXclientState *, GLbyte *);
typedef int (*__GLXdispatchVendorPrivProcPtr) (__GLXclientState *, GLbyte *);

/*
 * Dispatch for GLX commands.
 */
typedef int (*__GLXprocPtr) (__GLXclientState *, char *pc);

/*
 * Tables for computing the size of each rendering command.
 */
typedef int (*gl_proto_size_func) (const GLbyte *, Bool, int);

typedef struct {
    int bytes;
    gl_proto_size_func varsize;
} __GLXrenderSizeData;

/************************************************************************/

/*
** X resources.
*/
extern RESTYPE __glXContextRes;
extern RESTYPE __glXClientRes;
extern RESTYPE __glXPixmapRes;
extern RESTYPE __glXDrawableRes;

/************************************************************************/

/*
** Prototypes.
*/

extern char *__glXcombine_strings(const char *, const char *);

/*
** Routines for sending swapped replies.
*/

extern void __glXSwapMakeCurrentReply(ClientPtr client,
                                      xGLXMakeCurrentReply * reply);
extern void __glXSwapIsDirectReply(ClientPtr client, xGLXIsDirectReply * reply);
extern void __glXSwapQueryVersionReply(ClientPtr client,
                                       xGLXQueryVersionReply * reply);
extern void __glXSwapQueryContextInfoEXTReply(ClientPtr client,
                                              xGLXQueryContextInfoEXTReply *
                                              reply, int *buf);
extern void __glXSwapGetDrawableAttributesReply(ClientPtr client,
                                                xGLXGetDrawableAttributesReply *
                                                reply, CARD32 *buf);
extern void glxSwapQueryExtensionsStringReply(ClientPtr client,
                                              xGLXQueryExtensionsStringReply *
                                              reply, char *buf);
extern void glxSwapQueryServerStringReply(ClientPtr client,
                                          xGLXQueryServerStringReply * reply,
                                          char *buf);

/*
 * Routines for computing the size of variably-sized rendering commands.
 */

static _X_INLINE int
safe_add(int a, int b)
{
    if (a < 0 || b < 0)
        return -1;

    if (INT_MAX - a < b)
        return -1;

    return a + b;
}

static _X_INLINE int
safe_mul(int a, int b)
{
    if (a < 0 || b < 0)
        return -1;

    if (a == 0 || b == 0)
        return 0;

    if (a > INT_MAX / b)
        return -1;

    return a * b;
}

static _X_INLINE int
safe_pad(int a)
{
    int ret;

    if (a < 0)
        return -1;

    if ((ret = safe_add(a, 3)) < 0)
        return -1;

    return ret & (GLuint)~3;
}

extern int __glXTypeSize(GLenum enm);
extern int __glXImageSize(GLenum format, GLenum type,
                          GLenum target, GLsizei w, GLsizei h, GLsizei d,
                          GLint imageHeight, GLint rowLength, GLint skipImages,
                          GLint skipRows, GLint alignment);

extern unsigned glxMajorVersion;
extern unsigned glxMinorVersion;

extern int __glXEventBase;

#endif                          /* !__GLX_server_h__ */
