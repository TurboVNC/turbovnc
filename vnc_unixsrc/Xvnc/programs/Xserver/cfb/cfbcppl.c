/*
 * $TOG: cfb8cppl.c /main/16 1998/02/09 14:04:13 kaleb $
 *
Copyright 1990, 1998  The Open Group

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
 *
 * Author:  Keith Packard, MIT X Consortium
 */
/* $XFree86: xc/programs/Xserver/cfb/cfbcppl.c,v 1.6 2001/12/14 19:59:22 dawes Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xmd.h>
#include "gcstruct.h"
#include "window.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "cfb.h"
#if PSZ == 8
#undef   PSZ /* for maskbits.h */
#include "maskbits.h"
#define PSZ 8
#include "mergerop.h"
#else /* PSZ==8 */
#include "cfbtab.h" /* provides starttab, endttab, partmasks */
#endif /* PSZ==8 */


void
cfbCopyImagePlane(
    DrawablePtr pSrcDrawable,
    DrawablePtr pDstDrawable,
    int	rop,
    RegionPtr prgnDst,
    DDXPointPtr pptSrc,
    unsigned long planemask)
{
    /* note: there must be some sort of trick behind,
       passing a planemask value with all bits set
       whilst using the current planemask for the bitPlane value. */
#if PSZ == 8
    cfbCopyPlane8to1 (pSrcDrawable, pDstDrawable, rop, prgnDst, pptSrc,
		      (unsigned long) ~0L, planemask);
#endif
#if PSZ == 16
    cfbCopyPlane16to1 (pSrcDrawable, pDstDrawable, rop, prgnDst, pptSrc,
		      (unsigned long) ~0L, planemask);
#endif
#if PSZ == 24
    cfbCopyPlane24to1 (pSrcDrawable, pDstDrawable, rop, prgnDst, pptSrc,
		      (unsigned long) ~0L, planemask);
#endif
#if PSZ == 32
    cfbCopyPlane32to1 (pSrcDrawable, pDstDrawable, rop, prgnDst, pptSrc,
		      (unsigned long) ~0L, planemask);
#endif
}

#if PSZ == 8

#if BITMAP_BIT_ORDER == MSBFirst
#define LeftMost    (MFB_PPW-1)
#define StepBit(bit, inc)  ((bit) -= (inc))
#else
#define LeftMost    0
#define StepBit(bit, inc)  ((bit) += (inc))
#endif

#define GetBits(psrc, nBits, curBit, bitPos, bits) {\
    bits = 0; \
    while (nBits--) \
    { \
	bits |= (PixelType)(((*psrc++ >> bitPos) & 1)) << curBit; \
	StepBit (curBit, 1); \
    } \
}

void
cfbCopyPlane8to1(
    DrawablePtr pSrcDrawable,
    DrawablePtr pDstDrawable,
    int	rop,
    RegionPtr prgnDst,
    DDXPointPtr pptSrc,
    unsigned long planemask,
    unsigned long bitPlane)
{
    int			    srcx, srcy, dstx, dsty, width, height;
    unsigned char	    *psrcBase;
    PixelType		    *pdstBase;
    int			    widthSrc, widthDst;
    unsigned char	    *psrcLine;
    PixelType		    *pdstLine;
    register unsigned char  *psrc;
    register int	    i;
    register int	    curBit;
    register int	    bitPos;
    register CfbBits  bits;
    register PixelType	    *pdst;
    PixelType		    startmask, endmask;
    int			    niStart = 0, niEnd = 0;
    int			    bitStart = 0, bitEnd = 0;
    int			    nl, nlMiddle;
    int			    nbox;
    BoxPtr		    pbox;
    MROP_DECLARE()

    if (!(planemask & 1))
	return;

    if (rop != GXcopy)
	MROP_INITIALIZE (rop, planemask);

    cfbGetByteWidthAndPointer (pSrcDrawable, widthSrc, psrcBase)

    mfbGetPixelWidthAndPointer (pDstDrawable, widthDst, pdstBase)

    bitPos = ffs (bitPlane) - 1;

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
	psrcLine = psrcBase + srcy * widthSrc + srcx;
	pdstLine = mfbScanline(pdstBase, dstx, dsty, widthDst);
	dstx &= MFB_PIM;
	if (dstx + width <= MFB_PPW)
	{
	    maskpartialbits(dstx, width, startmask);
	    nlMiddle = 0;
	    endmask = 0;
	}
	else
	{
	    maskbits (dstx, width, startmask, endmask, nlMiddle);
	}
	if (startmask)
	{
	    niStart = min(MFB_PPW - dstx, width);
	    bitStart = LeftMost;
	    StepBit (bitStart, dstx);
	}
	if (endmask)
	{
	    niEnd = (dstx + width) & MFB_PIM;
	    bitEnd = LeftMost;
	}
	if (rop == GXcopy)
	{
	    while (height--)
	    {
	    	psrc = psrcLine;
	    	pdst = pdstLine;
	    	psrcLine += widthSrc;
	    	mfbScanlineInc(pdstLine, widthDst);
	    	if (startmask)
	    	{
		    i = niStart;
		    curBit = bitStart;
		    GetBits (psrc, i, curBit, bitPos, bits);
		    *pdst = (*pdst & ~startmask) | bits;
		    pdst++;
	    	}
	    	nl = nlMiddle;
	    	while (nl--)
	    	{
		    i = MFB_PPW;
		    curBit = LeftMost;
		    GetBits (psrc, i, curBit, bitPos, bits);
		    *pdst++ = bits;
	    	}
	    	if (endmask)
	    	{
		    i = niEnd;
		    curBit = bitEnd;
		    GetBits (psrc, i, curBit, bitPos, bits);
		    *pdst = (*pdst & ~endmask) | bits;
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
	    	mfbScanlineInc(pdstLine, widthDst);
	    	if (startmask)
	    	{
		    i = niStart;
		    curBit = bitStart;
		    GetBits (psrc, i, curBit, bitPos, bits);
		    *pdst = MROP_MASK(bits, *pdst, startmask);
		    pdst++;
	    	}
	    	nl = nlMiddle;
	    	while (nl--)
	    	{
		    i = MFB_PPW;
		    curBit = LeftMost;
		    GetBits (psrc, i, curBit, bitPos, bits);
		    *pdst = MROP_SOLID(bits, *pdst);
		    pdst++;
	    	}
	    	if (endmask)
	    	{
		    i = niEnd;
		    curBit = bitEnd;
		    GetBits (psrc, i, curBit, bitPos, bits);
		    *pdst = MROP_MASK (bits, *pdst, endmask);
	    	}
	    }
	}
    }
}

#else /*  PSZ == 8 */

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


#if PSZ == 24
#define GetBits(psrc, nBits, curBit, bitPos, bits) {\
    bits = 0; \
    while (nBits--) \
    { \
	if (bitPos < 8) \
        { \
	    bits |= ((*psrc++ >> bitPos) & 1) << curBit; \
	    psrc += 2; \
        } \
	else if (bitPos < 16) \
        { \
	    psrc++; \
	    bits |= ((*psrc++ >> (bitPos - 8)) & 1) << curBit; \
	    psrc++; \
        } \
	else \
        { \
	    psrc += 2; \
	    bits |= ((*psrc++ >> (bitPos - 16)) & 1) << curBit; \
        } \
	StepBit (curBit, 1); \
    } \
}
#else
#define GetBits(psrc, nBits, curBit, bitPos, bits) {\
    bits = 0; \
    while (nBits--) \
    { \
	bits |= ((*psrc++ >> bitPos) & 1) << curBit; \
	StepBit (curBit, 1); \
    } \
}
#endif

void
#if PSZ == 16
cfbCopyPlane16to1
#endif
#if PSZ == 24
cfbCopyPlane24to1
#endif
#if PSZ == 32
cfbCopyPlane32to1
#endif
(
    DrawablePtr pSrcDrawable,
    DrawablePtr pDstDrawable,
    int	rop,
    RegionPtr prgnDst,
    DDXPointPtr pptSrc,
    unsigned long planemask,
    unsigned long bitPlane)
{
    int			    srcx, srcy, dstx, dsty, width, height;
    CfbBits	    *psrcBase;
    CfbBits	    *pdstBase;
    int			    widthSrc, widthDst;
#if PSZ == 16
    unsigned short	    *psrcLine;
    register unsigned short *psrc;
#endif
#if PSZ == 24
    unsigned char	    *psrcLine;
    register unsigned char  *psrc;
#endif
#if PSZ == 32
    unsigned int	    *psrcLine;
    register unsigned int   *psrc;
#endif
    unsigned int	    *pdstLine;
    register unsigned int   *pdst;
    register int	    i;
    register int	    curBit;
    register int	    bitPos;
    register unsigned int   bits;
    unsigned int	    startmask = 0, endmask = 0;
    int			    niStart = 0, niEnd = 0;
    int			    bitStart = 0, bitEnd = 0;
    int			    nl, nlMiddle;
    int			    nbox;
    BoxPtr		    pbox;
    int result;


    if (!(planemask & 1))
	return;

    /* must explicitly ask for "int" widths, as code below expects it */
    /* on some machines (Alpha), "long" and "int" are not the same size */
    cfbGetTypedWidthAndPointer (pSrcDrawable, widthSrc, psrcBase, int, CfbBits)
    cfbGetTypedWidthAndPointer (pDstDrawable, widthDst, pdstBase, int, CfbBits)

#if PSZ == 16
    widthSrc <<= 1;
#endif
#if PSZ == 24
    widthSrc <<= 2;
#endif

    bitPos = ffs (bitPlane) - 1;

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
#if PSZ == 16
	psrcLine = (unsigned short *)psrcBase + srcy * widthSrc + srcx;
#endif
#if PSZ == 24
	psrcLine = (unsigned char *)psrcBase + srcy * widthSrc + srcx * 3;
#endif
#if PSZ == 32
	psrcLine = (unsigned int *)psrcBase + srcy * widthSrc + srcx;
#endif
	pdstLine = (unsigned int *)pdstBase + dsty * widthDst + (dstx >> 5);
	if (dstx + width <= 32)
	{
	    mfbmaskpartialbits(dstx, width, startmask);
	    nlMiddle = 0;
	    endmask = 0;
	}
	else
	{
	    mfbmaskbits (dstx, width, startmask, endmask, nlMiddle);
	}
	if (startmask)
	{
	    niStart = 32 - (dstx & 0x1f);
	    bitStart = LeftMost;
	    StepBit (bitStart, (dstx & 0x1f));
	}
	if (endmask)
	{
	    niEnd = (dstx + width) & 0x1f;
	    bitEnd = LeftMost;
	}
	if (rop == GXcopy)
	{
	    while (height--)
	    {
	    	psrc = psrcLine;
	    	pdst = pdstLine;
	    	psrcLine += widthSrc;
	    	pdstLine += widthDst;
	    	if (startmask)
	    	{
		    i = niStart;
		    curBit = bitStart;
		    GetBits (psrc, i, curBit, bitPos, bits);

		    *pdst = (*pdst & ~startmask) | bits;
		    pdst++;
	    	}
	    	nl = nlMiddle;
	    	while (nl--)
	    	{
		    i = 32;
		    curBit = LeftMost;
		    GetBits (psrc, i, curBit, bitPos, bits);
		    *pdst++ = bits;
	    	}
	    	if (endmask)
	    	{
		    i = niEnd;
		    curBit = bitEnd;
		    GetBits (psrc, i, curBit, bitPos, bits);

		    *pdst = (*pdst & ~endmask) | bits;
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
	    	if (startmask)
	    	{
		    i = niStart;
		    curBit = bitStart;
		    GetBits (psrc, i, curBit, bitPos, bits);
                    DoRop (result, rop, bits, *pdst);

		    *pdst = (*pdst & ~startmask) | 
			    (result & startmask);
		    pdst++;
	    	}
	    	nl = nlMiddle;
 	    	while (nl--)
	    	{
		    i = 32;
		    curBit = LeftMost;
		    GetBits (psrc, i, curBit, bitPos, bits);
                    DoRop (result, rop, bits, *pdst);
		    *pdst = result;
		    ++pdst;
	    	}
	    	if (endmask)
	    	{
		    i = niEnd;
		    curBit = bitEnd;
		    GetBits (psrc, i, curBit, bitPos, bits);
                    DoRop (result, rop, bits, *pdst);

		    *pdst = (*pdst & ~endmask) |
			    (result & endmask);
	    	}
	    }
	}
    }
}

#endif  /* PSZ == 8 */
