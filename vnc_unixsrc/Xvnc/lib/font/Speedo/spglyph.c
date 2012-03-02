/* $Xorg: spglyph.c,v 1.4 2001/02/09 02:04:00 xorgcvs Exp $ */
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
 * Author: Dave Lemke, Network Computing Devices Inc
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
/* $XFree86: xc/lib/font/Speedo/spglyph.c,v 1.7 2001/12/14 19:56:42 dawes Exp $ */

#include	<X11/X.h>	/* for bit order #defines */
#include	"spint.h"
#include	"fontutil.h"

#undef	CLIP_BBOX_NOISE

static CurrentFontValuesRec current_font_values;
static CurrentFontValuesPtr cfv = &current_font_values;
static int  bit_order,
            byte_order,
            scan;

unsigned long
sp_compute_data_size(
    FontPtr     pfont,
    int         mappad,
    int         scanlinepad,
    unsigned long start,
    unsigned long end)
{
    unsigned long ch;
    unsigned long size = 0;
    int         bpr;
    SpeedoFontPtr spf = (SpeedoFontPtr) pfont->fontPrivate;
    FontInfoPtr pinfo = &pfont->info;
    int         firstChar;

    firstChar = spf->master->first_char_id;

    /* allocate the space */
    switch (mappad) {
	int         charsize;
	CharInfoPtr ci;
	xCharInfo  *cim;

    case BitmapFormatImageRectMin:
	cfv->bpr = 0;
	for (ch = start; ch <= end; ch++) {
	    ci = &spf->encoding[ch - firstChar];
	    if (!ci)
		ci = spf->pDefault;
	    cim = &ci->metrics;
	    charsize = GLYPH_SIZE(ci, scanlinepad);
	    charsize *= cim->ascent + cim->descent;
	    size += charsize;
	}
	break;
    case BitmapFormatImageRectMaxWidth:
	bpr = GLWIDTHBYTESPADDED(FONT_MAX_WIDTH(pinfo), scanlinepad);
	cfv->bpr = bpr;
	for (ch = start; ch <= end; ch++) {
	    ci = &spf->encoding[ch - firstChar];
	    if (!ci)
		ci = spf->pDefault;
	    cim = &ci->metrics;
	    charsize = bpr * (cim->ascent + cim->descent);
	    size += charsize;
	}
	break;
    case BitmapFormatImageRectMax:
	bpr = GLWIDTHBYTESPADDED(FONT_MAX_WIDTH(pinfo), scanlinepad);
	cfv->bpr = bpr;
	size = (end - start + 1) * bpr * FONT_MAX_HEIGHT(pinfo);
	break;
    default:
	assert(0);
    }

    return size;
}

static void
finish_line(SpeedoFontPtr spf)
{
    int         bpr = cfv->bpr;
    CharInfoPtr ci = &spf->encoding[cfv->char_id - spf->master->first_char_id];

    if (bpr == 0) {
	bpr = GLYPH_SIZE(ci, cfv->scanpad);
    }
    if (bpr) {			/* char may not have any metrics... */
	cfv->bp = (char *)cfv->bp + bpr;
    }
    assert(cfv->bp - sp_fp_cur->bitmaps <= sp_fp_cur->bitmap_size);
}


void
sp_set_bitmap_bits(fix15 y, fix15 xbit1, fix15 xbit2)
{
    int         nmiddle;
    CARD8	startmask,
                endmask;
    CARD8	*dst;

    if (xbit1 > cfv->bit_width) {

#ifdef CLIP_BBOX_NOISE
	SpeedoErr("Run wider than bitmap width -- truncated\n");
#endif

	xbit1 = cfv->bit_width;
    }
    if (xbit2 > cfv->bit_width) {

#ifdef CLIP_BBOX_NOISE
	SpeedoErr("Run wider than bitmap width -- truncated\n");
#endif

	xbit2 = cfv->bit_width;
    }

    if (xbit2 < xbit1) {
	xbit2 = xbit1;
    }

    while (cfv->cur_y != y) {
	finish_line(sp_fp_cur);
	cfv->cur_y++;
    }

    cfv->last_y = y;
    if (y >= cfv->bit_height) {

#ifdef CLIP_BBOX_NOISE
	SpeedoErr("Y larger than bitmap height -- truncated\n");
#endif

	cfv->trunc = 1;
	return;
    }
    if (xbit1 < 0)		/* XXX this is more than a little bit rude... */
	xbit1 = 0;

    nmiddle = (xbit1 >> 3);
    dst = (CARD8 *)cfv->bp + nmiddle;
    xbit2 -= (xbit1 & ~7);
    nmiddle = (xbit2 >> 3);
    xbit1 &= 7;
    xbit2 &= 7;
    if (bit_order == MSBFirst) {
	startmask = ((CARD8) ~0) >> xbit1;
	endmask = ~(((CARD8) ~0) >> xbit2);
    } else {
	startmask = ((CARD8) ~0) << xbit1;
	endmask = ~(((CARD8) ~0) << xbit2);
    }
    if (nmiddle == 0)
	*dst |= endmask & startmask;
    else {
	*dst++ |= startmask;
	while (--nmiddle)
	    *dst++ = (CARD8)~0;
	*dst |= endmask;
    }
}

/* ARGSUSED */
void
sp_open_bitmap(fix31 x_set_width, fix31 y_set_width, fix31 xorg, fix31 yorg,
		fix15 xsize, fix15 ysize)
{
    CharInfoPtr ci = &sp_fp_cur->encoding[cfv->char_id - sp_fp_cur->master->first_char_id];

/*-
 * this is set to provide better quality bitmaps.  since the Speedo
 * sp_get_bbox() function returns an approximate (but guarenteed to contain)
 * set of metrics, some of the bitmaps can be place poorly inside and
 * look bad.
 *
 * with this set, the actual bitmap values are used instead of the bboxes.
 * it makes things look better, but causes two possible problems:
 *
 * 1 - the reported min & max bounds may not correspond to the extents
 *	reported
 * 2 - if the extents are reported before the character is generated,
 * 	a client could see them change.  this currently never happens,
 *	but will when a desired enhancement (don't reneder till needed)
 *	is made.
 */

#define	BBOX_FIXUP 1

#ifdef BBOX_FIXUP
    int         off_horz;
    int         off_vert;

    if (xorg < 0)
	off_horz = (fix15) ((xorg - 32768L) / 65536);
    else
	off_horz = (fix15) ((xorg + 32768L) / 65536);
    if (yorg < 0)
	off_vert = (fix15) ((yorg - 32768L) / 65536);
    else
	off_vert = (fix15) ((yorg + 32768L) / 65536);
    if (xsize != 0 || ysize != 0 || ci->metrics.characterWidth)
    {
	ci->metrics.leftSideBearing = off_horz;
	ci->metrics.descent = -off_vert;
	ci->metrics.rightSideBearing = xsize + off_horz;
	ci->metrics.ascent = ysize + off_vert;
    }
    else
    {
    /* If setting the proper size would cause the character to appear to
       be non-existent, fudge things by giving it a pixel to occupy.  */
	xsize = ysize = 1;
	ci->metrics.leftSideBearing = ci->metrics.descent = 0;
	ci->metrics.rightSideBearing = ci->metrics.ascent = 1;
    }

    cfv->bit_width = xsize;
    cfv->bit_height = ysize;
#else
    cfv->bit_width = ci->metrics.rightSideBearing -
	ci->metrics.leftSideBearing;
    cfv->bit_height = ci->metrics.ascent + ci->metrics.descent;
#endif

    assert(cfv->bp - sp_fp_cur->bitmaps <= sp_fp_cur->bitmap_size);
    ci->bits = (char *) cfv->bp;

    cfv->cur_y = 0;
}

void
sp_close_bitmap()
{
    CharInfoPtr ci = &sp_fp_cur->encoding[cfv->char_id - sp_fp_cur->master->first_char_id];
    int         bpr = cfv->bpr;

    if (bpr == 0)
	bpr = GLYPH_SIZE(ci, cfv->scanpad);
    if (!cfv->trunc)
	finish_line(sp_fp_cur);
    cfv->trunc = 0;
    cfv->last_y++;
    while (cfv->last_y < cfv->bit_height) {
	finish_line(sp_fp_cur);
	cfv->last_y++;
    }
    if (byte_order != bit_order) {
	switch (scan) {
	case 1:
	    break;
	case 2:
	    TwoByteSwap(cfv->bp, bpr * cfv->bit_height);
	    break;
	case 4:
	    FourByteSwap(cfv->bp, bpr * cfv->bit_height);
	    break;
	}
    }
}

int
sp_build_all_bitmaps(
    FontPtr     pfont,
    fsBitmapFormat format,
    fsBitmapFormatMask fmask)
{
    int         ret,
                glyph = 1,
                image = BitmapFormatImageRectMin;
    unsigned long glyph_size;
    SpeedoFontPtr spf = (SpeedoFontPtr) pfont->fontPrivate;
    SpeedoMasterFontPtr spmf = spf->master;
    pointer     bitmaps;
    int         start,
                end,
                i;

    scan = 1;
    ret = CheckFSFormat(format, fmask,
			&bit_order, &byte_order, &scan, &glyph, &image);

    pfont->bit = bit_order;
    pfont->byte = byte_order;
    pfont->glyph = glyph;
    pfont->scan = scan;
    if (ret != Successful)
	return BadFontFormat;

    start = spmf->first_char_id;
    end = spmf->max_id;
    glyph_size = sp_compute_data_size(pfont, image, glyph, start, end);

    /* XXX -- MONDO KLUDGE -- add some slop */
    /*
     * not sure why this is wanted, but it keeps the packer from going off the
     * end and toasting us down the line
     */
    glyph_size += 20;

#ifdef DEBUG
    spf->bitmap_size = glyph_size;
#endif

    bitmaps = (pointer) xalloc(glyph_size);
    if (!bitmaps)
	return AllocError;
    bzero((char *) bitmaps, glyph_size);

    /* set up some state */
    sp_fp_cur = spf;
    spf->bitmaps = bitmaps;
    cfv->format = format;
    cfv->scanpad = glyph;
    cfv->bp = bitmaps;

    for (i = 0; i < spmf->num_chars; i++) {
	int j;
	cfv->char_index = spmf->enc[i * 2 + 1];
	cfv->char_id = spmf->enc[i * 2];
#ifdef DEBUG
fprintf(stderr, "build_all_sp_bitmaps:i = %d, Char ID = %d\n", i, cfv->char_id);
#endif
	if (!cfv->char_id)
	    continue;

	/*
	 * See if this character is in the list of ranges specified in the
	 * XLFD name
	 */
	for (j = 0; j < spf->vals.nranges; j++)
	    if (cfv->char_id >= mincharno(spf->vals.ranges[j]) &&
		    cfv->char_id <= maxcharno(spf->vals.ranges[j]))
		break;

	  /* If not, don't realize it. */
	if (spf->vals.nranges && j == spf->vals.nranges)
	    continue;

	if (!sp_make_char(cfv->char_index)) {

#ifdef DEBUG			/* can be very common with some encodings */
	    SpeedoErr("Can't make char %d\n", cfv->char_index);
#endif
	}
    }

    return Successful;
}
