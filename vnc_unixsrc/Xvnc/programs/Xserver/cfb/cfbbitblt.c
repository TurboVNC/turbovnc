/*
 * cfb copy area
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

Author: Keith Packard

*/
/* $XConsortium: cfbbitblt.c,v 5.51 94/05/27 11:00:56 dpw Exp $ */

#include	"X.h"
#include	"Xmd.h"
#include	"Xproto.h"
#include	"gcstruct.h"
#include	"windowstr.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"cfb.h"
#include	"cfbmskbits.h"
#include	"cfb8bit.h"
#include	"fastblt.h"
#define MFB_CONSTS_ONLY
#include	"maskbits.h"

RegionPtr
cfbBitBlt (pSrcDrawable, pDstDrawable,
            pGC, srcx, srcy, width, height, dstx, dsty, doBitBlt, bitPlane)
    register DrawablePtr pSrcDrawable;
    register DrawablePtr pDstDrawable;
    GC *pGC;
    int srcx, srcy;
    int width, height;
    int dstx, dsty;
    void (*doBitBlt)();
    unsigned long bitPlane;
{
    RegionPtr prgnSrcClip;	/* may be a new region, or just a copy */
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
	    if (!((WindowPtr) pSrcDrawable)->parent)
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
		REGION_INIT(pGC->pScreen, &rgnDst, NullBox, 0);
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

	(*doBitBlt) (pSrcDrawable, pDstDrawable, pGC->alu, &rgnDst, pptSrc, pGC->planemask, bitPlane);
	DEALLOCATE_LOCAL(pptSrc);
    }

    prgnExposed = NULL;
    if ( cfbGetGCPrivate(pGC)->fExpose)
    {
	extern RegionPtr    miHandleExposures();

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
    void (*blt)() = cfbDoBitbltGeneral;
    if ((planemask & PMSK) == PMSK) {
	switch (alu) {
	case GXcopy:
	    blt = cfbDoBitbltCopy;
	    break;
	case GXxor:
	    blt = cfbDoBitbltXor;
	    break;
	case GXor:
	    blt = cfbDoBitbltOr;
	    break;
	}
    }
    (*blt) (pSrc, pDst, alu, prgnDst, pptSrc, planemask);
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
    void (*doBitBlt) ();
    
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
cfbCopyPlane1to8 (pSrcDrawable, pDstDrawable, rop, prgnDst, pptSrc, planemask, bitPlane)
    DrawablePtr pSrcDrawable;	/* must be a bitmap */
    DrawablePtr pDstDrawable;	/* must be depth 8 drawable */
    int	rop;		/* not used; caller must call cfb8CheckOpaqueStipple
			 * beforehand to get cfb8StippleRRop set correctly */
    unsigned long planemask;	/* to apply to destination writes */
    RegionPtr prgnDst;		/* region in destination to draw to;
				 * screen relative coords. if dest is a window;
				 * drawable relative if dest is a pixmap */
    DDXPointPtr pptSrc;		/* drawable relative src coords to copy from;
				 * must be one point for each box in prgnDst */
    unsigned long   bitPlane;	/* not used; assumed always to be 1 */
{
    int	srcx, srcy;	/* upper left corner of box being copied in source */
    int dstx, dsty;	/* upper left corner of box being copied in dest */
    int width, height;	/* in pixels, unpadded, of box being copied */
    int xoffSrc; /* bit # in leftmost word of row from which copying starts */
    int xoffDst; /* byte # in leftmost word of row from which copying starts */
    unsigned long *psrcBase, *pdstBase; /* start of drawable's pixel data */
    int	widthSrc;    /* # of groups of 32 pixels (1 bit/pixel) in src bitmap*/
    int widthDst;    /* # of groups of 4 pixels (8 bits/pixel) in dst */
    unsigned long *psrcLine, *pdstLine; /* steps a row at a time thru src/dst; 
					 * may point into middle of row */
    register unsigned long *psrc, *pdst; /* steps within the row */
    register unsigned long bits, tmp;	 /* bits from source */
    register int leftShift;
    register int rightShift;
    unsigned long startmask;		/* left edge pixel mask */
    unsigned long endmask;		/* right edge pixel mask */
    register int nlMiddle;   /* number of words in middle of the row to draw */
    register int nl;
    int firstoff;
    int secondoff;
    unsigned long src;
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
# define StoreBits(o,c)	StorePixels(pdst,o,*((unsigned long *)\
			    (((char *) cfb8Pixels) + (c & 0x3c))))
# define StoreRopBits(o,c)  StoreRopPixels(pdst,o, \
    *((unsigned long *) (((char *) cfb8StippleAnd) + (c & 0x3c))), \
    *((unsigned long *) (((char *) cfb8StippleXor) + (c & 0x3c))))
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

#endif

/* shared among all different cfb depths through linker magic */
RegionPtr   (*cfbPuntCopyPlane)();

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
    extern RegionPtr    miHandleExposures();
    void		(*doBitBlt)();

#if PSZ == 8

    if (pSrcDrawable->bitsPerPixel == 1 && pDstDrawable->bitsPerPixel == 8)
    {
    	if (bitPlane == 1)
	{
       	    doBitBlt = cfbCopyPlane1to8;
	    cfb8CheckOpaqueStipple (pGC->alu,
				    pGC->fgPixel, pGC->bgPixel,
				    pGC->planemask);
    	    ret = cfbBitBlt (pSrcDrawable, pDstDrawable,
	    	    pGC, srcx, srcy, width, height, dstx, dsty, doBitBlt, bitPlane);
	}
	else
	    ret = miHandleExposures (pSrcDrawable, pDstDrawable,
	    	pGC, srcx, srcy, width, height, dstx, dsty, bitPlane);
    }
    else if (pSrcDrawable->bitsPerPixel == 8 && pDstDrawable->bitsPerPixel == 1)
    {
	extern	int InverseAlu[16];
	int oldalu;

	oldalu = pGC->alu;
    	if ((pGC->fgPixel & 1) == 0 && (pGC->bgPixel&1) == 1)
	    pGC->alu = InverseAlu[pGC->alu];
    	else if ((pGC->fgPixel & 1) == (pGC->bgPixel & 1))
	    pGC->alu = mfbReduceRop(pGC->alu, pGC->fgPixel);
	ret = cfbBitBlt (pSrcDrawable, pDstDrawable,
			 pGC, srcx, srcy, width, height, dstx, dsty, 
			 cfbCopyPlane8to1, bitPlane);
	pGC->alu = oldalu;
    }
    else if (pSrcDrawable->bitsPerPixel == 8 && pDstDrawable->bitsPerPixel == 8)
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
	 * don't need to set pGC->fgPixel,bgPixel as copyPlane8to1
	 * ignores pixel values, expecting the rop to "do the
	 * right thing", which GXcopy will.
	 */
	ValidateGC ((DrawablePtr) pBitmap, pGC1);
	/* no exposures here, scratch GC's don't get graphics expose */
	(void) cfbBitBlt (pSrcDrawable, (DrawablePtr) pBitmap,
			  pGC1, srcx, srcy, width, height, 0, 0, 
			  cfbCopyPlane8to1, bitPlane);
	cfb8CheckOpaqueStipple (pGC->alu,
				pGC->fgPixel, pGC->bgPixel,
				pGC->planemask);
	/* no exposures here, copy bits from inside a pixmap */
	(void) cfbBitBlt ((DrawablePtr) pBitmap, pDstDrawable, pGC,
			    0, 0, width, height, dstx, dsty, cfbCopyPlane1to8, 1);
	FreeScratchGC (pGC1);
	(*pScreen->DestroyPixmap) (pBitmap);
	/* compute resultant exposures */
	ret = miHandleExposures (pSrcDrawable, pDstDrawable, pGC,
				 srcx, srcy, width, height,
				 dstx, dsty, bitPlane);
    }
    else
#endif
    ret = (*cfbPuntCopyPlane) (pSrcDrawable, pDstDrawable,
	    pGC, srcx, srcy, width, height, dstx, dsty, bitPlane);
    return ret;
}
