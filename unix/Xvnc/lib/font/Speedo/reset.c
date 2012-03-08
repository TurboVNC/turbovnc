/* $Xorg: reset.c,v 1.3 2000/08/17 19:46:26 cpqbld Exp $ */

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
/* $XFree86: xc/lib/font/Speedo/reset.c,v 1.3 2001/01/17 19:43:17 dawes Exp $ */



/******************************* R E S E T . C *******************************
 *                                                                           *
 * This module provides initialization functions.                            *
 *                                                                           *
 ****************************************************************************/

#include "spdo_prv.h"               /* General definitions for Speedo     */
#include "keys.h"                /* Font decryption keys */

#define   DEBUG      0

#if DEBUG
#include <stdio.h>
#define SHOW(X) printf("X = %d\n", X)
#else
#define SHOW(X)
#endif

/***** GLOBAL VARIABLES *****/

/*****  GLOBAL FUNCTIONS *****/

/***** EXTERNAL VARIABLES *****/

/***** EXTERNAL FUNCTIONS *****/

/***** STATIC VARIABLES *****/

/***** STATIC FUNCTIONS *****/


FUNCTION void reset()
GDECL
/*
 * Called by the host software to intialize the Speedo mechanism
 */
{
sp_globals.specs_valid = FALSE;            /* Flag specs not valid */

/* Reset decryption key */
sp_globals.key32 = (KEY3 << 8) | KEY2;
sp_globals.key4 = KEY4;
sp_globals.key6 = KEY6;
sp_globals.key7 = KEY7;
sp_globals.key8 = KEY8;

#if INCL_RULES
sp_globals.constr.font_id_valid = FALSE;
#endif

#if INCL_MULTIDEV
#if INCL_BLACK || INCL_SCREEN || INCL_2D
sp_globals.bitmap_device_set = FALSE;
#endif
#if INCL_OUTLINE
sp_globals.outline_device_set = FALSE;
#endif
#endif
}

#if INCL_KEYS
FUNCTION void set_key(
GDECL
ufix8 key[])         /* Specified decryption key */
/*
 * Dynamically sets font decryption key.
 */
{
sp_globals.key32 = ((ufix16)key[3] << 8) | key[2];
sp_globals.key4 = key[4];
sp_globals.key6 = key[6];
sp_globals.key7 = key[7];
sp_globals.key8 = key[8];
}
#endif



FUNCTION ufix16 get_cust_no(
GDECL
buff_t font_buff)
/*
	returns customer number from font 
*/
{ 
ufix8 FONTFAR *hdr2_org;
ufix16 private_off;

private_off = read_word_u(font_buff.org + FH_HEDSZ);
if (private_off + FH_CUSNR > font_buff.no_bytes)
	{
	report_error(1);           /* Insufficient font data loaded */
    return FALSE;
    }

hdr2_org = font_buff.org + private_off;

return (read_word_u(hdr2_org + FH_CUSNR));
}


