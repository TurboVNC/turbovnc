/* $XConsortium: decompress.c,v 1.5 94/04/17 20:26:57 dpw Exp $ */
/*

Copyright (c) 1990  X Consortium

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


/* 
 * decompress - cat a compressed file
 */

#include <stdio.h>

#ifdef TEST
#define xalloc(s)   malloc(s)
#define xfree(s)    free(s)

typedef char	*FID;

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

#else
#include    "Xos.h"
#include    "misc.h"
#endif

#define BITS	16

/*
 * a code_int must be able to hold 2**BITS values of type int, and also -1
 */
#if BITS > 15
typedef long int	code_int;
#else
typedef int		code_int;
#endif

typedef long int	  count_int;

#ifdef NO_UCHAR
 typedef char	char_type;
#else
 typedef	unsigned char	char_type;
#endif /* UCHAR */

static char_type magic_header[] = { "\037\235" };	/* 1F 9D */

/* Defines for third byte of header */
#define BIT_MASK	0x1f
#define BLOCK_MASK	0x80
/* Masks 0x40 and 0x20 are free.  I think 0x20 should mean that there is
   a fourth header byte (for expansion).
*/

#define INIT_BITS 9			/* initial number of bits/code */

#ifdef COMPATIBLE		/* But wrong! */
# define MAXCODE(n_bits)	(1 << (n_bits) - 1)
#else
# define MAXCODE(n_bits)	((1 << (n_bits)) - 1)
#endif /* COMPATIBLE */

static code_int getcode();

/*
 * the next two codes should not be changed lightly, as they must not
 * lie within the contiguous general code space.
 */ 
#define FIRST	257	/* first free entry */
#define	CLEAR	256	/* table clear output code */

#define STACK_SIZE  8192

typedef struct _compressedFILE {
    FILE	*file;

    char_type	    *stackp;
    code_int	    oldcode;
    char_type	    finchar;

    int		block_compress;
    int		maxbits;
    code_int	maxcode, maxmaxcode;

    code_int	free_ent;
    int		clear_flg;
    int		n_bits;

    /* bit buffer */
    int		offset, size;
    char_type	buf[BITS];

    char_type	    de_stack[STACK_SIZE];
    char_type	    *tab_suffix;
    unsigned short  *tab_prefix;
} CompressedFile;

static int hsize_table[] = {
    5003,	/* 12 bits - 80% occupancy */
    9001,	/* 13 bits - 91% occupancy */
    18013,	/* 14 bits - 91% occupancy */
    35023,	/* 15 bits - 94% occupancy */
    69001	/* 16 bits - 95% occupancy */
};

FID
CompressedFontFileInit (f)
    FILE	*f;
{
    int		    code;
    int		    maxbits;
    int		    hsize;
    CompressedFile  *file;
    int		    extra;

    if ((getc(f) != (magic_header[0] & 0xFF)) ||
	(getc(f) != (magic_header[1] & 0xFF)))
    {
	return 0;
    }
    code = getc (f);
    maxbits = code & BIT_MASK;
    if (maxbits > BITS || maxbits < 12)
	return 0;
    hsize = hsize_table[maxbits - 12];
    extra = (1 << maxbits) * sizeof (char_type) +
	    hsize * sizeof (unsigned short);
    file = (CompressedFile *) xalloc (sizeof (CompressedFile) + extra);
    if (!file)
	return 0;
    file->file = f;
    file->maxbits = maxbits;
    file->block_compress = code & BLOCK_MASK;
    file->maxmaxcode = 1 << file->maxbits;
    file->tab_suffix = (char_type *) &file[1];
    file->tab_prefix = (unsigned short *) (file->tab_suffix + file->maxmaxcode);
    /*
     * As above, initialize the first 256 entries in the table.
     */
    file->maxcode = MAXCODE(file->n_bits = INIT_BITS);
    for ( code = 255; code >= 0; code-- ) {
	file->tab_prefix[code] = 0;
	file->tab_suffix[code] = (char_type) code;
    }
    file->free_ent = ((file->block_compress) ? FIRST : 256 );
    file->clear_flg = 0;
    file->offset = 0;
    file->size = 0;
    file->stackp = file->de_stack;
    file->finchar = file->oldcode = getcode (file);
    if (file->oldcode != -1)
	*file->stackp++ = file->finchar;
    return (FID) file;
}

FILE *
CompressedFontFileDone (fid)
    FID		    fid;
{
    CompressedFile  *file;
    FILE    *f;

    file = (CompressedFile *) fid;
    f = file->file;
    xfree (file);
    return f;
}

#define getdcchar(file)    ((file)->stackp > (file)->de_stack ? (*--((file)->stackp)) : _filldcbuf (file))

_filldcbuf (file)
    CompressedFile  *file;
{
    register char_type *stackp;
    register code_int code, incode;

    if (file->stackp > file->de_stack)
	return *--file->stackp;

    if (file->oldcode == -1)
	return EOF;

    stackp = file->stackp;
    code = getcode (file);
    if (code == -1)
	return EOF;

    if ( (code == CLEAR) && file->block_compress ) {
	for ( code = 255; code >= 0; code-- )
	    file->tab_prefix[code] = 0;
	file->clear_flg = 1;
	file->free_ent = FIRST - 1;
	if ( (code = getcode (file)) == -1 )	/* O, untimely death! */
	    return EOF;
    }
    incode = code;
    /*
     * Special case for KwKwK string.
     */
    if ( code >= file->free_ent ) {
	*stackp++ = file->finchar;
	code = file->oldcode;
    }

    /*
     * Generate output characters in reverse order
     */
    while ( code >= 256 )
    {
	*stackp++ = file->tab_suffix[code];
	code = file->tab_prefix[code];
    }
    file->finchar = file->tab_suffix[code];

    /*
     * Generate the new entry.
     */
    if ( (code=file->free_ent) < file->maxmaxcode ) {
	file->tab_prefix[code] = (unsigned short)file->oldcode;
	file->tab_suffix[code] = file->finchar;
	file->free_ent = code+1;
    } 
    /*
     * Remember previous code.
     */
    file->oldcode = incode;
    file->stackp = stackp;
    return file->finchar;
}

/*****************************************************************
 * TAG( getcode )
 *
 * Read one code from the standard input.  If EOF, return -1.
 * Inputs:
 * 	stdin
 * Outputs:
 * 	code or -1 is returned.
 */

static char_type rmask[9] = {0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};

static code_int
getcode(file)
    CompressedFile  *file;
{
    register code_int code;
    register int r_off, bits;
    register char_type *bp = file->buf;
    register FILE   *fp;

    if ( file->clear_flg > 0 || file->offset >= file->size ||
	file->free_ent > file->maxcode )
    {
	/*
	 * If the next entry will be too big for the current code
	 * size, then we must increase the size.  This implies reading
	 * a new buffer full, too.
	 */
	if ( file->free_ent > file->maxcode ) {
	    file->n_bits++;
	    if ( file->n_bits == file->maxbits )
		file->maxcode = file->maxmaxcode;	/* won't get any bigger now */
	    else
		file->maxcode = MAXCODE(file->n_bits);
	}
	if ( file->clear_flg > 0) {
    	    file->maxcode = MAXCODE (file->n_bits = INIT_BITS);
	    file->clear_flg = 0;
	}
	bits = file->n_bits;
	fp = file->file;
	while (bits > 0 && (code = getc (fp)) != EOF)
	{
	    *bp++ = code;
	    --bits;
	}
	bp = file->buf;
	if (bits == file->n_bits)
	    return -1;			/* end of file */
	file->size = file->n_bits - bits;
	file->offset = 0;
	/* Round size down to integral number of codes */
	file->size = (file->size << 3) - (file->n_bits - 1);
    }
    r_off = file->offset;
    bits = file->n_bits;
    /*
     * Get to the first byte.
     */
    bp += (r_off >> 3);
    r_off &= 7;
    /* Get first part (low order bits) */
#ifdef NO_UCHAR
    code = ((*bp++ >> r_off) & rmask[8 - r_off]) & 0xff;
#else
    code = (*bp++ >> r_off);
#endif /* NO_UCHAR */
    bits -= (8 - r_off);
    r_off = 8 - r_off;		/* now, offset into code word */
    /* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
    if ( bits >= 8 ) {
#ifdef NO_UCHAR
	code |= (*bp++ & 0xff) << r_off;
#else
	code |= *bp++ << r_off;
#endif /* NO_UCHAR */
	r_off += 8;
	bits -= 8;
    }
    /* high order bits. */
    code |= (*bp & rmask[bits]) << r_off;
    file->offset += file->n_bits;

    return code;
}

CompressedFontFileRead (buf, itemsize, nitems, fid)
    char	*buf;
    unsigned	itemsize;
    unsigned	nitems;
    FID		fid;
{
    CompressedFile  *file;
    int		    c;
    int		    nbytes;

    file = (CompressedFile *) fid;
    nbytes = nitems * itemsize;
    while (nbytes)
    {
	if ((c = getdcchar (file)) == EOF)
	    break;
	*buf++ = c;
	--nbytes;
    }
    return nitems - nbytes / itemsize;
}

CompressedFontFileSkip (bytes, fid)
    unsigned	bytes;
    FID		fid;
{
    int	    c;

    while (bytes-- && ((c = getdcchar((CompressedFile *)fid)) != EOF))
	    ;
    return c;
}

#ifdef TEST
main ()
{
    CompressedFile  *input;
    int		    c;
    
    input = (CompressedFile *) CompressedFontFileInit (stdin);
    while ((c = getdcchar (input)) != -1)
	putchar (c);
}
#endif
