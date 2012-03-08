/* $Xorg: regions.c,v 1.3 2000/08/17 19:46:31 cpqbld Exp $ */
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
/* $XFree86: xc/lib/font/Type1/regions.c,v 3.9 2003/05/27 22:26:46 tsi Exp $ */
 /* REGIONS  CWEB         V0023 LOTS                                 */
/*
:h1 id=regions.REGIONS Module - Regions Operator Handler
 
This module is responsible for creating and manipulating regions.
 
&author. Jeffrey B. Lotspiech (lotspiech@almaden.ibm.com)
 
 
:h3.Include Files
 
The included files are:
*/
 
#include  "os.h"
#include  "objects.h"
#include  "spaces.h"
#include  "paths.h"
#include  "regions.h"
#include  "curves.h"
#include  "lines.h"
#include  "pictures.h"
#include  "fonts.h"
#include  "hints.h"
#include  "strokes.h"      /* to pick up 'DoStroke'                        */


static void newfilledge ( struct region *R, fractpel xmin, fractpel xmax, 
			  fractpel ymin, fractpel ymax, int isdown );
static struct edgelist *splitedge ( struct edgelist *list, pel y );
static void vertjoin ( struct edgelist *top, struct edgelist *bottom );
static int touches ( int h, pel *left, pel *right );
static int crosses ( int h, pel *left, pel *right );
static void edgemin ( int h, pel *e1, pel *e2 );
static void edgemax ( int h, pel *e1, pel *e2 );
static struct edgelist *NewEdge ( pel xmin, pel xmax, pel ymin, pel ymax, 
				  pel *xvalues, int isdown );
static struct edgelist *swathxsort ( struct edgelist *before0, 
				     struct edgelist *edge );
/*
:h3.Functions Provided to the TYPE1IMAGER User
 
This module provides the following TYPE1IMAGER entry points:
*/
 
/*SHARED LINE(S) ORIGINATED HERE*/
/*
:h3.Functions Provided to Other Modules
 
This module provides the following entry points to other modules:
*/
 
/*SHARED LINE(S) ORIGINATED HERE*/
/*
:h3.Macros Provided to Other Modules
 
:h4.GOING_TO() - Macro Predicate Needed for Changing Direction, Etc.
 
The actual generation of run end lists (edge boundaries) is left
to the low level rasterizing modules, LINES and CURVES.  There
are some global region-type
questions that occur when doing a low-level
rasterization:
:ol.
:li.Did we just change direction in Y and therefore need to start
a new edge?
:li.Did we run out of allocated edge space?
:li.Do the minimum or maximum X values for the current edge need
updating?
:eol.
In general the REGIONS is not smart enough to answer those questions
itself.  (For example, determining if and when a curve changes direction
may need detailed curve knowledge.)  Yet, this must be done efficiently.
We provide a macro "GOING_TO" where the invoker tells us where it is
heading for (x2,y2), plus where it is now (x1,y1), plus the current
region under construction, and the macro answers the questions above.
*/
 
/*SHARED LINE(S) ORIGINATED HERE*/
/*
:h2.Data Structures Used to Represent Regions
 
:h3.The "region" Structure
 
The region structure is an anchor for a linked list of "edgelist"
structures (see :hdref refid=edgelist..).  It also summarizes the
information in the edgelist structures (for example, the bounding
box of the region).  And, it contains scratch areas used during
the creation of a region.
*/
 
/*SHARED LINE(S) ORIGINATED HERE*/
/*
The ISOPTIMIZED flag tells us if we've put a permanent region in
'optimal' form.
*/
#define   ISOPTIMIZED(flag)    ((flag)&0x10)
 
/*
The ISRECTANGULAR flag tells us if a region is a rectangle.  We don't
always notice rectangles--if this flag is set, the region definitely
is a rectangle, but some rectangular regions will not have the flag
set.  The flag is used to optimize some paths.
*/
 
/*SHARED LINE(S) ORIGINATED HERE*/
/*
:h4."TT_INFINITY" - A Constant Region Structure of Infinite Extent
 
Infinity is the complement of a null area:
Note - removed the refcount = 1 init, replaced with references = 2 3-26-91 PNM
*/
static struct region _infinity = { REGIONTYPE,
                           ISCOMPLEMENT(ON)+ISINFINITE(ON)+ISPERMANENT(ON)+ISIMMORTAL(ON), 2,
                           {0, 0}, {0, 0},
                           0, 0, 0, 0,
                           NULL, NULL,
                           0, 0, 0, 0, 0, NULL, NULL,
                           NULL, 0, NULL, NULL };
struct region *TT_INFINITY = &_infinity;
 
/*
:h4."EmptyRegion" - A Region Structure with Zero Area
 
This structure is used to initialize the region to be built in
Interior():
Note - replaced refcount = 1 init with references = 2 3-26-91 PNM
*/
 
/*SHARED LINE(S) ORIGINATED HERE*/
struct region EmptyRegion = { REGIONTYPE,
                           ISPERMANENT(ON)+ISIMMORTAL(ON), 2,
                           {0, 0}, {0, 0},
                           MAXPEL, MAXPEL, MINPEL, MINPEL,
                           NULL, NULL,
                           0, 0, 0, 0, 0, NULL, NULL,
                           NULL, 0, NULL, NULL };
 
/*
:h3 id=edgelist.The "edgelist" Structure
 
Regions are represented by a linked list of 'edgelist' structures.
When a region is complete, the structures are paired, one for the
left and one for the right edge.  While a region is being built,
this rule may be violated temporarily.
 
An 'edgelist' structure contains the X values for a given span
of Y values.  The (X,Y) pairs define an edge.  We use the crack
and edge coordinate system, so that integer values of X and Y
go between pels.  The edge is defined between the minimum Y and
maximum Y.
 
The linked list is kept sorted from top to bottom, that is, in
increasing y.  Also, if 'e1' is an edgelist structure and 'e2' is the
next one in the list, they must have exactly the same ymin,ymax values
or be totally disjoint.  These two requirements mean that if e2's ymin
is less than e1's ymax, it must be exactly equal to e1's ymin.  A
sublist of structures with identical ymin and ymax values is called a
'swath'.
 
In addition, edgelist structures are separately linked together based
on what subpath originally created them; each subpath is kept as a
separate circular linked list.  This information is ignored unless
continuity checking is invoked.  See :hdref refid=subpath. for a
complete description of this.
*/
 
 
/*SHARED LINE(S) ORIGINATED HERE*/
 
/*
The "edgelist" structure follows the convention of TYPE1IMAGER user
objects, having a type field and a flag field as the first two
elements.  However, the user never sees "edgelist" structures
directly; he is given handles to "region" structures only.
 
By having a type field, we can use the "copy" feature of Allocate()
to duplicate edge lists quickly.
 
We also define two flag bits for this structure.  The ISDOWN bit is set
if the edge is going in the direction of increasing Y. The ISAMBIGUOUS
bit is set if the edge is identical to its neighbor (edge->link); such
edges may be "left" when they should be "right", or vice versa,
unnecessarily confusing the continuity checking logic.  The FixSubPaths()
routine in HINTS will swap ambiguous edges if that avoids crossing edges;
see :hdref refid=fixsubp..
*/
 
/*SHARED LINE(S) ORIGINATED HERE*/
 
/*
:h3.KillRegion() - Destroys a Region
 
KillRegion nominally just decrements the reference count to that region.
If the reference count becomes 0, all memory associated with it is
freed.  We just follow the linked list, freeing as we go, then kill any
associated (thresholded) picture.
Note - added conditional return based on references 3-26-91 PNM
*/
 
void 
KillRegion(struct region *area)  /* area to free                             */
{
        register struct edgelist *p;  /* loop variable                       */
        register struct edgelist *next;  /* loop variable                    */
 
        if (area->references < 0)
               Abort("KillRegion:  negative reference count");
        if ( (--(area->references) > 1) ||
           ( (area->references == 1) && !ISPERMANENT(area->flag) ) )
            return;
 
        for (p=area->anchor; p != NULL; p=next) {
               next = p->link;
               Free(p);
        }
        if (area->thresholded != NULL)
                 KillPicture(area->thresholded);
        Free(area);
}
/*
:h3.CopyRegion() - Makes a Copy of a Region
*/
struct region *
CopyRegion(struct region *area)        /* region to duplicate                */
{
        register struct region *r;  /* output region built here              */
        register struct edgelist *last = NULL;  /* loop variable             */
        register struct edgelist *p,*newp;  /* loop variables                */
 
        r = (struct region *)Allocate(sizeof(struct region), area, 0);
        r->anchor = NULL;
 
        for (p=area->anchor; VALIDEDGE(p); p=p->link) {
 
               newp = NewEdge(p->xmin, p->xmax, p->ymin, p->ymax, p->xvalues, ISDOWN(p->flag));
               if (r->anchor == NULL)
                       r->anchor = last = newp;
               else
                       last->link = newp;
 
               last = newp;
        }
        if (area->thresholded != NULL)
    /* replaced DupPicture with Dup() 3-26-91 PNM */
               r->thresholded = (struct picture *)Dup(area->thresholded);
        return(r);
}
/*
:h4.NewEdge() - Allocates and Returns a New "edgelist" Structure
 
We allocate space for the X values contiguously with the 'edgelist'
structure that locates them.  That way, we only have to free the
edgelist structure to free all memory associated with it.  Damn
clever, huh?
*/
 
static struct edgelist *     
NewEdge(pel xmin, pel xmax,  /* X extent of edge                             */
	pel ymin, pel ymax,  /* Y extent of edge                             */
	pel *xvalues,        /* list of X values for entire edge             */
	int isdown)          /* flag:  TRUE means edge progresses downward   */
{
       static struct edgelist template = {
                 EDGETYPE, 0, 1, NULL, NULL,
                 0, 0, 0, 0, NULL };
 
       register struct edgelist *r;  /* returned structure                   */
       register int iy;      /* ymin adjusted for 'long' alignment purposes  */
 
       if (ymin >= ymax)
               Abort("newedge: height not positive");
/*
We are going to copy the xvalues into a newly allocated area.  It
helps performance if the values are all "long" aligned.  We can test
if the xvalues are long aligned by ANDing the address with the
(sizeof(long) - 1)--if non zero, the xvalues are not aligned well.  We
set 'iy' to the ymin value that would give us good alignment:
*/
       iy = ymin - (((unsigned long)xvalues) & (sizeof(long)-1)) / sizeof(pel);
 
       r = (struct edgelist *)Allocate(sizeof(struct edgelist), &template,
                             (ymax - iy) * sizeof(pel));
 
       if (isdown) r->flag = ISDOWN(ON);
       r->xmin = xmin;
       r->xmax = xmax;
       r->ymin = ymin;
       r->ymax = ymax;
 
       r->xvalues = (pel *) FOLLOWING(r);
       if (ymin != iy) {
               r->xvalues += ymin - iy;
               xvalues -= ymin - iy;
       }
 
/*
We must round up (ymax - iy) so we get the ceiling of the number of
longs.  The destination must be able to hold these extra bytes because
Allocate() makes everything it allocates be in multiples of longs.
*/
       LONGCOPY(&r[1], xvalues, (ymax - iy) * sizeof(pel) + sizeof(long) - 1);
 
       return(r);
}

/*
:h3 id=discard.discard() - Discard All Edges Between Two Edges
 
At first glance it would seem that we could discard an edgelist
structure merely by unlinking it from the list and freeing it.  You are
wrong, region-breath!  For performance, the X values associated with an
edge are allocated contiguously with it.  So, we free the X values when
we free a structure.  However, once an edge has been split, we are no
longer sure which control block actually is part of the memory block
that contains the edges.  Rather than trying to decide, we play it safe
and never free part of a region.
 
So, to mark a 'edgelist' structure as discarded, we move it to the end
of the list and set ymin=ymax.
*/
 
static void
discard(struct edgelist *left,  /* all edges between here exclusive          */
	struct edgelist *right) /*   should be discarded                     */
{
       register struct edgelist *beg,*end,*p;
 
       beg = left->link;
       if (beg == right)
               return;
 
       for (p = beg; p != right; p = p->link) {
               if (p->link == NULL && right != NULL)
                       Abort("discard():  ran off end");
               p->ymin = p->ymax = 32767;
               end = p;
       }
       /*
       * now put the chain beg/end at the end of right, if it is not
       * already there:
       */
       if (right != NULL) {
               left->link = right;
               while (right->link != NULL)
                       right = right->link;
               right->link = beg;
       }
       end->link = NULL;
}
 
/*
:h4.Unwind() - Discards Edges That Fail the Winding Rule Test
 
The winding rule says that upward going edges should be paired with
downward going edges only, and vice versa.  So, if two upward edges
or two downward edges are nominally left/right pairs, Unwind() should
discard the second one.  Everything should balance; we should discard
an even number of edges; of course, we abort if we don't.
*/
static void
Unwind(struct edgelist *area)   /* input area modified in place              */
{
       register struct edgelist *last = NULL,*next;  /* struct before and after current one */
       register int y;       /* ymin of current swath                        */
       register int count,newcount;  /* winding count registers              */
 
       while (VALIDEDGE(area)) {
 
               count = 0;
               y = area->ymin;
 
               do {
                       next = area->link;
 
                       if (ISDOWN(area->flag))
                               newcount = count + 1;
                       else
                               newcount = count - 1;
 
                       if (count == 0 || newcount == 0)
                               last = area;
                       else
                               discard(last, next);
 
                       count = newcount;
                       area = next;
 
               } while (area != NULL && area->ymin == y);
 
               if (count != 0)
                       Abort("Unwind:  uneven edges");
       }
}
/*
:h2.Building Regions
 
:h3.Interior() - Iterate Through a Path, Building a Region
 
This routine is the workhorse driver routine that iterates through a
path, calling the appropriate stepping routines to actually produce the
run end "edgelist" structures.
 
:ol.
:li."Interior" calls StepLine or StepConic or StepBezier as appropriate
to produce run ends.
:li.Occasionally these routines will notice a change in Y direction
and will call ChangeDirection (through the GOING_TO macro); this is
a call back to the REGIONS module.
:li.ChangeDirection will call whatever function is in the region
structure; for Interior, this function is 'newfilledge'.
:li.Newfilledge will call NewEdge to create a new edgelist structure,
then, call SortSwath to sort it onto the linked list being built at
the region "anchor".
:eol.
 
By making the function called by ChangeDirection be a parameter of the
region, we allow the same ChangeDirection logic to be used by stroking.
*/
 
/*SHARED LINE(S) ORIGINATED HERE*/
 
struct region *
Interior(struct segment *p,  /* take interior of this path          */
	 int fillrule)       /* rule to follow if path crosses itself */
{
       register fractpel x,y;  /* keeps ending point of path segment         */
       fractpel lastx,lasty; /* previous x,y from path segment before        */
       register struct region *R;  /* region I will build                    */
       register struct segment *nextP; /* next segment of path */
       struct fractpoint hint; /* accumulated hint value */
       char tempflag;        /* flag; is path temporary?                     */
       char Cflag;           /* flag; should we apply continuity?            */
 
       if (p == NULL)
               return(NULL);
/*
Establish the 'Cflag' continuity flag based on user's fill rule and
our own 'Continuity' pragmatic (0: never do continuity, 1: do what
user asked, >1: do it regardless).
*/
       if (fillrule > 0) {
               Cflag = Continuity > 0;
               fillrule -= CONTINUITY;
       }
       else
               Cflag = Continuity > 1;
 
       ARGCHECK((fillrule != WINDINGRULE && fillrule != EVENODDRULE),
                       "Interior: bad fill rule", NULL, NULL, (1,p), struct region *);
 
       if (p->type == TEXTTYPE)
/*             if (fillrule != EVENODDRULE)
               else */
                       return((struct region *)UniquePath(p));
       if (p->type == STROKEPATHTYPE) {
               if (fillrule == WINDINGRULE)
                       return((struct region *)DoStroke(p));
               else
                       p = CoercePath(p);
       }
 
       R = (struct region *)Allocate(sizeof(struct region), &EmptyRegion, 0);
 
       ARGCHECK(!ISPATHANCHOR(p), "Interior:  bad path", p, R, (0), struct region *);
       ARGCHECK((p->type != MOVETYPE), "Interior:  path not closed", p, R, (0), struct region *);
 
 
/* changed definition from !ISPERMANENT to references <= 1 3-26-91 PNM */
       tempflag =  (p->references <= 1); /* only first segment in path is so marked */
       if (!ISPERMANENT(p->flag)) p->references -= 1;
 
       R->newedgefcn = newfilledge;
/*
Believe it or not, "R" is now completely initialized.  We are counting
on the copy of template to get other fields the way we want them,
namely
:ol.
:li.anchor = NULL
:li.xmin, ymin, xmax, ymax, to minimum and maximum values respectively.
:eol.
Anchor = NULL is very
important to ChangeDirection.
See :hdref refid=CD..
 
To minimize problems of "wrapping" in our pel arithmetic, we keep an
origin of the region which is the first move.  Hopefully, that keeps
numbers within plus or minus 32K pels.
*/
       R->origin.x = 0/*TOFRACTPEL(NEARESTPEL(p->dest.x))*/;
       R->origin.y = 0/*TOFRACTPEL(NEARESTPEL(p->dest.y))*/;
       lastx = - R->origin.x;
       lasty = - R->origin.y;
/*
ChangeDirection initializes other important fields in R, such as
lastdy, edge, edgeYstop, edgexmin, and edgexmax.  The first segment
is a MOVETYPE, so it will be called first.
*/
/*
The hints data structure must be initialized once for each path.
*/
 
       if (ProcessHints)
               InitHints(); /* initialize hint data structure */
 
       while (p != NULL)  {
 
               x = lastx + p->dest.x;
               y = lasty + p->dest.y;
 
               nextP = p->link;
 
/*
Here we start the hints processing by initializing the hint value to
zero.  If ProcessHints is FALSE, the value will remain zero.
Otherwise, hint accumulates the computed hint values.
*/
 
               hint.x = hint.y = 0;
 
/*
If we are processing hints, and this is a MOVE segment (other than
the first on the path), we need to close (reverse) any open hints.
*/
 
               if (ProcessHints)
                       if ((p->type == MOVETYPE) && (p->last == NULL)) {
                               CloseHints(&hint);
                       }
 
/*
Next we run through all the hint segments (if any) attached to this
segment.  If ProcessHints is TRUE, we will accumulate computed hint
values.  In either case, nextP will be advanced to the first non-HINT
segment (or NULL), and each hint segment will be freed if necessary.
*/
 
               while ((nextP != NULL) && (nextP->type == HINTTYPE))  {
                       if (ProcessHints)
                               ProcessHint((struct hintsegment *)nextP, 
					   x + hint.x, y + hint.y, &hint);
 
                       {
                               register struct segment *saveP = nextP;
 
                               nextP = nextP->link;
                               if (tempflag)
                                       Free(saveP);
                       }
               }
 
/*
We now apply the full hint value to the ending point of the path segment.
*/
 
               x += hint.x;
               y += hint.y;
 
               switch(p->type) {
 
                   case LINETYPE:
                       StepLine(R, lastx, lasty, x, y);
                       break;
 
                   case CONICTYPE:
                   {
 
/*
For a conic curve, we apply half the hint value to the conic midpoint.
*/
 
                   }
                       break;
 
                   case BEZIERTYPE:
                   {
                       register struct beziersegment *bp = (struct beziersegment *) p;
 
/*
For a Bezier curve, we apply the full hint value to the Bezier C point.
*/
 
                       StepBezier(R, lastx, lasty,
                                 lastx + bp->B.x, lasty + bp->B.y,
                                 lastx + bp->C.x + hint.x,
                                 lasty + bp->C.y + hint.y,
                                 x, y);
                   }
                       break;
 
                   case MOVETYPE:
/*
At this point we have encountered a MOVE segment.  This breaks the
path, making it disjoint.
*/
                       if (p->last == NULL) /* i.e., not first in path */
                               ChangeDirection(CD_LAST, R, lastx, lasty, (fractpel) 0);
 
                       ChangeDirection(CD_FIRST, R, x, y, (fractpel) 0);
/*
We'll just double check for closure here.  We forgive an appended
MOVETYPE at the end of the path, if it isn't closed:
*/
                       if (!ISCLOSED(p->flag) && p->link != NULL)
                               return((struct region *)ArgErr("Fill: sub-path not closed", p, NULL));
                       break;
 
                   default:
                       Abort("Interior: path type error");
               }
/*
We're done with this segment.  Advance to the next path segment in
the list, freeing this one if necessary:
*/
               lastx = x;  lasty = y;
 
               if (tempflag)
                       Free(p);
               p = nextP;
       }
       ChangeDirection(CD_LAST, R, lastx, lasty, (fractpel) 0);
       R->ending.x = lastx;
       R->ending.y = lasty;
/*
Finally, clean up the region's based on the user's 'fillrule' request:
*/
       if (Cflag)
             ApplyContinuity(R);
       if (fillrule == WINDINGRULE)
             Unwind(R->anchor);
       return(R);
}
/*
:h3."workedge" Array
 
This is a statically allocated array where edges are built
before being copied into more permanent storage by NewEdge().
*/
 
#ifndef   MAXEDGE
#define   MAXEDGE     1000
#endif
 
static pel workedge[MAXEDGE];
static pel *currentworkarea = workedge;
static pel currentsize = MAXEDGE;
 
/*
:h3 id=cd.ChangeDirection() - Called When Y Direction Changes
 
The rasterizing routines call this entry point when they detect
a change in Y.  We then build the current edge and sort it into
emerging edgelist at 'anchor' by calling whatever "newedgefcn"
is appropriate.
*/
 
void 
ChangeDirection(int type,    /* CD_FIRST, CD_CONTINUE, or CD_LAST            */
		struct region *R, /* region in which we are changing direction */
		fractpel x, fractpel y, /* current beginning x,y             */
		fractpel dy) /* direction and magnitude of change in y       */
{
       register fractpel ymin,ymax;  /* minimum and maximum Y since last call */
       register pel iy;      /* nearest integer pel to 'y'                   */
       register pel idy;     /* nearest integer pel to 'dy'                  */
       register int ydiff;   /* allowed Y difference in 'currentworkarea'    */
 
       if (type != CD_FIRST) {
 
               if (R->lastdy > 0) {
                       ymin = R->firsty;
                       ymax = y;
               }
               else {
                       ymin = y;
                       ymax = R->firsty;
               }
 
               if (ymax < ymin)
                       Abort("negative sized edge?");
 
 
               (*R->newedgefcn)(R, R->edgexmin, R->edgexmax, ymin, ymax,
                                   R->lastdy > 0);
 
       }
 
       R->firsty = y;
       R->firstx = x;
       R->lastdy = dy;
 
       iy = NEARESTPEL(y);
       idy = NEARESTPEL(dy);
       if (currentworkarea != workedge && idy < MAXEDGE && idy > -MAXEDGE) {
               NonObjectFree(currentworkarea);
               currentworkarea = workedge;
               currentsize = MAXEDGE;
       }
       ydiff = currentsize - 1;
       if (dy > 0) {
               R->edge = &currentworkarea[-iy];
               R->edgeYstop = TOFRACTPEL(ydiff + iy) + FPHALF;
       }
       else {
               R->edge = &currentworkarea[ydiff - iy];
               R->edgeYstop = TOFRACTPEL(iy - ydiff) - FPHALF;
       }
       R->edgexmax = R->edgexmin = x;
/*
If this is the end of a subpath, we complete the subpath circular
chain:
*/
       if (type == CD_LAST && R->lastedge != NULL) {
               register struct edgelist *e = R->firstedge;
 
               while (e->subpath != NULL)
                       e = e->subpath;
               e->subpath = R->lastedge;
               R->lastedge = R->firstedge = NULL;
       }
}
/*
:h3 id=newfill.newfilledge() - Called When We Have a New Edge While Filling
 
This is the prototypical "newedge" function passed to "Rasterize" and
stored in "newedgefcn" in the region being built.
 
If the edge is non-null, we sort it onto the list of edges we are
building at "anchor".
 
This function also has to keep the bounding box of the region
up to date.
*/
 
static void 
newfilledge(struct region *R,  /* region being built                         */
	    fractpel xmin, fractpel xmax, /* X range of this edge            */
	    fractpel ymin, fractpel ymax, /* Y range of this edge            */
	    int isdown)      /* flag:  TRUE means edge goes down, else up    */
{
 
       register pel pelxmin,pelymin,pelxmax,pelymax;  /* pel versions of bounds */
       register struct edgelist *edge;  /* newly created edge                */
 
       pelymin = NEARESTPEL(ymin);
       pelymax = NEARESTPEL(ymax);
       if (pelymin == pelymax)
               return;
 
       pelxmin = NEARESTPEL(xmin);
       pelxmax = NEARESTPEL(xmax);
 
       if (pelxmin < R->xmin)  R->xmin = pelxmin;
       if (pelxmax > R->xmax)  R->xmax = pelxmax;
       if (pelymin < R->ymin)  R->ymin = pelymin;
       if (pelymax > R->ymax)  R->ymax = pelymax;
 
       edge = NewEdge(pelxmin, pelxmax, pelymin, pelymax, &R->edge[pelymin], isdown);
       edge->subpath = R->lastedge;
       R->lastedge = edge;
       if (R->firstedge == NULL)
               R->firstedge = edge;
 
       R->anchor = SortSwath(R->anchor, edge, swathxsort);
 
}
 
/*
:h2.Sorting Edges
 
:h3.SortSwath() - Vertically Sort an Edge into a Region
 
This routine sorts an edge or a pair of edges into a growing region,
so that the region maintains its top-to-bottom, left-to-right form.
The rules for sorting horizontally may vary depending on what you
are doing, but the rules for vertical sorting are always the same.
This routine is passed an argument that is a function that will
perform the horizontal sort on demand (for example, swathxsort() or
SwathUnion()).
 
This is a recursive routine.  A new edge (or edge pair) may overlap
the list I am building in strange and wonderful ways.  Edges may
cross.  When this happens, my strategy is to split the incoming edge
(or the growing list) in two at that point, execute the actual sort on
the top part of the split, and recursively call myself to figure out
exactly where the bottom part belongs.
*/
 
#define   TOP(e)      ((e)->ymin)  /* the top of an edge (for readability    */
#define   BOTTOM(e)   ((e)->ymax)  /* the bottom of an edge (for readability */
 
struct edgelist *
SortSwath(struct edgelist *anchor,   /* list being built                     */
	  struct edgelist *edge,     /* incoming edge or pair of edges       */
	  SwathFunc swathfcn)        /* horizontal sorter                    */
{
       register struct edgelist *before,*after;
       struct edgelist base;
 
       if (anchor == NULL)
               return(edge);
 
       before = &base;
       before->ymin = before->ymax = MINPEL;
       before->link = after = anchor;
 
/*
If the incoming edge is above the current list, we connect the current
list to the bottom of the incoming edge.  One slight complication is
if the incoming edge overlaps into the current list.  Then, we
first split the incoming edge in two at the point of overlap and recursively
call ourselves to sort the bottom of the split into the current list:
*/
       if (TOP(edge) < TOP(after)) {
               if (BOTTOM(edge) > TOP(after)) {
 
                       after = SortSwath(after, splitedge(edge, TOP(after)), swathfcn);
               }
               vertjoin(edge, after);
               return(edge);
       }
/*
At this point the top of edge is not higher than the top of the list,
which we keep in 'after'.  We move the 'after' point down the list,
until the top of the edge occurs in the swath beginning with 'after'.
 
If the bottom of 'after' is below the bottom of the edge, we have to
split the 'after' swath into two parts, at the bottom of the edge.
If the bottom of 'after' is above the bottom of the swath,
*/
 
       while (VALIDEDGE(after)) {
 
               if (TOP(after) == TOP(edge)) {
                       if (BOTTOM(after) > BOTTOM(edge))
                               vertjoin(after, splitedge(after, BOTTOM(edge)));
                       else if (BOTTOM(after) < BOTTOM(edge)) {
                               after = SortSwath(after,
                                     splitedge(edge, BOTTOM(after)), swathfcn);
                       }
                       break;
               }
               else if (TOP(after) > TOP(edge)) {
                       if (BOTTOM(edge) > TOP(after)) {
                               after = SortSwath(after,
                                         splitedge(edge, TOP(after)), swathfcn);
                       }
                       break;
               }
               else if (BOTTOM(after) > TOP(edge))
                       vertjoin(after, splitedge(after, TOP(edge)));
 
               before = after;
               after = after->link;
       }
 
/*
At this point 'edge' exactly corresponds in height to the current
swath pointed to by 'after'.
*/
       if (after != NULL && TOP(after) == TOP(edge)) {
               before = (*swathfcn)(before, edge);
               after = before->link;
       }
/*
At this point 'after' contains all the edges after 'edge', and 'before'
contains all the edges before.  Whew!  A simple matter now of adding
'edge' to the linked list in its rightful place:
*/
       before->link = edge;
       if (RegionDebug > 1) {
               while (edge->link != NULL) {
                       edge = edge->link;
               }
       }
       else
               for (; edge->link != NULL; edge = edge->link) { ; }
 
       edge->link = after;
       return(base.link);
}
 
/*
:h3.splitedge() - Split an Edge or Swath in Two at a Given Y Value
 
This function returns the edge or swath beginning at the Y value, and
is guaranteed not to change the address of the old swath while splitting
it.
*/
 
static struct edgelist *
splitedge(struct edgelist *list, /* area to split                            */
	  pel y)             /* Y value to split list at                     */
{
       register struct edgelist *new;  /* anchor for newly built list        */
       register struct edgelist *last = NULL;  /* end of newly built list    */
       register struct edgelist *r;  /* temp pointer to new structure        */
       register struct edgelist *lastlist;  /* temp pointer to last 'list' value */
 
       lastlist = new = NULL;
 
       while (list != NULL) {
               if (y < list->ymin)
                       break;
               if (y >= list->ymax)
                       Abort("splitedge: above top of list");
               if (y == list->ymin)
                       Abort("splitedge: would be null");
 
               r = (struct edgelist *)Allocate(sizeof(struct edgelist), list, 0);
/*
At this point 'r' points to a copy of the single structure at 'list'.
We will make 'r' be the new split 'edgelist'--the lower half.
We don't bother to correct 'xmin' and 'xmax', we'll take the
the pessimistic answer that results from using the old values.
*/
               r->ymin = y;
               r->xvalues = list->xvalues + (y - list->ymin);
/*
Note that we do not need to allocate new memory for the X values,
they can remain with the old "edgelist" structure.  We do have to
update that old structure so it is not as high:
*/
               list->ymax = y;
/*
Insert 'r' in the subpath chain:
*/
               r->subpath = list->subpath;
               list->subpath = r;
/*
Now attach 'r' to the list we are building at 'new', and advance
'list' to point to the next element in the old list:
*/
               if (new == NULL)
                       new = r;
               else
                       last->link = r;
               last = r;
               lastlist = list;
               list = list->link;
       }
/*
At this point we have a new list built at 'new'.  We break the old
list at 'lastlist', and add the broken off part to the end of 'new'.
Then, we return the caller a pointer to 'new':
*/
       if (new == NULL)
               Abort("null splitedge");
       lastlist->link = NULL;
       last->link = list;
       return(new);
}
 
/*
:h3.vertjoin() - Join Two Disjoint Edge Lists Vertically
 
The two edges must be disjoint vertically.
*/
static void vertjoin(top, bottom)
       register struct edgelist *top;  /* uppermost region                   */
       register struct edgelist *bottom;  /* bottommost region               */
{
       if (BOTTOM(top) > TOP(bottom))
               Abort("vertjoin not disjoint");
 
       for (; top->link != NULL; top=top->link) { ; }
 
       top->link = bottom;
       return;
}
 
/*
:h3.swathxsort() - Sorting by X Values
 
We need to sort 'edge' into its rightful
place in the swath by X value, taking care that we do not accidentally
advance to the next swath while searching for the correct X value.  Like
all swath functions, this function returns a pointer to the edge
BEFORE the given edge in the sort.
*/
 
static struct edgelist *
swathxsort(struct edgelist *before0,  /* edge before this swath              */
	   struct edgelist *edge)     /* input edge                          */
{
       register struct edgelist *before;
       register struct edgelist *after;
       register pel y = 0;
 
       before = before0;
       after = before->link;
 
       while (after != NULL && TOP(after) == TOP(edge)) {
 
               register pel *x1,*x2;
 
               y = TOP(edge);
               x1 = after->xvalues;
               x2 = edge->xvalues;
 
               while (y < BOTTOM(edge) && *x1 == *x2) {
                       x1++; x2++; y++;
               }
               if (y >= BOTTOM(edge)) {
                       edge->flag |= ISAMBIGUOUS(ON);
                       after->flag |= ISAMBIGUOUS(ON);
                       break;
               }
 
               if (*x1 >= *x2)
                       break;
 
               before = after;
               after = after->link;
       }
 
/*
At this point, 'edge' is between 'before' and 'after'.  If 'edge' didn't
cross either of those other edges, we would be done.  We check for
crossing.  If it does cross, we split the problem up by calling SortSwath
recursively with the part of the edge that is below the crossing point:
*/
{
       register int h0,h;    /* height of edge--number of scans              */
 
       h0 = h = BOTTOM(edge) - y;
       y -= TOP(edge);
 
       if (h0 <= 0) {
               return(before);
       }
 
       if (TOP(before) == TOP(edge))
               h -= crosses(h, &before->xvalues[y], &edge->xvalues[y]);
       if (after != NULL && TOP(after) == TOP(edge))
               h -= crosses(h, &edge->xvalues[y], &after->xvalues[y]);
 
       if (h < h0) {
               SortSwath(before0->link,
                         splitedge(edge, TOP(edge) + y + h),
                         swathxsort);
 
       }
}
 
       return(before);
}
/*
:h3.SwathUnion() - Union Two Edges by X Value
 
We have a left and right edge that must be unioned into a growing
swath.  If they are totally disjoint, they are just added in.  The
fun comes in they overlap the existing edges.  Then some edges
will disappear.
*/
 
struct edgelist *
SwathUnion(struct edgelist *before0,   /* edge before the swath              */
	   struct edgelist *edge)      /* list of two edges to be unioned    */
{
       register int h;       /* saves height of edge                         */
       register struct edgelist *rightedge;  /* saves right edge of 'edge'   */
       register struct edgelist *before,*after;  /* edge before and after    */
       int h0;               /* saves initial height                         */
 
       h0 = h = edge->ymax - edge->ymin;
       if (h <= 0)
               Abort("SwathUnion:  0 height swath?");
 
       before = before0;
       after = before->link;
 
       while (after != NULL && TOP(after) == TOP(edge)) {
               register struct edgelist *right;
 
               right = after->link;
               if (right->xvalues[0] >= edge->xvalues[0])
                       break;
               before = right;
               after = before->link;
       }
/*
This is the picture at this point.  'L' indicates a left hand edge,
'R' indicates the right hand edge.
'<--->' indicates the degree of uncertainty as to its placement
relative to other edges:
:xmp atomic.
   before           after
     R            <---L---->  R             L   R    L   R
              <---L--->  <------R-------------------------->
                 edge
:exmp.
In case the left of 'edge' touches 'before', we need to reduce
the height by that amount.
*/
       if (TOP(before) == TOP(edge))
               h -= touches(h, before->xvalues, edge->xvalues);
 
       rightedge = edge->link;
 
       if (after == NULL || TOP(after) != TOP(edge) ||
                   after->xvalues[0] > rightedge->xvalues[0]) {
/*
On this side of the the above 'if', the new edge is disjoint from the
existing edges in the swath.  This is the picture:
:xmp atomic.
   before           after
     R                L    R     L   R    L   R
              L    R
             edge
:exmp.
We will verify it remains disjoint for the entire height.  If the
situation changes somewhere down the edge, we split the edge at that
point and recursively call ourselves (through 'SortSwath') to figure
out the new situation:
*/
               if (after != NULL && TOP(after) == TOP(edge))
                       h -= touches(h, rightedge->xvalues, after->xvalues);
               if (h < h0)
                       SortSwath(before0->link, splitedge(edge, edge->ymin + h), t1_SwathUnion);
               /* go to "return" this edge pair; it is totally disjoint */
       }
       else {
/*
At this point, at the 'else', we know that the
new edge overlaps one or more pairs in the existing swath.  Here is
a picture of our knowledge and uncertainties:
:xmp atomic.
   before       after
     R            L        R     L   R    L   R
            <---L--->   <---R------------------->
               edge
:exmp.
We need to move 'after' along until it is to the right of the
right of 'edge'.  ('After' should always point to a left edge of a pair:)
*/
               register struct edgelist *left;  /* variable to keep left edge in */
 
               do {
                        left = after;
                        after = (after->link)->link;
 
               } while  (after != NULL && TOP(after) == TOP(edge)
                               && after->xvalues[0] <= rightedge->xvalues[0]);
/*
At this point this is the picture:
:xmp atomic.
   before                 left        after
     R            L    R   L      R     L   R
            <---L--->        <---R--->
               edge
:exmp.
We need to verify that the situation stays like this all the way
down the edge.  Again, if the
situation changes somewhere down the edge, we split the edge at that
point and recursively call ourselves (through 'SortSwath') to figure
out the new situation:
*/
 
               h -= crosses(h, left->xvalues, rightedge->xvalues);
               h -= crosses(h, edge->xvalues, ((before->link)->link)->xvalues);
 
               if (after != NULL && TOP(after) == TOP(edge))
 
                       h -= touches(h, rightedge->xvalues, after->xvalues);
 
/*
OK, if we touched either of our neighbors we need to split at that point
and recursively sort the split edge onto the list.  One tricky part
is that when we recursively sort, 'after' will change if it was not
in our current swath:
*/
               if (h < h0) {
                       SortSwath(before0->link,
                                 splitedge(edge, edge->ymin + h),
                                 t1_SwathUnion);
 
                       if (after == NULL || TOP(after) != TOP(edge))
                                 for (after = before0->link;
                                      TOP(after) == TOP(edge);
                                      after = after->link) { ; }
               }
/*
Now we need to augment 'edge' by the left and right of the overlapped
swath, and to discard all edges between before and after, because they
were overlapped and have been combined with the new incoming 'edge':
*/
               edge->xmin = MIN(edge->xmin, (before->link)->xmin);
               edge->xmax = MIN(edge->xmax, (before->link)->xmax);
               edgemin(h, edge->xvalues, (before->link)->xvalues);
               rightedge->xmin = MAX(rightedge->xmin, (left->link)->xmin);
               rightedge->xmax = MAX(rightedge->xmax, (left->link)->xmax);
               edgemax(h, rightedge->xvalues, (left->link)->xvalues);
               discard(before, after);
       }
       return(before);
}
#ifdef notused
/*
:h3.swathrightmost() - Simply Sorts New Edge to Rightmost of Swath
 
Like all swath functions, this function returns a pointer to the edge
BEFORE the given edge in the sort.
*/
 
static struct edgelist *
swathrightmost(struct edgelist *before,  /* edge before this swath           */
	       struct edgelist *edge)    /* input edge                       */
{
       register struct edgelist *after;
 
       after = before->link;
 
       while (after != NULL && TOP(after) == TOP(edge)) {
               before = after;
               after = after->link;
       }
 
       return(before);
 
}
#endif
/*
:h3.touches() - Returns the Remaining Height When Two Edges Touch
 
So, it will return 0 if they never touch.  Allows incredibly(?) mnemonic
if (touches(...)) construct.
*/
 
static int 
touches(int h, pel *left, pel *right)
{
       for (; h > 0; h--)
               if (*left++ >= *right++)
                       break;
       return(h);
}
/*
:h3.crosses() - Returns the Remaining Height When Two Edges Cross
 
So, it will return 0 if they never cross.
*/
 
static int 
crosses(int h, pel *left, pel *right)
{
       for (; h > 0; h--)
               if (*left++ > *right++)
                       break;
       return(h);
}
/*
:h3.cedgemin() - Stores the Mininum of an Edge and an X Value
*/
 
static void 
cedgemin(int h, pel *e1, pel x)
{
       for (; --h >= 0; e1++)
               if (*e1 > x)
                       *e1 = x;
}
/*
:h3.cedgemax() - Stores the Maximum of an Edge and an X Value
*/
 
static void 
cedgemax(int h, pel *e1, pel x)
{
       for (; --h >= 0; e1++)
               if (*e1 < x)
                       *e1 = x;
}
/*
:h3.edgemin() - Stores the Mininum of Two Edges in First Edge
*/
 
static void 
edgemin(int h, pel *e1, pel *e2)
{
       for (; --h >= 0; e1++,e2++)
               if (*e1 > *e2)
                       *e1 = *e2;
}
/*
:h3.edgemax() - Stores the Maximum of Two Edges in First Edge
*/
 
static void 
edgemax(int h, pel *e1, pel *e2)
{
       for (; --h >= 0; e1++,e2++)
               if (*e1 < *e2)
                       *e1 = *e2;
}
 
/*
:h2.Changing the Representation of Regions
 
For convenience and/or performance, we sometimes like to change the way
regions are represented.  This does not change the object itself, just
the representation, so these transformations can be made on a permanent
region.
 
*/
 
void
MoveEdges(struct region *R,      /* region to modify                         */
	  fractpel dx, fractpel dy) /* delta X and Y to move edge list by    */
{
       register struct edgelist *edge;  /* for looping through edges         */
 
       R->origin.x += dx;
       R->origin.y += dy;
       R->ending.x += dx;
       R->ending.y += dy;
       if (R->thresholded != NULL) {
               R->thresholded->origin.x -= dx;
               R->thresholded->origin.y -= dy;
       }
/*
From now on we will deal with dx and dy as integer pel values:
*/
       dx = NEARESTPEL(dx);
       dy = NEARESTPEL(dy);
       if (dx == 0 && dy == 0)
               return;
 
       R->xmin += dx;
       R->xmax += dx;
       R->ymin += dy;
       R->ymax += dy;
 
       for (edge = R->anchor; VALIDEDGE(edge); edge = edge->link) {
               edge->ymin += dy;
               edge->ymax += dy;
               if (dx != 0) {
                       register int h;  /* loop index; height of edge        */
                       register pel *Xp;  /* loop pointer to X values        */
 
                       edge->xmin += dx;
                       edge->xmax += dx;
                       for (Xp = edge->xvalues, h = edge->ymax - edge->ymin;
                                     --h >= 0; )
                               *Xp++ += dx;
               }
       }
}
 
/*
:h3.UnJumble() - Sort a Region Top to Bottom
 
It is an open question whether it pays in general to do this.
*/
 
void 
UnJumble(struct region *region) /* region to sort                            */
{
       register struct edgelist *anchor;  /* new lists built here            */
       register struct edgelist *edge;  /* edge pointer for loop             */
       register struct edgelist *next;  /* ditto                             */
 
       anchor = NULL;
 
       for (edge=region->anchor; VALIDEDGE(edge); edge=next) {
               if (edge->link == NULL)
                       Abort("UnJumble:  unpaired edge?");
               next = edge->link->link;
               edge->link->link = NULL;
               anchor = SortSwath(anchor, edge, t1_SwathUnion);
       }
 
       if (edge != NULL)
               vertjoin(anchor, edge);
 
       region->anchor = anchor;
       region->flag &= ~ISJUMBLED(ON);
}
 
/*
*/
#ifdef notused 
static void
OptimizeRegion(struct region *R) /* region to optimize                       */
{
       register pel *xP;     /* pel pointer for inner loop                   */
       register int x;       /* holds X value                                */
       register int xmin,xmax;  /* holds X range                             */
       register int h;       /* loop counter                                 */
       register struct edgelist *e;  /* edgelist pointer for loop            */
 
       R->flag |= ISRECTANGULAR(ON);
 
       for (e = R->anchor; VALIDEDGE(e); e=e->link) {
               xmin = MAXPEL;
               xmax = MINPEL;
               for (h = e->ymax - e->ymin, xP = e->xvalues; --h >= 0;) {
                         x = *xP++;
                         if (x < xmin)  xmin = x;
                         if (x > xmax)  xmax = x;
               }
               if (xmin != xmax || (xmin != R->xmin && xmax != R->xmax))
                       R->flag &= ~ISRECTANGULAR(ON);
               if (xmin < e->xmin || xmax > e->xmax)
                       Abort("Tighten: existing edge bound was bad");
               if (xmin < R->xmin || xmax > R->xmax)
                       Abort("Tighten: existing region bound was bad");
               e->xmin = xmin;
               e->xmax = xmax;
       }
       R->flag |= ISOPTIMIZED(ON);
}
#endif

/*
:h2.Miscelaneous Routines
 
:h3.MoreWorkArea() - Allocate New Space for "edge"
 
Our strategy is to temporarily allocate an array to hold this
unexpectedly large edge.  ChangeDirection frees this array any time
it gets a shorter 'dy'.
*/
 
/*ARGSUSED*/
void 
MoreWorkArea(struct region *R, /* region we are generating                   */
	     fractpel x1, fractpel y1, /* starting point of line             */
	     fractpel x2, fractpel y2) /* ending point of line               */
{
       register int idy;     /* integer dy of line                           */
 
       idy = NEARESTPEL(y1) - NEARESTPEL(y2);
       if (idy < 0)  idy = - idy;
 
       /*
       * we must add one to the delta for the number of run ends we
       * need to store:
       */
       if (++idy > currentsize) {
               if (currentworkarea != workedge)
                       NonObjectFree(currentworkarea);
               currentworkarea = (pel *)Allocate(0, NULL, idy * sizeof(pel));
               currentsize = idy;
       }
       ChangeDirection(CD_CONTINUE, R, x1, y1, y2 - y1);
}
 
/*
:h3.BoxClip() - Clip a Region to a Rectangle
 
BoxClip also duplicates the region if it is permanent.  Note the
clipping box is specified in REGION coordinates, that is, in
coordinates relative to the region (0,0) point
*/
 
struct region *
BoxClip(struct region *R,       /* region to clip                            */
	pel xmin, pel ymin,     /* upper left hand corner of rectangle       */
	pel xmax, pel ymax)     /* lower right hand corner                   */
{
       struct edgelist anchor;  /* pretend edgelist to facilitate discards   */
       register struct edgelist *e,*laste;
 
       R = UniqueRegion(R);
 
       if (xmin > R->xmin) {
               R->xmin = xmin;
       }
       if (xmax < R->xmax) {
               R->xmax = xmax;
       }
 
       if (ymin > R->ymin) {
               R->ymin = ymin;
       }
       if (ymax < R->ymax) {
               R->ymax = ymax;
       }
 
 
       laste = &anchor;
       anchor.link = R->anchor;
 
       for (e = R->anchor; VALIDEDGE(e); e = e->link) {
               if (TOP(e) < ymin) {
                       e->xvalues += ymin - e->ymin;
                       e->ymin = ymin;
               }
               if (BOTTOM(e) > ymax)
                       e->ymax = ymax;
               if (TOP(e) >= BOTTOM(e)) {
                       discard(laste, e->link->link);
                       e = laste;
                       continue;
               }
               if (e->xmin < xmin) {
                       cedgemax(BOTTOM(e) - TOP(e), e->xvalues, xmin);
                       e->xmin = xmin;
                       e->xmax = MAX(e->xmax, xmin);
               }
               if (e->xmax > xmax) {
                       cedgemin(BOTTOM(e) - TOP(e), e->xvalues, xmax);
                       e->xmin = MIN(e->xmin, xmax);
                       e->xmax = xmax;
               }
               laste = e;
       }
 
       R->anchor = anchor.link;
 
       return(R);
}
 
#ifdef notdef
/*
:h3.CoerceRegion() - Force a TextPath Structure to Become a Region
 
We also save the newly created region in the textpath structure, if the
structure was permanent.  Then we don't have to do this again.  Why not
save it all the time?  Well, we certainly could, but I suspect it
wouldn't pay.  We would have to make this region permanent (because we
couldn't have it be consumed) and this would probably require
unnecessary CopyRegions in most cases.
*/
 
struct region *
CoerceRegion(struct textpath *tp) /* input TEXTTYPE                          */
{
       struct segment *path; /* temporary character path                     */
       struct region *R;     /* returned region                              */
 
 
       R = Interior(path, EVENODDRULE);
       return(R);
}
#endif
 
/*
:h3.RegionBounds() - Returns Bounding Box of a Region
*/
 
struct segment *
RegionBounds(struct region *R)
{
       register struct segment *path;  /* returned path                      */
 
       path = BoxPath(IDENTITY, R->ymax - R->ymin, R->xmax - R->xmin);
       path = Join(PathSegment(MOVETYPE, R->origin.x + TOFRACTPEL(R->xmin),
                                         R->origin.y + TOFRACTPEL(R->ymin) ),
                   path);
       return(path);
}
 
/*
:h2.Formatting/Dump Routines for Debug
 
:h3.DumpArea() - Display a Region
*/
void 
DumpArea(struct region *area)
{
       DumpEdges(area->anchor);
}
 
#define  INSWATH(p, y0, y1)  (p != NULL && p->ymin == y0 && p->ymax == y1)
/*
:h3.DumpEdges() - Display Run End Lists (Edge Lists)
*/

/*
:h3.edgecheck() - For Debug, Verify that an Edge Obeys the Rules
*/
 
/*ARGSUSED*/
static void
edgecheck(struct edgelist *edge, int oldmin, int oldmax)
{
       if (edge->type != EDGETYPE)
               Abort("EDGE ERROR: non EDGETYPE in list");
/*
The following check is not valid if the region is jumbled so I took it
out:
*/
/*     if (edge->ymin < oldmax && edge->ymin != oldmin)
               Abort("EDGE ERROR: overlapping swaths"); */
}
 
void 
DumpEdges(struct edgelist *edges)
{
       register struct edgelist *p,*p2;
       register pel ymin = MINPEL;
       register pel ymax = MINPEL;
 
       if (edges == NULL) {
               return;
       }
       if (RegionDebug <= 1) {
               for (p=edges; p != NULL; p = p->link) {
                       edgecheck(p, ymin, ymax);
                       ymin = p->ymin;  ymax = p->ymax;
               }
       }
       else {
 
               for (p2=edges; p2 != NULL; ) {
 
                       edgecheck(p2, ymin, ymax);
                       ymin = p2->ymin;
                       ymax = p2->ymax;
 
                       while (INSWATH(p2, ymin, ymax))
                               p2 = p2->link;
               }
       }
}
