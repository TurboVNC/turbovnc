/* $Xorg: xkbPrOtherEv.c,v 1.3 2000/08/17 19:53:48 cpqbld Exp $ */
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
/* $XFree86$ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include <math.h>
#define NEED_EVENTS 1
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/keysym.h>
#include "misc.h"
#include "inputstr.h"
#include <X11/extensions/XKBsrv.h>

#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>

extern	void	ProcessOtherEvent(
    xEvent *		/* xE */,
    DeviceIntPtr 	/* dev */,
    int 		/* count */
);

/***====================================================================***/

void
XkbProcessOtherEvent(xEvent *xE,DeviceIntPtr dev,int count)
{
Bool	xkbCares,isBtn;

    xkbCares= True;
    isBtn= False;
    switch ( xE->u.u.type ) {
      case KeyPress:		xE->u.u.type= DeviceKeyPress; break;
      case KeyRelease:		xE->u.u.type= DeviceKeyRelease; break;
      case ButtonPress:		xE->u.u.type= DeviceButtonPress; 
				isBtn= True; 
				break;
      case ButtonRelease:	xE->u.u.type= DeviceButtonRelease;  
				isBtn= True; 
				break;
      default:			xkbCares= False; break;
    }
    if (xkbCares) {
	if ((!isBtn)||((dev->button)&&(dev->button->xkb_acts))) {
	   DeviceIntPtr	kbd;
	   if (dev->key)	kbd= dev;
	   else		kbd= (DeviceIntPtr)LookupKeyboardDevice();
	   XkbHandleActions(dev,kbd,xE,count);
	   return;
	}
    }
    ProcessOtherEvent(xE,dev,count);
    return;
}

