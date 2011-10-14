/*
 * $Id: compwindow.c,v 1.11 2005/07/03 07:37:34 daniels Exp $
 *
 * Copyright © 2003 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "compint.h"

#ifdef COMPOSITE_DEBUG
static int
compCheckWindow (WindowPtr pWin, pointer data)
{
    ScreenPtr	pScreen = pWin->drawable.pScreen;
    PixmapPtr	pWinPixmap = (*pScreen->GetWindowPixmap) (pWin);
    PixmapPtr	pParentPixmap = pWin->parent ? (*pScreen->GetWindowPixmap) (pWin->parent) : 0;
    PixmapPtr	pScreenPixmap = (*pScreen->GetScreenPixmap) (pScreen);
    
    if (!pWin->parent)
    {
	assert (!pWin->redirectDraw);
	assert (pWinPixmap == pScreenPixmap);
    }
    else if (pWin->redirectDraw)
    {
	assert (pWinPixmap != pParentPixmap);
	assert (pWinPixmap != pScreenPixmap);
    }
    else
    {
	assert (pWinPixmap == pParentPixmap);
    }
    assert (0 < pWinPixmap->refcnt && pWinPixmap->refcnt < 3);
    assert (0 < pScreenPixmap->refcnt && pScreenPixmap->refcnt < 3);
    if (pParentPixmap)
	assert (0 <= pParentPixmap->refcnt && pParentPixmap->refcnt < 3);
    return WT_WALKCHILDREN;
}

void
compCheckTree (ScreenPtr pScreen)
{
    WalkTree (pScreen, compCheckWindow, 0);
}
#endif

typedef struct _compPixmapVisit {
    WindowPtr	pWindow;
    PixmapPtr	pPixmap;
} CompPixmapVisitRec, *CompPixmapVisitPtr;

static Bool
compRepaintBorder (ClientPtr pClient, pointer closure)
{
    WindowPtr	pWindow = LookupWindow ((XID) closure, pClient);

    if (pWindow)
    {
	RegionRec exposed;

	REGION_NULL(pScreen, &exposed);
	REGION_SUBTRACT(pScreen, &exposed, &pWindow->borderClip, &pWindow->winSize);
	(*pWindow->drawable.pScreen->PaintWindowBorder)(pWindow, &exposed, PW_BORDER);
	REGION_UNINIT(pScreen, &exposed);
    }
    return TRUE;
}

static int
compSetPixmapVisitWindow (WindowPtr pWindow, pointer data)
{
    CompPixmapVisitPtr	pVisit = (CompPixmapVisitPtr) data;
    ScreenPtr		pScreen = pWindow->drawable.pScreen;

    if (pWindow != pVisit->pWindow && pWindow->redirectDraw)
	return WT_DONTWALKCHILDREN;
    (*pScreen->SetWindowPixmap) (pWindow, pVisit->pPixmap);
    /*
     * Recompute winSize and borderSize.  This is duplicate effort
     * when resizing pixmaps, but necessary when changing redirection.
     * Might be nice to fix this.
     */
    SetWinSize (pWindow);
    SetBorderSize (pWindow);
    if (HasBorder (pWindow))
	QueueWorkProc (compRepaintBorder, serverClient, 
		       (pointer) pWindow->drawable.id);
    return WT_WALKCHILDREN;
}

void
compSetPixmap (WindowPtr pWindow, PixmapPtr pPixmap)
{
    CompPixmapVisitRec	visitRec;

    visitRec.pWindow = pWindow;
    visitRec.pPixmap = pPixmap;
    TraverseTree (pWindow, compSetPixmapVisitWindow, (pointer) &visitRec);
    compCheckTree (pWindow->drawable.pScreen);
}

Bool
compCheckRedirect (WindowPtr pWin)
{
    CompWindowPtr   cw = GetCompWindow (pWin);
    Bool	    should;

    should = pWin->realized && (pWin->drawable.class != InputOnly) &&
	     (cw != NULL);
    
    if (should != pWin->redirectDraw)
    {
	if (should)
	    return compAllocPixmap (pWin);
	else
	    compFreePixmap (pWin);
    }
    return TRUE;
}

Bool
compPositionWindow (WindowPtr pWin, int x, int y)
{
    ScreenPtr	    pScreen = pWin->drawable.pScreen;
    CompScreenPtr   cs = GetCompScreen (pScreen);
    Bool	    ret = TRUE;

    pScreen->PositionWindow = cs->PositionWindow;
    /*
     * "Shouldn't need this as all possible places should be wrapped
     *
    compCheckRedirect (pWin);
     */
#ifdef COMPOSITE_DEBUG
    if (pWin->redirectDraw != (pWin->viewable && (GetCompWindow(pWin) != NULL)))
	abort ();
#endif
    if (pWin->redirectDraw)
    {
	PixmapPtr   pPixmap = (*pScreen->GetWindowPixmap) (pWin);
	int	    bw = wBorderWidth (pWin);
	int	    nx = pWin->drawable.x - bw;
	int	    ny = pWin->drawable.y - bw;

	if (pPixmap->screen_x != nx || pPixmap->screen_y != ny)
	{
	    pPixmap->screen_x = nx;
	    pPixmap->screen_y = ny;
	    pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	}
    }

    if (!(*pScreen->PositionWindow) (pWin, x, y))
	ret = FALSE;
    cs->PositionWindow = pScreen->PositionWindow;
    pScreen->PositionWindow = compPositionWindow;
    compCheckTree (pWin->drawable.pScreen);
    return ret;
}

Bool
compRealizeWindow (WindowPtr pWin)
{
    ScreenPtr	    pScreen = pWin->drawable.pScreen;
    CompScreenPtr   cs = GetCompScreen (pScreen);
    Bool	    ret = TRUE;

    pScreen->RealizeWindow = cs->RealizeWindow;
    compCheckRedirect (pWin);
    if (!(*pScreen->RealizeWindow) (pWin))
	ret = FALSE;
    cs->RealizeWindow = pScreen->RealizeWindow;
    pScreen->RealizeWindow = compRealizeWindow;
    compCheckTree (pWin->drawable.pScreen);
    return ret;
}

Bool
compUnrealizeWindow (WindowPtr pWin)
{
    ScreenPtr	    pScreen = pWin->drawable.pScreen;
    CompScreenPtr   cs = GetCompScreen (pScreen);
    Bool	    ret = TRUE;

    pScreen->UnrealizeWindow = cs->UnrealizeWindow;
    compCheckRedirect (pWin);
    if (!(*pScreen->UnrealizeWindow) (pWin))
	ret = FALSE;
    cs->UnrealizeWindow = pScreen->UnrealizeWindow;
    pScreen->UnrealizeWindow = compUnrealizeWindow;
    compCheckTree (pWin->drawable.pScreen);
    return ret;
}

void
compPaintWindowBackground (WindowPtr pWin, RegionPtr pRegion, int what)
{
    ScreenPtr		pScreen = pWin->drawable.pScreen;
    CompSubwindowsPtr	csw = GetCompSubwindows (pWin);
    CompScreenPtr	cs = GetCompScreen (pScreen);
    
    if (csw && csw->update == CompositeRedirectManual)
	return;
    pScreen->PaintWindowBackground = cs->PaintWindowBackground;
    (*pScreen->PaintWindowBackground) (pWin, pRegion, what);
    cs->PaintWindowBackground = pScreen->PaintWindowBackground;
    pScreen->PaintWindowBackground = compPaintWindowBackground;
}

/*
 * Called after the borderClip for the window has settled down
 * We use this to make sure our extra borderClip has the right origin
 */

void
compClipNotify (WindowPtr pWin, int dx, int dy)
{
    ScreenPtr		pScreen = pWin->drawable.pScreen;
    CompScreenPtr	cs = GetCompScreen (pScreen);
    CompWindowPtr	cw = GetCompWindow (pWin);
    
    if (cw)
    {
	if (cw->borderClipX != pWin->drawable.x || 
	    cw->borderClipY != pWin->drawable.y)
	{
	    REGION_TRANSLATE (pScreen, &cw->borderClip,
			      pWin->drawable.x - cw->borderClipX,
			      pWin->drawable.y - cw->borderClipY);
	    cw->borderClipX = pWin->drawable.x;
	    cw->borderClipY = pWin->drawable.y;
	}
    }
    if (cs->ClipNotify)
    {
	pScreen->ClipNotify = cs->ClipNotify;
	(*pScreen->ClipNotify) (pWin, dx, dy);
	cs->ClipNotify = pScreen->ClipNotify;
	pScreen->ClipNotify = compClipNotify;
    }
}

/*
 * Returns TRUE if the window needs server-provided automatic redirect,
 * which is true if the child and parent aren't both regular or ARGB visuals
 */

static Bool
compIsAlternateVisual (ScreenPtr    pScreen,
		       XID	    visual)
{
    CompScreenPtr	cs = GetCompScreen (pScreen);
    int			i;

    for (i = 0; i < NUM_COMP_ALTERNATE_VISUALS; i++)
	if (cs->alternateVisuals[i] == visual)
	    return TRUE;
    return FALSE;
}

static Bool
compImplicitRedirect (WindowPtr pWin, WindowPtr pParent)
{
    if (pParent)
    {
	ScreenPtr	pScreen = pWin->drawable.pScreen;
	XID		winVisual = wVisual (pWin);
	XID		parentVisual = wVisual (pParent);
    
	if (winVisual != parentVisual &&
	    (compIsAlternateVisual (pScreen, winVisual) ||
	     compIsAlternateVisual (pScreen, parentVisual)))
	    return TRUE;
    }
    return FALSE;
}

void
compMoveWindow (WindowPtr pWin, int x, int y, WindowPtr pSib, VTKind kind)
{
    ScreenPtr		pScreen = pWin->drawable.pScreen;
    CompScreenPtr	cs = GetCompScreen (pScreen);

    compCheckTree (pScreen);
    if (pWin->redirectDraw)
    {
	WindowPtr		pParent;
	int			draw_x, draw_y;
	unsigned int		w, h, bw;
	
	/* if this is a root window, can't be moved */
	if (!(pParent = pWin->parent))
	   return;
	
	bw = wBorderWidth (pWin);
	draw_x = pParent->drawable.x + x + (int)bw;
	draw_y = pParent->drawable.y + y + (int)bw;
	w = pWin->drawable.width;
	h = pWin->drawable.height;
	compReallocPixmap (pWin, draw_x, draw_y, w, h, bw);
    }
    compCheckTree (pScreen);

    pScreen->MoveWindow = cs->MoveWindow;
    (*pScreen->MoveWindow) (pWin, x, y, pSib, kind);
    cs->MoveWindow = pScreen->MoveWindow;
    pScreen->MoveWindow = compMoveWindow;

    if (pWin->redirectDraw)
    {
	CompWindowPtr	cw = GetCompWindow (pWin);
	if (cw->pOldPixmap)
	{
	    (*pScreen->DestroyPixmap) (cw->pOldPixmap);
	    cw->pOldPixmap = NullPixmap;
	}
    }

    compCheckTree (pScreen);
}

void
compResizeWindow (WindowPtr pWin, int x, int y,
		  unsigned int w, unsigned int h, WindowPtr pSib)
{
    ScreenPtr		pScreen = pWin->drawable.pScreen;
    CompScreenPtr	cs = GetCompScreen (pScreen);

    compCheckTree (pScreen);
    if (pWin->redirectDraw)
    {
	WindowPtr		pParent;
	int			draw_x, draw_y;
	unsigned int		bw;
	
	/* if this is a root window, can't be moved */
	if (!(pParent = pWin->parent))
	   return;
	
	bw = wBorderWidth (pWin);
	draw_x = pParent->drawable.x + x + (int)bw;
	draw_y = pParent->drawable.y + y + (int)bw;
	compReallocPixmap (pWin, draw_x, draw_y, w, h, bw);
    }
    compCheckTree (pScreen);
    
    pScreen->ResizeWindow = cs->ResizeWindow;
    (*pScreen->ResizeWindow) (pWin, x, y, w, h, pSib);
    cs->ResizeWindow = pScreen->ResizeWindow;
    pScreen->ResizeWindow = compResizeWindow;
    if (pWin->redirectDraw)
    {
	CompWindowPtr	cw = GetCompWindow (pWin);
	if (cw->pOldPixmap)
	{
	    (*pScreen->DestroyPixmap) (cw->pOldPixmap);
	    cw->pOldPixmap = NullPixmap;
	}
    }
    compCheckTree (pWin->drawable.pScreen);
}

void
compChangeBorderWidth (WindowPtr pWin, unsigned int bw)
{
    ScreenPtr		pScreen = pWin->drawable.pScreen;
    CompScreenPtr	cs = GetCompScreen (pScreen);

    compCheckTree (pScreen);
    if (pWin->redirectDraw)
    {
	WindowPtr		pParent;
	int			draw_x, draw_y;
	unsigned int		w, h;
	
	/* if this is a root window, can't be moved */
	if (!(pParent = pWin->parent))
	   return;
	
	draw_x = pWin->drawable.x;
	draw_y = pWin->drawable.y;
	w = pWin->drawable.width;
	h = pWin->drawable.height;
	compReallocPixmap (pWin, draw_x, draw_y, w, h, bw);
    }
    compCheckTree (pScreen);

    pScreen->ChangeBorderWidth = cs->ChangeBorderWidth;
    (*pScreen->ChangeBorderWidth) (pWin, bw);
    cs->ChangeBorderWidth = pScreen->ChangeBorderWidth;
    pScreen->ChangeBorderWidth = compChangeBorderWidth;
    if (pWin->redirectDraw)
    {
	CompWindowPtr	cw = GetCompWindow (pWin);
	if (cw->pOldPixmap)
	{
	    (*pScreen->DestroyPixmap) (cw->pOldPixmap);
	    cw->pOldPixmap = NullPixmap;
	}
    }
    compCheckTree (pWin->drawable.pScreen);
}

void
compReparentWindow (WindowPtr pWin, WindowPtr pPriorParent)
{
    ScreenPtr		pScreen = pWin->drawable.pScreen;
    CompScreenPtr	cs = GetCompScreen (pScreen);

    pScreen->ReparentWindow = cs->ReparentWindow;
    /*
     * Remove any implicit redirect due to synthesized visual
     */
    if (compImplicitRedirect (pWin, pPriorParent))
	compUnredirectWindow (serverClient, pWin, CompositeRedirectAutomatic);
    /*
     * Handle subwindows redirection
     */
    compUnredirectOneSubwindow (pPriorParent, pWin);
    compRedirectOneSubwindow (pWin->parent, pWin);
    /*
     * Add any implict redirect due to synthesized visual
     */
    if (compImplicitRedirect (pWin, pWin->parent))
	compRedirectWindow (serverClient, pWin, CompositeRedirectAutomatic);
    
    /*
     * Allocate any necessary redirect pixmap
     * (this actually should never be true; pWin is always unmapped)
     */
    compCheckRedirect (pWin);
    
    /*
     * Reset pixmap pointers as appropriate
     */
    if (pWin->parent && !pWin->redirectDraw)
	compSetPixmap (pWin, (*pScreen->GetWindowPixmap) (pWin->parent));
    /*
     * Call down to next function
     */
    if (pScreen->ReparentWindow)
	(*pScreen->ReparentWindow) (pWin, pPriorParent);
    cs->ReparentWindow = pScreen->ReparentWindow;
    pScreen->ReparentWindow = compReparentWindow;
    compCheckTree (pWin->drawable.pScreen);
}

void
compCopyWindow (WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
    ScreenPtr	    pScreen = pWin->drawable.pScreen;
    CompScreenPtr   cs = GetCompScreen (pScreen);
    int		    dx = 0, dy = 0;

    if (pWin->redirectDraw)
    {
	PixmapPtr	pPixmap = (*pScreen->GetWindowPixmap) (pWin);
	CompWindowPtr	cw = GetCompWindow (pWin);
	
	assert (cw->oldx != COMP_ORIGIN_INVALID);
	assert (cw->oldy != COMP_ORIGIN_INVALID);
	if (cw->pOldPixmap)
	{
	    /*
	     * Ok, the old bits are available in pOldPixmap and
	     * need to be copied to pNewPixmap.
	     */
	    RegionRec	rgnDst;
	    PixmapPtr	pPixmap = (*pScreen->GetWindowPixmap) (pWin);
	    GCPtr	pGC;
	    
	    dx = ptOldOrg.x - pWin->drawable.x;
	    dy = ptOldOrg.y - pWin->drawable.y;
	    REGION_TRANSLATE(pWin->drawable.pScreen, prgnSrc, -dx, -dy);

	    REGION_NULL (pWin->drawable.pScreen, &rgnDst);

	    REGION_INTERSECT(pWin->drawable.pScreen, &rgnDst,
			     &pWin->borderClip, prgnSrc);
	    
	    REGION_TRANSLATE (pWin->drawable.pScreen, &rgnDst, 
			      -pPixmap->screen_x, -pPixmap->screen_y);
	    
	    dx = dx + pPixmap->screen_x - cw->oldx;
	    dy = dy + pPixmap->screen_y - cw->oldy;
	    pGC = GetScratchGC (pPixmap->drawable.depth, pScreen);
	    if (pGC)
	    {
		BoxPtr	pBox = REGION_RECTS (&rgnDst);
		int	nBox = REGION_NUM_RECTS (&rgnDst);
		
		ValidateGC(&pPixmap->drawable, pGC);
		while (nBox--)
		{
		    (void) (*pGC->ops->CopyArea) (&cw->pOldPixmap->drawable,
						  &pPixmap->drawable,
						  pGC,
						  pBox->x1 + dx, pBox->y1 + dy,
						  pBox->x2 - pBox->x1,
						  pBox->y2 - pBox->y1,
						  pBox->x1, pBox->y1);
		    pBox++;
		}
		FreeScratchGC (pGC);
	    }
	    return;
	}
	dx = pPixmap->screen_x - cw->oldx;
	dy = pPixmap->screen_y - cw->oldy;
	ptOldOrg.x += dx;
	ptOldOrg.y += dy;
    }
    
    pScreen->CopyWindow = cs->CopyWindow;
    if (ptOldOrg.x != pWin->drawable.x || ptOldOrg.y != pWin->drawable.y)
    {
	if (dx || dy)
	    REGION_TRANSLATE (pScreen, prgnSrc, dx, dy);
	(*pScreen->CopyWindow) (pWin, ptOldOrg, prgnSrc);
	if (dx || dy)
	    REGION_TRANSLATE (pScreen, prgnSrc, -dx, -dy);
    }
    else
    {
	ptOldOrg.x -= dx;
	ptOldOrg.y -= dy;
	REGION_TRANSLATE (prgnSrc, prgnSrc,
			  pWin->drawable.x - ptOldOrg.x,
			  pWin->drawable.y - ptOldOrg.y);
	DamageDamageRegion (&pWin->drawable, prgnSrc);
    }
    cs->CopyWindow = pScreen->CopyWindow;
    pScreen->CopyWindow = compCopyWindow;
    compCheckTree (pWin->drawable.pScreen);
}

Bool
compCreateWindow (WindowPtr pWin)
{
    ScreenPtr		pScreen = pWin->drawable.pScreen;
    CompScreenPtr	cs = GetCompScreen (pScreen);
    Bool		ret;

    pScreen->CreateWindow = cs->CreateWindow;
    ret = (*pScreen->CreateWindow) (pWin);
    if (pWin->parent && ret)
    {
	CompSubwindowsPtr	csw = GetCompSubwindows (pWin->parent);
        CompClientWindowPtr	ccw;

	(*pScreen->SetWindowPixmap) (pWin, (*pScreen->GetWindowPixmap) (pWin->parent));
	if (csw)
	    for (ccw = csw->clients; ccw; ccw = ccw->next)
		compRedirectWindow (clients[CLIENT_ID(ccw->id)],
				    pWin, ccw->update);
	if (compImplicitRedirect (pWin, pWin->parent))
	    compRedirectWindow (serverClient, pWin, CompositeRedirectAutomatic);
    }
    cs->CreateWindow = pScreen->CreateWindow;
    pScreen->CreateWindow = compCreateWindow;
    compCheckTree (pWin->drawable.pScreen);
    return ret;
}

Bool
compDestroyWindow (WindowPtr pWin)
{
    ScreenPtr		pScreen = pWin->drawable.pScreen;
    CompScreenPtr	cs = GetCompScreen (pScreen);
    CompWindowPtr	cw;
    CompSubwindowsPtr	csw;
    Bool		ret;

    pScreen->DestroyWindow = cs->DestroyWindow;
    while ((cw = GetCompWindow (pWin)))
	FreeResource (cw->clients->id, RT_NONE);
    while ((csw = GetCompSubwindows (pWin)))
	FreeResource (csw->clients->id, RT_NONE);
    
    if (pWin->redirectDraw)
	compFreePixmap (pWin);
    ret = (*pScreen->DestroyWindow) (pWin);
    cs->DestroyWindow = pScreen->DestroyWindow;
    pScreen->DestroyWindow = compDestroyWindow;
/*    compCheckTree (pWin->drawable.pScreen); can't check -- tree isn't good*/
    return ret;
}

void
compSetRedirectBorderClip (WindowPtr pWin, RegionPtr pRegion)
{
    CompWindowPtr   cw = GetCompWindow (pWin);
    RegionRec	    damage;

    REGION_NULL (pScreen, &damage);
    /*
     * Align old border clip with new border clip
     */
    REGION_TRANSLATE (pScreen, &cw->borderClip,
		      pWin->drawable.x - cw->borderClipX,
		      pWin->drawable.y - cw->borderClipY);
    /*
     * Compute newly visible portion of window for repaint
     */
    REGION_SUBTRACT (pScreen, &damage, pRegion, &cw->borderClip);
    /*
     * Report that as damaged so it will be redrawn
     */
    DamageDamageRegion (&pWin->drawable, &damage);
    REGION_UNINIT (pScreen, &damage);
    /*
     * Save the new border clip region
     */
    REGION_COPY (pScreen, &cw->borderClip, pRegion);
    cw->borderClipX = pWin->drawable.x;
    cw->borderClipY = pWin->drawable.y;
}

RegionPtr
compGetRedirectBorderClip (WindowPtr pWin)
{
    CompWindowPtr   cw = GetCompWindow (pWin);

    return &cw->borderClip;
}

static VisualPtr
compGetWindowVisual (WindowPtr pWin)
{
    ScreenPtr	    pScreen = pWin->drawable.pScreen;
    VisualID	    vid = wVisual (pWin);
    int		    i;

    for (i = 0; i < pScreen->numVisuals; i++)
	if (pScreen->visuals[i].vid == vid)
	    return &pScreen->visuals[i];
    return 0;
}

static PictFormatPtr
compWindowFormat (WindowPtr pWin)
{
    ScreenPtr	pScreen = pWin->drawable.pScreen;
    
    return PictureMatchVisual (pScreen, pWin->drawable.depth,
			       compGetWindowVisual (pWin));
}

static void
compWindowUpdateAutomatic (WindowPtr pWin)
{
    CompWindowPtr   cw = GetCompWindow (pWin);
    ScreenPtr	    pScreen = pWin->drawable.pScreen;
    WindowPtr	    pParent = pWin->parent;
    PixmapPtr	    pSrcPixmap = (*pScreen->GetWindowPixmap) (pWin);
    PictFormatPtr   pSrcFormat = compWindowFormat (pWin);
    PictFormatPtr   pDstFormat = compWindowFormat (pWin->parent);
    int		    error;
    RegionPtr	    pRegion = DamageRegion (cw->damage);
    PicturePtr	    pSrcPicture = CreatePicture (0, &pSrcPixmap->drawable,
						 pSrcFormat, 
						 0, 0,
						 serverClient,
						 &error);
    XID		    subwindowMode = IncludeInferiors;
    PicturePtr	    pDstPicture = CreatePicture (0, &pParent->drawable,
						 pDstFormat,
						 CPSubwindowMode, 
						 &subwindowMode,
						 serverClient,
						 &error);
    
    /*
     * First move the region from window to screen coordinates
     */
    REGION_TRANSLATE (pScreen, pRegion, 
		      pWin->drawable.x, pWin->drawable.y);
    
    /*
     * Clip against the "real" border clip
     */
    REGION_INTERSECT (pScreen, pRegion, pRegion, &cw->borderClip);

    /*
     * Now translate from screen to dest coordinates
     */
    REGION_TRANSLATE (pScreen, pRegion, 
		      -pParent->drawable.x, -pParent->drawable.y);
    
    /*
     * Clip the picture
     */
    SetPictureClipRegion (pDstPicture, 0, 0, pRegion);
    
    /*
     * And paint
     */
    CompositePicture (PictOpSrc,
		      pSrcPicture,
		      0,
		      pDstPicture,
		      0, 0, /* src_x, src_y */
		      0, 0, /* msk_x, msk_y */
		      pSrcPixmap->screen_x - pParent->drawable.x,
		      pSrcPixmap->screen_y - pParent->drawable.y,
		      pSrcPixmap->drawable.width,
		      pSrcPixmap->drawable.height);
    FreePicture (pSrcPicture, 0);
    FreePicture (pDstPicture, 0);
    /*
     * Empty the damage region.  This has the nice effect of
     * rendering the translations above harmless
     */
    DamageEmpty (cw->damage);
}

void
compWindowUpdate (WindowPtr pWin)
{
    WindowPtr	pChild;

    for (pChild = pWin->lastChild; pChild; pChild = pChild->prevSib)
	compWindowUpdate (pChild);
    if (pWin->redirectDraw)
    {
	CompWindowPtr	cw = GetCompWindow(pWin);

	if (cw->damaged)
	{
	    compWindowUpdateAutomatic (pWin);
	    cw->damaged = FALSE;
	}
    }
}
