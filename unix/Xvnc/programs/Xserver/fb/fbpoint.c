/*
 * Id: fbpoint.c,v 1.1 1999/11/02 03:54:45 keithp Exp $
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
/* $XFree86: xc/programs/Xserver/fb/fbpoint.c,v 1.8 2001/05/29 04:54:09 keithp Exp $ */

#include "fb.h"

typedef void	(*FbDots)  (FbBits	*dst,
			    FbStride    dstStride,
			    int		dstBpp,
			    BoxPtr	pBox,
			    xPoint	*pts,
			    int		npt,
			    int		xoff,
			    int		yoff,
			    FbBits	and,
			    FbBits	xor);

void
fbDots (FbBits	    *dstOrig,
	FbStride    dstStride,
	int	    dstBpp,
	BoxPtr	    pBox,
	xPoint	    *pts,
	int	    npt,
	int	    xoff,
	int	    yoff,
	FbBits	    andOrig,
	FbBits	    xorOrig)
{
    FbStip	*dst = (FbStip *) dstOrig;
    int		x1, y1, x2, y2;
    int		x, y;
    FbStip	*d;
    FbStip	and = andOrig;
    FbStip	xor = xorOrig;

    dstStride = FbBitsStrideToStipStride (dstStride);
    x1 = pBox->x1;
    y1 = pBox->y1;
    x2 = pBox->x2;
    y2 = pBox->y2;
    while (npt--)
    {
	x = pts->x + xoff;
	y = pts->y + yoff;
	pts++;
	if (x1 <= x && x < x2 && y1 <= y && y < y2)
	{
	    x *= dstBpp;
	    d = dst + (y * dstStride) + (x >> FB_STIP_SHIFT);
	    x &= FB_STIP_MASK;
#ifdef FB_24BIT
	    if (dstBpp == 24)
	    {
		FbStip	leftMask, rightMask;
		int	n, rot;
		FbStip	andT, xorT;
		
		rot = FbFirst24Rot (x);
		andT = FbRot24Stip(and,rot);
		xorT = FbRot24Stip(xor,rot);
		FbMaskStip (x, 24, leftMask, n, rightMask);
		if (leftMask)
		{
		    *d = FbDoMaskRRop (*d, andT, xorT, leftMask);
		    andT = FbNext24Stip(andT);
		    xorT = FbNext24Stip(xorT);
		    d++;
		}
		if (rightMask)
		    *d = FbDoMaskRRop(*d, andT, xorT, rightMask);
	    }
	    else
#endif
	    {
		FbStip	mask;
		mask = FbStipMask(x, dstBpp);
		*d = FbDoMaskRRop (*d, and, xor, mask);
	    }
	}
    }
}

void
fbPolyPoint (DrawablePtr    pDrawable,
	     GCPtr	    pGC,
	     int	    mode,
	     int	    nptInit,
	     xPoint	    *pptInit)
{
    FbGCPrivPtr pPriv = fbGetGCPrivate (pGC);
    RegionPtr	pClip = fbGetCompositeClip(pGC);
    FbBits	*dst;
    FbStride	dstStride;
    int		dstBpp;
    int		dstXoff, dstYoff;
    FbDots	dots;
    FbBits	and, xor;
    xPoint	*ppt;
    int		npt;
    BoxPtr	pBox;
    int		nBox;
    
    /* make pointlist origin relative */
    ppt = pptInit;
    npt = nptInit;
    if (mode == CoordModePrevious)
    {
	npt--;
	while(npt--)
	{
	    ppt++;
	    ppt->x += (ppt-1)->x;
	    ppt->y += (ppt-1)->y;
	}
    }
    fbGetDrawable (pDrawable, dst, dstStride, dstBpp, dstXoff, dstYoff);
    and = pPriv->and;
    xor = pPriv->xor;
    dots = fbDots;
#ifndef FBNOPIXADDR
    switch (dstBpp) {
    case 8:	dots = fbDots8; break;
    case 16:    dots = fbDots16; break;
#ifdef FB_24BIT
    case 24:    dots = fbDots24; break;
#endif
    case 32:    dots = fbDots32; break;
    }
#endif
    for (nBox = REGION_NUM_RECTS (pClip), pBox = REGION_RECTS (pClip);
	 nBox--; pBox++)
	(*dots) (dst, dstStride, dstBpp, pBox, pptInit, nptInit, 
		 pDrawable->x + dstXoff, pDrawable->y + dstYoff, and, xor);
}
