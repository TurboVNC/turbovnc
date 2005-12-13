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
 * popup.c - functions to deal with popup window.
 */

#include "vncviewer.h"

#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Scrollbar.h>


Widget popup, fullScreenToggle, button411, button422, button444, qualtext,
  qualslider;


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
    sprintf(ptr, "[%s Q%d]", appData.compressLevel==1?"4:1:1":
      appData.compressLevel==2?"4:2:2":"4:4:4", appData.qualityLevel);
    XtVaSetValues(toplevel, XtNtitle, title, XtNiconName, title, NULL);
  }
  XawScrollbarSetThumb(qualslider, (float)appData.qualityLevel/100., 0.);
  sprintf(text, "%d", appData.qualityLevel);
  XtVaSetValues(qualtext, XtNlabel, text, NULL);
  if(appData.compressLevel==0)
  {
    XtVaSetValues(button411, XtNstate, 0, NULL);
    XtVaSetValues(button422, XtNstate, 0, NULL);
    XtVaSetValues(button444, XtNstate, 1, NULL);
  }
  else if(appData.compressLevel==1)
  {
    XtVaSetValues(button411, XtNstate, 1, NULL);
    XtVaSetValues(button422, XtNstate, 0, NULL);
    XtVaSetValues(button444, XtNstate, 0, NULL);
  }
  else if(appData.compressLevel==2)
  {
    XtVaSetValues(button411, XtNstate, 0, NULL);
    XtVaSetValues(button422, XtNstate, 1, NULL);
    XtVaSetValues(button444, XtNstate, 0, NULL);
  }
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
button411Proc(Widget w, XtPointer client, XtPointer p)
{
  if((int)p==1) appData.compressLevel=1;
  UpdateQual();
}


void
button422Proc(Widget w, XtPointer client, XtPointer p)
{
  if((int)p==1) appData.compressLevel=2;
  UpdateQual();
}


void
button444Proc(Widget w, XtPointer client, XtPointer p)
{
  if((int)p==1) appData.compressLevel=0;
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

  label = XtCreateManagedWidget("qualLabel", labelWidgetClass, buttonForm, NULL, 0);
  XtVaSetValues(label, XtNfromVert, prevButton, XtNleft, XawChainLeft, XtNright,
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

  button411 =  XtCreateManagedWidget("subsamp411", toggleWidgetClass, buttonForm,
    NULL, 0);
  XtVaSetValues(button411, XtNfromVert, label, XtNleft, XawChainLeft, NULL);
  XtAddCallback(button411, XtNcallback, button411Proc, NULL);

  button422 =  XtCreateManagedWidget("subsamp422", toggleWidgetClass, buttonForm,
    NULL, 0);
  XtVaSetValues(button422, XtNfromVert, label, XtNfromHoriz, button411, 
    XtNradioGroup, button411, NULL);
  XtAddCallback(button422, XtNcallback, button422Proc, NULL);

  button444 =  XtCreateManagedWidget("subsamp444", toggleWidgetClass, buttonForm,
    NULL, 0);
  XtVaSetValues(button444, XtNfromVert, label, XtNfromHoriz, button422,
    XtNradioGroup, button411, NULL);
  XtAddCallback(button444, XtNcallback, button444Proc, NULL);
}
