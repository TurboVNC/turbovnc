/* $Xorg: chgptr.c,v 1.4 2001/02/09 02:04:33 xorgcvs Exp $ */

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
/* $XFree86: xc/programs/Xserver/Xi/chgptr.c,v 3.7 2001/12/14 19:58:55 dawes Exp $ */

/***********************************************************************
 *
 * Extension function to change the pointer device.
 *
 */

#define	 NEED_EVENTS
#define	 NEED_REPLIES
#include "X.h"				/* for inputstr.h    */
#include "Xproto.h"			/* Request macro     */
#include "inputstr.h"			/* DeviceIntPtr	     */
#include "XI.h"
#include "XIproto.h"
#include "XIstubs.h"
#include "windowstr.h"			/* window structure  */
#include "scrnintstr.h"			/* screen structure  */

#include "extnsionst.h"
#include "extinit.h"			/* LookupDeviceIntRec */

#include "dixevents.h"
#include "exevents.h"
#include "exglobals.h"

#include "chgptr.h"

/***********************************************************************
 *
 * This procedure is invoked to swap the request bytes if the server and
 * client have a different byte order.
 *
 */

int
SProcXChangePointerDevice(client)
    register ClientPtr client;
    {
    register char n;

    REQUEST(xChangePointerDeviceReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xChangePointerDeviceReq);
    return(ProcXChangePointerDevice(client));
    }

/***********************************************************************
 *
 * This procedure changes the device used as the X pointer.
 *
 */

int
ProcXChangePointerDevice (client)
    register ClientPtr client;
    {
    DeviceIntPtr 	xptr = inputInfo.pointer;
    DeviceIntPtr 	dev;
    ValuatorClassPtr 	v;
    xChangePointerDeviceReply	rep;
    changeDeviceNotify	ev;

    REQUEST(xChangePointerDeviceReq);
    REQUEST_SIZE_MATCH(xChangePointerDeviceReq);

    rep.repType = X_Reply;
    rep.RepType = X_ChangePointerDevice;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    dev = LookupDeviceIntRec (stuff->deviceid);
    if (dev == NULL)
	{
	rep.status = -1;
	SendErrorToClient(client, IReqCode, X_ChangePointerDevice, 0, 
	    BadDevice);
	return Success;
	}

    v = dev->valuator;
    if (v == NULL || v->numAxes < 2 || 
	stuff->xaxis >= v->numAxes ||
	stuff->yaxis >= v->numAxes)
	{
	rep.status = -1;
	SendErrorToClient(client, IReqCode, X_ChangePointerDevice, 0, BadMatch);
	return Success;
	}

    if (((dev->grab) && !SameClient(dev->grab, client)) ||
        ((xptr->grab) && !SameClient(xptr->grab, client)))
	rep.status = AlreadyGrabbed;
    else if ((dev->sync.frozen &&
	     dev->sync.other && !SameClient(dev->sync.other, client)) ||
	     (xptr->sync.frozen &&
	      xptr->sync.other && !SameClient(xptr->sync.other, client)))
	rep.status = GrabFrozen;
    else
	{
	if (ChangePointerDevice (
	    xptr, dev, stuff->xaxis, stuff->yaxis) != Success)
	    {
	    SendErrorToClient(client, IReqCode, X_ChangePointerDevice, 0, 
		BadDevice);
	    return Success;
	    }
	if (dev->focus)
	    DeleteFocusClassDeviceStruct(dev);
	if (!dev->button)
	    InitButtonClassDeviceStruct (dev, 0, NULL);
	if (!dev->ptrfeed)
	   InitPtrFeedbackClassDeviceStruct(dev, (PtrCtrlProcPtr)NoopDDA);
	RegisterOtherDevice (xptr);
	RegisterPointerDevice (dev);

	ev.type = ChangeDeviceNotify;
	ev.deviceid = stuff->deviceid;
	ev.time = currentTime.milliseconds;
	ev.request = NewPointer;

	SendEventToAllWindows (dev, ChangeDeviceNotifyMask, (xEvent *)&ev, 1);
	SendMappingNotify (MappingPointer, 0, 0, client);

	rep.status = 0;
	}

    WriteReplyToClient (client, sizeof (xChangePointerDeviceReply), 
	&rep);
    return Success;
    }

void
DeleteFocusClassDeviceStruct(dev)
    DeviceIntPtr dev;
    {
    xfree(dev->focus->trace);
    xfree(dev->focus);
    dev->focus = NULL;
    }

/***********************************************************************
 *
 * Send an event to interested clients in all windows on all screens.
 *
 */

void
SendEventToAllWindows (dev, mask, ev, count)
    DeviceIntPtr dev;
    Mask mask;
    xEvent *ev;
    int count;
    {
    int i;
    WindowPtr pWin, p1;

    for (i=0; i<screenInfo.numScreens; i++)
	{
	pWin = WindowTable[i];
	(void)DeliverEventsToWindow(pWin, ev, count, mask, NullGrab, dev->id);
	p1 = pWin->firstChild;
	FindInterestedChildren (dev, p1, mask, ev, count);
	}
    }

/***********************************************************************
 *
 * Walk through the window tree, finding all clients that want to know
 * about the ChangeDeviceNotify Event.
 *
 */

void
FindInterestedChildren (dev, p1, mask, ev, count)
    DeviceIntPtr	dev;
    WindowPtr 		p1;
    Mask		mask;
    xEvent		*ev;
    int			count;
    {
    WindowPtr p2;

    while (p1)
        {
        p2 = p1->firstChild;
	(void)DeliverEventsToWindow(p1, ev, count, mask, NullGrab, dev->id);
	FindInterestedChildren (dev, p2, mask, ev, count);
	p1 = p1->nextSib;
        }
    }

/***********************************************************************
 *
 * This procedure writes the reply for the XChangePointerDevice 
 * function, if the client and server have a different byte ordering.
 *
 */

void
SRepXChangePointerDevice (client, size, rep)
    ClientPtr	client;
    int		size;
    xChangePointerDeviceReply	*rep;
    {
    register char n;

    swaps(&rep->sequenceNumber, n);
    swapl(&rep->length, n);
    WriteToClient(client, size, (char *)rep);
    }
