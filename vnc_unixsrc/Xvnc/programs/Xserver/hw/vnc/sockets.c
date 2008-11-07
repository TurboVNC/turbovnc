/*
 * sockets.c - deal with TCP & UDP sockets.
 *
 * This code should be independent of any changes in the RFB protocol.  It just
 * deals with the X server scheduling stuff, calling rfbNewClientConnection and
 * rfbProcessClientMessage to actually deal with the protocol.  If a socket
 * needs to be closed for any reason then rfbCloseSock should be called, and
 * this in turn will call rfbClientConnectionGone.  To make an active
 * connection out, call rfbConnect - note that this does _not_ call
 * rfbNewClientConnection.
 *
 * This file is divided into two types of function.  Those beginning with
 * "rfb" are specific to sockets using the RFB protocol.  Those without the
 * "rfb" prefix are more general socket routines (which are used by the http
 * code).
 *
 * Thanks to Karl Hakimian for pointing out that some platforms return EAGAIN
 * not EWOULDBLOCK.
 */

/*
 *  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 *  USA.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

#ifndef USE_LIBWRAP
#define USE_LIBWRAP 0
#endif
#if USE_LIBWRAP
#include <syslog.h>
#include <tcpd.h>
int allow_severity = LOG_INFO;
int deny_severity = LOG_WARNING;
#endif

#include "rfb.h"


int rfbMaxClientWait = 20000;	/* time (ms) after which we decide client has
				   gone away - needed to stop us hanging */

int rfbPort = 0;
int rfbListenSock = -1;

int udpPort = 0;
int udpSock = -1;
Bool udpSockConnected = FALSE;
static struct sockaddr_in udpRemoteAddr;

static fd_set allFds;
static int maxFd = 0;


/*
 * rfbInitSockets sets up the TCP and UDP sockets to listen for RFB
 * connections.  It does nothing if called again.
 */

void
rfbInitSockets()
{
    static Bool done = FALSE;

    if (done)
	return;

    done = TRUE;

    if (inetdSock != -1) {
	const int one = 1;

	if (fcntl(inetdSock, F_SETFL, O_NONBLOCK) < 0) {
	    rfbLogPerror("fcntl");
	    exit(1);
	}

	if (setsockopt(inetdSock, IPPROTO_TCP, TCP_NODELAY,
		       (char *)&one, sizeof(one)) < 0) {
	    rfbLogPerror("setsockopt");
	    exit(1);
	}

    	AddEnabledDevice(inetdSock);
    	FD_ZERO(&allFds);
    	FD_SET(inetdSock, &allFds);
    	maxFd = inetdSock;
	return;
    }

    if (rfbPort == 0) {
	rfbPort = 5900 + atoi(display);
    }

    rfbLog("Listening for VNC connections on TCP port %d\n", rfbPort);

    if ((rfbListenSock = ListenOnTCPPort(rfbPort)) < 0) {
	rfbLogPerror("ListenOnTCPPort");
	exit(1);
    }

    AddEnabledDevice(rfbListenSock);

    FD_ZERO(&allFds);
    FD_SET(rfbListenSock, &allFds);
    maxFd = rfbListenSock;

    if (udpPort != 0) {
	rfbLog("rfbInitSockets: listening for input on UDP port %d\n",udpPort);

	if ((udpSock = ListenOnUDPPort(udpPort)) < 0) {
	    rfbLogPerror("ListenOnUDPPort");
	    exit(1);
	}
	AddEnabledDevice(udpSock);
	FD_SET(udpSock, &allFds);
	maxFd = max(udpSock,maxFd);
    }
}


/*
 * rfbCheckFds is called from ProcessInputEvents to check for input on the RFB
 * socket(s).  If there is input to process, the appropriate function in the
 * RFB server code will be called (rfbNewClientConnection,
 * rfbProcessClientMessage, etc).
 */

void
rfbCheckFds()
{
    int nfds;
    fd_set fds;
    struct timeval tv;
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);
    char buf[6];
    const int one = 1;
    int sock;
    static Bool inetdInitDone = FALSE;

    if (!inetdInitDone && inetdSock != -1) {
	rfbNewClientConnection(inetdSock); 
	inetdInitDone = TRUE;
    }

    memcpy((char *)&fds, (char *)&allFds, sizeof(fd_set));
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    nfds = select(maxFd + 1, &fds, NULL, NULL, &tv);
    if (nfds == 0) {
	return;
    }
    if (nfds < 0) {
	rfbLogPerror("rfbCheckFds: select");
	return;
    }

    if (rfbListenSock != -1 && FD_ISSET(rfbListenSock, &fds)) {

	if ((sock = accept(rfbListenSock,
			   (struct sockaddr *)&addr, &addrlen)) < 0) {
	    rfbLogPerror("rfbCheckFds: accept");
	    return;
	}

	if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0) {
	    rfbLogPerror("rfbCheckFds: fcntl");
	    close(sock);
	    return;
	}

	if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
		       (char *)&one, sizeof(one)) < 0) {
	    rfbLogPerror("rfbCheckFds: setsockopt");
	    close(sock);
	    return;
	}

	fprintf(stderr,"\n");

#if USE_LIBWRAP
        if (!hosts_ctl("Xvnc", STRING_UNKNOWN, inet_ntoa(addr.sin_addr),
                       STRING_UNKNOWN)) {
          rfbLog("Rejected connection from client %s\n",
                 inet_ntoa(addr.sin_addr));
          close(sock);
          return;
        }
#endif

	rfbLog("Got connection from client %s\n", inet_ntoa(addr.sin_addr));

	AddEnabledDevice(sock);
	FD_SET(sock, &allFds);
	maxFd = max(sock,maxFd);

	rfbNewClientConnection(sock);

	FD_CLR(rfbListenSock, &fds);
	if (--nfds == 0)
	    return;
    }

    if ((udpSock != -1) && FD_ISSET(udpSock, &fds)) {

	if (recvfrom(udpSock, buf, 1, MSG_PEEK,
		     (struct sockaddr *)&addr, &addrlen) < 0) {

	    rfbLogPerror("rfbCheckFds: UDP: recvfrom");
	    rfbDisconnectUDPSock();

	} else {

	    if (!udpSockConnected ||
		(memcmp(&addr, &udpRemoteAddr, addrlen) != 0))
	    {
		/* new remote end */
		rfbLog("rfbCheckFds: UDP: got connection\n");

		memcpy(&udpRemoteAddr, &addr, addrlen);
		udpSockConnected = TRUE;

		if (connect(udpSock,
			    (struct sockaddr *)&addr, addrlen) < 0) {
		    rfbLogPerror("rfbCheckFds: UDP: connect");
		    rfbDisconnectUDPSock();
		    return;
		}

		rfbNewUDPConnection(udpSock);
	    }

	    rfbProcessUDPInput(udpSock);
	}

	FD_CLR(udpSock, &fds);
	if (--nfds == 0)
	    return;
    }

    for (sock = 0; sock <= maxFd; sock++) {
	if (FD_ISSET(sock, &fds) && FD_ISSET(sock, &allFds)) {
	    rfbProcessClientMessage(sock);
	}
    }
}


void
rfbDisconnectUDPSock()
{
    udpSockConnected = FALSE;
}


void
rfbCloseSock(sock)
    int sock;
{
    close(sock);
    RemoveEnabledDevice(sock);
    FD_CLR(sock, &allFds);
    rfbClientConnectionGone(sock);
    if (sock == inetdSock)
	GiveUp(0);
}


/*
 * rfbWaitForClient can be called to wait for the RFB client to send us a
 * message.  When one is received it is processed by calling
 * rfbProcessClientMessage().
 */

void
rfbWaitForClient(sock)
    int sock;
{
    int n;
    fd_set fds;
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    tv.tv_sec = rfbMaxClientWait / 1000;
    tv.tv_usec = (rfbMaxClientWait % 1000) * 1000;
    n = select(sock+1, &fds, NULL, NULL, &tv);
    if (n < 0) {
	rfbLogPerror("rfbWaitForClient: select");
	exit(1);
    }
    if (n == 0) {
	rfbCloseSock(sock);
	return;
    }

    rfbProcessClientMessage(sock);
}


/*
 * rfbConnect is called to make a connection out to a given TCP address.
 */

int
rfbConnect(host, port)
    char *host;
    int port;
{
    int sock;
    int one = 1;

    fprintf(stderr,"\n");
    rfbLog("Making connection to client on host %s port %d\n",
	   host,port);

    if ((sock = ConnectToTcpAddr(host, port)) < 0) {
	rfbLogPerror("connection failed");
	return -1;
    }

    if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0) {
	rfbLogPerror("fcntl failed");
	close(sock);
	return -1;
    }

    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
		   (char *)&one, sizeof(one)) < 0) {
	rfbLogPerror("setsockopt failed");
	close(sock);
	return -1;
    }

    AddEnabledDevice(sock);
    FD_SET(sock, &allFds);
    maxFd = max(sock,maxFd);

    return sock;
}




/*
 * ReadExact reads an exact number of bytes on a TCP socket.  Returns 1 if
 * those bytes have been read, 0 if the other end has closed, or -1 if an error
 * occurred (errno is set to ETIMEDOUT if it timed out).
 */

int
ReadExact(sock, buf, len)
    int sock;
    char *buf;
    int len;
{
    int n;
    fd_set fds;
    struct timeval tv;

    while (len > 0) {
	n = read(sock, buf, len);

	if (n > 0) {

	    buf += n;
	    len -= n;

	} else if (n == 0) {

	    return 0;

	} else {
	    if (errno != EWOULDBLOCK && errno != EAGAIN) {
		return n;
	    }

	    FD_ZERO(&fds);
	    FD_SET(sock, &fds);
	    tv.tv_sec = rfbMaxClientWait / 1000;
	    tv.tv_usec = (rfbMaxClientWait % 1000) * 1000;
	    n = select(sock+1, &fds, NULL, NULL, &tv);
	    if (n < 0) {
		rfbLogPerror("ReadExact: select");
		return n;
	    }
	    if (n == 0) {
		errno = ETIMEDOUT;
		return -1;
	    }
	}
    }
    return 1;
}



/*
 * WriteExact writes an exact number of bytes on a TCP socket.  Returns 1 if
 * those bytes have been written, or -1 if an error occurred (errno is set to
 * ETIMEDOUT if it timed out).
 */

int
WriteExact(sock, buf, len)
    int sock;
    char *buf;
    int len;
{
    int n;
    fd_set fds;
    struct timeval tv;
    int totalTimeWaited = 0;


    while (len > 0) {
	n = write(sock, buf, len);

	if (n > 0) {

	    buf += n;
	    len -= n;

	} else if (n == 0) {

	    rfbLog("WriteExact: write returned 0?\n");
	    exit(1);

	} else {
	    if (errno != EWOULDBLOCK && errno != EAGAIN) {
		return n;
	    }

	    /* Retry every 5 seconds until we exceed rfbMaxClientWait.  We
	       need to do this because select doesn't necessarily return
	       immediately when the other end has gone away */

	    FD_ZERO(&fds);
	    FD_SET(sock, &fds);
	    tv.tv_sec = 5;
	    tv.tv_usec = 0;
	    n = select(sock+1, NULL, &fds, NULL, &tv);
	    if (n < 0) {
		rfbLogPerror("WriteExact: select");
		return n;
	    }
	    if (n == 0) {
		totalTimeWaited += 5000;
		if (totalTimeWaited >= rfbMaxClientWait) {
		    errno = ETIMEDOUT;
		    return -1;
		}
	    } else {
		totalTimeWaited = 0;
	    }
	}
    }
    return 1;
}


int
ListenOnTCPPort(port)
    int port;
{
    struct sockaddr_in addr;
    int sock;
    int one = 1;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = interface.s_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	return -1;
    }
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
		   (char *)&one, sizeof(one)) < 0) {
	close(sock);
	return -1;
    }
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
	close(sock);
	return -1;
    }
    if (listen(sock, 5) < 0) {
	close(sock);
	return -1;
    }

    return sock;
}


int
ConnectToTcpAddr(host, port)
    char *host;
    int port;
{
    struct hostent *hp;
    int sock;
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if ((addr.sin_addr.s_addr = inet_addr(host)) == -1)
    {
	if (!(hp = gethostbyname(host))) {
	    errno = EINVAL;
	    return -1;
	}
	addr.sin_addr.s_addr = *(unsigned long *)hp->h_addr;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	return -1;
    }

    if (connect(sock, (struct sockaddr *)&addr, (sizeof(addr))) < 0) {
	close(sock);
	return -1;
    }

    return sock;
}


int
ListenOnUDPPort(port)
    int port;
{
    struct sockaddr_in addr;
    int sock;
    int one = 1;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = interface.s_addr;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	return -1;
    }
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
		   (char *)&one, sizeof(one)) < 0) {
	return -1;
    }
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
	return -1;
    }

    return sock;
}
