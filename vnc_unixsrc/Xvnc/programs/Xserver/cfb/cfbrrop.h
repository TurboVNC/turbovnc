/*
 * $XConsortium: cfbrrop.h,v 1.10 95/06/08 23:20:39 gildea Exp $
 * $XFree86: xc/programs/Xserver/cfb/cfbrrop.h,v 3.2 1997/01/08 20:32:41 dawes Exp $
 *
Copyright (c) 1989  X Consortium

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
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#ifndef GXcopy
#include "X.h"
#endif

#define RROP_FETCH_GC(gc) \
    RROP_FETCH_GCPRIV(((cfbPrivGCPtr)(gc)->devPrivates[cfbGCPrivateIndex].ptr))

#ifndef RROP
#define RROP GXset
#endif

#if RROP == GXcopy
#if PSZ == 24
#define RROP_DECLARE	register unsigned long	rrop_xor; \
    unsigned long piQxelXor[3], spiQxelXor[8];
#define RROP_FETCH_GCPRIV(devPriv)  rrop_xor = (devPriv)->xor; \
    spiQxelXor[0] = rrop_xor & 0xFFFFFF; \
    spiQxelXor[2] = rrop_xor << 24; \
    spiQxelXor[3] = (rrop_xor & 0xFFFF00)>> 8; \
    spiQxelXor[4] = rrop_xor << 16; \
    spiQxelXor[5] = (rrop_xor & 0xFF0000)>> 16; \
    spiQxelXor[6] = rrop_xor << 8; \
    spiQxelXor[1] = spiQxelXor[7] = 0; \
    piQxelXor[0] = (rrop_xor & 0xFFFFFF)|(rrop_xor << 24); \
    piQxelXor[1] = (rrop_xor << 16)|((rrop_xor & 0xFFFF00)>> 8); \
    piQxelXor[2] = (rrop_xor << 8)|((rrop_xor & 0xFF0000)>> 16);
#define RROP_SOLID24(dst,index)	    {\
	    register int idx = ((index) & 3)<< 1; \
	    *(dst) = (*(dst) & cfbrmask[idx])|spiQxelXor[idx]; \
	    if (idx == 2  ||  idx == 4){ \
              idx++; \
	      *((dst)+1) = (*((dst)+1) & cfbrmask[idx])|spiQxelXor[idx]; \
	    } \
	}
#define RROP_SOLID(dst, idx) \
	    (*(dst) = piQxelXor[(idx)])
#define RROP_SOLID_MASK(dst,mask,idx) \
	    (*(dst) = (*(dst) & ~(mask))|(piQxelXor[(idx)] & (mask)))
#else
#define RROP_FETCH_GCPRIV(devPriv)  rrop_xor = (devPriv)->xor;
#define RROP_DECLARE	register unsigned long	rrop_xor;
#define RROP_SOLID(dst)	    (*(dst) = (rrop_xor))
#define RROP_SOLID_MASK(dst,mask) (*(dst) = (*(dst) & ~(mask)) | ((rrop_xor) & (mask)))
#define RROP_SOLID_lu(dst)	    stl_u(rrop_xor, dst)
#define RROP_SOLID_MASK_lu(dst,mask) stl_u((ldl_u(dst) & ~(mask)) | ((rrop_xor) & (mask)), dst)
#endif
#define RROP_NAME(prefix)   RROP_NAME_CAT(prefix,Copy)
#endif /* GXcopy */

#if RROP == GXxor
#if PSZ == 24
#define RROP_DECLARE	register unsigned long	rrop_xor; \
    unsigned long piQxelXor[3], spiQxelXor[8];
#define RROP_FETCH_GCPRIV(devPriv)  rrop_xor = (devPriv)->xor; \
    spiQxelXor[0] = rrop_xor & 0xFFFFFF; \
    spiQxelXor[2] = rrop_xor << 24; \
    spiQxelXor[3] = (rrop_xor & 0xFFFF00)>> 8; \
    spiQxelXor[4] = rrop_xor << 16; \
    spiQxelXor[5] = (rrop_xor & 0xFF0000)>> 16; \
    spiQxelXor[6] = rrop_xor << 8; \
    spiQxelXor[1] = spiQxelXor[7] = 0; \
    piQxelXor[0] = (rrop_xor & 0xFFFFFF)|(rrop_xor << 24); \
    piQxelXor[1] = (rrop_xor << 16)|((rrop_xor & 0xFFFF00)>> 8); \
    piQxelXor[2] = (rrop_xor << 8)|((rrop_xor & 0xFF0000)>> 16);
#define RROP_SOLID24(dst,index)	     {\
	    register int idx = ((index) & 3)<< 1; \
	    *(dst) ^= spiQxelXor[idx]; \
	    if (idx == 2  ||  idx == 4) \
	      *((dst)+1) ^= spiQxelXor[idx+1]; \
	}
#define RROP_SOLID(dst,idx) \
	    (*(dst) ^= piQxelXor[(idx)])
#define RROP_SOLID_MASK(dst,mask,idx) \
	    (*(dst) ^= (piQxelXor[(idx)] & (mask)))
#else
#define RROP_DECLARE	register unsigned long	rrop_xor;
#define RROP_FETCH_GCPRIV(devPriv)  rrop_xor = (devPriv)->xor;
#define RROP_SOLID(dst)	    (*(dst) ^= (rrop_xor))
#define RROP_SOLID_MASK(dst,mask) (*(dst) ^= ((rrop_xor) & (mask)))
#endif
#define RROP_NAME(prefix)   RROP_NAME_CAT(prefix,Xor)
#endif /* GXxor */

#if RROP == GXand
#if PSZ == 24
#define RROP_DECLARE	register unsigned long	rrop_and; \
    unsigned long piQxelAnd[3], spiQxelAnd[6];
#define RROP_FETCH_GCPRIV(devPriv)  rrop_and = (devPriv)->and; \
    spiQxelAnd[0] = (rrop_and & 0xFFFFFF) | 0xFF000000; \
    spiQxelAnd[2] = (rrop_and << 24) | 0xFFFFFF; \
    spiQxelAnd[3] = ((rrop_and & 0xFFFF00)>> 8) | 0xFFFF0000; \
    spiQxelAnd[4] = (rrop_and << 16) | 0xFFFF; \
    spiQxelAnd[5] = ((rrop_and & 0xFF0000)>> 16) | 0xFFFFFF00; \
    spiQxelAnd[1] = (rrop_and << 8) | 0xFF; \
    piQxelAnd[0] = (rrop_and & 0xFFFFFF)|(rrop_and << 24); \
    piQxelAnd[1] = (rrop_and << 16)|((rrop_and & 0xFFFF00)>> 8); \
    piQxelAnd[2] = (rrop_and << 8)|((rrop_and & 0xFF0000)>> 16); 
#define RROP_SOLID24(dst,index)	    {\
	    switch((index) & 3){ \
	    case 0: \
	      *(dst) &= spiQxelAnd[0]; \
	      break; \
	    case 3: \
	      *(dst) &= spiQxelAnd[1]; \
	      break; \
	    case 1: \
	      *(dst) &= spiQxelAnd[2]; \
	      *((dst)+1) &= spiQxelAnd[3]; \
	      break; \
	    case 2: \
	      *(dst) &= spiQxelAnd[4]; \
	      *((dst)+1) &= spiQxelAnd[5]; \
	      break; \
	    } \
	    }
#define RROP_SOLID(dst,idx) \
	    (*(dst) &= piQxelAnd[(idx)])
#define RROP_SOLID_MASK(dst,mask,idx) \
	    (*(dst) &= (piQxelAnd[(idx)] | ~(mask)))
#else
#define RROP_DECLARE	register unsigned long	rrop_and;
#define RROP_FETCH_GCPRIV(devPriv)  rrop_and = (devPriv)->and;
#define RROP_SOLID(dst)	    (*(dst) &= (rrop_and))
#define RROP_SOLID_MASK(dst,mask) (*(dst) &= ((rrop_and) | ~(mask)))
#endif
#define RROP_NAME(prefix)   RROP_NAME_CAT(prefix,And)
#endif /* GXand */

#if RROP == GXor
#if PSZ == 24
#define RROP_DECLARE	register unsigned long	rrop_or; \
    unsigned long piQxelOr[3], spiQxelOr[6];
#define RROP_FETCH_GCPRIV(devPriv)  rrop_or = (devPriv)->xor; \
    spiQxelOr[0] = rrop_or & 0xFFFFFF; \
    spiQxelOr[1] = rrop_or << 24; \
    spiQxelOr[2] = rrop_or << 16; \
    spiQxelOr[3] = rrop_or << 8; \
    spiQxelOr[4] = (rrop_or & 0xFFFF00)>> 8; \
    spiQxelOr[5] = (rrop_or & 0xFF0000)>> 16; \
    piQxelOr[0] = (rrop_or & 0xFFFFFF)|(rrop_or << 24); \
    piQxelOr[1] = (rrop_or << 16)|((rrop_or & 0xFFFF00)>> 8); \
    piQxelOr[2] = (rrop_or << 8)|((rrop_or & 0xFF0000)>> 16);
#define RROP_SOLID24(dst,index)	     {\
	    switch((index) & 3){ \
	    case 0: \
	      *(dst) |= spiQxelOr[0]; \
	      break; \
	    case 3: \
	      *(dst) |= spiQxelOr[3]; \
	      break; \
	    case 1: \
	      *(dst) |= spiQxelOr[1]; \
	      *((dst)+1) |= spiQxelOr[4]; \
	      break; \
	    case 2: \
	      *(dst) |= spiQxelOr[2]; \
	      *((dst)+1) |= spiQxelOr[5]; \
	      break; \
	    } \
	    }
#define RROP_SOLID(dst,idx) \
	    (*(dst) |= piQxelOr[(idx)])
#define RROP_SOLID_MASK(dst,mask,idx) \
	    (*(dst) |= (piQxelOr[(idx)] & (mask)))
#else
#define RROP_DECLARE	register unsigned long	rrop_or;
#define RROP_FETCH_GCPRIV(devPriv)  rrop_or = (devPriv)->xor;
#define RROP_SOLID(dst)	    (*(dst) |= (rrop_or))
#define RROP_SOLID_MASK(dst,mask) (*(dst) |= ((rrop_or) & (mask)))
#endif
#define RROP_NAME(prefix)   RROP_NAME_CAT(prefix,Or)
#endif /* GXor */

#if RROP == GXnoop
#define RROP_DECLARE
#define RROP_FETCH_GCPRIV(devPriv)
#define RROP_SOLID(dst)
#define RROP_SOLID_MASK(dst,mask)
#define RROP_NAME(prefix)   RROP_NAME_CAT(prefix,Noop)
#endif /* GXnoop */

#if RROP ==  GXset
#if PSZ == 24
#define RROP_DECLARE	    register unsigned long	rrop_and, rrop_xor; \
    unsigned long piQxelAnd[3], piQxelXor[3],  spiQxelAnd[6], spiQxelXor[6];
#define RROP_FETCH_GCPRIV(devPriv)  rrop_and = (devPriv)->and; \
				    rrop_xor = (devPriv)->xor; \
    spiQxelXor[0] = rrop_xor & 0xFFFFFF; \
    spiQxelXor[1] = rrop_xor << 24; \
    spiQxelXor[2] = rrop_xor << 16; \
    spiQxelXor[3] = rrop_xor << 8; \
    spiQxelXor[4] = (rrop_xor & 0xFFFF00)>> 8; \
    spiQxelXor[5] = (rrop_xor & 0xFF0000)>> 16; \
    spiQxelAnd[0] = (rrop_and & 0xFFFFFF) | 0xFF000000; \
    spiQxelAnd[1] = (rrop_and << 24) | 0xFFFFFF; \
    spiQxelAnd[2] = (rrop_and << 16) | 0xFFFF; \
    spiQxelAnd[3] = (rrop_and << 8) | 0xFF; \
    spiQxelAnd[4] = ((rrop_and & 0xFFFF00)>> 8) | 0xFFFF0000; \
    spiQxelAnd[5] = ((rrop_and & 0xFF0000)>> 16) | 0xFFFFFF00; \
    piQxelAnd[0] = (rrop_and & 0xFFFFFF)|(rrop_and << 24); \
    piQxelAnd[1] = (rrop_and << 16)|((rrop_and & 0xFFFF00)>> 8); \
    piQxelAnd[2] = (rrop_and << 8)|((rrop_and & 0xFF0000)>> 16); \
    piQxelXor[0] = (rrop_xor & 0xFFFFFF)|(rrop_xor << 24); \
    piQxelXor[1] = (rrop_xor << 16)|((rrop_xor & 0xFFFF00)>> 8); \
    piQxelXor[2] = (rrop_xor << 8)|((rrop_xor & 0xFF0000)>> 16);
#define RROP_SOLID24(dst,index)	     {\
	    switch((index) & 3){ \
	    case 0: \
	      *(dst) = ((*(dst) & (piQxelAnd[0] |0xFF000000))^(piQxelXor[0] & 0xFFFFFF)); \
	      break; \
	    case 3: \
	      *(dst) = ((*(dst) & (piQxelAnd[2]|0xFF))^(piQxelXor[2] & 0xFFFFFF00)); \
	      break; \
	    case 1: \
	      *(dst) = ((*(dst) & (piQxelAnd[0]|0xFFFFFF))^(piQxelXor[0] & 0xFF000000)); \
	      *((dst)+1) = ((*((dst)+1) & (piQxelAnd[1]|0xFFFF0000))^(piQxelXor[1] & 0xFFFF)); \
	      break; \
	    case 2: \
	      *(dst) = ((*(dst) & (piQxelAnd[1]|0xFFFF))^(piQxelXor[1] & 0xFFFF0000)); \
	      *((dst)+1) = ((*((dst)+1) & (piQxelAnd[2]|0xFFFFFF00))^(piQxelXor[2] & 0xFF)); \
	      break; \
	    } \
	    }
#define RROP_SOLID(dst,idx) \
	    (*(dst) = DoRRop (*(dst), piQxelAnd[(idx)], piQxelXor[(idx)]))
#define RROP_SOLID_MASK(dst,mask,idx) \
	    (*(dst) = DoMaskRRop (*(dst), piQxelAnd[(idx)], piQxelXor[(idx)], (mask)))
#else
#define RROP_DECLARE	    register unsigned long	rrop_and, rrop_xor;
#define RROP_FETCH_GCPRIV(devPriv)  rrop_and = (devPriv)->and; \
				    rrop_xor = (devPriv)->xor;
#define RROP_SOLID(dst)	    (*(dst) = DoRRop (*(dst), rrop_and, rrop_xor))
#define RROP_SOLID_MASK(dst,mask)   (*(dst) = DoMaskRRop (*(dst), rrop_and, rrop_xor, (mask)))
#endif
#define RROP_NAME(prefix)   RROP_NAME_CAT(prefix,General)
#endif /* GXset */

#define RROP_UNROLL_CASE1(p,i)    case (i): RROP_SOLID((p) - (i));
#define RROP_UNROLL_CASE2(p,i)    RROP_UNROLL_CASE1(p,(i)+1) RROP_UNROLL_CASE1(p,i)
#define RROP_UNROLL_CASE4(p,i)    RROP_UNROLL_CASE2(p,(i)+2) RROP_UNROLL_CASE2(p,i)
#define RROP_UNROLL_CASE8(p,i)    RROP_UNROLL_CASE4(p,(i)+4) RROP_UNROLL_CASE4(p,i)
#define RROP_UNROLL_CASE16(p,i)   RROP_UNROLL_CASE8(p,(i)+8) RROP_UNROLL_CASE8(p,i)
#define RROP_UNROLL_CASE3(p)	RROP_UNROLL_CASE2(p,2) RROP_UNROLL_CASE1(p,1)
#define RROP_UNROLL_CASE7(p)	RROP_UNROLL_CASE4(p,4) RROP_UNROLL_CASE3(p)
#define RROP_UNROLL_CASE15(p)	RROP_UNROLL_CASE8(p,8) RROP_UNROLL_CASE7(p)
#define RROP_UNROLL_CASE31(p)	RROP_UNROLL_CASE16(p,16) RROP_UNROLL_CASE15(p)
#ifdef LONG64
#define RROP_UNROLL_CASE63(p)	RROP_UNROLL_CASE32(p,32) RROP_UNROLL_CASE31(p)
#endif /* LONG64 */

#define RROP_UNROLL_LOOP1(p,i) RROP_SOLID((p) + (i));
#define RROP_UNROLL_LOOP2(p,i) RROP_UNROLL_LOOP1(p,(i)) RROP_UNROLL_LOOP1(p,(i)+1)
#define RROP_UNROLL_LOOP4(p,i) RROP_UNROLL_LOOP2(p,(i)) RROP_UNROLL_LOOP2(p,(i)+2)
#define RROP_UNROLL_LOOP8(p,i) RROP_UNROLL_LOOP4(p,(i)) RROP_UNROLL_LOOP4(p,(i)+4)
#define RROP_UNROLL_LOOP16(p,i) RROP_UNROLL_LOOP8(p,(i)) RROP_UNROLL_LOOP8(p,(i)+8)
#define RROP_UNROLL_LOOP32(p,i) RROP_UNROLL_LOOP16(p,(i)) RROP_UNROLL_LOOP16(p,(i)+16)
#ifdef LONG64
#define RROP_UNROLL_LOOP64(p,i) RROP_UNROLL_LOOP32(p,(i)) RROP_UNROLL_LOOP32(p,(i)+32)
#endif /* LONG64 */

#if defined (FAST_CONSTANT_OFFSET_MODE) && defined (SHARED_IDCACHE) && (RROP == GXcopy)

#ifdef LONG64
#define RROP_UNROLL_SHIFT	6
#define RROP_UNROLL_CASE(p)	RROP_UNROLL_CASE63(p)
#define RROP_UNROLL_LOOP(p)	RROP_UNROLL_LOOP64(p,-64)
#else /* not LONG64 */
#define RROP_UNROLL_SHIFT	5
#define RROP_UNROLL_CASE(p)	RROP_UNROLL_CASE31(p)
#define RROP_UNROLL_LOOP(p)	RROP_UNROLL_LOOP32(p,-32)
#endif /* LONG64 */
#define RROP_UNROLL		(1<<RROP_UNROLL_SHIFT)
#define RROP_UNROLL_MASK	(RROP_UNROLL-1)

#define RROP_SPAN(pdst,nmiddle) {\
    int part = (nmiddle) & RROP_UNROLL_MASK; \
    (nmiddle) >>= RROP_UNROLL_SHIFT; \
    (pdst) += part * (sizeof (unsigned long) / sizeof (*pdst)); \
    switch (part) {\
	RROP_UNROLL_CASE((unsigned long *) (pdst)) \
    } \
    while (--(nmiddle) >= 0) { \
	(pdst) += RROP_UNROLL * (sizeof (unsigned long) / sizeof (*pdst)); \
	RROP_UNROLL_LOOP((unsigned long *) (pdst)) \
    } \
}
#else
#define RROP_SPAN(pdst,nmiddle) \
    while (--(nmiddle) >= 0) { \
	RROP_SOLID((unsigned long *) (pdst)); \
	(pdst) += sizeof (unsigned long) / sizeof (*pdst); \
    }
#define RROP_SPAN_lu(pdst,nmiddle) \
    while (--(nmiddle) >= 0) { \
	RROP_SOLID_lu((unsigned long *) (pdst)); \
	(pdst) += sizeof (unsigned long) / sizeof (*pdst); \
    }
#endif

#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define RROP_NAME_CAT(prefix,suffix)	prefix##suffix
#else
#define RROP_NAME_CAT(prefix,suffix)	prefix/**/suffix
#endif
