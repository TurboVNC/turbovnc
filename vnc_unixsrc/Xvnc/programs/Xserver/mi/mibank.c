/*
 * Copyright 1997 through 2004 by Marc Aurele La France (TSI @ UQV), tsi@xfree86.org
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of Marc Aurele La France not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Marc Aurele La France makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as-is" without express or implied warranty.
 *
 * MARC AURELE LA FRANCE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL MARC AURELE LA FRANCE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Copyright 1990,91,92,93 by Thomas Roell, Germany.
 * Copyright 1991,92,93    by SGCS (Snitily Graphics Consulting Services), USA.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this  permission notice appear
 * in supporting documentation, and that the name of Thomas Roell nor
 * SGCS be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 * Thomas Roell nor SGCS makes no representations about the suitability
 * of this software for any purpose. It is provided "as is" without
 * express or implied warranty.
 *
 * THOMAS ROELL AND SGCS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THOMAS ROELL OR SGCS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* $XFree86: xc/programs/Xserver/mi/mibank.c,v 1.15 2003/11/10 18:39:16 tsi Exp $ */

/*
 * This thing originated from an idea of Edwin Goei and his bank switching
 * code for the DEC TX board.
 */

/*
 * Heavily modified for the XFree86 Project to turn this into an mi wrapper.
 * ---  Marc Aurele La France (tsi@xfree86.org)
 */

/*
 * "Heavily modified", indeed!  By the time this is finalized, there probably
 * won't be much left of Roell's code...
 *
 * Miscellaneous notes:
 * - Pixels with imbedded bank boundaries are required to be off-screen.  There
 *   >might< be a way to fool the underlying framebuffer into dealing with
 *   partial pixels.
 * - Plans to generalise this to do (hardware) colour plane switching have been
 *   dropped due to colour flashing concerns.
 *
 * TODO:
 * - Allow miModifyBanking() to change BankSize and nBankDepth.
 * - Re-instate shared and double banking for framebuffers whose pixmap formats
 *   don't describe how the server "sees" the screen.
 * - Remove remaining assumptions that a pixmap's devPrivate field points
 *   directly to its pixel data.
 */

/* #define NO_ALLOCA 1 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "servermd.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "mi.h"
#include "mibank.h"

#define BANK_SINGLE 0
#define BANK_SHARED 1
#define BANK_DOUBLE 2
#define BANK_NOBANK 3

typedef struct _miBankScreen
{
    miBankInfoRec BankInfo;
    unsigned int  nBankBPP;
    unsigned int  type;

    unsigned long nBitsPerBank;
    unsigned long nBitsPerScanline;
    unsigned long nPixelsPerScanlinePadUnit;

    PixmapPtr     pScreenPixmap;
    PixmapPtr     pBankPixmap;
    GCPtr         pBankGC;

    int           nBanks, maxRects;
    RegionPtr     *pBanks;

    pointer       pbits;

    /*
     * Screen Wrappers
     */
    CreateScreenResourcesProcPtr  CreateScreenResources;
    ModifyPixmapHeaderProcPtr     ModifyPixmapHeader;
    CloseScreenProcPtr            CloseScreen;
    GetImageProcPtr               GetImage;
    GetSpansProcPtr               GetSpans;
    CreateGCProcPtr               CreateGC;
    PaintWindowBackgroundProcPtr  PaintWindowBackground;
    PaintWindowBorderProcPtr      PaintWindowBorder;
    CopyWindowProcPtr             CopyWindow;
    BSFuncRec                     BackingStoreFuncs;
} miBankScreenRec, *miBankScreenPtr;

typedef struct _miBankGC
{
    GCOps     *wrappedOps,   *unwrappedOps;
    GCFuncs   *wrappedFuncs, *unwrappedFuncs;

    Bool      fastCopy, fastPlane;

    RegionPtr pBankedClips[1];
} miBankGCRec, *miBankGCPtr;

typedef struct _miBankQueue
{
    Bool           fastBlit;
    unsigned short srcBankNo;
    unsigned short dstBankNo;
    short          x;
    short          y;
    short          w;
    short          h;
} miBankQueue;

/*
 * CAVEAT:  This banking scheme requires that the DDX store Pixmap data in the
 *          server's address space.
 */

#define ModifyPixmap(_pPix, _width, _devKind, _pbits) \
    (*pScreen->ModifyPixmapHeader)((_pPix), \
        (_width), -1, -1, -1, (_devKind), (_pbits))

#define SET_SINGLE_BANK(_pPix, _width, _devKind, _no) \
    ModifyPixmap(_pPix, _width, _devKind, \
        (char *)pScreenPriv->BankInfo.pBankA + \
        (*pScreenPriv->BankInfo.SetSourceAndDestinationBanks)(pScreen, (_no)) - \
        (pScreenPriv->BankInfo.BankSize * (_no)))

#define SET_SOURCE_BANK(_pPix, _width, _devKind, _no) \
    ModifyPixmap(_pPix, _width, _devKind, \
        (char *)pScreenPriv->BankInfo.pBankA + \
        (*pScreenPriv->BankInfo.SetSourceBank)(pScreen, (_no)) - \
        (pScreenPriv->BankInfo.BankSize * (_no)))

#define SET_DESTINATION_BANK(_pPix, _width, _devKind, _no) \
    ModifyPixmap(_pPix, _width, _devKind, \
        (char *)pScreenPriv->BankInfo.pBankB + \
        (*pScreenPriv->BankInfo.SetDestinationBank)(pScreen, (_no)) - \
        (pScreenPriv->BankInfo.BankSize * (_no)))

#define ALLOCATE_LOCAL_ARRAY(atype, ntype) \
    (atype *)ALLOCATE_LOCAL((ntype) * sizeof(atype))

static int           miBankScreenIndex;
static int           miBankGCIndex;
static unsigned long miBankGeneration = 0;

#define BANK_SCRPRIVLVAL pScreen->devPrivates[miBankScreenIndex].ptr

#define BANK_SCRPRIVATE ((miBankScreenPtr)(BANK_SCRPRIVLVAL))

#define BANK_GCPRIVLVAL(pGC) (pGC)->devPrivates[miBankGCIndex].ptr

#define BANK_GCPRIVATE(pGC) ((miBankGCPtr)(BANK_GCPRIVLVAL(pGC)))

#define PIXMAP_STATUS(_pPix) \
    pointer pbits = (_pPix)->devPrivate.ptr

#define PIXMAP_SAVE(_pPix) \
    PIXMAP_STATUS(_pPix); \
    if (pbits == (pointer)pScreenPriv) \
        (_pPix)->devPrivate.ptr = pScreenPriv->pbits

#define PIXMAP_RESTORE(_pPix) \
    (_pPix)->devPrivate.ptr = pbits

#define BANK_SAVE \
    int width   = pScreenPriv->pBankPixmap->drawable.width; \
    int devKind = pScreenPriv->pBankPixmap->devKind; \
    PIXMAP_SAVE(pScreenPriv->pBankPixmap)

#define BANK_RESTORE \
    pScreenPriv->pBankPixmap->drawable.width = width; \
    pScreenPriv->pBankPixmap->devKind = devKind; \
    PIXMAP_RESTORE(pScreenPriv->pBankPixmap)

#define SCREEN_STATUS \
    PIXMAP_STATUS(pScreenPriv->pScreenPixmap)

#define SCREEN_SAVE \
    PIXMAP_SAVE(pScreenPriv->pScreenPixmap)

#define SCREEN_RESTORE \
    PIXMAP_RESTORE(pScreenPriv->pScreenPixmap)

#define SCREEN_INIT \
    miBankScreenPtr pScreenPriv = BANK_SCRPRIVATE

#define SCREEN_UNWRAP(field) \
    pScreen->field = pScreenPriv->field

#define SCREEN_WRAP(field, wrapper) \
    pScreenPriv->field = pScreen->field; \
    pScreen->field     = wrapper

#define GC_INIT(pGC) \
    miBankGCPtr pGCPriv = BANK_GCPRIVATE(pGC)

#define GC_UNWRAP(pGC) \
    pGCPriv->unwrappedOps   = (pGC)->ops; \
    pGCPriv->unwrappedFuncs = (pGC)->funcs; \
    (pGC)->ops              = pGCPriv->wrappedOps; \
    (pGC)->funcs            = pGCPriv->wrappedFuncs

#define GC_WRAP(pGC) \
    pGCPriv->wrappedOps   = (pGC)->ops; \
    pGCPriv->wrappedFuncs = (pGC)->funcs; \
    (pGC)->ops            = pGCPriv->unwrappedOps; \
    (pGC)->funcs          = pGCPriv->unwrappedFuncs

#define IS_BANKED(pDrawable) \
    ((pbits == (pointer)pScreenPriv) && \
     (((DrawablePtr)(pDrawable))->type == DRAWABLE_WINDOW))

#define CLIP_SAVE \
    RegionPtr pOrigCompositeClip = pGC->pCompositeClip

#define CLIP_RESTORE \
    pGC->pCompositeClip = pOrigCompositeClip

#define GCOP_INIT \
    ScreenPtr pScreen = pGC->pScreen; \
    SCREEN_INIT; \
    GC_INIT(pGC)

#define GCOP_UNWRAP \
    GC_UNWRAP(pGC)

#define GCOP_WRAP \
    GC_WRAP(pGC)

#define GCOP_TOP_PART \
    for (i = 0;  i < pScreenPriv->nBanks;  i++) \
    { \
        if (!(pGC->pCompositeClip = pGCPriv->pBankedClips[i])) \
            continue; \
        GCOP_UNWRAP; \
        SET_SINGLE_BANK(pScreenPriv->pScreenPixmap, -1, -1, i)

#define GCOP_BOTTOM_PART \
        GCOP_WRAP; \
    }

#define GCOP_SIMPLE(statement) \
    if (nArray > 0) \
    { \
        GCOP_INIT; \
        SCREEN_SAVE; \
        if (!IS_BANKED(pDrawable)) \
        { \
            GCOP_UNWRAP; \
            statement; \
            GCOP_WRAP; \
        } \
        else \
        { \
            int i; \
            CLIP_SAVE; \
            GCOP_TOP_PART; \
            statement; \
            GCOP_BOTTOM_PART; \
            CLIP_RESTORE; \
        } \
        SCREEN_RESTORE; \
    }

#define GCOP_0D_ARGS mode,
#define GCOP_1D_ARGS
#define GCOP_2D_ARGS shape, mode,

#define GCOP_COMPLEX(aop, atype) \
    if (nArray > 0) \
    { \
        GCOP_INIT; \
        SCREEN_SAVE; \
        if (!IS_BANKED(pDrawable)) \
        { \
            GCOP_UNWRAP; \
            (*pGC->ops->aop)(pDrawable, pGC, GCOP_ARGS nArray, pArray); \
            GCOP_WRAP; \
        } \
        else \
        { \
            atype *aarg = pArray, *acopy; \
            int   i; \
            CLIP_SAVE; \
            if ((acopy = ALLOCATE_LOCAL_ARRAY(atype, nArray))) \
                aarg = acopy; \
            GCOP_TOP_PART; \
            if (acopy) \
                memcpy(acopy, pArray, nArray * sizeof(atype)); \
            (*pGC->ops->aop)(pDrawable, pGC, GCOP_ARGS nArray, aarg); \
            GCOP_BOTTOM_PART; \
            DEALLOCATE_LOCAL(acopy); \
            CLIP_RESTORE; \
        } \
        SCREEN_RESTORE; \
    }

/*********************
 * Utility functions *
 *********************/

static int
miBankOf(
    miBankScreenPtr pScreenPriv,
    int             x,
    int             y
)
{
    int iBank = ((x * (int)pScreenPriv->nBankBPP) +
                 (y * (long)pScreenPriv->nBitsPerScanline)) /
                (long)pScreenPriv->nBitsPerBank;

    if (iBank < 0)
        iBank = 0;
    else if (iBank >= pScreenPriv->nBanks)
        iBank = pScreenPriv->nBanks - 1;

    return iBank;
}

#define FirstBankOf(_x, _y) miBankOf(pScreenPriv, (_x), (_y))
#define  LastBankOf(_x, _y) miBankOf(pScreenPriv, (_x) - 1, (_y))

/* Determine banking type from the BankInfoRec */
static unsigned int
miBankDeriveType(
    ScreenPtr     pScreen,
    miBankInfoPtr pBankInfo
)
{
    unsigned int type;

    if (pBankInfo->pBankA == pBankInfo->pBankB)
    {
        if (pBankInfo->SetSourceBank == pBankInfo->SetDestinationBank)
        {
            if (pBankInfo->SetSourceAndDestinationBanks !=
                pBankInfo->SetSourceBank)
                return BANK_NOBANK;

            type = BANK_SINGLE;
        }
        else
        {
            if (pBankInfo->SetSourceAndDestinationBanks ==
                pBankInfo->SetDestinationBank)
                return BANK_NOBANK;
            if (pBankInfo->SetSourceAndDestinationBanks ==
                pBankInfo->SetSourceBank)
                return BANK_NOBANK;

            type = BANK_SHARED;
        }
    }
    else
    {
        if ((unsigned long)abs((char *)pBankInfo->pBankA -
                               (char *)pBankInfo->pBankB) < pBankInfo->BankSize)
            return BANK_NOBANK;

        if (pBankInfo->SetSourceBank == pBankInfo->SetDestinationBank)
        {
            if (pBankInfo->SetSourceAndDestinationBanks !=
                pBankInfo->SetSourceBank)
                return BANK_NOBANK;
        }
        else
        {
            if (pBankInfo->SetSourceAndDestinationBanks ==
                pBankInfo->SetDestinationBank)
                return BANK_NOBANK;
        }

        type = BANK_DOUBLE;
    }

    /*
     * Internal limitation:  Currently, only single banking is supported when
     * the pixmap format and the screen's pixel format are different.  The
     * following test is only partially successful at detecting this condition.
     */
    if (pBankInfo->nBankDepth != pScreen->rootDepth)
        type = BANK_SINGLE;

    return type;
}

/* Least common multiple */
static unsigned int
miLCM(
    unsigned int x,
    unsigned int y
)
{
    unsigned int m = x, n = y, o;

    while ((o = m % n))
    {
        m = n;
        n = o;
    }

    return (x / n) * y;
}

/******************
 * GCOps wrappers *
 ******************/

static void
miBankFillSpans(
    DrawablePtr pDrawable,
    GCPtr       pGC,
    int         nArray,
    DDXPointPtr pptInit,
    int         *pwidthInit,
    int         fSorted
)
{
    GCOP_SIMPLE((*pGC->ops->FillSpans)(pDrawable, pGC,
        nArray, pptInit, pwidthInit, fSorted));
}

static void
miBankSetSpans(
    DrawablePtr pDrawable,
    GCPtr       pGC,
    char        *psrc,
    DDXPointPtr ppt,
    int         *pwidth,
    int         nArray,
    int         fSorted
)
{
    GCOP_SIMPLE((*pGC->ops->SetSpans)(pDrawable, pGC, psrc,
        ppt, pwidth, nArray, fSorted));
}

static void
miBankPutImage(
    DrawablePtr pDrawable,
    GCPtr       pGC,
    int         depth,
    int         x,
    int         y,
    int         w,
    int         h,
    int         leftPad,
    int         format,
    char        *pImage
)
{
    if ((w > 0) && (h > 0))
    {
        GCOP_INIT;
        SCREEN_SAVE;

        if (!IS_BANKED(pDrawable))
        {
            GCOP_UNWRAP;

            (*pGC->ops->PutImage)(pDrawable, pGC, depth, x, y, w, h,
                leftPad, format, pImage);

            GCOP_WRAP;
        }
        else
        {
            int i, j;

            CLIP_SAVE;

            i = FirstBankOf(x + pDrawable->x,     y + pDrawable->y);
            j =  LastBankOf(x + pDrawable->x + w, y + pDrawable->y + h);
            for (;  i <= j;  i++)
            {
                if (!(pGC->pCompositeClip = pGCPriv->pBankedClips[i]))
                    continue;

                GCOP_UNWRAP;

                SET_SINGLE_BANK(pScreenPriv->pScreenPixmap, -1, -1, i);

                (*pGC->ops->PutImage)(pDrawable, pGC, depth, x, y, w, h,
                    leftPad, format, pImage);

                GCOP_WRAP;
            }

            CLIP_RESTORE;
        }

        SCREEN_RESTORE;
    }
}

/*
 * Here the CopyArea/CopyPlane wrappers.  First off, we have to clip against
 * the source in order to make the minimal number of copies in case of slow
 * systems.  Also the exposure handling is quite tricky.  Special attention
 * is to be given to the way the copies are sequenced.  The list of boxes after
 * the source clip is used to build a workqueue, that contains the atomic
 * copies (i.e. only from one bank to one bank).  Doing so produces a minimal
 * list of things to do.
 */
static RegionPtr
miBankCopy(
    DrawablePtr   pSrc,
    DrawablePtr   pDst,
    GCPtr         pGC,
    int           srcx,
    int           srcy,
    int           w,
    int           h,
    int           dstx,
    int           dsty,
    unsigned long plane,
    Bool          SinglePlane
)
{
    int         cx1, cy1, cx2, cy2;
    int         ns, nd, nse, nde, dx, dy, xorg = 0, yorg = 0;
    int         maxWidth = 0, maxHeight = 0, paddedWidth = 0;
    int         nBox, nBoxClipSrc, nBoxClipDst, nQueue;
    BoxPtr      pBox, pBoxClipSrc, pBoxClipDst;
    BoxRec      fastBox, ccBox;
    RegionPtr   ret = NULL, prgnSrcClip = NULL;
    RegionRec   rgnDst;
    char        *pImage = NULL;
    miBankQueue *pQueue, *pQueueNew, *Queue;
    miBankQueue *pQueueTmp, *pQueueNext, *pQueueBase;
    Bool        fastBlit, freeSrcClip, fastClip;
    Bool        fExpose = FALSE, fastExpose = FALSE;

    GCOP_INIT;
    SCREEN_SAVE;

    if (!IS_BANKED(pSrc) && !IS_BANKED(pDst))
    {
        GCOP_UNWRAP;

        if (SinglePlane)
            ret = (*pGC->ops->CopyPlane)(pSrc, pDst, pGC,
                srcx, srcy, w, h, dstx, dsty, plane);
        else
            ret = (*pGC->ops->CopyArea)(pSrc, pDst, pGC,
                srcx, srcy, w, h, dstx, dsty);

        GCOP_WRAP;
    }
    else if (!IS_BANKED(pDst))
    {
        fExpose = pGC->fExpose;
        pGC->fExpose = FALSE;

        xorg = pSrc->x;
        yorg = pSrc->y;
        dx   = dstx - srcx;
        dy   = dsty - srcy;
        srcx += xorg;
        srcy += yorg;

        ns = FirstBankOf(srcx,     srcy);
        nse = LastBankOf(srcx + w, srcy + h);
        for (;  ns <= nse;  ns++)
        {
            if (!pScreenPriv->pBanks[ns])
                continue;

            nBox = REGION_NUM_RECTS(pScreenPriv->pBanks[ns]);
            pBox = REGION_RECTS(pScreenPriv->pBanks[ns]);

            for (;  nBox--;  pBox++)
            {
                cx1 = max(pBox->x1, srcx);
                cy1 = max(pBox->y1, srcy);
                cx2 = min(pBox->x2, srcx + w);
                cy2 = min(pBox->y2, srcy + h);

                if ((cx1 >= cx2) || (cy1 >= cy2))
                    continue;

                GCOP_UNWRAP;

                SET_SINGLE_BANK(pScreenPriv->pScreenPixmap, -1, -1, ns);

                if (SinglePlane)
                    (*pGC->ops->CopyPlane)(pSrc, pDst, pGC,
                        cx1 - xorg, cy1 - yorg,
                        cx2 - cx1, cy2 - cy1,
                        cx1 + dx - xorg, cy1 + dy - yorg, plane);
                else
                    (*pGC->ops->CopyArea)(pSrc, pDst, pGC,
                        cx1 - xorg, cy1 - yorg,
                        cx2 - cx1, cy2 - cy1,
                        cx1 + dx - xorg, cy1 + dy - yorg);

                GCOP_WRAP;
            }
        }

        pGC->fExpose = fExpose;
        srcx -= xorg;
        srcy -= yorg;
    }
    else if (!IS_BANKED(pSrc))
    {
        CLIP_SAVE;

        if (pGC->miTranslate)
        {
            xorg = pDst->x;
            yorg = pDst->y;
        }
        dx = srcx - dstx;
        dy = srcy - dsty;
        dstx += xorg;
        dsty += yorg;

        nd = FirstBankOf(dstx,     dsty);
        nde = LastBankOf(dstx + w, dsty + h);
        for (;  nd <= nde;  nd++)
        {
            if (!(pGC->pCompositeClip = pGCPriv->pBankedClips[nd]))
                continue;

            /*
             * It's faster to let the lower-level CopyArea do the clipping
             * within each bank.
             */
            nBox = REGION_NUM_RECTS(pScreenPriv->pBanks[nd]);
            pBox = REGION_RECTS(pScreenPriv->pBanks[nd]);

            for (;  nBox--;  pBox++)
            {
                cx1 = max(pBox->x1, dstx);
                cy1 = max(pBox->y1, dsty);
                cx2 = min(pBox->x2, dstx + w);
                cy2 = min(pBox->y2, dsty + h);

                if ((cx1 >= cx2) || (cy1 >= cy2))
                    continue;

                GCOP_UNWRAP;

                SET_SINGLE_BANK(pScreenPriv->pScreenPixmap, -1, -1, nd);

                if (SinglePlane)
                    (*pGC->ops->CopyPlane)(pSrc, pDst, pGC,
                        cx1 + dx - xorg, cy1 + dy - yorg,
                        cx2 - cx1, cy2 - cy1,
                        cx1 - xorg, cy1 - yorg, plane);
                else
                    (*pGC->ops->CopyArea)(pSrc, pDst, pGC,
                        cx1 + dx - xorg, cy1 + dy - yorg,
                        cx2 - cx1, cy2 - cy1,
                        cx1 - xorg, cy1 - yorg);

                GCOP_WRAP;
            }
        }

        CLIP_RESTORE;
    }
    else /* IS_BANKED(pSrc) && IS_BANKED(pDst) */
    {
        CLIP_SAVE;

        fExpose = pGC->fExpose;

        fastBox.x1 = srcx + pSrc->x;
        fastBox.y1 = srcy + pSrc->y;
        fastBox.x2 = fastBox.x1 + w;
        fastBox.y2 = fastBox.y1 + h;

        dx = dstx - fastBox.x1;
        dy = dsty - fastBox.y1;
        if (pGC->miTranslate)
        {
            xorg = pDst->x;
            yorg = pDst->y;
        }

        /*
         * Clip against the source.  Otherwise we will blit too much for SINGLE
         * and SHARED banked systems.
         */
        freeSrcClip = FALSE;
        fastClip    = FALSE;
        fastExpose  = FALSE;

        if (pGC->subWindowMode != IncludeInferiors)
            prgnSrcClip = &((WindowPtr)pSrc)->clipList;
        else if (!((WindowPtr)pSrc)->parent)
            fastClip = TRUE;
        else if ((pSrc == pDst) && (pGC->clientClipType == CT_NONE))
            prgnSrcClip = pGC->pCompositeClip;
        else
        {
            prgnSrcClip = NotClippedByChildren((WindowPtr)pSrc);
            freeSrcClip = TRUE;
        }

        if (fastClip)
        {
            fastExpose = TRUE;

            /*
             * Clip the source.  If regions extend beyond the source size, make
             * sure exposure events get sent.
             */
            if (fastBox.x1 < pSrc->x)
            {
                fastBox.x1 = pSrc->x;
                fastExpose = FALSE;
            }
            if (fastBox.y1 < pSrc->y)
            {
                fastBox.y1 = pSrc->y;
                fastExpose = FALSE;
            }
            if (fastBox.x2 > pSrc->x + (int) pSrc->width)
            {
                fastBox.x2 = pSrc->x + (int) pSrc->width;
                fastExpose = FALSE;
            }
            if (fastBox.y2 > pSrc->y + (int) pSrc->height)
            {
                fastBox.y2 = pSrc->y + (int) pSrc->height;
                fastExpose = FALSE;
            }

            nBox = 1;
            pBox = &fastBox;
        }
        else
        {
            REGION_INIT(pScreen, &rgnDst, &fastBox, 1);
            REGION_INTERSECT(pScreen, &rgnDst, &rgnDst, prgnSrcClip);
            pBox = REGION_RECTS(&rgnDst);
            nBox = REGION_NUM_RECTS(&rgnDst);
        }

        /*
         * fastBlit can only be TRUE if we don't need to worry about attempts
         * to read partial pixels through the destination bank.
         */
        if (SinglePlane)
            fastBlit = pGCPriv->fastPlane;
        else
            fastBlit = pGCPriv->fastCopy;

        nQueue = nBox * pScreenPriv->maxRects * 2;
        pQueue = Queue = ALLOCATE_LOCAL_ARRAY(miBankQueue, nQueue);

        if (Queue)
        {
            for (;  nBox--;  pBox++)
            {
                ns = FirstBankOf(pBox->x1, pBox->y1);
                nse = LastBankOf(pBox->x2, pBox->y2);
                for (;  ns <= nse;  ns++)
                {
                    if (!pScreenPriv->pBanks[ns])
                        continue;

                    nBoxClipSrc = REGION_NUM_RECTS(pScreenPriv->pBanks[ns]);
                    pBoxClipSrc = REGION_RECTS(pScreenPriv->pBanks[ns]);

                    for (;  nBoxClipSrc--;  pBoxClipSrc++)
                    {
                        cx1 = max(pBox->x1, pBoxClipSrc->x1);
                        cy1 = max(pBox->y1, pBoxClipSrc->y1);
                        cx2 = min(pBox->x2, pBoxClipSrc->x2);
                        cy2 = min(pBox->y2, pBoxClipSrc->y2);

                        /* Check to see if the region is empty */
                        if ((cx1 >= cx2) || (cy1 >= cy2))
                            continue;

                        /* Translate c[xy]* to destination coordinates */
                        cx1 += dx + xorg;
                        cy1 += dy + yorg;
                        cx2 += dx + xorg;
                        cy2 += dy + yorg;

                        nd = FirstBankOf(cx1, cy1);
                        nde = LastBankOf(cx2, cy2);
                        for (;  nd <= nde;  nd++)
                        {
                            if (!pGCPriv->pBankedClips[nd])
                                continue;

                            /*
                             * Clients can send quite large clip descriptions,
                             * so use the bank clips here instead.
                             */
                            nBoxClipDst =
                                REGION_NUM_RECTS(pScreenPriv->pBanks[nd]);
                            pBoxClipDst =
                                REGION_RECTS(pScreenPriv->pBanks[nd]);

                            for (;  nBoxClipDst--;  pBoxClipDst++)
                            {
                                ccBox.x1 = max(cx1, pBoxClipDst->x1);
                                ccBox.y1 = max(cy1, pBoxClipDst->y1);
                                ccBox.x2 = min(cx2, pBoxClipDst->x2);
                                ccBox.y2 = min(cy2, pBoxClipDst->y2);

                                /* Check to see if the region is empty */
                                if ((ccBox.x1 >= ccBox.x2) ||
                                    (ccBox.y1 >= ccBox.y2))
                                    continue;

                                pQueue->srcBankNo = ns;
                                pQueue->dstBankNo = nd;
                                pQueue->x         = ccBox.x1 - xorg;
                                pQueue->y         = ccBox.y1 - yorg;
                                pQueue->w         = ccBox.x2 - ccBox.x1;
                                pQueue->h         = ccBox.y2 - ccBox.y1;

                                if (maxWidth < pQueue->w)
                                    maxWidth = pQueue->w;
                                if (maxHeight < pQueue->h)
                                    maxHeight = pQueue->h;

                                /*
                                 * When shared banking is used and the source
                                 * and destination banks differ, prevent
                                 * attempts to fetch partial scanline pad units
                                 * through the destination bank.
                                 */
                                pQueue->fastBlit = fastBlit;
                                if (fastBlit &&
                                    (pScreenPriv->type == BANK_SHARED) &&
                                    (ns != nd) &&
                                    ((ccBox.x1 %
                                      pScreenPriv->nPixelsPerScanlinePadUnit) ||
                                     (ccBox.x2 %
                                      pScreenPriv->nPixelsPerScanlinePadUnit) ||
                                     (RECT_IN_REGION(pScreen,
                                       pGCPriv->pBankedClips[nd], &ccBox) !=
                                      rgnIN)))
                                    pQueue->fastBlit = FALSE;
                                pQueue++;
                            }
                        }
                    }
                }
            }
        }

        if (!fastClip)
        {
            REGION_UNINIT(pScreen, &rgnDst);
            if (freeSrcClip)
                REGION_DESTROY(pScreen, prgnSrcClip);
        }

        pQueueNew = pQueue;
        nQueue = pQueue - Queue;

        if (nQueue > 0)
        {
            BANK_SAVE;

            pQueue = Queue;

            if ((nQueue > 1) &&
                ((pSrc == pDst) || (pGC->subWindowMode == IncludeInferiors)))
            {
                if ((srcy + pSrc->y) < (dsty + yorg))
                {
                    /* Sort from bottom to top */
                    pQueueBase = pQueueNext = pQueue + nQueue - 1;

                    while (pQueueBase >= pQueue)
                    {
                        while ((pQueueNext >= pQueue) &&
                               (pQueueBase->y == pQueueNext->y))
                            pQueueNext--;

                        pQueueTmp = pQueueNext + 1;
                        while (pQueueTmp <= pQueueBase)
                            *pQueueNew++ = *pQueueTmp++;

                        pQueueBase = pQueueNext;
                    }

                    pQueueNew -= nQueue;
                    pQueue = pQueueNew;
                    pQueueNew = Queue;
                }

                if ((srcx + pSrc->x) < (dstx + xorg))
                {
                    /* Sort from right to left */
                    pQueueBase = pQueueNext = pQueue;

                    while (pQueueBase < pQueue + nQueue)
                    {
                        while ((pQueueNext < pQueue + nQueue) &&
                               (pQueueNext->y == pQueueBase->y))
                            pQueueNext++;

                        pQueueTmp = pQueueNext;
                        while (pQueueTmp != pQueueBase)
                            *pQueueNew++ = *--pQueueTmp;

                        pQueueBase = pQueueNext;
                    }

                    pQueueNew -= nQueue;
                    pQueue = pQueueNew;
                }
            }

            paddedWidth = PixmapBytePad(maxWidth,
                pScreenPriv->pScreenPixmap->drawable.depth);
            pImage = (char *)ALLOCATE_LOCAL(paddedWidth * maxHeight);

            pGC->fExpose = FALSE;

            while (nQueue--)
            {
                pGC->pCompositeClip = pGCPriv->pBankedClips[pQueue->dstBankNo];

                GCOP_UNWRAP;

                if (pQueue->srcBankNo == pQueue->dstBankNo)
                {
                    SET_SINGLE_BANK(pScreenPriv->pScreenPixmap,
                        -1, -1, pQueue->srcBankNo);

                    if (SinglePlane)
                        (*pGC->ops->CopyPlane)(pSrc, pDst, pGC,
                            pQueue->x - dx - pSrc->x, pQueue->y - dy - pSrc->y,
                            pQueue->w, pQueue->h, pQueue->x, pQueue->y, plane);
                    else
                        (*pGC->ops->CopyArea)(pSrc, pDst, pGC,
                            pQueue->x - dx - pSrc->x, pQueue->y - dy - pSrc->y,
                            pQueue->w, pQueue->h, pQueue->x, pQueue->y);
                }
                else if (pQueue->fastBlit)
                {
                    SET_SOURCE_BANK     (pScreenPriv->pBankPixmap,
                        pScreenPriv->pScreenPixmap->drawable.width,
                        pScreenPriv->pScreenPixmap->devKind,
                        pQueue->srcBankNo);
                    SET_DESTINATION_BANK(pScreenPriv->pScreenPixmap,
                        -1, -1, pQueue->dstBankNo);

                    if (SinglePlane)
                        (*pGC->ops->CopyPlane)(
                            (DrawablePtr)pScreenPriv->pBankPixmap, pDst, pGC,
                            pQueue->x - dx, pQueue->y - dy,
                            pQueue->w, pQueue->h, pQueue->x, pQueue->y, plane);
                    else
                        (*pGC->ops->CopyArea)(
                            (DrawablePtr)pScreenPriv->pBankPixmap, pDst, pGC,
                            pQueue->x - dx, pQueue->y - dy,
                            pQueue->w, pQueue->h, pQueue->x, pQueue->y);
                }
                else if (pImage)
                {
                    ModifyPixmap(pScreenPriv->pBankPixmap,
                        maxWidth, paddedWidth, pImage);

                    SET_SINGLE_BANK(pScreenPriv->pScreenPixmap,
                        -1, -1, pQueue->srcBankNo);

                    (*pScreenPriv->pBankGC->ops->CopyArea)(
                        pSrc, (DrawablePtr)pScreenPriv->pBankPixmap,
                        pScreenPriv->pBankGC,
                        pQueue->x - dx - pSrc->x, pQueue->y - dy - pSrc->y,
                        pQueue->w, pQueue->h, 0, 0);

                    SET_SINGLE_BANK(pScreenPriv->pScreenPixmap,
                        -1, -1, pQueue->dstBankNo);

                    if (SinglePlane)
                        (*pGC->ops->CopyPlane)(
                            (DrawablePtr)pScreenPriv->pBankPixmap,
                            pDst, pGC, 0, 0, pQueue->w, pQueue->h,
                            pQueue->x, pQueue->y, plane);
                    else
                        (*pGC->ops->CopyArea)(
                            (DrawablePtr)pScreenPriv->pBankPixmap,
                            pDst, pGC, 0, 0, pQueue->w, pQueue->h,
                            pQueue->x, pQueue->y);
                }

                GCOP_WRAP;

                pQueue++;
            }

            DEALLOCATE_LOCAL(pImage);

            BANK_RESTORE;
        }

        CLIP_RESTORE;

        pGC->fExpose = fExpose;

        DEALLOCATE_LOCAL(Queue);
    }

    SCREEN_RESTORE;

    if (!fExpose || fastExpose)
        return ret;

    return miHandleExposures(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, 0);
}

static RegionPtr
miBankCopyArea(
    DrawablePtr pSrc,
    DrawablePtr pDst,
    GCPtr       pGC,
    int         srcx,
    int         srcy,
    int         w,
    int         h,
    int         dstx,
    int         dsty
)
{
    return miBankCopy(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, 0, FALSE);
}

static RegionPtr
miBankCopyPlane(
    DrawablePtr   pSrc,
    DrawablePtr   pDst,
    GCPtr         pGC,
    int           srcx,
    int           srcy,
    int           w,
    int           h,
    int           dstx,
    int           dsty,
    unsigned long plane
)
{
    return
        miBankCopy(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, plane, TRUE);
}

static void
miBankPolyPoint(
    DrawablePtr pDrawable,
    GCPtr       pGC,
    int         mode,
    int         nArray,
    xPoint      *pArray
)
{
#   define GCOP_ARGS GCOP_0D_ARGS
    GCOP_COMPLEX(PolyPoint, xPoint);
#   undef GCOP_ARGS
}

static void
miBankPolylines(
    DrawablePtr pDrawable,
    GCPtr       pGC,
    int         mode,
    int         nArray,
    DDXPointPtr pArray
)
{
#   define GCOP_ARGS GCOP_0D_ARGS
    GCOP_COMPLEX(Polylines, DDXPointRec);
#   undef GCOP_ARGS
}

static void
miBankPolySegment(
    DrawablePtr pDrawable,
    GCPtr       pGC,
    int         nArray,
    xSegment    *pArray
)
{
#   define GCOP_ARGS GCOP_1D_ARGS
    GCOP_COMPLEX(PolySegment, xSegment);
#   undef GCOP_ARGS
}

static void
miBankPolyRectangle(
    DrawablePtr pDrawable,
    GCPtr       pGC,
    int         nArray,
    xRectangle  *pArray
)
{
#   define GCOP_ARGS GCOP_1D_ARGS
    GCOP_COMPLEX(PolyRectangle, xRectangle);
#   undef GCOP_ARGS
}

static void
miBankPolyArc(
    DrawablePtr pDrawable,
    GCPtr       pGC,
    int         nArray,
    xArc        *pArray
)
{
#   define GCOP_ARGS GCOP_1D_ARGS
    GCOP_COMPLEX(PolyArc, xArc);
#   undef GCOP_ARGS
}

static void
miBankFillPolygon(
    DrawablePtr pDrawable,
    GCPtr       pGC,
    int         shape,
    int         mode,
    int         nArray,
    DDXPointRec *pArray
)
{
#   define GCOP_ARGS GCOP_2D_ARGS
    GCOP_COMPLEX(FillPolygon, DDXPointRec);
#   undef GCOP_ARGS
}

static void
miBankPolyFillRect(
    DrawablePtr pDrawable,
    GCPtr       pGC,
    int         nArray,
    xRectangle  *pArray
)
{
#   define GCOP_ARGS GCOP_1D_ARGS
    GCOP_COMPLEX(PolyFillRect, xRectangle);
#   undef GCOP_ARGS
}

static void
miBankPolyFillArc(
    DrawablePtr pDrawable,
    GCPtr       pGC,
    int         nArray,
    xArc        *pArray
)
{
#   define GCOP_ARGS GCOP_1D_ARGS
    GCOP_COMPLEX(PolyFillArc, xArc);
#   undef GCOP_ARGS
}

static int
miBankPolyText8(
    DrawablePtr pDrawable,
    GCPtr       pGC,
    int         x,
    int         y,
    int         nArray,
    char        *pchar
)
{
    int retval = x;

    GCOP_SIMPLE(retval =
        (*pGC->ops->PolyText8)(pDrawable, pGC, x, y, nArray, pchar));

    return retval;
}

static int
miBankPolyText16(
    DrawablePtr    pDrawable,
    GCPtr          pGC,
    int            x,
    int            y,
    int            nArray,
    unsigned short *pchar
)
{
    int retval = x;

    GCOP_SIMPLE(retval =
        (*pGC->ops->PolyText16)(pDrawable, pGC, x, y, nArray, pchar));

    return retval;
}

static void
miBankImageText8(
    DrawablePtr pDrawable,
    GCPtr       pGC,
    int         x,
    int         y,
    int         nArray,
    char        *pchar
)
{
    GCOP_SIMPLE((*pGC->ops->ImageText8)(pDrawable, pGC, x, y, nArray, pchar));
}

static void
miBankImageText16(
    DrawablePtr    pDrawable,
    GCPtr          pGC,
    int            x,
    int            y,
    int            nArray,
    unsigned short *pchar
)
{
    GCOP_SIMPLE((*pGC->ops->ImageText16)(pDrawable, pGC, x, y, nArray, pchar));
}

static void
miBankImageGlyphBlt(
    DrawablePtr  pDrawable,
    GCPtr        pGC,
    int          x,
    int          y,
    unsigned int nArray,
    CharInfoPtr  *ppci,
    pointer      pglyphBase
)
{
    GCOP_SIMPLE((*pGC->ops->ImageGlyphBlt)(pDrawable, pGC,
        x, y, nArray, ppci, pglyphBase));
}

static void
miBankPolyGlyphBlt(
    DrawablePtr  pDrawable,
    GCPtr        pGC,
    int          x,
    int          y,
    unsigned int nArray,
    CharInfoPtr  *ppci,
    pointer      pglyphBase
)
{
    GCOP_SIMPLE((*pGC->ops->PolyGlyphBlt)(pDrawable, pGC,
        x, y, nArray, ppci, pglyphBase));
}

static void
miBankPushPixels(
    GCPtr       pGC,
    PixmapPtr   pBitmap,
    DrawablePtr pDrawable,
    int         w,
    int         h,
    int         x,
    int         y
)
{
    if ((w > 0) && (h > 0))
    {
        GCOP_INIT;
        SCREEN_SAVE;

        if (!IS_BANKED(pDrawable))
        {
            GCOP_UNWRAP;

            (*pGC->ops->PushPixels)(pGC, pBitmap, pDrawable, w, h, x, y);

            GCOP_WRAP;
        }
        else
        {
            int i, j;

            CLIP_SAVE;

            i = FirstBankOf(x,     y);
            j =  LastBankOf(x + w, y + h);
            for (;  i <= j;  i++)
            {
                if (!(pGC->pCompositeClip = pGCPriv->pBankedClips[i]))
                    continue;

                GCOP_UNWRAP;

                SET_SINGLE_BANK(pScreenPriv->pScreenPixmap, -1, -1, i);

                (*pGC->ops->PushPixels)(pGC, pBitmap, pDrawable, w, h, x, y);

                GCOP_WRAP;
            }

            CLIP_RESTORE;
        }

        SCREEN_RESTORE;
    }
}

static GCOps miBankGCOps =
{
    miBankFillSpans,
    miBankSetSpans,
    miBankPutImage,
    miBankCopyArea,
    miBankCopyPlane,
    miBankPolyPoint,
    miBankPolylines,
    miBankPolySegment,
    miBankPolyRectangle,
    miBankPolyArc,
    miBankFillPolygon,
    miBankPolyFillRect,
    miBankPolyFillArc,
    miBankPolyText8,
    miBankPolyText16,
    miBankImageText8,
    miBankImageText16,
    miBankImageGlyphBlt,
    miBankPolyGlyphBlt,
    miBankPushPixels,
#ifdef NEED_LINEHELPER
    NULL,               /* LineHelper */
#endif
    {NULL}              /* devPrivate */
};

/********************
 * GCFuncs wrappers *
 ********************/

static void
miBankValidateGC(
    GCPtr         pGC,
    unsigned long changes,
    DrawablePtr   pDrawable
)
{
    GC_INIT(pGC);
    GC_UNWRAP(pGC);

    (*pGC->funcs->ValidateGC)(pGC, changes, pDrawable);

    if ((changes & (GCClipXOrigin|GCClipYOrigin|GCClipMask|GCSubwindowMode)) ||
        (pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS)))
    {
        ScreenPtr     pScreen = pGC->pScreen;
        RegionPtr     prgnClip;
        unsigned long planemask;
        int           i;

        SCREEN_INIT;
        SCREEN_SAVE;

        if (IS_BANKED(pDrawable))
        {
            for (i = 0;  i < pScreenPriv->nBanks;  i++)
            {
                if (!pScreenPriv->pBanks[i])
                    continue;

                if (!(prgnClip = pGCPriv->pBankedClips[i]))
                    prgnClip = REGION_CREATE(pScreen, NULL, 1);

                REGION_INTERSECT(pScreen, prgnClip,
                    pScreenPriv->pBanks[i], pGC->pCompositeClip);

                if ((REGION_NUM_RECTS(prgnClip) <= 1) &&
                    ((prgnClip->extents.x1 == prgnClip->extents.x2) ||
                     (prgnClip->extents.y1 == prgnClip->extents.y2)))
                {
                    REGION_DESTROY(pScreen, prgnClip);
                    pGCPriv->pBankedClips[i] = NULL;
                }
                else
                    pGCPriv->pBankedClips[i] = prgnClip;
            }

            /*
             * fastCopy and fastPlane can only be TRUE if we don't need to
             * worry about attempts to read partial pixels through the
             * destination bank.
             */
            switch (pScreenPriv->type)
            {
                case BANK_SHARED:
                    pGCPriv->fastCopy = pGCPriv->fastPlane = FALSE;

                    if ((pGC->alu != GXclear) && (pGC->alu != GXcopy) &&
                        (pGC->alu != GXcopyInverted) && (pGC->alu != GXset))
                        break;

                    if (pScreen->rootDepth == 1)
                        pGCPriv->fastPlane = TRUE;

                    /* This is probably paranoia */
                    if ((pDrawable->depth != pScreen->rootDepth) ||
                        (pDrawable->depth != pGC->depth))
                        break;

                    planemask = (1 << pGC->depth) - 1;
                    if ((pGC->planemask & planemask) == planemask)
                        pGCPriv->fastCopy = TRUE;

                    break;

                case BANK_DOUBLE:
                    pGCPriv->fastCopy = pGCPriv->fastPlane = TRUE;
                    break;

                default:
                    pGCPriv->fastCopy = pGCPriv->fastPlane = FALSE;
                    break;
            }
        }
        else
        {
            /*
             * Here we are on a pixmap and don't need all that special clipping
             * stuff, hence free it.
             */
            for (i = 0;  i < pScreenPriv->nBanks;  i++)
            {
                if (!pGCPriv->pBankedClips[i])
                    continue;

                REGION_DESTROY(pScreen, pGCPriv->pBankedClips[i]);
                pGCPriv->pBankedClips[i] = NULL;
            }
        }

        SCREEN_RESTORE;
    }

    GC_WRAP(pGC);
}

static void
miBankChangeGC(
    GCPtr         pGC,
    unsigned long mask
)
{
    GC_INIT(pGC);
    GC_UNWRAP(pGC);

    (*pGC->funcs->ChangeGC)(pGC, mask);

    GC_WRAP(pGC);
}

static void
miBankCopyGC(
    GCPtr         pGCSrc,
    unsigned long mask,
    GCPtr         pGCDst
)
{
    GC_INIT(pGCDst);
    GC_UNWRAP(pGCDst);

    (*pGCDst->funcs->CopyGC)(pGCSrc, mask, pGCDst);

    GC_WRAP(pGCDst);
}

static void
miBankDestroyGC(
    GCPtr pGC
)
{
    ScreenPtr pScreen = pGC->pScreen;
    int       i;

    SCREEN_INIT;
    GC_INIT(pGC);
    GC_UNWRAP(pGC);

    (*pGC->funcs->DestroyGC)(pGC);

    for (i = 0;  i < pScreenPriv->nBanks;  i++)
    {
        if (!pGCPriv->pBankedClips[i])
            continue;

        REGION_DESTROY(pScreen, pGCPriv->pBankedClips[i]);
        pGCPriv->pBankedClips[i] = NULL;
    }

    GC_WRAP(pGC);
}

static void
miBankChangeClip(
    GCPtr   pGC,
    int     type,
    pointer pvalue,
    int     nrects
)
{
    GC_INIT(pGC);
    GC_UNWRAP(pGC);

    (*pGC->funcs->ChangeClip)(pGC, type, pvalue, nrects);

    GC_WRAP(pGC);
}

static void
miBankDestroyClip(
    GCPtr pGC
)
{
    GC_INIT(pGC);
    GC_UNWRAP(pGC);

    (*pGC->funcs->DestroyClip)(pGC);

    GC_WRAP(pGC);
}

static void
miBankCopyClip(
    GCPtr pGCDst,
    GCPtr pGCSrc
)
{
    GC_INIT(pGCDst);
    GC_UNWRAP(pGCDst);

    (*pGCDst->funcs->CopyClip)(pGCDst, pGCSrc);

    GC_WRAP(pGCDst);
}

static GCFuncs miBankGCFuncs =
{
    miBankValidateGC,
    miBankChangeGC,
    miBankCopyGC,
    miBankDestroyGC,
    miBankChangeClip,
    miBankDestroyClip,
    miBankCopyClip
};

/*******************
 * Screen Wrappers *
 *******************/

static Bool
miBankCreateScreenResources(
    ScreenPtr pScreen
)
{
    Bool retval;

    SCREEN_INIT;
    SCREEN_UNWRAP(CreateScreenResources);

    if ((retval = (*pScreen->CreateScreenResources)(pScreen)))
    {
        /* Set screen buffer address to something recognizable */
        pScreenPriv->pScreenPixmap = (*pScreen->GetScreenPixmap)(pScreen);
        pScreenPriv->pbits = pScreenPriv->pScreenPixmap->devPrivate.ptr;
        pScreenPriv->pScreenPixmap->devPrivate.ptr = (pointer)pScreenPriv;

        /* Get shadow pixmap;  width & height of 0 means no pixmap data */
        pScreenPriv->pBankPixmap = (*pScreen->CreatePixmap)(pScreen, 0, 0,
            pScreenPriv->pScreenPixmap->drawable.depth);
        if (!pScreenPriv->pBankPixmap)
            retval = FALSE;
    }

    /* Shadow the screen */
    if (retval)
        retval = (*pScreen->ModifyPixmapHeader)(pScreenPriv->pBankPixmap,
            pScreenPriv->pScreenPixmap->drawable.width,
            pScreenPriv->pScreenPixmap->drawable.height,
            pScreenPriv->pScreenPixmap->drawable.depth,
            pScreenPriv->pScreenPixmap->drawable.bitsPerPixel,
            pScreenPriv->pScreenPixmap->devKind, NULL);

    /* Create shadow GC */
    if (retval)
    {
        pScreenPriv->pBankGC = CreateScratchGC(pScreen,
            pScreenPriv->pBankPixmap->drawable.depth);
        if (!pScreenPriv->pBankGC)
            retval = FALSE;
    }

    /* Validate shadow GC */
    if (retval)
    {
        pScreenPriv->pBankGC->graphicsExposures = FALSE;
        pScreenPriv->pBankGC->subWindowMode = IncludeInferiors;
        ValidateGC((DrawablePtr)pScreenPriv->pBankPixmap,
            pScreenPriv->pBankGC);
    }

    SCREEN_WRAP(CreateScreenResources, miBankCreateScreenResources);

    return retval;
}

static Bool
miBankModifyPixmapHeader(
    PixmapPtr pPixmap,
    int       width,
    int       height,
    int       depth,
    int       bitsPerPixel,
    int       devKind,
    pointer   pPixData
)
{
    Bool retval = FALSE;

    if (pPixmap)
    {
        ScreenPtr pScreen = pPixmap->drawable.pScreen;

        SCREEN_INIT;
        PIXMAP_SAVE(pPixmap);
        SCREEN_UNWRAP(ModifyPixmapHeader);

        retval = (*pScreen->ModifyPixmapHeader)(pPixmap, width, height,
            depth, bitsPerPixel, devKind, pPixData);

        SCREEN_WRAP(ModifyPixmapHeader, miBankModifyPixmapHeader);

        if (pbits == (pointer)pScreenPriv)
        {
            pScreenPriv->pbits = pPixmap->devPrivate.ptr;
            pPixmap->devPrivate.ptr = pbits;
        }
    }

    return retval;
}

static Bool
miBankCloseScreen(
    int       nIndex,
    ScreenPtr pScreen
)
{
    int i;

    SCREEN_INIT;

    /* Free shadow GC */
    FreeScratchGC(pScreenPriv->pBankGC);

    /* Free shadow pixmap */
    (*pScreen->DestroyPixmap)(pScreenPriv->pBankPixmap);

    /* Restore screen pixmap devPrivate pointer */
    pScreenPriv->pScreenPixmap->devPrivate.ptr = pScreenPriv->pbits;

    /* Delete bank clips */
    for (i = 0;  i < pScreenPriv->nBanks;  i++)
        if (pScreenPriv->pBanks[i])
            REGION_DESTROY(pScreen, pScreenPriv->pBanks[i]);

    Xfree(pScreenPriv->pBanks);

    SCREEN_UNWRAP(CreateScreenResources);
    SCREEN_UNWRAP(ModifyPixmapHeader);
    SCREEN_UNWRAP(CloseScreen);
    SCREEN_UNWRAP(GetImage);
    SCREEN_UNWRAP(GetSpans);
    SCREEN_UNWRAP(CreateGC);
    SCREEN_UNWRAP(PaintWindowBackground);
    SCREEN_UNWRAP(PaintWindowBorder);
    SCREEN_UNWRAP(CopyWindow);
    SCREEN_UNWRAP(BackingStoreFuncs);

    Xfree(pScreenPriv);
    return (*pScreen->CloseScreen)(nIndex, pScreen);
}

static void
miBankGetImage(
    DrawablePtr   pDrawable,
    int           sx,
    int           sy,
    int           w,
    int           h,
    unsigned int  format,
    unsigned long planemask,
    char          *pImage
)
{
    if ((w > 0) && (h > 0))
    {
        ScreenPtr pScreen = pDrawable->pScreen;

        SCREEN_INIT;
        SCREEN_STATUS;
        SCREEN_UNWRAP(GetImage);

        if (!IS_BANKED(pDrawable))
        {
            (*pScreen->GetImage)(pDrawable, sx, sy, w, h,
                format, planemask, pImage);
        }
        else
        {
            int  paddedWidth;
            char *pBankImage;

            paddedWidth = PixmapBytePad(w,
                pScreenPriv->pScreenPixmap->drawable.depth);
            pBankImage = (char *)ALLOCATE_LOCAL(paddedWidth * h);

            if (pBankImage)
            {
                BANK_SAVE;

                ModifyPixmap(pScreenPriv->pBankPixmap, w, paddedWidth,
                    pBankImage);

                (*pScreenPriv->pBankGC->ops->CopyArea)(
                    (DrawablePtr)WindowTable[pScreen->myNum],
                    (DrawablePtr)pScreenPriv->pBankPixmap,
                    pScreenPriv->pBankGC,
                    sx + pDrawable->x, sy + pDrawable->y, w, h, 0, 0);

                (*pScreen->GetImage)((DrawablePtr)pScreenPriv->pBankPixmap,
                    0, 0, w, h, format, planemask, pImage);

                BANK_RESTORE;

                DEALLOCATE_LOCAL(pBankImage);
            }
        }

        SCREEN_WRAP(GetImage, miBankGetImage);
    }
}

static void
miBankGetSpans(
    DrawablePtr pDrawable,
    int         wMax,
    DDXPointPtr ppt,
    int         *pwidth,
    int         nspans,
    char        *pImage
)
{
    if (nspans > 0)
    {
        ScreenPtr pScreen = pDrawable->pScreen;

        SCREEN_INIT;
        SCREEN_STATUS;
        SCREEN_UNWRAP(GetSpans);

        if (!IS_BANKED(pDrawable))
        {
            (*pScreen->GetSpans)(pDrawable, wMax, ppt, pwidth, nspans, pImage);
        }
        else
        {
            char        *pBankImage;
            int         paddedWidth;
            DDXPointRec pt;

            pt.x = pt.y = 0;

            paddedWidth =
                PixmapBytePad(pScreenPriv->pScreenPixmap->drawable.width,
                    pScreenPriv->pScreenPixmap->drawable.depth);
            pBankImage = (char *)ALLOCATE_LOCAL(paddedWidth);

            if (pBankImage)
            {
                BANK_SAVE;

                ModifyPixmap(pScreenPriv->pBankPixmap,
                    pScreenPriv->pScreenPixmap->drawable.width,
                    paddedWidth, pBankImage);

                for (;  nspans--;  ppt++, pwidth++)
                {
                    if (*pwidth <= 0)
                        continue;

                    (*pScreenPriv->pBankGC->ops->CopyArea)(
                        (DrawablePtr)WindowTable[pScreen->myNum],
                        (DrawablePtr)pScreenPriv->pBankPixmap,
                        pScreenPriv->pBankGC,
                        ppt->x, ppt->y, *pwidth, 1, 0, 0);

                    (*pScreen->GetSpans)((DrawablePtr)pScreenPriv->pBankPixmap,
                        wMax, &pt, pwidth, 1, pImage);

                    pImage = pImage + PixmapBytePad(*pwidth, pDrawable->depth);
                }

                BANK_RESTORE;

                DEALLOCATE_LOCAL(pBankImage);
            }
        }

        SCREEN_WRAP(GetSpans, miBankGetSpans);
    }
}

static Bool
miBankCreateGC(
    GCPtr pGC
)
{
    ScreenPtr   pScreen = pGC->pScreen;
    miBankGCPtr pGCPriv = BANK_GCPRIVATE(pGC);
    Bool        ret;

    SCREEN_INIT;
    SCREEN_UNWRAP(CreateGC);

    if ((ret = (*pScreen->CreateGC)(pGC)))
    {
        pGCPriv->unwrappedOps = &miBankGCOps;
        pGCPriv->unwrappedFuncs = &miBankGCFuncs;
        GC_WRAP(pGC);

        memset(&pGCPriv->pBankedClips, 0,
            pScreenPriv->nBanks * sizeof(pGCPriv->pBankedClips));
    }

    SCREEN_WRAP(CreateGC, miBankCreateGC);

    return ret;
}

static void
miBankPaintWindow(
    WindowPtr pWin,
    RegionPtr pRegion,
    int       what
)
{
    ScreenPtr          pScreen = pWin->drawable.pScreen;
    RegionRec          tmpReg;
    int                i;
    PaintWindowProcPtr PaintWindow;

    SCREEN_INIT;
    SCREEN_SAVE;

    if (what == PW_BORDER)
    {
        SCREEN_UNWRAP(PaintWindowBorder);
        PaintWindow = pScreen->PaintWindowBorder;
    }
    else
    {
        SCREEN_UNWRAP(PaintWindowBackground);
        PaintWindow = pScreen->PaintWindowBackground;
    }

    if (!IS_BANKED(pWin))
    {
        (*PaintWindow)(pWin, pRegion, what);
    }
    else
    {
        REGION_NULL(pScreen, &tmpReg);

        for (i = 0;  i < pScreenPriv->nBanks;  i++)
        {
            if (!pScreenPriv->pBanks[i])
                continue;

            REGION_INTERSECT(pScreen, &tmpReg, pRegion,
                pScreenPriv->pBanks[i]);

            if (REGION_NIL(&tmpReg))
                continue;

            SET_SINGLE_BANK(pScreenPriv->pScreenPixmap, -1, -1, i);

            (*PaintWindow)(pWin, &tmpReg, what);
        }

        REGION_UNINIT(pScreen, &tmpReg);
    }

    if (what == PW_BORDER)
    {
        SCREEN_WRAP(PaintWindowBorder, miBankPaintWindow);
    }
    else
    {
        SCREEN_WRAP(PaintWindowBackground, miBankPaintWindow);
    }

    SCREEN_RESTORE;
}

static void
miBankCopyWindow(
    WindowPtr   pWindow,
    DDXPointRec ptOldOrg,
    RegionPtr   pRgnSrc
)
{
    ScreenPtr   pScreen = pWindow->drawable.pScreen;
    GCPtr       pGC;
    int         dx, dy, nBox;
    DrawablePtr pDrawable = (DrawablePtr)WindowTable[pScreen->myNum];
    RegionPtr   pRgnDst;
    BoxPtr      pBox, pBoxTmp, pBoxNext, pBoxBase, pBoxNew1, pBoxNew2;
    XID         subWindowMode = IncludeInferiors;

    pGC = GetScratchGC(pDrawable->depth, pScreen);

    ChangeGC(pGC, GCSubwindowMode, &subWindowMode);
    ValidateGC(pDrawable, pGC);

    pRgnDst = REGION_CREATE(pScreen, NULL, 1);

    dx = ptOldOrg.x - pWindow->drawable.x;
    dy = ptOldOrg.y - pWindow->drawable.y;
    REGION_TRANSLATE(pScreen, pRgnSrc, -dx, -dy);
    REGION_INTERSECT(pScreen, pRgnDst, &pWindow->borderClip, pRgnSrc);

    pBox = REGION_RECTS(pRgnDst);
    nBox = REGION_NUM_RECTS(pRgnDst);

    pBoxNew1 = NULL;
    pBoxNew2 = NULL;

    if (nBox > 1)
    {
        if (dy < 0)
        {
            /* Sort boxes from bottom to top */
            pBoxNew1 = ALLOCATE_LOCAL_ARRAY(BoxRec, nBox);

            if (pBoxNew1)
            {
                pBoxBase = pBoxNext = pBox + nBox - 1;

                while (pBoxBase >= pBox)
                {
                    while ((pBoxNext >= pBox) &&
                           (pBoxBase->y1 == pBoxNext->y1))
                        pBoxNext--;

                    pBoxTmp = pBoxNext + 1;

                    while (pBoxTmp <= pBoxBase)
                        *pBoxNew1++ = *pBoxTmp++;

                    pBoxBase = pBoxNext;
                }

                pBoxNew1 -= nBox;
                pBox = pBoxNew1;
            }
        }

        if (dx < 0)
        {
            /* Sort boxes from right to left */
            pBoxNew2 = ALLOCATE_LOCAL_ARRAY(BoxRec, nBox);

            if (pBoxNew2)
            {
                pBoxBase = pBoxNext = pBox;

                while (pBoxBase < pBox + nBox)
                {
                    while ((pBoxNext < pBox + nBox) &&
                           (pBoxNext->y1 == pBoxBase->y1))
                        pBoxNext++;

                    pBoxTmp = pBoxNext;

                    while (pBoxTmp != pBoxBase)
                        *pBoxNew2++ = *--pBoxTmp;

                    pBoxBase = pBoxNext;
                }

                pBoxNew2 -= nBox;
                pBox = pBoxNew2;
            }
        }
    }

    while (nBox--)
    {
        (*pGC->ops->CopyArea)(pDrawable, pDrawable, pGC,
            pBox->x1 + dx, pBox->y1 + dy,
            pBox->x2 - pBox->x1, pBox->y2 - pBox->y1,
            pBox->x1, pBox->y1);

        pBox++;
    }

    FreeScratchGC(pGC);

    REGION_DESTROY(pScreen, pRgnDst);

    DEALLOCATE_LOCAL(pBoxNew2);
    DEALLOCATE_LOCAL(pBoxNew1);
}

/**************************
 * Backing store wrappers *
 **************************/

static void
miBankSaveAreas(
    PixmapPtr pPixmap,
    RegionPtr prgnSave,
    int       xorg,
    int       yorg,
    WindowPtr pWin
)
{
    ScreenPtr   pScreen   = pPixmap->drawable.pScreen;
    RegionRec   rgnClipped;
    int         i;

    SCREEN_INIT;
    SCREEN_SAVE;
    SCREEN_UNWRAP(BackingStoreFuncs.SaveAreas);

    if (!IS_BANKED(pWin))
    {
        (*pScreen->BackingStoreFuncs.SaveAreas)(pPixmap, prgnSave, xorg, yorg,
            pWin);
    }
    else
    {
        REGION_NULL(pScreen, &rgnClipped);
        REGION_TRANSLATE(pScreen, prgnSave, xorg, yorg);

        for (i = 0;  i < pScreenPriv->nBanks;  i++)
        {
            if (!pScreenPriv->pBanks[i])
                continue;

            REGION_INTERSECT(pScreen, &rgnClipped,
                prgnSave, pScreenPriv->pBanks[i]);

            if (REGION_NIL(&rgnClipped))
                continue;

            SET_SINGLE_BANK(pScreenPriv->pScreenPixmap, -1, -1, i);

            REGION_TRANSLATE(pScreen, &rgnClipped, -xorg, -yorg);

            (*pScreen->BackingStoreFuncs.SaveAreas)(pPixmap, &rgnClipped,
                xorg, yorg, pWin);
        }

        REGION_TRANSLATE(pScreen, prgnSave, -xorg, -yorg);
        REGION_UNINIT(pScreen, &rgnClipped);
    }

    SCREEN_WRAP(BackingStoreFuncs.SaveAreas, miBankSaveAreas);
    SCREEN_RESTORE;
}

static void
miBankRestoreAreas(
    PixmapPtr pPixmap,
    RegionPtr prgnRestore,
    int       xorg,
    int       yorg,
    WindowPtr pWin
)
{
    ScreenPtr   pScreen   = pPixmap->drawable.pScreen;
    RegionRec   rgnClipped;
    int         i;

    SCREEN_INIT;
    SCREEN_SAVE;
    SCREEN_UNWRAP(BackingStoreFuncs.RestoreAreas);

    if (!IS_BANKED(pWin))
    {
        (*pScreen->BackingStoreFuncs.RestoreAreas)(pPixmap, prgnRestore,
            xorg, yorg, pWin);
    }
    else
    {
        REGION_NULL(pScreen, &rgnClipped);

        for (i = 0;  i < pScreenPriv->nBanks;  i++)
        {
            if (!pScreenPriv->pBanks[i])
                continue;

            REGION_INTERSECT(pScreen, &rgnClipped,
                prgnRestore, pScreenPriv->pBanks[i]);

            if (REGION_NIL(&rgnClipped))
                continue;

            SET_SINGLE_BANK(pScreenPriv->pScreenPixmap, -1, -1, i);

            (*pScreen->BackingStoreFuncs.RestoreAreas)(pPixmap, &rgnClipped,
                xorg, yorg, pWin);
        }

        REGION_UNINIT(pScreen, &rgnClipped);
    }

    SCREEN_WRAP(BackingStoreFuncs.RestoreAreas, miBankRestoreAreas);
    SCREEN_RESTORE;
}

Bool
miInitializeBanking(
    ScreenPtr     pScreen,
    unsigned int  xsize,
    unsigned int  ysize,
    unsigned int  width,
    miBankInfoPtr pBankInfo
)
{
    miBankScreenPtr pScreenPriv;
    unsigned long   nBitsPerBank, nBitsPerScanline, nPixelsPerScanlinePadUnit;
    unsigned long   BankBase, ServerPad;
    unsigned int    type, iBank, nBanks, maxRects, we, nBankBPP;
    int             i;

    if (!pBankInfo || !pBankInfo->BankSize)
        return TRUE;            /* No banking required */

    /* Sanity checks */

    if (!pScreen || !xsize || !ysize || (xsize > width) ||
        !pBankInfo->SetSourceBank || !pBankInfo->SetDestinationBank ||
        !pBankInfo->SetSourceAndDestinationBanks ||
        !pBankInfo->pBankA || !pBankInfo->pBankB ||
        !pBankInfo->nBankDepth)
        return FALSE;

    /*
     * DDX *must* have registered a pixmap format whose depth is
     * pBankInfo->nBankDepth.  This is not necessarily the rootDepth
     * pixmap format.
     */
    i = 0;
    while (screenInfo.formats[i].depth != pBankInfo->nBankDepth)
        if (++i >= screenInfo.numPixmapFormats)
            return FALSE;
    nBankBPP = screenInfo.formats[i].bitsPerPixel;

    i = 0;
    while (screenInfo.formats[i].depth != pScreen->rootDepth)
        if (++i >= screenInfo.numPixmapFormats)
            return FALSE;

    if (nBankBPP > screenInfo.formats[i].bitsPerPixel)
        return FALSE;

    /* Determine banking type */
    if ((type = miBankDeriveType(pScreen, pBankInfo)) == BANK_NOBANK)
        return FALSE;

    /* Internal data */

    nBitsPerBank = pBankInfo->BankSize * 8;
    ServerPad = PixmapBytePad(1, pBankInfo->nBankDepth) * 8;
    if (nBitsPerBank % ServerPad)
        return FALSE;
    nBitsPerScanline = PixmapBytePad(width, pBankInfo->nBankDepth) * 8;
    nBanks = ((nBitsPerScanline * (ysize - 1)) +
              (nBankBPP * xsize) + nBitsPerBank - 1) / nBitsPerBank;
    nPixelsPerScanlinePadUnit = miLCM(ServerPad, nBankBPP) / nBankBPP;

    /* Private areas */

    if (miBankGeneration != serverGeneration)
    {
        if (((miBankScreenIndex = AllocateScreenPrivateIndex()) < 0) ||
            ((miBankGCIndex = AllocateGCPrivateIndex()) < 0))
            return FALSE;

        miBankGeneration = serverGeneration;
    }

    if (!AllocateGCPrivate(pScreen, miBankGCIndex,
        (nBanks * sizeof(RegionPtr)) +
            (sizeof(miBankGCRec) - sizeof(RegionPtr))))
        return FALSE;

    if (!(pScreenPriv = (miBankScreenPtr)Xcalloc(sizeof(miBankScreenRec))))
        return FALSE;

    if (!(pScreenPriv->pBanks =                 /* Allocate and clear */
        (RegionPtr *)Xcalloc(nBanks * sizeof(RegionPtr))))
    {
        Xfree(pScreenPriv);
        return FALSE;
    }

    /*
     * Translate banks into clipping regions which are themselves clipped
     * against the screen.  This also ensures that pixels with imbedded bank
     * boundaries are off-screen.
     */

    BankBase = 0;
    maxRects = 0;
    we = 0;
    for (iBank = 0;  iBank < nBanks;  iBank++)
    {
        xRectangle   pRects[3], *pRect = pRects;
        unsigned int xb, yb, xe, ye;

        xb = ((BankBase + nBankBPP - 1) % nBitsPerScanline) / nBankBPP;
        yb =  (BankBase + nBankBPP - 1) / nBitsPerScanline;
        if (xb >= xsize)
        {
            xb = we = 0;
            yb++;
        }
        if (yb >= ysize)
        {
            we = 0;
            break;
        }

        if (we)
            break;

        BankBase += nBitsPerBank;

        we = (BankBase % nBitsPerScanline) % nBankBPP;
        xe = (BankBase % nBitsPerScanline) / nBankBPP;
        ye =  BankBase / nBitsPerScanline;
        if (xe >= xsize)
        {
            we = xe = 0;
            ye++;
        }
        if (ye >= ysize)
        {
            we = xe = 0;
            ye = ysize;
        }

        if (yb == ye)
        {
            if (xb >= xe)
                continue;

            pRect->x      = xb;
            pRect->y      = yb;
            pRect->width  = xe - xb;
            pRect->height = 1;
            maxRects += 2;
            pRect++;
        }
        else
        {
            if (xb)
            {
                pRect->x      = xb;
                pRect->y      = yb++;
                pRect->width  = xsize - xb;
                pRect->height = 1;
                maxRects += 2;
                pRect++;
            }

            if (yb < ye)
            {
                pRect->x      = 0;
                pRect->y      = yb;
                pRect->width  = xsize;
                pRect->height = ye - yb;
                maxRects += min(pRect->height, 3) + 1;
                pRect++;
            }

            if (xe)
            {
                pRect->x      = 0;
                pRect->y      = ye;
                pRect->width  = xe;
                pRect->height = 1;
                maxRects += 2;
                pRect++;
            }
        }

        pScreenPriv->pBanks[iBank] =
            RECTS_TO_REGION(pScreen, pRect - pRects, pRects, 0);
        if (!pScreenPriv->pBanks[iBank] ||
            REGION_NAR(pScreenPriv->pBanks[iBank]))
        {
            we = 1;
            break;
        }
    }

    if (we && (iBank < nBanks))
    {
        for (i = iBank;  i >= 0;  i--)
            if (pScreenPriv->pBanks[i])
                REGION_DESTROY(pScreen, pScreenPriv->pBanks[i]);

        Xfree(pScreenPriv->pBanks);
        Xfree(pScreenPriv);

        return FALSE;
    }

    /* Open for business */

    pScreenPriv->type = type;
    pScreenPriv->nBanks = nBanks;
    pScreenPriv->maxRects = maxRects;
    pScreenPriv->nBankBPP = nBankBPP;
    pScreenPriv->BankInfo = *pBankInfo;
    pScreenPriv->nBitsPerBank = nBitsPerBank;
    pScreenPriv->nBitsPerScanline = nBitsPerScanline;
    pScreenPriv->nPixelsPerScanlinePadUnit = nPixelsPerScanlinePadUnit;

    SCREEN_WRAP(CreateScreenResources, miBankCreateScreenResources);
    SCREEN_WRAP(ModifyPixmapHeader,    miBankModifyPixmapHeader);
    SCREEN_WRAP(CloseScreen,           miBankCloseScreen);
    SCREEN_WRAP(GetImage,              miBankGetImage);
    SCREEN_WRAP(GetSpans,              miBankGetSpans);
    SCREEN_WRAP(CreateGC,              miBankCreateGC);
    SCREEN_WRAP(PaintWindowBackground, miBankPaintWindow);
    SCREEN_WRAP(PaintWindowBorder,     miBankPaintWindow);
    SCREEN_WRAP(CopyWindow,            miBankCopyWindow);

    pScreenPriv->BackingStoreFuncs     = pScreen->BackingStoreFuncs;

    pScreen->BackingStoreFuncs.SaveAreas      = miBankSaveAreas;
    pScreen->BackingStoreFuncs.RestoreAreas   = miBankRestoreAreas;
    /* ??????????????????????????????????????????????????????????????
    pScreen->BackingStoreFuncs.SetClipmaskRgn = miBankSetClipmaskRgn;
    ?????????????????????????????????????????????????????????????? */

    BANK_SCRPRIVLVAL = (pointer)pScreenPriv;

    return TRUE;
}

/* This is used to force GC revalidation when the banking type is changed */
/*ARGSUSED*/
static int
miBankNewSerialNumber(
    WindowPtr pWin,
    pointer   unused
)
{
    pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    return WT_WALKCHILDREN;
}

/* This entry modifies the banking interface */
Bool
miModifyBanking(
    ScreenPtr     pScreen,
    miBankInfoPtr pBankInfo
)
{
    unsigned int type;

    if (!pScreen)
        return FALSE;

    if (miBankGeneration == serverGeneration)
    {
        SCREEN_INIT;

        if (pScreenPriv)
        {
            if (!pBankInfo || !pBankInfo->BankSize ||
                !pBankInfo->pBankA || !pBankInfo->pBankB ||
                !pBankInfo->SetSourceBank || !pBankInfo->SetDestinationBank ||
                !pBankInfo->SetSourceAndDestinationBanks)
                return FALSE;

            /* BankSize and nBankDepth cannot, as yet, be changed */
            if ((pScreenPriv->BankInfo.BankSize != pBankInfo->BankSize) ||
                (pScreenPriv->BankInfo.nBankDepth != pBankInfo->nBankDepth))
                return FALSE;

            if ((type = miBankDeriveType(pScreen, pBankInfo)) == BANK_NOBANK)
                return FALSE;

            /* Reset banking info */
            pScreenPriv->BankInfo = *pBankInfo;
            if (type != pScreenPriv->type)
            {
                /*
                 * Banking type is changing.  Revalidate all window GC's.
                 */
                pScreenPriv->type = type;
                WalkTree(pScreen, miBankNewSerialNumber, 0);
            }

            return TRUE;
        }
    }

    if (!pBankInfo || !pBankInfo->BankSize)
        return TRUE;                            /* No change requested */

    return FALSE;
}

/*
 * Given various screen attributes, determine the minimum scanline width such
 * that each scanline is server and DDX padded and any pixels with imbedded
 * bank boundaries are off-screen.  This function returns -1 if such a width
 * cannot exist.  This function exists because the DDX needs to be able to
 * determine this width before initializing a frame buffer.
 */
int
miScanLineWidth(
    unsigned int     xsize,         /* pixels */
    unsigned int     ysize,         /* pixels */
    unsigned int     width,         /* pixels */
    unsigned long    BankSize,      /* char's */
    PixmapFormatRec *pBankFormat,
    unsigned int     nWidthUnit     /* bits */
)
{
    unsigned long nBitsPerBank, nBitsPerScanline, nBitsPerScanlinePadUnit;
    unsigned long minBitsPerScanline, maxBitsPerScanline;

    /* Sanity checks */

    if (!nWidthUnit || !pBankFormat)
        return -1;

    nBitsPerBank = BankSize * 8;
    if (nBitsPerBank % pBankFormat->scanlinePad)
        return -1;

    if (xsize > width)
        width = xsize;
    nBitsPerScanlinePadUnit = miLCM(pBankFormat->scanlinePad, nWidthUnit);
    nBitsPerScanline =
        (((width * pBankFormat->bitsPerPixel) + nBitsPerScanlinePadUnit - 1) /
         nBitsPerScanlinePadUnit) * nBitsPerScanlinePadUnit;
    width = nBitsPerScanline / pBankFormat->bitsPerPixel;

    if (!xsize || !(nBitsPerBank % pBankFormat->bitsPerPixel))
        return (int)width;

    /*
     * Scanlines will be server-pad aligned at this point.  They will also be
     * a multiple of nWidthUnit bits long.  Ensure that pixels with imbedded
     * bank boundaries are off-screen.
     *
     * It seems reasonable to limit total frame buffer size to 1/16 of the
     * theoretical maximum address space size.  On a machine with 32-bit
     * addresses (to 8-bit quantities) this turns out to be 256MB.  Not only
     * does this provide a simple limiting condition for the loops below, but
     * it also prevents unsigned long wraparounds.
     */
    if (!ysize)
        return -1;

    minBitsPerScanline = xsize * pBankFormat->bitsPerPixel;
    if (minBitsPerScanline > nBitsPerBank)
        return -1;

    if (ysize == 1)
        return (int)width;

    maxBitsPerScanline =
        (((unsigned long)(-1) >> 1) - minBitsPerScanline) / (ysize - 1);
    while (nBitsPerScanline <= maxBitsPerScanline)
    {
        unsigned long BankBase, BankUnit;

        BankUnit = ((nBitsPerBank + nBitsPerScanline - 1) / nBitsPerBank) *
            nBitsPerBank;
        if (!(BankUnit % nBitsPerScanline))
            return (int)width;

        for (BankBase = BankUnit;  ;  BankBase += nBitsPerBank)
        {
            unsigned long x, y;

            y = BankBase / nBitsPerScanline;
            if (y >= ysize)
                return (int)width;

            x = BankBase % nBitsPerScanline;
            if (!(x % pBankFormat->bitsPerPixel))
                continue;

            if (x < minBitsPerScanline)
            {
                /*
                 * Skip ahead certain widths by dividing the excess scanline
                 * amongst the y's.
                 */
                y *= nBitsPerScanlinePadUnit;
                nBitsPerScanline +=
                    ((x + y - 1) / y) * nBitsPerScanlinePadUnit;
                width = nBitsPerScanline / pBankFormat->bitsPerPixel;
                break;
            }

            if (BankBase != BankUnit)
                continue;

            if (!(nBitsPerScanline % x))
                return (int)width;

            BankBase = ((nBitsPerScanline - minBitsPerScanline) /
                (nBitsPerScanline - x)) * BankUnit;
        }
    }

    return -1;
}
