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

Widget popup, fullScreenToggle;

void
ShowPopup(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  XtMoveWidget(popup, event->xbutton.x_root, event->xbutton.y_root);
  XtPopup(popup, XtGrabNone);
  XSetWMProtocols(dpy, XtWindow(popup), &wmDeleteWindow, 1);
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
CreatePopup()
{
  Widget buttonForm, button, prevButton = NULL;
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
}
