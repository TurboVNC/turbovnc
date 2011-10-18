/* Combined Purdue/PurduePlus patches, level 2.0, 1/17/89 */
/***********************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $XConsortium: mfbbres.c,v 1.22 94/04/17 20:28:17 dpw Exp $ */
#include "X.h"
#include "misc.h"
#include "mfb.h"
#include "maskbits.h"
#include "miline.h"

/* Solid bresenham line */
/* NOTES
   e2 is used less often than e1, so it's not in a register
*/

void
mfbBresS(rop, addrlbase, nlwidth, signdx, signdy, axis, x1, y1, e, e1, e2, len)
int rop;		/* a reduced rasterop */
PixelType *addrlbase;	/* pointer to base of bitmap */
int nlwidth;		/* width in longwords of bitmap */
int signdx, signdy;	/* signs of directions */
int axis;		/* major axis (Y_AXIS or X_AXIS) */
int x1, y1;		/* initial point */
register int e;		/* error accumulator */
register int e1;	/* bresenham increments */
int e2;
int len;		/* length of line */
{
    register int yinc;	/* increment to next scanline, in bytes */
    register PixelType *addrl;	/* bitmask long pointer */
    register PixelType bit;	/* current bit being set/cleared/etc.  */
    PixelType leftbit = mask[0]; /* leftmost bit to process in new word */
    PixelType rightbit = mask[PPW-1]; /* rightmost bit to process in new word */

    register int e3 = e2-e1;
    PixelType	tmp;

    /* point to longword containing first point */
    addrl = mfbScanline(addrlbase, x1, y1, nlwidth);
    yinc = signdy * nlwidth;
    e = e-e1;			/* to make looping easier */
    bit = mask[x1 & PIM];

    if (!len)
	return;
    if (rop == RROP_BLACK)
    {
        if (axis == X_AXIS)
        {
	    if (signdx > 0)
	    {
		tmp = *addrl;
		for (;;)
		{ 
		    tmp &= ~bit;
		    if (!--len)
			break;
		    bit = SCRRIGHT(bit,1);
		    e += e1;
 		    if (e >= 0)
		    {
			*addrl = tmp;
			mfbScanlineInc(addrl, yinc);
			e += e3;
			if (!bit)
			{
			    bit = leftbit;
			    addrl ++;
			}
			tmp = *addrl;
		    }
		    else if (!bit)
 		    {
			*addrl = tmp;
			bit = leftbit;
			addrl ++;
			tmp = *addrl;
		    }
		}
		*addrl = tmp;
	    }
	    else
	    {
		tmp = *addrl;
		for (;;)
		{ 
		    tmp &= ~bit;
		    if (!--len)
			break;
		    e += e1;
		    bit = SCRLEFT(bit,1);
		    if (e >= 0)
		    {
			*addrl = tmp;
			mfbScanlineInc(addrl, yinc);
			e += e3;
			if (!bit)
			{
			    bit = rightbit;
			    addrl --;
			}
			tmp = *addrl;
		    }
		    else if (!bit)
 		    {
			*addrl = tmp;
			bit = rightbit;
			addrl --;
			tmp = *addrl;
		    }
		}
		*addrl = tmp;
	    }
        } /* if X_AXIS */
        else
        {
	    if (signdx > 0)
	    {
		while(len--)
		{
		    *addrl &= ~bit;
		    e += e1;
		    if (e >= 0)
		    {
			bit = SCRRIGHT(bit,1);
			if (!bit) { bit = leftbit;addrl ++; }
			e += e3;
		    }
		    mfbScanlineInc(addrl, yinc);
		}
	    }
	    else
	    {
		while(len--)
		{
		    *addrl &= ~bit;
		    e += e1;
		    if (e >= 0)
		    {
			bit = SCRLEFT(bit,1);
			if (!bit) { bit = rightbit;addrl --; }
			e += e3;
		    }
		    mfbScanlineInc(addrl, yinc);
		}
	    }
        } /* else Y_AXIS */
    } 
    else if (rop == RROP_WHITE)
    {
        if (axis == X_AXIS)
        {
	    if (signdx > 0)
	    {
		tmp = *addrl;
		for (;;)
		{
		    tmp |= bit;
		    if (!--len)
			break;
		    e += e1;
		    bit = SCRRIGHT(bit,1);
		    if (e >= 0)
		    {
			*addrl = tmp;
			mfbScanlineInc(addrl, yinc);
			e += e3;
			if (!bit)
			{
			    bit = leftbit;
			    addrl ++;
			}
			tmp = *addrl;
		    }
		    else if (!bit)
 		    {
			*addrl = tmp;
			bit = leftbit;
			addrl ++;
			tmp = *addrl;
		    }
		}
		*addrl = tmp;
	    }
	    else
	    {
		tmp = *addrl;
		for (;;)
		{
		    tmp |= bit;
		    if (!--len)
			break;
		    e += e1;
		    bit = SCRLEFT(bit,1);
		    if (e >= 0)
		    {
			*addrl = tmp;
			mfbScanlineInc(addrl, yinc);
			e += e3;
			if (!bit)
			{
			    bit = rightbit;
			    addrl --;
			}
			tmp = *addrl;
		    }
		    else if (!bit)
		    {
			*addrl = tmp;
			bit = rightbit;
			addrl --;
			tmp = *addrl;
		    }
		}
		*addrl = tmp;
	    }
        } /* if X_AXIS */
        else
        {
	    if (signdx > 0)
	    {
		while(len--)
		{
		    *addrl |= bit;
		    e += e1;
		    if (e >= 0)
		    {
			bit = SCRRIGHT(bit,1);
			if (!bit) { bit = leftbit;addrl ++; }
			e += e3;
		    }
		    mfbScanlineInc(addrl, yinc);
		}
	    }
	    else
	    {
		while(len--)
		{
		    *addrl |= bit;
		    e += e1;
		    if (e >= 0)
		    {
			bit = SCRLEFT(bit,1);
			if (!bit) { bit = rightbit;addrl --; }
			e += e3;
		    }
		    mfbScanlineInc(addrl, yinc);
		}
	    }
        } /* else Y_AXIS */
    }
    else if (rop == RROP_INVERT)
    {
        if (axis == X_AXIS)
        {
	    if (signdx > 0)
	    {
		while(len--)
		{
		    *addrl ^= bit;
		    e += e1;
		    if (e >= 0)
		    {
			mfbScanlineInc(addrl, yinc);
			e += e3;
		    }
		    bit = SCRRIGHT(bit,1);
		    if (!bit) { bit = leftbit;addrl ++; }
		}
	    }
	    else
	    {
		while(len--)
		{
		    *addrl ^= bit;
		    e += e1;
		    if (e >= 0)
		    {
			mfbScanlineInc(addrl, yinc);
			e += e3;
		    }
		    bit = SCRLEFT(bit,1);
		    if (!bit) { bit = rightbit;addrl --; }
		}
	    }
        } /* if X_AXIS */
        else
        {
	    if (signdx > 0)
	    {
		while(len--)
		{
		    *addrl ^= bit;
		    e += e1;
		    if (e >= 0)
		    {
			bit = SCRRIGHT(bit,1);
			if (!bit) { bit = leftbit;addrl ++; }
			e += e3;
		    }
		    mfbScanlineInc(addrl, yinc);
		}
	    }
	    else
	    {
		while(len--)
		{
		    *addrl ^= bit;
		    e += e1;
		    if (e >= 0)
		    {
			bit = SCRLEFT(bit,1);
			if (!bit) { bit = rightbit; addrl --; }
			e += e3;
		    }
		    mfbScanlineInc(addrl, yinc);
		}
	    }
        } /* else Y_AXIS */
    }
} 
