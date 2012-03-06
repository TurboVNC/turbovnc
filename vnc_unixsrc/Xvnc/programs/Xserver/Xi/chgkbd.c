/* $Xorg: chgkbd.c,v 1.4 2001/02/09 02:04:33 xorgcvs Exp $ */

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
/* $XFree86: xc/programs/Xserver/Xi/chgkbd.c,v 3.6 2001/12/14 19:58:54 dawes Exp $ */

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
#include "XIstubs.h"
#include "globals.h"
#include "extnsionst.h"
#include "extinit.h"			/* LookupDeviceIntRec */

#include "exevents.h"
#include "exglobals.h"

#include "chgkbd.h"
#include "chgptr.h"

/***********************************************************************
 *
 * This procedure changes the keyboard device.
 *
 */

int
SProcXChangeKeyboardDevice(client)
    register ClientPtr client;
    {
    register char n;

    REQUEST(xChangeKeyboardDeviceReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xChangeKeyboardDeviceReq);
    return(ProcXChangeKeyboardDevice(client));
    }

/***********************************************************************
 *
 * This procedure is invoked to swap the request bytes if the server and
 * client have a different byte order.
 *
 */

int
ProcXChangeKeyboardDevice (client)
    register ClientPtr client;
    {
    int				i;
    DeviceIntPtr 		xkbd = inputInfo.keyboard;
    DeviceIntPtr 		dev;
    FocusClassPtr		xf = xkbd->focus;
    FocusClassPtr		df;
    KeyClassPtr 		k;
    xChangeKeyboardDeviceReply	rep;
    changeDeviceNotify		ev;

    REQUEST(xChangeKeyboardDeviceReq);
    REQUEST_SIZE_MATCH(xChangeKeyboardDeviceReq);

    rep.repType = X_Reply;
    rep.RepType = X_ChangeKeyboardDevice;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    dev = LookupDeviceIntRec (stuff->deviceid);
    if (dev == NULL)
	{
	rep.status = -1;
	SendErrorToClient(client, IReqCode, X_ChangeKeyboardDevice, 0, 
		BadDevice);
	return Success;
	}

    k = dev->key;
    if (k == NULL)
	{
	rep.status = -1;
	SendErrorToClient(client, IReqCode, X_ChangeKeyboardDevice, 0, 
		BadMatch);
	return Success;
	}

    if (((dev->grab) && !SameClient(dev->grab, client)) ||
        ((xkbd->grab) && !SameClient(xkbd->grab, client)))
	rep.status = AlreadyGrabbed;
    else if ((dev->sync.frozen &&
	     dev->sync.other && !SameClient(dev->sync.other, client)) ||
	     (xkbd->sync.frozen &&
	      xkbd->sync.other && !SameClient(xkbd->sync.other, client)))
	rep.status = GrabFrozen;
    else
	{
	if (ChangeKeyboardDevice (xkbd, dev) != Success)
	    {
	    SendErrorToClient(client, IReqCode, X_ChangeKeyboardDevice, 0, 
		BadDevice);
	    return Success;
	    }
	if (!dev->focus)
	    InitFocusClassDeviceStruct (dev);
	if (!dev->kbdfeed)
	   InitKbdFeedbackClassDeviceStruct(dev, (BellProcPtr)NoopDDA,
					    (KbdCtrlProcPtr)NoopDDA);
	df = dev->focus;
	df->win = xf->win;
	df->revert = xf->revert;
	df->time = xf->time;
	df->traceGood = xf->traceGood;
	if (df->traceSize != xf->traceSize)
	    {
	    Must_have_memory = TRUE; /* XXX */
	    df->trace = (WindowPtr *) xrealloc(df->trace, 
		xf->traceSize * sizeof(WindowPtr));
	    Must_have_memory = FALSE; /* XXX */
	    }
	df->traceSize = xf->traceSize;
	for (i=0; i<df->traceSize; i++)
	    df->trace[i] = xf->trace[i];
	RegisterOtherDevice (xkbd);
	RegisterKeyboardDevice (dev);

	ev.type = ChangeDeviceNotify;
	ev.deviceid = stuff->deviceid;
	ev.time = currentTime.milliseconds;
	ev.request = NewKeyboard;

	SendEventToAllWindows (dev, ChangeDeviceNotifyMask, (xEvent *)&ev, 1);
	SendMappingNotify (MappingKeyboard, k->curKeySyms.minKeyCode, 
	    k->curKeySyms.maxKeyCode - k->curKeySyms.minKeyCode + 1,client);

	rep.status = 0;
	}

    WriteReplyToClient (client, sizeof (xChangeKeyboardDeviceReply), 
	&rep);
    return Success;
    }

/***********************************************************************
 *
 * This procedure writes the reply for the XChangeKeyboardDevice 
 * function, if the client and server have a different byte ordering.
 *
 */

void
SRepXChangeKeyboardDevice (client, size, rep)
    ClientPtr	client;
    int		size;
    xChangeKeyboardDeviceReply	*rep;
    {
    register char n;

    swaps(&rep->sequenceNumber, n);
    swapl(&rep->length, n);
    WriteToClient(client, size, (char *)rep);
    }
