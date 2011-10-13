/* $XdotOrg: xc/lib/font/builtins/render.c,v 1.5 2005/07/30 18:56:32 alanc Exp $ */
/*
 * Id: render.c,v 1.2 1999/11/02 06:16:48 keithp Exp $
 *
 * Copyright 1999 SuSE, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of SuSE not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  SuSE makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * SuSE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL SuSE
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, SuSE, Inc.
 */
/* $XFree86: xc/lib/font/builtins/render.c,v 1.3 1999/12/30 02:29:51 robin Exp $ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include    <X11/fonts/fntfilst.h>
#include    "builtin.h"

BuiltinOpenBitmap (fpe, ppFont, flags, entry, fileName, format, fmask)
    FontPathElementPtr	fpe;
    FontPtr		*ppFont;
    int			flags;
    FontEntryPtr	entry;
    char		*fileName;
    fsBitmapFormat	format;
    fsBitmapFormatMask	fmask;
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

    file = BuiltinFileOpen (fileName);
    if (!file)
	return BadFontName;
    pFont = (FontPtr) xalloc(sizeof(FontRec));
    if (!pFont) {
	BuiltinFileClose (file);
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

    ret = pcfReadFont (pFont, file, bit, byte, glyph, scan);

    BuiltinFileClose (file);
    if (ret != Successful)
	xfree(pFont);
    else
	*ppFont = pFont;
    return ret;
}

BuiltinGetInfoBitmap (fpe, pFontInfo, entry, fileName)
    FontPathElementPtr	fpe;
    FontInfoPtr		pFontInfo;
    FontEntryPtr	entry;
    char		*fileName;
{
    FontFilePtr file;
    int		i;
    int		ret;
    FontRendererPtr renderer;

    file = BuiltinFileOpen (fileName);
    if (!file)
	return BadFontName;
    ret = pcfReadFontInfo (pFontInfo, file);
    BuiltinFileClose (file);
    return ret;
}

static FontRendererRec renderers[] = {
    ".builtin", 8,
    BuiltinOpenBitmap, 0, BuiltinGetInfoBitmap, 0, 0
};

#define numRenderers	(sizeof renderers / sizeof renderers[0])

void
BuiltinRegisterFontFileFunctions(void)
{
    int	i;
    for (i = 0; i < numRenderers; i++)
	FontFileRegisterRenderer ((FontRendererRec *) &renderers[i]);
}

