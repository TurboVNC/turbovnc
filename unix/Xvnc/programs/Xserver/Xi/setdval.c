/* $Xorg: setdval.c,v 1.4 2001/02/09 02:04:34 xorgcvs Exp $ */

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
/* $XFree86: xc/programs/Xserver/Xi/setdval.c,v 3.3 2001/12/14 19:58:59 dawes Exp $ */

/***********************************************************************
 *
 * Request to change the mode of an extension input device.
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
#include "extnsionst.h"
#include "extinit.h"			/* LookupDeviceIntRec */
#include "exglobals.h"

#include "setdval.h"

/***********************************************************************
 *
 * Handle a request from a client with a different byte order.
 *
 */

int
SProcXSetDeviceValuators(client)
    register ClientPtr client;
    {
    register char n;

    REQUEST(xSetDeviceValuatorsReq);
    swaps(&stuff->length, n);
    return(ProcXSetDeviceValuators(client));
    }

/***********************************************************************
 *
 * This procedure sets the value of valuators on an extension input device.
 *
 */

int
ProcXSetDeviceValuators(client)
    register ClientPtr client;
    {
    DeviceIntPtr dev;
    xSetDeviceValuatorsReply	rep;

    REQUEST(xSetDeviceValuatorsReq);
    REQUEST_AT_LEAST_SIZE(xSetDeviceValuatorsReq);

    rep.repType = X_Reply;
    rep.RepType = X_SetDeviceValuators;
    rep.length = 0;
    rep.status = Success;
    rep.sequenceNumber = client->sequence;

    if (stuff->length !=(sizeof(xSetDeviceValuatorsReq)>>2) + 
	stuff->num_valuators)
	{
	SendErrorToClient (client, IReqCode, X_SetDeviceValuators, 0, 
		BadLength);
	return Success;
	}
    dev = LookupDeviceIntRec (stuff->deviceid);
    if (dev == NULL)
	{
	SendErrorToClient (client, IReqCode, X_SetDeviceValuators, 0, 
	    BadDevice);
	return Success;
	}
    if (dev->valuator == NULL)
	{
	SendErrorToClient(client, IReqCode, X_SetDeviceValuators, 0, 
		BadMatch);
	return Success;
	}

    if (stuff->first_valuator + stuff->num_valuators > dev->valuator->numAxes)
	{
	SendErrorToClient(client, IReqCode, X_SetDeviceValuators, 0, 
		BadValue);
	return Success;
	}

    if ((dev->grab) && !SameClient(dev->grab, client))
	rep.status = AlreadyGrabbed;
    else
	rep.status = SetDeviceValuators (client, dev, (int *) &stuff[1],
	    stuff->first_valuator, stuff->num_valuators);

    if (rep.status != Success && rep.status != AlreadyGrabbed)
	SendErrorToClient(client, IReqCode, X_SetDeviceValuators, 0, 
	    rep.status);
    else
	WriteReplyToClient (client, sizeof (xSetDeviceValuatorsReply), &rep);

    return Success;
    }

/***********************************************************************
 *
 * This procedure writes the reply for the XSetDeviceValuators function,
 * if the client and server have a different byte ordering.
 *
 */

void
SRepXSetDeviceValuators (client, size, rep)
    ClientPtr	client;
    int		size;
    xSetDeviceValuatorsReply	*rep;
    {
    register char n;

    swaps(&rep->sequenceNumber, n);
    swapl(&rep->length, n);
    WriteToClient(client, size, (char *)rep);
    }
