/* $Xorg: set_spcs.c,v 1.3 2000/08/17 19:46:26 cpqbld Exp $ */

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
/* $XFree86: xc/lib/font/Speedo/set_spcs.c,v 1.4 2001/08/27 19:49:50 dawes Exp $ */


/*************************** S E T _ S P C S . C *****************************
 *                                                                           *
 * This module implements all sp_set_specs() functionality.                  *
 *                                                                           *
 ****************************************************************************/
#define SET_SPCS
#include "spdo_prv.h"               /* General definitions for Speedo    */
#include "keys.h"

#define   DEBUG      0

#if DEBUG
#include <stdio.h>
#define SHOW(X) printf("X = %d\n", X)
#else
#define SHOW(X)
#endif

/***** GLOBAL VARIABLES *****/

/***** GLOBAL FUNCTIONS *****/

/****** EXTERNAL VARIABLES *****/

/***** STATIC VARIABLES *****/


/****** STATIC FUNCTIONS *****/

static boolean sp_setup_consts(PROTO_DECL2 fix15 xmin, fix15 xmax,
	fix15 ymin, fix15 ymax);
static void sp_setup_tcb(PROTO_DECL2 tcb_t GLOBALFAR *ptcb);
static fix15 sp_setup_mult(PROTO_DECL2 fix31 input_mult);
static fix31 sp_setup_offset(PROTO_DECL2 fix31 input_offset);



FUNCTION boolean set_specs(
GDECL
specs_t STACKFAR *specsarg)     /* Bundle of conversion specifications */
/* 
 * Called by host software to set character generation specifications
 */
{
fix31   offcd;         /* Offset to start of character directory */
fix31   ofcns;         /* Offset to start of constraint data */ 
fix31   cd_size;       /* Size of character directory */
fix31   no_bytes_min;  /* Min number of bytes in font buffer */
ufix16  font_id;       /* Font ID */
ufix16  private_off;   /* offset to private header */
fix15   xmin;          /* Minimum X ORU value in font */
fix15   xmax;          /* Maximum X ORU value in font */
fix15   ymin;          /* Minimum Y ORU value in font */
fix15   ymax;          /* Maximum Y ORU value in font */

sp_globals.specs_valid = FALSE;           /* Flag specs not valid */

sp_globals.specs = *specsarg;   /* copy specs structure into sp_globals */
sp_globals.pspecs = &sp_globals.specs;
sp_globals.font = *sp_globals.pspecs->pfont;
sp_globals.pfont = &sp_globals.font;
sp_globals.font_org = sp_globals.font.org;

if (read_word_u(sp_globals.font_org + FH_FMVER + 4) != 0x0d0a)
    {
    report_error(4);           /* Font format error */
    return FALSE;
    }
if (read_word_u(sp_globals.font_org + FH_FMVER + 6) != 0x0000)
    {
    report_error(4);           /* Font format error */
    return FALSE;
    }

if (get_cust_no(*specsarg->pfont) == 0)
	{
	sp_globals.key32 = 0;
	sp_globals.key4 = 0;
	sp_globals.key6 = 0;
	sp_globals.key7 = 0;
	sp_globals.key8 = 0;
	}
else
	{
	sp_globals.key32 = (KEY3 << 8) | KEY2;
	sp_globals.key4 = KEY4;
	sp_globals.key6 = KEY6;
	sp_globals.key7 = KEY7;
	sp_globals.key8 = KEY8;
	}
	

sp_globals.no_chars_avail = read_word_u(sp_globals.font_org + FH_NCHRF);

/* Read sp_globals.orus per em from font header */
sp_globals.orus_per_em = read_word_u(sp_globals.font_org + FH_ORUPM);

/* compute address of private header */
private_off = read_word_u(sp_globals.font_org + FH_HEDSZ);
sp_globals.hdr2_org = sp_globals.font_org + private_off;

/* set metric resolution if specified, default to outline res otherwise */
if (private_off > EXP_FH_METRES)
	{
	sp_globals.metric_resolution = read_word_u(sp_globals.font_org + EXP_FH_METRES);
	}
else
	{
	sp_globals.metric_resolution = sp_globals.orus_per_em;
	}

#if INCL_METRICS
sp_globals.kern.tkorg = sp_globals.font_org + read_long(sp_globals.hdr2_org + FH_OFFTK);
sp_globals.kern.pkorg = sp_globals.font_org + read_long(sp_globals.hdr2_org + FH_OFFPK); 
sp_globals.kern.no_tracks = read_word_u(sp_globals.font_org + FH_NKTKS);
sp_globals.kern.no_pairs = read_word_u(sp_globals.font_org + FH_NKPRS);
#endif

offcd = read_long(sp_globals.hdr2_org + FH_OFFCD); /* Read offset to character directory */
ofcns = read_long(sp_globals.hdr2_org + FH_OFCNS); /* Read offset to constraint data */
cd_size = ofcns - offcd;
if ((((sp_globals.no_chars_avail << 1) + 3) != cd_size) &&
    (((sp_globals.no_chars_avail * 3) + 4) != cd_size))
    {
    report_error(4);           /* Font format error */
    return FALSE;
    }

#if INCL_LCD                   /* Dynamic character data load suppoorted? */
#if INCL_METRICS
no_bytes_min = read_long(sp_globals.hdr2_org + FH_OCHRD); /* Offset to character data */
#else                          /* Dynamic character data load not supported? */
no_bytes_min = read_long(sp_globals.hdr2_org + FH_OFFTK); /* Offset to track kerning data */
#endif
#else                          /* Dynamic character data load not supported? */
no_bytes_min = read_long(sp_globals.hdr2_org + FH_NBYTE); /* Offset to EOF + 1 */
#endif

sp_globals.font_buff_size = sp_globals.pfont->no_bytes;
if (sp_globals.font_buff_size < no_bytes_min)  /* Minimum data not loaded? */
    {
    report_error(1);           /* Insufficient font data loaded */
    return FALSE;
    }

sp_globals.pchar_dir = sp_globals.font_org + offcd;
sp_globals.first_char_idx = read_word_u(sp_globals.font_org + FH_FCHRF);

/* Register font name with sp_globals.constraint mechanism */
#if INCL_RULES
font_id = read_word_u(sp_globals.font_org + FH_FNTID);
if (!(sp_globals.constr.font_id_valid) || (sp_globals.constr.font_id != font_id))
    {
    sp_globals.constr.font_id = font_id;
    sp_globals.constr.font_id_valid = TRUE;
    sp_globals.constr.data_valid = FALSE;
    }
sp_globals.constr.org = sp_globals.font_org + ofcns;
sp_globals.constr.active = ((sp_globals.pspecs->flags & CONSTR_OFF) == 0);
#endif

/* Set up sliding point constants */
/* Set pixel shift to accomodate largest transformed pixel value */
xmin = read_word_u(sp_globals.font_org + FH_FXMIN);
xmax = read_word_u(sp_globals.font_org + FH_FXMAX);
ymin = read_word_u(sp_globals.font_org + FH_FYMIN);
ymax = read_word_u(sp_globals.font_org + FH_FYMAX);

if (!sp_setup_consts(xmin,xmax,ymin,ymax))
    {
    report_error(3);           /* Requested specs out of range */
    return FALSE;
    }
#if INCL_ISW
/* save the value of the max x oru that the fixed point constants are based on*/
sp_globals.isw_xmax = xmax; 
#endif

/* Setup transformation control block */
sp_setup_tcb(&sp_globals.tcb0);


/* Select output module */
sp_globals.output_mode = sp_globals.pspecs->flags & 0x0007;

#if INCL_USEROUT
if (!init_userout(sp_globals.pspecs))
#endif

switch (sp_globals.output_mode)
    {
#if INCL_BLACK
case MODE_BLACK:                        /* Output mode 0 (Black writer) */
	sp_globals.init_out = sp_init_black;
    sp_globals.begin_char		= sp_begin_char_black;
    sp_globals.begin_sub_char	= sp_begin_sub_char_out;
   	sp_globals.begin_contour	= sp_begin_contour_black;
    sp_globals.curve			= sp_curve_out;
   	sp_globals.line			= sp_line_black;
    sp_globals.end_contour		= sp_end_contour_out;
   	sp_globals.end_sub_char	= sp_end_sub_char_out;
    sp_globals.end_char		= sp_end_char_black;
    break;
#endif

#if INCL_SCREEN
case MODE_SCREEN:                        /* Output mode 1 (Screen writer) */
	sp_globals.init_out = sp_init_screen;
    sp_globals.begin_char		= sp_begin_char_screen;
    sp_globals.begin_sub_char	= sp_begin_sub_char_out;
   	sp_globals.begin_contour	= sp_begin_contour_screen;
    sp_globals.curve			= sp_curve_screen;
   	sp_globals.line			= sp_line_screen;
    sp_globals.end_contour		= sp_end_contour_screen;
   	sp_globals.end_sub_char	= sp_end_sub_char_out;
    sp_globals.end_char		= sp_end_char_screen;
	break;
#endif

#if INCL_OUTLINE
case MODE_OUTLINE:                        /* Output mode 2 (Vector) */
	sp_globals.init_out = sp_init_outline;
    sp_globals.begin_char		= sp_begin_char_outline;
    sp_globals.begin_sub_char	= sp_begin_sub_char_outline;
   	sp_globals.begin_contour	= sp_begin_contour_outline;
    sp_globals.curve			= sp_curve_outline;
   	sp_globals.line			= sp_line_outline;
    sp_globals.end_contour		= sp_end_contour_outline;
   	sp_globals.end_sub_char	= sp_end_sub_char_outline;
    sp_globals.end_char		= sp_end_char_outline;
	break;
#endif

#if INCL_2D
case MODE_2D:                        /* Output mode 3 */
	sp_globals.init_out = sp_init_2d;
    sp_globals.begin_char		= sp_begin_char_2d;
    sp_globals.begin_sub_char	= sp_begin_sub_char_out;
   	sp_globals.begin_contour	= sp_begin_contour_2d;
    sp_globals.curve			= sp_curve_out;
   	sp_globals.line			= sp_line_2d;
    sp_globals.end_contour		= sp_end_contour_out;
   	sp_globals.end_sub_char	= sp_end_sub_char_out;
    sp_globals.end_char		= sp_end_char_2d;
    break;
#endif

default:
    report_error(8);           /* Unsupported mode requested */
    return FALSE;
    }

	if (!fn_init_out(sp_globals.pspecs))
		{
		report_error(5);
		return FALSE;
		}
		

sp_globals.curves_out = sp_globals.pspecs->flags & CURVES_OUT;

if (sp_globals.pspecs->flags & BOGUS_MODE) /* Linear transformation requested? */
    {
    sp_globals.tcb0.xtype = sp_globals.tcb0.ytype = 4;
    }
else                           /* Intelligent transformation requested? */
    {
#if INCL_RULES
#else
    report_error(7);           /* Rules requested; not supported */
    return FALSE;
#endif
    }

if ((sp_globals.pspecs->flags & SQUEEZE_LEFT) ||
    (sp_globals.pspecs->flags & SQUEEZE_RIGHT) ||
    (sp_globals.pspecs->flags & SQUEEZE_TOP) ||
    (sp_globals.pspecs->flags & SQUEEZE_BOTTOM) )
    {
#if (INCL_SQUEEZING)
#else
     report_error(11);
     return FALSE;
#endif
    }

if ((sp_globals.pspecs->flags & CLIP_LEFT) ||
    (sp_globals.pspecs->flags & CLIP_RIGHT) ||
    (sp_globals.pspecs->flags & CLIP_TOP) ||
    (sp_globals.pspecs->flags & CLIP_BOTTOM) )
    {
#if (INCL_CLIPPING)
#else
     report_error(11);
     return FALSE;
#endif
    }

sp_globals.specs_valid = TRUE;
return TRUE;
}



#if INCL_MULTIDEV
#if INCL_BLACK || INCL_SCREEN || INCL_2D
FUNCTION boolean set_bitmap_device(
GDECL
bitmap_t *bfuncs,
ufix16 size)
{

if (size != sizeof(sp_globals.bitmap_device))
	return FALSE;

sp_globals.bitmap_device = *bfuncs;
sp_globals.bitmap_device_set = TRUE;
}
#endif

#if INCL_OUTLINE
FUNCTION boolean set_outline_device(
GDECL
outline_t *ofuncs,
ufix16 size)
{

if (size != sizeof(sp_globals.outline_device))
	return FALSE;

sp_globals.outline_device = *ofuncs;
sp_globals.outline_device_set = TRUE;
}
#endif
#endif


#ifdef old
FUNCTION boolean sp_setup_consts(
GDECL
fix15   xmin,          /* Minimum X ORU value in font */
fix15   xmax,          /* Maximum X ORU value in font */
fix15   ymin,          /* Minimum Y ORU value in font */
fix15   ymax)          /* Maximum Y ORU value in font */
#else
static FUNCTION boolean sp_setup_consts(
GDECL
fix15   xmin,          /* Minimum X ORU value in font */
fix15   xmax,          /* Maximum X ORU value in font */
fix15   ymin,          /* Minimum Y ORU value in font */
fix15   ymax)          /* Maximum Y ORU value in font */
#endif
/*
 * Sets the following constants used for fixed point arithmetic:
 *      sp_globals.multshift    multipliers and products; range is 14 to 8
 *      sp_globals.pixshift     pixels: range is 0 to 8
 *      sp_globals.mpshift      shift from product to sub-pixels (sp_globals.multshift - sp_globals.pixshift)
 *      sp_globals.multrnd      rounding for products
 *      sp_globals.pixrnd       rounding for pixels
 *      sp_globals.mprnd        rounding for sub-pixels
 *      sp_globals.onepix       1 pixel in shifted pixel units
 *      sp_globals.pixfix       mask to eliminate fractional bits of shifted pixels
 *      sp_globals.depth_adj    curve splitting depth adjustment
 * Returns FALSE if specs are out of range
 */
{
fix31   mult;          /* Successive multiplier values */
ufix32  num;           /* Numerator of largest multiplier value */
ufix32  numcopy;       /* Copy of numerator */
ufix32  denom;         /* Denominator of largest multiplier value */
ufix32  denomcopy;     /* Copy of denominator */
ufix32  pix_max;       /* Maximum pixel rounding error */
fix31   xmult;         /* Coefficient of X oru value in transformation */
fix31   ymult;         /* Coefficient of Y oru value in transformation */
fix31   offset;        /* Constant in transformation */
fix15   i;             /* Loop counter */
fix15   x, y;          /* Successive corners of bounding box in ORUs */
fix31   pixval;        /* Successive pixel values multiplied by orus per em */
fix15   xx = 0, yy = 0;/* Bounding box corner that produces max pixel value */

/* Determine numerator and denominator of largest multiplier value */
mult = sp_globals.pspecs->xxmult >> 16;
if (mult < 0)
    mult = -mult;
num = mult;

mult = sp_globals.pspecs->xymult >> 16;
if (mult < 0)
    mult = -mult;
if (mult > num)
    num = mult;

mult = sp_globals.pspecs->yxmult >> 16;
if (mult < 0)
    mult = -mult;
if (mult > num)
    num = mult;

mult = sp_globals.pspecs->yymult >> 16;
if (mult < 0)
    mult = -mult;
if (mult > num)
    num = mult;
num++;                 /* Max absolute pixels per em (rounded up) */
denom = (ufix32)sp_globals.orus_per_em;

/* Set curve splitting depth adjustment to accomodate largest multiplier value */
sp_globals.depth_adj = 0;   /* 0 = 0.5 pel, 1 = 0.13 pel, 2 = 0.04 pel accuracy */
denomcopy = denom;
/*  The following two occurances of a strange method of shifting twice by 1 
    are intentional and should not be changed to a single shift by 2.  
    It prevents MicroSoft C 5.1 from generating functions calls to do the shift.  
    Worse, using the REENTRANT_ALLOC option in conjunction with the /AC compiler 
    option, the function appears to be called incorrectly, causing depth_adj to always
	be set to -7, causing very angular characters. */

while ((num > denomcopy) && (sp_globals.depth_adj < 5)) /* > 1, 4, 16, ...  pixels per oru? */
    {
    denomcopy <<= 1;
    denomcopy <<= 1;
    sp_globals.depth_adj++; /* Add 1, 2, 3, ... to depth adjustment */
    }
numcopy = num << 2;
while ((numcopy <= denom) && (sp_globals.depth_adj > -4))  /* <= 1/4, 1/16, 1/64 pix per oru? */
    {
    numcopy <<= 1;
    numcopy <<= 1;
    sp_globals.depth_adj--; /* Subtract 1, 2, 3, ... from depth adjustment */
    }
SHOW(sp_globals.depth_adj);

/* Set multiplier shift to accomodate largest multiplier value */
sp_globals.multshift = 14;            
numcopy = num;
while (numcopy >= denom)     /* More than 1, 2, 4, ... pix per oru? */
    {
    numcopy >>= 1;
    sp_globals.multshift--; /* sp_globals.multshift is 13, 12, 11, ... */
    }

sp_globals.multrnd = ((fix31)1 << sp_globals.multshift) >> 1;
SHOW(sp_globals.multshift);


pix_max = (ufix32)( 0xffff & read_word_u(sp_globals.hdr2_org + FH_PIXMX));

num = 0;
xmult = ((sp_globals.pspecs->xxmult >> 16) + 1) >> 1;
ymult = ((sp_globals.pspecs->xymult >> 16) + 1) >> 1;
offset = ((sp_globals.pspecs->xoffset >> 16) + 1) >> 1;
for (i = 0; i < 8; i++)
    {
    if (i == 4)
        {
        xmult = ((sp_globals.pspecs->yxmult >> 16) + 1) >> 1;
        ymult = ((sp_globals.pspecs->yymult >> 16) + 1) >> 1;
        offset = ((sp_globals.pspecs->yoffset >> 16) + 1) >> 1;
        }
    x = (i & BIT1)? xmin: xmax;
    y = (i & BIT0)? ymin: ymax;
    pixval = (fix31)x * xmult + (fix31)y * ymult + offset * denom;
    if (pixval < 0)
        pixval = -pixval;
    if (pixval > num)
        {
        num = pixval;
        xx = x;
        yy = y;
        }
    }
if (xx < 0)
    xx = -xx;
if (yy < 0)
    yy = -yy;
num += xx + yy + ((pix_max + 2) * denom); 
                                  /* Allow (with 2:1 safety margin) for 1 pixel rounding errors in */
                                  /* xmult, ymult and offset values, pix_max pixel expansion */
                                  /* due to intelligent scaling, and */
                                  /* 1 pixel rounding of overall character position */
denom = denom << 14;              /* Note num is in units of half pixels times orus per em */

sp_globals.pixshift = -1;
while ((num <= denom) && (sp_globals.pixshift < 8))  /* Max pixels <= 32768, 16384, 8192, ... pixels? */
    {
    num <<= 1;
    sp_globals.pixshift++;        /* sp_globals.pixshift = 0, 1, 2, ... */
    }
if (sp_globals.pixshift < 0)
    return FALSE;

SHOW(sp_globals.pixshift);
sp_globals.poshift = 16 - sp_globals.pixshift;

sp_globals.onepix = (fix15)1 << sp_globals.pixshift;
sp_globals.pixrnd = sp_globals.onepix >> 1;
sp_globals.pixfix = ~0 << sp_globals.pixshift;

sp_globals.mpshift = sp_globals.multshift - sp_globals.pixshift;
if (sp_globals.mpshift < 0)
    return FALSE;
sp_globals.mprnd = ((fix31)1 << sp_globals.mpshift) >> 1;

return TRUE;
}

#ifdef old
FUNCTION void sp_setup_tcb(
GDECL
tcb_t GLOBALFAR *ptcb)           /* Pointer to transformation control bloxk */
#else
static FUNCTION void sp_setup_tcb(
GDECL
tcb_t GLOBALFAR *ptcb)           /* Pointer to transformation control bloxk */
#endif
/* 
 * Convert transformation coeffs to internal form 
 */
{

ptcb->xxmult = sp_setup_mult(sp_globals.pspecs->xxmult);
ptcb->xymult = sp_setup_mult(sp_globals.pspecs->xymult);
ptcb->xoffset = sp_setup_offset(sp_globals.pspecs->xoffset);
ptcb->yxmult = sp_setup_mult(sp_globals.pspecs->yxmult);
ptcb->yymult = sp_setup_mult(sp_globals.pspecs->yymult);
ptcb->yoffset = sp_setup_offset(sp_globals.pspecs->yoffset);

SHOW(ptcb->xxmult);
SHOW(ptcb->xymult);
SHOW(ptcb->xoffset);
SHOW(ptcb->yxmult);
SHOW(ptcb->yymult);
SHOW(ptcb->yoffset);

type_tcb(ptcb); /* Classify transformation type */
}

FUNCTION static fix15 sp_setup_mult(
GDECL
fix31   input_mult)    /* Multiplier in input format */
/*
 * Called by sp_setup_tcb() to convert multiplier in transformation
 * matrix from external to internal form.
 */
{
fix15   imshift;       /* Right shift to internal format */
fix31   imdenom;       /* Divisor to internal format */
fix31   imrnd;         /* Rounding for division operation */

imshift = 15 - sp_globals.multshift;
imdenom = (fix31)sp_globals.orus_per_em << imshift;
imrnd = imdenom >> 1;

input_mult >>= 1;
if (input_mult >= 0)
    return (fix15)((input_mult + imrnd) / imdenom);
else
    return -(fix15)((-input_mult + imrnd) / imdenom);
}

FUNCTION static fix31 sp_setup_offset(
GDECL
fix31   input_offset)   /* Multiplier in input format */
/*
 * Called by sp_setup_tcb() to convert offset in transformation
 * matrix from external to internal form.
 */
{
fix15   imshift;       /* Right shift to internal format */
fix31   imrnd;         /* Rounding for right shift operation */

imshift = 15 - sp_globals.multshift;
imrnd = ((fix31)1 << imshift) >> 1;

return (((input_offset >> 1) + imrnd) >> imshift) + sp_globals.mprnd;
}

FUNCTION void type_tcb(
GDECL
tcb_t GLOBALFAR *ptcb)           /* Pointer to transformation control bloxk */
{
fix15   x_trans_type;
fix15   y_trans_type;
fix15   xx_mult;
fix15   xy_mult;
fix15   yx_mult;
fix15   yy_mult;
fix15   h_pos;
fix15   v_pos;
fix15   x_ppo;
fix15   y_ppo;
fix15   x_pos;
fix15   y_pos;

/* check for mirror image transformations */
xx_mult = ptcb->xxmult;
xy_mult = ptcb->xymult;
yx_mult = ptcb->yxmult;
yy_mult = ptcb->yymult;

ptcb->mirror = ((((fix31)xx_mult*(fix31)yy_mult)-
                     ((fix31)xy_mult*(fix31)yx_mult)) < 0) ? -1 : 1;

if (sp_globals.pspecs->flags & BOGUS_MODE) /* Linear transformation requested? */
    {
    ptcb->xtype = 4;
    ptcb->ytype = 4;

    ptcb->xppo = 0;
    ptcb->yppo = 0;
    ptcb->xpos = 0;
    ptcb->ypos = 0;
    }
else                            /* Intelligent tranformation requested? */
    {
    h_pos = ((ptcb->xoffset >> sp_globals.mpshift) + sp_globals.pixrnd) & sp_globals.pixfix;
    v_pos = ((ptcb->yoffset >> sp_globals.mpshift) + sp_globals.pixrnd) & sp_globals.pixfix;

    x_trans_type = 4;
    x_ppo = 0;
    x_pos = 0;

    y_trans_type = 4;
    y_ppo = 0;
    y_pos = 0;

    if (xy_mult == 0)
        {
        if (xx_mult >= 0)
            {
            x_trans_type = 0;   /* X pix is function of X orus only */
            x_ppo = xx_mult;
            x_pos = h_pos;
            }
        else 
            {
            x_trans_type = 1;   /* X pix is function of -X orus only */
            x_ppo = -xx_mult;
            x_pos = -h_pos;
            }
        }

    else if (xx_mult == 0)
        {
        if (xy_mult >= 0)
            {
            x_trans_type = 2;   /* X pix is function of Y orus only */
            y_ppo = xy_mult;
            y_pos = h_pos;
            }
        else 
            {
            x_trans_type = 3;   /* X pix is function of -Y orus only */
            y_ppo = -xy_mult;
            y_pos = -h_pos;
            }
        }

    if (yx_mult == 0)
        {
        if (yy_mult >= 0)
            {
            y_trans_type = 0;   /* Y pix is function of Y orus only */
            y_ppo = yy_mult;
            y_pos = v_pos;
            }
        else 
            {
            y_trans_type = 1;   /* Y pix is function of -Y orus only */
            y_ppo = -yy_mult;
            y_pos = -v_pos;
            }
        }
    else if (yy_mult == 0)
        {
        if (yx_mult >= 0)
            {
            y_trans_type = 2;   /* Y pix is function of X orus only */
            x_ppo = yx_mult;
            x_pos = v_pos;
            }
        else 
            {
            y_trans_type = 3;   /* Y pix is function of -X orus only */
            x_ppo = -yx_mult;
            x_pos = -v_pos;
            }
        }

    ptcb->xtype = x_trans_type;
    ptcb->ytype = y_trans_type;

    ptcb->xppo = x_ppo;
    ptcb->yppo = y_ppo;
    ptcb->xpos = x_pos;
    ptcb->ypos = y_pos;
    }

sp_globals.normal = (ptcb->xtype != 4) && (ptcb->ytype != 4);

ptcb->xmode = 4;
ptcb->ymode = 4;   

SHOW(ptcb->xtype);
SHOW(ptcb->ytype);
SHOW(ptcb->xppo);
SHOW(ptcb->yppo);
SHOW(ptcb->xpos);
SHOW(ptcb->ypos);
}

FUNCTION fix31 read_long(
GDECL
ufix8 FONTFAR *pointer)    /* Pointer to first byte of encrypted 3-byte integer */
/*
 * Reads a 3-byte encrypted integer from the byte string starting at 
 * the specified point.
 * Returns the decrypted value read as a signed integer.
 */
{
fix31 tmpfix31;

tmpfix31 = (fix31)((*pointer++) ^ sp_globals.key4) << 8;            /* Read middle byte */
tmpfix31 += (fix31)(*pointer++) << 16;                              /* Read most significant byte */
tmpfix31 += (fix31)((*pointer) ^ sp_globals.key6);                    /* Read least significant byte */
return tmpfix31;
}

FUNCTION fix15 read_word_u(
GDECL
ufix8 FONTFAR *pointer)    /* Pointer to first byte of unencrypted 2-byte integer */
/*
 * Reads a 2-byte unencrypted integer from the byte string starting at 
 * the specified point.
 * Returns the decrypted value read as a signed integer.
 */
{
fix15 tmpfix15;

tmpfix15 = (fix15)(*pointer++) << 8;                                /* Read most significant byte */
tmpfix15 += (fix15)(*pointer);                                        /* Add least significant byte */
return tmpfix15;
}


