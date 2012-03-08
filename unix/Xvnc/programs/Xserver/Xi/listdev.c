/* $Xorg: listdev.c,v 1.4 2001/02/09 02:04:34 xorgcvs Exp $ */

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
/* $XFree86: xc/programs/Xserver/Xi/listdev.c,v 3.4 2001/12/14 19:58:58 dawes Exp $ */

/***********************************************************************
 *
 * Extension function to list the available input devices.
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
#include "exglobals.h"			/* FIXME */

#include "listdev.h"

#define VPC	20			/* Max # valuators per chunk */

/***********************************************************************
 *
 * This procedure lists the input devices available to the server.
 *
 */

int
SProcXListInputDevices(client)
    register ClientPtr client;
    {
    register char n;

    REQUEST(xListInputDevicesReq);
    swaps(&stuff->length, n);
    return(ProcXListInputDevices(client));
    }

/***********************************************************************
 *
 * This procedure lists the input devices available to the server.
 *
 */

int
ProcXListInputDevices (client)
    register ClientPtr client;
    {
    xListInputDevicesReply	rep;
    int			numdevs;
    int 		namesize = 1;	/* need 1 extra byte for strcpy */
    int 		size = 0;
    int 		total_length;
    char		*devbuf;
    char		*classbuf;
    char		*namebuf;
    char		*savbuf;
    xDeviceInfo 	*dev;
    DeviceIntPtr 	d;

    REQUEST_SIZE_MATCH(xListInputDevicesReq);

    rep.repType = X_Reply;
    rep.RepType = X_ListInputDevices;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    AddOtherInputDevices ();
    numdevs = inputInfo.numDevices;

    for (d=inputInfo.devices; d; d=d->next)
	SizeDeviceInfo (d, &namesize, &size);
    for (d=inputInfo.off_devices; d; d=d->next)
	SizeDeviceInfo (d, &namesize, &size);

    total_length = numdevs * sizeof (xDeviceInfo) + size + namesize;
    devbuf = (char *) xalloc (total_length);
    classbuf = devbuf + (numdevs * sizeof (xDeviceInfo));
    namebuf = classbuf + size;
    savbuf = devbuf;

    dev = (xDeviceInfoPtr) devbuf;
    for (d=inputInfo.devices; d; d=d->next,dev++)
        ListDeviceInfo (client, d, dev, &devbuf, &classbuf, &namebuf);
    for (d=inputInfo.off_devices; d; d=d->next,dev++)
        ListDeviceInfo (client, d, dev, &devbuf, &classbuf, &namebuf);

    rep.ndevices = numdevs;
    rep.length = (total_length + 3) >> 2;
    WriteReplyToClient (client, sizeof (xListInputDevicesReply), &rep);
    WriteToClient(client, total_length, savbuf);
    xfree (savbuf);
    return Success;
    }

/***********************************************************************
 *
 * This procedure calculates the size of the information to be returned
 * for an input device.
 *
 */

void
SizeDeviceInfo (d, namesize, size)
    DeviceIntPtr d;
    int *namesize;
    int *size;
    {
    int chunks;

    *namesize += 1;
    if (d->name)
	*namesize += strlen (d->name);
    if (d->key != NULL)
	*size += sizeof (xKeyInfo);
    if (d->button != NULL)
	*size += sizeof (xButtonInfo);
    if (d->valuator != NULL)
	{
	chunks = ((int) d->valuator->numAxes + 19) / VPC;
	*size += (chunks * sizeof(xValuatorInfo) + 
		d->valuator->numAxes * sizeof(xAxisInfo));
	}
    }

/***********************************************************************
 *
 * This procedure lists information to be returned for an input device.
 *
 */

void
ListDeviceInfo (client, d, dev, devbuf, classbuf, namebuf)
    ClientPtr client;
    DeviceIntPtr d;
    xDeviceInfoPtr dev;
    char **devbuf;
    char **classbuf;
    char **namebuf;
    {
    CopyDeviceName (namebuf, d->name);
    CopySwapDevice (client, d, 0, devbuf);
    if (d->key != NULL)
	{
	CopySwapKeyClass(client, d->key, classbuf);
	dev->num_classes++;
	}
    if (d->button != NULL)
	{
	CopySwapButtonClass(client, d->button, classbuf);
	dev->num_classes++;
	}
    if (d->valuator != NULL)
	{
	dev->num_classes += CopySwapValuatorClass(client, d->valuator, classbuf);
	}
    }

/***********************************************************************
 *
 * This procedure copies data to the DeviceInfo struct, swapping if necessary.
 *
 * We need the extra byte in the allocated buffer, because the trailing null
 * hammers one extra byte, which is overwritten by the next name except for
 * the last name copied.
 *
 */

void
CopyDeviceName (namebuf, name)
    char **namebuf;
    char *name;
    {
    char *nameptr = (char *) *namebuf;

    if (name)
	{
	*nameptr++ = strlen (name);
	strcpy (nameptr, name);
	*namebuf += (strlen (name)+1);
	}
    else
	{
	*nameptr++ = 0;
	*namebuf += 1;
	}
    }

/***********************************************************************
 *
 * This procedure copies data to the DeviceInfo struct, swapping if necessary.
 *
 */

void
CopySwapDevice (client, d, num_classes, buf)
    register ClientPtr 	client;
    DeviceIntPtr	d;
    int			num_classes;
    char 		**buf;
    {
    register char 	n;
    xDeviceInfoPtr dev;

    dev = (xDeviceInfoPtr) *buf;

    dev->id = d->id;
    dev->type = d->type;
    dev->num_classes = num_classes;
    if (d == inputInfo.keyboard)
	dev->use = IsXKeyboard;
    else if (d == inputInfo.pointer)
	dev->use = IsXPointer;
    else
	dev->use = IsXExtensionDevice;
    if (client->swapped)
	{
	swapl(&dev->type, n);	/* macro - braces are required */
	}
    *buf += sizeof (xDeviceInfo);
    }

/***********************************************************************
 *
 * This procedure copies KeyClass information, swapping if necessary.
 *
 */

void
CopySwapKeyClass (client, k, buf)
    register ClientPtr 	client;
    KeyClassPtr 	k;
    char 		**buf;
    {
    register char 	n;
    xKeyInfoPtr 	k2;

    k2 = (xKeyInfoPtr) *buf;
    k2->class = KeyClass;
    k2->length = sizeof (xKeyInfo);
    k2->min_keycode = k->curKeySyms.minKeyCode;
    k2->max_keycode = k->curKeySyms.maxKeyCode;
    k2->num_keys = k2->max_keycode - k2->min_keycode + 1;
    if (client->swapped)
	{
	swaps(&k2->num_keys,n);
	}
    *buf += sizeof (xKeyInfo);
    }

/***********************************************************************
 *
 * This procedure copies ButtonClass information, swapping if necessary.
 *
 */

void
CopySwapButtonClass (client, b, buf)
    register ClientPtr 	client;
    ButtonClassPtr 	b;
    char 		**buf;
    {
    register char 	n;
    xButtonInfoPtr 	b2;

    b2 = (xButtonInfoPtr) *buf;
    b2->class = ButtonClass;
    b2->length = sizeof (xButtonInfo);
    b2->num_buttons = b->numButtons;
    if (client->swapped)
	{
	swaps(&b2->num_buttons,n);	/* macro - braces are required */
	}
    *buf += sizeof (xButtonInfo);
    }

/***********************************************************************
 *
 * This procedure copies ValuatorClass information, swapping if necessary.
 *
 * Devices may have up to 255 valuators.  The length of a ValuatorClass is
 * defined to be sizeof(ValuatorClassInfo) + num_axes * sizeof (xAxisInfo).
 * The maximum length is therefore (8 + 255 * 12) = 3068.  However, the 
 * length field is one byte.  If a device has more than 20 valuators, we
 * must therefore return multiple valuator classes to the client.
 *
 */

int
CopySwapValuatorClass (client, v, buf)
    register ClientPtr 	client;
    ValuatorClassPtr 	v;
    char 		**buf;
{
    int			i, j, axes, t_axes;
    register char 	n;
    xValuatorInfoPtr 	v2;
    AxisInfo 		*a;
    xAxisInfoPtr 	a2;

    for (i=0,axes=v->numAxes; i < ((v->numAxes+19)/VPC);  i++, axes-=VPC) {
	t_axes = axes < VPC ? axes : VPC;
	if (t_axes < 0)
	    t_axes = v->numAxes % VPC;
	v2 = (xValuatorInfoPtr) *buf;
	v2->class = ValuatorClass;
	v2->length = sizeof (xValuatorInfo) + t_axes * sizeof (xAxisInfo);
	v2->num_axes  = t_axes;
	v2->mode  = v->mode & DeviceMode;
	v2->motion_buffer_size  = v->numMotionEvents;
	if (client->swapped)
	    {
	    swapl(&v2->motion_buffer_size,n);
	    }
	*buf += sizeof (xValuatorInfo);
	a = v->axes + (VPC * i);
	a2 = (xAxisInfoPtr) *buf;
	for (j=0; j<t_axes; j++) {
	    a2->min_value = a->min_value;
	    a2->max_value = a->max_value;
	    a2->resolution = a->resolution;
	    if (client->swapped) {
		swapl(&a2->min_value,n);
		swapl(&a2->max_value,n);
		swapl(&a2->resolution,n);
	    }
	    a2++;
	    a++;
	    *buf += sizeof (xAxisInfo);
	}
    }
    return (i);
}

/***********************************************************************
 *
 * This procedure writes the reply for the XListInputDevices function,
 * if the client and server have a different byte ordering.
 *
 */

void
SRepXListInputDevices (client, size, rep)
    ClientPtr	client;
    int		size;
    xListInputDevicesReply	*rep;
    {
    register char n;

    swaps(&rep->sequenceNumber, n);
    swapl(&rep->length, n);
    WriteToClient(client, size, (char *)rep);
    }
