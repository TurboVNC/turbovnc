/*
 * midispcur.c
 *
 * machine independent cursor display routines
 */

/* $XConsortium: midispcur.c,v 5.14 94/04/17 20:27:28 dpw Exp $ */

/*

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
*/

#define NEED_EVENTS
# include   "X.h"
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

extern WindowPtr    *WindowTable;

/* per-screen private data */

static int	miDCScreenIndex;
static unsigned long miDCGeneration = 0;

static Bool	miDCCloseScreen();

typedef struct {
    GCPtr	    pSourceGC, pMaskGC;
    GCPtr	    pSaveGC, pRestoreGC;
    GCPtr	    pMoveGC;
    GCPtr	    pPixSourceGC, pPixMaskGC;
    CloseScreenProcPtr CloseScreen;
    PixmapPtr	    pSave, pTemp;
} miDCScreenRec, *miDCScreenPtr;

/* per-cursor per-screen private data */
typedef struct {
    PixmapPtr		sourceBits;	    /* source bits */
    PixmapPtr		maskBits;	    /* mask bits */
} miDCCursorRec, *miDCCursorPtr;

/*
 * sprite/cursor method table
 */

static Bool	miDCRealizeCursor(),	    miDCUnrealizeCursor();
static Bool	miDCPutUpCursor(),	    miDCSaveUnderCursor();
static Bool	miDCRestoreUnderCursor(),   miDCMoveCursor();
static Bool	miDCChangeSave();

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

static Bool
miDCCloseScreen (index, pScreen)
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

static miDCCursorPtr
miDCRealize (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    miDCCursorPtr   pPriv;
    GCPtr	    pGC;
    XID		    gcvals[3];

    pPriv = (miDCCursorPtr) xalloc (sizeof (miDCCursorRec));
    if (!pPriv)
	return (miDCCursorPtr)NULL;
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
	(*pScreen->DestroyPixmap) (pPriv->sourceBits);
	(*pScreen->DestroyPixmap) (pPriv->maskBits);
	xfree ((pointer) pPriv);
	pCursor->bits->devPriv[pScreen->myNum] = (pointer)NULL;
    }
    return TRUE;
}

static void
miDCPutBits (pDrawable, pPriv, sourceGC, maskGC, x, y, w, h, source, mask)
    DrawablePtr	    pDrawable;
    GCPtr	    sourceGC, maskGC;
    int             x, y;
    unsigned        w, h;
    miDCCursorPtr   pPriv;
    unsigned long   source, mask;
{
    XID	    gcvals[1];

    if (sourceGC->fgPixel != source)
    {
	gcvals[0] = source;
	DoChangeGC (sourceGC, GCForeground, gcvals, 0);
    }
    if (sourceGC->serialNumber != pDrawable->serialNumber)
	ValidateGC (pDrawable, sourceGC);
    (*sourceGC->ops->PushPixels) (sourceGC, pPriv->sourceBits, pDrawable, w, h, x, y);
    if (maskGC->fgPixel != mask)
    {
	gcvals[0] = mask;
	DoChangeGC (maskGC, GCForeground, gcvals, 0);
    }
    if (maskGC->serialNumber != pDrawable->serialNumber)
	ValidateGC (pDrawable, maskGC);
    (*maskGC->ops->PushPixels) (maskGC, pPriv->maskBits, pDrawable, w, h, x, y);
}

#define EnsureGC(gc,win) (gc || miDCMakeGC(&gc, win))

static GCPtr
miDCMakeGC(ppGC, pWin)
    GCPtr	*ppGC;
    WindowPtr	pWin;
{
    GCPtr pGC;
    int   status;
    XID   gcvals[2];

    gcvals[0] = IncludeInferiors;
    gcvals[1] = FALSE;
    pGC = CreateGC((DrawablePtr)pWin,
		   GCSubwindowMode|GCGraphicsExposures, gcvals, &status);
    if (pGC)
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
