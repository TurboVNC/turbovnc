/* $Xorg: lines.c,v 1.3 2000/08/17 19:46:30 cpqbld Exp $ */
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
/* $XFree86: xc/lib/font/Type1/lines.c,v 1.5 2003/05/27 22:26:45 tsi Exp $ */

 /* LINES    CWEB         V0003 ********                             */
/*
:h1.LINES Module - Rasterizing Lines
 
&author. Duaine W. Pryor, Jr. and Jeffrey B. Lotspiech (lotspiech@almaden.ibm.com)
 
 
:h3.Include Files
 
The included files are:
*/
 
#include "objects.h"
#include "spaces.h"
#include "paths.h"
#include "regions.h"
#include "lines.h"
 
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
:h2.StepLine() - Produces Run Ends for a Line After Checks
 
The main work is done by Bresenham(); here we just perform checks and
get the line so that its Y direction is always increasing:
*/
 
void StepLine(R, x1, y1, x2, y2)
       register struct region *R;  /* region being built                     */
       register fractpel x1,y1;  /* starting point                           */
       register fractpel x2,y2;  /* ending point                             */
{
       register fractpel dy;
 
       dy = y2 - y1;
 
/*
We execute the "GOING_TO" macro to call back the REGIONS module, if
necessary (like if the Y direction of the edge has changed):
*/
       GOING_TO(R, x1, y1, x2, y2, dy);
 
       if (dy == 0)
               return;
 
       if (dy < 0)
               Bresenham(R->edge, x2, y2, x1, y1);
       else
               Bresenham(R->edge, x1, y1, x2, y2);
       return;
}
/*
:h3.Bresenham() - Actually Produces Run Ends
 
This routine runs a Bresenham line-stepping
algorithm.  See, for example, Newman and Sproul, :hp1/Principles
of Interactive Computer Graphics/, pp. 25-27.
When we enter this, we
are guaranteed that dy is positive.
We'd like to work in 8 bit precision, so we'll define some macros and
constants to let us do that:
*/
 
#define PREC 8               /* we'll keep fraction pels in 8 bit precision  */
/*
RoundFP() rounds down by 'b' bits:
*/
#define  RoundFP(xy,b)   (((xy)+(1<<((b)-1)))>>(b))
 
/*
TruncFP() truncates down by 'b' bits:
*/
#define  TruncFP(xy,b)   ((xy)>>(b))
 
 
void Bresenham(edgeP,x1,y1,x2,y2)
       register pel *edgeP;               /* pointer to top of list (y == 0) */
       register fractpel x1,y1;           /* starting point on line          */
       register fractpel x2,y2;           /* ending point on the line (down) */
{
       register long dx,dy;  /* change in x and y, in my own precision       */
       register long x,y;    /* integer pel starting point                   */
       register int count;   /* integer pel delta y                          */
       register long d;      /* the Bresenham algorithm error term           */
 
       x1 = TruncFP(x1, FRACTBITS-PREC);
       y1 = TruncFP(y1, FRACTBITS-PREC);
       x2 = TruncFP(x2, FRACTBITS-PREC);
       y2 = TruncFP(y2, FRACTBITS-PREC);
 
       dx = x2 - x1;
       dy = y2 - y1;
/*
Find the starting x and y integer pel coordinates:
*/
 
 x = RoundFP(x1,PREC);
 y = RoundFP(y1,PREC);
 edgeP += y;
 count = RoundFP(y2,PREC) - y;
/*------------------------------------------------------------------*/
/* Force dx to be positive so that dfy will be negative             */
/*       this means that vertical moves will decrease d             */
/*------------------------------------------------------------------*/
 if (dx<0)
 {
  dx = -dx;
#define P PREC
  d=(dy*(x1-(x<<P)+(1<<(P-1)))-dx*((y<<P)-y1+(1<<(P-1))))>>P;
#undef P
  while(--count >= 0 )
  {
   while(d<0)
   {
    --x;
    d += dy;
   }
   *(edgeP++) = x;
   d -= dx;
  }
 }
 else  /* positive dx */
 {
#define P PREC
  d = (dy*((x<<P)-x1+(1<<(P-1)))-dx*((y<<P)-y1+(1<<(P-1))))>>P;
#undef P
  while(--count >= 0 )
  {
   while(d<0)
   {
    ++x;
    d += dy;
   }
   *(edgeP++) = x;
   d -= dx;
  }
 }
}
