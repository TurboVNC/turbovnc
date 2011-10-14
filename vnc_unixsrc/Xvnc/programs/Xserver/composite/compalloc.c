/*
 * $Id: compalloc.c,v 1.7 2005/07/03 07:37:34 daniels Exp $
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

void
compReportDamage (DamagePtr pDamage, RegionPtr pRegion, void *closure)
{
    WindowPtr	    pWin = (WindowPtr) closure;
    ScreenPtr	    pScreen = pWin->drawable.pScreen;
    CompScreenPtr   cs = GetCompScreen (pScreen);
    CompWindowPtr   cw = GetCompWindow (pWin);

    cs->damaged = TRUE;
    cw->damaged = TRUE;
}

static void
compDestroyDamage (DamagePtr pDamage, void *closure)
{
    WindowPtr	    pWin = (WindowPtr) closure;
    CompWindowPtr   cw = GetCompWindow (pWin);

    cw->damage = 0;
}

/*
 * Redirect one window for one client
 */
int
compRedirectWindow (ClientPtr pClient, WindowPtr pWin, int update)
{
    CompWindowPtr	cw = GetCompWindow (pWin);
    CompClientWindowPtr	ccw;
    Bool		wasMapped = pWin->mapped;

    /*
     * Only one Manual update is allowed
     */
    if (cw && update == CompositeRedirectManual)
	for (ccw = cw->clients; ccw; ccw = ccw->next)
	    if (ccw->update == CompositeRedirectManual)
		return BadAccess;
    
    /*
     * Allocate per-client per-window structure 
     * The client *could* allocate multiple, but while supported,
     * it is not expected to be common
     */
    ccw = xalloc (sizeof (CompClientWindowRec));
    if (!ccw)
	return BadAlloc;
    ccw->id = FakeClientID (pClient->index);
    ccw->update = update;
    /*
     * Now make sure there's a per-window structure to hang this from
     */
    if (!cw)
    {
	cw = xalloc (sizeof (CompWindowRec));
	if (!cw)
	{
	    xfree (ccw);
	    return BadAlloc;
	}
	cw->damage = DamageCreate (compReportDamage,
				   compDestroyDamage,
				   DamageReportNonEmpty,
				   FALSE,
				   pWin->drawable.pScreen,
				   pWin);
	if (!cw->damage)
	{
	    xfree (ccw);
	    xfree (cw);
	    return BadAlloc;
	}
	if (wasMapped)
	    UnmapWindow (pWin, FALSE);

	REGION_NULL (pScreen, &cw->borderClip);
	cw->update = CompositeRedirectAutomatic;
	cw->clients = 0;
	cw->oldx = COMP_ORIGIN_INVALID;
	cw->oldy = COMP_ORIGIN_INVALID;
	cw->damageRegistered = FALSE;
	cw->damaged = FALSE;
	pWin->devPrivates[CompWindowPrivateIndex].ptr = cw;
    }
    ccw->next = cw->clients;
    cw->clients = ccw;
    if (!AddResource (ccw->id, CompositeClientWindowType, pWin))
	return BadAlloc;
    if (ccw->update == CompositeRedirectManual)
    {
	if (cw->damageRegistered)
	{
	    DamageUnregister (&pWin->drawable, cw->damage);
	    cw->damageRegistered = FALSE;
	}
	cw->update = CompositeRedirectManual;
    }

    if (!compCheckRedirect (pWin))
    {
	FreeResource (ccw->id, RT_NONE);
	return BadAlloc;
    }
    if (wasMapped && !pWin->mapped)
    {
	Bool	overrideRedirect = pWin->overrideRedirect;
	pWin->overrideRedirect = TRUE;
	MapWindow (pWin, pClient);
	pWin->overrideRedirect = overrideRedirect;
    }
    
    return Success;
}

/*
 * Free one of the per-client per-window resources, clearing
 * redirect and the per-window pointer as appropriate
 */
void
compFreeClientWindow (WindowPtr pWin, XID id)
{
    CompWindowPtr	cw = GetCompWindow (pWin);
    CompClientWindowPtr	ccw, *prev;
    Bool		wasMapped = pWin->mapped;

    if (!cw)
	return;
    for (prev = &cw->clients; (ccw = *prev); prev = &ccw->next)
    {
	if (ccw->id == id)
	{
	    *prev = ccw->next;
	    if (ccw->update == CompositeRedirectManual)
		cw->update = CompositeRedirectAutomatic;
	    xfree (ccw);
	    break;
	}
    }
    if (!cw->clients)
    {
	if (wasMapped)
	    UnmapWindow (pWin, FALSE);
    
	if (pWin->redirectDraw)
	    compFreePixmap (pWin);

	if (cw->damage)
	    DamageDestroy (cw->damage);
	
	REGION_UNINIT (pScreen, &cw->borderClip);
    
	pWin->devPrivates[CompWindowPrivateIndex].ptr = 0;
	xfree (cw);
    }
    else if (cw->update == CompositeRedirectAutomatic &&
	     !cw->damageRegistered && pWin->redirectDraw)
    {
	DamageRegister (&pWin->drawable, cw->damage);
	cw->damageRegistered = TRUE;
	DamageDamageRegion (&pWin->drawable, &pWin->borderSize);
    }
    if (wasMapped && !pWin->mapped)
    {
	Bool	overrideRedirect = pWin->overrideRedirect;
	pWin->overrideRedirect = TRUE;
	MapWindow (pWin, clients[CLIENT_ID(id)]);
	pWin->overrideRedirect = overrideRedirect;
    }
}

/*
 * This is easy, just free the appropriate resource.
 */

int
compUnredirectWindow (ClientPtr pClient, WindowPtr pWin, int update)
{
    CompWindowPtr	cw = GetCompWindow (pWin);
    CompClientWindowPtr	ccw;

    if (!cw)
	return BadValue;

    for (ccw = cw->clients; ccw; ccw = ccw->next)
	if (ccw->update == update && CLIENT_ID(ccw->id) == pClient->index)
	{
	    FreeResource (ccw->id, RT_NONE);
	    return Success;
	}
    return BadValue;
}
	
/*
 * Redirect all subwindows for one client
 */

int
compRedirectSubwindows (ClientPtr pClient, WindowPtr pWin, int update)
{
    CompSubwindowsPtr	csw = GetCompSubwindows (pWin);
    CompClientWindowPtr	ccw;
    WindowPtr		pChild;

    /*
     * Only one Manual update is allowed
     */
    if (csw && update == CompositeRedirectManual)
	for (ccw = csw->clients; ccw; ccw = ccw->next)
	    if (ccw->update == CompositeRedirectManual)
		return BadAccess;
    /*
     * Allocate per-client per-window structure 
     * The client *could* allocate multiple, but while supported,
     * it is not expected to be common
     */
    ccw = xalloc (sizeof (CompClientWindowRec));
    if (!ccw)
	return BadAlloc;
    ccw->id = FakeClientID (pClient->index);
    ccw->update = update;
    /*
     * Now make sure there's a per-window structure to hang this from
     */
    if (!csw)
    {
	csw = xalloc (sizeof (CompSubwindowsRec));
	if (!csw)
	{
	    xfree (ccw);
	    return BadAlloc;
	}
	csw->update = CompositeRedirectAutomatic;
	csw->clients = 0;
	pWin->devPrivates[CompSubwindowsPrivateIndex].ptr = csw;
    }
    /*
     * Redirect all existing windows
     */
    for (pChild = pWin->lastChild; pChild; pChild = pChild->prevSib)
    {
	int ret = compRedirectWindow (pClient, pChild, update);
	if (ret != Success)
	{
	    for (pChild = pChild->nextSib; pChild; pChild = pChild->nextSib)
		(void) compUnredirectWindow (pClient, pChild, update);
	    if (!csw->clients)
	    {
		xfree (csw);
		pWin->devPrivates[CompSubwindowsPrivateIndex].ptr = 0;
	    }
	    xfree (ccw);
	    return ret;
	}
    }
    /*
     * Hook into subwindows list
     */
    ccw->next = csw->clients;
    csw->clients = ccw;
    if (!AddResource (ccw->id, CompositeClientSubwindowsType, pWin))
	return BadAlloc;
    if (ccw->update == CompositeRedirectManual)
    {
	csw->update = CompositeRedirectManual;
	/* 
	 * tell damage extension that damage events for this client are
	 * critical output
	 */
	DamageExtSetCritical (pClient, TRUE);
    }
    return Success;
}

/*
 * Free one of the per-client per-subwindows resources,
 * which frees one redirect per subwindow
 */
void
compFreeClientSubwindows (WindowPtr pWin, XID id)
{
    CompSubwindowsPtr	csw = GetCompSubwindows (pWin);
    CompClientWindowPtr	ccw, *prev;
    WindowPtr		pChild;

    if (!csw)
	return;
    for (prev = &csw->clients; (ccw = *prev); prev = &ccw->next)
    {
	if (ccw->id == id)
	{
	    ClientPtr	pClient = clients[CLIENT_ID(id)];
	    
	    *prev = ccw->next;
	    if (ccw->update == CompositeRedirectManual)
	    {
		/* 
		 * tell damage extension that damage events for this client are
		 * critical output
		 */
		DamageExtSetCritical (pClient, FALSE);
		csw->update = CompositeRedirectAutomatic;
		if (pWin->mapped)
		    (*pWin->drawable.pScreen->ClearToBackground)(pWin, 0, 0, 0, 0, TRUE);
	    }

	    /*
	     * Unredirect all existing subwindows
	     */
	    for (pChild = pWin->lastChild; pChild; pChild = pChild->prevSib)
		(void) compUnredirectWindow (pClient, pChild, ccw->update);

	    xfree (ccw);
	    break;
	}
    }

    /*
     * Check if all of the per-client records are gone
     */
    if (!csw->clients)
    {
	pWin->devPrivates[CompSubwindowsPrivateIndex].ptr = 0;
	xfree (csw);
    }
}

/*
 * This is easy, just free the appropriate resource.
 */

int
compUnredirectSubwindows (ClientPtr pClient, WindowPtr pWin, int update)
{
    CompSubwindowsPtr	csw = GetCompSubwindows (pWin);
    CompClientWindowPtr	ccw;
    
    if (!csw)
	return BadValue;
    for (ccw = csw->clients; ccw; ccw = ccw->next)
	if (ccw->update == update && CLIENT_ID(ccw->id) == pClient->index)
	{
	    FreeResource (ccw->id, RT_NONE);
	    return Success;
	}
    return BadValue;
}

/*
 * Add redirection information for one subwindow (during reparent)
 */

int
compRedirectOneSubwindow (WindowPtr pParent, WindowPtr pWin)
{
    CompSubwindowsPtr	csw = GetCompSubwindows (pParent);
    CompClientWindowPtr	ccw;

    if (!csw)
	return Success;
    for (ccw = csw->clients; ccw; ccw = ccw->next)
    {
	int ret = compRedirectWindow (clients[CLIENT_ID(ccw->id)],
				      pWin, ccw->update);
	if (ret != Success)
	    return ret;
    }
    return Success;
}

/*
 * Remove redirection information for one subwindow (during reparent)
 */

int
compUnredirectOneSubwindow (WindowPtr pParent, WindowPtr pWin)
{
    CompSubwindowsPtr	csw = GetCompSubwindows (pParent);
    CompClientWindowPtr	ccw;

    if (!csw)
	return Success;
    for (ccw = csw->clients; ccw; ccw = ccw->next)
    {
	int ret = compUnredirectWindow (clients[CLIENT_ID(ccw->id)],
					pWin, ccw->update);
	if (ret != Success)
	    return ret;
    }
    return Success;
}

static PixmapPtr
compNewPixmap (WindowPtr pWin, int x, int y, int w, int h)
{
    ScreenPtr	    pScreen = pWin->drawable.pScreen;
    WindowPtr	    pParent = pWin->parent;
    PixmapPtr	    pPixmap;
    GCPtr	    pGC;

    pPixmap = (*pScreen->CreatePixmap) (pScreen, w, h, pWin->drawable.depth);

    if (!pPixmap)
	return 0;
    
    pPixmap->screen_x = x;
    pPixmap->screen_y = y;
    
    pGC = GetScratchGC (pWin->drawable.depth, pScreen);
    
    /*
     * Copy bits from the parent into the new pixmap so that it will
     * have "reasonable" contents in case for background None areas.
     */
    if (pGC)
    {
	XID val = IncludeInferiors;
	
	ValidateGC(&pPixmap->drawable, pGC);
	dixChangeGC (serverClient, pGC, GCSubwindowMode, &val, NULL);
	(*pGC->ops->CopyArea) (&pParent->drawable,
			       &pPixmap->drawable,
			       pGC,
			       x - pParent->drawable.x,
			       y - pParent->drawable.y,
			       w, h, 0, 0);
	FreeScratchGC (pGC);
    }
    return pPixmap;
}

Bool
compAllocPixmap (WindowPtr pWin)
{
    int		    bw = (int) pWin->borderWidth;
    int		    x = pWin->drawable.x - bw;
    int		    y = pWin->drawable.y - bw;
    int		    w = pWin->drawable.width + (bw << 1);
    int		    h = pWin->drawable.height + (bw << 1);
    PixmapPtr	    pPixmap = compNewPixmap (pWin, x, y, w, h);
    CompWindowPtr   cw = GetCompWindow (pWin);

    if (!pPixmap)
	return FALSE;
    pWin->redirectDraw = TRUE;
    compSetPixmap (pWin, pPixmap);
    cw->oldx = COMP_ORIGIN_INVALID;
    cw->oldy = COMP_ORIGIN_INVALID;
    cw->damageRegistered = FALSE;
    if (cw->update == CompositeRedirectAutomatic)
    {
	DamageRegister (&pWin->drawable, cw->damage);
	cw->damageRegistered = TRUE;
    }
    return TRUE;
}

void
compFreePixmap (WindowPtr pWin)
{
    ScreenPtr	    pScreen = pWin->drawable.pScreen;
    PixmapPtr	    pRedirectPixmap, pParentPixmap;
    CompWindowPtr   cw = GetCompWindow (pWin);

    if (cw->damageRegistered)
    {
	DamageUnregister (&pWin->drawable, cw->damage);
	cw->damageRegistered = FALSE;
	DamageEmpty (cw->damage);
    }
    /*
     * Move the parent-constrained border clip region back into
     * the window so that ValidateTree will handle the unmap
     * case correctly.  Unmap adds the window borderClip to the
     * parent exposed area; regions beyond the parent cause crashes
     */
    REGION_COPY (pScreen, &pWin->borderClip, &cw->borderClip);
    pRedirectPixmap = (*pScreen->GetWindowPixmap) (pWin);
    pParentPixmap = (*pScreen->GetWindowPixmap) (pWin->parent);
    pWin->redirectDraw = FALSE;
    compSetPixmap (pWin, pParentPixmap);
    (*pScreen->DestroyPixmap) (pRedirectPixmap);
}

/*
 * Make sure the pixmap is the right size and offset.  Allocate a new
 * pixmap to change size, adjust origin to change offset, leaving the
 * old pixmap in cw->pOldPixmap so bits can be recovered
 */
Bool
compReallocPixmap (WindowPtr pWin, int draw_x, int draw_y,
		   unsigned int w, unsigned int h, int bw)
{
    ScreenPtr	    pScreen = pWin->drawable.pScreen;
    PixmapPtr	    pOld = (*pScreen->GetWindowPixmap) (pWin);
    PixmapPtr	    pNew;
    CompWindowPtr   cw = GetCompWindow (pWin);
    int		    pix_x, pix_y;
    int		    pix_w, pix_h;

    assert (cw && pWin->redirectDraw);
    cw->oldx = pOld->screen_x;
    cw->oldy = pOld->screen_y;
    pix_x = draw_x - bw;
    pix_y = draw_y - bw;
    pix_w = w + (bw << 1);
    pix_h = h + (bw << 1);
    if (pix_w != pOld->drawable.width || pix_h != pOld->drawable.height)
    {
	pNew = compNewPixmap (pWin, pix_x, pix_y, pix_w, pix_h);
	if (!pNew)
	    return FALSE;
	cw->pOldPixmap = pOld;
	compSetPixmap (pWin, pNew);
    }
    else
    {
	pNew = pOld;
	cw->pOldPixmap = 0;
    }
    pNew->screen_x = pix_x;
    pNew->screen_y = pix_y;
    return TRUE;
}
