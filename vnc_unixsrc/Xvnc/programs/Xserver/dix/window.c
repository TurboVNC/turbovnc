/* $XConsortium: window.c /main/210 1996/10/28 07:24:59 kaleb $ */
/* $XFree86: xc/programs/Xserver/dix/window.c,v 3.6 1997/01/18 06:53:16 dawes Exp $ */
/*

Copyright (c) 1987  X Consortium

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


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,

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

*/

#include "misc.h"
#include "scrnintstr.h"
#include "os.h"
#include "regionstr.h"
#include "validate.h"
#include "windowstr.h"
#include "input.h"
#include "resource.h"
#include "colormapst.h"
#include "cursorstr.h"
#include "dixstruct.h"
#include "gcstruct.h"
#include "servermd.h"
#include "dixevents.h"
#ifdef XAPPGROUP
#include "extensions/Xagsrv.h"
#endif
#ifdef XCSECURITY
#define _SECURITY_SERVER
#include "extensions/security.h"
#endif

extern Bool permitOldBugs;

#if defined(NEED_SCREEN_REGIONS)
#define REGION_PTR(pScreen,pWin) \
    register ScreenPtr pScreen = pWin->drawable.pScreen;
#else
#define REGION_PTR(pScreen,pWin) /* nothing */
#endif

/******
 * Window stuff for server 
 *
 *    CreateRootWindow, CreateWindow, ChangeWindowAttributes,
 *    GetWindowAttributes, DeleteWindow, DestroySubWindows,
 *    HandleSaveSet, ReparentWindow, MapWindow, MapSubWindows,
 *    UnmapWindow, UnmapSubWindows, ConfigureWindow, CirculateWindow,
 *
 ******/

static unsigned char _back_lsb[4] = {0x88, 0x22, 0x44, 0x11};
static unsigned char _back_msb[4] = {0x11, 0x44, 0x22, 0x88};

int screenIsSaved = SCREEN_SAVER_OFF;

ScreenSaverStuffRec savedScreenInfo[MAXSCREENS];

extern WindowPtr *WindowTable;

extern int rand();

static Bool TileScreenSaver(
#if NeedFunctionPrototypes
    int /*i*/,
    int /*kind*/
#endif
);


#define INPUTONLY_LEGAL_MASK (CWWinGravity | CWEventMask | \
			      CWDontPropagate | CWOverrideRedirect | CWCursor )

#define BOXES_OVERLAP(b1, b2) \
      (!( ((b1)->x2 <= (b2)->x1)  || \
	( ((b1)->x1 >= (b2)->x2)) || \
	( ((b1)->y2 <= (b2)->y1)) || \
	( ((b1)->y1 >= (b2)->y2)) ) )

#define RedirectSend(pWin) \
    ((pWin->eventMask|wOtherEventMasks(pWin)) & SubstructureRedirectMask)

#define SubSend(pWin) \
    ((pWin->eventMask|wOtherEventMasks(pWin)) & SubstructureNotifyMask)

#define StrSend(pWin) \
    ((pWin->eventMask|wOtherEventMasks(pWin)) & StructureNotifyMask)

#define SubStrSend(pWin,pParent) (StrSend(pWin) || SubSend(pParent))


int numSaveUndersViewable = 0;
int deltaSaveUndersViewable = 0;

#ifdef DEBUG
/******
 * PrintWindowTree
 *    For debugging only
 ******/

int
PrintChildren(p1, indent)
    WindowPtr p1;
    int indent;
{
    WindowPtr p2;
    int i;

    while (p1)
    {
	p2 = p1->firstChild;
	for (i=0; i<indent; i++) ErrorF( " ");
	ErrorF( "%x\n", p1->drawable.id);
	miPrintRegion(&p1->clipList);
	PrintChildren(p2, indent+4);
	p1 = p1->nextSib;
    }
}

PrintWindowTree()
{
    int i;
    WindowPtr pWin, p1;

    for (i=0; i<screenInfo.numScreens; i++)
    {
	ErrorF( "WINDOW %d\n", i);
	pWin = WindowTable[i];
	miPrintRegion(&pWin->clipList);
	p1 = pWin->firstChild;
	PrintChildren(p1, 4);
    }
}
#endif

int
TraverseTree(pWin, func, data)
    register WindowPtr pWin;
    VisitWindowProcPtr func;
    pointer data;
{
    register int result;
    register WindowPtr pChild;

    if (!(pChild = pWin))
       return(WT_NOMATCH);
    while (1)
    {
	result = (* func)(pChild, data);
	if (result == WT_STOPWALKING)
	    return(WT_STOPWALKING);
	if ((result == WT_WALKCHILDREN) && pChild->firstChild)
	{
	    pChild = pChild->firstChild;
	    continue;
	}
	while (!pChild->nextSib && (pChild != pWin))
	    pChild = pChild->parent;
	if (pChild == pWin)
	    break;
	pChild = pChild->nextSib;
    }
    return(WT_NOMATCH);
}

/*****
 * WalkTree
 *   Walk the window tree, for SCREEN, preforming FUNC(pWin, data) on
 *   each window.  If FUNC returns WT_WALKCHILDREN, traverse the children,
 *   if it returns WT_DONTWALKCHILDREN, dont.  If it returns WT_STOPWALKING
 *   exit WalkTree.  Does depth-first traverse.
 *****/

int
WalkTree(pScreen, func, data)
    ScreenPtr pScreen;
    VisitWindowProcPtr func;
    pointer data;
{
    return(TraverseTree(WindowTable[pScreen->myNum], func, data));
}

/* hack for forcing backing store on all windows */
int	defaultBackingStore = NotUseful;
/* hack to force no backing store */
Bool	disableBackingStore = FALSE;
/* hack to force no save unders */
Bool	disableSaveUnders = FALSE;

static void
#if NeedFunctionPrototypes
SetWindowToDefaults(register WindowPtr pWin)
#else
SetWindowToDefaults(pWin)
    register WindowPtr pWin;
#endif
{
    pWin->prevSib = NullWindow;
    pWin->firstChild = NullWindow;
    pWin->lastChild = NullWindow;

    pWin->valdata = (ValidatePtr)NULL;
    pWin->optional = (WindowOptPtr)NULL;
    pWin->cursorIsNone = TRUE;

    pWin->backingStore = NotUseful;
    pWin->DIXsaveUnder = FALSE;
    pWin->backStorage = (pointer) NULL;

    pWin->mapped = FALSE;	    /* off */
    pWin->realized = FALSE;	/* off */
    pWin->viewable = FALSE;
    pWin->visibility = VisibilityNotViewable;
    pWin->overrideRedirect = FALSE;
    pWin->saveUnder = FALSE;

    pWin->bitGravity = ForgetGravity;
    pWin->winGravity = NorthWestGravity;

    pWin->eventMask = 0;
    pWin->deliverableEvents = 0;
    pWin->dontPropagate = 0;
    pWin->forcedBS = FALSE;
#ifdef NEED_DBE_BUF_BITS
    pWin->srcBuffer = DBE_FRONT_BUFFER;
    pWin->dstBuffer = DBE_FRONT_BUFFER;
#endif
}

static void
#if NeedFunctionPrototypes
MakeRootTile(WindowPtr pWin)
#else
MakeRootTile(pWin)
    WindowPtr pWin;
#endif
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    GCPtr pGC;
    unsigned char back[128];
    int len = BitmapBytePad(4);
    register unsigned char *from, *to;
    register int i, j;

    pWin->background.pixmap = (*pScreen->CreatePixmap)(pScreen, 4, 4,
						    pScreen->rootDepth);

    pWin->backgroundState = BackgroundPixmap;
    pGC = GetScratchGC(pScreen->rootDepth, pScreen);
    if (!pWin->background.pixmap || !pGC)
	FatalError("cound not create root tile");

    {
	CARD32 attributes[2];

	attributes[0] = pScreen->whitePixel;
	attributes[1] = pScreen->blackPixel;

	(void)ChangeGC(pGC, GCForeground | GCBackground, attributes);
    }

   ValidateGC((DrawablePtr)pWin->background.pixmap, pGC);

   from = (screenInfo.bitmapBitOrder == LSBFirst) ? _back_lsb : _back_msb;
   to = back;

   for (i = 4; i > 0; i--, from++)
	for (j = len; j > 0; j--)
	    *to++ = *from;

   (*pGC->ops->PutImage)((DrawablePtr)pWin->background.pixmap, pGC, 1,
		    0, 0, 4, 4, 0, XYBitmap, (char *)back);

   FreeScratchGC(pGC);

}

WindowPtr
AllocateWindow(pScreen)
    ScreenPtr pScreen;
{
    WindowPtr pWin;
    register char *ptr;
    register DevUnion *ppriv;
    register unsigned *sizes;
    register unsigned size;
    register int i;

    pWin = (WindowPtr)xalloc(pScreen->totalWindowSize);
    if (pWin)
    {
	ppriv = (DevUnion *)(pWin + 1);
	pWin->devPrivates = ppriv;
	sizes = pScreen->WindowPrivateSizes;
	ptr = (char *)(ppriv + pScreen->WindowPrivateLen);
	for (i = pScreen->WindowPrivateLen; --i >= 0; ppriv++, sizes++)
	{
	    if ( (size = *sizes) )
	    {
		ppriv->ptr = (pointer)ptr;
		ptr += size;
	    }
	    else
		ppriv->ptr = (pointer)NULL;
	}
    }
    return pWin;
}

/*****
 * CreateRootWindow
 *    Makes a window at initialization time for specified screen
 *****/

Bool
CreateRootWindow(pScreen)
    ScreenPtr	pScreen;
{
    WindowPtr	pWin;
    BoxRec	box;
    PixmapFormatRec *format;

    pWin = AllocateWindow(pScreen);
    if (!pWin)
	return FALSE;

    savedScreenInfo[pScreen->myNum].pWindow = NULL;
    savedScreenInfo[pScreen->myNum].wid = FakeClientID(0);
    savedScreenInfo[pScreen->myNum].ExternalScreenSaver = NULL;
    screenIsSaved = SCREEN_SAVER_OFF;

    WindowTable[pScreen->myNum] = pWin;

    pWin->drawable.pScreen = pScreen;
    pWin->drawable.type = DRAWABLE_WINDOW;

    pWin->drawable.depth = pScreen->rootDepth;
    for (format = screenInfo.formats;
	 format->depth != pScreen->rootDepth;
	 format++)
	;
    pWin->drawable.bitsPerPixel = format->bitsPerPixel;

    pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;

    pWin->parent = NullWindow;
    SetWindowToDefaults(pWin);

    pWin->optional = (WindowOptRec *) xalloc (sizeof (WindowOptRec));

    pWin->optional->dontPropagateMask = 0;
    pWin->optional->otherEventMasks = 0;
    pWin->optional->otherClients = NULL;
    pWin->optional->passiveGrabs = NULL;
    pWin->optional->userProps = NULL;
    pWin->optional->backingBitPlanes = ~0L;
    pWin->optional->backingPixel = 0;
#ifdef SHAPE
    pWin->optional->boundingShape = NULL;
    pWin->optional->clipShape = NULL;
#endif
#ifdef XINPUT
    pWin->optional->inputMasks = NULL;
#endif
    pWin->optional->colormap = pScreen->defColormap;
    pWin->optional->visual = pScreen->rootVisual;

    pWin->nextSib = NullWindow;

    pWin->drawable.id = FakeClientID(0);

    pWin->origin.x = pWin->origin.y = 0;
    pWin->drawable.height = pScreen->height;
    pWin->drawable.width = pScreen->width;
    pWin->drawable.x = pWin->drawable.y = 0;

    box.x1 = 0;
    box.y1 = 0;
    box.x2 = pScreen->width;
    box.y2 = pScreen->height;
    REGION_INIT(pScreen, &pWin->clipList, &box, 1);
    REGION_INIT(pScreen, &pWin->winSize, &box, 1);
    REGION_INIT(pScreen, &pWin->borderSize, &box, 1);
    REGION_INIT(pScreen, &pWin->borderClip, &box, 1);

    pWin->drawable.class = InputOutput;
    pWin->optional->visual = pScreen->rootVisual;

    pWin->backgroundState = BackgroundPixel;
    pWin->background.pixel = pScreen->whitePixel;

    pWin->borderIsPixel = TRUE;
    pWin->border.pixel = pScreen->blackPixel;
    pWin->borderWidth = 0;

    if (!AddResource(pWin->drawable.id, RT_WINDOW, (pointer)pWin))
	return FALSE;

    if (disableBackingStore)
	pScreen->backingStoreSupport = NotUseful;

#ifdef DO_SAVE_UNDERS
    if ((pScreen->backingStoreSupport != NotUseful) &&
	(pScreen->saveUnderSupport == NotUseful))
    {
	/*
	 * If the screen has backing-store but no save-unders, let the
	 * clients know we can support save-unders using backing-store.
	 */
	pScreen->saveUnderSupport = USE_DIX_SAVE_UNDERS;
    }
#endif /* DO_SAVE_UNDERS */
		
    if (disableSaveUnders)
	pScreen->saveUnderSupport = NotUseful;

    return TRUE;
}

void
InitRootWindow(pWin)
    WindowPtr pWin;
{
    ScreenPtr pScreen;

    pScreen = pWin->drawable.pScreen;
    if (!(*pScreen->CreateWindow)(pWin))
	return; /* XXX */
    (*pScreen->PositionWindow)(pWin, 0, 0);

    pWin->cursorIsNone = FALSE;
    pWin->optional->cursor = rootCursor;
    rootCursor->refcnt++;
    MakeRootTile(pWin);
    pWin->backingStore = defaultBackingStore;
    pWin->forcedBS = (defaultBackingStore != NotUseful);
    /* We SHOULD check for an error value here XXX */
    (*pScreen->ChangeWindowAttributes)(pWin,
		       CWBackPixmap|CWBorderPixel|CWCursor|CWBackingStore);

    MapWindow(pWin, serverClient);
}

/* Set the region to the intersection of the rectangle and the
 * window's winSize.  The window is typically the parent of the
 * window from which the region came.
 */

void
ClippedRegionFromBox(pWin, Rgn, x, y, w, h)
    register WindowPtr pWin;
    RegionPtr Rgn;
    register int x, y;
    int w, h;
{
    REGION_PTR(pScreen, pWin)
    BoxRec box;

    box = *(REGION_EXTENTS(pScreen, &pWin->winSize));
    /* we do these calculations to avoid overflows */
    if (x > box.x1)
	box.x1 = x;
    if (y > box.y1)
	box.y1 = y;
    x += w;
    if (x < box.x2)
	box.x2 = x;
    y += h;
    if (y < box.y2)
	box.y2 = y;
    if (box.x1 > box.x2)
	box.x2 = box.x1;
    if (box.y1 > box.y2)
	box.y2 = box.y1;
    REGION_RESET(pScreen, Rgn, &box);
    REGION_INTERSECT(pScreen, Rgn, Rgn, &pWin->winSize);
}

WindowPtr
RealChildHead(pWin)
    register WindowPtr pWin;
{
    if (!pWin->parent &&
	(screenIsSaved == SCREEN_SAVER_ON) &&
	(HasSaverWindow (pWin->drawable.pScreen->myNum)))
	return (pWin->firstChild);
    else
	return (NullWindow);
}

/*****
 * CreateWindow
 *    Makes a window in response to client request 
 *****/

WindowPtr
CreateWindow(wid, pParent, x, y, w, h, bw, class, vmask, vlist,
	     depth, client, visual, error)
    Window wid;
    register WindowPtr pParent;
    int x,y;
    unsigned int w, h, bw;
    unsigned int class;
    register Mask vmask;
    XID *vlist;
    int depth;
    ClientPtr client;
    VisualID visual;
    int *error;
{
    register WindowPtr pWin;
    WindowPtr pHead;
    register ScreenPtr pScreen;
    xEvent event;
    int idepth, ivisual;
    Bool fOK;
    DepthPtr pDepth;
    PixmapFormatRec *format;
    register WindowOptPtr ancwopt;

    if (class == CopyFromParent)
	class = pParent->drawable.class;

    if ((class != InputOutput) && (class != InputOnly))
    {
	*error = BadValue;
	client->errorValue = class;
	return NullWindow;
    }

    if ((class != InputOnly) && (pParent->drawable.class == InputOnly))
    {
	*error = BadMatch;
	return NullWindow;
    }

    if ((class == InputOnly) && ((bw != 0) || (depth != 0)))
    {
	*error = BadMatch;
	return NullWindow;
    }

    pScreen = pParent->drawable.pScreen;

    if ((class == InputOutput) && (depth == 0))
	depth = pParent->drawable.depth;
    ancwopt = pParent->optional;
    if (!ancwopt)
	ancwopt = FindWindowWithOptional(pParent)->optional;
    if (visual == CopyFromParent) {
#ifdef XAPPGROUP
	VisualID ag_visual;

	if (client->appgroup && !pParent->parent &&
	    (ag_visual = XagRootVisual (client)))
	    visual = ag_visual;
	else
#endif
	visual = ancwopt->visual;
    }

    /* Find out if the depth and visual are acceptable for this Screen */
    if ((visual != ancwopt->visual) || (depth != pParent->drawable.depth))
    {
	fOK = FALSE;
	for(idepth = 0; idepth < pScreen->numDepths; idepth++)
	{
	    pDepth = (DepthPtr) &pScreen->allowedDepths[idepth];
	    if ((depth == pDepth->depth) || (depth == 0))
	    {
		for (ivisual = 0; ivisual < pDepth->numVids; ivisual++)
		{
		    if (visual == pDepth->vids[ivisual])
		    {
			fOK = TRUE;
			break;
		    }
		}
	    }
	}
	if (fOK == FALSE)
	{
	    *error = BadMatch;
	    return NullWindow;
	}
    }

    if (((vmask & (CWBorderPixmap | CWBorderPixel)) == 0) &&
	(class != InputOnly) &&
	(depth != pParent->drawable.depth))
    {
	*error = BadMatch;
	return NullWindow;
    }

    if (((vmask & CWColormap) == 0) &&
	(class != InputOnly) &&
	((visual != ancwopt->visual) || (ancwopt->colormap == None)))
    {
	*error = BadMatch;
	return NullWindow;
    }

    pWin = AllocateWindow(pScreen);
    if (!pWin)
    {
	*error = BadAlloc;
	return NullWindow;
    }
    pWin->drawable = pParent->drawable;
    pWin->drawable.depth = depth;
    if (depth == pParent->drawable.depth)
	pWin->drawable.bitsPerPixel = pParent->drawable.bitsPerPixel;
    else
    {
	for (format = screenInfo.formats; format->depth != depth; format++)
	    ;
	pWin->drawable.bitsPerPixel = format->bitsPerPixel;
    }
    if (class == InputOnly)
	pWin->drawable.type = (short) UNDRAWABLE_WINDOW;
    pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;

    pWin->drawable.id = wid;
    pWin->drawable.class = class;

    pWin->parent = pParent;
    SetWindowToDefaults(pWin);

    if (visual != ancwopt->visual)
    {
	if (!MakeWindowOptional (pWin))
	{
	    xfree (pWin);
	    *error = BadAlloc;
	    return NullWindow;
	}
	pWin->optional->visual = visual;
	pWin->optional->colormap = None;
    }

    pWin->borderWidth = bw;
#ifdef XCSECURITY
    /*  can't let untrusted clients have background None windows;
     *  they make it too easy to steal window contents
     */
    if (client->trustLevel != XSecurityClientTrusted)
    {
	pWin->backgroundState = BackgroundPixel;
	pWin->background.pixel = 0;
    }
    else
#endif
    pWin->backgroundState = None;

    pWin->borderIsPixel = pParent->borderIsPixel;
    pWin->border = pParent->border;
    if (pWin->borderIsPixel == FALSE)
	pWin->border.pixmap->refcnt++;
		
    pWin->origin.x = x + (int)bw;
    pWin->origin.y = y + (int)bw;
    pWin->drawable.width = w;
    pWin->drawable.height = h;
    pWin->drawable.x = pParent->drawable.x + x + (int)bw;
    pWin->drawable.y = pParent->drawable.y + y + (int)bw;

	/* set up clip list correctly for unobscured WindowPtr */
    REGION_INIT(pScreen, &pWin->clipList, NullBox, 1);
    REGION_INIT(pScreen, &pWin->borderClip, NullBox, 1);
    REGION_INIT(pScreen, &pWin->winSize, NullBox, 1);
    REGION_INIT(pScreen, &pWin->borderSize, NullBox, 1);

    pHead = RealChildHead(pParent);
    if (pHead)
    {
	pWin->nextSib = pHead->nextSib;
	if (pHead->nextSib)
	    pHead->nextSib->prevSib = pWin;
	else
	    pParent->lastChild = pWin;
	pHead->nextSib = pWin;
	pWin->prevSib = pHead;
    }
    else
    {
	pWin->nextSib = pParent->firstChild;
	if (pParent->firstChild)
	    pParent->firstChild->prevSib = pWin;
	else
	    pParent->lastChild = pWin;
	pParent->firstChild = pWin;
    }

    SetWinSize (pWin);
    SetBorderSize (pWin);

    /* We SHOULD check for an error value here XXX */
    if (!(*pScreen->CreateWindow)(pWin))
    {
	*error = BadAlloc;
	DeleteWindow(pWin, None);
	return NullWindow;
    }
    /* We SHOULD check for an error value here XXX */
    (*pScreen->PositionWindow)(pWin, pWin->drawable.x, pWin->drawable.y);

    if (!(vmask & CWEventMask))
	RecalculateDeliverableEvents(pWin);

    if (vmask)
	*error = ChangeWindowAttributes(pWin, vmask, vlist, wClient (pWin));
    else
	*error = Success;

    if (*error != Success)
    {
	DeleteWindow(pWin, None);
	return NullWindow;
    }
    if (!(vmask & CWBackingStore) && (defaultBackingStore != NotUseful))
    {
	XID value = defaultBackingStore;
	(void)ChangeWindowAttributes(pWin, CWBackingStore, &value, wClient (pWin));
	pWin->forcedBS = TRUE;
    }

    if (SubSend(pParent))
    {
	event.u.u.type = CreateNotify;
	event.u.createNotify.window = wid;
	event.u.createNotify.parent = pParent->drawable.id;
	event.u.createNotify.x = x;
	event.u.createNotify.y = y;
	event.u.createNotify.width = w;
	event.u.createNotify.height = h;
	event.u.createNotify.borderWidth = bw;
	event.u.createNotify.override = pWin->overrideRedirect;
	DeliverEvents(pParent, &event, 1, NullWindow);		
    }

    return pWin;
}

static void
#if NeedFunctionPrototypes
FreeWindowResources(register WindowPtr pWin)
#else
FreeWindowResources(pWin)
    register WindowPtr pWin;
#endif
{
    register ScreenPtr pScreen = pWin->drawable.pScreen;

    DeleteWindowFromAnySaveSet(pWin);
    DeleteWindowFromAnySelections(pWin);
    DeleteWindowFromAnyEvents(pWin, TRUE);
    REGION_UNINIT(pScreen, &pWin->clipList);
    REGION_UNINIT(pScreen, &pWin->winSize);
    REGION_UNINIT(pScreen, &pWin->borderClip);
    REGION_UNINIT(pScreen, &pWin->borderSize);
#ifdef SHAPE
    if (wBoundingShape (pWin))
	REGION_DESTROY(pScreen, wBoundingShape (pWin));
    if (wClipShape (pWin))
	REGION_DESTROY(pScreen, wClipShape (pWin));
#endif
    if (pWin->borderIsPixel == FALSE)
	(*pScreen->DestroyPixmap)(pWin->border.pixmap);
    if (pWin->backgroundState == BackgroundPixmap)
	(*pScreen->DestroyPixmap)(pWin->background.pixmap);

    DeleteAllWindowProperties(pWin);
    /* We SHOULD check for an error value here XXX */
    (*pScreen->DestroyWindow)(pWin);
    DisposeWindowOptional (pWin);
}

static void
#if NeedFunctionPrototypes
CrushTree(WindowPtr pWin)
#else
CrushTree(pWin)
    WindowPtr pWin;
#endif
{
    register WindowPtr pChild, pSib, pParent;
    UnrealizeWindowProcPtr UnrealizeWindow;
    xEvent event;

    if (!(pChild = pWin->firstChild))
	return;
    UnrealizeWindow = pWin->drawable.pScreen->UnrealizeWindow;
    while (1)
    {
	if (pChild->firstChild)
	{
	    pChild = pChild->firstChild;
	    continue;
	}
	while (1)
	{
	    pParent = pChild->parent;
	    if (SubStrSend(pChild, pParent))
	    {
		event.u.u.type = DestroyNotify;
		event.u.destroyNotify.window = pChild->drawable.id;
		DeliverEvents(pChild, &event, 1, NullWindow);		
	    }
	    FreeResource(pChild->drawable.id, RT_WINDOW);
	    pSib = pChild->nextSib;
#ifdef DO_SAVE_UNDERS
	    if (pChild->saveUnder && pChild->viewable)
		deltaSaveUndersViewable--;
#endif
	    pChild->viewable = FALSE;
	    if (pChild->realized)
	    {
		pChild->realized = FALSE;
		(*UnrealizeWindow)(pChild);
	    }
	    FreeWindowResources(pChild);
	    xfree(pChild);
	    if ( (pChild = pSib) )
		break;
	    pChild = pParent;
	    pChild->firstChild = NullWindow;
	    pChild->lastChild = NullWindow;
	    if (pChild == pWin)
		return;
	}
    }
}
	
/*****
 *  DeleteWindow
 *	 Deletes child of window then window itself
 *	 If wid is None, don't send any events
 *****/

/*ARGSUSED*/
int
DeleteWindow(value, wid)
    pointer value;
    XID wid;
 {
    register WindowPtr pParent;
    register WindowPtr pWin = (WindowPtr)value;
    xEvent event;

    UnmapWindow(pWin, FALSE);

    CrushTree(pWin);

    pParent = pWin->parent;
    if (wid && pParent && SubStrSend(pWin, pParent))
    {
	event.u.u.type = DestroyNotify;
	event.u.destroyNotify.window = pWin->drawable.id;
	DeliverEvents(pWin, &event, 1, NullWindow);		
    }

    FreeWindowResources(pWin);
    if (pParent)
    {
	if (pParent->firstChild == pWin)
	    pParent->firstChild = pWin->nextSib;
	if (pParent->lastChild == pWin)
	    pParent->lastChild = pWin->prevSib;
	if (pWin->nextSib)
	    pWin->nextSib->prevSib = pWin->prevSib;
	if (pWin->prevSib)
	    pWin->prevSib->nextSib = pWin->nextSib;
    }
    xfree(pWin);
    return Success;
}

/*ARGSUSED*/
void
DestroySubwindows(pWin, client)
    register WindowPtr pWin;
    ClientPtr client;
{
    /* XXX
     * The protocol is quite clear that each window should be
     * destroyed in turn, however, unmapping all of the first
     * eliminates most of the calls to ValidateTree.  So,
     * this implementation is incorrect in that all of the
     * UnmapNotifies occur before all of the DestroyNotifies.
     * If you care, simply delete the call to UnmapSubwindows.
     */
    UnmapSubwindows(pWin);
    while (pWin->lastChild)
	FreeResource(pWin->lastChild->drawable.id, RT_NONE);
}

#define DeviceEventMasks (KeyPressMask | KeyReleaseMask | ButtonPressMask | \
    ButtonReleaseMask | PointerMotionMask)

/*****
 *  ChangeWindowAttributes
 *   
 *  The value-mask specifies which attributes are to be changed; the
 *  value-list contains one value for each one bit in the mask, from least
 *  to most significant bit in the mask.  
 *****/
 
int
ChangeWindowAttributes(pWin, vmask, vlist, client)
    register WindowPtr pWin;
    Mask vmask;
    XID *vlist;
    ClientPtr client;
{
    register Mask index2;
    register XID *pVlist;
    PixmapPtr pPixmap;
    Pixmap pixID;
    CursorPtr pCursor, pOldCursor;
    Cursor cursorID;
    WindowPtr pChild;
    Colormap cmap;
    ColormapPtr	pCmap;
    xEvent xE;
    int result;
    register ScreenPtr pScreen;
    Mask vmaskCopy = 0;
    register Mask tmask;
    unsigned int val;
    int error;
    Bool checkOptional = FALSE;
    Bool borderRelative = FALSE;
    WindowPtr pLayerWin;

    if ((pWin->drawable.class == InputOnly) && (vmask & (~INPUTONLY_LEGAL_MASK)))
	return BadMatch;

    error = Success;
    pScreen = pWin->drawable.pScreen;
    pVlist = vlist;
    tmask = vmask;
    while (tmask)
    {
	index2 = (Mask) lowbit (tmask);
	tmask &= ~index2;
	switch (index2)
	{
	  case CWBackPixmap:
	    pixID = (Pixmap )*pVlist;
	    pVlist++;
	    if (pWin->backgroundState == ParentRelative)
		borderRelative = TRUE;
	    if (pixID == None)
	    {
#ifdef XCSECURITY
		/*  can't let untrusted clients have background None windows */
		if (client->trustLevel == XSecurityClientTrusted)
		{
#endif
		if (pWin->backgroundState == BackgroundPixmap)
		    (*pScreen->DestroyPixmap)(pWin->background.pixmap);
		if (!pWin->parent)
		    MakeRootTile(pWin);
		else
		    pWin->backgroundState = None;
#ifdef XCSECURITY
		}
		else
		{ /* didn't change the background to None, so don't tell ddx */
		    index2 = 0; 
		}
#endif
	    }
	    else if (pixID == ParentRelative)
	    {
		if (pWin->parent &&
		    pWin->drawable.depth != pWin->parent->drawable.depth)
		{
		    error = BadMatch;
		    goto PatchUp;
		}
		if (pWin->backgroundState == BackgroundPixmap)
		    (*pScreen->DestroyPixmap)(pWin->background.pixmap);
		if (!pWin->parent)
		    MakeRootTile(pWin);
		else
		    pWin->backgroundState = ParentRelative;
		borderRelative = TRUE;
		/* Note that the parent's backgroundTile's refcnt is NOT
		 * incremented. */
	    }
	    else
	    {	
		pPixmap = (PixmapPtr)SecurityLookupIDByType(client, pixID,
						RT_PIXMAP, SecurityReadAccess);
		if (pPixmap != (PixmapPtr) NULL)
		{
		    if	((pPixmap->drawable.depth != pWin->drawable.depth) ||
			 (pPixmap->drawable.pScreen != pScreen))
		    {
			error = BadMatch;
			goto PatchUp;
		    }
		    if (pWin->backgroundState == BackgroundPixmap)
			(*pScreen->DestroyPixmap)(pWin->background.pixmap);
		    pWin->backgroundState = BackgroundPixmap;
		    pWin->background.pixmap = pPixmap;
		    pPixmap->refcnt++;
		}
		else
		{
		    error = BadPixmap;
		    client->errorValue = pixID;
		    goto PatchUp;
		}
	    }
	    break;
	  case CWBackPixel:
	    if (pWin->backgroundState == ParentRelative)
		borderRelative = TRUE;
	    if (pWin->backgroundState == BackgroundPixmap)
		(*pScreen->DestroyPixmap)(pWin->background.pixmap);
	    pWin->backgroundState = BackgroundPixel;
	    pWin->background.pixel = (CARD32 ) *pVlist;
		   /* background pixel overrides background pixmap,
		      so don't let the ddx layer see both bits */
	    vmaskCopy &= ~CWBackPixmap;
	    pVlist++;
	    break;
	  case CWBorderPixmap:
	    pixID = (Pixmap ) *pVlist;
	    pVlist++;
	    if (pixID == CopyFromParent)
	    {
		if (!pWin->parent ||
		    (pWin->drawable.depth != pWin->parent->drawable.depth))
		{
		    error = BadMatch;
		    goto PatchUp;
		}
		if (pWin->borderIsPixel == FALSE)
		    (*pScreen->DestroyPixmap)(pWin->border.pixmap);
		pWin->border = pWin->parent->border;
		if ((pWin->borderIsPixel = pWin->parent->borderIsPixel) == TRUE)
		{
		    index2 = CWBorderPixel;
		}
		else
		{
		    pWin->parent->border.pixmap->refcnt++;
		}
	    }
	    else
	    {	
		pPixmap = (PixmapPtr)SecurityLookupIDByType(client, pixID,
					RT_PIXMAP, SecurityReadAccess);
		if (pPixmap)
		{
		    if	((pPixmap->drawable.depth != pWin->drawable.depth) ||
			 (pPixmap->drawable.pScreen != pScreen))
		    {
			error = BadMatch;
			goto PatchUp;
		    }
		    if (pWin->borderIsPixel == FALSE)
			(*pScreen->DestroyPixmap)(pWin->border.pixmap);
		    pWin->borderIsPixel = FALSE;
		    pWin->border.pixmap = pPixmap;
		    pPixmap->refcnt++;
		}
		else
		{
		    error = BadPixmap;
		    client->errorValue = pixID;
		    goto PatchUp;
		}
	    }
	    break;
	  case CWBorderPixel:
	    if (pWin->borderIsPixel == FALSE)
		(*pScreen->DestroyPixmap)(pWin->border.pixmap);
	    pWin->borderIsPixel = TRUE;
	    pWin->border.pixel = (CARD32) *pVlist;
		    /* border pixel overrides border pixmap,
		       so don't let the ddx layer see both bits */
	    vmaskCopy &= ~CWBorderPixmap;
	    pVlist++;
	    break;
	  case CWBitGravity:
	    val = (CARD8 )*pVlist;
	    pVlist++;
	    if (val > StaticGravity)
	    {
		error = BadValue;
		client->errorValue = val;
		goto PatchUp;
	    }
	    pWin->bitGravity = val;
	    break;
	  case CWWinGravity:
	    val = (CARD8 )*pVlist;
	    pVlist++;
	    if (val > StaticGravity)
	    {
		error = BadValue;
		client->errorValue = val;
		goto PatchUp;
	    }
	    pWin->winGravity = val;
	    break;
	  case CWBackingStore:
	    val = (CARD8 )*pVlist;
	    pVlist++;
	    if ((val != NotUseful) && (val != WhenMapped) && (val != Always))
	    {
		error = BadValue;
		client->errorValue = val;
		goto PatchUp;
	    }
	    pWin->backingStore = val;
	    pWin->forcedBS = FALSE;
	    break;
	  case CWBackingPlanes:
	    if (pWin->optional || ((CARD32)*pVlist != (CARD32)~0L)) {
		if (!pWin->optional && !MakeWindowOptional (pWin))
		{
		    error = BadAlloc;
		    goto PatchUp;
		}
		pWin->optional->backingBitPlanes = (CARD32) *pVlist;
		if ((CARD32)*pVlist == (CARD32)~0L)
		    checkOptional = TRUE;
	    }
	    pVlist++;
	    break;
	  case CWBackingPixel:
	    if (pWin->optional || (CARD32) *pVlist) {
		if (!pWin->optional && !MakeWindowOptional (pWin))
		{
		    error = BadAlloc;
		    goto PatchUp;
		}
		pWin->optional->backingPixel = (CARD32) *pVlist;
		if (!*pVlist)
		    checkOptional = TRUE;
	    }
	    pVlist++;
	    break;
	  case CWSaveUnder:
	    val = (BOOL) *pVlist;
	    pVlist++;
	    if ((val != xTrue) && (val != xFalse))
	    {
		error = BadValue;
		client->errorValue = val;
		goto PatchUp;
	    }
#ifdef DO_SAVE_UNDERS
	    if (pWin->parent && (pWin->saveUnder != val) && (pWin->viewable) &&
		DO_SAVE_UNDERS(pWin))
	    {
		/*
		 * Re-check all siblings and inferiors for obscurity or
		 * exposition (hee hee).
		 */
		if (pWin->saveUnder)
		    deltaSaveUndersViewable--;
		else
		    deltaSaveUndersViewable++;
		pWin->saveUnder = val;

		if (pWin->firstChild)
		{
                    pLayerWin = (*pScreen->GetLayerWindow)(pWin);
                   if ((*pScreen->ChangeSaveUnder)(pLayerWin->parent, pWin->nextSib))
                       (*pScreen->PostChangeSaveUnder)(pLayerWin->parent,
                                                       pWin->nextSib);
               }
               else
               {
                   if ((*pScreen->ChangeSaveUnder)(pWin, pWin->nextSib))
                       (*pScreen->PostChangeSaveUnder)(pWin,
                                                       pWin->nextSib);
               }                                   
	    }
	    else
	    {
		/*  If we're changing the saveUnder attribute of the root 
		 *  window, all we do is set pWin->saveUnder so that
		 *  GetWindowAttributes returns the right value.  We don't
		 *  do the "normal" save-under processing (as above).
		 *  Hope that doesn't cause any problems.
		 */
		pWin->saveUnder = val;
	    }
#else
	    pWin->saveUnder = val;
#endif /* DO_SAVE_UNDERS */
	    break;
	  case CWEventMask:
	    result = EventSelectForWindow(pWin, client, (Mask )*pVlist);
	    if (result)
	    {
		error = result;
		goto PatchUp;
	    }
	    pVlist++;
	    break;
	  case CWDontPropagate:
	    result = EventSuppressForWindow(pWin, client, (Mask )*pVlist,
					    &checkOptional);
	    if (result)
	    {
		error = result;
		goto PatchUp;
	    }
	    pVlist++;
	    break;
	  case CWOverrideRedirect:
	    val = (BOOL ) *pVlist;
	    pVlist++;
	    if ((val != xTrue) && (val != xFalse))
	    {
		error = BadValue;
		client->errorValue = val;
		goto PatchUp;
	    }
	    pWin->overrideRedirect = val;
	    break;
	  case CWColormap:
	    cmap = (Colormap) *pVlist;
	    pVlist++;
	    if (cmap == CopyFromParent)
	    {
#ifdef XAPPGROUP
		Colormap ag_colormap;
		ClientPtr win_owner;

		/*
		 * win_owner == client for CreateWindow, other clients
		 * can ChangeWindowAttributes
		 */
		win_owner = LookupClient (pWin->drawable.id, client);

		if ( win_owner && win_owner->appgroup &&
		    !pWin->parent->parent &&
		    (ag_colormap = XagDefaultColormap (win_owner)))
		    cmap = ag_colormap;
		else
#endif
		if (pWin->parent &&
		    (!pWin->optional ||
		     pWin->optional->visual == wVisual (pWin->parent)))
		{
		    cmap = wColormap (pWin->parent);
		}
		else
		    cmap = None;
	    }
	    if (cmap == None)
	    {
		error = BadMatch;
		goto PatchUp;
	    }
	    pCmap = (ColormapPtr)SecurityLookupIDByType(client, cmap,
					      RT_COLORMAP, SecurityReadAccess);
	    if (!pCmap)
	    {
		error = BadColor;
		client->errorValue = cmap;
		goto PatchUp;
	    }
	    if (pCmap->pVisual->vid != wVisual (pWin) ||
		pCmap->pScreen != pScreen)
	    {
		error = BadMatch;
		goto PatchUp;
	    }
	    if (cmap != wColormap (pWin))
	    {
		if (!pWin->optional)
		{
		    if (!MakeWindowOptional (pWin))
		    {
			error = BadAlloc;
			goto PatchUp;
		    }
		}
		else if (pWin->parent && cmap == wColormap (pWin->parent))
		    checkOptional = TRUE;

		/*
		 * propagate the original colormap to any children
		 * inheriting it
		 */

		for (pChild = pWin->firstChild; pChild; pChild=pChild->nextSib)
		{
		    if (!pChild->optional && !MakeWindowOptional (pChild))
		    {
			error = BadAlloc;
			goto PatchUp;
		    }
		}

		pWin->optional->colormap = cmap;

		/*
		 * check on any children now matching the new colormap
		 */

		for (pChild = pWin->firstChild; pChild; pChild=pChild->nextSib)
		{
		    if (pChild->optional->colormap == cmap)
			CheckWindowOptionalNeed (pChild);
		}

		xE.u.u.type = ColormapNotify;
		xE.u.colormap.window = pWin->drawable.id;
		xE.u.colormap.colormap = cmap;
		xE.u.colormap.new = xTrue;
		xE.u.colormap.state = IsMapInstalled(cmap, pWin);
		DeliverEvents(pWin, &xE, 1, NullWindow);
	    }
	    break;
	  case CWCursor:
	    cursorID = (Cursor ) *pVlist;
	    pVlist++;
	    /*
	     * install the new
	     */
	    if ( cursorID == None)
	    {
		if (pWin == WindowTable[pWin->drawable.pScreen->myNum])
		    pCursor = rootCursor;
		else
		    pCursor = (CursorPtr) None;
	    }
	    else
	    {
		pCursor = (CursorPtr)SecurityLookupIDByType(client, cursorID,
						RT_CURSOR, SecurityReadAccess);
		if (!pCursor)
		{
		    error = BadCursor;
		    client->errorValue = cursorID;
		    goto PatchUp;
		}
	    }

	    if (pCursor != wCursor (pWin))
	    {
		/*
		 * patch up child windows so they don't lose cursors.
		 */

		for (pChild = pWin->firstChild; pChild; pChild=pChild->nextSib)
		{
		    if (!pChild->optional && !pChild->cursorIsNone &&
			!MakeWindowOptional (pChild))
		    {
			error = BadAlloc;
			goto PatchUp;
		    }
		}

		pOldCursor = 0;
		if (pCursor == (CursorPtr) None)
		{
		    pWin->cursorIsNone = TRUE;
		    if (pWin->optional)
		    {
			pOldCursor = pWin->optional->cursor;
			pWin->optional->cursor = (CursorPtr) None;
			checkOptional = TRUE;
		    }
		} else {
		    if (!pWin->optional)
		    {
			if (!MakeWindowOptional (pWin))
			{
			    error = BadAlloc;
			    goto PatchUp;
			}
		    }
		    else if (pWin->parent && pCursor == wCursor (pWin->parent))
			checkOptional = TRUE;
		    pOldCursor = pWin->optional->cursor;
		    pWin->optional->cursor = pCursor;
		    pCursor->refcnt++;
		    pWin->cursorIsNone = FALSE;
		    /*
		     * check on any children now matching the new cursor
		     */

		    for (pChild=pWin->firstChild; pChild; pChild=pChild->nextSib)
		    {
			if (pChild->optional &&
			    (pChild->optional->cursor == pCursor))
			    CheckWindowOptionalNeed (pChild);
		    }
		}

		if (pWin->realized)
		    WindowHasNewCursor( pWin);

		/* Can't free cursor until here - old cursor
		 * is needed in WindowHasNewCursor
		 */
		if (pOldCursor)
		    FreeCursor (pOldCursor, (Cursor)0);
	    }
	    break;
	 default:
	    error = BadValue;
	    client->errorValue = vmask;
	    goto PatchUp;
      }
      vmaskCopy |= index2;
    }
PatchUp:
    if (checkOptional)
	CheckWindowOptionalNeed (pWin);

	/* We SHOULD check for an error value here XXX */
    (*pScreen->ChangeWindowAttributes)(pWin, vmaskCopy);

    /* 
	If the border contents have changed, redraw the border. 
	Note that this has to be done AFTER pScreen->ChangeWindowAttributes
	for the tile to be rotated, and the correct function selected.
    */
    if (((vmaskCopy & (CWBorderPixel | CWBorderPixmap)) || borderRelative)
	&& pWin->viewable && HasBorder (pWin))
    {
	RegionRec exposed;

	REGION_INIT(pScreen, &exposed, NullBox, 0);
	REGION_SUBTRACT(pScreen, &exposed, &pWin->borderClip, &pWin->winSize);
	(*pWin->drawable.pScreen->PaintWindowBorder)(pWin, &exposed, PW_BORDER);
	REGION_UNINIT(pScreen, &exposed);
    }
    return error;
}


/*****
 * GetWindowAttributes
 *    Notice that this is different than ChangeWindowAttributes
 *****/

void
GetWindowAttributes(pWin, client, wa)
    register WindowPtr pWin;
    ClientPtr client;
    xGetWindowAttributesReply *wa;
{
    wa->type = X_Reply;
    wa->bitGravity = pWin->bitGravity;
    wa->winGravity = pWin->winGravity;
    if (pWin->forcedBS && pWin->backingStore != Always)
	wa->backingStore = NotUseful;
    else
	wa->backingStore = pWin->backingStore;
    wa->length = (sizeof(xGetWindowAttributesReply) -
		 sizeof(xGenericReply)) >> 2;
    wa->sequenceNumber = client->sequence;
    wa->backingBitPlanes =  wBackingBitPlanes (pWin);
    wa->backingPixel =  wBackingPixel (pWin);
    wa->saveUnder = (BOOL)pWin->saveUnder;
    wa->override = pWin->overrideRedirect;
    if (!pWin->mapped)
	wa->mapState = IsUnmapped;
    else if (pWin->realized)
	wa->mapState = IsViewable;
    else
	wa->mapState = IsUnviewable;

    wa->colormap =  wColormap (pWin);
    wa->mapInstalled = (wa->colormap == None) ? xFalse
				: IsMapInstalled(wa->colormap, pWin);

    wa->yourEventMask = EventMaskForClient(pWin, client);
    wa->allEventMasks = pWin->eventMask | wOtherEventMasks (pWin);
    wa->doNotPropagateMask = wDontPropagateMask (pWin);
    wa->class = pWin->drawable.class;
    wa->visualID = wVisual (pWin);
}


WindowPtr
MoveWindowInStack(pWin, pNextSib)
    register WindowPtr pWin, pNextSib;
{
    register WindowPtr pParent = pWin->parent;
    WindowPtr pFirstChange = pWin; /* highest window where list changes */

    if (pWin->nextSib != pNextSib)
    {
	if (!pNextSib)	      /* move to bottom */
	{
	    if (pParent->firstChild == pWin)
		pParent->firstChild = pWin->nextSib;
	    /* if (pWin->nextSib) */	 /* is always True: pNextSib == NULL
					  * and pWin->nextSib != pNextSib
					  * therefore pWin->nextSib != NULL */
	    pFirstChange = pWin->nextSib;
	    pWin->nextSib->prevSib = pWin->prevSib;
	    if (pWin->prevSib)
		pWin->prevSib->nextSib = pWin->nextSib;
	    pParent->lastChild->nextSib = pWin;
	    pWin->prevSib = pParent->lastChild;
	    pWin->nextSib = NullWindow;
	    pParent->lastChild = pWin;
	}
	else if (pParent->firstChild == pNextSib) /* move to top */
	{
	    pFirstChange = pWin;
	    if (pParent->lastChild == pWin)
	       pParent->lastChild = pWin->prevSib;
	    if (pWin->nextSib)
		pWin->nextSib->prevSib = pWin->prevSib;
	    if (pWin->prevSib)
		pWin->prevSib->nextSib = pWin->nextSib;
	    pWin->nextSib = pParent->firstChild;
	    pWin->prevSib = (WindowPtr ) NULL;
	    pNextSib->prevSib = pWin;
	    pParent->firstChild = pWin;
	}
	else			/* move in middle of list */
	{
	    WindowPtr pOldNext = pWin->nextSib;

	    pFirstChange = NullWindow;
	    if (pParent->firstChild == pWin)
		pFirstChange = pParent->firstChild = pWin->nextSib;
	    if (pParent->lastChild == pWin) {
	       pFirstChange = pWin;
	       pParent->lastChild = pWin->prevSib;
	    }
	    if (pWin->nextSib)
		pWin->nextSib->prevSib = pWin->prevSib;
	    if (pWin->prevSib)
		pWin->prevSib->nextSib = pWin->nextSib;
	    pWin->nextSib = pNextSib;
	    pWin->prevSib = pNextSib->prevSib;
	    if (pNextSib->prevSib)
		pNextSib->prevSib->nextSib = pWin;
	    pNextSib->prevSib = pWin;
	    if (!pFirstChange) {		     /* do we know it yet? */
		pFirstChange = pParent->firstChild;  /* no, search from top */
		while ((pFirstChange != pWin) && (pFirstChange != pOldNext))
		     pFirstChange = pFirstChange->nextSib;
	    }
	}
    }

    return( pFirstChange );
}

RegionPtr
CreateUnclippedWinSize (pWin)
    register WindowPtr	 pWin;
{
    RegionPtr	pRgn;
    BoxRec	box;

    box.x1 = pWin->drawable.x;
    box.y1 = pWin->drawable.y;
    box.x2 = pWin->drawable.x + (int) pWin->drawable.width;
    box.y2 = pWin->drawable.y + (int) pWin->drawable.height;
    pRgn = REGION_CREATE(pWin->drawable.pScreen, &box, 1);
#ifdef SHAPE
    if (wBoundingShape (pWin) || wClipShape (pWin)) {
	REGION_PTR(pScreen, pWin)

	REGION_TRANSLATE(pScreen, pRgn, - pWin->drawable.x,
			 - pWin->drawable.y);
	if (wBoundingShape (pWin))
	    REGION_INTERSECT(pScreen, pRgn, pRgn, wBoundingShape (pWin));
	if (wClipShape (pWin))
	    REGION_INTERSECT(pScreen, pRgn, pRgn, wClipShape (pWin));
	REGION_TRANSLATE(pScreen, pRgn, pWin->drawable.x, pWin->drawable.y);
    }
#endif
    return pRgn;
}

void
SetWinSize (pWin)
    register WindowPtr pWin;
{
    ClippedRegionFromBox(pWin->parent, &pWin->winSize,
			 pWin->drawable.x, pWin->drawable.y,
			 (int)pWin->drawable.width,
			 (int)pWin->drawable.height);
#ifdef SHAPE
    if (wBoundingShape (pWin) || wClipShape (pWin)) {
	REGION_PTR(pScreen, pWin)

	REGION_TRANSLATE(pScreen, &pWin->winSize, - pWin->drawable.x,
			 - pWin->drawable.y);
	if (wBoundingShape (pWin))
	    REGION_INTERSECT(pScreen, &pWin->winSize, &pWin->winSize,
			     wBoundingShape (pWin));
	if (wClipShape (pWin))
	    REGION_INTERSECT(pScreen, &pWin->winSize, &pWin->winSize,
			     wClipShape (pWin));
	REGION_TRANSLATE(pScreen, &pWin->winSize, pWin->drawable.x,
			 pWin->drawable.y);
    }
#endif
}

void
SetBorderSize (pWin)
    register WindowPtr pWin;
{
    int	bw;

    if (HasBorder (pWin)) {
	bw = wBorderWidth (pWin);
	ClippedRegionFromBox(pWin->parent, &pWin->borderSize,
		pWin->drawable.x - bw, pWin->drawable.y - bw,
		(int)(pWin->drawable.width + (bw<<1)),
		(int)(pWin->drawable.height + (bw<<1)));
#ifdef SHAPE
	if (wBoundingShape (pWin)) {
	    REGION_PTR(pScreen, pWin)

	    REGION_TRANSLATE(pScreen, &pWin->borderSize, - pWin->drawable.x,
			     - pWin->drawable.y);
	    REGION_INTERSECT(pScreen, &pWin->borderSize, &pWin->borderSize,
			     wBoundingShape (pWin));
	    REGION_TRANSLATE(pScreen, &pWin->borderSize, pWin->drawable.x,
			     pWin->drawable.y);
	    REGION_UNION(pScreen, &pWin->borderSize, &pWin->borderSize,
			 &pWin->winSize);
	}
#endif
    } else {
	REGION_COPY(pWin->drawable.pScreen, &pWin->borderSize,
					       &pWin->winSize);
    }
}

void
GravityTranslate (x, y, oldx, oldy, dw, dh, gravity, destx, desty)
    register int x, y;		/* new window position */
    int		oldx, oldy;	/* old window position */
    int		dw, dh;
    unsigned	gravity;
    register int *destx, *desty;	/* position relative to gravity */
{
    switch (gravity) {
    case NorthGravity:
	*destx = x + dw / 2;
	*desty = y;
	break;
    case NorthEastGravity:
	*destx = x + dw;
	*desty = y;
	break;
    case WestGravity:
	*destx = x;
	*desty = y + dh / 2;
	break;
    case CenterGravity:
	*destx = x + dw / 2;
	*desty = y + dh / 2;
	break;
    case EastGravity:
	*destx = x + dw;
	*desty = y + dh / 2;
	break;
    case SouthWestGravity:
	*destx = x;
	*desty = y + dh;
	break;
    case SouthGravity:
	*destx = x + dw / 2;
	*desty = y + dh;
	break;
    case SouthEastGravity:
	*destx = x + dw;
	*desty = y + dh;
	break;
    case StaticGravity:
	*destx = oldx;
	*desty = oldy;
	break;
    default:
	*destx = x;
	*desty = y;
	break;
    }
}

/* XXX need to retile border on each window with ParentRelative origin */
void
ResizeChildrenWinSize(pWin, dx, dy, dw, dh)
    register WindowPtr pWin;
    int dx, dy, dw, dh;
{
    register ScreenPtr pScreen;
    register WindowPtr pSib, pChild;
    Bool resized = (dw || dh);

    pScreen = pWin->drawable.pScreen;

    for (pSib = pWin->firstChild; pSib; pSib = pSib->nextSib)
    {
	if (resized && (pSib->winGravity > NorthWestGravity))
	{
	    int cwsx, cwsy;

	    cwsx = pSib->origin.x;
	    cwsy = pSib->origin.y;
	    GravityTranslate (cwsx, cwsy, cwsx - dx, cwsy - dy, dw, dh,
			pSib->winGravity, &cwsx, &cwsy);
	    if (cwsx != pSib->origin.x || cwsy != pSib->origin.y)
	    {
		xEvent event;

		event.u.u.type = GravityNotify;
		event.u.gravity.window = pSib->drawable.id;
		event.u.gravity.x = cwsx - wBorderWidth (pSib);
		event.u.gravity.y = cwsy - wBorderWidth (pSib);
		DeliverEvents (pSib, &event, 1, NullWindow);
		pSib->origin.x = cwsx;
		pSib->origin.y = cwsy;
	    }
	}
	pSib->drawable.x = pWin->drawable.x + pSib->origin.x;
	pSib->drawable.y = pWin->drawable.y + pSib->origin.y;
	SetWinSize (pSib);
	SetBorderSize (pSib);
	(*pScreen->PositionWindow)(pSib, pSib->drawable.x, pSib->drawable.y);
	if ( (pChild = pSib->firstChild) )
	{
	    while (1)
	    {
		pChild->drawable.x = pChild->parent->drawable.x +
				     pChild->origin.x;
		pChild->drawable.y = pChild->parent->drawable.y +
				     pChild->origin.y;
		SetWinSize (pChild);
		SetBorderSize (pChild);
		(*pScreen->PositionWindow)(pChild,
				    pChild->drawable.x, pChild->drawable.y);
		if (pChild->firstChild)
		{
		    pChild = pChild->firstChild;
		    continue;
		}
		while (!pChild->nextSib && (pChild != pSib))
		    pChild = pChild->parent;
		if (pChild == pSib)
		    break;
		pChild = pChild->nextSib;
	    }
	}
    }
}

#define GET_INT16(m, f) \
	if (m & mask) \
	  { \
	     f = (INT16) *pVlist;\
	    pVlist++; \
	 }
#define GET_CARD16(m, f) \
	if (m & mask) \
	 { \
	    f = (CARD16) *pVlist;\
	    pVlist++;\
	 }

#define GET_CARD8(m, f) \
	if (m & mask) \
	 { \
	    f = (CARD8) *pVlist;\
	    pVlist++;\
	 }

#define ChangeMask ((Mask)(CWX | CWY | CWWidth | CWHeight))

#define IllegalInputOnlyConfigureMask (CWBorderWidth)

/*
 * IsSiblingAboveMe
 *     returns Above if pSib above pMe in stack or Below otherwise 
 */

static int
#if NeedFunctionPrototypes
IsSiblingAboveMe(
    register WindowPtr pMe,
    register WindowPtr pSib)
#else
IsSiblingAboveMe(pMe, pSib)
    register WindowPtr pMe, pSib;
#endif
{
    register WindowPtr pWin;

    pWin = pMe->parent->firstChild;
    while (pWin)
    {
	if (pWin == pSib)
	    return(Above);
	else if (pWin == pMe)
	    return(Below);
	pWin = pWin->nextSib;
    }
    return(Below);
}

static BoxPtr
#if NeedFunctionPrototypes
WindowExtents(
    register WindowPtr pWin,
    register BoxPtr pBox)
#else
WindowExtents(pWin, pBox)
    register WindowPtr pWin;
    register BoxPtr pBox;
#endif
{
    pBox->x1 = pWin->drawable.x - wBorderWidth (pWin);
    pBox->y1 = pWin->drawable.y - wBorderWidth (pWin);
    pBox->x2 = pWin->drawable.x + (int)pWin->drawable.width
	       + wBorderWidth (pWin);
    pBox->y2 = pWin->drawable.y + (int)pWin->drawable.height
	       + wBorderWidth (pWin);
    return(pBox);
}

#ifdef SHAPE
#define IS_SHAPED(pWin)	(wBoundingShape (pWin) != (RegionPtr) NULL)

static RegionPtr
#if NeedFunctionPrototypes
MakeBoundingRegion (
    register WindowPtr	pWin,
    BoxPtr	pBox)
#else
MakeBoundingRegion (pWin, pBox)
    register WindowPtr	pWin;
    BoxPtr	pBox;
#endif
{
    RegionPtr	pRgn;
    REGION_PTR(pScreen, pWin)

    pRgn = REGION_CREATE(pScreen, pBox, 1);
    if (wBoundingShape (pWin)) {
	    REGION_TRANSLATE(pScreen, pRgn, -pWin->origin.x,
						  -pWin->origin.y);
	    REGION_INTERSECT(pScreen, pRgn, pRgn, wBoundingShape (pWin));
	    REGION_TRANSLATE(pScreen, pRgn, pWin->origin.x,
						  pWin->origin.y);
    }
    return pRgn;
}

static Bool
#if NeedFunctionPrototypes
ShapeOverlap (
    WindowPtr	pWin,
    BoxPtr	pWinBox,
    WindowPtr	pSib,
    BoxPtr	pSibBox)
#else
ShapeOverlap (pWin, pWinBox, pSib, pSibBox)
    WindowPtr	pWin, pSib;
    BoxPtr	pWinBox, pSibBox;
#endif
{
    RegionPtr	pWinRgn, pSibRgn;
    register ScreenPtr	pScreen;
    Bool	ret;

    if (!IS_SHAPED(pWin) && !IS_SHAPED(pSib))
	return TRUE;
    pScreen = pWin->drawable.pScreen;
    pWinRgn = MakeBoundingRegion (pWin, pWinBox);
    pSibRgn = MakeBoundingRegion (pSib, pSibBox);
    REGION_INTERSECT(pScreen, pWinRgn, pWinRgn, pSibRgn);
    ret = REGION_NOTEMPTY(pScreen, pWinRgn);
    REGION_DESTROY(pScreen, pWinRgn);
    REGION_DESTROY(pScreen, pSibRgn);
    return ret;
}
#endif

static Bool
#if NeedFunctionPrototypes
AnyWindowOverlapsMe(
    WindowPtr pWin,
    WindowPtr pHead,
    register BoxPtr box)
#else
AnyWindowOverlapsMe(pWin, pHead, box)
    WindowPtr pWin, pHead;
    register BoxPtr box;
#endif
{
    register WindowPtr pSib;
    BoxRec sboxrec;
    register BoxPtr sbox;

    for (pSib = pWin->prevSib; pSib != pHead; pSib = pSib->prevSib)
    {
	if (pSib->mapped)
	{
	    sbox = WindowExtents(pSib, &sboxrec);
	    if (BOXES_OVERLAP(sbox, box)
#ifdef SHAPE
	    && ShapeOverlap (pWin, box, pSib, sbox)
#endif
	    )
		return(TRUE);
	}
    }
    return(FALSE);
}

static Bool
#if NeedFunctionPrototypes
IOverlapAnyWindow(
    WindowPtr pWin,
    register BoxPtr box)
#else
IOverlapAnyWindow(pWin, box)
    WindowPtr pWin;
    register BoxPtr box;
#endif
{
    register WindowPtr pSib;
    BoxRec sboxrec;
    register BoxPtr sbox;

    for (pSib = pWin->nextSib; pSib; pSib = pSib->nextSib)
    {
	if (pSib->mapped)
	{
	    sbox = WindowExtents(pSib, &sboxrec);
	    if (BOXES_OVERLAP(sbox, box)
#ifdef SHAPE
	    && ShapeOverlap (pWin, box, pSib, sbox)
#endif
	    )
		return(TRUE);
	}
    }
    return(FALSE);
}

/*
 *   WhereDoIGoInTheStack() 
 *	  Given pWin and pSib and the relationshipe smode, return
 *	  the window that pWin should go ABOVE.
 *	  If a pSib is specified:
 *	      Above:  pWin is placed just above pSib
 *	      Below:  pWin is placed just below pSib
 *	      TopIf:  if pSib occludes pWin, then pWin is placed
 *		      at the top of the stack
 *	      BottomIf:	 if pWin occludes pSib, then pWin is 
 *			 placed at the bottom of the stack
 *	      Opposite: if pSib occludes pWin, then pWin is placed at the
 *			top of the stack, else if pWin occludes pSib, then
 *			pWin is placed at the bottom of the stack
 *
 *	  If pSib is NULL:
 *	      Above:  pWin is placed at the top of the stack
 *	      Below:  pWin is placed at the bottom of the stack
 *	      TopIf:  if any sibling occludes pWin, then pWin is placed at
 *		      the top of the stack
 *	      BottomIf: if pWin occludes any sibline, then pWin is placed at
 *			the bottom of the stack
 *	      Opposite: if any sibling occludes pWin, then pWin is placed at
 *			the top of the stack, else if pWin occludes any
 *			sibling, then pWin is placed at the bottom of the stack
 *
 */

static WindowPtr
#if NeedFunctionPrototypes
WhereDoIGoInTheStack(
    register WindowPtr pWin,
    register WindowPtr pSib,
    short x,
    short y,
    unsigned short w,
    unsigned short h,
    int smode)
#else
WhereDoIGoInTheStack(pWin, pSib, x, y, w, h, smode)
    register WindowPtr pWin, pSib;
    short x, y;
    unsigned short w, h;
    int smode;
#endif
{
    BoxRec box;
    register ScreenPtr pScreen;
    WindowPtr pHead, pFirst;

    if ((pWin == pWin->parent->firstChild) &&
	(pWin == pWin->parent->lastChild))
	return((WindowPtr ) NULL);
    pHead = RealChildHead(pWin->parent);
    pFirst = pHead ? pHead->nextSib : pWin->parent->firstChild;
    pScreen = pWin->drawable.pScreen;
    box.x1 = x;
    box.y1 = y;
    box.x2 = x + (int)w;
    box.y2 = y + (int)h;
    switch (smode)
    {
      case Above:
	if (pSib)
	   return(pSib);
	else if (pWin == pFirst)
	    return(pWin->nextSib);
	else
	    return(pFirst);
      case Below:
	if (pSib)
	    if (pSib->nextSib != pWin)
		return(pSib->nextSib);
	    else
		return(pWin->nextSib);
	else
	    return NullWindow;
      case TopIf:
	if ((!pWin->mapped || (pSib && !pSib->mapped)) && !permitOldBugs)
	    return(pWin->nextSib);
	else if (pSib)
	{
	    if ((IsSiblingAboveMe(pWin, pSib) == Above) &&
		(RECT_IN_REGION(pScreen, &pSib->borderSize, &box) != rgnOUT))
		return(pFirst);
	    else
		return(pWin->nextSib);
	}
	else if (AnyWindowOverlapsMe(pWin, pHead, &box))
	    return(pFirst);
	else
	    return(pWin->nextSib);
      case BottomIf:
	if ((!pWin->mapped || (pSib && !pSib->mapped)) && !permitOldBugs)
	    return(pWin->nextSib);
	else if (pSib)
	{
	    if ((IsSiblingAboveMe(pWin, pSib) == Below) &&
		(RECT_IN_REGION(pScreen, &pSib->borderSize, &box) != rgnOUT))
		return NullWindow;
	    else
		return(pWin->nextSib);
	}
	else if (IOverlapAnyWindow(pWin, &box))
	    return NullWindow;
	else
	    return(pWin->nextSib);
      case Opposite:
	if ((!pWin->mapped || (pSib && !pSib->mapped)) && !permitOldBugs)
	    return(pWin->nextSib);
	else if (pSib)
	{
	    if (RECT_IN_REGION(pScreen, &pSib->borderSize, &box) != rgnOUT)
	    {
		if (IsSiblingAboveMe(pWin, pSib) == Above)
		    return(pFirst);
		else
		    return NullWindow;
	    }
	    else
		return(pWin->nextSib);
	}
	else if (AnyWindowOverlapsMe(pWin, pHead, &box))
	{
	    /* If I'm occluded, I can't possibly be the first child
	     * if (pWin == pWin->parent->firstChild)
	     *	  return pWin->nextSib;
	     */
	    return(pFirst);
	}
	else if (IOverlapAnyWindow(pWin, &box))
	    return NullWindow;
	else
	    return pWin->nextSib;
      default:
      {
	ErrorF("Internal error in ConfigureWindow, smode == %d\n",smode );
	return pWin->nextSib;
      }
    }
}

static void
#if NeedFunctionPrototypes
ReflectStackChange(
    register WindowPtr pWin,
    register WindowPtr pSib,
    VTKind  kind)
#else
ReflectStackChange(pWin, pSib, kind)
    register WindowPtr pWin, pSib;
    VTKind  kind;
#endif
{
/* Note that pSib might be NULL */

    Bool WasViewable = (Bool)pWin->viewable;
    WindowPtr pParent;
    Bool anyMarked;
    WindowPtr pFirstChange;
#ifdef DO_SAVE_UNDERS
    Bool	dosave = FALSE;
#endif
    WindowPtr  pLayerWin;
    ScreenPtr pScreen = pWin->drawable.pScreen;

    /* if this is a root window, can't be restacked */
    if (!(pParent = pWin->parent))
	return ;

    pFirstChange = MoveWindowInStack(pWin, pSib);

    if (WasViewable)
    {
	anyMarked = (*pScreen->MarkOverlappedWindows)(pWin, pFirstChange,
						      &pLayerWin);
	if (pLayerWin != pWin) pFirstChange = pLayerWin;
#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    dosave = (*pScreen->ChangeSaveUnder)(pLayerWin, pFirstChange);
	}
#endif /* DO_SAVE_UNDERS */
	if (anyMarked)
	{
	    (*pScreen->ValidateTree)(pLayerWin->parent, pFirstChange, kind);
	    (*pScreen->HandleExposures)(pLayerWin->parent);
	}
#ifdef DO_SAVE_UNDERS
	if (dosave)
	    (*pScreen->PostChangeSaveUnder)(pLayerWin, pFirstChange);
#endif /* DO_SAVE_UNDERS */
	if (anyMarked && pWin->drawable.pScreen->PostValidateTree)
	    (*pScreen->PostValidateTree)(pLayerWin->parent, pFirstChange, kind);
    }
    if (pWin->realized)
	WindowsRestructured ();
}

/*****
 * ConfigureWindow
 *****/

int
ConfigureWindow(pWin, mask, vlist, client)
    register WindowPtr pWin;
    register Mask mask;
    XID *vlist;
    ClientPtr client;
{
#define RESTACK_WIN    0
#define MOVE_WIN       1
#define RESIZE_WIN     2
#define REBORDER_WIN   3
    register WindowPtr pSib = NullWindow;
    register WindowPtr pParent = pWin->parent;
    Window sibwid;
    Mask index2, tmask;
    register XID *pVlist;
    short x,   y, beforeX, beforeY;
    unsigned short w = pWin->drawable.width,
		   h = pWin->drawable.height,
		   bw = pWin->borderWidth;
    int action, smode = Above;
#ifdef XAPPGROUP
    ClientPtr win_owner;
    ClientPtr ag_leader = NULL;
#endif
    xEvent event;

    if ((pWin->drawable.class == InputOnly) && (mask & IllegalInputOnlyConfigureMask))
	return(BadMatch);

    if ((mask & CWSibling) && !(mask & CWStackMode))
	return(BadMatch);

    pVlist = vlist;

    if (pParent)
    {
	x = pWin->drawable.x - pParent->drawable.x - (int)bw;
	y = pWin->drawable.y - pParent->drawable.y - (int)bw;
    }
    else
    {
	x = pWin->drawable.x;
	y = pWin->drawable.y;
    }
    beforeX = x;
    beforeY = y;
    action = RESTACK_WIN;	
    if ((mask & (CWX | CWY)) && (!(mask & (CWHeight | CWWidth))))
    {
	GET_INT16(CWX, x);
	GET_INT16(CWY, y);
	action = MOVE_WIN;
    }
	/* or should be resized */
    else if (mask & (CWX |  CWY | CWWidth | CWHeight))
    {
	GET_INT16(CWX, x);
	GET_INT16(CWY, y);
	GET_CARD16(CWWidth, w);
	GET_CARD16 (CWHeight, h);
	if (!w || !h)
	{
	    client->errorValue = 0;
	    return BadValue;
	}
	action = RESIZE_WIN;
    }
    tmask = mask & ~ChangeMask;
    while (tmask)
    {
	index2 = (Mask)lowbit (tmask);
	tmask &= ~index2;
	switch (index2)
	{
	  case CWBorderWidth:
	    GET_CARD16(CWBorderWidth, bw);
	    break;
	  case CWSibling:
	    sibwid = (Window ) *pVlist;
	    pVlist++;
	    pSib = (WindowPtr )SecurityLookupIDByType(client, sibwid,
						RT_WINDOW, SecurityReadAccess);
	    if (!pSib)
	    {
		client->errorValue = sibwid;
		return(BadWindow);
	    }
	    if (pSib->parent != pParent)
		return(BadMatch);
	    if (pSib == pWin)
		return(BadMatch);
	    break;
	  case CWStackMode:
	    GET_CARD8(CWStackMode, smode);
	    if ((smode != TopIf) && (smode != BottomIf) &&
		(smode != Opposite) && (smode != Above) && (smode != Below))
	    {
		client->errorValue = smode;
		return(BadValue);
	    }
	    break;
	  default:
	    client->errorValue = mask;
	    return(BadValue);
	}
    }
	/* root really can't be reconfigured, so just return */
    if (!pParent)
	return Success;

	/* Figure out if the window should be moved.  Doesnt
	   make the changes to the window if event sent */

    if (mask & CWStackMode)
	pSib = WhereDoIGoInTheStack(pWin, pSib, pParent->drawable.x + x,
				    pParent->drawable.y + y,
				    w + (bw << 1), h + (bw << 1), smode);
    else
	pSib = pWin->nextSib;

#ifdef XAPPGROUP
    win_owner = clients[CLIENT_ID(pWin->drawable.id)];
    ag_leader = XagLeader (win_owner);
#endif

    if ((!pWin->overrideRedirect) && 
	(RedirectSend(pParent)
#ifdef XAPPGROUP
	|| (win_owner->appgroup && ag_leader && 
	    XagIsControlledRoot (client, pParent))
#endif
	))
    {
	event.u.u.type = ConfigureRequest;
	event.u.configureRequest.window = pWin->drawable.id;
	if (mask & CWSibling)
	   event.u.configureRequest.sibling = sibwid;
	else
	    event.u.configureRequest.sibling = None;
	if (mask & CWStackMode)
	   event.u.u.detail = smode;
	else
	    event.u.u.detail = Above;
	event.u.configureRequest.x = x;
	event.u.configureRequest.y = y;
	event.u.configureRequest.width = w;
	event.u.configureRequest.height = h;
	event.u.configureRequest.borderWidth = bw;
	event.u.configureRequest.valueMask = mask;
#ifdef XAPPGROUP
	/* make sure if the ag_leader maps the window it goes to the wm */
	if (ag_leader && ag_leader != client && 
	    XagIsControlledRoot (client, pParent)) {
	    event.u.configureRequest.parent = XagId (win_owner);
	    (void) TryClientEvents (ag_leader, &event, 1,
				    NoEventMask, NoEventMask, NullGrab);
	    return Success;
	}
#endif
	event.u.configureRequest.parent = pParent->drawable.id;
	if (MaybeDeliverEventsToClient(pParent, &event, 1,
		SubstructureRedirectMask, client) == 1)
	    return(Success);
    }
    if (action == RESIZE_WIN)
    {
	Bool size_change = (w != pWin->drawable.width)
			|| (h != pWin->drawable.height);
	if (size_change && ((pWin->eventMask|wOtherEventMasks(pWin)) & ResizeRedirectMask))
	{
	    xEvent eventT;
	    eventT.u.u.type = ResizeRequest;
	    eventT.u.resizeRequest.window = pWin->drawable.id;
	    eventT.u.resizeRequest.width = w;
	    eventT.u.resizeRequest.height = h;
	    if (MaybeDeliverEventsToClient(pWin, &eventT, 1,
				       ResizeRedirectMask, client) == 1)
	    {
		/* if event is delivered, leave the actual size alone. */
		w = pWin->drawable.width;
		h = pWin->drawable.height;
		size_change = FALSE;
	    }
	}
	if (!size_change)
	{
	    if (mask & (CWX | CWY))
		action = MOVE_WIN;
	    else if (mask & (CWStackMode | CWBorderWidth))
		action = RESTACK_WIN;
	    else   /* really nothing to do */
		return(Success) ;
	}
    }

    if (action == RESIZE_WIN)
	    /* we've already checked whether there's really a size change */
	    goto ActuallyDoSomething;
    if ((mask & CWX) && (x != beforeX))
	    goto ActuallyDoSomething;
    if ((mask & CWY) && (y != beforeY))
	    goto ActuallyDoSomething;
    if ((mask & CWBorderWidth) && (bw != wBorderWidth (pWin)))
	    goto ActuallyDoSomething;
    if (mask & CWStackMode)
    {
	if (pWin->nextSib != pSib)
	    goto ActuallyDoSomething;
    }
    return(Success);

ActuallyDoSomething:
    if (SubStrSend(pWin, pParent))
    {
	event.u.u.type = ConfigureNotify;
	event.u.configureNotify.window = pWin->drawable.id;
	if (pSib)
	    event.u.configureNotify.aboveSibling = pSib->drawable.id;
	else
	    event.u.configureNotify.aboveSibling = None;
	event.u.configureNotify.x = x;
	event.u.configureNotify.y = y;
	event.u.configureNotify.width = w;
	event.u.configureNotify.height = h;
	event.u.configureNotify.borderWidth = bw;
	event.u.configureNotify.override = pWin->overrideRedirect;
	DeliverEvents(pWin, &event, 1, NullWindow);
    }
    if (mask & CWBorderWidth)
    {
	if (action == RESTACK_WIN)
	{
	    action = MOVE_WIN;
	    pWin->borderWidth = bw;
	}
	else if ((action == MOVE_WIN) &&
		 (beforeX + wBorderWidth (pWin) == x + (int)bw) &&
		 (beforeY + wBorderWidth (pWin) == y + (int)bw))
	{
	    action = REBORDER_WIN;
	    (*pWin->drawable.pScreen->ChangeBorderWidth)(pWin, bw);
	}
	else
	    pWin->borderWidth = bw;
    }
    if (action == MOVE_WIN)
	(*pWin->drawable.pScreen->MoveWindow)(pWin, x, y, pSib,
		   (mask & CWBorderWidth) ? VTOther : VTMove);
    else if (action == RESIZE_WIN)
	(*pWin->drawable.pScreen->ResizeWindow)(pWin, x, y, w, h, pSib);
    else if (mask & CWStackMode)
	ReflectStackChange(pWin, pSib, VTOther);

    if (action != RESTACK_WIN)
	CheckCursorConfinement(pWin);

    return(Success);
#undef RESTACK_WIN
#undef MOVE_WIN
#undef RESIZE_WIN
#undef REBORDER_WIN
}


/******
 *
 * CirculateWindow
 *    For RaiseLowest, raises the lowest mapped child (if any) that is
 *    obscured by another child to the top of the stack.  For LowerHighest,
 *    lowers the highest mapped child (if any) that is obscuring another
 *    child to the bottom of the stack.	 Exposure processing is performed 
 *
 ******/

int
CirculateWindow(pParent, direction, client)
    WindowPtr pParent;
    int direction;
    ClientPtr client;
{
    register WindowPtr pWin, pHead, pFirst;
    xEvent event;
    BoxRec box;

    pHead = RealChildHead(pParent);
    pFirst = pHead ? pHead->nextSib : pParent->firstChild;
    if (direction == RaiseLowest)
    {
	for (pWin = pParent->lastChild;
	     (pWin != pHead) &&
	     !(pWin->mapped &&
	       AnyWindowOverlapsMe(pWin, pHead, WindowExtents(pWin, &box)));
	     pWin = pWin->prevSib) ;
	if (pWin == pHead)
	    return Success;
    }
    else
    {
	for (pWin = pFirst;
	     pWin &&
	     !(pWin->mapped &&
	       IOverlapAnyWindow(pWin, WindowExtents(pWin, &box)));
	     pWin = pWin->nextSib) ;
	if (!pWin)
	    return Success;
    }

    event.u.circulate.window = pWin->drawable.id;
    event.u.circulate.parent = pParent->drawable.id;
    event.u.circulate.event = pParent->drawable.id;
    if (direction == RaiseLowest)
	event.u.circulate.place = PlaceOnTop;
    else
	event.u.circulate.place = PlaceOnBottom;

    if (RedirectSend(pParent))
    {
	event.u.u.type = CirculateRequest;
	if (MaybeDeliverEventsToClient(pParent, &event, 1,
		SubstructureRedirectMask, client) == 1)
	    return(Success);
    }

    event.u.u.type = CirculateNotify;
    DeliverEvents(pWin, &event, 1, NullWindow);
    ReflectStackChange(pWin,
		       (direction == RaiseLowest) ? pFirst : NullWindow,
		       VTStack);

    return(Success);
}

static int
#if NeedFunctionPrototypes
CompareWIDs(
    WindowPtr pWin,
    pointer   value) /* must conform to VisitWindowProcPtr */
#else
CompareWIDs(pWin, value)
    WindowPtr pWin;
    pointer   value; /* must conform to VisitWindowProcPtr */
#endif
{
    Window *wid = (Window *)value;

    if (pWin->drawable.id == *wid)
       return(WT_STOPWALKING);
    else
       return(WT_WALKCHILDREN);
}

/*****
 *  ReparentWindow
 *****/

int
ReparentWindow(pWin, pParent, x, y, client)
    register WindowPtr pWin, pParent;
    int x,y;
    ClientPtr client;
{
    WindowPtr pPrev, pPriorParent;
    Bool WasMapped = (Bool)(pWin->mapped);
    xEvent event;
    int bw = wBorderWidth (pWin);
    register ScreenPtr pScreen;

    pScreen = pWin->drawable.pScreen;
    if (TraverseTree(pWin, CompareWIDs, (pointer)&pParent->drawable.id) == WT_STOPWALKING)
	return(BadMatch);		
    if (!MakeWindowOptional(pWin))
	return(BadAlloc);

    if (WasMapped)
       UnmapWindow(pWin, FALSE);

    event.u.u.type = ReparentNotify;
    event.u.reparent.window = pWin->drawable.id;
    event.u.reparent.parent = pParent->drawable.id;
    event.u.reparent.x = x;
    event.u.reparent.y = y;
    event.u.reparent.override = pWin->overrideRedirect;
    DeliverEvents(pWin, &event, 1, pParent);

    /* take out of sibling chain */

    pPriorParent = pPrev = pWin->parent;
    if (pPrev->firstChild == pWin)
	pPrev->firstChild = pWin->nextSib;
    if (pPrev->lastChild == pWin)
	pPrev->lastChild = pWin->prevSib;

    if (pWin->nextSib)
	pWin->nextSib->prevSib = pWin->prevSib;
    if (pWin->prevSib)
	pWin->prevSib->nextSib = pWin->nextSib;

    /* insert at begining of pParent */
    pWin->parent = pParent;
    pPrev = RealChildHead(pParent);
    if (pPrev)
    {
	pWin->nextSib = pPrev->nextSib;
	if (pPrev->nextSib)
	    pPrev->nextSib->prevSib = pWin;
	else
	    pParent->lastChild = pWin;
	pPrev->nextSib = pWin;
	pWin->prevSib = pPrev;
    }
    else
    {
	pWin->nextSib = pParent->firstChild;
	pWin->prevSib = NullWindow;
	if (pParent->firstChild)
	    pParent->firstChild->prevSib = pWin;
	else
	    pParent->lastChild = pWin;
	pParent->firstChild = pWin;
    }

    pWin->origin.x = x + bw;
    pWin->origin.y = y + bw;
    pWin->drawable.x = x + bw + pParent->drawable.x;
    pWin->drawable.y = y + bw + pParent->drawable.y;

    /* clip to parent */
    SetWinSize (pWin);
    SetBorderSize (pWin);

    if (pScreen->ReparentWindow)
	(*pScreen->ReparentWindow)(pWin, pPriorParent);
    (*pScreen->PositionWindow)(pWin, pWin->drawable.x, pWin->drawable.y);
    ResizeChildrenWinSize(pWin, 0, 0, 0, 0);

    CheckWindowOptionalNeed(pWin);

    if (WasMapped)
	MapWindow(pWin, client);
    RecalculateDeliverableEvents(pWin);
    return(Success);
}

static void
#if NeedFunctionPrototypes
RealizeTree(WindowPtr pWin)
#else
RealizeTree(pWin)
    WindowPtr pWin;
#endif
{
    register WindowPtr pChild;
    RealizeWindowProcPtr Realize;

    Realize = pWin->drawable.pScreen->RealizeWindow;
    pChild = pWin;
    while (1)
    {
	if (pChild->mapped)
	{
	    pChild->realized = TRUE;
#ifdef DO_SAVE_UNDERS
	    if (pChild->saveUnder)
		deltaSaveUndersViewable++;
#endif
	    pChild->viewable = (pChild->drawable.class == InputOutput);
	    (* Realize)(pChild);
	    if (pChild->firstChild)
	    {
		pChild = pChild->firstChild;
		continue;
	    }
	}
	while (!pChild->nextSib && (pChild != pWin))
	    pChild = pChild->parent;
	if (pChild == pWin)
	    return;
	pChild = pChild->nextSib;
    }
}

/*****
 * MapWindow
 *    If some other client has selected SubStructureReDirect on the parent
 *    and override-redirect is xFalse, then a MapRequest event is generated,
 *    but the window remains unmapped.	Otherwise, the window is mapped and a
 *    MapNotify event is generated.
 *****/

int
MapWindow(pWin, client)
    register WindowPtr pWin;
    ClientPtr client;
{
    register ScreenPtr pScreen;

    register WindowPtr pParent;
#ifdef DO_SAVE_UNDERS
    Bool	dosave = FALSE;
#endif
    WindowPtr  pLayerWin;

    if (pWin->mapped)
	return(Success);

#ifdef XCSECURITY
    /*  don't let an untrusted client map a child-of-trusted-window, InputOnly
     *  window; too easy to steal device input
     */
    if ( (client->trustLevel != XSecurityClientTrusted) &&
	 (pWin->drawable.class == InputOnly) &&
	 (wClient(pWin->parent)->trustLevel == XSecurityClientTrusted) )
	 return Success;
#endif	

    pScreen = pWin->drawable.pScreen;
    if ( (pParent = pWin->parent) )
    {
	xEvent event;
	Bool anyMarked;
#ifdef XAPPGROUP
	ClientPtr win_owner = clients[CLIENT_ID(pWin->drawable.id)];
	ClientPtr ag_leader = XagLeader (win_owner);
#endif

	if ((!pWin->overrideRedirect) && 
	    (RedirectSend(pParent)
#ifdef XAPPGROUP
	    || (win_owner->appgroup && ag_leader &&
		XagIsControlledRoot (client, pParent))
#endif
	))
	{
	    event.u.u.type = MapRequest;
	    event.u.mapRequest.window = pWin->drawable.id;
#ifdef XAPPGROUP
	    /* make sure if the ag_leader maps the window it goes to the wm */
	    if (ag_leader && ag_leader != client &&
		XagIsControlledRoot (client, pParent)) {
		event.u.mapRequest.parent = XagId (win_owner);
		(void) TryClientEvents (ag_leader, &event, 1,
					NoEventMask, NoEventMask, NullGrab);
		return Success;
	    }
#endif
	    event.u.mapRequest.parent = pParent->drawable.id;

	    if (MaybeDeliverEventsToClient(pParent, &event, 1,
		SubstructureRedirectMask, client) == 1)
		return(Success);
	}

	pWin->mapped = TRUE;
	if (SubStrSend(pWin, pParent))
	{
	    event.u.u.type = MapNotify;
	    event.u.mapNotify.window = pWin->drawable.id;
	    event.u.mapNotify.override = pWin->overrideRedirect;
	    DeliverEvents(pWin, &event, 1, NullWindow);
	}

	if (!pParent->realized)
	    return(Success);
	RealizeTree(pWin);
	if (pWin->viewable)
	{
	    anyMarked = (*pScreen->MarkOverlappedWindows)(pWin, pWin,
							  &pLayerWin);
#ifdef DO_SAVE_UNDERS
	    if (DO_SAVE_UNDERS(pWin))
	    {
		dosave = (*pScreen->ChangeSaveUnder)(pLayerWin, pWin->nextSib);
	    }
#endif /* DO_SAVE_UNDERS */
	    if (anyMarked)
	    {
		(*pScreen->ValidateTree)(pLayerWin->parent, pLayerWin, VTMap);
		(*pScreen->HandleExposures)(pLayerWin->parent);
	    }
#ifdef DO_SAVE_UNDERS
	    if (dosave)
		(*pScreen->PostChangeSaveUnder)(pLayerWin, pWin->nextSib);
#endif /* DO_SAVE_UNDERS */
	if (anyMarked && pScreen->PostValidateTree)
	    (*pScreen->PostValidateTree)(pLayerWin->parent, pLayerWin, VTMap);
	}
	WindowsRestructured ();
    }
    else
    {
	RegionRec   temp;

	pWin->mapped = TRUE;
	pWin->realized = TRUE;	   /* for roots */
	pWin->viewable = pWin->drawable.class == InputOutput;
	/* We SHOULD check for an error value here XXX */
	(*pScreen->RealizeWindow)(pWin);
	if (pScreen->ClipNotify)
	    (*pScreen->ClipNotify) (pWin, 0, 0);
	if (pScreen->PostValidateTree)
	    (*pScreen->PostValidateTree)(NullWindow, pWin, VTMap);
	REGION_INIT(pScreen, &temp, NullBox, 0);
	REGION_COPY(pScreen, &temp, &pWin->clipList);
	(*pScreen->WindowExposures) (pWin, &temp, NullRegion);
	REGION_UNINIT(pScreen, &temp);
    }

    return(Success);
}


/*****
 * MapSubwindows
 *    Performs a MapWindow all unmapped children of the window, in top
 *    to bottom stacking order.
 *****/

void
MapSubwindows(pParent, client)
    register WindowPtr pParent;
    ClientPtr client;
{
    register WindowPtr	pWin;
    WindowPtr		pFirstMapped = NullWindow;
#ifdef DO_SAVE_UNDERS
    WindowPtr		pFirstSaveUndered = NullWindow;
#endif
    register ScreenPtr	pScreen;
    register Mask	parentRedirect;
    register Mask	parentNotify;
    xEvent		event;
    Bool		anyMarked;
#ifdef DO_SAVE_UNDERS
    Bool	dosave = FALSE;
#endif
    WindowPtr		pLayerWin;

    pScreen = pParent->drawable.pScreen;
    parentRedirect = RedirectSend(pParent);
    parentNotify = SubSend(pParent);
    anyMarked = FALSE;
    for (pWin = pParent->firstChild; pWin; pWin = pWin->nextSib)
    {
	if (!pWin->mapped)
	{
	    if (parentRedirect && !pWin->overrideRedirect)
	    {
		event.u.u.type = MapRequest;
		event.u.mapRequest.window = pWin->drawable.id;
		event.u.mapRequest.parent = pParent->drawable.id;
    
		if (MaybeDeliverEventsToClient(pParent, &event, 1,
		    SubstructureRedirectMask, client) == 1)
		    continue;
	    }
    
	    pWin->mapped = TRUE;
	    if (parentNotify || StrSend(pWin))
	    {
		event.u.u.type = MapNotify;
		event.u.mapNotify.window = pWin->drawable.id;
		event.u.mapNotify.override = pWin->overrideRedirect;
		DeliverEvents(pWin, &event, 1, NullWindow);
	    }
    
	    if (!pFirstMapped)
		pFirstMapped = pWin;
	    if (pParent->realized)
	    {
		RealizeTree(pWin);
		if (pWin->viewable)
		{
		    anyMarked |= (*pScreen->MarkOverlappedWindows)(pWin, pWin,
							(WindowPtr *)NULL);
#ifdef DO_SAVE_UNDERS
		    if (DO_SAVE_UNDERS(pWin))
		    {
			dosave = TRUE;
		    }
#endif /* DO_SAVE_UNDERS */
		}
	    }
	}
    }

    if (pFirstMapped)
    {
	pLayerWin = (*pScreen->GetLayerWindow)(pParent);
	if (pLayerWin->parent != pParent) {
	    anyMarked |= (*pScreen->MarkOverlappedWindows)(pLayerWin,
							   pLayerWin,
							   (WindowPtr *)NULL);
	    pFirstMapped = pLayerWin;
	}
        if (anyMarked)
        {
#ifdef DO_SAVE_UNDERS
	    if (pLayerWin->parent != pParent)
	    {
		if (dosave || (DO_SAVE_UNDERS(pLayerWin)))
		{
		    dosave = (*pScreen->ChangeSaveUnder)(pLayerWin,
							 pLayerWin);
		}
	    }
	    else if (dosave)
	    {
		dosave = FALSE;
		for (pWin = pParent->firstChild; pWin; pWin = pWin->nextSib)
		{
		    if (DO_SAVE_UNDERS(pWin))
		    {
			dosave |= (*pScreen->ChangeSaveUnder)(pWin,
							      pWin->nextSib);
			if (dosave && !pFirstSaveUndered)
			    pFirstSaveUndered = pWin;
		    }
		}
            }
#endif /* DO_SAVE_UNDERS */
	    (*pScreen->ValidateTree)(pLayerWin->parent, pFirstMapped, VTMap);
	    (*pScreen->HandleExposures)(pLayerWin->parent);
	}
#ifdef DO_SAVE_UNDERS
        if (dosave)
	    (*pScreen->PostChangeSaveUnder)(pLayerWin,
					    pFirstSaveUndered->nextSib);
#endif /* DO_SAVE_UNDERS */
        if (anyMarked && pScreen->PostValidateTree)
	    (*pScreen->PostValidateTree)(pLayerWin->parent, pFirstMapped,
					 VTMap);
        WindowsRestructured ();
    }
}

static void
#if NeedFunctionPrototypes
UnrealizeTree(
    WindowPtr pWin,
    Bool fromConfigure)
#else
UnrealizeTree(pWin, fromConfigure)
    WindowPtr pWin;
    Bool fromConfigure;
#endif
{
    register WindowPtr pChild;
    UnrealizeWindowProcPtr Unrealize;
    MarkUnrealizedWindowProcPtr MarkUnrealizedWindow;

    Unrealize = pWin->drawable.pScreen->UnrealizeWindow;
    MarkUnrealizedWindow = pWin->drawable.pScreen->MarkUnrealizedWindow;
    pChild = pWin;
    while (1)
    {
	if (pChild->realized)
	{
	    pChild->realized = FALSE;
	    pChild->visibility = VisibilityNotViewable;
	    (* Unrealize)(pChild);
	    DeleteWindowFromAnyEvents(pChild, FALSE);
	    if (pChild->viewable)
	    {
#ifdef DO_SAVE_UNDERS
		if (pChild->saveUnder)
		    deltaSaveUndersViewable--;
#endif
		pChild->viewable = FALSE;
		if (pChild->backStorage)
		    (*pChild->drawable.pScreen->SaveDoomedAreas)(
					    pChild, &pChild->clipList, 0, 0);
		(* MarkUnrealizedWindow)(pChild, pWin, fromConfigure);
		pChild->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	    }
	    if (pChild->firstChild)
	    {
		pChild = pChild->firstChild;
		continue;
	    }
	}
	while (!pChild->nextSib && (pChild != pWin))
	    pChild = pChild->parent;
	if (pChild == pWin)
	    return;
	pChild = pChild->nextSib;
    }
}

/*****
 * UnmapWindow
 *    If the window is already unmapped, this request has no effect.
 *    Otherwise, the window is unmapped and an UnMapNotify event is
 *    generated.  Cannot unmap a root window.
 *****/

int
UnmapWindow(pWin, fromConfigure)
    register WindowPtr pWin;
    Bool fromConfigure;
{
    register WindowPtr pParent;
    xEvent event;
    Bool wasRealized = (Bool)pWin->realized;
    Bool wasViewable = (Bool)pWin->viewable;
    ScreenPtr pScreen = pWin->drawable.pScreen;
    WindowPtr pLayerWin = pWin;

    if ((!pWin->mapped) || (!(pParent = pWin->parent)))
	return(Success);
    if (SubStrSend(pWin, pParent))
    {
	event.u.u.type = UnmapNotify;
	event.u.unmapNotify.window = pWin->drawable.id;
	event.u.unmapNotify.fromConfigure = fromConfigure;
	DeliverEvents(pWin, &event, 1, NullWindow);
    }
    if (wasViewable && !fromConfigure)
    {
	pWin->valdata = UnmapValData;
	(*pScreen->MarkOverlappedWindows)(pWin, pWin->nextSib, &pLayerWin);
	(*pScreen->MarkWindow)(pLayerWin->parent);
    }
    pWin->mapped = FALSE;
    if (wasRealized)
	UnrealizeTree(pWin, fromConfigure);
    if (wasViewable)
    {
	if (!fromConfigure)
	{
	    (*pScreen->ValidateTree)(pLayerWin->parent, pWin, VTUnmap);
	    (*pScreen->HandleExposures)(pLayerWin->parent);
	}
#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    if ( (*pScreen->ChangeSaveUnder)(pLayerWin, pWin->nextSib) )
	    {
		(*pScreen->PostChangeSaveUnder)(pLayerWin, pWin->nextSib);
	    }
	}
	pWin->DIXsaveUnder = FALSE;
#endif /* DO_SAVE_UNDERS */
	if (!fromConfigure && pScreen->PostValidateTree)
	    (*pScreen->PostValidateTree)(pLayerWin->parent, pWin, VTUnmap);
    }
    if (wasRealized && !fromConfigure)
	WindowsRestructured ();
    return(Success);
}

/*****
 * UnmapSubwindows
 *    Performs an UnmapWindow request with the specified mode on all mapped
 *    children of the window, in bottom to top stacking order.
 *****/

void
UnmapSubwindows(pWin)
    register WindowPtr pWin;
{
    register WindowPtr pChild, pHead;
    xEvent event;
    Bool wasRealized = (Bool)pWin->realized;
    Bool wasViewable = (Bool)pWin->viewable;
    Bool anyMarked = FALSE;
    Mask parentNotify;
    WindowPtr pLayerWin;
    ScreenPtr pScreen = pWin->drawable.pScreen;

    if (!pWin->firstChild)
	return;
    parentNotify = SubSend(pWin);
    pHead = RealChildHead(pWin);

    if (wasViewable)
	pLayerWin = (*pScreen->GetLayerWindow)(pWin);

    for (pChild = pWin->lastChild; pChild != pHead; pChild = pChild->prevSib)
    {
	if (pChild->mapped)
	{
	    if (parentNotify || StrSend(pChild))
	    {
		event.u.u.type = UnmapNotify;
		event.u.unmapNotify.window = pChild->drawable.id;
		event.u.unmapNotify.fromConfigure = xFalse;
		DeliverEvents(pChild, &event, 1, NullWindow);
	    }
	    if (pChild->viewable)
	    {
		pChild->valdata = UnmapValData;
		anyMarked = TRUE;
	    }
	    pChild->mapped = FALSE;
	    if (pChild->realized)
		UnrealizeTree(pChild, FALSE);
	    if (wasViewable)
	    {
#ifdef DO_SAVE_UNDERS
		pChild->DIXsaveUnder = FALSE;
#endif /* DO_SAVE_UNDERS */
		if (pChild->backStorage)
		    (*pScreen->SaveDoomedAreas)(
					    pChild, &pChild->clipList, 0, 0);
	    }
	}
    }
    if (wasViewable)
    {
	if (anyMarked)
	{
	    if (pLayerWin->parent == pWin)
		(*pScreen->MarkWindow)(pWin);
	    else
	    {
		WindowPtr ptmp;
                (*pScreen->MarkOverlappedWindows)(pWin, pLayerWin,
						  (WindowPtr *)NULL);
		(*pScreen->MarkWindow)(pLayerWin->parent);
		
		/* Windows between pWin and pLayerWin may not have been marked */
		ptmp = pWin;
 
		while (ptmp != pLayerWin->parent)
		{
		    (*pScreen->MarkWindow)(ptmp);
		    ptmp = ptmp->parent;
		}
                pHead = pWin->firstChild;
	    }
	    (*pScreen->ValidateTree)(pLayerWin->parent, pHead, VTUnmap);
	    (*pScreen->HandleExposures)(pLayerWin->parent);
	}
#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    if ( (*pScreen->ChangeSaveUnder)(pLayerWin, pLayerWin))
		(*pScreen->PostChangeSaveUnder)(pLayerWin, pLayerWin);
	}
#endif /* DO_SAVE_UNDERS */
	if (anyMarked && pScreen->PostValidateTree)
	    (*pScreen->PostValidateTree)(pLayerWin->parent, pHead, VTUnmap);
    }
    if (wasRealized)
	WindowsRestructured ();
}


void
HandleSaveSet(client)
    register ClientPtr client;
{
    register WindowPtr pParent, pWin;
    register int j;

    for (j=0; j<client->numSaved; j++)
    {
	pWin = (WindowPtr)client->saveSet[j];
	pParent = pWin->parent;
	while (pParent && (wClient (pParent) == client))
	    pParent = pParent->parent;
	if (pParent)
	{
	    if (pParent != pWin->parent)
	    {
		ReparentWindow(pWin, pParent,
			       pWin->drawable.x - wBorderWidth (pWin) - pParent->drawable.x,
			       pWin->drawable.y - wBorderWidth (pWin) - pParent->drawable.y,
			       client);
		if(!pWin->realized && pWin->mapped)
		    pWin->mapped = FALSE;
	    }
	    MapWindow(pWin, client);
	}
    }
    xfree(client->saveSet);
    client->numSaved = 0;
    client->saveSet = (pointer *)NULL;
}

Bool
VisibleBoundingBoxFromPoint(pWin, x, y, box)
    register WindowPtr pWin;
    int x, y;	/* in root */
    BoxPtr box;	  /* "return" value */
{
    if (!pWin->realized)
	return (FALSE);
    if (POINT_IN_REGION(pWin->drawable.pScreen, &pWin->clipList, x, y, box))
	return(TRUE);
    return(FALSE);
}

Bool
PointInWindowIsVisible(pWin, x, y)
    register WindowPtr pWin;
    int x, y;	/* in root */
{
    BoxRec box;

    if (!pWin->realized)
	return (FALSE);
    if (POINT_IN_REGION(pWin->drawable.pScreen, &pWin->borderClip,
						  x, y, &box))
	return(TRUE);
    return(FALSE);
}


RegionPtr
NotClippedByChildren(pWin)
    register WindowPtr pWin;
{
    register ScreenPtr pScreen;
    RegionPtr pReg;

    pScreen = pWin->drawable.pScreen;
    pReg = REGION_CREATE(pScreen, NullBox, 1);
    if (pWin->parent ||
	screenIsSaved != SCREEN_SAVER_ON ||
	!HasSaverWindow (pWin->drawable.pScreen->myNum))
    {
	REGION_INTERSECT(pScreen, pReg, &pWin->borderClip, &pWin->winSize);
    }
    return(pReg);
}


void
SendVisibilityNotify(pWin)
    WindowPtr pWin;
{
    xEvent event;
    event.u.u.type = VisibilityNotify;
    event.u.visibility.window = pWin->drawable.id;
    event.u.visibility.state = pWin->visibility;
    DeliverEvents(pWin, &event, 1, NullWindow);
}


#define RANDOM_WIDTH 32

#ifndef NOLOGOHACK
extern int logoScreenSaver;
static void DrawLogo(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);
#endif

void
SaveScreens(on, mode)
    int on;
    int mode;
{
    int i;
    int what;
    int type;

    if (on == SCREEN_SAVER_FORCER)
    {
	UpdateCurrentTimeIf();
	lastDeviceEventTime = currentTime;
	if (mode == ScreenSaverReset)
	    what = SCREEN_SAVER_OFF;
	else
	    what = SCREEN_SAVER_ON;
	type = what;
    }
    else
    {
	what = on;
	type = what;
	if (what == screenIsSaved)
	    type = SCREEN_SAVER_CYCLE;
    }
    for (i = 0; i < screenInfo.numScreens; i++)
    {
	if (on == SCREEN_SAVER_FORCER)
	   (* screenInfo.screens[i]->SaveScreen) (screenInfo.screens[i], on);
	if (savedScreenInfo[i].ExternalScreenSaver)
	{
	    if ((*savedScreenInfo[i].ExternalScreenSaver)
		(screenInfo.screens[i], type, on == SCREEN_SAVER_FORCER))
		continue;
	}
	if (type == screenIsSaved)
	    continue;
	switch (type) {
	case SCREEN_SAVER_OFF:
	    if (savedScreenInfo[i].blanked == SCREEN_IS_BLANKED)
	    {
	       (* screenInfo.screens[i]->SaveScreen) (screenInfo.screens[i],
						      what);
	    }
	    else if (HasSaverWindow (i))
	    {
		savedScreenInfo[i].pWindow = NullWindow;
		FreeResource(savedScreenInfo[i].wid, RT_NONE);
	    }
	    break;
	case SCREEN_SAVER_CYCLE:
	    if (savedScreenInfo[i].blanked == SCREEN_IS_TILED)
	    {
		WindowPtr pWin = savedScreenInfo[i].pWindow;
		/* make it look like screen saver is off, so that
		 * NotClippedByChildren will compute a clip list
		 * for the root window, so miPaintWindow works
		 */
		screenIsSaved = SCREEN_SAVER_OFF;
#ifndef NOLOGOHACK
		if (logoScreenSaver)
		    (*pWin->drawable.pScreen->ClearToBackground)(pWin, 0, 0, 0, 0, FALSE);
#endif
		(*pWin->drawable.pScreen->MoveWindow)(pWin,
			   (short)(-(rand() % RANDOM_WIDTH)),
			   (short)(-(rand() % RANDOM_WIDTH)),
			   pWin->nextSib, VTMove);
#ifndef NOLOGOHACK
		if (logoScreenSaver)
		    DrawLogo(pWin);
#endif
		screenIsSaved = SCREEN_SAVER_ON;
	    }
	    break;
	case SCREEN_SAVER_ON:
	    if (ScreenSaverBlanking != DontPreferBlanking)
	    {
		if ((* screenInfo.screens[i]->SaveScreen)
		   (screenInfo.screens[i], what))
		{
		   savedScreenInfo[i].blanked = SCREEN_IS_BLANKED;
		   continue;
		}
		if ((ScreenSaverAllowExposures != DontAllowExposures) &&
		    TileScreenSaver(i, SCREEN_IS_BLACK))
		{
		    savedScreenInfo[i].blanked = SCREEN_IS_BLACK;
		    continue;
		}
	    }
	    if ((ScreenSaverAllowExposures != DontAllowExposures) &&
		TileScreenSaver(i, SCREEN_IS_TILED))
	    {
		savedScreenInfo[i].blanked = SCREEN_IS_TILED;
	    }
	    else
		savedScreenInfo[i].blanked = SCREEN_ISNT_SAVED;
	    break;
	}
    }
    screenIsSaved = what;
}

static Bool
#if NeedFunctionPrototypes
TileScreenSaver(int i, int kind)
#else
TileScreenSaver(i, kind)
    int i;
    int	kind;
#endif
{
    int j;
    int result;
    XID attributes[3];
    Mask mask;
    WindowPtr pWin;		
    CursorMetricRec cm;
    unsigned char *srcbits, *mskbits;
    CursorPtr cursor;
    XID	    cursorID;
    int	attri;

    mask = 0;
    attri = 0;
    switch (kind) {
    case SCREEN_IS_TILED:
	switch (WindowTable[i]->backgroundState) {
	case BackgroundPixel:
	    attributes[attri++] = WindowTable[i]->background.pixel;
	    mask |= CWBackPixel;
	    break;
	case BackgroundPixmap:
	    attributes[attri++] = None;
	    mask |= CWBackPixmap;
	    break;
	default:
	    break;
	}
	break;
    case SCREEN_IS_BLACK:
	attributes[attri++] = WindowTable[i]->drawable.pScreen->blackPixel;
	mask |= CWBackPixel;
	break;
    }
    mask |= CWOverrideRedirect;
    attributes[attri++] = xTrue;

    /*
     * create a blank cursor
     */

    cm.width=16;
    cm.height=16;
    cm.xhot=8;
    cm.yhot=8;
    srcbits = (unsigned char *)xalloc( BitmapBytePad(32)*16);
    mskbits = (unsigned char *)xalloc( BitmapBytePad(32)*16);
    if (!srcbits || !mskbits)
    {
	xfree(srcbits);
	xfree(mskbits);
	cursor = 0;
    }
    else
    {
	for (j=0; j<BitmapBytePad(32)*16; j++)
	    srcbits[j] = mskbits[j] = 0x0;
	cursor = AllocCursor(srcbits, mskbits, &cm, 0, 0, 0, 0, 0, 0);
	if (cursor)
	{
	    cursorID = FakeClientID(0);
	    if (AddResource (cursorID, RT_CURSOR, (pointer) cursor))
	    {
		attributes[attri] = cursorID;
		mask |= CWCursor;
	    }
	    else
		cursor = 0;
	}
	else
	{
	    xfree (srcbits);
	    xfree (mskbits);
	}
    }

    pWin = savedScreenInfo[i].pWindow =
	 CreateWindow(savedScreenInfo[i].wid,
	      WindowTable[i],
	      -RANDOM_WIDTH, -RANDOM_WIDTH,
	      (unsigned short)screenInfo.screens[i]->width + RANDOM_WIDTH,
	      (unsigned short)screenInfo.screens[i]->height + RANDOM_WIDTH,
	      0, InputOutput, mask, attributes, 0, serverClient,
	      wVisual (WindowTable[i]), &result);

    if (cursor)
	FreeResource (cursorID, RT_NONE);

    if (!pWin)
	return FALSE;

    if (!AddResource(pWin->drawable.id, RT_WINDOW,
		     (pointer)savedScreenInfo[i].pWindow))
	return FALSE;

    if (mask & CWBackPixmap)
    {
	MakeRootTile (pWin);
	(*pWin->drawable.pScreen->ChangeWindowAttributes)(pWin, CWBackPixmap);
    }
    MapWindow(pWin, serverClient);
#ifndef NOLOGOHACK
    if (kind == SCREEN_IS_TILED && logoScreenSaver)
	DrawLogo(pWin);
#endif
    return TRUE;
}

/*
 * FindWindowWithOptional
 *
 * search ancestors of the given window for an entry containing
 * a WindowOpt structure.  Assumptions:	 some parent will
 * contain the structure.
 */

WindowPtr
FindWindowWithOptional (w)
    register WindowPtr w;
{
    do
	w = w->parent;
    while (!w->optional);
    return w;
}

/*
 * CheckWindowOptionalNeed
 *
 * check each optional entry in the given window to see if
 * the value is satisfied by the default rules.	 If so,
 * release the optional record
 */

void
CheckWindowOptionalNeed (w)
    register WindowPtr w;
{
    register WindowOptPtr optional;
    register WindowOptPtr parentOptional;

    if (!w->parent)
	return;
    optional = w->optional;
    if (optional->dontPropagateMask != DontPropagateMasks[w->dontPropagate])
	return;
    if (optional->otherEventMasks != 0)
	return;
    if (optional->otherClients != NULL)
	return;
    if (optional->passiveGrabs != NULL)
	return;
    if (optional->userProps != NULL)
	return;
    if (optional->backingBitPlanes != ~0L)
	return;
    if (optional->backingPixel != 0)
	return;
#ifdef SHAPE
    if (optional->boundingShape != NULL)
	return;
    if (optional->clipShape != NULL)
	return;
#endif
#ifdef XINPUT
    if (optional->inputMasks != NULL)
	return;
#endif
    parentOptional = FindWindowWithOptional(w)->optional;
    if (optional->visual != parentOptional->visual)
	return;
    if (optional->cursor != None &&
	(optional->cursor != parentOptional->cursor ||
	 w->parent->cursorIsNone))
	return;
    if (optional->colormap != parentOptional->colormap)
	return;
    DisposeWindowOptional (w);
}

/*
 * MakeWindowOptional
 *
 * create an optional record and initialize it with the default
 * values.
 */

Bool
MakeWindowOptional (pWin)
    register WindowPtr pWin;
{
    register WindowOptPtr optional;
    register WindowOptPtr parentOptional;

    if (pWin->optional)
	return TRUE;
    optional = (WindowOptPtr) xalloc (sizeof (WindowOptRec));
    if (!optional)
	return FALSE;
    optional->dontPropagateMask = DontPropagateMasks[pWin->dontPropagate];
    optional->otherEventMasks = 0;
    optional->otherClients = NULL;
    optional->passiveGrabs = NULL;
    optional->userProps = NULL;
    optional->backingBitPlanes = ~0L;
    optional->backingPixel = 0;
#ifdef SHAPE
    optional->boundingShape = NULL;
    optional->clipShape = NULL;
#endif
#ifdef XINPUT
    optional->inputMasks = NULL;
#endif
    parentOptional = FindWindowWithOptional(pWin)->optional;
    optional->visual = parentOptional->visual;
    if (!pWin->cursorIsNone)
    {
	optional->cursor = parentOptional->cursor;
	optional->cursor->refcnt++;
    }
    else
    {
	optional->cursor = None;
    }
    optional->colormap = parentOptional->colormap;
    pWin->optional = optional;
    return TRUE;
}

void
DisposeWindowOptional (pWin)
    register WindowPtr pWin;
{
    if (!pWin->optional)
	return;
    /*
     * everything is peachy.  Delete the optional record
     * and clean up
     */
    if (pWin->optional->cursor)
    {
	FreeCursor (pWin->optional->cursor, (Cursor)0);
	pWin->cursorIsNone = FALSE;
    }
    else
	pWin->cursorIsNone = TRUE;
    xfree (pWin->optional);
    pWin->optional = NULL;
}

#ifndef NOLOGOHACK
static void
#if NeedFunctionPrototypes
DrawLogo(WindowPtr pWin)
#else
DrawLogo(pWin)
    WindowPtr pWin;
#endif
{
    DrawablePtr pDraw;
    ScreenPtr pScreen;
    int x, y;
    unsigned int width, height, size;
    GC *pGC;
    int thin, gap, d31;
    DDXPointRec poly[4];
    ChangeGCVal fore[2], back[2];
    xrgb rgb[2];
    BITS32 fmask, bmask;
    ColormapPtr cmap;

    pDraw = (DrawablePtr)pWin;
    pScreen = pDraw->pScreen;
    x = -pWin->origin.x;
    y = -pWin->origin.y;
    width = pScreen->width;
    height = pScreen->height;
    pGC = GetScratchGC(pScreen->rootDepth, pScreen);
    if (!pGC)
	return;

    if ((rand() % 100) <= 17) /* make the probability for white fairly low */
	fore[0].val = pScreen->whitePixel;
    else
	fore[0].val = pScreen->blackPixel;
    if ((pWin->backgroundState == BackgroundPixel) &&
	(cmap = (ColormapPtr)LookupIDByType(wColormap (pWin), RT_COLORMAP))) {
	Pixel querypixels[2];

	querypixels[0] = fore[0].val;
	querypixels[1] = pWin->background.pixel;
	QueryColors(cmap, 2, querypixels, rgb);
	if ((rgb[0].red == rgb[1].red) &&
	    (rgb[0].green == rgb[1].green) &&
	    (rgb[0].blue == rgb[1].blue)) {
	    if (fore[0].val == pScreen->blackPixel)
		fore[0].val = pScreen->whitePixel;
	    else
		fore[0].val = pScreen->blackPixel;
	}
    }
    fore[1].val = FillSolid;
    fmask = GCForeground|GCFillStyle;
    if (pWin->backgroundState == BackgroundPixel) {
	back[0].val = pWin->background.pixel;
	back[1].val = FillSolid;
	bmask = GCForeground|GCFillStyle;
    } else {
	back[0].val = 0;
	back[1].val = 0;
	dixChangeGC(NullClient, pGC, GCTileStipXOrigin|GCTileStipYOrigin,
		    NULL, back);
	back[0].val = FillTiled;
	back[1].ptr = pWin->background.pixmap;
	bmask = GCFillStyle|GCTile;
    }

    /* should be the same as the reference function XmuDrawLogo() */

    size = width;
    if (height < width)
	 size = height;
    size = RANDOM_WIDTH + rand() % (size - RANDOM_WIDTH);
    size &= ~1;
    x += rand() % (width - size);
    y += rand() % (height - size);

/*
 * Draw what will be the thin strokes.
 *
 *           -----
 *          /    /
 *         /    /
 *        /    /
 *       /    /
 *      /____/
 *           d
 *
 * Point d is 9/44 (~1/5) of the way across.
 */

    thin = (size / 11);
    if (thin < 1) thin = 1;
    gap = (thin+3) / 4;
    d31 = thin + thin + gap;
    poly[0].x = x + size;	       poly[0].y = y;
    poly[1].x = x + size-d31;	       poly[1].y = y;
    poly[2].x = x + 0;		       poly[2].y = y + size;
    poly[3].x = x + d31;	       poly[3].y = y + size;
    dixChangeGC(NullClient, pGC, fmask, NULL, fore);
    ValidateGC(pDraw, pGC);
    (*pGC->ops->FillPolygon)(pDraw, pGC, Convex, CoordModeOrigin, 4, poly);

/*
 * Erase area not needed for lower thin stroke.
 *
 *           ------
 *          /	  /
 *         /  __ /
 *        /  /	/
 *       /  /  /
 *      /__/__/
 */

    poly[0].x = x + d31/2;			 poly[0].y = y + size;
    poly[1].x = x + size / 2;			 poly[1].y = y + size/2;
    poly[2].x = x + (size/2)+(d31-(d31/2));	 poly[2].y = y + size/2;
    poly[3].x = x + d31;			 poly[3].y = y + size;
    dixChangeGC(NullClient, pGC, bmask, NULL, back);
    ValidateGC(pDraw, pGC);
    (*pGC->ops->FillPolygon)(pDraw, pGC, Convex, CoordModeOrigin, 4, poly);

/*
 * Erase area not needed for upper thin stroke.
 *
 *	     ------
 *	    /  /  /
 *	   /--/	 /
 *	  /	/
 *	 /     /
 *	/_____/
 */

    poly[0].x = x + size - d31/2;		 poly[0].y = y;
    poly[1].x = x + size / 2;			 poly[1].y = y + size/2;
    poly[2].x = x + (size/2)-(d31-(d31/2));	 poly[2].y = y + size/2;
    poly[3].x = x + size - d31;			 poly[3].y = y;
    ValidateGC(pDraw, pGC);
    (*pGC->ops->FillPolygon)(pDraw, pGC, Convex, CoordModeOrigin, 4, poly);

/*
 * Draw thick stroke.
 * Point b is 1/4 of the way across.
 *
 *      b
 * -----
 * \	\
 *  \	 \
 *   \	  \
 *    \	   \
 *     \____\
 */

    poly[0].x = x;		       poly[0].y = y;
    poly[1].x = x + size/4;	       poly[1].y = y;
    poly[2].x = x + size;	       poly[2].y = y + size;
    poly[3].x = x + size - size/4;     poly[3].y = y + size;
    dixChangeGC(NullClient, pGC, fmask, NULL, fore);
    ValidateGC(pDraw, pGC);
    (*pGC->ops->FillPolygon)(pDraw, pGC, Convex, CoordModeOrigin, 4, poly);

/*
 * Erase to create gap.
 *
 *	    /
 *	   /
 *	  /
 *	 /
 *	/
 */

    poly[0].x = x + size- thin;	      poly[0].y = y;
    poly[1].x = x + size-( thin+gap);  poly[1].y = y;
    poly[2].x = x + thin;	      poly[2].y = y + size;
    poly[3].x = x + thin + gap;	      poly[3].y = y + size;
    dixChangeGC(NullClient, pGC, bmask, NULL, back);
    ValidateGC(pDraw, pGC);
    (*pGC->ops->FillPolygon)(pDraw, pGC, Convex, CoordModeOrigin, 4, poly);

    FreeScratchGC(pGC);
}

#endif
