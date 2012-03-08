/* $Xorg: do_trns.c,v 1.3 2000/08/17 19:46:25 cpqbld Exp $ */

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
/* $XFree86: xc/lib/font/Speedo/do_trns.c,v 1.4 2001/08/27 19:49:50 dawes Exp $ */

/**************************** D O _ T R N S . C ******************************
 *                                                                           *
 * This module is responsible for executing all intelligent transformation   *
 * for bounding box and outline data                                         *
 *                                                                           *
 ****************************************************************************/


#include "spdo_prv.h"               /* General definitions for Speedo    */

#define   DEBUG      0

#if DEBUG
#include <stdio.h>
#define SHOW(X) printf("X = %d\n", X)
#else
#define SHOW(X)
#endif

/***** GLOBAL VARIABLES *****/

/***** GLOBAL FUNCTIONS *****/

/***** EXTERNAL VARIABLES *****/

/***** EXTERNAL FUNCTIONS *****/

/***** STATIC VARIABLES *****/

/***** STATIC FUNCTIONS *****/

static void sp_split_curve(PROTO_DECL2 point_t P1,point_t P2,point_t P3,fix15 depth);
static ufix8 FONTFAR *sp_get_args(PROTO_DECL2 ufix8 FONTFAR *pointer,ufix8  format,point_t STACKFAR *pP);


FUNCTION ufix8 FONTFAR *read_bbox(
GDECL
ufix8 FONTFAR *pointer,    /* Pointer to next byte in char data */
point_t STACKFAR *pPmin,      /* Lower left corner of bounding box */
point_t STACKFAR *pPmax,      /* Upper right corner of bounding box */
boolean set_flag) /* flag to indicate whether global oru bbox should be saved */
/*
 * Called by make_simp_char() and make_comp_char() to read the 
 * bounding box data from the font.
 * Sets Pmin and Pmax to the bottom left and top right corners
 * of the bounding box after transformation into device space.
 * The units of Pmin and Pmax are sub-pixels.
 * Updates *ppointer to point to the byte following the
 * bounding box data.
 */
{
ufix8    format1;
ufix8    format = 0;
fix15    i;
point_t  P;

sp_globals.x_int = 0;
sp_globals.y_int = sp_globals.Y_int_org;
sp_globals.x_orus = sp_globals.y_orus = 0;
format1 = NEXT_BYTE(pointer);
pointer = sp_get_args(pointer, format1, pPmin);
#if INCL_SQUEEZING || INCL_ISW
if (set_flag)
    {
    sp_globals.bbox_xmin_orus = sp_globals.x_orus;
    sp_globals.bbox_ymin_orus = sp_globals.y_orus;
    }
#endif
*pPmax = *pPmin;
for (i = 1; i < 4; i++)
    {
    switch(i)
        {
    case 1:
        if (format1 & BIT6)            /* Xmax requires X int zone 1? */
            sp_globals.x_int++;
        format = (format1 >> 4) | 0x0c;
        break;

    case 2:
        if (format1 & BIT7)            /* Ymax requires Y int zone 1? */
            sp_globals.y_int++;
        format = NEXT_BYTE(pointer);
        break;

    case 3:
        sp_globals.x_int = 0; 
        format >>= 4;
        break;

    default:
		break;
        }

    pointer = sp_get_args(pointer, format, &P);
#if INCL_SQUEEZING || INCL_ISW
    if (set_flag && (i==2))
	{
	sp_globals.bbox_xmax_orus = sp_globals.x_orus;
	sp_globals.bbox_ymax_orus = sp_globals.y_orus;
	}
#endif
    if ((i == 2) || (!sp_globals.normal)) 
        {
        if (P.x < pPmin->x)
            pPmin->x = P.x;
        if (P.y < pPmin->y)
            pPmin->y = P.y;
        if (P.x > pPmax->x)
            pPmax->x = P.x;
        if (P.y > pPmax->y)
            pPmax->y = P.y;
        }
    }

#if DEBUG
printf("BBOX %6.1f(Xint 0), %6.1f(Yint 0), %6.1f(Xint %d), %6.1f(Yint %d)\n",
    (real)pPmin->x / (real)sp_globals.onepix, 
    (real)pPmin->y / (real)sp_globals.onepix, 
    (real)pPmax->x / (real)sp_globals.onepix, 
    (format1 >> 6) & 0x01,
    (real)pPmax->y / (real)sp_globals.onepix,
    (format1 >> 7) & 0x01);

#endif
return pointer;
}

FUNCTION void proc_outl_data(
GDECL
ufix8 FONTFAR *pointer)      /* Pointer to next byte in char data */
/*
 * Called by make_simp_char() and make_comp_char() to read the 
 * outline data from the font.
 * The outline data is parsed, transformed into device coordinates
 * and passed to an output module for further processing.
 * Note that pointer is not updated to facilitate repeated
 * processing of the outline data when banding mode is in effect.
 */
{
ufix8    format1, format2;
point_t  P0, P1, P2, P3;
fix15    depth;
fix15    curve_count;

sp_globals.x_int = 0;
sp_globals.y_int = sp_globals.Y_int_org;
#if INCL_PLAID_OUT                 /* Plaid data monitoring included? */
record_xint((fix15)sp_globals.x_int);         /* Record xint data */
record_yint((fix15)(sp_globals.y_int - sp_globals.Y_int_org)); /* Record yint data */
#endif

sp_globals.x_orus = sp_globals.y_orus = 0;
curve_count = 0;
while(TRUE)
    {
    format1 = NEXT_BYTE(pointer);
    switch(format1 >> 4)
        {
    case 0:                        /* LINE */
        pointer = sp_get_args(pointer, format1, &P1);
#if DEBUG
        printf("LINE %6.1f, %6.1f\n",
            (real)P1.x / (real)sp_globals.onepix, (real)P1.y / (real)sp_globals.onepix);
#endif
        fn_line(P1);
        sp_globals.P0 = P1;
        continue;

    case 1:                        /* Short XINT */
        sp_globals.x_int = format1 & 0x0f;
#if DEBUG
        printf("XINT %d\n", sp_globals.x_int);
#endif
#if INCL_PLAID_OUT                 /* Plaid data monitoring included? */
record_xint((fix15)sp_globals.x_int);         /* Record xint data */
#endif
        continue;

    case 2:                        /* Short YINT */
        sp_globals.y_int = sp_globals.Y_int_org + (format1 & 0x0f);
#if DEBUG
        printf("YINT %d\n", sp_globals.y_int - sp_globals.Y_int_org);
#endif
#if INCL_PLAID_OUT                 /* Plaid data monitoring included? */
record_yint((fix15)(sp_globals.y_int - sp_globals.Y_int_org)); /* Record yint data */
#endif
        continue;
         
    case 3:                        /* Miscellaneous */
        switch(format1 & 0x0f)
            {
        case 0:                    /* END */
            if (curve_count)
                {
                fn_end_contour();
                }
            return;

        case 1:                     /* Long XINT */
            sp_globals.x_int = NEXT_BYTE(pointer);
#if DEBUG
            printf("XINT %d\n", sp_globals.x_int);
#endif
#if INCL_PLAID_OUT                 /* Plaid data monitoring included? */
record_xint((fix15)sp_globals.x_int);         /* Record xint data */
#endif
            continue;

        case 2:                     /* Long YINT */
            sp_globals.y_int = sp_globals.Y_int_org + NEXT_BYTE(pointer);
#if DEBUG
            printf("YINT %d\n", sp_globals.y_int - sp_globals.Y_int_org);
#endif
#if INCL_PLAID_OUT                 /* Plaid data monitoring included? */
record_yint((fix15)(sp_globals.y_int - sp_globals.Y_int_org)); /* Record yint data */
#endif
            continue;

        default:                    /* Not used */
            continue;
            }    

    case 4:                         /* MOVE Inside */
    case 5:                         /* MOVE Outside */
        if (curve_count++)
            {
            fn_end_contour();
            }                                
		
        pointer = sp_get_args(pointer, format1, &P0);
		sp_globals.P0 = P0;
#if DEBUG
        printf("MOVE %6.1f, %6.1f\n",
            (real)sp_globals.P0.x / (real)sp_globals.onepix, (real)sp_globals.P0.y / (real)sp_globals.onepix);
#endif
        fn_begin_contour(sp_globals.P0, (boolean)(format1 & BIT4));
        continue;

    case 6:                         /* Undefined */
#if DEBUG
        printf("*** Undefined instruction (Hex %4x)\n", format1);
#endif
        continue;

    case 7:                         /* Undefined */
#if DEBUG
        printf("*** Undefined instruction (Hex %4x)\n", format1);
#endif
        continue;

    default:                        /* CRVE */
        format2 = NEXT_BYTE(pointer);
        pointer = sp_get_args(pointer, format1, &P1);
        pointer = sp_get_args(pointer, format2, &P2);
        pointer = sp_get_args(pointer, (ufix8)(format2 >> 4), &P3);
        depth = (format1 >> 4) & 0x07;
#if DEBUG
        printf("CRVE %6.1f, %6.1f, %6.1f, %6.1f, %6.1f, %6.1f, %d\n",
            (real)P1.x / (real)sp_globals.onepix, (real)P1.y / (real)sp_globals.onepix, 
            (real)P2.x / (real)sp_globals.onepix, (real)P2.y / (real)sp_globals.onepix,
            (real)P3.x / (real)sp_globals.onepix, (real)P3.y / (real)sp_globals.onepix,
            depth);
#endif
        depth += sp_globals.depth_adj;
        if (sp_globals.curves_out)
            {
            fn_curve(P1, P2, P3, depth);
            sp_globals.P0 = P3;
            continue;
            }
        if (depth <= 0)
            {
            fn_line(P3);
            sp_globals.P0 = P3;
            continue;
            }   
        sp_split_curve(P1, P2, P3, depth);
        continue;
        }
    }
}

FUNCTION static void sp_split_curve(
GDECL
point_t P1,    /* First control point of Bezier curve */
point_t P2,    /* Second  control point of Bezier curve */
point_t P3,    /* End point of Bezier curve */
fix15   depth) /* Levels of recursive subdivision required */
/*
 * Called by proc_outl_data() to subdivide Bezier curves into an
 * appropriate number of vectors, whenever curves are not enabled
 * for output to the currently selected output module.
 * sp_split_curve() calls itself recursively to the depth specified
 * at which point it calls line() to deliver each vector resulting
 * from the spliting process.
 */
{
fix31   X0 = (fix31)sp_globals.P0.x;
fix31   Y0 = (fix31)sp_globals.P0.y;
fix31   X1 = (fix31)P1.x;
fix31   Y1 = (fix31)P1.y;
fix31   X2 = (fix31)P2.x;
fix31   Y2 = (fix31)P2.y;
fix31   X3 = (fix31)P3.x;
fix31   Y3 = (fix31)P3.y;
point_t Pmid;
point_t Pctrl1;
point_t Pctrl2;

#if DEBUG
printf("CRVE(%3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f)\n",
    (real)P1.x / (real)sp_globals.onepix, (real)P1.y / (real)sp_globals.onepix,
    (real)P2.x / (real)sp_globals.onepix, (real)P2.y / (real)sp_globals.onepix,
    (real)P3.x / (real)sp_globals.onepix, (real)P3.y / (real)sp_globals.onepix);
#endif


Pmid.x = (X0 + (X1 + X2) * 3 + X3 + 4) >> 3;
Pmid.y = (Y0 + (Y1 + Y2) * 3 + Y3 + 4) >> 3;
if ((--depth) <= 0)
    {
    fn_line(Pmid);
    sp_globals.P0 = Pmid;
    fn_line(P3);
    sp_globals.P0 = P3;
    }
else
    {
    Pctrl1.x = (X0 + X1 + 1) >> 1;
    Pctrl1.y = (Y0 + Y1 + 1) >> 1;
    Pctrl2.x = (X0 + (X1 << 1) + X2 + 2) >> 2;
    Pctrl2.y = (Y0 + (Y1 << 1) + Y2 + 2) >> 2;
    sp_split_curve(Pctrl1, Pctrl2, Pmid, depth);
    Pctrl1.x = (X1 + (X2 << 1) + X3 + 2) >> 2;
    Pctrl1.y = (Y1 + (Y2 << 1) + Y3 + 2) >> 2;
    Pctrl2.x = (X2 + X3 + 1) >> 1;
    Pctrl2.y = (Y2 + Y3 + 1) >> 1;
    sp_split_curve(Pctrl1, Pctrl2, P3, depth);
    }
}

FUNCTION static ufix8 FONTFAR *sp_get_args(
GDECL
ufix8 FONTFAR  *pointer,  /* Pointer to next byte in char data */
ufix8     format,    /* Format specifiaction of argument pair */
point_t STACKFAR *pP)        /* Resulting transformed point */
/*
 * Called by read_bbox() and proc_outl_data() to read an X Y argument
 * pair from the font.
 * The format is specified as follows:
 *     Bits 0-1: Type of X argument.
 *     Bits 2-3: Type of Y argument.
 * where the 4 possible argument types are:
 *     Type 0:   Controlled coordinate represented by one byte
 *               index into the X or Y controlled coordinate table.
 *     Type 1:   Interpolated coordinate represented by a two-byte
 *               signed integer.
 *     Type 2:   Interpolated coordinate represented by a one-byte
 *               signed increment/decrement relative to the 
 *               proceding X or Y coordinate.
 *     Type 3:   Repeat of preceding X or Y argument value and type.
 * The units of P are sub-pixels.
 * Updates *ppointer to point to the byte following the
 * argument pair.
 */
{
ufix8   edge;

/* Read X argument */
switch(format & 0x03)
    {
case 0:                           /* Index to controlled oru */
    edge = NEXT_BYTE(pointer);
    sp_globals.x_orus = sp_plaid.orus[edge];
#if INCL_RULES
    sp_globals.x_pix = sp_plaid.pix[edge];
#endif
    break;

case 1:                           /* 2 byte interpolated oru value */
    sp_globals.x_orus = NEXT_WORD(pointer);
    goto L1;

case 2:                           /* 1 byte signed oru increment */
    sp_globals.x_orus += (fix15)((fix7)NEXT_BYTE(pointer));
L1: 
#if INCL_RULES
    sp_globals.x_pix = TRANS(sp_globals.x_orus, sp_plaid.mult[sp_globals.x_int], sp_plaid.offset[sp_globals.x_int], sp_globals.mpshift);
#endif
    break;

default:                          /* No change in X value */
    break;
    }

/* Read Y argument */
switch((format >> 2) & 0x03)
    {
case 0:                           /* Index to controlled oru */
    edge = sp_globals.Y_edge_org + NEXT_BYTE(pointer);
    sp_globals.y_orus = sp_plaid.orus[edge];
#if INCL_RULES
    sp_globals.y_pix = sp_plaid.pix[edge];
#endif
    break;

case 1:                           /* 2 byte interpolated oru value */
    sp_globals.y_orus = NEXT_WORD(pointer);
    goto L2;

case 2:                           /* 1 byte signed oru increment */
    sp_globals.y_orus += (fix15)((fix7)NEXT_BYTE(pointer));
L2: 
#if INCL_RULES
    sp_globals.y_pix = TRANS(sp_globals.y_orus, sp_plaid.mult[sp_globals.y_int], sp_plaid.offset[sp_globals.y_int], sp_globals.mpshift);
#endif
    break;

default:                          /* No change in X value */
    break;
    }

#if INCL_RULES
switch(sp_globals.tcb.xmode)
    {
case 0:                           /* X mode 0 */
    pP->x = sp_globals.x_pix;
    break;

case 1:                           /* X mode 1 */
    pP->x = -sp_globals.x_pix;
    break;

case 2:                           /* X mode 2 */
    pP->x = sp_globals.y_pix;
    break;

case 3:                           /* X mode 3 */
    pP->x = -sp_globals.y_pix;
    break;

default:                          /* X mode 4 */
#endif
    pP->x = (MULT16(sp_globals.x_orus, sp_globals.tcb.xxmult) + 
             MULT16(sp_globals.y_orus, sp_globals.tcb.xymult) + 
             sp_globals.tcb.xoffset) >> sp_globals.mpshift;
#if INCL_RULES
    break;
    }

switch(sp_globals.tcb.ymode)
    {
case 0:                           /* Y mode 0 */
    pP->y = sp_globals.y_pix;
    break;

case 1:                           /* Y mode 1 */
    pP->y = -sp_globals.y_pix;
    break;

case 2:                           /* Y mode 2 */
    pP->y = sp_globals.x_pix;
    break;

case 3:                           /* Y mode 3 */
    pP->y = -sp_globals.x_pix;
    break;

default:                          /* Y mode 4 */
#endif
    pP->y = (MULT16(sp_globals.x_orus, sp_globals.tcb.yxmult) + 
             MULT16(sp_globals.y_orus, sp_globals.tcb.yymult) + 
             sp_globals.tcb.yoffset) >> sp_globals.mpshift;
#if INCL_RULES
    break;
    }
#endif

return pointer;
}



