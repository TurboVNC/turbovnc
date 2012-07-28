/* $XFree86: xc/programs/Xserver/mi/miinitext.c,v 3.68 2003/01/15 02:34:14 torrey Exp $ */
/***********************************************************

Copyright 1987, 1998  The Open Group
Copyright 2012  D. R. Commander

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

#include "misc.h"
#include "extension.h"
#include "micmap.h"

#ifdef NOPEXEXT /* sleaze for Solaris cpp building XsunMono */
#undef PEXEXT
#endif

extern Bool noTestExtensions;
#ifdef XKB
extern Bool noXkbExtension;
#endif
#ifdef RENDER
extern Bool noRenderExtension;
#endif

#if NeedFunctionPrototypes
#define INITARGS void
#else
#define INITARGS /*nothing*/
#endif
typedef void (*InitExtension)(INITARGS);

/* FIXME: this whole block of externs should be from the appropriate headers */
#ifdef BEZIER
extern void BezierExtensionInit(INITARGS);
#endif
#ifdef XTESTEXT1
extern void XTestExtension1Init(INITARGS);
#endif
#ifdef SHAPE
extern void ShapeExtensionInit(INITARGS);
#endif
#ifdef MITSHM
extern void ShmExtensionInit(INITARGS);
#endif
#ifdef PEXEXT
#ifndef PEX_MODULE
extern void PexExtensionInit(INITARGS);
#endif
InitExtension PexExtensionInitPtr = NULL;
#endif
#ifdef MULTIBUFFER
extern void MultibufferExtensionInit(INITARGS);
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
#endif
#ifdef XIE
#ifndef XIE_MODULE
extern void XieInit(INITARGS);
#endif
InitExtension XieInitPtr = NULL;
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
extern void     LbxExtensionInit(INITARGS);
#endif
#ifdef DBE
extern void     DbeExtensionInit(INITARGS);
#endif
#ifdef XAPPGROUP
extern void XagExtensionInit(INITARGS);
#endif
#ifdef XCSECURITY
extern void SecurityExtensionInit(INITARGS);
#endif
#ifdef XPRINT
extern void	XpExtensionInit(INITARGS);
#endif
#ifdef XF86VIDMODE
extern void	XFree86VidModeExtensionInit(INITARGS);
#endif
#ifdef XF86MISC
extern void	XFree86MiscExtensionInit(INITARGS);
#endif
#ifdef XFreeXDGA
extern void XFree86DGAExtensionInit(INITARGS);
#endif
#ifdef DPMSExtension
extern void DPMSExtensionInit(INITARGS);
#endif
#ifdef GLXEXT
#ifndef GLX_MODULE
extern void GlxExtensionInit(INITARGS);
#else
InitExtension GlxExtensionInitPtr = NULL;
#endif
#endif
#ifdef RENDER
extern void RenderExtensionInit(INITARGS);
#endif
extern void vncExtensionInit(INITARGS);

/*ARGSUSED*/
void
InitExtensions(argc, argv)
    int		argc;
    char	*argv[];
{
#ifdef BEZIER
    BezierExtensionInit();
#endif
#ifdef XTESTEXT1
    if (!noTestExtensions) XTestExtension1Init();
#endif
#ifdef SHAPE
    ShapeExtensionInit();
#endif
#ifdef MITSHM
    ShmExtensionInit();
#endif
#ifdef PEXEXT
#ifndef PEX_MODULE
    PexExtensionInit();
#else
    if (PexExtensionInitPtr != NULL) {
	(*PexExtensionInitPtr)();
    }
#endif
#endif
#ifdef MULTIBUFFER
    MultibufferExtensionInit();
#endif
#ifdef XINPUT
    XInputExtensionInit();
#endif
#ifdef XTEST
    if (!noTestExtensions) XTestExtensionInit();
#endif
#ifdef BIGREQS
    BigReqExtensionInit();
#endif
#ifdef MITMISC
    MITMiscExtensionInit();
#endif
#ifdef XIDLE
    XIdleExtensionInit();
#endif
#ifdef XTRAP
    if (!noTestExtensions) DEC_XTRAPInit();
#endif
#ifdef SCREENSAVER
    ScreenSaverExtensionInit ();
#endif
#ifdef XV
    XvExtensionInit();
#endif
#ifdef XIE
#ifndef XIE_MODULE
    XieInit();
#else
    if (XieInitPtr != NULL) {
	(*XieInitPtr)();
    }
#endif
#endif
#ifdef XSYNC
    SyncExtensionInit();
#endif
#ifdef XKB
    if (!noXkbExtension) XkbExtensionInit();
#endif
#ifdef XCMISC
    XCMiscExtensionInit();
#endif
#ifdef XRECORD
    if (!noTestExtensions) RecordExtensionInit(); 
#endif
#ifdef LBX
    LbxExtensionInit();
#endif
#ifdef DBE
    DbeExtensionInit();
#endif
#ifdef XAPPGROUP
    XagExtensionInit();
#endif
#ifdef XCSECURITY
    SecurityExtensionInit();
#endif
#ifdef XPRINT
    XpExtensionInit();
#endif
#if defined(XF86VIDMODE) && !defined(PRINT_ONLY_SERVER)
    XFree86VidModeExtensionInit();
#endif
#if defined(XF86MISC) && !defined(PRINT_ONLY_SERVER)
    XFree86MiscExtensionInit();
#endif
#if defined(XFreeXDGA) && !defined(PRINT_ONLY_SERVER)
    XFree86DGAExtensionInit();
#endif
#if defined(DPMSExtension) && !defined(PRINT_ONLY_SERVER)
    DPMSExtensionInit();
#endif
#ifdef GLXEXT
#ifndef GLX_MODULE
    GlxExtensionInit();
#else
    if (GlxExtensionInitPtr != NULL) {
        (*GlxExtensionInitPtr)();
    }
#endif
#endif
#ifdef RENDER
    if (!noRenderExtension) RenderExtensionInit();
#endif
    vncExtensionInit();
}
