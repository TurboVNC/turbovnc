/*
 * Fill 32 bit tiled rectangles.  Used by both PolyFillRect and PaintWindow.
 * no depth dependencies.
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
*/

/* $XConsortium: cfbtile32.c,v 1.8 94/04/17 20:29:05 dpw Exp $ */
/* $XFree86: xc/programs/Xserver/cfb/cfbtile32.c,v 3.0 1996/06/29 09:05:53 dawes Exp $ */

#include "X.h"
#include "Xmd.h"
#include "servermd.h"
#include "gcstruct.h"
#include "window.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "windowstr.h"

#include "cfb.h"
#include "cfbmskbits.h"
#include "cfb8bit.h"

#include "mergerop.h"

#include "mi.h"
#include "mispans.h"

#ifdef sparc
#define SHARED_IDCACHE
#endif

#if PSZ == 24
#define STORE(p)    (*(p) = MROP_PREBUILT_SOLID(srcpix,*(p)))
/*#define STORE24(p,index)    {\
	    register int idx = ((index) & 3)<< 1; \
	    *(p) = (((MROP_PREBUILT_SOLID(srcpix,*(p))<<cfb24Shift[idx])&cfbmask[idx])| \
	            (*(p)&cfbrmask[idx])); \
	    idx++; \
	    (p)++; \
	    *(p) = (((MROP_PREBUILT_SOLID(srcpix,*(p))>>cfb24Shift[idx])&cfbmask[idx])| \
	            (*(p)&cfbrmask[idx])); \
	    (p)--; \
	}*/
#define STORE24(p,index)    MROP_PREBUILT_SOLID24(srcpix, (p), index)

#define STORE_MASK(p,mask)    (*(p) = MROP_PREBUILT_MASK(srcpix,*(p),(mask)))
#define QSTORE(p)    ((*(p) = MROP_PREBUILT_SOLID(((srcpix<<24)|srcpix),*(p))), \
                      (p)++,(*(p) = MROP_PREBUILT_SOLID(((srcpix<<16)|(srcpix>>8)),*(p))), \
                      (p)++,(*(p) = MROP_PREBUILT_SOLID(((srcpix<<8)|(srcpix>>16)),*(p))))



/* XXX TJR: I doubt that this optimised case works (because the non-24 bit case
   was broken), so I've added the #if 0 below.  Someone who knows what they're
   doing can re-enable it if they fix it */

#if (MROP == Mcopy) && defined(FAST_CONSTANT_OFFSET_MODE) && defined(SHARED_IDCACHE) && 0
# define Expand(left,right) {\
    int part = nlwMiddle & ((PGSZB*2)-1); \
    nlwMiddle *= 3; \
    nlwMiddle >>= PWSH + 3; \
    while (h--) { \
	srcpix = psrc[srcy]; \
	MROP_PREBUILD(srcpix); \
	++srcy; \
	if (srcy == tileHeight) \
	    srcy = 0; \
	left \
	p += part; \
	switch (part) { \
	case 7: \
	    STORE24(p - 7, xtmp - 7); \
	case 6: \
	    STORE24(p - 6, xtmp - 6); \
	case 5: \
	    STORE24(p - 5, xtmp - 5); \
	case 4: \
	    STORE24(p - 4, xtmp - 4); \
	case 3: \
	    STORE24(p - 3, xtmp - 3); \
	case 2: \
	    STORE24(p - 2, xtmp - 2); \
	case 1: \
	    STORE24(p - 1, xtmp - 1); \
	} \
	nlw = nlwMiddle; \
	while (nlw) { \
	    STORE24 (p + 0, xtmp + 0); \
	    STORE24 (p + 1, xtmp + 1); \
	    STORE24 (p + 2, xtmp + 2); \
	    STORE24 (p + 3, xtmp + 3); \
	    STORE24 (p + 4, xtmp + 4); \
	    STORE24 (p + 5, xtmp + 5); \
	    STORE24 (p + 6, xtmp + 6); \
	    STORE24 (p + 7, xtmp + 7); \
	    p += 8; \
	    xtmp += 8; \
	    nlw--; \
	} \
	right \
	p += nlwExtra; \
    } \
}
#else
#define Expand(left,right) {\
    while (h--)	{ \
	srcpix = psrc[srcy]; \
	MROP_PREBUILD(srcpix); \
	++srcy; \
	if (srcy == tileHeight) \
	    srcy = 0; \
	left \
	while (nlw--) \
	{ \
	    STORE24(p,xtmp); \
	    if(xtmp&3) p++; \
	    xtmp++; \
	} \
	right \
	p += nlwExtra; \
    } \
}
#endif
#else /*PSZ != 24*/
#define STORE(p)    (*(p) = MROP_PREBUILT_SOLID(srcpix,*(p)))

#if (MROP == Mcopy) && defined(FAST_CONSTANT_OFFSET_MODE) && defined(SHARED_IDCACHE)
# define Expand(left,right) {\
    int part = nlwMiddle & 7; \
    nlwMiddle >>= 3; \
    while (h--) { \
	srcpix = psrc[srcy]; \
	MROP_PREBUILD(srcpix); \
	++srcy; \
	if (srcy == tileHeight) \
	    srcy = 0; \
	left \
	p += part; \
	switch (part) { \
	case 7: \
	    STORE(p - 7); \
	case 6: \
	    STORE(p - 6); \
	case 5: \
	    STORE(p - 5); \
	case 4: \
	    STORE(p - 4); \
	case 3: \
	    STORE(p - 3); \
	case 2: \
	    STORE(p - 2); \
	case 1: \
	    STORE(p - 1); \
	} \
	nlw = nlwMiddle; \
	while (nlw) { \
	    STORE (p + 0); \
	    STORE (p + 1); \
	    STORE (p + 2); \
	    STORE (p + 3); \
	    STORE (p + 4); \
	    STORE (p + 5); \
	    STORE (p + 6); \
	    STORE (p + 7); \
	    p += 8; \
	    nlw--; \
	} \
	right \
	p += nlwExtra; \
    } \
}
#else
#define Expand(left,right) {\
    while (h--)	{ \
	srcpix = psrc[srcy]; \
	MROP_PREBUILD(srcpix); \
	++srcy; \
	if (srcy == tileHeight) \
	    srcy = 0; \
	left \
	nlw = nlwMiddle; \
	while (nlw--) \
	{ \
	    STORE(p); \
	    p++; \
	} \
	right \
	p += nlwExtra; \
    } \
}
#endif
#endif /*PSZ == 24*/

void
MROP_NAME(cfbFillRectTile32) (pDrawable, pGC, nBox, pBox)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    int		    nBox;	/* number of boxes to fill */
    BoxPtr 	    pBox;	/* pointer to list of boxes to fill */
{
    register unsigned long srcpix;	
    unsigned long *psrc;		/* pointer to bits in tile, if needed */
    int tileHeight;	/* height of the tile */

    int nlwDst;		/* width in longwords of the dest pixmap */
    int w;		/* width of current box */
    register int h;	/* height of current box */
    register unsigned long startmask;
    register unsigned long endmask; /* masks for reggedy bits at either end of line */
    int nlwMiddle;	/* number of longwords between sides of boxes */
    int nlwExtra;	/* to get from right of box to left of next span */
    register int nlw;	/* loop version of nlwMiddle */
    register unsigned long *p;	/* pointer to bits we're writing */
    int y;		/* current scan line */
    int srcy;		/* current tile position */

    unsigned long *pbits;/* pointer to start of pixmap */
    PixmapPtr	    tile;	/* rotated, expanded tile */
    MROP_DECLARE_REG()
    MROP_PREBUILT_DECLARE()
#if PSZ == 24
    unsigned long xtmp;
#endif

    tile = cfbGetGCPrivate(pGC)->pRotatedPixmap;
    tileHeight = tile->drawable.height;
    psrc = (unsigned long *)tile->devPrivate.ptr;

    MROP_INITIALIZE(pGC->alu, pGC->planemask);

    cfbGetLongWidthAndPointer (pDrawable, nlwDst, pbits)

    while (nBox--)
    {
	w = pBox->x2 - pBox->x1;
	h = pBox->y2 - pBox->y1;
	y = pBox->y1;
#if PSZ == 24
	xtmp = pBox->x1;
	p = pbits + (y * nlwDst) + ((pBox->x1*3) >> 2);
/*	p = pbits + (y * nlwDst) + ((pBox->x1>> 2)*3);*/
#else
	p = pbits + (y * nlwDst) + (pBox->x1 >> PWSH);
#endif
	srcy = y % tileHeight;

#if PSZ == 24
	if (w == 1  &&  ((pBox->x1 & 3) == 0  ||  (pBox->x1 & 3) == 3))
#else
	if ( ((pBox->x1 & PIM) + w) <= PPW)
#endif
	{
	    maskpartialbits(pBox->x1, w, startmask);
	    nlwExtra = nlwDst;
	    while (h--)
	    {
		srcpix = psrc[srcy];
		MROP_PREBUILD(srcpix);
		++srcy;
		if (srcy == tileHeight)
		    srcy = 0;
		*p = MROP_PREBUILT_MASK (srcpix, *p, startmask);
		p += nlwExtra;
	    }
	}
	else
	{
	    maskbits(pBox->x1, w, startmask, endmask, nlwMiddle);
	    nlwExtra = nlwDst - nlwMiddle;

	    if (startmask)
	    {
		nlwExtra -= 1;
		if (endmask)
		{
		    Expand(*p = MROP_PREBUILT_MASK(srcpix, *p, startmask); p++;,
			   *p = MROP_PREBUILT_MASK(srcpix, *p, endmask);)
		}
		else
		{
		    Expand(*p = MROP_PREBUILT_MASK(srcpix, *p, startmask); p++;,
			   ;)
		}
	    }
	    else
	    {
		if (endmask)
		{
		    Expand(;,
			   *p = MROP_PREBUILT_MASK(srcpix, *p, endmask);)
		}
		else
		{
		    Expand(;,
			   ;)
		}
	    }
	}
        pBox++;
    }
}

void
MROP_NAME(cfbTile32FS)(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
				/* next three parameters are post-clip */
    int			n;	/* number of spans to fill */
    DDXPointPtr		ppt;	/* pointer to list of start points */
    int			*pwidth;/* pointer to list of n widths */
    unsigned long	*pbits;	/* pointer to start of bitmap */
    int			nlwDst;	/* width in longwords of bitmap */
    register unsigned long *p;	/* pointer to current longword in bitmap */
    register int	w;	/* current span width */
    register int	nlw;
    register int	x;
    register unsigned long startmask;
    register unsigned long endmask;
    register unsigned long  srcpix;
    int			y;
    int			*pwidthFree;/* copies of the pointers to free */
    DDXPointPtr		pptFree;
    PixmapPtr		tile;
    unsigned long	*psrc;	/* pointer to bits in tile */
    int			tileHeight;/* height of the tile */
    MROP_DECLARE_REG ()
    MROP_PREBUILT_DECLARE()
#if PSZ == 24      
    unsigned long	xtmp;
#endif

    n = nInit * miFindMaxBand( cfbGetCompositeClip(pGC) );
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

    tile = cfbGetGCPrivate(pGC)->pRotatedPixmap;
    tileHeight = tile->drawable.height;
    psrc = (unsigned long *)tile->devPrivate.ptr;

    MROP_INITIALIZE(pGC->alu, pGC->planemask);

    cfbGetLongWidthAndPointer (pDrawable, nlwDst, pbits)

#if MROP == Mcopy
    if (!(tileHeight & (tileHeight-1)))
    {
	tileHeight--;
    	while (n--)
    	{
	    x = ppt->x;
	    y = ppt->y;
	    ++ppt;
	    w = *pwidth++;
#if PSZ == 24
/*	    p = pbits + (y * nlwDst) + ((x*3) >> 2);*/
	    xtmp = x;
	    p = pbits + (y * nlwDst) + ((x >> 2)*3);
#else
	    p = pbits + (y * nlwDst) + (x >> PWSH);
#endif
	    srcpix = psrc[y & tileHeight];
	    MROP_PREBUILD(srcpix);
    
#if PSZ == 24
	    if ((x & 3) + w < 5)
#else
	    if ((x & PIM) + w < PPW)
#endif
	    {
	    	maskpartialbits(x, w, startmask);
	    	*p = MROP_PREBUILT_MASK (srcpix, *p, startmask);
	    }
	    else
	    {
	    	maskbits(x, w, startmask, endmask, nlw);
	    	if (startmask)
	    	{
		    *p = MROP_PREBUILT_MASK(srcpix, *p, startmask);
#if PSZ == 24
		    if(xtmp&3) p++;
		    xtmp++;
#else
		    p++;
#endif
	    	}
	    	while (nlw--)
	    	{
#if PSZ == 24
		    STORE24(p,xtmp);
		    if(xtmp&3) p++;
		    ++xtmp;
#else
		    STORE(p);
		    ++p;
#endif
	    	}
	    	if (endmask)
	    	{
		    *p = MROP_PREBUILT_MASK(srcpix, *p, endmask);
	    	}
	    }
    	}
    }
    else
#endif
    {
    	while (n--)
    	{
	    x = ppt->x;
	    y = ppt->y;
	    ++ppt;
	    w = *pwidth++;
#if PSZ == 24
/*	    p = pbits + (y * nlwDst) + ((x *3)>> 2);*/
	    p = pbits + (y * nlwDst) + ((x >> 2)*3);
	    xtmp = x;
#else
	    p = pbits + (y * nlwDst) + (x >> PWSH);
#endif
	    srcpix = psrc[y % tileHeight];
	    MROP_PREBUILD(srcpix);
    
#if PSZ == 24
	    if ((x & 3) + w < 5)
#else
	    if ((x & PIM) + w < PPW)
#endif
	    {
	    	maskpartialbits(x, w, startmask);
	    	*p = MROP_PREBUILT_MASK (srcpix, *p, startmask);
	    }
	    else
	    {
	    	maskbits(x, w, startmask, endmask, nlw);
	    	if (startmask)
	    	{
		    *p = MROP_PREBUILT_MASK(srcpix, *p, startmask);
#if PSZ == 24
		    if(xtmp&3)p++;
		    xtmp++;
#else
		    p++;
#endif
	    	}
	    	while (nlw--)
	    	{
#if PSZ == 24
		    STORE24(p,xtmp);
		    if(xtmp&3)p++;
		    xtmp++;
#else
		    STORE(p);
		    ++p;
#endif
	    	}
	    	if (endmask)
	    	{
		    *p = MROP_PREBUILT_MASK(srcpix, *p, endmask);
	    	}
	    }
    	}
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
}
