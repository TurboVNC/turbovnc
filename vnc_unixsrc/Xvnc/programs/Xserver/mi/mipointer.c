/*
 * mipointer.c
 */

/* $XConsortium: mipointer.c,v 5.24 94/04/17 20:27:39 dpw Exp $ */
/* $XFree86: xc/programs/Xserver/mi/mipointer.c,v 3.1 1996/03/10 12:12:44 dawes Exp $ */

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

# define NEED_EVENTS
# include   "X.h"
# include   "Xmd.h"
# include   "Xproto.h"
# include   "misc.h"
# include   "windowstr.h"
# include   "pixmapstr.h"
# include   "mi.h"
# include   "scrnintstr.h"
# include   "mipointrst.h"
# include   "cursorstr.h"
# include   "dixstruct.h"

static int  miPointerScreenIndex;
static unsigned long miPointerGeneration = 0;

#define GetScreenPrivate(s) ((miPointerScreenPtr) ((s)->devPrivates[miPointerScreenIndex].ptr))
#define SetupScreen(s)	miPointerScreenPtr  pScreenPriv = GetScreenPrivate(s)

/*
 * until more than one pointer device exists.
 */

static miPointerRec miPointer;

static Bool miPointerRealizeCursor (),	    miPointerUnrealizeCursor ();
static Bool miPointerDisplayCursor ();
static void miPointerConstrainCursor (),    miPointerPointerNonInterestBox();
static void miPointerCursorLimits ();
static Bool miPointerSetCursorPosition ();

static Bool miPointerCloseScreen();

static void miPointerMove ();

Bool
miPointerInitialize (pScreen, spriteFuncs, screenFuncs, waitForUpdate)
    ScreenPtr		    pScreen;
    miPointerSpriteFuncPtr  spriteFuncs;
    miPointerScreenFuncPtr  screenFuncs;
    Bool		    waitForUpdate;
{
    miPointerScreenPtr	pScreenPriv;

    if (miPointerGeneration != serverGeneration)
    {
	miPointerScreenIndex = AllocateScreenPrivateIndex();
	if (miPointerScreenIndex < 0)
	    return FALSE;
	miPointerGeneration = serverGeneration;
    }
    pScreenPriv = (miPointerScreenPtr) xalloc (sizeof (miPointerScreenRec));
    if (!pScreenPriv)
	return FALSE;
    pScreenPriv->spriteFuncs = spriteFuncs;
    pScreenPriv->screenFuncs = screenFuncs;
    /*
     * check for uninitialized methods
     */
    if (!screenFuncs->EnqueueEvent)
	screenFuncs->EnqueueEvent = mieqEnqueue;
    if (!screenFuncs->NewEventScreen)
	screenFuncs->NewEventScreen = mieqSwitchScreen;
    pScreenPriv->waitForUpdate = waitForUpdate;
    pScreenPriv->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = miPointerCloseScreen;
    pScreen->devPrivates[miPointerScreenIndex].ptr = (pointer) pScreenPriv;
    /*
     * set up screen cursor method table
     */
    pScreen->ConstrainCursor = miPointerConstrainCursor;
    pScreen->CursorLimits = miPointerCursorLimits;
    pScreen->DisplayCursor = miPointerDisplayCursor;
    pScreen->RealizeCursor = miPointerRealizeCursor;
    pScreen->UnrealizeCursor = miPointerUnrealizeCursor;
    pScreen->SetCursorPosition = miPointerSetCursorPosition;
    pScreen->RecolorCursor = miRecolorCursor;
    pScreen->PointerNonInterestBox = miPointerPointerNonInterestBox;
    /*
     * set up the pointer object
     */
    miPointer.pScreen = NULL;
    miPointer.pSpriteScreen = NULL;
    miPointer.pCursor = NULL;
    miPointer.pSpriteCursor = NULL;
    miPointer.limits.x1 = 0;
    miPointer.limits.x2 = 32767;
    miPointer.limits.y1 = 0;
    miPointer.limits.y2 = 32767;
    miPointer.confined = FALSE;
    miPointer.x = 0;
    miPointer.y = 0;
    miPointer.history_start = miPointer.history_end = 0;
    return TRUE;
}

static Bool
miPointerCloseScreen (index, pScreen)
    int		index;
    ScreenPtr	pScreen;
{
    SetupScreen(pScreen);

    if (pScreen == miPointer.pScreen)
	miPointer.pScreen = 0;
    if (pScreen == miPointer.pSpriteScreen)
	miPointer.pSpriteScreen = 0;
    pScreen->CloseScreen = pScreenPriv->CloseScreen;
    xfree ((pointer) pScreenPriv);
    return (*pScreen->CloseScreen) (index, pScreen);
}

/*
 * DIX/DDX interface routines
 */

static Bool
miPointerRealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    SetupScreen(pScreen);

    return (*pScreenPriv->spriteFuncs->RealizeCursor) (pScreen, pCursor);
}

static Bool
miPointerUnrealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    SetupScreen(pScreen);

    return (*pScreenPriv->spriteFuncs->UnrealizeCursor) (pScreen, pCursor);
}

static Bool
miPointerDisplayCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    SetupScreen(pScreen);

    miPointer.pCursor = pCursor;
    miPointer.pScreen = pScreen;
    miPointerUpdate ();
    return TRUE;
}

static void
miPointerConstrainCursor (pScreen, pBox)
    ScreenPtr	pScreen;
    BoxPtr	pBox;
{
    miPointer.limits = *pBox;
    miPointer.confined = PointerConfinedToScreen();
}

/*ARGSUSED*/
static void
miPointerPointerNonInterestBox (pScreen, pBox)
    ScreenPtr	pScreen;
    BoxPtr	pBox;
{
    /* until DIX uses this, this will remain a stub */
}

/*ARGSUSED*/
static void
miPointerCursorLimits(pScreen, pCursor, pHotBox, pTopLeftBox)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
    BoxPtr	pHotBox;
    BoxPtr	pTopLeftBox;
{
    *pTopLeftBox = *pHotBox;
}

static Bool GenerateEvent;

static Bool
miPointerSetCursorPosition(pScreen, x, y, generateEvent)
    ScreenPtr pScreen;
    int       x, y;
    Bool      generateEvent;
{
    SetupScreen (pScreen);

    GenerateEvent = generateEvent;
    /* device dependent - must pend signal and call miPointerWarpCursor */
    (*pScreenPriv->screenFuncs->WarpCursor) (pScreen, x, y);
    if (!generateEvent)
	miPointerUpdate();
    return TRUE;
}

/* Once signals are ignored, the WarpCursor function can call this */

void
miPointerWarpCursor (pScreen, x, y)
    ScreenPtr	pScreen;
    int		x, y;
{
    SetupScreen (pScreen);

    if (miPointer.pScreen != pScreen)
	(*pScreenPriv->screenFuncs->NewEventScreen) (pScreen, TRUE);

    if (GenerateEvent)
    {
	miPointerMove (pScreen, x, y, GetTimeInMillis()); 
    }
    else
    {
	/* everything from miPointerMove except the event and history */

    	if (!pScreenPriv->waitForUpdate && pScreen == miPointer.pSpriteScreen)
    	{
	    miPointer.devx = x;
	    miPointer.devy = y;
	    (*pScreenPriv->spriteFuncs->MoveCursor) (pScreen, x, y);
    	}
	miPointer.x = x;
	miPointer.y = y;
	miPointer.pScreen = pScreen;
    }
}

/*
 * Pointer/CursorDisplay interface routines
 */

int
miPointerGetMotionBufferSize ()
{
    return MOTION_SIZE;
}

int
miPointerGetMotionEvents (pPtr, coords, start, stop, pScreen)
    DeviceIntPtr    pPtr;
    xTimecoord	    *coords;
    unsigned long   start, stop;
    ScreenPtr	    pScreen;
{
    int		    i;
    int		    count = 0;
    miHistoryPtr    h;

    for (i = miPointer.history_start; i != miPointer.history_end;)
    {
	h = &miPointer.history[i];
	if (h->event.time >= stop)
	    break;
	if (h->event.time >= start)
	{
	    *coords++ = h->event;
	    count++;
	}
	if (++i == MOTION_SIZE) i = 0;
    }
    return count;
}

    
/*
 * miPointerUpdate
 *
 * Syncronize the sprite with the cursor - called from ProcessInputEvents
 */

void
miPointerUpdate ()
{
    ScreenPtr		pScreen;
    miPointerScreenPtr	pScreenPriv;
    int			x, y, devx, devy;

    pScreen = miPointer.pScreen;
    x = miPointer.x;
    y = miPointer.y;
    devx = miPointer.devx;
    devy = miPointer.devy;
    if (!pScreen)
	return;
    pScreenPriv = GetScreenPrivate (pScreen);
    /*
     * if the cursor has switched screens, disable the sprite
     * on the old screen
     */
    if (pScreen != miPointer.pSpriteScreen)
    {
	if (miPointer.pSpriteScreen)
	{
	    miPointerScreenPtr  pOldPriv;
    	
	    pOldPriv = GetScreenPrivate (miPointer.pSpriteScreen);
	    if (miPointer.pCursor)
	    {
	    	(*pOldPriv->spriteFuncs->SetCursor)
			    	(miPointer.pSpriteScreen, NullCursor, 0, 0);
	    }
	    (*pOldPriv->screenFuncs->CrossScreen) (miPointer.pSpriteScreen, FALSE);
	}
	(*pScreenPriv->screenFuncs->CrossScreen) (pScreen, TRUE);
	(*pScreenPriv->spriteFuncs->SetCursor)
				(pScreen, miPointer.pCursor, x, y);
	miPointer.devx = x;
	miPointer.devy = y;
	miPointer.pSpriteCursor = miPointer.pCursor;
	miPointer.pSpriteScreen = pScreen;
    }
    /*
     * if the cursor has changed, display the new one
     */
    else if (miPointer.pCursor != miPointer.pSpriteCursor)
    {
	(*pScreenPriv->spriteFuncs->SetCursor) 
	    (pScreen, miPointer.pCursor, x, y);
	miPointer.devx = x;
	miPointer.devy = y;
	miPointer.pSpriteCursor = miPointer.pCursor;
    }
    else if (x != devx || y != devy)
    {
	miPointer.devx = x;
	miPointer.devy = y;
	(*pScreenPriv->spriteFuncs->MoveCursor) (pScreen, x, y);
    }
}

/*
 * miPointerDeltaCursor.  The pointer has moved dx,dy from it's previous
 * position.
 */

void
miPointerDeltaCursor (dx, dy, time)
    int		    dx, dy;
    unsigned long   time;
{
    miPointerAbsoluteCursor (miPointer.x + dx, miPointer.y + dy, time);
}

/*
 * miPointerAbsoluteCursor.  The pointer has moved to x,y
 */

void
miPointerAbsoluteCursor (x, y, time)
    int		    x, y;
    unsigned long   time;
{
    miPointerScreenPtr	pScreenPriv;
    ScreenPtr		pScreen;
    ScreenPtr		newScreen;

    pScreen = miPointer.pScreen;
    if (!pScreen)
	return;	    /* called before ready */
    if (x < 0 || x >= pScreen->width || y < 0 || y >= pScreen->height)
    {
	pScreenPriv = GetScreenPrivate (pScreen);
	if (!miPointer.confined)
	{
	    newScreen = pScreen;
	    (*pScreenPriv->screenFuncs->CursorOffScreen) (&newScreen, &x, &y);
	    if (newScreen != pScreen)
	    {
		pScreen = newScreen;
		(*pScreenPriv->screenFuncs->NewEventScreen) (pScreen, FALSE);
		pScreenPriv = GetScreenPrivate (pScreen);
	    	/* Smash the confine to the new screen */
	    	miPointer.limits.x2 = pScreen->width;
	    	miPointer.limits.y2 = pScreen->height;
	    }
	}
    }
    /*
     * constrain the hot-spot to the current
     * limits
     */
    if (x < miPointer.limits.x1)
	x = miPointer.limits.x1;
    if (x >= miPointer.limits.x2)
	x = miPointer.limits.x2 - 1;
    if (y < miPointer.limits.y1)
	y = miPointer.limits.y1;
    if (y >= miPointer.limits.y2)
	y = miPointer.limits.y2 - 1;
    if (miPointer.x == x && miPointer.y == y && miPointer.pScreen == pScreen)
	return;
    miPointerMove (pScreen, x, y, time);
}

void
miPointerPosition (x, y)
    int	    *x, *y;
{
    *x = miPointer.x;
    *y = miPointer.y;
}

/*
 * miPointerMove.  The pointer has moved to x,y on current screen
 */

static void
miPointerMove (pScreen, x, y, time)
    ScreenPtr	    pScreen;
    int		    x, y;
    unsigned long   time;
{
    SetupScreen(pScreen);
    xEvent		xE;
    miHistoryPtr	history;
    int			prev, end, start;

    if (!pScreenPriv->waitForUpdate && pScreen == miPointer.pSpriteScreen)
    {
	miPointer.devx = x;
	miPointer.devy = y;
	(*pScreenPriv->spriteFuncs->MoveCursor) (pScreen, x, y);
    }
    miPointer.x = x;
    miPointer.y = y;
    miPointer.pScreen = pScreen;

    xE.u.u.type = MotionNotify;
    xE.u.keyButtonPointer.rootX = x;
    xE.u.keyButtonPointer.rootY = y;
    xE.u.keyButtonPointer.time = time;
    (*pScreenPriv->screenFuncs->EnqueueEvent) (&xE);

    end = miPointer.history_end;
    start = miPointer.history_start;
    prev = end - 1;
    if (end == 0)
	prev = MOTION_SIZE - 1;
    history = &miPointer.history[prev];
    if (end == start || history->event.time != time)
    {
    	history = &miPointer.history[end];
    	if (++end == MOTION_SIZE) 
	    end = 0;
    	if (end == start)
    	{
	    start = end + 1;
	    if (start == MOTION_SIZE)
	    	start = 0;
	    miPointer.history_start = start;
    	}
    	miPointer.history_end = end;
    }
    history->event.x = x;
    history->event.y = y;
    history->event.time = time;
    history->pScreen = pScreen;
}

void
_miRegisterPointerDevice (pScreen, pDevice)
    ScreenPtr	pScreen;
    DeviceIntPtr pDevice;
{
    miPointer.pPointer = (DevicePtr)pDevice;
}

/* obsolete: for binary compatibility */
#ifdef miRegisterPointerDevice
#undef miRegisterPointerDevice
void
miRegisterPointerDevice (pScreen, pDevice)
    ScreenPtr	pScreen;
    DevicePtr pDevice;
{
    miPointer.pPointer = pDevice;
}
#endif /* miRegisterPointerDevice */
