/*
 * sprite.c
 *
 * software sprite routines - based on misprite
 */

/* Copyright (C) 2012-2014, 2017-2018 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

/*

Copyright 1989, 1998  The Open Group

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
*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include   <X11/X.h>
#include   <X11/Xproto.h>
#include   "misc.h"
#include   "pixmapstr.h"
#include   "input.h"
#include   "mi.h"
#include   "cursorstr.h"
#include   <X11/fonts/font.h>
#include   "scrnintstr.h"
#include   "colormapst.h"
#include   "windowstr.h"
#include   "gcstruct.h"
#include   "mipointer.h"
#include   "misprite.h"
#include   "dixfontstr.h"
#include   <X11/fonts/fontstruct.h>
#include   "inputstr.h"
#include   "damage.h"
#include   "rfb.h"
#include   "sprite.h"

typedef struct {
    CursorPtr pCursor;
    int x;                      /* cursor hotspot */
    int y;
    BoxRec saved;               /* saved area from the screen */
    Bool isUp;                  /* cursor in frame buffer */
    Bool shouldBeUp;            /* cursor should be displayed */
    WindowPtr pCacheWin;        /* window the cursor last seen in */
    Bool isInCacheWin;
    Bool checkPixels;           /* check colormap collision */
    ScreenPtr pScreen;
} rfbCursorInfoRec, *rfbCursorInfoPtr;

/*
 * per screen information
 */

typedef struct {
    /* screen procedures */
    CloseScreenProcPtr CloseScreen;
    GetImageProcPtr GetImage;
    GetSpansProcPtr GetSpans;
    SourceValidateProcPtr SourceValidate;

    /* window procedures */
    CopyWindowProcPtr CopyWindow;

    /* colormap procedures */
    InstallColormapProcPtr InstallColormap;
    StoreColorsProcPtr StoreColors;

    /* os layer procedures */
    ScreenBlockHandlerProcPtr BlockHandler;

    /* VNC-specific */
    DisplayCursorProcPtr DisplayCursor;

    xColorItem colors[2];
    ColormapPtr pInstalledMap;
    ColormapPtr pColormap;
    VisualPtr pVisual;
    DamagePtr pDamage;          /* damage tracking structure */
    Bool damageRegistered;
    int numberOfCursors;
} rfbSpriteScreenRec, *rfbSpriteScreenPtr;

#define SOURCE_COLOR	0
#define MASK_COLOR	1

/*
 * Overlap BoxPtr and Box elements
 */
#define BOX_OVERLAP(pCbox,X1,Y1,X2,Y2) \
 	(((pCbox)->x1 <= (X2)) && ((X1) <= (pCbox)->x2) && \
	 ((pCbox)->y1 <= (Y2)) && ((Y1) <= (pCbox)->y2))

/*
 * Overlap BoxPtr, origins, and rectangle
 */
#define ORG_OVERLAP(pCbox,xorg,yorg,x,y,w,h) \
    BOX_OVERLAP((pCbox),(x)+(xorg),(y)+(yorg),(x)+(xorg)+(w),(y)+(yorg)+(h))

/*
 * Overlap BoxPtr, origins and RectPtr
 */
#define ORGRECT_OVERLAP(pCbox,xorg,yorg,pRect) \
    ORG_OVERLAP((pCbox),(xorg),(yorg),(pRect)->x,(pRect)->y, \
		(int)((pRect)->width), (int)((pRect)->height))
/*
 * Overlap BoxPtr and horizontal span
 */
#define SPN_OVERLAP(pCbox,y,x,w) BOX_OVERLAP((pCbox),(x),(y),(x)+(w),(y))

#define LINE_SORT(x1,y1,x2,y2) \
{ int _t; \
  if (x1 > x2) { _t = x1; x1 = x2; x2 = _t; } \
  if (y1 > y2) { _t = y1; y1 = y2; y2 = _t; } }

#define LINE_OVERLAP(pCbox,x1,y1,x2,y2,lw2) \
    BOX_OVERLAP((pCbox), (x1)-(lw2), (y1)-(lw2), (x2)+(lw2), (y2)+(lw2))

#define SPRITE_DEBUG_ENABLE 0
#if SPRITE_DEBUG_ENABLE
#define SPRITE_DEBUG(x)	ErrorF x
#else
#define SPRITE_DEBUG(x)
#endif

#define RFBSPRITE(dev) \
    (IsFloating(dev) ? \
       (rfbCursorInfoPtr)dixLookupPrivate(&dev->devPrivates, rfbSpriteDevPrivatesKey) : \
       (rfbCursorInfoPtr)dixLookupPrivate(&(GetMaster(dev, MASTER_POINTER))->devPrivates, rfbSpriteDevPrivatesKey))

static void
rfbSpriteDisableDamage(ScreenPtr pScreen, rfbSpriteScreenPtr pScreenPriv)
{
    if (pScreenPriv->damageRegistered) {
        DamageUnregister(pScreenPriv->pDamage);
        pScreenPriv->damageRegistered = 0;
    }
}

static void
rfbSpriteEnableDamage(ScreenPtr pScreen, rfbSpriteScreenPtr pScreenPriv)
{
    if (!pScreenPriv->damageRegistered) {
        pScreenPriv->damageRegistered = 1;
        DamageRegister(&(pScreen->GetScreenPixmap(pScreen)->drawable),
                       pScreenPriv->pDamage);
    }
}

static void
rfbSpriteIsUp(rfbCursorInfoPtr pDevCursor)
{
    pDevCursor->isUp = TRUE;
    rfbFB.cursorIsDrawn = TRUE;
}

static void
rfbSpriteIsDown(rfbCursorInfoPtr pDevCursor)
{
    pDevCursor->isUp = FALSE;
    rfbFB.cursorIsDrawn = FALSE;
}

/*
 * screen wrappers
 */

static DevPrivateKeyRec rfbSpriteScreenKeyRec;

#define rfbSpriteScreenKey (&rfbSpriteScreenKeyRec)
#define GetSpriteScreen(pScreen) \
	(dixLookupPrivate(&(pScreen)->devPrivates, rfbSpriteScreenKey))
static DevPrivateKeyRec rfbSpriteDevPrivatesKeyRec;

#define rfbSpriteDevPrivatesKey (&rfbSpriteDevPrivatesKeyRec)

static Bool rfbSpriteCloseScreen(ScreenPtr pScreen);
static void rfbSpriteGetImage(DrawablePtr pDrawable, int sx, int sy,
                              int w, int h, unsigned int format,
                              unsigned long planemask, char *pdstLine);
static void rfbSpriteGetSpans(DrawablePtr pDrawable, int wMax,
                              DDXPointPtr ppt, int *pwidth, int nspans,
                              char *pdstStart);
static void rfbSpriteSourceValidate(DrawablePtr pDrawable, int x, int y,
                                    int width, int height,
                                    unsigned int subWindowMode);
static void rfbSpriteCopyWindow(WindowPtr pWindow,
                                DDXPointRec ptOldOrg, RegionPtr prgnSrc);
static void rfbSpriteBlockHandler(ScreenPtr pScreen, void *timeout);
static void rfbSpriteInstallColormap(ColormapPtr pMap);
static void rfbSpriteStoreColors(ColormapPtr pMap, int ndef, xColorItem * pdef);

static void rfbSpriteComputeSaved(DeviceIntPtr pDev, ScreenPtr pScreen);

static Bool rfbSpriteDeviceCursorInitialize(DeviceIntPtr pDev,
                                           ScreenPtr pScreen);
static void rfbSpriteDeviceCursorCleanup(DeviceIntPtr pDev, ScreenPtr pScreen);

#define SCREEN_PROLOGUE(pPriv, pScreen, field) ((pScreen)->field = \
   (pPriv)->field)
#define SCREEN_EPILOGUE(pPriv, pScreen, field)\
    ((pPriv)->field = (pScreen)->field, (pScreen)->field = rfbSprite##field)

/*
 * pointer-sprite method table
 */

static Bool rfbSpriteRealizeCursor(DeviceIntPtr pDev, ScreenPtr pScreen,
                                   CursorPtr pCursor);
static Bool rfbSpriteUnrealizeCursor(DeviceIntPtr pDev, ScreenPtr pScreen,
                                     CursorPtr pCursor);
static void rfbSpriteSetCursor(DeviceIntPtr pDev, ScreenPtr pScreen,
                               CursorPtr pCursor, int x, int y);
static void rfbSpriteMoveCursor(DeviceIntPtr pDev, ScreenPtr pScreen,
                                int x, int y);

miPointerSpriteFuncRec rfbSpritePointerFuncs = {
    rfbSpriteRealizeCursor,
    rfbSpriteUnrealizeCursor,
    rfbSpriteSetCursor,
    rfbSpriteMoveCursor,
    rfbSpriteDeviceCursorInitialize,
    rfbSpriteDeviceCursorCleanup,
};

/*
 * other misc functions
 */

static void rfbSpriteRemoveCursor(DeviceIntPtr pDev, ScreenPtr pScreen);
static void rfbSpriteSaveUnderCursor(DeviceIntPtr pDev, ScreenPtr pScreen);
static void rfbSpriteRestoreCursor(DeviceIntPtr pDev, ScreenPtr pScreen);
static Bool rfbDisplayCursor(DeviceIntPtr pDev, ScreenPtr pScreen,
                             CursorPtr pCursor);

static void
rfbSpriteRegisterBlockHandler(ScreenPtr pScreen, rfbSpriteScreenPtr pScreenPriv)
{
    if (!pScreenPriv->BlockHandler) {
        pScreenPriv->BlockHandler = pScreen->BlockHandler;
        pScreen->BlockHandler = rfbSpriteBlockHandler;
    }
}

static void
rfbSpriteReportDamage(DamagePtr pDamage, RegionPtr pRegion, void *closure)
{
    ScreenPtr pScreen = closure;
    rfbCursorInfoPtr pCursorInfo;
    DeviceIntPtr pDev;

    for (pDev = inputInfo.devices; pDev; pDev = pDev->next) {
        if (DevHasCursor(pDev)) {
            pCursorInfo = RFBSPRITE(pDev);

            if (pCursorInfo->isUp &&
                pCursorInfo->pScreen == pScreen &&
                RegionContainsRect(pRegion, &pCursorInfo->saved) != rgnOUT) {
                SPRITE_DEBUG(("Damage remove\n"));
                rfbSpriteRemoveCursor(pDev, pScreen);
            }
        }
    }
}

/*
 * rfbSpriteInitialize -- called from device-dependent screen
 * initialization proc after all of the function pointers have
 * been stored in the screen structure.
 */

Bool
rfbSpriteInitialize(ScreenPtr pScreen, miPointerScreenFuncPtr screenFuncs)
{
    rfbSpriteScreenPtr pScreenPriv;
    VisualPtr pVisual;

    if (!DamageSetup(pScreen))
        return FALSE;

    if (!dixRegisterPrivateKey(&rfbSpriteScreenKeyRec, PRIVATE_SCREEN, 0))
        return FALSE;

    if (!dixRegisterPrivateKey
        (&rfbSpriteDevPrivatesKeyRec, PRIVATE_DEVICE, sizeof(rfbCursorInfoRec)))
        return FALSE;

    pScreenPriv = rfbAlloc(sizeof(rfbSpriteScreenRec));

    pScreenPriv->pDamage = DamageCreate(rfbSpriteReportDamage,
                                        NULL,
                                        DamageReportRawRegion,
                                        TRUE, pScreen, pScreen);

    if (!miPointerInitialize(pScreen, &rfbSpritePointerFuncs, screenFuncs, TRUE)) {
        free(pScreenPriv);
        return FALSE;
    }
    for (pVisual = pScreen->visuals;
         pVisual->vid != pScreen->rootVisual; pVisual++);
    pScreenPriv->pVisual = pVisual;
    pScreenPriv->CloseScreen = pScreen->CloseScreen;
    pScreenPriv->GetImage = pScreen->GetImage;
    pScreenPriv->GetSpans = pScreen->GetSpans;
    pScreenPriv->SourceValidate = pScreen->SourceValidate;

    pScreenPriv->CopyWindow = pScreen->CopyWindow;

    pScreenPriv->InstallColormap = pScreen->InstallColormap;
    pScreenPriv->StoreColors = pScreen->StoreColors;

    pScreenPriv->BlockHandler = NULL;

    pScreenPriv->DisplayCursor = pScreen->DisplayCursor;

    pScreenPriv->pInstalledMap = NULL;
    pScreenPriv->pColormap = NULL;
    pScreenPriv->colors[SOURCE_COLOR].red = 0;
    pScreenPriv->colors[SOURCE_COLOR].green = 0;
    pScreenPriv->colors[SOURCE_COLOR].blue = 0;
    pScreenPriv->colors[MASK_COLOR].red = 0;
    pScreenPriv->colors[MASK_COLOR].green = 0;
    pScreenPriv->colors[MASK_COLOR].blue = 0;
    pScreenPriv->damageRegistered = 0;
    pScreenPriv->numberOfCursors = 0;

    dixSetPrivate(&pScreen->devPrivates, rfbSpriteScreenKey, pScreenPriv);

    pScreen->CloseScreen = rfbSpriteCloseScreen;
    pScreen->GetImage = rfbSpriteGetImage;
    pScreen->GetSpans = rfbSpriteGetSpans;
    pScreen->SourceValidate = rfbSpriteSourceValidate;

    pScreen->CopyWindow = rfbSpriteCopyWindow;
    pScreen->InstallColormap = rfbSpriteInstallColormap;
    pScreen->StoreColors = rfbSpriteStoreColors;

    pScreen->DisplayCursor = rfbDisplayCursor;

    return TRUE;
}

/*
 * Screen wrappers
 */

/*
 * CloseScreen wrapper -- unwrap everything, free the private data
 * and call the wrapped function
 */

static Bool
rfbSpriteCloseScreen(ScreenPtr pScreen)
{
    rfbSpriteScreenPtr pScreenPriv = GetSpriteScreen(pScreen);

    pScreen->CloseScreen = pScreenPriv->CloseScreen;
    pScreen->GetImage = pScreenPriv->GetImage;
    pScreen->GetSpans = pScreenPriv->GetSpans;
    pScreen->SourceValidate = pScreenPriv->SourceValidate;
    pScreen->InstallColormap = pScreenPriv->InstallColormap;
    pScreen->StoreColors = pScreenPriv->StoreColors;

    DamageDestroy(pScreenPriv->pDamage);

    free(pScreenPriv);

    return (*pScreen->CloseScreen) (pScreen);
}

static void
rfbSpriteGetImage(DrawablePtr pDrawable, int sx, int sy, int w, int h,
                  unsigned int format, unsigned long planemask, char *pdstLine)
{
    ScreenPtr pScreen = pDrawable->pScreen;
    DeviceIntPtr pDev;
    rfbCursorInfoPtr pCursorInfo;
    rfbSpriteScreenPtr pPriv = GetSpriteScreen(pScreen);

    SCREEN_PROLOGUE(pPriv, pScreen, GetImage);

    if (pDrawable->type == DRAWABLE_WINDOW) {
        for (pDev = inputInfo.devices; pDev; pDev = pDev->next) {
            if (DevHasCursor(pDev)) {
                pCursorInfo = RFBSPRITE(pDev);
                if (pCursorInfo->isUp && pCursorInfo->pScreen == pScreen &&
                    ORG_OVERLAP(&pCursorInfo->saved, pDrawable->x, pDrawable->y,
                                sx, sy, w, h)) {
                    SPRITE_DEBUG(("GetImage remove\n"));
                    rfbSpriteRemoveCursor(pDev, pScreen);
                }
            }
        }
    }

    (*pScreen->GetImage) (pDrawable, sx, sy, w, h, format, planemask, pdstLine);

    SCREEN_EPILOGUE(pPriv, pScreen, GetImage);
}

static void
rfbSpriteGetSpans(DrawablePtr pDrawable, int wMax, DDXPointPtr ppt,
                  int *pwidth, int nspans, char *pdstStart)
{
    ScreenPtr pScreen = pDrawable->pScreen;
    DeviceIntPtr pDev;
    rfbCursorInfoPtr pCursorInfo;
    rfbSpriteScreenPtr pPriv = GetSpriteScreen(pScreen);

    SCREEN_PROLOGUE(pPriv, pScreen, GetSpans);

    if (pDrawable->type == DRAWABLE_WINDOW) {
        for (pDev = inputInfo.devices; pDev; pDev = pDev->next) {
            if (DevHasCursor(pDev)) {
                pCursorInfo = RFBSPRITE(pDev);

                if (pCursorInfo->isUp && pCursorInfo->pScreen == pScreen) {
                    DDXPointPtr pts;
                    int *widths;
                    int nPts;
                    int xorg, yorg;

                    xorg = pDrawable->x;
                    yorg = pDrawable->y;

                    for (pts = ppt, widths = pwidth, nPts = nspans;
                         nPts--; pts++, widths++) {
                        if (SPN_OVERLAP(&pCursorInfo->saved, pts->y + yorg,
                                        pts->x + xorg, *widths)) {
                            SPRITE_DEBUG(("GetSpans remove\n"));
                            rfbSpriteRemoveCursor(pDev, pScreen);
                            break;
                        }
                    }
                }
            }
        }
    }

    (*pScreen->GetSpans) (pDrawable, wMax, ppt, pwidth, nspans, pdstStart);

    SCREEN_EPILOGUE(pPriv, pScreen, GetSpans);
}

static void
rfbSpriteSourceValidate(DrawablePtr pDrawable, int x, int y, int width,
                        int height, unsigned int subWindowMode)
{
    ScreenPtr pScreen = pDrawable->pScreen;
    DeviceIntPtr pDev;
    rfbCursorInfoPtr pCursorInfo;
    rfbSpriteScreenPtr pPriv = GetSpriteScreen(pScreen);

    SCREEN_PROLOGUE(pPriv, pScreen, SourceValidate);

    if (pDrawable->type == DRAWABLE_WINDOW) {
        for (pDev = inputInfo.devices; pDev; pDev = pDev->next) {
            if (DevHasCursor(pDev)) {
                pCursorInfo = RFBSPRITE(pDev);
                if (pCursorInfo->isUp && pCursorInfo->pScreen == pScreen &&
                    ORG_OVERLAP(&pCursorInfo->saved, pDrawable->x, pDrawable->y,
                                x, y, width, height)) {
                    SPRITE_DEBUG(("SourceValidate remove\n"));
                    rfbSpriteRemoveCursor(pDev, pScreen);
                }
            }
        }
    }

    if (pScreen->SourceValidate)
        (*pScreen->SourceValidate) (pDrawable, x, y, width, height,
                                    subWindowMode);

    SCREEN_EPILOGUE(pPriv, pScreen, SourceValidate);
}

static void
rfbSpriteCopyWindow(WindowPtr pWindow, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
    ScreenPtr pScreen = pWindow->drawable.pScreen;
    DeviceIntPtr pDev;
    rfbCursorInfoPtr pCursorInfo;
    rfbSpriteScreenPtr pPriv = GetSpriteScreen(pScreen);

    SCREEN_PROLOGUE(pPriv, pScreen, CopyWindow);

    for (pDev = inputInfo.devices; pDev; pDev = pDev->next) {
        if (DevHasCursor(pDev)) {
            pCursorInfo = RFBSPRITE(pDev);
            /*
             * Damage will take care of destination check
             */
            if (pCursorInfo->isUp && pCursorInfo->pScreen == pScreen &&
                RegionContainsRect(prgnSrc, &pCursorInfo->saved) != rgnOUT) {
                SPRITE_DEBUG(("CopyWindow remove\n"));
                rfbSpriteRemoveCursor(pDev, pScreen);
            }
        }
    }

    (*pScreen->CopyWindow) (pWindow, ptOldOrg, prgnSrc);
    SCREEN_EPILOGUE(pPriv, pScreen, CopyWindow);
}

static void
rfbSpriteBlockHandler(ScreenPtr pScreen, void *timeout)
{
    rfbSpriteScreenPtr pPriv = GetSpriteScreen(pScreen);
    DeviceIntPtr pDev;
    rfbCursorInfoPtr pCursorInfo;
    Bool WorkToDo = FALSE;

    SCREEN_PROLOGUE(pPriv, pScreen, BlockHandler);

    for (pDev = inputInfo.devices; pDev; pDev = pDev->next) {
        if (DevHasCursor(pDev)) {
            pCursorInfo = RFBSPRITE(pDev);
            if (pCursorInfo && !pCursorInfo->isUp
                && pCursorInfo->pScreen == pScreen && pCursorInfo->shouldBeUp) {
                SPRITE_DEBUG(("BlockHandler save"));
                rfbSpriteSaveUnderCursor(pDev, pScreen);
            }
        }
    }
    for (pDev = inputInfo.devices; pDev; pDev = pDev->next) {
        if (DevHasCursor(pDev)) {
            pCursorInfo = RFBSPRITE(pDev);
            if (pCursorInfo && !pCursorInfo->isUp &&
                pCursorInfo->pScreen == pScreen && pCursorInfo->shouldBeUp) {
                SPRITE_DEBUG(("BlockHandler restore\n"));
                rfbSpriteRestoreCursor(pDev, pScreen);
                if (!pCursorInfo->isUp)
                    WorkToDo = TRUE;
            }
        }
    }

    (*pScreen->BlockHandler) (pScreen, timeout);

    if (WorkToDo)
        SCREEN_EPILOGUE(pPriv, pScreen, BlockHandler);
    else
        pPriv->BlockHandler = NULL;
}

static void
rfbSpriteInstallColormap(ColormapPtr pMap)
{
    ScreenPtr pScreen = pMap->pScreen;
    rfbSpriteScreenPtr pPriv = GetSpriteScreen(pScreen);

    SCREEN_PROLOGUE(pPriv, pScreen, InstallColormap);

    (*pScreen->InstallColormap) (pMap);

    SCREEN_EPILOGUE(pPriv, pScreen, InstallColormap);

    /* InstallColormap can be called before devices are initialized. */
    pPriv->pInstalledMap = pMap;
    if (pPriv->pColormap != pMap) {
        DeviceIntPtr pDev;
        rfbCursorInfoPtr pCursorInfo;

        for (pDev = inputInfo.devices; pDev; pDev = pDev->next) {
            if (DevHasCursor(pDev)) {
                pCursorInfo = RFBSPRITE(pDev);
                pCursorInfo->checkPixels = TRUE;
                if (pCursorInfo->isUp && pCursorInfo->pScreen == pScreen)
                    rfbSpriteRemoveCursor(pDev, pScreen);
            }
        }

    }
}

static void
rfbSpriteStoreColors(ColormapPtr pMap, int ndef, xColorItem * pdef)
{
    ScreenPtr pScreen = pMap->pScreen;
    rfbSpriteScreenPtr pPriv = GetSpriteScreen(pScreen);
    int i;
    int updated;
    VisualPtr pVisual;
    DeviceIntPtr pDev;
    rfbCursorInfoPtr pCursorInfo;

    SCREEN_PROLOGUE(pPriv, pScreen, StoreColors);

    (*pScreen->StoreColors) (pMap, ndef, pdef);

    SCREEN_EPILOGUE(pPriv, pScreen, StoreColors);

    if (pPriv->pColormap == pMap) {
        updated = 0;
        pVisual = pMap->pVisual;
        if (pVisual->class == DirectColor) {
            /* Direct color - match on any of the subfields */

#define MaskMatch(a,b,mask) (((a) & (pVisual->mask)) == ((b) & (pVisual->mask)))

#define UpdateDAC(dev, plane,dac,mask) {\
    if (MaskMatch (dev->colors[plane].pixel,pdef[i].pixel,mask)) {\
	dev->colors[plane].dac = pdef[i].dac; \
	updated = 1; \
    } \
}

#define CheckDirect(dev, plane) \
	    UpdateDAC(dev, plane,red,redMask) \
	    UpdateDAC(dev, plane,green,greenMask) \
	    UpdateDAC(dev, plane,blue,blueMask)

            for (i = 0; i < ndef; i++) {
                CheckDirect(pPriv, SOURCE_COLOR)
                    CheckDirect(pPriv, MASK_COLOR)
            }
        }
        else {
            /* PseudoColor/GrayScale - match on exact pixel */
            for (i = 0; i < ndef; i++) {
                if (pdef[i].pixel == pPriv->colors[SOURCE_COLOR].pixel) {
                    pPriv->colors[SOURCE_COLOR] = pdef[i];
                    if (++updated == 2)
                        break;
                }
                if (pdef[i].pixel == pPriv->colors[MASK_COLOR].pixel) {
                    pPriv->colors[MASK_COLOR] = pdef[i];
                    if (++updated == 2)
                        break;
                }
            }
        }
        if (updated) {
            for (pDev = inputInfo.devices; pDev; pDev = pDev->next) {
                if (DevHasCursor(pDev)) {
                    pCursorInfo = RFBSPRITE(pDev);
                    pCursorInfo->checkPixels = TRUE;
                    if (pCursorInfo->isUp && pCursorInfo->pScreen == pScreen)
                        rfbSpriteRemoveCursor(pDev, pScreen);
                }
            }
        }
    }
}

static void
rfbSpriteFindColors(rfbCursorInfoPtr pDevCursor, ScreenPtr pScreen)
{
    rfbSpriteScreenPtr pScreenPriv = GetSpriteScreen(pScreen);
    CursorPtr pCursor;
    xColorItem *sourceColor, *maskColor;

    pCursor = pDevCursor->pCursor;
    sourceColor = &pScreenPriv->colors[SOURCE_COLOR];
    maskColor = &pScreenPriv->colors[MASK_COLOR];
    if (pScreenPriv->pColormap != pScreenPriv->pInstalledMap ||
        !(pCursor->foreRed == sourceColor->red &&
          pCursor->foreGreen == sourceColor->green &&
          pCursor->foreBlue == sourceColor->blue &&
          pCursor->backRed == maskColor->red &&
          pCursor->backGreen == maskColor->green &&
          pCursor->backBlue == maskColor->blue)) {
        pScreenPriv->pColormap = pScreenPriv->pInstalledMap;
        sourceColor->red = pCursor->foreRed;
        sourceColor->green = pCursor->foreGreen;
        sourceColor->blue = pCursor->foreBlue;
        FakeAllocColor(pScreenPriv->pColormap, sourceColor);
        maskColor->red = pCursor->backRed;
        maskColor->green = pCursor->backGreen;
        maskColor->blue = pCursor->backBlue;
        FakeAllocColor(pScreenPriv->pColormap, maskColor);
        /* "free" the pixels right away, don't let this confuse you */
        FakeFreeColor(pScreenPriv->pColormap, sourceColor->pixel);
        FakeFreeColor(pScreenPriv->pColormap, maskColor->pixel);
    }

    pDevCursor->checkPixels = FALSE;

}

/*
 * miPointer interface routines
 */

#define SPRITE_PAD  8

static Bool
rfbSpriteRealizeCursor(DeviceIntPtr pDev, ScreenPtr pScreen, CursorPtr pCursor)
{
    rfbCursorInfoPtr pCursorInfo;

    if (IsFloating(pDev))
        return FALSE;

    pCursorInfo = RFBSPRITE(pDev);

    if (pCursor == pCursorInfo->pCursor)
        pCursorInfo->checkPixels = TRUE;

    return rfbDCRealizeCursor(pScreen, pCursor);
}

static Bool
rfbSpriteUnrealizeCursor(DeviceIntPtr pDev, ScreenPtr pScreen, CursorPtr pCursor)
{
    return rfbDCUnrealizeCursor(pScreen, pCursor);
}

static void
rfbSpriteSetCursor(DeviceIntPtr pDev, ScreenPtr pScreen,
                   CursorPtr pCursor, int x, int y)
{
    rfbCursorInfoPtr pPointer;
    rfbSpriteScreenPtr pScreenPriv;
    rfbClientPtr cl, nextCl;

    if (IsFloating(pDev))
        return;

    pPointer = RFBSPRITE(pDev);
    pScreenPriv = GetSpriteScreen(pScreen);

    if (!pCursor) {
        if (pPointer->shouldBeUp)
            --pScreenPriv->numberOfCursors;
        pPointer->shouldBeUp = FALSE;
        if (pPointer->isUp)
            rfbSpriteRemoveCursor(pDev, pScreen);
        if (pScreenPriv->numberOfCursors == 0)
            rfbSpriteDisableDamage(pScreen, pScreenPriv);
        pPointer->pCursor = 0;
        return;
    }
    if (!pPointer->shouldBeUp)
        pScreenPriv->numberOfCursors++;
    pPointer->shouldBeUp = TRUE;
    if (!pPointer->isUp)
        rfbSpriteRegisterBlockHandler(pScreen, pScreenPriv);
    if (pPointer->x == x &&
        pPointer->y == y &&
        pPointer->pCursor == pCursor && !pPointer->checkPixels) {
        return;
    }
    pPointer->x = x;
    pPointer->y = y;
    pPointer->pCacheWin = NullWindow;
    if (pPointer->checkPixels || pPointer->pCursor != pCursor) {
        pPointer->pCursor = pCursor;
        rfbSpriteFindColors(pPointer, pScreen);
    }
    if (pPointer->isUp) {
        /* TODO: reimplement flicker-free MoveCursor */
        SPRITE_DEBUG(("SetCursor remove %d\n", pDev->id));
        rfbSpriteRemoveCursor(pDev, pScreen);
    }

    if (!pPointer->isUp && pPointer->pCursor) {
        SPRITE_DEBUG(("SetCursor restore %d\n", pDev->id));
        rfbSpriteSaveUnderCursor(pDev, pScreen);
        rfbSpriteRestoreCursor(pDev, pScreen);
    }

    for (cl = rfbClientHead; cl; cl = nextCl) {
        nextCl = cl->next;
        if (cl->enableCursorPosUpdates) {
            if (x == cl->cursorX && y == cl->cursorY) {
                cl->cursorWasMoved = FALSE;
                continue;
            }
            cl->cursorWasMoved = TRUE;
        }
        if (!cl->deferredUpdateScheduled)
            /* cursorIsDrawn is guaranteed to be FALSE here, so we definitely
               want to send a screen update to the client, even if that's only
               putting up the cursor */
            rfbSendFramebufferUpdate(cl);
    }
}

static void
rfbSpriteMoveCursor(DeviceIntPtr pDev, ScreenPtr pScreen, int x, int y)
{
    CursorPtr pCursor;

    if (IsFloating(pDev))
        return;

    pCursor = RFBSPRITE(pDev)->pCursor;

    rfbSpriteSetCursor(pDev, pScreen, pCursor, x, y);
}

static Bool
rfbSpriteDeviceCursorInitialize(DeviceIntPtr pDev, ScreenPtr pScreen)
{
    int ret = rfbDCDeviceInitialize(pDev, pScreen);

    if (ret) {
        rfbCursorInfoPtr pCursorInfo;

        pCursorInfo =
            dixLookupPrivate(&pDev->devPrivates, rfbSpriteDevPrivatesKey);
        pCursorInfo->pCursor = NULL;
        pCursorInfo->x = 0;
        pCursorInfo->y = 0;
        pCursorInfo->isUp = FALSE;
        pCursorInfo->shouldBeUp = FALSE;
        pCursorInfo->pCacheWin = NullWindow;
        pCursorInfo->isInCacheWin = FALSE;
        pCursorInfo->checkPixels = TRUE;
        pCursorInfo->pScreen = FALSE;
    }

    return ret;
}

static void
rfbSpriteDeviceCursorCleanup(DeviceIntPtr pDev, ScreenPtr pScreen)
{
    rfbCursorInfoPtr pCursorInfo =
        dixLookupPrivate(&pDev->devPrivates, rfbSpriteDevPrivatesKey);

    if (DevHasCursor(pDev))
        rfbDCDeviceCleanup(pDev, pScreen);

    memset(pCursorInfo, 0, sizeof(rfbCursorInfoRec));
}

/*
 * undraw/draw cursor
 */

static void
rfbSpriteRemoveCursor(DeviceIntPtr pDev, ScreenPtr pScreen)
{
    rfbSpriteScreenPtr pScreenPriv;
    rfbCursorInfoPtr pCursorInfo;

    if (!rfbFB.cursorIsDrawn || IsFloating(pDev))
        return;

    DamageDrawInternal(pScreen, TRUE);
    pScreenPriv = GetSpriteScreen(pScreen);
    pCursorInfo = RFBSPRITE(pDev);

    rfbFB.dontSendFramebufferUpdate = TRUE;

    rfbSpriteIsDown(pCursorInfo);
    rfbSpriteRegisterBlockHandler(pScreen, pScreenPriv);
    pCursorInfo->pCacheWin = NullWindow;
    rfbSpriteDisableDamage(pScreen, pScreenPriv);
    if (!rfbDCRestoreUnderCursor(pDev,
                                pScreen,
                                pCursorInfo->saved.x1,
                                pCursorInfo->saved.y1,
                                pCursorInfo->saved.x2 -
                                pCursorInfo->saved.x1,
                                pCursorInfo->saved.y2 -
                                pCursorInfo->saved.y1)) {
        rfbSpriteIsUp(pCursorInfo);
    }
    rfbSpriteEnableDamage(pScreen, pScreenPriv);
    DamageDrawInternal(pScreen, FALSE);

    rfbFB.dontSendFramebufferUpdate = FALSE;
}

/*
 * Called from the block handler, saves area under cursor
 * before waiting for something to do.
 */

static void
rfbSpriteSaveUnderCursor(DeviceIntPtr pDev, ScreenPtr pScreen)
{
    rfbSpriteScreenPtr pScreenPriv;
    rfbCursorInfoPtr pCursorInfo;

    if (IsFloating(pDev))
        return;

    DamageDrawInternal(pScreen, TRUE);
    pScreenPriv = GetSpriteScreen(pScreen);
    pCursorInfo = RFBSPRITE(pDev);

    rfbSpriteComputeSaved(pDev, pScreen);

    rfbSpriteDisableDamage(pScreen, pScreenPriv);

    rfbDCSaveUnderCursor(pDev,
                         pScreen,
                         pCursorInfo->saved.x1,
                         pCursorInfo->saved.y1,
                         pCursorInfo->saved.x2 -
                         pCursorInfo->saved.x1,
                         pCursorInfo->saved.y2 - pCursorInfo->saved.y1);
    SPRITE_DEBUG(("SaveUnderCursor %d\n", pDev->id));
    rfbSpriteEnableDamage(pScreen, pScreenPriv);
    DamageDrawInternal(pScreen, FALSE);
}

/*
 * Called from the block handler, restores the cursor
 * before waiting for something to do.
 */

static void
rfbSpriteRestoreCursor(DeviceIntPtr pDev, ScreenPtr pScreen)
{
    rfbSpriteScreenPtr pScreenPriv;
    int x, y;
    CursorPtr pCursor;
    rfbCursorInfoPtr pCursorInfo;

    if (IsFloating(pDev))
        return;

    DamageDrawInternal(pScreen, TRUE);
    pScreenPriv = GetSpriteScreen(pScreen);
    pCursorInfo = RFBSPRITE(pDev);

    rfbSpriteComputeSaved(pDev, pScreen);
    pCursor = pCursorInfo->pCursor;

    if (rfbFB.cursorIsDrawn || !pCursor) {
        DamageDrawInternal(pScreen, FALSE);
        return;
    }

    rfbFB.dontSendFramebufferUpdate = TRUE;

    x = pCursorInfo->x - (int) pCursor->bits->xhot;
    y = pCursorInfo->y - (int) pCursor->bits->yhot;
    rfbSpriteDisableDamage(pScreen, pScreenPriv);
    SPRITE_DEBUG(("RestoreCursor %d\n", pDev->id));
    if (pCursorInfo->checkPixels)
        rfbSpriteFindColors(pCursorInfo, pScreen);
    if (rfbDCPutUpCursor(pDev, pScreen,
                         pCursor, x, y,
                         pScreenPriv->colors[SOURCE_COLOR].pixel,
                         pScreenPriv->colors[MASK_COLOR].pixel)) {
        rfbSpriteIsUp(pCursorInfo);
        pCursorInfo->pScreen = pScreen;
    }
    rfbSpriteEnableDamage(pScreen, pScreenPriv);
    DamageDrawInternal(pScreen, FALSE);

    rfbFB.dontSendFramebufferUpdate = FALSE;
}

/*
 * compute the desired area of the screen to save
 */

static void
rfbSpriteComputeSaved(DeviceIntPtr pDev, ScreenPtr pScreen)
{
    int x, y, w, h;
    int wpad, hpad;
    CursorPtr pCursor;
    rfbCursorInfoPtr pCursorInfo;

    if (IsFloating(pDev))
        return;

    pCursorInfo = RFBSPRITE(pDev);

    pCursor = pCursorInfo->pCursor;

    if (!pCursor)
        return;

    x = pCursorInfo->x - (int) pCursor->bits->xhot;
    y = pCursorInfo->y - (int) pCursor->bits->yhot;
    w = pCursor->bits->width;
    h = pCursor->bits->height;
    wpad = SPRITE_PAD;
    hpad = SPRITE_PAD;
    pCursorInfo->saved.x1 = x - wpad;
    pCursorInfo->saved.y1 = y - hpad;
    pCursorInfo->saved.x2 = pCursorInfo->saved.x1 + w + wpad * 2;
    pCursorInfo->saved.y2 = pCursorInfo->saved.y1 + h + hpad * 2;
}


/*
 * this function is called when the cursor shape is being changed
 */

static Bool
rfbDisplayCursor(DeviceIntPtr pDev, ScreenPtr pScreen, CursorPtr pCursor)
{
    rfbClientPtr        cl, nextCl;
    rfbSpriteScreenPtr  pPriv;
    Bool                result;

    pPriv = GetSpriteScreen(pScreen);
    result = (*pPriv->DisplayCursor)(pDev, pScreen, pCursor);

    for (cl = rfbClientHead; cl; cl = nextCl) {
        nextCl = cl->next;
        if (cl->enableCursorShapeUpdates) {
            cl->cursorWasChanged = TRUE;
            if (!cl->deferredUpdateScheduled)
                rfbSendFramebufferUpdate(cl);
        }
    }

    return result;
}


/*
 * obtain current cursor pointer
 */

CursorPtr
rfbSpriteGetCursorPtr(ScreenPtr pScreen)
{
    rfbCursorInfoPtr pCursorInfo;
    DeviceIntPtr pDev;

    for (pDev = inputInfo.devices; pDev; pDev = pDev->next) {
        if (DevHasCursor(pDev)) {
            pCursorInfo = RFBSPRITE(pDev);
            return pCursorInfo->pCursor;
        }
    }

    return NULL;
}


/*
 * obtain current cursor position
 */

void
rfbSpriteGetCursorPos(ScreenPtr pScreen, int *px, int *py)
{
    rfbCursorInfoPtr pCursorInfo;
    DeviceIntPtr pDev;

    for (pDev = inputInfo.devices; pDev; pDev = pDev->next) {
        if (DevHasCursor(pDev)) {
            pCursorInfo = RFBSPRITE(pDev);
            *px = pCursorInfo->x;
            *py = pCursorInfo->y;
            return;
        }
    }

    *px = *py = 0;
    return;
}


/*
 * Wrappers that allow us to interface with the older, non-device-aware code
 * in rfbserver.c
 */

void
rfbSpriteRemoveCursorAllDev(ScreenPtr pScreen)
{
    DeviceIntPtr pDev;

    for (pDev = inputInfo.devices; pDev; pDev = pDev->next) {
        if (DevHasCursor(pDev))
            rfbSpriteRemoveCursor(pDev, pScreen);
    }
}

void
rfbSpriteRestoreCursorAllDev(ScreenPtr pScreen)
{
    DeviceIntPtr pDev;

    for (pDev = inputInfo.devices; pDev; pDev = pDev->next) {
        if (DevHasCursor(pDev)) {
            rfbSpriteSaveUnderCursor(pDev, pScreen);
            rfbSpriteRestoreCursor(pDev, pScreen);
        }
    }
}
