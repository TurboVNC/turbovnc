/* $XFree86: xc/programs/Xserver/mfb/mfbimage.c,v 1.5 2001/01/17 22:37:03 dawes Exp $ */
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
/* $Xorg: mfbimage.c,v 1.4 2001/02/09 02:05:19 xorgcvs Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>

#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "gcstruct.h"

#include "mfb.h"
#include "mi.h"
#include <X11/Xmd.h>

#include "maskbits.h"

#include "servermd.h"

/* Put and Get images on a monochrome frame buffer
 *
 *   we do this by creating a temporary pixmap and making its
 * pointer to bits point to the buffer read in from the client.
 * this works because of the padding rules specified at startup
 *
 * Note that CopyArea must know how to copy a bitmap into the server-format
 * temporary pixmap.
 *
 * For speed, mfbPutImage should allocate the temporary pixmap on the stack.
 *
 *     even though an XYBitmap and an XYPixmap have the same
 * format (for this device), PutImage has different semantics for the
 * two.  XYPixmap just does the copy; XYBitmap takes gc.fgPixel for
 * a 1 bit, gc.bgPixel for a 0 bit, which we notice is exactly
 * like CopyPlane.
 *
 *   written by drewry, september 1986
 */



/*ARGSUSED*/
void
mfbPutImage(dst, pGC, depth, x, y, w, h, leftPad, format, pImage)
    DrawablePtr dst;
    GCPtr	pGC;
    int		depth, x, y, w, h;
    int leftPad;
    int format;
    char 	*pImage;
{
    PixmapPtr	pPixmap;

    if (!(pGC->planemask & 1))
	return;

    /* 0 may confuse CreatePixmap, and will sometimes be
       passed by the mi text code
    */
    if ((w == 0) || (h == 0))
	return;

    pPixmap = GetScratchPixmapHeader(dst->pScreen, w+leftPad, h, 1, 1,
				BitmapBytePad(w+leftPad), (pointer)pImage);
    if (!pPixmap)
	return;

    pGC->fExpose = FALSE;
    if (format != XYBitmap)
	(*pGC->ops->CopyArea)((DrawablePtr)pPixmap, dst, pGC, leftPad, 0,
			      w, h, x, y);
    else
	(*pGC->ops->CopyPlane)((DrawablePtr)pPixmap, dst, pGC, leftPad, 0,
			       w, h, x, y, 1);
    pGC->fExpose = TRUE;
    FreeScratchPixmapHeader(pPixmap);
}


/*
 * pdstLine points to space allocated by caller, which he can do since
 * he knows dimensions of the pixmap
 * we can call mfbDoBitblt because the dispatcher has promised not to send us
 * anything that would require going over the edge of the screen.
 *
 *	XYPixmap and ZPixmap are the same for mfb.
 *	For any planemask with bit 0 == 0, just fill the dst with 0.
 */
/*ARGSUSED*/
void
mfbGetImage( pDrawable, sx, sy, w, h, format, planeMask, pdstLine)
    DrawablePtr pDrawable;
    int		sx, sy, w, h;
    unsigned int format;
    unsigned long planeMask;
    char	*pdstLine;
{
    BoxRec box;
    DDXPointRec ptSrc;
    RegionRec rgnDst;

    if (planeMask & 0x1)
    {
	ScreenPtr pScreen = pDrawable->pScreen;
	PixmapPtr pPixmap;

	pPixmap = GetScratchPixmapHeader(pScreen, w, h, /*depth*/ 1, /*bpp*/ 1,
					 BitmapBytePad(w), (pointer)pdstLine);
	if (!pPixmap)
	    return;

        ptSrc.x = sx + pDrawable->x;
        ptSrc.y = sy + pDrawable->y;
        box.x1 = 0;
        box.y1 = 0;
        box.x2 = w;
        box.y2 = h;
        REGION_INIT(pScreen, &rgnDst, &box, 1);
        mfbDoBitblt(pDrawable, (DrawablePtr)pPixmap,
		    GXcopy, &rgnDst, &ptSrc);
        REGION_UNINIT(pScreen, &rgnDst);
	FreeScratchPixmapHeader(pPixmap);
    }
    else
    {
	bzero(pdstLine, BitmapBytePad(w) * h);
    }
}
