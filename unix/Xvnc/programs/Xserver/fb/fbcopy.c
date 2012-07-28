/*
 * Id: fbcopy.c,v 1.1 1999/11/02 03:54:45 keithp Exp $
 *
 * Copyright © 1998 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/fb/fbcopy.c,v 1.13 2003/11/10 18:21:47 tsi Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdlib.h>

#include "fb.h"
#include "fbmmx.h"

void
fbCopyNtoN (DrawablePtr	pSrcDrawable,
	    DrawablePtr	pDstDrawable,
	    GCPtr	pGC,
	    BoxPtr	pbox,
	    int		nbox,
	    int		dx,
	    int		dy,
	    Bool	reverse,
	    Bool	upsidedown,
	    Pixel	bitplane,
	    void	*closure)
{
    CARD8	alu = pGC ? pGC->alu : GXcopy;
    FbBits	pm = pGC ? fbGetGCPrivate(pGC)->pm : FB_ALLONES;
    FbBits	*src;
    FbStride	srcStride;
    int		srcBpp;
    int		srcXoff, srcYoff;
    FbBits	*dst;
    FbStride	dstStride;
    int		dstBpp;
    int		dstXoff, dstYoff;
    
    fbGetDrawable (pSrcDrawable, src, srcStride, srcBpp, srcXoff, srcYoff);
    fbGetDrawable (pDstDrawable, dst, dstStride, dstBpp, dstXoff, dstYoff);

    while (nbox--)
    {
#ifdef USE_MMX
	if (pm == FB_ALLONES && alu == GXcopy && !reverse &&
	    !upsidedown && fbHaveMMX())
	{
	    if (!fbCopyAreammx (pSrcDrawable,
				pDstDrawable,
				
				(pbox->x1 + dx),
				(pbox->y1 + dy),
				
				(pbox->x1),
				(pbox->y1),
				
				(pbox->x2 - pbox->x1),
				(pbox->y2 - pbox->y1)))
		goto fallback;
	    else
		goto next;
	}
    fallback:
#endif
	fbBlt (src + (pbox->y1 + dy + srcYoff) * srcStride,
	       srcStride,
	       (pbox->x1 + dx + srcXoff) * srcBpp,
	       
	       dst + (pbox->y1 + dstYoff) * dstStride,
	       dstStride,
	       (pbox->x1 + dstXoff) * dstBpp,
	       
	       (pbox->x2 - pbox->x1) * dstBpp,
	       (pbox->y2 - pbox->y1),
	       
	       alu,
	       pm,
	       dstBpp,
	       
	       reverse,
	       upsidedown);
#ifdef USE_MMX
    next:
#endif
	pbox++;
    }    
}

void
fbCopy1toN (DrawablePtr	pSrcDrawable,
	    DrawablePtr	pDstDrawable,
	    GCPtr	pGC,
	    BoxPtr	pbox,
	    int		nbox,
	    int		dx,
	    int		dy,
	    Bool	reverse,
	    Bool	upsidedown,
	    Pixel	bitplane,
	    void	*closure)
{
    FbGCPrivPtr	pPriv = fbGetGCPrivate(pGC);
    FbBits	*src;
    FbStride	srcStride;
    int		srcBpp;
    int		srcXoff, srcYoff;
    FbBits	*dst;
    FbStride	dstStride;
    int		dstBpp;
    int		dstXoff, dstYoff;

    fbGetDrawable (pSrcDrawable, src, srcStride, srcBpp, srcXoff, srcYoff);
    fbGetDrawable (pDstDrawable, dst, dstStride, dstBpp, dstXoff, dstYoff);

    while (nbox--)
    {
	if (dstBpp == 1)
	{
	    fbBlt (src + (pbox->y1 + dy + srcYoff) * srcStride,
		   srcStride,
		   (pbox->x1 + dx + srcXoff) * srcBpp,
    
		   dst + (pbox->y1 + dstYoff) * dstStride,
		   dstStride,
		   (pbox->x1 + dstXoff) * dstBpp,
    
		   (pbox->x2 - pbox->x1) * dstBpp,
		   (pbox->y2 - pbox->y1),
    
		   FbOpaqueStipple1Rop(pGC->alu,
				       pGC->fgPixel,pGC->bgPixel),
		   pPriv->pm,
		   dstBpp,
    
		   reverse,
		   upsidedown);
	}
	else
	{
	    fbBltOne ((FbStip *) (src + (pbox->y1 + dy + srcYoff) * srcStride),
		      srcStride*(FB_UNIT/FB_STIP_UNIT),
		      (pbox->x1 + dx + srcXoff),
    
		      dst + (pbox->y1 + dstYoff) * dstStride,
		      dstStride,
		      (pbox->x1 + dstXoff) * dstBpp,
		      dstBpp,
    
		      (pbox->x2 - pbox->x1) * dstBpp,
		      (pbox->y2 - pbox->y1),
    
		      pPriv->and, pPriv->xor,
		      pPriv->bgand, pPriv->bgxor);
	}
	pbox++;
    }
}

void
fbCopyNto1 (DrawablePtr	pSrcDrawable,
	    DrawablePtr	pDstDrawable,
	    GCPtr	pGC,
	    BoxPtr	pbox,
	    int		nbox,
	    int		dx,
	    int		dy,
	    Bool	reverse,
	    Bool	upsidedown,
	    Pixel	bitplane,
	    void	*closure)
{
    FbGCPrivPtr	pPriv = fbGetGCPrivate (pGC);
    
    while (nbox--)
    {
	if (pDstDrawable->bitsPerPixel == 1)
	{
	    FbBits	*src;
	    FbStride    srcStride;
	    int		srcBpp;
	    int		srcXoff, srcYoff;
    
	    FbStip	*dst;
	    FbStride    dstStride;
	    int		dstBpp;
	    int		dstXoff, dstYoff;
	    
	    fbGetDrawable (pSrcDrawable, src, srcStride, srcBpp, srcXoff, srcYoff);
	    fbGetStipDrawable (pDstDrawable, dst, dstStride, dstBpp, dstXoff, dstYoff);
	    fbBltPlane (src + (pbox->y1+ dy + srcYoff) * srcStride,
			srcStride,
			(pbox->x1 + dx + srcXoff) * srcBpp,
			srcBpp,
    
			dst + (pbox->y1 + dstYoff) * dstStride,
			dstStride,
			(pbox->x1 + dstXoff) * dstBpp,
    
			(pbox->x2 - pbox->x1) * srcBpp,
			(pbox->y2 - pbox->y1),
    
			(FbStip) pPriv->and, (FbStip) pPriv->xor,
			(FbStip) pPriv->bgand, (FbStip) pPriv->bgxor,
			bitplane);
	}
	else
	{
	    FbBits	*src;
	    FbStride    srcStride;
	    int		srcBpp;
	    int         srcXoff, srcYoff;

	    FbBits	*dst;
	    FbStride    dstStride;
	    int		dstBpp;
	    int		dstXoff, dstYoff;
    
	    FbStip	*tmp;
	    FbStride    tmpStride;
	    int		width, height;
	    
	    width = pbox->x2 - pbox->x1;
	    height = pbox->y2 - pbox->y1;
	    
	    tmpStride = ((width + FB_STIP_MASK) >> FB_STIP_SHIFT);
	    tmp = xalloc (tmpStride * height * sizeof (FbStip));
	    if (!tmp)
		return;
	    
	    fbGetDrawable (pSrcDrawable, src, srcStride, srcBpp, srcXoff, srcYoff);
	    fbGetDrawable (pDstDrawable, dst, dstStride, dstBpp, dstXoff, dstYoff);
	    
	    fbBltPlane (src + (pbox->y1+ dy + srcYoff) * srcStride,
			srcStride,
			(pbox->x1 + dx + srcXoff) * srcBpp,
			srcBpp,
    
			tmp,
			tmpStride,
			0,
    
			width * srcBpp,
			height,
    
			fbAndStip(GXcopy,FB_ALLONES,FB_ALLONES),
			fbXorStip(GXcopy,FB_ALLONES,FB_ALLONES),
			fbAndStip(GXcopy,0,FB_ALLONES),
			fbXorStip(GXcopy,0,FB_ALLONES),
			bitplane);
	    fbBltOne (tmp,
		      tmpStride,
		      0,
    
		      dst + (pbox->y1 + dstYoff) * dstStride,
		      dstStride,
		      (pbox->x1 + dstXoff) * dstBpp,
		      dstBpp,
    
		      width * dstBpp,
		      height,
    
		      pPriv->and, pPriv->xor,
		      pPriv->bgand, pPriv->bgxor);
	    xfree (tmp);
	}
	pbox++;
    }
}

void
fbCopyRegion (DrawablePtr   pSrcDrawable,
	      DrawablePtr   pDstDrawable,
	      GCPtr	    pGC,
	      RegionPtr	    pDstRegion,
	      int	    dx,
	      int	    dy,
	      fbCopyProc    copyProc,
	      Pixel	    bitPlane,
	      void	    *closure)
{
    int		careful;
    Bool	reverse;
    Bool	upsidedown;
    BoxPtr	pbox;
    int		nbox;
    BoxPtr	pboxNew1, pboxNew2, pboxBase, pboxNext, pboxTmp;
    
    pbox = REGION_RECTS(pDstRegion);
    nbox = REGION_NUM_RECTS(pDstRegion);
    
    /* XXX we have to err on the side of safety when both are windows,
     * because we don't know if IncludeInferiors is being used.
     */
    careful = ((pSrcDrawable == pDstDrawable) ||
	       ((pSrcDrawable->type == DRAWABLE_WINDOW) &&
		(pDstDrawable->type == DRAWABLE_WINDOW)));

    pboxNew1 = NULL;
    pboxNew2 = NULL;
    if (careful && dy < 0)
    {
	upsidedown = TRUE;

	if (nbox > 1)
	{
	    /* keep ordering in each band, reverse order of bands */
	    pboxNew1 = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
	    if(!pboxNew1)
		return;
	    pboxBase = pboxNext = pbox+nbox-1;
	    while (pboxBase >= pbox)
	    {
		while ((pboxNext >= pbox) &&
		       (pboxBase->y1 == pboxNext->y1))
		    pboxNext--;
		pboxTmp = pboxNext+1;
		while (pboxTmp <= pboxBase)
		{
		    *pboxNew1++ = *pboxTmp++;
		}
		pboxBase = pboxNext;
	    }
	    pboxNew1 -= nbox;
	    pbox = pboxNew1;
	}
    }
    else
    {
	/* walk source top to bottom */
	upsidedown = FALSE;
    }

    if (careful && dx < 0)
    {
	/* walk source right to left */
	if (dy <= 0)
	    reverse = TRUE;
	else
	    reverse = FALSE;

	if (nbox > 1)
	{
	    /* reverse order of rects in each band */
	    pboxNew2 = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
	    if(!pboxNew2)
	    {
		if (pboxNew1)
		    DEALLOCATE_LOCAL(pboxNew1);
		return;
	    }
	    pboxBase = pboxNext = pbox;
	    while (pboxBase < pbox+nbox)
	    {
		while ((pboxNext < pbox+nbox) &&
		       (pboxNext->y1 == pboxBase->y1))
		    pboxNext++;
		pboxTmp = pboxNext;
		while (pboxTmp != pboxBase)
		{
		    *pboxNew2++ = *--pboxTmp;
		}
		pboxBase = pboxNext;
	    }
	    pboxNew2 -= nbox;
	    pbox = pboxNew2;
	}
    }
    else
    {
	/* walk source left to right */
	reverse = FALSE;
    }

    (*copyProc) (pSrcDrawable,
		 pDstDrawable,
		 pGC,
		 pbox,
		 nbox,
		 dx, dy,
		 reverse, upsidedown, bitPlane, closure);
    
    if (pboxNew1)
	DEALLOCATE_LOCAL (pboxNew1);
    if (pboxNew2)
	DEALLOCATE_LOCAL (pboxNew2);
}

RegionPtr
fbDoCopy (DrawablePtr	pSrcDrawable,
	  DrawablePtr	pDstDrawable,
	  GCPtr		pGC,
	  int		xIn, 
	  int		yIn,
	  int		widthSrc, 
	  int		heightSrc,
	  int		xOut, 
	  int		yOut,
	  fbCopyProc	copyProc,
	  Pixel		bitPlane,
	  void		*closure)
{
    RegionPtr	prgnSrcClip = NULL; /* may be a new region, or just a copy */
    Bool	freeSrcClip = FALSE;
    RegionPtr	prgnExposed = NULL;
    RegionRec	rgnDst;
    int		dx;
    int		dy;
    int		numRects;
    int         box_x1;
    int         box_y1;
    int         box_x2;
    int         box_y2;
    Bool	fastSrc = FALSE;    /* for fast clipping with pixmap source */
    Bool	fastDst = FALSE;    /* for fast clipping with one rect dest */
    Bool	fastExpose = FALSE; /* for fast exposures with pixmap source */

    /* Short cut for unmapped windows */

    if (pDstDrawable->type == DRAWABLE_WINDOW && 
	!((WindowPtr)pDstDrawable)->realized)
    {
	return NULL;
    }

    if ((pSrcDrawable != pDstDrawable) &&
	pSrcDrawable->pScreen->SourceValidate)
    {
	(*pSrcDrawable->pScreen->SourceValidate) (pSrcDrawable, xIn, yIn, widthSrc, heightSrc);
    }

    /* Compute source clip region */
    if (pSrcDrawable->type == DRAWABLE_PIXMAP)
    {
	if ((pSrcDrawable == pDstDrawable) && (pGC->clientClipType == CT_NONE))
	    prgnSrcClip = fbGetCompositeClip(pGC);
	else
	    fastSrc = TRUE;
    }
    else
    {
	if (pGC->subWindowMode == IncludeInferiors)
	{
	    /*
	     * XFree86 DDX empties the border clip when the
	     * VT is inactive, make sure the region isn't empty
	     */
	    if (!((WindowPtr) pSrcDrawable)->parent &&
		REGION_NOTEMPTY (pSrcDrawable->pScreen,
				 &((WindowPtr) pSrcDrawable)->borderClip))
	    {
		/*
		 * special case bitblt from root window in
		 * IncludeInferiors mode; just like from a pixmap
		 */
		fastSrc = TRUE;
	    }
	    else if ((pSrcDrawable == pDstDrawable) &&
		     (pGC->clientClipType == CT_NONE))
	    {
		prgnSrcClip = fbGetCompositeClip(pGC);
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

    xIn += pSrcDrawable->x;
    yIn += pSrcDrawable->y;
    
    xOut += pDstDrawable->x;
    yOut += pDstDrawable->y;

    box_x1 = xIn;
    box_y1 = yIn;
    box_x2 = xIn + widthSrc;
    box_y2 = yIn + heightSrc;

    dx = xIn - xOut;
    dy = yIn - yOut;

    /* Don't create a source region if we are doing a fast clip */
    if (fastSrc)
    {
	RegionPtr cclip;
    
	fastExpose = TRUE;
	/*
	 * clip the source; if regions extend beyond the source size,
 	 * make sure exposure events get sent
	 */
	if (box_x1 < pSrcDrawable->x)
	{
	    box_x1 = pSrcDrawable->x;
	    fastExpose = FALSE;
	}
	if (box_y1 < pSrcDrawable->y)
	{
	    box_y1 = pSrcDrawable->y;
	    fastExpose = FALSE;
	}
	if (box_x2 > pSrcDrawable->x + (int) pSrcDrawable->width)
	{
	    box_x2 = pSrcDrawable->x + (int) pSrcDrawable->width;
	    fastExpose = FALSE;
	}
	if (box_y2 > pSrcDrawable->y + (int) pSrcDrawable->height)
	{
	    box_y2 = pSrcDrawable->y + (int) pSrcDrawable->height;
	    fastExpose = FALSE;
	}
	
	/* Translate and clip the dst to the destination composite clip */
        box_x1 -= dx;
        box_x2 -= dx;
        box_y1 -= dy;
        box_y2 -= dy;

	/* If the destination composite clip is one rectangle we can
	   do the clip directly.  Otherwise we have to create a full
	   blown region and call intersect */

	cclip = fbGetCompositeClip(pGC);
        if (REGION_NUM_RECTS(cclip) == 1)
        {
	    BoxPtr pBox = REGION_RECTS(cclip);

	    if (box_x1 < pBox->x1) box_x1 = pBox->x1;
	    if (box_x2 > pBox->x2) box_x2 = pBox->x2;
	    if (box_y1 < pBox->y1) box_y1 = pBox->y1;
	    if (box_y2 > pBox->y2) box_y2 = pBox->y2;
	    fastDst = TRUE;
	}
    }
    
    /* Check to see if the region is empty */
    if (box_x1 >= box_x2 || box_y1 >= box_y2)
    {
	REGION_NULL(pGC->pScreen, &rgnDst);
    }
    else
    {
        BoxRec	box;
	box.x1 = box_x1;
	box.y1 = box_y1;
	box.x2 = box_x2;
	box.y2 = box_y2;
	REGION_INIT(pGC->pScreen, &rgnDst, &box, 1);
    }
    
    /* Clip against complex source if needed */
    if (!fastSrc)
    {
	REGION_INTERSECT(pGC->pScreen, &rgnDst, &rgnDst, prgnSrcClip);
	REGION_TRANSLATE(pGC->pScreen, &rgnDst, -dx, -dy);
    }

    /* Clip against complex dest if needed */
    if (!fastDst)
    {
	REGION_INTERSECT(pGC->pScreen, &rgnDst, &rgnDst,
			 fbGetCompositeClip(pGC));
    }

    /* Do bit blitting */
    numRects = REGION_NUM_RECTS(&rgnDst);
    if (numRects && widthSrc && heightSrc)
	fbCopyRegion (pSrcDrawable, pDstDrawable, pGC,
		      &rgnDst, dx, dy, copyProc, bitPlane, closure);

    /* Pixmap sources generate a NoExposed (we return NULL to do this) */
    if (!fastExpose && fbGetExpose(pGC))
	prgnExposed = miHandleExposures(pSrcDrawable, pDstDrawable, pGC,
					xIn - pSrcDrawable->x,
					yIn - pSrcDrawable->y,
					widthSrc, heightSrc,
					xOut - pDstDrawable->x,
					yOut - pDstDrawable->y,
					(unsigned long) bitPlane);
    REGION_UNINIT(pGC->pScreen, &rgnDst);
    if (freeSrcClip)
	REGION_DESTROY(pGC->pScreen, prgnSrcClip);
    fbValidateDrawable (pDstDrawable);
    return prgnExposed;
}

RegionPtr
fbCopyArea (DrawablePtr	pSrcDrawable,
	    DrawablePtr	pDstDrawable,
	    GCPtr	pGC,
	    int		xIn, 
	    int		yIn,
	    int		widthSrc, 
	    int		heightSrc,
	    int		xOut, 
	    int		yOut)
{
    fbCopyProc	copy;

#ifdef FB_24_32BIT
    if (pSrcDrawable->bitsPerPixel != pDstDrawable->bitsPerPixel)
	copy = fb24_32CopyMtoN;
    else
#endif
	copy = fbCopyNtoN;
    return fbDoCopy (pSrcDrawable, pDstDrawable, pGC, xIn, yIn,
		     widthSrc, heightSrc, xOut, yOut, copy, 0, 0);
}

RegionPtr
fbCopyPlane (DrawablePtr    pSrcDrawable,
	     DrawablePtr    pDstDrawable,
	     GCPtr	    pGC,
	     int	    xIn, 
	     int	    yIn,
	     int	    widthSrc, 
	     int	    heightSrc,
	     int	    xOut, 
	     int	    yOut,
	     unsigned long  bitplane)
{
    if (pSrcDrawable->bitsPerPixel > 1)
	return fbDoCopy (pSrcDrawable, pDstDrawable, pGC,
			 xIn, yIn, widthSrc, heightSrc,
			 xOut, yOut, fbCopyNto1, (Pixel) bitplane, 0);
    else if (bitplane & 1)
	return fbDoCopy (pSrcDrawable, pDstDrawable, pGC, xIn, yIn,
			 widthSrc, heightSrc, xOut, yOut, fbCopy1toN,
			 (Pixel) bitplane, 0);
    else
	return miHandleExposures(pSrcDrawable, pDstDrawable, pGC,
				 xIn, yIn,
				 widthSrc,
				 heightSrc,
				 xOut, yOut, bitplane);
}
