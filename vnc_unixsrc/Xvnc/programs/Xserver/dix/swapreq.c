/* $XFree86: xc/programs/Xserver/dix/swapreq.c,v 3.4 2001/12/14 19:59:33 dawes Exp $ */
/************************************************************

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

********************************************************/

/* $Xorg: swapreq.c,v 1.4 2001/02/09 02:04:41 xorgcvs Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#define NEED_EVENTS
#include <X11/Xproto.h>
#include <X11/Xprotostr.h>
#include "misc.h"
#include "dixstruct.h"
#include "extnsionst.h"	/* for SendEvent */
#include "swapreq.h"

/* Thanks to Jack Palevich for testing and subsequently rewriting all this */

/* Byte swap a list of longs */

void
SwapLongs (register CARD32 *list, register unsigned long count)
{
	register char n;

	while (count >= 8) {
	    swapl(list+0, n);
	    swapl(list+1, n);
	    swapl(list+2, n);
	    swapl(list+3, n);
	    swapl(list+4, n);
	    swapl(list+5, n);
	    swapl(list+6, n);
	    swapl(list+7, n);
	    list += 8;
	    count -= 8;
	}
	if (count != 0) {
	    do {
		swapl(list, n);
		list++;
	    } while (--count != 0);
	}
}

/* Byte swap a list of shorts */

void
SwapShorts (register short *list, register unsigned long count)
{
	register char n;

	while (count >= 16) {
	    swaps(list+0, n);
	    swaps(list+1, n);
	    swaps(list+2, n);
	    swaps(list+3, n);
	    swaps(list+4, n);
	    swaps(list+5, n);
	    swaps(list+6, n);
	    swaps(list+7, n);
	    swaps(list+8, n);
	    swaps(list+9, n);
	    swaps(list+10, n);
	    swaps(list+11, n);
	    swaps(list+12, n);
	    swaps(list+13, n);
	    swaps(list+14, n);
	    swaps(list+15, n);
	    list += 16;
	    count -= 16;
	}
	if (count != 0) {
	    do {
		swaps(list, n);
		list++;
	    } while (--count != 0);
	}
}

/* The following is used for all requests that have
   no fields to be swapped (except "length") */
int
SProcSimpleReq(register ClientPtr client)
{
    register char n;

    REQUEST(xReq);
    swaps(&stuff->length, n);
    return(*ProcVector[stuff->reqType])(client);
}

/* The following is used for all requests that have
   only a single 32-bit field to be swapped, coming
   right after the "length" field */
int
SProcResourceReq(register ClientPtr client)
{
    register char n;

    REQUEST(xResourceReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xResourceReq); /* not EXACT */
    swapl(&stuff->id, n);
    return(*ProcVector[stuff->reqType])(client);
}

int
SProcCreateWindow(register ClientPtr client)
{
    register char n;

    REQUEST(xCreateWindowReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xCreateWindowReq);
    swapl(&stuff->wid, n);
    swapl(&stuff->parent, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    swaps(&stuff->borderWidth, n);
    swaps(&stuff->class, n);
    swapl(&stuff->visual, n);
    swapl(&stuff->mask, n);
    SwapRestL(stuff);
    return((* ProcVector[X_CreateWindow])(client));
}

int
SProcChangeWindowAttributes(register ClientPtr client)
{
    register char n;

    REQUEST(xChangeWindowAttributesReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xChangeWindowAttributesReq);
    swapl(&stuff->window, n);
    swapl(&stuff->valueMask, n);
    SwapRestL(stuff);
    return((* ProcVector[X_ChangeWindowAttributes])(client));
}

int
SProcReparentWindow(register ClientPtr client)
{
    register char n;
    REQUEST(xReparentWindowReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xReparentWindowReq);
    swapl(&stuff->window, n);
    swapl(&stuff->parent, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    return((* ProcVector[X_ReparentWindow])(client));
}

int
SProcConfigureWindow(register ClientPtr client)
{
    register char n;
    REQUEST(xConfigureWindowReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xConfigureWindowReq);
    swapl(&stuff->window, n);
    swaps(&stuff->mask, n);
    SwapRestL(stuff);
    return((* ProcVector[X_ConfigureWindow])(client));

}


int
SProcInternAtom(register ClientPtr client)
{
    register char n;
    REQUEST(xInternAtomReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xInternAtomReq);
    swaps(&stuff->nbytes, n);
    return((* ProcVector[X_InternAtom])(client));
}

int
SProcChangeProperty(register ClientPtr client)
{
    register char n;
    REQUEST(xChangePropertyReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xChangePropertyReq);
    swapl(&stuff->window, n);
    swapl(&stuff->property, n);
    swapl(&stuff->type, n);
    swapl(&stuff->nUnits, n);
    switch ( stuff->format ) {
        case 8 :
	    break;
        case 16:
	    SwapRestS(stuff);
	    break;
	case 32:
	    SwapRestL(stuff);
	    break;
	}
    return((* ProcVector[X_ChangeProperty])(client));
}

int 
SProcDeleteProperty(register ClientPtr client)
{
    register char n;
    REQUEST(xDeletePropertyReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDeletePropertyReq);
    swapl(&stuff->window, n);
    swapl(&stuff->property, n);
    return((* ProcVector[X_DeleteProperty])(client));
              
}

int 
SProcGetProperty(register ClientPtr client)
{
    register char n;
    REQUEST(xGetPropertyReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xGetPropertyReq);
    swapl(&stuff->window, n);
    swapl(&stuff->property, n);
    swapl(&stuff->type, n);
    swapl(&stuff->longOffset, n);
    swapl(&stuff->longLength, n);
    return((* ProcVector[X_GetProperty])(client));
}

int
SProcSetSelectionOwner(register ClientPtr client)
{
    register char n;
    REQUEST(xSetSelectionOwnerReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xSetSelectionOwnerReq);
    swapl(&stuff->window, n);
    swapl(&stuff->selection, n);
    swapl(&stuff->time, n);
    return((* ProcVector[X_SetSelectionOwner])(client));
}

int
SProcConvertSelection(register ClientPtr client)
{
    register char n;
    REQUEST(xConvertSelectionReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xConvertSelectionReq);
    swapl(&stuff->requestor, n);
    swapl(&stuff->selection, n);
    swapl(&stuff->target, n);
    swapl(&stuff->property, n);
    swapl(&stuff->time, n);
    return((* ProcVector[X_ConvertSelection])(client));
}

int
SProcSendEvent(register ClientPtr client)
{
    register char n;
    xEvent eventT;
    EventSwapPtr proc;
    REQUEST(xSendEventReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xSendEventReq);
    swapl(&stuff->destination, n);
    swapl(&stuff->eventMask, n);

    /* Swap event */
    proc = EventSwapVector[stuff->event.u.u.type & 0177];
    if (!proc ||  proc == NotImplemented)    /* no swapping proc; invalid event type? */
       return (BadValue);
    (*proc)(&stuff->event, &eventT);
    stuff->event = eventT;

    return((* ProcVector[X_SendEvent])(client));
}

int
SProcGrabPointer(register ClientPtr client)
{
    register char n;
    REQUEST(xGrabPointerReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xGrabPointerReq);
    swapl(&stuff->grabWindow, n);
    swaps(&stuff->eventMask, n);
    swapl(&stuff->confineTo, n);
    swapl(&stuff->cursor, n);
    swapl(&stuff->time, n);
    return((* ProcVector[X_GrabPointer])(client));
}

int
SProcGrabButton(register ClientPtr client)
{
    register char n;
    REQUEST(xGrabButtonReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xGrabButtonReq);
    swapl(&stuff->grabWindow, n);
    swaps(&stuff->eventMask, n);
    swapl(&stuff->confineTo, n);
    swapl(&stuff->cursor, n);
    swaps(&stuff->modifiers, n);
    return((* ProcVector[X_GrabButton])(client));
}

int
SProcUngrabButton(register ClientPtr client)
{
    register char n;
    REQUEST(xUngrabButtonReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xUngrabButtonReq);
    swapl(&stuff->grabWindow, n);
    swaps(&stuff->modifiers, n);
    return((* ProcVector[X_UngrabButton])(client));
}

int
SProcChangeActivePointerGrab(register ClientPtr client)
{
    register char n;
    REQUEST(xChangeActivePointerGrabReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xChangeActivePointerGrabReq);
    swapl(&stuff->cursor, n);
    swapl(&stuff->time, n);
    swaps(&stuff->eventMask, n);
    return((* ProcVector[X_ChangeActivePointerGrab])(client));
}

int
SProcGrabKeyboard(register ClientPtr client)
{
    register char n;
    REQUEST(xGrabKeyboardReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xGrabKeyboardReq);
    swapl(&stuff->grabWindow, n);
    swapl(&stuff->time, n);
    return((* ProcVector[X_GrabKeyboard])(client));
}

int
SProcGrabKey(register ClientPtr client)
{
    register char n;
    REQUEST(xGrabKeyReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xGrabKeyReq);
    swapl(&stuff->grabWindow, n);
    swaps(&stuff->modifiers, n);
    return((* ProcVector[X_GrabKey])(client));
}

int
SProcUngrabKey(register ClientPtr client)
{
    register char n;
    REQUEST(xUngrabKeyReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xUngrabKeyReq);
    swapl(&stuff->grabWindow, n);
    swaps(&stuff->modifiers, n);
    return((* ProcVector[X_UngrabKey])(client));
}

int
SProcGetMotionEvents(register ClientPtr client)
{
    register char n;
    REQUEST(xGetMotionEventsReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xGetMotionEventsReq);
    swapl(&stuff->window, n);
    swapl(&stuff->start, n);
    swapl(&stuff->stop, n);
    return((* ProcVector[X_GetMotionEvents])(client));
}

int
SProcTranslateCoords(register ClientPtr client)
{
    register char n;
    REQUEST(xTranslateCoordsReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xTranslateCoordsReq);
    swapl(&stuff->srcWid, n);
    swapl(&stuff->dstWid, n);
    swaps(&stuff->srcX, n);
    swaps(&stuff->srcY, n);
    return((* ProcVector[X_TranslateCoords])(client));
}

int
SProcWarpPointer(register ClientPtr client)
{
    register char n;
    REQUEST(xWarpPointerReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xWarpPointerReq);
    swapl(&stuff->srcWid, n);
    swapl(&stuff->dstWid, n);
    swaps(&stuff->srcX, n);
    swaps(&stuff->srcY, n);
    swaps(&stuff->srcWidth, n);
    swaps(&stuff->srcHeight, n);
    swaps(&stuff->dstX, n);
    swaps(&stuff->dstY, n);
    return((* ProcVector[X_WarpPointer])(client));
}

int
SProcSetInputFocus(register ClientPtr client)
{
    register char n;
    REQUEST(xSetInputFocusReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xSetInputFocusReq);
    swapl(&stuff->focus, n);
    swapl(&stuff->time, n);
    return((* ProcVector[X_SetInputFocus])(client));
}

int
SProcOpenFont(register ClientPtr client)
{
    register char n;
    REQUEST(xOpenFontReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xOpenFontReq);
    swapl(&stuff->fid, n);
    swaps(&stuff->nbytes, n);
    return((* ProcVector[X_OpenFont])(client));
}

int
SProcListFonts(register ClientPtr client)
{
    register char n;
    REQUEST(xListFontsReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xListFontsReq);
    swaps(&stuff->maxNames, n);
    swaps(&stuff->nbytes, n);
    return((* ProcVector[X_ListFonts])(client));
}

int
SProcListFontsWithInfo(register ClientPtr client)
{
    register char n;
    REQUEST(xListFontsWithInfoReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xListFontsWithInfoReq);
    swaps(&stuff->maxNames, n);
    swaps(&stuff->nbytes, n);
    return((* ProcVector[X_ListFontsWithInfo])(client));
}

int
SProcSetFontPath(register ClientPtr client)
{
    register char n;
    REQUEST(xSetFontPathReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xSetFontPathReq);
    swaps(&stuff->nFonts, n);
    return((* ProcVector[X_SetFontPath])(client));
}

int
SProcCreatePixmap(register ClientPtr client)
{
    register char n;
    REQUEST(xCreatePixmapReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xCreatePixmapReq);
    swapl(&stuff->pid, n);
    swapl(&stuff->drawable, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    return((* ProcVector[X_CreatePixmap])(client));
}

int
SProcCreateGC(register ClientPtr client)
{
    register char n;
    REQUEST(xCreateGCReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xCreateGCReq);
    swapl(&stuff->gc, n);
    swapl(&stuff->drawable, n);
    swapl(&stuff->mask, n);
    SwapRestL(stuff);
    return((* ProcVector[X_CreateGC])(client));
}

int
SProcChangeGC(register ClientPtr client)
{
    register char n;
    REQUEST(xChangeGCReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xChangeGCReq);
    swapl(&stuff->gc, n);
    swapl(&stuff->mask, n);
    SwapRestL(stuff);
    return((* ProcVector[X_ChangeGC])(client));
}

int
SProcCopyGC(register ClientPtr client)
{
    register char n;
    REQUEST(xCopyGCReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xCopyGCReq);
    swapl(&stuff->srcGC, n);
    swapl(&stuff->dstGC, n);
    swapl(&stuff->mask, n);
    return((* ProcVector[X_CopyGC])(client));
}

int
SProcSetDashes(register ClientPtr client)
{
    register char n;
    REQUEST(xSetDashesReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xSetDashesReq);
    swapl(&stuff->gc, n);
    swaps(&stuff->dashOffset, n);
    swaps(&stuff->nDashes, n);
    return((* ProcVector[X_SetDashes])(client));

}

int
SProcSetClipRectangles(register ClientPtr client)
{
    register char n;
    REQUEST(xSetClipRectanglesReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xSetClipRectanglesReq);
    swapl(&stuff->gc, n);
    swaps(&stuff->xOrigin, n);
    swaps(&stuff->yOrigin, n);
    SwapRestS(stuff);
    return((* ProcVector[X_SetClipRectangles])(client));
}

int
SProcClearToBackground(register ClientPtr client)
{
    register char n;
    REQUEST(xClearAreaReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xClearAreaReq);
    swapl(&stuff->window, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    return((* ProcVector[X_ClearArea])(client));
}

int
SProcCopyArea(register ClientPtr client)
{
    register char n;
    REQUEST(xCopyAreaReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xCopyAreaReq);
    swapl(&stuff->srcDrawable, n);
    swapl(&stuff->dstDrawable, n);
    swapl(&stuff->gc, n);
    swaps(&stuff->srcX, n);
    swaps(&stuff->srcY, n);
    swaps(&stuff->dstX, n);
    swaps(&stuff->dstY, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    return((* ProcVector[X_CopyArea])(client));
}

int
SProcCopyPlane(register ClientPtr client)
{
    register char n;
    REQUEST(xCopyPlaneReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xCopyPlaneReq);
    swapl(&stuff->srcDrawable, n);
    swapl(&stuff->dstDrawable, n);
    swapl(&stuff->gc, n);
    swaps(&stuff->srcX, n);
    swaps(&stuff->srcY, n);
    swaps(&stuff->dstX, n);
    swaps(&stuff->dstY, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    swapl(&stuff->bitPlane, n);
    return((* ProcVector[X_CopyPlane])(client));
}

/* The following routine is used for all Poly drawing requests
   (except FillPoly, which uses a different request format) */
int
SProcPoly(register ClientPtr client)
{
    register char n;

    REQUEST(xPolyPointReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xPolyPointReq);
    swapl(&stuff->drawable, n);
    swapl(&stuff->gc, n);
    SwapRestS(stuff);
    return((* ProcVector[stuff->reqType])(client));
}

/* cannot use SProcPoly for this one, because xFillPolyReq
   is longer than xPolyPointReq, and we don't want to swap
   the difference as shorts! */
int
SProcFillPoly(register ClientPtr client)
{
    register char n;

    REQUEST(xFillPolyReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xFillPolyReq);
    swapl(&stuff->drawable, n);
    swapl(&stuff->gc, n);
    SwapRestS(stuff);
    return((* ProcVector[X_FillPoly])(client));
}

int
SProcPutImage(register ClientPtr client)
{
    register char n;
    REQUEST(xPutImageReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xPutImageReq);
    swapl(&stuff->drawable, n);
    swapl(&stuff->gc, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    swaps(&stuff->dstX, n);
    swaps(&stuff->dstY, n);
    /* Image should already be swapped */
    return((* ProcVector[X_PutImage])(client));

}

int
SProcGetImage(register ClientPtr client)
{
    register char n;
    REQUEST(xGetImageReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xGetImageReq);
    swapl(&stuff->drawable, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    swapl(&stuff->planeMask, n);
    return((* ProcVector[X_GetImage])(client));
}

/* ProcPolyText used for both PolyText8 and PolyText16 */

int
SProcPolyText(register ClientPtr client)
{
    register char n;
    REQUEST(xPolyTextReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xPolyTextReq);
    swapl(&stuff->drawable, n);
    swapl(&stuff->gc, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    return((* ProcVector[stuff->reqType])(client));
}

/* ProcImageText used for both ImageText8 and ImageText16 */

int
SProcImageText(register ClientPtr client)
{
    register char n;
    REQUEST(xImageTextReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xImageTextReq);
    swapl(&stuff->drawable, n);
    swapl(&stuff->gc, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    return((* ProcVector[stuff->reqType])(client));
}

int
SProcCreateColormap(register ClientPtr client)
{
    register char n;
    REQUEST(xCreateColormapReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xCreateColormapReq);
    swapl(&stuff->mid, n);
    swapl(&stuff->window, n);
    swapl(&stuff->visual, n);
    return((* ProcVector[X_CreateColormap])(client));
}


int
SProcCopyColormapAndFree(register ClientPtr client)
{
    register char n;
    REQUEST(xCopyColormapAndFreeReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xCopyColormapAndFreeReq);
    swapl(&stuff->mid, n);
    swapl(&stuff->srcCmap, n);
    return((* ProcVector[X_CopyColormapAndFree])(client));

}

int
SProcAllocColor(register ClientPtr client)
{
    register char n;
    REQUEST(xAllocColorReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xAllocColorReq);
    swapl(&stuff->cmap, n);
    swaps(&stuff->red, n);
    swaps(&stuff->green, n);
    swaps(&stuff->blue, n);
    return((* ProcVector[X_AllocColor])(client));
}

int
SProcAllocNamedColor(register ClientPtr client)
{
    register char n;

    REQUEST(xAllocNamedColorReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xAllocNamedColorReq);
    swapl(&stuff->cmap, n);
    swaps(&stuff->nbytes, n);
    return((* ProcVector[X_AllocNamedColor])(client));
}

int
SProcAllocColorCells(register ClientPtr client)
{
    register char n;
    REQUEST(xAllocColorCellsReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xAllocColorCellsReq);
    swapl(&stuff->cmap, n);
    swaps(&stuff->colors, n);
    swaps(&stuff->planes, n);
    return((* ProcVector[X_AllocColorCells])(client));
}

int
SProcAllocColorPlanes(register ClientPtr client)
{
    register char n;
    REQUEST(xAllocColorPlanesReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xAllocColorPlanesReq);
    swapl(&stuff->cmap, n);
    swaps(&stuff->colors, n);
    swaps(&stuff->red, n);
    swaps(&stuff->green, n);
    swaps(&stuff->blue, n);
    return((* ProcVector[X_AllocColorPlanes])(client));
}

int
SProcFreeColors(register ClientPtr client)
{
    register char n;
    REQUEST(xFreeColorsReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xFreeColorsReq);
    swapl(&stuff->cmap, n);
    swapl(&stuff->planeMask, n);
    SwapRestL(stuff);
    return((* ProcVector[X_FreeColors])(client));

}

void
SwapColorItem(xColorItem *pItem)
{
    register char n;

    swapl(&pItem->pixel, n);
    swaps(&pItem->red, n);
    swaps(&pItem->green, n);
    swaps(&pItem->blue, n);
}

int
SProcStoreColors(register ClientPtr client)
{
    register char n;
    long count;
    xColorItem 	*pItem;

    REQUEST(xStoreColorsReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xStoreColorsReq);
    swapl(&stuff->cmap, n);
    pItem = (xColorItem *) &stuff[1];
    for(count = LengthRestB(stuff)/sizeof(xColorItem); --count >= 0; )
	SwapColorItem(pItem++);
    return((* ProcVector[X_StoreColors])(client));
}

int
SProcStoreNamedColor (register ClientPtr client)
{
    register char n;
    REQUEST(xStoreNamedColorReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xStoreNamedColorReq);
    swapl(&stuff->cmap, n);
    swapl(&stuff->pixel, n);
    swaps(&stuff->nbytes, n);
    return((* ProcVector[X_StoreNamedColor])(client));
}

int
SProcQueryColors (register ClientPtr client)
{
    register char n;
    REQUEST(xQueryColorsReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xQueryColorsReq);
    swapl(&stuff->cmap, n);
    SwapRestL(stuff);
    return((* ProcVector[X_QueryColors])(client));
} 

int
SProcLookupColor (register ClientPtr client)
{
    register char n;
    REQUEST(xLookupColorReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xLookupColorReq);
    swapl(&stuff->cmap, n);
    swaps(&stuff->nbytes, n);
    return((* ProcVector[X_LookupColor])(client));
}

int
SProcCreateCursor (register ClientPtr client)
{
    register char n;
    REQUEST(xCreateCursorReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xCreateCursorReq);
    swapl(&stuff->cid, n);
    swapl(&stuff->source, n);
    swapl(&stuff->mask, n);
    swaps(&stuff->foreRed, n);
    swaps(&stuff->foreGreen, n);
    swaps(&stuff->foreBlue, n);
    swaps(&stuff->backRed, n);
    swaps(&stuff->backGreen, n);
    swaps(&stuff->backBlue, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    return((* ProcVector[X_CreateCursor])(client));
}

int
SProcCreateGlyphCursor (register ClientPtr client)
{
    register char n;
    REQUEST(xCreateGlyphCursorReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xCreateGlyphCursorReq);
    swapl(&stuff->cid, n);
    swapl(&stuff->source, n);
    swapl(&stuff->mask, n);
    swaps(&stuff->sourceChar, n);
    swaps(&stuff->maskChar, n);
    swaps(&stuff->foreRed, n);
    swaps(&stuff->foreGreen, n);
    swaps(&stuff->foreBlue, n);
    swaps(&stuff->backRed, n);
    swaps(&stuff->backGreen, n);
    swaps(&stuff->backBlue, n);
    return((* ProcVector[X_CreateGlyphCursor])(client));
}


int
SProcRecolorCursor (register ClientPtr client)
{
    register char n;
    REQUEST(xRecolorCursorReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xRecolorCursorReq);
    swapl(&stuff->cursor, n);
    swaps(&stuff->foreRed, n);
    swaps(&stuff->foreGreen, n);
    swaps(&stuff->foreBlue, n);
    swaps(&stuff->backRed, n);
    swaps(&stuff->backGreen, n);
    swaps(&stuff->backBlue, n);
    return((* ProcVector[X_RecolorCursor])(client));
}

int
SProcQueryBestSize (register ClientPtr client)
{
    register char n;
    REQUEST(xQueryBestSizeReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xQueryBestSizeReq);
    swapl(&stuff->drawable, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    return((* ProcVector[X_QueryBestSize])(client));

}

int
SProcQueryExtension (register ClientPtr client)
{
    register char n;
    REQUEST(xQueryExtensionReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xQueryExtensionReq);
    swaps(&stuff->nbytes, n);
    return((* ProcVector[X_QueryExtension])(client));
}

int
SProcChangeKeyboardMapping (register ClientPtr client)
{
    register char n;
    REQUEST(xChangeKeyboardMappingReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xChangeKeyboardMappingReq);
    SwapRestL(stuff);
    return((* ProcVector[X_ChangeKeyboardMapping])(client));
}


int
SProcChangeKeyboardControl (register ClientPtr client)
{
    register char n;
    REQUEST(xChangeKeyboardControlReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xChangeKeyboardControlReq);
    swapl(&stuff->mask, n);
    SwapRestL(stuff);
    return((* ProcVector[X_ChangeKeyboardControl])(client));
}

int
SProcChangePointerControl (register ClientPtr client)
{
    register char n;
    REQUEST(xChangePointerControlReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xChangePointerControlReq);
    swaps(&stuff->accelNum, n);
    swaps(&stuff->accelDenum, n);
    swaps(&stuff->threshold, n);
    return((* ProcVector[X_ChangePointerControl])(client));
}


int
SProcSetScreenSaver (register ClientPtr client)
{
    register char n;
    REQUEST(xSetScreenSaverReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xSetScreenSaverReq);
    swaps(&stuff->timeout, n);
    swaps(&stuff->interval, n);
    return((* ProcVector[X_SetScreenSaver])(client));
}

int
SProcChangeHosts (register ClientPtr client)
{
    register char n;

    REQUEST(xChangeHostsReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xChangeHostsReq);
    swaps(&stuff->hostLength, n);
    return((* ProcVector[X_ChangeHosts])(client));

}

int SProcRotateProperties (register ClientPtr client)
{
    register char n;
    REQUEST(xRotatePropertiesReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xRotatePropertiesReq);
    swapl(&stuff->window, n);
    swaps(&stuff->nAtoms, n);
    swaps(&stuff->nPositions, n);
    SwapRestL(stuff);
    return ((* ProcVector[X_RotateProperties])(client));
}

int
SProcNoOperation(register ClientPtr client)
{
    register char n;
    REQUEST(xReq);
    swaps(&stuff->length, n);
    return ((* ProcVector[X_NoOperation])(client));
}

void
SwapConnClientPrefix(xConnClientPrefix *pCCP)
{
    register char n;

    swaps(&pCCP->majorVersion, n);
    swaps(&pCCP->minorVersion, n);
    swaps(&pCCP->nbytesAuthProto, n);
    swaps(&pCCP->nbytesAuthString, n);
}
