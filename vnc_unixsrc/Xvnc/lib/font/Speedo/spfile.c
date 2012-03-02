/* $Xorg: spfile.c,v 1.4 2001/02/09 02:04:00 xorgcvs Exp $ */
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
/* $XFree86: xc/lib/font/Speedo/spfile.c,v 1.14 2001/12/14 19:56:41 dawes Exp $ */

#include "fntfilst.h"
#include "fontenc.h"
#ifndef FONTMODULE
#include <stdio.h>
#else
#include "xf86_ansic.h"
#endif

#include "spint.h"
#include "bics-unicode.h"

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
read_2b(ufix8 *ptr)
{
    fix15       tmp;

    tmp = *ptr++;
    tmp = (tmp << 8) + *ptr;
    return tmp;
}

static      fix31
read_4b(ufix8 *ptr)
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
sp_load_char_data(fix31 file_offset, fix15 num, fix15 cb_offset)
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

struct speedo_encoding {
    char *name;
    int *enc;
    int enc_size;
};

/* Takes care of caching encodings already referenced */
static int
find_encoding(const char *fontname, const char *filename,
		int **enc, int *enc_size)
{
    static struct speedo_encoding *known_encodings=0;
    static int number_known_encodings=0;
    static int known_encodings_size=0;
    
    char *encoding_name;
    int iso8859_1;
    FontMapPtr mapping;
    int i, j, k, size;
    struct speedo_encoding *temp;
    int *new_enc;
    char *new_name;
    
    iso8859_1 = 0;
    
    encoding_name = FontEncFromXLFD(fontname, strlen(fontname));
    if(!encoding_name) {
        encoding_name="iso8859-1";
        iso8859_1=1;
    }
    /* We don't go through the font library if asked for Latin-1 */
    iso8859_1 = iso8859_1 || !strcmp(encoding_name, "iso8859-1");
    
    for(i=0; i<number_known_encodings; i++) {
        if(!strcmp(encoding_name, known_encodings[i].name)) {
            *enc=known_encodings[i].enc;
            *enc_size=known_encodings[i].enc_size;
            return Successful;
        }
    }
    
    /* it hasn't been cached yet, need to compute it */
    
    /* ensure we've got enough storage first */
  
    if(known_encodings==0) {
        if((known_encodings=
            (struct speedo_encoding*)xalloc(2*sizeof(struct speedo_encoding)))
           ==0)
            return AllocError;
        number_known_encodings=0;
        known_encodings_size=2;
    }
  
    if(number_known_encodings >= known_encodings_size) {
        if((temp=
            (struct speedo_encoding*)xrealloc(known_encodings,
                                              2*sizeof(struct speedo_encoding)*
                                              known_encodings_size))==0)
            return AllocError;
        known_encodings=temp;
        known_encodings_size*=2;
    }

    mapping=0;
    if(!iso8859_1) {
        mapping = FontEncMapFind(encoding_name, 
                                 FONT_ENCODING_UNICODE, -1, -1,
                                 filename);
    }
#define SPEEDO_RECODE(c) \
  (mapping? \
   unicode_to_bics(FontEncRecode(c, mapping)): \
   unicode_to_bics(c))
        
    if((new_name = (char*)xalloc(strlen(encoding_name)))==0)
        return AllocError;
    strcpy(new_name, encoding_name);
    
    /* For now, we limit ourselves to 256 glyphs */
    size=0;
    for(i=0; i < (mapping?mapping->encoding->size:256) && i < 256; i++)
        if(SPEEDO_RECODE(i)>=0)
            size++;
    new_enc = (int*)xalloc(2*size*sizeof(int));
    if(!new_enc) {
        xfree(new_name);
        return AllocError;
    }
    for(i=j=0; i < (mapping?mapping->encoding->size:256) && i < 256; i++)
        if((k = SPEEDO_RECODE(i))>=0) {
            new_enc[2*j] = i;
            new_enc[2*j+1] = k;
            j++;
        }
    known_encodings[number_known_encodings].name = new_name;
    known_encodings[number_known_encodings].enc = new_enc;
    known_encodings[number_known_encodings].enc_size = size;
    number_known_encodings++;
    
    *enc = new_enc;
    *enc_size = size;
    return Successful;
#undef SPEEDO_RECODE
}

int
sp_open_master(const char *fontname, const char *filename,
		SpeedoMasterFontPtr *master)
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


    spmf->enc = 0;
    spmf->enc_size = 0;

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

    if(!spmf->enc)
      if((ret=find_encoding(fontname, filename, &spmf->enc, &spmf->enc_size))
         !=Successful)
        goto cleanup;

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
sp_close_master_font(SpeedoMasterFontPtr spmf)
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
sp_close_master_file(SpeedoMasterFontPtr spmf)
{
    (void) fclose(spmf->fp);
    spmf->state &= ~MasterFileOpen;
}


/*
 * reset the encryption key, and make sure the file is opened
 */
void
sp_reset_master(SpeedoMasterFontPtr spmf)
{
    sp_set_key(spmf->key);
    if (!(spmf->state & MasterFileOpen)) {
	spmf->fp = fopen(spmf->fname, "r");
	/* XXX -- what to do if we can't open the file? */
	spmf->state |= MasterFileOpen;
    }
    fseek(spmf->fp, 0, 0);
}
