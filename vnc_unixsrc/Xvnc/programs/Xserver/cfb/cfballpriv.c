/*
 * $XConsortium: cfballpriv.c,v 1.5 94/04/17 20:28:42 dpw Exp $
 *
Copyright (c) 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#include "X.h"
#include "Xmd.h"
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

int cfbWindowPrivateIndex;
int cfbGCPrivateIndex;
#ifdef CFB_NEED_SCREEN_PRIVATE
int cfbScreenPrivateIndex;
#endif

extern RegionPtr (*cfbPuntCopyPlane)();

Bool
cfbAllocatePrivates(pScreen, window_index, gc_index)
    ScreenPtr	pScreen;
    int		*window_index, *gc_index;
{
    if (!window_index || !gc_index ||
	*window_index == -1 && *gc_index == -1)
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
    cfbPuntCopyPlane = miCopyPlane;
#ifdef CFB_NEED_SCREEN_PRIVATE
    cfbScreenPrivateIndex = AllocateScreenPrivateIndex ();
    if (cfbScreenPrivateIndex == -1)
	return FALSE;
#endif
    return TRUE;
}
