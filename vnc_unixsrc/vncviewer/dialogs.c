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
 * dialog.c - code to deal with dialog boxes.
 */

#include "vncviewer.h"
#include <X11/Xaw/Dialog.h>

static Bool serverDialogDone = False;
static Bool passwordDialogDone = False;

void
ServerDialogDone(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  serverDialogDone = True;
}

char *
DoServerDialog()
{
  Widget pshell, dialog;
  char *vncServerName;
  char *valueString;

  pshell = XtVaCreatePopupShell("serverDialog", transientShellWidgetClass,
				toplevel, NULL);
  dialog = XtVaCreateManagedWidget("dialog", dialogWidgetClass, pshell, NULL);

  XtMoveWidget(pshell, WidthOfScreen(XtScreen(pshell))*2/5,
	       HeightOfScreen(XtScreen(pshell))*2/5);
  XtPopup(pshell, XtGrabNonexclusive);
  XtRealizeWidget(pshell);

  serverDialogDone = False;

  while (!serverDialogDone) {
    XtAppProcessEvent(appContext, XtIMAll);
  }

  valueString = XawDialogGetValueString(dialog);
  vncServerName = XtNewString(valueString);

  XtPopdown(pshell);
  return vncServerName;
}

void
PasswordDialogDone(Widget w, XEvent *event, String *params,
		   Cardinal *num_params)
{
  passwordDialogDone = True;
}

char *
DoPasswordDialog()
{
  Widget pshell, dialog;
  char *password;
  char *valueString;

  pshell = XtVaCreatePopupShell("passwordDialog", transientShellWidgetClass,
				toplevel, NULL);
  dialog = XtVaCreateManagedWidget("dialog", dialogWidgetClass, pshell, NULL);

  XtMoveWidget(pshell, WidthOfScreen(XtScreen(pshell))*2/5,
	       HeightOfScreen(XtScreen(pshell))*2/5);
  XtPopup(pshell, XtGrabNonexclusive);
  XtRealizeWidget(pshell);

  passwordDialogDone = False;

  while (!passwordDialogDone) {
    XtAppProcessEvent(appContext, XtIMAll);
  }

  valueString = XawDialogGetValueString(dialog);
  password = XtNewString(valueString);

  XtPopdown(pshell);
  return password;
}
