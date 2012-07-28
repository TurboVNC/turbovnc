/* $XdotOrg: xc/programs/Xserver/fb/fbscreen.c,v 1.4 2004/12/04 00:42:50 kuhn Exp $
 * Id: fbscreen.c,v 1.1 1999/11/02 03:54:45 keithp Exp $
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
/* $XFree86: xc/programs/Xserver/fb/fbscreen.c,v 1.13 2001/05/29 04:54:09 keithp Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "fb.h"

Bool
fbCloseScreen (int index, ScreenPtr pScreen)
{
    int	    d;
    DepthPtr	depths = pScreen->allowedDepths;

    for (d = 0; d < pScreen->numDepths; d++)
	xfree (depths[d].vids);
    xfree (depths);
    xfree (pScreen->visuals);
    xfree (pScreen->devPrivate);
#ifdef FB_SCREEN_PRIVATE
    xfree (pScreen->devPrivates[fbScreenPrivateIndex].ptr);
#endif
    return TRUE;
}

Bool
fbRealizeFont(ScreenPtr pScreen, FontPtr pFont)
{
    return (TRUE);
}

Bool
fbUnrealizeFont(ScreenPtr pScreen, FontPtr pFont)
{
    return (TRUE);
}

void
fbQueryBestSize (int class, 
		 unsigned short *width, unsigned short *height,
		 ScreenPtr pScreen)
{
    unsigned short  w;
    
    switch (class) {
    case CursorShape:
	if (*width > pScreen->width)
	    *width = pScreen->width;
	if (*height > pScreen->height)
	    *height = pScreen->height;
	break;
    case TileShape:
    case StippleShape:
	w = *width;
	if ((w & (w - 1)) && w < FB_UNIT)
	{
	    for (w = 1; w < *width; w <<= 1)
		;
	    *width = w;
	}
    }
}

#ifndef FB_OLD_SCREEN
PixmapPtr
_fbGetWindowPixmap (WindowPtr pWindow)
{
    return fbGetWindowPixmap (pWindow);
}

void
_fbSetWindowPixmap (WindowPtr pWindow, PixmapPtr pPixmap)
{
#ifdef FB_NO_WINDOW_PIXMAPS
    FatalError ("Attempted to set window pixmap without fb support\n");
#else
    pWindow->devPrivates[fbWinPrivateIndex].ptr = (pointer) pPixmap;
#endif
}
#endif

Bool
fbSetupScreen(ScreenPtr	pScreen, 
	      pointer	pbits,		/* pointer to screen bitmap */
	      int	xsize, 		/* in pixels */
	      int	ysize,
	      int	dpix,		/* dots per inch */
	      int	dpiy,
	      int	width,		/* pixel width of frame buffer */
	      int	bpp)		/* bits per pixel for screen */
{
    if (!fbAllocatePrivates(pScreen, (int *) 0))
	return FALSE;
    pScreen->defColormap = FakeClientID(0);
    /* let CreateDefColormap do whatever it wants for pixels */ 
    pScreen->blackPixel = pScreen->whitePixel = (Pixel) 0;
    pScreen->QueryBestSize = fbQueryBestSize;
    /* SaveScreen */
    pScreen->GetImage = fbGetImage;
    pScreen->GetSpans = fbGetSpans;
    pScreen->CreateWindow = fbCreateWindow;
    pScreen->DestroyWindow = fbDestroyWindow;
    pScreen->PositionWindow = fbPositionWindow;
    pScreen->ChangeWindowAttributes = fbChangeWindowAttributes;
    pScreen->RealizeWindow = fbMapWindow;
    pScreen->UnrealizeWindow = fbUnmapWindow;
    pScreen->PaintWindowBackground = fbPaintWindow;
    pScreen->PaintWindowBorder = fbPaintWindow;
    pScreen->CopyWindow = fbCopyWindow;
    pScreen->CreatePixmap = fbCreatePixmap;
    pScreen->DestroyPixmap = fbDestroyPixmap;
    pScreen->RealizeFont = fbRealizeFont;
    pScreen->UnrealizeFont = fbUnrealizeFont;
    pScreen->CreateGC = fbCreateGC;
    pScreen->CreateColormap = fbInitializeColormap;
    pScreen->DestroyColormap = (void (*)(ColormapPtr))NoopDDA;
    pScreen->InstallColormap = fbInstallColormap;
    pScreen->UninstallColormap = fbUninstallColormap;
    pScreen->ListInstalledColormaps = fbListInstalledColormaps;
    pScreen->StoreColors = (void (*)(ColormapPtr, int, xColorItem *))NoopDDA;
    pScreen->ResolveColor = fbResolveColor;
    pScreen->BitmapToRegion = fbPixmapToRegion;
    
#ifndef FB_OLD_SCREEN
    pScreen->GetWindowPixmap = _fbGetWindowPixmap;
    pScreen->SetWindowPixmap = _fbSetWindowPixmap;

    pScreen->BackingStoreFuncs.SaveAreas = fbSaveAreas;
    pScreen->BackingStoreFuncs.RestoreAreas = fbRestoreAreas;
    pScreen->BackingStoreFuncs.SetClipmaskRgn = 0;
    pScreen->BackingStoreFuncs.GetImagePixmap = 0;
    pScreen->BackingStoreFuncs.GetSpansPixmap = 0;
#endif
    
    return TRUE;
}

Bool
fbFinishScreenInit(ScreenPtr	pScreen,
		   pointer	pbits,
		   int		xsize,
		   int		ysize,
		   int		dpix,
		   int		dpiy,
		   int		width,
		   int		bpp)
{
    VisualPtr	visuals;
    DepthPtr	depths;
    int		nvisuals;
    int		ndepths;
    int		rootdepth;
    VisualID	defaultVisual;
    int		imagebpp = bpp;

#ifdef FB_DEBUG
    int	stride;
    
    ysize -= 2;
    stride = (width * bpp) / 8;
    fbSetBits ((FbStip *) pbits, 
	       stride / sizeof (FbStip), FB_HEAD_BITS);
    pbits = (void *) ((char *) pbits + stride);
    fbSetBits ((FbStip *) ((char *) pbits + stride * ysize),
			   stride / sizeof (FbStip), FB_TAIL_BITS);
#endif
    /*
     * By default, a 24bpp screen will use 32bpp images, this avoids
     * problems with many applications which just can't handle packed
     * pixels.  If you want real 24bit images, include a 24bpp
     * format in the pixmap formats
     */
#ifdef FB_24_32BIT
    if (bpp == 24)
    {
	int	f;
	
	imagebpp = 32;
	/*
	 * Check to see if we're advertising a 24bpp image format,
	 * in which case windows will use it in preference to a 32 bit
	 * format.
	 */
	for (f = 0; f < screenInfo.numPixmapFormats; f++)
	{
	    if (screenInfo.formats[f].bitsPerPixel == 24)
	    {
		imagebpp = 24;
		break;
	    }
	}	    
    }
#endif
#ifdef FB_SCREEN_PRIVATE
    if (imagebpp == 32)
    {
	fbGetScreenPrivate(pScreen)->win32bpp = bpp;
	fbGetScreenPrivate(pScreen)->pix32bpp = bpp;
    }
    else
    {
	fbGetScreenPrivate(pScreen)->win32bpp = 32;
	fbGetScreenPrivate(pScreen)->pix32bpp = 32;
    }
#endif
    rootdepth = 0;
    if (!fbInitVisuals (&visuals, &depths, &nvisuals, &ndepths, &rootdepth,
			&defaultVisual,((unsigned long)1<<(imagebpp-1)), 8))
	return FALSE;
    if (! miScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width,
			rootdepth, ndepths, depths,
			defaultVisual, nvisuals, visuals
#ifdef FB_OLD_MISCREENINIT
		       , (miBSFuncPtr) 0
#endif
		       ))
	return FALSE;
    /* overwrite miCloseScreen with our own */
    pScreen->CloseScreen = fbCloseScreen;
#ifdef FB_24_32BIT
    if (bpp == 24 && imagebpp == 32)
    {
	pScreen->ModifyPixmapHeader = fb24_32ModifyPixmapHeader;
	pScreen->CreateScreenResources = fb24_32CreateScreenResources;
    }
#endif
#if 0
    /* leave backing store initialization to the enclosing code so
     * it can choose the correct order of wrappers
     */
    /* init backing store here so we can overwrite CloseScreen without stepping
     * on the backing store wrapped version */
    fbInitializeBackingStore (pScreen);
#endif
    return TRUE;
}

/* dts * (inch/dot) * (25.4 mm / inch) = mm */
Bool
fbScreenInit(ScreenPtr	pScreen,
	     pointer	pbits,
	     int	xsize,
	     int	ysize,
	     int	dpix,
	     int	dpiy,
	     int	width,
	     int	bpp)
{
    if (!fbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width, bpp))
	return FALSE;
    if (!fbFinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, 
			    width, bpp))
	return FALSE;
    return TRUE;
}


#ifdef FB_OLD_SCREEN
const miBSFuncRec fbBSFuncRec = {
    fbSaveAreas,
    fbRestoreAreas,
    (void (*)(GCPtr, RegionPtr)) 0,
    (PixmapPtr (*)(void)) 0,
    (PixmapPtr (*)(void)) 0,
};
#endif

void
fbInitializeBackingStore (ScreenPtr pScreen)
{
#ifdef FB_OLD_SCREEN
    miInitializeBackingStore (pScreen, (miBSFuncRec *) &fbBSFuncRec);
#else
    miInitializeBackingStore (pScreen);
#endif
}
