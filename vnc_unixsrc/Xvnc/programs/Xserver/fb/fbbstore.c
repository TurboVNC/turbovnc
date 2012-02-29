/*
 * Id: fbbstore.c,v 1.1 1999/11/02 03:54:45 keithp Exp $
 *
 * Copyright © 1998 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/fb/fbbstore.c,v 1.2 2000/02/23 20:29:42 dawes Exp $ */

#include "fb.h"

void
fbSaveAreas(PixmapPtr	pPixmap,
	    RegionPtr	prgnSave,
	    int		xorg,
	    int		yorg,
	    WindowPtr	pWin)
{
    fbCopyWindowProc (&pWin->drawable,
		      &pPixmap->drawable,
		      0,
		      REGION_RECTS(prgnSave),
		      REGION_NUM_RECTS(prgnSave),
		      xorg, yorg,
		      FALSE,
		      FALSE,
		      0,0);
}

void
fbRestoreAreas(PixmapPtr    pPixmap,
	       RegionPtr    prgnRestore,
	       int	    xorg,
	       int	    yorg,
	       WindowPtr    pWin)
{
    fbCopyWindowProc (&pPixmap->drawable,
		      &pWin->drawable,
		      0,
		      REGION_RECTS(prgnRestore),
		      REGION_NUM_RECTS(prgnRestore),
		      -xorg, -yorg,
		      FALSE,
		      FALSE,
		      0,0);
}
