/* $XFree86: xc/programs/Xserver/cfb/cfbfillsp.c,v 3.7tsi Exp $ */
/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or The Open Group
not be used in advertising or publicity pertaining to 
distribution  of  the software  without specific prior 
written permission. Sun and The Open Group make no 
representations about the suitability of this software for 
any purpose. It is provided "as is" without any express or 
implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

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

/* $Xorg: cfbfillsp.c,v 1.4 2001/02/09 02:04:37 xorgcvs Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xmd.h>
#include "servermd.h"
#include "gcstruct.h"
#include "window.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "windowstr.h"

#include "cfb.h"
#include "cfbmskbits.h"

#include "mergerop.h"

#if PSZ == 8
#include "cfb8bit.h"
#endif

#define MFB_CONSTS_ONLY
#include "maskbits.h"

#include "mi.h"
#include "mispans.h"

/* scanline filling for color frame buffer
   written by drewry, oct 1986 modified by smarks
   changes for compatibility with Little-endian systems Jul 1987; MIT:yba.

   these routines all clip.  they assume that anything that has called
them has already translated the points (i.e. pGC->miTranslate is
non-zero, which is howit gets set in cfbCreateGC().)

   the number of new scnalines created by clipping ==
MaxRectsPerBand * nSpans.

    FillSolid is overloaded to be used for OpaqueStipple as well,
if fgPixel == bgPixel.  
Note that for solids, PrivGC.rop == PrivGC.ropOpStip


    FillTiled is overloaded to be used for OpaqueStipple, if
fgPixel != bgPixel.  based on the fill style, it uses
{RotatedTile, gc.alu} or {RotatedStipple, PrivGC.ropOpStip}
*/

#ifdef	notdef
#include	<stdio.h>
static
dumpspans(n, ppt, pwidth)
    int	n;
    DDXPointPtr ppt;
    int *pwidth;
{
    fprintf(stderr,"%d spans\n", n);
    while (n--) {
	fprintf(stderr, "[%d,%d] %d\n", ppt->x, ppt->y, *pwidth);
	ppt++;
	pwidth++;
    }
    fprintf(stderr, "\n");
}
#endif

/* Fill spans with tiles that aren't 32 bits wide */
void
cfbUnnaturalTileFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
DrawablePtr pDrawable;
GC		*pGC;
int		nInit;		/* number of spans to fill */
DDXPointPtr pptInit;		/* pointer to list of start points */
int *pwidthInit;		/* pointer to list of n widths */
int fSorted;
{
    int n;			/* number of spans to fill */
    register DDXPointPtr ppt;	/* pointer to list of start points */
    register int *pwidth;	/* pointer to list of n widths */
    void    (*fill)(DrawablePtr, int, DDXPointPtr, int *, PixmapPtr, int, int, int, unsigned long);
    int	xrot, yrot;

    if (!(pGC->planemask))
	return;

#if PSZ == 24
    if (pGC->tile.pixmap->drawable.width & 3)
#else
    if (pGC->tile.pixmap->drawable.width & PIM)
#endif
    {
    	fill = cfbFillSpanTileOddGeneral;
    	if ((pGC->planemask & PMSK) == PMSK)
    	{
	    if (pGC->alu == GXcopy)
	    	fill = cfbFillSpanTileOddCopy;
    	}
    }
    else
    {
	fill = cfbFillSpanTile32sGeneral;
    	if ((pGC->planemask & PMSK) == PMSK)
    	{
	    if (pGC->alu == GXcopy)
		fill = cfbFillSpanTile32sCopy;
	}
    }
    n = nInit * miFindMaxBand( cfbGetCompositeClip(pGC) );
    if ( n == 0 )
	return;
    pwidth = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    ppt = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!ppt || !pwidth)
    {
	if (ppt) DEALLOCATE_LOCAL(ppt);
	if (pwidth) DEALLOCATE_LOCAL(pwidth);
	return;
    }
    n = miClipSpans( cfbGetCompositeClip(pGC),
		     pptInit, pwidthInit, nInit, 
		     ppt, pwidth, fSorted);

    xrot = pDrawable->x + pGC->patOrg.x;
    yrot = pDrawable->y + pGC->patOrg.y;

    (*fill) (pDrawable, n, ppt, pwidth, pGC->tile.pixmap, xrot, yrot, pGC->alu, pGC->planemask);

    DEALLOCATE_LOCAL(ppt);
    DEALLOCATE_LOCAL(pwidth);
}

#if PSZ == 8

void
cfbUnnaturalStippleFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
DrawablePtr pDrawable;
GC		*pGC;
int		nInit;		/* number of spans to fill */
DDXPointPtr pptInit;		/* pointer to list of start points */
int *pwidthInit;		/* pointer to list of n widths */
int fSorted;
{
				/* next three parameters are post-clip */
    int		    n;		/* number of spans to fill */
    DDXPointPtr	    ppt;	/* pointer to list of start points */
    int		    *pwidth;	/* pointer to list of n widths */
    int		    *pwidthFree;/* copies of the pointers to free */
    DDXPointPtr	    pptFree;
    CfbBits   *pdstBase;	/* pointer to start of bitmap */
    int		    nlwDst;	/* width in longwords of bitmap */
    register CfbBits    *pdst;	/* pointer to current word in bitmap */
    PixmapPtr	    pStipple;	/* pointer to stipple we want to fill with */
    int		    nlw;
    int		    x, y, w, xrem, xSrc, ySrc;
    int		    stwidth, stippleWidth;
    int		    stippleHeight;
    register CfbBits  bits, inputBits;
    register int    partBitsLeft;
    int		    nextPartBits;
    int		    bitsLeft, bitsWhole;
    CfbBits   *srcTemp, *srcStart;
    CfbBits   *psrcBase;
    CfbBits   startmask, endmask;

    if (pGC->fillStyle == FillStippled)
	cfb8CheckStipple (pGC->alu, pGC->fgPixel, pGC->planemask);
    else
	cfb8CheckOpaqueStipple (pGC->alu, pGC->fgPixel, pGC->bgPixel, pGC->planemask);

    if (cfb8StippleRRop == GXnoop)
	return;

    n = nInit * miFindMaxBand( cfbGetCompositeClip(pGC) );
    if ( n == 0 )
	return;
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree)
    {
	if (pptFree) DEALLOCATE_LOCAL(pptFree);
	if (pwidthFree) DEALLOCATE_LOCAL(pwidthFree);
	return;
    }

    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans( cfbGetCompositeClip(pGC),
		     pptInit, pwidthInit, nInit, 
		     ppt, pwidth, fSorted);

    /*
     *  OK,  so what's going on here?  We have two Drawables:
     *
     *  The Stipple:
     *		Depth = 1
     *		Width = stippleWidth
     *		Words per scanline = stwidth
     *		Pointer to pixels = pStipple->devPrivate.ptr
     */

    pStipple = pGC->stipple;

    stwidth = pStipple->devKind >> PWSH;
    stippleWidth = pStipple->drawable.width;
    stippleHeight = pStipple->drawable.height;
    psrcBase = (CfbBits *) pStipple->devPrivate.ptr;

    /*
     *	The Target:
     *		Depth = PSZ
     *		Width = determined from *pwidth
     *		Words per scanline = nlwDst
     *		Pointer to pixels = addrlBase
     */

    cfbGetLongWidthAndPointer (pDrawable, nlwDst, pdstBase)

    /* this replaces rotating the stipple. Instead we just adjust the offset
     * at which we start grabbing bits from the stipple.
     * Ensure that ppt->x - xSrc >= 0 and ppt->y - ySrc >= 0,
     * so that iline and xrem always stay within the stipple bounds.
     */

    modulus (pGC->patOrg.x, stippleWidth, xSrc);
    xSrc += pDrawable->x - stippleWidth;
    modulus (pGC->patOrg.y, stippleHeight, ySrc);
    ySrc += pDrawable->y - stippleHeight;

    bitsWhole = stippleWidth;

    while (n--)
    {
	x = ppt->x;
	y = ppt->y;
	ppt++;
	w = *pwidth++;
	pdst = pdstBase + y * nlwDst + (x >> PWSH);
	y = (y - ySrc) % stippleHeight;
	srcStart = psrcBase + y * stwidth;
	xrem = ((x & ~(PGSZB-1)) - xSrc) % stippleWidth;
	srcTemp = srcStart + (xrem >> MFB_PWSH);
	bitsLeft = stippleWidth - (xrem & ~MFB_PIM);
	xrem &= MFB_PIM;
	NextUnnaturalStippleWord
	if (partBitsLeft < xrem)
	    FatalError ("cfbUnnaturalStippleFS bad partBitsLeft %d xrem %d",
			partBitsLeft, xrem);
	NextSomeBits (inputBits, xrem);
	partBitsLeft -= xrem;
	if (((x & PIM) + w) <= PPW)
	{
	    maskpartialbits (x, w, startmask)
	    NextUnnaturalStippleBits
	    *pdst = MaskRRopPixels(*pdst,bits,startmask);
	}
	else
	{
	    maskbits (x, w, startmask, endmask, nlw);
	    nextPartBits = (x & (PGSZB-1)) + w;
	    if (nextPartBits < partBitsLeft)
	    {
		if (startmask)
		{
		    MaskRRopBitGroup(pdst,GetBitGroup(inputBits),startmask)
		    pdst++;
		    NextBitGroup (inputBits);
		}
		while (nlw--)
		{
		    RRopBitGroup (pdst, GetBitGroup (inputBits));
		    pdst++;
		    NextBitGroup (inputBits);
		}
		if (endmask)
		{
		    MaskRRopBitGroup(pdst,GetBitGroup(inputBits),endmask)
		}
	    }
	    else if (bitsLeft != bitsWhole && nextPartBits < partBitsLeft + bitsLeft)
	    {
	    	NextUnnaturalStippleBitsFast
	    	if (startmask)
	    	{
		    *pdst = MaskRRopPixels(*pdst,bits,startmask);
		    pdst++;
	    	    NextUnnaturalStippleBitsFast
	    	}
	    	while (nlw--)
	    	{
		    *pdst = RRopPixels(*pdst,bits);
		    pdst++;
	    	    NextUnnaturalStippleBitsFast
	    	}
	    	if (endmask)
		    *pdst = MaskRRopPixels (*pdst,bits,endmask);
	    }
	    else
	    {
	    	NextUnnaturalStippleBits
	    	if (startmask)
	    	{
		    *pdst = MaskRRopPixels(*pdst,bits,startmask);
		    pdst++;
	    	    NextUnnaturalStippleBits
	    	}
	    	while (nlw--)
	    	{
		    *pdst = RRopPixels(*pdst,bits);
		    pdst++;
	    	    NextUnnaturalStippleBits
	    	}
	    	if (endmask)
		    *pdst = MaskRRopPixels(*pdst,bits,endmask);
	    }
	}
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}

#else /* PSZ != 8 */

/* Fill spans with stipples that aren't 32 bits wide */
void
cfbUnnaturalStippleFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
DrawablePtr pDrawable;
GC		*pGC;
int		nInit;		/* number of spans to fill */
DDXPointPtr pptInit;		/* pointer to list of start points */
int *pwidthInit;		/* pointer to list of n widths */
int fSorted;
{
				/* next three parameters are post-clip */
    int			    n;		/* number of spans to fill */
    register DDXPointPtr    ppt;	/* pointer to list of start points */
    register int	    *pwidth;	/* pointer to list of n widths */
    int			    iline;	/* first line of tile to use */
    CfbBits	    *addrlBase;	/* pointer to start of bitmap */
    int			    nlwidth;	/* width in longwords of bitmap */
    register CfbBits  *pdst;	/* pointer to current word in bitmap */
    PixmapPtr		    pStipple;	/* pointer to stipple we want to fill with */
    register int	    w;
    int			    width,  x, xrem, xSrc, ySrc;
    CfbBits	    tmpSrc, tmpDst1, tmpDst2;
    int			    stwidth, stippleWidth;
    CfbBits	    *psrcS;
    int			    rop, stiprop = 0;
    int			    stippleHeight;
    int			    *pwidthFree;    /* copies of the pointers to free */
    DDXPointPtr		    pptFree;
    CfbBits	    fgfill, bgfill;

    if (!(pGC->planemask))
	return;

    n = nInit * miFindMaxBand( cfbGetCompositeClip(pGC) );
    if ( n == 0 )
	return;
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree)
    {
	if (pptFree) DEALLOCATE_LOCAL(pptFree);
	if (pwidthFree) DEALLOCATE_LOCAL(pwidthFree);
	return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans( cfbGetCompositeClip(pGC),
		     pptInit, pwidthInit, nInit, 
		     ppt, pwidth, fSorted);
    rop = pGC->alu;
    if (pGC->fillStyle == FillStippled) {
	switch (rop) {
	    case GXand:
	    case GXcopy:
	    case GXnoop:
	    case GXor:
		stiprop = rop;
		break;
	    default:
		stiprop = rop;
		rop = GXcopy;
	}
    }
    fgfill = PFILL(pGC->fgPixel);
    bgfill = PFILL(pGC->bgPixel);

    /*
     *  OK,  so what's going on here?  We have two Drawables:
     *
     *  The Stipple:
     *		Depth = 1
     *		Width = stippleWidth
     *		Words per scanline = stwidth
     *		Pointer to pixels = pStipple->devPrivate.ptr
     */
    pStipple = pGC->stipple;

    stwidth = pStipple->devKind / PGSZB;
    stippleWidth = pStipple->drawable.width;
    stippleHeight = pStipple->drawable.height;

    /*
     *	The Target:
     *		Depth = PSZ
     *		Width = determined from *pwidth
     *		Words per scanline = nlwidth
     *		Pointer to pixels = addrlBase
     */

    cfbGetLongWidthAndPointer (pDrawable, nlwidth, addrlBase)

    /* this replaces rotating the stipple. Instead we just adjust the offset
     * at which we start grabbing bits from the stipple.
     * Ensure that ppt->x - xSrc >= 0 and ppt->y - ySrc >= 0,
     * so that iline and xrem always stay within the stipple bounds.
     */
    modulus (pGC->patOrg.x, stippleWidth, xSrc);
    xSrc += pDrawable->x - stippleWidth;
    modulus (pGC->patOrg.y, stippleHeight, ySrc);
    ySrc += pDrawable->y - stippleHeight;

    while (n--)
    {
	iline = (ppt->y - ySrc) % stippleHeight;
	x = ppt->x;
	pdst = addrlBase + (ppt->y * nlwidth);
        psrcS = (CfbBits *) pStipple->devPrivate.ptr + (iline * stwidth);

	if (*pwidth)
	{
	    width = *pwidth;
	    while(width > 0)
	    {
		int xtemp;
#if PSZ != 32 || PPW != 1
		int tmpx;
#endif
		register CfbBits *ptemp;
		register CfbBits *pdsttmp;
		/*
		 *  Do a stripe through the stipple & destination w pixels
		 *  wide.  w is not more than:
		 *	-	the width of the destination
		 *	-	the width of the stipple
		 *	-	the distance between x and the next word 
		 *		boundary in the destination
		 *	-	the distance between x and the next word
		 *		boundary in the stipple
		 */

		/* width of dest/stipple */
                xrem = (x - xSrc) % stippleWidth;
#if PSZ == 24
		w = 1;
#else
	        w = min((stippleWidth - xrem), width);
		/* dist to word bound in dest */
		w = min(w, PPW - (x & PIM));
		/* dist to word bound in stip */
		w = min(w, MFB_PPW - (x & MFB_PIM));
#endif

	        xtemp = (xrem & MFB_PIM);
	        ptemp = (CfbBits *)(psrcS + (xrem >> MFB_PWSH));
#if PSZ == 24
		tmpx = x & 3;
		pdsttmp = pdst + ((x * 3)>>2);
#else
#if PSZ != 32 || PPW != 1
		tmpx = x & PIM;
#endif
		pdsttmp = pdst + (x>>PWSH);
#endif
		switch ( pGC->fillStyle ) {
		    case FillOpaqueStippled:
#if PSZ == 24
			getstipplepixels24(ptemp, xtemp, 0, &bgfill, &tmpDst1, xrem);
			getstipplepixels24(ptemp, xtemp, 1, &fgfill, &tmpDst2, xrem);
#else
			getstipplepixels(ptemp, xtemp, w, 0, &bgfill, &tmpDst1);
			getstipplepixels(ptemp, xtemp, w, 1, &fgfill, &tmpDst2);
#endif
			break;
		    case FillStippled:
			/* Fill tmpSrc with the source pixels */
#if PSZ == 24
			getbits24(pdsttmp, tmpSrc, x);
			getstipplepixels24(ptemp, xtemp, 0, &tmpSrc, &tmpDst1, xrem);
#else
			getbits(pdsttmp, tmpx, w, tmpSrc);
			getstipplepixels(ptemp, xtemp, w, 0, &tmpSrc, &tmpDst1);
#endif
			if (rop != stiprop) {
#if PSZ == 24
			    putbitsrop24(fgfill, 0, &tmpSrc, pGC->planemask, stiprop);
#else
			    putbitsrop(fgfill, 0, w, &tmpSrc, pGC->planemask, stiprop);
#endif
			} else {
			    tmpSrc = fgfill;
			}
#if PSZ == 24
			getstipplepixels24(ptemp, xtemp, 1, &tmpSrc, &tmpDst2, xrem);
#else
			getstipplepixels(ptemp, xtemp, w, 1, &tmpSrc, &tmpDst2);
#endif
			break;
		}
		tmpDst2 |= tmpDst1;
#if PSZ == 24
		putbitsrop24(tmpDst2, tmpx, pdsttmp, pGC->planemask, rop);
#else
		putbitsrop(tmpDst2, tmpx, w, pdsttmp, pGC->planemask, rop);
#endif
		x += w;
		width -= w;
	    }
	}
	ppt++;
	pwidth++;
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}

#endif /* PSZ == 8 */

#if PSZ == 8

void
cfb8Stipple32FS (pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
				/* next three parameters are post-clip */
    int		    n;			/* number of spans to fill */
    DDXPointPtr	    ppt;		/* pointer to list of start points */
    int		    *pwidth;		/* pointer to list of n widths */
    CfbBits   *src;		/* pointer to bits in stipple, if needed */
    int		    stippleHeight;	/* height of the stipple */
    PixmapPtr	    stipple;

    int		    nlwDst;		/* width in longwords of the dest pixmap */
    int		    x,y,w;		/* current span */
    CfbBits   startmask;
    CfbBits   endmask;
    register CfbBits *dst;	/* pointer to bits we're writing */
    register int    nlw;
    CfbBits   *dstTmp;
    int		    nlwTmp;

    CfbBits   *pbits;		/* pointer to start of pixmap */
    register CfbBits  xor;
    register CfbBits  mask;
    register CfbBits  bits;	/* bits from stipple */
    int		    wEnd;

    int		    *pwidthFree;	/* copies of the pointers to free */
    DDXPointPtr	    pptFree;
    cfbPrivGCPtr    devPriv;

    devPriv = cfbGetGCPrivate(pGC);
    cfb8CheckStipple (pGC->alu, pGC->fgPixel, pGC->planemask);
    n = nInit * miFindMaxBand(pGC->pCompositeClip);
    if ( n == 0 )
	return;
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree)
    {
	if (pptFree) DEALLOCATE_LOCAL(pptFree);
	if (pwidthFree) DEALLOCATE_LOCAL(pwidthFree);
	return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(pGC->pCompositeClip, pptInit, pwidthInit, nInit,
		     ppt, pwidth, fSorted);

    stipple = pGC->pRotatedPixmap;
    src = (CfbBits *)stipple->devPrivate.ptr;
    stippleHeight = stipple->drawable.height;

    cfbGetLongWidthAndPointer (pDrawable, nlwDst, pbits)

    while (n--)
    {
    	w = *pwidth++;
	x = ppt->x;
    	y = ppt->y;
	ppt++;
    	dst = pbits + (y * nlwDst) + (x >> PWSH);
	if (((x & PIM) + w) <= PPW)
	{
	    maskpartialbits(x, w, startmask);
	    endmask = 0;
	    nlw = 0;
	}
	else
	{
	    maskbits (x, w, startmask, endmask, nlw);
	}
	bits = src[y % stippleHeight];
	RotBitsLeft (bits, (x & ((PGSZ-1) & ~PIM)));
#if PPW == 4
	if (cfb8StippleRRop == GXcopy)
	{
	    xor = devPriv->xor;
	    if (w < (PGSZ*2))
	    {
		if (startmask)
		{
		    mask = cfb8PixelMasks[GetBitGroup(bits)];
		    *dst = (*dst & ~(mask & startmask)) |
			   (xor & (mask & startmask));
		    dst++;
		    RotBitsLeft (bits, PGSZB);
		}
		while (nlw--)
		{
		    WriteBitGroup (dst,xor,GetBitGroup(bits))
		    dst++;
		    RotBitsLeft (bits, PGSZB);
		}
		if (endmask)
		{
		    mask = cfb8PixelMasks[GetBitGroup(bits)];
		    *dst = (*dst & ~(mask & endmask)) |
			   (xor & (mask & endmask));
		}
	    }
	    else
	    { /* XXX constants probably not OK here */
		wEnd = 7 - (nlw & 7);
		nlw = (nlw >> 3) + 1;
		dstTmp = dst;
		nlwTmp = nlw;
		if (startmask)
		{
		    mask = cfb8PixelMasks[GetBitGroup(bits)];
		    *dstTmp = (*dstTmp & ~(mask & startmask)) |
			   (xor & (mask & startmask));
		    dstTmp++;
		    RotBitsLeft (bits, PGSZB);
		}
		w = 7 - wEnd;
		while (w--)
		{
		    dst = dstTmp;
		    dstTmp++;
		    nlw = nlwTmp;
#if defined(__GNUC__) && defined(mc68020)
		    mask = cfb8PixelMasks[GetBitGroup(bits)];
		    xor = xor & mask;
		    mask = ~mask;
		    while (nlw--)
		    {
			*dst = (*dst & mask) | xor;
			dst += 8;
		    }
		    xor = devPriv->xor;
#else
#define SwitchBitsLoop(body) \
    while (nlw--)	\
    {		\
	body	\
	dst += 8;	\
    }
		    SwitchBitGroup(dst, xor, GetBitGroup(bits));
#undef SwitchBitsLoop
#endif
		    NextBitGroup (bits);
		}
		nlwTmp--;
		w = wEnd + 1;
		if (endmask)
		{
		    mask = cfb8PixelMasks[GetBitGroup(bits)];
		    dst = dstTmp + (nlwTmp << 3);
		    *dst = (*dst & ~(mask & endmask)) |
			   (xor &  (mask & endmask));
		}
		while (w--)
		{
		    nlw = nlwTmp;
		    dst = dstTmp;
		    dstTmp++;
#if defined(__GNUC__) && defined(mc68020)
		    mask = cfb8PixelMasks[GetBitGroup(bits)];
		    xor = xor & mask;
		    mask = ~mask;
		    while (nlw--)
		    {
			*dst = (*dst & mask) | xor;
			dst += 8;
		    }
		    xor = devPriv->xor;
#else
#define SwitchBitsLoop(body) \
	while (nlw--)	\
	{		\
	    body	\
	    dst += 8;	\
	}
		    SwitchBitGroup(dst, xor, GetBitGroup(bits));
#undef SwitchBitsLoop
#endif
		    NextBitGroup (bits);
		}
	    }
	}
	else
#endif /* PPW == 4 */
	{
	    if (startmask)
	    {
		xor = GetBitGroup(bits);
		*dst = MaskRRopPixels(*dst, xor, startmask);
		dst++;
		RotBitsLeft (bits, PGSZB);
	    }
	    while (nlw--)
	    {
		RRopBitGroup(dst, GetBitGroup(bits));
		dst++;
		RotBitsLeft (bits, PGSZB);
	    }
	    if (endmask)
	    {
		xor = GetBitGroup(bits);
		*dst = MaskRRopPixels(*dst, xor, endmask);
	    }
	}
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}

void
cfb8OpaqueStipple32FS (pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
				/* next three parameters are post-clip */
    int		    n;			/* number of spans to fill */
    DDXPointPtr	    ppt;		/* pointer to list of start points */
    int		    *pwidth;		/* pointer to list of n widths */
    CfbBits   *src;		/* pointer to bits in stipple, if needed */
    int		    stippleHeight;	/* height of the stipple */
    PixmapPtr	    stipple;

    int		    nlwDst;		/* width in longwords of the dest pixmap */
    int		    x,y,w;		/* current span */
    CfbBits   startmask;
    CfbBits   endmask;
    register CfbBits *dst;	/* pointer to bits we're writing */
    register int    nlw;
    CfbBits   *dstTmp;
    int		    nlwTmp;

    CfbBits   *pbits;		/* pointer to start of pixmap */
    register CfbBits  xor;
    register CfbBits  bits;	/* bits from stipple */
    int		    wEnd;

    int		    *pwidthFree;	/* copies of the pointers to free */
    DDXPointPtr	    pptFree;
    cfbPrivGCPtr    devPriv;

    devPriv = cfbGetGCPrivate(pGC);

    cfb8CheckOpaqueStipple(pGC->alu, pGC->fgPixel, pGC->bgPixel, pGC->planemask);

    n = nInit * miFindMaxBand(pGC->pCompositeClip);
    if ( n == 0 )
	return;
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree)
    {
	if (pptFree) DEALLOCATE_LOCAL(pptFree);
	if (pwidthFree) DEALLOCATE_LOCAL(pwidthFree);
	return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(pGC->pCompositeClip, pptInit, pwidthInit, nInit,
		     ppt, pwidth, fSorted);

    stipple = pGC->pRotatedPixmap;
    src = (CfbBits *)stipple->devPrivate.ptr;
    stippleHeight = stipple->drawable.height;

    cfbGetLongWidthAndPointer (pDrawable, nlwDst, pbits)

    while (n--)
    {
    	w = *pwidth++;
	x = ppt->x;
    	y = ppt->y;
	ppt++;
    	dst = pbits + (y * nlwDst) + (x >> PWSH);
	if (((x & PIM) + w) <= PPW)
	{
	    maskpartialbits(x, w, startmask);
	    endmask = 0;
	    nlw = 0;
	}
	else
	{
	    maskbits (x, w, startmask, endmask, nlw);
	}
	bits = src[y % stippleHeight];
	RotBitsLeft (bits, (x & ((PGSZ-1) & ~PIM)));
#if PPW == 4
	if (cfb8StippleRRop == GXcopy)
	{
	    xor = devPriv->xor;
	    if (w < PGSZ*2)
	    {
		if (startmask)
		{
		    *dst = (*dst & ~startmask) |
			   (GetPixelGroup (bits) & startmask);
		    dst++;
		    RotBitsLeft (bits, PGSZB);
		}
		while (nlw--)
		{
		    *dst++ = GetPixelGroup(bits);
		    RotBitsLeft (bits, PGSZB);
		}
		if (endmask)
		{
		    *dst = (*dst & ~endmask) |
			   (GetPixelGroup (bits) & endmask);
		}
	    }
	    else
	    { /* XXX consts probably not OK here */
		wEnd = 7 - (nlw & 7);
		nlw = (nlw >> 3) + 1;
		dstTmp = dst;
		nlwTmp = nlw;
		if (startmask)
		{
		    *dstTmp = (*dstTmp & ~startmask) |
			      (GetPixelGroup (bits) & startmask);
		    dstTmp++;
		    RotBitsLeft (bits, PGSZB);
		}
		w = 7 - wEnd;
		while (w--)
		{
		    nlw = nlwTmp;
		    dst = dstTmp;
		    dstTmp++;
		    xor = GetPixelGroup (bits);
		    while (nlw--)
		    {
			*dst = xor;
			dst += 8;
		    }
		    NextBitGroup (bits);
		}
		nlwTmp--;
		w = wEnd + 1;
		if (endmask)
		{
		    dst = dstTmp + (nlwTmp << 3);
		    *dst = (*dst & ~endmask) |
			   (GetPixelGroup (bits) & endmask);
		}
		while (w--)
		{
		    nlw = nlwTmp;
		    dst = dstTmp;
		    dstTmp++;
		    xor = GetPixelGroup (bits);
		    while (nlw--)
		    {
			*dst = xor;
			dst += 8;
		    }
		    NextBitGroup (bits);
		}
	    }
	}
	else
#endif /* PPW == 4 */
	{
	    if (startmask)
	    {
		xor = GetBitGroup(bits);
		*dst = MaskRRopPixels(*dst, xor, startmask);
		dst++;
		RotBitsLeft (bits, PGSZB);
	    }
	    while (nlw--)
	    {
		RRopBitGroup(dst, GetBitGroup(bits));
		dst++;
		RotBitsLeft (bits, PGSZB);
	    }
	    if (endmask)
	    {
		xor = GetBitGroup(bits);
		*dst = MaskRRopPixels(*dst, xor, endmask);
	    }
	}
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}

#endif /* PSZ == 8 */
