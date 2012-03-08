/* $XConsortium: miclipn.c,v 5.1 94/04/17 20:27:26 rws Exp $ */
/*

Copyright (c) 1990  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

#include "X.h"
#include "windowstr.h"
#include "scrnintstr.h"

static void	(*clipNotify)() = 0;
static void	(*ClipNotifies[MAXSCREENS])();

static void
miClipNotifyWrapper(pWin, dx, dy)
    WindowPtr pWin;
    int dx, dy;
{
    if (clipNotify)
	(*clipNotify)(pWin, dx, dy);
    if (ClipNotifies[pWin->drawable.pScreen->myNum])
	(*ClipNotifies[pWin->drawable.pScreen->myNum])(pWin, dx, dy);
}

/*
 * miClipNotify --
 *	Hook to let DDX request notification when the clipList of
 *	a window is recomputed on any screen.  For R4 compatibility;
 *	better if you wrap the ClipNotify screen proc yourself.
 */

static unsigned long clipGeneration = 0;

void
miClipNotify (func)
    void (*func)();
{
    int i;

    clipNotify = func;
    if (clipGeneration != serverGeneration)
    {
	clipGeneration = serverGeneration;
	for (i = 0; i < screenInfo.numScreens; i++)
	{
	    ClipNotifies[i] = screenInfo.screens[i]->ClipNotify;
	    screenInfo.screens[i]->ClipNotify = miClipNotifyWrapper;
	}
    }
}
