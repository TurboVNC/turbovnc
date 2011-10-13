/* $Xorg: cfbgetsp.c,v 1.4 2001/02/09 02:04:38 xorgcvs Exp $ */
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
/* $XFree86: xc/programs/Xserver/cfb/cfbgetsp.c,v 3.9tsi Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xmd.h>
#include "servermd.h"

#include "misc.h"
#include "region.h"
#include "gc.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "cfb.h"
#include "cfbmskbits.h"

/* GetSpans -- for each span, gets bits from drawable starting at ppt[i]
 * and continuing for pwidth[i] bits
 * Each scanline returned will be server scanline padded, i.e., it will come
 * out to an integral number of words.
 */
void
cfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pchardstStart)
    DrawablePtr		pDrawable;	/* drawable from which to get bits */
    int			wMax;		/* largest value of all *pwidths */
    register DDXPointPtr ppt;		/* points to start copying from */
    int			*pwidth;	/* list of number of bits to copy */
    int			nspans;		/* number of scanlines to copy */
    char		*pchardstStart; /* where to put the bits */
{
    PixelGroup	*pdstStart = (PixelGroup *)pchardstStart;
    register PixelGroup	*pdst;		/* where to put the bits */
    register PixelGroup	*psrc;		/* where to get the bits */
    register PixelGroup	tmpSrc;		/* scratch buffer for bits */
    PixelGroup		*psrcBase;	/* start of src bitmap */
    int			widthSrc;	/* width of pixmap in bytes */
    register DDXPointPtr pptLast;	/* one past last point to get */
    int         	xEnd;		/* last pixel to copy from */
    int			nl, srcBit;
    int			w;
    PixelGroup		*pdstNext;
#if PSZ == 24
    register char *psrcb, *pdstb;
    register int xIndex = 0;
#else
    register int	nstart; 
#if PSZ != 32 || PPW != 1
    int	 		nend; 
#endif
    PixelGroup		startmask, endmask;
    int			nlMiddle;
#endif

    switch (pDrawable->bitsPerPixel) {
	case 1:
	    mfbGetSpans(pDrawable, wMax, ppt, pwidth, nspans, pchardstStart);
	    return;
	case PSZ:
	    break;
	default:
	    FatalError("cfbGetSpans: invalid depth\n");
    }

    /*
     * XFree86 DDX empties the root borderClip when the VT is
     * switched away; this checks for that case
     */
    if (!cfbDrawableEnabled(pDrawable))
	return;
    
    cfbGetLongWidthAndPointer (pDrawable, widthSrc, psrcBase)

#ifdef PIXEL_ADDR
# if PSZ != 24
    if ((nspans == 1) && (*pwidth == 1))
    {
	tmpSrc = *((PixelType *)(psrcBase + (ppt->y * widthSrc))
		   + ppt->x);
#if BITMAP_BIT_ORDER == MSBFirst
	tmpSrc <<= (sizeof (CfbBits) - sizeof (PixelType)) * 8;
#endif
	*pdstStart = tmpSrc;
	return;
    }
# endif /* PSZ != 24 */
#endif
    pdst = pdstStart;
    pptLast = ppt + nspans;
    while(ppt < pptLast)
    {
#if PSZ == 24
	xEnd = min(ppt->x + *pwidth, widthSrc * sizeof(CfbBits) / 3);
	w = xEnd - ppt->x;
	psrc = psrcBase + ppt->y * widthSrc;
	srcBit = ppt->x;
	psrcb = (char *)psrc + (ppt->x * 3);
	xIndex = 0;
	pdstb = (char *)pdst;
    	pdstNext = pdst + ((w * 3 + 3) >> 2);
#else
	xEnd = min(ppt->x + *pwidth, widthSrc << PWSH);
	w = xEnd - ppt->x;
	psrc = psrcBase + ppt->y * widthSrc + (ppt->x >> PWSH); 
	srcBit = ppt->x & PIM;
    	pdstNext = pdst + ((w + PPW - 1) >> PWSH);
#endif

#if PSZ == 24
	if (w < 0)
	  FatalError("cfb24GetSpans: Internal error (w < 0)\n");
	nl = w;
	while (nl--){ 
	  psrc = (PixelGroup *)((unsigned long)psrcb & ~0x03);
	  getbits24(psrc, tmpSrc, srcBit);
	  pdst = (PixelGroup *)((unsigned long)pdstb & ~0x03);
	  putbits24(tmpSrc, PPW, pdst, ~((CfbBits)0), xIndex);
	  srcBit++;
	  psrcb += 3;
	  xIndex++;
	  pdstb += 3;
	} 
	pdst = pdstNext;
#else /* PSZ == 24 */
	if (srcBit + w <= PPW) 
	{ 
	    getbits(psrc, srcBit, w, tmpSrc);
	    putbits(tmpSrc, 0, w, pdst, ~((CfbBits)0)); 
	    pdst++;
	} 
	else 
	{ 
	    maskbits(ppt->x, w, startmask, endmask, nlMiddle);
	    nstart = 0; 
	    if (startmask) 
	    { 
		nstart = PPW - srcBit; 
		getbits(psrc, srcBit, nstart, tmpSrc);
		putbits(tmpSrc, 0, nstart, pdst, ~((CfbBits)0));
		if(srcBit + nstart >= PPW)
		    psrc++;
	    } 
	    nl = nlMiddle; 
	    while (nl--) 
	    { 
		tmpSrc = *psrc;
		putbits(tmpSrc, nstart, PPW, pdst, ~((CfbBits)0));
		psrc++;
		pdst++;
	    } 
	    if (endmask) 
	    { 
#if PSZ != 32 || PPW != 1
		nend = xEnd & PIM; 
#endif
		getbits(psrc, 0, nend, tmpSrc);
		putbits(tmpSrc, nstart, nend, pdst, ~((CfbBits)0));
	    } 
	    pdst = pdstNext;
	} 
#endif /* PSZ == 24 */
        ppt++;
	pwidth++;
    }
}
