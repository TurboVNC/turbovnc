/*
 * $Xorg: Whead.c,v 1.4 2001/02/09 02:03:49 xorgcvs Exp $
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

/* $XFree86: xc/lib/Xdmcp/Whead.c,v 1.4 2001/12/14 19:54:55 dawes Exp $ */

#include <X11/Xos.h>
#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xdmcp.h>

int
XdmcpWriteHeader (
    XdmcpBufferPtr  buffer,
    XdmcpHeaderPtr  header)
{
    BYTE    *newData;

    if ((int)buffer->size < 6 + (int)header->length)
    {
	newData = (BYTE *) Xalloc (XDM_MAX_MSGLEN * sizeof (BYTE));
	if (!newData)
	    return FALSE;
	Xfree ((unsigned long *)(buffer->data));
	buffer->data = newData;
	buffer->size = XDM_MAX_MSGLEN;
    }
    buffer->pointer = 0;
    if (!XdmcpWriteCARD16 (buffer, header->version))
	return FALSE;
    if (!XdmcpWriteCARD16 (buffer, header->opcode))
	return FALSE;
    if (!XdmcpWriteCARD16 (buffer, header->length))
	return FALSE;
    return TRUE;
}
