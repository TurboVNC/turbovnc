/* $Xorg: arith.c,v 1.3 2000/08/17 19:46:29 cpqbld Exp $ */
/* Copyright International Business Machines, Corp. 1991
 * All Rights Reserved
 * Copyright Lexmark International, Inc. 1991
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM or Lexmark not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM AND LEXMARK PROVIDE THIS SOFTWARE "AS IS", WITHOUT ANY WARRANTIES OF
 * ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.  THE ENTIRE RISK AS TO THE
 * QUALITY AND PERFORMANCE OF THE SOFTWARE, INCLUDING ANY DUTY TO SUPPORT
 * OR MAINTAIN, BELONGS TO THE LICENSEE.  SHOULD ANY PORTION OF THE
 * SOFTWARE PROVE DEFECTIVE, THE LICENSEE (NOT IBM OR LEXMARK) ASSUMES THE
 * ENTIRE COST OF ALL SERVICING, REPAIR AND CORRECTION.  IN NO EVENT SHALL
 * IBM OR LEXMARK BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */
/* $XFree86: xc/lib/font/Type1/arith.c,v 1.7 2003/05/27 22:26:45 tsi Exp $ */

 /* ARITH    CWEB         V0006 ********                             */
/*
:h1.ARITH Module - Portable Module for Multiple Precision Fixed Point Arithmetic
 
This module provides division and multiplication of 64-bit fixed point
numbers.  (To be more precise, the module works on numbers that take
two 'longs' to store.  That is almost always equivalent to saying 64-bit
numbers.)
 
Note: it is frequently easy and desirable to recode these functions in
assembly language for the particular processor being used, because
assembly language, unlike C, will have 64-bit multiply products and
64-bit dividends.  This module is offered as a portable version.
 
&author. Jeffrey B. Lotspiech (lotspiech@almaden.ibm.com) and Sten F. Andler
 
 
:h3.Include Files
 
The included files are:
*/
 
#include "os.h"
#include "objects.h"
#include "spaces.h"
#include "arith.h"

 
/*
:h3.
*/
/*SHARED LINE(S) ORIGINATED HERE*/
/*
Reference for all algorithms:  Donald E. Knuth, "The Art of Computer
Programming, Volume 2, Semi-Numerical Algorithms," Addison-Wesley Co.,
Massachusetts, 1969, pp. 229-279.
 
Knuth talks about a 'digit' being an arbitrary sized unit and a number
being a sequence of digits.  We'll take a digit to be a 'short'.
The following assumption must be valid for these algorithms to work:
:ol.
:li.A 'long' is two 'short's.
:eol.
The following code is INDEPENDENT of:
:ol.
:li.The actual size of a short.
:li.Whether shorts and longs are stored most significant byte
first or least significant byte first.
:eol.
 
SHORTSIZE is the number of bits in a short; LONGSIZE is the number of
bits in a long; MAXSHORT is the maximum unsigned short:
*/
/*SHARED LINE(S) ORIGINATED HERE*/
/*
ASSEMBLE concatenates two shorts to form a long:
*/
#define     ASSEMBLE(hi,lo)   ((((unsigned long)hi)<<SHORTSIZE)+(lo))
/*
HIGHDIGIT extracts the most significant short from a long; LOWDIGIT
extracts the least significant short from a long:
*/
#define     HIGHDIGIT(u)      ((u)>>SHORTSIZE)
#define     LOWDIGIT(u)       ((u)&MAXSHORT)
 
/*
SIGNBITON tests the high order bit of a long 'w':
*/
#define    SIGNBITON(w)   (((long)w)<0)
 
/*SHARED LINE(S) ORIGINATED HERE*/
 
/*
:h2.Double Long Arithmetic
 
:h3.DLmult() - Multiply Two Longs to Yield a Double Long
 
The two multiplicands must be positive.
*/
 
void 
DLmult(doublelong *product, unsigned long u, unsigned long v)
{
#ifdef LONG64
/* printf("DLmult(? ?, %lx, %lx)\n", u, v); */
    *product = u*v;
/* printf("DLmult returns %lx\n", *product); */
#else
  register unsigned long u1, u2; /* the digits of u */
  register unsigned long v1, v2; /* the digits of v */
  register unsigned int w1, w2, w3, w4; /* the digits of w */
  register unsigned long t; /* temporary variable */
/* printf("DLmult(? ?, %x, %x)\n", u, v); */
  u1 = HIGHDIGIT(u);
  u2 = LOWDIGIT(u);
  v1 = HIGHDIGIT(v);
  v2 = LOWDIGIT(v);
 
  if (v2 == 0) w4 = w3 = w2 = 0;
  else
    {
    t = u2 * v2;
    w4 = LOWDIGIT(t);
    t = u1 * v2 + HIGHDIGIT(t);
    w3 = LOWDIGIT(t);
    w2 = HIGHDIGIT(t);
    }
 
  if (v1 == 0) w1 = 0;
  else
    {
    t = u2 * v1 + w3;
    w3 = LOWDIGIT(t);
    t = u1 * v1 + w2 + HIGHDIGIT(t);
    w2 = LOWDIGIT(t);
    w1 = HIGHDIGIT(t);
    }
 
  product->high = ASSEMBLE(w1, w2);
  product->low  = ASSEMBLE(w3, w4);
#endif /* LONG64 else */
}
 
/*
:h2.DLdiv() - Divide Two Longs by One Long, Yielding Two Longs
 
Both the dividend and the divisor must be positive.
*/
 
void 
DLdiv(doublelong *quotient,  /* also where dividend is, originally     */
      unsigned long divisor)
{
#ifdef LONG64
/* printf("DLdiv(%lx %lx)\n", quotient, divisor); */
	*quotient /= divisor;
/* printf("DLdiv returns %lx\n", *quotient); */
#else
       register unsigned long u1u2 = quotient->high;
       register unsigned long u3u4 = quotient->low;
       register long u3;     /* single digit of dividend                     */
       register int v1,v2;   /* divisor in registers                         */
       register long t;      /* signed copy of u1u2                          */
       register int qhat;    /* guess at the quotient digit                  */
       register unsigned long q3q4;  /* low two digits of quotient           */
       register int shift;   /* holds the shift value for normalizing        */
       register int j;       /* loop variable                                */
 
/* printf("DLdiv(%x %x, %x)\n", quotient->high, quotient->low, divisor); */
       /*
       * Knuth's algorithm works if the dividend is smaller than the
       * divisor.  We can get to that state quickly:
       */
       if (u1u2 >= divisor) {
               quotient->high = u1u2 / divisor;
               u1u2 %= divisor;
       }
       else
               quotient->high = 0;
 
       if (divisor <= MAXSHORT) {
 
               /*
               * This is the case where the divisor is contained in one
               * 'short'.  It is worthwhile making this fast:
               */
               u1u2 = ASSEMBLE(u1u2, HIGHDIGIT(u3u4));
               q3q4 = u1u2 / divisor;
               u1u2 %= divisor;
               u1u2 = ASSEMBLE(u1u2, LOWDIGIT(u3u4));
               quotient->low = ASSEMBLE(q3q4, u1u2 / divisor);
               return;
       }
 
 
       /*
       * At this point the divisor is a true 'long' so we must use
       * Knuth's algorithm.
       *
       * Step D1: Normalize divisor and dividend (this makes our 'qhat'
       *        guesses more accurate):
       */
       for (shift=0; !SIGNBITON(divisor); shift++, divisor <<= 1) { ; }
       shift--;
       divisor >>= 1;
 
       if ((u1u2 >> (LONGSIZE - shift)) != 0 && shift != 0)
               Abort("DLdiv:  dividend too large");
       u1u2 = (u1u2 << shift) + ((shift == 0) ? 0 : u3u4 >> (LONGSIZE - shift));
       u3u4 <<= shift;
 
       /*
       * Step D2:  Begin Loop through digits, dividing u1,u2,u3 by v1,v2,
       *           then shifting U left by 1 digit:
       */
       v1 = HIGHDIGIT(divisor);
       v2 = LOWDIGIT(divisor);
       q3q4 = 0;
       u3 = HIGHDIGIT(u3u4);
 
       for (j=0; j < 2; j++) {
 
               /*
               * Step D3:  make a guess (qhat) at the next quotient denominator:
               */
               qhat = (HIGHDIGIT(u1u2) == v1) ? MAXSHORT : u1u2 / v1;
               /*
               * At this point Knuth would have us further refine our
               * guess, since we know qhat is too big if
               *
               *      v2 * qhat > ASSEMBLE(u1u2 % v, u3)
               *
               * That would make sense if u1u2 % v was easy to find, as it
               * would be in assembly language.  I ignore this step, and
               * repeat step D6 if qhat is too big.
               */
 
               /*
               * Step D4: Multiply v1,v2 times qhat and subtract it from
               * u1,u2,u3:
               */
               u3 -= qhat * v2;
               /*
               * The high digit of u3 now contains the "borrow" for the
               * rest of the substraction from u1,u2.
               * Sometimes we can lose the sign bit with the above.
               * If so, we have to force the high digit negative:
               */
               t = HIGHDIGIT(u3);
               if (t > 0)
                       t |= -1 << SHORTSIZE;
               t += u1u2 - qhat * v1;
/* printf("..>divide step qhat=%x t=%x u3=%x u1u2=%x v1=%x v2=%x\n",
                             qhat, t, u3, u1u2, v1, v2); */
               while (t < 0) {  /* Test is Step D5.                          */
 
                       /*
                       * D6: Oops, qhat was too big.  Add back in v1,v2 and
                       * decrease qhat by 1:
                       */
                       u3 = LOWDIGIT(u3) + v2;
                       t += HIGHDIGIT(u3) + v1;
                       qhat--;
/* printf("..>>qhat correction t=%x u3=%x qhat=%x\n", t, u3, qhat); */
               }
               /*
               * Step D7:  shift U left one digit and loop:
               */
               u1u2 = t;
               if (HIGHDIGIT(u1u2) != 0)
                       Abort("divide algorithm error");
               u1u2 = ASSEMBLE(u1u2, LOWDIGIT(u3));
               u3 = LOWDIGIT(u3u4);
               q3q4 = ASSEMBLE(q3q4, qhat);
       }
       quotient->low = q3q4;
/* printf("DLdiv returns %x %x\n", quotient->high, quotient->low); */
#endif /* !LONG64 */
       return;
}
 
/*
:h3.DLadd() - Add Two Double Longs
 
In this case, the doublelongs may be signed.  The algorithm takes the
piecewise sum of the high and low longs, with the possibility that the
high should be incremented if there is a carry out of the low.  How to
tell if there is a carry?  Alex Harbury suggested that if the sum of
the lows is less than the max of the lows, there must have been a
carry.  Conversely, if there was a carry, the sum of the lows must be
less than the max of the lows.  So, the test is "if and only if".
*/
 
void 
DLadd(doublelong *u, /* u = u + v                                    */
      doublelong *v)
{
#ifdef LONG64
/* printf("DLadd(%lx %lx)\n", *u, *v); */
       *u = *u + *v;
/* printf("DLadd returns %lx\n", *u); */
#else
       register unsigned long lowmax = MAX(u->low, v->low);
 
/* printf("DLadd(%x %x, %x %x)\n", u->high, u->low, v->high, v->low); */
       u->high += v->high;
       u->low += v->low;
       if (lowmax > u->low)
               u->high++;
#endif
}
/*
:h3.DLsub() - Subtract Two Double Longs
 
Testing for a borrow is even easier.  If the v.low is greater than
u.low, there must be a borrow.
*/
 
void 
DLsub(doublelong *u, /* u = u - v                                    */
      doublelong *v)
{
#ifdef LONG64
/* printf("DLsub(%lx %lx)\n", *u, *v); */
       *u = *u - *v;
/* printf("DLsub returns %lx\n", *u); */
#else
/* printf("DLsub(%x %x, %x %x)\n", u->high, u->low, v->high, v->low);*/
       u->high -= v->high;
       if (v->low > u->low)
               u->high--;
       u->low -= v->low;
#endif
}
/*
:h3.DLrightshift() - Macro to Shift Double Long Right by N
*/
 
/*SHARED LINE(S) ORIGINATED HERE*/
 
/*
:h2.Fractional Pel Arithmetic
*/
/*
:h3.FPmult() - Multiply Two Fractional Pel Values
 
This funtion first calculates w = u * v to "doublelong" precision.
It then shifts w right by FRACTBITS bits, and checks that no
overflow will occur when the resulting value is passed back as
a fractpel.
*/
 
fractpel 
FPmult(fractpel u, fractpel v)
{
  doublelong w;
  register int negative = FALSE; /* sign flag */
#ifdef LONG64
  register fractpel ret;
#endif
 
  if ((u == 0) || (v == 0)) return (0);
 
 
  if (u < 0) {u = -u; negative = TRUE;}
  if (v < 0) {v = -v; negative = !negative;}
 
  if (u == TOFRACTPEL(1)) return ((negative) ? -v : v);
  if (v == TOFRACTPEL(1)) return ((negative) ? -u : u);
 
  DLmult(&w, u, v);
  DLrightshift(w, FRACTBITS);
#ifndef LONG64
  if (w.high != 0 || SIGNBITON(w.low)) {
        w.low = TOFRACTPEL(MAXSHORT);
  }
 
  return ((negative) ? -w.low : w.low);
#else
  if (w & 0xffffffff80000000L ) {
        ret = TOFRACTPEL(MAXSHORT);
  }
  else
        ret = (fractpel)w;
 
  return ((negative) ? -ret : ret);
#endif
}
 
/*
:h3.FPdiv() - Divide Two Fractional Pel Values
 
These values may be signed.  The function returns the quotient.
*/
 
fractpel 
FPdiv(fractpel dividend, fractpel divisor)
{
       doublelong w;         /* result will be built here                    */
       int negative = FALSE; /* flag for sign bit                            */
#ifdef LONG64
       register fractpel ret;
#endif
 
       if (dividend < 0) {
               dividend = -dividend;
               negative = TRUE;
       }
       if (divisor < 0) {
               divisor = -divisor;
               negative = !negative;
       }
#ifndef LONG64
       w.low = dividend << FRACTBITS;
       w.high = dividend >> (LONGSIZE - FRACTBITS);
       DLdiv(&w, divisor);
       if (w.high != 0 || SIGNBITON(w.low)) {
               w.low = TOFRACTPEL(MAXSHORT);
       }
       return( (negative) ? -w.low : w.low);
#else
       w = ((long)dividend) << FRACTBITS;
       DLdiv(&w, divisor);
       if (w & 0xffffffff80000000L ) {
               ret = TOFRACTPEL(MAXSHORT);
       }
       else
               ret = (fractpel)w;
       return( (negative) ? -ret : ret);
#endif
}
 
/*
:h3.FPstarslash() - Multiply then Divide
 
Borrowing a chapter from the language Forth, it is useful to define
an operator that first multiplies by one constant then divides by
another, keeping the intermediate result in extended precision.
*/
 
fractpel 
FPstarslash(fractpel a,   /* result = a * b / c                              */
	    fractpel b, 
	    fractpel c)
{
       doublelong w;         /* result will be built here                    */
       int negative = FALSE;
#ifdef LONG64
       register fractpel ret;
#endif
 
       if (a < 0) { a = -a; negative = TRUE; }
       if (b < 0) { b = -b; negative = !negative; }
       if (c < 0) { c = -c; negative = !negative; }
 
       DLmult(&w, a, b);
       DLdiv(&w, c);
#ifndef LONG64
       if (w.high != 0 || SIGNBITON(w.low)) {
               w.low = TOFRACTPEL(MAXSHORT);
       }
       return((negative) ? -w.low : w.low);
#else
       if (w & 0xffffffff80000000L ) {
               ret = TOFRACTPEL(MAXSHORT);
       }
       else
               ret = (fractpel)w;
       return( (negative) ? -ret : ret);
#endif
}
