/* $Xorg: hints.c,v 1.3 2000/08/17 19:46:30 cpqbld Exp $ */
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
/* $XFree86: xc/lib/font/Type1/hints.c,v 1.7tsi Exp $ */

 /* HINTS    CWEB         V0006 ********                             */
/*
:h1.HINTS Module - Processing Rasterization Hints
 
&author. Sten F. Andler; continuity by Jeffrey B. Lotspiech (lotspiech@almaden.ibm.com) and Duaine
W. Pryor, Jr.
 
 
:h3.Include Files
 
The included files are:
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef FONTMODULE
# include "os.h"
#endif
#include "objects.h"
#include "spaces.h"
#include "paths.h"
#include "regions.h"
#include "hints.h"

/*
:h3.Functions Provided to the TYPE1IMAGER User
 
None.
*/
 
/*
:h3.Functions Provided to Other Modules
 
This module provides the following entry point to other modules:
*/
 
 
/*SHARED LINE(S) ORIGINATED HERE*/
 
/*
:h3.Macros Provided to Other Modules
 
None.
*/
 
/*
:h2.InitHints() - Initialize hint data structure
*/
 
#define MAXLABEL 20
static struct {
  int inuse;
  int computed;
  struct fractpoint hint;
} oldHint[MAXLABEL];
 
#define ODD(x) (((int)(x)) & 01)
#define FPFLOOR(fp) TOFRACTPEL((fp) >> FRACTBITS)
#define FPROUND(fp) FPFLOOR((fp) + FPHALF)
 
void 
InitHints(void)
{
  int i;
 
  for (i = 0; i < MAXLABEL; i++)
    {
    oldHint[i].inuse    = FALSE;
    oldHint[i].computed = FALSE;
    }
}
 
/*
:h3.CloseHints(hintP) - Reverse hints that are still open
*/
 
void 
CloseHints(struct fractpoint *hintP)
{
  int i;
 
  for (i = 0; i < MAXLABEL; i++)
    {
    if (oldHint[i].inuse)
      {
      hintP->x -= oldHint[i].hint.x;
      hintP->y -= oldHint[i].hint.y;
 
      oldHint[i].inuse = FALSE;
      }
    }
}
 
/*
:h3.ComputeHint(hP, currX, currY, hintP) - Compute the value of a hint
*/
 
static void 
ComputeHint(struct hintsegment *hP, 
	    fractpel currX, fractpel currY, 
	    struct fractpoint *hintP)
{
  fractpel currRef, currWidth;
  int idealWidth;
  fractpel hintValue;
  char orientation;
 
/*
By construction, width is never zero.  Therefore we can use the
width value to determine if the hint has been rotated by a
multiple of 90 degrees.
*/
 
  if (hP->width.y == 0)
    {
    orientation = 'v';  /* vertical */
    }
  else if (hP->width.x == 0)
    {
    orientation = 'h';  /* horizontal */
    }
  else
    {
    hintP->x = hintP->y = 0;
    return;
    }
 
  /* Compute currRef and currWidth with a unit of 1 pel */
  if (orientation == 'v')      /* vertical */
    {
    currRef = hP->ref.x + currX;
    currWidth = ABS(hP->width.x);
    }
  else if (orientation == 'h') /* horizontal */
    {
    currRef = hP->ref.y + currY;
    currWidth = ABS(hP->width.y);
    }
  else                             /* error */
    {
    Abort("ComputeHint: invalid orientation");
    }
 
  if ((hP->hinttype == 'b')      /* Bar or stem */
    || (hP->hinttype == 's'))    /* Serif */
    {
    idealWidth = NEARESTPEL(currWidth);
    if (idealWidth == 0) idealWidth = 1;
    if (ODD(idealWidth))         /* Is ideal width odd? */
      {
      /* center "ref" over pel */
      hintValue = FPFLOOR(currRef) + FPHALF - currRef;
      }
    else
      {
      /* align "ref" on pel boundary */
      hintValue = FPROUND(currRef) - currRef;
      }
    }
  else if (hP->hinttype == 'c')  /* Curve extrema */
    {
    /* align "ref" on pel boundary */
    hintValue = FPROUND(currRef) - currRef;
    }
  else                           /* error */
    {
    Abort("ComputeHint: invalid hinttype");
    }
 
  if (orientation == 'v')      /* vertical */
    {
    hintP->x = hintValue;
    hintP->y = 0;
    }
  else if (orientation == 'h') /* horizontal */
    {
    hintP->x = 0;
    hintP->y = hintValue;
    }
  else                             /* error */
    {
    Abort("ComputeHint: invalid orientation");
    }
}
 
/*
:h3.ProcessHint(hP, currX, currY, hintP) - Process a rasterization hint
*/
 
void 
ProcessHint(struct hintsegment *hP, 
	    fractpel currX, fractpel currY, 
	    struct fractpoint *hintP)
{
  struct fractpoint thisHint;
 
  if ((hP->adjusttype == 'm')      /* Move */
    || (hP->adjusttype == 'a'))    /* Adjust */
    {
    /* Look up hint in oldHint table */
    if ((hP->label >= 0) && (hP->label < MAXLABEL))
      {
      if (oldHint[hP->label].computed)
        /* Use old hint value if already computed */
        {
        thisHint.x = oldHint[hP->label].hint.x;
        thisHint.y = oldHint[hP->label].hint.y;
        oldHint[hP->label].inuse    = TRUE;
        }
      else
        /* Compute new value for hint and store it for future use */
        {
        ComputeHint(hP, currX, currY, &thisHint);
 
        oldHint[hP->label].hint.x = thisHint.x;
        oldHint[hP->label].hint.y = thisHint.y;
        oldHint[hP->label].inuse    = TRUE;
        oldHint[hP->label].computed = TRUE;
        }
      }
    else                             /* error */
      {
      Abort("ProcessHint: invalid label");
      }
    }
  else if (hP->adjusttype == 'r')  /* Reverse */
    {
    /* Use the inverse of the existing hint value to reverse hint */
    if ((hP->label >= 0) && (hP->label < MAXLABEL))
      {
      if (oldHint[hP->label].inuse)
        {
        thisHint.x = -oldHint[hP->label].hint.x;
        thisHint.y = -oldHint[hP->label].hint.y;
        oldHint[hP->label].inuse = FALSE;
        }
      else                           /* error */
        {
        Abort("ProcessHint: label is not in use");
        }
      }
    else                           /* error */
      {
      Abort("ProcessHint: invalid label");
      }
 
    }
  else                           /* error */
    {
    Abort("ProcessHint: invalid adjusttype");
    }
 
  hintP->x += thisHint.x;
  hintP->y += thisHint.y;
}
 
/*
:h2 id=subpath.Navigation Through Edge Lists
 
For continuity checking purposes, we need to navigate through edge
lists by the "subpath" chains and answer questions about edges.  The
subpath chain links together edges that were part of the same subpath
(no intervening move segments) when the interior of the path was
calculated.  Here we use the term "edge" to mean every edge list
that was created in between changes of direction.
 
The subpath chains are singly-linked circular chains.  For the convenience
of building them, they direction of the list (from edge to edge) is the
reverse of the order in which they were built.  Within any single edge,
the subpath chain goes from top-to-bottom.  (There might be a violation
of this because of the way the user started the first chain; see
:hdref refid=fixsubp..).
 
:h3.ISTOP() and ISBOTTOM() - Flag Bits for Edge Lists at the Top and
Bottom of Their SubPaths
*/
 
#define   ISTOP(flag)     ((flag)&0x20)
#define   ISBOTTOM(flag)  ((flag)&0x10)
/*
:h3.ISLEFT() - Flag Bit for Left Edges
*/
 
#define   ISLEFT(flag)    ((flag)&0x08)
 
/*
:h3.XofY() - Macro to Find X Value at Given Y
 
This macro can only be used if it is known that the Y is within the
given edgelist's ymin and ymax.
*/
 
#define   XofY(edge, y)   edge->xvalues[y - edge->ymin]
 
/*
:h3.findXofY() - Like XofY(), Except not Restricted
 
If the Y is out of bounds of the given edgelist, this macro will
call SearchXofY to search the edge's subpath chain for the correct
Y range.  If the Y value is off the edge, MINPEL is returned.
*/
#define   findXofY(edge, y)  ((y < edge->ymin || y >= edge->ymax) ? SearchXofY(edge, y) : XofY(edge, y))
 
/*
:h4.SearchXofY() - Routine Called by FindXofY() for Difficult Cases
 
The concept of this routine is to follow the subpath chain to find the
edge just below (i.e., next in chain) or just above (i.e., immediately
before in chain.  It is assumed that the Y value is no more than one
off of the edge's range; XofY() could be replace by FindXofY() to
call ourselves recursively if this were not true.
*/
 
static pel 
SearchXofY(register struct edgelist *edge,   /* represents edge              */
	   register pel y)                   /* 'y' value to find edge for   */
{
       register struct edgelist *e;  /* loop variable                        */
 
       if (y < edge->ymin) {
               if (ISTOP(edge->flag))
                       return(MINPEL);
               for (e = edge->subpath; e->subpath != edge; e = e->subpath) { ; }
               if (e->ymax == edge->ymin)
                        return(XofY(e, y));
       }
       else if (y >= edge->ymax) {
               if (ISBOTTOM(edge->flag))
                       return(MINPEL);
               e = edge->subpath;
               if (e->ymin == edge->ymax)
                         return(XofY(e, y));
       }
       else
               return(XofY(edge, y));
 
       Abort("bad subpath chain");
       /*NOTREACHED*/
}
/*
:h3.ISBREAK() Macro - Tests if an Edge List is at a "Break"
 
The subpath chains are organized top to bottom.  When the bottom of
a given edge is reached, the subpath chain points to the top of the
next edge.  We call this a "break" in the chain.  The following macro
is the simple test for the break condition:
*/
 
#define  ISBREAK(top,bot) (top->ymax != bot->ymin)
 
 
/*
:h3.ImpliedHorizontalLine() - Tests for Horizontal Connectivity
 
This function returns true if two edges are connected horizontally.
They are connected horizontally if they are consecutive in the subpath,
and either we are at the bottom and the first edge is going down or we
are at the top and the first edge is going up.
*/
 
#define  BLACKABOVE  -1
#define  BLACKBELOW  +1
#define  NONE         0
 
static int 
ImpliedHorizontalLine(struct edgelist *e1, /* two edges to check            */
		      struct edgelist *e2, 
		      int y) /* y where they might be connected             */
{
       register struct edgelist *e3,*e4;
 
       if (ISDOWN(e1->flag) == ISDOWN(e2->flag))
               return(NONE);  /* can't be consecutive unless different directions */
/*
Now we check for consecutiveness:  Can we get from 'e1' to 'e2' with
only one intervening break?  Can we get from 'e2' to 'e1' with only one
intervening break?  'e3' will be as far as we can get after 'e1'; 'e4'
will be has far as we can get after 'e2':
*/
       for (e3 = e1; !ISBREAK(e3, e3->subpath); e3 = e3->subpath) { ; }
       for (e3 = e3->subpath; e3 != e2; e3 = e3->subpath)
               if (ISBREAK(e3, e3->subpath))
                       break;
 
       for (e4 = e2; !ISBREAK(e4, e4->subpath); e4 = e4->subpath) { ; }
       for (e4 = e4->subpath; e4 != e1; e4 = e4->subpath)
               if (ISBREAK(e4, e4->subpath))
                       break;
/*
If the edges are mutually consecutive, we must have horizontal lines
both top and bottom:
*/
       if (e3 == e2 && e4 == e1)
               return(TRUE);
/*
If the edges are not consecutive either way, no horizontal lines are
possible:
*/
       if (e3 != e2 && e4 != e1)
               return(NONE);
/*
Now let's swap 'e1' and 'e2' if necessary to enforce the rule that 'e2'
follows 'e1'.  Remember that subpath chains go in the opposite direction
from the way the subpaths were built; this led to the simplest way
do build them.
*/
       if (e4 != e1) {
               e2 = e1;
               e1 = e3;  /* remember e3 == e2, this just swaps 'e1' and 'e2' */
       }
/*
Now we have everything to return the answer:
*/
       if (ISTOP(e1->flag) && y == e1->ymin)
               return(ISDOWN(e2->flag));
       else if (ISBOTTOM(e1->flag) && y == e1->ymax)
               return(!ISDOWN(e2->flag));
       else
               Abort("ImpliedHorizontalLine:  why ask?");
       /*NOTREACHED*/
}
 
/*
:h3 id=fixsubp.FixSubPaths() - Must be Called to Organize Subpath Chains
 
The region-building code in Interior(), in particular splitedge(),
maintains the rule that sub-paths are linked top-to-bottom except
at breaks.  However, it is possible that there may be a "false break"
because the user started the subpath in the middle of an edge (and
went in the "wrong" direction from there, up instead of down).  This
routine finds and fixes false breaks.
 
Also, this routine sets the ISTOP and ISBOTTOM flags in the edge lists.
*/
 
static void 
FixSubPaths(struct region *R)           /* anchor of region                  */
{
       register struct edgelist *e;     /* fast loop variable                */
       register struct edgelist *edge;  /* current edge in region            */
       register struct edgelist *next;  /* next in subpath after 'edge'      */
       register struct edgelist *break1;  /* first break after 'next'        */
       register struct edgelist *break2 = NULL;  /* last break before 'edge' */
       register struct edgelist *prev;    /* previous edge for fixing links  */
       int left = TRUE;
 
       for (edge = R->anchor; edge != NULL; edge = edge->link) {
 
               if (left)
                       edge->flag |= ISLEFT(ON);
               left = !left;
 
               next = edge->subpath;
 
               if (!ISBREAK(edge, next))
                       continue;
               if (edge->ymax < next->ymin)
                       Abort("disjoint subpath?");
/*
'edge' now contains an edgelist at the bottom of an edge, and 'next'
contains the next subsequent edgelist in the subpath, which must be at
the top.  We refer to this a "break" in the subpath.
*/
               next->flag |= ISTOP(ON);
               edge->flag |= ISBOTTOM(ON);
 
               if (ISDOWN(edge->flag) != ISDOWN(next->flag))
                       continue;
/*
We are now in the unusual case; both edges are going in the same
direction so this must be a "false break" due to the way that the user
created the path.  We'll have to fix it.
*/
               for (break1 = next; !ISBREAK(break1, break1->subpath); break1 = break1->subpath) { ; }
 
               for (e = break1->subpath; e != edge; e = e->subpath)
                       if (ISBREAK(e, e->subpath))
                               break2 = e;
/*
Now we've set up 'break1' and 'break2'.  I've found the following
diagram invaluable.  'break1' is the first break after 'next'.  'break2'
is the LAST break before 'edge'.
&drawing.
         next
        +------+     +---->+------+
   +--->|    >-----+ |     |    >-----+
   |    |      |   | |     |      |   |
   | +-------------+ |  +-------------+
   | |  |break1|     |  |  |      |
   | +->|    >-------+  +->|    >-----+
   |    |      |           |      |   |
   |    |      |        +-------------+
   |    +------+        |  |      |
   | +----------------+ |  |      |
   | |  +------+      | +->|    >-----+
   | +->|    >-----+  |    |      |   |
   |    |      |   |  | +-------------+
   | +-------------+  | |  |      |
   | |  |edge  |      | |  |break2|
   | +->|    >-----+  | +->|    >-----+
   |    |      |   |  |    |      |   |
   |    |      |   |  |    |      |   |
   |    |      |   |  |    |      |   |
   |    +------+   |  |    +------+   |
   |               |  |               |
   +---------------+  +---------------+
 
&edrawing.
We want to fix this situation by having 'edge' point to where 'break1'
now points, and having 'break1' point to where 'break2' now points.
Finally, 'break2' should point to 'next'.  Also, we observe that
'break1' can't be a bottom, and is also not a top unless it is the same
as 'next':
*/
               edge->subpath = break1->subpath;
 
               break1->subpath = break2->subpath;
               if (ISBREAK(break1, break1->subpath))
                       Abort("unable to fix subpath break?");
 
               break2->subpath = next;
 
               break1->flag &= ~ISBOTTOM(ON);
               if (break1 != next)
                       break1->flag &= ~ISTOP(ON);
       }
/*
This region might contain "ambiguous" edges; edges exactly equal to
edge->link.  Due to the random dynamics of where they get sorted into
the list, they can yield false crossings, where the edges appear
to cross.  This confuses our continuity logic no end.  Since we can
swap them without changing the region, we do.
*/
       for (edge = R->anchor, prev = NULL; VALIDEDGE(edge); prev = edge, edge = prev->link) {
 
               if (! ISAMBIGUOUS(edge->flag))
                       continue;
 
               next = edge->subpath;
 
               while (ISAMBIGUOUS(next->flag) && next != edge)
                       next = next->subpath;
/*
We've finally found a non-ambiguous edge; we make sure it is left/right
compatible with 'edge':
*/
               if ( (ISLEFT(edge->flag) == ISLEFT(next->flag) && ISDOWN(edge->flag) == ISDOWN(next->flag) )
                    || (ISLEFT(edge->flag) != ISLEFT(next->flag) && ISDOWN(edge->flag) != ISDOWN(next->flag) ) )
                       continue;
 
/*
Incompatible, we will swap 'edge' and the following edge in the list.
You may think that there must be a next edge in this swath.  So did I.
No!  If there is a totally ambiguous inner loop, for example, we could
get all the way to the outside without resolving ambiguity.
*/
               next = edge->link;  /* note new meaning of 'next' */
               if (next == NULL || edge->ymin != next->ymin)
                       continue;
               if (prev == NULL)
                       R->anchor = next;
               else
                       prev->link = next;
               edge->link = next->link;
               next->link = edge;
               edge->flag ^= ISLEFT(ON);
               edge->flag &= ~ISAMBIGUOUS(ON);
               next->flag ^= ISLEFT(ON);
               next->flag &= ~ISAMBIGUOUS(ON);
               edge = next;
       }
}
/*
:h3.DumpSubPaths()
 
A debug tool.
*/
 
static struct edgelist *before(struct edgelist *e);  /* subroutine of DumpSubPaths             */
 
static void
DumpSubPaths(struct edgelist *anchor)
{
 
       register struct edgelist *edge,*e,*e2;
       pel y;
 
       for (edge = anchor; VALIDEDGE(edge); edge = edge->link) {
               if (ISPERMANENT(edge->flag))
                       continue;
               for (e2 = edge; !ISPERMANENT(e2->flag);) {
                       if (ISDOWN(e2->flag)) {
                               for (e = e2;; e = e->subpath) {
                                       for (y=e->ymin+1; y < e->ymax; y++)
                                       e->flag |= ISPERMANENT(ON);
                                       if (ISBREAK(e, e->subpath))
                                               break;
                               }
                       }
                       else {
                               for (e = e2; !ISBREAK(e, e->subpath); e = e->subpath) { ; }
                               for (;; e=before(e)) {
                                       for (y=e->ymax-2; y >= e->ymin; y--)
                                       e->flag |= ISPERMANENT(ON);
                                       if (e == e2)
                                               break;
                               }
                       }
                       do {
                               e2 = before(e2);
                       } while (!ISBREAK(before(e2), e2));
               }
       }
}
 
static struct edgelist *
before(struct edgelist *e)
{
       struct edgelist *r;
       for (r = e->subpath; r->subpath != e; r = r->subpath) { ; }
       return(r);
}
 
/*
:h2.Fixing Region Continuity Problems
 
Small regions may become disconnected when their connecting segments are
less than a pel wide.  This may be correct in some applications, but in
many (especially small font characters), it is more pleasing to keep
connectivity.  ApplyContinuity() (invoked by +CONTINUITY on the
Interior() fill rule) fixes connection breaks.  The resulting region
is geometrically less accurate, but may be more pleasing to the eye.
*/
/*
Here are some macros which we will need:
*/
 
#define IsValidPel(j) (j!=MINPEL)
 
/*
:h3.writeXofY() - Stuffs an X Value Into an "edgelist"
 
writeXofY writes an x value into an edge at position 'y'.  It must
update the edge's xmin and xmax.  If there is a possibility that this
new x might exceed the region's bounds, updating those are the
responsibility of the caller.
*/
 
static void 
writeXofY(struct edgelist *e,/* relevant edgelist                            */
	  int y,             /* y value                                      */
	  int x)             /* new x value                                  */
{
       if (e->xmin > x)  e->xmin = x;
       if (e->xmax < x)  e->xmax = x;
       e->xvalues[y - e->ymin] = x;
}
 
/*-------------------------------------------------------------------------*/
/* the following three macros tell us whether we are at a birth point, a    */
/* death point, or simply in the middle of the character                */
/*-------------------------------------------------------------------------*/
#define WeAreAtTop(e,i) (ISTOP(e->flag) && e->ymin == i)
#define WeAreAtBottom(e,i) (ISBOTTOM(e->flag) && e->ymax-1 == i)
#define WeAreInMiddle(e,i) \
      ((!ISTOP(e->flag) && !ISBOTTOM(e->flag))||(i < e->ymax-1 && i > e->ymin))
/*
The following macro tests if two "edgelist" structures are in the same
swath:
*/
#define SAMESWATH(e1,e2)  (e1->ymin == e2->ymin)
 
/*
:h3.CollapseWhiteRun() - Subroutine of ApplyContinuity()
 
When we have a white run with an implied horizontal line above or
below it, we better have black on the other side of this line.  This
function both tests to see if black is there, and adjusts the end
points (collapses) the white run as necessary if it is not.  The
goal is to collapse the white run as little as possible.
*/
 
static void 
CollapseWhiteRun(struct edgelist *anchor, /* anchor of edge list             */
		 pel yblack, /* y of (hopefully) black run above or below    */
		 struct edgelist *left, /* edgelist at left of WHITE run     */
		 struct edgelist *right, /* edgelist at right of WHITE run   */
		 pel ywhite) /* y location of white run                      */
{
       struct edgelist *edge;
       struct edgelist *swathstart = anchor;
       register pel x;
 
       if (XofY(left, ywhite) >= XofY(right, ywhite))
               return;
/*
Find the swath with 'yblack'.  If we don't find it, completely collapse
the white run and return:
*/
       while (VALIDEDGE(swathstart)) {
               if (yblack < swathstart->ymin)  {
                      writeXofY(left, ywhite, XofY(right, ywhite));
                      return;
               }
               if (yblack < swathstart->ymax) break;
               swathstart = swathstart->link->link;
       }
       if(!VALIDEDGE(swathstart)) {
               writeXofY(left, ywhite, XofY(right, ywhite));
               return;
       }
/*
Now we are in the swath that contains 'y', the reference line above
or below that we are trying to maintain continuity with.  If black
in this line begins in the middle of our white run, we must collapse
the white run from the left to that point.  If black ends in the
middle of our white run, we must collapse the white run from the right
to that point.
*/
       for (edge = swathstart; VALIDEDGE(edge); edge = edge->link) {
 
               if (!SAMESWATH(swathstart,edge))
                       break;
               if( XofY(edge, yblack) > XofY(left, ywhite)) {
                       if (ISLEFT(edge->flag)) {
                                x = XofY(edge, yblack);
                                if (XofY(right, ywhite) < x)
                                       x = XofY(right, ywhite);
                                writeXofY(left, ywhite, x);
                       }
                       else {
                                x = XofY(edge, yblack);
                                while (edge->link != NULL && SAMESWATH(edge, edge->link)
                                       && x >= XofY(edge->link, yblack) ) {
                                       edge = edge->link->link;
                                       x = XofY(edge, yblack);
                                }
                                if (x < XofY(right, ywhite))
                                       writeXofY(right, ywhite, x);
                                return;
                       }
               }
       }
       writeXofY(left, ywhite, XofY(right, ywhite));
}
 
/*
:h3.ApplyContinuity() - Fix False Breaks in a Region
 
This is the externally visible routine called from the REGIONS module
when the +CONTINUITY flag is on the Interior() fill rule.
*/
 
void 
ApplyContinuity(struct region *R)
{
 struct edgelist *left;
 struct edgelist *right;
 struct edgelist *edge,*e2;
 pel rightXabove,rightXbelow,leftXabove,leftXbelow;
 pel leftX,rightX;
 int i;
 long newcenter,abovecenter,belowcenter;
 
 FixSubPaths(R);
 if (RegionDebug >= 3)
        DumpSubPaths(R->anchor);
 left = R->anchor;
/* loop through and do all of the easy checking. ( no tops or bottoms) */
 while(VALIDEDGE(left))
 {
  right = left->link;
  for(i=left->ymin;i<left->ymax;++i)
  {
   leftX       = findXofY(left,i);
   rightX      = findXofY(right,i);
   leftXbelow  = findXofY(left,i+1);
   rightXbelow = findXofY(right,i+1);
   if(rightX <= leftX)
   {
/* then, we have a break in a near vertical line */
     leftXabove  = findXofY(left,i-1);
     rightXabove = findXofY(right,i-1);
     if( IsValidPel(leftXabove) && IsValidPel(rightXabove) )
     {
      abovecenter = leftXabove + rightXabove;
     }
     else
     {
      abovecenter = leftX + rightX;
     }
     if( IsValidPel(leftXbelow) && IsValidPel(rightXbelow) )
     {
      belowcenter = leftXbelow + rightXbelow;
     }
     else
     {
      belowcenter = leftX + rightX;
     }
     newcenter = abovecenter + belowcenter;
     if( newcenter > 4*leftX )
     {
      rightX = rightX + 1;
     }
     else if( newcenter < 4*leftX)
     {
      leftX = leftX - 1;
     }
     else
     {
      rightX = rightX + 1;
     }
     writeXofY(right,i,rightX);
     writeXofY(left,i,leftX);
     if(rightX > R->xmax) {R->xmax = rightX;}
     if(leftX < R->xmin) {R->xmin = leftX;}
   }
   if( !WeAreAtBottom(left,i) && (leftXbelow>=rightX))
   {
/* then we have a break in a near horizontal line in the middle */
    writeXofY(right,i,leftXbelow);
   }
   if( !WeAreAtBottom(right,i) && (leftX >=rightXbelow))
   {
/* then we have a break in a near horizontal line in the middle */
    writeXofY(left,i,rightXbelow);
   }
  }
  left = right->link;
 }
/*
There may be "implied horizontal lines" between edges that have
implications for continuity.  This loop looks for white runs that
have implied horizontal lines on the top or bottom, and calls
CollapseWhiteRuns to check and fix any continuity problems from
them.
*/
      for (edge = R->anchor; VALIDEDGE(edge); edge = edge->link) {
              if ((!ISTOP(edge->flag) && !ISBOTTOM(edge->flag)) || ISLEFT(edge->flag))
                      continue;  /* at some future date we may want left edge logic here too */
              for (e2 = edge->link; VALIDEDGE(e2) && SAMESWATH(edge,e2); e2 = e2->link) {
                      if (ISTOP(e2->flag) && ISTOP(edge->flag)
                          && NONE != ImpliedHorizontalLine(edge,e2,edge->ymin)) {
                              if (ISLEFT(e2->flag))
                                      CollapseWhiteRun(R->anchor, edge->ymin-1,
                                                       edge, e2, edge->ymin);
                      }
                      if (ISBOTTOM(e2->flag) && ISBOTTOM(edge->flag)
                          && NONE != ImpliedHorizontalLine(edge,e2, edge->ymax)) {
                              if (ISLEFT(e2->flag))
                                      CollapseWhiteRun(R->anchor, edge->ymax,
                                                       edge, e2, edge->ymax-1);
                      }
              }
      }
}
 
 
 
 
