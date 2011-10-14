/* $Xorg: ddxDevBtn.c,v 1.3 2000/08/17 19:53:45 cpqbld Exp $ */
/************************************************************
Copyright (c) 1995 by Silicon Graphics Computer Systems, Inc.

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of Silicon Graphics not be 
used in advertising or publicity pertaining to distribution 
of the software without specific prior written permission.
Silicon Graphics makes no representation about the suitability 
of this software for any purpose. It is provided "as is"
without any express or implied warranty.

SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS 
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/
/* $XFree86: xc/programs/Xserver/xkb/ddxDevBtn.c,v 3.3 2001/08/23 21:49:51 tsi Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#define	NEED_EVENTS 1
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/keysym.h>
#include "inputstr.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include <X11/extensions/XKBsrv.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>

extern	int	DeviceValuator;

void
XkbDDXFakeDeviceButton(DeviceIntPtr dev,Bool press,int button)
{
int *			devVal;
INT32 *			evVal;
xEvent			events[2];
deviceKeyButtonPointer *btn;
deviceValuator *	val;
int			x,y;
int			nAxes, i, count;

    if ((dev==(DeviceIntPtr)LookupPointerDevice())||(!dev->public.on))
	return;

    nAxes = (dev->valuator?dev->valuator->numAxes:0);
    if (nAxes > 6)
	nAxes = 6;

    GetSpritePosition(&x,&y);
    btn= (deviceKeyButtonPointer *) &events[0];
    val= (deviceValuator *) &events[1];
    if (press)		btn->type= DeviceButtonPress;
    else		btn->type= DeviceButtonRelease;
    btn->detail= 	button;
    btn->time= 		GetTimeInMillis();
    btn->root_x=	x;
    btn->root_y=	y;
    btn->deviceid= 	dev->id;
    count= 1;
    if (nAxes>0) {
	btn->deviceid|=	0x80;
	val->type = DeviceValuator;
	val->deviceid = dev->id;
	val->first_valuator = 0;

	evVal=	&val->valuator0;
	devVal= dev->valuator->axisVal;
	for (i=nAxes;i>0;i--) {
	    *evVal++ = *devVal++;
	    if (evVal > &val->valuator5) {
		int	tmp = val->first_valuator+6;
		val->num_valuators = 6;
		val++;
		evVal= &val->valuator0;
		val->first_valuator= tmp;
	    }
	}
	if ((nAxes % 6) != 0) {
	    val->num_valuators = (nAxes % 6);
	}
	count= 1+((nAxes+5)/6);
    }

    (*dev->public.processInputProc)((xEventPtr)btn, dev, count);
    return;
}
