/*
 * Fill odd tiled rectangles and spans.
 * no depth dependencies.
 */
/* $XFree86: xc/programs/Xserver/cfb/cfbtileodd.c,v 3.6tsi Exp $ */

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
*/

/* $Xorg: cfbtileodd.c,v 1.4 2001/02/09 02:04:39 xorgcvs Exp $ */

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
#include "cfb8bit.h"

#include "mergerop.h"

#if PSZ == 24
#define LEFTSHIFT_AMT (3)
#else /* PSZ != 24 */
#define LEFTSHIFT_AMT (5 - PWSH)
#endif /* PSZ == 24*/

#define LastTileBits {\
    tmp = bits; \
    if (tileEndPart) \
	bits = (*pSrc & tileEndMask) | BitRight (*pSrcLine, tileEndLeftShift); \
    else \
	bits = *pSrc; \
}

#if PSZ == 24
#define ResetTileBits {\
    pSrc = pSrcLine; \
    nlwSrc = widthSrc;\
    if (tileEndPart) { \
	if (4 - xoff + tileEndPart <= 4) {\
	    bits = *pSrc++; \
	    nlwSrc--; \
	} else \
	    bits = BitLeft(tmp, tileEndLeftShift) | \
		   BitRight(bits, tileEndRightShift); \
	xoff = (xoff + xoffStep) & 3; \
	leftShift = xoff << LEFTSHIFT_AMT; \
	rightShift = PGSZ - leftShift; \
    }\
}
#else
#define ResetTileBits {\
    pSrc = pSrcLine; \
    nlwSrc = widthSrc;\
    if (tileEndPart) { \
	if (PPW - xoff + tileEndPart <= PPW) {\
	    bits = *pSrc++; \
	    nlwSrc--; \
	} else \
	    bits = BitLeft(tmp, tileEndLeftShift) | \
		   BitRight(bits, tileEndRightShift); \
	xoff = (xoff + xoffStep) & PIM; \
	leftShift = xoff << LEFTSHIFT_AMT; \
	rightShift = PGSZ - leftShift; \
    }\
}
#endif

#define NextTileBits {\
    if (nlwSrc == 1) {\
	LastTileBits\
    } else { \
    	if (nlwSrc == 0) {\
	    ResetTileBits\
    	} \
	if (nlwSrc == 1) {\
	    LastTileBits\
	} else {\
	    tmp = bits; \
	    bits = *pSrc++; \
	}\
    }\
    nlwSrc--; \
}

void
MROP_NAME(cfbFillBoxTileOdd) (pDrawable, nBox, pBox, tile, xrot, yrot, alu, planemask)
    DrawablePtr	    pDrawable;
    int		    nBox;	/* number of boxes to fill */
    register BoxPtr pBox;	/* pointer to list of boxes to fill */
    PixmapPtr	    tile;	/* tile */
    int		    xrot, yrot;
    int		    alu;
    unsigned long   planemask;
{
    int tileWidth;	/* width of tile in pixels */
    int tileHeight;	/* height of the tile */
    int widthSrc;

    int widthDst;	/* width in longwords of the dest pixmap */
    int w;		/* width of current box */
    int h;		/* height of current box */
    CfbBits startmask;
    CfbBits endmask;/* masks for reggedy bits at either end of line */
    int nlwMiddle;	/* number of longwords between sides of boxes */
    int nlwSrc;		/* number of whole longwords in source */
    
    register int nlw;	/* loop version of nlwMiddle */
    int srcy;		/* current tile y position */
    int srcx;		/* current tile x position */
    int xoffDst, xoffSrc;
    int leftShift, rightShift;

#if MROP == 0 && PSZ == 24
    DeclareMergeRop24()
#else
    MROP_DECLARE_REG()
#endif

    CfbBits *pdstBase;	/* pointer to start of dest */
    CfbBits *pDstLine;	/* poitner to start of dest box */
    CfbBits *pSrcBase;	/* pointer to start of source */
    CfbBits *pSrcLine;	/* pointer to start of source line */
    register CfbBits *pDst;
    register CfbBits *pSrc;
    register CfbBits bits, tmp = 0;
    int xoffStart, xoff;
    int leftShiftStart, rightShiftStart, nlwSrcStart;
    CfbBits tileEndMask;
    int tileEndLeftShift, tileEndRightShift;
    int	xoffStep;
    int tileEndPart;
    int needFirst;
    CfbBits   narrow[2];
    CfbBits   narrowMask = 0;
    int	    narrowShift = 0;
    Bool    narrowTile;

#if MROP == 0 && PSZ == 24
    InitializeMergeRop24 (alu, planemask)
#else
    MROP_INITIALIZE (alu, planemask)
#endif

    tileHeight = tile->drawable.height;
    tileWidth = tile->drawable.width;
    widthSrc = tile->devKind / PGSZB;
    narrowTile = FALSE;
    if (widthSrc == 1)
    {
	narrowShift = tileWidth;
	narrowMask = cfbendpartial [tileWidth];
	tileWidth *= 2;
	widthSrc = 2;
	narrowTile = TRUE;
    }
    pSrcBase = (CfbBits *)tile->devPrivate.ptr;

    cfbGetLongWidthAndPointer (pDrawable, widthDst, pdstBase)

#if PSZ == 24
    tileEndPart = (4 - tileWidth) & 3;
    tileEndMask = cfbendpartial[tileWidth & 3];
#else
    tileEndPart = tileWidth & PIM;
    tileEndMask = cfbendpartial[tileEndPart];
#endif /* PSZ == 24 */
    tileEndLeftShift = (tileEndPart) << LEFTSHIFT_AMT;
    tileEndRightShift = PGSZ - tileEndLeftShift;
#if PSZ == 24
    xoffStep = 4 - tileEndPart;
#else
    xoffStep = PPW - tileEndPart;
#endif /* PSZ == 24 */
    /*
     * current assumptions: tile > 32 bits wide.
     */
    while (nBox--)
    {
	w = pBox->x2 - pBox->x1;
	h = pBox->y2 - pBox->y1;
	modulus (pBox->x1 - xrot, tileWidth, srcx);
	modulus (pBox->y1 - yrot, tileHeight, srcy);
#if PSZ == 24
	xoffDst = (4 - pBox->x1) & 3;
	if (w == 1  &&  (xoffDst == 0  ||  xoffDst == 1))
#else
	xoffDst = pBox->x1 & PIM;
	if (xoffDst + w < PPW)
#endif
	{
	    maskpartialbits(pBox->x1, w, startmask);
	    endmask = 0;
	    nlwMiddle = 0;
	}
	else
	{
	    maskbits (pBox->x1, w, startmask, endmask, nlwMiddle)
	}
#if PSZ == 24
	pDstLine = pdstBase + (pBox->y1 * widthDst) + ((pBox->x1*3) >> 2);
#else
	pDstLine = pdstBase + (pBox->y1 * widthDst) + (pBox->x1 >> PWSH);
#endif
	pSrcLine = pSrcBase + (srcy * widthSrc);
#if PSZ == 24
	xoffSrc = (4 - srcx) & 3;
#else
	xoffSrc = srcx & PIM;
#endif
	if (xoffSrc >= xoffDst)
	{
	    xoffStart = xoffSrc - xoffDst;
	    needFirst = 1;
	}
	else
	{
#if PSZ == 24
	    xoffStart = 4 - (xoffDst - xoffSrc);
#else
	    xoffStart = PPW - (xoffDst - xoffSrc);
#endif
	    needFirst = 0;
	}
	leftShiftStart = (xoffStart) << LEFTSHIFT_AMT;
	rightShiftStart = PGSZ - leftShiftStart;
#if PSZ == 24
	nlwSrcStart = widthSrc - ((srcx*3) >> 2);
#else
	nlwSrcStart = widthSrc - (srcx >> PWSH);
#endif
	while (h--)
	{
	    /* XXX only works when narrowShift >= PPW/2 */
	    if (narrowTile)
	    {
		tmp = pSrcBase[srcy] & narrowMask; /* source width == 1 */
		narrow[0] = tmp | SCRRIGHT (tmp, narrowShift);
#if PSZ == 24
		narrow[1] = BitLeft (tmp, 8) |
			    BitRight(tmp, 16);
#else
		narrow[1] = SCRLEFT (tmp, PPW - narrowShift) |
			    SCRRIGHT(tmp, 2 * narrowShift - PPW);
#endif
		pSrcLine = narrow;
	    }
	    xoff = xoffStart;
	    leftShift = leftShiftStart;
	    rightShift = rightShiftStart;
	    nlwSrc = nlwSrcStart;
#if PSZ == 24
	    pSrc = pSrcLine + ((srcx * 3) >> 2);
#else
	    pSrc = pSrcLine + (srcx >> PWSH);
#endif
	    pDst = pDstLine;
	    bits = 0;
	    if (needFirst)
	    {
		NextTileBits
	    }
	    if (startmask)
	    {
		NextTileBits
		tmp = BitLeft(tmp, leftShift);
 		if (rightShift != PGSZ)
		    tmp |= BitRight(bits,rightShift);
		*pDst = MROP_MASK (tmp, *pDst, startmask);
		++pDst;
	    }
	    nlw = nlwMiddle;
	    while (nlw)
	    {
#if MROP == Mcopy
		if (nlwSrc > 1)
		{
		    int nlwPart = nlw;

		    if (nlwPart >= nlwSrc)
			nlwPart = nlwSrc - 1;
		    nlw -= nlwPart;
		    nlwSrc -= nlwPart;
		    if (rightShift != PGSZ)
		    {
			while (nlwPart--)
			{
			    tmp = bits;
			    bits = *pSrc++;
			    *pDst = MROP_SOLID(BitLeft(tmp, leftShift) |
					      BitRight (bits, rightShift),
					      *pDst);
			    ++pDst;
			}
		    }
		    else
		    {
			if (nlwPart)
			{
			    *pDst = MROP_SOLID (bits, *pDst);
			    ++pDst;
			    nlwPart--;
			    while (nlwPart--)
			    {
				*pDst = MROP_SOLID(*pSrc, *pDst);
				++pDst; ++pSrc;
			    }
			    bits = *pSrc++;
			}
		    }
		}
		else
#endif
		{
		    NextTileBits
		    if (rightShift != PGSZ)
		    {
			*pDst = MROP_SOLID(BitLeft(tmp, leftShift) |
					   BitRight(bits, rightShift),
					   *pDst);
		    }
		    else
		    {
			*pDst = MROP_SOLID (tmp, *pDst);
		    }
		    ++pDst;
		    nlw--;
		}
	    }
	    if (endmask)
	    {
		NextTileBits
		if (rightShift == PGSZ)
		    bits = 0;
		*pDst = MROP_MASK (BitLeft(tmp, leftShift) |
				   BitRight(bits,rightShift),
				   *pDst, endmask);
	    }
	    pDstLine += widthDst;
	    pSrcLine += widthSrc;
	    if (++srcy == tileHeight)
	    {
		srcy = 0;
		pSrcLine = pSrcBase;
	    }
	}
	pBox++;
    }
}

void
MROP_NAME(cfbFillSpanTileOdd) (pDrawable, n, ppt, pwidth, tile, xrot, yrot, alu, planemask)
    DrawablePtr	pDrawable;
    int		n;
    DDXPointPtr	ppt;
    int		*pwidth;
    PixmapPtr	tile;
    int		xrot, yrot;
    int		alu;
    unsigned long   planemask;
{
    int tileWidth;	/* width of tile in pixels */
    int tileHeight;	/* height of the tile */
    int widthSrc;

    int widthDst;		/* width in longwords of the dest pixmap */
    int w;		/* width of current span */
    CfbBits startmask;
    CfbBits endmask;	/* masks for reggedy bits at either end of line */
    int nlwSrc;		/* number of whole longwords in source */
    
    register int nlw;	/* loop version of nlwMiddle */
    int srcy;		/* current tile y position */
    int srcx;		/* current tile x position */
    int xoffDst, xoffSrc;
    int leftShift, rightShift;

#if MROP == 0 && PSZ == 24
    DeclareMergeRop24()
#else
    MROP_DECLARE_REG()
#endif

    CfbBits *pdstBase;	/* pointer to start of dest */
    CfbBits *pDstLine;	/* poitner to start of dest box */
    CfbBits *pSrcBase;	/* pointer to start of source */
    CfbBits *pSrcLine;	/* pointer to start of source line */
    register CfbBits *pDst;
    register CfbBits *pSrc;
    register CfbBits bits, tmp = 0;
    int xoffStart, xoff;
    int leftShiftStart, rightShiftStart, nlwSrcStart;
    CfbBits tileEndMask;
    int tileEndLeftShift, tileEndRightShift;
    int	xoffStep;
    int tileEndPart;
    int needFirst;
    CfbBits   narrow[2];
    CfbBits   narrowMask = 0;
    int	    narrowShift = 0;
    Bool    narrowTile;

#if MROP == 0 && PSZ == 24
    InitializeMergeRop24 (alu, planemask)
#else
    MROP_INITIALIZE (alu, planemask)
#endif

    tileHeight = tile->drawable.height;
    tileWidth = tile->drawable.width;
    widthSrc = tile->devKind / PGSZB;
    narrowTile = FALSE;
    if (widthSrc == 1)
    {
	narrowShift = tileWidth;
	narrowMask = cfbendpartial [tileWidth];
	tileWidth *= 2;
	widthSrc = 2;
	narrowTile = TRUE;
    }
    pSrcBase = (CfbBits *)tile->devPrivate.ptr;

    cfbGetLongWidthAndPointer (pDrawable, widthDst, pdstBase)

#if PSZ == 24
    tileEndPart = (4 - tileWidth) & 3;
    tileEndMask = cfbendpartial[tileWidth & 3];
#else
    tileEndPart = tileWidth & PIM;
    tileEndMask = cfbendpartial[tileEndPart];
#endif
    tileEndLeftShift = (tileEndPart) << LEFTSHIFT_AMT;
    tileEndRightShift = PGSZ - tileEndLeftShift;
#if PSZ == 24
    xoffStep = 4 - tileEndPart;
#else
    xoffStep = PPW - tileEndPart;
#endif
    while (n--)
    {
	w = *pwidth++;
	modulus (ppt->x - xrot, tileWidth, srcx);
	modulus (ppt->y - yrot, tileHeight, srcy);
#if PSZ == 24
	xoffDst = (4 - ppt->x) & 3;
	if (w == 1  &&  (xoffDst == 0  ||  xoffDst == 1))
#else
	xoffDst = ppt->x & PIM;
	if (xoffDst + w < PPW)
#endif
	{
	    maskpartialbits(ppt->x, w, startmask);
	    endmask = 0;
	    nlw = 0;
	}
	else
	{
	    maskbits (ppt->x, w, startmask, endmask, nlw)
	}
#if PSZ == 24
	pDstLine = pdstBase + (ppt->y * widthDst)  + ((ppt->x *3)>> 2);
#else
	pDstLine = pdstBase + (ppt->y * widthDst) + (ppt->x >> PWSH);
#endif
	pSrcLine = pSrcBase + (srcy * widthSrc);
#if PSZ == 24
	xoffSrc = (4 - srcx) & 3;
#else
	xoffSrc = srcx & PIM;
#endif
	if (xoffSrc >= xoffDst)
	{
	    xoffStart = xoffSrc - xoffDst;
	    needFirst = 1;
	}
	else
	{
#if PSZ == 24
	    xoffStart = 4 - (xoffDst - xoffSrc);
#else
	    xoffStart = PPW - (xoffDst - xoffSrc);
#endif
	    needFirst = 0;
	}
	leftShiftStart = (xoffStart) << LEFTSHIFT_AMT;
	rightShiftStart = PGSZ - leftShiftStart;
#if PSZ == 24
	nlwSrcStart = widthSrc - ((srcx*3) >> 2);
#else
	nlwSrcStart = widthSrc - (srcx >> PWSH);
#endif
	/* XXX only works when narrowShift >= PPW/2 */
	if (narrowTile)
	{
	    tmp = pSrcBase[srcy] & narrowMask;	/* source width == 1 */
	    narrow[0] = tmp | SCRRIGHT (tmp, narrowShift);
#if PSZ == 24
	    narrow[1] = BitLeft (tmp, 8) |
			BitRight(tmp, 16);
#else
	    narrow[1] = SCRLEFT (tmp, PPW - narrowShift) |
			SCRRIGHT(tmp, 2 * narrowShift - PPW);
#endif
	    pSrcLine = narrow;
	}
	xoff = xoffStart;
	leftShift = leftShiftStart;
	rightShift = rightShiftStart;
	nlwSrc = nlwSrcStart;
#if PSZ == 24
	pSrc = pSrcLine + ((srcx * 3) >> 2);
#else
	pSrc = pSrcLine + (srcx >> PWSH);
#endif
	pDst = pDstLine;
	bits = 0;
	if (needFirst)
	{
	    NextTileBits
	}
	if (startmask)
	{
	    NextTileBits
	    tmp = BitLeft(tmp, leftShift);
	    if (rightShift != PGSZ)
		tmp |= BitRight(bits,rightShift);
	    *pDst = MROP_MASK (tmp, *pDst, startmask);
	    ++pDst;
	}
	while (nlw)
	{
#if MROP == Mcopy
	    if (nlwSrc > 1)
	    {
		int nlwPart = nlw;

		if (nlwPart >= nlwSrc)
		    nlwPart = nlwSrc - 1;
		nlw -= nlwPart;
		nlwSrc -= nlwPart;
		if (rightShift != PGSZ)
		{
		    while (nlwPart--)
		    {
			tmp = bits;
			bits = *pSrc++;
			*pDst = MROP_SOLID(BitLeft(tmp, leftShift) |
					  BitRight (bits, rightShift),
					  *pDst);
			++pDst;
		    }
		}
		else
		{
		    if (nlwPart)
		    {
			*pDst = MROP_SOLID (bits, *pDst);
			++pDst;
			nlwPart--;
			while (nlwPart--)
			{
			    *pDst = MROP_SOLID(*pSrc, *pDst);
			    ++pDst; ++pSrc;
			}
			bits = *pSrc++;
		    }
		}
	    }
	    else
#endif
	    {
		NextTileBits
		if (rightShift != PGSZ)
		{
		    *pDst = MROP_SOLID(BitLeft(tmp, leftShift) |
				       BitRight(bits, rightShift),
				       *pDst);
		    ++pDst;
		}
		else
		{
		    *pDst = MROP_SOLID (tmp, *pDst);
		    ++pDst;
		}
		nlw--;
	    }
	}
	if (endmask)
	{
	    NextTileBits
	    if (rightShift == PGSZ)
		bits = 0;
	    *pDst = MROP_MASK (BitLeft(tmp, leftShift) |
			       BitRight(bits,rightShift),
			       *pDst, endmask);
	}
	ppt++;
    }
}

# include "fastblt.h"

#define IncSrcPtr   psrc++; if (!--srcRemaining) { srcRemaining = widthSrc; psrc = psrcStart; }

void
MROP_NAME(cfbFillBoxTile32s) (pDrawable, nBox, pBox, tile, xrot, yrot, alu, planemask)
    DrawablePtr	    pDrawable;
    int		    nBox;	/* number of boxes to fill */
    register BoxPtr pBox;	/* pointer to list of boxes to fill */
    PixmapPtr	    tile;	/* tile */
    int		    xrot, yrot;
    int		    alu;
    unsigned long   planemask;
{
    int	tileWidth;	/* width of tile */
    int tileHeight;	/* height of the tile */
    int	widthSrc;	/* width in longwords of the source tile */

    int widthDst;	/* width in longwords of the dest pixmap */
    int w;		/* width of current box */
    int h;		/* height of current box */
    CfbBits startmask;
    CfbBits endmask;/* masks for reggedy bits at either end of line */
    int nlMiddle;	/* number of longwords between sides of boxes */
    
    register int nl;	/* loop version of nlMiddle */
    int srcy;		/* current tile y position */
    int srcx;		/* current tile x position */
    int	srcRemaining;	/* number of longwords remaining in source */
    int xoffDst, xoffSrc;
    int	srcStart;	/* number of longwords source offset at left of box */
    int	leftShift, rightShift;

#if MROP == 0 && PSZ == 24
    DeclareMergeRop24()
#else
    MROP_DECLARE_REG()
#endif

    CfbBits	    *pdstBase;	/* pointer to start of dest */
    CfbBits	    *pdstLine;	/* poitner to start of dest box */
    CfbBits	    *psrcBase;	/* pointer to start of source */
    CfbBits	    *psrcLine;	/* pointer to fetch point of source */
    CfbBits	    *psrcStart;	/* pointer to start of source line */
    register CfbBits  *pdst;
    register CfbBits  *psrc;
    register CfbBits  bits, bits1;
    register int	    nlTemp;

#if MROP == 0 && PSZ == 24
    InitializeMergeRop24 (alu, planemask)
#else
    MROP_INITIALIZE (alu, planemask)
#endif

    psrcBase = (CfbBits *)tile->devPrivate.ptr;
    tileHeight = tile->drawable.height;
    tileWidth = tile->drawable.width;
#if PSZ == 24
    widthSrc = tile->devKind / PGSZB;
#else
    widthSrc = tileWidth >> PWSH;
#endif

    cfbGetLongWidthAndPointer (pDrawable, widthDst, pdstBase)

    while (nBox--)
    {
	w = pBox->x2 - pBox->x1;
	h = pBox->y2 - pBox->y1;

	/* set up source */
	modulus (pBox->x1 - xrot, tileWidth, srcx);
	modulus (pBox->y1 - yrot, tileHeight, srcy);
#if PSZ == 24
	xoffSrc = (4 - srcx) & 3;
	srcStart = (srcx * 3) >> 2;
#else
	xoffSrc = srcx & PIM;
	srcStart = (srcx >> PWSH);
#endif
	psrcStart = psrcBase + (srcy * widthSrc);
	psrcLine = psrcStart + srcStart;

	/* set up dest */
#if PSZ == 24
	xoffDst = (4 - pBox->x1) & 3;
	pdstLine = pdstBase + (pBox->y1 * widthDst) + ((pBox->x1*3) >> 2);
#else
	xoffDst = pBox->x1 & PIM;
	pdstLine = pdstBase + (pBox->y1 * widthDst) + (pBox->x1 >> PWSH);
#endif
	/* set up masks */
#if PSZ == 24
	if (w == 1  &&  (xoffDst == 0  ||  xoffDst == 1))
#else
	if (xoffDst + w < PPW)
#endif
	{
	    maskpartialbits(pBox->x1, w, startmask);
	    endmask = 0;
	    nlMiddle = 0;
	}
	else
	{
	    maskbits (pBox->x1, w, startmask, endmask, nlMiddle)
	}
	if (xoffSrc == xoffDst)
	{
	    while (h--)
	    {
		psrc = psrcLine;
		pdst = pdstLine;
		srcRemaining = widthSrc - srcStart;
		if (startmask)
		{
		    *pdst = MROP_MASK (*psrc, *pdst, startmask);
		    pdst++;
		    IncSrcPtr
		}
		nlTemp = nlMiddle;
		while (nlTemp)
		{
		    nl = nlTemp;
		    if (nl > srcRemaining)
			nl = srcRemaining;

		    nlTemp -= nl;
		    srcRemaining -= nl;

#if MROP == Mcopy
#ifdef LARGE_INSTRUCTION_CACHE
#ifdef FAST_CONSTANT_OFFSET_MODE

		    psrc += nl & (UNROLL-1);
		    pdst += nl & (UNROLL-1);

#define BodyOdd(n) pdst[-n] = MROP_SOLID (psrc[-n], pdst[-n]);
#define BodyEven(n) pdst[-n] = MROP_SOLID (psrc[-n], pdst[-n]);

#define LoopReset \
pdst += UNROLL; \
psrc += UNROLL;

#else

#define BodyOdd(n)  *pdst = MROP_SOLID (*psrc, *pdst); pdst++; psrc++;
#define BodyEven(n) BodyOdd(n)

#define LoopReset   ;

#endif
		    PackedLoop

#undef BodyOdd
#undef BodyEven
#undef LoopReset

#else
		    DuffL(nl, label1,
			    *pdst = MROP_SOLID (*psrc, *pdst);
			    pdst++; psrc++;)
#endif
#else
		    while (nl--) {
			    *pdst = MROP_SOLID (*psrc, *pdst);
			    pdst++; psrc++;
		    }
#endif
		    if (!srcRemaining)
		    {
			srcRemaining = widthSrc;
			psrc = psrcStart;
		    }
		}
		if (endmask)
		{
		    *pdst = MROP_MASK (*psrc, *pdst, endmask);
		}
		pdstLine += widthDst;
		psrcLine += widthSrc;
		psrcStart += widthSrc;
		if (++srcy == tileHeight)
		{
		    psrcStart = psrcBase;
		    psrcLine = psrcStart + srcStart;
		    srcy = 0;
		}
	    }
	}
	else
	{
	    if (xoffSrc > xoffDst)
	    {
		leftShift = (xoffSrc - xoffDst) << LEFTSHIFT_AMT;
		rightShift = PGSZ - leftShift;
	    }
	    else
	    {
		rightShift = (xoffDst - xoffSrc) << LEFTSHIFT_AMT;
		leftShift = PGSZ - rightShift;
	    }
	    while (h--)
	    {
		psrc = psrcLine;
		pdst = pdstLine;
		bits = 0;
		srcRemaining = widthSrc - srcStart;
		if (xoffSrc > xoffDst)
		{
		    bits = *psrc;
		    IncSrcPtr
		}
		if (startmask)
		{
		    bits1 = BitLeft(bits,leftShift);
		    bits = *psrc;
		    IncSrcPtr
		    bits1 |= BitRight(bits,rightShift);
		    *pdst = MROP_MASK(bits1, *pdst, startmask);
		    pdst++;
		}
		nlTemp = nlMiddle;
		while (nlTemp)
		{
		    nl = nlTemp;
		    if (nl > srcRemaining)
			nl = srcRemaining;

		    nlTemp -= nl;
		    srcRemaining -= nl;
    
#if MROP == Mcopy
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
#else
		    while (nl--) {
		    	bits1 = BitLeft(bits, leftShift);
		    	bits = *psrc++;
		    	*pdst = MROP_SOLID (bits1 | BitRight(bits, rightShift), *pdst);
		    	pdst++;
		    }
#endif
		    if (!srcRemaining)
		    {
			srcRemaining = widthSrc;
			psrc = psrcStart;
		    }
		}

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
		pdstLine += widthDst;
		psrcLine += widthSrc;
		psrcStart += widthSrc;
		if (++srcy == tileHeight)
		{
		    psrcStart = psrcBase;
		    psrcLine = psrcStart + srcStart;
		    srcy = 0;
		}
	    }
	}
	pBox++;
    }
}

void
MROP_NAME(cfbFillSpanTile32s) (pDrawable, n, ppt, pwidth, tile, xrot, yrot, alu, planemask)
    DrawablePtr	pDrawable;
    int		n;
    DDXPointPtr	ppt;
    int		*pwidth;
    PixmapPtr	tile;
    int		xrot, yrot;
    int		alu;
    unsigned long   planemask;
{
    int	tileWidth;	/* width of tile */
    int tileHeight;	/* height of the tile */
    int	widthSrc;	/* width in longwords of the source tile */

    int widthDst;	/* width in longwords of the dest pixmap */
    int w;		/* width of current box */
    CfbBits startmask;
    CfbBits endmask;/* masks for reggedy bits at either end of line */
    int nlMiddle;	/* number of longwords between sides of boxes */
    
    register int nl;	/* loop version of nlMiddle */
    int srcy;		/* current tile y position */
    int srcx;		/* current tile x position */
    int	srcRemaining;	/* number of longwords remaining in source */
    int xoffDst, xoffSrc;
    int	srcStart;	/* number of longwords source offset at left of box */
    int	leftShift, rightShift;

#if MROP == 0 && PSZ == 24
    DeclareMergeRop24()
#else
    MROP_DECLARE_REG()
#endif

    CfbBits	    *pdstBase;	/* pointer to start of dest */
    CfbBits	    *pdstLine;	/* poitner to start of dest box */
    CfbBits	    *psrcBase;	/* pointer to start of source */
    CfbBits	    *psrcLine;	/* pointer to fetch point of source */
    CfbBits	    *psrcStart;	/* pointer to start of source line */
    register CfbBits  *pdst;
    register CfbBits  *psrc;
    register CfbBits  bits, bits1;
    register int	    nlTemp;

#if MROP == 0 && PSZ == 24
    InitializeMergeRop24 (alu, planemask)
#else
    MROP_INITIALIZE (alu, planemask)
#endif

    psrcBase = (CfbBits *)tile->devPrivate.ptr;
    tileHeight = tile->drawable.height;
    tileWidth = tile->drawable.width;
#if PSZ == 24
    widthSrc = tile->devKind / PGSZB;
#else
    widthSrc = tileWidth >> PWSH;
#endif

    cfbGetLongWidthAndPointer (pDrawable, widthDst, pdstBase)

    while (n--)
    {
	w = *pwidth++;

	/* set up source */
	modulus (ppt->x - xrot, tileWidth, srcx);
	modulus (ppt->y - yrot, tileHeight, srcy);
#if PSZ == 24
	xoffSrc = (4 - srcx) & 3;
	srcStart = (srcx * 3) >> 2;
#else
	xoffSrc = srcx & PIM;
	srcStart = (srcx >> PWSH);
#endif
	psrcStart = psrcBase + (srcy * widthSrc);
	psrcLine = psrcStart + srcStart;

	/* set up dest */
#if PSZ == 24
	xoffDst = (4 - ppt->x) & 3;
	pdstLine = pdstBase + (ppt->y * widthDst) + ((ppt->x *3) >> 2);
	/* set up masks */
	if (w == 1  &&  (xoffDst == 0  ||  xoffDst == 1))
#else
	xoffDst = ppt->x & PIM;
	pdstLine = pdstBase + (ppt->y * widthDst) + (ppt->x >> PWSH);
	/* set up masks */
	if (xoffDst + w < PPW)
#endif
	{
	    maskpartialbits(ppt->x, w, startmask);
	    endmask = 0;
	    nlMiddle = 0;
	}
	else
	{
	    maskbits (ppt->x, w, startmask, endmask, nlMiddle)
	}

	if (xoffSrc == xoffDst)
	{
	    psrc = psrcLine;
	    pdst = pdstLine;
	    srcRemaining = widthSrc - srcStart;
	    if (startmask)
	    {
		*pdst = MROP_MASK (*psrc, *pdst, startmask);
		pdst++;
		IncSrcPtr
	    }
	    nlTemp = nlMiddle;
	    while (nlTemp)
	    {
		nl = nlTemp;
		if (nl > srcRemaining)
		    nl = srcRemaining;

		nlTemp -= nl;
		srcRemaining -= nl;

#if MROP == Mcopy
#ifdef LARGE_INSTRUCTION_CACHE
#ifdef FAST_CONSTANT_OFFSET_MODE

		psrc += nl & (UNROLL-1);
		pdst += nl & (UNROLL-1);

#define BodyOdd(n) pdst[-n] = MROP_SOLID (psrc[-n], pdst[-n]);
#define BodyEven(n) pdst[-n] = MROP_SOLID (psrc[-n], pdst[-n]);

#define LoopReset \
pdst += UNROLL; \
psrc += UNROLL;

#else

#define BodyOdd(n)  *pdst = MROP_SOLID (*psrc, *pdst); pdst++; psrc++;
#define BodyEven(n) BodyOdd(n)

#define LoopReset   ;

#endif
		PackedLoop

#undef BodyOdd
#undef BodyEven
#undef LoopReset

#else
		DuffL(nl, label1,
			*pdst = MROP_SOLID (*psrc, *pdst);
			pdst++; psrc++;)
#endif
#else
		while (nl--) {
			*pdst = MROP_SOLID (*psrc, *pdst);
			pdst++; psrc++;
		}
#endif
		if (!srcRemaining)
		{
		    srcRemaining = widthSrc;
		    psrc = psrcStart;
		}
	    }
	    if (endmask)
	    {
		*pdst = MROP_MASK (*psrc, *pdst, endmask);
	    }
	}
	else
	{
	    if (xoffSrc > xoffDst)
	    {
		leftShift = (xoffSrc - xoffDst) << LEFTSHIFT_AMT;
		rightShift = PGSZ - leftShift;
	    }
	    else
	    {
		rightShift = (xoffDst - xoffSrc) << LEFTSHIFT_AMT;
		leftShift = PGSZ - rightShift;
	    }
	    psrc = psrcLine;
	    pdst = pdstLine;
	    bits = 0;
	    srcRemaining = widthSrc - srcStart;
	    if (xoffSrc > xoffDst)
	    {
		bits = *psrc;
		IncSrcPtr
	    }
	    if (startmask)
	    {
		bits1 = BitLeft(bits,leftShift);
		bits = *psrc;
		IncSrcPtr
		bits1 |= BitRight(bits,rightShift);
		*pdst = MROP_MASK(bits1, *pdst, startmask);
		pdst++;
	    }
	    nlTemp = nlMiddle;
	    while (nlTemp)
	    {
		nl = nlTemp;
		if (nl > srcRemaining)
		    nl = srcRemaining;

		nlTemp -= nl;
		srcRemaining -= nl;

#if MROP == Mcopy
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
#else
		while (nl--) {
		    bits1 = BitLeft(bits,leftShift);
		    bits = *psrc++;
		    *pdst = MROP_SOLID(bits1|BitRight(bits,rightShift), *pdst);
		    pdst++;
		}
#endif
		if (!srcRemaining)
		{
		    srcRemaining = widthSrc;
		    psrc = psrcStart;
		}
	    }

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
	ppt++;
    }
}
