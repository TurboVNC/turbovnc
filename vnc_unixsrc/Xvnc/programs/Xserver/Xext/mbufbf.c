/*

Copyright (c) 1989  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/

/* $XConsortium: mbufbf.c,v 1.5 94/04/17 20:32:53 dpw Exp $ */
/* $XFree86: xc/programs/Xserver/Xext/mbufbf.c,v 3.0 1994/05/08 05:17:30 dawes Exp $ */

#define NEED_REPLIES
#define NEED_EVENTS
#include <stdio.h>
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "os.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "extnsionst.h"
#include "dixstruct.h"
#include "resource.h"
#include "opaque.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "inputstr.h"
#include "validate.h"
#ifndef MINIX
#include <sys/time.h>
#endif

#define _MULTIBUF_SERVER_	/* don't want Xlib structures */
#define _MULTIBUF_BUFFER_
#include "multibufst.h"

/* 
Support for doublebuffer hardare

This code is designed to support doublebuffer hardware where the
displayed buffer is selected on a per-pixel basis by an additional bit
plane, called the select plane. It could probably be easily modified
to work with systems that use window-id planes.

This is done by creating a new drawable type, DRAWABLE_BUFFER. The
type has the same exact layout as a window drawable. Your code should
treat a DRAWABLE_BUFFER the same as it would tread a DRAWABLE_WINDOW
when handling the gc drawing functions. In addition, PaintWindowBackground,
CopyWindow, and all of the gc drawing functions to be able to draw into both
framebuffers. Which framebuffer to draw into is selected by the contents of
	pWin->devPrivates[frameWindowPrivateIndex].
The content of the devPrivate is either from frameBuffer[0] or
frameBuffer[1], depending on which buffer is being drawn into. When
	pWin->devPrivates[frameWindowPrivateIndex] == frameBuffer[0],
the functions should draw into the front framebuffer. When
	pWin->devPrivates[frameWindowPrivateIndex] == frameBuffer[1],
the functions should draw into the back framebuffer.

In addition, you need to provide a function that allows you to copy
bits between the buffers (optional since CopyArea can be used) and a
function that draws into the select plane. Then, you need to register
your functions and other information, by calling:

void
RegisterDoubleBufferHardware(pScreen, nInfo, pInfo, frameBuffer, selectPlane,
			     CopyBufferBitsFunc, DrawSelectPlaneFunc)
    int			nInfo;
    xMbufBufferInfo	*pInfo;
    DevUnion		*frameBuffer;
    DevUnion		selectPlane;

"pInfo" is an array indicating which visuals and depths that double
buffering is supported on. "nInfo" is the length of the array.

"frameBuffer" is array of length 2. The contents of the array element
is ddx-specific. The content of frameBuffer[0] should, when placed in
the window private, indicate that framebuffer 0 should be drawn into.
The contents of frameBuffer[1], when placed into the window private,
should indicate that framebuffer 1 should be drawn into.

"selectPlane" is ddx-specific. It should contain information
neccessary for your displayProc to access the select plane.
It is passed to DrawSelectPlaneFunc.

"CopyBufferBitsFunc" is a ddx-specific function that copies from one
buffer of a multibuffered window to another buffer. If the CopyBufferBitsFunc
is NULL, a default function will be used that calls pScreen->CopyArea.

    void CopyBufferBitsFunc(pMBWindow, srcBufferNum, dstBufferNum)
        mbufWindowPtr pMBWindow;
        int srcBufferNum, dstBufferNum;

"DrawSelectPlaneFunc" is a ddx-specific function that fills the
regions "prgn" of select plane with the value "bufferNum". If 
selectPlane is a DrawablePtr (such as a PixmapPtr), you can pass
NULL for DrawSelectPlaneFunc, a default function will be used that
calls FillRectangle on the selectPlane.

    void DrawSelectPlaneFunc(pScreen, selectPlane, prgn, bufferNum)
        ScreenPtr	pScreen;
        DevUnion	selectPlane;
        RegionPtr	prgn;
        long		bufferNum;

...
...
...

*/

#define MAX_BUFFERS  2	/* Only supports 2 buffers */
#define FRONT_BUFFER 0
#define BACK_BUFFER  1


/* Buffer drawables have the same structure as window drawables */
typedef WindowRec BufferRec;
typedef WindowPtr BufferPtr;


/*
 * Call RegisterHdwrBuffer for every screen that has doublebuffer hardware. 
 */

static int		bufNumInfo[MAXSCREENS];
static xMbufBufferInfo	*bufInfo[MAXSCREENS];
static DevUnion		*bufFrameBuffer[MAXSCREENS];
static DevUnion		bufselectPlane[MAXSCREENS];
static void		(* bufCopyBufferBitsFunc[MAXSCREENS])();
static void		(* bufDrawSelectPlaneFunc[MAXSCREENS])();

static Bool bufMultibufferInit();


void
RegisterDoubleBufferHardware(pScreen, nInfo, pInfo, frameBuffer, selectPlane,
			     CopyBufferBitsFunc, DrawSelectPlaneFunc)
    ScreenPtr		pScreen;
    int			nInfo;
    xMbufBufferInfo	*pInfo;
    DevUnion		*frameBuffer;
    DevUnion		selectPlane;
    void		(* CopyBufferBitsFunc)();
    void		(* DrawSelectPlaneFunc)();
{
    bufNumInfo[pScreen->myNum]     = nInfo;
    bufInfo[pScreen->myNum]        = pInfo;
    bufFrameBuffer[pScreen->myNum] = frameBuffer;
    bufselectPlane[pScreen->myNum] = selectPlane;

    bufCopyBufferBitsFunc[pScreen->myNum]  = CopyBufferBitsFunc;
    bufDrawSelectPlaneFunc[pScreen->myNum] = DrawSelectPlaneFunc;

    /* Register ourselves with device-independent multibuffers code */
    RegisterMultibufferInit(pScreen, bufMultibufferInit);
}


/*
 * Called by Multibuffer extension initialization.
 * Initializes mbufScreenRec and its devPrivate.
 */
    
static Bool NoopDDA_True() { return TRUE; }
static Bool bufPositionWindow();
static int  bufCreateImageBuffers();
static void bufDestroyImageBuffers();
static void bufDisplayImageBuffers();
static void bufClearImageBufferArea();
static void bufDestroyBuffer();
static void bufCopyBufferBits();
static void bufDrawSelectPlane();
static void bufWrapScreenFuncs();
static void bufResetProc();

static void bufPostValidateTree();
static void bufClipNotify();
static void bufWindowExposures();
static Bool bufChangeWindowAttributes();
static void bufClearToBackground();
static void bufCopyWindow();

extern WindowPtr *WindowTable;

static Bool
bufMultibufferInit(pScreen, pMBScreen)
    ScreenPtr pScreen;
    mbufScreenPtr pMBScreen;
{
    mbufBufferPrivPtr	pMBPriv;
    BoxRec		box;

    /* Multibuffer info */
    pMBScreen->nInfo = bufNumInfo[pScreen->myNum];
    pMBScreen->pInfo = bufInfo[pScreen->myNum];

    /* Hooks */
    pMBScreen->CreateImageBuffers = bufCreateImageBuffers;
    pMBScreen->DestroyImageBuffers = bufDestroyImageBuffers;
    pMBScreen->DisplayImageBuffers = bufDisplayImageBuffers;
    pMBScreen->ClearImageBufferArea = bufClearImageBufferArea;
    pMBScreen->ChangeMBufferAttributes = NoopDDA_True;
    pMBScreen->ChangeBufferAttributes = NoopDDA_True;
    pMBScreen->DeleteBufferDrawable = bufDestroyBuffer;
    pMBScreen->WrapScreenFuncs = bufWrapScreenFuncs;
    pMBScreen->ResetProc = bufResetProc;
    /* Create devPrivate part */
    pMBPriv = (mbufBufferPrivPtr) xalloc(sizeof *pMBPriv);
    if (!pMBPriv)
	return (FALSE);

    pMBScreen->devPrivate.ptr = (pointer) pMBPriv;
    pMBPriv->frameBuffer  = bufFrameBuffer[pScreen->myNum];
    pMBPriv->selectPlane = bufselectPlane[pScreen->myNum];

    /*
     * Initializing the subtractRgn to the screen area will ensure that
     * the selectPlane will get cleared on the first PostValidateTree.
     */

    box.x1 = 0;
    box.y1 = 0;
    box.x2 = pScreen->width;
    box.y2 = pScreen->height;

    pMBPriv->rgnChanged = TRUE;
    REGION_INIT(pScreen, &pMBPriv->backBuffer, &box, 1);
    REGION_INIT(pScreen, &pMBPriv->subtractRgn, &box, 1);
    REGION_INIT(pScreen, &pMBPriv->unionRgn, NullBox, 0);

    /* Misc functions */
    pMBPriv->CopyBufferBits  = bufCopyBufferBitsFunc[pScreen->myNum];
    pMBPriv->DrawSelectPlane = bufDrawSelectPlaneFunc[pScreen->myNum];

    if (!pMBPriv->CopyBufferBits)
	pMBPriv->CopyBufferBits = bufCopyBufferBits;

    if (!pMBPriv->DrawSelectPlane)
	pMBPriv->DrawSelectPlane = bufDrawSelectPlane;

    /* screen functions */
    pMBPriv->funcsWrapped = 0;
    pMBPriv->inClearToBackground = FALSE;
    pMBPriv->WindowExposures = NULL;
    pMBPriv->CopyWindow = NULL;
    pMBPriv->ClearToBackground = NULL;
    pMBPriv->ClipNotify = NULL;
    pMBPriv->ChangeWindowAttributes = NULL;

    /* Start out wrapped to clear select plane */
    WRAP_SCREEN_FUNC(pScreen,pMBPriv,PostValidateTree, bufPostValidateTree);
    return TRUE;
}

static void
UpdateBufferFromWindow(pBuffer, pWin)
    BufferPtr	pBuffer;
    WindowPtr	pWin;
{
    pBuffer->drawable.x      = pWin->drawable.x;
    pBuffer->drawable.y      = pWin->drawable.y;
    pBuffer->drawable.width  = pWin->drawable.width;
    pBuffer->drawable.height = pWin->drawable.height;

    pBuffer->drawable.serialNumber = NEXT_SERIAL_NUMBER;

    /* Update for PaintWindowBackground */
    pBuffer->parent = pWin->parent;

    /*
     * Make the borderClip the same as the clipList so
     * NotClippedByChildren comes out with just clipList.
     */

    pBuffer->clipList   = pWin->clipList;
    pBuffer->borderClip = pWin->clipList;
    pBuffer->winSize    = pWin->winSize;
    pBuffer->borderSize = pWin->borderSize;

    pBuffer->origin = pWin->origin;
}

static BufferPtr
bufCreateBuffer(pScreen, pWin, bufferNum)
    ScreenPtr	pScreen;
    WindowPtr	pWin;
    int		bufferNum;
{
    mbufBufferPrivPtr	pMBPriv;
    DevUnion	*devPrivates;
    BufferPtr	pBuffer;
    int		i;

    pMBPriv = MB_SCREEN_PRIV_BUFFER(pScreen);

    pBuffer = AllocateWindow(pWin->drawable.pScreen);
    if (!pBuffer)
	return (NULL);

    /* XXX- Until we know what is needed, copy everything. */
    devPrivates = pBuffer->devPrivates;
    *pBuffer = *pWin;
    pBuffer->devPrivates   = devPrivates;

    pBuffer->drawable.type = DRAWABLE_BUFFER;
    pBuffer->drawable.serialNumber = NEXT_SERIAL_NUMBER;

    pBuffer->nextSib    = NULL;
    pBuffer->prevSib    = NULL;
    pBuffer->firstChild = NULL;
    pBuffer->lastChild  = NULL;

    /* XXX - Worry about backingstore later */
    pBuffer->backStorage   = NULL;
    pBuffer->backingStore  = NotUseful;

    /* XXX - Need to call pScreen->CreateWindow for tile/stipples
     *       or should I just copy the devPrivates?
     */
    
    for (i=0; i < pScreen->WindowPrivateLen; i++)
	pBuffer->devPrivates[i] = pWin->devPrivates[i];

    pBuffer->devPrivates[frameWindowPrivateIndex] =
	pMBPriv->frameBuffer[bufferNum];

    return pBuffer;
}

static void
bufDestroyBuffer(pDrawable)
    DrawablePtr	pDrawable;
{
    xfree(pDrawable);
}

/*ARGSUSED*/
static int
bufCreateImageBuffers (pWin, nbuf, ids, action, hint)
    WindowPtr	pWin;
    int		nbuf;
    XID		*ids;
    int		action;
    int		hint;
{
    ScreenPtr		pScreen;
    mbufScreenPtr	pMBScreen;
    mbufWindowPtr	pMBWindow;
    mbufBufferPtr	pMBBuffer;
    int			i;

    pScreen   = pWin->drawable.pScreen;
    pMBScreen = MB_SCREEN_PRIV(pScreen);
    pMBWindow = MB_WINDOW_PRIV(pWin);

    pMBWindow->devPrivate.ptr = (pointer) REGION_CREATE(pScreen, 0,0);
    if (!pMBWindow->devPrivate.ptr)
	return(0);
    REGION_COPY(pScreen, (RegionPtr) pMBWindow->devPrivate.ptr,
			    &pWin->clipList);

    for (i = 0; i < nbuf; i++)
    {
	pMBBuffer = pMBWindow->buffers + i;
	pMBBuffer->pDrawable = (DrawablePtr) bufCreateBuffer(pScreen,pWin,i);

	if (!pMBBuffer->pDrawable)
	    break;

	if (!AddResource (ids[i], MultibufferDrawableResType,
			  (pointer) pMBBuffer->pDrawable))
	{
	    bufDestroyBuffer((BufferPtr) pMBBuffer->pDrawable);
	    break;
	}
	pMBBuffer->pDrawable->id = ids[i];

	/*
	 * If window is already mapped, generate exposures and
	 * clear the area of the newly buffers.
	 */

	if ((pWin->realized) && (i != pMBWindow->displayedMultibuffer))
	    (* pMBScreen->ClearImageBufferArea)(pMBBuffer, 0,0, 0,0, TRUE);
    }

    return i;
}

static void
bufDestroyImageBuffers(pWin)
    WindowPtr	pWin;
{
    ScreenPtr		pScreen;
    mbufWindowPtr	pMBWindow;

    pScreen   = pWin->drawable.pScreen;

    if (pMBWindow = MB_WINDOW_PRIV(pWin))
    {
	mbufBufferPrivPtr pMBPriv = MB_SCREEN_PRIV_BUFFER(pScreen);

	/*
	 * if the backbuffer is currently being displayed, move the bits
	 * to the frontbuffer and display it instead.
	 */

	if (pWin->realized && (pMBWindow->displayedMultibuffer == BACK_BUFFER))
	{
	    (* pMBPriv->CopyBufferBits)(pMBWindow, BACK_BUFFER, FRONT_BUFFER);
	    REGION_SUBTRACT(pScreen, &pMBPriv->backBuffer,
				  &pMBPriv->backBuffer, &pWin->clipList);
	    (* pMBPriv->DrawSelectPlane)(pScreen, pMBPriv->selectPlane,
			    &pWin->clipList, FRONT_BUFFER);
	}

	/* Switch window rendering to front buffer */
	pWin->devPrivates[frameWindowPrivateIndex] =
	    pMBPriv->frameBuffer[FRONT_BUFFER];

	REGION_DESTROY(pScreen, (RegionPtr) pMBWindow->devPrivate.ptr);
	pMBWindow->devPrivate.ptr = NULL;
    }
}

/*
 * Can be replaced by pScreen->ClearToBackground if pBuffer->eventMask
 * and wOtherEventsMasks(pBuffer) were setup.
 */

static void
bufClearImageBufferArea(pMBBuffer, x,y, w,h, generateExposures)
    mbufBufferPtr	pMBBuffer;
    short		x,y;
    unsigned short	w,h;
    Bool		generateExposures;
{
    BoxRec box;
    RegionRec	reg;
    RegionPtr pBSReg = NullRegion;
    ScreenPtr	pScreen;
    BoxPtr  extents;
    int	    x1, y1, x2, y2;
    BufferPtr pBuffer;

    pBuffer = (BufferPtr) pMBBuffer->pDrawable;
    /* compute everything using ints to avoid overflow */

    x1 = pBuffer->drawable.x + x;
    y1 = pBuffer->drawable.y + y;
    if (w)
        x2 = x1 + (int) w;
    else
        x2 = x1 + (int) pBuffer->drawable.width - (int) x;
    if (h)
        y2 = y1 + h;	
    else
        y2 = y1 + (int) pBuffer->drawable.height - (int) y;

    extents = &pBuffer->clipList.extents;
    
    /* clip the resulting rectangle to the window clipList extents.  This
     * makes sure that the result will fit in a box, given that the
     * screen is < 32768 on a side.
     */

    if (x1 < extents->x1)
	x1 = extents->x1;
    if (x2 > extents->x2)
	x2 = extents->x2;
    if (y1 < extents->y1)
	y1 = extents->y1;
    if (y2 > extents->y2)
	y2 = extents->y2;

    if (x2 <= x1 || y2 <= y1)
    {
	x2 = x1 = 0;
	y2 = y1 = 0;
    }

    box.x1 = x1;
    box.x2 = x2;
    box.y1 = y1;
    box.y2 = y2;

    pScreen = pBuffer->drawable.pScreen;
    REGION_INIT(pScreen, &reg, &box, 1);
    if (pBuffer->backStorage)
    {
	/*
	 * If the window has backing-store on, call through the
	 * ClearToBackground vector to handle the special semantics
	 * (i.e. things backing store is to be cleared out and
	 * an Expose event is to be generated for those areas in backing
	 * store if generateExposures is TRUE).
	 */
	pBSReg = (* pScreen->ClearBackingStore)(pBuffer, x, y, w, h,
						 generateExposures);
    }

    REGION_INTERSECT(pScreen, &reg, &reg, &pBuffer->clipList);
    if (pBuffer->backgroundState != None)
	(*pScreen->PaintWindowBackground)(pBuffer, &reg, PW_BACKGROUND);
    if (generateExposures)
	MultibufferExpose(pMBBuffer, &reg);
#ifdef _notdef
    /* XXBS - This is the original miClearToBackground code.
     * WindowExposures needs to be called (or the functionality emulated)
     * in order for backingStore to work, but first, pBuffer->eventMask
     * and wOtherEventsMasks(pBuffer) need to be setup correctly.
     */

    if (generateExposures)
	(*pScreen->WindowExposures)(pBuffer, &reg, pBSReg);
    else if (pBuffer->backgroundState != None)
        (*pScreen->PaintWindowBackground)(pBuffer, &reg, PW_BACKGROUND);
#endif
    REGION_UNINIT(pScreen, &reg);
    if (pBSReg)
	REGION_DESTROY(pScreen, pBSReg);
}

static void
bufWrapScreenFuncs(pScreen)
    ScreenPtr pScreen;
{
    mbufBufferPrivPtr pMBPriv = MB_SCREEN_PRIV_BUFFER(pScreen);

    WRAP_SCREEN_FUNC(pScreen,pMBPriv,PostValidateTree, bufPostValidateTree);
    WRAP_SCREEN_FUNC(pScreen,pMBPriv,ClipNotify, bufClipNotify);
    WRAP_SCREEN_FUNC(pScreen,pMBPriv,WindowExposures,bufWindowExposures);
    WRAP_SCREEN_FUNC(pScreen,pMBPriv,ChangeWindowAttributes, bufChangeWindowAttributes);
    WRAP_SCREEN_FUNC(pScreen,pMBPriv,ClearToBackground,bufClearToBackground);
    WRAP_SCREEN_FUNC(pScreen,pMBPriv,CopyWindow,bufCopyWindow);
}

static void
bufResetProc(pScreen)
    ScreenPtr pScreen;
{
    mbufBufferPrivPtr pMBPriv = MB_SCREEN_PRIV_BUFFER(pScreen);

    /*
     * frameBuffer, selectPlane, and pInfo should be freed by
     * whoever called RegisterDoubleBufferHardware
     */

    REGION_UNINIT(pScreen, &pMBPriv->backBuffer);
    REGION_UNINIT(pScreen, &pMBPriv->subtractRgn);
    REGION_UNINIT(pScreen, &pMBPriv->unionRgn);
    xfree(pMBPriv);
}

/*---------------------------------------------------------------------------*/

/* 
 * Used if CopyBufferBitsFunc is not provided when registering.
 * This should work for everybody since CopyArea needs to support
 * copying between buffers anyway.
 */

static void
bufCopyBufferBits(pMBWindow, srcBufferNum, dstBufferNum)
    mbufWindowPtr pMBWindow;
    int srcBufferNum, dstBufferNum;
{
    DrawablePtr pSrcBuffer, pDstBuffer;
    GCPtr pGC;

    pSrcBuffer = pMBWindow->buffers[srcBufferNum].pDrawable;
    pDstBuffer = pMBWindow->buffers[dstBufferNum].pDrawable;

    pGC = GetScratchGC (pDstBuffer->depth, pDstBuffer->pScreen);
    if (!pGC)
	return;

    ValidateGC (pDstBuffer, pGC);
    (* pGC->ops->CopyArea) (pSrcBuffer, pDstBuffer, pGC,
		    0,0, pDstBuffer->width, pDstBuffer->height, 0,0);
    FreeScratchGC (pGC);
}

/*
 * Used if DrawSelectPlanFunc is not provided for when registering.
 * However, it only works if selectPlane.ptr is a drawable. Also
 * assumes that painting with color 0 selects the front buffer,
 * while color 1 selects the back buffer.
 */

static void
bufDrawSelectPlane(pScreen, selectPlane, prgn, bufferNum)
    ScreenPtr	pScreen;
    DevUnion	selectPlane;
    RegionPtr	prgn;
    long	bufferNum;
{
    DrawablePtr pDrawable;
    GCPtr pGC;
    register int i;
    register BoxPtr pbox;
    register xRectangle *prect;
    int numRects;
    XID	value;

    if (REGION_NUM_RECTS(prgn) == 0)
	return;

    pDrawable = (DrawablePtr) selectPlane.ptr;
    pGC = GetScratchGC (pDrawable->depth, pScreen);
    if (!pGC)
	return;

    prect = (xRectangle *)ALLOCATE_LOCAL(REGION_NUM_RECTS(prgn) *
					 sizeof(xRectangle));
    if (!prect)
    {
	FreeScratchGC(pGC);
	return;
    }

    value = (XID) bufferNum;
    DoChangeGC(pGC, GCForeground, &value, 0);
    ValidateGC(pDrawable, pGC);

    numRects = REGION_NUM_RECTS(prgn);
    pbox = REGION_RECTS(prgn);
    for (i= numRects; --i >= 0; pbox++, prect++)
    {
	prect->x = pbox->x1;
	prect->y = pbox->y1;
	prect->width = pbox->x2 - pbox->x1;
	prect->height = pbox->y2 - pbox->y1;
    }
    prect -= numRects;
    (* pGC->ops->PolyFillRect)(pDrawable, pGC, numRects, prect);

    DEALLOCATE_LOCAL(prect);
    FreeScratchGC (pGC);
}


static void
bufDisplayImageBuffers(pScreen, ppMBWindow, ppMBBuffer, nbuf)
    ScreenPtr		pScreen;
    mbufBufferPtr	*ppMBBuffer;
    mbufWindowPtr	*ppMBWindow;
    int			nbuf;
{
    WindowPtr       pWin;
    BufferPtr	    pPrevBuffer, pNewBuffer;
    int		    i, number;
    mbufBufferPrivPtr pMBPriv;
    mbufBufferPtr   pPrevMBBuffer;

    pMBPriv   = MB_SCREEN_PRIV_BUFFER(pScreen);

    for (i = 0; i < nbuf; i++)
    {
	number = ppMBBuffer[i]->number; /* 0=frontbuffer, 1=backbuffer */
	pWin = ppMBWindow[i]->pWindow;
	pPrevMBBuffer = MB_DISPLAYED_BUFFER(ppMBWindow[i]);

	pPrevBuffer = (BufferPtr) pPrevMBBuffer->pDrawable;
	pNewBuffer  = (BufferPtr) ppMBBuffer[i]->pDrawable;

	if (pPrevBuffer != pNewBuffer)
	{
	    RegionPtr backBuffer = &pMBPriv->backBuffer;

	    /*
	     * Update the select plane and the backBuffer region.
	     */

	    (* pMBPriv->DrawSelectPlane)(pScreen, pMBPriv->selectPlane,
			    &pWin->clipList, number);

	    if (number == BACK_BUFFER)
		REGION_UNION(pScreen, backBuffer, backBuffer,
				   &pWin->clipList);
	    else
		REGION_SUBTRACT(pScreen, backBuffer, backBuffer,
				   &pWin->clipList);

	    /* Switch which framebuffer the window draws into */
	    pWin->devPrivates[frameWindowPrivateIndex] =
		pMBPriv->frameBuffer[number];
	}

	switch (ppMBWindow[i]->updateAction)
	{
	case MultibufferUpdateActionUndefined:
	    break;
	case MultibufferUpdateActionBackground:
	    (* MB_SCREEN_PRIV(pScreen)->ClearImageBufferArea)
		(pPrevMBBuffer, 0,0, 0,0, FALSE);
	    break;
	case MultibufferUpdateActionUntouched:
	    break;
	case MultibufferUpdateActionCopied:
	    if (pPrevBuffer != pNewBuffer)
	    {
		(* pMBPriv->CopyBufferBits) (ppMBWindow[i],
			ppMBBuffer[i]->number, pPrevMBBuffer->number);
	    }
	    break;
	}
    }
}

/* Updates the backBuffer region and paints the selectPlane. */

static void
bufPostValidateTree(pParent, pChild, kind)
    WindowPtr	pParent, pChild;
    VTKind	kind;
{
    ScreenPtr pScreen;
    mbufBufferPrivPtr pMBPriv;

    if (pParent)
	pScreen = pParent->drawable.pScreen;
    else if (pChild)
	pScreen = pChild->drawable.pScreen;
    else
	return; /* Hopeless */

    pMBPriv = MB_SCREEN_PRIV_BUFFER(pScreen);

    UNWRAP_SCREEN_FUNC(pScreen, pMBPriv, void, PostValidateTree);
    if (pScreen->PostValidateTree)
	(* pScreen->PostValidateTree)(pParent, pChild, kind);
    REWRAP_SCREEN_FUNC(pScreen, pMBPriv, void, PostValidateTree);

    /* Does backBuffer need to change? */
    if (pMBPriv->rgnChanged)
    {
	RegionRec exposed;
	RegionPtr pSubtractRgn, pUnionRgn;
	Bool overlap;

	pMBPriv->rgnChanged = FALSE;

	pSubtractRgn = &pMBPriv->subtractRgn;
	pUnionRgn    = &pMBPriv->unionRgn;
	REGION_VALIDATE(pScreen, pSubtractRgn, &overlap);
#ifdef DEBUG
	if (overlap)
	    FatalError("bufPostValidateTree: subtractRgn overlaps");
#endif
	REGION_VALIDATE(pScreen, pUnionRgn, &overlap);
#ifdef DEBUG
	if (overlap)
	    FatalError("bufPostValidateTree: unionRgn overlaps");
#endif

	/* Update backBuffer: subtract must come before union */
	REGION_SUBTRACT(pScreen, &pMBPriv->backBuffer, &pMBPriv->backBuffer,
			      pSubtractRgn);
	REGION_UNION(pScreen, &pMBPriv->backBuffer, &pMBPriv->backBuffer,
			      pUnionRgn);

	/* Paint gained and lost backbuffer areas in select plane */
	REGION_INIT(pScreen, &exposed, NullBox, 0);
	REGION_SUBTRACT(pScreen, &exposed, pSubtractRgn, pUnionRgn);
	(* pMBPriv->DrawSelectPlane)(pScreen, pMBPriv->selectPlane,
				     &exposed, FRONT_BUFFER);

	REGION_SUBTRACT(pScreen, &exposed, pUnionRgn, pSubtractRgn);
	(* pMBPriv->DrawSelectPlane)(pScreen, pMBPriv->selectPlane,
				    &exposed, BACK_BUFFER);
	
	REGION_UNINIT(pScreen, &exposed);
	REGION_EMPTY(pScreen, pSubtractRgn);
	REGION_EMPTY(pScreen, pUnionRgn);
    }
}

/* XXX - Knows region internals. */

static Bool
RegionsEqual(reg1, reg2)
    RegionPtr reg1;
    RegionPtr reg2;
{
    int i;
    BoxPtr rects1, rects2;

    if (reg1->extents.x1 != reg2->extents.x1) return FALSE;
    if (reg1->extents.x2 != reg2->extents.x2) return FALSE;
    if (reg1->extents.y1 != reg2->extents.y1) return FALSE;
    if (reg1->extents.y2 != reg2->extents.y2) return FALSE;
    if (REGION_NUM_RECTS(reg1) != REGION_NUM_RECTS(reg2)) return FALSE;
    
    rects1 = REGION_RECTS(reg1);
    rects2 = REGION_RECTS(reg2);
    for (i = 0; i != REGION_NUM_RECTS(reg1); i++) {
	if (rects1[i].x1 != rects2[i].x1) return FALSE;
	if (rects1[i].x2 != rects2[i].x2) return FALSE;
	if (rects1[i].y1 != rects2[i].y1) return FALSE;
	if (rects1[i].y2 != rects2[i].y2) return FALSE;
    }
    return TRUE;
}

/*
 * If the window is multibuffered and displaying the backbuffer,
 * add the old clipList to the subtractRgn and add the new clipList
 * to the unionRgn. PostValidateTree will use subtractRgn and unionRgn
 * to update the backBuffer region and the selectPlane.
 *
 * Copy changes to the window structure into the buffers.
 * Send ClobberNotify events.
 */

static void
bufClipNotify(pWin, dx,dy)
    WindowPtr pWin;
    int       dx,dy;
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    mbufBufferPrivPtr pMBPriv = MB_SCREEN_PRIV_BUFFER(pScreen);
    mbufWindowPtr	pMBWindow;
    int i;

    UNWRAP_SCREEN_FUNC(pScreen, pMBPriv, void, ClipNotify);
    if (pScreen->ClipNotify)
	(* pScreen->ClipNotify)(pWin, dx,dy);
    REWRAP_SCREEN_FUNC(pScreen, pMBPriv, void, ClipNotify);

    if (pMBWindow = MB_WINDOW_PRIV(pWin))
    {
	RegionPtr pOldClipList = (RegionPtr) pMBWindow->devPrivate.ptr;

	if (! RegionsEqual(pOldClipList, &pWin->clipList))
	{
	    if (pMBWindow->displayedMultibuffer == BACK_BUFFER)
	    {
		pMBPriv->rgnChanged = TRUE;
		REGION_APPEND(pScreen, &pMBPriv->subtractRgn, pOldClipList);
		REGION_APPEND(pScreen, &pMBPriv->unionRgn, &pWin->clipList);
	    }

	    REGION_COPY(pScreen, pOldClipList,&pWin->clipList);
	}

	/* Update buffer x,y,w,h, and clipList */
	for (i=0; i<pMBWindow->numMultibuffer; i++)
	{
	    mbufBufferPtr pMBBuffer = pMBWindow->buffers + i;
	    if (pMBBuffer->clobber != pWin->visibility)
	    {
		pMBBuffer->clobber = pWin->visibility;
		MultibufferClobber(pMBBuffer);
	    }
	    UpdateBufferFromWindow(pMBBuffer->pDrawable, pWin);
	}
    }
}

/*
 * Updates buffer's background fields when the window's changes.
 * This is necessary because pScreen->PaintWindowBackground
 * is used to paint the buffer.
 *
 * XXBS - Backingstore state will have be tracked too if it is supported.
 */

static Bool
bufChangeWindowAttributes(pWin, mask)
    WindowPtr pWin;
    unsigned long mask;
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    mbufBufferPrivPtr pMBPriv = MB_SCREEN_PRIV_BUFFER(pScreen);
    mbufWindowPtr pMBWindow;
    Bool ret;

    UNWRAP_SCREEN_FUNC(pScreen, pMBPriv, Bool, ChangeWindowAttributes);
    ret = (* pScreen->ChangeWindowAttributes)(pWin, mask);
    REWRAP_SCREEN_FUNC(pScreen, pMBPriv, Bool, ChangeWindowAttributes);

    if (pMBWindow = MB_WINDOW_PRIV(pWin))
    {
	if (mask & (CWBackPixmap | CWBackPixel))
	{
	    BufferPtr pBuffer;
	    int i;

	    for (i=0; i<pMBWindow->displayedMultibuffer; i++)
	    {
		pBuffer = (BufferPtr) pMBWindow->buffers[i].pDrawable;
		pBuffer->backgroundState = pWin->backgroundState;
		pBuffer->background = pWin->background;
	    }
	}
    }
    return ret;
}

/*
 * Send exposures and clear the background for a buffer whenever
 * its corresponding window is exposed, except when called by
 * ClearToBackground.
 */

static void 
bufWindowExposures(pWin, prgn, other_exposed)
    WindowPtr pWin;
    register RegionPtr prgn, other_exposed;
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    mbufWindowPtr pMBWindow = MB_WINDOW_PRIV(pWin);
    mbufBufferPrivPtr pMBPriv = MB_SCREEN_PRIV_BUFFER(pScreen);
    RegionRec tmp_rgn;
    int i;
    Bool handleBuffers;

    handleBuffers = (!pMBPriv->inClearToBackground) &&
	(pWin->drawable.type == DRAWABLE_WINDOW) &&
	pMBWindow && (prgn && !REGION_NIL(prgn));

    /* miWindowExposures munges prgn and other_exposed. */
    if (handleBuffers)
    {
	REGION_INIT(pScreen, &tmp_rgn, NullBox, 0);
	REGION_COPY(pScreen, &tmp_rgn,prgn);
    }

    UNWRAP_SCREEN_FUNC(pScreen, pMBPriv, void, WindowExposures);
    (* pScreen->WindowExposures) (pWin, prgn, other_exposed);
    REWRAP_SCREEN_FUNC(pScreen, pMBPriv, void, WindowExposures);

    if (!handleBuffers)
	return;

    /*
     * Send expose events to all clients. Paint the exposed region for all
     * buffers except the displayed buffer since it is handled when the
     * window is painted.
     *
     * XXBS - Will have to be re-written to handle BackingStore on buffers.
     */

    for (i=0; i<pMBWindow->numMultibuffer; i++)
    {
	mbufBufferPtr pMBBuffer;
	BufferPtr pBuffer;

	pMBBuffer = pMBWindow->buffers + i;
	pBuffer = (BufferPtr) pMBBuffer->pDrawable;

	if (i != pMBWindow->displayedMultibuffer)
	    (* pScreen->PaintWindowBackground)(pBuffer,&tmp_rgn,PW_BACKGROUND);
	if ((pMBBuffer->otherEventMask | pMBBuffer->eventMask) & ExposureMask)
	    MultibufferExpose(pMBBuffer, &tmp_rgn);
    }

    REGION_UNINIT(pScreen, &tmp_rgn);
}

/*
 * Set ``inClearToBackground'' so that WindowExposures does not attempt
 * to send expose events or clear the background on the buffers.
 */

static void
bufClearToBackground(pWin, x,y,w,h, sendExpose)
    WindowPtr pWin;
    int x,y, w,h;
    Bool sendExpose;
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    mbufBufferPrivPtr pMBPriv = MB_SCREEN_PRIV_BUFFER(pScreen);

    pMBPriv->inClearToBackground = TRUE;

    UNWRAP_SCREEN_FUNC(pScreen, pMBPriv, void, ClearToBackground);
    (* pScreen->ClearToBackground)(pWin, x,y,w,h, sendExpose);
    REWRAP_SCREEN_FUNC(pScreen, pMBPriv, void, ClearToBackground);

    pMBPriv->inClearToBackground = FALSE;
}

/*
 * Move bits in both buffers. It does this by calling pScreen->CopyWindow
 * twice, once with the root window's devPrivate[frameWindowPrivateIndex]
 * pointing to the frontbuffer pixmap and once with it pointed to the
 * backbuffer pixmap. It does this if there are *any* existing multibuffered
 * window... a possible optimization is to copy the backbuffer only if this
 * window or its inferiors are multibuffered. May be faster, maybe not.
 *
 * XXX - Only works if your CopyWindow checks the root window's devPrivate
 *       to see which buffer to draw into. Works for cfbPaintWindow.
 */

/*ARGSUSED*/
static void 
bufCopyWindow(pWin, ptOldOrg, prgnSrc)
    WindowPtr pWin;
    DDXPointRec ptOldOrg;
    RegionPtr prgnSrc;
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    mbufBufferPrivPtr pMBPriv = MB_SCREEN_PRIV_BUFFER(pScreen);
    WindowPtr pwinroot;
    DevUnion save;

    UNWRAP_SCREEN_FUNC(pScreen, pMBPriv, void, CopyWindow);

    pwinroot = WindowTable[pScreen->myNum];
    save = pwinroot->devPrivates[frameWindowPrivateIndex];

    /*
     * Copy front buffer
     */

    pwinroot->devPrivates[frameWindowPrivateIndex] =
	pMBPriv->frameBuffer[FRONT_BUFFER];
    (* pScreen->CopyWindow)(pWin, ptOldOrg, prgnSrc);

    /*
     * Copy back buffer
     */

    /* CopyWindow translates prgnSrc... translate it back for 2nd call. */
    REGION_TRANSLATE(pScreen, prgnSrc,
				  ptOldOrg.x - pWin->drawable.x,
				  ptOldOrg.y - pWin->drawable.y);
    pwinroot->devPrivates[frameWindowPrivateIndex] =
	pMBPriv->frameBuffer[BACK_BUFFER];
    (* pScreen->CopyWindow)(pWin, ptOldOrg, prgnSrc);

    pwinroot->devPrivates[frameWindowPrivateIndex] = save;
    REWRAP_SCREEN_FUNC(pScreen, pMBPriv, void, CopyWindow);
}
