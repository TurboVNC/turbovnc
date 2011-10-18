/* $XConsortium: spint.h /main/12 1996/09/28 16:47:40 rws $ */
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
 */

/*

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

#ifndef _SPINT_H_
#define _SPINT_H_

#include <stdio.h>
#include "fntfilst.h"
#include <X11/Xfuncproto.h>
#include "speedo.h"

#define	SaveMetrics	0x1
#define ComputeBoundsOnly	0x2

#define GLWIDTHBYTESPADDED(bits,nbytes) \
        ((nbytes) == 1 ? (((bits)+7)>>3)        /* pad to 1 byte */ \
        :(nbytes) == 2 ? ((((bits)+15)>>3)&~1)  /* pad to 2 bytes */ \
        :(nbytes) == 4 ? ((((bits)+31)>>3)&~3)  /* pad to 4 bytes */ \
        :(nbytes) == 8 ? ((((bits)+63)>>3)&~7)  /* pad to 8 bytes */ \
        : 0)

#define GLYPH_SIZE(ch, nbytes)          \
        GLWIDTHBYTESPADDED((ch)->metrics.rightSideBearing - \
                        (ch)->metrics.leftSideBearing, (nbytes))

#define mincharno(p) ((p).min_char_low + ((p).min_char_high << 8))
#define maxcharno(p) ((p).max_char_low + ((p).max_char_high << 8))

#define	MasterFileOpen	0x1

typedef struct _sp_master {
    FontEntryPtr    entry;	/* back pointer */
    FILE       *fp;
    char       *fname;
    ufix8      *f_buffer;
    ufix8      *c_buffer;
    char       *copyright;
    ufix8      *key;
    buff_t      font;
    buff_t      char_data;
    ufix16      mincharsize;
    int         first_char_id;
    int         num_chars;
    int         max_id;
    int         state;		/* open, closed */
    int         refcount;	/* number of instances */
    int        *enc;
    int         enc_size;
}           SpeedoMasterFontRec, *SpeedoMasterFontPtr;

typedef struct _cur_font_stats {
    fsBitmapFormat format;
    /* current glyph info */
    ufix16      char_index;
    ufix16      char_id;

    fix15       bit_width,
                bit_height;
    fix15       cur_y;
    int         bpr;

    /*
     * since Speedo returns extents that are not identical to what it feeds to
     * the bitmap builder, and we want to be able to use the extents for
     * preformance reasons, some of the bitmaps require padding out.  the next
     * two flags keep track of this.
     */
    fix15       last_y;
    int         trunc;

    pointer     bp;
    int         scanpad;
}           CurrentFontValuesRec, *CurrentFontValuesPtr;


typedef struct _sp_font {
    struct _sp_master *master;
    specs_t     specs;

    FontEntryPtr    entry;

    FontScalableRec vals;

    /* char & metric data */
    CharInfoPtr encoding;
    CharInfoPtr pDefault;
    pointer     bitmaps;

#ifdef DEBUG
    unsigned long bitmap_size;
#endif

}           SpeedoFontRec, *SpeedoFontPtr;

extern SpeedoFontPtr sp_fp_cur;

extern int  sp_open_font();
extern int  sp_open_master();
extern void sp_close_font();
extern void sp_close_master_font();
extern void sp_close_master_file();
extern void sp_reset_master();
#if NeedVarargsPrototypes
extern void SpeedoErr(char *fmt, ...);
#else
extern void SpeedoErr();
#endif

extern void sp_make_standard_props();
extern void sp_make_header();
extern void sp_compute_bounds();
extern void sp_compute_props();
extern int  sp_build_all_bitmaps();
extern unsigned long sp_compute_data_size();

extern int  sp_bics_map[];
extern int  sp_bics_map_size;

#ifdef EXTRAFONTS
extern int  adobe_map[];
extern int  adobe_map_size;

#endif

#endif				/* _SPINT_H_ */
