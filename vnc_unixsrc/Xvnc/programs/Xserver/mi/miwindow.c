/***********************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


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
/* $XConsortium: miwindow.c,v 5.16 94/04/17 20:28:03 dpw Exp $ */
#include "X.h"
#include "miscstruct.h"
#include "region.h"
#include "mi.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "mivalidate.h"

void 
miClearToBackground(pWin, x, y, w, h, generateExposures)
    WindowPtr pWin;
    int x,y;
    int w,h;
    Bool generateExposures;
{
    BoxRec box;
    RegionRec	reg;
    RegionPtr pBSReg = NullRegion;
    ScreenPtr	pScreen;
    BoxPtr  extents;
    int	    x1, y1, x2, y2;

    /* compute everything using ints to avoid overflow */

    x1 = pWin->drawable.x + x;
    y1 = pWin->drawable.y + y;
    if (w)
        x2 = x1 + (int) w;
    else
        x2 = x1 + (int) pWin->drawable.width - (int) x;
    if (h)
        y2 = y1 + h;	
    else
        y2 = y1 + (int) pWin->drawable.height - (int) y;

    extents = &pWin->clipList.extents;
    
    /* clip the resulting rectangle to the window clipList extents.  This
     * makes sure that the result will fit in a box, given that the
     * screen is < 32768 on a side.
     */

    if (x1 < extents->x1)
	x1 = extents->x1;
    if (x2 > extents->x2)
	x2 = extents->x2;
    if (y1 < extents->y1)
	y1 = extents->y1;
    if (y2 > extents->y2)
	y2 = extents->y2;

    if (x2 <= x1 || y2 <= y1)
    {
	x2 = x1 = 0;
	y2 = y1 = 0;
    }

    box.x1 = x1;
    box.x2 = x2;
    box.y1 = y1;
    box.y2 = y2;

    pScreen = pWin->drawable.pScreen;
    REGION_INIT(pScreen, &reg, &box, 1);
    if (pWin->backStorage)
    {
	/*
	 * If the window has backing-store on, call through the
	 * ClearToBackground vector to handle the special semantics
	 * (i.e. things backing store is to be cleared out and
	 * an Expose event is to be generated for those areas in backing
	 * store if generateExposures is TRUE).
	 */
	pBSReg = (* pScreen->ClearBackingStore)(pWin, x, y, w, h,
						 generateExposures);
    }

    REGION_INTERSECT(pScreen, &reg, &reg, &pWin->clipList);
    if (generateExposures)
	(*pScreen->WindowExposures)(pWin, &reg, pBSReg);
    else if (pWin->backgroundState != None)
        (*pScreen->PaintWindowBackground)(pWin, &reg, PW_BACKGROUND);
    REGION_UNINIT(pScreen, &reg);
    if (pBSReg)
	REGION_DESTROY(pScreen, pBSReg);
}

/*
 * For SaveUnders using backing-store. The idea is that when a window is mapped
 * with saveUnder set TRUE, any windows it obscures will have its backing
 * store turned on setting the DIXsaveUnder bit,
 * The backing-store code must be written to allow for this
 */

/*-
 *-----------------------------------------------------------------------
 * miCheckSubSaveUnder --
 *	Check all the inferiors of a window for coverage by saveUnder
 *	windows. Called from ChangeSaveUnder and CheckSaveUnder.
 *	This code is very inefficient.
 *
 * Results:
 *	TRUE if any windows need to have backing-store removed.
 *
 * Side Effects:
 *	Windows may have backing-store turned on or off.
 *
 *-----------------------------------------------------------------------
 */
static Bool
miCheckSubSaveUnder(pParent, pFirst, pRegion)
    register WindowPtr	pParent;	/* Parent to check */
    WindowPtr		pFirst;		/* first reconfigured window */
    RegionPtr		pRegion;	/* Initial area obscured by saveUnder */
{
    register WindowPtr	pChild;		/* Current child */
    register ScreenPtr	pScreen;	/* Screen to use */
    RegionRec		SubRegion;	/* Area of children obscured */
    Bool		res = FALSE;	/* result */
    Bool		subInited=FALSE;/* SubRegion initialized */

    pScreen = pParent->drawable.pScreen;
    if ( (pChild = pParent->firstChild) )
    {
	/*
	 * build region above first changed window
	 */

	for (; pChild != pFirst; pChild = pChild->nextSib)
	    if (pChild->viewable && pChild->saveUnder)
		REGION_UNION(pScreen, pRegion, pRegion, &pChild->borderSize);
	
	/*
	 * check region below and including first changed window
	 */

	for (; pChild; pChild = pChild->nextSib)
	{
	    if (pChild->viewable)
	    {
		/*
		 * don't save under nephew/niece windows;
		 * use a separate region
		 */

		if (pChild->firstChild)
		{
		    if (!subInited)
		    {
			REGION_INIT(pScreen, &SubRegion, NullBox, 0);
			subInited = TRUE;
		    }
		    REGION_COPY(pScreen, &SubRegion, pRegion);
		    res |= miCheckSubSaveUnder(pChild, pChild->firstChild,
					     &SubRegion);
		}
		else
		{
		    res |= miCheckSubSaveUnder(pChild, pChild->firstChild,
					     pRegion);
		}

		if (pChild->saveUnder)
		    REGION_UNION(pScreen, pRegion, pRegion, &pChild->borderSize);
	    }
	}

	if (subInited)
	    REGION_UNINIT(pScreen, &SubRegion);
    }

    /*
     * Check the state of this window.	DIX save unders are
     * enabled for viewable windows with some client expressing
     * exposure interest and which intersect the save under region
     */

    if (pParent->viewable && 
	((pParent->eventMask | wOtherEventMasks(pParent)) & ExposureMask) &&
	REGION_NOTEMPTY(pScreen, &pParent->borderSize) &&
	RECT_IN_REGION(pScreen, pRegion, REGION_EXTENTS(pScreen, 
					&pParent->borderSize)) != rgnOUT)
    {
	if (!pParent->DIXsaveUnder)
	{
	    pParent->DIXsaveUnder = TRUE;
	    (*pScreen->ChangeWindowAttributes) (pParent, CWBackingStore);
	}
    }
    else
    {
	if (pParent->DIXsaveUnder)
	{
	    res = TRUE;
	    pParent->DIXsaveUnder = FALSE;
	}
    }
    return res;
}


/*-
 *-----------------------------------------------------------------------
 * miChangeSaveUnder --
 *	Change the save-under state of a tree of windows. Called when
 *	a window with saveUnder TRUE is mapped/unmapped/reconfigured.
 *	
 * Results:
 *	TRUE if any windows need to have backing-store removed (which
 *	means that PostChangeSaveUnder needs to be called later to 
 *	finish the job).
 *
 * Side Effects:
 *	Windows may have backing-store turned on or off.
 *
 *-----------------------------------------------------------------------
 */
Bool
miChangeSaveUnder(pWin, first)
    register WindowPtr	pWin;
    WindowPtr		first;		/* First window to check.
					 * Used when pWin was restacked */
{
    RegionRec	rgn;	/* Area obscured by saveUnder windows */
    register ScreenPtr pScreen;
    Bool	res;

    if (!deltaSaveUndersViewable && !numSaveUndersViewable)
	return FALSE;
    numSaveUndersViewable += deltaSaveUndersViewable;
    deltaSaveUndersViewable = 0;
    pScreen = pWin->drawable.pScreen;
    REGION_INIT(pScreen, &rgn, NullBox, 1);
    res = miCheckSubSaveUnder (pWin->parent,
			       pWin->saveUnder ? first : pWin->nextSib,
			       &rgn);
    REGION_UNINIT(pScreen, &rgn);
    return res;
}

/*-
 *-----------------------------------------------------------------------
 * miPostChangeSaveUnder --
 *	Actually turn backing-store off for those windows that no longer
 *	need to have it on.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Backing-store and SAVE_UNDER_CHANGE_BIT are turned off for those
 *	windows affected.
 *
 *-----------------------------------------------------------------------
 */
void
miPostChangeSaveUnder(pWin, pFirst)
    WindowPtr		pWin;
    WindowPtr		pFirst;
{
    register WindowPtr pParent, pChild;
    ChangeWindowAttributesProcPtr ChangeWindowAttributes;

    if (!(pParent = pWin->parent))
	return;
    ChangeWindowAttributes = pParent->drawable.pScreen->ChangeWindowAttributes;
    if (!pParent->DIXsaveUnder &&
	(pParent->backingStore == NotUseful) && pParent->backStorage)
	(*ChangeWindowAttributes)(pParent, CWBackingStore);
    if (!(pChild = pFirst))
	return;
    while (1)
    {
	if (!pChild->DIXsaveUnder &&
	    (pChild->backingStore == NotUseful) && pChild->backStorage)
	    (*ChangeWindowAttributes)(pChild, CWBackingStore);
	if (pChild->firstChild)
	{
	    pChild = pChild->firstChild;
	    continue;
	}
	while (!pChild->nextSib)
	{
	    pChild = pChild->parent;
	    if (pChild == pParent)
		return;
	}
	pChild = pChild->nextSib;
    }
}

void
miMarkWindow(pWin)
    register WindowPtr pWin;
{
    register ValidatePtr val;

    if (pWin->valdata)
	return;
    val = (ValidatePtr)xnfalloc(sizeof(ValidateRec));
    val->before.oldAbsCorner.x = pWin->drawable.x;
    val->before.oldAbsCorner.y = pWin->drawable.y;
    val->before.borderVisible = NullRegion;
    val->before.resized = FALSE;
    pWin->valdata = val;
}

Bool
miMarkOverlappedWindows(pWin, pFirst, ppLayerWin)
    WindowPtr pWin;
    WindowPtr pFirst;
    WindowPtr *ppLayerWin;
{
    register BoxPtr box;
    register WindowPtr pChild, pLast;
    Bool anyMarked = FALSE;
    void (* MarkWindow)() = pWin->drawable.pScreen->MarkWindow;
    ScreenPtr pScreen = pWin->drawable.pScreen;

    /* single layered systems are easy */
    if (ppLayerWin) *ppLayerWin = pWin;

    if (pWin == pFirst)
    {
	/* Blindly mark pWin and all of its inferiors.	 This is a slight
	 * overkill if there are mapped windows that outside pWin's border,
	 * but it's better than wasting time on RectIn checks.
	 */
	pChild = pWin;
	while (1)
	{
	    if (pChild->viewable)
	    {
		(* MarkWindow)(pChild);
		if (pChild->firstChild)
		{
		    pChild = pChild->firstChild;
		    continue;
		}
	    }
	    while (!pChild->nextSib && (pChild != pWin))
		pChild = pChild->parent;
	    if (pChild == pWin)
		break;
	    pChild = pChild->nextSib;
	}
	anyMarked = TRUE;
	pFirst = pFirst->nextSib;
    }
    if ( (pChild = pFirst) )
    {
	box = REGION_EXTENTS(pChild->drawable.pScreen, &pWin->borderSize);
	pLast = pChild->parent->lastChild;
	while (1)
	{
	    if (pChild->viewable && RECT_IN_REGION(pScreen, &pChild->borderSize,
						       box))
	    {
		(* MarkWindow)(pChild);
		anyMarked = TRUE;
		if (pChild->firstChild)
		{
		    pChild = pChild->firstChild;
		    continue;
		}
	    }
	    while (!pChild->nextSib && (pChild != pLast))
		pChild = pChild->parent;
	    if (pChild == pLast)
		break;
	    pChild = pChild->nextSib;
	}
    }
    if (anyMarked)
	(* MarkWindow)(pWin->parent);
    return anyMarked;
}

/*****
 *  miHandleValidateExposures(pWin)
 *    starting at pWin, draw background in any windows that have exposure
 *    regions, translate the regions, restore any backing store,
 *    and then send any regions still exposed to the client
 *****/
void
miHandleValidateExposures(pWin)
    WindowPtr pWin;
{
    register WindowPtr pChild;
    register ValidatePtr val;
    ScreenPtr pScreen = pWin->drawable.pScreen;
    void (* WindowExposures)();

    pChild = pWin;
    WindowExposures = pChild->drawable.pScreen->WindowExposures;
    while (1)
    {
	if ( (val = pChild->valdata) )
	{
	    if (REGION_NOTEMPTY(pScreen, &val->after.borderExposed))
		(*pChild->drawable.pScreen->PaintWindowBorder)(pChild,
						    &val->after.borderExposed,
						    PW_BORDER);
	    REGION_UNINIT(pScreen, &val->after.borderExposed);
	    (*WindowExposures)(pChild, &val->after.exposed, NullRegion);
	    REGION_UNINIT(pScreen, &val->after.exposed);
	    xfree(val);
	    pChild->valdata = (ValidatePtr)NULL;
	    if (pChild->firstChild)
	    {
		pChild = pChild->firstChild;
		continue;
	    }
	}
	while (!pChild->nextSib && (pChild != pWin))
	    pChild = pChild->parent;
	if (pChild == pWin)
	    break;
	pChild = pChild->nextSib;
    }
}

void
miMoveWindow(pWin, x, y, pNextSib, kind)
    register WindowPtr pWin;
    int x,y;
    WindowPtr pNextSib;
    VTKind kind;
{
    WindowPtr pParent;
    Bool WasViewable = (Bool)(pWin->viewable);
    short bw;
    RegionPtr oldRegion;
    DDXPointRec oldpt;
    Bool anyMarked;
    register ScreenPtr pScreen;
    WindowPtr windowToValidate;
#ifdef DO_SAVE_UNDERS
    Bool dosave = FALSE;
#endif
    WindowPtr pLayerWin;

    /* if this is a root window, can't be moved */
    if (!(pParent = pWin->parent))
       return ;
    pScreen = pWin->drawable.pScreen;
    bw = wBorderWidth (pWin);

    oldpt.x = pWin->drawable.x;
    oldpt.y = pWin->drawable.y;
    if (WasViewable)
    {
	oldRegion = REGION_CREATE(pScreen, NullBox, 1);
	REGION_COPY(pScreen, oldRegion, &pWin->borderClip);
	anyMarked = (*pScreen->MarkOverlappedWindows)(pWin, pWin, &pLayerWin);
    }
    pWin->origin.x = x + (int)bw;
    pWin->origin.y = y + (int)bw;
    x = pWin->drawable.x = pParent->drawable.x + x + (int)bw;
    y = pWin->drawable.y = pParent->drawable.y + y + (int)bw;

    SetWinSize (pWin);
    SetBorderSize (pWin);

    (*pScreen->PositionWindow)(pWin, x, y);

    windowToValidate = MoveWindowInStack(pWin, pNextSib);

    ResizeChildrenWinSize(pWin, x - oldpt.x, y - oldpt.y, 0, 0);

    if (WasViewable)
    {
	if (pLayerWin == pWin)
	    anyMarked |= (*pScreen->MarkOverlappedWindows)
				(pWin, windowToValidate, (WindowPtr *)NULL);
	else
	    anyMarked |= (*pScreen->MarkOverlappedWindows)
				(pWin, pLayerWin, (WindowPtr *)NULL);

#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    dosave = (*pScreen->ChangeSaveUnder)(pLayerWin, windowToValidate);
	}
#endif /* DO_SAVE_UNDERS */

	if (anyMarked)
	{
	    (*pScreen->ValidateTree)(pLayerWin->parent, NullWindow, kind);
	    (* pWin->drawable.pScreen->CopyWindow)(pWin, oldpt, oldRegion);
	    REGION_DESTROY(pScreen, oldRegion);
	    /* XXX need to retile border if ParentRelative origin */
	    (*pScreen->HandleExposures)(pLayerWin->parent);
	}
#ifdef DO_SAVE_UNDERS
	if (dosave)
	    (*pScreen->PostChangeSaveUnder)(pLayerWin, windowToValidate);
#endif /* DO_SAVE_UNDERS */
	if (anyMarked && pScreen->PostValidateTree)
	    (*pScreen->PostValidateTree)(pLayerWin->parent, NullWindow, kind);
    }
    if (pWin->realized)
	WindowsRestructured ();
}


/*
 * pValid is a region of the screen which has been
 * successfully copied -- recomputed exposed regions for affected windows
 */

static int
miRecomputeExposures (pWin, value)
    register WindowPtr	pWin;
    pointer		value; /* must conform to VisitWindowProcPtr */
{
    register ScreenPtr	pScreen;
    RegionPtr	pValid = (RegionPtr)value;

    if (pWin->valdata)
    {
	pScreen = pWin->drawable.pScreen;
	/*
	 * compute exposed regions of this window
	 */
	REGION_SUBTRACT(pScreen, &pWin->valdata->after.exposed,
			&pWin->clipList, pValid);
	/*
	 * compute exposed regions of the border
	 */
	REGION_SUBTRACT(pScreen, &pWin->valdata->after.borderExposed,
			     &pWin->borderClip, &pWin->winSize);
	REGION_SUBTRACT(pScreen, &pWin->valdata->after.borderExposed,
			     &pWin->valdata->after.borderExposed, pValid);
	return WT_WALKCHILDREN;
    }
    return WT_NOMATCH;
}

void
miSlideAndSizeWindow(pWin, x, y, w, h, pSib)
    register WindowPtr pWin;
    int x,y;
    unsigned int w, h;
    WindowPtr pSib;
{
    WindowPtr pParent;
    Bool WasViewable = (Bool)(pWin->viewable);
    unsigned short width = pWin->drawable.width,
		   height = pWin->drawable.height;
    short oldx = pWin->drawable.x,
	  oldy = pWin->drawable.y;
    int bw = wBorderWidth (pWin);
    short dw, dh;
    DDXPointRec oldpt;
    RegionPtr oldRegion;
    Bool anyMarked;
    register ScreenPtr pScreen;
    WindowPtr pFirstChange;
    register WindowPtr pChild;
    RegionPtr	gravitate[StaticGravity + 1];
    register unsigned g;
    int		nx, ny;		/* destination x,y */
    int		newx, newy;	/* new inner window position */
    RegionPtr	pRegion;
    RegionPtr	destClip;	/* portions of destination already written */
    RegionPtr	oldWinClip;	/* old clip list for window */
    RegionPtr	borderVisible = NullRegion; /* visible area of the border */
    RegionPtr	bsExposed = NullRegion;	    /* backing store exposures */
    Bool	shrunk = FALSE; /* shrunk in an inner dimension */
    Bool	moved = FALSE;	/* window position changed */
#ifdef DO_SAVE_UNDERS
    Bool	dosave = FALSE;
#endif
    WindowPtr  pLayerWin;

    /* if this is a root window, can't be resized */
    if (!(pParent = pWin->parent))
	return ;

    pScreen = pWin->drawable.pScreen;
    newx = pParent->drawable.x + x + bw;
    newy = pParent->drawable.y + y + bw;
    if (WasViewable)
    {
	anyMarked = FALSE;
	/*
	 * save the visible region of the window
	 */
	oldRegion = REGION_CREATE(pScreen, NullBox, 1);
	REGION_COPY(pScreen, oldRegion, &pWin->winSize);

	/*
	 * categorize child windows into regions to be moved
	 */
	for (g = 0; g <= StaticGravity; g++)
	    gravitate[g] = (RegionPtr) NULL;
	for (pChild = pWin->firstChild; pChild; pChild = pChild->nextSib)
	{
	    g = pChild->winGravity;
	    if (g != UnmapGravity)
	    {
		if (!gravitate[g])
		    gravitate[g] = REGION_CREATE(pScreen, NullBox, 1);
		REGION_UNION(pScreen, gravitate[g],
				   gravitate[g], &pChild->borderClip);
	    }
	    else
	    {
		UnmapWindow(pChild, TRUE);
		anyMarked = TRUE;
	    }
	}
	anyMarked |= (*pScreen->MarkOverlappedWindows)(pWin, pWin, 
						       &pLayerWin);

	oldWinClip = NULL;
	if (pWin->bitGravity != ForgetGravity)
	{
	    oldWinClip = REGION_CREATE(pScreen, NullBox, 1);
	    REGION_COPY(pScreen, oldWinClip, &pWin->clipList);
	}
	/*
	 * if the window is changing size, borderExposed
	 * can't be computed correctly without some help.
	 */
	if (pWin->drawable.height > h || pWin->drawable.width > w)
	    shrunk = TRUE;

	if (newx != oldx || newy != oldy)
	    moved = TRUE;

	if ((pWin->drawable.height != h || pWin->drawable.width != w) &&
	    HasBorder (pWin))
	{
	    borderVisible = REGION_CREATE(pScreen, NullBox, 1);
	    /* for tiled borders, we punt and draw the whole thing */
	    if (pWin->borderIsPixel || !moved)
	    {
		if (shrunk || moved)
		    REGION_SUBTRACT(pScreen, borderVisible,
					  &pWin->borderClip,
					  &pWin->winSize);
		else
		    REGION_COPY(pScreen, borderVisible,
					    &pWin->borderClip);
	    }
	}
    }
    pWin->origin.x = x + bw;
    pWin->origin.y = y + bw;
    pWin->drawable.height = h;
    pWin->drawable.width = w;

    x = pWin->drawable.x = newx;
    y = pWin->drawable.y = newy;

    SetWinSize (pWin);
    SetBorderSize (pWin);

    dw = (int)w - (int)width;
    dh = (int)h - (int)height;
    ResizeChildrenWinSize(pWin, x - oldx, y - oldy, dw, dh);

    /* let the hardware adjust background and border pixmaps, if any */
    (*pScreen->PositionWindow)(pWin, x, y);

    pFirstChange = MoveWindowInStack(pWin, pSib);

    if (WasViewable)
    {
	pRegion = REGION_CREATE(pScreen, NullBox, 1);
	if (pWin->backStorage)
	    REGION_COPY(pScreen, pRegion, &pWin->clipList);

	if (pLayerWin == pWin)
	    anyMarked |= (*pScreen->MarkOverlappedWindows)(pWin, pFirstChange,
						(WindowPtr *)NULL);
	else
	    anyMarked |= (*pScreen->MarkOverlappedWindows)(pWin, pLayerWin,
						(WindowPtr *)NULL);

	if (pWin->valdata)
	{
	    pWin->valdata->before.resized = TRUE;
	    pWin->valdata->before.borderVisible = borderVisible;
	}

#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    dosave = (*pScreen->ChangeSaveUnder)(pLayerWin, pFirstChange);
	}
#endif /* DO_SAVE_UNDERS */

	if (anyMarked)
	    (*pScreen->ValidateTree)(pLayerWin->parent, pFirstChange, VTOther);
	/*
	 * the entire window is trashed unless bitGravity
	 * recovers portions of it
	 */
	REGION_COPY(pScreen, &pWin->valdata->after.exposed, &pWin->clipList);
    }

    GravityTranslate (x, y, oldx, oldy, dw, dh, pWin->bitGravity, &nx, &ny);

    if (pWin->backStorage &&
	((pWin->backingStore == Always) || WasViewable))
    {
	if (!WasViewable)
	    pRegion = &pWin->clipList; /* a convenient empty region */
	if (pWin->bitGravity == ForgetGravity)
	    bsExposed = (*pScreen->TranslateBackingStore)
				(pWin, 0, 0, NullRegion, oldx, oldy);
	else
	{
	    bsExposed = (*pScreen->TranslateBackingStore)
			     (pWin, nx - x, ny - y, pRegion, oldx, oldy);
	}
    }

    if (WasViewable)
    {
	/* avoid the border */
	if (HasBorder (pWin))
	{
	    int	offx, offy, dx, dy;

	    /* kruft to avoid double translates for each gravity */
	    offx = 0;
	    offy = 0;
	    for (g = 0; g <= StaticGravity; g++)
	    {
		if (!gravitate[g])
		    continue;

		/* align winSize to gravitate[g].
		 * winSize is in new coordinates,
		 * gravitate[g] is still in old coordinates */
		GravityTranslate (x, y, oldx, oldy, dw, dh, g, &nx, &ny);
		
		dx = (oldx - nx) - offx;
		dy = (oldy - ny) - offy;
		if (dx || dy)
		{
		    REGION_TRANSLATE(pScreen, &pWin->winSize, dx, dy);
		    offx += dx;
		    offy += dy;
		}
		REGION_INTERSECT(pScreen, gravitate[g], gravitate[g],
				 &pWin->winSize);
	    }
	    /* get winSize back where it belongs */
	    if (offx || offy)
		REGION_TRANSLATE(pScreen, &pWin->winSize, -offx, -offy);
	}
	/*
	 * add screen bits to the appropriate bucket
	 */

	if (oldWinClip)
	{
	    /*
	     * clip to new clipList
	     */
	    REGION_COPY(pScreen, pRegion, oldWinClip);
	    REGION_TRANSLATE(pScreen, pRegion, nx - oldx, ny - oldy);
	    REGION_INTERSECT(pScreen, oldWinClip, pRegion, &pWin->clipList);
	    /*
	     * don't step on any gravity bits which will be copied after this
	     * region.	Note -- this assumes that the regions will be copied
	     * in gravity order.
	     */
	    for (g = pWin->bitGravity + 1; g <= StaticGravity; g++)
	    {
		if (gravitate[g])
		    REGION_SUBTRACT(pScreen, oldWinClip, oldWinClip,
					gravitate[g]);
	    }
	    REGION_TRANSLATE(pScreen, oldWinClip, oldx - nx, oldy - ny);
	    g = pWin->bitGravity;
	    if (!gravitate[g])
		gravitate[g] = oldWinClip;
	    else
	    {
		REGION_UNION(pScreen, gravitate[g], gravitate[g], oldWinClip);
		REGION_DESTROY(pScreen, oldWinClip);
	    }
	}

	/*
	 * move the bits on the screen
	 */

	destClip = NULL;

	for (g = 0; g <= StaticGravity; g++)
	{
	    if (!gravitate[g])
		continue;

	    GravityTranslate (x, y, oldx, oldy, dw, dh, g, &nx, &ny);

	    oldpt.x = oldx + (x - nx);
	    oldpt.y = oldy + (y - ny);

	    /* Note that gravitate[g] is *translated* by CopyWindow */

	    /* only copy the remaining useful bits */

	    REGION_INTERSECT(pScreen, gravitate[g], gravitate[g], oldRegion);

	    /* clip to not overwrite already copied areas */

	    if (destClip) {
		REGION_TRANSLATE(pScreen, destClip, oldpt.x - x, oldpt.y - y);
		REGION_SUBTRACT(pScreen, gravitate[g], gravitate[g], destClip);
		REGION_TRANSLATE(pScreen, destClip, x - oldpt.x, y - oldpt.y);
	    }

	    /* and move those bits */

	    if (oldpt.x != x || oldpt.y != y)
		(*pWin->drawable.pScreen->CopyWindow)(pWin, oldpt, gravitate[g]);

	    /* remove any overwritten bits from the remaining useful bits */

	    REGION_SUBTRACT(pScreen, oldRegion, oldRegion, gravitate[g]);

	    /*
	     * recompute exposed regions of child windows
	     */
	
	    for (pChild = pWin->firstChild; pChild; pChild = pChild->nextSib)
	    {
		if (pChild->winGravity != g)
		    continue;
		REGION_INTERSECT(pScreen, pRegion,
				       &pChild->borderClip, gravitate[g]);
		TraverseTree (pChild, miRecomputeExposures, (pointer)pRegion);
	    }

	    /*
	     * remove the successfully copied regions of the
	     * window from its exposed region
	     */

	    if (g == pWin->bitGravity)
		REGION_SUBTRACT(pScreen, &pWin->valdata->after.exposed,
				     &pWin->valdata->after.exposed, gravitate[g]);
	    if (!destClip)
		destClip = gravitate[g];
	    else
	    {
		REGION_UNION(pScreen, destClip, destClip, gravitate[g]);
		REGION_DESTROY(pScreen, gravitate[g]);
	    }
	}

	REGION_DESTROY(pScreen, oldRegion);
	REGION_DESTROY(pScreen, pRegion);
	if (destClip)
	    REGION_DESTROY(pScreen, destClip);
	if (bsExposed)
	{
	    RegionPtr	valExposed = NullRegion;

	    if (pWin->valdata)
		valExposed = &pWin->valdata->after.exposed;
	    (*pScreen->WindowExposures) (pWin, valExposed, bsExposed);
	    if (valExposed)
		REGION_EMPTY(pScreen, valExposed);
	    REGION_DESTROY(pScreen, bsExposed);
	}
	if (anyMarked)
	    (*pScreen->HandleExposures)(pLayerWin->parent);
#ifdef DO_SAVE_UNDERS
	if (dosave)
	{
	    (*pScreen->PostChangeSaveUnder)(pLayerWin, pFirstChange);
	}
#endif /* DO_SAVE_UNDERS */
	if (anyMarked && pScreen->PostValidateTree)
	    (*pScreen->PostValidateTree)(pLayerWin->parent, pFirstChange,
					  VTOther);
    }
    else if (bsExposed)
    {
	(*pScreen->WindowExposures) (pWin, NullRegion, bsExposed);
	REGION_DESTROY(pScreen, bsExposed);
    }
    if (pWin->realized)
	WindowsRestructured ();
}

WindowPtr
miGetLayerWindow(pWin)
    WindowPtr pWin;
{
    return pWin->firstChild;
}

#ifdef SHAPE
/******
 *
 * miSetShape
 *    The border/window shape has changed.  Recompute winSize/borderSize
 *    and send appropriate exposure events
 */

void
miSetShape(pWin)
    register WindowPtr	pWin;
{
    Bool	WasViewable = (Bool)(pWin->viewable);
    register ScreenPtr pScreen = pWin->drawable.pScreen;
    Bool	anyMarked;
    WindowPtr	pParent = pWin->parent;
    RegionPtr	pOldClip, bsExposed;
#ifdef DO_SAVE_UNDERS
    Bool	dosave = FALSE;
#endif
    WindowPtr   pLayerWin;

    if (WasViewable)
    {
	anyMarked = (*pScreen->MarkOverlappedWindows)(pWin, pWin,
						      &pLayerWin);
	if (pWin->valdata)
	{
	    if (HasBorder (pWin))
	    {
		RegionPtr	borderVisible;

		borderVisible = REGION_CREATE(pScreen, NullBox, 1);
		REGION_SUBTRACT(pScreen, borderVisible,
				      &pWin->borderClip, &pWin->winSize);
		pWin->valdata->before.borderVisible = borderVisible;
	    }
	    pWin->valdata->before.resized = TRUE;
	}
    }

    SetWinSize (pWin);
    SetBorderSize (pWin);

    ResizeChildrenWinSize(pWin, 0, 0, 0, 0);

    if (WasViewable)
    {
	if (pWin->backStorage)
	{
	    pOldClip = REGION_CREATE(pScreen, NullBox, 1);
	    REGION_COPY(pScreen, pOldClip, &pWin->clipList);
	}

	anyMarked |= (*pScreen->MarkOverlappedWindows)(pWin, pWin,
						(WindowPtr *)NULL);

#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    dosave = (*pScreen->ChangeSaveUnder)(pLayerWin, pLayerWin);
	}
#endif /* DO_SAVE_UNDERS */

	if (anyMarked)
	    (*pScreen->ValidateTree)(pLayerWin->parent, NullWindow, VTOther);
    }

    if (pWin->backStorage &&
	((pWin->backingStore == Always) || WasViewable))
    {
	if (!WasViewable)
	    pOldClip = &pWin->clipList; /* a convenient empty region */
	bsExposed = (*pScreen->TranslateBackingStore)
			     (pWin, 0, 0, pOldClip,
			      pWin->drawable.x, pWin->drawable.y);
	if (WasViewable)
	    REGION_DESTROY(pScreen, pOldClip);
	if (bsExposed)
	{
	    RegionPtr	valExposed = NullRegion;
    
	    if (pWin->valdata)
		valExposed = &pWin->valdata->after.exposed;
	    (*pScreen->WindowExposures) (pWin, valExposed, bsExposed);
	    if (valExposed)
		REGION_EMPTY(pScreen, valExposed);
	    REGION_DESTROY(pScreen, bsExposed);
	}
    }
    if (WasViewable)
    {
	if (anyMarked)
	    (*pScreen->HandleExposures)(pLayerWin->parent);
#ifdef DO_SAVE_UNDERS
	if (dosave)
	    (*pScreen->PostChangeSaveUnder)(pLayerWin, pLayerWin);
#endif /* DO_SAVE_UNDERS */
	if (anyMarked && pScreen->PostValidateTree)
	    (*pScreen->PostValidateTree)(pLayerWin->parent, NullWindow, VTOther);
    }
    if (pWin->realized)
	WindowsRestructured ();
    CheckCursorConfinement(pWin);
}
#endif

/* Keeps the same inside(!) origin */

void
miChangeBorderWidth(pWin, width)
    register WindowPtr pWin;
    unsigned int width;
{
    WindowPtr pParent;
    int oldwidth;
    Bool anyMarked;
    register ScreenPtr pScreen;
    Bool WasViewable = (Bool)(pWin->viewable);
    Bool HadBorder;
#ifdef DO_SAVE_UNDERS
    Bool	dosave = FALSE;
#endif
    WindowPtr  pLayerWin;

    oldwidth = wBorderWidth (pWin);
    if (oldwidth == width)
	return;
    HadBorder = HasBorder(pWin);
    pScreen = pWin->drawable.pScreen;
    pParent = pWin->parent;
    if (WasViewable && width < oldwidth)
	anyMarked = (*pScreen->MarkOverlappedWindows)(pWin, pWin, &pLayerWin);

    pWin->borderWidth = width;
    SetBorderSize (pWin);

    if (WasViewable)
    {
	if (width > oldwidth)
	{
	    anyMarked = (*pScreen->MarkOverlappedWindows)(pWin, pWin,
							  &pLayerWin);
	    /*
	     * save the old border visible region to correctly compute
	     * borderExposed.
	     */
	    if (pWin->valdata && HadBorder)
	    {
		RegionPtr   borderVisible;
		borderVisible = REGION_CREATE(pScreen, NULL, 1);
		REGION_SUBTRACT(pScreen, borderVisible,
				      &pWin->borderClip, &pWin->winSize);
		pWin->valdata->before.borderVisible = borderVisible;
	    }
	}
#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    dosave = (*pScreen->ChangeSaveUnder)(pLayerWin, pWin->nextSib);
	}
#endif /* DO_SAVE_UNDERS */

	if (anyMarked)
	{
	    (*pScreen->ValidateTree)(pLayerWin->parent, pLayerWin, VTOther);
	    (*pScreen->HandleExposures)(pLayerWin->parent);
	}
#ifdef DO_SAVE_UNDERS
	if (dosave)
	    (*pScreen->PostChangeSaveUnder)(pLayerWin, pWin->nextSib);
#endif /* DO_SAVE_UNDERS */
	if (anyMarked && pScreen->PostValidateTree)
	    (*pScreen->PostValidateTree)(pLayerWin->parent, pLayerWin,
					  VTOther);
    }
    if (pWin->realized)
	WindowsRestructured ();
}

void
miMarkUnrealizedWindow(pChild, pWin, fromConfigure)
    WindowPtr pChild;
    WindowPtr pWin;
    Bool fromConfigure;
{
    if ((pChild != pWin) || fromConfigure)
    {
	REGION_EMPTY(pChild->drawable.pScreen, &pChild->clipList);
	if (pChild->drawable.pScreen->ClipNotify)
	    (* pChild->drawable.pScreen->ClipNotify)(pChild, 0, 0);
	REGION_EMPTY(pChild->drawable.pScreen, &pChild->borderClip);
    }
}
