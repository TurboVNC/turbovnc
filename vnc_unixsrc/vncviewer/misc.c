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
 * misc.c - miscellaneous functions.
 */

#include <vncviewer.h>
#include <signal.h>
#include <fcntl.h>

static void CleanupSignalHandler(int sig);
static int CleanupXErrorHandler(Display *dpy, XErrorEvent *error);
static int CleanupXIOErrorHandler(Display *dpy);
static void CleanupXtErrorHandler(String message);
static Bool IconifyNamedWindow(Window w, char *name, Bool undo);

Dimension dpyWidth, dpyHeight;
Atom wmDeleteWindow, wmState;

static Bool xloginIconified = False;
static XErrorHandler defaultXErrorHandler;
static XIOErrorHandler defaultXIOErrorHandler;
static XtErrorHandler defaultXtErrorHandler;


/*
 * ToplevelInitBeforeRealization sets the title, geometry and other resources
 * on the toplevel window.
 */

void
ToplevelInitBeforeRealization()
{
  char *titleFormat;
  char *title;
  char *geometry;

  XtVaGetValues(toplevel, XtNtitle, &titleFormat, NULL);
  title = XtMalloc(strlen(titleFormat) + strlen(desktopName) + 1);
  sprintf(title, titleFormat, desktopName);
  XtVaSetValues(toplevel, XtNtitle, title, XtNiconName, title, NULL);

  XtVaSetValues(toplevel, XtNmaxWidth, si.framebufferWidth,
		XtNmaxHeight, si.framebufferHeight, NULL);

  dpyWidth = WidthOfScreen(DefaultScreenOfDisplay(dpy));
  dpyHeight = HeightOfScreen(DefaultScreenOfDisplay(dpy));

  if (appData.fullScreen) {

    /* full screen - set position to 0,0, but defer size calculation until
       widgets are realized */

    XtVaSetValues(toplevel, XtNoverrideRedirect, True,
		  XtNgeometry, "+0+0", NULL);

  } else {

    /* not full screen - work out geometry for middle of screen unless
       specified by user */

    XtVaGetValues(toplevel, XtNgeometry, &geometry, NULL);

    if (geometry == NULL) {
      Dimension toplevelX, toplevelY;
      Dimension toplevelWidth = si.framebufferWidth;
      Dimension toplevelHeight = si.framebufferHeight;

      if ((toplevelWidth + appData.wmDecorationWidth) >= dpyWidth)
	toplevelWidth = dpyWidth - appData.wmDecorationWidth;

      if ((toplevelHeight + appData.wmDecorationHeight) >= dpyHeight)
	toplevelHeight = dpyHeight - appData.wmDecorationHeight;

      toplevelX = (dpyWidth - toplevelWidth - appData.wmDecorationWidth) / 2;

      toplevelY = (dpyHeight - toplevelHeight - appData.wmDecorationHeight) /2;

      /* set position via "geometry" so that window manager thinks it's a
	 user-specified position and therefore honours it */

      geometry = XtMalloc(256);

      sprintf(geometry, "%dx%d+%d+%d",
	      toplevelWidth, toplevelHeight, toplevelX, toplevelY);
      XtVaSetValues(toplevel, XtNgeometry, geometry, NULL);
    }
  }

  /* Test if the keyboard is grabbed.  If so, it's probably because the
     XDM login window is up, so try iconifying it to release the grab */

  if (XGrabKeyboard(dpy, DefaultRootWindow(dpy), False, GrabModeSync,
		    GrabModeSync, CurrentTime) == GrabSuccess) {
    XUngrabKeyboard(dpy, CurrentTime);
  } else {
    wmState = XInternAtom(dpy, "WM_STATE", False);

    if (IconifyNamedWindow(DefaultRootWindow(dpy), "xlogin", False)) {
      xloginIconified = True;
      XSync(dpy, False);
      sleep(1);
    }
  }

  /* Set handlers for signals and X errors to perform cleanup */

  signal(SIGHUP, CleanupSignalHandler);
  signal(SIGINT, CleanupSignalHandler);
  signal(SIGTERM, CleanupSignalHandler);
  defaultXErrorHandler = XSetErrorHandler(CleanupXErrorHandler);
  defaultXIOErrorHandler = XSetIOErrorHandler(CleanupXIOErrorHandler);
  defaultXtErrorHandler = XtAppSetErrorHandler(appContext,
					       CleanupXtErrorHandler);
}


/*
 * ToplevelInitAfterRealization initialises things which require the X windows
 * to exist.  It goes into full-screen mode if appropriate, and tells the
 * window manager we accept the "delete window" message.
 */

void
ToplevelInitAfterRealization()
{
  if (appData.fullScreen) {
    FullScreenOn();
  }

  wmDeleteWindow = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(dpy, XtWindow(toplevel), &wmDeleteWindow, 1);
  XtOverrideTranslations
      (toplevel, XtParseTranslationTable ("<Message>WM_PROTOCOLS: Quit()"));
}


/*
 * TimeFromEvent() gets the time field out of the given event.  It returns
 * CurrentTime if the event has no time field.
 */

Time
TimeFromEvent(XEvent *ev)
{
  switch (ev->type) {
  case KeyPress:
  case KeyRelease:
    return ev->xkey.time;
  case ButtonPress:
  case ButtonRelease:
    return ev->xbutton.time;
  case MotionNotify:
    return ev->xmotion.time;
  case EnterNotify:
  case LeaveNotify:
    return ev->xcrossing.time;
  case PropertyNotify:
    return ev->xproperty.time;
  case SelectionClear:
    return ev->xselectionclear.time;
  case SelectionRequest:
    return ev->xselectionrequest.time;
  case SelectionNotify:
    return ev->xselection.time;
  default:
    return CurrentTime;
  }
}


/*
 * Pause is an action which pauses for a number of milliseconds (100 by
 * default).  It is sometimes useful to space out "fake" pointer events
 * generated by SendRFBEvent.
 */

void
Pause(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  int msec;

  if (*num_params == 0) {
    msec = 100;
  } else {
    msec = atoi(params[0]);
  }

  usleep(msec * 1000);
}


/*
 * Run an arbitrary command via execvp()
 */
void
RunCommand(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  int childstatus;

  if (*num_params == 0)
    return;

  if (fcntl (ConnectionNumber (dpy), F_SETFD, 1L) == -1)
      fprintf(stderr, "warning: file descriptor %d unusable for spawned program", ConnectionNumber(dpy));
  
  if (fcntl (rfbsock, F_SETFD, 1L) == -1)
      fprintf(stderr, "warning: file descriptor %d unusable for spawned program", rfbsock);

  switch (fork()) {
  case -1: 
    perror("fork"); 
    break;
  case 0:
      /* Child 1. Fork again. */
      switch (fork()) {
      case -1:
	  perror("fork");
	  break;

      case 0:
	  /* Child 2. Do some work. */
	  execvp(params[0], params);
	  perror("exec");
	  exit(1);
	  break;  

      default:
	  break;
      }

      /* Child 1. Exit, and let init adopt our child */
      exit(0);

  default:
    break;
  }

  /* Wait for Child 1 to die */
  wait(&childstatus);
  
  return;
}


/*
 * Quit action - called when we get a "delete window" message.
 */

void
Quit(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  Cleanup();
  exit(0);
}


/*
 * Cleanup - perform any cleanup operations prior to exiting.
 */

void
Cleanup()
{
  if (xloginIconified) {
    IconifyNamedWindow(DefaultRootWindow(dpy), "xlogin", True);
    XFlush(dpy);
  }
#ifdef MITSHM
  if (appData.useShm)
    ShmCleanup();
#endif
}

static int
CleanupXErrorHandler(Display *dpy, XErrorEvent *error)
{
  fprintf(stderr,"CleanupXErrorHandler called\n");
  Cleanup();
  return (*defaultXErrorHandler)(dpy, error);
}

static int
CleanupXIOErrorHandler(Display *dpy)
{
  fprintf(stderr,"CleanupXIOErrorHandler called\n");
  Cleanup();
  return (*defaultXIOErrorHandler)(dpy);
}

static void
CleanupXtErrorHandler(String message)
{
  fprintf(stderr,"CleanupXtErrorHandler called\n");
  Cleanup();
  (*defaultXtErrorHandler)(message);
}

static void
CleanupSignalHandler(int sig)
{
  fprintf(stderr,"CleanupSignalHandler called\n");
  Cleanup();
  exit(1);
}


/*
 * IconifyNamedWindow iconifies another client's window with the given name.
 */

static Bool
IconifyNamedWindow(Window w, char *name, Bool undo)
{
  Window *children, dummy;
  unsigned int nchildren;
  int i;
  char *window_name;
  Atom type = None;
  int format;
  unsigned long nitems, after;
  unsigned char *data;

  if (XFetchName(dpy, w, &window_name)) {
    if (strcmp(window_name, name) == 0) {
      if (undo) {
	XMapWindow(dpy, w);
      } else {
	XIconifyWindow(dpy, w, DefaultScreen(dpy));
      }
      XFree(window_name);
      return True;
    }
    XFree(window_name);
  }

  XGetWindowProperty(dpy, w, wmState, 0, 0, False,
		     AnyPropertyType, &type, &format, &nitems,
		     &after, &data);
  if (type != None) {
    XFree(data);
    return False;
  }

  if (!XQueryTree(dpy, w, &dummy, &dummy, &children, &nchildren))
    return False;

  for (i = 0; i < nchildren; i++) {
    if (IconifyNamedWindow(children[i], name, undo)) {
      XFree ((char *)children);
      return True;
    }
  }
  if (children) XFree ((char *)children);
  return False;
}
