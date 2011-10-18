/************************************************************

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

********************************************************/

/* $XConsortium: cfbpolypnt.c,v 5.17 94/04/17 20:28:57 dpw Exp $ */
/* $XFree86: xc/programs/Xserver/cfb/cfbpolypnt.c,v 3.0 1996/06/29 09:05:47 dawes Exp $ */

#include "X.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "cfb.h"
#include "cfbmskbits.h"

#define isClipped(c,ul,lr)  ((((c) - (ul)) | ((lr) - (c))) & ClipMask)

/* WARNING: pbox contains two shorts. This code assumes they are packed
 * and can be referenced together as an INT32.
 */

#define PointLoop(fill) { \
    for (nbox = REGION_NUM_RECTS(cclip), pbox = REGION_RECTS(cclip); \
	 --nbox >= 0; \
	 pbox++) \
    { \
	c1 = *((INT32 *) &pbox->x1) - off; \
	c2 = *((INT32 *) &pbox->x2) - off - 0x00010001; \
	for (ppt = (INT32 *) pptInit, i = npt; --i >= 0;) \
	{ \
	    pt = *ppt++; \
	    if (!isClipped(pt,c1,c2)) { \
		fill \
	    } \
	} \
    } \
}

#if PSZ == 24
# include "cfbrrop24.h"
#endif

void
cfbPolyPoint(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr pGC;
    int mode;
    int npt;
    xPoint *pptInit;
{
    register INT32   pt;
    register INT32   c1, c2;
    register unsigned long   ClipMask = 0x80008000;
    register unsigned long   xor;
#ifdef PIXEL_ADDR
    register PixelType   *addrp;
    register int    npwidth;
    PixelType	    *addrpt;
#else
    register unsigned long    *addrl;
    register int    nlwidth;
    register int    xoffset;
    unsigned long   *addrlt;
#endif
#if PSZ == 24
    RROP_DECLARE
    register int xtmp;
    register PixelType *p;
#endif
    register INT32  *ppt;
    RegionPtr	    cclip;
    int		    nbox;
    register int    i;
    register BoxPtr pbox;
    unsigned long   and;
    int		    rop = pGC->alu;
    int		    off;
    cfbPrivGCPtr    devPriv;
    xPoint	    *pptPrev;

    devPriv =cfbGetGCPrivate(pGC);
    rop = devPriv->rop;
    if (rop == GXnoop)
	return;
    cclip = devPriv->pCompositeClip;
    xor = devPriv->xor;
    if ((mode == CoordModePrevious) && (npt > 1))
    {
	for (pptPrev = pptInit + 1, i = npt - 1; --i >= 0; pptPrev++)
	{
	    pptPrev->x += (pptPrev-1)->x;
	    pptPrev->y += (pptPrev-1)->y;
	}
    }
    off = *((int *) &pDrawable->x);
    off -= (off & 0x8000) << 1;
#ifdef PIXEL_ADDR
    cfbGetPixelWidthAndPointer(pDrawable, npwidth, addrp);
#if PSZ == 24
    addrp = addrp + pDrawable->y * npwidth;
#else
    addrp = addrp + pDrawable->y * npwidth + pDrawable->x;
#endif
    if (rop == GXcopy)
    {
#if PSZ == 24
      RROP_COPY_SETUP(xor)
#endif
	if (!(npwidth & (npwidth - 1)))
	{
	    npwidth = ffs(npwidth) - 1;
#if PSZ == 24
	    PointLoop(
		      xtmp = pDrawable->x + intToX(pt);
		      p = addrp + (intToY(pt) << npwidth) + ((xtmp * 3) >>2);
		      RROP_SOLID24_COPY(p, xtmp))
#else
	    PointLoop(*(addrp + (intToY(pt) << npwidth) + intToX(pt)) = xor;)
#endif
	}
#ifdef sun
	else if (npwidth == 1152)
	{
	    register int    y;
	    PointLoop(y = intToY(pt); *(addrp + (y << 10) + (y << 7) + intToX(pt)) = xor;)
	}
#endif
	else
	{
#if PSZ == 24
	    PointLoop(
		      xtmp = pDrawable->x + intToX(pt);
		      p = addrp + intToY(pt) * npwidth + ((xtmp * 3) >> 2);
		      RROP_SOLID24_COPY(p, xtmp))
#else
	    PointLoop(*(addrp + intToY(pt) * npwidth + intToX(pt)) = xor;)
#endif
	}
    }
    else
    {
	and = devPriv->and;
#if PSZ == 24
	RROP_SET_SETUP(xor, and)
	PointLoop(  
		  xtmp = pDrawable->x + intToX(pt);
		  p = addrp + intToY(pt) * npwidth + ((xtmp * 3) >> 2);
		  RROP_SOLID24_SET(p, xtmp))
#else
	PointLoop(  addrpt = addrp + intToY(pt) * npwidth + intToX(pt);
		    *addrpt = DoRRop (*addrpt, and, xor);)
#endif
    }
#else /* !PIXEL_ADDR */
    cfbGetLongWidthAndPointer(pDrawable, nlwidth, addrl);
    addrl = addrl + pDrawable->y * nlwidth + (pDrawable->x >> PWSH);
    xoffset = pDrawable->x & PIM;
    and = devPriv->and;
#if PSZ == 24
    PointLoop(   addrlt = addrl + intToY(pt) * nlwidth
 	                   + ((intToX(pt) + xoffset) >> PWSH);
 		   *addrlt = DoRRop (*addrlt,
 			   and | ~cfbmask[(intToX(pt) + xoffset) & PIM],
 			   xor & cfbmask[(intToX(pt) + xoffset) & PIM]);
	     )
#else
    PointLoop(   addrlt = addrl + intToY(pt) * nlwidth
 	                   + ((intToX(pt) + xoffset) >> PWSH);
 		   *addrlt = DoRRop (*addrlt,
 			   and | ~cfbmask[((intToX(pt) + xoffset) & 3)<<1],
 			   xor & cfbmask[((intToX(pt) + xoffset) & 3)<<1]);
	     )
#endif
#endif /* PIXEL_ADDR */
}
