/***********************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* $XConsortium: miexpose.c /main/43 1996/08/01 19:25:26 dpw $ */
/* $XFree86: xc/programs/Xserver/mi/miexpose.c,v 3.1 1996/12/23 07:09:44 dawes Exp $ */

#include "X.h"
#define NEED_EVENTS
#include "Xproto.h"
#include "Xprotostr.h"

#include "misc.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmap.h"
#include "input.h"

#include "dixstruct.h"
#include "mi.h"
#include "Xmd.h"

extern WindowPtr *WindowTable;

/*
    machine-independent graphics exposure code.  any device that uses
the region package can call this.
*/

#ifndef RECTLIMIT
#define RECTLIMIT 25		/* pick a number, any number > 8 */
#endif

/* miHandleExposures 
    generate a region for exposures for areas that were copied from obscured or
non-existent areas to non-obscured areas of the destination.  Paint the
background for the region, if the destination is a window.

NOTE:
     this should generally be called, even if graphicsExposures is false,
because this is where bits get recovered from backing store.

NOTE:
     added argument 'plane' is used to indicate how exposures from backing
store should be accomplished. If plane is 0 (i.e. no bit plane), CopyArea
should be used, else a CopyPlane of the indicated plane will be used. The
exposing is done by the backing store's GraphicsExpose function, of course.

*/

RegionPtr
miHandleExposures(pSrcDrawable, pDstDrawable,
		  pGC, srcx, srcy, width, height, dstx, dsty, plane)
    register DrawablePtr	pSrcDrawable;
    register DrawablePtr	pDstDrawable;
    GCPtr 			pGC;
    int 			srcx, srcy;
    int 			width, height;
    int 			dstx, dsty;
    unsigned long		plane;
{
    register ScreenPtr pscr = pGC->pScreen;
    RegionPtr prgnSrcClip;	/* drawable-relative source clip */
    RegionRec rgnSrcRec;
    RegionPtr prgnDstClip;	/* drawable-relative dest clip */
    RegionRec rgnDstRec;
    BoxRec srcBox;		/* unclipped source */
    RegionRec rgnExposed;	/* exposed region, calculated source-
				   relative, made dst relative to
				   intersect with visible parts of
				   dest and send events to client, 
				   and then screen relative to paint 
				   the window background
				*/
    WindowPtr pSrcWin;
    BoxRec expBox;
    Bool extents;

    /* avoid work if we can */
    if (!pGC->graphicsExposures &&
	(pDstDrawable->type == DRAWABLE_PIXMAP) &&
	((pSrcDrawable->type == DRAWABLE_PIXMAP) ||
	 (((WindowPtr)pSrcDrawable)->backStorage == NULL)))
	return NULL;
	
    srcBox.x1 = srcx;
    srcBox.y1 = srcy;
    srcBox.x2 = srcx+width;
    srcBox.y2 = srcy+height;

    if (pSrcDrawable->type != DRAWABLE_PIXMAP)
    {
	BoxRec TsrcBox;

	TsrcBox.x1 = srcx + pSrcDrawable->x;
	TsrcBox.y1 = srcy + pSrcDrawable->y;
	TsrcBox.x2 = TsrcBox.x1 + width;
	TsrcBox.y2 = TsrcBox.y1 + height;
	pSrcWin = (WindowPtr) pSrcDrawable;
	if (pGC->subWindowMode == IncludeInferiors)
 	{
	    prgnSrcClip = NotClippedByChildren (pSrcWin);
	    if ((RECT_IN_REGION(pscr, prgnSrcClip, &TsrcBox)) == rgnIN)
	    {
		REGION_DESTROY(pscr, prgnSrcClip);
		return NULL;
	    }
	}
 	else
 	{
	    if ((RECT_IN_REGION(pscr, &pSrcWin->clipList, &TsrcBox)) == rgnIN)
		return NULL;
	    prgnSrcClip = &rgnSrcRec;
	    REGION_INIT(pscr, prgnSrcClip, NullBox, 0);
	    REGION_COPY(pscr, prgnSrcClip, &pSrcWin->clipList);
	}
	REGION_TRANSLATE(pscr, prgnSrcClip,
				-pSrcDrawable->x, -pSrcDrawable->y);
    }
    else
    {
	BoxRec	box;

	if ((srcBox.x1 >= 0) && (srcBox.y1 >= 0) &&
	    (srcBox.x2 <= pSrcDrawable->width) &&
 	    (srcBox.y2 <= pSrcDrawable->height))
	    return NULL;

	box.x1 = 0;
	box.y1 = 0;
	box.x2 = pSrcDrawable->width;
	box.y2 = pSrcDrawable->height;
	prgnSrcClip = &rgnSrcRec;
	REGION_INIT(pscr, prgnSrcClip, &box, 1);
	pSrcWin = (WindowPtr)NULL;
    }

    if (pDstDrawable == pSrcDrawable)
    {
	prgnDstClip = prgnSrcClip;
    }
    else if (pDstDrawable->type != DRAWABLE_PIXMAP)
    {
	if (pGC->subWindowMode == IncludeInferiors)
	{
	    prgnDstClip = NotClippedByChildren((WindowPtr)pDstDrawable);
	}
	else
	{
	    prgnDstClip = &rgnDstRec;
	    REGION_INIT(pscr, prgnDstClip, NullBox, 0);
	    REGION_COPY(pscr, prgnDstClip,
				&((WindowPtr)pDstDrawable)->clipList);
	}
	REGION_TRANSLATE(pscr, prgnDstClip,
				 -pDstDrawable->x, -pDstDrawable->y);
    }
    else
    {
	BoxRec	box;

	box.x1 = 0;
	box.y1 = 0;
	box.x2 = pDstDrawable->width;
	box.y2 = pDstDrawable->height;
	prgnDstClip = &rgnDstRec;
	REGION_INIT(pscr, prgnDstClip, &box, 1);
    }

    /* drawable-relative source region */
    REGION_INIT(pscr, &rgnExposed, &srcBox, 1);

    /* now get the hidden parts of the source box*/
    REGION_SUBTRACT(pscr, &rgnExposed, &rgnExposed, prgnSrcClip);

    if (pSrcWin && pSrcWin->backStorage)
    {
	/*
	 * Copy any areas from the source backing store. Modifies
	 * rgnExposed.
	 */
	(* pSrcWin->drawable.pScreen->ExposeCopy) ((WindowPtr)pSrcDrawable,
					      pDstDrawable,
					      pGC,
					      &rgnExposed,
					      srcx, srcy,
					      dstx, dsty,
					      plane);
    }
    
    /* move them over the destination */
    REGION_TRANSLATE(pscr, &rgnExposed, dstx-srcx, dsty-srcy);

    /* intersect with visible areas of dest */
    REGION_INTERSECT(pscr, &rgnExposed, &rgnExposed, prgnDstClip);

    /*
     * If we have LOTS of rectangles, we decide to take the extents
     * and force an exposure on that.  This should require much less
     * work overall, on both client and server.  This is cheating, but
     * isn't prohibited by the protocol ("spontaneous combustion" :-)
     * for windows.
     */
    extents = pGC->graphicsExposures &&
	      (REGION_NUM_RECTS(&rgnExposed) > RECTLIMIT) &&
	      (pDstDrawable->type != DRAWABLE_PIXMAP);
#ifdef SHAPE
    if (pSrcWin)
    {
	RegionPtr	region;
    	if (!(region = wClipShape (pSrcWin)))
    	    region = wBoundingShape (pSrcWin);
    	/*
     	 * If you try to CopyArea the extents of a shaped window, compacting the
     	 * exposed region will undo all our work!
     	 */
    	if (extents && pSrcWin && region &&
    	    (RECT_IN_REGION(pscr, region, &srcBox) != rgnIN))
	    	extents = FALSE;
    }
#endif
    if (extents)
    {
	WindowPtr pWin = (WindowPtr)pDstDrawable;

	expBox = *REGION_EXTENTS(pscr, &rgnExposed);
	REGION_RESET(pscr, &rgnExposed, &expBox);
	/* need to clear out new areas of backing store */
	if (pWin->backStorage)
	    (void) (* pWin->drawable.pScreen->ClearBackingStore)(
					 pWin,
					 expBox.x1,
					 expBox.y1,
					 expBox.x2 - expBox.x1,
					 expBox.y2 - expBox.y1,
					 FALSE);
    }
    if ((pDstDrawable->type != DRAWABLE_PIXMAP) &&
	(((WindowPtr)pDstDrawable)->backgroundState != None))
    {
	WindowPtr pWin = (WindowPtr)pDstDrawable;

	/* make the exposed area screen-relative */
	REGION_TRANSLATE(pscr, &rgnExposed, 
				 pDstDrawable->x, pDstDrawable->y);

	if (extents)
	{
	    /* PaintWindowBackground doesn't clip, so we have to */
	    REGION_INTERSECT(pscr, &rgnExposed, &rgnExposed, &pWin->clipList);
	}
	(*pWin->drawable.pScreen->PaintWindowBackground)(
			(WindowPtr)pDstDrawable, &rgnExposed, PW_BACKGROUND);

	if (extents)
	{
	    REGION_RESET(pscr, &rgnExposed, &expBox);
	}
	else
	    REGION_TRANSLATE(pscr, &rgnExposed,
				     -pDstDrawable->x, -pDstDrawable->y);
    }
    if (prgnDstClip == &rgnDstRec)
    {
	REGION_UNINIT(pscr, prgnDstClip);
    }
    else if (prgnDstClip != prgnSrcClip)
    {
	REGION_DESTROY(pscr, prgnDstClip);
    }

    if (prgnSrcClip == &rgnSrcRec)
    {
	REGION_UNINIT(pscr, prgnSrcClip);
    }
    else
    {
	REGION_DESTROY(pscr, prgnSrcClip);
    }

    if (pGC->graphicsExposures)
    {
	/* don't look */
	RegionPtr exposed = REGION_CREATE(pscr, NullBox, 0);
	*exposed = rgnExposed;
	return exposed;
    }
    else
    {
	REGION_UNINIT(pscr, &rgnExposed);
	return NULL;
    }
}

/* send GraphicsExpose events, or a NoExpose event, based on the region */

void
miSendGraphicsExpose (client, pRgn, drawable, major, minor)
    ClientPtr	client;
    RegionPtr	pRgn;
    XID		drawable;
    int	major;
    int	minor;
{
    if (pRgn && !REGION_NIL(pRgn))
    {
        xEvent *pEvent;
	register xEvent *pe;
	register BoxPtr pBox;
	register int i;
	int numRects;

	numRects = REGION_NUM_RECTS(pRgn);
	pBox = REGION_RECTS(pRgn);
	if(!(pEvent = (xEvent *)ALLOCATE_LOCAL(numRects * sizeof(xEvent))))
		return;
	pe = pEvent;

	for (i=1; i<=numRects; i++, pe++, pBox++)
	{
	    pe->u.u.type = GraphicsExpose;
	    pe->u.graphicsExposure.drawable = drawable;
	    pe->u.graphicsExposure.x = pBox->x1;
	    pe->u.graphicsExposure.y = pBox->y1;
	    pe->u.graphicsExposure.width = pBox->x2 - pBox->x1;
	    pe->u.graphicsExposure.height = pBox->y2 - pBox->y1;
	    pe->u.graphicsExposure.count = numRects - i;
	    pe->u.graphicsExposure.majorEvent = major;
	    pe->u.graphicsExposure.minorEvent = minor;
	}
	TryClientEvents(client, pEvent, numRects,
			    (Mask)0, NoEventMask, NullGrab);
	DEALLOCATE_LOCAL(pEvent);
    }
    else
    {
        xEvent event;
	event.u.u.type = NoExpose;
	event.u.noExposure.drawable = drawable;
	event.u.noExposure.majorEvent = major;
	event.u.noExposure.minorEvent = minor;
	TryClientEvents(client, &event, 1,
	    (Mask)0, NoEventMask, NullGrab);
    }
}

void
miSendExposures(pWin, pRgn, dx, dy)
    WindowPtr pWin;
    RegionPtr pRgn;
    register int dx, dy;
{
    register BoxPtr pBox;
    int numRects;
    register xEvent *pEvent, *pe;
    register int i;

    pBox = REGION_RECTS(pRgn);
    numRects = REGION_NUM_RECTS(pRgn);
    if(!(pEvent = (xEvent *) ALLOCATE_LOCAL(numRects * sizeof(xEvent))))
	return;

    for (i=numRects, pe = pEvent; --i >= 0; pe++, pBox++)
    {
	pe->u.u.type = Expose;
	pe->u.expose.window = pWin->drawable.id;
	pe->u.expose.x = pBox->x1 - dx;
	pe->u.expose.y = pBox->y1 - dy;
	pe->u.expose.width = pBox->x2 - pBox->x1;
	pe->u.expose.height = pBox->y2 - pBox->y1;
	pe->u.expose.count = i;
    }
    DeliverEvents(pWin, pEvent, numRects, NullWindow);
    DEALLOCATE_LOCAL(pEvent);
}

void 
miWindowExposures(pWin, prgn, other_exposed)
    WindowPtr pWin;
    register RegionPtr prgn, other_exposed;
{
    RegionPtr   exposures = prgn;
    if (pWin->backStorage && prgn)
	/*
	 * in some cases, backing store will cause a different
	 * region to be exposed than needs to be repainted
	 * (like when a window is mapped).  RestoreAreas is
	 * allowed to return a region other than prgn,
	 * in which case this routine will free the resultant
	 * region.  If exposures is null, then no events will
	 * be sent to the client; if prgn is empty
	 * no areas will be repainted.
	 */
	exposures = (*pWin->drawable.pScreen->RestoreAreas)(pWin, prgn);
    if ((prgn && !REGION_NIL(prgn)) || 
	(exposures && !REGION_NIL(exposures)) || other_exposed)
    {
	RegionRec   expRec;
	int	    clientInterested;

	/*
	 * Restore from backing-store FIRST.
	 */
	clientInterested = (pWin->eventMask|wOtherEventMasks(pWin)) & ExposureMask;
	if (other_exposed)
	{
	    if (exposures)
	    {
		REGION_UNION(pWin->drawable.pScreen, other_exposed,
						  exposures,
					          other_exposed);
		if (exposures != prgn)
		    REGION_DESTROY(pWin->drawable.pScreen, exposures);
	    }
	    exposures = other_exposed;
	}
	if (clientInterested && exposures && (REGION_NUM_RECTS(exposures) > RECTLIMIT))
	{
	    /*
	     * If we have LOTS of rectangles, we decide to take the extents
	     * and force an exposure on that.  This should require much less
	     * work overall, on both client and server.  This is cheating, but
	     * isn't prohibited by the protocol ("spontaneous combustion" :-).
	     */
	    BoxRec box;

	    box = *REGION_EXTENTS( pWin->drawable.pScreen, exposures);
	    if (exposures == prgn) {
		exposures = &expRec;
		REGION_INIT( pWin->drawable.pScreen, exposures, &box, 1);
		REGION_RESET( pWin->drawable.pScreen, prgn, &box);
	    } else {
		REGION_RESET( pWin->drawable.pScreen, exposures, &box);
		REGION_UNION( pWin->drawable.pScreen, prgn, prgn, exposures);
	    }
	    /* PaintWindowBackground doesn't clip, so we have to */
	    REGION_INTERSECT( pWin->drawable.pScreen, prgn, prgn, &pWin->clipList);
	    /* need to clear out new areas of backing store, too */
	    if (pWin->backStorage)
		(void) (* pWin->drawable.pScreen->ClearBackingStore)(
					     pWin,
					     box.x1 - pWin->drawable.x,
					     box.y1 - pWin->drawable.y,
					     box.x2 - box.x1,
					     box.y2 - box.y1,
					     FALSE);
	}
	if (prgn && !REGION_NIL(prgn))
	    (*pWin->drawable.pScreen->PaintWindowBackground)(pWin, prgn, PW_BACKGROUND);
	if (clientInterested && exposures && !REGION_NIL(exposures))
	    miSendExposures(pWin, exposures,
			    pWin->drawable.x, pWin->drawable.y);
	if (exposures == &expRec)
	{
	    REGION_UNINIT( pWin->drawable.pScreen, exposures);
	}
	else if (exposures && exposures != prgn && exposures != other_exposed)
	    REGION_DESTROY( pWin->drawable.pScreen, exposures);
	if (prgn)
	    REGION_EMPTY( pWin->drawable.pScreen, prgn);
    }
    else if (exposures && exposures != prgn)
	REGION_DESTROY( pWin->drawable.pScreen, exposures);
}


/*
    this code is highly unlikely.  it is not haile selassie.

    there is some hair here.  we can't just use the window's
clip region as it is, because if we are painting the border,
the border is not in the client area and so we will be excluded
when we validate the GC, and if we are painting a parent-relative
background, the area we want to paint is in some other window.
since we trust the code calling us to tell us to paint only areas
that are really ours, we will temporarily give the window a
clipList the size of the whole screen and an origin at (0,0).
this more or less assumes that ddX code will do translation
based on the window's absolute position, and that ValidateGC will
look at clipList, and that no other fields from the
window will be used.  it's not possible to just draw
in the root because it may be a different depth.

to get the tile to align correctly we set the GC's tile origin to
be the (x,y) of the window's upper left corner, after which we
get the right bits when drawing into the root.

because the clip_mask is being set to None, we may call DoChangeGC with
fPointer set true, thus we no longer need to install the background or
border tile in the resource table.
*/

static RESTYPE ResType = 0;
static int numGCs = 0;
static GCPtr	screenContext[MAXSCREENS];

/*ARGSUSED*/
static int
tossGC (value, id)
pointer value;
XID id;
{
    GCPtr pGC = (GCPtr)value;
    screenContext[pGC->pScreen->myNum] = (GCPtr)NULL;
    FreeGC (pGC, id);
    numGCs--;
    if (!numGCs)
	ResType = 0;

    return 0;
}


void
miPaintWindow(pWin, prgn, what)
register WindowPtr pWin;
RegionPtr prgn;
int what;
{
    int	status;

    Bool usingScratchGC = FALSE;
    WindowPtr pRoot;
	
#define FUNCTION	0
#define FOREGROUND	1
#define TILE		2
#define FILLSTYLE	3
#define ABSX		4
#define ABSY		5
#define CLIPMASK	6
#define SUBWINDOW	7
#define COUNT_BITS	8

    ChangeGCVal gcval[7];
    ChangeGCVal newValues [COUNT_BITS];

    BITS32 gcmask, index, mask;
    RegionRec prgnWin;
    DDXPointRec oldCorner;
    BoxRec box;
    WindowPtr	pBgWin;
    GCPtr pGC;
    register int i;
    register BoxPtr pbox;
    register ScreenPtr pScreen = pWin->drawable.pScreen;
    register xRectangle *prect;
    int numRects;

    gcmask = 0;

    if (what == PW_BACKGROUND)
    {
	switch (pWin->backgroundState) {
	case None:
	    return;
	case ParentRelative:
	    (*pWin->parent->drawable.pScreen->PaintWindowBackground)(pWin->parent, prgn, what);
	    return;
	case BackgroundPixel:
	    newValues[FOREGROUND].val = pWin->background.pixel;
	    newValues[FILLSTYLE].val  = FillSolid;
	    gcmask |= GCForeground | GCFillStyle;
	    break;
	case BackgroundPixmap:
	    newValues[TILE].ptr = (pointer)pWin->background.pixmap;
	    newValues[FILLSTYLE].val = FillTiled;
	    gcmask |= GCTile | GCFillStyle | GCTileStipXOrigin | GCTileStipYOrigin;
	    break;
	}
    }
    else
    {
	if (pWin->borderIsPixel)
	{
	    newValues[FOREGROUND].val = pWin->border.pixel;
	    newValues[FILLSTYLE].val  = FillSolid;
	    gcmask |= GCForeground | GCFillStyle;
	}
	else
	{
	    newValues[TILE].ptr = (pointer)pWin->border.pixmap;
	    newValues[FILLSTYLE].val = FillTiled;
	    gcmask |= GCTile | GCFillStyle | GCTileStipXOrigin | GCTileStipYOrigin;
	}
    }

    prect = (xRectangle *)ALLOCATE_LOCAL(REGION_NUM_RECTS(prgn) *
					 sizeof(xRectangle));
    if (!prect)
	return;

    newValues[FUNCTION].val = GXcopy;
    gcmask |= GCFunction | GCClipMask;

    i = pScreen->myNum;
    pRoot = WindowTable[i];

    pBgWin = pWin;
    if (what == PW_BORDER)
    {
	while (pBgWin->backgroundState == ParentRelative)
	    pBgWin = pBgWin->parent;
    }

    if ((pWin->drawable.depth != pRoot->drawable.depth) ||
	(pWin->drawable.bitsPerPixel != pRoot->drawable.bitsPerPixel))
    {
	usingScratchGC = TRUE;
	pGC = GetScratchGC(pWin->drawable.depth, pWin->drawable.pScreen);
	if (!pGC)
	{
	    DEALLOCATE_LOCAL(prect);
	    return;
	}
	/*
	 * mash the clip list so we can paint the border by
	 * mangling the window in place, pretending it
	 * spans the entire screen
	 */
	if (what == PW_BORDER)
	{
	    prgnWin = pWin->clipList;
	    oldCorner.x = pWin->drawable.x;
	    oldCorner.y = pWin->drawable.y;
	    pWin->drawable.x = pWin->drawable.y = 0;
	    box.x1 = 0;
	    box.y1 = 0;
	    box.x2 = pScreen->width;
	    box.y2 = pScreen->height;
	    REGION_INIT(pScreen, &pWin->clipList, &box, 1);
	    pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	    newValues[ABSX].val = pBgWin->drawable.x;
	    newValues[ABSY].val = pBgWin->drawable.y;
	}
	else
	{
	    newValues[ABSX].val = 0;
	    newValues[ABSY].val = 0;
	}
    } else {
	/*
	 * draw the background to the root window
	 */
	if (screenContext[i] == (GCPtr)NULL)
	{
	    if (!ResType && !(ResType = CreateNewResourceType(tossGC)))
		return;
	    screenContext[i] = CreateGC((DrawablePtr)pWin, (BITS32) 0,
					(XID *)NULL, &status);
	    if (!screenContext[i])
		return;
	    numGCs++;
	    if (!AddResource(FakeClientID(0), ResType,
			     (pointer)screenContext[i]))
	        return;
	}
	pGC = screenContext[i];
	newValues[SUBWINDOW].val = IncludeInferiors;
	newValues[ABSX].val = pBgWin->drawable.x;
	newValues[ABSY].val = pBgWin->drawable.y;
	gcmask |= GCSubwindowMode;
	pWin = pRoot;
    }
    
    if (pWin->backStorage)
	(*pWin->drawable.pScreen->DrawGuarantee) (pWin, pGC, GuaranteeVisBack);

    mask = gcmask;
    gcmask = 0;
    i = 0;
    while (mask) {
    	index = lowbit (mask);
	mask &= ~index;
	switch (index) {
	case GCFunction:
	    if (pGC->alu != newValues[FUNCTION].val) {
		gcmask |= index;
		gcval[i++].val = newValues[FUNCTION].val;
	    }
	    break;
	case GCTileStipXOrigin:
	    if ( pGC->patOrg.x != newValues[ABSX].val) {
		gcmask |= index;
		gcval[i++].val = newValues[ABSX].val;
	    }
	    break;
	case GCTileStipYOrigin:
	    if ( pGC->patOrg.y != newValues[ABSY].val) {
		gcmask |= index;
		gcval[i++].val = newValues[ABSY].val;
	    }
	    break;
	case GCClipMask:
	    if ( pGC->clientClipType != CT_NONE) {
		gcmask |= index;
		gcval[i++].val = CT_NONE;
	    }
	    break;
	case GCSubwindowMode:
	    if ( pGC->subWindowMode != newValues[SUBWINDOW].val) {
		gcmask |= index;
		gcval[i++].val = newValues[SUBWINDOW].val;
	    }
	    break;
	case GCTile:
	    if (pGC->tileIsPixel || pGC->tile.pixmap != newValues[TILE].ptr)
 	    {
		gcmask |= index;
		gcval[i++].ptr = newValues[TILE].ptr;
	    }
	    break;
	case GCFillStyle:
	    if ( pGC->fillStyle != newValues[FILLSTYLE].val) {
		gcmask |= index;
		gcval[i++].val = newValues[FILLSTYLE].val;
	    }
	    break;
	case GCForeground:
	    if ( pGC->fgPixel != newValues[FOREGROUND].val) {
		gcmask |= index;
		gcval[i++].val = newValues[FOREGROUND].val;
	    }
	    break;
	}
    }

    if (gcmask)
        dixChangeGC(NullClient, pGC, gcmask, NULL, gcval);

    if (pWin->drawable.serialNumber != pGC->serialNumber)
	ValidateGC((DrawablePtr)pWin, pGC);

    numRects = REGION_NUM_RECTS(prgn);
    pbox = REGION_RECTS(prgn);
    for (i= numRects; --i >= 0; pbox++, prect++)
    {
	prect->x = pbox->x1 - pWin->drawable.x;
	prect->y = pbox->y1 - pWin->drawable.y;
	prect->width = pbox->x2 - pbox->x1;
	prect->height = pbox->y2 - pbox->y1;
    }
    prect -= numRects;
    (*pGC->ops->PolyFillRect)((DrawablePtr)pWin, pGC, numRects, prect);
    DEALLOCATE_LOCAL(prect);

    if (pWin->backStorage)
	(*pWin->drawable.pScreen->DrawGuarantee) (pWin, pGC, GuaranteeNothing);

    if (usingScratchGC)
    {
	if (what == PW_BORDER)
	{
	    REGION_UNINIT(pScreen, &pWin->clipList);
	    pWin->clipList = prgnWin;
	    pWin->drawable.x = oldCorner.x;
	    pWin->drawable.y = oldCorner.y;
	    pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	}
	FreeScratchGC(pGC);
    }
}


/* MICLEARDRAWABLE -- sets the entire drawable to the background color of
 * the GC.  Useful when we have a scratch drawable and need to initialize 
 * it. */
miClearDrawable(pDraw, pGC)
    DrawablePtr	pDraw;
    GCPtr	pGC;
{
    XID fg = pGC->fgPixel;
    XID bg = pGC->bgPixel;
    xRectangle rect;

    rect.x = 0;
    rect.y = 0;
    rect.width = pDraw->width;
    rect.height = pDraw->height;
    DoChangeGC(pGC, GCForeground, &bg, 0);
    ValidateGC(pDraw, pGC);
    (*pGC->ops->PolyFillRect)(pDraw, pGC, 1, &rect);
    DoChangeGC(pGC, GCForeground, &fg, 0);
    ValidateGC(pDraw, pGC);
}
