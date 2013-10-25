/*
 * $XFree86: xc/programs/Xserver/randr/mirandr.c,v 1.5 2001/06/04 09:45:40 keithp Exp $
 *
 * Copyright © 2000, Compaq Computer Corporation, 
 * Copyright © 2002, Hewlett Packard, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Compaq or HP not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  HP makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * HP DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL HP
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Jim Gettys, HP Labs, Hewlett-Packard, Inc.
 */


#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "scrnintstr.h"
#include "mi.h"
#include <X11/extensions/randr.h>
#include "randrstr.h"
#include <stdio.h>

/*
 * This function assumes that only a single depth can be
 * displayed at a time, but that all visuals of that depth
 * can be displayed simultaneously.  It further assumes that
 * only a single size is available.  Hardware providing
 * additional capabilties should use different code.
 * XXX what to do here....
 */

Bool
miRRGetInfo (ScreenPtr pScreen, Rotation *rotations)
{
    int	i;
    Bool setConfig = FALSE;
    
    *rotations = RR_Rotate_0;
    for (i = 0; i < pScreen->numDepths; i++)
    {
	if (pScreen->allowedDepths[i].numVids)
	{
		RRScreenSizePtr		pSize;

		pSize = RRRegisterSize (pScreen,
					pScreen->width,
					pScreen->height,
					pScreen->mmWidth,
					pScreen->mmHeight);
		if (!pSize)
		    return FALSE;
		if (!setConfig)
		{
		    RRSetCurrentConfig (pScreen, RR_Rotate_0, 0, pSize);
		    setConfig = TRUE;
		}
	}
    }
    return TRUE;
}

/*
 * Any hardware that can actually change anything will need something
 * different here
 */
Bool
miRRSetConfig (ScreenPtr	pScreen,
	       Rotation		rotation,
	       int		rate,
	       RRScreenSizePtr	pSize)
{
    return TRUE;
}


Bool
miRandRInit (ScreenPtr pScreen)
{
    rrScrPrivPtr    rp;
    
    if (!RRScreenInit (pScreen))
	return FALSE;
    rp = rrGetScrPriv(pScreen);
    rp->rrGetInfo = miRRGetInfo;
    rp->rrSetConfig = miRRSetConfig;
    return TRUE;
}
