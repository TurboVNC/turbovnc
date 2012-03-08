/* $Xorg: out_blk.c,v 1.3 2000/08/17 19:46:26 cpqbld Exp $ */

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
/* $XFree86: xc/lib/font/Speedo/out_blk.c,v 1.3 2001/01/17 19:43:17 dawes Exp $ */


/*************************** O U T _ B L K . C *********************************
 *                                                                           *
 * This is an output module for black-writer mode.                           *
 *                                                                           *
 *****************************************************************************/


#include "spdo_prv.h"               /* General definitions for Speedo   */

#define   DEBUG      0
#define   LOCAL      static
#define   ABS(X)     ( (X < 0) ? -X : X)

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

#if INCL_BLACK
static void sp_add_intercept_black(PROTO_DECL2 fix15 y, fix15 x);
static void sp_proc_intercepts_black(PROTO_DECL1);
#endif


#if INCL_BLACK
FUNCTION boolean init_black(
GDECL
specs_t GLOBALFAR *specsarg)
/*
 * init_out0() is called by sp_set_specs() to initialize the output module.
 * Returns TRUE if output module can accept requested specifications.
 * Returns FALSE otherwise.
 */
{
#if DEBUG
printf("INIT_BLK()\n");
#endif
if (specsarg->flags & CURVES_OUT)
    return FALSE;           /* Curves out not supported */
return (TRUE);
}
#endif


#if INCL_BLACK
FUNCTION boolean begin_char_black(
GDECL
point_t Psw,
point_t Pmin,
point_t Pmax)
/* Called once at the start of the character generation process
 */
{
#if DEBUG
printf("BEGIN_CHAR_BLACK(%3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f\n", 
                    (real)Psw.x / (real)sp_globals.onepix, (real)Psw.y / (real)sp_globals.onepix,
                    (real)Pmin.x / (real)sp_globals.onepix, (real)Pmin.y / (real)sp_globals.onepix,
                    (real)Pmax.x / (real)sp_globals.onepix, (real)Pmax.y / (real)sp_globals.onepix);
#endif
init_char_out(Psw,Pmin,Pmax);
return TRUE;
}
#endif


#if INCL_BLACK
FUNCTION void begin_contour_black(
GDECL
point_t P1,
boolean outside)
/* Called at the start of each contour
 */
{

#if DEBUG
printf("BEGIN_CONTOUR_BLACK(%3.1f, %3.1f, %s)\n", 
    (real)P1.x / (real)sp_globals.onepix, (real)P1.y / (real)sp_globals.onepix, outside? "outside": "inside");
#endif
sp_globals.x0_spxl = P1.x;
sp_globals.y0_spxl = P1.y;
sp_globals.y_pxl = (sp_globals.y0_spxl + sp_globals.pixrnd) >> sp_globals.pixshift;
}
#endif

#if INCL_BLACK
FUNCTION void line_black(
GDECL
point_t P1)
/* Called for each vector in the transformed character
 */
{
register fix15     how_many_y;       /* # of intercepts at y = n + 1/2  */
register fix15     yc, i;            /* Current scan-line */
         fix15     temp1;            /* various uses */
         fix15     temp2;            /* various uses */
register fix31     dx_dy;            /* slope of line in 16.16 form */
register fix31     xc;               /* high-precision (16.16) x coordinate */
         fix15     x0,y0,x1,y1;      /* PIX.FRAC start and endpoints */

x0 = sp_globals.x0_spxl;                 /* get start of line (== current point) */
y0 = sp_globals.y0_spxl;
sp_globals.x0_spxl = x1 = P1.x; /* end of line */
sp_globals.y0_spxl = y1 = P1.y; /*  (also update current point to end of line) */

yc = sp_globals.y_pxl;                   /* current scan line = end of last line */
sp_globals.y_pxl = (y1 + sp_globals.pixrnd) >> sp_globals.pixshift;   /* calculate new end-scan sp_globals.line */


#if DEBUG
printf("LINE_BLACK(%3.4f, %3.4f)\n", 
       (real)P1.x/(real)sp_globals.onepix, 
       (real)P1.y/(real)sp_globals.onepix);
#endif

if (sp_globals.extents_running)
    {
    if (sp_globals.x0_spxl > sp_globals.bmap_xmax)         
        sp_globals.bmap_xmax = sp_globals.x0_spxl;
    if (sp_globals.x0_spxl < sp_globals.bmap_xmin)
        sp_globals.bmap_xmin = sp_globals.x0_spxl;
    if (sp_globals.y0_spxl > sp_globals.bmap_ymax)
        sp_globals.bmap_ymax = sp_globals.y0_spxl;
    if (sp_globals.y0_spxl < sp_globals.bmap_ymin)
        sp_globals.bmap_ymin = sp_globals.y0_spxl;
    }

if (sp_globals.intercept_oflo) return;

if ((how_many_y = sp_globals.y_pxl - yc) == 0) return; /* Don't draw a null line */

if (how_many_y < 0) yc--; /* Predecrment downward lines */

if (yc > sp_globals.y_band.band_max) /* Is start point above band? */
    {
    if (sp_globals.y_pxl > sp_globals.y_band.band_max) return; /* line has to go down! */
    how_many_y = sp_globals.y_pxl - (yc = sp_globals.y_band.band_max) - 1; /* Yes, limit it */
    }

if (yc < sp_globals.y_band.band_min)   /* Is start point below band? */
    {
    if (sp_globals.y_pxl < sp_globals.y_band.band_min) return; /* line has to go up! */
    how_many_y = sp_globals.y_pxl - (yc = sp_globals.y_band.band_min);   /* Yes, limit it */
    }

xc = (fix31)(x0 + sp_globals.pixrnd) << (16 - sp_globals.pixshift); /* Original x coordinate with built in  */
                                            /* rounding. 16.16 form */


if ( (temp1 = (x1 - x0)) == 0)  /* check for vertical line */
    {
    yc -= sp_globals.y_band.band_min; /* yc is now an offset relative to the band */
    temp1 = (fix15)(xc >> 16); 
    if (how_many_y < 0)
        {   /* Vector down */
        if ((how_many_y += yc + 1) < 0) how_many_y = 0; /* can't go below 0 */
        for (i = yc; i >= how_many_y; i--)
            sp_add_intercept_black(i,temp1); 
        }
    else
        {   /* Vector up */
     /* check to see that line doesn't extend beyond top of band */
        if ((how_many_y += yc) > sp_globals.no_y_lists) how_many_y = sp_globals.no_y_lists;
        for (i = yc; i != how_many_y; i++)
            sp_add_intercept_black(i,temp1); 
        }
    return;
    }
          
/* calculate dx_dy at 16.16 fixed point */

dx_dy = ( (fix31)temp1 << 16 )/(fix31)(y1 - y0);

/* We have to check for a @#$%@# possible multiply overflow  */
/* by doing another @#$*& multiply.  In assembly language,   */
/* the program could just check the OVerflow flag or whatever*/
/* works on the particular processor.  This C code is meant  */
/* to be processor independant.                              */

temp1 = (yc << sp_globals.pixshift) - y0 + sp_globals.pixrnd;
/* This sees if the sign bits start at bit 15 */
/* if they do, no overflow has occurred       */

temp2 = (fix15)(MULT16(temp1,(fix15)(dx_dy >> 16)) >> 15);

if (  (temp2 != (fix15)0xFFFF) &&
      (temp2 != 0x0000) &&
      /* Overflow. Pick point closest to yc + .5 */
    (ABS(temp1) < ABS((yc << sp_globals.pixshift) - y1 + sp_globals.pixrnd)) )
    { /* use x1 instead of x0 */
    xc = (fix31)(x1 + sp_globals.pixrnd) << (16 - sp_globals.pixshift);
    }
else
    {
/* calculate new xc at the center of the *current* scan line */
/* due to banding, yc may be several lines away from y0      */
/*  xc += (yc + .5 - y0) * dx_dy */
/* This multiply generates a subpixel delta. */
/* So we shift it to be a 16.16 delta */

    xc += ((fix31)temp1 * dx_dy) >> sp_globals.pixshift;
    }

yc -= sp_globals.y_band.band_min; /* yc is now an offset relative to the band */

if (how_many_y < 0)
    {   /* Vector down */
    if (how_many_y == -1)
        sp_add_intercept_black(yc, (fix15) (xc >> 16));
    else
        {
        if ((how_many_y += yc + 1) < 0) how_many_y = 0; /* can't go below 0 */
        for (i = yc; i >= how_many_y; i--)
            {
            temp1 = (fix15)(xc >> 16); 
            sp_add_intercept_black(i,temp1); 
            xc -= dx_dy;
            }
        }
    }
    else
    {   /* Vector up */
     /* check to see that line doesn't extend beyond top of band */
    if (how_many_y == 1)
        sp_add_intercept_black(yc, (fix15) (xc >> 16));
    else
        {
        if ((how_many_y += yc) > sp_globals.no_y_lists) how_many_y = sp_globals.no_y_lists;
        for (i = yc; i != how_many_y; i++)
            {
            temp1 = (fix15)(xc >> 16);
            sp_add_intercept_black(i,temp1); 
            xc += dx_dy;
            }
        }
    }
}
#endif
#if INCL_BLACK
FUNCTION boolean end_char_black()
GDECL
/* Called when all character data has been output
 * Return TRUE if output process is complete
 * Return FALSE to repeat output of the transformed data beginning
 * with the first contour
 */
{

fix31 xorg;
fix31 yorg;
#if INCL_CLIPPING
fix31 bmap_max, bmap_min;
#endif

#if DEBUG
printf("END_CHAR_BLACK()\n");
#endif

if (sp_globals.first_pass)
    {
    if (sp_globals.bmap_xmax >= sp_globals.bmap_xmin)
        {
        sp_globals.xmin = (sp_globals.bmap_xmin + sp_globals.pixrnd + 1) >> sp_globals.pixshift;
        sp_globals.xmax = (sp_globals.bmap_xmax + sp_globals.pixrnd) >> sp_globals.pixshift;
        }
    else
        {
        sp_globals.xmin = sp_globals.xmax = 0;
        }
    if (sp_globals.bmap_ymax >= sp_globals.bmap_ymin)
        {

#if INCL_CLIPPING
    switch(sp_globals.tcb0.xtype)
       {
       case 1: /* 180 degree rotation */
            if (sp_globals.specs.flags & CLIP_TOP)
               {
               sp_globals.clip_ymin = (fix31)((fix31)EM_TOP * sp_globals.tcb0.yppo + ((1<<sp_globals.multshift)/2));
               sp_globals.clip_ymin = sp_globals.clip_ymin >> sp_globals.multshift;
               bmap_min = (sp_globals.bmap_ymin + sp_globals.pixrnd + 1) >> sp_globals.pixshift;
	       sp_globals.clip_ymin = -1 * sp_globals.clip_ymin;
	       if (bmap_min < sp_globals.clip_ymin)
		    sp_globals.ymin = sp_globals.clip_ymin;
               else
                    sp_globals.ymin = bmap_min;
               }
            if (sp_globals.specs.flags & CLIP_BOTTOM)
               {
               sp_globals.clip_ymax = (fix31)((fix31)(-1 * EM_BOT) * sp_globals.tcb0.yppo + ((1<<sp_globals.multshift)/2));
               sp_globals.clip_ymax = sp_globals.clip_ymax >> sp_globals.multshift;
               bmap_max = (sp_globals.bmap_ymax + sp_globals.pixrnd) >> sp_globals.pixshift;
	       if (bmap_max < sp_globals.clip_ymax)
                    sp_globals.ymax = bmap_max;
               else
		    sp_globals.ymax = sp_globals.clip_ymax;
               }
               sp_globals.clip_xmax =  -sp_globals.xmin;
               sp_globals.clip_xmin = ((sp_globals.set_width.x+32768L) >> 16) -
                                      sp_globals.xmin;
               break;
       case 2: /* 90 degree rotation */
            if (sp_globals.specs.flags & CLIP_TOP)
               {
               sp_globals.clip_xmin = (fix31)((fix31)(-1 * EM_BOT) * sp_globals.tcb0.yppo + ((1<<sp_globals.multshift)/2));
               sp_globals.clip_xmin = sp_globals.clip_xmin >> sp_globals.multshift;
               sp_globals.clip_xmin = -1 * sp_globals.clip_xmin;
               bmap_min = (sp_globals.bmap_xmin + sp_globals.pixrnd + 1) >> sp_globals.pixshift;
	       if (bmap_min > sp_globals.clip_xmin)
                    sp_globals.clip_xmin = bmap_min;

	       /* normalize to x origin */
               sp_globals.clip_xmin -= sp_globals.xmin;
               }
            if (sp_globals.specs.flags & CLIP_BOTTOM)
               {
               sp_globals.clip_xmax = (fix31)((fix31)EM_TOP * sp_globals.tcb0.yppo + ((1<<sp_globals.multshift)/2));
               sp_globals.clip_xmax = sp_globals.clip_xmax >> sp_globals.multshift;
               bmap_max = (sp_globals.bmap_xmax + sp_globals.pixrnd) >> sp_globals.pixshift;
	       if (bmap_max < sp_globals.clip_xmax)
                    sp_globals.xmax = bmap_max;
               else
		    sp_globals.xmax = sp_globals.clip_xmax;
	       sp_globals.clip_ymax = 0;
	       if ((sp_globals.specs.flags & CLIP_TOP) && 
                   (sp_globals.ymax > sp_globals.clip_ymax))
		    sp_globals.ymax = sp_globals.clip_ymax;
	       sp_globals.clip_ymin = ((sp_globals.set_width.y+32768L) >> 16);
               if ((sp_globals.specs.flags & CLIP_BOTTOM) && 
                   (sp_globals.ymin < sp_globals.clip_ymin))
                    sp_globals.ymin = sp_globals.clip_ymin;
	       /* normalize to x origin */
               sp_globals.clip_xmax -= sp_globals.xmin;
               }
               break;
       case 3: /* 270 degree rotation */
            if (sp_globals.specs.flags & CLIP_TOP)
               {
               sp_globals.clip_xmin = (fix31)((fix31)EM_TOP * sp_globals.tcb0.yppo + ((1<<sp_globals.multshift)/2));
               sp_globals.clip_xmin = sp_globals.clip_xmin >> sp_globals.multshift;
	       sp_globals.clip_xmin = -1 * sp_globals.clip_xmin;
               bmap_min = (sp_globals.bmap_xmin + sp_globals.pixrnd + 1) >> sp_globals.pixshift;

               /* let the minimum be the larger of these two values */
	       if (bmap_min > sp_globals.clip_xmin)
		    sp_globals.clip_xmin = bmap_min;

	       /* normalize the x value to new xorgin */
               sp_globals.clip_xmin -= sp_globals.xmin;
               }
            if (sp_globals.specs.flags & CLIP_BOTTOM)
               {
               sp_globals.clip_xmax = (fix31)((fix31)(-1 * EM_BOT) * sp_globals.tcb0.yppo + ((1<<sp_globals.multshift)/2));
               sp_globals.clip_xmax = sp_globals.clip_xmax >> sp_globals.multshift;
               bmap_max = (sp_globals.bmap_xmax + sp_globals.pixrnd) >> sp_globals.pixshift;

	       /* let the max be the lesser of these two values */
	       if (bmap_max < sp_globals.clip_xmax)
		    {
		    sp_globals.xmax = bmap_max; 
		    sp_globals.clip_xmax = bmap_max;
		    }
               else
                    sp_globals.xmax = sp_globals.clip_xmax;

	       /* normalize the x value to new x origin */
	       sp_globals.clip_xmax -= sp_globals.xmin;
               }
               /* compute y clip values */
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
	       sp_globals.clip_ymax = (fix31)((fix31)EM_TOP * sp_globals.tcb0.yppo + ((1<<sp_globals.multshift)/2));
               sp_globals.clip_ymax = sp_globals.clip_ymax >> sp_globals.multshift;
               bmap_max = (sp_globals.bmap_ymax + sp_globals.pixrnd) >> sp_globals.pixshift;
	       if (bmap_max > sp_globals.clip_ymax)
                    sp_globals.ymax = bmap_max;
               else
		    sp_globals.ymax = sp_globals.clip_ymax;
               }
            if (sp_globals.specs.flags & CLIP_BOTTOM)
               {
	       sp_globals.clip_ymin = (fix31)((fix31)(-1 * EM_BOT) * sp_globals.tcb0.yppo +  ((1<<sp_globals.multshift)/2));
               sp_globals.clip_ymin = sp_globals.clip_ymin >> sp_globals.multshift;
	       sp_globals.clip_ymin = - sp_globals.clip_ymin;
               bmap_min = (sp_globals.bmap_ymin + sp_globals.pixrnd + 1) >> sp_globals.pixshift;
	       if (bmap_min < sp_globals.clip_ymin)
		    sp_globals.ymin = sp_globals.clip_ymin;
               else
                    sp_globals.ymin = bmap_min;
               }
               sp_globals.clip_xmin = -sp_globals.xmin;
               sp_globals.clip_xmax = ((sp_globals.set_width.x+32768L) >> 16) -
                                      sp_globals.xmin;
               break;
       }
if ( !(sp_globals.specs.flags & CLIP_TOP))
#endif
            sp_globals.ymax = (sp_globals.bmap_ymax + sp_globals.pixrnd) >> sp_globals.pixshift;

#if INCL_CLIPPING
if ( !(sp_globals.specs.flags & CLIP_BOTTOM))
#endif

        sp_globals.ymin = (sp_globals.bmap_ymin + sp_globals.pixrnd + 1) >> sp_globals.pixshift;
        }
    else
        {
        sp_globals.ymin = sp_globals.ymax = 0;
        }

    /* add in the rounded out part (from xform.) of the left edge */
    if (sp_globals.tcb.xmode == 0)    /* for X pix is function of X orus only add the round */
    	xorg = (((fix31)sp_globals.xmin << 16) + (sp_globals.rnd_xmin << sp_globals.poshift));
    else
        if (sp_globals.tcb.xmode == 1) /* for X pix is function of -X orus only, subtr. round */
        	xorg = (((fix31)sp_globals.xmin << 16) - (sp_globals.rnd_xmin << sp_globals.poshift)) ;
        else
        	xorg = (fix31)sp_globals.xmin << 16;   /* for other cases don't use round on x */
           
    if (sp_globals.tcb.ymode == 2)   /* for Y pix is function of X orus only, add round error */ 
    	yorg = (((fix31)sp_globals.ymin << 16) + (sp_globals.rnd_xmin << sp_globals.poshift));
    else
        if (sp_globals.tcb.ymode == 3) /* for Y pix is function of -X orus only, sub round */
        	yorg = (((fix31)sp_globals.ymin << 16) - (sp_globals.rnd_xmin << sp_globals.poshift));
        else                          /* all other cases have no round error on yorg */
         	yorg = (fix31)sp_globals.ymin << 16;

    open_bitmap(sp_globals.set_width.x, sp_globals.set_width.y, xorg, yorg,
				 sp_globals.xmax - sp_globals.xmin, sp_globals.ymax -  sp_globals.ymin);
    if (sp_globals.intercept_oflo)
        {
        sp_globals.y_band.band_min = sp_globals.ymin;
        sp_globals.y_band.band_max = sp_globals.ymax;
        init_intercepts_out();
        sp_globals.first_pass = FALSE;
        sp_globals.extents_running = FALSE;
        return FALSE;
        }
    else
        {
        sp_proc_intercepts_black();
        close_bitmap();
        return TRUE;
        }
    }
else
    {
    if (sp_globals.intercept_oflo)
        {
        reduce_band_size_out();
        init_intercepts_out();
        return FALSE;
        }
    else
        {
        sp_proc_intercepts_black();
        if (next_band_out())
            {
            init_intercepts_out();
            return FALSE;
            }
        close_bitmap();
        return TRUE;
        }
    }
}
#endif

#if INCL_BLACK
FUNCTION LOCAL  void sp_add_intercept_black(
GDECL
fix15 y,                 /* Y coordinate in relative pixel units */
                         /* (0 is lowest sample in band) */
fix15 x)                 /* X coordinate of intercept in subpixel units */

/*  Called by line() to add an intercept to the intercept list structure
 */

{
register fix15 from;   /* Insertion pointers for the linked list sort */
register fix15 to;

#if DEBUG
printf("    Add intercept(%2d, %d)\n", y + sp_globals.y_band.band_min,x);

/* Bounds checking IS done in debug mode */
if (y < 0)       /* Y value below bottom of current band? */
    {
    printf(" Intecerpt less than 0!!!\007\n");
    return;
    }

if (y > (sp_globals.no_y_lists - 1))              /* Y value above top of current band? */
    {
    printf(" Intercept too big for band!!!!!\007\n");
    return;
    }
#endif

/* Store new values */

sp_intercepts.car[sp_globals.next_offset] = x;

/* Find slot to insert new element (between from and to) */

from = y; /* Start at list head */

while( (to = sp_intercepts.cdr[from]) >= sp_globals.first_offset) /* Until to == end of list */
    {
    if (x <= sp_intercepts.car[to]) /* If next item is larger than or same as this one... */
        goto insert_element; /* ... drop out and insert here */
    from = to; /* move forward in list */
    }

insert_element: /* insert element "sp_globals.next_offset" between elements "from" */
                /* and "to" */

sp_intercepts.cdr[from] = sp_globals.next_offset;
sp_intercepts.cdr[sp_globals.next_offset] = to;

if (++sp_globals.next_offset >= MAX_INTERCEPTS) /* Intercept buffer full? */
    {
    sp_globals.intercept_oflo = TRUE;
/* There may be a few more calls to "add_intercept" from the current line */
/* To avoid problems, we set next_offset to a safe value. We don't care   */
/* if the intercept table gets trashed at this point                      */
    sp_globals.next_offset = sp_globals.first_offset;
    }
}

#endif

#if INCL_BLACK
FUNCTION LOCAL  void sp_proc_intercepts_black()
GDECL

/*  Called by sp_make_char to output accumulated intercept lists
 *  Clips output to sp_globals.xmin, sp_globals.xmax, sp_globals.ymin, sp_globals.ymax boundaries
 */
{
register fix15 i;
register fix15 from, to;          /* Start and end of run in pixel units   
                            relative to left extent of character  */
register fix15 y;
register fix15 scan_line;
         fix15 first_y, last_y;

#if DEBUG
printf("\nIntercept lists:\n");
#endif

#if INCL_CLIPPING
if ((sp_globals.specs.flags & CLIP_LEFT) != 0)
    clipleft = TRUE;
else
    clipleft = FALSE;
if ((sp_globals.specs.flags & CLIP_RIGHT) != 0)
    clipright = TRUE;
else
    clipright = FALSE;
if (clipleft || clipright)
	{
	xmax = sp_globals.clip_xmax;
	xmin = sp_globals.clip_xmin;
	}
if (!clipright)
        xmax = ((sp_globals.set_width.x+32768L) >> 16);
#endif

if ((first_y = sp_globals.y_band.band_max) >= sp_globals.ymax)    
    first_y = sp_globals.ymax - 1;               /* Clip to sp_globals.ymax boundary */

if ((last_y = sp_globals.y_band.band_min) < sp_globals.ymin)      
    last_y = sp_globals.ymin;                    /* Clip to sp_globals.ymin boundary */

last_y  -= sp_globals.y_band.band_min;
#if DEBUG
/* Print out all of the intercept info */
scan_line = sp_globals.ymax - first_y - 1;

for (y = first_y - sp_globals.y_band.band_min; y >= last_y; y--, scan_line++)
    {
    i = y;                            /* Index head of intercept list */
    while ((i = sp_intercepts.cdr[i]) != 0)         /* Link to next intercept if present */
        {
        if ((from = sp_intercepts.car[i] - sp_globals.xmin) < 0)
            from = 0;                 /* Clip to sp_globals.xmin boundary */
        i = sp_intercepts.cdr[i];                   /* Link to next intercept */
        if (i == 0)                   /* End of list? */
            {
            printf("****** proc_intercepts: odd number of intercepts\n");
            break;
            }
        if ((to = sp_intercepts.car[i]) > sp_globals.xmax)
            to = sp_globals.xmax - sp_globals.xmin;         /* Clip to sp_globals.xmax boundary */
        else
            to -= sp_globals.xmin;
        printf("    Y = %2d (scanline %2d): %d %d:\n", 
            y + sp_globals.y_band.band_min, scan_line, from, to);
        }
    }
#endif

/* Draw the image */
scan_line = sp_globals.ymax - first_y - 1;

for (y = first_y - sp_globals.y_band.band_min; y >= last_y; y--, scan_line++)
    {
    i = y;                            /* Index head of intercept list */
    while ((i = sp_intercepts.cdr[i]) != 0)         /* Link to next intercept if present */
        {
        if ((from = sp_intercepts.car[i] - sp_globals.xmin) < 0)
            from = 0;                 /* Clip to sp_globals.xmin boundary */
        i = sp_intercepts.cdr[i];                   /* Link to next intercept */

        if ((to = sp_intercepts.car[i]) > sp_globals.xmax)
            to = sp_globals.xmax - sp_globals.xmin;         /* Clip to sp_globals.xmax boundary */
        else
            to -= sp_globals.xmin;
        if (from >= to)
			{
			if (from >= sp_globals.xmax - sp_globals.xmin)
				{
				--from ;
				}
			to = from+1;
			}
#if INCL_CLIPPING
		if (clipleft)
			{
			if (to <= xmin)
				continue;
			if (from < xmin)
				from = xmin;
			}
	if (clipright)
			{
			if (from >= xmax)
				continue;
			if (to > xmax)
				to = xmax;
			}
#endif
        set_bitmap_bits(scan_line, from, to);
        }
    }
}

#endif




