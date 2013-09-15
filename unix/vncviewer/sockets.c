/*
 *  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
 *  Copyright (C) 2011-2012 D. R. Commander.  All Rights Reserved.
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

/*
 * sockets.c - functions to deal with sockets.
 */

#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <assert.h>
#include <vncviewer.h>


void PrintInHex(char *buf, int len);

Bool errorMessageOnReadFailure = True;

#define BUF_SIZE 8192
static char buf[BUF_SIZE];
static char *bufoutptr = buf;
static int buffered = 0;
int family = AF_INET;


/*
 * ReadFromRFBServer is called whenever we want to read some data from the RFB
 * server.  It is non-trivial for two reasons:
 *
 * 1. For efficiency, it performs some intelligent buffering, avoiding invoking
 *    the read() system call too often.  For small chunks of data, it simply
 *    copies the data out of an internal buffer.  For large amounts of data, it
 *    reads directly into the buffer provided by the caller.
 *
 * 2. Whenever read() would block, it invokes the Xt event dispatching
 *    mechanism to process X events.  In fact, this is the only place where
 *    these events are processed, as there is no XtAppMainLoop() in the
 *    program.
 */

static Bool rfbsockReady = False;

static void rfbsockReadyCallback(XtPointer clientData, int *fd, XtInputId *id)
{
  rfbsockReady = True;
  XtRemoveInput(*id);
}


static void ProcessXtEvents()
{
  rfbsockReady = False;
  XtAppAddInput(appContext, rfbsock, (XtPointer)XtInputReadMask,
                rfbsockReadyCallback, NULL);
  while (!rfbsockReady) {
    XtAppProcessEvent(appContext, XtIMAll);
  }
}


Bool ReadFromRFBServer(char *out, unsigned int n)
{
  double tRecvStart = 0.0;

  if (benchFile) {
    Bool status = True;
    tRecvStart = gettime();
    if (fread(out, n, 1, benchFile) < 1) {
      if (ferror(benchFile)) {
        perror("Cannot read from session capture");
        clearerr(benchFile);
      }
      status = False;
    }
    tRecv += gettime() - tRecvStart;
    return status;
  }

  if (rfbProfile) tRecvStart = gettime();

  if (n <= buffered) {
    memcpy(out, bufoutptr, n);
    bufoutptr += n;
    buffered -= n;
    if (rfbProfile) tRecv += gettime() - tRecvStart;
    return True;
  }

  memcpy(out, bufoutptr, buffered);

  out += buffered;
  n -= buffered;

  bufoutptr = buf;
  buffered = 0;

  if (n <= BUF_SIZE) {

    while (buffered < n) {
      int i = read(rfbsock, buf + buffered, BUF_SIZE - buffered);
      if (i <= 0) {
        if (i < 0) {
          if (errno == EWOULDBLOCK || errno == EAGAIN) {
            ProcessXtEvents();
            i = 0;
          } else {
            fprintf(stderr, "%s", programName);
            perror(": read");
            return False;
          }
        } else {
          if (errorMessageOnReadFailure) {
            fprintf(stderr, "%s: VNC server closed connection\n", programName);
          }
          return False;
        }
      }
      buffered += i;
      recvBytes += i;
    }

    memcpy(out, bufoutptr, n);
    bufoutptr += n;
    buffered -= n;

  } else {

    while (n > 0) {
      int i = read(rfbsock, out, n);
      if (i <= 0) {
        if (i < 0) {
          if (errno == EWOULDBLOCK || errno == EAGAIN) {
            ProcessXtEvents();
            i = 0;
          } else {
            fprintf(stderr, "%s", programName);
            perror(": read");
            return False;
          }
        } else {
          if (errorMessageOnReadFailure) {
            fprintf(stderr, "%s: VNC server closed connection\n", programName);
          }
          return False;
        }
      }
      out += i;
      recvBytes += i;
      n -= i;
    }
  }

  if (rfbProfile) tRecv += gettime() - tRecvStart;

  return True;
}


/*
 * Write an exact number of bytes, and don't return until you've sent them.
 */

Bool WriteExact(int sock, char *buf, int n)
{
  fd_set fds;
  int i = 0;
  int j;

  while (i < n) {
    j = write(sock, buf + i, (n - i));
    if (j <= 0) {
      if (j < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
          FD_ZERO(&fds);
          FD_SET(rfbsock, &fds);

          if (select(rfbsock + 1, NULL, &fds, NULL, NULL) <= 0) {
            fprintf(stderr, "%s", programName);
            perror(": select");
            return False;
          }
          j = 0;
        } else {
          fprintf(stderr, "%s", programName);
          perror(": write");
          return False;
        }
      } else {
        fprintf(stderr, "%s: write failed\n", programName);
        return False;
      }
    }
    i += j;
  }
  return True;
}


int ConnectToTcpAddr(const char *hostname, int port)
{
  char portname[10];
  int sock;
  struct addrinfo hints, *addr;
  int one = 1;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  snprintf(portname, 10, "%d", port);
  if (strlen(hostname) < 1)
    hostname = NULL;
  if (getaddrinfo(hostname, portname, &hints, &addr) != 0) {
    fprintf(stderr, "Couldn't convert '%s' to host address\n", hostname);
    return -1;
  }

  sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
  if (sock < 0) {
    fprintf(stderr, "%s", programName);
    perror(": ConnectToTcpAddr: socket");
    freeaddrinfo(addr);
    return -1;
  }

  if (connect(sock, addr->ai_addr, addr->ai_addrlen) < 0) {
    fprintf(stderr, "%s", programName);
    perror(": ConnectToTcpAddr: connect");
    close(sock);
    freeaddrinfo(addr);
    return -1;
  }

  freeaddrinfo(addr);

  if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
                 (char *)&one, sizeof(one)) < 0) {
    fprintf(stderr, "%s", programName);
    perror(": ConnectToTcpAddr: setsockopt");
    close(sock);
    return -1;
  }

  return sock;
}


/*
 * FindFreeTcpPort() tries to find an unused TCP port.  Returns 0 on failure.
 */

int FindFreeTcpPort(void)
{
  int sock;
  struct sockaddr_in addr;
  socklen_t n;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    fprintf(stderr, "%s", programName);
    perror(": FindFreeTcpPort: unable to create socket");
    return 0;
  }

  addr.sin_port = 0;
  if (bind (sock, (struct sockaddr *)&addr, sizeof (addr)) < 0) {
    close(sock);
    fprintf(stderr, "%s", programName);
    perror(": FindFreeTcpPort: unable to find free port");
  }

  n = sizeof(addr);
  if (getsockname (sock, (struct sockaddr *)&addr, &n) < 0) {
    close(sock);
    fprintf(stderr, "%s", programName);
    perror(": FindFreeTcpPort: unable to get port number");
  }

  close(sock);
  return ntohs(addr.sin_port);
}


int ListenAtTcpPort(int port)
{
  int sock;
  struct sockaddr_storage addr;
  struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&addr;
  struct sockaddr_in *addr4 = (struct sockaddr_in *)&addr;
  socklen_t addrlen;
  int one = 1;

  memset(&addr, 0, sizeof(addr));
  if (family == AF_INET6) {
    addr6->sin6_family = family;
    addr6->sin6_port = htons(port);
    addr6->sin6_addr = in6addr_any;
    addrlen = sizeof(struct sockaddr_in6);
  } else {
    addr4->sin_family = family;
    addr4->sin_port = htons(port);
    addr4->sin_addr.s_addr = INADDR_ANY;
    addrlen = sizeof(struct sockaddr_in);
  }

  sock = socket(family, SOCK_STREAM, 0);
  if (sock < 0) {
    fprintf(stderr, "%s", programName);
    perror(": ListenAtTcpPort: socket");
    return -1;
  }

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                 (const char *)&one, sizeof(one)) < 0) {
    fprintf(stderr, "%s", programName);
    perror(": ListenAtTcpPort: setsockopt");
    close(sock);
    return -1;
  }

  if (bind(sock, (struct sockaddr *)&addr, addrlen) < 0) {
    fprintf(stderr, "%s", programName);
    perror(": ListenAtTcpPort: bind");
    close(sock);
    return -1;
  }

  if (listen(sock, 5) < 0) {
    fprintf(stderr, "%s", programName);
    perror(": ListenAtTcpPort: listen");
    close(sock);
    return -1;
  }

  return sock;
}


int AcceptTcpConnection(int listenSock)
{
  int sock;
  struct sockaddr_storage addr;
  socklen_t addrlen = sizeof(addr);
  int one = 1;

  sock = accept(listenSock, (struct sockaddr *) &addr, &addrlen);
  if (sock < 0) {
    fprintf(stderr, "%s", programName);
    perror(": AcceptTcpConnection: accept");
    return -1;
  }

  if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
                 (char *)&one, sizeof(one)) < 0) {
    fprintf(stderr, "%s", programName);
    perror(": AcceptTcpConnection: setsockopt");
    close(sock);
    return -1;
  }

  return sock;
}


Bool SetNonBlocking(int sock)
{
  if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0) {
    fprintf(stderr, "%s", programName);
    perror(": AcceptTcpConnection: fcntl");
    return False;
  }
  return True;
}


Bool SameMachine(int sock)
{
  struct sockaddr_storage peeraddr, myaddr;
  struct sockaddr_in *peeraddr4 = (struct sockaddr_in *)&peeraddr;
  struct sockaddr_in6 *peeraddr6 = (struct sockaddr_in6 *)&peeraddr;
  struct sockaddr_in *myaddr4 = (struct sockaddr_in *)&myaddr;
  struct sockaddr_in6 *myaddr6 = (struct sockaddr_in6 *)&myaddr;
  socklen_t addrlen = sizeof(struct sockaddr_storage);

  getpeername(sock, (struct sockaddr *)&peeraddr, &addrlen);
  getsockname(sock, (struct sockaddr *)&myaddr, &addrlen);

  if (myaddr.ss_family == AF_INET6)
    return (!memcmp(&peeraddr6->sin6_addr, &myaddr6->sin6_addr,
                    sizeof(struct in6_addr)));
  else
    return (peeraddr4->sin_addr.s_addr == myaddr4->sin_addr.s_addr);
}


/*
 * Print out the contents of a packet for debugging.
 */

void PrintInHex(char *buf, int len)
{
  int i, j;
  char c, str[17];

  str[16] = 0;

  fprintf(stderr, "ReadExact: ");

  for (i = 0; i < len; i++) {
    if ((i % 16 == 0) && (i != 0))
      fprintf(stderr, "           ");
    c = buf[i];
    str[i % 16] = (((c > 31) && (c < 127)) ? c : '.');
    fprintf(stderr, "%02x ", (unsigned char)c);
    if ((i % 4) == 3)
      fprintf(stderr, " ");
    if ((i % 16) == 15)
      fprintf(stderr, "%s\n", str);
  }
  if ((i % 16) != 0) {
    for (j = i % 16; j < 16; j++) {
      fprintf(stderr, "   ");
      if ((j % 4) == 3) fprintf(stderr, " ");
    }
    str[i % 16] = 0;
    fprintf(stderr, "%s\n", str);
  }

  fflush(stderr);
}
