/* $XConsortium: migc.c,v 1.4 94/04/17 20:27:36 dpw Exp $ */
/*

Copyright (c) 1993  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/


#include "scrnintstr.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "migc.h"

/* This structure has to line up with the mfb and cfb gc private structures so
 * that when it is superimposed on them, the three fields that migc.c needs to
 * see will be accessed correctly.  I know this is not beautiful, but it seemed
 * better than all the code duplication in cfb and mfb.
 */
typedef struct {
    unsigned char       pad1;
    unsigned char       pad2;
    unsigned char       pad3;
    unsigned		pad4:1;
    unsigned		freeCompClip:1;
    PixmapPtr		pRotatedPixmap;
    RegionPtr		pCompositeClip;
} miPrivGC;

static int miGCPrivateIndex;

void
miRegisterGCPrivateIndex(gcindex)
    int gcindex;
{
    miGCPrivateIndex = gcindex;
}

/* ARGSUSED */
void
miChangeGC(pGC, mask)
    GCPtr           pGC;
    unsigned long   mask;
{
    return;
}

void
miDestroyGC(pGC)
    GCPtr           pGC;
{
    miPrivGC       *pPriv;

    pPriv = (miPrivGC *) (pGC->devPrivates[miGCPrivateIndex].ptr);
    if (pPriv->pRotatedPixmap)
	(*pGC->pScreen->DestroyPixmap) (pPriv->pRotatedPixmap);
    if (pPriv->freeCompClip)
	REGION_DESTROY(pGC->pScreen, pPriv->pCompositeClip);
    miDestroyGCOps(pGC->ops);
}

/*
 * create a private op array for a gc
 */

GCOpsPtr
miCreateGCOps(prototype)
    GCOpsPtr        prototype;
{
    GCOpsPtr        ret;
    extern Bool     Must_have_memory;

     /* XXX */ Must_have_memory = TRUE;
    ret = (GCOpsPtr) xalloc(sizeof(GCOps));
     /* XXX */ Must_have_memory = FALSE;
    if (!ret)
	return 0;
    *ret = *prototype;
    ret->devPrivate.val = 1;
    return ret;
}

void
miDestroyGCOps(ops)
    GCOpsPtr        ops;
{
    if (ops->devPrivate.val)
	xfree(ops);
}


void
miDestroyClip(pGC)
    GCPtr           pGC;
{
    if (pGC->clientClipType == CT_NONE)
	return;
    else if (pGC->clientClipType == CT_PIXMAP)
    {
	(*pGC->pScreen->DestroyPixmap) ((PixmapPtr) (pGC->clientClip));
    }
    else
    {
	/*
	 * we know we'll never have a list of rectangles, since ChangeClip
	 * immediately turns them into a region
	 */
	REGION_DESTROY(pGC->pScreen, pGC->clientClip);
    }
    pGC->clientClip = NULL;
    pGC->clientClipType = CT_NONE;
}

void
miChangeClip(pGC, type, pvalue, nrects)
    GCPtr           pGC;
    int             type;
    pointer         pvalue;
    int             nrects;
{
    (*pGC->funcs->DestroyClip) (pGC);
    if (type == CT_PIXMAP)
    {
	/* convert the pixmap to a region */
	pGC->clientClip = (pointer) BITMAP_TO_REGION(pGC->pScreen,
							(PixmapPtr) pvalue);
	(*pGC->pScreen->DestroyPixmap) (pvalue);
    }
    else if (type == CT_REGION)
    {
	/* stuff the region in the GC */
	pGC->clientClip = pvalue;
    }
    else if (type != CT_NONE)
    {
	pGC->clientClip = (pointer) RECTS_TO_REGION(pGC->pScreen, nrects,
						      (xRectangle *) pvalue,
								    type);
	xfree(pvalue);
    }
    pGC->clientClipType = (type != CT_NONE && pGC->clientClip) ? CT_REGION : CT_NONE;
    pGC->stateChanges |= GCClipMask;
}

void
miCopyClip(pgcDst, pgcSrc)
    GCPtr           pgcDst, pgcSrc;
{
    RegionPtr       prgnNew;

    switch (pgcSrc->clientClipType)
    {
      case CT_PIXMAP:
	((PixmapPtr) pgcSrc->clientClip)->refcnt++;
	/* Fall through !! */
      case CT_NONE:
	(*pgcDst->funcs->ChangeClip) (pgcDst, (int) pgcSrc->clientClipType,
				   pgcSrc->clientClip, 0);
	break;
      case CT_REGION:
	prgnNew = REGION_CREATE(pgcSrc->pScreen, NULL, 1);
	REGION_COPY(pgcDst->pScreen, prgnNew,
					(RegionPtr) (pgcSrc->clientClip));
	(*pgcDst->funcs->ChangeClip) (pgcDst, CT_REGION, (pointer) prgnNew, 0);
	break;
    }
}

/* ARGSUSED */
void
miCopyGC(pGCSrc, changes, pGCDst)
    GCPtr           pGCSrc;
    unsigned long   changes;
    GCPtr           pGCDst;
{
    return;
}

void
miComputeCompositeClip(pGC, pDrawable)
    GCPtr           pGC;
    DrawablePtr     pDrawable;
{
    ScreenPtr       pScreen = pGC->pScreen;
    miPrivGC *devPriv = (miPrivGC *) (pGC->devPrivates[miGCPrivateIndex].ptr);

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	WindowPtr       pWin = (WindowPtr) pDrawable;
	RegionPtr       pregWin;
	Bool            freeTmpClip, freeCompClip;

	if (pGC->subWindowMode == IncludeInferiors)
	{
	    pregWin = NotClippedByChildren(pWin);
	    freeTmpClip = TRUE;
	}
	else
	{
	    pregWin = &pWin->clipList;
	    freeTmpClip = FALSE;
	}
	freeCompClip = devPriv->freeCompClip;

	/*
	 * if there is no client clip, we can get by with just keeping the
	 * pointer we got, and remembering whether or not should destroy (or
	 * maybe re-use) it later.  this way, we avoid unnecessary copying of
	 * regions.  (this wins especially if many clients clip by children
	 * and have no client clip.)
	 */
	if (pGC->clientClipType == CT_NONE)
	{
	    if (freeCompClip)
		REGION_DESTROY(pScreen, devPriv->pCompositeClip);
	    devPriv->pCompositeClip = pregWin;
	    devPriv->freeCompClip = freeTmpClip;
	}
	else
	{
	    /*
	     * we need one 'real' region to put into the composite clip. if
	     * pregWin the current composite clip are real, we can get rid of
	     * one. if pregWin is real and the current composite clip isn't,
	     * use pregWin for the composite clip. if the current composite
	     * clip is real and pregWin isn't, use the current composite
	     * clip. if neither is real, create a new region.
	     */

	    REGION_TRANSLATE(pScreen, pGC->clientClip,
					 pDrawable->x + pGC->clipOrg.x,
					 pDrawable->y + pGC->clipOrg.y);

	    if (freeCompClip)
	    {
		REGION_INTERSECT(pGC->pScreen, devPriv->pCompositeClip,
					    pregWin, pGC->clientClip);
		if (freeTmpClip)
		    REGION_DESTROY(pScreen, pregWin);
	    }
	    else if (freeTmpClip)
	    {
		REGION_INTERSECT(pScreen, pregWin, pregWin, pGC->clientClip);
		devPriv->pCompositeClip = pregWin;
	    }
	    else
	    {
		devPriv->pCompositeClip = REGION_CREATE(pScreen, NullBox, 0);
		REGION_INTERSECT(pScreen, devPriv->pCompositeClip,
				       pregWin, pGC->clientClip);
	    }
	    devPriv->freeCompClip = TRUE;
	    REGION_TRANSLATE(pScreen, pGC->clientClip,
					 -(pDrawable->x + pGC->clipOrg.x),
					 -(pDrawable->y + pGC->clipOrg.y));
	}
    }	/* end of composite clip for a window */
    else
    {
	BoxRec          pixbounds;

	/* XXX should we translate by drawable.x/y here ? */
	pixbounds.x1 = 0;
	pixbounds.y1 = 0;
	pixbounds.x2 = pDrawable->width;
	pixbounds.y2 = pDrawable->height;

	if (devPriv->freeCompClip)
	{
	    REGION_RESET(pScreen, devPriv->pCompositeClip, &pixbounds);
	}
	else
	{
	    devPriv->freeCompClip = TRUE;
	    devPriv->pCompositeClip = REGION_CREATE(pScreen, &pixbounds, 1);
	}

	if (pGC->clientClipType == CT_REGION)
	{
	    REGION_TRANSLATE(pScreen, devPriv->pCompositeClip,
					 -pGC->clipOrg.x, -pGC->clipOrg.y);
	    REGION_INTERSECT(pScreen, devPriv->pCompositeClip,
				devPriv->pCompositeClip, pGC->clientClip);
	    REGION_TRANSLATE(pScreen, devPriv->pCompositeClip,
					 pGC->clipOrg.x, pGC->clipOrg.y);
	}
    }	/* end of composite clip for pixmap */
} /* end miComputeCompositeClip */
