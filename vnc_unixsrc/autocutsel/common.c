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
 
#include "common.h"

Widget box;
Display* dpy;
XtAppContext context;
Atom selection;
int buffer;
OptionsRec options;

void PrintValue(char *value, int length)
{
  unsigned char c;
  int len = 0;
  
  putc('"', stdout);
  for (; length > 0; length--, value++) {
    c = (unsigned char)*value;
    switch (c) {
    case '\n':
      printf("\\n");
      break;
    case '\r':
      printf("\\r");
      break;
    case '\t':
      printf("\\t");
      break;
    default:
      if (c < 32 || c > 127)
        printf("\\x%02X", c);
      else
        putc(c, stdout);
    }
    len++;
    if (len >= 48) {
      printf("\"...");
      return;
    }
  }
  putc('"', stdout);
}
  


// called when someone requests the selection value
Boolean ConvertSelection(Widget w, Atom *selection, Atom *target,
                                Atom *type, XtPointer *value,
                                unsigned long *length, int *format)
{
  Display* d = XtDisplay(w);
  XSelectionRequestEvent* req =
    XtGetSelectionRequest(w, *selection, (XtRequestId)NULL);
  
  if (options.debug) {
    printf("Window 0x%lx requested %s of selection %s.\n",
      req->requestor,
      XGetAtomName(d, *target),
      XGetAtomName(d, *selection));
  }
  
  if (*target == XA_TARGETS(d)) {
    Atom *targetP, *atoms;
    XPointer std_targets;
    unsigned long std_length;
    int i;
    
    XmuConvertStandardSelection(w, req->time, selection, target, type,
        &std_targets, &std_length, format);
    *value = XtMalloc(sizeof(Atom)*(std_length + 4));
    targetP = *(Atom**)value;
    atoms = targetP;
    *length = std_length + 4;
    *targetP++ = XA_STRING;
    *targetP++ = XA_TEXT(d);
    *targetP++ = XA_LENGTH(d);
    *targetP++ = XA_LIST_LENGTH(d);
    memmove( (char*)targetP, (char*)std_targets, sizeof(Atom)*std_length);
    XtFree((char*)std_targets);
    *type = XA_ATOM;
    *format = 32;
    
    if (options.debug) {
      printf("Targets are: ");
      for (i=0; i<*length; i++)
        printf("%s ", XGetAtomName(d, atoms[i]));
      printf("\n");
    }

    return True;
  }
  
  if (*target == XA_STRING || *target == XA_TEXT(d)) {
    *type = XA_STRING;
    *value = XtMalloc((Cardinal) options.length);
    memmove((char *)*value, options.value, options.length);
    *length = options.length;
    *format = 8;

    if (options.debug) {
      printf("Returning ");
      PrintValue((char*)*value, *length);
      printf("\n");
    }
   
    return True;
  }
  
  if (*target == XA_LIST_LENGTH(d)) {
    CARD32 *temp = (CARD32 *) XtMalloc(sizeof(CARD32));
    *temp = 1L;
    *value = (XtPointer) temp;
    *type = XA_INTEGER;
    *length = 1;
    *format = 32;

    if (options.debug)
      printf("Returning %ld\n", *temp);

    return True;
  }
  
  if (*target == XA_LENGTH(d)) {
    CARD32 *temp = (CARD32 *) XtMalloc(sizeof(CARD32));
    *temp = options.length;
    *value = (XtPointer) temp;
    *type = XA_INTEGER;
    *length = 1;
    *format = 32;

    if (options.debug)
      printf("Returning %ld\n", *temp);

    return True;
  }
  
  if (XmuConvertStandardSelection(w, req->time, selection, target, type,
          (XPointer *)value, length, format)) {
    printf("Returning conversion of standard selection\n");
    return True;
  }
   
  /* else */
  if (options.debug)
    printf("Target not supported\n");

  return False;
}
