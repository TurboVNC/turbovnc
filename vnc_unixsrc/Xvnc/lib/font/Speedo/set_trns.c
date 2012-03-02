/* $Xorg: set_trns.c,v 1.3 2000/08/17 19:46:27 cpqbld Exp $ */

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
/* $XFree86: xc/lib/font/Speedo/set_trns.c,v 1.6 2003/05/27 22:26:44 tsi Exp $ */



/*************************** S E T _ T R N S . C *****************************
 *                                                                           *
 * This module is called from do_char.c to set up the intelligent            *
 * transformation for one character (or sub-character of a composite         *
 * character.
 *                                                                           *
 ****************************************************************************/


#include "spdo_prv.h"               /* General definitions for Speedo   */

#define   DEBUG      0

#if DEBUG
#include <stdio.h>
#define SHOW(X) printf("X = %d\n", X)
#else
#define SHOW(X)
#endif
/***** LOCAL MACROS     *****/

#define SQUEEZE_X_ORU(A,B,C) ((((fix31)A * (fix31)B) + C) >> 16)
#define ABS(A) ((A < 0)? -A:A) /* absolute value */
#define IMPORT_FACTOR    \
	shift = 16;\
	while (*x_factor > (0x7fffffffL / (isw_scale >> (16 - shift))))\
		shift--;\
    	*x_factor = (*x_factor * (isw_scale>>(16-shift))) >> shift;

/***** GLOBAL VARIABLES *****/

/*****  GLOBAL FUNCTIONS *****/

/***** EXTERNAL VARIABLES *****/

/***** EXTERNAL FUNCTIONS *****/

/***** STATIC VARIABLES *****/

/***** STATIC FUNCTIONS *****/

static void sp_constr_update(PROTO_DECL1);
static ufix8 FONTFAR *sp_setup_pix_table(PROTO_DECL2 ufix8 FONTFAR *pointer,boolean short_form,fix15 no_X_ctrl_zones,fix15 no_Y_ctrl_zones);
static ufix8 FONTFAR *sp_setup_int_table(PROTO_DECL2 ufix8 FONTFAR *pointer,fix15 no_X_int_zones,fix15 no_Y_int_zones);


FUNCTION void init_tcb()
GDECL
/*
 * Called by sp_make_char() and make_comp_char() to initialize the current
 * transformation control block to the top level transformation.
 */
{
sp_globals.tcb = sp_globals.tcb0;
}

FUNCTION void scale_tcb(
GDECL
tcb_t GLOBALFAR *ptcb,    /* Transformation control block */
fix15  x_pos,   /* X position (outline res units) */
fix15  y_pos,   /* Y position (outline res units) */
fix15  x_scale, /* X scale factor * ONE_SCALE */
fix15  y_scale) /* Y scale factor * ONE_SCALE */
/*
 * Called by make_comp_char() to apply position and scale for each of the
 * components of a compound character.
 */
{     
fix15 xx_mult = ptcb->xxmult;
fix15 xy_mult = ptcb->xymult;
fix31 x_offset = ptcb->xoffset;
fix15 yx_mult = ptcb->yxmult;
fix15 yy_mult = ptcb->yymult;
fix31 y_offset = ptcb->yoffset;

ptcb->xxmult = TRANS(xx_mult, x_scale, (fix31)SCALE_RND, SCALE_SHIFT);
ptcb->xymult = TRANS(xy_mult, y_scale, (fix31)SCALE_RND, SCALE_SHIFT);
ptcb->xoffset = MULT16(xx_mult, x_pos) + MULT16(xy_mult, y_pos) + x_offset;
ptcb->yxmult = TRANS(yx_mult, x_scale, (fix31)SCALE_RND, SCALE_SHIFT);
ptcb->yymult = TRANS(yy_mult, y_scale, (fix31)SCALE_RND, SCALE_SHIFT);
ptcb->yoffset = MULT16(yx_mult, x_pos) + MULT16(yy_mult, y_pos) + y_offset;

type_tcb(ptcb); /* Reclassify transformation types */
}

FUNCTION ufix8 FONTFAR *skip_interpolation_table(
GDECL
ufix8 FONTFAR *pointer,  /* Pointer to next byte in char data */
ufix8    format)    /* Character format byte */
{
fix15 i,n;
ufix8 intsize[9];

intsize[0] = 1;
intsize[1] = 2;
intsize[2] = 3;
intsize[3] = 1;
intsize[4] = 2;
intsize[5] = 1;
intsize[6] = 2;
intsize[7] = 0;
intsize[8] = 0;

n =  ((format & BIT6)? (fix15)NEXT_BYTE(pointer): 0);
n += ((format & BIT7)? (fix15)NEXT_BYTE(pointer): 0);
for (i = 0; i < n; i++)          /* For each entry in int table ... */
    {
    format = NEXT_BYTE(pointer); /* Read format byte */
    if (format & BIT7)           /* Short Start/End point spec? */
        {
        pointer++;               /* Skip Start/End point byte */
        }
    else
        {
        pointer += intsize[format & 0x7];  /* Skip Start point spec */
        pointer += intsize[(format >> 3) & 0x7]; /* Skip End point spec */
        }
    }
return pointer;
}
FUNCTION ufix8 FONTFAR *skip_control_zone(
GDECL
ufix8 FONTFAR *pointer,  /* Pointer to next byte in char data */
ufix8    format)    /* Character format byte */
{
fix15    i,n;
ufix16   tmpufix16;

n = sp_globals.no_X_orus + sp_globals.no_Y_orus - 2;
for (i = 0; i < n; i++)          /* For each entry in control table ... */
    {
    if (format & BIT4)
        pointer++;               /* Skip short form From/To fields */
    else
        pointer += 2;            /* Skip FROM and TO fields */
    /* skip constraints field */
    NEXT_BYTES (pointer, tmpufix16);

    }
return pointer;
}

#if INCL_RULES
#else
FUNCTION ufix8 FONTFAR *plaid_tcb(
GDECL
ufix8 FONTFAR *pointer,  /* Pointer to next byte in char data */
ufix8    format)    /* Character format byte */
/* 
 * Called by make_simp_char() and make_comp_char() to set up the controlled
 * coordinate table and skip all other intelligent scaling rules embedded
 * in the character data.
 * Updates pointer to first byte after plaid data.
 * This is used only if intelligent scaling is not supported in the
 * configuration definitions.
 */
{
fix15  i, n;



sp_globals.no_X_orus = (format & BIT2)?
    (fix15)NEXT_BYTE(pointer):
    0;
sp_globals.no_Y_orus = (format & BIT3)?
    (fix15)NEXT_BYTE(pointer):
    0;
pointer = read_oru_table(pointer);        /* Updates no_X/Y/orus */
sp_globals.Y_edge_org = sp_globals.no_X_orus;

/* Skip over control zone table */
pointer = skip_control_zone(pointer,format);

/* Skip over interpolation table */
pointer = skip_interpolation_table(pointer,format);
return pointer;
}
#endif

#if INCL_RULES
FUNCTION ufix8 FONTFAR *plaid_tcb(
GDECL
ufix8 FONTFAR *pointer,  /* Pointer to next byte in char data */
ufix8    format)    /* Character format byte */
/* 
 * Called by make_simp_char() and make_comp_char() to set up the controlled
 * coordinate table and process all intelligent scaling rules embedded
 * in the character data.
 * Updates pointer to first byte after plaid data.
 * This is used only if intelligent scaling is enabled in the
 * configuration definitions.
 */
{
fix15 no_X_ctrl_zones;
fix15 no_Y_ctrl_zones;
fix15 no_X_int_zones;
fix15 no_Y_int_zones;

#if INCL_PLAID_OUT         /* Plaid data monitoring included? */
begin_plaid_data();
#endif

sp_constr_update();           /* Update constraint table if required */

sp_globals.no_X_orus = (format & BIT2)?  
    (fix15)NEXT_BYTE(pointer):
    0;
sp_globals.no_Y_orus = (format & BIT3)?
    (fix15)NEXT_BYTE(pointer):
    0;
pointer = read_oru_table(pointer);  /* Updates no_X/Y/orus to include zero values */
sp_globals.Y_edge_org = sp_globals.no_X_orus;                                                  
if (sp_globals.no_X_orus > 1)         /* 2 or more controlled X coordinates? */
    sp_globals.tcb.xmode = sp_globals.tcb.xtype; /* Enable intelligent scaling in X */

if (sp_globals.no_Y_orus > 1)         /* 2 or more controlled Y coordinates? */
    sp_globals.tcb.ymode = sp_globals.tcb.ytype; /* Enable intelligent scaling in Y */

no_X_ctrl_zones = sp_globals.no_X_orus - 1;
no_Y_ctrl_zones = sp_globals.no_Y_orus - 1;
pointer = sp_setup_pix_table(pointer, (boolean)(format & BIT4), 
    no_X_ctrl_zones, no_Y_ctrl_zones);

no_X_int_zones = (format & BIT6)?
    (fix15)NEXT_BYTE(pointer):
    0;
no_Y_int_zones = (format & BIT7)?
    (fix15)NEXT_BYTE(pointer):
    0;
sp_globals.Y_int_org = no_X_int_zones;
pointer = sp_setup_int_table(pointer, no_X_int_zones, no_Y_int_zones);

#if INCL_PLAID_OUT         /* Plaid data monitoring included? */
end_plaid_data();
#endif

return pointer;
}
#endif

#if INCL_RULES
FUNCTION static void sp_constr_update()
GDECL
/*
 * Called by plaid_tcb() to update the constraint table for the current
 * transformation.
 * This is always carried out whenever a character is generated following
 * a change of font or scale factor or after initialization.     
 */
{
fix31    ppo;
fix15    xppo;
fix15    yppo;
ufix8 FONTFAR  *pointer;
fix15    no_X_constr;
fix15    no_Y_constr;
fix15    i, j, k, l, n;
fix15    ppm;
ufix8    format;
ufix8    format1;
fix15    limit;
ufix16   constr_org;
fix15    constr_nr;
fix15    size;
fix31    off;
fix15    min;     
fix15    orus;
fix15    pix; 
ufix16   tmpufix16;  /* in extended mode, macro uses secnd term */

if (sp_globals.constr.data_valid &&         /* Constr table already done and ... */
    (sp_globals.tcb.xppo == sp_globals.constr.xppo) && /* ... X pix per oru unchanged and ... */
    (sp_globals.tcb.yppo == sp_globals.constr.yppo))   /* ... Y pix per oru unchanged? */
    {
    return;                      /* No need to update constraint table */
    }

sp_globals.constr.xppo = xppo = sp_globals.tcb.xppo;   /* Update X pixels per oru indicator */
sp_globals.constr.yppo = yppo = sp_globals.tcb.yppo;   /* Update Y pixels per oru indicator */
sp_globals.constr.data_valid = TRUE;        /* Mark constraint table valid */

pointer = sp_globals.constr.org;            /* Point to first byte of constraint data */
no_X_constr = NEXT_BYTES(pointer, tmpufix16); /* Read nmbr of X constraints */
no_Y_constr = NEXT_BYTES(pointer, tmpufix16); /* Read nmbr of Y constraints */

i = 0;
constr_org = 0;
n = no_X_constr;
ppo = xppo;
for (j = 0; ; j++)
    {
    sp_globals.c_act[i] = FALSE;            /* Flag constraint 0 not active */
    sp_globals.c_pix[i++] = 0;              /* Constraint 0 implies no minimum */
    sp_globals.c_act[i] = FALSE;            /* Flag constraint 1 not active */
    sp_globals.c_pix[i++] = sp_globals.onepix; /* Constraint 1 implies min 1 pixel*/
    ppm = (ppo * (fix31)sp_globals.orus_per_em) >> sp_globals.multshift;
    for (k = 0; k < n; k++)
        {
        format = NEXT_BYTE(pointer);        /* Read format byte */
        limit = (fix15)NEXT_BYTE(pointer);  /* Read limit field */
        sp_globals.c_act[i] = 
            ((ppm < limit) || (limit == 255)) &&
            sp_globals.constr.active;
        if (sp_globals.c_act[i])            /* Constraint active? */
            {
            if ((format & BIT1) &&          /* Constraint specified and ... */
                (constr_nr = constr_org +
                    ((format & BIT0)?       /* Read unsigned constraint value */
                    NEXT_WORD(pointer): 
                    (fix15)NEXT_BYTE(pointer)),
                 sp_globals.c_act[constr_nr])) /* ... and specified constraint active? */ 
                {
                pix = sp_globals.c_pix[constr_nr]; /* Use constrained pixel value */
                format1 = format;
                for (l = 2; l > 0; l--)     /* Skip 2 arguments */
                    {
                    format1 >>= 2;
                    if ((size = format1 & 0x03))
                        pointer += size - 1;
                    }
                }
            else                 /* Constraint absent or inactive? */
                {
                orus = (format & BIT2)? /* Read unsigned oru value */
                    NEXT_WORD(pointer):
                    (fix15)NEXT_BYTE(pointer);

                if (format & BIT5) /* Specified offset value? */
                    {
                    off = (fix31)((format & BIT4)? /* Read offset value */
                        NEXT_WORD(pointer):
                        (fix7)NEXT_BYTE(pointer));
                    off = (off << (sp_globals.multshift - 6)) + sp_globals.multrnd;
                    }
                else             /* Unspecified (zero) offset value? */
                    {
                    off = sp_globals.multrnd;
                    }

                pix = (fix15)(((fix31)orus * ppo + off) / (1 << sp_globals.mpshift)) & sp_globals.pixfix;
                }
            }
        else                     /* Constraint inactive? */
            {
            format1 = format;
            for (l = 3; l > 0; l--) /* Skip over 3 arguments */
                {
                if ((size = format1 & 0x03))
                    pointer += size - 1;
                format1 >>= 2;
                }
            pix = 0;
            }

        if (format & 0xc0) /* Specified minimum value? */
            {
            min = (format & BIT7)? /* Read unsigned minimum value */
                (fix15)NEXT_BYTE(pointer) << sp_globals.pixshift:
                sp_globals.onepix;
            }
        else             /* Unspecified (zero) minimum value? */
            {
            min = 0;
            }

        sp_globals.c_pix[i] = (pix < min)? min: pix;
        i++;
        }
    if (j) break;                /* Finished if second time around loop */
    constr_org = sp_globals.Y_constr_org = i;
    n = no_Y_constr;
    ppo = yppo;
    }

#if DEBUG
printf("\nCONSTRAINT TABLE\n");
n = no_X_constr + 2;
for (i = 0; i < n; i++)
    {
    printf("%3d   ", i);
    if (sp_globals.c_act[i])
        {
        printf("T ");
        }
    else
        {
        printf("F ");
        }
    printf("%5.1f\n", ((real)sp_globals.c_pix[i] / (real)sp_globals.onepix));
    }
printf("--------------\n");
n = no_Y_constr + 2;
for (i = 0; i < n; i++)
    {
    j = i + sp_globals.Y_constr_org;
    printf("%3d   ", i);
    if (sp_globals.c_act[j])
        {
        printf("T ");
        }
    else
        {
        printf("F ");
        }
    printf("%5.1f\n", ((real)sp_globals.c_pix[j] / (real)sp_globals.onepix));
    }
#endif

}
#endif

FUNCTION ufix8 FONTFAR *read_oru_table(
GDECL
ufix8 FONTFAR *pointer)   /* Pointer to first byte in controlled coord table */
/*
 * Called by plaid_tcb() to read the controlled coordinate table from the
 * character data in the font. 
 * Updates the pointer to the byte following the controlled coordinate
 * data.
 */
{
fix15    i, j, k, n;
boolean  zero_not_in;
boolean  zero_added;
fix15    oru;

#if INCL_RULES
fix15    pos;
#endif

i = 0;
n = sp_globals.no_X_orus;
#if INCL_RULES
pos = sp_globals.tcb.xpos;
#endif
for (j = 0; ; j++)
    {
    zero_not_in = TRUE;
    zero_added = FALSE;
    for (k = 0; k < n; k++)
        {
        oru = NEXT_WORD(pointer);
        if (zero_not_in && (oru >= 0)) /* First positive oru value? */
            {
#if INCL_RULES
            sp_plaid.pix[i] = pos;        /* Insert position in pix array */
#endif
            if (oru != 0)        /* Zero oru value omitted? */
                {
                sp_plaid.orus[i++] = 0;   /* Insert zero value in oru array */
                zero_added = TRUE; /* Remember to increment size of array */
                }
            zero_not_in = FALSE; /* Inhibit further testing for zero ins */
            }
        sp_plaid.orus[i++] = oru;         /* Add specified oru value to array */
        }
    if (zero_not_in)             /* All specified oru values negative? */
        {
#if INCL_RULES
        sp_plaid.pix[i] = pos;            /* Insert position in pix array */
#endif
        sp_plaid.orus[i++] = 0;           /* Add zero oru value */
        zero_added = TRUE;       /* Remember to increment size of array */
        }
    if (j)                       /* Both X and Y orus read? */
        break;
    if (zero_added)                                 
        sp_globals.no_X_orus++;             /* Increment X array size */
    n = sp_globals.no_Y_orus;               /* Prepare to read Y oru values */
#if INCL_RULES
    pos = sp_globals.tcb.ypos;
#endif
    }
if (zero_added)                  /* Zero Y oru value added to array? */
    sp_globals.no_Y_orus++;                 /* Increment Y array size */

#if DEBUG
printf("\nX ORUS\n");
n = sp_globals.no_X_orus;
for (i = 0; i < n; i++)
    {
    printf("%2d %4d\n", i, sp_plaid.orus[i]);
    }
printf("\nY ORUS\n");
n = sp_globals.no_Y_orus;
for (i = 0; i < n; i++)
    {
    printf("%2d %4d\n", i, sp_plaid.orus[i + sp_globals.no_X_orus]);
    }
#endif

return pointer;             /* Update pointer */
}
#if INCL_SQUEEZING || INCL_ISW
FUNCTION static void calculate_x_pix(
GDECL
ufix8 start_edge, ufix8 end_edge,
ufix16 constr_nr,
fix31 x_scale,
fix31 x_offset,
fix31 ppo,
fix15    setwidth_pix)
/*
 * Called by sp_setup_pix_table() when X squeezing is necessary
 * to insert the correct edge in the global pix array
 */
{
fix15 zone_pix;
fix15 start_oru, end_oru;

/* compute scaled oru coordinates */
start_oru= (fix15)(SQUEEZE_X_ORU(sp_plaid.orus[start_edge], x_scale, x_offset));
end_oru   = (fix15)(SQUEEZE_X_ORU(sp_plaid.orus[end_edge], x_scale, x_offset));

if (!sp_globals.c_act[constr_nr]) /* constraint inactive */
    {
    /* calculate zone width */
    zone_pix = (fix15)(((((fix31)end_oru - (fix31)start_oru) * ppo) /
	(1<<sp_globals.mpshift)) + sp_globals.pixrnd) & sp_globals.pixfix;
    /* check for overflow */
    if (((end_oru-start_oru) > 0) && (zone_pix < 0))
	zone_pix = 0x7ffff;
    /* check for minimum */
    if ((ABS(zone_pix)) >= sp_globals.c_pix[constr_nr])
    	goto Lx;
    }
/* use the zone size from the constr table - scale it */
zone_pix = (fix15)(((SQUEEZE_MULT(x_scale,sp_globals.c_pix[constr_nr]))
            + sp_globals.pixrnd) & sp_globals.pixfix);

/* look for overflow */
if ((sp_globals.c_pix[constr_nr] > 0) && (zone_pix < 0))
	zone_pix = 0x7fff;

if (start_edge > end_edge)
    {
    zone_pix = -zone_pix;
    }
Lx:
/* assign pixel value to global pix array */
sp_plaid.pix[end_edge]=sp_plaid.pix[start_edge] + zone_pix;

/* check for overflow */
if (((sp_plaid.pix[start_edge] >0) && (zone_pix >0)) &&
    (sp_plaid.pix[end_edge] < 0))
	sp_plaid.pix[end_edge] = 0x7fff; /* set it to the max */

/* be sure to be in the setwidth !*/
#if INCL_ISW
if (!sp_globals.import_setwidth_act) /* only check left edge if not isw only */
#endif
if ((sp_globals.pspecs->flags & SQUEEZE_LEFT) && (sp_plaid.pix[end_edge] < 0))
    sp_plaid.pix[end_edge] = 0;
if ((sp_globals.pspecs->flags & SQUEEZE_RIGHT) && 
    (sp_plaid.pix[end_edge] > setwidth_pix))
    sp_plaid.pix[end_edge] = setwidth_pix;

}
#endif

#if INCL_SQUEEZING
FUNCTION static void calculate_y_pix(
GDECL
ufix8 start_edge, ufix8 end_edge,
ufix16 constr_nr,
fix31 top_scale, fix31 bottom_scale,
fix31 ppo,
fix15 em_top_pix, fix15 em_bot_pix)

/*
 * Called by sp_setup_pix_table() when Y squeezing is necessary
 * to insert the correct edge in the global pix array
 */
{
fix15 zone_pix;
fix15 start_oru, end_oru;
fix31 zone_width, above_base, below_base;

/* check whether edge is above or below the baseline                */
/* and apply appropriate scale factor to get scaled oru coordinates */
if (sp_plaid.orus[start_edge] < 0)
    start_oru =(fix15)(SQUEEZE_MULT(sp_plaid.orus[start_edge], bottom_scale));
else
    start_oru =(fix15)(SQUEEZE_MULT(sp_plaid.orus[start_edge], top_scale));

if (sp_plaid.orus[end_edge] < 0)
    end_oru =(fix15)(SQUEEZE_MULT(sp_plaid.orus[end_edge], bottom_scale));
else
    end_oru =(fix15)(SQUEEZE_MULT(sp_plaid.orus[end_edge], top_scale));

if (!sp_globals.c_act[constr_nr])   /* Constraint inactive? */
   {
   /* calculate zone width */
    zone_pix = (fix15)(((((fix31)end_oru - (fix31)start_oru) * ppo)
		>> sp_globals.mpshift)+ sp_globals.pixrnd) & sp_globals.pixfix;
   /* check minimum */
    if ((ABS(zone_pix)) >= sp_globals.c_pix[constr_nr])
                    goto Ly;
    }

/* Use zone size from constr table */
if ((end_oru >= 0) && (start_oru >=0))
    /* all above baseline */
    zone_pix = (fix15)(SQUEEZE_MULT(top_scale, sp_globals.c_pix[constr_nr]));
else if ((end_oru <= 0) && (start_oru <=0))
    /* all below baseline */
    zone_pix = (fix15)(SQUEEZE_MULT(bottom_scale, sp_globals.c_pix[constr_nr]));
else
    {
    /* mixture */
    if (start_oru > 0)
        {
	zone_width = start_oru - end_oru;
        /* get % above baseline in 16.16 fixed point */
        above_base = (((fix31)start_oru) << 16) /
		     ((fix31)zone_width) ;
        /* get % below baseline in 16.16 fixed point */
        below_base = (((fix31)-end_oru) << 16) /
		     ((fix31)zone_width) ;
	}
    else
        {
        zone_width = end_oru - start_oru;
        /* get % above baseline in 16.16 fixed point */
        above_base = (((fix31)-start_oru) << 16) /
		     ((fix31)zone_width) ;
        /* get % below baseline in 16.16 fixed point */
        below_base = (((fix31)end_oru) << 16) /
		     ((fix31)zone_width) ;
       }
    /* % above baseline * total zone * top_scale +  */
    /* % below baseline * total zone * bottom_scale */
    zone_pix = ((((above_base * (fix31)sp_globals.c_pix[constr_nr]) >> 16) *
                top_scale) +
	       (((below_base * (fix31)sp_globals.c_pix[constr_nr]) >> 16) *
		bottom_scale)) >> 16;
    }

/* make this zone pix fall on a pixel boundary */
zone_pix = (zone_pix + sp_globals.pixrnd) & sp_globals.pixfix;

/* if minimum is in effect make the zone one pixel */
if ((sp_globals.c_pix[constr_nr] != 0) && (zone_pix < sp_globals.onepix)) 
    zone_pix = sp_globals.onepix; 
    
if (start_edge > end_edge) 
       {
        zone_pix = -zone_pix; /* Use negatve zone size */
        }
Ly:
/* assign global pix value */
sp_plaid.pix[end_edge] = sp_plaid.pix[start_edge] + zone_pix; /* Insert end pixels */

/* make sure it is in the EM !*/
if ((sp_globals.pspecs->flags & SQUEEZE_TOP) && 
    (sp_plaid.pix[end_edge] > em_top_pix))
    sp_plaid.pix[end_edge] = em_top_pix;
if ((sp_globals.pspecs->flags & SQUEEZE_BOTTOM) &&
    (sp_plaid.pix[end_edge] < em_bot_pix))
    sp_plaid.pix[end_edge] = em_bot_pix;
}

FUNCTION boolean calculate_x_scale(x_factor, x_offset, no_X_ctrl_zones)
GDECL
fix31 *x_factor,
fix31 *x_offset,
fix15   no_X_ctrl_zones) /* Number of X control zones */
/*
 * Called by sp_setup_pix_table() when squeezing is included
 * to determine whether X scaling is necessary.  If it is, the
 * scale factor and offset are computed.  This function returns
 * a boolean value TRUE = X squeezind is necessary, FALSE = no
 * X squeezing is necessary.
 */
{
boolean squeeze_left, squeeze_right;
boolean out_on_right, out_on_left;
fix15 bbox_width,set_width;
fix15 bbox_xmin, bbox_xmax;
fix15 x_offset_pix;
fix15 i;
#if INCL_ISW
fix31 isw_scale;
fix15 shift;
#endif


/* set up some flags and common calculations */
squeeze_left = (sp_globals.pspecs->flags & SQUEEZE_LEFT)? TRUE:FALSE;
squeeze_right = (sp_globals.pspecs->flags & SQUEEZE_RIGHT)? TRUE:FALSE;
bbox_xmin = sp_globals.bbox_xmin_orus;
bbox_xmax = sp_globals.bbox_xmax_orus;
set_width = sp_globals.setwidth_orus;

if (bbox_xmax > set_width)
    out_on_right = TRUE;
else
    out_on_right = FALSE;
if (bbox_xmin < 0)
    out_on_left = TRUE;
else
    out_on_left = FALSE;
bbox_width =bbox_xmax - bbox_xmin;

/*
 * don't need X squeezing if:
 *     - X squeezing not enabled
 *     - bbox doesn't violate on left or right
 *     - left squeezing only is enabled and char isn't out on left
 *     - right squeezing only is enabled and char isn't out on right
 */

if ((!squeeze_left && !squeeze_right) || 
   (!out_on_right && !out_on_left) ||     
   (squeeze_left && !squeeze_right && !out_on_left) ||
   (squeeze_right && !squeeze_left && !out_on_right))
    return FALSE;

#if INCL_ISW
if (sp_globals.import_setwidth_act)
    {
    /* if both isw and squeezing is going on - let the imported */
    /* setwidth factor be factored in with the squeeze          */
    isw_scale = compute_isw_scale();
    /*sp_globals.setwidth_orus = sp_globals.imported_width;*/
    }
else
    isw_scale = 0x10000L; /* 1 in 16.16 notation */
#endif

/* squeezing on left and right ?  */
if (squeeze_left && squeeze_right)
    {
    /* calculate scale factor */
    if (bbox_width < set_width)
	*x_factor = 0x10000L; /* 1 in 16.16 notation */
    else
	*x_factor = ((fix31)set_width<<16)/(fix31)bbox_width;
#if INCL_ISW
    IMPORT_FACTOR
#endif
    /* calculate offset */
    if (out_on_left) /* fall out on left ? */
	*x_offset = -(fix31)*x_factor * (fix31)bbox_xmin;
    /* fall out on right and I am shifting only ? */
    else if (out_on_right && (*x_factor == 0x10000L))
        *x_offset = -(fix31)*x_factor * (fix31)(bbox_xmax - set_width);
    else
	*x_offset = 0x0L; /* 0 in 16.16 notation */
    }
/* squeezing on left only and violates left */
else if (squeeze_left)
    {
    if (bbox_width < set_width) /* will it fit if I shift it ? */
	*x_factor = 0x10000L; /* 1 in 16.16 notation */
    else if (out_on_right)
	*x_factor = ((fix31)set_width<<16)/(fix31)bbox_width;
    else
	*x_factor = ((fix31)set_width<<16)/
		    (fix31)(bbox_width - (bbox_xmax-set_width));
#if INCL_ISW
    IMPORT_FACTOR
#endif
    *x_offset = (fix31)-*x_factor * (fix31)bbox_xmin;
    }

/* I must be squeezing on right, and violates right */
else 
    {
    if (bbox_width < set_width) /* will it fit if I shift it ? */
	{  /* just shift it left - it will fit in the bbox */
        *x_factor = 0x10000L; /* 1 in 16.16 notation */
#if INCL_ISW
    IMPORT_FACTOR
#endif
        *x_offset = (fix31)-*x_factor * (fix31)bbox_xmin;
	}
    else if (out_on_left)
	{
        *x_factor = ((fix31)set_width<<16)/(fix31)bbox_width;
#if INCL_ISW
    IMPORT_FACTOR
#endif
	*x_offset = 0x0L; /* 0 in 16.16 notation */
	}
    else
	{
        *x_factor = ((fix31)set_width<<16)/(fix31)bbox_xmax;
#if INCL_ISW
    IMPORT_FACTOR
#endif
	*x_offset = 0x0L; /* 0 in 16.16 notation */
 	}
    }

x_offset_pix = (fix15)(((*x_offset >> 16) * sp_globals.tcb0.xppo)
		/ (1<<sp_globals.mpshift)); 

if ((x_offset_pix >0) && (x_offset_pix < sp_globals.onepix))
    x_offset_pix = sp_globals.onepix; 

/* look for the first non-negative oru value, scale and add the offset    */
/* to the corresponding pixel value - note that the pixel value           */
/* is set in read_oru_table.                                              */

/* look at all the X edges */
for (i=0; i < (no_X_ctrl_zones+1); i++)
    if (sp_plaid.orus[i] >= 0)
        {
        sp_plaid.pix[i] = (SQUEEZE_MULT(sp_plaid.pix[i], *x_factor) 
		  +sp_globals.pixrnd + x_offset_pix) & sp_globals.pixfix;
        break;
        }

return TRUE;
}

FUNCTION boolean calculate_y_scale(
GDECL
fix31   *top_scale, fix31 *bottom_scale,
fix15  first_Y_zone,
fix15  no_Y_ctrl_zones)
/*
 * Called by sp_setup_pix_table() when squeezing is included
 * to determine whether Y scaling is necessary.  If it is, 
 * two scale factors are computed, one for above the baseline,
 * and one for below the basline.
 * This function returns a boolean value TRUE = Y squeezind is necessary, 
 * FALSE = no Y squeezing is necessary.
 */
{
boolean squeeze_top, squeeze_bottom;
boolean out_on_top, out_on_bottom;
fix15 	bbox_top, bbox_bottom;
fix15 	bbox_height;
fix15   i;

/* set up some flags and common calculations */
squeeze_top = (sp_globals.pspecs->flags & SQUEEZE_TOP)? TRUE:FALSE;
squeeze_bottom = (sp_globals.pspecs->flags & SQUEEZE_BOTTOM)? TRUE:FALSE;
bbox_top = sp_globals.bbox_ymax_orus;
bbox_bottom = sp_globals.bbox_ymin_orus;
bbox_height = bbox_top - bbox_bottom;

if (bbox_top > EM_TOP)
    out_on_top = TRUE;
else
    out_on_top = FALSE;

if (bbox_bottom < EM_BOT)
    out_on_bottom = TRUE;
else
    out_on_bottom = FALSE;

/*
 * don't need Y squeezing if:
 *     - Y squeezing not enabled
 *     - bbox doesn't violate on top or bottom
 *     - top squeezing only is enabled and char isn't out on top
 *     - bottom squeezing only is enabled and char isn't out on bottom
 */
if ((!squeeze_top && !squeeze_bottom) || 
    (!out_on_top && !out_on_bottom) ||
    (squeeze_top && !squeeze_bottom && !out_on_top) || 
    (squeeze_bottom && !squeeze_top && !out_on_bottom)) 
    return FALSE;

if (squeeze_top && (bbox_top > EM_TOP))
    *top_scale = ((fix31)EM_TOP << 16)/(fix31)(bbox_top);
else
    *top_scale = 0x10000L;  /* 1 in 16.16 fixed point */

if (squeeze_bottom && (bbox_bottom < EM_BOT))
    *bottom_scale = ((fix31)-(EM_BOT) << 16)/(fix31)-bbox_bottom;
else
    *bottom_scale = 0x10000L;

if (sp_globals.squeezing_compound)
    {
    for (i=first_Y_zone; i < (first_Y_zone + no_Y_ctrl_zones + 1); i++)
        {
        if (sp_plaid.orus[i] >= 0)
            sp_plaid.pix[i] = (SQUEEZE_MULT(sp_plaid.pix[i], *top_scale)
                              +sp_globals.pixrnd) & sp_globals.pixfix;
        else
            sp_plaid.pix[i] = (SQUEEZE_MULT(sp_plaid.pix[i], *bottom_scale)
                              +sp_globals.pixrnd) & sp_globals.pixfix;
        }
    }
return TRUE;
}
#endif

#if INCL_RULES
FUNCTION static ufix8 FONTFAR *sp_setup_pix_table(
GDECL
ufix8 FONTFAR *pointer,   /* Pointer to first byte in control zone table */
boolean short_form, /* TRUE if 1 byte from/to specification */
fix15   no_X_ctrl_zones, /* Number of X control zones */
fix15   no_Y_ctrl_zones) /* Number of Y control zones */
/*
 * Called by plaid_tcb() to read the control zone table from the
 * character data in the font.
 * Sets up a table of pixel values for all controlled coordinates. 
 * Updates the pointer to the byte following the control zone
 * data.
 */
{
fix15    i, j, n;
fix31    ppo;  
#if INCL_SQUEEZING || INCL_ISW
fix31    xppo0; /* top level pixels per oru */
fix31    yppo0; /* top level pixels per oru */
#endif
ufix8    edge_org;
ufix8    edge;
ufix8    start_edge;
ufix8    end_edge;
ufix16   constr_org;
fix15    constr_nr;
fix15    zone_pix;
fix31    whole_zone; /* non-transformed value of the first X zone */
ufix16   tmpufix16;  /* in extended mode, macro uses secnd term */
#if INCL_SQUEEZING
fix31    x_scale;
fix31	 y_top_scale, y_bottom_scale;
fix31    x_offset;
boolean  squeezed_y;
fix15    setwidth_pix, em_top_pix, em_bot_pix;
#endif

#if INCL_ISW
boolean  imported_width;
fix31	 isw_scale;
fix15    isw_setwidth_pix;
#endif

#if INCL_ISW || INCL_SQUEEZING
boolean squeezed_x;
#endif

#if INCL_PLAID_OUT               /* Plaid data monitoring included? */
begin_ctrl_zones(no_X_ctrl_zones, no_Y_ctrl_zones);
#endif                                                    


edge_org = 0;
constr_org = 0;
sp_globals.rnd_xmin = 0;  /* initialize the error for chars with no zone */
n = no_X_ctrl_zones;
ppo = sp_globals.tcb.xppo;
#if INCL_SQUEEZING || INCL_ISW
xppo0 = sp_globals.tcb0.xppo;
yppo0 = sp_globals.tcb0.yppo;
squeezed_x = FALSE;
#endif

#if INCL_SQUEEZING
squeezed_x = calculate_x_scale (&x_scale, &x_offset, no_X_ctrl_zones);
squeezed_y = calculate_y_scale(&y_top_scale,&y_bottom_scale,(n+1),
	     no_Y_ctrl_zones);
#if INCL_ISW
if (sp_globals.import_setwidth_act == TRUE)
setwidth_pix = ((fix15)(((fix31)sp_globals.imported_width * xppo0) >> 
	     sp_globals.mpshift) + sp_globals.pixrnd) & sp_globals.pixfix;

else
#endif
setwidth_pix = ((fix15)(((fix31)sp_globals.setwidth_orus * xppo0) >> 
	     sp_globals.mpshift) + sp_globals.pixrnd) & sp_globals.pixfix;
/* check for overflow */
if (setwidth_pix < 0)
	setwidth_pix = 0x7fff; /* set to maximum */
em_bot_pix = ((fix15)(((fix31)EM_BOT * yppo0) >> 
	     sp_globals.mpshift) + sp_globals.pixrnd) & sp_globals.pixfix;
em_top_pix = ((fix15)(((fix31)EM_TOP * yppo0) >> 
	     sp_globals.mpshift) + sp_globals.pixrnd) & sp_globals.pixfix;
#endif

#if INCL_ISW
/* convert to pixels */
isw_setwidth_pix = ((fix15)(((fix31)sp_globals.imported_width * xppo0) >> 
	     sp_globals.mpshift) + sp_globals.pixrnd) & sp_globals.pixfix;
/* check for overflow */
if (isw_setwidth_pix < 0)
	isw_setwidth_pix = 0x7fff; /* set to maximum */
if (!squeezed_x && ((imported_width = sp_globals.import_setwidth_act) == TRUE))
    {
    isw_scale = compute_isw_scale();

    /* look for the first non-negative oru value, scale and add the offset    */
    /* to the corresponding pixel value - note that the pixel value           */
    /* is set in read_oru_table.                                              */
    
    /* look at all the X edges */
        for (i=0; i < (no_X_ctrl_zones+1); i++)
        if (sp_plaid.orus[i] >= 0)
           {
           sp_plaid.pix[i] = (SQUEEZE_MULT(sp_plaid.pix[i], isw_scale)
                  +sp_globals.pixrnd) & sp_globals.pixfix;
           break;
           }

    }
#endif

for (i = 0; ; i++)               /* For X and Y control zones... */
    {
    for (j = 0; j < n; j++)      /* For each zone in X or Y... */
        {
        if (short_form)          /* 1 byte from/to specification? */
            {
            edge = NEXT_BYTE(pointer); /* Read packed from/to spec */
            start_edge = edge_org + (edge & 0x0f); /* Extract start edge */
            end_edge = edge_org + (edge >> 4); /* Extract end edge */
            }
        else                     /* 2 byte from/to specification? */
            {
            start_edge = edge_org + NEXT_BYTE(pointer); /* Read start edge */
            end_edge = edge_org + NEXT_BYTE(pointer); /* read end edge */
            }
        constr_nr = constr_org +
            NEXT_BYTES(pointer, tmpufix16); /* Read constraint number */ 
#if INCL_SQUEEZING
        if (i == 0 && squeezed_x)
	    calculate_x_pix(start_edge, end_edge, constr_nr,
                            x_scale, x_offset, ppo, setwidth_pix);
	else if (i == 1 && squeezed_y)
	    calculate_y_pix(start_edge, end_edge,constr_nr,
 		y_top_scale, y_bottom_scale, ppo, em_top_pix, em_bot_pix);
	else
	{
#endif
#if INCL_ISW
	if (i==0 && imported_width)
            calculate_x_pix(start_edge, end_edge, constr_nr,
                            isw_scale, 0,  ppo, isw_setwidth_pix);
	else
	{
#endif
        if (!sp_globals.c_act[constr_nr])   /* Constraint inactive? */
            {
            zone_pix = ((fix15)((((fix31)sp_plaid.orus[end_edge] -
			(fix31)sp_plaid.orus[start_edge]) * ppo) /
			(1<<sp_globals.mpshift)) + sp_globals.pixrnd) &
			sp_globals.pixfix;
            if ((ABS(zone_pix)) >= sp_globals.c_pix[constr_nr])
                goto L1;
            }
        zone_pix = sp_globals.c_pix[constr_nr]; /* Use zone size from constr table */
        if (start_edge > end_edge) /* sp_plaid.orus[start_edge] > sp_plaid.orus[end_edge]? */
            {
            zone_pix = -zone_pix; /* Use negatve zone size */
            }
    L1:
                        /* inter-character spacing fix */
        if ((j == 0) && (i == 0))      /* if this is the 1st X zone, save rounding error */
            {                          /*  get the non-xformed - xformed zone, in right direction */
            whole_zone = (((fix31)sp_plaid.orus[end_edge] -
			(fix31)sp_plaid.orus[start_edge]) *
			ppo) / (1<<sp_globals.mpshift);
            sp_globals.rnd_xmin = whole_zone - zone_pix;
            }
        sp_plaid.pix[end_edge] = sp_plaid.pix[start_edge] + zone_pix; /* Insert end pixels */
#if INCL_SQUEEZING
        if (i == 0)  /* in the x direction */
            { /* brute force squeeze */
            if ((sp_globals.pspecs->flags & SQUEEZE_LEFT) && 
                (sp_plaid.pix[end_edge] < 0))
                sp_plaid.pix[end_edge] = 0;
            if ((sp_globals.pspecs->flags & SQUEEZE_RIGHT) && 
                (sp_plaid.pix[end_edge] > setwidth_pix))
                sp_plaid.pix[end_edge] = setwidth_pix;
            }
        if (i == 1) /* in the y direction */
            {  /* brute force squeeze */
            if ((sp_globals.pspecs->flags & SQUEEZE_TOP) && 
                (sp_plaid.pix[end_edge] > em_top_pix))
                sp_plaid.pix[end_edge] = em_top_pix;
            if ((sp_globals.pspecs->flags & SQUEEZE_BOTTOM) &&
                (sp_plaid.pix[end_edge] < em_bot_pix))
                sp_plaid.pix[end_edge] = em_bot_pix;
            }
#endif
#if INCL_SQUEEZING
	}
#endif
#if INCL_ISW
	}
#endif
#if INCL_PLAID_OUT               /* Plaid data monitoring included? */
        record_ctrl_zone(
            (fix31)sp_plaid.pix[start_edge] << (16 - sp_globals.pixshift), 
            (fix31)sp_plaid.pix[end_edge] << (16 - sp_globals.pixshift), 
            (fix15)(constr_nr - constr_org));
#endif
        }
    if (i)                       /* Y pixels done? */
        break;                                          
    edge_org = sp_globals.Y_edge_org;       /* Prepare to process Y ctrl zones */
    constr_org = sp_globals.Y_constr_org;
    n = no_Y_ctrl_zones;                      
    ppo = sp_globals.tcb.yppo;                            
    }

#if DEBUG
printf("\nX PIX TABLE\n");
n = no_X_ctrl_zones + 1;
for (i = 0; i < n; i++)
    printf("%2d %6.1f\n", i, (real)sp_plaid.pix[i] / (real)sp_globals.onepix);
printf("\nY PIX TABLE\n");
n = no_Y_ctrl_zones + 1;
for (i = 0; i < n; i++)
    {
    j = i + no_X_ctrl_zones + 1;
    printf("%2d %6.1f\n", i, (real)sp_plaid.pix[j] / (real)sp_globals.onepix);
    }
#endif

return pointer;
}
#endif


#if INCL_RULES
FUNCTION static ufix8 FONTFAR *sp_setup_int_table(
GDECL
ufix8 FONTFAR *pointer,   /* Pointer to first byte in interpolation zone table */
fix15  no_X_int_zones, /* Number of X interpolation zones */
fix15  no_Y_int_zones) /* Number of X interpolation zones */
/*
 * Called by plaid_tcb() to read the interpolation zone table from the
 * character data in the font. 
 * Sets up a table of interpolation coefficients with one entry for
 * every X or Y interpolation zone.
 * Updates the pointer to the byte following the interpolation zone
 * data.
 */
{
fix15    i, j, k, l, n;
ufix8    format;
ufix8    format_copy;
ufix8    tmpufix8;
fix15    start_orus = 0;
ufix8    edge_org;
ufix8    edge;
ufix16   adj_factor;
fix15    adj_orus;
fix15    end_orus = 0;
fix31    zone_orus;
fix15    start_pix = 0;
fix15    end_pix = 0;


#if INCL_PLAID_OUT               /* Plaid data monitoring included? */
begin_int_zones(no_X_int_zones, no_Y_int_zones);
#endif

i = 0;
edge_org = 0;
n = no_X_int_zones;
for (j = 0; ; j++)
    {
    for (k = 0; k < n; k++)
        {
        format = NEXT_BYTE(pointer);
        if (format & BIT7)       /* Short start/end point spec? */
            {
            tmpufix8 = NEXT_BYTE(pointer);
            edge = edge_org + (tmpufix8 & 0xf);
            start_orus = sp_plaid.orus[edge];
            start_pix = sp_plaid.pix[edge];
            edge = edge_org + (tmpufix8 >> 4);
            end_orus = sp_plaid.orus[edge];
            end_pix = sp_plaid.pix[edge];
            }
        else                     /* Standard start and end point spec? */
            {
            format_copy = format;
            for (l = 0; ; l++)   /* Loop for start and end point */
                {
                switch (format_copy & 0x7) /* Decode start/end point format */
                    {

                case 0:          /* Index to control edge */
                    edge = edge_org + NEXT_BYTE(pointer);
                    end_orus = sp_plaid.orus[edge];
                    end_pix = sp_plaid.pix[edge];
                    break;

                case 1:          /* 1 byte fractional distance to next edge */
                    adj_factor =  0xffff & NEXT_BYTE(pointer) << 8;
                    goto L1;


                case 2:          /* 2 byte fractional distance to next edge */
		    adj_factor = 0xffff & NEXT_WORD(pointer);
                L1: edge = edge_org + NEXT_BYTE(pointer);
                    end_orus = sp_plaid.orus[edge] +
                        ((((fix31)sp_plaid.orus[edge + 1] - (fix31)sp_plaid.orus[edge]) * 
                        (ufix32)adj_factor + (fix31)32768) >> 16);
                    end_pix = sp_plaid.pix[edge] +
                        ((((fix31)sp_plaid.pix[edge + 1] - (fix31)sp_plaid.pix[edge]) * 
                        (ufix32)adj_factor + (fix31)32768) >> 16);
                    break;

                case 3:          /* 1 byte delta orus before first edge */
                    adj_orus = -(fix15)NEXT_BYTE(pointer); 
                    goto L2;

                case 4:          /* 2 byte delta orus before first edge */
                    adj_orus = -NEXT_WORD(pointer);
                L2: edge = edge_org;
                    goto L4;

                case 5:          /* 1 byte delta orus after last edge */
                    adj_orus = (fix15)NEXT_BYTE(pointer);
                    goto L3;

                case 6:          /* 2 byte delta orus after last edge */
                    adj_orus = NEXT_WORD(pointer);
                L3: edge = j? sp_globals.Y_edge_org + sp_globals.no_Y_orus - 1: sp_globals.no_X_orus - 1;
                L4: end_orus = sp_plaid.orus[edge] + adj_orus;
                    end_pix = sp_plaid.pix[edge] + 
                        (((fix31)adj_orus * (fix31)(j? sp_globals.tcb.yppo: sp_globals.tcb.xppo) + 
                          sp_globals.mprnd) / (1<<sp_globals.mpshift));
                    break;

                    }

                if (l)           /* Second time round loop? */
                    break;
                format_copy >>= 3; /* Adj format to decode end point format */
                start_orus = end_orus; /* Save start point oru value */
                start_pix = end_pix; /* Save start point pixel value */
                }
            }
#if INCL_PLAID_OUT               /* Plaid data monitoring included? */
        record_int_zone(
            (fix31)start_pix << (16 - sp_globals.pixshift), 
            (fix31)end_pix << (16 - sp_globals.pixshift));
#endif
        zone_orus = (fix31)end_orus - (fix31)start_orus;
        sp_plaid.mult[i] = ((((fix31)end_pix - (fix31)start_pix) << sp_globals.mpshift) + 
                   (zone_orus / 2)) / zone_orus;
        sp_plaid.offset[i] = 
            (((((fix31)start_pix + (fix31)end_pix) << sp_globals.mpshift) - 
              ((fix31)sp_plaid.mult[i] * ((fix31)start_orus + (fix31)end_orus))) / 2) + 
            sp_globals.mprnd;
        i++;
        }
    if (j)                       /* Finished? */
        break;
    edge_org = sp_globals.Y_edge_org;       /* Prepare to process Y ctrl zones */
    n = no_Y_int_zones;
    }

#if DEBUG
printf("\nX INT TABLE\n");
n = no_X_int_zones;
for (i = 0; i < n; i++)
    {
    printf("%2d %7.4f %7.4f\n", i, 
        (real)sp_plaid.mult[i] / (real)(1 << sp_globals.multshift), 
        (real)sp_plaid.offset[i] / (real)(1 << sp_globals.multshift));
    }
printf("\nY INT TABLE\n");
n = no_Y_int_zones;
for (i = 0; i < n; i++)
    {
    j = i + no_X_int_zones;
    printf("%2d %7.4f %7.4f\n", i, 
        (real)sp_plaid.mult[j] / (real)(1 << sp_globals.multshift), 
        (real)sp_plaid.offset[j] / (real)(1 << sp_globals.multshift));
    }
#endif

return pointer;
}
#endif
#if INCL_ISW
FUNCTION fix31 compute_isw_scale()
GDECL
{
fix31 isw_scale;
	
if (sp_globals.setwidth_orus == 0)
    isw_scale = 0x00010000;
else
    isw_scale = ((fix31)sp_globals.imported_width << 16)/
                 (fix31)sp_globals.setwidth_orus;
return isw_scale;
}
#endif
