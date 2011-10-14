/* $XFree86: xc/programs/Xserver/GL/glx/glxscreens.c,v 1.14 2004/02/09 23:46:31 alanh Exp $ */
/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
** 
** http://oss.sgi.com/projects/FreeB
** 
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
** 
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
** 
** Additional Notice Provisions: The application programming interfaces
** established by SGI in conjunction with the Original Code are The
** OpenGL(R) Graphics System: A Specification (Version 1.2.1), released
** April 1, 1999; The OpenGL(R) Graphics System Utility Library (Version
** 1.3), released November 4, 1998; and OpenGL(R) Graphics with the X
** Window System(R) (Version 1.3), released October 19, 1998. This software
** was created using the OpenGL(R) version 1.2.1 Sample Implementation
** published by SGI, but has not been independently verified as being
** compliant with the OpenGL(R) version 1.2.1 Specification.
**
*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifdef IN_MODULE
#include <xf86_ansic.h>
#else
#include <string.h>
#include <signal.h>
#endif

#include <windowstr.h>

#include "glxserver.h"
#include "glxutil.h"
#include "glxext.h"

const char GLServerVersion[] = "1.2";
static const char GLServerExtensions[] = 
			"GL_ARB_depth_texture "
			"GL_ARB_imaging "
			"GL_ARB_multitexture "
			"GL_ARB_point_parameters "
			"GL_ARB_point_sprite "
			"GL_ARB_shadow "
			"GL_ARB_shadow_ambient "
			"GL_ARB_texture_border_clamp "
			"GL_ARB_texture_cube_map "
			"GL_ARB_texture_env_add "
			"GL_ARB_texture_env_combine "
			"GL_ARB_texture_env_crossbar "
			"GL_ARB_texture_env_dot3 "
			"GL_ARB_texture_mirrored_repeat "
			"GL_ARB_transpose_matrix "
			"GL_ARB_window_pos "
			"GL_EXT_abgr "
			"GL_EXT_bgra "
 			"GL_EXT_blend_color "
			"GL_EXT_blend_func_separate "
			"GL_EXT_blend_logic_op "
 			"GL_EXT_blend_minmax "
 			"GL_EXT_blend_subtract "
			"GL_EXT_clip_volume_hint "
			"GL_EXT_copy_texture "
			"GL_EXT_draw_range_elements "
			"GL_EXT_fog_coord "
			"GL_EXT_multi_draw_arrays "
			"GL_EXT_packed_pixels "
			"GL_EXT_polygon_offset "
			"GL_EXT_rescale_normal "
			"GL_EXT_secondary_color "
			"GL_EXT_separate_specular_color "
			"GL_EXT_shadow_funcs "
 			"GL_EXT_stencil_two_side "
			"GL_EXT_stencil_wrap "
			"GL_EXT_subtexture "
			"GL_EXT_texture "
			"GL_EXT_texture3D "
			"GL_EXT_texture_edge_clamp "
 			"GL_EXT_texture_env_add "
 			"GL_EXT_texture_env_combine "
 			"GL_EXT_texture_env_dot3 "
			"GL_EXT_texture_lod "
 			"GL_EXT_texture_lod_bias "
			"GL_EXT_texture_object "
			"GL_EXT_texture_rectangle "
			"GL_EXT_vertex_array "
			"GL_APPLE_packed_pixels "
			"GL_ATI_texture_mirror_once "
			"GL_ATI_texture_env_combine3 "
 			"GL_HP_occlusion_test "
			"GL_IBM_texture_mirrored_repeat "
			"GL_MESA_pack_invert "
			"GL_MESA_ycbcr_texture "
			"GL_NV_blend_square "
			"GL_NV_point_sprite "
			"GL_NV_texgen_reflection "
			"GL_NV_texture_rectangle "
			"GL_SGIS_generate_mipmap "
			"GL_SGIS_texture_border_clamp "
			"GL_SGIS_texture_edge_clamp "
			"GL_SGIS_texture_lod "
			"GL_SGIX_depth_texture "
			"GL_SGIX_shadow "
			"GL_SGIX_shadow_ambient "
			;

/*
** We have made the simplifying assuption that the same extensions are 
** supported across all screens in a multi-screen system.
*/
static char GLXServerVendorName[] = "SGI";
static char GLXServerVersion[] = "1.2";
static char GLXServerExtensions[] =
			"GLX_ARB_multisample "
			"GLX_EXT_visual_info "
			"GLX_EXT_visual_rating "
			"GLX_EXT_import_context "
			"GLX_OML_swap_method "
			"GLX_SGI_make_current_read "
#ifndef __DARWIN__
			"GLX_SGIS_multisample "
                        "GLX_SGIX_hyperpipe "
                        "GLX_SGIX_swap_barrier "
#endif
			"GLX_SGIX_fbconfig "
			;

/*
 * __glDDXScreenInfo comes from GLcore, so we can't resolve this symbol at
 * module open time.  Leave a placeholder, and fill this in when we first
 * need it (in __glXScreenInit).  XXX Why make this an array?
 */
static __GLXscreenInfo *__glXScreens[] = {
    NULL /* &__glDDXScreenInfo */ ,
};

static GLint __glXNumStaticScreens =
	(sizeof __glXScreens / sizeof __glXScreens[0]);

__GLXscreenInfo *__glXActiveScreens;
GLint __glXNumActiveScreens;

__GLXSwapBarrierExtensionFuncs *__glXSwapBarrierFuncs = NULL;
static int __glXNumSwapBarrierFuncs = 0;
__GLXHyperpipeExtensionFuncs *__glXHyperpipeFuncs = NULL;
static int __glXNumHyperpipeFuncs = 0;


RESTYPE __glXDrawableRes;

__GLXscreenInfo *__glXgetActiveScreen(int num) {
	return &__glXActiveScreens[num];
}

/*
** Destroy routine that gets called when a drawable is freed.  A drawable
** contains the ancillary buffers needed for rendering.
*/
static Bool DrawableGone(__GLXdrawablePrivate *glxPriv, XID xid)
{
    __GLXcontext *cx, *cx1;

    /*
    ** Use glxPriv->type to figure out what kind of drawable this is. Don't
    ** use glxPriv->pDraw->type because by the time this routine is called,
    ** the pDraw might already have been freed.
    */
    if (glxPriv->type == DRAWABLE_WINDOW) {
	/*
	** When a window is destroyed, notify all context bound to 
	** it, that there are no longer bound to anything.
	*/
	for (cx = glxPriv->drawGlxc; cx; cx = cx1) {
	    cx1 = cx->nextDrawPriv;
	    cx->pendingState |= __GLX_PENDING_DESTROY;
	}

	for (cx = glxPriv->readGlxc; cx; cx = cx1) {
	    cx1 = cx->nextReadPriv;
	    cx->pendingState |= __GLX_PENDING_DESTROY;
	}
    }

    /*
    ** set the size to 0, so that context that may still be using this 
    ** drawable not do anything harmful
    */
    glxPriv->xorigin = 0;
    glxPriv->yorigin = 0;
    glxPriv->width = 0;
    glxPriv->height = 0;

    __glXUnrefDrawablePrivate(glxPriv);

    return True;
}

/*
** This hook gets called when a window moves or changes size.
*/
static Bool PositionWindow(WindowPtr pWin, int x, int y)
{
    ScreenPtr pScreen;
    __GLXcontext *glxc;
    __GLXdrawablePrivate *glxPriv;
    Bool ret;

    /*
    ** Call wrapped position window routine
    */
    pScreen = pWin->drawable.pScreen;
    pScreen->PositionWindow =
	__glXActiveScreens[pScreen->myNum].WrappedPositionWindow;
    ret = (*pScreen->PositionWindow)(pWin, x, y);
    pScreen->PositionWindow = PositionWindow;

    /*
    ** Tell all contexts rendering into this window that the window size
    ** has changed.
    */
    glxPriv = (__GLXdrawablePrivate *) LookupIDByType(pWin->drawable.id,
						      __glXDrawableRes);
    if (glxPriv == NULL) {
	/*
	** This window is not being used by the OpenGL.
	*/
	return ret;
    }

    /*
    ** resize the drawable
    */
    /* first change the drawable size */
    if (__glXResizeDrawableBuffers(glxPriv) == GL_FALSE) {
	/* resize failed! */
	/* XXX: what can we possibly do here? */
	ret = False;
    }

    /* mark contexts as needing resize */

    for (glxc = glxPriv->drawGlxc; glxc; glxc = glxc->nextDrawPriv) {
	glxc->pendingState |= __GLX_PENDING_RESIZE;
    }

    for (glxc = glxPriv->readGlxc; glxc; glxc = glxc->nextReadPriv) {
	glxc->pendingState |= __GLX_PENDING_RESIZE;
    }

    return ret;
}

/*
** Wrap our own PositionWindow routine around the server's, so we can
** be notified when a window changes size
*/
static void wrapPositionWindow(int screen)
{
    ScreenPtr pScreen = screenInfo.screens[screen];

    __glXActiveScreens[screen].WrappedPositionWindow = pScreen->PositionWindow;
    pScreen->PositionWindow = PositionWindow;
}

/*
 * If your DDX driver wants to register support for swap barriers or hyperpipe
 * topology, it should call __glXHyperpipeInit() or __glXSwapBarrierInit()
 * with a dispatch table of functions to handle the requests.   In the XFree86
 * DDX, for example, you would call these near the bottom of the driver's
 * ScreenInit method, after DRI has been initialized.
 *
 * This should be replaced with a better method when we teach the server how
 * to load DRI drivers.
 */

void __glXHyperpipeInit(int screen, __GLXHyperpipeExtensionFuncs *funcs)
{
    if (__glXNumHyperpipeFuncs < screen + 1) {
        __glXHyperpipeFuncs = __glXRealloc(__glXHyperpipeFuncs,
                                           (screen+1) * sizeof(__GLXHyperpipeExtensionFuncs));
        __glXNumHyperpipeFuncs = screen + 1;
    }

    __glXHyperpipeFuncs[screen].queryHyperpipeNetworkFunc =
        *funcs->queryHyperpipeNetworkFunc;
    __glXHyperpipeFuncs[screen].queryHyperpipeConfigFunc =
        *funcs->queryHyperpipeConfigFunc;
    __glXHyperpipeFuncs[screen].destroyHyperpipeConfigFunc =
        *funcs->destroyHyperpipeConfigFunc;
    __glXHyperpipeFuncs[screen].hyperpipeConfigFunc =
        *funcs->hyperpipeConfigFunc;
}

void __glXSwapBarrierInit(int screen, __GLXSwapBarrierExtensionFuncs *funcs)
{
    if (__glXNumSwapBarrierFuncs < screen + 1) {
        __glXSwapBarrierFuncs = __glXRealloc(__glXSwapBarrierFuncs,
                                           (screen+1) * sizeof(__GLXSwapBarrierExtensionFuncs));
        __glXNumSwapBarrierFuncs = screen + 1;
    }

    __glXSwapBarrierFuncs[screen].bindSwapBarrierFunc =
        funcs->bindSwapBarrierFunc;
    __glXSwapBarrierFuncs[screen].queryMaxSwapBarriersFunc =
        funcs->queryMaxSwapBarriersFunc;
}

void __glXScreenInit(GLint numscreens)
{
    GLint i,j;

    __glXScreens[0] = __glXglDDXScreenInfo(); /* from GLcore */

    /*
    ** This alloc has to work or else the server might as well core dump.
    */
    __glXActiveScreens =
      (__GLXscreenInfo *) __glXMalloc(sizeof(__GLXscreenInfo) * numscreens);
    
    for (i=0; i < numscreens; i++) {
	/*
	** Probe each static screen to see which exists.
	*/
	for (j=0; j < __glXNumStaticScreens; j++) {
	    if ((*__glXScreens[j]->screenProbe)(i)) {
		__glXActiveScreens[i] = *__glXScreens[j];

		__glXActiveScreens[i].numUsableVisuals = __glXActiveScreens[i].numVisuals;
		__glXActiveScreens[i].GLextensions = __glXStrdup(GLServerExtensions);
		__glXActiveScreens[i].GLXvendor = __glXStrdup(GLXServerVendorName);
		__glXActiveScreens[i].GLXversion = __glXStrdup(GLXServerVersion);
		__glXActiveScreens[i].GLXextensions = __glXStrdup(GLXServerExtensions);

		__glXDrawableRes = CreateNewResourceType((DeleteType)DrawableGone);
		wrapPositionWindow(i);
	    }
	}
    }
    __glXNumActiveScreens = numscreens;
}

void __glXScreenReset(void)
{
  int i;

  for (i = 0; i < __glXNumActiveScreens; i++) {
      __glXFree(__glXActiveScreens[i].GLXvendor);
      __glXFree(__glXActiveScreens[i].GLXversion);
      __glXFree(__glXActiveScreens[i].GLXextensions);
      __glXFree(__glXActiveScreens[i].GLextensions);
  }
  xfree(__glXActiveScreens);
  xfree(__glXHyperpipeFuncs);
  xfree(__glXSwapBarrierFuncs);
  __glXNumHyperpipeFuncs = 0;
  __glXNumSwapBarrierFuncs = 0;
  __glXHyperpipeFuncs = NULL;
  __glXSwapBarrierFuncs = NULL;
  __glXActiveScreens = NULL;
  __glXNumActiveScreens = 0;
}
