/* $XConsortium: t1imager.h,v 1.5 92/03/20 14:36:05 keith Exp $ */
/* Copyright International Business Machines,Corp. 1991
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is
 * hereby granted, provided that the above copyright notice
 * appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation,
 * and that the name of IBM not be used in advertising or
 * publicity pertaining to distribution of the software without
 * specific, written prior permission.
 *
 * IBM PROVIDES THIS SOFTWARE "AS IS", WITHOUT ANY WARRANTIES
 * OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT
 * LIMITED TO ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS.  THE ENTIRE RISK AS TO THE QUALITY AND
 * PERFORMANCE OF THE SOFTWARE, INCLUDING ANY DUTY TO SUPPORT
 * OR MAINTAIN, BELONGS TO THE LICENSEE.  SHOULD ANY PORTION OF
 * THE SOFTWARE PROVE DEFECTIVE, THE LICENSEE (NOT IBM) ASSUMES
 * THE ENTIRE COST OF ALL SERVICING, REPAIR AND CORRECTION.  IN
 * NO EVENT SHALL IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#include "fontmisc.h"
 
typedef  pointer xobject;
typedef  pointer location;
typedef  pointer path;
typedef  pointer region;
typedef  pointer XYspace;
 
#ifndef NOEXTERNS
/*
The following are the user entry locations to TYPE1IMAGER
*/
extern path    t1_Bezier();
extern path    t1_ClosePath();
extern xobject t1_Destroy();
extern xobject t1_Dup();
extern char   *t1_ErrorMsg();
extern void    t1_InitImager();
extern region  t1_Interior();
extern location  t1_ILoc();
extern xobject t1_Join();
extern path    t1_Line();
extern xobject t1_Permanent();
extern path    t1_Phantom();
extern location   t1_Loc();
extern xobject t1_Scale();
extern xobject t1_Snap();
extern location  t1_SubLoc();
extern xobject t1_Temporary();
 
#endif
 
#ifndef   NULL
#define   NULL         0
#endif
 
#ifndef   TRUE
#define   TRUE          1
#endif
 
#ifndef   FALSE
#define   FALSE         0
#endif
 
/*
Here are some TYPE1IMAGER functions that are defined in terms of others:
*/
 
#define   t1_AddLoc(p1,p2)    t1_Join(p1,p2)
 
#ifndef   NONAMES
/*
Define the simple form of all the subroutine names:
*/
#define   AddLoc(p1,p2)          t1_AddLoc(p1,p2)
#define   Bezier(B,C,D)      t1_Bezier(B,C,D)
#define   ClosePath(p)       t1_ClosePath(p,0)
#define   Complement(area)   t1_Complement(area)
#define   Destroy(o)         t1_Destroy(o)
#define   Dup(o)             t1_Dup(o)
#define   ErrorMsg()         t1_ErrorMsg()
#define   HeadSegment(p)     t1_HeadSegment(p)
#define   InitImager()       t1_InitImager()
#define   Interior(p,rule)   t1_Interior(p,rule)
#define   ILoc(S,x,y)        t1_ILoc(S,x,y)
#define   Join(p1,p2)        t1_Join(p1,p2)
#define   Line(P)            t1_Line(P)
#define   Permanent(o)       t1_Permanent(o)
#define   Phantom(o)         t1_Phantom(o)
#define   Loc(S,x,y)         t1_Loc(S,(double)x,(double)y)
#define   Scale(o,sx,sy)     t1_Scale(o,(double)sx,(double)sy)
#define   Snap(o)            t1_Snap(o)
#define   SubLoc(a,b)        t1_SubLoc(a,b)
#define   Temporary(o)       t1_Temporary(o)
#define   TermImager()       t1_TermImager()
#define   Transform(o,cxx,cyx,cxy,cyy)  t1_Transform(o,(double)cxx,(double)cyx,\
    (double)cxy,(double)cyy)
 
#endif
 
#define   WINDINGRULE -2
#define   EVENODDRULE -3
 
#define   CONTINUITY  0x80   /* can be added to above rules; e.g. WINDINGRULE+CONTINUITY */
 
/*
Stroke() line style constants:
*/
 
/*
Coordinate space constants:
*/
#define   IDENTITY      t1_Identity
extern XYspace *IDENTITY;
 
/*
Generic null object definition:
*/
#define    NULLOBJECT   ((xobject)NULL)
 
/*
Null path definition:
*/
#define    NULLPATH     NULLOBJECT
 
/*
Full page and null region definition:
*/
#define    INFINITY     t1_Infinity
#ifndef NOEXTERNS
extern     region       *INFINITY;
#endif
#define    NULLREGION   NULLOBJECT
 
#define    FF_PARSE_ERROR  5
#define    FF_PATH         1
 
extern pointer xiStub();
