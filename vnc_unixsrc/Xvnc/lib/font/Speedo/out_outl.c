/* $Xorg: out_outl.c,v 1.3 2000/08/17 19:46:26 cpqbld Exp $ */

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


/**************************** O U T _ 2 _ 1 . C ******************************
 *                                                                           *
 * This is the standard output module for vector output mode.                *
 *                                                                           *
 ****************************************************************************/


#include "spdo_prv.h"               /* General definitions for Speedo     */


#define   DEBUG      0

#if DEBUG
#include <stdio.h>
#define SHOW(X) printf("X = %d\n", X)
#else
#define SHOW(X)
#endif

/* the following macro is used to limit points on the outline to the bounding box */

#define RANGECHECK(value,min,max) (((value) >= (min) ? (value) : (min)) < (max) ? (value) : (max))
/***** GLOBAL VARIABLES *****/

/***** GLOBAL FUNCTIONS *****/

/***** EXTERNAL VARIABLES *****/

/***** EXTERNAL FUNCTIONS *****/

/***** STATIC VARIABLES *****/

/***** STATIC FUNCTIONS *****/


#if INCL_OUTLINE
FUNCTION boolean init_outline(specsarg)
GDECL
specs_t GLOBALFAR *specsarg;
/*
 * init_out2() is called by sp_set_specs() to initialize the output module.
 * Returns TRUE if output module can accept requested specifications.
 * Returns FALSE otherwise.
 */
{
#if DEBUG
printf("INIT_OUT_2()\n");
#endif
if (specsarg->flags & (CLIP_LEFT + CLIP_RIGHT + CLIP_TOP + CLIP_BOTTOM))
    return FALSE;           /* Clipping not supported */
return (TRUE); 
}
#endif

#if INCL_OUTLINE
FUNCTION boolean begin_char_outline(Psw, Pmin, Pmax)
GDECL
point_t Psw;       /* End of escapement vector (sub-pixels) */            
point_t Pmin;      /* Bottom left corner of bounding box */             
point_t Pmax;      /* Top right corner of bounding box */
/*
 * If two or more output modules are included in the configuration, begin_char2()
 * is called by begin_char() to signal the start of character output data.
 * If only one output module is included in the configuration, begin_char() is 
 * called by make_simp_char() and make_comp_char().
 */
{
fix31 set_width_x;
fix31 set_width_y;
fix31  xmin;
fix31  xmax;
fix31  ymin;
fix31  ymax;

#if DEBUG
printf("BEGIN_CHAR_2(%3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f\n", 
                    (real)Psw.x / (real)onepix, (real)Psw.y / (real)onepix,
                    (real)Pmin.x / (real)onepix, (real)Pmin.y / (real)onepix,
                    (real)Pmax.x / (real)onepix, (real)Pmax.y / (real)onepix);
#endif
sp_globals.poshift = 16 - sp_globals.pixshift;
set_width_x = (fix31)Psw.x << sp_globals.poshift;
set_width_y = (fix31)Psw.y << sp_globals.poshift;
xmin = (fix31)Pmin.x << sp_globals.poshift;
xmax = (fix31)Pmax.x << sp_globals.poshift;
ymin = (fix31)Pmin.y << sp_globals.poshift;
ymax = (fix31)Pmax.y << sp_globals.poshift;
sp_globals.xmin = Pmin.x;
sp_globals.xmax = Pmax.x;
sp_globals.ymin = Pmin.y;
sp_globals.ymax = Pmax.y;
open_outline(set_width_x, set_width_y, xmin, xmax, ymin, ymax);
return TRUE;
}
#endif

#if INCL_OUTLINE
FUNCTION void begin_sub_char_outline(Psw, Pmin, Pmax)
GDECL
point_t Psw;       /* End of sub-char escapement vector */            
point_t Pmin;      /* Bottom left corner of sub-char bounding box */             
point_t Pmax;      /* Top right corner of sub-char bounding box */
/*
 * If two or more output modules are included in the configuration, begin_sub_char2()
 * is called by begin_sub_char() to signal the start of sub-character output data.
 * If only one output module is included in the configuration, begin_sub_char() is 
 * called by make_comp_char().
 */
{
#if DEBUG
printf("BEGIN_SUB_CHAR_2(%3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f\n", 
                    (real)Psw.x / (real)onepix, (real)Psw.y / (real)onepix,
                    (real)Pmin.x / (real)onepix, (real)Pmin.y / (real)onepix,
                    (real)Pmax.x / (real)onepix, (real)Pmax.y / (real)onepix);
#endif
start_new_char();
}
#endif


#if INCL_OUTLINE
FUNCTION void begin_contour_outline(P1, outside)
GDECL
point_t P1;       /* Start point of contour */            
boolean outside;  /* TRUE if outside (counter-clockwise) contour */
/*
 * If two or more output modules are included in the configuration, begin_contour2()
 * is called by begin_contour() to define the start point of a new contour
 * and to indicate whether it is an outside (counter-clockwise) contour
 * or an inside (clockwise) contour.
 * If only one output module is included in the configuration, begin_sub_char() is 
 * called by proc_outl_data().
 */
{
fix15 x,y;
#if DEBUG
printf("BEGIN_CONTOUR_2(%3.1f, %3.1f, %s)\n", 
    (real)P1.x / (real)onepix, (real)P1.y / (real)onepix, outside? "outside": "inside");
#endif
x = RANGECHECK(P1.x,sp_globals.xmin,sp_globals.xmax);
y = RANGECHECK(P1.y,sp_globals.ymin,sp_globals.ymax);

start_contour((fix31)x << sp_globals.poshift, (fix31)y << sp_globals.poshift, outside);
}
#endif

#if INCL_OUTLINE
FUNCTION void curve_outline(P1, P2, P3,depth)
GDECL
point_t P1;      /* First control point of Bezier curve */
point_t P2;      /* Second control point of Bezier curve */
point_t P3;      /* End point of Bezier curve */
fix15 depth;
/*
 * If two or more output modules are included in the configuration, curve2()
 * is called by curve() to output one curve segment.
 * If only one output module is included in the configuration, curve() is 
 * called by proc_outl_data().
 * This function is only called when curve output is enabled.
 */
{
fix15 x1,y1,x2,y2,x3,y3;
#if DEBUG
printf("CURVE_2(%3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f)\n", 
    (real)P1.x / (real)onepix, (real)P1.y / (real)onepix,
    (real)P2.x / (real)onepix, (real)P2.y / (real)onepix,
    (real)P3.x / (real)onepix, (real)P3.y / (real)onepix);
#endif
x1= RANGECHECK(P1.x,sp_globals.xmin,sp_globals.xmax);
y1= RANGECHECK(P1.y,sp_globals.ymin,sp_globals.ymax);

x2= RANGECHECK(P2.x,sp_globals.xmin,sp_globals.xmax);
y2= RANGECHECK(P2.y,sp_globals.ymin,sp_globals.ymax);

x3= RANGECHECK(P3.x,sp_globals.xmin,sp_globals.xmax);
y3= RANGECHECK(P3.y,sp_globals.ymin,sp_globals.ymax);

curve_to((fix31)x1 << sp_globals.poshift, (fix31)y1 << sp_globals.poshift,
         (fix31)x2<< sp_globals.poshift, (fix31)y2 << sp_globals.poshift,
         (fix31)x3 << sp_globals.poshift, (fix31)y3 << sp_globals.poshift);
}
#endif

#if INCL_OUTLINE
FUNCTION void line_outline(P1)
GDECL
point_t P1;      /* End point of vector */             
/*
 * If two or more output modules are included in the configuration, line2()
 * is called by line() to output one vector.
 * If only one output module is included in the configuration, line() is 
 * called by proc_outl_data(). If curve output is enabled, line() is also
 * called by split_curve().
 */
{
fix15 x1,y1;
#if DEBUG
printf("LINE_2(%3.1f, %3.1f)\n", (real)P1.x / (real)onepix, (real)P1.y / (real)onepix);
#endif
x1= RANGECHECK(P1.x,sp_globals.xmin,sp_globals.xmax);
y1= RANGECHECK(P1.y,sp_globals.ymin,sp_globals.ymax);

line_to((fix31)x1 << sp_globals.poshift, (fix31)y1 << sp_globals.poshift);
}
#endif

#if INCL_OUTLINE
FUNCTION void end_contour_outline()
GDECL
/*
 * If two or more output modules are included in the configuration, end_contour2()
 * is called by end_contour() to signal the end of a contour.
 * If only one output module is included in the configuration, end_contour() is 
 * called by proc_outl_data().
 */
{
#if DEBUG
printf("END_CONTOUR_2()\n");
#endif
close_contour();
}
#endif


#if INCL_OUTLINE
FUNCTION void end_sub_char_outline()
GDECL
/*
 * If two or more output modules are included in the configuration, end_sub_char2()
 * is called by end_sub_char() to signal the end of sub-character data.
 * If only one output module is included in the configuration, end_sub_char() is 
 * called by make_comp_char().
 */
{
#if DEBUG
printf("END_SUB_CHAR_2()\n");
#endif
}
#endif


#if INCL_OUTLINE
FUNCTION boolean end_char_outline()
GDECL
/*
 * If two or more output modules are included in the configuration, end_char2()
 * is called by end_char() to signal the end of the character data.
 * If only one output module is included in the configuration, end_char() is 
 * called by make_simp_char() and make_comp_char().
 * Returns TRUE if output process is complete
 * Returns FALSE to repeat output of the transformed data beginning
 * with the first contour (of the first sub-char if compound).
 */
{
#if DEBUG
printf("END_CHAR_2()\n");
#endif
close_outline();
return TRUE;
}
#endif

