/* $Xorg: curves.c,v 1.3 2000/08/17 19:46:29 cpqbld Exp $ */
/* Copyright International Business Machines,Corp. 1991              */
/* All Rights Reserved                                               */
 
/* License to use, copy, modify, and distribute this software        */
/* and its documentation for any purpose and without fee is          */
/* hereby granted, provided that licensee provides a license to      */
/* IBM, Corp. to use, copy, modify, and distribute derivative        */
/* works and their documentation for any purpose and without         */
/* fee, that the above copyright notice appear in all copies         */
/* and that both that copyright notice and this permission           */
/* notice appear in supporting documentation, and that the name      */
/* of IBM not be used in advertising or publicity pertaining to      */
/* distribution of the software without specific, written prior      */
/* permission.                                                       */
 
/* IBM PROVIDES THIS SOFTWARE "AS IS", WITHOUT ANY WARRANTIES        */
/* OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT        */
/* LIMITED TO ANY IMPLIED WARRANTIES OF MERCHANTABILITY,             */
/* FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT OF          */
/* THIRD PARTY RIGHTS.  THE ENTIRE RISK AS TO THE QUALITY AND        */
/* PERFORMANCE OF THE SOFTWARE, INCLUDING ANY DUTY TO SUPPORT        */
/* OR MAINTAIN, BELONGS TO THE LICENSEE.  SHOULD ANY PORTION OF      */
/* THE SOFTWARE PROVE DEFECTIVE, THE LICENSEE (NOT IBM) ASSUMES      */
/* THE ENTIRE COST OF ALL SERVICING, REPAIR AND CORRECTION.  IN      */
/* NO EVENT SHALL IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR         */
/* CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING         */
/* FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF        */
/* CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT        */
/* OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS           */
/* SOFTWARE.                                                         */
/* $XFree86: xc/lib/font/Type1/curves.c,v 1.7 2001/08/27 19:49:52 dawes Exp $ */

/*
:h1.CURVES Module - Stepping Beziers
 
This module is responsible for "rasterizing"
third order curves.  That is, it changes the high level curve
specification into a list of pels that that curve travels
through.
 
:h3.Include Files
 
Include files needed:
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
#include "curves.h"
#include "lines.h"
#include "arith.h"
 
 
/*
:h3.Functions Provided to Other Modules
 
External entry points:
*/
/*SHARED LINE(S) ORIGINATED HERE*/
 
/*
Note that "stepping" and "flattening" are so similiar that they use the
same routine.  When the "region" parameter is NULL, that is a flag that
we are flattening instead of stepping.
*/
/*
:h2.Bezier Third Order Curves
*/
/*
:h3.The "bezierinfo" Structure
 
This structure is used to store information used when we subdivide
Bezier curves.
*/
 
struct bezierinfo {
       struct region *region;  /* the region being built or NULL             */
       struct fractpoint last;  /* not used yet; maybe could save some work  */
       struct fractpoint origin;  /* the origin of the bezier                */
} ;
 
/*
   Checking for termination of the subdivision process:
   This is the stupidest test in the world, just check if the coordinatewise
   distance from an end control point to the next control point is less than
   one half pel.   If so, we must be done.
   This returns 1 if the subdivision is terminated and 0 if you still need
   to subdivide.
*/
 
static int 
BezierTerminationTest(fractpel xa, fractpel ya,
		      fractpel xb, fractpel yb,
		      fractpel xc, fractpel yc,
		      fractpel xd, fractpel yd)
{
  fractpel dmax;
  dmax =          ABS(xa - xb);
  dmax = MAX(dmax,ABS(ya - yb));
  dmax = MAX(dmax,ABS(xd - xc));
  dmax = MAX(dmax,ABS(yd - yc));
  if(dmax > FPHALF)
    return(0); /* not done yet */
  else
    return(1); /* done */
}
 
/*
:h3.StepBezierRecurse() - The Recursive Logic in StepBezier()
 
The recursion involves dividing the control polygon into two smaller
control polygons by finding the midpoints of the lines.  This idea is
described in any graphics text book and its simplicity is what caused
Bezier to define his curves as he did.  If the input region 'R' is NULL,
the result is a path that is the 'flattened' curve; otherwise StepBezier
returns nothing special.
*/
static struct segment *
StepBezierRecurse(struct bezierinfo *I, /* Region under construction or NULL */
		  fractpel xA, fractpel yA, /* A control point               */
		  fractpel xB, fractpel yB, /* B control point               */
		  fractpel xC, fractpel yC, /* C control point               */
		  fractpel xD, fractpel yD) /* D control point               */
{
 if (BezierTerminationTest(xA,yA,xB,yB,xC,yC,xD,yD))
 {
  if (I->region == NULL)
   return(PathSegment(LINETYPE, xD - xA, yD - yA));
  else
   StepLine(I->region, I->origin.x + xA, I->origin.y + yA,
                       I->origin.x + xD, I->origin.y + yD);
 }
 else
 {
  fractpel xAB,yAB;
  fractpel xBC,yBC;
  fractpel xCD,yCD;
  fractpel xABC,yABC;
  fractpel xBCD,yBCD;
  fractpel xABCD,yABCD;
 
  xAB = xA + xB;  yAB = yA + yB;
  xBC = xB + xC;  yBC = yB + yC;
  xCD = xC + xD;  yCD = yC + yD;
 
  xABC = xAB + xBC;  yABC = yAB + yBC;
  xBCD = xBC + xCD;  yBCD = yBC + yCD;
 
  xABCD = xABC + xBCD;  yABCD = yABC + yBCD;
 
  xAB >>= 1;   yAB >>= 1;
  xBC >>= 1;   yBC >>= 1;
  xCD >>= 1;   yCD >>= 1;
  xABC >>= 2;   yABC >>= 2;
  xBCD >>= 2;   yBCD >>= 2;
  xABCD >>= 3;   yABCD >>= 3;
 
  if (I->region == NULL)
  {
   return( Join(
    StepBezierRecurse(I, xA, yA, xAB, yAB, xABC, yABC, xABCD, yABCD),
    StepBezierRecurse(I, xABCD, yABCD, xBCD, yBCD, xCD, yCD, xD, yD)
                )
         );
  }
  else
  {
   StepBezierRecurse(I, xA, yA, xAB, yAB, xABC, yABC, xABCD, yABCD);
   StepBezierRecurse(I, xABCD, yABCD, xBCD, yBCD, xCD, yCD, xD, yD);
  }
 }
 return NULL;
 /*NOTREACHED*/
}
 
/*
:h3.TOOBIG() - Macro to Test if a Coordinate is Too Big to Bezier SubDivide Normally
 
Intermediate values in the Bezier subdivision are 8 times bigger than
the starting values.  If this overflows, a 'long', we are in trouble:
*/
 
#undef   BITS
#define  BITS         (sizeof(long)*8)
#define  HIGHTEST(p)  (((p)>>(BITS-4)) != 0)  /* includes sign bit */
#define  TOOBIG(xy)   ((xy < 0) ? HIGHTEST(-xy) : HIGHTEST(xy))
 
/*
:h3.StepBezier() - Produce Run Ends for a Bezier Curve
 
This is the entry point called from outside the module.
*/
 
struct segment *
StepBezier(struct region *R, /* Region under construction or NULL            */
	   fractpel xA, fractpel yA, /* A control point                      */
	   fractpel xB, fractpel yB, /* B control point                      */
	   fractpel xC, fractpel yC, /* C control point                      */
	   fractpel xD, fractpel yD) /* D control point                      */
{
       struct bezierinfo Info;
 
       Info.region = R;
       Info.origin.x = xA;
       Info.origin.y = yA;
 
       xB -= xA;
       xC -= xA;
       xD -= xA;
       yB -= yA;
       yC -= yA;
       yD -= yA;
 
       if ( TOOBIG(xB) || TOOBIG(yB) || TOOBIG(xC) || TOOBIG(yC)
            || TOOBIG(xD) || TOOBIG(yD) )
               Abort("Beziers this big not yet supported");
 
       return(StepBezierRecurse(&Info,
                                (fractpel) 0, (fractpel) 0, xB, yB, xC, yC, xD, yD));
}
 
