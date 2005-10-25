/*
 *  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 *  USA.
 */

/*
 * fullscreen.c - functions to deal with full-screen mode.
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



/*
 * FullScreenOn goes into full-screen mode.  It makes the toplevel window
 * unmanaged by the window manager and sets its geometry appropriately.
 *
 * We have toplevel -> form -> viewport -> desktop.  "form" must always be the
 * same size as "toplevel".  "desktop" should always be fixed at the size of
 * the VNC desktop.  Normally "viewport" is the same size as "toplevel" (<=
 * size of "desktop"), and "viewport" deals with any difference by putting up
 * scrollbars.
 *
 * When we go into full-screen mode, we allow "viewport" and "form" to be
 * different sizes, and we effectively need to work out all the geometries
 * ourselves.  There are two cases to deal with:
 *
 * 1. When the desktop is smaller than the display, "viewport" is simply the
 *    size of the desktop and "toplevel" (and "form") are the size of the
 *    display.  "form" is visible around the edges of the desktop.
 *
 * 2. When the desktop is bigger than the display in either or both dimensions,
 *    we force "viewport" to have scrollbars.
 *
 *    If the desktop width is bigger than the display width, then the width of
 *    "viewport" is the display width plus the scrollbar width, otherwise it's
 *    the desktop width plus the scrollbar width.  The width of "toplevel" (and
 *    "form") is then either the same as "viewport", or just the display width,
 *    respectively.  Similarly for the height of "viewport" and the height of
 *    "toplevel".
 *
 *    So if the desktop is bigger than the display in both dimensions then both
 *    the scrollbars will be just off the screen.  If it's bigger in only one
 *    dimension then that scrollbar _will_ be visible, with the other one just
 *    off the screen.  We treat this as a "feature" rather than a problem - you
 *    can't easily get around it if you want to use the Athena viewport for
 *    doing the scrolling.
 *
 * In either case, we position "viewport" in the middle of "form".
 *
 * We store the calculated size of "viewport" and the scrollbars in global
 * variables so that FullScreenOff can use them.
 */

void
FullScreenOn()
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

    if (si.framebufferWidth > dpyWidth) {
      viewportWidth = toplevelWidth = dpyWidth + scrollbarWidth;
    } else {
      viewportWidth = si.framebufferWidth + scrollbarWidth;
      toplevelWidth = dpyWidth;
    }

    if (si.framebufferHeight > dpyHeight) {
      viewportHeight = toplevelHeight = dpyHeight + scrollbarHeight;
    } else {
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


  /* We want to stop the window manager from managing our toplevel window.
     This is not really a nice thing to do, so may not work properly with every
     window manager.  We do this simply by setting overrideRedirect and
     reparenting our window to the root.  The window manager will get a
     ReparentNotify and hopefully clean up its frame window. */

  XtVaSetValues(toplevel, XtNoverrideRedirect, True, NULL);

  XReparentWindow(dpy, XtWindow(toplevel), DefaultRootWindow(dpy), 0, 0);

  /* Some WMs does not obey x,y values of XReparentWindow; the window
     is not placed in the upper, left corner. The code below fixes
     this: It manually moves the window, after the Xserver is done
     with XReparentWindow. The last XSync seems to prevent losing
     focus, but I don't know why. */
  XSync(dpy, False);
  XMoveWindow(dpy, XtWindow(toplevel), 0, 0);
  XSync(dpy, False);

  /* Now we want to fix the size of "viewport".  We shouldn't just change it
     directly.  Instead we set "toplevel" to the required size (which should
     propagate through "form" to "viewport").  Then we remove "viewport" from
     being managed by "form", change its resources to position it and make sure
     that "form" won't attempt to resize it, then ask "form" to manage it
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

  if (appData.grabKeyboard &&
      XtGrabKeyboard(desktop, True, GrabModeAsync,
		     GrabModeAsync, CurrentTime) != GrabSuccess) {
    fprintf(stderr, "XtGrabKeyboard() failed.\n");
  }
}


/*
 * FullScreenOff leaves full-screen mode.  It makes the toplevel window
 * managed by the window manager and sets its geometry appropriately.
 *
 * We also want to reestablish the link between the geometry of "form" and
 * "viewport".  We do this similarly to the way we broke it in FullScreenOn, by
 * making "viewport" unmanaged, changing certain resources on it and asking
 * "form" to manage it again.
 *
 * There seems to be a slightly strange behaviour with setting forceBars back
 * to false, which results in "desktop" being stretched by the size of the
 * scrollbars under certain circumstances.  Resizing both "toplevel" and
 * "viewport" to the full-screen viewport size minus the scrollbar size seems
 * to fix it, though I'm not entirely sure why. */

void
FullScreenOff()
{
  int toplevelWidth = si.framebufferWidth;
  int toplevelHeight = si.framebufferHeight;

  appData.fullScreen = False;

  if (appData.grabKeyboard)
    XtUngrabKeyboard(desktop, CurrentTime);

  XtUnmapWidget(toplevel);

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

  XtMapWidget(toplevel);
  XSync(dpy, False);

  /* Set the popup back to non-overrideRedirect */

  XtVaSetValues(popup, XtNoverrideRedirect, False, NULL);
}


/*
 * SetFullScreenState is an action which sets the "state" resource of a toggle
 * widget to reflect whether we're in full-screen mode.
 */

void
SetFullScreenState(Widget w, XEvent *ev, String *params, Cardinal *num_params)
{
  if (appData.fullScreen)
    XtVaSetValues(w, XtNstate, True, NULL);
  else
    XtVaSetValues(w, XtNstate, False, NULL);
}


/*
 * ToggleFullScreen is an action which toggles in and out of full-screen mode.
 */

void
ToggleFullScreen(Widget w, XEvent *ev, String *params, Cardinal *num_params)
{
  if (appData.fullScreen) {
    FullScreenOff();
  } else {
    FullScreenOn();
  }
}


/*
 * BumpScroll is called when in full-screen mode and the mouse is against one
 * of the edges of the screen.  It returns true if any scrolling was done.
 */

Bool
BumpScroll(XEvent *ev)
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

static Bool
DoBumpScroll()
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

static void
BumpScrollTimerCallback(XtPointer clientData, XtIntervalId *id)
{
  DoBumpScroll();
}
