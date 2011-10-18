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

/* $XConsortium: miglblt.c,v 5.9 94/04/17 20:27:37 dpw Exp $ */

#include	"X.h"
#include	"Xmd.h"
#include	"Xproto.h"
#include	"misc.h"
#include	"fontstruct.h"
#include	"dixfontstr.h"
#include	"gcstruct.h"
#include	"windowstr.h"
#include	"scrnintstr.h"
#include	"pixmap.h"
#include	"servermd.h"

/*
    machine-independent glyph blt.
    assumes that glyph bits in snf are written in bytes,
have same bit order as the server's bitmap format,
and are byte padded.  this corresponds to the snf distributed
with the sample server.

    get a scratch GC.
    in the scratch GC set alu = GXcopy, fg = 1, bg = 0
    allocate a bitmap big enough to hold the largest glyph in the font
    validate the scratch gc with the bitmap
    for each glyph
	carefully put the bits of the glyph in a buffer,
	    padded to the server pixmap scanline padding rules
	fake a call to PutImage from the buffer into the bitmap
	use the bitmap in a call to PushPixels
*/

void
miPolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GC 		*pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    unsigned char *pglyphBase;	/* start of array of glyphs */
{
    int width, height;
    PixmapPtr pPixmap;
    int nbyLine;			/* bytes per line of padded pixmap */
    FontPtr pfont;
    GCPtr pGCtmp;
    register int i;
    register int j;
    unsigned char *pbits;		/* buffer for PutImage */
    register unsigned char *pb;		/* temp pointer into buffer */
    register CharInfoPtr pci;		/* currect char info */
    register unsigned char *pglyph;	/* pointer bits in glyph */
    int gWidth, gHeight;		/* width and height of glyph */
    register int nbyGlyphWidth;		/* bytes per scanline of glyph */
    int nbyPadGlyph;			/* server padded line of glyph */

    XID gcvals[3];

    if (pGC->miTranslate)
    {
	x += pDrawable->x;
	y += pDrawable->y;
    }

    pfont = pGC->font;
    width = FONTMAXBOUNDS(pfont,rightSideBearing) - 
	    FONTMINBOUNDS(pfont,leftSideBearing);
    height = FONTMAXBOUNDS(pfont,ascent) +
	     FONTMAXBOUNDS(pfont,descent);

    pPixmap = (*pDrawable->pScreen->CreatePixmap)(pDrawable->pScreen,
						  width, height, 1);
    if (!pPixmap)
	return;

    pGCtmp = GetScratchGC(1, pDrawable->pScreen);
    if (!pGCtmp)
    {
	(*pDrawable->pScreen->DestroyPixmap)(pPixmap);
	return;
    }

    gcvals[0] = GXcopy;
    gcvals[1] = 1;
    gcvals[2] = 0;

    DoChangeGC(pGCtmp, GCFunction|GCForeground|GCBackground, gcvals, 0);

    nbyLine = BitmapBytePad(width);
    pbits = (unsigned char *)ALLOCATE_LOCAL(height*nbyLine);
    if (!pbits)
    {
	(*pDrawable->pScreen->DestroyPixmap)(pPixmap);
	FreeScratchGC(pGCtmp);
        return;
    }
    while(nglyph--)
    {
	pci = *ppci++;
	pglyph = FONTGLYPHBITS(pglyphBase, pci);
	gWidth = GLYPHWIDTHPIXELS(pci);
	gHeight = GLYPHHEIGHTPIXELS(pci);
	if (gWidth && gHeight)
	{
	    nbyGlyphWidth = GLYPHWIDTHBYTESPADDED(pci);
	    nbyPadGlyph = BitmapBytePad(gWidth);

	    if (nbyGlyphWidth == nbyPadGlyph
#if GLYPHPADBYTES != 4
	        && (((int) pglyph) & 3) == 0
#endif
		)
	    {
		pb = pglyph;
	    }
	    else
	    {
		for (i=0, pb = pbits; i<gHeight; i++, pb = pbits+(i*nbyPadGlyph))
		    for (j = 0; j < nbyGlyphWidth; j++)
			*pb++ = *pglyph++;
		pb = pbits;
	    }

	    if ((pGCtmp->serialNumber) != (pPixmap->drawable.serialNumber))
		ValidateGC((DrawablePtr)pPixmap, pGCtmp);
	    (*pGCtmp->ops->PutImage)((DrawablePtr)pPixmap, pGCtmp,
				pPixmap->drawable.depth,
				0, 0, gWidth, gHeight, 
				0, XYBitmap, (char *)pb);

	    if ((pGC->serialNumber) != (pDrawable->serialNumber))
		ValidateGC(pDrawable, pGC);
	    (*pGC->ops->PushPixels)(pGC, pPixmap, pDrawable,
			       gWidth, gHeight,
			       x + pci->metrics.leftSideBearing,
			       y - pci->metrics.ascent);
	}
	x += pci->metrics.characterWidth;
    }
    (*pDrawable->pScreen->DestroyPixmap)(pPixmap);
    DEALLOCATE_LOCAL(pbits);
    FreeScratchGC(pGCtmp);
}


void
miImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GC 		*pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    unsigned char *pglyphBase;	/* start of array of glyphs */
{
    ExtentInfoRec info;		/* used by QueryGlyphExtents() */
    XID gcvals[3];
    int oldAlu, oldFS;
    unsigned long	oldFG;
    xRectangle backrect;

    QueryGlyphExtents(pGC->font, ppci, (unsigned long)nglyph, &info);

    if (info.overallWidth >= 0)
    {
    	backrect.x = x;
    	backrect.width = info.overallWidth;
    }
    else
    {
	backrect.x = x + info.overallWidth;
	backrect.width = -info.overallWidth;
    }
    backrect.y = y - FONTASCENT(pGC->font);
    backrect.height = FONTASCENT(pGC->font) + FONTDESCENT(pGC->font);

    oldAlu = pGC->alu;
    oldFG = pGC->fgPixel;
    oldFS = pGC->fillStyle;

    /* fill in the background */
    gcvals[0] = GXcopy;
    gcvals[1] = pGC->bgPixel;
    gcvals[2] = FillSolid;
    DoChangeGC(pGC, GCFunction|GCForeground|GCFillStyle, gcvals, 0);
    ValidateGC(pDrawable, pGC);
    (*pGC->ops->PolyFillRect)(pDrawable, pGC, 1, &backrect);

    /* put down the glyphs */
    gcvals[0] = oldFG;
    DoChangeGC(pGC, GCForeground, gcvals, 0);
    ValidateGC(pDrawable, pGC);
    (*pGC->ops->PolyGlyphBlt)(pDrawable, pGC, x, y, nglyph, ppci,
			      (char *)pglyphBase);

    /* put all the toys away when done playing */
    gcvals[0] = oldAlu;
    gcvals[1] = oldFG;
    gcvals[2] = oldFS;
    DoChangeGC(pGC, GCFunction|GCForeground|GCFillStyle, gcvals, 0);

}
