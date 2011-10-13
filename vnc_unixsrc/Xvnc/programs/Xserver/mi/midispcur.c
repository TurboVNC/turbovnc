/*
 * midispcur.c
 *
 * machine independent cursor display routines
 */

/* $Xorg: midispcur.c,v 1.4 2001/02/09 02:05:20 xorgcvs Exp $ */

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
/* $XFree86: xc/programs/Xserver/mi/midispcur.c,v 1.9 2002/12/09 04:10:57 tsi Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#define NEED_EVENTS
# include   <X11/X.h>
# include   "misc.h"
# include   "input.h"
# include   "cursorstr.h"
# include   "windowstr.h"
# include   "regionstr.h"
# include   "dixstruct.h"
# include   "scrnintstr.h"
# include   "servermd.h"
# include   "mipointer.h"
# include   "misprite.h"
# include   "gcstruct.h"

#ifdef ARGB_CURSOR
# include   "picturestr.h"
#endif

/* per-screen private data */

static int	miDCScreenIndex;
static unsigned long miDCGeneration = 0;

static Bool	miDCCloseScreen(int index, ScreenPtr pScreen);

typedef struct {
    GCPtr	    pSourceGC, pMaskGC;
    GCPtr	    pSaveGC, pRestoreGC;
    GCPtr	    pMoveGC;
    GCPtr	    pPixSourceGC, pPixMaskGC;
    CloseScreenProcPtr CloseScreen;
    PixmapPtr	    pSave, pTemp;
#ifdef ARGB_CURSOR
    PicturePtr	    pRootPicture;
    PicturePtr	    pTempPicture;
#endif
} miDCScreenRec, *miDCScreenPtr;

/* per-cursor per-screen private data */
typedef struct {
    PixmapPtr		sourceBits;	    /* source bits */
    PixmapPtr		maskBits;	    /* mask bits */
#ifdef ARGB_CURSOR
    PicturePtr		pPicture;
#endif
} miDCCursorRec, *miDCCursorPtr;

/*
 * sprite/cursor method table
 */

static Bool	miDCRealizeCursor(ScreenPtr pScreen, CursorPtr pCursor);
static Bool	miDCUnrealizeCursor(ScreenPtr pScreen, CursorPtr pCursor);
static Bool	miDCPutUpCursor(ScreenPtr pScreen, CursorPtr pCursor,
				int x, int y, unsigned long source,
				unsigned long mask);
static Bool	miDCSaveUnderCursor(ScreenPtr pScreen, int x, int y,
				    int w, int h);
static Bool	miDCRestoreUnderCursor(ScreenPtr pScreen, int x, int y,
				       int w, int h);
static Bool	miDCMoveCursor(ScreenPtr pScreen, CursorPtr pCursor,
			       int x, int y, int w, int h, int dx, int dy,
			       unsigned long source, unsigned long mask);
static Bool	miDCChangeSave(ScreenPtr pScreen, int x, int y, int w, int h,	
			       int dx, int dy);

static miSpriteCursorFuncRec miDCFuncs = {
    miDCRealizeCursor,
    miDCUnrealizeCursor,
    miDCPutUpCursor,
    miDCSaveUnderCursor,
    miDCRestoreUnderCursor,
    miDCMoveCursor,
    miDCChangeSave,
};

Bool
miDCInitialize (pScreen, screenFuncs)
    ScreenPtr		    pScreen;
    miPointerScreenFuncPtr  screenFuncs;
{
    miDCScreenPtr   pScreenPriv;

    if (miDCGeneration != serverGeneration)
    {
	miDCScreenIndex = AllocateScreenPrivateIndex ();
	if (miDCScreenIndex < 0)
	    return FALSE;
	miDCGeneration = serverGeneration;
    }
    pScreenPriv = (miDCScreenPtr) xalloc (sizeof (miDCScreenRec));
    if (!pScreenPriv)
	return FALSE;

    /*
     * initialize the entire private structure to zeros
     */

    pScreenPriv->pSourceGC =
	pScreenPriv->pMaskGC =
	pScreenPriv->pSaveGC =
 	pScreenPriv->pRestoreGC =
 	pScreenPriv->pMoveGC =
 	pScreenPriv->pPixSourceGC =
	pScreenPriv->pPixMaskGC = NULL;
#ifdef ARGB_CURSOR
    pScreenPriv->pRootPicture = NULL;
    pScreenPriv->pTempPicture = NULL;
#endif
    
    pScreenPriv->pSave = pScreenPriv->pTemp = NULL;

    pScreenPriv->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = miDCCloseScreen;
    
    pScreen->devPrivates[miDCScreenIndex].ptr = (pointer) pScreenPriv;

    if (!miSpriteInitialize (pScreen, &miDCFuncs, screenFuncs))
    {
	xfree ((pointer) pScreenPriv);
	return FALSE;
    }
    return TRUE;
}

#define tossGC(gc)  (gc ? FreeGC (gc, (GContext) 0) : 0)
#define tossPix(pix)	(pix ? (*pScreen->DestroyPixmap) (pix) : TRUE)
#define tossPict(pict)	(pict ? FreePicture (pict, 0) : 0)

static Bool
miDCCloseScreen (index, pScreen)
    int		index;
    ScreenPtr	pScreen;
{
    miDCScreenPtr   pScreenPriv;

    pScreenPriv = (miDCScreenPtr) pScreen->devPrivates[miDCScreenIndex].ptr;
    pScreen->CloseScreen = pScreenPriv->CloseScreen;
    tossGC (pScreenPriv->pSourceGC);
    tossGC (pScreenPriv->pMaskGC);
    tossGC (pScreenPriv->pSaveGC);
    tossGC (pScreenPriv->pRestoreGC);
    tossGC (pScreenPriv->pMoveGC);
    tossGC (pScreenPriv->pPixSourceGC);
    tossGC (pScreenPriv->pPixMaskGC);
    tossPix (pScreenPriv->pSave);
    tossPix (pScreenPriv->pTemp);
#ifdef ARGB_CURSOR
    tossPict (pScreenPriv->pRootPicture);
    tossPict (pScreenPriv->pTempPicture);
#endif
    xfree ((pointer) pScreenPriv);
    return (*pScreen->CloseScreen) (index, pScreen);
}

static Bool
miDCRealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    if (pCursor->bits->refcnt <= 1)
	pCursor->bits->devPriv[pScreen->myNum] = (pointer)NULL;
    return TRUE;
}

#ifdef ARGB_CURSOR
#define EnsurePicture(picture,draw,win) (picture || miDCMakePicture(&picture,draw,win))

static VisualPtr
miDCGetWindowVisual (WindowPtr pWin)
{
    ScreenPtr	    pScreen = pWin->drawable.pScreen;
    VisualID	    vid = wVisual (pWin);
    int		    i;

    for (i = 0; i < pScreen->numVisuals; i++)
	if (pScreen->visuals[i].vid == vid)
	    return &pScreen->visuals[i];
    return 0;
}

static PicturePtr
miDCMakePicture (PicturePtr *ppPicture, DrawablePtr pDraw, WindowPtr pWin)
{
    ScreenPtr	    pScreen = pDraw->pScreen;
    VisualPtr	    pVisual;
    PictFormatPtr   pFormat;
    XID		    subwindow_mode = IncludeInferiors;
    PicturePtr	    pPicture;
    int		    error;
    
    pVisual = miDCGetWindowVisual (pWin);
    if (!pVisual)
	return 0;
    pFormat = PictureMatchVisual (pScreen, pDraw->depth, pVisual);
    if (!pFormat)
	return 0;
    pPicture = CreatePicture (0, pDraw, pFormat,
			      CPSubwindowMode, &subwindow_mode,
			      serverClient, &error);
    *ppPicture = pPicture;
    return pPicture;
}
#endif

static miDCCursorPtr
miDCRealize (
    ScreenPtr	pScreen,
    CursorPtr	pCursor)
{
    miDCCursorPtr   pPriv;
    GCPtr	    pGC;
    XID		    gcvals[3];

    pPriv = (miDCCursorPtr) xalloc (sizeof (miDCCursorRec));
    if (!pPriv)
	return (miDCCursorPtr)NULL;
#ifdef ARGB_CURSOR
    if (pCursor->bits->argb)
    {
	PixmapPtr	pPixmap;
	PictFormatPtr	pFormat;
	int		error;
	
	pFormat = PictureMatchFormat (pScreen, 32, PICT_a8r8g8b8);
	if (!pFormat)
	{
	    xfree ((pointer) pPriv);
	    return (miDCCursorPtr)NULL;
	}
	
	pPriv->sourceBits = 0;
	pPriv->maskBits = 0;
	pPixmap = (*pScreen->CreatePixmap) (pScreen, pCursor->bits->width,
					    pCursor->bits->height, 32);
	if (!pPixmap)
	{
	    xfree ((pointer) pPriv);
	    return (miDCCursorPtr)NULL;
	}
	pGC = GetScratchGC (32, pScreen);
	if (!pGC)
	{
	    (*pScreen->DestroyPixmap) (pPixmap);
	    xfree ((pointer) pPriv);
	    return (miDCCursorPtr)NULL;
	}
	ValidateGC (&pPixmap->drawable, pGC);
	(*pGC->ops->PutImage) (&pPixmap->drawable, pGC, 32,
			       0, 0, pCursor->bits->width,
			       pCursor->bits->height,
			       0, ZPixmap, (char *) pCursor->bits->argb);
	FreeScratchGC (pGC);
	pPriv->pPicture = CreatePicture (0, &pPixmap->drawable,
					pFormat, 0, 0, serverClient, &error);
        (*pScreen->DestroyPixmap) (pPixmap);
	if (!pPriv->pPicture)
	{
	    xfree ((pointer) pPriv);
	    return (miDCCursorPtr)NULL;
	}
	pCursor->bits->devPriv[pScreen->myNum] = (pointer) pPriv;
	return pPriv;
    }
    pPriv->pPicture = 0;
#endif
    pPriv->sourceBits = (*pScreen->CreatePixmap) (pScreen, pCursor->bits->width, pCursor->bits->height, 1);
    if (!pPriv->sourceBits)
    {
	xfree ((pointer) pPriv);
	return (miDCCursorPtr)NULL;
    }
    pPriv->maskBits =  (*pScreen->CreatePixmap) (pScreen, pCursor->bits->width, pCursor->bits->height, 1);
    if (!pPriv->maskBits)
    {
	(*pScreen->DestroyPixmap) (pPriv->sourceBits);
	xfree ((pointer) pPriv);
	return (miDCCursorPtr)NULL;
    }
    pCursor->bits->devPriv[pScreen->myNum] = (pointer) pPriv;

    /* create the two sets of bits, clipping as appropriate */

    pGC = GetScratchGC (1, pScreen);
    if (!pGC)
    {
	(void) miDCUnrealizeCursor (pScreen, pCursor);
	return (miDCCursorPtr)NULL;
    }

    ValidateGC ((DrawablePtr)pPriv->sourceBits, pGC);
    (*pGC->ops->PutImage) ((DrawablePtr)pPriv->sourceBits, pGC, 1,
			   0, 0, pCursor->bits->width, pCursor->bits->height,
 			   0, XYPixmap, (char *)pCursor->bits->source);
    gcvals[0] = GXand;
    ChangeGC (pGC, GCFunction, gcvals);
    ValidateGC ((DrawablePtr)pPriv->sourceBits, pGC);
    (*pGC->ops->PutImage) ((DrawablePtr)pPriv->sourceBits, pGC, 1,
			   0, 0, pCursor->bits->width, pCursor->bits->height,
 			   0, XYPixmap, (char *)pCursor->bits->mask);

    /* mask bits -- pCursor->mask & ~pCursor->source */
    gcvals[0] = GXcopy;
    ChangeGC (pGC, GCFunction, gcvals);
    ValidateGC ((DrawablePtr)pPriv->maskBits, pGC);
    (*pGC->ops->PutImage) ((DrawablePtr)pPriv->maskBits, pGC, 1,
			   0, 0, pCursor->bits->width, pCursor->bits->height,
 			   0, XYPixmap, (char *)pCursor->bits->mask);
    gcvals[0] = GXandInverted;
    ChangeGC (pGC, GCFunction, gcvals);
    ValidateGC ((DrawablePtr)pPriv->maskBits, pGC);
    (*pGC->ops->PutImage) ((DrawablePtr)pPriv->maskBits, pGC, 1,
			   0, 0, pCursor->bits->width, pCursor->bits->height,
 			   0, XYPixmap, (char *)pCursor->bits->source);
    FreeScratchGC (pGC);
    return pPriv;
}

static Bool
miDCUnrealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    miDCCursorPtr   pPriv;

    pPriv = (miDCCursorPtr) pCursor->bits->devPriv[pScreen->myNum];
    if (pPriv && (pCursor->bits->refcnt <= 1))
    {
	if (pPriv->sourceBits)
	    (*pScreen->DestroyPixmap) (pPriv->sourceBits);
	if (pPriv->maskBits)
	    (*pScreen->DestroyPixmap) (pPriv->maskBits);
#ifdef ARGB_CURSOR
	if (pPriv->pPicture)
	    FreePicture (pPriv->pPicture, 0);
#endif
	xfree ((pointer) pPriv);
	pCursor->bits->devPriv[pScreen->myNum] = (pointer)NULL;
    }
    return TRUE;
}

static void
miDCPutBits (
    DrawablePtr	    pDrawable,
    miDCCursorPtr   pPriv,
    GCPtr	    sourceGC,
    GCPtr	    maskGC,
    int             x_org,
    int             y_org,
    unsigned        w,
    unsigned        h,
    unsigned long   source,
    unsigned long   mask)
{
    XID	    gcvals[1];
    int     x, y;

    if (sourceGC->fgPixel != source)
    {
	gcvals[0] = source;
	DoChangeGC (sourceGC, GCForeground, gcvals, 0);
    }
    if (sourceGC->serialNumber != pDrawable->serialNumber)
	ValidateGC (pDrawable, sourceGC);

    if(sourceGC->miTranslate) 
    {
        x = pDrawable->x + x_org;
        y = pDrawable->y + y_org;
    } 
    else
    {
        x = x_org;
        y = y_org;
    }

    (*sourceGC->ops->PushPixels) (sourceGC, pPriv->sourceBits, pDrawable, w, h, x, y);
    if (maskGC->fgPixel != mask)
    {
	gcvals[0] = mask;
	DoChangeGC (maskGC, GCForeground, gcvals, 0);
    }
    if (maskGC->serialNumber != pDrawable->serialNumber)
	ValidateGC (pDrawable, maskGC);

    if(maskGC->miTranslate) 
    {
        x = pDrawable->x + x_org;
        y = pDrawable->y + y_org;
    } 
    else
    {
        x = x_org;
        y = y_org;
    }

    (*maskGC->ops->PushPixels) (maskGC, pPriv->maskBits, pDrawable, w, h, x, y);
}

#define EnsureGC(gc,win) (gc || miDCMakeGC(&gc, win))

static GCPtr
miDCMakeGC(
    GCPtr	*ppGC,
    WindowPtr	pWin)
{
    GCPtr pGC;
    int   status;
    XID   gcvals[2];

    gcvals[0] = IncludeInferiors;
    gcvals[1] = FALSE;
    pGC = CreateGC((DrawablePtr)pWin,
		   GCSubwindowMode|GCGraphicsExposures, gcvals, &status);
    if (pGC && pWin->drawable.pScreen->DrawGuarantee)
	(*pWin->drawable.pScreen->DrawGuarantee) (pWin, pGC, GuaranteeVisBack);
    *ppGC = pGC;
    return pGC;
}


static Bool
miDCPutUpCursor (pScreen, pCursor, x, y, source, mask)
    ScreenPtr	    pScreen;
    CursorPtr	    pCursor;
    int		    x, y;
    unsigned long   source, mask;
{
    miDCScreenPtr   pScreenPriv;
    miDCCursorPtr   pPriv;
    WindowPtr	    pWin;

    pPriv = (miDCCursorPtr) pCursor->bits->devPriv[pScreen->myNum];
    if (!pPriv)
    {
	pPriv = miDCRealize(pScreen, pCursor);
	if (!pPriv)
	    return FALSE;
    }
    pScreenPriv = (miDCScreenPtr) pScreen->devPrivates[miDCScreenIndex].ptr;
    pWin = WindowTable[pScreen->myNum];
#ifdef ARGB_CURSOR
    if (pPriv->pPicture)
    {
	if (!EnsurePicture(pScreenPriv->pRootPicture, &pWin->drawable, pWin))
	    return FALSE;
	CompositePicture (PictOpOver,
			  pPriv->pPicture,
			  NULL,
			  pScreenPriv->pRootPicture,
			  0, 0, 0, 0, 
			  x, y, 
			  pCursor->bits->width,
			  pCursor->bits->height);
    }
    else
#endif
    {
	if (!EnsureGC(pScreenPriv->pSourceGC, pWin))
	    return FALSE;
	if (!EnsureGC(pScreenPriv->pMaskGC, pWin))
	{
	    FreeGC (pScreenPriv->pSourceGC, (GContext) 0);
	    pScreenPriv->pSourceGC = 0;
	    return FALSE;
	}
	miDCPutBits ((DrawablePtr)pWin, pPriv,
		     pScreenPriv->pSourceGC, pScreenPriv->pMaskGC,
		     x, y, pCursor->bits->width, pCursor->bits->height,
		     source, mask);
    }
    return TRUE;
}

static Bool
miDCSaveUnderCursor (pScreen, x, y, w, h)
    ScreenPtr	pScreen;
    int		x, y, w, h;
{
    miDCScreenPtr   pScreenPriv;
    PixmapPtr	    pSave;
    WindowPtr	    pWin;
    GCPtr	    pGC;

    pScreenPriv = (miDCScreenPtr) pScreen->devPrivates[miDCScreenIndex].ptr;
    pSave = pScreenPriv->pSave;
    pWin = WindowTable[pScreen->myNum];
    if (!pSave || pSave->drawable.width < w || pSave->drawable.height < h)
    {
	if (pSave)
	    (*pScreen->DestroyPixmap) (pSave);
	pScreenPriv->pSave = pSave =
		(*pScreen->CreatePixmap) (pScreen, w, h, pScreen->rootDepth);
	if (!pSave)
	    return FALSE;
    }
    if (!EnsureGC(pScreenPriv->pSaveGC, pWin))
	return FALSE;
    pGC = pScreenPriv->pSaveGC;
    if (pSave->drawable.serialNumber != pGC->serialNumber)
	ValidateGC ((DrawablePtr) pSave, pGC);
    (*pGC->ops->CopyArea) ((DrawablePtr) pWin, (DrawablePtr) pSave, pGC,
			    x, y, w, h, 0, 0);
    return TRUE;
}

static Bool
miDCRestoreUnderCursor (pScreen, x, y, w, h)
    ScreenPtr	pScreen;
    int		x, y, w, h;
{
    miDCScreenPtr   pScreenPriv;
    PixmapPtr	    pSave;
    WindowPtr	    pWin;
    GCPtr	    pGC;

    pScreenPriv = (miDCScreenPtr) pScreen->devPrivates[miDCScreenIndex].ptr;
    pSave = pScreenPriv->pSave;
    pWin = WindowTable[pScreen->myNum];
    if (!pSave)
	return FALSE;
    if (!EnsureGC(pScreenPriv->pRestoreGC, pWin))
	return FALSE;
    pGC = pScreenPriv->pRestoreGC;
    if (pWin->drawable.serialNumber != pGC->serialNumber)
	ValidateGC ((DrawablePtr) pWin, pGC);
    (*pGC->ops->CopyArea) ((DrawablePtr) pSave, (DrawablePtr) pWin, pGC,
			    0, 0, w, h, x, y);
    return TRUE;
}

static Bool
miDCChangeSave (pScreen, x, y, w, h, dx, dy)
    ScreenPtr	    pScreen;
    int		    x, y, w, h, dx, dy;
{
    miDCScreenPtr   pScreenPriv;
    PixmapPtr	    pSave;
    WindowPtr	    pWin;
    GCPtr	    pGC;
    int		    sourcex, sourcey, destx, desty, copyw, copyh;

    pScreenPriv = (miDCScreenPtr) pScreen->devPrivates[miDCScreenIndex].ptr;
    pSave = pScreenPriv->pSave;
    pWin = WindowTable[pScreen->myNum];
    /*
     * restore the bits which are about to get trashed
     */
    if (!pSave)
	return FALSE;
    if (!EnsureGC(pScreenPriv->pRestoreGC, pWin))
	return FALSE;
    pGC = pScreenPriv->pRestoreGC;
    if (pWin->drawable.serialNumber != pGC->serialNumber)
	ValidateGC ((DrawablePtr) pWin, pGC);
    /*
     * copy the old bits to the screen.
     */
    if (dy > 0)
    {
	(*pGC->ops->CopyArea) ((DrawablePtr) pSave, (DrawablePtr) pWin, pGC,
			       0, h - dy, w, dy, x + dx, y + h);
    }
    else if (dy < 0)
    {
	(*pGC->ops->CopyArea) ((DrawablePtr) pSave, (DrawablePtr) pWin, pGC,
			       0, 0, w, -dy, x + dx, y + dy);
    }
    if (dy >= 0)
    {
	desty = y + dy;
	sourcey = 0;
	copyh = h - dy;
    }
    else
    {
	desty = y;
	sourcey = - dy;
	copyh = h + dy;
    }
    if (dx > 0)
    {
	(*pGC->ops->CopyArea) ((DrawablePtr) pSave, (DrawablePtr) pWin, pGC,
			       w - dx, sourcey, dx, copyh, x + w, desty);
    }
    else if (dx < 0)
    {
	(*pGC->ops->CopyArea) ((DrawablePtr) pSave, (DrawablePtr) pWin, pGC,
			       0, sourcey, -dx, copyh, x + dx, desty);
    }
    if (!EnsureGC(pScreenPriv->pSaveGC, pWin))
	return FALSE;
    pGC = pScreenPriv->pSaveGC;
    if (pSave->drawable.serialNumber != pGC->serialNumber)
	ValidateGC ((DrawablePtr) pSave, pGC);
    /*
     * move the bits that are still valid within the pixmap
     */
    if (dx >= 0)
    {
	sourcex = 0;
	destx = dx;
	copyw = w - dx;
    }
    else
    {
	destx = 0;
	sourcex = - dx;
	copyw = w + dx;
    }
    if (dy >= 0)
    {
	sourcey = 0;
	desty = dy;
	copyh = h - dy;
    }
    else
    {
	desty = 0;
	sourcey = -dy;
	copyh = h + dy;
    }
    (*pGC->ops->CopyArea) ((DrawablePtr) pSave, (DrawablePtr) pSave, pGC,
			   sourcex, sourcey, copyw, copyh, destx, desty);
    /*
     * copy the new bits from the screen into the remaining areas of the
     * pixmap
     */
    if (dy > 0)
    {
	(*pGC->ops->CopyArea) ((DrawablePtr) pWin, (DrawablePtr) pSave, pGC,
			       x, y, w, dy, 0, 0);
    }
    else if (dy < 0)
    {
	(*pGC->ops->CopyArea) ((DrawablePtr) pWin, (DrawablePtr) pSave, pGC,
			       x, y + h + dy, w, -dy, 0, h + dy);
    }
    if (dy >= 0)
    {
	desty = dy;
	sourcey = y + dy;
	copyh = h - dy;
    }
    else
    {
	desty = 0;
	sourcey = y;
	copyh = h + dy;
    }
    if (dx > 0)
    {
	(*pGC->ops->CopyArea) ((DrawablePtr) pWin, (DrawablePtr) pSave, pGC,
			       x, sourcey, dx, copyh, 0, desty);
    }
    else if (dx < 0)
    {
	(*pGC->ops->CopyArea) ((DrawablePtr) pWin, (DrawablePtr) pSave, pGC,
			       x + w + dx, sourcey, -dx, copyh, w + dx, desty);
    }
    return TRUE;
}

static Bool
miDCMoveCursor (pScreen, pCursor, x, y, w, h, dx, dy, source, mask)
    ScreenPtr	    pScreen;
    CursorPtr	    pCursor;
    int		    x, y, w, h, dx, dy;
    unsigned long   source, mask;
{
    miDCCursorPtr   pPriv;
    miDCScreenPtr   pScreenPriv;
    int		    status;
    WindowPtr	    pWin;
    GCPtr	    pGC;
    XID		    gcval = FALSE;
    PixmapPtr	    pTemp;

    pPriv = (miDCCursorPtr) pCursor->bits->devPriv[pScreen->myNum];
    if (!pPriv)
    {
	pPriv = miDCRealize(pScreen, pCursor);
	if (!pPriv)
	    return FALSE;
    }
    pScreenPriv = (miDCScreenPtr) pScreen->devPrivates[miDCScreenIndex].ptr;
    pWin = WindowTable[pScreen->myNum];
    pTemp = pScreenPriv->pTemp;
    if (!pTemp ||
	pTemp->drawable.width != pScreenPriv->pSave->drawable.width ||
	pTemp->drawable.height != pScreenPriv->pSave->drawable.height)
    {
	if (pTemp)
	    (*pScreen->DestroyPixmap) (pTemp);
#ifdef ARGB_CURSOR
	if (pScreenPriv->pTempPicture)
	{
	    FreePicture (pScreenPriv->pTempPicture, 0);
	    pScreenPriv->pTempPicture = 0;
	}
#endif
	pScreenPriv->pTemp = pTemp = (*pScreen->CreatePixmap)
	    (pScreen, w, h, pScreenPriv->pSave->drawable.depth);
	if (!pTemp)
	    return FALSE;
    }
    if (!pScreenPriv->pMoveGC)
    {
	pScreenPriv->pMoveGC = CreateGC ((DrawablePtr)pTemp,
	    GCGraphicsExposures, &gcval, &status);
	if (!pScreenPriv->pMoveGC)
	    return FALSE;
    }
    /*
     * copy the saved area to a temporary pixmap
     */
    pGC = pScreenPriv->pMoveGC;
    if (pGC->serialNumber != pTemp->drawable.serialNumber)
	ValidateGC ((DrawablePtr) pTemp, pGC);
    (*pGC->ops->CopyArea)((DrawablePtr)pScreenPriv->pSave,
			  (DrawablePtr)pTemp, pGC, 0, 0, w, h, 0, 0);
    
    /*
     * draw the cursor in the temporary pixmap
     */
#ifdef ARGB_CURSOR
    if (pPriv->pPicture)
    {
	if (!EnsurePicture(pScreenPriv->pTempPicture, &pTemp->drawable, pWin))
	    return FALSE;
	CompositePicture (PictOpOver,
			  pPriv->pPicture,
			  NULL,
			  pScreenPriv->pTempPicture,
			  0, 0, 0, 0, 
			  dx, dy, 
			  pCursor->bits->width,
			  pCursor->bits->height);
    }
    else
#endif
    {
	if (!pScreenPriv->pPixSourceGC)
	{
	    pScreenPriv->pPixSourceGC = CreateGC ((DrawablePtr)pTemp,
		GCGraphicsExposures, &gcval, &status);
	    if (!pScreenPriv->pPixSourceGC)
		return FALSE;
	}
	if (!pScreenPriv->pPixMaskGC)
	{
	    pScreenPriv->pPixMaskGC = CreateGC ((DrawablePtr)pTemp,
		GCGraphicsExposures, &gcval, &status);
	    if (!pScreenPriv->pPixMaskGC)
		return FALSE;
	}
	miDCPutBits ((DrawablePtr)pTemp, pPriv,
		     pScreenPriv->pPixSourceGC, pScreenPriv->pPixMaskGC,
		     dx, dy, pCursor->bits->width, pCursor->bits->height,
		     source, mask);
    }

    /*
     * copy the temporary pixmap onto the screen
     */

    if (!EnsureGC(pScreenPriv->pRestoreGC, pWin))
	return FALSE;
    pGC = pScreenPriv->pRestoreGC;
    if (pWin->drawable.serialNumber != pGC->serialNumber)
	ValidateGC ((DrawablePtr) pWin, pGC);

    (*pGC->ops->CopyArea) ((DrawablePtr) pTemp, (DrawablePtr) pWin,
			    pGC,
			    0, 0, w, h, x, y);
    return TRUE;
}
