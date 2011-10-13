/* $XFree86: xc/programs/Xserver/mfb/mfbscrinit.c,v 3.8tsi Exp $ */
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
/* $Xorg: mfbscrinit.c,v 1.4 2001/02/09 02:05:19 xorgcvs Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>	/* for xColorItem */
#include <X11/Xmd.h>
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "resource.h"
#include "colormap.h"
#include "mfb.h"
#include "mistruct.h"
#include "dix.h"
#include "mi.h"
#include "mibstore.h"
#include "migc.h"
#include "servermd.h"

#ifdef PIXMAP_PER_WINDOW
int frameWindowPrivateIndex;
int frameGetWindowPrivateIndex(void) { return frameWindowPrivateIndex; }
#endif
int mfbWindowPrivateIndex;
int mfbGetWindowPrivateIndex(void) { return mfbWindowPrivateIndex; }
int mfbGCPrivateIndex;
int mfbGetGCPrivateIndex(void) { return mfbGCPrivateIndex; }
static unsigned long mfbGeneration = 0;

static VisualRec visual = {
/* vid  class       bpRGB cmpE nplan rMask gMask bMask oRed oGreen oBlue */
   0,   StaticGray, 1,    2,   1,    0,    0,    0,    0,   0,     0
};

static VisualID VID;

static DepthRec depth = {
/* depth	numVid		vids */
    1,		1,		&VID
};


BSFuncRec mfbBSFuncRec = {
    mfbSaveAreas,
    mfbRestoreAreas,
    (BackingStoreSetClipmaskRgnProcPtr) 0,
    (BackingStoreGetImagePixmapProcPtr) 0,
    (BackingStoreGetSpansPixmapProcPtr) 0,
};


Bool
mfbAllocatePrivates(pScreen, pWinIndex, pGCIndex)
    ScreenPtr pScreen;
    int *pWinIndex, *pGCIndex;
{
    if (mfbGeneration != serverGeneration)
    {
#ifdef PIXMAP_PER_WINDOW
	frameWindowPrivateIndex = AllocateWindowPrivateIndex();
#endif
	mfbWindowPrivateIndex = AllocateWindowPrivateIndex();
	mfbGCPrivateIndex = miAllocateGCPrivateIndex();
	visual.vid = FakeClientID(0);
	VID = visual.vid;
	mfbGeneration = serverGeneration;
    }
    if (pWinIndex)
	*pWinIndex = mfbWindowPrivateIndex;
    if (pGCIndex)
	*pGCIndex = mfbGCPrivateIndex;
    pScreen->GetWindowPixmap = mfbGetWindowPixmap;
    pScreen->SetWindowPixmap = mfbSetWindowPixmap;
    return (AllocateWindowPrivate(pScreen, mfbWindowPrivateIndex,
				  sizeof(mfbPrivWin)) &&
	    AllocateGCPrivate(pScreen, mfbGCPrivateIndex, sizeof(mfbPrivGC)));
}


/* dts * (inch/dot) * (25.4 mm / inch) = mm */
Bool
mfbScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
{
    if 	(!mfbAllocatePrivates(pScreen, (int *)NULL, (int *)NULL))
	return FALSE;
    pScreen->defColormap = (Colormap) FakeClientID(0);
    /* whitePixel, blackPixel */
    pScreen->QueryBestSize = mfbQueryBestSize;
    /* SaveScreen */
    pScreen->GetImage = mfbGetImage;
    pScreen->GetSpans = mfbGetSpans;
    pScreen->CreateWindow = mfbCreateWindow;
    pScreen->DestroyWindow = mfbDestroyWindow;
    pScreen->PositionWindow = mfbPositionWindow;
    pScreen->ChangeWindowAttributes = mfbChangeWindowAttributes;
    pScreen->RealizeWindow = mfbMapWindow;
    pScreen->UnrealizeWindow = mfbUnmapWindow;
    pScreen->PaintWindowBackground = mfbPaintWindow;
    pScreen->PaintWindowBorder = mfbPaintWindow;
    pScreen->CopyWindow = mfbCopyWindow;
    pScreen->CreatePixmap = mfbCreatePixmap;
    pScreen->DestroyPixmap = mfbDestroyPixmap;
    pScreen->RealizeFont = mfbRealizeFont;
    pScreen->UnrealizeFont = mfbUnrealizeFont;
    pScreen->CreateGC = mfbCreateGC;
    pScreen->CreateColormap = mfbCreateColormap;
    pScreen->DestroyColormap = mfbDestroyColormap;
    pScreen->InstallColormap = mfbInstallColormap;
    pScreen->UninstallColormap = mfbUninstallColormap;
    pScreen->ListInstalledColormaps = mfbListInstalledColormaps;
    pScreen->StoreColors = (StoreColorsProcPtr)NoopDDA;
    pScreen->ResolveColor = mfbResolveColor;
    pScreen->BitmapToRegion = mfbPixmapToRegion;
    if (!miScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width,
			1, 1, &depth, VID, 1, &visual))
	return FALSE;
    pScreen->BackingStoreFuncs = mfbBSFuncRec;
    return TRUE;
}

PixmapPtr
mfbGetWindowPixmap(pWin)
    WindowPtr pWin;
{
#ifdef PIXMAP_PER_WINDOW
    return (PixmapPtr)(pWin->devPrivates[frameWindowPrivateIndex].ptr);
#else
    ScreenPtr pScreen = pWin->drawable.pScreen;

    return (* pScreen->GetScreenPixmap)(pScreen);
#endif
}

void
mfbSetWindowPixmap(pWin, pPix)
    WindowPtr pWin;
    PixmapPtr pPix;
{
#ifdef PIXMAP_PER_WINDOW
    pWin->devPrivates[frameWindowPrivateIndex].ptr = (pointer)pPix;
#else
    (* pWin->drawable.pScreen->SetScreenPixmap)(pPix);
#endif
}

void mfbFillInScreen(ScreenPtr pScreen)
{
    pScreen->ChangeWindowAttributes = mfbChangeWindowAttributes;
    pScreen->RealizeWindow = mfbMapWindow;
    pScreen->UnrealizeWindow = mfbUnmapWindow;
    pScreen->DestroyPixmap = mfbDestroyPixmap;
    pScreen->RealizeFont = mfbRealizeFont;
    pScreen->UnrealizeFont = mfbUnrealizeFont;
    pScreen->BitmapToRegion = mfbPixmapToRegion;
}
