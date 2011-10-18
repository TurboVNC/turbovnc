/*
 * $XConsortium: Flush.c /main/11 1996/11/13 14:44:22 lehors $
 * $XFree86: xc/lib/Xdmcp/Flush.c,v 3.4 1997/01/18 06:52:07 dawes Exp $
 *
 * 
Copyright (c) 1989  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
 * *
 * Author:  Keith Packard, MIT X Consortium
 */

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
#ifndef MINIX
#ifndef Lynx
#include <sys/socket.h>
#else
#include <socket.h>
#endif /* !Lynx */
#endif /* !MINIX */
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
#ifdef MINIX
    struct sockaddr_in *to_addr;
    char *b;
    udp_io_hdr_t *udp_io_hdr;
    int flags, s_errno;
#endif /* MINIX */

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
#ifndef MINIX
    result = sendto (fd, (char *)buffer->data, buffer->pointer, 0,
		     (struct sockaddr *)to, tolen);
    if (result != buffer->pointer)
	return FALSE;
#else /* MINIX */
    to_addr= (struct sockaddr_in *)to;
    b= (char *)Xalloc(buffer->pointer + sizeof(udp_io_hdr_t));
    if (b == NULL)
    	return FALSE;
    udp_io_hdr= (udp_io_hdr_t *)b;
    bcopy((char *)buffer->data, b+sizeof(udp_io_hdr_t), buffer->pointer);
    udp_io_hdr->uih_dst_addr= to_addr->sin_addr.s_addr;
    udp_io_hdr->uih_dst_port= to_addr->sin_port;
    udp_io_hdr->uih_ip_opt_len= 0;
    udp_io_hdr->uih_data_len= buffer->pointer;

    /* Make the write synchronous by turning of asynch I/O */
    flags= fcntl(fd, F_GETFD);
    fcntl(fd, F_SETFD, flags & ~FD_ASYNCHIO);
    result= write(fd, b, buffer->pointer + sizeof(udp_io_hdr_t));
    s_errno= errno;
    Xfree(b);
    fcntl(fd, F_SETFD, flags);
    if (result != buffer->pointer + sizeof(udp_io_hdr_t))
    	return FALSE;
#endif /* MINIX */
#endif
    return TRUE;
}
