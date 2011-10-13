/* $XFree86: xc/programs/Xserver/cfb/cfbbstore.c,v 1.4 1999/01/31 12:21:41 dawes Exp $ */
/*-
 * cfbbstore.c --
 *	Functions required by the backing-store implementation in MI.
 *
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 *
 */
/* $Xorg: cfbbstore.c,v 1.3 2000/08/17 19:48:13 cpqbld Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include    "cfb.h"
#include    <X11/X.h>
#include    "mibstore.h"
#include    "regionstr.h"
#include    "scrnintstr.h"
#include    "pixmapstr.h"
#include    "windowstr.h"

/*-
 *-----------------------------------------------------------------------
 * cfbSaveAreas --
 *	Function called by miSaveAreas to actually fetch the areas to be
 *	saved into the backing pixmap. This is very simple to do, since
 *	cfbDoBitblt is designed for this very thing. The region to save is
 *	already destination-relative and we're given the offset to the
 *	window origin, so we have only to create an array of points of the
 *	u.l. corners of the boxes in the region translated to the screen
 *	coordinate system and fetch the screen pixmap out of its devPrivate
 *	field....
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Data are copied from the screen into the pixmap.
 *
 *-----------------------------------------------------------------------
 */
void
cfbSaveAreas(pPixmap, prgnSave, xorg, yorg, pWin)
    PixmapPtr	  	pPixmap;  	/* Backing pixmap */
    RegionPtr	  	prgnSave; 	/* Region to save (pixmap-relative) */
    int	    	  	xorg;	    	/* X origin of region */
    int	    	  	yorg;	    	/* Y origin of region */
    WindowPtr		pWin;
{
    register DDXPointPtr pPt;
    DDXPointPtr		pPtsInit;
    register BoxPtr	pBox;
    register int	i;
    ScreenPtr		pScreen = pPixmap->drawable.pScreen;
    PixmapPtr		pScrPix;
    
    i = REGION_NUM_RECTS(prgnSave);
    pPtsInit = (DDXPointPtr)ALLOCATE_LOCAL(i * sizeof(DDXPointRec));
    if (!pPtsInit)
	return;
    
    pBox = REGION_RECTS(prgnSave);
    pPt = pPtsInit;
    while (--i >= 0) {
	pPt->x = pBox->x1 + xorg;
	pPt->y = pBox->y1 + yorg;
	pPt++;
	pBox++;
    }

    pScrPix = (*pScreen->GetWindowPixmap)(pWin);


    cfbDoBitbltCopy((DrawablePtr) pScrPix, (DrawablePtr)pPixmap,
		    GXcopy, prgnSave, pPtsInit, ~0L);

    DEALLOCATE_LOCAL (pPtsInit);
}

/*-
 *-----------------------------------------------------------------------
 * cfbRestoreAreas --
 *	Function called by miRestoreAreas to actually fetch the areas to be
 *	restored from the backing pixmap. This is very simple to do, since
 *	cfbDoBitblt is designed for this very thing. The region to restore is
 *	already destination-relative and we're given the offset to the
 *	window origin, so we have only to create an array of points of the
 *	u.l. corners of the boxes in the region translated to the pixmap
 *	coordinate system and fetch the screen pixmap out of its devPrivate
 *	field....
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Data are copied from the pixmap into the screen.
 *
 *-----------------------------------------------------------------------
 */
void
cfbRestoreAreas(pPixmap, prgnRestore, xorg, yorg, pWin)
    PixmapPtr	  	pPixmap;  	/* Backing pixmap */
    RegionPtr	  	prgnRestore; 	/* Region to restore (screen-relative)*/
    int	    	  	xorg;	    	/* X origin of window */
    int	    	  	yorg;	    	/* Y origin of window */
    WindowPtr		pWin;
{
    register DDXPointPtr pPt;
    DDXPointPtr		pPtsInit;
    register BoxPtr	pBox;
    register int	i;
    ScreenPtr		pScreen = pPixmap->drawable.pScreen;
    PixmapPtr		pScrPix;
    
    i = REGION_NUM_RECTS(prgnRestore);
    pPtsInit = (DDXPointPtr)ALLOCATE_LOCAL(i*sizeof(DDXPointRec));
    if (!pPtsInit)
	return;
    
    pBox = REGION_RECTS(prgnRestore);
    pPt = pPtsInit;
    while (--i >= 0) {
	pPt->x = pBox->x1 - xorg;
	pPt->y = pBox->y1 - yorg;
	pPt++;
	pBox++;
    }

    pScrPix = (*pScreen->GetWindowPixmap)(pWin);

    cfbDoBitbltCopy((DrawablePtr)pPixmap, (DrawablePtr) pScrPix,
		    GXcopy, prgnRestore, pPtsInit, ~0L);

    DEALLOCATE_LOCAL (pPtsInit);
}
