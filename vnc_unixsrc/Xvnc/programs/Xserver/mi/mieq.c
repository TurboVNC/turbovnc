/*
 * $XConsortium: mieq.c,v 1.8 94/11/02 15:59:29 kaleb Exp $
 *
Copyright (c) 1990  X Consortium

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

/*
 * mieq.c
 *
 * Machine independent event queue
 *
 */

# define NEED_EVENTS
# include   "X.h"
# include   "Xmd.h"
# include   "Xproto.h"
# include   "misc.h"
# include   "windowstr.h"
# include   "pixmapstr.h"
# include   "inputstr.h"
# include   "mi.h"
# include   "scrnintstr.h"

#define QUEUE_SIZE  256

typedef struct _Event {
    xEvent	event;
    ScreenPtr	pScreen;
} EventRec, *EventPtr;

typedef struct _EventQueue {
    HWEventQueueType	head, tail;	    /* long for SetInputCheck */
    CARD32	lastEventTime;	    /* to avoid time running backwards */
    Bool	lastMotion;
    EventRec	events[QUEUE_SIZE]; /* static allocation for signals */
    DevicePtr	pKbd, pPtr;	    /* device pointer, to get funcs */
    ScreenPtr	pEnqueueScreen;	    /* screen events are being delivered to */
    ScreenPtr	pDequeueScreen;	    /* screen events are being dispatched to */
} EventQueueRec, *EventQueuePtr;

static EventQueueRec miEventQueue;

Bool
mieqInit (pKbd, pPtr)
    DevicePtr	pKbd, pPtr;
{
    miEventQueue.head = miEventQueue.tail = 0;
    miEventQueue.lastEventTime = GetTimeInMillis ();
    miEventQueue.pKbd = pKbd;
    miEventQueue.pPtr = pPtr;
    miEventQueue.lastMotion = FALSE;
    miEventQueue.pEnqueueScreen = screenInfo.screens[0];
    miEventQueue.pDequeueScreen = miEventQueue.pEnqueueScreen;
    SetInputCheck (&miEventQueue.head, &miEventQueue.tail);
    return TRUE;
}

/*
 * Must be reentrant with ProcessInputEvents.  Assumption: mieqEnqueue
 * will never be interrupted.  If this is called from both signal
 * handlers and regular code, make sure the signal is suspended when
 * called from regular code.
 */

void
mieqEnqueue (e)
    xEvent	*e;
{
    HWEventQueueType	oldtail, newtail, prevtail;
    Bool    isMotion;

    oldtail = miEventQueue.tail;
    isMotion = e->u.u.type == MotionNotify;
    if (isMotion && miEventQueue.lastMotion && oldtail != miEventQueue.head)
    {
	if (oldtail == 0)
	    oldtail = QUEUE_SIZE;
	oldtail = oldtail - 1;
    }
    else
    {
    	newtail = oldtail + 1;
    	if (newtail == QUEUE_SIZE)
	    newtail = 0;
    	/* Toss events which come in late */
    	if (newtail == miEventQueue.head)
	    return;
	miEventQueue.tail = newtail;
    }
    miEventQueue.lastMotion = isMotion;
    miEventQueue.events[oldtail].event = *e;
    /*
     * Make sure that event times don't go backwards - this
     * is "unnecessary", but very useful
     */
    if (e->u.keyButtonPointer.time < miEventQueue.lastEventTime &&
	miEventQueue.lastEventTime - e->u.keyButtonPointer.time < 10000)
    {
	miEventQueue.events[oldtail].event.u.keyButtonPointer.time =
	    miEventQueue.lastEventTime;
    }
    miEventQueue.events[oldtail].pScreen = miEventQueue.pEnqueueScreen;
}

void
mieqSwitchScreen (pScreen, fromDIX)
    ScreenPtr	pScreen;
    Bool	fromDIX;
{
    miEventQueue.pEnqueueScreen = pScreen;
    if (fromDIX)
	miEventQueue.pDequeueScreen = pScreen;
}

/*
 * Call this from ProcessInputEvents()
 */

mieqProcessInputEvents ()
{
    EventRec	*e;
    int		x, y;
    xEvent	xe;

    while (miEventQueue.head != miEventQueue.tail)
    {
	extern int  screenIsSaved;

	if (screenIsSaved == SCREEN_SAVER_ON)
	    SaveScreens (SCREEN_SAVER_OFF, ScreenSaverReset);

	e = &miEventQueue.events[miEventQueue.head];
	/*
	 * Assumption - screen switching can only occur on motion events
	 */
	if (e->pScreen != miEventQueue.pDequeueScreen)
	{
	    miEventQueue.pDequeueScreen = e->pScreen;
	    x = e->event.u.keyButtonPointer.rootX;
	    y = e->event.u.keyButtonPointer.rootY;
	    if (miEventQueue.head == QUEUE_SIZE - 1)
	    	miEventQueue.head = 0;
	    else
	    	++miEventQueue.head;
	    NewCurrentScreen (miEventQueue.pDequeueScreen, x, y);
	}
	else
	{
	    xe = e->event;
	    if (miEventQueue.head == QUEUE_SIZE - 1)
	    	miEventQueue.head = 0;
	    else
	    	++miEventQueue.head;
	    switch (xe.u.u.type) 
	    {
	    case KeyPress:
	    case KeyRelease:
	    	(*miEventQueue.pKbd->processInputProc)
				(&xe, (DeviceIntPtr)miEventQueue.pKbd, 1);
	    	break;
	    default:
	    	(*miEventQueue.pPtr->processInputProc)
				(&xe, (DeviceIntPtr)miEventQueue.pPtr, 1);
	    	break;
	    }
	}
    }
}
