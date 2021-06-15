/*
 * sockets.c - deal with TCP sockets.
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
 *  Copyright (C) 2021 Steffen Kieß
 *  Copyright (C) 2012-2020 D. R. Commander.  All Rights Reserved.
 *  Copyright (C) 2011 Gernot Tenchio
 *  Copyright (C) 2011 Joel Martin
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
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/un.h>
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


/* Maximum time (in ms) to wait before deciding that the client has gone away -
   needed to prevent the server from hanging */
int rfbMaxClientWait = DEFAULT_MAX_CLIENT_WAIT;

int rfbPort = 0;
const char *rfbUnixPath = NULL;
int rfbUnixMode = 0600;
Bool rfbUnixSocketCreated = FALSE;
int rfbListenSock = -1;
int rfbMaxClientConnections = DEFAULT_MAX_CONNECTIONS;

extern unsigned long long sendBytes;

/* from log.c */
void
AbortServer(void)
    _X_NORETURN;

static void rfbSockNotify(int fd, int ready, void *data);


/*
 * Convenience function to return a string from either an IPv4 or an IPv6
 * address
 */
const char *sockaddr_string(rfbSockAddr *addr, char *buf, int len)
{
  const char *string = NULL;

  if (!addr || !buf || len < 1)
    return "Invalid argument";

  if (addr->u.ss.ss_family == AF_INET6)
    string =
      inet_ntop(addr->u.ss.ss_family, &addr->u.sin6.sin6_addr, buf, len);
  else
    string = inet_ntop(addr->u.ss.ss_family, &addr->u.sin.sin_addr, buf, len);

  if (!string)
    return strerror(errno);

  return string;
}


/*
 * rfbInitSockets sets up the TCP sockets to listen for RFB connections.
 * It does nothing if called again.
 */

void rfbInitSockets(void)
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

    if (setsockopt(inetdSock, IPPROTO_TCP, TCP_NODELAY, (char *)&one,
                   sizeof(one)) < 0) {
      rfbLogPerror("setsockopt");
      exit(1);
    }

    SetNotifyFd(inetdSock, rfbSockNotify, X_NOTIFY_READ, NULL);
    return;
  }

  if (rfbUnixPath) {
    rfbLog(
        "Listening for VNC connections on unix domain socket %s (mode %04o)\n",
        rfbUnixPath, rfbUnixMode);

    if ((rfbListenSock = ListenOnUnixDomainSocket(rfbUnixPath, rfbUnixMode)) <
        0) {
      rfbLogPerror("ListenOnUnixDomainSocket");
      AbortServer(); /* Clean up lock file and X11 socket file and exit */
    }

    /* Make sure that ddxGiveUp() will remove the socket */
    rfbUnixSocketCreated = TRUE;

    SetNotifyFd(rfbListenSock, rfbSockNotify, X_NOTIFY_READ, "UNIX");
    return;
  }

  if (rfbPort == 0)
    rfbPort = 5900 + atoi(display);

  rfbLog("Listening for VNC connections on TCP port %d\n", rfbPort);

  if ((rfbListenSock = ListenOnTCPPort(rfbPort)) < 0) {
    rfbLogPerror("ListenOnTCPPort");
    exit(1);
  }

  SetNotifyFd(rfbListenSock, rfbSockNotify, X_NOTIFY_READ, NULL);
}


static void rfbSockNotify(int fd, int ready, void *data)
{
  rfbSockAddr addr;
  socklen_t addrlen = sizeof(struct sockaddr_storage);
  char addrStr[INET6_ADDRSTRLEN];
  const int one = 1;
  int sock, numClientConnections = 0;
  rfbClientPtr cl, nextCl;

  if (rfbListenSock != -1 && fd == rfbListenSock) {

    if ((sock = accept(rfbListenSock, &addr.u.sa, &addrlen)) < 0) {
      rfbLogPerror("rfbSockNotify: accept");
      return;
    }

    if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0) {
      rfbLogPerror("rfbSockNotify: fcntl");
      close(sock);
      return;
    }

    if (!data) {
      if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&one,
                     sizeof(one)) < 0) {
        rfbLogPerror("rfbSockNotify: setsockopt");
        close(sock);
        return;
      }
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

    for (cl = rfbClientHead; cl; cl = cl->next)
      numClientConnections++;
    if (numClientConnections >= rfbMaxClientConnections) {
      rfbClientRec tempCl;
      rfbProtocolVersionMsg pv;
      const char *errMsg = "Connection limit reached";
      CARD32 secType = rfbSecTypeInvalid;
      CARD32 errMsgLen = strlen(errMsg), errMsgLenWire = Swap32IfLE(errMsgLen);

      memset(&tempCl, 0, sizeof(rfbClientRec));
      tempCl.sock = sock;
      getpeername(sock, &addr.u.sa, &addrlen);
      tempCl.host = strdup(sockaddr_string(&addr, addrStr, INET6_ADDRSTRLEN));
      rfbLog("Limit of %d connections reached-- rejecting %s\n",
             rfbMaxClientConnections, tempCl.host);

      sprintf(pv, rfbProtocolVersionFormat, 3, 3);
      if (WriteExact(&tempCl, pv, sz_rfbProtocolVersionMsg) >= 0 &&
          WriteExact(&tempCl, (char *)&secType, sizeof(CARD32)) >= 0 &&
          WriteExact(&tempCl, (char *)&errMsgLenWire, sizeof(CARD32)) >= 0 &&
          WriteExact(&tempCl, (char *)errMsg, errMsgLen) >= 0) { }

      free(tempCl.host);
      close(sock);
      return;
    }

    rfbLog("Got connection from client %s\n",
           sockaddr_string(&addr, addrStr, INET6_ADDRSTRLEN));

    SetNotifyFd(sock, rfbSockNotify, X_NOTIFY_READ, NULL);

    rfbNewClientConnection(sock);
    return;
  }

  for (cl = rfbClientHead; cl; cl = nextCl) {
    nextCl = cl->next;
    if (fd == cl->sock) {
      do {
        rfbProcessClientMessage(cl);
        CHECK_CLIENT_PTR(cl, break)
      } while (cl->sock > 0 && webSocketsHasDataInBuffer(cl));
    }
  }
}


/*
 * rfbCorkSock enables the TCP cork functionality on Linux to inform the TCP
 * layer to send only complete packets
 */

void rfbCorkSock(int sock)
{
  static int alreadywarned = 0;
#ifdef TCP_CORK
  int one = 1;

  if (setsockopt(sock, IPPROTO_TCP, TCP_CORK, (char *)&one, sizeof(one)) < 0) {
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

void rfbUncorkSock(int sock)
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


void rfbCloseSock(int sock)
{
  close(sock);
  RemoveNotifyFd(sock);
  if (sock == inetdSock)
    GiveUp(0);
}


void rfbCloseClient(rfbClientPtr cl)
{
  int sock = cl->sock;

#if USETLS
  if (cl->sslctx) {
    shutdown(sock, SHUT_RDWR);
    rfbssl_destroy(cl);
  }
#endif
  if (cl->wsctx)
    webSocketsFree(cl);
  close(sock);
  RemoveNotifyFd(sock);
  rfbClientConnectionGone(cl);
  if (sock == inetdSock)
    GiveUp(0);
}


/*
 * rfbConnect is called to make a connection out to a given TCP address.
 */

int rfbConnect(char *host, int port)
{
  int sock;
  int one = 1;

  fprintf(stderr, "\n");
  rfbLog("Making connection to client on host %s port %d\n", host, port);

  if ((sock = ConnectToTcpAddr(host, port)) < 0) {
    rfbLogPerror("connection failed");
    return -1;
  }

  if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0) {
    rfbLogPerror("fcntl failed");
    close(sock);
    return -1;
  }

  if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&one,
                 sizeof(one)) < 0) {
    rfbLogPerror("setsockopt failed");
    close(sock);
    return -1;
  }

  SetNotifyFd(sock, rfbSockNotify, X_NOTIFY_READ, NULL);

  return sock;
}


/*
 * ReadExact reads an exact number of bytes on a TCP socket.  Returns 1 if
 * those bytes have been read, 0 if the other end has closed, or -1 if an error
 * occurred (errno is set to ETIMEDOUT if it timed out).
 */

int ReadExactTimeout(rfbClientPtr cl, char *buf, int len, int timeout)
{
  int n;
  fd_set readfds, exceptfds;
  struct timeval tv;
  int sock = cl->sock;

  while (len > 0) {
    do {
      if (cl->wsctx)
        n = webSocketsDecode(cl, buf, len);
#if USETLS
      else if (cl->sslctx)
        n = rfbssl_read(cl, buf, len);
#endif
      else
        n = read(sock, buf, len);
    } while (n < 0 && errno == EINTR);

    if (n > 0) {

      buf += n;
      len -= n;

    } else if (n == 0) {

      return 0;

    } else {
      if (errno != EWOULDBLOCK && errno != EAGAIN)
        return n;

#if USETLS
      if (cl->sslctx) {
        if (rfbssl_pending(cl))
          continue;
      }
#endif
      FD_ZERO(&readfds);
      FD_SET(sock, &readfds);
      FD_ZERO(&exceptfds);
      FD_SET(sock, &exceptfds);
      tv.tv_sec = timeout / 1000;
      tv.tv_usec = (timeout % 1000) * 1000;
      do {
        n = select(sock + 1, &readfds, NULL, &exceptfds, &tv);
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


int ReadExact(rfbClientPtr cl, char *buf, int len)
{
  return ReadExactTimeout(cl, buf, len, rfbMaxClientWait);
}


/*
 * SkipExact reads an exact number of bytes on a TCP socket into a temporary
 * buffer and then discards them.  Returns 1 on success, 0 if the other end has
 * closed, or -1 if an error occurred (errno is set to ETIMEDOUT if it timed
 * out).
 */

int SkipExact(rfbClientPtr cl, int len)
{
  char *tmpbuf = NULL;
  int bufLen = min(len, 65536), i, retval = 1;

  tmpbuf = (char *)rfbAlloc(bufLen);

  for (i = 0; i < len; i += bufLen) {
    retval = ReadExact(cl, tmpbuf, min(bufLen, len - i));
    if (retval <= 0) break;
  }

  free(tmpbuf);
  return retval;
}


/*
 * PeekExact peeks at an exact number of bytes from a client.  Returns 1 if
 * those bytes have been read, 0 if the other end has closed, or -1 if an
 * error occurred (errno is set to ETIMEDOUT if it timed out).
 */

int PeekExactTimeout(rfbClientPtr cl, char *buf, int len, int timeout)
{
  int n;
  fd_set readfds, exceptfds;
  struct timeval tv;
  int sock = cl->sock;

  while (len > 0) {
    do {
#if USETLS
      if (cl->sslctx)
        n = rfbssl_peek(cl, buf, len);
      else
#endif
      n = recv(sock, buf, len, MSG_PEEK);
    } while (n < 0 && errno == EINTR);

    if (n == len) {

      break;

    } else if (n == 0) {

      return 0;

    } else {
      if (errno != EWOULDBLOCK && errno != EAGAIN)
        return n;

#if USETLS
      if (cl->sslctx) {
        if (rfbssl_pending(cl))
          continue;
      }
#endif
      FD_ZERO(&readfds);
      FD_SET(sock, &readfds);
      FD_ZERO(&exceptfds);
      FD_SET(sock, &exceptfds);
      tv.tv_sec = timeout / 1000;
      tv.tv_usec = (timeout % 1000) * 1000;
      do {
        n = select(sock + 1, &readfds, NULL, &exceptfds, &tv);
      } while (n < 0 && errno == EINTR);
      if (n < 0) {
        rfbLogPerror("PeekExact: select");
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

int WriteExact(rfbClientPtr cl, char *buf, int len)
{
  int n, bytesWritten = 0;
  fd_set fds;
  struct timeval tv;
  int totalTimeWaited = 0;
  int sock = cl->sock;

  if (cl->wsctx) {
    char *tmp = NULL;
    if ((len = webSocketsEncode(cl, buf, len, &tmp)) < 0) {
      rfbLog("WriteExact: WebSockets encode error\n");
      return -1;
    }
    buf = tmp;
  }

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
      if (errno != EWOULDBLOCK && errno != EAGAIN && errno != 0)
        return n;

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

  cl->sockOffset += bytesWritten;

  return 1;
}


int ListenOnTCPPort(int port)
{
  rfbSockAddr addr;
  socklen_t addrlen;
  char hostname[NI_MAXHOST];
  int sock;
  int one = 1;

  memset(&addr, 0, sizeof(addr));
  if (family == AF_INET6) {
    addr.u.sin6.sin6_family = family;
    addr.u.sin6.sin6_port = htons(port);
    addr.u.sin6.sin6_addr = interface6;
    addrlen = sizeof(struct sockaddr_in6);
  } else {
    family = AF_INET;
    addr.u.sin.sin_family = family;
    addr.u.sin.sin_port = htons(port);
    addr.u.sin.sin_addr.s_addr = interface.s_addr;
    addrlen = sizeof(struct sockaddr_in);
  }

  if ((sock = socket(family, SOCK_STREAM, 0)) < 0)
    return -1;

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&one,
                 sizeof(one)) < 0) {
    close(sock);
    return -1;
  }
  if (bind(sock, &addr.u.sa, addrlen) < 0) {
    close(sock);
    return -1;
  }
  if (listen(sock, 5) < 0) {
    close(sock);
    return -1;
  }

  if (getnameinfo(&addr.u.sa, addrlen, hostname, NI_MAXHOST, NULL, 0,
                  NI_NUMERICHOST) == 0)
    rfbLog("  Interface %s\n", hostname);

  return sock;
}


int ListenOnUnixDomainSocket(const char *path, int mode)
{
  struct sockaddr_un addr;
  mode_t saved_umask;
  char hostname[NI_MAXHOST];
  int sock;
  int result;
  int err;

  if (strlen(path) >= sizeof(addr.sun_path)) {
    rfbLog("socket path is too long");
    errno = ENAMETOOLONG;
    return -1;
  }
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, path);

  /* Remove the socket if it exists and is stale */
  if (access(path, F_OK) == 0) {
    /* Check whether the socket is stale by trying to connect to it */
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
      return -1;

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      /* If the socket is stale, delete it */
      if (errno == ECONNREFUSED)
        unlink(path);
    }

    close(sock);
  }

  if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    return -1;

  saved_umask = umask(0777 & ~mode);
  result = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
  err = errno;
  umask(saved_umask);
  if (result < 0) {
    close(sock);
    errno = err;
    return -1;
  }

  if (listen(sock, 5) < 0) {
    close(sock);
    return -1;
  }

  return sock;
}


int ConnectToTcpAddr(char *host, int port)
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

  if ((sock =
       socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) < 0) {
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
