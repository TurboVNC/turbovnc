/* Copyright (C) 2017, 2021 D. R. Commander.  All Rights Reserved.
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

/*
 * This is a C port of the relevant portions of vncconfig from RealVNC.  In
 * TurboVNC, it is used solely to get and set Xvnc parameters.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "vncExt.h"


Display *dpy;
char *displayname, *programName;
int vncExtEventBase, vncExtErrorBase;


static void FatalError(const char *format, ...)
{
  va_list arglist;
  va_start(arglist, format);
  fprintf(stderr, "Fatal error in %s:\n", programName);
  vfprintf(stderr, format, arglist);
  fprintf(stderr, "\n");
  va_end(arglist);
  exit(1);
}


static void usage(void)
{
  fprintf(stderr, "\nUSAGE: %s [options] [-set] <Xvnc-param>=<value> ...\n",
          programName);
  fprintf(stderr, "       %s [options] -list\n", programName);
  fprintf(stderr, "       %s [options] -get <Xvnc-param>\n", programName);
  fprintf(stderr, "       %s [options] -desc <Xvnc-param>\n\n", programName);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "-display <d> = Get/set parameters for X display <d>\n");
  fprintf(stderr, "-v = Verbose output (print descriptions with -get and -list)\n\n");
  exit(1);
}


static Bool PrintParameterValue(char *param)
{
  char *data = NULL;
  int len;

  if (!XVncExtGetParam(dpy, param, &data, &len)) {
    if (data) XFree(data);
    return False;
  }
  printf("%.*s\n", len, data);

  XFree(data);
  return True;
}


static Bool PrintParameterDesc(char *param)
{
  char *desc;

  if ((desc = XVncExtGetParamDesc(dpy, param)) == NULL)
    return False;
  printf("%s\n", desc);

  XFree(desc);
  return True;
}


int main(int argc, char **argv)
{
  int i;
  Bool verbose = False;

  programName = argv[0];

  if (argc < 2) usage();

  for (i = 1; i < argc; i++) {
    if (!strncmp(argv[i], "-di", 3)) {
      if (++i >= argc) usage();
      displayname = argv[i];
    } else if (!strncmp(argv[i], "-h", 2) || !strcmp(argv[i], "-?"))
      usage();
    else if (!strncmp(argv[i], "-v", 2)) verbose = True;
    else break;
  }

  if (!(dpy = XOpenDisplay(displayname)))
    FatalError("Unable to open display \"%s\"", XDisplayName(displayname));

  if (!XVncExtQueryExtension(dpy, &vncExtEventBase, &vncExtErrorBase))
    FatalError("No VNC extension on display %s", XDisplayName(displayname));

  if (i < argc) {
    for (; i < argc; i++) {
      if (!strncmp(argv[i], "-g", 2)) {
        if (++i >= argc) usage();
        if (verbose) printf("%s = ", argv[i]);
        if (!PrintParameterValue(argv[i]))
          FatalError("Could not get parameter %s.\n", argv[i]);
        if (verbose) {
          if (!PrintParameterDesc(argv[i]))
            FatalError("Could not get description for parameter %s.\n", argv[i]);
        }
      } else if (!strncmp(argv[i], "-de", 3)) {
        if (++i >= argc) usage();
        if (verbose) printf("%s:\n", argv[i]);
        if (!PrintParameterDesc(argv[i]))
          FatalError("Could not get description for parameter %s.\n", argv[i]);
      } else if (!strncmp(argv[i], "-l", 2)) {
        int nParams;
        char **list = XVncExtListParams(dpy, &nParams);

        if (!list)
          FatalError("Could not list parameters\n");
        for (i = 0; i < nParams; i++) {
          printf("%s", list[i]);
          if (verbose) {
            printf(" = ");
            if (!PrintParameterValue(list[i]))
              printf("*** COULD NOT READ PARAMETER (parameter may be write-only) ***\n");
            if (!PrintParameterDesc(list[i]))
              printf("*** COULD NOT GET PARAMETER DESCRIPTION ***\n");
          }
          printf("\n");
        }
        XVncExtFreeParamList(list);
      } else if (!strncmp(argv[i], "-s", 2)) {
        char *ptr;

        if (++i >= argc) usage();
        if (!XVncExtSetParam(dpy, argv[i]))
          FatalError("Could not set parameter %s.\n", argv[i]);
        ptr = strstr(argv[i], "=");
        if (ptr) *ptr = '\0';
        printf("%s = ", argv[i]);
        PrintParameterValue(argv[i]);
      } else {
        char *ptr;

        if (!XVncExtSetParam(dpy, argv[i]))
          FatalError("Could not set parameter %s.\n", argv[i]);
        ptr = strstr(argv[i], "=");
        if (ptr) *ptr = '\0';
        printf("%s = ", argv[i]);
        PrintParameterValue(argv[i]);
      }
    }
  } else usage();

  XCloseDisplay(dpy);

  return 0;
}
