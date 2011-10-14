/*
 * $Id: compext.c,v 1.5 2005/07/03 07:37:34 daniels Exp $
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

static CARD8	CompositeReqCode;
int		CompositeClientPrivateIndex;
RESTYPE		CompositeClientWindowType;
RESTYPE		CompositeClientSubwindowsType;

typedef struct _CompositeClient {
    int	    major_version;
    int	    minor_version;
} CompositeClientRec, *CompositeClientPtr;

#define GetCompositeClient(pClient)    ((CompositeClientPtr) (pClient)->devPrivates[CompositeClientPrivateIndex].ptr)

static void
CompositeClientCallback (CallbackListPtr	*list,
		      pointer		closure,
		      pointer		data)
{
    NewClientInfoRec	*clientinfo = (NewClientInfoRec *) data;
    ClientPtr		pClient = clientinfo->client;
    CompositeClientPtr	pCompositeClient = GetCompositeClient (pClient);

    pCompositeClient->major_version = 0;
    pCompositeClient->minor_version = 0;
}

static void
CompositeResetProc (ExtensionEntry *extEntry)
{
}
    
static int
FreeCompositeClientWindow (pointer value, XID ccwid)
{
    WindowPtr	pWin = value;

    compFreeClientWindow (pWin, ccwid);
    return Success;
}

static int
FreeCompositeClientSubwindows (pointer value, XID ccwid)
{
    WindowPtr	pWin = value;

    compFreeClientSubwindows (pWin, ccwid);
    return Success;
}

static int
ProcCompositeQueryVersion (ClientPtr client)
{
    CompositeClientPtr pCompositeClient = GetCompositeClient (client);
    xCompositeQueryVersionReply rep;
    register int n;
    REQUEST(xCompositeQueryVersionReq);

    REQUEST_SIZE_MATCH(xCompositeQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    if (stuff->majorVersion < COMPOSITE_MAJOR) {
	rep.majorVersion = stuff->majorVersion;
	rep.minorVersion = stuff->minorVersion;
    } else {
	rep.majorVersion = COMPOSITE_MAJOR;
	if (stuff->majorVersion == COMPOSITE_MAJOR && 
	    stuff->minorVersion < COMPOSITE_MINOR)
	    rep.minorVersion = stuff->minorVersion;
	else
	    rep.minorVersion = COMPOSITE_MINOR;
    }
    pCompositeClient->major_version = rep.majorVersion;
    pCompositeClient->minor_version = rep.minorVersion;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swapl(&rep.majorVersion, n);
	swapl(&rep.minorVersion, n);
    }
    WriteToClient(client, sizeof(xCompositeQueryVersionReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcCompositeRedirectWindow (ClientPtr client)
{
    WindowPtr	pWin;
    REQUEST(xCompositeRedirectWindowReq);

    REQUEST_SIZE_MATCH(xCompositeRedirectWindowReq);
    pWin = (WindowPtr) LookupIDByType (stuff->window, RT_WINDOW);
    if (!pWin)
    {
	client->errorValue = stuff->window;
	return BadWindow;
    }
    return compRedirectWindow (client, pWin, stuff->update);
}

static int
ProcCompositeRedirectSubwindows (ClientPtr client)
{
    WindowPtr	pWin;
    REQUEST(xCompositeRedirectSubwindowsReq);

    REQUEST_SIZE_MATCH(xCompositeRedirectSubwindowsReq);
    pWin = (WindowPtr) LookupIDByType (stuff->window, RT_WINDOW);
    if (!pWin)
    {
	client->errorValue = stuff->window;
	return BadWindow;
    }
    return compRedirectSubwindows (client, pWin, stuff->update);
}

static int
ProcCompositeUnredirectWindow (ClientPtr client)
{
    WindowPtr	pWin;
    REQUEST(xCompositeUnredirectWindowReq);

    REQUEST_SIZE_MATCH(xCompositeUnredirectWindowReq);
    pWin = (WindowPtr) LookupIDByType (stuff->window, RT_WINDOW);
    if (!pWin)
    {
	client->errorValue = stuff->window;
	return BadWindow;
    }
    return compUnredirectWindow (client, pWin, stuff->update);
}

static int
ProcCompositeUnredirectSubwindows (ClientPtr client)
{
    WindowPtr	pWin;
    REQUEST(xCompositeUnredirectSubwindowsReq);

    REQUEST_SIZE_MATCH(xCompositeUnredirectSubwindowsReq);
    pWin = (WindowPtr) LookupIDByType (stuff->window, RT_WINDOW);
    if (!pWin)
    {
	client->errorValue = stuff->window;
	return BadWindow;
    }
    return compUnredirectSubwindows (client, pWin, stuff->update);
}

static int
ProcCompositeCreateRegionFromBorderClip (ClientPtr client)
{
    WindowPtr	    pWin;
    CompWindowPtr   cw;
    RegionPtr	    pBorderClip, pRegion;
    REQUEST(xCompositeCreateRegionFromBorderClipReq);

    REQUEST_SIZE_MATCH(xCompositeCreateRegionFromBorderClipReq);
    pWin = (WindowPtr) LookupIDByType (stuff->window, RT_WINDOW);
    if (!pWin)
    {
	client->errorValue = stuff->window;
	return BadWindow;
    }
    
    LEGAL_NEW_RESOURCE (stuff->region, client);
    
    cw = GetCompWindow (pWin);
    if (cw)
	pBorderClip = &cw->borderClip;
    else
	pBorderClip = &pWin->borderClip;
    pRegion = XFixesRegionCopy (pBorderClip);
    if (!pRegion)
	return BadAlloc;
    REGION_TRANSLATE (pScreen, pRegion, -pWin->drawable.x, -pWin->drawable.y);
    
    if (!AddResource (stuff->region, RegionResType, (pointer) pRegion))
	return BadAlloc;

    return(client->noClientException);
}

static int
ProcCompositeNameWindowPixmap (ClientPtr client)
{
    WindowPtr	    pWin;
    CompWindowPtr   cw;
    PixmapPtr	    pPixmap;
    REQUEST(xCompositeNameWindowPixmapReq);

    REQUEST_SIZE_MATCH(xCompositeNameWindowPixmapReq);
    pWin = (WindowPtr) LookupIDByType (stuff->window, RT_WINDOW);
    if (!pWin)
    {
	client->errorValue = stuff->window;
	return BadWindow;
    }
    
    LEGAL_NEW_RESOURCE (stuff->pixmap, client);
    
    cw = GetCompWindow (pWin);
    if (!cw)
	return BadMatch;

    pPixmap = (*pWin->drawable.pScreen->GetWindowPixmap) (pWin);
    if (!pPixmap)
	return BadMatch;

    ++pPixmap->refcnt;
    
    if (!AddResource (stuff->pixmap, RT_PIXMAP, (pointer) pPixmap))
	return BadAlloc;

    return(client->noClientException);
}

int (*ProcCompositeVector[CompositeNumberRequests])(ClientPtr) = {
    ProcCompositeQueryVersion,
    ProcCompositeRedirectWindow,
    ProcCompositeRedirectSubwindows,
    ProcCompositeUnredirectWindow,
    ProcCompositeUnredirectSubwindows,
    ProcCompositeCreateRegionFromBorderClip,
    ProcCompositeNameWindowPixmap,
};

static int
ProcCompositeDispatch (ClientPtr client)
{
    REQUEST(xReq);
    
    if (stuff->data < CompositeNumberRequests)
	return (*ProcCompositeVector[stuff->data]) (client);
    else
	return BadRequest;
}

static int
SProcCompositeQueryVersion (ClientPtr client)
{
    int n;
    REQUEST(xCompositeQueryVersionReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xCompositeQueryVersionReq);
    swapl(&stuff->majorVersion, n);
    swapl(&stuff->minorVersion, n);
    return (*ProcCompositeVector[stuff->compositeReqType]) (client);
}

static int
SProcCompositeRedirectWindow (ClientPtr client)
{
    int n;
    REQUEST(xCompositeRedirectWindowReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xCompositeRedirectWindowReq);
    swapl (&stuff->window, n);
    return (*ProcCompositeVector[stuff->compositeReqType]) (client);
}

static int
SProcCompositeRedirectSubwindows (ClientPtr client)
{
    int n;
    REQUEST(xCompositeRedirectSubwindowsReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xCompositeRedirectSubwindowsReq);
    swapl (&stuff->window, n);
    return (*ProcCompositeVector[stuff->compositeReqType]) (client);
}

static int
SProcCompositeUnredirectWindow (ClientPtr client)
{
    int n;
    REQUEST(xCompositeUnredirectWindowReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xCompositeUnredirectWindowReq);
    swapl (&stuff->window, n);
    return (*ProcCompositeVector[stuff->compositeReqType]) (client);
}

static int
SProcCompositeUnredirectSubwindows (ClientPtr client)
{
    int n;
    REQUEST(xCompositeUnredirectSubwindowsReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xCompositeUnredirectSubwindowsReq);
    swapl (&stuff->window, n);
    return (*ProcCompositeVector[stuff->compositeReqType]) (client);
}

static int
SProcCompositeCreateRegionFromBorderClip (ClientPtr client)
{
    int n;
    REQUEST(xCompositeCreateRegionFromBorderClipReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xCompositeCreateRegionFromBorderClipReq);
    swapl (&stuff->region, n);
    swapl (&stuff->window, n);
    return (*ProcCompositeVector[stuff->compositeReqType]) (client);
}

static int
SProcCompositeNameWindowPixmap (ClientPtr client)
{
    int n;
    REQUEST(xCompositeNameWindowPixmapReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xCompositeNameWindowPixmapReq);
    swapl (&stuff->window, n);
    swapl (&stuff->pixmap, n);
    return (*ProcCompositeVector[stuff->compositeReqType]) (client);
}

int (*SProcCompositeVector[CompositeNumberRequests])(ClientPtr) = {
    SProcCompositeQueryVersion,
    SProcCompositeRedirectWindow,
    SProcCompositeRedirectSubwindows,
    SProcCompositeUnredirectWindow,
    SProcCompositeUnredirectSubwindows,
    SProcCompositeCreateRegionFromBorderClip,
    SProcCompositeNameWindowPixmap,
};

static int
SProcCompositeDispatch (ClientPtr client)
{
    REQUEST(xReq);
    
    if (stuff->data < CompositeNumberRequests)
	return (*SProcCompositeVector[stuff->data]) (client);
    else
	return BadRequest;
}

void
CompositeExtensionInit (void)
{
    ExtensionEntry  *extEntry;
    int		    s;

    CompositeClientWindowType = CreateNewResourceType (FreeCompositeClientWindow);
    if (!CompositeClientWindowType)
	return;

    CompositeClientSubwindowsType = CreateNewResourceType (FreeCompositeClientSubwindows);
    if (!CompositeClientSubwindowsType)
	return;

    CompositeClientPrivateIndex = AllocateClientPrivateIndex ();
    if (!AllocateClientPrivate (CompositeClientPrivateIndex, 
				sizeof (CompositeClientRec)))
	return;
    if (!AddCallback (&ClientStateCallback, CompositeClientCallback, 0))
	return;

    extEntry = AddExtension (COMPOSITE_NAME, 0, 0,
			     ProcCompositeDispatch, SProcCompositeDispatch,
			     CompositeResetProc, StandardMinorOpcode);
    if (!extEntry)
	return;
    CompositeReqCode = (CARD8) extEntry->base;

    
    for (s = 0; s < screenInfo.numScreens; s++)
	if (!compScreenInit (screenInfo.screens[s]))
	    return;
    miRegisterRedirectBorderClipProc (compSetRedirectBorderClip,
				      compGetRedirectBorderClip);
}
