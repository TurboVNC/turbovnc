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
#ifndef SERVERMD_H
#define SERVERMD_H 1
/* $XConsortium: servermd.h /main/58 1996/12/02 10:22:09 lehors $ */
/* $XFree86: xc/programs/Xserver/include/servermd.h,v 3.19.2.3 1997/07/28 14:17:34 dawes Exp $ */

/*
 * Machine dependent values:
 * GLYPHPADBYTES should be chosen with consideration for the space-time
 * trade-off.  Padding to 0 bytes means that there is no wasted space
 * in the font bitmaps (both on disk and in memory), but that access of
 * the bitmaps will cause odd-address memory references.  Padding to
 * 2 bytes would ensure even address memory references and would
 * be suitable for a 68010-class machine, but at the expense of wasted
 * space in the font bitmaps.  Padding to 4 bytes would be good
 * for real 32 bit machines, etc.  Be sure that you tell the font
 * compiler what kind of padding you want because its defines are
 * kept separate from this.  See server/include/font.h for how
 * GLYPHPADBYTES is used.
 *
 * Along with this, you should choose an appropriate value for
 * GETLEFTBITS_ALIGNMENT, which is used in ddx/mfb/maskbits.h.  This
 * constant choses what kind of memory references are guarenteed during
 * font access; either 1, 2 or 4, for byte, word or longword access,
 * respectively.  For instance, if you have decided to to have
 * GLYPHPADBYTES == 4, then it is pointless for you to have a
 * GETLEFTBITS_ALIGNMENT > 1, because the padding of the fonts has already
 * guarenteed you that your fonts are longword aligned.  On the other
 * hand, even if you have chosen GLYPHPADBYTES == 1 to save space, you may
 * also decide that the computing involved in aligning the pointer is more
 * costly than an odd-address access; you choose GETLEFTBITS_ALIGNMENT == 1.
 *
 * Next, choose the tuning parameters which are appropriate for your
 * hardware; these modify the behaviour of the raw frame buffer code
 * in ddx/mfb and ddx/cfb.  Defining these incorrectly will not cause
 * the server to run incorrectly, but defining these correctly will
 * cause some noticeable speed improvements:
 *
 *  AVOID_MEMORY_READ - (8-bit cfb only)
 *	When stippling pixels on the screen (polytext and pushpixels),
 *	don't read long words from the display and mask in the
 *	appropriate values.  Rather, perform multiple byte/short/long
 *	writes as appropriate.  This option uses many more instructions
 *	but runs much faster when the destination is much slower than
 *	the CPU and at least 1 level of write buffer is availible (2
 *	is much better).  Defined currently for SPARC and MIPS.
 *
 *  FAST_CONSTANT_OFFSET_MODE - (cfb and mfb)
 *	This define is used on machines which have no auto-increment
 *	addressing mode, but do have an effectively free constant-offset
 *	addressing mode.  Currently defined for MIPS and SPARC, even though
 *	I remember the cg6 as performing better without it (cg3 definitely
 *	performs better with it).
 *	
 *  LARGE_INSTRUCTION_CACHE -
 *	This define increases the number of times some loops are
 *	unrolled.  On 68020 machines (with 256 bytes of i-cache),
 *	this define will slow execution down as instructions miss
 *	the cache frequently.  On machines with real i-caches, this
 *	reduces loop overhead, causing a slight performance improvement.
 *	Currently defined for MIPS and SPARC
 *
 *  FAST_UNALIGNED_READS -
 *	For machines with more memory bandwidth than CPU, this
 *	define uses unaligned reads for 8-bit BitBLT instead of doing
 *	aligned reads and combining the results with shifts and
 *	logical-ors.  Currently defined for 68020 and vax.
 *  PLENTIFUL_REGISTERS -
 *	For machines with > 20 registers.  Currently used for
 *	unrolling the text painting code a bit more.  Currently
 *	defined for MIPS.
 *  SHARED_IDCACHE -
 *	For non-Harvard RISC machines, those which share the same
 *	CPU memory bus for instructions and data.  This unrolls some
 *	solid fill loops which are otherwise best left rolled up.
 *	Currently defined for SPARC.
 */

#ifdef vax

#define IMAGE_BYTE_ORDER	LSBFirst        /* Values for the VAX only */
#define BITMAP_BIT_ORDER	LSBFirst
#define	GLYPHPADBYTES		1
#define GETLEFTBITS_ALIGNMENT	4
#define FAST_UNALIGNED_READS

#endif /* vax */

#if (defined(Lynx) && defined(__powerpc__))

/* For now this is for Xvfb only */
#define IMAGE_BYTE_ORDER        MSBFirst
#define BITMAP_BIT_ORDER        MSBFirst
#define GLYPHPADBYTES           4
#define GETLEFTBITS_ALIGNMENT   1

#define LARGE_INSTRUCTION_CACHE
#define FAST_CONSTANT_OFFSET_MODE
#define PLENTIFUL_REGISTERS
#define AVOID_MEMORY_READ

#define FAST_MEMCPY

#endif /* LynxOS PowerPC */

#if (defined(sun) && !(defined(i386) && defined(SVR4))) || \
    (defined(AMOEBA) && (defined(sparc) || defined(mc68000))) || \
    (defined(__uxp__) && (defined(sparc) || defined(mc68000))) || \
    (defined(Lynx) && defined(__sparc__)) || \
    ((defined(__NetBSD__) || defined(__OpenBSD__)) && \
     (defined(__sparc__) || defined(__mc68000__)))

#if defined(sun386) || defined(sun5)
# define IMAGE_BYTE_ORDER	LSBFirst        /* Values for the SUN only */
# define BITMAP_BIT_ORDER	LSBFirst
#else
# define IMAGE_BYTE_ORDER	MSBFirst        /* Values for the SUN only */
# define BITMAP_BIT_ORDER	MSBFirst
#endif

#ifdef sparc
# define AVOID_MEMORY_READ
# define LARGE_INSTRUCTION_CACHE
# define FAST_CONSTANT_OFFSET_MODE
# define SHARED_IDCACHE
#endif

#ifdef mc68020
#define FAST_UNALIGNED_READS
#endif

#define	GLYPHPADBYTES		4
#define GETLEFTBITS_ALIGNMENT	1

#endif /* sun && !(i386 && SVR4) */


#if defined(AIXV3)

#define IMAGE_BYTE_ORDER        MSBFirst        /* Values for the RISC/6000 */
#define BITMAP_BIT_ORDER        MSBFirst
#define GLYPHPADBYTES           4
#define GETLEFTBITS_ALIGNMENT   1

#define LARGE_INSTRUCTION_CACHE
#define FAST_CONSTANT_OFFSET_MODE
#define PLENTIFUL_REGISTERS
#define AVOID_MEMORY_READ

#define FAST_MEMCPY
#endif /* AIXV3 */

#if defined(ibm032) || defined (ibm)

#ifdef i386
# define IMAGE_BYTE_ORDER	LSBFirst	/* Value for PS/2 only */
#else
# define IMAGE_BYTE_ORDER	MSBFirst        /* Values for the RT only*/
#endif
#define BITMAP_BIT_ORDER	MSBFirst
#define	GLYPHPADBYTES		1
#define GETLEFTBITS_ALIGNMENT	4
/* ibm pcc doesn't understand pragmas. */

#ifdef i386
#define BITMAP_SCANLINE_UNIT	8
#endif

#endif /* ibm */

#ifdef hpux

#define IMAGE_BYTE_ORDER	MSBFirst        /* Values for the HP only */
#define BITMAP_BIT_ORDER	MSBFirst
#define	GLYPHPADBYTES		2		/* to match product server */
#define	GETLEFTBITS_ALIGNMENT	4		/* PA forces longs to 4 */
						/* byte boundries */
#define AVOID_MEMORY_READ
#define FAST_CONSTANT_OFFSET_MODE
#define LARGE_INSTRUCTION_CACHE
#define PLENTIFUL_REGISTERS

#endif /* hpux */

#if defined (M4310) || defined(M4315) || defined(M4317) || defined(M4319) || defined(M4330)

#define IMAGE_BYTE_ORDER	MSBFirst        /* Values for Pegasus only */
#define BITMAP_BIT_ORDER	MSBFirst
#define GLYPHPADBYTES		4
#define GETLEFTBITS_ALIGNMENT	1

#define FAST_UNALIGNED_READS

#endif /* tektronix */

#ifdef macII

#define IMAGE_BYTE_ORDER      	MSBFirst        /* Values for the MacII only */
#define BITMAP_BIT_ORDER      	MSBFirst
#define GLYPHPADBYTES         	4
#define GETLEFTBITS_ALIGNMENT 	1

/* might want FAST_UNALIGNED_READS for frame buffers with < 1us latency */

#endif /* macII */

#if (defined(mips) || defined(__mips)) && !defined(sgi)

#if defined(MIPSEL) || defined(__MIPSEL__)
# define IMAGE_BYTE_ORDER	LSBFirst        /* Values for the PMAX only */
# define BITMAP_BIT_ORDER	LSBFirst
# define GLYPHPADBYTES		4
# define GETLEFTBITS_ALIGNMENT	1
#else
# define IMAGE_BYTE_ORDER	MSBFirst        /* Values for the MIPS only */
# define BITMAP_BIT_ORDER	MSBFirst
# define GLYPHPADBYTES		4
# define GETLEFTBITS_ALIGNMENT	1
#endif

#define AVOID_MEMORY_READ
#define FAST_CONSTANT_OFFSET_MODE
#define LARGE_INSTRUCTION_CACHE
#define PLENTIFUL_REGISTERS

#endif /* mips */

#if defined(__alpha) || defined(__alpha__) || defined(__alphaCross)
# define IMAGE_BYTE_ORDER	LSBFirst	/* Values for the Alpha only */

# if defined(XF86MONOVGA) || defined(XF86VGA16) || defined(XF86MONO)
#  define BITMAP_BIT_ORDER      MSBFirst
# else
#  define BITMAP_BIT_ORDER      LSBFirst
# endif

# if defined(XF86MONOVGA) || defined(XF86VGA16)
#  define BITMAP_SCANLINE_UNIT  8
# else
   /* pad scanline to a longword */
#  define BITMAP_SCANLINE_UNIT			64
# endif

# define BITMAP_SCANLINE_PAD 			64
# define LOG2_BITMAP_PAD			6
# define LOG2_BYTES_PER_SCANLINE_PAD		3
# define GLYPHPADBYTES		4
# define GETLEFTBITS_ALIGNMENT	1
# define FAST_CONSTANT_OFFSET_MODE
# define LARGE_INSTRUCTION_CACHE
# define PLENTIFUL_REGISTERS

/* Add for handling protocol XPutImage and XGetImage; see comment below */
#define INTERNAL_VS_EXTERNAL_PADDING
#define BITMAP_SCANLINE_UNIT_PROTO		32

#define BITMAP_SCANLINE_PAD_PROTO 	 	32
#define LOG2_BITMAP_PAD_PROTO			5
#define LOG2_BYTES_PER_SCANLINE_PAD_PROTO	2

#endif /* alpha */

#ifdef stellar

#define IMAGE_BYTE_ORDER	MSBFirst       /* Values for the stellar only*/
#define BITMAP_BIT_ORDER	MSBFirst
#define	GLYPHPADBYTES		4
#define GETLEFTBITS_ALIGNMENT	4
#define IMAGE_BUFSIZE		(64*1024)
/*
 * Use SysV random number generator.
 */
#define random rand

#endif /* stellar */

#ifdef luna

#define IMAGE_BYTE_ORDER        MSBFirst   	/* Values for the OMRON only*/
#define BITMAP_BIT_ORDER	MSBFirst
#define	GLYPHPADBYTES		4
#define GETLEFTBITS_ALIGNMENT	1

#ifndef mc68000
#define FAST_CONSTANT_OFFSET_MODE
#define AVOID_MEMORY_READ
#define LARGE_INSTRUCTION_CACHE
#define PLENTIFUL_REGISTERS
#endif

#endif /* luna */

#if ((defined(__s390__) || defined(__s390x__)) && defined(linux))
#define IMAGE_BYTE_ORDER   MSBFirst
#define BITMAP_BIT_ORDER   MSBFirst
#endif /* (__s390__ || __s390x__) && linux */
    
#if (defined(i386) && (defined(SVR4) || defined(SYSV) || (defined(sun) && defined(SVR4))) || defined(__bsdi__) || (defined(__NetBSD__) && defined(__i386__)) || (defined(__OpenBSD__) && defined(__i386__)) || defined(__FreeBSD__) || defined(MACH386) || (defined(linux) && !defined(__mc68000__)) || (defined(AMOEBA) && defined(i80386)) || defined(MINIX) || defined(__EMX__) || (defined(Lynx) && defined(__i386__)))

#ifndef IMAGE_BYTE_ORDER
#define IMAGE_BYTE_ORDER	LSBFirst
#endif

#ifndef BITMAP_BIT_ORDER
# if defined(XF86MONOVGA) || defined(XF86VGA16) || defined(XF86MONO)
#  define BITMAP_BIT_ORDER      MSBFirst
# else
#  define BITMAP_BIT_ORDER      LSBFirst
# endif
#endif

#ifndef BITMAP_SCANLINE_UNIT
# if defined(XF86MONOVGA) || defined(XF86VGA16)
#  define BITMAP_SCANLINE_UNIT  8
# endif
#endif

#ifndef GLYPHPADBYTES
#define GLYPHPADBYTES           4
#endif

#define GETLEFTBITS_ALIGNMENT	1
#define AVOID_MEMORY_READ
#ifdef XSVGA
#define AVOID_GLYPHBLT
#define FAST_CONSTANT_OFFSET_MODE
#define FAST_MEMCPY
#define NO_ONE_RECT
#endif

/* Values for AMD Opteron and Intel 64 bit extensions. Copied from Alpha.
 */
#ifdef __x86_64__
#  define BITMAP_SCANLINE_UNIT			64
# define BITMAP_SCANLINE_PAD 			64
# define LOG2_BITMAP_PAD			6
# define LOG2_BYTES_PER_SCANLINE_PAD		3

/* Add for handling protocol XPutImage and XGetImage; see comment in
 * Alpha section.
 */
#define INTERNAL_VS_EXTERNAL_PADDING
#define BITMAP_SCANLINE_UNIT_PROTO		32

#define BITMAP_SCANLINE_PAD_PROTO 	 	32
#define LOG2_BITMAP_PAD_PROTO			5
#define LOG2_BYTES_PER_SCANLINE_PAD_PROTO	2
#endif

#endif /* SVR4 / BSD / i386 */

#if defined (linux) && defined (__mc68000__)

#define IMAGE_BYTE_ORDER       MSBFirst
#define BITMAP_BIT_ORDER       MSBFirst
#define FAST_UNALIGNED_READS
#define GLYPHPADBYTES          4
#define GETLEFTBITS_ALIGNMENT  1

#endif /* linux/m68k */

#if defined (linux) && defined(__powerpc__)

#define IMAGE_BYTE_ORDER       MSBFirst
#define BITMAP_BIT_ORDER       MSBFirst
#define GLYPHPADBYTES          4
#define GETLEFTBITS_ALIGNMENT  1

#define LARGE_INSTRUCTION_CACHE  
#define FAST_CONSTANT_OFFSET_MODE
#define PLENTIFUL_REGISTERS
#define AVOID_MEMORY_READ

#define FAST_MEMCPY

#endif /* Linux/PPC */

#if defined(__MACH__) && defined(__POWERPC__)

#define IMAGE_BYTE_ORDER       MSBFirst
#define BITMAP_BIT_ORDER       MSBFirst
#define GLYPHPADBYTES          4
#define GETLEFTBITS_ALIGNMENT  1

#define LARGE_INSTRUCTION_CACHE  
#define FAST_CONSTANT_OFFSET_MODE
#define PLENTIFUL_REGISTERS
#define AVOID_MEMORY_READ

#define FAST_MEMCPY

#endif /* MACH/PPC */

#ifdef sgi

#define IMAGE_BYTE_ORDER	MSBFirst
#define BITMAP_BIT_ORDER	MSBFirst

#if (_MIPS_SZLONG == 64)

# define GLYPHPADBYTES		4
# define GETLEFTBITS_ALIGNMENT	1

/* pad scanline to a longword */
#define BITMAP_SCANLINE_UNIT			64

#define BITMAP_SCANLINE_PAD 			64
#define LOG2_BITMAP_PAD				6
#define LOG2_BYTES_PER_SCANLINE_PAD		3

/* Add for handling protocol XPutImage and XGetImage; see comment below */
#define INTERNAL_VS_EXTERNAL_PADDING
#define BITMAP_SCANLINE_UNIT_PROTO		32

#define BITMAP_SCANLINE_PAD_PROTO 	 	32
#define LOG2_BITMAP_PAD_PROTO			5
#define LOG2_BYTES_PER_SCANLINE_PAD_PROTO	2

#else

#define GLYPHPADBYTES		2
#define GETLEFTBITS_ALIGNMENT	4

#endif

#define AVOID_MEMORY_READ
#define FAST_CONSTANT_OFFSET_MODE
#define LARGE_INSTRUCTION_CACHE
#define PLENTIFUL_REGISTERS

#endif

/* size of buffer to use with GetImage, measured in bytes. There's obviously
 * a trade-off between the amount of stack (or whatever ALLOCATE_LOCAL gives
 * you) used and the number of times the ddx routine has to be called.
 * 
 * for a 1024 x 864 bit monochrome screen  with a 32 bit word we get 
 * 8192/4 words per buffer 
 * (1024/32) = 32 words per scanline
 * 2048 words per buffer / 32 words per scanline = 64 scanlines per buffer
 * 864 scanlines / 64 scanlines = 14 buffers to draw a full screen
 */
#ifndef IMAGE_BUFSIZE
#define IMAGE_BUFSIZE		8192
#endif

/* pad scanline to a longword */
#ifndef BITMAP_SCANLINE_UNIT
#define BITMAP_SCANLINE_UNIT	32
#endif

#ifndef BITMAP_SCANLINE_PAD
#define BITMAP_SCANLINE_PAD  32
#define LOG2_BITMAP_PAD		5
#define LOG2_BYTES_PER_SCANLINE_PAD	2
#endif

/* 
 *   This returns the number of padding units, for depth d and width w.
 * For bitmaps this can be calculated with the macros above.
 * Other depths require either grovelling over the formats field of the
 * screenInfo or hardwired constants.
 */

typedef struct _PaddingInfo {
	int     padRoundUp;	/* pixels per pad unit - 1 */
	int	padPixelsLog2;	/* log 2 (pixels per pad unit) */
	int     padBytesLog2;	/* log 2 (bytes per pad unit) */
	int	notPower2;	/* bitsPerPixel not a power of 2 */
	int	bytesPerPixel;	/* only set when notPower2 is TRUE */
} PaddingInfo;
extern PaddingInfo PixmapWidthPaddingInfo[];

#define PixmapWidthInPadUnits(w, d) \
    (PixmapWidthPaddingInfo[d].notPower2 ? \
    (((int)(w) * PixmapWidthPaddingInfo[d].bytesPerPixel +  \
	         PixmapWidthPaddingInfo[d].bytesPerPixel) >> \
	PixmapWidthPaddingInfo[d].padBytesLog2) : \
    ((int)((w) + PixmapWidthPaddingInfo[d].padRoundUp) >> \
	PixmapWidthPaddingInfo[d].padPixelsLog2))

/*
 *	Return the number of bytes to which a scanline of the given
 * depth and width will be padded.
 */
#define PixmapBytePad(w, d) \
    (PixmapWidthInPadUnits(w, d) << PixmapWidthPaddingInfo[d].padBytesLog2)

#define BitmapBytePad(w) \
    (((int)((w) + BITMAP_SCANLINE_PAD - 1) >> LOG2_BITMAP_PAD) << LOG2_BYTES_PER_SCANLINE_PAD)

#ifdef INTERNAL_VS_EXTERNAL_PADDING

/*  This is defined if the server's internal padding is different from the padding
 *  advertised in the protocol.  The protocol does not allow for padding to
 *  64 bits, for example, so if the server wants to use 64 bit padding internally,
 *  it has to advertise 32 bit padding and do padding fixups whenever images
 *  cross the wire.  (See ProcGetImage and ProcPutImage.)
 *
 *  The macros and constants that end in Proto or PROTO refer to the advertised
 *  padding, and the ones without Proto are for internal padding.
 */

extern PaddingInfo PixmapWidthPaddingInfoProto[];

#define PixmapWidthInPadUnitsProto(w, d) \
    (PixmapWidthPaddingInfoProto[d].notPower2 ? \
    (((int)(w) * PixmapWidthPaddingInfoProto[d].bytesPerPixel +  \
	         PixmapWidthPaddingInfoProto[d].bytesPerPixel) >> \
	PixmapWidthPaddingInfoProto[d].padBytesLog2) : \
    ((int)((w) + PixmapWidthPaddingInfoProto[d].padRoundUp) >> \
	PixmapWidthPaddingInfoProto[d].padPixelsLog2))

#define PixmapBytePadProto(w, d) \
    (PixmapWidthInPadUnitsProto(w, d) << \
    PixmapWidthPaddingInfoProto[d].padBytesLog2)

#define BitmapBytePadProto(w) \
    ((((w) + BITMAP_SCANLINE_PAD_PROTO - 1) >> LOG2_BITMAP_PAD_PROTO) \
    << LOG2_BYTES_PER_SCANLINE_PAD_PROTO)

#else /* protocol and internal padding is the same */

#define PixmapWidthInPadUnitsProto(w, d) PixmapWidthInPadUnits(w, d)
#define PixmapBytePadProto(w, d) PixmapBytePad(w, d)
#define BitmapBytePadProto(w) BitmapBytePad(w)

#endif /* protocol vs. internal padding  */

#endif /* SERVERMD_H */
