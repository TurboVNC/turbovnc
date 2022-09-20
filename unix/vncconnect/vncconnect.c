/* Copyright (C) 2013, 2015, 2018 D. R. Commander.  All Rights Reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "vncExt.h"


static char *programName;


static void usage(void)
{
  fprintf(stderr, "\nUSAGE: %s [-display <d>] [-disconnect] client[:port]\n\n",
          programName);
  fprintf(stderr, "-display <d> = specify the X display of the TurboVNC session that you wish to\n"
                  "               connect to a listening viewer (for instance, :1).  If this is\n"
                  "               not specified, then the value of the DISPLAY environment\n"
                  "               variable is used.\n");
  fprintf(stderr, "-id <id> = specify the ID number of the TurboVNC session, if connecting to an\n"
                  "           instance of the UltraVNC Repeater in Mode II.\n");
  fprintf(stderr, "-disconnect = disconnect all listening viewers from the specified TurboVNC\n"
                  "              session.\n\n");
  exit(1);
}


int main(int argc, char **argv)
{
  char *displayname = NULL;
  Display *dpy;  int id = -1;
  int i, disconnect = 0, status = 0;

  programName = argv[0];

  for (i = 1; i < argc; i++) {
    if (argv[i][0] != '-')
      break;

    if (!strncmp(argv[i], "-disp", 5)) {
      if (++i >= argc) usage();
      displayname = argv[i];
    } else if (!strncmp(argv[i], "-disc", 5)) {
      disconnect = 1;
    } else if (!strncmp(argv[i], "-id", 3)) {
      int _id = -1;

      if (++i >= argc) usage();
      _id = atoi(argv[i]);
      if (_id <= 0) {
        fprintf(stderr, "ERROR: ID must be greater than 0.\n");
        exit(1);
      }
      id = _id;
    } else usage();
  }

  if (argc != i + 1 && !disconnect)
    usage();

  if (!(dpy = XOpenDisplay(displayname))) {
    fprintf(stderr, "%s: unable to open display \"%s\"\n", programName,
            XDisplayName(displayname));
    exit(1);
  }

  if (disconnect) {
    if (!XVncExtConnect(dpy, "")) {
      fprintf(stderr, "Could not disconnect listening viewers (perhaps there are none)\n");
      status = 1;
    }
  } else {
    char temps[256];

    if (id >= 0) {
      strncpy(temps, argv[i], 255);
      temps[255] = 0;
      snprintf(&temps[strlen(temps)], 255 - strlen(temps), "#%d", id);
    }
    if (!XVncExtConnect(dpy, id >= 0 ? temps : argv[i])) {
      fprintf(stderr, "Reverse connection to %s failed\n", argv[i]);
      status = 1;
    }
  }

  XCloseDisplay(dpy);

  return status;
}
