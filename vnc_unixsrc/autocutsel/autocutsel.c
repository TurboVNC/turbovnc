/*
 * autocutsel by Michael Witrant <mike @ lepton . fr>
 * Synchronizes the cutbuffer and the selection
 * Copyright (c) 2001-2006 Michael Witrant.
 * 
 * Most code taken from:
 * * clear-cut-buffers.c by "E. Jay Berkenbilt" <ejb @ ql . org>
 *   in this messages:
 *     http://boudicca.tux.org/mhonarc/ma-linux/2001-Feb/msg00824.html
 * 
 * * xcutsel.c by Ralph Swick, DEC/Project Athena
 *   from the XFree86 project: http://www.xfree86.org/
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This program is distributed under the terms
 * of the GNU General Public License (read the COPYING file)
 * 
 */

#include "common.h"

static XrmOptionDescRec optionDesc[] = {
  {"-selection", "selection", XrmoptionSepArg, NULL},
  {"-select",    "selection", XrmoptionSepArg, NULL},
  {"-sel",       "selection", XrmoptionSepArg, NULL},
  {"-s",         "selection", XrmoptionSepArg, NULL},
  {"-cutbuffer", "cutBuffer", XrmoptionSepArg, NULL},
  {"-cut",       "cutBuffer", XrmoptionSepArg, NULL},
  {"-c",         "cutBuffer", XrmoptionSepArg, NULL},
  {"-debug",     "debug",     XrmoptionNoArg,  "on"},
  {"-d",         "debug",     XrmoptionNoArg,  "on"},
  {"-verbose",   "verbose",   XrmoptionNoArg,  "on"},
  {"-v",         "verbose",   XrmoptionNoArg,  "on"},
  {"-fork",      "fork",      XrmoptionNoArg,  "on"},
  {"-f",         "fork",      XrmoptionNoArg,  "on"},
  {"-pause",     "pause",     XrmoptionSepArg, NULL},
  {"-p",         "pause",     XrmoptionSepArg, NULL},
  {"-buttonup",  "buttonup",  XrmoptionNoArg,  "on"},
};

int Syntax(char *call)
{
  fprintf (stderr,
    "usage:  %s [-selection <name>] [-cutbuffer <number>]"
    " [-pause <milliseconds>] [-debug] [-verbose] [-fork] [-buttonup]\n", 
    call);
  exit (1);
}

#define Offset(field) XtOffsetOf(OptionsRec, field)

static XtResource resources[] = {
  {"selection", "Selection", XtRString, sizeof(String),
    Offset(selection_name), XtRString, "CLIPBOARD"},
  {"cutBuffer", "CutBuffer", XtRInt, sizeof(int),
    Offset(buffer), XtRImmediate, (XtPointer)0},
  {"debug", "Debug", XtRString, sizeof(String),
    Offset(debug_option), XtRString, "off"},
  {"verbose", "Verbose", XtRString, sizeof(String),
    Offset(verbose_option), XtRString, "off"},
  {"fork", "Fork", XtRString, sizeof(String),
    Offset(fork_option), XtRString, "off"},
  {"kill", "kill", XtRString, sizeof(String),
    Offset(kill), XtRString, "off"},
  {"pause", "Pause", XtRInt, sizeof(int),
    Offset(pause), XtRImmediate, (XtPointer)500},
  {"buttonup", "ButtonUp", XtRString, sizeof(String),
    Offset(buttonup_option), XtRString, "off"},
};

#undef Offset

static void CloseStdFds()
{
  int fd;

  for (fd = 0; fd < 3; fd++)
    close (fd);
}

static RETSIGTYPE Terminate(int caught)
{
  exit (0);
}

static void TrapSignals()
{
  int catch;
  struct sigaction action;

  sigemptyset (&action.sa_mask);
  action.sa_flags = 0;
  for (catch = 1; catch < NSIG; catch++) {
    if (catch != SIGKILL) {
      action.sa_handler = Terminate;
      sigaction (catch, &action, (struct sigaction *) 0);
    }
  }
}

// Called when we no longer own the selection
static void LoseSelection(Widget w, Atom *selection)
{
  if (options.debug)
    printf("Selection lost\n");
  options.own_selection = 0;
}

// Returns true if value (or length) is different
// than current ones.
static int ValueDiffers(char *value, int length)
{
  return (!options.value ||
    length != options.length ||
    memcmp(options.value, value, options.length));
}

// Update the current value
static void ChangeValue(char *value, int length)
{
  if (options.value)
    XtFree(options.value);

  options.length = length;
  options.value = XtMalloc(options.length);
  if (!options.value)
    printf("WARNING: Unable to allocate memory to store the new value\n");
  else {
    memcpy(options.value, value, options.length);

    if (options.debug) {
      printf("New value saved: ");
      PrintValue(options.value, options.length);
      printf("\n");
    }
  }
}

// Called just before owning the selection, to ensure we don't
// do it if the selection already has the same value
static void OwnSelectionIfDiffers(Widget w, XtPointer client_data,
                                  Atom *selection, Atom *type, XtPointer value,
                                  unsigned long *received_length, int *format)
{
  int length = *received_length;
  
  if (*type == 0 || 
      *type == XT_CONVERT_FAIL || 
      length == 0 || 
      ValueDiffers(value, length)) {
    if (options.debug)
      printf("Selection is out of date. Owning it\n");
    
    if (options.verbose)
    {
      printf("cut -> sel: ");
      PrintValue(options.value, options.length);
      printf("\n");
    }
    
    if (XtOwnSelection(box, options.selection,
        0, //XtLastTimestampProcessed(dpy),
        ConvertSelection, LoseSelection, NULL) == True) {
      if (options.debug)
        printf("Selection owned\n");
      options.own_selection = 1;
    }
    else
      printf("WARNING: Unable to own selection!\n");
  }
  XtFree(value);
}

// Look for change in the buffer, and update the selection if necessary
static void CheckBuffer()
{
  char *value;
  int length;
  
  value = XFetchBuffer(dpy, &length, buffer);
  
  if (length > 0 && ValueDiffers(value, length)) {
    if (options.debug) {
      printf("Buffer changed: ");
      PrintValue(value, length);
      printf("\n");
    }
    
    ChangeValue(value, length);
    XtGetSelectionValue(box, selection, XA_STRING,
      OwnSelectionIfDiffers, NULL,
      CurrentTime);
  }

  XFree(value);
}

// Called when the requested selection value is availiable
static void SelectionReceived(Widget w, XtPointer client_data, Atom *selection,
                              Atom *type, XtPointer value,
                              unsigned long *received_length, int *format)
{
  int length = *received_length;
  
  if (*type != 0 && *type != XT_CONVERT_FAIL) {
    if (length > 0 && ValueDiffers(value, length)) {
      if (options.debug) {
        printf("Selection changed: ");
        PrintValue((char*)value, length);
        printf("\n");
      }
    
      ChangeValue((char*)value, length);
      if (options.verbose) {
        printf("sel -> cut: ");
        PrintValue(options.value, options.length);
        printf("\n");
      }
      if (options.debug)
        printf("Updating buffer\n");
      
      XStoreBuffer(XtDisplay(w),
             (char*)options.value,
             (int)(options.length),
             buffer );
      
      XtFree(value);
      return;
    }
  }
  XtFree(value);
  
  // Unless a new selection value is found, check the buffer value
  CheckBuffer();
}

// Called each <pause arg=500> milliseconds
void timeout(XtPointer p, XtIntervalId* i)
{
  if (options.own_selection)
    CheckBuffer();
  else {
    int get_value = 1;
    if (options.buttonup) {
      int screen_num = DefaultScreen(dpy);
      int root_x, root_y, win_x, win_y;
      unsigned int mask;
      Window root_wnd, child_wnd;
      XQueryPointer(dpy, RootWindow(dpy,screen_num), &root_wnd, &child_wnd,
        &root_x, &root_y, &win_x, &win_y, &mask);
      if (mask & (ShiftMask | Button1Mask))
        get_value = 0;
    }
    if (get_value)
      XtGetSelectionValue(box, selection, XA_STRING,    
        SelectionReceived, NULL,
        CurrentTime);
  }
  
  XtAppAddTimeOut(context, options.pause, timeout, 0);
}

int main(int argc, char* argv[])
{
  Widget top;
  top = XtVaAppInitialize(&context, "AutoCutSel",
        optionDesc, XtNumber(optionDesc), &argc, argv, NULL,
        XtNoverrideRedirect, True,
        XtNgeometry, "-10-10",
        NULL);

  if (argc != 1) Syntax(argv[0]);

  XtGetApplicationResources(top, (XtPointer)&options,
          resources, XtNumber(resources),
          NULL, ZERO );


  if (strcmp(options.debug_option, "on") == 0)
    options.debug = 1;
  else
    options.debug = 0;
  
  if (strcmp(options.verbose_option, "on") == 0)
    options.verbose = 1;
  else
    options.verbose = 0;
  
  if (options.debug || options.verbose)
    printf("autcutsel v%s\n", VERSION);
   
  if (strcmp(options.buttonup_option, "on") == 0)
    options.buttonup = 1;
  else
    options.buttonup = 0;
  
  if (strcmp(options.fork_option, "on") == 0) {
    options.fork = 1;
    options.verbose = 0;
    options.debug = 0;
  }
  else
    options.fork = 0;

  if (options.fork) {
    if (getppid () != 1) {
#ifdef SETPGRP_VOID
      setpgrp();
#else
      setpgrp(0, 0);
#endif
      switch (fork()) {
      case -1:
        fprintf (stderr, "could not fork, exiting\n");
        return errno;
      case 0:
        sleep(3); /* Wait for father to exit */
        chdir("/");
        TrapSignals();
        CloseStdFds();
        break;
      default:
        return 0;
      }
    }
  }

  options.value = NULL;
  options.length = 0;

  options.own_selection = 0;
   
  box = XtCreateManagedWidget("box", boxWidgetClass, top, NULL, 0);
  dpy = XtDisplay(top);

  selection = XInternAtom(dpy, options.selection_name, 0);
  options.selection = selection;
  buffer = 0;
   
  options.value = XFetchBuffer(dpy, &options.length, buffer);

  XtAppAddTimeOut(context, options.pause, timeout, 0);
  XtRealizeWidget(top);
  XUnmapWindow(XtDisplay(top), XtWindow(top));
  XtAppMainLoop(context);
  
  return 0;
}
