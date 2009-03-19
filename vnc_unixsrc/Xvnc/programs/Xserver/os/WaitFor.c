/***********************************************************

Copyright (c) 1987  X Consortium

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

******************************************************************/

/* $XConsortium: WaitFor.c /main/55 1996/12/02 10:22:24 lehors $ */
/* $XFree86: xc/programs/Xserver/os/WaitFor.c,v 3.11.2.3 1998/01/31 14:23:33 hohndel Exp $ */

/*****************************************************************
 * OS Dependent input routines:
 *
 *  WaitForSomething
 *  TimerForce, TimerSet, TimerCheck, TimerFree
 *
 *****************************************************************/

#ifdef WIN32
#include <X11/Xwinsock.h>
#endif
#include "Xos.h"			/* for strings, fcntl, time */

#include <errno.h>
#ifdef X_NOT_STDC_ENV
extern int errno;
#endif

#include <stdio.h>
#include "X.h"
#include "misc.h"

#ifdef MINIX
#include <sys/nbio.h>
#define select(n,r,w,x,t) nbio_select(n,r,w,x,t)
#endif
#ifdef __EMX__
#define select(n,r,w,x,t) os2PseudoSelect(n,r,w,x,t)
#endif
#include <X11/Xpoll.h>
#include "osdep.h"
#include "dixstruct.h"
#include "opaque.h"

#ifdef DPMSExtension
#include "dpms.h"
extern void DPMSSet();
#endif

extern fd_set AllSockets;
extern fd_set AllClients;
extern fd_set LastSelectMask;
extern fd_set WellKnownConnections;
extern fd_set EnabledDevices;
extern fd_set ClientsWithInput;
extern fd_set ClientsWriteBlocked;
extern fd_set OutputPending;

extern int ConnectionTranslation[];

extern Bool NewOutputPending;
extern Bool AnyClientsWriteBlocked;

extern WorkQueuePtr workQueue;


#ifdef XTESTEXT1
/*
 * defined in xtestext1dd.c
 */
extern int playback_on;
#endif /* XTESTEXT1 */

struct _OsTimerRec {
    OsTimerPtr		next;
    CARD32		expires;
    OsTimerCallback	callback;
    pointer		arg;
};

static void DoTimer();
static OsTimerPtr timers;

/*****************
 * WaitForSomething:
 *     Make the server suspend until there is
 *	1. data from clients or
 *	2. input events available or
 *	3. ddx notices something of interest (graphics
 *	   queue ready, etc.) or
 *	4. clients that have buffered replies/events are ready
 *
 *     If the time between INPUT events is
 *     greater than ScreenSaverTime, the display is turned off (or
 *     saved, depending on the hardware).  So, WaitForSomething()
 *     has to handle this also (that's why the select() has a timeout.
 *     For more info on ClientsWithInput, see ReadRequestFromClient().
 *     pClientsReady is an array to store ready client->index values into.
 *****************/

static INT32 timeTilFrob = 0;		/* while screen saving */

#if !defined(AMOEBA)

int
WaitForSomething(pClientsReady)
    int *pClientsReady;
{
    int i;
    struct timeval waittime, *wt;
    INT32 timeout;
#ifdef DPMSExtension
    INT32 standbyTimeout, suspendTimeout, offTimeout;
#endif
    fd_set clientsReadable;
    fd_set clientsWritable;
    int curclient;
    int selecterr;
    int nready;
    fd_set devicesReadable;
    CARD32 now;

    FD_ZERO(&clientsReadable);

    /* We need a while loop here to handle 
       crashed connections and the screen saver timeout */
    while (1)
    {
	/* deal with any blocked jobs */
	if (workQueue)
	    ProcessWorkQueue();

	if (XFD_ANYSET (&ClientsWithInput))
	{
	    XFD_COPYSET (&ClientsWithInput, &clientsReadable);
	    break;
	}
#ifdef DPMSExtension
	if (ScreenSaverTime > 0 || DPMSEnabled || timers)
#else
	if (ScreenSaverTime > 0 || timers)
#endif
	    now = GetTimeInMillis();
	wt = NULL;
	if (timers)
	{
	    while (timers && timers->expires <= now)
		DoTimer(timers, now, &timers);
	    if (timers)
	    {
		timeout = timers->expires - now;
		waittime.tv_sec = timeout / MILLI_PER_SECOND;
		waittime.tv_usec = (timeout % MILLI_PER_SECOND) *
		    (1000000 / MILLI_PER_SECOND);
		wt = &waittime;
	    }
	}
#ifdef DPMSExtension
	if (ScreenSaverTime > 0 ||
	    (DPMSEnabled &&
	     (DPMSStandbyTime > 0 || DPMSSuspendTime > 0 || DPMSOffTime > 0)))
#else
	if (ScreenSaverTime > 0)
#endif
	{
#ifdef DPMSExtension

	    if (ScreenSaverTime > 0)
		timeout = (ScreenSaverTime -
			   (now - lastDeviceEventTime.milliseconds));
	    if (DPMSStandbyTime > 0)
		standbyTimeout = (DPMSStandbyTime -
				  (now - lastDeviceEventTime.milliseconds));
	    if (DPMSSuspendTime > 0)
		suspendTimeout = (DPMSSuspendTime -
				  (now - lastDeviceEventTime.milliseconds));
	    if (DPMSOffTime > 0)
		offTimeout = (DPMSOffTime -
			      (now - lastDeviceEventTime.milliseconds));
#else
	    timeout = (ScreenSaverTime -
		       (now - lastDeviceEventTime.milliseconds));
#endif /* DPMSExtension */
#ifdef DPMSExtension
	    if (timeout <= 0 && ScreenSaverTime > 0)
#else
	    if (timeout <= 0) /* may be forced by AutoResetServer() */
#endif /* DPMSExtension */
	    {
		INT32 timeSinceSave;

		timeSinceSave = -timeout;
		if (timeSinceSave >= timeTilFrob && timeTilFrob >= 0)
		{
		    ResetOsBuffers(); /* not ideal, but better than nothing */
		    SaveScreens(SCREEN_SAVER_ON, ScreenSaverActive);
#ifdef DPMSExtension
		    if (ScreenSaverInterval > 0 &&
			DPMSPowerLevel == DPMSModeOn)
#else
		    if (ScreenSaverInterval)
#endif /* DPMSExtension */
			/* round up to the next ScreenSaverInterval */
			timeTilFrob = ScreenSaverInterval *
				((timeSinceSave + ScreenSaverInterval) /
					ScreenSaverInterval);
		    else
			timeTilFrob = -1;
		}
		timeout = timeTilFrob - timeSinceSave;
	    }
	    else
	    {
		if (ScreenSaverTime > 0 && timeout > ScreenSaverTime)
		    timeout = ScreenSaverTime;
		timeTilFrob = 0;
	    }
#ifdef DPMSExtension
	    if (DPMSEnabled)
	    {
		if (standbyTimeout > 0 
		    && (timeout <= 0 || timeout > standbyTimeout))
		    timeout = standbyTimeout;
		if (suspendTimeout > 0 
		    && (timeout <= 0 || timeout > suspendTimeout))
		    timeout = suspendTimeout;
		if (offTimeout > 0 
		    && (timeout <= 0 || timeout > offTimeout))
		    timeout = offTimeout;
	    }
#endif
	    if (timeout > 0 && (!wt || timeout < (timers->expires - now)))
	    {
		waittime.tv_sec = timeout / MILLI_PER_SECOND;
		waittime.tv_usec = (timeout % MILLI_PER_SECOND) *
					(1000000 / MILLI_PER_SECOND);
		wt = &waittime;
	    }
#ifdef DPMSExtension
	    /* don't bother unless it's switched on */
	    if (DPMSEnabled)
	    {
		/*
		 * If this mode's enabled, and if the time's come
		 * and if we're still at a lesser mode, do it now.
		 */
		if (DPMSStandbyTime > 0) {
		    if (standbyTimeout <= 0) {
			if (DPMSPowerLevel < DPMSModeStandby) {
			    DPMSSet(DPMSModeStandby);
			}
		    }
		}
		/*
		 * and ditto.  Note that since these modes can have the
		 * same timeouts, they can happen at the same time.
		 */
		if (DPMSSuspendTime > 0) {
		    if (suspendTimeout <= 0) {
			if (DPMSPowerLevel < DPMSModeSuspend) {
			    DPMSSet(DPMSModeSuspend);
			}
		    }
		}
		if (DPMSOffTime > 0) {
		    if (offTimeout <= 0) {
			if (DPMSPowerLevel < DPMSModeOff) {
			    DPMSSet(DPMSModeOff);
			}
		    }
		}
           }
#endif
	}
	XFD_COPYSET(&AllSockets, &LastSelectMask);
	BlockHandler((pointer)&wt, (pointer)&LastSelectMask);
	if (NewOutputPending)
	    FlushAllOutput();
#ifdef XTESTEXT1
	/* XXX how does this interact with new write block handling? */
	if (playback_on) {
	    wt = &waittime;
	    XTestComputeWaitTime (&waittime);
	}
#endif /* XTESTEXT1 */
	/* keep this check close to select() call to minimize race */
	if (dispatchException)
	    i = -1;
	else if (AnyClientsWriteBlocked)
	{
	    XFD_COPYSET(&ClientsWriteBlocked, &clientsWritable);
	    i = Select (MAXSOCKS, &LastSelectMask, &clientsWritable, NULL, wt);
	}
	else
	    i = Select (MAXSOCKS, &LastSelectMask, NULL, NULL, wt);
	selecterr = errno;
	WakeupHandler(i, (pointer)&LastSelectMask);
#ifdef XTESTEXT1
	if (playback_on) {
	    i = XTestProcessInputAction (i, &waittime);
	}
#endif /* XTESTEXT1 */
	if (i <= 0) /* An error or timeout occurred */
	{

	    if (dispatchException)
		return 0;
	    FD_ZERO(&clientsWritable);
	    if (i < 0) 
		if (selecterr == EBADF)    /* Some client disconnected */
		{
		    CheckConnections ();
		    if (! XFD_ANYSET (&AllClients))
			return 0;
		}
		else if (selecterr == EINVAL)
		{
		    FatalError("WaitForSomething(): select: errno=%d\n",
			selecterr);
		}
		else if (selecterr != EINTR)
		{
		    ErrorF("WaitForSomething(): select: errno=%d\n",
			selecterr);
		}
	    if (timers)
	    {
		now = GetTimeInMillis();
		while (timers && timers->expires <= now)
		    DoTimer(timers, now, &timers);
	    }
	    if (*checkForInput[0] != *checkForInput[1])
		return 0;
	}
	else
	{
#ifdef WIN32
	    fd_set tmp_set;
#endif
	    if (AnyClientsWriteBlocked && XFD_ANYSET (&clientsWritable))
	    {
		NewOutputPending = TRUE;
		XFD_ORSET(&OutputPending, &clientsWritable, &OutputPending);
		XFD_UNSET(&ClientsWriteBlocked, &clientsWritable);
		if (! XFD_ANYSET(&ClientsWriteBlocked))
		    AnyClientsWriteBlocked = FALSE;
	    }

	    XFD_ANDSET(&devicesReadable, &LastSelectMask, &EnabledDevices);
	    XFD_ANDSET(&clientsReadable, &LastSelectMask, &AllClients); 
#ifndef WIN32
	    if (LastSelectMask.fds_bits[0] & WellKnownConnections.fds_bits[0]) 
#else
	    XFD_ANDSET(&tmp_set, &LastSelectMask, &WellKnownConnections);
	    if (XFD_ANYSET(&tmp_set))
#endif
		QueueWorkProc(EstablishNewConnections, NULL,
			      (pointer)&LastSelectMask);
#ifdef DPMSExtension
	    if (XFD_ANYSET (&devicesReadable) && (DPMSPowerLevel != DPMSModeOn))
		DPMSSet(DPMSModeOn);
#endif
	    if (XFD_ANYSET (&devicesReadable) || XFD_ANYSET (&clientsReadable))
		break;
	}
    }

    nready = 0;
    if (XFD_ANYSET (&clientsReadable))
    {
#ifndef WIN32
	for (i=0; i<howmany(XFD_SETSIZE, NFDBITS); i++)
	{
	    int highest_priority;

	    while (clientsReadable.fds_bits[i])
	    {
	        int client_priority, client_index;

		curclient = ffsl(clientsReadable.fds_bits[i]) - 1;
		client_index = ConnectionTranslation[curclient + (i * sizeof(fd_mask) * 8)];
#else
	int highest_priority;
	fd_set savedClientsReadable;
	XFD_COPYSET(&clientsReadable, &savedClientsReadable);
	for (i = 0; i < XFD_SETCOUNT(&savedClientsReadable); i++)
	{
	    int client_priority, client_index;

	    curclient = XFD_FD(&savedClientsReadable, i);
	    client_index = ConnectionTranslation[curclient];
#endif
#ifdef XSYNC
		/*  We implement "strict" priorities.
		 *  Only the highest priority client is returned to
		 *  dix.  If multiple clients at the same priority are
		 *  ready, they are all returned.  This means that an
		 *  aggressive client could take over the server.
		 *  This was not considered a big problem because
		 *  aggressive clients can hose the server in so many 
		 *  other ways :)
		 */
		client_priority = clients[client_index]->priority;
		if (nready == 0 || client_priority > highest_priority)
		{
		    /*  Either we found the first client, or we found
		     *  a client whose priority is greater than all others
		     *  that have been found so far.  Either way, we want 
		     *  to initialize the list of clients to contain just
		     *  this client.
		     */
		    pClientsReady[0] = client_index;
		    highest_priority = client_priority;
		    nready = 1;
		}
		/*  the following if makes sure that multiple same-priority 
		 *  clients get batched together
		 */
		else if (client_priority == highest_priority)
#endif
		{
		    pClientsReady[nready++] = client_index;
		}
#ifndef WIN32
		clientsReadable.fds_bits[i] &= ~(((fd_mask)1) << curclient);
	    }
#else
	    FD_CLR(curclient, &clientsReadable);
#endif
	}
    }
    return nready;
}

#if 0
/*
 * This is not always a macro.
 */
ANYSET(src)
    FdMask	*src;
{
    int i;

    for (i=0; i<mskcnt; i++)
	if (src[ i ])
	    return (TRUE);
    return (FALSE);
}
#endif

#else /* AMOEBA */

#define dbprintf(list)  /* printf list */

int
WaitForSomething(pClientsReady)
    int		*pClientsReady;
{
    register int	i, wt, nt;
    struct timeval	*wtp;
    long        	alwaysCheckForInput[2];
    int 		nready;
    int 		timeout;
    unsigned long	now;

    WakeupInitWaiters();

    /* Be sure to check for input on every sweep in the dispatcher.
     * This routine should be in InitInput, but since this is more
     * or less a device dependent routine, and the semantics of it
     * are device independent I decided to put it here.
     */
    alwaysCheckForInput[0] = 0;
    alwaysCheckForInput[1] = 1;
    SetInputCheck(&alwaysCheckForInput[0], &alwaysCheckForInput[1]);

    while (1) {
	/* deal with any blocked jobs */
	if (workQueue)
	    ProcessWorkQueue();

	if (ANYSET(ClientsWithInput)) {
	    FdSet clientsReadable;
	    int highest_priority;

	    COPYBITS(ClientsWithInput, clientsReadable);
	    dbprintf(("WaitFor: "));
	    nready = 0;
	    for (i=0; i < mskcnt; i++) {
		while (clientsReadable[i]) {
		    int client_priority, curclient, client_index;

		    curclient = ffsl(clientsReadable[i]) - 1;
		    client_index = ConnectionTranslation[curclient + (i * sizeof(fd_mask) * 8)];
		    dbprintf(("%d has input\n", curclient));
#ifdef XSYNC
		    client_priority = clients[client_index]->priority;
		    if (nready == 0 || client_priority > highest_priority)
		    {
		        pClientsReady[0] = client_index;
		        highest_priority = client_priority;
		        nready = 1;
		    }
		    else if (client_priority == highest_priority)
#endif
		    {
		        pClientsReady[nready++] = client_index;
		    }
		    clientsReadable[i] &= ~(((FdMask)1) << curclient);
		}
	    }
	    break;
	}	

	wt = -1;
	now = GetTimeInMillis();
	if (timers)
	{
	    while (timers && timers->expires <= now)
		DoTimer(timers, now, &timers);
	    if (timers)
	    {
		timeout = timers->expires - now;
		wt = timeout;
	    }
	}
	if (ScreenSaverTime) {
	    timeout = ScreenSaverTime - TimeSinceLastInputEvent();
	    if (timeout <= 0) { /* may be forced by AutoResetServer() */
		long timeSinceSave;

		timeSinceSave = -timeout;
		if ((timeSinceSave >= timeTilFrob) && (timeTilFrob >= 0)) {
		    SaveScreens(SCREEN_SAVER_ON, ScreenSaverActive);
		    if (ScreenSaverInterval)
			/* round up to the next ScreenSaverInterval */
			timeTilFrob = ScreenSaverInterval *
				((timeSinceSave + ScreenSaverInterval) /
					ScreenSaverInterval);
		    else
			timeTilFrob = -1;
		}
		timeout = timeTilFrob - timeSinceSave;
	    } else {
		if (timeout > ScreenSaverTime)
		    timeout = ScreenSaverTime;
		timeTilFrob = 0;
	    }
	    
	    if (wt < 0 || (timeTilFrob >= 0 && wt > timeout)) {
		wt = timeout;
	    }
	}

	/* Check for new clients. We do this here and not in the listener
	 * threads because we cannot be sure that dix is re-entrant, and
	 * we need to call some dix routines during startup.
	 */
	if (nNewConns) {
	    QueueWorkProc(EstablishNewConnections, NULL,
			  (pointer) 0);
	}

	/* Call device dependent block handlers, which may want to
	 * specify a different timeout (e.g. used for key auto-repeat).
	 */
	wtp = (struct timeval *) NULL;
	BlockHandler((pointer)&wtp, (pointer)NULL);
	if (wtp) wt = (wtp->tv_sec * 1000) + (wtp->tv_usec / 1000);

	if (NewOutputPending)
	    FlushAllOutput();

	/* TODO: XTESTEXT1 */

	nready = AmFindReadyClients(pClientsReady, AllSockets);

	/* If we found some work, or the iop server has us informed about
	 * new device events, we return.
	 */
	if (nready || AmoebaEventsAvailable())
	    break;

	if (dispatchException)
	    return 0;

	/* Nothing interesting is available. Go to sleep with a timeout.
	 * The other threads will wake us when needed.
	 */
	i = SleepMainThread(wt);

	/* Wake up any of the sleeping handlers */
	WakeupHandler((unsigned long)0, (pointer)NULL);

	/* TODO: XTESTEXT1 */

	if (dispatchException)
	    return 0;

	if (i == -1) {
	    /* An error or timeout occurred */
	    return 0;
	}
    }

    dbprintf(("WaitForSomething: %d clients ready\n", nready));
    return nready;
}

#endif /* AMOEBA */


static void
DoTimer(timer, now, prev)
    register OsTimerPtr timer;
    CARD32 now;
    OsTimerPtr *prev;
{
    CARD32 newTime;

    *prev = timer->next;
    timer->next = NULL;
    newTime = (*timer->callback)(timer, now, timer->arg);
    if (newTime)
	TimerSet(timer, 0, newTime, timer->callback, timer->arg);
}

OsTimerPtr
TimerSet(timer, flags, millis, func, arg)
    register OsTimerPtr timer;
    int flags;
    CARD32 millis;
    OsTimerCallback func;
    pointer arg;
{
    register OsTimerPtr *prev;
    CARD32 now = GetTimeInMillis();

    if (!timer)
    {
	timer = (OsTimerPtr)xalloc(sizeof(struct _OsTimerRec));
	if (!timer)
	    return NULL;
    }
    else
    {
	for (prev = &timers; *prev; prev = &(*prev)->next)
	{
	    if (*prev == timer)
	    {
		*prev = timer->next;
		if (flags & TimerForceOld)
		    (void)(*timer->callback)(timer, now, timer->arg);
		break;
	    }
	}
    }
    if (!millis)
	return timer;
    if (!(flags & TimerAbsolute))
	millis += now;
    timer->expires = millis;
    timer->callback = func;
    timer->arg = arg;
    if (millis <= now)
    {
	timer->next = NULL;
	millis = (*timer->callback)(timer, now, timer->arg);
	if (!millis)
	    return timer;
    }
    for (prev = &timers;
	 *prev && millis > (*prev)->expires;
	 prev = &(*prev)->next)
	;
    timer->next = *prev;
    *prev = timer;
    return timer;
}

Bool
TimerForce(timer)
    register OsTimerPtr timer;
{
    register OsTimerPtr *prev;
    register CARD32 newTime;

    for (prev = &timers; *prev; prev = &(*prev)->next)
    {
	if (*prev == timer)
	{
	    DoTimer(timer, GetTimeInMillis(), prev);
	    return TRUE;
	}
    }
    return FALSE;
}


void
TimerCancel(timer)
    register OsTimerPtr timer;
{
    register OsTimerPtr *prev;

    if (!timer)
	return;
    for (prev = &timers; *prev; prev = &(*prev)->next)
    {
	if (*prev == timer)
	{
	    *prev = timer->next;
	    break;
	}
    }
}

void
TimerFree(timer)
    register OsTimerPtr timer;
{
    if (!timer)
	return;
    TimerCancel(timer);
    xfree(timer);
}

void
TimerCheck()
{
    register CARD32 now = GetTimeInMillis();

    while (timers && timers->expires <= now)
	DoTimer(timers, now, &timers);
}

void
TimerInit()
{
    OsTimerPtr timer;

    while (timer = timers)
    {
	timers = timer->next;
	xfree(timer);
    }
}
