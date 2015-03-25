/*  Copyright (C)2015 D. R. Commander.  All Rights Reserved.
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 *  USA.
 */

#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include "jawt_md.h"
#include "com_turbovnc_vncviewer_Viewport.h"

/*
 * These functions are borrowed from OpenSUSE.  They give the window manager a
 * hint that the window is full-screen so the taskbar won't appear on top
 * of it.
 */

#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */

static void netwm_set_state(Display *dpy, Window win, int operation,
                            Atom state)
{
  XEvent e;
  Atom _NET_WM_STATE = XInternAtom(dpy, "_NET_WM_STATE", False);

  memset(&e, 0, sizeof(e));
  e.xclient.type = ClientMessage;
  e.xclient.message_type = _NET_WM_STATE;
  e.xclient.display = dpy;
  e.xclient.window = win;
  e.xclient.format = 32;
  e.xclient.data.l[0] = operation;
  e.xclient.data.l[1] = state;

  XSendEvent(dpy, DefaultRootWindow(dpy), False, SubstructureRedirectMask, &e);
}

static void netwm_fullscreen(Display *dpy, Window win, int state)
{
  Atom _NET_WM_STATE_FULLSCREEN =
    XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
  int op = state ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
  netwm_set_state(dpy, win, op, _NET_WM_STATE_FULLSCREEN);
}

#define _throw(msg) {  \
  jclass _exccls=(*env)->FindClass(env, "java/lang/Exception");  \
  if(!_exccls) goto bailout;  \
  (*env)->ThrowNew(env, _exccls, msg);  \
  goto bailout;  \
}

typedef jboolean JNICALL (*__JAWT_GetAWT_type)(JNIEnv* env, JAWT* awt);
static __JAWT_GetAWT_type __JAWT_GetAWT = NULL;

static void *handle = NULL;

JNIEXPORT void JNICALL Java_com_turbovnc_vncviewer_Viewport_x11FullScreen
  (JNIEnv *env, jobject obj, jboolean on)
{
  JAWT awt;
  JAWT_DrawingSurface *ds = NULL;
  JAWT_DrawingSurfaceInfo *dsi = NULL;
  JAWT_X11DrawingSurfaceInfo *x11dsi = NULL;

  awt.version = JAWT_VERSION_1_3;
  if (!handle) {
    if ((handle = dlopen("libjawt.so", RTLD_LAZY)) == NULL)
      _throw(dlerror());
    if ((__JAWT_GetAWT = dlsym(handle, "JAWT_GetAWT")) == NULL)
      _throw(dlerror());
  }

  if(__JAWT_GetAWT(env, &awt) == JNI_FALSE)
    _throw("Could not initialize AWT native interface");

  if((ds = awt.GetDrawingSurface(env, obj)) == NULL)
    _throw("Could not get drawing surface");

  if((ds->Lock(ds) & JAWT_LOCK_ERROR) != 0)
    _throw("Could not lock surface");

  if((dsi = ds->GetDrawingSurfaceInfo(ds)) == NULL)
    _throw("Could not get drawing surface info");

  if((x11dsi = (JAWT_X11DrawingSurfaceInfo*)dsi->platformInfo) == NULL)
    _throw("Could not get X11 drawing surface info");

  netwm_fullscreen(x11dsi->display, x11dsi->drawable, on);
  XSync(x11dsi->display, False);

  printf("TurboVNC Helper: %s X11 full-screen mode for window 0x%.8lx\n",
         on ? "Enabling" : "Disabling", x11dsi->drawable);

  bailout:
  if(ds)
  {
    if(dsi) ds->FreeDrawingSurfaceInfo(dsi);
    ds->Unlock(ds);
    awt.FreeDrawingSurface(ds);
  }
}

JNIEXPORT void JNICALL Java_com_turbovnc_vncviewer_Viewport_grabKeyboard
  (JNIEnv *env, jobject obj, jboolean on, jboolean pointer)
{
  JAWT awt;
  JAWT_DrawingSurface *ds = NULL;
  JAWT_DrawingSurfaceInfo *dsi = NULL;
  JAWT_X11DrawingSurfaceInfo *x11dsi = NULL;
  int ret;

  awt.version = JAWT_VERSION_1_3;
  if (!handle) {
    if ((handle = dlopen("libjawt.so", RTLD_LAZY)) == NULL)
      _throw(dlerror());
    if ((__JAWT_GetAWT = dlsym(handle, "JAWT_GetAWT")) == NULL)
      _throw(dlerror());
  }

  if(__JAWT_GetAWT(env, &awt) == JNI_FALSE)
    _throw("Could not initialize AWT native interface");

  if((ds = awt.GetDrawingSurface(env, obj)) == NULL)
    _throw("Could not get drawing surface");

  if((ds->Lock(ds) & JAWT_LOCK_ERROR) != 0)
    _throw("Could not lock surface");

  if((dsi = ds->GetDrawingSurfaceInfo(ds)) == NULL)
    _throw("Could not get drawing surface info");

  if((x11dsi = (JAWT_X11DrawingSurfaceInfo*)dsi->platformInfo) == NULL)
    _throw("Could not get X11 drawing surface info");

  XSync(x11dsi->display, False);
  if (on) {
    int count = 5;
    while ((ret = XGrabKeyboard(x11dsi->display, x11dsi->drawable, True,
                                GrabModeAsync, GrabModeAsync, CurrentTime))
           != GrabSuccess) {
      switch (ret) {
        case AlreadyGrabbed:
          _throw("Could not grab keyboard: already grabbed by another application");
        case GrabInvalidTime:
          _throw("Could not grab keyboard: invalid time");
        case GrabNotViewable:
          /* The window should theoretically be viewable by now, but in
             practice, sometimes a race condition occurs with Swing.  It is
             unclear why, since everything should be happening in the EDT. */
          if (count == 0)
            _throw("Could not grab keyboard: window not viewable");
          usleep(100000);
          count--;
          continue;
        case GrabFrozen:
          _throw("Could not grab keyboard: keyboard frozen by another application");
      }
    }

    if (pointer) {
      ret = XGrabPointer(x11dsi->display, x11dsi->drawable, True,
                         ButtonPressMask | ButtonReleaseMask |
                         ButtonMotionMask | PointerMotionMask, GrabModeAsync,
                         GrabModeAsync, None, None, CurrentTime);
      switch (ret) {
        case AlreadyGrabbed:
          _throw("Could not grab pointer: already grabbed by another application");
        case GrabInvalidTime:
          _throw("Could not grab pointer: invalid time");
        case GrabNotViewable:
          _throw("Could not grab pointer: window not viewable");
        case GrabFrozen:
          _throw("Could not grab pointer: pointer frozen by another application");
      }
    }

    printf("TurboVNC Helper: Grabbed keyboard%s for window 0x%.8lx\n",
           pointer? " & pointer" : "", x11dsi->drawable);
  } else {
    XUngrabKeyboard(x11dsi->display, CurrentTime);
    if (pointer)
      XUngrabPointer(x11dsi->display, CurrentTime);
    printf("TurboVNC Helper: Ungrabbed keyboard%s\n",
           pointer ? " & pointer" : "");
  }
  XSync(x11dsi->display, False);

  bailout:
  if(ds)
  {
    if(dsi) ds->FreeDrawingSurfaceInfo(dsi);
    ds->Unlock(ds);
    awt.FreeDrawingSurface(ds);
  }
}
