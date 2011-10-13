/* $Xorg: panoramiX.c,v 1.5 2000/08/17 19:47:57 cpqbld Exp $ */
/*****************************************************************
Copyright (c) 1991, 1997 Digital Equipment Corporation, Maynard, Massachusetts.
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
DIGITAL EQUIPMENT CORPORATION BE LIABLE FOR ANY CLAIM, DAMAGES, INCLUDING,
BUT NOT LIMITED TO CONSEQUENTIAL OR INCIDENTAL DAMAGES, OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Digital Equipment Corporation
shall not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from Digital
Equipment Corporation.
******************************************************************/
/* $XFree86: xc/programs/Xserver/Xext/panoramiX.c,v 3.37tsi Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifdef HAVE_DMX_CONFIG_H
#include <dmx-config.h>
#endif

#define NEED_REPLIES
#include <stdio.h>
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/Xarch.h>
#include "misc.h"
#include "cursor.h"
#include "cursorstr.h"
#include "extnsionst.h"
#include "dixstruct.h"
#include "gc.h"
#include "gcstruct.h"
#include "scrnintstr.h"
#include "window.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "panoramiX.h"
#include <X11/extensions/panoramiXproto.h>
#include "panoramiXsrv.h"
#include "globals.h"
#include "servermd.h"
#include "resource.h"
#ifdef RENDER
#include "picturestr.h"
#endif
#include "modinit.h"


#ifdef GLXPROXY
extern VisualPtr glxMatchVisual(ScreenPtr pScreen,
				VisualPtr pVisual,
				ScreenPtr pMatchScreen);
#endif

#if 0
static unsigned char PanoramiXReqCode = 0;
#endif
/*
 *	PanoramiX data declarations
 */

int 		PanoramiXPixWidth = 0;
int 		PanoramiXPixHeight = 0;
int 		PanoramiXNumScreens = 0;

PanoramiXData 	*panoramiXdataPtr = NULL;
RegionRec   	PanoramiXScreenRegion = {{0, 0, 0, 0}, NULL};

static int		PanoramiXNumDepths;
static DepthPtr		PanoramiXDepths;
static int		PanoramiXNumVisuals;
static VisualPtr	PanoramiXVisuals;

/* We support at most 256 visuals */
XID		*PanoramiXVisualTable = NULL;

unsigned long XRC_DRAWABLE;
unsigned long XRT_WINDOW;
unsigned long XRT_PIXMAP;
unsigned long XRT_GC;
unsigned long XRT_COLORMAP;

/*
 *	Function prototypes
 */

static int panoramiXGeneration;
static int ProcPanoramiXDispatch(ClientPtr client); 

static void PanoramiXResetProc(ExtensionEntry*);

/*
 *	External references for functions and data variables
 */

#include "panoramiXh.h"

int (* SavedProcVector[256]) (ClientPtr client) = { NULL, };
ScreenInfo *GlobalScrInfo = NULL;

static int PanoramiXGCIndex = -1;
static int PanoramiXScreenIndex = -1;

typedef struct {
  DDXPointRec clipOrg;
  DDXPointRec patOrg;
  GCFuncs *wrapFuncs;
} PanoramiXGCRec, *PanoramiXGCPtr;

typedef struct {
  CreateGCProcPtr	CreateGC;
  CloseScreenProcPtr	CloseScreen;
} PanoramiXScreenRec, *PanoramiXScreenPtr;

RegionRec XineramaScreenRegions[MAXSCREENS];

static void XineramaValidateGC(GCPtr, unsigned long, DrawablePtr);
static void XineramaChangeGC(GCPtr, unsigned long);
static void XineramaCopyGC(GCPtr, unsigned long, GCPtr);
static void XineramaDestroyGC(GCPtr);
static void XineramaChangeClip(GCPtr, int, pointer, int);
static void XineramaDestroyClip(GCPtr);
static void XineramaCopyClip(GCPtr, GCPtr);

GCFuncs XineramaGCFuncs = {
    XineramaValidateGC, XineramaChangeGC, XineramaCopyGC, XineramaDestroyGC,
    XineramaChangeClip, XineramaDestroyClip, XineramaCopyClip
};

#define Xinerama_GC_FUNC_PROLOGUE(pGC)\
    PanoramiXGCPtr  pGCPriv = \
		(PanoramiXGCPtr) (pGC)->devPrivates[PanoramiXGCIndex].ptr;\
    (pGC)->funcs = pGCPriv->wrapFuncs;

#define Xinerama_GC_FUNC_EPILOGUE(pGC)\
    pGCPriv->wrapFuncs = (pGC)->funcs;\
    (pGC)->funcs = &XineramaGCFuncs;


static Bool
XineramaCloseScreen (int i, ScreenPtr pScreen)
{
    PanoramiXScreenPtr pScreenPriv = 
        (PanoramiXScreenPtr) pScreen->devPrivates[PanoramiXScreenIndex].ptr;

    pScreen->CloseScreen = pScreenPriv->CloseScreen;
    pScreen->CreateGC = pScreenPriv->CreateGC;

    REGION_UNINIT(pScreen, &XineramaScreenRegions[pScreen->myNum]);
    if (pScreen->myNum == 0)
	REGION_UNINIT(pScreen, &PanoramiXScreenRegion);

    xfree ((pointer) pScreenPriv);

    return (*pScreen->CloseScreen) (i, pScreen);
}

Bool
XineramaCreateGC(GCPtr pGC)
{
    ScreenPtr pScreen = pGC->pScreen;
    PanoramiXScreenPtr pScreenPriv = 
        (PanoramiXScreenPtr) pScreen->devPrivates[PanoramiXScreenIndex].ptr;
    Bool ret;

    pScreen->CreateGC = pScreenPriv->CreateGC;
    if((ret = (*pScreen->CreateGC)(pGC))) {
	PanoramiXGCPtr pGCPriv = 
		(PanoramiXGCPtr) pGC->devPrivates[PanoramiXGCIndex].ptr;

	pGCPriv->wrapFuncs = pGC->funcs;
        pGC->funcs = &XineramaGCFuncs;

	pGCPriv->clipOrg.x = pGC->clipOrg.x; 
	pGCPriv->clipOrg.y = pGC->clipOrg.y;
	pGCPriv->patOrg.x = pGC->patOrg.x;
	pGCPriv->patOrg.y = pGC->patOrg.y;
    }
    pScreen->CreateGC = XineramaCreateGC;

    return ret;
}

static void
XineramaValidateGC(
   GCPtr         pGC,
   unsigned long changes,
   DrawablePtr   pDraw 
){
    Xinerama_GC_FUNC_PROLOGUE (pGC);

    if((pDraw->type == DRAWABLE_WINDOW) && !(((WindowPtr)pDraw)->parent)) {
	/* the root window */
	int x_off = panoramiXdataPtr[pGC->pScreen->myNum].x;
	int y_off = panoramiXdataPtr[pGC->pScreen->myNum].y;
	int new_val;

	new_val = pGCPriv->clipOrg.x - x_off;
	if(pGC->clipOrg.x != new_val) {
	    pGC->clipOrg.x = new_val;
	    changes |= GCClipXOrigin;
	}
	new_val = pGCPriv->clipOrg.y - y_off;
	if(pGC->clipOrg.y != new_val) {
	    pGC->clipOrg.y = new_val;
	    changes |= GCClipYOrigin;
	}
	new_val = pGCPriv->patOrg.x - x_off;
	if(pGC->patOrg.x != new_val) {
	    pGC->patOrg.x = new_val;
	    changes |= GCTileStipXOrigin;
	}
	new_val = pGCPriv->patOrg.y - y_off;
	if(pGC->patOrg.y != new_val) {
	    pGC->patOrg.y = new_val;
	    changes |= GCTileStipYOrigin;
	}
    } else {
	if(pGC->clipOrg.x != pGCPriv->clipOrg.x) {
	    pGC->clipOrg.x = pGCPriv->clipOrg.x;
	    changes |= GCClipXOrigin;
	}
	if(pGC->clipOrg.y != pGCPriv->clipOrg.y) {
	    pGC->clipOrg.y = pGCPriv->clipOrg.y;
	    changes |= GCClipYOrigin;
	}
	if(pGC->patOrg.x != pGCPriv->patOrg.x) {
	    pGC->patOrg.x = pGCPriv->patOrg.x;
	    changes |= GCTileStipXOrigin;
	}
	if(pGC->patOrg.y != pGCPriv->patOrg.y) {
	    pGC->patOrg.y = pGCPriv->patOrg.y;
	    changes |= GCTileStipYOrigin;
	}
    }
  
    (*pGC->funcs->ValidateGC)(pGC, changes, pDraw);
    Xinerama_GC_FUNC_EPILOGUE (pGC);
}

static void
XineramaDestroyGC(GCPtr pGC)
{
    Xinerama_GC_FUNC_PROLOGUE (pGC);
    (*pGC->funcs->DestroyGC)(pGC);
    Xinerama_GC_FUNC_EPILOGUE (pGC);
}

static void
XineramaChangeGC (
    GCPtr	    pGC,
    unsigned long   mask
){
    Xinerama_GC_FUNC_PROLOGUE (pGC);

    if(mask & GCTileStipXOrigin)
	pGCPriv->patOrg.x = pGC->patOrg.x;
    if(mask & GCTileStipYOrigin)
	pGCPriv->patOrg.y = pGC->patOrg.y;
    if(mask & GCClipXOrigin)
	pGCPriv->clipOrg.x = pGC->clipOrg.x; 
    if(mask & GCClipYOrigin)
	pGCPriv->clipOrg.y = pGC->clipOrg.y;

    (*pGC->funcs->ChangeGC) (pGC, mask);
    Xinerama_GC_FUNC_EPILOGUE (pGC);
}

static void
XineramaCopyGC (
    GCPtr	    pGCSrc, 
    unsigned long   mask,
    GCPtr	    pGCDst
){
    PanoramiXGCPtr pSrcPriv =
                (PanoramiXGCPtr) pGCSrc->devPrivates[PanoramiXGCIndex].ptr;
    Xinerama_GC_FUNC_PROLOGUE (pGCDst);

    if(mask & GCTileStipXOrigin)
        pGCPriv->patOrg.x = pSrcPriv->patOrg.x;
    if(mask & GCTileStipYOrigin)
        pGCPriv->patOrg.y = pSrcPriv->patOrg.y;
    if(mask & GCClipXOrigin)
        pGCPriv->clipOrg.x = pSrcPriv->clipOrg.x;
    if(mask & GCClipYOrigin)
        pGCPriv->clipOrg.y = pSrcPriv->clipOrg.y;

    (*pGCDst->funcs->CopyGC) (pGCSrc, mask, pGCDst);
    Xinerama_GC_FUNC_EPILOGUE (pGCDst);
}

static void
XineramaChangeClip (
    GCPtr   pGC,
    int		type,
    pointer	pvalue,
    int		nrects 
){
    Xinerama_GC_FUNC_PROLOGUE (pGC);
    (*pGC->funcs->ChangeClip) (pGC, type, pvalue, nrects);
    Xinerama_GC_FUNC_EPILOGUE (pGC);
}

static void
XineramaCopyClip(GCPtr pgcDst, GCPtr pgcSrc)
{
    Xinerama_GC_FUNC_PROLOGUE (pgcDst);
    (* pgcDst->funcs->CopyClip)(pgcDst, pgcSrc);
    Xinerama_GC_FUNC_EPILOGUE (pgcDst);
}

static void
XineramaDestroyClip(GCPtr pGC)
{
    Xinerama_GC_FUNC_PROLOGUE (pGC);
    (* pGC->funcs->DestroyClip)(pGC);
    Xinerama_GC_FUNC_EPILOGUE (pGC);
}



int
XineramaDeleteResource(pointer data, XID id)
{
    xfree(data);
    return 1;
}


static Bool 
XineramaFindIDOnAnyScreen(pointer resource, XID id, pointer privdata)
{
    PanoramiXRes *res = (PanoramiXRes*)resource;
    int j;

    FOR_NSCREENS(j) 
	if(res->info[j].id == *((XID*)privdata)) return TRUE;
    
    return FALSE;
}

PanoramiXRes *
PanoramiXFindIDOnAnyScreen(RESTYPE type, XID id)
{
    return LookupClientResourceComplex(clients[CLIENT_ID(id)], type,
		XineramaFindIDOnAnyScreen, &id);
}

typedef struct {
   int screen;
   int id;
} PanoramiXSearchData; 


static Bool 
XineramaFindIDByScrnum(pointer resource, XID id, pointer privdata)
{
    PanoramiXRes *res = (PanoramiXRes*)resource;
    PanoramiXSearchData *data = (PanoramiXSearchData*)privdata;
    
    return (res->info[data->screen].id == data->id);
}

PanoramiXRes *
PanoramiXFindIDByScrnum(RESTYPE type, XID id, int screen)
{
    PanoramiXSearchData data;

    if(!screen) 
	return LookupIDByType(id, type);

    data.screen = screen;
    data.id = id;

    return LookupClientResourceComplex(clients[CLIENT_ID(id)], type,
		XineramaFindIDByScrnum, &data);
}

WindowPtr
PanoramiXChangeWindow(int ScrnNum, WindowPtr pWin)
{
    int num = pWin->drawable.pScreen->myNum;

    if(num != ScrnNum) {
	PanoramiXRes	*win;

	win = PanoramiXFindIDByScrnum(XRT_WINDOW, pWin->drawable.id, num);

        if (win) 
           pWin = (WindowPtr) LookupIDByType(win->info[ScrnNum].id, RT_WINDOW);
    }
  
    return pWin;
}

typedef struct _connect_callback_list {
    void (*func)(void);
    struct _connect_callback_list *next;
} XineramaConnectionCallbackList;

static XineramaConnectionCallbackList *ConnectionCallbackList = NULL;

Bool
XineramaRegisterConnectionBlockCallback(void (*func)(void))
{
    XineramaConnectionCallbackList *newlist;

    if(!(newlist = xalloc(sizeof(XineramaConnectionCallbackList))))
	return FALSE;

    newlist->next = ConnectionCallbackList;
    newlist->func = func;
    ConnectionCallbackList = newlist;

    return TRUE;
}

static void XineramaInitData(ScreenPtr pScreen)
{
    int i, w, h;

    REGION_NULL(pScreen, &PanoramiXScreenRegion)
    for (i = 0; i < PanoramiXNumScreens; i++) {
	BoxRec TheBox;

        pScreen = screenInfo.screens[i];

	panoramiXdataPtr[i].x = dixScreenOrigins[i].x;
	panoramiXdataPtr[i].y = dixScreenOrigins[i].y;
	panoramiXdataPtr[i].width = pScreen->width;
	panoramiXdataPtr[i].height = pScreen->height;

	TheBox.x1 = panoramiXdataPtr[i].x;
	TheBox.x2 = TheBox.x1 + panoramiXdataPtr[i].width;
	TheBox.y1 = panoramiXdataPtr[i].y;
	TheBox.y2 = TheBox.y1 + panoramiXdataPtr[i].height;

	REGION_INIT(pScreen, &XineramaScreenRegions[i], &TheBox, 1);
	REGION_UNION(pScreen, &PanoramiXScreenRegion, &PanoramiXScreenRegion,
		     &XineramaScreenRegions[i]);
    }

    PanoramiXPixWidth = panoramiXdataPtr[0].x + panoramiXdataPtr[0].width;
    PanoramiXPixHeight = panoramiXdataPtr[0].y + panoramiXdataPtr[0].height;

    for (i = 1; i < PanoramiXNumScreens; i++) {
	w = panoramiXdataPtr[i].x + panoramiXdataPtr[i].width;
	h = panoramiXdataPtr[i].y + panoramiXdataPtr[i].height;

	if (PanoramiXPixWidth < w)
	    PanoramiXPixWidth = w;
	if (PanoramiXPixHeight < h)
	    PanoramiXPixHeight = h;
    }
}

void XineramaReinitData(ScreenPtr pScreen)
{
    int i;

    REGION_UNINIT(pScreen, &PanoramiXScreenRegion);
    for (i = 0; i < PanoramiXNumScreens; i++)
	REGION_UNINIT(pScreen, &XineramaScreenRegions[i]);

    XineramaInitData(pScreen);
}

/*
 *	PanoramiXExtensionInit():
 *		Called from InitExtensions in main().  
 *		Register PanoramiXeen Extension
 *		Initialize global variables.
 */ 

void PanoramiXExtensionInit(int argc, char *argv[])
{
    int 	     	i;
    Bool	     	success = FALSE;
    ExtensionEntry 	*extEntry;
    ScreenPtr		pScreen = screenInfo.screens[0];
    PanoramiXScreenPtr	pScreenPriv;

    if (noPanoramiXExtension) 
	return;

    GlobalScrInfo = &screenInfo;		/* For debug visibility */
    PanoramiXNumScreens = screenInfo.numScreens;
    if (PanoramiXNumScreens == 1) {		/* Only 1 screen 	*/
	noPanoramiXExtension = TRUE;
	return;
    }

    while (panoramiXGeneration != serverGeneration) {
	extEntry = AddExtension(PANORAMIX_PROTOCOL_NAME, 0,0, 
				ProcPanoramiXDispatch,
				SProcPanoramiXDispatch, PanoramiXResetProc, 
				StandardMinorOpcode);
	if (!extEntry) {
	    ErrorF("PanoramiXExtensionInit(): failed to AddExtension\n");
	    break;
 	}

#if 0
	PanoramiXReqCode = (unsigned char)extEntry->base;
#endif

	/*
	 *	First make sure all the basic allocations succeed.  If not,
	 *	run in non-PanoramiXeen mode.
	 */

	panoramiXdataPtr = (PanoramiXData *) 
		xcalloc(PanoramiXNumScreens, sizeof(PanoramiXData));

        BREAK_IF(!panoramiXdataPtr);
	BREAK_IF((PanoramiXGCIndex = AllocateGCPrivateIndex()) < 0);
	BREAK_IF((PanoramiXScreenIndex = AllocateScreenPrivateIndex()) < 0);
	
	for (i = 0; i < PanoramiXNumScreens; i++) {
	   pScreen = screenInfo.screens[i];
	   if(!AllocateGCPrivate(pScreen, PanoramiXGCIndex, 
						sizeof(PanoramiXGCRec))) {
		noPanoramiXExtension = TRUE;
		return;
	   }

	   pScreenPriv = xalloc(sizeof(PanoramiXScreenRec));
	   pScreen->devPrivates[PanoramiXScreenIndex].ptr = 
						(pointer)pScreenPriv;
	   if(!pScreenPriv) {
		noPanoramiXExtension = TRUE;
		return;
	   }
	
	   pScreenPriv->CreateGC = pScreen->CreateGC;
	   pScreenPriv->CloseScreen = pScreen->CloseScreen;
	
	   pScreen->CreateGC = XineramaCreateGC;
	   pScreen->CloseScreen = XineramaCloseScreen;
	}

	XRC_DRAWABLE = CreateNewResourceClass();
	XRT_WINDOW = CreateNewResourceType(XineramaDeleteResource) | 
						XRC_DRAWABLE;
	XRT_PIXMAP = CreateNewResourceType(XineramaDeleteResource) | 
						XRC_DRAWABLE;
	XRT_GC = CreateNewResourceType(XineramaDeleteResource);
	XRT_COLORMAP = CreateNewResourceType(XineramaDeleteResource);

	panoramiXGeneration = serverGeneration;
	success = TRUE;
    }

    if (!success) {
	noPanoramiXExtension = TRUE;
	ErrorF("%s Extension failed to initialize\n", PANORAMIX_PROTOCOL_NAME);
	return;
    }
  
    XineramaInitData(pScreen);

    /*
     *	Put our processes into the ProcVector
     */

    for (i = 256; i--; )
	SavedProcVector[i] = ProcVector[i];

    ProcVector[X_CreateWindow] = PanoramiXCreateWindow;
    ProcVector[X_ChangeWindowAttributes] = PanoramiXChangeWindowAttributes;
    ProcVector[X_DestroyWindow] = PanoramiXDestroyWindow;
    ProcVector[X_DestroySubwindows] = PanoramiXDestroySubwindows;
    ProcVector[X_ChangeSaveSet] = PanoramiXChangeSaveSet;
    ProcVector[X_ReparentWindow] = PanoramiXReparentWindow;
    ProcVector[X_MapWindow] = PanoramiXMapWindow;
    ProcVector[X_MapSubwindows] = PanoramiXMapSubwindows;
    ProcVector[X_UnmapWindow] = PanoramiXUnmapWindow;
    ProcVector[X_UnmapSubwindows] = PanoramiXUnmapSubwindows;
    ProcVector[X_ConfigureWindow] = PanoramiXConfigureWindow;
    ProcVector[X_CirculateWindow] = PanoramiXCirculateWindow;
    ProcVector[X_GetGeometry] = PanoramiXGetGeometry;
    ProcVector[X_TranslateCoords] = PanoramiXTranslateCoords;
    ProcVector[X_CreatePixmap] = PanoramiXCreatePixmap;
    ProcVector[X_FreePixmap] = PanoramiXFreePixmap;
    ProcVector[X_CreateGC] = PanoramiXCreateGC;
    ProcVector[X_ChangeGC] = PanoramiXChangeGC;
    ProcVector[X_CopyGC] = PanoramiXCopyGC;
    ProcVector[X_SetDashes] = PanoramiXSetDashes;
    ProcVector[X_SetClipRectangles] = PanoramiXSetClipRectangles;
    ProcVector[X_FreeGC] = PanoramiXFreeGC;
    ProcVector[X_ClearArea] = PanoramiXClearToBackground;
    ProcVector[X_CopyArea] = PanoramiXCopyArea;;
    ProcVector[X_CopyPlane] = PanoramiXCopyPlane;;
    ProcVector[X_PolyPoint] = PanoramiXPolyPoint;
    ProcVector[X_PolyLine] = PanoramiXPolyLine;
    ProcVector[X_PolySegment] = PanoramiXPolySegment;
    ProcVector[X_PolyRectangle] = PanoramiXPolyRectangle;
    ProcVector[X_PolyArc] = PanoramiXPolyArc;
    ProcVector[X_FillPoly] = PanoramiXFillPoly;
    ProcVector[X_PolyFillRectangle] = PanoramiXPolyFillRectangle;
    ProcVector[X_PolyFillArc] = PanoramiXPolyFillArc;
    ProcVector[X_PutImage] = PanoramiXPutImage;
    ProcVector[X_GetImage] = PanoramiXGetImage;
    ProcVector[X_PolyText8] = PanoramiXPolyText8;
    ProcVector[X_PolyText16] = PanoramiXPolyText16;
    ProcVector[X_ImageText8] = PanoramiXImageText8;
    ProcVector[X_ImageText16] = PanoramiXImageText16;
    ProcVector[X_CreateColormap] = PanoramiXCreateColormap;
    ProcVector[X_FreeColormap] = PanoramiXFreeColormap;
    ProcVector[X_CopyColormapAndFree] = PanoramiXCopyColormapAndFree;
    ProcVector[X_InstallColormap] = PanoramiXInstallColormap;
    ProcVector[X_UninstallColormap] = PanoramiXUninstallColormap;
    ProcVector[X_AllocColor] = PanoramiXAllocColor;
    ProcVector[X_AllocNamedColor] = PanoramiXAllocNamedColor;
    ProcVector[X_AllocColorCells] = PanoramiXAllocColorCells;
    ProcVector[X_AllocColorPlanes] = PanoramiXAllocColorPlanes;    
    ProcVector[X_FreeColors] = PanoramiXFreeColors;
    ProcVector[X_StoreColors] = PanoramiXStoreColors;    
    ProcVector[X_StoreNamedColor] = PanoramiXStoreNamedColor;    

#ifdef RENDER
    PanoramiXRenderInit ();
#endif
}

extern Bool CreateConnectionBlock(void);

Bool PanoramiXCreateConnectionBlock(void)
{
    int i, j, length;
    Bool disableBackingStore = FALSE;
    Bool disableSaveUnders = FALSE;
    int old_width, old_height;
    float width_mult, height_mult;
    xWindowRoot *root;
    xVisualType *visual;
    xDepth *depth;
    VisualPtr pVisual;
    ScreenPtr pScreen;

    /*
     *	Do normal CreateConnectionBlock but faking it for only one screen
     */

    if(!PanoramiXNumDepths) {
	ErrorF("PanoramiX error: Incompatible screens. No common visuals\n");
	return FALSE;
    }

    for(i = 1; i < screenInfo.numScreens; i++) {
	pScreen = screenInfo.screens[i];
	if(pScreen->rootDepth != screenInfo.screens[0]->rootDepth) {
	    ErrorF("PanoramiX error: Incompatible screens. Root window depths differ\n");
	    return FALSE;
	}
	if(pScreen->backingStoreSupport != screenInfo.screens[0]->backingStoreSupport)
	     disableBackingStore = TRUE;
	if(pScreen->saveUnderSupport != screenInfo.screens[0]->saveUnderSupport)
	     disableSaveUnders = TRUE;
    }

    if(disableBackingStore || disableSaveUnders) {
    	for(i = 0; i < screenInfo.numScreens; i++) {
	    pScreen = screenInfo.screens[i];
	    if(disableBackingStore)
		pScreen->backingStoreSupport = NotUseful;
	    if(disableSaveUnders)
		pScreen->saveUnderSupport = NotUseful;
	}
    }

    i = screenInfo.numScreens;
    screenInfo.numScreens = 1;
    if (!CreateConnectionBlock()) {
	screenInfo.numScreens = i;
	return FALSE;
    }

    screenInfo.numScreens = i;
    
    root = (xWindowRoot *) (ConnectionInfo + connBlockScreenStart);
    length = connBlockScreenStart + sizeof(xWindowRoot);

    /* overwrite the connection block */
    root->nDepths = PanoramiXNumDepths;

    for (i = 0; i < PanoramiXNumDepths; i++) {
	depth = (xDepth *) (ConnectionInfo + length);
	depth->depth = PanoramiXDepths[i].depth;
	depth->nVisuals = PanoramiXDepths[i].numVids;
	length += sizeof(xDepth);
	visual = (xVisualType *)(ConnectionInfo + length);
	
	for (j = 0; j < depth->nVisuals; j++, visual++) {
	    visual->visualID = PanoramiXDepths[i].vids[j];

	    for (pVisual = PanoramiXVisuals;
		 pVisual->vid != visual->visualID;
		 pVisual++)
	         ;

	    visual->class = pVisual->class;
	    visual->bitsPerRGB = pVisual->bitsPerRGBValue;
	    visual->colormapEntries = pVisual->ColormapEntries;
	    visual->redMask = pVisual->redMask;
	    visual->greenMask = pVisual->greenMask;
	    visual->blueMask = pVisual->blueMask;
	}

	length += (depth->nVisuals * sizeof(xVisualType));
    }

    connSetupPrefix.length = length >> 2;

    xfree(PanoramiXVisuals);
    for (i = 0; i < PanoramiXNumDepths; i++)
	xfree(PanoramiXDepths[i].vids);
    xfree(PanoramiXDepths);

    /*
     *  OK, change some dimensions so it looks as if it were one big screen
     */
    
    old_width = root->pixWidth;
    old_height = root->pixHeight;

    root->pixWidth = PanoramiXPixWidth;
    root->pixHeight = PanoramiXPixHeight;
    width_mult = (1.0 * root->pixWidth) / old_width;
    height_mult = (1.0 * root->pixHeight) / old_height;
    root->mmWidth *= width_mult;
    root->mmHeight *= height_mult;

    while(ConnectionCallbackList) {
	pointer tmp;

	tmp = (pointer)ConnectionCallbackList;
	(*ConnectionCallbackList->func)();
	ConnectionCallbackList = ConnectionCallbackList->next;
	xfree(tmp);
    }

    return TRUE;
}

extern
void PanoramiXConsolidate(void)
{
    int 	i, j, k;
    VisualPtr   pVisual, pVisual2;
    ScreenPtr   pScreen, pScreen2;
    DepthPtr    pDepth, pDepth2;
    PanoramiXRes *root, *defmap, *saver;
    Bool        foundDepth, missingDepth;

    if(!PanoramiXVisualTable)
	PanoramiXVisualTable = xcalloc(256 * MAXSCREENS, sizeof(XID));

    pScreen = screenInfo.screens[0];
    pVisual = pScreen->visuals; 
    pDepth  = pScreen->allowedDepths;

    PanoramiXNumDepths = 0;
    PanoramiXDepths = xcalloc(pScreen->numDepths,sizeof(DepthRec));
    PanoramiXNumVisuals = 0;
    PanoramiXVisuals = xcalloc(pScreen->numVisuals,sizeof(VisualRec));

    for (i = 0; i < pScreen->numDepths; i++, pDepth++) {
        missingDepth = FALSE;
        for (j = 1; j < PanoramiXNumScreens; j++) {
             pScreen2 = screenInfo.screens[j];
             pDepth2 = pScreen2->allowedDepths;

             foundDepth = FALSE;
             for (k = 0; k < pScreen2->numDepths; k++, pDepth2++) {
                 if(pDepth2->depth == pDepth->depth) {
                     foundDepth = TRUE;
                     break;
                 }
             }

             if(!foundDepth) {
                missingDepth = TRUE;
                break;
             }
        }
          
        if(!missingDepth) {
            PanoramiXDepths[PanoramiXNumDepths].depth = pDepth->depth;
            PanoramiXDepths[PanoramiXNumDepths].numVids = 0;
            if(pDepth->numVids)
                PanoramiXDepths[PanoramiXNumDepths].vids = 
                      xalloc(sizeof(VisualID) * pDepth->numVids);      
            else
                PanoramiXDepths[PanoramiXNumDepths].vids = NULL;
            PanoramiXNumDepths++;
        }
    }

    for (i = 0; i < pScreen->numVisuals; i++, pVisual++) {
	PanoramiXVisualTable[pVisual->vid * MAXSCREENS] = pVisual->vid;

	/* check if the visual exists on all screens */
	for (j = 1; j < PanoramiXNumScreens; j++) {
	    pScreen2 = screenInfo.screens[j];

#ifdef GLXPROXY
	    pVisual2 = glxMatchVisual(pScreen, pVisual, pScreen2);
	    if (pVisual2) {
		PanoramiXVisualTable[(pVisual->vid * MAXSCREENS) + j] =
		    pVisual2->vid;
		continue;
	    } else if (glxMatchVisual(pScreen, pVisual, pScreen)) {
		PanoramiXVisualTable[(pVisual->vid * MAXSCREENS) + j] = 0;
		break;
	    }
#endif
	    pVisual2 = pScreen2->visuals;

	    for (k = 0; k < pScreen2->numVisuals; k++, pVisual2++) {
		if ((pVisual->class == pVisual2->class) &&
		    (pVisual->ColormapEntries == pVisual2->ColormapEntries) &&
		    (pVisual->nplanes == pVisual2->nplanes) &&
		    (pVisual->redMask == pVisual2->redMask) &&
		    (pVisual->greenMask == pVisual2->greenMask) &&
		    (pVisual->blueMask == pVisual2->blueMask) &&
		    (pVisual->offsetRed == pVisual2->offsetRed) &&
		    (pVisual->offsetGreen == pVisual2->offsetGreen) &&
		    (pVisual->offsetBlue == pVisual2->offsetBlue))
		{
                /* We merely assign the first visual that matches.  OpenGL
                   will need to get involved at some point if you want
                   match GLX visuals */
			PanoramiXVisualTable[(pVisual->vid * MAXSCREENS) + j] =
					 pVisual2->vid;
			break;
		}
	    }
	}
	
	/* if it doesn't exist on all screens we can't use it */
	for (j = 0; j < PanoramiXNumScreens; j++) {
	    if (!PanoramiXVisualTable[(pVisual->vid * MAXSCREENS) + j]) {
		PanoramiXVisualTable[pVisual->vid * MAXSCREENS] = 0;
		break;
	    }
	}

	/* if it does, make sure it's in the list of supported depths and visuals */
	if(PanoramiXVisualTable[pVisual->vid * MAXSCREENS]) {
	    PanoramiXVisuals[PanoramiXNumVisuals].vid = pVisual->vid;
	    PanoramiXVisuals[PanoramiXNumVisuals].class = pVisual->class;
	    PanoramiXVisuals[PanoramiXNumVisuals].bitsPerRGBValue = pVisual->bitsPerRGBValue;
	    PanoramiXVisuals[PanoramiXNumVisuals].ColormapEntries = pVisual->ColormapEntries;
	    PanoramiXVisuals[PanoramiXNumVisuals].nplanes = pVisual->nplanes;
	    PanoramiXVisuals[PanoramiXNumVisuals].redMask = pVisual->redMask;
	    PanoramiXVisuals[PanoramiXNumVisuals].greenMask = pVisual->greenMask;
	    PanoramiXVisuals[PanoramiXNumVisuals].blueMask = pVisual->blueMask;
	    PanoramiXVisuals[PanoramiXNumVisuals].offsetRed = pVisual->offsetRed;
	    PanoramiXVisuals[PanoramiXNumVisuals].offsetGreen = pVisual->offsetGreen;
	    PanoramiXVisuals[PanoramiXNumVisuals].offsetBlue = pVisual->offsetBlue;
	    PanoramiXNumVisuals++;	

	    for (j = 0; j < PanoramiXNumDepths; j++) {
	        if (PanoramiXDepths[j].depth == pVisual->nplanes) {
		    PanoramiXDepths[j].vids[PanoramiXDepths[j].numVids] = pVisual->vid;
		    PanoramiXDepths[j].numVids++;
		    break;
		}	
	    }   
	}
    } 


    root = (PanoramiXRes *) xalloc(sizeof(PanoramiXRes));
    root->type = XRT_WINDOW;
    defmap = (PanoramiXRes *) xalloc(sizeof(PanoramiXRes));
    defmap->type = XRT_COLORMAP;
    saver = (PanoramiXRes *) xalloc(sizeof(PanoramiXRes));
    saver->type = XRT_WINDOW;


    for (i =  0; i < PanoramiXNumScreens; i++) {
	root->info[i].id = WindowTable[i]->drawable.id;
	root->u.win.class = InputOutput;
        root->u.win.root = TRUE;
        saver->info[i].id = savedScreenInfo[i].wid;
        saver->u.win.class = InputOutput;
        saver->u.win.root = TRUE;
	defmap->info[i].id = (screenInfo.screens[i])->defColormap;
    }

    AddResource(root->info[0].id, XRT_WINDOW, root);
    AddResource(saver->info[0].id, XRT_WINDOW, saver);
    AddResource(defmap->info[0].id, XRT_COLORMAP, defmap);
}


/*
 *	PanoramiXResetProc()
 *		Exit, deallocating as needed.
 */

static void PanoramiXResetProc(ExtensionEntry* extEntry)
{
    int		i;

#ifdef RENDER
    PanoramiXRenderReset ();
#endif
    screenInfo.numScreens = PanoramiXNumScreens;
    for (i = 256; i--; )
	ProcVector[i] = SavedProcVector[i];

    Xfree(panoramiXdataPtr);    
}


int
ProcPanoramiXQueryVersion (ClientPtr client)
{
    /* REQUEST(xPanoramiXQueryVersionReq); */
    xPanoramiXQueryVersionReply		rep;
    register 	int			n;

    REQUEST_SIZE_MATCH (xPanoramiXQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = PANORAMIX_MAJOR_VERSION;
    rep.minorVersion = PANORAMIX_MINOR_VERSION;   
    if (client->swapped) { 
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);     
        swaps(&rep.majorVersion, n);
        swaps(&rep.minorVersion, n);
    }
    WriteToClient(client, sizeof (xPanoramiXQueryVersionReply), (char *)&rep);
    return (client->noClientException);
}

int
ProcPanoramiXGetState(ClientPtr client)
{
	REQUEST(xPanoramiXGetStateReq);
    	WindowPtr			pWin;
	xPanoramiXGetStateReply		rep;
	register int			n;
	
	REQUEST_SIZE_MATCH(xPanoramiXGetStateReq);
	pWin = LookupWindow (stuff->window, client);
	if (!pWin)
	     return BadWindow;
	rep.type = X_Reply;
	rep.length = 0;
	rep.sequenceNumber = client->sequence;
	rep.state = !noPanoramiXExtension;
    	if (client->swapped) {
	    swaps (&rep.sequenceNumber, n);
	    swapl (&rep.length, n);
	    swaps (&rep.state, n);
	}	
	WriteToClient (client, sizeof (xPanoramiXGetStateReply), (char *) &rep);
	return client->noClientException;

}

int 
ProcPanoramiXGetScreenCount(ClientPtr client)
{
	REQUEST(xPanoramiXGetScreenCountReq);
    	WindowPtr			pWin;
	xPanoramiXGetScreenCountReply	rep;
	register int			n;

	REQUEST_SIZE_MATCH(xPanoramiXGetScreenCountReq);
	pWin = LookupWindow (stuff->window, client);
	if (!pWin)
	     return BadWindow;
	rep.type = X_Reply;
	rep.length = 0;
	rep.sequenceNumber = client->sequence;
	rep.ScreenCount = PanoramiXNumScreens;
    	if (client->swapped) {
	    swaps (&rep.sequenceNumber, n);
	    swapl (&rep.length, n);
	    swaps (&rep.ScreenCount, n);
	}	
	WriteToClient (client, sizeof (xPanoramiXGetScreenCountReply), (char *) &rep);
	return client->noClientException;
}

int 
ProcPanoramiXGetScreenSize(ClientPtr client)
{
	REQUEST(xPanoramiXGetScreenSizeReq);
    	WindowPtr			pWin;
	xPanoramiXGetScreenSizeReply	rep;
	register int			n;
	
	REQUEST_SIZE_MATCH(xPanoramiXGetScreenSizeReq);
	pWin = LookupWindow (stuff->window, client);
	if (!pWin)
	     return BadWindow;
	rep.type = X_Reply;
	rep.length = 0;
	rep.sequenceNumber = client->sequence;
		/* screen dimensions */
	rep.width  = panoramiXdataPtr[stuff->screen].width; 
	rep.height = panoramiXdataPtr[stuff->screen].height; 
    	if (client->swapped) {
	    swaps (&rep.sequenceNumber, n);
	    swapl (&rep.length, n);
	    swaps (&rep.width, n);
	    swaps (&rep.height, n);
	}	
	WriteToClient (client, sizeof (xPanoramiXGetScreenSizeReply), (char *) &rep);
	return client->noClientException;
}


int
ProcXineramaIsActive(ClientPtr client)
{
    /* REQUEST(xXineramaIsActiveReq); */
    xXineramaIsActiveReply	rep;

    REQUEST_SIZE_MATCH(xXineramaIsActiveReq);

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
#if 1
    {
	/* The following hack fools clients into thinking that Xinerama
	 * is disabled even though it is not. */
	extern Bool PanoramiXExtensionDisabledHack;
	rep.state = !noPanoramiXExtension && !PanoramiXExtensionDisabledHack;
    }
#else
    rep.state = !noPanoramiXExtension;
#endif
    if (client->swapped) {
	register int n;
	swaps (&rep.sequenceNumber, n);
	swapl (&rep.length, n);
	swapl (&rep.state, n);
    }	
    WriteToClient (client, sizeof (xXineramaIsActiveReply), (char *) &rep);
    return client->noClientException;
}


int
ProcXineramaQueryScreens(ClientPtr client)
{
    /* REQUEST(xXineramaQueryScreensReq); */
    xXineramaQueryScreensReply	rep;

    REQUEST_SIZE_MATCH(xXineramaQueryScreensReq);

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.number = (noPanoramiXExtension) ? 0 : PanoramiXNumScreens;
    rep.length = rep.number * sz_XineramaScreenInfo >> 2;
    if (client->swapped) {
	register int n;
	swaps (&rep.sequenceNumber, n);
	swapl (&rep.length, n);
	swapl (&rep.number, n);
    }	
    WriteToClient (client, sizeof (xXineramaQueryScreensReply), (char *) &rep);

    if(!noPanoramiXExtension) {
	xXineramaScreenInfo scratch;
	int i;

	for(i = 0; i < PanoramiXNumScreens; i++) {
	    scratch.x_org  = panoramiXdataPtr[i].x;
	    scratch.y_org  = panoramiXdataPtr[i].y;
	    scratch.width  = panoramiXdataPtr[i].width;
	    scratch.height = panoramiXdataPtr[i].height;
	
	    if(client->swapped) {
		register int n;
		swaps (&scratch.x_org, n);
		swaps (&scratch.y_org, n);
		swaps (&scratch.width, n);
		swaps (&scratch.height, n);
	    }
	    WriteToClient (client, sz_XineramaScreenInfo, (char *) &scratch);
	}
    }

    return client->noClientException;
}


static int
ProcPanoramiXDispatch (ClientPtr client)
{   REQUEST(xReq);
    switch (stuff->data)
    {
	case X_PanoramiXQueryVersion:
	     return ProcPanoramiXQueryVersion(client);
	case X_PanoramiXGetState:
	     return ProcPanoramiXGetState(client);
	case X_PanoramiXGetScreenCount:
	     return ProcPanoramiXGetScreenCount(client);
	case X_PanoramiXGetScreenSize:
	     return ProcPanoramiXGetScreenSize(client);
	case X_XineramaIsActive:
	     return ProcXineramaIsActive(client);
	case X_XineramaQueryScreens:
	     return ProcXineramaQueryScreens(client);
    }
    return BadRequest;
}


#if X_BYTE_ORDER == X_LITTLE_ENDIAN
#define SHIFT_L(v,s) (v) << (s)
#define SHIFT_R(v,s) (v) >> (s)
#else
#define SHIFT_L(v,s) (v) >> (s)
#define SHIFT_R(v,s) (v) << (s)
#endif

static void
CopyBits(char *dst, int shiftL, char *src, int bytes)
{
   /* Just get it to work.  Worry about speed later */
    int shiftR = 8 - shiftL;

    while(bytes--) {
	*dst |= SHIFT_L(*src, shiftL);
	*(dst + 1) |= SHIFT_R(*src, shiftR);
	dst++; src++;
    }     
}


/* Caution.  This doesn't support 2 and 4 bpp formats.  We expect
   1 bpp and planar data to be already cleared when presented
   to this function */

void
XineramaGetImageData(
    DrawablePtr *pDrawables,
    int left,
    int top,
    int width, 
    int height,
    unsigned int format,
    unsigned long planemask,
    char *data,
    int pitch,
    Bool isRoot
){
    RegionRec SrcRegion, GrabRegion;
    BoxRec SrcBox, *pbox;
    int x, y, w, h, i, j, nbox, size, sizeNeeded, ScratchPitch, inOut, depth;
    DrawablePtr pDraw = pDrawables[0];
    ScreenPtr pScreen = pDraw->pScreen;
    char *ScratchMem = NULL;

    size = 0;

    /* find box in logical screen space */
    SrcBox.x1 = left;
    SrcBox.y1 = top;
    if(!isRoot) {
	SrcBox.x1 += pDraw->x + panoramiXdataPtr[0].x;
	SrcBox.y1 += pDraw->y + panoramiXdataPtr[0].y;
    }
    SrcBox.x2 = SrcBox.x1 + width;
    SrcBox.y2 = SrcBox.y1 + height;
    
    REGION_INIT(pScreen, &SrcRegion, &SrcBox, 1);
    REGION_NULL(pScreen, &GrabRegion);

    depth = (format == XYPixmap) ? 1 : pDraw->depth;

    for(i = 0; i < PanoramiXNumScreens; i++) {
	pDraw = pDrawables[i];

	inOut = RECT_IN_REGION(pScreen,&XineramaScreenRegions[i],&SrcBox);

	if(inOut == rgnIN) {	   
	    (*pDraw->pScreen->GetImage)(pDraw, 
			SrcBox.x1 - pDraw->x - panoramiXdataPtr[i].x,
			SrcBox.y1 - pDraw->y - panoramiXdataPtr[i].y, 
			width, height, format, planemask, data);
	    break;
	} else if (inOut == rgnOUT)
	    continue;

	REGION_INTERSECT(pScreen, &GrabRegion, &SrcRegion, 
					&XineramaScreenRegions[i]);

	nbox = REGION_NUM_RECTS(&GrabRegion);

	if(nbox) {
	    pbox = REGION_RECTS(&GrabRegion);

	    while(nbox--) {
		w = pbox->x2 - pbox->x1;
		h = pbox->y2 - pbox->y1;
		ScratchPitch = PixmapBytePad(w, depth);
		sizeNeeded = ScratchPitch * h;

		if(sizeNeeded > size) {
		    char *tmpdata = ScratchMem;
		    ScratchMem = xrealloc(ScratchMem, sizeNeeded);
		    if(ScratchMem)
			size = sizeNeeded;
		    else {
			ScratchMem = tmpdata;
			break;
		    }	
		}

		x = pbox->x1 - pDraw->x - panoramiXdataPtr[i].x;
		y = pbox->y1 - pDraw->y - panoramiXdataPtr[i].y;

		(*pDraw->pScreen->GetImage)(pDraw, x, y, w, h, 
					format, planemask, ScratchMem);
		
		/* copy the memory over */

		if(depth == 1) {
		   int k, shift, leftover, index, index2;
		
		   x = pbox->x1 - SrcBox.x1;
		   y = pbox->y1 - SrcBox.y1;
		   shift = x & 7;
		   x >>= 3;
		   leftover = w & 7;
		   w >>= 3;

		   /* clean up the edge */
		   if(leftover) {
			int mask = (1 << leftover) - 1;
			for(j = h, k = w; j--; k += ScratchPitch)
			    ScratchMem[k] &= mask;
		   }

		   for(j = 0, index = (pitch * y) + x, index2 = 0; j < h;
		       j++, index += pitch, index2 += ScratchPitch) 
		   {
			if(w) {
			    if(!shift)
				memcpy(data + index, ScratchMem + index2, w);
			    else
				CopyBits(data + index, shift, 
						ScratchMem + index2, w);
			}
	
			if(leftover) {
			    data[index + w] |= 
				SHIFT_L(ScratchMem[index2 + w], shift);
			    if((shift + leftover) > 8)
				data[index + w + 1] |= 
				  SHIFT_R(ScratchMem[index2 + w],(8 - shift));
			}
		    }
		} else {
		    j = BitsPerPixel(depth) >> 3;
		    x = (pbox->x1 - SrcBox.x1) * j;
		    y = pbox->y1 - SrcBox.y1;
		    w *= j;

		    for(j = 0; j < h; j++) {
			memcpy(data + (pitch * (y + j)) + x, 
				ScratchMem + (ScratchPitch * j), w);
		    }
		}
		pbox++;
	    }

	    REGION_SUBTRACT(pScreen, &SrcRegion, &SrcRegion, &GrabRegion);
	    if(!REGION_NOTEMPTY(pScreen, &SrcRegion))
		break;
	}
	
    }

    if(ScratchMem)
	xfree(ScratchMem);

    REGION_UNINIT(pScreen, &SrcRegion);
    REGION_UNINIT(pScreen, &GrabRegion);
}
