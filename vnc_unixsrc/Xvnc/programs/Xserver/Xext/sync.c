/* $XConsortium: sync.c /main/13 1996/12/16 16:51:55 rws $ */
/* $XFree86: xc/programs/Xserver/Xext/sync.c,v 3.3 1997/01/18 06:53:00 dawes Exp $ */
/*

Copyright (c) 1991, 1993  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.


Copyright 1991, 1993 by Digital Equipment Corporation, Maynard, Massachusetts,
and Olivetti Research Limited, Cambridge, England.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Digital or Olivetti 
not be used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  Digital and Olivetti
make no representations about the suitability of this software
for any purpose.  It is provided "as is" without express or implied warranty.

DIGITAL AND OLIVETTI DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS, IN NO EVENT SHALL THEY BE LIABLE FOR ANY SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

*/

#define NEED_REPLIES
#define NEED_EVENTS
#include <stdio.h>
#include "X.h"
#include "Xproto.h"
#include "Xmd.h"
#include "misc.h"
#include "os.h"
#include "extnsionst.h"
#include "dixstruct.h"
#include "resource.h"
#include "opaque.h"
#define _SYNC_SERVER
#include "sync.h"
#include "syncstr.h"

/*
 * Local Global Variables
 */
static int      SyncReqCode;
static int      SyncEventBase;
static int      SyncErrorBase;
static RESTYPE  RTCounter = 0;
static RESTYPE  RTAwait;
static RESTYPE  RTAlarm;
static RESTYPE  RTAlarmClient;
static int SyncNumSystemCounters = 0;
static SyncCounter **SysCounterList = NULL;

#define IsSystemCounter(pCounter) \
    (pCounter && (pCounter->client == NULL))

/* these are all the alarm attributes that pertain to the alarm's trigger */
#define XSyncCAAllTrigger \
    (XSyncCACounter | XSyncCAValueType | XSyncCAValue | XSyncCATestType)

static int
FreeAlarm(
#if NeedFunctionPrototypes
    pointer /* addr */,
    XID /* id */
#endif
);

static int
FreeAlarmClient(
#if NeedFunctionPrototypes
    pointer /* value */,
    XID /* id */
#endif
);

static int
FreeAwait(
#if NeedFunctionPrototypes
    pointer /* addr */,
    XID /* id */
#endif
);

static void
ServertimeBracketValues(
#if NeedFunctionPrototypes
    pointer /* pCounter */,
    CARD64 * /* pbracket_less */,
    CARD64 * /* pbracket_greater */
#endif
);

static void
ServertimeQueryValue(
#if NeedFunctionPrototypes
    pointer /* pCounter */,
    CARD64 * /* pValue_return */
#endif
);

static void
ServertimeWakeupHandler(
#if NeedFunctionPrototypes
    pointer /* env */,
    int /* rc */,
    pointer /* LastSelectMask */
#endif
);

static int 
SyncInitTrigger(
#if NeedFunctionPrototypes
    ClientPtr /* client */,
    SyncTrigger * /* pTrigger */,
    XSyncCounter /* counter */,
    Mask /* changes */
#endif
);

static void
SAlarmNotifyEvent(
#if NeedFunctionPrototypes
    xSyncAlarmNotifyEvent * /* from */,
    xSyncAlarmNotifyEvent * /* to */
#endif
);

static void
SCounterNotifyEvent(
#if NeedFunctionPrototypes
    xSyncCounterNotifyEvent * /* from */,
    xSyncCounterNotifyEvent * /* to */
#endif
);

static void
ServertimeBlockHandler(
#if NeedFunctionPrototypes
    pointer  /* env */,
    struct timeval ** /* wt */,
    pointer  /* LastSelectMask */
#endif
);

static int
SyncAddTriggerToCounter(
#if NeedFunctionPrototypes
    SyncTrigger * /* pTrigger */
#endif
);

extern void
SyncAlarmCounterDestroyed(
#if NeedFunctionPrototypes
    SyncTrigger * /* pTrigger */
#endif
);

static void
SyncAlarmTriggerFired(
#if NeedFunctionPrototypes
    SyncTrigger * /* pTrigger */
#endif
);

static void
SyncAwaitTriggerFired(
#if NeedFunctionPrototypes
    SyncTrigger * /* pTrigger */
#endif
);

static int
SyncChangeAlarmAttributes(
#if NeedFunctionPrototypes
    ClientPtr /* client */,
    SyncAlarm * /* pAlarm */,
    Mask /* mask */,
    CARD32 * /* values */
#endif
);

static Bool
SyncCheckTriggerNegativeComparison(
#if NeedFunctionPrototypes
    SyncTrigger * /* pTrigger */,
    CARD64 /* oldval */
#endif
);

static Bool
SyncCheckTriggerNegativeTransition(
#if NeedFunctionPrototypes
    SyncTrigger * /* pTrigger */,
    CARD64 /* oldval */
#endif
);

static Bool
SyncCheckTriggerPositiveComparison(
#if NeedFunctionPrototypes
    SyncTrigger * /* pTrigger */,
    CARD64 /* oldval */
#endif
);

static Bool
SyncCheckTriggerPositiveTransition(
#if NeedFunctionPrototypes
    SyncTrigger * /* pTrigger */,
    CARD64 /* oldval */
#endif
);

static SyncCounter *
SyncCreateCounter(
#if NeedFunctionPrototypes
    ClientPtr /* client */,
    XSyncCounter /* id */,
    CARD64 /* initialvalue */
#endif
);

static void SyncComputeBracketValues(
#if NeedFunctionPrototypes
    SyncCounter * /* pCounter */,
    Bool /* startOver */
#endif
);

static void
SyncDeleteTriggerFromCounter(
#if NeedFunctionPrototypes
    SyncTrigger * /* pTrigger */
#endif
);

static Bool
SyncEventSelectForAlarm(
#if NeedFunctionPrototypes
    SyncAlarm * /* pAlarm */,
    ClientPtr /* client */,
    Bool /* wantevents */
#endif
);

static void
SyncInitServerTime(
#if NeedFunctionPrototypes
    void
#endif
);

static void 
SyncResetProc(
#if NeedFunctionPrototypes
    ExtensionEntry * /* extEntry */
#endif
);

static void
SyncSendAlarmNotifyEvents(
#if NeedFunctionPrototypes
    SyncAlarm * /* pAlarm */
#endif
);

static void
SyncSendCounterNotifyEvents(
#if NeedFunctionPrototypes
    ClientPtr /* client */,
    SyncAwait ** /* ppAwait */,
    int /* num_events */
#endif
);

static DISPATCH_PROC(ProcSyncAwait);
static DISPATCH_PROC(ProcSyncChangeAlarm);
static DISPATCH_PROC(ProcSyncChangeCounter);
static DISPATCH_PROC(ProcSyncCreateAlarm);
static DISPATCH_PROC(ProcSyncCreateCounter);
static DISPATCH_PROC(ProcSyncDestroyAlarm);
static DISPATCH_PROC(ProcSyncDestroyCounter);
static DISPATCH_PROC(ProcSyncDispatch);
static DISPATCH_PROC(ProcSyncGetPriority);
static DISPATCH_PROC(ProcSyncInitialize);
static DISPATCH_PROC(ProcSyncListSystemCounters);
static DISPATCH_PROC(ProcSyncListSystemCounters);
static DISPATCH_PROC(ProcSyncQueryAlarm);
static DISPATCH_PROC(ProcSyncQueryCounter);
static DISPATCH_PROC(ProcSyncSetCounter);
static DISPATCH_PROC(ProcSyncSetPriority);
static DISPATCH_PROC(SProcSyncAwait);
static DISPATCH_PROC(SProcSyncChangeAlarm);
static DISPATCH_PROC(SProcSyncChangeCounter);
static DISPATCH_PROC(SProcSyncCreateAlarm);
static DISPATCH_PROC(SProcSyncCreateCounter);
static DISPATCH_PROC(SProcSyncDestroyAlarm);
static DISPATCH_PROC(SProcSyncDestroyCounter);
static DISPATCH_PROC(SProcSyncDispatch);
static DISPATCH_PROC(SProcSyncDispatch);
static DISPATCH_PROC(SProcSyncGetPriority);
static DISPATCH_PROC(SProcSyncInitialize);
static DISPATCH_PROC(SProcSyncListSystemCounters);
static DISPATCH_PROC(SProcSyncQueryAlarm);
static DISPATCH_PROC(SProcSyncQueryCounter);
static DISPATCH_PROC(SProcSyncSetCounter);
static DISPATCH_PROC(SProcSyncSetPriority);

/*  Each counter maintains a simple linked list of triggers that are
 *  interested in the counter.  The two functions below are used to
 *  delete and add triggers on this list.
 */
static void
SyncDeleteTriggerFromCounter(pTrigger)
    SyncTrigger *pTrigger;
{
    SyncTriggerList *pCur, *pPrev = NULL;

    /* pCounter needs to be stored in pTrigger before calling here. */

    if (!pTrigger->pCounter)
	return;

    for (pCur = pTrigger->pCounter->pTriglist; pCur; pCur = pCur->next)
    {
	if (pCur->pTrigger == pTrigger)
	{
	    if (pPrev)
		pPrev->next = pCur->next;
	    else
		pTrigger->pCounter->pTriglist = pCur->next;
	    xfree(pCur);
	    break;
	}
    }

    if (IsSystemCounter(pTrigger->pCounter))
	SyncComputeBracketValues(pTrigger->pCounter, /*startOver*/ TRUE);
}


static int
SyncAddTriggerToCounter(pTrigger)
    SyncTrigger *pTrigger;
{
    SyncTriggerList *pCur;

    if (!pTrigger->pCounter)
	return Success;

    /* don't do anything if it's already there */
    for (pCur = pTrigger->pCounter->pTriglist; pCur; pCur = pCur->next)
    {
	if (pCur->pTrigger == pTrigger)
	    return Success;
    }

    if (!(pCur = (SyncTriggerList *)xalloc(sizeof(SyncTriggerList))))
	return BadAlloc;

    pCur->pTrigger = pTrigger;
    pCur->next = pTrigger->pCounter->pTriglist;
    pTrigger->pCounter->pTriglist = pCur;

    if (IsSystemCounter(pTrigger->pCounter))
	SyncComputeBracketValues(pTrigger->pCounter, /*startOver*/ TRUE);

    return Success;
}


/*  Below are four possible functions that can be plugged into 
 *  pTrigger->CheckTrigger, corresponding to the four possible
 *  test-types.  These functions are called after the counter's
 *  value changes but are also passed the old counter value
 *  so they can inspect both the old and new values.
 *  (PositiveTransition and NegativeTransition need to see both
 *  pieces of information.)  These functions return the truth value
 *  of the trigger.
 *
 *  All of them include the condition pTrigger->pCounter == NULL.
 *  This is because the spec says that a trigger with a counter value 
 *  of None is always TRUE.
 */

static Bool
SyncCheckTriggerPositiveComparison(pTrigger, oldval)
    SyncTrigger *pTrigger;
    CARD64	oldval;
{
    return (pTrigger->pCounter == NULL ||
	    XSyncValueGreaterOrEqual(pTrigger->pCounter->value,
				     pTrigger->test_value));
}

static Bool
SyncCheckTriggerNegativeComparison(pTrigger, oldval)
    SyncTrigger *pTrigger;
    CARD64	oldval;
{
    return (pTrigger->pCounter == NULL ||
	    XSyncValueLessOrEqual(pTrigger->pCounter->value,
				  pTrigger->test_value));
}

static Bool
SyncCheckTriggerPositiveTransition(pTrigger, oldval)
    SyncTrigger *pTrigger;
    CARD64	oldval;
{
    return (pTrigger->pCounter == NULL ||
	    (XSyncValueLessThan(oldval, pTrigger->test_value) &&
	     XSyncValueGreaterOrEqual(pTrigger->pCounter->value,
				      pTrigger->test_value)));
}

static Bool
SyncCheckTriggerNegativeTransition(pTrigger, oldval)
    SyncTrigger *pTrigger;
    CARD64	oldval;
{
    return (pTrigger->pCounter == NULL ||
	    (XSyncValueGreaterThan(oldval, pTrigger->test_value) &&
	     XSyncValueLessOrEqual(pTrigger->pCounter->value,
				   pTrigger->test_value)));
}



static int 
SyncInitTrigger(client, pTrigger, counter, changes) 
    ClientPtr	     client;    /* so we can set errorValue */
    SyncTrigger      *pTrigger;
    XSyncCounter     counter; 
    Mask	     changes;
{
    SyncCounter *pCounter = pTrigger->pCounter;
    int		status;
    Bool	newcounter = FALSE;

    if (changes & XSyncCACounter)
    {
	if (counter == None)
	    pCounter = NULL;
	else if (!(pCounter = (SyncCounter *)SecurityLookupIDByType(
			client, counter, RTCounter, SecurityReadAccess)))
	{
	    client->errorValue = counter;
	    return SyncErrorBase + XSyncBadCounter;
	}
	if (pCounter != pTrigger->pCounter)
	{ /* new counter for trigger */
	    SyncDeleteTriggerFromCounter(pTrigger);
	    pTrigger->pCounter = pCounter;
	    newcounter = TRUE;
	}
    }

    /* if system counter, ask it what the current value is */

    if (IsSystemCounter(pCounter))
    {
	(*pCounter->pSysCounterInfo->QueryValue) ((pointer) pCounter,
						  &pCounter->value);
    }

    if (changes & XSyncCAValueType)
    {
	if (pTrigger->value_type != XSyncRelative &&
	    pTrigger->value_type != XSyncAbsolute)
	{
	    client->errorValue = pTrigger->value_type;
	    return BadValue;
	}
    }

    if (changes & XSyncCATestType)
    {
	if (pTrigger->test_type != XSyncPositiveTransition &&
	    pTrigger->test_type != XSyncNegativeTransition &&
	    pTrigger->test_type != XSyncPositiveComparison &&
	    pTrigger->test_type != XSyncNegativeComparison)
	{
	    client->errorValue = pTrigger->test_type;
	    return BadValue;
	}
	/* select appropriate CheckTrigger function */

	switch (pTrigger->test_type)
	{
        case XSyncPositiveTransition: 
	    pTrigger->CheckTrigger = SyncCheckTriggerPositiveTransition;
	    break;
        case XSyncNegativeTransition: 
	    pTrigger->CheckTrigger = SyncCheckTriggerNegativeTransition;
	    break;
        case XSyncPositiveComparison: 
	    pTrigger->CheckTrigger = SyncCheckTriggerPositiveComparison;
	    break;
        case XSyncNegativeComparison: 
	    pTrigger->CheckTrigger = SyncCheckTriggerNegativeComparison;
	    break;
	}
    }

    if (changes & (XSyncCAValueType | XSyncCAValue))
    {
	if (pTrigger->value_type == XSyncAbsolute)
	    pTrigger->test_value = pTrigger->wait_value;
	else /* relative */
	{
	    Bool overflow;
	    if (pCounter == NULL)
		return BadMatch;

	    XSyncValueAdd(&pTrigger->test_value, pCounter->value, 
			  pTrigger->wait_value, &overflow);
	    if (overflow)
	    {
		client->errorValue = XSyncValueHigh32(pTrigger->wait_value);
		return BadValue;
	    }
	}
    }

    /*  we wait until we're sure there are no errors before registering
     *  a new counter on a trigger
     */
    if (newcounter)
    {
	if ((status = SyncAddTriggerToCounter(pTrigger)) != Success)
	    return status;
    }
    else if (IsSystemCounter(pCounter))
    {
	SyncComputeBracketValues(pCounter, /*startOver*/ TRUE);
    }
    
    return Success;
}

/*  AlarmNotify events happen in response to actions taken on an Alarm or
 *  the counter used by the alarm.  AlarmNotify may be sent to multiple 
 *  clients.  The alarm maintains a list of clients interested in events.
 */
static void
SyncSendAlarmNotifyEvents(pAlarm)
    SyncAlarm *pAlarm;
{
    SyncAlarmClientList *pcl;
    xSyncAlarmNotifyEvent ane;
    SyncTrigger *pTrigger = &pAlarm->trigger;

    UpdateCurrentTime();

    ane.type = SyncEventBase + XSyncAlarmNotify;
    ane.kind = XSyncAlarmNotify;
    ane.sequenceNumber = pAlarm->client->sequence;
    ane.alarm = pAlarm->alarm_id;
    if (pTrigger->pCounter)
    {
	ane.counter_value_hi = XSyncValueHigh32(pTrigger->pCounter->value);
	ane.counter_value_lo = XSyncValueLow32(pTrigger->pCounter->value);
    }
    else
    { /* XXX what else can we do if there's no counter? */
	ane.counter_value_hi = ane.counter_value_lo = 0;
    }

    ane.alarm_value_hi = XSyncValueHigh32(pTrigger->test_value);
    ane.alarm_value_lo = XSyncValueLow32(pTrigger->test_value);
    ane.time = currentTime.milliseconds;
    ane.state = pAlarm->state;

    /* send to owner */
    if (pAlarm->events && !pAlarm->client->clientGone) 
	WriteEventsToClient(pAlarm->client, 1, (xEvent *) &ane);

    /* send to other interested clients */
    for (pcl = pAlarm->pEventClients; pcl; pcl = pcl->next)
    {
	if (!pAlarm->client->clientGone)
	{
	    ane.sequenceNumber = pcl->client->sequence;
	    WriteEventsToClient(pcl->client, 1, (xEvent *) &ane);
	}
    }
}


/*  CounterNotify events only occur in response to an Await.  The events 
 *  go only to the Awaiting client.
 */
static void
SyncSendCounterNotifyEvents(client, ppAwait, num_events)
    ClientPtr client;
    SyncAwait **ppAwait;
    int num_events;
{
    xSyncCounterNotifyEvent *pEvents, *pev;
    int i;

    if (client->clientGone)
	return;
    pev = pEvents = (xSyncCounterNotifyEvent *)
		 ALLOCATE_LOCAL(num_events * sizeof(xSyncCounterNotifyEvent));
    if (!pEvents) 
	return;
    UpdateCurrentTime();
    for (i = 0; i < num_events; i++, ppAwait++, pev++)
    {
	SyncTrigger *pTrigger = &(*ppAwait)->trigger;
	pev->type = SyncEventBase + XSyncCounterNotify;
	pev->kind = XSyncCounterNotify;
	pev->sequenceNumber = client->sequence;
	pev->counter = pTrigger->pCounter->id;
	pev->wait_value_lo = XSyncValueLow32(pTrigger->test_value);
	pev->wait_value_hi = XSyncValueHigh32(pTrigger->test_value);
	pev->counter_value_lo = XSyncValueLow32(pTrigger->pCounter->value);
	pev->counter_value_hi = XSyncValueHigh32(pTrigger->pCounter->value);
	pev->time = currentTime.milliseconds;
	pev->count = num_events - i - 1; /* events remaining */
	pev->destroyed = pTrigger->pCounter->beingDestroyed;
    }
    /* swapping will be taken care of by this */
    WriteEventsToClient(client, num_events, (xEvent *)pEvents);
    DEALLOCATE_LOCAL(pEvents);
}


/* This function is called when an alarm's counter is destroyed.
 * It is plugged into pTrigger->CounterDestroyed (for alarm triggers).
 */
void
SyncAlarmCounterDestroyed(pTrigger)
    SyncTrigger *pTrigger;
{
    SyncAlarm *pAlarm = (SyncAlarm *)pTrigger;

    pAlarm->state = XSyncAlarmInactive;
    SyncSendAlarmNotifyEvents(pAlarm);
    pTrigger->pCounter = NULL;
}


/*  This function is called when an alarm "goes off."  
 *  It is plugged into pTrigger->TriggerFired (for alarm triggers).
 */
static void
SyncAlarmTriggerFired(pTrigger)
    SyncTrigger *pTrigger;
{
    SyncAlarm *pAlarm = (SyncAlarm *)pTrigger;
    CARD64 new_test_value;

    /* no need to check alarm unless it's active */
    if (pAlarm->state != XSyncAlarmActive)
	return;

    /*  " if the counter value is None, or if the delta is 0 and
     *    the test-type is PositiveComparison or NegativeComparison,
     *    no change is made to value (test-value) and the alarm
     *    state is changed to Inactive before the event is generated."
     */
    if (pAlarm->trigger.pCounter == NULL
	|| (XSyncValueIsZero(pAlarm->delta)
	    && (pAlarm->trigger.test_type == XSyncPositiveComparison
		|| pAlarm->trigger.test_type == XSyncNegativeComparison)))
	pAlarm->state = XSyncAlarmInactive;

    new_test_value = pAlarm->trigger.test_value;

    if (pAlarm->state == XSyncAlarmActive)
    {
	Bool overflow;
	CARD64 oldvalue;
	SyncTrigger *paTrigger = &pAlarm->trigger;

	/* "The alarm is updated by repeatedly adding delta to the
	 *  value of the trigger and re-initializing it until it
	 *  becomes FALSE."
	 */
	oldvalue = paTrigger->test_value;

	/* XXX really should do something smarter here */

	do
	{
	    XSyncValueAdd(&paTrigger->test_value, paTrigger->test_value,
			  pAlarm->delta, &overflow);
	} while (!overflow && 
	      (*paTrigger->CheckTrigger)(paTrigger,
					paTrigger->pCounter->value));

	new_test_value = paTrigger->test_value;
	paTrigger->test_value = oldvalue;

	/* "If this update would cause value to fall outside the range
	 *  for an INT64...no change is made to value (test-value) and
	 *  the alarm state is changed to Inactive before the event is
	 *  generated."
	 */
	if (overflow)
	{
	    new_test_value = oldvalue;
	    pAlarm->state = XSyncAlarmInactive;
	}
    }
    /*  The AlarmNotify event has to have the "new state of the alarm"
     *  which we can't be sure of until this point.  However, it has
     *  to have the "old" trigger test value.  That's the reason for
     *  all the newvalue/oldvalue shuffling above.  After we send the
     *  events, give the trigger its new test value.
     */
    SyncSendAlarmNotifyEvents(pAlarm);
    pTrigger->test_value = new_test_value;
}


/*  This function is called when an Await unblocks, either as a result
 *  of the trigger firing OR the counter being destroyed.
 *  It goes into pTrigger->TriggerFired AND pTrigger->CounterDestroyed
 *  (for Await triggers).
 */
static void
SyncAwaitTriggerFired(pTrigger)
    SyncTrigger *pTrigger;
{
    SyncAwait *pAwait = (SyncAwait *)pTrigger;
    int numwaits;
    SyncAwaitUnion *pAwaitUnion;
    SyncAwait **ppAwait;
    int num_events = 0;

    pAwaitUnion = (SyncAwaitUnion *)pAwait->pHeader;
    numwaits = pAwaitUnion->header.num_waitconditions;
    ppAwait = (SyncAwait **)ALLOCATE_LOCAL(numwaits * sizeof(SyncAwait *));
    if (!ppAwait)
	goto bail;

    pAwait = &(pAwaitUnion+1)->await;

    /* "When a client is unblocked, all the CounterNotify events for
     *  the Await request are generated contiguously. If count is 0
     *  there are no more events to follow for this request. If
     *  count is n, there are at least n more events to follow."
     *
     *  Thus, it is best to find all the counters for which events
     *  need to be sent first, so that an accurate count field can
     *  be stored in the events.
     */
    for ( ; numwaits; numwaits--, pAwait++)
    {
	CARD64 diff;
	Bool overflow, diffgreater, diffequal;

	/* "A CounterNotify event with the destroyed flag set to TRUE is
	 *  always generated if the counter for one of the triggers is
	 *  destroyed."
	 */
	if (pAwait->trigger.pCounter->beingDestroyed)
	{
	    ppAwait[num_events++] = pAwait;
	    continue;
	}

	/* "The difference between the counter and the test value is
	 *  calculated by subtracting the test value from the value of
	 *  the counter."
	 */
	XSyncValueSubtract(&diff, pAwait->trigger.pCounter->value,
			   pAwait->trigger.test_value, &overflow);

	/* "If the difference lies outside the range for an INT64, an
	 *  event is not generated."
	 */
	if (overflow)
	    continue;
	diffgreater = XSyncValueGreaterThan(diff, pAwait->event_threshold);
	diffequal   = XSyncValueEqual(diff, pAwait->event_threshold);

	/* "If the test-type is PositiveTransition or
	 *  PositiveComparison, a CounterNotify event is generated if
	 *  the difference is at least event-threshold. If the test-type
	 *  is NegativeTransition or NegativeComparison, a CounterNotify
	 *  event is generated if the difference is at most
	 *  event-threshold."
	 */

	if ( ((pAwait->trigger.test_type == XSyncPositiveComparison ||
	       pAwait->trigger.test_type == XSyncPositiveTransition)
	       && (diffgreater || diffequal))
	     ||
	     ((pAwait->trigger.test_type == XSyncNegativeComparison ||
	       pAwait->trigger.test_type == XSyncNegativeTransition)
	      && (!diffgreater) /* less or equal */
	      )
	   )
	{
	    ppAwait[num_events++] = pAwait;
	}
    }
    if (num_events)
	SyncSendCounterNotifyEvents(pAwaitUnion->header.client, ppAwait,
				    num_events);
    DEALLOCATE_LOCAL(ppAwait);

bail:
    /* unblock the client */
    AttendClient(pAwaitUnion->header.client);
    /* delete the await */
    FreeResource(pAwaitUnion->header.delete_id, RT_NONE);
}


/*  This function should always be used to change a counter's value so that
 *  any triggers depending on the counter will be checked.
 */
void
SyncChangeCounter(pCounter, newval)
    SyncCounter    *pCounter;
    CARD64         newval;
{
    SyncTriggerList       *ptl, *pnext;
    CARD64 oldval;

    oldval = pCounter->value;
    pCounter->value = newval;

    /* run through triggers to see if any become true */
    for (ptl = pCounter->pTriglist; ptl; ptl = pnext)
    {
	pnext = ptl->next;
	if ((*ptl->pTrigger->CheckTrigger)(ptl->pTrigger, oldval))
	    (*ptl->pTrigger->TriggerFired)(ptl->pTrigger);
    }

    if (IsSystemCounter(pCounter))
    {
	SyncComputeBracketValues(pCounter, /* startOver */ FALSE);
    }
}


/* loosely based on dix/events.c/EventSelectForWindow */
static Bool
SyncEventSelectForAlarm(pAlarm, client, wantevents)
    SyncAlarm *pAlarm;
    ClientPtr client;
    Bool      wantevents;
{
    SyncAlarmClientList *pClients;

    if (client == pAlarm->client) /* alarm owner */
    {
	pAlarm->events = wantevents;
	return Success;
    }

    /* see if the client is already on the list (has events selected) */

    for (pClients = pAlarm->pEventClients; pClients;
	 pClients = pClients->next)
    {
	if (pClients->client == client)
	{
	    /* client's presence on the list indicates desire for 
	     * events.  If the client doesn't want events, remove it 
	     * from the list.  If the client does want events, do
	     * nothing, since it's already got them.
	     */
	    if (!wantevents)
	    {
		FreeResource(pClients->delete_id, RT_NONE);
	    }
	    return Success;
	}
    }

    /*  if we get here, this client does not currently have
     *  events selected on the alarm
     */

    if (!wantevents)
	/* client doesn't want events, and we just discovered that it 
	 * doesn't have them, so there's nothing to do.
	 */
	return Success;

    /* add new client to pAlarm->pEventClients */

    pClients = (SyncAlarmClientList *) xalloc(sizeof(SyncAlarmClientList));
    if (!pClients)
	return BadAlloc;

    /*  register it as a resource so it will be cleaned up 
     *  if the client dies
     */

    pClients->delete_id = FakeClientID(client->index);
    if (!AddResource(pClients->delete_id, RTAlarmClient, pAlarm))
    {
	xfree(pClients);
	return BadAlloc;
    }

    /* link it into list after we know all the allocations succeed */

    pClients->next = pAlarm->pEventClients;
    pAlarm->pEventClients = pClients;
    pClients->client = client;
    return Success;
}

/*
 * ** SyncChangeAlarmAttributes ** This is used by CreateAlarm and ChangeAlarm
 */
static int 
SyncChangeAlarmAttributes(client, pAlarm, mask, values)
    ClientPtr       client;
    SyncAlarm      *pAlarm;
    Mask	    mask;
    CARD32	    *values;
{
    int		   status;
    XSyncCounter   counter;
    Mask	   origmask = mask;

    counter = pAlarm->trigger.pCounter ? pAlarm->trigger.pCounter->id : None;

    while (mask)
    {
	int    index2 = lowbit(mask);
	mask &= ~index2;
	switch (index2)
	{
	  case XSyncCACounter:
	    mask &= ~XSyncCACounter;
	    /* sanity check in SyncInitTrigger */
	    counter = *values++;
	    break;

	  case XSyncCAValueType:
	    mask &= ~XSyncCAValueType;
	    /* sanity check in SyncInitTrigger */
	    pAlarm->trigger.value_type = *values++;
	    break;

	  case XSyncCAValue:
	    mask &= ~XSyncCAValue;
	    XSyncIntsToValue(&pAlarm->trigger.wait_value, values[1], values[0]);
	    values += 2;
	    break;

	  case XSyncCATestType:
	    mask &= ~XSyncCATestType;
	    /* sanity check in SyncInitTrigger */
	    pAlarm->trigger.test_type = *values++;
	    break;

	  case XSyncCADelta:
	    mask &= ~XSyncCADelta;
	    XSyncIntsToValue(&pAlarm->delta, values[1], values[0]);
	    values += 2;
	    break;

	  case XSyncCAEvents:
	    mask &= ~XSyncCAEvents;
	    if ((*values != xTrue) && (*values != xFalse))
	    {
		client->errorValue = *values;
		return BadValue;
	    }
	    status = SyncEventSelectForAlarm(pAlarm, client,
					     (Bool)(*values++));
	    if (status != Success)
		return status;
	    break;

	  default:
	    client->errorValue = mask;
	    return BadValue;
	}
    }

    /* "If the test-type is PositiveComparison or PositiveTransition
     *  and delta is less than zero, or if the test-type is
     *  NegativeComparison or NegativeTransition and delta is
     *  greater than zero, a Match error is generated."
     */
    if (origmask & (XSyncCADelta|XSyncCATestType))
    {
	CARD64 zero;
	XSyncIntToValue(&zero, 0);
	if ((((pAlarm->trigger.test_type == XSyncPositiveComparison) ||
	      (pAlarm->trigger.test_type == XSyncPositiveTransition))
	     && XSyncValueLessThan(pAlarm->delta, zero))
	    ||
	    (((pAlarm->trigger.test_type == XSyncNegativeComparison) ||
	      (pAlarm->trigger.test_type == XSyncNegativeTransition))
	     && XSyncValueGreaterThan(pAlarm->delta, zero))
	   )
	{
	    return BadMatch;
	}
    }

    /* postpone this until now, when we're sure nothing else can go wrong */
    if ((status = SyncInitTrigger(client, &pAlarm->trigger, counter,
			     origmask & XSyncCAAllTrigger)) != Success)
	return status;

    /* XXX spec does not really say to do this - needs clarification */
    pAlarm->state = XSyncAlarmActive;
    return Success;
}


static SyncCounter *
SyncCreateCounter(client, id, initialvalue)
    ClientPtr	client;
    XSyncCounter id;
    CARD64      initialvalue;
{
    SyncCounter *pCounter;

    if (!(pCounter = (SyncCounter *) xalloc(sizeof(SyncCounter))))
	return (SyncCounter *)NULL;

    if (!AddResource(id, RTCounter, (pointer) pCounter))
    {
	xfree((pointer) pCounter);
	return (SyncCounter *)NULL;
    }

    pCounter->client = client;
    pCounter->id = id;
    pCounter->value = initialvalue;
    pCounter->pTriglist = NULL;
    pCounter->beingDestroyed = FALSE;
    pCounter->pSysCounterInfo = NULL;
    return pCounter;
}

static int FreeCounter(
#if NeedFunctionPrototypes
    pointer /*env*/,
    XID     /*id*/
#endif
);

/*
 * ***** System Counter utilities
 */

pointer 
SyncCreateSystemCounter(name, initial, resolution, counterType,
			QueryValue, BracketValues)
    char           *name;
    CARD64          initial;
    CARD64          resolution;
    SyncCounterType counterType;
    void            (*QueryValue) ();
    void            (*BracketValues) ();
{
    SyncCounter    *pCounter;

    SysCounterList = (SyncCounter **)xrealloc(SysCounterList,
			    (SyncNumSystemCounters+1)*sizeof(SyncCounter *));
    if (!SysCounterList)
	return (pointer)NULL;

    /* this function may be called before SYNC has been initialized, so we
     * have to make sure RTCounter is created.
     */
    if (RTCounter == 0)
    {
	RTCounter = CreateNewResourceType(FreeCounter);
	if (RTCounter == 0)
	{
	    return (pointer)NULL;
	}
    }

    pCounter = SyncCreateCounter((ClientPtr)NULL, FakeClientID(0), initial);

    if (pCounter)
    {
	SysCounterInfo *psci;

	psci = (SysCounterInfo *)xalloc(sizeof(SysCounterInfo));
	if (!psci)
	{
	    FreeResource(pCounter->id, RT_NONE);
	    return (pointer) pCounter;
	}
	pCounter->pSysCounterInfo = psci;
	psci->name = name;
	psci->resolution = resolution;
	psci->counterType = counterType;
	psci->QueryValue = QueryValue;
	psci->BracketValues = BracketValues;
	XSyncMaxValue(&psci->bracket_greater);
	XSyncMinValue(&psci->bracket_less);
	SysCounterList[SyncNumSystemCounters++] = pCounter;
    }
    return (pointer) pCounter;
}

void
SyncDestroySystemCounter(pSysCounter)
    pointer pSysCounter;
{
    SyncCounter *pCounter = (SyncCounter *)pSysCounter;
    FreeResource(pCounter->id, RT_NONE);
}

static void
SyncComputeBracketValues(pCounter, startOver)
    SyncCounter *pCounter;
    Bool startOver;
{
    SyncTriggerList *pCur;
    SyncTrigger *pTrigger;
    SysCounterInfo *psci = pCounter->pSysCounterInfo;
    CARD64 *pnewgtval = NULL;
    CARD64 *pnewltval = NULL;
    SyncCounterType ct;

    if (!pCounter)
	return;

    ct = pCounter->pSysCounterInfo->counterType;
    if (ct == XSyncCounterNeverChanges)
	return;

    if (startOver)
    {
	XSyncMaxValue(&psci->bracket_greater);
	XSyncMinValue(&psci->bracket_less);
    }

    for (pCur = pCounter->pTriglist; pCur; pCur = pCur->next)
    {
	pTrigger = pCur->pTrigger;
	
        if (pTrigger->test_type == XSyncPositiveComparison &&
	    ct != XSyncCounterNeverIncreases)
	{
	    if (XSyncValueLessThan(pCounter->value, pTrigger->test_value) &&
		XSyncValueLessThan(pTrigger->test_value,
				   psci->bracket_greater))
	    {
		psci->bracket_greater = pTrigger->test_value;
		pnewgtval = &psci->bracket_greater;
	    }
	}
	else if (pTrigger->test_type == XSyncNegativeComparison &&
		 ct != XSyncCounterNeverDecreases)
	{
	    if (XSyncValueGreaterThan(pCounter->value, pTrigger->test_value) &&
		XSyncValueGreaterThan(pTrigger->test_value,
				      psci->bracket_less))
	    {
		psci->bracket_less = pTrigger->test_value;
		pnewltval = &psci->bracket_less;
	    }
	}
	else if ( (pTrigger->test_type == XSyncPositiveTransition &&
		   ct != XSyncCounterNeverIncreases)
		 ||
		 (pTrigger->test_type == XSyncNegativeTransition &&
		  ct != XSyncCounterNeverDecreases)
		 )
	{
	    if (XSyncValueLessThan(pCounter->value, pTrigger->test_value))
	    {
		if (XSyncValueLessThan(pTrigger->test_value,
				       psci->bracket_greater))
		{
		    psci->bracket_greater = pTrigger->test_value;
		    pnewgtval = &psci->bracket_greater;
		}
		else
		if (XSyncValueGreaterThan(pTrigger->test_value,
					  psci->bracket_less))
		{
		    psci->bracket_less = pTrigger->test_value;
		    pnewltval = &psci->bracket_less;
		}
	    }
	}
    } /* end for each trigger */

    if (pnewgtval || pnewltval)
    {
	(*psci->BracketValues)((pointer)pCounter, pnewltval, pnewgtval);
    }
}

/*
 * *****  Resource delete functions
 */

/* ARGSUSED */
static int
FreeAlarm(addr, id)
    pointer         addr;
    XID             id;
{
    SyncAlarm      *pAlarm = (SyncAlarm *) addr;

    pAlarm->state = XSyncAlarmDestroyed;

    SyncSendAlarmNotifyEvents(pAlarm);

    /* delete event selections */

    while (pAlarm->pEventClients)
	FreeResource(pAlarm->pEventClients->delete_id, RT_NONE);

    SyncDeleteTriggerFromCounter(&pAlarm->trigger);

    xfree(pAlarm);
    return Success;
}


/*
 * ** Cleanup after the destruction of a Counter
 */
/* ARGSUSED */
static int
FreeCounter(env, id)
    pointer         env;
    XID             id;
{
    SyncCounter     *pCounter = (SyncCounter *) env;
    SyncTriggerList *ptl, *pnext;

    pCounter->beingDestroyed = TRUE;
    /* tell all the counter's triggers that the counter has been destroyed */
    for (ptl = pCounter->pTriglist; ptl; ptl = pnext)
    {
	(*ptl->pTrigger->CounterDestroyed)(ptl->pTrigger);
	pnext = ptl->next;
	xfree(ptl); /* destroy the trigger list as we go */
    }
    if (IsSystemCounter(pCounter))
    {
	int i, found = 0;

	xfree(pCounter->pSysCounterInfo);

	/* find the counter in the list of system counters and remove it */

	if (SysCounterList)
	{
	    for (i = 0; i < SyncNumSystemCounters; i++)
	    {
		if (SysCounterList[i] == pCounter)
		{
		    found = i;
		    break;
		}
	    }
	    if (found < (SyncNumSystemCounters-1))
	    {
		for (i = found; i < SyncNumSystemCounters-1; i++)
		{
		    SysCounterList[i] = SysCounterList[i+1];
		}
	    }
	}
	SyncNumSystemCounters--;
    }
    xfree(pCounter);
    return Success;
}

/*
 * ** Cleanup after Await
 */
/* ARGSUSED */
static int
FreeAwait(addr, id)
    pointer         addr;
    XID             id;
{
    SyncAwaitUnion *pAwaitUnion = (SyncAwaitUnion *) addr;
    SyncAwait *pAwait;
    int numwaits;

    pAwait = &(pAwaitUnion+1)->await; /* first await on list */

    /* remove triggers from counters */

    for (numwaits = pAwaitUnion->header.num_waitconditions; numwaits;
	 numwaits--, pAwait++)
    {
	/* If the counter is being destroyed, FreeCounter will delete 
	 * the trigger list itself, so don't do it here.
	 */
	SyncCounter *pCounter = pAwait->trigger.pCounter;
	if (pCounter && !pCounter->beingDestroyed)
	    SyncDeleteTriggerFromCounter(&pAwait->trigger);
    }
    xfree(pAwaitUnion);
    return Success;
}

/* loosely based on dix/events.c/OtherClientGone */
static int
FreeAlarmClient(value, id)
    pointer value; /* must conform to DeleteType */
    XID   id;
{
    SyncAlarm *pAlarm = (SyncAlarm *)value;
    SyncAlarmClientList *pCur, *pPrev;

    for (pPrev = NULL, pCur = pAlarm->pEventClients;
	 pCur;
	 pPrev = pCur, pCur = pCur->next)
    {
	if (pCur->delete_id == id)
	{
	    if (pPrev)
		pPrev->next = pCur->next;
	    else
		pAlarm->pEventClients = pCur->next;
	    xfree(pCur);
	    return(Success);
	}
    }
    FatalError("alarm client not on event list");
    /*NOTREACHED*/
}


/*
 * *****  Proc functions
 */


/*
 * ** Initialize the extension
 */
static int 
ProcSyncInitialize(client)
    ClientPtr       client;
{
    xSyncInitializeReply  rep;
    int   n;

    REQUEST_SIZE_MATCH(xSyncInitializeReq);

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = SYNC_MAJOR_VERSION;
    rep.minorVersion = SYNC_MINOR_VERSION;
    rep.length = 0;

    if (client->swapped)
    {
	swaps(&rep.sequenceNumber, n);
    }
    WriteToClient(client, sizeof(rep), (char *) &rep);
    return (client->noClientException);
}

/*
 * ** Get list of system counters available through the extension
 */
static int 
ProcSyncListSystemCounters(client)
    ClientPtr       client;
{
    xSyncListSystemCountersReply  rep;
    int i, len;
    xSyncSystemCounter *list, *walklist;
    
    REQUEST_SIZE_MATCH(xSyncListSystemCountersReq);

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.nCounters = SyncNumSystemCounters;

    for (i = len = 0; i < SyncNumSystemCounters; i++)
    {
	char *name = SysCounterList[i]->pSysCounterInfo->name;
	/* pad to 4 byte boundary */
	len += (sz_xSyncSystemCounter + strlen(name) + 3) & ~3;
    }

    if (len)
    {
	walklist = list = (xSyncSystemCounter *) ALLOCATE_LOCAL(len);
	if (!list)
	    return BadAlloc;
    }

    rep.length = len >> 2;

    if (client->swapped)
    {
	register char n;
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swapl(&rep.nCounters, n);
    }

    for (i = 0; i < SyncNumSystemCounters; i++)
    {
	int namelen;
	char *pname_in_reply;
	SysCounterInfo *psci = SysCounterList[i]->pSysCounterInfo;

	walklist->counter = SysCounterList[i]->id;
	walklist->resolution_hi = XSyncValueHigh32(psci->resolution);
	walklist->resolution_lo = XSyncValueLow32(psci->resolution);
	namelen = strlen(psci->name);
	walklist->name_length = namelen;

	if (client->swapped)
	{
	    register char n;
	    swapl(&walklist->counter, n);
	    swapl(&walklist->resolution_hi, n);
	    swapl(&walklist->resolution_lo, n);
	    swaps(&walklist->name_length, n);
	}

	pname_in_reply = ((char *)walklist) + sz_xSyncSystemCounter;
	strncpy(pname_in_reply, psci->name, namelen);
	walklist = (xSyncSystemCounter *) (((char *)walklist) + 
				((sz_xSyncSystemCounter + namelen + 3) & ~3));
    }

    WriteToClient(client, sizeof(rep), (char *) &rep);
    if (len) 
    {
	WriteToClient(client, len, (char *) list);
	DEALLOCATE_LOCAL(list);
    }

    return (client->noClientException);
}

/*
 * ** Set client Priority
 */
static int 
ProcSyncSetPriority(client)
    ClientPtr       client;
{
    REQUEST(xSyncSetPriorityReq);
    ClientPtr priorityclient;

    REQUEST_SIZE_MATCH(xSyncSetPriorityReq);

    if (stuff->id == None)
	priorityclient = client;
    else if (!(priorityclient = LookupClient(stuff->id, client)))
    {
	client->errorValue = stuff->id;
	return BadMatch;
    }

    if (priorityclient->priority != stuff->priority)
    {
	priorityclient->priority = stuff->priority;

	/*  The following will force the server back into WaitForSomething
	 *  so that the change in this client's priority is immediately
	 *  reflected.
	 */
	isItTimeToYield = TRUE;
	dispatchException |= DE_PRIORITYCHANGE;
    }
    return Success;
}

/*
 * ** Get client Priority
 */
static int 
ProcSyncGetPriority(client)
    ClientPtr       client;
{
    REQUEST(xSyncGetPriorityReq);
    xSyncGetPriorityReply rep;
    ClientPtr priorityclient;

    REQUEST_SIZE_MATCH(xSyncGetPriorityReq);

    if (stuff->id == None)
	priorityclient = client;
    else if (!(priorityclient = LookupClient(stuff->id, client)))
    {
	client->errorValue = stuff->id;
	return BadMatch;
    }

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.priority = priorityclient->priority;

    if (client->swapped)
    {
	register char n;
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.priority, n);
    }

    WriteToClient(client, sizeof(xSyncGetPriorityReply), (char *) &rep);

    return (client->noClientException);
}

/*
 * ** Create a new counter
 */
static int 
ProcSyncCreateCounter(client)
    ClientPtr       client;
{
    REQUEST(xSyncCreateCounterReq);
    CARD64          initial;

    REQUEST_SIZE_MATCH(xSyncCreateCounterReq);

    LEGAL_NEW_RESOURCE(stuff->cid, client);

    XSyncIntsToValue(&initial, stuff->initial_value_lo, stuff->initial_value_hi);
    if (!SyncCreateCounter(client, stuff->cid, initial))
	return BadAlloc;

    return (client->noClientException);
}

/*
 * ** Set Counter value
 */
static int 
ProcSyncSetCounter(client)
    ClientPtr       client;
{
    REQUEST(xSyncSetCounterReq);
    SyncCounter    *pCounter;
    CARD64	   newvalue;

    pCounter = (SyncCounter *)SecurityLookupIDByType(client, stuff->cid,
					   RTCounter, SecurityWriteAccess);
    if (pCounter == NULL)
    {
	client->errorValue = stuff->cid;
	return SyncErrorBase + XSyncBadCounter;
    }

    if (IsSystemCounter(pCounter))
    {
	client->errorValue = stuff->cid;
	return BadAccess;
    }

    XSyncIntsToValue(&newvalue, stuff->value_lo, stuff->value_hi);
    SyncChangeCounter(pCounter, newvalue);
    return Success;
}

/*
 * ** Change Counter value
 */
static int 
ProcSyncChangeCounter(client)
    ClientPtr       client;
{
    REQUEST(xSyncChangeCounterReq);
    SyncCounter    *pCounter;
    CARD64          newvalue;
    Bool	    overflow;

    REQUEST_SIZE_MATCH(xSyncChangeCounterReq);

    pCounter = (SyncCounter *) SecurityLookupIDByType(client, stuff->cid,
					    RTCounter, SecurityWriteAccess);
    if (pCounter == NULL)
    {
	client->errorValue = stuff->cid;
	return SyncErrorBase + XSyncBadCounter;
    }

    if (IsSystemCounter(pCounter))
    {
	client->errorValue = stuff->cid;
	return BadAccess;
    }

    XSyncIntsToValue(&newvalue, stuff->value_lo, stuff->value_hi);
    XSyncValueAdd(&newvalue, pCounter->value, newvalue, &overflow);
    if (overflow)
    {
	/* XXX 64 bit value can't fit in 32 bits; do the best we can */
	client->errorValue = stuff->value_hi; 
	return BadValue;
    }
    SyncChangeCounter(pCounter, newvalue);
    return Success;
}

/*
 * ** Destroy a counter
 */
static int 
ProcSyncDestroyCounter(client)
    ClientPtr       client;
{
    REQUEST(xSyncDestroyCounterReq);
    SyncCounter    *pCounter;

    REQUEST_SIZE_MATCH(xSyncDestroyCounterReq);

    pCounter = (SyncCounter *)SecurityLookupIDByType(client, stuff->counter,
					   RTCounter, SecurityDestroyAccess);
    if (pCounter == NULL)
    {
	client->errorValue = stuff->counter;
	return SyncErrorBase + XSyncBadCounter;
    }
    if (IsSystemCounter(pCounter))
    {
	client->errorValue = stuff->counter;
	return BadAccess;
    }
    FreeResource(pCounter->id, RT_NONE);
    return Success;
}


/*
 * ** Await
 */
static int 
ProcSyncAwait(client)
    ClientPtr       client;
{
    REQUEST(xSyncAwaitReq);
    int             len, items;
    int             i;
    xSyncWaitCondition *pProtocolWaitConds;
    SyncAwaitUnion *pAwaitUnion;
    SyncAwait	   *pAwait;
    int		   status;

    REQUEST_AT_LEAST_SIZE(xSyncAwaitReq);

    len = client->req_len << 2;
    len -= sz_xSyncAwaitReq;
    items = len / sz_xSyncWaitCondition;

    if (items * sz_xSyncWaitCondition != len)
    {
	return BadLength;
    }
    if (items == 0)
    {
	client->errorValue = items; /* XXX protocol change */
	return BadValue;
    }

    pProtocolWaitConds = (xSyncWaitCondition *) & stuff[1];

    /*  all the memory for the entire await list is allocated 
     *  here in one chunk
     */
    pAwaitUnion = (SyncAwaitUnion *)xalloc((items+1) * sizeof(SyncAwaitUnion));
    if (!pAwaitUnion)
	return BadAlloc;

    /* first item is the header, remainder are real wait conditions */

    pAwaitUnion->header.delete_id = FakeClientID(client->index);
    if (!AddResource(pAwaitUnion->header.delete_id, RTAwait, pAwaitUnion))
    {
	xfree(pAwaitUnion);
	return BadAlloc;
    }

    /* don't need to do any more memory allocation for this request! */

    pAwaitUnion->header.client = client;
    pAwaitUnion->header.num_waitconditions = 0;

    pAwait = &(pAwaitUnion+1)->await; /* skip over header */
    for (i = 0; i < items; i++, pProtocolWaitConds++, pAwait++)
    {
	if (pProtocolWaitConds->counter == None) /* XXX protocol change */
	{
	    /*  this should take care of removing any triggers created by
	     *  this request that have already been registered on counters
	     */
	    FreeResource(pAwaitUnion->header.delete_id, RT_NONE);
	    client->errorValue = pProtocolWaitConds->counter;
	    return SyncErrorBase + XSyncBadCounter;
	}

	/* sanity checks are in SyncInitTrigger */
	pAwait->trigger.pCounter = NULL;
	pAwait->trigger.value_type = pProtocolWaitConds->value_type;
	XSyncIntsToValue(&pAwait->trigger.wait_value,
			 pProtocolWaitConds->wait_value_lo,
			 pProtocolWaitConds->wait_value_hi);
	pAwait->trigger.test_type = pProtocolWaitConds->test_type;

	status = SyncInitTrigger(client, &pAwait->trigger,
			 pProtocolWaitConds->counter, XSyncCAAllTrigger);
	if (status != Success)
	{
	    /*  this should take care of removing any triggers created by
	     *  this request that have already been registered on counters
	     */
	    FreeResource(pAwaitUnion->header.delete_id, RT_NONE);
	    return status;
	}
	/* this is not a mistake -- same function works for both cases */
	pAwait->trigger.TriggerFired = SyncAwaitTriggerFired;
	pAwait->trigger.CounterDestroyed = SyncAwaitTriggerFired;
	XSyncIntsToValue(&pAwait->event_threshold,
			 pProtocolWaitConds->event_threshold_lo,
			 pProtocolWaitConds->event_threshold_hi);
	pAwait->pHeader = &pAwaitUnion->header;
	pAwaitUnion->header.num_waitconditions++;
    }

    IgnoreClient(client);

    /* see if any of the triggers are already true */

    pAwait = &(pAwaitUnion+1)->await; /* skip over header */
    for (i = 0; i < items; i++, pAwait++)
    {
	/*  don't have to worry about NULL counters because the request
	 *  errors before we get here out if they occur
	 */
	if ((*pAwait->trigger.CheckTrigger)(&pAwait->trigger,
					    pAwait->trigger.pCounter->value))
	{
	    (*pAwait->trigger.TriggerFired)(&pAwait->trigger);
	    break; /* once is enough */
	}
    }
    return Success;
}


/*
 * ** Query a counter
 */
static int 
ProcSyncQueryCounter(client)
    ClientPtr       client;
{
    REQUEST(xSyncQueryCounterReq);
    xSyncQueryCounterReply rep;
    SyncCounter    *pCounter;

    REQUEST_SIZE_MATCH(xSyncQueryCounterReq);

    pCounter = (SyncCounter *)SecurityLookupIDByType(client, stuff->counter,
					    RTCounter, SecurityReadAccess);
    if (pCounter == NULL)
    {
	client->errorValue = stuff->counter;
	return SyncErrorBase + XSyncBadCounter;
    }

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    /* if system counter, ask it what the current value is */

    if (IsSystemCounter(pCounter))
    {
	(*pCounter->pSysCounterInfo->QueryValue) ((pointer) pCounter,
						  &pCounter->value);
    }

    rep.value_hi = XSyncValueHigh32(pCounter->value);
    rep.value_lo = XSyncValueLow32(pCounter->value);
    if (client->swapped)
    {
	register char n;
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swapl(&rep.value_hi, n);
	swapl(&rep.value_lo, n);
    }
    WriteToClient(client, sizeof(xSyncQueryCounterReply), (char *) &rep);
    return (client->noClientException);
}


/*
 * ** Create Alarm
 */
static int 
ProcSyncCreateAlarm(client)
    ClientPtr       client;
{
    REQUEST(xSyncCreateAlarmReq);
    SyncAlarm      *pAlarm;
    int             status;
    unsigned long   len, vmask;
    SyncTrigger	    *pTrigger;

    REQUEST_AT_LEAST_SIZE(xSyncCreateAlarmReq);

    LEGAL_NEW_RESOURCE(stuff->id, client);

    vmask = stuff->valueMask;
    len = client->req_len - (sizeof(xSyncCreateAlarmReq) >> 2);
    /* the "extra" call to Ones accounts for the presence of 64 bit values */
    if (len != (Ones(vmask) + Ones(vmask & (XSyncCAValue|XSyncCADelta))))
	return BadLength;

    if (!(pAlarm = (SyncAlarm *) xalloc(sizeof(SyncAlarm))))
    {
	return BadAlloc;
    }

    /* set up defaults */

    pTrigger = &pAlarm->trigger;
    pTrigger->pCounter = NULL;
    pTrigger->value_type = XSyncAbsolute;
    XSyncIntToValue(&pTrigger->wait_value, 0L);
    pTrigger->test_type = XSyncPositiveComparison;
    pTrigger->TriggerFired = SyncAlarmTriggerFired;
    pTrigger->CounterDestroyed = SyncAlarmCounterDestroyed;
    status = SyncInitTrigger(client, pTrigger, None, XSyncCAAllTrigger);
    if (status != Success)
    {
	xfree(pAlarm);
	return status;
    }

    pAlarm->client = client;
    pAlarm->alarm_id = stuff->id;
    XSyncIntToValue(&pAlarm->delta, 1L);
    pAlarm->events = TRUE;
    pAlarm->state = XSyncAlarmInactive;
    pAlarm->pEventClients = NULL;
    status = SyncChangeAlarmAttributes(client, pAlarm, vmask,
				       (CARD32 *)&stuff[1]);
    if (status != Success)
    {
	xfree(pAlarm);
	return status;
    }

    if (!AddResource(stuff->id, RTAlarm, pAlarm))
    {
	xfree(pAlarm);
	return BadAlloc;
    }

    /*  see if alarm already triggered.  NULL counter will not trigger
     *  in CreateAlarm and sets alarm state to Inactive.
     */

    if (!pTrigger->pCounter)
    {
	pAlarm->state = XSyncAlarmInactive; /* XXX protocol change */
    }
    else if ((*pTrigger->CheckTrigger)(pTrigger, pTrigger->pCounter->value))
    {
	(*pTrigger->TriggerFired)(pTrigger);
    }

    return Success;
}

/*
 * ** Change Alarm
 */
static int 
ProcSyncChangeAlarm(client)
    ClientPtr       client;
{
    REQUEST(xSyncChangeAlarmReq);
    SyncAlarm   *pAlarm;
    long        vmask;
    int         len, status;

    REQUEST_AT_LEAST_SIZE(xSyncChangeAlarmReq);

    if (!(pAlarm = (SyncAlarm *)SecurityLookupIDByType(client, stuff->alarm,
					      RTAlarm, SecurityWriteAccess)))
    {
	client->errorValue = stuff->alarm;
	return SyncErrorBase + XSyncBadAlarm;
    }

    vmask = stuff->valueMask;
    len = client->req_len - (sizeof(xSyncChangeAlarmReq) >> 2);
    /* the "extra" call to Ones accounts for the presence of 64 bit values */
    if (len != (Ones(vmask) + Ones(vmask & (XSyncCAValue|XSyncCADelta))))
	return BadLength;

    if ((status = SyncChangeAlarmAttributes(client, pAlarm, vmask, 
					    (CARD32 *)&stuff[1])) != Success)
	return status;

    /*  see if alarm already triggered.  NULL counter WILL trigger
     *  in ChangeAlarm.
     */

    if (!pAlarm->trigger.pCounter ||
	(*pAlarm->trigger.CheckTrigger)(&pAlarm->trigger,
					pAlarm->trigger.pCounter->value))
    {
	(*pAlarm->trigger.TriggerFired)(&pAlarm->trigger);
    }
    return Success;
}

static int 
ProcSyncQueryAlarm(client)
    ClientPtr       client;
{
    REQUEST(xSyncQueryAlarmReq);
    SyncAlarm      *pAlarm;
    xSyncQueryAlarmReply rep;
    SyncTrigger    *pTrigger;

    REQUEST_SIZE_MATCH(xSyncQueryAlarmReq);

    pAlarm = (SyncAlarm *)SecurityLookupIDByType(client, stuff->alarm,
						RTAlarm, SecurityReadAccess);
    if (!pAlarm)
    {
	client->errorValue = stuff->alarm;
	return (SyncErrorBase + XSyncBadAlarm);
    }

    rep.type = X_Reply;
    rep.length = (sizeof(xSyncQueryAlarmReply) - sizeof(xGenericReply)) >> 2;
    rep.sequenceNumber = client->sequence;

    pTrigger = &pAlarm->trigger;
    rep.counter = (pTrigger->pCounter) ? pTrigger->pCounter->id : None;

#if 0 /* XXX unclear what to do, depends on whether relative value-types
       * are "consumed" immediately and are considered absolute from then
       * on.
       */
    rep.value_type = pTrigger->value_type;
    rep.wait_value_hi = XSyncValueHigh32(pTrigger->wait_value);
    rep.wait_value_lo = XSyncValueLow32(pTrigger->wait_value);
#else
    rep.value_type = XSyncAbsolute;
    rep.wait_value_hi = XSyncValueHigh32(pTrigger->test_value);
    rep.wait_value_lo = XSyncValueLow32(pTrigger->test_value);
#endif

    rep.test_type = pTrigger->test_type;
    rep.delta_hi = XSyncValueHigh32(pAlarm->delta);
    rep.delta_lo = XSyncValueLow32(pAlarm->delta);
    rep.events = pAlarm->events;
    rep.state = pAlarm->state;

    if (client->swapped)
    {
	register char n;
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swapl(&rep.counter, n);
	swapl(&rep.wait_value_hi, n);
	swapl(&rep.wait_value_lo, n);
	swapl(&rep.test_type, n);
	swapl(&rep.delta_hi, n);
	swapl(&rep.delta_lo, n);
    }

    WriteToClient(client, sizeof(xSyncQueryAlarmReply), (char *) &rep);
    return (client->noClientException);
}


static int 
ProcSyncDestroyAlarm(client)
    ClientPtr       client;
{
    SyncAlarm      *pAlarm;
    REQUEST(xSyncDestroyAlarmReq);

    REQUEST_SIZE_MATCH(xSyncDestroyAlarmReq);

    if (!(pAlarm = (SyncAlarm *)SecurityLookupIDByType(client, stuff->alarm,
					      RTAlarm, SecurityDestroyAccess)))
    {
	client->errorValue = stuff->alarm;
	return SyncErrorBase + XSyncBadAlarm;
    }

    FreeResource(stuff->alarm, RT_NONE);
    return (client->noClientException);
}

/*
 * ** Given an extension request, call the appropriate request procedure
 */
static int 
ProcSyncDispatch(client)
    ClientPtr       client;
{
    REQUEST(xReq);

    switch (stuff->data)
    {

      case X_SyncInitialize:
	return ProcSyncInitialize(client);
      case X_SyncListSystemCounters:
	return ProcSyncListSystemCounters(client);
      case X_SyncCreateCounter:
	return ProcSyncCreateCounter(client);
      case X_SyncSetCounter:
	return ProcSyncSetCounter(client);
      case X_SyncChangeCounter:
	return ProcSyncChangeCounter(client);
      case X_SyncQueryCounter:
	return ProcSyncQueryCounter(client);
      case X_SyncDestroyCounter:
	return ProcSyncDestroyCounter(client);
      case X_SyncAwait:
	return ProcSyncAwait(client);
      case X_SyncCreateAlarm:
	return ProcSyncCreateAlarm(client);
      case X_SyncChangeAlarm:
	return ProcSyncChangeAlarm(client);
      case X_SyncQueryAlarm:
	return ProcSyncQueryAlarm(client);
      case X_SyncDestroyAlarm:
	return ProcSyncDestroyAlarm(client);
      case X_SyncSetPriority:
	return ProcSyncSetPriority(client);
      case X_SyncGetPriority:
	return ProcSyncGetPriority(client);
      default:
	return BadRequest;
    }
}

/*
 * Boring Swapping stuff ...
 */

static int 
SProcSyncInitialize(client)
    ClientPtr       client;
{
    REQUEST(xSyncInitializeReq);
    register char   n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH (xSyncInitializeReq);

    return ProcSyncInitialize(client);
}

static int 
SProcSyncListSystemCounters(client)
    ClientPtr       client;
{
    REQUEST(xSyncListSystemCountersReq);
    register char   n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH (xSyncListSystemCountersReq);

    return ProcSyncListSystemCounters(client);
}

static int 
SProcSyncCreateCounter(client)
    ClientPtr       client;
{
    REQUEST(xSyncCreateCounterReq);
    register char   n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH (xSyncCreateCounterReq);
    swapl(&stuff->cid, n);
    swapl(&stuff->initial_value_lo, n);
    swapl(&stuff->initial_value_hi, n);

    return ProcSyncCreateCounter(client);
}

static int 
SProcSyncSetCounter(client)
    ClientPtr       client;
{
    REQUEST(xSyncSetCounterReq);
    register char   n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH (xSyncSetCounterReq);
    swapl(&stuff->cid, n);
    swapl(&stuff->value_lo, n);
    swapl(&stuff->value_hi, n);

    return ProcSyncSetCounter(client);
}

static int 
SProcSyncChangeCounter(client)
    ClientPtr       client;
{
    REQUEST(xSyncChangeCounterReq);
    register char   n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH (xSyncChangeCounterReq);
    swapl(&stuff->cid, n);
    swapl(&stuff->value_lo, n);
    swapl(&stuff->value_hi, n);

    return ProcSyncChangeCounter(client);
}

static int 
SProcSyncQueryCounter(client)
    ClientPtr       client;
{
    REQUEST(xSyncQueryCounterReq);
    register char   n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH (xSyncQueryCounterReq);
    swapl(&stuff->counter, n);

    return ProcSyncQueryCounter(client);
}

static int 
SProcSyncDestroyCounter(client)
    ClientPtr       client;
{
    REQUEST(xSyncDestroyCounterReq);
    register char   n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH (xSyncDestroyCounterReq);
    swapl(&stuff->counter, n);

    return ProcSyncDestroyCounter(client);
}

static int 
SProcSyncAwait(client)
    ClientPtr       client;
{
    REQUEST(xSyncAwaitReq);
    register char   n;

    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xSyncAwaitReq);
    SwapRestL(stuff);

    return ProcSyncAwait(client);
}


static int 
SProcSyncCreateAlarm(client)
    ClientPtr       client;
{
    REQUEST(xSyncCreateAlarmReq);
    register char   n;

    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xSyncCreateAlarmReq);
    swapl(&stuff->id, n);
    swapl(&stuff->valueMask, n);
    SwapRestL(stuff);

    return ProcSyncCreateAlarm(client);
}

static int 
SProcSyncChangeAlarm(client)
    ClientPtr       client;
{
    REQUEST(xSyncChangeAlarmReq);
    register char   n;

    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xSyncChangeAlarmReq);
    swapl(&stuff->alarm, n);
    swapl(&stuff->valueMask, n);
    SwapRestL(stuff);
    return ProcSyncChangeAlarm(client);
}

static int 
SProcSyncQueryAlarm(client)
    ClientPtr       client;
{
    REQUEST(xSyncQueryAlarmReq);
    register char   n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH (xSyncQueryAlarmReq);
    swapl(&stuff->alarm, n);

    return ProcSyncQueryAlarm(client);
}

static int 
SProcSyncDestroyAlarm(client)
    ClientPtr       client;
{
    REQUEST(xSyncDestroyAlarmReq);
    register char   n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH (xSyncDestroyAlarmReq);
    swapl(&stuff->alarm, n);

    return ProcSyncDestroyAlarm(client);
}

static int 
SProcSyncSetPriority(client)
    ClientPtr       client;
{
    REQUEST(xSyncSetPriorityReq);
    register char   n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH (xSyncSetPriorityReq);
    swapl(&stuff->id, n);
    swapl(&stuff->priority, n);

    return ProcSyncSetPriority(client);
}

static int 
SProcSyncGetPriority(client)
    ClientPtr       client;
{
    REQUEST(xSyncGetPriorityReq);
    register char   n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH (xSyncGetPriorityReq);
    swapl(&stuff->id, n);

    return ProcSyncGetPriority(client);
}


static int 
SProcSyncDispatch(client)
    ClientPtr       client;
{
    REQUEST(xReq);

    switch (stuff->data)
    {
      case X_SyncInitialize:
	return SProcSyncInitialize(client);
      case X_SyncListSystemCounters:
	return SProcSyncListSystemCounters(client);
      case X_SyncCreateCounter:
	return SProcSyncCreateCounter(client);
      case X_SyncSetCounter:
	return SProcSyncSetCounter(client);
      case X_SyncChangeCounter:
	return SProcSyncChangeCounter(client);
      case X_SyncQueryCounter:
	return SProcSyncQueryCounter(client);
      case X_SyncDestroyCounter:
	return SProcSyncDestroyCounter(client);
      case X_SyncAwait:
	return SProcSyncAwait(client);
      case X_SyncCreateAlarm:
	return SProcSyncCreateAlarm(client);
      case X_SyncChangeAlarm:
	return SProcSyncChangeAlarm(client);
      case X_SyncQueryAlarm:
	return SProcSyncQueryAlarm(client);
      case X_SyncDestroyAlarm:
	return SProcSyncDestroyAlarm(client);
      case X_SyncSetPriority:
	return SProcSyncSetPriority(client);
      case X_SyncGetPriority:
	return SProcSyncGetPriority(client);
      default:
	return BadRequest;
    }
}

/*
 * Event Swapping
 */

static void 
SCounterNotifyEvent(from, to)
    xSyncCounterNotifyEvent *from, *to;
{
    to->type = from->type;
    to->kind = from->kind;
    cpswaps(from->sequenceNumber, to->sequenceNumber);
    cpswapl(from->counter, to->counter);
    cpswapl(from->wait_value_lo, to->wait_value_lo);
    cpswapl(from->wait_value_hi, to->wait_value_hi);
    cpswapl(from->counter_value_lo, to->counter_value_lo);
    cpswapl(from->counter_value_hi, to->counter_value_hi);
    cpswapl(from->time, to->time);
    cpswaps(from->count, to->count);
    to->destroyed = from->destroyed;
}


static void 
SAlarmNotifyEvent(from, to)
    xSyncAlarmNotifyEvent *from, *to;
{
    to->type = from->type;
    to->kind = from->kind;
    cpswaps(from->sequenceNumber, to->sequenceNumber);
    cpswapl(from->alarm, to->alarm);
    cpswapl(from->counter_value_lo, to->counter_value_lo);
    cpswapl(from->counter_value_hi, to->counter_value_hi);
    cpswapl(from->alarm_value_lo, to->alarm_value_lo);
    cpswapl(from->alarm_value_hi, to->alarm_value_hi);
    cpswapl(from->time, to->time);
    to->state = from->state;
}

/*
 * ** Close everything down. ** This is fairly simple for now.
 */
/* ARGSUSED */
static void 
SyncResetProc(extEntry)
    ExtensionEntry *extEntry;
{
    xfree(SysCounterList);
    SysCounterList = NULL;
    RTCounter = 0;
}


/*
 * ** Initialise the extension.
 */
void 
SyncExtensionInit()
{
    ExtensionEntry *extEntry;

    if (RTCounter == 0)
    {
	RTCounter = CreateNewResourceType(FreeCounter);
    }
    RTAlarm = CreateNewResourceType(FreeAlarm);
    RTAwait = CreateNewResourceType(FreeAwait)|RC_NEVERRETAIN;
    RTAlarmClient = CreateNewResourceType(FreeAlarmClient)|RC_NEVERRETAIN;

    if (RTCounter == 0 || RTAwait == 0 || RTAlarm == 0 ||
	RTAlarmClient == 0 ||
	(extEntry = AddExtension(SYNC_NAME,
				 XSyncNumberEvents, XSyncNumberErrors,
				 ProcSyncDispatch, SProcSyncDispatch,
				 SyncResetProc,
				 StandardMinorOpcode)) == NULL)
    {
	ErrorF("Sync Extension %d.%d failed to Initialise\n",
		SYNC_MAJOR_VERSION, SYNC_MINOR_VERSION);
	return;
    }

    SyncReqCode = extEntry->base;
    SyncEventBase = extEntry->eventBase;
    SyncErrorBase = extEntry->errorBase;
    EventSwapVector[SyncEventBase + XSyncCounterNotify] = (EventSwapPtr) SCounterNotifyEvent;
    EventSwapVector[SyncEventBase + XSyncAlarmNotify] = (EventSwapPtr) SAlarmNotifyEvent;

    /*
     * Although SERVERTIME is implemented by the OS layer, we initialise it
     * here because doing it in OsInit() is too early. The resource database
     * is not initialised when OsInit() is called. This is just about OK
     * because there is always a servertime counter.
     */
    SyncInitServerTime();

#ifdef DEBUG
    fprintf(stderr, "Sync Extension %d.%d\n",
	    SYNC_MAJOR_VERSION, SYNC_MINOR_VERSION);
#endif
}


/*
 * ***** SERVERTIME implementation - should go in its own file in OS directory?
 */


#if !defined(WIN32) && !defined(MINIX) && !defined(Lynx)
#include <sys/time.h>
#endif

static pointer ServertimeCounter;
static XSyncValue Now;
static XSyncValue *pnext_time;

#define GetTime()\
{\
    unsigned long millis = GetTimeInMillis();\
    unsigned long maxis = XSyncValueHigh32(Now);\
    if (millis < XSyncValueLow32(Now)) maxis++;\
    XSyncIntsToValue(&Now, millis, maxis);\
}

/*
*** Server Block Handler
*** code inspired by multibuffer extension
 */
/*ARGSUSED*/
static void ServertimeBlockHandler(env, wt, LastSelectMask)
pointer env;
struct timeval **wt;
pointer LastSelectMask;
{
    XSyncValue delay;
    unsigned long timeout;

    if (pnext_time)
    {
        GetTime();

        if (XSyncValueGreaterOrEqual(Now, *pnext_time))
	{
            timeout = 0;
        } 
	else
	{
	    Bool overflow;
            XSyncValueSubtract(&delay, *pnext_time, Now, &overflow);
            timeout = XSyncValueLow32(delay);
        }
        AdjustWaitForDelay(wt, timeout); /* os/utils.c */
    }
}

/*
*** Wakeup Handler
 */
/*ARGSUSED*/
static void ServertimeWakeupHandler(env, rc, LastSelectMask)
pointer env;
int rc;
pointer LastSelectMask;
{
    if (pnext_time)
    {
        GetTime();

        if (XSyncValueGreaterOrEqual(Now, *pnext_time))
	{
            SyncChangeCounter(ServertimeCounter, Now);
        }
    }
}

static void
ServertimeQueryValue(pCounter, pValue_return)
    pointer pCounter;
    CARD64 *pValue_return;
{
    GetTime();
    *pValue_return = Now;
}

static void
ServertimeBracketValues(pCounter, pbracket_less, pbracket_greater)
    pointer pCounter;
    CARD64 *pbracket_less;
    CARD64 *pbracket_greater;
{
    if (!pnext_time && pbracket_greater)
    {
	RegisterBlockAndWakeupHandlers(ServertimeBlockHandler,
				       ServertimeWakeupHandler,
				       NULL);
    }
    else if (pnext_time && !pbracket_greater)
    {
	RemoveBlockAndWakeupHandlers(ServertimeBlockHandler,
				     ServertimeWakeupHandler,
				     NULL);
    }
    pnext_time = pbracket_greater;
}

static void
SyncInitServerTime()
{
    CARD64 resolution;

    XSyncIntsToValue(&Now, GetTimeInMillis(), 0);
    XSyncIntToValue(&resolution, 4);
    ServertimeCounter = SyncCreateSystemCounter("SERVERTIME", Now, resolution,
			    XSyncCounterNeverDecreases,
			    ServertimeQueryValue, ServertimeBracketValues);
    pnext_time = NULL;
}
