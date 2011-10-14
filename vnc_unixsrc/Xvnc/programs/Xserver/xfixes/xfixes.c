/*
 * $Id: xfixes.c,v 1.7 2005/07/03 07:37:35 daniels Exp $
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

#include "xfixesint.h"

unsigned char	XFixesReqCode;
int		XFixesEventBase;
int		XFixesErrorBase;
int		XFixesClientPrivateIndex;

static int
ProcXFixesQueryVersion(ClientPtr client)
{
    XFixesClientPtr pXFixesClient = GetXFixesClient (client);
    xXFixesQueryVersionReply rep;
    register int n;
    REQUEST(xXFixesQueryVersionReq);

    REQUEST_SIZE_MATCH(xXFixesQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    if (stuff->majorVersion < XFIXES_MAJOR) {
	rep.majorVersion = stuff->majorVersion;
	rep.minorVersion = stuff->minorVersion;
    } else {
	rep.majorVersion = XFIXES_MAJOR;
	if (stuff->majorVersion == XFIXES_MAJOR && 
	    stuff->minorVersion < XFIXES_MINOR)
	    rep.minorVersion = stuff->minorVersion;
	else
	    rep.minorVersion = XFIXES_MINOR;
    }
    pXFixesClient->major_version = rep.majorVersion;
    pXFixesClient->minor_version = rep.minorVersion;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swapl(&rep.majorVersion, n);
	swapl(&rep.minorVersion, n);
    }
    WriteToClient(client, sizeof(xXFixesQueryVersionReply), (char *)&rep);
    return(client->noClientException);
}

/* Major version controls available requests */
static const int version_requests[] = {
    X_XFixesQueryVersion,	/* before client sends QueryVersion */
    X_XFixesGetCursorImage,	/* Version 1 */
    X_XFixesChangeCursorByName,	/* Version 2 */
    X_XFixesExpandRegion,	/* Version 3 */
};

#define NUM_VERSION_REQUESTS	(sizeof (version_requests) / sizeof (version_requests[0]))
    
int	(*ProcXFixesVector[XFixesNumberRequests])(ClientPtr) = {
/*************** Version 1 ******************/
    ProcXFixesQueryVersion,
    ProcXFixesChangeSaveSet,
    ProcXFixesSelectSelectionInput,
    ProcXFixesSelectCursorInput,
    ProcXFixesGetCursorImage,
/*************** Version 2 ******************/
    ProcXFixesCreateRegion,
    ProcXFixesCreateRegionFromBitmap,
    ProcXFixesCreateRegionFromWindow,
    ProcXFixesCreateRegionFromGC,
    ProcXFixesCreateRegionFromPicture,
    ProcXFixesDestroyRegion,
    ProcXFixesSetRegion,
    ProcXFixesCopyRegion,
    ProcXFixesCombineRegion,
    ProcXFixesCombineRegion,
    ProcXFixesCombineRegion,
    ProcXFixesInvertRegion,
    ProcXFixesTranslateRegion,
    ProcXFixesRegionExtents,
    ProcXFixesFetchRegion,
    ProcXFixesSetGCClipRegion,
    ProcXFixesSetWindowShapeRegion,
    ProcXFixesSetPictureClipRegion,
    ProcXFixesSetCursorName,
    ProcXFixesGetCursorName,
    ProcXFixesGetCursorImageAndName,
    ProcXFixesChangeCursor,
    ProcXFixesChangeCursorByName,
/*************** Version 3 ******************/
    ProcXFixesExpandRegion,
};

static int
ProcXFixesDispatch (ClientPtr client)
{
    REQUEST(xXFixesReq);
    XFixesClientPtr pXFixesClient = GetXFixesClient (client);

    if (pXFixesClient->major_version > NUM_VERSION_REQUESTS)
	return BadRequest;
    if (stuff->xfixesReqType > version_requests[pXFixesClient->major_version])
	return BadRequest;
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

static int
SProcXFixesQueryVersion(ClientPtr client)
{
    register int n;
    REQUEST(xXFixesQueryVersionReq);

    swaps(&stuff->length, n);
    swapl(&stuff->majorVersion, n);
    swapl(&stuff->minorVersion, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int	(*SProcXFixesVector[XFixesNumberRequests])(ClientPtr) = {
/*************** Version 1 ******************/
    SProcXFixesQueryVersion,
    SProcXFixesChangeSaveSet,
    SProcXFixesSelectSelectionInput,
    SProcXFixesSelectCursorInput,
    SProcXFixesGetCursorImage,
/*************** Version 2 ******************/
    SProcXFixesCreateRegion,
    SProcXFixesCreateRegionFromBitmap,
    SProcXFixesCreateRegionFromWindow,
    SProcXFixesCreateRegionFromGC,
    SProcXFixesCreateRegionFromPicture,
    SProcXFixesDestroyRegion,
    SProcXFixesSetRegion,
    SProcXFixesCopyRegion,
    SProcXFixesCombineRegion,
    SProcXFixesCombineRegion,
    SProcXFixesCombineRegion,
    SProcXFixesInvertRegion,
    SProcXFixesTranslateRegion,
    SProcXFixesRegionExtents,
    SProcXFixesFetchRegion,
    SProcXFixesSetGCClipRegion,
    SProcXFixesSetWindowShapeRegion,
    SProcXFixesSetPictureClipRegion,
    SProcXFixesSetCursorName,
    SProcXFixesGetCursorName,
    SProcXFixesGetCursorImageAndName,
    SProcXFixesChangeCursor,
    SProcXFixesChangeCursorByName,
/*************** Version 3 ******************/
    SProcXFixesExpandRegion,
};

static int
SProcXFixesDispatch (ClientPtr client)
{
    REQUEST(xXFixesReq);
    if (stuff->xfixesReqType >= XFixesNumberRequests)
	return BadRequest;
    return (*SProcXFixesVector[stuff->xfixesReqType]) (client);
}

static void
XFixesClientCallback (CallbackListPtr	*list,
		      pointer		closure,
		      pointer		data)
{
    NewClientInfoRec	*clientinfo = (NewClientInfoRec *) data;
    ClientPtr		pClient = clientinfo->client;
    XFixesClientPtr	pXFixesClient = GetXFixesClient (pClient);

    pXFixesClient->major_version = 0;
    pXFixesClient->minor_version = 0;
}

/*ARGSUSED*/
static void
XFixesResetProc (ExtensionEntry *extEntry)
{
    DeleteCallback (&ClientStateCallback, XFixesClientCallback, 0);
}

void
XFixesExtensionInit(void)
{
    ExtensionEntry *extEntry;

    XFixesClientPrivateIndex = AllocateClientPrivateIndex ();
    if (!AllocateClientPrivate (XFixesClientPrivateIndex, 
				sizeof (XFixesClientRec)))
	return;
    if (!AddCallback (&ClientStateCallback, XFixesClientCallback, 0))
	return;

    if (XFixesSelectionInit() && XFixesCursorInit () && XFixesRegionInit () &&
	(extEntry = AddExtension(XFIXES_NAME, XFixesNumberEvents, 
				 XFixesNumberErrors,
				 ProcXFixesDispatch, SProcXFixesDispatch,
				 XFixesResetProc, StandardMinorOpcode)) != 0)
    {
	XFixesReqCode = (unsigned char)extEntry->base;
	XFixesEventBase = extEntry->eventBase;
	XFixesErrorBase = extEntry->errorBase;
	EventSwapVector[XFixesEventBase + XFixesSelectionNotify] =
	    (EventSwapPtr) SXFixesSelectionNotifyEvent;
	EventSwapVector[XFixesEventBase + XFixesCursorNotify] =
	    (EventSwapPtr) SXFixesCursorNotifyEvent;
    }
}
