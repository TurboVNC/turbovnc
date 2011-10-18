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
/* $XConsortium: cfbbres.c,v 1.15 94/04/17 20:28:45 dpw Exp $ */
/* $XFree86: xc/programs/Xserver/cfb/cfbbres.c,v 3.1 1996/08/25 14:05:40 dawes Exp $ */
#include "X.h"
#include "misc.h"
#include "cfb.h"
#include "cfbmskbits.h"
#include "servermd.h"
#include "miline.h"

/* Solid bresenham line */
/* NOTES
   e2 is used less often than e1, so it's not in a register
*/

void
cfbBresS(rop, and, xor, addrl, nlwidth, signdx, signdy, axis, x1, y1, e, e1,
	 e2, len)
    int		    rop;
    unsigned long   and, xor;
    unsigned long   *addrl;		/* pointer to base of bitmap */
    int		    nlwidth;		/* width in longwords of bitmap */
    register int    signdx;
    int		    signdy;		/* signs of directions */
    int		    axis;		/* major axis (Y_AXIS or X_AXIS) */
    int		    x1, y1;		/* initial point */
    register int    e;			/* error accumulator */
    register int    e1;			/* bresenham increments */
    int		    e2;
    int		    len;		/* length of line */
{
    register int	e3 = e2-e1;
#if PSZ == 24
    unsigned long piQxelXor[3],piQxelAnd[3];
    char *addrb;
    int nlwidth3, signdx3;
#endif
#ifdef PIXEL_ADDR
    register PixelType	*addrp;		/* Pixel pointer */

    if (len == 0)
    	return;
    /* point to first point */
    nlwidth <<= PWSH;
#if PSZ == 24
    addrp = (PixelType *)(addrl) + (y1 * nlwidth);
    addrb = (char *)addrp + x1 * 3;

    piQxelXor[0] = (xor << 24) | xor;
    piQxelXor[1] = (xor << 16)| (xor >> 8);
    piQxelXor[2] = (xor << 8) | (xor >> 16);
    piQxelAnd[0] = (and << 24) | and;
    piQxelAnd[1] = (and << 16)| (and >> 8);
    piQxelAnd[2] = (and << 8) | (and >> 16);
#else
    addrp = (PixelType *)(addrl) + (y1 * nlwidth) + x1;
#endif
    if (signdy < 0)
    	nlwidth = -nlwidth;
    e = e-e1;			/* to make looping easier */
#if PSZ == 24
    nlwidth3 = nlwidth * sizeof (long);
    signdx3 = signdx * 3;
#endif
    
    if (axis == Y_AXIS)
    {
	int	t;

	t = nlwidth;
	nlwidth = signdx;
	signdx = t;
#if PSZ == 24
	t = nlwidth3;
	nlwidth3 = signdx3;
	signdx3 = t;
#endif
    }
    if (rop == GXcopy)
    {
	--len;
#if PSZ == 24
#define body_copy \
	    addrp = (PixelType *)((unsigned long)addrb & ~0x03); \
	    switch((unsigned long)addrb & 3){ \
	    case 0: \
	      *addrp = ((*addrp)&0xFF000000)|(piQxelXor[0] & 0xFFFFFF); \
	      break; \
	    case 1: \
	      *addrp = ((*addrp)&0xFF)|(piQxelXor[2] & 0xFFFFFF00); \
	      break; \
	    case 3: \
	      *addrp = ((*addrp)&0xFFFFFF)|(piQxelXor[0] & 0xFF000000); \
	      *(addrp+1) = ((*(addrp+1))&0xFFFF0000)|(piQxelXor[1] & 0xFFFF); \
	      break; \
	    case 2: \
	      *addrp = ((*addrp)&0xFFFF)|(piQxelXor[1] & 0xFFFF0000); \
	      *(addrp+1) = ((*(addrp+1))&0xFFFFFF00)|(piQxelXor[2] & 0xFF); \
	      break; \
	    }
#define body {\
	    body_copy \
	    addrb += signdx3; \
	    e += e1; \
	    if (e >= 0) \
	    { \
		addrb += nlwidth3; \
		e += e3; \
	     } \
	    }
#else /* PSZ == 24 */
#define body {\
	    *addrp = xor; \
	    addrp += signdx; \
	    e += e1; \
	    if (e >= 0) \
	    { \
		addrp += nlwidth; \
		e += e3; \
	    } \
	}
#endif /* PSZ == 24 */
	while (len >= 4)
	{
	    body body body body
	    len -= 4;
	}
	switch (len)
	{
	case  3: body case  2: body case  1: body
	}
#undef body
#if PSZ == 24
	body_copy
# undef body_copy
#else
	*addrp = xor;
#endif
    }
    else /* not GXcopy */
    {
	while(len--)
	{ 
#if PSZ == 24
	    addrp = (PixelType *)((unsigned long)addrb & ~0x03);
	    switch((unsigned long)addrb & 3){
	    case 0:
	      *addrp = (*addrp & (piQxelAnd[0]|0xFF000000))
			^ (piQxelXor[0] & 0xFFFFFF);
	      break;
	    case 1:
	      *addrp = (*addrp & (piQxelAnd[2]|0xFF))
			^ (piQxelXor[2] & 0xFFFFFF00);
	      break;
	    case 3:
	      *addrp = (*addrp & (piQxelAnd[0]|0xFFFFFF))
			^ (piQxelXor[0] & 0xFF000000);
	      *(addrp+1) = (*(addrp+1) & (piQxelAnd[1]|0xFFFF0000))
			^ (piQxelXor[1] & 0xFFFF);
	      break;
	    case 2:
	      *addrp = (*addrp & (piQxelAnd[1]|0xFFFF))
			^ (piQxelXor[1] & 0xFFFF0000);
	      *(addrp+1) = (*(addrp+1) & (piQxelAnd[2]|0xFFFFFF00))
			^ (piQxelXor[2] & 0xFF);
	      break;
	    }
	    e += e1;
	    if (e >= 0)
	    {
		addrb += nlwidth3;
		e += e3;
	    }
	    addrb += signdx3;
#else /* PSZ == 24 */
	    *addrp = DoRRop (*addrp, and, xor);
	    e += e1;
	    if (e >= 0)
	    {
		addrp += nlwidth;
		e += e3;
	    }
	    addrp += signdx;
#endif /* PSZ == 24 */
	}
    }
#else /* !PIXEL_ADDR */
    register unsigned long   tmp, bit;
    unsigned long leftbit, rightbit;

    /* point to longword containing first point */
#if PSZ == 24
    addrl = (addrl + (y1 * nlwidth) + ((x1 * 3) >>2);
#else
    addrl = (addrl + (y1 * nlwidth) + (x1 >> PWSH));
#endif
    if (signdy < 0)
	    nlwidth = -nlwidth;
    e = e-e1;			/* to make looping easier */

    leftbit = cfbmask[0];
#if PSZ == 24
    rightbit = cfbmask[(PPW-1)<<1];
    bit = cfbmask[(x1 & 3)<<1];
#else
    rightbit = cfbmask[PPW-1];
    bit = cfbmask[x1 & PIM];
#endif

    if (axis == X_AXIS)
    {
	if (signdx > 0)
	{
	    while (len--)
	    { 
		*addrl = DoMaskRRop (*addrl, and, xor, bit);
		bit = SCRRIGHT(bit,1);
		e += e1;
		if (e >= 0)
		{
		    addrl += nlwidth;
		    e += e3;
		}
		if (!bit)
		{
		    bit = leftbit;
		    addrl++;
		}
	    }
	}
	else
	{
	    while (len--)
	    { 
		*addrl = DoMaskRRop (*addrl, and, xor, bit);
		e += e1;
		bit = SCRLEFT(bit,1);
		if (e >= 0)
		{
		    addrl += nlwidth;
		    e += e3;
		}
		if (!bit)
		{
		    bit = rightbit;
		    addrl--;
		}
	    }
	}
    } /* if X_AXIS */
    else
    {
	if (signdx > 0)
	{
	    while(len--)
	    {
		*addrl = DoMaskRRop (*addrl, and, xor, bit);
		e += e1;
		if (e >= 0)
		{
		    bit = SCRRIGHT(bit,1);
		    if (!bit)
		    {
			bit = leftbit;
			addrl++;
		    }
		    e += e3;
		}
		addrl += nlwidth;
	    }
	}
	else
	{
	    while(len--)
	    {
		*addrl = DoMaskRRop (*addrl, and, xor, bit);
		e += e1;
		if (e >= 0)
		{
		    bit = SCRLEFT(bit,1);
		    if (!bit)
		    {
			bit = rightbit;
			addrl--;
		    }
		    e += e3;
		}
		addrl += nlwidth;
	    }
	}
    } /* else Y_AXIS */
#endif
}
