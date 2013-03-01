/*
 *  Copyright (C) 2012-2013 D. R. Commander.  All Rights Reserved.
 *  Copyright (C) 2005-2008 Sun Microsystems, Inc.  All Rights Reserved.
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 *  USA.
 */

/*
 * misc.c - miscellaneous functions.
 */

#include <vncviewer.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <zlib.h>


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

extern void ShutdownThreads(void);
extern z_stream zlibStream[4];
extern Bool zlibStreamActive[4];

static const char *subsampLevel2str[TVNC_SAMPOPT] = {
  "1X", "4X", "2X", "Gray"
};


/*
 * HasEncoding returns True if the encodings string contains the given
 * substring.
 */

Bool HasEncoding(const char *str)
{
  if (appData.encodingsString) {
    char *encStr = appData.encodingsString;
    int encStrLen;
    do {
      char *nextEncStr = strchr(encStr, ' ');
      if (nextEncStr) {
        encStrLen = nextEncStr - encStr;
        nextEncStr++;
      } else {
        encStrLen = strlen(encStr);
      }
      if (strncasecmp(encStr, str, encStrLen) == 0) return True;
      encStr = nextEncStr;
    } while (encStr);
  }
  return False;
}


static int lastEncoding = -1;

void SetLastEncoding(int enc)
{
  char *titleFormat, title[1024], *ptr;
  if (enc != lastEncoding) {
    lastEncoding = enc;
    XtVaGetValues(toplevel, XtNtitle, &titleFormat, NULL);
    strncpy(title, titleFormat, 1023);
    if ((ptr = strrchr(title, '[')) != NULL) {
      UpdateTitleString(ptr, 1024 - (ptr - title));
      XtVaSetValues(toplevel, XtNtitle, title, XtNiconName, title, NULL);
    }
  }
}


/*
 * Update window title
 */

void UpdateTitleString(char *str, int len)
{
  if ((!appData.encodingsString || HasEncoding("tight")) &&
      (lastEncoding < 0 || lastEncoding == rfbEncodingTight)) {
    char zlibstr[80];
    zlibstr[0] = 0;
    if (!appData.enableJPEG) {
      if (appData.compressLevel == 1)
        snprintf(zlibstr, 79, " + Zlib");
      if (appData.compressLevel > 1)
        snprintf(zlibstr, 79, " + CL %d", appData.compressLevel);
      snprintf(str, len, "[Lossless Tight%s]", zlibstr);
    }
    else {
      if (appData.compressLevel > 1)
        snprintf(zlibstr, 79, " + CL %d", appData.compressLevel);
      snprintf(str, len, "[Tight + JPEG %s Q%d%s]",
        subsampLevel2str[appData.subsampLevel], appData.qualityLevel,
        zlibstr);
    }
  } else if (lastEncoding >= 0 && lastEncoding <= rfbEncodingHextile) {
    char encStr[6][8] = {"Raw", "", "", "", "", "Hextile"};
    snprintf(str, len, "[%s]", encStr[lastEncoding]);
  } else
    snprintf(str, len, "[%s]", appData.encodingsString);
}


/*
 * ToplevelInitBeforeRealization() sets the title, geometry, and other
 * resources on the toplevel window.
 */

void ToplevelInitBeforeRealization()
{
  char *titleFormat;
  char *title;
  char *geometry;
  char temps[80];

  XtVaGetValues(toplevel, XtNtitle, &titleFormat, NULL);
  temps[0] = ' ';
  UpdateTitleString(&temps[1], 79);

  title = XtMalloc(strlen(titleFormat) + strlen(desktopName)
    + strlen(temps) + 1);
  sprintf(title, titleFormat, desktopName);
  strncat(title,  temps, strlen(temps));
  XtVaSetValues(toplevel, XtNtitle, title, XtNiconName, title, NULL);
  XtFree(title);

  XtVaSetValues(toplevel, XtNmaxWidth, si.framebufferWidth,
                XtNmaxHeight, si.framebufferHeight, NULL);

  dpyWidth = WidthOfScreen(DefaultScreenOfDisplay(dpy));
  dpyHeight = HeightOfScreen(DefaultScreenOfDisplay(dpy));

  /* not full-screen - work out geometry for middle of screen, unless
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

    /* set position via "geometry" so that the window manager thinks it's a
       user-specified position and therefore honors it */
    geometry = XtMalloc(256);

    sprintf(geometry, "%dx%d+%d+%d",
            toplevelWidth, toplevelHeight, toplevelX, toplevelY);
    XtVaSetValues(toplevel, XtNgeometry, geometry, NULL);
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
 * ToplevelInitAfterRealization() initialises things that require the X windows
 * to exist.  It goes into full-screen mode if appropriate, and tells the
 * window manager that we accept the "delete window" message.
 */

void ToplevelInitAfterRealization()
{
  if (appData.fullScreen)
    FullScreenOn();

  wmDeleteWindow = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(dpy, XtWindow(toplevel), &wmDeleteWindow, 1);
  XtOverrideTranslations
    (toplevel, XtParseTranslationTable ("<Message>WM_PROTOCOLS: Quit()"));
}


/*
 * TimeFromEvent() gets the time field out of the given event.  It returns
 * CurrentTime if the event has no time field.
 */

Time TimeFromEvent(XEvent *ev)
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
 * Pause() is an action that pauses for a number of milliseconds (100 by
 * default).  It is sometimes useful to space out "fake" pointer events
 * generated by SendRFBEvent.
 */

void Pause(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  int msec;

  if (*num_params == 0)
    msec = 100;
  else
    msec = atoi(params[0]);

  usleep(msec * 1000);
}


/*
 * Run an arbitrary command via execvp()
 */

void RunCommand(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  int childstatus;

  if (*num_params == 0)
    return;

  if (fcntl(ConnectionNumber (dpy), F_SETFD, 1L) == -1)
      fprintf(stderr, "warning: file descriptor %d unusable for spawned program",
              ConnectionNumber(dpy));

  if (fcntl(rfbsock, F_SETFD, 1L) == -1)
      fprintf(stderr, "warning: file descriptor %d unusable for spawned program",
              rfbsock);

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

void Quit(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  Cleanup();
  exit(0);
}


/*
 * Cleanup - perform any cleanup operations prior to exiting.
 */

void Cleanup()
{
  if (xloginIconified) {
    IconifyNamedWindow(DefaultRootWindow(dpy), "xlogin", True);
    XFlush(dpy);
  }
#ifdef MITSHM
  if (appData.useShm)
    ShmCleanup();
#endif
  ShutdownThreads();
  UngrabKeyboard();
}

static int CleanupXErrorHandler(Display *dpy, XErrorEvent *error)
{
  fprintf(stderr, "CleanupXErrorHandler called\n");
  Cleanup();
  return (*defaultXErrorHandler)(dpy, error);
}

static int CleanupXIOErrorHandler(Display *dpy)
{
  fprintf(stderr, "CleanupXIOErrorHandler called\n");
  Cleanup();
  return (*defaultXIOErrorHandler)(dpy);
}

static void CleanupXtErrorHandler(String message)
{
  fprintf(stderr, "CleanupXtErrorHandler called\n");
  Cleanup();
  (*defaultXtErrorHandler)(message);
}

static void CleanupSignalHandler(int sig)
{
  fprintf(stderr, "CleanupSignalHandler called\n");
  Cleanup();
  exit(1);
}


/*
 * IconifyNamedWindow iconifies another client's window with the given name.
 */

static Bool IconifyNamedWindow(Window w, char *name, Bool undo)
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


/*
 * Measure how long it takes to decode and display a pre-encoded RFB session
 * capture.
 */

Bool RunBenchmark(void)
{
  int i, stream_id;
  double tStart, tTotal, tAvg = 0.0, tAvgDecode = 0.0, tAvgBlit = 0.0;

  for (i = 0; i < benchIter + benchWarmup; i++) {
    tStart = gettime();
    
    if (i < benchWarmup)
      printf("Benchmark warmup run %d\n", i + 1);
    else
      printf("Benchmark run %d:\n", i + 1 - benchWarmup);
    while (HandleRFBServerMessage()) {
    }
    tTotal = gettime() - tStart - tRecv;
    if (i >= benchWarmup) {
      printf("%f s (Decode = %f, Blit = %f)\n", tTotal, tDecode, tBlit);
      printf("     Blit statistics:\n");
      printf("     %.3f Mpixels, %.3f Mpixels/sec, %lu rect, %.0f pixels/rect\n",
             (double)tBlitPixels / 1000000.,
             (double)tBlitPixels / 1000000. / tBlit, tBlitRect,
             (double)tBlitPixels / (double)tBlitRect);
      tAvg += tTotal;
      tAvgDecode += tDecode;
      tAvgBlit += tBlit;
    }
    printf("\n");
    tRecv = tDecode = tBlit = 0.0;
    tBlitPixels = tBlitRect = 0;
    ShutdownThreads();
    for (stream_id = 0; stream_id < 4; stream_id++) {
      if (zlibStreamActive[stream_id]) {
        if (inflateEnd (&zlibStream[stream_id]) != Z_OK &&
            zlibStream[stream_id].msg != NULL) {
          fprintf(stderr, "inflateEnd: %s\n", zlibStream[stream_id].msg);
          return False;
        }
      }
      zlibStreamActive[stream_id] = False;
    }
    fseek(benchFile, benchFileStart, SEEK_SET);
  }
  if (benchIter > 1)
    printf("Average          :  %f s (Decode = %f, Blit = %f)\n",
           tAvg / (double)benchIter, tAvgDecode / (double)benchIter,
           tAvgBlit / (double)benchIter);
  return TRUE;
}
