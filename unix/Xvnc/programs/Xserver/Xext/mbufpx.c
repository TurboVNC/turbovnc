/************************************************************

Copyright (c) 1989  X Consortium

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

********************************************************/

/* $XConsortium: mbufpx.c,v 1.5 94/04/17 20:32:54 dpw Exp $ */
/* $XFree86: xc/programs/Xserver/Xext/mbufpx.c,v 3.0 1994/05/08 05:17:32 dawes Exp $ */
#define NEED_REPLIES
#define NEED_EVENTS
#include <stdio.h>
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "os.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "extnsionst.h"
#include "dixstruct.h"
#include "resource.h"
#include "opaque.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "inputstr.h"
#ifndef MINIX
#include <sys/time.h>
#endif

#define _MULTIBUF_SERVER_	/* don't want Xlib structures */
#define _MULTIBUF_PIXMAP_
#include "multibufst.h"


static Bool NoopDDA_True() { return TRUE; }

static Bool pixPositionWindow();
static int  pixCreateImageBuffers();
static void pixDisplayImageBuffers();
static void pixClearImageBufferArea();
static void pixDeleteBufferDrawable();
static void pixWrapScreenFuncs();
static void pixResetProc();

Bool
pixMultibufferInit(pScreen, pMBScreen)
    ScreenPtr pScreen;
    mbufScreenPtr pMBScreen;
{
    int			i, j, k;
    xMbufBufferInfo	*pInfo;
    int			nInfo;
    DepthPtr		pDepth;
    mbufPixmapPrivPtr	pMBPriv;

    pMBScreen->CreateImageBuffers = pixCreateImageBuffers;
    pMBScreen->DestroyImageBuffers = (void (*)())NoopDDA;
    pMBScreen->DisplayImageBuffers = pixDisplayImageBuffers;
    pMBScreen->ClearImageBufferArea = pixClearImageBufferArea;
    pMBScreen->ChangeMBufferAttributes = NoopDDA_True;
    pMBScreen->ChangeBufferAttributes = NoopDDA_True;
    pMBScreen->DeleteBufferDrawable = pixDeleteBufferDrawable;
    pMBScreen->WrapScreenFuncs = pixWrapScreenFuncs;
    pMBScreen->ResetProc = pixResetProc;

    /* Support every depth and visual combination that the screen does */

    nInfo = 0;
    for (i = 0; i < pScreen->numDepths; i++)
    {
	pDepth = &pScreen->allowedDepths[i];
	nInfo += pDepth->numVids;
    }

    pInfo = (xMbufBufferInfo *) xalloc (nInfo * sizeof (xMbufBufferInfo));
    if (!pInfo)
	return FALSE;

    k = 0;
    for (i = 0; i < pScreen->numDepths; i++)
    {
	pDepth = &pScreen->allowedDepths[i];
	for (j = 0; j < pDepth->numVids; j++)
	{
	    pInfo[k].visualID = pDepth->vids[j];
	    pInfo[k].maxBuffers = 0;
	    pInfo[k].depth = pDepth->depth;
	    k++;
	}
    }

    pMBScreen->nInfo = nInfo;
    pMBScreen->pInfo = pInfo;

    /*
     * Setup the devPrivate to mbufScreenRec
     */

    pMBPriv = (mbufPixmapPrivPtr) xalloc(sizeof(* pMBPriv));
    if (!pMBPriv)
    {
	xfree(pInfo);
	return (FALSE);
    }
    pMBScreen->devPrivate.ptr = (pointer) pMBPriv;
    pMBPriv->PositionWindow = NULL;
    pMBPriv->funcsWrapped = 0;

    return TRUE;
}

/*ARGSUSED*/
static int
pixCreateImageBuffers (pWin, nbuf, ids, action, hint)
    WindowPtr	pWin;
    int		nbuf;
    XID		*ids;
    int		action;
    int		hint;
{
    mbufWindowPtr	pMBWindow;
    mbufBufferPtr	pMBBuffer;
    ScreenPtr		pScreen;
    int			width, height, depth;
    int			i;

    pMBWindow = MB_WINDOW_PRIV(pWin);

    width = pWin->drawable.width;
    height = pWin->drawable.height;
    depth = pWin->drawable.depth;
    pScreen = pWin->drawable.pScreen;

    for (i = 0; i < nbuf; i++)
    {
	pMBBuffer = &pMBWindow->buffers[i];
	pMBBuffer->pDrawable = (DrawablePtr)
	    (*pScreen->CreatePixmap) (pScreen, width, height, depth);
	if (!pMBBuffer->pDrawable)
	    break;

	if (!AddResource (ids[i], MultibufferDrawableResType,
			  (pointer) pMBBuffer->pDrawable))
	{
	    (*pScreen->DestroyPixmap) ((PixmapPtr) pMBBuffer->pDrawable);
	    break;
	}
	pMBBuffer->pDrawable->id = ids[i];

	/*
	 * In the description of the CreateImageBuffers request:
         * "If the window is mapped, or if these image buffers have
         *  backing store, their contents will be tiled with the window
         *  background, and zero or more expose events will be generated
         *  for each of these buffers."
	 */

	(* MB_SCREEN_PRIV(pScreen)->ClearImageBufferArea)
	    (pMBBuffer, 0,0, 0,0, TRUE);
    }

    return i;
}

/*
 * set up the gc to clear the pixmaps;
 */
static Bool
SetupBackgroundPainter (pWin, pGC)
    WindowPtr	pWin;
    GCPtr	pGC;
{
    XID		    gcvalues[4];
    int		    ts_x_origin, ts_y_origin;
    PixUnion	    background;
    int		    backgroundState;
    Mask	    gcmask;

    /*
     * First take care of any ParentRelative stuff by altering the
     * tile/stipple origin to match the coordinates of the upper-left
     * corner of the first ancestor without a ParentRelative background.
     * This coordinate is, of course, negative.
     */

    ts_x_origin = ts_y_origin = 0;
    while (pWin->backgroundState == ParentRelative) {
	ts_x_origin -= pWin->origin.x;
	ts_y_origin -= pWin->origin.y;
	pWin = pWin->parent;
    }
    backgroundState = pWin->backgroundState;
    background = pWin->background;

    switch (backgroundState)
    {
    case BackgroundPixel:
	gcvalues[0] = (XID) background.pixel;
	gcvalues[1] = FillSolid;
	gcmask = GCForeground|GCFillStyle;
	break;

    case BackgroundPixmap:
	gcvalues[0] = FillTiled;
	gcvalues[1] = (XID) background.pixmap;
	gcvalues[2] = ts_x_origin;
	gcvalues[3] = ts_y_origin;
	gcmask = GCFillStyle|GCTile|GCTileStipXOrigin|GCTileStipYOrigin;
	break;

    default:
	return FALSE;
    }
    DoChangeGC(pGC, gcmask, gcvalues, TRUE);
    return TRUE;
}

static void
MultibufferPaintBackgroundRectangles(pWin, pDrawable, nrects, pRects)
    WindowPtr pWin;
    DrawablePtr pDrawable;
    int nrects;
    xRectangle *pRects;
{
    GCPtr      pGC;

    pGC = GetScratchGC (pWin->drawable.depth, pWin->drawable.pScreen);
    if (SetupBackgroundPainter(pWin, pGC))
    {
	ValidateGC(pDrawable, pGC);
	(*pGC->ops->PolyFillRect) (pDrawable, pGC, nrects, pRects);
    }
    FreeScratchGC(pGC);
}

static void
MultibufferPaintBackgroundRegion(pWin, pDrawable, pRegion)
    WindowPtr pWin;
    DrawablePtr pDrawable;
    RegionPtr pRegion;
{
    xRectangle *pRects;
    int nrects  = REGION_NUM_RECTS(pRegion);
    BoxPtr pbox = REGION_RECTS(pRegion);

    pRects = (xRectangle *)ALLOCATE_LOCAL(nrects * sizeof(xRectangle));
    if (pRects)
    {
	int i;
	for (i = 0; i < nrects; i++)
	{
	    pRects[i].x = pbox->x1;
	    pRects[i].y = pbox->y1;
	    pRects[i].width  = pbox->x2 - pbox->x1;
	    pRects[i].height = pbox->y2 - pbox->y1;
	}
	MultibufferPaintBackgroundRectangles(pWin, pDrawable, nrects, pRects);
	DEALLOCATE_LOCAL(pRects);
    }
}

static void
pixDisplayImageBuffers(pScreen, ppMBWindow, ppMBBuffer, nbuf)
    mbufBufferPtr	    *ppMBBuffer;
    mbufWindowPtr	    *ppMBWindow;
    int		    nbuf;
{
    GCPtr	    pGC = NULL;
    PixmapPtr	    pPrevPixmap, pNewPixmap;
    WindowPtr	    pWin;
    RegionPtr	    pExposed;
    int		    i;
    mbufBufferPtr  pPrevMBBuffer;
    XID		    bool;
    xRectangle      r;

    UpdateCurrentTime ();
    for (i = 0; i < nbuf; i++)
    {
	pWin = ppMBWindow[i]->pWindow;

	/* Time to get a different scratch GC? */

	if (!pGC
	    || pGC->depth   != pWin->drawable.depth
	    || pGC->pScreen != pWin->drawable.pScreen)
	{
	    if (pGC) FreeScratchGC(pGC);
	    pGC = GetScratchGC (pWin->drawable.depth, pWin->drawable.pScreen);
	}
	pPrevMBBuffer = MB_DISPLAYED_BUFFER(ppMBWindow[i]);
	pPrevPixmap = (PixmapPtr) pPrevMBBuffer->pDrawable;
	pNewPixmap = (PixmapPtr) ppMBBuffer[i]->pDrawable;

	if (pPrevPixmap == pNewPixmap)
	{
	  /* "If a specified buffer is already displayed, any delays and
	   *  update action will still be performed for that buffer."
	   *
	   *  We special-case this because applications do occasionally
	   *  request a redundant DisplayImageBuffers, and we can save
	   *  strokes by recognizing that the only update action that will
	   *  change the buffer contents in this case is Background.
	   */
	    if (ppMBWindow[i]->updateAction == MultibufferUpdateActionBackground)
	    {
		r.x = r.y = 0;
		r.width  = pWin->drawable.width;
		r.height = pWin->drawable.height;
		MultibufferPaintBackgroundRectangles(pWin, (DrawablePtr)pWin,
						     1, &r);
	    }
	}
	else /* different buffer is being displayed */
	{
	    /* perform update action */

	    switch (ppMBWindow[i]->updateAction)
	    {
	    case MultibufferUpdateActionUndefined:
		break;

	    case MultibufferUpdateActionBackground:

		r.x = r.y = 0;
		r.width  = pPrevPixmap->drawable.width;
		r.height = pPrevPixmap->drawable.height;
		MultibufferPaintBackgroundRectangles(pWin,
						     (DrawablePtr)pPrevPixmap,
						     1, &r);
		break;

	    case MultibufferUpdateActionUntouched:
		
		/* copy the window to the pixmap that represents the
		 * currently displayed buffer
		 */

		if (pPrevMBBuffer->eventMask & ExposureMask)
		{
		    bool = TRUE;
		    DoChangeGC (pGC, GCGraphicsExposures, &bool, FALSE);
		}
		ValidateGC ((DrawablePtr)pPrevPixmap, pGC);
		pExposed = (*pGC->ops->CopyArea)((DrawablePtr) pWin,
						 (DrawablePtr) pPrevPixmap,
						 pGC,
						 0, 0,
						 pWin->drawable.width,
						 pWin->drawable.height,
						 0, 0);

		/* if we couldn't copy the whole window to the buffer,
		 * send expose events (if any client wants them)
		 */

		if (pPrevMBBuffer->eventMask & ExposureMask)
		{ /* some client wants expose events */
		    if (pExposed)
		    {
			RegionPtr	 pWinSize;
			extern RegionPtr CreateUnclippedWinSize();
			ScreenPtr pScreen = pWin->drawable.pScreen;
			pWinSize = CreateUnclippedWinSize (pWin);
			/*
			 * pExposed is window-relative, but at this point
			 * pWinSize is screen-relative.  Make pWinSize be
			 * window-relative so that region ops involving
			 * pExposed and pWinSize behave sensibly.
			 */
			REGION_TRANSLATE(pScreen, pWinSize,
						     -pWin->drawable.x,
						     -pWin->drawable.y);
			REGION_INTERSECT(pScreen, pExposed, pExposed, pWinSize);
			REGION_DESTROY(pScreen, pWinSize);
			MultibufferExpose (pPrevMBBuffer, pExposed);
			REGION_DESTROY(pScreen, pExposed);
		    }
		    bool = FALSE;
		    DoChangeGC (pGC, GCGraphicsExposures, &bool, FALSE);
		} /* end some client wants expose events */

		break; /* end case MultibufferUpdateActionUntouched */

	    case MultibufferUpdateActionCopied:

		ValidateGC ((DrawablePtr)pPrevPixmap, pGC);
		(*pGC->ops->CopyArea) ((DrawablePtr)pNewPixmap,
				       (DrawablePtr)pPrevPixmap, pGC,
				       0, 0, pWin->drawable.width,
				       pWin->drawable.height, 0, 0);
		break;

	    } /* end switch on update action */

	    /* display the new buffer */

	    ValidateGC ((DrawablePtr)pWin, pGC);
	    (*pGC->ops->CopyArea) ((DrawablePtr)pNewPixmap, (DrawablePtr)pWin,
				   pGC, 0, 0,
				   pWin->drawable.width, pWin->drawable.height,
				   0, 0);
	}

	ppMBWindow[i]->lastUpdate = currentTime;
    }

    if (pGC) FreeScratchGC (pGC);
    return;
}

/*
 * resize the buffers when the window is resized
 */ 

static Bool
pixPositionWindow (pWin, x, y)
    WindowPtr	pWin;
    int		x, y;
{
    ScreenPtr	    pScreen;
    mbufPixmapPrivPtr pMBPriv;
    mbufWindowPtr   pMBWindow;
    mbufBufferPtr   pMBBuffer;
    int		    width, height;
    int		    i;
    int		    dx, dy, dw, dh;
    int		    sourcex, sourcey;
    int		    destx, desty;
    PixmapPtr	    pPixmap;
    GCPtr	    pGC;
    int		    savewidth, saveheight;
    Bool	    clear;
    RegionRec       exposedRegion;
    Bool	    ret;

    pScreen = pWin->drawable.pScreen;
    pMBPriv = MB_SCREEN_PRIV_PIXMAP(pScreen);

    UNWRAP_SCREEN_FUNC(pScreen, pMBPriv, Bool, PositionWindow);
    ret = (* pScreen->PositionWindow) (pWin, x, y);
    REWRAP_SCREEN_FUNC(pScreen, pMBPriv, Bool, PositionWindow);

    if (!(pMBWindow = MB_WINDOW_PRIV(pWin)))
	return ret;

    /* if new size is same as old, we're done */

    if (pMBWindow->width == pWin->drawable.width &&
        pMBWindow->height == pWin->drawable.height)
	return ret;

    width = pWin->drawable.width;
    height = pWin->drawable.height;
    dx = pWin->drawable.x - pMBWindow->x;
    dy = pWin->drawable.x - pMBWindow->y;
    dw = width - pMBWindow->width;
    dh = height - pMBWindow->height;
    GravityTranslate (0, 0, -dx, -dy, dw, dh,
		      pWin->bitGravity, &destx, &desty);

    /* if the window grew, remember to paint the window background,
     * and maybe send expose events, for the new areas of the buffers
     */

    clear = pMBWindow->width < width || pMBWindow->height < height ||
	    pWin->bitGravity == ForgetGravity;

    sourcex = 0;
    sourcey = 0;
    savewidth = pMBWindow->width;
    saveheight = pMBWindow->height;
    /* clip rectangle to source and destination */
    if (destx < 0)
    {
	savewidth += destx;
	sourcex -= destx;
	destx = 0;
    }
    if (destx + savewidth > width)
	savewidth = width - destx;
    if (desty < 0)
    {
	saveheight += desty;
	sourcey -= desty;
	desty = 0;
    }
    if (desty + saveheight > height)
	saveheight = height - desty;

    pMBWindow->width = width;
    pMBWindow->height = height;
    pMBWindow->x = pWin->drawable.x;
    pMBWindow->y = pWin->drawable.y;

    if (clear)
    {
	BoxRec box;

	box.x1 = box.y1 = 0;
	box.x2 = width;
	box.y2 = height;
	REGION_INIT(pScreen, &exposedRegion, &box, 1);
	if (pWin->bitGravity != ForgetGravity)
	{
	    RegionRec preservedRegion;
	    box.x1 = destx;
	    box.y1 = desty;
	    box.x2 = destx + savewidth;
	    box.y2 = desty + saveheight;
	    REGION_INIT(pScreen, &preservedRegion, &box, 1);
	    REGION_SUBTRACT(pScreen, &exposedRegion, &exposedRegion, &preservedRegion);
	    REGION_UNINIT(pScreen, &preservedRegion);
	}

    } /* end if (clear) */

    pGC = GetScratchGC (pWin->drawable.depth, pScreen);

    /* create buffers with new window size */

    for (i = 0; i < pMBWindow->numMultibuffer; i++)
    {
	pMBBuffer = &pMBWindow->buffers[i];
	pPixmap = (*pScreen->CreatePixmap) (pScreen, width, height, pWin->drawable.depth);
	if (!pPixmap)
	{
	    (* MB_SCREEN_PRIV(pScreen)->DestroyImageBuffers)(pWin);
	    break;
	}
	if (clear)
	{
	    MultibufferPaintBackgroundRegion(pWin, (DrawablePtr)pPixmap, &exposedRegion);
	    MultibufferExpose(pMBBuffer, &exposedRegion);
	}
	if (pWin->bitGravity != ForgetGravity)
	{
	    ValidateGC ((DrawablePtr)pPixmap, pGC);
	    (*pGC->ops->CopyArea) (pMBBuffer->pDrawable, (DrawablePtr)pPixmap,
				   pGC,
				   sourcex, sourcey, savewidth, saveheight,
				   destx, desty);
	}
	pPixmap->drawable.id = pMBBuffer->pDrawable->id;
	(*pScreen->DestroyPixmap) ((PixmapPtr) pMBBuffer->pDrawable);
	pMBBuffer->pDrawable = (DrawablePtr) pPixmap;
	if (i != pMBWindow->displayedMultibuffer)
	{
	    ChangeResourceValue (pPixmap->drawable.id,
				 MultibufferDrawableResType,
				 (pointer) pPixmap);
	}
    }
    FreeScratchGC (pGC);
    if (clear)
	REGION_UNINIT(pScreen, &exposedRegion);
    return TRUE;
}

static void
pixWrapScreenFuncs(pScreen)
    ScreenPtr pScreen;
{
    mbufPixmapPrivPtr pMBPriv = MB_SCREEN_PRIV_PIXMAP(pScreen);
    WRAP_SCREEN_FUNC(pScreen, pMBPriv, PositionWindow, pixPositionWindow);
}

static void
pixResetProc(pScreen)
    ScreenPtr pScreen;
{
    mbufScreenPtr pMBScreen = MB_SCREEN_PRIV(pScreen);
    mbufPixmapPrivPtr pMBPriv = MB_SCREEN_PRIV_PIXMAP(pScreen);

    xfree(pMBScreen->pInfo);
    xfree(pMBPriv);
}

static void
pixClearImageBufferArea(pMBBuffer, x,y, width,height, exposures)
    mbufBufferPtr	pMBBuffer;
    short		x, y;
    unsigned short	width, height;
    Bool		exposures;
{
    WindowPtr pWin;
    ScreenPtr pScreen;
    BoxRec box;
    RegionRec region;
    int w_width, w_height;
    DrawablePtr pDrawable;

    pWin = pMBBuffer->pMBWindow->pWindow;
    pScreen = pWin->drawable.pScreen;

    w_width  = pWin->drawable.width;
    w_height = pWin->drawable.height;

    box.x1 = x;
    box.y1 = y;
    box.x2 = width  ? (box.x1 + width)  : w_width;
    box.y2 = height ? (box.y1 + height) : w_height;

    if (box.x1 < 0)        box.x1 = 0;
    if (box.y1 < 0)        box.y1 = 0;
    if (box.x2 > w_width)  box.x2 = w_width;
    if (box.y2 > w_height) box.y2 = w_height;

    REGION_INIT(pScreen, &region, &box, 1);

    if (pMBBuffer->number == pMBBuffer->pMBWindow->displayedMultibuffer)
      pDrawable = (DrawablePtr) pWin;
    else
      pDrawable = pMBBuffer->pDrawable;

    MultibufferPaintBackgroundRegion(pWin, pDrawable, &region);

    if (exposures)
	MultibufferExpose(pMBBuffer, &region);

    REGION_UNINIT(pScreen, &region);
}

static void
pixDeleteBufferDrawable(pDrawable)
    DrawablePtr	pDrawable;
{
    (* pDrawable->pScreen->DestroyPixmap)((PixmapPtr) pDrawable);
}
