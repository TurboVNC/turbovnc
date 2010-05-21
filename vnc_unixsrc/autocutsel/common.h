/*
 * autocutsel by Michael Witrant <mike @ lepton . fr>
 * Manipulates the cutbuffer and the selection
 * Copyright (c) 2001-2006 Michael Witrant.
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

#include "config.h"

#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/StdSel.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Shell.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xmd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>


typedef struct {
  String  selection_name;
  int     buffer;
  String  debug_option;
  String  verbose_option;
  String  fork_option;
  String  buttonup_option;
  String  kill;
  int     pause;
  int     debug; 
  int     verbose; 
  int     fork;
  Atom    selection;
  char*   value;
  int     length;
  int     own_selection;
  int     buttonup;
} OptionsRec;

extern Widget box;
extern Display* dpy;
extern XtAppContext context;
extern Atom selection;
extern int buffer;
extern OptionsRec options;


void PrintValue(char *value, int length);
Boolean ConvertSelection(Widget w, Atom *selection, Atom *target,
                                Atom *type, XtPointer *value,
                                unsigned long *length, int *format);
