/* $Xorg: spfuncs.c,v 1.4 2001/02/09 02:04:00 xorgcvs Exp $ */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices or Digital
 * not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES AND DIGITAL DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES OR DIGITAL BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Dave Lemke, Network Computing Devices, Inc
 */

/*

Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/lib/font/Speedo/spfuncs.c,v 1.8 2001/12/14 19:56:42 dawes Exp $ */

#ifndef FONTMODULE
#include <X11/Xos.h>
#endif
#include "fntfilst.h"
#include "fontutil.h"
#include "spint.h"

/* ARGSUSED */
static int 
SpeedoOpenScalable (
    FontPathElementPtr	fpe,
    FontPtr		*pFont,
    int			flags,
    FontEntryPtr	entry,
    char		*fileName,
    FontScalablePtr	vals,
    fsBitmapFormat	format,
    fsBitmapFormatMask	fmask,
    FontPtr		non_cachable_font)	/* We don't do licensing */
{
    char	fullName[MAXFONTNAMELEN];

    strcpy (fullName, entry->name.name);
    return SpeedoFontLoad (pFont, fullName, fileName, entry, vals,
			    format, fmask, flags);
}

/*
 * XXX
 *
 * this does a lot more then i'd like, but it has to get the bitmaps
 * in order to get accurate metrics (which it *must* have).
 *
 * a possible optimization is to avoid allocating the glyph memory
 * and to simply save the values without doing the work.
 */
static int
get_font_info(
    FontInfoPtr pinfo,
    char       *fontname,
    char       *filename,
    FontEntryPtr	entry,
    FontScalablePtr	vals,
    SpeedoFontPtr *spfont)
{
    SpeedoFontPtr spf;
    int         err;
    long	sWidth;

    err = sp_open_font(fontname, filename, entry, vals,
	       (fsBitmapFormat) 0, (fsBitmapFormatMask) 0, (unsigned long) 0,
		       &spf);

    if (err != Successful)
	return err;

    sp_fp_cur = spf;
    sp_reset_master(spf->master);

    sp_make_header(spf, pinfo);

    sp_compute_bounds(spf, pinfo, (unsigned long) 0, &sWidth);

    sp_compute_props(spf, fontname, pinfo, sWidth);

    /* compute remaining accelerators */
    FontComputeInfoAccelerators (pinfo);

    *spfont = spf;

    return Successful;
}

/* ARGSUSED */
static int
SpeedoGetInfoScaleable(
    FontPathElementPtr	fpe,
    FontInfoPtr		pFontInfo,
    FontEntryPtr	entry,
    FontNamePtr		fontName,
    char		*fileName,
    FontScalablePtr	vals)
{
    SpeedoFontPtr spf = NULL;
    char        fullName[MAXFONTNAMELEN];
    int         err;

    strcpy(fullName, entry->name.name);
    FontParseXLFDName(fullName, vals, FONT_XLFD_REPLACE_VALUE);

    err = get_font_info(pFontInfo, fullName, fileName, entry, vals, &spf);

    if (spf)
	sp_close_font(spf);

    return err;
}

static FontRendererRec renderer = {
    ".spd", 4, NULL, SpeedoOpenScalable,
	NULL, SpeedoGetInfoScaleable, 0
    , CAP_MATRIX | CAP_CHARSUBSETTING
};
    
void
SpeedoRegisterFontFileFunctions()
{
    sp_make_standard_props();
    sp_reset();
    FontFileRegisterRenderer(&renderer);
}
