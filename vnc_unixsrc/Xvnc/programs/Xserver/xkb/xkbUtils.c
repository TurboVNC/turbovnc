/* $Xorg: xkbUtils.c,v 1.3 2000/08/17 19:53:48 cpqbld Exp $ */
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
/* $XFree86: xc/programs/Xserver/xkb/xkbUtils.c,v 3.16 2003/11/03 05:12:02 tsi Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#define NEED_EVENTS 1
#include <X11/X.h>
#include <X11/Xproto.h>
#define	XK_CYRILLIC
#include <X11/keysym.h>
#include "misc.h"
#include "inputstr.h"

#define	XKBSRV_NEED_FILE_FUNCS
#include <X11/extensions/XKBsrv.h>
#include <X11/extensions/XKBgeom.h>
#include "xkb.h"

#ifdef MODE_SWITCH
extern Bool noKME; /* defined in os/utils.c */
#endif

int	XkbDisableLockActions = 0;

/***====================================================================***/

#ifndef RETURN_SHOULD_REPEAT
#if (defined(__osf__) && defined(__alpha))
#define RETURN_SHOULD_REPEAT 1
#else
#define	RETURN_SHOULD_REPEAT 0
#endif
#endif

/***====================================================================***/

DeviceIntPtr
_XkbLookupAnyDevice(int id,int *why_rtrn)
{
DeviceIntPtr dev = NULL;

    dev= (DeviceIntPtr)LookupKeyboardDevice();
    if ((id==XkbUseCoreKbd)||(dev->id==id))
	return dev;

    dev= (DeviceIntPtr)LookupPointerDevice();
    if ((id==XkbUseCorePtr)||(dev->id==id))
	return dev;

    if (id&(~0xff))
	 dev = NULL;

    dev= (DeviceIntPtr)LookupDevice(id);
    if (dev!=NULL) 
	return dev;
    if ((!dev)&&(why_rtrn))
	*why_rtrn= XkbErr_BadDevice;
    return dev;
}

DeviceIntPtr
_XkbLookupKeyboard(int id,int *why_rtrn)
{
DeviceIntPtr dev = NULL;

    if ((dev= _XkbLookupAnyDevice(id,why_rtrn))==NULL)
	return NULL;
    else if ((!dev->key)||(!dev->key->xkbInfo)) {
	if (why_rtrn)
	   *why_rtrn= XkbErr_BadClass;
	return NULL;
    }
    return dev;
}

DeviceIntPtr
_XkbLookupBellDevice(int id,int *why_rtrn)
{
DeviceIntPtr dev = NULL;

    if ((dev= _XkbLookupAnyDevice(id,why_rtrn))==NULL)
	return NULL;
    else if ((!dev->kbdfeed)&&(!dev->bell)) {
	if (why_rtrn)
	   *why_rtrn= XkbErr_BadClass;
	return NULL;
    }
    return dev;
}

DeviceIntPtr
_XkbLookupLedDevice(int id,int *why_rtrn)
{
DeviceIntPtr dev = NULL;

    if ((dev= _XkbLookupAnyDevice(id,why_rtrn))==NULL)
	return NULL;
    else if ((!dev->kbdfeed)&&(!dev->leds)) {
	if (why_rtrn)
	   *why_rtrn= XkbErr_BadClass;
	return NULL;
    }
    return dev;
}

DeviceIntPtr
_XkbLookupButtonDevice(int id,int *why_rtrn)
{
DeviceIntPtr dev = NULL;

    if ((dev= _XkbLookupAnyDevice(id,why_rtrn))==NULL)
	return NULL;
    else if (!dev->button) {
	if (why_rtrn)
	   *why_rtrn= XkbErr_BadClass;
	return NULL;
    }
    return dev;
}

void
XkbSetActionKeyMods(XkbDescPtr xkb,XkbAction *act,unsigned mods)
{
register unsigned	tmp;

    switch (act->type) {
	case XkbSA_SetMods: case XkbSA_LatchMods: case XkbSA_LockMods:
	    if (act->mods.flags&XkbSA_UseModMapMods)
		act->mods.real_mods= act->mods.mask= mods;
	    if ((tmp= XkbModActionVMods(&act->mods))!=0)
		act->mods.mask|= XkbMaskForVMask(xkb,tmp);
	    break;
	case XkbSA_ISOLock:
	    if (act->iso.flags&XkbSA_UseModMapMods)
		act->iso.real_mods= act->iso.mask= mods;
	    if ((tmp= XkbModActionVMods(&act->iso))!=0)
		act->iso.mask|= XkbMaskForVMask(xkb,tmp);
	    break;
    }
    return;
}

unsigned
XkbMaskForVMask(XkbDescPtr xkb,unsigned vmask)
{
register int i,bit;
register unsigned mask;
    
    for (mask=i=0,bit=1;i<XkbNumVirtualMods;i++,bit<<=1) {
	if (vmask&bit)
	    mask|= xkb->server->vmods[i];
    }
    return mask;
}


Bool
XkbApplyVModChanges(	XkbSrvInfoPtr		xkbi,
			unsigned		changed,
			XkbChangesPtr		changes,
			unsigned *		needChecksRtrn,
			XkbEventCausePtr	cause)
{
XkbDescPtr		xkb;
Bool			check;

    xkb= xkbi->desc;
#ifdef DEBUG
{
register unsigned i,bit;
    for (i=0,bit=1;i<XkbNumVirtualMods;i++,bit<<=1) {
	if ((changed&bit)==0)
	    continue;
	if (xkbDebugFlags)
	    ErrorF("Should be applying: change vmod %d to 0x%x\n",i,
					xkb->server->vmods[i]);
    }
}
#endif
    check= XkbApplyVirtualModChanges(xkb,changed,changes);
    XkbApplyVModChangesToAllDevices(xkbi->device,xkb,changed,cause);

    if (needChecksRtrn!=NULL)  {
	if (check)
	     *needChecksRtrn= XkbStateNotifyMask|XkbIndicatorStateNotifyMask;
	else *needChecksRtrn= 0;
    }
    else if (check) {
	/* 7/12/95 (ef) -- XXX check compatibility and/or indicator state */
    }
    return 1;
}

/***====================================================================***/

void
XkbUpdateKeyTypesFromCore(	DeviceIntPtr	pXDev,
				KeyCode	 	first,
				CARD8	 	num,
				XkbChangesPtr	changes)
{
XkbDescPtr		xkb;
unsigned		key,nG,explicit;
KeySymsPtr		pCore;
int			types[XkbNumKbdGroups];
KeySym			tsyms[XkbMaxSymsPerKey],*syms;
XkbMapChangesPtr	mc;

    xkb= pXDev->key->xkbInfo->desc;
#ifdef NOTYET
    if (first<xkb->min_key_code) {
	if (first>=XkbMinLegalKeyCode) {
	    xkb->min_key_code= first;
	    /* 1/12/95 (ef) -- XXX! should zero out the new maps */
	    changes->map.changed|= XkbKeycodesMask;
generate a NewKeyboard notify here?
	}
    }
#endif
    if (first+num-1>xkb->max_key_code) {
	/* 1/12/95 (ef) -- XXX! should allow XKB structures to grow */
	num= xkb->max_key_code-first+1;
    }

    mc= (changes?(&changes->map):NULL);

    pCore= &pXDev->key->curKeySyms;
    syms= &pCore->map[(first-xkb->min_key_code)*pCore->mapWidth];
    for (key=first; key<(first+num); key++,syms+= pCore->mapWidth) {
        explicit= xkb->server->explicit[key]&XkbExplicitKeyTypesMask;
        types[XkbGroup1Index]= XkbKeyKeyTypeIndex(xkb,key,XkbGroup1Index);
        types[XkbGroup2Index]= XkbKeyKeyTypeIndex(xkb,key,XkbGroup2Index);
        types[XkbGroup3Index]= XkbKeyKeyTypeIndex(xkb,key,XkbGroup3Index);
        types[XkbGroup4Index]= XkbKeyKeyTypeIndex(xkb,key,XkbGroup4Index);
        nG= XkbKeyTypesForCoreSymbols(xkb,pCore->mapWidth,syms,explicit,types,
									tsyms);
	XkbChangeTypesOfKey(xkb,key,nG,XkbAllGroupsMask,types,mc);
	memcpy((char *)XkbKeySymsPtr(xkb,key),(char *)tsyms,
					XkbKeyNumSyms(xkb,key)*sizeof(KeySym));
    }
    if (changes->map.changed&XkbKeySymsMask) {
	CARD8 oldLast,newLast;
	oldLast = changes->map.first_key_sym+changes->map.num_key_syms-1;
	newLast = first+num-1;

	if (first<changes->map.first_key_sym)
	    changes->map.first_key_sym = first;
	if (oldLast>newLast)
	    newLast= oldLast;
	changes->map.num_key_syms = newLast-changes->map.first_key_sym+1;
    }
    else {
	changes->map.changed|= XkbKeySymsMask;
	changes->map.first_key_sym = first;
	changes->map.num_key_syms = num;
    }
    return;
}

void
XkbUpdateDescActions(	XkbDescPtr		xkb,
			KeyCode		 	first,
			CARD8		 	num,
			XkbChangesPtr	 	changes)
{
register unsigned	key;

    for (key=first;key<(first+num);key++) {
	XkbApplyCompatMapToKey(xkb,key,changes);
    }

    if (changes->map.changed&(XkbVirtualModMapMask|XkbModifierMapMask)) {
        unsigned char           newVMods[XkbNumVirtualMods];
        register  unsigned      bit,i;
        unsigned                present;

        bzero(newVMods,XkbNumVirtualMods);
        present= 0;
        for (key=xkb->min_key_code;key<=xkb->max_key_code;key++) {
            if (xkb->server->vmodmap[key]==0)
                continue;
            for (i=0,bit=1;i<XkbNumVirtualMods;i++,bit<<=1) {
                if (bit&xkb->server->vmodmap[key]) {
                    present|= bit;
                    newVMods[i]|= xkb->map->modmap[key];
                }
            }
        }
        for (i=0,bit=1;i<XkbNumVirtualMods;i++,bit<<=1) {
            if ((bit&present)&&(newVMods[i]!=xkb->server->vmods[i])) {
                changes->map.changed|= XkbVirtualModsMask;
                changes->map.vmods|= bit;
                xkb->server->vmods[i]= newVMods[i];
            }
        }
    }
    if (changes->map.changed&XkbVirtualModsMask)
        XkbApplyVirtualModChanges(xkb,changes->map.vmods,changes);

    if (changes->map.changed&XkbKeyActionsMask) {
	CARD8 oldLast,newLast;
	oldLast= changes->map.first_key_act+changes->map.num_key_acts-1;
	newLast = first+num-1;

	if (first<changes->map.first_key_act)
	    changes->map.first_key_act = first;
	if (newLast>oldLast)
	    newLast= oldLast;
	changes->map.num_key_acts= newLast-changes->map.first_key_act+1;
    }
    else {
	changes->map.changed|= XkbKeyActionsMask;
	changes->map.first_key_act = first;
	changes->map.num_key_acts = num;
    }
    return;
}

void
XkbUpdateActions(	DeviceIntPtr	 	pXDev,
			KeyCode		 	first,
			CARD8		 	num,
			XkbChangesPtr	 	changes,
			unsigned *	 	needChecksRtrn,
			XkbEventCausePtr	cause)
{
XkbSrvInfoPtr		xkbi;
XkbDescPtr		xkb;
CARD8 *			repeat;

    if (needChecksRtrn)
	*needChecksRtrn= 0;
    xkbi= pXDev->key->xkbInfo;
    xkb= xkbi->desc;
    repeat= xkb->ctrls->per_key_repeat;

    if (pXDev->kbdfeed)
	memcpy(repeat,pXDev->kbdfeed->ctrl.autoRepeats,32);

    XkbUpdateDescActions(xkb,first,num,changes);

    if ((pXDev->kbdfeed)&&
	(changes->ctrls.enabled_ctrls_changes&XkbPerKeyRepeatMask)) {
        memcpy(pXDev->kbdfeed->ctrl.autoRepeats,repeat, 32);
	(*pXDev->kbdfeed->CtrlProc)(pXDev, &pXDev->kbdfeed->ctrl);
    }
    return;
}

void
XkbUpdateCoreDescription(DeviceIntPtr keybd,Bool resize)
{
register int		key,tmp;
int			maxSymsPerKey,maxKeysPerMod;
int			first,last,firstCommon,lastCommon;
XkbDescPtr		xkb;
KeyClassPtr		keyc;
CARD8			keysPerMod[XkbNumModifiers];

    if (!keybd || !keybd->key || !keybd->key->xkbInfo)
	return;
    xkb= keybd->key->xkbInfo->desc;
    keyc= keybd->key;
    maxSymsPerKey= maxKeysPerMod= 0;
    bzero(keysPerMod,sizeof(keysPerMod));
    memcpy(keyc->modifierMap,xkb->map->modmap,xkb->max_key_code+1);
    if ((xkb->min_key_code==keyc->curKeySyms.minKeyCode)&&
	(xkb->max_key_code==keyc->curKeySyms.maxKeyCode)) {
	first= firstCommon= xkb->min_key_code;
	last= lastCommon= xkb->max_key_code;
    }
    else if (resize) {
	keyc->curKeySyms.minKeyCode= xkb->min_key_code;
	keyc->curKeySyms.maxKeyCode= xkb->max_key_code;
	tmp= keyc->curKeySyms.mapWidth*_XkbCoreNumKeys(keyc);
	keyc->curKeySyms.map= _XkbTypedRealloc(keyc->curKeySyms.map,tmp,KeySym);
	if (!keyc->curKeySyms.map)
	   FatalError("Couldn't allocate keysyms\n");
	first= firstCommon= xkb->min_key_code;
	last= lastCommon= xkb->max_key_code;
    }
    else {
	if (xkb->min_key_code<keyc->curKeySyms.minKeyCode) {
	    first= xkb->min_key_code;
	    firstCommon= keyc->curKeySyms.minKeyCode;
	}
	else {
	    firstCommon= xkb->min_key_code;
	    first= keyc->curKeySyms.minKeyCode;
	}
	if (xkb->max_key_code>keyc->curKeySyms.maxKeyCode) {
	    lastCommon= keyc->curKeySyms.maxKeyCode;
	    last= xkb->max_key_code;
	}
	else {
	    lastCommon= xkb->max_key_code;
	    last= keyc->curKeySyms.maxKeyCode;
	}
    }

    /* determine sizes */
    for (key=first;key<=last;key++) {
	if (XkbKeycodeInRange(xkb,key)) {
	    int	nGroups;
	    int	w;
	    nGroups= XkbKeyNumGroups(xkb,key);
	    tmp= 0;
	    if (nGroups>0) {
		if ((w=XkbKeyGroupWidth(xkb,key,XkbGroup1Index))<=2)
		     tmp+= 2;
		else tmp+= w + 2;
	    }
	    if (nGroups>1) {
                if (tmp <= 2) {
		     if ((w=XkbKeyGroupWidth(xkb,key,XkbGroup2Index))<2)
		          tmp+= 2;
		     else tmp+= w;
                } else {
                     if ((w=XkbKeyGroupWidth(xkb,key,XkbGroup2Index))>2)
                          tmp+= w - 2;
                }
	    }
	    if (nGroups>2)
		tmp+= XkbKeyGroupWidth(xkb,key,XkbGroup3Index);
	    if (nGroups>3)
		tmp+= XkbKeyGroupWidth(xkb,key,XkbGroup4Index);
	    if (tmp>maxSymsPerKey)
		maxSymsPerKey= tmp;
	}
	if (_XkbCoreKeycodeInRange(keyc,key)) {
	    if (keyc->modifierMap[key]!=0) {
		register unsigned bit,i,mask;
		mask= keyc->modifierMap[key];
		for (i=0,bit=1;i<XkbNumModifiers;i++,bit<<=1) {
		    if (mask&bit) {
			keysPerMod[i]++;
			if (keysPerMod[i]>maxKeysPerMod)
			    maxKeysPerMod= keysPerMod[i];
		    }
		}
	    }
	}
    }

    if (maxKeysPerMod>0) {
	tmp= maxKeysPerMod*XkbNumModifiers;
	if (keyc->modifierKeyMap==NULL)
	    keyc->modifierKeyMap= (KeyCode *)_XkbCalloc(1, tmp);
	else if (keyc->maxKeysPerModifier<maxKeysPerMod)
	    keyc->modifierKeyMap= (KeyCode *)_XkbRealloc(keyc->modifierKeyMap,tmp);
	if (keyc->modifierKeyMap==NULL)
	    FatalError("Couldn't allocate modifierKeyMap in UpdateCore\n");
	bzero(keyc->modifierKeyMap,tmp);
    }
    else if ((keyc->maxKeysPerModifier>0)&&(keyc->modifierKeyMap!=NULL)) {
	_XkbFree(keyc->modifierKeyMap);
	keyc->modifierKeyMap= NULL;
    }
    keyc->maxKeysPerModifier= maxKeysPerMod;

    if (maxSymsPerKey>0) {
	tmp= maxSymsPerKey*_XkbCoreNumKeys(keyc);
	keyc->curKeySyms.map= _XkbTypedRealloc(keyc->curKeySyms.map,tmp,KeySym);
	if (keyc->curKeySyms.map==NULL)
	    FatalError("Couldn't allocate symbols map in UpdateCore\n");
    }
    else if ((keyc->curKeySyms.mapWidth>0)&&(keyc->curKeySyms.map!=NULL)) {
	_XkbFree(keyc->curKeySyms.map);
	keyc->curKeySyms.map= NULL;
    }
    keyc->curKeySyms.mapWidth= maxSymsPerKey;

    bzero(keysPerMod,sizeof(keysPerMod));
    for (key=firstCommon;key<=lastCommon;key++) {
	if (keyc->curKeySyms.map!=NULL) {
	    KeySym *pCore,*pXKB;
	    unsigned nGroups,groupWidth,n,nOut;

	    nGroups= XkbKeyNumGroups(xkb,key);
	    n= (key-keyc->curKeySyms.minKeyCode)*maxSymsPerKey;
	    pCore= &keyc->curKeySyms.map[n];
	    bzero(pCore,maxSymsPerKey*sizeof(KeySym));
	    pXKB= XkbKeySymsPtr(xkb,key);
	    nOut= 2;
	    if (nGroups>0) {
		groupWidth= XkbKeyGroupWidth(xkb,key,XkbGroup1Index);
		if (groupWidth>0)	pCore[0]= pXKB[0];
		if (groupWidth>1)	pCore[1]= pXKB[1];
		for (n=2;n<groupWidth;n++) {
		    pCore[2+n]= pXKB[n];
		}
		if (groupWidth>2)
		    nOut= groupWidth;
	    }
	    pXKB+= XkbKeyGroupsWidth(xkb,key);
	    nOut+= 2;
	    if (nGroups>1) {
		groupWidth= XkbKeyGroupWidth(xkb,key,XkbGroup2Index);
		if (groupWidth>0)	pCore[2]= pXKB[0];
		if (groupWidth>1)	pCore[3]= pXKB[1];
		for (n=2;n<groupWidth;n++) {
		    pCore[nOut+(n-2)]= pXKB[n];
		}
		if (groupWidth>2)
		    nOut+= (groupWidth-2);
	    }
	    pXKB+= XkbKeyGroupsWidth(xkb,key);
	    for (n=XkbGroup3Index;n<nGroups;n++) {
		register int s;
		groupWidth= XkbKeyGroupWidth(xkb,key,n);
		for (s=0;s<groupWidth;s++) {
		    pCore[nOut++]= pXKB[s];
		}
		pXKB+= XkbKeyGroupsWidth(xkb,key);
	    }
	    if (!pCore[2] && !pCore[3] && maxSymsPerKey >= 6 &&
                (pCore[4] || pCore[5])) {
                pCore[2] = pCore[4];
                pCore[3] = pCore[5];
	    }
	}
	if (keyc->modifierMap[key]!=0) {
	    register unsigned bit,i,mask;
	    mask= keyc->modifierMap[key];
	    for (i=0,bit=1;i<XkbNumModifiers;i++,bit<<=1) {
		if (mask&bit) {
		    tmp= i*maxKeysPerMod+keysPerMod[i];
		    keyc->modifierKeyMap[tmp]= key;
		    keysPerMod[i]++;
		}
	    }
	}
    }
#ifdef MODE_SWITCH
    /* Fix up any of the KME stuff if we changed the core description.
     */
    if (!noKME)
	HandleKeyBinding(keyc, &keyc->curKeySyms);
#endif
    return;
}

void
XkbSetRepeatKeys(DeviceIntPtr pXDev,int key,int onoff)
{
    if (pXDev && pXDev->key && pXDev->key->xkbInfo) {
	xkbControlsNotify	cn;
	XkbControlsPtr		ctrls = pXDev->key->xkbInfo->desc->ctrls;
	XkbControlsRec 		old;
	old = *ctrls;

	if (key== -1) {	/* global autorepeat setting changed */
	    if (onoff)	ctrls->enabled_ctrls |= XkbRepeatKeysMask;
	    else	ctrls->enabled_ctrls &= ~XkbRepeatKeysMask;
	}
	else if (pXDev->kbdfeed) {
	    ctrls->per_key_repeat[key/8] = 
		pXDev->kbdfeed->ctrl.autoRepeats[key/8];
	}
	
	if (XkbComputeControlsNotify(pXDev,&old,ctrls,&cn,True))
	    XkbSendControlsNotify(pXDev,&cn);
    }
    return;
}

void
XkbApplyMappingChange(	DeviceIntPtr	kbd,
			CARD8		 request,
			KeyCode		 firstKey,
			CARD8		 num,
			ClientPtr	 client)
{
XkbEventCauseRec	cause;
XkbChangesRec	 	changes;
unsigned	 	check;

    if (kbd->key->xkbInfo==NULL)
	XkbInitDevice(kbd);
    bzero(&changes,sizeof(XkbChangesRec));
    check= 0;
    if (request==MappingKeyboard) {
	XkbSetCauseCoreReq(&cause,X_ChangeKeyboardMapping,client);
	XkbUpdateKeyTypesFromCore(kbd,firstKey,num,&changes);
	XkbUpdateActions(kbd,firstKey,num,&changes,&check,&cause);
	if (check)
	    XkbCheckSecondaryEffects(kbd->key->xkbInfo,check,&changes,&cause);
    }
    else if (request==MappingModifier) {
	XkbDescPtr	xkb= kbd->key->xkbInfo->desc;

	XkbSetCauseCoreReq(&cause,X_SetModifierMapping,client);

	num = xkb->max_key_code-xkb->min_key_code+1;
	memcpy(xkb->map->modmap,kbd->key->modifierMap,xkb->max_key_code+1);

	changes.map.changed|= XkbModifierMapMask;
	changes.map.first_modmap_key= xkb->min_key_code;
	changes.map.num_modmap_keys= num;
	XkbUpdateActions(kbd,xkb->min_key_code,num,&changes,&check,&cause);
	if (check)
	    XkbCheckSecondaryEffects(kbd->key->xkbInfo,check,&changes,&cause);
    }
    /* 3/26/94 (ef) -- XXX! Doesn't deal with input extension requests */
    XkbSendNotification(kbd,&changes,&cause);
    return;
}

void
XkbDisableComputedAutoRepeats(DeviceIntPtr dev,unsigned key)
{
XkbSrvInfoPtr	xkbi = dev->key->xkbInfo;
xkbMapNotify	mn;

    xkbi->desc->server->explicit[key]|= XkbExplicitAutoRepeatMask;
    bzero(&mn,sizeof(mn));
    mn.changed= XkbExplicitComponentsMask;
    mn.firstKeyExplicit= key;
    mn.nKeyExplicit= 1;
    XkbSendMapNotify(dev,&mn);
    return;
}

unsigned
XkbStateChangedFlags(XkbStatePtr old,XkbStatePtr new)
{
int		changed;

    changed=(old->group!=new->group?XkbGroupStateMask:0);
    changed|=(old->base_group!=new->base_group?XkbGroupBaseMask:0);
    changed|=(old->latched_group!=new->latched_group?XkbGroupLatchMask:0);
    changed|=(old->locked_group!=new->locked_group?XkbGroupLockMask:0);
    changed|=(old->mods!=new->mods?XkbModifierStateMask:0);
    changed|=(old->base_mods!=new->base_mods?XkbModifierBaseMask:0);
    changed|=(old->latched_mods!=new->latched_mods?XkbModifierLatchMask:0);
    changed|=(old->locked_mods!=new->locked_mods?XkbModifierLockMask:0);
    changed|=(old->compat_state!=new->compat_state?XkbCompatStateMask:0);
    changed|=(old->grab_mods!=new->grab_mods?XkbGrabModsMask:0);
    if (old->compat_grab_mods!=new->compat_grab_mods)
	changed|= XkbCompatGrabModsMask;
    changed|=(old->lookup_mods!=new->lookup_mods?XkbLookupModsMask:0);
    if (old->compat_lookup_mods!=new->compat_lookup_mods)
	changed|= XkbCompatLookupModsMask;
    changed|=(old->ptr_buttons!=new->ptr_buttons?XkbPointerButtonMask:0);
    return changed;
}

void
XkbComputeCompatState(XkbSrvInfoPtr xkbi)
{
CARD16 		grp_mask;
XkbStatePtr	state= &xkbi->state;
XkbCompatMapPtr	map;

    map= xkbi->desc->compat;
    grp_mask= map->groups[state->group].mask;
    state->compat_state = state->mods|grp_mask;
    state->compat_lookup_mods= state->lookup_mods|grp_mask;

    if (xkbi->desc->ctrls->enabled_ctrls&XkbIgnoreGroupLockMask)
	 grp_mask= map->groups[state->base_group].mask;
    state->compat_grab_mods= state->grab_mods|grp_mask;
    return;
}

unsigned
XkbAdjustGroup(int group,XkbControlsPtr ctrls)
{
unsigned	act;

    act= XkbOutOfRangeGroupAction(ctrls->groups_wrap);
    if (group<0) {
	while ( group < 0 )  {
	    if (act==XkbClampIntoRange) {
		group= XkbGroup1Index;
	    }
	    else if (act==XkbRedirectIntoRange) {
		int newGroup;
		newGroup= XkbOutOfRangeGroupNumber(ctrls->groups_wrap);
		if (newGroup>=ctrls->num_groups)
		     group= XkbGroup1Index;
		else group= newGroup;
	    }
	    else {
		group+= ctrls->num_groups;
	    }
	}
    }
    else if (group>=ctrls->num_groups) {
	if (act==XkbClampIntoRange) {
	    group= ctrls->num_groups-1;
	}
	else if (act==XkbRedirectIntoRange) {
	    int newGroup;
	    newGroup= XkbOutOfRangeGroupNumber(ctrls->groups_wrap);
	    if (newGroup>=ctrls->num_groups)
		 group= XkbGroup1Index;
	    else group= newGroup;
	}
	else {
	    group%= ctrls->num_groups;
	}
    }
    return group;
}

void
XkbComputeDerivedState(XkbSrvInfoPtr xkbi)
{
XkbStatePtr	state= &xkbi->state;
XkbControlsPtr	ctrls= xkbi->desc->ctrls;
unsigned char	grp;

    state->mods= (state->base_mods|state->latched_mods);
    state->mods|= state->locked_mods;
    state->lookup_mods= state->mods&(~ctrls->internal.mask);
    state->grab_mods= state->lookup_mods&(~ctrls->ignore_lock.mask);
    state->grab_mods|= 
	((state->base_mods|state->latched_mods)&ctrls->ignore_lock.mask);


    grp= state->locked_group;
    if (grp>=ctrls->num_groups)
	state->locked_group= XkbAdjustGroup(grp,ctrls);

    grp= state->locked_group+state->base_group+state->latched_group;
    if (grp>=ctrls->num_groups)
	 state->group= XkbAdjustGroup(grp,ctrls);
    else state->group= grp;
    XkbComputeCompatState(xkbi);
    return;
}

/***====================================================================***/

void
XkbCheckSecondaryEffects(	XkbSrvInfoPtr		xkbi,
				unsigned		which,
				XkbChangesPtr 		changes,
				XkbEventCausePtr	cause)
{
    if (which&XkbStateNotifyMask) {
	XkbStateRec old;
	old= xkbi->state;
	changes->state_changes|= XkbStateChangedFlags(&old,&xkbi->state);
	XkbComputeDerivedState(xkbi);
    }
    if (which&XkbIndicatorStateNotifyMask)
	XkbUpdateIndicators(xkbi->device,XkbAllIndicatorsMask,True,changes,
									cause);
    return;
}

/***====================================================================***/

void
XkbSetPhysicalLockingKey(DeviceIntPtr dev,unsigned key)
{
XkbDescPtr	xkb;

    xkb= dev->key->xkbInfo->desc;
    if ((key>=xkb->min_key_code) && (key<=xkb->max_key_code)) {
	xkb->server->behaviors[key].type= XkbKB_Lock|XkbKB_Permanent;
    }
    else ErrorF("Internal Error!  Bad XKB info in SetPhysicalLockingKey\n");
    return;
}

/***====================================================================***/

Bool
XkbEnableDisableControls(	XkbSrvInfoPtr		xkbi,
				unsigned long		change,
				unsigned long		newValues,
				XkbChangesPtr		changes,
				XkbEventCausePtr	cause)
{
XkbControlsPtr		ctrls;
unsigned 		old;
XkbSrvLedInfoPtr	sli;

    ctrls= xkbi->desc->ctrls;
    old= ctrls->enabled_ctrls;
    ctrls->enabled_ctrls&= ~change;
    ctrls->enabled_ctrls|= (change&newValues);
    if (old==ctrls->enabled_ctrls)
	return False;
    if (cause!=NULL) {
	xkbControlsNotify cn;
	cn.numGroups= ctrls->num_groups;
	cn.changedControls|= XkbControlsEnabledMask;
	cn.enabledControls= ctrls->enabled_ctrls;
	cn.enabledControlChanges= (ctrls->enabled_ctrls^old);
	cn.keycode= cause->kc;
	cn.eventType= cause->event;
	cn.requestMajor= cause->mjr;
	cn.requestMinor= cause->mnr;
	XkbSendControlsNotify(xkbi->device,&cn);
    }
    else {
	/* Yes, this really should be an XOR.  If ctrls->enabled_ctrls_changes*/
	/* is non-zero, the controls in question changed already in "this" */
	/* request and this change merely undoes the previous one.  By the */
	/* same token, we have to figure out whether or not ControlsEnabled */
	/* should be set or not in the changes structure */
	changes->ctrls.enabled_ctrls_changes^= (ctrls->enabled_ctrls^old);
	if (changes->ctrls.enabled_ctrls_changes)
	     changes->ctrls.changed_ctrls|= XkbControlsEnabledMask;
	else changes->ctrls.changed_ctrls&= ~XkbControlsEnabledMask;
    }
    sli= XkbFindSrvLedInfo(xkbi->device,XkbDfltXIClass,XkbDfltXIId,0);
    XkbUpdateIndicators(xkbi->device,sli->usesControls,True,changes,cause);
    return True;
}

/***====================================================================***/

#define	MAX_TOC	16

XkbGeometryPtr 
XkbLookupNamedGeometry(DeviceIntPtr dev,Atom name,Bool *shouldFree)
{
XkbSrvInfoPtr	xkbi=	dev->key->xkbInfo;
XkbDescPtr	xkb=	xkbi->desc;

    *shouldFree= 0;
    if (name==None) {
	if (xkb->geom!=NULL)
	    return xkb->geom;
	name= xkb->names->geometry;
    }
    if ((xkb->geom!=NULL)&&(xkb->geom->name==name))
	return xkb->geom;
    else if ((name==xkb->names->geometry)&&(xkb->geom==NULL)) {
	FILE *file= XkbDDXOpenConfigFile(XkbInitialMap,NULL,0);
	if (file!=NULL) {
	    XkbFileInfo		xkbFInfo;
	    xkmFileInfo		finfo;
	    xkmSectionInfo	toc[MAX_TOC],*entry;
	    bzero(&xkbFInfo,sizeof(xkbFInfo));
	    xkbFInfo.xkb= xkb;
	    if (XkmReadTOC(file,&finfo,MAX_TOC,toc)) {
		entry= XkmFindTOCEntry(&finfo,toc,XkmGeometryIndex);
		if (entry!=NULL)
		    XkmReadFileSection(file,entry,&xkbFInfo,NULL);
	    }
	    fclose(file);
	    if (xkb->geom) {
		*shouldFree= 0;
		return xkb->geom;
	    }
	}
    }
    *shouldFree= 1;
    return NULL;
}

void
XkbConvertCase(register KeySym sym, KeySym *lower, KeySym *upper)
{
    *lower = sym;
    *upper = sym;
    switch(sym >> 8) {
    case 0: /* Latin 1 */
	if ((sym >= XK_A) && (sym <= XK_Z))
	    *lower += (XK_a - XK_A);
	else if ((sym >= XK_a) && (sym <= XK_z))
	    *upper -= (XK_a - XK_A);
	else if ((sym >= XK_Agrave) && (sym <= XK_Odiaeresis))
	    *lower += (XK_agrave - XK_Agrave);
	else if ((sym >= XK_agrave) && (sym <= XK_odiaeresis))
	    *upper -= (XK_agrave - XK_Agrave);
	else if ((sym >= XK_Ooblique) && (sym <= XK_Thorn))
	    *lower += (XK_oslash - XK_Ooblique);
	else if ((sym >= XK_oslash) && (sym <= XK_thorn))
	    *upper -= (XK_oslash - XK_Ooblique);
	break;
    case 1: /* Latin 2 */
	/* Assume the KeySym is a legal value (ignore discontinuities) */
	if (sym == XK_Aogonek)
	    *lower = XK_aogonek;
	else if (sym >= XK_Lstroke && sym <= XK_Sacute)
	    *lower += (XK_lstroke - XK_Lstroke);
	else if (sym >= XK_Scaron && sym <= XK_Zacute)
	    *lower += (XK_scaron - XK_Scaron);
	else if (sym >= XK_Zcaron && sym <= XK_Zabovedot)
	    *lower += (XK_zcaron - XK_Zcaron);
	else if (sym == XK_aogonek)
	    *upper = XK_Aogonek;
	else if (sym >= XK_lstroke && sym <= XK_sacute)
	    *upper -= (XK_lstroke - XK_Lstroke);
	else if (sym >= XK_scaron && sym <= XK_zacute)
	    *upper -= (XK_scaron - XK_Scaron);
	else if (sym >= XK_zcaron && sym <= XK_zabovedot)
	    *upper -= (XK_zcaron - XK_Zcaron);
	else if (sym >= XK_Racute && sym <= XK_Tcedilla)
	    *lower += (XK_racute - XK_Racute);
	else if (sym >= XK_racute && sym <= XK_tcedilla)
	    *upper -= (XK_racute - XK_Racute);
	break;
    case 2: /* Latin 3 */
	/* Assume the KeySym is a legal value (ignore discontinuities) */
	if (sym >= XK_Hstroke && sym <= XK_Hcircumflex)
	    *lower += (XK_hstroke - XK_Hstroke);
	else if (sym >= XK_Gbreve && sym <= XK_Jcircumflex)
	    *lower += (XK_gbreve - XK_Gbreve);
	else if (sym >= XK_hstroke && sym <= XK_hcircumflex)
	    *upper -= (XK_hstroke - XK_Hstroke);
	else if (sym >= XK_gbreve && sym <= XK_jcircumflex)
	    *upper -= (XK_gbreve - XK_Gbreve);
	else if (sym >= XK_Cabovedot && sym <= XK_Scircumflex)
	    *lower += (XK_cabovedot - XK_Cabovedot);
	else if (sym >= XK_cabovedot && sym <= XK_scircumflex)
	    *upper -= (XK_cabovedot - XK_Cabovedot);
	break;
    case 3: /* Latin 4 */
	/* Assume the KeySym is a legal value (ignore discontinuities) */
	if (sym >= XK_Rcedilla && sym <= XK_Tslash)
	    *lower += (XK_rcedilla - XK_Rcedilla);
	else if (sym >= XK_rcedilla && sym <= XK_tslash)
	    *upper -= (XK_rcedilla - XK_Rcedilla);
	else if (sym == XK_ENG)
	    *lower = XK_eng;
	else if (sym == XK_eng)
	    *upper = XK_ENG;
	else if (sym >= XK_Amacron && sym <= XK_Umacron)
	    *lower += (XK_amacron - XK_Amacron);
	else if (sym >= XK_amacron && sym <= XK_umacron)
	    *upper -= (XK_amacron - XK_Amacron);
	break;
    case 6: /* Cyrillic */
	/* Assume the KeySym is a legal value (ignore discontinuities) */
	if (sym >= XK_Serbian_DJE && sym <= XK_Serbian_DZE)
	    *lower -= (XK_Serbian_DJE - XK_Serbian_dje);
	else if (sym >= XK_Serbian_dje && sym <= XK_Serbian_dze)
	    *upper += (XK_Serbian_DJE - XK_Serbian_dje);
	else if (sym >= XK_Cyrillic_YU && sym <= XK_Cyrillic_HARDSIGN)
	    *lower -= (XK_Cyrillic_YU - XK_Cyrillic_yu);
	else if (sym >= XK_Cyrillic_yu && sym <= XK_Cyrillic_hardsign)
	    *upper += (XK_Cyrillic_YU - XK_Cyrillic_yu);
        break;
    case 7: /* Greek */
	/* Assume the KeySym is a legal value (ignore discontinuities) */
	if (sym >= XK_Greek_ALPHAaccent && sym <= XK_Greek_OMEGAaccent)
	    *lower += (XK_Greek_alphaaccent - XK_Greek_ALPHAaccent);
	else if (sym >= XK_Greek_alphaaccent && sym <= XK_Greek_omegaaccent &&
		 sym != XK_Greek_iotaaccentdieresis &&
		 sym != XK_Greek_upsilonaccentdieresis)
	    *upper -= (XK_Greek_alphaaccent - XK_Greek_ALPHAaccent);
	else if (sym >= XK_Greek_ALPHA && sym <= XK_Greek_OMEGA)
	    *lower += (XK_Greek_alpha - XK_Greek_ALPHA);
	else if (sym >= XK_Greek_alpha && sym <= XK_Greek_omega &&
		 sym != XK_Greek_finalsmallsigma)
	    *upper -= (XK_Greek_alpha - XK_Greek_ALPHA);
        break;
    }
}
