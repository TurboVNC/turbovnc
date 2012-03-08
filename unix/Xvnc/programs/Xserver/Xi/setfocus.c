/* $Xorg: setfocus.c,v 1.4 2001/02/09 02:04:34 xorgcvs Exp $ */

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
/* $XFree86: xc/programs/Xserver/Xi/setfocus.c,v 3.3 2001/12/14 19:58:59 dawes Exp $ */

/***********************************************************************
 *
 * Request to set the focus for an extension device.
 *
 */

#define	 NEED_EVENTS
#define	 NEED_REPLIES
#include "X.h"				/* for inputstr.h    */
#include "Xproto.h"			/* Request macro     */
#include "windowstr.h"			/* focus struct      */
#include "inputstr.h"			/* DeviceIntPtr	     */
#include "XI.h"
#include "XIproto.h"

#include "dixevents.h"

#include "extnsionst.h"
#include "extinit.h"			/* LookupDeviceIntRec */
#include "exglobals.h"

#include "setfocus.h"

/***********************************************************************
 *
 * This procedure sets the focus for a device.
 *
 */

int
SProcXSetDeviceFocus(client)
    register ClientPtr client;
    {
    register char n;

    REQUEST(xSetDeviceFocusReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xSetDeviceFocusReq);
    swapl(&stuff->focus, n);
    swapl(&stuff->time, n);
    return(ProcXSetDeviceFocus(client));
    }

/***********************************************************************
 *
 * This procedure sets the focus for a device.
 *
 */

int
ProcXSetDeviceFocus(client)
    register ClientPtr client;
    {
    int				ret;
    register DeviceIntPtr	dev;

    REQUEST(xSetDeviceFocusReq);
    REQUEST_SIZE_MATCH(xSetDeviceFocusReq);

    dev = LookupDeviceIntRec (stuff->device);
    if (dev==NULL || !dev->focus)
	{
	SendErrorToClient(client, IReqCode, X_SetDeviceFocus, 0, BadDevice);
	return Success;
	}

    ret = SetInputFocus (client, dev, stuff->focus, stuff->revertTo, 
	stuff->time, TRUE);
    if (ret != Success)
	SendErrorToClient(client, IReqCode, X_SetDeviceFocus, 0, ret);

    return Success;
    }
