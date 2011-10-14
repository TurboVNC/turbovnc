/* $XdotOrg: xc/programs/Xserver/xkb/ddxFakeMtn.c,v 1.5 2005/07/03 07:02:09 daniels Exp $ */
/* $Xorg: ddxFakeMtn.c,v 1.3 2000/08/17 19:53:45 cpqbld Exp $ */
/************************************************************
Copyright (c) 1993 by Silicon Graphics Computer Systems, Inc.

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of Silicon Graphics not be 
used in advertising or publicity pertaining to distribution 
of the software without specific prior written permission.
Silicon Graphics makes no representation about the suitability 
of this software for any purpose. It is provided "as is"
without any express or implied warranty.

SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS 
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/
/* $XFree86: xc/programs/Xserver/xkb/ddxFakeMtn.c,v 1.5 2003/09/13 16:39:01 dawes Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#define	NEED_EVENTS 1
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/keysym.h>
#include "inputstr.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include <X11/extensions/XKBsrv.h>
#include <X11/extensions/XI.h>

#ifdef PANORAMIX
#include "panoramiX.h"
#include "panoramiXsrv.h"
#endif

#include "mipointer.h"
#include "mipointrst.h"

void
XkbDDXFakePointerMotion(unsigned flags,int x,int y)
{
int 		   oldX,oldY;
ScreenPtr	   pScreen, oldScreen;

    GetSpritePosition(&oldX, &oldY);
    pScreen = oldScreen = GetSpriteWindow()->drawable.pScreen;

#ifdef PANORAMIX
    if (!noPanoramiXExtension) {
	BoxRec box;
	int i;

	if(!POINT_IN_REGION(pScreen, &XineramaScreenRegions[pScreen->myNum],
							    oldX, oldY, &box)) {
	    FOR_NSCREENS(i) {
		if(i == pScreen->myNum)
		    continue;
		if(POINT_IN_REGION(pScreen, &XineramaScreenRegions[i],
				   oldX, oldY, &box)) {
		    pScreen = screenInfo.screens[i];
		    break;
		}
	    }
	}
	oldScreen = pScreen;

	if (flags&XkbSA_MoveAbsoluteX)
	     oldX=  x;
	else oldX+= x;
	if (flags&XkbSA_MoveAbsoluteY)
	     oldY=  y;
	else oldY+= y;

	if(!POINT_IN_REGION(pScreen, &XineramaScreenRegions[pScreen->myNum],
							    oldX, oldY, &box)) {
	    FOR_NSCREENS(i) {
		if(i == pScreen->myNum)
		    continue;
		if(POINT_IN_REGION(pScreen, &XineramaScreenRegions[i],
				   oldX, oldY, &box)) {
		    pScreen = screenInfo.screens[i];
		    break;
		}
	    }
	}
	oldX -= panoramiXdataPtr[pScreen->myNum].x;
	oldY -= panoramiXdataPtr[pScreen->myNum].y;
    }
    else
#endif
    {
	if (flags&XkbSA_MoveAbsoluteX)
	     oldX=  x;
	else oldX+= x;
	if (flags&XkbSA_MoveAbsoluteY)
	     oldY=  y;
	else oldY+= y;

#define GetScreenPrivate(s) ((miPointerScreenPtr) ((s)->devPrivates[miPointerScreenIndex].ptr))	
	(*(GetScreenPrivate(oldScreen))->screenFuncs->CursorOffScreen)
	    (&pScreen, &oldX, &oldY);
    }

    if (pScreen != oldScreen)
	NewCurrentScreen(pScreen, oldX, oldY);
    if (pScreen->SetCursorPosition)
	(*pScreen->SetCursorPosition)(pScreen, oldX, oldY, TRUE);
}
