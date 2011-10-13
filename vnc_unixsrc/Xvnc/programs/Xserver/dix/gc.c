/* $XFree86: xc/programs/Xserver/dix/gc.c,v 3.9 2001/12/14 19:59:32 dawes Exp $ */
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

/* $Xorg: gc.c,v 1.4 2001/02/09 02:04:40 xorgcvs Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "resource.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "dixfontstr.h"
#include "scrnintstr.h"
#include "region.h"

#include "dix.h"
#include <assert.h>

extern XID clientErrorValue;
extern FontPtr defaultFont;

static Bool CreateDefaultTile(GCPtr pGC);

unsigned char DefaultDash[2] = {4, 4};

void
ValidateGC(DrawablePtr pDraw, GC *pGC)
{
    (*pGC->funcs->ValidateGC) (pGC, pGC->stateChanges, pDraw);
    pGC->stateChanges = 0;
    pGC->serialNumber = pDraw->serialNumber;
}


/* dixChangeGC(client, pGC, mask, pC32, pUnion)
 * 
 * This function was created as part of the Security extension
 * implementation.  The client performing the gc change must be passed so
 * that access checks can be performed on any tiles, stipples, or fonts
 * that are specified.  ddxen can call this too; they should normally
 * pass NullClient for the client since any access checking should have
 * already been done at a higher level.
 * 
 * Since we had to create a new function anyway, we decided to change the
 * way the list of gc values is passed to eliminate the compiler warnings
 * caused by the DoChangeGC interface.  You can pass the values via pC32
 * or pUnion, but not both; one of them must be NULL.  If you don't need
 * to pass any pointers, you can use either one:
 * 
 *     example calling dixChangeGC using pC32 parameter
 *
 *     CARD32 v[2];
 *     v[0] = foreground;
 *     v[1] = background;
 *     dixChangeGC(client, pGC, GCForeground|GCBackground, v, NULL);
 * 
 *     example calling dixChangeGC using pUnion parameter;
 *     same effect as above
 *
 *     ChangeGCVal v[2];
 *     v[0].val = foreground;
 *     v[1].val = background;
 *     dixChangeGC(client, pGC, GCForeground|GCBackground, NULL, v);
 * 
 * However, if you need to pass a pointer to a pixmap or font, you MUST
 * use the pUnion parameter.
 * 
 *     example calling dixChangeGC passing pointers in the value list
 *     v[1].ptr is a pointer to a pixmap
 *
 *     ChangeGCVal v[2];
 *     v[0].val = FillTiled;
 *     v[1].ptr = pPixmap;
 *     dixChangeGC(client, pGC, GCFillStyle|GCTile, NULL, v);
 * 
 * Note: we could have gotten by with just the pUnion parameter, but on
 * 64 bit machines that would have forced us to copy the value list that
 * comes in the ChangeGC request.
 * 
 * Ideally, we'd change all the DoChangeGC calls to dixChangeGC, but this
 * is far too many changes to consider at this time, so we've only
 * changed the ones that caused compiler warnings.  New code should use
 * dixChangeGC.
 * 
 * dpw
 */

#define NEXTVAL(_type, _var) { \
      if (pC32) _var = (_type)*pC32++; \
      else { \
	_var = (_type)(pUnion->val); pUnion++; \
      } \
    }

#define NEXT_PTR(_type, _var) { \
    assert(pUnion); _var = (_type)pUnion->ptr; pUnion++; }

int
dixChangeGC(ClientPtr client, register GC *pGC, register BITS32 mask, CARD32 *pC32, ChangeGCValPtr pUnion)
{
    register BITS32 	index2;
    register int 	error = 0;
    PixmapPtr 		pPixmap;
    BITS32		maskQ;

    assert( (pC32 && !pUnion) || (!pC32 && pUnion) );
    pGC->serialNumber |= GC_CHANGE_SERIAL_BIT;

    maskQ = mask;	/* save these for when we walk the GCque */
    while (mask && !error) 
    {
	index2 = (BITS32) lowbit (mask);
	mask &= ~index2;
	pGC->stateChanges |= index2;
	switch (index2)
	{
	    case GCFunction:
	    {
		CARD8 newalu;
		NEXTVAL(CARD8, newalu);
		if (newalu <= GXset)
		    pGC->alu = newalu;
		else
		{
		    clientErrorValue = newalu;
		    error = BadValue;
		}
		break;
	    }
	    case GCPlaneMask:
		NEXTVAL(unsigned long, pGC->planemask);
		break;
	    case GCForeground:
		NEXTVAL(unsigned long, pGC->fgPixel);
		/*
		 * this is for CreateGC
		 */
		if (!pGC->tileIsPixel && !pGC->tile.pixmap)
		{
		    pGC->tileIsPixel = TRUE;
		    pGC->tile.pixel = pGC->fgPixel;
		}
		break;
	    case GCBackground:
		NEXTVAL(unsigned long, pGC->bgPixel);
		break;
	    case GCLineWidth:		/* ??? line width is a CARD16 */
		 NEXTVAL(CARD16, pGC->lineWidth);
		break;
	    case GCLineStyle:
	    {
		unsigned int newlinestyle;
		NEXTVAL(unsigned int, newlinestyle);
		if (newlinestyle <= LineDoubleDash)
		    pGC->lineStyle = newlinestyle;
		else
		{
		    clientErrorValue = newlinestyle;
		    error = BadValue;
		}
		break;
	    }
	    case GCCapStyle:
	    {
		unsigned int newcapstyle;
		NEXTVAL(unsigned int, newcapstyle);
		if (newcapstyle <= CapProjecting)
		    pGC->capStyle = newcapstyle;
		else
		{
		    clientErrorValue = newcapstyle;
		    error = BadValue;
		}
		break;
	    }
	    case GCJoinStyle:
	    {
		unsigned int newjoinstyle;
		NEXTVAL(unsigned int, newjoinstyle);
		if (newjoinstyle <= JoinBevel)
		    pGC->joinStyle = newjoinstyle;
		else
		{
		    clientErrorValue = newjoinstyle;
		    error = BadValue;
		}
		break;
	    }
	    case GCFillStyle:
	    {
		unsigned int newfillstyle;
		NEXTVAL(unsigned int, newfillstyle);
		if (newfillstyle <= FillOpaqueStippled)
		    pGC->fillStyle = newfillstyle;
		else
		{
		    clientErrorValue = newfillstyle;
		    error = BadValue;
		}
		break;
	    }
	    case GCFillRule:
	    {
		unsigned int newfillrule;
		NEXTVAL(unsigned int, newfillrule);
		if (newfillrule <= WindingRule)
		    pGC->fillRule = newfillrule;
		else
		{
		    clientErrorValue = newfillrule;
		    error = BadValue;
		}
		break;
	    }
	    case GCTile:
	    {
		XID newpix = 0;
		if (pUnion)
		{
		    NEXT_PTR(PixmapPtr, pPixmap);
		}
		else
		{
		    NEXTVAL(XID, newpix);
		    pPixmap = (PixmapPtr)SecurityLookupIDByType(client,
					newpix, RT_PIXMAP, SecurityReadAccess);
		}
		if (pPixmap)
		{
		    if ((pPixmap->drawable.depth != pGC->depth) ||
			(pPixmap->drawable.pScreen != pGC->pScreen))
		    {
			error = BadMatch;
		    }
		    else
		    {
			pPixmap->refcnt++;
			if (!pGC->tileIsPixel)
			    (* pGC->pScreen->DestroyPixmap)(pGC->tile.pixmap);
			pGC->tileIsPixel = FALSE;
			pGC->tile.pixmap = pPixmap;
		    }
		}
		else
		{
		    clientErrorValue = newpix;
		    error = BadPixmap;
		}
		break;
	    }
	    case GCStipple:
	    {
		XID newstipple = 0;
		if (pUnion)
		{
		    NEXT_PTR(PixmapPtr, pPixmap);
		}
		else
		{
		    NEXTVAL(XID, newstipple)
		    pPixmap = (PixmapPtr)SecurityLookupIDByType(client,
				newstipple, RT_PIXMAP, SecurityReadAccess);
		}
		if (pPixmap)
		{
		    if ((pPixmap->drawable.depth != 1) ||
			(pPixmap->drawable.pScreen != pGC->pScreen))
		    {
			error = BadMatch;
		    }
		    else
		    {
			pPixmap->refcnt++;
			if (pGC->stipple)
			    (* pGC->pScreen->DestroyPixmap)(pGC->stipple);
			pGC->stipple = pPixmap;
		    }
		}
		else
		{
		    clientErrorValue = newstipple;
		    error = BadPixmap;
		}
		break;
	    }
	    case GCTileStipXOrigin:
		NEXTVAL(INT16, pGC->patOrg.x);
		break;
	    case GCTileStipYOrigin:
		NEXTVAL(INT16, pGC->patOrg.y);
		break;
	    case GCFont:
    	    {
		FontPtr	pFont;
		XID newfont = 0;
		if (pUnion)
		{
		    NEXT_PTR(FontPtr, pFont);
		}
		else
		{
		    NEXTVAL(XID, newfont)
		    pFont = (FontPtr)SecurityLookupIDByType(client, newfont,
						RT_FONT, SecurityReadAccess);
		}
		if (pFont)
		{
		    pFont->refcnt++;
		    if (pGC->font)
    		        CloseFont(pGC->font, (Font)0);
		    pGC->font = pFont;
		 }
		else
		{
		    clientErrorValue = newfont;
		    error = BadFont;
		}
		break;
	    }
	    case GCSubwindowMode:
	    {
		unsigned int newclipmode;
		NEXTVAL(unsigned int, newclipmode);
		if (newclipmode <= IncludeInferiors)
		    pGC->subWindowMode = newclipmode;
		else
		{
		    clientErrorValue = newclipmode;
		    error = BadValue;
		}
		break;
	    }
	    case GCGraphicsExposures:
    	    {
		unsigned int newge;
		NEXTVAL(unsigned int, newge);
		if (newge <= xTrue)
		    pGC->graphicsExposures = newge;
		else
		{
		    clientErrorValue = newge;
		    error = BadValue;
		}
		break;
	    }
	    case GCClipXOrigin:
		NEXTVAL(INT16, pGC->clipOrg.x);
		break;
	    case GCClipYOrigin:
		NEXTVAL(INT16, pGC->clipOrg.y);
		break;
	    case GCClipMask:
	    {
		Pixmap pid = 0;
		int    clipType = 0;

		if (pUnion)
		{
		    NEXT_PTR(PixmapPtr, pPixmap);
		}
		else
		{
		    NEXTVAL(Pixmap, pid)
		    if (pid == None)
		    {
			clipType = CT_NONE;
			pPixmap = NullPixmap;
		    }
		    else
		        pPixmap = (PixmapPtr)SecurityLookupIDByType(client,
					pid, RT_PIXMAP, SecurityReadAccess);
		}

		if (pPixmap)
		{
		    if ((pPixmap->drawable.depth != 1) ||
			(pPixmap->drawable.pScreen != pGC->pScreen))
		    {
			error = BadMatch;
		    }
		    else
		    {
			clipType = CT_PIXMAP;
			pPixmap->refcnt++;
		    }
		}
		else if (!pUnion && (pid != None))
		{
		    clientErrorValue = pid;
		    error = BadPixmap;
		}
		if(error == Success)
		{
		    (*pGC->funcs->ChangeClip)(pGC, clipType,
					      (pointer)pPixmap, 0);
		}
		break;
	    }
	    case GCDashOffset:
		NEXTVAL(INT16, pGC->dashOffset);
		break;
	    case GCDashList:
	    {
		CARD8 newdash;
		NEXTVAL(CARD8, newdash);
		if (newdash == 4)
		{
		    if (pGC->dash != DefaultDash)
		    {
			xfree(pGC->dash);
			pGC->numInDashList = 2;
			pGC->dash = DefaultDash;
		    }
		}
		else if (newdash != 0)
 		{
		    unsigned char *dash;

		    dash = (unsigned char *)xalloc(2 * sizeof(unsigned char));
		    if (dash)
		    {
			if (pGC->dash != DefaultDash)
			    xfree(pGC->dash);
			pGC->numInDashList = 2;
			pGC->dash = dash;
			dash[0] = newdash;
			dash[1] = newdash;
		    }
		    else
			error = BadAlloc;
		}
 		else
		{
		   clientErrorValue = newdash;
		   error = BadValue;
		}
		break;
	    }
	    case GCArcMode:
	    {
		unsigned int newarcmode;
		NEXTVAL(unsigned int, newarcmode);
		if (newarcmode <= ArcPieSlice)
		    pGC->arcMode = newarcmode;
		else
		{
		    clientErrorValue = newarcmode;
		    error = BadValue;
		}
		break;
	    }
	    default:
		clientErrorValue = maskQ;
		error = BadValue;
		break;
	}
    } /* end while mask && !error */

    if (pGC->fillStyle == FillTiled && pGC->tileIsPixel)
    {
	if (!CreateDefaultTile (pGC))
	{
	    pGC->fillStyle = FillSolid;
	    error = BadAlloc;
	}
    }
    (*pGC->funcs->ChangeGC)(pGC, maskQ);
    return error;
}

#undef NEXTVAL
#undef NEXT_PTR

/* Publically defined entry to ChangeGC.  Just calls dixChangeGC and tells
 * it that all of the entries are constants or IDs */
int
ChangeGC(register GC *pGC, register BITS32 mask, XID *pval)
{
    return (dixChangeGC(NullClient, pGC, mask, pval, NULL));
}

/* DoChangeGC(pGC, mask, pval, fPointer)
   mask is a set of bits indicating which values to change.
   pval contains an appropriate value for each mask.
   fPointer is true if the values for tiles, stipples, fonts or clipmasks
   are pointers instead of IDs.  Note: if you are passing pointers you
   MUST declare the array of values as type pointer!  Other data types
   may not be large enough to hold pointers on some machines.  Yes,
   this means you have to cast to (XID *) when you pass the array to
   DoChangeGC.  Similarly, if you are not passing pointers (fPointer = 0) you
   MUST declare the array as type XID (not unsigned long!), or again the wrong
   size data type may be used.  To avoid this cruftiness, use dixChangeGC
   above.

   if there is an error, the value is marked as changed 
   anyway, which is probably wrong, but infrequent.

NOTE:
	all values sent over the protocol for ChangeGC requests are
32 bits long
*/
int
DoChangeGC(register GC *pGC, register BITS32 mask, XID *pval, int fPointer)
{
    if (fPointer)
    /* XXX might be a problem on 64 bit big-endian servers */
	return dixChangeGC(NullClient, pGC, mask, NULL, (ChangeGCValPtr)pval);
    else
	return dixChangeGC(NullClient, pGC, mask, pval, NULL);
}


/* CreateGC(pDrawable, mask, pval, pStatus)
   creates a default GC for the given drawable, using mask to fill
   in any non-default values.
   Returns a pointer to the new GC on success, NULL otherwise.
   returns status of non-default fields in pStatus
BUG:
   should check for failure to create default tile

*/

static GCPtr
AllocateGC(ScreenPtr pScreen)
{
    GCPtr pGC;
    register char *ptr;
    register DevUnion *ppriv;
    register unsigned *sizes;
    register unsigned size;
    register int i;

    pGC = (GCPtr)xalloc(pScreen->totalGCSize);
    if (pGC)
    {
	ppriv = (DevUnion *)(pGC + 1);
	pGC->devPrivates = ppriv;
	sizes = pScreen->GCPrivateSizes;
	ptr = (char *)(ppriv + pScreen->GCPrivateLen);
	for (i = pScreen->GCPrivateLen; --i >= 0; ppriv++, sizes++)
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
    return pGC;
}

GCPtr
CreateGC(DrawablePtr pDrawable, BITS32 mask, XID *pval, int *pStatus)
{
    register GCPtr pGC;

    pGC = AllocateGC(pDrawable->pScreen);
    if (!pGC)
    {
	*pStatus = BadAlloc;
	return (GCPtr)NULL;
    }

    pGC->pScreen = pDrawable->pScreen;
    pGC->depth = pDrawable->depth;
    pGC->alu = GXcopy; /* dst <- src */
    pGC->planemask = ~0;
    pGC->serialNumber = GC_CHANGE_SERIAL_BIT;
    pGC->funcs = 0;

    pGC->fgPixel = 0;
    pGC->bgPixel = 1;
    pGC->lineWidth = 0;
    pGC->lineStyle = LineSolid;
    pGC->capStyle = CapButt;
    pGC->joinStyle = JoinMiter;
    pGC->fillStyle = FillSolid;
    pGC->fillRule = EvenOddRule;
    pGC->arcMode = ArcPieSlice;
    if (mask & GCForeground)
    {
	/*
	 * magic special case -- ChangeGC checks for this condition
	 * and snags the Foreground value to create a pseudo default-tile
	 */
	pGC->tileIsPixel = FALSE;
	pGC->tile.pixmap = NullPixmap;
    }
    else
    {
	pGC->tileIsPixel = TRUE;
	pGC->tile.pixel = 0;
    }

    pGC->patOrg.x = 0;
    pGC->patOrg.y = 0;
    pGC->subWindowMode = ClipByChildren;
    pGC->graphicsExposures = TRUE;
    pGC->clipOrg.x = 0;
    pGC->clipOrg.y = 0;
    pGC->clientClipType = CT_NONE;
    pGC->clientClip = (pointer)NULL;
    pGC->numInDashList = 2;
    pGC->dash = DefaultDash;
    pGC->dashOffset = 0;
    pGC->lastWinOrg.x = 0;
    pGC->lastWinOrg.y = 0;

    /* use the default font and stipple */
    pGC->font = defaultFont;
    defaultFont->refcnt++;
    pGC->stipple = pGC->pScreen->PixmapPerDepth[0];
    pGC->stipple->refcnt++;

    pGC->stateChanges = (1 << (GCLastBit+1)) - 1;
    if (!(*pGC->pScreen->CreateGC)(pGC))
	*pStatus = BadAlloc;
    else if (mask)
        *pStatus = ChangeGC(pGC, mask, pval);
    else
	*pStatus = Success;
    if (*pStatus != Success)
    {
	if (!pGC->tileIsPixel && !pGC->tile.pixmap)
	    pGC->tileIsPixel = TRUE; /* undo special case */
	FreeGC(pGC, (XID)0);
	pGC = (GCPtr)NULL;
    }

    return (pGC);
}

static Bool
CreateDefaultTile (GCPtr pGC)
{
    XID		tmpval[3];
    PixmapPtr 	pTile;
    GCPtr	pgcScratch;
    xRectangle	rect;
    CARD16	w, h;

    w = 1;
    h = 1;
    (*pGC->pScreen->QueryBestSize)(TileShape, &w, &h, pGC->pScreen);
    pTile = (PixmapPtr)
	    (*pGC->pScreen->CreatePixmap)(pGC->pScreen,
					  w, h, pGC->depth);
    pgcScratch = GetScratchGC(pGC->depth, pGC->pScreen);
    if (!pTile || !pgcScratch)
    {
	if (pTile)
	    (*pTile->drawable.pScreen->DestroyPixmap)(pTile);
	if (pgcScratch)
	    FreeScratchGC(pgcScratch);
	return FALSE;
    }
    tmpval[0] = GXcopy;
    tmpval[1] = pGC->tile.pixel;
    tmpval[2] = FillSolid;
    (void)ChangeGC(pgcScratch, GCFunction | GCForeground | GCFillStyle, 
		   tmpval);
    ValidateGC((DrawablePtr)pTile, pgcScratch);
    rect.x = 0;
    rect.y = 0;
    rect.width = w;
    rect.height = h;
    (*pgcScratch->ops->PolyFillRect)((DrawablePtr)pTile, pgcScratch, 1, &rect);
    /* Always remember to free the scratch graphics context after use. */
    FreeScratchGC(pgcScratch);

    pGC->tileIsPixel = FALSE;
    pGC->tile.pixmap = pTile;
    return TRUE;
}

int
CopyGC(register GC *pgcSrc, register GC *pgcDst, register BITS32 mask)
{
    register BITS32	index2;
    BITS32		maskQ;
    int 		error = 0;

    if (pgcSrc == pgcDst)
	return Success;
    pgcDst->serialNumber |= GC_CHANGE_SERIAL_BIT;
    pgcDst->stateChanges |= mask;
    maskQ = mask;
    while (mask)
    {
	index2 = (BITS32) lowbit (mask);
	mask &= ~index2;
	switch (index2)
	{
	    case GCFunction:
		pgcDst->alu = pgcSrc->alu;
		break;
	    case GCPlaneMask:
		pgcDst->planemask = pgcSrc->planemask;
		break;
	    case GCForeground:
		pgcDst->fgPixel = pgcSrc->fgPixel;
		break;
	    case GCBackground:
		pgcDst->bgPixel = pgcSrc->bgPixel;
		break;
	    case GCLineWidth:
		pgcDst->lineWidth = pgcSrc->lineWidth;
		break;
	    case GCLineStyle:
		pgcDst->lineStyle = pgcSrc->lineStyle;
		break;
	    case GCCapStyle:
		pgcDst->capStyle = pgcSrc->capStyle;
		break;
	    case GCJoinStyle:
		pgcDst->joinStyle = pgcSrc->joinStyle;
		break;
	    case GCFillStyle:
		pgcDst->fillStyle = pgcSrc->fillStyle;
		break;
	    case GCFillRule:
		pgcDst->fillRule = pgcSrc->fillRule;
		break;
	    case GCTile:
		{
		    if (EqualPixUnion(pgcDst->tileIsPixel,
				      pgcDst->tile,
				      pgcSrc->tileIsPixel,
				      pgcSrc->tile))
		    {
			break;
		    }
		    if (!pgcDst->tileIsPixel)
			(* pgcDst->pScreen->DestroyPixmap)(pgcDst->tile.pixmap);
		    pgcDst->tileIsPixel = pgcSrc->tileIsPixel;
		    pgcDst->tile = pgcSrc->tile;
		    if (!pgcDst->tileIsPixel)
		       pgcDst->tile.pixmap->refcnt++;
		    break;
		}
	    case GCStipple:
		{
		    if (pgcDst->stipple == pgcSrc->stipple)
			break;
		    if (pgcDst->stipple)
			(* pgcDst->pScreen->DestroyPixmap)(pgcDst->stipple);
		    pgcDst->stipple = pgcSrc->stipple;
		    if (pgcDst->stipple)
			pgcDst->stipple->refcnt ++;
		    break;
		}
	    case GCTileStipXOrigin:
		pgcDst->patOrg.x = pgcSrc->patOrg.x;
		break;
	    case GCTileStipYOrigin:
		pgcDst->patOrg.y = pgcSrc->patOrg.y;
		break;
	    case GCFont:
		if (pgcDst->font == pgcSrc->font)
		    break;
		if (pgcDst->font)
		    CloseFont(pgcDst->font, (Font)0);
		if ((pgcDst->font = pgcSrc->font) != NullFont)
		    (pgcDst->font)->refcnt++;
		break;
	    case GCSubwindowMode:
		pgcDst->subWindowMode = pgcSrc->subWindowMode;
		break;
	    case GCGraphicsExposures:
		pgcDst->graphicsExposures = pgcSrc->graphicsExposures;
		break;
	    case GCClipXOrigin:
		pgcDst->clipOrg.x = pgcSrc->clipOrg.x;
		break;
	    case GCClipYOrigin:
		pgcDst->clipOrg.y = pgcSrc->clipOrg.y;
		break;
	    case GCClipMask:
		(* pgcDst->funcs->CopyClip)(pgcDst, pgcSrc);
		break;
	    case GCDashOffset:
		pgcDst->dashOffset = pgcSrc->dashOffset;
		break;
	    case GCDashList:
		if (pgcSrc->dash == DefaultDash)
		{
		    if (pgcDst->dash != DefaultDash)
		    {
			xfree(pgcDst->dash);
			pgcDst->numInDashList = pgcSrc->numInDashList;
			pgcDst->dash = pgcSrc->dash;
		    }
		}
		else
		{
		    unsigned char *dash;
		    unsigned int i;

		    dash = (unsigned char *)xalloc(pgcSrc->numInDashList *
						   sizeof(unsigned char));
		    if (dash)
		    {
			if (pgcDst->dash != DefaultDash)
			    xfree(pgcDst->dash);
			pgcDst->numInDashList = pgcSrc->numInDashList;
			pgcDst->dash = dash;
			for (i=0; i<pgcSrc->numInDashList; i++)
			    dash[i] = pgcSrc->dash[i];
		    }
		    else
			error = BadAlloc;
		}
		break;
	    case GCArcMode:
		pgcDst->arcMode = pgcSrc->arcMode;
		break;
	    default:
		clientErrorValue = maskQ;
		error = BadValue;
		break;
	}
    }
    if (pgcDst->fillStyle == FillTiled && pgcDst->tileIsPixel)
    {
	if (!CreateDefaultTile (pgcDst))
	{
	    pgcDst->fillStyle = FillSolid;
	    error = BadAlloc;
	}
    }
    (*pgcDst->funcs->CopyGC) (pgcSrc, maskQ, pgcDst);
    return error;
}

/**
 * does the diX part of freeing the characteristics in the GC.
 *
 *  \param value  must conform to DeleteType
 */
int
FreeGC(pointer value, XID gid)
{
    GCPtr pGC = (GCPtr)value;

    CloseFont(pGC->font, (Font)0);
    (* pGC->funcs->DestroyClip)(pGC);

    if (!pGC->tileIsPixel)
	(* pGC->pScreen->DestroyPixmap)(pGC->tile.pixmap);
    if (pGC->stipple)
	(* pGC->pScreen->DestroyPixmap)(pGC->stipple);

    (*pGC->funcs->DestroyGC) (pGC);
    if (pGC->dash != DefaultDash)
	xfree(pGC->dash);
    xfree(pGC);
    return(Success);
}

void
SetGCMask(GCPtr pGC, Mask selectMask, Mask newDataMask)
{
    pGC->stateChanges = (~selectMask & pGC->stateChanges) |
		        (selectMask & newDataMask);
    if (selectMask & newDataMask)
        pGC->serialNumber |= GC_CHANGE_SERIAL_BIT;        
}



/* CreateScratchGC(pScreen, depth)
    like CreateGC, but doesn't do the default tile or stipple,
since we can't create them without already having a GC.  any code
using the tile or stipple has to set them explicitly anyway,
since the state of the scratch gc is unknown.  This is OK
because ChangeGC() has to be able to deal with NULL tiles and
stipples anyway (in case the CreateGC() call has provided a 
value for them -- we can't set the default tile until the
client-supplied attributes are installed, since the fgPixel
is what fills the default tile.  (maybe this comment should
go with CreateGC() or ChangeGC().)
*/

GCPtr
CreateScratchGC(ScreenPtr pScreen, unsigned depth)
{
    register GCPtr pGC;

    pGC = AllocateGC(pScreen);
    if (!pGC)
	return (GCPtr)NULL;

    pGC->pScreen = pScreen;
    pGC->depth = depth;
    pGC->alu = GXcopy; /* dst <- src */
    pGC->planemask = ~0;
    pGC->serialNumber = 0;

    pGC->fgPixel = 0;
    pGC->bgPixel = 1;
    pGC->lineWidth = 0;
    pGC->lineStyle = LineSolid;
    pGC->capStyle = CapButt;
    pGC->joinStyle = JoinMiter;
    pGC->fillStyle = FillSolid;
    pGC->fillRule = EvenOddRule;
    pGC->arcMode = ArcPieSlice;
    pGC->font = defaultFont;
    if ( pGC->font)  /* necessary, because open of default font could fail */
	pGC->font->refcnt++;
    pGC->tileIsPixel = TRUE;
    pGC->tile.pixel = 0;
    pGC->stipple = NullPixmap;
    pGC->patOrg.x = 0;
    pGC->patOrg.y = 0;
    pGC->subWindowMode = ClipByChildren;
    pGC->graphicsExposures = TRUE;
    pGC->clipOrg.x = 0;
    pGC->clipOrg.y = 0;
    pGC->clientClipType = CT_NONE;
    pGC->dashOffset = 0;
    pGC->numInDashList = 2;
    pGC->dash = DefaultDash;
    pGC->lastWinOrg.x = 0;
    pGC->lastWinOrg.y = 0;

    pGC->stateChanges = (1 << (GCLastBit+1)) - 1;
    if (!(*pScreen->CreateGC)(pGC))
    {
	FreeGC(pGC, (XID)0);
	pGC = (GCPtr)NULL;
    }
    return pGC;
}

void
FreeGCperDepth(int screenNum)
{
    register int i;
    register ScreenPtr pScreen;
    GCPtr *ppGC;

    pScreen = screenInfo.screens[screenNum];
    ppGC = pScreen->GCperDepth;

    for (i = 0; i <= pScreen->numDepths; i++)
	(void)FreeGC(ppGC[i], (XID)0);
    pScreen->rgf = ~0L;
}


Bool
CreateGCperDepth(int screenNum)
{
    register int i;
    register ScreenPtr pScreen;
    DepthPtr pDepth;
    GCPtr *ppGC;

    pScreen = screenInfo.screens[screenNum];
    pScreen->rgf = 0;
    ppGC = pScreen->GCperDepth;
    /* do depth 1 separately because it's not included in list */
    if (!(ppGC[0] = CreateScratchGC(pScreen, 1)))
	return FALSE;
    ppGC[0]->graphicsExposures = FALSE;
    /* Make sure we don't overflow GCperDepth[] */
    if( pScreen->numDepths > MAXFORMATS )
	    return FALSE;

    pDepth = pScreen->allowedDepths;
    for (i=0; i<pScreen->numDepths; i++, pDepth++)
    {
	if (!(ppGC[i+1] = CreateScratchGC(pScreen, pDepth->depth)))
	{
	    for (; i >= 0; i--)
		(void)FreeGC(ppGC[i], (XID)0);
	    return FALSE;
	}
	ppGC[i+1]->graphicsExposures = FALSE;
    }
    return TRUE;
}

Bool
CreateDefaultStipple(int screenNum)
{
    register ScreenPtr pScreen;
    XID tmpval[3];
    xRectangle rect;
    CARD16 w, h;
    GCPtr pgcScratch;

    pScreen = screenInfo.screens[screenNum];

    w = 16;
    h = 16;
    (* pScreen->QueryBestSize)(StippleShape, &w, &h, pScreen);
    if (!(pScreen->PixmapPerDepth[0] =
			(*pScreen->CreatePixmap)(pScreen, w, h, 1)))
	return FALSE;
    /* fill stipple with 1 */
    tmpval[0] = GXcopy; tmpval[1] = 1; tmpval[2] = FillSolid;
    pgcScratch = GetScratchGC(1, pScreen);
    if (!pgcScratch)
    {
	(*pScreen->DestroyPixmap)(pScreen->PixmapPerDepth[0]);
	return FALSE;
    }
    (void)ChangeGC(pgcScratch, GCFunction|GCForeground|GCFillStyle, tmpval);
    ValidateGC((DrawablePtr)pScreen->PixmapPerDepth[0], pgcScratch);
    rect.x = 0;
    rect.y = 0;
    rect.width = w;
    rect.height = h;
    (*pgcScratch->ops->PolyFillRect)((DrawablePtr)pScreen->PixmapPerDepth[0], 
				     pgcScratch, 1, &rect);
    FreeScratchGC(pgcScratch);
    return TRUE;
}

void
FreeDefaultStipple(int screenNum)
{
    ScreenPtr pScreen = screenInfo.screens[screenNum];
    (*pScreen->DestroyPixmap)(pScreen->PixmapPerDepth[0]);
}

int
SetDashes(register GCPtr pGC, unsigned offset, unsigned ndash, unsigned char *pdash)
{
    register long i;
    register unsigned char *p, *indash;
    BITS32 maskQ = 0;

    i = ndash;
    p = pdash;
    while (i--)
    {
	if (!*p++)
	{
	    /* dash segment must be > 0 */
	    clientErrorValue = 0;
	    return BadValue;
	}
    }

    if (ndash & 1)
	p = (unsigned char *)xalloc(2 * ndash * sizeof(unsigned char));
    else
	p = (unsigned char *)xalloc(ndash * sizeof(unsigned char));
    if (!p)
	return BadAlloc;

    pGC->serialNumber |= GC_CHANGE_SERIAL_BIT;
    if (offset != pGC->dashOffset)
    {
	pGC->dashOffset = offset;
	pGC->stateChanges |= GCDashOffset;
	maskQ |= GCDashOffset;
    }

    if (pGC->dash != DefaultDash)
	xfree(pGC->dash);
    pGC->numInDashList = ndash;
    pGC->dash = p;
    if (ndash & 1)
    {
	pGC->numInDashList += ndash;
	indash = pdash;
	i = ndash;
	while (i--)
	    *p++ = *indash++;
    }
    while(ndash--)
	*p++ = *pdash++;
    pGC->stateChanges |= GCDashList;
    maskQ |= GCDashList;

    if (pGC->funcs->ChangeGC)
	(*pGC->funcs->ChangeGC) (pGC, maskQ);
    return Success;
}

int
VerifyRectOrder(int nrects, xRectangle *prects, int ordering)
{
    register xRectangle	*prectP, *prectN;
    register int	i;

    switch(ordering)
    {
      case Unsorted:
	  return CT_UNSORTED;
      case YSorted:
	  if(nrects > 1)
	  {
	      for(i = 1, prectP = prects, prectN = prects + 1;
		  i < nrects;
		  i++, prectP++, prectN++)
		  if(prectN->y < prectP->y)
		      return -1;
	  }
	  return CT_YSORTED;
      case YXSorted:
	  if(nrects > 1)
	  {
	      for(i = 1, prectP = prects, prectN = prects + 1;
		  i < nrects;
		  i++, prectP++, prectN++)
		  if((prectN->y < prectP->y) ||
		      ( (prectN->y == prectP->y) &&
		        (prectN->x < prectP->x) ) )
		      return -1;
	  }
	  return CT_YXSORTED;
      case YXBanded:
	  if(nrects > 1)
	  {
	      for(i = 1, prectP = prects, prectN = prects + 1;
		  i < nrects;
		  i++, prectP++, prectN++)
		  if((prectN->y != prectP->y &&
 		      prectN->y < prectP->y + (int) prectP->height) ||
		     ((prectN->y == prectP->y) &&
		      (prectN->height != prectP->height ||
		       prectN->x < prectP->x + (int) prectP->width)))
		      return -1;
	  }
	  return CT_YXBANDED;
    }
    return -1;
}

int
SetClipRects(GCPtr pGC, int xOrigin, int yOrigin, int nrects, 
             xRectangle *prects, int ordering)
{
    int			newct, size;
    xRectangle 		*prectsNew;

    newct = VerifyRectOrder(nrects, prects, ordering);
    if (newct < 0)
	return(BadMatch);
    size = nrects * sizeof(xRectangle);
    prectsNew = (xRectangle *) xalloc(size);
    if (!prectsNew && size)
	return BadAlloc;

    pGC->serialNumber |= GC_CHANGE_SERIAL_BIT;
    pGC->clipOrg.x = xOrigin;
    pGC->stateChanges |= GCClipXOrigin;
		 
    pGC->clipOrg.y = yOrigin;
    pGC->stateChanges |= GCClipYOrigin;

    if (size)
	memmove((char *)prectsNew, (char *)prects, size);
    (*pGC->funcs->ChangeClip)(pGC, newct, (pointer)prectsNew, nrects);
    if (pGC->funcs->ChangeGC)
	(*pGC->funcs->ChangeGC) (pGC, GCClipXOrigin|GCClipYOrigin|GCClipMask);
    return Success;
}


/*
   sets reasonable defaults 
   if we can get a pre-allocated one, use it and mark it as used.
   if we can't, create one out of whole cloth (The Velveteen GC -- if
   you use it often enough it will become real.)
*/
GCPtr
GetScratchGC(register unsigned depth, register ScreenPtr pScreen)
{
    register int i;
    register GCPtr pGC;

    for (i=0; i<=pScreen->numDepths; i++)
        if ( pScreen->GCperDepth[i]->depth == depth &&
	     !(pScreen->rgf & (1L << (i+1)))
	   )
	{
	    pScreen->rgf |= (1L << (i+1));
            pGC = (pScreen->GCperDepth[i]);

	    pGC->alu = GXcopy;
	    pGC->planemask = ~0;
	    pGC->serialNumber = 0;
	    pGC->fgPixel = 0;
	    pGC->bgPixel = 1;
	    pGC->lineWidth = 0;
	    pGC->lineStyle = LineSolid;
	    pGC->capStyle = CapButt;
	    pGC->joinStyle = JoinMiter;
	    pGC->fillStyle = FillSolid;
	    pGC->fillRule = EvenOddRule;
	    pGC->arcMode = ArcChord;
	    pGC->patOrg.x = 0;
	    pGC->patOrg.y = 0;
	    pGC->subWindowMode = ClipByChildren;
	    pGC->graphicsExposures = FALSE;
	    pGC->clipOrg.x = 0;
	    pGC->clipOrg.y = 0;
	    if (pGC->clientClipType != CT_NONE)
		(*pGC->funcs->ChangeClip) (pGC, CT_NONE, NULL, 0);
	    pGC->stateChanges = (1 << (GCLastBit+1)) - 1;
	    return pGC;
	}
    /* if we make it this far, need to roll our own */
    pGC = CreateScratchGC(pScreen, depth);
    if (pGC)
	pGC->graphicsExposures = FALSE;
    return pGC;
}

/*
   if the gc to free is in the table of pre-existing ones,
mark it as available.
   if not, free it for real
*/
void
FreeScratchGC(register GCPtr pGC)
{
    register ScreenPtr pScreen = pGC->pScreen;
    register int i;

    for (i=0; i<=pScreen->numDepths; i++)
    {
        if ( pScreen->GCperDepth[i] == pGC)
	{
	    pScreen->rgf &= ~(1L << (i+1));
	    return;
	}
    }
    (void)FreeGC(pGC, (GContext)0);
}
