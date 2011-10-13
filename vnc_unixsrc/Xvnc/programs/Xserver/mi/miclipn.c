/* $Xorg: miclipn.c,v 1.4 2001/02/09 02:05:20 xorgcvs Exp $ */
/*

Copyright 1990, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/programs/Xserver/mi/miclipn.c,v 1.3 2001/08/06 21:46:04 dawes Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include "windowstr.h"
#include "scrnintstr.h"
#include "mi.h"

static void	(*clipNotify)(WindowPtr,int,int) = 0;
static void	(*ClipNotifies[MAXSCREENS])(WindowPtr,int,int);

static void
miClipNotifyWrapper(
    WindowPtr pWin,
    int dx, 
    int dy )
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
miClipNotify (
    void (*func)(
        WindowPtr /* pWin */,
        int /* dx */,
        int /* dy */
		) )
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
