/* $Xorg: hints.h,v 1.3 2000/08/17 19:46:30 cpqbld Exp $ */
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
/* $XFree86: xc/lib/font/Type1/hints.h,v 1.3 1999/08/22 08:58:51 dawes Exp $ */

/*SHARED*/
 
#define   InitHints   t1_InitHints
extern void t1_InitHints ( void );	/* Initialize hint data structure */
 
#define   CloseHints(hintP)   t1_CloseHints(hintP)
/* Reverse hints that are still open */
extern void t1_CloseHints ( struct fractpoint *hintP );
 
#define   ProcessHint(hP, currX, currY, hintP)   t1_ProcessHint(hP, currX, currY, hintP)
/* Process a rasterization hint */
extern void t1_ProcessHint ( struct hintsegment *hP, fractpel currX, fractpel currY, struct fractpoint *hintP );

#define   ApplyContinuity(R)  t1_ApplyContinuity(R)
/* fix false connection breaks in a region      */
extern void t1_ApplyContinuity ( struct region *R );

/*END SHARED*/
