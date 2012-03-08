/* $XConsortium: iopreader.c,v 1.2 94/04/12 17:24:33 dpw Exp $ */

/* Copyright (c) 1987 by the Regents of the University of California
 * Copyright (c) 1994 by the Vrije Universiteit, Amsterdam.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * and the Vrije Universiteit make no representations about
 * the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 */

#ifdef AMOEBA
/*
 * iopreader.c
 *
 */
#define port am_port_t
#include <amoeba.h>
#include <ampolicy.h>
#include <cmdreg.h>
#include <stdcom.h>
#include <stderr.h>
#include <server/iop/iop.h>
#undef port

#include "osdep.h"

#define DEVREADER_STACK	8000
#define	MAXEVENTQUEUE	32

capability iopcap;

static mutex lock;
static semaphore empty, filled;

static IOPEvent event_queue[MAXEVENTQUEUE];
static int event_qin, event_qout;

void IOPCleanUp();
static void IOPServerReader();

/*
 * Initialize the IOP server
 */
void
InitializeIOPServerReader()
{
    capability		hostcap;
    errstat		err;

    /*
     * Initialize event queue
     */
    event_qin = event_qout = 0;
    sema_init(&empty, MAXEVENTQUEUE);
    sema_init(&filled, 0);
    mu_init(&lock);

    /* 
     * Get IOP capability, and enable the server
     */
    if (XServerHostName == NULL)
	FatalError("No hostname, no screen\n");
    
    if ((err = host_lookup(XServerHostName, &hostcap)) != STD_OK ||
        (err = dir_lookup(&hostcap, DEF_IOPSVRNAME, &iopcap)) != STD_OK)
    {
	FatalError("Cannot find IOP server for %s: %s\n",
		    XServerHostName, err_why(err));
    }

    /*
     * Enable IOP server
     */
    if ((err = iop_enable(&iopcap)) != STD_OK)
	FatalError("iop_enable failed (%s)\n", err_why(err));

    /*
     * Start IOP reader thread
     */
    atexit(IOPCleanUp);
    if (thread_newthread(IOPServerReader, DEVREADER_STACK, 0, 0) <= 0)
	FatalError("Cannot start IOP reader thread\n");
}

/*
 * IOP clean up, actuall disable the IOP server. Its the IOP's own choice
 * what do do (perhaps restore the screen?).
 */
void
IOPCleanUp()
{
    errstat err;

    if ((err = iop_disable(&iopcap)) != STD_OK)
	ErrorF("iop_disable failed (%s)\n", err_why(err));
}

/*
 * This threads polls the IOP server for events. Once an event (or a
 * number of events) are read, they are queued up using a traditional
 * producer/consumer approach.
 */
static void
IOPServerReader()
{
    IOPEvent		queue[MAXEVENTQUEUE-1];
    int			nevents, i;
    errstat		err;

    WaitForInitialization();

#ifdef XDEBUG
    if (amDebug) ErrorF("IOPServerReader() running ...\n");
#endif

    for (;;) {
	do {
	    nevents = MAXEVENTQUEUE - 1;
	    err = iop_getevents(&iopcap, queue, &nevents);
	    if (err != STD_OK) {
	        if (err != RPC_FAILURE) {
		    ErrorF("iop_getevents failed (%s)\n", err_why(err));
		}
		nevents = 0;
	    }
	} while (nevents <= 0);

	/* store event(s) in the global event queue */
	sema_mdown(&empty, nevents);
	mu_lock(&lock);
	for (i = 0; i < nevents; i++) {
	    event_queue[event_qin] = queue[i];
	    event_qin = (event_qin + 1) % MAXEVENTQUEUE;
	}
	mu_unlock(&lock);
	sema_mup(&filled, nevents);
	WakeUpMainThread();
    }
}

/*
 * Return the number of IOP events waiting
 */
int
AmoebaEventsAvailable()
{
    return sema_level(&filled);
}

/*
 * Get the IOP events from the queue. ``size'' is the maximum the
 * requestor cares to handle, the actual size read is returned as
 * result.
 */
int
AmoebaGetEvents(queue, size)
    IOPEvent	*queue;
    int		size;
{
    int		nevents, i;

    if (sema_level(&filled) <= 0) return 0;
    if ((nevents = sema_level(&filled)) > size)
	nevents = size;
    sema_mdown(&filled, nevents);
    mu_lock(&lock);
    for (i = 0; i < nevents; i++) {
	queue[i] = event_queue[event_qout];
	event_qout = (event_qout + 1) % MAXEVENTQUEUE;
    }
    mu_unlock(&lock);
    sema_mup(&empty, nevents);
    return nevents;
}
#endif /* AMOEBA */
