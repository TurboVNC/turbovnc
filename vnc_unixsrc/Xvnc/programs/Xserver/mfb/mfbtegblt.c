/* $XConsortium: mfbtegblt.c,v 5.14 94/04/17 20:28:35 dpw Exp $ */
/* Combined Purdue/PurduePlus patches, level 2.0, 1/17/89 */
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
#include	"X.h"
#include	"Xmd.h"
#include	"Xproto.h"
#include	"mfb.h"
#include	"fontstruct.h"
#include	"dixfontstr.h"
#include	"gcstruct.h"
#include	"windowstr.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"maskbits.h"

/*
    this works for fonts with glyphs <= PPW bits wide.

    This should be called only with a terminal-emulator font;
this means that the FIXED_METRICS flag is set, and that
glyphbounds == charbounds.

    in theory, this goes faster; even if it doesn't, it reduces the
flicker caused by writing a string over itself with image text (since
the background gets repainted per character instead of per string.)
this seems to be important for some converted X10 applications.

    Image text looks at the bits in the glyph and the fg and bg in the
GC.  it paints a rectangle, as defined in the protocol dcoument,
and the paints the characters.

   to avoid source proliferation, this file is compiled
two times:
	MFBTEGLYPHBLT		OP
	mfbTEGlyphBltWhite		(white text, black bg )
	mfbTEGlyphBltBlack	~	(black text, white bg )

*/

#if defined(NO_3_60_CG4) && defined(FASTPUTBITS) && defined(FASTGETBITS)
#define FASTCHARS
#endif

/*
 * this macro "knows" that only characters <= 8 bits wide will
 * fit this case (which is why it is independent of GLYPHPADBYTES)
 */

#if (BITMAP_BIT_ORDER == MSBFirst) && (GLYPHPADBYTES != 4)
#if GLYPHPADBYTES == 1
#define ShiftAmnt   24
#else
#define ShiftAmnt   16
#endif

/*
 * Note: for BITMAP_BIT_ORDER != IMAGE_BYTE_ORDER, SCRRIGHT() evaluates its
 * first argument more than once.  Thus the imbedded char++ have to be moved.
 * (DHD)
 */
#if BITMAP_BIT_ORDER == IMAGE_BYTE_ORDER
#if PPW == 32
#define GetBits4    c = (*char1++ << ShiftAmnt) | \
			SCRRIGHT (*char2++ << ShiftAmnt, xoff2) | \
			SCRRIGHT (*char3++ << ShiftAmnt, xoff3) | \
			SCRRIGHT (*char4++ << ShiftAmnt, xoff4);
#else /* PPW */
#define GetBits4    c = ((unsigned long)(*char1++ << ShiftAmnt) << 32 )  | \
			(SCRRIGHT (*char2++ << ShiftAmnt, xoff2) << 32 ) | \
			(SCRRIGHT (*char3++ << ShiftAmnt, xoff3) << 32 ) | \
			(SCRRIGHT (*char4++ << ShiftAmnt, xoff4) << 32 ) | \
			(*char5++ << ShiftAmnt) 			 | \
			SCRRIGHT (*char6++ << ShiftAmnt, xoff6) 	 | \
			SCRRIGHT (*char7++ << ShiftAmnt, xoff7) 	 | \
			SCRRIGHT (*char8++ << ShiftAmnt, xoff8);
#endif /* PPW */
#else /* BITMAP_BIT_ORDER != IMAGE_BYTE_ORDER */
#if PPW == 32
#define GetBits4    c = (*char1++ << ShiftAmnt) | \
			SCRRIGHT (*char2 << ShiftAmnt, xoff2) | \
			SCRRIGHT (*char3 << ShiftAmnt, xoff3) | \
			SCRRIGHT (*char4 << ShiftAmnt, xoff4); \
			char2++; char3++; char4++;
#else /* PPW == 64 */
#define GetBits4    c = ((unsigned long)(*char1++ << ShiftAmnt) << 32 )  | \
			(SCRRIGHT (*char2 << ShiftAmnt, xoff2) << 32 ) | \
			(SCRRIGHT (*char3 << ShiftAmnt, xoff3) << 32 ) | \
			(SCRRIGHT (*char4 << ShiftAmnt, xoff4) << 32 ) | \
			(*char5++ << ShiftAmnt) 			 | \
			SCRRIGHT (*char6 << ShiftAmnt, xoff6) 	 | \
			SCRRIGHT (*char7 << ShiftAmnt, xoff7) 	 | \
			SCRRIGHT (*char8 << ShiftAmnt, xoff8); \
			char2++; char3++; char4++; char6++; char7++; char8++;
#endif /* PPW */
#endif /* BITMAP_BIT_ORDER == IMAGE_BYTE_ORDER */

#else /* (BITMAP_BIT_ORDER != MSBFirst) || (GLYPHPADBYTES == 4) */

#if BITMAP_BIT_ORDER == IMAGE_BYTE_ORDER
#if PPW == 32
#define GetBits4    c = *char1++ | \
			SCRRIGHT (*char2++, xoff2) | \
			SCRRIGHT (*char3++, xoff3) | \
			SCRRIGHT (*char4++, xoff4);
#else /* PPW == 64 */
#define GetBits4    c = (unsigned long)(((*char1++) << 64 ) | \
                        (SCRRIGHT (*char2++, xoff2) << 64 ) | \
                        (SCRRIGHT (*char3++, xoff3) << 64 ) | \
                        (SCRRIGHT (*char4++, xoff4) << 64 ) | \
                        SCRRIGHT (*char5++, xoff5)          | \
                        SCRRIGHT (*char6++, xoff6)          | \
                        SCRRIGHT (*char7++, xoff7)          | \
                        SCRRIGHT (*char8++, xoff8));
#endif /* PPW */
#else /* BITMAP_BIT_ORDER != IMAGE_BYTE_ORDER */
#if PPW == 32
#define GetBits4    c = *char1++ | \
			SCRRIGHT (*char2, xoff2) | \
			SCRRIGHT (*char3, xoff3) | \
			SCRRIGHT (*char4, xoff4); \
			char2++; char3++; char4++;
#else /* PPW == 64 */
#define GetBits4    c = (unsigned long)(((*char1++) << 64 ) | \
                        (SCRRIGHT (*char2, xoff2) << 64 ) | \
                        (SCRRIGHT (*char3, xoff3) << 64 ) | \
                        (SCRRIGHT (*char4, xoff4) << 64 ) | \
                        SCRRIGHT (*char5, xoff5)          | \
                        SCRRIGHT (*char6, xoff6)          | \
                        SCRRIGHT (*char7, xoff7)          | \
                        SCRRIGHT (*char8, xoff8)); \
			char2++; char3++; char4++; \
			char5++; char6++; char7++; char8++;
#endif /* PPW */
#endif /* BITMAP_BIT_ORDER == IMAGE_BYTE_ORDER */

#endif /* BITMAP_BIT_ORDER && GLYPHPADBYTES */


#if GLYPHPADBYTES == 1
typedef	unsigned char	*glyphPointer;
#define USE_LEFTBITS
#endif

#if GLYPHPADBYTES == 2
typedef unsigned short	*glyphPointer;
#define USE_LEFTBITS
#endif

#if GLYPHPADBYTES == 4
typedef unsigned int	*glyphPointer;
#endif

#ifdef USE_LEFTBITS
#define GetBits1    getleftbits (char1, widthGlyph, c); \
		    c &= glyphMask; \
		    char1 = (glyphPointer) (((char *) char1) + glyphBytes);
#else
#define GetBits1    c = *char1++;
#endif

void
MFBTEGLYPHBLT(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GC 		*pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    pointer	pglyphBase;	/* start of array of glyphs */
{
    FontPtr	pfont = pGC->font;
    int widthDst;
    PixelType *pdstBase;	/* pointer to longword with top row 
				   of current glyph */

    int h;			/* height of glyph and char */
    register int xpos;		/* current x  */
    int ypos;			/* current y */
    int widthGlyph;

    int hTmp;			/* counter for height */
    register PixelType startmask, endmask;
    int nfirst;			/* used if glyphs spans a longword boundary */
    BoxRec bbox;		/* for clipping */
    int	widthGlyphs;
    register PixelType  *dst;
    register PixelType  c;
    register int	    xoff1, xoff2, xoff3, xoff4;
    register glyphPointer   char1, char2, char3, char4;
#if PPW == 64
    register int	    xoff5, xoff6, xoff7, xoff8;
    register glyphPointer   char5, char6, char7, char8;
#endif /* PPW */

#ifdef USE_LEFTBITS
    register PixelType  glyphMask;
    register PixelType  tmpSrc;
    register int	    glyphBytes;
#endif

    if (!(pGC->planemask & 1))
	return;

    mfbGetPixelWidthAndPointer(pDrawable, widthDst, pdstBase);

    xpos = x + pDrawable->x;
    ypos = y + pDrawable->y;

    widthGlyph = FONTMAXBOUNDS(pfont,characterWidth);
    h = FONTASCENT(pfont) + FONTDESCENT(pfont);

    xpos += FONTMAXBOUNDS(pfont,leftSideBearing);
    ypos -= FONTASCENT(pfont);

    bbox.x1 = xpos;
    bbox.x2 = xpos + (widthGlyph * nglyph);
    bbox.y1 = ypos;
    bbox.y2 = ypos + h;

    switch (RECT_IN_REGION(pGC->pScreen, 
           ((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip, &bbox))
    {
      case rgnPART:
	/* this is the WRONG thing to do, but it works.
	   calling the non-terminal text is easy, but slow, given
	   what we know about the font.

	   the right thing to do is something like:
	    for each clip rectangle
		compute at which row the glyph starts to be in it,
		   and at which row the glyph ceases to be in it
		compute which is the first glyph inside the left
		    edge, and the last one inside the right edge
		draw a fractional first glyph, using only
		    the rows we know are in
		draw all the whole glyphs, using the appropriate rows
		draw any pieces of the last glyph, using the right rows

	   this way, the code would take advantage of knowing that
	   all glyphs are the same height and don't overlap.

	   one day...
	*/
	CLIPTETEXT(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
      case rgnOUT:
	return;
    }
    pdstBase = mfbScanlineDeltaNoBankSwitch(pdstBase, ypos, widthDst);
    widthGlyphs = widthGlyph * PGSZB;

#ifdef USE_LEFTBITS
    glyphMask = endtab[widthGlyph];
    glyphBytes = GLYPHWIDTHBYTESPADDED(*ppci);
#endif

    if (nglyph >= PGSZB && widthGlyphs <= PPW)
    {
	while (nglyph >= PGSZB)
	{
	    nglyph -= PGSZB;
	    xoff1 = xpos & PIM;
	    xoff2 = widthGlyph;
	    xoff3 = xoff2 + widthGlyph;
	    xoff4 = xoff3 + widthGlyph;
#if PPW == 64
	    xoff5 = xoff4 + widthGlyph;
	    xoff6 = xoff5 + widthGlyph;
	    xoff7 = xoff6 + widthGlyph;
	    xoff8 = xoff7 + widthGlyph;
#endif /* PPW */
	    char1 = (glyphPointer) FONTGLYPHBITS(pglyphBase,(*ppci++));
	    char2 = (glyphPointer) FONTGLYPHBITS(pglyphBase,(*ppci++));
	    char3 = (glyphPointer) FONTGLYPHBITS(pglyphBase,(*ppci++));
	    char4 = (glyphPointer) FONTGLYPHBITS(pglyphBase,(*ppci++));
#if PPW == 64
	    char5 = (glyphPointer) FONTGLYPHBITS(pglyphBase,(*ppci++));
	    char6 = (glyphPointer) FONTGLYPHBITS(pglyphBase,(*ppci++));
	    char7 = (glyphPointer) FONTGLYPHBITS(pglyphBase,(*ppci++));
	    char8 = (glyphPointer) FONTGLYPHBITS(pglyphBase,(*ppci++));
#endif /* PPW */

	    hTmp = h;
	    dst = mfbScanlineOffset(pdstBase, (xpos >> PWSH)); /* switch now */

#ifndef FASTCHARS
	    if (xoff1 + widthGlyphs <= PPW)
	    {
		maskpartialbits (xoff1, widthGlyphs, startmask);
#endif
		while (hTmp--)
		{
		    GetBits4
#ifdef FASTCHARS
# if BITMAP_BIT_ORDER == MSBFirst
		    c >>= PPW - widthGlyphs;
# endif
		    FASTPUTBITS(OP(c), xoff1, widthGlyphs, dst);
#else
		    *(dst) = (*dst) & ~startmask | OP(SCRRIGHT(c, xoff1)) & startmask;
#endif
		    mfbScanlineInc(dst, widthDst);
		}
#ifndef FASTCHARS
	    }
	    else
	    {
		maskPPWbits (xoff1, widthGlyphs, startmask, endmask);
		nfirst = PPW - xoff1;
		while (hTmp--)
		{
		    GetBits4
		    dst[0] = dst[0] & ~startmask |
			     OP(SCRRIGHT(c,xoff1)) & startmask;
		    dst[1] = dst[1] & ~endmask |
			     OP(SCRLEFT(c,nfirst)) & endmask;
		    mfbScanlineInc(dst, widthDst);
		}
	    }
#endif
	    xpos += widthGlyphs;
	}
    }

    while(nglyph--)
    {
	xoff1 = xpos & PIM;
	char1 = (glyphPointer) FONTGLYPHBITS(pglyphBase,(*ppci++));
	hTmp = h;
	dst = mfbScanlineOffset(pdstBase, (xpos >> PWSH));

#ifndef FASTCHARS
	if (xoff1 + widthGlyph <= PPW)
	{
	    maskpartialbits (xoff1, widthGlyph, startmask);
#endif
	    while (hTmp--)
	    {
#ifdef FASTCHARS
#ifdef USE_LEFTBITS
		FASTGETBITS (char1,0,widthGlyph,c);
		char1 = (glyphPointer) (((char *) char1) + glyphBytes);
#else
		c = *char1++;
#if BITMAP_BIT_ORDER == MSBFirst
		c >>= PPW - widthGlyph;
#endif
#endif
		FASTPUTBITS (OP(c),xoff1,widthGlyph,dst);
#else
		GetBits1
		(*dst) = (*dst) & ~startmask | OP(SCRRIGHT(c, xoff1)) & startmask;
#endif
		mfbScanlineInc(dst, widthDst);
	    }
#ifndef FASTCHARS
	}
	else
	{
	    maskPPWbits (xoff1, widthGlyph, startmask, endmask);
	    nfirst = PPW - xoff1;
	    while (hTmp--)
	    {
		GetBits1
		dst[0] = dst[0] & ~startmask |
			 OP(SCRRIGHT(c,xoff1)) & startmask;
		dst[1] = dst[1] & ~endmask |
			 OP(SCRLEFT(c,nfirst)) & endmask;
		mfbScanlineInc(dst, widthDst);
	    }
	}
#endif
	xpos += widthGlyph;
    }
}
