/* $XFree86: xc/programs/Xserver/GL/glx/glxext.c,v 1.9 2003/09/28 20:15:43 alanh Exp $
** The contents of this file are subject to the GLX Public License Version 1.0
** (the "License"). You may not use this file except in compliance with the
** License. You may obtain a copy of the License at Silicon Graphics, Inc.,
** attn: Legal Services, 2011 N. Shoreline Blvd., Mountain View, CA 94043
** or at http://www.sgi.com/software/opensource/glx/license.html.
**
** Software distributed under the License is distributed on an "AS IS"
** basis. ALL WARRANTIES ARE DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY
** IMPLIED WARRANTIES OF MERCHANTABILITY, OF FITNESS FOR A PARTICULAR
** PURPOSE OR OF NON- INFRINGEMENT. See the License for the specific
** language governing rights and limitations under the License.
**
** The Original Software is GLX version 1.2 source code, released February,
** 1999. The developer of the Original Software is Silicon Graphics, Inc.
** Those portions of the Subject Software created by Silicon Graphics, Inc.
** are Copyright (c) 1991-9 Silicon Graphics, Inc. All Rights Reserved.
**
*/

#define NEED_REPLIES
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "glxserver.h"
#include <windowstr.h>
#include <propertyst.h>
#include <os.h>
#include "g_disptab.h"
#include "unpack.h"
#include "glxutil.h"
#include "glxext.h"
#include "micmap.h"


void GlxWrapInitVisuals(miInitVisualsProcPtr *);
void GlxSetVisualConfigs(int nconfigs, 
                         __GLXvisualConfig *configs, void **privates);

static __GLXextensionInfo *__glXExt /* = &__glDDXExtensionInfo */;

/*
** Forward declarations.
*/
static int __glXSwapDispatch(ClientPtr);
static int __glXDispatch(ClientPtr);

/*
** Called when the extension is reset.
*/
static void ResetExtension(ExtensionEntry* extEntry)
{
    __glXFlushContextCache();
    (*__glXExt->resetExtension)();
    __glXScreenReset();
}

/*
** Initialize the per-client context storage.
*/
static void ResetClientState(int clientIndex)
{
    __GLXclientState *cl = __glXClients[clientIndex];

    if (cl->returnBuf) __glXFree(cl->returnBuf);
    if (cl->largeCmdBuf) __glXFree(cl->largeCmdBuf);
    if (cl->currentContexts) __glXFree(cl->currentContexts);
    __glXMemset(cl, 0, sizeof(__GLXclientState));
    /*
    ** By default, assume that the client supports
    ** GLX major version 1 minor version 0 protocol.
    */
    cl->GLClientmajorVersion = 1;
    cl->GLClientminorVersion = 0;
    if (cl->GLClientextensions) __glXFree(cl->GLClientextensions);

}

/*
** Reset state used to keep track of large (multi-request) commands.
*/
void __glXResetLargeCommandStatus(__GLXclientState *cl)
{
    cl->largeCmdBytesSoFar = 0;
    cl->largeCmdBytesTotal = 0;
    cl->largeCmdRequestsSoFar = 0;
    cl->largeCmdRequestsTotal = 0;
}

/*
** This procedure is called when the client who created the context goes
** away OR when glXDestroyContext is called.  In either case, all we do is
** flag that the ID is no longer valid, and (maybe) free the context.
** use.
*/
static int ContextGone(__GLXcontext* cx, XID id)
{
    cx->idExists = GL_FALSE;
    if (!cx->isCurrent) {
	__glXFreeContext(cx);
    }

    return True;
}

/*
** Free a client's state.
*/
static int ClientGone(int clientIndex, XID id)
{
    __GLXcontext *cx;
    __GLXclientState *cl = __glXClients[clientIndex];
    int i;

    if (cl) {
	/*
	** Free all the contexts that are current for this client.
	*/
	for (i=0; i < cl->numCurrentContexts; i++) {
	    cx = cl->currentContexts[i];
	    if (cx) {
		__glXDeassociateContext(cx);
		cx->isCurrent = GL_FALSE;
		if (!cx->idExists) {
		    __glXFreeContext(cx);
		}
	    }
	}
	/*
	** Re-initialize the client state structure.  Don't free it because
	** we'll probably get another client with this index and use the struct
	** again.  There is a maximum of MAXCLIENTS of these structures.
	*/
	ResetClientState(clientIndex);
    }

    return True;
}

/*
** Free a GLX Pixmap.
*/
static int PixmapGone(__GLXpixmap *pGlxPixmap, XID id)
{
    PixmapPtr pPixmap = (PixmapPtr) pGlxPixmap->pDraw;

    pGlxPixmap->idExists = False;
    if (!pGlxPixmap->refcnt) {
	/*
	** The DestroyPixmap routine should decrement the refcount and free
	** only if it's zero.
	*/
	(*pGlxPixmap->pScreen->DestroyPixmap)(pPixmap);
	__glXFree(pGlxPixmap);
    }

    return True;
}

/*
** Free a context.
*/
GLboolean __glXFreeContext(__GLXcontext *cx)
{
    if (cx->idExists || cx->isCurrent) return GL_FALSE;
    
    if (!cx->isDirect) {
	if ((*cx->gc->exports.destroyContext)((__GLcontext *)cx->gc) == GL_FALSE) {
	    return GL_FALSE;
	}
    }
    if (cx->feedbackBuf) __glXFree(cx->feedbackBuf);
    if (cx->selectBuf) __glXFree(cx->selectBuf);
    __glXFree(cx);
    if (cx == __glXLastContext) {
	__glXFlushContextCache();
    }

    return GL_TRUE;
}

extern RESTYPE __glXSwapBarrierRes;

static int SwapBarrierGone(int screen, XID drawable)
{
    if (__glXSwapBarrierFuncs &&
        __glXSwapBarrierFuncs[screen].bindSwapBarrierFunc != NULL) {
        __glXSwapBarrierFuncs[screen].bindSwapBarrierFunc(screen, drawable, 0);
    }
    FreeResourceByType(drawable, __glXSwapBarrierRes, FALSE);
    return True;
}

/************************************************************************/

/*
** These routines can be used to check whether a particular GL command
** has caused an error.  Specifically, we use them to check whether a
** given query has caused an error, in which case a zero-length data
** reply is sent to the client.
*/

static GLboolean errorOccured = GL_FALSE;

/*
** The GL was will call this routine if an error occurs.
*/
void __glXErrorCallBack(__GLinterface *gc, GLenum code)
{
    errorOccured = GL_TRUE;
}

/*
** Clear the error flag before calling the GL command.
*/
void __glXClearErrorOccured(void)
{
    errorOccured = GL_FALSE;
}

/*
** Check if the GL command caused an error.
*/
GLboolean __glXErrorOccured(void)
{
    return errorOccured;
}

/************************************************************************/

/*
** Initialize the GLX extension.
*/
void GlxExtensionInit(void)
{
    ExtensionEntry *extEntry;
    int i;
    
    __glXContextRes = CreateNewResourceType((DeleteType)ContextGone);
    __glXClientRes = CreateNewResourceType((DeleteType)ClientGone);
    __glXPixmapRes = CreateNewResourceType((DeleteType)PixmapGone);

    /*
    ** Add extension to server extensions.
    */
    extEntry = AddExtension(GLX_EXTENSION_NAME, __GLX_NUMBER_EVENTS,
			    __GLX_NUMBER_ERRORS, __glXDispatch,
			    __glXSwapDispatch, ResetExtension,
			    StandardMinorOpcode);
    if (!extEntry) {
	FatalError("__glXExtensionInit: AddExtensions failed\n");
	return;
    }
    if (!AddExtensionAlias(GLX_EXTENSION_ALIAS, extEntry)) {
	ErrorF("__glXExtensionInit: AddExtensionAlias failed\n");
	return;
    }

    __glXBadContext = extEntry->errorBase + GLXBadContext;
    __glXBadContextState = extEntry->errorBase + GLXBadContextState;
    __glXBadDrawable = extEntry->errorBase + GLXBadDrawable;
    __glXBadPixmap = extEntry->errorBase + GLXBadPixmap;
    __glXBadContextTag = extEntry->errorBase + GLXBadContextTag;
    __glXBadCurrentWindow = extEntry->errorBase + GLXBadCurrentWindow;
    __glXBadRenderRequest = extEntry->errorBase + GLXBadRenderRequest;
    __glXBadLargeRequest = extEntry->errorBase + GLXBadLargeRequest;
    __glXUnsupportedPrivateRequest = extEntry->errorBase +
      			GLXUnsupportedPrivateRequest;

    __glXSwapBarrierRes = CreateNewResourceType((DeleteType)SwapBarrierGone);

    /*
    ** Initialize table of client state.  There is never a client 0.
    */
    for (i=1; i <= MAXCLIENTS; i++) {
	__glXClients[i] = 0;
    }

    /*
    ** Initialize screen specific data.
    */
    __glXScreenInit(screenInfo.numScreens);
}

/************************************************************************/

Bool __glXCoreType(void)
{
    return __glXExt->type;
}

/************************************************************************/

void GlxSetVisualConfigs(int nconfigs, 
                         __GLXvisualConfig *configs, void **privates)
{
    (*__glXExt->setVisualConfigs)(nconfigs, configs, privates);
}

static miInitVisualsProcPtr saveInitVisualsProc;

Bool GlxInitVisuals(VisualPtr *visualp, DepthPtr *depthp,
		    int *nvisualp, int *ndepthp,
		    int *rootDepthp, VisualID *defaultVisp,
		    unsigned long sizes, int bitsPerRGB,
		    int preferredVis)
{
    Bool ret;

    if (saveInitVisualsProc) {
        ret = saveInitVisualsProc(visualp, depthp, nvisualp, ndepthp,
                                  rootDepthp, defaultVisp, sizes, bitsPerRGB,
                                  preferredVis);
        if (!ret)
            return False;
    }
    (*__glXExt->initVisuals)(visualp, depthp, nvisualp, ndepthp, rootDepthp,
                             defaultVisp, sizes, bitsPerRGB);
    return True;
}

void
GlxWrapInitVisuals(miInitVisualsProcPtr *initVisProc)
{
    saveInitVisualsProc = *initVisProc;
    *initVisProc = GlxInitVisuals;
    /* HACK: this shouldn't be done here but it's the earliest time */
    __glXExt = __glXglDDXExtensionInfo();       /* from GLcore */
}

/************************************************************************/

void __glXFlushContextCache(void)
{
    __glXLastContext = 0;
}

/*
** Make a context the current one for the GL (in this implementation, there
** is only one instance of the GL, and we use it to serve all GL clients by
** switching it between different contexts).  While we are at it, look up
** a context by its tag and return its (__GLXcontext *).
*/
__GLXcontext *__glXForceCurrent(__GLXclientState *cl, GLXContextTag tag,
				int *error)
{
    __GLXcontext *cx;

    /*
    ** See if the context tag is legal; it is managed by the extension,
    ** so if it's invalid, we have an implementation error.
    */
    cx = (__GLXcontext *) __glXLookupContextByTag(cl, tag);
    if (!cx) {
	cl->client->errorValue = tag;
	*error = __glXBadContextTag;
	return 0;
    }

    if (!cx->isDirect) {
	if (cx->drawPriv == NULL) {
	    /*
	    ** The drawable has vanished.  It must be a window, because only
	    ** windows can be destroyed from under us; GLX pixmaps are
	    ** refcounted and don't go away until no one is using them.
	    */
	    *error = __glXBadCurrentWindow;
	    return 0;
    	}
    }
    
    if (cx == __glXLastContext) {
	/* No need to re-bind */
	return cx;
    }

    /* Make this context the current one for the GL. */
    if (!cx->isDirect) {
	if (!(*cx->gc->exports.forceCurrent)((__GLcontext *)cx->gc)) {
	    /* Bind failed, and set the error code.  Bummer */
	    cl->client->errorValue = cx->id;
	    *error = __glXBadContextState;
	    return 0;
    	}
    }
    __glXLastContext = cx;
    return cx;
}

/************************************************************************/

/*
** Top level dispatcher; all commands are executed from here down.
*/
static int __glXDispatch(ClientPtr client)
{
    REQUEST(xGLXSingleReq);
    CARD8 opcode;
    int (*proc)(__GLXclientState *cl, GLbyte *pc);
    __GLXclientState *cl;

    opcode = stuff->glxCode;
    cl = __glXClients[client->index];
    if (!cl) {
	cl = (__GLXclientState *) __glXMalloc(sizeof(__GLXclientState));
	 __glXClients[client->index] = cl;
	if (!cl) {
	    return BadAlloc;
	}
	__glXMemset(cl, 0, sizeof(__GLXclientState));
    }
    
    if (!cl->inUse) {
	/*
	** This is first request from this client.  Associate a resource
	** with the client so we will be notified when the client dies.
	*/
	XID xid = FakeClientID(client->index);
	if (!AddResource( xid, __glXClientRes, (pointer)(long)client->index)) {
	    return BadAlloc;
	}
	ResetClientState(client->index);
	cl->inUse = GL_TRUE;
	cl->client = client;
    }

    /*
    ** Check for valid opcode.
    */
    if (opcode >= __GLX_SINGLE_TABLE_SIZE) {
	return BadRequest;
    }

    /*
    ** If we're expecting a glXRenderLarge request, this better be one.
    */
    if ((cl->largeCmdRequestsSoFar != 0) && (opcode != X_GLXRenderLarge)) {
	client->errorValue = stuff->glxCode;
	return __glXBadLargeRequest;
    }

    /*
    ** Use the opcode to index into the procedure table.
    */
    proc = __glXSingleTable[opcode];
    return (*proc)(cl, (GLbyte *) stuff);
}

static int __glXSwapDispatch(ClientPtr client)
{
    REQUEST(xGLXSingleReq);
    CARD8 opcode;
    int (*proc)(__GLXclientState *cl, GLbyte *pc);
    __GLXclientState *cl;

    opcode = stuff->glxCode;
    cl = __glXClients[client->index];
    if (!cl) {
	cl = (__GLXclientState *) __glXMalloc(sizeof(__GLXclientState));
	 __glXClients[client->index] = cl;
	if (!cl) {
	    return BadAlloc;
	}
	__glXMemset(cl, 0, sizeof(__GLXclientState));
    }
    
    if (!cl->inUse) {
	/*
	** This is first request from this client.  Associate a resource
	** with the client so we will be notified when the client dies.
	*/
	XID xid = FakeClientID(client->index);
	if (!AddResource( xid, __glXClientRes, (pointer)(long)client->index)) {
	    return BadAlloc;
	}
	ResetClientState(client->index);
	cl->inUse = GL_TRUE;
	cl->client = client;
    }

    /*
    ** Check for valid opcode.
    */
    if (opcode >= __GLX_SINGLE_TABLE_SIZE) {
	return BadRequest;
    }

    /*
    ** Use the opcode to index into the procedure table.
    */
    proc = __glXSwapSingleTable[opcode];
    return (*proc)(cl, (GLbyte *) stuff);
}

int __glXNoSuchSingleOpcode(__GLXclientState *cl, GLbyte *pc)
{
    return BadRequest;
}

void __glXNoSuchRenderOpcode(GLbyte *pc)
{
    return;
}

