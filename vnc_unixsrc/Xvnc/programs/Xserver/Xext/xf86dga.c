/* $XFree86: xc/programs/Xserver/Xext/xf86dga.c,v 3.21 2000/06/30 19:06:54 keithp Exp $ */

/*

Copyright (c) 1995  Jon Tombs
Copyright (c) 1995, 1996, 1999  XFree86 Inc

*/

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#define NEED_REPLIES
#define NEED_EVENTS
#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "colormapst.h"
#include "cursorstr.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include <X11/extensions/xf86dga.h>
#include <X11/extensions/xf86dgastr.h>
#include "swaprep.h"
#include "dgaproc.h"

#include "xf86dgaext.h"


static DISPATCH_PROC(ProcXF86DGADirectVideo);
static DISPATCH_PROC(ProcXF86DGAGetVidPage);
static DISPATCH_PROC(ProcXF86DGAGetVideoLL);
static DISPATCH_PROC(ProcXF86DGAGetViewPortSize);
static DISPATCH_PROC(ProcXF86DGASetVidPage);
static DISPATCH_PROC(ProcXF86DGASetViewPort);
static DISPATCH_PROC(ProcXF86DGAInstallColormap);
static DISPATCH_PROC(ProcXF86DGAQueryDirectVideo);
static DISPATCH_PROC(ProcXF86DGAViewPortChanged);


static int
ProcXF86DGAGetVideoLL(ClientPtr client)
{
    REQUEST(xXF86DGAGetVideoLLReq);
    xXF86DGAGetVideoLLReply rep;
    XDGAModeRec mode;
    int num, offset, flags;
    char *name;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    REQUEST_SIZE_MATCH(xXF86DGAGetVideoLLReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    if(!DGAAvailable(stuff->screen))
	return (DGAErrorBase + XF86DGANoDirectVideoMode);

    if(!(num = DGAGetOldDGAMode(stuff->screen)))
	return (DGAErrorBase + XF86DGANoDirectVideoMode);

    /* get the parameters for the mode that best matches */
    DGAGetModeInfo(stuff->screen, &mode, num);

    if(!DGAOpenFramebuffer(stuff->screen, &name, 
			(unsigned char**)(&rep.offset), 	
			(int*)(&rep.bank_size), &offset, &flags))
	return BadAlloc;

    rep.offset += mode.offset;
    rep.width = mode.bytesPerScanline / (mode.bitsPerPixel >> 3);
    rep.ram_size = rep.bank_size >> 10;

    WriteToClient(client, SIZEOF(xXF86DGAGetVideoLLReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXF86DGADirectVideo(ClientPtr client)
{
    int num;
    PixmapPtr pix;
    XDGAModeRec mode;
    REQUEST(xXF86DGADirectVideoReq);

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    REQUEST_SIZE_MATCH(xXF86DGADirectVideoReq);

    if (!DGAAvailable(stuff->screen)) 
	return DGAErrorBase + XF86DGANoDirectVideoMode;

    if (stuff->enable & XF86DGADirectGraphics) {
	if(!(num = DGAGetOldDGAMode(stuff->screen)))
	    return (DGAErrorBase + XF86DGANoDirectVideoMode);
    } else
	num = 0;

    if(Success != DGASetMode(stuff->screen, num, &mode, &pix))
	return (DGAErrorBase + XF86DGAScreenNotActive);

    DGASetInputMode (stuff->screen, 
		     (stuff->enable & XF86DGADirectKeyb) != 0,
		     (stuff->enable & XF86DGADirectMouse) != 0);

    return (client->noClientException);
}

static int
ProcXF86DGAGetViewPortSize(ClientPtr client)
{
    int num;
    XDGAModeRec mode;
    REQUEST(xXF86DGAGetViewPortSizeReq);
    xXF86DGAGetViewPortSizeReply rep;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    REQUEST_SIZE_MATCH(xXF86DGAGetViewPortSizeReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    if (!DGAAvailable(stuff->screen)) 
	return (DGAErrorBase + XF86DGANoDirectVideoMode);

    if(!(num = DGAGetOldDGAMode(stuff->screen)))
	return (DGAErrorBase + XF86DGANoDirectVideoMode);

    DGAGetModeInfo(stuff->screen, &mode, num);

    rep.width = mode.viewportWidth;
    rep.height = mode.viewportHeight;

    WriteToClient(client, SIZEOF(xXF86DGAGetViewPortSizeReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXF86DGASetViewPort(ClientPtr client)
{
    REQUEST(xXF86DGASetViewPortReq);

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    REQUEST_SIZE_MATCH(xXF86DGASetViewPortReq);

    if (!DGAActive(stuff->screen))
    {
	int num;
	PixmapPtr pix;
	XDGAModeRec mode;
	
	if(!(num = DGAGetOldDGAMode(stuff->screen)))
	    return (DGAErrorBase + XF86DGANoDirectVideoMode);
	if(Success != DGASetMode(stuff->screen, num, &mode, &pix))
	    return (DGAErrorBase + XF86DGAScreenNotActive);
    }

    if (DGASetViewport(stuff->screen, stuff->x, stuff->y, DGA_FLIP_RETRACE)
		!= Success)
	return DGAErrorBase + XF86DGADirectNotActivated;

    return (client->noClientException);
}

static int
ProcXF86DGAGetVidPage(ClientPtr client)
{
    REQUEST(xXF86DGAGetVidPageReq);
    xXF86DGAGetVidPageReply rep;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    REQUEST_SIZE_MATCH(xXF86DGAGetVidPageReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.vpage = 0;  /* silently fail */

    WriteToClient(client, SIZEOF(xXF86DGAGetVidPageReply), (char *)&rep);
    return (client->noClientException);
}


static int
ProcXF86DGASetVidPage(ClientPtr client)
{
    REQUEST(xXF86DGASetVidPageReq);

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    REQUEST_SIZE_MATCH(xXF86DGASetVidPageReq);

    /* silently fail */

    return (client->noClientException);
}


static int
ProcXF86DGAInstallColormap(ClientPtr client)
{
    ColormapPtr pcmp;
    REQUEST(xXF86DGAInstallColormapReq);

    REQUEST_SIZE_MATCH(xXF86DGAInstallColormapReq);

    if (!DGAActive(stuff->screen))
	return (DGAErrorBase + XF86DGADirectNotActivated);

    pcmp = (ColormapPtr  )LookupIDByType(stuff->id, RT_COLORMAP);
    if (pcmp) {
	DGAInstallCmap(pcmp);
        return (client->noClientException);
    } else {
        client->errorValue = stuff->id;
        return (BadColor);
    }
}

static int
ProcXF86DGAQueryDirectVideo(ClientPtr client)
{
    REQUEST(xXF86DGAQueryDirectVideoReq);
    xXF86DGAQueryDirectVideoReply rep;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    REQUEST_SIZE_MATCH(xXF86DGAQueryDirectVideoReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.flags = 0;

    if (DGAAvailable(stuff->screen))
	rep.flags = XF86DGADirectPresent;

    WriteToClient(client, SIZEOF(xXF86DGAQueryDirectVideoReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXF86DGAViewPortChanged(ClientPtr client)
{
    REQUEST(xXF86DGAViewPortChangedReq);
    xXF86DGAViewPortChangedReply rep;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    REQUEST_SIZE_MATCH(xXF86DGAViewPortChangedReq);

    if (!DGAActive(stuff->screen))
	return (DGAErrorBase + XF86DGADirectNotActivated);

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.result = 1;

    WriteToClient(client, SIZEOF(xXF86DGAViewPortChangedReply), (char *)&rep);
    return (client->noClientException);
}

int
ProcXF86DGADispatch(register ClientPtr client)
{
    REQUEST(xReq);

    switch (stuff->data)
    {
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
	return ProcXF86DGAInstallColormap(client);
    case X_XF86DGAQueryDirectVideo:
	return ProcXF86DGAQueryDirectVideo(client);
    case X_XF86DGAViewPortChanged:
	return ProcXF86DGAViewPortChanged(client);
    default:
	return BadRequest;
    }
}

