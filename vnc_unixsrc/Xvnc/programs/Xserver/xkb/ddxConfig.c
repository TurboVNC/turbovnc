/* $Xorg: ddxConfig.c,v 1.3 2000/08/17 19:53:45 cpqbld Exp $ */
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
/* $XFree86: xc/programs/Xserver/xkb/ddxConfig.c,v 3.8 2002/12/20 20:18:35 paulo Exp $ */

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
#include "os.h"
#define	XKBSRV_NEED_FILE_FUNCS
#include <X11/extensions/XKBsrv.h>
#include <X11/extensions/XKBconfig.h>

Bool
XkbDDXApplyConfig(XPointer cfg_in,XkbSrvInfoPtr info)
{
XkbConfigRtrnPtr 	rtrn;
XkbDescPtr		xkb;
Bool			ok;
XkbEventCauseRec	cause;

    xkb= info->desc;
    rtrn= (XkbConfigRtrnPtr)cfg_in;
    if (rtrn==NULL)
	return True;
    ok= XkbCFApplyRtrnValues(rtrn,XkbCFDflts,xkb);
    if (rtrn->initial_mods.replace) {
	info->state.locked_mods= rtrn->initial_mods.mods;
    }
    else {
	info->state.locked_mods|= rtrn->initial_mods.mods;
	if (rtrn->initial_mods.mods_clear)
	    info->state.locked_mods&= ~rtrn->initial_mods.mods_clear;
    }
    XkbComputeDerivedState(info);
    XkbSetCauseUnknown(&cause);
    XkbUpdateIndicators(info->device,XkbAllIndicatorsMask,False,NULL,&cause);
    if (info->device && info->device->kbdfeed) {
	DeviceIntPtr	dev;
	KeybdCtrl	newCtrl;
	dev= info->device;
	newCtrl= dev->kbdfeed->ctrl;
	if (rtrn->click_volume>=0)
	    newCtrl.click= rtrn->click_volume;
	if (rtrn->bell_volume>=0)
	    newCtrl.bell= rtrn->bell_volume;
	if (rtrn->bell_pitch>0)
	    newCtrl.bell_pitch= rtrn->bell_pitch;
	if (rtrn->bell_duration>0)
	    newCtrl.bell_duration= rtrn->bell_duration;
	if (dev->kbdfeed->CtrlProc)
	    (*dev->kbdfeed->CtrlProc)(dev,&newCtrl);
    }
    XkbCFFreeRtrn(rtrn,XkbCFDflts,xkb);
    return ok;
}

XPointer
XkbDDXPreloadConfig(	char **			rulesRtrn,
			XkbRF_VarDefsPtr 	defs,
			XkbComponentNamesPtr	names,
			DeviceIntPtr 		dev)
{
char			buf[PATH_MAX];
char *			dName;
FILE *			file;
XkbConfigRtrnPtr	rtrn;

#if defined(MetroLink)
    if (dev && dev->name)
	 dName= dev->name;
    else dName= "";
    /* It doesn't appear that XkbBaseDirectory could ever get set to NULL */
    sprintf(buf,"%s/X%s-config%s%s",XkbBaseDirectory,display,
 						(dName[0]?".":""),dName);
#else
    if (dev && dev->name)
	 dName= dev->name;
    else dName= "";
    if (XkbBaseDirectory!=NULL) {
	if (strlen(XkbBaseDirectory)+strlen(display)
		+strlen(dName)+10+(dName[0]?1:0) > PATH_MAX)
	{
#ifdef DEBUG
	    ErrorF("path exceeds max length\n");
#endif
	    return NULL;
	}
	sprintf(buf,"%s/X%s-config%s%s",XkbBaseDirectory,display,
						(dName[0]?".":""),dName);
    }
    else {
	if (strlen(display)+strlen(dName)+10+(dName[0]?1:0) > PATH_MAX)
	{
#ifdef DEBUG
	    ErrorF("path exceeds max length\n");
#endif
	    return NULL;
	}
        sprintf(buf,"X%s-config%s%s",display,(dName[0]?".":""),dName);
    }
#endif
#ifdef __UNIXOS2__
    strcpy(buf,(char*)__XOS2RedirRoot(buf));
#endif
#ifdef DEBUG
    ErrorF("Looking for keyboard configuration in %s...",buf);
#endif
    file= fopen(buf,"r");
    if (file==NULL) {
#ifdef DEBUG
	ErrorF("file not found\n");
#endif
	return NULL;
    }
    rtrn= _XkbTypedCalloc(1,XkbConfigRtrnRec);
    if (rtrn!=NULL) {
	if (!XkbCFParse(file,XkbCFDflts,NULL,rtrn)) {
#ifdef DEBUG
	    ErrorF("error\n");
#endif
	    ErrorF("Error parsing config file: ");
	    XkbCFReportError(stderr,buf,rtrn->error,rtrn->line);
	    _XkbFree(rtrn);
	    fclose(file);
	    return NULL;
	}
#ifdef DEBUG
	ErrorF("found it\n");
#endif
        if (rtrn->rules_file) {
	    *rulesRtrn= rtrn->rules_file;
	    rtrn->rules_file= NULL;
	}
	if (rtrn->model) {
	    defs->model= rtrn->model;
	    rtrn->model= NULL;
	}
	if (rtrn->layout) {
	    defs->layout= rtrn->layout;
	    rtrn->layout= NULL;
	}
	if (rtrn->variant) {
	    defs->variant= rtrn->variant;
	    rtrn->variant= NULL;
	}
	if (rtrn->options) {
	    defs->options= rtrn->options;
	    rtrn->options= NULL;
	}
	XkbSetRulesUsed(defs);

	if (rtrn->keycodes!=NULL) {
	    if (names->keycodes) _XkbFree(names->keycodes);
	    names->keycodes= rtrn->keycodes;
	    rtrn->keycodes= NULL;
	}
	if (rtrn->geometry!=NULL) {
	    if (names->geometry) _XkbFree(names->geometry);
	    names->geometry= rtrn->geometry;
	    rtrn->geometry= NULL;
	}
	if (rtrn->symbols!=NULL) {
	    if (rtrn->phys_symbols==NULL)
		rtrn->phys_symbols= _XkbDupString(names->symbols);
	    if (names->symbols) _XkbFree(names->symbols);
	    names->symbols= rtrn->symbols;
	    rtrn->symbols= NULL;
	}
	if (rtrn->types!=NULL) {
	    if (names->types) _XkbFree(names->types);
	    names->types= rtrn->types;
	    rtrn->types= NULL;
	}
	if (rtrn->compat!=NULL) {
	    if (names->compat) _XkbFree(names->compat);
	    names->compat= rtrn->compat;
	    rtrn->compat= NULL;
	}
    }
    fclose(file);
    return (XPointer)rtrn;
}
