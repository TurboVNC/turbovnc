/*
 * $XFree86: xc/programs/Xserver/fb/fboverlay.c,v 1.7 2003/11/10 18:21:47 tsi Exp $
 *
 * Copyright © 2000 SuSE, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of SuSE not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  SuSE makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * SuSE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL SuSE
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, SuSE, Inc.
 */

#include "fb.h"
#include "fboverlay.h"

int	fbOverlayGeneration;
int	fbOverlayScreenPrivateIndex = -1;

/*
 * Replace this if you want something supporting
 * multiple overlays with the same depth
 */
Bool
fbOverlayCreateWindow(WindowPtr pWin)
{
    FbOverlayScrPrivPtr	pScrPriv = fbOverlayGetScrPriv(pWin->drawable.pScreen);
    int			i;
    PixmapPtr		pPixmap;
    
    if (pWin->drawable.class != InputOutput)
	return TRUE;

#ifdef FB_SCREEN_PRIVATE
    if (pWin->drawable.bitsPerPixel == 32)
	pWin->drawable.bitsPerPixel = fbGetScreenPrivate(pWin->drawable.pScreen)->win32bpp;
#endif

    for (i = 0; i < pScrPriv->nlayers; i++)
    {
	pPixmap = pScrPriv->layer[i].u.run.pixmap;
	if (pWin->drawable.depth == pPixmap->drawable.depth)
	{
	    pWin->devPrivates[fbWinPrivateIndex].ptr = (pointer) pPixmap;
	    /*
	     * Make sure layer keys are written correctly by
	     * having non-root layers set to full while the
	     * root layer is set to empty.  This will cause
	     * all of the layers to get painted when the root
	     * is mapped
	     */
	    if (!pWin->parent)
	    {
		REGION_EMPTY (pWin->drawable.pScreen,
			      &pScrPriv->layer[i].u.run.region);
	    }
	    return TRUE;
	}
    }
    return FALSE;
}

Bool
fbOverlayCloseScreen (int iScreen, ScreenPtr pScreen)
{
    FbOverlayScrPrivPtr	pScrPriv = fbOverlayGetScrPriv(pScreen);
    int			i;

    for (i = 0; i < pScrPriv->nlayers; i++)
    {
	(*pScreen->DestroyPixmap)(pScrPriv->layer[i].u.run.pixmap);
	REGION_UNINIT (pScreen, &pScrPriv->layer[i].u.run.region);
    }
    return TRUE;
}

/*
 * Return layer containing this window
 */
int
fbOverlayWindowLayer(WindowPtr pWin)
{
    FbOverlayScrPrivPtr pScrPriv = fbOverlayGetScrPriv(pWin->drawable.pScreen);
    int                 i;

    for (i = 0; i < pScrPriv->nlayers; i++)
	if (pWin->devPrivates[fbWinPrivateIndex].ptr ==
	    (pointer) pScrPriv->layer[i].u.run.pixmap)
	    return i;
    return 0;
}

Bool
fbOverlayCreateScreenResources(ScreenPtr pScreen)
{
    int			i;
    FbOverlayScrPrivPtr	pScrPriv = fbOverlayGetScrPriv(pScreen);
    PixmapPtr		pPixmap;
    pointer		pbits;
    int			width;
    int			depth;
    BoxRec		box;
    
    if (!miCreateScreenResources(pScreen))
	return FALSE;

    box.x1 = 0;
    box.y1 = 0;
    box.x2 = pScreen->width;
    box.y2 = pScreen->height;
    for (i = 0; i < pScrPriv->nlayers; i++)
    {
	pbits = pScrPriv->layer[i].u.init.pbits;
	width = pScrPriv->layer[i].u.init.width;
	depth = pScrPriv->layer[i].u.init.depth;
	pPixmap = (*pScreen->CreatePixmap)(pScreen, 0, 0, depth);
	if (!pPixmap)
	    return FALSE;
	if (!(*pScreen->ModifyPixmapHeader)(pPixmap, pScreen->width,
					    pScreen->height, depth,
					    BitsPerPixel(depth),
					    PixmapBytePad(width, depth),
					    pbits))
	    return FALSE;
	pScrPriv->layer[i].u.run.pixmap = pPixmap;
	REGION_INIT(pScreen, &pScrPriv->layer[i].u.run.region, &box, 0);
    }
    pScreen->devPrivate = pScrPriv->layer[0].u.run.pixmap;
    return TRUE;
}

void
fbOverlayPaintKey (DrawablePtr	pDrawable,
		   RegionPtr	pRegion,
		   CARD32	pixel,
		   int		layer)
{
    fbFillRegionSolid (pDrawable, pRegion, 0, 
		       fbReplicatePixel (pixel, pDrawable->bitsPerPixel));
}

/*
 * Track visible region for each layer
 */
void
fbOverlayUpdateLayerRegion (ScreenPtr	pScreen,
			    int		layer,
			    RegionPtr	prgn)
{
    FbOverlayScrPrivPtr pScrPriv = fbOverlayGetScrPriv(pScreen);
    int			i;
    RegionRec		rgnNew;
    
    if (!prgn || !REGION_NOTEMPTY(pScreen, prgn))
	return;
    for (i = 0; i < pScrPriv->nlayers; i++)
    {
	if (i == layer)
	{
	    /* add new piece to this fb */
	    REGION_UNION (pScreen,
			  &pScrPriv->layer[i].u.run.region,
			  &pScrPriv->layer[i].u.run.region,
			  prgn);
	}
	else if (REGION_NOTEMPTY (pScreen, 
				  &pScrPriv->layer[i].u.run.region))
	{
	    /* paint new piece with chroma key */
	    REGION_NULL (pScreen, &rgnNew);
	    REGION_INTERSECT (pScreen,
			      &rgnNew, 
			      prgn, 
			      &pScrPriv->layer[i].u.run.region);
	    (*pScrPriv->PaintKey) (&pScrPriv->layer[i].u.run.pixmap->drawable,
				   &rgnNew,
				   pScrPriv->layer[i].key,
				   i);
	    REGION_UNINIT(pScreen, &rgnNew);
	    /* remove piece from other fbs */
	    REGION_SUBTRACT (pScreen,
			     &pScrPriv->layer[i].u.run.region,
			     &pScrPriv->layer[i].u.run.region,
			     prgn);
	}
    }
}

/*
 * Copy only areas in each layer containing real bits
 */
void
fbOverlayCopyWindow(WindowPtr	pWin,
		    DDXPointRec	ptOldOrg,
		    RegionPtr	prgnSrc)
{
    ScreenPtr		pScreen = pWin->drawable.pScreen;
    FbOverlayScrPrivPtr	pScrPriv = fbOverlayGetScrPriv(pWin->drawable.pScreen);
    RegionRec		rgnDst;
    int			dx, dy;
    int			i;
    RegionRec		layerRgn[FB_OVERLAY_MAX];
    PixmapPtr		pPixmap;

    dx = ptOldOrg.x - pWin->drawable.x;
    dy = ptOldOrg.y - pWin->drawable.y;

    /*
     * Clip to existing bits
     */
    REGION_TRANSLATE(pScreen, prgnSrc, -dx, -dy);
    REGION_NULL (pScreen, &rgnDst);
    REGION_INTERSECT(pScreen, &rgnDst, &pWin->borderClip, prgnSrc);
    REGION_TRANSLATE(pScreen, &rgnDst, dx, dy);
    /*
     * Compute the portion of each fb affected by this copy
     */
    for (i = 0; i < pScrPriv->nlayers; i++)
    {
	REGION_NULL (pScreen, &layerRgn[i]);
	REGION_INTERSECT(pScreen, &layerRgn[i], &rgnDst,
			 &pScrPriv->layer[i].u.run.region);
	if (REGION_NOTEMPTY (pScreen, &layerRgn[i]))
	{
	    REGION_TRANSLATE(pScreen, &layerRgn[i], -dx, -dy);
	    pPixmap = pScrPriv->layer[i].u.run.pixmap;
	    fbCopyRegion (&pPixmap->drawable, &pPixmap->drawable,
			  0,
			  &layerRgn[i], dx, dy, pScrPriv->CopyWindow, 0,
			  (void *)(long) i);
	}
    }
    /*
     * Update regions
     */
    for (i = 0; i < pScrPriv->nlayers; i++)
    {
	if (REGION_NOTEMPTY (pScreen, &layerRgn[i]))
	    fbOverlayUpdateLayerRegion (pScreen, i, &layerRgn[i]);

	REGION_UNINIT(pScreen, &layerRgn[i]);
    }
    REGION_UNINIT(pScreen, &rgnDst);
}   

void
fbOverlayWindowExposures (WindowPtr	pWin,
			  RegionPtr	prgn,
			  RegionPtr	other_exposed)
{
    fbOverlayUpdateLayerRegion (pWin->drawable.pScreen,
				fbOverlayWindowLayer (pWin),
				prgn);
    miWindowExposures(pWin, prgn, other_exposed);
}

void
fbOverlayPaintWindow(WindowPtr pWin, RegionPtr pRegion, int what)
{
    if (what == PW_BORDER)
	fbOverlayUpdateLayerRegion (pWin->drawable.pScreen,
				    fbOverlayWindowLayer (pWin),
				    pRegion);
    fbPaintWindow (pWin, pRegion, what);
}

Bool
fbOverlaySetupScreen(ScreenPtr	pScreen,
		     pointer	pbits1,
		     pointer	pbits2,
		     int	xsize,
		     int	ysize,
		     int	dpix,
		     int	dpiy,
		     int	width1,
		     int	width2,
		     int	bpp1,
		     int	bpp2)
{
    return fbSetupScreen (pScreen,
			  pbits1,
			  xsize,
			  ysize,
			  dpix,
			  dpiy,
			  width1,
			  bpp1);
}

static Bool
fb24_32OverlayCreateScreenResources(ScreenPtr pScreen)
{
    FbOverlayScrPrivPtr	pScrPriv = fbOverlayGetScrPriv(pScreen);
    int pitch;
    Bool retval;
    int i;

    if((retval = fbOverlayCreateScreenResources(pScreen))) {
	for (i = 0; i < pScrPriv->nlayers; i++)
	{
	    /* fix the screen pixmap */
	    PixmapPtr pPix = (PixmapPtr) pScrPriv->layer[i].u.run.pixmap;
	    if (pPix->drawable.bitsPerPixel == 32) {
		pPix->drawable.bitsPerPixel = 24;
		pitch = BitmapBytePad(pPix->drawable.width * 24);
		pPix->devKind = pitch;
	    }
	}
    }

    return retval;
}

Bool
fbOverlayFinishScreenInit(ScreenPtr	pScreen,
			  pointer	pbits1,
			  pointer	pbits2,
			  int		xsize,
			  int		ysize,
			  int		dpix,
			  int		dpiy,
			  int		width1,
			  int		width2,
			  int		bpp1,
			  int		bpp2,
			  int		depth1,
			  int		depth2)
{
    VisualPtr	visuals;
    DepthPtr	depths;
    int		nvisuals;
    int		ndepths;
    int		bpp = 0, imagebpp = 32;
    VisualID	defaultVisual;
    FbOverlayScrPrivPtr	pScrPriv;

    if (fbOverlayGeneration != serverGeneration)
    {
	fbOverlayScreenPrivateIndex = AllocateScreenPrivateIndex ();
	fbOverlayGeneration = serverGeneration;
    }

    pScrPriv = xalloc (sizeof (FbOverlayScrPrivRec));
    if (!pScrPriv)
	return FALSE;
 
#ifdef FB_24_32BIT
    if (bpp1 == 32 || bpp2 == 32)
	bpp = 32;
    else if (bpp1 == 24 || bpp2 == 24)
	bpp = 24;

    if (bpp == 24)
    {
	int	f;
	
	imagebpp = 32;
	/*
	 * Check to see if we're advertising a 24bpp image format,
	 * in which case windows will use it in preference to a 32 bit
	 * format.
	 */
	for (f = 0; f < screenInfo.numPixmapFormats; f++)
	{
	    if (screenInfo.formats[f].bitsPerPixel == 24)
	    {
		imagebpp = 24;
		break;
	    }
	}	    
    }
#endif
#ifdef FB_SCREEN_PRIVATE
    if (imagebpp == 32)
    {
	fbGetScreenPrivate(pScreen)->win32bpp = bpp;
	fbGetScreenPrivate(pScreen)->pix32bpp = bpp;
    }
    else
    {
	fbGetScreenPrivate(pScreen)->win32bpp = 32;
	fbGetScreenPrivate(pScreen)->pix32bpp = 32;
    }
#endif
   
    if (!fbInitVisuals (&visuals, &depths, &nvisuals, &ndepths, &depth1,
			&defaultVisual, ((unsigned long)1<<(bpp1-1)) |
			((unsigned long)1<<(bpp2-1)), 8))
	return FALSE;
    if (! miScreenInit(pScreen, 0, xsize, ysize, dpix, dpiy, 0,
			depth1, ndepths, depths,
			defaultVisual, nvisuals, visuals
#ifdef FB_OLD_SCREEN
		       , (miBSFuncPtr) 0
#endif
		       ))
	return FALSE;
    /* MI thinks there's no frame buffer */
#ifdef MITSHM
    ShmRegisterFbFuncs(pScreen);
#endif
    pScreen->minInstalledCmaps = 1;
    pScreen->maxInstalledCmaps = 2;
    
    pScrPriv->nlayers = 2;
    pScrPriv->PaintKey = fbOverlayPaintKey;
    pScrPriv->CopyWindow = fbCopyWindowProc;
    pScrPriv->layer[0].u.init.pbits = pbits1;
    pScrPriv->layer[0].u.init.width = width1;
    pScrPriv->layer[0].u.init.depth = depth1;

    pScrPriv->layer[1].u.init.pbits = pbits2;
    pScrPriv->layer[1].u.init.width = width2;
    pScrPriv->layer[1].u.init.depth = depth2;
    
    pScreen->devPrivates[fbOverlayScreenPrivateIndex].ptr = (pointer) pScrPriv;
    
    /* overwrite miCloseScreen with our own */
    pScreen->CloseScreen = fbOverlayCloseScreen;
    pScreen->CreateScreenResources = fbOverlayCreateScreenResources;
    pScreen->CreateWindow = fbOverlayCreateWindow;
    pScreen->WindowExposures = fbOverlayWindowExposures;
    pScreen->CopyWindow = fbOverlayCopyWindow;
    pScreen->PaintWindowBorder = fbOverlayPaintWindow;
#ifdef FB_24_32BIT
    if (bpp == 24 && imagebpp == 32)
    {
	pScreen->ModifyPixmapHeader = fb24_32ModifyPixmapHeader;
  	pScreen->CreateScreenResources = fb24_32OverlayCreateScreenResources;
    }
#endif

    return TRUE;
}
