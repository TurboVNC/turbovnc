/* $Xorg: mibstore.c,v 1.4 2001/02/09 02:05:20 xorgcvs Exp $ */
/***********************************************************

Copyright 1987, 1998  The Open Group

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


Copyright 1987 by the Regents of the University of California

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name The Open Group not be used in advertising or publicity
pertaining to distribution of the software without specific, written prior
permission.  

The University of California makes no representations about the suitability
of this software for any purpose.  It is provided "as is" without express or
implied warranty.

******************************************************************/

/* $XFree86: xc/programs/Xserver/mi/mibstore.c,v 1.10tsi Exp $ */

#define NEED_EVENTS
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include <X11/fonts/fontstruct.h>
#include "dixfontstr.h"
#include "dixstruct.h"		/* For requestingClient */
#include "mi.h"
#include "mibstorest.h"

/*
 * When the server fails to allocate a backing store pixmap, if you want
 * it to dynamically retry to allocate backing store on every subsequent
 * graphics op, you can enable BSEAGER; otherwise, backing store will be
 * disabled on the window until it is unmapped and then remapped.
 */
/* #define BSEAGER */

/*-
 * NOTES ON USAGE:
 *
 * The functions in this file implement a machine-independent backing-store
 * scheme. To use it, the output library must do the following:
 *	- Provide a SaveAreas function that takes a destination pixmap, a
 *	    region of the areas to save (in the pixmap's coordinate system)
 *	    and the screen origin of the region. It should copy the areas from
 *	    the screen into the pixmap.
 *	- Provide a RestoreAreas function that takes a source pixmap, a region
 *	    of the areas to restore (in the screen's coordinate system) and the
 *	    origin of the pixmap on the screen. It should copy the areas from
 *	    the pixmap into the screen.
 *	- Provide a SetClipmaskRgn function that takes a gc and a region
 *	    and merges the region into any CT_PIXMAP client clip that
 *	    is specified in the GC.  This routine is only needed if
 *	    miValidateBackingStore will see CT_PIXMAP clip lists; not
 *	    true for any of the sample servers (which convert the PIXMAP
 *	    clip lists into CT_REGION clip lists; an expensive but simple
 *	    to code option).
 *	- The function placed in a window's ClearToBackground vector must call
 *	    pScreen->ClearBackingStore with the window, followed by
 *	    the window-relative x and y coordinates, followed by the width and
 *	    height of the area to be cleared, followed by the generateExposures
 *	    flag. This has been taken care of in miClearToBackground.
 *	- Whatever determines GraphicsExpose events for the CopyArea and
 *	    CopyPlane requests should call pWin->backStorage->ExposeCopy
 *	    with the source and destination drawables, the GC used, a source-
 *	    window-relative region of exposed areas, the source and destination
 *	    coordinates and the bitplane copied, if CopyPlane, or 0, if
 *	    CopyArea.
 *
 * JUSTIFICATION
 *    This is a cross between saving everything and just saving the
 * obscued areas (as in Pike's layers.)  This method has the advantage
 * of only doing each output operation once per pixel, visible or
 * invisible, and avoids having to do all the crufty storage
 * management of keeping several separate rectangles.  Since the
 * ddx layer ouput primitives are required to draw through clipping
 * rectangles anyway, sending multiple drawing requests for each of
 * several rectangles isn't necessary.  (Of course, it could be argued
 * that the ddx routines should just take one rectangle each and
 * get called multiple times, but that would make taking advantage of
 * smart hardware harder, and probably be slower as well.)
 */

#define SETUP_BACKING_TERSE(pGC) \
    miBSGCPtr	pGCPrivate = (miBSGCPtr)(pGC)->devPrivates[miBSGCIndex].ptr; \
    GCFuncs	*oldFuncs = pGC->funcs;

#define SETUP_BACKING(pDrawable,pGC) \
    miBSWindowPtr pBackingStore = \
    	(miBSWindowPtr)((WindowPtr)(pDrawable))->backStorage; \
    DrawablePtr	  pBackingDrawable = (DrawablePtr) \
        pBackingStore->pBackingPixmap; \
    SETUP_BACKING_TERSE(pGC) \
    GCPtr	pBackingGC = pGCPrivate->pBackingGC;

#define PROLOGUE(pGC) { \
    pGC->ops = pGCPrivate->wrapOps;\
    pGC->funcs = pGCPrivate->wrapFuncs; \
    }

#define EPILOGUE(pGC) { \
    pGCPrivate->wrapOps = (pGC)->ops; \
    (pGC)->ops = &miBSGCOps; \
    (pGC)->funcs = oldFuncs; \
    }
   
static void	    miCreateBSPixmap(WindowPtr pWin, BoxPtr pExtents);
static void	    miDestroyBSPixmap(WindowPtr pWin);
static void	    miTileVirtualBS(WindowPtr pWin);
static void	    miBSAllocate(WindowPtr pWin), miBSFree(WindowPtr pWin);
static Bool	    miBSCreateGCPrivate(GCPtr pGC);
static void	    miBSClearBackingRegion(WindowPtr pWin, RegionPtr pRgn);

#define MoreCopy0 ;
#define MoreCopy2 *dstCopy++ = *srcCopy++; *dstCopy++ = *srcCopy++;
#define MoreCopy4 MoreCopy2 MoreCopy2

#define copyData(src,dst,n,morecopy) \
{ \
    register short *srcCopy = (short *)(src); \
    register short *dstCopy = (short *)(dst); \
    register int i; \
    register int bsx = pBackingStore->x; \
    register int bsy = pBackingStore->y; \
    for (i = n; --i >= 0; ) \
    { \
	*dstCopy++ = *srcCopy++ - bsx; \
	*dstCopy++ = *srcCopy++ - bsy; \
	morecopy \
    } \
}

#define copyPoints(src,dst,n,mode) \
if (mode == CoordModeOrigin) \
{ \
    copyData(src,dst,n,MoreCopy0); \
} \
else \
{ \
    memmove((char *)(dst), (char *)(src), (n) << 2); \
    *((short *)(dst)) -= pBackingStore->x; \
    *((short *)(dst) + 1) -= pBackingStore->y; \
}

/*
 * wrappers for screen funcs
 */

static int  miBSScreenIndex;
static unsigned long miBSGeneration = 0;

static Bool	    miBSCloseScreen(int i, ScreenPtr pScreen);
static void	    miBSGetImage(DrawablePtr pDrawable, int sx, int sy,
				 int w, int h, unsigned int format,
				 unsigned long planemask, char *pdstLine);
static void	    miBSGetSpans(DrawablePtr pDrawable, int wMax,
				 DDXPointPtr ppt, int *pwidth, int nspans,
				 char *pdstStart);
static Bool	    miBSChangeWindowAttributes(WindowPtr pWin,
					       unsigned long mask);
static Bool	    miBSCreateGC(GCPtr pGC);
static Bool	    miBSDestroyWindow(WindowPtr pWin);

/*
 * backing store screen functions
 */

static void	    miBSSaveDoomedAreas(WindowPtr pWin, RegionPtr pObscured,
					int dx, int dy);
static RegionPtr    miBSRestoreAreas(WindowPtr pWin, RegionPtr prgnExposed);
static void	    miBSExposeCopy(WindowPtr pSrc, DrawablePtr pDst,
				   GCPtr pGC, RegionPtr prgnExposed,
				   int srcx, int srcy, int dstx, int dsty,
				   unsigned long plane);
static RegionPtr    miBSTranslateBackingStore(WindowPtr pWin, int windx,
					      int windy, RegionPtr oldClip,
					      int oldx, int oldy);
static RegionPtr    miBSClearBackingStore(WindowPtr pWin, int x, int y,
					  int w, int h, Bool generateExposures);
static void	    miBSDrawGuarantee(WindowPtr pWin, GCPtr pGC,
				      int guarantee);

/*
 * wrapper vectors for GC funcs and ops
 */

static int  miBSGCIndex;

static void miBSValidateGC(GCPtr pGC, unsigned long stateChanges,
			   DrawablePtr pDrawable);
static void miBSCopyGC(GCPtr pGCSrc, unsigned long mask, GCPtr pGCDst);
static void miBSDestroyGC(GCPtr pGC);
static void miBSChangeGC(GCPtr pGC, unsigned long mask);
static void miBSChangeClip(GCPtr pGC, int type, pointer pvalue, int nrects);
static void miBSDestroyClip(GCPtr pGC);
static void miBSCopyClip(GCPtr pgcDst, GCPtr pgcSrc);

static GCFuncs	miBSGCFuncs = {
    miBSValidateGC,
    miBSChangeGC,
    miBSCopyGC,
    miBSDestroyGC,
    miBSChangeClip,
    miBSDestroyClip,
    miBSCopyClip,
};

static void	    miBSFillSpans(DrawablePtr pDrawable, GCPtr pGC, int nInit,
				  DDXPointPtr pptInit, int *pwidthInit,
				  int fSorted);
static void	    miBSSetSpans(DrawablePtr pDrawable, GCPtr pGC, char *psrc,
				 DDXPointPtr ppt, int *pwidth, int nspans,
				 int fSorted);
static void	    miBSPutImage(DrawablePtr pDrawable, GCPtr pGC, int depth,
				 int x, int y, int w, int h, int leftPad,
				 int format, char *pBits);
static RegionPtr    miBSCopyArea(DrawablePtr pSrc, DrawablePtr pDst,
				 GCPtr pGC, int srcx, int srcy, int w, int h,
				 int dstx, int dsty);
static RegionPtr    miBSCopyPlane(DrawablePtr pSrc, DrawablePtr pDst,
				  GCPtr pGC, int srcx, int srcy, int w, int h,
				  int dstx, int dsty, unsigned long plane);
static void	    miBSPolyPoint(DrawablePtr pDrawable, GCPtr pGC, int mode,
				  int npt, xPoint *pptInit);
static void	    miBSPolylines(DrawablePtr pDrawable, GCPtr pGC, int mode,
				  int npt, DDXPointPtr pptInit);
static void	    miBSPolySegment(DrawablePtr pDrawable, GCPtr pGC, int nseg,
				    xSegment *pSegs);
static void	    miBSPolyRectangle(DrawablePtr pDrawable, GCPtr pGC,
				      int nrects, xRectangle *pRects);
static void	    miBSPolyArc(DrawablePtr pDrawable, GCPtr pGC, int narcs,
				xArc *parcs);
static void	    miBSFillPolygon(DrawablePtr pDrawable, GCPtr pGC,
				    int shape, int mode, int count,
				    DDXPointPtr pPts);
static void	    miBSPolyFillRect(DrawablePtr pDrawable, GCPtr pGC,
				     int nrectFill, xRectangle *prectInit);
static void	    miBSPolyFillArc(DrawablePtr pDrawable, GCPtr pGC,
				    int narcs, xArc *parcs);
static int	    miBSPolyText8(DrawablePtr pDrawable, GCPtr pGC,
				  int x, int y, int count, char *chars);
static int	    miBSPolyText16(DrawablePtr pDrawable, GCPtr pGC,
				   int x, int y, int count,
				   unsigned short *chars);
static void	    miBSImageText8(DrawablePtr pDrawable, GCPtr pGC,
				   int x, int y, int count, char *chars);
static void	    miBSImageText16(DrawablePtr pDrawable, GCPtr pGC,
				    int x, int y, int count,
				    unsigned short *chars);
static void	    miBSImageGlyphBlt(DrawablePtr pDrawable, GCPtr pGC,
				      int x, int y, unsigned int nglyph,
				      CharInfoPtr *ppci, pointer pglyphBase);
static void	    miBSPolyGlyphBlt(DrawablePtr pDrawable, GCPtr pGC,
				     int x, int y, unsigned int nglyph,
				     CharInfoPtr *ppci, pointer pglyphBase);
static void	    miBSPushPixels(GCPtr pGC, PixmapPtr pBitMap,
				   DrawablePtr pDst, int w, int h,
				   int x, int y);
#ifdef NEED_LINEHELPER
static void	    miBSLineHelper(void);
#endif

static GCOps miBSGCOps = {
    miBSFillSpans,	miBSSetSpans,	    miBSPutImage,	
    miBSCopyArea,	miBSCopyPlane,	    miBSPolyPoint,
    miBSPolylines,	miBSPolySegment,    miBSPolyRectangle,
    miBSPolyArc,	miBSFillPolygon,    miBSPolyFillRect,
    miBSPolyFillArc,	miBSPolyText8,	    miBSPolyText16,
    miBSImageText8,	miBSImageText16,    miBSImageGlyphBlt,
    miBSPolyGlyphBlt,	miBSPushPixels
#ifdef NEED_LINEHELPER
    , miBSLineHelper
#endif
};

#define FUNC_PROLOGUE(pGC, pPriv) \
    ((pGC)->funcs = pPriv->wrapFuncs),\
    ((pGC)->ops = pPriv->wrapOps)

#define FUNC_EPILOGUE(pGC, pPriv) \
    ((pGC)->funcs = &miBSGCFuncs),\
    ((pGC)->ops = &miBSGCOps)

/*
 * every GC in the server is initially wrapped with these
 * "cheap" functions.  This allocates no memory and is used
 * to discover GCs used with windows which have backing
 * store enabled
 */

static void miBSCheapValidateGC(GCPtr pGC, unsigned long stateChanges,
				DrawablePtr pDrawable);
static void miBSCheapCopyGC(GCPtr pGCSrc, unsigned long mask, GCPtr pGCDst);
static void miBSCheapDestroyGC(GCPtr pGC);
static void miBSCheapChangeGC(GCPtr pGC, unsigned long mask);
static void miBSCheapChangeClip(GCPtr pGC, int type, pointer pvalue,
				int nrects);
static void miBSCheapDestroyClip(GCPtr pGC);
static void miBSCheapCopyClip(GCPtr pgcDst, GCPtr pgcSrc);

static GCFuncs miBSCheapGCFuncs = {
    miBSCheapValidateGC,
    miBSCheapChangeGC,
    miBSCheapCopyGC,
    miBSCheapDestroyGC,
    miBSCheapChangeClip,
    miBSCheapDestroyClip,
    miBSCheapCopyClip,
};

#define CHEAP_FUNC_PROLOGUE(pGC) \
    ((pGC)->funcs = (GCFuncs *) (pGC)->devPrivates[miBSGCIndex].ptr)

#define CHEAP_FUNC_EPILOGUE(pGC) \
    ((pGC)->funcs = &miBSCheapGCFuncs)

/*
 * called from device screen initialization proc.  Gets a GCPrivateIndex
 * and wraps appropriate per-screen functions.  pScreen->BackingStoreFuncs
 * must be previously initialized.
 */

void
miInitializeBackingStore (pScreen)
    ScreenPtr	pScreen;
{
    miBSScreenPtr    pScreenPriv;

    if (miBSGeneration != serverGeneration)
    {
	miBSScreenIndex = AllocateScreenPrivateIndex ();
	if (miBSScreenIndex < 0)
	    return;
	miBSGCIndex = AllocateGCPrivateIndex ();
	miBSGeneration = serverGeneration;
    }
    if (!AllocateGCPrivate(pScreen, miBSGCIndex, 0))
	return;
    pScreenPriv = (miBSScreenPtr) xalloc (sizeof (miBSScreenRec));
    if (!pScreenPriv)
	return;

    pScreenPriv->CloseScreen = pScreen->CloseScreen;
    pScreenPriv->GetImage = pScreen->GetImage;
    pScreenPriv->GetSpans = pScreen->GetSpans;
    pScreenPriv->ChangeWindowAttributes = pScreen->ChangeWindowAttributes;
    pScreenPriv->CreateGC = pScreen->CreateGC;
    pScreenPriv->DestroyWindow = pScreen->DestroyWindow;

    pScreen->CloseScreen = miBSCloseScreen;
    pScreen->GetImage = miBSGetImage;
    pScreen->GetSpans = miBSGetSpans;
    pScreen->ChangeWindowAttributes = miBSChangeWindowAttributes;
    pScreen->CreateGC = miBSCreateGC;
    pScreen->DestroyWindow = miBSDestroyWindow;

    pScreen->SaveDoomedAreas = miBSSaveDoomedAreas;
    pScreen->RestoreAreas = miBSRestoreAreas;
    pScreen->ExposeCopy = miBSExposeCopy;
    pScreen->TranslateBackingStore = miBSTranslateBackingStore;
    pScreen->ClearBackingStore = miBSClearBackingStore;
    pScreen->DrawGuarantee = miBSDrawGuarantee;

    pScreen->devPrivates[miBSScreenIndex].ptr = (pointer) pScreenPriv;
}

/*
 * Screen function wrappers
 */

#define SCREEN_PROLOGUE(pScreen, field)\
  ((pScreen)->field = \
   ((miBSScreenPtr) \
    (pScreen)->devPrivates[miBSScreenIndex].ptr)->field)

#define SCREEN_EPILOGUE(pScreen, field, wrapper)\
    ((pScreen)->field = wrapper)

/*
 * CloseScreen wrapper -- unwrap everything, free the private data
 * and call the wrapped function
 */

static Bool
miBSCloseScreen (i, pScreen)
    int		i;
    ScreenPtr	pScreen;
{
    miBSScreenPtr   pScreenPriv;

    pScreenPriv = (miBSScreenPtr) pScreen->devPrivates[miBSScreenIndex].ptr;

    pScreen->CloseScreen = pScreenPriv->CloseScreen;
    pScreen->GetImage = pScreenPriv->GetImage;
    pScreen->GetSpans = pScreenPriv->GetSpans;
    pScreen->ChangeWindowAttributes = pScreenPriv->ChangeWindowAttributes;
    pScreen->CreateGC = pScreenPriv->CreateGC;

    xfree ((pointer) pScreenPriv);

    return (*pScreen->CloseScreen) (i, pScreen);
}

static void miBSFillVirtualBits(DrawablePtr pDrawable, GCPtr pGC,
				RegionPtr pRgn, int x, int y, int state,
				PixUnion pixunion, unsigned long planemask);

static void
miBSGetImage (pDrawable, sx, sy, w, h, format, planemask, pdstLine)
    DrawablePtr	    pDrawable;
    int		    sx, sy, w, h;
    unsigned int    format;
    unsigned long   planemask;
    char	    *pdstLine;
{
    ScreenPtr		    pScreen = pDrawable->pScreen;
    BoxRec		    bounds;
    unsigned char	    depth;
    
    SCREEN_PROLOGUE (pScreen, GetImage);

    if (pDrawable->type != DRAWABLE_PIXMAP &&
	((WindowPtr) pDrawable)->visibility != VisibilityUnobscured)
    {
	PixmapPtr	pPixmap;
	miBSWindowPtr	pWindowPriv;
	GCPtr		pGC = NULL;
	WindowPtr	pWin, pSrcWin;
	int		xoff, yoff;
	RegionRec	Remaining;
	RegionRec	Border;
	RegionRec	Inside;
	BoxPtr		pBox;
	int		n;

	pWin = (WindowPtr) pDrawable;
	pPixmap = 0;
	depth = pDrawable->depth;
	bounds.x1 = sx + pDrawable->x;
	bounds.y1 = sy + pDrawable->y;
	bounds.x2 = bounds.x1 + w;
	bounds.y2 = bounds.y1 + h;
	REGION_INIT(pScreen, &Remaining, &bounds, 0);
	for (;;)
 	{
	    bounds.x1 = sx + pDrawable->x - pWin->drawable.x;
	    bounds.y1 = sy + pDrawable->y - pWin->drawable.y;
	    bounds.x2 = bounds.x1 + w;
	    bounds.y2 = bounds.y1 + h;
	    if (pWin->viewable && pWin->backStorage &&
		pWin->drawable.depth == depth &&
	        (RECT_IN_REGION(pScreen, &(pWindowPriv =
		    (miBSWindowPtr) pWin->backStorage)->SavedRegion,
		    &bounds) != rgnOUT ||
		 RECT_IN_REGION(pScreen, &Remaining,
		  REGION_EXTENTS(pScreen, &pWin->borderSize)) != rgnOUT))
	    {
		if (!pPixmap)
		{
		    XID	subWindowMode = IncludeInferiors;
		    int	x, y;

		    pPixmap = (*pScreen->CreatePixmap) (pScreen, w, h, depth);
		    if (!pPixmap)
			goto punt;
		    pGC = GetScratchGC (depth, pScreen);
		    if (!pGC)
		    {
			(*pScreen->DestroyPixmap) (pPixmap);
			goto punt;
		    }
		    ChangeGC (pGC, GCSubwindowMode, &subWindowMode);
		    ValidateGC ((DrawablePtr)pPixmap, pGC);
		    REGION_NULL(pScreen, &Border);
		    REGION_NULL(pScreen, &Inside);
		    pSrcWin = (WindowPtr) pDrawable;
		    x = sx;
		    y = sy;
		    if (pSrcWin->parent)
		    {
			x += pSrcWin->origin.x;
			y += pSrcWin->origin.y;
			pSrcWin = pSrcWin->parent;
		    }
		    (*pGC->ops->CopyArea) ((DrawablePtr)pSrcWin,
 					    (DrawablePtr)pPixmap, pGC,
					    x, y, w, h,
					    0, 0);
		    REGION_SUBTRACT(pScreen, &Remaining, &Remaining,
				    &((WindowPtr) pDrawable)->borderClip);
		}

		REGION_INTERSECT(pScreen, &Inside, &Remaining, &pWin->winSize);
		REGION_TRANSLATE(pScreen, &Inside,
					     -pWin->drawable.x,
 					     -pWin->drawable.y);
		REGION_INTERSECT(pScreen, &Inside, &Inside,
				 &pWindowPriv->SavedRegion);

		/* offset of sub-window in GetImage pixmap */
		xoff = pWin->drawable.x - pDrawable->x - sx;
		yoff = pWin->drawable.y - pDrawable->y - sy;

		if (REGION_NUM_RECTS(&Inside) > 0)
		{
		    switch (pWindowPriv->status)
		    {
		    case StatusContents:
			pBox = REGION_RECTS(&Inside);
			for (n = REGION_NUM_RECTS(&Inside); --n >= 0;)
			{
			    (*pGC->ops->CopyArea) (
				(DrawablePtr)pWindowPriv->pBackingPixmap,
						   (DrawablePtr)pPixmap, pGC,
						   pBox->x1 - pWindowPriv->x,
						   pBox->y1 - pWindowPriv->y,
						   pBox->x2 - pBox->x1,
						   pBox->y2 - pBox->y1,
						   pBox->x1 + xoff,
						   pBox->y1 + yoff);
			    ++pBox;
			}
			break;
		    case StatusVirtual:
		    case StatusVDirty:
			if (pWindowPriv->backgroundState == BackgroundPixmap ||
			    pWindowPriv->backgroundState == BackgroundPixel)
			miBSFillVirtualBits ((DrawablePtr) pPixmap, pGC, &Inside,
					    xoff, yoff,
					    (int) pWindowPriv->backgroundState,
					    pWindowPriv->background, ~0L);
			break;
		    }
		}
		REGION_SUBTRACT(pScreen, &Border, &pWin->borderSize,
				&pWin->winSize);
		REGION_INTERSECT(pScreen, &Border, &Border, &Remaining);
		if (REGION_NUM_RECTS(&Border) > 0)
		{
		    REGION_TRANSLATE(pScreen, &Border, -pWin->drawable.x,
						  -pWin->drawable.y);
		    miBSFillVirtualBits ((DrawablePtr) pPixmap, pGC, &Border,
				    	xoff, yoff,
				    	pWin->borderIsPixel ? (int)BackgroundPixel : (int)BackgroundPixmap,
				    	pWin->border, ~0L);
		}
	    }

	    if (pWin->viewable && pWin->firstChild)
		pWin = pWin->firstChild;
	    else
	    {
		while (!pWin->nextSib && pWin != (WindowPtr) pDrawable)
		    pWin = pWin->parent;
		if (pWin == (WindowPtr) pDrawable)
		    break;
		pWin = pWin->nextSib;
	    }
	}

	REGION_UNINIT(pScreen, &Remaining);

	if (pPixmap)
	{
	    REGION_UNINIT(pScreen, &Border);
	    REGION_UNINIT(pScreen, &Inside);
	    (*pScreen->GetImage) ((DrawablePtr) pPixmap,
		0, 0, w, h, format, planemask, pdstLine);
	    (*pScreen->DestroyPixmap) (pPixmap);
	    FreeScratchGC (pGC);
	}
	else
	{
	    goto punt;
	}
    }
    else
    {
punt:	;
	(*pScreen->GetImage) (pDrawable, sx, sy, w, h,
			      format, planemask, pdstLine);
    }

    SCREEN_EPILOGUE (pScreen, GetImage, miBSGetImage);
}

static void
miBSGetSpans (pDrawable, wMax, ppt, pwidth, nspans, pdstStart)
    DrawablePtr	pDrawable;
    int		wMax;
    DDXPointPtr	ppt;
    int		*pwidth;
    int		nspans;
    char	*pdstStart;
{
    ScreenPtr		    pScreen = pDrawable->pScreen;
    BoxRec		    bounds;
    int			    i;
    WindowPtr		    pWin;
    int			    dx, dy;
    
    SCREEN_PROLOGUE (pScreen, GetSpans);

    if (pDrawable->type != DRAWABLE_PIXMAP && ((WindowPtr) pDrawable)->backStorage)
    {
	PixmapPtr	pPixmap;
	miBSWindowPtr	pWindowPriv;
	GCPtr		pGC;

	pWin = (WindowPtr) pDrawable;
	pWindowPriv = (miBSWindowPtr) pWin->backStorage;
	pPixmap = pWindowPriv->pBackingPixmap;

    	bounds.x1 = ppt->x;
    	bounds.y1 = ppt->y;
    	bounds.x2 = bounds.x1 + *pwidth;
    	bounds.y2 = ppt->y;
    	for (i = 0; i < nspans; i++)
    	{
	    if (ppt[i].x < bounds.x1)
	    	bounds.x1 = ppt[i].x;
	    if (ppt[i].x + pwidth[i] > bounds.x2)
	    	bounds.x2 = ppt[i].x + pwidth[i];
	    if (ppt[i].y < bounds.y1)
	    	bounds.y1 = ppt[i].y;
	    else if (ppt[i].y > bounds.y2)
	    	bounds.y2 = ppt[i].y;
    	}
    
    	switch (RECT_IN_REGION(pScreen, &pWindowPriv->SavedRegion, &bounds))
 	{
	case rgnPART:
	    if (!pPixmap)
	    {
		miCreateBSPixmap (pWin, NullBox);
		if (!(pPixmap = pWindowPriv->pBackingPixmap))
		    break;
	    }
	    pWindowPriv->status = StatusNoPixmap;
	    pGC = GetScratchGC(pPixmap->drawable.depth,
			       pPixmap->drawable.pScreen);
	    if (pGC)
	    {
		ValidateGC ((DrawablePtr) pPixmap, pGC);
		(*pGC->ops->CopyArea)
		    (pDrawable, (DrawablePtr) pPixmap, pGC,
		    bounds.x1, bounds.y1,
		    bounds.x2 - bounds.x1, bounds.y2 - bounds.y1,
		    bounds.x1 + pPixmap->drawable.x - pWin->drawable.x -
		     pWindowPriv->x,
		    bounds.y1 + pPixmap->drawable.y - pWin->drawable.y -
		     pWindowPriv->y);
		FreeScratchGC(pGC);
	    }
	    pWindowPriv->status = StatusContents;
	    /* fall through */
	case rgnIN:
	    if (!pPixmap)
	    {
		miCreateBSPixmap (pWin, NullBox);
		if (!(pPixmap = pWindowPriv->pBackingPixmap))
		    break;
	    }
	    dx = pPixmap->drawable.x - pWin->drawable.x - pWindowPriv->x;
	    dy = pPixmap->drawable.y - pWin->drawable.y - pWindowPriv->y;
	    for (i = 0; i < nspans; i++)
	    {
		ppt[i].x += dx;
		ppt[i].y += dy;
	    }
	    (*pScreen->GetSpans) ((DrawablePtr) pPixmap, wMax, ppt, pwidth,
				  nspans, pdstStart);
	    break;
	case rgnOUT:
	    (*pScreen->GetSpans) (pDrawable, wMax, ppt, pwidth, nspans,
				  pdstStart);
	    break;
	}
    }
    else
    {
	(*pScreen->GetSpans) (pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
    }

    SCREEN_EPILOGUE (pScreen, GetSpans, miBSGetSpans);
}

static Bool
miBSChangeWindowAttributes (pWin, mask)
    WindowPtr	    pWin;
    unsigned long   mask;
{
    ScreenPtr	pScreen;
    Bool	ret;

    pScreen = pWin->drawable.pScreen;

    SCREEN_PROLOGUE (pScreen, ChangeWindowAttributes);

    ret = (*pScreen->ChangeWindowAttributes) (pWin, mask);

    if (ret && (mask & CWBackingStore))
    {
	if (pWin->backingStore != NotUseful || pWin->DIXsaveUnder)
	    miBSAllocate (pWin);
	else
	    miBSFree (pWin);
    }

    SCREEN_EPILOGUE (pScreen, ChangeWindowAttributes, miBSChangeWindowAttributes);

    return ret;
}

/*
 * GC Create wrapper.  Set up the cheap GC func wrappers to track
 * GC validation on BackingStore windows
 */

static Bool
miBSCreateGC (pGC)
    GCPtr   pGC;
{
    ScreenPtr	pScreen = pGC->pScreen;
    Bool	ret;

    SCREEN_PROLOGUE (pScreen, CreateGC);
    
    if ( (ret = (*pScreen->CreateGC) (pGC)) )
    {
    	pGC->devPrivates[miBSGCIndex].ptr = (pointer) pGC->funcs;
    	pGC->funcs = &miBSCheapGCFuncs;
    }

    SCREEN_EPILOGUE (pScreen, CreateGC, miBSCreateGC);

    return ret;
}

static Bool
miBSDestroyWindow (pWin)
    WindowPtr	pWin;
{
    ScreenPtr	pScreen = pWin->drawable.pScreen;
    Bool	ret;

    SCREEN_PROLOGUE (pScreen, DestroyWindow);
    
    ret = (*pScreen->DestroyWindow) (pWin);

    miBSFree (pWin);

    SCREEN_EPILOGUE (pScreen, DestroyWindow, miBSDestroyWindow);

    return ret;
}

/*
 * cheap GC func wrappers.  Simply track validation on windows
 * with backing store to enable the real func/op wrappers
 */

static void
miBSCheapValidateGC (pGC, stateChanges, pDrawable)
    GCPtr	    pGC;
    unsigned long   stateChanges;
    DrawablePtr	    pDrawable;
{
    CHEAP_FUNC_PROLOGUE (pGC);
    
    if (pDrawable->type != DRAWABLE_PIXMAP &&
        ((WindowPtr) pDrawable)->backStorage != NULL &&
	miBSCreateGCPrivate (pGC))
    {
	(*pGC->funcs->ValidateGC) (pGC, stateChanges, pDrawable);
    }
    else
    {
	(*pGC->funcs->ValidateGC) (pGC, stateChanges, pDrawable);

	/* rewrap funcs as Validate may have changed them */
	pGC->devPrivates[miBSGCIndex].ptr = (pointer) pGC->funcs;

	CHEAP_FUNC_EPILOGUE (pGC);
    }
}

static void
miBSCheapChangeGC (pGC, mask)
    GCPtr   pGC;
    unsigned long   mask;
{
    CHEAP_FUNC_PROLOGUE (pGC);

    (*pGC->funcs->ChangeGC) (pGC, mask);

    CHEAP_FUNC_EPILOGUE (pGC);
}

static void
miBSCheapCopyGC (pGCSrc, mask, pGCDst)
    GCPtr   pGCSrc, pGCDst;
    unsigned long   mask;
{
    CHEAP_FUNC_PROLOGUE (pGCDst);

    (*pGCDst->funcs->CopyGC) (pGCSrc, mask, pGCDst);

    CHEAP_FUNC_EPILOGUE (pGCDst);
}

static void
miBSCheapDestroyGC (pGC)
    GCPtr   pGC;
{
    CHEAP_FUNC_PROLOGUE (pGC);

    (*pGC->funcs->DestroyGC) (pGC);

    /* leave it unwrapped */
}

static void
miBSCheapChangeClip (pGC, type, pvalue, nrects)
    GCPtr   pGC;
    int		type;
    pointer	pvalue;
    int		nrects;
{
    CHEAP_FUNC_PROLOGUE (pGC);

    (*pGC->funcs->ChangeClip) (pGC, type, pvalue, nrects);

    CHEAP_FUNC_EPILOGUE (pGC);
}

static void
miBSCheapCopyClip(pgcDst, pgcSrc)
    GCPtr pgcDst, pgcSrc;
{
    CHEAP_FUNC_PROLOGUE (pgcDst);

    (* pgcDst->funcs->CopyClip)(pgcDst, pgcSrc);

    CHEAP_FUNC_EPILOGUE (pgcDst);
}

static void
miBSCheapDestroyClip(pGC)
    GCPtr	pGC;
{
    CHEAP_FUNC_PROLOGUE (pGC);

    (* pGC->funcs->DestroyClip)(pGC);

    CHEAP_FUNC_EPILOGUE (pGC);
}

/*
 * create the full func/op wrappers for a GC
 */

static Bool
miBSCreateGCPrivate (pGC)
    GCPtr   pGC;
{
    miBSGCRec	*pPriv;

    pPriv = (miBSGCRec *) xalloc (sizeof (miBSGCRec));
    if (!pPriv)
	return FALSE;
    pPriv->pBackingGC = NULL;
    pPriv->guarantee = GuaranteeNothing;
    pPriv->serialNumber = 0;
    pPriv->stateChanges = (1 << (GCLastBit + 1)) - 1;
    pPriv->wrapOps = pGC->ops;
    pPriv->wrapFuncs = pGC->funcs;
    pGC->funcs = &miBSGCFuncs;
    pGC->ops = &miBSGCOps;
    pGC->devPrivates[miBSGCIndex].ptr = (pointer) pPriv;
    return TRUE;
}

static void
miBSDestroyGCPrivate (GCPtr pGC)
{
    miBSGCRec	*pPriv;

    pPriv = (miBSGCRec *) pGC->devPrivates[miBSGCIndex].ptr;
    if (pPriv)
    {
	pGC->devPrivates[miBSGCIndex].ptr = (pointer) pPriv->wrapFuncs;
	pGC->funcs = &miBSCheapGCFuncs;
	pGC->ops = pPriv->wrapOps;
	if (pPriv->pBackingGC)
	    FreeGC (pPriv->pBackingGC, (GContext) 0);
	xfree ((pointer) pPriv);
    }
}

/*
 * GC ops -- wrap each GC operation with our own function
 */

/*-
 *-----------------------------------------------------------------------
 * miBSFillSpans --
 *	Perform a FillSpans, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSFillSpans(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
    DDXPointPtr	pptCopy, pptReset;
    int 	*pwidthCopy;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pptCopy = (DDXPointPtr)ALLOCATE_LOCAL(nInit*sizeof(DDXPointRec));
    pwidthCopy=(int *)ALLOCATE_LOCAL(nInit*sizeof(int));
    if (pptCopy && pwidthCopy)
    {
	copyData(pptInit, pptCopy, nInit, MoreCopy0);
	memmove((char *)pwidthCopy,(char *)pwidthInit,nInit*sizeof(int));

	(* pGC->ops->FillSpans)(pDrawable, pGC, nInit, pptInit,
			     pwidthInit, fSorted);
	if (pGC->miTranslate)
	{
	    int	dx, dy;
	    int	nReset;

	    pptReset = pptCopy;
	    dx = pDrawable->x - pBackingDrawable->x;
	    dy = pDrawable->y - pBackingDrawable->y;
	    nReset = nInit;
	    while (nReset--)
	    {
		pptReset->x -= dx;
		pptReset->y -= dy;
		++pptReset;
	    }
	}
	(* pBackingGC->ops->FillSpans)(pBackingDrawable,
				  pBackingGC, nInit, pptCopy, pwidthCopy,
				  fSorted);
    }
    if (pwidthCopy) DEALLOCATE_LOCAL(pwidthCopy);
    if (pptCopy) DEALLOCATE_LOCAL(pptCopy);

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSSetSpans --
 *	Perform a SetSpans, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSSetSpans(pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted)
    DrawablePtr		pDrawable;
    GCPtr		pGC;
    char		*psrc;
    register DDXPointPtr ppt;
    int			*pwidth;
    int			nspans;
    int			fSorted;
{
    DDXPointPtr	pptCopy, pptReset;
    int 	*pwidthCopy;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pptCopy = (DDXPointPtr)ALLOCATE_LOCAL(nspans*sizeof(DDXPointRec));
    pwidthCopy=(int *)ALLOCATE_LOCAL(nspans*sizeof(int));
    if (pptCopy && pwidthCopy)
    {
	copyData(ppt, pptCopy, nspans, MoreCopy0);
	memmove((char *)pwidthCopy,(char *)pwidth,nspans*sizeof(int));

	(* pGC->ops->SetSpans)(pDrawable, pGC, psrc, ppt, pwidth,
			       nspans, fSorted);
	if (pGC->miTranslate)
	{
	    int	dx, dy;
	    int	nReset;

	    pptReset = pptCopy;
	    dx = pDrawable->x - pBackingDrawable->x;
	    dy = pDrawable->y - pBackingDrawable->y;
	    nReset = nspans;
	    while (nReset--)
	    {
		pptReset->x -= dx;
		pptReset->y -= dy;
		++pptReset;
	    }
	}
	(* pBackingGC->ops->SetSpans)(pBackingDrawable, pBackingGC,
				psrc, pptCopy, pwidthCopy, nspans, fSorted);
    }
    if (pwidthCopy) DEALLOCATE_LOCAL(pwidthCopy);
    if (pptCopy) DEALLOCATE_LOCAL(pptCopy);

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSPutImage --
 *	Perform a PutImage, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPutImage(pDrawable, pGC, depth, x, y, w, h, leftPad, format, pBits)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int		  depth;
    int	    	  x;
    int	    	  y;
    int	    	  w;
    int	    	  h;
    int		  leftPad;
    int	    	  format;
    char    	  *pBits;
{
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    (*pGC->ops->PutImage)(pDrawable, pGC,
		     depth, x, y, w, h, leftPad, format, pBits);
    (*pBackingGC->ops->PutImage)(pBackingDrawable, pBackingGC,
		     depth, x - pBackingStore->x, y - pBackingStore->y,
		     w, h, leftPad, format, pBits);

    EPILOGUE (pGC);
}

typedef RegionPtr (* CopyAreaProcPtr)(DrawablePtr, DrawablePtr, GCPtr,
				      int, int, int, int, int, int);
typedef RegionPtr (* CopyPlaneProcPtr)(DrawablePtr, DrawablePtr, GCPtr,
				      int, int, int, int, int, int,
				      unsigned long bitPlane);
/*-
 *-----------------------------------------------------------------------
 * miBSDoCopy --
 *	Perform a CopyArea or CopyPlane within a window that has backing
 *	store enabled.
 *
 * Results:
 *	TRUE if the copy was performed or FALSE if a regular one should
 *	be done.
 *
 * Side Effects:
 *	Things are copied (no s***!)
 *
 * Notes:
 *	The idea here is to form two regions that cover the source box.
 *	One contains the exposed rectangles while the other contains
 *	the obscured ones. An array of <box, drawable> pairs is then
 *	formed where the <box> indicates the area to be copied and the
 *	<drawable> indicates from where it is to be copied (exposed regions
 *	come from the screen while obscured ones come from the backing
 *	pixmap). The array 'sequence' is then filled with the indices of
 *	the pairs in the order in which they should be copied to prevent
 *	things from getting screwed up. A call is also made through the
 *	backingGC to take care of any copying into the backing pixmap.
 *
 *-----------------------------------------------------------------------
 */
static Bool
miBSDoCopy(
    WindowPtr	  pWin,	    	    /* Window being scrolled */
    GCPtr   	  pGC,	    	    /* GC we're called through */
    int	    	  srcx,	    	    /* X of source rectangle */
    int	    	  srcy,	    	    /* Y of source rectangle */
    int	    	  w,	    	    /* Width of source rectangle */
    int	    	  h,	    	    /* Height of source rectangle */
    int	    	  dstx,	    	    /* X of destination rectangle */
    int	    	  dsty,	    	    /* Y of destination rectangle */
    unsigned long plane,    	    /* Plane to copy (0 for CopyArea) */
    CopyPlaneProcPtr copyProc,      /* Procedure to call to perform the copy */
    RegionPtr	  *ppRgn)	    /* resultant Graphics Expose region */
{
    RegionPtr 	    	pRgnExp;    /* Exposed region */
    RegionPtr	  	pRgnObs;    /* Obscured region */
    BoxRec	  	box;	    /* Source box (screen coord) */
    struct BoxDraw {
	BoxPtr	  	pBox;	    	/* Source box */
	enum {
	    win, pix
	}   	  	source;	    	/* Place from which to copy */
    }	    	  	*boxes;	    /* Array of box/drawable pairs covering
				     * source box. */
    int  	  	*sequence;  /* Sequence of boxes to move */
    register int  	i, j, k, l, y;
    register BoxPtr	pBox;
    int	    	  	dx, dy, nrects;
    Bool    	  	graphicsExposures;
    CopyPlaneProcPtr  	pixCopyProc;
    int			numRectsExp, numRectsObs;
    BoxPtr		pBoxExp, pBoxObs;

    SETUP_BACKING (pWin, pGC);
    (void)oldFuncs;

    /*
     * Create a region of exposed boxes in pRgnExp.
     */
    box.x1 = srcx + pWin->drawable.x;
    box.x2 = box.x1 + w;
    box.y1 = srcy + pWin->drawable.y;
    box.y2 = box.y1 + h;
    
    pRgnExp = REGION_CREATE(pGC->pScreen, &box, 1);
    REGION_INTERSECT(pGC->pScreen, pRgnExp, pRgnExp, &pWin->clipList);
    pRgnObs = REGION_CREATE(pGC->pScreen, NULL, 1);
    REGION_INVERSE( pGC->pScreen, pRgnObs, pRgnExp, &box);

    /*
     * Translate regions into window coordinates for proper calls
     * to the copyProc, then make sure none of the obscured region sticks
     * into invalid areas of the backing pixmap.
     */
    REGION_TRANSLATE(pGC->pScreen, pRgnExp,
				      -pWin->drawable.x,
				      -pWin->drawable.y);
    REGION_TRANSLATE(pGC->pScreen, pRgnObs,
				      -pWin->drawable.x,
				      -pWin->drawable.y);
    REGION_INTERSECT(pGC->pScreen, pRgnObs, pRgnObs, &pBackingStore->SavedRegion);

    /*
     * If the obscured region is empty, there's no point being fancy.
     */
    if (!REGION_NOTEMPTY(pGC->pScreen, pRgnObs))
    {
	REGION_DESTROY(pGC->pScreen, pRgnExp);
	REGION_DESTROY(pGC->pScreen, pRgnObs);

	return (FALSE);
    }

    numRectsExp = REGION_NUM_RECTS(pRgnExp);
    pBoxExp = REGION_RECTS(pRgnExp);
    pBoxObs = REGION_RECTS(pRgnObs);
    numRectsObs = REGION_NUM_RECTS(pRgnObs);
    nrects = numRectsExp + numRectsObs;
    
    boxes = (struct BoxDraw *)ALLOCATE_LOCAL(nrects * sizeof(struct BoxDraw));
    sequence = (int *) ALLOCATE_LOCAL(nrects * sizeof(int));
    *ppRgn = NULL;

    if (!boxes || !sequence)
    {
	if (sequence) DEALLOCATE_LOCAL(sequence);
	if (boxes) DEALLOCATE_LOCAL(boxes);
	REGION_DESTROY(pGC->pScreen, pRgnExp);
	REGION_DESTROY(pGC->pScreen, pRgnObs);

	return(TRUE);
    }

    /*
     * Order the boxes in the two regions so we know from which drawable
     * to copy which box, storing the result in the boxes array
     */
    for (i = 0, j = 0, k = 0;
	 (i < numRectsExp) && (j < numRectsObs);
	 k++)
    {
	if (pBoxExp[i].y1 < pBoxObs[j].y1)
	{
	    boxes[k].pBox = &pBoxExp[i];
	    boxes[k].source = win;
	    i++;
	}
	else if ((pBoxObs[j].y1 < pBoxExp[i].y1) ||
		 (pBoxObs[j].x1 < pBoxExp[i].x1))
	{
	    boxes[k].pBox = &pBoxObs[j];
	    boxes[k].source = pix;
	    j++;
	}
	else
	{
	    boxes[k].pBox = &pBoxExp[i];
	    boxes[k].source = win;
	    i++;
	}
    }

    /*
     * Catch any leftover boxes from either region (note that only
     * one can have leftover boxes...)
     */
    if (i != numRectsExp)
    {
	do
	{
	    boxes[k].pBox = &pBoxExp[i];
	    boxes[k].source = win;
	    i++;
	    k++;
	} while (i < numRectsExp);

    }
    else
    {
	do
	{
	    boxes[k].pBox = &pBoxObs[j];
	    boxes[k].source = pix;
	    j++;
	    k++;
	} while (j < numRectsObs);
    }
    
    if (dsty <= srcy)
    {
	/*
	 * Scroll up or vertically stationary, so vertical order is ok.
	 */
	if (dstx <= srcx)
	{
	    /*
	     * Scroll left or horizontally stationary, so horizontal order
	     * is ok as well.
	     */
	    for (i = 0; i < nrects; i++)
	    {
		sequence[i] = i;
	    }
	}
	else
	{
	    /*
	     * Scroll right. Need to reverse the rectangles within each
	     * band.
	     */
	    for (i = 0, j = 1, k = 0;
		 i < nrects;
		 j = i + 1, k = i)
	    {
		y = boxes[i].pBox->y1;
		while ((j < nrects) && (boxes[j].pBox->y1 == y))
		{
		    j++;
		}
		for (j--; j >= k; j--, i++)
		{
		    sequence[i] = j;
		}
	    }
	}
    }
    else
    {
	/*
	 * Scroll down. Must reverse vertical banding, at least.
	 */
	if (dstx < srcx)
	{
	    /*
	     * Scroll left. Horizontal order is ok.
	     */
	    for (i = nrects - 1, j = i - 1, k = i, l = 0;
		 i >= 0;
		 j = i - 1, k = i)
	    {
		/*
		 * Find extent of current horizontal band, then reverse
		 * the order of the whole band.
		 */
		y = boxes[i].pBox->y1;
		while ((j >= 0) && (boxes[j].pBox->y1 == y))
		{
		    j--;
		}
		for (j++; j <= k; j++, i--, l++)
		{
		    sequence[l] = j;
		}
	    }
	}
	else
	{
	    /*
	     * Scroll right or horizontal stationary.
	     * Reverse horizontal order as well (if stationary, horizontal
	     * order can be swapped without penalty and this is faster
             * to compute).
	     */
	    for (i = 0, j = nrects - 1; i < nrects; i++, j--)
	    {
		sequence[i] = j;
	    }
	}
    }
	    
    /*
     * XXX: To avoid getting multiple NoExpose events from this operation,
     * we turn OFF graphicsExposures in the gc and deal with any uncopied
     * areas later, if there's something not in backing-store.
     */

    graphicsExposures = pGC->graphicsExposures;
    pGC->graphicsExposures = FALSE;
    
    dx = dstx - srcx;
    dy = dsty - srcy;

    /*
     * Figure out which copy procedure to use from the backing GC. Note we
     * must do this because some implementations (sun's, e.g.) have
     * pBackingGC a fake GC with the real one below it, thus the devPriv for
     * pBackingGC won't be what the output library expects.
     */
    if (plane != 0)
    {
	pixCopyProc = pBackingGC->ops->CopyPlane;
    }
    else
    {
	pixCopyProc = (CopyPlaneProcPtr)pBackingGC->ops->CopyArea;
    }
    
    for (i = 0; i < nrects; i++)
    {
	pBox = boxes[sequence[i]].pBox;
	
	/*
	 * If we're copying from the pixmap, we need to place its contents
	 * onto the screen before scrolling the pixmap itself. If we're copying
	 * from the window, we need to copy its contents into the pixmap before
	 * we scroll the window itself.
	 */
	if (boxes[sequence[i]].source == pix)
	{
	    (void) (* copyProc) (pBackingDrawable, &(pWin->drawable), pGC,
			  pBox->x1 - pBackingStore->x,
			  pBox->y1 - pBackingStore->y,
			  pBox->x2 - pBox->x1, pBox->y2 - pBox->y1,
			  pBox->x1 + dx, pBox->y1 + dy, plane);
	    (void) (* pixCopyProc) (pBackingDrawable, pBackingDrawable, pBackingGC,
			     pBox->x1 - pBackingStore->x,
			     pBox->y1 - pBackingStore->y,
			     pBox->x2 - pBox->x1, pBox->y2 - pBox->y1,
			     pBox->x1 + dx - pBackingStore->x,
			     pBox->y1 + dy - pBackingStore->y, plane);
	}
	else
	{
	    (void) (* pixCopyProc) (&(pWin->drawable), pBackingDrawable, pBackingGC,
			     pBox->x1, pBox->y1,
			     pBox->x2 - pBox->x1, pBox->y2 - pBox->y1,
			     pBox->x1 + dx - pBackingStore->x,
			     pBox->y1 + dy - pBackingStore->y, plane);
	    (void) (* copyProc) (&(pWin->drawable), &(pWin->drawable), pGC,
			  pBox->x1, pBox->y1,
			  pBox->x2 - pBox->x1, pBox->y2 - pBox->y1,
			  pBox->x1 + dx, pBox->y1 + dy, plane);
	}
    }
    DEALLOCATE_LOCAL(sequence);
    DEALLOCATE_LOCAL(boxes);

    pGC->graphicsExposures = graphicsExposures;
    /*
     * Form union of rgnExp and rgnObs and see if covers entire area
     * to be copied.  Store the resultant region for miBSCopyArea
     * to return to dispatch which will send the appropriate expose
     * events.
     */
    REGION_UNION(pGC->pScreen, pRgnExp, pRgnExp, pRgnObs);
    box.x1 = srcx;
    box.x2 = srcx + w;
    box.y1 = srcy;
    box.y2 = srcy + h;
    if (RECT_IN_REGION(pGC->pScreen, pRgnExp, &box) == rgnIN)
    {
	REGION_EMPTY(pGC->pScreen, pRgnExp);
    }
    else
    {
	REGION_INVERSE( pGC->pScreen, pRgnExp, pRgnExp, &box);
	REGION_TRANSLATE( pGC->pScreen, pRgnExp,
					   dx + pWin->drawable.x,
 					   dy + pWin->drawable.y);
	REGION_INTERSECT( pGC->pScreen, pRgnObs, pRgnExp, &pWin->clipList);
	(*pWin->drawable.pScreen->PaintWindowBackground) (pWin,
						pRgnObs, PW_BACKGROUND);
	REGION_TRANSLATE( pGC->pScreen, pRgnExp,
					   -pWin->drawable.x,
 					   -pWin->drawable.y);
	miBSClearBackingRegion (pWin, pRgnExp);
    }
    if (graphicsExposures)
	*ppRgn = pRgnExp;
    else
	REGION_DESTROY(pGC->pScreen, pRgnExp);
    REGION_DESTROY(pGC->pScreen, pRgnObs);

    return (TRUE);
}

/*-
 *-----------------------------------------------------------------------
 * miBSCopyArea --
 *	Perform a CopyArea from the source to the destination, extracting
 *	from the source's backing-store and storing into the destination's
 *	backing-store without messing anything up. If the source and
 *	destination are different, there's not too much to worry about:
 *	we can just issue several calls to the regular CopyArea function.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static RegionPtr
miBSCopyArea (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty)
    DrawablePtr	  pSrc;
    DrawablePtr	  pDst;
    GCPtr   	  pGC;
    int	    	  srcx;
    int	    	  srcy;
    int	    	  w;
    int	    	  h;
    int	    	  dstx;
    int	    	  dsty;
{
    BoxPtr	pExtents;
    long	dx, dy;
    int		bsrcx, bsrcy, bw, bh, bdstx, bdsty;
    RegionPtr	pixExposed = 0, winExposed = 0;

    SETUP_BACKING(pDst, pGC);

    PROLOGUE(pGC);

    if ((pSrc != pDst) ||
	(!miBSDoCopy((WindowPtr)pSrc, pGC, srcx, srcy, w, h, dstx, dsty,
		     (unsigned long) 0, (CopyPlaneProcPtr)pGC->ops->CopyArea,
		     &winExposed)))
    {
	/*
	 * always copy to the backing store first, miBSDoCopy
	 * returns FALSE if the *source* region is disjoint
	 * from the backing store saved region.  So, copying
	 * *to* the backing store is always safe
	 */
	if (pGC->clientClipType != CT_PIXMAP)
	{
	    /*
	     * adjust srcx, srcy, w, h, dstx, dsty to be clipped to
	     * the backing store.  An unnecessary optimisation,
	     * but a useful one when GetSpans is slow.
	     */
	    pExtents = REGION_EXTENTS(pDst->pScreen,
				      (RegionPtr)pBackingGC->clientClip);
	    bsrcx = srcx;
	    bsrcy = srcy;
	    bw = w;
	    bh = h;
	    bdstx = dstx;
	    bdsty = dsty;
	    dx = pExtents->x1 - bdstx;
	    if (dx > 0)
	    {
		bsrcx += dx;
		bdstx += dx;
		bw -= dx;
	    }
	    dy = pExtents->y1 - bdsty;
	    if (dy > 0)
	    {
		bsrcy += dy;
		bdsty += dy;
		bh -= dy;
	    }
	    dx = (bdstx + bw) - pExtents->x2;
	    if (dx > 0)
		bw -= dx;
	    dy = (bdsty + bh) - pExtents->y2;
	    if (dy > 0)
		bh -= dy;
	    if (bw > 0 && bh > 0)
		pixExposed = (* pBackingGC->ops->CopyArea) (pSrc, 
			    pBackingDrawable, pBackingGC, 
			    bsrcx, bsrcy, bw, bh, bdstx - pBackingStore->x,
			    bdsty - pBackingStore->y);
	}
	else
	    pixExposed = (* pBackingGC->ops->CopyArea) (pSrc, 
			    pBackingDrawable, pBackingGC,
			    srcx, srcy, w, h,
			    dstx - pBackingStore->x, dsty - pBackingStore->y);

	winExposed = (* pGC->ops->CopyArea) (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty);
    }

    /*
     * compute the composite graphics exposure region
     */
    if (winExposed)
    {
	if (pixExposed){
	    REGION_UNION(pDst->pScreen, winExposed, winExposed, pixExposed);
	    REGION_DESTROY(pDst->pScreen, pixExposed);
	}
    } else
	winExposed = pixExposed;

    EPILOGUE (pGC);

    return winExposed;
}

/*-
 *-----------------------------------------------------------------------
 * miBSCopyPlane --
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static RegionPtr
miBSCopyPlane (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, plane)
    DrawablePtr	  pSrc;
    DrawablePtr	  pDst;
    register GC   *pGC;
    int     	  srcx,
		  srcy;
    int     	  w,
		  h;
    int     	  dstx,
		  dsty;
    unsigned long  plane;
{
    BoxPtr	pExtents;
    long	dx, dy;
    int		bsrcx, bsrcy, bw, bh, bdstx, bdsty;
    RegionPtr	winExposed = 0, pixExposed = 0;
    SETUP_BACKING(pDst, pGC);

    PROLOGUE(pGC);

    if ((pSrc != pDst) ||
	(!miBSDoCopy((WindowPtr)pSrc, pGC, srcx, srcy, w, h, dstx, dsty,
		     plane,  pGC->ops->CopyPlane, &winExposed)))
    {
	/*
	 * always copy to the backing store first, miBSDoCopy
	 * returns FALSE if the *source* region is disjoint
	 * from the backing store saved region.  So, copying
	 * *to* the backing store is always safe
	 */
	if (pGC->clientClipType != CT_PIXMAP)
	{
	    /*
	     * adjust srcx, srcy, w, h, dstx, dsty to be clipped to
	     * the backing store.  An unnecessary optimisation,
	     * but a useful one when GetSpans is slow.
	     */
	    pExtents = REGION_EXTENTS(pDst->pScreen,
				      (RegionPtr)pBackingGC->clientClip);
	    bsrcx = srcx;
	    bsrcy = srcy;
	    bw = w;
	    bh = h;
	    bdstx = dstx;
	    bdsty = dsty;
	    dx = pExtents->x1 - bdstx;
	    if (dx > 0)
	    {
		bsrcx += dx;
		bdstx += dx;
		bw -= dx;
	    }
	    dy = pExtents->y1 - bdsty;
	    if (dy > 0)
	    {
		bsrcy += dy;
		bdsty += dy;
		bh -= dy;
	    }
	    dx = (bdstx + bw) - pExtents->x2;
	    if (dx > 0)
		bw -= dx;
	    dy = (bdsty + bh) - pExtents->y2;
	    if (dy > 0)
		bh -= dy;
	    if (bw > 0 && bh > 0)
		pixExposed = (* pBackingGC->ops->CopyPlane) (pSrc, 
				    pBackingDrawable,
				    pBackingGC, bsrcx, bsrcy, bw, bh,
				    bdstx - pBackingStore->x,
				    bdsty - pBackingStore->y, plane);
	}
	else
	    pixExposed = (* pBackingGC->ops->CopyPlane) (pSrc, 
				    pBackingDrawable,
				    pBackingGC, srcx, srcy, w, h,
				    dstx - pBackingStore->x,
				    dsty - pBackingStore->y, plane);

	winExposed = (* pGC->ops->CopyPlane) (pSrc, pDst, pGC, srcx, srcy, w, h,
			      dstx, dsty, plane);
	
    }

    /*
     * compute the composite graphics exposure region
     */
    if (winExposed)
    {
	if (pixExposed)
	{
	    REGION_UNION(pDst->pScreen, winExposed, winExposed, pixExposed);
	    REGION_DESTROY(pDst->pScreen, pixExposed);
	}
    } else
	winExposed = pixExposed;

    EPILOGUE (pGC);

    return winExposed;
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyPoint --
 *	Perform a PolyPoint, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPolyPoint (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		mode;		/* Origin or Previous */
    int		npt;
    xPoint 	*pptInit;
{
    xPoint	  *pptCopy;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pptCopy = (xPoint *)ALLOCATE_LOCAL(npt*sizeof(xPoint));
    if (pptCopy)
    {
	copyPoints(pptInit, pptCopy, npt, mode);

	(* pGC->ops->PolyPoint) (pDrawable, pGC, mode, npt, pptInit);

	(* pBackingGC->ops->PolyPoint) (pBackingDrawable,
				   pBackingGC, mode, npt, pptCopy);

	DEALLOCATE_LOCAL(pptCopy);
    }

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyLines --
 *	Perform a Polylines, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPolylines (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int	    	  mode;
    int	    	  npt;
    DDXPointPtr	  pptInit;
{
    DDXPointPtr	pptCopy;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pptCopy = (DDXPointPtr)ALLOCATE_LOCAL(npt*sizeof(DDXPointRec));
    if (pptCopy)
    {
	copyPoints(pptInit, pptCopy, npt, mode);

	(* pGC->ops->Polylines)(pDrawable, pGC, mode, npt, pptInit);
	(* pBackingGC->ops->Polylines)(pBackingDrawable,
				  pBackingGC, mode, npt, pptCopy);
	DEALLOCATE_LOCAL(pptCopy);
    }

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolySegment --
 *	Perform a PolySegment, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPolySegment(pDrawable, pGC, nseg, pSegs)
    DrawablePtr pDrawable;
    GCPtr 	pGC;
    int		nseg;
    xSegment	*pSegs;
{
    xSegment	*pSegsCopy;

    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pSegsCopy = (xSegment *)ALLOCATE_LOCAL(nseg*sizeof(xSegment));
    if (pSegsCopy)
    {
	copyData(pSegs, pSegsCopy, nseg << 1, MoreCopy0);

	(* pGC->ops->PolySegment)(pDrawable, pGC, nseg, pSegs);
	(* pBackingGC->ops->PolySegment)(pBackingDrawable,
				    pBackingGC, nseg, pSegsCopy);

	DEALLOCATE_LOCAL(pSegsCopy);
    }

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyRectangle --
 *	Perform a PolyRectangle, routing output to backing-store as needed.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPolyRectangle(pDrawable, pGC, nrects, pRects)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		nrects;
    xRectangle	*pRects;
{
    xRectangle	*pRectsCopy;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pRectsCopy =(xRectangle *)ALLOCATE_LOCAL(nrects*sizeof(xRectangle));
    if (pRectsCopy)
    {
	copyData(pRects, pRectsCopy, nrects, MoreCopy2);

	(* pGC->ops->PolyRectangle)(pDrawable, pGC, nrects, pRects);
	(* pBackingGC->ops->PolyRectangle)(pBackingDrawable,
				      pBackingGC, nrects, pRectsCopy);

	DEALLOCATE_LOCAL(pRectsCopy);
    }

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyArc --
 *	Perform a PolyArc, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPolyArc(pDrawable, pGC, narcs, parcs)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    xArc  *pArcsCopy;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pArcsCopy = (xArc *)ALLOCATE_LOCAL(narcs*sizeof(xArc));
    if (pArcsCopy)
    {
	copyData(parcs, pArcsCopy, narcs, MoreCopy4);

	(* pGC->ops->PolyArc)(pDrawable, pGC, narcs, parcs);
	(* pBackingGC->ops->PolyArc)(pBackingDrawable, pBackingGC,
				narcs, pArcsCopy);

	DEALLOCATE_LOCAL(pArcsCopy);
    }

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSFillPolygon --
 *	Perform a FillPolygon, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSFillPolygon(pDrawable, pGC, shape, mode, count, pPts)
    DrawablePtr		pDrawable;
    register GCPtr	pGC;
    int			shape, mode;
    register int	count;
    DDXPointPtr		pPts;
{
    DDXPointPtr	pPtsCopy;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pPtsCopy = (DDXPointPtr)ALLOCATE_LOCAL(count*sizeof(DDXPointRec));
    if (pPtsCopy)
    {
	copyPoints(pPts, pPtsCopy, count, mode);
	(* pGC->ops->FillPolygon)(pDrawable, pGC, shape, mode, count, pPts);
	(* pBackingGC->ops->FillPolygon)(pBackingDrawable,
				    pBackingGC, shape, mode,
				    count, pPtsCopy);

	DEALLOCATE_LOCAL(pPtsCopy);
    }

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyFillRect --
 *	Perform a PolyFillRect, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPolyFillRect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
    xRectangle	*pRectCopy;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pRectCopy =
	(xRectangle *)ALLOCATE_LOCAL(nrectFill*sizeof(xRectangle));
    if (pRectCopy)
    {
	copyData(prectInit, pRectCopy, nrectFill, MoreCopy2);

	(* pGC->ops->PolyFillRect)(pDrawable, pGC, nrectFill, prectInit);
	(* pBackingGC->ops->PolyFillRect)(pBackingDrawable,
				     pBackingGC, nrectFill, pRectCopy);

	DEALLOCATE_LOCAL(pRectCopy);
    }

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyFillArc --
 *	Perform a PolyFillArc, routing output to backing-store as needed.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPolyFillArc(pDrawable, pGC, narcs, parcs)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    xArc  *pArcsCopy;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    pArcsCopy = (xArc *)ALLOCATE_LOCAL(narcs*sizeof(xArc));
    if (pArcsCopy)
    {
	copyData(parcs, pArcsCopy, narcs, MoreCopy4);
	(* pGC->ops->PolyFillArc)(pDrawable, pGC, narcs, parcs);
	(* pBackingGC->ops->PolyFillArc)(pBackingDrawable,
				    pBackingGC, narcs, pArcsCopy);
	DEALLOCATE_LOCAL(pArcsCopy);
    }

    EPILOGUE (pGC);
}


/*-
 *-----------------------------------------------------------------------
 * miBSPolyText8 --
 *	Perform a PolyText8, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static int
miBSPolyText8(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int 	count;
    char	*chars;
{
    int	    result;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    result = (* pGC->ops->PolyText8)(pDrawable, pGC, x, y, count, chars);
    (* pBackingGC->ops->PolyText8)(pBackingDrawable, pBackingGC,
				   x - pBackingStore->x, y - pBackingStore->y,
				   count, chars);

    EPILOGUE (pGC);
    return result;
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyText16 --
 *	Perform a PolyText16, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static int
miBSPolyText16(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int		count;
    unsigned short *chars;
{
    int	result;
    SETUP_BACKING (pDrawable, pGC);

    PROLOGUE(pGC);

    result = (* pGC->ops->PolyText16)(pDrawable, pGC, x, y, count, chars);
    (* pBackingGC->ops->PolyText16)(pBackingDrawable, pBackingGC,
				    x - pBackingStore->x, y - pBackingStore->y,
				    count, chars);

    EPILOGUE (pGC);

    return result;
}

/*-
 *-----------------------------------------------------------------------
 * miBSImageText8 --
 *	Perform a ImageText8, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSImageText8(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int		count;
    char	*chars;
{
    SETUP_BACKING (pDrawable, pGC);
    PROLOGUE(pGC);

    (* pGC->ops->ImageText8)(pDrawable, pGC, x, y, count, chars);
    (* pBackingGC->ops->ImageText8)(pBackingDrawable, pBackingGC,
				    x - pBackingStore->x, y - pBackingStore->y,
				    count, chars);

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSImageText16 --
 *	Perform a ImageText16, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSImageText16(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int		count;
    unsigned short *chars;
{
    SETUP_BACKING (pDrawable, pGC);
    PROLOGUE(pGC);

    (* pGC->ops->ImageText16)(pDrawable, pGC, x, y, count, chars);
    (* pBackingGC->ops->ImageText16)(pBackingDrawable, pBackingGC,
				    x - pBackingStore->x, y - pBackingStore->y,
				     count, chars);

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSImageGlyphBlt --
 *	Perform a ImageGlyphBlt, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GCPtr 	pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    pointer 	pglyphBase;	/* start of array of glyphs */
{
    SETUP_BACKING (pDrawable, pGC);
    PROLOGUE(pGC);

    (* pGC->ops->ImageGlyphBlt)(pDrawable, pGC, x, y, nglyph, ppci,
			     pglyphBase);
    (* pBackingGC->ops->ImageGlyphBlt)(pBackingDrawable, pBackingGC,
				    x - pBackingStore->x, y - pBackingStore->y,
				       nglyph, ppci, pglyphBase);

    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSPolyGlyphBlt --
 *	Perform a PolyGlyphBlt, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    pointer	pglyphBase;	/* start of array of glyphs */
{
    SETUP_BACKING (pDrawable, pGC);
    PROLOGUE(pGC);

    (* pGC->ops->PolyGlyphBlt)(pDrawable, pGC, x, y, nglyph,
			    ppci, pglyphBase);
    (* pBackingGC->ops->PolyGlyphBlt)(pBackingDrawable, pBackingGC,
				    x - pBackingStore->x, y - pBackingStore->y,
				      nglyph, ppci, pglyphBase);
    EPILOGUE (pGC);
}

/*-
 *-----------------------------------------------------------------------
 * miBSPushPixels --
 *	Perform a PushPixels, routing output to backing-store as needed.
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
miBSPushPixels(pGC, pBitMap, pDst, w, h, x, y)
    GCPtr	pGC;
    PixmapPtr	pBitMap;
    DrawablePtr pDst;
    int		w, h, x, y;
{
    SETUP_BACKING (pDst, pGC);
    PROLOGUE(pGC);

    (* pGC->ops->PushPixels)(pGC, pBitMap, pDst, w, h, x, y);
    if (pGC->miTranslate) {
 	x -= pDst->x;
 	y -= pDst->y;
    }
    (* pBackingGC->ops->PushPixels)(pBackingGC, pBitMap,
			       pBackingDrawable, w, h,
			       x - pBackingStore->x, y - pBackingStore->y);

    EPILOGUE (pGC);
}

#ifdef NEED_LINEHELPER
/*-
 *-----------------------------------------------------------------------
 * miBSLineHelper --
 *
 * Results: should never be called
 *
 * Side Effects: server dies
 *
 *-----------------------------------------------------------------------
 */
static void
miBSLineHelper()
{
    FatalError("miBSLineHelper called\n");
}
#endif

/*-
 *-----------------------------------------------------------------------
 * miBSClearBackingStore --
 *	Clear the given area of the backing pixmap with the background of
 *	the window, whatever it is. If generateExposures is TRUE, generate
 *	exposure events for the area. Note that if the area has any
 *	part outside the saved portions of the window, we do not allow the
 *	count in the expose events to be 0, since there will be more
 *	expose events to come.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Areas of pixmap are cleared and Expose events are generated.
 *
 *-----------------------------------------------------------------------
 */
static RegionPtr
miBSClearBackingStore(pWin, x, y, w, h, generateExposures)
    WindowPtr	  	pWin;
    int	    	  	x;
    int	    	  	y;
    int	    	  	w;
    int	    	  	h;
    Bool    	  	generateExposures;
{
    RegionPtr	  	pRgn;
    int	    	  	i;
    miBSWindowPtr	pBackingStore;
    ScreenPtr	  	pScreen;
    GCPtr   	  	pGC;
    int	    	  	ts_x_origin,
			ts_y_origin;
    pointer    	  	gcvalues[4];
    unsigned long 	gcmask;
    xRectangle	  	*rects;
    BoxPtr  	  	pBox;
    BoxRec  	  	box;
    PixUnion		background;
    char		backgroundState;
    int			numRects;

    pBackingStore = (miBSWindowPtr)pWin->backStorage;
    pScreen = pWin->drawable.pScreen;

    if ((pBackingStore->status == StatusNoPixmap) ||
	(pBackingStore->status == StatusBadAlloc))
	return NullRegion;
    
    if (w == 0)
	w = (int) pWin->drawable.width - x;
    if (h == 0)
	h = (int) pWin->drawable.height - y;

    box.x1 = x;
    box.y1 = y;
    box.x2 = x + w;
    box.y2 = y + h;
    pRgn = REGION_CREATE(pWin->drawable.pScreen, &box, 1);
    if (!pRgn)
	return NullRegion;
    REGION_INTERSECT( pScreen, pRgn, pRgn, &pBackingStore->SavedRegion);

    if (REGION_NOTEMPTY( pScreen, pRgn))
    {
	/*
	 * if clearing entire window, simply make new virtual
	 * tile.  For the root window, we also destroy the pixmap
	 * to save a pile of memory
	 */
	if (x == 0 && y == 0 &&
 	    w == pWin->drawable.width &&
 	    h == pWin->drawable.height)
	{
	    if (!pWin->parent)
		miDestroyBSPixmap (pWin);
	    if (pBackingStore->status != StatusContents)
		 miTileVirtualBS (pWin);
	}

	ts_x_origin = ts_y_origin = 0;

	backgroundState = pWin->backgroundState;
	background = pWin->background;
	if (backgroundState == ParentRelative) {
	    WindowPtr	pParent;

	    pParent = pWin;
	    while (pParent->backgroundState == ParentRelative) {
		ts_x_origin -= pParent->origin.x;
		ts_y_origin -= pParent->origin.y;
		pParent = pParent->parent;
	    }
	    backgroundState = pParent->backgroundState;
	    background = pParent->background;
	}

	if ((backgroundState != None) &&
	    ((pBackingStore->status == StatusContents) ||
	     !SameBackground (pBackingStore->backgroundState,
			      pBackingStore->background,
			      backgroundState,
			      background)))
	{
	    if (!pBackingStore->pBackingPixmap)
		miCreateBSPixmap(pWin, NullBox);

	    pGC = GetScratchGC(pWin->drawable.depth, pScreen);
	    if (pGC && pBackingStore->pBackingPixmap)
	    {
		/*
		 * First take care of any ParentRelative stuff by altering the
		 * tile/stipple origin to match the coordinates of the upper-left
		 * corner of the first ancestor without a ParentRelative background.
		 * This coordinate is, of course, negative.
		 */
	    
		if (backgroundState == BackgroundPixel)
		{
		    gcvalues[0] = (pointer) background.pixel;
		    gcvalues[1] = (pointer)FillSolid;
		    gcmask = GCForeground|GCFillStyle;
		}
		else
		{
		    gcvalues[0] = (pointer)FillTiled;
		    gcvalues[1] = (pointer) background.pixmap;
		    gcmask = GCFillStyle|GCTile;
		}
		gcvalues[2] = (pointer)(long)(ts_x_origin - pBackingStore->x);
		gcvalues[3] = (pointer)(long)(ts_y_origin - pBackingStore->y);
		gcmask |= GCTileStipXOrigin|GCTileStipYOrigin;
		DoChangeGC(pGC, gcmask, (XID *)gcvalues, TRUE);
		ValidateGC((DrawablePtr)pBackingStore->pBackingPixmap, pGC);
    
		/*
		 * Figure out the array of rectangles to fill and fill them with
		 * PolyFillRect in the proper mode, as set in the GC above.
		 */
		numRects = REGION_NUM_RECTS(pRgn);
		rects = (xRectangle *)ALLOCATE_LOCAL(numRects*sizeof(xRectangle));
	    
		if (rects)
		{
		    for (i = 0, pBox = REGION_RECTS(pRgn);
			 i < numRects;
			 i++, pBox++)
		    {
			rects[i].x = pBox->x1 - pBackingStore->x;
			rects[i].y = pBox->y1 - pBackingStore->y;
			rects[i].width = pBox->x2 - pBox->x1;
			rects[i].height = pBox->y2 - pBox->y1;
		    }
		    (* pGC->ops->PolyFillRect) (
				(DrawablePtr)pBackingStore->pBackingPixmap,
				       pGC, numRects, rects);
		    DEALLOCATE_LOCAL(rects);
		}	
		FreeScratchGC(pGC);
	    }
	}	

	if (!generateExposures)
 	{
	    REGION_DESTROY(pScreen, pRgn);
	    pRgn = NULL;
	}
	else
	{
	    /*
	     * result must be screen relative, but is currently
	     * drawable relative.
	     */
	    REGION_TRANSLATE(pScreen, pRgn, pWin->drawable.x,
			     pWin->drawable.y);
	}
    }
    else
    {
	REGION_DESTROY( pScreen, pRgn);
	pRgn = NULL;
    }
    return pRgn;
}

static void
miBSClearBackingRegion (pWin, pRgn)
    WindowPtr	pWin;
    RegionPtr	pRgn;
{
    BoxPtr	pBox;
    int		i;

    i = REGION_NUM_RECTS(pRgn);
    pBox = REGION_RECTS(pRgn);
    while (i--)
    {
	(void) miBSClearBackingStore(pWin, pBox->x1, pBox->y1,
					pBox->x2 - pBox->x1,
					pBox->y2 - pBox->y1,
					FALSE);
	pBox++;
    }
}

/*
 * fill a region of the destination with virtual bits
 *
 * pRgn is to be translated by (x,y)
 */

static void
miBSFillVirtualBits (pDrawable, pGC, pRgn, x, y, state, pixunion, planeMask)
    DrawablePtr		pDrawable;
    GCPtr		pGC;
    RegionPtr		pRgn;
    int			x, y;
    int			state;
    PixUnion		pixunion;
    unsigned long	planeMask;
{
    int		i;
    BITS32	gcmask;
    pointer	gcval[5];
    xRectangle	*pRect;
    BoxPtr	pBox;
    WindowPtr	pWin;
    int		numRects;

    if (state == None)
	return;
    numRects = REGION_NUM_RECTS(pRgn);
    pRect = (xRectangle *)ALLOCATE_LOCAL(numRects * sizeof(xRectangle));
    if (!pRect)
	return;
    pWin = 0;
    if (pDrawable->type != DRAWABLE_PIXMAP)
    {
	pWin = (WindowPtr) pDrawable;
	if (!pWin->backStorage)
	    pWin = 0;
    }
    i = 0;
    gcmask = 0;
    gcval[i++] = (pointer)planeMask;
    gcmask |= GCPlaneMask;
    if (state == BackgroundPixel)
    {
	if (pGC->fgPixel != pixunion.pixel)
	{
	    gcval[i++] = (pointer)pixunion.pixel;
	    gcmask |= GCForeground;
	}
	if (pGC->fillStyle != FillSolid)
	{
	    gcval[i++] = (pointer)FillSolid;
	    gcmask |= GCFillStyle;
	}
    }
    else
    {
	if (pGC->fillStyle != FillTiled)
	{
	    gcval[i++] = (pointer)FillTiled;
	    gcmask |= GCFillStyle;
	}
	if (pGC->tileIsPixel || pGC->tile.pixmap != pixunion.pixmap)
	{
	    gcval[i++] = (pointer)pixunion.pixmap;
	    gcmask |= GCTile;
	}
	if (pGC->patOrg.x != x)
	{
	    gcval[i++] = (pointer)(long)x;
	    gcmask |= GCTileStipXOrigin;
	}
	if (pGC->patOrg.y != y)
	{
	    gcval[i++] = (pointer)(long)y;
	    gcmask |= GCTileStipYOrigin;
	}
    }
    if (gcmask)
	DoChangeGC (pGC, gcmask, (XID *)gcval, 1);

    if (pWin)
	(*pWin->drawable.pScreen->DrawGuarantee) (pWin, pGC, GuaranteeVisBack);

    if (pDrawable->serialNumber != pGC->serialNumber)
	ValidateGC (pDrawable, pGC);

    pBox = REGION_RECTS(pRgn);
    for (i = numRects; --i >= 0; pBox++, pRect++)
    {
    	pRect->x = pBox->x1 + x;
	pRect->y = pBox->y1 + y;
	pRect->width = pBox->x2 - pBox->x1;
	pRect->height = pBox->y2 - pBox->y1;
    }
    pRect -= numRects;
    (*pGC->ops->PolyFillRect) (pDrawable, pGC, numRects, pRect);
    if (pWin)
	(*pWin->drawable.pScreen->DrawGuarantee) (pWin, pGC, GuaranteeNothing);
    DEALLOCATE_LOCAL (pRect);
}

/*-
 *-----------------------------------------------------------------------
 * miBSAllocate --
 *	Create and install backing store info for a window
 *
 *-----------------------------------------------------------------------
 */

static void
miBSAllocate(pWin)
    WindowPtr 	  pWin;
{
    register miBSWindowPtr  pBackingStore;
    register ScreenPtr 	    pScreen;
	
    if (pWin->drawable.pScreen->backingStoreSupport == NotUseful)
	return;
    pScreen = pWin->drawable.pScreen;
    if (!(pBackingStore = (miBSWindowPtr)pWin->backStorage))
    {

	pBackingStore = (miBSWindowPtr)xalloc(sizeof(miBSWindowRec));
	if (!pBackingStore)
	    return;

	pBackingStore->pBackingPixmap = NullPixmap;
	pBackingStore->x = 0;
	pBackingStore->y = 0;
	REGION_NULL( pScreen, &pBackingStore->SavedRegion);
	pBackingStore->viewable = (char)pWin->viewable;
	pBackingStore->status = StatusNoPixmap;
	pBackingStore->backgroundState = None;
	pWin->backStorage = (pointer) pBackingStore;
    }
	
    /*
     * Now want to initialize the backing pixmap and SavedRegion if
     * necessary. The initialization consists of finding all the
     * currently-obscured regions, by taking the inverse of the window's
     * clip list, storing the result in SavedRegion, and exposing those
     * areas of the window.
     */

    if (pBackingStore->status == StatusNoPixmap &&
	((pWin->backingStore == WhenMapped && pWin->viewable) ||
	 (pWin->backingStore == Always)))
    {
	BoxRec  	box;
	RegionPtr	pSavedRegion;

	pSavedRegion = &pBackingStore->SavedRegion;

	box.x1 = pWin->drawable.x;
	box.x2 = box.x1 + (int) pWin->drawable.width;
	box.y1 = pWin->drawable.y;
	box.y2 = pWin->drawable.y + (int) pWin->drawable.height;

	REGION_INVERSE( pScreen, pSavedRegion, &pWin->clipList,  &box);
	REGION_TRANSLATE( pScreen, pSavedRegion,
				      -pWin->drawable.x,
				      -pWin->drawable.y);
#ifdef SHAPE
	if (wBoundingShape (pWin))
	    REGION_INTERSECT(pScreen, pSavedRegion, pSavedRegion,
			     wBoundingShape (pWin));
	if (wClipShape (pWin))
	    REGION_INTERSECT(pScreen, pSavedRegion, pSavedRegion,
			     wClipShape (pWin));
#endif
	/* if window is already on-screen, assume it has been drawn to */
	if (pWin->viewable)
	    pBackingStore->status = StatusVDirty;
	miTileVirtualBS (pWin);
	
	/*
	 * deliver all the newly available regions
	 * as exposure events to the window
	 */

	miSendExposures(pWin, pSavedRegion, 0, 0);
    }
    else if (!pWin->viewable)
    {
        /*
         * Turn off backing store when we're not supposed to
         * be saving anything
         */
        if (pBackingStore->status != StatusNoPixmap)
        {
            REGION_EMPTY( pScreen, &pBackingStore->SavedRegion);
            miDestroyBSPixmap (pWin);
        }
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSFree --
 *	Destroy and free all the stuff associated with the backing-store
 *	for the given window.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The backing pixmap and all the regions and GC's are destroyed.
 *
 *-----------------------------------------------------------------------
 */
static void
miBSFree(pWin)
    WindowPtr pWin;
{
    miBSWindowPtr 	pBackingStore;
    register ScreenPtr	pScreen;

    pScreen = pWin->drawable.pScreen;

    pBackingStore = (miBSWindowPtr)pWin->backStorage;
    if (pBackingStore)
    {
	miDestroyBSPixmap (pWin);

	REGION_UNINIT( pScreen, &pBackingStore->SavedRegion);

	xfree(pBackingStore);
	pWin->backStorage = NULL;
    }
}

/*-
 *-----------------------------------------------------------------------
 * miResizeBackingStore --
 *	Alter the size of the backing pixmap as necessary when the
 *	SavedRegion changes size. The contents of the old pixmap are
 *	copied/shifted into the new/same pixmap.
 *
 * Results:
 *	The new Pixmap is created as necessary.
 *
 * Side Effects:
 *	The old pixmap is destroyed.
 *
 *-----------------------------------------------------------------------
 */
static void
miResizeBackingStore(
    WindowPtr	pWin,
    int		dx,	    /* bits are moving this far */
    int		dy,	    /* bits are moving this far */
    Bool	saveBits)   /* bits are useful */
{
    miBSWindowPtr pBackingStore;
    PixmapPtr pBackingPixmap;
    ScreenPtr pScreen;
    GC	   *pGC;
    BoxPtr  extents;
    PixmapPtr pNewPixmap;
    int nx, ny;
    int	nw, nh;

    pBackingStore = (miBSWindowPtr)(pWin->backStorage);
    pBackingPixmap = pBackingStore->pBackingPixmap;
    if (!pBackingPixmap)
	return;
    pScreen = pWin->drawable.pScreen;
    extents = REGION_EXTENTS(pScreen, &pBackingStore->SavedRegion);
    pNewPixmap = pBackingPixmap;

    nw = extents->x2 - extents->x1;
    nh = extents->y2 - extents->y1;

    /* the policy here could be more sophisticated */
    if (nw != pBackingPixmap->drawable.width ||
	nh != pBackingPixmap->drawable.height)
    {
	if (!saveBits || !nw || !nh)
	{
	    pNewPixmap = NullPixmap;
	    pBackingStore->status = StatusNoPixmap;
	}
	else
	{
	    pNewPixmap = (PixmapPtr)(*pScreen->CreatePixmap)
					    (pScreen,
					     nw, nh,
					     pWin->drawable.depth);
	    if (!pNewPixmap)
	    {
#ifdef BSEAGER
		pBackingStore->status = StatusNoPixmap;
#else
		pBackingStore->status = StatusBadAlloc;
#endif
	    }
	}
    }
    if (!pNewPixmap)
    {
	pBackingStore->x = 0;
	pBackingStore->y = 0;
    }
    else
    {
    	nx = pBackingStore->x - extents->x1 + dx;
    	ny = pBackingStore->y - extents->y1 + dy;
    	pBackingStore->x = extents->x1;
    	pBackingStore->y = extents->y1;
    	
    	if (saveBits && (pNewPixmap != pBackingPixmap || nx != 0 || ny != 0))
    	{
    	    pGC = GetScratchGC(pNewPixmap->drawable.depth, pScreen);
    	    if (pGC)
    	    {
	    	ValidateGC((DrawablePtr)pNewPixmap, pGC);
	    	/* if we implement a policy where the pixmap can be larger than
		 * the region extents, we might want to optimize this copyarea
		 * by only copying the old extents, rather than the entire
		 * pixmap
		 */
	    	(*pGC->ops->CopyArea)((DrawablePtr)pBackingPixmap,
				      (DrawablePtr)pNewPixmap, pGC,
				      0, 0,
				      pBackingPixmap->drawable.width,
				      pBackingPixmap->drawable.height,
				      nx, ny);
	    	FreeScratchGC(pGC);
    	    }
    	}
    }
    /* SavedRegion is used in the backingGC clip; force an update */
    pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    if (pNewPixmap != pBackingPixmap)
    {
	(* pScreen->DestroyPixmap)(pBackingPixmap);
	pBackingStore->pBackingPixmap = pNewPixmap;
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSSaveDoomedAreas --
 *	Saved the areas of the given window that are about to be
 *	obscured.  If the window has moved, pObscured is expected to
 *	be at the new screen location and (dx,dy) is expected to be the offset
 *	to the window's previous location.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The region is copied from the screen into pBackingPixmap and
 *	SavedRegion is updated.
 *
 *-----------------------------------------------------------------------
 */
static void
miBSSaveDoomedAreas(pWin, pObscured, dx, dy)
    register WindowPtr pWin;
    RegionPtr 	       pObscured;
    int		       dx, dy;
{
    miBSWindowPtr 	pBackingStore;
    ScreenPtr	  	pScreen;
    int			x, y;

    pBackingStore = (miBSWindowPtr)pWin->backStorage;
    pScreen = pWin->drawable.pScreen;

    /*
     * If the window isn't realized, it's being unmapped, thus we don't
     * want to save anything if backingStore isn't Always.
     */
    if (!pWin->realized)
    {
	pBackingStore->viewable = (char)pWin->viewable;
	if (pWin->backingStore != Always)
	{
	    REGION_EMPTY( pScreen, &pBackingStore->SavedRegion);
	    miDestroyBSPixmap (pWin);
	    return;
	}
	if (pBackingStore->status == StatusBadAlloc)
	    pBackingStore->status = StatusNoPixmap;
    }

    /* Don't even pretend to save anything for a virtual background None */
    if ((pBackingStore->status == StatusVirtual) &&
	(pBackingStore->backgroundState == None))
	return;

    if (REGION_NOTEMPTY(pScreen, pObscured))
    {
	BoxRec	oldExtents;
	x = pWin->drawable.x;
	y = pWin->drawable.y;
	REGION_TRANSLATE(pScreen, pObscured, -x, -y);
	oldExtents = *REGION_EXTENTS(pScreen, &pBackingStore->SavedRegion);
	REGION_UNION( pScreen, &pBackingStore->SavedRegion,
			   &pBackingStore->SavedRegion,
			   pObscured);
	/*
	 * only save the bits if we've actually
	 * started using backing store
	 */
	if (pBackingStore->status != StatusVirtual)
	{
	    if (!pBackingStore->pBackingPixmap)
		miCreateBSPixmap (pWin, &oldExtents);
	    else
		miResizeBackingStore(pWin, 0, 0, TRUE);

	    if (pBackingStore->pBackingPixmap) {
		if (pBackingStore->x | pBackingStore->y)
		{
		    REGION_TRANSLATE( pScreen, pObscured,
						  -pBackingStore->x,
						  -pBackingStore->y);
		    x += pBackingStore->x;
		    y += pBackingStore->y;
		}
		(* pScreen->BackingStoreFuncs.SaveAreas)
		    (pBackingStore->pBackingPixmap, pObscured,
		     x - dx, y - dy, pWin);
	    }
	}
	REGION_TRANSLATE(pScreen, pObscured, x, y);
    }
    else
    {
	if (REGION_BROKEN (pScreen, pObscured))
	{
	    REGION_EMPTY( pScreen, &pBackingStore->SavedRegion);
	    miDestroyBSPixmap (pWin);
	    return;
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSRestoreAreas --
 *	Restore areas from backing-store that are no longer obscured.
 *	expects prgnExposed to contain a screen-relative area.
 *
 * Results:
 *	The region to generate exposure events on (which may be
 *	different from the region to paint).
 *
 * Side Effects:
 *	Areas are copied from pBackingPixmap to the screen. prgnExposed
 *	is altered to contain the region that could not be restored from
 *	backing-store.
 *
 * Notes:
 *	This is called before sending any exposure events to the client,
 *	and so might be called if the window has grown.  Changing the backing
 *	pixmap doesn't require revalidating the backingGC because the
 *	client's next output request will result in a call to ValidateGC,
 *	since the window clip region has changed, which will in turn call
 *	miValidateBackingStore.
 *-----------------------------------------------------------------------
 */
static RegionPtr
miBSRestoreAreas(pWin, prgnExposed)
    register WindowPtr pWin;
    RegionPtr prgnExposed;
{
    PixmapPtr pBackingPixmap;
    miBSWindowPtr pBackingStore;
    RegionPtr prgnSaved;
    RegionPtr prgnRestored;
    register ScreenPtr pScreen;
    RegionPtr exposures = prgnExposed;

    pScreen = pWin->drawable.pScreen;
    pBackingStore = (miBSWindowPtr)pWin->backStorage;
    pBackingPixmap = pBackingStore->pBackingPixmap;

    prgnSaved = &pBackingStore->SavedRegion;

    if (pBackingStore->status == StatusContents)
    {
	REGION_TRANSLATE(pScreen, prgnSaved, pWin->drawable.x,
			 pWin->drawable.y);

	prgnRestored = REGION_CREATE( pScreen, (BoxPtr)NULL, 1);
	REGION_INTERSECT( pScreen, prgnRestored, prgnExposed, prgnSaved);
	
	/*
	 * Since prgnExposed is no longer obscured, we no longer
	 * will have a valid copy of it in backing-store, but there is a valid
	 * copy of it on screen, so subtract the area we just restored from
	 * from the area to be exposed.
	 */

	if (REGION_NOTEMPTY( pScreen, prgnRestored))
	{
	    REGION_SUBTRACT( pScreen, prgnSaved, prgnSaved, prgnExposed);
	    REGION_SUBTRACT( pScreen, prgnExposed, prgnExposed, prgnRestored);

	    /*
	     * Do the actual restoration
	     */
	    (* pScreen->BackingStoreFuncs.RestoreAreas) (pBackingPixmap,
					  prgnRestored,
					  pWin->drawable.x + pBackingStore->x,
					  pWin->drawable.y + pBackingStore->y,
					  pWin);
	    /*
	     * if the saved region is completely empty, dispose of the
	     * backing pixmap, otherwise, retranslate the saved
	     * region to window relative
	     */

	    if (REGION_NOTEMPTY(pScreen, prgnSaved))
	    {
		REGION_TRANSLATE(pScreen, prgnSaved,
					     -pWin->drawable.x,
					     -pWin->drawable.y);
		miResizeBackingStore(pWin, 0, 0, TRUE);
	    }
	    else
		miDestroyBSPixmap (pWin);
	}
	else
	    REGION_TRANSLATE(pScreen, prgnSaved,
				-pWin->drawable.x, -pWin->drawable.y);
	REGION_DESTROY( pScreen, prgnRestored);

    }
    else if ((pBackingStore->status == StatusVirtual) ||
	     (pBackingStore->status == StatusVDirty))
    {
	REGION_TRANSLATE(pScreen, prgnSaved,
				     pWin->drawable.x, pWin->drawable.y);
	exposures = REGION_CREATE( pScreen, NullBox, 1);
	if (SameBackground (pBackingStore->backgroundState,
			    pBackingStore->background,
			    pWin->backgroundState,
 			    pWin->background))
	{
	    REGION_SUBTRACT( pScreen, exposures, prgnExposed, prgnSaved);
	}
	else
	{
	    miTileVirtualBS(pWin);

	    /* we need to expose all we have (virtually) retiled */
	    REGION_UNION( pScreen, exposures, prgnExposed, prgnSaved);
	}
	REGION_SUBTRACT( pScreen, prgnSaved, prgnSaved, prgnExposed);
	REGION_TRANSLATE(pScreen, prgnSaved,
				     -pWin->drawable.x, -pWin->drawable.y);
    }
    else if (pWin->viewable && !pBackingStore->viewable &&
	     pWin->backingStore != Always)
    {
	/*
	 * The window was just mapped and nothing has been saved in
	 * backing-store from the last time it was mapped. We want to capture
	 * any output to regions that are already obscured but there are no
	 * bits to snag off the screen, so we initialize things just as we did
	 * in miBSAllocate, above.
	 */
	BoxRec  box;
	
	prgnSaved = &pBackingStore->SavedRegion;

	box.x1 = pWin->drawable.x;
	box.x2 = box.x1 + (int) pWin->drawable.width;
	box.y1 = pWin->drawable.y;
	box.y2 = box.y1 + (int) pWin->drawable.height;
	
	REGION_INVERSE( pScreen, prgnSaved, &pWin->clipList,  &box);
	REGION_TRANSLATE( pScreen, prgnSaved,
				      -pWin->drawable.x,
				      -pWin->drawable.y);
#ifdef SHAPE
	if (wBoundingShape (pWin))
	    REGION_INTERSECT(pScreen, prgnSaved, prgnSaved,
			     wBoundingShape (pWin));
	if (wClipShape (pWin))
	    REGION_INTERSECT(pScreen, prgnSaved, prgnSaved,
			     wClipShape (pWin));
#endif
	miTileVirtualBS(pWin);

	exposures = REGION_CREATE( pScreen, &box, 1);
    }
    pBackingStore->viewable = (char)pWin->viewable;
    return exposures;
}


/*-
 *-----------------------------------------------------------------------
 * miBSTranslateBackingStore --
 *	Shift the backing-store in the given direction. Called when bit
 *	gravity is shifting things around. 
 *
 * Results:
 *	An occluded region of the window which should be sent exposure events.
 *	This region should be in absolute coordinates (i.e. include
 *	new window position).
 *
 * Side Effects:
 *	If the window changed size as well as position, the backing pixmap
 *	is resized. The contents of the backing pixmap are shifted
 *
 * Warning:
 *	Bob and I have rewritten this routine quite a few times, each
 *	time it gets a few more cases correct, and introducing some
 *	interesting bugs.  Naturally, I think the code is correct this
 *	time.
 *
 *	Let me try to explain what this routine is for:
 *
 *	It's called from SlideAndSizeWindow whenever a window
 *	with backing store is resized.  There are two separate
 *	possibilities:
 *
 *	a)  The window has ForgetGravity
 *
 *	    In this case, windx, windy will be 0 and oldClip will
 *	    be NULL.  This indicates that all of the window contents
 *	    currently saved offscreen should be discarded, and the
 *	    entire window exposed.  TranslateBackingStore, then, should
 *	    prepare a completely new backing store region based on the
 *	    new window clipList and return that region for exposure.
 *
 *	b)  The window has some other gravity
 *
 *	    In this case, windx, windy will be set to the distance
 *	    that the bits should move within the window.  oldClip
 *	    will be set to the old visible portion of the window.
 *	    TranslateBackingStore, then, should adjust the backing
 *	    store to accommodate the portion of the existing backing
 *	    store bits which coorespond to backing store bits which
 *	    will still be occluded in the new configuration.  oldx,oldy
 *	    are set to the old position of the window on the screen.
 *
 *	    Furthermore, in this case any contents of the screen which
 *	    are about to become occluded should be fetched from the screen
 *	    and placed in backing store.  This is to avoid the eventual
 *	    occlusion by the win gravity shifting the child window bits around
 *	    on top of this window, and potentially losing information
 *
 *	It's also called from SetShape, but I think (he says not
 *	really knowing for sure) that this code will even work
 *	in that case.
 *-----------------------------------------------------------------------
 */

static RegionPtr
miBSTranslateBackingStore(pWin, windx, windy, oldClip, oldx, oldy)
    WindowPtr 	  pWin;
    int     	  windx;	/* bit translation distance in window */
    int     	  windy;
    RegionPtr	  oldClip;  	/* Region being copied */
    int     	  oldx;		/* old window position */
    int     	  oldy;
{
    register miBSWindowPtr 	pBackingStore;
    register RegionPtr 	    	pSavedRegion;
    register RegionPtr 	    	newSaved, doomed;
    register ScreenPtr		pScreen;
    BoxRec			extents;
    int     	  scrdx;	/* bit translation distance on screen */
    int     	  scrdy;
    int		  dx;		/* distance window moved  on screen */
    int		  dy;

    pScreen = pWin->drawable.pScreen;
    pBackingStore = (miBSWindowPtr)(pWin->backStorage);
    if ((pBackingStore->status == StatusNoPixmap) ||
	(pBackingStore->status == StatusBadAlloc))
	return NullRegion;

    /*
     * Compute the new saved region
     */

    newSaved = REGION_CREATE( pScreen, NullBox, 1);
    extents.x1 = pWin->drawable.x;
    extents.x2 = pWin->drawable.x + (int) pWin->drawable.width;
    extents.y1 = pWin->drawable.y;
    extents.y2 = pWin->drawable.y + (int) pWin->drawable.height;
    REGION_INVERSE( pScreen, newSaved, &pWin->clipList, &extents);

    REGION_TRANSLATE( pScreen, newSaved,
			-pWin->drawable.x, -pWin->drawable.y);
#ifdef SHAPE
    if (wBoundingShape (pWin) || wClipShape (pWin)) {
	if (wBoundingShape (pWin))
	    REGION_INTERSECT( pScreen, newSaved, newSaved,
				wBoundingShape (pWin));
	if (wClipShape (pWin))
	    REGION_INTERSECT( pScreen, newSaved, newSaved, wClipShape (pWin));
    }
#endif
    
    pSavedRegion = &pBackingStore->SavedRegion;

    /* now find any visible areas we can save from the screen */
    /* and then translate newSaved to old local coordinates */
    if (oldClip)
    {
    	/* bit gravity makes things virtually too hard, punt */
    	if (((windx != 0) || (windy != 0)) &&
	    (pBackingStore->status != StatusContents))
	    miCreateBSPixmap(pWin, NullBox);
    
	/*
	 * The window is moving this far on the screen
	 */
    	dx = pWin->drawable.x - oldx;
    	dy = pWin->drawable.y - oldy;
	/*
	 * The bits will be moving on the screen by the
	 * amount the window is moving + the amount the
	 * bits are moving within the window
	 */
    	scrdx = windx + dx;
    	scrdy = windy + dy;
    
	/*
 	 * intersect at old bit position to discover the
	 * bits on the screen which can be put into the
	 * new backing store
 	 */
	REGION_TRANSLATE( pScreen, oldClip, windx - oldx, windy - oldy);
	doomed = REGION_CREATE( pScreen, NullBox, 1);
	REGION_INTERSECT( pScreen, doomed, oldClip, newSaved);
	REGION_TRANSLATE( pScreen, oldClip, oldx - windx, oldy - windy);

	/*
	 * Translate the old saved region to the position in the
	 * window where it will appear to be
	 */
	REGION_TRANSLATE( pScreen, pSavedRegion, windx, windy);

	/*
	 * Add the old saved region to the new saved region, so
	 * that calls to RestoreAreas will be able to fetch those
	 * bits back
	 */
	REGION_UNION( pScreen, newSaved, newSaved, pSavedRegion);

	/*
	 * Swap the new saved region into the window
	 */
	{
	    RegionRec	tmp;

	    tmp = *pSavedRegion;
	    *pSavedRegion = *newSaved;
	    *newSaved = tmp;
	}
	miResizeBackingStore (pWin, windx, windy, TRUE);

	/*
	 * Compute the newly enabled region
	 * of backing store.  This region will be
	 * set to background in the backing pixmap and
	 * sent as exposure events to the client.
	 */
	REGION_SUBTRACT( pScreen, newSaved, pSavedRegion, newSaved);

	/*
	 * Fetch bits which will be obscured from
	 * the screen
	 */
	if (REGION_NOTEMPTY( pScreen, doomed))
	{
	    /*
	     * Don't clear regions which have bits on the
	     * screen
	     */
	    REGION_SUBTRACT( pScreen, newSaved, newSaved, doomed);

	    /*
	     * Make the region to SaveDoomedAreas absolute, instead
	     * of window relative.
	     */
	    REGION_TRANSLATE( pScreen, doomed,
					  pWin->drawable.x, pWin->drawable.y);
	    (* pScreen->SaveDoomedAreas) (pWin, doomed, scrdx, scrdy);
	}
	
	REGION_DESTROY(pScreen, doomed);

    	/*
 	 * and clear whatever there is that's new
 	 */
    	if (REGION_NOTEMPTY( pScreen, newSaved))
    	{
	    miBSClearBackingRegion (pWin, newSaved);
	    /*
	     * Make the exposed region absolute
	     */
	    REGION_TRANSLATE(pScreen, newSaved,
				     	 pWin->drawable.x,
				     	 pWin->drawable.y);
    	}
    	else
    	{
	    REGION_DESTROY(pScreen, newSaved);
	    newSaved = NullRegion;
    	}
    }
    else
    {
	/*
	 * ForgetGravity: just reset backing store and
	 * expose the whole mess
	 */
	REGION_COPY( pScreen, pSavedRegion, newSaved);
	REGION_TRANSLATE( pScreen, newSaved,
				      pWin->drawable.x, pWin->drawable.y);

	miResizeBackingStore (pWin, 0, 0, FALSE);
	(void) miBSClearBackingStore (pWin, 0, 0, 0, 0, FALSE);
    }

    return newSaved;
}

/*
 * Inform the backing store layer that you are about to validate
 * a gc with a window, and that subsequent output to the window
 * is (or is not) guaranteed to be already clipped to the visible
 * regions of the window.
 */

static void
miBSDrawGuarantee (pWin, pGC, guarantee)
    WindowPtr	pWin;
    GCPtr	pGC;
    int		guarantee;
{
    miBSGCPtr 	pPriv;

    if (pWin->backStorage)
    {
	pPriv = (miBSGCPtr)pGC->devPrivates[miBSGCIndex].ptr;
	if (!pPriv)
	    (void) miBSCreateGCPrivate (pGC);
	pPriv = (miBSGCPtr)pGC->devPrivates[miBSGCIndex].ptr;
	if (pPriv)
	{
	    /*
	     * XXX KLUDGE ALERT
	     *
	     * when the GC is Cheap pPriv will point
	     * at some device's gc func structure.  guarantee
	     * will point at the ChangeGC entry of that struct
	     * and will never match a valid guarantee value.
	     */
	    switch (pPriv->guarantee)
	    {
	    case GuaranteeNothing:
	    case GuaranteeVisBack:
		pPriv->guarantee = guarantee;
		break;
	    }
	}
    }
}

#define noBackingCopy (GCGraphicsExposures|GCClipXOrigin|GCClipYOrigin| \
		       GCClipMask|GCSubwindowMode| \
		       GCTileStipXOrigin|GCTileStipYOrigin)

/*-
 *-----------------------------------------------------------------------
 * miBSValidateGC --
 *	Wrapper around output-library's ValidateGC routine
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 * Notes:
 *	The idea here is to perform several functions:
 *	    - All the output calls must be intercepted and routed to
 *	      backing-store as necessary.
 *	    - pGC in the window's devBackingStore must be set up with the
 *	      clip list appropriate for writing to pBackingPixmap (i.e.
 *	      the inverse of the window's clipList intersected with the
 *	      clientClip of the GC). Since the destination for this GC is
 *	      a pixmap, it is sufficient to set the clip list as its
 *	      clientClip.
 *-----------------------------------------------------------------------
 */

static void
miBSValidateGC (pGC, stateChanges, pDrawable)
    GCPtr   	  pGC;
    unsigned long stateChanges;
    DrawablePtr   pDrawable;
{
    GCPtr   	  	pBackingGC;
    miBSWindowPtr	pWindowPriv = NULL;
    miBSGCPtr		pPriv;
    WindowPtr		pWin;
    int			lift_functions;
    RegionPtr		backingCompositeClip = NULL;

    if (pDrawable->type != DRAWABLE_PIXMAP)
    {
        pWin = (WindowPtr) pDrawable;
	pWindowPriv = (miBSWindowPtr) pWin->backStorage;
	lift_functions = (pWindowPriv == (miBSWindowPtr) NULL);
    }
    else
    {
        pWin = (WindowPtr) NULL;
	lift_functions = TRUE;
    }

    pPriv = (miBSGCPtr)pGC->devPrivates[miBSGCIndex].ptr;

    FUNC_PROLOGUE (pGC, pPriv);

    (*pGC->funcs->ValidateGC) (pGC, stateChanges, pDrawable);

    /*
     * rewrap funcs and ops as Validate may have changed them
     */

    pPriv->wrapFuncs = pGC->funcs;
    pPriv->wrapOps = pGC->ops;

    if (!lift_functions && ((pPriv->guarantee == GuaranteeVisBack) ||
                            (pWindowPriv->status == StatusNoPixmap) ||
                            (pWindowPriv->status == StatusBadAlloc)))
        lift_functions = TRUE;

    /*
     * check to see if a new backingCompositeClip region must
     * be generated
     */

    if (!lift_functions && 
        ((pDrawable->serialNumber != pPriv->serialNumber) ||
	 (stateChanges&(GCClipXOrigin|GCClipYOrigin|GCClipMask|GCSubwindowMode))))
    {
	if (REGION_NOTEMPTY(pGC->pScreen, &pWindowPriv->SavedRegion))
 	{
	    backingCompositeClip = REGION_CREATE(pGC->pScreen, NULL, 1);
	    if ((pGC->clientClipType == CT_NONE) || 
		(pGC->clientClipType == CT_PIXMAP))
	    {
		REGION_COPY(pGC->pScreen, backingCompositeClip,
					     &pWindowPriv->SavedRegion); 
	    }
	    else
	    {
		/*
		 * Make a new copy of the client clip, translated to
		 * its proper origin.
		 */

		REGION_COPY(pGC->pScreen, backingCompositeClip,
				pGC->clientClip);
		REGION_TRANSLATE(pGC->pScreen, backingCompositeClip,
						  pGC->clipOrg.x,
						  pGC->clipOrg.y);
		REGION_INTERSECT(pGC->pScreen, backingCompositeClip,
					backingCompositeClip,
					&pWindowPriv->SavedRegion);
	    }
	    if (pGC->subWindowMode == IncludeInferiors)
 	    {
		RegionPtr translatedClip;

		/* XXX
		 * any output in IncludeInferiors mode will not
		 * be redirected to Inferiors backing store.  This
		 * can be fixed only at great cost to the shadow routines.
		 */
		translatedClip = NotClippedByChildren (pWin);
		REGION_TRANSLATE(pGC->pScreen, translatedClip,
						  -pDrawable->x,
						  -pDrawable->y);
		REGION_SUBTRACT(pGC->pScreen, backingCompositeClip,
				backingCompositeClip, translatedClip);
		REGION_DESTROY(pGC->pScreen, translatedClip);
	    }
	    if (!REGION_NOTEMPTY(pGC->pScreen, backingCompositeClip))
		lift_functions = TRUE;
	}
 	else
 	{
	    lift_functions = TRUE;
	}

	/* Reset the status when drawing to an unoccluded window so that
	 * future SaveAreas will actually copy bits from the screen.  Note that
	 * output to root window in IncludeInferiors mode will not cause this
	 * to change.  This causes all transient graphics by the window
	 * manager to the root window to not enable backing store.
	 */
	if (lift_functions && (pWindowPriv->status == StatusVirtual) &&
	    (pWin->parent || pGC->subWindowMode != IncludeInferiors))
	    pWindowPriv->status = StatusVDirty;
    }

    /*
     * if no backing store has been allocated, and it's needed,
     * create it now.
     */

    if (!lift_functions && !pWindowPriv->pBackingPixmap)
    {
	miCreateBSPixmap (pWin, NullBox);
	if (!pWindowPriv->pBackingPixmap)
	    lift_functions = TRUE;
    }
    
    /*
     * create the backing GC if needed, lift functions
     * if the creation fails
     */

    if (!lift_functions && !pPriv->pBackingGC)
    {
	int status;
	XID noexpose = xFalse;

	/* We never want ops with the backingGC to generate GraphicsExpose */
	pBackingGC = CreateGC ((DrawablePtr)pWindowPriv->pBackingPixmap,
			       GCGraphicsExposures, &noexpose, &status);
	if (status != Success)
	    lift_functions = TRUE;
	else
	    pPriv->pBackingGC = pBackingGC;
    }

    pBackingGC = pPriv->pBackingGC;

    pPriv->stateChanges |= stateChanges;

    if (lift_functions)
    {
	if (backingCompositeClip)
	    REGION_DESTROY( pGC->pScreen, backingCompositeClip);

	/* unwrap the GC again */
	miBSDestroyGCPrivate (pGC);

	return;
    }

    /*
     * the rest of this function gets the pBackingGC
     * into shape for possible draws
     */

    pPriv->stateChanges &= ~noBackingCopy;
    if (pPriv->stateChanges)
	CopyGC(pGC, pBackingGC, pPriv->stateChanges);
    if ((pGC->patOrg.x - pWindowPriv->x) != pBackingGC->patOrg.x ||
	(pGC->patOrg.y - pWindowPriv->y) != pBackingGC->patOrg.y)
    {
	XID vals[2];
	vals[0] = pGC->patOrg.x - pWindowPriv->x;
	vals[1] = pGC->patOrg.y - pWindowPriv->y;
	DoChangeGC(pBackingGC, GCTileStipXOrigin|GCTileStipYOrigin, vals, 0);
    }
    pPriv->stateChanges = 0;

    if (backingCompositeClip)
    {
	XID vals[2];

	if (pGC->clientClipType == CT_PIXMAP)
	{
	    (*pBackingGC->funcs->CopyClip)(pBackingGC, pGC);
	    REGION_TRANSLATE(pGC->pScreen, backingCompositeClip,
					-pGC->clipOrg.x, -pGC->clipOrg.y);
	    vals[0] = pGC->clipOrg.x - pWindowPriv->x;
	    vals[1] = pGC->clipOrg.y - pWindowPriv->y;
	    DoChangeGC(pBackingGC, GCClipXOrigin|GCClipYOrigin, vals, 0);
	    (* pGC->pScreen->BackingStoreFuncs.SetClipmaskRgn)
		(pBackingGC, backingCompositeClip);
	    REGION_DESTROY( pGC->pScreen, backingCompositeClip);
	}
	else
	{
	    vals[0] = -pWindowPriv->x;
	    vals[1] = -pWindowPriv->y;
	    DoChangeGC(pBackingGC, GCClipXOrigin|GCClipYOrigin, vals, 0);
	    (*pBackingGC->funcs->ChangeClip) (pBackingGC, CT_REGION, backingCompositeClip, 0);
	}
	pPriv->serialNumber = pDrawable->serialNumber;
    }
    
    if (pWindowPriv->pBackingPixmap->drawable.serialNumber
    	!= pBackingGC->serialNumber)
    {
	ValidateGC((DrawablePtr)pWindowPriv->pBackingPixmap, pBackingGC);
    }

    if (pBackingGC->clientClip == 0)
    	ErrorF ("backing store clip list nil");

    FUNC_EPILOGUE (pGC, pPriv);
}

static void
miBSChangeGC (pGC, mask)
    GCPtr   pGC;
    unsigned long   mask;
{
    miBSGCPtr	pPriv = (miBSGCPtr) (pGC)->devPrivates[miBSGCIndex].ptr;

    FUNC_PROLOGUE (pGC, pPriv);

    (*pGC->funcs->ChangeGC) (pGC, mask);

    FUNC_EPILOGUE (pGC, pPriv);
}

static void
miBSCopyGC (pGCSrc, mask, pGCDst)
    GCPtr   pGCSrc, pGCDst;
    unsigned long   mask;
{
    miBSGCPtr	pPriv = (miBSGCPtr) (pGCDst)->devPrivates[miBSGCIndex].ptr;

    FUNC_PROLOGUE (pGCDst, pPriv);

    (*pGCDst->funcs->CopyGC) (pGCSrc, mask, pGCDst);

    FUNC_EPILOGUE (pGCDst, pPriv);
}

static void
miBSDestroyGC (pGC)
    GCPtr   pGC;
{
    miBSGCPtr	pPriv = (miBSGCPtr) (pGC)->devPrivates[miBSGCIndex].ptr;

    FUNC_PROLOGUE (pGC, pPriv);

    if (pPriv->pBackingGC)
	FreeGC(pPriv->pBackingGC, (GContext)0);

    (*pGC->funcs->DestroyGC) (pGC);

    FUNC_EPILOGUE (pGC, pPriv);

    xfree(pPriv);
}

static void
miBSChangeClip(pGC, type, pvalue, nrects)
    GCPtr	pGC;
    int		type;
    pointer	pvalue;
    int		nrects;
{
    miBSGCPtr	pPriv = (miBSGCPtr) (pGC)->devPrivates[miBSGCIndex].ptr;

    FUNC_PROLOGUE (pGC, pPriv);

    (* pGC->funcs->ChangeClip)(pGC, type, pvalue, nrects);

    FUNC_EPILOGUE (pGC, pPriv);
}

static void
miBSCopyClip(pgcDst, pgcSrc)
    GCPtr pgcDst, pgcSrc;
{
    miBSGCPtr	pPriv = (miBSGCPtr) (pgcDst)->devPrivates[miBSGCIndex].ptr;

    FUNC_PROLOGUE (pgcDst, pPriv);

    (* pgcDst->funcs->CopyClip)(pgcDst, pgcSrc);

    FUNC_EPILOGUE (pgcDst, pPriv);
}

static void
miBSDestroyClip(pGC)
    GCPtr	pGC;
{
    miBSGCPtr	pPriv = (miBSGCPtr) (pGC)->devPrivates[miBSGCIndex].ptr;

    FUNC_PROLOGUE (pGC, pPriv);

    (* pGC->funcs->DestroyClip)(pGC);

    FUNC_EPILOGUE (pGC, pPriv);
}

static void
miDestroyBSPixmap (pWin)
    WindowPtr	pWin;
{
    miBSWindowPtr	pBackingStore;
    ScreenPtr		pScreen;
    
    pScreen = pWin->drawable.pScreen;
    pBackingStore = (miBSWindowPtr) pWin->backStorage;
    if (pBackingStore->pBackingPixmap)
	(* pScreen->DestroyPixmap)(pBackingStore->pBackingPixmap);
    pBackingStore->pBackingPixmap = NullPixmap;
    pBackingStore->x = 0;
    pBackingStore->y = 0;
    if (pBackingStore->backgroundState == BackgroundPixmap)
	(* pScreen->DestroyPixmap)(pBackingStore->background.pixmap);
    pBackingStore->backgroundState = None;
    pBackingStore->status = StatusNoPixmap;
    pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;
}

static void
miTileVirtualBS (pWin)
    WindowPtr	pWin;
{
    miBSWindowPtr	pBackingStore;

    pBackingStore = (miBSWindowPtr) pWin->backStorage;
    if (pBackingStore->backgroundState == BackgroundPixmap)
 	(* pWin->drawable.pScreen->DestroyPixmap)
	    (pBackingStore->background.pixmap);
    pBackingStore->backgroundState = pWin->backgroundState;
    pBackingStore->background = pWin->background;
    if (pBackingStore->backgroundState == BackgroundPixmap)
	pBackingStore->background.pixmap->refcnt++;

    if (pBackingStore->status != StatusVDirty)
	pBackingStore->status = StatusVirtual;

    /*
     * punt parent relative tiles and do it now
     */
    if (pBackingStore->backgroundState == ParentRelative)
	miCreateBSPixmap (pWin, NullBox);
}

#ifdef DEBUG
static int BSAllocationsFailed = 0;
#define FAILEDSIZE	32
static struct { int w, h; } failedRecord[FAILEDSIZE];
static int failedIndex;
#endif

static void
miCreateBSPixmap (pWin, pExtents)
    WindowPtr	pWin;
    BoxPtr	pExtents;
{
    miBSWindowPtr	pBackingStore;
    ScreenPtr		pScreen;
    PixUnion		background;
    char		backgroundState = 0;
    BoxPtr		extents;
    Bool		backSet;

    pScreen = pWin->drawable.pScreen;
    pBackingStore = (miBSWindowPtr) pWin->backStorage;
    if (pBackingStore->status == StatusBadAlloc)
	return;
    backSet = ((pBackingStore->status == StatusVirtual) ||
	       (pBackingStore->status == StatusVDirty));

    extents = REGION_EXTENTS( pScreen, &pBackingStore->SavedRegion);

    if (!pBackingStore->pBackingPixmap &&
	extents->x2 != extents->x1 &&
	extents->y2 != extents->y1)
    {
	/* the policy here could be more sophisticated */
	pBackingStore->x = extents->x1;
	pBackingStore->y = extents->y1;
	pBackingStore->pBackingPixmap =
    	    (PixmapPtr)(* pScreen->CreatePixmap)
			   (pScreen,
			    extents->x2 - extents->x1,
			    extents->y2 - extents->y1,
			    pWin->drawable.depth);
    }
    if (!pBackingStore->pBackingPixmap)
    {
#ifdef DEBUG
	BSAllocationsFailed++;
	/*
	 * record failed allocations
	 */
	failedRecord[failedIndex].w = pWin->drawable.width;
	failedRecord[failedIndex].h = pWin->drawable.height;
	failedIndex++;
	if (failedIndex == FAILEDSIZE)
		failedIndex = 0;
#endif
#ifdef BSEAGER
	pBackingStore->status = StatusNoPixmap;
#else
	pBackingStore->status = StatusBadAlloc;
#endif
	return;
    }

    pBackingStore->status = StatusContents;

    if (backSet)
    {
	backgroundState = pWin->backgroundState;
	background = pWin->background;
    
	pWin->backgroundState = pBackingStore->backgroundState;
	pWin->background = pBackingStore->background;
	if (pWin->backgroundState == BackgroundPixmap)
	    pWin->background.pixmap->refcnt++;
    }

    if (!pExtents)
	pExtents = extents;

    if (pExtents->y1 != pExtents->y2)
    {
	RegionPtr exposed;

	exposed = miBSClearBackingStore(pWin,
			      pExtents->x1, pExtents->y1,
			      pExtents->x2 - pExtents->x1,
			      pExtents->y2 - pExtents->y1,
			      !backSet);
	if (exposed)
	{
	    miSendExposures(pWin, exposed, pWin->drawable.x, pWin->drawable.y);
	    REGION_DESTROY( pScreen, exposed);
	}
    }

    if (backSet)
    {
	if (pWin->backgroundState == BackgroundPixmap)
	    (* pScreen->DestroyPixmap) (pWin->background.pixmap);
	pWin->backgroundState = backgroundState;
	pWin->background = background;
	if (pBackingStore->backgroundState == BackgroundPixmap)
	    (* pScreen->DestroyPixmap) (pBackingStore->background.pixmap);
	pBackingStore->backgroundState = None;
    }
}

/*-
 *-----------------------------------------------------------------------
 * miBSExposeCopy --
 *	Handle the restoration of areas exposed by graphics operations.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	prgnExposed has the areas exposed from backing-store removed
 *	from it.
 *
 *-----------------------------------------------------------------------
 */
static void
miBSExposeCopy (pSrc, pDst, pGC, prgnExposed, srcx, srcy, dstx, dsty, plane)
    WindowPtr	  	pSrc;
    DrawablePtr	  	pDst;
    GCPtr   	  	pGC;
    RegionPtr	  	prgnExposed;
    int	    	  	srcx, srcy;
    int	    	  	dstx, dsty;
    unsigned long 	plane;
{
    RegionRec	  	tempRgn;
    miBSWindowPtr	pBackingStore;
    CopyPlaneProcPtr 	copyProc;
    GCPtr		pScratchGC;
    register BoxPtr	pBox;
    register int  	i;
    register int  	dx, dy;
    BITS32		gcMask;

    if (!REGION_NOTEMPTY(pGC->pScreen, prgnExposed))
	return;
    pBackingStore = (miBSWindowPtr)pSrc->backStorage;
    
    if ((pBackingStore->status == StatusNoPixmap) ||
	(pBackingStore->status == StatusBadAlloc))
    	return;

    REGION_NULL( pGC->pScreen, &tempRgn);
    REGION_INTERSECT( pGC->pScreen, &tempRgn, prgnExposed,
				 &pBackingStore->SavedRegion);
    REGION_SUBTRACT( pGC->pScreen, prgnExposed, prgnExposed, &tempRgn);

    if (plane != 0) {
	copyProc = pGC->ops->CopyPlane;
    } else {
	copyProc = (CopyPlaneProcPtr)pGC->ops->CopyArea;
    }
    
    dx = dstx - srcx;
    dy = dsty - srcy;
    
    switch (pBackingStore->status) {
    case StatusVirtual:
    case StatusVDirty:
	pScratchGC = GetScratchGC (pDst->depth, pDst->pScreen);
	if (pScratchGC)
	{
	    gcMask = 0;
	    if (pGC->alu != pScratchGC->alu)
	    	gcMask = GCFunction;
	    if (pGC->planemask != pScratchGC->planemask)
	    	gcMask |= GCPlaneMask;
	    if (gcMask)
	    	CopyGC (pGC, pScratchGC, gcMask);
	    miBSFillVirtualBits (pDst, pScratchGC, &tempRgn, dx, dy,
				 (int) pBackingStore->backgroundState,
				 pBackingStore->background,
				 ~0L);
	    FreeScratchGC (pScratchGC);
	}
	break;
    case StatusContents:
	for (i = REGION_NUM_RECTS(&tempRgn), pBox = REGION_RECTS(&tempRgn);
	     --i >= 0;
	     pBox++)
	{
	    (* copyProc) (&(pBackingStore->pBackingPixmap->drawable), pDst, pGC,
			  pBox->x1 - pBackingStore->x,
			  pBox->y1 - pBackingStore->y,
			  pBox->x2 - pBox->x1, pBox->y2 - pBox->y1,
			  pBox->x1 + dx, pBox->y1 + dy, plane);
	}
	break;
    }
    REGION_UNINIT( pGC->pScreen, &tempRgn);
}
