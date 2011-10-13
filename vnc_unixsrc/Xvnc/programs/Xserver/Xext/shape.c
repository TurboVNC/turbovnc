/* $XdotOrg: xc/programs/Xserver/Xext/shape.c,v 1.9 2005/07/03 08:53:36 daniels Exp $ */
/* $XFree86: xc/programs/Xserver/Xext/shape.c,v 3.18 2003/10/28 23:08:43 tsi Exp $ */
/************************************************************

Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

********************************************************/

/* $Xorg: shape.c,v 1.4 2001/02/09 02:04:32 xorgcvs Exp $ */
#define NEED_REPLIES
#define NEED_EVENTS
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "os.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "extnsionst.h"
#include "dixstruct.h"
#include "resource.h"
#include "opaque.h"
#define _SHAPE_SERVER_	/* don't want Xlib structures */
#include <X11/extensions/shapestr.h>
#include "regionstr.h"
#include "gcstruct.h"
#ifdef EXTMODULE
#include "xf86_ansic.h"
#endif
#include "modinit.h"

typedef	RegionPtr (*CreateDftPtr)(
	WindowPtr /* pWin */
	);

static int ShapeFreeClient(
	pointer /* data */,
	XID /* id */
	);
static int ShapeFreeEvents(
	pointer /* data */,
	XID /* id */
	);
static void ShapeResetProc(
	ExtensionEntry * /* extEntry */
	);
static void SShapeNotifyEvent(
	xShapeNotifyEvent * /* from */,
	xShapeNotifyEvent * /* to */
	);
static int
RegionOperate (
	ClientPtr /* client */,
	WindowPtr /* pWin */,
	int /* kind */,
	RegionPtr * /* destRgnp */,
	RegionPtr /* srcRgn */,
	int /* op */,
	int /* xoff */,
	int /* yoff */,
	CreateDftPtr /* create */
	);

/* SendShapeNotify, CreateBoundingShape and CreateClipShape are used
 * externally by the Xfixes extension and are now defined in window.h
 */

static DISPATCH_PROC(ProcShapeCombine);
static DISPATCH_PROC(ProcShapeDispatch);
static DISPATCH_PROC(ProcShapeGetRectangles);
static DISPATCH_PROC(ProcShapeInputSelected);
static DISPATCH_PROC(ProcShapeMask);
static DISPATCH_PROC(ProcShapeOffset);
static DISPATCH_PROC(ProcShapeQueryExtents);
static DISPATCH_PROC(ProcShapeQueryVersion);
static DISPATCH_PROC(ProcShapeRectangles);
static DISPATCH_PROC(ProcShapeSelectInput);
static DISPATCH_PROC(SProcShapeCombine);
static DISPATCH_PROC(SProcShapeDispatch);
static DISPATCH_PROC(SProcShapeGetRectangles);
static DISPATCH_PROC(SProcShapeInputSelected);
static DISPATCH_PROC(SProcShapeMask);
static DISPATCH_PROC(SProcShapeOffset);
static DISPATCH_PROC(SProcShapeQueryExtents);
static DISPATCH_PROC(SProcShapeQueryVersion);
static DISPATCH_PROC(SProcShapeRectangles);
static DISPATCH_PROC(SProcShapeSelectInput);

#ifdef PANORAMIX
#include "panoramiX.h"
#include "panoramiXsrv.h"
#endif

#if 0
static unsigned char ShapeReqCode = 0;
#endif
static int ShapeEventBase = 0;
static RESTYPE ClientType, EventType; /* resource types for event masks */

/*
 * each window has a list of clients requesting
 * ShapeNotify events.  Each client has a resource
 * for each window it selects ShapeNotify input for,
 * this resource is used to delete the ShapeNotifyRec
 * entry from the per-window queue.
 */

typedef struct _ShapeEvent *ShapeEventPtr;

typedef struct _ShapeEvent {
    ShapeEventPtr   next;
    ClientPtr	    client;
    WindowPtr	    window;
    XID		    clientResource;
} ShapeEventRec;

/****************
 * ShapeExtensionInit
 *
 * Called from InitExtensions in main() or from QueryExtension() if the
 * extension is dynamically loaded.
 *
 ****************/

void
ShapeExtensionInit(INITARGS)
{
    ExtensionEntry *extEntry;

    ClientType = CreateNewResourceType(ShapeFreeClient);
    EventType = CreateNewResourceType(ShapeFreeEvents);
    if (ClientType && EventType &&
	(extEntry = AddExtension(SHAPENAME, ShapeNumberEvents, 0,
				 ProcShapeDispatch, SProcShapeDispatch,
				 ShapeResetProc, StandardMinorOpcode)))
    {
#if 0
	ShapeReqCode = (unsigned char)extEntry->base;
#endif
	ShapeEventBase = extEntry->eventBase;
	EventSwapVector[ShapeEventBase] = (EventSwapPtr) SShapeNotifyEvent;
    }
}

/*ARGSUSED*/
static void
ShapeResetProc (extEntry)
ExtensionEntry	*extEntry;
{
}

static int
RegionOperate (client, pWin, kind, destRgnp, srcRgn, op, xoff, yoff, create)
    ClientPtr	client;
    WindowPtr	pWin;
    int		kind;
    RegionPtr	*destRgnp, srcRgn;
    int		op;
    int		xoff, yoff;
    CreateDftPtr create;	/* creates a reasonable *destRgnp */
{
    ScreenPtr	pScreen = pWin->drawable.pScreen;

    if (srcRgn && (xoff || yoff))
	REGION_TRANSLATE(pScreen, srcRgn, xoff, yoff);
    if (!pWin->parent)
    {
	if (srcRgn)
	    REGION_DESTROY(pScreen, srcRgn);
	return Success;
    }

    /* May/30/2001:
     * The shape.PS specs say if src is None, existing shape is to be
     * removed (and so the op-code has no meaning in such removal);
     * see shape.PS, page 3, ShapeMask.
     */
    if (srcRgn == NULL) {
      if (*destRgnp != NULL) {
	REGION_DESTROY (pScreen, *destRgnp);
	*destRgnp = 0;
	/* go on to remove shape and generate ShapeNotify */
      }
      else {
	/* May/30/2001:
	 * The target currently has no shape in effect, so nothing to
	 * do here.  The specs say that ShapeNotify is generated whenever
	 * the client region is "modified"; since no modification is done
	 * here, we do not generate that event.  The specs does not say
	 * "it is an error to request removal when there is no shape in
	 * effect", so we return good status.
	 */
	return Success;
      }
    }
    else switch (op) {
    case ShapeSet:
	if (*destRgnp)
	    REGION_DESTROY(pScreen, *destRgnp);
	*destRgnp = srcRgn;
	srcRgn = 0;
	break;
    case ShapeUnion:
	if (*destRgnp)
	    REGION_UNION(pScreen, *destRgnp, *destRgnp, srcRgn);
	break;
    case ShapeIntersect:
	if (*destRgnp)
	    REGION_INTERSECT(pScreen, *destRgnp, *destRgnp, srcRgn);
	else {
	    *destRgnp = srcRgn;
	    srcRgn = 0;
	}
	break;
    case ShapeSubtract:
	if (!*destRgnp)
	    *destRgnp = (*create)(pWin);
	REGION_SUBTRACT(pScreen, *destRgnp, *destRgnp, srcRgn);
	break;
    case ShapeInvert:
	if (!*destRgnp)
	    *destRgnp = REGION_CREATE(pScreen, (BoxPtr) 0, 0);
	else
	    REGION_SUBTRACT(pScreen, *destRgnp, srcRgn, *destRgnp);
	break;
    default:
	client->errorValue = op;
	return BadValue;
    }
    if (srcRgn)
	REGION_DESTROY(pScreen, srcRgn);
    (*pScreen->SetShape) (pWin);
    SendShapeNotify (pWin, kind);
    return Success;
}

RegionPtr
CreateBoundingShape (pWin)
    WindowPtr	pWin;
{
    BoxRec	extents;

    extents.x1 = -wBorderWidth (pWin);
    extents.y1 = -wBorderWidth (pWin);
    extents.x2 = pWin->drawable.width + wBorderWidth (pWin);
    extents.y2 = pWin->drawable.height + wBorderWidth (pWin);
    return REGION_CREATE(pWin->drawable.pScreen, &extents, 1);
}

RegionPtr
CreateClipShape (pWin)
    WindowPtr	pWin;
{
    BoxRec	extents;

    extents.x1 = 0;
    extents.y1 = 0;
    extents.x2 = pWin->drawable.width;
    extents.y2 = pWin->drawable.height;
    return REGION_CREATE(pWin->drawable.pScreen, &extents, 1);
}

static int
ProcShapeQueryVersion (client)
    register ClientPtr	client;
{
    xShapeQueryVersionReply	rep;
    register int		n;

    REQUEST_SIZE_MATCH (xShapeQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = SHAPE_MAJOR_VERSION;
    rep.minorVersion = SHAPE_MINOR_VERSION;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swaps(&rep.majorVersion, n);
	swaps(&rep.minorVersion, n);
    }
    WriteToClient(client, sizeof (xShapeQueryVersionReply), (char *)&rep);
    return (client->noClientException);
}

/*****************
 * ProcShapeRectangles
 *
 *****************/

static int
ProcShapeRectangles (client)
    register ClientPtr client;
{
    WindowPtr		pWin;
    ScreenPtr		pScreen;
    REQUEST(xShapeRectanglesReq);
    xRectangle		*prects;
    int		        nrects, ctype;
    RegionPtr		srcRgn;
    RegionPtr		*destRgn;
    CreateDftPtr	createDefault;

    REQUEST_AT_LEAST_SIZE (xShapeRectanglesReq);
    UpdateCurrentTime();
    pWin = LookupWindow (stuff->dest, client);
    if (!pWin)
	return BadWindow;
    switch (stuff->destKind) {
    case ShapeBounding:
	createDefault = CreateBoundingShape;
	break;
    case ShapeClip:
	createDefault = CreateClipShape;
	break;
    case ShapeInput:
	createDefault = CreateBoundingShape;
	break;
    default:
	client->errorValue = stuff->destKind;
	return BadValue;
    }
    if ((stuff->ordering != Unsorted) && (stuff->ordering != YSorted) &&
	(stuff->ordering != YXSorted) && (stuff->ordering != YXBanded))
    {
	client->errorValue = stuff->ordering;
        return BadValue;
    }
    pScreen = pWin->drawable.pScreen;
    nrects = ((stuff->length  << 2) - sizeof(xShapeRectanglesReq));
    if (nrects & 4)
	return BadLength;
    nrects >>= 3;
    prects = (xRectangle *) &stuff[1];
    ctype = VerifyRectOrder(nrects, prects, (int)stuff->ordering);
    if (ctype < 0)
	return BadMatch;
    srcRgn = RECTS_TO_REGION(pScreen, nrects, prects, ctype);

    if (!pWin->optional)
	MakeWindowOptional (pWin);
    switch (stuff->destKind) {
    case ShapeBounding:
	destRgn = &pWin->optional->boundingShape;
	break;
    case ShapeClip:
	destRgn = &pWin->optional->clipShape;
	break;
    case ShapeInput:
	destRgn = &pWin->optional->inputShape;
	break;
    default:
	return BadValue;
    }

    return RegionOperate (client, pWin, (int)stuff->destKind,
			  destRgn, srcRgn, (int)stuff->op,
			  stuff->xOff, stuff->yOff, createDefault);
}

#ifdef PANORAMIX
static int
ProcPanoramiXShapeRectangles(
    register ClientPtr client)
{
    REQUEST(xShapeRectanglesReq);
    PanoramiXRes	*win;
    int        		j, result = 0;

    REQUEST_AT_LEAST_SIZE (xShapeRectanglesReq);

    if(!(win = (PanoramiXRes *)SecurityLookupIDByType(
		client, stuff->dest, XRT_WINDOW, SecurityWriteAccess)))
	return BadWindow;

    FOR_NSCREENS(j) {
	stuff->dest = win->info[j].id;
	result = ProcShapeRectangles (client);
	BREAK_IF(result != Success);
    }
    return (result);
}
#endif


/**************
 * ProcShapeMask
 **************/


static int
ProcShapeMask (client)
    register ClientPtr client;
{
    WindowPtr		pWin;
    ScreenPtr		pScreen;
    REQUEST(xShapeMaskReq);
    RegionPtr		srcRgn;
    RegionPtr		*destRgn;
    PixmapPtr		pPixmap;
    CreateDftPtr	createDefault;

    REQUEST_SIZE_MATCH (xShapeMaskReq);
    UpdateCurrentTime();
    pWin = SecurityLookupWindow (stuff->dest, client, SecurityWriteAccess);
    if (!pWin)
	return BadWindow;
    switch (stuff->destKind) {
    case ShapeBounding:
	createDefault = CreateBoundingShape;
	break;
    case ShapeClip:
	createDefault = CreateClipShape;
	break;
    case ShapeInput:
	createDefault = CreateBoundingShape;
	break;
    default:
	client->errorValue = stuff->destKind;
	return BadValue;
    }
    pScreen = pWin->drawable.pScreen;
    if (stuff->src == None)
	srcRgn = 0;
    else {
        pPixmap = (PixmapPtr) SecurityLookupIDByType(client, stuff->src,
						RT_PIXMAP, SecurityReadAccess);
        if (!pPixmap)
	    return BadPixmap;
	if (pPixmap->drawable.pScreen != pScreen ||
	    pPixmap->drawable.depth != 1)
	    return BadMatch;
	srcRgn = BITMAP_TO_REGION(pScreen, pPixmap);
	if (!srcRgn)
	    return BadAlloc;
    }

    if (!pWin->optional)
	MakeWindowOptional (pWin);
    switch (stuff->destKind) {
    case ShapeBounding:
	destRgn = &pWin->optional->boundingShape;
	break;
    case ShapeClip:
	destRgn = &pWin->optional->clipShape;
	break;
    case ShapeInput:
	destRgn = &pWin->optional->inputShape;
	break;
    default:
	return BadValue;
    }

    return RegionOperate (client, pWin, (int)stuff->destKind,
			  destRgn, srcRgn, (int)stuff->op,
			  stuff->xOff, stuff->yOff, createDefault);
}

#ifdef PANORAMIX
static int
ProcPanoramiXShapeMask(
    register ClientPtr client)
{
    REQUEST(xShapeMaskReq);
    PanoramiXRes	*win, *pmap;
    int 		j, result = 0;

    REQUEST_SIZE_MATCH (xShapeMaskReq);

    if(!(win = (PanoramiXRes *)SecurityLookupIDByType(
		client, stuff->dest, XRT_WINDOW, SecurityWriteAccess)))
	return BadWindow;

    if(stuff->src != None) {
	if(!(pmap = (PanoramiXRes *)SecurityLookupIDByType(
		client, stuff->src, XRT_PIXMAP, SecurityReadAccess)))
	    return BadPixmap;
    } else
	pmap = NULL;

    FOR_NSCREENS(j) {
	stuff->dest = win->info[j].id;
	if(pmap)
	    stuff->src  = pmap->info[j].id;
	result = ProcShapeMask (client);
	BREAK_IF(result != Success);
    }
    return (result);
}
#endif


/************
 * ProcShapeCombine
 ************/

static int
ProcShapeCombine (client)
    register ClientPtr client;
{
    WindowPtr		pSrcWin, pDestWin;
    ScreenPtr		pScreen;
    REQUEST(xShapeCombineReq);
    RegionPtr		srcRgn;
    RegionPtr		*destRgn;
    CreateDftPtr	createDefault;
    CreateDftPtr	createSrc;
    RegionPtr		tmp;

    REQUEST_SIZE_MATCH (xShapeCombineReq);
    UpdateCurrentTime();
    pDestWin = LookupWindow (stuff->dest, client);
    if (!pDestWin)
	return BadWindow;
    if (!pDestWin->optional)
	MakeWindowOptional (pDestWin);
    switch (stuff->destKind) {
    case ShapeBounding:
	createDefault = CreateBoundingShape;
	break;
    case ShapeClip:
	createDefault = CreateClipShape;
	break;
    case ShapeInput:
	createDefault = CreateBoundingShape;
	break;
    default:
	client->errorValue = stuff->destKind;
	return BadValue;
    }
    pScreen = pDestWin->drawable.pScreen;

    pSrcWin = LookupWindow (stuff->src, client);
    if (!pSrcWin)
	return BadWindow;
    switch (stuff->srcKind) {
    case ShapeBounding:
	srcRgn = wBoundingShape (pSrcWin);
	createSrc = CreateBoundingShape;
	break;
    case ShapeClip:
	srcRgn = wClipShape (pSrcWin);
	createSrc = CreateClipShape;
	break;
    case ShapeInput:
	srcRgn = wInputShape (pSrcWin);
	createSrc = CreateBoundingShape;
	break;
    default:
	client->errorValue = stuff->srcKind;
	return BadValue;
    }
    if (pSrcWin->drawable.pScreen != pScreen)
    {
	return BadMatch;
    }

    if (srcRgn) {
        tmp = REGION_CREATE(pScreen, (BoxPtr) 0, 0);
        REGION_COPY(pScreen, tmp, srcRgn);
        srcRgn = tmp;
    } else
	srcRgn = (*createSrc) (pSrcWin);

    if (!pDestWin->optional)
	MakeWindowOptional (pDestWin);
    switch (stuff->destKind) {
    case ShapeBounding:
	destRgn = &pDestWin->optional->boundingShape;
	break;
    case ShapeClip:
	destRgn = &pDestWin->optional->clipShape;
	break;
    case ShapeInput:
	destRgn = &pDestWin->optional->inputShape;
	break;
    default:
	return BadValue;
    }

    return RegionOperate (client, pDestWin, (int)stuff->destKind,
			  destRgn, srcRgn, (int)stuff->op,
			  stuff->xOff, stuff->yOff, createDefault);
}


#ifdef PANORAMIX
static int
ProcPanoramiXShapeCombine(
    register ClientPtr client)
{
    REQUEST(xShapeCombineReq);
    PanoramiXRes	*win, *win2;
    int 		j, result = 0;

    REQUEST_AT_LEAST_SIZE (xShapeCombineReq);

    if(!(win = (PanoramiXRes *)SecurityLookupIDByType(
		client, stuff->dest, XRT_WINDOW, SecurityWriteAccess)))
	return BadWindow;

    if(!(win2 = (PanoramiXRes *)SecurityLookupIDByType(
		client, stuff->src, XRT_WINDOW, SecurityReadAccess)))
	return BadWindow;

    FOR_NSCREENS(j) {
	stuff->dest = win->info[j].id;
	stuff->src =  win2->info[j].id;
	result = ProcShapeCombine (client);
	BREAK_IF(result != Success);
    }
    return (result);
}
#endif

/*************
 * ProcShapeOffset
 *************/

static int
ProcShapeOffset (client)
    register ClientPtr client;
{
    WindowPtr		pWin;
    ScreenPtr		pScreen;
    REQUEST(xShapeOffsetReq);
    RegionPtr		srcRgn;

    REQUEST_SIZE_MATCH (xShapeOffsetReq);
    UpdateCurrentTime();
    pWin = LookupWindow (stuff->dest, client);
    if (!pWin)
	return BadWindow;
    switch (stuff->destKind) {
    case ShapeBounding:
	srcRgn = wBoundingShape (pWin);
	break;
    case ShapeClip:
	srcRgn = wClipShape(pWin);
	break;
    case ShapeInput:
	srcRgn = wInputShape (pWin);
	break;
    default:
	client->errorValue = stuff->destKind;
	return BadValue;
    }
    pScreen = pWin->drawable.pScreen;
    if (srcRgn)
    {
        REGION_TRANSLATE(pScreen, srcRgn, stuff->xOff, stuff->yOff);
        (*pScreen->SetShape) (pWin);
    }
    SendShapeNotify (pWin, (int)stuff->destKind);
    return Success;
}


#ifdef PANORAMIX
static int
ProcPanoramiXShapeOffset(
    register ClientPtr client)
{
    REQUEST(xShapeOffsetReq);
    PanoramiXRes *win;
    int j, result = 0;

    REQUEST_AT_LEAST_SIZE (xShapeOffsetReq);
   
    if(!(win = (PanoramiXRes *)SecurityLookupIDByType(
		client, stuff->dest, XRT_WINDOW, SecurityWriteAccess)))
	return BadWindow;

    FOR_NSCREENS(j) {
	stuff->dest = win->info[j].id;
	result = ProcShapeOffset (client);
	if(result != Success) break;
    }
    return (result);
}
#endif


static int
ProcShapeQueryExtents (client)
    register ClientPtr	client;
{
    REQUEST(xShapeQueryExtentsReq);
    WindowPtr		pWin;
    xShapeQueryExtentsReply	rep;
    BoxRec		extents, *pExtents;
    register int	n;
    RegionPtr		region;

    REQUEST_SIZE_MATCH (xShapeQueryExtentsReq);
    pWin = LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.boundingShaped = (wBoundingShape(pWin) != 0);
    rep.clipShaped = (wClipShape(pWin) != 0);
    if ((region = wBoundingShape(pWin))) {
     /* this is done in two steps because of a compiler bug on SunOS 4.1.3 */
	pExtents = REGION_EXTENTS(pWin->drawable.pScreen, region);
	extents = *pExtents;
    } else {
	extents.x1 = -wBorderWidth (pWin);
	extents.y1 = -wBorderWidth (pWin);
	extents.x2 = pWin->drawable.width + wBorderWidth (pWin);
	extents.y2 = pWin->drawable.height + wBorderWidth (pWin);
    }
    rep.xBoundingShape = extents.x1;
    rep.yBoundingShape = extents.y1;
    rep.widthBoundingShape = extents.x2 - extents.x1;
    rep.heightBoundingShape = extents.y2 - extents.y1;
    if ((region = wClipShape(pWin))) {
     /* this is done in two steps because of a compiler bug on SunOS 4.1.3 */
	pExtents = REGION_EXTENTS(pWin->drawable.pScreen, region);
	extents = *pExtents;
    } else {
	extents.x1 = 0;
	extents.y1 = 0;
	extents.x2 = pWin->drawable.width;
	extents.y2 = pWin->drawable.height;
    }
    rep.xClipShape = extents.x1;
    rep.yClipShape = extents.y1;
    rep.widthClipShape = extents.x2 - extents.x1;
    rep.heightClipShape = extents.y2 - extents.y1;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swaps(&rep.xBoundingShape, n);
	swaps(&rep.yBoundingShape, n);
	swaps(&rep.widthBoundingShape, n);
	swaps(&rep.heightBoundingShape, n);
	swaps(&rep.xClipShape, n);
	swaps(&rep.yClipShape, n);
	swaps(&rep.widthClipShape, n);
	swaps(&rep.heightClipShape, n);
    }
    WriteToClient(client, sizeof (xShapeQueryExtentsReply), (char *)&rep);
    return (client->noClientException);
}

/*ARGSUSED*/
static int
ShapeFreeClient (data, id)
    pointer	    data;
    XID		    id;
{
    ShapeEventPtr   pShapeEvent;
    WindowPtr	    pWin;
    ShapeEventPtr   *pHead, pCur, pPrev;

    pShapeEvent = (ShapeEventPtr) data;
    pWin = pShapeEvent->window;
    pHead = (ShapeEventPtr *) LookupIDByType(pWin->drawable.id, EventType);
    if (pHead) {
	pPrev = 0;
	for (pCur = *pHead; pCur && pCur != pShapeEvent; pCur=pCur->next)
	    pPrev = pCur;
	if (pCur)
	{
	    if (pPrev)
	    	pPrev->next = pShapeEvent->next;
	    else
	    	*pHead = pShapeEvent->next;
	}
    }
    xfree ((pointer) pShapeEvent);
    return 1;
}

/*ARGSUSED*/
static int
ShapeFreeEvents (data, id)
    pointer	    data;
    XID		    id;
{
    ShapeEventPtr   *pHead, pCur, pNext;

    pHead = (ShapeEventPtr *) data;
    for (pCur = *pHead; pCur; pCur = pNext) {
	pNext = pCur->next;
	FreeResource (pCur->clientResource, ClientType);
	xfree ((pointer) pCur);
    }
    xfree ((pointer) pHead);
    return 1;
}

static int
ProcShapeSelectInput (client)
    register ClientPtr	client;
{
    REQUEST(xShapeSelectInputReq);
    WindowPtr		pWin;
    ShapeEventPtr	pShapeEvent, pNewShapeEvent, *pHead;
    XID			clientResource;

    REQUEST_SIZE_MATCH (xShapeSelectInputReq);
    pWin = SecurityLookupWindow (stuff->window, client, SecurityWriteAccess);
    if (!pWin)
	return BadWindow;
    pHead = (ShapeEventPtr *)SecurityLookupIDByType(client,
			pWin->drawable.id, EventType, SecurityWriteAccess);
    switch (stuff->enable) {
    case xTrue:
	if (pHead) {

	    /* check for existing entry. */
	    for (pShapeEvent = *pHead;
		 pShapeEvent;
 		 pShapeEvent = pShapeEvent->next)
	    {
		if (pShapeEvent->client == client)
		    return Success;
	    }
	}

	/* build the entry */
    	pNewShapeEvent = (ShapeEventPtr)
			    xalloc (sizeof (ShapeEventRec));
    	if (!pNewShapeEvent)
	    return BadAlloc;
    	pNewShapeEvent->next = 0;
    	pNewShapeEvent->client = client;
    	pNewShapeEvent->window = pWin;
    	/*
 	 * add a resource that will be deleted when
     	 * the client goes away
     	 */
   	clientResource = FakeClientID (client->index);
    	pNewShapeEvent->clientResource = clientResource;
    	if (!AddResource (clientResource, ClientType, (pointer)pNewShapeEvent))
	    return BadAlloc;
    	/*
     	 * create a resource to contain a pointer to the list
     	 * of clients selecting input.  This must be indirect as
     	 * the list may be arbitrarily rearranged which cannot be
     	 * done through the resource database.
     	 */
    	if (!pHead)
    	{
	    pHead = (ShapeEventPtr *) xalloc (sizeof (ShapeEventPtr));
	    if (!pHead ||
	    	!AddResource (pWin->drawable.id, EventType, (pointer)pHead))
	    {
	    	FreeResource (clientResource, RT_NONE);
	    	return BadAlloc;
	    }
	    *pHead = 0;
    	}
    	pNewShapeEvent->next = *pHead;
    	*pHead = pNewShapeEvent;
	break;
    case xFalse:
	/* delete the interest */
	if (pHead) {
	    pNewShapeEvent = 0;
	    for (pShapeEvent = *pHead; pShapeEvent; pShapeEvent = pShapeEvent->next) {
		if (pShapeEvent->client == client)
		    break;
		pNewShapeEvent = pShapeEvent;
	    }
	    if (pShapeEvent) {
		FreeResource (pShapeEvent->clientResource, ClientType);
		if (pNewShapeEvent)
		    pNewShapeEvent->next = pShapeEvent->next;
		else
		    *pHead = pShapeEvent->next;
		xfree (pShapeEvent);
	    }
	}
	break;
    default:
	client->errorValue = stuff->enable;
	return BadValue;
    }
    return Success;
}

/*
 * deliver the event
 */

void
SendShapeNotify (pWin, which)
    WindowPtr	pWin;
    int		which;
{
    ShapeEventPtr	*pHead, pShapeEvent;
    ClientPtr		client;
    xShapeNotifyEvent	se;
    BoxRec		extents;
    RegionPtr		region;
    BYTE		shaped;

    pHead = (ShapeEventPtr *) LookupIDByType(pWin->drawable.id, EventType);
    if (!pHead)
	return;
    switch (which) {
    case ShapeBounding:
	region = wBoundingShape(pWin);
	if (region) {
	    extents = *REGION_EXTENTS(pWin->drawable.pScreen, region);
	    shaped = xTrue;
	} else {
	    extents.x1 = -wBorderWidth (pWin);
	    extents.y1 = -wBorderWidth (pWin);
	    extents.x2 = pWin->drawable.width + wBorderWidth (pWin);
	    extents.y2 = pWin->drawable.height + wBorderWidth (pWin);
	    shaped = xFalse;
	}
	break;
    case ShapeClip:
	region = wClipShape(pWin);
	if (region) {
	    extents = *REGION_EXTENTS(pWin->drawable.pScreen, region);
	    shaped = xTrue;
	} else {
	    extents.x1 = 0;
	    extents.y1 = 0;
	    extents.x2 = pWin->drawable.width;
	    extents.y2 = pWin->drawable.height;
	    shaped = xFalse;
	}
	break;
    case ShapeInput:
	region = wInputShape(pWin);
	if (region) {
	    extents = *REGION_EXTENTS(pWin->drawable.pScreen, region);
	    shaped = xTrue;
	} else {
	    extents.x1 = -wBorderWidth (pWin);
	    extents.y1 = -wBorderWidth (pWin);
	    extents.x2 = pWin->drawable.width + wBorderWidth (pWin);
	    extents.y2 = pWin->drawable.height + wBorderWidth (pWin);
	    shaped = xFalse;
	}
	break;
    default:
	return;
    }
    for (pShapeEvent = *pHead; pShapeEvent; pShapeEvent = pShapeEvent->next) {
	client = pShapeEvent->client;
	if (client == serverClient || client->clientGone)
	    continue;
	se.type = ShapeNotify + ShapeEventBase;
	se.kind = which;
	se.window = pWin->drawable.id;
	se.sequenceNumber = client->sequence;
	se.x = extents.x1;
	se.y = extents.y1;
	se.width = extents.x2 - extents.x1;
	se.height = extents.y2 - extents.y1;
	se.time = currentTime.milliseconds;
	se.shaped = shaped;
	WriteEventsToClient (client, 1, (xEvent *) &se);
    }
}

static int
ProcShapeInputSelected (client)
    register ClientPtr	client;
{
    REQUEST(xShapeInputSelectedReq);
    WindowPtr		pWin;
    ShapeEventPtr	pShapeEvent, *pHead;
    int			enabled;
    xShapeInputSelectedReply	rep;
    register int		n;

    REQUEST_SIZE_MATCH (xShapeInputSelectedReq);
    pWin = LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    pHead = (ShapeEventPtr *) SecurityLookupIDByType(client,
			pWin->drawable.id, EventType, SecurityReadAccess);
    enabled = xFalse;
    if (pHead) {
    	for (pShapeEvent = *pHead;
	     pShapeEvent;
	     pShapeEvent = pShapeEvent->next)
    	{
	    if (pShapeEvent->client == client) {
	    	enabled = xTrue;
		break;
	    }
    	}
    }
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.enabled = enabled;
    if (client->swapped) {
	swaps (&rep.sequenceNumber, n);
	swapl (&rep.length, n);
    }
    WriteToClient (client, sizeof (xShapeInputSelectedReply), (char *) &rep);
    return (client->noClientException);
}

static int
ProcShapeGetRectangles (client)
    register ClientPtr	client;
{
    REQUEST(xShapeGetRectanglesReq);
    WindowPtr			pWin;
    xShapeGetRectanglesReply	rep;
    xRectangle			*rects;
    int				nrects, i;
    RegionPtr			region;
    register int		n;

    REQUEST_SIZE_MATCH(xShapeGetRectanglesReq);
    pWin = LookupWindow (stuff->window, client);
    if (!pWin)
	return BadWindow;
    switch (stuff->kind) {
    case ShapeBounding:
	region = wBoundingShape(pWin);
	break;
    case ShapeClip:
	region = wClipShape(pWin);
	break;
    case ShapeInput:
	region = wInputShape (pWin);
	break;
    default:
	client->errorValue = stuff->kind;
	return BadValue;
    }
    if (!region) {
	nrects = 1;
	rects = (xRectangle *) ALLOCATE_LOCAL (sizeof (xRectangle));
	if (!rects)
	    return BadAlloc;
	switch (stuff->kind) {
	case ShapeBounding:
	    rects->x = - (int) wBorderWidth (pWin);
	    rects->y = - (int) wBorderWidth (pWin);
	    rects->width = pWin->drawable.width + wBorderWidth (pWin);
	    rects->height = pWin->drawable.height + wBorderWidth (pWin);
	    break;
	case ShapeClip:
	    rects->x = 0;
	    rects->y = 0;
	    rects->width = pWin->drawable.width;
	    rects->height = pWin->drawable.height;
	    break;
	case ShapeInput:
	    rects->x = - (int) wBorderWidth (pWin);
	    rects->y = - (int) wBorderWidth (pWin);
	    rects->width = pWin->drawable.width + wBorderWidth (pWin);
	    rects->height = pWin->drawable.height + wBorderWidth (pWin);
	    break;
	}
    } else {
	BoxPtr box;
	nrects = REGION_NUM_RECTS(region);
	box = REGION_RECTS(region);
	rects = (xRectangle *) ALLOCATE_LOCAL (nrects * sizeof (xRectangle));
	if (!rects && nrects)
	    return BadAlloc;
	for (i = 0; i < nrects; i++, box++) {
	    rects[i].x = box->x1;
	    rects[i].y = box->y1;
	    rects[i].width = box->x2 - box->x1;
	    rects[i].height = box->y2 - box->y1;
	}
    }
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = (nrects * sizeof (xRectangle)) >> 2;
    rep.ordering = YXBanded;
    rep.nrects = nrects;
    if (client->swapped) {
	swaps (&rep.sequenceNumber, n);
	swapl (&rep.length, n);
	swapl (&rep.nrects, n);
	SwapShorts ((short *)rects, (unsigned long)nrects * 4);
    }
    WriteToClient (client, sizeof (rep), (char *) &rep);
    WriteToClient (client, nrects * sizeof (xRectangle), (char *) rects);
    DEALLOCATE_LOCAL (rects);
    return client->noClientException;
}

static int
ProcShapeDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_ShapeQueryVersion:
	return ProcShapeQueryVersion (client);
    case X_ShapeRectangles:
#ifdef PANORAMIX
        if ( !noPanoramiXExtension )
	    return ProcPanoramiXShapeRectangles (client);
        else 
#endif
	return ProcShapeRectangles (client);
    case X_ShapeMask:
#ifdef PANORAMIX
        if ( !noPanoramiXExtension )
           return ProcPanoramiXShapeMask (client);
	else
#endif
	return ProcShapeMask (client);
    case X_ShapeCombine:
#ifdef PANORAMIX
        if ( !noPanoramiXExtension )
           return ProcPanoramiXShapeCombine (client);
	else
#endif
	return ProcShapeCombine (client);
    case X_ShapeOffset:
#ifdef PANORAMIX
        if ( !noPanoramiXExtension )
           return ProcPanoramiXShapeOffset (client);
	else
#endif
	return ProcShapeOffset (client);
    case X_ShapeQueryExtents:
	return ProcShapeQueryExtents (client);
    case X_ShapeSelectInput:
	return ProcShapeSelectInput (client);
    case X_ShapeInputSelected:
	return ProcShapeInputSelected (client);
    case X_ShapeGetRectangles:
	return ProcShapeGetRectangles (client);
    default:
	return BadRequest;
    }
}

static void
SShapeNotifyEvent(from, to)
    xShapeNotifyEvent *from, *to;
{
    to->type = from->type;
    to->kind = from->kind;
    cpswapl (from->window, to->window);
    cpswaps (from->sequenceNumber, to->sequenceNumber);
    cpswaps (from->x, to->x);
    cpswaps (from->y, to->y);
    cpswaps (from->width, to->width);
    cpswaps (from->height, to->height);
    cpswapl (from->time, to->time);
    to->shaped = from->shaped;
}

static int
SProcShapeQueryVersion (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xShapeQueryVersionReq);

    swaps (&stuff->length, n);
    return ProcShapeQueryVersion (client);
}

static int
SProcShapeRectangles (client)
    register ClientPtr	client;
{
    register char   n;
    REQUEST (xShapeRectanglesReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE (xShapeRectanglesReq);
    swapl (&stuff->dest, n);
    swaps (&stuff->xOff, n);
    swaps (&stuff->yOff, n);
    SwapRestS(stuff);
    return ProcShapeRectangles (client);
}

static int
SProcShapeMask (client)
    register ClientPtr	client;
{
    register char   n;
    REQUEST (xShapeMaskReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xShapeMaskReq);
    swapl (&stuff->dest, n);
    swaps (&stuff->xOff, n);
    swaps (&stuff->yOff, n);
    swapl (&stuff->src, n);
    return ProcShapeMask (client);
}

static int
SProcShapeCombine (client)
    register ClientPtr	client;
{
    register char   n;
    REQUEST (xShapeCombineReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xShapeCombineReq);
    swapl (&stuff->dest, n);
    swaps (&stuff->xOff, n);
    swaps (&stuff->yOff, n);
    swapl (&stuff->src, n);
    return ProcShapeCombine (client);
}

static int
SProcShapeOffset (client)
    register ClientPtr	client;
{
    register char   n;
    REQUEST (xShapeOffsetReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xShapeOffsetReq);
    swapl (&stuff->dest, n);
    swaps (&stuff->xOff, n);
    swaps (&stuff->yOff, n);
    return ProcShapeOffset (client);
}

static int
SProcShapeQueryExtents (client)
    register ClientPtr	client;
{
    register char   n;
    REQUEST (xShapeQueryExtentsReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xShapeQueryExtentsReq);
    swapl (&stuff->window, n);
    return ProcShapeQueryExtents (client);
}

static int
SProcShapeSelectInput (client)
    register ClientPtr	client;
{
    register char   n;
    REQUEST (xShapeSelectInputReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xShapeSelectInputReq);
    swapl (&stuff->window, n);
    return ProcShapeSelectInput (client);
}

static int
SProcShapeInputSelected (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xShapeInputSelectedReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xShapeInputSelectedReq);
    swapl (&stuff->window, n);
    return ProcShapeInputSelected (client);
}

static int
SProcShapeGetRectangles (client)
    register ClientPtr	client;
{
    REQUEST(xShapeGetRectanglesReq);
    register char   n;

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH(xShapeGetRectanglesReq);
    swapl (&stuff->window, n);
    return ProcShapeGetRectangles (client);
}

static int
SProcShapeDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_ShapeQueryVersion:
	return SProcShapeQueryVersion (client);
    case X_ShapeRectangles:
	return SProcShapeRectangles (client);
    case X_ShapeMask:
	return SProcShapeMask (client);
    case X_ShapeCombine:
	return SProcShapeCombine (client);
    case X_ShapeOffset:
	return SProcShapeOffset (client);
    case X_ShapeQueryExtents:
	return SProcShapeQueryExtents (client);
    case X_ShapeSelectInput:
	return SProcShapeSelectInput (client);
    case X_ShapeInputSelected:
	return SProcShapeInputSelected (client);
    case X_ShapeGetRectangles:
	return SProcShapeGetRectangles (client);
    default:
	return BadRequest;
    }
}
