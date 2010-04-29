/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2010 D. R. Commander.  All Rights Reserved.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */
//
// VNC server configuration utility
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <signal.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include "vncExt.h"

char* displayname = NULL;
Bool sendPrimaryCB = 1;
int pollTime = 0;
Bool acceptClipboard = 1;
Bool sendClipboard = 1;
char* cutText = NULL;
int cutTextLen = 0;
char* selection[2] = {0, 0};
int selectionLen[2] = {0, 0};
Bool debug = 0;
Bool selectionOwner_XA_PRIMARY = False;
Time selectionOwnTime_XA_PRIMARY = 0;
Bool selectionOwner_xaCLIPBOARD = False;
Time selectionOwnTime_xaCLIPBOARD = 0;
Time cutBufferTime = 0;
struct timeval dueTime = {0, 0};
Atom xaCLIPBOARD = 0, xaTARGETS = 0, xaTIMESTAMP = 0;
Window win = 0;

inline const char* selectionName(Atom sel) {
  if (sel == xaCLIPBOARD) return "CLIPBOARD";
  if (sel == XA_PRIMARY) return "PRIMARY";
  return "unknown";
}

#define ACCEPT_CUT_TEXT "AcceptCutText"
#define SEND_CUT_TEXT "SendCutText"

char* programName = 0;
Display* dpy;
int vncExtEventBase, vncExtErrorBase;

Bool isBefore(struct timeval other) {
  return (dueTime.tv_sec < other.tv_sec) ||
    ((dueTime.tv_sec == other.tv_sec) &&
     (dueTime.tv_usec < other.tv_usec));
}

inline static struct timeval addMillis(struct timeval inTime, int millis) {
  int secs = millis / 1000;
  millis = millis % 1000;
  inTime.tv_sec += secs;
  inTime.tv_usec += millis * 1000;
  if (inTime.tv_usec >= 1000000) {
    inTime.tv_sec++;
    inTime.tv_usec -= 1000000;
  }
  return inTime;
}

inline static int diffTimeMillis(struct timeval later,
                                 struct timeval earlier) {
  return ((later.tv_sec - earlier.tv_sec) * 1000)
          + ((later.tv_usec - earlier.tv_usec) / 1000);
}

#ifndef max
 #define max(a,b) ((a)>(b)?(a):(b))
#endif

// handleEvent(). If we get a ClientCutTextNotify event from Xvnc, set the
// primary and clipboard selections to the clientCutText. If we get a
// SelectionChangeNotify event from Xvnc, set the serverCutText to the value
// of the new selection.

void handleEvent(XEvent* ev) {
  if (acceptClipboard) {
    if (ev->type == vncExtEventBase + VncExtClientCutTextNotify) {
      XVncExtClientCutTextEvent* cutEv = (XVncExtClientCutTextEvent*)ev;
      if (cutText)
        XFree(cutText);
      cutText = 0;
      if (XVncExtGetClientCutText(dpy, &cutText, &cutTextLen)) {
        if (debug) fprintf(stderr, "Got client cut text: '%.*s%s'\n",
                   cutTextLen<9?cutTextLen:8, cutText,
                   cutTextLen<9?"":"...");
        XStoreBytes(dpy, cutText, cutTextLen);
        XSetSelectionOwner(dpy, XA_PRIMARY, win, cutEv->time);
        if (XGetSelectionOwner(dpy, XA_PRIMARY) == win) {
          selectionOwner_XA_PRIMARY = True;
          selectionOwnTime_XA_PRIMARY = cutEv->time;
        }
        XSetSelectionOwner(dpy, xaCLIPBOARD, win, cutEv->time);
        if (XGetSelectionOwner(dpy, xaCLIPBOARD) == win) {
          selectionOwner_xaCLIPBOARD = True;
          selectionOwnTime_xaCLIPBOARD = cutEv->time;
        }
        free(selection[0]);
        free(selection[1]);
        selection[0] = selection[1] = 0;
        selectionLen[0] = selectionLen[1] = 0;
      }
    }
  }
  if (sendClipboard) {
    if (ev->type == vncExtEventBase + VncExtSelectionChangeNotify) {
      if (debug) fprintf(stderr, "selection change event\n");
      XVncExtSelectionChangeEvent* selEv = (XVncExtSelectionChangeEvent*)ev;
      if ((selEv->selection == xaCLIPBOARD && !selectionOwner_xaCLIPBOARD)
          || (selEv->selection == XA_PRIMARY && sendPrimaryCB
          && !selectionOwner_XA_PRIMARY)) {
        XConvertSelection(dpy, selEv->selection, XA_STRING,
                          selEv->selection, win, CurrentTime);
      }
    }
  }
}

// selectionRequest() is called when we are the selection owner and another X
// client has requested the selection.  We simply put the server's cut text
// into the requested property.  TXWindow will handle the rest.

Bool selectionRequest(Window requestor, Atom selection, Atom property)
{
  if (cutText)
    XChangeProperty(dpy, requestor, property, XA_STRING, 8,
                    PropModeReplace, (unsigned char*)cutText,
                    cutTextLen);
  return (cutText != NULL);
}

// selectionNotify() is called when we have requested the selection from the
// selection owner.

void selectionNotify(XSelectionEvent* ev, Atom type, int format,
                     int nitems, void* data)
{
  if (ev->requestor != win || ev->target != XA_STRING)
    return;

  if (data && format == 8) {
    int i = (ev->selection == XA_PRIMARY ? 0 : 1);
    if (selectionLen[i] == nitems && memcmp(selection[i], data, nitems) == 0)
      return;
    free (selection[i]);
    selection[i] = (char *)malloc(nitems);
    if (!selection[i]) {
      fprintf(stderr, "Memory allocation failure.\n");
      exit(1);
    }
    memcpy(selection[i], data, nitems);
    selectionLen[i] = nitems;
    if (cutTextLen == nitems && memcmp(cutText, data, nitems) == 0) {
      if (debug) fprintf(stderr, "ignoring duplicate cut text\n");
      return;
    }
    if (cutText)
      XFree(cutText);
    cutText = (char*)malloc(nitems); // assuming XFree() same as free()
    memcpy(cutText, data, nitems);
    cutTextLen = nitems;
    if (debug) fprintf(stderr, "sending %s selection as server cut text: '%.*s%s'\n",
               selectionName(ev->selection),cutTextLen<9?cutTextLen:8,
               cutText, cutTextLen<9?"":"...");
    XVncExtSetServerCutText(dpy, cutText, cutTextLen);
  }
}

void handleXEvent(XEvent* ev)
{
  switch (ev->type) {

  case SelectionNotify:
    if (ev->xselection.property != None) {
      Atom type;
      int format;
      unsigned long nitems, after;
      unsigned char *data;
      XGetWindowProperty(dpy, win, ev->xselection.property, 0, 16384, True,
                         AnyPropertyType, &type, &format,
                         &nitems, &after, &data);
      if (type != None) {
        selectionNotify(&ev->xselection, type, format, nitems, data);
        XFree(data);
        break;
      }
    }
    selectionNotify(&ev->xselection, 0, 0, 0, 0);
    break;

  case SelectionRequest:
    {
      XSelectionEvent se;
      se.type = SelectionNotify;
      se.display = ev->xselectionrequest.display;
      se.requestor = ev->xselectionrequest.requestor;
      se.selection = ev->xselectionrequest.selection;
      se.time = ev->xselectionrequest.time;
      se.target = ev->xselectionrequest.target;
      if (ev->xselectionrequest.property == None)
        ev->xselectionrequest.property = ev->xselectionrequest.target;
      if ((se.selection == XA_PRIMARY && !selectionOwner_XA_PRIMARY)
         || (se.selection == xaCLIPBOARD && !selectionOwner_xaCLIPBOARD)) {
        se.property = None;
      } else {
        se.property = ev->xselectionrequest.property;
        if (se.target == xaTARGETS) {
          Atom targets[2];
          targets[0] = xaTIMESTAMP;
          targets[1] = XA_STRING;
          XChangeProperty(dpy, se.requestor, se.property, XA_ATOM, 32,
                          PropModeReplace, (unsigned char*)targets, 2);
        } else if (se.target == xaTIMESTAMP) {
          Time t = se.selection == XA_PRIMARY ? selectionOwnTime_XA_PRIMARY :
                   (se.selection == xaCLIPBOARD ? selectionOwnTime_xaCLIPBOARD
                   : 0);
          XChangeProperty(dpy, se.requestor, se.property, XA_INTEGER, 32,
                          PropModeReplace, (unsigned char*)&t, 1);
        } else if (se.target == XA_STRING) {
          if (!selectionRequest(se.requestor, se.selection, se.property))
            se.property = None;
        }
      }
      XSendEvent(dpy, se.requestor, False, 0, (XEvent*)&se);
      break;
    }

  case SelectionClear:
    if (ev->xselectionclear.selection == XA_PRIMARY) {
      selectionOwner_XA_PRIMARY = False;
    } else if (ev->xselectionclear.selection == xaCLIPBOARD) {
      selectionOwner_xaCLIPBOARD = False;
    }
    break;
  }

  handleEvent(ev);
}

// rfb::Timer::Callback interface

Bool handleTimeout(void) {
  if (sendPrimaryCB && !selectionOwner_XA_PRIMARY)
    XConvertSelection(dpy, XA_PRIMARY, XA_STRING,
                      XA_PRIMARY, win, CurrentTime);
  if (!selectionOwner_xaCLIPBOARD)
    XConvertSelection(dpy, xaCLIPBOARD, XA_STRING,
                      xaCLIPBOARD, win, CurrentTime);
  return True;
}

static void usage()
{
  fprintf(stderr,"\nusage: %s [-display <dpy>]\n", programName);
  fprintf(stderr,"       [-nosend] [-norecv] [-noprimary] [-polltime <t>] [-d]\n",
          programName);
  fprintf(stderr,"\n"
          "-display <dpy> = Connect to X display specified by <dpy>.  The VNC extension\n"
          "                 must be present on this X display\n"
          "-nosend = Do not send the clipboard to VNC viewers\n"
          "-norecv = Do not receive the clipboard from VNC viewers\n"
          "-noprimary = Do not send the primary selection to VNC viewers\n"
          "-polltime <t> = Poll for clipboard changes every <t> ms (default = 0)\n"
          "-d = Enable debugging output\n\n");
  exit(1);
}

int main(int argc, char** argv)
{
  struct timeval now;
  int i, toWait;
  programName = argv[0];

  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-display") == 0) {
      i++;
      if (i >= argc) usage();
      displayname = argv[i];
    } else if (strcmp(argv[i], "-polltime") == 0) {
      i++;
      if (i >= argc) usage();
      pollTime = atoi(argv[i]);
      if (pollTime < 0) usage();
    } else if (strcmp(argv[i], "-noprimary") == 0) {
      sendPrimaryCB = False;
    } else if (strcmp(argv[i], "-nosend") == 0) {
      sendClipboard = False;
    } else if (strcmp(argv[i], "-norecv") == 0) {
      acceptClipboard = False;
    } else if (strncmp(argv[i], "-d", 2) == 0) {
      debug = 1;
    } else if (strncmp(argv[i], "-h", 2) == 0) {
      usage();
    } else if (strncmp(argv[i], "-?", 2) == 0) {
      usage();
    }
  }

  if (!(dpy = XOpenDisplay(displayname))) {
    fprintf(stderr,"%s: unable to open display \"%s\"\n",
            programName, XDisplayName(displayname));
    exit(1);
  }

  if (!XVncExtQueryExtension(dpy, &vncExtEventBase, &vncExtErrorBase)) {
    fprintf(stderr,"No VNC extension on display %s\n",
            XDisplayName(displayname));
    exit(1);
  }

  if (!(xaCLIPBOARD = XInternAtom(dpy, "CLIPBOARD", False))) {
    fprintf(stderr,"Could not obtain CLIPBOARD atom");
    exit(1);
  }
  if (!(xaTIMESTAMP = XInternAtom(dpy, "TIMESTAMP", False))) {
    fprintf(stderr,"Could not obtain TIMESTAMP atom");
    exit(1);
  }
  if (!(xaTARGETS = XInternAtom(dpy, "TARGETS", False))) {
    fprintf(stderr,"Could not obtain TARGETS atom");
    exit(1);
  }

  if (!(win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 1, 1,
                                  0, WhitePixel(dpy, DefaultScreen(dpy)),
                                  BlackPixel(dpy, DefaultScreen(dpy))))) {
    fprintf(stderr,"Could not create window\n");
    exit(1);
  }

  XVncExtSelectInput(dpy, win,
                     VncExtClientCutTextMask|
                     VncExtSelectionChangeMask|
                     VncExtQueryConnectMask);
  XConvertSelection(dpy, XA_PRIMARY, XA_STRING,
                    XA_PRIMARY, win, CurrentTime);
  XConvertSelection(dpy, xaCLIPBOARD, XA_STRING,
                    xaCLIPBOARD, win, CurrentTime);

  if (pollTime != 0) {
    gettimeofday(&now, 0);
    dueTime = addMillis(now, pollTime);
  }

  XMapWindow(dpy, win);

  while (True) {
    struct timeval tv;
    struct timeval* tvp = 0;

    // Process any incoming X events
    while (XPending(dpy)) {
      XEvent ev;
      XNextEvent(dpy, &ev);
      if (ev.type == PropertyNotify &&
          ev.xproperty.window == DefaultRootWindow(dpy) &&
          ev.xproperty.atom == XA_CUT_BUFFER0) {
        cutBufferTime = ev.xproperty.time;
      } else {
        if (win == ev.xany.window)
          handleXEvent(&ev);
      }
    }

     
    // Process expired timers and get the time until the next one
    if (pollTime != 0) {
      gettimeofday(&now, 0);
      if (isBefore(now)) {
        if (debug) fprintf(stderr, "handleTimeout\n");
        if (handleTimeout()) {
          dueTime = addMillis(dueTime, pollTime);
          if (isBefore(now)) {
            // Time has jumped forwards!
            fprintf(stderr, "time has moved forwards!\n");
            dueTime = addMillis(now, pollTime);
          }
        }
      }

      gettimeofday(&now, 0);
      toWait = max(1, diffTimeMillis(dueTime, now));
      printf("toWait = %f\n", toWait / 1000.);
      tv.tv_sec = toWait / 1000;
      tv.tv_usec = (toWait % 1000) * 1000;
      tvp = &tv;
    }
      
    // If there are X requests pending then poll, don't wait!
    if (XPending(dpy)) {
      tv.tv_usec = tv.tv_sec = 0;
      tvp = &tv;
    }
      
    // Wait for X events, VNC traffic, or the next timer expiry
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(ConnectionNumber(dpy), &rfds);
    int n = select(FD_SETSIZE, &rfds, 0, 0, tvp);
    if (n < 0) {
      fprintf(stderr, "ERROR in select(): %s\n", strerror(errno));
      exit(1);
    }
  }

  XCloseDisplay(dpy);

  return 0;
}
