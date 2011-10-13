/*
 * misprite.c
 *
 * machine independent software sprite routines
 */

/* $Xorg: misprite.c,v 1.4 2001/02/09 02:05:22 xorgcvs Exp $ */

/*

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
*/
/* $XFree86: xc/programs/Xserver/mi/misprite.c,v 3.10tsi Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

# include   <X11/X.h>
# include   <X11/Xproto.h>
# include   "misc.h"
# include   "pixmapstr.h"
# include   "input.h"
# include   "mi.h"
# include   "cursorstr.h"
# include   <X11/fonts/font.h>
# include   "scrnintstr.h"
# include   "colormapst.h"
# include   "windowstr.h"
# include   "gcstruct.h"
# include   "mipointer.h"
# include   "mispritest.h"
# include   "dixfontstr.h"
# include   <X11/fonts/fontstruct.h>

#ifdef RENDER
# include   "mipict.h"
#endif
# include   "damage.h"

#define SPRITE_DEBUG_ENABLE 0
#if SPRITE_DEBUG_ENABLE
#define SPRITE_DEBUG(x)	ErrorF x
#else
#define SPRITE_DEBUG(x)
#endif

/*
 * screen wrappers
 */

static int  miSpriteScreenIndex;
static unsigned long miSpriteGeneration = 0;

static Bool	    miSpriteCloseScreen(int i, ScreenPtr pScreen);
static void	    miSpriteGetImage(DrawablePtr pDrawable, int sx, int sy,
				     int w, int h, unsigned int format,
				     unsigned long planemask, char *pdstLine);
static void	    miSpriteGetSpans(DrawablePtr pDrawable, int wMax,
				     DDXPointPtr ppt, int *pwidth, int nspans,
				     char *pdstStart);
static void	    miSpriteSourceValidate(DrawablePtr pDrawable, int x, int y,
					   int width, int height);
static void	    miSpriteCopyWindow (WindowPtr pWindow,
					DDXPointRec ptOldOrg,
					RegionPtr prgnSrc);
static void	    miSpriteBlockHandler(int i, pointer blockData,
					 pointer pTimeout,
					 pointer pReadMask);
static void	    miSpriteInstallColormap(ColormapPtr pMap);
static void	    miSpriteStoreColors(ColormapPtr pMap, int ndef,
					xColorItem *pdef);

static void	    miSpriteSaveDoomedAreas(WindowPtr pWin,
					    RegionPtr pObscured, int dx,
					    int dy);
static void	    miSpriteComputeSaved(ScreenPtr pScreen);

#define SCREEN_PROLOGUE(pScreen, field)\
  ((pScreen)->field = \
   ((miSpriteScreenPtr) (pScreen)->devPrivates[miSpriteScreenIndex].ptr)->field)

#define SCREEN_EPILOGUE(pScreen, field, wrapper)\
    ((pScreen)->field = wrapper)

/*
 * pointer-sprite method table
 */

static Bool miSpriteRealizeCursor(ScreenPtr pScreen, CursorPtr pCursor);
static Bool miSpriteUnrealizeCursor(ScreenPtr pScreen, CursorPtr pCursor);
static void miSpriteSetCursor(ScreenPtr pScreen, CursorPtr pCursor,
			      int x, int y);
static void miSpriteMoveCursor(ScreenPtr pScreen, int x, int y);

miPointerSpriteFuncRec miSpritePointerFuncs = {
    miSpriteRealizeCursor,
    miSpriteUnrealizeCursor,
    miSpriteSetCursor,
    miSpriteMoveCursor,
};

/*
 * other misc functions
 */

static void miSpriteRemoveCursor(ScreenPtr pScreen);
static void miSpriteRestoreCursor(ScreenPtr pScreen);

static void
miSpriteReportDamage (DamagePtr pDamage, RegionPtr pRegion, void *closure)
{
    ScreenPtr		    pScreen = closure;
    miSpriteScreenPtr	    pScreenPriv;
    
    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    
    if (pScreenPriv->isUp &&
	RECT_IN_REGION (pScreen, pRegion, &pScreenPriv->saved) != rgnOUT)
    {
	SPRITE_DEBUG(("Damage remove\n"));
	miSpriteRemoveCursor (pScreen);
    }
}

/*
 * miSpriteInitialize -- called from device-dependent screen
 * initialization proc after all of the function pointers have
 * been stored in the screen structure.
 */

Bool
miSpriteInitialize (pScreen, cursorFuncs, screenFuncs)
    ScreenPtr		    pScreen;
    miSpriteCursorFuncPtr   cursorFuncs;
    miPointerScreenFuncPtr  screenFuncs;
{
    miSpriteScreenPtr	pScreenPriv;
    VisualPtr		pVisual;
    
    if (!DamageSetup (pScreen))
	return FALSE;

    if (miSpriteGeneration != serverGeneration)
    {
	miSpriteScreenIndex = AllocateScreenPrivateIndex ();
	if (miSpriteScreenIndex < 0)
	    return FALSE;
	miSpriteGeneration = serverGeneration;
    }
    
    pScreenPriv = (miSpriteScreenPtr) xalloc (sizeof (miSpriteScreenRec));
    if (!pScreenPriv)
	return FALSE;
    
    pScreenPriv->pDamage = DamageCreate (miSpriteReportDamage,
					 (DamageDestroyFunc) 0,
					 DamageReportRawRegion,
					 TRUE,
					 pScreen,
					 (void *) pScreen);

    if (!miPointerInitialize (pScreen, &miSpritePointerFuncs, screenFuncs,TRUE))
    {
	xfree ((pointer) pScreenPriv);
	return FALSE;
    }
    for (pVisual = pScreen->visuals;
	 pVisual->vid != pScreen->rootVisual;
	 pVisual++)
	;
    pScreenPriv->pVisual = pVisual;
    pScreenPriv->CloseScreen = pScreen->CloseScreen;
    pScreenPriv->GetImage = pScreen->GetImage;
    pScreenPriv->GetSpans = pScreen->GetSpans;
    pScreenPriv->SourceValidate = pScreen->SourceValidate;

    pScreenPriv->CopyWindow = pScreen->CopyWindow;
    
    pScreenPriv->SaveDoomedAreas = pScreen->SaveDoomedAreas;
    
    pScreenPriv->InstallColormap = pScreen->InstallColormap;
    pScreenPriv->StoreColors = pScreen->StoreColors;
    
    pScreenPriv->BlockHandler = pScreen->BlockHandler;
    
    pScreenPriv->pCursor = NULL;
    pScreenPriv->x = 0;
    pScreenPriv->y = 0;
    pScreenPriv->isUp = FALSE;
    pScreenPriv->shouldBeUp = FALSE;
    pScreenPriv->pCacheWin = NullWindow;
    pScreenPriv->isInCacheWin = FALSE;
    pScreenPriv->checkPixels = TRUE;
    pScreenPriv->pInstalledMap = NULL;
    pScreenPriv->pColormap = NULL;
    pScreenPriv->funcs = cursorFuncs;
    pScreenPriv->colors[SOURCE_COLOR].red = 0;
    pScreenPriv->colors[SOURCE_COLOR].green = 0;
    pScreenPriv->colors[SOURCE_COLOR].blue = 0;
    pScreenPriv->colors[MASK_COLOR].red = 0;
    pScreenPriv->colors[MASK_COLOR].green = 0;
    pScreenPriv->colors[MASK_COLOR].blue = 0;
    pScreen->devPrivates[miSpriteScreenIndex].ptr = (pointer) pScreenPriv;
    
    pScreen->CloseScreen = miSpriteCloseScreen;
    pScreen->GetImage = miSpriteGetImage;
    pScreen->GetSpans = miSpriteGetSpans;
    pScreen->SourceValidate = miSpriteSourceValidate;
    
    pScreen->CopyWindow = miSpriteCopyWindow;
    
    pScreen->SaveDoomedAreas = miSpriteSaveDoomedAreas;
    
    pScreen->InstallColormap = miSpriteInstallColormap;
    pScreen->StoreColors = miSpriteStoreColors;

    pScreen->BlockHandler = miSpriteBlockHandler;
    
    return TRUE;
}

/*
 * Screen wrappers
 */

/*
 * CloseScreen wrapper -- unwrap everything, free the private data
 * and call the wrapped function
 */

static Bool
miSpriteCloseScreen (i, pScreen)
    int i;
    ScreenPtr	pScreen;
{
    miSpriteScreenPtr   pScreenPriv;

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;

    pScreen->CloseScreen = pScreenPriv->CloseScreen;
    pScreen->GetImage = pScreenPriv->GetImage;
    pScreen->GetSpans = pScreenPriv->GetSpans;
    pScreen->SourceValidate = pScreenPriv->SourceValidate;
    pScreen->BlockHandler = pScreenPriv->BlockHandler;
    pScreen->InstallColormap = pScreenPriv->InstallColormap;
    pScreen->StoreColors = pScreenPriv->StoreColors;

    pScreen->SaveDoomedAreas = pScreenPriv->SaveDoomedAreas;
    miSpriteIsUpFALSE (pScreen, pScreenPriv);
    DamageDestroy (pScreenPriv->pDamage);
    
    xfree ((pointer) pScreenPriv);

    return (*pScreen->CloseScreen) (i, pScreen);
}

static void
miSpriteGetImage (pDrawable, sx, sy, w, h, format, planemask, pdstLine)
    DrawablePtr	    pDrawable;
    int		    sx, sy, w, h;
    unsigned int    format;
    unsigned long   planemask;
    char	    *pdstLine;
{
    ScreenPtr	    pScreen = pDrawable->pScreen;
    miSpriteScreenPtr    pScreenPriv;
    
    SCREEN_PROLOGUE (pScreen, GetImage);

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;

    if (pDrawable->type == DRAWABLE_WINDOW &&
        pScreenPriv->isUp &&
	ORG_OVERLAP(&pScreenPriv->saved,pDrawable->x,pDrawable->y, sx, sy, w, h))
    {
	SPRITE_DEBUG (("GetImage remove\n"));
	miSpriteRemoveCursor (pScreen);
    }

    (*pScreen->GetImage) (pDrawable, sx, sy, w, h,
			  format, planemask, pdstLine);

    SCREEN_EPILOGUE (pScreen, GetImage, miSpriteGetImage);
}

static void
miSpriteGetSpans (pDrawable, wMax, ppt, pwidth, nspans, pdstStart)
    DrawablePtr	pDrawable;
    int		wMax;
    DDXPointPtr	ppt;
    int		*pwidth;
    int		nspans;
    char	*pdstStart;
{
    ScreenPtr		    pScreen = pDrawable->pScreen;
    miSpriteScreenPtr	    pScreenPriv;
    
    SCREEN_PROLOGUE (pScreen, GetSpans);

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;

    if (pDrawable->type == DRAWABLE_WINDOW && pScreenPriv->isUp)
    {
	register DDXPointPtr    pts;
	register int    	*widths;
	register int    	nPts;
	register int    	xorg,
				yorg;

	xorg = pDrawable->x;
	yorg = pDrawable->y;

	for (pts = ppt, widths = pwidth, nPts = nspans;
	     nPts--;
	     pts++, widths++)
 	{
	    if (SPN_OVERLAP(&pScreenPriv->saved,pts->y+yorg,
			     pts->x+xorg,*widths))
	    {
		SPRITE_DEBUG (("GetSpans remove\n"));
		miSpriteRemoveCursor (pScreen);
		break;
	    }
	}
    }

    (*pScreen->GetSpans) (pDrawable, wMax, ppt, pwidth, nspans, pdstStart);

    SCREEN_EPILOGUE (pScreen, GetSpans, miSpriteGetSpans);
}

static void
miSpriteSourceValidate (pDrawable, x, y, width, height)
    DrawablePtr	pDrawable;
    int		x, y, width, height;
{
    ScreenPtr		    pScreen = pDrawable->pScreen;
    miSpriteScreenPtr	    pScreenPriv;
    
    SCREEN_PROLOGUE (pScreen, SourceValidate);

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;

    if (pDrawable->type == DRAWABLE_WINDOW && pScreenPriv->isUp &&
	ORG_OVERLAP(&pScreenPriv->saved, pDrawable->x, pDrawable->y,
		    x, y, width, height))
    {
	SPRITE_DEBUG (("SourceValidate remove\n"));
	miSpriteRemoveCursor (pScreen);
    }

    if (pScreen->SourceValidate)
	(*pScreen->SourceValidate) (pDrawable, x, y, width, height);

    SCREEN_EPILOGUE (pScreen, SourceValidate, miSpriteSourceValidate);
}

static void
miSpriteCopyWindow (WindowPtr pWindow, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
    ScreenPtr	pScreen = pWindow->drawable.pScreen;
    miSpriteScreenPtr	    pScreenPriv;
    
    SCREEN_PROLOGUE (pScreen, CopyWindow);

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    /*
     * Damage will take care of destination check
     */
    if (pScreenPriv->isUp &&
	RECT_IN_REGION (pScreen, prgnSrc, &pScreenPriv->saved) != rgnOUT)
    {
	SPRITE_DEBUG (("CopyWindow remove\n"));
	miSpriteRemoveCursor (pScreen);
    }

    (*pScreen->CopyWindow) (pWindow, ptOldOrg, prgnSrc);
    SCREEN_EPILOGUE (pScreen, CopyWindow, miSpriteCopyWindow);
}

static void
miSpriteBlockHandler (i, blockData, pTimeout, pReadmask)
    int	i;
    pointer	blockData;
    pointer	pTimeout;
    pointer	pReadmask;
{
    ScreenPtr		pScreen = screenInfo.screens[i];
    miSpriteScreenPtr	pPriv;

    pPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;

    SCREEN_PROLOGUE(pScreen, BlockHandler);
    
    (*pScreen->BlockHandler) (i, blockData, pTimeout, pReadmask);

    SCREEN_EPILOGUE(pScreen, BlockHandler, miSpriteBlockHandler);

    if (!pPriv->isUp && pPriv->shouldBeUp)
    {
	SPRITE_DEBUG (("BlockHandler restore\n"));
	miSpriteRestoreCursor (pScreen);
    }
}

static void
miSpriteInstallColormap (pMap)
    ColormapPtr	pMap;
{
    ScreenPtr		pScreen = pMap->pScreen;
    miSpriteScreenPtr	pPriv;

    pPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;

    SCREEN_PROLOGUE(pScreen, InstallColormap);
    
    (*pScreen->InstallColormap) (pMap);

    SCREEN_EPILOGUE(pScreen, InstallColormap, miSpriteInstallColormap);

    pPriv->pInstalledMap = pMap;
    if (pPriv->pColormap != pMap)
    {
    	pPriv->checkPixels = TRUE;
	if (pPriv->isUp)
	    miSpriteRemoveCursor (pScreen);
    }
}

static void
miSpriteStoreColors (pMap, ndef, pdef)
    ColormapPtr	pMap;
    int		ndef;
    xColorItem	*pdef;
{
    ScreenPtr		pScreen = pMap->pScreen;
    miSpriteScreenPtr	pPriv;
    int			i;
    int			updated;
    VisualPtr		pVisual;

    pPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;

    SCREEN_PROLOGUE(pScreen, StoreColors);
    
    (*pScreen->StoreColors) (pMap, ndef, pdef);

    SCREEN_EPILOGUE(pScreen, StoreColors, miSpriteStoreColors);

    if (pPriv->pColormap == pMap)
    {
	updated = 0;
	pVisual = pMap->pVisual;
	if (pVisual->class == DirectColor)
	{
	    /* Direct color - match on any of the subfields */

#define MaskMatch(a,b,mask) (((a) & (pVisual->mask)) == ((b) & (pVisual->mask)))

#define UpdateDAC(plane,dac,mask) {\
    if (MaskMatch (pPriv->colors[plane].pixel,pdef[i].pixel,mask)) {\
	pPriv->colors[plane].dac = pdef[i].dac; \
	updated = 1; \
    } \
}

#define CheckDirect(plane) \
	    UpdateDAC(plane,red,redMask) \
	    UpdateDAC(plane,green,greenMask) \
	    UpdateDAC(plane,blue,blueMask)

	    for (i = 0; i < ndef; i++)
	    {
		CheckDirect (SOURCE_COLOR)
		CheckDirect (MASK_COLOR)
	    }
	}
	else
	{
	    /* PseudoColor/GrayScale - match on exact pixel */
	    for (i = 0; i < ndef; i++)
	    {
	    	if (pdef[i].pixel == pPriv->colors[SOURCE_COLOR].pixel)
	    	{
		    pPriv->colors[SOURCE_COLOR] = pdef[i];
		    if (++updated == 2)
		    	break;
	    	}
	    	if (pdef[i].pixel == pPriv->colors[MASK_COLOR].pixel)
	    	{
		    pPriv->colors[MASK_COLOR] = pdef[i];
		    if (++updated == 2)
		    	break;
	    	}
	    }
	}
    	if (updated)
    	{
	    pPriv->checkPixels = TRUE;
	    if (pPriv->isUp)
	    	miSpriteRemoveCursor (pScreen);
    	}
    }
}

static void
miSpriteFindColors (ScreenPtr pScreen)
{
    miSpriteScreenPtr	pScreenPriv = (miSpriteScreenPtr)
			    pScreen->devPrivates[miSpriteScreenIndex].ptr;
    CursorPtr		pCursor;
    xColorItem		*sourceColor, *maskColor;

    pCursor = pScreenPriv->pCursor;
    sourceColor = &pScreenPriv->colors[SOURCE_COLOR];
    maskColor = &pScreenPriv->colors[MASK_COLOR];
    if (pScreenPriv->pColormap != pScreenPriv->pInstalledMap ||
	!(pCursor->foreRed == sourceColor->red &&
	  pCursor->foreGreen == sourceColor->green &&
          pCursor->foreBlue == sourceColor->blue &&
	  pCursor->backRed == maskColor->red &&
	  pCursor->backGreen == maskColor->green &&
	  pCursor->backBlue == maskColor->blue))
    {
	pScreenPriv->pColormap = pScreenPriv->pInstalledMap;
	sourceColor->red = pCursor->foreRed;
	sourceColor->green = pCursor->foreGreen;
	sourceColor->blue = pCursor->foreBlue;
	FakeAllocColor (pScreenPriv->pColormap, sourceColor);
	maskColor->red = pCursor->backRed;
	maskColor->green = pCursor->backGreen;
	maskColor->blue = pCursor->backBlue;
	FakeAllocColor (pScreenPriv->pColormap, maskColor);
	/* "free" the pixels right away, don't let this confuse you */
	FakeFreeColor(pScreenPriv->pColormap, sourceColor->pixel);
	FakeFreeColor(pScreenPriv->pColormap, maskColor->pixel);
    }
    pScreenPriv->checkPixels = FALSE;
}

/*
 * BackingStore wrappers
 */

static void
miSpriteSaveDoomedAreas (pWin, pObscured, dx, dy)
    WindowPtr	pWin;
    RegionPtr	pObscured;
    int		dx, dy;
{
    ScreenPtr		pScreen;
    miSpriteScreenPtr   pScreenPriv;
    BoxRec		cursorBox;

    pScreen = pWin->drawable.pScreen;
    
    SCREEN_PROLOGUE (pScreen, SaveDoomedAreas);

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    if (pScreenPriv->isUp)
    {
	cursorBox = pScreenPriv->saved;

	if (dx || dy)
 	{
	    cursorBox.x1 += dx;
	    cursorBox.y1 += dy;
	    cursorBox.x2 += dx;
	    cursorBox.y2 += dy;
	}
	if (RECT_IN_REGION( pScreen, pObscured, &cursorBox) != rgnOUT)
	    miSpriteRemoveCursor (pScreen);
    }

    (*pScreen->SaveDoomedAreas) (pWin, pObscured, dx, dy);

    SCREEN_EPILOGUE (pScreen, SaveDoomedAreas, miSpriteSaveDoomedAreas);
}

/*
 * miPointer interface routines
 */

#define SPRITE_PAD  8

static Bool
miSpriteRealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    miSpriteScreenPtr	pScreenPriv;

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    if (pCursor == pScreenPriv->pCursor)
	pScreenPriv->checkPixels = TRUE;
    return (*pScreenPriv->funcs->RealizeCursor) (pScreen, pCursor);
}

static Bool
miSpriteUnrealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    miSpriteScreenPtr	pScreenPriv;

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    return (*pScreenPriv->funcs->UnrealizeCursor) (pScreen, pCursor);
}

static void
miSpriteSetCursor (pScreen, pCursor, x, y)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
    int		x;
    int		y;
{
    miSpriteScreenPtr	pScreenPriv;

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    if (!pCursor)
    {
    	pScreenPriv->shouldBeUp = FALSE;
    	if (pScreenPriv->isUp)
	    miSpriteRemoveCursor (pScreen);
	pScreenPriv->pCursor = 0;
	return;
    }
    pScreenPriv->shouldBeUp = TRUE;
    if (pScreenPriv->x == x &&
	pScreenPriv->y == y &&
	pScreenPriv->pCursor == pCursor &&
	!pScreenPriv->checkPixels)
    {
	return;
    }
    pScreenPriv->x = x;
    pScreenPriv->y = y;
    pScreenPriv->pCacheWin = NullWindow;
    if (pScreenPriv->checkPixels || pScreenPriv->pCursor != pCursor)
    {
	pScreenPriv->pCursor = pCursor;
	miSpriteFindColors (pScreen);
    }
    if (pScreenPriv->isUp) {
	int	sx, sy;
	/*
	 * check to see if the old saved region
	 * encloses the new sprite, in which case we use
	 * the flicker-free MoveCursor primitive.
	 */
	sx = pScreenPriv->x - (int)pCursor->bits->xhot;
	sy = pScreenPriv->y - (int)pCursor->bits->yhot;
	if (sx + (int) pCursor->bits->width >= pScreenPriv->saved.x1 &&
	    sx < pScreenPriv->saved.x2 &&
	    sy + (int) pCursor->bits->height >= pScreenPriv->saved.y1 &&
	    sy < pScreenPriv->saved.y2 &&
	    (int) pCursor->bits->width + (2 * SPRITE_PAD) ==
		pScreenPriv->saved.x2 - pScreenPriv->saved.x1 &&
	    (int) pCursor->bits->height + (2 * SPRITE_PAD) ==
		pScreenPriv->saved.y2 - pScreenPriv->saved.y1
	    )
	{
	    DamageDrawInternal (pScreen, TRUE);
	    miSpriteIsUpFALSE (pScreen, pScreenPriv);
	    if (!(sx >= pScreenPriv->saved.x1 &&
	      	  sx + (int)pCursor->bits->width < pScreenPriv->saved.x2 &&
	      	  sy >= pScreenPriv->saved.y1 &&
	      	  sy + (int)pCursor->bits->height < pScreenPriv->saved.y2))
	    {
		int oldx1, oldy1, dx, dy;

		oldx1 = pScreenPriv->saved.x1;
		oldy1 = pScreenPriv->saved.y1;
		dx = oldx1 - (sx - SPRITE_PAD);
		dy = oldy1 - (sy - SPRITE_PAD);
		pScreenPriv->saved.x1 -= dx;
		pScreenPriv->saved.y1 -= dy;
		pScreenPriv->saved.x2 -= dx;
		pScreenPriv->saved.y2 -= dy;
		(void) (*pScreenPriv->funcs->ChangeSave) (pScreen,
				pScreenPriv->saved.x1,
 				pScreenPriv->saved.y1,
				pScreenPriv->saved.x2 - pScreenPriv->saved.x1,
				pScreenPriv->saved.y2 - pScreenPriv->saved.y1,
				dx, dy);
	    }
	    (void) (*pScreenPriv->funcs->MoveCursor) (pScreen, pCursor,
				  pScreenPriv->saved.x1,
 				  pScreenPriv->saved.y1,
				  pScreenPriv->saved.x2 - pScreenPriv->saved.x1,
				  pScreenPriv->saved.y2 - pScreenPriv->saved.y1,
				  sx - pScreenPriv->saved.x1,
				  sy - pScreenPriv->saved.y1,
				  pScreenPriv->colors[SOURCE_COLOR].pixel,
				  pScreenPriv->colors[MASK_COLOR].pixel);
	    miSpriteIsUpTRUE (pScreen, pScreenPriv);
	    DamageDrawInternal (pScreen, FALSE);
	}
	else
	{
	    SPRITE_DEBUG (("SetCursor remove\n"));
	    miSpriteRemoveCursor (pScreen);
	}
    }
    if (!pScreenPriv->isUp && pScreenPriv->pCursor)
    {
	SPRITE_DEBUG (("SetCursor restore\n"));
	miSpriteRestoreCursor (pScreen);
    }
}

static void
miSpriteMoveCursor (pScreen, x, y)
    ScreenPtr	pScreen;
    int		x, y;
{
    miSpriteScreenPtr	pScreenPriv;

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    miSpriteSetCursor (pScreen, pScreenPriv->pCursor, x, y);
}

/*
 * undraw/draw cursor
 */

static void
miSpriteRemoveCursor (pScreen)
    ScreenPtr	pScreen;
{
    miSpriteScreenPtr   pScreenPriv;

    DamageDrawInternal (pScreen, TRUE);
    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    miSpriteIsUpFALSE (pScreen, pScreenPriv);
    pScreenPriv->pCacheWin = NullWindow;
    if (!(*pScreenPriv->funcs->RestoreUnderCursor) (pScreen,
					 pScreenPriv->saved.x1,
					 pScreenPriv->saved.y1,
					 pScreenPriv->saved.x2 - pScreenPriv->saved.x1,
					 pScreenPriv->saved.y2 - pScreenPriv->saved.y1))
    {
	miSpriteIsUpTRUE (pScreen, pScreenPriv);
    }
    DamageDrawInternal (pScreen, FALSE);
}

/*
 * Called from the block handler, restores the cursor
 * before waiting for something to do.
 */

static void
miSpriteRestoreCursor (pScreen)
    ScreenPtr	pScreen;
{
    miSpriteScreenPtr   pScreenPriv;
    int			x, y;
    CursorPtr		pCursor;

    DamageDrawInternal (pScreen, TRUE);
    miSpriteComputeSaved (pScreen);
    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    pCursor = pScreenPriv->pCursor;
    x = pScreenPriv->x - (int)pCursor->bits->xhot;
    y = pScreenPriv->y - (int)pCursor->bits->yhot;
    if ((*pScreenPriv->funcs->SaveUnderCursor) (pScreen,
				      pScreenPriv->saved.x1,
				      pScreenPriv->saved.y1,
				      pScreenPriv->saved.x2 - pScreenPriv->saved.x1,
				      pScreenPriv->saved.y2 - pScreenPriv->saved.y1))
    {
	if (pScreenPriv->checkPixels)
	    miSpriteFindColors (pScreen);
	if ((*pScreenPriv->funcs->PutUpCursor) (pScreen, pCursor, x, y,
				  pScreenPriv->colors[SOURCE_COLOR].pixel,
				  pScreenPriv->colors[MASK_COLOR].pixel))
	{
	    miSpriteIsUpTRUE (pScreen, pScreenPriv);
	}
    }
    DamageDrawInternal (pScreen, FALSE);
}

/*
 * compute the desired area of the screen to save
 */

static void
miSpriteComputeSaved (pScreen)
    ScreenPtr	pScreen;
{
    miSpriteScreenPtr   pScreenPriv;
    int		    x, y, w, h;
    int		    wpad, hpad;
    CursorPtr	    pCursor;

    pScreenPriv = (miSpriteScreenPtr) pScreen->devPrivates[miSpriteScreenIndex].ptr;
    pCursor = pScreenPriv->pCursor;
    x = pScreenPriv->x - (int)pCursor->bits->xhot;
    y = pScreenPriv->y - (int)pCursor->bits->yhot;
    w = pCursor->bits->width;
    h = pCursor->bits->height;
    wpad = SPRITE_PAD;
    hpad = SPRITE_PAD;
    pScreenPriv->saved.x1 = x - wpad;
    pScreenPriv->saved.y1 = y - hpad;
    pScreenPriv->saved.x2 = pScreenPriv->saved.x1 + w + wpad * 2;
    pScreenPriv->saved.y2 = pScreenPriv->saved.y1 + h + hpad * 2;
}
