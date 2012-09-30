/*
 *  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
 *  Copyright (C) 2012 D. R. Commander.  All Rights Reserved.
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

#include <vncviewer.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/Toggle.h>


static Bool DoBumpScroll();
static void BumpScrollTimerCallback(XtPointer clientData, XtIntervalId *id);
static XtIntervalId timer;
static Bool timerSet = False;
static Bool scrollLeft, scrollRight, scrollUp, scrollDown;
static Position desktopX, desktopY;
static Dimension viewportWidth, viewportHeight;
static Dimension scrollbarWidth, scrollbarHeight;
static Bool keyboardGrabbed = False;


/*
 * These functions are borrowed from OpenSUSE.  They give the window manager a
 * hint that the window is full-screen so the taskbar won't appear on top
 * of it.
 */

#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */

static void netwm_set_state(Display *dpy, Window win, int operation, Atom state)
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


/*
 * Grab/ungrab keyboard
 */

void GrabKeyboard(void)
{
#ifndef __APPLE__
    if (keyboardGrabbed) return;
    if (XtGrabKeyboard(toplevel, True, GrabModeAsync,
                       GrabModeAsync, CurrentTime) != GrabSuccess) {
        fprintf(stderr, "XtGrabKeyboard() failed.\n");
    } else {
        keyboardGrabbed = True;
    }
#endif
}

void UngrabKeyboard(void)
{
#ifndef __APPLE__
    if (!keyboardGrabbed) return;
    XtUngrabKeyboard(toplevel, CurrentTime);
    keyboardGrabbed = False;
#endif
}

void ToggleGrabKeyboard(Widget w, XEvent *event, String *params,
                   Cardinal *num_params)
{
    if (!keyboardGrabbed) GrabKeyboard();
    else UngrabKeyboard();
}

void SetGrabKeyboardState(Widget w, XEvent *ev, String *params,
                     Cardinal *num_params)
{
  if (keyboardGrabbed)
    XtVaSetValues(w, XtNstate, True, NULL);
  else
    XtVaSetValues(w, XtNstate, False, NULL);
}


/*
 * FullScreenOn() enters full-screen mode.  It makes the toplevel window
 * unmanaged by the window manager and sets its geometry appropriately.
 *
 * We have toplevel -> form -> viewport -> desktop.  "form" must always be the
 * same size as "toplevel".  "desktop" should always be fixed at the size of
 * the VNC desktop.  Normally, "viewport" is the same size as "toplevel" (<=
 * size of "desktop"), and "viewport" deals with any difference by putting up
 * scrollbars.
 *
 * When we go into full-screen mode, we allow "viewport" and "form" to be
 * different sizes, and we effectively need to work out all the geometries
 * ourselves.  There are two cases to deal with:
 *
 * 1. When the desktop is smaller than the display, "viewport" is simply the
 *    size of the desktop, and "toplevel" (and "form") are the size of the
 *    display.  "form" is visible around the edges of the desktop.
 *
 * 2. When the desktop is bigger than the display in either or both dimensions,
 *    we force "viewport" to have scrollbars.
 *
 *    If the desktop width is bigger than the display width, then the width of
 *    "viewport" is the display width plus the scrollbar width.  Otherwise,
 *    it's the desktop width plus the scrollbar width.  The width of "toplevel"
 *    (and "form") is then either the same as "viewport", or just the display
 *    width, respectively.  Similarly for the height of "viewport" and the
 *    height of "toplevel".
 *
 *    So, if the desktop is bigger than the display in both dimensions, then
 *    both the scrollbars will be just off the screen.  If it's bigger in only
 *    one dimension, then that scrollbar _will_ be visible, with the other one
 *    just off the screen.  We treat this as a "feature" rather than a problem
 *    - you can't easily get around it if you want to use the Athena viewport
 *    for doing the scrolling.
 *
 * In either case, we position "viewport" in the middle of "form".
 *
 * We store the calculated size of "viewport" and the scrollbars in global
 * variables so that FullScreenOff() can use them.
 */

void FullScreenOn()
{
  Dimension toplevelWidth, toplevelHeight;
  Dimension oldViewportWidth, oldViewportHeight, clipWidth, clipHeight;
  Position viewportX, viewportY;

  appData.fullScreen = True;

  if (si.framebufferWidth > dpyWidth || si.framebufferHeight > dpyHeight) {

    XtVaSetValues(viewport, XtNforceBars, True, NULL);
    XtVaGetValues(viewport, XtNwidth, &oldViewportWidth,
                  XtNheight, &oldViewportHeight, NULL);
    XtVaGetValues(XtNameToWidget(viewport, "clip"),
                  XtNwidth, &clipWidth, XtNheight, &clipHeight, NULL);

    scrollbarWidth = oldViewportWidth - clipWidth;
    scrollbarHeight = oldViewportHeight - clipHeight;

    if (si.framebufferWidth > dpyWidth || si.framebufferHeight > dpyHeight) {
      viewportWidth = toplevelWidth = dpyWidth + scrollbarWidth;
      viewportHeight = toplevelHeight = dpyHeight + scrollbarHeight;
    } else {
      viewportWidth = si.framebufferWidth + scrollbarWidth;
      toplevelWidth = dpyWidth;
      viewportHeight = si.framebufferHeight + scrollbarHeight;
      toplevelHeight = dpyHeight;
    }

  } else {
    viewportWidth = si.framebufferWidth;
    viewportHeight = si.framebufferHeight;
    toplevelWidth = dpyWidth;
    toplevelHeight = dpyHeight;
  }

  viewportX = (toplevelWidth - viewportWidth) / 2;
  viewportY = (toplevelHeight - viewportHeight) / 2;

  XtVaSetValues(toplevel, XtNmaxWidth, toplevelWidth,
                XtNmaxHeight, toplevelHeight, NULL);
  netwm_fullscreen(dpy, XtWindow(toplevel), True);
  XSync(dpy, False);

  /* We want to stop the window manager from managing our toplevel window.
     This is not really a nice thing to do and may not work properly with every
     window manager.  We do this simply by setting overrideRedirect. */

  XtVaSetValues(toplevel, XtNoverrideRedirect, True, NULL);

  /* Some WMs do not obey x, y values of XReparentWindow(), and thus the window
     is not placed in the upper left corner.  The code below fixes this.  It
     manually moves the window after the Xserver is done with
     XReparentWindow().  The last XSync() seems to prevent losing focus, but I
     don't know why. */
  XSync(dpy, False);
  XMoveWindow(dpy, XtWindow(toplevel), 0, 0);
  XSync(dpy, False);

  /* Now we want to fix the size of "viewport".  We shouldn't just change it
     directly.  Instead, we set "toplevel" to the required size (which should
     propagate through "form" to "viewport".)  Then, we remove "viewport" from
     being managed by "form", change its resources to position it and make sure
     that "form" won't attempt to resize it.  Then we ask "form" to manage it
     again. */

  XtResizeWidget(toplevel, viewportWidth, viewportHeight, 0);

  XtUnmanageChild(viewport);

  XtVaSetValues(viewport,
                XtNhorizDistance, viewportX,
                XtNvertDistance, viewportY,
                XtNleft, XtChainLeft,
                XtNright, XtChainLeft,
                XtNtop, XtChainTop,
                XtNbottom, XtChainTop,
                NULL);

  XtManageChild(viewport);

  /* Now we can set "toplevel" to its proper size. */

  XtResizeWidget(toplevel, toplevelWidth, toplevelHeight, 0);

  /* Set the popup to overrideRedirect too */

  XtVaSetValues(popup, XtNoverrideRedirect, True, NULL);

  /* Try to get the input focus. */

  XSetInputFocus(dpy, DefaultRootWindow(dpy), RevertToPointerRoot,
                 CurrentTime);

  /* Optionally, grab the keyboard. */

  if (appData.grabKeyboard == TVNC_FS) GrabKeyboard();
}


/*
 * FullScreenOff() leaves full-screen mode.  It makes the toplevel window
 * managed by the window manager and sets its geometry appropriately.
 *
 * We also want to reestablish the link between the geometry of "form" and
 * "viewport".  We do this similarly to the way we broke it in FullScreenOn():
 * by making "viewport" unmanaged, changing certain resources on it, and asking
 * "form" to manage it again.
 *
 * There seems to be a slightly strange behaviour with setting forceBars back
 * to false, which results in "desktop" being stretched by the size of the
 * scrollbars under certain circumstances.  Resizing both "toplevel" and
 * "viewport" to the full-screen viewport size minus the scrollbar size seems
 * to fix it, though I'm not entirely sure why. */

void FullScreenOff()
{
  int toplevelWidth = si.framebufferWidth;
  int toplevelHeight = si.framebufferHeight;

  appData.fullScreen = False;

  if (appData.grabKeyboard == TVNC_FS) UngrabKeyboard();

  XtVaSetValues(toplevel, XtNmaxWidth, si.framebufferWidth,
                XtNmaxHeight, si.framebufferHeight, NULL);

  XtResizeWidget(toplevel,
                 viewportWidth - scrollbarWidth,
                 viewportHeight - scrollbarHeight, 0);
  XtResizeWidget(viewport,
                 viewportWidth - scrollbarWidth,
                 viewportHeight - scrollbarHeight, 0);

  XtVaSetValues(viewport, XtNforceBars, False, NULL);

  XtUnmanageChild(viewport);

  XtVaSetValues(viewport,
                XtNhorizDistance, 0,
                XtNvertDistance, 0,
                XtNleft, XtChainLeft,
                XtNright, XtChainRight,
                XtNtop, XtChainTop,
                XtNbottom, XtChainBottom,
                NULL);

  XtManageChild(viewport);

  XtVaSetValues(toplevel, XtNoverrideRedirect, False, NULL);

  if ((toplevelWidth + appData.wmDecorationWidth) >= dpyWidth)
    toplevelWidth = dpyWidth - appData.wmDecorationWidth;

  if ((toplevelHeight + appData.wmDecorationHeight) >= dpyHeight)
    toplevelHeight = dpyHeight - appData.wmDecorationHeight;

  XtResizeWidget(toplevel, toplevelWidth, toplevelHeight, 0);

  XSync(dpy, False);

  /* Set the popup back to non-overrideRedirect */

  XtVaSetValues(popup, XtNoverrideRedirect, False, NULL);

  netwm_fullscreen(dpy, XtWindow(toplevel), False);
}


/*
 * SetFullScreenState() is an action that sets the "state" resource of a toggle
 * widget to reflect whether we're in full-screen mode.
 */

void SetFullScreenState(Widget w, XEvent *ev, String *params,
                        Cardinal *num_params)
{
  if (appData.fullScreen)
    XtVaSetValues(w, XtNstate, True, NULL);
  else
    XtVaSetValues(w, XtNstate, False, NULL);
}


/*
 * ToggleFullScreen() is an action that toggles in and out of full-screen mode.
 */

void ToggleFullScreen(Widget w, XEvent *ev, String *params,
                      Cardinal *num_params)
{
  if (appData.fullScreen) {
    FullScreenOff();
  } else {
    FullScreenOn();
  }
}


/*
 * BumpScroll() is called when in full-screen mode and the mouse is against one
 * of the edges of the screen.  It returns true if any scrolling was done.
 */

Bool BumpScroll(XEvent *ev)
{
  scrollLeft = scrollRight = scrollUp = scrollDown = False;

  if (ev->xmotion.x_root >= dpyWidth - 3)
    scrollRight = True;
  else if (ev->xmotion.x_root <= 2)
    scrollLeft = True;

  if (ev->xmotion.y_root >= dpyHeight - 3)
    scrollDown = True;
  else if (ev->xmotion.y_root <= 2)
    scrollUp = True;

  if (scrollLeft || scrollRight || scrollUp || scrollDown) {
    if (timerSet)
      return True;

    XtVaGetValues(desktop, XtNx, &desktopX, XtNy, &desktopY, NULL);
    desktopX = -desktopX;
    desktopY = -desktopY;

    return DoBumpScroll();
  }

  if (timerSet) {
    XtRemoveTimeOut(timer);
    timerSet = False;
  }

  return False;
}


static Bool DoBumpScroll()
{
  int oldx = desktopX, oldy = desktopY;

  if (scrollRight) {
    if (desktopX < si.framebufferWidth - dpyWidth) {
      desktopX += appData.bumpScrollPixels;
      if (desktopX > si.framebufferWidth - dpyWidth)
        desktopX = si.framebufferWidth - dpyWidth;
    }
  } else if (scrollLeft) {
    if (desktopX > 0) {
      desktopX -= appData.bumpScrollPixels;
      if (desktopX < 0)
        desktopX = 0;
    }
  }

  if (scrollDown) {
    if (desktopY < si.framebufferHeight - dpyHeight) {
      desktopY += appData.bumpScrollPixels;
      if (desktopY > si.framebufferHeight - dpyHeight)
        desktopY = si.framebufferHeight - dpyHeight;
    }
  } else if (scrollUp) {
    if (desktopY > 0) {
      desktopY -= appData.bumpScrollPixels;
      if (desktopY < 0)
        desktopY = 0;
    }
  }

  if (oldx != desktopX || oldy != desktopY) {
    XawViewportSetCoordinates(viewport, desktopX, desktopY);
    timer = XtAppAddTimeOut(appContext, appData.bumpScrollTime,
                            BumpScrollTimerCallback, NULL);
    timerSet = True;
    return True;
  }

  timerSet = False;
  return False;
}


static void BumpScrollTimerCallback(XtPointer clientData, XtIntervalId *id)
{
  DoBumpScroll();
}
