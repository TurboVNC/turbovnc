/*
 * $Xorg: cfbsolid.c,v 1.4 2001/02/09 02:04:38 xorgcvs Exp $
 *
Copyright 1990, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 *
 * Author:  Keith Packard, MIT X Consortium
 */
/* $XFree86: xc/programs/Xserver/cfb/cfbsolid.c,v 3.8tsi Exp $ */


#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xmd.h>
#include "servermd.h"
#include "gcstruct.h"
#include "window.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "windowstr.h"

#include "cfb.h"
#include "cfbmskbits.h"
#include "cfbrrop.h"

#include "mi.h"
#include "mispans.h"

#if defined(FAST_CONSTANT_OFFSET_MODE) && (RROP != GXcopy)
# define Expand(left,right,leftAdjust) {\
    int part = nmiddle & 3; \
    int widthStep; \
    widthStep = widthDst - nmiddle - leftAdjust; \
    nmiddle >>= 2; \
    pdst = pdstRect; \
    while (h--) { \
	left \
	pdst += part; \
	switch (part) { \
	    RROP_UNROLL_CASE3(pdst) \
	} \
	m = nmiddle; \
	while (m) { \
	    pdst += 4; \
	    RROP_UNROLL_LOOP4(pdst,-4) \
	    m--; \
	} \
	right \
	pdst += widthStep; \
    } \
}
#else
# ifdef RROP_UNROLL
#  define Expand(left,right,leftAdjust) {\
    int part = nmiddle & RROP_UNROLL_MASK; \
    int widthStep; \
    widthStep = widthDst - nmiddle - leftAdjust; \
    nmiddle >>= RROP_UNROLL_SHIFT; \
    pdst = pdstRect; \
    while (h--) { \
	left \
	pdst += part; \
	switch (part) { \
	    RROP_UNROLL_CASE(pdst) \
	} \
	m = nmiddle; \
	while (m) { \
	    pdst += RROP_UNROLL; \
	    RROP_UNROLL_LOOP(pdst) \
	    m--; \
	} \
	right \
	pdst += widthStep; \
    } \
}

# else
#  define Expand(left, right, leftAdjust) { \
    while (h--) { \
	pdst = pdstRect; \
	left \
	m = nmiddle; \
	while (m--) {\
	    RROP_SOLID(pdst); \
	    pdst++; \
	} \
	right \
	pdstRect += widthDst; \
    } \
}
# endif
#endif
	

void
RROP_NAME(cfbFillRectSolid) (pDrawable, pGC, nBox, pBox)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    int		    nBox;
    BoxPtr	    pBox;
{
    register int    m;
    register CfbBits   *pdst;
    RROP_DECLARE
    CfbBits   *pdstBase, *pdstRect;
    int		    nmiddle;
    int		    h;
    int		    w;
    int		    widthDst;
#if PSZ == 24
    int		    leftIndex, rightIndex;
#else
    register CfbBits   leftMask, rightMask;
#endif

    cfbGetLongWidthAndPointer (pDrawable, widthDst, pdstBase)

    RROP_FETCH_GC(pGC)
    
    for (; nBox; nBox--, pBox++)
    {
    	pdstRect = pdstBase + pBox->y1 * widthDst;
    	h = pBox->y2 - pBox->y1;
	w = pBox->x2 - pBox->x1;
#if PSZ == 8
	if (w == 1)
	{
	    register char    *pdstb = ((char *) pdstRect) + pBox->x1;
	    int	    incr = widthDst * PGSZB;

	    while (h--)
	    {
		RROP_SOLID (pdstb);
		pdstb += incr;
	    }
	}
	else
	{
#endif
#if PSZ == 24
	leftIndex = pBox->x1 &3;
/*	rightIndex = ((leftIndex+w)<5)?0:pBox->x2 &3;*/
	rightIndex = pBox->x2 &3;

	nmiddle = w - rightIndex;
	if(leftIndex){
	  nmiddle -= (4 - leftIndex);
	}
	nmiddle >>= 2;
	if(nmiddle < 0)
	  nmiddle = 0;

	pdstRect += (pBox->x1 * 3) >> 2;
	pdst = pdstRect;	
	switch(leftIndex+w){
	case 4:
	    switch(leftIndex){
	    case 0:
		while(h--){
#if RROP == GXcopy
		    *pdst++ = piQxelXor[0];
		    *pdst++ = piQxelXor[1];
		    *pdst-- = piQxelXor[2];
#endif
#if RROP == GXxor
		    *pdst++ ^= piQxelXor[0];
		    *pdst++ ^= piQxelXor[1];
		    *pdst-- ^= piQxelXor[2];
#endif
#if RROP == GXand
		    *pdst++ &= piQxelAnd[0];
		    *pdst++ &= piQxelAnd[1];
		    *pdst-- &= piQxelAnd[2];
#endif
#if RROP == GXor
		    *pdst++ |= piQxelOr[0];
		    *pdst++ |= piQxelOr[1];
		    *pdst-- |= piQxelOr[2];
#endif
#if RROP == GXset
		    *pdst = DoRRop((*pdst), piQxelAnd[0], piQxelXor[0]);
		    pdst++;
		    *pdst = DoRRop((*pdst), piQxelAnd[1], piQxelXor[1]);
		    pdst++;
		    *pdst = DoRRop((*pdst), piQxelAnd[2], piQxelXor[2]);
		    pdst--;
#endif
		    pdst--;
		    pdst += widthDst;
		}
		break;
	    case 1:
		while(h--){
#if RROP == GXcopy
		    *pdst = ((*pdst) & 0xFFFFFF) | (piQxelXor[0] & 0xFF000000);
		    pdst++;
		    *pdst++ = piQxelXor[1];
		    *pdst-- = piQxelXor[2];
#endif
#if RROP == GXxor
		    *pdst++ ^= (piQxelXor[0] & 0xFF000000);
		    *pdst++ ^= piQxelXor[1];
		    *pdst-- ^= piQxelXor[2];
#endif
#if RROP == GXand
		    *pdst++ &= (piQxelAnd[0] | 0x00FFFFFF);
		    *pdst++ &= piQxelAnd[1];
		    *pdst-- &= piQxelAnd[2];
#endif
#if RROP == GXor
		    *pdst++ |= (piQxelOr[0] & 0xFF000000);
		    *pdst++ |= piQxelOr[1];
		    *pdst-- |= piQxelOr[2];
#endif
#if RROP == GXset
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[0], piQxelXor[0], 0xFF000000);
		    pdst++;
		    *pdst = DoRRop((*pdst), piQxelAnd[1], piQxelXor[1]);
		    pdst++;
		    *pdst = DoRRop((*pdst), piQxelAnd[2], piQxelXor[2]);
		    pdst--;
#endif
		    pdst--;
		    pdst += widthDst;
		}
		break;
	    case 2:
		while(h--){
#if RROP == GXcopy
		    *pdst = ((*pdst) & 0xFFFF) | (piQxelXor[1] & 0xFFFF0000);
		    pdst++;
		    *pdst-- = piQxelXor[2];
#endif
#if RROP == GXxor
		    *pdst++ ^= (piQxelXor[1] & 0xFFFF0000);
		    *pdst-- ^= piQxelXor[2];
#endif
#if RROP == GXand
		    *pdst++ &= (piQxelAnd[1] | 0xFFFF);
		    *pdst-- &= piQxelAnd[2];
#endif
#if RROP == GXor
		    *pdst++ |= (piQxelOr[1] & 0xFFFF0000);
		    *pdst-- |= piQxelOr[2];
#endif
#if RROP == GXset
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[1], piQxelXor[1], 0xFFFF0000);
		    pdst++;
		    *pdst = DoRRop((*pdst), piQxelAnd[2], piQxelXor[2]);
		    pdst--;
#endif
		    pdst += widthDst;
		}
		break;
	    case 3:
		while(h--){
#if RROP == GXcopy
		    *pdst = ((*pdst) & 0xFF) | (piQxelXor[2] & 0xFFFFFF00);
#endif
#if RROP == GXxor
		    *pdst ^= (piQxelXor[2] & 0xFFFFFF00);
#endif
#if RROP == GXand
		    *pdst &= (piQxelAnd[2] | 0xFF);
#endif
#if RROP == GXor
		    *pdst |= (piQxelOr[2] & 0xFFFFFF00);
#endif
#if RROP == GXset
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[2], piQxelXor[2], 0xFFFFFF00);
#endif
		    pdst += widthDst;
		}
		break;
	    }
	    break;
	case 3:
	    switch(leftIndex){
	    case 0:
		while(h--){
#if RROP == GXcopy
		    *pdst++ = piQxelXor[0];
		    *pdst++ = piQxelXor[1];
		    *pdst = ((*pdst) & 0xFFFFFF00) | (piQxelXor[2] & 0xFF);
		    pdst--;
#endif
#if RROP == GXxor
		    *pdst++ ^= piQxelXor[0];
		    *pdst++ ^= piQxelXor[1];
		    *pdst-- ^= (piQxelXor[2] & 0xFF);
#endif
#if RROP == GXand
		    *pdst++ &= piQxelAnd[0];
		    *pdst++ &= piQxelAnd[1];
		    *pdst-- &= (piQxeAnd[2] | 0xFFFFFF00);
#endif
#if RROP == GXor
		    *pdst++ |= piQxelOr[0];
		    *pdst++ |= piQxelOr[1];
		    *pdst-- |= (piQxelOr[2] & 0xFF);
#endif
#if RROP == GXset
		    *pdst = DoRRop((*pdst), piQxelAnd[0], piQxelXor[0]);
		    pdst++;
		    *pdst = DoRRop((*pdst), piQxelAnd[1], piQxelXor[1]);
		    pdst++;
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[2], piQxelXor[2], 0xFF);
		    pdst--;
#endif
		    pdst--;
		    pdst += widthDst;
		}
		break;
	    case 1:
		while(h--){
#if RROP == GXcopy
		    *pdst = ((*pdst) & 0xFFFFFF) | (piQxelXor[0] & 0xFF000000);
		    pdst++;
		    *pdst++ = piQxelXor[1];
		    *pdst = ((*pdst) & 0xFFFFFF00) | (piQxelXor[2] & 0xFF);
		    pdst--;
#endif
#if RROP == GXxor
		    *pdst++ ^= (piQxelXor[0] & 0xFF000000);
		    *pdst++ ^= piQxelXor[1];
		    *pdst-- ^= (piQxelXor[2] & 0xFF);
#endif
#if RROP == GXand
		    *pdst++ &= (piQxelAnd[0] | 0x00FFFFFF);
		    *pdst++ &= piQxelAnd[1];
		    *pdst-- &= (piQxelAnd[2] | 0xFFFFFF00);
#endif
#if RROP == GXor
		    *pdst++ |= (piQxelOr[0] & 0xFF000000);
		    *pdst++ |= piQxelOr[1];
		    *pdst-- |= (piQxelOr[2] & 0xFF);
#endif
#if RROP == GXset
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[0], piQxelXor[0], 0xFF000000);
		    pdst++;
		    *pdst = DoRRop((*pdst), piQxelAnd[1], piQxelXor[1]);
		    pdst++;
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[2], piQxelXor[2], 0xFF);
		    pdst--;
#endif
		    pdst--;
		    pdst += widthDst;
		}
		break;
	    case 2:
		while(h--){
#if RROP == GXcopy
		    *pdst = ((*pdst) & 0xFFFF) | (piQxelXor[1] & 0xFFFF0000);
		    pdst++;
		    *pdst = ((*pdst) & 0xFFFFFF00) | (piQxelXor[2] & 0xFF);
		    pdst--;
#endif
#if RROP == GXxor
		    *pdst++ ^= (piQxelXor[1] & 0xFFFF0000);
		    *pdst-- ^= (piQxelXor[2] & 0xFF);
#endif
#if RROP == GXand
		    *pdst++ &= (piQxelAnd[1] | 0xFFFF);
		    *pdst-- &= (piQxelAnd[2] | 0xFFFFFF00);
#endif
#if RROP == GXor
		    *pdst++ |= (piQxelOr[1] & 0xFFFF0000);
		    *pdst-- |= (piQxelOr[2] & 0xFF);
#endif
#if RROP == GXset
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[1], piQxelXor[1], 0xFFFF0000);
		    pdst++;
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[2], piQxelXor[2], 0xFF);
		    pdst--;
#endif
		    pdst += widthDst;
		}
		break;
	    case 3:
		while(h--){
#if RROP == GXcopy
		    *pdst = ((*pdst) & 0xFF) | (piQxelXor[2] & 0xFFFFFF00);
#endif
#if RROP == GXxor
		    *pdst ^= (piQxelXor[2] & 0xFFFFFF00);
#endif
#if RROP == GXand
		    *pdst &= (piQxelAnd[2] | 0xFF);
#endif
#if RROP == GXor
		    *pdst |= (piQxelOr[2] & 0xFFFFFF00);
#endif
#if RROP == GXset
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[2], piQxelXor[2], 0xFFFFFF00);
#endif
		    pdst += widthDst;
		}
		break;
	    }
	    break;
	case 2: /* leftIndex + w = 2*/
	    switch(leftIndex){
	    case 2:
		while(h--){
#if RROP == GXcopy
		    *pdst = ((*pdst) & 0xFFFF) | (piQxelXor[1] & 0xFFFF0000);
		    pdst++;
		    *pdst = ((*pdst) & 0xFFFFFF00) | (piQxelXor[2] & 0xFF);
		    pdst--;
#endif
#if RROP == GXxor
		    *pdst++ ^= (piQxelXor[1] & 0xFFFF0000);
		    *pdst-- ^= (piQxelXor[2] & 0xFF);
#endif
#if RROP == GXand
		    *pdst++ &= (piQxelAnd[1] | 0xFFFF0000);
		    *pdst-- &= (piQxelAnd[2] | 0xFF);
#endif
#if RROP == GXor
		    *pdst++ |= (piQxelOr[1] & 0xFFFF0000);
		    *pdst-- |= (piQxelOr[2] & 0xFF);
#endif
#if RROP == GXset
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[1], piQxelXor[1], 0xFFFF0000);
		    pdst++;
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[2], piQxelXor[2], 0xFF);
		    pdst--;
#endif
		    pdst += widthDst;
		  }
		break;
	    case 1:
		while(h--){
#if RROP == GXcopy
		    *pdst = ((*pdst) & 0xFFFFFF) | (piQxelXor[0] & 0xFF000000);
		    pdst++;
		    *pdst = ((*pdst) & 0xFFFF0000) | (piQxelXor[1] & 0xFFFF);
		    pdst--;
#endif
#if RROP == GXxor
		    *pdst++ ^= (piQxelXor[0] & 0xFF000000);
		    *pdst-- ^= (piQxelXor[1] & 0xFFFF);
#endif
#if RROP == GXand
		    *pdst++ &= (piQxelAnd[0] | 0xFFFFFF);
		    *pdst-- &= (piQxelAnd[1] | 0xFFFF0000);
#endif
#if RROP == GXor
		    *pdst++ |= (piQxelOr[0] & 0xFF000000);
		    *pdst-- |= (piQxelOr[1] & 0xFFFF);
#endif
#if RROP == GXset
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[0], piQxelXor[0], 0xFF000000);
		    pdst++;
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[1], piQxelXor[1], 0xFFFF);
		    pdst--;
#endif
		    pdst += widthDst;
		  }
		break;
	    case 0: /*case 2 leftIndex == 0 */
		while(h--){
#if RROP == GXcopy
		    *pdst++ = piQxelXor[0];
		    *pdst = ((*pdst) & 0xFFFF0000) | (piQxelXor[1] & 0xFFFF);
		    pdst--;
#endif
#if RROP == GXxor
		    *pdst++ ^= piQxelXor[0];
		    *pdst-- ^= (piQxelXor[1] & 0xFFFF);
#endif
#if RROP == GXand
		    *pdst++ &= piQxelAnd[0];
		    *pdst-- &= (piQxelAnd[1] | 0xFFFF0000);
#endif
#if RROP == GXor
		    *pdst++ |= piQxelOr[0];
		    *pdst-- |= (piQxelOr[1] & 0xFFFF);
#endif
#if RROP == GXset
		    *pdst = DoRRop((*pdst), piQxelAnd[0], piQxelXor[0]);
		    pdst++;
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[1], piQxelXor[1], 0xFFFF);
		    pdst--;
#endif
		    pdst += widthDst;
		}
		break;
	    }
	    break;
	case 1: /*only if leftIndex = 0 and w = 1*/
	    while(h--){
#if RROP == GXcopy
		*pdst = ((*pdst) & 0xFF000000) | (piQxelXor[0] & 0xFFFFFF);
#endif
#if RROP == GXxor
		*pdst ^= (piQxelXor[0] & 0xFFFFFF);
#endif
#if RROP == GXand
		*pdst &= (piQxelAnd[0] | 0xFF000000);
#endif
#if RROP == GXor
		*pdst |= (piQxelOr[0] & 0xFFFFFF);
#endif
#if RROP == GXset
		*pdst = DoMaskRRop((*pdst), piQxelAnd[0], piQxelXor[0], 0xFFFFFF);
#endif
		pdst += widthDst;
	    }
	    break;
	case 0: /*never*/
	    break;
	default:
	    {
		while(h--){
		    pdst = pdstRect;
		    switch(leftIndex){
		    case 0:
			break;
		    case 1:
#if RROP == GXcopy
			*pdst = ((*pdst) & 0xFFFFFF) | (piQxelXor[0] & 0xFF000000);
			pdst++;
			*pdst++ = piQxelXor[1];
			*pdst++ = piQxelXor[2];
#endif
#if RROP == GXxor
			*pdst++ ^= (piQxelXor[0] & 0xFF000000);
			*pdst++ ^= piQxelXor[1];
			*pdst++ ^= piQxelXor[2];
#endif
#if RROP == GXand
			*pdst++ &= (piQxelAnd[0] | 0xFFFFFF);
			*pdst++ &= piQxelAnd[1];
			*pdst++ &= piQxelAnd[2];
#endif
#if RROP == GXor
			*pdst++ |= (piQxelOr[0] & 0xFF000000);
			*pdst++ |= piQxelOr[1];
			*pdst++ |= piQxelOr[2];
#endif
#if RROP == GXset
			*pdst = DoMaskRRop((*pdst), piQxelAnd[0], piQxelXor[0], 0xFF000000);
			pdst++;
			*pdst = DoRRop((*pdst), piQxelAnd[1], piQxelXor[1]);
			pdst++;
			*pdst = DoRRop((*pdst), piQxelAnd[2], piQxelXor[2]);
			pdst++;
#endif
		    break;
		    case 2:
#if RROP == GXcopy
			*pdst = (((*pdst) & 0xFFFF) | (piQxelXor[1] & 0xFFFF0000));
			pdst++;
			*pdst++ = piQxelXor[2];
#endif
#if RROP == GXxor
			*pdst++ ^=(piQxelXor[1] & 0xFFFF0000);
			*pdst++ ^= piQxelXor[2];
#endif
#if RROP == GXand
			*pdst++ &= (piQxelAnd[1] | 0xFFFF);
			*pdst++ &= piQxelAnd[2];
#endif
#if RROP == GXor
			*pdst++ |= (piQxelOr[1] & 0xFFFF0000);
			*pdst++ |= piQxelOr[2];
#endif
#if RROP == GXset
			*pdst = DoMaskRRop((*pdst), piQxelAnd[1], piQxelXor[1], 0xFFFF0000);
			pdst++;
			*pdst = DoRRop((*pdst), piQxelAnd[2], piQxelXor[2]);
			pdst++;
#endif
			break;
		    case 3:
#if RROP == GXcopy
			*pdst = ((*pdst) & 0xFF) | (piQxelXor[2] & 0xFFFFFF00);
			pdst++;
#endif
#if RROP == GXxor
			*pdst++ ^= (piQxelXor[2] & 0xFFFFFF00);
#endif
#if RROP == GXand
			*pdst++ &= (piQxelAnd[2] | 0xFF);
#endif
#if RROP == GXor
			*pdst++ |= (piQxelOr[2] & 0xFFFFFF00);
#endif
#if RROP == GXset
			*pdst = DoMaskRRop((*pdst), piQxelAnd[2], piQxelXor[2], 0xFFFFFF00);
			pdst++;
#endif
			break;
		      }
		    m = nmiddle;
		    while(m--){
#if RROP == GXcopy
			*pdst++ = piQxelXor[0];
			*pdst++ = piQxelXor[1];
			*pdst++ = piQxelXor[2];
#endif
#if RROP == GXxor
			*pdst++ ^= piQxelXor[0];
			*pdst++ ^= piQxelXor[1];
			*pdst++ ^= piQxelXor[2];
#endif
#if RROP == GXand
			*pdst++ &= piQxelAnd[0];
			*pdst++ &= piQxelAnd[1];
			*pdst++ &= piQxelAnd[2];
#endif
#if RROP == GXor
			*pdst++ |= piQxelOr[0];
			*pdst++ |= piQxelOr[1];
			*pdst++ |= piQxelOr[2];
#endif
#if RROP == GXset
			*pdst = DoRRop((*pdst), piQxelAnd[0], piQxelXor[0]);
			pdst++;
			*pdst = DoRRop((*pdst), piQxelAnd[1], piQxelXor[1]);
			pdst++;
			*pdst = DoRRop((*pdst), piQxelAnd[2], piQxelXor[2]);
			pdst++;
#endif
		}
		switch(rightIndex){
		case 0:
		  break;
		case 1:
#if RROP == GXcopy
		  *pdst = ((*pdst) & 0xFF000000) | (piQxelXor[0] & 0xFFFFFF);
		  pdst++;
#endif
#if RROP == GXxor
		  *pdst++ ^= (piQxelXor[0] & 0xFFFFFF);
#endif
#if RROP == GXand
		  *pdst++ &= (piQxelAnd[0] | 0xFF);
#endif
#if RROP == GXor
		  *pdst++ |= (piQxelOr[0] & 0xFFFFFF);
#endif
#if RROP == GXset
		  *pdst = DoMaskRRop((*pdst), piQxelAnd[0], piQxelXor[0], 0xFFFFFF);
		  pdst++;
#endif
		  break;
		case 2:
#if RROP == GXcopy
		  *pdst++ = piQxelXor[0];
		  *pdst = ((*pdst) & 0xFFFF0000) | (piQxelXor[1] & 0xFFFF);
		  pdst++;
#endif
#if RROP == GXxor
		  *pdst++ ^= piQxelXor[0];
		  *pdst++ ^= (piQxelXor[1] & 0xFFFF);
#endif
#if RROP == GXand
		  *pdst++ &= piQxelAnd[0];
		  *pdst++ &= (piQxelAnd[1] | 0xFFFF0000);
#endif
#if RROP == GXor
		  *pdst++ |= piQxelOr[0];
		  *pdst++ |= (piQxelOr[1] & 0xFFFF);
#endif
#if RROP == GXset
		  *pdst = DoRRop((*pdst), piQxelAnd[0], piQxelXor[0]);
		  pdst++;
		  *pdst = DoMaskRRop((*pdst), piQxelAnd[1], piQxelXor[1], 0xFFFF);
		  pdst++;
#endif
		  break;
		case 3:
#if RROP == GXcopy
		  *pdst++ = piQxelXor[0];
		  *pdst++ = piQxelXor[1];
		  *pdst = ((*pdst) & 0xFFFFFF00) | (piQxelXor[2] & 0xFF);
		  pdst++;
#endif
#if RROP == GXxor
		  *pdst++ ^= piQxelXor[0];
		  *pdst++ ^= piQxelXor[1];
		  *pdst++ ^= (piQxelXor[2] & 0xFF);
#endif
#if RROP == GXand
		  *pdst++ &= piQxelAnd[0];
		  *pdst++ &= piQxelAnd[1];
		  *pdst++ &= (piQxelAnd[2] | 0xFFFFFF00);
#endif
#if RROP == GXor
		  *pdst++ |= piQxelOr[0];
		  *pdst++ |= piQxelOr[1];
		  *pdst++ |= (piQxelOr[2] & 0xFF);
#endif
#if RROP == GXset
		  *pdst = DoRRop((*pdst), piQxelAnd[0], piQxelXor[0]);
		  pdst++;
		  *pdst = DoRRop((*pdst), piQxelAnd[1], piQxelXor[1]);
		  pdst++;
		  *pdst = DoMaskRRop((*pdst), piQxelAnd[2], piQxelXor[2], 0xFF);
		  pdst++;
#endif
		  break;
		}
		pdstRect += widthDst;
	    }
	}
	}
#else /* PSZ != 24 */
	pdstRect += (pBox->x1 >> PWSH);
	if ((pBox->x1 & PIM) + w <= PPW)
	{
	    maskpartialbits(pBox->x1, w, leftMask);
	    pdst = pdstRect;
	    while (h--) {
		RROP_SOLID_MASK (pdst, leftMask);
		pdst += widthDst;
	    }
	}
	else
	{
	    maskbits (pBox->x1, w, leftMask, rightMask, nmiddle);
	    if (leftMask)
	    {
		if (rightMask)	/* left mask and right mask */
		{
		    Expand(RROP_SOLID_MASK (pdst, leftMask); pdst++;,
			   RROP_SOLID_MASK (pdst, rightMask);, 1)
		}
		else	/* left mask and no right mask */
		{
		    Expand(RROP_SOLID_MASK (pdst, leftMask); pdst++;,
			   ;, 1)
		}
	    }
	    else
	    {
		if (rightMask)	/* no left mask and right mask */
		{
		    Expand(;,
			   RROP_SOLID_MASK (pdst, rightMask);, 0)
		}
		else	/* no left mask and no right mask */
		{
		    Expand(;,
			    ;, 0)
		}
	    }
	}
#endif
#if PSZ == 8
	}
#endif
    }
    RROP_UNDECLARE
}

void
RROP_NAME(cfbSolidSpans) (pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
    CfbBits   *pdstBase;
    int		    widthDst;

    RROP_DECLARE
    
    register CfbBits  *pdst;
    register int	    nlmiddle;
    register int	    w;
    int			    x;
    
				/* next three parameters are post-clip */
    int		    n;		/* number of spans to fill */
    DDXPointPtr	    ppt;	/* pointer to list of start points */
    int		    *pwidthFree;/* copies of the pointers to free */
    DDXPointPtr	    pptFree;
    int		    *pwidth;
    cfbPrivGCPtr    devPriv;
#if PSZ == 24
    int		    leftIndex, rightIndex;
#else
    register CfbBits  startmask, endmask;
#endif

    devPriv = cfbGetGCPrivate(pGC);
    RROP_FETCH_GCPRIV(devPriv)
    n = nInit * miFindMaxBand(pGC->pCompositeClip);
    pwidthFree = (int *)ALLOCATE_LOCAL(n * sizeof(int));
    pptFree = (DDXPointRec *)ALLOCATE_LOCAL(n * sizeof(DDXPointRec));
    if(!pptFree || !pwidthFree)
    {
	if (pptFree) DEALLOCATE_LOCAL(pptFree);
	if (pwidthFree) DEALLOCATE_LOCAL(pwidthFree);
	return;
    }
    pwidth = pwidthFree;
    ppt = pptFree;
    n = miClipSpans(pGC->pCompositeClip, pptInit, pwidthInit, nInit,
		     ppt, pwidth, fSorted);

    cfbGetLongWidthAndPointer (pDrawable, widthDst, pdstBase)

    while (n--)
    {
	x = ppt->x;
	pdst = pdstBase + (ppt->y * widthDst);
	++ppt;
	w = *pwidth++;
	if (!w)
	    continue;
#if PSZ == 24
	leftIndex = x &3;
/*	rightIndex = ((leftIndex+w)<5)?0:(x+w)&3;*/
	rightIndex = (x+w)&3;

	nlmiddle = w - rightIndex;
	if(leftIndex){
	  nlmiddle -= (4 - leftIndex);
	}
/*	nlmiddle += 3;*/
	nlmiddle >>= 2;
	if(nlmiddle < 0)
	  nlmiddle = 0;

	pdst += (x >> 2)*3;
	pdst += leftIndex? (leftIndex -1):0;
	switch(leftIndex+w){
	case 4:
	    switch(leftIndex){
	    case 0:
#if RROP == GXcopy
		    *pdst++ = piQxelXor[0];
		    *pdst++ = piQxelXor[1];
		    *pdst-- = piQxelXor[2];
#endif
#if RROP == GXxor
		    *pdst++ ^= piQxelXor[0];
		    *pdst++ ^= piQxelXor[1];
		    *pdst-- ^= piQxelXor[2];
#endif
#if RROP == GXand
		    *pdst++ &= piQxelAnd[0];
		    *pdst++ &= piQxelAnd[1];
		    *pdst-- &= piQxelAnd[2];
#endif
#if RROP == GXor
		    *pdst++ |= piQxelOr[0];
		    *pdst++ |= piQxelOr[1];
		    *pdst-- |= piQxelOr[2];
#endif
#if RROP == GXset
		    *pdst = DoRRop((*pdst), piQxelAnd[0], piQxelXor[0]);
		    pdst++;
		    *pdst = DoRRop((*pdst), piQxelAnd[1], piQxelXor[1]);
		    pdst++;
		    *pdst = DoRRop((*pdst), piQxelAnd[2], piQxelXor[2]);
		    pdst--;
#endif
		    pdst--;
		break;
	    case 1:
#if RROP == GXcopy
		    *pdst = ((*pdst) & 0xFFFFFF) | (piQxelXor[0] & 0xFF000000);
		    pdst++;
		    *pdst++ = piQxelXor[1];
		    *pdst-- = piQxelXor[2];
#endif
#if RROP == GXxor
		    *pdst++ ^= (piQxelXor[0] & 0xFF000000);
		    *pdst++ ^= piQxelXor[1];
		    *pdst-- ^= piQxelXor[2];
#endif
#if RROP == GXand
		    *pdst++ &= (piQxelAnd[0] | 0x00FFFFFF);
		    *pdst++ &= piQxelAnd[1];
		    *pdst-- &= piQxelAnd[2];
#endif
#if RROP == GXor
		    *pdst++ |= (piQxelOr[0] & 0xFF000000);
		    *pdst++ |= piQxelOr[1];
		    *pdst-- |= piQxelOr[2];
#endif
#if RROP == GXset
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[0], piQxelXor[0], 0xFF000000);
		    pdst++;
		    *pdst = DoRRop((*pdst), piQxelAnd[1], piQxelXor[1]);
		    pdst++;
		    *pdst = DoRRop((*pdst), piQxelAnd[2], piQxelXor[2]);
		    pdst--;
#endif
		    pdst--;
		break;
	    case 2:
#if RROP == GXcopy
		    *pdst = ((*pdst) & 0xFFFF) | (piQxelXor[1] & 0xFFFF0000);
		    pdst++;
		    *pdst-- = piQxelXor[2];
#endif
#if RROP == GXxor
		    *pdst++ ^= (piQxelXor[1] & 0xFFFF0000);
		    *pdst-- ^= piQxelXor[2];
#endif
#if RROP == GXand
		    *pdst++ &= (piQxelAnd[1] | 0xFFFF);
		    *pdst-- &= piQxelAnd[2];
#endif
#if RROP == GXor
		    *pdst++ |= (piQxelOr[1] & 0xFFFF0000);
		    *pdst-- |= piQxelOr[2];
#endif
#if RROP == GXset
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[1], piQxelXor[1], 0xFFFF0000);
		    pdst++;
		    *pdst = DoRRop((*pdst), piQxelAnd[2], piQxelXor[2]);
		    pdst--;
#endif
		break;
	    case 3:
#if RROP == GXcopy
		    *pdst = ((*pdst) & 0xFF) | (piQxelXor[2] & 0xFFFFFF00);
#endif
#if RROP == GXxor
		    *pdst ^= (piQxelXor[2] & 0xFFFFFF00);
#endif
#if RROP == GXand
		    *pdst &= (piQxelAnd[2] | 0xFF);
#endif
#if RROP == GXor
		    *pdst |= (piQxelOr[2] & 0xFFFFFF00);
#endif
#if RROP == GXset
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[2], piQxelXor[2], 0xFFFFFF00);
#endif
		break;
	    }
	    break;
	case 3:
	    switch(leftIndex){
	    case 0:
#if RROP == GXcopy
		    *pdst++ = piQxelXor[0];
		    *pdst++ = piQxelXor[1];
		    *pdst = ((*pdst) & 0xFFFFFF00) | (piQxelXor[2] & 0xFF);
		    pdst--;
#endif
#if RROP == GXxor
		    *pdst++ ^= piQxelXor[0];
		    *pdst++ ^= piQxelXor[1];
		    *pdst-- ^= (piQxelXor[2] & 0xFF);
#endif
#if RROP == GXand
		    *pdst++ &= piQxelAnd[0];
		    *pdst++ &= piQxelAnd[1];
		    *pdst-- &= (piQxelAnd[2] | 0xFFFFFF00);
#endif
#if RROP == GXor
		    *pdst++ |= piQxelOr[0];
		    *pdst++ |= piQxelOr[1];

		    *pdst-- |= (piQxelOr[2] & 0xFF);
#endif
#if RROP == GXset
		    *pdst = DoRRop((*pdst), piQxelAnd[0], piQxelXor[0]);
		    pdst++;
		    *pdst = DoRRop((*pdst), piQxelAnd[1], piQxelXor[1]);
		    pdst++;
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[2], piQxelXor[2], 0xFF);
		    pdst--;
#endif
		    pdst--;
		break;
	    case 1:
#if RROP == GXcopy
		    *pdst = ((*pdst) & 0xFFFFFF) | (piQxelXor[0] & 0xFF000000);
		    pdst++;
		    *pdst++ = piQxelXor[1];
		    *pdst = ((*pdst) & 0xFFFFFF00) | (piQxelXor[2] & 0xFF);
		    pdst--;
#endif
#if RROP == GXxor
		    *pdst++ ^= (piQxelXor[0] & 0xFF000000);
		    *pdst++ ^= piQxelXor[1];
		    *pdst-- ^= (piQxelXor[2] & 0xFF);
#endif
#if RROP == GXand
		    *pdst++ &= (piQxelAnd[0] | 0x00FFFFFF);
		    *pdst++ &= piQxelAnd[1];
		    *pdst-- &= (piQxelAnd[2] | 0xFFFFFF00);
#endif
#if RROP == GXor
		    *pdst++ |= (piQxelOr[0] & 0xFF000000);
		    *pdst++ |= piQxelOr[1];
		    *pdst-- |= (piQxelOr[2] & 0xFF);
#endif
#if RROP == GXset
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[0], piQxelXor[0], 0xFF000000);
		    pdst++;
		    *pdst = DoRRop((*pdst), piQxelAnd[1], piQxelXor[1]);
		    pdst++;
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[2], piQxelXor[2], 0xFF);
		    pdst--;
#endif
		    pdst--;
		break;
	    case 2:
/*		pdst++;*/
#if RROP == GXcopy
		    *pdst = ((*pdst) & 0xFFFF) | (piQxelXor[1] & 0xFFFF0000);
		    pdst++;
		    *pdst = ((*pdst) & 0xFFFFFF00) | (piQxelXor[2] & 0xFF);
		    pdst--;
#endif
#if RROP == GXxor
		    *pdst++ ^= (piQxelXor[1] & 0xFFFF0000);
		    *pdst-- ^= (piQxelXor[2] & 0xFF);
#endif
#if RROP == GXand
		    *pdst++ &= (piQxelAnd[1] | 0xFFFF);
		    *pdst-- &= (piQxelAnd[2] | 0xFFFFFF00);
#endif
#if RROP == GXor
		    *pdst++ |= (piQxelOr[1] & 0xFFFF0000);
		    *pdst-- |= (piQxelOr[2] & 0xFF);
#endif
#if RROP == GXset
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[1], piQxelXor[1], 0xFFFF0000);
		    pdst++;
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[2], piQxelXor[2], 0xFF);
		    pdst--;
#endif
		break;
	    }
	    break;
	case 2: /* leftIndex + w = 2*/
	    if(leftIndex){
#if RROP == GXcopy
		    *pdst = ((*pdst) & 0xFFFFFF) | (piQxelXor[0] & 0xFF000000);
		    pdst++;
		    *pdst = ((*pdst) & 0xFFFF0000) | (piQxelXor[1] & 0xFFFF);
		    pdst--;
#endif
#if RROP == GXxor
		    *pdst++ ^= (piQxelXor[0] & 0xFF000000);
		    *pdst-- ^= (piQxelXor[1] & 0xFFFF);
#endif
#if RROP == GXand
		    *pdst++ &= (piQxelAnd[0] | 0xFFFFFF);
		    *pdst-- &= (piQxelAnd[1] | 0xFFFF0000);
#endif
#if RROP == GXor
		    *pdst++ |= (piQxelOr[0] & 0xFF000000);
		    *pdst-- |= (piQxelOr[1] & 0xFFFF);
#endif
#if RROP == GXset
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[0], piQxelXor[0], 0xFF000000);
		    pdst++;
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[1], piQxelXor[1], 0xFFFF);
		    pdst--;
#endif
	    }
	    else{ /*case 2 leftIndex === 0 */
#if RROP == GXcopy
		    *pdst++ = piQxelXor[0];
		    *pdst = ((*pdst) & 0xFFFF0000) | (piQxelXor[1] & 0xFFFF);
		    pdst--;
#endif
#if RROP == GXxor
		    *pdst++ ^= piQxelXor[0];
		    *pdst-- ^= (piQxelXor[1] & 0xFFFF);
#endif
#if RROP == GXand
		    *pdst++ &= piQxelAnd[0];
		    *pdst-- &= (piQxelAnd[1] | 0xFFFF0000);
#endif
#if RROP == GXor
		    *pdst++ |= piQxelOr[0];
		    *pdst-- |= (piQxelOr[1] & 0xFFFF);
#endif
#if RROP == GXset
		    *pdst = DoRRop((*pdst), piQxelAnd[0], piQxelXor[0]);
		    pdst++;
		    *pdst = DoMaskRRop((*pdst), piQxelAnd[1], piQxelXor[1], 0xFFFF);
		    pdst--;
#endif
	    }
	    break;
	case 1: /*only if leftIndex = 0 and w = 1*/
#if RROP == GXcopy
		*pdst = ((*pdst) & 0xFF000000) | (piQxelXor[0] & 0xFFFFFF);
#endif
#if RROP == GXxor
		*pdst ^= (piQxelXor[0] & 0xFFFFFF);
#endif
#if RROP == GXand
		*pdst &= (piQxelAnd[0] | 0xFF000000);
#endif
#if RROP == GXor
		*pdst |= (piQxelOr[0] & 0xFFFFFF);
#endif
#if RROP == GXset
		*pdst = DoMaskRRop((*pdst), piQxelAnd[0], piQxelXor[0], 0xFFFFFF);
#endif
	    break;
	case 0: /*never*/
	    break;
	default:
	{
	    switch(leftIndex){
		    case 0:
			break;
		    case 1:
#if RROP == GXcopy
			*pdst = ((*pdst) & 0xFFFFFF) | (piQxelXor[0] & 0xFF000000);
			pdst++;
			*pdst++ = piQxelXor[1];
			*pdst++ = piQxelXor[2];
#endif
#if RROP == GXxor
			*pdst++ ^= (piQxelXor[0] & 0xFF000000);
			*pdst++ ^= piQxelXor[1];
			*pdst++ ^= piQxelXor[2];
#endif
#if RROP == GXand
			*pdst++ &= (piQxelAnd[0] | 0xFFFFFF);
			*pdst++ &= piQxelAnd[1];
			*pdst++ &= piQxelAnd[2];
#endif
#if RROP == GXor
			*pdst++ |= (piQxelOr[0] & 0xFF000000);
			*pdst++ |= piQxelOr[1];
			*pdst++ |= piQxelOr[2];
#endif
#if RROP == GXset
			*pdst = DoMaskRRop((*pdst), piQxelAnd[0], piQxelXor[0], 0xFF000000);
			pdst++;
			*pdst = DoRRop((*pdst), piQxelAnd[1], piQxelXor[1]);
			pdst++;
			*pdst = DoRRop((*pdst), piQxelAnd[2], piQxelXor[2]);
			pdst++;
#endif
		    break;
		    case 2:
#if RROP == GXcopy
			*pdst = (((*pdst) & 0xFFFF) | (piQxelXor[1] & 0xFFFF0000));
			pdst++;
			*pdst++ = piQxelXor[2];
#endif
#if RROP == GXxor
			*pdst++ ^=(piQxelXor[1] & 0xFFFF0000);
			*pdst++ ^= piQxelXor[2];
#endif
#if RROP == GXand
			*pdst++ &= (piQxelAnd[1] | 0xFFFF);
			*pdst++ &= piQxelAnd[2];
#endif
#if RROP == GXor
			*pdst++ |= (piQxelOr[1] & 0xFFFF0000);
			*pdst++ |= piQxelOr[2];
#endif
#if RROP == GXset
			*pdst = DoMaskRRop((*pdst), piQxelAnd[1], piQxelXor[1], 0xFFFF0000);
			pdst++;
			*pdst = DoRRop((*pdst), piQxelAnd[2], piQxelXor[2]);
			pdst++;
#endif
			break;
		    case 3:
#if RROP == GXcopy
			*pdst = ((*pdst) & 0xFF) | (piQxelXor[2] & 0xFFFFFF00);
			pdst++;
#endif
#if RROP == GXxor
			*pdst++ ^= (piQxelXor[2] & 0xFFFFFF00);
#endif
#if RROP == GXand
			*pdst++ &= (piQxelAnd[2] | 0xFF);
#endif
#if RROP == GXor
			*pdst++ |= (piQxelOr[2] & 0xFFFFFF00);
#endif
#if RROP == GXset
			*pdst = DoMaskRRop((*pdst), piQxelAnd[2], piQxelXor[2], 0xFFFFFF00);
			pdst++;
#endif
			break;
		    }
		    while(nlmiddle--){
#if RROP == GXcopy
			*pdst++ = piQxelXor[0];
			*pdst++ = piQxelXor[1];
			*pdst++ = piQxelXor[2];
#endif
#if RROP == GXxor
			*pdst++ ^= piQxelXor[0];
			*pdst++ ^= piQxelXor[1];
			*pdst++ ^= piQxelXor[2];
#endif
#if RROP == GXand
			*pdst++ &= piQxelAnd[0];
			*pdst++ &= piQxelAnd[1];
			*pdst++ &= piQxelAnd[2];
#endif
#if RROP == GXor
			*pdst++ |= piQxelOr[0];
			*pdst++ |= piQxelOr[1];
			*pdst++ |= piQxelOr[2];
#endif
#if RROP == GXset
			*pdst = DoRRop((*pdst), piQxelAnd[0], piQxelXor[0]);
			pdst++;
			*pdst = DoRRop((*pdst), piQxelAnd[1], piQxelXor[1]);
			pdst++;
			*pdst = DoRRop((*pdst), piQxelAnd[2], piQxelXor[2]);
			pdst++;
#endif
		}
		switch(rightIndex){
		case 0:
		  break;
		case 1:
#if RROP == GXcopy
		  *pdst = ((*pdst) & 0xFF000000) | (piQxelXor[0] & 0xFFFFFF);
		  pdst++;
#endif
#if RROP == GXxor
		  *pdst++ ^= (piQxelXor[0] & 0xFFFFFF);
#endif
#if RROP == GXand
		  *pdst++ &= (piQxelAnd[0] | 0xFF);
#endif
#if RROP == GXor
		  *pdst++ |= (piQxelOr[0] & 0xFFFFFF);
#endif
#if RROP == GXset
		  *pdst = DoMaskRRop((*pdst), piQxelAnd[0], piQxelXor[0], 0xFFFFFF);
#endif
		  break;
		case 2:
#if RROP == GXcopy
		  *pdst++ = piQxelXor[0];
		  *pdst = ((*pdst) & 0xFFFF0000) | (piQxelXor[1] & 0xFFFF);
		  pdst++;
#endif
#if RROP == GXxor
		  *pdst++ ^= piQxelXor[0];
		  *pdst++ ^= (piQxelXor[1] & 0xFFFF);
#endif
#if RROP == GXand
		  *pdst++ &= piQxelAnd[0];
		  *pdst++ &= (piQxelAnd[1] | 0xFFFF0000);
#endif
#if RROP == GXor
		  *pdst++ |= piQxelOr[0];
		  *pdst++ |= (piQxelOr[1] & 0xFFFF);
#endif
#if RROP == GXset
		  *pdst = DoRRop((*pdst), piQxelAnd[0], piQxelXor[0]);
		  pdst++;
		  *pdst = DoMaskRRop((*pdst), piQxelAnd[1], piQxelXor[1], 0xFFFF);
		  pdst++;
#endif
		  break;
		case 3:
#if RROP == GXcopy
		  *pdst++ = piQxelXor[0];
		  *pdst++ = piQxelXor[1];
		  *pdst = ((*pdst) & 0xFFFFFF00) | (piQxelXor[2] & 0xFF);
		  pdst++;
#endif
#if RROP == GXxor
		  *pdst++ ^= piQxelXor[0];
		  *pdst++ ^= piQxelXor[1];
		  *pdst++ ^= (piQxelXor[2] & 0xFF);
#endif
#if RROP == GXand
		  *pdst++ &= piQxelAnd[0];
		  *pdst++ &= piQxelAnd[1];
		  *pdst++ &= (piQxelAnd[2] | 0xFFFFFF00);
#endif
#if RROP == GXor
		  *pdst++ |= piQxelOr[0];
		  *pdst++ |= piQxelOr[1];
		  *pdst++ |= (piQxelOr[2] & 0xFF);
#endif
#if RROP == GXset
		  *pdst = DoRRop((*pdst), piQxelAnd[0], piQxelXor[0]);
		  pdst++;
		  *pdst = DoRRop((*pdst), piQxelAnd[1], piQxelXor[1]);
		  pdst++;
		  *pdst = DoMaskRRop((*pdst), piQxelAnd[2], piQxelXor[2], 0xFF);
		  pdst++;
#endif
		  break;
		}
	}
}
#else
#if PSZ == 8
	if (w <= PGSZB)
	{
	    register char   *addrb;

	    addrb = ((char *) pdst) + x;
	    while (w--)
	    {
		RROP_SOLID (addrb);
		addrb++;
	    }
	}
#else
	if ((x & PIM) + w <= PPW)
	{
	    pdst += x >> PWSH;
	    maskpartialbits (x, w, startmask);
	    RROP_SOLID_MASK (pdst, startmask);
	}
#endif
	else
	{
	    pdst += x >> PWSH;
	    maskbits (x, w, startmask, endmask, nlmiddle);
	    if (startmask)
	    {
		RROP_SOLID_MASK (pdst, startmask);
		++pdst;
	    }
	    
	    RROP_SPAN(pdst,nlmiddle)
	    if (endmask)
	    {
		RROP_SOLID_MASK (pdst, endmask);
	    }
	}
#endif
    }
    DEALLOCATE_LOCAL(pptFree);
    DEALLOCATE_LOCAL(pwidthFree);
    RROP_UNDECLARE
}
