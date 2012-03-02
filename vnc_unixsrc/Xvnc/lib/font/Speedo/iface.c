/* $Xorg: iface.c,v 1.3 2000/08/17 19:46:25 cpqbld Exp $ */

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
/* $XFree86: xc/lib/font/Speedo/iface.c,v 1.3 2001/01/17 19:43:17 dawes Exp $ */

/***************************** I F A C E . C *********************************
 *                                                                           *
 * This module provides a layer to make Speedo function calls to and         *
 * from it compatible with Fontware 2.X function calls.                      *
 *                                                                           *
 ****************************************************************************/

#include "speedo.h"               /* General definitions for Speedo */
#ifndef FONTMODULE
#include <math.h>
#else
#include "xf86_ansic.h"
#endif

#define   DEBUG      0

#if DEBUG
#include <stdio.h>
#define SHOW(X) printf("X = %d\n", X)
#else
#define SHOW(X)
#endif

#define  PI     3.1415926536       /* pi */
#define  PTPERINCH   72.2892       /* nbr points per inch, exactly! */

#define  BIT8           0x0100
#define  BIT9           0x0200
#define  BIT10          0x0400
#define  BIT11          0x0800
#define  BIT12          0x1000
#define  BIT13          0x2000
#define  BIT14          0x4000
#define  BIT15          0x8000

#define   READ      0

typedef   short     bool16;
typedef   int       bool;

typedef
struct
    {
    bool16  left;
    bool16  right;
    bool16  top;
    bool16  bottom;
    }   lrtb;


typedef
struct
    {
    buff_t *pfont;              /* Pointer to font data                    */
    ufix16  mode;               /* what mode is the font generator in      */
    real    point_size_x;       /* Point size in X dimension               */
    real    point_size_y;       /* Point size in Y dimension               */
    real    res_hor;            /* Horizontal resolution of output device  */
    real    res_ver;            /* Vertical resolution of output device    */
    real    rot_angle;          /* Rotation angle in degrees (clockwise)   */
    real    obl_angle;          /* Obliquing angle in degrees (clockwise)  */
    bool16  whitewrite;         /* if T, generate bitmaps for whitewriters */
    fix15   thresh;             /* Scan conversion threshold               *
                                 * Thickens characters on each edge by     *
                                 * <thresh> sub-pixels                     */  
    bool16  import_widths;      /* Use imported width table                */
    lrtb    clip;               /* Clip to standard character cell         */
    lrtb    squeeze;            /* Squeeze to standard character cell      */
    bool16  bogus_mode;         /* if T, ignore plaid data                 */
    }   comp_char_desc;         /* character attributes for scan conv      */

/***** GLOBAL VARIABLES *****/

/*****  GLOBAL FUNCTIONS *****/
     fw_reset();                /* Fontware 2.X reset call                 */
     fw_set_specs();            /* Fontware 2.X set specs call             */
bool fw_make_char();            /* Fontware 2.X make character call        */
       
/***** EXTERNAL VARIABLES *****/

/***** EXTERNAL FUNCTIONS *****/
void    _open_bitmap();
void    _close_bitmap();
void    _set_bitmap_bits();
void    _open_outline();
void    _open_outline();
void    _start_new_char();
void    _start_curve();
void    _line_to();
void    _close_curve();
void    _close_outline();

/***** STATIC VARIABLES *****/
static specs_t *pspecs;
static buff_t  *pfont;
static buff_t   char_data;
static fix15    set_width_x;
static specs_t  specsarg;

/***** STATIC FUNCTIONS *****/
static fix31 make_mult();


FUNCTION fw_reset()
{
sp_reset();
}

FUNCTION fw_set_specs(pspecs)
comp_char_desc  *pspecs;  /* Pointer to scan conversion parameters structure */

/*  Fontware 2.X character generator call to set font specifications
 *  compc -- pointer to structure containing scan conversion parameters.
 *   ->compf -- compressed font data structure
 *   ->point_size_x -- x pointsize
 *   ->point_size_y -- y pointsize
 *   ->res_hor -- horizontal pixels per inch
 *   ->res_ver -- vertical pixels per inch
 *   ->rot_angle -- rotation angle in degrees (clockwise)
 *   ->obl_angle -- obliquing angle in degrees (clockwise)
 *   ->whitewrite -- if true, generate bitmaps for whitewriters
 *   ->thresh -- scan-conversion threshold
 *   ->import_widths -- if true, use external width table
 *   ->clip.left -- clips min x at left of emsquare
 *   ->clip.right -- clips max x at right of emsquare
 *   ->clip.bottom -- clips min x at bottom of emsquare
 *   ->clip.top -- clips max x at top of emsquare
 *   ->squeeze.left -- squeezes min x at left of emsquare
 *   ->squeeze.right, .top, .bottom  &c
 *   ->sw_fixed -- if TRUE, match pixel widths to scaled outline widths
 *   ->bogus_mode -- ignore plaid data if TRUE
 */

{
fix15   irot;
fix15   iobl;
fix15   x_trans_type;
fix15   y_trans_type;
fix31   xx_mult;
fix31   xy_mult;
fix31   yx_mult;
fix31   yy_mult;
real    sinrot, cosrot, tanobl;
real    x_distortion;
real    pixperem_h;
real    pixperem_v;
real    point_size_x;
real    point_size_y;
real    res_hor;
real    res_ver;
fix15   mode;

specsarg.pfont = pspecs->pfont;

irot = floor(pspecs->rot_angle + 0.5);
iobl = floor(pspecs->obl_angle + 0.5);
if (iobl > 85)
    iobl = 85;
if (iobl < -85)
    iobl = -85;
if ((irot % 90) == 0)
    {
    x_trans_type = y_trans_type = irot / 90 & 0x0003;
    if (iobl != 0)
        {
        if (x_trans_type & 0x01)
            y_trans_type = 4;
        else
            x_trans_type = 4;
        }
    }
else if (((irot + iobl) % 90) == 0)
    {
    x_trans_type = y_trans_type = (irot + iobl) / 90 & 0x0003;
    if (iobl != 0)
        {
        if (x_trans_type & 0x01)
            x_trans_type = 4;
        else
            y_trans_type = 4;
        }
    }
else
    {
    x_trans_type = y_trans_type = 4;
    }

point_size_x = pspecs->point_size_x;
point_size_y = pspecs->point_size_y;
res_hor = pspecs->res_hor;
res_ver = pspecs->res_ver;

switch (x_trans_type)
    {
case 0: 
    xx_mult = make_mult(point_size_x, res_hor);
    xy_mult = 0;
    break;

case 1:
    xx_mult = 0;
    xy_mult = make_mult(point_size_y, res_hor);
    break;

case 2: 
    xx_mult = -make_mult(point_size_x, res_hor);
    xy_mult = 0;
    break;

case 3:
    xx_mult = 0;
    xy_mult = -make_mult(point_size_y, res_hor);
    break;

default:
    sinrot = sin((real)irot * PI / 180.);
    cosrot = cos((real)irot * PI / 180.);
    tanobl = tan((real)iobl * PI / 180.);
    x_distortion = point_size_x / point_size_y;
    pixperem_h = point_size_y * res_hor / (real)PTPERINCH;   /* this is NOT a bug */
    xx_mult = floor(cosrot * x_distortion * pixperem_h * 65536.0 + 0.5);
    xy_mult = floor((sinrot + cosrot * tanobl) * pixperem_h * 65536.0 + 0.5);
    break;
    }

switch (y_trans_type)
    {
case 0: 
    yx_mult = 0;
    yy_mult = make_mult(point_size_y, res_ver);
    break;

case 1:
    yx_mult = -make_mult(point_size_x, res_hor);
    yy_mult = 0;
    break;

case 2: 
    yx_mult = 0;
    yy_mult = -make_mult(point_size_y, res_ver);
    break;

case 3:
    yx_mult = make_mult(point_size_x, res_ver);
    yy_mult = 0;
    break;

default:
    sinrot = sin((real)irot * PI / 180.);
    cosrot = cos((real)irot * PI / 180.);
    tanobl = tan((real)iobl * PI / 180.);
    x_distortion = point_size_x / point_size_y;
    pixperem_v = point_size_y * res_ver / (real)PTPERINCH;
    yx_mult = floor(-sinrot * x_distortion * pixperem_v * 65536.0 + 0.5);
    yy_mult = floor((cosrot - sinrot * tanobl) * pixperem_v * 65536.0 + 0.5);
    break;
    }

specsarg.xxmult = xx_mult;
specsarg.xymult = xy_mult;
specsarg.xoffset = 0;
specsarg.yxmult = yx_mult;
specsarg.yymult = yy_mult;
specsarg.yoffset = 0;
specsarg.out_info = 0;

/* Select processing mode */
switch (pspecs->mode)
    {
case 1:
    if (pspecs->whitewrite)           /* White-write requested? */
        {
        mode = 1;
        }
    else
        {
        mode = 0;
        }
    break;
    
case 2:
    mode = 2;
    break;

default:
    mode = pspecs->mode;
    break;
    }

if (pspecs->bogus_mode)        /* Linear transformation requested? */
    {
    mode |= BIT4;              /* Set linear tranformation flag */
    }

if (pspecs->import_widths)     /* Imported widths requested? */
    {
    mode |= BIT6;              /* Set imported width flag */
    }

if (pspecs->clip.left)         /* Clip left requested? */
    {
    mode |= BIT8;              /* Set clip left flag */
    }

if (pspecs->clip.right)        /* Clip right requested? */
    {
    mode |= BIT9;              /* Set clip right flag */
    }

if (pspecs->clip.top)          /* Clip top requested? */
    {
    mode |= BIT10;             /* Set clip top flag */
    }

if (pspecs->clip.bottom)       /* Clip bottom requested? */
    {
    mode |= BIT11;             /* Set clip bottom flag */
    }

if (pspecs->squeeze.left)      /* Squeeze left requested? */
    {
    mode |= BIT12;             /* Set squeeze left flag */
    }

if (pspecs->squeeze.right)     /* Squeeze right requested? */
    {
    mode |= BIT13;             /* Set squeeze right flag */
    }

if (pspecs->squeeze.top)       /* Squeeze top requested? */
    {
    mode |= BIT14;             /* Set squeeze top flag */
    }

if (pspecs->squeeze.bottom)    /* Squeeze bottom requested? */
    {
    mode |= BIT15;             /* Set squeeze bottom flag */
    }

specsarg.flags = mode;

sp_set_specs(&specsarg);
}

FUNCTION static fix31 make_mult(point_size, resolution)
real point_size;
real resolution;
{
real ms_factor;

return (fix31)floor((point_size * resolution * 65536.0) / (real)PTPERINCH + 0.5);
}

FUNCTION bool fw_make_char(char_index)
ufix16 char_index;
{
return sp_make_char(char_index);
}

FUNCTION buff_t *sp_load_char_data(file_offset, no_bytes, cb_offset)
fix31    file_offset;
fix15    no_bytes;
fix15    cb_offset;
/* 
 * Called by Speedo character generator to request that character
 * data be loaded from the font file.
 * This is a dummy function that assumes that the entire font has
 * been loaded.
 */
{
#if DEBUG
printf("load_char_data(%d, %d, %d)\n", file_offset, no_bytes, char_offset);
#endif
char_data.org = pfont->org + file_offset;
char_data.no_bytes = no_bytes;
return &char_data;
}

FUNCTION void sp_report_error(n)
fix15 n;        /* Error identification number */
/*
 * Called by Speedo character generator to report an error.
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
    printf("Character data not available()\n");
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



FUNCTION void sp_open_bitmap(sw_x, sw_y, xorg, yorg, xsize, ysize)
fix31  sw_x;                           /* X component of escapement vector */
fix31  sw_y;                           /* Y component of escapement vector */
fix31  xorg;                           /* X origin */
fix31  yorg;                           /* Y origin */
fix15 xsize;                           /* width of bitmap */
fix15 ysize;                           /* height of bitmap */
/* 
 * Called by Speedo character generator to initialize a buffer prior
 * to receiving bitmap data.
 */
{

fix15 xmin,xmax,ymin,ymax;

#if DEBUG
printf("sp_open_bitmap:\n");
printf("    X component of set width vector = %3.1f\n", (real)sw_x / 65536.0);
printf("    Y component of set width vector = %3.1f\n", (real)sw_y / 65536.0);
printf("    Bounding box is (%d, %d, %d, %d)\n", xmin, ymin, xmax, ymax);
#endif

xmin = xorg >> 16;
ymin = yorg >> 16;
xmax = xmin + xsize;
ymax = ymin + ysize;

set_width_x = ((sw_x >> 15) + 1) >> 1;
open_bitmap(set_width_x, xmin, xmax, ymin, ymax);
}

FUNCTION void sp_set_bitmap_bits(y, x1, x2)
fix15 y, x1, x2;
/* 
 * Called by Speedo character generator to write one row of pixels 
 * into the generated bitmap character.                               
 */
{
#if DEBUG
printf("set_bitmap_bits(%d, %d, %d)\n", y, x1, x2);
#endif

set_bitmap_bits(y, x1, x2);
}

FUNCTION void sp_close_bitmap()
/* 
 * Called by Speedo character generator to indicate all bitmap data
 * has been generated.
 */
{
#if DEBUG
printf("close_bitmap()\n");
#endif

close_bitmap();
}

FUNCTION void sp_open_outline(sw_x, sw_y, xmin, xmax, ymin, ymax)
fix31  sw_x;                           /* X component of escapement vector */
fix31  sw_y;                           /* Y component of escapement vector */
fix31  xmin;                           /* Minimum X value in outline */
fix31  xmax;                           /* Maximum X value in outline */
fix31  ymin;                           /* Minimum Y value in outline */
fix31  ymax;                           /* Maximum Y value in outline */
/*
 * Called by Speedo character generator to initialize prior to
 * outputting scaled outline data.
 */
{
#if DEBUG
printf("open_outline(%3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f)\n",
    (real)sw_x / 65536.0, (real)sw_y / 65536.0,
    (real)xmin / 65536.0, (real)xmax / 65536.0, (real)ymin / 65536.0, (real)ymax / 65536.0);
#endif

set_width_x = ((sw_x >> 15) + 1) >> 1;
open_outline(set_width_x);
}

FUNCTION void sp_start_new_char()
/*
 * Called by Speedo character generator to initialize prior to
 * outputting scaled outline data for a sub-character in a compound
 * character.
 */
{
#if DEBUG
printf("start_new_char()\n");
#endif

start_new_char();
}

FUNCTION void sp_start_contour(x, y, outside)
fix31    x;       /* X coordinate of start point in 1/65536 pixels */
fix31    y;       /* Y coordinate of start point in 1/65536 pixels */
boolean outside;  /* TRUE if curve encloses ink (Counter-clockwise) */
/*
 * Called by Speedo character generator at the start of each contour
 * in the outline data of the character.
 */
{
real realx, realy;

realx = (real)x / 65536.0;
realy = (real)y / 65536.0;

#if DEBUG
printf("start_curve(%3.1f, %3.1f, %s)\n", 
    realx, realy, 
    outside? "outside": "inside");
#endif

start_curve(realx, realy, outside);
}

FUNCTION void sp_curve_to(x1, y1, x2, y2, x3, y3)
fix31 x1;               /* X coordinate of first control point in 1/65536 pixels */
fix31 y1;               /* Y coordinate of first control  point in 1/65536 pixels */
fix31 x2;               /* X coordinate of second control point in 1/65536 pixels */
fix31 y2;               /* Y coordinate of second control point in 1/65536 pixels */
fix31 x3;               /* X coordinate of curve end point in 1/65536 pixels */
fix31 y3;               /* Y coordinate of curve end point in 1/65536 pixels */
/*
 * Called by Speedo character generator once for each curve in the
 * scaled outline data of the character. This function is only called if curve
 * output is enabled in the set_specs() call.
 */
{
#if DEBUG
printf("curve_to(%3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f)\n", 
    (real)x1 / 65536.0, (real)y1 / 65536.0,
    (real)x2 / 65536.0, (real)y2 / 65536.0,
    (real)x3 / 65536.0, (real)y3 / 65536.0);
#endif
}

FUNCTION void sp_line_to(x, y)
fix31 x;               /* X coordinate of vector end point in 1/65536 pixels */
fix31 y;               /* Y coordinate of vector end point in 1/65536 pixels */
/*
 * Called by Speedo character generator onece for each vector in the
 * scaled outline data for the character. This include curve data that has
 * been sub-divided into vectors if curve output has not been enabled
 * in the set_specs() call.
 */
{
real realx, realy;

realx = (real)x / 65536.0;
realy = (real)y / 65536.0;

#if DEBUG
printf("line_to(%3.1f, %3.1f)\n", 
    realx, realy); 
#endif            

line_to(realx, realy);
}

FUNCTION void sp_close_contour()
/*
 * Called by Speedo character generator at the end of each contour
 * in the outline data of the character.
 */
{
#if DEBUG
printf("close_curve()\n");
#endif

close_curve();
}

FUNCTION void sp_close_outline()
/*
 * Called by Speedo character generator at the end of output of the
 * scaled outline of the character.
 */
{
#if DEBUG
printf("close_outline()\n");
#endif

close_outline(set_width_x);
}

