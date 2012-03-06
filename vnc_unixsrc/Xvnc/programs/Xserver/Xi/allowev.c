/* $Xorg: allowev.c,v 1.4 2001/02/09 02:04:33 xorgcvs Exp $ */

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
/* $XFree86: xc/programs/Xserver/Xi/allowev.c,v 3.4 2001/12/14 19:58:54 dawes Exp $ */

/***********************************************************************
 *
 * Function to allow frozen events to be routed from extension input devices.
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

#include "allowev.h"
#include "dixevents.h"

/***********************************************************************
 *
 * This procedure allows frozen events to be routed.
 *
 */

int
SProcXAllowDeviceEvents(client)
    register ClientPtr client;
    {
    register char n;

    REQUEST(xAllowDeviceEventsReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xAllowDeviceEventsReq);
    swapl(&stuff->time, n);
    return(ProcXAllowDeviceEvents(client));
    }

/***********************************************************************
 *
 * This procedure allows frozen events to be routed.
 *
 */

int
ProcXAllowDeviceEvents(client)
    register ClientPtr client;
    {
    TimeStamp		time;
    DeviceIntPtr	thisdev;

    REQUEST(xAllowDeviceEventsReq);
    REQUEST_SIZE_MATCH(xAllowDeviceEventsReq);

    thisdev = LookupDeviceIntRec (stuff->deviceid);
    if (thisdev == NULL)
	{
	SendErrorToClient(client, IReqCode, X_AllowDeviceEvents, 0, BadDevice);
	return Success;
	}
    time = ClientTimeToServerTime(stuff->time);

    switch (stuff->mode)
        {
	case ReplayThisDevice:
	    AllowSome(client, time, thisdev, NOT_GRABBED);
	    break;
	case SyncThisDevice: 
	    AllowSome(client, time, thisdev, FREEZE_NEXT_EVENT);
	    break;
	case AsyncThisDevice: 
	    AllowSome(client, time, thisdev, THAWED);
	    break;
	case AsyncOtherDevices: 
	    AllowSome(client, time, thisdev, THAW_OTHERS);
	    break;
	case SyncAll:
	    AllowSome(client, time, thisdev, FREEZE_BOTH_NEXT_EVENT);
	    break;
	case AsyncAll:
	    AllowSome(client, time, thisdev, THAWED_BOTH);
	    break;
	default: 
	    SendErrorToClient(client, IReqCode, X_AllowDeviceEvents, 0, 
		BadValue);
	    client->errorValue = stuff->mode;
	    return Success;
        }
    return Success;
    }
