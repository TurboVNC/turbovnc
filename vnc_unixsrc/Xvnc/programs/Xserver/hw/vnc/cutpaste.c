/*
 * cutpaste.c - routines to deal with cut & paste buffers / selection.
 */

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

#include <stdio.h>
#define NEED_EVENTS
#include <X.h>
#include <Xproto.h>
#include "rfb.h"
#include "selection.h"
#include "input.h"
#include <Xatom.h>

extern WindowPtr *WindowTable; /* Why isn't this in a header file? */
extern Selection *CurrentSelections;
extern int NumCurrentSelections;


static Bool inSetXCutText = FALSE;

/*
 * rfbSetXCutText sets the cut buffer to be the given string.  We also clear
 * the primary selection.  Ideally we'd like to set it to the same thing, but I
 * can't work out how to do that without some kind of helper X client.
 */

void
rfbSetXCutText(char *str, int len)
{
    int i = 0;

    inSetXCutText = TRUE;
    ChangeWindowProperty(WindowTable[0], XA_CUT_BUFFER0, XA_STRING,
			 8, PropModeReplace, len,
			 (pointer)str, TRUE);
    
    while ((i < NumCurrentSelections) && 
	   CurrentSelections[i].selection != XA_PRIMARY)
	i++;

    if (i < NumCurrentSelections) {
	xEvent event;

	if (CurrentSelections[i].client) {
	    event.u.u.type = SelectionClear;
	    event.u.selectionClear.time = GetTimeInMillis();
	    event.u.selectionClear.window = CurrentSelections[i].window;
	    event.u.selectionClear.atom = CurrentSelections[i].selection;
	    (void) TryClientEvents (CurrentSelections[i].client, &event, 1,
				NoEventMask, NoEventMask /* CantBeFiltered */,
				NullGrab);
	}

	CurrentSelections[i].window = None;
	CurrentSelections[i].pWin = NULL;
	CurrentSelections[i].client = NullClient;
    }

    inSetXCutText = FALSE;
}


void rfbGotXCutText(char *str, int len)
{
    if (!inSetXCutText)
	rfbSendServerCutText(str, len);
}
