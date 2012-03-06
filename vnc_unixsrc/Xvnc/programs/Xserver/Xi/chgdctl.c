/* $Xorg: chgdctl.c,v 1.4 2001/02/09 02:04:33 xorgcvs Exp $ */

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
/* $XFree86: xc/programs/Xserver/Xi/chgdctl.c,v 3.4 2001/12/14 19:58:54 dawes Exp $ */

/********************************************************************
 *
 *  Change Device control attributes for an extension device.
 *
 */

#define	 NEED_EVENTS			/* for inputstr.h    */
#define	 NEED_REPLIES
#include "X.h"				/* for inputstr.h    */
#include "Xproto.h"			/* Request macro     */
#include "inputstr.h"			/* DeviceIntPtr	     */
#include "XI.h"
#include "XIproto.h"			/* control constants */
#include "XIstubs.h"

#include "extnsionst.h"
#include "extinit.h"			/* LookupDeviceIntRec */
#include "exglobals.h"

#include "chgdctl.h"

/***********************************************************************
 *
 * This procedure changes the control attributes for an extension device,
 * for clients on machines with a different byte ordering than the server.
 *
 */

int
SProcXChangeDeviceControl(client)
    register ClientPtr client;
    {
    register char n;

    REQUEST(xChangeDeviceControlReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xChangeDeviceControlReq);
    swaps(&stuff->control, n);
    return(ProcXChangeDeviceControl(client));
    }

/***********************************************************************
 *
 * Change the control attributes.
 *
 */

int
ProcXChangeDeviceControl(client)
    ClientPtr client;
    {
    unsigned len;
    int i, status;
    DeviceIntPtr dev;
    xDeviceResolutionCtl *r;
    xChangeDeviceControlReply rep;
    AxisInfoPtr a;
    CARD32 *resolution;

    REQUEST(xChangeDeviceControlReq);
    REQUEST_AT_LEAST_SIZE(xChangeDeviceControlReq);

    len = stuff->length - (sizeof(xChangeDeviceControlReq) >>2);
    dev = LookupDeviceIntRec (stuff->deviceid);
    if (dev == NULL)
	{
	SendErrorToClient (client, IReqCode, X_ChangeDeviceControl, 0, 
		BadDevice);
	return Success;
	}

    rep.repType = X_Reply;
    rep.RepType = X_ChangeDeviceControl;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    switch (stuff->control) 
	{
	case DEVICE_RESOLUTION:
    	    r = (xDeviceResolutionCtl *) &stuff[1];
	    if ((len < (sizeof(xDeviceResolutionCtl)>>2)) ||
	        (len != (sizeof(xDeviceResolutionCtl)>>2) +
		 r->num_valuators))
		{
		SendErrorToClient (client, IReqCode, X_ChangeDeviceControl, 
			0, BadLength);
		return Success;
		}
	    if (!dev->valuator)
		{
		SendErrorToClient (client, IReqCode, X_ChangeDeviceControl, 0, 
		    BadMatch);
		return Success;
		}
	    if ((dev->grab) && !SameClient(dev->grab, client))
		{
		rep.status = AlreadyGrabbed;
		WriteReplyToClient(client, sizeof(xChangeDeviceControlReply), 
		    &rep);
		return Success;
		}
	    resolution = (CARD32 *) (r + 1);
	    if (r->first_valuator + r->num_valuators > dev->valuator->numAxes)
		{
		SendErrorToClient (client, IReqCode, X_ChangeDeviceControl, 0, 
		    BadValue);
		return Success;
		}
	    status = ChangeDeviceControl(client, dev, (xDeviceCtl*) r);
	    if (status == Success)
		{
	        a = &dev->valuator->axes[r->first_valuator];
		for (i=0; i<r->num_valuators; i++)
		    if (*(resolution+i) < (a+i)->min_resolution ||
		        *(resolution+i) > (a+i)->max_resolution)
			{
			SendErrorToClient (client, IReqCode, 
			    X_ChangeDeviceControl, 0, BadValue);
			return Success;
			}
		for (i=0; i<r->num_valuators; i++)
		    (a++)->resolution = *resolution++; 
		}
	    else if (status == DeviceBusy)
		{
		rep.status = DeviceBusy;
		WriteReplyToClient(client, sizeof(xChangeDeviceControlReply), 
		    &rep);
		return Success;
		}
	    else 
		{
		SendErrorToClient (client, IReqCode, X_ChangeDeviceControl, 0, 
		    BadMatch);
		return Success;
		}
	    break;
	default:
	    SendErrorToClient (client, IReqCode, X_ChangeDeviceControl, 0, 
		BadValue);
	    return Success;
	}
    WriteReplyToClient(client, sizeof(xChangeDeviceControlReply), &rep);
    return Success;
    }

/***********************************************************************
 *
 * This procedure writes the reply for the xChangeDeviceControl function,
 * if the client and server have a different byte ordering.
 *
 */

void
SRepXChangeDeviceControl (client, size, rep)
    ClientPtr	client;
    int		size;
    xChangeDeviceControlReply	*rep;
    {
    register char n;

    swaps(&rep->sequenceNumber, n);
    swapl(&rep->length, n);
    WriteToClient(client, size, (char *)rep);
    }

