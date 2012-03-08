/* $Xorg: nsample.c,v 1.3 2000/08/17 19:46:26 cpqbld Exp $ */

/*

Copyright 1989-1991, Bitstream Inc., Cambridge, MA.
You are hereby granted permission under all Bitstream propriety rights to
use, copy, modify, sublicense, sell, and redistribute the Bitstream Speedo
software and the Bitstream Charter outline font for any purpose and without
restrictions; provided, that this notice is left intact on all copies of such
software or font and that Bitstream's trademark is acknowledged as shown below
on all unmodified copies of such font.

BITSTREAM CHARTER is a registered trademark of Bitstream Inc.


BITSTREAM INC. DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING
WITHOUT LIMITATION THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE.  BITSTREAM SHALL NOT BE LIABLE FOR ANY DIRECT OR INDIRECT
DAMAGES, INCLUDING BUT NOT LIMITED TO LOST PROFITS, LOST DATA, OR ANY OTHER
INCIDENTAL OR CONSEQUENTIAL DAMAGES, ARISING OUT OF OR IN ANY WAY CONNECTED
WITH THE SPEEDO SOFTWARE OR THE BITSTREAM CHARTER OUTLINE FONT.

*/


/*************************** N S A M P L E . C *******************************
 *                                                                           *
 *                 SPEEDO CHARACTER GENERATOR TEST MODULE                    *
 *                                                                           *
 * This is an illustration of what external resources are required to        *
 * load a Speedo outline and use the Speedo character generator to generate  *
 * bitmaps or scaled outlines according to the desired specification.        *                                                    *
 *                                                                           *
 * This program loads a Speedo outline, defines a set of character           *
 * generation specifications, generates bitmap (or outline) data for each    *
 * character in the font and prints them on the standard output.             *
 *                                                                           *
 * If the font buffer is too small to hold the entire font, the first        *
 * part of the font is loaded. Character data is then loaded dynamically     *
 * as required.                                                              *
 *                                                                           *
 ****************************************************************************/

#include <stdio.h>
#if PROTOS_AVAIL
#include <stddef.h>
#include <malloc.h>
#include <stdlib.h>
void main(int argc,char *argv[]);
#else
void* malloc();
#endif

#include "speedo.h"                 /* General definition for make_bmap */
#include "keys.h"                  /* Font decryption keys */

#define DEBUG  0

#if DEBUG
#define SHOW(X) printf("X = %d\n", X)
#else
#define SHOW(X)
#endif

#define MAX_BITS  256              /* Max line length of generated bitmap */

/***** GLOBAL FUNCTIONS *****/

/***** EXTERNAL FUNCTIONS *****/

/***** STATIC FUNCTIONS *****/

#if PROTOS_AVAIL
fix31 read_4b(ufix8 FONTFAR *ptr);
fix15 read_2b(ufix8 FONTFAR *ptr);
#else
fix31 read_4b();
fix15 read_2b();
#endif
/***** STATIC VARIABLES *****/
static  char   pathname[100];      /* Name of font file to be output */
static  ufix8 FONTFAR *font_buffer;       /* Pointer to allocated Font buffer */
static  ufix8 FONTFAR *char_buffer;       /* Pointer to allocate Character buffer */
static  buff_t font;               /* Buffer descriptor for font data */
#if INCL_LCD
static  buff_t char_data;          /* Buffer descriptor for character data */
#endif
static  FILE  *fdescr;             /* Speedo outline file descriptor */
static  ufix16 char_index;         /* Index of character to be generated */
static  ufix16 char_id;            /* Character ID */
static  ufix16 minchrsz;              /* minimum character buffer size */

static  ufix8  key[] = 
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
    };                             /* Font decryption key */

static  fix15  raswid;             /* raster width  */
static  fix15  rashgt;             /* raster height */
static  fix15  offhor;             /* horizontal offset from left edge of emsquare */
static  fix15  offver;             /* vertical offset from baseline */
static  fix15  set_width;          /* character set width */
static  fix15  y_cur;              /* Current y value being generated and printed */
static  char   line_of_bits[2 * MAX_BITS + 1]; /* Buffer for row of generated bits */

#if INCL_MULTIDEV
#if INCL_BLACK || INCL_SCREEN || INCL_2D
bitmap_t bfuncs = { sp_open_bitmap, sp_set_bitmap_bits, sp_close_bitmap };
#endif
#if INCL_OUTLINE
outline_t ofuncs = { sp_open_outline, sp_start_new_char, sp_start_contour, sp_curve_to,
                     sp_line_to, sp_close_contour, sp_close_outline };
#endif
#endif


ufix8   temp[16];             /* temp buffer for first 16 bytes of font */


FUNCTION void main(argc,argv)
int argc;
char *argv[];
{
ufix16     bytes_read;           /* Number of bytes read from font file */
specs_t specs;                /* Bundle of character generation specs  */
int     first_char_index;     /* Index of first character in font */
int     no_layout_chars;        /* number of characters in layout */
ufix32  i;
ufix32  minbufsz;             /* minimum font buffer size to allocate */
ufix16 cust_no;
ufix8  FONTFAR *byte_ptr;

#if REENTRANT_ALLOC
SPEEDO_GLOBALS* sp_global_ptr;
#endif


if (argc != 2) 
    {
    fprintf(stderr,"Usage: nsample {fontfile}\n\n"); 
    exit (1);
    }

sprintf(pathname, argv[1]);

/* Load Speedo outline file */
fdescr = fopen (pathname, "rb");
if (fdescr == NULL)
    {
    printf("****** Cannot open file %s\n", pathname);
    return;                  
    }

/* get minimum font buffer size - read first 16 bytes to get the minimum
   size field from the header, then allocate buffer dynamically  */

bytes_read = fread(temp, sizeof(ufix8), 16, fdescr);

if (bytes_read != 16)
	{
	printf("****** Error on reading %s: %x\n", pathname, bytes_read);
	fclose(fdescr);
	return;
	}
#if INCL_LCD
minbufsz = (ufix32)read_4b(temp+FH_FBFSZ);
#else
minbufsz = (ufix32)read_4b(temp+FH_FNTSZ);
if (minbufsz >= 0x10000)
	{
	printf("****** Cannot process fonts greater than 64K - use dynamic character loading configuration option\n");
	fclose(fdescr);
	return;
	}
#endif

#if (defined(M_I86SM) || defined(M_I86MM))
font_buffer = (ufix8 FONTFAR *)_fmalloc((ufix16)minbufsz);
#else
font_buffer = (ufix8 *)malloc((ufix16)minbufsz);
#endif
      
if (font_buffer == NULL)
	{
	printf("****** Unable to allocate memory for font buffer\n");
    fclose(fdescr);
	return;
	}

#if DEBUG
printf("Loading font file %s\n", pathname);
#endif

fseek(fdescr, (ufix32)0, 0);
#if (defined(M_I86SM) || (defined(M_I86MM)))
byte_ptr = font_buffer;
for (i=0; i< minbufsz; i++){
    int ch;
    ch = getc(fdescr);
    if (ch == EOF)
       {printf ("Premature EOF in reading font buffer, %ld bytes read\n",i);
        exit(2);}
    *byte_ptr=(ufix8)ch;
    byte_ptr++;
    }
bytes_read = i;
#else
bytes_read = fread((ufix8 *)font_buffer, sizeof(ufix8), (ufix16)minbufsz, fdescr);
if (bytes_read == 0)
    {
    printf("****** Error on reading %s: %x\n", pathname, bytes_read);
    fclose(fdescr);     
    return;
    }
#endif

#if INCL_LCD
/* now allocate minimum character buffer */

minchrsz = read_2b(font_buffer+FH_CBFSZ);
#if (defined(M_I86SM) || (defined(M_I86MM)))
char_buffer = (ufix8 FONTFAR *)_fmalloc(minchrsz);
#else
char_buffer = (ufix8*)malloc(minchrsz);
#endif

if (char_buffer == NULL)
	{
	printf("****** Unable to allocate memory for character buffer\n");
    fclose(fdescr);     
	return;
	}
#endif

#if DYNAMIC_ALLOC || REENTRANT_ALLOC
	sp_global_ptr = (SPEEDO_GLOBALS *)malloc(sizeof(SPEEDO_GLOBALS));
	memset(sp_global_ptr,(ufix8)0,sizeof(SPEEDO_GLOBALS));
#endif


/* Initialization */
#if REENTRANT_ALLOC
sp_reset(sp_global_ptr);                   /* Reset Speedo character generator */
#else
sp_reset();                   /* Reset Speedo character generator */
#endif

font.org = font_buffer;
font.no_bytes = bytes_read;

#if REENTRANT_ALLOC
if ((cust_no=sp_get_cust_no(sp_global_ptr,font)) != CUS0 && /* NOT STANDARD ENCRYPTION */
#else
if ((cust_no=sp_get_cust_no(font)) != CUS0 && /* NOT STANDARD ENCRYPTION */
#endif
				cust_no != 0)
	{
#if REENTRANT_ALLOC
	printf("Unable to use fonts for customer number %d\n",
				sp_get_cust_no(sp_global_ptr(font)));
#else
	printf("Unable to use fonts for customer number %d\n",
				sp_get_cust_no(font));
#endif
    fclose(fdescr);
	return;
   	}

#if INCL_KEYS
#if REENTRANT_ALLOC
sp_set_key(sp_global_ptr,key);              /* Set decryption key */
#else
sp_set_key(key);              /* Set decryption key */
#endif
#endif

#if INCL_MULTIDEV
#if INCL_BLACK || INCL_SCREEN || INCL_2D
#if REENTRANT_ALLOC
sp_set_bitmap_device(sp_global_ptr,&bfuncs,sizeof(bfuncs));              /* Set decryption key */
#else
sp_set_bitmap_device(&bfuncs,sizeof(bfuncs));              /* Set decryption key */
#endif
#endif
#if INCL_OUTLINE
#if REENTRANT_ALLOC
sp_set_outline_device(sp_global_ptr,&ofuncs,sizeof(ofuncs));              /* Set decryption key */
#else
sp_set_outline_device(&ofuncs,sizeof(ofuncs));              /* Set decryption key */
#endif
#endif
#endif

first_char_index = read_2b(font_buffer + FH_FCHRF);
no_layout_chars = read_2b(font_buffer + FH_NCHRL);

/* Set specifications for character to be generated */
specs.pfont = &font;          /* Pointer to Speedo outline structure */
specs.xxmult = 25L << 16;     /* Coeff of X to calculate X pixels */
specs.xymult = 0L << 16;      /* Coeff of Y to calculate X pixels */
specs.xoffset = 0L << 16;     /* Position of X origin */
specs.yxmult = 0L << 16;      /* Coeff of X to calculate Y pixels */
specs.yymult = 25L << 16;     /* Coeff of Y to calculate Y pixels */
specs.yoffset = 0L << 16;     /* Position of Y origin */
specs.flags = 0;         /* Mode flags */
specs.out_info = NULL;   


#if REENTRANT_ALLOC
if (!sp_set_specs(sp_global_ptr,&specs))    /* Set character generation specifications */
#else
if (!sp_set_specs(&specs))    /* Set character generation specifications */
#endif
    {
    printf("****** Cannot set requested specs\n");
    }
else
    {
    for (i = 0; i < no_layout_chars; i++)   /* For each character in font */
        {
        char_index = i + first_char_index;
#if REENTRANT_ALLOC
        char_id = sp_get_char_id(sp_global_ptr,char_index);
#else
        char_id = sp_get_char_id(char_index);
#endif
		if (char_id != 0)
			{
#if REENTRANT_ALLOC
	        if (!sp_make_char(sp_global_ptr,char_index))
#else
	        if (!sp_make_char(char_index))
#endif
	            {
    	        printf("****** Cannot generate character %d\n", char_index);
        	    }
			}
        }
    }

fclose(fdescr);     
}

#if INCL_LCD
#if REENTRANT_ALLOC
FUNCTION buff_t *sp_load_char_data(sp_global_ptr, file_offset, no_bytes, cb_offset)
SPEEDO_GLOBALS *sp_global_ptr;
#else
FUNCTION buff_t *sp_load_char_data(file_offset, no_bytes, cb_offset)
#endif
fix31    file_offset;  /* Offset in bytes from the start of the font file */
fix15    no_bytes;     /* Number of bytes to be loaded */
fix15    cb_offset;    /* Offset in bytes from start of char buffer */
/*
 * Called by Speedo character generator to request that character
 * data be loaded from the font file into a character data buffer.
 * The character buffer offset is zero for all characters except elements
 * of compound characters. If a single buffer is allocated for character
 * data, cb_offset ensures that sub-character data is loaded after the
 * top-level compound character.
 * Returns a pointer to a buffer descriptor.
 */
{
int     bytes_read;

#if DEBUG
printf("\nCharacter data(%d, %d, %d) requested\n", file_offset, no_bytes, cb_offset);
#endif
if (fseek(fdescr, (long)file_offset, (int)0) != 0)
    {
    printf("****** Error in seeking character\n");
    fclose(fdescr);     
    exit(1);
    }

if ((no_bytes + cb_offset) > minchrsz)
    {
    printf("****** Character buffer overflow\n");
    fclose(fdescr);     
    exit(3);
    }

bytes_read = fread((char_buffer + cb_offset), sizeof(ufix8), no_bytes, fdescr);
if (bytes_read != no_bytes)
    {
    printf("****** Error on reading character data\n");
    fclose(fdescr);     
    exit(2);
    }

#if DEBUG
printf("Character data loaded\n");
#endif

char_data.org = (ufix8 FONTFAR *)char_buffer + cb_offset;
char_data.no_bytes = no_bytes;
return &char_data;
}
#endif


#if REENTRANT_ALLOC
FUNCTION void sp_report_error(sp_global_ptr,n)
SPEEDO_GLOBALS *sp_global_ptr;
#else
FUNCTION void sp_report_error(n)
#endif
fix15 n;        /* Error identification number */
/*
 * Called by Speedo character generator to report an error.
 *
 *  Since character data not available is one of those errors
 *  that happens many times, don't report it to user
 */
{

switch(n)
    {
case 1:
    printf("Insufficient font data loaded\n");
    break;

case 3:
    printf("Transformation matrix out of range\n");
    break;

case 4:
    printf("Font format error\n");
    break;
                 
case 5:
    printf("Requested specs not compatible with output module\n");
    break;

case 7:
    printf("Intelligent transformation requested but not supported\n");
    break;

case 8:
    printf("Unsupported output mode requested\n");
    break;

case 9:
    printf("Extended font loaded but only compact fonts supported\n");
    break;

case 10:
    printf("Font specs not set prior to use of font\n");
    break;

case 12:
    break;

case 13:
    printf("Track kerning data not available()\n");
    break;

case 14:
    printf("Pair kerning data not available()\n");
    break;

default:
    printf("report_error(%d)\n", n);
    break;
    }
}

#if REENTRANT_ALLOC
FUNCTION void sp_open_bitmap(sp_global_ptr, x_set_width, y_set_width, xorg, yorg, xsize, ysize)
SPEEDO_GLOBALS *sp_global_ptr;
#else
FUNCTION void sp_open_bitmap(x_set_width, y_set_width, xorg, yorg, xsize, ysize)
#endif
fix31 x_set_width;
fix31 y_set_width;   /* Set width vector */
fix31 xorg;    /* Pixel boundary at left extent of bitmap character */
fix31 yorg;    /* Pixel boundary at right extent of bitmap character */
fix15 xsize;    /* Pixel boundary of bottom extent of bitmap character */
fix15 ysize;    /* Pixel boundary of top extent of bitmap character */
/* 
 * Called by Speedo character generator to initialize a buffer prior
 * to receiving bitmap data.
 */
{
fix15 i;

#if DEBUG
printf("open_bitmap(%3.1f, %3.1f, %3.1f, %3.1f, %d, %d)\n",
    (real)x_set_width / 65536.0, (real)y_set_width / 65536.0,
    (real)xorg / 65536.0, (real)yorg / 65536.0, (int)xsize, (int)ysize);
#endif
raswid = xsize;
rashgt = ysize;
offhor = (fix15)(xorg >> 16);
offver = (fix15)(yorg >> 16);

if (raswid > MAX_BITS)
    raswid = MAX_BITS;

printf("\nCharacter index = %d, ID = %d\n", char_index, char_id);
printf("set width = %3.1f, %3.1f\n", (real)x_set_width / 65536.0, (real)y_set_width / 65536.0);
printf("X offset  = %d\n", offhor);
printf("Y offset  = %d\n\n", offver);
for (i = 0; i < raswid; i++)
    {
    line_of_bits[i << 1] = '.';
    line_of_bits[(i << 1) + 1] = ' ';
    }
line_of_bits[raswid << 1] = '\0';
y_cur = 0;
}

#if REENTRANT_ALLOC
FUNCTION void sp_set_bitmap_bits (sp_global_ptr, y, xbit1, xbit2)
SPEEDO_GLOBALS *sp_global_ptr;
#else
FUNCTION void sp_set_bitmap_bits (y, xbit1, xbit2)
#endif
  fix15     y;     /* Scan line (0 = first row above baseline) */
  fix15     xbit1; /* Pixel boundary where run starts */
  fix15     xbit2; /* Pixel boundary where run ends */

/* 
 * Called by Speedo character generator to write one row of pixels 
 * into the generated bitmap character.                               
 */

{
fix15 i;

#if DEBUG
printf("set_bitmap_bits(%d, %d, %d)\n", (int)y, (int)xbit1, (int)xbit2);
#endif
/* Clip runs beyond end of buffer */
if (xbit1 > MAX_BITS)
    xbit1 = MAX_BITS;

if (xbit2 > MAX_BITS)
    xbit2 = MAX_BITS;

/* Output backlog lines if any */
while (y_cur != y)
    {
    printf("    %s\n", line_of_bits);
    for (i = 0; i < raswid; i++)
        {
        line_of_bits[i << 1] = '.';
        }
    y_cur++;
    }

/* Add bits to current line */
for (i = xbit1; i < xbit2; i++)
    {
    line_of_bits[i << 1] = 'X';
    }
}

#if REENTRANT_ALLOC
FUNCTION void sp_close_bitmap(sp_global_ptr)
SPEEDO_GLOBALS *sp_global_ptr;
#else
FUNCTION void sp_close_bitmap()
#endif
/* 
 * Called by Speedo character generator to indicate all bitmap data
 * has been generated.
 */
{
#if DEBUG
printf("close_bitmap()\n");
#endif
printf("    %s\n", line_of_bits);
}

#if INCL_OUTLINE
#if REENTRANT_ALLOC
FUNCTION void sp_open_outline(sp_global_ptr, x_set_width, y_set_width, xmin, xmax, ymin, ymax)
SPEEDO_GLOBALS *sp_global_ptr;
#else
FUNCTION void sp_open_outline(x_set_width, y_set_width, xmin, xmax, ymin, ymax)
#endif
fix31 x_set_width;
fix31 y_set_width;  /* Transformed escapement vector */
fix31  xmin;                           /* Minimum X value in outline */
fix31  xmax;                           /* Maximum X value in outline */
fix31  ymin;                           /* Minimum Y value in outline */
fix31  ymax;                           /* Maximum Y value in outline */
/*
 * Called by Speedo character generator to initialize prior to
 * outputting scaled outline data.
 */
{
printf("\nopen_outline(%3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f)\n",
    (real)x_set_width / 65536.0, (real)y_set_width / 65536.0,
    (real)xmin / 65536.0, (real)xmax / 65536.0, (real)ymin / 65536.0, (real)ymax / 65536.0);
}


#if REENTRANT_ALLOC
FUNCTION void sp_start_new_char(sp_global_ptr)
SPEEDO_GLOBALS *sp_global_ptr;
#else
FUNCTION void sp_start_new_char()
#endif
/*
 * Called by Speedo character generator to initialize prior to
 * outputting scaled outline data for a sub-character in a compound
 * character.
 */
{
printf("start_new_char()\n");
}

#if REENTRANT_ALLOC
FUNCTION void sp_start_contour(sp_global_ptr, x, y, outside)
SPEEDO_GLOBALS *sp_global_ptr;
#else
FUNCTION void sp_start_contour(x, y, outside)
#endif
fix31    x;       /* X coordinate of start point in 1/65536 pixels */
fix31    y;       /* Y coordinate of start point in 1/65536 pixels */
boolean outside;  /* TRUE if curve encloses ink (Counter-clockwise) */
/*
 * Called by Speedo character generator at the start of each contour
 * in the outline data of the character.
 */
{
printf("start_contour(%3.1f, %3.1f, %s)\n", 
    (real)x / 65536.0, (real)y / 65536.0, 
    outside? "outside": "inside");
}

#if REENTRANT_ALLOC
FUNCTION void sp_curve_to(sp_global_ptr, x1, y1, x2, y2, x3, y3)
SPEEDO_GLOBALS *sp_global_ptr;
#else
FUNCTION void sp_curve_to(x1, y1, x2, y2, x3, y3)
#endif
fix31 x1;               /* X coordinate of first control point in 1/65536 pixels */
fix31 y1;               /* Y coordinate of first control  point in 1/65536 pixels */
fix31 x2;               /* X coordinate of second control point in 1/65536 pixels */
fix31 y2;               /* Y coordinate of second control point in 1/65536 pixels */
fix31 x3;               /* X coordinate of curve end point in 1/65536 pixels */
fix31 y3;               /* Y coordinate of curve end point in 1/65536 pixels */
/*
 * Called by Speedo character generator onece for each curve in the
 * scaled outline data of the character. This function is only called if curve
 * output is enabled in the set_specs() call.
 */
{
printf("curve_to(%3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f)\n", 
    (real)x1 / 65536.0, (real)y1 / 65536.0,
    (real)x2 / 65536.0, (real)y2 / 65536.0,
    (real)x3 / 65536.0, (real)y3 / 65536.0);
}

#if REENTRANT_ALLOC
FUNCTION void sp_line_to(sp_global_ptr, x, y)
SPEEDO_GLOBALS *sp_global_ptr;
#else
FUNCTION void sp_line_to(x, y)
#endif
fix31 x;               /* X coordinate of vector end point in 1/65536 pixels */
fix31 y;               /* Y coordinate of vector end point in 1/65536 pixels */
/*
 * Called by Speedo character generator onece for each vector in the
 * scaled outline data for the character. This include curve data that has
 * been sub-divided into vectors if curve output has not been enabled
 * in the set_specs() call.
 */
{
printf("line_to(%3.1f, %3.1f)\n", 
    (real)x / 65536.0, (real)y / 65536.0);
}


#if REENTRANT_ALLOC
FUNCTION void sp_close_contour(sp_global_ptr)
SPEEDO_GLOBALS *sp_global_ptr;
#else
FUNCTION void sp_close_contour()
#endif
/*
 * Called by Speedo character generator at the end of each contour
 * in the outline data of the character.
 */
{
printf("close_contour()\n");
}

#if REENTRANT_ALLOC
FUNCTION void sp_close_outline(sp_global_ptr)
SPEEDO_GLOBALS *sp_global_ptr;
#else
FUNCTION void sp_close_outline()
#endif
/*
 * Called by Speedo character generator at the end of output of the
 * scaled outline of the character.
 */
{
printf("close_outline()\n");
}

#endif

FUNCTION fix15 read_2b(pointer)
ufix8 FONTFAR *pointer;
/*
 * Reads 2-byte field from font buffer 
 */
{
fix15 temp;

temp = *pointer++;
temp = (temp << 8) + *(pointer);
return temp;
}


FUNCTION fix31 read_4b(pointer)
ufix8 FONTFAR *pointer;
/*
 * Reads 4-byte field from font buffer 
 */
{
fix31 temp;

temp = *pointer++;
temp = (temp << 8) + *(pointer++);
temp = (temp << 8) + *(pointer++);
temp = (temp << 8) + *(pointer);
return temp;
}


