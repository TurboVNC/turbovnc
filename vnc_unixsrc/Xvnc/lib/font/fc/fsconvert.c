/* $XConsortium: fsconvert.c,v 1.17 94/02/14 17:47:35 gildea Exp $ */
/*
 * Copyright 1990 Network Computing Devices
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Network Computing Devices not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  Network Computing Devices
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * NETWORK COMPUTING DEVICES DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL NETWORK COMPUTING DEVICES BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  	Dave Lemke, Network Computing Devices, Inc
 */
/*
 * FS data conversion
 */

#include        <X11/X.h>
#include	"FS.h"
#include	"FSproto.h"
#include	"fontmisc.h"
#include	"fontstruct.h"
#include	"fservestr.h"

extern char _fs_glyph_undefined;
extern char _fs_glyph_requested;
extern char _fs_glyph_zero_length;

extern int _fs_load_glyphs();

/*
 * converts data from font server form to X server form
 */

void
_fs_convert_char_info(src, dst)
    fsXCharInfo *src;
    xCharInfo *dst;
{
    dst->ascent = src->ascent;
    dst->descent = src->descent;
    dst->leftSideBearing = src->left;
    dst->rightSideBearing = src->right;
    dst->characterWidth = src->width;
    dst->attributes = src->attributes;
}

void
_fs_init_fontinfo(conn, pfi)
    FSFpePtr    conn;
    FontInfoPtr pfi;
{
    if (conn->fsMajorVersion == 1) {
	unsigned short n;
	n = pfi->firstCol;
	pfi->firstCol = pfi->firstRow;
	pfi->firstRow = n;
	n = pfi->lastCol;
	pfi->lastCol = pfi->lastRow;
	pfi->lastRow = n;
	pfi->defaultCh = (pfi->defaultCh >> 8) & 0xff
	                   + (pfi->defaultCh & 0xff) << 8;
    }

    if (FontCouldBeTerminal (pfi))
    {
	pfi->terminalFont = TRUE;
	pfi->minbounds.ascent = pfi->fontAscent;
	pfi->minbounds.descent = pfi->fontDescent;
	pfi->minbounds.leftSideBearing = 0;
	pfi->minbounds.rightSideBearing = pfi->minbounds.characterWidth;
	pfi->maxbounds = pfi->minbounds;
    }

    FontComputeInfoAccelerators (pfi);
}

int
_fs_convert_props(pi, po, pd, pfi)
    fsPropInfo *pi;
    fsPropOffset *po;
    pointer     pd;
    FontInfoPtr pfi;
{
    FontPropPtr dprop;
    int         i,
                nprops;
    char       *is_str;
    fsPropOffset local_off;
    char *off_adr;

/* stolen from server/include/resource.h */
#define BAD_RESOURCE 0xe0000000

    nprops = pfi->nprops = pi->num_offsets;

    dprop = (FontPropPtr) xalloc(sizeof(FontPropRec) * nprops);
    is_str = (char *) xalloc(sizeof(char) * nprops);
    if (!dprop || !is_str) {
	xfree(is_str);
	xfree(dprop);
	return -1;
    }
    pfi->props = dprop;
    pfi->isStringProp = is_str;

    off_adr = (char *)po;
    for (i = 0; i < nprops; i++, dprop++, is_str++) {
	memcpy(&local_off, off_adr, SIZEOF(fsPropOffset));
	dprop->name = MakeAtom(&pd[local_off.name.position],
			       local_off.name.length, 1);
	if (local_off.type != PropTypeString) {
	    *is_str = FALSE;
	    dprop->value = local_off.value.position;
	} else {
	    *is_str = TRUE;
	    dprop->value = (INT32) MakeAtom(&pd[local_off.value.position],
					    local_off.value.length, 1);
	    if (dprop->value == BAD_RESOURCE)
	    {
		xfree (pfi->props);
		xfree (pfi->isStringProp);
		pfi->props = 0;
		pfi->isStringProp = 0;
		return -1;
	    }
	}
	off_adr += SIZEOF(fsPropOffset);
    }

    return nprops;
}

int
_fs_convert_lfwi_reply(conn, pfi, fsrep, pi, po, pd)
    FSFpePtr    conn;
    FontInfoPtr pfi;
    fsListFontsWithXInfoReply *fsrep;
    fsPropInfo *pi;
    fsPropOffset *po;
    pointer     pd;
{
    fsUnpack_XFontInfoHeader(fsrep, pfi);
    _fs_init_fontinfo(conn, pfi);

    if (_fs_convert_props(pi, po, pd, pfi) == -1)
	return AllocError;

    return Successful;
}


#define ENCODING_UNDEFINED(enc) \
	((enc)->bits == &_fs_glyph_undefined ? \
	 TRUE : \
	 (access_done = access_done && (enc)->bits != &_fs_glyph_requested, \
	  FALSE))

#define GLYPH_UNDEFINED(loc) ENCODING_UNDEFINED(encoding + (loc))

/*
 * figures out what glyphs to request
 *
 * Includes logic to attempt to reduce number of round trips to the font
 * server:  when a glyph is requested, fs_build_range() requests a
 * 16-glyph range of glyphs that contains the requested glyph.  This is
 * predicated on the belief that using a glyph increases the chances
 * that nearby glyphs will be used: a good assumption for phonetic
 * alphabets, but a questionable one for ideographic/pictographic ones.
 */
/* ARGSUSED */
int
fs_build_range(pfont, range_flag, count, item_size, data, nranges, ranges)
    FontPtr     pfont;
    Bool	range_flag;
    register unsigned int count;
    int         item_size;
    register unsigned char *data;
    int	       *nranges;
    fsRange   **ranges;
{
    FSFontDataPtr fsd = (FSFontDataPtr) (pfont->fpePrivate);
    FSFontPtr fsfont = (FSFontPtr) (pfont->fontPrivate);
    register CharInfoPtr encoding = fsfont->encoding;
    FontInfoPtr pfi = &(pfont->info);
    fsRange	range;
    int		access_done = TRUE;
    int		err;
    register unsigned long firstrow, lastrow, firstcol, lastcol;
    register unsigned long row;
    register unsigned long col;
    register unsigned long loc;

    if (!fsd->glyphs_to_get)
	return AccessDone;

    firstrow = pfi->firstRow;
    lastrow = pfi->lastRow;
    firstcol = pfi->firstCol;
    lastcol = pfi->lastCol;

    /* Make sure we have default char */
    if (fsfont->pDefault && ENCODING_UNDEFINED(fsfont->pDefault))
    {
	loc = fsfont->pDefault - encoding;
	row = loc / (lastcol - firstcol + 1) + firstrow;
	col = loc % (lastcol - firstcol + 1) + firstcol;

	range.min_char_low = range.max_char_low = col;
	range.min_char_high = range.max_char_high = row;

	if ((err = add_range(&range, nranges, ranges, FALSE)) !=
	    Successful) return err;
	encoding[loc].bits = &_fs_glyph_requested;
	access_done = FALSE;
    }

    if (!range_flag && item_size == 1)
    {
	if (firstrow != 0) return AccessDone;
	while (count--)
	{
	    col = *data++;
	    if (col >= firstcol && col <= lastcol &&
		GLYPH_UNDEFINED(col - firstcol))
	    {
		int col1, col2;
		col1 = col & 0xf0;
		col2 = col1 + 15;
		if (col1 < firstcol) col1 = firstcol;
		if (col2 > lastcol) col2 = lastcol;
		/* Collect a 16-glyph neighborhood containing the requested
		   glyph... should in most cases reduce the number of round
		   trips to the font server. */
		for (col = col1; col <= col2; col++)
		{
		    if (!GLYPH_UNDEFINED(col - firstcol)) continue;
		    range.min_char_low = range.max_char_low = col;
		    range.min_char_high = range.max_char_high = 0;
		    if ((err = add_range(&range, nranges, ranges, FALSE)) !=
		        Successful) return err;
		    encoding[col - firstcol].bits = &_fs_glyph_requested;
		    access_done = FALSE;
		}
	    }
	}
    }
    else
    {
	fsRange fullrange[1];

	if (range_flag && count == 0)
	{
	    count = 2;
	    data = (unsigned char *)fullrange;
	    fullrange[0].min_char_high = firstrow;
	    fullrange[0].min_char_low = firstcol;
	    fullrange[0].max_char_high = lastrow;
	    fullrange[0].max_char_low = lastcol;
	}

	while (count--)
	{
	    int row1, col1, row2, col2;
	    row1 = row2 = *data++;
	    col1 = col2 = *data++;
	    if (range_flag)
	    {
		if (count)
		{
		    row2 = *data++;
		    col2 = *data++;
		    count--;
		}
		else
		{
		    row2 = lastrow;
		    col2 = lastcol;
		}
		if (row1 < firstrow) row1 = firstrow;
		if (row2 > lastrow) row2 = lastrow;
		if (col1 < firstcol) col1 = firstcol;
		if (col2 > lastcol) col2 = lastcol;
	    }
	    else
	    {
		if (row1 < firstrow || row1 > lastrow ||
		    col1 < firstcol || col1 > lastcol)
		    continue;
	    }
	    for (row = row1; row <= row2; row++)
	    {
	    expand_glyph_range: ;
		loc = (row - firstrow) * (lastcol + 1 - firstcol) +
		      (col1 - firstcol);
		for (col = col1; col <= col2; col++, loc++)
		{
		    if (GLYPH_UNDEFINED(loc))
		    {
			if (row1 == row2 &&
			    ((col1 & 0xf) && col1 > firstcol ||
			     (col2 & 0xf) != 0xf) && col2 < lastcol)
			{
			    /* If we're loading from a single row, expand
			       range of glyphs loaded to a multiple of
			       a 16-glyph range -- attempt to reduce number
			       of round trips to the font server. */
			    col1 &= 0xf0;
			    col2 = (col2 & 0xf0) + 15;
			    if (col1 < firstcol) col1 = firstcol;
			    if (col2 > lastcol) col2 = lastcol;
			    goto expand_glyph_range;
			}
			range.min_char_low = range.max_char_low = col;
			range.min_char_high = range.max_char_high = row;
			if ((err = add_range(&range, nranges, ranges, FALSE)) !=
			    Successful) return err;
			encoding[loc].bits = &_fs_glyph_requested;
			access_done = FALSE;
		    }
		}
	    }
	}
    }

    return access_done ?
	   AccessDone :
	   Successful;
}

#undef GLYPH_UNDEFINED
#undef ENCODING_UNDEFINED


/* _fs_clean_aborted_loadglyphs(): Undoes the changes to the encoding array
   performed by fs_build_range(); for use if the associated LoadGlyphs
   requests needs to be cancelled. */

_fs_clean_aborted_loadglyphs(pfont, num_expected_ranges, expected_ranges)
    FontPtr pfont;
    int num_expected_ranges;
    fsRange *expected_ranges;
{
    register FSFontPtr fsfont;
    register FSFontDataRec *fsd;
    register int i;

    fsfont = (FSFontPtr) pfont->fontPrivate;
    fsd = (FSFontDataRec *) pfont->fpePrivate;
    if (fsfont->encoding)
    {
	fsRange full_range[1];
	if (!num_expected_ranges)
	{
	    full_range[0].min_char_low = pfont->info.firstCol;
	    full_range[0].min_char_high = pfont->info.firstRow;
	    full_range[0].max_char_low = pfont->info.lastCol;
	    full_range[0].max_char_high = pfont->info.lastRow;
	    num_expected_ranges = 1;
	    expected_ranges = full_range;
	}

	for (i = 0; i < num_expected_ranges; i++)
	{
	    int row, col;
	    for (row = expected_ranges[i].min_char_high;
		 row <= expected_ranges[i].max_char_high;
		 row++)
	    {
		register CharInfoPtr encoding = fsfont->encoding +
		    ((row - pfont->info.firstRow) *
		     (pfont->info.lastCol -
		      pfont->info.firstCol + 1) +
		     expected_ranges[i].min_char_low -
		     pfont->info.firstCol);
		for (col = expected_ranges[i].min_char_low;
		     col <= expected_ranges[i].max_char_low;
		     encoding++, col++)
		{
		    if (encoding->bits == &_fs_glyph_requested)
			encoding->bits = &_fs_glyph_undefined;
		}
	    }
	}
    }
}


/*
 * figures out what extents to request
 * this is where lots of extra
 * smarts wants to live
 */
/* ARGSUSED */
int
_fs_check_extents(pfont, flags, nranges, range, blockrec)
    FontPtr     pfont;
    Mask        flags;
    int         nranges;
    fsRange    *range;
    FSBlockDataPtr blockrec;
{
/* XXX -- either fill in the requested info if we have it somewhere
 * and return AccessDone, or else return Successful
 */
    return Successful;
}

/*
 * figures out what glyphs to request
 * this is where lots of extra
 * smarts wants to live
 */
/* ARGSUSED */
int
_fs_check_bitmaps(pfont, format, flags, nranges, range, blockrec)
    FontPtr     pfont;
    fsBitmapFormat format;
    Mask        flags;
    int         nranges;
    fsRange    *range;
    FSBlockDataPtr blockrec;
{
/* XXX -- either fill in the requested info if we have it somewhere
 * and return AccessDone, or else return Successful
 */
    return Successful;
}

int
_fs_get_glyphs(pFont, count, chars, charEncoding, glyphCount, glyphs)
    FontPtr     pFont;
    unsigned long count;
    register unsigned char *chars;
    FontEncoding charEncoding;
    unsigned long *glyphCount;	/* RETURN */
    CharInfoPtr *glyphs;	/* RETURN */
{
    FSFontPtr   fsdata;
    unsigned int firstCol;
    register unsigned int numCols;
    unsigned int firstRow;
    unsigned int numRows;
    CharInfoPtr *glyphsBase;
    register unsigned int c;
    register CharInfoPtr pci;
    unsigned int r;
    CharInfoPtr encoding;
    CharInfoPtr pDefault;
    FSFontDataPtr fsd = (FSFontDataPtr) pFont->fpePrivate;
    int         itemSize;
    int         err = Successful;

    fsdata = (FSFontPtr) pFont->fontPrivate;
    encoding = fsdata->encoding;
    pDefault = fsdata->pDefault;
    firstCol = pFont->info.firstCol;
    numCols = pFont->info.lastCol - firstCol + 1;
    glyphsBase = glyphs;


    if (charEncoding == Linear8Bit || charEncoding == TwoD8Bit)
	itemSize = 1;
    else
	itemSize = 2;

    /* In this age of glyph caching, any glyphs gotten through this
       procedure should already be loaded.  If they are not, we are
       dealing with someone (perhaps a ddx driver optimizing a font)
       that doesn't understand the finer points of glyph caching.  The
       CHECK_ENCODING macro checks for this condition...  if found, it
       calls fs_load_all_glyphs(), which corrects it.  Since the caller
       of this code will not know how to handle a return value of
       Suspended, the fs_load_all_glyphs() procedure will block and
       freeze the server until the load operation is done.  Moral: the
       glyphCachingMode flag really must indicate the capabilities of
       the ddx drivers.  */

#define CHECK_ENCODING(cnum) \
    ( pci = encoding + (cnum), \
      fsd->glyphs_to_get ? \
      ( pci->bits == &_fs_glyph_undefined || pci->bits == &_fs_glyph_requested ? \
	((err = fs_load_all_glyphs(pFont)), pci) : \
	pci ) : \
      pci )

    switch (charEncoding) {

    case Linear8Bit:
    case TwoD8Bit:
	if (pFont->info.firstRow > 0)
	    break;
	if (pFont->info.allExist && pDefault) {
	    while (err == Successful && count--) {
		c = (*chars++) - firstCol;
		if (c < numCols)
		    *glyphs++ = CHECK_ENCODING(c);
		else
		    *glyphs++ = pDefault;
	    }
	} else {
	    while (err == Successful && count--) {
		c = (*chars++) - firstCol;
		if (c < numCols && CHECK_ENCODING(c)->bits)
		    *glyphs++ = pci;
		else if (pDefault)
		    *glyphs++ = pDefault;
	    }
	}
	break;
    case Linear16Bit:
	if (pFont->info.allExist && pDefault) {
	    while (err == Successful && count--) {
		c = *chars++ << 8;
		c = (c | *chars++) - firstCol;
		if (c < numCols)
		    *glyphs++ = CHECK_ENCODING(c);
		else
		    *glyphs++ = pDefault;
	    }
	} else {
	    while (err == Successful && count--) {
		c = *chars++ << 8;
		c = (c | *chars++) - firstCol;
		if (c < numCols && CHECK_ENCODING(c)->bits)
		    *glyphs++ = pci;
		else if (pDefault)
		    *glyphs++ = pDefault;
	    }
	}
	break;

    case TwoD16Bit:
	firstRow = pFont->info.firstRow;
	numRows = pFont->info.lastRow - firstRow + 1;
	while (err == Successful && count--) {
	    r = (*chars++) - firstRow;
	    c = (*chars++) - firstCol;
	    if (r < numRows && c < numCols &&
		    CHECK_ENCODING(r * numCols + c)->bits)
		*glyphs++ = pci;
	    else if (pDefault)
		*glyphs++ = pDefault;
	}
	break;
    }
    *glyphCount = glyphs - glyphsBase;
    return err;
}


static int
_fs_get_metrics(pFont, count, chars, charEncoding, glyphCount, glyphs)
    FontPtr     pFont;
    unsigned long count;
    register unsigned char *chars;
    FontEncoding charEncoding;
    unsigned long *glyphCount;	/* RETURN */
    xCharInfo **glyphs;		/* RETURN */
{
    FSFontPtr   fsdata;
    unsigned int firstCol;
    register unsigned int numCols;
    unsigned int firstRow;
    unsigned int numRows;
    xCharInfo **glyphsBase;
    register unsigned int c;
    unsigned int r;
    CharInfoPtr encoding;
    CharInfoPtr pDefault;
    FSFontDataPtr fsd = (FSFontDataPtr) pFont->fpePrivate;
    int         itemSize;
    int         err = Successful;

    fsdata = (FSFontPtr) pFont->fontPrivate;
    encoding = fsdata->inkMetrics;
    pDefault = fsdata->pDefault;
    firstCol = pFont->info.firstCol;
    numCols = pFont->info.lastCol - firstCol + 1;
    glyphsBase = glyphs;


    /* XXX - this should be much smarter */
    /* make sure the glyphs are there */
    if (charEncoding == Linear8Bit || charEncoding == TwoD8Bit)
	itemSize = 1;
    else
	itemSize = 2;

    switch (charEncoding) {

    case Linear8Bit:
    case TwoD8Bit:
	if (pFont->info.firstRow > 0)
	    break;
	if (pFont->info.allExist && pDefault) {
	    while (count--) {
		c = (*chars++) - firstCol;
		if (c < numCols)
		    *glyphs++ = (xCharInfo *)&encoding[c];
		else
		    *glyphs++ = (xCharInfo *)pDefault;
	    }
	} else {
	    while (count--) {
		c = (*chars++) - firstCol;
		if (c < numCols)
		    *glyphs++ = (xCharInfo *)(encoding + c);
		else if (pDefault)
		    *glyphs++ = (xCharInfo *)pDefault;
	    }
	}
	break;
    case Linear16Bit:
	if (pFont->info.allExist && pDefault) {
	    while (count--) {
		c = *chars++ << 8;
		c = (c | *chars++) - firstCol;
		if (c < numCols)
		    *glyphs++ = (xCharInfo *)(encoding + c);
		else
		    *glyphs++ = (xCharInfo *)pDefault;
	    }
	} else {
	    while (count--) {
		c = *chars++ << 8;
		c = (c | *chars++) - firstCol;
		if (c < numCols)
		    *glyphs++ = (xCharInfo *)(encoding + c);
		else if (pDefault)
		    *glyphs++ = (xCharInfo *)pDefault;
	    }
	}
	break;

    case TwoD16Bit:
	firstRow = pFont->info.firstRow;
	numRows = pFont->info.lastRow - firstRow + 1;
	while (count--) {
	    r = (*chars++) - firstRow;
	    c = (*chars++) - firstCol;
	    if (r < numRows && c < numCols)
		*glyphs++ = (xCharInfo *)(encoding + (r * numCols + c));
	    else if (pDefault)
		*glyphs++ = (xCharInfo *)pDefault;
	}
	break;
    }
    *glyphCount = glyphs - glyphsBase;
    return Successful;
}


void
_fs_unload_font(pfont)
    FontPtr     pfont;
{
    FSFontPtr   fsdata = (FSFontPtr) pfont->fontPrivate;

    if (fsdata->encoding)
    {
	register int i;
	register CharInfoPtr encoding = fsdata->encoding;
	FontInfoPtr pfi = &pfont->info;
	for (i = (pfi->lastCol - pfi->firstCol + 1) *
	         (pfi->lastRow - pfi->firstRow + 1);
	     i > 0;
	     encoding++, i--)
	{
	    if (encoding->bits &&
		encoding->bits != &_fs_glyph_undefined &&
		encoding->bits != &_fs_glyph_requested &&
		encoding->bits != &_fs_glyph_zero_length)
		xfree(encoding->bits);
	}
    }
    xfree(fsdata->encoding);
    xfree(fsdata);

    pfont->fontPrivate = 0;
}

void
_fs_init_font(pfont)
    FontPtr     pfont;
{
    /* set font function pointers */
    pfont->get_glyphs = _fs_get_glyphs;
    pfont->get_metrics = _fs_get_metrics;
    pfont->unload_font = _fs_unload_font;
    pfont->unload_glyphs = (void (*)())0;
}
