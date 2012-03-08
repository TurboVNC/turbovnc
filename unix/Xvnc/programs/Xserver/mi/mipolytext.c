/*******************************************************************

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

************************************************************************/
/* $XConsortium: mipolytext.c,v 5.4 94/04/17 20:27:45 keith Exp $ */
/*
 * mipolytext.c - text routines
 *
 * Author:	haynes
 * 		Digital Equipment Corporation
 * 		Western Software Laboratory
 * Date:	Thu Feb  5 1987
 */

#include	"X.h"
#include	"Xmd.h"
#include	"Xproto.h"
#include	"misc.h"
#include	"gcstruct.h"
#include	"fontstruct.h"
#include	"dixfontstr.h"

int
miPolyText(pDraw, pGC, x, y, count, chars, fontEncoding)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int		count;
    char 	*chars;
    FontEncoding fontEncoding;
{
    unsigned long n, i;
    int w;
    CharInfoPtr charinfo[255];	/* encoding only has 1 byte for count */

    GetGlyphs(pGC->font, (unsigned long)count, (unsigned char *)chars,
	      fontEncoding, &n, charinfo);
    w = 0;
    for (i=0; i < n; i++) w += charinfo[i]->metrics.characterWidth;
    if (n != 0)
        (*pGC->ops->PolyGlyphBlt)(
	    pDraw, pGC, x, y, n, charinfo, FONTGLYPHS(pGC->font));
    return x+w;
}


int
miPolyText8(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int 	count;
    char	*chars;
{
    unsigned long n, i;
    int w;
    CharInfoPtr charinfo[255];	/* encoding only has 1 byte for count */

    GetGlyphs(pGC->font, (unsigned long)count, (unsigned char *)chars,
	      Linear8Bit, &n, charinfo);
    w = 0;
    for (i=0; i < n; i++) w += charinfo[i]->metrics.characterWidth;
    if (n != 0)
        (*pGC->ops->PolyGlyphBlt)(
	    pDraw, pGC, x, y, n, charinfo, FONTGLYPHS(pGC->font));
    return x+w;
}


int
miPolyText16(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int		count;
    unsigned short *chars;
{
    unsigned long n, i;
    int w;
    CharInfoPtr charinfo[255];	/* encoding only has 1 byte for count */

    GetGlyphs(pGC->font, (unsigned long)count, (unsigned char *)chars,
	      (FONTLASTROW(pGC->font) == 0) ? Linear16Bit : TwoD16Bit,
	      &n, charinfo);
    w = 0;
    for (i=0; i < n; i++) w += charinfo[i]->metrics.characterWidth;
    if (n != 0)
        (*pGC->ops->PolyGlyphBlt)(
	    pDraw, pGC, x, y, n, charinfo, FONTGLYPHS(pGC->font));
    return x+w;
}


int
miImageText(pDraw, pGC, x, y, count, chars, fontEncoding)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int 	x, y;
    int 	count;
    char 	*chars;
    FontEncoding fontEncoding;
{
    unsigned long n, i;
    FontPtr font = pGC->font;
    int w;
    CharInfoPtr charinfo[255];

    GetGlyphs(font, (unsigned long)count, (unsigned char *)chars,
	      fontEncoding, &n, charinfo);
    w = 0;
    for (i=0; i < n; i++) w += charinfo[i]->metrics.characterWidth;
    if (n !=0 )
        (*pGC->ops->ImageGlyphBlt)(pDraw, pGC, x, y, n, charinfo, FONTGLYPHS(font));
    return x+w;
}


void
miImageText8(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int		count;
    char	*chars;
{
    unsigned long n;
    FontPtr font = pGC->font;
    CharInfoPtr charinfo[255];	/* encoding only has 1 byte for count */

    GetGlyphs(font, (unsigned long)count, (unsigned char *)chars,
	      Linear8Bit, &n, charinfo);
    if (n !=0 )
        (*pGC->ops->ImageGlyphBlt)(pDraw, pGC, x, y, n, charinfo, FONTGLYPHS(font));
}


void
miImageText16(pDraw, pGC, x, y, count, chars)
    DrawablePtr pDraw;
    GCPtr	pGC;
    int		x, y;
    int		count;
    unsigned short *chars;
{
    unsigned long n;
    FontPtr font = pGC->font;
    CharInfoPtr charinfo[255];	/* encoding only has 1 byte for count */

    GetGlyphs(font, (unsigned long)count, (unsigned char *)chars,
	      (FONTLASTROW(pGC->font) == 0) ? Linear16Bit : TwoD16Bit,
	      &n, charinfo);
    if (n !=0 )
        (*pGC->ops->ImageGlyphBlt)(pDraw, pGC, x, y, n, charinfo, FONTGLYPHS(font));
}
