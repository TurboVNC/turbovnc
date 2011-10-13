/* $XdotOrg: xc/programs/Xserver/Xext/saver.c,v 1.10 2005/07/03 08:53:36 daniels Exp $ */
/*
 * $XConsortium: saver.c,v 1.12 94/04/17 20:59:36 dpw Exp $
 *
Copyright (c) 1992  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/* $XFree86: xc/programs/Xserver/Xext/saver.c,v 3.7 2003/10/28 23:08:43 tsi Exp $ */

#define NEED_REPLIES
#define NEED_EVENTS
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "os.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "extnsionst.h"
#include "dixstruct.h"
#include "resource.h"
#include "opaque.h"
#include <X11/extensions/saverproto.h>
#include "gcstruct.h"
#include "cursorstr.h"
#include "colormapst.h"
#ifdef PANORAMIX
#include "panoramiX.h"
#include "panoramiXsrv.h"
#endif


#ifdef IN_MODULE
#include <xf86_ansic.h>
#else
#include <stdio.h>
#endif

#include "modinit.h"

#if 0
static unsigned char ScreenSaverReqCode = 0;
#endif
static int ScreenSaverEventBase = 0;

extern DISPATCH_PROC(ProcScreenSaverQueryInfo);
static DISPATCH_PROC(ProcScreenSaverDispatch);
static DISPATCH_PROC(ProcScreenSaverQueryVersion);
static DISPATCH_PROC(ProcScreenSaverSelectInput);
static DISPATCH_PROC(ProcScreenSaverSetAttributes);
static DISPATCH_PROC(ProcScreenSaverUnsetAttributes);
static DISPATCH_PROC(SProcScreenSaverDispatch);
static DISPATCH_PROC(SProcScreenSaverQueryInfo);
static DISPATCH_PROC(SProcScreenSaverQueryVersion);
static DISPATCH_PROC(SProcScreenSaverSelectInput);
static DISPATCH_PROC(SProcScreenSaverSetAttributes);
static DISPATCH_PROC(SProcScreenSaverUnsetAttributes);

static Bool ScreenSaverHandle (
	ScreenPtr /* pScreen */,
	int /* xstate */,
	Bool /* force */
	);

static Bool
CreateSaverWindow (
	ScreenPtr /* pScreen */
	);

static Bool
DestroySaverWindow (
	ScreenPtr /* pScreen */
	);

static void
UninstallSaverColormap (
	ScreenPtr /* pScreen */
	);

static void
CheckScreenPrivate (
	ScreenPtr /* pScreen */
	);

static void SScreenSaverNotifyEvent (
	xScreenSaverNotifyEvent * /* from */,
	xScreenSaverNotifyEvent * /* to */
	);

static void ScreenSaverResetProc (
	ExtensionEntry * /* extEntry */
	);

/*
 * each screen has a list of clients requesting
 * ScreenSaverNotify events.  Each client has a resource
 * for each screen it selects ScreenSaverNotify input for,
 * this resource is used to delete the ScreenSaverNotifyRec
 * entry from the per-screen queue.
 */

static RESTYPE EventType;   /* resource type for event masks */

typedef struct _ScreenSaverEvent *ScreenSaverEventPtr;

typedef struct _ScreenSaverEvent {
    ScreenSaverEventPtr	next;
    ClientPtr		client;
    ScreenPtr		screen;
    XID			resource;
    CARD32		mask;
} ScreenSaverEventRec;

static int ScreenSaverFreeEvents(
    pointer /* value */,
    XID /* id */
);

static Bool setEventMask (
    ScreenPtr /* pScreen */,
    ClientPtr /* client */,
    unsigned long /* mask */
);

static unsigned long getEventMask (
    ScreenPtr /* pScreen */,
    ClientPtr /* client */
);

/*
 * when a client sets the screen saver attributes, a resource is
 * kept to be freed when the client exits
 */

static RESTYPE AttrType;    /* resource type for attributes */

typedef struct _ScreenSaverAttr {
    ScreenPtr	    screen;
    ClientPtr	    client;
    XID		    resource;
    short	    x, y;
    unsigned short  width, height, borderWidth;
    unsigned char   class;
    unsigned char   depth;
    VisualID	    visual;
    CursorPtr	    pCursor;
    PixmapPtr	    pBackgroundPixmap;
    PixmapPtr	    pBorderPixmap;
    Colormap	    colormap;
    unsigned long   mask;		/* no pixmaps or cursors */
    unsigned long   *values;
} ScreenSaverAttrRec, *ScreenSaverAttrPtr;

static int ScreenSaverFreeAttr (
    pointer /* value */,
    XID /* id */
);

static void FreeAttrs (
    ScreenSaverAttrPtr	/* pAttr */
);

static void FreeScreenAttr (
    ScreenSaverAttrPtr	/* pAttr */
);

static void
SendScreenSaverNotify (
    ScreenPtr /* pScreen */,
    int /* state */,
    Bool /* forced */
);

typedef struct _ScreenSaverScreenPrivate {
    ScreenSaverEventPtr	    events;
    ScreenSaverAttrPtr	    attr;
    Bool		    hasWindow;
    Colormap		    installedMap;
} ScreenSaverScreenPrivateRec, *ScreenSaverScreenPrivatePtr;

static ScreenSaverScreenPrivatePtr
MakeScreenPrivate (
	ScreenPtr /* pScreen */
	);

static int ScreenPrivateIndex;

#define GetScreenPrivate(s) ((ScreenSaverScreenPrivatePtr)(s)->devPrivates[ScreenPrivateIndex].ptr)
#define SetScreenPrivate(s,v) ((s)->devPrivates[ScreenPrivateIndex].ptr = (pointer) v);
#define SetupScreen(s)	ScreenSaverScreenPrivatePtr pPriv = (s ? GetScreenPrivate(s) : NULL)

#define New(t)	((t *) xalloc (sizeof (t)))

/****************
 * ScreenSaverExtensionInit
 *
 * Called from InitExtensions in main() or from QueryExtension() if the
 * extension is dynamically loaded.
 *
 ****************/

void
ScreenSaverExtensionInit(INITARGS)
{
    ExtensionEntry *extEntry;
    int		    i;
    ScreenPtr	    pScreen;

    AttrType = CreateNewResourceType(ScreenSaverFreeAttr);
    EventType = CreateNewResourceType(ScreenSaverFreeEvents);
    ScreenPrivateIndex = AllocateScreenPrivateIndex ();
    for (i = 0; i < screenInfo.numScreens; i++)
    {
	pScreen = screenInfo.screens[i];
	SetScreenPrivate (pScreen, NULL);
    }
    if (AttrType && EventType && ScreenPrivateIndex != -1 &&
	(extEntry = AddExtension(ScreenSaverName, ScreenSaverNumberEvents, 0,
				 ProcScreenSaverDispatch, SProcScreenSaverDispatch,
				 ScreenSaverResetProc, StandardMinorOpcode)))
    {
#if 0
	ScreenSaverReqCode = (unsigned char)extEntry->base;
#endif
	ScreenSaverEventBase = extEntry->eventBase;
	EventSwapVector[ScreenSaverEventBase] = (EventSwapPtr) SScreenSaverNotifyEvent;
    }
}

/*ARGSUSED*/
static void
ScreenSaverResetProc (extEntry)
ExtensionEntry	*extEntry;
{
}

static void
CheckScreenPrivate (pScreen)
    ScreenPtr	pScreen;
{
    SetupScreen (pScreen);

    if (!pPriv)
	return;
    if (!pPriv->attr && !pPriv->events &&
	!pPriv->hasWindow && pPriv->installedMap == None)
    {
	xfree (pPriv);
	SetScreenPrivate (pScreen, NULL);
	savedScreenInfo[pScreen->myNum].ExternalScreenSaver = NULL;
    }
}

static ScreenSaverScreenPrivatePtr
MakeScreenPrivate (pScreen)
    ScreenPtr	pScreen;
{
    SetupScreen (pScreen);

    if (pPriv)
	return pPriv;
    pPriv = New (ScreenSaverScreenPrivateRec);
    if (!pPriv)
	return 0;
    pPriv->events = 0;
    pPriv->attr = 0;
    pPriv->hasWindow = FALSE;
    pPriv->installedMap = None;
    SetScreenPrivate (pScreen, pPriv);
    savedScreenInfo[pScreen->myNum].ExternalScreenSaver = ScreenSaverHandle;
    return pPriv;
}

static unsigned long
getEventMask (pScreen, client)
    ScreenPtr	pScreen;
    ClientPtr	client;
{
    SetupScreen(pScreen);
    ScreenSaverEventPtr	pEv;

    if (!pPriv)
	return 0;
    for (pEv = pPriv->events; pEv; pEv = pEv->next)
	if (pEv->client == client)
	    return pEv->mask;
    return 0;
}

static Bool
setEventMask (pScreen, client, mask)
    ScreenPtr	pScreen;
    ClientPtr	client;
    unsigned long   mask;
{
    SetupScreen(pScreen);
    ScreenSaverEventPtr	pEv, *pPrev;

    if (getEventMask (pScreen, client) == mask)
	return TRUE;
    if (!pPriv)
    {
	pPriv = MakeScreenPrivate (pScreen);
	if (!pPriv)
	    return FALSE;
    }
    for (pPrev = &pPriv->events; (pEv = *pPrev) != 0; pPrev = &pEv->next)
	if (pEv->client == client)
	    break;
    if (mask == 0)
    {
	FreeResource (pEv->resource, EventType);
	*pPrev = pEv->next;
	xfree (pEv);
	CheckScreenPrivate (pScreen);
    }
    else
    {
    	if (!pEv) 
    	{
	    pEv = New (ScreenSaverEventRec);
	    if (!pEv) 
	    {
		CheckScreenPrivate (pScreen);
	    	return FALSE;
	    }
    	    *pPrev = pEv;
    	    pEv->next = NULL;
    	    pEv->client = client;
    	    pEv->screen = pScreen;
    	    pEv->resource = FakeClientID (client->index);
	    if (!AddResource (pEv->resource, EventType, (pointer) pEv))
		return FALSE;
    	}
	pEv->mask = mask;
    }
    return TRUE;
}

static void
FreeAttrs (pAttr)
    ScreenSaverAttrPtr	pAttr;
{
    PixmapPtr	    pPixmap;
    CursorPtr	    pCursor;

    if ((pPixmap = pAttr->pBackgroundPixmap) != 0)
	(*pPixmap->drawable.pScreen->DestroyPixmap)(pPixmap);
    if ((pPixmap = pAttr->pBorderPixmap) != 0)
	(*pPixmap->drawable.pScreen->DestroyPixmap)(pPixmap);
    if ((pCursor = pAttr->pCursor) != 0)
	FreeCursor (pCursor, (Cursor) 0);
}

static void
FreeScreenAttr (pAttr)
    ScreenSaverAttrPtr	pAttr;
{
    FreeAttrs (pAttr);
    xfree (pAttr->values);
    xfree (pAttr);
}

static int
ScreenSaverFreeEvents (value, id)
    pointer value;
    XID id;
{
    ScreenSaverEventPtr	pOld = (ScreenSaverEventPtr)value;
    ScreenPtr pScreen = pOld->screen;
    SetupScreen (pScreen);
    ScreenSaverEventPtr	pEv, *pPrev;

    if (!pPriv)
	return TRUE;
    for (pPrev = &pPriv->events; (pEv = *pPrev) != 0; pPrev = &pEv->next)
	if (pEv == pOld)
	    break;
    if (!pEv)
	return TRUE;
    *pPrev = pEv->next;
    xfree (pEv);
    CheckScreenPrivate (pScreen);
    return TRUE;
}

static int
ScreenSaverFreeAttr (value, id)
    pointer value;
    XID id;
{
    ScreenSaverAttrPtr	pOldAttr = (ScreenSaverAttrPtr)value;
    ScreenPtr	pScreen = pOldAttr->screen;
    SetupScreen (pScreen);

    if (!pPriv)
	return TRUE;
    if (pPriv->attr != pOldAttr)
	return TRUE;
    FreeScreenAttr (pOldAttr);
    pPriv->attr = NULL;
    if (pPriv->hasWindow)
    {
	SaveScreens (SCREEN_SAVER_FORCER, ScreenSaverReset);
	SaveScreens (SCREEN_SAVER_FORCER, ScreenSaverActive);
    }
    CheckScreenPrivate (pScreen);
    return TRUE;
}

static void
SendScreenSaverNotify (pScreen, state, forced)
    ScreenPtr			pScreen;
    int	    state;
    Bool    forced;
{
    ScreenSaverScreenPrivatePtr	pPriv;
    ScreenSaverEventPtr		pEv;
    unsigned long		mask;
    xScreenSaverNotifyEvent	ev;
    ClientPtr			client;
    int				kind;

    UpdateCurrentTimeIf ();
    mask = ScreenSaverNotifyMask;
    if (state == ScreenSaverCycle)
	mask = ScreenSaverCycleMask;
    pScreen = screenInfo.screens[pScreen->myNum];
    pPriv = GetScreenPrivate(pScreen);
    if (!pPriv)
	return;
    if (pPriv->attr)
	kind = ScreenSaverExternal;
    else if (ScreenSaverBlanking != DontPreferBlanking)
	kind = ScreenSaverBlanked;
    else
	kind = ScreenSaverInternal;
    for (pEv = pPriv->events; pEv; pEv = pEv->next)
    {
	client = pEv->client;
	if (client->clientGone)
	    continue;
	if (!(pEv->mask & mask))
	    continue;
	ev.type = ScreenSaverNotify + ScreenSaverEventBase;
	ev.state = state;
	ev.sequenceNumber = client->sequence;
	ev.timestamp = currentTime.milliseconds;
	ev.root = WindowTable[pScreen->myNum]->drawable.id;
	ev.window = savedScreenInfo[pScreen->myNum].wid;
	ev.kind = kind;
	ev.forced = forced;
	WriteEventsToClient (client, 1, (xEvent *) &ev);
    }
}

static void
SScreenSaverNotifyEvent (from, to)
    xScreenSaverNotifyEvent *from, *to;
{
    to->type = from->type;
    to->state = from->state;
    cpswaps (from->sequenceNumber, to->sequenceNumber);
    cpswapl (from->timestamp, to->timestamp);    
    cpswapl (from->root, to->root);    
    cpswapl (from->window, to->window);    
    to->kind = from->kind;
    to->forced = from->forced;
}

static void
UninstallSaverColormap (pScreen)
    ScreenPtr	pScreen;
{
    SetupScreen(pScreen);
    ColormapPtr			pCmap;

    if (pPriv && pPriv->installedMap != None)
    {
	pCmap = (ColormapPtr) LookupIDByType (pPriv->installedMap, RT_COLORMAP);
	if (pCmap)
	    (*pCmap->pScreen->UninstallColormap) (pCmap);
	pPriv->installedMap = None;
	CheckScreenPrivate (pScreen);
    }
}

static Bool
CreateSaverWindow (pScreen)
    ScreenPtr	pScreen;
{
    SetupScreen (pScreen);
    ScreenSaverStuffPtr		pSaver;
    ScreenSaverAttrPtr		pAttr;
    WindowPtr			pWin;
    int				result;
    unsigned long		mask;
    Colormap			*installedMaps;
    int				numInstalled;
    int				i;
    Colormap			wantMap;
    ColormapPtr			pCmap;

    pSaver = &savedScreenInfo[pScreen->myNum];
    if (pSaver->pWindow)
    {
	pSaver->pWindow = NullWindow;
	FreeResource (pSaver->wid, RT_NONE);
	if (pPriv)
	{
	    UninstallSaverColormap (pScreen);
	    pPriv->hasWindow = FALSE;
	    CheckScreenPrivate (pScreen);
	}
    }

    if (!pPriv || !(pAttr = pPriv->attr))
	return FALSE;

    pPriv->installedMap = None;

    if (GrabInProgress && GrabInProgress != pAttr->client->index)
	return FALSE;

    pWin = CreateWindow (pSaver->wid, WindowTable[pScreen->myNum],
			 pAttr->x, pAttr->y, pAttr->width, pAttr->height,
			 pAttr->borderWidth, pAttr->class, 
			 pAttr->mask, (XID *)pAttr->values, 
			 pAttr->depth, serverClient, pAttr->visual, 
			 &result);
    if (!pWin)
	return FALSE;

    if (!AddResource(pWin->drawable.id, RT_WINDOW, pWin))
	return FALSE;

    mask = 0;
    if (pAttr->pBackgroundPixmap)
    {
	pWin->backgroundState = BackgroundPixmap;
	pWin->background.pixmap = pAttr->pBackgroundPixmap;
	pAttr->pBackgroundPixmap->refcnt++;
	mask |= CWBackPixmap;
    }
    if (pAttr->pBorderPixmap)
    {
	pWin->borderIsPixel = FALSE;
	pWin->border.pixmap = pAttr->pBorderPixmap;
	pAttr->pBorderPixmap->refcnt++;
	mask |= CWBorderPixmap;
    }
    if (pAttr->pCursor)
    {
	if (!pWin->optional)
	    if (!MakeWindowOptional (pWin))
	    {
    	    	FreeResource (pWin->drawable.id, RT_NONE);
    	    	return FALSE;
	    }
	if (pWin->optional->cursor)
	    FreeCursor (pWin->optional->cursor, (Cursor)0);
	pWin->optional->cursor = pAttr->pCursor;
	pAttr->pCursor->refcnt++;
	pWin->cursorIsNone = FALSE;
	CheckWindowOptionalNeed (pWin);
	mask |= CWCursor;
    }
    if (mask)
	(*pScreen->ChangeWindowAttributes) (pWin, mask);

    if (pAttr->colormap != None)
	(void) ChangeWindowAttributes (pWin, CWColormap, &pAttr->colormap,
				       serverClient);

    MapWindow (pWin, serverClient);

    pPriv->hasWindow = TRUE;
    pSaver->pWindow = pWin;

    /* check and install our own colormap if it isn't installed now */
    wantMap = wColormap (pWin);
    if (wantMap == None)
	return TRUE;
    installedMaps = (Colormap *) ALLOCATE_LOCAL (pScreen->maxInstalledCmaps *
						 sizeof (Colormap));
    numInstalled = (*pWin->drawable.pScreen->ListInstalledColormaps)
						    (pScreen, installedMaps);
    for (i = 0; i < numInstalled; i++) 
	if (installedMaps[i] == wantMap)
	    break;

    DEALLOCATE_LOCAL ((char *) installedMaps);

    if (i < numInstalled)
	return TRUE;

    pCmap = (ColormapPtr) LookupIDByType (wantMap, RT_COLORMAP);
    if (!pCmap)
	return TRUE;

    pPriv->installedMap = wantMap;

    (*pCmap->pScreen->InstallColormap) (pCmap);

    return TRUE;
}

static Bool
DestroySaverWindow (pScreen)
    ScreenPtr	pScreen;
{
    SetupScreen(pScreen);
    ScreenSaverStuffPtr		pSaver;

    if (!pPriv || !pPriv->hasWindow)
	return FALSE;

    pSaver = &savedScreenInfo[pScreen->myNum];
    if (pSaver->pWindow)
    {
	pSaver->pWindow = NullWindow;
	FreeResource (pSaver->wid, RT_NONE);
    }
    pPriv->hasWindow = FALSE;
    CheckScreenPrivate (pScreen);
    UninstallSaverColormap (pScreen);
    return TRUE;
}

static Bool
ScreenSaverHandle (pScreen, xstate, force)
    ScreenPtr	pScreen;
    int		xstate;
    Bool	force;
{
    int				state = 0;
    Bool			ret = FALSE;
    ScreenSaverScreenPrivatePtr	pPriv;

    switch (xstate)
    {
    case SCREEN_SAVER_ON:	
	state = ScreenSaverOn;
	ret = CreateSaverWindow (pScreen);
	break;
    case SCREEN_SAVER_OFF:	
	state = ScreenSaverOff;
	ret = DestroySaverWindow (pScreen);
	break;
    case SCREEN_SAVER_CYCLE:	
	state = ScreenSaverCycle;
	pPriv = GetScreenPrivate (pScreen);
	if (pPriv && pPriv->hasWindow)
	    ret = TRUE;
	
    }
#ifdef PANORAMIX
    if(noPanoramiXExtension || !pScreen->myNum)
#endif
       SendScreenSaverNotify (pScreen, state, force);
    return ret;
}

static int
ProcScreenSaverQueryVersion (client)
    register ClientPtr	client;
{
    xScreenSaverQueryVersionReply	rep;
    register int		n;

    REQUEST_SIZE_MATCH (xScreenSaverQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = ScreenSaverMajorVersion;
    rep.minorVersion = ScreenSaverMinorVersion;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    }
    WriteToClient(client, sizeof (xScreenSaverQueryVersionReply), (char *)&rep);
    return (client->noClientException);
}

int
ProcScreenSaverQueryInfo (client)
    register ClientPtr	client;
{
    REQUEST(xScreenSaverQueryInfoReq);
    xScreenSaverQueryInfoReply	rep;
    register int		n;
    ScreenSaverStuffPtr		pSaver;
    DrawablePtr			pDraw;
    CARD32			lastInput;
    ScreenSaverScreenPrivatePtr	pPriv;

    REQUEST_SIZE_MATCH (xScreenSaverQueryInfoReq);
    pDraw = (DrawablePtr) LookupDrawable (stuff->drawable, client);
    if (!pDraw)
	return BadDrawable;

    pSaver = &savedScreenInfo[pDraw->pScreen->myNum];
    pPriv = GetScreenPrivate (pDraw->pScreen);

    UpdateCurrentTime ();
    lastInput = GetTimeInMillis() - lastDeviceEventTime.milliseconds;

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.window = pSaver->wid;
    if (screenIsSaved != SCREEN_SAVER_OFF)
    {
	rep.state = ScreenSaverOn;
	if (ScreenSaverTime)
	    rep.tilOrSince = lastInput - ScreenSaverTime;
	else
	    rep.tilOrSince = 0;
    }
    else
    {
	if (ScreenSaverTime)
	{
	    rep.state = ScreenSaverOff;
	    if (ScreenSaverTime < lastInput)
		rep.tilOrSince = 0;
	    else
		rep.tilOrSince = ScreenSaverTime - lastInput;
	}
	else
	{
	    rep.state = ScreenSaverDisabled;
	    rep.tilOrSince = 0;
	}
    }
    rep.idle = lastInput;
    rep.eventMask = getEventMask (pDraw->pScreen, client);
    if (pPriv && pPriv->attr)
	rep.kind = ScreenSaverExternal;
    else if (ScreenSaverBlanking != DontPreferBlanking)
	rep.kind = ScreenSaverBlanked;
    else
	rep.kind = ScreenSaverInternal;
    if (client->swapped)
    {
	swaps (&rep.sequenceNumber, n);
	swapl (&rep.length, n);
	swapl (&rep.window, n);
	swapl (&rep.tilOrSince, n);
	swapl (&rep.idle, n);
	swapl (&rep.eventMask, n);
    }
    WriteToClient(client, sizeof (xScreenSaverQueryInfoReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcScreenSaverSelectInput (client)
    register ClientPtr	client;
{
    REQUEST(xScreenSaverSelectInputReq);
    DrawablePtr			pDraw;

    REQUEST_SIZE_MATCH (xScreenSaverSelectInputReq);
    pDraw = (DrawablePtr) LookupDrawable (stuff->drawable, client);
    if (!pDraw)
	return BadDrawable;
    if (!setEventMask (pDraw->pScreen, client, stuff->eventMask))
	return BadAlloc;
    return Success;
}

static int
ScreenSaverSetAttributes (ClientPtr client)
{
    REQUEST(xScreenSaverSetAttributesReq);
    DrawablePtr			pDraw;
    WindowPtr			pParent;
    ScreenPtr			pScreen;
    ScreenSaverScreenPrivatePtr pPriv = 0;
    ScreenSaverAttrPtr		pAttr = 0;
    int				ret;
    int				len;
    int				class, bw, depth;
    unsigned long		visual;
    int				idepth, ivisual;
    Bool			fOK;
    DepthPtr			pDepth;
    WindowOptPtr		ancwopt;
    unsigned int		*pVlist;
    unsigned long		*values = 0;
    unsigned long		tmask, imask;
    unsigned long		val;
    Pixmap			pixID;
    PixmapPtr			pPixmap;
    Cursor			cursorID;
    CursorPtr			pCursor;
    Colormap			cmap;
    ColormapPtr			pCmap;

    REQUEST_AT_LEAST_SIZE (xScreenSaverSetAttributesReq);
    pDraw = (DrawablePtr) LookupDrawable (stuff->drawable, client);
    if (!pDraw)
	return BadDrawable;
    pScreen = pDraw->pScreen;
    pParent = WindowTable[pScreen->myNum];

    len = stuff->length -  (sizeof(xScreenSaverSetAttributesReq) >> 2);
    if (Ones(stuff->mask) != len)
        return BadLength;
    if (!stuff->width || !stuff->height)
    {
	client->errorValue = 0;
        return BadValue;
    }
    switch (class = stuff->c_class) 
    {
    case CopyFromParent:
    case InputOnly:
    case InputOutput:
	break;
    default:
	client->errorValue = class;
	return BadValue;
    }
    bw = stuff->borderWidth;
    depth = stuff->depth;
    visual = stuff->visualID;

    /* copied directly from CreateWindow */

    if (class == CopyFromParent)
	class = pParent->drawable.class;

    if ((class != InputOutput) && (class != InputOnly))
    {
	client->errorValue = class;
	return BadValue;
    }

    if ((class != InputOnly) && (pParent->drawable.class == InputOnly))
        return BadMatch;

    if ((class == InputOnly) && ((bw != 0) || (depth != 0)))
        return BadMatch;

    if ((class == InputOutput) && (depth == 0))
        depth = pParent->drawable.depth;
    ancwopt = pParent->optional;
    if (!ancwopt)
	ancwopt = FindWindowWithOptional(pParent)->optional;
    if (visual == CopyFromParent)
	visual = ancwopt->visual;

    /* Find out if the depth and visual are acceptable for this Screen */
    if ((visual != ancwopt->visual) || (depth != pParent->drawable.depth))
    {
	fOK = FALSE;
	for(idepth = 0; idepth < pScreen->numDepths; idepth++)
	{
	    pDepth = (DepthPtr) &pScreen->allowedDepths[idepth];
	    if ((depth == pDepth->depth) || (depth == 0))
	    {
		for (ivisual = 0; ivisual < pDepth->numVids; ivisual++)
		{
		    if (visual == pDepth->vids[ivisual])
		    {
			fOK = TRUE;
			break;
		    }
		}
	    }
	}
	if (fOK == FALSE)
	    return BadMatch;
    }

    if (((stuff->mask & (CWBorderPixmap | CWBorderPixel)) == 0) &&
	(class != InputOnly) &&
	(depth != pParent->drawable.depth))
    {
        return BadMatch;
    }

    if (((stuff->mask & CWColormap) == 0) &&
	(class != InputOnly) &&
	((visual != ancwopt->visual) || (ancwopt->colormap == None)))
    {
	return BadMatch;
    }

    /* end of errors from CreateWindow */

    pPriv = GetScreenPrivate (pScreen);
    if (pPriv && pPriv->attr)
    {
	if (pPriv->attr->client != client)
	    return BadAccess;
    }
    if (!pPriv)
    {
	pPriv = MakeScreenPrivate (pScreen);
	if (!pPriv)
	    return FALSE;
    }
    pAttr = New (ScreenSaverAttrRec);
    if (!pAttr)
    {
	ret = BadAlloc;
	goto bail;
    }
    /* over allocate for override redirect */
    values = (unsigned long *) xalloc ((len + 1) * sizeof (unsigned long));
    if (!values)
    {
	ret = BadAlloc;
	goto bail;
    }
    pAttr->screen = pScreen;
    pAttr->client = client;
    pAttr->x = stuff->x;
    pAttr->y = stuff->y;
    pAttr->width = stuff->width;
    pAttr->height = stuff->height;
    pAttr->borderWidth = stuff->borderWidth;
    pAttr->class = stuff->c_class;
    pAttr->depth = depth;
    pAttr->visual = visual;
    pAttr->colormap = None;
    pAttr->pCursor = NullCursor;
    pAttr->pBackgroundPixmap = NullPixmap;
    pAttr->pBorderPixmap = NullPixmap;
    pAttr->values = values;
    /*
     * go through the mask, checking the values,
     * looking up pixmaps and cursors and hold a reference
     * to them.
     */
    pAttr->mask = tmask = stuff->mask | CWOverrideRedirect;
    pVlist = (unsigned int *) (stuff + 1);
    while (tmask) {
	imask = lowbit (tmask);
	tmask &= ~imask;
	switch (imask)
        {
	case CWBackPixmap:
	    pixID = (Pixmap )*pVlist;
	    if (pixID == None)
	    {
		*values++ = None;
	    }
	    else if (pixID == ParentRelative)
	    {
		if (depth != pParent->drawable.depth)
		{
		    ret = BadMatch;
		    goto PatchUp;
		}
		*values++ = ParentRelative;
	    }
            else
	    {	
                pPixmap = (PixmapPtr)LookupIDByType(pixID, RT_PIXMAP);
                if (pPixmap != (PixmapPtr) NULL)
		{
                    if  ((pPixmap->drawable.depth != depth) ||
			 (pPixmap->drawable.pScreen != pScreen))
		    {
                        ret = BadMatch;
			goto PatchUp;
		    }
		    pAttr->pBackgroundPixmap = pPixmap;
		    pPixmap->refcnt++;
		    pAttr->mask &= ~CWBackPixmap;
		}
	        else
		{
		    ret = BadPixmap;
		    client->errorValue = pixID;
		    goto PatchUp;
		}
	    }
	    break;
	case CWBackPixel:
	    *values++ = (CARD32) *pVlist;
	    break;
	case CWBorderPixmap:
	    pixID = (Pixmap ) *pVlist;
	    if (pixID == CopyFromParent)
	    {
		if (depth != pParent->drawable.depth)
		{
		    ret = BadMatch;
		    goto PatchUp;
		}
		*values++ = CopyFromParent;
	    }
	    else
	    {	
		pPixmap = (PixmapPtr)LookupIDByType(pixID, RT_PIXMAP);
		if (pPixmap)
		{
                    if  ((pPixmap->drawable.depth != depth) ||
			 (pPixmap->drawable.pScreen != pScreen))
		    {
			ret = BadMatch;
			goto PatchUp;
		    }
		    pAttr->pBorderPixmap = pPixmap;
		    pPixmap->refcnt++;
		    pAttr->mask &= ~CWBorderPixmap;
		}
    	        else
		{
		    ret = BadPixmap;
		    client->errorValue = pixID;
		    goto PatchUp;
		}
	    }
	    break;
	case CWBorderPixel:
            *values++ = (CARD32) *pVlist;
            break;
	case CWBitGravity:
	    val = (CARD8 )*pVlist;
	    if (val > StaticGravity)
	    {
		ret = BadValue;
		client->errorValue = val;
		goto PatchUp;
	    }
	    *values++ = val;
	    break;
	case CWWinGravity:
	    val = (CARD8 )*pVlist;
	    if (val > StaticGravity)
	    {
		ret = BadValue;
		client->errorValue = val;
		goto PatchUp;
	    }
	    *values++ = val;
	    break;
	case CWBackingStore:
	    val = (CARD8 )*pVlist;
	    if ((val != NotUseful) && (val != WhenMapped) && (val != Always))
	    {
		ret = BadValue;
		client->errorValue = val;
		goto PatchUp;
	    }
	    *values++ = val;
	    break;
	case CWBackingPlanes:
	    *values++ = (CARD32) *pVlist;
	    break;
	case CWBackingPixel:
	    *values++ = (CARD32) *pVlist;
	    break;
	case CWSaveUnder:
	    val = (BOOL) *pVlist;
	    if ((val != xTrue) && (val != xFalse))
	    {
		ret = BadValue;
		client->errorValue = val;
		goto PatchUp;
	    }
	    *values++ = val;
	    break;
	case CWEventMask:
	    *values++ = (CARD32) *pVlist;
	    break;
	case CWDontPropagate:
	    *values++ = (CARD32) *pVlist;
	    break;
	case CWOverrideRedirect:
	    if (!(stuff->mask & CWOverrideRedirect))
		pVlist--;
	    else
	    {
	    	val = (BOOL ) *pVlist;
	    	if ((val != xTrue) && (val != xFalse))
	    	{
		    ret = BadValue;
		    client->errorValue = val;
		    goto PatchUp;
	    	}
	    }
	    *values++ = xTrue;
	    break;
	case CWColormap:
	    cmap = (Colormap) *pVlist;
	    pCmap = (ColormapPtr)LookupIDByType(cmap, RT_COLORMAP);
	    if (!pCmap)
	    {
		ret = BadColor;
		client->errorValue = cmap;
		goto PatchUp;
	    }
	    if (pCmap->pVisual->vid != visual || pCmap->pScreen != pScreen)
	    {
		ret = BadMatch;
		goto PatchUp;
	    }
	    pAttr->colormap = cmap;
	    pAttr->mask &= ~CWColormap;
	    break;
	case CWCursor:
	    cursorID = (Cursor ) *pVlist;
	    if ( cursorID == None)
	    {
		*values++ = None;
	    }
	    else
	    {
	    	pCursor = (CursorPtr)LookupIDByType(cursorID, RT_CURSOR);
	    	if (!pCursor)
	    	{
		    ret = BadCursor;
		    client->errorValue = cursorID;
		    goto PatchUp;
	    	}
		pCursor->refcnt++;
		pAttr->pCursor = pCursor;
		pAttr->mask &= ~CWCursor;
	    }
	    break;
     	 default:
	    ret = BadValue;
	    client->errorValue = stuff->mask;
	    goto PatchUp;
	}
	pVlist++;
    }
    if (pPriv->attr)
	FreeScreenAttr (pPriv->attr);
    pPriv->attr = pAttr;
    pAttr->resource = FakeClientID (client->index);
    if (!AddResource (pAttr->resource, AttrType, (pointer) pAttr))
	return BadAlloc;
    return Success;
PatchUp:
    FreeAttrs (pAttr);
bail:
    CheckScreenPrivate (pScreen);
    if (pAttr) xfree (pAttr->values);
    xfree (pAttr);
    return ret;
}

static int
ScreenSaverUnsetAttributes (ClientPtr client)
{
    REQUEST(xScreenSaverSetAttributesReq);
    DrawablePtr			pDraw;
    ScreenSaverScreenPrivatePtr	pPriv;

    REQUEST_SIZE_MATCH (xScreenSaverUnsetAttributesReq);
    pDraw = (DrawablePtr) LookupDrawable (stuff->drawable, client);
    if (!pDraw)
	return BadDrawable;
    pPriv = GetScreenPrivate (pDraw->pScreen);
    if (pPriv && pPriv->attr && pPriv->attr->client == client)
    {
	FreeResource (pPriv->attr->resource, AttrType);
    	FreeScreenAttr (pPriv->attr);
	pPriv->attr = NULL;
	CheckScreenPrivate (pDraw->pScreen);
    }
    return Success;
}

static int
ProcScreenSaverSetAttributes (ClientPtr client)
{
#ifdef PANORAMIX
    if(!noPanoramiXExtension) {
       REQUEST(xScreenSaverSetAttributesReq);
       PanoramiXRes *draw;
       PanoramiXRes *backPix = NULL;
       PanoramiXRes *bordPix = NULL;
       PanoramiXRes *cmap    = NULL;
       int i, status = 0, len;
       int  pback_offset = 0, pbord_offset = 0, cmap_offset = 0;
       XID orig_visual, tmp;

       REQUEST_AT_LEAST_SIZE (xScreenSaverSetAttributesReq);

       if(!(draw = (PanoramiXRes *)SecurityLookupIDByClass(
                   client, stuff->drawable, XRC_DRAWABLE, SecurityWriteAccess)))
           return BadDrawable;

       len = stuff->length -  (sizeof(xScreenSaverSetAttributesReq) >> 2);
       if (Ones(stuff->mask) != len)
           return BadLength;

       if((Mask)stuff->mask & CWBackPixmap) {
          pback_offset = Ones((Mask)stuff->mask & (CWBackPixmap - 1));
          tmp = *((CARD32 *) &stuff[1] + pback_offset);
          if ((tmp != None) && (tmp != ParentRelative)) {
             if(!(backPix = (PanoramiXRes*) SecurityLookupIDByType(
                  client, tmp, XRT_PIXMAP, SecurityReadAccess)))
                return BadPixmap;
          }
       }

       if ((Mask)stuff->mask & CWBorderPixmap) {
          pbord_offset = Ones((Mask)stuff->mask & (CWBorderPixmap - 1));
          tmp = *((CARD32 *) &stuff[1] + pbord_offset);
          if (tmp != CopyFromParent) {
             if(!(bordPix = (PanoramiXRes*) SecurityLookupIDByType(
                  client, tmp, XRT_PIXMAP, SecurityReadAccess)))
                return BadPixmap;
          }
       }

       if ((Mask)stuff->mask & CWColormap) {
           cmap_offset = Ones((Mask)stuff->mask & (CWColormap - 1));
           tmp = *((CARD32 *) &stuff[1] + cmap_offset);
           if ((tmp != CopyFromParent) && (tmp != None)) {
             if(!(cmap = (PanoramiXRes*) SecurityLookupIDByType(
                  client, tmp, XRT_COLORMAP, SecurityReadAccess)))
                 return BadColor;
           }
       }

       orig_visual = stuff->visualID;

       FOR_NSCREENS_BACKWARD(i) {
          stuff->drawable = draw->info[i].id;  
          if (backPix)
             *((CARD32 *) &stuff[1] + pback_offset) = backPix->info[i].id;
          if (bordPix)
             *((CARD32 *) &stuff[1] + pbord_offset) = bordPix->info[i].id;
          if (cmap)
             *((CARD32 *) &stuff[1] + cmap_offset) = cmap->info[i].id;

          if (orig_visual != CopyFromParent) 
            stuff->visualID = 
                     PanoramiXVisualTable[(orig_visual*MAXSCREENS) + i];

          status = ScreenSaverSetAttributes(client);
       }

       return status;
    }
#endif

    return ScreenSaverSetAttributes(client);
}

static int
ProcScreenSaverUnsetAttributes (ClientPtr client)
{
#ifdef PANORAMIX
    if(!noPanoramiXExtension) {
       REQUEST(xScreenSaverUnsetAttributesReq);
       PanoramiXRes *draw;
       int i;

       if(!(draw = (PanoramiXRes *)SecurityLookupIDByClass(
                   client, stuff->drawable, XRC_DRAWABLE, SecurityWriteAccess)))
           return BadDrawable;

       for(i = PanoramiXNumScreens - 1; i > 0; i--) {
            stuff->drawable = draw->info[i].id;
            ScreenSaverUnsetAttributes(client);
       }

       stuff->drawable = draw->info[0].id;
    }
#endif

    return ScreenSaverUnsetAttributes(client);
}

static DISPATCH_PROC((*NormalVector[])) = {
    ProcScreenSaverQueryVersion,
    ProcScreenSaverQueryInfo,
    ProcScreenSaverSelectInput,
    ProcScreenSaverSetAttributes,
    ProcScreenSaverUnsetAttributes,
};

#define NUM_REQUESTS	((sizeof NormalVector) / (sizeof NormalVector[0]))

static int
ProcScreenSaverDispatch (client)
    ClientPtr	client;
{
    REQUEST(xReq);

    if (stuff->data < NUM_REQUESTS)
	return (*NormalVector[stuff->data])(client);
    return BadRequest;
}

static int
SProcScreenSaverQueryVersion (client)
    ClientPtr	client;
{
    REQUEST(xScreenSaverQueryVersionReq);
    int	    n;

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH(xScreenSaverQueryVersionReq);
    return ProcScreenSaverQueryVersion (client);
}

static int
SProcScreenSaverQueryInfo (client)
    ClientPtr	client;
{
    REQUEST(xScreenSaverQueryInfoReq);
    int	    n;

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH(xScreenSaverQueryInfoReq);
    swapl (&stuff->drawable, n);
    return ProcScreenSaverQueryInfo (client);
}

static int
SProcScreenSaverSelectInput (client)
    ClientPtr	client;
{
    REQUEST(xScreenSaverSelectInputReq);
    int	    n;

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH(xScreenSaverSelectInputReq);
    swapl (&stuff->drawable, n);
    swapl (&stuff->eventMask, n);
    return ProcScreenSaverSelectInput (client);
}

static int
SProcScreenSaverSetAttributes (client)
    ClientPtr	client;
{
    REQUEST(xScreenSaverSetAttributesReq);
    int	    n;

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xScreenSaverSetAttributesReq);
    swapl (&stuff->drawable, n);
    swaps (&stuff->x, n);
    swaps (&stuff->y, n);
    swaps (&stuff->width, n);
    swaps (&stuff->height, n);
    swaps (&stuff->borderWidth, n);
    swapl (&stuff->visualID, n);
    swapl (&stuff->mask, n);
    SwapRestL(stuff);
    return ProcScreenSaverSetAttributes (client);
}

static int
SProcScreenSaverUnsetAttributes (client)
    ClientPtr	client;
{
    REQUEST(xScreenSaverUnsetAttributesReq);
    int	    n;

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH(xScreenSaverUnsetAttributesReq);
    swapl (&stuff->drawable, n);
    return ProcScreenSaverUnsetAttributes (client);
}

static DISPATCH_PROC((*SwappedVector[])) = {
    SProcScreenSaverQueryVersion,
    SProcScreenSaverQueryInfo,
    SProcScreenSaverSelectInput,
    SProcScreenSaverSetAttributes,
    SProcScreenSaverUnsetAttributes,
};

static int
SProcScreenSaverDispatch (client)
    ClientPtr	client;
{
    REQUEST(xReq);

    if (stuff->data < NUM_REQUESTS)
	return (*SwappedVector[stuff->data])(client);
    return BadRequest;
}
