/*
 *  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
 *  Copyright (C) 2012 D. R. Commander.  All Rights Reserved.
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

/*
 * vncviewer.c - the Xt-based VNC viewer.
 */

#include <stdio.h>
#include <sys/socket.h>
#include <X11/Intrinsic.h>
#include "vncviewer.h"


char *programName;
XtAppContext appContext;
Display* dpy;
FILE *benchFile = NULL;
int benchIter = 1, benchWarmup = 0;
long benchFileStart = -1;

Widget toplevel;


int main(int argc, char **argv)
{
  int i;
  programName = argv[0];
  double tStart;

  /* The -listen option is used to start a daemon process that listens for
     incoming connections from servers, rather than actively connecting to a
     given server.  The -tunnel and -via options are useful for creating
     connections tunneled via SSH port forwarding.  We must test for the
     -listen option before invoking any Xt functions - this is because listen
     mode uses forking, and Xt doesn't seem to cope with forking very well
     (you might say it's forking clueless.)  When -listen is specified and a
     successful incoming connection has been accepted,
     listenForIncomingConnections() returns, setting the listenSpecified
     flag. */

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-ipv6") == 0) {
      family = AF_INET6;
      removeArgs(&argc, argv, i, 1);
    }
  }

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
    if (strcmp(argv[i], "-bench") == 0 && i < argc - 1) {
      if ((benchFile = fopen(argv[++i], "rb")) == NULL) {
        perror("Could not open session capture");
        exit(1);
      }
    }
    if (strcmp(argv[i], "-benchiter") == 0 && i < argc -1) {
      int iter = atoi(argv[++i]);
      if (iter > 0) benchIter = iter;
    }
    if (strcmp(argv[i], "-benchwarmup") == 0 && i < argc -1) {
      int warmup = atoi(argv[++i]);
      if (warmup > 0) benchWarmup = warmup;
    }
  }

  /* Call the main Xt initialisation function.  It parses command-line options,
     generating appropriate resource specs, and makes a connection to the X
     display. */
  toplevel = XtVaAppInitialize(&appContext, "Tvncviewer",
                               cmdLineOptions, numCmdLineOptions,
                               &argc, argv, fallback_resources,
                               XtNborderWidth, 0, NULL);

  dpy = XtDisplay(toplevel);

  /* Interpret resource specs and process any remaining command-line arguments
     (i.e. the VNC server name.)  If the server name isn't specified on the
     command line, GetArgsAndResources() will pop up a dialog box and wait
     for one to be entered. */
  GetArgsAndResources(argc, argv);

  if (!benchFile) {

    /* Unless we accepted an incoming connection, make a TCP connection to the
       given VNC server */
    if (!listenSpecified) {
      if (!ConnectToRFBServer(vncServerHost, vncServerPort)) exit(1);
    }

    /* Initialise the VNC connection, including reading the password */
    if (!InitialiseRFBConnection()) exit(1);

  } else {
    if (!ReadServerInitMessage()) exit(1);
  }

  /* Create the "popup" widget - this won't actually appear on the screen until
     some user-defined event causes the "ShowPopup" action to be invoked */
  CreatePopup();

  /* Find the best pixel format and X visual/colormap to use */
  SetVisualAndCmap();

  /* Create the "desktop" widget and perform initialisation that has to be
     done before the widgets are realized */
  ToplevelInitBeforeRealization();

  DesktopInitBeforeRealization();

  /* "Realize" all the widgets, i.e. actually create and map their X windows */
  XtRealizeWidget(toplevel);

  /* Perform initialisation that needs to be done after realization, now that
     the X windows exist */
  InitialiseSelection();

  ToplevelInitAfterRealization();

  DesktopInitAfterRealization();

  /* Run benchmark */
  if (benchFile) {
    Bool status = RunBenchmark();
    Cleanup();
    if (!status) exit(1);
    exit(0);
  }

  /* Tell the VNC server which pixel format and encodings we want to use */
  encodingChange = True;

  if (appData.grabKeyboard == TVNC_ALWAYS) GrabKeyboard();

  /* Now enter the main loop, processing VNC messages.  X events will
     automatically be processed whenever the VNC connection is idle. */
  tStart = gettime();
  while (1) {
    double tNew;
    XtInputMask mask;
    if (!HandleRFBServerMessage())
      break;
    if ((tNew = gettime()) - tStart >= 0.1) {
      /* Normally, Xt events are only processed when the socket is idle, which
         will be practically never if CU is enabled.  Thus, we have to check
         for new events periodically in that case. */
      if (continuousUpdates) {
        while ((mask = XtAppPending(appContext)) != 0)
          XtAppProcessEvent(appContext, mask);
      }
      tStart = tNew;
    }
  }

  Cleanup();

  return 0;
}
