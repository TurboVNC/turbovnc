/*
 * $XConsortium: Fill.c /main/11 1996/11/13 14:44:18 lehors $
 * $XFree86: xc/lib/Xdmcp/Fill.c,v 3.4 1997/01/18 06:52:06 dawes Exp $
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

#ifndef MINIX
int
XdmcpFill (fd, buffer, from, fromlen)
    int		    fd;
    XdmcpBufferPtr  buffer;
    XdmcpNetaddr    from;	/* return */
    int		    *fromlen;	/* return */
{
    BYTE    *newBuf;
#ifdef STREAMSCONN
    struct t_unitdata dataunit;
    int gotallflag, result;
#endif

    if (buffer->size < XDM_MAX_MSGLEN)
    {
	newBuf = (BYTE *) Xalloc (XDM_MAX_MSGLEN);
	if (newBuf)
	{
	    Xfree (buffer->data);
	    buffer->data = newBuf;
	    buffer->size = XDM_MAX_MSGLEN;
	}
    }
    buffer->pointer = 0;
#ifdef STREAMSCONN
    dataunit.addr.buf = from;
    dataunit.addr.maxlen = *fromlen;
    dataunit.opt.maxlen = 0;	/* don't care to know about options */
    dataunit.udata.buf = (char *)buffer->data;
    dataunit.udata.maxlen = buffer->size;
    result = t_rcvudata (fd, &dataunit, &gotallflag);
    if (result < 0) {
	return FALSE;
    }
    buffer->count = dataunit.udata.len;
    *fromlen = dataunit.addr.len;
#else
    buffer->count = recvfrom (fd, (char*)buffer->data, buffer->size, 0,
			      (struct sockaddr *)from, fromlen);
#endif
    if (buffer->count < 6) {
	buffer->count = 0;
	return FALSE;
    }
    return TRUE;
}
#else /* MINIX */
int
MNX_XdmcpFill (fd, buffer, from, fromlen, data, datalen)
    int		    fd;
    XdmcpBufferPtr  buffer;
    XdmcpNetaddr    from;	/* return */
    int		    *fromlen;	/* return */
    char	    *data;
    int		    datalen;
{
    BYTE    *newBuf;
    struct sockaddr_in *from_addr;
    udp_io_hdr_t *udp_io_hdr;

    if (buffer->size < XDM_MAX_MSGLEN)
    {
	newBuf = (BYTE *) Xalloc (XDM_MAX_MSGLEN);
	if (newBuf)
	{
	    Xfree (buffer->data);
	    buffer->data = newBuf;
	    buffer->size = XDM_MAX_MSGLEN;
	}
    }
    buffer->pointer = 0;
    udp_io_hdr= (udp_io_hdr_t *)data;
    data += sizeof(udp_io_hdr_t) + udp_io_hdr->uih_ip_opt_len;
    datalen -= sizeof(udp_io_hdr_t) + udp_io_hdr->uih_ip_opt_len;
    buffer->count= udp_io_hdr->uih_data_len;
    if (buffer->count > datalen)
    {
    	buffer->count= 0;
    	return FALSE;
    }
    bcopy(data, (char *)buffer->data, buffer->count);
    from_addr= (struct sockaddr_in *)from;
    from_addr->sin_family= AF_INET;
    from_addr->sin_addr.s_addr= udp_io_hdr->uih_src_addr;
    from_addr->sin_port= udp_io_hdr->uih_src_port;
    if (buffer->count < 6) {
	buffer->count = 0;
	return FALSE;
    }
    return TRUE;
}
#endif /* !MINIX */
