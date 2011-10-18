/* $XConsortium: arith.h,v 1.3 94/03/22 19:08:56 gildea Exp $ */
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
/*SHARED*/

#include <X11/Xmd.h>		/* LONG64 */

void DLmult(),DLdiv(),DLadd(),DLsub();
 
fractpel FPmult();
fractpel FPdiv();
fractpel FPstarslash();
 
/*END SHARED*/
/*SHARED*/
 
#define     SHORTSIZE         (sizeof(short)*8)
#define     LONGSIZE          (SHORTSIZE*2)
#define     MAXSHORT          ((1<<SHORTSIZE)-1)
 
/*END SHARED*/
/*SHARED*/
 
#ifdef LONG64
typedef long doublelong;
#else
typedef struct {
       long high;
       unsigned long low;
} doublelong;
#endif /* LONG64 else */

/*END SHARED*/
/*SHARED*/
 
#ifdef LONG64
#define  DLrightshift(dl,N)  ((dl) >>= (N))
#else
#define  DLrightshift(dl,N)  { \
       dl.low = (dl.low >> N) + (((unsigned long) dl.high) << (LONGSIZE - N)); \
       dl.high >>= N; \
}
#endif

/*END SHARED*/
