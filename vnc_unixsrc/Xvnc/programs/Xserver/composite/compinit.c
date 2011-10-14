/*
 * $Id: compinit.c,v 1.9 2005/07/03 07:37:34 daniels Exp $
 *
 * Copyright © 2003 Keith Packard
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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "compint.h"

int	CompScreenPrivateIndex;
int	CompWindowPrivateIndex;
int	CompSubwindowsPrivateIndex;
int	CompGeneration;

static Bool
compCloseScreen (int index, ScreenPtr pScreen)
{
    CompScreenPtr   cs = GetCompScreen (pScreen);
    Bool	    ret;

    pScreen->CloseScreen = cs->CloseScreen;
    pScreen->BlockHandler = cs->BlockHandler;
    pScreen->InstallColormap = cs->InstallColormap;
    pScreen->ReparentWindow = cs->ReparentWindow;
    pScreen->MoveWindow = cs->MoveWindow;
    pScreen->ResizeWindow = cs->ResizeWindow;
    pScreen->ChangeBorderWidth = cs->ChangeBorderWidth;
    
    pScreen->ClipNotify = cs->ClipNotify;
    pScreen->PaintWindowBackground = cs->PaintWindowBackground;
    pScreen->UnrealizeWindow = cs->UnrealizeWindow;
    pScreen->RealizeWindow = cs->RealizeWindow;
    pScreen->DestroyWindow = cs->DestroyWindow;
    pScreen->CreateWindow = cs->CreateWindow;
    pScreen->CopyWindow = cs->CopyWindow;
    pScreen->PositionWindow = cs->PositionWindow;
    xfree (cs);
    pScreen->devPrivates[CompScreenPrivateIndex].ptr = 0;
    ret = (*pScreen->CloseScreen) (index, pScreen);
    return ret;
}

static void
compInstallColormap (ColormapPtr pColormap)
{
    VisualPtr	    pVisual = pColormap->pVisual;
    ScreenPtr	    pScreen = pColormap->pScreen;
    CompScreenPtr   cs = GetCompScreen (pScreen);
    int		    a;

    for (a = 0; a < NUM_COMP_ALTERNATE_VISUALS; a++)
	if (pVisual->vid == cs->alternateVisuals[a])
	    return;
    pScreen->InstallColormap = cs->InstallColormap;
    (*pScreen->InstallColormap) (pColormap);
    cs->InstallColormap = pScreen->InstallColormap;
    pScreen->InstallColormap = compInstallColormap;
}

static void
compScreenUpdate (ScreenPtr pScreen)
{
    CompScreenPtr   cs = GetCompScreen (pScreen);

    compCheckTree (pScreen);
    if (cs->damaged)
    {
	compWindowUpdate (WindowTable[pScreen->myNum]);
	cs->damaged = FALSE;
    }
}

static void
compBlockHandler (int	    i,
		  pointer   blockData,
		  pointer   pTimeout,
		  pointer   pReadmask)
{
    ScreenPtr	    pScreen = screenInfo.screens[i];
    CompScreenPtr   cs = GetCompScreen (pScreen);

    pScreen->BlockHandler = cs->BlockHandler;
    compScreenUpdate (pScreen);
    (*pScreen->BlockHandler) (i, blockData, pTimeout, pReadmask);
    cs->BlockHandler = pScreen->BlockHandler;
    pScreen->BlockHandler = compBlockHandler;
}

/*
 * Add alternate visuals -- always expose an ARGB32 and RGB24 visual
 */

static DepthPtr
compFindVisuallessDepth (ScreenPtr pScreen, int d)
{
    int		i;

    for (i = 0; i < pScreen->numDepths; i++)
    {
	DepthPtr    depth = &pScreen->allowedDepths[i];
	if (depth->depth == d)
	{
	    /*
	     * Make sure it doesn't have visuals already
	     */
	    if (depth->numVids)
		return 0;
	    /*
	     * looks fine
	     */
	    return depth;
	}
    }
    /*
     * If there isn't one, then it's gonna be hard to have 
     * an associated visual
     */
    return 0;
}

typedef struct _alternateVisual {
    int		depth;
    CARD32	format;
} CompAlternateVisual;

static CompAlternateVisual  altVisuals[NUM_COMP_ALTERNATE_VISUALS] = {
#if COMP_INCLUDE_RGB24_VISUAL
    {	24,	PICT_r8g8b8 },
#endif
    {	32,	PICT_a8r8g8b8 },
};

static Bool
compAddAlternateVisuals (ScreenPtr pScreen, CompScreenPtr cs)
{
    VisualPtr	    visuals;
    DepthPtr	    depths[NUM_COMP_ALTERNATE_VISUALS];
    PictFormatPtr   pPictFormats[NUM_COMP_ALTERNATE_VISUALS];
    int		    i;
    int		    numVisuals;
    VisualID	    *vids[NUM_COMP_ALTERNATE_VISUALS];
    XID		    *installedCmaps;
    ColormapPtr	    installedCmap;
    int		    numInstalledCmaps;
    int		    numAlternate = 0;
    int		    alt;
    
    memset (cs->alternateVisuals, '\0', sizeof (cs->alternateVisuals));

    for (alt = 0; alt < NUM_COMP_ALTERNATE_VISUALS; alt++)
    {
	DepthPtr	depth;
	PictFormatPtr   pPictFormat;
	
	depth = compFindVisuallessDepth (pScreen, altVisuals[alt].depth);
	if (!depth)
	    continue;
	/*
	 * Find the right picture format
	 */
	pPictFormat = PictureMatchFormat (pScreen, altVisuals[alt].depth,
					  altVisuals[alt].format);
	if (!pPictFormat)
	    continue;

	/*
	 * Allocate vid list for this depth
	 */
	vids[numAlternate] = xalloc (sizeof (VisualID));
	if (!vids[numAlternate])
	    continue;
	depths[numAlternate] = depth;
	pPictFormats[numAlternate] = pPictFormat;
	numAlternate++;
    }
    
    if (!numAlternate)
	return TRUE;

    /*
     * Find the installed colormaps
     */
    installedCmaps = xalloc (pScreen->maxInstalledCmaps * sizeof (XID));
    if (!installedCmaps)
    {
	for (alt = 0; alt < numAlternate; alt++)
	    xfree (vids[alt]);
	return FALSE;
    }
    numInstalledCmaps = (*pScreen->ListInstalledColormaps) (pScreen, 
							    installedCmaps);
    
    /*
     * realloc the visual array to fit the new one in place
     */
    numVisuals = pScreen->numVisuals;
    visuals = xrealloc (pScreen->visuals,
			(numVisuals + numAlternate) * sizeof (VisualRec));
    if (!visuals)
    {
	for (alt = 0; alt < numAlternate; alt++)
	    xfree (vids[alt]);
	xfree (installedCmaps);
	return FALSE;
    }
    
    /*
     * Fix up any existing installed colormaps -- we'll assume that
     * the only ones created so far have been installed.  If this
     * isn't true, we'll have to walk the resource database looking
     * for all colormaps.
     */
    for (i = 0; i < numInstalledCmaps; i++)
    {
	int j;
	
	installedCmap = LookupIDByType (installedCmaps[i], RT_COLORMAP);
	if (!installedCmap)
	    continue;
	j = installedCmap->pVisual - pScreen->visuals;
	installedCmap->pVisual = &visuals[j];
    }

    xfree (installedCmaps);

    pScreen->visuals = visuals;
    pScreen->numVisuals = numVisuals + numAlternate;

    for (alt = 0; alt < numAlternate; alt++)
    {
	DepthPtr	depth = depths[alt];
	PictFormatPtr	pPictFormat = pPictFormats[alt];
	VisualPtr	visual = &visuals[numVisuals + alt];
	unsigned long	alphaMask;

	/*
	 * Initialize the visual
	 */
	visual->class = TrueColor;
	visual->bitsPerRGBValue = 8;

	visual->vid = FakeClientID (0);
	visual->redMask   = (((unsigned long) pPictFormat->direct.redMask) << 
			     pPictFormat->direct.red);
	visual->greenMask = (((unsigned long) pPictFormat->direct.greenMask) << 
			     pPictFormat->direct.green);
	visual->blueMask  = (((unsigned long) pPictFormat->direct.blueMask) << 
			     pPictFormat->direct.blue);
	alphaMask =  (((unsigned long) pPictFormat->direct.alphaMask) << 
		      pPictFormat->direct.alpha);
	visual->offsetRed   = pPictFormat->direct.red;
	visual->offsetGreen = pPictFormat->direct.green;
	visual->offsetBlue  = pPictFormat->direct.blue;
	/*
	 * Include A bits in this (unlike GLX which includes only RGB)
	 * This lets DIX compute suitable masks for colormap allocations
	 */
	visual->nplanes = Ones (visual->redMask |
				visual->greenMask |
				visual->blueMask |
				alphaMask);
	/*
	 * find widest component
	 */
	visual->ColormapEntries = (1 << max (Ones (visual->redMask),
					     max (Ones (visual->greenMask),
						  Ones (visual->blueMask))));

	/*
	 * remember the visual ID to detect auto-update windows
	 */
	cs->alternateVisuals[alt] = visual->vid;
	
	/*
	 * Fix up the depth
	 */
	vids[alt][0] = visual->vid;
	depth->numVids = 1;
	depth->vids = vids[alt];
    }
    return TRUE;
}

Bool
compScreenInit (ScreenPtr pScreen)
{
    CompScreenPtr   cs;

    if (CompGeneration != serverGeneration)
    {
	CompScreenPrivateIndex = AllocateScreenPrivateIndex ();
	if (CompScreenPrivateIndex == -1)
	    return FALSE;
	CompWindowPrivateIndex = AllocateWindowPrivateIndex ();
	if (CompWindowPrivateIndex == -1)
	    return FALSE;
	CompSubwindowsPrivateIndex = AllocateWindowPrivateIndex ();
	if (CompSubwindowsPrivateIndex == -1)
	    return FALSE;
	CompGeneration = serverGeneration;
    }
    if (!AllocateWindowPrivate (pScreen, CompWindowPrivateIndex, 0))
	return FALSE;

    if (!AllocateWindowPrivate (pScreen, CompSubwindowsPrivateIndex, 0))
	return FALSE;

    if (GetCompScreen (pScreen))
	return TRUE;
    cs = (CompScreenPtr) xalloc (sizeof (CompScreenRec));
    if (!cs)
	return FALSE;

    cs->damaged = FALSE;

    if (!compAddAlternateVisuals (pScreen, cs))
    {
	xfree (cs);
	return FALSE;
    }

    cs->PositionWindow = pScreen->PositionWindow;
    pScreen->PositionWindow = compPositionWindow;

    cs->CopyWindow = pScreen->CopyWindow;
    pScreen->CopyWindow = compCopyWindow;

    cs->CreateWindow = pScreen->CreateWindow;
    pScreen->CreateWindow = compCreateWindow;

    cs->DestroyWindow = pScreen->DestroyWindow;
    pScreen->DestroyWindow = compDestroyWindow;

    cs->RealizeWindow = pScreen->RealizeWindow;
    pScreen->RealizeWindow = compRealizeWindow;

    cs->UnrealizeWindow = pScreen->UnrealizeWindow;
    pScreen->UnrealizeWindow = compUnrealizeWindow;

    cs->PaintWindowBackground = pScreen->PaintWindowBackground;
    pScreen->PaintWindowBackground = compPaintWindowBackground;

    cs->ClipNotify = pScreen->ClipNotify;
    pScreen->ClipNotify = compClipNotify;

    cs->MoveWindow = pScreen->MoveWindow;
    pScreen->MoveWindow = compMoveWindow;

    cs->ResizeWindow = pScreen->ResizeWindow;
    pScreen->ResizeWindow = compResizeWindow;

    cs->ChangeBorderWidth = pScreen->ChangeBorderWidth;
    pScreen->ChangeBorderWidth = compChangeBorderWidth;

    cs->ReparentWindow = pScreen->ReparentWindow;
    pScreen->ReparentWindow = compReparentWindow;

    cs->InstallColormap = pScreen->InstallColormap;
    pScreen->InstallColormap = compInstallColormap;

    cs->BlockHandler = pScreen->BlockHandler;
    pScreen->BlockHandler = compBlockHandler;

    cs->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = compCloseScreen;

    pScreen->devPrivates[CompScreenPrivateIndex].ptr = (pointer) cs;
    return TRUE;
}
