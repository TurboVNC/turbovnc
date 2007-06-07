/*
 *  Copyright (C) 2005-2006 Sun Microsystems, Inc.  All Rights Reserved.
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
 * popup.c - functions to deal with popup window.
 */

#include "vncviewer.h"

#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Scrollbar.h>


Widget popup, fullScreenToggle, button4X, button2X, button1X, buttonGray,
  qualtext, qualslider, wanbutton, buttonRGB, buttonJPEG;


void
UpdateQual(void)
{
  char *titleFormat, title[1024], *ptr;
  char text[10];
  SetFormatAndEncodings();
  XtVaGetValues(toplevel, XtNtitle, &titleFormat, NULL);
  strncpy(title, titleFormat, 1023);
  if((ptr=strrchr(title, '['))!=NULL)
  {
    if(appData.compressType == TVNC_RGB)
      sprintf(ptr, "[RGB]");
    else
      sprintf(ptr, "[JPEG %s Q%d]", compressLevel2str[appData.compressLevel],
        appData.qualityLevel);
    XtVaSetValues(toplevel, XtNtitle, title, XtNiconName, title, NULL);
  }
  XawScrollbarSetThumb(qualslider, (float)appData.qualityLevel/100., 0.);
  sprintf(text, "%d", appData.qualityLevel);
  XtVaSetValues(qualtext, XtNlabel, text, NULL);

  if(appData.compressType == TVNC_RGB)
  {
    XtVaSetValues(buttonRGB, XtNstate, 1, NULL);
    XtVaSetValues(buttonJPEG, XtNstate, 0, NULL);
  }
  else if(appData.compressType == TVNC_JPEG)
  {
    XtVaSetValues(buttonRGB, XtNstate, 0, NULL);
    XtVaSetValues(buttonJPEG, XtNstate, 1, NULL);
  }

  if(appData.compressLevel==TVNC_1X)
  {
    XtVaSetValues(buttonGray, XtNstate, 0, NULL);
    XtVaSetValues(button4X, XtNstate, 0, NULL);
    XtVaSetValues(button2X, XtNstate, 0, NULL);
    XtVaSetValues(button1X, XtNstate, 1, NULL);
  }
  else if(appData.compressLevel==TVNC_4X)
  {
    XtVaSetValues(buttonGray, XtNstate, 0, NULL);
    XtVaSetValues(button4X, XtNstate, 1, NULL);
    XtVaSetValues(button2X, XtNstate, 0, NULL);
    XtVaSetValues(button1X, XtNstate, 0, NULL);
  }
  else if(appData.compressLevel==TVNC_2X)
  {
    XtVaSetValues(buttonGray, XtNstate, 0, NULL);
    XtVaSetValues(button4X, XtNstate, 0, NULL);
    XtVaSetValues(button2X, XtNstate, 1, NULL);
    XtVaSetValues(button1X, XtNstate, 0, NULL);
  }
  else if(appData.compressLevel==TVNC_GRAY)
  {
    XtVaSetValues(buttonGray, XtNstate, 1, NULL);
    XtVaSetValues(button4X, XtNstate, 0, NULL);
    XtVaSetValues(button2X, XtNstate, 0, NULL);
    XtVaSetValues(button1X, XtNstate, 0, NULL);
  }
  XtVaSetValues(wanbutton, XtNstate, appData.optimizeForWAN, NULL);
}


void
ShowPopup(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  XtMoveWidget(popup, event->xbutton.x_root, event->xbutton.y_root);
  XtPopup(popup, XtGrabNone);
  XSetWMProtocols(dpy, XtWindow(popup), &wmDeleteWindow, 1);
  UpdateQual();
}

void
HidePopup(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  XtPopdown(popup);
}


static XtResource resources[] = {
  {
    "type", "Type", XtRString, sizeof(String), 0, XtRString,
    (XtPointer) "command",
  },
};


void
qualScrollProc(Widget w, XtPointer client, XtPointer p)
{
  float	size, val;  int qual, pos=(int)p;
  XtVaGetValues(w, XtNshown, &size, XtNtopOfThumb, &val, 0);
  if(pos<0) val-=.1;  else val+=.1;
  qual=(int)(val*100.);  if(qual<1) qual=1;  if(qual>100) qual=100;
  XawScrollbarSetThumb(w, val, 0.);
  appData.qualityLevel=qual;
  UpdateQual();
}


void
qualJumpProc(Widget w, XtPointer client, XtPointer p)
{
  float val=*(float *)p;  int qual;
  qual=(int)(val*100.);  if(qual<1) qual=1;  if(qual>100) qual=100;
  appData.qualityLevel=qual;
  UpdateQual();
}


void
buttonRGBProc(Widget w, XtPointer client, XtPointer p)
{
  if((int)p==1) appData.compressType=TVNC_RGB;
  UpdateQual();
}


void
buttonJPEGProc(Widget w, XtPointer client, XtPointer p)
{
  if((int)p==1) appData.compressType=TVNC_JPEG;
  UpdateQual();
}


void
buttonGrayProc(Widget w, XtPointer client, XtPointer p)
{
  if((int)p==1) appData.compressLevel=TVNC_GRAY;
  UpdateQual();
}


void
button4XProc(Widget w, XtPointer client, XtPointer p)
{
  if((int)p==1) appData.compressLevel=TVNC_4X;
  UpdateQual();
}


void
button2XProc(Widget w, XtPointer client, XtPointer p)
{
  if((int)p==1) appData.compressLevel=TVNC_2X;
  UpdateQual();
}


void
button1XProc(Widget w, XtPointer client, XtPointer p)
{
  if((int)p==1) appData.compressLevel=TVNC_1X;
  UpdateQual();
}

void
wanbuttonProc(Widget w, XtPointer client, XtPointer p)
{
  if((int)p==1) appData.optimizeForWAN=1;
	else appData.optimizeForWAN=0;
  UpdateQual();
}

void
CreatePopup()
{
  Widget buttonForm, button, prevButton = NULL, label;
  int i;
  char buttonName[12];
  String buttonType;

  popup = XtVaCreatePopupShell("popup", transientShellWidgetClass, toplevel,
			       NULL);

  buttonForm = XtVaCreateManagedWidget("buttonForm", formWidgetClass, popup,
				       NULL);

  if (appData.popupButtonCount > 100) {
    fprintf(stderr,"Too many popup buttons\n");
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
      fprintf(stderr,"unknown button type '%s'\n",buttonType);
    }
    prevButton = button;
  }

  label = XtCreateManagedWidget("compressLabel", labelWidgetClass, buttonForm,
    NULL, 0);
  XtVaSetValues(label, XtNfromVert, prevButton, XtNleft, XawChainLeft,
    XtNright, XawChainRight, NULL);

  buttonRGB = XtCreateManagedWidget("compressRGB", toggleWidgetClass,
    buttonForm, NULL, 0);
  XtVaSetValues(buttonRGB, XtNfromVert, label, XtNleft, XawChainLeft, NULL);
  XtAddCallback(buttonRGB, XtNcallback, buttonRGBProc, NULL);

  buttonJPEG = XtCreateManagedWidget("compressJPEG", toggleWidgetClass,
    buttonForm, NULL, 0);
  XtVaSetValues(buttonJPEG, XtNfromVert, label, XtNfromHoriz, buttonRGB);
  XtAddCallback(buttonJPEG, XtNcallback, buttonJPEGProc, NULL);

  label = XtCreateManagedWidget("qualLabel", labelWidgetClass, buttonForm, NULL, 0);
  XtVaSetValues(label, XtNfromVert, buttonRGB, XtNleft, XawChainLeft, XtNright,
    XawChainRight, NULL);

  qualslider = XtCreateManagedWidget("qualBar", scrollbarWidgetClass, buttonForm,
    NULL, 0);
  XtVaSetValues(qualslider, XtNfromVert, label, XtNleft, XawChainLeft, NULL);
  XtAddCallback(qualslider, XtNscrollProc, qualScrollProc, NULL) ;
  XtAddCallback(qualslider, XtNjumpProc, qualJumpProc, NULL) ;

  qualtext = XtCreateManagedWidget("qualText", labelWidgetClass, buttonForm,
    NULL, 0);
  XtVaSetValues(qualtext, XtNfromVert, label, XtNfromHoriz, qualslider, XtNright,
    XawChainRight, NULL);

  label = XtCreateManagedWidget("subsampLabel", labelWidgetClass, buttonForm,
    NULL, 0);
  XtVaSetValues(label, XtNfromVert, qualslider, XtNleft, XawChainLeft,
    XtNright, XawChainRight, NULL);

  buttonGray = XtCreateManagedWidget("subsampGray", toggleWidgetClass, buttonForm,
    NULL, 0);
  XtVaSetValues(buttonGray, XtNfromVert, label, XtNleft, XawChainLeft, NULL);
  XtAddCallback(buttonGray, XtNcallback, buttonGrayProc, NULL);

  button4X = XtCreateManagedWidget("subsamp4X", toggleWidgetClass, buttonForm,
    NULL, 0);
  XtVaSetValues(button4X, XtNfromVert, label, XtNfromHoriz, buttonGray);
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

  wanbutton =  XtCreateManagedWidget("wanopt", toggleWidgetClass, buttonForm,
    NULL, 0);
  XtVaSetValues(wanbutton, XtNfromVert, buttonGray, XtNleft, XawChainLeft, NULL);
  XtAddCallback(wanbutton, XtNcallback, wanbuttonProc, NULL);

}
