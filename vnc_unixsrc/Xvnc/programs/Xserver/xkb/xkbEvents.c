/* $Xorg: xkbEvents.c,v 1.3 2000/08/17 19:53:47 cpqbld Exp $ */
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
/* $XFree86: xc/programs/Xserver/xkb/xkbEvents.c,v 3.11 2003/07/16 01:39:10 dawes Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#define NEED_EVENTS 1
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/keysym.h>
#include <X11/extensions/XI.h>
#include "inputstr.h"
#include "windowstr.h"
#include <X11/extensions/XKBsrv.h>
#include "xkb.h"

/***====================================================================***/

void
XkbSendNewKeyboardNotify(DeviceIntPtr kbd,xkbNewKeyboardNotify *pNKN)
{		
register int	i;
Time 		time;
CARD16		changed;

    pNKN->type = XkbEventCode + XkbEventBase;
    pNKN->xkbType = XkbNewKeyboardNotify;
    pNKN->time = time = GetTimeInMillis();
    changed = pNKN->changed;

    for (i=1; i<currentMaxClients; i++) {
        if ((!clients[i]) || clients[i]->clientGone ||
				(clients[i]->requestVector==InitialVector)) {
	    continue;
	}

	if (clients[i]->xkbClientFlags&_XkbClientInitialized)  {
	    if (clients[i]->newKeyboardNotifyMask&changed) {
		pNKN->sequenceNumber = clients[i]->sequence;
		pNKN->time = time;
		pNKN->changed = changed;
		if ( clients[i]->swapped ) {
		    register int n;
		    swaps(&pNKN->sequenceNumber,n);
		    swapl(&pNKN->time,n);
		    swaps(&pNKN->changed,n);
		}
		WriteToClient(clients[i],sizeof(xEvent),(char *)pNKN);
		if (changed&XkbNKN_KeycodesMask) {
		    clients[i]->minKC= pNKN->minKeyCode;
		    clients[i]->maxKC= pNKN->maxKeyCode;
		}
	    }
	}
	else if (changed&XkbNKN_KeycodesMask) {
	    xEvent	event;
	    event.u.u.type= MappingNotify;
	    event.u.mappingNotify.request= MappingKeyboard;
	    event.u.mappingNotify.firstKeyCode= clients[i]->minKC;
	    event.u.mappingNotify.count= clients[i]->maxKC-clients[i]->minKC+1;
	    event.u.u.sequenceNumber= clients[i]->sequence;
	    if (clients[i]->swapped) {
		int n;
		swaps(&event.u.u.sequenceNumber,n);
	    }
	    WriteToClient(clients[i],SIZEOF(xEvent), (char *)&event);
	    event.u.mappingNotify.request= MappingModifier;
	    WriteToClient(clients[i],SIZEOF(xEvent), (char *)&event);
	}
    }
    return;
}

/***====================================================================***/

void
XkbSendStateNotify(DeviceIntPtr kbd,xkbStateNotify *pSN)
{
XkbSrvInfoPtr	xkbi;
XkbStatePtr	state;
XkbInterestPtr	interest;
Time 		time;
register CARD16	changed,bState;

    interest = kbd->xkb_interest;
    if (!interest)
	return;
    xkbi = kbd->key->xkbInfo;
    state= &xkbi->state;

    pSN->type = XkbEventCode + XkbEventBase;
    pSN->xkbType = XkbStateNotify;
    pSN->deviceID = kbd->id;
    pSN->time = time = GetTimeInMillis();
    pSN->mods = state->mods;
    pSN->baseMods = state->base_mods;
    pSN->latchedMods = state->latched_mods;
    pSN->lockedMods = state->locked_mods;
    pSN->group = state->group;
    pSN->baseGroup = state->base_group;
    pSN->latchedGroup = state->latched_group;
    pSN->lockedGroup = state->locked_group;
    pSN->compatState = state->compat_state;
    pSN->grabMods = state->grab_mods;
    pSN->compatGrabMods = state->compat_grab_mods;
    pSN->lookupMods = state->lookup_mods;
    pSN->compatLookupMods = state->compat_lookup_mods;
    pSN->ptrBtnState = state->ptr_buttons;
    changed = pSN->changed;
    bState= pSN->ptrBtnState;

    while (interest) {
	if ((!interest->client->clientGone) &&
	    (interest->client->requestVector != InitialVector) &&
	    (interest->client->xkbClientFlags&_XkbClientInitialized) &&
	    (interest->stateNotifyMask&changed)) {
	    pSN->sequenceNumber = interest->client->sequence;
	    pSN->time = time;
	    pSN->changed = changed;
	    pSN->ptrBtnState = bState;
	    if ( interest->client->swapped ) {
		register int n;
		swaps(&pSN->sequenceNumber,n);
		swapl(&pSN->time,n);
		swaps(&pSN->changed,n);
		swaps(&pSN->ptrBtnState,n);
	    }
	    WriteToClient(interest->client, sizeof(xEvent), (char *)pSN);
	}
	interest= interest->next;
    }
    return;
}

/***====================================================================***/

void
XkbSendMapNotify(DeviceIntPtr kbd,xkbMapNotify *pMN)
{
int 		i;
XkbSrvInfoPtr	xkbi;
unsigned	time = 0,initialized;
CARD16		changed;

    xkbi = kbd->key->xkbInfo;
    initialized= 0;

    changed = pMN->changed;
    pMN->minKeyCode= xkbi->desc->min_key_code;
    pMN->maxKeyCode= xkbi->desc->max_key_code;
    for (i=1; i<currentMaxClients; i++) {
        if (clients[i] && ! clients[i]->clientGone &&
	    (clients[i]->requestVector != InitialVector) &&
	    (clients[i]->xkbClientFlags&_XkbClientInitialized) &&
	    (clients[i]->mapNotifyMask&changed))
	{
	    if (!initialized) {
		pMN->type = XkbEventCode + XkbEventBase;
		pMN->xkbType = XkbMapNotify;
		pMN->deviceID = kbd->id;
		time = GetTimeInMillis();
		initialized= 1;
	    }
	    pMN->time= time;
	    pMN->sequenceNumber = clients[i]->sequence;
	    pMN->changed = changed;
	    if ( clients[i]->swapped ) {
		register int n;
		swaps(&pMN->sequenceNumber,n);
		swapl(&pMN->time,n);
		swaps(&pMN->changed,n);
	    }
	    WriteToClient(clients[i],sizeof(xEvent),(char *)pMN);
	}
    }
    return;
}

int
XkbComputeControlsNotify(	DeviceIntPtr	 	kbd,
				XkbControlsPtr		old,
				XkbControlsPtr		new,
				xkbControlsNotify *	pCN,
				Bool			forceCtrlProc)
{
int		i;
CARD32 		changedControls;

    changedControls= 0;
    if (old->enabled_ctrls!=new->enabled_ctrls)
	changedControls|= XkbControlsEnabledMask;
    if ((old->repeat_delay!=new->repeat_delay)||
	(old->repeat_interval!=new->repeat_interval))
	changedControls|= XkbRepeatKeysMask;
    for (i = 0; i < XkbPerKeyBitArraySize; i++)
	if (old->per_key_repeat[i] != new->per_key_repeat[i])
	    changedControls|= XkbPerKeyRepeatMask;
    if (old->slow_keys_delay!=new->slow_keys_delay)
	changedControls|= XkbSlowKeysMask;
    if (old->debounce_delay!=new->debounce_delay)
	changedControls|= XkbBounceKeysMask;
    if ((old->mk_delay!=new->mk_delay)||
	(old->mk_interval!=new->mk_interval)||
	(old->mk_dflt_btn!=new->mk_dflt_btn))
	changedControls|= XkbMouseKeysMask;
    if ((old->mk_time_to_max!=new->mk_time_to_max)||
	(old->mk_curve!=new->mk_curve)||
	(old->mk_max_speed!=new->mk_max_speed))
	changedControls|= XkbMouseKeysAccelMask;
    if (old->ax_options!=new->ax_options)
	changedControls|= XkbAccessXKeysMask;
    if ((old->ax_options^new->ax_options) & XkbAX_SKOptionsMask)
	changedControls|= XkbStickyKeysMask;
    if ((old->ax_options^new->ax_options) & XkbAX_FBOptionsMask)
	changedControls|= XkbAccessXFeedbackMask;
    if ((old->ax_timeout!=new->ax_timeout)||
	(old->axt_ctrls_mask!=new->axt_ctrls_mask)||
	(old->axt_ctrls_values!=new->axt_ctrls_values)||
	(old->axt_opts_mask!=new->axt_opts_mask)||
	(old->axt_opts_values!= new->axt_opts_values)) {
	changedControls|= XkbAccessXTimeoutMask;
    }
    if ((old->internal.mask!=new->internal.mask)||
	(old->internal.real_mods!=new->internal.real_mods)||
	(old->internal.vmods!=new->internal.vmods))
	changedControls|= XkbInternalModsMask;
    if ((old->ignore_lock.mask!=new->ignore_lock.mask)||
	(old->ignore_lock.real_mods!=new->ignore_lock.real_mods)||
	(old->ignore_lock.vmods!=new->ignore_lock.vmods))
	changedControls|= XkbIgnoreLockModsMask;

    if (new->enabled_ctrls&XkbRepeatKeysMask)
	 kbd->kbdfeed->ctrl.autoRepeat=TRUE;
    else kbd->kbdfeed->ctrl.autoRepeat=FALSE;

    if (kbd->kbdfeed && kbd->kbdfeed->CtrlProc &&
	(changedControls || forceCtrlProc))
	(*kbd->kbdfeed->CtrlProc)(kbd, &kbd->kbdfeed->ctrl);

    if ((!changedControls)&&(old->num_groups==new->num_groups))
	return 0;

    if (!kbd->xkb_interest)
	return 0;

    pCN->changedControls = changedControls;
    pCN->enabledControls = new->enabled_ctrls;
    pCN->enabledControlChanges = (new->enabled_ctrls^old->enabled_ctrls);
    pCN->numGroups = new->num_groups;

    return 1;
}

void
XkbSendControlsNotify(DeviceIntPtr kbd,xkbControlsNotify *pCN)
{
int			initialized;
CARD32 		 	changedControls, enabledControls, enabledChanges = 0;
XkbSrvInfoPtr		xkbi;
XkbInterestPtr		interest;
Time 		 	time = 0;

    interest = kbd->xkb_interest;
    if (!interest)
	return;
    xkbi = kbd->key->xkbInfo;
 
    initialized = 0;
    enabledControls = xkbi->desc->ctrls->enabled_ctrls;
    changedControls = pCN->changedControls;
    pCN->numGroups= xkbi->desc->ctrls->num_groups;
    while (interest) {
	if ((!interest->client->clientGone) &&
	    (interest->client->requestVector != InitialVector) &&
	    (interest->client->xkbClientFlags&_XkbClientInitialized) &&
	    (interest->ctrlsNotifyMask&changedControls)) {
	    if (!initialized) {
		pCN->type = XkbEventCode + XkbEventBase;
		pCN->xkbType = XkbControlsNotify;
		pCN->deviceID = kbd->id;
		pCN->time = time = GetTimeInMillis();
		enabledChanges = pCN->enabledControlChanges;
		initialized= 1;
	    }
	    pCN->changedControls = changedControls;
	    pCN->enabledControls = enabledControls;
	    pCN->enabledControlChanges = enabledChanges;
	    pCN->sequenceNumber = interest->client->sequence;
	    pCN->time = time;
	    if ( interest->client->swapped ) {
		register int n;
		swaps(&pCN->sequenceNumber,n);
		swapl(&pCN->changedControls,n);
		swapl(&pCN->enabledControls,n);
		swapl(&pCN->enabledControlChanges,n);
		swapl(&pCN->time,n);
	    }
	    WriteToClient(interest->client, sizeof(xEvent), (char *)pCN);
	}
	interest= interest->next;
    }
    return;
}

void
XkbSendIndicatorNotify(DeviceIntPtr kbd,int xkbType,xkbIndicatorNotify *pEv)
{
int		initialized;
XkbInterestPtr	interest;
Time 		time = 0;
CARD32		state,changed;

    interest = kbd->xkb_interest;
    if (!interest)
	return;
 
    initialized = 0;
    state = pEv->state;
    changed = pEv->changed;
    while (interest) {
	if ((!interest->client->clientGone) &&
	    (interest->client->requestVector != InitialVector) &&
	    (interest->client->xkbClientFlags&_XkbClientInitialized) &&
	    (((xkbType==XkbIndicatorStateNotify)&&
				(interest->iStateNotifyMask&changed))||
	     ((xkbType==XkbIndicatorMapNotify)&&
	    			(interest->iMapNotifyMask&changed)))) {
	    if (!initialized) {
		pEv->type = XkbEventCode + XkbEventBase;
		pEv->xkbType = xkbType;
		pEv->deviceID = kbd->id;
		pEv->time = time = GetTimeInMillis();
		initialized= 1;
	    }
	    pEv->sequenceNumber = interest->client->sequence;
	    pEv->time = time;
	    pEv->changed = changed;
	    pEv->state = state;
	    if ( interest->client->swapped ) {
		register int n;
		swaps(&pEv->sequenceNumber,n);
		swapl(&pEv->time,n);
		swapl(&pEv->changed,n);
		swapl(&pEv->state,n);
	    }
	    WriteToClient(interest->client, sizeof(xEvent), (char *)pEv);
	}
	interest= interest->next;
    }
    return;
}


void
XkbHandleBell(	BOOL		 force,
		BOOL		 eventOnly,
		DeviceIntPtr	 kbd,
		CARD8		 percent,
		pointer		 pCtrl,
		CARD8		 class,
		Atom		 name,
		WindowPtr	 pWin,
		ClientPtr	 pClient)
{
xkbBellNotify	bn;
int		initialized;
XkbSrvInfoPtr	xkbi;
XkbInterestPtr	interest;
CARD8		id;
CARD16		pitch,duration;
Time 		time = 0;
XID		winID = 0;

    xkbi = kbd->key->xkbInfo;

    if ((force||(xkbi->desc->ctrls->enabled_ctrls&XkbAudibleBellMask))&&
							(!eventOnly)) {
	(*kbd->kbdfeed->BellProc)(percent,kbd,(pointer)pCtrl,class);
    }
    interest = kbd->xkb_interest;
    if ((!interest)||(force))
	return;

    if ((class==0)||(class==KbdFeedbackClass)) {
	KeybdCtrl *pKeyCtrl= (KeybdCtrl *)pCtrl;
	id= pKeyCtrl->id;
	pitch= pKeyCtrl->bell_pitch;
	duration= pKeyCtrl->bell_duration;
    }
    else if (class==BellFeedbackClass) {
	BellCtrl *pBellCtrl= (BellCtrl *)pCtrl;
	id= pBellCtrl->id;
	pitch= pBellCtrl->pitch;
	duration= pBellCtrl->duration;
    }
    else return;
 
    initialized = 0;
    while (interest) {
	if ((!interest->client->clientGone) &&
	    (interest->client->requestVector != InitialVector) &&
	    (interest->client->xkbClientFlags&_XkbClientInitialized) &&
	    (interest->bellNotifyMask)) {
	    if (!initialized) {
		time = GetTimeInMillis();
		bn.type = XkbEventCode + XkbEventBase;
		bn.xkbType = XkbBellNotify;
		bn.deviceID = kbd->id;
		bn.bellClass = class;
		bn.bellID = id;
		bn.percent= percent;
		bn.eventOnly = (eventOnly!=0);
		winID= (pWin?pWin->drawable.id:None);
		initialized= 1;
	    }
	    bn.sequenceNumber = interest->client->sequence;
	    bn.time = time;
	    bn.pitch = pitch;
	    bn.duration = duration;
	    bn.name = name;
	    bn.window=  winID;
	    if ( interest->client->swapped ) {
		register int n;
		swaps(&bn.sequenceNumber,n);
		swapl(&bn.time,n);
		swaps(&bn.pitch,n);
		swaps(&bn.duration,n);
		swapl(&bn.name,n);
		swapl(&bn.window,n);
	    }
	    WriteToClient(interest->client, sizeof(xEvent), (char *)&bn);
	}
	interest= interest->next;
    }
    return;
}

void
XkbSendAccessXNotify(DeviceIntPtr kbd,xkbAccessXNotify *pEv)
{
int		initialized;
XkbInterestPtr	interest;
Time 		time = 0;
CARD16		sk_delay,db_delay;

    interest = kbd->xkb_interest;
    if (!interest)
	return;
 
    initialized = 0;
    sk_delay= pEv->slowKeysDelay;
    db_delay= pEv->debounceDelay;
    while (interest) {
	if ((!interest->client->clientGone) &&
	    (interest->client->requestVector != InitialVector) &&
	    (interest->client->xkbClientFlags&_XkbClientInitialized) &&
	    (interest->accessXNotifyMask&(1<<pEv->detail))) {
	    if (!initialized) {
		pEv->type = XkbEventCode + XkbEventBase;
		pEv->xkbType = XkbAccessXNotify;
		pEv->deviceID = kbd->id;
		pEv->time = time = GetTimeInMillis();
		initialized= 1;
	    }
	    pEv->sequenceNumber = interest->client->sequence;
	    pEv->time = time;
	    pEv->slowKeysDelay = sk_delay;
	    pEv->debounceDelay = db_delay;
	    if ( interest->client->swapped ) {
		register int n;
		swaps(&pEv->sequenceNumber,n);
		swapl(&pEv->time,n);
		swaps(&pEv->slowKeysDelay,n);
		swaps(&pEv->debounceDelay,n);
	    }
	    WriteToClient(interest->client, sizeof(xEvent), (char *)pEv);
	}
	interest= interest->next;
    }
    return;
}

void
XkbSendNamesNotify(DeviceIntPtr kbd,xkbNamesNotify *pEv)
{
int		initialized;
XkbInterestPtr	interest;
Time 		time = 0;
CARD16		changed,changedVirtualMods;
CARD32		changedIndicators;

    interest = kbd->xkb_interest;
    if (!interest)
	return;
 
    initialized = 0;
    changed= pEv->changed;
    changedIndicators= pEv->changedIndicators;
    changedVirtualMods= pEv->changedVirtualMods;
    while (interest) {
	if ((!interest->client->clientGone) &&
	    (interest->client->requestVector != InitialVector) &&
	    (interest->client->xkbClientFlags&_XkbClientInitialized) &&
	    (interest->namesNotifyMask&pEv->changed)) {
	    if (!initialized) {
		pEv->type = XkbEventCode + XkbEventBase;
		pEv->xkbType = XkbNamesNotify;
		pEv->deviceID = kbd->id;
		pEv->time = time = GetTimeInMillis();
		initialized= 1;
	    }
	    pEv->sequenceNumber = interest->client->sequence;
	    pEv->time = time;
	    pEv->changed = changed;
	    pEv->changedIndicators = changedIndicators;
	    pEv->changedVirtualMods= changedVirtualMods;
	    if ( interest->client->swapped ) {
		register int n;
		swaps(&pEv->sequenceNumber,n);
		swapl(&pEv->time,n);
		swaps(&pEv->changed,n);
		swapl(&pEv->changedIndicators,n);
		swaps(&pEv->changedVirtualMods,n);
	    }
	    WriteToClient(interest->client, sizeof(xEvent), (char *)pEv);
	}
	interest= interest->next;
    }
    return;
}

void
XkbSendCompatMapNotify(DeviceIntPtr kbd,xkbCompatMapNotify *pEv)
{
int		initialized;
XkbInterestPtr	interest;
Time 		time = 0;
CARD16		firstSI = 0, nSI = 0, nTotalSI = 0;

    interest = kbd->xkb_interest;
    if (!interest)
	return;
 
    initialized = 0;
    while (interest) {
	if ((!interest->client->clientGone) &&
	    (interest->client->requestVector != InitialVector) &&
	    (interest->client->xkbClientFlags&_XkbClientInitialized) &&
	    (interest->compatNotifyMask)) {
	    if (!initialized) {
		pEv->type = XkbEventCode + XkbEventBase;
		pEv->xkbType = XkbCompatMapNotify;
		pEv->deviceID = kbd->id;
		pEv->time = time = GetTimeInMillis();
		firstSI= pEv->firstSI;
		nSI= pEv->nSI;
		nTotalSI= pEv->nTotalSI;
		initialized= 1;
	    }
	    pEv->sequenceNumber = interest->client->sequence;
	    pEv->time = time;
	    pEv->firstSI = firstSI;
	    pEv->nSI = nSI;
	    pEv->nTotalSI = nTotalSI;
	    if ( interest->client->swapped ) {
		register int n;
		swaps(&pEv->sequenceNumber,n);
		swapl(&pEv->time,n);
		swaps(&pEv->firstSI,n);
		swaps(&pEv->nSI,n);
		swaps(&pEv->nTotalSI,n);
	    }
	    WriteToClient(interest->client, sizeof(xEvent), (char *)pEv);
	}
	interest= interest->next;
    }
    return;
}

void
XkbSendActionMessage(DeviceIntPtr kbd,xkbActionMessage *pEv)
{
int		 initialized;
XkbSrvInfoPtr	 xkbi;
XkbInterestPtr	 interest;
Time 		 time = 0;

    xkbi = kbd->key->xkbInfo;
    interest = kbd->xkb_interest;
    if (!interest)
	return;
 
    initialized = 0;
    pEv->mods= xkbi->state.mods;
    pEv->group= xkbi->state.group;
    while (interest) {
	if ((!interest->client->clientGone) &&
	    (interest->client->requestVector != InitialVector) &&
	    (interest->client->xkbClientFlags&_XkbClientInitialized) &&
	    (interest->actionMessageMask)) {
	    if (!initialized) {
		pEv->type = XkbEventCode + XkbEventBase;
		pEv->xkbType = XkbActionMessage;
		pEv->deviceID = kbd->id;
		pEv->sequenceNumber = interest->client->sequence;
		pEv->time = time = GetTimeInMillis();
		initialized= 1;
	    }
	    pEv->sequenceNumber = interest->client->sequence;
	    pEv->time = time;
	    if ( interest->client->swapped ) {
		register int n;
		swaps(&pEv->sequenceNumber,n);
		swapl(&pEv->time,n);
	    }
	    WriteToClient(interest->client, sizeof(xEvent), (char *)pEv);
	}
	interest= interest->next;
    }
    return;
}

void
XkbSendExtensionDeviceNotify(	DeviceIntPtr 			dev,
				ClientPtr			client,
				xkbExtensionDeviceNotify *	pEv)
{
int		 initialized;
XkbInterestPtr	 interest;
Time 		 time = 0;
CARD32		 defined, state;
CARD16		 reason, supported = 0;

    interest = dev->xkb_interest;
    if (!interest)
	return;
 
    initialized = 0;
    reason= pEv->reason;
    defined= pEv->ledsDefined;
    state= pEv->ledState;
    while (interest) {
	if ((!interest->client->clientGone) &&
	    (interest->client->requestVector != InitialVector) &&
	    (interest->client->xkbClientFlags&_XkbClientInitialized) &&
	    (interest->extDevNotifyMask&reason)) {
	    if (!initialized) {
		pEv->type = XkbEventCode + XkbEventBase;
		pEv->xkbType = XkbExtensionDeviceNotify;
		pEv->deviceID = dev->id;
		pEv->sequenceNumber = interest->client->sequence;
		pEv->time = time = GetTimeInMillis();
		supported= pEv->supported;
		initialized= 1;
	    }
	    else {
		pEv->sequenceNumber = interest->client->sequence;
		pEv->time = time;
		pEv->ledsDefined= defined;
		pEv->ledState= state;
		pEv->reason= reason;
		pEv->supported= supported;
	    }
	    if (client!=interest->client) {
		/* only report UnsupportedFeature to the client that */
		/* issued the failing request */
		pEv->reason&= ~XkbXI_UnsupportedFeatureMask;
		if ((interest->extDevNotifyMask&reason)==0)
		    continue;
	    }
	    if ( interest->client->swapped ) {
		register int n;
		swaps(&pEv->sequenceNumber,n);
		swapl(&pEv->time,n);
		swapl(&pEv->ledsDefined,n);
		swapl(&pEv->ledState,n);
		swaps(&pEv->reason,n);
		swaps(&pEv->supported,n);
	    }
	    WriteToClient(interest->client, sizeof(xEvent), (char *)pEv);
	}
	interest= interest->next;
    }
    return;
}

void
XkbSendNotification(	DeviceIntPtr		kbd,
			XkbChangesPtr		pChanges,
			XkbEventCausePtr	cause)
{
XkbSrvLedInfoPtr	sli;

    sli= NULL;
    if (pChanges->state_changes) {
	xkbStateNotify sn;
	sn.changed= pChanges->state_changes;
	sn.keycode= cause->kc;
	sn.eventType= cause->event;
	sn.requestMajor= cause->mjr;
	sn.requestMinor= cause->mnr;
	XkbSendStateNotify(kbd,&sn);
    }
    if (pChanges->map.changed) {
	xkbMapNotify mn;
	mn.changed= pChanges->map.changed;
	mn.firstType= pChanges->map.first_type;
	mn.nTypes= pChanges->map.num_types;
	mn.firstKeySym= pChanges->map.first_key_sym;
	mn.nKeySyms= pChanges->map.num_key_syms;
	mn.firstKeyAct= pChanges->map.first_key_act;
	mn.nKeyActs= pChanges->map.num_key_acts;
	mn.firstKeyBehavior= pChanges->map.first_key_behavior;
	mn.nKeyBehaviors= pChanges->map.num_key_behaviors;
	mn.virtualMods= pChanges->map.vmods;
	mn.firstKeyExplicit= pChanges->map.first_key_explicit;
	mn.nKeyExplicit= pChanges->map.num_key_explicit;
	mn.firstModMapKey= pChanges->map.first_modmap_key;
	mn.nModMapKeys= pChanges->map.num_modmap_keys;
	mn.firstVModMapKey= pChanges->map.first_vmodmap_key;
	mn.nVModMapKeys= pChanges->map.num_vmodmap_keys;
	XkbSendMapNotify(kbd,&mn);
    }
    if ((pChanges->ctrls.changed_ctrls)||
	(pChanges->ctrls.enabled_ctrls_changes)) {
	xkbControlsNotify cn;
	cn.changedControls= pChanges->ctrls.changed_ctrls;
	cn.enabledControlChanges= pChanges->ctrls.enabled_ctrls_changes;
	cn.keycode= cause->kc;
	cn.eventType= cause->event;
	cn.requestMajor= cause->mjr;
	cn.requestMinor= cause->mnr;
	XkbSendControlsNotify(kbd,&cn);
    }
    if (pChanges->indicators.map_changes) {
	xkbIndicatorNotify in;
	if (sli==NULL)
	    sli= XkbFindSrvLedInfo(kbd,XkbDfltXIClass,XkbDfltXIId,0);
	in.state= sli->effectiveState;
	in.changed= pChanges->indicators.map_changes;
	XkbSendIndicatorNotify(kbd,XkbIndicatorMapNotify,&in);
    }
    if (pChanges->indicators.state_changes) {
	xkbIndicatorNotify in;
	if (sli==NULL)
	    sli= XkbFindSrvLedInfo(kbd,XkbDfltXIClass,XkbDfltXIId,0);
	in.state= sli->effectiveState;
	in.changed= pChanges->indicators.state_changes;
	XkbSendIndicatorNotify(kbd,XkbIndicatorStateNotify,&in);
    }
    if (pChanges->names.changed) {
	xkbNamesNotify nn;
	nn.changed= pChanges->names.changed;
	nn.firstType= pChanges->names.first_type;
	nn.nTypes= pChanges->names.num_types;
	nn.firstLevelName= pChanges->names.first_lvl;
	nn.nLevelNames= pChanges->names.num_lvls;
	nn.nRadioGroups= pChanges->names.num_rg;
	nn.changedVirtualMods= pChanges->names.changed_vmods;
	nn.changedIndicators= pChanges->names.changed_indicators;
	XkbSendNamesNotify(kbd,&nn);
    }
    if ((pChanges->compat.changed_groups)||(pChanges->compat.num_si>0)) {
	xkbCompatMapNotify cmn;
	cmn.changedGroups= pChanges->compat.changed_groups;
	cmn.firstSI= pChanges->compat.first_si;
	cmn.nSI= pChanges->compat.num_si;
	cmn.nTotalSI= kbd->key->xkbInfo->desc->compat->num_si;
	XkbSendCompatMapNotify(kbd,&cmn);
    }
    return;
}

/***====================================================================***/

Bool
XkbFilterEvents(ClientPtr pClient,int nEvents,xEvent *xE)
{
int	i, button_mask;
DeviceIntPtr pXDev = (DeviceIntPtr)LookupKeyboardDevice();
XkbSrvInfoPtr	xkbi;

    xkbi= pXDev->key->xkbInfo;
    if ( pClient->xkbClientFlags & _XkbClientInitialized ) {
#ifdef DEBUG
	if ((xkbDebugFlags&0x10)&&
		((xE[0].u.u.type==KeyPress)||(xE[0].u.u.type==KeyRelease))) {
	    ErrorF("XKbFilterWriteEvents:\n");
	    ErrorF("   Event state= 0x%04x\n",xE[0].u.keyButtonPointer.state);
	    ErrorF("   XkbLastRepeatEvent!=xE (0x%x!=0x%x) %s\n",
			XkbLastRepeatEvent,xE,
			((XkbLastRepeatEvent!=(pointer)xE)?"True":"False"));
	    ErrorF("   (xkbClientEventsFlags&XWDA)==0 (0x%x) %s\n",
		pClient->xkbClientFlags,
		(_XkbWantsDetectableAutoRepeat(pClient)?"True":"False"));
	    ErrorF("   !IsRelease(%d) %s\n",xE[0].u.u.type,
			(!_XkbIsReleaseEvent(xE[0].u.u.type))?"True":"False");
	}
#endif /* DEBUG */
	if (	(XkbLastRepeatEvent==(pointer)xE) &&
	     	(_XkbWantsDetectableAutoRepeat(pClient)) &&
	     	(_XkbIsReleaseEvent(xE[0].u.u.type)) ) {
	    return False;
	}
	if ((pXDev->grab != NullGrab) && pXDev->fromPassiveGrab &&
	    ((xE[0].u.u.type==KeyPress)||(xE[0].u.u.type==KeyRelease))) {
	    register unsigned state,flags;

	    flags= pClient->xkbClientFlags;
	    state= xkbi->state.compat_grab_mods;
	    if (flags & XkbPCF_GrabsUseXKBStateMask) {
		int group;
		if (flags&XkbPCF_LookupStateWhenGrabbed) {
		     group= xkbi->state.group;
		     state= xkbi->state.lookup_mods;
		}
		else {
		    state= xkbi->state.grab_mods;
		    group= xkbi->state.base_group+xkbi->state.latched_group;
		    if ((group<0)||(group>=xkbi->desc->ctrls->num_groups)) {
			group= XkbAdjustGroup(group,xkbi->desc->ctrls);
		    }
		}
		state = XkbBuildCoreState(state, group);
	    }
	    else if (flags&XkbPCF_LookupStateWhenGrabbed)
		state= xkbi->state.compat_lookup_mods;
	    xE[0].u.keyButtonPointer.state= state;
	}
	button_mask = 1 << xE[0].u.u.detail;
	if (xE[0].u.u.type == ButtonPress &&
	    ((xE[0].u.keyButtonPointer.state >> 7) & button_mask) == button_mask &&
	    (xkbi->lockedPtrButtons & button_mask) == button_mask) {
#ifdef DEBUG
	    /* If the MouseKeys is pressed, and the "real" mouse is also pressed
	     * when the mouse is released, the server does not behave properly.
	     * Faking a release of the button here solves the problem.
	     */
	    ErrorF("Faking release of button %d\n", xE[0].u.u.detail);
#endif
	    XkbDDXFakePointerButton(ButtonRelease, xE[0].u.u.detail);
        }
    }
    else {
	register CARD8 	type;

	for (i=0;i<nEvents;i++) {
	    type= xE[i].u.u.type;
#ifdef DEBUG
	    if ((xkbDebugFlags&0x4)&&
		((xE[0].u.u.type==KeyPress)||(xE[0].u.u.type==KeyRelease))) {
		XkbStatePtr s= &xkbi->state;
		ErrorF("XKbFilterWriteEvents (non-XKB):\n");
		ErrorF("event= 0x%04x\n",xE[0].u.keyButtonPointer.state);
		ErrorF("lookup= 0x%02x, grab= 0x%02x\n",s->lookup_mods,
							s->grab_mods);
		ErrorF("compat lookup= 0x%02x, grab= 0x%02x\n",
							s->compat_lookup_mods,
							s->compat_grab_mods);
	    }
#endif
	    if ( (type>=KeyPress)&&(type<=MotionNotify) ) {
		CARD16	old,new;

		old= xE[i].u.keyButtonPointer.state&(~0x1f00);
		new= xE[i].u.keyButtonPointer.state&0x1F00;

		if (old==XkbStateFieldFromRec(&xkbi->state))
		     new|= xkbi->state.compat_lookup_mods;
		else new|= xkbi->state.compat_grab_mods;
		xE[i].u.keyButtonPointer.state= new;
	    }
	    else if ((type==EnterNotify)||(type==LeaveNotify)) {
		xE->u.enterLeave.state&= 0x1F00;
		xE->u.enterLeave.state|= xkbi->state.compat_grab_mods;
	    }
	    button_mask = 1 << xE[i].u.u.detail;
	    if (type == ButtonPress &&
		((xE[i].u.keyButtonPointer.state >> 7) & button_mask) == button_mask &&
		(xkbi->lockedPtrButtons & button_mask) == button_mask) {
#ifdef DEBUG
		ErrorF("Faking release of button %d\n", xE[i].u.u.detail);
#endif
		XkbDDXFakePointerButton(ButtonRelease, xE[i].u.u.detail);
	    }
	}
    }
    return True;
}

/***====================================================================***/

XkbInterestPtr	
XkbFindClientResource(DevicePtr inDev,ClientPtr client)
{
DeviceIntPtr	dev = (DeviceIntPtr)inDev;
XkbInterestPtr	interest;

    if ( dev->xkb_interest ) {
	interest = dev->xkb_interest;
	while (interest){
	    if (interest->client==client) {
		return interest;
	    }
	    interest = interest->next;
	}
    }
    return NULL;
}

XkbInterestPtr	
XkbAddClientResource(DevicePtr inDev,ClientPtr client,XID id)
{
DeviceIntPtr	dev = (DeviceIntPtr)inDev;
XkbInterestPtr	interest;

    interest = dev->xkb_interest;
    while (interest) {
	if (interest->client==client)
	    return ((interest->resource==id)?interest:NULL);
	interest = interest->next;
    }
    interest = _XkbTypedAlloc(XkbInterestRec);
    bzero(interest,sizeof(XkbInterestRec));
    if (interest) {
	interest->dev = dev;
	interest->client = client;
	interest->resource = id;
	interest->stateNotifyMask= 0;
	interest->ctrlsNotifyMask= 0;
	interest->namesNotifyMask= 0;
	interest->compatNotifyMask= 0;
	interest->bellNotifyMask= FALSE;
	interest->accessXNotifyMask= 0;
	interest->iStateNotifyMask= 0;
	interest->iMapNotifyMask= 0;
	interest->altSymsNotifyMask= 0;
	interest->next = dev->xkb_interest;
	dev->xkb_interest= interest;
	return interest;
    }
    return NULL;
}

int
XkbRemoveClient(DevicePtr inDev,ClientPtr client)
{
XkbSrvInfoPtr	xkbi;
DeviceIntPtr	dev = (DeviceIntPtr)inDev;
XkbInterestPtr	interest;
unsigned long	autoCtrls,autoValues;
Bool		found;

    found= False;
    autoCtrls= autoValues= 0;
    if ( dev->xkb_interest ) {
	interest = dev->xkb_interest;
	if (interest && (interest->client==client)){
	    dev->xkb_interest = interest->next;
	    autoCtrls= interest->autoCtrls;
	    autoValues= interest->autoCtrlValues;
	    _XkbFree(interest);
	    found= True;
	}
	while ((!found)&&(interest->next)) {
	    if (interest->next->client==client) {
		XkbInterestPtr	victim = interest->next;
		interest->next = victim->next;
		autoCtrls= victim->autoCtrls;
		autoValues= victim->autoCtrlValues;
		_XkbFree(victim);
		found= True;
	    }
	    interest = interest->next;
	}
    }
    if (found && autoCtrls && dev->key && dev->key->xkbInfo ) {
	XkbEventCauseRec cause;

	xkbi= dev->key->xkbInfo;
	XkbSetCauseXkbReq(&cause,X_kbPerClientFlags,client);
	XkbEnableDisableControls(xkbi,autoCtrls,autoValues,NULL,&cause);
    }
    return found;
}

int
XkbRemoveResourceClient(DevicePtr inDev,XID id) 
{
XkbSrvInfoPtr	xkbi;
DeviceIntPtr	dev = (DeviceIntPtr)inDev;
XkbInterestPtr	interest;
Bool		found;
unsigned long	autoCtrls,autoValues;
ClientPtr	client = NULL;

    found= False;
    autoCtrls= autoValues= 0;
    if ( dev->xkb_interest ) {
	interest = dev->xkb_interest;
	if (interest && (interest->resource==id)){
	    dev->xkb_interest = interest->next;
	    autoCtrls= interest->autoCtrls;
	    autoValues= interest->autoCtrlValues;
	    client= interest->client;
	    _XkbFree(interest);
	    found= True;
	}
	while ((!found)&&(interest->next)) {
	    if (interest->next->resource==id) {
		XkbInterestPtr	victim = interest->next;
		interest->next = victim->next;
		autoCtrls= victim->autoCtrls;
		autoValues= victim->autoCtrlValues;
		client= victim->client;
		_XkbFree(victim);
		found= True;
	    }
	    interest = interest->next;
	}
    }
    if (found && autoCtrls && dev->key && dev->key->xkbInfo ) {
	XkbEventCauseRec cause;

	xkbi= dev->key->xkbInfo;
	XkbSetCauseXkbReq(&cause,X_kbPerClientFlags,client);
	XkbEnableDisableControls(xkbi,autoCtrls,autoValues,NULL,&cause);
    }
    return found;
}
