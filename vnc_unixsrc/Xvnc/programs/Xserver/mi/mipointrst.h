/*
 * mipointrst.h
 *
 */

/* $XConsortium: mipointrst.h,v 5.7 94/04/17 20:27:41 dpw Exp $ */

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

# include   <mipointer.h>
# include   <input.h>

#define MOTION_SIZE	256

typedef struct {
    xTimecoord	    event;
    ScreenPtr	    pScreen;
} miHistoryRec, *miHistoryPtr;

typedef struct {
    ScreenPtr		    pScreen;    /* current screen */
    ScreenPtr		    pSpriteScreen;/* screen containing current sprite */
    CursorPtr		    pCursor;    /* current cursor */
    CursorPtr		    pSpriteCursor;/* cursor on screen */
    BoxRec		    limits;	/* current constraints */
    Bool		    confined;	/* pointer can't change screens */
    int			    x, y;	/* hot spot location */
    int			    devx, devy;	/* sprite position */
    DevicePtr		    pPointer;   /* pointer device structure */
    miHistoryRec	    history[MOTION_SIZE];
    int			    history_start, history_end;
} miPointerRec, *miPointerPtr;

typedef struct {
    miPointerSpriteFuncPtr  spriteFuncs;	/* sprite-specific methods */
    miPointerScreenFuncPtr  screenFuncs;	/* screen-specific methods */
    CloseScreenProcPtr	    CloseScreen;
    Bool		    waitForUpdate;	/* don't move cursor in SIGIO */
} miPointerScreenRec, *miPointerScreenPtr;
