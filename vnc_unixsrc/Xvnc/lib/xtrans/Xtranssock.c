/* $XConsortium: Xtranssock.c /main/58 1996/12/04 10:22:50 lehors $ */
/* $XFree86: xc/lib/xtrans/Xtranssock.c,v 3.25.2.4 1998/02/01 16:04:34 robin Exp $ */
/*

Copyright (c) 1993, 1994  X Consortium

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

/* Copyright (c) 1993, 1994 NCR Corporation - Dayton, Ohio, USA
 *
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name NCR not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  NCR makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NCR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NCR BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <ctype.h>
#ifdef XTHREADS
#include <X11/Xthreads.h>
#endif
#ifndef WIN32

#if defined(TCPCONN) || defined(UNIXCONN)
#include <netinet/in.h>
#else
#ifdef ESIX
#include <lan/in.h>
#endif
#endif
#if defined(TCPCONN) || defined(UNIXCONN)
#define X_INCLUDE_NETDB_H
#define XOS_USE_NO_LOCKING
#include <X11/Xos_r.h>
#endif

#ifdef UNIXCONN
#ifndef X_NO_SYS_UN
#ifndef Lynx
#include <sys/un.h>
#else
#include <un.h>
#endif
#endif
#include <sys/stat.h>
#endif

#if defined(hpux) || defined(__EMX__)
#define NO_TCP_H
#endif /* hpux */
#ifdef MOTOROLA
#ifdef SYSV
#define NO_TCP_H
#endif /* SYSV */
#endif /* MOTOROLA */
#ifndef NO_TCP_H
#ifdef __osf__
#include <sys/param.h>
#endif /* osf */
#if defined(__NetBSD__) || defined(__OpenBSD__) || defined(__FreeBSD__) 
#include <machine/endian.h>
#endif /* __NetBSD__ || __OpenBSD__ || __FreeBSD__ */
#include <netinet/tcp.h>
#endif /* !NO_TCP_H */
#include <sys/ioctl.h>
#if defined(SVR4) && !defined(SCO325)
#include <sys/filio.h>
#endif
#if (defined(i386) && defined(SYSV)) || defined(_SEQUENT_)
#if !defined(_SEQUENT_) && !defined(ESIX) && !defined(sco)
#include <net/errno.h>
#endif /* _SEQUENT_  || ESIX  || SCO */
#if !defined(ISC) || !defined(I_NREAD) || defined(SCO325)
#include <sys/stropts.h>
#endif
#endif /* i386 && SYSV || _SEQUENT_ */

#else /* !WIN32 */

#include <X11/Xwinsock.h>
#include <X11/Xw32defs.h>
#undef close
#define close closesocket
#define ECONNREFUSED WSAECONNREFUSED
#define EADDRINUSE WSAEADDRINUSE
#define EPROTOTYPE WSAEPROTOTYPE
#undef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#undef EINTR
#define EINTR WSAEINTR
#define X_INCLUDE_NETDB_H
#define XOS_USE_MTSAFE_NETDBAPI
#include <X11/Xos_r.h>
#endif /* WIN32 */

#if defined(SO_DONTLINGER) && defined(SO_LINGER)
#undef SO_DONTLINGER
#endif

#if defined(__EMX__)
#if defined(NOT_EMX09A)
static int IBMsockInit = 0;
#define SocketInitOnce()\
    if (!IBMsockInit) {\
	sock_init();\
	IBMsockInit = 1;\
    }
#undef EINTR
#define EINTR SOCEINTR
#undef EINVAL
#define EINVAL SOCEINVAL
#undef errno
#define errno sock_errno()
#undef close
#define close soclose
#undef ioctl
#define ioctl sockioctl
#else
#define SocketInitOnce() /**/
#endif
/* this is still not there */
#define SOCKET int
#else
/* others don't need this */
#define SocketInitOnce() /**/
#endif

#define MIN_BACKLOG 128
#ifdef SOMAXCONN
#if SOMAXCONN > MIN_BACKLOG
#define BACKLOG SOMAXCONN
#endif
#endif
#ifndef BACKLOG
#define BACKLOG MIN_BACKLOG
#endif
/*
 * This is the Socket implementation of the X Transport service layer
 *
 * This file contains the implementation for both the UNIX and INET domains,
 * and can be built for either one, or both.
 *
 */

typedef struct _Sockettrans2dev {      
    char	*transname;
    int		family;
    int		devcotsname;
    int		devcltsname;
    int		protocol;
} Sockettrans2dev;

static Sockettrans2dev Sockettrans2devtab[] = {
#ifdef TCPCONN
    {"inet",AF_INET,SOCK_STREAM,SOCK_DGRAM,0},
    {"tcp",AF_INET,SOCK_STREAM,SOCK_DGRAM,0},
#endif /* TCPCONN */
#ifdef UNIXCONN
    {"unix",AF_UNIX,SOCK_STREAM,SOCK_DGRAM,0},
#if !defined(LOCALCONN)
    {"local",AF_UNIX,SOCK_STREAM,SOCK_DGRAM,0},
#endif /* !LOCALCONN */
#endif /* UNIXCONN */
};

#define NUMSOCKETFAMILIES (sizeof(Sockettrans2devtab)/sizeof(Sockettrans2dev))


#ifdef UNIXCONN

#ifdef hpux

#if defined(X11_t)
#define UNIX_PATH "/usr/spool/sockets/X11/"
#define UNIX_DIR "/usr/spool/sockets/X11"
#define OLD_UNIX_PATH "/tmp/.X11-unix/X"
#endif /* X11_t */
#if defined(XIM_t)
#define UNIX_PATH "/usr/spool/sockets/XIM/"
#define UNIX_DIR "/usr/spool/sockets/XIM"
#define OLD_UNIX_PATH "/tmp/.XIM-unix/XIM"
#endif /* XIM_t */
#if defined(FS_t) || defined(FONT_t)
#define UNIX_PATH "/usr/spool/sockets/fontserv/"
#define UNIX_DIR "/usr/spool/sockets/fontserv"
#endif /* FS_t || FONT_t */
#if defined(ICE_t)
#define UNIX_PATH "/usr/spool/sockets/ICE/"
#define UNIX_DIR "/usr/spool/sockets/ICE"
#endif /* ICE_t */
#if defined(TEST_t)
#define UNIX_PATH "/usr/spool/sockets/xtrans_test/"
#define UNIX_DIR "/usr/spool/sockets/xtrans_test"
#endif

#else /* !hpux */

#if defined(X11_t)
#define UNIX_PATH "/tmp/.X11-unix/X"
#define UNIX_DIR "/tmp/.X11-unix"
#endif /* X11_t */
#if defined(XIM_t)
#define UNIX_PATH "/tmp/.XIM-unix/XIM"
#define UNIX_DIR "/tmp/.XIM-unix"
#endif /* XIM_t */
#if defined(FS_t) || defined(FONT_t)
#define UNIX_PATH "/tmp/.font-unix/fs"
#define UNIX_DIR "/tmp/.font-unix"
#endif /* FS_t || FONT_t */
#if defined(ICE_t)
#define UNIX_PATH "/tmp/.ICE-unix/"
#define UNIX_DIR "/tmp/.ICE-unix"
#endif /* ICE_t */
#if defined(TEST_t)
#define UNIX_PATH "/tmp/.Test-unix/test"
#define UNIX_DIR "/tmp/.Test-unix"
#endif

#endif /* hpux */

#endif /* UNIXCONN */


/*
 * These are some utility function used by the real interface function below.
 */

static int
TRANS(SocketSelectFamily) (family)

char *family;

{
    int     i;

    PRMSG (3,"SocketSelectFamily(%s)\n", family, 0, 0);

    for (i = 0; i < NUMSOCKETFAMILIES;i++)
    {
        if (!strcmp (family, Sockettrans2devtab[i].transname))
	    return i;
    }

    return -1;
}


/*
 * This function gets the local address of the socket and stores it in the
 * XtransConnInfo structure for the connection.
 */

static int
TRANS(SocketINETGetAddr) (ciptr)

XtransConnInfo ciptr;

{
    struct sockaddr_in 	sockname;
#if defined(SVR4) || defined(SCO325)
    size_t namelen = sizeof sockname;
#else
    int namelen = sizeof sockname;
#endif

    PRMSG (3,"SocketINETGetAddr(%x)\n", ciptr, 0, 0);

    if (getsockname (ciptr->fd,(struct sockaddr *) &sockname, &namelen) < 0)
    {
	PRMSG (1,"SocketINETGetAddr: getsockname() failed: %d\n",
	    EGET(),0, 0);
	return -1;
    }

    /*
     * Everything looks good: fill in the XtransConnInfo structure.
     */

    if ((ciptr->addr = (char *) xalloc (namelen)) == NULL)
    {
        PRMSG (1,
	    "SocketINETGetAddr: Can't allocate space for the addr\n",
	    0, 0, 0);
        return -1;
    }

    ciptr->family = sockname.sin_family;
    ciptr->addrlen = namelen;
    memcpy (ciptr->addr, &sockname, ciptr->addrlen);

    return 0;
}


/*
 * This function gets the remote address of the socket and stores it in the
 * XtransConnInfo structure for the connection.
 */

static int
TRANS(SocketINETGetPeerAddr) (ciptr)

XtransConnInfo ciptr;

{
    struct sockaddr_in 	sockname;
#if defined(SVR4) || defined(SCO325)
    size_t namelen = sizeof sockname;
#else
    int namelen = sizeof sockname;
#endif

    PRMSG (3,"SocketINETGetPeerAddr(%x)\n", ciptr, 0, 0);

    if (getpeername (ciptr->fd, (struct sockaddr *) &sockname, &namelen) < 0)
    {
	PRMSG (1,"SocketINETGetPeerAddr: getpeername() failed: %d\n",
	    EGET(), 0, 0);
	return -1;
    }

    /*
     * Everything looks good: fill in the XtransConnInfo structure.
     */

    if ((ciptr->peeraddr = (char *) xalloc (namelen)) == NULL)
    {
        PRMSG (1,
	   "SocketINETGetPeerAddr: Can't allocate space for the addr\n",
	   0, 0, 0);
        return -1;
    }

    ciptr->peeraddrlen = namelen;
    memcpy (ciptr->peeraddr, &sockname, ciptr->peeraddrlen);

    return 0;
}


static XtransConnInfo
TRANS(SocketOpen) (i, type)

int i;
int type;

{
    XtransConnInfo	ciptr;

    PRMSG (3,"SocketOpen(%d,%d)\n", i, type, 0);

    if ((ciptr = (XtransConnInfo) xcalloc (
	1, sizeof(struct _XtransConnInfo))) == NULL)
    {
	PRMSG (1, "SocketOpen: malloc failed\n", 0, 0, 0);
	return NULL;
    }

    if ((ciptr->fd = socket(Sockettrans2devtab[i].family, type,
	Sockettrans2devtab[i].protocol)) < 0
#ifndef WIN32
#if (defined(X11_t) && !defined(USE_POLL)) || defined(FS_t) || defined(FONT_t)
       || ciptr->fd >= TRANS_OPEN_MAX
#endif
#endif
      ) {
	PRMSG (1, "SocketOpen: socket() failed for %s\n",
	    Sockettrans2devtab[i].transname, 0, 0);

	xfree ((char *) ciptr);
	return NULL;
    }

#ifdef TCP_NODELAY
    if (Sockettrans2devtab[i].family == AF_INET)
    {
	/*
	 * turn off TCP coalescence for INET sockets
	 */

	int tmp = 1;
	setsockopt (ciptr->fd, IPPROTO_TCP, TCP_NODELAY,
	    (char *) &tmp, sizeof (int));
    }
#endif

    return ciptr;
}


#ifdef TRANS_REOPEN

static XtransConnInfo
TRANS(SocketReopen) (i, type, fd, port)

int  i;
int  type;
int  fd;
char *port;

{
    XtransConnInfo	ciptr;

    PRMSG (3,"SocketReopen(%d,%d,%s)\n", type, fd, port);

    if ((ciptr = (XtransConnInfo) xcalloc (
	1, sizeof(struct _XtransConnInfo))) == NULL)
    {
	PRMSG (1, "SocketReopen: malloc failed\n", 0, 0, 0);
	return NULL;
    }

    ciptr->fd = fd;

    return ciptr;
}

#endif /* TRANS_REOPEN */


/*
 * These functions are the interface supplied in the Xtransport structure
 */

#ifdef TRANS_CLIENT

static XtransConnInfo
TRANS(SocketOpenCOTSClient) (thistrans, protocol, host, port)

Xtransport *thistrans;
char 	   *protocol;
char 	   *host;
char       *port;

{
    XtransConnInfo	ciptr;
    int			i;

    PRMSG (2, "SocketOpenCOTSClient(%s,%s,%s)\n",
	protocol, host, port);

    SocketInitOnce();

    if ((i = TRANS(SocketSelectFamily) (thistrans->TransName)) < 0)
    {
	PRMSG (1,
       "SocketOpenCOTSClient: Unable to determine socket type for %s\n",
	    thistrans->TransName, 0, 0);
	return NULL;
    }

    if ((ciptr = TRANS(SocketOpen) (
	i, Sockettrans2devtab[i].devcotsname)) == NULL)
    {
	PRMSG (1,"SocketOpenCOTSClient: Unable to open socket for %s\n",
	    thistrans->TransName, 0, 0);
	return NULL;
    }

    /* Save the index for later use */

    ciptr->index = i;

    return ciptr;
}

#endif /* TRANS_CLIENT */


#ifdef TRANS_SERVER

static XtransConnInfo
TRANS(SocketOpenCOTSServer) (thistrans, protocol, host, port)

Xtransport *thistrans;
char 	   *protocol;
char 	   *host;
char 	   *port;

{
    XtransConnInfo	ciptr;
    int	i;

    PRMSG (2,"SocketOpenCOTSServer(%s,%s,%s)\n", protocol, host, port);

    SocketInitOnce();

    if ((i = TRANS(SocketSelectFamily) (thistrans->TransName)) < 0)
    {
	PRMSG (1,
       "SocketOpenCOTSServer: Unable to determine socket type for %s\n",
	    thistrans->TransName, 0, 0);
	return NULL;
    }

    if ((ciptr = TRANS(SocketOpen) (
	i, Sockettrans2devtab[i].devcotsname)) == NULL)
    {
	PRMSG (1,"SocketOpenCOTSServer: Unable to open socket for %s\n",
	    thistrans->TransName, 0, 0);
	return NULL;
    }

    /*
     * Using this prevents the bind() check for an existing server listening
     * on the same port, but it is required for other reasons.
     */
#ifdef SO_REUSEADDR

    /*
     * SO_REUSEADDR only applied to AF_INET
     */

    if (Sockettrans2devtab[i].family == AF_INET)
    {
	int one = 1;
	setsockopt (ciptr->fd, SOL_SOCKET, SO_REUSEADDR,
		    (char *) &one, sizeof (int));
    }
#endif

    /* Save the index for later use */

    ciptr->index = i;

    return ciptr;
}

#endif /* TRANS_SERVER */


#ifdef TRANS_CLIENT

static XtransConnInfo
TRANS(SocketOpenCLTSClient) (thistrans, protocol, host, port)

Xtransport *thistrans;
char 	   *protocol;
char 	   *host;
char 	   *port;

{
    XtransConnInfo	ciptr;
    int			i;

    PRMSG (2,"SocketOpenCLTSClient(%s,%s,%s)\n", protocol, host, port);

    SocketInitOnce();

    if ((i = TRANS(SocketSelectFamily) (thistrans->TransName)) < 0)
    {
	PRMSG (1,
       "SocketOpenCLTSClient: Unable to determine socket type for %s\n",
	    thistrans->TransName, 0, 0);
	return NULL;
    }

    if ((ciptr = TRANS(SocketOpen) (
	i, Sockettrans2devtab[i].devcotsname)) == NULL)
    {
	PRMSG (1,"SocketOpenCLTSClient: Unable to open socket for %s\n",
	      thistrans->TransName, 0, 0);
	return NULL;
    }

    /* Save the index for later use */

    ciptr->index = i;

    return ciptr;
}

#endif /* TRANS_CLIENT */


#ifdef TRANS_SERVER

static XtransConnInfo
TRANS(SocketOpenCLTSServer) (thistrans, protocol, host, port)

Xtransport *thistrans;
char 	   *protocol;
char 	   *host;
char 	   *port;

{
    XtransConnInfo	ciptr;
    int	i;

    PRMSG (2,"SocketOpenCLTSServer(%s,%s,%s)\n", protocol, host, port);

    SocketInitOnce();

    if ((i = TRANS(SocketSelectFamily) (thistrans->TransName)) < 0)
    {
	PRMSG (1,
       "SocketOpenCLTSServer: Unable to determine socket type for %s\n",
	      thistrans->TransName, 0, 0);
	return NULL;
    }

    if ((ciptr = TRANS(SocketOpen) (
	i, Sockettrans2devtab[i].devcotsname)) == NULL)
    {
	PRMSG (1,"SocketOpenCLTSServer: Unable to open socket for %s\n",
	      thistrans->TransName, 0, 0);
	return NULL;
    }

    /* Save the index for later use */

    ciptr->index = i;

    return ciptr;
}

#endif /* TRANS_SERVER */


#ifdef TRANS_REOPEN

static XtransConnInfo
TRANS(SocketReopenCOTSServer) (thistrans, fd, port)

Xtransport *thistrans;
int	   fd;
char	   *port;

{
    XtransConnInfo	ciptr;
    int			i;

    PRMSG (2,
	"SocketReopenCOTSServer(%d, %s)\n", fd, port, 0);

    SocketInitOnce();

    if ((i = TRANS(SocketSelectFamily) (thistrans->TransName)) < 0)
    {
	PRMSG (1,
       "SocketReopenCOTSServer: Unable to determine socket type for %s\n",
	    thistrans->TransName, 0, 0);
	return NULL;
    }

    if ((ciptr = TRANS(SocketReopen) (
	i, Sockettrans2devtab[i].devcotsname, fd, port)) == NULL)
    {
	PRMSG (1,
	    "SocketReopenCOTSServer: Unable to reopen socket for %s\n",
	    thistrans->TransName, 0, 0);
	return NULL;
    }

    /* Save the index for later use */

    ciptr->index = i;

    return ciptr;
}

static XtransConnInfo
TRANS(SocketReopenCLTSServer) (thistrans, fd, port)

Xtransport *thistrans;
int	   fd;
char	   *port;

{
    XtransConnInfo	ciptr;
    int			i;

    PRMSG (2,
	"SocketReopenCLTSServer(%d, %s)\n", fd, port, 0);

    SocketInitOnce();

    if ((i = TRANS(SocketSelectFamily) (thistrans->TransName)) < 0)
    {
	PRMSG (1,
       "SocketReopenCLTSServer: Unable to determine socket type for %s\n",
	      thistrans->TransName, 0, 0);
	return NULL;
    }

    if ((ciptr = TRANS(SocketReopen) (
	i, Sockettrans2devtab[i].devcotsname, fd, port)) == NULL)
    {
	PRMSG (1,
	     "SocketReopenCLTSServer: Unable to reopen socket for %s\n",
	     thistrans->TransName, 0, 0);
	return NULL;
    }

    /* Save the index for later use */

    ciptr->index = i;

    return ciptr;
}

#endif /* TRANS_REOPEN */


static int
TRANS(SocketSetOption) (ciptr, option, arg)

XtransConnInfo 	ciptr;
int 		option;
int 		arg;

{
    PRMSG (2,"SocketSetOption(%d,%d,%d)\n", ciptr->fd, option, arg);

    return -1;
}


#ifdef TRANS_SERVER

static int
TRANS(SocketCreateListener) (ciptr, sockname, socknamelen)

XtransConnInfo	ciptr;
struct sockaddr	*sockname;
int		socknamelen;

{
    int	namelen = socknamelen;
    int	fd = ciptr->fd;
    int	retry;

    PRMSG (3, "SocketCreateListener(%x,%d)\n", ciptr, fd, 0);

    if (Sockettrans2devtab[ciptr->index].family == AF_INET)
	retry = 20;
    else
	retry = 0;

    while (bind (fd, (struct sockaddr *) sockname, namelen) < 0)
    {
	if (errno == EADDRINUSE)
	    return TRANS_ADDR_IN_USE;

	if (retry-- == 0) {
	    PRMSG (1, "SocketCreateListener: failed to bind listener\n",
		0, 0, 0);
	    close (fd);
	    return TRANS_CREATE_LISTENER_FAILED;
	}
#ifdef SO_REUSEADDR
	sleep (1);
#else
	sleep (10);
#endif /* SO_REUSEDADDR */
    }

    if (Sockettrans2devtab[ciptr->index].family == AF_INET) {
#ifdef SO_DONTLINGER
	setsockopt (fd, SOL_SOCKET, SO_DONTLINGER, (char *) NULL, 0);
#else
#ifdef SO_LINGER
    {
	static int linger[2] = { 0, 0 };
	setsockopt (fd, SOL_SOCKET, SO_LINGER,
		(char *) linger, sizeof (linger));
    }
#endif
#endif
}

    if (listen (fd, BACKLOG) < 0)
    {
	PRMSG (1, "SocketCreateListener: listen() failed\n", 0, 0, 0);
	close (fd);
	return TRANS_CREATE_LISTENER_FAILED;
    }
	
    /* Set a flag to indicate that this connection is a listener */

    ciptr->flags = 1;

    return 0;
}


#ifdef TCPCONN
static int
TRANS(SocketINETCreateListener) (ciptr, port)

XtransConnInfo 	ciptr;
char 		*port;

{
    struct sockaddr_in	sockname;
    int		namelen = sizeof(sockname);
    int		status;
    short	tmpport;
    _Xgetservbynameparams sparams;
    struct servent *servp;

#define PORTBUFSIZE	64	/* what is a real size for this? */

    char	portbuf[PORTBUFSIZE];
    
    PRMSG (2, "SocketINETCreateListener(%s)\n", port, 0, 0);

#ifdef X11_t
    /*
     * X has a well known port, that is transport dependent. It is easier
     * to handle it here, than try and come up with a transport independent
     * representation that can be passed in and resolved the usual way.
     *
     * The port that is passed here is really a string containing the idisplay
     * from ConnectDisplay().
     */

    if (is_numeric (port))
    {
	tmpport = (short) atoi (port);

	sprintf (portbuf,"%d", X_TCP_PORT+tmpport);
    }
    else
	strncpy (portbuf, port, PORTBUFSIZE);

    port = portbuf;
#endif

    if (port && *port)
    {
	/* Check to see if the port string is just a number (handles X11) */

	if (!is_numeric (port))
	{
	    if ((servp = _XGetservbyname (port,"tcp",sparams)) == NULL)
	    {
		PRMSG (1,
	     "SocketINETCreateListener: Unable to get service for %s\n",
		      port, 0, 0);
		return TRANS_CREATE_LISTENER_FAILED;
	    }
	    sockname.sin_port = servp->s_port;
	}
	else
	{
	    tmpport = (short) atoi (port);
	    sockname.sin_port = htons (tmpport);
	}
    }
    else
	sockname.sin_port = htons (0);

#ifdef BSD44SOCKETS
    sockname.sin_len = sizeof (sockname);
#endif
    sockname.sin_family = AF_INET;
    sockname.sin_addr.s_addr = htonl (INADDR_ANY);

    if ((status = TRANS(SocketCreateListener) (ciptr,
	(struct sockaddr *) &sockname, namelen)) < 0)
    {
	PRMSG (1,
    "SocketINETCreateListener: ...SocketCreateListener() failed\n",
	    0, 0, 0);
	return status;
    }

    if (TRANS(SocketINETGetAddr) (ciptr) < 0)
    {
	PRMSG (1,
       "SocketINETCreateListener: ...SocketINETGetAddr() failed\n",
	    0, 0, 0);
	return TRANS_CREATE_LISTENER_FAILED;
    }

    return 0;
}

#endif /* SOCKCONN */


#ifdef UNIXCONN

static
TRANS(SocketUNIXCreateListener) (ciptr, port)

XtransConnInfo ciptr;
char *port;

{
    struct sockaddr_un	sockname;
    int			namelen;
    int			oldUmask;
    int			status;

    PRMSG (2, "SocketUNIXCreateListener(%s)\n",
	port ? port : "NULL", 0, 0);

    /* Make sure the directory is created */

    oldUmask = umask (0);

#ifdef UNIX_DIR
    if (!mkdir (UNIX_DIR, 01777))
        chmod (UNIX_DIR, 01777);
#endif

    sockname.sun_family = AF_UNIX;

    if (port && *port) {
	if (*port == '/') { /* a full pathname */
	    sprintf (sockname.sun_path, "%s", port);
	} else {
	    sprintf (sockname.sun_path, "%s%s", UNIX_PATH, port);
	}
    } else {
	sprintf (sockname.sun_path, "%s%d", UNIX_PATH, getpid());
    }

#if defined(BSD44SOCKETS) && !defined(Lynx)
    sockname.sun_len = strlen(sockname.sun_path);
    namelen = SUN_LEN(&sockname);
#else
    namelen = strlen(sockname.sun_path) + sizeof(sockname.sun_family);
#endif

    unlink (sockname.sun_path);

    if ((status = TRANS(SocketCreateListener) (ciptr,
	(struct sockaddr *) &sockname, namelen)) < 0)
    {
	PRMSG (1,
    "SocketUNIXCreateListener: ...SocketCreateListener() failed\n",
	    0, 0, 0);
	return status;
    }

    /*
     * Now that the listener is esablished, create the addr info for
     * this connection. getpeername() doesn't work for UNIX Domain Sockets
     * on some systems (hpux at least), so we will just do it manually, instead
     * of calling something like TRANS(SocketUNIXGetAddr).
     */

    namelen = sizeof (sockname); /* this will always make it the same size */

    if ((ciptr->addr = (char *) xalloc (namelen)) == NULL)
    {
        PRMSG (1,
        "SocketUNIXCreateListener: Can't allocate space for the addr\n",
	    0, 0, 0);
        return TRANS_CREATE_LISTENER_FAILED;
    }

    ciptr->family = sockname.sun_family;
    ciptr->addrlen = namelen;
    memcpy (ciptr->addr, &sockname, ciptr->addrlen);

    (void) umask (oldUmask);

    return 0;
}


static
TRANS(SocketUNIXResetListener) (ciptr)

XtransConnInfo ciptr;

{
    /*
     * See if the unix domain socket has disappeared.  If it has, recreate it.
     */

    struct sockaddr_un 	*unsock = (struct sockaddr_un *) ciptr->addr;
    struct stat		statb;
    int 		status = TRANS_RESET_NOOP;
    void 		TRANS(FreeConnInfo) ();

    PRMSG (3, "SocketUNIXResetListener(%x,%d)\n", ciptr, ciptr->fd, 0);

    if (stat (unsock->sun_path, &statb) == -1 ||
        ((statb.st_mode & S_IFMT) !=
#if (defined (sun) && defined(SVR4)) || defined(NCR) || defined(SCO) || defined(sco) || !defined(S_IFSOCK)
	  		S_IFIFO))
#else
			S_IFSOCK))
#endif
    {
	int oldUmask = umask (0);

#ifdef UNIX_DIR
	if (!mkdir (UNIX_DIR, 01777))
	    chmod (UNIX_DIR, 01777);
#endif

	close (ciptr->fd);
	unlink (unsock->sun_path);

	if ((ciptr->fd = socket (AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
	    TRANS(FreeConnInfo) (ciptr);
	    return TRANS_RESET_FAILURE;
	}

	if (bind (ciptr->fd, (struct sockaddr *) unsock, ciptr->addrlen) < 0)
	{
	    close (ciptr->fd);
	    TRANS(FreeConnInfo) (ciptr);
	    return TRANS_RESET_FAILURE;
	}

	if (listen (ciptr->fd, BACKLOG) < 0)
	{
	    close (ciptr->fd);
	    TRANS(FreeConnInfo) (ciptr);
	    return TRANS_RESET_FAILURE;
	}

	umask (oldUmask);

	status = TRANS_RESET_NEW_FD;
    }

    return status;
}

#endif /* UNIXCONN */


#ifdef TCPCONN

static XtransConnInfo
TRANS(SocketINETAccept) (ciptr, status)

XtransConnInfo ciptr;
int	       *status;

{
    XtransConnInfo	newciptr;
    struct sockaddr_in	sockname;
    int			namelen = sizeof(sockname);

    PRMSG (2, "SocketINETAccept(%x,%d)\n", ciptr, ciptr->fd, 0);

    if ((newciptr = (XtransConnInfo) xcalloc (
	1, sizeof(struct _XtransConnInfo))) == NULL)
    {
	PRMSG (1, "SocketINETAccept: malloc failed\n", 0, 0, 0);
	*status = TRANS_ACCEPT_BAD_MALLOC;
	return NULL;
    }

    if ((newciptr->fd = accept (ciptr->fd,
	(struct sockaddr *) &sockname, &namelen)) < 0)
    {
	PRMSG (1, "SocketINETAccept: accept() failed\n", 0, 0, 0);
	xfree (newciptr);
	*status = TRANS_ACCEPT_FAILED;
	return NULL;
    }

#ifdef TCP_NODELAY
    {
	/*
	 * turn off TCP coalescence for INET sockets
	 */

	int tmp = 1;
	setsockopt (newciptr->fd, IPPROTO_TCP, TCP_NODELAY,
	    (char *) &tmp, sizeof (int));
    }
#endif

    /*
     * Get this address again because the transport may give a more 
     * specific address now that a connection is established.
     */

    if (TRANS(SocketINETGetAddr) (newciptr) < 0)
    {
	PRMSG (1,
	    "SocketINETAccept: ...SocketINETGetAddr() failed:\n",
	    0, 0, 0);
	close (newciptr->fd);
	xfree (newciptr);
	*status = TRANS_ACCEPT_MISC_ERROR;
        return NULL;
    }

    if (TRANS(SocketINETGetPeerAddr) (newciptr) < 0)
    {
	PRMSG (1,
	  "SocketINETAccept: ...SocketINETGetPeerAddr() failed:\n",
		0, 0, 0);
	close (newciptr->fd);
	if (newciptr->addr) xfree (newciptr->addr);
	xfree (newciptr);
	*status = TRANS_ACCEPT_MISC_ERROR;
        return NULL;
    }

    *status = 0;

    return newciptr;
}

#endif /* TCPCONN */


#ifdef UNIXCONN
static XtransConnInfo
TRANS(SocketUNIXAccept) (ciptr, status)

XtransConnInfo ciptr;
int	       *status;

{
    XtransConnInfo	newciptr;
    struct sockaddr_un	sockname;
#if defined(SVR4) || defined(SCO325)
    size_t namelen = sizeof sockname;
#else
    int namelen = sizeof sockname;
#endif

    PRMSG (2, "SocketUNIXAccept(%x,%d)\n", ciptr, ciptr->fd, 0);

    if ((newciptr = (XtransConnInfo) xcalloc (
	1, sizeof(struct _XtransConnInfo))) == NULL)
    {
	PRMSG (1, "SocketUNIXAccept: malloc() failed\n", 0, 0, 0);
	*status = TRANS_ACCEPT_BAD_MALLOC;
	return NULL;
    }

    if ((newciptr->fd = accept (ciptr->fd,
	(struct sockaddr *) &sockname, &namelen)) < 0)
    {
	PRMSG (1, "SocketUNIXAccept: accept() failed\n", 0, 0, 0);
	xfree (newciptr);
	*status = TRANS_ACCEPT_FAILED;
	return NULL;
    }

    /*
     * Get the socket name and the peer name from the listener socket,
     * since this is unix domain.
     */

    if ((newciptr->addr = (char *) xalloc (ciptr->addrlen)) == NULL)
    {
        PRMSG (1,
        "SocketUNIXAccept: Can't allocate space for the addr\n",
	      0, 0, 0);
	close (newciptr->fd);
	xfree (newciptr);
	*status = TRANS_ACCEPT_BAD_MALLOC;
        return NULL;
    }


    newciptr->addrlen = ciptr->addrlen;
    memcpy (newciptr->addr, ciptr->addr, newciptr->addrlen);

    if ((newciptr->peeraddr = (char *) xalloc (ciptr->addrlen)) == NULL)
    {
        PRMSG (1,
	      "SocketUNIXAccept: Can't allocate space for the addr\n",
	      0, 0, 0);
	close (newciptr->fd);
	if (newciptr->addr) xfree (newciptr->addr);
	xfree (newciptr);
	*status = TRANS_ACCEPT_BAD_MALLOC;
        return NULL;
    }
    
    newciptr->peeraddrlen = ciptr->addrlen;
    memcpy (newciptr->peeraddr, ciptr->addr, newciptr->addrlen);

    newciptr->family = AF_UNIX;

    *status = 0;

    return newciptr;
}

#endif /* UNIXCONN */

#endif /* TRANS_SERVER */


#ifdef TRANS_CLIENT

#ifdef TCPCONN
static int
TRANS(SocketINETConnect) (ciptr, host, port)

XtransConnInfo 	ciptr;
char 		*host;
char 		*port;

{
    struct sockaddr_in	sockname;
#if defined(SVR4) || defined(SCO325)
    size_t namelen = sizeof sockname;
#else
    int namelen = sizeof sockname;
#endif
    _Xgethostbynameparams hparams;
    _Xgetservbynameparams sparams;
    struct hostent	*hostp;
    struct servent	*servp;

#define PORTBUFSIZE	64	/* what is a real size for this? */
    char	portbuf[PORTBUFSIZE];

    int			ret;
    short		tmpport;
    unsigned long 	tmpaddr;
    char 		hostnamebuf[256];		/* tmp space */

    PRMSG (2,"SocketINETConnect(%d,%s,%s)\n", ciptr->fd, host, port);

    if (!host)
    {
	hostnamebuf[0] = '\0';
	(void) TRANS(GetHostname) (hostnamebuf, sizeof hostnamebuf);
	host = hostnamebuf;
    }

#ifdef X11_t
    /*
     * X has a well known port, that is transport dependent. It is easier
     * to handle it here, than try and come up with a transport independent
     * representation that can be passed in and resolved the usual way.
     *
     * The port that is passed here is really a string containing the idisplay
     * from ConnectDisplay().
     */

    if (is_numeric (port))
    {
	tmpport = (short) atoi (port);

	sprintf (portbuf, "%d", X_TCP_PORT + tmpport);
    }
    else
#endif
	strncpy (portbuf, port, PORTBUFSIZE);

    /*
     * Build the socket name.
     */

#ifdef BSD44SOCKETS
    sockname.sin_len = sizeof (struct sockaddr_in);
#endif
    sockname.sin_family = AF_INET;

    /*
     * fill in sin_addr
     */

    /* check for ww.xx.yy.zz host string */

    if (isascii (host[0]) && isdigit (host[0])) {
	tmpaddr = inet_addr (host); /* returns network byte order */
    } else {
	tmpaddr = -1;
    }

    PRMSG (4,"SocketINETConnect: inet_addr(%s) = %x\n",
	host, tmpaddr, 0);

    if (tmpaddr == -1)
    {
	if ((hostp = _XGethostbyname(host,hparams)) == NULL)
	{
	    PRMSG (1,"SocketINETConnect: Can't get address for %s\n",
		  host, 0, 0);
	    ESET(EINVAL);
	    return TRANS_CONNECT_FAILED;
	}
	if (hostp->h_addrtype != AF_INET)  /* is IP host? */
	{
	    PRMSG (1,"SocketINETConnect: not INET host%s\n",
		  host, 0, 0);
	    ESET(EPROTOTYPE);
	    return TRANS_CONNECT_FAILED;
	}
	
#if defined(CRAY) && defined(OLDTCP)
        /* Only Cray UNICOS3 and UNICOS4 will define this */
        {
	long t;
	memcpy ((char *)&t, (char *) hostp->h_addr, sizeof (t));
	sockname.sin_addr = t;
        }
#else
        memcpy ((char *) &sockname.sin_addr, (char *) hostp->h_addr,
		sizeof (sockname.sin_addr));
#endif /* CRAY and OLDTCP */
	
    }
else
    {
#if defined(CRAY) && defined(OLDTCP)
	/* Only Cray UNICOS3 and UNICOS4 will define this */
	sockname.sin_addr = tmpaddr;
#else
	sockname.sin_addr.s_addr = tmpaddr;
#endif /* CRAY and OLDTCP */
    }

    /*
     * fill in sin_port
     */
    
    /* Check for number in the port string */

    if (!is_numeric (portbuf))
    {
	if ((servp = _XGetservbyname (portbuf,"tcp",sparams)) == NULL)
	{
	    PRMSG (1,"SocketINETConnect: Can't get service for %s\n",
		  portbuf, 0, 0);
	    return TRANS_CONNECT_FAILED;
	}
	sockname.sin_port = servp->s_port;
    }
    else
    {
	tmpport = (short) atoi (portbuf);
	sockname.sin_port = htons (tmpport);
    }
    
    PRMSG (4,"SocketINETConnect: sockname.sin_port = %d\n",
	  ntohs(sockname.sin_port), 0, 0);

    /*
     * Do the connect()
     */

    if (connect (ciptr->fd, (struct sockaddr *) &sockname, namelen) < 0)
    {
#ifdef WIN32
	int olderrno = WSAGetLastError();
#else
	int olderrno = errno;
#endif

	PRMSG (1,"SocketINETConnect: Can't connect: errno = %d\n",
	  EGET(),0, 0);

	/*
	 * If the error was ECONNREFUSED, the server may be overloaded
	 * and we should try again.
	 *
	 * If the error was EINTR, the connect was interrupted and we
	 * should try again.
	 */

	if (olderrno == ECONNREFUSED || olderrno == EINTR)
	    return TRANS_TRY_CONNECT_AGAIN;
	else
	    return TRANS_CONNECT_FAILED;	
    }
    

    /*
     * Sync up the address fields of ciptr.
     */
    
    if (TRANS(SocketINETGetAddr) (ciptr) < 0)
    {
	PRMSG (1,
	   "SocketINETConnect: ...SocketINETGetAddr() failed:\n",
	   0, 0, 0);
	return TRANS_CONNECT_FAILED;
    }

    if (TRANS(SocketINETGetPeerAddr) (ciptr) < 0)
    {
	PRMSG (1,
	 "SocketINETConnect: ...SocketINETGetPeerAddr() failed:\n",
	      0, 0, 0);
	return TRANS_CONNECT_FAILED;
    }

    return 0;
}

#endif /* TCPCONN */



#ifdef UNIXCONN

/*
 * Make sure 'host' is really local.
 */

static int
UnixHostReallyLocal (host)

char *host;

{
    char hostnamebuf[256];

    TRANS(GetHostname) (hostnamebuf, sizeof (hostnamebuf));

    if (strcmp (hostnamebuf, host) == 0)
    {
	return (1);
    }
    else
    {
	/*
	 * A host may have more than one network address.  If any of the
	 * network addresses of 'host' (specified to the connect call)
	 * match any of the network addresses of 'hostname' (determined
	 * by TRANS(GetHostname)), then the two hostnames are equivalent,
	 * and we know that 'host' is really a local host.
	 */
	char specified_local_addr_list[10][4];
	int scount, equiv, i, j;
	_Xgethostbynameparams hparams;
	struct hostent *hostp;

	if ((hostp = _XGethostbyname (host,hparams)) == NULL)
	    return (0);

	scount = 0;
	while (hostp->h_addr_list[scount] && scount <= 8)
	{
	    /*
	     * The 2nd call to gethostname() overrides the data
	     * from the 1st call, so we must save the address list.
	     */

	    specified_local_addr_list[scount][0] = 
				hostp->h_addr_list[scount][0];
	    specified_local_addr_list[scount][1] = 
				hostp->h_addr_list[scount][1];
	    specified_local_addr_list[scount][2] = 
				hostp->h_addr_list[scount][2];
	    specified_local_addr_list[scount][3] = 
				hostp->h_addr_list[scount][3];
	    scount++;
	}
	if ((hostp = _XGethostbyname (hostnamebuf,hparams)) == NULL)
	    return (0);

	equiv = 0;
	i = 0;

	while (i < scount && !equiv)
	{
	    j = 0;

	    while (hostp->h_addr_list[j])
	    {
		if ((specified_local_addr_list[i][0] == 
					hostp->h_addr_list[j][0]) &&
		    (specified_local_addr_list[i][1] == 
					hostp->h_addr_list[j][1]) &&
		    (specified_local_addr_list[i][2] == 
					hostp->h_addr_list[j][2]) &&
		    (specified_local_addr_list[i][3] == 
					hostp->h_addr_list[j][3]))
		{
		    /* They're equal, so we're done */
		    
		    equiv = 1;
		    break;
		}

		j++;
	    }

	    i++;
	}
	
    return (equiv);
    }
}

static int
TRANS(SocketUNIXConnect) (ciptr, host, port)

XtransConnInfo ciptr;
char *host;
char *port;

{
    struct sockaddr_un	sockname;
    int			namelen;

#if defined(hpux) && defined(X11_t)
    struct sockaddr_un	old_sockname;
    int			old_namelen;
#endif


    PRMSG (2,"SocketUNIXConnect(%d,%s,%s)\n", ciptr->fd, host, port);
    
    /*
     * Make sure 'host' is really local.  If not, we return failure.
     * The reason we make this check is because a process may advertise
     * a "local" network ID for which it can accept connections, but if
     * a process on a remote machine tries to connect to this network ID,
     * we know for sure it will fail.
     */

    if (strcmp (host, "unix") != 0 && !UnixHostReallyLocal (host))
    {
	PRMSG (1,
	   "SocketUNIXConnect: Cannot connect to non-local host %s\n",
	       host, 0, 0);
	return TRANS_CONNECT_FAILED;
    }


    /*
     * Check the port.
     */

    if (!port || !*port)
    {
	PRMSG (1,"SocketUNIXConnect: Missing port specification\n",
	      0, 0, 0);
	return TRANS_CONNECT_FAILED;
    }

    /*
     * Build the socket name.
     */
    
    sockname.sun_family = AF_UNIX;

    if (*port == '/') { /* a full pathname */
	sprintf (sockname.sun_path, "%s", port);
    } else {
	sprintf (sockname.sun_path, "%s%s", UNIX_PATH, port);
    }

#if defined(BSD44SOCKETS) && !defined(Lynx)
    sockname.sun_len = strlen (sockname.sun_path);
    namelen = SUN_LEN (&sockname);
#else
    namelen = strlen (sockname.sun_path) + sizeof (sockname.sun_family);
#endif


#if defined(hpux) && defined(X11_t)
    /*
     * This is gross, but it was in Xlib
     */
    old_sockname.sun_family = AF_UNIX;
    if (*port == '/') { /* a full pathname */
	sprintf (old_sockname.sun_path, "%s", port);
    } else {
	sprintf (old_sockname.sun_path, "%s%s", OLD_UNIX_PATH, port);
    }
    old_namelen = strlen (old_sockname.sun_path) +
	sizeof (old_sockname.sun_family);
#endif


    /*
     * Do the connect()
     */

    if (connect (ciptr->fd, (struct sockaddr *) &sockname, namelen) < 0)
    {
	int olderrno = errno;
	int connected = 0;
	
#if defined(hpux) && defined(X11_t)
	if (olderrno == ENOENT)
	{
	    if (connect (ciptr->fd,
		(struct sockaddr *) &old_sockname, old_namelen) >= 0)
	    {
		connected = 1;
	    }
	    else
		olderrno = errno;
	}
#endif
	if (!connected)
	{
	    errno = olderrno;
	    
	    PRMSG (1,"SocketUNIXConnect: Can't connect: errno = %d\n",
		  EGET(),0, 0);

	    if (olderrno == ENOENT || olderrno == EINTR)
		return TRANS_TRY_CONNECT_AGAIN;
	    else
		return TRANS_CONNECT_FAILED;
	}
    }

    /*
     * Get the socket name and the peer name from the connect socket,
     * since this is unix domain.
     */

    if ((ciptr->addr = (char *) xalloc(namelen)) == NULL ||
       (ciptr->peeraddr = (char *) xalloc(namelen)) == NULL)
    {
        PRMSG (1,
	"SocketUNIXCreateListener: Can't allocate space for the addr\n",
	      0, 0, 0);
        return TRANS_CONNECT_FAILED;
    }

    ciptr->family = AF_UNIX;
    ciptr->addrlen = namelen;
    ciptr->peeraddrlen = namelen;
    memcpy (ciptr->addr, &sockname, ciptr->addrlen);
    memcpy (ciptr->peeraddr, &sockname, ciptr->peeraddrlen);
    
    return 0;
}

#endif /* UNIXCONN */

#endif /* TRANS_CLIENT */


static int
TRANS(SocketBytesReadable) (ciptr, pend)

XtransConnInfo ciptr;
BytesReadable_t *pend;
{
    PRMSG (2,"SocketBytesReadable(%x,%d,%x)\n",
	ciptr, ciptr->fd, pend);

#ifdef WIN32
    return ioctlsocket ((SOCKET) ciptr->fd, FIONREAD, (u_long *) pend);
#else
#if (defined(i386) && defined(SYSV)) || defined(_SEQUENT_)
    return ioctl (ciptr->fd, I_NREAD, (char *) pend);
#else
#if defined(__EMX__)
    return ioctl (ciptr->fd, FIONREAD, (char*) pend, sizeof(int));
#else
    return ioctl (ciptr->fd, FIONREAD, (char *) pend);
#endif /* __EMX__ */
#endif /* i386 && SYSV && !SCO || _SEQUENT_ */
#endif /* WIN32 */
}


static int
TRANS(SocketRead) (ciptr, buf, size)

XtransConnInfo	ciptr;
char		*buf;
int		size;

{
    PRMSG (2,"SocketRead(%d,%x,%d)\n", ciptr->fd, buf, size);

#if defined(WIN32) || defined(__EMX__)
    return recv ((SOCKET)ciptr->fd, buf, size, 0);
#else
    return read (ciptr->fd, buf, size);
#endif /* WIN32 */
}


static int
TRANS(SocketWrite) (ciptr, buf, size)

XtransConnInfo ciptr;
char 	       *buf;
int 	       size;

{
    PRMSG (2,"SocketWrite(%d,%x,%d)\n", ciptr->fd, buf, size);

#if defined(WIN32) || defined(__EMX__)
    return send ((SOCKET)ciptr->fd, buf, size, 0);
#else
    return write (ciptr->fd, buf, size);
#endif /* WIN32 */
}


static int
TRANS(SocketReadv) (ciptr, buf, size)

XtransConnInfo	ciptr;
struct iovec 	*buf;
int 		size;

{
    PRMSG (2,"SocketReadv(%d,%x,%d)\n", ciptr->fd, buf, size);

    return READV (ciptr, buf, size);
}


static int
TRANS(SocketWritev) (ciptr, buf, size)

XtransConnInfo 	ciptr;
struct iovec 	*buf;
int 		size;

{
    PRMSG (2,"SocketWritev(%d,%x,%d)\n", ciptr->fd, buf, size);

    return WRITEV (ciptr, buf, size);
}


static int
TRANS(SocketDisconnect) (ciptr)

XtransConnInfo ciptr;

{
    PRMSG (2,"SocketDisconnect(%x,%d)\n", ciptr, ciptr->fd, 0);

    return shutdown (ciptr->fd, 2); /* disallow further sends and receives */
}


#ifdef TCPCONN
static int
TRANS(SocketINETClose) (ciptr)

XtransConnInfo ciptr;

{
    PRMSG (2,"SocketINETClose(%x,%d)\n", ciptr, ciptr->fd, 0);

    return close (ciptr->fd);
}

#endif /* TCPCONN */


#ifdef UNIXCONN
static int
TRANS(SocketUNIXClose) (ciptr)

XtransConnInfo ciptr;

{
    /*
     * If this is the server side, then once the socket is closed,
     * it must be unlinked to completely close it
     */

    struct sockaddr_un	*sockname = (struct sockaddr_un *) ciptr->addr;
    char	path[200]; /* > sizeof sun_path +1 */
    int ret;

    PRMSG (2,"SocketUNIXClose(%x,%d)\n", ciptr, ciptr->fd, 0);

    ret = close(ciptr->fd);

    if (ciptr->flags
       && sockname
       && sockname->sun_family == AF_UNIX
       && sockname->sun_path[0])
    {
	strncpy (path, sockname->sun_path,
		ciptr->addrlen - sizeof (sockname->sun_family));
	unlink (path);
    }

    return ret;
}

static int
TRANS(SocketUNIXCloseForCloning) (ciptr)

XtransConnInfo ciptr;

{
    /*
     * Don't unlink path.
     */

    int ret;

    PRMSG (2,"SocketUNIXCloseForCloning(%x,%d)\n",
	ciptr, ciptr->fd, 0);

    ret = close(ciptr->fd);

    return ret;
}

#endif /* UNIXCONN */


#ifdef TCPCONN
Xtransport	TRANS(SocketTCPFuncs) = {
	/* Socket Interface */
	"tcp",
        0,
#ifdef TRANS_CLIENT
	TRANS(SocketOpenCOTSClient),
#endif /* TRANS_CLIENT */
#ifdef TRANS_SERVER
	TRANS(SocketOpenCOTSServer),
#endif /* TRANS_SERVER */
#ifdef TRANS_CLIENT
	TRANS(SocketOpenCLTSClient),
#endif /* TRANS_CLIENT */
#ifdef TRANS_SERVER
	TRANS(SocketOpenCLTSServer),
#endif /* TRANS_SERVER */
#ifdef TRANS_REOPEN
	TRANS(SocketReopenCOTSServer),
	TRANS(SocketReopenCLTSServer),
#endif
	TRANS(SocketSetOption),
#ifdef TRANS_SERVER
	TRANS(SocketINETCreateListener),
	NULL,		       			/* ResetListener */
	TRANS(SocketINETAccept),
#endif /* TRANS_SERVER */
#ifdef TRANS_CLIENT
	TRANS(SocketINETConnect),
#endif /* TRANS_CLIENT */
	TRANS(SocketBytesReadable),
	TRANS(SocketRead),
	TRANS(SocketWrite),
	TRANS(SocketReadv),
	TRANS(SocketWritev),
	TRANS(SocketDisconnect),
	TRANS(SocketINETClose),
	TRANS(SocketINETClose),
	};

Xtransport	TRANS(SocketINETFuncs) = {
	/* Socket Interface */
	"inet",
	TRANS_ALIAS,
#ifdef TRANS_CLIENT
	TRANS(SocketOpenCOTSClient),
#endif /* TRANS_CLIENT */
#ifdef TRANS_SERVER
	TRANS(SocketOpenCOTSServer),
#endif /* TRANS_SERVER */
#ifdef TRANS_CLIENT
	TRANS(SocketOpenCLTSClient),
#endif /* TRANS_CLIENT */
#ifdef TRANS_SERVER
	TRANS(SocketOpenCLTSServer),
#endif /* TRANS_SERVER */
#ifdef TRANS_REOPEN
	TRANS(SocketReopenCOTSServer),
	TRANS(SocketReopenCLTSServer),
#endif
	TRANS(SocketSetOption),
#ifdef TRANS_SERVER
	TRANS(SocketINETCreateListener),
	NULL,		       			/* ResetListener */
	TRANS(SocketINETAccept),
#endif /* TRANS_SERVER */
#ifdef TRANS_CLIENT
	TRANS(SocketINETConnect),
#endif /* TRANS_CLIENT */
	TRANS(SocketBytesReadable),
	TRANS(SocketRead),
	TRANS(SocketWrite),
	TRANS(SocketReadv),
	TRANS(SocketWritev),
	TRANS(SocketDisconnect),
	TRANS(SocketINETClose),
	TRANS(SocketINETClose),
	};

#endif /* TCPCONN */

#ifdef UNIXCONN
#if !defined(LOCALCONN)
Xtransport	TRANS(SocketLocalFuncs) = {
	/* Socket Interface */
	"local",
	0,
#ifdef TRANS_CLIENT
	TRANS(SocketOpenCOTSClient),
#endif /* TRANS_CLIENT */
#ifdef TRANS_SERVER
	TRANS(SocketOpenCOTSServer),
#endif /* TRANS_SERVER */
#ifdef TRANS_CLIENT
	TRANS(SocketOpenCLTSClient),
#endif /* TRANS_CLIENT */
#ifdef TRANS_SERVER
	TRANS(SocketOpenCLTSServer),
#endif /* TRANS_SERVER */
#ifdef TRANS_REOPEN
	TRANS(SocketReopenCOTSServer),
	TRANS(SocketReopenCLTSServer),
#endif
	TRANS(SocketSetOption),
#ifdef TRANS_SERVER
	TRANS(SocketUNIXCreateListener),
	TRANS(SocketUNIXResetListener),
	TRANS(SocketUNIXAccept),
#endif /* TRANS_SERVER */
#ifdef TRANS_CLIENT
	TRANS(SocketUNIXConnect),
#endif /* TRANS_CLIENT */
	TRANS(SocketBytesReadable),
	TRANS(SocketRead),
	TRANS(SocketWrite),
	TRANS(SocketReadv),
	TRANS(SocketWritev),
	TRANS(SocketDisconnect),
	TRANS(SocketUNIXClose),
	TRANS(SocketUNIXCloseForCloning),
	};
#endif /* !LOCALCONN */

Xtransport	TRANS(SocketUNIXFuncs) = {
	/* Socket Interface */
	"unix",
#if !defined(LOCALCONN)
        TRANS_ALIAS,
#else
	0,
#endif
#ifdef TRANS_CLIENT
	TRANS(SocketOpenCOTSClient),
#endif /* TRANS_CLIENT */
#ifdef TRANS_SERVER
	TRANS(SocketOpenCOTSServer),
#endif /* TRANS_SERVER */
#ifdef TRANS_CLIENT
	TRANS(SocketOpenCLTSClient),
#endif /* TRANS_CLIENT */
#ifdef TRANS_SERVER
	TRANS(SocketOpenCLTSServer),
#endif /* TRANS_SERVER */
#ifdef TRANS_REOPEN
	TRANS(SocketReopenCOTSServer),
	TRANS(SocketReopenCLTSServer),
#endif
	TRANS(SocketSetOption),
#ifdef TRANS_SERVER
	TRANS(SocketUNIXCreateListener),
	TRANS(SocketUNIXResetListener),
	TRANS(SocketUNIXAccept),
#endif /* TRANS_SERVER */
#ifdef TRANS_CLIENT
	TRANS(SocketUNIXConnect),
#endif /* TRANS_CLIENT */
	TRANS(SocketBytesReadable),
	TRANS(SocketRead),
	TRANS(SocketWrite),
	TRANS(SocketReadv),
	TRANS(SocketWritev),
	TRANS(SocketDisconnect),
	TRANS(SocketUNIXClose),
	TRANS(SocketUNIXCloseForCloning),
	};

#endif /* UNIXCONN */
