/* $XConsortium: regions.h,v 1.6 94/04/02 15:49:51 rws Exp $ */
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
 
#define   Interior(p,rule)        t1_Interior(p,rule)
#define   Union(a1,a2)            t1_Union(a1,a2)
#define   Intersect(a1,a2)        t1_Intersect(a1,a2)
#define   Complement(area)        t1_Complement(area)
#define   Overlap(a1,a2)          t1_OverLap(a1,a2)
 
struct region *t1_Interior(); /* returns the interior of a closed path        */
struct region *t1_Union();   /* set union of paths or regions                */
struct region *t1_Intersect();  /* set intersection of regions                */
struct region *t1_Complement();  /* complement of a region                    */
int t1_Overlap();             /* returns a Boolean; TRUE if regions overlap   */
 
#define   INFINITY    t1_Infinity
 
/*END SHARED*/
/*SHARED*/
 
#define   ChangeDirection(type,R,x,y,dy)  t1_ChangeDirection(type,R,x,y,dy)
 
void t1_ChangeDirection();    /* called when we change direction in Y         */
#define   CD_FIRST         -1  /* enumeration of ChangeDirection type       */
#define   CD_CONTINUE       0  /* enumeration of ChangeDirection type        */
#define   CD_LAST           1  /* enumeration of ChangeDirection type        */
 
#define    MoreWorkArea(R,x1,y1,x2,y2)     t1_MoreWorkArea(R,x1,y1,x2,y2)
#define    KillRegion(area)   t1_KillRegion(area)
#define    CopyRegion(area)   t1_CopyRegion(area)
#define    BoxClip(R,xmin,ymin,xmax,ymax)  t1_BoxClip(R,xmin,ymin,xmax,ymax)
#define    SortSwath(a,p,f)   t1_SortSwath(a,p,f)
#define    SwathUnion(b,e)    t1_SwathUnion(b,e)
#define    RegionBounds(r)    t1_RegionBounds(r)
#define    CoerceRegion(p)    t1_CoerceRegion(p)
#define    MoveEdges(R,dx,dy) t1_MoveEdges(R,dx,dy)
#define    UnJumble(R)        t1_UnJumble(R)
 
void t1_MoreWorkArea();       /* get longer edge list for stepping            */
struct region *t1_CopyRegion();  /* duplicate a region                       */
void t1_KillRegion();         /* destroy a region                             */
struct region *t1_BoxClip();  /* clip a region to a rectangle                 */
struct edgelist *t1_SortSwath();  /* sort edges onto growing edge list        */
struct edgelist *t1_SwathUnion();  /* 'union' two edges into a swath          */
struct segment *t1_RegionBounds();  /* returns bounding box of a region       */
struct region *t1_CoerceRegion();  /* force text to become a true region      */
void t1_MoveEdges();          /* moves the edge values in a region            */
void t1_UnJumble();           /* sort the edges and reset the jumbled flag    */
 
/*END SHARED*/
/*SHARED*/
 
#define GOING_TO(R, x1, y1, x2, y2, dy) { \
   if (dy < 0) { \
      if (R->lastdy >= 0) \
          ChangeDirection(CD_CONTINUE, R, x1, y1, dy); \
      if (y2 < R->edgeYstop) \
          MoreWorkArea(R, x1, y1, x2, y2); \
   } \
   else if (dy > 0) { \
      if (R->lastdy <= 0) \
          ChangeDirection(CD_CONTINUE, R, x1, y1, dy); \
      if (y2 > R->edgeYstop) \
          MoreWorkArea(R, x1, y1, x2, y2); \
   } \
   else /* dy == 0 */ ChangeDirection(CD_CONTINUE, R, x1, y1, dy); \
   if (x2 < R->edgexmin) R->edgexmin = x2; \
   else if (x2 > R->edgexmax) R->edgexmax = x2; \
}
 
#ifndef __sxg__
#include <limits.h>
#endif
#ifdef SHRT_MIN
#define MINPEL SHRT_MIN
#else
#define MINPEL ((pel)(-1<<(8*sizeof(pel)-1)))  /* smallest value fitting in a pel */
#endif
#ifdef SHRT_MAX
#define MAXPEL SHRT_MAX
#else
#define MAXPEL ((pel)((1<<(8*sizeof(pel)-1))-1))/* largest value fitting in a pel */
#endif
 
/*
The "Unique"-type macro is different (unique?) for regions, because some
regions structures are shared among several objects, and might have
to be made unique for that reason (i.e., references > 1).
*/
 
#define    ConsumeRegion(R)   MAKECONSUME(R,KillRegion(R))
#define    UniqueRegion(R)    MAKEUNIQUE(R,CopyRegion(R))
 
 
/*END SHARED*/
/*SHARED*/
 
struct region {
       XOBJ_COMMON           /* xobject common data define 3-26-91 PNM    */
                             /* type = REGIONTYPE                         */
       struct fractpoint origin;    /* beginning handle:  X,Y origin of region      */
       struct fractpoint ending;    /* ending handle:  X,Y change after painting region */
       pel xmin,ymin;        /* minimum X,Y of region                        */
       pel xmax,ymax;        /* mat1_mum X,Y of region                        */
       struct edgelist *anchor;  /* list of edges that bound the region      */
       struct picture *thresholded;  /* region defined by thresholded picture*/
/*
Note that the ending handle and the bounding box values are stored
relative to 'origin'.
 
The above elements describe a region.  The following elements are
scratchpad areas used while the region is being built:
*/
       fractpel lastdy;      /* direction of last segment                    */
       fractpel firstx,firsty;    /* starting point of current edge          */
       fractpel edgexmin,edgexmax;  /* x extent of current edge              */
       struct edgelist *lastedge,*firstedge;  /* last and first edges in subpath */
       pel *edge;            /* pointer to array of X values for edge        */
       fractpel edgeYstop;   /* Y value where 'edges' array ends             */
       void (*newedgefcn)();  /* function to use when building a new edge    */
       struct strokeinfo *strokeinfo;  /* scratchpad info during stroking only */
} ;
/*
The ISCOMPLEMENT flag indicates the region is reversed--it is the
"outside" of the nominal region.
*/
#define   ISCOMPLEMENT(flag)   ((flag)&0x80)
/*
The ISJUMBLED flag indicates the region is not sorted top-to-bottom.
*/
#define   ISJUMBLED(flag)      ((flag)&0x40)
/*
The ISINFINITE flag allows a quick check for an INFINITE region, which
is frequently intersected.
*/
#define   ISINFINITE(flag)     ((flag)&0x20)
 
/*END SHARED*/
/*SHARED*/
 
#define   ISRECTANGULAR(flag)  ((flag)&0x08)
 
/*END SHARED*/
/*SHARED*/
 
#define  EmptyRegion   t1_EmptyRegion
 
/*END SHARED*/
/*SHARED*/
 
struct edgelist {
       XOBJ_COMMON          /* xobject common data define 3-26-91 PNM        */
                            /* type = EDGETYPE                               */
       struct edgelist *link;  /* pointer to next in linked list             */
       struct edgelist *subpath;  /* informational link for "same subpath"   */
       pel xmin,xmax;        /* range of edge in X                           */
       pel ymin,ymax;        /* range of edge in Y                           */
       pel *xvalues;         /* pointer to ymax-ymin X values                */
} ;
/*
The end of the list is marked by either "link" being NULL, or by
ymin == ymax.  See :hdref refid=discard..  We define the VALIDEDGE
predicate to test for the opposite of these conditions:
*/
 
#define   VALIDEDGE(p)    ((p)!=NULL&&(p)->ymin<(p)->ymax)
 
/*END SHARED*/
/*SHARED*/
 
#define   ISDOWN(f)       ((f)&0x80)
 
#define   ISAMBIGUOUS(f)  ((f)&0x40)
 
/*END SHARED*/
/*SHARED*/
 
/*
Interior() rule enumerations:
*/
#define   WINDINGRULE -2
#define   EVENODDRULE -3
 
#define   CONTINUITY  0x80   /* can be added to above rules; e.g. WINDINGRULE+CONTINUITY */
 
/*END SHARED*/
