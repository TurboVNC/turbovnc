/* $Xorg: ddxCtrls.c,v 1.3 2000/08/17 19:53:45 cpqbld Exp $ */
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
/* $XFree86: xc/programs/Xserver/xkb/ddxCtrls.c,v 1.3 2001/01/17 22:37:14 dawes Exp $ */

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

void
XkbDDXKeybdCtrlProc(DeviceIntPtr dev,KeybdCtrl *ctrl)
{
int realRepeat;

    realRepeat= ctrl->autoRepeat;
    if ((dev->kbdfeed)&&(XkbDDXUsesSoftRepeat(dev)))
	ctrl->autoRepeat= 0;
#ifdef DEBUG
if (xkbDebugFlags&0x4) {
    ErrorF("XkbDDXKeybdCtrlProc: setting repeat to %d (real repeat is %d)\n",
					ctrl->autoRepeat,realRepeat);
}
#endif
    if (dev->key && dev->key->xkbInfo && dev->key->xkbInfo->kbdProc)
	(*dev->key->xkbInfo->kbdProc)(dev,ctrl);
    ctrl->autoRepeat= realRepeat;
    return;
}


int
XkbDDXUsesSoftRepeat(DeviceIntPtr pXDev)
{
#ifndef XKB_ALWAYS_USES_SOFT_REPEAT
    if (pXDev && pXDev->kbdfeed ) {
	if (pXDev->kbdfeed->ctrl.autoRepeat) {
	    if (pXDev->key && pXDev->key->xkbInfo) {
		XkbDescPtr	xkb;
		xkb= pXDev->key->xkbInfo->desc;
		if ((xkb->ctrls->repeat_delay == 660) &&
		    (xkb->ctrls->repeat_interval == 40) &&
		    ((xkb->ctrls->enabled_ctrls&(XkbSlowKeysMask|
						 XkbBounceKeysMask|
						 XkbMouseKeysMask))==0)) {
			return 0;
		}
		return ((xkb->ctrls->enabled_ctrls&XkbRepeatKeysMask)!=0);
	    }
	}
    }
    return 0;
#else
    return 1;
#endif
}

void
XkbDDXChangeControls(DeviceIntPtr dev,XkbControlsPtr old,XkbControlsPtr new)
{
unsigned	changed, i;
unsigned 	char *rep_old, *rep_new, *rep_fb;

    changed= new->enabled_ctrls^old->enabled_ctrls;
#ifdef NOTDEF
    if (changed&XkbRepeatKeysMask) {
	if (dev->kbdfeed) {
	    int realRepeat;

	    if (new->enabled_ctrls&XkbRepeatKeysMask)
		 dev->kbdfeed->ctrl.autoRepeat= realRepeat= 1;
	    else dev->kbdfeed->ctrl.autoRepeat= realRepeat= 0;

	    if (XkbDDXUsesSoftRepeat(dev))
		dev->kbdfeed->ctrl.autoRepeat= FALSE;
	    if (dev->kbdfeed->CtrlProc)
		(*dev->kbdfeed->CtrlProc)(dev,&dev->kbdfeed->ctrl);
	    dev->kbdfeed->ctrl.autoRepeat= realRepeat;
	}
    }
#endif
    for (rep_old = old->per_key_repeat,
         rep_new = new->per_key_repeat,
	 rep_fb  = dev->kbdfeed->ctrl.autoRepeats,
         i = 0; i < XkbPerKeyBitArraySize; i++) {
        if (rep_old[i] != rep_new[i]) {
            rep_fb[i] = rep_new[i];
            changed &= XkbPerKeyRepeatMask;
        }
    }

    if (changed&XkbPerKeyRepeatMask) {
	if (dev->kbdfeed->CtrlProc)
	    (*dev->kbdfeed->CtrlProc)(dev,&dev->kbdfeed->ctrl);
    }
    return;
}

