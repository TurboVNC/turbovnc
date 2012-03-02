/* $Xorg: bitmapfunc.c,v 1.5 2001/02/09 02:04:02 xorgcvs Exp $ */

/*

Copyright 1991, 1998  The Open Group

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

/* $XFree86: xc/lib/font/bitmap/bitmapfunc.c,v 3.18 2002/09/24 20:48:52 tsi Exp $ */

/*
 * Author:  Keith Packard, MIT X Consortium
 */

#include "fntfilst.h"
#include "bitmap.h"
#include "fontutil.h"
#include "bdfint.h"
#include "pcf.h"
#include "snfstr.h"

typedef struct _BitmapFileFunctions {
    int         (*ReadFont) (FontPtr /* pFont */, FontFilePtr /* file */,  
			     int /* bit */, int /* byte */, 
			     int /* glyph */, int /* scan */);
    int         (*ReadInfo) (  FontInfoPtr /* pFontInfo */, 
			       FontFilePtr /* file */ );
}           BitmapFileFunctionsRec, *BitmapFileFunctionsPtr;


/*
 * the readers[] and renderers[] arrays must be in the same order,
 * and also in the same order as scale[] and find_scale[] in bitscale.c
 *
 */
static BitmapFileFunctionsRec readers[] = {
#ifdef PCFFORMAT
    { pcfReadFont, pcfReadFontInfo} ,
    { pcfReadFont, pcfReadFontInfo} ,
#ifdef X_GZIP_FONT_COMPRESSION
    { pcfReadFont, pcfReadFontInfo} ,
#endif
#endif
#ifdef SNFFORMAT
    { snfReadFont, snfReadFontInfo},
    { snfReadFont, snfReadFontInfo},
#ifdef X_GZIP_FONT_COMPRESSION
    { snfReadFont, snfReadFontInfo} ,
#endif
#endif
#ifdef BDFFORMAT
    { bdfReadFont, bdfReadFontInfo} ,
    { bdfReadFont, bdfReadFontInfo} ,
#ifdef X_GZIP_FONT_COMPRESSION
    { bdfReadFont, bdfReadFontInfo} ,
#endif
#endif
#ifdef PCFFORMAT
    { pmfReadFont, pcfReadFontInfo} ,
#endif
};


#define CAPABILITIES (CAP_MATRIX | CAP_CHARSUBSETTING)

static FontRendererRec	renderers[] = {
#ifdef	PCFFORMAT
    { ".pcf", 4, BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES },
    { ".pcf.Z", 6, BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES },
#ifdef X_GZIP_FONT_COMPRESSION
    { ".pcf.gz", 7,
    BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES },
#endif
#endif
#ifdef	SNFFORMAT
    { ".snf", 4, BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES },
    { ".snf.Z", 6, BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES },
#ifdef X_GZIP_FONT_COMPRESSION
    { ".snf.gz", 7, BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES },
#endif
#endif
#ifdef	BDFFORMAT
    { ".bdf", 4, BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES },
    { ".bdf.Z", 6, BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES },
#ifdef X_GZIP_FONT_COMPRESSION
    { ".bdf.gz", 7, BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES },
#endif
#endif
#ifdef	PCFFORMAT
    { ".pmf", 4, BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
	CAPABILITIES }
#endif
};

int
BitmapOpenBitmap (FontPathElementPtr fpe, FontPtr *ppFont, int flags, 
		  FontEntryPtr entry, char *fileName, 
		  fsBitmapFormat format, fsBitmapFormatMask fmask,
		  FontPtr non_cachable_font) /* We don't do licensing */
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
    if (!(pFont = CreateFontRec())) {
	fprintf(stderr, "Error: Couldn't allocate pFont (%ld)\n",
		(unsigned long)sizeof(FontRec));
	FontFileClose (file);
	return AllocError;
    }
    /* set up default values */
    FontDefaultFormat(&bit, &byte, &glyph, &scan);
    /* get any changes made from above */
    ret = CheckFSFormat(format, fmask, &bit, &byte, &scan, &glyph, &image);

    /* Fill in font record. Data format filled in by reader. */
    pFont->refcnt = 0;

    ret = (*readers[i].ReadFont) (pFont, file, bit, byte, glyph, scan);

    FontFileClose (file);
    if (ret != Successful) {
	xfree(pFont);
    } else {
	*ppFont = pFont;
    }
    return ret;
}

int
BitmapGetInfoBitmap (FontPathElementPtr fpe, FontInfoPtr pFontInfo, 
		     FontEntryPtr entry, char *fileName)
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

void
BitmapRegisterFontFileFunctions (void)
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
int
BitmapGetRenderIndex(FontRendererPtr renderer)
{
    return renderer - renderers;
}
