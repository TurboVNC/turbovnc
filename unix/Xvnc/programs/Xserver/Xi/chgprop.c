/* $Xorg: chgprop.c,v 1.4 2001/02/09 02:04:33 xorgcvs Exp $ */

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
/* $XFree86: xc/programs/Xserver/Xi/chgprop.c,v 3.3 2001/12/14 19:58:55 dawes Exp $ */

/***********************************************************************
 *
 * Function to modify the dont-propagate-list for an extension input device.
 *
 */

#define	 NEED_EVENTS
#define	 NEED_REPLIES
#include "X.h"				/* for inputstr.h    */
#include "Xproto.h"			/* Request macro     */
#include "inputstr.h"			/* DeviceIntPtr	     */
#include "windowstr.h"
#include "XI.h"
#include "XIproto.h"
#include "extnsionst.h"
#include "extinit.h"			/* LookupDeviceIntRec */

#include "exevents.h"
#include "exglobals.h"

#include "chgprop.h"
#include "grabdev.h"

/***********************************************************************
 *
 * This procedure returns the extension version.
 *
 */

int
SProcXChangeDeviceDontPropagateList(client)
    register ClientPtr client;
    {
    register char n;
    register long *p;
    register int i;

    REQUEST(xChangeDeviceDontPropagateListReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xChangeDeviceDontPropagateListReq);
    swapl(&stuff->window, n);
    swaps(&stuff->count, n);
    p = (long *) &stuff[1];
    for (i=0; i<stuff->count; i++)
        {
        swapl(p, n);
	p++;
        }
    return(ProcXChangeDeviceDontPropagateList(client));
    }

/***********************************************************************
 *
 * This procedure changes the dont-propagate list for the specified window.
 *
 */

int
ProcXChangeDeviceDontPropagateList (client)
    register ClientPtr client;
    {
    int			i;
    WindowPtr		pWin;
    struct 		tmask tmp[EMASKSIZE];
    OtherInputMasks	*others;

    REQUEST(xChangeDeviceDontPropagateListReq);
    REQUEST_AT_LEAST_SIZE(xChangeDeviceDontPropagateListReq);

    if (stuff->length !=(sizeof(xChangeDeviceDontPropagateListReq)>>2) + 
	stuff->count)
	{
	SendErrorToClient (client, IReqCode, X_ChangeDeviceDontPropagateList, 0,
	    BadLength);
	return Success;
	}

    pWin = (WindowPtr) LookupWindow (stuff->window, client);
    if (!pWin)
        {
	client->errorValue = stuff->window;
	SendErrorToClient(client, IReqCode, X_ChangeDeviceDontPropagateList, 0, 
		BadWindow);
	return Success;
        }

    if (stuff->mode != AddToList && stuff->mode != DeleteFromList)
        {
	client->errorValue = stuff->window;
	SendErrorToClient(client, IReqCode, X_ChangeDeviceDontPropagateList, 0, 
		BadMode);
	return Success;
        }

    if (CreateMaskFromList (client, (XEventClass *)&stuff[1], 
	stuff->count, tmp, NULL, X_ChangeDeviceDontPropagateList) != Success)
	    return Success;

    others = wOtherInputMasks(pWin);
    if (!others && stuff->mode == DeleteFromList)
	return Success;
    for (i=0; i<EMASKSIZE; i++)
	{
	if (tmp[i].mask == 0)
	    continue;

	if (stuff->mode == DeleteFromList)
	    tmp[i].mask = (others->dontPropagateMask[i] & ~tmp[i].mask);
	else if (others)
	    tmp[i].mask |= others->dontPropagateMask[i];

	if (DeviceEventSuppressForWindow (pWin,client,tmp[i].mask,i) != Success)
	    {
	    SendErrorToClient ( client, IReqCode, X_ChangeDeviceDontPropagateList, 0, 
		BadClass);
	    return Success;
	    }
	}

    return Success;
    }
