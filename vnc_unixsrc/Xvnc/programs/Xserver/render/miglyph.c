/*
 * $XFree86: xc/programs/Xserver/render/miglyph.c,v 1.6 2000/12/05 03:13:31 keithp Exp $
 *
 * Copyright © 2000 SuSE, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of SuSE not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  SuSE makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * SuSE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL SuSE
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, SuSE, Inc.
 */

#include "scrnintstr.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "mi.h"
#include "picturestr.h"
#include "mipict.h"

void
miGlyphExtents (int		nlist,
		GlyphListPtr	list,
		GlyphPtr	*glyphs,
		BoxPtr		extents)
{
    int		x1, x2, y1, y2;
    int		n;
    GlyphPtr	glyph;
    int		x, y;
    
    x = 0;
    y = 0;
    extents->x1 = MAXSHORT;
    extents->x2 = MINSHORT;
    extents->y1 = MAXSHORT;
    extents->y2 = MINSHORT;
    while (nlist--)
    {
	x += list->xOff;
	y += list->yOff;
	n = list->len;
	list++;
	while (n--)
	{
	    glyph = *glyphs++;
	    x1 = x - glyph->info.x;
	    if (x1 < MINSHORT)
		x1 = MINSHORT;
	    y1 = y - glyph->info.y;
	    if (y1 < MINSHORT)
		y1 = MINSHORT;
	    x2 = x1 + glyph->info.width;
	    if (x2 > MAXSHORT)
		x2 = MAXSHORT;
	    y2 = y1 + glyph->info.height;
	    if (y2 > MAXSHORT)
		y2 = MAXSHORT;
	    if (x1 < extents->x1)
		extents->x1 = x1;
	    if (x2 > extents->x2)
		extents->x2 = x2;
	    if (y1 < extents->y1)
		extents->y1 = y1;
	    if (y2 > extents->y2)
		extents->y2 = y2;
	    x += glyph->info.xOff;
	    y += glyph->info.yOff;
	}
    }
}

#define NeedsComponent(f) (PICT_FORMAT_A(f) != 0 && PICT_FORMAT_RGB(f) != 0)

void
miGlyphs (CARD8		op,
	  PicturePtr	pSrc,
	  PicturePtr	pDst,
	  PictFormatPtr	maskFormat,
	  INT16		xSrc,
	  INT16		ySrc,
	  int		nlist,
	  GlyphListPtr	list,
	  GlyphPtr	*glyphs)
{
    PixmapPtr	pPixmap = 0;
    PicturePtr	pPicture;
    PixmapPtr   pMaskPixmap = 0;
    PicturePtr  pMask;
    ScreenPtr   pScreen = pDst->pDrawable->pScreen;
    int		width = 0, height = 0;
    int		x, y;
    int		xDst = list->xOff, yDst = list->yOff;
    int		n;
    GlyphPtr	glyph;
    int		error;
    BoxRec	extents;
    CARD32	component_alpha;
    
    if (maskFormat)
    {
	GCPtr	    pGC;
	xRectangle  rect;
	
	miGlyphExtents (nlist, list, glyphs, &extents);
	
	if (extents.x2 <= extents.x1 || extents.y2 <= extents.y1)
	    return;
	width = extents.x2 - extents.x1;
	height = extents.y2 - extents.y1;
	pMaskPixmap = (*pScreen->CreatePixmap) (pScreen, width, height, maskFormat->depth);
	if (!pMaskPixmap)
	    return;
	component_alpha = NeedsComponent(maskFormat->format);
	pMask = CreatePicture (0, &pMaskPixmap->drawable,
			       maskFormat, CPComponentAlpha, &component_alpha,
			       serverClient, &error);
	if (!pMask)
	{
	    (*pScreen->DestroyPixmap) (pMaskPixmap);
	    return;
	}
	pGC = GetScratchGC (pMaskPixmap->drawable.depth, pScreen);
	ValidateGC (&pMaskPixmap->drawable, pGC);
	rect.x = 0;
	rect.y = 0;
	rect.width = width;
	rect.height = height;
	(*pGC->ops->PolyFillRect) (&pMaskPixmap->drawable, pGC, 1, &rect);
	FreeScratchGC (pGC);
	x = -extents.x1;
	y = -extents.y1;
    }
    else
    {
	pMask = pDst;
	x = 0;
	y = 0;
    }
    pPicture = 0;
    while (nlist--)
    {
	x += list->xOff;
	y += list->yOff;
	n = list->len;
	while (n--)
	{
	    glyph = *glyphs++;
	    if (!pPicture)
	    {
		pPixmap = GetScratchPixmapHeader (pScreen, glyph->info.width, glyph->info.height, 
						  list->format->depth,
						  list->format->depth, 
						  0, (pointer) (glyph + 1));
		if (!pPixmap)
		    return;
		component_alpha = NeedsComponent(list->format->format);
		pPicture = CreatePicture (0, &pPixmap->drawable, list->format,
					  CPComponentAlpha, &component_alpha, 
					  serverClient, &error);
		if (!pPicture)
		{
		    FreeScratchPixmapHeader (pPixmap);
		    return;
		}
	    }
	    (*pScreen->ModifyPixmapHeader) (pPixmap, 
					    glyph->info.width, glyph->info.height,
					    0, 0, -1, (pointer) (glyph + 1));
	    pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	    if (maskFormat)
	    {
		CompositePicture (PictOpAdd,
				  pPicture,
				  None,
				  pMask,
				  0, 0,
				  0, 0,
				  x - glyph->info.x,
				  y - glyph->info.y,
				  glyph->info.width,
				  glyph->info.height);
	    }
	    else
	    {
		CompositePicture (op,
				  pSrc,
				  pPicture,
				  pDst,
				  xSrc + (x - glyph->info.x) - xDst,
				  ySrc + (y - glyph->info.y) - yDst,
				  0, 0,
				  x - glyph->info.x,
				  y - glyph->info.y,
				  glyph->info.width,
				  glyph->info.height);
	    }
	    x += glyph->info.xOff;
	    y += glyph->info.yOff;
	}
	list++;
	if (pPicture)
	{
	    FreeScratchPixmapHeader (pPixmap);
	    FreePicture ((pointer) pPicture, 0);
	    pPicture = 0;
	    pPixmap = 0;
	}
    }
    if (maskFormat)
    {
	x = extents.x1;
	y = extents.y1;
	CompositePicture (op,
			  pSrc,
			  pMask,
			  pDst,
			  xSrc + x - xDst,
			  ySrc + y - yDst,
			  0, 0,
			  x, y,
			  width, height);
	FreePicture ((pointer) pMask, (XID) 0);
	(*pScreen->DestroyPixmap) (pMaskPixmap);
    }
}
