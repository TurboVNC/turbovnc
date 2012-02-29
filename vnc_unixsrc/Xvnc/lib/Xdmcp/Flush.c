/*
 * $Xorg: Flush.c,v 1.4 2001/02/09 02:03:48 xorgcvs Exp $
 *
 * 
Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 * *
 * Author:  Keith Packard, MIT X Consortium
 */

/* $XFree86: xc/lib/Xdmcp/Flush.c,v 3.8 2001/12/14 19:54:54 dawes Exp $ */

#ifdef WIN32
#define _WILLWINSOCK_
#endif
#include <X11/Xos.h>
#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xdmcp.h>

#ifdef STREAMSCONN
#include <tiuser.h>
#else
#ifdef WIN32
#include <X11/Xwinsock.h>
#else
#ifndef Lynx
#include <sys/socket.h>
#else
#include <socket.h>
#endif /* !Lynx */
#endif
#endif

int
XdmcpFlush (fd, buffer, to, tolen)
    int		    fd;
    XdmcpBufferPtr  buffer;
    XdmcpNetaddr    to;
    int		    tolen;
{
    int result;
#ifdef STREAMSCONN
    struct t_unitdata dataunit;

    dataunit.addr.buf = to;
    dataunit.addr.len = tolen;
    dataunit.opt.len = 0;	/* default options */
    dataunit.udata.buf = (char *)buffer->data;
    dataunit.udata.len = buffer->pointer;
    result = t_sndudata(fd, &dataunit);
    if (result < 0)
	return FALSE;
#else
    result = sendto (fd, (char *)buffer->data, buffer->pointer, 0,
		     (struct sockaddr *)to, tolen);
    if (result != buffer->pointer)
	return FALSE;
#endif
    return TRUE;
}
