/* Copyright (C) 2011 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

/*
 * This is a C port of the relevant portions of vncconfig from RealVNC.  In
 * TurboVNC, it is used solely to support clipboard transfer
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/time.h>
#include <errno.h>

#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include "vncExt.h"


char* programName = "tvncconfig";
Display* dpy;
char* displayname;
Window win, selectionOwner_[2] = {0, 0};
Time selectionOwnTime_[2] = {0, 0};
Atom xaCLIPBOARD, xaTIMESTAMP, xaTARGETS;
int vncExtEventBase, vncExtErrorBase;
char* cutText = 0;
int cutTextLen = 0;
char* selection[2] = {0, 0};
int selectionLen[2] = {0, 0};
int debug = 0;
int syncPrimary = 1;

inline const char *selectionName(Atom sel) {
  if (sel == xaCLIPBOARD) return "CLIPBOARD";
  if (sel == XA_PRIMARY) return "PRIMARY";
  return "unknown";
}


#define selectionOwner(a) selectionOwner_[a == XA_PRIMARY ? 0 : 1]
#define selectionOwnTime(a) selectionOwnTime_[a == XA_PRIMARY ? 0 : 1]

inline void ownSelection(Atom sel, Window win, Time time) {
  XSetSelectionOwner(dpy, sel, win, time);
  selectionOwner(sel) = win;
  selectionOwnTime(sel) = time;
}


void FatalError(const char *format, ...)
{
  va_list arglist;
  va_start(arglist, format);
  fprintf(stderr, "Fatal error in %s:\n", programName);
  vfprintf(stderr, format, arglist);
  fprintf(stderr, "\n");
  va_end(arglist);
  exit(1);
}


void debugprint(const char *format, ...)
{
  if (!debug) return;
  va_list arglist;
  va_start(arglist, format);
  vfprintf(stderr, format, arglist);
  fprintf(stderr, "\n");
  va_end(arglist);
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
    free(selection[i]);
    if ((selection[i] = (char *)malloc(nitems)) == NULL)
      FatalError("malloc(): %s", strerror(errno));
    memcpy(selection[i], data, nitems);
    selectionLen[i] = nitems;
    if (cutTextLen == nitems && memcmp(cutText, data, nitems) == 0) {
      debugprint("ignoring duplicate cut text");
      return;
    }
    if (cutText)
      XFree(cutText);
    cutText = (char*)malloc(nitems); // assuming XFree() same as free()
    memcpy(cutText, data, nitems);
    cutTextLen = nitems;
    if (syncPrimary || ev->selection != XA_PRIMARY) {
      debugprint("sending %s selection as server cut text: '%.*s%s'",
                 selectionName(ev->selection), cutTextLen < 9 ? cutTextLen : 8,
                 cutText, cutTextLen < 9 ? "" : "...");
      XVncExtSetServerCutText(dpy, cutText, cutTextLen);
    }
  }
}


// handleEvent(). If we get a ClientCutTextNotify event from Xvnc, set the
// primary and clipboard selections to the clientCutText. If we get a
// SelectionChangeNotify event from Xvnc, set the serverCutText to the value
// of the new selection.

void handleEvent(XEvent* ev)
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
    return;

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
      if (selectionOwner(se.selection) != win) {
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
          Time t = selectionOwnTime(se.selection);
          XChangeProperty(dpy, se.requestor, se.property, XA_INTEGER, 32,
                          PropModeReplace, (unsigned char*)&t, 1);
        } else if (se.target == XA_STRING) {
          if (cutText)
            XChangeProperty(dpy, se.requestor, se.property, XA_STRING, 8,
                            PropModeReplace, (unsigned char*)cutText,
                            cutTextLen);
          else
            se.property = None;
        } else
          se.property = None;
      }
      XSendEvent(dpy, se.requestor, False, 0, (XEvent*)&se);
      return;
    }

  case SelectionClear:
    selectionOwner(ev->xselectionclear.selection) = 0;
    return;

  }

  if (ev->type == vncExtEventBase + VncExtClientCutTextNotify) {
    XVncExtClientCutTextEvent* cutEv = (XVncExtClientCutTextEvent*)ev;
    if (cutText)
      XFree(cutText);
    cutText = 0;
    if (XVncExtGetClientCutText(dpy, &cutText, &cutTextLen)) {
      debugprint("Got client cut text: '%.*s%s'",
                 cutTextLen < 9 ? cutTextLen : 8, cutText,
                 cutTextLen < 9 ? "" : "...");
      if (syncPrimary) ownSelection(XA_PRIMARY, win, cutEv->time);
      ownSelection(xaCLIPBOARD, win, cutEv->time);
      free(selection[0]);
      free(selection[1]);
      selection[0] = selection[1] = 0;
      selectionLen[0] = selectionLen[1] = 0;
    }
  } else if (ev->type == vncExtEventBase + VncExtSelectionChangeNotify) {
    debugprint("selection change event");
    XVncExtSelectionChangeEvent* selEv = (XVncExtSelectionChangeEvent*)ev;
    if (selEv->selection == xaCLIPBOARD ||
        (selEv->selection == XA_PRIMARY && syncPrimary)) {
      if (selectionOwner(selEv->selection) != win)
        XConvertSelection(dpy, selEv->selection, XA_STRING,
                          selEv->selection, win, CurrentTime);
    }
  }
}


void usage(char *progName)
{
  fprintf(stderr, "\nUSAGE: %s [-display <d>] [-debug] [-noprimary]\n\n",
          progName);
  fprintf(stderr, "-display <d> = Handle clipboard for X display <d>\n");
  fprintf(stderr, "               (default: read from DISPLAY environment variable)\n");
  fprintf(stderr, "-debug = Print debugging output\n");
  fprintf(stderr, "-noprimary = Do not sync PRIMARY selection with client machine's clipboard\n\n");
  exit(1);
}


int main(int argc, char** argv)
{
  int i;
  for (i = 1; i < argc; i++) {
    if (!strncasecmp(argv[i], "-di", 3)) {
      if (i < argc - 1) displayname = argv[++i];
      else usage(argv[0]);
    }
    else if (!strncasecmp(argv[i], "-de", 3)) debug = 1;
    else if (!strncasecmp(argv[i], "-n", 2)) syncPrimary = 0;
    else if (!strncasecmp(argv[i], "-h", 2)) usage(argv[0]);
    else if (!strncasecmp(argv[i], "-?", 2)) usage(argv[0]);
  }

  if (!(dpy = XOpenDisplay(displayname)))
    FatalError("Unable to open display \"%s\"", XDisplayName(displayname));

  if (!XVncExtQueryExtension(dpy, &vncExtEventBase, &vncExtErrorBase))
    FatalError("No VNC extension on display %s", XDisplayName(displayname));

  xaCLIPBOARD = XInternAtom(dpy, "CLIPBOARD", False);
  xaTIMESTAMP = XInternAtom(dpy, "TIMESTAMP", False);
  xaTARGETS = XInternAtom(dpy, "TARGETS", False);

  win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 1, 1, 0,
                            WhitePixel(dpy, DefaultScreen(dpy)),
                            BlackPixel(dpy, DefaultScreen(dpy)));
  if (!win) FatalError("Unable to create window");

  XVncExtSelectInput(dpy, win, VncExtClientCutTextMask |
                               VncExtSelectionChangeMask);
  if (syncPrimary)
    XConvertSelection(dpy, XA_PRIMARY, XA_STRING,
                      XA_PRIMARY, win, CurrentTime);
  XConvertSelection(dpy, xaCLIPBOARD, XA_STRING,
                    xaCLIPBOARD, win, CurrentTime);

  while (1) {
    struct timeval tv;
    struct timeval* tvp = 0;

    // Process any incoming X events
    while (XPending(dpy)) {
      XEvent ev;
      XNextEvent(dpy, &ev);
      handleEvent(&ev);
    }

    // If there are X requests pending, then poll, don't wait!
    if (XPending(dpy)) {
      tv.tv_usec = tv.tv_sec = 0;
      tvp = &tv;
    }

    // Wait for X events, VNC traffic, or the next timer expiration
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(ConnectionNumber(dpy), &rfds);
    int n = select(FD_SETSIZE, &rfds, 0, 0, tvp);
    if (n < 0)
      FatalError("select(): %s", strerror(errno));
  }

  XCloseDisplay(dpy);

  return 0;
}
