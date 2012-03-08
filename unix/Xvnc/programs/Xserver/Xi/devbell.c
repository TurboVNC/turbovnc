/* $Xorg: devbell.c,v 1.4 2001/02/09 02:04:33 xorgcvs Exp $ */

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
/* $XFree86: xc/programs/Xserver/Xi/devbell.c,v 3.3 2001/12/14 19:58:55 dawes Exp $ */

/***********************************************************************
 *
 * Extension function to change the keyboard device.
 *
 */

#define	 NEED_EVENTS
#define	 NEED_REPLIES
#include "X.h"				/* for inputstr.h    */
#include "Xproto.h"			/* Request macro     */
#include "inputstr.h"			/* DeviceIntPtr	     */
#include "XI.h"
#include "XIproto.h"
#include "extnsionst.h"
#include "extinit.h"			/* LookupDeviceIntRec */
#include "exglobals.h"

#include "devbell.h"

/***********************************************************************
 *
 * This procedure is invoked to swap the request bytes if the server and
 * client have a different byte order.
 *
 */

int
SProcXDeviceBell(client)
    register ClientPtr client;
    {
    register char n;

    REQUEST(xDeviceBellReq);
    swaps(&stuff->length, n);
    return(ProcXDeviceBell(client));
    }

/***********************************************************************
 *
 * This procedure rings a bell on an extension device.
 *
 */

int
ProcXDeviceBell (client)
    register ClientPtr client;
    {
    DeviceIntPtr dev;
    KbdFeedbackPtr k;
    BellFeedbackPtr b;
    int base;
    int newpercent;
    CARD8 class;
    pointer ctrl;
    BellProcPtr proc;

    REQUEST(xDeviceBellReq);
    REQUEST_SIZE_MATCH(xDeviceBellReq);

    dev = LookupDeviceIntRec (stuff->deviceid);
    if (dev == NULL)
	{
	client->errorValue = stuff->deviceid;
	SendErrorToClient(client, IReqCode, X_DeviceBell, 0, BadDevice);
	return Success;
	}

    if (stuff->percent < -100 || stuff->percent > 100)
	{
	client->errorValue = stuff->percent;
	SendErrorToClient(client, IReqCode, X_DeviceBell, 0, BadValue);
	return Success;
	}
    if (stuff->feedbackclass == KbdFeedbackClass)
	{
	for (k=dev->kbdfeed; k; k=k->next)
	    if (k->ctrl.id == stuff->feedbackid)
		break;
	if (!k)
	    {
	    client->errorValue = stuff->feedbackid;
	    SendErrorToClient(client, IReqCode, X_DeviceBell, 0, BadValue);
	    return Success;
	    }
	base = k->ctrl.bell;
	proc = k->BellProc;
	ctrl = (pointer) &(k->ctrl);
	class = KbdFeedbackClass;
	}
    else if (stuff->feedbackclass == BellFeedbackClass)
	{
	for (b=dev->bell; b; b=b->next)
	    if (b->ctrl.id == stuff->feedbackid)
		break;
	if (!b)
	    {
	    client->errorValue = stuff->feedbackid;
	    SendErrorToClient(client, IReqCode, X_DeviceBell, 0, BadValue);
	    return Success;
	    }
	base = b->ctrl.percent;
	proc = b->BellProc;
	ctrl = (pointer) &(b->ctrl);
	class = BellFeedbackClass;
	}
    else
	{
	client->errorValue = stuff->feedbackclass;
	SendErrorToClient(client, IReqCode, X_DeviceBell, 0, BadValue);
	return Success;
	}
    newpercent = (base * stuff->percent) / 100;
    if (stuff->percent < 0)
        newpercent = base + newpercent;
    else
    	newpercent = base - newpercent + stuff->percent;
    (*proc)(newpercent, dev, ctrl, class);

    return Success;
    }
