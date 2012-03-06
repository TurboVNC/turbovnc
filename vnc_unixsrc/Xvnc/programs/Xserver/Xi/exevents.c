/* $Xorg: exevents.c,v 1.4 2001/02/09 02:04:33 xorgcvs Exp $ */
/************************************************************

Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

Copyright 1989 by Hewlett-Packard Company, Palo Alto, California.

			All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Hewlett-Packard not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
HEWLETT-PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/
/* $XFree86: xc/programs/Xserver/Xi/exevents.c,v 3.11 2003/11/17 22:20:29 dawes Exp $ */

/********************************************************************
 *
 *  Routines to register and initialize extension input devices.
 *  This also contains ProcessOtherEvent, the routine called from DDX
 *  to route extension events.
 *
 */

#define	 NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "XI.h"
#include "XIproto.h"
#include "inputstr.h"
#include "windowstr.h"
#include "miscstruct.h"
#include "region.h"
#include "exevents.h"
#include "extnsionst.h"
#include "extinit.h"			/* LookupDeviceIntRec */
#include "exglobals.h"
#include "dixevents.h"			/* DeliverFocusedEvent */
#include "dixgrabs.h"			/* CreateGrab() */

#include "chgptr.h"

#define WID(w) ((w) ? ((w)->drawable.id) : 0)
#define AllModifiersMask ( \
	ShiftMask | LockMask | ControlMask | Mod1Mask | Mod2Mask | \
	Mod3Mask | Mod4Mask | Mod5Mask )
#define AllButtonsMask ( \
	Button1Mask | Button2Mask | Button3Mask | Button4Mask | Button5Mask )
#define Motion_Filter(class) (DevicePointerMotionMask | \
			      (class)->state | (class)->motionMask)

static Bool		ShouldFreeInputMasks(
				WindowPtr /* pWin */,
				Bool /* ignoreSelectedEvents */
				);
static Bool		MakeInputMasks (
				WindowPtr /* pWin */
				);

/**************************************************************************
 *
 * Procedures for extension device event routing.
 *
 */

void
RegisterOtherDevice (device)
    DeviceIntPtr device;
    {
    device->public.processInputProc = ProcessOtherEvent;
    device->public.realInputProc = ProcessOtherEvent;
    (device)->ActivateGrab = ActivateKeyboardGrab;
    (device)->DeactivateGrab = DeactivateKeyboardGrab;
    }

/*ARGSUSED*/
void
ProcessOtherEvent (xE, other, count)
    xEventPtr xE;
    register DeviceIntPtr other;
    int count;
    {
    register BYTE   	*kptr;
    register int    	i;
    register CARD16 	modifiers;
    register CARD16 	mask;
    GrabPtr         	grab = other->grab;
    Bool            	deactivateDeviceGrab = FALSE;
    int             	key = 0, bit = 0, rootX, rootY;
    ButtonClassPtr	b = other->button;
    KeyClassPtr		k = other->key;
    ValuatorClassPtr	v = other->valuator;
    deviceValuator	*xV = (deviceValuator *) xE;

    if (xE->u.u.type != DeviceValuator) {
        GetSpritePosition(&rootX, &rootY);
        xE->u.keyButtonPointer.rootX = rootX;
        xE->u.keyButtonPointer.rootY = rootY;
        key = xE->u.u.detail;
        NoticeEventTime(xE);
        xE->u.keyButtonPointer.state = inputInfo.keyboard->key->state | 
		    inputInfo.pointer->button->state;
        bit = 1 << (key & 7);
    }
    if (DeviceEventCallback)
    {
	DeviceEventInfoRec eventinfo;
	eventinfo.events = (xEventPtr) xE;
	eventinfo.count = count;
	CallCallbacks(&DeviceEventCallback, (pointer)&eventinfo);
    }
    for (i=1; i<count; i++)
	if ((++xV)->type == DeviceValuator)
	    {
	    int first = xV->first_valuator;
	    int *axisvals;

	    if (xV->num_valuators && (!v || (xV->num_valuators && (first + xV->num_valuators > v->numAxes))))
		FatalError("Bad valuators reported for device %s\n",other->name);
	    xV->device_state = 0;
	    if (k)
		xV->device_state |= k->state;
	    if (b)
	        xV->device_state |= b->state;
	    if (v && v->axisVal)
		{
	        axisvals = v->axisVal;
	        switch (xV->num_valuators) {
		    case 6:
		        *(axisvals+first+5) = xV->valuator5;
		    case 5:
		        *(axisvals+first+4) = xV->valuator4;
		    case 4:
		        *(axisvals+first+3) = xV->valuator3;
		    case 3:
		        *(axisvals+first+2) = xV->valuator2;
		    case 2:
		        *(axisvals+first+1) = xV->valuator1;
		    case 1:
		        *(axisvals+first) = xV->valuator0;
		    case 0:
		    default:
		        break;
		    }
		}
	    }
    
    if (xE->u.u.type == DeviceKeyPress)
	{
	modifiers = k->modifierMap[key];
        kptr = &k->down[key >> 3];
	if (*kptr & bit) /* allow ddx to generate multiple downs */
	    {   
	    if (!modifiers)
		{
		xE->u.u.type = DeviceKeyRelease;
		ProcessOtherEvent(xE, other, count);
		xE->u.u.type = DeviceKeyPress;
		/* release can have side effects, don't fall through */
		ProcessOtherEvent(xE, other, count);
		}
	    return;
	    }
	if (other->valuator)
	    other->valuator->motionHintWindow = NullWindow;
	*kptr |= bit;
	k->prev_state = k->state;
	for (i = 0, mask = 1; modifiers; i++, mask <<= 1)
	    {
	    if (mask & modifiers) 
	        {
		/* This key affects modifier "i" */
		k->modifierKeyCount[i]++;
		k->state |= mask;
		modifiers &= ~mask;
		}
	    }
	if (!grab && CheckDeviceGrabs(other, xE, 0, count))
	    {
	    other->activatingKey = key;
	    return;
	    }
	}
    else if (xE->u.u.type == DeviceKeyRelease)
	{
        kptr = &k->down[key >> 3];
	if (!(*kptr & bit)) /* guard against duplicates */
	    return;
	modifiers = k->modifierMap[key];
	if (other->valuator)
	    other->valuator->motionHintWindow = NullWindow;
	*kptr &= ~bit;
	k->prev_state = k->state;
	for (i = 0, mask = 1; modifiers; i++, mask <<= 1)
	    {
	    if (mask & modifiers) 
	        {
		/* This key affects modifier "i" */
		if (--k->modifierKeyCount[i] <= 0) 
		    {
		    k->modifierKeyCount[i] = 0;
		    k->state &= ~mask;
		    }
		modifiers &= ~mask;
		}
	    }

	if (other->fromPassiveGrab && (key == other->activatingKey))
	    deactivateDeviceGrab = TRUE;
	}
    else if (xE->u.u.type == DeviceButtonPress)
	{
        kptr = &b->down[key >> 3];
	*kptr |= bit;
	if (other->valuator)
	    other->valuator->motionHintWindow = NullWindow;
	b->buttonsDown++;
	b->motionMask = DeviceButtonMotionMask;
	xE->u.u.detail = b->map[key];
	if (xE->u.u.detail == 0)
	     return;
	if (xE->u.u.detail <= 5)
	    b->state |= (Button1Mask >> 1) << xE->u.u.detail;
	SetMaskForEvent(Motion_Filter(b),DeviceMotionNotify);
	if (!grab)
	    if (CheckDeviceGrabs(other, xE, 0, count))
		return;

	}
    else if (xE->u.u.type == DeviceButtonRelease)
	{
        kptr = &b->down[key >> 3];
	*kptr &= ~bit;
	if (other->valuator)
	    other->valuator->motionHintWindow = NullWindow;
	if (!--b->buttonsDown)
		b->motionMask = 0;
	xE->u.u.detail = b->map[key];
	if (xE->u.u.detail == 0)
	    return;
	if (xE->u.u.detail <= 5)
	    b->state &= ~((Button1Mask >> 1) << xE->u.u.detail);
	SetMaskForEvent(Motion_Filter(b),DeviceMotionNotify);
	if (!b->state && other->fromPassiveGrab)
	    deactivateDeviceGrab = TRUE;
	}
    else if (xE->u.u.type == ProximityIn)
	other->valuator->mode &= ~OutOfProximity;
    else if (xE->u.u.type == ProximityOut)
	other->valuator->mode |= OutOfProximity;

    if (grab)
	DeliverGrabbedEvent(xE, other, deactivateDeviceGrab, count);
    else if (other->focus)
	DeliverFocusedEvent(other, xE, GetSpriteWindow(), count);
    else
	DeliverDeviceEvents(GetSpriteWindow(), xE, NullGrab, NullWindow,
			    other, count);

    if (deactivateDeviceGrab == TRUE)
        (*other->DeactivateGrab)(other);
    }

int
InitProximityClassDeviceStruct( DeviceIntPtr dev)
{
    register ProximityClassPtr proxc;

    proxc = (ProximityClassPtr)xalloc(sizeof(ProximityClassRec));
    if (!proxc)
	return FALSE;
    dev->proximity = proxc;
    return TRUE;
}

void
InitValuatorAxisStruct(	DeviceIntPtr dev,
						int axnum,
						int minval,
						int maxval,
						int resolution,
						int min_res,
						int max_res )
{
    register AxisInfoPtr ax = dev->valuator->axes + axnum;

    ax->min_value = minval;
    ax->max_value = maxval;
    ax->resolution = resolution;
    ax->min_resolution = min_res;
    ax->max_resolution = max_res;
}

static void
FixDeviceStateNotify (
    DeviceIntPtr dev,
    deviceStateNotify *ev,
    KeyClassPtr k,
    ButtonClassPtr b,
    ValuatorClassPtr v,
    int first)
{
    ev->type = DeviceStateNotify;
    ev->deviceid = dev->id;
    ev->time = currentTime.milliseconds;
    ev->classes_reported = 0;
    ev->num_keys = 0;
    ev->num_buttons = 0;
    ev->num_valuators = 0;

    if (b) {
	ev->classes_reported |= (1 << ButtonClass);
	ev->num_buttons = b->numButtons;
	memmove((char *) &ev->buttons[0], (char *) b->down, 4);
	}
    else if (k) {
	ev->classes_reported |= (1 << KeyClass);
	ev->num_keys = k->curKeySyms.maxKeyCode - k->curKeySyms.minKeyCode;
	memmove((char *) &ev->keys[0], (char *) k->down, 4);
	}
    if (v) {
	int nval = v->numAxes - first;
	ev->classes_reported |= (1 << ValuatorClass);
	ev->classes_reported |= (dev->valuator->mode << ModeBitsShift);
	ev->num_valuators = nval < 3 ? nval : 3;
	switch (ev->num_valuators) 
	    {
	    case 3:
		ev->valuator2 = v->axisVal[first+2];
	    case 2:
		ev->valuator1 = v->axisVal[first+1];
	    case 1:
		ev->valuator0 = v->axisVal[first];
	    break;
	    }
	}
    }

static void
FixDeviceValuator (
    DeviceIntPtr dev,
    deviceValuator *ev,
    ValuatorClassPtr v,
    int first)
{
    int nval = v->numAxes - first;

    ev->type = DeviceValuator;
    ev->deviceid = dev->id;
    ev->num_valuators = nval < 3 ? nval : 3;
    ev->first_valuator = first;
    switch (ev->num_valuators) {
	case 3:
	    ev->valuator2 = v->axisVal[first+2];
	case 2:
	    ev->valuator1 = v->axisVal[first+1];
	case 1:
	    ev->valuator0 = v->axisVal[first];
	break;
	}
    first += ev->num_valuators;
    }

void
DeviceFocusEvent(dev, type, mode, detail, pWin)
    DeviceIntPtr dev;
    int type, mode, detail;
    register WindowPtr pWin;
    {
    deviceFocus	event;

    if (type == FocusIn)
	type = DeviceFocusIn;
    else
	type = DeviceFocusOut;

    event.deviceid = dev->id;
    event.mode = mode;
    event.type = type;
    event.detail = detail;
    event.window = pWin->drawable.id;
    event.time = currentTime.milliseconds;

    (void) DeliverEventsToWindow(pWin, (xEvent *)&event, 1,
    	DeviceFocusChangeMask, NullGrab, dev->id);

    if ((type == DeviceFocusIn) && 
	(wOtherInputMasks(pWin)) &&
	(wOtherInputMasks(pWin)->inputEvents[dev->id] & DeviceStateNotifyMask))
        {
	int 			evcount = 1;
	deviceStateNotify 	*ev, *sev;
	deviceKeyStateNotify 	*kev;
	deviceButtonStateNotify *bev;

	KeyClassPtr k;
	ButtonClassPtr b;
	ValuatorClassPtr v;
	int nval=0, nkeys=0, nbuttons=0, first=0;

	if ((b=dev->button) != NULL) {
	    nbuttons = b->numButtons;
	    if (nbuttons > 32)
		evcount++;
	}
	if ((k=dev->key) != NULL) {
	    nkeys = k->curKeySyms.maxKeyCode - k->curKeySyms.minKeyCode;
	    if (nkeys > 32)
		evcount++;
	    if (nbuttons > 0) {
		evcount++;
	    }
	}
	if ((v=dev->valuator) != NULL) {
	    nval = v->numAxes;

	    if (nval > 3)
		evcount++;
	    if (nval > 6) {
		if (!(k && b))
		    evcount++;
		if (nval > 9)
		    evcount += ((nval - 7) / 3);
	    }
	}

	sev = ev = (deviceStateNotify *) xalloc(evcount * sizeof(xEvent));
	FixDeviceStateNotify (dev, ev, NULL, NULL, NULL, first);

	if (b != NULL) {
	    FixDeviceStateNotify (dev, ev++, NULL, b, v, first);
	    first += 3;
	    nval -= 3;
	    if (nbuttons > 32) {
		(ev-1)->deviceid |= MORE_EVENTS;
		bev = (deviceButtonStateNotify *) ev++; 
		bev->type = DeviceButtonStateNotify;
		bev->deviceid = dev->id;
		memmove((char *) &bev->buttons[0], (char *) &b->down[4], 28);
	    }
	    if (nval > 0) {
		(ev-1)->deviceid |= MORE_EVENTS;
		FixDeviceValuator (dev, (deviceValuator *) ev++, v, first);
		first += 3;
		nval -= 3;
	    }
	}

	if (k != NULL) {
	    FixDeviceStateNotify (dev, ev++, k, NULL, v, first);
	    first += 3;
	    nval -= 3;
	    if (nkeys > 32) {
		(ev-1)->deviceid |= MORE_EVENTS;
		kev = (deviceKeyStateNotify *) ev++; 
		kev->type = DeviceKeyStateNotify;
		kev->deviceid = dev->id;
		memmove((char *) &kev->keys[0], (char *) &k->down[4], 28);
	    }
	    if (nval > 0) {
		(ev-1)->deviceid |= MORE_EVENTS;
		FixDeviceValuator (dev, (deviceValuator *) ev++, v, first);
		first += 3;
		nval -= 3;
	    }
	}

	while (nval > 0) {
	    FixDeviceStateNotify (dev, ev++, NULL, NULL, v, first);
	    first += 3;
	    nval -= 3;
	    if (nval > 0) {
		(ev-1)->deviceid |= MORE_EVENTS;
		FixDeviceValuator (dev, (deviceValuator *) ev++, v, first);
		first += 3;
		nval -= 3;
	    }
	}

	(void) DeliverEventsToWindow(pWin, (xEvent *)sev, evcount,
	    DeviceStateNotifyMask, NullGrab, dev->id);
	xfree (sev);
        }
    }

int
GrabButton(
    ClientPtr client,
    DeviceIntPtr dev,
    BYTE this_device_mode,
    BYTE other_devices_mode,
    CARD16 modifiers,
    DeviceIntPtr modifier_device,
    CARD8 button,
    Window grabWindow,
    BOOL ownerEvents,
    Cursor rcursor,
    Window rconfineTo,
    Mask eventMask)
{
    WindowPtr pWin, confineTo;
    CursorPtr cursor;
    GrabPtr grab;

    if ((this_device_mode != GrabModeSync) &&
	(this_device_mode != GrabModeAsync))
    {
	client->errorValue = this_device_mode;
        return BadValue;
    }
    if ((other_devices_mode != GrabModeSync) &&
	(other_devices_mode != GrabModeAsync))
    {
	client->errorValue = other_devices_mode;
        return BadValue;
    }
    if ((modifiers != AnyModifier) &&
	(modifiers & ~AllModifiersMask))
    {
	client->errorValue = modifiers;
	return BadValue;
    }
    if ((ownerEvents != xFalse) && (ownerEvents != xTrue))
    {
	client->errorValue = ownerEvents;
	return BadValue;
    }
    pWin = LookupWindow(grabWindow, client);
    if (!pWin)
	return BadWindow;
    if (rconfineTo == None)
	confineTo = NullWindow;
    else
    {
	confineTo = LookupWindow(rconfineTo, client);
	if (!confineTo)
	    return BadWindow;
    }
    if (rcursor == None)
	cursor = NullCursor;
    else
    {
	cursor = (CursorPtr)LookupIDByType(rcursor, RT_CURSOR);
	if (!cursor)
	{
	    client->errorValue = rcursor;
	    return BadCursor;
	}
    }

    grab = CreateGrab(client->index, dev, pWin, eventMask,
	(Bool)ownerEvents, (Bool) this_device_mode, (Bool)other_devices_mode,
	modifier_device, modifiers, DeviceButtonPress, button, confineTo, 
	cursor);
    if (!grab)
	return BadAlloc;
    return AddPassiveGrabToList(grab);
    }

int
GrabKey(
    ClientPtr client,
    DeviceIntPtr dev,
    BYTE this_device_mode,
    BYTE other_devices_mode,
    CARD16 modifiers,
    DeviceIntPtr modifier_device,
    CARD8 key,
    Window grabWindow,
    BOOL ownerEvents,
    Mask mask)
{
    WindowPtr pWin;
    GrabPtr grab;
    KeyClassPtr k = dev->key;

    if (k==NULL)
	return BadMatch;
    if ((other_devices_mode != GrabModeSync) &&
	(other_devices_mode != GrabModeAsync))
    {
	client->errorValue = other_devices_mode;
        return BadValue;
    }
    if ((this_device_mode != GrabModeSync) &&
	(this_device_mode != GrabModeAsync))
    {
	client->errorValue = this_device_mode;
        return BadValue;
    }
    if (((key > k->curKeySyms.maxKeyCode) || 
	 (key < k->curKeySyms.minKeyCode))
	&& (key != AnyKey))
    {
	client->errorValue = key;
        return BadValue;
    }
    if ((modifiers != AnyModifier) &&
	(modifiers & ~AllModifiersMask))
    {
	client->errorValue = modifiers;
	return BadValue;
    }
    if ((ownerEvents != xTrue) && (ownerEvents != xFalse))
    {
	client->errorValue = ownerEvents;
        return BadValue;
    }
    pWin = LookupWindow(grabWindow, client);
    if (!pWin)
	return BadWindow;

    grab = CreateGrab(client->index, dev, pWin, 
	mask, ownerEvents, this_device_mode, other_devices_mode, 
	modifier_device, modifiers, DeviceKeyPress, key, NullWindow, 
	NullCursor);
    if (!grab)
	return BadAlloc;
    return AddPassiveGrabToList(grab);
    }

int
SelectForWindow(dev, pWin, client, mask, exclusivemasks, validmasks)
	DeviceIntPtr dev;
	WindowPtr pWin;
	ClientPtr client;
	Mask mask;
	Mask exclusivemasks;
	Mask validmasks;
{
    int mskidx = dev->id;
    int i, ret;
    Mask check;
    InputClientsPtr others;

    if (mask & ~validmasks)
    {
	client->errorValue = mask;
	return BadValue;
    }
    check = (mask & exclusivemasks);
    if (wOtherInputMasks(pWin))
	{
	if (check & wOtherInputMasks(pWin)->inputEvents[mskidx])
	    {			       /* It is illegal for two different
				          clients to select on any of the
				          events for maskcheck. However,
				          it is OK, for some client to
				          continue selecting on one of those
				          events.  */
	    for (others = wOtherInputMasks(pWin)->inputClients; others; 
		others = others->next)
	        {
	        if (!SameClient(others, client) && (check & 
		    others->mask[mskidx]))
		    return BadAccess;
	        }
            }
	for (others = wOtherInputMasks(pWin)->inputClients; others; 
		others = others->next)
	    {
	    if (SameClient(others, client))
	        {
		check = others->mask[mskidx];
		others->mask[mskidx] = mask;
		if (mask == 0)
		    {
		    for (i=0; i<EMASKSIZE; i++)
			if (i != mskidx && others->mask[i] != 0)
			    break;
		    if (i == EMASKSIZE)
			{
			RecalculateDeviceDeliverableEvents(pWin);
			if (ShouldFreeInputMasks(pWin, FALSE))
			    FreeResource(others->resource, RT_NONE);
		        return Success;
			}
		    }
		goto maskSet;
	        }
	    }
	}
    check = 0;
    if ((ret = AddExtensionClient (pWin, client, mask, mskidx)) != Success)
	return ret;
maskSet: 
    if (dev->valuator)
	if ((dev->valuator->motionHintWindow == pWin) &&
	    (mask & DevicePointerMotionHintMask) &&
	    !(check & DevicePointerMotionHintMask) &&
	    !dev->grab)
	    dev->valuator->motionHintWindow = NullWindow;
    RecalculateDeviceDeliverableEvents(pWin);
    return Success;
}

int 
AddExtensionClient (pWin, client, mask, mskidx)
    WindowPtr pWin;
    ClientPtr client;
    Mask mask;
    int mskidx;
    {
    InputClientsPtr others;

    if (!pWin->optional && !MakeWindowOptional (pWin))
	return BadAlloc;
    others = (InputClients *) xalloc(sizeof(InputClients));
    if (!others)
	return BadAlloc;
    if (!pWin->optional->inputMasks && !MakeInputMasks (pWin))
	return BadAlloc;
    bzero((char *) &others->mask[0], sizeof(Mask)*EMASKSIZE);
    others->mask[mskidx] = mask;
    others->resource = FakeClientID(client->index);
    others->next = pWin->optional->inputMasks->inputClients;
    pWin->optional->inputMasks->inputClients = others;
    if (!AddResource(others->resource, RT_INPUTCLIENT, (pointer)pWin))
	return BadAlloc;
    return Success;
    }

static Bool
MakeInputMasks (pWin)
    WindowPtr	pWin;
    {
    struct _OtherInputMasks *imasks;

    imasks = (struct _OtherInputMasks *) 
	xalloc (sizeof (struct _OtherInputMasks));
    if (!imasks)
	return FALSE;
    bzero((char *) imasks, sizeof (struct _OtherInputMasks));
    pWin->optional->inputMasks = imasks;
    return TRUE;
    }

void
RecalculateDeviceDeliverableEvents(pWin)
    WindowPtr pWin;
    {
    register InputClientsPtr others;
    struct _OtherInputMasks *inputMasks;   /* default: NULL */
    register WindowPtr pChild, tmp;
    int i;

    pChild = pWin;
    while (1)
	{
	if ((inputMasks = wOtherInputMasks(pChild)) != 0)
	    {
	    for (others = inputMasks->inputClients; others; 
		others = others->next)
		{
		for (i=0; i<EMASKSIZE; i++)
		    inputMasks->inputEvents[i] |= others->mask[i];
		}
	    for (i=0; i<EMASKSIZE; i++)
		inputMasks->deliverableEvents[i] = inputMasks->inputEvents[i];
	    for (tmp = pChild->parent; tmp; tmp=tmp->parent)
		if (wOtherInputMasks(tmp))
		    for (i=0; i<EMASKSIZE; i++)
			inputMasks->deliverableEvents[i] |=
			(wOtherInputMasks(tmp)->deliverableEvents[i] 
			& ~inputMasks->dontPropagateMask[i] & PropagateMask[i]);
	    }
	if (pChild->firstChild)
	    {
	    pChild = pChild->firstChild;
	    continue;
	    }
	while (!pChild->nextSib && (pChild != pWin))
	    pChild = pChild->parent;
	if (pChild == pWin)
	    break;
	pChild = pChild->nextSib;
	}
    }

int
InputClientGone(pWin, id)
    register WindowPtr pWin;
    XID   id;
    {
    register InputClientsPtr other, prev;
    if (!wOtherInputMasks(pWin))
	return(Success);
    prev = 0;
    for (other = wOtherInputMasks(pWin)->inputClients; other; 
	other = other->next)
	{
	if (other->resource == id)
	    {
	    if (prev)
		{
		prev->next = other->next;
		xfree(other);
		}
	    else if (!(other->next))
		{
	        if (ShouldFreeInputMasks(pWin, TRUE))
		    {
		    wOtherInputMasks(pWin)->inputClients = other->next;
		    xfree(wOtherInputMasks(pWin));
		    pWin->optional->inputMasks = (OtherInputMasks *) NULL;
		    CheckWindowOptionalNeed (pWin);
		    xfree(other);
		    }
		else
		    {
		    other->resource = FakeClientID(0);
		    if (!AddResource(other->resource, RT_INPUTCLIENT, 
			(pointer)pWin))
			return BadAlloc;
		    }
		}
	    else
		{
		wOtherInputMasks(pWin)->inputClients = other->next;
		xfree(other);
		}
	    RecalculateDeviceDeliverableEvents(pWin);
	    return(Success);
	    }
	prev = other;
        }
    FatalError("client not on device event list");
    /*NOTREACHED*/
    return 0;
    }

int
SendEvent (client, d, dest, propagate, ev, mask, count)
    ClientPtr		client;
    DeviceIntPtr	d;
    Window		dest;
    Bool		propagate;
    xEvent		*ev;
    Mask		mask;
    int			count;
    {
    WindowPtr pWin;
    WindowPtr effectiveFocus = NullWindow; /* only set if dest==InputFocus */
    WindowPtr spriteWin=GetSpriteWindow();

    if (dest == PointerWindow)
	pWin = spriteWin;
    else if (dest == InputFocus)
    {
	WindowPtr inputFocus;
	
	if (!d->focus)
	    inputFocus = spriteWin;
	else
	    inputFocus = d->focus->win;

	if (inputFocus == FollowKeyboardWin)
	    inputFocus = inputInfo.keyboard->focus->win;

	if (inputFocus == NoneWin)
	    return Success;

	/* If the input focus is PointerRootWin, send the event to where
	the pointer is if possible, then perhaps propogate up to root. */
   	if (inputFocus == PointerRootWin)
	    inputFocus = GetCurrentRootWindow();

	if (IsParent(inputFocus, spriteWin))
	{
	    effectiveFocus = inputFocus;
	    pWin = spriteWin;
	}
	else
	    effectiveFocus = pWin = inputFocus;
    }
    else
	pWin = LookupWindow(dest, client);
    if (!pWin)
	return BadWindow;
    if ((propagate != xFalse) && (propagate != xTrue))
    {
	client->errorValue = propagate;
	return BadValue;
    }
    ev->u.u.type |= 0x80;
    if (propagate)
    {
	for (;pWin; pWin = pWin->parent)
	{
	    if (DeliverEventsToWindow( pWin, ev, count, mask, NullGrab, d->id))
		return Success;
	    if (pWin == effectiveFocus)
		return Success;
	    if (wOtherInputMasks(pWin))
		mask &= ~wOtherInputMasks(pWin)->dontPropagateMask[d->id];
	    if (!mask)
		break;
	}
    }
    else
	(void)(DeliverEventsToWindow( pWin, ev, count, mask, NullGrab, d->id));
    return Success;
    }

int
SetButtonMapping (client, dev, nElts, map)
    ClientPtr client;
    DeviceIntPtr dev;
    int nElts;
    BYTE *map;
    {
    register int i;
    ButtonClassPtr b = dev->button;

    if (b == NULL)
	return BadMatch;

    if (nElts != b->numButtons)
    {
	client->errorValue = nElts;
	return BadValue;
    }
    if (BadDeviceMap(&map[0], nElts, 1, 255, &client->errorValue))
	return BadValue;
    for (i=0; i < nElts; i++)
	if ((b->map[i + 1] != map[i]) &&
		BitIsOn(b->down, i + 1))
    	    return MappingBusy;
    for (i = 0; i < nElts; i++)
	b->map[i + 1] = map[i];
    return Success;
    }

int 
SetModifierMapping(client, dev, len, rlen, numKeyPerModifier, inputMap, k)
    ClientPtr client;
    DeviceIntPtr dev;
    int len;
    int rlen;
    int numKeyPerModifier;
    KeyCode *inputMap;
    KeyClassPtr *k;
{
    KeyCode *map = NULL;
    int inputMapLen;
    register int i;
    
    *k = dev->key;
    if (*k == NULL)
	return BadMatch;
    if (len != ((numKeyPerModifier<<1) + rlen))
	return BadLength;

    inputMapLen = 8*numKeyPerModifier;

    /*
     *	Now enforce the restriction that "all of the non-zero keycodes must be
     *	in the range specified by min-keycode and max-keycode in the
     *	connection setup (else a Value error)"
     */
    i = inputMapLen;
    while (i--) {
	if (inputMap[i]
	    && (inputMap[i] < (*k)->curKeySyms.minKeyCode
		|| inputMap[i] > (*k)->curKeySyms.maxKeyCode)) {
		client->errorValue = inputMap[i];
		return -1; /* BadValue collides with MappingFailed */
		}
    }

    /*
     *	Now enforce the restriction that none of the old or new
     *	modifier keys may be down while we change the mapping,  and
     *	that the DDX layer likes the choice.
     */
    if (!AllModifierKeysAreUp (dev, (*k)->modifierKeyMap, 
	(int)(*k)->maxKeysPerModifier, inputMap, (int)numKeyPerModifier)
	    ||
	!AllModifierKeysAreUp(dev, inputMap, (int)numKeyPerModifier,
	      (*k)->modifierKeyMap, (int)(*k)->maxKeysPerModifier)) {
	return MappingBusy;
    } else {
	for (i = 0; i < inputMapLen; i++) {
	    if (inputMap[i] && !LegalModifier(inputMap[i], (DevicePtr)dev)) {
		return MappingFailed;
	    }
	}
    }

    /*
     *	Now build the keyboard's modifier bitmap from the
     *	list of keycodes.
     */
    if (inputMapLen) {
	map = (KeyCode *)xalloc(inputMapLen);
        if (!map)
            return BadAlloc;
    }
    if ((*k)->modifierKeyMap)
        xfree((*k)->modifierKeyMap);
    if (inputMapLen) {
        (*k)->modifierKeyMap = map;
        memmove((char *)(*k)->modifierKeyMap, (char *)inputMap, inputMapLen);
    } else
	(*k)->modifierKeyMap = NULL;

    (*k)->maxKeysPerModifier = numKeyPerModifier;
    for (i = 0; i < MAP_LENGTH; i++)
        (*k)->modifierMap[i] = 0;
    for (i = 0; i < inputMapLen; i++) if (inputMap[i]) {
        (*k)->modifierMap[inputMap[i]]
          |= (1<<(i/ (*k)->maxKeysPerModifier));
    }

    return(MappingSuccess);
    }

void
SendDeviceMappingNotify(
    CARD8 request,
    KeyCode firstKeyCode,
    CARD8 count,
    DeviceIntPtr dev)
{
    xEvent event;
    deviceMappingNotify         *ev = (deviceMappingNotify *) &event;

    ev->type = DeviceMappingNotify;
    ev->request = request;
    ev->deviceid = dev->id;
    ev->time = currentTime.milliseconds;
    if (request == MappingKeyboard)
	{
	ev->firstKeyCode = firstKeyCode;
	ev->count = count;
	}

    SendEventToAllWindows (dev, DeviceMappingNotifyMask, (xEvent *)ev, 1);
    }

int
ChangeKeyMapping(
    ClientPtr 	client,
    DeviceIntPtr dev,
    unsigned 	len,
    int 	type,
    KeyCode 	firstKeyCode,
    CARD8 	keyCodes,
    CARD8 	keySymsPerKeyCode,
    KeySym	*map)
{
    KeySymsRec keysyms;
    KeyClassPtr k = dev->key;

    if (k == NULL)
	return (BadMatch);

    if (len != (keyCodes * keySymsPerKeyCode))
            return BadLength;

    if ((firstKeyCode < k->curKeySyms.minKeyCode) ||
	(firstKeyCode + keyCodes - 1 > k->curKeySyms.maxKeyCode))
    {
	    client->errorValue = firstKeyCode;
	    return BadValue;
    }
    if (keySymsPerKeyCode == 0)
    {
	    client->errorValue = 0;
            return BadValue;
    }
    keysyms.minKeyCode = firstKeyCode;
    keysyms.maxKeyCode = firstKeyCode + keyCodes - 1;
    keysyms.mapWidth = keySymsPerKeyCode;
    keysyms.map = map;
    if (!SetKeySymsMap(&k->curKeySyms, &keysyms))
	return BadAlloc;
    SendDeviceMappingNotify(MappingKeyboard, firstKeyCode, keyCodes,
	dev);
    return client->noClientException;
    }

void
DeleteWindowFromAnyExtEvents(pWin, freeResources)
    WindowPtr		pWin;
    Bool		freeResources;
    {
    int			i;
    DeviceIntPtr	dev;
    InputClientsPtr	ic;
    struct _OtherInputMasks *inputMasks;

    for (dev=inputInfo.devices; dev; dev=dev->next)
	{
	if (dev == inputInfo.pointer ||
	    dev == inputInfo.keyboard)
	    continue;
	DeleteDeviceFromAnyExtEvents(pWin, dev);
	}

    for (dev=inputInfo.off_devices; dev; dev=dev->next)
	DeleteDeviceFromAnyExtEvents(pWin, dev);

    if (freeResources)
	while ((inputMasks = wOtherInputMasks(pWin)) != 0)
	    {
	    ic = inputMasks->inputClients;
	    for (i=0; i<EMASKSIZE; i++)
		inputMasks->dontPropagateMask[i] = 0;
	    FreeResource(ic->resource, RT_NONE);
	    }
    }

void
DeleteDeviceFromAnyExtEvents(pWin, dev)
    WindowPtr		pWin;
    DeviceIntPtr	dev;
    {
    WindowPtr		parent;

    /* Deactivate any grabs performed on this window, before making
	any input focus changes.
        Deactivating a device grab should cause focus events. */

    if (dev->grab && (dev->grab->window == pWin))
	(*dev->DeactivateGrab)(dev);

    /* If the focus window is a root window (ie. has no parent) 
	then don't delete the focus from it. */
    
    if (dev->focus && (pWin==dev->focus->win) && (pWin->parent != NullWindow))
	{
	int focusEventMode = NotifyNormal;

 	/* If a grab is in progress, then alter the mode of focus events. */

	if (dev->grab)
	    focusEventMode = NotifyWhileGrabbed;

	switch (dev->focus->revert)
	    {
	    case RevertToNone:
		DoFocusEvents(dev, pWin, NoneWin, focusEventMode);
		dev->focus->win = NoneWin;
		dev->focus->traceGood = 0;
		break;
	    case RevertToParent:
		parent = pWin;
		do
		    {
		    parent = parent->parent;
		    dev->focus->traceGood--;
		    } while (!parent->realized);
		DoFocusEvents(dev, pWin, parent, focusEventMode);
		dev->focus->win = parent;
		dev->focus->revert = RevertToNone;
		break;
	    case RevertToPointerRoot:
		DoFocusEvents(dev, pWin, PointerRootWin, focusEventMode);
		dev->focus->win = PointerRootWin;
		dev->focus->traceGood = 0;
		break;
	    case RevertToFollowKeyboard:
		if (inputInfo.keyboard->focus->win) {
		    DoFocusEvents(dev, pWin, inputInfo.keyboard->focus->win,
				  focusEventMode);
		    dev->focus->win = FollowKeyboardWin;
		    dev->focus->traceGood = 0;
		} else {
		    DoFocusEvents(dev, pWin, NoneWin, focusEventMode);
		    dev->focus->win = NoneWin;
		    dev->focus->traceGood = 0;
		}
		break;
	    }
	}

    if (dev->valuator)
	if (dev->valuator->motionHintWindow == pWin)
	    dev->valuator->motionHintWindow = NullWindow;
    }

int
MaybeSendDeviceMotionNotifyHint (pEvents, mask)
    deviceKeyButtonPointer *pEvents;
    Mask mask;
    {
    DeviceIntPtr dev;

    dev = LookupDeviceIntRec (pEvents->deviceid & DEVICE_BITS);
    if (pEvents->type == DeviceMotionNotify)
	{
	if (mask & DevicePointerMotionHintMask)
	    {
	    if (WID(dev->valuator->motionHintWindow) == pEvents->event)
		{
		return 1; /* don't send, but pretend we did */
		}
	    pEvents->detail = NotifyHint;
	    }
	 else
	    {
	    pEvents->detail = NotifyNormal;
	    }
	}
    return (0);
    }

void
CheckDeviceGrabAndHintWindow (pWin, type, xE, grab, client, deliveryMask)
    WindowPtr pWin;
    int type;
    deviceKeyButtonPointer *xE;
    GrabPtr grab;
    ClientPtr client;
    Mask deliveryMask;
    {
    DeviceIntPtr dev;

    dev = LookupDeviceIntRec (xE->deviceid & DEVICE_BITS);
    if (type == DeviceMotionNotify)
	dev->valuator->motionHintWindow = pWin;
    else if ((type == DeviceButtonPress) && (!grab) && 
	(deliveryMask & DeviceButtonGrabMask))
        {
	GrabRec tempGrab;

	tempGrab.device = dev;
	tempGrab.resource = client->clientAsMask;
	tempGrab.window = pWin;
	tempGrab.ownerEvents = (deliveryMask & DeviceOwnerGrabButtonMask) ? TRUE : FALSE;
	tempGrab.eventMask = deliveryMask;
	tempGrab.keyboardMode = GrabModeAsync;
	tempGrab.pointerMode = GrabModeAsync;
	tempGrab.confineTo = NullWindow;
	tempGrab.cursor = NullCursor;
	(*dev->ActivateGrab)(dev, &tempGrab, currentTime, TRUE);
        }
    }

Mask
DeviceEventMaskForClient(dev, pWin, client)
    DeviceIntPtr	dev;
    WindowPtr		pWin;
    ClientPtr		client;
    {
    register InputClientsPtr other;

    if (!wOtherInputMasks(pWin))
	return 0;
    for (other = wOtherInputMasks(pWin)->inputClients; other; 
	other = other->next)
	{
	if (SameClient(other, client))
	    return other->mask[dev->id];
	}
    return 0;
    }

void
MaybeStopDeviceHint(dev, client)
    register DeviceIntPtr dev;
    ClientPtr client;
{
    WindowPtr pWin;
    GrabPtr grab = dev->grab;
    pWin = dev->valuator->motionHintWindow;

    if ((grab && SameClient(grab, client) &&
	 ((grab->eventMask & DevicePointerMotionHintMask) ||
	  (grab->ownerEvents &&
	   (DeviceEventMaskForClient(dev, pWin, client) &
	    DevicePointerMotionHintMask)))) ||
	(!grab &&
	 (DeviceEventMaskForClient(dev, pWin, client) &
	  DevicePointerMotionHintMask)))
	dev->valuator->motionHintWindow = NullWindow;
}

int
DeviceEventSuppressForWindow(pWin, client, mask, maskndx)
	WindowPtr pWin;
	ClientPtr client;
	Mask mask;
	int maskndx;
    {
    struct _OtherInputMasks *inputMasks = wOtherInputMasks (pWin);

    if (mask & ~PropagateMask[maskndx])
	{
	client->errorValue = mask;
	return BadValue;
	}

    if (mask == 0) 
	{
	if (inputMasks)
	    inputMasks->dontPropagateMask[maskndx] = mask;
	} 
    else 
	{
	if (!inputMasks)
	    AddExtensionClient (pWin, client, 0, 0);
	inputMasks = wOtherInputMasks(pWin);
	inputMasks->dontPropagateMask[maskndx] = mask;
	}
    RecalculateDeviceDeliverableEvents(pWin);
    if (ShouldFreeInputMasks(pWin, FALSE))
        FreeResource(inputMasks->inputClients->resource, RT_NONE);
    return Success;
    }

static Bool
ShouldFreeInputMasks (pWin, ignoreSelectedEvents)
    WindowPtr pWin;
    Bool ignoreSelectedEvents;
    {
    int i;
    Mask allInputEventMasks = 0;
    struct _OtherInputMasks *inputMasks = wOtherInputMasks (pWin);

    for (i=0; i<EMASKSIZE; i++)
	allInputEventMasks |= inputMasks->dontPropagateMask[i];
    if (!ignoreSelectedEvents)
	for (i=0; i<EMASKSIZE; i++)
	    allInputEventMasks |= inputMasks->inputEvents[i];
    if (allInputEventMasks == 0)
	return TRUE;
    else
	return FALSE;
    }
