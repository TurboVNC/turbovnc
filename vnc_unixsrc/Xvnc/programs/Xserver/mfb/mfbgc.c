/* $XFree86: xc/programs/Xserver/mfb/mfbgc.c,v 1.9 2003/07/16 01:38:55 dawes Exp $ */
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
/* $Xorg: mfbgc.c,v 1.4 2001/02/09 02:05:19 xorgcvs Exp $ */
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xproto.h>
#include "mfb.h"
#include "dixfontstr.h"
#include <X11/fonts/fontstruct.h>
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "region.h"

#include "mistruct.h"
#include "migc.h"

#include "maskbits.h"

static GCFuncs	mfbFuncs = {
	mfbValidateGC,
	miChangeGC,
	miCopyGC,
	miDestroyGC,
	miChangeClip,
	miDestroyClip,
	miCopyClip
};


static GCOps	whiteTECopyOps = {
	mfbWhiteSolidFS,
	mfbSetSpans,
	mfbPutImage,
	mfbCopyArea,
	mfbCopyPlane,
	mfbPolyPoint,
	mfbLineSS,
	mfbSegmentSS,
	miPolyRectangle,
	mfbZeroPolyArcSS,
	mfbFillPolyWhite,
	mfbPolyFillRect,
	mfbPolyFillArcSolid,
	miPolyText8,
	miPolyText16,
	miImageText8,
	miImageText16,
	mfbTEGlyphBltWhite,
	mfbPolyGlyphBltWhite,
	mfbSolidPP
#ifdef NEED_LINEHELPER
	,NULL
#endif
};

static GCOps	blackTECopyOps = {
	mfbBlackSolidFS,
	mfbSetSpans,
	mfbPutImage,
	mfbCopyArea,
	mfbCopyPlane,
	mfbPolyPoint,
	mfbLineSS,
	mfbSegmentSS,
	miPolyRectangle,
	mfbZeroPolyArcSS,
	mfbFillPolyBlack,
	mfbPolyFillRect,
	mfbPolyFillArcSolid,
	miPolyText8,
	miPolyText16,
	miImageText8,
	miImageText16,
	mfbTEGlyphBltBlack,
	mfbPolyGlyphBltBlack,
	mfbSolidPP
#ifdef NEED_LINEHELPER
	,NULL
#endif
};

static GCOps	whiteTEInvertOps = {
	mfbInvertSolidFS,
	mfbSetSpans,
	mfbPutImage,
	miCopyArea,
	miCopyPlane,
	mfbPolyPoint,
	mfbLineSS,
	mfbSegmentSS,
	miPolyRectangle,
	miZeroPolyArc,
	mfbFillPolyInvert,
	mfbPolyFillRect,
	mfbPolyFillArcSolid,
	miPolyText8,
	miPolyText16,
	miImageText8,
	miImageText16,
	mfbTEGlyphBltWhite,
	mfbPolyGlyphBltInvert,
	mfbSolidPP
#ifdef NEED_LINEHELPER
	,NULL
#endif
};

static GCOps	blackTEInvertOps = {
	mfbInvertSolidFS,
	mfbSetSpans,
	mfbPutImage,
	mfbCopyArea,
	mfbCopyPlane,
	mfbPolyPoint,
	mfbLineSS,
	mfbSegmentSS,
	miPolyRectangle,
	miZeroPolyArc,
	mfbFillPolyInvert,
	mfbPolyFillRect,
	mfbPolyFillArcSolid,
	miPolyText8,
	miPolyText16,
	miImageText8,
	miImageText16,
	mfbTEGlyphBltBlack,
	mfbPolyGlyphBltInvert,
	mfbSolidPP
#ifdef NEED_LINEHELPER
	,NULL
#endif
};

static GCOps	whiteCopyOps = {
	mfbWhiteSolidFS,
	mfbSetSpans,
	mfbPutImage,
	mfbCopyArea,
	mfbCopyPlane,
	mfbPolyPoint,
	mfbLineSS,
	mfbSegmentSS,
	miPolyRectangle,
	mfbZeroPolyArcSS,
	mfbFillPolyWhite,
	mfbPolyFillRect,
	mfbPolyFillArcSolid,
	miPolyText8,
	miPolyText16,
	miImageText8,
	miImageText16,
	mfbImageGlyphBltWhite,
	mfbPolyGlyphBltWhite,
	mfbSolidPP
#ifdef NEED_LINEHELPER
	,NULL
#endif
};

static GCOps	blackCopyOps = {
	mfbBlackSolidFS,
	mfbSetSpans,
	mfbPutImage,
	mfbCopyArea,
	mfbCopyPlane,
	mfbPolyPoint,
	mfbLineSS,
	mfbSegmentSS,
	miPolyRectangle,
	mfbZeroPolyArcSS,
	mfbFillPolyBlack,
	mfbPolyFillRect,
	mfbPolyFillArcSolid,
	miPolyText8,
	miPolyText16,
	miImageText8,
	miImageText16,
	mfbImageGlyphBltBlack,
	mfbPolyGlyphBltBlack,
	mfbSolidPP
#ifdef NEED_LINEHELPER
	,NULL
#endif
};

static GCOps	whiteInvertOps = {
	mfbInvertSolidFS,
	mfbSetSpans,
	mfbPutImage,
	mfbCopyArea,
	mfbCopyPlane,
	mfbPolyPoint,
	mfbLineSS,
	mfbSegmentSS,
	miPolyRectangle,
	miZeroPolyArc,
	mfbFillPolyInvert,
	mfbPolyFillRect,
	mfbPolyFillArcSolid,
	miPolyText8,
	miPolyText16,
	miImageText8,
	miImageText16,
	mfbImageGlyphBltWhite,
	mfbPolyGlyphBltInvert,
	mfbSolidPP
#ifdef NEED_LINEHELPER
	,NULL
#endif
};

static GCOps	blackInvertOps = {
	mfbInvertSolidFS,
	mfbSetSpans,
	mfbPutImage,
	mfbCopyArea,
	mfbCopyPlane,
	mfbPolyPoint,
	mfbLineSS,
	mfbSegmentSS,
	miPolyRectangle,
	miZeroPolyArc,
	mfbFillPolyInvert,
	mfbPolyFillRect,
	mfbPolyFillArcSolid,
	miPolyText8,
	miPolyText16,
	miImageText8,
	miImageText16,
	mfbImageGlyphBltBlack,
	mfbPolyGlyphBltInvert,
	mfbSolidPP
#ifdef NEED_LINEHELPER
	,NULL
#endif
};

static GCOps	whiteWhiteCopyOps = {
	mfbWhiteSolidFS,
	mfbSetSpans,
	mfbPutImage,
	mfbCopyArea,
	mfbCopyPlane,
	mfbPolyPoint,
	mfbLineSS,
	mfbSegmentSS,
	miPolyRectangle,
	mfbZeroPolyArcSS,
	mfbFillPolyWhite,
	mfbPolyFillRect,
	mfbPolyFillArcSolid,
	miPolyText8,
	miPolyText16,
	miImageText8,
	miImageText16,
	miImageGlyphBlt,
	mfbPolyGlyphBltWhite,
	mfbSolidPP
#ifdef NEED_LINEHELPER
	,NULL
#endif
};

static GCOps	blackBlackCopyOps = {
	mfbBlackSolidFS,
	mfbSetSpans,
	mfbPutImage,
	mfbCopyArea,
	mfbCopyPlane,
	mfbPolyPoint,
	mfbLineSS,
	mfbSegmentSS,
	miPolyRectangle,
	mfbZeroPolyArcSS,
	mfbFillPolyBlack,
	mfbPolyFillRect,
	mfbPolyFillArcSolid,
	miPolyText8,
	miPolyText16,
	miImageText8,
	miImageText16,
	miImageGlyphBlt,
	mfbPolyGlyphBltBlack,
	mfbSolidPP
#ifdef NEED_LINEHELPER
	,NULL
#endif
};

static GCOps	fgEqBgInvertOps = {
	mfbInvertSolidFS,
	mfbSetSpans,
	mfbPutImage,
	mfbCopyArea,
	mfbCopyPlane,
	mfbPolyPoint,
	mfbLineSS,
	mfbSegmentSS,
	miPolyRectangle,
	miZeroPolyArc,
	mfbFillPolyInvert,
	mfbPolyFillRect,
	mfbPolyFillArcSolid,
	miPolyText8,
	miPolyText16,
	miImageText8,
	miImageText16,
	miImageGlyphBlt,
	mfbPolyGlyphBltInvert,
	mfbSolidPP
#ifdef NEED_LINEHELPER
	,NULL
#endif
};


struct commonOps {
    int		    fg, bg;
    int		    rrop;
    int		    terminalFont;
    GCOps	    *ops;
    void	    (*fillArea)(
	DrawablePtr /*pDraw*/,
    	int /*nbox*/,
    	BoxPtr /*pbox*/,
    	int /*alu*/,
    	PixmapPtr /*nop*/);
};

static struct commonOps mfbCommonOps[] = {
    { 1, 0, RROP_WHITE, 1, &whiteTECopyOps, mfbSolidWhiteArea },
    { 0, 1, RROP_BLACK, 1, &blackTECopyOps, mfbSolidBlackArea },
    { 1, 0, RROP_INVERT, 1, &whiteTEInvertOps, mfbSolidInvertArea },
    { 0, 1, RROP_INVERT, 1, &blackTEInvertOps, mfbSolidInvertArea },
    { 1, 0, RROP_WHITE, 0, &whiteCopyOps, mfbSolidWhiteArea },
    { 0, 1, RROP_BLACK, 0, &blackCopyOps, mfbSolidBlackArea },
    { 1, 0, RROP_INVERT, 0, &whiteInvertOps, mfbSolidInvertArea },
    { 0, 1, RROP_INVERT, 0, &blackInvertOps, mfbSolidInvertArea },
    { 1, 1, RROP_WHITE, 0, &whiteWhiteCopyOps, mfbSolidWhiteArea },
    { 0, 0, RROP_BLACK, 0, &blackBlackCopyOps, mfbSolidBlackArea },
    { 1, 1, RROP_INVERT, 0, &fgEqBgInvertOps, mfbSolidInvertArea },
    { 0, 0, RROP_INVERT, 0, &fgEqBgInvertOps, mfbSolidInvertArea },
};

#define numberCommonOps	(sizeof (mfbCommonOps) / sizeof (mfbCommonOps[0]))

static GCOps *
matchCommon (
    GCPtr   pGC)
{
    int	i;
    struct commonOps	*cop;
    mfbPrivGC		*priv;

    if (pGC->lineWidth != 0)
	return 0;
    if (pGC->lineStyle != LineSolid)
	return 0;
    if (pGC->fillStyle != FillSolid)
	return 0;
    if (!pGC->font ||
        FONTMAXBOUNDS(pGC->font,rightSideBearing) -
	FONTMINBOUNDS(pGC->font,leftSideBearing) > 32 ||
	FONTMINBOUNDS(pGC->font,characterWidth) < 0)
	return 0;
    priv = (mfbPrivGC *) pGC->devPrivates[mfbGCPrivateIndex].ptr;
    for (i = 0; i < numberCommonOps; i++) {
	cop = &mfbCommonOps[i];
	if ((pGC->fgPixel & 1) != cop->fg)
	    continue;
	if ((pGC->bgPixel & 1) != cop->bg)
	    continue;
	if (priv->rop != cop->rrop)
	    continue;
	if (cop->terminalFont && !TERMINALFONT(pGC->font))
	    continue;
	priv->FillArea = cop->fillArea;
	return cop->ops;
    }
    return 0;
}


Bool
mfbCreateGC(pGC)
    register GCPtr pGC;
{
    mfbPrivGC 	*pPriv;

    pGC->clientClip = NULL;
    pGC->clientClipType = CT_NONE;
    
    /* some of the output primitives aren't really necessary, since
       they will be filled in ValidateGC because of dix/CreateGC()
       setting all the change bits.  Others are necessary because although
       they depend on being a monochrome frame buffer, they don't change 
    */

    pGC->ops = &whiteCopyOps;
    pGC->funcs = &mfbFuncs;

    /* mfb wants to translate before scan convesion */
    pGC->miTranslate = 1;

    pPriv = (mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr);
    pPriv->rop = mfbReduceRop(pGC->alu, pGC->fgPixel);
    pGC->fExpose = TRUE;
    pGC->pRotatedPixmap = NullPixmap;
    pGC->freeCompClip = FALSE;
    pPriv->FillArea = mfbSolidInvertArea;
    return TRUE;
}

/* some noop functions */
static void
mfbPolyGlyphBltNoop(
    DrawablePtr pDrawable,
    GCPtr pGC,
    int x,
    int y,
    unsigned int nglyph,
    CharInfoPtr * ppci,
    pointer pglyphBase)
{
    /* this is a no-op function */
}

static void
mfbNoopFS(
    DrawablePtr pDrawable,
    GCPtr pGC,
    int nInit,
    DDXPointPtr pptInit,
    int * pwidthInit,
    int fSorted)
{
    /* this is a no-op function */
}

static void
mfbFillPolyNoop(
    DrawablePtr pDrawable,
    GCPtr pGC,
    int shape,
    int mode,
    int count,
    DDXPointPtr ptsIn)
{
    /* this is a no-op function */
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

/*ARGSUSED*/
void
mfbValidateGC(pGC, changes, pDrawable)
    register GCPtr 	pGC;
    unsigned long	changes;
    DrawablePtr 	pDrawable;
{
    register mfbPrivGCPtr	devPriv;
    int mask;			/* stateChanges */
    int index;			/* used for stepping through bitfields */
    int	xrot, yrot;		/* rotations for tile and stipple pattern */
    int rrop;			/* reduced rasterop */
				/* flags for changing the proc vector 
				   and updating things in devPriv
				*/
    int new_rotate, new_rrop,  new_line, new_text, new_fill;
    DDXPointRec	oldOrg;		/* origin of thing GC was last used with */

    oldOrg = pGC->lastWinOrg;

    pGC->lastWinOrg.x = pDrawable->x;
    pGC->lastWinOrg.y = pDrawable->y;

    /* we need to re-rotate the tile if the previous window/pixmap
       origin (oldOrg) differs from the new window/pixmap origin
       (pGC->lastWinOrg)
    */
    new_rotate = (oldOrg.x != pGC->lastWinOrg.x) ||
		 (oldOrg.y != pGC->lastWinOrg.y);

    devPriv = ((mfbPrivGCPtr) (pGC->devPrivates[mfbGCPrivateIndex].ptr));

    /*
	if the client clip is different or moved OR
	the subwindowMode has changed OR
	the window's clip has changed since the last validation
	we need to recompute the composite clip
    */
    if ((changes & (GCClipXOrigin|GCClipYOrigin|GCClipMask|GCSubwindowMode)) ||
	(pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS))
       )
    {
	miComputeCompositeClip(pGC, pDrawable);
    }

    new_rrop = FALSE;
    new_line = FALSE;
    new_text = FALSE;
    new_fill = FALSE;

    mask = changes;
    while (mask)
    {
	index = lowbit (mask);
	mask &= ~index;

	/* this switch acculmulates a list of which procedures
	   might have to change due to changes in the GC.  in
	   some cases (e.g. changing one 16 bit tile for another)
	   we might not really need a change, but the code is
	   being paranoid.
	   this sort of batching wins if, for example, the alu
	   and the font have been changed, or any other pair
	   of items that both change the same thing.
	*/
	switch (index)
	{
	  case GCFunction:
	  case GCForeground:
	    new_rrop = TRUE;
	    break;
	  case GCPlaneMask:
	    break;
	  case GCBackground:
	    new_rrop = TRUE;	/* for opaque stipples */
	    break;
	  case GCLineStyle:
	  case GCLineWidth:
	  case GCJoinStyle:
	    new_line = TRUE;
	    break;
	  case GCCapStyle:
	    break;
	  case GCFillStyle:
	    new_fill = TRUE;
	    break;
	  case GCFillRule:
	    break;
	  case GCTile:
	    if(pGC->tileIsPixel)
		break;
	    new_rotate = TRUE;
	    new_fill = TRUE;
	    break;

	  case GCStipple:
	    if(pGC->stipple == (PixmapPtr)NULL)
		break;
	    new_rotate = TRUE;
	    new_fill = TRUE;
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

    /* deal with the changes we've collected .
       new_rrop must be done first because subsequent things
       depend on it.
    */

    if(new_rotate || new_fill)
    {
	Bool new_pix = FALSE;

	/* figure out how much to rotate */
	xrot = pGC->patOrg.x;
	yrot = pGC->patOrg.y;
	xrot += pDrawable->x;
	yrot += pDrawable->y;

	switch (pGC->fillStyle)
	{
	case FillTiled:
	    /* copy current tile and stipple */
	    if (!pGC->tileIsPixel && (pGC->tile.pixmap->drawable.width <= PPW) &&
		!(pGC->tile.pixmap->drawable.width & (pGC->tile.pixmap->drawable.width - 1)))
	    {
		mfbCopyRotatePixmap(pGC->tile.pixmap,
				    &pGC->pRotatedPixmap, xrot, yrot);
		new_pix = TRUE;
	    }
	    break;
	case FillStippled:
	case FillOpaqueStippled:
	    if (pGC->stipple && (pGC->stipple->drawable.width <= PPW) &&
	    	!(pGC->stipple->drawable.width & (pGC->stipple->drawable.width - 1)))
	    {
		mfbCopyRotatePixmap(pGC->stipple,
				    &pGC->pRotatedPixmap, xrot, yrot);
		new_pix = TRUE;
	    }
	}
	/* destroy any previously rotated tile or stipple */
	if (!new_pix && pGC->pRotatedPixmap)
	{
	    (*pDrawable->pScreen->DestroyPixmap)(pGC->pRotatedPixmap);
	    pGC->pRotatedPixmap = (PixmapPtr)NULL;
	}
    }

    /*
     * duck out here when the GC is unchanged
     */

    if (!changes)
	return;

    if (new_rrop || new_fill)
    {
	rrop = mfbReduceRop(pGC->alu, pGC->fgPixel);
	devPriv->rop = rrop;
	new_fill = TRUE;
	/* FillArea raster op is GC's for tile filling,
	   and the reduced rop for solid and stipple
	*/
	if (pGC->fillStyle == FillTiled)
	    devPriv->ropFillArea = pGC->alu;
	else
	    devPriv->ropFillArea = rrop;

	/* opaque stipples:
	   fg	bg	ropOpStip	fill style
	   1	0	alu		tile
	   0	1	inverseAlu	tile
	   1	1	rrop(fg, alu)	solid
	   0	0	rrop(fg, alu)	solid
	Note that rrop(fg, alu) == mfbPrivGC.rop, so we don't really need to
	compute it.
	*/
        if (pGC->fillStyle == FillOpaqueStippled)
        {
	    if ((pGC->fgPixel & 1) != (pGC->bgPixel & 1))
	    {
	        if (pGC->fgPixel & 1)
		    devPriv->ropOpStip = pGC->alu;
	        else
		    devPriv->ropOpStip = InverseAlu[pGC->alu];
	    }
	    else
	        devPriv->ropOpStip = rrop;
	    devPriv->ropFillArea = devPriv->ropOpStip;
        }
    }
    else
	rrop = devPriv->rop;

    if (new_line || new_fill || new_text)
    {
	GCOps	*newops;

	if ((newops = matchCommon (pGC)))
 	{
	    if (pGC->ops->devPrivate.val)
		miDestroyGCOps (pGC->ops);
	    pGC->ops = newops;
	    new_line = new_fill = new_text = 0;
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

    if (new_line || new_fill)
    {
	if (pGC->lineWidth == 0)
	{
	    if ((pGC->lineStyle == LineSolid) && (pGC->fillStyle == FillSolid)
		&& ((rrop == RROP_WHITE) || (rrop == RROP_BLACK)))
		pGC->ops->PolyArc = mfbZeroPolyArcSS;
	    else
		pGC->ops->PolyArc = miZeroPolyArc;
	}
	else
	    pGC->ops->PolyArc = miPolyArc;
	if (pGC->lineStyle == LineSolid)
	{
	    if(pGC->lineWidth == 0)
	    {
	        if (pGC->fillStyle == FillSolid)
		{
		    pGC->ops->PolySegment = mfbSegmentSS;
		    pGC->ops->Polylines = mfbLineSS;
	        }
 		else
		{
		    pGC->ops->PolySegment = miPolySegment;
		    pGC->ops->Polylines = miZeroLine;
		}
	    }
	    else
	    {
		pGC->ops->PolySegment = miPolySegment;
		pGC->ops->Polylines = miWideLine;
	    }
	}
	else
	{
	    if(pGC->lineWidth == 0 && pGC->fillStyle == FillSolid)
	    {
	        pGC->ops->Polylines = mfbLineSD;
		pGC->ops->PolySegment = mfbSegmentSD;
	    }
	    else
	    {
	        pGC->ops->Polylines = miWideDash;
		pGC->ops->PolySegment = miPolySegment;
	    }
	}
    }

    if (new_text || new_fill)
    {
	if ((pGC->font) &&
	    (FONTMAXBOUNDS(pGC->font,rightSideBearing) -
	     FONTMINBOUNDS(pGC->font,leftSideBearing) > 32 ||
	     FONTMINBOUNDS(pGC->font,characterWidth) < 0))
	{
	    pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
	    pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
	}
	else
	{
	    /* special case ImageGlyphBlt for terminal emulator fonts */


	    if ((pGC->font) &&
		TERMINALFONT(pGC->font) &&
		((pGC->fgPixel & 1) != (pGC->bgPixel & 1)))
	    {
		/* pcc bug makes this not compile...
		pGC->ops->ImageGlyphBlt = (pGC->fgPixel & 1) ? mfbTEGlyphBltWhite :
						      mfbTEGlyphBltBlack;
		*/
		if (pGC->fgPixel & 1)
		    pGC->ops->ImageGlyphBlt = mfbTEGlyphBltWhite;
		else
		    pGC->ops->ImageGlyphBlt = mfbTEGlyphBltBlack;
	    }
	    else


	    {
	        if (pGC->fgPixel & 1)
		    pGC->ops->ImageGlyphBlt = mfbImageGlyphBltWhite;
	        else
		    pGC->ops->ImageGlyphBlt = mfbImageGlyphBltBlack;
	    }

	    /* now do PolyGlyphBlt */
	    if (pGC->fillStyle == FillSolid ||
		(pGC->fillStyle == FillOpaqueStippled &&
		 (pGC->fgPixel & 1) == (pGC->bgPixel & 1)
		)
	       )
	    {
		if (rrop == RROP_WHITE)
		    pGC->ops->PolyGlyphBlt = mfbPolyGlyphBltWhite;
		else if (rrop == RROP_BLACK)
		    pGC->ops->PolyGlyphBlt = mfbPolyGlyphBltBlack;
		else if (rrop == RROP_INVERT)
		    pGC->ops->PolyGlyphBlt = mfbPolyGlyphBltInvert;
		else
		    pGC->ops->PolyGlyphBlt = mfbPolyGlyphBltNoop;
	    }
	    else
	    {
		pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
	    }
	}
    }

    if (new_fill)


    {
	/* install a suitable fillspans and pushpixels */
	pGC->ops->PushPixels = mfbPushPixels;
	pGC->ops->FillPolygon = miFillPolygon;
	if ((pGC->fillStyle == FillSolid) ||
	    ((pGC->fillStyle == FillOpaqueStippled) &&
	     ((pGC->fgPixel & 1) == (pGC->bgPixel & 1))))
	{
	    pGC->ops->PushPixels = mfbSolidPP;
	    switch(devPriv->rop)
	    {
	      case RROP_WHITE:
		pGC->ops->FillSpans = mfbWhiteSolidFS;
		pGC->ops->FillPolygon = mfbFillPolyWhite;
		break;
	      case RROP_BLACK:
		pGC->ops->FillSpans = mfbBlackSolidFS;
		pGC->ops->FillPolygon = mfbFillPolyBlack;
		break;
	      case RROP_INVERT:
		pGC->ops->FillSpans = mfbInvertSolidFS;
		pGC->ops->FillPolygon = mfbFillPolyInvert;
		break;
	      case RROP_NOP:
		pGC->ops->FillSpans = mfbNoopFS;
		pGC->ops->FillPolygon = mfbFillPolyNoop;
		break;
	    }
	}
	/* beyond this point, opaqueStippled ==> fg != bg */
	else if (((pGC->fillStyle == FillTiled) ||
		  (pGC->fillStyle == FillOpaqueStippled)) &&
		 !pGC->pRotatedPixmap)
	{
	    pGC->ops->FillSpans = mfbUnnaturalTileFS;
	}
	else if ((pGC->fillStyle == FillStippled) && !pGC->pRotatedPixmap)
	{
	    pGC->ops->FillSpans = mfbUnnaturalStippleFS;
	}
	else if (pGC->fillStyle == FillStippled)
	{
	    switch(devPriv->rop)
	    {
	      case RROP_WHITE:
		pGC->ops->FillSpans = mfbWhiteStippleFS;
		break;
	      case RROP_BLACK:
		pGC->ops->FillSpans = mfbBlackStippleFS;
		break;
	      case RROP_INVERT:
		pGC->ops->FillSpans = mfbInvertStippleFS;
		break;
	      case RROP_NOP:
		pGC->ops->FillSpans = mfbNoopFS;
		break;
	    }
	}
	else /* overload tiles to do parti-colored opaque stipples */
	{
	    pGC->ops->FillSpans = mfbTileFS;
	}
	if (pGC->fillStyle == FillSolid)
	    pGC->ops->PolyFillArc = mfbPolyFillArcSolid;
	else
	    pGC->ops->PolyFillArc = miPolyFillArc;
	/* the rectangle code doesn't deal with opaque stipples that
	   are two colors -- we can fool it for fg==bg, though
	 */
	if ((((pGC->fillStyle == FillTiled) ||
	      (pGC->fillStyle == FillStippled)) &&
	     !pGC->pRotatedPixmap) ||
	    ((pGC->fillStyle == FillOpaqueStippled) &&
	     ((pGC->fgPixel & 1) != (pGC->bgPixel & 1)))
	   )
	{
	    pGC->ops->PolyFillRect = miPolyFillRect;
	}
	else /* deal with solids and natural stipples and tiles */
	{
	    pGC->ops->PolyFillRect = mfbPolyFillRect;

	    if ((pGC->fillStyle == FillSolid) ||
		((pGC->fillStyle == FillOpaqueStippled) &&
		 ((pGC->fgPixel & 1) == (pGC->bgPixel & 1))))
	    {
		switch(devPriv->rop)
		{
		  case RROP_WHITE:
		    devPriv->FillArea = mfbSolidWhiteArea;
		    break;
		  case RROP_BLACK:
		    devPriv->FillArea = mfbSolidBlackArea;
		    break;
		  case RROP_INVERT:
		    devPriv->FillArea = mfbSolidInvertArea;
		    break;
		  case RROP_NOP:
		    devPriv->FillArea = (mfbFillAreaProcPtr)NoopDDA;
		    break;
		}
	    }
	    else if (pGC->fillStyle == FillStippled)
	    {
		switch(devPriv->rop)
		{
		  case RROP_WHITE:
		    devPriv->FillArea = mfbStippleWhiteArea;
		    break;
		  case RROP_BLACK:
		    devPriv->FillArea = mfbStippleBlackArea;
		    break;
		  case RROP_INVERT:
		    devPriv->FillArea = mfbStippleInvertArea;
		    break;
		  case RROP_NOP:
		    devPriv->FillArea = (mfbFillAreaProcPtr)NoopDDA;
		    break;
		}
	    }
	    else /* deal with tiles */
	    {
		switch (pGC->alu)
		{
		  case GXcopy:
		    devPriv->FillArea = mfbTileAreaPPWCopy;
		    break;
		  default:
		    devPriv->FillArea = mfbTileAreaPPWGeneral;
		    break;
		}
	    }
	} /* end of natural rectangles */
    } /* end of new_fill */


}

/* table to map alu(src, dst) to alu(~src, dst) */
int InverseAlu[16] = {
	GXclear,
	GXandInverted,
	GXnor,
	GXcopyInverted,
	GXand,
	GXnoop,
	GXequiv,
	GXorInverted,
	GXandReverse,
	GXxor,
	GXinvert,
	GXnand,
	GXcopy,
	GXor,
	GXorReverse,
	GXset
};

int mfbGetInverseAlu(i)
    int i;
{
    return InverseAlu[i];
}

int
mfbReduceRop(alu, src)
    register int alu;
    register Pixel src;
{
    int rop = 0;
    if ((src & 1) == 0)	/* src is black */
    {
	switch(alu)
	{
	  case GXclear:
	    rop = RROP_BLACK;
	    break;
	  case GXand:
	    rop = RROP_BLACK;
	    break;
	  case GXandReverse:
	    rop = RROP_BLACK;
	    break;
	  case GXcopy:
	    rop = RROP_BLACK;
	    break;
	  case GXandInverted:
	    rop = RROP_NOP;
	    break;
	  case GXnoop:
	    rop = RROP_NOP;
	    break;
	  case GXxor:
	    rop = RROP_NOP;
	    break;
	  case GXor:
	    rop = RROP_NOP;
	    break;
	  case GXnor:
	    rop = RROP_INVERT;
	    break;
	  case GXequiv:
	    rop = RROP_INVERT;
	    break;
	  case GXinvert:
	    rop = RROP_INVERT;
	    break;
	  case GXorReverse:
	    rop = RROP_INVERT;
	    break;
	  case GXcopyInverted:
	    rop = RROP_WHITE;
	    break;
	  case GXorInverted:
	    rop = RROP_WHITE;
	    break;
	  case GXnand:
	    rop = RROP_WHITE;
	    break;
	  case GXset:
	    rop = RROP_WHITE;
	    break;
	}
    }
    else /* src is white */
    {
	switch(alu)
	{
	  case GXclear:
	    rop = RROP_BLACK;
	    break;
	  case GXand:
	    rop = RROP_NOP;
	    break;
	  case GXandReverse:
	    rop = RROP_INVERT;
	    break;
	  case GXcopy:
	    rop = RROP_WHITE;
	    break;
	  case GXandInverted:
	    rop = RROP_BLACK;
	    break;
	  case GXnoop:
	    rop = RROP_NOP;
	    break;
	  case GXxor:
	    rop = RROP_INVERT;
	    break;
	  case GXor:
	    rop = RROP_WHITE;
	    break;
	  case GXnor:
	    rop = RROP_BLACK;
	    break;
	  case GXequiv:
	    rop = RROP_NOP;
	    break;
	  case GXinvert:
	    rop = RROP_INVERT;
	    break;
	  case GXorReverse:
	    rop = RROP_WHITE;
	    break;
	  case GXcopyInverted:
	    rop = RROP_BLACK;
	    break;
	  case GXorInverted:
	    rop = RROP_NOP;
	    break;
	  case GXnand:
	    rop = RROP_INVERT;
	    break;
	  case GXset:
	    rop = RROP_WHITE;
	    break;
	}
    }
    return rop;
}
