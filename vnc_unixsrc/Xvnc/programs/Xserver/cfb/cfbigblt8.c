/*
 * $Xorg: cfbigblt8.c,v 1.4 2001/02/09 02:04:38 xorgcvs Exp $
 *
Copyright 1990, 1998  The Open Group

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

/* $XFree86: xc/programs/Xserver/cfb/cfbigblt8.c,v 1.5 2001/10/28 03:33:01 tsi Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include	<X11/X.h>
#include	<X11/Xmd.h>
#include	<X11/Xproto.h>
#include	"mi.h"
#include	"cfb.h"
#include	<X11/fonts/fontstruct.h>
#include	"dixfontstr.h"
#include	"gcstruct.h"
#include	"windowstr.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"cfbmskbits.h"
#include	"cfb8bit.h"

void
cfbImageGlyphBlt8 (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    int		    x, y;
    unsigned int    nglyph;
    CharInfoPtr	    *ppci;
    pointer	    pglyphBase;
{
    ExtentInfoRec info;		/* used by QueryGlyphExtents() */
    xRectangle backrect;
    int		fgPixel;
    cfbPrivGC	*priv;

    /*
     * We can't avoid GC validations if calling mi functions.
     */
    if ((pGC->ops->PolyFillRect == miPolyFillRect) ||
        (pGC->ops->PolyGlyphBlt == miPolyGlyphBlt))
    {
        miImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
        return;
    }

    QueryGlyphExtents(pGC->font, ppci, (unsigned long)nglyph, &info);

    if (info.overallWidth >= 0)
    {
    	backrect.x = x;
    	backrect.width = info.overallWidth;
    }
    else
    {
	backrect.x = x + info.overallWidth;
	backrect.width = -info.overallWidth;
    }
    backrect.y = y - FONTASCENT(pGC->font);
    backrect.height = FONTASCENT(pGC->font) + FONTDESCENT(pGC->font);

    priv = cfbGetGCPrivate(pGC);

    /* this code cheats by knowing that ValidateGC isn't
     * necessary for PolyFillRect
     */

    fgPixel = pGC->fgPixel;

    pGC->fgPixel = pGC->bgPixel;
    priv->xor = PFILL(pGC->bgPixel);

    (*pGC->ops->PolyFillRect) (pDrawable, pGC, 1, &backrect);

    pGC->fgPixel = fgPixel;

    priv->xor = PFILL(pGC->fgPixel);

    (*pGC->ops->PolyGlyphBlt) (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
    
}
