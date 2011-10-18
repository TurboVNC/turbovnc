/* $XFree86: xc/programs/Xserver/Xext/xf86dga.c,v 3.8 1997/01/18 06:53:01 dawes Exp $ */

/*

Copyright (c) 1995  Jon Tombs
Copyright (c) 1995, 1996  XFree86 Inc

*/

#define NEED_REPLIES
#define NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "colormapst.h"
#include "cursorstr.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "xf86dgastr.h"
#include "swaprep.h"
#include "../hw/xfree86/common/xf86.h"

#include <X11/Xtrans.h>
#include "../os/osdep.h"
#include <X11/Xauth.h>
#ifndef ESIX
#ifndef Lynx
#include <sys/socket.h>
#else
#include <socket.h>
#endif
#else
#include <lan/socket.h>
#endif

extern int xf86ScreenIndex;

static int DGAErrorBase;

static DISPATCH_PROC(ProcDGAQueryVersion);
static DISPATCH_PROC(ProcXF86DGADirectVideo);
static DISPATCH_PROC(ProcXF86DGADispatch);
static DISPATCH_PROC(ProcXF86DGAGetVidPage);
static DISPATCH_PROC(ProcXF86DGAGetVideoLL);
static DISPATCH_PROC(ProcXF86DGAGetViewPortSize);
static DISPATCH_PROC(ProcXF86DGASetVidPage);
static DISPATCH_PROC(ProcXF86DGASetViewPort);
static DISPATCH_PROC(ProcDGAInstallColormap);
static DISPATCH_PROC(ProcDGAQueryDirectVideo);
static DISPATCH_PROC(ProcDGAViewPortChanged);

/*
 * SProcs should probably be deleted, a local connection can never
 * be byte flipped!? - Jon.
 */
static DISPATCH_PROC(SProcXF86DGADirectVideo);
static DISPATCH_PROC(SProcXF86DGADispatch);
static DISPATCH_PROC(SProcXF86DGAQueryVersion);

static void XF86DGAResetProc(
#if NeedFunctionPrototypes
    ExtensionEntry* /* extEntry */
#endif
);

static unsigned char DGAReqCode = 0;

void
XFree86DGAExtensionInit()
{
    ExtensionEntry* extEntry;
#ifdef XF86DGA_EVENTS
    int		    i;
    ScreenPtr	    pScreen;

    EventType = CreateNewResourceType(XF86DGAFreeEvents);
    ScreenPrivateIndex = AllocateScreenPrivateIndex ();
    for (i = 0; i < screenInfo.numScreens; i++)
    {
	pScreen = screenInfo.screens[i];
	SetScreenPrivate (pScreen, NULL);
    }
#endif

    if (
#ifdef XF86DGA_EVENTS
        EventType && ScreenPrivateIndex != -1 &&
#endif
	(extEntry = AddExtension(XF86DGANAME,
				XF86DGANumberEvents,
				XF86DGANumberErrors,
				ProcXF86DGADispatch,
				SProcXF86DGADispatch,
				XF86DGAResetProc,
				StandardMinorOpcode))) {
	DGAReqCode = (unsigned char)extEntry->base;
	DGAErrorBase = extEntry->errorBase;
    }
}

/*ARGSUSED*/
static void
XF86DGAResetProc (extEntry)
    ExtensionEntry* extEntry;
{
}

static int
ProcDGAQueryVersion(client)
    register ClientPtr client;
{
    xXF86DGAQueryVersionReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xXF86DGAQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = XF86DGA_MAJOR_VERSION;
    rep.minorVersion = XF86DGA_MINOR_VERSION;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    }
    WriteToClient(client, sizeof(xXF86DGAQueryVersionReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXF86DGAGetVideoLL(client)
    register ClientPtr client;
{
    REQUEST(xXF86DGAGetVideoLLReq);
    xXF86DGAGetVideoLLReply rep;
    ScrnInfoPtr vptr;
    register int n;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    vptr = (ScrnInfoPtr) screenInfo.screens[stuff->screen]->devPrivates[xf86ScreenIndex].ptr;
    REQUEST_SIZE_MATCH(xXF86DGAGetVideoLLReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
#if 0
    xf86GetVidMemData(stuff->screen, &rep.offset, &rep.bank_size);
#else
    rep.offset = vptr->physBase;
    rep.bank_size = vptr->physSize;
#endif
    rep.width = vptr->displayWidth;
    rep.ram_size = vptr->videoRam;

    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swapl(&rep.offset, n);
    	swapl(&rep.width, n);
    	swapl(&rep.bank_size, n);
    	swapl(&rep.ram_size, n);
    }
    WriteToClient(client, SIZEOF(xXF86DGAGetVideoLLReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXF86DGADirectVideo(client)
    register ClientPtr client;
{
    REQUEST(xXF86DGADirectVideoReq);
    ScrnInfoPtr vptr;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    vptr = (ScrnInfoPtr) screenInfo.screens[stuff->screen]->devPrivates[xf86ScreenIndex].ptr;

    REQUEST_SIZE_MATCH(xXF86DGADirectVideoReq);
    if (!(vptr->directMode&XF86DGADirectPresent)) {
       /* chipset doesn't know about directVideoMode */
	return DGAErrorBase + XF86DGANoDirectVideoMode;
    }
    
    /* Check that the current screen is active. */
    if (!xf86VTSema && !(vptr->directMode & XF86DGADirectGraphics)) {
	return DGAErrorBase + XF86DGAScreenNotActive;
    }

    if (stuff->enable&XF86DGADirectGraphics) {
       vptr->directMode = stuff->enable|XF86DGADirectPresent;
       if (xf86VTSema == TRUE) {
	  vptr->EnterLeaveVT(LEAVE, stuff->screen);
	  xf86VTSema = FALSE;
       }
    } else {
       if (xf86VTSema == FALSE) {
          xf86VTSema = TRUE;
          vptr->EnterLeaveVT(ENTER, stuff->screen);
       }
       vptr->directMode = (0x0f&stuff->enable)|XF86DGADirectPresent;
    }

    return (client->noClientException);
}

static int
ProcXF86DGAGetViewPortSize(client)
    register ClientPtr client;
{
    REQUEST(xXF86DGAGetViewPortSizeReq);
    xXF86DGAGetViewPortSizeReply rep;
    register int n;
    ScrnInfoPtr vptr;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    vptr = (ScrnInfoPtr) screenInfo.screens[stuff->screen]->devPrivates[xf86ScreenIndex].ptr;

    REQUEST_SIZE_MATCH(xXF86DGAGetViewPortSizeReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.width = vptr->modes->HDisplay;
    rep.height = vptr->modes->VDisplay;

    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swapl(&rep.width, n);
    	swapl(&rep.height, n);
    }
    WriteToClient(client, SIZEOF(xXF86DGAGetViewPortSizeReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXF86DGASetViewPort(client)
    register ClientPtr client;
{
    REQUEST(xXF86DGASetViewPortReq);
    ScrnInfoPtr vptr;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    vptr = (ScrnInfoPtr) screenInfo.screens[stuff->screen]->devPrivates[xf86ScreenIndex].ptr;

    REQUEST_SIZE_MATCH(xXF86DGASetViewPortReq);

    if (vptr->AdjustFrame &&
	(xf86VTSema == TRUE || vptr->directMode&XF86DGADirectGraphics))
	vptr->AdjustFrame(stuff->x, stuff->y);
    else
	return DGAErrorBase + XF86DGAScreenNotActive;

    return (client->noClientException);
}

static int
ProcXF86DGAGetVidPage(client)
    register ClientPtr client;
{
    REQUEST(xXF86DGAGetVidPageReq);
    ScrnInfoPtr vptr;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    vptr = (ScrnInfoPtr) screenInfo.screens[stuff->screen]->devPrivates[xf86ScreenIndex].ptr;
    ErrorF("XF86DGAGetVidPage not yet implemented\n");

    REQUEST_SIZE_MATCH(xXF86DGAGetVidPageReq);
    return (client->noClientException);
}


static int
ProcXF86DGASetVidPage(client)
    register ClientPtr client;
{
    REQUEST(xXF86DGASetVidPageReq);
    ScrnInfoPtr vptr;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    vptr = (ScrnInfoPtr) screenInfo.screens[stuff->screen]->devPrivates[xf86ScreenIndex].ptr;

    REQUEST_SIZE_MATCH(xXF86DGASetVidPageReq);

    if (xf86VTSema == TRUE) {/* only valid when switched away! */
	return DGAErrorBase + XF86DGADirectNotActivated;
    }
    if (!xf86VTSema && !(vptr->directMode & XF86DGADirectGraphics)) {
	return DGAErrorBase + XF86DGAScreenNotActive;
    }

    if (vptr->setBank) {
	vptr->setBank(stuff->vpage);
    }
    return (client->noClientException);
}


static int
ProcDGAInstallColormap(client)
    register ClientPtr client;
{
    ColormapPtr pcmp;
    ScrnInfoPtr vptr;
    REQUEST(xXF86DGAInstallColormapReq);

    REQUEST_SIZE_MATCH(xXF86DGAInstallColormapReq);

    vptr = (ScrnInfoPtr) screenInfo.screens[stuff->screen]->devPrivates[xf86ScreenIndex].ptr;

    if (xf86VTSema == TRUE) {/* only valid when switched away! */
	return DGAErrorBase + XF86DGADirectNotActivated;
    }
    if (!xf86VTSema && !(vptr->directMode & XF86DGADirectGraphics)) {
	return DGAErrorBase + XF86DGAScreenNotActive;
    }

    pcmp = (ColormapPtr  )LookupIDByType(stuff->id, RT_COLORMAP);
    if (pcmp)
    {
	vptr->directMode |= XF86DGADirectColormap;
        vptr->directMode |= XF86DGAHasColormap;
        (*(pcmp->pScreen->InstallColormap)) (pcmp);
        vptr->directMode &= ~XF86DGAHasColormap;
        return (client->noClientException);
    }
    else
    {
        client->errorValue = stuff->id;
        return (BadColor);
    }
}

static int
ProcXF86DGAQueryDirectVideo(client)
    register ClientPtr client;
{
    REQUEST(xXF86DGAQueryDirectVideoReq);
    xXF86DGAQueryDirectVideoReply rep;
    register int n;
    ScrnInfoPtr vptr;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    vptr = (ScrnInfoPtr) screenInfo.screens[stuff->screen]->devPrivates[xf86ScreenIndex].ptr;

    REQUEST_SIZE_MATCH(xXF86DGAQueryDirectVideoReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.flags = vptr->directMode;

    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swapl(&rep.flags, n);
    }
    WriteToClient(client, SIZEOF(xXF86DGAQueryDirectVideoReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXF86DGAViewPortChanged(client)
    register ClientPtr client;
{
    REQUEST(xXF86DGAViewPortChangedReq);
    xXF86DGAViewPortChangedReply rep;
    register int n;
    ScrnInfoPtr vptr;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    vptr = (ScrnInfoPtr) screenInfo.screens[stuff->screen]->devPrivates[xf86ScreenIndex].ptr;

    REQUEST_SIZE_MATCH(xXF86DGAViewPortChangedReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    /* For the moment, always return TRUE. */
    rep.result = TRUE;

    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swapl(&rep.result, n);
    }
    WriteToClient(client, SIZEOF(xXF86DGAViewPortChangedReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXF86DGADispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);

    if (!LocalClient(client))
	return DGAErrorBase + XF86DGAClientNotLocal;

    switch (stuff->data)
    {
    case X_XF86DGAQueryVersion:
	return ProcDGAQueryVersion(client);
    case X_XF86DGAGetVideoLL:
	return ProcXF86DGAGetVideoLL(client);
    case X_XF86DGADirectVideo:
	return ProcXF86DGADirectVideo(client);
    case X_XF86DGAGetViewPortSize:
	return ProcXF86DGAGetViewPortSize(client);
    case X_XF86DGASetViewPort:
	return ProcXF86DGASetViewPort(client);
    case X_XF86DGAGetVidPage:
	return ProcXF86DGAGetVidPage(client);
    case X_XF86DGASetVidPage:
	return ProcXF86DGASetVidPage(client);
    case X_XF86DGAInstallColormap:
	return ProcDGAInstallColormap(client);
    case X_XF86DGAQueryDirectVideo:
	return ProcXF86DGAQueryDirectVideo(client);
    case X_XF86DGAViewPortChanged:
	return ProcXF86DGAViewPortChanged(client);
    default:
	return BadRequest;
    }
}

static int
SProcXF86DGAQueryVersion(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xXF86DGAQueryVersionReq);
    swaps(&stuff->length, n);
    return ProcDGAQueryVersion(client);
}

static int
SProcXF86DGADirectVideo(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xXF86DGADirectVideoReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86DGADirectVideoReq);
    swaps(&stuff->screen, n);
    swaps(&stuff->enable, n);
    return ProcXF86DGADirectVideo(client);
}

static int
SProcXF86DGADispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);

    /* It is bound to be non-local when there is byte swapping */
    if (!LocalClient(client))
	return DGAErrorBase + XF86DGAClientNotLocal;

    switch (stuff->data)
    {
    case X_XF86DGAQueryVersion:
	return SProcXF86DGAQueryVersion(client);
    case X_XF86DGADirectVideo:
	return SProcXF86DGADirectVideo(client);
    default:
	return BadRequest;
    }
}

