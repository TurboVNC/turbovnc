/* $XFree86: xc/programs/Xserver/mfb/mfbbitblt.c,v 1.7tsi Exp $ */
/* Combined Purdue/PurduePlus patches, level 2.0, 1/17/89 */
/***********************************************************

Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $Xorg: mfbbitblt.c,v 1.4 2001/02/09 02:05:18 xorgcvs Exp $ */
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xprotostr.h>

#include "regionstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "mi.h"

#include "mfb.h"
#include "maskbits.h"


/* CopyArea and CopyPlane for a monchrome frame buffer


    clip the source rectangle to the source's available bits.  (this
avoids copying unnecessary pieces that will just get exposed anyway.)
this becomes the new shape of the destination.
    clip the destination region to the composite clip in the
GC.  this requires translating the destination region to (dstx, dsty).
    build a list of source points, one for each rectangle in the
destination.  this is a simple translation.
    go do the multiple rectangle copies
    do graphics exposures
*/
/** Optimized for drawing pixmaps into windows, especially when drawing into
 ** unobscured windows.  Calls to the general-purpose region code were
 ** replaced with rectangle-to-rectangle clipping comparisions.  This is
 ** possible, since the pixmap is a single rectangle.  In an unobscured
 ** window, the destination clip is also a single rectangle, and region
 ** code can be avoided entirely.  This is a big savings, since the region
 ** code uses XAlloc() and makes many function calls.
 **
 ** In addition, if source is a pixmap, there is no need to call the
 ** expensive miHandleExposures() routine.  Instead, we simply return NULL.
 **
 ** Previously, drawing a pixmap into an unobscured window executed at least
 ** 8 XAlloc()'s, 30 function calls, and hundreds of lines of code.
 **
 ** Now, the same operation requires no XAlloc()'s, no region function calls,
 ** and much less overhead.  Nice for drawing lots of small pixmaps.
 */
 

void
mfbDoBitblt (pSrc, pDst, alu, prgnDst, pptSrc)
    DrawablePtr	    pSrc, pDst;
    int		    alu;
    RegionPtr	    prgnDst;
    DDXPointPtr	    pptSrc;
{
    switch (alu)
    {
    case GXcopy:
	mfbDoBitbltCopy (pSrc, pDst, alu, prgnDst, pptSrc);
	break;
    case GXxor:
	mfbDoBitbltXor (pSrc, pDst, alu, prgnDst, pptSrc);
	break;
    case GXcopyInverted:
	mfbDoBitbltCopyInverted (pSrc, pDst, alu, prgnDst, pptSrc);
	break;
    case GXor:
	mfbDoBitbltOr (pSrc, pDst, alu, prgnDst, pptSrc);
	break;
    default:
	mfbDoBitbltGeneral (pSrc, pDst, alu, prgnDst, pptSrc);
	break;
    }
}

RegionPtr
mfbCopyArea(pSrcDrawable, pDstDrawable,
	    pGC, srcx, srcy, width, height, dstx, dsty)
register DrawablePtr pSrcDrawable;
register DrawablePtr pDstDrawable;
register GC *pGC;
int srcx, srcy;
int width, height;
int dstx, dsty;
{
    RegionPtr prgnSrcClip = NULL; /* may be a new region, or just a copy */
    Bool freeSrcClip = FALSE;

    RegionPtr prgnExposed;
    RegionRec rgnDst;
    DDXPointPtr pptSrc;
    register DDXPointPtr ppt;
    register BoxPtr pbox;
    int i;
    register int dx;
    register int dy;
    xRectangle origSource;
    DDXPointRec origDest;
    int numRects;
    BoxRec fastBox;
    int fastClip = 0;		/* for fast clipping with pixmap source */
    int fastExpose = 0;		/* for fast exposures with pixmap source */
    void (*localDoBitBlt)(
	DrawablePtr /*pSrc*/,
	DrawablePtr /*pDst*/,
	int /*alu*/,
	RegionPtr /*prgnDst*/,
	DDXPointPtr /*pptSrc*/);

    origSource.x = srcx;
    origSource.y = srcy;
    origSource.width = width;
    origSource.height = height;
    origDest.x = dstx;
    origDest.y = dsty;

    if ((pSrcDrawable != pDstDrawable) &&
	pSrcDrawable->pScreen->SourceValidate)
    {
	(*pSrcDrawable->pScreen->SourceValidate) (pSrcDrawable, srcx, srcy, width, height);
    }

    switch (pGC->alu) {
    case GXcopy:
	localDoBitBlt = mfbDoBitbltCopy;
	break;
    case GXcopyInverted:
	localDoBitBlt = mfbDoBitbltCopyInverted;
	break;
    case GXxor:
	localDoBitBlt = mfbDoBitbltXor;
	break;
    case GXor:
	localDoBitBlt = mfbDoBitbltOr;
	break;
    default:
	localDoBitBlt = mfbDoBitbltGeneral;
	break;
    }

    srcx += pSrcDrawable->x;
    srcy += pSrcDrawable->y;

    /* clip the source */

    if (pSrcDrawable->type == DRAWABLE_PIXMAP)
    {
	if ((pSrcDrawable == pDstDrawable) &&
	    (pGC->clientClipType == CT_NONE))
	{
	    prgnSrcClip = pGC->pCompositeClip;
	}
	else
	{
	    fastClip = 1;
	}
    }
    else
    {
	if (pGC->subWindowMode == IncludeInferiors)
	{
	    if (!((WindowPtr) pSrcDrawable)->parent)
	    {
		/*
		 * special case bitblt from root window in
		 * IncludeInferiors mode; just like from a pixmap
		 */
		fastClip = 1;
	    }
	    else if ((pSrcDrawable == pDstDrawable) &&
		(pGC->clientClipType == CT_NONE))
	    {
		prgnSrcClip = pGC->pCompositeClip;
	    }
	    else
	    {
		prgnSrcClip = NotClippedByChildren((WindowPtr)pSrcDrawable);
		freeSrcClip = TRUE;
	    }
	}
	else
	{
	    prgnSrcClip = &((WindowPtr)pSrcDrawable)->clipList;
	}
    }

    fastBox.x1 = srcx;
    fastBox.y1 = srcy;
    fastBox.x2 = srcx + width;
    fastBox.y2 = srcy + height;

    /* Don't create a source region if we are doing a fast clip */
    if (fastClip)
    {
	fastExpose = 1;
	/*
	 * clip the source; if regions extend beyond the source size,
 	 * make sure exposure events get sent
	 */
	if (fastBox.x1 < pSrcDrawable->x)
	{
	    fastBox.x1 = pSrcDrawable->x;
	    fastExpose = 0;
	}
	if (fastBox.y1 < pSrcDrawable->y)
	{
	    fastBox.y1 = pSrcDrawable->y;
	    fastExpose = 0;
	}
	if (fastBox.x2 > pSrcDrawable->x + (int) pSrcDrawable->width)
	{
	    fastBox.x2 = pSrcDrawable->x + (int) pSrcDrawable->width;
	    fastExpose = 0;
	}
	if (fastBox.y2 > pSrcDrawable->y + (int) pSrcDrawable->height)
	{
	    fastBox.y2 = pSrcDrawable->y + (int) pSrcDrawable->height;
	    fastExpose = 0;
	}
    }
    else
    {
	REGION_INIT(pGC->pScreen, &rgnDst, &fastBox, 1);
	REGION_INTERSECT(pGC->pScreen, &rgnDst, &rgnDst, prgnSrcClip);
    }

    dstx += pDstDrawable->x;
    dsty += pDstDrawable->y;

    if (pDstDrawable->type == DRAWABLE_WINDOW)
    {
	if (!((WindowPtr)pDstDrawable)->realized)
	{
	    if (!fastClip)
		REGION_UNINIT(pGC->pScreen, &rgnDst);
	    if (freeSrcClip)
		REGION_DESTROY(pGC->pScreen, prgnSrcClip);
	    return NULL;
	}
    }

    dx = srcx - dstx;
    dy = srcy - dsty;

    /* Translate and clip the dst to the destination composite clip */
    if (fastClip)
    {
	RegionPtr cclip;

        /* Translate the region directly */
        fastBox.x1 -= dx;
        fastBox.x2 -= dx;
        fastBox.y1 -= dy;
        fastBox.y2 -= dy;

	/* If the destination composite clip is one rectangle we can
	   do the clip directly.  Otherwise we have to create a full
	   blown region and call intersect */
	cclip = pGC->pCompositeClip;
        if (REGION_NUM_RECTS(cclip) == 1)
        {
	    BoxPtr pBox = REGION_RECTS(cclip);

	    if (fastBox.x1 < pBox->x1) fastBox.x1 = pBox->x1;
	    if (fastBox.x2 > pBox->x2) fastBox.x2 = pBox->x2;
	    if (fastBox.y1 < pBox->y1) fastBox.y1 = pBox->y1;
	    if (fastBox.y2 > pBox->y2) fastBox.y2 = pBox->y2;

	    /* Check to see if the region is empty */
	    if (fastBox.x1 >= fastBox.x2 || fastBox.y1 >= fastBox.y2)
	    {
		REGION_NULL(pGC->pScreen, &rgnDst);
	    }
	    else
	    {
		REGION_INIT(pGC->pScreen, &rgnDst, &fastBox, 1);
	    }
	}
        else
	{
	    /* We must turn off fastClip now, since we must create
	       a full blown region.  It is intersected with the
	       composite clip below. */
	    fastClip = 0;
	    REGION_INIT(pGC->pScreen, &rgnDst, &fastBox, 1);
	}
    }
    else
    {
        REGION_TRANSLATE(pGC->pScreen, &rgnDst, -dx, -dy);
    }

    if (!fastClip)
    {
	REGION_INTERSECT(pGC->pScreen, &rgnDst, &rgnDst, pGC->pCompositeClip);
    }

    /* Do bit blitting */
    numRects = REGION_NUM_RECTS(&rgnDst);
    if (numRects && width && height)
    {
	if(!(pptSrc = (DDXPointPtr)ALLOCATE_LOCAL(numRects *
						  sizeof(DDXPointRec))))
	{
	    REGION_UNINIT(pGC->pScreen, &rgnDst);
	    if (freeSrcClip)
		REGION_DESTROY(pGC->pScreen, prgnSrcClip);
	    return NULL;
	}
	pbox = REGION_RECTS(&rgnDst);
	ppt = pptSrc;
	for (i = numRects; --i >= 0; pbox++, ppt++)
	{
	    ppt->x = pbox->x1 + dx;
	    ppt->y = pbox->y1 + dy;
	}
    
	if (pGC->planemask & 1)
	    (*localDoBitBlt) (pSrcDrawable, pDstDrawable, pGC->alu, &rgnDst, pptSrc);

	DEALLOCATE_LOCAL(pptSrc);
    }

    prgnExposed = NULL;
    if (pGC->fExpose) 
    {
        /* Pixmap sources generate a NoExposed (we return NULL to do this) */
        if (!fastExpose)
	    prgnExposed =
		miHandleExposures(pSrcDrawable, pDstDrawable, pGC,
				  origSource.x, origSource.y,
				  (int)origSource.width,
				  (int)origSource.height,
				  origDest.x, origDest.y, (unsigned long)0);
    }
    REGION_UNINIT(pGC->pScreen, &rgnDst);
    if (freeSrcClip)
	REGION_DESTROY(pGC->pScreen, prgnSrcClip);
    return prgnExposed;
}


/*
 * Devices which use mfb for 1-bit pixmap support
 * must register a function for n-to-1 copy operations
 */

static unsigned long	copyPlaneGeneration;
static int		copyPlaneScreenIndex = -1;

Bool
mfbRegisterCopyPlaneProc (pScreen, proc)
    ScreenPtr	pScreen;
    RegionPtr	(*proc)(
        DrawablePtr         /* pSrcDrawable */,
        DrawablePtr         /* pDstDrawable */,
        GCPtr               /* pGC */,
        int                 /* srcx */,
        int                 /* srcy */,
        int                 /* width */,
        int                 /* height */,
        int                 /* dstx */,
        int                 /* dsty */,
        unsigned long       /* bitPlane */);
{
    if (copyPlaneGeneration != serverGeneration)
    {
	copyPlaneScreenIndex = AllocateScreenPrivateIndex();
	if (copyPlaneScreenIndex < 0)
	    return FALSE;
	copyPlaneGeneration = serverGeneration;
    }
    pScreen->devPrivates[copyPlaneScreenIndex].fptr = proc;
    return TRUE;
}

/*
    if fg == 1 and bg ==0, we can do an ordinary CopyArea.
    if fg == bg, we can do a CopyArea with alu = mfbReduceRop(alu, fg)
    if fg == 0 and bg == 1, we use the same rasterop, with
	source operand inverted.

    CopyArea deals with all of the graphics exposure events.
    This code depends on knowing that we can change the
alu in the GC without having to call ValidateGC() before calling
CopyArea().

*/


RegionPtr
mfbCopyPlane(pSrcDrawable, pDstDrawable,
	    pGC, srcx, srcy, width, height, dstx, dsty, plane)
DrawablePtr pSrcDrawable, pDstDrawable;
register GC *pGC;
int srcx, srcy;
int width, height;
int dstx, dsty;
unsigned long plane;
{
    int alu;
    RegionPtr	prgnExposed;
    RegionPtr	(*copyPlane)(
        DrawablePtr         /* pSrcDrawable */,
        DrawablePtr         /* pDstDrawable */,
        GCPtr               /* pGC */,
        int                 /* srcx */,
        int                 /* srcy */,
        int                 /* width */,
        int                 /* height */,
        int                 /* dstx */,
        int                 /* dsty */,
        unsigned long       /* bitPlane */);


    if (pSrcDrawable->depth != 1)
    {
	if (copyPlaneScreenIndex >= 0 &&
	    (copyPlane =
		pSrcDrawable->pScreen->devPrivates[copyPlaneScreenIndex].fptr)
	    )
	{
	    return (*copyPlane) (pSrcDrawable, pDstDrawable,
			   pGC, srcx, srcy, width, height, dstx, dsty, plane);
	}  
	else
	{
	    FatalError ("No copyPlane proc registered for depth %d\n",
			pSrcDrawable->depth);
	}
    }
    if (plane != 1)
	return NULL;

    if ((pGC->fgPixel & 1) == 1 && (pGC->bgPixel & 1) == 0)
    {
	prgnExposed = (*pGC->ops->CopyArea)(pSrcDrawable, pDstDrawable,
			 pGC, srcx, srcy, width, height, dstx, dsty);
    }
    else if ((pGC->fgPixel & 1) == (pGC->bgPixel & 1))
    {
	alu = pGC->alu;
	pGC->alu = mfbReduceRop(pGC->alu, pGC->fgPixel);
	prgnExposed = (*pGC->ops->CopyArea)(pSrcDrawable, pDstDrawable,
			 pGC, srcx, srcy, width, height, dstx, dsty);
	pGC->alu = alu;
    }
    else /* need to invert the src */
    {
	alu = pGC->alu;
	pGC->alu = InverseAlu[alu];
	prgnExposed = (*pGC->ops->CopyArea)(pSrcDrawable, pDstDrawable,
			 pGC, srcx, srcy, width, height, dstx, dsty);
	pGC->alu = alu;
    }
    return prgnExposed;
}

