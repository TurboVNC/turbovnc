/* $XConsortium: pixmap.c /main/4 1996/08/12 22:04:49 dpw $ */
/* $XFree86: xc/programs/Xserver/dix/pixmap.c,v 3.1 1996/12/23 06:29:47 dawes Exp $ */
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

#include "X.h"
#include "scrnintstr.h"
#include "misc.h"
#include "os.h"
#include "windowstr.h"
#include "resource.h"
#include "dixstruct.h"
#include "gcstruct.h"
#include "servermd.h"
#include "site.h"


/*
 *  Scratch pixmap management and device independent pixmap allocation
 *  function.
 */


/* callable by ddx */
PixmapPtr
GetScratchPixmapHeader(pScreen, width, height, depth, bitsPerPixel, devKind,
		       pPixData)
    ScreenPtr   pScreen;
    int		width;
    int		height;
    int		depth;
    int		bitsPerPixel;
    int		devKind;
    pointer     pPixData;
{
    PixmapPtr pPixmap = pScreen->pScratchPixmap;

    if (pPixmap)
	pScreen->pScratchPixmap = NULL;
    else
	/* width and height of 0 means don't allocate any pixmap data */
	pPixmap = (*pScreen->CreatePixmap)(pScreen, 0, 0, depth);

    if (pPixmap)
	if ((*pScreen->ModifyPixmapHeader)(pPixmap, width, height, depth,
					   bitsPerPixel, devKind, pPixData))
	    return pPixmap;
    return NullPixmap;
}


/* callable by ddx */
void
FreeScratchPixmapHeader(pPixmap)
    PixmapPtr pPixmap;
{
    if (pPixmap)
    {
	ScreenPtr pScreen = pPixmap->drawable.pScreen;

	pPixmap->devPrivate.ptr = NULL; /* lest ddx chases bad ptr */
	if (pScreen->pScratchPixmap)
	    (*pScreen->DestroyPixmap)(pPixmap);
	else
	    pScreen->pScratchPixmap = pPixmap;
    }
}


Bool
CreateScratchPixmapsForScreen(scrnum)
    int scrnum;
{
    /* let it be created on first use */
    screenInfo.screens[scrnum]->pScratchPixmap = NULL;
    return TRUE;
}


void
FreeScratchPixmapsForScreen(scrnum)
    int scrnum;
{
    FreeScratchPixmapHeader(screenInfo.screens[scrnum]->pScratchPixmap);
}


/* callable by ddx */
PixmapPtr
AllocatePixmap(pScreen, pixDataSize)
    ScreenPtr pScreen;
    int pixDataSize;
{
    PixmapPtr pPixmap;
#ifdef PIXPRIV
    char *ptr;
    DevUnion *ppriv;
    unsigned *sizes;
    unsigned size;
    int i;

    pPixmap = (PixmapPtr)xalloc(pScreen->totalPixmapSize + pixDataSize);
    if (!pPixmap)
	return NullPixmap;
    ppriv = (DevUnion *)(pPixmap + 1);
    pPixmap->devPrivates = ppriv;
    sizes = pScreen->PixmapPrivateSizes;
    ptr = (char *)(ppriv + pScreen->PixmapPrivateLen);
    for (i = pScreen->PixmapPrivateLen; --i >= 0; ppriv++, sizes++)
    {
        if ((size = *sizes) != 0)
        {
	    ppriv->ptr = (pointer)ptr;
	    ptr += size;
        }
        else
	    ppriv->ptr = (pointer)NULL;
    }
#else
    pPixmap = (PixmapPtr)xalloc(sizeof(PixmapRec) + pixDataSize);
#endif
    return pPixmap;
}
