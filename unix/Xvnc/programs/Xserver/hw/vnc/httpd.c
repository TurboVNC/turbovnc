/*
 * httpd.c - a simple HTTP server
 */

/*
 *  Copyright (C) 2012, 2015, 2017, 2019 D. R. Commander.  All Rights Reserved.
 *  Copyright (C) 2010 University Corporation for Atmospheric Research.
 *                     All Rights Reserved.
 *  Copyright (C) 2002 Constantin Kaplinsky.  All Rights Reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <arpa/inet.h>

#ifndef USE_LIBWRAP
#define USE_LIBWRAP 0
#endif
#if USE_LIBWRAP
#include <tcpd.h>
#include <arpa/inet.h>
#endif

#include "rfb.h"

#define NOT_FOUND_STR "HTTP/1.0 404 Not found\r\n\r\n"  \
                      "<HEAD><TITLE>File Not Found</TITLE></HEAD>\n"  \
                      "<BODY><H1>File Not Found</H1></BODY>\n"

#define OK_STR "HTTP/1.0 200 OK\r\n"
#define CONTENT_STR "Content-Type: application/x-java-jnlp-file\r\n"

static void httpProcessInput(void);
static Bool compareAndSkip(char **ptr, const char *str);
static Bool parseParams(const char *request, char *result, int max_bytes);
static Bool validateString(char *str);

int httpPort = 0;
char *httpDir = NULL;

int httpListenSock = -1;
int httpSock = -1;

#define BUF_SIZE 32768

static char buf[BUF_SIZE];
static size_t buf_filled = 0;
static rfbClientRec cl;

static void httpSockNotify(int fd, int ready, void *data);


/*
 * httpInitSockets sets up the TCP socket to listen for HTTP connections.
 */

void httpInitSockets(void)
{
  static Bool done = FALSE;

  if (done)
    return;

  done = TRUE;

  if (!httpDir)
    return;

  if (rfbAuthDisableHTTP) {
    rfbLog("NOTICE: HTTP server disabled per system policy\n");
    httpDir = NULL;
    return;
  }

  if (httpPort == 0)
    httpPort = 5800 + atoi(display);

  rfbLog("Listening for HTTP connections on TCP port %d\n", httpPort);

  rfbLog("  URL http://%s:%d\n", rfbThisHost, httpPort);

  if ((httpListenSock = ListenOnTCPPort(httpPort)) < 0) {
    rfbLogPerror("ListenOnTCPPort");
    exit(1);
  }

  SetNotifyFd(httpListenSock, httpSockNotify, X_NOTIFY_READ, NULL);
}


static void httpSockNotify(int fd, int ready, void *data)
{
  rfbSockAddr addr;
  socklen_t addrlen = sizeof(struct sockaddr_storage);
#if USE_LIBWRAP
  char addrStr[INET6_ADDRSTRLEN];
#endif

  if (!httpDir)
    return;

  if ((httpSock >= 0) && fd == httpSock) {
    httpProcessInput();
    return;
  }

  if (fd == httpListenSock) {
    int flags;

    if (httpSock >= 0) {
      RemoveNotifyFd(httpSock);
      close(httpSock);
    }

    if ((httpSock = accept(httpListenSock, &addr.u.sa, &addrlen)) < 0) {
      rfbLogPerror("httpSockNotify: accept");
      return;
    }

#if USE_LIBWRAP
    if (!hosts_ctl("Xvnc", STRING_UNKNOWN,
                   sockaddr_string(&addr, addrStr, INET6_ADDRSTRLEN),
                   STRING_UNKNOWN)) {
      rfbLog("Rejected HTTP connection from client %s\n",
             sockaddr_string(&addr, addrStr, INET6_ADDRSTRLEN));
      RemoveNotifyFd(httpSock);
      close(httpSock);
      httpSock = -1;
      return;
    }
#endif

    flags = fcntl(httpSock, F_GETFL);

    if (flags == -1 || fcntl(httpSock, F_SETFL, flags | O_NONBLOCK) == -1) {
      rfbLogPerror("httpSockNotify: fcntl");
      RemoveNotifyFd(httpSock);
      close(httpSock);
      httpSock = -1;
      return;
    }

    SetNotifyFd(httpSock, httpSockNotify, X_NOTIFY_READ, NULL);
  }
}


static void httpCloseSock(void)
{
  close(httpSock);
  RemoveNotifyFd(httpSock);
  httpSock = -1;
  buf_filled = 0;
}


/*
 * httpProcessInput is called when input is received on the HTTP socket.
 */

static void httpProcessInput(void)
{
  rfbSockAddr addr;
  socklen_t addrlen = sizeof(struct sockaddr_storage);
  char addrStr[INET6_ADDRSTRLEN];
  char fullFname[512];
  char params[1024];
  char *ptr;
  char *fname;
  int maxFnameLen;
  int fd;
  Bool performSubstitutions = FALSE;
  char str[256];
  struct passwd *user;
  struct stat st;

  cl.sock = httpSock;

  user = getpwuid(getuid());

  if (strlen(httpDir) > 255) {
    rfbLog("-httpd directory too long\n");
    httpCloseSock();
    return;
  }
  strcpy(fullFname, httpDir);
  fname = &fullFname[strlen(fullFname)];
  maxFnameLen = 511 - strlen(fullFname);

  /* Read data from the HTTP client until we get a complete request. */
  while (1) {
    ssize_t got =
      read(httpSock, buf + buf_filled, sizeof(buf) - buf_filled - 1);

    if (got <= 0) {
      if (got == 0) {
        rfbLog("httpd: premature connection close\n");
      } else {
        if (errno == EAGAIN)
          return;
        rfbLogPerror("httpProcessInput: read");
      }
      httpCloseSock();
      return;
    }

    buf_filled += got;
    buf[buf_filled] = '\0';

    /* Is it complete yet (is there a blank line)? */
    if (strstr(buf, "\r\r") || strstr(buf, "\n\n") ||
        strstr(buf, "\r\n\r\n") || strstr(buf, "\n\r\n\r"))
      break;
  }

  /* Process the request. */
  if (strncmp(buf, "GET ", 4)) {
    rfbLog("httpd: no GET line\n");
    httpCloseSock();
    return;
  } else {
    /* Only use the first line. */
    buf[strcspn(buf, "\n\r")] = '\0';
  }

  if (strlen(buf) > maxFnameLen) {
    rfbLog("httpd: GET line too long\n");
    httpCloseSock();
    return;
  }

  if (sscanf(buf, "GET %s HTTP/1.0", fname) != 1) {
    rfbLog("httpd: couldn't parse GET line\n");
    httpCloseSock();
    return;
  }

  if (fname[0] != '/') {
    rfbLog("httpd: filename didn't begin with '/'\n");
    WriteExact(&cl, NOT_FOUND_STR, strlen(NOT_FOUND_STR));
    httpCloseSock();
    return;
  }

  if (strchr(fname + 1, '/') != NULL) {
    rfbLog("httpd: asking for file in other directory\n");
    WriteExact(&cl, NOT_FOUND_STR, strlen(NOT_FOUND_STR));
    httpCloseSock();
    return;
  }

  getpeername(httpSock, &addr.u.sa, &addrlen);
  rfbLog("httpd: get '%s' for %s\n", fname + 1,
         sockaddr_string(&addr, addrStr, INET6_ADDRSTRLEN));

  /* Extract parameters from the URL string if necessary */

  params[0] = '\0';
  ptr = strchr(fname, '?');
  if (ptr != NULL) {
    *ptr = '\0';
    if (!parseParams(&ptr[1], params, sizeof(params))) {
      params[0] = '\0';
      rfbLog("httpd: bad parameters in the URL\n");
    }
  }

  /* If we were asked for '/', actually read the file VncViewer.jnlp */

  if (strcmp(fname, "/") == 0) {
    strcpy(fname, "/VncViewer.jnlp");
    rfbLog("httpd: defaulting to '%s'\n", fname + 1);
  }

  /* Substitutions are performed on files ending in .jnlp */

  if (strlen(fname) >= 5 && strcmp(&fname[strlen(fname) - 5], ".jnlp") == 0)
    performSubstitutions = TRUE;

  /* Open the file */

  if ((fd = open(fullFname, O_RDONLY)) < 0) {
    rfbLogPerror("httpProcessInput: open");
    WriteExact(&cl, NOT_FOUND_STR, strlen(NOT_FOUND_STR));
    httpCloseSock();
    return;
  }

  if (fstat(fd, &st) < 0) {
    rfbLogPerror("httpProcessInput: fstat");
    WriteExact(&cl, NOT_FOUND_STR, strlen(NOT_FOUND_STR));
    httpCloseSock();
    return;
  }

  WriteExact(&cl, OK_STR, strlen(OK_STR));
  snprintf(str, 256, "Content-Length: %ld\r\n", (long)st.st_size);
  WriteExact(&cl, str, strlen(str));
  strftime(str, 256, "Last-Modified: %a, %d %b %Y %T GMT\r\n",
           gmtime(&st.st_mtime));
  WriteExact(&cl, str, strlen(str));
  WriteExact(&cl, CONTENT_STR, strlen(CONTENT_STR));
  WriteExact(&cl, "\r\n", 2);

  while (1) {
    int n = read(fd, buf, BUF_SIZE - 1);
    if (n < 0) {
      rfbLogPerror("httpProcessInput: read");
      close(fd);
      httpCloseSock();
      return;
    }

    if (n == 0)
      break;

    if (performSubstitutions) {

      /* Substitute $PORT, $SERVER, etc. with the appropriate values.
         This won't quite work properly if the .jnlp file is longer than
         BUF_SIZE, but it's reasonable to assume that .jnlp files will
         always be short. */

      char *dollar;
      ptr = buf;
      buf[n] = 0;       /* make sure it's null-terminated */

      while ((dollar = strchr(ptr, '$'))) {
        WriteExact(&cl, ptr, (dollar - ptr));

        ptr = dollar;

        if (compareAndSkip(&ptr, "$PORT")) {

          sprintf(str, "%d", rfbPort);
          WriteExact(&cl, str, strlen(str));

        } else if (compareAndSkip(&ptr, "$HTTPPORT")) {

          sprintf(str, "%d", httpPort);
          WriteExact(&cl, str, strlen(str));

        } else if (compareAndSkip(&ptr, "$DESKTOP")) {

          WriteExact(&cl, desktopName, strlen(desktopName));

        } else if (compareAndSkip(&ptr, "$DISPLAY")) {

          sprintf(str, "%s:%s", rfbThisHost, display);
          WriteExact(&cl, str, strlen(str));

        } else if (compareAndSkip(&ptr, "$SERVER")) {

          sprintf(str, "%s", rfbThisHost);
          WriteExact(&cl, str, strlen(str));

        } else if (compareAndSkip(&ptr, "$USER")) {

          if (user)
            WriteExact(&cl, user->pw_name, strlen(user->pw_name));
          else
            WriteExact(&cl, "?", 1);

        } else if (compareAndSkip(&ptr, "$PARAMS")) {

          if (params[0] != '\0')
            WriteExact(&cl, params, strlen(params));

        } else {
          if (!compareAndSkip(&ptr, "$$"))
            ptr++;

          if (WriteExact(&cl, "$", 1) < 0) {
            close(fd);
            httpCloseSock();
            return;
          }
        }
      }
      if (WriteExact(&cl, ptr, (&buf[n] - ptr)) < 0)
        break;

    } else {

      /* For files not ending in .jnlp, just write out the buffer */

      if (WriteExact(&cl, buf, n) < 0)
        break;
    }
  }

  close(fd);
  httpCloseSock();
}


static Bool compareAndSkip(char **ptr, const char *str)
{
  if (strncmp(*ptr, str, strlen(str)) == 0) {
    *ptr += strlen(str);
    return TRUE;
  }

  return FALSE;
}


/*
 * Parse the request tail after the '?' character, and format a sequence
 * of <param> tags for inclusion in a JNLP file.
 */

static Bool parseParams(const char *request, char *result, int max_bytes)
{
  char param_request[128];
  char param_formatted[196];
  const char *tail;
  char *delim_ptr;
  char *value_str;
  int cur_bytes, len = 0;

  result[0] = '\0';
  cur_bytes = 0;

  tail = request;
  for (;;) {
    /* Copy individual "name=value" string into a buffer */
    delim_ptr = strchr((char *)tail, '&');
    if (delim_ptr == NULL) {
      if (strlen(tail) >= sizeof(param_request))
        return FALSE;
      strcpy(param_request, tail);
      len = strlen(tail);
    } else {
      len = delim_ptr - tail;
      if (len >= sizeof(param_request))
        return FALSE;
      memcpy(param_request, tail, len);
      param_request[len] = '\0';
    }

    /* len could be zero here! */
    if (len > 0) {
      /* Split the request into parameter name and value */
      value_str = strchr(&param_request[1], '=');
      if (value_str == NULL)
        return FALSE;
      *value_str++ = '\0';
      if (strlen(value_str) == 0)
        return FALSE;

      /* Validate both parameter name and value */
      if (!validateString(param_request) || !validateString(value_str))
        return FALSE;

      /* Prepare JNLP-formatted representation of the name=value pair */
      len = snprintf(param_formatted, sizeof(param_formatted),
                     "<argument>%s=%s</argument>\n", param_request, value_str);
      if ((len >= sizeof(param_formatted)) ||
          (cur_bytes + len + 1 > max_bytes))
        return FALSE;
      strcat(result, param_formatted);
      cur_bytes += len;
    }

    /* Go to the next parameter */
    if (delim_ptr == NULL)
      break;
    tail = delim_ptr + 1;
  }
  return TRUE;
}


/*
 * Check if the string consists only of alphanumeric characters, '+'
 * signs, underscores, and dots. Replace all '+' signs with spaces.
 */

static Bool validateString(char *str)
{
  char *ptr;

  for (ptr = str; *ptr != '\0'; ptr++) {
    if (!isalnum(*ptr) && *ptr != '_' && *ptr != '.' && *ptr != '@' &&
        *ptr != ':') {
      if (*ptr == '+')
        *ptr = ' ';
      else
        return FALSE;
    }
  }
  return TRUE;
}
