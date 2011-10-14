/*
 * $Id: damageext.c,v 1.6 2005/07/03 07:37:35 daniels Exp $
 *
 * Copyright © 2002 Keith Packard
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

#include "damageextint.h"

unsigned char	DamageReqCode;
int		DamageEventBase;
int		DamageErrorBase;
int		DamageClientPrivateIndex;
RESTYPE		DamageExtType;
RESTYPE		DamageExtWinType;

#define prScreen	screenInfo.screens[0]

static void
DamageExtNotify (DamageExtPtr pDamageExt, BoxPtr pBoxes, int nBoxes)
{
    ClientPtr		pClient = pDamageExt->pClient;
    DamageClientPtr	pDamageClient = GetDamageClient (pClient);
    DrawablePtr		pDrawable = pDamageExt->pDrawable;
    xDamageNotifyEvent	ev;
    int			i;

    UpdateCurrentTimeIf ();
    ev.type = DamageEventBase + XDamageNotify;
    ev.level = pDamageExt->level;
    ev.sequenceNumber = pClient->sequence;
    ev.drawable = pDrawable->id;
    ev.damage = pDamageExt->id;
    ev.timestamp = currentTime.milliseconds;
    ev.geometry.x = pDrawable->x;
    ev.geometry.y = pDrawable->y;
    ev.geometry.width = pDrawable->width;
    ev.geometry.height = pDrawable->height;
    if (pBoxes)
    {
	for (i = 0; i < nBoxes; i++)
	{
	    ev.level = pDamageExt->level;
	    if (i < nBoxes - 1)
		ev.level |= DamageNotifyMore;
	    ev.area.x = pBoxes[i].x1;
	    ev.area.y = pBoxes[i].y1;
	    ev.area.width = pBoxes[i].x2 - pBoxes[i].x1;
	    ev.area.height = pBoxes[i].y2 - pBoxes[i].y1;
	    if (!pClient->clientGone)
		WriteEventsToClient (pClient, 1, (xEvent *) &ev);
	}
    }
    else
    {
	ev.area.x = 0;
	ev.area.y = 0;
	ev.area.width = pDrawable->width;
	ev.area.height = pDrawable->height;
	if (!pClient->clientGone)
	    WriteEventsToClient (pClient, 1, (xEvent *) &ev);
    }
    /* Composite extension marks clients with manual Subwindows as critical */
    if (pDamageClient->critical > 0)
    {
	SetCriticalOutputPending ();
#ifdef SMART_SCHEDULE
	pClient->smart_priority = SMART_MAX_PRIORITY;
#endif
    }
}

static void
DamageExtReport (DamagePtr pDamage, RegionPtr pRegion, void *closure)
{
    DamageExtPtr    pDamageExt = closure;

    switch (pDamageExt->level) {
    case DamageReportRawRegion:
    case DamageReportDeltaRegion:
	DamageExtNotify (pDamageExt, REGION_RECTS(pRegion), REGION_NUM_RECTS(pRegion));
	break;
    case DamageReportBoundingBox:
	DamageExtNotify (pDamageExt, REGION_EXTENTS(prScreen, pRegion), 1);
	break;
    case DamageReportNonEmpty:
	DamageExtNotify (pDamageExt, NullBox, 0);
	break;
    case DamageReportNone:
	break;
    }
}

static void
DamageExtDestroy (DamagePtr pDamage, void *closure)
{
    DamageExtPtr    pDamageExt = closure;
    
    pDamageExt->pDamage = 0;
    if (pDamageExt->id)
	FreeResource (pDamageExt->id, RT_NONE);
}

void
DamageExtSetCritical (ClientPtr pClient, Bool critical)
{
    DamageClientPtr pDamageClient = GetDamageClient (pClient);

    if (pDamageClient)
	pDamageClient->critical += critical ? 1 : -1;
}

static int
ProcDamageQueryVersion(ClientPtr client)
{
    DamageClientPtr pDamageClient = GetDamageClient (client);
    xDamageQueryVersionReply rep;
    register int n;
    REQUEST(xDamageQueryVersionReq);

    REQUEST_SIZE_MATCH(xDamageQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    if (stuff->majorVersion < DAMAGE_MAJOR) {
	rep.majorVersion = stuff->majorVersion;
	rep.minorVersion = stuff->minorVersion;
    } else {
	rep.majorVersion = DAMAGE_MAJOR;
	if (stuff->majorVersion == DAMAGE_MAJOR && 
	    stuff->minorVersion < DAMAGE_MINOR)
	    rep.minorVersion = stuff->minorVersion;
	else
	    rep.minorVersion = DAMAGE_MINOR;
    }
    pDamageClient->major_version = rep.majorVersion;
    pDamageClient->minor_version = rep.minorVersion;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swapl(&rep.majorVersion, n);
	swapl(&rep.minorVersion, n);
    }
    WriteToClient(client, sizeof(xDamageQueryVersionReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcDamageCreate (ClientPtr client)
{
    DrawablePtr		pDrawable;
    DamageExtPtr	pDamageExt;
    DamageReportLevel	level;
    RegionPtr		pRegion;
    
    REQUEST(xDamageCreateReq);

    REQUEST_SIZE_MATCH(xDamageCreateReq);
    LEGAL_NEW_RESOURCE(stuff->damage, client);
    SECURITY_VERIFY_DRAWABLE (pDrawable, stuff->drawable, client,
			      SecurityReadAccess);
    switch (stuff->level) {
    case XDamageReportRawRectangles:
	level = DamageReportRawRegion;
	break;
    case XDamageReportDeltaRectangles:
	level = DamageReportDeltaRegion;
	break;
    case XDamageReportBoundingBox:
	level = DamageReportBoundingBox;
	break;
    case XDamageReportNonEmpty:
	level = DamageReportNonEmpty;
	break;
    default:
	client->errorValue = stuff->level;
	return BadValue;
    }
    
    pDamageExt = xalloc (sizeof (DamageExtRec));
    if (!pDamageExt)
	return BadAlloc;
    pDamageExt->id = stuff->damage;
    pDamageExt->pDrawable = pDrawable;
    pDamageExt->level = level;
    pDamageExt->pClient = client;
    pDamageExt->pDamage = DamageCreate (DamageExtReport,
					DamageExtDestroy,
					level,
					FALSE,
					pDrawable->pScreen,
					pDamageExt);
    if (!pDamageExt->pDamage)
    {
	xfree (pDamageExt);
	return BadAlloc;
    }
    if (!AddResource (stuff->damage, DamageExtType, (pointer) pDamageExt))
	return BadAlloc;

    DamageRegister (pDamageExt->pDrawable, pDamageExt->pDamage);

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	pRegion = &((WindowPtr) pDrawable)->borderClip;
	DamageDamageRegion (pDrawable, pRegion);
    }

    return (client->noClientException);
}

static int
ProcDamageDestroy (ClientPtr client)
{
    REQUEST(xDamageDestroyReq);
    DamageExtPtr    pDamageExt;

    REQUEST_SIZE_MATCH(xDamageDestroyReq);
    VERIFY_DAMAGEEXT(pDamageExt, stuff->damage, client, SecurityWriteAccess);
    FreeResource (stuff->damage, RT_NONE);
    return (client->noClientException);
}

static int
ProcDamageSubtract (ClientPtr client)
{
    REQUEST(xDamageSubtractReq);
    DamageExtPtr    pDamageExt;
    RegionPtr	    pRepair;
    RegionPtr	    pParts;

    REQUEST_SIZE_MATCH(xDamageSubtractReq);
    VERIFY_DAMAGEEXT(pDamageExt, stuff->damage, client, SecurityWriteAccess);
    VERIFY_REGION_OR_NONE(pRepair, stuff->repair, client, SecurityWriteAccess);
    VERIFY_REGION_OR_NONE(pParts, stuff->parts, client, SecurityWriteAccess);

    if (pDamageExt->level != DamageReportRawRegion)
    {
	DamagePtr   pDamage = pDamageExt->pDamage;
	if (pRepair)
	{
	    if (pParts)
		REGION_INTERSECT (prScreen, pParts, DamageRegion (pDamage), pRepair);
	    if (DamageSubtract (pDamage, pRepair))
		DamageExtReport (pDamage, DamageRegion (pDamage), (void *) pDamageExt);
	}
	else
	{
	    if (pParts)
		REGION_COPY (prScreen, pParts, DamageRegion (pDamage));
	    DamageEmpty (pDamage);
	}
    }
    return (client->noClientException);
}

/* Major version controls available requests */
static const int version_requests[] = {
    X_DamageQueryVersion,	/* before client sends QueryVersion */
    X_DamageSubtract,		/* Version 1 */
};

#define NUM_VERSION_REQUESTS	(sizeof (version_requests) / sizeof (version_requests[0]))
    
int	(*ProcDamageVector[XDamageNumberRequests])(ClientPtr) = {
/*************** Version 1 ******************/
    ProcDamageQueryVersion,
    ProcDamageCreate,
    ProcDamageDestroy,
    ProcDamageSubtract,
};


static int
ProcDamageDispatch (ClientPtr client)
{
    REQUEST(xDamageReq);
    DamageClientPtr pDamageClient = GetDamageClient (client);

    if (pDamageClient->major_version > NUM_VERSION_REQUESTS)
	return BadRequest;
    if (stuff->damageReqType > version_requests[pDamageClient->major_version])
	return BadRequest;
    return (*ProcDamageVector[stuff->damageReqType]) (client);
}

static int
SProcDamageQueryVersion(ClientPtr client)
{
    register int n;
    REQUEST(xDamageQueryVersionReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDamageQueryVersionReq);
    swapl(&stuff->majorVersion, n);
    swapl(&stuff->minorVersion, n);
    return (*ProcDamageVector[stuff->damageReqType]) (client);
}

static int
SProcDamageCreate (ClientPtr client)
{
    register int n;
    REQUEST(xDamageCreateReq);
    
    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH(xDamageCreateReq);
    swapl (&stuff->damage, n);
    swapl (&stuff->drawable, n);
    return (*ProcDamageVector[stuff->damageReqType]) (client);
}

static int
SProcDamageDestroy (ClientPtr client)
{
    register int n;
    REQUEST(xDamageDestroyReq);
    
    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH(xDamageDestroyReq);
    swapl (&stuff->damage, n);
    return (*ProcDamageVector[stuff->damageReqType]) (client);
}

static int
SProcDamageSubtract (ClientPtr client)
{
    register int n;
    REQUEST(xDamageSubtractReq);
    
    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH(xDamageSubtractReq);
    swapl (&stuff->damage, n);
    swapl (&stuff->repair, n);
    swapl (&stuff->parts, n);
    return (*ProcDamageVector[stuff->damageReqType]) (client);
}

int	(*SProcDamageVector[XDamageNumberRequests])(ClientPtr) = {
/*************** Version 1 ******************/
    SProcDamageQueryVersion,
    SProcDamageCreate,
    SProcDamageDestroy,
    SProcDamageSubtract,
};

static int
SProcDamageDispatch (ClientPtr client)
{
    REQUEST(xDamageReq);
    if (stuff->damageReqType >= XDamageNumberRequests)
	return BadRequest;
    return (*SProcDamageVector[stuff->damageReqType]) (client);
}

static void
DamageClientCallback (CallbackListPtr	*list,
		      pointer		closure,
		      pointer		data)
{
    NewClientInfoRec	*clientinfo = (NewClientInfoRec *) data;
    ClientPtr		pClient = clientinfo->client;
    DamageClientPtr	pDamageClient = GetDamageClient (pClient);

    pDamageClient->critical = 0;
    pDamageClient->major_version = 0;
    pDamageClient->minor_version = 0;
}

/*ARGSUSED*/
static void
DamageResetProc (ExtensionEntry *extEntry)
{
    DeleteCallback (&ClientStateCallback, DamageClientCallback, 0);
}

static int
FreeDamageExt (pointer value, XID did)
{
    DamageExtPtr    pDamageExt = (DamageExtPtr) value;

    /*
     * Get rid of the resource table entry hanging from the window id
     */
    pDamageExt->id = 0;
    if (WindowDrawable(pDamageExt->pDrawable->type))
	FreeResourceByType (pDamageExt->pDrawable->id, DamageExtWinType, TRUE);
    if (pDamageExt->pDamage)
    {
	DamageUnregister (pDamageExt->pDrawable, pDamageExt->pDamage);
	DamageDestroy (pDamageExt->pDamage);
    }
    xfree (pDamageExt);
    return Success;
}

static int
FreeDamageExtWin (pointer value, XID wid)
{
    DamageExtPtr    pDamageExt = (DamageExtPtr) value;

    if (pDamageExt->id)
	FreeResource (pDamageExt->id, RT_NONE);
    return Success;
}

void
SDamageNotifyEvent (xDamageNotifyEvent *from,
		    xDamageNotifyEvent *to)
{
    to->type = from->type;
    cpswaps (from->sequenceNumber, to->sequenceNumber);
    cpswapl (from->drawable, to->drawable);
    cpswapl (from->damage, to->damage);
    cpswaps (from->area.x, to->area.x);
    cpswaps (from->area.y, to->area.y);
    cpswaps (from->area.width, to->area.width);
    cpswaps (from->area.height, to->area.height);
    cpswaps (from->geometry.x, to->geometry.x);
    cpswaps (from->geometry.y, to->geometry.y);
    cpswaps (from->geometry.width, to->geometry.width);
    cpswaps (from->geometry.height, to->geometry.height);
}

void
DamageExtensionInit(void)
{
    ExtensionEntry *extEntry;
    int		    s;

    for (s = 0; s < screenInfo.numScreens; s++)
	DamageSetup (screenInfo.screens[s]);

    DamageExtType = CreateNewResourceType (FreeDamageExt);
    if (!DamageExtType)
	return;

    DamageExtWinType = CreateNewResourceType (FreeDamageExtWin);
    if (!DamageExtWinType)
	return;

    DamageClientPrivateIndex = AllocateClientPrivateIndex ();
    if (!AllocateClientPrivate (DamageClientPrivateIndex, 
				sizeof (DamageClientRec)))
	return;
    if (!AddCallback (&ClientStateCallback, DamageClientCallback, 0))
	return;

    if ((extEntry = AddExtension(DAMAGE_NAME, XDamageNumberEvents, 
				 XDamageNumberErrors,
				 ProcDamageDispatch, SProcDamageDispatch,
				 DamageResetProc, StandardMinorOpcode)) != 0)
    {
	DamageReqCode = (unsigned char)extEntry->base;
	DamageEventBase = extEntry->eventBase;
	DamageErrorBase = extEntry->errorBase;
	EventSwapVector[DamageEventBase + XDamageNotify] =
			(EventSwapPtr) SDamageNotifyEvent;
    }
}
