/* $XFree86: xc/programs/Xserver/GL/glx/glxcmds.c,v 1.12 2004/01/28 18:11:50 alanh Exp $ */
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
#define FONT_PCF
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "glxserver.h"
#include <GL/glxtokens.h>
#include <unpack.h>
#include "g_disptab.h"
#include <pixmapstr.h>
#include <windowstr.h>
#include "g_disptab_EXT.h"
#include "glximports.h"
#include "glxutil.h"
#include "glxext.h"
#include "GL/glx_ansic.h"
#include "glcontextmodes.h"

/************************************************************************/

static __GLimports imports = {
    __glXImpMalloc,
    __glXImpCalloc,
    __glXImpRealloc,
    __glXImpFree,
    __glXImpWarning,
    __glXImpFatal,
    __glXImpGetenv,
    __glXImpAtoi,
    __glXImpSprintf,
    __glXImpFopen,
    __glXImpFclose,
    __glXImpFprintf,
    __glXImpGetDrawablePrivate,
    __glXImpGetReadablePrivate,
    NULL
};

static int __glXGetFBConfigsSGIX(__GLXclientState *cl, GLbyte *pc);
static int __glXCreateContextWithConfigSGIX(__GLXclientState *cl, GLbyte *pc);
static int __glXCreateGLXPixmapWithConfigSGIX(__GLXclientState *cl, GLbyte *pc);
static int __glXMakeCurrentReadSGI(__GLXclientState *cl, GLbyte *pc);

static int __glXBindSwapBarrierSGIX(__GLXclientState *cl, GLbyte *pc);
static int __glXQueryMaxSwapBarriersSGIX(__GLXclientState *cl, GLbyte *pc);
static int __glxQueryHyperpipeNetworkSGIX(__GLXclientState *cl, GLbyte *pc);
static int __glxDestroyHyperpipeConfigSGIX (__GLXclientState *cl, GLbyte *pc);
static int __glxQueryHyperpipeConfigSGIX(__GLXclientState *cl, GLbyte *pc);
static int __glxHyperpipeConfigSGIX(__GLXclientState *cl, GLbyte *pc);


/************************************************************************/

/**
 * Create a GL context with the given properties.  This routine is used
 * to implement \c glXCreateContext, \c glXCreateNewContext, and
 * \c glXCreateContextWithConfigSGIX.  This works becuase of the hack way
 * that GLXFBConfigs are implemented.  Basically, the FBConfigID is the
 * same as the VisualID.
 */

int DoCreateContext(__GLXclientState *cl, GLXContextID gcId,
		    GLXContextID shareList, VisualID visual,
		    GLuint screen, GLboolean isDirect)
{
    ClientPtr client = cl->client;
    VisualPtr pVisual;
    ScreenPtr pScreen;
    __GLXcontext *glxc, *shareglxc;
    __GLcontextModes *modes;
    __GLXscreenInfo *pGlxScreen;
    __GLinterface *shareGC;
    GLint i;

    LEGAL_NEW_RESOURCE(gcId, client);
    
    /*
    ** Check if screen exists.
    */
    if (screen >= screenInfo.numScreens) {
	client->errorValue = screen;
	return BadValue;
    }
    pScreen = screenInfo.screens[screen];
    pGlxScreen = &__glXActiveScreens[screen];
    
    /*
    ** Check if the visual ID is valid for this screen.
    */
    pVisual = pScreen->visuals;
    for (i = 0; i < pScreen->numVisuals; i++, pVisual++) {
	if (pVisual->vid == visual) {
	    break;
	}
    }
    if (i == pScreen->numVisuals) {
	client->errorValue = visual;
	return BadValue;
    }

    /*
    ** Get configuration of the visual.  This assumes that the
    ** glXActiveScreens structure contains visual configurations only for the
    ** subset of Visuals that are supported by this implementation of the
    ** OpenGL.
    */

    modes = _gl_context_modes_find_visual( pGlxScreen->modes, visual );
    if (modes == NULL) {
	/*
	** Visual not support on this screen by this OpenGL implementation.
	*/
	client->errorValue = visual;
	return BadValue;
    }

    /*
    ** Find the display list space that we want to share.  
    **
    ** NOTE: In a multithreaded X server, we would need to keep a reference
    ** count for each display list so that if one client detroyed a list that 
    ** another client was using, the list would not really be freed until it 
    ** was no longer in use.  Since this sample implementation has no support 
    ** for multithreaded servers, we don't do this.  
    */
    if (shareList == None) {
	shareGC = 0;
    } else {
	shareglxc = (__GLXcontext *) LookupIDByType(shareList, __glXContextRes);
	if (!shareglxc) {
	    client->errorValue = shareList;
	    return __glXBadContext;
	}
	if (shareglxc->isDirect) {
	    /*
	    ** NOTE: no support for sharing display lists between direct
	    ** contexts, even if they are in the same address space.
	    */
#if 0
            /* Disabling this code seems to allow shared display lists
             * and texture objects to work.  We'll leave it disabled for now.
             */
	    client->errorValue = shareList;
	    return BadMatch;
#endif
	} else {
	    /*
	    ** Create an indirect context regardless of what the client asked
	    ** for; this way we can share display list space with shareList.
	    */
	    isDirect = GL_FALSE;
	}
	shareGC = shareglxc->gc;
    }

    /*
    ** Allocate memory for the new context
    */
    glxc = (__GLXcontext *) __glXMalloc(sizeof(__GLXcontext));
    if (!glxc) {
	return BadAlloc;
    }
    __glXMemset(glxc, 0, sizeof(__GLXcontext));

    /*
    ** Initially, setup the part of the context that could be used by
    ** a GL core that needs windowing information (e.g., Mesa).
    */
    glxc->pScreen = pScreen;
    glxc->pGlxScreen = pGlxScreen;
    glxc->pVisual = pVisual;
    glxc->modes = modes;

    if (!isDirect) {

	/*
	** Allocate a GL context
	*/
	imports.other = (void *)glxc;
	glxc->gc = (*pGlxScreen->createContext)(&imports, glxc->modes, shareGC);
	if (!glxc->gc) {
	    __glXFree(glxc);
	    client->errorValue = gcId;
	    return BadAlloc;
	}
    } else {
	/*
	** Don't need local GL context for a direct context.
	*/
	glxc->gc = 0;
    }
    /*
    ** Register this context as a resource.
    */
    if (!AddResource(gcId, __glXContextRes, (pointer)glxc)) {
	if (!isDirect) {
	    (*glxc->gc->exports.destroyContext)((__GLcontext *)glxc->gc);
        }
	__glXFree(glxc);
	client->errorValue = gcId;
	return BadAlloc;
    }
    
    /*
    ** Finally, now that everything is working, setup the rest of the
    ** context.
    */
    glxc->id = gcId;
    glxc->share_id = shareList;
    glxc->idExists = GL_TRUE;
    glxc->isCurrent = GL_FALSE;
    glxc->isDirect = isDirect;
    glxc->renderMode = GL_RENDER;

    return Success;
}


int __glXCreateContext(__GLXclientState *cl, GLbyte *pc)
{
    xGLXCreateContextReq *req = (xGLXCreateContextReq *) pc;
    return DoCreateContext( cl, req->context, req->shareList, req->visual,
			    req->screen, req->isDirect );
}


int __glXCreateNewContext(__GLXclientState *cl, GLbyte *pc)
{
    xGLXCreateNewContextReq *req = (xGLXCreateNewContextReq *) pc;
    return DoCreateContext( cl, req->context, req->shareList, req->fbconfig,
			    req->screen, req->isDirect );
}


int __glXCreateContextWithConfigSGIX(__GLXclientState *cl, GLbyte *pc)
{
    xGLXCreateContextWithConfigSGIXReq *req = 
	(xGLXCreateContextWithConfigSGIXReq *) pc;
    return DoCreateContext( cl, req->context, req->shareList, req->fbconfig,
			    req->screen, req->isDirect );
}

/*
** Destroy a GL context as an X resource.
*/
int __glXDestroyContext(__GLXclientState *cl, GLbyte *pc)
{
    ClientPtr client = cl->client;
    xGLXDestroyContextReq *req = (xGLXDestroyContextReq *) pc;
    GLXContextID gcId = req->context;
    __GLXcontext *glxc;
    
    glxc = (__GLXcontext *) LookupIDByType(gcId, __glXContextRes);
    if (glxc) {
	/*
	** Just free the resource; don't actually destroy the context,
	** because it might be in use.  The
	** destroy method will be called by the resource destruction routine
	** if necessary.
	*/
	FreeResourceByType(gcId, __glXContextRes, FALSE);
	return Success;
    } else {
	client->errorValue = gcId;
	return __glXBadContext;
    }
}

/*****************************************************************************/

/*
** For each client, the server keeps a table of all the contexts that are
** current for that client (each thread of a client may have its own current
** context).  These routines add, change, and lookup contexts in the table.
*/

/*
** Add a current context, and return the tag that will be used to refer to it.
*/
static int AddCurrentContext(__GLXclientState *cl, __GLXcontext *glxc)
{
    int i;
    int num = cl->numCurrentContexts;
    __GLXcontext **table = cl->currentContexts;

    if (!glxc) return -1;
    
    /*
    ** Try to find an empty slot and use it.
    */
    for (i=0; i < num; i++) {
	if (!table[i]) {
	    table[i] = glxc;
	    return i+1;
	}
    }
    /*
    ** Didn't find a free slot, so we'll have to grow the table.
    */
    if (!num) {
	table = (__GLXcontext **) __glXMalloc(sizeof(__GLXcontext *));
    } else {
	table = (__GLXcontext **) __glXRealloc(table,
					   (num+1)*sizeof(__GLXcontext *));
    }
    table[num] = glxc;
    cl->currentContexts = table;
    cl->numCurrentContexts++;
    return num+1;
}

/*
** Given a tag, change the current context for the corresponding entry.
*/
static void ChangeCurrentContext(__GLXclientState *cl, __GLXcontext *glxc,
				GLXContextTag tag)
{
    __GLXcontext **table = cl->currentContexts;
    table[tag-1] = glxc;
}

/*
** For this implementation we have chosen to simply use the index of the
** context's entry in the table as the context tag.  A tag must be greater
** than 0.
*/
__GLXcontext *__glXLookupContextByTag(__GLXclientState *cl, GLXContextTag tag)
{
    int num = cl->numCurrentContexts;

    if (tag < 1 || tag > num) {
	return 0;
    } else {
	return cl->currentContexts[tag-1];
    }
}

/*****************************************************************************/

static void StopUsingContext(__GLXcontext *glxc)
{
    if (glxc) {
	if (glxc == __glXLastContext) {
	    /* Tell server GL library */
	    __glXLastContext = 0;
	}
	glxc->isCurrent = GL_FALSE;
	if (!glxc->idExists) {
	    __glXFreeContext(glxc);
	}
    }
}

static void StartUsingContext(__GLXclientState *cl, __GLXcontext *glxc)
{
    glxc->isCurrent = GL_TRUE;
}

/*****************************************************************************/
/*
** Make an OpenGL context and drawable current.
*/

int __glXMakeCurrent(__GLXclientState *cl, GLbyte *pc)
{
    xGLXMakeCurrentReq *req = (xGLXMakeCurrentReq *) pc;

    return DoMakeCurrent( cl, req->drawable, req->drawable,
			  req->context, req->oldContextTag );
}

int __glXMakeContextCurrent(__GLXclientState *cl, GLbyte *pc)
{
    xGLXMakeContextCurrentReq *req = (xGLXMakeContextCurrentReq *) pc;

    return DoMakeCurrent( cl, req->drawable, req->readdrawable,
			  req->context, req->oldContextTag );
}

int __glXMakeCurrentReadSGI(__GLXclientState *cl, GLbyte *pc)
{
    xGLXMakeCurrentReadSGIReq *req = (xGLXMakeCurrentReadSGIReq *) pc;

    return DoMakeCurrent( cl, req->drawable, req->readable,
			  req->context, req->oldContextTag );
}


/**
 * Given a drawable ID, get the associated drawable and / or pixmap.
 * 
 * If the specified drawable ID is not a pixmap, \c ppPixmap will be set
 * to \c NULL on return.  In either case, \c ppDraw will be set to a drawable.
 * In the case where the drawable ID is a pixmap, \c ppDraw will be set to
 * the drawable associated with that pixmap.
 *
 * \param glxc      Associated GLX context.
 * \param drawId    ID of the drawable.
 * \param ppDraw    Location to store the pointer to the drawable.
 * \param ppPixmap  Location to store the pointer to the pixmap.
 * \param client    Pointer to the client state.
 * \return  Zero is returned on success.  Otherwise a GLX / X11 protocol error
 *          is returned.
 * 
 * \notes This function will need some modification when support pbuffers
 *        is added.
 */
static int GetDrawableOrPixmap( __GLXcontext *glxc, GLXDrawable drawId,
				DrawablePtr *ppDraw, __GLXpixmap **ppPixmap,
				ClientPtr client )
{
    DrawablePtr pDraw;
    __GLXpixmap *drawPixmap = NULL;

    pDraw = (DrawablePtr) LookupDrawable(drawId, client);
    if (pDraw) {
	if (pDraw->type == DRAWABLE_WINDOW) {
	    /*
	    ** Drawable is an X Window.
	    */
	    WindowPtr pWin = (WindowPtr)pDraw;
	    VisualID vid = wVisual(pWin);

	    /*
	    ** Check if window and context are similar.
	    */
	    if ((vid != glxc->pVisual->vid) ||
		(pWin->drawable.pScreen != glxc->pScreen)) {
		client->errorValue = drawId;
		return BadMatch;
	    }
	} else {
	    /*
	    ** An X Pixmap is not allowed as a parameter (a GLX Pixmap
	    ** is, but it must first be created with glxCreateGLXPixmap).
	    */
	    client->errorValue = drawId;
	    return __glXBadDrawable;
	}
    } else {
	drawPixmap = (__GLXpixmap *) LookupIDByType(drawId, __glXPixmapRes);
	if (drawPixmap) {
	    /*
	    ** Check if pixmap and context are similar.
	    */
	    if (drawPixmap->pScreen != glxc->pScreen ||
		drawPixmap->modes->visualID != glxc->modes->visualID) {
		client->errorValue = drawId;
		return BadMatch;
	    }
	    pDraw = drawPixmap->pDraw;

	} else {
	    /*
	    ** Drawable is neither a Window nor a GLXPixmap.
	    */
	    client->errorValue = drawId;
	    return __glXBadDrawable;
	}
    }

    *ppPixmap = drawPixmap;
    *ppDraw = pDraw;

    return 0;
}


int DoMakeCurrent( __GLXclientState *cl,
		   GLXDrawable drawId, GLXDrawable readId,
		   GLXContextID contextId, GLXContextTag tag )
{
    ClientPtr client = cl->client;
    DrawablePtr pDraw;
    DrawablePtr pRead;
    xGLXMakeCurrentReply reply;
    __GLXpixmap *drawPixmap = NULL;
    __GLXpixmap *readPixmap = NULL;
    __GLXcontext *glxc, *prevglxc;
    __GLinterface *gc, *prevgc;
    __GLXdrawablePrivate *drawPriv = NULL;
    __GLXdrawablePrivate *readPriv = NULL;
    GLint error;
    GLuint  mask;

    /*
    ** If one is None and the other isn't, it's a bad match.
    */

    mask  = (drawId == None)    ? (1 << 0) : 0;
    mask |= (readId == None)    ? (1 << 1) : 0;
    mask |= (contextId == None) ? (1 << 2) : 0;

    if ( (mask != 0x00) && (mask != 0x07) ) {
	return BadMatch;
    }
    
    /*
    ** Lookup old context.  If we have one, it must be in a usable state.
    */
    if (tag != 0) {
	prevglxc = __glXLookupContextByTag(cl, tag);
	if (!prevglxc) {
	    /*
	    ** Tag for previous context is invalid.
	    */
	    return __glXBadContextTag;
	}
	if (prevglxc->renderMode != GL_RENDER) {
	    /* Oops.  Not in render mode render. */
	    client->errorValue = prevglxc->id;
	    return __glXBadContextState;
	}
	prevgc = prevglxc->gc;
    } else {
	prevglxc = 0;
	prevgc = 0;
    }

    /*
    ** Lookup new context.  It must not be current for someone else.
    */
    if (contextId != None) {
	int  status;

	glxc = (__GLXcontext *) LookupIDByType(contextId, __glXContextRes);
	if (!glxc) {
	    client->errorValue = contextId;
	    return __glXBadContext;
	}
	if ((glxc != prevglxc) && glxc->isCurrent) {
	    /* Context is current to somebody else */
	    return BadAccess;
	}
	gc = glxc->gc;


	assert( drawId != None );
	assert( readId != None );

	status = GetDrawableOrPixmap( glxc, drawId, & pDraw, & drawPixmap,
				      client );
	if ( status != 0 ) {
	    return status;
	}

	if ( readId != drawId ) {
	    status = GetDrawableOrPixmap( glxc, readId, & pRead, & readPixmap,
					  client );
	    if ( status != 0 ) {
		return status;
	    }
	} else {
	    pRead = pDraw;
	}

	/* FIXME: Finish refactoring this. - idr */
	/* get the drawable private */
	if (pDraw) {
	    drawPriv = __glXGetDrawablePrivate(pDraw, drawId, glxc->modes);
	    if (drawPriv == NULL) {
		return __glXBadDrawable;
	    }
	}

	if (pRead != pDraw) {
	    readPriv = __glXGetDrawablePrivate(pRead, readId, glxc->modes);
	    if (readPriv == NULL) {
		return __glXBadDrawable;
	    }
	} else {
	    readPriv = drawPriv;
	}

    } else {
	/* Switching to no context.  Ignore new drawable. */
	glxc = 0;
	gc = 0;
	pDraw = 0;
	pRead = 0;
    }


    if (prevglxc) {
	/*
	** Flush the previous context if needed.
	*/
	if (__GLX_HAS_UNFLUSHED_CMDS(prevglxc)) {
	    if (__glXForceCurrent(cl, tag, (int *)&error)) {
		glFlush();
		__GLX_NOTE_FLUSHED_CMDS(prevglxc);
	    } else {
		return error;
	    }
	}

	/*
	** Make the previous context not current.
	*/
	if (!(*prevgc->exports.loseCurrent)((__GLcontext *)prevgc)) {
	    return __glXBadContext;
	}
	__glXDeassociateContext(prevglxc);
    }
	

    if ((glxc != 0) && !glxc->isDirect) {

	glxc->drawPriv = drawPriv;
	glxc->readPriv = readPriv;
	__glXCacheDrawableSize(drawPriv);

	/* make the context current */
	if (!(*gc->exports.makeCurrent)((__GLcontext *)gc)) {
	    glxc->drawPriv = NULL;
	    glxc->readPriv = NULL;
	    return __glXBadContext;
	}

	/* resize the buffers */
	if (!__glXResizeDrawableBuffers(drawPriv)) {
	    /* could not do initial resize.  make current failed */
	    (*gc->exports.loseCurrent)((__GLcontext *)gc);
	    glxc->drawPriv = NULL;
	    glxc->readPriv = NULL;
	    return __glXBadContext;
	}

	glxc->isCurrent = GL_TRUE;
	__glXAssociateContext(glxc);
	assert(drawPriv->drawGlxc == glxc);
	assert(readPriv->readGlxc == glxc);
    }

    if (prevglxc) {
	if (prevglxc->drawPixmap) {
	    if (prevglxc->readPixmap &&
		prevglxc->drawPixmap != prevglxc->readPixmap) {
		/*
		** The previous drawable was a glx pixmap, release it.
		*/
		prevglxc->readPixmap->refcnt--;
		if (!prevglxc->readPixmap->idExists &&
		    !prevglxc->readPixmap->refcnt) {
		    PixmapPtr pPixmap = (PixmapPtr) prevglxc->readPixmap->pDraw;
		    /*
		    ** The DestroyPixmap routine should decrement the
		    ** refcount of the X pixmap and free only if it's zero.
		    */
		    (*prevglxc->readPixmap->pScreen->DestroyPixmap)(pPixmap);
		    __glXFree(prevglxc->readPixmap);
		}
	    }

	    /*
	    ** The previous drawable was a glx pixmap, release it.
	    */
	    prevglxc->drawPixmap->refcnt--;
	    if (!prevglxc->drawPixmap->idExists &&
		!prevglxc->drawPixmap->refcnt) {
		PixmapPtr pPixmap = (PixmapPtr) prevglxc->drawPixmap->pDraw;
		/*
		** The DestroyPixmap routine should decrement the
		** refcount of the X pixmap and free only if it's zero.
		*/
		(*prevglxc->drawPixmap->pScreen->DestroyPixmap)(pPixmap);
		__glXFree(prevglxc->drawPixmap);
	    }

	    prevglxc->drawPixmap = NULL;
	}
	ChangeCurrentContext(cl, glxc, tag);
	StopUsingContext(prevglxc);
    } else {
	tag = AddCurrentContext(cl, glxc);
    }

    if (glxc) {
	if (drawPixmap) {
	    drawPixmap->refcnt++;
	    glxc->drawPixmap = drawPixmap;
	}

	if (readPixmap && (readPixmap != drawPixmap)) {
	    readPixmap->refcnt++;
	    glxc->readPixmap = readPixmap;
	}

	StartUsingContext(cl, glxc);
	reply.contextTag = tag;
    } else {
	reply.contextTag = 0;
    }

    reply.length = 0;
    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;

    if (client->swapped) {
	__glXSwapMakeCurrentReply(client, &reply);
    } else {
	WriteToClient(client, sz_xGLXMakeCurrentReply, (char *)&reply);
    }
    return Success;
}

int __glXIsDirect(__GLXclientState *cl, GLbyte *pc)
{
    ClientPtr client = cl->client;
    xGLXIsDirectReq *req = (xGLXIsDirectReq *) pc;
    xGLXIsDirectReply reply;
    __GLXcontext *glxc;

    /*
    ** Find the GL context.
    */
    glxc = (__GLXcontext *) LookupIDByType(req->context, __glXContextRes);
    if (!glxc) {
	client->errorValue = req->context;
	return __glXBadContext;
    }

    reply.isDirect = glxc->isDirect;
    reply.length = 0;
    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;

    if (client->swapped) {
	__glXSwapIsDirectReply(client, &reply);
    } else {
	WriteToClient(client, sz_xGLXIsDirectReply, (char *)&reply);
    }

    return Success;
}

int __glXQueryVersion(__GLXclientState *cl, GLbyte *pc)
{
    ClientPtr client = cl->client;
    xGLXQueryVersionReq *req = (xGLXQueryVersionReq *) pc;
    xGLXQueryVersionReply reply;
    GLuint major, minor;

    major = req->majorVersion;
    minor = req->minorVersion;
    (void)major;
    (void)minor;

    /*
    ** Server should take into consideration the version numbers sent by the
    ** client if it wants to work with older clients; however, in this
    ** implementation the server just returns its version number.
    */
    reply.majorVersion = GLX_SERVER_MAJOR_VERSION;
    reply.minorVersion = GLX_SERVER_MINOR_VERSION;
    reply.length = 0;
    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;

    if (client->swapped) {
	__glXSwapQueryVersionReply(client, &reply);
    } else {
	WriteToClient(client, sz_xGLXQueryVersionReply, (char *)&reply);
    }
    return Success;
}

int __glXWaitGL(__GLXclientState *cl, GLbyte *pc)
{
    xGLXWaitGLReq *req = (xGLXWaitGLReq *)pc;
    int error;
    
    if (!__glXForceCurrent(cl, req->contextTag, &error)) {
	return error;
    }
    glFinish();
    return Success;
}

int __glXWaitX(__GLXclientState *cl, GLbyte *pc)
{
    xGLXWaitXReq *req = (xGLXWaitXReq *)pc;
    int error;
    
    if (!__glXForceCurrent(cl, req->contextTag, &error)) {
	return error;
    }
    /*
    ** In a multithreaded server that had separate X and GL threads, we would
    ** have to wait for the X thread to finish before returning.  As it stands,
    ** this sample implementation only supports singlethreaded servers, and
    ** nothing needs to be done here.
    */
    return Success;
}

int __glXCopyContext(__GLXclientState *cl, GLbyte *pc)
{
    ClientPtr client = cl->client;
    xGLXCopyContextReq *req = (xGLXCopyContextReq *) pc;
    GLXContextID source = req->source;
    GLXContextID dest = req->dest;
    GLXContextTag tag = req->contextTag;
    unsigned long mask = req->mask;
    __GLXcontext *src, *dst;
    int error;

    /*
    ** Check that each context exists.
    */
    src = (__GLXcontext *) LookupIDByType(source, __glXContextRes);
    if (!src) {
	client->errorValue = source;
	return __glXBadContext;
    }
    dst = (__GLXcontext *) LookupIDByType(dest, __glXContextRes);
    if (!dst) {
	client->errorValue = dest;
	return __glXBadContext;
    }

    /*
    ** They must be in the same address space, and same screen.
    ** NOTE: no support for direct rendering contexts here.
    */
    if (src->isDirect || dst->isDirect ||
	(src->pGlxScreen != dst->pGlxScreen)) {
	client->errorValue = source;
	return BadMatch;
    }

    /*
    ** The destination context must not be current for any client.
    */
    if (dst->isCurrent) {
	client->errorValue = dest;
	return BadAccess;
    }

    if (tag) {
	__GLXcontext *tagcx = __glXLookupContextByTag(cl, tag);
	
	if (!tagcx) {
	    return __glXBadContextTag;
	}
	if (tagcx != src) {
	    /*
	    ** This would be caused by a faulty implementation of the client
	    ** library.
	    */
	    return BadMatch;
	}
	/*
	** In this case, glXCopyContext is in both GL and X streams, in terms
	** of sequentiality.
	*/
	if (__glXForceCurrent(cl, tag, &error)) {
	    /*
	    ** Do whatever is needed to make sure that all preceding requests
	    ** in both streams are completed before the copy is executed.
	    */
	    glFinish();
	    __GLX_NOTE_FLUSHED_CMDS(tagcx);
	} else {
	    return error;
	}
    }
    /*
    ** Issue copy.  The only reason for failure is a bad mask.
    */
    if (!(*dst->gc->exports.copyContext)((__GLcontext *)dst->gc, 
					 (__GLcontext *)src->gc,
					 mask)) {
	client->errorValue = mask;
	return BadValue;
    }
    return Success;
}


int DoGetVisualConfigs(__GLXclientState *cl, unsigned screen,
		       GLboolean do_swap)
{
    ClientPtr client = cl->client;
    xGLXGetVisualConfigsReply reply;
    __GLXscreenInfo *pGlxScreen;
    __GLcontextModes *modes;
    CARD32 buf[__GLX_TOTAL_CONFIG];
    int p;
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_DECLARE_SWAP_ARRAY_VARIABLES;

    if (screen >= screenInfo.numScreens) {
	/* The client library must send a valid screen number. */
	client->errorValue = screen;
	return BadValue;
    }
    pGlxScreen = &__glXActiveScreens[screen];

    reply.numVisuals = pGlxScreen->numUsableVisuals;
    reply.numProps = __GLX_TOTAL_CONFIG;
    reply.length = (pGlxScreen->numUsableVisuals * __GLX_SIZE_CARD32 *
		    __GLX_TOTAL_CONFIG) >> 2;
    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;

    if ( do_swap ) {
	__GLX_SWAP_SHORT(&reply.sequenceNumber);
	__GLX_SWAP_INT(&reply.length);
	__GLX_SWAP_INT(&reply.numVisuals);
	__GLX_SWAP_INT(&reply.numProps);
    }

    WriteToClient(client, sz_xGLXGetVisualConfigsReply, (char *)&reply);

    for ( modes = pGlxScreen->modes ; modes != NULL ; modes = modes->next ) {
	if (modes->visualID == 0) {
	    /* not a usable visual */
	    continue;
	}
	p = 0;
	buf[p++] = modes->visualID;
	buf[p++] = _gl_convert_to_x_visual_type( modes->visualType );
	buf[p++] = modes->rgbMode;

	buf[p++] = modes->redBits;
	buf[p++] = modes->greenBits;
	buf[p++] = modes->blueBits;
	buf[p++] = modes->alphaBits;
	buf[p++] = modes->accumRedBits;
	buf[p++] = modes->accumGreenBits;
	buf[p++] = modes->accumBlueBits;
	buf[p++] = modes->accumAlphaBits;

	buf[p++] = modes->doubleBufferMode;
	buf[p++] = modes->stereoMode;

	buf[p++] = modes->rgbBits;
	buf[p++] = modes->depthBits;
	buf[p++] = modes->stencilBits;
	buf[p++] = modes->numAuxBuffers;
	buf[p++] = modes->level;
	/* 
	** Add token/value pairs for extensions.
	*/
	buf[p++] = GLX_VISUAL_CAVEAT_EXT;
	buf[p++] = modes->visualRating;
	buf[p++] = GLX_TRANSPARENT_TYPE;
	buf[p++] = modes->transparentPixel;
	buf[p++] = GLX_TRANSPARENT_RED_VALUE;
	buf[p++] = modes->transparentRed;
	buf[p++] = GLX_TRANSPARENT_GREEN_VALUE;
	buf[p++] = modes->transparentGreen;
	buf[p++] = GLX_TRANSPARENT_BLUE_VALUE;
	buf[p++] = modes->transparentBlue;
	buf[p++] = GLX_TRANSPARENT_ALPHA_VALUE;
	buf[p++] = modes->transparentAlpha;
	buf[p++] = GLX_TRANSPARENT_INDEX_VALUE;
	buf[p++] = modes->transparentIndex;

	if ( do_swap ) {
	    __GLX_SWAP_INT_ARRAY(buf, __GLX_TOTAL_CONFIG);
	}
	WriteToClient(client, __GLX_SIZE_CARD32 * __GLX_TOTAL_CONFIG, 
		(char *)buf);
    }
    return Success;
}

int __glXGetVisualConfigs(__GLXclientState *cl, GLbyte *pc)
{
    xGLXGetVisualConfigsReq *req = (xGLXGetVisualConfigsReq *) pc;
    return DoGetVisualConfigs( cl, req->screen, GL_FALSE );
}



#define __GLX_TOTAL_FBCONFIG_ATTRIBS (28)
#define __GLX_FBCONFIG_ATTRIBS_LENGTH (__GLX_TOTAL_FBCONFIG_ATTRIBS * 2)
/**
 * Send the set of GLXFBConfigs to the client.  There is not currently
 * and interface into the driver on the server-side to get GLXFBConfigs,
 * so we "invent" some based on the \c __GLXvisualConfig structures that
 * the driver does supply.
 * 
 * The reply format for both \c glXGetFBConfigs and \c glXGetFBConfigsSGIX
 * is the same, so this routine pulls double duty.
 */

int DoGetFBConfigs(__GLXclientState *cl, unsigned screen, GLboolean do_swap)
{
    ClientPtr client = cl->client;
    xGLXGetFBConfigsReply reply;
    __GLXscreenInfo *pGlxScreen;
    CARD32 buf[__GLX_FBCONFIG_ATTRIBS_LENGTH];
    int p;
    __GLcontextModes *modes;
    __GLX_DECLARE_SWAP_VARIABLES;
    __GLX_DECLARE_SWAP_ARRAY_VARIABLES;


    if (screen >= screenInfo.numScreens) {
	/* The client library must send a valid screen number. */
	client->errorValue = screen;
	return BadValue;
    }
    pGlxScreen = &__glXActiveScreens[screen];

    reply.numFBConfigs = pGlxScreen->numUsableVisuals;
    reply.numAttribs = __GLX_TOTAL_FBCONFIG_ATTRIBS;
    reply.length = (__GLX_FBCONFIG_ATTRIBS_LENGTH * reply.numFBConfigs);
    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;

    if ( do_swap ) {
	__GLX_SWAP_SHORT(&reply.sequenceNumber);
	__GLX_SWAP_INT(&reply.length);
	__GLX_SWAP_INT(&reply.numFBConfigs);
	__GLX_SWAP_INT(&reply.numAttribs);
    }

    WriteToClient(client, sz_xGLXGetFBConfigsReply, (char *)&reply);

    for ( modes = pGlxScreen->modes ; modes != NULL ; modes = modes->next ) {
	if (modes->visualID == 0) {
	    /* not a usable visual */
	    continue;
	}
	p = 0;

#define WRITE_PAIR(tag,value) \
    do { buf[p++] = tag ; buf[p++] = value ; } while( 0 )

	WRITE_PAIR( GLX_VISUAL_ID,        modes->visualID );
	WRITE_PAIR( GLX_FBCONFIG_ID,      modes->visualID );
	WRITE_PAIR( GLX_X_RENDERABLE,     GL_TRUE );

	WRITE_PAIR( GLX_RGBA,             modes->rgbMode );
	WRITE_PAIR( GLX_DOUBLEBUFFER,     modes->doubleBufferMode );
	WRITE_PAIR( GLX_STEREO,           modes->stereoMode );

	WRITE_PAIR( GLX_BUFFER_SIZE,      modes->rgbBits );
	WRITE_PAIR( GLX_LEVEL,            modes->level );
	WRITE_PAIR( GLX_AUX_BUFFERS,      modes->numAuxBuffers );
	WRITE_PAIR( GLX_RED_SIZE,         modes->redBits );
	WRITE_PAIR( GLX_GREEN_SIZE,       modes->greenBits );
	WRITE_PAIR( GLX_BLUE_SIZE,        modes->blueBits );
	WRITE_PAIR( GLX_ALPHA_SIZE,       modes->alphaBits );
	WRITE_PAIR( GLX_ACCUM_RED_SIZE,   modes->accumRedBits );
	WRITE_PAIR( GLX_ACCUM_GREEN_SIZE, modes->accumGreenBits );
	WRITE_PAIR( GLX_ACCUM_BLUE_SIZE,  modes->accumBlueBits );
	WRITE_PAIR( GLX_ACCUM_ALPHA_SIZE, modes->accumAlphaBits );
	WRITE_PAIR( GLX_DEPTH_SIZE,       modes->depthBits );
	WRITE_PAIR( GLX_STENCIL_SIZE,     modes->stencilBits );

	WRITE_PAIR( GLX_X_VISUAL_TYPE,    modes->visualType );

	/* 
	** Add token/value pairs for extensions.
	*/
	WRITE_PAIR( GLX_CONFIG_CAVEAT, modes->visualRating );
	WRITE_PAIR( GLX_TRANSPARENT_TYPE, modes->transparentPixel );
	WRITE_PAIR( GLX_TRANSPARENT_RED_VALUE, modes->transparentRed );
	WRITE_PAIR( GLX_TRANSPARENT_GREEN_VALUE, modes->transparentGreen );
	WRITE_PAIR( GLX_TRANSPARENT_BLUE_VALUE, modes->transparentBlue );
	WRITE_PAIR( GLX_TRANSPARENT_ALPHA_VALUE, modes->transparentAlpha );
	WRITE_PAIR( GLX_TRANSPARENT_INDEX_VALUE, modes->transparentIndex );
	WRITE_PAIR( GLX_SWAP_METHOD_OML, modes->swapMethod );

	if ( do_swap ) {
	    __GLX_SWAP_INT_ARRAY(buf, __GLX_FBCONFIG_ATTRIBS_LENGTH);
	}
	WriteToClient(client, __GLX_SIZE_CARD32 * __GLX_FBCONFIG_ATTRIBS_LENGTH,
		      (char *)buf);
    }
    return Success;
}


int __glXGetFBConfigs(__GLXclientState *cl, GLbyte *pc)
{
    xGLXGetFBConfigsReq *req = (xGLXGetFBConfigsReq *) pc;
    return DoGetFBConfigs( cl, req->screen, GL_FALSE );
}


int __glXGetFBConfigsSGIX(__GLXclientState *cl, GLbyte *pc)
{
    xGLXGetFBConfigsSGIXReq *req = (xGLXGetFBConfigsSGIXReq *) pc;
    return DoGetFBConfigs( cl, req->screen, GL_FALSE );
}


/*
** Create a GLX Pixmap from an X Pixmap.
*/
int DoCreateGLXPixmap(__GLXclientState *cl, VisualID visual,
		      GLuint screenNum, XID pixmapId, XID glxpixmapId)
{
    ClientPtr client = cl->client;
    DrawablePtr pDraw;
    ScreenPtr pScreen;
    VisualPtr pVisual;
    __GLXpixmap *pGlxPixmap;
    __GLXscreenInfo *pGlxScreen;
    __GLcontextModes *modes;
    int i;

    LEGAL_NEW_RESOURCE(glxpixmapId, client);
    
    pDraw = (DrawablePtr) LookupDrawable(pixmapId, client);
    if (!pDraw || pDraw->type != DRAWABLE_PIXMAP) {
	client->errorValue = pixmapId;
	return BadPixmap;
    }

    /*
    ** Check if screen of visual matches screen of pixmap.
    */
    pScreen = pDraw->pScreen;
    if (screenNum != pScreen->myNum) {
	return BadMatch;
    }

    /*
    ** Find the VisualRec for this visual.
    */
    pVisual = pScreen->visuals;
    for (i=0; i < pScreen->numVisuals; i++, pVisual++) {
	if (pVisual->vid == visual) {
	    break;
	}
    }
    if (i == pScreen->numVisuals) {
	client->errorValue = visual;
	return BadValue;
    }
    /*
    ** Check if depth of visual matches depth of pixmap.
    */
    if (pVisual->nplanes != pDraw->depth) {
	return BadMatch;
    }

    /*
    ** Get configuration of the visual.
    */
    pGlxScreen = &__glXActiveScreens[screenNum];
    modes = _gl_context_modes_find_visual( pGlxScreen->modes, visual );
    if (modes == NULL) {
	/*
	** Visual not support on this screen by this OpenGL implementation.
	*/
	client->errorValue = visual;
	return BadValue;
    }

    pGlxPixmap = (__GLXpixmap *) __glXMalloc(sizeof(__GLXpixmap));
    if (!pGlxPixmap) {
	return BadAlloc;
    }
    if (!(AddResource(glxpixmapId, __glXPixmapRes, pGlxPixmap))) {
	return BadAlloc;
    }
    pGlxPixmap->pDraw = pDraw;
    pGlxPixmap->pGlxScreen = pGlxScreen;
    pGlxPixmap->pScreen = pScreen;
    pGlxPixmap->idExists = True;
    pGlxPixmap->refcnt = 0;

    pGlxPixmap->modes = modes;

    /*
    ** Bump the ref count on the X pixmap so it won't disappear.
    */
    ((PixmapPtr) pDraw)->refcnt++;

    return Success;
}

int __glXCreateGLXPixmap(__GLXclientState *cl, GLbyte *pc)
{
    xGLXCreateGLXPixmapReq *req = (xGLXCreateGLXPixmapReq *) pc;
    return DoCreateGLXPixmap( cl, req->visual, req->screen,
			      req->pixmap, req->glxpixmap );
}

int __glXCreatePixmap(__GLXclientState *cl, GLbyte *pc)
{
    xGLXCreatePixmapReq *req = (xGLXCreatePixmapReq *) pc;
    return DoCreateGLXPixmap( cl, req->fbconfig, req->screen,
			      req->pixmap, req->glxpixmap );
}

int __glXCreateGLXPixmapWithConfigSGIX(__GLXclientState *cl, GLbyte *pc)
{
    xGLXCreateGLXPixmapWithConfigSGIXReq *req = 
	(xGLXCreateGLXPixmapWithConfigSGIXReq *) pc;
    return DoCreateGLXPixmap( cl, req->fbconfig, req->screen,
			      req->pixmap, req->glxpixmap );
}


/**
 * Destroy a GLX pixmap.  This function is used for both
 * \c glXDestroyGLXPixmap and \c glXDestroyPixmap.
 */

int __glXDestroyGLXPixmap(__GLXclientState *cl, GLbyte *pc)
{
    ClientPtr client = cl->client;
    xGLXDestroyGLXPixmapReq *req = (xGLXDestroyGLXPixmapReq *) pc;
    XID glxpixmap = req->glxpixmap;

    /*
    ** Check if it's a valid GLX pixmap.
    */
    if (!LookupIDByType(glxpixmap, __glXPixmapRes)) {
	client->errorValue = glxpixmap;
	return __glXBadPixmap;
    }
    FreeResource(glxpixmap, FALSE);
    return Success;
}

/*****************************************************************************/

/*
** NOTE: There is no portable implementation for swap buffers as of
** this time that is of value.  Consequently, this code must be
** implemented by somebody other than SGI.
*/
int __glXSwapBuffers(__GLXclientState *cl, GLbyte *pc)
{
    ClientPtr client = cl->client;
    DrawablePtr pDraw;
    xGLXSwapBuffersReq *req = (xGLXSwapBuffersReq *) pc;
    GLXContextTag tag = req->contextTag;
    XID drawId = req->drawable;
    __GLXpixmap *pGlxPixmap;
    __GLXcontext *glxc = NULL;
    int error;
    
    /*
    ** Check that the GLX drawable is valid.
    */
    pDraw = (DrawablePtr) LookupDrawable(drawId, client);
    if (pDraw) {
	if (pDraw->type == DRAWABLE_WINDOW) {
	    /*
	    ** Drawable is an X window.
	    */
	} else {
	    /*
	    ** Drawable is an X pixmap, which is not allowed.
	    */
	    client->errorValue = drawId;
	    return __glXBadDrawable;
	}
    } else {
	pGlxPixmap = (__GLXpixmap *) LookupIDByType(drawId,
						    __glXPixmapRes);
	if (pGlxPixmap) {
	    /*
	    ** Drawable is a GLX pixmap.
	    */
	} else {
	    /*
	    ** Drawable is neither a X window nor a GLX pixmap.
	    */
	    client->errorValue = drawId;
	    return __glXBadDrawable;
	}
    }

    if (tag) {
	glxc = __glXLookupContextByTag(cl, tag);
	if (!glxc) {
	    return __glXBadContextTag;
	}
	/*
	** The calling thread is swapping its current drawable.  In this case,
	** glxSwapBuffers is in both GL and X streams, in terms of
	** sequentiality.
	*/
	if (__glXForceCurrent(cl, tag, &error)) {
	    /*
	    ** Do whatever is needed to make sure that all preceding requests
	    ** in both streams are completed before the swap is executed.
	    */
	    glFinish();
	    __GLX_NOTE_FLUSHED_CMDS(glxc);
	} else {
	    return error;
	}
    }

    if (pDraw) {
	__GLXdrawablePrivate *glxPriv;

	if (glxc) {
	    glxPriv = __glXGetDrawablePrivate(pDraw, drawId, glxc->modes);
	    if (glxPriv == NULL) {
		return __glXBadDrawable;
	    }
	}
	else {
	    glxPriv = __glXFindDrawablePrivate(drawId);
	    if (glxPriv == NULL) {
		/* This is a window we've never seen before, do nothing */
		return Success;
	    }
	}

	if ((*glxPriv->swapBuffers)(glxPriv) == GL_FALSE) {
	    return __glXBadDrawable;
	}
    }

    return Success;
}


int __glXQueryContextInfoEXT(__GLXclientState *cl, GLbyte *pc)
{
    ClientPtr client = cl->client;
    __GLXcontext *ctx;
    xGLXQueryContextInfoEXTReq *req;
    xGLXQueryContextInfoEXTReply reply;
    int nProps;
    int *sendBuf, *pSendBuf;
    int nReplyBytes;

    req = (xGLXQueryContextInfoEXTReq *)pc;
    ctx = (__GLXcontext *) LookupIDByType(req->context, __glXContextRes);
    if (!ctx) {
	client->errorValue = req->context;
	return __glXBadContext;
    }

    nProps = 3;
    reply.length = nProps << 1;
    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;
    reply.n = nProps;

    nReplyBytes = reply.length << 2;
    sendBuf = (int *)__glXMalloc((size_t)nReplyBytes);
    if (sendBuf == NULL) {
	return __glXBadContext;	/* XXX: Is this correct? */
    }
    pSendBuf = sendBuf;
    *pSendBuf++ = GLX_SHARE_CONTEXT_EXT;
    *pSendBuf++ = (int)(ctx->share_id);
    *pSendBuf++ = GLX_VISUAL_ID_EXT;
    *pSendBuf++ = (int)(ctx->pVisual->vid);
    *pSendBuf++ = GLX_SCREEN_EXT;
    *pSendBuf++ = (int)(ctx->pScreen->myNum);

    if (client->swapped) {
	__glXSwapQueryContextInfoEXTReply(client, &reply, sendBuf);
    } else {
	WriteToClient(client, sz_xGLXQueryContextInfoEXTReply, (char *)&reply);
	WriteToClient(client, nReplyBytes, (char *)sendBuf);
    }
    __glXFree((char *)sendBuf);

    return Success;
}


/************************************************************************/

/*
** Render and Renderlarge are not in the GLX API.  They are used by the GLX
** client library to send batches of GL rendering commands.
*/

/*
** Execute all the drawing commands in a request.
*/
int __glXRender(__GLXclientState *cl, GLbyte *pc)
{
    xGLXRenderReq *req;
    ClientPtr client= cl->client;
    int left, cmdlen, error;
    int commandsDone;
    CARD16 opcode;
    __GLXrenderHeader *hdr;
    __GLXcontext *glxc;

    /*
    ** NOTE: much of this code also appears in the byteswapping version of this
    ** routine, __glXSwapRender().  Any changes made here should also be
    ** duplicated there.
    */
    
    req = (xGLXRenderReq *) pc;
    glxc = __glXForceCurrent(cl, req->contextTag, &error);
    if (!glxc) {
	return error;
    }

    commandsDone = 0;
    pc += sz_xGLXRenderReq;
    left = (req->length << 2) - sz_xGLXRenderReq;
    while (left > 0) {
        __GLXrenderSizeData *entry;
        int extra;
	void (* proc)(GLbyte *);

	/*
	** Verify that the header length and the overall length agree.
	** Also, each command must be word aligned.
	*/
	hdr = (__GLXrenderHeader *) pc;
	cmdlen = hdr->length;
	opcode = hdr->opcode;

	/*
	** Check for core opcodes and grab entry data.
	*/
	if ( (opcode >= __GLX_MIN_RENDER_OPCODE) && 
	     (opcode <= __GLX_MAX_RENDER_OPCODE) ) {
	    entry = &__glXRenderSizeTable[opcode];
	    proc = __glXRenderTable[opcode];
#if __GLX_MAX_RENDER_OPCODE_EXT > __GLX_MIN_RENDER_OPCODE_EXT
	} else if ( (opcode >= __GLX_MIN_RENDER_OPCODE_EXT) && 
		    (opcode <= __GLX_MAX_RENDER_OPCODE_EXT) ) {
	    entry = 
		&__glXRenderSizeTable_EXT[opcode - 
					 __GLX_MIN_RENDER_OPCODE_EXT];
	    proc = __glXRenderTable_EXT[opcode - 
				       __GLX_MIN_RENDER_OPCODE_EXT];
#endif /* __GLX_MAX_RENDER_OPCODE_EXT > __GLX_MIN_RENDER_OPCODE_EXT */
	} else {
	    client->errorValue = commandsDone;
	    return __glXBadRenderRequest;
	}

        if (!entry->bytes) {
            /* unused opcode */
            client->errorValue = commandsDone;
            return __glXBadRenderRequest;
        }
        if (entry->varsize) {
            /* variable size command */
            extra = (*entry->varsize)(pc + __GLX_RENDER_HDR_SIZE, False);
            if (extra < 0) {
                extra = 0;
            }
            if (cmdlen != __GLX_PAD(entry->bytes + extra)) {
                return BadLength;
            }
        } else {
            /* constant size command */
            if (cmdlen != __GLX_PAD(entry->bytes)) {
                return BadLength;
            }
        }
	if (left < cmdlen) {
	    return BadLength;
	}

	/*
	** Skip over the header and execute the command.  We allow the
	** caller to trash the command memory.  This is useful especially
	** for things that require double alignment - they can just shift
	** the data towards lower memory (trashing the header) by 4 bytes
	** and achieve the required alignment.
	*/
	(*proc)(pc + __GLX_RENDER_HDR_SIZE);
	pc += cmdlen;
	left -= cmdlen;
	commandsDone++;
    }
    __GLX_NOTE_UNFLUSHED_CMDS(glxc);
    return Success;
}

/*
** Execute a large rendering request (one that spans multiple X requests).
*/
int __glXRenderLarge(__GLXclientState *cl, GLbyte *pc)
{
    xGLXRenderLargeReq *req;
    ClientPtr client= cl->client;
    GLuint dataBytes;
    void (*proc)(GLbyte *);
    __GLXrenderLargeHeader *hdr;
    __GLXcontext *glxc;
    int error;
    CARD16 opcode;

    /*
    ** NOTE: much of this code also appears in the byteswapping version of this
    ** routine, __glXSwapRenderLarge().  Any changes made here should also be
    ** duplicated there.
    */
    
    req = (xGLXRenderLargeReq *) pc;
    glxc = __glXForceCurrent(cl, req->contextTag, &error);
    if (!glxc) {
	/* Reset in case this isn't 1st request. */
	__glXResetLargeCommandStatus(cl);
	return error;
    }
    dataBytes = req->dataBytes;

    /*
    ** Check the request length.
    */
    if ((req->length << 2) != __GLX_PAD(dataBytes) + sz_xGLXRenderLargeReq) {
	client->errorValue = req->length;
	/* Reset in case this isn't 1st request. */
	__glXResetLargeCommandStatus(cl);
	return BadLength;
    }
    pc += sz_xGLXRenderLargeReq;
    
    if (cl->largeCmdRequestsSoFar == 0) {
	__GLXrenderSizeData *entry;
	int extra, cmdlen;
	/*
	** This is the first request of a multi request command.
	** Make enough space in the buffer, then copy the entire request.
	*/
	if (req->requestNumber != 1) {
	    client->errorValue = req->requestNumber;
	    return __glXBadLargeRequest;
	}

	hdr = (__GLXrenderLargeHeader *) pc;
	cmdlen = hdr->length;
	opcode = hdr->opcode;

	/*
	** Check for core opcodes and grab entry data.
	*/
	if ( (opcode >= __GLX_MIN_RENDER_OPCODE) && 
	     (opcode <= __GLX_MAX_RENDER_OPCODE) ) {
	    entry = &__glXRenderSizeTable[opcode];
#if __GLX_MAX_RENDER_OPCODE_EXT > __GLX_MIN_RENDER_OPCODE_EXT
	} else if ( (opcode >= __GLX_MIN_RENDER_OPCODE_EXT) && 
	     (opcode <= __GLX_MAX_RENDER_OPCODE_EXT) ) {
	    opcode -= __GLX_MIN_RENDER_OPCODE_EXT;
	    entry = &__glXRenderSizeTable_EXT[opcode];
#endif /* __GLX_MAX_RENDER_OPCODE_EXT > __GLX_MIN_RENDER_OPCODE_EXT */
	} else {
	    client->errorValue = opcode;
	    return __glXBadLargeRequest;
	}

        if (!entry->bytes) {
            /* unused opcode */
            client->errorValue = opcode;
            return __glXBadLargeRequest;
        }
	if (entry->varsize) {
	    /*
	    ** If it's a variable-size command (a command whose length must
	    ** be computed from its parameters), all the parameters needed
	    ** will be in the 1st request, so it's okay to do this.
	    */
	    extra = (*entry->varsize)(pc + __GLX_RENDER_LARGE_HDR_SIZE, False);
	    if (extra < 0) {
		extra = 0;
	    }
	    /* large command's header is 4 bytes longer, so add 4 */
	    if (cmdlen != __GLX_PAD(entry->bytes + 4 + extra)) {
		return BadLength;
	    }
	} else {
	    /* constant size command */
	    if (cmdlen != __GLX_PAD(entry->bytes + 4)) {
		return BadLength;
	    }
	}
	/*
	** Make enough space in the buffer, then copy the entire request.
	*/
	if (cl->largeCmdBufSize < cmdlen) {
	    if (!cl->largeCmdBuf) {
		cl->largeCmdBuf = (GLbyte *) __glXMalloc((size_t)cmdlen);
	    } else {
		cl->largeCmdBuf = (GLbyte *) __glXRealloc(cl->largeCmdBuf, 
							  (size_t)cmdlen);
	    }
	    if (!cl->largeCmdBuf) {
		return BadAlloc;
	    }
	    cl->largeCmdBufSize = cmdlen;
	}
	__glXMemcpy(cl->largeCmdBuf, pc, dataBytes);

	cl->largeCmdBytesSoFar = dataBytes;
	cl->largeCmdBytesTotal = cmdlen;
	cl->largeCmdRequestsSoFar = 1;
	cl->largeCmdRequestsTotal = req->requestTotal;
	return Success;
	
    } else {
	/*
	** We are receiving subsequent (i.e. not the first) requests of a
	** multi request command.
	*/

	/*
	** Check the request number and the total request count.
	*/
	if (req->requestNumber != cl->largeCmdRequestsSoFar + 1) {
	    client->errorValue = req->requestNumber;
	    __glXResetLargeCommandStatus(cl);
	    return __glXBadLargeRequest;
	}
	if (req->requestTotal != cl->largeCmdRequestsTotal) {
	    client->errorValue = req->requestTotal;
	    __glXResetLargeCommandStatus(cl);
	    return __glXBadLargeRequest;
	}

	/*
	** Check that we didn't get too much data.
	*/
	if ((cl->largeCmdBytesSoFar + dataBytes) > cl->largeCmdBytesTotal) {
	    client->errorValue = dataBytes;
	    __glXResetLargeCommandStatus(cl);
	    return __glXBadLargeRequest;
	}
	__glXMemcpy(cl->largeCmdBuf + cl->largeCmdBytesSoFar, pc, dataBytes);
	cl->largeCmdBytesSoFar += dataBytes;
	cl->largeCmdRequestsSoFar++;

	if (req->requestNumber == cl->largeCmdRequestsTotal) {
	    /*
	    ** This is the last request; it must have enough bytes to complete
	    ** the command.
	    */
	    /* NOTE: the two pad macros have been added below; they are needed
	    ** because the client library pads the total byte count, but not
	    ** the per-request byte counts.  The Protocol Encoding says the
	    ** total byte count should not be padded, so a proposal will be 
	    ** made to the ARB to relax the padding constraint on the total 
	    ** byte count, thus preserving backward compatibility.  Meanwhile, 
	    ** the padding done below fixes a bug that did not allow
	    ** large commands of odd sizes to be accepted by the server.
	    */
	    if (__GLX_PAD(cl->largeCmdBytesSoFar) !=
		__GLX_PAD(cl->largeCmdBytesTotal)) {
		client->errorValue = dataBytes;
		__glXResetLargeCommandStatus(cl);
		return __glXBadLargeRequest;
	    }
	    hdr = (__GLXrenderLargeHeader *) cl->largeCmdBuf;
	    opcode = hdr->opcode;

	    /*
	    ** Use the opcode to index into the procedure table.
	    */
	    if ( (opcode >= __GLX_MIN_RENDER_OPCODE) && 
		 (opcode <= __GLX_MAX_RENDER_OPCODE) ) {
		proc = __glXRenderTable[opcode];
#if __GLX_MAX_RENDER_OPCODE_EXT > __GLX_MIN_RENDER_OPCODE_EXT
	    } else if ( (opcode >= __GLX_MIN_RENDER_OPCODE_EXT) && 
		 (opcode <= __GLX_MAX_RENDER_OPCODE_EXT) ) {
		opcode -= __GLX_MIN_RENDER_OPCODE_EXT;
		proc = __glXRenderTable_EXT[opcode];
#endif /* __GLX_MAX_RENDER_OPCODE_EXT > __GLX_MIN_RENDER_OPCODE_EXT */
	    } else {
		client->errorValue = opcode;
		return __glXBadLargeRequest;
	    }

	    /*
	    ** Skip over the header and execute the command.
	    */
	    (*proc)(cl->largeCmdBuf + __GLX_RENDER_LARGE_HDR_SIZE);
	    __GLX_NOTE_UNFLUSHED_CMDS(glxc);

	    /*
	    ** Reset for the next RenderLarge series.
	    */
	    __glXResetLargeCommandStatus(cl);
	} else {
	    /*
	    ** This is neither the first nor the last request.
	    */
	}
	return Success;
    }
}

extern RESTYPE __glXSwapBarrierRes;

static int __glXBindSwapBarrierSGIX(__GLXclientState *cl, GLbyte *pc)
{
    ClientPtr client = cl->client;
    xGLXBindSwapBarrierSGIXReq *req = (xGLXBindSwapBarrierSGIXReq *) pc;
    XID drawable = req->drawable;
    int barrier = req->barrier;
    DrawablePtr pDraw = (DrawablePtr) LookupDrawable(drawable, client);
    int screen = pDraw->pScreen->myNum;


    if (pDraw && (pDraw->type == DRAWABLE_WINDOW)) {
        if (__glXSwapBarrierFuncs &&
            __glXSwapBarrierFuncs[screen].bindSwapBarrierFunc) {
            int ret = __glXSwapBarrierFuncs[screen].bindSwapBarrierFunc(screen, drawable, barrier);
            if (ret == Success) {
                if (barrier)
                    /* add source for cleanup when drawable is gone */
                    AddResource(drawable, __glXSwapBarrierRes, (pointer)screen);
                else
                    /* delete source */
                    FreeResourceByType(drawable, __glXSwapBarrierRes, FALSE);
            }
            return ret;
        }
    }
    client->errorValue = drawable;
    return __glXBadDrawable;
}


static int __glXQueryMaxSwapBarriersSGIX(__GLXclientState *cl, GLbyte *pc)
{
    ClientPtr client = cl->client;
    xGLXQueryMaxSwapBarriersSGIXReq *req =
                                    (xGLXQueryMaxSwapBarriersSGIXReq *) pc;
    xGLXQueryMaxSwapBarriersSGIXReply reply;
    int screen = req->screen;

    if (__glXSwapBarrierFuncs &&
        __glXSwapBarrierFuncs[screen].queryMaxSwapBarriersFunc)
        reply.max = __glXSwapBarrierFuncs[screen].queryMaxSwapBarriersFunc(screen);
    else
        reply.max = 0;


    reply.length = 0;
    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;

    if (client->swapped) {
        __GLX_DECLARE_SWAP_VARIABLES;
        __GLX_SWAP_SHORT(&reply.sequenceNumber);
    }

    WriteToClient(client, sz_xGLXQueryMaxSwapBarriersSGIXReply,
                        (char *) &reply);
    return Success;
}

#define GLX_BAD_HYPERPIPE_SGIX 92

static int __glxQueryHyperpipeNetworkSGIX(__GLXclientState *cl, GLbyte *pc)
{
    ClientPtr client = cl->client;
    xGLXQueryHyperpipeNetworkSGIXReq * req = (xGLXQueryHyperpipeNetworkSGIXReq *) pc;
    xGLXQueryHyperpipeNetworkSGIXReply reply;
    int screen = req->screen;
    void *rdata = NULL;

    int length=0;
    int npipes=0;

    int n= 0;

    if (__glXHyperpipeFuncs &&
        __glXHyperpipeFuncs[screen].queryHyperpipeNetworkFunc != NULL) {
        rdata =
            (__glXHyperpipeFuncs[screen].queryHyperpipeNetworkFunc(screen, &npipes, &n));
    }
    length = __GLX_PAD(n) >> 2;
    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;
    reply.length = length;
    reply.n = n;
    reply.npipes = npipes;

    if (client->swapped) {
        __GLX_DECLARE_SWAP_VARIABLES;
        __GLX_SWAP_SHORT(&reply.sequenceNumber);
        __GLX_SWAP_INT(&reply.length);
        __GLX_SWAP_INT(&reply.n);
        __GLX_SWAP_INT(&reply.npipes);
    }
    WriteToClient(client, sz_xGLXQueryHyperpipeNetworkSGIXReply,
                  (char *) &reply);

    WriteToClient(client, length << 2, (char *)rdata);

    return Success;
}

static int __glxDestroyHyperpipeConfigSGIX (__GLXclientState *cl, GLbyte *pc)
{
    ClientPtr client = cl->client;
    xGLXDestroyHyperpipeConfigSGIXReq * req =
        (xGLXDestroyHyperpipeConfigSGIXReq *) pc;
    xGLXDestroyHyperpipeConfigSGIXReply reply;
    int screen = req->screen;
    int  success = GLX_BAD_HYPERPIPE_SGIX;
    int hpId ;

    hpId = req->hpId;


    if (__glXHyperpipeFuncs &&
        __glXHyperpipeFuncs[screen].destroyHyperpipeConfigFunc != NULL) {
        success = __glXHyperpipeFuncs[screen].destroyHyperpipeConfigFunc(screen, hpId);
    }

    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;
    reply.length = __GLX_PAD(0) >> 2;
    reply.n = 0;
    reply.success = success;


    if (client->swapped) {
        __GLX_DECLARE_SWAP_VARIABLES;
        __GLX_SWAP_SHORT(&reply.sequenceNumber);
    }
    WriteToClient(client,
                  sz_xGLXDestroyHyperpipeConfigSGIXReply,
                  (char *) &reply);
    return Success;
}

static int __glxQueryHyperpipeConfigSGIX(__GLXclientState *cl, GLbyte *pc)
{
    ClientPtr client = cl->client;
    xGLXQueryHyperpipeConfigSGIXReq * req =
        (xGLXQueryHyperpipeConfigSGIXReq *) pc;
    xGLXQueryHyperpipeConfigSGIXReply reply;
    int screen = req->screen;
    void *rdata = NULL;
    int length;
    int npipes=0;
    int n= 0;
    int hpId;

    hpId = req->hpId;

    if (__glXHyperpipeFuncs &&
        __glXHyperpipeFuncs[screen].queryHyperpipeConfigFunc != NULL) {
        rdata = __glXHyperpipeFuncs[screen].queryHyperpipeConfigFunc(screen, hpId,&npipes, &n);
    }

    length = __GLX_PAD(n) >> 2;
    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;
    reply.length = length;
    reply.n = n;
    reply.npipes = npipes;


    if (client->swapped) {
        __GLX_DECLARE_SWAP_VARIABLES;
        __GLX_SWAP_SHORT(&reply.sequenceNumber);
        __GLX_SWAP_INT(&reply.length);
        __GLX_SWAP_INT(&reply.n);
        __GLX_SWAP_INT(&reply.npipes);
    }

    WriteToClient(client, sz_xGLXQueryHyperpipeConfigSGIXReply,
                  (char *) &reply);

    WriteToClient(client, length << 2, (char *)rdata);

    return Success;
}

static int __glxHyperpipeConfigSGIX(__GLXclientState *cl, GLbyte *pc)
{
    ClientPtr client = cl->client;
    xGLXHyperpipeConfigSGIXReq * req =
        (xGLXHyperpipeConfigSGIXReq *) pc;
    xGLXHyperpipeConfigSGIXReply reply;
    int screen = req->screen;
    void *rdata;

    int npipes=0, networkId;
    int hpId=-1;

    networkId = (int)req->networkId;
    npipes = (int)req->npipes;
    rdata = (void *)(req +1);

    if (__glXHyperpipeFuncs &&
        __glXHyperpipeFuncs[screen].hyperpipeConfigFunc != NULL) {
        __glXHyperpipeFuncs[screen].hyperpipeConfigFunc(screen,networkId,
                                                        &hpId, &npipes,
                                                        (void *) rdata);
    }

    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;
    reply.length = __GLX_PAD(0) >> 2;
    reply.n = 0;
    reply.npipes = npipes;
    reply.hpId = hpId;

    if (client->swapped) {
        __GLX_DECLARE_SWAP_VARIABLES;
        __GLX_SWAP_SHORT(&reply.sequenceNumber);
        __GLX_SWAP_INT(&reply.npipes);
        __GLX_SWAP_INT(&reply.hpId);
    }

    WriteToClient(client, sz_xGLXHyperpipeConfigSGIXReply,
                  (char *) &reply);

    return Success;
}


/************************************************************************/

/*
** No support is provided for the vendor-private requests other than
** allocating the entry points in the dispatch table.
*/

int __glXVendorPrivate(__GLXclientState *cl, GLbyte *pc)
{
    xGLXVendorPrivateReq *req;
    GLint vendorcode;

    req = (xGLXVendorPrivateReq *) pc;
    vendorcode = req->vendorCode;

#ifndef __DARWIN__
    switch( vendorcode ) {
    case X_GLvop_SampleMaskSGIS:
	glSampleMaskSGIS(*(GLfloat *)(pc + 4),
			 *(GLboolean *)(pc + 8));
	return Success;
    case X_GLvop_SamplePatternSGIS:
	glSamplePatternSGIS( *(GLenum *)(pc + 4));
	return Success;
    case X_GLXvop_BindSwapBarrierSGIX:
        return __glXBindSwapBarrierSGIX(cl, pc);
    }
#endif

    if ((vendorcode >= __GLX_MIN_VENDPRIV_OPCODE_EXT) &&
          (vendorcode <= __GLX_MAX_VENDPRIV_OPCODE_EXT))  {
	(*__glXVendorPrivTable_EXT[vendorcode-__GLX_MIN_VENDPRIV_OPCODE_EXT])
							(cl, (GLbyte*)req);
	return Success;
    }
    /*
    ** This sample implemention does not support any private requests.
    */
    cl->client->errorValue = req->vendorCode;
    return __glXUnsupportedPrivateRequest;
}

int __glXVendorPrivateWithReply(__GLXclientState *cl, GLbyte *pc)
{
    xGLXVendorPrivateWithReplyReq *req;
    GLint vendorcode;

    req = (xGLXVendorPrivateWithReplyReq *) pc;
    vendorcode = req->vendorCode;

    switch (vendorcode) {
      case X_GLXvop_QueryContextInfoEXT:
	return __glXQueryContextInfoEXT(cl, pc);
      case X_GLXvop_MakeCurrentReadSGI:
	return __glXMakeCurrentReadSGI(cl, pc);
      case X_GLXvop_QueryMaxSwapBarriersSGIX:
        return __glXQueryMaxSwapBarriersSGIX(cl, pc);
      case X_GLXvop_QueryHyperpipeNetworkSGIX:
        return __glxQueryHyperpipeNetworkSGIX(cl, pc);
      case X_GLXvop_QueryHyperpipeConfigSGIX:
        return __glxQueryHyperpipeConfigSGIX(cl, pc);
      case X_GLXvop_DestroyHyperpipeConfigSGIX:
        return __glxDestroyHyperpipeConfigSGIX(cl, pc);
      case X_GLXvop_HyperpipeConfigSGIX:
        return __glxHyperpipeConfigSGIX(cl, pc);
      case X_GLXvop_GetFBConfigsSGIX:
	return __glXGetFBConfigsSGIX(cl, pc);
      case X_GLXvop_CreateContextWithConfigSGIX:
	return __glXCreateContextWithConfigSGIX(cl, pc);
      case X_GLXvop_CreateGLXPixmapWithConfigSGIX:
	return __glXCreateGLXPixmapWithConfigSGIX(cl, pc);
      default:
	break;
    }

    if ((vendorcode >= __GLX_MIN_VENDPRIV_OPCODE_EXT) &&
          (vendorcode <= __GLX_MAX_VENDPRIV_OPCODE_EXT))  {
	return 
	(*__glXVendorPrivTable_EXT[vendorcode-__GLX_MIN_VENDPRIV_OPCODE_EXT])
							(cl, (GLbyte*)req);
    }

    cl->client->errorValue = vendorcode;
    return __glXUnsupportedPrivateRequest;
}

int __glXQueryExtensionsString(__GLXclientState *cl, GLbyte *pc)
{
    ClientPtr client = cl->client;
    xGLXQueryExtensionsStringReq *req = (xGLXQueryExtensionsStringReq *) pc;
    xGLXQueryExtensionsStringReply reply;
    GLuint screen;
    size_t n, length;
    const char *ptr;
    char *buf;

    screen = req->screen;
    /*
    ** Check if screen exists.
    */
    if (screen >= screenInfo.numScreens) {
	client->errorValue = screen;
	return BadValue;
    }

    ptr = __glXActiveScreens[screen].GLXextensions;

    n = __glXStrlen(ptr) + 1;
    length = __GLX_PAD(n) >> 2;
    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;
    reply.length = length;
    reply.n = n;

    if ((buf = (char *) __glXMalloc(length << 2)) == NULL) {
        return BadAlloc;
    }
    __glXStrncpy(buf, ptr, n);

    if (client->swapped) {
        glxSwapQueryExtensionsStringReply(client, &reply, buf);
    } else {
        WriteToClient(client, sz_xGLXQueryExtensionsStringReply,(char *)&reply);
        WriteToClient(client, (int)(length << 2), (char *)buf);
    }

    __glXFree(buf);
    return Success;
}

int __glXQueryServerString(__GLXclientState *cl, GLbyte *pc)
{
    ClientPtr client = cl->client;
    xGLXQueryServerStringReq *req = (xGLXQueryServerStringReq *) pc;
    xGLXQueryServerStringReply reply;
    int name;
    GLuint screen;
    size_t n, length;
    const char *ptr;
    char *buf;

    name = req->name;
    screen = req->screen;
    /*
    ** Check if screen exists.
    */
    if (screen >= screenInfo.numScreens) {
	client->errorValue = screen;
	return BadValue;
    }
    switch(name) {
	case GLX_VENDOR:
	    ptr = __glXActiveScreens[screen].GLXvendor;
	    break;
	case GLX_VERSION:
	    ptr = __glXActiveScreens[screen].GLXversion;
	    break;
	case GLX_EXTENSIONS:
	    ptr = __glXActiveScreens[screen].GLXextensions;
	    break;
	default:
	    return BadValue; 
    }

    n = __glXStrlen(ptr) + 1;
    length = __GLX_PAD(n) >> 2;
    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;
    reply.length = length;
    reply.n = n;

    if ((buf = (char *) Xalloc(length << 2)) == NULL) {
        return BadAlloc;
    }
    __glXStrncpy(buf, ptr, n);

    if (client->swapped) {
        glxSwapQueryServerStringReply(client, &reply, buf);
    } else {
        WriteToClient(client, sz_xGLXQueryServerStringReply, (char *)&reply);
        WriteToClient(client, (int)(length << 2), buf);
    }

    __glXFree(buf);
    return Success;
}

int __glXClientInfo(__GLXclientState *cl, GLbyte *pc)
{
    xGLXClientInfoReq *req = (xGLXClientInfoReq *) pc;
    const char *buf;
   
    cl->GLClientmajorVersion = req->major;
    cl->GLClientminorVersion = req->minor;
    if (cl->GLClientextensions) __glXFree(cl->GLClientextensions);
    buf = (const char *)(req+1);
    cl->GLClientextensions = __glXStrdup(buf);

    return Success;
}

