/*
 * $XConsortium: stipsparc.s,v 1.9 94/04/17 20:29:09 rws Exp $
 *
Copyright (c) 1990  X Consortium

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

/*
 * SPARC assembly code for optimized text rendering.
 *
 * Other stippling could be done in assembly, but the payoff is
 * not nearly as large.  Mostly because large areas are heavily
 * optimized already.
 */

/* not that I expect to ever see an LSB SPARC, but ... */
#ifdef LITTLE_ENDIAN
# define BitsR		sll
# define BitsL		srl
# define BO(o)		o
# define HO(o)		o
# define WO(o)		o
# define FourBits(dest,bits)	and	bits, 0xf, dest
#else
# define BitsR		srl
# define BitsL		sll
# define BO(o)		3-o
# define HO(o)		2-o
# define WO(o)		o
# define FourBits(dest,bits)	srl	bits, 28, dest
#endif

/*
 * cfbStippleStack(addr, stipple, value, stride, Count, Shift)
 *               4       5       6      7     16(sp) 20(sp)
 *
 *  Apply successive 32-bit stipples starting at addr, addr+stride, ...
 *
 *  Used for text rendering, but only when no data could be lost
 *  when the stipple is shifted left by Shift bits
 */
/* arguments */
#define addr	%i0
#define stipple	%i1
#define value	%i2
#define stride	%i3
#define count	%i4
#define shift	%i5

/* local variables */
#define atemp	%l0
#define bits	%l1
#define lshift	%l2
#define sbase	%l3
#define stemp	%l4

#define CASE_SIZE	5	/* case blocks are 2^5 bytes each */
#define CASE_MASK	0x1e0	/* first case mask */

#define ForEachLine	LY1
#define NextLine	LY2
#define CaseBegin	LY3
#define ForEachBits	LY4
#define NextBits	LY5

#if defined(SVR4) || ( defined(linux) && defined(__ELF__) )
#ifdef TETEXT
#define	_cfbStippleStack	cfbStippleStackTE
#else
#define	_cfbStippleStack	cfbStippleStack
#endif
#else
#ifdef TETEXT
#define	_cfbStippleStack	_cfbStippleStackTE
#endif
#endif
	.seg	"text"
	.proc	16
	.globl	_cfbStippleStack
_cfbStippleStack:
	save	%sp,-64,%sp
	sethi	%hi(CaseBegin),sbase		/* load up switch table */
	or	sbase,%lo(CaseBegin),sbase

	mov	4,lshift			/* compute offset within */
	sub	lshift, shift, lshift		/*  stipple of remaining bits */
#ifdef LITTLE_ENDIAN
	inc	CASE_SIZE, shift		/* first shift for LSB */
#else
	inc	28-CASE_SIZE, shift		/* first shift for MSB */
#endif
	/* do ... while (--count > 0); */
ForEachLine:
	ld	[stipple],bits			/* get stipple bits */
	mov	addr,atemp			/* set up for this line */
#ifdef TETEXT
	/* Terminal emulator fonts are expanded and have many 0 rows */
	tst	bits
	bz	NextLine			/* skip out early on 0 */
#endif
	add	addr, stride, addr		/* step for the loop */
	BitsR	bits, shift, stemp		/* get first bits */
	and	stemp, CASE_MASK, stemp		/* compute first jump */
	BitsL	bits, lshift, bits		/* set remaining bits */
	jmp	sbase+stemp			/*  ... */
	tst	bits

ForEachBits:
	inc	4, atemp
ForEachBits1:
	FourBits(stemp, bits)			/* compute jump for */
	sll	stemp, CASE_SIZE, stemp		/*  these four bits */
	BitsL	bits, 4, bits			/* step for remaining bits */
	jmp	sbase+stemp			/* jump */
	tst	bits
CaseBegin:
	bnz,a	ForEachBits1			/* 0 */
	inc	4, atemp
NextLine:
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
	nop

	bnz	ForEachBits			/* 1 */
	stb	value, [atemp+BO(0)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
	nop
					
	bnz	ForEachBits			/* 2 */
	stb	value, [atemp+BO(1)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
	nop
					
	bnz	ForEachBits			/* 3 */
	sth	value, [atemp+HO(0)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
	nop
					
	bnz	ForEachBits			/* 4 */
	stb	value, [atemp+BO(2)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
	nop
					
	stb	value, [atemp+BO(0)]		/* 5 */
	bnz	ForEachBits
	stb	value, [atemp+BO(2)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	stb	value, [atemp+BO(1)]		/* 6 */
	bnz	ForEachBits
	stb	value, [atemp+BO(2)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	sth	value, [atemp+HO(0)]		/* 7 */
	bnz	ForEachBits
	stb	value, [atemp+BO(2)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	bnz	ForEachBits			/* 8 */
	stb	value, [atemp+BO(3)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
	nop
					
	stb	value, [atemp+BO(0)]		/* 9 */
	bnz	ForEachBits
	stb	value, [atemp+BO(3)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	stb	value, [atemp+BO(1)]		/* a */
	bnz	ForEachBits
	stb	value, [atemp+BO(3)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	sth	value, [atemp+HO(0)]		/* b */
	bnz	ForEachBits
	stb	value, [atemp+BO(3)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	bnz	ForEachBits			/* c */
	sth	value, [atemp+HO(2)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
	nop
					
	stb	value, [atemp+BO(0)]		/* d */
	bnz	ForEachBits
	sth	value, [atemp+HO(2)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	stb	value, [atemp+BO(1)]		/* e */
	bnz	ForEachBits
	sth	value, [atemp+HO(2)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	bnz	ForEachBits			/* f */
	st	value, [atemp+WO(0)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
