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
/* $XConsortium: cfbhrzvert.c,v 1.8 94/04/17 20:28:51 dpw Exp $ */
/* $XFree86: xc/programs/Xserver/cfb/cfbhrzvert.c,v 3.2 1996/11/24 09:51:43 dawes Exp $ */
#include "X.h"

#include "gc.h"
#include "window.h"
#include "pixmap.h"
#include "region.h"

#include "cfb.h"
#include "cfbmskbits.h"

/* horizontal solid line
   abs(len) > 1
*/
cfbHorzS(rop, and, xor, addrl, nlwidth, x1, y1, len)
register int rop;
register unsigned long and;
register unsigned long xor;
register unsigned long *addrl;	/* pointer to base of bitmap */
int nlwidth;		/* width in longwords of bitmap */
int x1;			/* initial point */ 
int y1;
int len;		/* length of line */
{
    register int nlmiddle;
    register unsigned long startmask;
    register unsigned long endmask;
#if PSZ == 24
    int leftIndex, rightIndex, xOffset;
    unsigned long piQxelAnd[3], piQxelXor[3];
    piQxelAnd[0] = (and & 0xFFFFFF) | ((and<<24)  & 0xFF000000);
    piQxelAnd[1] = ((and>>8)  & 0xFFFF)| ((and<<16) & 0xFFFF0000);
    piQxelAnd[2] = ((and<<8) & 0xFFFFFF00) | ((and>>16) & 0xFF);

    piQxelXor[0] = (xor & 0xFFFFFF) | ((xor<<24) & 0xFF000000);
    piQxelXor[1] = ((xor>>8)  & 0xFFFF)| ((xor<<16) & 0xFFFF0000);
    piQxelXor[2] = ((xor<<8) & 0xFFFFFF00) | ((xor>>16) & 0xFF);

    leftIndex = x1 & 3;
    rightIndex = ((x1 + len) < 5)?0:(x1 + len)&3;
    nlmiddle = len;
    if(leftIndex){
      nlmiddle -= (4 - leftIndex);
    }
    if(rightIndex){
      nlmiddle -= rightIndex;
    }
    if (nlmiddle < 0)
      nlmiddle = 0;

    nlmiddle >>= 2;

    addrl += (y1 * nlwidth) + (x1 >> 2)*3 + (leftIndex?leftIndex-1:0);

    switch(leftIndex+len){
    case 4:
      switch(leftIndex){
      case 0:
	*addrl++ = DoRRop (*addrl, piQxelAnd[0], piQxelXor[0]);
	*addrl++ = DoRRop (*addrl, piQxelAnd[1], piQxelXor[1]);
	*addrl   = DoRRop (*addrl, piQxelAnd[2], piQxelXor[2]);
	break;
      case 1:
	*addrl++ = DoMaskRRop (*addrl, piQxelAnd[0], piQxelXor[0], 0xFF000000);
	*addrl++ = DoRRop (*addrl, piQxelAnd[1], piQxelXor[1]);
	*addrl   = DoRRop (*addrl, piQxelAnd[2], piQxelXor[2]);
	break;
      case 2:
	*addrl++ = DoMaskRRop (*addrl, piQxelAnd[1], piQxelXor[1], 0xFFFF0000);
	*addrl   = DoRRop (*addrl, piQxelAnd[2], piQxelXor[2]);
	break;
      case 3:
	*addrl = DoMaskRRop (*addrl, piQxelAnd[2], piQxelXor[2], 0xFFFFFF00);
	break;
      }
      break;
    case 3:
      switch(leftIndex){
      case 0:
	*addrl++ = DoRRop (*addrl, piQxelAnd[0], piQxelXor[0]);
	*addrl++ = DoRRop (*addrl, piQxelAnd[1], piQxelXor[1]);
	*addrl = DoMaskRRop (*addrl, piQxelAnd[2], piQxelXor[2], 0xFF);
	break;
      case 1:
	*addrl++ = DoMaskRRop (*addrl, piQxelAnd[0], piQxelXor[0], 0xFF000000);
	*addrl++ = DoRRop (*addrl, piQxelAnd[1], piQxelXor[1]);
	*addrl = DoMaskRRop (*addrl, piQxelAnd[2], piQxelXor[2], 0xFF);
	break;
      case 2:
	*addrl++ = DoMaskRRop (*addrl, piQxelAnd[1], piQxelXor[1], 0xFFFF0000);
	*addrl = DoMaskRRop (*addrl, piQxelAnd[2], piQxelXor[2], 0xFF);
	break;
      }
      break;
    case 2:
      if(leftIndex){
	*addrl++ = DoMaskRRop (*addrl, piQxelAnd[0], piQxelXor[0], 0xFF000000);
      }
      else{
	*addrl++ = DoRRop (*addrl, piQxelAnd[0], piQxelXor[0]);
      }
      *addrl =  DoMaskRRop (*addrl, piQxelAnd[1], piQxelXor[1], 0xFFFF);
      break;
    case 1: /*only if leftIndex = 0 and w = 1*/
      *addrl = DoMaskRRop (*addrl, piQxelAnd[0], piQxelXor[0], 0xFFFFFF);
      break;
    case 0: /*never*/
      break;
    default:
      {
	if (rop == GXcopy){
	  switch(leftIndex){
	  case 0:
	    break;
	  case 1:
	    *addrl++ = ((*addrl) & 0xFFFFFF) | (piQxelXor[0] & 0xFF000000);
	    *addrl++ = piQxelXor[1];
	    *addrl++ = piQxelXor[2];
	    break;
	  case 2:
	    *addrl++ = ((*addrl) & 0xFFFF) | (piQxelXor[1] & 0xFFFF0000);
	    *addrl++ = piQxelXor[2];
	    break;
	  case 3:
	    *addrl++ = ((*addrl) & 0xFF) | (piQxelXor[2] & 0xFFFFFF00);
	    break;
	  }
	  while(nlmiddle--){
	    *addrl++ = piQxelXor[0];
	    *addrl++ = piQxelXor[1];
	    *addrl++ = piQxelXor[2];
	  }
	  switch(rightIndex){
	  case 0:
	    break;
	  case 1:
	    *addrl = ((*addrl) & 0xFF000000) | (piQxelXor[0] & 0xFFFFFF);
	    break;
	  case 2:
	    *addrl++ = piQxelXor[0];
	    *addrl = ((*addrl) & 0xFFFF0000) | (piQxelXor[1] & 0xFFFF);
	    break;
	  case 3:
	    *addrl++ = piQxelXor[0];
	    *addrl++ = piQxelXor[1];
	    *addrl = ((*addrl) & 0xFFFFFF00) | (piQxelXor[2] & 0xFF);
	    break;
	  }
	}
	else{
	  if(rop == GXxor){
	  switch(leftIndex){
	  case 0:
	    break;
	  case 1:
	    *addrl++ ^= (piQxelXor[0]&0xFF000000);
	    *addrl++ ^= piQxelXor[1];
	    *addrl++ ^= piQxelXor[2];
	    break;
	  case 2:
	    *addrl++ ^= (piQxelXor[1]& 0xFFFF0000);
	    *addrl++ ^= piQxelXor[2];
	    break;
	  case 3:
	    *addrl++ ^= (piQxelXor[2]& 0xFFFFFF00);
	    break;
	  }
	  while(nlmiddle--){
	    *addrl++ ^= piQxelXor[0];
	    *addrl++ ^= piQxelXor[1];
	    *addrl++ ^= piQxelXor[2];
	  }
	  switch(rightIndex){
	  case 0:
	    break;
	  case 1:
	    *addrl ^= (piQxelXor[0]& 0xFFFFFF);
	    break;
	  case 2:
	    *addrl++ ^= piQxelXor[0];
	    *addrl ^= (piQxelXor[1]&0xFFFF);
	    break;
	  case 3:
	    *addrl++ ^= piQxelXor[0];
	    *addrl++ ^= piQxelXor[1];
	    *addrl ^= (piQxelXor[2]&0xFF);
	    break;
	  }
	}
	  else{
	    switch(leftIndex){
	    case 0:
	      break;
	    case 1:
	      *addrl++ = DoMaskRRop (*addrl, piQxelAnd[0], piQxelXor[0], 0xFF000000);
	      *addrl++ = DoRRop (*addrl, piQxelAnd[1], piQxelXor[1]);
	      *addrl++ = DoRRop (*addrl, piQxelAnd[2], piQxelXor[2]);
	      break;
	    case 2:
	      *addrl++ = DoMaskRRop (*addrl, piQxelAnd[1], piQxelXor[1], 0xFFFF0000);
	      *addrl++ = DoRRop (*addrl, piQxelAnd[2], piQxelXor[2]);
	      break;
	    case 3:
	      *addrl++ = DoMaskRRop (*addrl, piQxelAnd[2], piQxelXor[2], 0xFFFFFF00);
	      break;
	  }
	  while(nlmiddle--){
	    *addrl++ = DoRRop (*addrl, piQxelAnd[0], piQxelXor[0]);
	    *addrl++ = DoRRop (*addrl, piQxelAnd[1], piQxelXor[1]);
	    *addrl++ = DoRRop (*addrl, piQxelAnd[2], piQxelXor[2]);
	  }
	  switch(rightIndex){
	  case 0:
	    break;
	  case 1:
	    *addrl++ = DoMaskRRop (*addrl, piQxelAnd[0], piQxelXor[0], 0xFFFFFF);
	    break;
	  case 2:
	    *addrl++ = DoRRop (*addrl, piQxelAnd[0], piQxelXor[0]);
	    *addrl = DoMaskRRop (*addrl, piQxelAnd[1], piQxelXor[1], 0xFFFF);
	    break;
	  case 3:
	    *addrl++ = DoRRop (*addrl, piQxelAnd[0], piQxelXor[0]);
	    *addrl++ = DoRRop (*addrl, piQxelAnd[1], piQxelXor[1]);
	    *addrl = DoMaskRRop (*addrl, piQxelAnd[2], piQxelXor[2], 0xFF);
	    break;
	  }

	  }
	}
      }
    }
#else
    addrl = addrl + (y1 * nlwidth) + (x1 >> PWSH);

    /* all bits inside same longword */
    if ( ((x1 & PIM) + len) < PPW)
    {
	maskpartialbits(x1, len, startmask);
	*addrl = DoMaskRRop (*addrl, and, xor, startmask);
    }
    else
    {
	maskbits(x1, len, startmask, endmask, nlmiddle);
	if (rop == GXcopy)
	{
	    if (startmask)
	    {
		*addrl = (*addrl & ~startmask) | (xor & startmask);
		addrl++;
	    }
	    while (nlmiddle--)
	    	*addrl++ = xor;
	    if (endmask)
		*addrl = (*addrl & ~endmask) | (xor & endmask);
	}
	else
	{
	    if (startmask)
	    {
		*addrl = DoMaskRRop (*addrl, and, xor, startmask);
		addrl++;
	    }
	    if (rop == GXxor)
	    {
		while (nlmiddle--)
		    *addrl++ ^= xor;
	    }
	    else
	    {
		while (nlmiddle--)
		{
		    *addrl = DoRRop (*addrl, and, xor);
		    addrl++;
		}
	    }
	    if (endmask)
		*addrl = DoMaskRRop (*addrl, and, xor, endmask);
	}
    }
#endif
}

/* vertical solid line */

void
cfbVertS(rop, and, xor, addrl, nlwidth, x1, y1, len)
int rop;
register unsigned long and, xor;
register unsigned long *addrl;	/* pointer to base of bitmap */
register int nlwidth;	/* width in longwords of bitmap */
int x1, y1;		/* initial point */
register int len;	/* length of line */
{
#if PSZ == 24
    int xIdx;
    unsigned long and2, xor2, offset, mask, mask2;
#endif
#ifdef PIXEL_ADDR
    register PixelType    *bits = (PixelType *) addrl;

#if PSZ == 24
    nlwidth <<= PWSH;
    xIdx = x1 & 3;
    bits = (PixelType *)(addrl + (y1 * nlwidth) + ((x1*3) >> 2));
#else
    nlwidth <<= PWSH;
    bits = bits + (y1 * nlwidth) + x1;
#endif
#if PSZ == 24
    mask2 = 0;
    switch(xIdx){
      case 0:
        mask = 0xFF000000;
	xor &= 0xFFFFFF;
	and |= 0xFF000000;
	break;
      case 3:
	mask = 0xFF;
	xor &= 0xFFFFFF;
	xor <<= 8;
	and <<= 8;
	and |= 0xFF;
	break;
      case 1:
	mask = 0xFFFFFF;
	mask2 = 0xFFFF0000;
	xor2 = (xor>>8) & 0xFFFF;
	xor &= 0xFF;
	xor <<= 24;
	and2 = (and >> 8 ) | 0xFFFF0000;
	and <<= 24;
	and |= 0xFFFFFF;
	break;
      case 2:
	mask = 0x0000FFFF;
	mask2 = 0xFFFFFF00;
	xor2 = (xor >> 16) & 0xFF;
	xor <<= 16;
	xor &= 0xFFFF0000;
	and2 = (and >> 16) | 0xFFFFFF00;
	and <<= 16;
	and |= 0xFFFF;
	break;
      }
#endif

    /*
     * special case copy and xor to avoid a test per pixel
     */
    if (rop == GXcopy)
    {
#if PSZ == 24
      switch(xIdx){
      case 0:
      case 3:
	while (len--){
	  *bits = (*bits & mask)| xor;
	  bits += nlwidth;
	}
	break;
      case 1:
      case 2:
	while (len--){
	  *bits = (*bits & mask)| xor;
	  bits++;
	  *bits = (*bits & mask2)| xor2;
	  bits--;
	  bits += nlwidth;
	}
	break;
      }
#else
	while (len--)
	{
	    *bits = xor;
	    bits += nlwidth;
	}
#endif
    }
    else if (rop == GXxor)
    {
#if PSZ == 24
      switch(xIdx){
      case 0:
      case 3:
	while (len--){
	  *bits ^=  xor;
	  bits += nlwidth;
	}
	break;
      case 1:
      case 2:
	while (len--){
	  *bits ^= xor;
	  bits++;
	  *bits ^= xor2;
	  bits--;
	  bits += nlwidth;
	}
	break;
      }
#else
	while (len--)
	{
	    *bits ^= xor;
	    bits += nlwidth;
	}
#endif
    }
    else
    {
#if PSZ == 24
      switch(xIdx){
      case 0:
	while (len--){
	  *bits = DoMaskRRop(*bits, and, xor, 0x00FFFFFF);
	  bits += nlwidth;
	}
	break;
      case 3:
	while (len--){
	  *bits = DoMaskRRop(*bits, and, xor, 0xFFFFFF00);
	  bits += nlwidth;
	}
	break;
      case 1:
	while (len--){
	  *bits = DoMaskRRop(*bits, and, xor, 0xFF000000);
	  bits++;
	  *bits = DoMaskRRop(*bits, and2, xor2, 0x0000FFFF);
	  bits--;
	  bits += nlwidth;
	}
	break;
      case 2:
	while (len--){
	  *bits = DoMaskRRop(*bits, and, xor, 0xFFFF0000);
	  bits++;
	  *bits = DoMaskRRop(*bits, and2, xor2, 0x000000FF);
	  bits--;
	  bits += nlwidth;
	}
	break;
      }
#else
	while (len--)
	{
	    *bits = DoRRop(*bits, and, xor);
	    bits += nlwidth;
	}
#endif
    }
#else /* !PIXEL_ADDR */
#if PSZ == 24
    addrl = addrl + (y1 * nlwidth) + ((x1*3) >>2);

    and |= ~cfbmask[(x1 & 3)<<1];
    xor &= cfbmask[(x1 & 3)<<1];
#else
    addrl = addrl + (y1 * nlwidth) + (x1 >> PWSH);

    and |= ~cfbmask[x1 & PIM];
    xor &= cfbmask[x1 & PIM];
#endif

    while (len--)
    {
	*addrl = DoRRop (*addrl, and, xor);
	addrl += nlwidth;
    }
#endif
}
