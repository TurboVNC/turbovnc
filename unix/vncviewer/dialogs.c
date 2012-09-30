/*
 *  Copyright (C) 2010 University Corporation for Atmospheric Research.
                       All Rights Reserved.
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

#include "vncviewer.h"
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/AsciiText.h>


static Bool serverDialogDone = False;
static Bool passwordDialogDone = False;


void ServerDialogDone(Widget w, XEvent *event, String *params,
                      Cardinal *num_params)
{
  serverDialogDone = True;
}


char *DoServerDialog()
{
  Widget pshell, dialog;
  char *vncServerName;
  char *valueString;

  pshell = XtVaCreatePopupShell("serverDialog", transientShellWidgetClass,
                                toplevel, NULL);
  dialog = XtVaCreateManagedWidget("dialog", dialogWidgetClass, pshell, NULL);

  XtMoveWidget(pshell, WidthOfScreen(XtScreen(pshell)) * 2 / 5,
               HeightOfScreen(XtScreen(pshell)) * 2 / 5);
  XtPopup(pshell, XtGrabNonexclusive);
  XtRealizeWidget(pshell);

  serverDialogDone = False;

  while (!serverDialogDone)
    XtAppProcessEvent(appContext, XtIMAll);

  valueString = XawDialogGetValueString(dialog);
  vncServerName = XtNewString(valueString);

  XtPopdown(pshell);
  return vncServerName;
}


void PasswordDialogDone(Widget w, XEvent *event, String *params,
                        Cardinal *num_params)
{
  passwordDialogDone = True;
}


char *DoPasswordDialog()
{
  Widget pshell, dialog;
  char *password;
  char *valueString;

  pshell = XtVaCreatePopupShell("passwordDialog", transientShellWidgetClass,
                                toplevel, NULL);
  dialog = XtVaCreateManagedWidget("dialog", dialogWidgetClass, pshell, NULL);

  XtMoveWidget(pshell, WidthOfScreen(XtScreen(pshell)) * 2 / 5,
               HeightOfScreen(XtScreen(pshell)) * 2 / 5);
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


static Bool userPwdDialogDone = False;

void UserPwdDialogDone(Widget w, XEvent *event, String *params,
                       Cardinal *num_params)
{
  userPwdDialogDone = True;
}


static Widget userPwdForm;
static Widget pwdField;
static Widget userField;

void UserPwdNextField(Widget w, XEvent *event, String *params,
                      Cardinal *num_params)
{
  if (userPwdForm == NULL)
    return;

  if ((w == userField) && (pwdField != NULL)) {
    XtSetKeyboardFocus(userPwdForm, pwdField);
    return;
  }

  if ((w == pwdField) && (userField != NULL)) {
    XtSetKeyboardFocus(userPwdForm, userField);
    return;
  }
}


void UserPwdSetFocus(Widget w, XEvent *event, String *params,
                     Cardinal *num_params)
{
  if (userPwdForm == NULL)
    return;

  if (w == userField) {
    XtSetKeyboardFocus(userPwdForm, userField);
    return;
  }

  if (w == pwdField) {
    XtSetKeyboardFocus(userPwdForm, pwdField);
    return;
  }
}


void DoUserPwdDialog(char** user, char** password)
{
  Widget pshell;
  Widget userLabel;
  Widget pwdLabel;
  String string;
  Arg args[] = {{XtNstring, (XtArgVal) ""}};
  struct passwd pwbuf;
  struct passwd* pw;
  char buf[256];

  pshell = XtVaCreatePopupShell("userPwdDialog", transientShellWidgetClass,
                                toplevel, NULL);
  userPwdForm = XtVaCreateManagedWidget("form", formWidgetClass, pshell, NULL);
  userLabel = XtVaCreateManagedWidget("userLabel", labelWidgetClass,
                                      userPwdForm, NULL);
  userField = XtVaCreateManagedWidget("userField", asciiTextWidgetClass,
                                      userPwdForm, NULL);
  pwdLabel = XtVaCreateManagedWidget("pwdLabel", labelWidgetClass, userPwdForm,
                                     NULL);
  pwdField = XtVaCreateManagedWidget("pwdField", asciiTextWidgetClass,
                                     userPwdForm, NULL);

  XtMoveWidget(pshell, WidthOfScreen(XtScreen(pshell)) * 2 / 5,
               HeightOfScreen(XtScreen(pshell)) * 2 / 5);
  XtPopup(pshell, XtGrabNonexclusive);

  if (*user != NULL) {
    args[0].value = (XtArgVal) *user;
    XtSetValues(userField, args, XtNumber(args));
    XawTextSetInsertionPoint(userField, strlen(*user));

  } else if (getpwuid_r(getuid(), &pwbuf, buf, sizeof(buf), &pw) == 0) {
    args[0].value = (XtArgVal) pwbuf.pw_name;
    XtSetValues(userField, args, XtNumber(args));
    XawTextSetInsertionPoint(userField, strlen(pwbuf.pw_name));
  }

  XtRealizeWidget(pshell);
  XtSetKeyboardFocus(userPwdForm, userField);

  userPwdDialogDone = False;

  while (!userPwdDialogDone) {
    XtAppProcessEvent(appContext, XtIMAll);
  }

  args[0].value = (XtArgVal) &string;
  XtGetValues(userField, args, XtNumber(args));
  *user = XtNewString(string);

  XtGetValues(pwdField, args, XtNumber(args));
  *password = XtNewString(string);
  while (*string != '\0')
    *string++ = '\0';

  XtPopdown(pshell);
  userField = NULL;
  pwdField = NULL;
  userPwdForm = NULL;
  XtDestroyWidget(pshell);
}
