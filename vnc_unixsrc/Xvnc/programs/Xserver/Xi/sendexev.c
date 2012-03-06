/* $Xorg: sendexev.c,v 1.4 2001/02/09 02:04:34 xorgcvs Exp $ */

/************************************************************

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

Copyright 1989 by Hewlett-Packard Company, Palo Alto, California.

			All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Hewlett-Packard not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
HEWLETT-PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/
/* $XFree86: xc/programs/Xserver/Xi/sendexev.c,v 3.3 2001/12/14 19:58:58 dawes Exp $ */

/***********************************************************************
 *
 * Request to send an extension event.
 *
 */

#define EXTENSION_EVENT_BASE  64
#define	 NEED_EVENTS
#define	 NEED_REPLIES
#include "X.h"				/* for inputstr.h    */
#include "Xproto.h"			/* Request macro     */
#include "inputstr.h"			/* DeviceIntPtr	     */
#include "windowstr.h"			/* Window      	     */
#include "XI.h"
#include "XIproto.h"
#include "extnsionst.h"
#include "extinit.h"			/* LookupDeviceIntRec */
#include "exevents.h"
#include "exglobals.h"

#include "grabdev.h"
#include "sendexev.h"

extern int 		lastEvent; 		/* Defined in extension.c */

/***********************************************************************
 *
 * Handle requests from clients with a different byte order than us.
 *
 */

int
SProcXSendExtensionEvent(client)
    register ClientPtr client;
    {
    register char n;
    register long *p;
    register int i;
    xEvent eventT;
    xEvent *eventP;
    EventSwapPtr proc;

    REQUEST(xSendExtensionEventReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xSendExtensionEventReq);
    swapl(&stuff->destination, n);
    swaps(&stuff->count, n);
    eventP = (xEvent *) &stuff[1];
    for (i=0; i<stuff->num_events; i++,eventP++)
        {
	proc = EventSwapVector[eventP->u.u.type & 0177];
 	if (proc == NotImplemented)   /* no swapping proc; invalid event type? */
	    return (BadValue);
	(*proc)(eventP, &eventT);
	*eventP = eventT;
	}

    p = (long *) (((xEvent *) &stuff[1]) + stuff->num_events);
    for (i=0; i<stuff->count; i++)
        {
        swapl(p, n);
	p++;
        }
    return(ProcXSendExtensionEvent(client));
    }

/***********************************************************************
 *
 * Send an event to some client, as if it had come from an extension input 
 * device.
 *
 */

int
ProcXSendExtensionEvent (client)
    register ClientPtr client;
    {
    int			ret;
    DeviceIntPtr	dev;
    xEvent		*first;
    XEventClass		*list;
    struct tmask	tmp[EMASKSIZE];

    REQUEST(xSendExtensionEventReq);
    REQUEST_AT_LEAST_SIZE(xSendExtensionEventReq);

    if (stuff->length !=(sizeof(xSendExtensionEventReq)>>2) + stuff->count +
	(stuff->num_events * (sizeof (xEvent) >> 2)))
	{
	SendErrorToClient (client, IReqCode, X_SendExtensionEvent, 0, 
		BadLength);
	return Success;
	}

    dev = LookupDeviceIntRec (stuff->deviceid);
    if (dev == NULL)
	{
	SendErrorToClient(client, IReqCode, X_SendExtensionEvent, 0, 
		BadDevice);
	return Success;
	}

    /* The client's event type must be one defined by an extension. */

    first = ((xEvent *) &stuff[1]);
    if ( ! ((EXTENSION_EVENT_BASE  <= first->u.u.type) &&
	(first->u.u.type < lastEvent)) )
	{
	client->errorValue = first->u.u.type;
	SendErrorToClient(client, IReqCode, X_SendExtensionEvent, 0, 
		BadValue);
	return Success;
	}

    list = (XEventClass *) (first + stuff->num_events);
    if ((ret = CreateMaskFromList (client, list, stuff->count, tmp, dev, 
	X_SendExtensionEvent)) != Success)
	return Success;

    ret =  (SendEvent (client, dev, stuff->destination,
	stuff->propagate, (xEvent *)&stuff[1], tmp[stuff->deviceid].mask, 
	stuff->num_events));

    if (ret != Success)
	SendErrorToClient(client, IReqCode, X_SendExtensionEvent, 0, ret);

    return Success;
    }
