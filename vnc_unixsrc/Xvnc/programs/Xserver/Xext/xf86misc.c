/* $XFree86: xc/programs/Xserver/Xext/xf86misc.c,v 3.41 2003/11/17 22:20:27 dawes Exp $ */

/*
 * Copyright (c) 1995, 1996  The XFree86 Project, Inc
 */

/* THIS IS NOT AN X CONSORTIUM STANDARD */

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
#include "scrnintstr.h"
#include "inputstr.h"
#include "servermd.h"
#define _XF86MISC_SERVER_
#undef _XF86MISC_SAVER_COMPAT_
#include <X11/extensions/xf86mscstr.h>
#include "swaprep.h"
#include "xf86.h"
#include <X11/Xfuncproto.h>
#include "xf86miscproc.h"

#if 0
#include <X11/Xtrans/Xtrans.h>
#include "../os/osdep.h"
#include <X11/Xauth.h>
#ifndef USL
#ifndef Lynx
#include <sys/socket.h>
#else
#include <socket.h>
#endif
#endif /* USL */
#endif

#ifdef EXTMODULE
#include "xf86_ansic.h"
#endif

static int miscErrorBase;
static int MiscGeneration = 0;
static int MiscClientPrivateIndex;

/* This holds the client's version information */
typedef struct {
    int		major;
    int		minor;
} MiscPrivRec, *MiscPrivPtr;

#define MPRIV(c) ((c)->devPrivates[MiscClientPrivateIndex].ptr)

static void XF86MiscResetProc(
    ExtensionEntry* /* extEntry */
);

static void
ClientVersion(ClientPtr client, int *major, int *minor)
{
    MiscPrivPtr pPriv;

    pPriv = MPRIV(client);
    if (!pPriv) {
	if (major) *major = 0;
	if (minor) *minor = 0;
	return;
    }
    
    if (major) *major = pPriv->major;
    if (minor) *minor = pPriv->minor;
}

static DISPATCH_PROC(ProcXF86MiscDispatch);
static DISPATCH_PROC(ProcXF86MiscQueryVersion);
static DISPATCH_PROC(ProcXF86MiscGetKbdSettings);
static DISPATCH_PROC(ProcXF86MiscGetMouseSettings);
static DISPATCH_PROC(ProcXF86MiscSetKbdSettings);
static DISPATCH_PROC(ProcXF86MiscSetMouseSettings);
static DISPATCH_PROC(ProcXF86MiscSetGrabKeysState);
static DISPATCH_PROC(ProcXF86MiscSetClientVersion);
static DISPATCH_PROC(ProcXF86MiscGetFilePaths);
static DISPATCH_PROC(ProcXF86MiscPassMessage);
#ifdef _XF86MISC_SAVER_COMPAT_
static DISPATCH_PROC(ProcXF86MiscGetSaver);
static DISPATCH_PROC(ProcXF86MiscSetSaver);
#endif
static DISPATCH_PROC(SProcXF86MiscDispatch);
static DISPATCH_PROC(SProcXF86MiscQueryVersion);
static DISPATCH_PROC(SProcXF86MiscGetKbdSettings);
static DISPATCH_PROC(SProcXF86MiscGetMouseSettings);
static DISPATCH_PROC(SProcXF86MiscSetKbdSettings);
static DISPATCH_PROC(SProcXF86MiscSetMouseSettings);
static DISPATCH_PROC(SProcXF86MiscSetGrabKeysState);
static DISPATCH_PROC(SProcXF86MiscSetClientVersion);
static DISPATCH_PROC(SProcXF86MiscGetFilePaths);
static DISPATCH_PROC(SProcXF86MiscPassMessage);
#ifdef _XF86MISC_SAVER_COMPAT_
static DISPATCH_PROC(SProcXF86MiscGetSaver);
static DISPATCH_PROC(SProcXF86MiscSetSaver);
#endif

#if 0
static unsigned char XF86MiscReqCode = 0;
#endif

#ifdef DEBUG
# define DEBUG_P(x) ErrorF(x"\n");
#else
# define DEBUG_P(x) /**/
#endif

#define MISCERR(x)	(miscErrorBase + x)

void
XFree86MiscExtensionInit(void)
{
    ExtensionEntry* extEntry;

    DEBUG_P("XFree86MiscExtensionInit");

    if (!xf86GetModInDevEnabled())
	return;

    /*
     * Allocate a client private index to hold the client's version
     * information.
     */
    if (MiscGeneration != serverGeneration) {
	MiscClientPrivateIndex = AllocateClientPrivateIndex();
	/*
	 * Allocate 0 length, and use the private to hold a pointer to our
	 * MiscPrivRec.
	 */
	if (!AllocateClientPrivate(MiscClientPrivateIndex, 0)) {
	    ErrorF("XFree86MiscExtensionInit: "
		   "AllocateClientPrivate failed\n");
	    return;
	}
	MiscGeneration = serverGeneration;
    }
    
    if (
	(extEntry = AddExtension(XF86MISCNAME,
				XF86MiscNumberEvents,
				XF86MiscNumberErrors,
				ProcXF86MiscDispatch,
				SProcXF86MiscDispatch,
				XF86MiscResetProc,
				StandardMinorOpcode))) {
#if 0
	XF86MiscReqCode = (unsigned char)extEntry->base;
#endif
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

    DEBUG_P("XF86MiscQueryVersion");

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

#ifdef _XF86MISC_SAVER_COMPAT_
/* THESE HAVE NOT BEEN CONVERTED TO THE NEW DESIGN */

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

#endif /* _XF86MISC_SAVER_COMPAT_ */

static int
ProcXF86MiscGetMouseSettings(client)
    register ClientPtr client;
{
    xXF86MiscGetMouseSettingsReply rep;
    char *devname;
    pointer mouse;
    register int n;

    DEBUG_P("XF86MiscGetMouseSettings");

    REQUEST_SIZE_MATCH(xXF86MiscGetMouseSettingsReq);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;

    if (!MiscExtGetMouseSettings(&mouse, &devname))
	return BadValue;

    rep.mousetype =	  MiscExtGetMouseValue(mouse, MISC_MSE_PROTO);
    rep.baudrate =	  MiscExtGetMouseValue(mouse, MISC_MSE_BAUDRATE);
    rep.samplerate =	  MiscExtGetMouseValue(mouse, MISC_MSE_SAMPLERATE);
    rep.resolution =	  MiscExtGetMouseValue(mouse, MISC_MSE_RESOLUTION);
    rep.buttons =	  MiscExtGetMouseValue(mouse, MISC_MSE_BUTTONS);
    rep.emulate3buttons = MiscExtGetMouseValue(mouse, MISC_MSE_EM3BUTTONS);
    rep.emulate3timeout = MiscExtGetMouseValue(mouse, MISC_MSE_EM3TIMEOUT);
    rep.chordmiddle =	  MiscExtGetMouseValue(mouse, MISC_MSE_CHORDMIDDLE);
    rep.flags =		  MiscExtGetMouseValue(mouse, MISC_MSE_FLAGS);
    rep.devnamelen = (devname? strlen(devname): 0);
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
    MiscExtDestroyStruct(mouse, MISC_POINTER);
    
    if (rep.devnamelen)
        WriteToClient(client, rep.devnamelen, devname);
    return (client->noClientException);
}

static int
ProcXF86MiscGetKbdSettings(client)
    register ClientPtr client;
{
    xXF86MiscGetKbdSettingsReply rep;
    pointer kbd;
    register int n;

    DEBUG_P("XF86MiscGetKbdSettings");

    REQUEST_SIZE_MATCH(xXF86MiscGetKbdSettingsReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    if (!MiscExtGetKbdSettings(&kbd))
	return BadValue;

    rep.kbdtype =	MiscExtGetKbdValue(kbd, MISC_KBD_TYPE);
    rep.rate =		MiscExtGetKbdValue(kbd, MISC_KBD_RATE);
    rep.delay =		MiscExtGetKbdValue(kbd, MISC_KBD_DELAY);
    rep.servnumlock =	MiscExtGetKbdValue(kbd, MISC_KBD_SERVNUMLOCK);
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
    MiscExtReturn ret;
    pointer mouse;
    char *devname = NULL;
    int major, minor;
    
    REQUEST(xXF86MiscSetMouseSettingsReq);

    DEBUG_P("XF86MiscSetMouseSettings");

    REQUEST_AT_LEAST_SIZE(xXF86MiscSetMouseSettingsReq);

    ClientVersion(client, &major, &minor);
    
    if (xf86GetVerbosity() > 1) {
	ErrorF("SetMouseSettings - type: %d brate: %d srate: %d chdmid: %d\n",
		(int)stuff->mousetype, (int)stuff->baudrate,
		(int)stuff->samplerate, stuff->chordmiddle);
	ErrorF("                   em3but: %d em3tim: %d res: %d flags: %ld\n",
		stuff->emulate3buttons, (int)stuff->emulate3timeout,
		(int)stuff->resolution, (unsigned long)stuff->flags);
    }

    if ((mouse = MiscExtCreateStruct(MISC_POINTER)) == (pointer) 0)
	return BadAlloc;

    MiscExtSetMouseValue(mouse, MISC_MSE_PROTO,		stuff->mousetype);
    MiscExtSetMouseValue(mouse, MISC_MSE_BAUDRATE,	stuff->baudrate);
    MiscExtSetMouseValue(mouse, MISC_MSE_SAMPLERATE,	stuff->samplerate);
    MiscExtSetMouseValue(mouse, MISC_MSE_RESOLUTION,	stuff->resolution);
    MiscExtSetMouseValue(mouse, MISC_MSE_BUTTONS,	stuff->buttons);
    MiscExtSetMouseValue(mouse, MISC_MSE_EM3BUTTONS,	stuff->emulate3buttons);
    MiscExtSetMouseValue(mouse, MISC_MSE_EM3TIMEOUT,	stuff->emulate3timeout);
    MiscExtSetMouseValue(mouse, MISC_MSE_CHORDMIDDLE,	stuff->chordmiddle);
    MiscExtSetMouseValue(mouse, MISC_MSE_FLAGS,		stuff->flags);
    
    if ((major > 0 || minor > 5) && stuff->devnamelen) {
	int size = sizeof(xXF86MiscSetMouseSettingsReq) + stuff->devnamelen;
	size = (size + 3) >> 2;
	if (client->req_len < size)
	    return BadLength;
	if (stuff->devnamelen) {
	    if (!(devname = xalloc(stuff->devnamelen)))
		return BadAlloc;
	    strncpy(devname,(char*)(&stuff[1]),stuff->devnamelen);
	    if (xf86GetVerbosity() > 1)
		ErrorF("SetMouseSettings - device: %s\n",devname);
	    MiscExtSetMouseDevice(mouse, devname);
	}
    }

    ret = MiscExtApply(mouse, MISC_POINTER);

    if (devname)
	xfree(devname);
    
    switch ((ret)) {
        case MISC_RET_SUCCESS:      break;
	case MISC_RET_BADVAL:       return BadValue;
	case MISC_RET_BADMSEPROTO:  return MISCERR(XF86MiscBadMouseProtocol);
	case MISC_RET_BADBAUDRATE:  return MISCERR(XF86MiscBadMouseBaudRate);
	case MISC_RET_BADFLAGS:     return MISCERR(XF86MiscBadMouseFlags);
        case MISC_RET_BADCOMBO:     return MISCERR(XF86MiscBadMouseCombo);
        case MISC_RET_NOMODULE:     return MISCERR(XF86MiscNoModule);
        default:
	    ErrorF("Unexpected return from MiscExtApply(POINTER) = %d\n", ret);
	    return BadImplementation;
    }

    if (xf86GetVerbosity() > 1)
	ErrorF("SetMouseSettings - Succeeded\n");
    return (client->noClientException);
}

static int
ProcXF86MiscSetKbdSettings(client)
    register ClientPtr client;
{
    MiscExtReturn ret;
    pointer kbd;
    REQUEST(xXF86MiscSetKbdSettingsReq);

    DEBUG_P("XF86MiscSetKbdSettings");

    REQUEST_SIZE_MATCH(xXF86MiscSetKbdSettingsReq);

    if (xf86GetVerbosity() > 1)
	ErrorF("SetKbdSettings - type: %d rate: %d delay: %d snumlk: %d\n",
		(int)stuff->kbdtype, (int)stuff->rate,
		(int)stuff->delay, stuff->servnumlock);

    if ((kbd = MiscExtCreateStruct(MISC_KEYBOARD)) == (pointer) 0)
	return BadAlloc;

    MiscExtSetKbdValue(kbd, MISC_KBD_TYPE,		stuff->kbdtype);
    MiscExtSetKbdValue(kbd, MISC_KBD_RATE,		stuff->rate);
    MiscExtSetKbdValue(kbd, MISC_KBD_DELAY,		stuff->delay);
    MiscExtSetKbdValue(kbd, MISC_KBD_SERVNUMLOCK,	stuff->servnumlock);

    switch ((ret = MiscExtApply(kbd, MISC_KEYBOARD))) {
	case MISC_RET_SUCCESS:      break;
	case MISC_RET_BADVAL:       return BadValue;
	case MISC_RET_BADKBDTYPE:   return MISCERR(XF86MiscBadKbdType);
	default:
	    ErrorF("Unexpected return from MiscExtApply(KEYBOARD) = %d\n", ret);
	    return BadImplementation;
    }

    if (xf86GetVerbosity() > 1)
	ErrorF("SetKbdSettings - Succeeded\n");
    return (client->noClientException);
}

static int
ProcXF86MiscSetGrabKeysState(client)
    register ClientPtr client;
{
    int n, status;
    xXF86MiscSetGrabKeysStateReply rep;
    REQUEST(xXF86MiscSetGrabKeysStateReq);

    DEBUG_P("XF86MiscSetGrabKeysState");

    REQUEST_SIZE_MATCH(xXF86MiscSetGrabKeysStateReq);

    if ((status = MiscExtSetGrabKeysState(client, stuff->enable)) == 0) {
	if (xf86GetVerbosity() > 1)
	    ErrorF("SetGrabKeysState - %s\n",
		   stuff->enable ? "enabled" : "disabled");
    }

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.status = status;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swapl(&rep.status, n);
    }
    WriteToClient(client, SIZEOF(xXF86MiscSetGrabKeysStateReply), (char *)&rep);

    return (client->noClientException);
}

static int
ProcXF86MiscSetClientVersion(ClientPtr client)
{
    REQUEST(xXF86MiscSetClientVersionReq);

    MiscPrivPtr pPriv;

    DEBUG_P("XF86MiscSetClientVersion");

    REQUEST_SIZE_MATCH(xXF86MiscSetClientVersionReq);

    if ((pPriv = MPRIV(client)) == NULL) {
	pPriv = xalloc(sizeof(MiscPrivRec));
	if (!pPriv)
	    return BadAlloc;
	MPRIV(client) = pPriv;
    }
    ErrorF("SetClientVersion: %i %i\n",stuff->major,stuff->minor);
    pPriv->major = stuff->major;
    pPriv->minor = stuff->minor;
    
    return (client->noClientException);
}

static int
ProcXF86MiscGetFilePaths(client)
    register ClientPtr client;
{
    xXF86MiscGetFilePathsReply rep;
    const char *configfile;
    const char *modulepath;
    const char *logfile;
    register int n;

    DEBUG_P("XF86MiscGetFilePaths");

    REQUEST_SIZE_MATCH(xXF86MiscGetFilePathsReq);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;

    if (!MiscExtGetFilePaths(&configfile, &modulepath, &logfile))
	return BadValue;

    rep.configlen = (configfile? strlen(configfile): 0);
    rep.modulelen = (modulepath? strlen(modulepath): 0);
    rep.loglen = (logfile? strlen(logfile): 0);
    rep.length = (SIZEOF(xXF86MiscGetFilePathsReply) - SIZEOF(xGenericReply) +
		  ((rep.configlen + 3) & ~3) +
		  ((rep.modulelen + 3) & ~3) +
		  ((rep.loglen + 3) & ~3) ) >> 2;
    
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swaps(&rep.configlen, n);
    	swaps(&rep.modulelen, n);
    	swaps(&rep.loglen, n);
    }
    WriteToClient(client, SIZEOF(xXF86MiscGetFilePathsReply), (char *)&rep);
    
    if (rep.configlen)
        WriteToClient(client, rep.configlen, (char *)configfile);
    if (rep.modulelen)
        WriteToClient(client, rep.modulelen, (char *)modulepath);
    if (rep.loglen)
        WriteToClient(client, rep.loglen, (char *)logfile);

    return (client->noClientException);
}

static int
ProcXF86MiscPassMessage(client)
    register ClientPtr client;
{
    xXF86MiscPassMessageReply rep;
    char *msgtype, *msgval, *retstr;
    int retval, size;
    register int n;

    REQUEST(xXF86MiscPassMessageReq);

    DEBUG_P("XF86MiscPassMessage");

    REQUEST_AT_LEAST_SIZE(xXF86MiscPassMessageReq);
    size = (sizeof(xXF86MiscPassMessageReq) + 3) >> 2;
    size+= (stuff->typelen + 3) >> 2;
    size+= (stuff->vallen  + 3) >> 2;
    if (client->req_len < size)
	return BadLength;
    if (stuff->typelen) {
	if (!(msgtype = xalloc(stuff->typelen)))
	    return BadAlloc;
	strncpy(msgtype,(char*)(&stuff[1]),stuff->typelen);
    } else return BadValue;
    if (stuff->vallen) {
	if (!(msgval = xalloc(stuff->vallen))) {
	    xfree(msgtype);
	    return BadAlloc;
	}
	strncpy(msgval,(char*)((char*)&stuff[1] + ((stuff->typelen + 3) & ~3)),
			stuff->vallen);
    } else {
	xfree(msgtype);
	return BadValue;
    }

    if ((retval = MiscExtPassMessage(stuff->screen,msgtype,msgval,&retstr)) != 0) {
	xfree(msgtype);
	xfree(msgval);
	return retval;
    }

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.mesglen = (retstr? strlen(retstr): 0);
    rep.length = (SIZEOF(xXF86MiscPassMessageReply) - SIZEOF(xGenericReply) +
		  ((rep.mesglen + 3) & ~3)) >> 2;
    rep.status = 0;
    
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swaps(&rep.mesglen, n);
    }
    WriteToClient(client, SIZEOF(xXF86MiscPassMessageReply), (char *)&rep);
    
    if (rep.mesglen)
        WriteToClient(client, rep.mesglen, (char *)retstr);

    xfree(msgtype);
    xfree(msgval);
    
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
#ifdef _XF86MISC_SAVER_COMPAT_
    case X_XF86MiscGetSaver:
	return ProcXF86MiscGetSaver(client);
    case X_XF86MiscSetSaver:
	return ProcXF86MiscSetSaver(client);
#endif
    case X_XF86MiscGetMouseSettings:
	return ProcXF86MiscGetMouseSettings(client);
    case X_XF86MiscGetKbdSettings:
	return ProcXF86MiscGetKbdSettings(client);
    case X_XF86MiscSetClientVersion:
		return ProcXF86MiscSetClientVersion(client);
    case X_XF86MiscGetFilePaths:
	return ProcXF86MiscGetFilePaths(client);
    case X_XF86MiscPassMessage:
	return ProcXF86MiscPassMessage(client);
    default:
	if (!xf86GetModInDevEnabled())
	    return miscErrorBase + XF86MiscModInDevDisabled;
	if (xf86GetModInDevAllowNonLocal() || LocalClient (client)) {
	    switch (stuff->data) {
	        case X_XF86MiscSetMouseSettings:
		    return ProcXF86MiscSetMouseSettings(client);
	        case X_XF86MiscSetKbdSettings:
		    return ProcXF86MiscSetKbdSettings(client);
		case X_XF86MiscSetGrabKeysState:
		    return ProcXF86MiscSetGrabKeysState(client);
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

#ifdef _XF86MISC_SAVER_COMPAT_
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
#endif /* _XF86MISC_SAVER_COMPAT_ */

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
SProcXF86MiscSetGrabKeysState(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xXF86MiscSetGrabKeysStateReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86MiscSetGrabKeysStateReq);
    swaps(&stuff->enable, n);
    return ProcXF86MiscSetGrabKeysState(client);
}

static int
SProcXF86MiscSetClientVersion(ClientPtr client)
{
    register int n;
    REQUEST(xXF86MiscSetClientVersionReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86MiscSetClientVersionReq);
    swaps(&stuff->major, n);
    swaps(&stuff->minor, n);
    return ProcXF86MiscSetClientVersion(client);
}

static int
SProcXF86MiscGetFilePaths(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xXF86MiscGetFilePathsReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86MiscGetFilePathsReq);
    return ProcXF86MiscGetFilePaths(client);
}

static int
SProcXF86MiscPassMessage(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xXF86MiscPassMessageReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86MiscPassMessageReq);
    return ProcXF86MiscPassMessage(client);
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
#ifdef _XF86MISC_SAVER_COMPAT_
    case X_XF86MiscGetSaver:
	return SProcXF86MiscGetSaver(client);
    case X_XF86MiscSetSaver:
	return SProcXF86MiscSetSaver(client);
#endif
    case X_XF86MiscGetMouseSettings:
	return SProcXF86MiscGetMouseSettings(client);
    case X_XF86MiscGetKbdSettings:
	return SProcXF86MiscGetKbdSettings(client);
    case X_XF86MiscSetClientVersion:
	return SProcXF86MiscSetClientVersion(client);
    case X_XF86MiscGetFilePaths:
	return SProcXF86MiscGetFilePaths(client);
    case X_XF86MiscPassMessage:
	return SProcXF86MiscPassMessage(client);
    default:
	if (!xf86GetModInDevEnabled())
	    return miscErrorBase + XF86MiscModInDevDisabled;
	if (xf86GetModInDevAllowNonLocal() || LocalClient (client)) {
	    switch (stuff->data) {
	        case X_XF86MiscSetMouseSettings:
		    return SProcXF86MiscSetMouseSettings(client);
	        case X_XF86MiscSetKbdSettings:
		    return SProcXF86MiscSetKbdSettings(client);
	        case X_XF86MiscSetGrabKeysState:
		    return SProcXF86MiscSetGrabKeysState(client);
	        default:
		    return BadRequest;
	    }
	} else
	    return miscErrorBase + XF86MiscModInDevClientNotLocal;
    }
}

