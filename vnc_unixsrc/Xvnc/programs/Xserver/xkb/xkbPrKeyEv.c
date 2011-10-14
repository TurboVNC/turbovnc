/* $Xorg: xkbPrKeyEv.c,v 1.3 2000/08/17 19:53:48 cpqbld Exp $ */
/************************************************************
Copyright (c) 1993 by Silicon Graphics Computer Systems, Inc.

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
/* $XFree86: xc/programs/Xserver/xkb/xkbPrKeyEv.c,v 3.8 2001/01/17 22:37:15 dawes Exp $ */

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
#include <ctype.h>


/***====================================================================***/

void
XkbProcessKeyboardEvent(xEvent *xE,DeviceIntPtr keybd,int count)
{
KeyClassPtr	keyc = keybd->key;
XkbSrvInfoPtr	xkbi;
int		key;
XkbBehavior	behavior;
unsigned        ndx;

    xkbi= keyc->xkbInfo;
    key= xE->u.u.detail;
#ifdef DEBUG
    if (xkbDebugFlags&0x8) {
	ErrorF("XkbPKE: Key %d %s\n",key,(xE->u.u.type==KeyPress?"down":"up"));
    }
#endif

    if ( (xkbi->repeatKey==key) && (xE->u.u.type==KeyRelease) &&
	 ((xkbi->desc->ctrls->enabled_ctrls&XkbRepeatKeysMask)==0) ) {
	AccessXCancelRepeatKey(xkbi,key);
    }

    behavior= xkbi->desc->server->behaviors[key];
    /* The "permanent" flag indicates a hard-wired behavior that occurs */
    /* below XKB, such as a key that physically locks.   XKB does not   */
    /* do anything to implement the behavior, but it *does* report that */
    /* key is hardwired */
    if ((behavior.type&XkbKB_Permanent)==0) {
	switch (behavior.type) {
	    case XkbKB_Default:
		if (( xE->u.u.type == KeyPress ) && 
		    (keyc->down[key>>3] & (1<<(key&7)))) {
		    XkbLastRepeatEvent=	(pointer)xE;
		    xE->u.u.type = KeyRelease;
		    XkbHandleActions(keybd,keybd,xE,count);
		    xE->u.u.type = KeyPress;
		    XkbHandleActions(keybd,keybd,xE,count);
		    XkbLastRepeatEvent= NULL;
		    return;
		}
		else if ((xE->u.u.type==KeyRelease) &&
			(!(keyc->down[key>>3]&(1<<(key&7))))) {
		    XkbLastRepeatEvent=	(pointer)&xE;
		    xE->u.u.type = KeyPress;
		    XkbHandleActions(keybd,keybd,xE,count);
		    xE->u.u.type = KeyRelease;
		    XkbHandleActions(keybd,keybd,xE,count);
		    XkbLastRepeatEvent= NULL;
		    return;
		}
		break;
	    case XkbKB_Lock:
		if ( xE->u.u.type == KeyRelease )
		    return;
		else {
		    int	bit= 1<<(key&7);
		    if ( keyc->down[key>>3]&bit )
			xE->u.u.type= KeyRelease;
		}
		break;
	    case XkbKB_RadioGroup:
		ndx= (behavior.data&(~XkbKB_RGAllowNone));
		if ( ndx<xkbi->nRadioGroups ) {
		    XkbRadioGroupPtr	rg;

		    if ( xE->u.u.type == KeyRelease )
		        return;

		    rg = &xkbi->radioGroups[ndx];
		    if ( rg->currentDown == xE->u.u.detail ) {
		        if (behavior.data&XkbKB_RGAllowNone) {
		            xE->u.u.type = KeyRelease;
			    XkbHandleActions(keybd,keybd,xE,count);
			    rg->currentDown= 0;
		        }
		        return;
		    }
		    if ( rg->currentDown!=0 ) {
			int key = xE->u.u.detail;
			xE->u.u.type= KeyRelease;
			xE->u.u.detail= rg->currentDown;
		        XkbHandleActions(keybd,keybd,xE,count);
		        xE->u.u.type= KeyPress;
		        xE->u.u.detail= key;
		    }
		    rg->currentDown= key;
		}
		else ErrorF("InternalError! Illegal radio group %d\n",ndx);
		break;
	    case XkbKB_Overlay1: case XkbKB_Overlay2:
		{
		    unsigned	which;
		    if (behavior.type==XkbKB_Overlay1)	which= XkbOverlay1Mask;
		    else				which= XkbOverlay2Mask;
		    if ( (xkbi->desc->ctrls->enabled_ctrls&which)==0 )
			break;
		    if ((behavior.data>=xkbi->desc->min_key_code)&&
			(behavior.data<=xkbi->desc->max_key_code)) {
			xE->u.u.detail= behavior.data;
			/* 9/11/94 (ef) -- XXX! need to match release with */
			/*                 press even if the state of the  */
			/*                 corresponding overlay control   */
			/*                 changes while the key is down   */
		    }
		}
		break;
	    default:
		ErrorF("unknown key behavior 0x%04x\n",behavior.type);
#if defined(MetroLink)
		return;
#else
		break;
#endif
	}
    }
    XkbHandleActions(keybd,keybd,xE,count);
    return;
}

void
ProcessKeyboardEvent(xEvent *xE,DeviceIntPtr keybd,int count)
{
KeyClassPtr	keyc = keybd->key;
XkbSrvInfoPtr	xkbi;

    xkbi= keyc->xkbInfo;

#ifdef DEBUG
    if (xkbDebugFlags&0x8) {
	int key= xE->u.u.detail;
	ErrorF("PKE: Key %d %s\n",key,(xE->u.u.type==KeyPress?"down":"up"));
    }
#endif
    if ((xkbi->desc->ctrls->enabled_ctrls&XkbAllFilteredEventsMask)==0)
	XkbProcessKeyboardEvent(xE,keybd,count);
    else if (xE->u.u.type==KeyPress)
	AccessXFilterPressEvent(xE,keybd,count);
    else if (xE->u.u.type==KeyRelease)
	AccessXFilterReleaseEvent(xE,keybd,count);
    return;
}

