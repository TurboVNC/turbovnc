/* $TOG: spfile.c /main/13 1997/06/09 09:38:27 barstow $ */
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
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES OR DIGITAL 
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Dave Lemke, Network Computing Devices Inc
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

#include <stdio.h>
#include "fntfilst.h"

#include "spint.h"

SpeedoFontPtr sp_fp_cur = (SpeedoFontPtr) 0;

#ifdef EXTRAFONTS
#include "ncdkeys.h"
#endif

#include "keys.h"

#ifdef EXTRAFONTS
static ufix8 skey[] =
{
    SKEY0,
    SKEY1,
    SKEY2,
    SKEY3,
    SKEY4,
    SKEY5,
    SKEY6,
    SKEY7,
    SKEY8
};				/* Sample Font decryption key */

static ufix8 rkey[] =
{
    RKEY0,
    RKEY1,
    RKEY2,
    RKEY3,
    RKEY4,
    RKEY5,
    RKEY6,
    RKEY7,
    RKEY8
};				/* Retail Font decryption key */

#endif				/* EXTRAFONTS */

#ifdef XSAMPLEFONTS
static ufix8 xkey[] =
{
    XKEY0,
    XKEY1,
    XKEY2,
    XKEY3,
    XKEY4,
    XKEY5,
    XKEY6,
    XKEY7,
    XKEY8
};				/* Sample Font decryption key */
#endif

static ufix8 mkey[] =
{
    KEY0,
    KEY1,
    KEY2,
    KEY3,
    KEY4,
    KEY5,
    KEY6,
    KEY7,
    KEY8
};				/* Font decryption key */


static      fix15
read_2b(ptr)
    ufix8      *ptr;
{
    fix15       tmp;

    tmp = *ptr++;
    tmp = (tmp << 8) + *ptr;
    return tmp;
}

static      fix31
read_4b(ptr)
    ufix8      *ptr;
{
    fix31       tmp;

    tmp = *ptr++;
    tmp = (tmp << 8) + *ptr++;
    tmp = (tmp << 8) + *ptr++;
    tmp = (tmp << 8) + *ptr;
    return tmp;
}

/*
 * loads the specified char's data
 */
buff_t     *
sp_load_char_data(file_offset, num, cb_offset)
    fix31       file_offset;
    fix15       num;
    fix15       cb_offset;
{
    SpeedoMasterFontPtr master = sp_fp_cur->master;

    if (fseek(master->fp, (long) file_offset, (int) 0)) {
	SpeedoErr("can't seek to char\n");
    }
    if ((num + cb_offset) > master->mincharsize) {
	SpeedoErr("char buf overflow\n");
    }
    if (fread((master->c_buffer + cb_offset), sizeof(ufix8), num,
	      master->fp) != num) {
	SpeedoErr("can't get char data\n");
    }
    master->char_data.org = (ufix8 *) master->c_buffer + cb_offset;
    master->char_data.no_bytes = num;

    return &master->char_data;
}

int
sp_open_master(filename, master)
    char       *filename;
    SpeedoMasterFontPtr *master;
{
    SpeedoMasterFontPtr spmf;
    ufix8       tmp[16];
    ufix16      cust_no;
    FILE       *fp;
    ufix32      minbufsize;
    ufix16      mincharsize;
    ufix8      *f_buffer;
    ufix8      *c_buffer;
    int         ret;
    ufix8      *key;

    spmf = (SpeedoMasterFontPtr) xalloc(sizeof(SpeedoMasterFontRec));
    if (!spmf)
	return AllocError;
    bzero(spmf, sizeof(SpeedoMasterFontRec));
    spmf->entry = NULL;
    spmf->f_buffer = NULL;
    spmf->c_buffer = NULL;

    /* open font */
    spmf->fname = (char *) xalloc(strlen(filename) + 1);
    if (!spmf->fname)
	return AllocError;
    fp = fopen(filename, "r");
    if (!fp) {
	ret = BadFontName;
	goto cleanup;
    }
    strcpy(spmf->fname, filename);
    spmf->fp = fp;
    spmf->state |= MasterFileOpen;

    if (fread(tmp, sizeof(ufix8), 16, fp) != 16) {
	ret = BadFontName;
	goto cleanup;
    }
    minbufsize = (ufix32) read_4b(tmp + FH_FBFSZ);
    f_buffer = (ufix8 *) xalloc(minbufsize);
    if (!f_buffer) {
	ret = AllocError;
	goto cleanup;
    }
    spmf->f_buffer = f_buffer;

    fseek(fp, (ufix32) 0, 0);

    /* read in the font */
    if (fread(f_buffer, sizeof(ufix8), (ufix16) minbufsize, fp) != minbufsize) {
	ret = BadFontName;
	goto cleanup;
    }
    spmf->copyright = (char *) (f_buffer + FH_CPYRT);
    spmf->mincharsize = mincharsize = read_2b(f_buffer + FH_CBFSZ);

    c_buffer = (ufix8 *) xalloc(mincharsize);
    if (!c_buffer) {
	ret = AllocError;
	goto cleanup;
    }
    spmf->c_buffer = c_buffer;

    spmf->font.org = spmf->f_buffer;
    spmf->font.no_bytes = minbufsize;

    cust_no = sp_get_cust_no(spmf->font);

    /* XXX add custom encryption stuff here */

#ifdef EXTRAFONTS
    if (cust_no == SCUS0) {
	key = skey;
    } else if (cust_no == RCUS0) {
	key = rkey;
    } else
#endif

#ifdef XSAMPLEFONTS
    if (cust_no == XCUS0) {
	key = xkey;
    } else
#endif

    if (cust_no == CUS0) {
	key = mkey;
    } else {
	SpeedoErr("Non - standard encryption for \"%s\"\n", filename);
	ret = BadFontName;
	goto cleanup;
    }
    spmf->key = key;
    sp_set_key(key);

    spmf->first_char_id = read_2b(f_buffer + FH_FCHRF);
    spmf->num_chars = read_2b(f_buffer + FH_NCHRL);


    spmf->enc = sp_bics_map;
    spmf->enc_size = sp_bics_map_size;

#ifdef EXTRAFONTS
    {				/* choose the proper encoding */
	char       *f;

	f = strrchr(filename, '/');
	if (f) {
	    f++;
	    if (strncmp(f, "bx113", 5) == 0) {
		spmf->enc = adobe_map;
		spmf->enc_size = adobe_map_size;
	    }
	}
    }
#endif

    /* XXX slam back to ISO Latin1 */
    spmf->first_char_id = spmf->enc[0];
    /* size of extents array */
    spmf->max_id = spmf->enc[(spmf->enc_size - 1) * 2];
    spmf->num_chars = spmf->enc_size;

    *master = spmf;

    return Successful;

cleanup:
    *master = (SpeedoMasterFontPtr) 0;
    sp_close_master_font(spmf);
    return ret;
}

void
sp_close_master_font(spmf)
    SpeedoMasterFontPtr spmf;
{
    if (!spmf)
	return;
    if (spmf->state & MasterFileOpen)
	fclose(spmf->fp);
    if (spmf->entry)
	spmf->entry->u.scalable.extra->private = NULL;
    xfree(spmf->fname);
    xfree(spmf->f_buffer);
    xfree(spmf->c_buffer);
    xfree(spmf);
}

void
sp_close_master_file(spmf)
    SpeedoMasterFontPtr spmf;
{
    (void) fclose(spmf->fp);
    spmf->state &= ~MasterFileOpen;
}


/*
 * reset the encryption key, and make sure the file is opened
 */
void
sp_reset_master(spmf)
    SpeedoMasterFontPtr spmf;
{
    sp_set_key(spmf->key);
    if (!(spmf->state & MasterFileOpen)) {
	spmf->fp = fopen(spmf->fname, "r");
	/* XXX -- what to do if we can't open the file? */
	spmf->state |= MasterFileOpen;
    }
    fseek(spmf->fp, 0, 0);
}
