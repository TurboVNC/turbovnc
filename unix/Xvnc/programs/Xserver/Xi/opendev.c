/* $Xorg: opendev.c,v 1.4 2001/02/09 02:04:34 xorgcvs Exp $ */

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
/* $XFree86: xc/programs/Xserver/Xi/opendev.c,v 3.3 2001/12/14 19:58:58 dawes Exp $ */

/***********************************************************************
 *
 * Request to open an extension input device.
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
#include "extnsionst.h"
#include "extinit.h"			/* LookupDeviceIntRec */
#include "exglobals.h"

#include "opendev.h"

extern	CARD8		event_base [];

/***********************************************************************
 *
 * This procedure swaps the request if the server and client have different
 * byte orderings.
 *
 */

int
SProcXOpenDevice(client)
    register ClientPtr client;
    {
    register char n;

    REQUEST(xOpenDeviceReq);
    swaps(&stuff->length, n);
    return(ProcXOpenDevice(client));
    }

/***********************************************************************
 *
 * This procedure causes the server to open an input device.
 *
 */

int
ProcXOpenDevice(client)
    register ClientPtr client;
    {
    xInputClassInfo evbase [numInputClasses];
    Bool enableit = FALSE;
    int j=0;
    int status = Success;
    xOpenDeviceReply	rep;
    DeviceIntPtr dev;

    REQUEST(xOpenDeviceReq);
    REQUEST_SIZE_MATCH(xOpenDeviceReq);

    if (stuff->deviceid == inputInfo.pointer->id || 
	stuff->deviceid == inputInfo.keyboard->id)
	{
	SendErrorToClient(client, IReqCode, X_OpenDevice, 0, BadDevice);
        return Success;
	}

    if ((dev = LookupDeviceIntRec(stuff->deviceid)) == NULL) /* not open */
	{
        for (dev=inputInfo.off_devices; dev; dev=dev->next)
	    if (dev->id == stuff->deviceid)
		break;
	if (dev == NULL)
	    {
	    SendErrorToClient(client, IReqCode, X_OpenDevice, 0, BadDevice);
	    return Success;
	    }
	enableit = TRUE;
	}

    OpenInputDevice (dev, client, &status);
    if (status != Success)
	{
	SendErrorToClient(client, IReqCode, X_OpenDevice, 0, status);
	return Success;
	}
    if (enableit && dev->inited && dev->startup)
	(void)EnableDevice(dev);

    rep.repType = X_Reply;
    rep.RepType = X_OpenDevice;
    rep.sequenceNumber = client->sequence;
    if (dev->key != NULL)
	{
	evbase[j].class = KeyClass;
	evbase[j++].event_type_base = event_base[KeyClass];
	}
    if (dev->button != NULL)
	{
	evbase[j].class = ButtonClass;
	evbase[j++].event_type_base = event_base[ButtonClass];
	}
    if (dev->valuator != NULL)
	{
	evbase[j].class = ValuatorClass;
	evbase[j++].event_type_base = event_base[ValuatorClass];
	}
    if (dev->kbdfeed != NULL || dev->ptrfeed != NULL || dev->leds != NULL ||
	dev->intfeed != NULL || dev->bell != NULL || dev->stringfeed != NULL)
	{
	evbase[j].class = FeedbackClass;
	evbase[j++].event_type_base = event_base[FeedbackClass];
	}
    if (dev->focus != NULL)
	{
	evbase[j].class = FocusClass;
	evbase[j++].event_type_base = event_base[FocusClass];
	}
    if (dev->proximity != NULL)
	{
	evbase[j].class = ProximityClass;
	evbase[j++].event_type_base = event_base[ProximityClass];
	}
    evbase[j].class = OtherClass;
    evbase[j++].event_type_base = event_base[OtherClass];
    rep.length = (j * sizeof (xInputClassInfo) + 3) >> 2;
    rep.num_classes = j;
    WriteReplyToClient (client, sizeof (xOpenDeviceReply), &rep);
    WriteToClient(client, j * sizeof (xInputClassInfo), (char *)evbase);
    return (Success);
    }

/***********************************************************************
 *
 * This procedure writes the reply for the XOpenDevice function,
 * if the client and server have a different byte ordering.
 *
 */

void
SRepXOpenDevice (client, size, rep)
    ClientPtr	client;
    int		size;
    xOpenDeviceReply	*rep;
    {
    register char n;

    swaps(&rep->sequenceNumber, n);
    swapl(&rep->length, n);
    WriteToClient(client, size, (char *)rep);
    }
