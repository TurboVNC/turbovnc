/*
 *  Copyright (C) 2009-2012 D. R. Commander.  All Rights Reserved.
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

#include "vncviewer.h"

#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Scrollbar.h>


extern Bool HasEncoding(const char *);

Widget popup, fullScreenToggle, button4X, button2X, button1X, buttonGray,
  qualtext, qualslider, buttonCompress[10], buttonJPEG;


void UpdateQual(void)
{
  char *titleFormat, title[1024], *ptr;
  char text[10];  int i;

  encodingChange = True;
  XtVaGetValues(toplevel, XtNtitle, &titleFormat, NULL);
  strncpy(title, titleFormat, 1023);
  if ((ptr = strrchr(title, '[')) != NULL) {
    UpdateTitleString(ptr, 1024 - (ptr - title));
    XtVaSetValues(toplevel, XtNtitle, title, XtNiconName, title, NULL);
  }
  XawScrollbarSetThumb(qualslider, (float)appData.qualityLevel/100., 0.);
  sprintf(text, "%d", appData.qualityLevel);
  XtVaSetValues(qualtext, XtNlabel, text, NULL);

  for (i = 0; i <= 9; i++) {
    if (buttonCompress[i]) {
      if (appData.compressLevel == i)
        XtVaSetValues(buttonCompress[i], XtNstate, 1, NULL);
      else
        XtVaSetValues(buttonCompress[i], XtNstate, 0, NULL);
    }
  }
  if (appData.enableJPEG) {
    if (appData.compressLevel < 0)
      XtVaSetValues(buttonCompress[1], XtNstate, 1, NULL);
    XtVaSetValues(buttonJPEG, XtNstate, 1, NULL);
  } else
    XtVaSetValues(buttonJPEG, XtNstate, 0, NULL);

  if (appData.subsampLevel == TVNC_1X) {
    XtVaSetValues(buttonGray, XtNstate, 0, NULL);
    XtVaSetValues(button4X, XtNstate, 0, NULL);
    XtVaSetValues(button2X, XtNstate, 0, NULL);
    XtVaSetValues(button1X, XtNstate, 1, NULL);
  } else if (appData.subsampLevel == TVNC_4X) {
    XtVaSetValues(buttonGray, XtNstate, 0, NULL);
    XtVaSetValues(button4X, XtNstate, 1, NULL);
    XtVaSetValues(button2X, XtNstate, 0, NULL);
    XtVaSetValues(button1X, XtNstate, 0, NULL);
  } else if (appData.subsampLevel == TVNC_2X) {
    XtVaSetValues(buttonGray, XtNstate, 0, NULL);
    XtVaSetValues(button4X, XtNstate, 0, NULL);
    XtVaSetValues(button2X, XtNstate, 1, NULL);
    XtVaSetValues(button1X, XtNstate, 0, NULL);
  } else if (appData.subsampLevel == TVNC_GRAY) {
    XtVaSetValues(buttonGray, XtNstate, 1, NULL);
    XtVaSetValues(button4X, XtNstate, 0, NULL);
    XtVaSetValues(button2X, XtNstate, 0, NULL);
    XtVaSetValues(button1X, XtNstate, 0, NULL);
  }
}


void ShowPopup(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  XtMoveWidget(popup, event->xbutton.x_root, event->xbutton.y_root);
  XtPopup(popup, XtGrabNone);
  XSetWMProtocols(dpy, XtWindow(popup), &wmDeleteWindow, 1);
  UpdateQual();
}


void HidePopup(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  XtPopdown(popup);
}


static XtResource resources[] = {
  {
    "type", "Type", XtRString, sizeof(String), 0, XtRString,
    (XtPointer) "command",
  },
};


void qualScrollProc(Widget w, XtPointer client, XtPointer p)
{
  float size, val;  int qual;  long pos = (long)p;
  XtVaGetValues(w, XtNshown, &size, XtNtopOfThumb, &val, NULL);
  if (pos < 0) val -= .1;  else val += .1;
  qual = (int)(val * 100.);
  if (qual < 1) qual = 1;  if (qual > 100) qual = 100;
  XawScrollbarSetThumb(w, val, 0.);
  appData.qualityLevel = qual;
  UpdateQual();
}


void qualJumpProc(Widget w, XtPointer client, XtPointer p)
{
  float val = *(float *)p;  int qual;
  qual = (int)(val * 100.);
  if (qual < 1) qual = 1;  if (qual > 100) qual = 100;
  appData.qualityLevel = qual;
  UpdateQual();
}


void buttonCompressProc(Widget w, XtPointer client, XtPointer p)
{
  int i;
  for (i = 0; i <= 9; i++)
    if (buttonCompress[i] && w == buttonCompress[i] && (long)p == 1)
      appData.compressLevel = i;
  UpdateQual();
}


void buttonJPEGProc(Widget w, XtPointer client, XtPointer p)
{
  if ((long)p == 1)
    appData.enableJPEG = True;
  else
    appData.enableJPEG = False;
  UpdateQual();
}


void buttonGrayProc(Widget w, XtPointer client, XtPointer p)
{
  if ((long)p == 1) appData.subsampLevel = TVNC_GRAY;
  UpdateQual();
}


void button4XProc(Widget w, XtPointer client, XtPointer p)
{
  if ((long)p == 1) appData.subsampLevel = TVNC_4X;
  UpdateQual();
}


void button2XProc(Widget w, XtPointer client, XtPointer p)
{
  if ((long)p == 1) appData.subsampLevel = TVNC_2X;
  UpdateQual();
}


void button1XProc(Widget w, XtPointer client, XtPointer p)
{
  if ((long)p == 1) appData.subsampLevel = TVNC_1X;
  UpdateQual();
}


void SetViewOnlyState(Widget w, XEvent *ev, String *params,
                      Cardinal *num_params)
{
  if (appData.viewOnly)
    XtVaSetValues(w, XtNstate, True, NULL);
  else
    XtVaSetValues(w, XtNstate, False, NULL);
}


void CreatePopup()
{
  Widget buttonForm, button = NULL, prevButton = NULL, label, labelLo, labelHi;
  int i, compressLevelMax;
  char buttonName[15];
  String buttonType;

  popup = XtVaCreatePopupShell("popup", transientShellWidgetClass, toplevel,
                               NULL);

  buttonForm = XtVaCreateManagedWidget("buttonForm", formWidgetClass, popup,
                                       NULL);

  if (appData.popupButtonCount > 100) {
    fprintf(stderr, "Too many popup buttons\n");
    exit(1);
  }

  for (i = 1; i <= appData.popupButtonCount; i++) {
    sprintf(buttonName, "button%d", i);
    XtVaGetSubresources(buttonForm, (XtPointer)&buttonType, buttonName,
                        "Button", resources, 1, NULL);

    if (strcmp(buttonType, "command") == 0) {
      button = XtVaCreateManagedWidget(buttonName, commandWidgetClass,
                                       buttonForm, NULL);
      XtVaSetValues(button, XtNfromVert, prevButton,
                    XtNleft, XawChainLeft, XtNright, XawChainRight, NULL);
    } else if (strcmp(buttonType, "toggle") == 0) {
      button = XtVaCreateManagedWidget(buttonName, toggleWidgetClass,
                                       buttonForm, NULL);
      XtVaSetValues(button, XtNfromVert, prevButton,
                    XtNleft, XawChainLeft, XtNright, XawChainRight, NULL);
    } else {
      fprintf(stderr, "unknown button type '%s'\n", buttonType);
    }
    prevButton = button;
  }

  buttonJPEG = XtCreateManagedWidget("enableJPEG", toggleWidgetClass,
                                     buttonForm, NULL, 0);
  XtVaSetValues(buttonJPEG, XtNfromVert, prevButton, XtNleft, XawChainLeft,
    NULL);
  XtAddCallback(buttonJPEG, XtNcallback, buttonJPEGProc, NULL);

  label = XtCreateManagedWidget("qualLabel", labelWidgetClass, buttonForm,
                                NULL, 0);
  XtVaSetValues(label, XtNfromVert, buttonJPEG, XtNleft, XawChainLeft,
                XtNright, XawChainRight, NULL);

  labelLo = XtCreateManagedWidget("qualLabelLo", labelWidgetClass, buttonForm,
                                  NULL, 0);
  XtVaSetValues(labelLo, XtNfromVert, label, XtNleft, XawChainLeft,
                XtNright, XawChainRight, NULL);

  qualslider = XtCreateManagedWidget("qualBar", scrollbarWidgetClass,
                                     buttonForm, NULL, 0);
  XtVaSetValues(qualslider, XtNfromVert, label, XtNfromHoriz, labelLo, NULL);
  XtAddCallback(qualslider, XtNscrollProc, qualScrollProc, NULL) ;
  XtAddCallback(qualslider, XtNjumpProc, qualJumpProc, NULL) ;

  qualtext = XtCreateManagedWidget("qualText", labelWidgetClass, buttonForm,
                                   NULL, 0);
  XtVaSetValues(qualtext, XtNfromVert, label, XtNfromHoriz, qualslider, NULL);

  labelHi = XtCreateManagedWidget("qualLabelHi", labelWidgetClass, buttonForm,
                                  NULL, 0);
  XtVaSetValues(labelHi, XtNfromVert, label, XtNfromHoriz, qualtext, NULL);

  label = XtCreateManagedWidget("subsampLabel", labelWidgetClass, buttonForm,
                                NULL, 0);
  XtVaSetValues(label, XtNfromVert, qualtext, XtNleft, XawChainLeft,
                XtNright, XawChainRight, NULL);

  labelLo = XtCreateManagedWidget("subsampLabelLo", labelWidgetClass,
                                  buttonForm, NULL, 0);
  XtVaSetValues(labelLo, XtNfromVert, label, XtNleft, XawChainLeft,
                XtNright, XawChainRight, NULL);

  buttonGray = XtCreateManagedWidget("subsampGray", toggleWidgetClass,
                                     buttonForm, NULL, 0);
  XtVaSetValues(buttonGray, XtNfromVert, label, XtNfromHoriz, labelLo, NULL);
  XtAddCallback(buttonGray, XtNcallback, buttonGrayProc, NULL);

  button4X = XtCreateManagedWidget("subsamp4X", toggleWidgetClass, buttonForm,
                                   NULL, 0);
  XtVaSetValues(button4X, XtNfromVert, label, XtNfromHoriz, buttonGray, NULL);
  XtAddCallback(button4X, XtNcallback, button4XProc, NULL);

  button2X = XtCreateManagedWidget("subsamp2X", toggleWidgetClass, buttonForm,
                                   NULL, 0);
  XtVaSetValues(button2X, XtNfromVert, label, XtNfromHoriz, button4X,
                XtNradioGroup, buttonGray, NULL);
  XtAddCallback(button2X, XtNcallback, button2XProc, NULL);

  button1X = XtCreateManagedWidget("subsamp1X", toggleWidgetClass, buttonForm,
                                   NULL, 0);
  XtVaSetValues(button1X, XtNfromVert, label, XtNfromHoriz, button2X,
                XtNradioGroup, buttonGray, NULL);
  XtAddCallback(button1X, XtNcallback, button1XProc, NULL);

  labelHi = XtCreateManagedWidget("subsampLabelHi", labelWidgetClass,
                                  buttonForm, NULL, 0);
  XtVaSetValues(labelHi, XtNfromVert, label, XtNfromHoriz, button1X, NULL);

  compressLevelMax = 1;
  if (appData.compressLevel > 1 ||
      (appData.encodingsString && !HasEncoding("tight")))
    compressLevelMax = 9;

  if (compressLevelMax == 9)
    label = XtCreateManagedWidget("compressLabel", labelWidgetClass, buttonForm,
                                  NULL, 0);
  else
    label = XtCreateManagedWidget("zlibCompressLabel", labelWidgetClass,
                                  buttonForm, NULL, 0);
  XtVaSetValues(label, XtNfromVert, buttonGray, XtNleft, XawChainLeft,
                XtNright, XawChainRight, NULL);

  labelLo = XtCreateManagedWidget("compressLabelLo", labelWidgetClass,
                                  buttonForm, NULL, 0);
  XtVaSetValues(labelLo, XtNfromVert, label, XtNleft, XawChainLeft,
                XtNright, XawChainRight, NULL);

  for (i = 0; i <= compressLevelMax; i++) {
    char text[2];
    snprintf(buttonName, 15, "compressButton%d", i);
    buttonCompress[i] = XtCreateManagedWidget(buttonName, toggleWidgetClass,
                                              buttonForm, NULL, 0);
    XtVaSetValues(buttonCompress[i], XtNfromVert, label, NULL);
    if (i == 0)
      XtVaSetValues(buttonCompress[i], XtNfromHoriz, labelLo, NULL);
    else
      XtVaSetValues(buttonCompress[i], XtNfromHoriz, buttonCompress[i - 1],
                    NULL);
    if (compressLevelMax == 1 && i == 0)
      XtVaSetValues(buttonCompress[i], XtNlabel, "None", NULL);
    else {
      snprintf(text, 2, "%d", i);
      XtVaSetValues(buttonCompress[i], XtNlabel, text, NULL);
    }
    XtAddCallback(buttonCompress[i], XtNcallback, buttonCompressProc, NULL);
    if (appData.compressLevel == i)
      XtVaSetValues(buttonCompress[i], XtNstate, 1, NULL);
  }

  labelHi = XtCreateManagedWidget("compressLabelHi", labelWidgetClass,
                                  buttonForm, NULL, 0);
  XtVaSetValues(labelHi, XtNfromVert, label, XtNfromHoriz,
                buttonCompress[i - 1], NULL);
}
