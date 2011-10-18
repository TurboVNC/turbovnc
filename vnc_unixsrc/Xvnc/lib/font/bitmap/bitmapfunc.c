/* $XConsortium: bitmapfunc.c /main/10 1996/11/03 19:31:55 kaleb $ */
/* $XFree86: xc/lib/font/bitmap/bitmapfunc.c,v 3.4 1996/12/23 06:01:49 dawes Exp $ */

/*

Copyright (c) 1991  X Consortium

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

/*
 * Author:  Keith Packard, MIT X Consortium
 */

#include "fntfilst.h"
#include "bitmap.h"

typedef struct _BitmapFileFunctions {
    int         (*ReadFont) ( /* pFont, file, bit, byte, glyph, scan */ );
    int         (*ReadInfo) ( /* pFontInfo, file */ );
}           BitmapFileFunctionsRec, *BitmapFileFunctionsPtr;

extern int  pcfReadFont(), pcfReadFontInfo();
extern int  snfReadFont(), snfReadFontInfo();
extern int  bdfReadFont(), bdfReadFontInfo();
extern int  pmfReadFont();
int	    BitmapOpenBitmap ();
extern int  BitmapOpenScalable ();
int	    BitmapGetInfoBitmap ();
extern int  BitmapGetInfoScalable ();
int	    BitmapGetRenderIndex ();

/*
 * these two arrays must be in the same order
 */
static BitmapFileFunctionsRec readers[] = {
    pcfReadFont, pcfReadFontInfo,
    pcfReadFont, pcfReadFontInfo,
#ifdef X_GZIP_FONT_COMPRESSION
    pcfReadFont, pcfReadFontInfo,
#endif
#ifdef __EMX__
    pcfReadFont, pcfReadFontInfo,
#endif
    snfReadFont, snfReadFontInfo,
    snfReadFont, snfReadFontInfo,
#ifdef X_GZIP_FONT_COMPRESSION
    snfReadFont, snfReadFontInfo,
#endif
    bdfReadFont, bdfReadFontInfo,
    bdfReadFont, bdfReadFontInfo,
#ifdef X_GZIP_FONT_COMPRESSION
    bdfReadFont, bdfReadFontInfo,
#endif
    pmfReadFont, pcfReadFontInfo,
};


#define CAPABILITIES (CAP_MATRIX | CAP_CHARSUBSETTING)

static FontRendererRec	renderers[] = {
    ".pcf", 4,
    BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES,
    ".pcf.Z", 6,
    BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES,
#ifdef X_GZIP_FONT_COMPRESSION
    ".pcf.gz", 7,
    BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES,
#endif
#ifdef __EMX__
    ".pcz", 4,
    BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES,
#endif
    ".snf", 4,
    BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES,
    ".snf.Z", 6,
    BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES,
#ifdef X_GZIP_FONT_COMPRESSION
    ".snf.gz", 7,
    BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES,
#endif
    ".bdf", 4,
    BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES,
    ".bdf.Z", 6,
    BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES,
#ifdef X_GZIP_FONT_COMPRESSION
    ".bdf.gz", 7,
    BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES,
#endif
    ".pmf", 4,
      BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES 
};

BitmapOpenBitmap (fpe, ppFont, flags, entry, fileName, format, fmask,
		  non_cachable_font)
    FontPathElementPtr	fpe;
    FontPtr		*ppFont;
    int			flags;
    FontEntryPtr	entry;
    char		*fileName;
    fsBitmapFormat	format;
    fsBitmapFormatMask	fmask;
    FontPtr		non_cachable_font;	/* We don't do licensing */
{
    FontFilePtr	file;
    FontPtr     pFont;
    int         i;
    int         ret;
    int         bit,
                byte,
                glyph,
                scan,
		image;

    i = BitmapGetRenderIndex(entry->u.bitmap.renderer);
    file = FontFileOpen (fileName);
    if (!file)
	return BadFontName;
    pFont = (FontPtr) xalloc(sizeof(FontRec));
    if (!pFont) {
	FontFileClose (file);
	return AllocError;
    }
    /* set up default values */
    FontDefaultFormat(&bit, &byte, &glyph, &scan);
    /* get any changes made from above */
    ret = CheckFSFormat(format, fmask, &bit, &byte, &scan, &glyph, &image);

    /* Fill in font record. Data format filled in by reader. */
    pFont->refcnt = 0;
    pFont->maxPrivate = -1;
    pFont->devPrivates = (pointer *) 0;

    ret = (*readers[i].ReadFont) (pFont, file, bit, byte, glyph, scan);

    FontFileClose (file);
    if (ret != Successful)
	xfree(pFont);
    else
	*ppFont = pFont;
    return ret;
}

BitmapGetInfoBitmap (fpe, pFontInfo, entry, fileName)
    FontPathElementPtr	fpe;
    FontInfoPtr		pFontInfo;
    FontEntryPtr	entry;
    char		*fileName;
{
    FontFilePtr file;
    int		i;
    int		ret;
    FontRendererPtr renderer;

    renderer = FontFileMatchRenderer (fileName);
    if (!renderer)
	return BadFontName;
    i = BitmapGetRenderIndex(renderer);
    file = FontFileOpen (fileName);
    if (!file)
	return BadFontName;
    ret = (*readers[i].ReadInfo) (pFontInfo, file);
    FontFileClose (file);
    return ret;
}

#define numRenderers	(sizeof renderers / sizeof renderers[0])

BitmapRegisterFontFileFunctions ()
{
    int	    i;

    for (i = 0; i < numRenderers; i++)
	FontFileRegisterRenderer (&renderers[i]);
}

/*
 * compute offset into renderers array - used to find the font reader,
 * the font info reader, and the bitmap scaling routine.  All users
 * of this routine must be kept in step with the renderer array.
 */
BitmapGetRenderIndex(renderer)
    FontRendererPtr renderer;
{
    return renderer - renderers;
}
