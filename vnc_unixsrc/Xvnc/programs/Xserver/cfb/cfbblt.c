/*
 * cfb copy area
 */
/* $XFree86: xc/programs/Xserver/cfb/cfbblt.c,v 3.13tsi Exp $ */

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
/* $Xorg: cfbblt.c,v 1.4 2001/02/09 02:04:37 xorgcvs Exp $ */

/* 24-bit bug fixes: Peter Wainwright, 1998/11/28 */

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
#include	"cfb.h"
#include	"cfbmskbits.h"
#include	"cfb8bit.h"
#include	"fastblt.h"
#include	"mergerop.h"

#ifdef notdef /* XXX fails right now, walks off end of pixmaps */
#if defined (FAST_UNALIGNED_READS) && PSZ == 8
#define DO_UNALIGNED_BITBLT
#endif
#endif

#if defined(FAST_MEMCPY) && (MROP == Mcopy) && PSZ == 8
#define DO_MEMCPY
#endif

/* ................................................. */
/* SPECIAL CODE FOR 24 BITS      by Peter Wainwright */

#if PSZ == 24 && (MROP) == 0

/* The default macros are defined in mergerop.h, and none of them are
   really appropriate for what we want to do.

   There are two ways of fixing this: either define SLOW_24BIT_COPY
   to copy pixel by pixel, or (by default) use the following macros
   modified from mergerop.h

   MROP_SOLID and MROP_MASK are defined for each of the operations,
   i.e. each value of MROP.

   There are special cases for Mcopy, McopyInverted, Mxor, and Mor.
   There is a completely generic version for MROP=0, and a simplified
   generic version which works for (Mcopy|Mxor|MandReverse|Mor).

   However, the generic version does not work for the 24-bit case
   because the pixels cannot be packed exactly into a machine word (32
   bits).

   Alternative macros MROP_SOLID24 and MROP_MASK24 are provided for
   the 24-bit case. However, these each copy a single *pixel*, not a
   single machine word. They take an rvalue source pixel, an lvalue
   destination, and the pixel index. The latter is used to find the
   position of the pixel data within the two words *dst and *(dst+1).

   Further macros MROP_SOLID24P and MROP_MASK24P are used to copy from
   an lvalue source to an lvalue destination. MROP_PIXEL24 is used to
   assemble the source pixel from the adjacent words *src and
   *(src+1), and this is then split between the destination words
   using the non-P macros above.

   But we want to copy entire words for the sake of efficiency.
   Unfortunately if a plane mask is specified this must be shifted
   from one word to the next.  Fortunately the pattern repeats after 3
   words, so we unroll the planemask here and redefine MROP_SOLID
   and MROP_MASK. */


#endif /* MROP == 0 && PSZ == 24 */

/* ................................................. */

#if PSZ == 24
#define BYPP 3
#if PGSZ == 32
#define P3W 4 /* pixels in 3 machine words */
#define PAM 3 /* pixel align mask; PAM = P3W -1 */
#define P2WSH 2
#else
#define P3W 8 /* pixels in 3 machine words */
#define PAM 7 /* pixel align mask; PAM = P3W -1 */
#define P2WSH 3
#endif
#endif

void
MROP_NAME(cfbDoBitblt)(
    DrawablePtr	    pSrc, 
    DrawablePtr	    pDst,
    int		    alu,
    RegionPtr	    prgnDst,
    DDXPointPtr	    pptSrc,
    unsigned long   planemask)
{
    CfbBits *psrcBase, *pdstBase;	
				/* start of src and dst bitmaps */
    int widthSrc, widthDst;	/* add to get to same position in next line */

    BoxPtr pbox;
    int nbox;

    BoxPtr pboxTmp, pboxNext, pboxBase, pboxNew1, pboxNew2;
				/* temporaries for shuffling rectangles */
    DDXPointPtr pptTmp, pptNew1, pptNew2;
				/* shuffling boxes entails shuffling the
				   source points too */
    int w, h;
    int xdir;			/* 1 = left right, -1 = right left/ */
    int ydir;			/* 1 = top down, -1 = bottom up */

    CfbBits *psrcLine, *pdstLine;	
				/* pointers to line with current src and dst */
    register CfbBits *psrc;/* pointer to current src longword */
    register CfbBits *pdst;/* pointer to current dst longword */

    MROP_DECLARE_REG()

				/* following used for looping through a line */
    CfbBits startmask, endmask;	/* masks for writing ends of dst */
    int nlMiddle;		/* whole longwords in dst */
    int xoffSrc, xoffDst;
    register int nl;		/* temp copy of nlMiddle */
    int careful;

#if (PSZ != 24) || (MROP != 0)
    register int leftShift, rightShift;
    register CfbBits bits;
    register CfbBits bits1;
#endif

#if PSZ == 24
#ifdef DO_MEMCPY
    int w2;
#endif

#if MROP == 0
    int widthSrcBytes = cfbGetByteWidth(pSrc);
    int widthDstBytes = cfbGetByteWidth(pDst);
#endif
#endif

    MROP_INITIALIZE(alu,planemask)

    cfbGetLongWidthAndPointer (pSrc, widthSrc, psrcBase)

    cfbGetLongWidthAndPointer (pDst, widthDst, pdstBase)

    /* XXX we have to err on the side of safety when both are windows,
     * because we don't know if IncludeInferiors is being used.
     */
    careful = ((pSrc == pDst) ||
	       ((pSrc->type == DRAWABLE_WINDOW) &&
		(pDst->type == DRAWABLE_WINDOW)));

    pbox = REGION_RECTS(prgnDst);
    nbox = REGION_NUM_RECTS(prgnDst);

    pboxNew1 = NULL;
    pptNew1 = NULL;
    pboxNew2 = NULL;
    pptNew2 = NULL;
    if (careful && (pptSrc->y < pbox->y1))
    {
        /* walk source botttom to top */
	ydir = -1;
	widthSrc = -widthSrc;
	widthDst = -widthDst;

	if (nbox > 1)
	{
	    /* keep ordering in each band, reverse order of bands */
	    pboxNew1 = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
	    if(!pboxNew1)
		return;
	    pptNew1 = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * nbox);
	    if(!pptNew1)
	    {
	        DEALLOCATE_LOCAL(pboxNew1);
	        return;
	    }
	    pboxBase = pboxNext = pbox+nbox-1;
	    while (pboxBase >= pbox)
	    {
	        while ((pboxNext >= pbox) &&
		       (pboxBase->y1 == pboxNext->y1))
		    pboxNext--;
	        pboxTmp = pboxNext+1;
	        pptTmp = pptSrc + (pboxTmp - pbox);
	        while (pboxTmp <= pboxBase)
	        {
		    *pboxNew1++ = *pboxTmp++;
		    *pptNew1++ = *pptTmp++;
	        }
	        pboxBase = pboxNext;
	    }
	    pboxNew1 -= nbox;
	    pbox = pboxNew1;
	    pptNew1 -= nbox;
	    pptSrc = pptNew1;
        }
    }
    else
    {
	/* walk source top to bottom */
	ydir = 1;
    }

    if (careful && (pptSrc->x < pbox->x1))
    {
	/* walk source right to left */
        xdir = -1;

	if (nbox > 1)
	{
	    /* reverse order of rects in each band */
	    pboxNew2 = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
	    pptNew2 = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * nbox);
	    if(!pboxNew2 || !pptNew2)
	    {
		if (pptNew2) DEALLOCATE_LOCAL(pptNew2);
		if (pboxNew2) DEALLOCATE_LOCAL(pboxNew2);
		if (pboxNew1)
		{
		    DEALLOCATE_LOCAL(pptNew1);
		    DEALLOCATE_LOCAL(pboxNew1);
		}
	        return;
	    }
	    pboxBase = pboxNext = pbox;
	    while (pboxBase < pbox+nbox)
	    {
	        while ((pboxNext < pbox+nbox) &&
		       (pboxNext->y1 == pboxBase->y1))
		    pboxNext++;
	        pboxTmp = pboxNext;
	        pptTmp = pptSrc + (pboxTmp - pbox);
	        while (pboxTmp != pboxBase)
	        {
		    *pboxNew2++ = *--pboxTmp;
		    *pptNew2++ = *--pptTmp;
	        }
	        pboxBase = pboxNext;
	    }
	    pboxNew2 -= nbox;
	    pbox = pboxNew2;
	    pptNew2 -= nbox;
	    pptSrc = pptNew2;
	}
    }
    else
    {
	/* walk source left to right */
        xdir = 1;
    }

    while(nbox--)
    {
	w = pbox->x2 - pbox->x1;
	h = pbox->y2 - pbox->y1;

#if PSZ == 24
#ifdef DO_MEMCPY
	w2 = w * BYPP;
#endif
#endif
	if (ydir == -1) /* start at last scanline of rectangle */
	{
	    psrcLine = psrcBase + ((pptSrc->y+h-1) * -widthSrc);
	    pdstLine = pdstBase + ((pbox->y2-1) * -widthDst);
	}
	else /* start at first scanline */
	{
	    psrcLine = psrcBase + (pptSrc->y * widthSrc);
	    pdstLine = pdstBase + (pbox->y1 * widthDst);
	}
#if PSZ == 24
	if (w == 1 && ((pbox->x1 & PAM) == 0  ||  (pbox->x1 & PAM) == PAM))
#else
	if ((pbox->x1 & PIM) + w <= PPW)
#endif
	{
	    maskpartialbits (pbox->x1, w, endmask);
	    startmask = 0;
	    nlMiddle = 0;
	}
	else
	{
	    maskbits(pbox->x1, w, startmask, endmask, nlMiddle);
	}

#if PSZ == 24
#if 0
	nlMiddle = w - (pbox->x2 &PAM);;
	if(pbox->x1 & PAM){
	  nlMiddle -= (PAM+1 - (pbox->x1 &PAM));
	}
	nlMiddle >>= P2WSH;
	if(nlMiddle < 0)
	  nlMiddle = 0;
#endif
#endif

#ifdef DO_MEMCPY
	/* If the src and dst scanline don't overlap, do forward case.  */

	if ((xdir == 1) || (pptSrc->y != pbox->y1)
		|| (pptSrc->x + w <= pbox->x1))
	{
#if PSZ == 24
	    char *psrc = (char *) psrcLine + (pptSrc->x * BYPP);
	    char *pdst = (char *) pdstLine + (pbox->x1 * BYPP);
#else
	    char *psrc = (char *) psrcLine + pptSrc->x;
	    char *pdst = (char *) pdstLine + pbox->x1;
#endif
	    while (h--)
	    {
#if PSZ == 24
	    	memcpy(pdst, psrc, w2);
#else
	    	memcpy(pdst, psrc, w);
#endif
		pdst += widthDst << PWSH;
		psrc += widthSrc << PWSH;
	    }
	}
#else /* ! DO_MEMCPY */
	if (xdir == 1)
	{
#if PSZ == 24 && MROP == 0
	    /* Note: x is a pixel number; the byte offset is 3*x;
	       therefore the offset within a word is (3*x) & 3 ==
	       (4*x-x) & 3 == (-x) & 3.  The offsets therefore
	       DECREASE by 1 for each pixel.
	    */
	  xoffSrc = ( - pptSrc->x) & PAM;
	  xoffDst = ( - pbox->x1) & PAM;
#if 1
	  if((int)xoffSrc != (int)xoffDst /* Alignments must be same. */
	     || ((widthDstBytes & PAM) != (widthSrcBytes & PAM) && h > 1))
#else
	    if(1)
#endif
	    /* Width also must be same, if hight > 1 */
	    {
	      /* ...otherwise, pixel by pixel operation */
	  while (h--)
	    {
	      register int i, si, sii, di;

	      for (i = 0, si = pptSrc->x, di = pbox->x1;
		   i < w;
		   i++, si++, di++) {
		    psrc = psrcLine + ((si * BYPP) >> P2WSH);
		    pdst = pdstLine + ((di * BYPP) >> P2WSH);
		sii = (si & 3);
		MROP_SOLID24P(psrc, pdst, sii, di);
	      }
	      pdstLine += widthDst;
	      psrcLine += widthSrc;
	    }
	  }
	  else
#endif
	  {

#if PSZ == 24

#if MROP != 0
	    xoffSrc = ( - pptSrc->x) & PAM;
	    xoffDst = ( - pbox->x1) & PAM;
#endif
	    pdstLine += (pbox->x1 * BYPP) >> P2WSH;
	    psrcLine += (pptSrc->x * BYPP) >> P2WSH;
#else
	    xoffSrc = pptSrc->x & PIM;
	    xoffDst = pbox->x1 & PIM;
	    pdstLine += (pbox->x1 >> PWSH);
	    psrcLine += (pptSrc->x >> PWSH);
#endif
#ifdef DO_UNALIGNED_BITBLT
	    nl = xoffSrc - xoffDst;
	    psrcLine = (CfbBits *)
			(((unsigned char *) psrcLine) + nl);
#else
#if PSZ == 24 && MROP == 0
	    /* alredy satisfied */
#else
	    if (xoffSrc == xoffDst)
#endif
#endif
	    {
		while (h--)
		{
#if PSZ == 24 && MROP == 0
		    register int index;
		    register int im3;
#endif /*  PSZ == 24 && MROP == 0 */
		    psrc = psrcLine;
		    pdst = pdstLine;
		    pdstLine += widthDst;
		    psrcLine += widthSrc;
#if PSZ == 24 && MROP == 0
		    index = (int)(pdst - pdstBase);
		    im3 = index % 3;
#endif /*  PSZ == 24 && MROP == 0 */
		    if (startmask)
		    {
#if PSZ == 24 && MROP == 0
		      	*pdst = DoMaskMergeRop24u(*psrc, *pdst, startmask, im3);
			index++;
			im3 = index % 3;
#else /* PSZ != 24 || MROP != 0 */
			*pdst = MROP_MASK(*psrc, *pdst, startmask);
#endif /*  PSZ == 24 && MROP == 0 */
			psrc++;
			pdst++;
		    }

		    nl = nlMiddle;
#ifdef LARGE_INSTRUCTION_CACHE
#ifdef FAST_CONSTANT_OFFSET_MODE

		    psrc += nl & (UNROLL-1);
		    pdst += nl & (UNROLL-1);

#if PSZ == 24 && MROP == 0
#define BodyOdd(n) pdst[-n] = DoMergeRop24u(psrc[-n], pdst[-n], ((int)(pdst - n - pdstBase))%3);
#define BodyEven(n) pdst[-n] = DoMergeRop24u(psrc[-n], pdst[-n], ((int)(pdst - n - pdstBase))%3);
#else /* PSZ != 24 || MROP != 0 */
#define BodyOdd(n) pdst[-n] = MROP_SOLID (psrc[-n], pdst[-n]);
#define BodyEven(n) pdst[-n] = MROP_SOLID (psrc[-n], pdst[-n]);
#endif /*  PSZ == 24 && MROP == 0 */

#define LoopReset \
pdst += UNROLL; \
psrc += UNROLL;

#else

#if PSZ == 24 && MROP == 0
#define BodyOdd(n)  *pdst = DoMergeRop24u(*psrc, *pdst, im3); pdst++; psrc++; index++; im3 = index % 3;
#define BodyEven(n) BodyOdd(n)
#else /* PSZ != 24 || MROP != 0 */
#define BodyOdd(n)  *pdst = MROP_SOLID (*psrc, *pdst); pdst++; psrc++;
#define BodyEven(n) BodyOdd(n)
#endif /*  PSZ == 24 && MROP == 0 */

#define LoopReset   ;

#endif
		    PackedLoop

#undef BodyOdd
#undef BodyEven
#undef LoopReset

#else
#ifdef NOTDEF
		    /* you'd think this would be faster --
		     * a single instruction instead of 6
		     * but measurements show it to be ~15% slower
		     */
		    while ((nl -= 6) >= 0)
		    {
			asm ("moveml %1+,#0x0c0f;moveml#0x0c0f,%0"
			     : "=m" (*(char *)pdst)
			     : "m" (*(char *)psrc)
			     : "d0", "d1", "d2", "d3",
			       "a2", "a3");
			pdst += 6;
		    }
		    nl += 6;
		    while (nl--)
			*pdst++ = *psrc++;
#endif
#if 0 /*PSZ == 24 && MROP == 0*/
		    DuffL(nl, label1,
			    *pdst = DoMergeRop24u(*psrc, *pdst, im3);
			    pdst++; psrc++; index++;im3 = index % 3;)
#else /* !(PSZ == 24 && MROP == 0) */
		    DuffL(nl, label1,
			    *pdst = MROP_SOLID (*psrc, *pdst);
			    pdst++; psrc++;)
#endif /* PSZ == 24 && MROP == 0 */
#endif

		    if (endmask)
#if PSZ == 24 && MROP == 0
			*pdst = DoMaskMergeRop24u(*psrc, *pdst, endmask, (int)(pdst - pdstBase) % 3);
#else /* !(PSZ == 24 && MROP == 0) */
			*pdst = MROP_MASK(*psrc, *pdst, endmask);
#endif /* PSZ == 24 && MROP == 0 */
		}
	    }
#ifndef DO_UNALIGNED_BITBLT
#if PSZ == 24 && MROP == 0
		/* can not happen */ 
#else /* !(PSZ == 24 && MROP == 0) */
	    else /* xoffSrc != xoffDst */
	    {
		if (xoffSrc > xoffDst)
		{
#if PSZ == 24
		    leftShift = (xoffSrc - xoffDst) << 3;
#else
#if PGSZ == 32
		    leftShift = (xoffSrc - xoffDst) << (5 - PWSH);
#else /* PGSZ == 64 */
		    leftShift = (xoffSrc - xoffDst) << (6 - PWSH);
#endif /* PGSZ */
#endif
		    rightShift = PGSZ - leftShift;
		}
		else
		{
#if PSZ == 24
		    rightShift = (xoffDst - xoffSrc) << 3;
#else
#if PGSZ == 32
		    rightShift = (xoffDst - xoffSrc) << (5 - PWSH);
#else /* PGSZ == 64 */
		    rightShift = (xoffDst - xoffSrc) << (6 - PWSH);
#endif /* PGSZ */
#endif
		    leftShift = PGSZ - rightShift;
		}
		while (h--)
		{
		    psrc = psrcLine;
		    pdst = pdstLine;
		    pdstLine += widthDst;
		    psrcLine += widthSrc;
		    bits = 0;
		    if (xoffSrc > xoffDst)
			bits = *psrc++;
		    if (startmask)
		    {
			bits1 = BitLeft(bits,leftShift);
			bits = *psrc++;
			bits1 |= BitRight(bits,rightShift);
			*pdst = MROP_MASK(bits1, *pdst, startmask);
			pdst++;
		    }
		    nl = nlMiddle;
#ifdef LARGE_INSTRUCTION_CACHE
		    bits1 = bits;

#ifdef FAST_CONSTANT_OFFSET_MODE

		    psrc += nl & (UNROLL-1);
		    pdst += nl & (UNROLL-1);

#define BodyOdd(n) \
bits = psrc[-n]; \
pdst[-n] = MROP_SOLID(BitLeft(bits1, leftShift) | BitRight(bits, rightShift), pdst[-n]);

#define BodyEven(n) \
bits1 = psrc[-n]; \
pdst[-n] = MROP_SOLID(BitLeft(bits, leftShift) | BitRight(bits1, rightShift), pdst[-n]);

#define LoopReset \
pdst += UNROLL; \
psrc += UNROLL;

#else

#define BodyOdd(n) \
bits = *psrc++; \
*pdst = MROP_SOLID(BitLeft(bits1, leftShift) | BitRight(bits, rightShift), *pdst); \
pdst++;
		   
#define BodyEven(n) \
bits1 = *psrc++; \
*pdst = MROP_SOLID(BitLeft(bits, leftShift) | BitRight(bits1, rightShift), *pdst); \
pdst++;

#define LoopReset   ;

#endif	/* !FAST_CONSTANT_OFFSET_MODE */

		    PackedLoop

#undef BodyOdd
#undef BodyEven
#undef LoopReset

#else
		    DuffL (nl,label2,
			bits1 = BitLeft(bits, leftShift);
			bits = *psrc++;
			*pdst = MROP_SOLID (bits1 | BitRight(bits, rightShift), *pdst);
			pdst++;
		    )
#endif

		    if (endmask)
		    {
			bits1 = BitLeft(bits, leftShift);
			if (BitLeft(endmask, rightShift))
			{
			    bits = *psrc;
			    bits1 |= BitRight(bits, rightShift);
			}
			*pdst = MROP_MASK (bits1, *pdst, endmask);
		    }
		}
	    }
#endif /* (PSZ == 24 && MROP == 0) */
#endif /* DO_UNALIGNED_BITBLT */

	  }
	}
#endif /* ! DO_MEMCPY */
	else	/* xdir == -1 */
	{
#if PSZ == 24 && MROP == 0
	  xoffSrc = (-(pptSrc->x + w)) & PAM;
	  xoffDst = (-pbox->x2) & PAM;
#if 1
	  if(xoffSrc != xoffDst /* Alignments must be same. */
	     || ((widthDstBytes & PAM) != (widthSrcBytes & PAM) && h > 1))
#else
	    if(1)
#endif
	    /* Width also must be same, if hight > 1 */
	    {
	      /* ...otherwise, pixel by pixel operation */
	  while (h--)
	    {
	      register int i, si, sii, di;

		    for (i = 0, si = pptSrc->x + w - 1, di = pbox->x2 - 1;
		   i < w;
			 i++, si--, di--) {
		      psrc = psrcLine + ((si * BYPP) >> P2WSH);
		      pdst = pdstLine + ((di * BYPP) >> P2WSH);
		      sii = (si & PAM);
		MROP_SOLID24P(psrc, pdst, sii, di);
	      }
	      psrcLine += widthSrc;
	      pdstLine += widthDst;
	    }
	  }else
#endif /* MROP == 0 && PSZ == 24 */
	    {

#if PSZ == 24
#if MROP == 0
	      /* already calculated */
#else
	    xoffSrc = (pptSrc->x + w) & PAM;
	    xoffDst = pbox->x2 & PAM;
#endif
	    pdstLine += ((pbox->x2 * BYPP - 1) >> P2WSH) + 1;
	    psrcLine += (((pptSrc->x+w) * BYPP - 1) >> P2WSH) + 1;
#else
	    xoffSrc = (pptSrc->x + w - 1) & PIM;
	    xoffDst = (pbox->x2 - 1) & PIM;
	    pdstLine += ((pbox->x2-1) >> PWSH) + 1;
	    psrcLine += ((pptSrc->x+w - 1) >> PWSH) + 1;
#endif
#ifdef DO_UNALIGNED_BITBLT
#if PSZ == 24
	    nl = xoffDst - xoffSrc;
#else
	    nl = xoffSrc - xoffDst;
#endif
	    psrcLine = (CfbBits *)
			(((unsigned char *) psrcLine) + nl);
#else
#if PSZ == 24 && MROP == 0
	    /* already satisfied */
#else
	    if (xoffSrc == xoffDst)
#endif
#endif
	    {
		while (h--)
		{
#if PSZ == 24 && MROP == 0
		    register int index;
		    register int im3;
#endif /*  PSZ == 24 && MROP == 0 */
		    psrc = psrcLine;
		    pdst = pdstLine;
		    pdstLine += widthDst;
		    psrcLine += widthSrc;
#if PSZ == 24 && MROP == 0
		    index = (int)(pdst - pdstBase);
#endif /*  PSZ == 24 && MROP == 0 */

		    if (endmask)
		    {
			pdst--;
			psrc--;
#if PSZ == 24 && MROP == 0
			index--;
			im3 = index % 3;
			*pdst = DoMaskMergeRop24u(*psrc, *pdst, endmask, im3);
#else /* !(PSZ == 24 && MROP == 0) */
			*pdst = MROP_MASK (*psrc, *pdst, endmask);
#endif /* PSZ == 24 && MROP == 0 */
		    }
		    nl = nlMiddle;
#ifdef LARGE_INSTRUCTION_CACHE
#ifdef FAST_CONSTANT_OFFSET_MODE
		    psrc -= nl & (UNROLL - 1);
		    pdst -= nl & (UNROLL - 1);

#if PSZ == 24 && MROP == 0
#define BodyOdd(n) pdst[n-1] = DoMergeRop24u(psrc[n-1], pdst[n-1], ((int)(pdst - (n - 1) -pdstBase)) % 3);
#else /* !(PSZ == 24 && MROP == 0) */
#define BodyOdd(n) pdst[n-1] = MROP_SOLID (psrc[n-1], pdst[n-1]);
#endif /* PSZ == 24 && MROP == 0 */

#define BodyEven(n) BodyOdd(n)

#define LoopReset \
pdst -= UNROLL;\
psrc -= UNROLL;

#else

#if PSZ == 24 && MROP == 0
#define BodyOdd(n)  --pdst; --psrc; --index; im3 = index % 3;*pdst = DoMergeRop24u(*psrc, *pdst, im3);
#else /* !(PSZ == 24 && MROP == 0) */
#define BodyOdd(n)  --pdst; --psrc; *pdst = MROP_SOLID(*psrc, *pdst);
#endif /* PSZ == 24 && MROP == 0 */
#define BodyEven(n) BodyOdd(n)
#define LoopReset   ;

#endif
		    PackedLoop

#undef BodyOdd
#undef BodyEven
#undef LoopReset

#else
#if PSZ == 24 && MROP == 0
		    DuffL(nl,label3,
			  --pdst; --psrc; --index; im3= index%3;*pdst = DoMergeRop24u(*psrc, *pdst, im3);)
#else /* !(PSZ == 24 && MROP == 0) */
		    DuffL(nl,label3,
			 --pdst; --psrc; *pdst = MROP_SOLID (*psrc, *pdst);)
#endif /* PSZ == 24 && MROP == 0 */
#endif

		    if (startmask)
		    {
			--pdst;
			--psrc;
#if PSZ == 24 && MROP == 0
			*pdst = DoMaskMergeRop24u(*psrc, *pdst, startmask, (int)(pdst - pdstBase) % 3);
#else /* !(PSZ == 24 && MROP == 0) */
			*pdst = MROP_MASK(*psrc, *pdst, startmask);
#endif /* PSZ == 24 && MROP == 0 */
		    }
		}
	    }
#ifndef DO_UNALIGNED_BITBLT
#if PSZ == 24 && MROP == 0
	    /* can not happen */
#else /* !( PSZ == 24 && MROP == 0) */
	    else
	    {
		if (xoffDst > xoffSrc)
		{
#if PSZ == 24
		    leftShift = (xoffDst - xoffSrc) << 3;
		    rightShift = PGSZ - leftShift;
#else
#if PGSZ == 32
		    rightShift = (xoffDst - xoffSrc) << (5 - PWSH);
#else /* PGSZ == 64 */
		    rightShift = (xoffDst - xoffSrc) << (6 - PWSH);
#endif /* PGSZ */
		    leftShift = PGSZ - rightShift;
#endif
		}
		else
		{
#if PSZ == 24
		    rightShift = (xoffSrc - xoffDst) << 3;
		    leftShift = PGSZ - rightShift;
#else
#if PGSZ == 32
		    leftShift = (xoffSrc - xoffDst) << (5 - PWSH);
#else /* PGSZ == 64 */
		    leftShift = (xoffSrc - xoffDst) << (6 - PWSH);
#endif /* PGSZ */
		    rightShift = PGSZ - leftShift;
#endif
		}
		while (h--)
		{
		    psrc = psrcLine;
		    pdst = pdstLine;
		    pdstLine += widthDst;
		    psrcLine += widthSrc;
		    bits = 0;
#if PSZ == 24
		    if (xoffSrc > xoffDst)
#else
		    if (xoffDst > xoffSrc)
#endif
			bits = *--psrc;
		    if (endmask)
		    {
			bits1 = BitRight(bits, rightShift);
			bits = *--psrc;
			bits1 |= BitLeft(bits, leftShift);
			pdst--;
			*pdst = MROP_MASK(bits1, *pdst, endmask);
		    }
		    nl = nlMiddle;
#ifdef LARGE_INSTRUCTION_CACHE
		    bits1 = bits;
#ifdef FAST_CONSTANT_OFFSET_MODE
		    psrc -= nl & (UNROLL - 1);
		    pdst -= nl & (UNROLL - 1);

#define BodyOdd(n) \
bits = psrc[n-1]; \
pdst[n-1] = MROP_SOLID(BitRight(bits1, rightShift) | BitLeft(bits, leftShift),pdst[n-1]);

#define BodyEven(n) \
bits1 = psrc[n-1]; \
pdst[n-1] = MROP_SOLID(BitRight(bits, rightShift) | BitLeft(bits1, leftShift),pdst[n-1]);

#define LoopReset \
pdst -= UNROLL; \
psrc -= UNROLL;

#else

#define BodyOdd(n) \
bits = *--psrc; --pdst; \
*pdst = MROP_SOLID(BitRight(bits1, rightShift) | BitLeft(bits, leftShift),*pdst);

#define BodyEven(n) \
bits1 = *--psrc; --pdst; \
*pdst = MROP_SOLID(BitRight(bits, rightShift) | BitLeft(bits1, leftShift),*pdst);

#define LoopReset   ;

#endif

		    PackedLoop

#undef BodyOdd
#undef BodyEven
#undef LoopReset

#else
		    DuffL (nl, label4,
			bits1 = BitRight(bits, rightShift);
			bits = *--psrc;
			--pdst;
			*pdst = MROP_SOLID(bits1 | BitLeft(bits, leftShift),*pdst);
		    )
#endif

		    if (startmask)
		    {
			bits1 = BitRight(bits, rightShift);
			if (BitRight (startmask, leftShift))
			{
			    bits = *--psrc;
			    bits1 |= BitLeft(bits, leftShift);
			}
			--pdst;
			*pdst = MROP_MASK(bits1, *pdst, startmask);
		    }
		}
	    }
#endif  /* PSZ == 24 && MROP == 0 */
#endif
	    }
	}
	pbox++;
	pptSrc++;
    }
    if (pboxNew2)
    {
	DEALLOCATE_LOCAL(pptNew2);
	DEALLOCATE_LOCAL(pboxNew2);
    }
    if (pboxNew1)
    {
	DEALLOCATE_LOCAL(pptNew1);
	DEALLOCATE_LOCAL(pboxNew1);
    }
}
