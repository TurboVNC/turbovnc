/* $XFree86: xc/programs/Xserver/mfb/mfbcmap.c,v 1.7tsi Exp $ */
/***********************************************************

Copyright 1987, 1998  The Open Group

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


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $Xorg: mfbcmap.c,v 1.4 2001/02/09 02:05:18 xorgcvs Exp $ */
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include "scrnintstr.h"
#include "colormapst.h"
#include "resource.h"
#include "micmap.h"
#include "mfb.h"

/* A monochrome frame buffer is a static gray colormap with two entries.
 * We have a "required list" of length 1.  Because we can only support 1
 * colormap, we never have to change it, but we may have to change the 
 * name we call it.  If someone installs a new colormap, we know it must
 * look just like the old one (because we've checked in dispatch that it was
 * a valid colormap identifier, and all the colormap IDs for this device
 * look the same).  Nevertheless, we still have to uninstall the old colormap
 * and install the new one.  Similarly, if someone uninstalls a colormap,
 * we have to install the default map, even though we know those two looked
 * alike.  
 * The required list concept is pretty much irrelevant when you can only
 * have one map installed at a time.  
 */

int
mfbListInstalledColormaps(pScreen, pmaps)
    ScreenPtr	pScreen;
    Colormap	*pmaps;
{
    return miListInstalledColormaps(pScreen, pmaps);
}


void
mfbInstallColormap(pmap)
    ColormapPtr	pmap;
{
    miInstallColormap(pmap);
}

void
mfbUninstallColormap(pmap)
    ColormapPtr	pmap;
{
    miUninstallColormap(pmap);
}

/*ARGSUSED*/
void
mfbResolveColor (pred, pgreen, pblue, pVisual)
    unsigned short	*pred;
    unsigned short	*pgreen;
    unsigned short	*pblue;
    VisualPtr		pVisual;
{
    /* 
     * Gets intensity from RGB.  If intensity is >= half, pick white, else
     * pick black.  This may well be more trouble than it's worth.
     */
    *pred = *pgreen = *pblue = 
        (((30L * *pred +
           59L * *pgreen +
           11L * *pblue) >> 8) >= (((1<<8)-1)*50)) ? ~0 : 0;
}

Bool
mfbCreateColormap(pMap)
    ColormapPtr	pMap;
{
    ScreenPtr	pScreen;
    unsigned short  red0, green0, blue0;
    unsigned short  red1, green1, blue1;
    Pixel pix;
    
    pScreen = pMap->pScreen;
    if (pScreen->whitePixel == 0)
    {
	red0 = green0 = blue0 = ~0;
	red1 = green1 = blue1 = 0;
    }
    else
    {
	red0 = green0 = blue0 = 0;
	red1 = green1 = blue1 = ~0;
    }

    /* this is a monochrome colormap, it only has two entries, just fill
     * them in by hand.  If it were a more complex static map, it would be
     * worth writing a for loop or three to initialize it */

    /* this will be pixel 0 */
    pix = 0;
    if (AllocColor(pMap, &red0, &green0, &blue0, &pix, 0) != Success)
	return FALSE;

    /* this will be pixel 1 */
    if (AllocColor(pMap, &red1, &green1, &blue1, &pix, 0) != Success)
	return FALSE;
    return TRUE;
}

/*ARGSUSED*/
void
mfbDestroyColormap (pMap)
    ColormapPtr	pMap;
{
    return;
}

Bool
mfbCreateDefColormap (pScreen)
    ScreenPtr	pScreen;
{
    return miCreateDefColormap(pScreen);
}
