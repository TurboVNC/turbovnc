/*
 * cmap.c
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

/*

Copyright (c) 1993  X Consortium

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

#include <stdio.h>
#include "scrnintstr.h"
#include "resource.h"
#include "colormapst.h"
#include "rfb.h"

ColormapPtr rfbInstalledColormap;

int
rfbListInstalledColormaps(pScreen, pmaps)
    ScreenPtr	pScreen;
    Colormap	*pmaps;
{
    /* By the time we are processing requests, we can guarantee that there
     * is always a colormap installed */
    *pmaps = rfbInstalledColormap->mid;
    return (1);
}


void
rfbInstallColormap(pmap)
    ColormapPtr	pmap;
{
    ColormapPtr oldpmap = rfbInstalledColormap;

    if (pmap != oldpmap) {

	if(oldpmap != (ColormapPtr)None)
	    WalkTree(pmap->pScreen, TellLostMap, (char *)&oldpmap->mid);
	/* Install pmap */
	rfbInstalledColormap = pmap;
	WalkTree(pmap->pScreen, TellGainedMap, (char *)&pmap->mid);

	rfbSetClientColourMaps(0, 0);
    }
}

void
rfbUninstallColormap(pmap)
    ColormapPtr	pmap;
{
    ColormapPtr curpmap = rfbInstalledColormap;

    if(pmap == curpmap)
    {
	if (pmap->mid != pmap->pScreen->defColormap)
	{
	    curpmap = (ColormapPtr) LookupIDByType(pmap->pScreen->defColormap,
						   RT_COLORMAP);
	    (*pmap->pScreen->InstallColormap)(curpmap);
	}
    }
}


/*
 * rfbStoreColors.  We have a set of pixels but they may be in any order.
 * If some of them happen to be in continuous ascending order then we can
 * group them together into a single call to rfbSetClientColourMaps.
 */

void
rfbStoreColors(pmap, ndef, pdefs)
    ColormapPtr pmap;
    int         ndef;
    xColorItem  *pdefs;
{
    int i;
    int first = -1;
    int n = 0;

    if (pmap == rfbInstalledColormap) {
	for (i = 0; i < ndef; i++) {
	    if ((first != -1) && (first + n == pdefs[i].pixel)) {
		n++;
	    } else {
		if (first != -1) {
		    rfbSetClientColourMaps(first, n);
		}
		first = pdefs[i].pixel;
		n = 1;
	    }
	}
	rfbSetClientColourMaps(first, n);
    }
}
