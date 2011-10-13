/***********************************************************

Copyright 1987, 1989, 1998  The Open Group

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


Copyright 1987, 1989 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.


******************************************************************/
/* $Xorg: io.c,v 1.6 2001/02/09 02:05:23 xorgcvs Exp $ */
/*****************************************************************
 * i/o functions
 *
 *   WriteToClient, ReadRequestFromClient
 *   InsertFakeRequest, ResetCurrentRequest
 *
 *****************************************************************/
/* $XFree86: xc/programs/Xserver/os/io.c,v 3.34 2002/05/31 18:46:05 dawes Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#if 0
#define DEBUG_COMMUNICATION
#endif
#ifdef WIN32
#include <X11/Xwinsock.h>
#endif
#include <stdio.h>
#include <X11/Xtrans/Xtrans.h>
#include <X11/Xmd.h>
#include <errno.h>
#if !defined(__UNIXOS2__) && !defined(WIN32)
#ifndef Lynx
#include <sys/uio.h>
#else
#include <uio.h>
#endif
#endif
#include <X11/X.h>
#define NEED_REPLIES
#include <X11/Xproto.h>
#include "os.h"
#include "osdep.h"
#include <X11/Xpoll.h>
#include "opaque.h"
#include "dixstruct.h"
#include "misc.h"
#ifdef LBX
#include "colormapst.h"
#include "propertyst.h"
#include "lbxserve.h"
#endif

CallbackListPtr       ReplyCallback;
CallbackListPtr       FlushCallback;

/* check for both EAGAIN and EWOULDBLOCK, because some supposedly POSIX
 * systems are broken and return EWOULDBLOCK when they should return EAGAIN
 */
#ifndef __UNIXOS2__
#ifndef WIN32
#if defined(EAGAIN) && defined(EWOULDBLOCK)
#define ETEST(err) (err == EAGAIN || err == EWOULDBLOCK)
#else
#ifdef EAGAIN
#define ETEST(err) (err == EAGAIN)
#else
#define ETEST(err) (err == EWOULDBLOCK)
#endif
#endif
#else /* WIN32 The socket errorcodes differ from the normal errors*/
#define ETEST(err) (err == EAGAIN || err == WSAEWOULDBLOCK)
#endif
#else /* __UNIXOS2__  Writing to full pipes may return ENOSPC */
#define ETEST(err) (err == EAGAIN || err == EWOULDBLOCK || err == ENOSPC)
#endif

Bool CriticalOutputPending;
int timesThisConnection = 0;
ConnectionInputPtr FreeInputs = (ConnectionInputPtr)NULL;
ConnectionOutputPtr FreeOutputs = (ConnectionOutputPtr)NULL;
OsCommPtr AvailableInput = (OsCommPtr)NULL;

#define get_req_len(req,cli) ((cli)->swapped ? \
			      lswaps((req)->length) : (req)->length)

#ifdef BIGREQS
#include <X11/extensions/bigreqstr.h>

#define get_big_req_len(req,cli) ((cli)->swapped ? \
				  lswapl(((xBigReq *)(req))->length) : \
				  ((xBigReq *)(req))->length)
#endif

#define MAX_TIMES_PER         10

/*
 *   A lot of the code in this file manipulates a ConnectionInputPtr:
 *
 *    -----------------------------------------------
 *   |------- bufcnt ------->|           |           |
 *   |           |- gotnow ->|           |           |
 *   |           |-------- needed ------>|           |
 *   |-----------+--------- size --------+---------->|
 *    -----------------------------------------------
 *   ^           ^
 *   |           |
 *   buffer   bufptr
 *
 *  buffer is a pointer to the start of the buffer.
 *  bufptr points to the start of the current request.
 *  bufcnt counts how many bytes are in the buffer.
 *  size is the size of the buffer in bytes.
 *
 *  In several of the functions, gotnow and needed are local variables
 *  that do the following:
 *
 *  gotnow is the number of bytes of the request that we're
 *  trying to read that are currently in the buffer.
 *  Typically, gotnow = (buffer + bufcnt) - bufptr
 *
 *  needed = the length of the request that we're trying to
 *  read.  Watch out: needed sometimes counts bytes and sometimes
 *  counts CARD32's.
 */


/*****************************************************************
 * ReadRequestFromClient
 *    Returns one request in client->requestBuffer.  The request
 *    length will be in client->req_len.  Return status is:
 *
 *    > 0  if  successful, specifies length in bytes of the request
 *    = 0  if  entire request is not yet available
 *    < 0  if  client should be terminated
 *
 *    The request returned must be contiguous so that it can be
 *    cast in the dispatcher to the correct request type.  Because requests
 *    are variable length, ReadRequestFromClient() must look at the first 4
 *    or 8 bytes of a request to determine the length (the request length is
 *    in the 3rd and 4th bytes of the request unless it is a Big Request
 *    (see the Big Request Extension), in which case the 3rd and 4th bytes
 *    are zero and the following 4 bytes are the request length.
 *
 *    Note: in order to make the server scheduler (WaitForSomething())
 *    "fair", the ClientsWithInput mask is used.  This mask tells which
 *    clients have FULL requests left in their buffers.  Clients with
 *    partial requests require a read.  Basically, client buffers
 *    are drained before select() is called again.  But, we can't keep
 *    reading from a client that is sending buckets of data (or has
 *    a partial request) because others clients need to be scheduled.
 *****************************************************************/

#define YieldControl()				\
        { isItTimeToYield = TRUE;		\
	  timesThisConnection = 0; }
#define YieldControlNoInput()			\
        { YieldControl();			\
	  FD_CLR(fd, &ClientsWithInput); }
#define YieldControlDeath()			\
        { timesThisConnection = 0; }

#ifdef hpux_not_tog
#define LBX_NEED_OLD_SYMBOL_FOR_LOADABLES
#endif

#ifdef LBX
#ifdef LBX_NEED_OLD_SYMBOL_FOR_LOADABLES
#undef ReadRequestFromClient
int
ReadRequestFromClient(ClientPtr client)
{
    return (*client->readRequest)(client);
}
#endif
int
StandardReadRequestFromClient(ClientPtr client)
#else
int
ReadRequestFromClient(ClientPtr client)
#endif
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    ConnectionInputPtr oci = oc->input;
    int fd = oc->fd;
    unsigned int gotnow, needed;
    int result;
    register xReq *request;
    Bool need_header;
#ifdef BIGREQS
    Bool move_header;
#endif

    /* If an input buffer was empty, either free it if it is too big
     * or link it into our list of free input buffers.  This means that
     * different clients can share the same input buffer (at different
     * times).  This was done to save memory.
     */

    if (AvailableInput)
    {
	if (AvailableInput != oc)
	{
	    register ConnectionInputPtr aci = AvailableInput->input;
	    if (aci->size > BUFWATERMARK)
	    {
		xfree(aci->buffer);
		xfree(aci);
	    }
	    else
	    {
		aci->next = FreeInputs;
		FreeInputs = aci;
	    }
	    AvailableInput->input = (ConnectionInputPtr)NULL;
	}
	AvailableInput = (OsCommPtr)NULL;
    }

    /* make sure we have an input buffer */

    if (!oci)
    {
	if ((oci = FreeInputs))
	{
	    FreeInputs = oci->next;
	}
	else if (!(oci = AllocateInputBuffer()))
	{
	    YieldControlDeath();
	    return -1;
	}
	oc->input = oci;
    }

    /* advance to start of next request */

    oci->bufptr += oci->lenLastReq;

    need_header = FALSE;
#ifdef BIGREQS
    move_header = FALSE;
#endif
    gotnow = oci->bufcnt + oci->buffer - oci->bufptr;
    if (gotnow < sizeof(xReq))
    {
	/* We don't have an entire xReq yet.  Can't tell how big
	 * the request will be until we get the whole xReq.
	 */
	needed = sizeof(xReq);
	need_header = TRUE;
    }
    else
    {
	/* We have a whole xReq.  We can tell how big the whole
	 * request will be unless it is a Big Request.
	 */
	request = (xReq *)oci->bufptr;
	needed = get_req_len(request, client);
#ifdef BIGREQS
	if (!needed && client->big_requests)
	{
	    /* It's a Big Request. */
	    move_header = TRUE;
	    if (gotnow < sizeof(xBigReq))
	    {
		/* Still need more data to tell just how big. */
		needed = sizeof(xBigReq) >> 2; /* needed is in CARD32s now */
		need_header = TRUE;
	    }
	    else
		needed = get_big_req_len(request, client);
	}
#endif
	client->req_len = needed;
	needed <<= 2; /* needed is in bytes now */
    }
    if (gotnow < needed)
    {
	/* Need to read more data, either so that we can get a
	 * complete xReq (if need_header is TRUE), a complete
	 * xBigReq (if move_header is TRUE), or the rest of the
	 * request (if need_header and move_header are both FALSE).
	 */

	oci->lenLastReq = 0;
	if (needed > MAXBUFSIZE)
	{
	    /* request is too big for us to handle */
	    YieldControlDeath();
	    return -1;
	}
	if ((gotnow == 0) ||
	    ((oci->bufptr - oci->buffer + needed) > oci->size))
	{
	    /* no data, or the request is too big to fit in the buffer */

	    if ((gotnow > 0) && (oci->bufptr != oci->buffer))
		/* save the data we've already read */
		memmove(oci->buffer, oci->bufptr, gotnow);
	    if (needed > oci->size)
	    {
		/* make buffer bigger to accomodate request */
		char *ibuf;

		ibuf = (char *)xrealloc(oci->buffer, needed);
		if (!ibuf)
		{
		    YieldControlDeath();
		    return -1;
		}
		oci->size = needed;
		oci->buffer = ibuf;
	    }
	    oci->bufptr = oci->buffer;
	    oci->bufcnt = gotnow;
	}
	/*  XXX this is a workaround.  This function is sometimes called
	 *  after the trans_conn has been freed.  In this case trans_conn
	 *  will be null.  Really ought to restructure things so that we
	 *  never get here in those circumstances.
	 */
	if (!oc->trans_conn)
	{
	    /*  treat as if an error occured on the read, which is what
	     *  used to happen
	     */
	    YieldControlDeath();
	    return -1;
	}
#ifdef LBX
	if (oc->proxy && oc->proxy->compHandle)
	    result = (*oc->proxy->streamOpts.streamCompRead)(fd,
			     (unsigned char *)oci->buffer + oci->bufcnt,
			     oci->size - oci->bufcnt);
	else
#endif
	    result = _XSERVTransRead(oc->trans_conn, oci->buffer + oci->bufcnt,
				     oci->size - oci->bufcnt); 
	if (result <= 0)
	{
	    if ((result < 0) && ETEST(errno))
	    {
#if defined(SVR4) && defined(i386) && !defined(sun)
#if defined(LBX) && 0
		/*
		 * For LBX connections, we can get a valid EWOULDBLOCK
		 * There is probably a better way of distinguishing LBX
		 * connections, but this works. (DHD)
		 */
		extern int LbxRead();
		if (oc->Read == LbxRead)
#else
		if (0)
#endif
#endif
		{
		    YieldControlNoInput();
		    return 0;
		}
	    }
	    YieldControlDeath();
	    return -1;
	}
	oci->bufcnt += result;
	gotnow += result;
	/* free up some space after huge requests */
	if ((oci->size > BUFWATERMARK) &&
	    (oci->bufcnt < BUFSIZE) && (needed < BUFSIZE))
	{
	    char *ibuf;

	    ibuf = (char *)xrealloc(oci->buffer, BUFSIZE);
	    if (ibuf)
	    {
		oci->size = BUFSIZE;
		oci->buffer = ibuf;
		oci->bufptr = ibuf + oci->bufcnt - gotnow;
	    }
	}
	if (need_header && gotnow >= needed)
	{
	    /* We wanted an xReq, now we've gotten it. */
	    request = (xReq *)oci->bufptr;
	    needed = get_req_len(request, client);
#ifdef BIGREQS
	    if (!needed && client->big_requests)
	    {
		move_header = TRUE;
		if (gotnow < sizeof(xBigReq))
		    needed = sizeof(xBigReq) >> 2;
		else
		    needed = get_big_req_len(request, client);
	    }
#endif
	    client->req_len = needed;
	    needed <<= 2;
	}
	if (gotnow < needed)
	{
	    /* Still don't have enough; punt. */
	    YieldControlNoInput();
	    return 0;
	}
    }
    if (needed == 0)
    {
#ifdef BIGREQS
	if (client->big_requests)
	    needed = sizeof(xBigReq);
	else
#endif
	    needed = sizeof(xReq);
    }
    oci->lenLastReq = needed;

    /*
     *  Check to see if client has at least one whole request in the
     *  buffer beyond the request we're returning to the caller.
     *  If there is only a partial request, treat like buffer
     *  is empty so that select() will be called again and other clients
     *  can get into the queue.   
     */

    gotnow -= needed;
    if (gotnow >= sizeof(xReq)) 
    {
	request = (xReq *)(oci->bufptr + needed);
	if (gotnow >= (result = (get_req_len(request, client) << 2))
#ifdef BIGREQS
	    && (result ||
		(client->big_requests &&
		 (gotnow >= sizeof(xBigReq) &&
		  gotnow >= (get_big_req_len(request, client) << 2))))
#endif
	    )
	    FD_SET(fd, &ClientsWithInput);
	else
	{
#ifdef SMART_SCHEDULE
	    if (!SmartScheduleDisable)
		FD_CLR(fd, &ClientsWithInput);
	    else
#endif
		YieldControlNoInput();
	}
    }
    else
    {
	if (!gotnow)
	    AvailableInput = oc;
#ifdef SMART_SCHEDULE
	if (!SmartScheduleDisable)
	    FD_CLR(fd, &ClientsWithInput);
	else
#endif
	    YieldControlNoInput();
    }
#ifdef SMART_SCHEDULE
    if (SmartScheduleDisable)
#endif
    if (++timesThisConnection >= MAX_TIMES_PER)
	YieldControl();
#ifdef BIGREQS
    if (move_header)
    {
	request = (xReq *)oci->bufptr;
	oci->bufptr += (sizeof(xBigReq) - sizeof(xReq));
	*(xReq *)oci->bufptr = *request;
	oci->lenLastReq -= (sizeof(xBigReq) - sizeof(xReq));
	client->req_len -= (sizeof(xBigReq) - sizeof(xReq)) >> 2;
    }
#endif
    client->requestBuffer = (pointer)oci->bufptr;
#ifdef DEBUG_COMMUNICATION
    {
	xReq *req = client->requestBuffer;
	ErrorF("REQUEST: ClientIDX: %i, type: 0x%x data: 0x%x len: %i\n",
	       client->index,req->reqType,req->data,req->length);
    }
#endif
    return needed;
}

/*****************************************************************
 * InsertFakeRequest
 *    Splice a consed up (possibly partial) request in as the next request.
 *
 **********************/

Bool
InsertFakeRequest(ClientPtr client, char *data, int count)
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    ConnectionInputPtr oci = oc->input;
    int fd = oc->fd;
    int gotnow, moveup;

    if (AvailableInput)
    {
	if (AvailableInput != oc)
	{
	    ConnectionInputPtr aci = AvailableInput->input;
	    if (aci->size > BUFWATERMARK)
	    {
		xfree(aci->buffer);
		xfree(aci);
	    }
	    else
	    {
		aci->next = FreeInputs;
		FreeInputs = aci;
	    }
	    AvailableInput->input = (ConnectionInputPtr)NULL;
	}
	AvailableInput = (OsCommPtr)NULL;
    }
    if (!oci)
    {
	if ((oci = FreeInputs))
	    FreeInputs = oci->next;
	else if (!(oci = AllocateInputBuffer()))
	    return FALSE;
	oc->input = oci;
    }
    oci->bufptr += oci->lenLastReq;
    oci->lenLastReq = 0;
    gotnow = oci->bufcnt + oci->buffer - oci->bufptr;
    if ((gotnow + count) > oci->size)
    {
	char *ibuf;

	ibuf = (char *)xrealloc(oci->buffer, gotnow + count);
	if (!ibuf)
	    return(FALSE);
	oci->size = gotnow + count;
	oci->buffer = ibuf;
	oci->bufptr = ibuf + oci->bufcnt - gotnow;
    }
    moveup = count - (oci->bufptr - oci->buffer);
    if (moveup > 0)
    {
	if (gotnow > 0)
	    memmove(oci->bufptr + moveup, oci->bufptr, gotnow);
	oci->bufptr += moveup;
	oci->bufcnt += moveup;
    }
    memmove(oci->bufptr - count, data, count);
    oci->bufptr -= count;
    gotnow += count;
    if ((gotnow >= sizeof(xReq)) &&
	(gotnow >= (int)(get_req_len((xReq *)oci->bufptr, client) << 2)))
	FD_SET(fd, &ClientsWithInput);
    else
	YieldControlNoInput();
    return(TRUE);
}

/*****************************************************************
 * ResetRequestFromClient
 *    Reset to reexecute the current request, and yield.
 *
 **********************/

void
ResetCurrentRequest(ClientPtr client)
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    register ConnectionInputPtr oci = oc->input;
    int fd = oc->fd;
    register xReq *request;
    int gotnow, needed;
#ifdef LBX
    LbxClientPtr lbxClient = LbxClient(client);

    if (lbxClient) {
	LbxSetForBlock(lbxClient);
	if (!oci) {
	    AppendFakeRequest(client,
			      client->requestBuffer, client->req_len << 2);
	    return;
	}
    }
#endif
    if (AvailableInput == oc)
	AvailableInput = (OsCommPtr)NULL;
    oci->lenLastReq = 0;
    gotnow = oci->bufcnt + oci->buffer - oci->bufptr;
    if (gotnow < sizeof(xReq))
    {
	YieldControlNoInput();
    }
    else
    {
	request = (xReq *)oci->bufptr;
	needed = get_req_len(request, client);
#ifdef BIGREQS
	if (!needed && client->big_requests)
	{
	    oci->bufptr -= sizeof(xBigReq) - sizeof(xReq);
	    *(xReq *)oci->bufptr = *request;
	    ((xBigReq *)oci->bufptr)->length = client->req_len;
	    if (client->swapped)
	    {
		char n;
		swapl(&((xBigReq *)oci->bufptr)->length, n);
	    }
	}
#endif
	if (gotnow >= (needed << 2))
	{
	    if (FD_ISSET(fd, &AllClients))
	    {
		FD_SET(fd, &ClientsWithInput);
	    }
	    else
	    {
		FD_SET(fd, &IgnoredClientsWithInput);
	    }
	    YieldControl();
	}
	else
	    YieldControlNoInput();
    }
}



/*****************************************************************
 *  PeekNextRequest and SkipRequests were implemented to support DBE 
 *  idioms, but can certainly be used outside of DBE.  There are two 
 *  related macros in os.h, ReqLen and CastxReq.  See the porting 
 *  layer document for more details.
 *
 **********************/


/*****************************************************************
 *  PeekNextRequest
 *      lets you look ahead at the unexecuted requests in a 
 *      client's request buffer.
 *
 *      Note: this implementation of PeekNextRequest ignores the
 *      readmore parameter.
 *
 **********************/

xReqPtr
PeekNextRequest(
    xReqPtr req,	/* request we're starting from */
    ClientPtr client,	/* client whose requests we're skipping */
    Bool readmore)	/* attempt to read more if next request isn't there? */
{
    register ConnectionInputPtr oci = ((OsCommPtr)client->osPrivate)->input;
    xReqPtr pnextreq;
    int needed, gotnow, reqlen;

    if (!oci) return NULL;

    if (!req)
    {
	/* caller wants the request after the one currently being executed */
	pnextreq = (xReqPtr)
	    (((CARD32 *)client->requestBuffer) + client->req_len);
    }
    else
    {
	/* caller wants the request after the one specified by req */
	reqlen = get_req_len(req, client);
#ifdef BIGREQS
	if (!reqlen) reqlen = get_big_req_len(req, client);
#endif
	pnextreq = (xReqPtr)(((char *)req) + (reqlen << 2));
    }

    /* see how much of the next request we have available */

    gotnow = oci->bufcnt - (((char *)pnextreq) - oci->buffer);

    if (gotnow < sizeof(xReq))
	return NULL;

    needed = get_req_len(pnextreq, client) << 2;
#ifdef BIGREQS
    if (!needed)
    {
	/* it's a big request */
	if (gotnow < sizeof(xBigReq))
	    return NULL;
	needed = get_big_req_len(pnextreq, client) << 2;
    }
#endif

    /* if we have less than we need, return NULL */

    return (gotnow < needed) ? NULL : pnextreq;
}

/*****************************************************************
 *  SkipRequests 
 *      lets you skip over some of the requests in a client's
 *      request buffer.  Presumably the caller has used PeekNextRequest
 *      to examine the requests being skipped and has performed whatever 
 *      actions they dictate.
 *
 **********************/

CallbackListPtr SkippedRequestsCallback = NULL;

void
SkipRequests(
    xReqPtr req,	/* last request being skipped */
    ClientPtr client,   /* client whose requests we're skipping */
    int numskipped)	/* how many requests we're skipping */
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    register ConnectionInputPtr oci = oc->input;
    int reqlen;

    /* see if anyone wants to snoop the skipped requests */

    if (SkippedRequestsCallback)
    {
	SkippedRequestInfoRec skipinfo;
	skipinfo.req = req;
	skipinfo.client = client;
	skipinfo.numskipped = numskipped;
	CallCallbacks(&SkippedRequestsCallback, &skipinfo);
    }

    /* adjust the sequence number */
    client->sequence += numskipped;

    /* twiddle the oci to skip over the requests */

    reqlen = get_req_len(req, client);
#ifdef BIGREQS
    if (!reqlen) reqlen = get_big_req_len(req, client);
#endif
    reqlen <<= 2;
    oci->bufptr = (char *)req;
    oci->lenLastReq = reqlen;

    /* see if any requests left in the buffer */

    if ( ((char *)req + reqlen) == (oci->buffer + oci->bufcnt) )
    {
	/* no requests; mark input buffer as available and client
	 * as having no input
	 */
	int fd = oc->fd;
	AvailableInput = oc;
	YieldControlNoInput();
    }
}


    /* lookup table for adding padding bytes to data that is read from
    	or written to the X socket.  */
static int padlength[4] = {0, 3, 2, 1};

 /********************
 * FlushAllOutput()
 *    Flush all clients with output.  However, if some client still
 *    has input in the queue (more requests), then don't flush.  This
 *    will prevent the output queue from being flushed every time around
 *    the round robin queue.  Now, some say that it SHOULD be flushed
 *    every time around, but...
 *
 **********************/

void
FlushAllOutput(void)
{
    register int index, base;
    register fd_mask mask; /* raphael */
    OsCommPtr oc;
    register ClientPtr client;
    Bool newoutput = NewOutputPending;
#if defined(WIN32)
    fd_set newOutputPending;
#endif

    if (FlushCallback)
	CallCallbacks(&FlushCallback, NULL);

    if (!newoutput)
	return;

    /*
     * It may be that some client still has critical output pending,
     * but he is not yet ready to receive it anyway, so we will
     * simply wait for the select to tell us when he's ready to receive.
     */
    CriticalOutputPending = FALSE;
    NewOutputPending = FALSE;

#ifndef WIN32
    for (base = 0; base < howmany(XFD_SETSIZE, NFDBITS); base++)
    {
	mask = OutputPending.fds_bits[ base ];
	OutputPending.fds_bits[ base ] = 0;
	while (mask)
	{
	    index = ffs(mask) - 1;
	    mask &= ~lowbit(mask);
	    if ((index = ConnectionTranslation[(base * (sizeof(fd_mask)*8)) + index]) == 0)
		continue;
	    client = clients[index];
	    if (client->clientGone)
		continue;
	    oc = (OsCommPtr)client->osPrivate;
	    if (
#ifdef LBX
		!oc->proxy &&
#endif
		FD_ISSET(oc->fd, &ClientsWithInput))
	    {
		FD_SET(oc->fd, &OutputPending); /* set the bit again */
		NewOutputPending = TRUE;
	    }
	    else
		(void)FlushClient(client, oc, (char *)NULL, 0);
	}
    }
#else  /* WIN32 */
    FD_ZERO(&newOutputPending);
    for (base = 0; base < XFD_SETCOUNT(&OutputPending); base++)
    {
	    index = XFD_FD(&OutputPending, base);
	    if ((index = GetConnectionTranslation(index)) == 0)
		continue;
	    client = clients[index];
	    if (client->clientGone)
		continue;
	    oc = (OsCommPtr)client->osPrivate;
	    if (
#ifdef LBX
		!oc->proxy &&
#endif
		FD_ISSET(oc->fd, &ClientsWithInput))
	    {
		FD_SET(oc->fd, &newOutputPending); /* set the bit again */
		NewOutputPending = TRUE;
	    }
	    else
		(void)FlushClient(client, oc, (char *)NULL, 0);
    }
    XFD_COPYSET(&newOutputPending, &OutputPending);
#endif /* WIN32 */
}

void
FlushIfCriticalOutputPending(void)
{
    if (CriticalOutputPending)
	FlushAllOutput();
}

void
SetCriticalOutputPending(void)
{
    CriticalOutputPending = TRUE;
}

/*****************
 * WriteToClient
 *    Copies buf into ClientPtr.buf if it fits (with padding), else
 *    flushes ClientPtr.buf and buf to client.  As of this writing,
 *    every use of WriteToClient is cast to void, and the result
 *    is ignored.  Potentially, this could be used by requests
 *    that are sending several chunks of data and want to break
 *    out of a loop on error.  Thus, we will leave the type of
 *    this routine as int.
 *****************/

int
WriteToClient (ClientPtr who, int count, char *buf)
{
    OsCommPtr oc = (OsCommPtr)who->osPrivate;
    ConnectionOutputPtr oco = oc->output;
    int padBytes;
#ifdef DEBUG_COMMUNICATION
    Bool multicount = FALSE;
#endif
    if (!count)
	return(0);
#ifdef DEBUG_COMMUNICATION
    {
	char info[128];
	xError *err;
	xGenericReply *rep;
	xEvent *ev;
	
	if (!who->replyBytesRemaining) {
	    switch(buf[0]) {
	    case X_Reply:
		rep = (xGenericReply*)buf;
		if (rep->sequenceNumber == who->sequence) {
		    snprintf(info,127,"Xreply: type: 0x%x data: 0x%x "
			     "len: %i seq#: 0x%x", rep->type, rep->data1,
			     rep->length, rep->sequenceNumber);
		    multicount = TRUE;
		}
		break;
	    case X_Error:
		err = (xError*)buf;
		snprintf(info,127,"Xerror: Code: 0x%x resID: 0x%x maj: 0x%x "
			 "min: %x", err->errorCode,err->resourceID,
			 err->minorCode,err->majorCode);
		break;
	    default:
		if ((buf[0] & 0x7f) == KeymapNotify) 
		    snprintf(info,127,"KeymapNotifyEvent: %i",buf[0]);
		else {
		    ev = (xEvent*)buf;
		    snprintf(info,127,"XEvent: type: 0x%x detail: 0x%x "
			     "seq#: 0x%x",  ev->u.u.type, ev->u.u.detail,
			     ev->u.u.sequenceNumber);
		}
	    }
	    ErrorF("REPLY: ClientIDX: %i %s\n",who->index, info);
	} else
	    multicount = TRUE;
    }
#endif

    if (!oco)
    {
	if ((oco = FreeOutputs))
	{
	    FreeOutputs = oco->next;
	}
	else if (!(oco = AllocateOutputBuffer()))
	{
	    if (oc->trans_conn) {
		_XSERVTransDisconnect(oc->trans_conn);
		_XSERVTransClose(oc->trans_conn);
		oc->trans_conn = NULL;
	    }
	    MarkClientException(who);
	    return -1;
	}
	oc->output = oco;
    }

    padBytes = padlength[count & 3];

    if(ReplyCallback)
    {
        ReplyInfoRec replyinfo;

	replyinfo.client = who;
	replyinfo.replyData = buf;
	replyinfo.dataLenBytes = count + padBytes;
	if (who->replyBytesRemaining)
	{ /* still sending data of an earlier reply */
	    who->replyBytesRemaining -= count + padBytes;
	    replyinfo.startOfReply = FALSE;
	    replyinfo.bytesRemaining = who->replyBytesRemaining;
	    CallCallbacks((&ReplyCallback), (pointer)&replyinfo);
	}
	else if (who->clientState == ClientStateRunning
		 && buf[0] == X_Reply)
        { /* start of new reply */
	    CARD32 replylen;
	    unsigned long bytesleft;
	    char n;

	    replylen = ((xGenericReply *)buf)->length;
	    if (who->swapped)
		swapl(&replylen, n);
	    bytesleft = (replylen * 4) + SIZEOF(xReply) - count - padBytes;
	    replyinfo.startOfReply = TRUE;
	    replyinfo.bytesRemaining = who->replyBytesRemaining = bytesleft;
	    CallCallbacks((&ReplyCallback), (pointer)&replyinfo);
	} 	                      
    }
#ifdef DEBUG_COMMUNICATION
    else if (multicount) {
	if (who->replyBytesRemaining) {
	    who->replyBytesRemaining -= (count + padBytes);
	} else {
	    CARD32 replylen;
	    replylen = ((xGenericReply *)buf)->length;
	    who->replyBytesRemaining =
		(replylen * 4) + SIZEOF(xReply) - count - padBytes;
	}
    }
#endif
    if (oco->count + count + padBytes > oco->size)
    {
	FD_CLR(oc->fd, &OutputPending);
	if(!XFD_ANYSET(&OutputPending)) {
	  CriticalOutputPending = FALSE;
	  NewOutputPending = FALSE;
	}
	return FlushClient(who, oc, buf, count);
    }

    NewOutputPending = TRUE;
    FD_SET(oc->fd, &OutputPending);
    memmove((char *)oco->buf + oco->count, buf, count);
    oco->count += count + padBytes;
    return(count);
}

 /********************
 * FlushClient()
 *    If the client isn't keeping up with us, then we try to continue
 *    buffering the data and set the apropriate bit in ClientsWritable
 *    (which is used by WaitFor in the select).  If the connection yields
 *    a permanent error, or we can't allocate any more space, we then
 *    close the connection.
 *
 **********************/

#ifdef LBX
#ifdef LBX_NEED_OLD_SYMBOL_FOR_LOADABLES
#undef FlushClient
int
FlushClient(ClientPtr who, OsCommPtr oc, char *extraBuf, int extraCount)
{
    return (*oc->Flush)(who, oc, extraBuf, extraCount);
}
#endif
int
StandardFlushClient(ClientPtr who, OsCommPtr oc, 
    char *extraBuf, int extraCount)
#else
int
FlushClient(ClientPtr who, OsCommPtr oc, char *extraBuf, int extraCount)
#endif
{
    ConnectionOutputPtr oco = oc->output;
    int connection = oc->fd;
    XtransConnInfo trans_conn = oc->trans_conn;
    struct iovec iov[3];
    static char padBuffer[3];
    long written;
    long padsize;
    long notWritten;
    long todo;

    if (!oco)
	return 0;
    written = 0;
    padsize = padlength[extraCount & 3];
    notWritten = oco->count + extraCount + padsize;
    todo = notWritten;
    while (notWritten) {
	long before = written;	/* amount of whole thing written */
	long remain = todo;	/* amount to try this time, <= notWritten */
	int i = 0;
	long len;
	
	/* You could be very general here and have "in" and "out" iovecs
	 * and write a loop without using a macro, but what the heck.  This
	 * translates to:
	 *
	 *     how much of this piece is new?
	 *     if more new then we are trying this time, clamp
	 *     if nothing new
	 *         then bump down amount already written, for next piece
	 *         else put new stuff in iovec, will need all of next piece
	 *
	 * Note that todo had better be at least 1 or else we'll end up
	 * writing 0 iovecs.
	 */
#define InsertIOV(pointer, length) \
	len = (length) - before; \
	if (len > remain) \
	    len = remain; \
	if (len <= 0) { \
	    before = (-len); \
	} else { \
	    iov[i].iov_len = len; \
	    iov[i].iov_base = (pointer) + before; \
	    i++; \
	    remain -= len; \
	    before = 0; \
	}

	InsertIOV ((char *)oco->buf, oco->count)
	InsertIOV (extraBuf, extraCount)
	InsertIOV (padBuffer, padsize)

	errno = 0;
	if (trans_conn && (len = _XSERVTransWritev(trans_conn, iov, i)) >= 0)
	{
	    written += len;
	    notWritten -= len;
	    todo = notWritten;
	}
	else if (ETEST(errno)
#ifdef SUNSYSV /* check for another brain-damaged OS bug */
		 || (errno == 0)
#endif
#ifdef EMSGSIZE /* check for another brain-damaged OS bug */
		 || ((errno == EMSGSIZE) && (todo == 1))
#endif
		)
	{
	    /* If we've arrived here, then the client is stuffed to the gills
	       and not ready to accept more.  Make a note of it and buffer
	       the rest. */
	    FD_SET(connection, &ClientsWriteBlocked);
	    AnyClientsWriteBlocked = TRUE;

	    if (written < oco->count)
	    {
		if (written > 0)
		{
		    oco->count -= written;
		    memmove((char *)oco->buf,
			    (char *)oco->buf + written,
			  oco->count);
		    written = 0;
		}
	    }
	    else
	    {
		written -= oco->count;
		oco->count = 0;
	    }

	    if (notWritten > oco->size)
	    {
		unsigned char *obuf;

		obuf = (unsigned char *)xrealloc(oco->buf,
						 notWritten + BUFSIZE);
		if (!obuf)
		{
		    _XSERVTransDisconnect(oc->trans_conn);
		    _XSERVTransClose(oc->trans_conn);
		    oc->trans_conn = NULL;
		    MarkClientException(who);
		    oco->count = 0;
		    return(-1);
		}
		oco->size = notWritten + BUFSIZE;
		oco->buf = obuf;
	    }

	    /* If the amount written extended into the padBuffer, then the
	       difference "extraCount - written" may be less than 0 */
	    if ((len = extraCount - written) > 0)
		memmove ((char *)oco->buf + oco->count,
			 extraBuf + written,
		       len);

	    oco->count = notWritten; /* this will include the pad */
	    /* return only the amount explicitly requested */
	    return extraCount;
	}
#ifdef EMSGSIZE /* check for another brain-damaged OS bug */
	else if (errno == EMSGSIZE)
	{
	    todo >>= 1;
	}
#endif
	else
	{
	    if (oc->trans_conn)
	    {
		_XSERVTransDisconnect(oc->trans_conn);
		_XSERVTransClose(oc->trans_conn);
		oc->trans_conn = NULL;
	    }
	    MarkClientException(who);
	    oco->count = 0;
	    return(-1);
	}
    }

    /* everything was flushed out */
    oco->count = 0;
    /* check to see if this client was write blocked */
    if (AnyClientsWriteBlocked)
    {
	FD_CLR(oc->fd, &ClientsWriteBlocked);
 	if (! XFD_ANYSET(&ClientsWriteBlocked))
	    AnyClientsWriteBlocked = FALSE;
    }
    if (oco->size > BUFWATERMARK)
    {
	xfree(oco->buf);
	xfree(oco);
    }
    else
    {
	oco->next = FreeOutputs;
	FreeOutputs = oco;
    }
    oc->output = (ConnectionOutputPtr)NULL;
    return extraCount; /* return only the amount explicitly requested */
}

ConnectionInputPtr
AllocateInputBuffer(void)
{
    ConnectionInputPtr oci;

    oci = (ConnectionInputPtr)xalloc(sizeof(ConnectionInput));
    if (!oci)
	return (ConnectionInputPtr)NULL;
    oci->buffer = (char *)xalloc(BUFSIZE);
    if (!oci->buffer)
    {
	xfree(oci);
	return (ConnectionInputPtr)NULL;
    }
    oci->size = BUFSIZE;
    oci->bufptr = oci->buffer;
    oci->bufcnt = 0;
    oci->lenLastReq = 0;
    return oci;
}

ConnectionOutputPtr
AllocateOutputBuffer(void)
{
    ConnectionOutputPtr oco;

    oco = (ConnectionOutputPtr)xalloc(sizeof(ConnectionOutput));
    if (!oco)
	return (ConnectionOutputPtr)NULL;
    oco->buf = (unsigned char *) xalloc(BUFSIZE);
    if (!oco->buf)
    {
	xfree(oco);
	return (ConnectionOutputPtr)NULL;
    }
    oco->size = BUFSIZE;
    oco->count = 0;
#ifdef LBX
    oco->nocompress = FALSE;
#endif
    return oco;
}

void
FreeOsBuffers(OsCommPtr oc)
{
    ConnectionInputPtr oci;
    ConnectionOutputPtr oco;

    if (AvailableInput == oc)
	AvailableInput = (OsCommPtr)NULL;
    if ((oci = oc->input))
    {
	if (FreeInputs)
	{
	    xfree(oci->buffer);
	    xfree(oci);
	}
	else
	{
	    FreeInputs = oci;
	    oci->next = (ConnectionInputPtr)NULL;
	    oci->bufptr = oci->buffer;
	    oci->bufcnt = 0;
	    oci->lenLastReq = 0;
	}
    }
    if ((oco = oc->output))
    {
	if (FreeOutputs)
	{
	    xfree(oco->buf);
	    xfree(oco);
	}
	else
	{
	    FreeOutputs = oco;
	    oco->next = (ConnectionOutputPtr)NULL;
	    oco->count = 0;
	}
    }
#ifdef LBX
    if ((oci = oc->largereq)) {
	xfree(oci->buffer);
	xfree(oci);
    }
#endif
}

void
ResetOsBuffers(void)
{
    ConnectionInputPtr oci;
    ConnectionOutputPtr oco;

    while ((oci = FreeInputs))
    {
	FreeInputs = oci->next;
	xfree(oci->buffer);
	xfree(oci);
    }
    while ((oco = FreeOutputs))
    {
	FreeOutputs = oco->next;
	xfree(oco->buf);
	xfree(oco);
    }
}
