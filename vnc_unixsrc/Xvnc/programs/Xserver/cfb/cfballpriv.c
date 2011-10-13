/*
 * $Xorg: cfballpriv.c,v 1.4 2001/02/09 02:04:37 xorgcvs Exp $
 *
Copyright 1991, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 *
 * Author:  Keith Packard, MIT X Consortium
 */
/* $XFree86: xc/programs/Xserver/cfb/cfballpriv.c,v 1.11 2001/01/30 22:06:15 tsi Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xmd.h>
#include "servermd.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "resource.h"
#include "colormap.h"
#include "colormapst.h"
#include "cfb.h"
#include "mi.h"
#include "mistruct.h"
#include "dix.h"
#include "cfbmskbits.h"
#include "mibstore.h"

#if 1 || PSZ==8
int cfbWindowPrivateIndex = -1;
int cfbGCPrivateIndex = -1;
#endif
#ifdef CFB_NEED_SCREEN_PRIVATE
int cfbScreenPrivateIndex = -1;
static unsigned long cfbGeneration = 0;
#endif


Bool
cfbAllocatePrivates(pScreen, window_index, gc_index)
    ScreenPtr	pScreen;
    int		*window_index, *gc_index;
{
    if (!window_index || !gc_index ||
	(*window_index == -1 && *gc_index == -1))
    {
    	if (!mfbAllocatePrivates(pScreen,
			     	 &cfbWindowPrivateIndex, &cfbGCPrivateIndex))
	    return FALSE;
    	if (window_index)
	    *window_index = cfbWindowPrivateIndex;
    	if (gc_index)
	    *gc_index = cfbGCPrivateIndex;
    }
    else
    {
	cfbWindowPrivateIndex = *window_index;
	cfbGCPrivateIndex = *gc_index;
    }
    if (!AllocateWindowPrivate(pScreen, cfbWindowPrivateIndex,
			       sizeof(cfbPrivWin)) ||
	!AllocateGCPrivate(pScreen, cfbGCPrivateIndex, sizeof(cfbPrivGC)))
	return FALSE;
#ifdef CFB_NEED_SCREEN_PRIVATE
    if (cfbGeneration != serverGeneration)
    {
      cfbScreenPrivateIndex = AllocateScreenPrivateIndex ();
      cfbGeneration = serverGeneration;
    }
    if (cfbScreenPrivateIndex == -1)
	return FALSE;
#endif
    return TRUE;
}
