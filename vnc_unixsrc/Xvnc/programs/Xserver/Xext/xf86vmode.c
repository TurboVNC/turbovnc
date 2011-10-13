/* $XFree86: xc/programs/Xserver/Xext/xf86vmode.c,v 3.58 2003/11/06 18:37:57 tsi Exp $ */

/*

Copyright 1995  Kaleb S. KEITHLEY

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL Kaleb S. KEITHLEY BE LIABLE FOR ANY CLAIM, DAMAGES 
OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Kaleb S. KEITHLEY 
shall not be used in advertising or otherwise to promote the sale, use 
or other dealings in this Software without prior written authorization
from Kaleb S. KEITHLEY

*/
/* $XdotOrg: xc/programs/Xserver/Xext/xf86vmode.c,v 1.8 2005/07/16 03:49:58 kem Exp $ */
/* $Xorg: xf86vmode.c,v 1.3 2000/08/17 19:47:59 cpqbld Exp $ */
/* THIS IS NOT AN X CONSORTIUM STANDARD OR AN X PROJECT TEAM SPECIFICATION */

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
#include "servermd.h"
#define _XF86VIDMODE_SERVER_
#include <X11/extensions/xf86vmstr.h>
#include "swaprep.h"
#include "xf86.h"
#include "vidmodeproc.h"

#ifdef EXTMODULE
#include "xf86_ansic.h"
#endif

#define DEFAULT_XF86VIDMODE_VERBOSITY	3

static int VidModeErrorBase;
static int VidModeGeneration = 0;
static int VidModeClientPrivateIndex;

/* This holds the client's version information */
typedef struct {
    int		major;
    int		minor;
} VidModePrivRec, *VidModePrivPtr;

#define VMPRIV(c) ((c)->devPrivates[VidModeClientPrivateIndex].ptr)

static void XF86VidModeResetProc(
    ExtensionEntry* /* extEntry */
);

static DISPATCH_PROC(ProcXF86VidModeDispatch);
static DISPATCH_PROC(ProcXF86VidModeGetAllModeLines);
static DISPATCH_PROC(ProcXF86VidModeGetModeLine);
static DISPATCH_PROC(ProcXF86VidModeGetMonitor);
static DISPATCH_PROC(ProcXF86VidModeLockModeSwitch);
static DISPATCH_PROC(ProcXF86VidModeAddModeLine);
static DISPATCH_PROC(ProcXF86VidModeDeleteModeLine);
static DISPATCH_PROC(ProcXF86VidModeModModeLine);
static DISPATCH_PROC(ProcXF86VidModeValidateModeLine);
static DISPATCH_PROC(ProcXF86VidModeQueryVersion);
static DISPATCH_PROC(ProcXF86VidModeSwitchMode);
static DISPATCH_PROC(ProcXF86VidModeSwitchToMode);
static DISPATCH_PROC(ProcXF86VidModeGetViewPort);
static DISPATCH_PROC(ProcXF86VidModeSetViewPort);
static DISPATCH_PROC(ProcXF86VidModeGetDotClocks);
static DISPATCH_PROC(ProcXF86VidModeSetGamma);
static DISPATCH_PROC(ProcXF86VidModeGetGamma);
static DISPATCH_PROC(ProcXF86VidModeSetClientVersion);
static DISPATCH_PROC(ProcXF86VidModeGetGammaRamp);
static DISPATCH_PROC(ProcXF86VidModeSetGammaRamp);
static DISPATCH_PROC(ProcXF86VidModeGetGammaRampSize);
static DISPATCH_PROC(SProcXF86VidModeDispatch);
static DISPATCH_PROC(SProcXF86VidModeGetAllModeLines);
static DISPATCH_PROC(SProcXF86VidModeGetModeLine);
static DISPATCH_PROC(SProcXF86VidModeGetMonitor);
static DISPATCH_PROC(SProcXF86VidModeLockModeSwitch);
static DISPATCH_PROC(SProcXF86VidModeAddModeLine);
static DISPATCH_PROC(SProcXF86VidModeDeleteModeLine);
static DISPATCH_PROC(SProcXF86VidModeModModeLine);
static DISPATCH_PROC(SProcXF86VidModeValidateModeLine);
static DISPATCH_PROC(SProcXF86VidModeQueryVersion);
static DISPATCH_PROC(SProcXF86VidModeSwitchMode);
static DISPATCH_PROC(SProcXF86VidModeSwitchToMode);
static DISPATCH_PROC(SProcXF86VidModeGetViewPort);
static DISPATCH_PROC(SProcXF86VidModeSetViewPort);
static DISPATCH_PROC(SProcXF86VidModeGetDotClocks);
static DISPATCH_PROC(SProcXF86VidModeSetGamma);
static DISPATCH_PROC(SProcXF86VidModeGetGamma);
static DISPATCH_PROC(SProcXF86VidModeSetClientVersion);
static DISPATCH_PROC(SProcXF86VidModeGetGammaRamp);
static DISPATCH_PROC(SProcXF86VidModeSetGammaRamp);
static DISPATCH_PROC(SProcXF86VidModeGetGammaRampSize);

#if 0
static unsigned char XF86VidModeReqCode = 0;
#endif

/* The XF86VIDMODE_EVENTS code is far from complete */

#ifdef XF86VIDMODE_EVENTS
static int XF86VidModeEventBase = 0;

static void SXF86VidModeNotifyEvent();
    xXF86VidModeNotifyEvent * /* from */,
    xXF86VidModeNotifyEvent * /* to */
);

extern WindowPtr *WindowTable;

static RESTYPE EventType;	/* resource type for event masks */

typedef struct _XF86VidModeEvent *XF86VidModeEventPtr;

typedef struct _XF86VidModeEvent {
    XF86VidModeEventPtr	next;
    ClientPtr		client;
    ScreenPtr		screen;
    XID			resource;
    CARD32		mask;
} XF86VidModeEventRec;

static int XF86VidModeFreeEvents();

typedef struct _XF86VidModeScreenPrivate {
    XF86VidModeEventPtr	events;
    Bool		hasWindow;
} XF86VidModeScreenPrivateRec, *XF86VidModeScreenPrivatePtr;
   
static int ScreenPrivateIndex;

#define GetScreenPrivate(s) ((ScreenSaverScreenPrivatePtr)(s)->devPrivates[ScreenPrivateIndex].ptr)
#define SetScreenPrivate(s,v) ((s)->devPrivates[ScreenPrivateIndex].ptr = (pointer) v);
#define SetupScreen(s)  ScreenSaverScreenPrivatePtr pPriv = GetScreenPrivate(s)

#define New(t)  (xalloc (sizeof (t)))
#endif

#ifdef DEBUG
# define DEBUG_P(x) ErrorF(x"\n");
#else
# define DEBUG_P(x) /**/
#endif

void
XFree86VidModeExtensionInit(void)
{
    ExtensionEntry* extEntry;
    ScreenPtr pScreen;
    int		    i;
    Bool	    enabled = FALSE;

    DEBUG_P("XFree86VidModeExtensionInit");

#ifdef XF86VIDMODE_EVENTS
    EventType = CreateNewResourceType(XF86VidModeFreeEvents);
    ScreenPrivateIndex = AllocateScreenPrivateIndex ();
#endif

    for(i = 0; i < screenInfo.numScreens; i++) {
        pScreen = screenInfo.screens[i];
	if (VidModeExtensionInit(pScreen))
	    enabled = TRUE;
#ifdef XF86VIDMODE_EVENTS
	SetScreenPrivate (pScreen, NULL);
#endif
    }
    /* This means that the DDX doesn't want the vidmode extension enabled */
    if (!enabled)
	return;

    /*
     * Allocate a client private index to hold the client's version
     * information.
     */
    if (VidModeGeneration != serverGeneration) {
	VidModeClientPrivateIndex = AllocateClientPrivateIndex();
	/*
	 * Allocate 0 length, and use the private to hold a pointer to our
	 * VidModePrivRec.
	 */
	if (!AllocateClientPrivate(VidModeClientPrivateIndex, 0)) {
	    ErrorF("XFree86VidModeExtensionInit: "
		   "AllocateClientPrivate failed\n");
	    return;
	}
	VidModeGeneration = serverGeneration;
    }

    if (
#ifdef XF86VIDMODE_EVENTS
        EventType && ScreenPrivateIndex != -1 &&
#endif
	(extEntry = AddExtension(XF86VIDMODENAME,
				XF86VidModeNumberEvents,
				XF86VidModeNumberErrors,
				ProcXF86VidModeDispatch,
				SProcXF86VidModeDispatch,
				XF86VidModeResetProc,
				StandardMinorOpcode))) {
#if 0
	XF86VidModeReqCode = (unsigned char)extEntry->base;
#endif
	VidModeErrorBase = extEntry->errorBase;
#ifdef XF86VIDMODE_EVENTS
	XF86VidModeEventBase = extEntry->eventBase;
	EventSwapVector[XF86VidModeEventBase] = (EventSwapPtr)SXF86VidModeNotifyEvent;
#endif
    }
}

/*ARGSUSED*/
static void
XF86VidModeResetProc (extEntry)
    ExtensionEntry* extEntry;
{
}

static int
ClientMajorVersion(ClientPtr client)
{
    VidModePrivPtr pPriv;

    pPriv = VMPRIV(client);
    if (!pPriv)
	return 0;
    else
	return pPriv->major;
}

#ifdef XF86VIDMODE_EVENTS
static void
CheckScreenPrivate (pScreen)
    ScreenPtr	pScreen;
{
    SetupScreen (pScreen);

    if (!pPriv)
	return;
    if (!pPriv->events && !pPriv->hasWindow) {
	xfree (pPriv);
	SetScreenPrivate (pScreen, NULL);
    }
}
    
static XF86VidModeScreenPrivatePtr
MakeScreenPrivate (pScreen)
    ScreenPtr	pScreen;
{
    SetupScreen (pScreen);

    if (pPriv)
	return pPriv;
    pPriv = New (XF86VidModeScreenPrivateRec);
    if (!pPriv)
	return 0;
    pPriv->events = 0;
    pPriv->hasWindow = FALSE;
    SetScreenPrivate (pScreen, pPriv);
    return pPriv;
}

static unsigned long
getEventMask (ScreenPtr pScreen, ClientPtr client)
{
    SetupScreen(pScreen);
    XF86VidModeEventPtr pEv;

    if (!pPriv)
	return 0;
    for (pEv = pPriv->events; pEv; pEv = pEv->next)
	if (pEv->client == client)
	    return pEv->mask;
    return 0;
}

static Bool
setEventMask (ScreenPtr pScreen, ClientPtr client, unsigned long mask)
{
    SetupScreen(pScreen);
    XF86VidModeEventPtr pEv, *pPrev;

    if (getEventMask (pScreen, client) == mask)
	return TRUE;
    if (!pPriv) {
	pPriv = MakeScreenPrivate (pScreen);
	if (!pPriv)
	    return FALSE;
    }
    for (pPrev = &pPriv->events; pEv = *pPrev; pPrev = &pEv->next)
	if (pEv->client == client)
	    break;
    if (mask == 0) {
	*pPrev = pEv->next;
	xfree (pEv);
	CheckScreenPrivate (pScreen);
    } else {
	if (!pEv) {
	    pEv = New (ScreenSaverEventRec);
	    if (!pEv) {
		CheckScreenPrivate (pScreen);
		return FALSE;
	    }
	    *pPrev = pEv;
	    pEv->next = NULL;
	    pEv->client = client;
	    pEv->screen = pScreen;
	    pEv->resource = FakeClientID (client->index);
	}
	pEv->mask = mask;
    }
    return TRUE;
}

static int
XF86VidModeFreeEvents(pointer value, XID id)
{
    XF86VidModeEventPtr	pOld = (XF86VidModeEventPtr)value;
    ScreenPtr pScreen = pOld->screen;
    SetupScreen (pScreen);
    XF86VidModeEventPtr	pEv, *pPrev;

    if (!pPriv)
	return TRUE;
    for (pPrev = &pPriv->events; pEv = *pPrev; pPrev = &pEv->next)
	if (pEv == pOld)
	    break;
    if (!pEv)
	return TRUE;
    *pPrev = pEv->next;
    xfree (pEv);
    CheckScreenPrivate (pScreen);
    return TRUE;
}

static void
SendXF86VidModeNotify(ScreenPtr pScreen, int state, Bool forced)
{
    XF86VidModeScreenPrivatePtr	pPriv;
    XF86VidModeEventPtr		pEv;
    unsigned long		mask;
    xXF86VidModeNotifyEvent	ev;
    ClientPtr			client;
    int				kind;

    UpdateCurrentTimeIf ();
    mask = XF86VidModeNotifyMask;
    pScreen = screenInfo.screens[pScreen->myNum];
    pPriv = GetScreenPrivate(pScreen);
    if (!pPriv)
	return;
    kind = XF86VidModeModeChange;
    for (pEv = pPriv->events; pEv; pEv = pEv->next)
    {
	client = pEv->client;
	if (client->clientGone)
	    continue;
	if (!(pEv->mask & mask))
	    continue;
	ev.type = XF86VidModeNotify + XF86VidModeEventBase;
	ev.state = state;
	ev.sequenceNumber = client->sequence;
	ev.timestamp = currentTime.milliseconds;
	ev.root = WindowTable[pScreen->myNum]->drawable.id;
	ev.kind = kind;
	ev.forced = forced;
	WriteEventsToClient (client, 1, (xEvent *) &ev);
    }
}

static void
SXF86VidModeNotifyEvent(xXF86VidModeNotifyEvent *from,
			xXF86VidModeNotifyEvent *to)
{
    to->type = from->type;
    to->state = from->state;
    cpswaps (from->sequenceNumber, to->sequenceNumber);
    cpswapl (from->timestamp, to->timestamp);    
    cpswapl (from->root, to->root);    
    to->kind = from->kind;
    to->forced = from->forced;
}
#endif
	
static int
ProcXF86VidModeQueryVersion(ClientPtr client)
{
    xXF86VidModeQueryVersionReply rep;
    register int n;

    DEBUG_P("XF86VidModeQueryVersion");

    REQUEST_SIZE_MATCH(xXF86VidModeQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = XF86VIDMODE_MAJOR_VERSION;
    rep.minorVersion = XF86VIDMODE_MINOR_VERSION;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swaps(&rep.majorVersion, n);
    	swaps(&rep.minorVersion, n);
    }
    WriteToClient(client, sizeof(xXF86VidModeQueryVersionReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXF86VidModeGetModeLine(ClientPtr client)
{
    REQUEST(xXF86VidModeGetModeLineReq);
    xXF86VidModeGetModeLineReply rep;
    xXF86OldVidModeGetModeLineReply oldrep;
    pointer mode;
    register int n;
    int dotClock;
    int ver;

    DEBUG_P("XF86VidModeGetModeline");

    ver = ClientMajorVersion(client);
    REQUEST_SIZE_MATCH(xXF86VidModeGetModeLineReq);
    rep.type = X_Reply;
    if (ver < 2) {
	rep.length = (SIZEOF(xXF86OldVidModeGetModeLineReply) -
			SIZEOF(xGenericReply)) >> 2;
    } else {
	rep.length = (SIZEOF(xXF86VidModeGetModeLineReply) -
			SIZEOF(xGenericReply)) >> 2;
    }
    rep.sequenceNumber = client->sequence;

    if(stuff->screen >= screenInfo.numScreens)
        return BadValue;

    if (!VidModeGetCurrentModeline(stuff->screen, &mode, &dotClock))
	return BadValue;

    rep.dotclock = dotClock;
    rep.hdisplay = VidModeGetModeValue(mode, VIDMODE_H_DISPLAY);
    rep.hsyncstart = VidModeGetModeValue(mode, VIDMODE_H_SYNCSTART);
    rep.hsyncend = VidModeGetModeValue(mode, VIDMODE_H_SYNCEND);
    rep.htotal = VidModeGetModeValue(mode, VIDMODE_H_TOTAL);
    rep.hskew = VidModeGetModeValue(mode, VIDMODE_H_SKEW);
    rep.vdisplay = VidModeGetModeValue(mode, VIDMODE_V_DISPLAY);
    rep.vsyncstart = VidModeGetModeValue(mode, VIDMODE_V_SYNCSTART);
    rep.vsyncend = VidModeGetModeValue(mode, VIDMODE_V_SYNCEND);
    rep.vtotal = VidModeGetModeValue(mode, VIDMODE_V_TOTAL);
    rep.flags = VidModeGetModeValue(mode, VIDMODE_FLAGS);

    if (xf86GetVerbosity() > DEFAULT_XF86VIDMODE_VERBOSITY) {
	ErrorF("GetModeLine - scrn: %d clock: %ld\n",
	       stuff->screen, (unsigned long)rep.dotclock);
	ErrorF("GetModeLine - hdsp: %d hbeg: %d hend: %d httl: %d\n",
	       rep.hdisplay, rep.hsyncstart,
	       rep.hsyncend, rep.htotal);
	ErrorF("              vdsp: %d vbeg: %d vend: %d vttl: %d flags: %ld\n",
	       rep.vdisplay, rep.vsyncstart, rep.vsyncend,
	       rep.vtotal, (unsigned long)rep.flags);
    }
    
    /*
     * Older servers sometimes had server privates that the VidMode 
     * extention made available. So to be compatiable pretend that
     * there are no server privates to pass to the client
     */
    rep.privsize = 0;

    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swapl(&rep.dotclock, n);
    	swaps(&rep.hdisplay, n);
    	swaps(&rep.hsyncstart, n);
    	swaps(&rep.hsyncend, n);
    	swaps(&rep.htotal, n);
    	swaps(&rep.hskew, n);
    	swaps(&rep.vdisplay, n);
    	swaps(&rep.vsyncstart, n);
    	swaps(&rep.vsyncend, n);
    	swaps(&rep.vtotal, n);
	swapl(&rep.flags, n);
	swapl(&rep.privsize, n);
    }
    if (ver < 2) {
	oldrep.type = rep.type;
	oldrep.sequenceNumber = rep.sequenceNumber;
	oldrep.length = rep.length;
	oldrep.dotclock = rep.dotclock;
	oldrep.hdisplay = rep.hdisplay;
	oldrep.hsyncstart = rep.hsyncstart;
	oldrep.hsyncend = rep.hsyncend;
	oldrep.htotal = rep.htotal;
	oldrep.vdisplay = rep.vdisplay;
	oldrep.vsyncstart = rep.vsyncstart;
	oldrep.vsyncend = rep.vsyncend;
	oldrep.vtotal = rep.vtotal;
	oldrep.flags = rep.flags;
	oldrep.privsize = rep.privsize;
	WriteToClient(client, sizeof(xXF86OldVidModeGetModeLineReply),
			(char *)&oldrep);
    } else {
	WriteToClient(client, sizeof(xXF86VidModeGetModeLineReply),
			(char *)&rep);
    }
    return (client->noClientException);
}

static int
ProcXF86VidModeGetAllModeLines(ClientPtr client)
{
    REQUEST(xXF86VidModeGetAllModeLinesReq);
    xXF86VidModeGetAllModeLinesReply rep;
    xXF86VidModeModeInfo mdinf;
    xXF86OldVidModeModeInfo oldmdinf;
    pointer mode;
    int modecount, dotClock;
    register int n;
    int ver;

    DEBUG_P("XF86VidModeGetAllModelines");

    REQUEST_SIZE_MATCH(xXF86VidModeGetAllModeLinesReq);

    if(stuff->screen >= screenInfo.numScreens)
        return BadValue;

    ver = ClientMajorVersion(client);

    modecount = VidModeGetNumOfModes(stuff->screen);
    if (modecount < 1)
      return (VidModeErrorBase + XF86VidModeExtensionDisabled);

    if (!VidModeGetFirstModeline(stuff->screen, &mode, &dotClock))
	return BadValue;
    
    rep.type = X_Reply;
    rep.length = SIZEOF(xXF86VidModeGetAllModeLinesReply) -
		 SIZEOF(xGenericReply);
    if (ver < 2)
	rep.length += modecount * sizeof(xXF86OldVidModeModeInfo);
    else
	rep.length += modecount * sizeof(xXF86VidModeModeInfo);
    rep.length >>= 2;
    rep.sequenceNumber = client->sequence;
    rep.modecount = modecount;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swapl(&rep.modecount, n);
    }
    WriteToClient(client, sizeof(xXF86VidModeGetAllModeLinesReply), (char *)&rep);

    do {
	mdinf.dotclock = dotClock;
	mdinf.hdisplay = VidModeGetModeValue(mode, VIDMODE_H_DISPLAY);
	mdinf.hsyncstart = VidModeGetModeValue(mode, VIDMODE_H_SYNCSTART);
	mdinf.hsyncend = VidModeGetModeValue(mode, VIDMODE_H_SYNCEND);
	mdinf.htotal = VidModeGetModeValue(mode, VIDMODE_H_TOTAL);
	mdinf.hskew = VidModeGetModeValue(mode, VIDMODE_H_SKEW);
	mdinf.vdisplay = VidModeGetModeValue(mode, VIDMODE_V_DISPLAY);
	mdinf.vsyncstart = VidModeGetModeValue(mode, VIDMODE_V_SYNCSTART);
	mdinf.vsyncend = VidModeGetModeValue(mode, VIDMODE_V_SYNCEND);
	mdinf.vtotal = VidModeGetModeValue(mode, VIDMODE_V_TOTAL);
	mdinf.flags = VidModeGetModeValue(mode, VIDMODE_FLAGS);
	mdinf.privsize = 0;
        if (client->swapped) {
	    swapl(&mdinf.dotclock, n);
    	    swaps(&mdinf.hdisplay, n);
    	    swaps(&mdinf.hsyncstart, n);
    	    swaps(&mdinf.hsyncend, n);
    	    swaps(&mdinf.htotal, n);
    	    swaps(&mdinf.hskew, n);
    	    swaps(&mdinf.vdisplay, n);
    	    swaps(&mdinf.vsyncstart, n);
    	    swaps(&mdinf.vsyncend, n);
    	    swaps(&mdinf.vtotal, n);
	    swapl(&mdinf.flags, n);
	    swapl(&mdinf.privsize, n);
	}
	if (ver < 2) {
	    oldmdinf.dotclock = mdinf.dotclock;
	    oldmdinf.hdisplay = mdinf.hdisplay;
	    oldmdinf.hsyncstart = mdinf.hsyncstart;
	    oldmdinf.hsyncend = mdinf.hsyncend;
	    oldmdinf.htotal = mdinf.htotal;
	    oldmdinf.vdisplay = mdinf.vdisplay;
	    oldmdinf.vsyncstart = mdinf.vsyncstart;
	    oldmdinf.vsyncend = mdinf.vsyncend;
	    oldmdinf.vtotal = mdinf.vtotal;
	    oldmdinf.flags = mdinf.flags;
	    oldmdinf.privsize = mdinf.privsize;
            WriteToClient(client, sizeof(xXF86OldVidModeModeInfo),
			  (char *)&oldmdinf);
	} else {
            WriteToClient(client, sizeof(xXF86VidModeModeInfo), (char *)&mdinf);
	}

   } while (VidModeGetNextModeline(stuff->screen, &mode, &dotClock));

    return (client->noClientException);
}

#define MODEMATCH(mode,stuff)	  \
     (VidModeGetModeValue(mode, VIDMODE_H_DISPLAY)  == stuff->hdisplay \
     && VidModeGetModeValue(mode, VIDMODE_H_SYNCSTART)  == stuff->hsyncstart \
     && VidModeGetModeValue(mode, VIDMODE_H_SYNCEND)  == stuff->hsyncend \
     && VidModeGetModeValue(mode, VIDMODE_H_TOTAL)  == stuff->htotal \
     && VidModeGetModeValue(mode, VIDMODE_V_DISPLAY)  == stuff->vdisplay \
     && VidModeGetModeValue(mode, VIDMODE_V_SYNCSTART)  == stuff->vsyncstart \
     && VidModeGetModeValue(mode, VIDMODE_V_SYNCEND)  == stuff->vsyncend \
     && VidModeGetModeValue(mode, VIDMODE_V_TOTAL)  == stuff->vtotal \
     && VidModeGetModeValue(mode, VIDMODE_FLAGS)  == stuff->flags )

static int
ProcXF86VidModeAddModeLine(ClientPtr client)
{
    REQUEST(xXF86VidModeAddModeLineReq);
    xXF86OldVidModeAddModeLineReq *oldstuff =
			(xXF86OldVidModeAddModeLineReq *)client->requestBuffer;
    xXF86VidModeAddModeLineReq newstuff;
    pointer mode;
    int len;
    int dotClock;
    int ver;

    DEBUG_P("XF86VidModeAddModeline");

    ver = ClientMajorVersion(client);
    if (ver < 2) {
	/* convert from old format */
	stuff = &newstuff;
	stuff->length = oldstuff->length;
	stuff->screen = oldstuff->screen;
	stuff->dotclock = oldstuff->dotclock;
	stuff->hdisplay = oldstuff->hdisplay;
	stuff->hsyncstart = oldstuff->hsyncstart;
	stuff->hsyncend = oldstuff->hsyncend;
	stuff->htotal = oldstuff->htotal;
	stuff->hskew = 0;
	stuff->vdisplay = oldstuff->vdisplay;
	stuff->vsyncstart = oldstuff->vsyncstart;
	stuff->vsyncend = oldstuff->vsyncend;
	stuff->vtotal = oldstuff->vtotal;
	stuff->flags = oldstuff->flags;
	stuff->privsize = oldstuff->privsize;
	stuff->after_dotclock = oldstuff->after_dotclock;
	stuff->after_hdisplay = oldstuff->after_hdisplay;
	stuff->after_hsyncstart = oldstuff->after_hsyncstart;
	stuff->after_hsyncend = oldstuff->after_hsyncend;
	stuff->after_htotal = oldstuff->after_htotal;
	stuff->after_hskew = 0;
	stuff->after_vdisplay = oldstuff->after_vdisplay;
	stuff->after_vsyncstart = oldstuff->after_vsyncstart;
	stuff->after_vsyncend = oldstuff->after_vsyncend;
	stuff->after_vtotal = oldstuff->after_vtotal;
	stuff->after_flags = oldstuff->after_flags;
    }
    if (xf86GetVerbosity() > DEFAULT_XF86VIDMODE_VERBOSITY) {
	ErrorF("AddModeLine - scrn: %d clock: %ld\n",
		(int)stuff->screen, (unsigned long)stuff->dotclock);
	ErrorF("AddModeLine - hdsp: %d hbeg: %d hend: %d httl: %d\n",
		stuff->hdisplay, stuff->hsyncstart,
		stuff->hsyncend, stuff->htotal);
	ErrorF("              vdsp: %d vbeg: %d vend: %d vttl: %d flags: %ld\n",
		stuff->vdisplay, stuff->vsyncstart, stuff->vsyncend,
		stuff->vtotal, (unsigned long)stuff->flags);
	ErrorF("      after - scrn: %d clock: %ld\n",
		(int)stuff->screen, (unsigned long)stuff->after_dotclock);
	ErrorF("              hdsp: %d hbeg: %d hend: %d httl: %d\n",
		stuff->after_hdisplay, stuff->after_hsyncstart,
		stuff->after_hsyncend, stuff->after_htotal);
	ErrorF("              vdsp: %d vbeg: %d vend: %d vttl: %d flags: %ld\n",
		stuff->after_vdisplay, stuff->after_vsyncstart,
		stuff->after_vsyncend, stuff->after_vtotal,
		(unsigned long)stuff->after_flags);
    }

    if (ver < 2) {
	REQUEST_AT_LEAST_SIZE(xXF86OldVidModeAddModeLineReq);
	len = client->req_len - (sizeof(xXF86OldVidModeAddModeLineReq) >> 2);
    } else {
	REQUEST_AT_LEAST_SIZE(xXF86VidModeAddModeLineReq);
	len = client->req_len - (sizeof(xXF86VidModeAddModeLineReq) >> 2);
    }
    if (len != stuff->privsize)
	return BadLength;

    if(stuff->screen >= screenInfo.numScreens)
        return BadValue;

    if (stuff->hsyncstart < stuff->hdisplay   ||
	stuff->hsyncend   < stuff->hsyncstart ||
	stuff->htotal     < stuff->hsyncend   ||
	stuff->vsyncstart < stuff->vdisplay   ||
	stuff->vsyncend   < stuff->vsyncstart ||
	stuff->vtotal     < stuff->vsyncend)
	return BadValue;

    if (stuff->after_hsyncstart < stuff->after_hdisplay   ||
	stuff->after_hsyncend   < stuff->after_hsyncstart ||
	stuff->after_htotal     < stuff->after_hsyncend   ||
	stuff->after_vsyncstart < stuff->after_vdisplay   ||
	stuff->after_vsyncend   < stuff->after_vsyncstart ||
	stuff->after_vtotal     < stuff->after_vsyncend)
	return BadValue;

    if (stuff->after_htotal != 0 || stuff->after_vtotal != 0) {
	Bool found = FALSE;
	if (VidModeGetFirstModeline(stuff->screen, &mode, &dotClock)) {
	    do {
		if ((VidModeGetDotClock(stuff->screen, stuff->dotclock)
			== dotClock) && MODEMATCH(mode, stuff)) {
		    found = TRUE;
		    break;
		}
	    } while (VidModeGetNextModeline(stuff->screen, &mode, &dotClock));
	}
	if (!found)
	    return BadValue;
    }


    mode = VidModeCreateMode();
    if (mode == NULL)
	return BadValue;

    VidModeSetModeValue(mode, VIDMODE_CLOCK, stuff->dotclock);
    VidModeSetModeValue(mode, VIDMODE_H_DISPLAY, stuff->hdisplay);
    VidModeSetModeValue(mode, VIDMODE_H_SYNCSTART, stuff->hsyncstart); 
    VidModeSetModeValue(mode, VIDMODE_H_SYNCEND, stuff->hsyncend);
    VidModeSetModeValue(mode, VIDMODE_H_TOTAL, stuff->htotal);
    VidModeSetModeValue(mode, VIDMODE_H_SKEW, stuff->hskew);
    VidModeSetModeValue(mode, VIDMODE_V_DISPLAY, stuff->vdisplay);
    VidModeSetModeValue(mode, VIDMODE_V_SYNCSTART, stuff->vsyncstart); 
    VidModeSetModeValue(mode, VIDMODE_V_SYNCEND, stuff->vsyncend);
    VidModeSetModeValue(mode, VIDMODE_V_TOTAL, stuff->vtotal);
    VidModeSetModeValue(mode, VIDMODE_FLAGS, stuff->flags);

    if (stuff->privsize)
	ErrorF("AddModeLine - Privates in request have been ignored\n");

    /* Check that the mode is consistent with the monitor specs */
    switch (VidModeCheckModeForMonitor(stuff->screen, mode)) {
    	case MODE_OK:
	    break;
	case MODE_HSYNC:
	case MODE_H_ILLEGAL:
	    xfree(mode);
	    return VidModeErrorBase + XF86VidModeBadHTimings;
	case MODE_VSYNC:
	case MODE_V_ILLEGAL:
	    xfree(mode);
	    return VidModeErrorBase + XF86VidModeBadVTimings;
	default:
	    xfree(mode);
	    return VidModeErrorBase + XF86VidModeModeUnsuitable;
    }

    /* Check that the driver is happy with the mode */
    if (VidModeCheckModeForDriver(stuff->screen, mode) != MODE_OK) {
	xfree(mode);
	return VidModeErrorBase + XF86VidModeModeUnsuitable;
    }

    VidModeSetCrtcForMode(stuff->screen, mode);
    
    VidModeAddModeline(stuff->screen, mode);
    
    if (xf86GetVerbosity() > DEFAULT_XF86VIDMODE_VERBOSITY)
	ErrorF("AddModeLine - Succeeded\n");
    return client->noClientException;
}

static int
ProcXF86VidModeDeleteModeLine(ClientPtr client)
{
    REQUEST(xXF86VidModeDeleteModeLineReq);
    xXF86OldVidModeDeleteModeLineReq *oldstuff =
		(xXF86OldVidModeDeleteModeLineReq *)client->requestBuffer;
    xXF86VidModeDeleteModeLineReq newstuff;
    pointer mode;
    int len, dotClock;
    int ver;

    DEBUG_P("XF86VidModeDeleteModeline");

    ver = ClientMajorVersion(client);
    if (ver < 2) {
	/* convert from old format */
	stuff = &newstuff;
	stuff->length = oldstuff->length;
	stuff->screen = oldstuff->screen;
	stuff->dotclock = oldstuff->dotclock;
	stuff->hdisplay = oldstuff->hdisplay;
	stuff->hsyncstart = oldstuff->hsyncstart;
	stuff->hsyncend = oldstuff->hsyncend;
	stuff->htotal = oldstuff->htotal;
	stuff->hskew = 0;
	stuff->vdisplay = oldstuff->vdisplay;
	stuff->vsyncstart = oldstuff->vsyncstart;
	stuff->vsyncend = oldstuff->vsyncend;
	stuff->vtotal = oldstuff->vtotal;
	stuff->flags = oldstuff->flags;
	stuff->privsize = oldstuff->privsize;
    }
    if (xf86GetVerbosity() > DEFAULT_XF86VIDMODE_VERBOSITY) {
	ErrorF("DeleteModeLine - scrn: %d clock: %ld\n",
		(int)stuff->screen, (unsigned long)stuff->dotclock);
	ErrorF("                 hdsp: %d hbeg: %d hend: %d httl: %d\n",
		stuff->hdisplay, stuff->hsyncstart,
		stuff->hsyncend, stuff->htotal);
	ErrorF("                 vdsp: %d vbeg: %d vend: %d vttl: %d flags: %ld\n",
		stuff->vdisplay, stuff->vsyncstart, stuff->vsyncend,
		stuff->vtotal, (unsigned long)stuff->flags);
    }

    if (ver < 2) {
	REQUEST_AT_LEAST_SIZE(xXF86OldVidModeDeleteModeLineReq);
	len = client->req_len - (sizeof(xXF86OldVidModeDeleteModeLineReq) >> 2);
    } else {
	REQUEST_AT_LEAST_SIZE(xXF86VidModeDeleteModeLineReq);
	len = client->req_len - (sizeof(xXF86VidModeDeleteModeLineReq) >> 2);
    }
    if (len != stuff->privsize) {
	if (xf86GetVerbosity() > DEFAULT_XF86VIDMODE_VERBOSITY) {
	    ErrorF("req_len = %ld, sizeof(Req) = %d, privsize = %ld, "
		   "len = %d, length = %d\n",
		    (unsigned long)client->req_len,
		    (int)sizeof(xXF86VidModeDeleteModeLineReq)>>2,
		    (unsigned long)stuff->privsize, len, stuff->length);
	}
	return BadLength;
    }

    if(stuff->screen >= screenInfo.numScreens)
        return BadValue;

    if (!VidModeGetCurrentModeline(stuff->screen, &mode, &dotClock))
	return BadValue;

    if (xf86GetVerbosity() > DEFAULT_XF86VIDMODE_VERBOSITY) {
	ErrorF("Checking against clock: %d (%d)\n",
		VidModeGetModeValue(mode, VIDMODE_CLOCK), dotClock);
	ErrorF("                 hdsp: %d hbeg: %d hend: %d httl: %d\n",
	       VidModeGetModeValue(mode, VIDMODE_H_DISPLAY),
	       VidModeGetModeValue(mode, VIDMODE_H_SYNCSTART),
	       VidModeGetModeValue(mode, VIDMODE_H_SYNCEND),
	       VidModeGetModeValue(mode, VIDMODE_H_TOTAL));
	ErrorF("                 vdsp: %d vbeg: %d vend: %d vttl: %d flags: %d\n",
	       VidModeGetModeValue(mode, VIDMODE_V_DISPLAY),
	       VidModeGetModeValue(mode, VIDMODE_V_SYNCSTART),
	       VidModeGetModeValue(mode, VIDMODE_V_SYNCEND),
	       VidModeGetModeValue(mode, VIDMODE_V_TOTAL),
	       VidModeGetModeValue(mode, VIDMODE_FLAGS));
    }
    if ((VidModeGetDotClock(stuff->screen, stuff->dotclock) == dotClock) &&
	    MODEMATCH(mode, stuff))
	return BadValue;

    if (!VidModeGetFirstModeline(stuff->screen, &mode, &dotClock))
	return BadValue;

     do {
	if (xf86GetVerbosity() > DEFAULT_XF86VIDMODE_VERBOSITY) {
	    ErrorF("Checking against clock: %d (%d)\n",
		 VidModeGetModeValue(mode, VIDMODE_CLOCK), dotClock);
	    ErrorF("                 hdsp: %d hbeg: %d hend: %d httl: %d\n",
		 VidModeGetModeValue(mode, VIDMODE_H_DISPLAY),
		 VidModeGetModeValue(mode, VIDMODE_H_SYNCSTART),
		 VidModeGetModeValue(mode, VIDMODE_H_SYNCEND),
		 VidModeGetModeValue(mode, VIDMODE_H_TOTAL));
	    ErrorF("                 vdsp: %d vbeg: %d vend: %d vttl: %d flags: %d\n",
		 VidModeGetModeValue(mode, VIDMODE_V_DISPLAY),
		 VidModeGetModeValue(mode, VIDMODE_V_SYNCSTART),
		 VidModeGetModeValue(mode, VIDMODE_V_SYNCEND),
		 VidModeGetModeValue(mode, VIDMODE_V_TOTAL),
		 VidModeGetModeValue(mode, VIDMODE_FLAGS));
	}
	if ((VidModeGetDotClock(stuff->screen, stuff->dotclock) == dotClock) &&
		MODEMATCH(mode, stuff)) {
	    VidModeDeleteModeline(stuff->screen, mode);
	    if (xf86GetVerbosity() > DEFAULT_XF86VIDMODE_VERBOSITY)
		ErrorF("DeleteModeLine - Succeeded\n");
	    return(client->noClientException);
	}
    } while (VidModeGetNextModeline(stuff->screen, &mode, &dotClock));

    return BadValue;
}

static int
ProcXF86VidModeModModeLine(ClientPtr client)
{
    REQUEST(xXF86VidModeModModeLineReq);
    xXF86OldVidModeModModeLineReq *oldstuff =
			(xXF86OldVidModeModModeLineReq *)client->requestBuffer;
    xXF86VidModeModModeLineReq newstuff;
    pointer mode, modetmp;
    int len, dotClock;
    int ver;

    DEBUG_P("XF86VidModeModModeline");

    ver = ClientMajorVersion(client);
    if (ver < 2 ) {
	/* convert from old format */
	stuff = &newstuff;
	stuff->length = oldstuff->length;
	stuff->screen = oldstuff->screen;
	stuff->hdisplay = oldstuff->hdisplay;
	stuff->hsyncstart = oldstuff->hsyncstart;
	stuff->hsyncend = oldstuff->hsyncend;
	stuff->htotal = oldstuff->htotal;
	stuff->hskew = 0;
	stuff->vdisplay = oldstuff->vdisplay;
	stuff->vsyncstart = oldstuff->vsyncstart;
	stuff->vsyncend = oldstuff->vsyncend;
	stuff->vtotal = oldstuff->vtotal;
	stuff->flags = oldstuff->flags;
	stuff->privsize = oldstuff->privsize;
    }
    if (xf86GetVerbosity() > DEFAULT_XF86VIDMODE_VERBOSITY) {
	ErrorF("ModModeLine - scrn: %d hdsp: %d hbeg: %d hend: %d httl: %d\n",
		(int)stuff->screen, stuff->hdisplay, stuff->hsyncstart,
		stuff->hsyncend, stuff->htotal);
	ErrorF("              vdsp: %d vbeg: %d vend: %d vttl: %d flags: %ld\n",
		stuff->vdisplay, stuff->vsyncstart, stuff->vsyncend,
		stuff->vtotal, (unsigned long)stuff->flags);
    }

    if (ver < 2) {
	REQUEST_AT_LEAST_SIZE(xXF86OldVidModeModModeLineReq);
	len = client->req_len - (sizeof(xXF86OldVidModeModModeLineReq) >> 2);
    } else {
	REQUEST_AT_LEAST_SIZE(xXF86VidModeModModeLineReq);
	len = client->req_len - (sizeof(xXF86VidModeModModeLineReq) >> 2);
    }
    if (len != stuff->privsize)
	return BadLength;

    if (stuff->hsyncstart < stuff->hdisplay   ||
	stuff->hsyncend   < stuff->hsyncstart ||
	stuff->htotal     < stuff->hsyncend   ||
	stuff->vsyncstart < stuff->vdisplay   ||
	stuff->vsyncend   < stuff->vsyncstart ||
	stuff->vtotal     < stuff->vsyncend)
	return BadValue;

    if(stuff->screen >= screenInfo.numScreens)
        return BadValue;

    if (!VidModeGetCurrentModeline(stuff->screen, &mode, &dotClock))
	return BadValue;

    modetmp = VidModeCreateMode();
    VidModeCopyMode(mode, modetmp);

    VidModeSetModeValue(modetmp, VIDMODE_H_DISPLAY, stuff->hdisplay);
    VidModeSetModeValue(modetmp, VIDMODE_H_SYNCSTART, stuff->hsyncstart); 
    VidModeSetModeValue(modetmp, VIDMODE_H_SYNCEND, stuff->hsyncend);
    VidModeSetModeValue(modetmp, VIDMODE_H_TOTAL, stuff->htotal);
    VidModeSetModeValue(modetmp, VIDMODE_H_SKEW, stuff->hskew);
    VidModeSetModeValue(modetmp, VIDMODE_V_DISPLAY, stuff->vdisplay);
    VidModeSetModeValue(modetmp, VIDMODE_V_SYNCSTART, stuff->vsyncstart); 
    VidModeSetModeValue(modetmp, VIDMODE_V_SYNCEND, stuff->vsyncend);
    VidModeSetModeValue(modetmp, VIDMODE_V_TOTAL, stuff->vtotal);
    VidModeSetModeValue(modetmp, VIDMODE_FLAGS, stuff->flags);

    if (stuff->privsize)
	ErrorF("ModModeLine - Privates in request have been ignored\n");

    /* Check that the mode is consistent with the monitor specs */
    switch (VidModeCheckModeForMonitor(stuff->screen, modetmp)) {
    	case MODE_OK:
	    break;
	case MODE_HSYNC:
	case MODE_H_ILLEGAL:
	    xfree(modetmp);
	    return VidModeErrorBase + XF86VidModeBadHTimings;
	case MODE_VSYNC:
	case MODE_V_ILLEGAL:
	    xfree(modetmp);
	    return VidModeErrorBase + XF86VidModeBadVTimings;
	default:
	    xfree(modetmp);
	    return VidModeErrorBase + XF86VidModeModeUnsuitable;
    }

    /* Check that the driver is happy with the mode */
    if (VidModeCheckModeForDriver(stuff->screen, modetmp) != MODE_OK) {
	xfree(modetmp);
	return VidModeErrorBase + XF86VidModeModeUnsuitable;
    }
    xfree(modetmp);

    VidModeSetModeValue(mode, VIDMODE_H_DISPLAY, stuff->hdisplay);
    VidModeSetModeValue(mode, VIDMODE_H_SYNCSTART, stuff->hsyncstart); 
    VidModeSetModeValue(mode, VIDMODE_H_SYNCEND, stuff->hsyncend);
    VidModeSetModeValue(mode, VIDMODE_H_TOTAL, stuff->htotal);
    VidModeSetModeValue(mode, VIDMODE_H_SKEW, stuff->hskew);
    VidModeSetModeValue(mode, VIDMODE_V_DISPLAY, stuff->vdisplay);
    VidModeSetModeValue(mode, VIDMODE_V_SYNCSTART, stuff->vsyncstart); 
    VidModeSetModeValue(mode, VIDMODE_V_SYNCEND, stuff->vsyncend);
    VidModeSetModeValue(mode, VIDMODE_V_TOTAL, stuff->vtotal);
    VidModeSetModeValue(mode, VIDMODE_FLAGS, stuff->flags);

    VidModeSetCrtcForMode(stuff->screen, mode);
    VidModeSwitchMode(stuff->screen, mode);

    if (xf86GetVerbosity() > DEFAULT_XF86VIDMODE_VERBOSITY)
	ErrorF("ModModeLine - Succeeded\n");
    return(client->noClientException);
}

static int
ProcXF86VidModeValidateModeLine(ClientPtr client)
{
    REQUEST(xXF86VidModeValidateModeLineReq);
    xXF86OldVidModeValidateModeLineReq *oldstuff =
		(xXF86OldVidModeValidateModeLineReq *)client->requestBuffer;
    xXF86VidModeValidateModeLineReq newstuff;
    xXF86VidModeValidateModeLineReply rep;
    pointer mode, modetmp = NULL;
    int len, status, dotClock;
    int ver;

    DEBUG_P("XF86VidModeValidateModeline");

    ver = ClientMajorVersion(client);
    if (ver < 2) {
	/* convert from old format */
	stuff = &newstuff;
	stuff->length = oldstuff->length;
	stuff->screen = oldstuff->screen;
	stuff->dotclock = oldstuff->dotclock;
	stuff->hdisplay = oldstuff->hdisplay;
	stuff->hsyncstart = oldstuff->hsyncstart;
	stuff->hsyncend = oldstuff->hsyncend;
	stuff->htotal = oldstuff->htotal;
	stuff->hskew = 0;
	stuff->vdisplay = oldstuff->vdisplay;
	stuff->vsyncstart = oldstuff->vsyncstart;
	stuff->vsyncend = oldstuff->vsyncend;
	stuff->vtotal = oldstuff->vtotal;
	stuff->flags = oldstuff->flags;
	stuff->privsize = oldstuff->privsize;
    }
    if (xf86GetVerbosity() > DEFAULT_XF86VIDMODE_VERBOSITY) {
	ErrorF("ValidateModeLine - scrn: %d clock: %ld\n",
		(int)stuff->screen, (unsigned long)stuff->dotclock);
	ErrorF("                   hdsp: %d hbeg: %d hend: %d httl: %d\n",
		stuff->hdisplay, stuff->hsyncstart,
		stuff->hsyncend, stuff->htotal);
	ErrorF("                   vdsp: %d vbeg: %d vend: %d vttl: %d flags: %ld\n",
		stuff->vdisplay, stuff->vsyncstart, stuff->vsyncend,
		stuff->vtotal, (unsigned long)stuff->flags);
    }

    if (ver < 2) {
	REQUEST_AT_LEAST_SIZE(xXF86OldVidModeValidateModeLineReq);
	len = client->req_len -
			(sizeof(xXF86OldVidModeValidateModeLineReq) >> 2);
    } else {
	REQUEST_AT_LEAST_SIZE(xXF86VidModeValidateModeLineReq);
	len = client->req_len - (sizeof(xXF86VidModeValidateModeLineReq) >> 2);
    }
    if (len != stuff->privsize)
	return BadLength;

    if(stuff->screen >= screenInfo.numScreens)
        return BadValue;

    status = MODE_OK;

    if (stuff->hsyncstart < stuff->hdisplay   ||
	stuff->hsyncend   < stuff->hsyncstart ||
	stuff->htotal     < stuff->hsyncend   ||
	stuff->vsyncstart < stuff->vdisplay   ||
	stuff->vsyncend   < stuff->vsyncstart ||
	stuff->vtotal     < stuff->vsyncend)
    {
	status = MODE_BAD;
	goto status_reply;
    }

    if (!VidModeGetCurrentModeline(stuff->screen, &mode, &dotClock))
	return BadValue;

    modetmp = VidModeCreateMode();
    VidModeCopyMode(mode, modetmp);

    VidModeSetModeValue(modetmp, VIDMODE_H_DISPLAY, stuff->hdisplay);
    VidModeSetModeValue(modetmp, VIDMODE_H_SYNCSTART, stuff->hsyncstart); 
    VidModeSetModeValue(modetmp, VIDMODE_H_SYNCEND, stuff->hsyncend);
    VidModeSetModeValue(modetmp, VIDMODE_H_TOTAL, stuff->htotal);
    VidModeSetModeValue(modetmp, VIDMODE_H_SKEW, stuff->hskew);
    VidModeSetModeValue(modetmp, VIDMODE_V_DISPLAY, stuff->vdisplay);
    VidModeSetModeValue(modetmp, VIDMODE_V_SYNCSTART, stuff->vsyncstart); 
    VidModeSetModeValue(modetmp, VIDMODE_V_SYNCEND, stuff->vsyncend);
    VidModeSetModeValue(modetmp, VIDMODE_V_TOTAL, stuff->vtotal);
    VidModeSetModeValue(modetmp, VIDMODE_FLAGS, stuff->flags);
    if (stuff->privsize)
	ErrorF("ValidateModeLine - Privates in request have been ignored\n");

    /* Check that the mode is consistent with the monitor specs */
    if ((status = VidModeCheckModeForMonitor(stuff->screen, modetmp)) != MODE_OK)
	goto status_reply;

    /* Check that the driver is happy with the mode */
    status = VidModeCheckModeForDriver(stuff->screen, modetmp);

status_reply:
    if(modetmp)
      xfree(modetmp);

    rep.type = X_Reply;
    rep.length = (SIZEOF(xXF86VidModeValidateModeLineReply)
   			 - SIZEOF(xGenericReply)) >> 2;
    rep.sequenceNumber = client->sequence;
    rep.status = status;
    if (client->swapped) {
        register int n;
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swapl(&rep.status, n);
    }
    WriteToClient(client, sizeof(xXF86VidModeValidateModeLineReply), (char *)&rep);
    if (xf86GetVerbosity() > DEFAULT_XF86VIDMODE_VERBOSITY)
	ErrorF("ValidateModeLine - Succeeded (status = %d)\n", status);
    return(client->noClientException);
}

static int
ProcXF86VidModeSwitchMode(ClientPtr client)
{
    REQUEST(xXF86VidModeSwitchModeReq);

    DEBUG_P("XF86VidModeSwitchMode");

    REQUEST_SIZE_MATCH(xXF86VidModeSwitchModeReq);

    if(stuff->screen >= screenInfo.numScreens)
        return BadValue;

    VidModeZoomViewport(stuff->screen, (short)stuff->zoom);

    return (client->noClientException);
}

static int
ProcXF86VidModeSwitchToMode(ClientPtr client)
{
    REQUEST(xXF86VidModeSwitchToModeReq);
    xXF86OldVidModeSwitchToModeReq *oldstuff =
		(xXF86OldVidModeSwitchToModeReq *)client->requestBuffer;
    xXF86VidModeSwitchToModeReq newstuff;
    pointer mode;
    int len, dotClock;
    int ver;

    DEBUG_P("XF86VidModeSwitchToMode");

    ver = ClientMajorVersion(client);
    if (ver < 2) {
	/* convert from old format */
	stuff = &newstuff;
	stuff->length = oldstuff->length;
	stuff->screen = oldstuff->screen;
	stuff->dotclock = oldstuff->dotclock;
	stuff->hdisplay = oldstuff->hdisplay;
	stuff->hsyncstart = oldstuff->hsyncstart;
	stuff->hsyncend = oldstuff->hsyncend;
	stuff->htotal = oldstuff->htotal;
	stuff->hskew = 0;
	stuff->vdisplay = oldstuff->vdisplay;
	stuff->vsyncstart = oldstuff->vsyncstart;
	stuff->vsyncend = oldstuff->vsyncend;
	stuff->vtotal = oldstuff->vtotal;
	stuff->flags = oldstuff->flags;
	stuff->privsize = oldstuff->privsize;
    }
    if (xf86GetVerbosity() > DEFAULT_XF86VIDMODE_VERBOSITY) {
	ErrorF("SwitchToMode - scrn: %d clock: %ld\n",
		(int)stuff->screen, (unsigned long)stuff->dotclock);
	ErrorF("               hdsp: %d hbeg: %d hend: %d httl: %d\n",
		stuff->hdisplay, stuff->hsyncstart,
		stuff->hsyncend, stuff->htotal);
	ErrorF("               vdsp: %d vbeg: %d vend: %d vttl: %d flags: %ld\n",
		stuff->vdisplay, stuff->vsyncstart, stuff->vsyncend,
		stuff->vtotal, (unsigned long)stuff->flags);
    }

    if (ver < 2) {
	REQUEST_AT_LEAST_SIZE(xXF86OldVidModeSwitchToModeReq);
	len = client->req_len - (sizeof(xXF86OldVidModeSwitchToModeReq) >> 2);
    } else {
	REQUEST_AT_LEAST_SIZE(xXF86VidModeSwitchToModeReq);
	len = client->req_len - (sizeof(xXF86VidModeSwitchToModeReq) >> 2);
    }
    if (len != stuff->privsize)
	return BadLength;

    if(stuff->screen >= screenInfo.numScreens)
        return BadValue;

    if (!VidModeGetCurrentModeline(stuff->screen, &mode, &dotClock))
	return BadValue;

    if ((VidModeGetDotClock(stuff->screen, stuff->dotclock) == dotClock)
	    && MODEMATCH(mode, stuff))
	return (client->noClientException);

    if (!VidModeGetFirstModeline(stuff->screen, &mode, &dotClock))
	return BadValue;

    do {
	if (xf86GetVerbosity() > DEFAULT_XF86VIDMODE_VERBOSITY) {
	    ErrorF("Checking against clock: %d (%d)\n",
		 VidModeGetModeValue(mode, VIDMODE_CLOCK), dotClock);
	    ErrorF("                 hdsp: %d hbeg: %d hend: %d httl: %d\n",
		 VidModeGetModeValue(mode, VIDMODE_H_DISPLAY),
		 VidModeGetModeValue(mode, VIDMODE_H_SYNCSTART),
		 VidModeGetModeValue(mode, VIDMODE_H_SYNCEND),
		 VidModeGetModeValue(mode, VIDMODE_H_TOTAL));
	    ErrorF("                 vdsp: %d vbeg: %d vend: %d vttl: %d flags: %d\n",
		 VidModeGetModeValue(mode, VIDMODE_V_DISPLAY),
		 VidModeGetModeValue(mode, VIDMODE_V_SYNCSTART),
		 VidModeGetModeValue(mode, VIDMODE_V_SYNCEND),
		 VidModeGetModeValue(mode, VIDMODE_V_TOTAL),
		 VidModeGetModeValue(mode, VIDMODE_FLAGS));
	}
	if ((VidModeGetDotClock(stuff->screen, stuff->dotclock) == dotClock) &&
		MODEMATCH(mode, stuff)) {

	    if (!VidModeSwitchMode(stuff->screen, mode))
		return BadValue;

	    if (xf86GetVerbosity() > DEFAULT_XF86VIDMODE_VERBOSITY)
		ErrorF("SwitchToMode - Succeeded\n");
	    return(client->noClientException);
	}
    } while (VidModeGetNextModeline(stuff->screen, &mode, &dotClock));

    return BadValue;
}

static int
ProcXF86VidModeLockModeSwitch(ClientPtr client)
{
    REQUEST(xXF86VidModeLockModeSwitchReq);

    REQUEST_SIZE_MATCH(xXF86VidModeLockModeSwitchReq);

    DEBUG_P("XF86VidModeLockModeSwitch");

    if(stuff->screen >= screenInfo.numScreens)
        return BadValue;

    if (!VidModeLockZoom(stuff->screen, (short)stuff->lock))
	return VidModeErrorBase + XF86VidModeZoomLocked;

    return (client->noClientException);
}

static int
ProcXF86VidModeGetMonitor(ClientPtr client)
{
    REQUEST(xXF86VidModeGetMonitorReq);
    xXF86VidModeGetMonitorReply rep;
    register int n;
    CARD32 *hsyncdata, *vsyncdata;
    int i, nHsync, nVrefresh;
    pointer monitor;
    
    DEBUG_P("XF86VidModeGetMonitor");

    REQUEST_SIZE_MATCH(xXF86VidModeGetMonitorReq);

    if(stuff->screen >= screenInfo.numScreens)
        return BadValue;

    if (!VidModeGetMonitor(stuff->screen, &monitor))
	return BadValue;

    nHsync = VidModeGetMonitorValue(monitor, VIDMODE_MON_NHSYNC, 0).i;
    nVrefresh = VidModeGetMonitorValue(monitor, VIDMODE_MON_NVREFRESH, 0).i;
    
    rep.type = X_Reply;
    if ((char *)(VidModeGetMonitorValue(monitor, VIDMODE_MON_VENDOR, 0)).ptr)
	rep.vendorLength = strlen((char *)(VidModeGetMonitorValue(monitor,
				  VIDMODE_MON_VENDOR, 0)).ptr);
    else
	rep.vendorLength = 0;
    if ((char *)(VidModeGetMonitorValue(monitor, VIDMODE_MON_MODEL, 0)).ptr)
	rep.modelLength = strlen((char *)(VidModeGetMonitorValue(monitor,
				  VIDMODE_MON_MODEL, 0)).ptr);
    else
	rep.modelLength = 0;
    rep.length = (SIZEOF(xXF86VidModeGetMonitorReply) - SIZEOF(xGenericReply) +
		  (nHsync + nVrefresh) * sizeof(CARD32) +
	          ((rep.vendorLength + 3) & ~3) +
		  ((rep.modelLength + 3) & ~3)) >> 2;
    rep.sequenceNumber = client->sequence;
    rep.nhsync = nHsync;
    rep.nvsync = nVrefresh;
    hsyncdata = ALLOCATE_LOCAL(nHsync * sizeof(CARD32));
    if (!hsyncdata) {
	return BadAlloc;
    }

    vsyncdata = ALLOCATE_LOCAL(nVrefresh * sizeof(CARD32));
    if (!vsyncdata) {
	DEALLOCATE_LOCAL(hsyncdata);
	return BadAlloc;
    }

    for (i = 0; i < nHsync; i++) {
	hsyncdata[i] = (unsigned short)(VidModeGetMonitorValue(monitor,
			     VIDMODE_MON_HSYNC_LO, i)).f |
		       (unsigned short)(VidModeGetMonitorValue(monitor,
			     VIDMODE_MON_HSYNC_HI, i)).f << 16;
    }
    for (i = 0; i < nVrefresh; i++) {
	vsyncdata[i] = (unsigned short)(VidModeGetMonitorValue(monitor,
			     VIDMODE_MON_VREFRESH_LO, i)).f |
		       (unsigned short)(VidModeGetMonitorValue(monitor,
			     VIDMODE_MON_VREFRESH_HI, i)).f << 16;
    }
    

    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    }
    WriteToClient(client, SIZEOF(xXF86VidModeGetMonitorReply), (char *)&rep);
    client->pSwapReplyFunc = (ReplySwapPtr) Swap32Write;
    WriteSwappedDataToClient(client, nHsync * sizeof(CARD32),
			     hsyncdata);
    WriteSwappedDataToClient(client, nVrefresh * sizeof(CARD32),
			     vsyncdata);
    if (rep.vendorLength)
	WriteToClient(client, rep.vendorLength, (char *)(VidModeGetMonitorValue(monitor, VIDMODE_MON_VENDOR, 0)).ptr);
    if (rep.modelLength)
	WriteToClient(client, rep.modelLength, (char *)(VidModeGetMonitorValue(monitor, VIDMODE_MON_MODEL, 0)).ptr);

    DEALLOCATE_LOCAL(hsyncdata);
    DEALLOCATE_LOCAL(vsyncdata);

    return (client->noClientException);
}

static int
ProcXF86VidModeGetViewPort(ClientPtr client)
{
    REQUEST(xXF86VidModeGetViewPortReq);
    xXF86VidModeGetViewPortReply rep;
    int x, y, n;

    DEBUG_P("XF86VidModeGetViewPort");

    REQUEST_SIZE_MATCH(xXF86VidModeGetViewPortReq);

    if(stuff->screen >= screenInfo.numScreens)
        return BadValue;

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    VidModeGetViewPort(stuff->screen, &x, &y);
    rep.x = x;
    rep.y = y;

    if (client->swapped) {
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swapl(&rep.x, n);
	swapl(&rep.y, n);
    }
    WriteToClient(client, SIZEOF(xXF86VidModeGetViewPortReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXF86VidModeSetViewPort(ClientPtr client)
{
    REQUEST(xXF86VidModeSetViewPortReq);

    DEBUG_P("XF86VidModeSetViewPort");

    REQUEST_SIZE_MATCH(xXF86VidModeSetViewPortReq);

    if(stuff->screen >= screenInfo.numScreens)
        return BadValue;

    if (!VidModeSetViewPort(stuff->screen, stuff->x, stuff->y))
	return BadValue;

    return (client->noClientException);
}

static int
ProcXF86VidModeGetDotClocks(ClientPtr client)
{
    REQUEST(xXF86VidModeGetDotClocksReq);
    xXF86VidModeGetDotClocksReply rep;
    register int n;
    int numClocks;
    CARD32 dotclock;
    int *Clocks = NULL;
    Bool ClockProg;

    DEBUG_P("XF86VidModeGetDotClocks");

    REQUEST_SIZE_MATCH(xXF86VidModeGetDotClocksReq);

    if(stuff->screen >= screenInfo.numScreens)
        return BadValue;

    numClocks = VidModeGetNumOfClocks(stuff->screen, &ClockProg);

    rep.type = X_Reply;
    rep.length = (SIZEOF(xXF86VidModeGetDotClocksReply)
		    - SIZEOF(xGenericReply) + numClocks) >> 2;
    rep.sequenceNumber = client->sequence;
    rep.clocks = numClocks;
    rep.maxclocks = MAXCLOCKS;
    rep.flags = 0;

    if (!ClockProg) {
	Clocks = ALLOCATE_LOCAL(numClocks * sizeof(int));
	if (!Clocks)
	    return BadValue;
	if (!VidModeGetClocks(stuff->screen, Clocks)) {
	    DEALLOCATE_LOCAL(Clocks);
	    return BadValue;
	}
    }

    if (ClockProg) {
    	rep.flags |= CLKFLAG_PROGRAMABLE;
    }
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swapl(&rep.clocks, n);
	swapl(&rep.maxclocks, n);
	swapl(&rep.flags, n);
    }
    WriteToClient(client, sizeof(xXF86VidModeGetDotClocksReply), (char *)&rep);
    if (!ClockProg) {
	for (n = 0; n < numClocks; n++) {
    	    dotclock = *Clocks++;
	    if (client->swapped) {
		WriteSwappedDataToClient(client, 4, (char *)&dotclock);
	    } else {
		WriteToClient(client, 4, (char *)&dotclock);
	    }
	}
    }

    DEALLOCATE_LOCAL(Clocks);
    return (client->noClientException);
}

static int
ProcXF86VidModeSetGamma(ClientPtr client)
{
    REQUEST(xXF86VidModeSetGammaReq);

    DEBUG_P("XF86VidModeSetGamma");

    REQUEST_SIZE_MATCH(xXF86VidModeSetGammaReq);

    if(stuff->screen >= screenInfo.numScreens)
        return BadValue;

    if (!VidModeSetGamma(stuff->screen, ((float)stuff->red)/10000.,
		((float)stuff->green)/10000., ((float)stuff->blue)/10000.))
	return BadValue;

    return (client->noClientException);
}

static int
ProcXF86VidModeGetGamma(ClientPtr client)
{
    REQUEST(xXF86VidModeGetGammaReq);
    xXF86VidModeGetGammaReply rep;
    register int n;
    float red, green, blue;

    DEBUG_P("XF86VidModeGetGamma");

    REQUEST_SIZE_MATCH(xXF86VidModeGetGammaReq);

    if(stuff->screen >= screenInfo.numScreens)
        return BadValue;

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    if (!VidModeGetGamma(stuff->screen, &red, &green, &blue))
	return BadValue;
    rep.red = (CARD32)(red * 10000.);
    rep.green = (CARD32)(green * 10000.);
    rep.blue = (CARD32)(blue * 10000.);
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swapl(&rep.red, n);
    	swapl(&rep.green, n);
    	swapl(&rep.blue, n);
    }
    WriteToClient(client, sizeof(xXF86VidModeGetGammaReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXF86VidModeSetGammaRamp(ClientPtr client)
{
    CARD16 *r, *g, *b;
    int length;
    REQUEST(xXF86VidModeSetGammaRampReq);

    if(stuff->screen >= screenInfo.numScreens)
	return BadValue;

    if(stuff->size != VidModeGetGammaRampSize(stuff->screen))
	return BadValue;

    length = (stuff->size + 1) & ~1;

    REQUEST_FIXED_SIZE(xXF86VidModeSetGammaRampReq, length * 6);

    r = (CARD16*)&stuff[1];
    g = r + length;
    b = g + length;

    if (!VidModeSetGammaRamp(stuff->screen, stuff->size, r, g, b))
        return BadValue;

    return (client->noClientException);
}

static int
ProcXF86VidModeGetGammaRamp(ClientPtr client)
{
    CARD16 *ramp = NULL;
    int n, length, i;
    xXF86VidModeGetGammaRampReply rep;
    REQUEST(xXF86VidModeGetGammaRampReq);

    if(stuff->screen >= screenInfo.numScreens)
        return BadValue;

    if(stuff->size != VidModeGetGammaRampSize(stuff->screen))
        return BadValue;

    REQUEST_SIZE_MATCH(xXF86VidModeGetGammaRampReq);

    length = (stuff->size + 1) & ~1;

    if(stuff->size) {
        if(!(ramp = xalloc(length * 3 * sizeof(CARD16))))
	    return BadAlloc;
   
        if (!VidModeGetGammaRamp(stuff->screen, stuff->size, 
		ramp, ramp + length, ramp + (length * 2)))
            return BadValue;
    }

    rep.type = X_Reply;
    rep.length = (length >> 1) * 3;
    rep.sequenceNumber = client->sequence;
    rep.size = stuff->size;
    if(client->swapped) {
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swaps(&rep.size, n);
	for(i = 0; i < length * 3; i++)
	    swaps(&ramp[i],n);
    }
    WriteToClient(client, sizeof(xXF86VidModeGetGammaRampReply), (char *)&rep);

    if(stuff->size) {
	WriteToClient(client, rep.length << 2, (char*)ramp);
        xfree(ramp);
    }

    return (client->noClientException);
}


static int
ProcXF86VidModeGetGammaRampSize(ClientPtr client)
{
    xXF86VidModeGetGammaRampSizeReply rep;
    int n;
    REQUEST(xXF86VidModeGetGammaRampSizeReq);

    if(stuff->screen >= screenInfo.numScreens)
        return BadValue;

    REQUEST_SIZE_MATCH(xXF86VidModeGetGammaRampSizeReq);

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.size = VidModeGetGammaRampSize(stuff->screen); 
    if(client->swapped) {
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
        swaps(&rep.size, n);
    }
    WriteToClient(client,sizeof(xXF86VidModeGetGammaRampSizeReply),(char*)&rep);

    return (client->noClientException);
}

static int
ProcXF86VidModeGetPermissions(ClientPtr client)
{
    xXF86VidModeGetPermissionsReply rep;
    int n;
    REQUEST(xXF86VidModeGetPermissionsReq);

    if(stuff->screen >= screenInfo.numScreens)
        return BadValue;

    REQUEST_SIZE_MATCH(xXF86VidModeGetPermissionsReq);

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.permissions = XF86VM_READ_PERMISSION;
    if (xf86GetVidModeEnabled() &&
	(xf86GetVidModeAllowNonLocal() || LocalClient (client))) {
	rep.permissions |= XF86VM_WRITE_PERMISSION;
    }
    if(client->swapped) {
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
        swapl(&rep.permissions, n);
    }
    WriteToClient(client,sizeof(xXF86VidModeGetPermissionsReply),(char*)&rep);

    return (client->noClientException);
}


static int
ProcXF86VidModeSetClientVersion(ClientPtr client)
{
    REQUEST(xXF86VidModeSetClientVersionReq);

    VidModePrivPtr pPriv;

    DEBUG_P("XF86VidModeSetClientVersion");

    REQUEST_SIZE_MATCH(xXF86VidModeSetClientVersionReq);

    if ((pPriv = VMPRIV(client)) == NULL) {
	pPriv = xalloc(sizeof(VidModePrivRec));
	if (!pPriv)
	    return BadAlloc;
	VMPRIV(client) = pPriv;
    }
    pPriv->major = stuff->major;
    pPriv->minor = stuff->minor;
    
    return (client->noClientException);
}

static int
ProcXF86VidModeDispatch(ClientPtr client)
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_XF86VidModeQueryVersion:
	return ProcXF86VidModeQueryVersion(client);
    case X_XF86VidModeGetModeLine:
	return ProcXF86VidModeGetModeLine(client);
    case X_XF86VidModeGetMonitor:
	return ProcXF86VidModeGetMonitor(client);
    case X_XF86VidModeGetAllModeLines:
	return ProcXF86VidModeGetAllModeLines(client);
    case X_XF86VidModeValidateModeLine:
	return ProcXF86VidModeValidateModeLine(client);
    case X_XF86VidModeGetViewPort:
	return ProcXF86VidModeGetViewPort(client);
    case X_XF86VidModeGetDotClocks:
	return ProcXF86VidModeGetDotClocks(client);
    case X_XF86VidModeSetClientVersion:
	return ProcXF86VidModeSetClientVersion(client);
    case X_XF86VidModeGetGamma:
	return ProcXF86VidModeGetGamma(client);
    case X_XF86VidModeGetGammaRamp:
	return ProcXF86VidModeGetGammaRamp(client);
    case X_XF86VidModeGetGammaRampSize:
	return ProcXF86VidModeGetGammaRampSize(client);
    case X_XF86VidModeGetPermissions:
	return ProcXF86VidModeGetPermissions(client);
    default:
	if (!xf86GetVidModeEnabled())
	    return VidModeErrorBase + XF86VidModeExtensionDisabled;
	if (xf86GetVidModeAllowNonLocal() || LocalClient (client)) {
	    switch (stuff->data) {
	    case X_XF86VidModeAddModeLine:
		return ProcXF86VidModeAddModeLine(client);
	    case X_XF86VidModeDeleteModeLine:
		return ProcXF86VidModeDeleteModeLine(client);
	    case X_XF86VidModeModModeLine:
		return ProcXF86VidModeModModeLine(client);
	    case X_XF86VidModeSwitchMode:
		return ProcXF86VidModeSwitchMode(client);
	    case X_XF86VidModeSwitchToMode:
		return ProcXF86VidModeSwitchToMode(client);
	    case X_XF86VidModeLockModeSwitch:
		return ProcXF86VidModeLockModeSwitch(client);
	    case X_XF86VidModeSetViewPort:
		return ProcXF86VidModeSetViewPort(client);
	    case X_XF86VidModeSetGamma:
		return ProcXF86VidModeSetGamma(client);
	    case X_XF86VidModeSetGammaRamp:
		return ProcXF86VidModeSetGammaRamp(client);
	    default:
		return BadRequest;
	    }
	} else
	    return VidModeErrorBase + XF86VidModeClientNotLocal;
    }
}

static int
SProcXF86VidModeQueryVersion(ClientPtr client)
{
    register int n;
    REQUEST(xXF86VidModeQueryVersionReq);
    swaps(&stuff->length, n);
    return ProcXF86VidModeQueryVersion(client);
}

static int
SProcXF86VidModeGetModeLine(ClientPtr client)
{
    register int n;
    REQUEST(xXF86VidModeGetModeLineReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86VidModeGetModeLineReq);
    swaps(&stuff->screen, n);
    return ProcXF86VidModeGetModeLine(client);
}

static int
SProcXF86VidModeGetAllModeLines(ClientPtr client)
{
    register int n;
    REQUEST(xXF86VidModeGetAllModeLinesReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86VidModeGetAllModeLinesReq);
    swaps(&stuff->screen, n);
    return ProcXF86VidModeGetAllModeLines(client);
}

static int
SProcXF86VidModeAddModeLine(ClientPtr client)
{
    xXF86OldVidModeAddModeLineReq *oldstuff =
			(xXF86OldVidModeAddModeLineReq *)client->requestBuffer;
    int ver;
    register int n;
    
    REQUEST(xXF86VidModeAddModeLineReq);
    ver = ClientMajorVersion(client);
    if (ver < 2) {
	swaps(&oldstuff->length, n);
	REQUEST_AT_LEAST_SIZE(xXF86OldVidModeAddModeLineReq);
	swapl(&oldstuff->screen, n);
	swaps(&oldstuff->hdisplay, n);
	swaps(&oldstuff->hsyncstart, n);
	swaps(&oldstuff->hsyncend, n);
	swaps(&oldstuff->htotal, n);
	swaps(&oldstuff->vdisplay, n);
	swaps(&oldstuff->vsyncstart, n);
	swaps(&oldstuff->vsyncend, n);
	swaps(&oldstuff->vtotal, n);
	swapl(&oldstuff->flags, n);
	swapl(&oldstuff->privsize, n);
	SwapRestL(oldstuff);
    } else {
	swaps(&stuff->length, n);
	REQUEST_AT_LEAST_SIZE(xXF86VidModeAddModeLineReq);
	swapl(&stuff->screen, n);
	swaps(&stuff->hdisplay, n);
	swaps(&stuff->hsyncstart, n);
	swaps(&stuff->hsyncend, n);
	swaps(&stuff->htotal, n);
	swaps(&stuff->hskew, n);
	swaps(&stuff->vdisplay, n);
	swaps(&stuff->vsyncstart, n);
	swaps(&stuff->vsyncend, n);
	swaps(&stuff->vtotal, n);
	swapl(&stuff->flags, n);
	swapl(&stuff->privsize, n);
	SwapRestL(stuff);
    }
    return ProcXF86VidModeAddModeLine(client);
}

static int
SProcXF86VidModeDeleteModeLine(ClientPtr client)
{
    xXF86OldVidModeDeleteModeLineReq *oldstuff =
		(xXF86OldVidModeDeleteModeLineReq *)client->requestBuffer;
    int ver;
    register int n;

    REQUEST(xXF86VidModeDeleteModeLineReq);
    ver = ClientMajorVersion(client);
    if (ver < 2) {
	swaps(&oldstuff->length, n);
	REQUEST_AT_LEAST_SIZE(xXF86OldVidModeDeleteModeLineReq);
	swapl(&oldstuff->screen, n);
	swaps(&oldstuff->hdisplay, n);
	swaps(&oldstuff->hsyncstart, n);
	swaps(&oldstuff->hsyncend, n);
	swaps(&oldstuff->htotal, n);
	swaps(&oldstuff->vdisplay, n);
	swaps(&oldstuff->vsyncstart, n);
	swaps(&oldstuff->vsyncend, n);
	swaps(&oldstuff->vtotal, n);
	swapl(&oldstuff->flags, n);
	swapl(&oldstuff->privsize, n);
	SwapRestL(oldstuff);
    } else {
	swaps(&stuff->length, n);
	REQUEST_AT_LEAST_SIZE(xXF86VidModeDeleteModeLineReq);
	swapl(&stuff->screen, n);
	swaps(&stuff->hdisplay, n);
	swaps(&stuff->hsyncstart, n);
	swaps(&stuff->hsyncend, n);
	swaps(&stuff->htotal, n);
	swaps(&stuff->hskew, n);
	swaps(&stuff->vdisplay, n);
	swaps(&stuff->vsyncstart, n);
	swaps(&stuff->vsyncend, n);
	swaps(&stuff->vtotal, n);
	swapl(&stuff->flags, n);
	swapl(&stuff->privsize, n);
	SwapRestL(stuff);
    }
    return ProcXF86VidModeDeleteModeLine(client);
}

static int
SProcXF86VidModeModModeLine(ClientPtr client)
{
    xXF86OldVidModeModModeLineReq *oldstuff =
		(xXF86OldVidModeModModeLineReq *)client->requestBuffer;
    int ver;
    register int n;

    REQUEST(xXF86VidModeModModeLineReq);
    ver = ClientMajorVersion(client);
    if (ver < 2) {
	swaps(&oldstuff->length, n);
	REQUEST_AT_LEAST_SIZE(xXF86OldVidModeModModeLineReq);
	swapl(&oldstuff->screen, n);
	swaps(&oldstuff->hdisplay, n);
	swaps(&oldstuff->hsyncstart, n);
	swaps(&oldstuff->hsyncend, n);
	swaps(&oldstuff->htotal, n);
	swaps(&oldstuff->vdisplay, n);
	swaps(&oldstuff->vsyncstart, n);
	swaps(&oldstuff->vsyncend, n);
	swaps(&oldstuff->vtotal, n);
	swapl(&oldstuff->flags, n);
	swapl(&oldstuff->privsize, n);
	SwapRestL(oldstuff);
    } else {
	swaps(&stuff->length, n);
	REQUEST_AT_LEAST_SIZE(xXF86VidModeModModeLineReq);
	swapl(&stuff->screen, n);
	swaps(&stuff->hdisplay, n);
	swaps(&stuff->hsyncstart, n);
	swaps(&stuff->hsyncend, n);
	swaps(&stuff->htotal, n);
	swaps(&stuff->hskew, n);
	swaps(&stuff->vdisplay, n);
	swaps(&stuff->vsyncstart, n);
	swaps(&stuff->vsyncend, n);
	swaps(&stuff->vtotal, n);
	swapl(&stuff->flags, n);
	swapl(&stuff->privsize, n);
	SwapRestL(stuff);      
    }
    return ProcXF86VidModeModModeLine(client);
}

static int
SProcXF86VidModeValidateModeLine(ClientPtr client)
{
    xXF86OldVidModeValidateModeLineReq *oldstuff =
		(xXF86OldVidModeValidateModeLineReq *)client->requestBuffer;
    int ver;
    register int n;

    REQUEST(xXF86VidModeValidateModeLineReq);
    ver = ClientMajorVersion(client);
    if (ver < 2) {
	swaps(&oldstuff->length, n);
	REQUEST_AT_LEAST_SIZE(xXF86OldVidModeValidateModeLineReq);
	swapl(&oldstuff->screen, n);
	swaps(&oldstuff->hdisplay, n);
	swaps(&oldstuff->hsyncstart, n);
	swaps(&oldstuff->hsyncend, n);
	swaps(&oldstuff->htotal, n);
	swaps(&oldstuff->vdisplay, n);
	swaps(&oldstuff->vsyncstart, n);
	swaps(&oldstuff->vsyncend, n);
	swaps(&oldstuff->vtotal, n);
	swapl(&oldstuff->flags, n);
	swapl(&oldstuff->privsize, n);
	SwapRestL(oldstuff);
    } else {
	swaps(&stuff->length, n);
	REQUEST_AT_LEAST_SIZE(xXF86VidModeValidateModeLineReq);
	swapl(&stuff->screen, n);
	swaps(&stuff->hdisplay, n);
	swaps(&stuff->hsyncstart, n);
	swaps(&stuff->hsyncend, n);
	swaps(&stuff->htotal, n);
	swaps(&stuff->hskew, n);
	swaps(&stuff->vdisplay, n);
	swaps(&stuff->vsyncstart, n);
	swaps(&stuff->vsyncend, n);
	swaps(&stuff->vtotal, n);
	swapl(&stuff->flags, n);
	swapl(&stuff->privsize, n);
	SwapRestL(stuff);
    }
    return ProcXF86VidModeValidateModeLine(client);
}

static int
SProcXF86VidModeSwitchMode(ClientPtr client)
{
    register int n;
    REQUEST(xXF86VidModeSwitchModeReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86VidModeSwitchModeReq);
    swaps(&stuff->screen, n);
    swaps(&stuff->zoom, n);
    return ProcXF86VidModeSwitchMode(client);
}

static int
SProcXF86VidModeSwitchToMode(ClientPtr client)
{
    register int n;
    REQUEST(xXF86VidModeSwitchToModeReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86VidModeSwitchToModeReq);
    swaps(&stuff->screen, n);
    return ProcXF86VidModeSwitchToMode(client);
}

static int
SProcXF86VidModeLockModeSwitch(ClientPtr client)
{
    register int n;
    REQUEST(xXF86VidModeLockModeSwitchReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86VidModeLockModeSwitchReq);
    swaps(&stuff->screen, n);
    swaps(&stuff->lock, n);
    return ProcXF86VidModeLockModeSwitch(client);
}

static int
SProcXF86VidModeGetMonitor(ClientPtr client)
{
    register int n;
    REQUEST(xXF86VidModeGetMonitorReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86VidModeGetMonitorReq);
    swaps(&stuff->screen, n);
    return ProcXF86VidModeGetMonitor(client);
}

static int
SProcXF86VidModeGetViewPort(ClientPtr client)
{
    register int n;
    REQUEST(xXF86VidModeGetViewPortReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86VidModeGetViewPortReq);
    swaps(&stuff->screen, n);
    return ProcXF86VidModeGetViewPort(client);
}

static int
SProcXF86VidModeSetViewPort(ClientPtr client)
{
    register int n;
    REQUEST(xXF86VidModeSetViewPortReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86VidModeSetViewPortReq);
    swaps(&stuff->screen, n);
    swapl(&stuff->x, n);
    swapl(&stuff->y, n);
    return ProcXF86VidModeSetViewPort(client);
}

static int
SProcXF86VidModeGetDotClocks(ClientPtr client)
{
    register int n;
    REQUEST(xXF86VidModeGetDotClocksReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86VidModeGetDotClocksReq);
    swaps(&stuff->screen, n);
    return ProcXF86VidModeGetDotClocks(client);
}

static int
SProcXF86VidModeSetClientVersion(ClientPtr client)
{
    register int n;
    REQUEST(xXF86VidModeSetClientVersionReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86VidModeSetClientVersionReq);
    swaps(&stuff->major, n);
    swaps(&stuff->minor, n);
    return ProcXF86VidModeSetClientVersion(client);
}

static int
SProcXF86VidModeSetGamma(ClientPtr client)
{
    register int n;
    REQUEST(xXF86VidModeSetGammaReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86VidModeSetGammaReq);
    swaps(&stuff->screen, n);
    swapl(&stuff->red, n);
    swapl(&stuff->green, n);
    swapl(&stuff->blue, n);
    return ProcXF86VidModeSetGamma(client);
}

static int
SProcXF86VidModeGetGamma(ClientPtr client)
{
    register int n;
    REQUEST(xXF86VidModeGetGammaReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86VidModeGetGammaReq);
    swaps(&stuff->screen, n);
    return ProcXF86VidModeGetGamma(client);
}

static int
SProcXF86VidModeSetGammaRamp(ClientPtr client)
{
    CARD16 *ramp;
    int length, n;
    REQUEST(xXF86VidModeSetGammaRampReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xXF86VidModeSetGammaRampReq);
    swaps(&stuff->size, n);
    swaps(&stuff->screen, n);
    length = ((stuff->size + 1) & ~1) * 6;
    REQUEST_FIXED_SIZE(xXF86VidModeSetGammaRampReq, length);
    ramp = (CARD16*)&stuff[1];
    while(length--) {
	swaps(ramp, n);
	ramp++;
    }
    return ProcXF86VidModeSetGammaRamp(client);
}

static int
SProcXF86VidModeGetGammaRamp(ClientPtr client)
{
    int n;
    REQUEST(xXF86VidModeGetGammaRampReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86VidModeGetGammaRampReq);
    swaps(&stuff->size, n);
    swaps(&stuff->screen, n);
    return ProcXF86VidModeGetGammaRamp(client);
}

static int
SProcXF86VidModeGetGammaRampSize(ClientPtr client)
{   
    int n;
    REQUEST(xXF86VidModeGetGammaRampSizeReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86VidModeGetGammaRampSizeReq);
    swaps(&stuff->screen, n);
    return ProcXF86VidModeGetGammaRampSize(client);
}

static int
SProcXF86VidModeGetPermissions(ClientPtr client)
{   
    int n;
    REQUEST(xXF86VidModeGetPermissionsReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXF86VidModeGetPermissionsReq);
    swaps(&stuff->screen, n);
    return ProcXF86VidModeGetPermissions(client);
}


static int
SProcXF86VidModeDispatch(ClientPtr client)
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_XF86VidModeQueryVersion:
	return SProcXF86VidModeQueryVersion(client);
    case X_XF86VidModeGetModeLine:
	return SProcXF86VidModeGetModeLine(client);
    case X_XF86VidModeGetMonitor:
	return SProcXF86VidModeGetMonitor(client);
    case X_XF86VidModeGetAllModeLines:
	return SProcXF86VidModeGetAllModeLines(client);
    case X_XF86VidModeGetViewPort:
	return SProcXF86VidModeGetViewPort(client);
    case X_XF86VidModeValidateModeLine:
	return SProcXF86VidModeValidateModeLine(client);
    case X_XF86VidModeGetDotClocks:
	return SProcXF86VidModeGetDotClocks(client);
    case X_XF86VidModeSetClientVersion:
	return SProcXF86VidModeSetClientVersion(client);
    case X_XF86VidModeGetGamma:
	return SProcXF86VidModeGetGamma(client);
    case X_XF86VidModeGetGammaRamp:
	return SProcXF86VidModeGetGammaRamp(client);
    case X_XF86VidModeGetGammaRampSize:
	return SProcXF86VidModeGetGammaRampSize(client);
    case X_XF86VidModeGetPermissions:
	return SProcXF86VidModeGetPermissions(client);
    default:
	if (!xf86GetVidModeEnabled())
	    return VidModeErrorBase + XF86VidModeExtensionDisabled;
	if (xf86GetVidModeAllowNonLocal() || LocalClient(client)) {
	    switch (stuff->data) {
	    case X_XF86VidModeAddModeLine:
		return SProcXF86VidModeAddModeLine(client);
	    case X_XF86VidModeDeleteModeLine:
		return SProcXF86VidModeDeleteModeLine(client);
	    case X_XF86VidModeModModeLine:
		return SProcXF86VidModeModModeLine(client);
	    case X_XF86VidModeSwitchMode:
		return SProcXF86VidModeSwitchMode(client);
	    case X_XF86VidModeSwitchToMode:
		return SProcXF86VidModeSwitchToMode(client);
	    case X_XF86VidModeLockModeSwitch:
		return SProcXF86VidModeLockModeSwitch(client);
	    case X_XF86VidModeSetViewPort:
		return SProcXF86VidModeSetViewPort(client);
	    case X_XF86VidModeSetGamma:
		return SProcXF86VidModeSetGamma(client);
	    case X_XF86VidModeSetGammaRamp:
		return SProcXF86VidModeSetGammaRamp(client);
	    default:
		return BadRequest;
	    }
	} else
	    return VidModeErrorBase + XF86VidModeClientNotLocal;
    }
}
