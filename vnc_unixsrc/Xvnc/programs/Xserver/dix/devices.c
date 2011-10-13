/* $XFree86: xc/programs/Xserver/dix/devices.c,v 3.20 2001/12/14 19:59:30 dawes Exp $ */
/************************************************************

Copyright 1987, 1998  The Open Group

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


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/


/* $Xorg: devices.c,v 1.4 2001/02/09 02:04:39 xorgcvs Exp $ */
/* $XdotOrg: xc/programs/Xserver/dix/devices.c,v 1.8 2005/07/03 08:53:38 daniels Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include "misc.h"
#include "resource.h"
#define NEED_EVENTS
#define NEED_REPLIES
#include <X11/Xproto.h>
#include "windowstr.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "dixstruct.h"
#include "site.h"
#define	XKB_IN_SERVER
#ifdef XKB
#include <X11/extensions/XKBsrv.h>
#endif
#ifdef XCSECURITY
#define _SECURITY_SERVER
#include <X11/extensions/security.h>
#endif
#ifdef LBX
#include "lbxserve.h"
#endif

#include "dispatch.h"
#include "swaprep.h"
#include "dixevents.h"

DeviceIntPtr
_AddInputDevice(DeviceProc deviceProc, Bool autoStart)
{
    register DeviceIntPtr dev;

    if (inputInfo.numDevices >= MAX_DEVICES)
	return (DeviceIntPtr)NULL;
    dev = (DeviceIntPtr) xalloc(sizeof(DeviceIntRec));
    if (!dev)
	return (DeviceIntPtr)NULL;
    dev->name = (char *)NULL;
    dev->type = 0;
    dev->id = inputInfo.numDevices;
    inputInfo.numDevices++;
    dev->public.on = FALSE;
    dev->public.processInputProc = (ProcessInputProc)NoopDDA;
    dev->public.realInputProc = (ProcessInputProc)NoopDDA;
    dev->public.enqueueInputProc = EnqueueEvent;
    dev->deviceProc = deviceProc;
    dev->startup = autoStart;
    dev->sync.frozen = FALSE;
    dev->sync.other = NullGrab;
    dev->sync.state = NOT_GRABBED;
    dev->sync.event = (xEvent *) NULL;
    dev->sync.evcount = 0;
    dev->grab = NullGrab;
    dev->grabTime = currentTime;
    dev->fromPassiveGrab = FALSE;
    dev->key = (KeyClassPtr)NULL;
    dev->valuator = (ValuatorClassPtr)NULL;
    dev->button = (ButtonClassPtr)NULL;
    dev->focus = (FocusClassPtr)NULL;
    dev->proximity = (ProximityClassPtr)NULL;
    dev->kbdfeed = (KbdFeedbackPtr)NULL;
    dev->ptrfeed = (PtrFeedbackPtr)NULL;
    dev->intfeed = (IntegerFeedbackPtr)NULL;
    dev->stringfeed = (StringFeedbackPtr)NULL;
    dev->bell = (BellFeedbackPtr)NULL;
    dev->leds = (LedFeedbackPtr)NULL;
    dev->next = inputInfo.off_devices;
#ifdef XKB
    dev->xkb_interest= NULL;
#endif
    dev->nPrivates = 0;
    dev->devPrivates = dev->unwrapProc = NULL;
    inputInfo.off_devices = dev;
    return dev;
}

Bool
EnableDevice(register DeviceIntPtr dev)
{
    register DeviceIntPtr *prev;

    for (prev = &inputInfo.off_devices;
	 *prev && (*prev != dev);
	 prev = &(*prev)->next)
	;
    if ((*prev != dev) || !dev->inited ||
	((*dev->deviceProc)(dev, DEVICE_ON) != Success))
	return FALSE;
    *prev = dev->next;
    dev->next = inputInfo.devices;
    inputInfo.devices = dev;
    return TRUE;
}

Bool
DisableDevice(register DeviceIntPtr dev)
{
    register DeviceIntPtr *prev;

    for (prev = &inputInfo.devices;
	 *prev && (*prev != dev);
	 prev = &(*prev)->next)
	;
    if (*prev != dev)
	return FALSE;
    (void)(*dev->deviceProc)(dev, DEVICE_OFF);
    *prev = dev->next;
    dev->next = inputInfo.off_devices;
    inputInfo.off_devices = dev;
    return TRUE;
}

int
InitAndStartDevices()
{
    register DeviceIntPtr dev, next;

    for (dev = inputInfo.off_devices; dev; dev = dev->next)
	dev->inited = ((*dev->deviceProc)(dev, DEVICE_INIT) == Success);
    for (dev = inputInfo.off_devices; dev; dev = next)
    {
	next = dev->next;
	if (dev->inited && dev->startup)
	    (void)EnableDevice(dev);
    }
    for (dev = inputInfo.devices;
	 dev && (dev != inputInfo.keyboard);
	 dev = dev->next)
	;
    if (!dev || (dev != inputInfo.keyboard)) {
	ErrorF("No core keyboard\n");
	return BadImplementation;
    }
    for (dev = inputInfo.devices;
	 dev && (dev != inputInfo.pointer);
	 dev = dev->next)
	;
    if (!dev || (dev != inputInfo.pointer)) {
	ErrorF("No core pointer\n");
	return BadImplementation;
    }
    return Success;
}

static void
CloseDevice(register DeviceIntPtr dev)
{
    KbdFeedbackPtr k, knext;
    PtrFeedbackPtr p, pnext;
    IntegerFeedbackPtr i, inext;
    StringFeedbackPtr s, snext;
    BellFeedbackPtr b, bnext;
    LedFeedbackPtr l, lnext;

    if (dev->inited)
	(void)(*dev->deviceProc)(dev, DEVICE_CLOSE);
    xfree(dev->name);
    if (dev->key)
    {
#ifdef XKB
	if (dev->key->xkbInfo)
	    XkbFreeInfo(dev->key->xkbInfo);
#endif
	xfree(dev->key->curKeySyms.map);
	xfree(dev->key->modifierKeyMap);
	xfree(dev->key);
    }
    xfree(dev->valuator);
#ifdef XKB
    if ((dev->button)&&(dev->button->xkb_acts))
	xfree(dev->button->xkb_acts);
#endif
    xfree(dev->button);
    if (dev->focus)
    {
	xfree(dev->focus->trace);
	xfree(dev->focus);
    }
    xfree(dev->proximity);
    for (k=dev->kbdfeed; k; k=knext)
    {
	knext = k->next;
#ifdef XKB
	if (k->xkb_sli)
	    XkbFreeSrvLedInfo(k->xkb_sli);
#endif
	xfree(k);
    }
    for (p=dev->ptrfeed; p; p=pnext)
    {
	pnext = p->next;
	xfree(p);
    }
    for (i=dev->intfeed; i; i=inext)
    {
	inext = i->next;
	xfree(i);
    }
    for (s=dev->stringfeed; s; s=snext)
    {
	snext = s->next;
	xfree(s->ctrl.symbols_supported);
	xfree(s->ctrl.symbols_displayed);
	xfree(s);
    }
    for (b=dev->bell; b; b=bnext)
    {
	bnext = b->next;
	xfree(b);
    }
    for (l=dev->leds; l; l=lnext)
    {
	lnext = l->next;
#ifdef XKB
	if (l->xkb_sli)
	    XkbFreeSrvLedInfo(l->xkb_sli);
#endif
	xfree(l);
    }
#ifdef XKB
    while (dev->xkb_interest) {
	XkbRemoveResourceClient((DevicePtr)dev,dev->xkb_interest->resource);
    }
#endif
    xfree(dev->sync.event);
    xfree(dev);
}

void
CloseDownDevices()
{
    register DeviceIntPtr dev, next;

    for (dev = inputInfo.devices; dev; dev = next)
    {
	next = dev->next;
	CloseDevice(dev);
    }
    for (dev = inputInfo.off_devices; dev; dev = next)
    {
	next = dev->next;
	CloseDevice(dev);
    }
    inputInfo.devices = NULL;
    inputInfo.off_devices = NULL;
    inputInfo.keyboard = NULL;
    inputInfo.pointer = NULL;
}

void
RemoveDevice(register DeviceIntPtr dev)
{
    register DeviceIntPtr prev,tmp,next;

    prev= NULL;
    for (tmp= inputInfo.devices; tmp; (prev = tmp), (tmp = next)) {
	next = tmp->next;
	if (tmp==dev) {
	    CloseDevice(tmp);
	    if (prev==NULL)
		inputInfo.devices = next;
	    else
		prev->next = next;
	    inputInfo.numDevices--;
	    if (inputInfo.keyboard == tmp)
	        inputInfo.keyboard = NULL;
	    else if (inputInfo.pointer == tmp)
	        inputInfo.pointer = NULL;
	    return;
	}
    }

    prev= NULL;
    for (tmp= inputInfo.off_devices; tmp; (prev = tmp), (tmp = next)) {
	next = tmp->next;
	if (tmp==dev) {
	    CloseDevice(tmp);
	    if (prev==NULL)
		inputInfo.off_devices = next;
	    else
		prev->next = next;
	    inputInfo.numDevices--;
	    if (inputInfo.keyboard == tmp)
	        inputInfo.keyboard = NULL;
	    else if (inputInfo.pointer == tmp)
	        inputInfo.pointer = NULL;
	    return;
	}
    }
    ErrorF("Internal Error! Attempt to remove a non-existent device\n");
    return;
}

int
NumMotionEvents()
{
    return inputInfo.pointer->valuator->numMotionEvents;
}

void
_RegisterPointerDevice(DeviceIntPtr device)
{
    inputInfo.pointer = device;
#ifdef XKB
    device->public.processInputProc = CoreProcessPointerEvent;
    device->public.realInputProc = CoreProcessPointerEvent;
    if (!noXkbExtension)
       XkbSetExtension(device,ProcessPointerEvent);
#else
    device->public.processInputProc = ProcessPointerEvent;
    device->public.realInputProc = ProcessPointerEvent;
#endif
    device->ActivateGrab = ActivatePointerGrab;
    device->DeactivateGrab = DeactivatePointerGrab;
    if (!device->name)
    {
	char *p = "pointer";
	device->name = (char *)xalloc(strlen(p) + 1);
	strcpy(device->name, p);
    }
}

void
_RegisterKeyboardDevice(DeviceIntPtr device)
{
    inputInfo.keyboard = device;
#ifdef XKB
    device->public.processInputProc = CoreProcessKeyboardEvent;
    device->public.realInputProc = CoreProcessKeyboardEvent;
    if (!noXkbExtension)
       XkbSetExtension(device,ProcessKeyboardEvent);
#else
    device->public.processInputProc = ProcessKeyboardEvent;
    device->public.realInputProc = ProcessKeyboardEvent;
#endif
    device->ActivateGrab = ActivateKeyboardGrab;
    device->DeactivateGrab = DeactivateKeyboardGrab;
    if (!device->name)
    {
	char *k = "keyboard";
	device->name = (char *)xalloc(strlen(k) + 1);
	strcpy(device->name, k);
    }
}

DevicePtr
LookupKeyboardDevice()
{
    return inputInfo.keyboard ? &inputInfo.keyboard->public : NULL;
}

DevicePtr
LookupPointerDevice()
{
    return inputInfo.pointer ? &inputInfo.pointer->public : NULL;
}

DevicePtr
LookupDevice(int id)
{
    DeviceIntPtr dev;

    for (dev=inputInfo.devices; dev; dev=dev->next) {
        if (dev->id == (CARD8)id)
            return (DevicePtr)dev;
    }
    for (dev=inputInfo.off_devices; dev; dev=dev->next) {
        if (dev->id == (CARD8)id)
            return (DevicePtr)dev;
    }
    return NULL;
}

void
QueryMinMaxKeyCodes(KeyCode *minCode, KeyCode *maxCode)
{
    if (inputInfo.keyboard) {
	*minCode = inputInfo.keyboard->key->curKeySyms.minKeyCode;
	*maxCode = inputInfo.keyboard->key->curKeySyms.maxKeyCode;
    }
}

Bool
SetKeySymsMap(register KeySymsPtr dst, register KeySymsPtr src)
{
    int i, j;
    int rowDif = src->minKeyCode - dst->minKeyCode;
           /* if keysym map size changes, grow map first */

    if (src->mapWidth < dst->mapWidth)
    {
        for (i = src->minKeyCode; i <= src->maxKeyCode; i++)
	{
#define SI(r, c) (((r-src->minKeyCode)*src->mapWidth) + (c))
#define DI(r, c) (((r - dst->minKeyCode)*dst->mapWidth) + (c))
	    for (j = 0; j < src->mapWidth; j++)
		dst->map[DI(i, j)] = src->map[SI(i, j)];
	    for (j = src->mapWidth; j < dst->mapWidth; j++)
		dst->map[DI(i, j)] = NoSymbol;
#undef SI
#undef DI
	}
	return TRUE;
    }
    else if (src->mapWidth > dst->mapWidth)
    {
        KeySym *map;
	int bytes = sizeof(KeySym) * src->mapWidth *
		    (dst->maxKeyCode - dst->minKeyCode + 1);
        map = (KeySym *)xalloc(bytes);
	if (!map)
	    return FALSE;
	bzero((char *)map, bytes);
        if (dst->map)
	{
            for (i = 0; i <= dst->maxKeyCode-dst->minKeyCode; i++)
		memmove((char *)&map[i*src->mapWidth],
			(char *)&dst->map[i*dst->mapWidth],
		      dst->mapWidth * sizeof(KeySym));
	    xfree(dst->map);
	}
	dst->mapWidth = src->mapWidth;
	dst->map = map;
    }
    memmove((char *)&dst->map[rowDif * dst->mapWidth],
	    (char *)src->map,
	  (int)(src->maxKeyCode - src->minKeyCode + 1) *
	  dst->mapWidth * sizeof(KeySym));
    return TRUE;
}

static Bool
InitModMap(register KeyClassPtr keyc)
{
    int i, j;
    CARD8 keysPerModifier[8];
    CARD8 mask;

    keyc->maxKeysPerModifier = 0;
    for (i = 0; i < 8; i++)
	keysPerModifier[i] = 0;
    for (i = 8; i < MAP_LENGTH; i++)
    {
	for (j = 0, mask = 1; j < 8; j++, mask <<= 1)
	{
	    if (mask & keyc->modifierMap[i])
	    {
		if (++keysPerModifier[j] > keyc->maxKeysPerModifier)
		    keyc->maxKeysPerModifier = keysPerModifier[j];
	    }
	}
    }
    keyc->modifierKeyMap = (KeyCode *)xalloc(8*keyc->maxKeysPerModifier);
    if (!keyc->modifierKeyMap && keyc->maxKeysPerModifier)
	return (FALSE);
    bzero((char *)keyc->modifierKeyMap, 8*(int)keyc->maxKeysPerModifier);
    for (i = 0; i < 8; i++)
	keysPerModifier[i] = 0;
    for (i = 8; i < MAP_LENGTH; i++)
    {
	for (j = 0, mask = 1; j < 8; j++, mask <<= 1)
	{
	    if (mask & keyc->modifierMap[i])
	    {
		keyc->modifierKeyMap[(j*keyc->maxKeysPerModifier) +
				     keysPerModifier[j]] = i;
		keysPerModifier[j]++;
	    }
	}
    }
    return TRUE;
}

Bool
InitKeyClassDeviceStruct(DeviceIntPtr dev, KeySymsPtr pKeySyms, CARD8 pModifiers[])
{
    int i;
    register KeyClassPtr keyc;

    keyc = (KeyClassPtr)xalloc(sizeof(KeyClassRec));
    if (!keyc)
	return FALSE;
    keyc->curKeySyms.map = (KeySym *)NULL;
    keyc->curKeySyms.mapWidth = 0;
    keyc->curKeySyms.minKeyCode = pKeySyms->minKeyCode;
    keyc->curKeySyms.maxKeyCode = pKeySyms->maxKeyCode;
    keyc->modifierKeyMap = (KeyCode *)NULL;
    keyc->state = 0;
    keyc->prev_state = 0;
    if (pModifiers)
	memmove((char *)keyc->modifierMap, (char *)pModifiers, MAP_LENGTH);
    else
	bzero((char *)keyc->modifierMap, MAP_LENGTH);
    bzero((char *)keyc->down, DOWN_LENGTH);
    for (i = 0; i < 8; i++)
	keyc->modifierKeyCount[i] = 0;
    if (!SetKeySymsMap(&keyc->curKeySyms, pKeySyms) || !InitModMap(keyc))
    {
	xfree(keyc->curKeySyms.map);
	xfree(keyc->modifierKeyMap);
	xfree(keyc);
	return FALSE;
    }
    dev->key = keyc;
#ifdef XKB
    dev->key->xkbInfo= NULL;
    if (!noXkbExtension) XkbInitDevice(dev);
#endif
    return TRUE;
}

Bool
InitButtonClassDeviceStruct(register DeviceIntPtr dev, int numButtons, 
                            CARD8 *map)
{
    register ButtonClassPtr butc;
    int i;

    butc = (ButtonClassPtr)xalloc(sizeof(ButtonClassRec));
    if (!butc)
	return FALSE;
    butc->numButtons = numButtons;
    for (i = 1; i <= numButtons; i++)
	butc->map[i] = map[i];
    butc->buttonsDown = 0;
    butc->state = 0;
    butc->motionMask = 0;
    bzero((char *)butc->down, DOWN_LENGTH);
#ifdef XKB
    butc->xkb_acts=	NULL;
#endif
    dev->button = butc;
    return TRUE;
}

Bool
InitValuatorClassDeviceStruct(DeviceIntPtr dev, int numAxes, 
                              ValuatorMotionProcPtr motionProc, 
                              int numMotionEvents, int mode)
{
    int i;
    register ValuatorClassPtr valc;

    valc = (ValuatorClassPtr)xalloc(sizeof(ValuatorClassRec) +
				    numAxes * sizeof(AxisInfo) +
				    numAxes * sizeof(unsigned int));
    if (!valc)
	return FALSE;
    valc->GetMotionProc = motionProc;
    valc->numMotionEvents = numMotionEvents;
    valc->motionHintWindow = NullWindow;
    valc->numAxes = numAxes;
    valc->mode = mode;
    valc->axes = (AxisInfoPtr)(valc + 1);
    valc->axisVal = (int *)(valc->axes + numAxes);
    for (i=0; i<numAxes; i++)
	valc->axisVal[i]=0;
    dev->valuator = valc;
    return TRUE;
}

Bool
InitFocusClassDeviceStruct(DeviceIntPtr dev)
{
    register FocusClassPtr focc;

    focc = (FocusClassPtr)xalloc(sizeof(FocusClassRec));
    if (!focc)
	return FALSE;
    focc->win = PointerRootWin;
    focc->revert = None;
    focc->time = currentTime;
    focc->trace = (WindowPtr *)NULL;
    focc->traceSize = 0;
    focc->traceGood = 0;
    dev->focus = focc;
    return TRUE;
}

Bool
InitKbdFeedbackClassDeviceStruct(DeviceIntPtr dev, BellProcPtr bellProc, 
                                 KbdCtrlProcPtr controlProc)
{
    register KbdFeedbackPtr feedc;

    feedc = (KbdFeedbackPtr)xalloc(sizeof(KbdFeedbackClassRec));
    if (!feedc)
	return FALSE;
    feedc->BellProc = bellProc;
    feedc->CtrlProc = controlProc;
#ifdef XKB
    defaultKeyboardControl.autoRepeat = TRUE;
#endif
    feedc->ctrl = defaultKeyboardControl;
    feedc->ctrl.id = 0;
    if ((feedc->next = dev->kbdfeed) != 0)
	feedc->ctrl.id = dev->kbdfeed->ctrl.id + 1;
    dev->kbdfeed = feedc;
#ifdef XKB
    feedc->xkb_sli= NULL;
    if (!noXkbExtension)
	XkbFinishDeviceInit(dev);
#endif
    (*dev->kbdfeed->CtrlProc)(dev,&dev->kbdfeed->ctrl);
    return TRUE;
}

Bool
InitPtrFeedbackClassDeviceStruct(DeviceIntPtr dev, PtrCtrlProcPtr controlProc)
{
    register PtrFeedbackPtr feedc;

    feedc = (PtrFeedbackPtr)xalloc(sizeof(PtrFeedbackClassRec));
    if (!feedc)
	return FALSE;
    feedc->CtrlProc = controlProc;
#ifdef sgi
    feedc->ctrl.num = 1;
    feedc->ctrl.den = 1;
    feedc->ctrl.threshold = 1;
#else
    feedc->ctrl = defaultPointerControl;
#endif
    feedc->ctrl.id = 0;
    if ( (feedc->next = dev->ptrfeed) )
        feedc->ctrl.id = dev->ptrfeed->ctrl.id + 1;
    dev->ptrfeed = feedc;
    (*controlProc)(dev, &feedc->ctrl);
    return TRUE;
}


LedCtrl defaultLedControl = {
	DEFAULT_LEDS, DEFAULT_LEDS_MASK, 0};

BellCtrl defaultBellControl = {
	DEFAULT_BELL,
	DEFAULT_BELL_PITCH,
	DEFAULT_BELL_DURATION,
	0};

IntegerCtrl defaultIntegerControl = {
	DEFAULT_INT_RESOLUTION,
	DEFAULT_INT_MIN_VALUE,
	DEFAULT_INT_MAX_VALUE,
	DEFAULT_INT_DISPLAYED,
	0};

Bool
InitStringFeedbackClassDeviceStruct (
      DeviceIntPtr dev, StringCtrlProcPtr controlProc, 
      int max_symbols, int num_symbols_supported, KeySym *symbols)
{
    int i;
    register StringFeedbackPtr feedc;

    feedc = (StringFeedbackPtr)xalloc(sizeof(StringFeedbackClassRec));
    if (!feedc)
	return FALSE;
    feedc->CtrlProc = controlProc;
    feedc->ctrl.num_symbols_supported = num_symbols_supported;
    feedc->ctrl.num_symbols_displayed = 0;
    feedc->ctrl.max_symbols = max_symbols;
    feedc->ctrl.symbols_supported = (KeySym *) 
	xalloc (sizeof (KeySym) * num_symbols_supported);
    feedc->ctrl.symbols_displayed = (KeySym *) 
	xalloc (sizeof (KeySym) * max_symbols);
    if (!feedc->ctrl.symbols_supported || !feedc->ctrl.symbols_displayed)
    {
	if (feedc->ctrl.symbols_supported)
	    xfree(feedc->ctrl.symbols_supported);
	if (feedc->ctrl.symbols_displayed)
	    xfree(feedc->ctrl.symbols_displayed);
	xfree(feedc);
	return FALSE;
    }
    for (i=0; i<num_symbols_supported; i++)
	*(feedc->ctrl.symbols_supported+i) = *symbols++;
    for (i=0; i<max_symbols; i++)
	*(feedc->ctrl.symbols_displayed+i) = (KeySym) NULL;
    feedc->ctrl.id = 0;
    if ( (feedc->next = dev->stringfeed) )
	feedc->ctrl.id = dev->stringfeed->ctrl.id + 1;
    dev->stringfeed = feedc;
    (*controlProc)(dev, &feedc->ctrl);
    return TRUE;
}

Bool
InitBellFeedbackClassDeviceStruct (DeviceIntPtr dev, BellProcPtr bellProc, 
                                   BellCtrlProcPtr controlProc)
{
    register BellFeedbackPtr feedc;

    feedc = (BellFeedbackPtr)xalloc(sizeof(BellFeedbackClassRec));
    if (!feedc)
	return FALSE;
    feedc->CtrlProc = controlProc;
    feedc->BellProc = bellProc;
    feedc->ctrl = defaultBellControl;
    feedc->ctrl.id = 0;
    if ( (feedc->next = dev->bell) )
	feedc->ctrl.id = dev->bell->ctrl.id + 1;
    dev->bell = feedc;
    (*controlProc)(dev, &feedc->ctrl);
    return TRUE;
}

Bool
InitLedFeedbackClassDeviceStruct (DeviceIntPtr dev, LedCtrlProcPtr controlProc)
{
    register LedFeedbackPtr feedc;

    feedc = (LedFeedbackPtr)xalloc(sizeof(LedFeedbackClassRec));
    if (!feedc)
	return FALSE;
    feedc->CtrlProc = controlProc;
    feedc->ctrl = defaultLedControl;
    feedc->ctrl.id = 0;
    if ( (feedc->next = dev->leds) )
	feedc->ctrl.id = dev->leds->ctrl.id + 1;
#ifdef XKB
    feedc->xkb_sli= NULL;
#endif
    dev->leds = feedc;
    (*controlProc)(dev, &feedc->ctrl);
    return TRUE;
}

Bool
InitIntegerFeedbackClassDeviceStruct (DeviceIntPtr dev, IntegerCtrlProcPtr controlProc)
{
    register IntegerFeedbackPtr feedc;

    feedc = (IntegerFeedbackPtr)xalloc(sizeof(IntegerFeedbackClassRec));
    if (!feedc)
	return FALSE;
    feedc->CtrlProc = controlProc;
    feedc->ctrl = defaultIntegerControl;
    feedc->ctrl.id = 0;
    if ( (feedc->next = dev->intfeed) )
	feedc->ctrl.id = dev->intfeed->ctrl.id + 1;
    dev->intfeed = feedc;
    (*controlProc)(dev, &feedc->ctrl);
    return TRUE;
}

Bool
InitPointerDeviceStruct(DevicePtr device, CARD8 *map, int numButtons, 
                        ValuatorMotionProcPtr motionProc, 
                        PtrCtrlProcPtr controlProc, int numMotionEvents)
{
    DeviceIntPtr dev = (DeviceIntPtr)device;

    return(InitButtonClassDeviceStruct(dev, numButtons, map) &&
	   InitValuatorClassDeviceStruct(dev, 2, motionProc,
					 numMotionEvents, 0) &&
	   InitPtrFeedbackClassDeviceStruct(dev, controlProc));
}

Bool
InitKeyboardDeviceStruct(DevicePtr device, KeySymsPtr pKeySyms, 
                         CARD8 pModifiers[], BellProcPtr bellProc, 
                         KbdCtrlProcPtr controlProc) 
{
    DeviceIntPtr dev = (DeviceIntPtr)device;

    return(InitKeyClassDeviceStruct(dev, pKeySyms, pModifiers) &&
	   InitFocusClassDeviceStruct(dev) &&
	   InitKbdFeedbackClassDeviceStruct(dev, bellProc, controlProc));
}

void
SendMappingNotify(unsigned request, unsigned firstKeyCode, unsigned count, 
                  ClientPtr client)
{
    int i;
    xEvent event;

    event.u.u.type = MappingNotify;
    event.u.mappingNotify.request = request;
    if (request == MappingKeyboard)
    {
        event.u.mappingNotify.firstKeyCode = firstKeyCode;
        event.u.mappingNotify.count = count;
    }
#ifdef XKB
    if (!noXkbExtension &&
	((request == MappingKeyboard) || (request == MappingModifier))) {
	XkbApplyMappingChange(inputInfo.keyboard,request,firstKeyCode,count,
									client);
    }
#endif

   /* 0 is the server client */
    for (i=1; i<currentMaxClients; i++)
    {
	if (clients[i] && clients[i]->clientState == ClientStateRunning)
	{
#ifdef XKB
	    if (!noXkbExtension &&
		(request == MappingKeyboard) &&
		(clients[i]->xkbClientFlags != 0) &&
		(clients[i]->mapNotifyMask&XkbKeySymsMask))
		continue;
#endif
	    event.u.u.sequenceNumber = clients[i]->sequence;
	    WriteEventsToClient(clients[i], 1, &event);
	}
    }
}

/*
 * n-squared algorithm. n < 255 and don't want to copy the whole thing and
 * sort it to do the checking. How often is it called? Just being lazy?
 */
Bool
BadDeviceMap(register BYTE *buff, int length, unsigned low, unsigned high, XID *errval)
{
    register int     i, j;

    for (i = 0; i < length; i++)
	if (buff[i])		       /* only check non-zero elements */
	{
	    if ((low > buff[i]) || (high < buff[i]))
	    {
		*errval = buff[i];
		return TRUE;
	    }
	    for (j = i + 1; j < length; j++)
		if (buff[i] == buff[j])
		{
		    *errval = buff[i];
		    return TRUE;
		}
	}
    return FALSE;
}

Bool
AllModifierKeysAreUp(dev, map1, per1, map2, per2)
    register DeviceIntPtr dev;
    register CARD8 *map1, *map2;
    int per1, per2;
{
    register int i, j, k;
    register CARD8 *down = dev->key->down;

    for (i = 8; --i >= 0; map2 += per2)
    {
	for (j = per1; --j >= 0; map1++)
	{
	    if (*map1 && BitIsOn(down, *map1))
	    {
		for (k = per2; (--k >= 0) && (*map1 != map2[k]);)
		  ;
		if (k < 0)
		    return FALSE;
	    }
	}
    }
    return TRUE;
}

int 
ProcSetModifierMapping(ClientPtr client)
{
    xSetModifierMappingReply rep;
    REQUEST(xSetModifierMappingReq);
    KeyCode *inputMap;
    int inputMapLen;
    register int i;
    DeviceIntPtr keybd = inputInfo.keyboard;
    register KeyClassPtr keyc = keybd->key;
    
    REQUEST_AT_LEAST_SIZE(xSetModifierMappingReq);

    if (client->req_len != ((stuff->numKeyPerModifier<<1) +
			    (sizeof (xSetModifierMappingReq)>>2)))
	return BadLength;

    inputMapLen = 8*stuff->numKeyPerModifier;
    inputMap = (KeyCode *)&stuff[1];

    /*
     *	Now enforce the restriction that "all of the non-zero keycodes must be
     *	in the range specified by min-keycode and max-keycode in the
     *	connection setup (else a Value error)"
     */
    i = inputMapLen;
    while (i--)
    {
	if (inputMap[i]
	    && (inputMap[i] < keyc->curKeySyms.minKeyCode
		|| inputMap[i] > keyc->curKeySyms.maxKeyCode))
	{
	    client->errorValue = inputMap[i];
	    return BadValue;
	}
    }

#ifdef XCSECURITY
    if (!SecurityCheckDeviceAccess(client, keybd, TRUE))
	return BadAccess;
#endif 

#ifdef LBX
    LbxFlushModifierMapTag();
#endif
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.success = MappingSuccess;

    /*
     *	Now enforce the restriction that none of the old or new
     *	modifier keys may be down while we change the mapping,  and
     *	that the DDX layer likes the choice.
     */
    if (!AllModifierKeysAreUp(keybd, keyc->modifierKeyMap,
			      (int)keyc->maxKeysPerModifier,
			      inputMap, (int)stuff->numKeyPerModifier)
	    ||
	!AllModifierKeysAreUp(keybd, inputMap, (int)stuff->numKeyPerModifier,
			      keyc->modifierKeyMap,
			      (int)keyc->maxKeysPerModifier))
    {
	rep.success = MappingBusy;
    }
    else
    {
	for (i = 0; i < inputMapLen; i++)
	{
	    if (inputMap[i] && !LegalModifier(inputMap[i], (DevicePtr)keybd))
	    {
		rep.success = MappingFailed;
		break;
	    }
	}
    }

    if (rep.success == MappingSuccess)
    {
	KeyCode *map;
	/*
	 *	Now build the keyboard's modifier bitmap from the
	 *	list of keycodes.
	 */
	map = (KeyCode *)xalloc(inputMapLen);
	if (!map && inputMapLen)
	    return BadAlloc;
	if (keyc->modifierKeyMap)
	    xfree(keyc->modifierKeyMap);
	keyc->modifierKeyMap = map;
	memmove((char *)map, (char *)inputMap, inputMapLen);

	keyc->maxKeysPerModifier = stuff->numKeyPerModifier;
	for (i = 0; i < MAP_LENGTH; i++)
	    keyc->modifierMap[i] = 0;
	for (i = 0; i < inputMapLen; i++)
	{
	    if (inputMap[i])
		keyc->modifierMap[inputMap[i]] |=
		    (1<<(((unsigned int)i)/keyc->maxKeysPerModifier));
	}
    }

    if (rep.success == MappingSuccess)
        SendMappingNotify(MappingModifier, 0, 0, client);

    WriteReplyToClient(client, sizeof(xSetModifierMappingReply), &rep);

    return(client->noClientException);
}

int
ProcGetModifierMapping(ClientPtr client)
{
    xGetModifierMappingReply rep;
    register KeyClassPtr keyc = inputInfo.keyboard->key;

    REQUEST_SIZE_MATCH(xReq);
    rep.type = X_Reply;
    rep.numKeyPerModifier = keyc->maxKeysPerModifier;
    rep.sequenceNumber = client->sequence;
    /* length counts 4 byte quantities - there are 8 modifiers 1 byte big */
    rep.length = keyc->maxKeysPerModifier << 1;

    WriteReplyToClient(client, sizeof(xGetModifierMappingReply), &rep);

    /* Use the (modified by DDX) map that SetModifierMapping passed in */
    (void)WriteToClient(client, (int)(keyc->maxKeysPerModifier << 3),
			(char *)keyc->modifierKeyMap);
    return client->noClientException;
}

int
ProcChangeKeyboardMapping(ClientPtr client)
{
    REQUEST(xChangeKeyboardMappingReq);
    unsigned len;
    KeySymsRec keysyms;
    register KeySymsPtr curKeySyms = &inputInfo.keyboard->key->curKeySyms;
    REQUEST_AT_LEAST_SIZE(xChangeKeyboardMappingReq);

    len = client->req_len - (sizeof(xChangeKeyboardMappingReq) >> 2);  
    if (len != (stuff->keyCodes * stuff->keySymsPerKeyCode))
            return BadLength;
    if ((stuff->firstKeyCode < curKeySyms->minKeyCode) ||
	(stuff->firstKeyCode > curKeySyms->maxKeyCode))
    {
	    client->errorValue = stuff->firstKeyCode;
	    return BadValue;
    }
    if ( ((unsigned)(stuff->firstKeyCode + stuff->keyCodes - 1) >
	  curKeySyms->maxKeyCode) ||
	(stuff->keySymsPerKeyCode == 0))
    {
	    client->errorValue = stuff->keySymsPerKeyCode;
	    return BadValue;
    }
#ifdef XCSECURITY
    if (!SecurityCheckDeviceAccess(client, inputInfo.keyboard,
				   TRUE))
	return BadAccess;
#endif 
    keysyms.minKeyCode = stuff->firstKeyCode;
    keysyms.maxKeyCode = stuff->firstKeyCode + stuff->keyCodes - 1;
    keysyms.mapWidth = stuff->keySymsPerKeyCode;
    keysyms.map = (KeySym *)&stuff[1];
    if (!SetKeySymsMap(curKeySyms, &keysyms))
	return BadAlloc;
#ifdef LBX
    LbxFlushKeyboardMapTag();
#endif
    SendMappingNotify(MappingKeyboard, stuff->firstKeyCode, stuff->keyCodes,
									client);
    return client->noClientException;

}

int
ProcSetPointerMapping(ClientPtr client)
{
    REQUEST(xSetPointerMappingReq);
    BYTE *map;
    xSetPointerMappingReply rep;
    register unsigned int i;
    DeviceIntPtr mouse = inputInfo.pointer;

    REQUEST_AT_LEAST_SIZE(xSetPointerMappingReq);
    if (client->req_len != (sizeof(xSetPointerMappingReq)+stuff->nElts+3) >> 2)
	return BadLength;
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.success = MappingSuccess;
    map = (BYTE *)&stuff[1];
    if (stuff->nElts != mouse->button->numButtons)
    {
	client->errorValue = stuff->nElts;
	return BadValue;
    }
    if (BadDeviceMap(&map[0], (int)stuff->nElts, 1, 255, &client->errorValue))
	return BadValue;
    for (i=0; i < stuff->nElts; i++)
	if ((mouse->button->map[i + 1] != map[i]) &&
	    BitIsOn(mouse->button->down, i + 1))
	{
    	    rep.success = MappingBusy;
	    WriteReplyToClient(client, sizeof(xSetPointerMappingReply), &rep);
            return Success;
	}
    for (i = 0; i < stuff->nElts; i++)
	mouse->button->map[i + 1] = map[i];
    SendMappingNotify(MappingPointer, 0, 0, client);
    WriteReplyToClient(client, sizeof(xSetPointerMappingReply), &rep);
    return Success;
}

int
ProcGetKeyboardMapping(ClientPtr client)
{
    xGetKeyboardMappingReply rep;
    REQUEST(xGetKeyboardMappingReq);
    KeySymsPtr curKeySyms = &inputInfo.keyboard->key->curKeySyms;

    REQUEST_SIZE_MATCH(xGetKeyboardMappingReq);

    if ((stuff->firstKeyCode < curKeySyms->minKeyCode) ||
        (stuff->firstKeyCode > curKeySyms->maxKeyCode))
    {
	client->errorValue = stuff->firstKeyCode;
	return BadValue;
    }
    if (stuff->firstKeyCode + stuff->count >
	(unsigned)(curKeySyms->maxKeyCode + 1))
    {
	client->errorValue = stuff->count;
        return BadValue;
    }

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.keySymsPerKeyCode = curKeySyms->mapWidth;
    /* length is a count of 4 byte quantities and KeySyms are 4 bytes */
    rep.length = (curKeySyms->mapWidth * stuff->count);
    WriteReplyToClient(client, sizeof(xGetKeyboardMappingReply), &rep);
    client->pSwapReplyFunc = (ReplySwapPtr) CopySwap32Write;
    WriteSwappedDataToClient(
	client,
	curKeySyms->mapWidth * stuff->count * sizeof(KeySym),
	&curKeySyms->map[(stuff->firstKeyCode - curKeySyms->minKeyCode) *
			 curKeySyms->mapWidth]);

    return client->noClientException;
}

int
ProcGetPointerMapping(ClientPtr client)
{
    xGetPointerMappingReply rep;
    ButtonClassPtr butc = inputInfo.pointer->button;

    REQUEST_SIZE_MATCH(xReq);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.nElts = butc->numButtons;
    rep.length = ((unsigned)rep.nElts + (4-1))/4;
    WriteReplyToClient(client, sizeof(xGetPointerMappingReply), &rep);
    (void)WriteToClient(client, (int)rep.nElts, (char *)&butc->map[1]);
    return Success;    
}

void
NoteLedState(DeviceIntPtr keybd, int led, Bool on)
{
    KeybdCtrl *ctrl = &keybd->kbdfeed->ctrl;
    if (on)
	ctrl->leds |= ((Leds)1 << (led - 1));
    else
	ctrl->leds &= ~((Leds)1 << (led - 1));
}

int
Ones(unsigned long mask)             /* HACKMEM 169 */
{
    register unsigned long y;

    y = (mask >> 1) &033333333333;
    y = mask - y - ((y >>1) & 033333333333);
    return (((y + (y >> 3)) & 030707070707) % 077);
}

int
ProcChangeKeyboardControl (ClientPtr client)
{
#define DO_ALL    (-1)
    KeybdCtrl ctrl;
    DeviceIntPtr keybd = inputInfo.keyboard;
    XID *vlist;
    int t;
    int led = DO_ALL;
    int key = DO_ALL;
    BITS32 vmask, index2;
    int mask, i;
    REQUEST(xChangeKeyboardControlReq);

    REQUEST_AT_LEAST_SIZE(xChangeKeyboardControlReq);
    vmask = stuff->mask;
    if (client->req_len != (sizeof(xChangeKeyboardControlReq)>>2)+Ones(vmask))
	return BadLength;
#ifdef XCSECURITY
    if (!SecurityCheckDeviceAccess(client, keybd, TRUE))
	return BadAccess;
#endif 
    vlist = (XID *)&stuff[1];		/* first word of values */
    ctrl = keybd->kbdfeed->ctrl;
    while (vmask)
    {
	index2 = (BITS32) lowbit (vmask);
	vmask &= ~index2;
	switch (index2)
	{
	case KBKeyClickPercent: 
	    t = (INT8)*vlist;
	    vlist++;
	    if (t == -1)
		t = defaultKeyboardControl.click;
	    else if (t < 0 || t > 100)
	    {
		client->errorValue = t;
		return BadValue;
	    }
	    ctrl.click = t;
	    break;
	case KBBellPercent:
	    t = (INT8)*vlist;
	    vlist++;
	    if (t == -1)
		t = defaultKeyboardControl.bell;
	    else if (t < 0 || t > 100)
	    {
		client->errorValue = t;
		return BadValue;
	    }
	    ctrl.bell = t;
	    break;
	case KBBellPitch:
	    t = (INT16)*vlist;
	    vlist++;
	    if (t == -1)
		t = defaultKeyboardControl.bell_pitch;
	    else if (t < 0)
	    {
		client->errorValue = t;
		return BadValue;
	    }
	    ctrl.bell_pitch = t;
	    break;
	case KBBellDuration:
	    t = (INT16)*vlist;
	    vlist++;
	    if (t == -1)
		t = defaultKeyboardControl.bell_duration;
	    else if (t < 0)
	    {
		client->errorValue = t;
		return BadValue;
	    }
	    ctrl.bell_duration = t;
	    break;
	case KBLed:
	    led = (CARD8)*vlist;
	    vlist++;
	    if (led < 1 || led > 32)
	    {
		client->errorValue = led;
		return BadValue;
	    }
	    if (!(stuff->mask & KBLedMode))
		return BadMatch;
	    break;
	case KBLedMode:
	    t = (CARD8)*vlist;
	    vlist++;
	    if (t == LedModeOff)
	    {
		if (led == DO_ALL)
		    ctrl.leds = 0x0;
		else
		    ctrl.leds &= ~(((Leds)(1)) << (led - 1));
	    }
	    else if (t == LedModeOn)
	    {
		if (led == DO_ALL)
		    ctrl.leds = ~0L;
		else
		    ctrl.leds |= (((Leds)(1)) << (led - 1));
	    }
	    else
	    {
		client->errorValue = t;
		return BadValue;
	    }
#ifdef XKB
	    if (!noXkbExtension) {
		XkbEventCauseRec	cause;
		XkbSetCauseCoreReq(&cause,X_ChangeKeyboardControl,client);
		XkbSetIndicators(keybd,((led == DO_ALL) ? ~0L : (1L<<(led-1))),
				 			ctrl.leds, &cause);
		ctrl.leds = keybd->kbdfeed->ctrl.leds;
	    }
#endif
	    break;
	case KBKey:
	    key = (KeyCode)*vlist;
	    vlist++;
	    if ((KeyCode)key < inputInfo.keyboard->key->curKeySyms.minKeyCode ||
		(KeyCode)key > inputInfo.keyboard->key->curKeySyms.maxKeyCode)
	    {
		client->errorValue = key;
		return BadValue;
	    }
	    if (!(stuff->mask & KBAutoRepeatMode))
		return BadMatch;
	    break;
	case KBAutoRepeatMode:
	    i = (key >> 3);
	    mask = (1 << (key & 7));
	    t = (CARD8)*vlist;
	    vlist++;
#ifdef XKB
	    if (!noXkbExtension && key != DO_ALL)
		XkbDisableComputedAutoRepeats(keybd,key);
#endif
	    if (t == AutoRepeatModeOff)
	    {
		if (key == DO_ALL)
		    ctrl.autoRepeat = FALSE;
		else
		    ctrl.autoRepeats[i] &= ~mask;
	    }
	    else if (t == AutoRepeatModeOn)
	    {
		if (key == DO_ALL)
		    ctrl.autoRepeat = TRUE;
		else
		    ctrl.autoRepeats[i] |= mask;
	    }
	    else if (t == AutoRepeatModeDefault)
	    {
		if (key == DO_ALL)
		    ctrl.autoRepeat = defaultKeyboardControl.autoRepeat;
		else
		    ctrl.autoRepeats[i] =
			    (ctrl.autoRepeats[i] & ~mask) |
			    (defaultKeyboardControl.autoRepeats[i] & mask);
	    }
	    else
	    {
		client->errorValue = t;
		return BadValue;
	    }
	    break;
	default:
	    client->errorValue = stuff->mask;
	    return BadValue;
	}
    }
    keybd->kbdfeed->ctrl = ctrl;
#ifdef XKB
    /* The XKB RepeatKeys control and core protocol global autorepeat */
    /* value are linked	*/
    if (!noXkbExtension) {
	XkbSetRepeatKeys(keybd,key,keybd->kbdfeed->ctrl.autoRepeat);
    }
    else
#endif
    (*keybd->kbdfeed->CtrlProc)(keybd, &keybd->kbdfeed->ctrl);
    return Success;
#undef DO_ALL
} 

int
ProcGetKeyboardControl (ClientPtr client)
{
    int i;
    register KeybdCtrl *ctrl = &inputInfo.keyboard->kbdfeed->ctrl;
    xGetKeyboardControlReply rep;

    REQUEST_SIZE_MATCH(xReq);
    rep.type = X_Reply;
    rep.length = 5;
    rep.sequenceNumber = client->sequence;
    rep.globalAutoRepeat = ctrl->autoRepeat;
    rep.keyClickPercent = ctrl->click;
    rep.bellPercent = ctrl->bell;
    rep.bellPitch = ctrl->bell_pitch;
    rep.bellDuration = ctrl->bell_duration;
    rep.ledMask = ctrl->leds;
    for (i = 0; i < 32; i++)
	rep.map[i] = ctrl->autoRepeats[i];
    WriteReplyToClient(client, sizeof(xGetKeyboardControlReply), &rep);
    return Success;
} 

int
ProcBell(ClientPtr client)
{
    register DeviceIntPtr keybd = inputInfo.keyboard;
    int base = keybd->kbdfeed->ctrl.bell;
    int newpercent;
    REQUEST(xBellReq);
    REQUEST_SIZE_MATCH(xBellReq);
    if (stuff->percent < -100 || stuff->percent > 100)
    {
	client->errorValue = stuff->percent;
	return BadValue;
    }
    newpercent = (base * stuff->percent) / 100;
    if (stuff->percent < 0)
        newpercent = base + newpercent;
    else
    	newpercent = base - newpercent + stuff->percent;
#ifdef XKB
    if (!noXkbExtension)
	XkbHandleBell(FALSE,FALSE, keybd, newpercent, &keybd->kbdfeed->ctrl, 0, 
		      None, NULL, client);
	else
#endif
    (*keybd->kbdfeed->BellProc)(newpercent, keybd,
				(pointer) &keybd->kbdfeed->ctrl, 0);
    return Success;
} 

int
ProcChangePointerControl(ClientPtr client)
{
    DeviceIntPtr mouse = inputInfo.pointer;
    PtrCtrl ctrl;		/* might get BadValue part way through */
    REQUEST(xChangePointerControlReq);

    REQUEST_SIZE_MATCH(xChangePointerControlReq);
    ctrl = mouse->ptrfeed->ctrl;
    if ((stuff->doAccel != xTrue) && (stuff->doAccel != xFalse))
    {
	client->errorValue = stuff->doAccel;
	return(BadValue);
    }
    if ((stuff->doThresh != xTrue) && (stuff->doThresh != xFalse))
    {
	client->errorValue = stuff->doThresh;
	return(BadValue);
    }
    if (stuff->doAccel)
    {
	if (stuff->accelNum == -1)
	    ctrl.num = defaultPointerControl.num;
	else if (stuff->accelNum < 0)
	{
	    client->errorValue = stuff->accelNum;
	    return BadValue;
	}
	else ctrl.num = stuff->accelNum;
	if (stuff->accelDenum == -1)
	    ctrl.den = defaultPointerControl.den;
	else if (stuff->accelDenum <= 0)
	{
	    client->errorValue = stuff->accelDenum;
	    return BadValue;
	}
	else ctrl.den = stuff->accelDenum;
    }
    if (stuff->doThresh)
    {
	if (stuff->threshold == -1)
	    ctrl.threshold = defaultPointerControl.threshold;
	else if (stuff->threshold < 0)
	{
	    client->errorValue = stuff->threshold;
	    return BadValue;
	}
	else ctrl.threshold = stuff->threshold;
    }
    mouse->ptrfeed->ctrl = ctrl;
    (*mouse->ptrfeed->CtrlProc)(mouse, &mouse->ptrfeed->ctrl);
    return Success;
} 

int
ProcGetPointerControl(ClientPtr client)
{
    register PtrCtrl *ctrl = &inputInfo.pointer->ptrfeed->ctrl;
    xGetPointerControlReply rep;

    REQUEST_SIZE_MATCH(xReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.threshold = ctrl->threshold;
    rep.accelNumerator = ctrl->num;
    rep.accelDenominator = ctrl->den;
    WriteReplyToClient(client, sizeof(xGenericReply), &rep);
    return Success;
}

void
MaybeStopHint(register DeviceIntPtr dev, ClientPtr client)
{
    GrabPtr grab = dev->grab;

    if ((grab && SameClient(grab, client) &&
	 ((grab->eventMask & PointerMotionHintMask) ||
	  (grab->ownerEvents &&
	   (EventMaskForClient(dev->valuator->motionHintWindow, client) &
	    PointerMotionHintMask)))) ||
	(!grab &&
	 (EventMaskForClient(dev->valuator->motionHintWindow, client) &
	  PointerMotionHintMask)))
	dev->valuator->motionHintWindow = NullWindow;
}

int
ProcGetMotionEvents(ClientPtr client)
{
    WindowPtr pWin;
    xTimecoord * coords = (xTimecoord *) NULL;
    xGetMotionEventsReply rep;
    int     i, count, xmin, xmax, ymin, ymax;
    unsigned long nEvents;
    DeviceIntPtr mouse = inputInfo.pointer;
    TimeStamp start, stop;
    REQUEST(xGetMotionEventsReq);

    REQUEST_SIZE_MATCH(xGetMotionEventsReq);
    pWin = SecurityLookupWindow(stuff->window, client, TRUE);
    if (!pWin)
	return BadWindow;
    if (mouse->valuator->motionHintWindow)
	MaybeStopHint(mouse, client);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    nEvents = 0;
    start = ClientTimeToServerTime(stuff->start);
    stop = ClientTimeToServerTime(stuff->stop);
    if ((CompareTimeStamps(start, stop) != LATER) &&
	(CompareTimeStamps(start, currentTime) != LATER) &&
	mouse->valuator->numMotionEvents)
    {
	if (CompareTimeStamps(stop, currentTime) == LATER)
	    stop = currentTime;
	coords = (xTimecoord *)ALLOCATE_LOCAL(mouse->valuator->numMotionEvents
					      * sizeof(xTimecoord));
	if (!coords)
	    return BadAlloc;
	count = (*mouse->valuator->GetMotionProc) (mouse, coords,
						   start.milliseconds,
						   stop.milliseconds,
						   pWin->drawable.pScreen);
	xmin = pWin->drawable.x - wBorderWidth (pWin);
	xmax = pWin->drawable.x + (int)pWin->drawable.width +
		wBorderWidth (pWin);
	ymin = pWin->drawable.y - wBorderWidth (pWin);
	ymax = pWin->drawable.y + (int)pWin->drawable.height +
		wBorderWidth (pWin);
	for (i = 0; i < count; i++)
	    if ((xmin <= coords[i].x) && (coords[i].x < xmax) &&
		    (ymin <= coords[i].y) && (coords[i].y < ymax))
	    {
		coords[nEvents].time = coords[i].time;
		coords[nEvents].x = coords[i].x - pWin->drawable.x;
		coords[nEvents].y = coords[i].y - pWin->drawable.y;
		nEvents++;
	    }
    }
    rep.length = nEvents * (sizeof(xTimecoord) >> 2);
    rep.nEvents = nEvents;
    WriteReplyToClient(client, sizeof(xGetMotionEventsReply), &rep);
    if (nEvents)
    {
	client->pSwapReplyFunc = (ReplySwapPtr) SwapTimeCoordWrite;
	WriteSwappedDataToClient(client, nEvents * sizeof(xTimecoord),
				 (char *)coords);
    }
    if (coords)
	DEALLOCATE_LOCAL(coords);
    return Success;
}

int
ProcQueryKeymap(ClientPtr client)
{
    xQueryKeymapReply rep;
    int i;
    CARD8 *down = inputInfo.keyboard->key->down;

    REQUEST_SIZE_MATCH(xReq);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 2;
#ifdef XCSECURITY
    if (!SecurityCheckDeviceAccess(client, inputInfo.keyboard, TRUE))
    {
	bzero((char *)&rep.map[0], 32);
    }
    else
#endif
    for (i = 0; i<32; i++)
	rep.map[i] = down[i];
    WriteReplyToClient(client, sizeof(xQueryKeymapReply), &rep);
    return Success;
}

/******************************************************************************
 * The following entrypoints are provided for binary compatibility with
 * previous versions (they make casts, where the current version changes types
 * for more stringent prototype checking).
 ******************************************************************************/
#ifdef AddInputDevice
#undef AddInputDevice

DevicePtr
AddInputDevice(DeviceProc deviceProc, Bool autoStart)
{
    return (DevicePtr)_AddInputDevice(deviceProc, autoStart);
}
#endif /* AddInputDevice */

#ifdef RegisterPointerDevice
#undef RegisterPointerDevice

void
RegisterPointerDevice(DevicePtr device)
{
    _RegisterPointerDevice((DeviceIntPtr)device);
}
#endif /* RegisterPointerDevice */

#ifdef RegisterKeyboardDevice
#undef RegisterKeyboardDevice

void
RegisterKeyboardDevice(DevicePtr device)
{
    _RegisterKeyboardDevice((DeviceIntPtr)device);
}
#endif /* RegisterKeyboardDevice */
