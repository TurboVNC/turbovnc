/*
 * $Xorg: stipple68k.s,v 1.4 2001/02/09 02:04:39 xorgcvs Exp $
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
/* $XFree86: xc/programs/Xserver/cfb/stipple68k.s,v 3.2 2001/01/17 22:36:38 dawes Exp $ */

/*
 * Stipple code for 68k processors
 */

#ifdef __ELF__
#ifdef TETEXT
#define _cfbStippleStack cfbStippleStackTE
#else
#define _cfbStippleStack cfbStippleStack
#endif
#else
#ifdef TETEXT
#define _cfbStippleStack _cfbStippleStackTE
#endif
#endif


#define atemp	a0
#define addr	a1
#define stipple	a2
#define stride	a3
#define case	a4

#define ctemp	d0
#define value	d1
#define count	d2
#define shift	d3
#define c	d4
#define lshift	d5
#define rshift	d6

#define PushMask	#0x3e38
#define PopMask		#0x1c7c
#define NumReg	8
#define arg0	36
#define arg1	40
#define arg2	44
#define arg3	48
#define arg4	52
#define arg5	56
#define arg6	60

#ifdef __ELF__
#define ForEachLine	.L2
#define ForEachBits	.L5
#define a0 %A0
#define a1 %A1
#define a2 %A2
#define a3 %A3
#define a4 %A4
#define a5 %A5
#define a6 %A6
#define sp %SP
#define d0 %D0
#define d1 %D1
#define d2 %D2
#define d3 %D3
#define d4 %D4
#define d5 %D5
#define d6 %D6
#define d7 %D7
#else
#define ForEachLine	L2
#define ForEachBits	L5
#endif
#define CASE_SIZE	5

.text
	.even
	.globl _cfbStippleStack
_cfbStippleStack:
	moveml	PushMask,sp@-
	movel	sp@(arg0),addr
	movel	sp@(arg1),stipple
	movel	sp@(arg2),value
	movel	sp@(arg3),stride
	movew	sp@(arg4+2),count
	movel	sp@(arg5),shift
	subqw	#1,count		/* predecrement count */
	lea	CaseBegin,case
	movew	#28,lshift
	addl	shift,lshift
	movew	#28,rshift
	subql	#4,shift
	negl	shift
ForEachLine:
	movel	addr,atemp
	addl	stride,addr
	movel	stipple@+,c
#ifdef TETEXT
	jeq	NextLine
#endif
	/* Get first few bits */
	movel	c,ctemp
	lsrl	lshift,ctemp
	lsll	#CASE_SIZE,ctemp
	lsll	shift,c			/* set up for next bits */
	jmp	case@(ctemp:l)

ForEachBits:
	addl	#4,atemp
	movel	c,ctemp
	lsrl	rshift,ctemp		/* better than lsrl, andi */
	lsll	#CASE_SIZE,ctemp
	lsll	#4,c			/* set up for next bits */
	jmp	case@(ctemp:l)

#define Break				\
	andl	c,c		 	; \
	jne	ForEachBits	 	; \
	dbra	count,ForEachLine	; \
	moveml	sp@+,PopMask		; \
	rts				;

CaseBegin:
	jne	ForEachBits		/* 0 */
NextLine:
	dbra	count,ForEachLine
	moveml	sp@+,PopMask
	rts
	
	. = CaseBegin + 0x20

	moveb	value,atemp@(3)		/* 1 */
	Break

	. = CaseBegin + 0x40

	moveb	value,atemp@(2)		/* 2 */
	Break

	. = CaseBegin + 0x60

	movew	value,atemp@(2)		/* 3 */
	Break

	. = CaseBegin + 0x80

	moveb	value,atemp@(1)		/* 4 */
	Break

	. = CaseBegin + 0xa0

	moveb	value,atemp@(3)		/* 5 */
	moveb	value,atemp@(1)
	Break

	. = CaseBegin + 0xc0

	movew	value,atemp@(1)		/* 6 */
	Break

	. = CaseBegin + 0xe0

	movew	value,atemp@(2)		/* 7 */
	moveb	value,atemp@(1)	
	Break

	. = CaseBegin + 0x100

	moveb	value,atemp@		/* 8 */
	Break

	. = CaseBegin + 0x120

	moveb	value,atemp@(3)		/* 9 */
	moveb	value,atemp@
	Break

	. = CaseBegin + 0x140

	moveb	value,atemp@(2)		/* a */
	moveb	value,atemp@
	Break

	. = CaseBegin + 0x160

	movew	value,atemp@(2)		/* b */
	moveb	value,atemp@
	Break

	. = CaseBegin + 0x180

	movew	value,atemp@		/* c */
	Break

	. = CaseBegin + 0x1a0

	moveb	value,atemp@(3)		/* d */
	movew	value,atemp@
	Break

	. = CaseBegin + 0x1c0

	moveb	value,atemp@(2)		/* e */
	movew	value,atemp@
	Break

	. = CaseBegin + 0x1e0

	movel	value,atemp@		/* f */
	Break
