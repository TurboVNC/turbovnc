/* $XFree86: xc/programs/Xserver/Xext/xf86misc.c,v 3.21.2.4 1998/02/25 14:26:43 dawes Exp $ */

/*
 * Copyright (c) 1995, 1996  The XFree86 Project, Inc
 */

/* THIS IS NOT AN X CONSORTIUM STANDARD */

#define NEED_REPLIES
#define NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "scrnintstr.h"
#include "inputstr.h"
#include "servermd.h"
#define _XF86MISC_SERVER_
#define _XF86MISC_SAVER_COMPAT_
#include "xf86mscstr.h"
#include "Xfuncproto.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

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

#include "swaprep.h"

extern int xf86ScreenIndex;
extern Bool xf86MiscModInDevEnabled;
extern Bool xf86MiscModInDevAllowNonLocal;

static int miscErrorBase;

static void XF86MiscResetProc(
#if NeedFunctionPrototypes
    ExtensionEntry* /* extEntry */
#endif
);

static DISPATCH_PROC(ProcXF86MiscDispatch);
static DISPATCH_PROC(ProcXF86MiscGetKbdSettings);
static DISPATCH_PROC(ProcXF86MiscGetMouseSettings);
static DISPATCH_PROC(ProcXF86MiscGetSaver);
static DISPATCH_PROC(ProcXF86MiscQueryVersion);
static DISPATCH_PROC(ProcXF86MiscSetKbdSettings);
static DISPATCH_PROC(ProcXF86MiscSetMouseSettings);
static DISPATCH_PROC(ProcXF86MiscSetSaver);
static DISPATCH_PROC(SProcXF86MiscDispatch);
static DISPATCH_PROC(SProcXF86MiscGetKbdSettings);
static DISPATCH_PROC(SProcXF86MiscGetMouseSettings);
static DISPATCH_PROC(SProcXF86MiscGetSaver);
static DISPATCH_PROC(SProcXF86MiscQueryVersion);
static DISPATCH_PROC(SProcXF86MiscSetKbdSettings);
static DISPATCH_PROC(SProcXF86MiscSetMouseSettings);
static DISPATCH_PROC(SProcXF86MiscSetSaver);

static unsigned char XF86MiscReqCode = 0;

extern InputInfo inputInfo;

void
XFree86MiscExtensionInit()
{
    ExtensionEntry* extEntry;

    if (
	(extEntry = AddExtension(XF86MISCNAME,
				XF86MiscNumberEvents,
				XF86MiscNumberErrors,
				ProcXF86MiscDispatch,
				SProcXF86MiscDispatch,
				XF86MiscResetProc,
				StandardMinorOpcode))) {
	XF86MiscReqCode = (unsigned char)extEntry->base;
	miscErrorBase = extEntry->errorBase;
    }
}

/*ARGSUSED*/
static void
XF86MiscResetProc (extEntry)
    ExtensionEntry* extEntry;
{
}

static int
ProcXF86MiscQueryVersion(client)
    register ClientPtr client;
{
    xXF86MiscQueryVersionReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xXF86MiscQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = XF86MISC_MAJOR_VERSION;
    rep.minorVersion = XF86MISC_MINOR_VERSION;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swaps(&rep.majorVersion, n);
    	swaps(&rep.minorVersion, n);
    }
    WriteToClient(client, sizeof(xXF86MiscQueryVersionReply), (char *)&rep);
    return (client->noClientException);
}

/*
 * This will go away, but remains for now for compatibility with older
 * clients.
 */
static int
ProcXF86MiscSetSaver(client)
    register ClientPtr client;
{
    REQUEST(xXF86MiscSetSaverReq);
    ScrnInfoPtr vptr;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    vptr = (ScrnInfoPtr) screenInfo.screens[stuff->screen]->devPrivates[xf86ScreenIndex].ptr;

    REQUEST_SIZE_MATCH(xXF86MiscSetSaverReq);

    if (stuff->suspendTime < 0)
	return BadValue;
    if (stuff->offTime < 0)
	return BadValue;

    return (client->noClientException);
}

/*
 * This will go away, but remains for now for compatibility with older
 * clients.
 */
static int
ProcXF86MiscGetSaver(client)
    register ClientPtr client;
{
    REQUEST(xXF86MiscGetSaverReq);
    xXF86MiscGetSaverReply rep;
    register int n;
    ScrnInfoPtr vptr;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    vptr = (ScrnInfoPtr) screenInfo.screens[stuff->screen]->devPrivates[xf86ScreenIndex].ptr;

    REQUEST_SIZE_MATCH(xXF86MiscGetSaverReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.suspendTime = 0;
    rep.offTime = 0;
    
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swapl(&rep.suspendTime, n);
    	swapl(&rep.offTime, n);
    }
    WriteToClient(client, SIZEOF(xXF86MiscGetSaverReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXF86MiscGetMouseSettings(client)
    register ClientPtr client;
{
    xXF86MiscGetMouseSettingsReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xXF86MiscGetMouseSettingsReq);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.mousetype = xf86Info.mouseDev->mseType;
#ifdef XQUEUE
    if (xf86Info.mouseDev->mseProc == xf86XqueMseProc)
        rep.mousetype = MTYPE_XQUEUE;
#endif
#if defined(USE_OSMOUSE) || defined(OSMOUSE_ONLY)
    if (xf86Info.mouseDev->mseProc == xf86OsMouseProc)
        rep.mousetype = MTYPE_OSMOUSE;
#endif
    rep.baudrate = xf86Info.mouseDev->baudRate;
    rep.samplerate = xf86Info.mouseDev->sampleRate;
    rep.resolution = xf86Info.mouseDev->resolution;
    rep.buttons = xf86Info.mouseDev->buttons;
    rep.emulate3buttons = xf86Info.mouseDev->emulate3Buttons;
    rep.emulate3timeout = xf86Info.mouseDev->emulate3Timeout;
    rep.chordmiddle = xf86Info.mouseDev->chordMiddle;
    rep.flags = xf86Info.mouseDev->mouseFlags;
    if (xf86Info.mouseDev->mseDevice)
        rep.devnamelen = strlen(xf86Info.mouseDev->mseDevice);
    else
        rep.devnamelen = 0;
    rep.length = (sizeof(xXF86MiscGetMouseSettingsReply) -
		  sizeof(xGenericReply) + ((rep.devnamelen+3) & ~3)) >> 2;
    
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swapl(&rep.mousetype, n);
    	swapl(&rep.baudrate, n);
    	swapl(&rep.samplerate, n);
    	swapl(&rep.resolution, n);
    	swapl(&rep.buttons, n);
    	swapl(&rep.emulate3buttons, n);
    	swapl(&rep.emulate3timeout, n);
    	swapl(&rep.chordmiddle, n);
    	swapl(&rep.flags, n);
    }
    WriteToClient(client, SIZEOF(xXF86MiscGetMouseSettingsReply), (char *)&rep);
    if (rep.devnamelen)
        WriteToClient(client, rep.devnamelen, xf86Info.mouseDev->mseDevice);
    return (client->noClientException);
}

static int
ProcXF86MiscGetKbdSettings(client)
    register ClientPtr client;
{
    xXF86MiscGetKbdSettingsReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xXF86MiscGetKbdSettingsReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.kbdtype = xf86Info.kbdType;
#ifdef XQUEUE
    if (xf86Info.kbdProc == xf86XqueKbdProc)
        rep.kbdtype = KTYPE_XQUEUE;
#endif
    rep.rate = xf86Info.kbdRate;
    rep.delay = xf86Info.kbdDelay;
    rep.servnumlock = xf86Info.serverNumLock;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swapl(&rep.kbdtype, n);
    	swapl(&rep.rate, n);
    	swapl(&rep.delay, n);
    }
    WriteToClient(client, SIZEOF(xXF86MiscGetKbdSettingsReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXF86MiscSetMouseSettings(client)
    register ClientPtr client;
{
    int reopen, msetype, flags, baudrate, samplerate, resolution;

    REQUEST(xXF86MiscSetMouseSettingsReq);

    REQUEST_SIZE_MATCH(xXF86MiscSetMouseSettingsReq);

    if (xf86Verbose) {
	ErrorF("SetMouseSettings - type: %d brate: %d srate: %d chdmid: %d\n",
		stuff->mousetype, stuff->baudrate,
		stuff->samplerate, stuff->chordmiddle);
	ErrorF("                   em3but: %d em3tim: %d res: %d flags: %d\n",
		stuff->emulate3buttons, stuff->emulate3timeout,
		stuff->resolution, stuff->flags);
    }
    if (stuff->mousetype > MTYPE_OSMOUSE
            || stuff->mousetype < MTYPE_MICROSOFT)
	return miscErrorBase + XF86MiscBadMouseProtocol;
#ifdef OSMOUSE_ONLY
    if (stuff->mousetype != MTYPE_OSMOUSE)
	return miscErrorBase + XF86MiscBadMouseProtocol;
#else
#ifndef XQUEUE
    if (stuff->mousetype == MTYPE_XQUEUE)
	return miscErrorBase + XF86MiscBadMouseProtocol;
#endif
#ifndef USE_OSMOUSE
    if (stuff->mousetype == MTYPE_OSMOUSE)
	return miscErrorBase + XF86MiscBadMouseProtocol;
#endif
#endif /* OSMOUSE_ONLY */

    if (stuff->emulate3timeout < 0)
	return BadValue;

    if (stuff->mousetype == MTYPE_LOGIMAN
            && !(stuff->baudrate == 1200 || stuff->baudrate == 9600) )
	return miscErrorBase + XF86MiscBadMouseBaudRate;
    if (stuff->mousetype == MTYPE_LOGIMAN && stuff->samplerate)
	return miscErrorBase + XF86MiscBadMouseCombo;

    samplerate = xf86Info.mouseDev->sampleRate;
    resolution = xf86Info.mouseDev->resolution;
    baudrate   = xf86Info.mouseDev->baudRate;
    flags      = xf86Info.mouseDev->mouseFlags;
    msetype    = xf86Info.mouseDev->mseType;
#ifdef XQUEUE
    if (xf86Info.mouseDev->mseProc == xf86XqueMseProc)
        msetype = MTYPE_XQUEUE;
#endif
#if defined(USE_OSMOUSE) || defined(OSMOUSE_ONLY)
    if (xf86Info.mouseDev->mseProc == xf86OsMouseProc)
        msetype = MTYPE_OSMOUSE;
#endif

    reopen     = 0;

    if (stuff->mousetype != msetype)
	if (stuff->mousetype == MTYPE_XQUEUE
		|| stuff->mousetype == MTYPE_OSMOUSE
		|| msetype == MTYPE_XQUEUE
		|| msetype == MTYPE_OSMOUSE)
	    return miscErrorBase + XF86MiscBadMouseProtocol;
	else {
	    reopen++;
	    msetype = stuff->mousetype;
	}

    if (stuff->flags & MF_REOPEN) {
	reopen++;
	stuff->flags &= ~MF_REOPEN;
    }
    if (stuff->mousetype != MTYPE_OSMOUSE
	    && stuff->mousetype != MTYPE_XQUEUE
	    && stuff->mousetype != MTYPE_PS_2
	    && stuff->mousetype != MTYPE_BUSMOUSE
	    && stuff->mousetype != MTYPE_IMPS2
	    && stuff->mousetype != MTYPE_THINKINGPS2
	    && stuff->mousetype != MTYPE_MMANPLUSPS2
	    && stuff->mousetype != MTYPE_GLIDEPOINTPS2
	    && stuff->mousetype != MTYPE_NETPS2
	    && stuff->mousetype != MTYPE_NETSCROLLPS2
	    && stuff->mousetype != MTYPE_SYSMOUSE)
    {
        if (stuff->baudrate < 1200)
	    return miscErrorBase + XF86MiscBadMouseBaudRate;
        if (stuff->baudrate % 1200 != 0
                || stuff->baudrate < 1200 || stuff->baudrate > 9600)
	    return miscErrorBase + XF86MiscBadMouseBaudRate;
	if (xf86Info.mouseDev->baudRate != stuff->baudrate) {
		reopen++;
		baudrate = stuff->baudrate;
	}
    }
    if (stuff->flags & (MF_CLEAR_DTR|MF_CLEAR_RTS))
	if (stuff->mousetype != MTYPE_MOUSESYS)
	    return miscErrorBase + XF86MiscBadMouseFlags;
	else if (xf86Info.mouseDev->mouseFlags != stuff->flags) {
	    reopen++;
            flags = stuff->flags;
	}

    if (stuff->mousetype != MTYPE_OSMOUSE
	    && stuff->mousetype != MTYPE_XQUEUE
	    && stuff->mousetype != MTYPE_BUSMOUSE)
    {
        if (stuff->samplerate < 0)
	    return BadValue;
	
	if (xf86Info.mouseDev->sampleRate != stuff->samplerate) {
		reopen++;
		samplerate = stuff->samplerate;
	}
    }

    if (stuff->resolution < 0)
	return BadValue;
    if (xf86Info.mouseDev->resolution != stuff->resolution) {
	reopen++;
	resolution = stuff->resolution;
    }

#if 0
    /* Ignore the buttons field */
    if (xf86Info.mouseDev->buttons != stuff->buttons)
	/* we cannot change this field on the fly... */
	return BadValue;
#endif

    if (stuff->chordmiddle)
        if (stuff->emulate3buttons
		|| !(stuff->mousetype == MTYPE_MICROSOFT
		     || stuff->mousetype == MTYPE_LOGIMAN) )
	    return miscErrorBase + XF86MiscBadMouseCombo;

    xf86Info.mouseDev->chordMiddle = stuff->chordmiddle!=0;
    xf86Info.mouseDev->emulate3Buttons = stuff->emulate3buttons!=0;
    xf86Info.mouseDev->emulate3Timeout = stuff->emulate3timeout;

    if (reopen && msetype != MTYPE_OSMOUSE && msetype != MTYPE_XQUEUE) {

        (xf86Info.mouseDev->mseProc)(xf86Info.pMouse, DEVICE_CLOSE);

        xf86Info.mouseDev->mseType    = msetype;
        xf86Info.mouseDev->mouseFlags = flags;
        xf86Info.mouseDev->baudRate   = baudrate;
        xf86Info.mouseDev->sampleRate = samplerate;
	xf86Info.mouseDev->resolution = resolution;

	xf86Info.pMouse->public.on = FALSE;
	xf86AllowMouseOpenFail = TRUE;
	xf86MouseInit(xf86Info.mouseDev);
        (xf86Info.mouseDev->mseProc)(xf86Info.pMouse, DEVICE_ON);
    }

    if (xf86Verbose)
	ErrorF("SetMouseSettings - Succeeded\n");
    return (client->noClientException);
}

static int
ProcXF86MiscSetKbdSettings(client)
    register ClientPtr client;
{
    REQUEST(xXF86MiscSetKbdSettingsReq);

    REQUEST_SIZE_MATCH(xXF86MiscSetKbdSettingsReq);

    if (xf86Verbose)
	ErrorF("SetKbdSettings - type: %d rate: %d delay: %d snumlk: %d\n",
		stuff->kbdtype, stuff->rate,
		stuff->delay, stuff->servnumlock);
    if (stuff->rate < 0)
	return BadValue;
    if (stuff->delay < 0)
	return BadValue;
    if (stuff->kbdtype < KTYPE_UNKNOWN || stuff->kbdtype > KTYPE_XQUEUE)
	return miscErrorBase + XF86MiscBadKbdType;

    if (xf86Info.kbdRate!=stuff->rate || xf86Info.kbdDelay!=stuff->delay) {
	char rad;

	xf86Info.kbdRate = stuff->rate;
	xf86Info.kbdDelay = stuff->delay;
        if      (xf86Info.kbdDelay <= 375) rad = 0x00;
        else if (xf86Info.kbdDelay <= 625) rad = 0x20;
        else if (xf86Info.kbdDelay <= 875) rad = 0x40;
        else                               rad = 0x60;
    
        if      (xf86Info.kbdRate <=  2)   rad |= 0x1F;
        else if (xf86Info.kbdRate >= 30)   rad |= 0x00;
        else                               rad |= ((58/xf86Info.kbdRate)-2);
    
        xf86SetKbdRepeat(rad);
    }
#if 0	/* Not done yet */
    xf86Info.kbdType = stuff->kbdtype;
    xf86Info.serverNumLock = stuff->servnumlock!=0;
#endif

    if (xf86Verbose)
	ErrorF("SetKbdSettings - Succeeded\n");
    return (client->noClientException);
}

static int
ProcXF86MiscDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_XF86MiscQueryVersion:
	return ProcXF86MiscQueryVersion(client);
    case X_XF86MiscGetSaver:
	return ProcXF86MiscGetSaver(client);
    case X_XF86MiscSetSaver:
	return ProcXF86MiscSetSaver(client);
    case X_XF86MiscGetMouseSettings:
	return ProcXF86MiscGetMouseSettings(client);
    case X_XF86MiscGetKbdSettings:
	return ProcXF86MiscGetKbdSettings(client);
    default:
	if (!xf86MiscModInDevEnabled)
	    return miscErrorBase + XF86MiscModInDevDisabled;
	if (xf86MiscModInDevAllowNonLocal || LocalClient (client)) {
	    switch (stuff->data) {
	        case X_XF86MiscSetMouseSettings:
		    return ProcXF86MiscSetMouseSettings(client);
	        case X_XF86MiscSetKbdSettings:
		    return ProcXF86MiscSetKbdSettings(client);
	        default:
		    return BadRequest;
	    }
	} else
	    return miscErrorBase + XF86MiscModInDevClientNotLocal;
    }
}

static int
SProcXF86MiscQueryVersion(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xXF86MiscQueryVersionReq);
    swaps(&stuff->length, n);
    return ProcXF86MiscQueryVersion(client);
}

static int
SProcXF86MiscGetSaver(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xXF86MiscGetSaverReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86MiscGetSaverReq);
    swaps(&stuff->screen, n);
    return ProcXF86MiscGetSaver(client);
}

static int
SProcXF86MiscSetSaver(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xXF86MiscSetSaverReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86MiscSetSaverReq);
    swaps(&stuff->screen, n);
    swapl(&stuff->suspendTime, n);
    swapl(&stuff->offTime, n);
    return ProcXF86MiscSetSaver(client);
}

static int
SProcXF86MiscGetMouseSettings(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xXF86MiscGetMouseSettingsReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86MiscGetMouseSettingsReq);
    return ProcXF86MiscGetMouseSettings(client);
}

static int
SProcXF86MiscGetKbdSettings(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xXF86MiscGetKbdSettingsReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86MiscGetKbdSettingsReq);
    return ProcXF86MiscGetKbdSettings(client);
}

static int
SProcXF86MiscSetMouseSettings(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xXF86MiscSetMouseSettingsReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86MiscSetMouseSettingsReq);
    swapl(&stuff->mousetype, n);
    swapl(&stuff->baudrate, n);
    swapl(&stuff->samplerate, n);
    swapl(&stuff->resolution, n);
    swapl(&stuff->buttons, n);
    swapl(&stuff->emulate3timeout, n);
    swapl(&stuff->flags, n);
    return ProcXF86MiscSetMouseSettings(client);
}

static int
SProcXF86MiscSetKbdSettings(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xXF86MiscSetKbdSettingsReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86MiscSetKbdSettingsReq);
    swapl(&stuff->kbdtype, n);
    swapl(&stuff->rate, n);
    swapl(&stuff->delay, n);
    return ProcXF86MiscSetKbdSettings(client);
}

static int
SProcXF86MiscDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_XF86MiscQueryVersion:
	return SProcXF86MiscQueryVersion(client);
    case X_XF86MiscGetSaver:
	return SProcXF86MiscGetSaver(client);
    case X_XF86MiscSetSaver:
	return SProcXF86MiscSetSaver(client);
    case X_XF86MiscGetMouseSettings:
	return SProcXF86MiscGetMouseSettings(client);
    case X_XF86MiscGetKbdSettings:
	return SProcXF86MiscGetKbdSettings(client);
    default:
	if (!xf86MiscModInDevEnabled)
	    return miscErrorBase + XF86MiscModInDevDisabled;
	if (xf86MiscModInDevAllowNonLocal || LocalClient (client)) {
	    switch (stuff->data) {
	        case X_XF86MiscSetMouseSettings:
		    return SProcXF86MiscSetMouseSettings(client);
	        case X_XF86MiscSetKbdSettings:
		    return SProcXF86MiscSetKbdSettings(client);
	        default:
		    return BadRequest;
	    }
	} else
	    return miscErrorBase + XF86MiscModInDevClientNotLocal;
    }
}

