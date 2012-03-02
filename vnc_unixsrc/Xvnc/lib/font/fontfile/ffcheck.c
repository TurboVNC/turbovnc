/* $Xorg: ffcheck.c,v 1.4 2001/02/09 02:04:03 xorgcvs Exp $ */

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
/* $XFree86: xc/lib/font/fontfile/ffcheck.c,v 1.16 2003/05/27 22:26:49 tsi Exp $ */

/*
 * Author:  Keith Packard, MIT X Consortium
 */
/* $NCDXorg: @(#)fontfile.c,v 1.6 1991/07/02 17:00:46 lemke Exp $ */

#include "fntfilst.h"
#include "bitmap.h"
#ifdef LOADABLEFONTS
#include "fontmod.h"
#endif

/*
 * Map FPE functions to renderer functions
 */


/* Here we must check the client to see if it has a context attached to
 * it that allows us to access the printer fonts
 */

static int
FontFileCheckOpenFont (pointer client, FontPathElementPtr fpe, Mask flags, 
		       char *name, int namelen, 
		       fsBitmapFormat format, fsBitmapFormatMask fmask,
		       XID id, FontPtr *pFont, char **aliasName, 
		       FontPtr non_cachable_font)
{
    if (XpClientIsBitmapClient(client))
	return (FontFileOpenFont  (client, fpe, flags, name, namelen, format, 
		fmask, id, pFont, aliasName, non_cachable_font));
    return BadFontName;
}

static int
FontFileCheckListFonts (pointer client, FontPathElementPtr fpe, 
			char *pat, int len, int max, FontNamesPtr names)
{
    if (XpClientIsBitmapClient(client))
	return FontFileListFonts (client, fpe, pat, len, max, names);
    return BadFontName;
}

static int
FontFileCheckStartListFontsWithInfo(pointer client, FontPathElementPtr fpe, 
				    char *pat, int len, int max, 
				    pointer *privatep)
{
    if (XpClientIsBitmapClient(client))
	return FontFileStartListFontsWithInfo(client, fpe, pat, len, 
				max, privatep);
    return BadFontName;
}

static int
FontFileCheckListNextFontWithInfo(pointer client, FontPathElementPtr fpe, 
				  char **namep, int *namelenp, 
				  FontInfoPtr *pFontInfo,
				  int *numFonts, pointer private)
{
    if (XpClientIsBitmapClient(client))
	return FontFileListNextFontWithInfo(client, fpe, namep, namelenp, 
				pFontInfo, numFonts, private);
    return BadFontName;
}

static int
FontFileCheckStartListFontsAndAliases(pointer client, FontPathElementPtr fpe, 
				      char *pat, int len, int max, 
				      pointer *privatep)
{
    if (XpClientIsBitmapClient(client))
	return FontFileStartListFontsAndAliases(client, fpe, pat, len, 
				max, privatep);
    return BadFontName;
}

static int
FontFileCheckListNextFontOrAlias(pointer client, FontPathElementPtr fpe, 
				 char **namep, int *namelenp, 
				 char **resolvedp, int *resolvedlenp, 
				 pointer private)
{
    if (XpClientIsBitmapClient(client))
	return FontFileListNextFontOrAlias(client, fpe, namep, namelenp, 
				resolvedp, resolvedlenp, private);
    return BadFontName;
}

void
FontFileCheckRegisterFpeFunctions (void)
{
#ifndef LOADABLEFONTS
    BitmapRegisterFontFileFunctions ();

#ifndef LOWMEMFTPT

#ifndef CRAY
#ifdef BUILD_SPEEDO
    SpeedoRegisterFontFileFunctions ();
#endif
#ifdef BUILD_TYPE1
    Type1RegisterFontFileFunctions();
#endif
#endif
#ifdef BUILD_CID
    CIDRegisterFontFileFunctions();
#endif
#ifdef BUILD_FREETYPE
    FreeTypeRegisterFontFileFunctions();
#endif
#ifdef BUILD_XTRUETYPE
    XTrueTypeRegisterFontFileFunctions();
#endif

#endif /* ifndef LOWMEMFTPT */

#else

    {
	int i;

	if (FontModuleList) {
	    for (i = 0; FontModuleList[i].name; i++) {
		if (FontModuleList[i].initFunc)
		    FontModuleList[i].initFunc();
	    }
	}
    }
#endif

    RegisterFPEFunctions(FontFileNameCheck,
			 FontFileInitFPE,
			 FontFileFreeFPE,
			 FontFileResetFPE,
			 FontFileCheckOpenFont,
			 FontFileCloseFont,
			 FontFileCheckListFonts,
			 FontFileCheckStartListFontsWithInfo,
			 FontFileCheckListNextFontWithInfo,
			 NULL,
			 NULL,
			 NULL,
			 FontFileCheckStartListFontsAndAliases,
			 FontFileCheckListNextFontOrAlias,
			 FontFileEmptyBitmapSource);
}
