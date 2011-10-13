/* $XdotOrg: xc/programs/Xserver/Xext/xtest.c,v 1.6 2005/07/03 08:53:36 daniels Exp $ */
/* $Xorg: xtest.c,v 1.4 2001/02/09 02:04:33 xorgcvs Exp $ */
/*

Copyright 1992, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/programs/Xserver/Xext/xtest.c,v 3.10 2003/10/28 23:08:44 tsi Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#define NEED_EVENTS
#include <X11/Xproto.h>
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "windowstr.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "dixevents.h"
#include "sleepuntil.h"
#define _XTEST_SERVER_
#include <X11/extensions/XTest.h>
#include <X11/extensions/xteststr.h>
#ifdef XINPUT
#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>
#define EXTENSION_EVENT_BASE	64
#include "extinit.h"		/* LookupDeviceIntRec */
#endif /* XINPUT */
#ifdef EXTMODULE
#include "xf86_ansic.h"
#endif

#include "modinit.h"

#if 0
static unsigned char XTestReqCode;
#endif

#ifdef XINPUT
extern int DeviceValuator;
#endif /* XINPUT */

#ifdef PANORAMIX
#include "panoramiX.h"
#include "panoramiXsrv.h"
#endif

static void XTestResetProc(
    ExtensionEntry * /* extEntry */
);
static int XTestSwapFakeInput(
    ClientPtr /* client */,
    xReq * /* req */
);

static DISPATCH_PROC(ProcXTestCompareCursor);
static DISPATCH_PROC(ProcXTestDispatch);
static DISPATCH_PROC(ProcXTestFakeInput);
static DISPATCH_PROC(ProcXTestGetVersion);
static DISPATCH_PROC(ProcXTestGrabControl);
static DISPATCH_PROC(SProcXTestCompareCursor);
static DISPATCH_PROC(SProcXTestDispatch);
static DISPATCH_PROC(SProcXTestFakeInput);
static DISPATCH_PROC(SProcXTestGetVersion);
static DISPATCH_PROC(SProcXTestGrabControl);

void
XTestExtensionInit(INITARGS)
{
#if 0
    ExtensionEntry *extEntry;

    if ((extEntry = AddExtension(XTestExtensionName, 0, 0,
				 ProcXTestDispatch, SProcXTestDispatch,
				 XTestResetProc, StandardMinorOpcode)) != 0)
	XTestReqCode = (unsigned char)extEntry->base;
#else
    (void) AddExtension(XTestExtensionName, 0, 0,
			ProcXTestDispatch, SProcXTestDispatch,
			XTestResetProc, StandardMinorOpcode);
#endif
}

/*ARGSUSED*/
static void
XTestResetProc (extEntry)
ExtensionEntry	*extEntry;
{
}

static int
ProcXTestGetVersion(client)
    register ClientPtr client;
{
    xXTestGetVersionReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xXTestGetVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = XTestMajorVersion;
    rep.minorVersion = XTestMinorVersion;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
	swaps(&rep.minorVersion, n);
    }
    WriteToClient(client, sizeof(xXTestGetVersionReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcXTestCompareCursor(client)
    register ClientPtr client;
{
    REQUEST(xXTestCompareCursorReq);
    xXTestCompareCursorReply rep;
    WindowPtr pWin;
    CursorPtr pCursor;
    register int n;

    REQUEST_SIZE_MATCH(xXTestCompareCursorReq);
    pWin = (WindowPtr)LookupWindow(stuff->window, client);
    if (!pWin)
        return(BadWindow);
    if (stuff->cursor == None)
	pCursor = NullCursor;
    else if (stuff->cursor == XTestCurrentCursor)
	pCursor = GetSpriteCursor();
    else {
	pCursor = (CursorPtr)LookupIDByType(stuff->cursor, RT_CURSOR);
	if (!pCursor) 
	{
	    client->errorValue = stuff->cursor;
	    return (BadCursor);
	}
    }
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.same = (wCursor(pWin) == pCursor);
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    }
    WriteToClient(client, sizeof(xXTestCompareCursorReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcXTestFakeInput(client)
    register ClientPtr client;
{
    REQUEST(xXTestFakeInputReq);
    int nev;
    int	n;
    xEvent *ev;
    DeviceIntPtr dev = NULL;
    WindowPtr root;
    int type;
#ifdef XINPUT
    Bool extension = FALSE;
    deviceValuator *dv = NULL;
    int base;
    int *values;
#endif /* XINPUT */

    nev = (stuff->length << 2) - sizeof(xReq);
    if ((nev % sizeof(xEvent)) || !nev)
	return BadLength;
    nev /= sizeof(xEvent);
    UpdateCurrentTime();
    ev = (xEvent *)&((xReq *)stuff)[1];
    type = ev->u.u.type & 0177;
#ifdef XINPUT
    if (type >= EXTENSION_EVENT_BASE)
    {
	type -= DeviceValuator;
	switch (type) {
	case XI_DeviceKeyPress:
	case XI_DeviceKeyRelease:
	case XI_DeviceButtonPress:
	case XI_DeviceButtonRelease:
	case XI_DeviceMotionNotify:
	case XI_ProximityIn:
	case XI_ProximityOut:
	    break;
	default:
	    client->errorValue = ev->u.u.type;
	    return BadValue;
	}
	if (nev == 1 && type == XI_DeviceMotionNotify)
	    return BadLength;
	if (type == XI_DeviceMotionNotify)
	    base = ((deviceValuator *)(ev+1))->first_valuator;
	else
	    base = 0;
	for (n = 1; n < nev; n++)
	{
	    dv = (deviceValuator *)(ev + n);
	    if (dv->type != DeviceValuator)
	    {
		client->errorValue = dv->type;
		return BadValue;
	    }
	    if (dv->first_valuator != base)
	    {
		client->errorValue = dv->first_valuator;
		return BadValue;
	    }
	    if (!dv->num_valuators || dv->num_valuators > 6)
	    {
		client->errorValue = dv->num_valuators;
		return BadValue;
	    }
	    base += dv->num_valuators;
	}
	type = type - XI_DeviceKeyPress + KeyPress;
	extension = TRUE;
    }
    else
#endif /* XINPUT */
    {
	if (nev != 1)
	    return BadLength;
	switch (type)
	{
	case KeyPress:
	case KeyRelease:
	case MotionNotify:
	case ButtonPress:
	case ButtonRelease:
	    break;
	default:
	    client->errorValue = ev->u.u.type;
	    return BadValue;
	}
    }
    if (ev->u.keyButtonPointer.time)
    {
	TimeStamp activateTime;
	CARD32 ms;

	activateTime = currentTime;
	ms = activateTime.milliseconds + ev->u.keyButtonPointer.time;
	if (ms < activateTime.milliseconds)
	    activateTime.months++;
	activateTime.milliseconds = ms;
	ev->u.keyButtonPointer.time = 0;

	/* see mbuf.c:QueueDisplayRequest for code similar to this */

	if (!ClientSleepUntil(client, &activateTime, NULL, NULL))
	{
	    return BadAlloc;
	}
	/* swap the request back so we can simply re-execute it */
	if (client->swapped)
	{
    	    (void) XTestSwapFakeInput(client, (xReq *)stuff);
	    swaps(&stuff->length, n);
	}
	ResetCurrentRequest (client);
	client->sequence--;
	return Success;
    }
#ifdef XINPUT
    if (extension)
    {
	dev = LookupDeviceIntRec(stuff->deviceid & 0177);
	if (!dev)
	{
	    client->errorValue = stuff->deviceid & 0177;
	    return BadValue;
	}
	if (nev > 1)
	{
	    dv = (deviceValuator *)(ev + 1);
	    if (!dev->valuator || dv->first_valuator >= dev->valuator->numAxes)
	    {
		client->errorValue = dv->first_valuator;
		return BadValue;
	    }
	    if (dv->first_valuator + dv->num_valuators >
		dev->valuator->numAxes)
	    {
		client->errorValue = dv->num_valuators;
		return BadValue;
	    }
	}
    }
#endif /* XINPUT */
    switch (type)
    {
    case KeyPress:
    case KeyRelease:
#ifdef XINPUT
	if (!extension)
#endif /* XINPUT */
	    dev = (DeviceIntPtr)LookupKeyboardDevice();
	if (ev->u.u.detail < dev->key->curKeySyms.minKeyCode ||
	    ev->u.u.detail > dev->key->curKeySyms.maxKeyCode)
	{
	    client->errorValue = ev->u.u.detail;
	    return BadValue;
	}
	break;
    case MotionNotify:
#ifdef XINPUT
	if (extension)
	{
	    if (ev->u.u.detail != xFalse && ev->u.u.detail != xTrue)
	    {
		client->errorValue = ev->u.u.detail;
		return BadValue;
	    }
	    if (ev->u.u.detail == xTrue && dev->valuator->mode == Absolute)
	    {
		values = dev->valuator->axisVal + dv->first_valuator;
		for (n = 1; n < nev; n++)
		{
		    dv = (deviceValuator *)(ev + n);
		    switch (dv->num_valuators)
		    {
		    case 6:
			dv->valuator5 += values[5];
		    case 5:
			dv->valuator4 += values[4];
		    case 4:
			dv->valuator3 += values[3];
		    case 3:
			dv->valuator2 += values[2];
		    case 2:
			dv->valuator1 += values[1];
		    case 1:
			dv->valuator0 += values[0];
		    }
		    values += 6;
		}
	    }
	    break;
	}
#endif /* XINPUT */
	dev = (DeviceIntPtr)LookupPointerDevice();
	if (ev->u.keyButtonPointer.root == None)
	    root = GetCurrentRootWindow();
	else
	{
	    root = LookupWindow(ev->u.keyButtonPointer.root, client);
	    if (!root)
		return BadWindow;
	    if (root->parent)
	    {
		client->errorValue = ev->u.keyButtonPointer.root;
		return BadValue;
	    }
	}
	if (ev->u.u.detail == xTrue)
	{
	    int x, y;
	    GetSpritePosition(&x, &y);
	    ev->u.keyButtonPointer.rootX += x;
	    ev->u.keyButtonPointer.rootY += y;
	}
	else if (ev->u.u.detail != xFalse)
	{
	    client->errorValue = ev->u.u.detail;
	    return BadValue;
	}

#ifdef PANORAMIX
	if (!noPanoramiXExtension) {
	    ScreenPtr pScreen = root->drawable.pScreen;
	    BoxRec    box;
	    int       i;
	    int       x = ev->u.keyButtonPointer.rootX + panoramiXdataPtr[0].x;
	    int       y = ev->u.keyButtonPointer.rootY + panoramiXdataPtr[0].y;
	    if (!POINT_IN_REGION(pScreen, &XineramaScreenRegions[pScreen->myNum],
				 x, y, &box)) {
		FOR_NSCREENS(i) {
		    if (i == pScreen->myNum) continue;
		    if (POINT_IN_REGION(pScreen,
					&XineramaScreenRegions[i],
					x, y, &box)) {
			root = WindowTable[i];
			x   -= panoramiXdataPtr[i].x;
			y   -= panoramiXdataPtr[i].y;
			ev->u.keyButtonPointer.rootX = x;
			ev->u.keyButtonPointer.rootY = y;
			break;
		    }
		}
	    }
	}
#endif

	if (ev->u.keyButtonPointer.rootX < 0)
	    ev->u.keyButtonPointer.rootX = 0;
	else if (ev->u.keyButtonPointer.rootX >= root->drawable.width)
	    ev->u.keyButtonPointer.rootX = root->drawable.width - 1;
	if (ev->u.keyButtonPointer.rootY < 0)
	    ev->u.keyButtonPointer.rootY = 0;
	else if (ev->u.keyButtonPointer.rootY >= root->drawable.height)
	    ev->u.keyButtonPointer.rootY = root->drawable.height - 1;

#ifdef PANORAMIX
	if ((!noPanoramiXExtension
	     && root->drawable.pScreen->myNum != XineramaGetCursorScreen())
	    || (noPanoramiXExtension && root != GetCurrentRootWindow()))

#else
	if (root != GetCurrentRootWindow())
#endif
	{
	    NewCurrentScreen(root->drawable.pScreen,
			     ev->u.keyButtonPointer.rootX,
			     ev->u.keyButtonPointer.rootY);
	    return client->noClientException;
	}
	(*root->drawable.pScreen->SetCursorPosition)
	    (root->drawable.pScreen,
	     ev->u.keyButtonPointer.rootX,
	     ev->u.keyButtonPointer.rootY, FALSE);
	break;
    case ButtonPress:
    case ButtonRelease:
#ifdef XINPUT
	if (!extension)
#endif /* XINPUT */
	    dev = (DeviceIntPtr)LookupPointerDevice();
	if (!ev->u.u.detail || ev->u.u.detail > dev->button->numButtons)
	{
	    client->errorValue = ev->u.u.detail;
	    return BadValue;
	}
	break;
    }
    if (screenIsSaved == SCREEN_SAVER_ON)
	SaveScreens(SCREEN_SAVER_OFF, ScreenSaverReset);
    ev->u.keyButtonPointer.time = currentTime.milliseconds;
    (*dev->public.processInputProc)(ev, dev, nev);
    return client->noClientException;
}

static int
ProcXTestGrabControl(client)
    register ClientPtr client;
{
    REQUEST(xXTestGrabControlReq);

    REQUEST_SIZE_MATCH(xXTestGrabControlReq);
    if ((stuff->impervious != xTrue) && (stuff->impervious != xFalse))
    {
	client->errorValue = stuff->impervious;
        return(BadValue);
    }
    if (stuff->impervious)
	MakeClientGrabImpervious(client);
    else
	MakeClientGrabPervious(client);
    return(client->noClientException);
}

static int
ProcXTestDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_XTestGetVersion:
	return ProcXTestGetVersion(client);
    case X_XTestCompareCursor:
	return ProcXTestCompareCursor(client);
    case X_XTestFakeInput:
	return ProcXTestFakeInput(client);
    case X_XTestGrabControl:
	return ProcXTestGrabControl(client);
    default:
	return BadRequest;
    }
}

static int
SProcXTestGetVersion(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xXTestGetVersionReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXTestGetVersionReq);
    swaps(&stuff->minorVersion, n);
    return ProcXTestGetVersion(client);
}

static int
SProcXTestCompareCursor(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xXTestCompareCursorReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXTestCompareCursorReq);
    swapl(&stuff->window, n);
    swapl(&stuff->cursor, n);
    return ProcXTestCompareCursor(client);
}

static int
XTestSwapFakeInput(client, req)
    register ClientPtr	client;
    xReq *req;
{
    register int nev;
    register xEvent *ev;
    xEvent sev;
    EventSwapPtr proc;

    nev = ((req->length << 2) - sizeof(xReq)) / sizeof(xEvent);
    for (ev = (xEvent *)&req[1]; --nev >= 0; ev++)
    {
    	/* Swap event */
    	proc = EventSwapVector[ev->u.u.type & 0177];
	/* no swapping proc; invalid event type? */
    	if (!proc ||  proc ==  NotImplemented) {
	    client->errorValue = ev->u.u.type;
	    return BadValue;
	}
    	(*proc)(ev, &sev);
	*ev = sev;
    }
    return Success;
}

static int
SProcXTestFakeInput(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xReq);

    swaps(&stuff->length, n);
    n = XTestSwapFakeInput(client, stuff);
    if (n != Success)
	return n;
    return ProcXTestFakeInput(client);
}

static int
SProcXTestGrabControl(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xXTestGrabControlReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXTestGrabControlReq);
    return ProcXTestGrabControl(client);
}

static int
SProcXTestDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_XTestGetVersion:
	return SProcXTestGetVersion(client);
    case X_XTestCompareCursor:
	return SProcXTestCompareCursor(client);
    case X_XTestFakeInput:
	return SProcXTestFakeInput(client);
    case X_XTestGrabControl:
	return SProcXTestGrabControl(client);
    default:
	return BadRequest;
    }
}
