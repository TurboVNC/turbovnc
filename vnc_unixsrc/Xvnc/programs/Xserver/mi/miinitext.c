/* $XdotOrg: xc/programs/Xserver/mi/miinitext.c,v 1.26 2005/07/16 03:49:59 kem Exp $ */
/* $XFree86: xc/programs/Xserver/mi/miinitext.c,v 3.67 2003/01/12 02:44:27 dawes Exp $ */
/***********************************************************

Copyright 1987, 1998  The Open Group

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


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $Xorg: miinitext.c,v 1.4 2001/02/09 02:05:21 xorgcvs Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#ifdef HAVE_DMX_CONFIG_H
#include <dmx-config.h>
#endif

#ifdef HAVE_XNEST_CONFIG_H
#include <xnest-config.h>
#undef DPMSExtension
#endif

#include "misc.h"
#include "extension.h"
#include "micmap.h"

#if defined(QNX4) /* sleaze for Watcom on QNX4 ... */
#undef GLXEXT
#endif

/* Make sure Xprt only announces extensions it supports */
#ifdef PRINT_ONLY_SERVER
#undef MITSHM /* this is incompatible to the vector-based Xprint DDX */
#undef XKB
#undef PANORAMIX
#undef RES
#undef XINPUT
#undef XV
#undef SCREENSAVER
#undef XIDLE
#undef XRECORD
#undef XF86VIDMODE
#undef XF86MISC
#undef XFreeXDGA
#undef XF86DRI
#undef DPMSExtension
#undef DPSEXT
#undef FONTCACHE
#undef DAMAGE
#undef XFIXES
#undef XEVIE
#else
#ifndef LOADABLEPRINTDDX
#undef XPRINT
#endif /* LOADABLEPRINTDDX */
#endif /* PRINT_ONLY_SERVER */


extern Bool noTestExtensions;

#ifdef BIGREQS
extern Bool noBigReqExtension;
#endif
#ifdef COMPOSITE
extern Bool noCompositeExtension;
#endif
#ifdef DAMAGE
extern Bool noDamageExtension;
#endif
#ifdef DBE
extern Bool noDbeExtension;
#endif
#ifdef DPSEXT
extern Bool noDPSExtension;
#endif
#ifdef DPMSExtension
extern Bool noDPMSExtension;
#endif
#ifdef EVI
extern Bool noEVIExtension;
#endif
#ifdef FONTCACHE
extern Bool noFontCacheExtension;
#endif
#ifdef GLXEXT
extern Bool noGlxExtension;
#endif
#ifdef LBX
extern Bool noLbxExtension;
#endif
#ifdef SCREENSAVER
extern Bool noScreenSaverExtension;
#endif
#ifdef MITSHM
extern Bool noMITShmExtension;
#endif
#ifdef MITMISC
extern Bool noMITMiscExtension;
#endif
#ifdef MULTIBUFFER
extern Bool noMultibufferExtension;
#endif
#ifdef RANDR
extern Bool noRRExtension;
#endif
#ifdef RENDER
extern Bool noRenderExtension;
#endif
#ifdef SHAPE
extern Bool noShapeExtension;
#endif
#ifdef XCSECURITY
extern Bool noSecurityExtension;
#endif
#ifdef XSYNC
extern Bool noSyncExtension;
#endif
#ifdef TOGCUP
extern Bool noXcupExtension;
#endif
#ifdef RES
extern Bool noResExtension;
#endif
#ifdef XAPPGROUP
extern Bool noXagExtension;
#endif
#ifdef XCMISC
extern Bool noXCMiscExtension;
#endif
#ifdef XEVIE
extern Bool noXevieExtension;
#endif
#ifdef XF86BIGFONT
extern Bool noXFree86BigfontExtension;
#endif
#ifdef XFreeXDGA
extern Bool noXFree86DGAExtension;
#endif
#ifdef XF86DRI
extern Bool noXFree86DRIExtension;
#endif
#ifdef XF86MISC
extern Bool noXFree86MiscExtension;
#endif
#ifdef XF86VIDMODE
extern Bool noXFree86VidModeExtension;
#endif
#ifdef XFIXES
extern Bool noXFixesExtension;
#endif
#ifdef XKB
/* |noXkbExtension| is defined in xc/programs/Xserver/xkb/xkbInit.c */
extern Bool noXkbExtension;
#endif
#ifdef PANORAMIX
extern Bool noPanoramiXExtension;
#endif
#ifdef XINPUT
extern Bool noXInputExtension;
#endif
#ifdef XIDLE
extern Bool noXIdleExtension;
#endif
#ifdef XV
extern Bool noXvExtension;
#endif

#ifndef XFree86LOADER
#define INITARGS void
typedef void (*InitExtension)(INITARGS);
#else /* XFree86Loader */
#include "loaderProcs.h"
#endif

#ifdef MITSHM
#define _XSHM_SERVER_
#include <X11/extensions/shmstr.h>
#endif
#ifdef XTEST
#define _XTEST_SERVER_
#include <X11/extensions/XTest.h>
#endif
#ifdef XKB
#include <X11/extensions/XKB.h>
#endif
#ifdef LBX
#define _XLBX_SERVER_
#include <X11/extensions/lbxstr.h>
#endif
#ifdef XPRINT
#include "Print.h"
#endif
#ifdef XAPPGROUP
#define _XAG_SERVER_
#include <X11/extensions/Xagstr.h>
#endif
#ifdef XCSECURITY
#define _SECURITY_SERVER
#include <X11/extensions/securstr.h>
#endif
#ifdef PANORAMIX
#include <X11/extensions/panoramiXproto.h>
#endif
#ifdef XF86BIGFONT
#include <X11/extensions/xf86bigfstr.h>
#endif
#ifdef RES
#include <X11/extensions/XResproto.h>
#endif

/* FIXME: this whole block of externs should be from the appropriate headers */
#ifdef XTESTEXT1
extern void XTestExtension1Init(INITARGS);
#endif
#ifdef SHAPE
extern void ShapeExtensionInit(INITARGS);
#endif
#ifdef EVI
extern void EVIExtensionInit(INITARGS);
#endif
#ifdef MITSHM
extern void ShmExtensionInit(INITARGS);
#endif
#ifdef MULTIBUFFER
extern void MultibufferExtensionInit(INITARGS);
#endif
#ifdef PANORAMIX
extern void PanoramiXExtensionInit(INITARGS);
#endif
#ifdef XINPUT
extern void XInputExtensionInit(INITARGS);
#endif
#ifdef XTEST
extern void XTestExtensionInit(INITARGS);
#endif
#ifdef BIGREQS
extern void BigReqExtensionInit(INITARGS);
#endif
#ifdef MITMISC
extern void MITMiscExtensionInit(INITARGS);
#endif
#ifdef XIDLE
extern void XIdleExtensionInit(INITARGS);
#endif
#ifdef XTRAP
extern void DEC_XTRAPInit(INITARGS);
#endif
#ifdef SCREENSAVER
extern void ScreenSaverExtensionInit (INITARGS);
#endif
#ifdef XV
extern void XvExtensionInit(INITARGS);
extern void XvMCExtensionInit(INITARGS);
#endif
#ifdef XSYNC
extern void SyncExtensionInit(INITARGS);
#endif
#ifdef XKB
extern void XkbExtensionInit(INITARGS);
#endif
#ifdef XCMISC
extern void XCMiscExtensionInit(INITARGS);
#endif
#ifdef XRECORD
extern void RecordExtensionInit(INITARGS);
#endif
#ifdef LBX
extern void LbxExtensionInit(INITARGS);
#endif
#ifdef DBE
extern void DbeExtensionInit(INITARGS);
#endif
#ifdef XAPPGROUP
extern void XagExtensionInit(INITARGS);
#endif
#ifdef XCSECURITY
extern void SecurityExtensionInit(INITARGS);
#endif
#ifdef XPRINT
extern void XpExtensionInit(INITARGS);
#endif
#ifdef XF86BIGFONT
extern void XFree86BigfontExtensionInit(INITARGS);
#endif
#ifdef XF86VIDMODE
extern void XFree86VidModeExtensionInit(INITARGS);
#endif
#ifdef XF86MISC
extern void XFree86MiscExtensionInit(INITARGS);
#endif
#ifdef XFreeXDGA
extern void XFree86DGAExtensionInit(INITARGS);
#endif
#ifdef GLXEXT
#ifndef __DARWIN__
extern void GlxExtensionInit(INITARGS);
extern void GlxWrapInitVisuals(miInitVisualsProcPtr *);
#else
extern void DarwinGlxExtensionInit(INITARGS);
extern void DarwinGlxWrapInitVisuals(miInitVisualsProcPtr *);
#endif
#endif
#ifdef XF86DRI
extern void XFree86DRIExtensionInit(INITARGS);
#endif
#ifdef TOGCUP
extern void XcupExtensionInit(INITARGS);
#endif
#ifdef DPMSExtension
extern void DPMSExtensionInit(INITARGS);
#endif
#ifdef DPSEXT
extern void DPSExtensionInit(INITARGS);
#endif
#ifdef FONTCACHE
extern void FontCacheExtensionInit(INITARGS);
#endif
#ifdef RENDER
extern void RenderExtensionInit(INITARGS);
#endif
#ifdef RANDR
extern void RRExtensionInit(INITARGS);
#endif
#ifdef RES
extern void ResExtensionInit(INITARGS);
#endif
#ifdef DMXEXT
extern void DMXExtensionInit(INITARGS);
#endif
#ifdef XEVIE
extern void XevieExtensionInit(INITARGS);
#endif
#ifdef XFIXES
extern void XFixesExtensionInit(INITARGS);
#endif
#ifdef DAMAGE
extern void DamageExtensionInit(INITARGS);
#endif
#ifdef COMPOSITE
extern void CompositeExtensionInit(INITARGS);
#endif

/* The following is only a small first step towards run-time
 * configurable extensions.
 */
typedef struct {
    char *name;
    Bool *disablePtr;
} ExtensionToggle;

static ExtensionToggle ExtensionToggleList[] =
{
    /* sort order is extension name string as shown in xdpyinfo */
#ifdef BIGREQS
    { "BIG-REQUESTS", &noBigReqExtension },
#endif
#ifdef COMPOSITE
    { "Composite", &noCompositeExtension },
#endif
#ifdef DAMAGE
    { "DAMAGE", &noDamageExtension },
#endif
#ifdef DBE
    { "DOUBLE-BUFFER", &noDbeExtension },
#endif
#ifdef DPSEXT
    { "DPSExtension", &noDPSExtension },
#endif
#ifdef DPMSExtension
    { "DPMS", &noDPMSExtension },
#endif
#ifdef EVI
    { "Extended-Visual-Information", &noEVIExtension },
#endif
#ifdef FONTCACHE
    { "FontCache", &noFontCacheExtension },
#endif
#ifdef GLXEXT
    { "GLX", &noGlxExtension },
#endif
#ifdef LBX
    { "LBX", &noLbxExtension },
#endif
#ifdef SCREENSAVER
    { "MIT-SCREEN-SAVER", &noScreenSaverExtension },
#endif
#ifdef MITSHM
    { SHMNAME, &noMITShmExtension },
#endif
#ifdef MITMISC
    { "MIT-SUNDRY-NONSTANDARD", &noMITMiscExtension },
#endif
#ifdef MULTIBUFFER
    { "Multi-Buffering", &noMultibufferExtension },
#endif
#ifdef RANDR
    { "RANDR", &noRRExtension },
#endif
#ifdef RENDER
    { "RENDER", &noRenderExtension },
#endif
#ifdef SHAPE
    { "SHAPE", &noShapeExtension },
#endif
#ifdef XCSECURITY
    { "SECURITY", &noSecurityExtension },
#endif
#ifdef XSYNC
    { "SYNC", &noSyncExtension },
#endif
#ifdef TOGCUP
    { "TOG-CUP", &noXcupExtension },
#endif
#ifdef RES
    { "X-Resource", &noResExtension },
#endif
#ifdef XAPPGROUP
    { "XC-APPGROUP", &noXagExtension },
#endif
#ifdef XCMISC
    { "XC-MISC", &noXCMiscExtension },
#endif
#ifdef XEVIE
    { "XEVIE", &noXevieExtension },
#endif
#ifdef XF86BIGFONT
    { "XFree86-Bigfont", &noXFree86BigfontExtension },
#endif
#ifdef XFreeXDGA
    { "XFree86-DGA", &noXFree86DGAExtension },
#endif
#ifdef XF86DRI
    { "XFree86-DRI", &noXFree86DRIExtension },
#endif
#ifdef XF86MISC
    { "XFree86-Misc", &noXFree86MiscExtension },
#endif
#ifdef XF86VIDMODE
    { "XFree86-VidModeExtension", &noXFree86VidModeExtension },
#endif
#ifdef XFIXES
    { "XFIXES", &noXFixesExtension },
#endif
#ifdef PANORAMIX
    { "XINERAMA", &noPanoramiXExtension },
#endif
#ifdef XINPUT
    { "XInputExtension", &noXInputExtension },
#endif
#ifdef XKB
    { "XKEYBOARD", &noXkbExtension },
#endif
    { "XTEST", &noTestExtensions },
#ifdef XV
    { "XVideo", &noXvExtension },
#endif
    { NULL, NULL }
};

Bool EnableDisableExtension(char *name, Bool enable)
{
    ExtensionToggle *ext = &ExtensionToggleList[0];

    for (ext = &ExtensionToggleList[0]; ext->name != NULL; ext++) {
	if (strcmp(name, ext->name) == 0) {
	    *ext->disablePtr = !enable;
	    return TRUE;
	}
    }

    return FALSE;
}

void EnableDisableExtensionError(char *name, Bool enable)
{
    ExtensionToggle *ext = &ExtensionToggleList[0];

    ErrorF("Extension \"%s\" is not recognized\n", name);
    ErrorF("Only the following extensions can be run-time %s:\n",
	   enable ? "enabled" : "disabled");
    for (ext = &ExtensionToggleList[0]; ext->name != NULL; ext++)
	ErrorF("    %s\n", ext->name);
}

#ifndef XFree86LOADER

/*ARGSUSED*/
void
InitExtensions(argc, argv)
    int		argc;
    char	*argv[];
{
#ifdef PANORAMIX
# if !defined(PRINT_ONLY_SERVER) && !defined(NO_PANORAMIX)
  if (!noPanoramiXExtension) PanoramiXExtensionInit();
# endif
#endif
#ifdef XTESTEXT1
    if (!noTestExtensions) XTestExtension1Init();
#endif
#ifdef SHAPE
    if (!noShapeExtension) ShapeExtensionInit();
#endif
#ifdef MITSHM
    if (!noMITShmExtension) ShmExtensionInit();
#endif
#ifdef EVI
    if (!noEVIExtension) EVIExtensionInit();
#endif
#ifdef MULTIBUFFER
    if (!noMultibufferExtension) MultibufferExtensionInit();
#endif
#if defined(XINPUT) && !defined(NO_HW_ONLY_EXTS)
    if (!noXInputExtension) XInputExtensionInit();
#endif
#ifdef XTEST
    if (!noTestExtensions) XTestExtensionInit();
#endif
#ifdef BIGREQS
    if (!noBigReqExtension) BigReqExtensionInit();
#endif
#ifdef MITMISC
    if (!noMITMiscExtension) MITMiscExtensionInit();
#endif
#ifdef XIDLE
    if (!noXIdleExtension) XIdleExtensionInit();
#endif
#ifdef XTRAP
    if (!noTestExtensions) DEC_XTRAPInit();
#endif
#if defined(SCREENSAVER) && !defined(PRINT_ONLY_SERVER)
    if (!noScreenSaverExtension) ScreenSaverExtensionInit ();
#endif
#ifdef XV
    if (!noXvExtension) {
      XvExtensionInit();
      XvMCExtensionInit();
    }
#endif
#ifdef XSYNC
    if (!noSyncExtension) SyncExtensionInit();
#endif
#if defined(XKB) && !defined(PRINT_ONLY_SERVER) && !defined(NO_HW_ONLY_EXTS)
    if (!noXkbExtension) XkbExtensionInit();
#endif
#ifdef XCMISC
    if (!noXCMiscExtension) XCMiscExtensionInit();
#endif
#ifdef XRECORD
    if (!noTestExtensions) RecordExtensionInit(); 
#endif
#ifdef LBX
    if (!noLbxExtension) LbxExtensionInit();
#endif
#ifdef DBE
    if (!noDbeExtension) DbeExtensionInit();
#endif
#ifdef XAPPGROUP
    if (!noXagExtension) XagExtensionInit();
#endif
#ifdef XCSECURITY
    if (!noSecurityExtension) SecurityExtensionInit();
#endif
#ifdef XPRINT
    XpExtensionInit(); /* server-specific extension, cannot be disabled */
#endif
#ifdef TOGCUP
    if (!noXcupExtension) XcupExtensionInit();
#endif
#if defined(DPMSExtension) && !defined(NO_HW_ONLY_EXTS)
    if (!noDPMSExtension) DPMSExtensionInit();
#endif
#ifdef FONTCACHE
    if (!noFontCacheExtension) FontCacheExtensionInit();
#endif
#ifdef XF86BIGFONT
    if (!noXFree86BigfontExtension) XFree86BigfontExtensionInit();
#endif
#if !defined(PRINT_ONLY_SERVER) && !defined(NO_HW_ONLY_EXTS)
#if defined(XF86VIDMODE)
    if (!noXFree86VidModeExtension) XFree86VidModeExtensionInit();
#endif
#if defined(XF86MISC)
    if (!noXFree86MiscExtension) XFree86MiscExtensionInit();
#endif
#if defined(XFreeXDGA)
    if (!noXFree86DGAExtension) XFree86DGAExtensionInit();
#endif
#ifdef XF86DRI
    if (!noXFree86DRIExtension) XFree86DRIExtensionInit();
#endif
#endif
#ifdef GLXEXT
#ifndef __DARWIN__
    if (!noGlxExtension) GlxExtensionInit();
#else
    if (!noGlxExtension) DarwinGlxExtensionInit();
#endif
#endif
#ifdef DPSEXT
#ifndef XPRINT
    if (!noDPSExtension) DPSExtensionInit();
#endif
#endif
#ifdef XFIXES
    /* must be before Render to layer DisplayCursor correctly */
    if (!noXFixesExtension) XFixesExtensionInit();
#endif
#ifdef RENDER
    if (!noRenderExtension) RenderExtensionInit();
#endif
#ifdef RANDR
    if (!noRRExtension) RRExtensionInit();
#endif
#ifdef RES
    if (!noResExtension) ResExtensionInit();
#endif
#ifdef DMXEXT
    DMXExtensionInit(); /* server-specific extension, cannot be disabled */
#endif
#ifdef XEVIE
    if (!noXevieExtension) XevieExtensionInit();
#endif
#ifdef COMPOSITE
    if (!noCompositeExtension) CompositeExtensionInit();
#endif
#ifdef DAMAGE
    if (!noDamageExtension) DamageExtensionInit();
#endif
}

void
InitVisualWrap()
{
    miResetInitVisuals();
#ifdef GLXEXT
#ifndef __DARWIN__
    GlxWrapInitVisuals(&miInitVisualsProc);
#else
    DarwinGlxWrapInitVisuals(&miInitVisualsProc);
#endif
#endif
}

#else /* XFree86LOADER */
/* List of built-in (statically linked) extensions */
static ExtensionModule staticExtensions[] = {
#ifdef XTESTEXT1
    { XTestExtension1Init, "XTEST1", &noTestExtensions, NULL, NULL },
#endif
#ifdef MITSHM
    { ShmExtensionInit, SHMNAME, &noMITShmExtension, NULL, NULL },
#endif
#ifdef XINPUT
    { XInputExtensionInit, "XInputExtension", &noXInputExtension, NULL, NULL },
#endif
#ifdef XTEST
    { XTestExtensionInit, XTestExtensionName, &noTestExtensions, NULL, NULL },
#endif
#ifdef XIDLE
    { XIdleExtensionInit, "XIDLE", &noXIdleExtension, NULL, NULL },
#endif
#ifdef XKB
    { XkbExtensionInit, XkbName, &noXkbExtension, NULL, NULL },
#endif
#ifdef LBX
    { LbxExtensionInit, LBXNAME, &noLbxExtension, NULL, NULL },
#endif
#ifdef XAPPGROUP
    { XagExtensionInit, XAGNAME, &noXagExtension, NULL, NULL },
#endif
#ifdef XCSECURITY
    { SecurityExtensionInit, SECURITY_EXTENSION_NAME, &noSecurityExtension, NULL, NULL },
#endif
#ifdef XPRINT
    { XpExtensionInit, XP_PRINTNAME, NULL, NULL, NULL },
#endif
#ifdef PANORAMIX
    { PanoramiXExtensionInit, PANORAMIX_PROTOCOL_NAME, &noPanoramiXExtension, NULL, NULL },
#endif
#ifdef XFIXES
    /* must be before Render to layer DisplayCursor correctly */
    { XFixesExtensionInit, "XFIXES", &noXFixesExtension, NULL, NULL },
#endif
#ifdef XF86BIGFONT
    { XFree86BigfontExtensionInit, XF86BIGFONTNAME, &noXFree86BigfontExtension, NULL, NULL },
#endif
#ifdef RENDER
    { RenderExtensionInit, "RENDER", &noRenderExtension, NULL, NULL },
#endif
#ifdef RANDR
    { RRExtensionInit, "RANDR", &noRRExtension, NULL, NULL },
#endif
#ifdef COMPOSITE
    { CompositeExtensionInit, "COMPOSITE", &noCompositeExtension, NULL },
#endif
#ifdef DAMAGE
    { DamageExtensionInit, "DAMAGE", &noDamageExtension, NULL },
#endif
#ifdef XEVIE
    { XevieExtensionInit, "XEVIE", &noXevieExtension, NULL },
#endif 
    { NULL, NULL, NULL, NULL, NULL }
};
    
/*ARGSUSED*/
void
InitExtensions(argc, argv)
    int		argc;
    char	*argv[];
{
    int i;
    ExtensionModule *ext;
    static Bool listInitialised = FALSE;

    if (!listInitialised) {
	/* Add built-in extensions to the list. */
	for (i = 0; staticExtensions[i].name; i++)
	    LoadExtension(&staticExtensions[i], TRUE);

	/* Sort the extensions according the init dependencies. */
	LoaderSortExtensions();
	listInitialised = TRUE;
    }

    for (i = 0; ExtensionModuleList[i].name != NULL; i++) {
	ext = &ExtensionModuleList[i];
	if (ext->initFunc != NULL && 
	    (ext->disablePtr == NULL || 
	     (ext->disablePtr != NULL && !*ext->disablePtr))) {
	    (ext->initFunc)();
	}
    }
}

static void (*__miHookInitVisualsFunction)(miInitVisualsProcPtr *);

void
InitVisualWrap()
{
    miResetInitVisuals();
    if (__miHookInitVisualsFunction)
	(*__miHookInitVisualsFunction)(&miInitVisualsProc);
}

void miHookInitVisuals(void (**old)(miInitVisualsProcPtr *),
		       void (*new)(miInitVisualsProcPtr *))
{
    if (old)
	*old = __miHookInitVisualsFunction;
    __miHookInitVisualsFunction = new;
}

#endif /* XFree86LOADER */
