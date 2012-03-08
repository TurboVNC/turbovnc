/* $Xorg: out_util.c,v 1.3 2000/08/17 19:46:26 cpqbld Exp $ */

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
/* $XFree86: xc/lib/font/Speedo/out_util.c,v 1.3 2001/01/17 19:43:17 dawes Exp $ */


#define	DEBUG	0

/*************************** O U T _ U T I L . C *****************************
 *                                                                           *
 * This is a utility module share by all bitmap output modules               *
 *                                                                           *
 *****************************************************************************/


#include "spdo_prv.h"               /* General definitions for Speedo   */
/* absolute value function */
#define   ABS(X)     ( (X < 0) ? -X : X)
#if INCL_BLACK || INCL_2D || INCL_SCREEN

FUNCTION  void init_char_out(
GDECL
point_t Psw, point_t Pmin, point_t Pmax)
{
sp_globals.set_width.x = (fix31)Psw.x << sp_globals.poshift;
sp_globals.set_width.y = (fix31)Psw.y << sp_globals.poshift;
set_first_band_out(Pmin, Pmax);
init_intercepts_out();
if (sp_globals.normal)
    {
    sp_globals.bmap_xmin = Pmin.x;
    sp_globals.bmap_xmax = Pmax.x;
    sp_globals.bmap_ymin = Pmin.y;
    sp_globals.bmap_ymax = Pmax.y;
    sp_globals.extents_running = FALSE;
    }
else
    {
    sp_globals.bmap_xmin = 32000;
    sp_globals.bmap_xmax = -32000;
    sp_globals.bmap_ymin = 32000;
    sp_globals.bmap_ymax = -32000;
    sp_globals.extents_running = TRUE;
    }
sp_globals.first_pass = TRUE;
}

FUNCTION void begin_sub_char_out(
GDECL
point_t Psw,
point_t Pmin,
point_t Pmax)
/* Called at the start of each sub-character in a composite character
 */
{
#if DEBUG
printf("BEGIN_SUB_CHAR_out(%3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f\n", 
                    (real)Psw.x / (real)sp_globals.onepix, (real)Psw.y / (real)sp_globals.onepix,
                    (real)Pmin.x / (real)sp_globals.onepix, (real)Pmin.y / (real)sp_globals.onepix,
                    (real)Pmax.x / (real)sp_globals.onepix, (real)Pmax.y / (real)sp_globals.onepix);
#endif
restart_intercepts_out();
if (!sp_globals.extents_running)
	{
    sp_globals.bmap_xmin = 32000;
    sp_globals.bmap_xmax = -32000;
    sp_globals.bmap_ymin = 32000;
    sp_globals.bmap_ymax = -32000;
    sp_globals.extents_running = TRUE;
	}
}

FUNCTION void curve_out(
GDECL
point_t P1, point_t P2, point_t P3,
fix15 depth)
/* Called for each curve in the transformed character if curves out enabled
 */
{
#if DEBUG
printf("CURVE_OUT(%3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f)\n", 
    (real)P1.x / (real)sp_globals.onepix, (real)P1.y / (real)sp_globals.onepix,
    (real)P2.x / (real)sp_globals.onepix, (real)P2.y / (real)sp_globals.onepix,
    (real)P3.x / (real)sp_globals.onepix, (real)P3.y / (real)sp_globals.onepix);
#endif
}



FUNCTION void end_contour_out()
GDECL
/* Called after the last vector in each contour
 */
{
#if DEBUG
printf("END_CONTOUR_OUT()\n");
#endif
}


FUNCTION void end_sub_char_out()
GDECL
/* Called after the last contour in each sub-character in a compound character
 */
{
#if DEBUG
printf("END_SUB_CHAR_OUT()\n");
#endif
}


FUNCTION void init_intercepts_out()
GDECL
/*  Called to initialize intercept storage data structure
 */

{
fix15 i;
fix15 no_lists;

#if DEBUG
printf("    Init intercepts (Y band from %d to %d)\n", sp_globals.y_band.band_min, sp_globals.y_band.band_max);
if (sp_globals.x_scan_active)
    printf("                    (X band from %d to %d)\n", sp_globals.x_band.band_min, sp_globals.x_band.band_max);
#endif 

sp_globals.intercept_oflo = FALSE;

sp_globals.no_y_lists = sp_globals.y_band.band_max - sp_globals.y_band.band_min + 1;
#if INCL_2D
if (sp_globals.output_mode == MODE_2D)
	{
	sp_globals.no_x_lists = sp_globals.x_scan_active ? 
		sp_globals.x_band.band_max - sp_globals.x_band.band_min + 1 : 0;
	no_lists = sp_globals.no_y_lists + sp_globals.no_x_lists;
	} 
else
#endif
	no_lists = sp_globals.no_y_lists;

#if INCL_2D
sp_globals.y_band.band_floor = 0;
sp_globals.y_band.band_ceiling = sp_globals.no_y_lists;
#endif
                                        
if (no_lists >= MAX_INTERCEPTS)  /* Not enough room for list table? */
    {
    no_lists = sp_globals.no_y_lists = MAX_INTERCEPTS;
    sp_globals.intercept_oflo = TRUE;
	sp_globals.y_band.band_min = sp_globals.y_band.band_max - sp_globals.no_y_lists + 1;
#if INCL_2D
    sp_globals.y_band.band_array_offset = sp_globals.y_band.band_min;
    sp_globals.y_band.band_ceiling = sp_globals.no_y_lists;
    sp_globals.no_x_lists = 0;
    sp_globals.x_scan_active = FALSE;
#endif
    }

for (i = 0; i < no_lists; i++)   /* For each active value... */
    {
#if INCL_SCREEN
	if (sp_globals.output_mode == MODE_SCREEN)
		sp_intercepts.inttype[i]=0;
#endif
    sp_intercepts.cdr[i] = 0;                    /* Mark each intercept list empty */
    }

sp_globals.first_offset = sp_globals.next_offset = no_lists;

#if INCL_2D
sp_globals.y_band.band_array_offset = sp_globals.y_band.band_min;
sp_globals.x_band.band_array_offset = sp_globals.x_band.band_min - sp_globals.no_y_lists;
sp_globals.x_band.band_floor = sp_globals.no_y_lists;
sp_globals.x_band.band_ceiling = no_lists;
#endif
#if INCL_SCREEN
sp_intercepts.inttype[sp_globals.no_y_lists-1] = END_INT;
#endif

}


FUNCTION void restart_intercepts_out()
GDECL

/*  Called by sp_make_char when a new sub character is started
 *  Freezes current sorted lists
 */

{

#if DEBUG
printf("    Restart intercepts:\n");
#endif
sp_globals.first_offset = sp_globals.next_offset;
}



FUNCTION void set_first_band_out(
GDECL
point_t Pmin,
point_t Pmax)
{

sp_globals.ymin = Pmin.y;
sp_globals.ymax = Pmax.y;

sp_globals.ymin = (sp_globals.ymin - sp_globals.onepix + 1) >> sp_globals.pixshift;
sp_globals.ymax = (sp_globals.ymax + sp_globals.onepix - 1) >> sp_globals.pixshift;

#if INCL_CLIPPING
    switch(sp_globals.tcb0.xtype)
       {
       case 1: /* 180 degree rotation */
	    if (sp_globals.specs.flags & CLIP_TOP)
               {
               sp_globals.clip_ymin = (fix31)((fix31)EM_TOP * sp_globals.tcb0.yppo + ((1<<sp_globals.multshift)/2));
               sp_globals.clip_ymin = sp_globals.clip_ymin >> sp_globals.multshift;
	       sp_globals.clip_ymin = -1* sp_globals.clip_ymin;
	       if (sp_globals.ymin < sp_globals.clip_ymin)
		    sp_globals.ymin = sp_globals.clip_ymin;
	       }
            if (sp_globals.specs.flags & CLIP_BOTTOM)
	       {
               sp_globals.clip_ymax = (fix31)((fix31)(-1 * EM_BOT) * sp_globals.tcb0.yppo + ((1<<sp_globals.multshift)/2));
               sp_globals.clip_ymax = sp_globals.clip_ymax >> sp_globals.multshift;
	       if (sp_globals.ymax > sp_globals.clip_ymax)
		    sp_globals.ymax = sp_globals.clip_ymax;
               }
               break;
       case 2: /* 90 degree rotation */
            sp_globals.clip_ymax = 0;
            if ((sp_globals.specs.flags & CLIP_TOP) &&
                (sp_globals.ymax > sp_globals.clip_ymax))
                 sp_globals.ymax = sp_globals.clip_ymax;
            sp_globals.clip_ymin = ((sp_globals.set_width.y+32768L) >> 16);
            if ((sp_globals.specs.flags & CLIP_BOTTOM) &&
                (sp_globals.ymin < sp_globals.clip_ymin))
                 sp_globals.ymin = sp_globals.clip_ymin;
            break;
       case 3: /* 270 degree rotation */
               sp_globals.clip_ymax = ((sp_globals.set_width.y+32768L) >> 16);
               if ((sp_globals.specs.flags & CLIP_TOP) &&
                   (sp_globals.ymax > sp_globals.clip_ymax))
                    sp_globals.ymax = sp_globals.clip_ymax;
               sp_globals.clip_ymin = 0;
               if ((sp_globals.specs.flags & CLIP_BOTTOM) &&
                   (sp_globals.ymin < sp_globals.clip_ymin))
                    sp_globals.ymin = sp_globals.clip_ymin;
               break;
       default: /* this is for zero degree rotation and arbitrary rotation */
	    if (sp_globals.specs.flags & CLIP_TOP)
               {
	       sp_globals.clip_ymax = (fix31)((fix31)EM_TOP * sp_globals.tcb0.yppo +  ((1<<sp_globals.multshift)/2));
               sp_globals.clip_ymax = sp_globals.clip_ymax >> sp_globals.multshift;
	       if (sp_globals.ymax > sp_globals.clip_ymax)
		    sp_globals.ymax = sp_globals.clip_ymax;
	       }
            if (sp_globals.specs.flags & CLIP_BOTTOM)
	       {
	       sp_globals.clip_ymin = (fix31)((fix31)(-1 * EM_BOT) * sp_globals.tcb0.yppo +  ((1<<sp_globals.multshift)/2));
               sp_globals.clip_ymin = sp_globals.clip_ymin >> sp_globals.multshift;
	       sp_globals.clip_ymin = - sp_globals.clip_ymin;
	       if (sp_globals.ymin < sp_globals.clip_ymin)
		    sp_globals.ymin = sp_globals.clip_ymin;
               }
               break;
       }
#endif
sp_globals.y_band.band_min = sp_globals.ymin;
sp_globals.y_band.band_max = sp_globals.ymax - 1; 

sp_globals.xmin = (Pmin.x + sp_globals.pixrnd) >> sp_globals.pixshift;
sp_globals.xmax = (Pmax.x + sp_globals.pixrnd) >> sp_globals.pixshift;


#if INCL_2D
sp_globals.x_band.band_min = sp_globals.xmin - 1; /* subtract one pixel of "safety margin" */
sp_globals.x_band.band_max = sp_globals.xmax /* - 1 + 1 */; /* Add one pixel of "safety margin" */
#endif
}




                                  


FUNCTION void reduce_band_size_out()
GDECL
{
sp_globals.y_band.band_min = sp_globals.y_band.band_max - ((sp_globals.y_band.band_max - sp_globals.y_band.band_min) >> 1);
#if INCL_2D
sp_globals.y_band.band_array_offset = sp_globals.y_band.band_min;
#endif
}


FUNCTION boolean next_band_out()
GDECL
{
fix15  tmpfix15;

if (sp_globals.y_band.band_min <= sp_globals.ymin)
    return FALSE;
tmpfix15 = sp_globals.y_band.band_max - sp_globals.y_band.band_min;
sp_globals.y_band.band_max = sp_globals.y_band.band_min - 1;
sp_globals.y_band.band_min = sp_globals.y_band.band_max - tmpfix15;
if (sp_globals.y_band.band_min < sp_globals.ymin)
    sp_globals.y_band.band_min = sp_globals.ymin;
#if INCL_2D
sp_globals.y_band.band_array_offset = sp_globals.y_band.band_min;
#endif
return TRUE;
}
#endif

