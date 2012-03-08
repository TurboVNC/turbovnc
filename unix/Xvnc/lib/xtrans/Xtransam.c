/* $XConsortium: Xtransam.c,v 1.4 94/04/17 20:23:01 mor Exp $ */
/* $XFree86: xc/lib/xtrans/Xtransam.c,v 3.1 1996/05/10 06:55:45 dawes Exp $ */
/*

Copyright (c) 1994  X Consortium

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

*/

/* Copyright (c) 1994 Vrije Universiteit Amsterdam, Netherlands
 *
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name Vrije Universiteit not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The Vrije Universiteit
 * makes no representations about the suitability of this software for
 * any purpose.  It is provided "as is" without express or implied warranty.
 *
 * THE VRIJE UNIVERSITEIT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL THE VRIJE UNIVERSITEIT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This is the Amoeba implementation of the X Transport service layer
 */

#define event am_event_t
#define interval am_interval_t
#define port am_port_t
#include <amoeba.h>
#include <semaphore.h>
#include <cmdreg.h>
#include <stdcom.h>
#include <stderr.h>
#include <vc.h>
#include <circbuf.h>
#include <exception.h>
#include <module/signals.h>
#include <ampolicy.h>
#include <stdlib.h>
#include <stdio.h>
#include <exception.h>
#include <fault.h>
#include <signal.h>
#include <ctype.h>
#include <module/name.h>
#include <server/x11/Xamoeba.h>
#include <server/ip/hton.h>
#include <server/ip/types.h>
#include <server/ip/gen/in.h>
#include <server/ip/gen/tcp.h>
#include <server/ip/tcpip.h>
#include <server/ip/tcp_io.h>
#include <server/ip/gen/tcp_io.h>
#include <server/ip/gen/netdb.h>
#include <server/ip/gen/inet.h>
#undef event
#undef interval
#undef port

extern char *strdup();

/* a new family for Amoeba RPC connections */
#define AF_AMOEBA	33
#define FamilyAmoeba    33

#define	MAX_TCPIP_RETRY	4
#define	CIRCBUFSIZE	4096 /* was 1024 */

/*
 * Amoeba channel description:
 */
typedef struct _XAmChanDesc {
    int			state;		/* current state of connection */
    int			type;		/* type of connection */
    int			status;		/* status used by server */
    signum		signal;		/* signal to kill TCP/IP reader */
    semaphore		*sema;		/* select semaphore */
    struct vc		*virtcirc;	/* virtual circuit for Amoeba RPC */
    struct circbuf	*circbuf;	/* circular buffer for TCP/IP */
    capability		chancap;	/* TCP/IP channel capability */
    XtransConnInfo	conninfo;	/* back pointer to the connect info */
} XAmChanDesc;

/* Amoeba channel descriptor states */
#define	ACDS_FREE	0		/* unused */
#define	ACDS_USED	1		/* intermediate state */
#define	ACDS_CLOSED	2		/* just closed */

/* Amoeba channel types */
#define	ACDT_TCPIP	1		/* TCP/IP connection */
#define	ACDT_VIRTCIRC	2		/* Amoeba virtual circuit connection */


#ifdef XSERV_t
#include "dix.h" /* clients[] needed by AmFindReadyClients */
#define Error(list) ErrorF list
#define Fatal(list) FatalError list
#else
#define Error(list) printf list
#define Fatal(list) { printf list; exit(1); }
#endif

#define dbprintf(list) /* printf list */
#define doprintf(list) printf list /**/

/*
 * First: utility functions.
 */

#if defined(XSERV_t) || defined(FS_t)

static semaphore main_sema;

/* The X-server consists of one main thread, running the non re-entrant
 * X code, and a number of auxilary threads that take care of reading
 * the input streams, and input devices. The following set of routines
 * wake up the main thread when it has something to do.
 */
void
InitMainThread()
{
    sema_init(&main_sema, 0);
}

void
WakeUpMainThread()
{
    sema_up(&main_sema);
}

int
SleepMainThread(timeout)
am_interval_t timeout;
{
    dbprintf(("Sleeping main thread timeout %d\n", timeout));
    return (sema_trydown(&main_sema, timeout) == 0) ? 0 : -1;
}


static int init_waiters;
static semaphore init_sema;

void
AmInitWaitFor()
{
    init_waiters = 0;
    sema_init(&init_sema, 0);
}

/*
 * Force caller thread to wait until main has finished the initialization.
 */
void
WaitForInitialization()
{
    init_waiters++;
    dbprintf(("Waiting for initialization (%d)\n", init_waiters));
    sema_down(&init_sema);
}

void
WakeupInitWaiters()
{
    /* wakeup threads in initial sleep */
    if (init_waiters > 0) {
	dbprintf(("%d waiters wait for something ...\n", init_waiters));
	while (init_waiters-- > 0) {
	    sema_up(&init_sema);
	}
    }
}

#endif /* XSERV_t || FS_t */


#define	THREAD_STACK_SIZE (8*1024)

/*
 * Amoeba connection information is stored in, so called,
 * channel descriptors. Channel descriptors are identified
 * by their index in the table below.
 */
static XAmChanDesc XAmChanDescriptors[OPEN_MAX];
static void XAmCleanUpChanDesc(); /* forward */

/*
 * Cleanup connection descriptors on a signal
 */
static void
XAmSignalCleanUpChanDesc(sig)
    int sig;
{
    XAmCleanUpChanDesc();
    _exit(sig | 0x80);
}

/*
 * Cleanup connection descriptors
 */
static void
XAmCleanUpChanDesc()
{
    register int i;

    for (i = 0; i < OPEN_MAX; i++) {
	switch (XAmChanDescriptors[i].type) {
	case ACDT_TCPIP:
	    /* The Amoeba TCP/IP server is capability based, i.e.
	     * it uses capabilities to identify connections.  Since a
	     * capability is only destroyed when it has aged too much
	     * or is explicitly deleted, the connection it identifies
	     * will tend to exist for some while even if the client is
	     * already gone. To force connections to close this loop
	     * destroys all open TCP/IP connection. This loop us auto-
	     * matically executed when exit() is called.
	     */
	    std_destroy(&XAmChanDescriptors[i].chancap);
	    break;
	case ACDT_VIRTCIRC:
	    /* Close the virtual circuit asynchronously, or otherwise
	     * we may hang for a minute under some (?) conditions.
	     */
	    vc_close(XAmChanDescriptors[i].virtcirc, VC_BOTH | VC_ASYNC);
	    break;
	}
	XAmChanDescriptors[i].state = ACDS_FREE;
    }
}

/*
 * Allocate a channel descriptor
 */
static XAmChanDesc *
XAmAllocChanDesc()
{
    register int i;

#ifndef XSERV_t

    static int initialized = 0;

    /*
     * Since the TCP/IP server is capability based its connections exists
     * even if the owner process is long gone. To overcome this nuisance,
     * a sweep is made over the connection descriptors when exit() is
     * called or when an un-catched (by application program) signal is 
     * received.
     */
    if (!initialized) {
	initialized = 1;
	atexit(XAmCleanUpChanDesc);
	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
	    signal(SIGHUP, XAmSignalCleanUpChanDesc);
	if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
	    signal(SIGQUIT, XAmSignalCleanUpChanDesc);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
	    signal(SIGINT, XAmSignalCleanUpChanDesc);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
	    signal(SIGTERM, XAmSignalCleanUpChanDesc);
    }
#endif

    for (i = 0; i < OPEN_MAX; i++) {
	if (XAmChanDescriptors[i].state == ACDS_FREE) {
	    XAmChanDescriptors[i].state = ACDS_USED;
	    XAmChanDescriptors[i].conninfo = NULL;
	    return &XAmChanDescriptors[i];
	}
    }
    return NULL;
}

/*
 * Convert ``file descriptor'' to channel descriptor
 */
static XAmChanDesc *
XAmFdToChanDesc(fd)
    int fd;
{
    if (fd >= 0 && fd < OPEN_MAX) {
	return &XAmChanDescriptors[fd];
    } else {
	return NULL;
    }
}

/*
 * Convert channel descriptor to ``file descriptor''
 */
static int
XAmChanDescToFd(chandesc)
    XAmChanDesc *chandesc;
{
    return chandesc - XAmChanDescriptors;
}

/*
 * Free channel descriptor
 */
static void
XAmFreeChanDesc(chandesc)
    XAmChanDesc *chandesc;
{
    if (chandesc->sema) {
	xfree(chandesc->sema);
	chandesc->sema = NULL;
    }
    chandesc->state = ACDS_FREE;
}

static void XAmReaderThread();

static int
MakeAmConnection(phostname, idisplay, familyp, saddrlenp, saddrp)
    char *phostname;
    int idisplay;
    int *familyp;			/* RETURN */
    int *saddrlenp;			/* RETURN */
    char **saddrp;			/* RETURN */
{
    capability xservercap;
    char xserverpath[256];
    XAmChanDesc *chandesc;
    errstat err;

    /* Amoeba requires a server hostname */
    if (phostname == NULL || *phostname == '\0') {
	fprintf(stderr, "MakeAmConnection: Display name expected\n");
	return -1;
    }

    /* allocate channel descriptor */
    chandesc = XAmAllocChanDesc();
    if (chandesc == NULL) {
	fprintf(stderr, "MakeAmConnection: Out of channel capabilities\n");
	return -1;
    }

    /*
     * There are two possible way to make a connection on Amoeba. Either
     * through an Amoeba RPC or a TCP/IP connection. Depending on whether
     * the X server resides on Amoeba, Amoeba RPC's are used. Otherwise
     * it uses a TCP/IP connection.
     */
    (void)sprintf(xserverpath, "%s/%s:%d", DEF_XSVRDIR, phostname, idisplay);
    if ((err = name_lookup(xserverpath, &xservercap)) == STD_OK) {
	am_port_t vccaps[2];
	bufsize size;
	errstat err;
	header hdr;

	/* Amoeba virtual circuit connection */
	chandesc->type = ACDT_VIRTCIRC;

	/* get the two connection ports from the X-server */
	hdr.h_command = AX_CONNECT;
	hdr.h_port = xservercap.cap_port;
	hdr.h_priv = xservercap.cap_priv;
	size = trans(&hdr, NILBUF, 0, &hdr, (char *)vccaps, sizeof(vccaps));
	if (ERR_STATUS(size)) {
	    err = ERR_CONVERT(size);
	} else {
	    err = ERR_CONVERT(hdr.h_status);
	}
	if (err != STD_OK || size != sizeof(vccaps)) {
	    fprintf(stderr, "Xlib: connect to Amoeba X-server failed (%s)\n",
		    err_why(err));
	    XAmFreeChanDesc(chandesc);
	    return -1;
	}

	/* setup an Amoeba virtual circuit */
	chandesc->virtcirc =
	    vc_create(&vccaps[1], &vccaps[0], MAXBUFSIZE, MAXBUFSIZE);
	if (chandesc->virtcirc == (struct vc *)NULL) {
	    fprintf(stderr, "Xlib: Amoeba virtual circuit create failed\n");
	    XAmFreeChanDesc(chandesc);
	    return -1;
	}

	/* Special Amoeba family type. For Amoeba no additional access control
	 * mechanism exists;  when you have the server capability, you have
	 * the access.  Just use a fake address.
	 */
	*familyp = AF_AMOEBA;
	*saddrp = strdup("Amoeba");
	*saddrlenp = strlen(*saddrp);
    } else {
	char tcpname[256];
	capability tcpcap;
	ipaddr_t ipaddr;
	char *tcpsvr;
	nwio_tcpcl_t tcpcl;
	nwio_tcpconf_t tcpconf;
	XAmChanDesc **param;
	int result;

	/* Amoeba TCP/IP connection */
	chandesc->type = ACDT_TCPIP;

	/* lookup up TCP/IP server */
	if ((tcpsvr = getenv("TCP_SERVER")) == NULL) {
	    tcpsvr = TCP_SVR_NAME;
	}
	if ((err = name_lookup(tcpsvr, &tcpcap)) != STD_OK) {
	    fprintf(stderr, "Xlib: Cannot lookup %s (%s)\n",
		    tcpsvr, err_why(err));
	    std_destroy(&chandesc->chancap);
	    XAmFreeChanDesc(chandesc);
	    return -1;
	}

	/* establish TCP/IP connection */
	if ((err = tcpip_open(&tcpcap, &chandesc->chancap)) != STD_OK) {
	    fprintf(stderr, "Xlib: Cannot open TCP/IP server on %s (%s)\n",
		    tcpsvr, tcpip_why(err));
	    std_destroy(&chandesc->chancap);
	    XAmFreeChanDesc(chandesc);
	    return -1;
	}

	/* lookup TCP/IP hostname */
	if (isdigit(phostname[0])) {
	    ipaddr = inet_addr(phostname);
	} else {
	    struct hostent *hp = gethostbyname(phostname);
	    if (hp == NULL) {
		fprintf(stderr, "Xlib: %s unknown host\n", phostname);
		return -1;
	    }
	    memcpy(&ipaddr, hp->h_addr, hp->h_length);
	}

	/* set remote address/port on the TCP/IP connection */
	tcpconf.nwtc_flags = NWTC_SET_RA|NWTC_SET_RP|NWTC_LP_SEL;
	tcpconf.nwtc_remaddr = ipaddr;
	tcpconf.nwtc_remport = htons(6000+idisplay);
	if ((err = tcp_ioc_setconf(&chandesc->chancap, &tcpconf)) != STD_OK) {
	    fprintf(stderr, "Xlib: Cannot configure TCP/IP server (%s)\n",
		    tcpip_why(err));
	    std_destroy(&chandesc->chancap);
	    XAmFreeChanDesc(chandesc);
	    return -1;
	}

	/* make the actual TCP/IP connection */
	tcpcl.nwtcl_flags = 0;
	if ((err = tcp_ioc_connect(&chandesc->chancap, &tcpcl)) != STD_OK) {
	    fprintf(stderr, "Xlib: Cannot make TCP/IP connection (%s)\n",
		    tcpip_why(err));
	    std_destroy(&chandesc->chancap);
	    XAmFreeChanDesc(chandesc);
	    return -1;
	}

	/* start TCP/IP reader thread */
	chandesc->signal = sig_uniq();
	chandesc->circbuf = cb_alloc(CIRCBUFSIZE);
	param = (XAmChanDesc **) xalloc(sizeof(XAmChanDesc *)); /* error checking? */
	*param = chandesc;
	result = thread_newthread(XAmReaderThread, THREAD_STACK_SIZE,
				  (char *)param, sizeof(XAmChanDesc *));
	if (result == 0) {
	    fprintf(stderr, "Xlib: Cannot start reader thread\n");
	    std_destroy(&chandesc->chancap);
	    XAmFreeChanDesc(chandesc);
	    return -1;
	}
	threadswitch(); /* give reader a try */

	/*
	 * Family type is set to Internet so that the .Xauthority
	 * files from Unix will work under Amoeba (for Unix displays).
	 */
	*familyp = AF_INET;
	*saddrlenp = sizeof(ipaddr_t);
	*saddrp = xalloc(sizeof(ipaddr_t));
	memcpy(*saddrp, (char *)&ipaddr, sizeof(ipaddr_t)); /* error checking? */
    }

    return XAmChanDescToFd(chandesc);
}

/*
 * The TCP/IP server silently assumes a maximum buffer size of 30000 bytes.
 */
#define	TCPIP_BUFSIZE	16384

static void
XAMCloseChannel(chandesc)
XAmChanDesc *chandesc;
{
    if (chandesc->state == ACDS_USED && chandesc->type == ACDT_TCPIP) {
	cb_close(chandesc->circbuf);
	chandesc->state = ACDS_CLOSED;
    }
}


/*
 * Shutdown TCP/IP reader thread
 */
static void
XAmReaderSignalCatcher(sig, us, extra)
    signum sig;
    thread_ustate *us;
    _VOIDSTAR extra;
{
    register XAmChanDesc *chandesc = (XAmChanDesc *)extra;

    XAMCloseChannel(chandesc);
    thread_exit();
}

/*
 * TCP/IP reader thread
 */
static void
XAmReaderThread(argptr, argsize)
    void *argptr;
    int argsize;
{
    register XAmChanDesc *chandesc;

    chandesc = *((XAmChanDesc **)argptr);
    (void) sig_catch(chandesc->signal, XAmReaderSignalCatcher,
		     (_VOIDSTAR) chandesc);

    while (chandesc->state == ACDS_USED) {
	char buffer[CIRCBUFSIZE];
	bufsize size;

	size = tcpip_read(&chandesc->chancap, buffer, sizeof(buffer));
	if (ERR_STATUS(size) || size == 0) {
	    if (size == 0) {
		static char msg[] = "Xlib: TCP/IP channel closed\n";

		write(2, msg, sizeof(msg));
	    } else {
		fprintf(stderr, "Xlib: TCP/IP read failed (%s)\n",
			err_why(ERR_CONVERT(size)));
	    }
	    XAMCloseChannel(chandesc);
	    break;
	}

	if (cb_puts(chandesc->circbuf, buffer, size) != 0) {
	    fprintf(stderr, "Xlib: short write to circular buffer\n");
	    XAMCloseChannel(chandesc);
	}
    }

    thread_exit();
}

/*
 * Wait until input is available or until the timer expires.
 */
int
TRANS(AmSelect)(ifd, timout)
    int ifd;
    int timout;
{
    XAmChanDesc *chandesc;
    int n;

    errno = 0;

    chandesc = XAmFdToChanDesc(ifd);
    if (chandesc == NULL || chandesc->state != ACDS_USED) {
	errno = EBADF;
	return -1;
    }

    if (chandesc->sema == NULL) {
	/* Allocate semaphore to sleep on when no data is
	 * available. The underlying circular buffer and
	 * virtual circuit packages manage this semaphore.
	 */
	chandesc->sema = (semaphore *) xalloc(sizeof(semaphore));
	if (chandesc->sema == NULL) {
	    errno = ENOMEM;
	    return -1;
	}

	sema_init(chandesc->sema, 0);
	switch (chandesc->type) {
	case ACDT_TCPIP:
	    cb_setsema(chandesc->circbuf, chandesc->sema);
	    break;
	case ACDT_VIRTCIRC:
	    vc_setsema(chandesc->virtcirc, chandesc->sema);
	    break;
	}
    }

    switch (chandesc->type) {
    case ACDT_TCPIP:
	if ((n = cb_full(chandesc->circbuf)) != 0) {
	    if (n < 0) errno = EPIPE;
	    return n; /* includes error as well */
	}
	if (sema_trydown(chandesc->sema, timout) < 0) {
	    errno = EINTR;
	    return -1;
	} else {
	    /* we down for all the bytes in AMRead, so undo the down */
	    sema_up(chandesc->sema);
	}
	if ((n = cb_full(chandesc->circbuf)) < 0) {
	    errno = EPIPE;
	    return -1;
	}
	return n;

    case ACDT_VIRTCIRC:
	if ((n = vc_avail(chandesc->virtcirc, VC_IN)) != 0) {
	    if (n < 0) errno = EPIPE;
	    return n; /* includes error as well */
	}
	if (sema_trydown(chandesc->sema, timout) < 0) {
	    errno = EINTR;
	    return -1;
	} else {
	    /* we down for all the bytes in AMRead, so undo the down */
	    sema_up(chandesc->sema);
	}
	if ((n = vc_avail(chandesc->virtcirc, VC_IN)) < 0) {
	    errno = EPIPE;
	    return -1;
	}
	return n;
    }

    errno = EINVAL;
    return -1;
}

/*
 * This function gets the local address of the transport and stores it in the
 * XtransConnInfo structure for the connection.
 */

static int
TRANS(AMGetAddr)(ciptr)
XtransConnInfo	ciptr;
{
    PRMSG(1,"AMGetAddr(%x)\n", ciptr, 0,0 );
    PRMSG(1,"AMGetAddr: TODO\n", 0, 0, 0);

    return -1;
}


/*
 * This function gets the remote address of the socket and stores it in the
 * XtransConnInfo structure for the connection.
 */

static int
TRANS(AMGetPeerAddr)(ciptr)
XtransConnInfo	ciptr;
{
    struct nwio_tcpconf tcpconf;
    errstat err;
    XAmChanDesc *chandesc;

    PRMSG(2,"AMGetPeerAddr(%x)\n", ciptr, 0,0 );

    chandesc = XAmFdToChanDesc(ciptr->fd);
    if (chandesc == NULL || chandesc->state != ACDS_USED) {
	errno = EBADF;
	return -1;
    }

    switch (chandesc->type) {
    case ACDT_TCPIP:
	/* get the remote adress from the TCP/IP server */
	if ((err = tcp_ioc_getconf(&chandesc->chancap, &tcpconf)) != STD_OK) {
	    PRMSG (1, "AMGetPeerAddr: Cannot get remote address (%d)\n",
		   (int) err, 0, 0);
	    return -1;
	}

#if 0 /* debug */
        {
	    struct hostent *remote;
	    char *hostname;

	    remote = gethostbyaddr((char *) &tcpconf.nwtc_remaddr,
				   sizeof(tcpconf.nwtc_remaddr), AF_INET);
	    if ((remote == NULL) || (remote->h_name == NULL)) {
		hostname = inet_ntoa(tcpconf.nwtc_remaddr);
	    } else {
		hostname = remote->h_name;
	    }
	    PRMSG (1, "AMGetPeerAddr: remote addr `%s'\n",
		   hostname, 0, 0);
	}
#endif

	ciptr->peeraddrlen = sizeof(tcpconf.nwtc_remaddr);
	ciptr->peeraddr = (char *) xalloc (ciptr->peeraddrlen);
	if (ciptr->peeraddr == NULL) {
	    PRMSG (1, "AMGetPeerAddr: Can't allocate peeraddr\n",
		   0, 0, 0);
	    return -1;
	}

	memcpy (ciptr->peeraddr, &tcpconf.nwtc_remaddr, ciptr->peeraddrlen);
	break;

    case ACDT_VIRTCIRC:
	/* for Amoeba virtual circuits just copy the client address */
	if ((ciptr->peeraddr = (char *) xalloc (ciptr->addrlen)) == NULL) {
	    PRMSG (1, "AMGetPeerAddr: Can't allocate peeraddr\n",
		   0, 0, 0);
	    return -1;
	}

	ciptr->peeraddrlen = ciptr->addrlen;
	memcpy (ciptr->peeraddr, ciptr->addr, ciptr->peeraddrlen);
	break;
    }

    return 0;
}


static XtransConnInfo
TRANS(AMOpen)(device)
char	*device;
{
    PRMSG(1,"AMOpen(%s)\n", device, 0,0 );
    PRMSG(1,"AMOpen: TODO\n", 0, 0, 0);

    return NULL;
}


static	int
TRANS(AMAddrToNetbuf)(tlifamily, host, port, netbufp)
int		tlifamily;
char		*host;
char		*port;
struct netbuf	*netbufp;
{
    PRMSG(1,"AMAddrToNetbuf(%d,%s,%s)\n", tlifamily, host, port );
    PRMSG(1,"AMAddrToNetbuf: TODO\n", 0, 0, 0);

    return -1;
}

/*
 * These functions are the interface supplied in the Xtransport structure
 */

#ifdef TRANS_CLIENT

static XtransConnInfo
TRANS(AMOpenCOTSClient)(thistrans, protocol, host, port)
Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;
{
    XtransConnInfo  ciptr;
    XAmChanDesc    *chandesc;

    PRMSG(2,"AMOpenCOTSClient(%s,%s,%s)\n", protocol, host, port );
    
    ciptr = (XtransConnInfo) xcalloc (1, sizeof(struct _XtransConnInfo));
    if (ciptr == NULL) {
        PRMSG (1, "AMOpenCotsClient: malloc failed\n", 0, 0, 0);
        return NULL;
    }

    ciptr->fd = MakeAmConnection (host, 0 /* TODO */, &ciptr->family,
				  &ciptr->addrlen, &ciptr->addr);
    if (ciptr->fd < 0) {
	PRMSG(1,"AMOpenCOTSClient: Unable to make connection to %s\n",
	      host, 0,0 );
	xfree(ciptr);
	return NULL;
    }

    /* set the back pointer */
    chandesc = XAmFdToChanDesc(ciptr->fd);
    chandesc->conninfo = ciptr;

    TRANS(AMGetPeerAddr)(ciptr);

    PRMSG(2,"AMOpenCOTSClient: made connection to %s; fd = %d, family = %d\n",
	  host, ciptr->fd, ciptr->family);

    return ciptr;
}

#endif /* TRANS_CLIENT */

#if defined(XSERV_t) || defined(FS_t)

/* The following defines come from osdep.h;
 * they should removed from there eventually.
 */

/*
 * Some fundamental constants
 */
#define CONNECTOR_STACK 4000    /* stack for connector task */
#define DEVREADER_STACK 4000    /* stack for device reader */
#define CREATOR_STACK   4000    /* stack for connection creator */
#define MAXTASKS        100     /* Maximum # clients */

/*
 * OsComm status bits
 */
#define CONN_KILLED     0x1     /* Connection being closed */
#define REQ_PUSHBACK    0x2     /* Request pushed back */
#define IGNORE          0x4     /* True if client ignored */
#define CONN_INIT       0x8     /* True if still initializing */
#define CONN_ALIVE      0x10    /* True if living */


#define REPLY_BUFSIZE   30000

capability	X;			/* X capability */
char		*XServerHostName;	/* X server host name */
char		*XTcpServerName;	/* TCP/IP server name */

static XtransConnInfo NewConns[MAXTASKS]; /* new client connections */
int	              nNewConns;	  /* # of new clients */

int		maxClient;		/* Highest allocated client fd + 1*/
static int	minClient = 1;		/* Lowest allocated client fd */

static char *
OsCommStatus(status)
    int status;
{
    static char buf[100];

    buf[0] = '\0';
    if (status == 0)
	sprintf(buf, "NONE");
    if (status & CONN_INIT)
	sprintf(buf, "%s INIT", buf);
    if (status & CONN_ALIVE)
	sprintf(buf, "%s ALIVE", buf);
    if (status & CONN_KILLED)
	sprintf(buf, "%s KILLED", buf);
    if (status & REQ_PUSHBACK)
	sprintf(buf, "%s PUSHBACK", buf);
    if (status & IGNORE)
	sprintf(buf, "%s IGNORE", buf);
    return buf;
}

static char *
OsCommFamily(family)
    int family;
{
    if (family == FamilyAmoeba) {
	return "AMOEBA";
    } else {
	return "TCP/IP";
    }
}


/*
 * Return status information about the open connections
 */
static errstat
ConnectionStatus(hdr, buf, size)
    header *hdr;
    char *buf;
    int size;
{
    char	*begin, *end;
    char	*bprintf();
    int fd;
    XAmChanDesc *chandesc;

    begin = buf;
    end = buf + size;


    /* all active clients */
    begin = bprintf(begin, end, "Active clients:\n");
    for (fd = minClient; fd < maxClient; fd++) {
	static XAmChanDesc *chandesc;

	chandesc = XAmFdToChanDesc(fd);
	if (chandesc != NULL && chandesc->conninfo != NULL) {
	    begin = bprintf(begin, end, "%d: Family %s, State %d, Status %s\n",
			    fd, OsCommFamily(chandesc->conninfo->family),
			    chandesc->state, OsCommStatus(chandesc->status));
	}
    }

    if (begin == NULL) {
	hdr->h_size = 0;
	return STD_SYSERR;
    } else {
	hdr->h_size = begin - buf;
	return STD_OK;
    }
}

/*
 * Wakeup main thread if necessary
 */
static void
UnblockMain(fd)
int fd;
{
    XAmChanDesc *chandesc;

    chandesc = XAmFdToChanDesc(fd);
    if (chandesc != NULL) {
	if ((chandesc->status & IGNORE) == 0) {
	    WakeUpMainThread();
	}
    } else {
	Error(("UnblockMain: invalid fd %d\n", fd));
    }
}

static void
TcpIpReaderSignalCatcher(sig, us, extra)
    signum sig;
    thread_ustate *us;
    _VOIDSTAR extra;
{
    XAmChanDesc *chandesc = (XAmChanDesc *) extra;

    if (chandesc->conninfo != NULL) {
	dbprintf(("TcpIpReaderSignalCatcher(%d), number %d\n",
		  sig, chandesc->conninfo->fd));
	if (chandesc->signal != sig) {
	    Error(("TCP/IP Reader: Connection %s got unexpected signal %d\n",
		   chandesc->conninfo->fd, sig));
	}
    }

    chandesc->signal = -1;
    thread_exit();
}

void
TcpIpReaderThread(argptr, argsize)
    void *argptr;
    int argsize;
{
    XAmChanDesc *chandesc;

    if (argsize != sizeof(XAmChanDesc *)) {
	Fatal(("Internal error: TcpIpReaderThread incorrectly called\n"));
    }

    chandesc = * ((XAmChanDesc **) argptr);
    (void) sig_catch(chandesc->signal, TcpIpReaderSignalCatcher,
		     (_VOIDSTAR) chandesc);

    for (;;) {
	char buffer[MAXBUFSIZE];
	bufsize size;

	size = tcpip_read(&chandesc->chancap, buffer, sizeof(buffer));

	dbprintf(("TcpIpReaderThread() read %d bytes\n", size));
	if (ERR_STATUS(size)) {
	    Error(("TCP/IP read failed (%s)\n", tcpip_why(ERR_CONVERT(size))));
	    chandesc->status |= CONN_KILLED;
	    chandesc->status &= ~CONN_ALIVE;
	    chandesc->signal = -1;
	    thread_exit();
	}

	if (size == 0 || cb_puts(chandesc->circbuf, buffer, size)) {
	    if (size != 0) {
		Error(("TCP/IP short write to circular buffer for %d\n",
		       XAmChanDescToFd(chandesc)));
	    } else {
		Error(("TCP/IP read failed for client %d\n",
		       XAmChanDescToFd(chandesc)));
	    }

	    chandesc->status |= CONN_KILLED;
	    chandesc->status &= ~CONN_ALIVE;
	    chandesc->signal = -1;
	    thread_exit();
	}
	UnblockMain(XAmChanDescToFd(chandesc));
    }
}

static XAmChanDesc *
AllocClientChannel()
{
    XAmChanDesc *chandesc;
    int fd;

    chandesc = XAmAllocChanDesc();
    if (chandesc == NULL) {
	return NULL;
    }

    fd = XAmChanDescToFd(chandesc);
    if (fd >= maxClient) {
	maxClient = fd + 1;
	dbprintf(("new max Client: %d\n", fd));
    }
    if (fd < minClient) {
	minClient = fd;
    }

    return chandesc;
}

static errstat
AmRegisterRPCconn(client_ports, server_ports)
am_port_t client_ports[2];
am_port_t server_ports[2];
{
    XAmChanDesc *chandesc;

    if ((chandesc = AllocClientChannel()) == NULL) {
	return STD_NOSPACE;
    }

    chandesc->type = ACDT_VIRTCIRC;
    chandesc->virtcirc = vc_create(&server_ports[0], &server_ports[1],
				   MAXBUFSIZE, MAXBUFSIZE);
    if (chandesc->virtcirc == NULL) {
	Error(("Connection refused: No memory for virtual circuit\n"));
	XAmFreeChanDesc(chandesc);
	return STD_NOSPACE;
    }

    dbprintf(("Amoeba connection registered\n"));

    vc_warn(chandesc->virtcirc, VC_IN, UnblockMain, XAmChanDescToFd(chandesc));
    
    chandesc->status = CONN_INIT;

    /* cause WaitFor to call EstablishNewConnections: */
    nNewConns++;
    WakeUpMainThread();

    return STD_OK;
}

static XAmChanDesc *
XAmFetchConnectingClient()
{
    XAmChanDesc *chandesc;
    int fd;

    for (fd = minClient; fd < maxClient; fd++) {
	chandesc = XAmFdToChanDesc(fd);

	if (chandesc->status & CONN_INIT) {
	    Error(("Client %d is connecting\n", fd));
	    chandesc->status &= ~CONN_INIT;
	    return chandesc;
	}
    }

    return NULL;
}

static errstat
AmRegisterTCPconn(chancap)
capability *chancap;
{
    XAmChanDesc *chandesc, **param;

    if ((chandesc = AllocClientChannel()) == NULL) {
	return STD_NOSPACE;
    }
    
    chandesc->type = ACDT_TCPIP;
    chandesc->chancap = *chancap;

    if ((chandesc->circbuf = cb_alloc(MAXBUFSIZE)) == NULL) {
	Error(("TCPconn refused: No memory for circular buffer\n"));
	XAmFreeChanDesc(chandesc);
	return STD_NOSPACE;
    }

    chandesc->signal = sig_uniq();
    param = (XAmChanDesc **) xalloc(sizeof(XAmChanDesc *)); /* error checking? */
    *param = chandesc;
    if (thread_newthread(TcpIpReaderThread, MAXBUFSIZE + CONNECTOR_STACK,
			 (char *)param, sizeof(XAmChanDesc *)) == 0)
    {
	Error(("TCPconn refused: Cannot start reader thread\n"));
	cb_close(chandesc->circbuf);
	cb_free(chandesc->circbuf);
	XAmFreeChanDesc(chandesc);
	return STD_NOSPACE;
    }

    dbprintf(("TCP connection registered\n"));

    chandesc->status = CONN_INIT;

    /* cause WaitFor to call EstablishNewConnections: */
    nNewConns++;
    WakeUpMainThread();

    return STD_OK;
}


/*
 * Establishing a new connection is done in two phases. This thread does the
 * first part. It filters out bad connect requests. A new rendevous port is
 * sent to the client and the main loop is informed if there is a legal
 * request. The sleep synchronizes with the main loop so that the paperwork
 * is finished for the current connect request before the thread is ready to
 * accept another connect.
 */
static void
AmConnectorThread()
{
    header	req, rep;
    am_port_t	client_ports[2];
    am_port_t	server_ports[2];
    short	s;
    char	*repb;
    extern	CreateNewClient();

    WaitForInitialization();
    dbprintf(("AmConnectorThread() running ...\n"));
    if ((repb = (char *)xalloc(REPLY_BUFSIZE)) == NULL) {
	Fatal(("Amoeba connector thread: malloc failed"));
    }
    for (;;) {
	do {
	    req.h_port = X.cap_port;
	    s = getreq(&req, NILBUF, 0);
	} while (ERR_CONVERT(s) == RPC_ABORTED);
	if (ERR_STATUS(s))
	    Fatal(("Amoeba connector thread: getreq failed"));

	/* TODO: check privilege fields here */

	dbprintf(("AmConnectorThread() accepting a request\n"));

	switch (req.h_command) {

	case STD_INFO:
	    rep.h_status = STD_OK;
	    sprintf(repb, "X11R6 server on %s", XServerHostName);
	    rep.h_size = strlen(repb);
	    putrep(&rep, repb, rep.h_size);
	    break;

	case STD_STATUS:
	    rep.h_status = ConnectionStatus(&rep, repb, REPLY_BUFSIZE);
	    putrep(&rep, repb, rep.h_size);
	    break;

#ifdef XSERV_t
	case AX_SHUTDOWN:
	    rep.h_status = STD_OK;
	    putrep(&rep, NILBUF, 0);
	    GiveUp(SIGTERM);
	    break;

	case AX_REINIT:
	    rep.h_status = STD_OK;
	    putrep(&rep, NILBUF, 0);
	    AutoResetServer(SIGINT);
	    break;
#endif

	case AX_CONNECT:
	    uniqport(&client_ports[0]);
	    uniqport(&server_ports[1]);
	    priv2pub(&client_ports[0], &server_ports[0]);
	    priv2pub(&server_ports[1], &client_ports[1]);

	    rep.h_status = AmRegisterRPCconn(client_ports, server_ports);
	    if (rep.h_status == STD_OK) {
		putrep(&rep, (bufptr)client_ports, 2*sizeof(am_port_t));
	    } else {
		putrep(&rep, NILBUF, 0);
	    }
	    break;

	default:
	    rep.h_status = STD_COMBAD;
	    putrep(&rep, NILBUF, 0);
	    break;
	}
    }
}

#ifdef XSERV_t

/*
 * To prevent the X-server from generating lots of error messages,
 * in case the server is gone or when its full.
 */
#define	LOOP_OPEN	1
#define	LOOP_SETCONF	2
#define	LOOP_LISTEN	4

extern char *display;		/* The display number */

/*
 * The TCP/IP connector thread listens to a well known port (6000 +
 * display number) for connection request. When such a request arrives
 * it allocates a communication structure and a reader thread. This
 * thread prevents the main loop from blocking when there's no data.
 */
static void
AmTCPConnectorThread()
{
    capability		svrcap, chancap;
    nwio_tcpconf_t	tcpconf;
    nwio_tcpcl_t	tcpconnopt;
    char 		name[BUFSIZ];
    errstat 		err;
    int			result;
    int			looping = 0;

    strncpy(name, XTcpServerName, BUFSIZ);
    if ((err = name_lookup(name, &svrcap)) != STD_OK) {
	sprintf(name, "%s/%s", TCP_SVR_NAME, XTcpServerName);
	if ((err = name_lookup(name, &svrcap)) != STD_OK)
	    Fatal(("Lookup %s failed: %s\n", XTcpServerName, err_why(err)));
    }

    WaitForInitialization();
    dbprintf(("AmTCPConnectorThread() running ...\n"));

    for (;;) {
	/*
	 * Listen to TCP/IP port X_TCP_PORT + display for connections.
	 * Some interesting actions have to be taken to keep this connection
	 * alive and kicking :-)
	 */
	if ((err = tcpip_open(&svrcap, &chancap)) != STD_OK) {
	    /* the server probably disappeared, just wait for it to return */
	    if (looping & LOOP_OPEN) {
		Error(("TCP/IP open failed: %s\n", tcpip_why(err)));
		looping |= LOOP_OPEN;
	    }
	    sleep(60);
	    (void) name_lookup(name, &svrcap);
	    continue;
	}
	looping &= ~LOOP_OPEN;

	tcpconf.nwtc_locport = htons(X_TCP_PORT + atoi(display));
	tcpconf.nwtc_flags = NWTC_EXCL | NWTC_LP_SET | NWTC_UNSET_RA | 
								NWTC_UNSET_RP;
	if ((err = tcp_ioc_setconf(&chancap, &tcpconf)) != STD_OK) {
	    /* couldn't configure, probably server space problem */
	    if (looping & LOOP_SETCONF) {
		Error(("TCP/IP setconf failed: %s\n", tcpip_why(err)));
		looping |= LOOP_SETCONF;
	    }
	    std_destroy(&chancap);
	    sleep(60);
	    continue;
	}
	looping &= ~LOOP_SETCONF;

	tcpconnopt.nwtcl_flags = 0;
	if ((err = tcp_ioc_listen(&chancap, &tcpconnopt)) != STD_OK) {
	    /* couldn't listen, definitely a server memory problem */
	    if (looping & LOOP_LISTEN) {
		Error(("TCP/IP listen failed: %s\n", tcpip_why(err)));
		looping |= LOOP_LISTEN;
	    }
	    std_destroy(&chancap);
	    sleep(60);
	    continue;
	}
	looping &= ~LOOP_LISTEN;

	if ((err = tcpip_keepalive_cap(&chancap)) != STD_OK) {
	    Error(("TCP/IP keep alive failed: %s\n", tcpip_why(err)));
	    std_destroy(&chancap);
	    continue;
	}

	err = AmRegisterTCPconn(&chancap);
	if (err != STD_OK) {
	    Error(("AmRegisterTCPconn failed (%s)\n", err_why(err)));
	    std_destroy(&chancap);
	}
    }
}

static void
AmStartXserverThreads(chandesc)
XAmChanDesc *chandesc;
{
    char		host[100];
    errstat		err;
    capability		pubX;
    static int		threadsStarted = 0;

    /*
     * Each time the server is reset this routine is called to
     * setup the new well known sockets. For Amoeba we'll just
     * keep using the old threads that are already running.
     */
    if (!threadsStarted) {
	threadsStarted = 1;

	/*
	 * Create a new capability for this X server
	 */
	if (XServerHostName == NULL)
	    XServerHostName = getenv("XHOST");
	if (XServerHostName == NULL) {
	    Fatal(("XHOST not set, or server host name not given\n"));
	}
	sprintf(host, "%s/%s:%s", DEF_XSVRDIR, XServerHostName, display);

	uniqport(&X.cap_port);
	priv2pub(&X.cap_port, &pubX.cap_port);
	(void) name_delete(host);
	if ((err = name_append(host, &pubX)) != 0) {
	    Error(("Cannot create capability %s: %s\n", host, err_why(err)));
	    exit(1);
	}

	/* Allow WaitFor module to initialize */
	AmInitWaitFor();

	/* Also, initialize main thread locking */
	InitMainThread();

	/* Initialize and start IOP reader thread */
	InitializeIOPServerReader();

	/* Start native Amoeba service threads */
	if (thread_newthread(AmConnectorThread, CONNECTOR_STACK, 0, 0) <= 0) {
	    Fatal(("Cannot start Amoeba connector thread\n"));
	}
	if (thread_newthread(AmConnectorThread, CONNECTOR_STACK, 0, 0) <= 0) {
	    Fatal(("Cannot start Amoeba connector thread\n"));
	}
	chandesc->type = ACDT_VIRTCIRC;
	chandesc->status = CONN_ALIVE;

	/*
	 * Start TCP/IP service threads
	 */
	if (XTcpServerName) {
	    if (thread_newthread(AmTCPConnectorThread,
				 CONNECTOR_STACK, 0, 0) <= 0)
		Fatal(("Cannot start TCP connector thread\n"));
	    if (thread_newthread(AmTCPConnectorThread,
				 CONNECTOR_STACK, 0, 0) <= 0)
		Fatal(("Cannot start TCP connector thread\n"));
	}
    }
}

int
AmFindReadyClients(pClientsReady, mask)
int *pClientsReady;
long *mask;
{
    /* Check for clients needing attention.  They may have input,
     * or they might be dying.  Ignore the clients not present in
     * the file descriptor bit vector `mask'.  This is used for
     * implementing server grabs.
     * Returns the number of clients having data for us.
     */
    extern int ConnectionTranslation[];
    XAmChanDesc *chandesc;
    int fd;
    int nready;

    /* Since we're scheduled non-preemptively by default, allow the
     * listener threads to run first, if needed:
     */
    threadswitch();

    nready = 0;
    for (fd = minClient; fd < maxClient; fd++) {
	int which;
	int n;

	if (fd > 0 && (fd % 32) == 0) {
	    /* switch to next fd mask */
	    mask++;
	}

	if ((*mask & (1L << fd)) == 0) {
	    dbprintf(("skip %d\n", fd));
	    continue;
	}

	chandesc = XAmFdToChanDesc(fd);
	if (chandesc->state != ACDS_USED) {
	    dbprintf(("AmFindReady: fd %d not in use\n", fd));
	    continue;
	}

	which = ConnectionTranslation[fd];
	dbprintf(("checking client %d (fd %d) of %d\n",
		  fd, which, maxClient));

	if (chandesc->status & CONN_KILLED) {
	    dbprintf(("conn killed; close client with fd %d\n", fd));
	    CloseDownClient(clients[which]);
	    chandesc->status &= ~(CONN_KILLED | CONN_ALIVE);
	    continue;
	}

	if ((chandesc->status & CONN_ALIVE) == 0) {
	    dbprintf(("conn with %d is not alive\n", fd));
	    continue;
	}

	/* see if there is data available */
	switch (chandesc->type) {
	case ACDT_TCPIP:
	    n = cb_full(chandesc->circbuf);
	    break;
	case ACDT_VIRTCIRC:
	    n = vc_avail(chandesc->virtcirc, VC_IN);
	    break;
	default:
	    n = -1;
	}

	if (n < 0) {
	    dbprintf(("avail %d; close client %d\n", n, which));
	    CloseDownClient(clients[which]);
	    continue;
	} else {
	    if (n > 0) {
		*pClientsReady++ = which;
		nready++;
		dbprintf(("client %d has %d bytes available\n", which, n));
	    } else {
		dbprintf(("client %d has no data available\n", which, n));
	    }
	}

	/* Clients that already have (possibly inserted) data are found
	 * with help from io.c (the ClientsWithData bit array).
	 */
    }

    return nready;
}

#endif /* XSERV_t */

#endif /* XSERV_t || FS_t */

static
TRANS(AmSetAddr)(ciptr, chandesc)
    XtransConnInfo  ciptr;
    XAmChanDesc    *chandesc;
{
    switch (chandesc->type) {
    case ACDT_TCPIP:
	/* should really ask the TCP/IP server */
	ciptr->family = AF_INET;
	ciptr->addr = strdup("XXXXTODO");
	ciptr->addrlen = strlen("XXXXTODO");
	break;
    case ACDT_VIRTCIRC:
	/* For Amoeba connections the adress is not really used,
	 * so just fake something
	 */
	ciptr->family = AF_AMOEBA;
	ciptr->addr = strdup("Amoeba");
	ciptr->addrlen = strlen(ciptr->addr);
	break;
    }
}

#ifdef TRANS_SERVER

static XtransConnInfo
TRANS(AMOpenCOTSServer)(thistrans, protocol, given_host, port)
Xtransport	*thistrans;
char		*protocol;
char		*given_host;
char		*port;
{
    XAmChanDesc    *chandesc;
    XtransConnInfo  ciptr;

    PRMSG(2,"AMOpenCOTSServer(%s,%s,%s)\n", protocol, given_host, port);

    ciptr = (XtransConnInfo) xcalloc (1, sizeof(struct _XtransConnInfo));
    if (ciptr == NULL) {
        PRMSG (1, "AMOpenCotsClient: malloc failed\n", 0, 0, 0);
        return NULL;
    }

    chandesc = XAmAllocChanDesc();
    if (chandesc == NULL) {
	return NULL;
    }

#ifdef XSERV_t
    AmStartXserverThreads(chandesc);
#endif

    chandesc->conninfo = ciptr;
    ciptr->fd = XAmChanDescToFd(chandesc);

    TRANS(AmSetAddr)(ciptr, chandesc);
    TRANS(AMGetPeerAddr)(ciptr);

    return ciptr;
}

#endif /* TRANS_SERVER */


#ifdef TRANS_CLIENT

static XtransConnInfo
TRANS(AMOpenCLTSClient)(thistrans, protocol, host, port)
Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;
{
    XtransConnInfo	ciptr;
    int 		i;
    
    PRMSG(1,"AMOpenCLTSClient(%d,%s,%s)\n", protocol, host, port );
    /* TODO */
    return NULL;
}			

#endif /* TRANS_CLIENT */


#ifdef TRANS_SERVER

static XtransConnInfo
TRANS(AMOpenCLTSServer)(thistrans, protocol, host, port)
Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;
{
    XtransConnInfo	ciptr;
    int 		i;
    
    PRMSG(1,"AMOpenCLTSServer(%d,%s,%s)\n", protocol, host, port );
    /* TODO */
    return NULL;
}			

static int
TRANS(AMResetListener)(ciptr)
XtransConnInfo	ciptr;
{
    PRMSG(2,"AMResetListener()\n", 0, 0, 0 );

    /* nothing to do? */
    return 0;
}

#endif /* TRANS_SERVER */

static
TRANS(AMSetOption)(ciptr, option, arg)
XtransConnInfo	ciptr;
int		option;
int		arg;
{
    PRMSG(1,"AMSetOption(%d,%d,%d)\n", ciptr->fd, option, arg );
    /* TODO */
    return -1;
}


#ifdef TRANS_SERVER

static
TRANS(AMCreateListener)(ciptr, req)
XtransConnInfo	ciptr;
char	       *req;
{
    PRMSG(2,"AMCreateListener(%x->%d,%x)\n", ciptr, ciptr->fd, req );

    /* Listener threads are already created at this point */
    return 0;
}


static XtransConnInfo
TRANS(AMAccept)(ciptr)
XtransConnInfo	ciptr;
{
    XAmChanDesc    *chandesc;
    XtransConnInfo  newciptr;

    PRMSG(2,"AMAccept(%x->%d)\n", ciptr, ciptr->fd, 0 );

#if defined(XSERV_t) || defined(FS_t)
    chandesc = XAmFetchConnectingClient();
    if (chandesc == NULL) {
        PRMSG (1, "AMAccept: no client waiting?\n", 0, 0, 0);
        return NULL;
    }
    nNewConns--;

    newciptr = (XtransConnInfo) xcalloc (1, sizeof(struct _XtransConnInfo));
    if (newciptr == NULL)
    {
        PRMSG (1, "AMAccept: malloc failed\n", 0, 0, 0);
        return NULL;
    }

    newciptr->fd = XAmChanDescToFd(chandesc);
    chandesc->conninfo = newciptr;
    chandesc->status |= CONN_ALIVE;

    PRMSG(2,"AMAccept: OK: (%x->%d)\n", newciptr, newciptr->fd, 0 );

    TRANS(AmSetAddr)(newciptr, chandesc);
    TRANS(AMGetPeerAddr)(newciptr);
    
    return newciptr;
#else
    return NULL;
#endif
}

#endif /* TRANS_SERVER */


#ifdef TRANS_CLIENT

static
TRANS(AMConnect)(ciptr, host, port)
XtransConnInfo	ciptr;
char		*host;
char		*port;
{
    /* If this function is called, we are already connected */
    PRMSG(2, "AMConnect(%d,%s)\n", ciptr->fd, host, 0);
    return 0;
}

#endif /* TRANS_CLIENT */


int
TRANS(AmFdBytesReadable)(fd, count)
int fd;
BytesReadable_t	*count;
{
    register XAmChanDesc *chandesc;

    PRMSG(2, "AmFdBytesReadable(%d,%x): ", fd, count, 0 );

#ifndef XSERV_t
    /* give reader threads a chance: */
    threadswitch();
#endif

    errno = 0;
    chandesc = XAmFdToChanDesc(fd);
    if (chandesc == NULL || chandesc->state != ACDS_USED) {
	errno = EBADF;
	*count = 0;
	return -1;
    }

    switch (chandesc->type) {
    case ACDT_TCPIP:
	*count = cb_full(chandesc->circbuf);
	break;
    case ACDT_VIRTCIRC:
	*count = vc_avail(chandesc->virtcirc, VC_IN);
	break;
    }

    if (*count < 0) {
	errno = (chandesc->state == ACDS_CLOSED) ? EINTR : EPIPE;
	*count = 0;
	return -1;
    }

    PRMSG(2, "AMFdBytesReadable: %d\n", *count, 0, 0 );

    return 0;
}

static
TRANS(AMBytesReadable)(ciptr, count)
XtransConnInfo	ciptr;
BytesReadable_t	*count;
{
    return TRANS(AmFdBytesReadable)(ciptr->fd, count);
}


static
TRANS(AMRead)(ciptr, buf, count)
XtransConnInfo	ciptr;
char		*buf;
int		count;
{
    int fdi;
    register XAmChanDesc *chandesc;
    register int rv;
    BytesReadable_t avail;

    fdi = ciptr->fd;
    PRMSG(2, "AMRead(%d,%x,%d)\n", ciptr->fd, buf, count );

    errno = 0;
    chandesc = XAmFdToChanDesc(fdi);
    if (chandesc == NULL || chandesc->state != ACDS_USED) {
	errno = EBADF;
	return -1;
    }

    /* do a non-blocking read (maybe only conditionally?) */
    if ((TRANS(AMBytesReadable)(ciptr, &avail)) == 0) {
	if (avail <= 0) {
	    PRMSG(2, "AMRead: nothing available yet\n", 0, 0, 0);
	    errno = EAGAIN;
	    return 0;
	} else if (count > avail) {
	    PRMSG(2, "AMRead(%d): only %d of %d available\n",
		  ciptr->fd, avail, count);
	    count = avail; /* just read amount available */
	}
    } else {
	PRMSG(1, "AMRead: ...BytesReadable failed\n", 0, 0, 0);
	return -1;
    }

    switch (chandesc->type) {
    case ACDT_TCPIP:
	rv = cb_gets(chandesc->circbuf, buf, count, count);
	if (rv != count) {
	    if (rv == 0) {
		fprintf(stderr, "Xlib: Cannot read circbuf\n");
		errno = EPIPE;
		rv = -1;
	    } else {
		fprintf(stderr, "Xlib: Cannot read circbuf (%d)\n", rv);
	    }
	}
	break;

    case ACDT_VIRTCIRC:
	rv = vc_readall(chandesc->virtcirc, buf, count);
	if (rv < 0) {
	    fprintf(stderr, "Xlib: Cannot read virtual circuit\n");
	    errno = EPIPE;
	    rv = -1;
	}
	break;
    }

    /* The circular buffer writer will only UP the semaphore when
     * characters are available; we have to down it ourselfs.
     */
    if (chandesc->sema && rv > 0)
	sema_mdown(chandesc->sema, rv);

    PRMSG(2, "AMRead: %d bytes\n", rv, 0, 0);

    return rv;
}


static
TRANS(AMWrite)(ciptr, buf, count)
XtransConnInfo	ciptr;
char		*buf;
int		count;
{
    register XAmChanDesc *chandesc;
    register int written;
    
    PRMSG(2, "AMWrite(%d,%x,%d)\n", ciptr->fd, buf, count );

    errno = 0;
    written = 0;

    chandesc = XAmFdToChanDesc(ciptr->fd);
    if (chandesc == NULL || chandesc->state != ACDS_USED) {
	errno = EBADF;
	return -1;
    }

    switch (chandesc->type) {
    case ACDT_TCPIP:
	while (count > 0) {
	    bufsize bsize;
	    int wrcnt;

	    wrcnt = count > TCPIP_BUFSIZE ? TCPIP_BUFSIZE : count;
	    bsize = tcpip_write(&chandesc->chancap, buf, wrcnt);
	    if (ERR_STATUS(bsize)) {
		fprintf(stderr, "Xlib: TCP/IP write failed (%s)\n",
			tcpip_why(ERR_CONVERT(bsize)));
		errno = EPIPE;
		return -1;
	    }
	    if (bsize != wrcnt) {
		fprintf(stderr,
			"Xlib: TCP/IP write failed (expected %d, wrote %d)\n",
			(int)bsize, wrcnt);
		errno = EPIPE;
		return -1;
	    }
	    buf += bsize;
	    count -= (int) bsize;
	    written += (int) bsize;
	}
	break;

    case ACDT_VIRTCIRC:
	if ((written = vc_write(chandesc->virtcirc, buf, count)) < 0) {
	    fprintf(stderr, "Xlib: virtual circuit write failed\n");
	    errno = EPIPE;
	    return -1;
	}
	break;
    }

    return written;
}


static
TRANS(AMReadv)(ciptr, iov, n)
XtransConnInfo	ciptr;
struct iovec   *iov;
int		n;
{
    int i;
    int count = 0, thiscount;

    PRMSG(2, "AMReadv(%d,%x,%d)\n", ciptr->fd, ciptr, n );

    for (i = 0; i < n; i++, iov++) {
	if (iov->iov_len) {
	    thiscount = TRANS(AMRead)(ciptr, iov->iov_base, iov->iov_len);
	    if (thiscount < 0) return thiscount;
	    count += thiscount;
	    if (thiscount < iov->iov_len) break;
	}
    }

    return count;
}


static
TRANS(AMWritev)(ciptr, iov, n)
XtransConnInfo	ciptr;
struct iovec	*iov;
int		n;
{
    int i;
    int count = 0, thiscount;

    PRMSG(2, "AMWritev(%d,%x,%d)\n", ciptr->fd, iov, n );

    for (i = 0; i < n; i++, iov++) {
	if (iov->iov_len) {
	    thiscount = TRANS(AMWrite)(ciptr, iov->iov_base, iov->iov_len);
	    if (thiscount < 0)
		return thiscount;
	    count += thiscount;
	    if (thiscount < iov->iov_len) break;
	}
    }

    return count;
}


static
TRANS(AMDisconnect)(ciptr)
XtransConnInfo	ciptr;
{
    register XAmChanDesc *chandesc;

    PRMSG(2, "AMDisconnect(%x->%d)\n", ciptr, ciptr->fd, 0 );

    chandesc = XAmFdToChanDesc(ciptr->fd);
    if (chandesc != NULL) {
	switch (chandesc->type) {
	case ACDT_TCPIP:
	    if (chandesc->signal != -1) {
		sig_raise(chandesc->signal);
		chandesc->signal = -1;
	    }
	    std_destroy(&chandesc->chancap);
	    break;

	case ACDT_VIRTCIRC:
	    vc_close(chandesc->virtcirc, VC_BOTH | VC_ASYNC);
	    break;
	}
#ifdef XSERV_t
	if (ciptr->fd == maxClient - 1) {
	    maxClient--;
	    /* we could look if maxClient can be reduced even more */
	}
#endif
	XAmFreeChanDesc(chandesc);
    }

    return 0;
}


static
TRANS(AMClose)(ciptr)
XtransConnInfo	ciptr;
{
    PRMSG(2, "AMClose(%x->%d)\n", ciptr, ciptr->fd, 0 );

    return TRANS(AMDisconnect)(ciptr);
}


Xtransport	TRANS(AmConnFuncs) = {
	/* Combined AMOEBA RPC/TCP Interface; maybe we should split this  */
	"amcon",
	0,
#ifdef TRANS_CLIENT
	TRANS(AMOpenCOTSClient),
#endif /* TRANS_CLIENT */
#ifdef TRANS_SERVER
	TRANS(AMOpenCOTSServer),
#endif /* TRANS_SERVER */
#ifdef TRANS_CLIENT
	TRANS(AMOpenCLTSClient),
#endif /* TRANS_CLIENT */
#ifdef TRANS_SERVER
	TRANS(AMOpenCLTSServer),
#endif /* TRANS_SERVER */
	TRANS(AMSetOption),
#ifdef TRANS_SERVER
	TRANS(AMCreateListener),
	TRANS(AMResetListener),
	TRANS(AMAccept),
#endif /* TRANS_SERVER */
#ifdef TRANS_CLIENT
	TRANS(AMConnect),
#endif /* TRANS_CLIENT */
	TRANS(AMBytesReadable),
	TRANS(AMRead),
	TRANS(AMWrite),
	TRANS(AMReadv),
	TRANS(AMWritev),
	TRANS(AMDisconnect),
	TRANS(AMClose),
};
