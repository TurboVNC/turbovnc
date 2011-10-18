/***********************************************************

Copyright (c) 1987  X Consortium

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
/* $XConsortium: mfbpushpxl.c,v 5.6 94/04/17 20:28:31 dpw Exp $ */

#include "X.h"
#include "gcstruct.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "miscstruct.h"
#include "maskbits.h"
#include "regionstr.h"
#include "mfb.h"

/*  mfbSolidPP is courtesy of xhacks@csri.toronto.edu

    For fillStyle==FillSolid, a monochrome PushPixels can be reduced to
    a ROP in the following way:  (Note that the ROP is the same as the
    result of ROP(src=0x3,dst=0x5))

			src=0011 0000 0011
			dst=0101 0101 0101
			rop      fg=0 fg=1
	GXclear         0x0 0000 0100 0100 0
	GXand           0x1 0001 0100 0101  s&d
	GXandReverse    0x2 0010 0100 0110 s&~d
	GXcopy          0x3 0011 0100 0111 s
	GXandInverted   0x4 0100 0101 0100 ~s&d
	GXnoop          0x5 0101 0101 0101 d
	GXxor           0x6 0110 0101 0110 s^d
	GXor            0x7 0111 0101 0111 s|d
	GXnor           0x8 1000 0110 0100 ~s&~d
	GXequiv         0x9 1001 0110 0101 ~s^d
	GXinvert        0xa 1010 0110 0110 ~d
	GXorReverse     0xb 1011 0110 0111 s|~d
	GXcopyInverted  0xc 1100 0111 0100 ~s
	GXorInverted    0xd 1101 0111 0101 ~s|d
	GXnand          0xe 1110 0111 0110 ~s|~d
	GXset           0xf 1111 0111 0111 1

For src=0: newRop = 0x4|(rop>>2)
For src=1: newRop = 0x4|(rop&3)
*/

/* mfbSolidPP -- squeegees the forground color of pGC through pBitMap
 * into pDrawable.  pBitMap is a stencil (dx by dy of it is used, it may
 * be bigger) which is placed on the drawable at xOrg, yOrg.  Where a 1 bit
 * is set in the bitmap, the fill style is put onto the drawable using
 * the GC's logical function. The drawable is not changed where the bitmap
 * has a zero bit or outside the area covered by the stencil.
 */
void
mfbSolidPP(pGC, pBitMap, pDrawable, dx, dy, xOrg, yOrg)
    GCPtr	pGC;
    PixmapPtr	pBitMap;
    DrawablePtr pDrawable;
    int		dx, dy, xOrg, yOrg;
{
    unsigned char alu;
    RegionRec rgnDst;
    DDXPointPtr pptSrc;
    BoxRec srcBox;
    register DDXPointPtr ppt;
    register BoxPtr pbox;
    int i;

    if (!pGC->planemask & 1) return;

    /* compute the reduced rop function */
    alu = pGC->alu;
    if (!(pGC->fgPixel&1)) alu >>= 2;
    alu = (alu & 0x3) | 0x4;
    if (alu == GXnoop) return;

    srcBox.x1 = xOrg;
    srcBox.y1 = yOrg;
    srcBox.x2 = xOrg + dx;
    srcBox.y2 = yOrg + dy;
    REGION_INIT(pGC->pScreen, &rgnDst, &srcBox, 1);

    /* clip the shape of the dst to the destination composite clip */
    REGION_INTERSECT(pGC->pScreen, &rgnDst, &rgnDst,
	((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip);

    if (!REGION_NIL(&rgnDst))
    {
	i = REGION_NUM_RECTS(&rgnDst);
	pptSrc = (DDXPointPtr)ALLOCATE_LOCAL(i * sizeof(DDXPointRec));
        if(pptSrc)
        {
	    for (pbox = REGION_RECTS(&rgnDst), ppt = pptSrc;
		 --i >= 0;
		 pbox++, ppt++)
	    {
		ppt->x = pbox->x1 - xOrg;
		ppt->y = pbox->y1 - yOrg;
	    }
	    mfbDoBitblt((DrawablePtr)pBitMap, pDrawable, alu, &rgnDst, pptSrc);
	    DEALLOCATE_LOCAL(pptSrc);
	}
    }
    REGION_UNINIT(pGC->pScreen, &rgnDst);
}

#define NPT 128

/* mfbPushPixels -- squeegees the forground color of pGC through pBitMap
 * into pDrawable.  pBitMap is a stencil (dx by dy of it is used, it may
 * be bigger) which is placed on the drawable at xOrg, yOrg.  Where a 1 bit
 * is set in the bitmap, the fill style is put onto the drawable using
 * the GC's logical function. The drawable is not changed where the bitmap
 * has a zero bit or outside the area covered by the stencil.
 */
void
mfbPushPixels(pGC, pBitMap, pDrawable, dx, dy, xOrg, yOrg)
    GCPtr	pGC;
    PixmapPtr	pBitMap;
    DrawablePtr pDrawable;
    int		dx, dy, xOrg, yOrg;
{
    int		h, dxDivPPW, ibEnd;
    PixelType *pwLineStart;
    register PixelType	*pw, *pwEnd;
    register PixelType mask;
    register int ib;
    register PixelType w;
    register int ipt;		/* index into above arrays */
    Bool 	fInBox;
    DDXPointRec	pt[NPT];
    int		width[NPT];

    /* Now scan convert the pixmap and use the result to call fillspans in
     * in the drawable with the original GC */
    ipt = 0;
    dxDivPPW = dx/PPW;
    for(h = 0; h < dy; h++)
    {

	pw = (PixelType *)
	     (((char *)(pBitMap->devPrivate.ptr))+(h * pBitMap->devKind));
	pwLineStart = pw;
	/* Process all words which are fully in the pixmap */
	
	fInBox = FALSE;
	pwEnd = pwLineStart + dxDivPPW;
	while(pw  < pwEnd)
	{
	    w = *pw;
	    mask = endtab[1];
	    for(ib = 0; ib < PPW; ib++)
	    {
		if(w & mask)
		{
		    if(!fInBox)
		    {
			pt[ipt].x = ((pw - pwLineStart) << PWSH) + ib + xOrg;
			pt[ipt].y = h + yOrg;
			/* start new box */
			fInBox = TRUE;
		    }
		}
		else
		{
		    if(fInBox)
		    {
			width[ipt] = ((pw - pwLineStart) << PWSH) + 
				     ib + xOrg - pt[ipt].x;
			if (++ipt >= NPT)
			{
			    (*pGC->ops->FillSpans)(pDrawable, pGC, NPT, pt,
			                      width, TRUE);
			    ipt = 0;
			}
			/* end box */
			fInBox = FALSE;
		    }
		}
		mask = SCRRIGHT(mask, 1);
	    }
	    pw++;
	}
	ibEnd = dx & PIM;
	if(ibEnd)
	{
	    /* Process final partial word on line */
	    w = *pw;
	    mask = endtab[1];
	    for(ib = 0; ib < ibEnd; ib++)
	    {
		if(w & mask)
		{
		    if(!fInBox)
		    {
			/* start new box */
			pt[ipt].x = ((pw - pwLineStart) << PWSH) + ib + xOrg;
			pt[ipt].y = h + yOrg;
			fInBox = TRUE;
		    }
		}
		else
		{
		    if(fInBox)
		    {
			/* end box */
			width[ipt] = ((pw - pwLineStart) << PWSH) + 
				     ib + xOrg - pt[ipt].x;
			if (++ipt >= NPT)
			{
			    (*pGC->ops->FillSpans)(pDrawable, pGC, NPT, pt,
			                      width, TRUE);
			    ipt = 0;
			}
			fInBox = FALSE;
		    }
		}
		mask = SCRRIGHT(mask, 1);
	    }
	}
	/* If scanline ended with last bit set, end the box */
	if(fInBox)
	{
	    width[ipt] = dx + xOrg - pt[ipt].x;
	    if (++ipt >= NPT)
	    {
		(*pGC->ops->FillSpans)(pDrawable, pGC, NPT, pt, width, TRUE);
		ipt = 0;
	    }
	}
    }
    /* Flush any remaining spans */
    if (ipt)
    {
	(*pGC->ops->FillSpans)(pDrawable, pGC, ipt, pt, width, TRUE);
    }
}
