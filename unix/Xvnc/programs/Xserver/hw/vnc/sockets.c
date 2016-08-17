/*
 * sockets.c - deal with TCP & UDP sockets.
 *
 * This code should be independent of any changes in the RFB protocol.  It just
 * deals with the X server scheduling stuff, calling rfbNewClientConnection and
 * rfbProcessClientMessage to actually deal with the protocol.  If a socket
 * needs to be closed for any reason then rfbCloseClient should be called, and
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
 *  Copyright (C) 2012-2015 D. R. Commander.  All Rights Reserved.
 *  Copyright (C) 2011 Gernot Tenchio
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 *  USA.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

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


int rfbMaxClientWait = 20000;   /* time (ms) after which we decide client has
                                   gone away - needed to stop us hanging */

int rfbPort = 0;
int rfbListenSock = -1;

int udpPort = 0;
int udpSock = -1;
Bool udpSockConnected = FALSE;
static struct sockaddr_storage udpRemoteAddr;

static fd_set allFds;
static int maxFd = 0;

extern unsigned long long sendBytes;


/*
 * Convenience function to return a string from either an IPv4 or an IPv6
 * address
 */
const char *
sockaddr_string(struct sockaddr_storage *addr, char *buf, int len)
{
    const char *string = NULL;
    if (!addr || !buf || len < 1)
        return "Invalid argument";
    if (addr->ss_family == AF_INET6)
        string = inet_ntop(addr->ss_family,
                           &((struct sockaddr_in6 *)addr)->sin6_addr, buf,
                           len);
    else
        string = inet_ntop(addr->ss_family,
                           &((struct sockaddr_in *)addr)->sin_addr, buf, len);
    if (!string)
        return strerror(errno);
    return string;
}


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
        rfbLog("rfbInitSockets: listening for input on UDP port %d\n",
               udpPort);

        if ((udpSock = ListenOnUDPPort(udpPort)) < 0) {
            rfbLogPerror("ListenOnUDPPort");
            exit(1);
        }
        AddEnabledDevice(udpSock);
        FD_SET(udpSock, &allFds);
        maxFd = max(udpSock, maxFd);
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
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);
    char addrStr[INET6_ADDRSTRLEN];
    char buf[6];
    const int one = 1;
    int sock;
    rfbClientPtr cl;
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

        fprintf(stderr, "\n");

#if USE_LIBWRAP
        if (!hosts_ctl("Xvnc", STRING_UNKNOWN,
                       sockaddr_string(&addr, addrStr, INET6_ADDRSTRLEN),
                       STRING_UNKNOWN)) {
          rfbLog("Rejected connection from client %s\n",
                 sockaddr_string(&addr, addrStr, INET6_ADDRSTRLEN))
          close(sock);
          return;
        }
#endif

        rfbLog("Got connection from client %s\n",
               sockaddr_string(&addr, addrStr, INET6_ADDRSTRLEN));

        AddEnabledDevice(sock);
        FD_SET(sock, &allFds);
        maxFd = max(sock, maxFd);

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

    for (cl = rfbClientHead; cl; cl = cl->next) {
        if (FD_ISSET(cl->sock, &fds) && FD_ISSET(cl->sock, &allFds)) {
            rfbClientPtr cl2;
#if USETLS
            do {
                rfbProcessClientMessage(cl);
                /* Make sure cl hasn't been freed */
                for (cl2 = rfbClientHead; cl2; cl2 = cl2->next) {
                    if (cl2 == cl)
                        break;
                }
                if (cl2 == NULL) return;
            } while (cl->sslctx && rfbssl_pending(cl) > 0);
#else
            rfbProcessClientMessage(cl);
            for (cl2 = rfbClientHead; cl2; cl2 = cl2->next) {
                if (cl2 == cl)
                    break;
            }
            if (cl2 == NULL) return;
#endif
        }
    }
}


/*
 * rfbCorkSock enables the TCP cork functionality on Linux to inform the TCP
 * layer to send only complete packets
 */

void
rfbCorkSock(int sock)
{
    static int alreadywarned = 0;
#ifdef TCP_CORK
    int one = 1;
    if (setsockopt(sock, IPPROTO_TCP, TCP_CORK, (char *)&one,
        sizeof(one)) < 0) {
        if (!alreadywarned) {
            rfbLogPerror("Could not enable TCP corking");
            alreadywarned = 1;
        }
    }
#else
    if (!alreadywarned) {
        rfbLogPerror("TCP corking not available on this system.");
        alreadywarned = 1;
    }
#endif
}


/*
 * rfbUncorkSock disables corking and sends all partially-complete packets
 */

void
rfbUncorkSock(int sock)
{
#ifdef TCP_CORK
    static int alreadywarned = 0;
    int zero = 0;
    if (setsockopt(sock, IPPROTO_TCP, TCP_CORK, (char *)&zero,
        sizeof(zero)) < 0) {
        if (!alreadywarned) {
            rfbLogPerror("Could not disable TCP corking");
            alreadywarned = 1;
        }
    }
#endif
}


void
rfbDisconnectUDPSock()
{
    udpSockConnected = FALSE;
}


void
rfbCloseSock(int sock)
{
    close(sock);
    RemoveEnabledDevice(sock);
    FD_CLR(sock, &allFds);
    if (sock == inetdSock)
        GiveUp(0);
}


void
rfbCloseClient(rfbClientPtr cl)
{
    int sock = cl->sock;
#if USETLS
    if (cl->sslctx)
        rfbssl_destroy(cl);
#endif
    close(sock);
    RemoveEnabledDevice(sock);
    FD_CLR(sock, &allFds);
    rfbClientConnectionGone(cl);
    if (sock == inetdSock)
        GiveUp(0);
}


/*
 * rfbConnect is called to make a connection out to a given TCP address.
 */

int
rfbConnect(char *host, int port)
{
    int sock;
    int one = 1;

    fprintf(stderr, "\n");
    rfbLog("Making connection to client on host %s port %d\n",
           host, port);

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
    maxFd = max(sock, maxFd);

    return sock;
}


/*
 * ReadExact reads an exact number of bytes on a TCP socket.  Returns 1 if
 * those bytes have been read, 0 if the other end has closed, or -1 if an error
 * occurred (errno is set to ETIMEDOUT if it timed out).
 */

int
ReadExact(rfbClientPtr cl, char *buf, int len)
{
    int n;
    fd_set fds;
    struct timeval tv;
    int sock = cl->sock;

    while (len > 0) {
        do {
#if USETLS
            if (cl->sslctx)
                n = rfbssl_read(cl, buf, len);
            else
#endif
            n = read(sock, buf, len);
        } while (n < 0 && errno == EINTR);

        if (n > 0) {

            buf += n;
            len -= n;

        } else if (n == 0) {

            return 0;

        } else {
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                return n;
            }

#if USETLS
            if (cl->sslctx) {
                if (rfbssl_pending(cl))
                    continue;
            }
#endif
            FD_ZERO(&fds);
            FD_SET(sock, &fds);
            tv.tv_sec = rfbMaxClientWait / 1000;
            tv.tv_usec = (rfbMaxClientWait % 1000) * 1000;
            do {
                n = select(sock + 1, &fds, NULL, NULL, &tv);
            } while (n < 0 && errno == EINTR);
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
 * SkipExact reads an exact number of bytes on a TCP socket into a temporary
 * buffer and then discards them.  Returns 1 on success, 0 if the other end has
 * closed, or -1 if an error occurred (errno is set to ETIMEDOUT if it timed
 * out).
 */

int
SkipExact(rfbClientPtr cl, int len)
{
    char *tmpbuf = NULL;
    int bufLen = min(len, 65536), i, retval = 1;

    tmpbuf = (char *)malloc(bufLen);
    if (tmpbuf == NULL) {
        rfbLogPerror("SkipExact: out of memory");
        return -1;
    }

    for (i = 0; i < len; i += bufLen) {
        retval = ReadExact(cl, tmpbuf, min(bufLen, len - i));
        if (retval <= 0) break;
    }

    free(tmpbuf);
    return retval;
}


/*
 * WriteExact writes an exact number of bytes on a TCP socket.  Returns 1 if
 * those bytes have been written, or -1 if an error occurred (errno is set to
 * ETIMEDOUT if it timed out).
 */

int
WriteExact(rfbClientPtr cl, char *buf, int len)
{
    int n, bytesWritten = 0;
    fd_set fds;
    struct timeval tv;
    int totalTimeWaited = 0;
    int sock = cl->sock;

    while (len > 0) {
        do {
#if USETLS
            if (cl->sslctx)
                n = rfbssl_write(cl, buf, len);
            else
#endif
            n = write(sock, buf, len);
        } while (n < 0 && errno == EINTR);

        if (n > 0) {

            buf += n;
            len -= n;
            bytesWritten += n;
            sendBytes += n;

        } else if (n == 0) {

            rfbLog("WriteExact: write returned 0?\n");
            exit(1);

        } else {
            if (errno != EWOULDBLOCK && errno != EAGAIN && errno != 0) {
                return n;
            }

            /* Retry every 5 seconds until we exceed rfbMaxClientWait.  We
               need to do this because select doesn't necessarily return
               immediately when the other end has gone away */

            FD_ZERO(&fds);
            FD_SET(sock, &fds);
            tv.tv_sec = 5;
            tv.tv_usec = 0;
            do {
              n = select(sock + 1, NULL, &fds, NULL, &tv);
            } while (n < 0 && errno == EINTR);
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

    gettimeofday(&cl->lastWrite, NULL);
    cl->sockOffset += bytesWritten;

    return 1;
}


int
ListenOnTCPPort(int port)
{
    struct sockaddr_storage addr;
    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&addr;
    struct sockaddr_in *addr4 = (struct sockaddr_in *)&addr;
    socklen_t addrlen;
    char hostname[NI_MAXHOST];
    int sock;
    int one = 1;

    memset(&addr, 0, sizeof(addr));
    if (family == AF_INET6) {
        addr6->sin6_family = family;
        addr6->sin6_port = htons(port);
        addr6->sin6_addr = interface6;
        addrlen = sizeof(struct sockaddr_in6);
    } else {
        family = AF_INET;
        addr4->sin_family = family;
        addr4->sin_port = htons(port);
        addr4->sin_addr.s_addr = interface.s_addr;
        addrlen = sizeof(struct sockaddr_in);
    }

    if ((sock = socket(family, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                   (char *)&one, sizeof(one)) < 0) {
        close(sock);
        return -1;
    }
    if (bind(sock, (struct sockaddr *)&addr, addrlen) < 0) {
        close(sock);
        return -1;
    }
    if (listen(sock, 5) < 0) {
        close(sock);
        return -1;
    }

    if (getnameinfo((struct sockaddr *)&addr, addrlen, hostname,
                    NI_MAXHOST, NULL, 0, NI_NUMERICHOST) == 0) {
        rfbLog("  Interface %s\n", hostname);
    }
    return sock;
}


int
ConnectToTcpAddr(char *host, int port)
{
    char portname[10];
    int sock;
    struct addrinfo hints, *addr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    snprintf(portname, 10, "%d", port);
    if (getaddrinfo(host, portname, &hints, &addr) != 0)
        return -1;

    if ((sock = socket(addr->ai_family, addr->ai_socktype,
                       addr->ai_protocol)) < 0) {
        freeaddrinfo(addr);
        return -1;
    }

    if (connect(sock, addr->ai_addr, addr->ai_addrlen) < 0) {
        close(sock);
        freeaddrinfo(addr);
        return -1;
    }

    freeaddrinfo(addr);
    return sock;
}


int
ListenOnUDPPort(int port)
{
    struct sockaddr_storage addr;
    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&addr;
    struct sockaddr_in *addr4 = (struct sockaddr_in *)&addr;
    socklen_t addrlen;
    int sock;
    int one = 1;

    memset(&addr, 0, sizeof(addr));
    if (family == AF_INET6) {
        addr6->sin6_family = family;
        addr6->sin6_port = htons(port);
        addr6->sin6_addr = interface6;
        addrlen = sizeof(struct sockaddr_in6);
    } else {
        family = AF_INET;
        addr4->sin_family = family;
        addr4->sin_port = htons(port);
        addr4->sin_addr.s_addr = interface.s_addr;
        addrlen = sizeof(struct sockaddr_in);
    }

    if ((sock = socket(family, SOCK_DGRAM, 0)) < 0) {
        return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                   (char *)&one, sizeof(one)) < 0) {
        return -1;
    }
    if (bind(sock, (struct sockaddr *)&addr, addrlen) < 0) {
        return -1;
    }

    return sock;
}
