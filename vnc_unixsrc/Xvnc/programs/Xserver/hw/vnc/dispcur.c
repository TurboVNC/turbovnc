/*
 * dispcur.c
 *
 * cursor display routines - based on midispcur.c
 */

/*
 *  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 *  USA.
 */

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
# include   "sprite.h"
# include   "gcstruct.h"

extern WindowPtr    *WindowTable;

/* per-screen private data */

static int	rfbDCScreenIndex;
static unsigned long rfbDCGeneration = 0;

static Bool	rfbDCCloseScreen();

typedef struct {
    GCPtr	    pSourceGC, pMaskGC;
    GCPtr	    pSaveGC, pRestoreGC;
    GCPtr	    pPixSourceGC, pPixMaskGC;
    CloseScreenProcPtr CloseScreen;
    PixmapPtr	    pSave;
} rfbDCScreenRec, *rfbDCScreenPtr;

/* per-cursor per-screen private data */
typedef struct {
    PixmapPtr		sourceBits;	    /* source bits */
    PixmapPtr		maskBits;	    /* mask bits */
} rfbDCCursorRec, *rfbDCCursorPtr;

/*
 * sprite/cursor method table
 */

static Bool	rfbDCRealizeCursor(),	    rfbDCUnrealizeCursor();
static Bool	rfbDCPutUpCursor(),	    rfbDCSaveUnderCursor();
static Bool	rfbDCRestoreUnderCursor();

static rfbSpriteCursorFuncRec rfbDCFuncs = {
    rfbDCRealizeCursor,
    rfbDCUnrealizeCursor,
    rfbDCPutUpCursor,
    rfbDCSaveUnderCursor,
    rfbDCRestoreUnderCursor,
};

Bool
rfbDCInitialize (pScreen, screenFuncs)
    ScreenPtr		    pScreen;
    miPointerScreenFuncPtr  screenFuncs;
{
    rfbDCScreenPtr   pScreenPriv;

    if (rfbDCGeneration != serverGeneration)
    {
	rfbDCScreenIndex = AllocateScreenPrivateIndex ();
	if (rfbDCScreenIndex < 0)
	    return FALSE;
	rfbDCGeneration = serverGeneration;
    }
    pScreenPriv = (rfbDCScreenPtr) xalloc (sizeof (rfbDCScreenRec));
    if (!pScreenPriv)
	return FALSE;

    /*
     * initialize the entire private structure to zeros
     */

    pScreenPriv->pSourceGC =
	pScreenPriv->pMaskGC =
	pScreenPriv->pSaveGC =
 	pScreenPriv->pRestoreGC =
 	pScreenPriv->pPixSourceGC =
	pScreenPriv->pPixMaskGC = NULL;
    
    pScreenPriv->pSave = NULL;

    pScreenPriv->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = rfbDCCloseScreen;
    
    pScreen->devPrivates[rfbDCScreenIndex].ptr = (pointer) pScreenPriv;

    if (!rfbSpriteInitialize (pScreen, &rfbDCFuncs, screenFuncs))
    {
	xfree ((pointer) pScreenPriv);
	return FALSE;
    }
    return TRUE;
}

#define tossGC(gc)  (gc ? FreeGC (gc, (GContext) 0) : 0)
#define tossPix(pix)	(pix ? (*pScreen->DestroyPixmap) (pix) : TRUE)

static Bool
rfbDCCloseScreen (index, pScreen)
    ScreenPtr	pScreen;
{
    rfbDCScreenPtr   pScreenPriv;

    pScreenPriv = (rfbDCScreenPtr) pScreen->devPrivates[rfbDCScreenIndex].ptr;
    pScreen->CloseScreen = pScreenPriv->CloseScreen;
    tossGC (pScreenPriv->pSourceGC);
    tossGC (pScreenPriv->pMaskGC);
    tossGC (pScreenPriv->pSaveGC);
    tossGC (pScreenPriv->pRestoreGC);
    tossGC (pScreenPriv->pPixSourceGC);
    tossGC (pScreenPriv->pPixMaskGC);
    tossPix (pScreenPriv->pSave);
    xfree ((pointer) pScreenPriv);
    return (*pScreen->CloseScreen) (index, pScreen);
}

static Bool
rfbDCRealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    if (pCursor->bits->refcnt <= 1)
	pCursor->bits->devPriv[pScreen->myNum] = (pointer)NULL;
    return TRUE;
}

static rfbDCCursorPtr
rfbDCRealize (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    rfbDCCursorPtr   pPriv;
    GCPtr	    pGC;
    XID		    gcvals[3];

    pPriv = (rfbDCCursorPtr) xalloc (sizeof (rfbDCCursorRec));
    if (!pPriv)
	return (rfbDCCursorPtr)NULL;
    pPriv->sourceBits = (*pScreen->CreatePixmap) (pScreen, pCursor->bits->width, pCursor->bits->height, 1);
    if (!pPriv->sourceBits)
    {
	xfree ((pointer) pPriv);
	return (rfbDCCursorPtr)NULL;
    }
    pPriv->maskBits =  (*pScreen->CreatePixmap) (pScreen, pCursor->bits->width, pCursor->bits->height, 1);
    if (!pPriv->maskBits)
    {
	(*pScreen->DestroyPixmap) (pPriv->sourceBits);
	xfree ((pointer) pPriv);
	return (rfbDCCursorPtr)NULL;
    }
    pCursor->bits->devPriv[pScreen->myNum] = (pointer) pPriv;

    /* create the two sets of bits, clipping as appropriate */

    pGC = GetScratchGC (1, pScreen);
    if (!pGC)
    {
	(void) rfbDCUnrealizeCursor (pScreen, pCursor);
	return (rfbDCCursorPtr)NULL;
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
rfbDCUnrealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    rfbDCCursorPtr   pPriv;

    pPriv = (rfbDCCursorPtr) pCursor->bits->devPriv[pScreen->myNum];
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
rfbDCPutBits (pDrawable, pPriv, sourceGC, maskGC, x, y, w, h, source, mask)
    DrawablePtr	    pDrawable;
    GCPtr	    sourceGC, maskGC;
    int             x, y;
    unsigned        w, h;
    rfbDCCursorPtr   pPriv;
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

#define EnsureGC(gc,win) (gc || rfbDCMakeGC(&gc, win))

static GCPtr
rfbDCMakeGC(ppGC, pWin)
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
rfbDCPutUpCursor (pScreen, pCursor, x, y, source, mask)
    ScreenPtr	    pScreen;
    CursorPtr	    pCursor;
    int		    x, y;
    unsigned long   source, mask;
{
    rfbDCScreenPtr   pScreenPriv;
    rfbDCCursorPtr   pPriv;
    WindowPtr	    pWin;

    pPriv = (rfbDCCursorPtr) pCursor->bits->devPriv[pScreen->myNum];
    if (!pPriv)
    {
	pPriv = rfbDCRealize(pScreen, pCursor);
	if (!pPriv)
	    return FALSE;
    }
    pScreenPriv = (rfbDCScreenPtr) pScreen->devPrivates[rfbDCScreenIndex].ptr;
    pWin = WindowTable[pScreen->myNum];
    if (!EnsureGC(pScreenPriv->pSourceGC, pWin))
	return FALSE;
    if (!EnsureGC(pScreenPriv->pMaskGC, pWin))
    {
	FreeGC (pScreenPriv->pSourceGC, (GContext) 0);
	pScreenPriv->pSourceGC = 0;
	return FALSE;
    }
    rfbDCPutBits ((DrawablePtr)pWin, pPriv,
		 pScreenPriv->pSourceGC, pScreenPriv->pMaskGC,
		 x, y, pCursor->bits->width, pCursor->bits->height,
		 source, mask);
    return TRUE;
}

static Bool
rfbDCSaveUnderCursor (pScreen, x, y, w, h)
    ScreenPtr	pScreen;
    int		x, y, w, h;
{
    rfbDCScreenPtr   pScreenPriv;
    PixmapPtr	    pSave;
    WindowPtr	    pWin;
    GCPtr	    pGC;

    pScreenPriv = (rfbDCScreenPtr) pScreen->devPrivates[rfbDCScreenIndex].ptr;
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
rfbDCRestoreUnderCursor (pScreen, x, y, w, h)
    ScreenPtr	pScreen;
    int		x, y, w, h;
{
    rfbDCScreenPtr   pScreenPriv;
    PixmapPtr	    pSave;
    WindowPtr	    pWin;
    GCPtr	    pGC;

    pScreenPriv = (rfbDCScreenPtr) pScreen->devPrivates[rfbDCScreenIndex].ptr;
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
