/* $XFree86: xc/programs/Xserver/cfb/cfbgc.c,v 1.5 2001/01/17 22:36:35 dawes Exp $ */
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


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* $Xorg: cfbgc.c,v 1.4 2001/02/09 02:04:37 xorgcvs Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xproto.h>
#include "cfb.h"
#include <X11/fonts/fontstruct.h>
#include "dixfontstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "region.h"

#include "mistruct.h"
#include "mibstore.h"
#include "migc.h"

#include "cfbmskbits.h"
#include "cfb8bit.h"

#if PSZ == 8
# define useTEGlyphBlt  cfbTEGlyphBlt8
#else
# ifdef WriteBitGroup
#  define useTEGlyphBlt	cfbImageGlyphBlt8
# else
#  define useTEGlyphBlt	cfbTEGlyphBlt
# endif
#endif

#ifdef WriteBitGroup
# define useImageGlyphBlt	cfbImageGlyphBlt8
# define usePolyGlyphBlt	cfbPolyGlyphBlt8
#else
# define useImageGlyphBlt	miImageGlyphBlt
# define usePolyGlyphBlt	miPolyGlyphBlt
#endif

static void cfbUnPushPixels (GCPtr, PixmapPtr, DrawablePtr, int, int, int, int);

#ifdef FOUR_BIT_CODE
# define usePushPixels	cfbPushPixels8
#else
# define usePushPixels	cfbUnPushPixels
#endif

#ifdef PIXEL_ADDR
# define ZeroPolyArc	cfbZeroPolyArcSS8Copy
#else
# define ZeroPolyArc	miZeroPolyArc
#endif

GCFuncs cfbGCFuncs = {
    cfbValidateGC,
    miChangeGC,
    miCopyGC,
    miDestroyGC,
    miChangeClip,
    miDestroyClip,
    miCopyClip,
};

GCOps	cfbTEOps1Rect = {
    cfbSolidSpansCopy,
    cfbSetSpans,
    cfbPutImage,
    cfbCopyArea,
    cfbCopyPlane,
    cfbPolyPoint,
#ifdef PIXEL_ADDR
    cfb8LineSS1Rect,
    cfb8SegmentSS1Rect,
#else
    cfbLineSS,
    cfbSegmentSS,
#endif
    miPolyRectangle,
    ZeroPolyArc,
    cfbFillPoly1RectCopy,
    cfbPolyFillRect,
    cfbPolyFillArcSolidCopy,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    useTEGlyphBlt,
    usePolyGlyphBlt,
    usePushPixels
#ifdef NEED_LINEHELPER
    ,NULL
#endif
};

GCOps	cfbNonTEOps1Rect = {
    cfbSolidSpansCopy,
    cfbSetSpans,
    cfbPutImage,
    cfbCopyArea,
    cfbCopyPlane,
    cfbPolyPoint,
#ifdef PIXEL_ADDR
    cfb8LineSS1Rect,
    cfb8SegmentSS1Rect,
#else
    cfbLineSS,
    cfbSegmentSS,
#endif
    miPolyRectangle,
    ZeroPolyArc,
    cfbFillPoly1RectCopy,
    cfbPolyFillRect,
    cfbPolyFillArcSolidCopy,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    useImageGlyphBlt,
    usePolyGlyphBlt,
    usePushPixels
#ifdef NEED_LINEHELPER
    ,NULL
#endif
};

GCOps	cfbTEOps = {
    cfbSolidSpansCopy,
    cfbSetSpans,
    cfbPutImage,
    cfbCopyArea,
    cfbCopyPlane,
    cfbPolyPoint,
    cfbLineSS,
    cfbSegmentSS,
    miPolyRectangle,
    ZeroPolyArc,
    miFillPolygon,
    cfbPolyFillRect,
    cfbPolyFillArcSolidCopy,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    useTEGlyphBlt,
    usePolyGlyphBlt,
    usePushPixels
#ifdef NEED_LINEHELPER
    ,NULL
#endif
};

GCOps	cfbNonTEOps = {
    cfbSolidSpansCopy,
    cfbSetSpans,
    cfbPutImage,
    cfbCopyArea,
    cfbCopyPlane,
    cfbPolyPoint,
    cfbLineSS,
    cfbSegmentSS,
    miPolyRectangle,
#ifdef PIXEL_ADDR
    cfbZeroPolyArcSS8Copy,
#else
    miZeroPolyArc,
#endif
    miFillPolygon,
    cfbPolyFillRect,
    cfbPolyFillArcSolidCopy,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    useImageGlyphBlt,
    usePolyGlyphBlt,
    usePushPixels
#ifdef NEED_LINEHELPER
    ,NULL
#endif
};

GCOps *
cfbMatchCommon (pGC, devPriv)
    GCPtr	    pGC;
    cfbPrivGCPtr    devPriv;
{
    if (pGC->lineWidth != 0)
	return 0;
    if (pGC->lineStyle != LineSolid)
	return 0;
    if (pGC->fillStyle != FillSolid)
	return 0;
    if (devPriv->rop != GXcopy)
	return 0;
    if (pGC->font &&
	FONTMAXBOUNDS(pGC->font,rightSideBearing) -
        FONTMINBOUNDS(pGC->font,leftSideBearing) <= 32 &&
	FONTMINBOUNDS(pGC->font,characterWidth) >= 0)
    {
	if (TERMINALFONT(pGC->font)
#ifdef FOUR_BIT_CODE
	    && FONTMAXBOUNDS(pGC->font,characterWidth) >= PGSZB
#endif
	)
#ifdef NO_ONE_RECT
            return &cfbTEOps1Rect;
#else
	    if (devPriv->oneRect)
		return &cfbTEOps1Rect;
	    else
		return &cfbTEOps;
#endif
	else
#ifdef NO_ONE_RECT
	    return &cfbNonTEOps1Rect;
#else
	    if (devPriv->oneRect)
		return &cfbNonTEOps1Rect;
	    else
		return &cfbNonTEOps;
#endif
    }
    return 0;
}

Bool
cfbCreateGC(pGC)
    register GCPtr pGC;
{
    cfbPrivGC  *pPriv;

    if (PixmapWidthPaddingInfo[pGC->depth].padPixelsLog2 == LOG2_BITMAP_PAD)
	return (mfbCreateGC(pGC));
    pGC->clientClip = NULL;
    pGC->clientClipType = CT_NONE;

    if (cfbNonTEOps.PushPixels == cfbUnPushPixels)
    {
        cfbTEOps1Rect.PushPixels    = mfbPushPixelsWeak();
        cfbNonTEOps1Rect.PushPixels = mfbPushPixelsWeak();
        cfbTEOps.PushPixels         = mfbPushPixelsWeak();
        cfbNonTEOps.PushPixels      = mfbPushPixelsWeak();		    
    }

    /*
     * some of the output primitives aren't really necessary, since they
     * will be filled in ValidateGC because of dix/CreateGC() setting all
     * the change bits.  Others are necessary because although they depend
     * on being a color frame buffer, they don't change 
     */

    pGC->ops = &cfbNonTEOps;
    pGC->funcs = &cfbGCFuncs;

    /* cfb wants to translate before scan conversion */
    pGC->miTranslate = 1;

    pPriv = cfbGetGCPrivate(pGC);
    pPriv->rop = pGC->alu;
    pPriv->oneRect = FALSE;
    pGC->fExpose = TRUE;
    pGC->freeCompClip = FALSE;
    pGC->pRotatedPixmap = (PixmapPtr) NULL;
    return TRUE;
}

/* Clipping conventions
	if the drawable is a window
	    CT_REGION ==> pCompositeClip really is the composite
	    CT_other ==> pCompositeClip is the window clip region
	if the drawable is a pixmap
	    CT_REGION ==> pCompositeClip is the translated client region
		clipped to the pixmap boundary
	    CT_other ==> pCompositeClip is the pixmap bounding box
*/

void
cfbValidateGC(pGC, changes, pDrawable)
    register GCPtr  pGC;
    unsigned long   changes;
    DrawablePtr	    pDrawable;
{
    int         mask;		/* stateChanges */
    int         index;		/* used for stepping through bitfields */
    int		new_rrop;
    int         new_line, new_text, new_fillspans, new_fillarea;
    int		new_rotate;
    int		xrot, yrot;
    /* flags for changing the proc vector */
    cfbPrivGCPtr devPriv;
    int		oneRect;

    new_rotate = pGC->lastWinOrg.x != pDrawable->x ||
		 pGC->lastWinOrg.y != pDrawable->y;

    pGC->lastWinOrg.x = pDrawable->x;
    pGC->lastWinOrg.y = pDrawable->y;
    devPriv = cfbGetGCPrivate(pGC);

    new_rrop = FALSE;
    new_line = FALSE;
    new_text = FALSE;
    new_fillspans = FALSE;
    new_fillarea = FALSE;

    /*
     * if the client clip is different or moved OR the subwindowMode has
     * changed OR the window's clip has changed since the last validation
     * we need to recompute the composite clip 
     */

    if ((changes & (GCClipXOrigin|GCClipYOrigin|GCClipMask|GCSubwindowMode)) ||
	(pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS))
	)
    {
	miComputeCompositeClip (pGC, pDrawable);
#ifdef NO_ONE_RECT
	devPriv->oneRect = FALSE;
#else
	oneRect = REGION_NUM_RECTS(pGC->pCompositeClip) == 1;
	if (oneRect != devPriv->oneRect)
	    new_line = TRUE;
	devPriv->oneRect = oneRect;
#endif
    }

    mask = changes;
    while (mask) {
	index = lowbit (mask);
	mask &= ~index;

	/*
	 * this switch acculmulates a list of which procedures might have
	 * to change due to changes in the GC.  in some cases (e.g.
	 * changing one 16 bit tile for another) we might not really need
	 * a change, but the code is being paranoid. this sort of batching
	 * wins if, for example, the alu and the font have been changed,
	 * or any other pair of items that both change the same thing. 
	 */
	switch (index) {
	case GCFunction:
	case GCForeground:
	    new_rrop = TRUE;
	    break;
	case GCPlaneMask:
	    new_rrop = TRUE;
	    new_text = TRUE;
	    break;
	case GCBackground:
	    break;
	case GCLineStyle:
	case GCLineWidth:
	    new_line = TRUE;
	    break;
	case GCJoinStyle:
	case GCCapStyle:
	    break;
	case GCFillStyle:
	    new_text = TRUE;
	    new_fillspans = TRUE;
	    new_line = TRUE;
	    new_fillarea = TRUE;
	    break;
	case GCFillRule:
	    break;
	case GCTile:
	    new_fillspans = TRUE;
	    new_fillarea = TRUE;
	    break;

	case GCStipple:
	    if (pGC->stipple)
	    {
		int width = pGC->stipple->drawable.width;
		PixmapPtr nstipple;

		if ((width <= PGSZ) && !(width & (width - 1)) &&
		    (nstipple = cfbCopyPixmap(pGC->stipple)))
		{
		    cfbPadPixmap(nstipple);
		    (*pGC->pScreen->DestroyPixmap)(pGC->stipple);
		    pGC->stipple = nstipple;
		}
	    }
	    new_fillspans = TRUE;
	    new_fillarea = TRUE;
	    break;

	case GCTileStipXOrigin:
	    new_rotate = TRUE;
	    break;

	case GCTileStipYOrigin:
	    new_rotate = TRUE;
	    break;

	case GCFont:
	    new_text = TRUE;
	    break;
	case GCSubwindowMode:
	    break;
	case GCGraphicsExposures:
	    break;
	case GCClipXOrigin:
	    break;
	case GCClipYOrigin:
	    break;
	case GCClipMask:
	    break;
	case GCDashOffset:
	    break;
	case GCDashList:
	    break;
	case GCArcMode:
	    break;
	default:
	    break;
	}
    }

    /*
     * If the drawable has changed,  ensure suitable
     * entries are in the proc vector. 
     */
    if (pDrawable->serialNumber != (pGC->serialNumber & (DRAWABLE_SERIAL_BITS))) {
	new_fillspans = TRUE;	/* deal with FillSpans later */
    }

    if (new_rotate || new_fillspans)
    {
	Bool new_pix = FALSE;

	xrot = pGC->patOrg.x + pDrawable->x;
	yrot = pGC->patOrg.y + pDrawable->y;

	switch (pGC->fillStyle)
	{
	case FillTiled:
	    if (!pGC->tileIsPixel)
	    {
		int width = pGC->tile.pixmap->drawable.width * PSZ;

		if ((width <= PGSZ) && !(width & (width - 1)))
		{
		    cfbCopyRotatePixmap(pGC->tile.pixmap, &pGC->pRotatedPixmap,
					xrot, yrot);
		    new_pix = TRUE;
		}
	    }
	    break;
#ifdef FOUR_BIT_CODE
	case FillStippled:
	case FillOpaqueStippled:
	    {
		int width = pGC->stipple->drawable.width;

		if ((width <= PGSZ) && !(width & (width - 1)))
		{
		    mfbCopyRotatePixmap(pGC->stipple, &pGC->pRotatedPixmap,
					xrot, yrot);
		    new_pix = TRUE;
		}
	    }
	    break;
#endif
	}
	if (!new_pix && pGC->pRotatedPixmap)
	{
	    (*pGC->pScreen->DestroyPixmap)(pGC->pRotatedPixmap);
	    pGC->pRotatedPixmap = (PixmapPtr) NULL;
	}
    }

    if (new_rrop)
    {
	int old_rrop;

	old_rrop = devPriv->rop;
	devPriv->rop = cfbReduceRasterOp (pGC->alu, pGC->fgPixel,
					   pGC->planemask,
					   &devPriv->and, &devPriv->xor);
	if (old_rrop == devPriv->rop)
	    new_rrop = FALSE;
	else
	{
#ifdef PIXEL_ADDR
	    new_line = TRUE;
#endif
#ifdef WriteBitGroup
	    new_text = TRUE;
#endif
	    new_fillspans = TRUE;
	    new_fillarea = TRUE;
	}
    }

    if (new_rrop || new_fillspans || new_text || new_fillarea || new_line)
    {
	GCOps	*newops;

	if ((newops = cfbMatchCommon (pGC, devPriv)))
 	{
	    if (pGC->ops->devPrivate.val)
		miDestroyGCOps (pGC->ops);
	    pGC->ops = newops;
	    new_rrop = new_line = new_fillspans = new_text = new_fillarea = 0;
	}
 	else
 	{
	    if (!pGC->ops->devPrivate.val)
	    {
		pGC->ops = miCreateGCOps (pGC->ops);
		pGC->ops->devPrivate.val = 1;
	    }
	}
    }

    /* deal with the changes we've collected */
    if (new_line)
    {
	pGC->ops->FillPolygon = miFillPolygon;
#ifdef NO_ONE_RECT
	if (pGC->fillStyle == FillSolid)
	{
	    switch (devPriv->rop) {
	    case GXcopy:
		pGC->ops->FillPolygon = cfbFillPoly1RectCopy;
		break;
	    default:
		pGC->ops->FillPolygon = cfbFillPoly1RectGeneral;
		break;
	    }
	}
#else
	if (devPriv->oneRect && pGC->fillStyle == FillSolid)
	{
	    switch (devPriv->rop) {
	    case GXcopy:
		pGC->ops->FillPolygon = cfbFillPoly1RectCopy;
		break;
	    default:
		pGC->ops->FillPolygon = cfbFillPoly1RectGeneral;
		break;
	    }
	}
#endif
	if (pGC->lineWidth == 0)
	{
#ifdef PIXEL_ADDR
	    if ((pGC->lineStyle == LineSolid) && (pGC->fillStyle == FillSolid))
	    {
		switch (devPriv->rop)
		{
		case GXxor:
		    pGC->ops->PolyArc = cfbZeroPolyArcSS8Xor;
		    break;
		case GXcopy:
		    pGC->ops->PolyArc = cfbZeroPolyArcSS8Copy;
		    break;
		default:
		    pGC->ops->PolyArc = cfbZeroPolyArcSS8General;
		    break;
		}
	    }
	    else
#endif
		pGC->ops->PolyArc = miZeroPolyArc;
	}
	else
	    pGC->ops->PolyArc = miPolyArc;
	pGC->ops->PolySegment = miPolySegment;
	switch (pGC->lineStyle)
	{
	case LineSolid:
	    if(pGC->lineWidth == 0)
	    {
		if (pGC->fillStyle == FillSolid)
		{
#if defined(PIXEL_ADDR) && !defined(NO_ONE_RECT)
		    if (devPriv->oneRect &&
			((pDrawable->x >= pGC->pScreen->width - 32768) &&
			 (pDrawable->y >= pGC->pScreen->height - 32768)))
		    {
			pGC->ops->Polylines = cfb8LineSS1Rect;
			pGC->ops->PolySegment = cfb8SegmentSS1Rect;
		    } else
#endif
#ifdef NO_ONE_RECT
		    {
			pGC->ops->Polylines = cfb8LineSS1Rect;
			pGC->ops->PolySegment = cfb8SegmentSS1Rect;
		    }
#else
		    {
		    	pGC->ops->Polylines = cfbLineSS;
		    	pGC->ops->PolySegment = cfbSegmentSS;
		    }
#endif
		}
 		else
		    pGC->ops->Polylines = miZeroLine;
	    }
	    else
		pGC->ops->Polylines = miWideLine;
	    break;
	case LineOnOffDash:
	case LineDoubleDash:
	    if (pGC->lineWidth == 0 && pGC->fillStyle == FillSolid)
	    {
		pGC->ops->Polylines = cfbLineSD;
		pGC->ops->PolySegment = cfbSegmentSD;
	    } else
		pGC->ops->Polylines = miWideDash;
	    break;
	}
    }

    if (new_text && (pGC->font))
    {
        if (FONTMAXBOUNDS(pGC->font,rightSideBearing) -
            FONTMINBOUNDS(pGC->font,leftSideBearing) > 32 ||
	    FONTMINBOUNDS(pGC->font,characterWidth) < 0)
        {
            pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
            pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
        }
        else
        {
#ifdef WriteBitGroup
	    if (pGC->fillStyle == FillSolid)
	    {
		if (devPriv->rop == GXcopy)
		    pGC->ops->PolyGlyphBlt = cfbPolyGlyphBlt8;
		else
#ifdef FOUR_BIT_CODE
		    pGC->ops->PolyGlyphBlt = cfbPolyGlyphRop8;
#else
		    pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
#endif
	    }
	    else
#endif
		pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
            /* special case ImageGlyphBlt for terminal emulator fonts */
#if !defined(WriteBitGroup) || PSZ == 8
	    if (TERMINALFONT(pGC->font) &&
		(pGC->planemask & PMSK) == PMSK
#ifdef FOUR_BIT_CODE
		&& FONTMAXBOUNDS(pGC->font,characterWidth) >= PGSZB
#endif
		)
	    {
		pGC->ops->ImageGlyphBlt = useTEGlyphBlt;
	    }
            else
#endif
	    {
#ifdef WriteBitGroup
		if (devPriv->rop == GXcopy &&
		    pGC->fillStyle == FillSolid &&
		    (pGC->planemask & PMSK) == PMSK)
		    pGC->ops->ImageGlyphBlt = cfbImageGlyphBlt8;
		else
#endif
		    pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
	    }
        }
    }    


    if (new_fillspans) {
	switch (pGC->fillStyle) {
	case FillSolid:
	    switch (devPriv->rop) {
	    case GXcopy:
		pGC->ops->FillSpans = cfbSolidSpansCopy;
		break;
	    case GXxor:
		pGC->ops->FillSpans = cfbSolidSpansXor;
		break;
	    default:
		pGC->ops->FillSpans = cfbSolidSpansGeneral;
		break;
	    }
	    break;
	case FillTiled:
	    if (pGC->pRotatedPixmap)
	    {
		if (pGC->alu == GXcopy && (pGC->planemask & PMSK) == PMSK)
		    pGC->ops->FillSpans = cfbTile32FSCopy;
		else
		    pGC->ops->FillSpans = cfbTile32FSGeneral;
	    }
	    else
		pGC->ops->FillSpans = cfbUnnaturalTileFS;
	    break;
	case FillStippled:
#ifdef FOUR_BIT_CODE
	    if (pGC->pRotatedPixmap)
		pGC->ops->FillSpans = cfb8Stipple32FS;
	    else
#endif
		pGC->ops->FillSpans = cfbUnnaturalStippleFS;
	    break;
	case FillOpaqueStippled:
#ifdef FOUR_BIT_CODE
	    if (pGC->pRotatedPixmap)
		pGC->ops->FillSpans = cfb8OpaqueStipple32FS;
	    else
#endif
		pGC->ops->FillSpans = cfbUnnaturalStippleFS;
	    break;
	default:
	    FatalError("cfbValidateGC: illegal fillStyle\n");
	}
    } /* end of new_fillspans */

    if (new_fillarea) {
#ifndef FOUR_BIT_CODE
	pGC->ops->PolyFillRect = miPolyFillRect;
	if (pGC->fillStyle == FillSolid || pGC->fillStyle == FillTiled)
	{
	    pGC->ops->PolyFillRect = cfbPolyFillRect;
	}
#endif
#ifdef FOUR_BIT_CODE
	pGC->ops->PushPixels = mfbPushPixelsWeak();
	if (pGC->fillStyle == FillSolid && devPriv->rop == GXcopy)
	    pGC->ops->PushPixels = cfbPushPixels8;
#endif
	pGC->ops->PolyFillArc = miPolyFillArc;
	if (pGC->fillStyle == FillSolid)
	{
	    switch (devPriv->rop)
	    {
	    case GXcopy:
		pGC->ops->PolyFillArc = cfbPolyFillArcSolidCopy;
		break;
	    default:
		pGC->ops->PolyFillArc = cfbPolyFillArcSolidGeneral;
		break;
	    }
	}
    }
}

/*
 * this is never called, it just exists to have its address
 * taken in mfbCreateGC.
 */
static void
cfbUnPushPixels (pGC, pBitmap, pDrawable, dx, dy, xOrg, yOrg)
    GCPtr       pGC;
    PixmapPtr   pBitmap;
    DrawablePtr pDrawable;
    int         dx, dy, xOrg, yOrg;
{
    return;
}
