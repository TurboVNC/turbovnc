/* $XFree86: xc/programs/Xserver/mfb/mfbplygblt.c,v 3.4tsi Exp $ */
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
/* $Xorg: mfbplygblt.c,v 1.4 2001/02/09 02:05:19 xorgcvs Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xproto.h>
#include "mfb.h"
#include <X11/fonts/fontstruct.h>
#include "dixfontstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "regionstr.h"
#include "maskbits.h"
#include "miscstruct.h"

/*
    we should eventually special-case fixed-width fonts, although
its more important for ImageText, which is meant for terminal
emulators.

    this works for fonts with glyphs <= 32 bits wide.

    the clipping calculations are done for worst-case fonts.
we make no assumptions about the heights, widths, or bearings
of the glyphs.  if we knew that the glyphs are all the same height,
we could clip the tops and bottoms per clipping box, rather
than per character per clipping box.  if we knew that the glyphs'
left and right bearings were well-behaved, we could clip a single
character at the start, output until the last unclipped
character, and then clip the last one.  this is all straightforward
to determine based on max-bounds and min-bounds from the font.
    there is some inefficiency introduced in the per-character
clipping to make what's going on clearer.

    (it is possible, for example, for a font to be defined in which the
next-to-last character in a font would be clipped out, but the last
one wouldn't.  the code below deals with this.)

    PolyText looks at the fg color and the rasterop; mfbValidateGC
swaps in the right routine after looking at the reduced ratserop
in the private field of the GC.  

   the register allocations are provisional; in particualr startmask and
endmask might not be the right things.  pglyph, xoff, pdst, and tmpSrc
are fairly obvious, though.

   to avoid source proliferation, this file is compiled
three times:
	MFBPOLYGLYPHBLT		OPEQ
	mfbPolyGlyphBltWhite	|=
	mfbPolyGlyphBltBlack	&=~
	mfbPolyGlyphBltInvert	^=
*/

void
MFBPOLYGLYPHBLT(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    pointer	pglyphBase;	/* start of array of glyphs (unused in R5) */
{
    ExtentInfoRec info;	/* used by QueryGlyphExtents() */
    BoxRec bbox;		/* string's bounding box */

    CharInfoPtr pci;
    int xorg, yorg;	/* origin of drawable in bitmap */
    int widthDst;	/* width of dst in longwords */

			/* these keep track of the character origin */
    PixelType *pdstBase;
			/* points to longword with character origin */
    int xchar;		/* xorigin of char (mod 32) */

			/* these are used for placing the glyph */
    register int xoff;	/* x offset of left edge of glyph (mod 32) */
    register PixelType *pdst;
			/* pointer to current longword in dst */

    int w;		/* width of glyph in bits */
    int h;		/* height of glyph */
    int widthGlyph;	/* width of glyph, in bytes */
    register unsigned char *pglyph;
			/* pointer to current row of glyph */

			/* used for putting down glyph */
    register PixelType tmpSrc;
			/* for getting bits from glyph */
    register PixelType startmask;
    register PixelType endmask;
    register int nFirst;/* bits of glyph in current longword */

    if (!(pGC->planemask & 1))
	return;

    xorg = pDrawable->x;
    yorg = pDrawable->y;
    mfbGetPixelWidthAndPointer(pDrawable, widthDst, pdstBase);

    x += xorg;
    y += yorg;

    QueryGlyphExtents(pGC->font, ppci, (unsigned long)nglyph, &info);
    bbox.x1 = x + info.overallLeft;
    bbox.x2 = x + info.overallRight;
    bbox.y1 = y - info.overallAscent;
    bbox.y2 = y + info.overallDescent;

    switch (RECT_IN_REGION(pGC->pScreen, pGC->pCompositeClip, &bbox))
    {
      case rgnOUT:
	break;
      case rgnIN:
        pdstBase = mfbScanlineNoBankSwitch(pdstBase, x, y, widthDst);
        xchar = x & PIM;

        while(nglyph--)
        {
	    pci = *ppci;
	    pglyph = FONTGLYPHBITS(pglyphBase, pci);
	    w = pci->metrics.rightSideBearing - pci->metrics.leftSideBearing;
	    h = pci->metrics.ascent + pci->metrics.descent;
	    widthGlyph = GLYPHWIDTHBYTESPADDED(pci);

	    /* start at top scanline of glyph */
	    pdst = pdstBase;

	    /* find correct word in scanline and x offset within it
	       for left edge of glyph
	    */
	    xoff = xchar + pci->metrics.leftSideBearing;
	    if (xoff > PLST)
	    {
	        pdst++;
	        xoff &= PIM;
	    }
	    else if (xoff < 0)
	    {
	        xoff += PPW;
	        pdst--;
	    }

	    pdst = mfbScanlineDelta(pdst, -pci->metrics.ascent, widthDst);

	    if ((xoff + w) <= PPW)
	    {
	        /* glyph all in one longword */
	        maskpartialbits(xoff, w, startmask);
	        while (h--)
	        {
		    getleftbits(pglyph, w, tmpSrc);
		    *pdst OPEQ (SCRRIGHT(tmpSrc, xoff) & startmask);
		    pglyph += widthGlyph;
		    mfbScanlineInc(pdst, widthDst);
	        }
	    }
	    else
	    {
	        /* glyph crosses longword boundary */
	        maskPPWbits(xoff, w, startmask, endmask);
	        nFirst = PPW - xoff;
	        while (h--)
	        {
		    getleftbits(pglyph, w, tmpSrc);
		    *pdst OPEQ (SCRRIGHT(tmpSrc, xoff) & startmask);
		    *(pdst+1) OPEQ (SCRLEFT(tmpSrc, nFirst) & endmask);
		    pglyph += widthGlyph;
		    mfbScanlineInc(pdst, widthDst);
	        }
	    } /* glyph crosses longwords boundary */

	    /* update character origin */
	    x += pci->metrics.characterWidth;
	    xchar += pci->metrics.characterWidth;
	    if (xchar > PLST)
	    {
	        xchar -= PPW;
	        pdstBase++;
	    }
	    else if (xchar < 0)
	    {
	        xchar += PPW;
	        pdstBase--;
	    }
	    ppci++;
        } /* while nglyph-- */
	break;
      case rgnPART:
      {
	TEXTPOS *ppos;
	RegionPtr cclip;
	int nbox;
	BoxPtr pbox;
	int xpos;		/* x position of char origin */
	int i;
	BoxRec clip;
	int leftEdge, rightEdge;
	int topEdge, bottomEdge;
	int glyphRow;		/* first row of glyph not wholly
				   clipped out */
	int glyphCol;		/* leftmost visible column of glyph */
#if GETLEFTBITS_ALIGNMENT > 1
	int getWidth;		/* bits to get from glyph */
#endif

	if(!(ppos = (TEXTPOS *)ALLOCATE_LOCAL(nglyph * sizeof(TEXTPOS))))
	    return;

        pdstBase = mfbScanlineNoBankSwitch(pdstBase, x, y, widthDst);
        xpos = x;
	xchar = xpos & PIM;

	for (i=0; i<nglyph; i++)
	{
	    pci = ppci[i];

	    ppos[i].xpos = xpos;
	    ppos[i].xchar = xchar;
	    ppos[i].leftEdge = xpos + pci->metrics.leftSideBearing;
	    ppos[i].rightEdge = xpos + pci->metrics.rightSideBearing;
	    ppos[i].topEdge = y - pci->metrics.ascent;
	    ppos[i].bottomEdge = y + pci->metrics.descent;
	    ppos[i].pdstBase = pdstBase;
	    ppos[i].widthGlyph = GLYPHWIDTHBYTESPADDED(pci);

	    xpos += pci->metrics.characterWidth;
	    xchar += pci->metrics.characterWidth;
	    if (xchar > PLST)
	    {
		xchar &= PIM;
		pdstBase++;
	    }
	    else if (xchar < 0)
	    {
		xchar += PPW;
		pdstBase--;
	    }
	}

	cclip = pGC->pCompositeClip;
	pbox = REGION_RECTS(cclip);
	nbox = REGION_NUM_RECTS(cclip);

	for (; --nbox >= 0; pbox++)
	{
	    clip.x1 = max(bbox.x1, pbox->x1);
	    clip.y1 = max(bbox.y1, pbox->y1);
	    clip.x2 = min(bbox.x2, pbox->x2);
	    clip.y2 = min(bbox.y2, pbox->y2);
	    if ((clip.x2<=clip.x1) || (clip.y2<=clip.y1))
		continue;

	    for(i=0; i<nglyph; i++)
	    {
		pci = ppci[i];
		xchar = ppos[i].xchar;

		/* clip the left and right edges */
		if (ppos[i].leftEdge < clip.x1)
		    leftEdge = clip.x1;
		else
		    leftEdge = ppos[i].leftEdge;

		if (ppos[i].rightEdge > clip.x2)
		    rightEdge = clip.x2;
		else
		    rightEdge = ppos[i].rightEdge;

		w = rightEdge - leftEdge;
		if (w <= 0)
		    continue;

		/* clip the top and bottom edges */
		if (ppos[i].topEdge < clip.y1)
		    topEdge = clip.y1;
		else
		    topEdge = ppos[i].topEdge;

		if (ppos[i].bottomEdge > clip.y2)
		    bottomEdge = clip.y2;
		else
		    bottomEdge = ppos[i].bottomEdge;

		h = bottomEdge - topEdge;
		if (h <= 0)
		    continue;

		glyphRow = (topEdge - y) + pci->metrics.ascent;
		widthGlyph = ppos[i].widthGlyph;
		pglyph = FONTGLYPHBITS(pglyphBase, pci);
		pglyph += (glyphRow * widthGlyph);

		pdst = ppos[i].pdstBase;

		glyphCol = (leftEdge - ppos[i].xpos) -
			   (pci->metrics.leftSideBearing);
#if GETLEFTBITS_ALIGNMENT > 1
		getWidth = w + glyphCol;
#endif
		xoff = xchar + (leftEdge - ppos[i].xpos);
		if (xoff > PLST)
		{
		    xoff &= PIM;
		    pdst++;
		}
		else if (xoff < 0)
		{
		    xoff += PPW;
		    pdst--;
		}

		pdst = mfbScanlineDelta(pdst, -(y-topEdge), widthDst);

		if ((xoff + w) <= PPW)
		{
		    maskpartialbits(xoff, w, startmask);
		    while (h--)
		    {
			getshiftedleftbits(pglyph, glyphCol, getWidth, tmpSrc);
			*pdst OPEQ (SCRRIGHT(tmpSrc, xoff) & startmask);
			pglyph += widthGlyph;
			mfbScanlineInc(pdst, widthDst);
		    }
		}
		else
		{
		    maskPPWbits(xoff, w, startmask, endmask);
		    nFirst = PPW - xoff;
		    while (h--)
		    {
			getshiftedleftbits(pglyph, glyphCol, getWidth, tmpSrc);
			*pdst OPEQ (SCRRIGHT(tmpSrc, xoff) & startmask);
			*(pdst+1) OPEQ (SCRLEFT(tmpSrc, nFirst) & endmask);
			pglyph += widthGlyph;
			mfbScanlineInc(pdst, widthDst);
		    }
		}
	    } /* for each glyph */
	} /* while nbox-- */
	DEALLOCATE_LOCAL(ppos);
	break;
      }
      default:
	break;
    }
}
