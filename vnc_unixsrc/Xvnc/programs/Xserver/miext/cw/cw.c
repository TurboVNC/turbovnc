/*
 * Copyright © 2004 Eric Anholt
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Eric Anholt not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Eric Anholt makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * ERIC ANHOLT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ERIC ANHOLT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/* $Header: /cvs/xorg/xc/programs/Xserver/miext/cw/cw.c,v 1.23 2005/10/02 08:28:26 anholt Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "gcstruct.h"
#include "windowstr.h"
#include "cw.h"

#define CW_DEBUG 1

#if CW_DEBUG
#define CW_ASSERT(x) do {						\
    if (!(x)) {								\
	ErrorF("composite wrapper: assertion failed at %s:%d\n", __FUNC__, \
	    __LINE__);							\
    }									\
} while (0)
#else
#define CW_ASSERT(x) do {} while (0)
#endif

int cwGCIndex;
int cwScreenIndex;
int cwWindowIndex;
#ifdef RENDER
int cwPictureIndex;
#endif
static Bool cwDisabled[MAXSCREENS];
static unsigned long cwGeneration = 0;
extern GCOps cwGCOps;

static Bool
cwCloseScreen (int i, ScreenPtr pScreen);

static void
cwValidateGC(GCPtr pGC, unsigned long stateChanges, DrawablePtr pDrawable);
static void
cwChangeGC(GCPtr pGC, unsigned long mask);
static void
cwCopyGC(GCPtr pGCSrc, unsigned long mask, GCPtr pGCDst);
static void
cwDestroyGC(GCPtr pGC);
static void
cwChangeClip(GCPtr pGC, int type, pointer pvalue, int nrects);
static void
cwCopyClip(GCPtr pgcDst, GCPtr pgcSrc);
static void
cwDestroyClip(GCPtr pGC);

GCFuncs cwGCFuncs = {
    cwValidateGC,
    cwChangeGC,
    cwCopyGC,
    cwDestroyGC,
    cwChangeClip,
    cwDestroyClip,
    cwCopyClip,
};

/* Find the real drawable to draw to, and provide offsets that will translate
 * window coordinates to backing pixmap coordinates.
 */
DrawablePtr
cwGetBackingDrawable(DrawablePtr pDrawable, int *x_off, int *y_off)
{
    PixmapPtr	pPixmap;
    
    if (pDrawable->type == DRAWABLE_WINDOW && 
	(pPixmap = getCwPixmap ((WindowPtr) pDrawable)))
    {
	*x_off = pDrawable->x - pPixmap->screen_x;
	*y_off = pDrawable->y - pPixmap->screen_y;
	return &pPixmap->drawable;
    } else {
	*x_off = *y_off = 0;
	return pDrawable;
    }
}

#define FUNC_PROLOGUE(pGC, pPriv) do {					\
    (pGC)->funcs = (pPriv)->wrapFuncs;					\
    (pGC)->ops = (pPriv)->wrapOps;					\
} while (0)

#define FUNC_EPILOGUE(pGC, pPriv) do {					\
    (pPriv)->wrapFuncs = (pGC)->funcs;					\
    (pPriv)->wrapOps = (pGC)->ops;					\
    (pGC)->funcs = &cwGCFuncs;						\
    (pGC)->ops = &cwGCOps;						\
} while (0)


static Bool
cwCreateBackingGC(GCPtr pGC, DrawablePtr pDrawable)
{
    cwGCRec *pPriv = getCwGC(pGC);
    int status, x_off, y_off;
    XID noexpose = xFalse;
    DrawablePtr pBackingDrawable;

    pBackingDrawable = cwGetBackingDrawable(pDrawable, &x_off, &y_off);
    pPriv->pBackingGC = CreateGC(pBackingDrawable, GCGraphicsExposures,
				 &noexpose, &status);
    if (status != Success)
	return FALSE;

    pPriv->serialNumber = 0;
    pPriv->stateChanges = (1 << (GCLastBit + 1)) - 1;

    return TRUE;
}

static void
cwDestroyBackingGC(GCPtr pGC)
{
    cwGCPtr pPriv;

    pPriv = (cwGCPtr) getCwGC (pGC);

    if (pPriv->pBackingGC) {
	FreeGC(pPriv->pBackingGC, (XID)0);
	pPriv->pBackingGC = NULL;
    }
}

static void
cwValidateGC(GCPtr pGC, unsigned long stateChanges, DrawablePtr pDrawable)
{
    GCPtr   	  	pBackingGC;
    cwGCPtr		pPriv;
    DrawablePtr		pBackingDrawable;
    int			x_off, y_off;

    pPriv = (cwGCPtr) getCwGC (pGC);

    FUNC_PROLOGUE(pGC, pPriv);

    /*
     * Must call ValidateGC to ensure pGC->pCompositeClip is valid
     */
    (*pGC->funcs->ValidateGC)(pGC, stateChanges, pDrawable);

    if (!cwDrawableIsRedirWindow(pDrawable)) {
	cwDestroyBackingGC(pGC);
	FUNC_EPILOGUE(pGC, pPriv);
	return;
    } else {
	if (!pPriv->pBackingGC && !cwCreateBackingGC(pGC, pDrawable)) {
	    FUNC_EPILOGUE(pGC, pPriv);
	    return;
	}
    }

    pBackingGC = pPriv->pBackingGC;
    pBackingDrawable = cwGetBackingDrawable(pDrawable, &x_off, &y_off);

    pPriv->stateChanges |= stateChanges;

    /*
     * Copy the composite clip into the backing GC if either
     * the drawable clip list has changed or the client has changed
     * the client clip data
     */
    if (pDrawable->serialNumber != pPriv->serialNumber ||
	(pPriv->stateChanges & (GCClipXOrigin|GCClipYOrigin|GCClipMask)))
    {
	XID vals[2];
	RegionPtr   pCompositeClip;

	pCompositeClip = REGION_CREATE (pScreen, NULL, 0);
	REGION_COPY (pScreen, pCompositeClip, pGC->pCompositeClip);

	/* Either the drawable has changed, or the clip list in the drawable has
	 * changed.  Copy the new clip list over and set the new translated
	 * offset for it.
	 */
	
	(*pBackingGC->funcs->ChangeClip) (pBackingGC, CT_REGION,
					  (pointer) pCompositeClip, 0);
	
	vals[0] = x_off - pDrawable->x;
	vals[1] = y_off - pDrawable->y;
	dixChangeGC(NullClient, pBackingGC,
		    (GCClipXOrigin | GCClipYOrigin), vals, NULL);

	pPriv->serialNumber = pDrawable->serialNumber;
	/*
	 * Mask off any client clip changes to make sure
	 * the clip list set above remains in effect
	 */
	pPriv->stateChanges &= ~(GCClipXOrigin|GCClipYOrigin|GCClipMask);
    }

    if (pPriv->stateChanges) {
	CopyGC(pGC, pBackingGC, pPriv->stateChanges);
	pPriv->stateChanges = 0;
    }

    if ((pGC->patOrg.x + x_off) != pBackingGC->patOrg.x ||
	(pGC->patOrg.y + y_off) != pBackingGC->patOrg.y)
    {
	XID vals[2];
	vals[0] = pGC->patOrg.x + x_off;
	vals[1] = pGC->patOrg.y + y_off;
	dixChangeGC(NullClient, pBackingGC,
		    (GCTileStipXOrigin | GCTileStipYOrigin), vals, NULL);
    }

    ValidateGC(pBackingDrawable, pBackingGC);

    FUNC_EPILOGUE(pGC, pPriv);
}

static void
cwChangeGC(GCPtr pGC, unsigned long mask)
{
    cwGCPtr	pPriv = (cwGCPtr)(pGC)->devPrivates[cwGCIndex].ptr;

    FUNC_PROLOGUE(pGC, pPriv);

    (*pGC->funcs->ChangeGC) (pGC, mask);

    FUNC_EPILOGUE(pGC, pPriv);
}

static void
cwCopyGC(GCPtr pGCSrc, unsigned long mask, GCPtr pGCDst)
{
    cwGCPtr	pPriv = (cwGCPtr)(pGCDst)->devPrivates[cwGCIndex].ptr;

    FUNC_PROLOGUE(pGCDst, pPriv);

    (*pGCDst->funcs->CopyGC) (pGCSrc, mask, pGCDst);

    FUNC_EPILOGUE(pGCDst, pPriv);
}

static void
cwDestroyGC(GCPtr pGC)
{
    cwGCPtr	pPriv = (cwGCPtr)(pGC)->devPrivates[cwGCIndex].ptr;

    FUNC_PROLOGUE(pGC, pPriv);

    cwDestroyBackingGC(pGC);

    (*pGC->funcs->DestroyGC) (pGC);

    /* leave it unwrapped */
}

static void
cwChangeClip(GCPtr pGC, int type, pointer pvalue, int nrects)
{
    cwGCPtr	pPriv = (cwGCPtr)(pGC)->devPrivates[cwGCIndex].ptr;

    FUNC_PROLOGUE(pGC, pPriv);

    (*pGC->funcs->ChangeClip)(pGC, type, pvalue, nrects);

    FUNC_EPILOGUE(pGC, pPriv);
}

static void
cwCopyClip(GCPtr pgcDst, GCPtr pgcSrc)
{
    cwGCPtr	pPriv = (cwGCPtr)(pgcDst)->devPrivates[cwGCIndex].ptr;

    FUNC_PROLOGUE(pgcDst, pPriv);

    (*pgcDst->funcs->CopyClip)(pgcDst, pgcSrc);

    FUNC_EPILOGUE(pgcDst, pPriv);
}

static void
cwDestroyClip(GCPtr pGC)
{
    cwGCPtr	pPriv = (cwGCPtr)(pGC)->devPrivates[cwGCIndex].ptr;

    FUNC_PROLOGUE(pGC, pPriv);

    (*pGC->funcs->DestroyClip)(pGC);

    FUNC_EPILOGUE(pGC, pPriv);
}

/*
 * Screen wrappers.
 */

#define SCREEN_PROLOGUE(pScreen, field)				\
  ((pScreen)->field = getCwScreen(pScreen)->field)

#define SCREEN_EPILOGUE(pScreen, field, wrapper) do {		\
    getCwScreen(pScreen)->field = (pScreen)->field;		\
    (pScreen)->field = (wrapper);				\
} while (0)

static Bool
cwCreateGC(GCPtr pGC)
{
    cwGCPtr	pPriv = getCwGC(pGC);
    ScreenPtr	pScreen = pGC->pScreen;
    Bool	ret;

    bzero(pPriv, sizeof(cwGCRec));
    SCREEN_PROLOGUE(pScreen, CreateGC);

    if ( (ret = (*pScreen->CreateGC)(pGC)) )
	FUNC_EPILOGUE(pGC, pPriv);

    SCREEN_EPILOGUE(pScreen, CreateGC, cwCreateGC);

    return ret;
}

static void
cwGetImage(DrawablePtr pSrc, int x, int y, int w, int h, unsigned int format,
	   unsigned long planemask, char *pdstLine)
{
    ScreenPtr pScreen = pSrc->pScreen;
    DrawablePtr pBackingDrawable;
    int src_off_x, src_off_y;
    
    SCREEN_PROLOGUE(pScreen, GetImage);

    pBackingDrawable = cwGetBackingDrawable(pSrc, &src_off_x, &src_off_y);

    CW_OFFSET_XY_SRC(x, y);

    (*pScreen->GetImage)(pBackingDrawable, x, y, w, h, format, planemask,
			 pdstLine);

    SCREEN_EPILOGUE(pScreen, GetImage, cwGetImage);
}

static void
cwGetSpans(DrawablePtr pSrc, int wMax, DDXPointPtr ppt, int *pwidth,
	   int nspans, char *pdstStart)
{
    ScreenPtr pScreen = pSrc->pScreen;
    DrawablePtr pBackingDrawable;
    int i;
    int src_off_x, src_off_y;
    
    SCREEN_PROLOGUE(pScreen, GetSpans);

    pBackingDrawable = cwGetBackingDrawable(pSrc, &src_off_x, &src_off_y);

    for (i = 0; i < nspans; i++)
	CW_OFFSET_XY_SRC(ppt[i].x, ppt[i].y);

    (*pScreen->GetSpans)(pBackingDrawable, wMax, ppt, pwidth, nspans,
			 pdstStart);

    SCREEN_EPILOGUE(pScreen, GetSpans, cwGetSpans);
}

static void
cwFillRegionSolid(DrawablePtr pDrawable, RegionPtr pRegion, unsigned long pixel)
{
    ScreenPtr pScreen = pDrawable->pScreen;
    GCPtr     pGC;
    BoxPtr    pBox;
    int       nbox, i;
    ChangeGCVal v[3];

    pGC = GetScratchGC(pDrawable->depth, pScreen);
    v[0].val = GXcopy;
    v[1].val = pixel;
    v[2].val = FillSolid;
    dixChangeGC(NullClient, pGC, (GCFunction | GCForeground | GCFillStyle),
		NULL, v);
    ValidateGC(pDrawable, pGC);

    pBox = REGION_RECTS(pRegion);
    nbox = REGION_NUM_RECTS(pRegion);

    for (i = 0; i < nbox; i++, pBox++) {
	xRectangle rect;
	rect.x      = pBox->x1;
	rect.y      = pBox->y1;
	rect.width  = pBox->x2 - pBox->x1;
	rect.height = pBox->y2 - pBox->y1;
	(*pGC->ops->PolyFillRect)(pDrawable, pGC, 1, &rect);
    }

   FreeScratchGC(pGC);
}

static void
cwFillRegionTiled(DrawablePtr pDrawable, RegionPtr pRegion, PixmapPtr pTile,
		  int x_off, int y_off)
{
    ScreenPtr pScreen = pDrawable->pScreen;
    GCPtr     pGC;
    BoxPtr    pBox;
    int       nbox, i;
    ChangeGCVal v[5];

    pGC = GetScratchGC(pDrawable->depth, pScreen);
    v[0].val = GXcopy;
    v[1].val = FillTiled;
    v[2].ptr = (pointer) pTile;
    v[3].val = x_off;
    v[4].val = y_off;
    dixChangeGC(NullClient, pGC, (GCFunction | GCFillStyle | GCTile |
		GCTileStipXOrigin | GCTileStipYOrigin), NULL, v);

    ValidateGC(pDrawable, pGC);

    pBox = REGION_RECTS(pRegion);
    nbox = REGION_NUM_RECTS(pRegion);

    for (i = 0; i < nbox; i++, pBox++) {
	xRectangle rect;
	rect.x      = pBox->x1;
	rect.y      = pBox->y1;
	rect.width  = pBox->x2 - pBox->x1;
	rect.height = pBox->y2 - pBox->y1;
	(*pGC->ops->PolyFillRect)(pDrawable, pGC, 1, &rect);
    }

   FreeScratchGC(pGC);
}

static void
cwPaintWindowBackground(WindowPtr pWin, RegionPtr pRegion, int what)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;

    SCREEN_PROLOGUE(pScreen, PaintWindowBackground);

    if (!cwDrawableIsRedirWindow((DrawablePtr)pWin)) {
	(*pScreen->PaintWindowBackground)(pWin, pRegion, what);
    } else {
	DrawablePtr pBackingDrawable;
	int x_off, y_off, x_screen, y_screen;

	while (pWin && pWin->backgroundState == ParentRelative)
	    pWin = pWin->parent;

	pBackingDrawable = cwGetBackingDrawable((DrawablePtr)pWin, &x_off,
						&y_off);

	x_screen = x_off - pWin->drawable.x;
	y_screen = y_off - pWin->drawable.y;

	if (pWin && (pWin->backgroundState == BackgroundPixel ||
		pWin->backgroundState == BackgroundPixmap))
	{
	    REGION_TRANSLATE(pScreen, pRegion, x_screen, y_screen);

	    if (pWin->backgroundState == BackgroundPixel) {
		cwFillRegionSolid(pBackingDrawable, pRegion,
				  pWin->background.pixel);
	    } else {
		cwFillRegionTiled(pBackingDrawable, pRegion,
				  pWin->background.pixmap, x_off, y_off);
	    }

	    REGION_TRANSLATE(pScreen, pRegion, -x_screen, -y_screen);
	}
    }

    SCREEN_EPILOGUE(pScreen, PaintWindowBackground, cwPaintWindowBackground);
}

static void
cwPaintWindowBorder(WindowPtr pWin, RegionPtr pRegion, int what)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;

    SCREEN_PROLOGUE(pScreen, PaintWindowBorder);

    if (!cwDrawableIsRedirWindow((DrawablePtr)pWin)) {
	(*pScreen->PaintWindowBorder)(pWin, pRegion,  what);
    } else {
	DrawablePtr pBackingDrawable;
	int x_off, y_off, x_screen, y_screen;

	pBackingDrawable = cwGetBackingDrawable((DrawablePtr)pWin, &x_off,
						&y_off);

	x_screen = x_off - pWin->drawable.x;
	y_screen = y_off - pWin->drawable.y;

	REGION_TRANSLATE(pScreen, pRegion, x_screen, y_screen);

	if (pWin->borderIsPixel) {
	    cwFillRegionSolid(pBackingDrawable, pRegion, pWin->border.pixel);
	} else {
	    cwFillRegionTiled(pBackingDrawable, pRegion, pWin->border.pixmap,
			      x_off, y_off);
	}

	REGION_TRANSLATE(pScreen, pRegion, -x_screen, -y_screen);
    }

    SCREEN_EPILOGUE(pScreen, PaintWindowBorder, cwPaintWindowBorder);
}

static void
cwCopyWindow(WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;

    SCREEN_PROLOGUE(pScreen, CopyWindow);

    if (!cwDrawableIsRedirWindow((DrawablePtr)pWin)) {
	(*pScreen->CopyWindow)(pWin, ptOldOrg, prgnSrc);
    } else {
	GCPtr	    pGC;
	BoxPtr	    pExtents;
	int	    x_off, y_off;
	int	    dx, dy;
	PixmapPtr   pBackingPixmap;
	RegionPtr   pClip;
	int	    src_x, src_y, dst_x, dst_y, w, h;

	dx = ptOldOrg.x - pWin->drawable.x;
	dy = ptOldOrg.y - pWin->drawable.y;

	pExtents = REGION_EXTENTS(pScreen, prgnSrc);

	pBackingPixmap = (PixmapPtr) cwGetBackingDrawable((DrawablePtr)pWin,
							  &x_off, &y_off);

	src_x = pExtents->x1 - pBackingPixmap->screen_x;
	src_y = pExtents->y1 - pBackingPixmap->screen_y;
	w = pExtents->x2 - pExtents->x1;
	h = pExtents->y2 - pExtents->y1;
	dst_x = src_x - dx;
	dst_y = src_y - dy;
			       
	/* Translate region (as required by API) */
	REGION_TRANSLATE(pScreen, prgnSrc, -dx, -dy);
	
	pGC = GetScratchGC(pBackingPixmap->drawable.depth, pScreen);
	/*
	 * Copy region to GC as clip, aligning as dest clip
	 */
	pClip = REGION_CREATE (pScreen, NULL, 0);
	REGION_INTERSECT(pScreen, pClip, &pWin->borderClip, prgnSrc);
	REGION_TRANSLATE(pScreen, pClip, 
			 -pBackingPixmap->screen_x,
			 -pBackingPixmap->screen_y);
	
	(*pGC->funcs->ChangeClip) (pGC, CT_REGION, pClip, 0);

	ValidateGC(&pBackingPixmap->drawable, pGC);

	(*pGC->ops->CopyArea) (&pBackingPixmap->drawable,
			       &pBackingPixmap->drawable, pGC,
			       src_x, src_y, w, h, dst_x, dst_y);

	(*pGC->funcs->DestroyClip) (pGC);

	FreeScratchGC(pGC);
    }
	
    SCREEN_EPILOGUE(pScreen, CopyWindow, cwCopyWindow);
}

static PixmapPtr
cwGetWindowPixmap (WindowPtr pWin)
{
    PixmapPtr	pPixmap = getCwPixmap (pWin);

    if (!pPixmap)
    {
	ScreenPtr   pScreen = pWin->drawable.pScreen;
	SCREEN_PROLOGUE(pScreen, GetWindowPixmap);
	if (pScreen->GetWindowPixmap)
	    pPixmap = (*pScreen->GetWindowPixmap) (pWin);
	SCREEN_EPILOGUE(pScreen, GetWindowPixmap, cwGetWindowPixmap);
    }
    return pPixmap;
}

static void
cwSetWindowPixmap (WindowPtr pWindow, PixmapPtr pPixmap)
{
    ScreenPtr	pScreen = pWindow->drawable.pScreen;
    
    if (pPixmap == (*pScreen->GetScreenPixmap) (pScreen))
	pPixmap = NULL;
    setCwPixmap (pWindow, pPixmap);
}

/* Screen initialization/teardown */
void
miInitializeCompositeWrapper(ScreenPtr pScreen)
{
    cwScreenPtr pScreenPriv;

    if (cwDisabled[pScreen->myNum])
	return;

    if (cwGeneration != serverGeneration)
    {
	cwScreenIndex = AllocateScreenPrivateIndex();
	if (cwScreenIndex < 0)
	    return;
	cwGCIndex = AllocateGCPrivateIndex();
	cwWindowIndex = AllocateWindowPrivateIndex();
#ifdef RENDER
	cwPictureIndex = AllocatePicturePrivateIndex();
#endif
	cwGeneration = serverGeneration;
    }
    if (!AllocateGCPrivate(pScreen, cwGCIndex, sizeof(cwGCRec)))
	return;
    if (!AllocateWindowPrivate(pScreen, cwWindowIndex, 0))
	return;
#ifdef RENDER
    if (!AllocatePicturePrivate(pScreen, cwPictureIndex, 0))
	return;
#endif
    pScreenPriv = (cwScreenPtr)xalloc(sizeof(cwScreenRec));
    if (!pScreenPriv)
	return;

    pScreen->devPrivates[cwScreenIndex].ptr = (pointer)pScreenPriv;
    
    SCREEN_EPILOGUE(pScreen, CloseScreen, cwCloseScreen);
    SCREEN_EPILOGUE(pScreen, GetImage, cwGetImage);
    SCREEN_EPILOGUE(pScreen, GetSpans, cwGetSpans);
    SCREEN_EPILOGUE(pScreen, CreateGC, cwCreateGC);
    SCREEN_EPILOGUE(pScreen, PaintWindowBackground, cwPaintWindowBackground);
    SCREEN_EPILOGUE(pScreen, PaintWindowBorder, cwPaintWindowBorder);
    SCREEN_EPILOGUE(pScreen, CopyWindow, cwCopyWindow);

    SCREEN_EPILOGUE(pScreen, SetWindowPixmap, cwSetWindowPixmap);
    SCREEN_EPILOGUE(pScreen, GetWindowPixmap, cwGetWindowPixmap);

#ifdef RENDER
    if (GetPictureScreen (pScreen))
	cwInitializeRender(pScreen);
#endif
}

void
miDisableCompositeWrapper(ScreenPtr pScreen)
{
    cwDisabled[pScreen->myNum] = TRUE;
}

static Bool
cwCloseScreen (int i, ScreenPtr pScreen)
{
    cwScreenPtr   pScreenPriv;
#ifdef RENDER
    PictureScreenPtr ps = GetPictureScreenIfSet(pScreen);
#endif

    pScreenPriv = (cwScreenPtr)pScreen->devPrivates[cwScreenIndex].ptr;

    pScreen->CloseScreen = pScreenPriv->CloseScreen;
    pScreen->GetImage = pScreenPriv->GetImage;
    pScreen->GetSpans = pScreenPriv->GetSpans;
    pScreen->CreateGC = pScreenPriv->CreateGC;
    pScreen->PaintWindowBackground = pScreenPriv->PaintWindowBackground;
    pScreen->PaintWindowBorder = pScreenPriv->PaintWindowBorder;
    pScreen->CopyWindow = pScreenPriv->CopyWindow;

#ifdef RENDER
    if (ps)
	cwFiniRender(pScreen);
#endif

    xfree((pointer)pScreenPriv);

    return (*pScreen->CloseScreen)(i, pScreen);
}
