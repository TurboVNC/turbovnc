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
 * selection.c - functions to deal with the selection and cut buffer.
 */

#include <vncviewer.h>

static void GetInitialSelectionTimeCallback(Widget w, XtPointer clientData,
					    Atom* selection, Atom* type,
					    XtPointer value,
					    unsigned long* length,
					    int* format);
static void GetSelectionCallback(Widget w, XtPointer clientData,
				 Atom* selection, Atom* type, XtPointer value,
				 unsigned long* length, int* format);
static void GetSelectionTimeCallback(Widget w, XtPointer clientData,
				     Atom* selection, Atom* type,
				     XtPointer value, unsigned long* length,
				     int* format);
static void SendCutBuffer();
static void CutBufferChange(Widget w, XtPointer ptr, XEvent *ev,
			    Boolean *cont);
static Boolean ConvertSelection(Widget w, Atom* selection, Atom* target,
				Atom* type, XtPointer* value,
				unsigned long* length, int* format);
static void LoseSelection(Widget w, Atom *selection);

static Bool iAmSelectionOwner = False;
static Time prevSelectionTime = 0L;
static Time cutBufferTime = 0L;

#define TIME_LATER(a, b) ((a) != 0 && ((b) == 0 || (INT32)((a) - (b)) > 0))


/*
 * InitialiseSelection() must be called after realizing widgets (because
 * otherwise XtGetSelectionValue() fails).  We register events on the root
 * window to appear as if on the toplevel window, and catch cut buffer
 * PropertyNotify events.  Then we try to get the timestamp of any existing
 * selection by calling XtGetSelectionValue() with target "TIMESTAMP".  In the
 * normal case we won't send this selection to the VNC server, but we need to
 * know its timestamp so that we can tell when a new selection has been made.
 * GetInitialSelectionTimeCallback() will be invoked when the timestamp is
 * available.
 */

void
InitialiseSelection()
{
#if XtSpecificationRelease >= 6
  XtRegisterDrawable(dpy, DefaultRootWindow(dpy), toplevel);
#else
  _XtRegisterWindow(DefaultRootWindow(dpy), toplevel);
#endif
  XSelectInput(dpy, DefaultRootWindow(dpy), PropertyChangeMask);

  XtAddRawEventHandler(toplevel, PropertyChangeMask, False, CutBufferChange,
		       NULL);

  XtGetSelectionValue(toplevel, XA_PRIMARY,
		      XInternAtom(dpy, "TIMESTAMP", False),
		      GetInitialSelectionTimeCallback, NULL, CurrentTime);
}


/*
 * GetInitialSelectionTimeCallback() is invoked when the selection owner has
 * told us the timestamp of the initial selection.  We just set
 * prevSelectionTime to the value returned, or the special value 0 (the same as
 * CurrentTime) if there is no selection.
 */

static void
GetInitialSelectionTimeCallback(Widget w, XtPointer clientData,
				Atom* selection, Atom* type, XtPointer value,
				unsigned long* length, int* format)
{
  if (value && *format == 32 && *length == 1)
    prevSelectionTime = *(CARD32 *)value;
  else
    prevSelectionTime = 0L;

  if (value)
    XtFree(value);
}


/*
 * SelectionToVNC() is an action which is usually invoked when the mouse enters
 * the viewer window.  With the "always" argument we always transfer the
 * current selection to the VNC server.  With no argument or the "new" argument
 * we transfer the selection only if it is "new" i.e. its timestamp is later
 * than the previously transferred selection.
 *
 * In the former case we call XtGetSelectionValue() with target "STRING" to get
 * the selection.  GetSelectionCallback() will be invoked when the selection is
 * available.
 *
 * In the latter case we call XtGetSelectionValue() with target "TIMESTAMP".
 * GetSelectionTimeCallback() will be invoked when the timestamp is available.
 */

void
SelectionToVNC(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  Bool always = False;

  if (*num_params != 0) {
    if (strcmp(params[0],"always") == 0) {
      always = True;
    } else if (strcmp(params[0],"new") == 0) {
      always = False;
    } else {
      fprintf(stderr,"Invalid params: SelectionToVNC(always|new)\n");
      return;
    }
  }

  if (always) {
    XtGetSelectionValue(w, XA_PRIMARY, XA_STRING, GetSelectionCallback, NULL,
			TimeFromEvent(event));
  } else {
    XtGetSelectionValue(w, XA_PRIMARY, XInternAtom(dpy, "TIMESTAMP", False),
			GetSelectionTimeCallback, NULL, TimeFromEvent(event));
  }
}


/*
 * GetSelectionCallback() is invoked when the selection has been retrieved from
 * the selection owner - we simply send it to the VNC server.  If there is
 * no PRIMARY selection available, we try sending CUT_BUFFER0 instead.
 */

static void
GetSelectionCallback(Widget w, XtPointer clientData, Atom* selection,
		     Atom* type, XtPointer value, unsigned long* length,
		     int* format)
{
  int len = *length;
  char *str = (char *)value;

  if (str)
    SendClientCutText(str, len);
  else
    SendCutBuffer();
}


/*
 * GetSelectionTimeCallback() is invoked when the selection owner has told us
 * the timestamp of the selection.  If the timestamp is later than that of the
 * previous selection then we call XtGetSelectionValue() with target "STRING"
 * to get the selection.  GetSelectionCallback() will be invoked when the
 * selection is available.  If there is no selection we see if the time
 * CUT_BUFFER0 was last changed is later than that of the previous selection,
 * and if so, send it.
 */

static void
GetSelectionTimeCallback(Widget w, XtPointer clientData, Atom* selection,
			 Atom* type, XtPointer value, unsigned long* length,
			 int* format)
{
  if (value && *format == 32 && *length == 1) {

    Time t = *(CARD32 *)value;

    if (TIME_LATER(t, prevSelectionTime)) {
      prevSelectionTime = t;
      XtGetSelectionValue(w, XA_PRIMARY, XA_STRING, GetSelectionCallback, NULL,
			  CurrentTime);
    }

  } else {

    if (TIME_LATER(cutBufferTime, prevSelectionTime)) {
      prevSelectionTime = cutBufferTime;
      SendCutBuffer();
    }
  }

  if (value)
    XtFree(value);
}


/*
 * SendCutBuffer gets the CUT_BUFFER0 property from the root window and sends
 * it to the VNC server.
 */

static void
SendCutBuffer()
{
  char *str;
  int len;

  str = XFetchBytes(dpy, &len);
  if (!str) return;

  SendClientCutText(str, len);
  XFree(str);
}


/*
 * CutBufferChange - check PropertyNotify events on the root window to see if
 * the cut buffer has changed.  If so, record its timestamp.
 */

static void
CutBufferChange(Widget w, XtPointer ptr, XEvent *ev, Boolean *cont)
{
  if (ev->type != PropertyNotify || ev->xproperty.atom != XA_CUT_BUFFER0)
    return;

  cutBufferTime = ev->xproperty.time;
}


/*
 * SelectionFromVNC() is an action which is usually invoked when the mouse
 * leaves the viewer window.  With the "always" argument we always set the
 * PRIMARY selection and CUT_BUFFER0 to the current value of the VNC server
 * "cut text".  With no argument or the "new" argument we set the selection
 * only if it is "new" i.e. there has been new "cut text" since the last time
 * it was called.
 */

void
SelectionFromVNC(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  Bool always = False;
  Time t = TimeFromEvent(event);

  if (*num_params != 0) {
    if (strcmp(params[0],"always") == 0) {
      always = True;
    } else if (strcmp(params[0],"new") == 0) {
      always = False;
    } else {
      fprintf(stderr,"Invalid params: SelectionFromVNC(always|new)\n");
      return;
    }
  }

  if (t == CurrentTime) {
    fprintf(stderr,"Error in translations: SelectionFromVNC() must act on "
	    "event with time field\n");
    return;
  }

  if (!serverCutText || (!always && !newServerCutText))
    return;

  newServerCutText = False;

  XStoreBytes(dpy, serverCutText, strlen(serverCutText));
  if (XtOwnSelection(desktop, XA_PRIMARY, t, ConvertSelection, LoseSelection,
		     NULL)) {
    iAmSelectionOwner = True;
  }
}


/*
 * ConvertSelection is called when another X client requests the selection.
 * Simply send the "server cut text" for a STRING target, or do a standard
 * conversion for anything else. 
 */

static Boolean
ConvertSelection(Widget w, Atom* selection, Atom* target, Atom* type,
		 XtPointer* value, unsigned long* length, int* format)
{

  if (*target == XA_STRING && serverCutText != NULL) {
    *type = XA_STRING;
    *length = strlen(serverCutText);
    *value = (XtPointer)XtMalloc(*length);
    memcpy((char*)*value, serverCutText, *length);
    *format = 8;
    return True;
  }

  if (XmuConvertStandardSelection(w, CurrentTime, selection, target, type,
				  (XPointer*)value, length, format)) {
    if (*target == XInternAtom(dpy, "TARGETS", False)) {
      /* add STRING to list of standard targets */
      Atom* targetP;
      Atom* std_targets = (Atom*)*value;
      unsigned long std_length = *length;

      *length = std_length + 1;
      *value = (XtPointer)XtMalloc(sizeof(Atom)*(*length));
      targetP = *(Atom**)value;
      *targetP++ = XA_STRING;
      memmove((char*)targetP, (char*)std_targets, sizeof(Atom)*std_length);
      XtFree((char*)std_targets);
      *type = XA_ATOM;
      *format = 32;
      return True;
    }

    return True;
  }
  return False;
}


/*
 * LoseSelection is called when we've lost ownership of the selection.
 */

static void
LoseSelection(Widget w, Atom *selection)
{
  iAmSelectionOwner = False;
}
