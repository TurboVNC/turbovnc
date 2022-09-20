/* Copyright (C) 2018, 2022 D. R. Commander.  All Rights Reserved.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#include "com_jcraft_jsch_agentproxy_usocket_JNIUSocketFactory.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


#define BAILIF0(f) {  \
  if (!(f) || (*env)->ExceptionCheck(env))  \
    goto bailout;  \
}

#define _throwunix() {  \
  jclass _exccls = (*env)->FindClass(env, "java/io/IOException");  \
  BAILIF0(_exccls);  \
  (*env)->ThrowNew(env, _exccls, strerror(errno));  \
  goto bailout;  \
}


JNIEXPORT jint JNICALL Java_com_jcraft_jsch_agentproxy_usocket_JNIUSocketFactory_openSocket
  (JNIEnv *env, jclass cls, jstring jpath)
{
  int sock;
  struct sockaddr_un addr;
  const char *path = NULL;

  if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    _throwunix()

  if (fcntl(sock, F_SETFD, 8) < 0)
    _throwunix()

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  path = (*env)->GetStringUTFChars(env, jpath, 0);
  strncpy(addr.sun_path, path, sizeof(addr.sun_path));
  addr.sun_path[sizeof(addr.sun_path) - 1] = 0;
  (*env)->ReleaseStringUTFChars(env, jpath, path);

  if (connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0)
    _throwunix()

  return sock;

  bailout:
  if (sock >= 0) close(sock);
  return -1;
}


JNIEXPORT jint JNICALL Java_com_jcraft_jsch_agentproxy_usocket_JNIUSocketFactory_readSocket
  (JNIEnv *env, jclass cls, jint fd, jbyteArray jbuf, jint len)
{
  char *buf = NULL;
  size_t retval = 0;

  BAILIF0(buf = (*env)->GetPrimitiveArrayCritical(env, jbuf, 0));
  if ((retval = read(fd, buf, len)) <= 0)
    _throwunix();

  bailout:
  if (buf) (*env)->ReleasePrimitiveArrayCritical(env, jbuf, buf, 0);
  return (jint)retval;
}


JNIEXPORT void JNICALL Java_com_jcraft_jsch_agentproxy_usocket_JNIUSocketFactory_writeSocket
  (JNIEnv *env, jclass cls, jint fd, jbyteArray jbuf, jint len)
{
  char *buf = NULL;

  BAILIF0(buf = (*env)->GetPrimitiveArrayCritical(env, jbuf, 0));
  if (write(fd, buf, len) < 0)
    _throwunix();

  bailout:
  if (buf) (*env)->ReleasePrimitiveArrayCritical(env, jbuf, buf, 0);
}


JNIEXPORT void JNICALL Java_com_jcraft_jsch_agentproxy_usocket_JNIUSocketFactory_closeSocket
  (JNIEnv *env, jclass cls, jint fd)
{
  close(fd);
}
