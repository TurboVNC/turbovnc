/* $Xorg: curves.h,v 1.3 2000/08/17 19:46:29 cpqbld Exp $ */
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
/* $XFree86: xc/lib/font/Type1/curves.h,v 1.3 1999/08/22 08:58:50 dawes Exp $ */

/*SHARED*/
 
#define   StepConic(R,xA,yA,xB,yB,xC,yC,r)      t1_StepConic(R,xA,yA,xB,yB,xC,yC,r)
#define   StepBezier(R,xA,yA,xB,yB,xC,yC,xD,yD) t1_StepBezier(R,xA,yA,xB,yB,xC,yC,xD,yD)
 
#define   FlattenConic(xM,yM,xC,yC,r)        t1_StepConic(NULL,(fractpel)0,(fractpel)0,xM,yM,xC,yC,r)
#define   FlattenBezier(xB,yB,xC,yC,xD,yD)   t1_StepBezier(NULL,(fractpel)0,(fractpel)0,xB,yB,xC,yC,xD,yD)

#if 0 
struct segment *t1_StepConic();
#endif
extern struct segment *t1_StepBezier ( struct region *R, fractpel xA, fractpel yA, fractpel xB, fractpel yB, fractpel xC, fractpel yC, fractpel xD, fractpel yD );

/*END SHARED*/
