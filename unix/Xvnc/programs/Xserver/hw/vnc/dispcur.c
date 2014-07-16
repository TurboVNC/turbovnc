/*
 * dispcur.c
 *
 * cursor display routines - based on midispcur.c
 *
 * Modified for XFree86 4.x by Alan Hourihane <alanh@fairlite.demon.co.uk>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
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
#ifdef ARGB_CURSOR
# include   "picturestr.h"
#endif

/* per-screen private data */

int     rfbDCScreenIndex = -1;
static unsigned long rfbDCGeneration = 0;

static Bool     rfbDCCloseScreen();

typedef struct {
    GCPtr           pSourceGC, pMaskGC;
    GCPtr           pSaveGC, pRestoreGC;
    GCPtr           pPixSourceGC, pPixMaskGC;
    CloseScreenProcPtr CloseScreen;
    PixmapPtr       pSave;
#ifdef ARGB_CURSOR
    PicturePtr      pRootPicture;
    PicturePtr      pTempPicture;
#endif
} rfbDCScreenRec, *rfbDCScreenPtr;

/* per-cursor per-screen private data */
typedef struct {
    PixmapPtr           sourceBits;         /* source bits */
    PixmapPtr           maskBits;           /* mask bits */
#ifdef ARGB_CURSOR
    PicturePtr          pPicture;
#endif
} rfbDCCursorRec, *rfbDCCursorPtr;

/*
 * sprite/cursor method table
 */

static Bool     rfbDCRealizeCursor(),       rfbDCUnrealizeCursor();
static Bool     rfbDCPutUpCursor(),         rfbDCSaveUnderCursor();
static Bool     rfbDCRestoreUnderCursor();

static rfbSpriteCursorFuncRec rfbDCFuncs = {
    rfbDCRealizeCursor,
    rfbDCUnrealizeCursor,
    rfbDCPutUpCursor,
    rfbDCSaveUnderCursor,
    rfbDCRestoreUnderCursor,
};

Bool
rfbDCInitialize (pScreen, screenFuncs)
    ScreenPtr               pScreen;
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
#ifdef ARGB_CURSOR
    pScreenPriv->pRootPicture = NULL;
    pScreenPriv->pTempPicture = NULL;
#endif

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
#define tossPix(pix)    (pix ? (*pScreen->DestroyPixmap) (pix) : TRUE)
#define tossPict(pict)  (pict ? FreePicture (pict, 0) : 0)

static Bool
rfbDCCloseScreen (index, pScreen)
    int         index;
    ScreenPtr   pScreen;
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
#ifdef ARGB_CURSOR
    tossPict (pScreenPriv->pRootPicture);
    tossPict (pScreenPriv->pTempPicture);
#endif
    xfree ((pointer) pScreenPriv);
    return (*pScreen->CloseScreen) (index, pScreen);
}

static Bool
rfbDCRealizeCursor (pScreen, pCursor)
    ScreenPtr   pScreen;
    CursorPtr   pCursor;
{
    if (pCursor->bits->refcnt <= 1)
        pCursor->bits->devPriv[pScreen->myNum] = (pointer)NULL;
    return TRUE;
}

#ifdef ARGB_CURSOR
#define EnsurePicture(picture, draw, win)  \
    (picture || rfbDCMakePicture(&picture, draw, win))

static VisualPtr
rfbDCGetWindowVisual (WindowPtr pWin)
{
    ScreenPtr       pScreen = pWin->drawable.pScreen;
    VisualID        vid = wVisual (pWin);
    int             i;

    for (i = 0; i < pScreen->numVisuals; i++)
        if (pScreen->visuals[i].vid == vid)
            return &pScreen->visuals[i];
    return 0;
}

static PicturePtr
rfbDCMakePicture (PicturePtr *ppPicture, DrawablePtr pDraw, WindowPtr pWin)
{
    ScreenPtr       pScreen = pDraw->pScreen;
    VisualPtr       pVisual;
    PictFormatPtr   pFormat;
    XID             subwindow_mode = IncludeInferiors;
    PicturePtr      pPicture;
    int             error;

    pVisual = rfbDCGetWindowVisual (pWin);
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

static rfbDCCursorPtr
rfbDCRealize (pScreen, pCursor)
    ScreenPtr   pScreen;
    CursorPtr   pCursor;
{
    rfbDCCursorPtr   pPriv;
    GCPtr           pGC;
    XID             gcvals[3];

    pPriv = (rfbDCCursorPtr) xalloc (sizeof (rfbDCCursorRec));
    if (!pPriv)
        return (rfbDCCursorPtr)NULL;
#ifdef ARGB_CURSOR
    if (pCursor->bits->argb)
    {
        PixmapPtr       pPixmap;
        PictFormatPtr   pFormat;
        int             error;

        pFormat = PictureMatchFormat (pScreen, 32, PICT_a8r8g8b8);
        if (!pFormat)
        {
            xfree ((pointer) pPriv);
            return (rfbDCCursorPtr)NULL;
        }

        pPriv->sourceBits = 0;
        pPriv->maskBits = 0;
        pPixmap = (*pScreen->CreatePixmap) (pScreen, pCursor->bits->width,
                                            pCursor->bits->height, 32);
        if (!pPixmap)
        {
            xfree ((pointer) pPriv);
            return (rfbDCCursorPtr)NULL;
        }
        pGC = GetScratchGC (32, pScreen);
        if (!pGC)
        {
            (*pScreen->DestroyPixmap) (pPixmap);
            xfree ((pointer) pPriv);
            return (rfbDCCursorPtr)NULL;
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
            return (rfbDCCursorPtr)NULL;
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
    ScreenPtr   pScreen;
    CursorPtr   pCursor;
{
    rfbDCCursorPtr   pPriv;

    pPriv = (rfbDCCursorPtr) pCursor->bits->devPriv[pScreen->myNum];
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
rfbDCPutBits (pDrawable, pPriv, sourceGC, maskGC, x, y, w, h, source, mask)
    DrawablePtr     pDrawable;
    GCPtr           sourceGC, maskGC;
    int             x, y;
    unsigned        w, h;
    rfbDCCursorPtr   pPriv;
    unsigned long   source, mask;
{
    XID     gcvals[1];

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

#define EnsureGC(gc, win) (gc || rfbDCMakeGC(&gc, win))

static GCPtr
rfbDCMakeGC(ppGC, pWin)
    GCPtr       *ppGC;
    WindowPtr   pWin;
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
rfbDCPutUpCursor (pScreen, pCursor, x, y, source, mask)
    ScreenPtr       pScreen;
    CursorPtr       pCursor;
    int             x, y;
    unsigned long   source, mask;
{
    rfbDCScreenPtr   pScreenPriv;
    rfbDCCursorPtr   pPriv;
    WindowPtr       pWin;

    pPriv = (rfbDCCursorPtr) pCursor->bits->devPriv[pScreen->myNum];
    if (!pPriv)
    {
        pPriv = rfbDCRealize(pScreen, pCursor);
        if (!pPriv)
            return FALSE;
    }
    pScreenPriv = (rfbDCScreenPtr) pScreen->devPrivates[rfbDCScreenIndex].ptr;
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
        rfbDCPutBits ((DrawablePtr)pWin, pPriv,
                     pScreenPriv->pSourceGC, pScreenPriv->pMaskGC,
                     x, y, pCursor->bits->width, pCursor->bits->height,
                     source, mask);
    }
    return TRUE;
}

static Bool
rfbDCSaveUnderCursor (pScreen, x, y, w, h)
    ScreenPtr   pScreen;
    int         x, y, w, h;
{
    rfbDCScreenPtr   pScreenPriv;
    PixmapPtr       pSave;
    WindowPtr       pWin;
    GCPtr           pGC;

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
    ScreenPtr   pScreen;
    int         x, y, w, h;
{
    rfbDCScreenPtr   pScreenPriv;
    PixmapPtr       pSave;
    WindowPtr       pWin;
    GCPtr           pGC;

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
