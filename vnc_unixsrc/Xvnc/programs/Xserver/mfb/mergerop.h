/*
 * $XConsortium: mergerop.h,v 1.11 95/06/08 23:20:39 gildea Exp $
 * $XFree86: xc/programs/Xserver/mfb/mergerop.h,v 3.1 1996/06/29 09:10:20 dawes Exp $
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

#ifndef _MERGEROP_H_
#define _MERGEROP_H_

#ifndef GXcopy
#include "X.h"
#endif

typedef struct _mergeRopBits {
    unsigned long   ca1, cx1, ca2, cx2;
} mergeRopRec, *mergeRopPtr;

extern mergeRopRec	mergeRopBits[16];

#if PPW != PGSZ	/* cfb */
#define DeclareMergeRop() unsigned long   _ca1, _cx1, _ca2, _cx2;
#define DeclarePrebuiltMergeRop()	unsigned long	_cca, _ccx;
#else /* mfb */
#define DeclareMergeRop() unsigned long   _ca1, _cx1, _ca2, _cx2;
#define DeclarePrebuiltMergeRop()	unsigned long	_cca, _ccx;
#endif

#if PPW != PGSZ	/* cfb */
#define InitializeMergeRop(alu,pm) {\
    unsigned long   _pm; \
    mergeRopPtr  _bits; \
    _pm = PFILL(pm); \
    _bits = &mergeRopBits[alu]; \
    _ca1 = _bits->ca1 &  _pm; \
    _cx1 = _bits->cx1 | ~_pm; \
    _ca2 = _bits->ca2 &  _pm; \
    _cx2 = _bits->cx2 &  _pm; \
}
#else /* mfb */
#define InitializeMergeRop(alu,pm) {\
    mergeRopPtr  _bits; \
    _bits = &mergeRopBits[alu]; \
    _ca1 = _bits->ca1; \
    _cx1 = _bits->cx1; \
    _ca2 = _bits->ca2; \
    _cx2 = _bits->cx2; \
}
#endif

/* AND has higher precedence than XOR */

#define DoMergeRop(src, dst) \
    ((dst) & ((src) & _ca1 ^ _cx1) ^ ((src) & _ca2 ^ _cx2))

#define DoMergeRop24(src,dst,index) {\
	register int idx = ((index) & 3)<< 1; \
	*(dst) = (((*(dst)) & cfbrmask[idx]) | (((*(dst)) & cfbmask[idx]) & \
	((((src) & _ca1 ^ _cx1)<<cfb24Shift[idx])&cfbmask[idx]) ^ \
	((((src) & _ca2 ^ _cx2)<<cfb24Shift[idx])&cfbmask[idx]))); \
	idx++; \
	(dst)++; \
	*(dst) = (((*(dst)) & cfbrmask[idx]) | (((*(dst)) & cfbmask[idx]) & \
	((((src) & _ca1 ^ _cx1)>>cfb24Shift[idx])&cfbmask[idx]) ^ \
	((((src) & _ca2 ^ _cx2)>>cfb24Shift[idx])&cfbmask[idx]))); \
	(dst)--; \
	}

#define DoPrebuiltMergeRop(dst) ((dst) & _cca ^ _ccx)

#define DoPrebuiltMergeRop24(dst,index) { \
	register int idx = ((index) & 3)<< 1; \
	*(dst) = (((*(dst)) & cfbrmask[idx]) | (((*(dst)) & cfbmask[idx]) &\
	(( _cca <<cfb24Shift[idx])&cfbmask[idx]) ^ \
	(( _ccx <<cfb24Shift[idx])&cfbmask[idx]))); \
	idx++; \
	(dst)++; \
	*(dst) = (((*(dst)) & cfbrmask[idx]) | (((*(dst)) & cfbmask[idx]) &\
	(( _cca >>cfb24Shift[idx])&cfbmask[idx]) ^ \
	(( _ccx >>cfb24Shift[idx])&cfbmask[idx]))); \
	(dst)--; \
	}

#define DoMaskPrebuiltMergeRop(dst,mask) \
    ((dst) & (_cca | ~(mask)) ^ (_ccx & (mask)))

#define PrebuildMergeRop(src) ((_cca = (src) & _ca1 ^ _cx1), \
			       (_ccx = (src) & _ca2 ^ _cx2))

#define DoMaskMergeRop(src, dst, mask) \
    ((dst) & (((src) & _ca1 ^ _cx1) | ~(mask)) ^ (((src) & _ca2 ^ _cx2) & (mask)))

#define DoMaskMergeRop24(src, dst, mask, index)  {\
	register int idx = ((index) & 3)<< 1; \
	*(dst) = (((*(dst)) & cfbrmask[idx]) | (((*(dst)) & cfbmask[idx]) & \
	((((((src) & _ca1 ^ _cx1) |(~mask))<<cfb24Shift[idx])&cfbmask[idx]) ^ \
	(((((src) & _ca2 ^ _cx2)&(mask))<<cfb24Shift[idx])&cfbmask[idx])))); \
	idx++; \
	(dst)++; \
	*(dst) = (((*(dst)) & cfbrmask[idx]) | (((*(dst)) & cfbmask[idx]) & \
	((((((src) & _ca1 ^ _cx1) |(~mask))>>cfb24Shift[idx])&cfbmask[idx]) ^ \
	(((((src) & _ca2 ^ _cx2)&(mask))>>cfb24Shift[idx])&cfbmask[idx])))); \
	(dst)--; \
	}

#ifndef MROP
#define MROP 0
#endif

#define Mclear		(1<<GXclear)
#define Mand		(1<<GXand)
#define MandReverse	(1<<GXandReverse)
#define Mcopy		(1<<GXcopy)
#define MandInverted	(1<<GXandInverted)
#define Mnoop		(1<<GXnoop)
#define Mxor		(1<<GXxor)
#define Mor		(1<<GXor)
#define Mnor		(1<<GXnor)
#define Mequiv		(1<<GXequiv)
#define Minvert		(1<<GXinvert)
#define MorReverse	(1<<GXorReverse)
#define McopyInverted	(1<<GXcopyInverted)
#define MorInverted	(1<<GXorInverted)
#define Mnand		(1<<GXnand)
#define Mset		(1<<GXset)

#define MROP_PIXEL24(pix, idx) \
	(((*(pix) & cfbmask[(idx)<<1]) >> cfb24Shift[(idx)<<1])| \
	((*((pix)+1) & cfbmask[((idx)<<1)+1]) << cfb24Shift[((idx)<<1)+1]))

#define MROP_SOLID24P(src,dst,sindex, index) \
	MROP_SOLID24(MROP_PIXEL24(src,sindex),dst,index)
#define MROP_MASK24P(src,dst,mask,sindex,index)	\
	MROP_MASK24(MROP_PIXEL24(src,sindex),dst,mask,index)

#if (MROP) == Mcopy
#define MROP_DECLARE()
#define MROP_DECLARE_REG()
#define MROP_INITIALIZE(alu,pm)
#define MROP_SOLID(src,dst)	(src)
#define MROP_SOLID24(src,dst,index)	    {\
	register int idx = ((index) & 3)<< 1; \
	*(dst) = (*(dst) & cfbrmask[idx])|(((src)<<cfb24Shift[idx])&cfbmask[idx]); \
	idx++; \
	*((dst)+1) = (*((dst)+1) & cfbrmask[idx])|(((src)>>cfb24Shift[idx])&cfbmask[idx]); \
	}
#define MROP_MASK(src,dst,mask)	((dst) & ~(mask) | (src) & (mask))
#define MROP_MASK24(src,dst,mask,index)	{\
	register int idx = ((index) & 3)<< 1; \
	*(dst) = (*(dst) & cfbrmask[idx] &(~(((mask)<< cfb24Shift[idx])&cfbmask[idx])) | \
		((((src)&(mask))<<cfb24Shift[idx])&cfbmask[idx])); \
	idx++; \
	*((dst)+1) = (*((dst)+1) & cfbrmask[idx] &(~(((mask)>>cfb24Shift[idx])&cfbmask[idx])) | \
		((((src)&(mask))>>cfb24Shift[idx])&cfbmask[idx])); \
	}
#define MROP_NAME(prefix)	MROP_NAME_CAT(prefix,Copy)
#endif

#if (MROP) == McopyInverted
#define MROP_DECLARE()
#define MROP_DECLARE_REG()
#define MROP_INITIALIZE(alu,pm)
#define MROP_SOLID(src,dst)	(~(src))
#define MROP_SOLID24(src,dst,index)	    {\
	register int idx = ((index) & 3)<< 1; \
	*(dst) = (*(dst) & cfbrmask[idx])|(((~(src))<<cfb24Shift[idx])&cfbmask[idx]); \
	idx++; \
	(dst)++; \
	*(dst) = (*(dst) & cfbrmask[idx])|(((~(src))>>cfb24Shift[idx])&cfbmask[idx]); \
	(dst)--; \
	}
#define MROP_MASK(src,dst,mask)	((dst) & ~(mask) | (~(src)) & (mask))
#define MROP_MASK24(src,dst,mask,index)	{\
	register int idx = ((index) & 3)<< 1; \
	*(dst) = (*(dst) & cfbrmask[idx] &(~(((mask)<< cfb24Shift[idx])&cfbmask[idx])) | \
		((((~(src))&(mask))<<cfb24Shift[idx])&cfbmask[idx])); \
	idx++; \
	(dst)++; \
	*(dst) = (*(dst) & cfbrmask[idx] &(~(((mask)>>cfb24Shift[idx])&cfbmask[idx])) | \
		((((~(src))&(mask))>>cfb24Shift[idx])&cfbmask[idx])); \
	(dst)--; \
	}
#define MROP_NAME(prefix)	MROP_NAME_CAT(prefix,CopyInverted)
#endif

#if (MROP) == Mxor
#define MROP_DECLARE()
#define MROP_DECLARE_REG()
#define MROP_INITIALIZE(alu,pm)
#define MROP_SOLID(src,dst)	((src) ^ (dst))
#define MROP_SOLID24(src,dst,index)	    {\
	register int idx = ((index) & 3)<< 1; \
	*(dst) ^= (((src)<<cfb24Shift[idx])&cfbmask[idx]); \
	idx++; \
	(dst)++; \
	*(dst) ^= (((src)>>cfb24Shift[idx])&cfbmask[idx]); \
	(dst)--; \
	}
#define MROP_MASK(src,dst,mask)	(((src) & (mask)) ^ (dst))
#define MROP_MASK24(src,dst,mask,index)	{\
	register int idx = ((index) & 3)<< 1; \
	*(dst) ^= ((((src)&(mask))<<cfb24Shift[idx])&cfbmask[idx]); \
	idx++; \
	(dst)++; \
	*(dst) ^= ((((src)&(mask))>>cfb24Shift[idx])&cfbmask[idx]); \
	(dst)--; \
	}
#define MROP_NAME(prefix)	MROP_NAME_CAT(prefix,Xor)
#endif

#if (MROP) == Mor
#define MROP_DECLARE()
#define MROP_DECLARE_REG()
#define MROP_INITIALIZE(alu,pm)
#define MROP_SOLID(src,dst)	((src) | (dst))
#define MROP_SOLID24(src,dst,index)	    {\
	register int idx = ((index) & 3)<< 1; \
	*(dst) |= (((src)<<cfb24Shift[idx])&cfbmask[idx]); \
	idx++; \
	(dst)++; \
	*(dst) |= (((src)>>cfb24Shift[idx])&cfbmask[idx]); \
	(dst)--; \
	}
#define MROP_MASK(src,dst,mask)	(((src) & (mask)) | (dst))
#define MROP_MASK24(src,dst,mask,index)	{\
	register int idx = ((index) & 3)<< 1; \
	*(dst) |= ((((src)&(mask))<<cfb24Shift[idx])&cfbmask[idx]); \
	idx++; \
	(dst)++; \
	*(dst) |= ((((src)&(mask))>>cfb24Shift[idx])&cfbmask[idx]); \
	(dst)--; \
	}
#define MROP_NAME(prefix)	MROP_NAME_CAT(prefix,Or)
#endif

#if (MROP) == (Mcopy|Mxor|MandReverse|Mor)
#define MROP_DECLARE()	unsigned long _ca1, _cx1;
#define MROP_DECLARE_REG()	register MROP_DECLARE()
#define MROP_INITIALIZE(alu,pm)	{ \
    mergeRopPtr  _bits; \
    _bits = &mergeRopBits[alu]; \
    _ca1 = _bits->ca1; \
    _cx1 = _bits->cx1; \
}
#define MROP_SOLID(src,dst) \
    ((dst) & ((src) & _ca1 ^ _cx1) ^ (src))
#define MROP_MASK(src,dst,mask)	\
    ((dst) & (((src) & _ca1 ^ _cx1) | ~(mask)) ^ ((src) & (mask)))
#define MROP_NAME(prefix)	MROP_NAME_CAT(prefix,CopyXorAndReverseOr)
#define MROP_PREBUILD(src)	PrebuildMergeRop(src)
#define MROP_PREBUILT_DECLARE()	DeclarePrebuiltMergeRop()
#define MROP_PREBUILT_SOLID(src,dst)	DoPrebuiltMergeRop(dst)
#define MROP_PREBUILT_SOLID24(src,dst,index)	DoPrebuiltMergeRop24(dst,index)
#define MROP_PREBUILT_MASK(src,dst,mask)    DoMaskPrebuiltMergeRop(dst,mask)
#define MROP_PREBUILT_MASK24(src,dst,mask,index)    DoMaskPrebuiltMergeRop24(dst,mask,index)
#endif

#if (MROP) == 0
#define MROP_DECLARE()	DeclareMergeRop()
#define MROP_DECLARE_REG()	register DeclareMergeRop()
#define MROP_INITIALIZE(alu,pm)	InitializeMergeRop(alu,pm)
#define MROP_SOLID(src,dst)	DoMergeRop(src,dst)
#define MROP_SOLID24(src,dst,index)	DoMergeRop24(src,dst,index)
#define MROP_MASK(src,dst,mask)	DoMaskMergeRop(src, dst, mask)
#define MROP_MASK24(src,dst,mask,index)	DoMaskMergeRop24(src, dst, mask,index)
#define MROP_NAME(prefix)	MROP_NAME_CAT(prefix,General)
#define MROP_PREBUILD(src)	PrebuildMergeRop(src)
#define MROP_PREBUILT_DECLARE()	DeclarePrebuiltMergeRop()
#define MROP_PREBUILT_SOLID(src,dst)	DoPrebuiltMergeRop(dst)
#define MROP_PREBUILT_SOLID24(src,dst,index)	DoPrebuiltMergeRop24(dst,index)
#define MROP_PREBUILT_MASK(src,dst,mask)    DoMaskPrebuiltMergeRop(dst,mask)
#define MROP_PREBUILT_MASK24(src,dst,mask,index) \
	DoMaskPrebuiltMergeRop24(dst,mask,index)
#endif

#ifndef MROP_PREBUILD
#define MROP_PREBUILD(src)
#define MROP_PREBUILT_DECLARE()
#define MROP_PREBUILT_SOLID(src,dst)	MROP_SOLID(src,dst)
#define MROP_PREBUILT_SOLID24(src,dst,index)	MROP_SOLID24(src,dst,index)
#define MROP_PREBUILT_MASK(src,dst,mask)    MROP_MASK(src,dst,mask)
#define MROP_PREBUILT_MASK24(src,dst,mask,index) MROP_MASK24(src,dst,mask,index)
#endif

#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define MROP_NAME_CAT(prefix,suffix)	prefix##suffix
#else
#define MROP_NAME_CAT(prefix,suffix)	prefix/**/suffix
#endif

#endif
