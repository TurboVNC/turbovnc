/*
 * cfb copy area
 */

/* $XFree86: xc/programs/Xserver/cfb/cfbbitblt.c,v 1.19tsi Exp $ */

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

Author: Keith Packard

*/
/* $Xorg: cfbbitblt.c,v 1.4 2001/02/09 02:04:37 xorgcvs Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include	<X11/X.h>
#include	<X11/Xmd.h>
#include	<X11/Xproto.h>
#include	"gcstruct.h"
#include	"windowstr.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"mi.h"
#include	"cfb.h"
#include	"cfbmskbits.h"
#include	"cfb8bit.h"
#include	"fastblt.h"
#define MFB_CONSTS_ONLY
#include	"maskbits.h"

#if PSZ == 8
#define cfbCopyPlane1toN cfbCopyPlane1to8
#define cfbCopyPlaneNto1 cfbCopyPlane8to1
#else
static unsigned int FgPixel, BgPixel;
# if PSZ == 16
#define cfbCopyPlane1toN cfbCopyPlane1to16
#define cfbCopyPlaneNto1 cfbCopyPlane16to1
# endif
# if PSZ == 24
#define cfbCopyPlane1toN cfbCopyPlane1to24
#define cfbCopyPlaneNto1 cfbCopyPlane24to1
# endif
# if PSZ == 32
#define cfbCopyPlane1toN cfbCopyPlane1to32
#define cfbCopyPlaneNto1 cfbCopyPlane32to1
# endif
#endif

/* cfbBitBltcfb == cfbCopyPlaneExpand */
RegionPtr
cfbBitBlt (
    register DrawablePtr pSrcDrawable,
    register DrawablePtr pDstDrawable,
    GC *pGC,
    int srcx, int srcy,
    int width, int height,
    int dstx, int dsty,
    void (*doBitBlt)(
        DrawablePtr /*pSrc*/,
        DrawablePtr /*pDst*/,
        int /*alu*/,
        RegionPtr /*prgnDst*/,
        DDXPointPtr /*pptSrc*/,
        unsigned long /*planemask*/),
    unsigned long bitPlane)
{
    RegionPtr prgnSrcClip = NULL; /* may be a new region, or just a copy */
    Bool freeSrcClip = FALSE;

    RegionPtr prgnExposed;
    RegionRec rgnDst;
    DDXPointPtr pptSrc;
    register DDXPointPtr ppt;
    register BoxPtr pbox;
    int i;
    register int dx;
    register int dy;
    xRectangle origSource;
    DDXPointRec origDest;
    int numRects;
    BoxRec fastBox;
    int fastClip = 0;		/* for fast clipping with pixmap source */
    int fastExpose = 0;		/* for fast exposures with pixmap source */

    origSource.x = srcx;
    origSource.y = srcy;
    origSource.width = width;
    origSource.height = height;
    origDest.x = dstx;
    origDest.y = dsty;

    if ((pSrcDrawable != pDstDrawable) &&
	pSrcDrawable->pScreen->SourceValidate)
    {
	(*pSrcDrawable->pScreen->SourceValidate) (pSrcDrawable, srcx, srcy, width, height);
    }

    srcx += pSrcDrawable->x;
    srcy += pSrcDrawable->y;

    /* clip the source */

    if (pSrcDrawable->type == DRAWABLE_PIXMAP)
    {
	if ((pSrcDrawable == pDstDrawable) &&
	    (pGC->clientClipType == CT_NONE))
	{
	    prgnSrcClip = cfbGetCompositeClip(pGC);
	}
	else
	{
	    fastClip = 1;
	}
    }
    else
    {
	if (pGC->subWindowMode == IncludeInferiors)
	{
	    /*
	     * XFree86 DDX empties the border clip when the
	     * VT is inactive
	     */
	    if (!((WindowPtr) pSrcDrawable)->parent &&
		REGION_NOTEMPTY (pSrcDrawable->pScreen,
				 &((WindowPtr) pSrcDrawable)->borderClip))
	    {
		/*
		 * special case bitblt from root window in
		 * IncludeInferiors mode; just like from a pixmap
		 */
		fastClip = 1;
	    }
	    else if ((pSrcDrawable == pDstDrawable) &&
		(pGC->clientClipType == CT_NONE))
	    {
		prgnSrcClip = cfbGetCompositeClip(pGC);
	    }
	    else
	    {
		prgnSrcClip = NotClippedByChildren((WindowPtr)pSrcDrawable);
		freeSrcClip = TRUE;
	    }
	}
	else
	{
	    prgnSrcClip = &((WindowPtr)pSrcDrawable)->clipList;
	}
    }

    fastBox.x1 = srcx;
    fastBox.y1 = srcy;
    fastBox.x2 = srcx + width;
    fastBox.y2 = srcy + height;

    /* Don't create a source region if we are doing a fast clip */
    if (fastClip)
    {
	fastExpose = 1;
	/*
	 * clip the source; if regions extend beyond the source size,
 	 * make sure exposure events get sent
	 */
	if (fastBox.x1 < pSrcDrawable->x)
	{
	    fastBox.x1 = pSrcDrawable->x;
	    fastExpose = 0;
	}
	if (fastBox.y1 < pSrcDrawable->y)
	{
	    fastBox.y1 = pSrcDrawable->y;
	    fastExpose = 0;
	}
	if (fastBox.x2 > pSrcDrawable->x + (int) pSrcDrawable->width)
	{
	    fastBox.x2 = pSrcDrawable->x + (int) pSrcDrawable->width;
	    fastExpose = 0;
	}
	if (fastBox.y2 > pSrcDrawable->y + (int) pSrcDrawable->height)
	{
	    fastBox.y2 = pSrcDrawable->y + (int) pSrcDrawable->height;
	    fastExpose = 0;
	}
    }
    else
    {
	REGION_INIT(pGC->pScreen, &rgnDst, &fastBox, 1);
	REGION_INTERSECT(pGC->pScreen, &rgnDst, &rgnDst, prgnSrcClip);
    }

    dstx += pDstDrawable->x;
    dsty += pDstDrawable->y;

    if (pDstDrawable->type == DRAWABLE_WINDOW)
    {
	if (!((WindowPtr)pDstDrawable)->realized)
	{
	    if (!fastClip)
		REGION_UNINIT(pGC->pScreen, &rgnDst);
	    if (freeSrcClip)
		REGION_DESTROY(pGC->pScreen, prgnSrcClip);
	    return NULL;
	}
    }

    dx = srcx - dstx;
    dy = srcy - dsty;

    /* Translate and clip the dst to the destination composite clip */
    if (fastClip)
    {
	RegionPtr cclip;

        /* Translate the region directly */
        fastBox.x1 -= dx;
        fastBox.x2 -= dx;
        fastBox.y1 -= dy;
        fastBox.y2 -= dy;

	/* If the destination composite clip is one rectangle we can
	   do the clip directly.  Otherwise we have to create a full
	   blown region and call intersect */

	/* XXX because CopyPlane uses this routine for 8-to-1 bit
	 * copies, this next line *must* also correctly fetch the
	 * composite clip from an mfb gc
	 */

	cclip = cfbGetCompositeClip(pGC);
        if (REGION_NUM_RECTS(cclip) == 1)
        {
	    BoxPtr pBox = REGION_RECTS(cclip);

	    if (fastBox.x1 < pBox->x1) fastBox.x1 = pBox->x1;
	    if (fastBox.x2 > pBox->x2) fastBox.x2 = pBox->x2;
	    if (fastBox.y1 < pBox->y1) fastBox.y1 = pBox->y1;
	    if (fastBox.y2 > pBox->y2) fastBox.y2 = pBox->y2;

	    /* Check to see if the region is empty */
	    if (fastBox.x1 >= fastBox.x2 || fastBox.y1 >= fastBox.y2)
	    {
		REGION_NULL(pGC->pScreen, &rgnDst);
	    }
	    else
	    {
		REGION_INIT(pGC->pScreen, &rgnDst, &fastBox, 1);
	    }
	}
        else
	{
	    /* We must turn off fastClip now, since we must create
	       a full blown region.  It is intersected with the
	       composite clip below. */
	    fastClip = 0;
	    REGION_INIT(pGC->pScreen, &rgnDst, &fastBox,1);
	}
    }
    else
    {
        REGION_TRANSLATE(pGC->pScreen, &rgnDst, -dx, -dy);
    }

    if (!fastClip)
    {
	REGION_INTERSECT(pGC->pScreen, &rgnDst,
				   &rgnDst,
				   cfbGetCompositeClip(pGC));
    }

    /* Do bit blitting */
    numRects = REGION_NUM_RECTS(&rgnDst);
    if (numRects && width && height)
    {
	if(!(pptSrc = (DDXPointPtr)ALLOCATE_LOCAL(numRects *
						  sizeof(DDXPointRec))))
	{
	    REGION_UNINIT(pGC->pScreen, &rgnDst);
	    if (freeSrcClip)
		REGION_DESTROY(pGC->pScreen, prgnSrcClip);
	    return NULL;
	}
	pbox = REGION_RECTS(&rgnDst);
	ppt = pptSrc;
	for (i = numRects; --i >= 0; pbox++, ppt++)
	{
	    ppt->x = pbox->x1 + dx;
	    ppt->y = pbox->y1 + dy;
	}

	(*doBitBlt) (pSrcDrawable, pDstDrawable, pGC->alu, &rgnDst, pptSrc, pGC->planemask);
	DEALLOCATE_LOCAL(pptSrc);
    }

    prgnExposed = NULL;
    if (pGC->fExpose)
    {
        /* Pixmap sources generate a NoExposed (we return NULL to do this) */
        if (!fastExpose)
	    prgnExposed =
		miHandleExposures(pSrcDrawable, pDstDrawable, pGC,
				  origSource.x, origSource.y,
				  (int)origSource.width,
				  (int)origSource.height,
				  origDest.x, origDest.y, bitPlane);
    }
    REGION_UNINIT(pGC->pScreen, &rgnDst);
    if (freeSrcClip)
	REGION_DESTROY(pGC->pScreen, prgnSrcClip);
    return prgnExposed;
}


RegionPtr
cfbCopyPlaneReduce (
    register DrawablePtr pSrcDrawable,
    register DrawablePtr pDstDrawable,
    GC *pGC,
    int srcx, int srcy,
    int width, int height,
    int dstx, int dsty,
    void (*doCopyPlane)(
        DrawablePtr /*pSrc*/,
        DrawablePtr /*pDst*/,
        int /*alu*/,
        RegionPtr /*prgnDst*/,
        DDXPointPtr /*pptSrc*/,
        unsigned long /*planemask*/,
        unsigned long /*bitPlane*/),
    unsigned long bitPlane)
{
    RegionPtr prgnSrcClip = NULL; /* may be a new region, or just a copy */
    Bool freeSrcClip = FALSE;

    RegionPtr prgnExposed;
    RegionRec rgnDst;
    DDXPointPtr pptSrc;
    register DDXPointPtr ppt;
    register BoxPtr pbox;
    int i;
    register int dx;
    register int dy;
    xRectangle origSource;
    DDXPointRec origDest;
    int numRects;
    BoxRec fastBox;
    int fastClip = 0;		/* for fast clipping with pixmap source */
    int fastExpose = 0;		/* for fast exposures with pixmap source */

    origSource.x = srcx;
    origSource.y = srcy;
    origSource.width = width;
    origSource.height = height;
    origDest.x = dstx;
    origDest.y = dsty;

    if ((pSrcDrawable != pDstDrawable) &&
	pSrcDrawable->pScreen->SourceValidate)
    {
	(*pSrcDrawable->pScreen->SourceValidate) (pSrcDrawable, srcx, srcy, width, height);
    }

    srcx += pSrcDrawable->x;
    srcy += pSrcDrawable->y;

    /* clip the source */

    if (pSrcDrawable->type == DRAWABLE_PIXMAP)
    {
	if ((pSrcDrawable == pDstDrawable) &&
	    (pGC->clientClipType == CT_NONE))
	{
	    prgnSrcClip = cfbGetCompositeClip(pGC);
	}
	else
	{
	    fastClip = 1;
	}
    }
    else
    {
	if (pGC->subWindowMode == IncludeInferiors)
	{
	    /*
	     * XFree86 DDX empties the border clip when the
	     * VT is inactive
	     */
	    if (!((WindowPtr) pSrcDrawable)->parent &&
		REGION_NOTEMPTY (pSrcDrawable->pScreen,
				 &((WindowPtr) pSrcDrawable)->borderClip))
	    {
		/*
		 * special case bitblt from root window in
		 * IncludeInferiors mode; just like from a pixmap
		 */
		fastClip = 1;
	    }
	    else if ((pSrcDrawable == pDstDrawable) &&
		(pGC->clientClipType == CT_NONE))
	    {
		prgnSrcClip = cfbGetCompositeClip(pGC);
	    }
	    else
	    {
		prgnSrcClip = NotClippedByChildren((WindowPtr)pSrcDrawable);
		freeSrcClip = TRUE;
	    }
	}
	else
	{
	    prgnSrcClip = &((WindowPtr)pSrcDrawable)->clipList;
	}
    }

    fastBox.x1 = srcx;
    fastBox.y1 = srcy;
    fastBox.x2 = srcx + width;
    fastBox.y2 = srcy + height;

    /* Don't create a source region if we are doing a fast clip */
    if (fastClip)
    {
	fastExpose = 1;
	/*
	 * clip the source; if regions extend beyond the source size,
 	 * make sure exposure events get sent
	 */
	if (fastBox.x1 < pSrcDrawable->x)
	{
	    fastBox.x1 = pSrcDrawable->x;
	    fastExpose = 0;
	}
	if (fastBox.y1 < pSrcDrawable->y)
	{
	    fastBox.y1 = pSrcDrawable->y;
	    fastExpose = 0;
	}
	if (fastBox.x2 > pSrcDrawable->x + (int) pSrcDrawable->width)
	{
	    fastBox.x2 = pSrcDrawable->x + (int) pSrcDrawable->width;
	    fastExpose = 0;
	}
	if (fastBox.y2 > pSrcDrawable->y + (int) pSrcDrawable->height)
	{
	    fastBox.y2 = pSrcDrawable->y + (int) pSrcDrawable->height;
	    fastExpose = 0;
	}
    }
    else
    {
	REGION_INIT(pGC->pScreen, &rgnDst, &fastBox, 1);
	REGION_INTERSECT(pGC->pScreen, &rgnDst, &rgnDst, prgnSrcClip);
    }

    dstx += pDstDrawable->x;
    dsty += pDstDrawable->y;

    if (pDstDrawable->type == DRAWABLE_WINDOW)
    {
	if (!((WindowPtr)pDstDrawable)->realized)
	{
	    if (!fastClip)
		REGION_UNINIT(pGC->pScreen, &rgnDst);
	    if (freeSrcClip)
		REGION_DESTROY(pGC->pScreen, prgnSrcClip);
	    return NULL;
	}
    }

    dx = srcx - dstx;
    dy = srcy - dsty;

    /* Translate and clip the dst to the destination composite clip */
    if (fastClip)
    {
	RegionPtr cclip;

        /* Translate the region directly */
        fastBox.x1 -= dx;
        fastBox.x2 -= dx;
        fastBox.y1 -= dy;
        fastBox.y2 -= dy;

	/* If the destination composite clip is one rectangle we can
	   do the clip directly.  Otherwise we have to create a full
	   blown region and call intersect */

	/* XXX because CopyPlane uses this routine for 8-to-1 bit
	 * copies, this next line *must* also correctly fetch the
	 * composite clip from an mfb gc
	 */

	cclip = cfbGetCompositeClip(pGC);
        if (REGION_NUM_RECTS(cclip) == 1)
        {
	    BoxPtr pBox = REGION_RECTS(cclip);

	    if (fastBox.x1 < pBox->x1) fastBox.x1 = pBox->x1;
	    if (fastBox.x2 > pBox->x2) fastBox.x2 = pBox->x2;
	    if (fastBox.y1 < pBox->y1) fastBox.y1 = pBox->y1;
	    if (fastBox.y2 > pBox->y2) fastBox.y2 = pBox->y2;

	    /* Check to see if the region is empty */
	    if (fastBox.x1 >= fastBox.x2 || fastBox.y1 >= fastBox.y2)
	    {
		REGION_NULL(pGC->pScreen, &rgnDst);
	    }
	    else
	    {
		REGION_INIT(pGC->pScreen, &rgnDst, &fastBox, 1);
	    }
	}
        else
	{
	    /* We must turn off fastClip now, since we must create
	       a full blown region.  It is intersected with the
	       composite clip below. */
	    fastClip = 0;
	    REGION_INIT(pGC->pScreen, &rgnDst, &fastBox, 1);
	}
    }
    else
    {
        REGION_TRANSLATE(pGC->pScreen, &rgnDst, -dx, -dy);
    }

    if (!fastClip)
    {
	REGION_INTERSECT(pGC->pScreen, &rgnDst,
				   &rgnDst,
				   cfbGetCompositeClip(pGC));
    }

    /* Do bit blitting */
    numRects = REGION_NUM_RECTS(&rgnDst);
    if (numRects && width && height)
    {
	if(!(pptSrc = (DDXPointPtr)ALLOCATE_LOCAL(numRects *
						  sizeof(DDXPointRec))))
	{
	    REGION_UNINIT(pGC->pScreen, &rgnDst);
	    if (freeSrcClip)
		REGION_DESTROY(pGC->pScreen, prgnSrcClip);
	    return NULL;
	}
	pbox = REGION_RECTS(&rgnDst);
	ppt = pptSrc;
	for (i = numRects; --i >= 0; pbox++, ppt++)
	{
	    ppt->x = pbox->x1 + dx;
	    ppt->y = pbox->y1 + dy;
	}

	(*doCopyPlane) (pSrcDrawable, pDstDrawable, pGC->alu, &rgnDst, pptSrc, pGC->planemask, bitPlane);
	DEALLOCATE_LOCAL(pptSrc);
    }

    prgnExposed = NULL;
    if (pGC->fExpose)
    {
        /* Pixmap sources generate a NoExposed (we return NULL to do this) */
        if (!fastExpose)
	    prgnExposed =
		miHandleExposures(pSrcDrawable, pDstDrawable, pGC,
				  origSource.x, origSource.y,
				  (int)origSource.width,
				  (int)origSource.height,
				  origDest.x, origDest.y, bitPlane);
    }
    REGION_UNINIT(pGC->pScreen, &rgnDst);
    if (freeSrcClip)
	REGION_DESTROY(pGC->pScreen, prgnSrcClip);
    return prgnExposed;
}


void
cfbDoBitblt (pSrc, pDst, alu, prgnDst, pptSrc, planemask)
    DrawablePtr	    pSrc, pDst;
    int		    alu;
    RegionPtr	    prgnDst;
    DDXPointPtr	    pptSrc;
    unsigned long   planemask;
{
    void (*doBitBlt)(
        DrawablePtr /*pSrc*/,
        DrawablePtr /*pDst*/,
        int /*alu*/,
        RegionPtr /*prgnDst*/,
        DDXPointPtr /*pptSrc*/,
        unsigned long /*planemask*/)
        = cfbDoBitbltGeneral;

    if ((planemask & PMSK) == PMSK) {
	switch (alu) {
	case GXcopy:
	    doBitBlt = cfbDoBitbltCopy;
	    break;
	case GXxor:
	    doBitBlt = cfbDoBitbltXor;
	    break;
	case GXor:
	    doBitBlt = cfbDoBitbltOr;
	    break;
	}
    }
    (*doBitBlt) (pSrc, pDst, alu, prgnDst, pptSrc, planemask);
}

RegionPtr
cfbCopyArea(pSrcDrawable, pDstDrawable,
            pGC, srcx, srcy, width, height, dstx, dsty)
    register DrawablePtr pSrcDrawable;
    register DrawablePtr pDstDrawable;
    GC *pGC;
    int srcx, srcy;
    int width, height;
    int dstx, dsty;
{
    void (*doBitBlt) (
        DrawablePtr /*pSrc*/,
        DrawablePtr /*pDst*/,
        int /*alu*/,
        RegionPtr /*prgnDst*/,
        DDXPointPtr /*pptSrc*/,
        unsigned long /*planemask*/);
    
    doBitBlt = cfbDoBitbltCopy;
    if (pGC->alu != GXcopy || (pGC->planemask & PMSK) != PMSK)
    {
	doBitBlt = cfbDoBitbltGeneral;
	if ((pGC->planemask & PMSK) == PMSK)
	{
	    switch (pGC->alu) {
	    case GXxor:
		doBitBlt = cfbDoBitbltXor;
		break;
	    case GXor:
		doBitBlt = cfbDoBitbltOr;
		break;
	    }
	}
    }
    return cfbBitBlt (pSrcDrawable, pDstDrawable,
            pGC, srcx, srcy, width, height, dstx, dsty, doBitBlt, 0L);
}

#if PSZ == 8
void
cfbCopyPlane1to8 (pSrcDrawable, pDstDrawable, rop, prgnDst, pptSrc, planemask)
    DrawablePtr pSrcDrawable;	/* must be a bitmap */
    DrawablePtr pDstDrawable;	/* must be depth 8 drawable */
    int	rop;		/* not used; caller must call cfb8CheckOpaqueStipple
			 * beforehand to get cfb8StippleRRop set correctly */
    RegionPtr prgnDst;		/* region in destination to draw to;
				 * screen relative coords. if dest is a window;
				 * drawable relative if dest is a pixmap */
    DDXPointPtr pptSrc;		/* drawable relative src coords to copy from;
				 * must be one point for each box in prgnDst */
    unsigned long planemask;	/* to apply to destination writes */
{
    int	srcx, srcy;	/* upper left corner of box being copied in source */
    int dstx, dsty;	/* upper left corner of box being copied in dest */
    int width, height;	/* in pixels, unpadded, of box being copied */
    int xoffSrc; /* bit # in leftmost word of row from which copying starts */
    int xoffDst; /* byte # in leftmost word of row from which copying starts */
    CfbBits *psrcBase, *pdstBase; /* start of drawable's pixel data */
    int	widthSrc;    /* # of groups of 32 pixels (1 bit/pixel) in src bitmap*/
    int widthDst;    /* # of groups of 4 pixels (8 bits/pixel) in dst */
    CfbBits *psrcLine, *pdstLine; /* steps a row at a time thru src/dst; 
					 * may point into middle of row */
    register CfbBits *psrc, *pdst; /* steps within the row */
    register CfbBits bits, tmp;	 /* bits from source */
    register int leftShift;
    register int rightShift;
    CfbBits startmask;		/* left edge pixel mask */
    CfbBits endmask;		/* right edge pixel mask */
    register int nlMiddle;   /* number of words in middle of the row to draw */
    register int nl;
    int firstoff = 0;
    int secondoff = 0;
    CfbBits src;
    int nbox;		/* number of boxes in region to copy */
    BoxPtr  pbox;	/* steps thru boxes in region */
    int pixelsRemainingOnRightEdge; /* # pixels to be drawn on a row after
				     * the main "middle" loop */

    cfbGetLongWidthAndPointer (pSrcDrawable, widthSrc, psrcBase)
    cfbGetLongWidthAndPointer (pDstDrawable, widthDst, pdstBase)

    nbox = REGION_NUM_RECTS(prgnDst);
    pbox = REGION_RECTS(prgnDst);
    while (nbox--)
    {
	dstx = pbox->x1;
	dsty = pbox->y1;
	srcx = pptSrc->x;
	srcy = pptSrc->y;
	width = pbox->x2 - pbox->x1;
	height = pbox->y2 - pbox->y1;
	pbox++;
	pptSrc++;

	psrcLine = psrcBase + srcy * widthSrc + (srcx >> MFB_PWSH);
	pdstLine = pdstBase + dsty * widthDst + (dstx >> PWSH);
	xoffSrc = srcx & MFB_PIM; /* finds starting bit in src */
	xoffDst = dstx & PIM;  /* finds starting byte in dst */

	/* compute startmask, endmask, nlMiddle */

	if (xoffDst + width < PPW) /* XXX should this be '<= PPW' ? */
	{ /* the copy only affects one word per row in destination */
	    maskpartialbits(dstx, width, startmask);
	    endmask = 0;  /* nothing on right edge */
	    nlMiddle = 0; /* nothing in middle */
	}
	else
	{ /* the copy will affect multiple words per row in destination */
	    maskbits(dstx, width, startmask, endmask, nlMiddle);
	}

	/*
	 * compute constants for the first four bits to be
	 * copied.  This avoids troubles with partial first
	 * writes, and difficult shift computation
	 */
	if (startmask)
	{
	    firstoff = xoffSrc - xoffDst;
	    if (firstoff > (MFB_PPW-PPW))
		secondoff = MFB_PPW - firstoff;
	    if (xoffDst)
	    {
	    	srcx += (PPW-xoffDst);
	    	xoffSrc = srcx & MFB_PIM;
	    }
	}
	leftShift = xoffSrc;
	rightShift = MFB_PPW - leftShift;

	pixelsRemainingOnRightEdge = (nlMiddle & 7) * PPW +
	    				((dstx + width) & PIM);

	/* setup is done; now let's move some bits */

	/* caller must call cfb8CheckOpaqueStipple before this function
	 * to set cfb8StippleRRop!
	 */

	if (cfb8StippleRRop == GXcopy)
	{
	    while (height--)
	    { /* one iteration of this loop copies one row */
	    	psrc = psrcLine;
	    	pdst = pdstLine;
	    	psrcLine += widthSrc;
	    	pdstLine += widthDst;
	    	bits = *psrc++;
	    	if (startmask)
	    	{
		    if (firstoff < 0)
		    	tmp = BitRight (bits, -firstoff);
		    else
		    {
		    	tmp = BitLeft (bits, firstoff);
			/*
			 * need a more cautious test for partialmask
			 * case...
			 */
		    	if (firstoff >= (MFB_PPW-PPW))
		    	{
			    bits = *psrc++;
			    if (firstoff != (MFB_PPW-PPW))
				tmp |= BitRight (bits, secondoff);
		    	}
		    }
		    *pdst = (*pdst & ~startmask) | (GetPixelGroup(tmp) & startmask);
		    pdst++;
	    	}
	    	nl = nlMiddle;
	    	while (nl >= 8)
	    	{
		    nl -= 8;
		    tmp = BitLeft(bits, leftShift);
		    bits = *psrc++;
		    if (rightShift != MFB_PPW)
		    	tmp |= BitRight(bits, rightShift);

#ifdef FAST_CONSTANT_OFFSET_MODE
# define StorePixels(pdst,o,pixels)	(pdst)[o] = (pixels)
# define EndStep(pdst,o)		(pdst) += (o)
# define StoreRopPixels(pdst,o,and,xor)	(pdst)[o] = DoRRop((pdst)[o],and,xor);
#else
# define StorePixels(pdst,o,pixels)	*(pdst)++ = (pixels)
# define EndStep(pdst,o)
# define StoreRopPixels(pdst,o,and,xor)	*(pdst) = DoRRop(*(pdst),and,xor); (pdst)++;
#endif

#define Step(c)			NextBitGroup(c);
#define StoreBitsPlain(o,c)	StorePixels(pdst,o,GetPixelGroup(c))
#define StoreRopBitsPlain(o,c)	StoreRopPixels(pdst,o,\
					cfb8StippleAnd[GetBitGroup(c)], \
					cfb8StippleXor[GetBitGroup(c)])
#define StoreBits0(c)		StoreBitsPlain(0,c)
#define StoreRopBits0(c)	StoreRopBitsPlain(0,c)

#if (BITMAP_BIT_ORDER == MSBFirst)
# define StoreBits(o,c)	StoreBitsPlain(o,c)
# define StoreRopBits(o,c)  StoreRopBitsPlain(o,c)
# define FirstStep(c)	Step(c)
#else /* BITMAP_BIT_ORDER == LSBFirst */
#if PGSZ == 64
# define StoreBits(o,c)	StorePixels(pdst,o, (cfb8Pixels[c & 0xff]))
# define StoreRopBits(o,c)  StoreRopPixels(pdst,o, \
    (cfb8StippleAnd[c & 0xff]), \
    (cfb8StippleXor[c & 0xff]))
# define FirstStep(c)	c = BitLeft (c, 8);
#else
/* 0x3c is 0xf << 2 (4 bits, long word) */
# define StoreBits(o,c)	StorePixels(pdst,o,*((CfbBits *)\
			    (((char *) cfb8Pixels) + (c & 0x3c))))
# define StoreRopBits(o,c)  StoreRopPixels(pdst,o, \
    *((CfbBits *) (((char *) cfb8StippleAnd) + (c & 0x3c))), \
    *((CfbBits *) (((char *) cfb8StippleXor) + (c & 0x3c))))
# define FirstStep(c)	c = BitLeft (c, 2);
#endif /* PGSZ */
#endif /* BITMAP_BIT_ORDER */

		    StoreBits0(tmp);	FirstStep(tmp);
		    StoreBits(1,tmp);	Step(tmp);
		    StoreBits(2,tmp);	Step(tmp);
		    StoreBits(3,tmp);	Step(tmp);
		    StoreBits(4,tmp);	Step(tmp);
		    StoreBits(5,tmp);	Step(tmp);
		    StoreBits(6,tmp);	Step(tmp);
		    StoreBits(7,tmp);   EndStep (pdst,8);
	    	}

		/* do rest of middle and partial word on right edge */

	    	if (pixelsRemainingOnRightEdge)
	    	{
		    tmp = BitLeft(bits, leftShift);

		    if (pixelsRemainingOnRightEdge > rightShift)
		    {
		    	bits = *psrc++;
		    	tmp |= BitRight (bits, rightShift);
		    }
		    EndStep (pdst, nl);
		    switch (nl)
		    {
		    case 7:
			StoreBitsPlain(-7,tmp);	Step(tmp);
		    case 6:
			StoreBitsPlain(-6,tmp);	Step(tmp);
		    case 5:
			StoreBitsPlain(-5,tmp);	Step(tmp);
		    case 4:
			StoreBitsPlain(-4,tmp);	Step(tmp);
		    case 3:
			StoreBitsPlain(-3,tmp);	Step(tmp);
		    case 2:
			StoreBitsPlain(-2,tmp);	Step(tmp);
		    case 1:
			StoreBitsPlain(-1,tmp);	Step(tmp);
		    }
		    if (endmask)
		    	*pdst = (*pdst & ~endmask) | (GetPixelGroup(tmp) & endmask);
	    	}
	    }
	}
	else /* cfb8StippleRRop != GXcopy */
	{
	    while (height--)
	    { /* one iteration of this loop copies one row */
	    	psrc = psrcLine;
	    	pdst = pdstLine;
	    	psrcLine += widthSrc;
	    	pdstLine += widthDst;
	    	bits = *psrc++;

		/* do partial word on left edge */

	    	if (startmask)
	    	{
		    if (firstoff < 0)
		    	tmp = BitRight (bits, -firstoff);
		    else
		    {
		    	tmp = BitLeft (bits, firstoff);
		    	if (firstoff >= (MFB_PPW-PPW))
		    	{
			    bits = *psrc++;
			    if (firstoff != (MFB_PPW-PPW))
				tmp |= BitRight (bits, secondoff);
		    	}
		    }
		    src = GetBitGroup(tmp);
		    *pdst = MaskRRopPixels (*pdst, src, startmask);
		    pdst++;
	    	}

		/* do middle of row */

	    	nl = nlMiddle;
		while (nl >= 8)
		{
		    nl -= 8;
		    tmp = BitLeft(bits, leftShift);
		    bits = *psrc++;
		    if (rightShift != MFB_PPW)
			tmp |= BitRight(bits, rightShift);
		    StoreRopBits0(tmp);		FirstStep(tmp);
		    StoreRopBits(1,tmp);	Step(tmp);
		    StoreRopBits(2,tmp);	Step(tmp);
		    StoreRopBits(3,tmp);	Step(tmp);
		    StoreRopBits(4,tmp);	Step(tmp);
		    StoreRopBits(5,tmp);	Step(tmp);
		    StoreRopBits(6,tmp);	Step(tmp);
		    StoreRopBits(7,tmp);	EndStep(pdst,8);
		}

		/* do rest of middle and partial word on right edge */

	    	if (pixelsRemainingOnRightEdge)
	    	{
		    tmp = BitLeft(bits, leftShift);

		    if (pixelsRemainingOnRightEdge > rightShift)
		    {
		    	bits = *psrc++; /* XXX purify abr here */
		    	tmp |= BitRight (bits, rightShift);
		    }
		    while (nl--)
		    {
			src = GetBitGroup (tmp);
			*pdst = RRopPixels (*pdst, src);
		    	pdst++;
			NextBitGroup(tmp);
		    }
		    if (endmask)
		    {
			src = GetBitGroup (tmp);
			*pdst = MaskRRopPixels (*pdst, src, endmask);
		    }
	    	}
	    } /* end copy one row */
	} /* end alu is non-copy-mode case */
    } /* end iteration over region boxes */
}

#else /* PSZ == 8 */

#define mfbmaskbits(x, w, startmask, endmask, nlw) \
    startmask = mfbGetstarttab((x)&0x1f); \
    endmask = mfbGetendtab(((x)+(w)) & 0x1f); \
    if (startmask) \
	nlw = (((w) - (32 - ((x)&0x1f))) >> 5); \
    else \
	nlw = (w) >> 5;

#define mfbmaskpartialbits(x, w, mask) \
    mask = mfbGetpartmasks((x)&0x1f,(w)&0x1f);

#define LeftMost    0
#define StepBit(bit, inc)  ((bit) += (inc))


#define GetBits(psrc, nBits, curBit, bitPos, bits) {\
    bits = 0; \
    while (nBits--) \
    { \
	bits |= ((*psrc++ >> bitPos) & 1) << curBit; \
	StepBit (curBit, 1); \
    } \
}

/******************************************************************/

static void
#if PSZ == 16
cfbCopyPlane1to16
#endif
#if PSZ == 24
cfbCopyPlane1to24
#endif
#if PSZ == 32
cfbCopyPlane1to32
#endif
(
    DrawablePtr pSrcDrawable,
    DrawablePtr pDstDrawable,
    int	rop,
    RegionPtr prgnDst,
    DDXPointPtr pptSrc,
    unsigned long planemask)
{
    int	srcx, srcy, dstx, dsty;
    int width, height;
    int xoffSrc;
    CfbBits *psrcBase, *pdstBase;
    int	widthSrc, widthDst;
    unsigned int *psrcLine;
    register unsigned int *psrc;
#if PSZ == 16
    unsigned short *pdstLine;
    register unsigned short *pdst;
#endif
#if PSZ == 32
    unsigned int *pdstLine;
    register unsigned int *pdst;
#endif
#if PSZ == 24
    unsigned char *pdstLine;
    register unsigned char *pdst;
#endif
    register unsigned int  bits, tmp;
    register unsigned int  fgpixel, bgpixel;
    register unsigned int  src;
#if PSZ == 24
    register unsigned int  dst;
#endif
    register int  leftShift, rightShift;
    register int  i, nl;
    int nbox;
    BoxPtr pbox;
    int  result;

#if PSZ == 16
    unsigned int doublet[4];	/* Pixel values for 16bpp expansion. */
#endif
#if PSZ == 32
    unsigned int doublet[8];	/* Pixel values for 32bpp expansion */
#endif

    fgpixel = FgPixel & planemask;
    bgpixel = BgPixel & planemask;    

#if PSZ == 16
    if (rop == GXcopy && (planemask & PMSK) == PMSK) {
        doublet[0] = bgpixel | (bgpixel << 16);
        doublet[1] = fgpixel | (bgpixel << 16);
        doublet[2] = bgpixel | (fgpixel << 16);
        doublet[3] = fgpixel | (fgpixel << 16);
    }
#endif
#if PSZ == 32
    if (rop == GXcopy && (planemask & PMSK) == PMSK) {
        doublet[0] = bgpixel; doublet[1] = bgpixel;
        doublet[2] = fgpixel; doublet[3] = bgpixel;
        doublet[4] = bgpixel; doublet[5] = fgpixel;
        doublet[6] = fgpixel; doublet[7] = fgpixel;
    }
#endif

    /* must explicitly ask for "int" widths, as code below expects it */
    /* on some machines (Alpha), "long" and "int" are not the same size */
    cfbGetTypedWidthAndPointer (pSrcDrawable, widthSrc, psrcBase, int, CfbBits)
    cfbGetTypedWidthAndPointer (pDstDrawable, widthDst, pdstBase, int, CfbBits)

#if PSZ == 16
    widthDst <<= 1;
#endif
#if PSZ == 24
    widthDst <<= 2;
#endif

    nbox = REGION_NUM_RECTS(prgnDst);
    pbox = REGION_RECTS(prgnDst);

    while (nbox--)
    {
	dstx = pbox->x1;
	dsty = pbox->y1;
	srcx = pptSrc->x;
	srcy = pptSrc->y;
	width = pbox->x2 - pbox->x1;
	height = pbox->y2 - pbox->y1;
	pbox++;
	pptSrc++;
	psrcLine = (unsigned int *)psrcBase + srcy * widthSrc + (srcx >> 5);
#if PSZ == 16
	pdstLine = (unsigned short *)pdstBase + dsty * widthDst + dstx;
#endif
#if PSZ == 24
	pdstLine = (unsigned char *)pdstBase + dsty * widthDst + dstx * 3;
#endif
#if PSZ == 32
	pdstLine = (unsigned int *)pdstBase + dsty * widthDst + dstx;
#endif
	xoffSrc = srcx & 0x1f;

	/*
	 * compute constants for the first four bits to be
	 * copied.  This avoids troubles with partial first
	 * writes, and difficult shift computation
	 */
	leftShift = xoffSrc;
	rightShift = 32 - leftShift;

	if (rop == GXcopy && (planemask & PMSK) == PMSK)
	{
	    while (height--)
	    {
	        psrc = psrcLine;
	        pdst = pdstLine;
	        psrcLine += widthSrc;
	        pdstLine += widthDst;
	        bits = *psrc++;
	        nl = width;
   	        while (nl >= 32)
	        {
		    tmp = BitLeft(bits, leftShift);
		    bits = *psrc++;
		    if (rightShift != 32)
		        tmp |= BitRight(bits, rightShift);
		    i = 0;
#if PSZ == 16
		    /*
		     * I've thrown in some optimization to at least write
		     * some aligned 32-bit words instead of 16-bit shorts.
		     */
		    if ((unsigned long)psrc & 2) {
		        /* Write unaligned 16-bit word at left edge. */
		        if (tmp & 0x01)
		            *pdst = fgpixel;
		        else
		            *pdst = bgpixel;
		        pdst++;
		        i++;
		    }
		    while (i <= 24)
		    {
		        unsigned tmpbits = tmp >> i;
		        *(unsigned int *)pdst = doublet[tmpbits & 0x03];
	        	*(unsigned int *)(pdst + 2) =
		            doublet[(tmpbits >> 2) & 0x03];
	        	*(unsigned int *)(pdst + 4) =
		            doublet[(tmpbits >> 4) & 0x03];
	        	*(unsigned int *)(pdst + 6) =
		            doublet[(tmpbits >> 6) & 0x03];
		        pdst += 8;	/* Advance four 32-bit words. */
		        i += 8;
		    }
		    while (i <= 30)
		    {
		        *(unsigned int *)pdst =
		            doublet[(tmp >> i) & 0x03];
		        pdst += 2;	/* Advance one 32-bit word. */
		        i += 2;
		    }
		    if (i == 31) {
		        if ((tmp >> 31) & 0x01)
		            *pdst = fgpixel;
		        else
		            *pdst = bgpixel;
		        pdst++;
		    }
#endif		    
#if PSZ == 24
		    while (i < 32) {
		        if ((tmp >> i) & 0x01) {
		            *pdst = fgpixel;
		            *(pdst + 1) = fgpixel >> 8;
		            *(pdst + 2) = fgpixel >> 16;
		        }
		        else {
		            *pdst = bgpixel;
		            *(pdst + 1) = bgpixel >> 8;
		            *(pdst + 2) = bgpixel >> 16;
		        }
		        pdst += 3;
		        i++;
		    }
#endif
#if PSZ == 32
		    while (i <= 28) {
		        int pair;
		        pair = (tmp >> i) & 0x03;
		        *pdst = doublet[pair * 2];
		        *(pdst + 1) = doublet[pair * 2 + 1];
		        pair = (tmp >> (i + 2)) & 0x03;
		        *(pdst + 2) = doublet[pair * 2];
		        *(pdst + 3) = doublet[pair * 2 + 1];
		        pdst += 4;
		        i += 4;
		    }
		    while (i < 32) {
		        *pdst = ((tmp >> i) & 0x01) ? fgpixel : bgpixel;
		        pdst++;
		        i++;
		    }
#endif
		    nl -= 32;
	        }

	        if (nl)
	        {
		    tmp = BitLeft(bits, leftShift);
		    /*
		     * better condition needed -- mustn't run
		     * off the end of the source...
		     */
		    if (rightShift != 32)
		    {
		        bits = *psrc++;
		        tmp |= BitRight (bits, rightShift);
		    }
		    i = 32;
		    while (nl--)
		    {
		        --i;
#if PSZ == 24
		        if ((tmp >> (31 - i)) & 0x01) {
		            *pdst = fgpixel;
		            *(pdst + 1) = fgpixel >> 8;
		            *(pdst + 2) = fgpixel >> 16;
		        }
		        else {
		            *pdst = bgpixel;
		            *(pdst + 1) = bgpixel >> 8;
		            *(pdst + 2) = bgpixel >> 16;
		        }
		        pdst += 3;
#else
		        *pdst = ((tmp >> (31 - i)) & 0x01) ? fgpixel : bgpixel;
		        pdst++;
#endif
		    }
	        }
            }
        }
	else
	{
	    while (height--)
	    {
	        psrc = psrcLine;
	        pdst = pdstLine;
	        psrcLine += widthSrc;
	        pdstLine += widthDst;
	        bits = *psrc++;
	        nl = width;
   	        while (nl >= 32)
	        {
		    tmp = BitLeft(bits, leftShift);
		    bits = *psrc++;
		    if (rightShift != 32)
		        tmp |= BitRight(bits, rightShift);
		    i = 32;
		    while (i--)
		    {
		        src = ((tmp >> (31 - i)) & 0x01) ? fgpixel : bgpixel;
#if PSZ == 24
		        dst = *pdst;
			dst |= (*(pdst + 1)) << 8;
			dst |= (*(pdst + 2)) << 16;
                        DoRop (result, rop, src, dst);
			*pdst = (dst & ~planemask) |
		  	        (result & planemask);
			*(pdst+1) = ((dst & ~planemask) >> 8) |
		  	        ((result & planemask) >> 8);
			*(pdst+2) = ((dst & ~planemask) >> 16) |
		  	        ((result & planemask) >> 16);
			pdst += 3;
#else
                        DoRop (result, rop, src, *pdst);

		        *pdst = (*pdst & ~planemask) |
		  	        (result & planemask);
		        pdst++;
#endif
		    }
		    nl -= 32;
	        }

	        if (nl)
	        {
		    tmp = BitLeft(bits, leftShift);
		    /*
		     * better condition needed -- mustn't run
		     * off the end of the source...
		     */
		    if (rightShift != 32)
		    {
		        bits = *psrc++;
		        tmp |= BitRight (bits, rightShift);
		    }
		    i = 32;
		    while (nl--)
		    {
		        --i;
		        src = ((tmp >> (31 - i)) & 0x01) ? fgpixel : bgpixel;
#if PSZ == 24
		        dst = *pdst;
			dst |= (*(pdst + 1)) << 8;
			dst |= (*(pdst + 2)) << 16;
                        DoRop (result, rop, src, dst);
			*pdst = (dst & ~planemask) |
		  	        (result & planemask);
			*(pdst+1) = ((dst & ~planemask) >> 8) |
		  	        ((result & planemask) >> 8);
			*(pdst+2) = ((dst & ~planemask) >> 16) |
		  	        ((result & planemask) >> 16);
			pdst += 3;
#else
                        DoRop (result, rop, src, *pdst);

		        *pdst = (*pdst & ~planemask) |
		  	        (result & planemask);
		        pdst++;
#endif
		    }
	        }
            }
        }
    }
}

#endif  /* PSZ == 8 */

/* shared among all different cfb depths through linker magic */

RegionPtr cfbCopyPlane(pSrcDrawable, pDstDrawable,
	    pGC, srcx, srcy, width, height, dstx, dsty, bitPlane)
    DrawablePtr 	pSrcDrawable;
    DrawablePtr		pDstDrawable;
    GCPtr		pGC;
    int 		srcx, srcy;
    int 		width, height;
    int 		dstx, dsty;
    unsigned long	bitPlane;
{
    RegionPtr	ret;

#if IMAGE_BYTE_ORDER == LSBFirst

    void (*doCopyPlaneExpand)(
        DrawablePtr /*pSrc*/,
        DrawablePtr /*pDst*/,
        int /*alu*/,
        RegionPtr /*prgnDst*/,
        DDXPointPtr /*pptSrc*/,
        unsigned long /*planemask*/);

    if (pSrcDrawable->bitsPerPixel == 1 && pDstDrawable->bitsPerPixel == PSZ)
    {
    	if (bitPlane == 1)
	{
       	    doCopyPlaneExpand = cfbCopyPlane1toN;
#if PSZ == 8
	    cfb8CheckOpaqueStipple (pGC->alu,
				    pGC->fgPixel, pGC->bgPixel,
				    pGC->planemask);
#else
	    FgPixel = pGC->fgPixel;
	    BgPixel = pGC->bgPixel;
#endif
    	    ret = cfbCopyPlaneExpand (pSrcDrawable, pDstDrawable,
	    	    pGC, srcx, srcy, width, height, dstx, dsty, doCopyPlaneExpand, bitPlane);
	}
	else
	    ret = miHandleExposures (pSrcDrawable, pDstDrawable,
	    	pGC, srcx, srcy, width, height, dstx, dsty, bitPlane);
    }
    else if (pSrcDrawable->bitsPerPixel == PSZ && pDstDrawable->bitsPerPixel == 1)
    {
	int oldalu;

	oldalu = pGC->alu;
    	if ((pGC->fgPixel & 1) == 0 && (pGC->bgPixel&1) == 1)
	    pGC->alu = mfbGetInverseAlu(pGC->alu);
    	else if ((pGC->fgPixel & 1) == (pGC->bgPixel & 1))
	    pGC->alu = mfbReduceRop(pGC->alu, pGC->fgPixel);
	ret = cfbCopyPlaneReduce(pSrcDrawable, pDstDrawable,
			 pGC, srcx, srcy, width, height, dstx, dsty, 
			 cfbCopyPlaneNto1, bitPlane);
	pGC->alu = oldalu;
    }
    else if (pSrcDrawable->bitsPerPixel == PSZ && pDstDrawable->bitsPerPixel == PSZ)
    {
	PixmapPtr	pBitmap;
	ScreenPtr	pScreen = pSrcDrawable->pScreen;
	GCPtr		pGC1;

	pBitmap = (*pScreen->CreatePixmap) (pScreen, width, height, 1);
	if (!pBitmap)
	    return NULL;
	pGC1 = GetScratchGC (1, pScreen);
	if (!pGC1)
	{
	    (*pScreen->DestroyPixmap) (pBitmap);
	    return NULL;
	}
	/*
	 * don't need to set pGC->fgPixel,bgPixel as copyPlaneNto1
	 * ignores pixel values, expecting the rop to "do the
	 * right thing", which GXcopy will.
	 */
	ValidateGC ((DrawablePtr) pBitmap, pGC1);
	/* no exposures here, scratch GC's don't get graphics expose */
	cfbCopyPlaneReduce(pSrcDrawable, (DrawablePtr) pBitmap,
			  pGC1, srcx, srcy, width, height, 0, 0, 
			  cfbCopyPlaneNto1, bitPlane);
#if PSZ == 8
	cfb8CheckOpaqueStipple (pGC->alu,
				pGC->fgPixel, pGC->bgPixel,
				pGC->planemask);
#else
	    FgPixel = pGC->fgPixel;
	    BgPixel = pGC->bgPixel;
#endif
	/* no exposures here, copy bits from inside a pixmap */
	cfbCopyPlaneExpand((DrawablePtr) pBitmap, pDstDrawable, pGC,
			    0, 0, width, height, dstx, dsty, cfbCopyPlane1toN, 1);
	FreeScratchGC (pGC1);
	(*pScreen->DestroyPixmap) (pBitmap);
	/* compute resultant exposures */
	ret = miHandleExposures (pSrcDrawable, pDstDrawable, pGC,
				 srcx, srcy, width, height,
				 dstx, dsty, bitPlane);
    }
    else
#endif
	ret = miCopyPlane (pSrcDrawable, pDstDrawable,
	    pGC, srcx, srcy, width, height, dstx, dsty, bitPlane);
    return ret;
}


