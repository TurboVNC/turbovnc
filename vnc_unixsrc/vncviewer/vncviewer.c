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
 * vncviewer.c - the Xt-based VNC viewer.
 */

#include "vncviewer.h"

char *programName;
XtAppContext appContext;
Display* dpy;

Widget toplevel;

int
main(int argc, char **argv)
{
  int i;
  programName = argv[0];

  /* The -listen option is used to make us a daemon process which listens for
     incoming connections from servers, rather than actively connecting to a
     given server. The -tunnel and -via options are useful to create
     connections tunneled via SSH port forwarding. We must test for the
     -listen option before invoking any Xt functions - this is because we use
     forking, and Xt doesn't seem to cope with forking very well. For -listen
     option, when a successful incoming connection has been accepted,
     listenForIncomingConnections() returns, setting the listenSpecified
     flag. */

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-listen") == 0) {
      listenForIncomingConnections(&argc, argv, i);
      break;
    }
    if (strcmp(argv[i], "-tunnel") == 0 || strcmp(argv[i], "-via") == 0) {
      if (!createTunnel(&argc, argv, i))
	exit(1);
      break;
    }
  }

  /* Call the main Xt initialisation function.  It parses command-line options,
     generating appropriate resource specs, and makes a connection to the X
     display. */

  toplevel = XtVaAppInitialize(&appContext, "Vncviewer",
			       cmdLineOptions, numCmdLineOptions,
			       &argc, argv, fallback_resources,
			       XtNborderWidth, 0, NULL);

  dpy = XtDisplay(toplevel);

  /* Interpret resource specs and process any remaining command-line arguments
     (i.e. the VNC server name).  If the server name isn't specified on the
     command line, getArgsAndResources() will pop up a dialog box and wait
     for one to be entered. */

  GetArgsAndResources(argc, argv);

  /* Unless we accepted an incoming connection, make a TCP connection to the
     given VNC server */

  if (!listenSpecified) {
    if (!ConnectToRFBServer(vncServerHost, vncServerPort)) exit(1);
  }

  /* Initialise the VNC connection, including reading the password */

  if (!InitialiseRFBConnection()) exit(1);

  /* Create the "popup" widget - this won't actually appear on the screen until
     some user-defined event causes the "ShowPopup" action to be invoked */

  CreatePopup();

  /* Find the best pixel format and X visual/colormap to use */

  SetVisualAndCmap();

  /* Create the "desktop" widget, and perform initialisation which needs doing
     before the widgets are realized */

  ToplevelInitBeforeRealization();

  DesktopInitBeforeRealization();

  /* "Realize" all the widgets, i.e. actually create and map their X windows */

  XtRealizeWidget(toplevel);

  /* Perform initialisation that needs doing after realization, now that the X
     windows exist */

  InitialiseSelection();

  ToplevelInitAfterRealization();

  DesktopInitAfterRealization();

  /* Tell the VNC server which pixel format and encodings we want to use */

  SetFormatAndEncodings();

  /* Now enter the main loop, processing VNC messages.  X events will
     automatically be processed whenever the VNC connection is idle. */

  while (1) {
    if (!HandleRFBServerMessage())
      break;
  }

  Cleanup();

  return 0;
}
