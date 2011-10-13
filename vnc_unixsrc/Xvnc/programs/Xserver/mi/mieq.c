/*
 * $Xorg: mieq.c,v 1.4 2001/02/09 02:05:20 xorgcvs Exp $
 *
Copyright 1990, 1998  The Open Group

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
 *
 * Author:  Keith Packard, MIT X Consortium
 */
/* $XFree86: xc/programs/Xserver/mi/mieq.c,v 1.2 2001/05/25 18:41:01 dawes Exp $ */

/*
 * mieq.c
 *
 * Machine independent event queue
 *
 */

# define NEED_EVENTS
# include   <X11/X.h>
# include   <X11/Xmd.h>
# include   <X11/Xproto.h>
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
    HWEventQueueType	oldtail, newtail;
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
    miEventQueue.lastEventTime =
	miEventQueue.events[oldtail].event.u.keyButtonPointer.time;
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

void mieqProcessInputEvents ()
{
    EventRec	*e;
    int		x, y;
    xEvent	xe;

    while (miEventQueue.head != miEventQueue.tail)
    {
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
