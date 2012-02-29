/*
 * $XFree86: xc/programs/Xserver/render/render.c,v 1.28 2003/11/03 05:12:02 tsi Exp $
 *
 * Copyright © 2000 SuSE, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of SuSE not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  SuSE makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * SuSE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL SuSE
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, SuSE, Inc.
 */

#define NEED_REPLIES
#define NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "resource.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "colormapst.h"
#include "extnsionst.h"
#include "servermd.h"
#include "render.h"
#include "renderproto.h"
#include "picturestr.h"
#include "glyphstr.h"
#include "Xfuncproto.h"
#include "cursorstr.h"
#ifdef EXTMODULE
#include "xf86_ansic.h"
#endif

static int ProcRenderQueryVersion (ClientPtr pClient);
static int ProcRenderQueryPictFormats (ClientPtr pClient);
static int ProcRenderQueryPictIndexValues (ClientPtr pClient);
static int ProcRenderQueryDithers (ClientPtr pClient);
static int ProcRenderCreatePicture (ClientPtr pClient);
static int ProcRenderChangePicture (ClientPtr pClient);
static int ProcRenderSetPictureClipRectangles (ClientPtr pClient);
static int ProcRenderFreePicture (ClientPtr pClient);
static int ProcRenderComposite (ClientPtr pClient);
static int ProcRenderScale (ClientPtr pClient);
static int ProcRenderTrapezoids (ClientPtr pClient);
static int ProcRenderTriangles (ClientPtr pClient);
static int ProcRenderTriStrip (ClientPtr pClient);
static int ProcRenderTriFan (ClientPtr pClient);
static int ProcRenderColorTrapezoids (ClientPtr pClient);
static int ProcRenderColorTriangles (ClientPtr pClient);
static int ProcRenderTransform (ClientPtr pClient);
static int ProcRenderCreateGlyphSet (ClientPtr pClient);
static int ProcRenderReferenceGlyphSet (ClientPtr pClient);
static int ProcRenderFreeGlyphSet (ClientPtr pClient);
static int ProcRenderAddGlyphs (ClientPtr pClient);
static int ProcRenderAddGlyphsFromPicture (ClientPtr pClient);
static int ProcRenderFreeGlyphs (ClientPtr pClient);
static int ProcRenderCompositeGlyphs (ClientPtr pClient);
static int ProcRenderFillRectangles (ClientPtr pClient);
static int ProcRenderCreateCursor (ClientPtr pClient);
static int ProcRenderSetPictureTransform (ClientPtr pClient);
static int ProcRenderQueryFilters (ClientPtr pClient);
static int ProcRenderSetPictureFilter (ClientPtr pClient);
static int ProcRenderCreateAnimCursor (ClientPtr pClient);

static int ProcRenderDispatch (ClientPtr pClient);

static int SProcRenderQueryVersion (ClientPtr pClient);
static int SProcRenderQueryPictFormats (ClientPtr pClient);
static int SProcRenderQueryPictIndexValues (ClientPtr pClient);
static int SProcRenderQueryDithers (ClientPtr pClient);
static int SProcRenderCreatePicture (ClientPtr pClient);
static int SProcRenderChangePicture (ClientPtr pClient);
static int SProcRenderSetPictureClipRectangles (ClientPtr pClient);
static int SProcRenderFreePicture (ClientPtr pClient);
static int SProcRenderComposite (ClientPtr pClient);
static int SProcRenderScale (ClientPtr pClient);
static int SProcRenderTrapezoids (ClientPtr pClient);
static int SProcRenderTriangles (ClientPtr pClient);
static int SProcRenderTriStrip (ClientPtr pClient);
static int SProcRenderTriFan (ClientPtr pClient);
static int SProcRenderColorTrapezoids (ClientPtr pClient);
static int SProcRenderColorTriangles (ClientPtr pClient);
static int SProcRenderTransform (ClientPtr pClient);
static int SProcRenderCreateGlyphSet (ClientPtr pClient);
static int SProcRenderReferenceGlyphSet (ClientPtr pClient);
static int SProcRenderFreeGlyphSet (ClientPtr pClient);
static int SProcRenderAddGlyphs (ClientPtr pClient);
static int SProcRenderAddGlyphsFromPicture (ClientPtr pClient);
static int SProcRenderFreeGlyphs (ClientPtr pClient);
static int SProcRenderCompositeGlyphs (ClientPtr pClient);
static int SProcRenderFillRectangles (ClientPtr pClient);
static int SProcRenderCreateCursor (ClientPtr pClient);
static int SProcRenderSetPictureTransform (ClientPtr pClient);
static int SProcRenderQueryFilters (ClientPtr pClient);
static int SProcRenderSetPictureFilter (ClientPtr pClient);
static int SProcRenderCreateAnimCursor (ClientPtr pClient);

static int SProcRenderDispatch (ClientPtr pClient);

int	(*ProcRenderVector[RenderNumberRequests])(ClientPtr) = {
    ProcRenderQueryVersion,
    ProcRenderQueryPictFormats,
    ProcRenderQueryPictIndexValues,
    ProcRenderQueryDithers,
    ProcRenderCreatePicture,
    ProcRenderChangePicture,
    ProcRenderSetPictureClipRectangles,
    ProcRenderFreePicture,
    ProcRenderComposite,
    ProcRenderScale,
    ProcRenderTrapezoids,
    ProcRenderTriangles,
    ProcRenderTriStrip,
    ProcRenderTriFan,
    ProcRenderColorTrapezoids,
    ProcRenderColorTriangles,
    ProcRenderTransform,
    ProcRenderCreateGlyphSet,
    ProcRenderReferenceGlyphSet,
    ProcRenderFreeGlyphSet,
    ProcRenderAddGlyphs,
    ProcRenderAddGlyphsFromPicture,
    ProcRenderFreeGlyphs,
    ProcRenderCompositeGlyphs,
    ProcRenderCompositeGlyphs,
    ProcRenderCompositeGlyphs,
    ProcRenderFillRectangles,
    ProcRenderCreateCursor,
    ProcRenderSetPictureTransform,
    ProcRenderQueryFilters,
    ProcRenderSetPictureFilter,
    ProcRenderCreateAnimCursor,
};

int	(*SProcRenderVector[RenderNumberRequests])(ClientPtr) = {
    SProcRenderQueryVersion,
    SProcRenderQueryPictFormats,
    SProcRenderQueryPictIndexValues,
    SProcRenderQueryDithers,
    SProcRenderCreatePicture,
    SProcRenderChangePicture,
    SProcRenderSetPictureClipRectangles,
    SProcRenderFreePicture,
    SProcRenderComposite,
    SProcRenderScale,
    SProcRenderTrapezoids,
    SProcRenderTriangles,
    SProcRenderTriStrip,
    SProcRenderTriFan,
    SProcRenderColorTrapezoids,
    SProcRenderColorTriangles,
    SProcRenderTransform,
    SProcRenderCreateGlyphSet,
    SProcRenderReferenceGlyphSet,
    SProcRenderFreeGlyphSet,
    SProcRenderAddGlyphs,
    SProcRenderAddGlyphsFromPicture,
    SProcRenderFreeGlyphs,
    SProcRenderCompositeGlyphs,
    SProcRenderCompositeGlyphs,
    SProcRenderCompositeGlyphs,
    SProcRenderFillRectangles,
    SProcRenderCreateCursor,
    SProcRenderSetPictureTransform,
    SProcRenderQueryFilters,
    SProcRenderSetPictureFilter,
    SProcRenderCreateAnimCursor,
};

static void
RenderResetProc (ExtensionEntry *extEntry);
    
#if 0
static CARD8	RenderReqCode;
#endif
int	RenderErrBase;
int	RenderClientPrivateIndex;

typedef struct _RenderClient {
    int	    major_version;
    int	    minor_version;
} RenderClientRec, *RenderClientPtr;

#define GetRenderClient(pClient)    ((RenderClientPtr) (pClient)->devPrivates[RenderClientPrivateIndex].ptr)

static void
RenderClientCallback (CallbackListPtr	*list,
		      pointer		closure,
		      pointer		data)
{
    NewClientInfoRec	*clientinfo = (NewClientInfoRec *) data;
    ClientPtr		pClient = clientinfo->client;
    RenderClientPtr	pRenderClient = GetRenderClient (pClient);

    pRenderClient->major_version = 0;
    pRenderClient->minor_version = 0;
}

void
RenderExtensionInit (void)
{
    ExtensionEntry *extEntry;

    if (!PictureType)
	return;
    if (!PictureFinishInit ())
	return;
    RenderClientPrivateIndex = AllocateClientPrivateIndex ();
    if (!AllocateClientPrivate (RenderClientPrivateIndex, 
				sizeof (RenderClientRec)))
	return;
    if (!AddCallback (&ClientStateCallback, RenderClientCallback, 0))
	return;

    extEntry = AddExtension (RENDER_NAME, 0, RenderNumberErrors,
			     ProcRenderDispatch, SProcRenderDispatch,
			     RenderResetProc, StandardMinorOpcode);
    if (!extEntry)
	return;
#if 0
    RenderReqCode = (CARD8) extEntry->base;
#endif
    RenderErrBase = extEntry->errorBase;
}

static void
RenderResetProc (ExtensionEntry *extEntry)
{
}

static int
ProcRenderQueryVersion (ClientPtr client)
{
    RenderClientPtr pRenderClient = GetRenderClient (client);
    xRenderQueryVersionReply rep;
    register int n;
    REQUEST(xRenderQueryVersionReq);

    pRenderClient->major_version = stuff->majorVersion;
    pRenderClient->minor_version = stuff->minorVersion;

    REQUEST_SIZE_MATCH(xRenderQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = RENDER_MAJOR;
    rep.minorVersion = RENDER_MINOR;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swapl(&rep.majorVersion, n);
	swapl(&rep.minorVersion, n);
    }
    WriteToClient(client, sizeof(xRenderQueryVersionReply), (char *)&rep);
    return (client->noClientException);
}

#if 0
static int
VisualDepth (ScreenPtr pScreen, VisualPtr pVisual)
{
    DepthPtr    pDepth;
    int		d, v;

    for (d = 0; d < pScreen->numDepths; d++)
    {
	pDepth = pScreen->allowedDepths + d;
	for (v = 0; v < pDepth->numVids; v++)
	{
	    if (pDepth->vids[v] == pVisual->vid)
		return pDepth->depth;
	}
    }
    return 0;
}
#endif

static VisualPtr
findVisual (ScreenPtr pScreen, VisualID vid)
{
    VisualPtr	pVisual;
    int		v;

    for (v = 0; v < pScreen->numVisuals; v++)
    {
	pVisual = pScreen->visuals + v;
	if (pVisual->vid == vid)
	    return pVisual;
    }
    return 0;
}

extern char *ConnectionInfo;

static int
ProcRenderQueryPictFormats (ClientPtr client)
{
    RenderClientPtr		    pRenderClient = GetRenderClient (client);
    xRenderQueryPictFormatsReply    *reply;
    xPictScreen			    *pictScreen;
    xPictDepth			    *pictDepth;
    xPictVisual			    *pictVisual;
    xPictFormInfo		    *pictForm;
    CARD32			    *pictSubpixel;
    ScreenPtr			    pScreen;
    VisualPtr			    pVisual;
    DepthPtr			    pDepth;
    int				    v, d;
    PictureScreenPtr		    ps;
    PictFormatPtr		    pFormat;
    int				    nformat;
    int				    ndepth;
    int				    nvisual;
    int				    rlength;
    int				    s;
    int				    n;
    int				    numScreens;
    int				    numSubpixel;
/*    REQUEST(xRenderQueryPictFormatsReq); */

    REQUEST_SIZE_MATCH(xRenderQueryPictFormatsReq);

#ifdef PANORAMIX
    if (noPanoramiXExtension)
	numScreens = screenInfo.numScreens;
    else 
        numScreens = ((xConnSetup *)ConnectionInfo)->numRoots;
#else
    numScreens = screenInfo.numScreens;
#endif
    ndepth = nformat = nvisual = 0;
    for (s = 0; s < numScreens; s++)
    {
	pScreen = screenInfo.screens[s];
	for (d = 0; d < pScreen->numDepths; d++)
	{
	    pDepth = pScreen->allowedDepths + d;
	    ++ndepth;

	    for (v = 0; v < pDepth->numVids; v++)
	    {
		pVisual = findVisual (pScreen, pDepth->vids[v]);
		if (pVisual && PictureMatchVisual (pScreen, pDepth->depth, pVisual))
		    ++nvisual;
	    }
	}
	ps = GetPictureScreenIfSet(pScreen);
	if (ps)
	    nformat += ps->nformats;
    }
    if (pRenderClient->major_version == 0 && pRenderClient->minor_version < 6)
	numSubpixel = 0;
    else
	numSubpixel = numScreens;
    
    rlength = (sizeof (xRenderQueryPictFormatsReply) +
	       nformat * sizeof (xPictFormInfo) +
	       numScreens * sizeof (xPictScreen) +
	       ndepth * sizeof (xPictDepth) +
	       nvisual * sizeof (xPictVisual) +
	       numSubpixel * sizeof (CARD32));
    reply = (xRenderQueryPictFormatsReply *) xalloc (rlength);
    if (!reply)
	return BadAlloc;
    reply->type = X_Reply;
    reply->sequenceNumber = client->sequence;
    reply->length = (rlength - sizeof(xGenericReply)) >> 2;
    reply->numFormats = nformat;
    reply->numScreens = numScreens;
    reply->numDepths = ndepth;
    reply->numVisuals = nvisual;
    reply->numSubpixel = numSubpixel;
    
    pictForm = (xPictFormInfo *) (reply + 1);
    
    for (s = 0; s < numScreens; s++)
    {
	pScreen = screenInfo.screens[s];
	ps = GetPictureScreenIfSet(pScreen);
	if (ps)
	{
	    for (nformat = 0, pFormat = ps->formats; 
		 nformat < ps->nformats;
		 nformat++, pFormat++)
	    {
		pictForm->id = pFormat->id;
		pictForm->type = pFormat->type;
		pictForm->depth = pFormat->depth;
		pictForm->direct.red = pFormat->direct.red;
		pictForm->direct.redMask = pFormat->direct.redMask;
		pictForm->direct.green = pFormat->direct.green;
		pictForm->direct.greenMask = pFormat->direct.greenMask;
		pictForm->direct.blue = pFormat->direct.blue;
		pictForm->direct.blueMask = pFormat->direct.blueMask;
		pictForm->direct.alpha = pFormat->direct.alpha;
		pictForm->direct.alphaMask = pFormat->direct.alphaMask;
		if (pFormat->type == PictTypeIndexed && pFormat->index.pColormap)
		    pictForm->colormap = pFormat->index.pColormap->mid;
		else
		    pictForm->colormap = None;
		if (client->swapped)
		{
		    swapl (&pictForm->id, n);
		    swaps (&pictForm->direct.red, n);
		    swaps (&pictForm->direct.redMask, n);
		    swaps (&pictForm->direct.green, n);
		    swaps (&pictForm->direct.greenMask, n);
		    swaps (&pictForm->direct.blue, n);
		    swaps (&pictForm->direct.blueMask, n);
		    swaps (&pictForm->direct.alpha, n);
		    swaps (&pictForm->direct.alphaMask, n);
		    swapl (&pictForm->colormap, n);
		}
		pictForm++;
	    }
	}
    }
    
    pictScreen = (xPictScreen *) pictForm;
    for (s = 0; s < numScreens; s++)
    {
	pScreen = screenInfo.screens[s];
	pictDepth = (xPictDepth *) (pictScreen + 1);
	ndepth = 0;
	for (d = 0; d < pScreen->numDepths; d++)
	{
	    pictVisual = (xPictVisual *) (pictDepth + 1);
	    pDepth = pScreen->allowedDepths + d;

	    nvisual = 0;
	    for (v = 0; v < pDepth->numVids; v++)
	    {
		pVisual = findVisual (pScreen, pDepth->vids[v]);
		if (pVisual && (pFormat = PictureMatchVisual (pScreen, 
							      pDepth->depth, 
							      pVisual)))
		{
		    pictVisual->visual = pVisual->vid;
		    pictVisual->format = pFormat->id;
		    if (client->swapped)
		    {
			swapl (&pictVisual->visual, n);
			swapl (&pictVisual->format, n);
		    }
		    pictVisual++;
		    nvisual++;
		}
	    }
	    pictDepth->depth = pDepth->depth;
	    pictDepth->nPictVisuals = nvisual;
	    if (client->swapped)
	    {
		swaps (&pictDepth->nPictVisuals, n);
	    }
	    ndepth++;
	    pictDepth = (xPictDepth *) pictVisual;
	}
	pictScreen->nDepth = ndepth;
	ps = GetPictureScreenIfSet(pScreen);
	if (ps)
	    pictScreen->fallback = ps->fallback->id;
	else
	    pictScreen->fallback = 0;
	if (client->swapped)
	{
	    swapl (&pictScreen->nDepth, n);
	    swapl (&pictScreen->fallback, n);
	}
	pictScreen = (xPictScreen *) pictDepth;
    }
    pictSubpixel = (CARD32 *) pictScreen;
    
    for (s = 0; s < numSubpixel; s++)
    {
	pScreen = screenInfo.screens[s];
	ps = GetPictureScreenIfSet(pScreen);
	if (ps)
	    *pictSubpixel = ps->subpixel;
	else
	    *pictSubpixel = SubPixelUnknown;
	if (client->swapped)
	{
	    swapl (pictSubpixel, n);
	}
	++pictSubpixel;
    }
    
    if (client->swapped)
    {
	swaps (&reply->sequenceNumber, n);
	swapl (&reply->length, n);
	swapl (&reply->numFormats, n);
	swapl (&reply->numScreens, n);
	swapl (&reply->numDepths, n);
	swapl (&reply->numVisuals, n);
	swapl (&reply->numSubpixel, n);
    }
    WriteToClient(client, rlength, (char *) reply);
    xfree (reply);
    return client->noClientException;
}

static int
ProcRenderQueryPictIndexValues (ClientPtr client)
{
    PictFormatPtr   pFormat;
    int		    num;
    int		    rlength;
    int		    i, n;
    REQUEST(xRenderQueryPictIndexValuesReq);
    xRenderQueryPictIndexValuesReply *reply;
    xIndexValue	    *values;

    REQUEST_AT_LEAST_SIZE(xRenderQueryPictIndexValuesReq);

    pFormat = (PictFormatPtr) SecurityLookupIDByType (client, 
						      stuff->format,
						      PictFormatType,
						      SecurityReadAccess);

    if (!pFormat)
    {
	client->errorValue = stuff->format;
	return RenderErrBase + BadPictFormat;
    }
    if (pFormat->type != PictTypeIndexed)
    {
	client->errorValue = stuff->format;
	return BadMatch;
    }
    num = pFormat->index.nvalues;
    rlength = (sizeof (xRenderQueryPictIndexValuesReply) + 
	       num * sizeof(xIndexValue));
    reply = (xRenderQueryPictIndexValuesReply *) xalloc (rlength);
    if (!reply)
	return BadAlloc;

    reply->type = X_Reply;
    reply->sequenceNumber = client->sequence;
    reply->length = (rlength - sizeof(xGenericReply)) >> 2;
    reply->numIndexValues = num;

    values = (xIndexValue *) (reply + 1);
    
    memcpy (reply + 1, pFormat->index.pValues, num * sizeof (xIndexValue));
    
    if (client->swapped)
    {
	for (i = 0; i < num; i++)
	{
	    swapl (&values[i].pixel, n);
	    swaps (&values[i].red, n);
	    swaps (&values[i].green, n);
	    swaps (&values[i].blue, n);
	    swaps (&values[i].alpha, n);
	}
	swaps (&reply->sequenceNumber, n);
	swapl (&reply->length, n);
	swapl (&reply->numIndexValues, n);
    }

    WriteToClient(client, rlength, (char *) reply);
    xfree(reply);
    return (client->noClientException);
}

static int
ProcRenderQueryDithers (ClientPtr client)
{
    return BadImplementation;
}

static int
ProcRenderCreatePicture (ClientPtr client)
{
    PicturePtr	    pPicture;
    DrawablePtr	    pDrawable;
    PictFormatPtr   pFormat;
    int		    len;
    int		    error;
    REQUEST(xRenderCreatePictureReq);

    REQUEST_AT_LEAST_SIZE(xRenderCreatePictureReq);

    LEGAL_NEW_RESOURCE(stuff->pid, client);
    SECURITY_VERIFY_DRAWABLE(pDrawable, stuff->drawable, client,
			     SecurityWriteAccess);
    pFormat = (PictFormatPtr) SecurityLookupIDByType (client, 
						      stuff->format,
						      PictFormatType,
						      SecurityReadAccess);
    if (!pFormat)
    {
	client->errorValue = stuff->format;
	return RenderErrBase + BadPictFormat;
    }
    if (pFormat->depth != pDrawable->depth)
	return BadMatch;
    len = client->req_len - (sizeof(xRenderCreatePictureReq) >> 2);
    if (Ones(stuff->mask) != len)
	return BadLength;
    
    pPicture = CreatePicture (stuff->pid,
			      pDrawable,
			      pFormat,
			      stuff->mask,
			      (XID *) (stuff + 1),
			      client,
			      &error);
    if (!pPicture)
	return error;
    if (!AddResource (stuff->pid, PictureType, (pointer)pPicture))
	return BadAlloc;
    return Success;
}

static int
ProcRenderChangePicture (ClientPtr client)
{
    PicturePtr	    pPicture;
    REQUEST(xRenderChangePictureReq);
    int len;

    REQUEST_AT_LEAST_SIZE(xRenderChangePictureReq);
    VERIFY_PICTURE (pPicture, stuff->picture, client, SecurityWriteAccess,
		    RenderErrBase + BadPicture);
    len = client->req_len - (sizeof(xRenderChangePictureReq) >> 2);
    if (Ones(stuff->mask) != len)
	return BadLength;
    
    return ChangePicture (pPicture, stuff->mask, (XID *) (stuff + 1),
			  (DevUnion *) 0, client);
}

static int
ProcRenderSetPictureClipRectangles (ClientPtr client)
{
    REQUEST(xRenderSetPictureClipRectanglesReq);
    PicturePtr	    pPicture;
    int		    nr;
    int		    result;

    REQUEST_AT_LEAST_SIZE(xRenderSetPictureClipRectanglesReq);
    VERIFY_PICTURE (pPicture, stuff->picture, client, SecurityWriteAccess,
		    RenderErrBase + BadPicture);
    nr = (client->req_len << 2) - sizeof(xRenderChangePictureReq);
    if (nr & 4)
	return BadLength;
    nr >>= 3;
    result = SetPictureClipRects (pPicture, 
				  stuff->xOrigin, stuff->yOrigin,
				  nr, (xRectangle *) &stuff[1]);
    if (client->noClientException != Success)
        return(client->noClientException);
    else
        return(result);
}

static int
ProcRenderFreePicture (ClientPtr client)
{
    PicturePtr	pPicture;
    REQUEST(xRenderFreePictureReq);

    REQUEST_SIZE_MATCH(xRenderFreePictureReq);

    VERIFY_PICTURE (pPicture, stuff->picture, client, SecurityDestroyAccess,
		    RenderErrBase + BadPicture);
    FreeResource (stuff->picture, RT_NONE);
    return(client->noClientException);
}

static Bool
PictOpValid (CARD8 op)
{
    if (/*PictOpMinimum <= op && */ op <= PictOpMaximum)
	return TRUE;
    if (PictOpDisjointMinimum <= op && op <= PictOpDisjointMaximum)
	return TRUE;
    if (PictOpConjointMinimum <= op && op <= PictOpConjointMaximum)
	return TRUE;
    return FALSE;
}

static int
ProcRenderComposite (ClientPtr client)
{
    PicturePtr	pSrc, pMask, pDst;
    REQUEST(xRenderCompositeReq);

    REQUEST_SIZE_MATCH(xRenderCompositeReq);
    if (!PictOpValid (stuff->op))
    {
	client->errorValue = stuff->op;
	return BadValue;
    }
    VERIFY_PICTURE (pSrc, stuff->src, client, SecurityReadAccess, 
		    RenderErrBase + BadPicture);
    VERIFY_ALPHA (pMask, stuff->mask, client, SecurityReadAccess, 
		  RenderErrBase + BadPicture);
    VERIFY_PICTURE (pDst, stuff->dst, client, SecurityWriteAccess, 
		    RenderErrBase + BadPicture);
    if (pSrc->pDrawable->pScreen != pDst->pDrawable->pScreen ||
	(pMask && pSrc->pDrawable->pScreen != pMask->pDrawable->pScreen))
	return BadMatch;
    CompositePicture (stuff->op,
		      pSrc,
		      pMask,
		      pDst,
		      stuff->xSrc,
		      stuff->ySrc,
		      stuff->xMask,
		      stuff->yMask,
		      stuff->xDst,
		      stuff->yDst,
		      stuff->width,
		      stuff->height);
    return Success;
}

static int
ProcRenderScale (ClientPtr client)
{
    return BadImplementation;
}

static int
ProcRenderTrapezoids (ClientPtr client)
{
    int		ntraps;
    PicturePtr	pSrc, pDst;
    PictFormatPtr   pFormat;
    REQUEST(xRenderTrapezoidsReq);

    REQUEST_AT_LEAST_SIZE(xRenderTrapezoidsReq);
    if (!PictOpValid (stuff->op))
    {
	client->errorValue = stuff->op;
	return BadValue;
    }
    VERIFY_PICTURE (pSrc, stuff->src, client, SecurityReadAccess, 
		    RenderErrBase + BadPicture);
    VERIFY_PICTURE (pDst, stuff->dst, client, SecurityWriteAccess, 
		    RenderErrBase + BadPicture);
    if (pSrc->pDrawable->pScreen != pDst->pDrawable->pScreen)
	return BadMatch;
    if (stuff->maskFormat)
    {
	pFormat = (PictFormatPtr) SecurityLookupIDByType (client,
							  stuff->maskFormat,
							  PictFormatType,
							  SecurityReadAccess);
	if (!pFormat)
	{
	    client->errorValue = stuff->maskFormat;
	    return RenderErrBase + BadPictFormat;
	}
    }
    else
	pFormat = 0;
    ntraps = (client->req_len << 2) - sizeof (xRenderTrapezoidsReq);
    if (ntraps % sizeof (xTrapezoid))
	return BadLength;
    ntraps /= sizeof (xTrapezoid);
    if (ntraps)
	CompositeTrapezoids (stuff->op, pSrc, pDst, pFormat,
			     stuff->xSrc, stuff->ySrc,
			     ntraps, (xTrapezoid *) &stuff[1]);
    return client->noClientException;
}

static int
ProcRenderTriangles (ClientPtr client)
{
    int		ntris;
    PicturePtr	pSrc, pDst;
    PictFormatPtr   pFormat;
    REQUEST(xRenderTrianglesReq);

    REQUEST_AT_LEAST_SIZE(xRenderTrianglesReq);
    if (!PictOpValid (stuff->op))
    {
	client->errorValue = stuff->op;
	return BadValue;
    }
    VERIFY_PICTURE (pSrc, stuff->src, client, SecurityReadAccess, 
		    RenderErrBase + BadPicture);
    VERIFY_PICTURE (pDst, stuff->dst, client, SecurityWriteAccess, 
		    RenderErrBase + BadPicture);
    if (pSrc->pDrawable->pScreen != pDst->pDrawable->pScreen)
	return BadMatch;
    if (stuff->maskFormat)
    {
	pFormat = (PictFormatPtr) SecurityLookupIDByType (client,
							  stuff->maskFormat,
							  PictFormatType,
							  SecurityReadAccess);
	if (!pFormat)
	{
	    client->errorValue = stuff->maskFormat;
	    return RenderErrBase + BadPictFormat;
	}
    }
    else
	pFormat = 0;
    ntris = (client->req_len << 2) - sizeof (xRenderTrianglesReq);
    if (ntris % sizeof (xTriangle))
	return BadLength;
    ntris /= sizeof (xTriangle);
    if (ntris)
	CompositeTriangles (stuff->op, pSrc, pDst, pFormat,
			    stuff->xSrc, stuff->ySrc,
			    ntris, (xTriangle *) &stuff[1]);
    return client->noClientException;
}

static int
ProcRenderTriStrip (ClientPtr client)
{
    int		npoints;
    PicturePtr	pSrc, pDst;
    PictFormatPtr   pFormat;
    REQUEST(xRenderTrianglesReq);

    REQUEST_AT_LEAST_SIZE(xRenderTrianglesReq);
    if (!PictOpValid (stuff->op))
    {
	client->errorValue = stuff->op;
	return BadValue;
    }
    VERIFY_PICTURE (pSrc, stuff->src, client, SecurityReadAccess, 
		    RenderErrBase + BadPicture);
    VERIFY_PICTURE (pDst, stuff->dst, client, SecurityWriteAccess, 
		    RenderErrBase + BadPicture);
    if (pSrc->pDrawable->pScreen != pDst->pDrawable->pScreen)
	return BadMatch;
    if (stuff->maskFormat)
    {
	pFormat = (PictFormatPtr) SecurityLookupIDByType (client,
							  stuff->maskFormat,
							  PictFormatType,
							  SecurityReadAccess);
	if (!pFormat)
	{
	    client->errorValue = stuff->maskFormat;
	    return RenderErrBase + BadPictFormat;
	}
    }
    else
	pFormat = 0;
    npoints = ((client->req_len << 2) - sizeof (xRenderTriStripReq));
    if (npoints & 4)
	return(BadLength);
    npoints >>= 3;
    if (npoints >= 3)
	CompositeTriStrip (stuff->op, pSrc, pDst, pFormat,
			   stuff->xSrc, stuff->ySrc,
			   npoints, (xPointFixed *) &stuff[1]);
    return client->noClientException;
}

static int
ProcRenderTriFan (ClientPtr client)
{
    int		npoints;
    PicturePtr	pSrc, pDst;
    PictFormatPtr   pFormat;
    REQUEST(xRenderTrianglesReq);

    REQUEST_AT_LEAST_SIZE(xRenderTrianglesReq);
    if (!PictOpValid (stuff->op))
    {
	client->errorValue = stuff->op;
	return BadValue;
    }
    VERIFY_PICTURE (pSrc, stuff->src, client, SecurityReadAccess, 
		    RenderErrBase + BadPicture);
    VERIFY_PICTURE (pDst, stuff->dst, client, SecurityWriteAccess, 
		    RenderErrBase + BadPicture);
    if (pSrc->pDrawable->pScreen != pDst->pDrawable->pScreen)
	return BadMatch;
    if (stuff->maskFormat)
    {
	pFormat = (PictFormatPtr) SecurityLookupIDByType (client,
							  stuff->maskFormat,
							  PictFormatType,
							  SecurityReadAccess);
	if (!pFormat)
	{
	    client->errorValue = stuff->maskFormat;
	    return RenderErrBase + BadPictFormat;
	}
    }
    else
	pFormat = 0;
    npoints = ((client->req_len << 2) - sizeof (xRenderTriStripReq));
    if (npoints & 4)
	return(BadLength);
    npoints >>= 3;
    if (npoints >= 3)
	CompositeTriFan (stuff->op, pSrc, pDst, pFormat,
			 stuff->xSrc, stuff->ySrc,
			 npoints, (xPointFixed *) &stuff[1]);
    return client->noClientException;
}

static int
ProcRenderColorTrapezoids (ClientPtr client)
{
    return BadImplementation;
}

static int
ProcRenderColorTriangles (ClientPtr client)
{
    return BadImplementation;
}

static int
ProcRenderTransform (ClientPtr client)
{
    return BadImplementation;
}

static int
ProcRenderCreateGlyphSet (ClientPtr client)
{
    GlyphSetPtr	    glyphSet;
    PictFormatPtr   format;
    int		    f;
    REQUEST(xRenderCreateGlyphSetReq);

    REQUEST_SIZE_MATCH(xRenderCreateGlyphSetReq);

    LEGAL_NEW_RESOURCE(stuff->gsid, client);
    format = (PictFormatPtr) SecurityLookupIDByType (client,
						     stuff->format,
						     PictFormatType,
						     SecurityReadAccess);
    if (!format)
    {
	client->errorValue = stuff->format;
	return RenderErrBase + BadPictFormat;
    }
    switch (format->depth) {
    case 1:
	f = GlyphFormat1;
	break;
    case 4:
	f = GlyphFormat4;
	break;
    case 8:
	f = GlyphFormat8;
	break;
    case 16:
	f = GlyphFormat16;
	break;
    case 32:
	f = GlyphFormat32;
	break;
    default:
	return BadMatch;
    }
    if (format->type != PictTypeDirect)
	return BadMatch;
    glyphSet = AllocateGlyphSet (f, format);
    if (!glyphSet)
	return BadAlloc;
    if (!AddResource (stuff->gsid, GlyphSetType, (pointer)glyphSet))
	return BadAlloc;
    return Success;
}

static int
ProcRenderReferenceGlyphSet (ClientPtr client)
{
    GlyphSetPtr     glyphSet;
    REQUEST(xRenderReferenceGlyphSetReq);

    REQUEST_SIZE_MATCH(xRenderReferenceGlyphSetReq);

    LEGAL_NEW_RESOURCE(stuff->gsid, client);

    glyphSet = (GlyphSetPtr) SecurityLookupIDByType (client,
						     stuff->existing,
						     GlyphSetType,
						     SecurityWriteAccess);
    if (!glyphSet)
    {
	client->errorValue = stuff->existing;
	return RenderErrBase + BadGlyphSet;
    }
    glyphSet->refcnt++;
    if (!AddResource (stuff->gsid, GlyphSetType, (pointer)glyphSet))
	return BadAlloc;
    return client->noClientException;
}

#define NLOCALDELTA	64
#define NLOCALGLYPH	256

static int
ProcRenderFreeGlyphSet (ClientPtr client)
{
    GlyphSetPtr     glyphSet;
    REQUEST(xRenderFreeGlyphSetReq);

    REQUEST_SIZE_MATCH(xRenderFreeGlyphSetReq);
    glyphSet = (GlyphSetPtr) SecurityLookupIDByType (client,
						     stuff->glyphset,
						     GlyphSetType,
						     SecurityDestroyAccess);
    if (!glyphSet)
    {
	client->errorValue = stuff->glyphset;
	return RenderErrBase + BadGlyphSet;
    }
    FreeResource (stuff->glyphset, RT_NONE);
    return client->noClientException;
}

typedef struct _GlyphNew {
    Glyph	id;
    GlyphPtr    glyph;
} GlyphNewRec, *GlyphNewPtr;

static int
ProcRenderAddGlyphs (ClientPtr client)
{
    GlyphSetPtr     glyphSet;
    REQUEST(xRenderAddGlyphsReq);
    GlyphNewRec	    glyphsLocal[NLOCALGLYPH];
    GlyphNewPtr	    glyphsBase, glyphs;
    GlyphPtr	    glyph;
    int		    remain, nglyphs;
    CARD32	    *gids;
    xGlyphInfo	    *gi;
    CARD8	    *bits;
    int		    size;
    int		    err = BadAlloc;

    REQUEST_AT_LEAST_SIZE(xRenderAddGlyphsReq);
    glyphSet = (GlyphSetPtr) SecurityLookupIDByType (client,
						     stuff->glyphset,
						     GlyphSetType,
						     SecurityWriteAccess);
    if (!glyphSet)
    {
	client->errorValue = stuff->glyphset;
	return RenderErrBase + BadGlyphSet;
    }

    nglyphs = stuff->nglyphs;
    if (nglyphs <= NLOCALGLYPH)
	glyphsBase = glyphsLocal;
    else
    {
	glyphsBase = (GlyphNewPtr) ALLOCATE_LOCAL (nglyphs * sizeof (GlyphNewRec));
	if (!glyphsBase)
	    return BadAlloc;
    }

    remain = (client->req_len << 2) - sizeof (xRenderAddGlyphsReq);

    glyphs = glyphsBase;

    gids = (CARD32 *) (stuff + 1);
    gi = (xGlyphInfo *) (gids + nglyphs);
    bits = (CARD8 *) (gi + nglyphs);
    remain -= (sizeof (CARD32) + sizeof (xGlyphInfo)) * nglyphs;
    while (remain >= 0 && nglyphs)
    {
	glyph = AllocateGlyph (gi, glyphSet->fdepth);
	if (!glyph)
	{
	    err = BadAlloc;
	    goto bail;
	}
	
	glyphs->glyph = glyph;
	glyphs->id = *gids;	
	
	size = glyph->size - sizeof (xGlyphInfo);
	if (remain < size)
	    break;
	memcpy ((CARD8 *) (glyph + 1), bits, size);
	
	if (size & 3)
	    size += 4 - (size & 3);
	bits += size;
	remain -= size;
	gi++;
	gids++;
	glyphs++;
	nglyphs--;
    }
    if (nglyphs || remain)
    {
	err = BadLength;
	goto bail;
    }
    nglyphs = stuff->nglyphs;
    if (!ResizeGlyphSet (glyphSet, nglyphs))
    {
	err = BadAlloc;
	goto bail;
    }
    glyphs = glyphsBase;
    while (nglyphs--)
	AddGlyph (glyphSet, glyphs->glyph, glyphs->id);

    if (glyphsBase != glyphsLocal)
	DEALLOCATE_LOCAL (glyphsBase);
    return client->noClientException;
bail:
    while (glyphs != glyphsBase)
    {
	--glyphs;
	xfree (glyphs->glyph);
    }
    if (glyphsBase != glyphsLocal)
	DEALLOCATE_LOCAL (glyphsBase);
    return err;
}

static int
ProcRenderAddGlyphsFromPicture (ClientPtr client)
{
    return BadImplementation;
}

static int
ProcRenderFreeGlyphs (ClientPtr client)
{
    REQUEST(xRenderFreeGlyphsReq);
    GlyphSetPtr     glyphSet;
    int		    nglyph;
    CARD32	    *gids;
    CARD32	    glyph;

    REQUEST_AT_LEAST_SIZE(xRenderFreeGlyphsReq);
    glyphSet = (GlyphSetPtr) SecurityLookupIDByType (client,
						     stuff->glyphset,
						     GlyphSetType,
						     SecurityWriteAccess);
    if (!glyphSet)
    {
	client->errorValue = stuff->glyphset;
	return RenderErrBase + BadGlyphSet;
    }
    nglyph = ((client->req_len << 2) - sizeof (xRenderFreeGlyphsReq)) >> 2;
    gids = (CARD32 *) (stuff + 1);
    while (nglyph-- > 0)
    {
	glyph = *gids++;
	if (!DeleteGlyph (glyphSet, glyph))
	{
	    client->errorValue = glyph;
	    return RenderErrBase + BadGlyph;
	}
    }
    return client->noClientException;
}

static int
ProcRenderCompositeGlyphs (ClientPtr client)
{
    GlyphSetPtr     glyphSet;
    GlyphSet	    gs;
    PicturePtr      pSrc, pDst;
    PictFormatPtr   pFormat;
    GlyphListRec    listsLocal[NLOCALDELTA];
    GlyphListPtr    lists, listsBase;
    GlyphPtr	    glyphsLocal[NLOCALGLYPH];
    Glyph	    glyph;
    GlyphPtr	    *glyphs, *glyphsBase;
    xGlyphElt	    *elt;
    CARD8	    *buffer, *end;
    int		    nglyph;
    int		    nlist;
    int		    space;
    int		    size;
    int		    n;
    
    REQUEST(xRenderCompositeGlyphsReq);

    REQUEST_AT_LEAST_SIZE(xRenderCompositeGlyphsReq);

    switch (stuff->renderReqType) {
    default:			    size = 1; break;
    case X_RenderCompositeGlyphs16: size = 2; break;
    case X_RenderCompositeGlyphs32: size = 4; break;
    }
	    
    if (!PictOpValid (stuff->op))
    {
	client->errorValue = stuff->op;
	return BadValue;
    }
    VERIFY_PICTURE (pSrc, stuff->src, client, SecurityReadAccess,
		    RenderErrBase + BadPicture);
    VERIFY_PICTURE (pDst, stuff->dst, client, SecurityWriteAccess,
		    RenderErrBase + BadPicture);
    if (pSrc->pDrawable->pScreen != pDst->pDrawable->pScreen)
	return BadMatch;
    if (stuff->maskFormat)
    {
	pFormat = (PictFormatPtr) SecurityLookupIDByType (client,
							  stuff->maskFormat,
							  PictFormatType,
							  SecurityReadAccess);
	if (!pFormat)
	{
	    client->errorValue = stuff->maskFormat;
	    return RenderErrBase + BadPictFormat;
	}
    }
    else
	pFormat = 0;

    glyphSet = (GlyphSetPtr) SecurityLookupIDByType (client,
						     stuff->glyphset,
						     GlyphSetType,
						     SecurityReadAccess);
    if (!glyphSet)
    {
	client->errorValue = stuff->glyphset;
	return RenderErrBase + BadGlyphSet;
    }

    buffer = (CARD8 *) (stuff + 1);
    end = (CARD8 *) stuff + (client->req_len << 2);
    nglyph = 0;
    nlist = 0;
    while (buffer + sizeof (xGlyphElt) < end)
    {
	elt = (xGlyphElt *) buffer;
	buffer += sizeof (xGlyphElt);
	
	if (elt->len == 0xff)
	{
	    buffer += 4;
	}
	else
	{
	    nlist++;
	    nglyph += elt->len;
	    space = size * elt->len;
	    if (space & 3)
		space += 4 - (space & 3);
	    buffer += space;
	}
    }
    if (nglyph <= NLOCALGLYPH)
	glyphsBase = glyphsLocal;
    else
    {
	glyphsBase = (GlyphPtr *) ALLOCATE_LOCAL (nglyph * sizeof (GlyphPtr));
	if (!glyphsBase)
	    return BadAlloc;
    }
    if (nlist <= NLOCALDELTA)
	listsBase = listsLocal;
    else
    {
	listsBase = (GlyphListPtr) ALLOCATE_LOCAL (nlist * sizeof (GlyphListRec));
	if (!listsBase)
	    return BadAlloc;
    }
    buffer = (CARD8 *) (stuff + 1);
    glyphs = glyphsBase;
    lists = listsBase;
    while (buffer + sizeof (xGlyphElt) < end)
    {
	elt = (xGlyphElt *) buffer;
	buffer += sizeof (xGlyphElt);
	
	if (elt->len == 0xff)
	{
	    if (buffer + sizeof (GlyphSet) < end)
	    {
		gs = *(GlyphSet *) buffer;
		glyphSet = (GlyphSetPtr) SecurityLookupIDByType (client,
								 gs,
								 GlyphSetType,
								 SecurityReadAccess);
		if (!glyphSet)
		{
		    client->errorValue = gs;
		    if (glyphsBase != glyphsLocal)
			DEALLOCATE_LOCAL (glyphsBase);
		    if (listsBase != listsLocal)
			DEALLOCATE_LOCAL (listsBase);
		    return RenderErrBase + BadGlyphSet;
		}
	    }
	    buffer += 4;
	}
	else
	{
	    lists->xOff = elt->deltax;
	    lists->yOff = elt->deltay;
	    lists->format = glyphSet->format;
	    lists->len = 0;
	    n = elt->len;
	    while (n--)
	    {
		if (buffer + size <= end)
		{
		    switch (size) {
		    case 1:
			glyph = *((CARD8 *)buffer); break;
		    case 2:
			glyph = *((CARD16 *)buffer); break;
		    case 4:
		    default:
			glyph = *((CARD32 *)buffer); break;
		    }
		    if ((*glyphs = FindGlyph (glyphSet, glyph)))
		    {
			lists->len++;
			glyphs++;
		    }
		}
		buffer += size;
	    }
	    space = size * elt->len;
	    if (space & 3)
		buffer += 4 - (space & 3);
	    lists++;
	}
    }
    if (buffer > end)
	return BadLength;

    CompositeGlyphs (stuff->op,
		     pSrc,
		     pDst,
		     pFormat,
		     stuff->xSrc,
		     stuff->ySrc,
		     nlist,
		     listsBase,
		     glyphsBase);

    if (glyphsBase != glyphsLocal)
	DEALLOCATE_LOCAL (glyphsBase);
    if (listsBase != listsLocal)
	DEALLOCATE_LOCAL (listsBase);
    
    return client->noClientException;
}

static int
ProcRenderFillRectangles (ClientPtr client)
{
    PicturePtr	    pDst;
    int             things;
    REQUEST(xRenderFillRectanglesReq);
    
    REQUEST_AT_LEAST_SIZE (xRenderFillRectanglesReq);
    if (!PictOpValid (stuff->op))
    {
	client->errorValue = stuff->op;
	return BadValue;
    }
    VERIFY_PICTURE (pDst, stuff->dst, client, SecurityWriteAccess, 
		    RenderErrBase + BadPicture);
    
    things = (client->req_len << 2) - sizeof(xRenderFillRectanglesReq);
    if (things & 4)
	return(BadLength);
    things >>= 3;
    
    CompositeRects (stuff->op,
		    pDst,
		    &stuff->color,
		    things,
		    (xRectangle *) &stuff[1]);
    
    return client->noClientException;
}

static void
SetBit (unsigned char *line, int x, int bit)
{
    unsigned char   mask;
    
    if (screenInfo.bitmapBitOrder == LSBFirst)
	mask = (1 << (x & 7));
    else
	mask = (0x80 >> (x & 7));
    /* XXX assumes byte order is host byte order */
    line += (x >> 3);
    if (bit)
	*line |= mask;
    else
	*line &= ~mask;
}

#define DITHER_DIM 2

static CARD32 orderedDither[DITHER_DIM][DITHER_DIM] = {
    {  1,  3,  },
    {  4,  2,  },
};

#define DITHER_SIZE  ((sizeof orderedDither / sizeof orderedDither[0][0]) + 1)

static int
ProcRenderCreateCursor (ClientPtr client)
{
    REQUEST(xRenderCreateCursorReq);
    PicturePtr	    pSrc;
    ScreenPtr	    pScreen;
    unsigned short  width, height;
    CARD32	    *argbbits, *argb;
    unsigned char   *srcbits, *srcline;
    unsigned char   *mskbits, *mskline;
    int		    stride;
    int		    x, y;
    int		    nbytes_mono;
    CursorMetricRec cm;
    CursorPtr	    pCursor;
    CARD32	    twocolor[3];
    int		    ncolor;

    REQUEST_SIZE_MATCH (xRenderCreateCursorReq);
    LEGAL_NEW_RESOURCE(stuff->cid, client);
    
    VERIFY_PICTURE (pSrc, stuff->src, client, SecurityReadAccess, 
		    RenderErrBase + BadPicture);
    pScreen = pSrc->pDrawable->pScreen;
    width = pSrc->pDrawable->width;
    height = pSrc->pDrawable->height;
    if ( stuff->x > width 
      || stuff->y > height )
	return (BadMatch);
    argbbits = xalloc (width * height * sizeof (CARD32));
    if (!argbbits)
	return (BadAlloc);
    
    stride = BitmapBytePad(width);
    nbytes_mono = stride*height;
    srcbits = (unsigned char *)xalloc(nbytes_mono);
    if (!srcbits)
    {
	xfree (argbbits);
	return (BadAlloc);
    }
    mskbits = (unsigned char *)xalloc(nbytes_mono);
    if (!mskbits)
    {
	xfree(argbbits);
	xfree(srcbits);
	return (BadAlloc);
    }
    bzero ((char *) mskbits, nbytes_mono);
    bzero ((char *) srcbits, nbytes_mono);

    if (pSrc->format == PICT_a8r8g8b8)
    {
	(*pScreen->GetImage) (pSrc->pDrawable,
			      0, 0, width, height, ZPixmap,
			      0xffffffff, (pointer) argbbits);
    }
    else
    {
	PixmapPtr	pPixmap;
	PicturePtr	pPicture;
	PictFormatPtr	pFormat;
	int		error;

	pFormat = PictureMatchFormat (pScreen, 32, PICT_a8r8g8b8);
	if (!pFormat)
	{
	    xfree (argbbits);
	    xfree (srcbits);
	    xfree (mskbits);
	    return (BadImplementation);
	}
	pPixmap = (*pScreen->CreatePixmap) (pScreen, width, height, 32);
	if (!pPixmap)
	{
	    xfree (argbbits);
	    xfree (srcbits);
	    xfree (mskbits);
	    return (BadAlloc);
	}
	pPicture = CreatePicture (0, &pPixmap->drawable, pFormat, 0, 0, 
				  client, &error);
	if (!pPicture);
	{
	    xfree (argbbits);
	    xfree (srcbits);
	    xfree (mskbits);
	    return error;
	}
	(*pScreen->DestroyPixmap) (pPixmap);
	CompositePicture (PictOpSrc,
			  pSrc, 0, pPicture,
			  0, 0, 0, 0, 0, 0, width, height);
	(*pScreen->GetImage) (pPicture->pDrawable,
			      0, 0, width, height, ZPixmap,
			      0xffffffff, (pointer) argbbits);
	FreePicture (pPicture, 0);
    }
    /*
     * Check whether the cursor can be directly supported by 
     * the core cursor code
     */
    ncolor = 0;
    argb = argbbits;
    for (y = 0; ncolor <= 2 && y < height; y++)
    {
	for (x = 0; ncolor <= 2 && x < width; x++)
	{
	    CARD32  p = *argb++;
	    CARD32  a = (p >> 24);

	    if (a == 0)	    /* transparent */
		continue;
	    if (a == 0xff)  /* opaque */
	    {
		int n;
		for (n = 0; n < ncolor; n++)
		    if (p == twocolor[n])
			break;
		if (n == ncolor)
		    twocolor[ncolor++] = p;
	    }
	    else
		ncolor = 3;
	}
    }
    
    /*
     * Convert argb image to two plane cursor
     */
    srcline = srcbits;
    mskline = mskbits;
    argb = argbbits;
    for (y = 0; y < height; y++)
    {
	for (x = 0; x < width; x++)
	{
	    CARD32  p = *argb++;

	    if (ncolor <= 2)
	    {
		CARD32	a = ((p >> 24));

		SetBit (mskline, x, a != 0);
		SetBit (srcline, x, a != 0 && p == twocolor[0]);
	    }
	    else
	    {
		CARD32	a = ((p >> 24) * DITHER_SIZE + 127) / 255;
		CARD32	i = ((CvtR8G8B8toY15(p) >> 7) * DITHER_SIZE + 127) / 255;
		CARD32	d = orderedDither[y&(DITHER_DIM-1)][x&(DITHER_DIM-1)];
		/* Set mask from dithered alpha value */
		SetBit(mskline, x, a > d);
		/* Set src from dithered intensity value */
		SetBit(srcline, x, a > d && i <= d);
	    }
	}
	srcline += stride;
	mskline += stride;
    }
    /*
     * Dither to white and black if the cursor has more than two colors
     */
    if (ncolor > 2)
    {
	twocolor[0] = 0xff000000;
	twocolor[1] = 0xffffffff;
    }
    else
    {
	xfree (argbbits);
	argbbits = 0;
    }
    
#define GetByte(p,s)	(((p) >> (s)) & 0xff)
#define GetColor(p,s)	(GetByte(p,s) | (GetByte(p,s) << 8))
    
    cm.width = width;
    cm.height = height;
    cm.xhot = stuff->x;
    cm.yhot = stuff->y;
    pCursor = AllocCursorARGB (srcbits, mskbits, argbbits, &cm,
			       GetColor(twocolor[0], 16),
			       GetColor(twocolor[0], 8),
			       GetColor(twocolor[0], 0),
			       GetColor(twocolor[1], 16),
			       GetColor(twocolor[1], 8),
			       GetColor(twocolor[1], 0));
    if (pCursor && AddResource(stuff->cid, RT_CURSOR, (pointer)pCursor))
	return (client->noClientException);
    return BadAlloc;
}

static int
ProcRenderSetPictureTransform (ClientPtr client)
{
    REQUEST(xRenderSetPictureTransformReq);
    PicturePtr	pPicture;
    int		result;

    REQUEST_SIZE_MATCH(xRenderSetPictureTransformReq);
    VERIFY_PICTURE (pPicture, stuff->picture, client, SecurityWriteAccess,
		    RenderErrBase + BadPicture);
    result = SetPictureTransform (pPicture, (PictTransform *) &stuff->transform);
    if (client->noClientException != Success)
        return(client->noClientException);
    else
        return(result);
}

static int
ProcRenderQueryFilters (ClientPtr client)
{
    REQUEST (xRenderQueryFiltersReq);
    DrawablePtr			pDrawable;
    xRenderQueryFiltersReply	*reply;
    int				nbytesName;
    int				nnames;
    ScreenPtr			pScreen;
    PictureScreenPtr		ps;
    int				i, j;
    int				len;
    int				total_bytes;
    INT16			*aliases;
    char			*names;

    REQUEST_SIZE_MATCH(xRenderQueryFiltersReq);
    SECURITY_VERIFY_DRAWABLE(pDrawable, stuff->drawable, client, SecurityReadAccess);
    
    pScreen = pDrawable->pScreen;
    nbytesName = 0;
    nnames = 0;
    ps = GetPictureScreenIfSet(pScreen);
    if (ps)
    {
	for (i = 0; i < ps->nfilters; i++)
	    nbytesName += 1 + strlen (ps->filters[i].name);
	for (i = 0; i < ps->nfilterAliases; i++)
	    nbytesName += 1 + strlen (ps->filterAliases[i].alias);
	nnames = ps->nfilters + ps->nfilterAliases;
    }
    len = ((nnames + 1) >> 1) + ((nbytesName + 3) >> 2);
    total_bytes = sizeof (xRenderQueryFiltersReply) + (len << 2);
    reply = (xRenderQueryFiltersReply *) xalloc (total_bytes);
    if (!reply)
	return BadAlloc;
    aliases = (INT16 *) (reply + 1);
    names = (char *) (aliases + ((nnames + 1) & ~1));
    
    reply->type = X_Reply;
    reply->sequenceNumber = client->sequence;
    reply->length = len;
    reply->numAliases = nnames;
    reply->numFilters = nnames;
    if (ps)
    {

	/* fill in alias values */
	for (i = 0; i < ps->nfilters; i++)
	    aliases[i] = FilterAliasNone;
	for (i = 0; i < ps->nfilterAliases; i++)
	{
	    for (j = 0; j < ps->nfilters; j++)
		if (ps->filterAliases[i].filter_id == ps->filters[j].id)
		    break;
	    if (j == ps->nfilters)
	    {
		for (j = 0; j < ps->nfilterAliases; j++)
		    if (ps->filterAliases[i].filter_id == 
			ps->filterAliases[j].alias_id)
		    {
			break;
		    }
		if (j == ps->nfilterAliases)
		    j = FilterAliasNone;
		else
		    j = j + ps->nfilters;
	    }
	    aliases[i + ps->nfilters] = j;
	}

	/* fill in filter names */
	for (i = 0; i < ps->nfilters; i++)
	{
	    j = strlen (ps->filters[i].name);
	    *names++ = j;
	    strncpy (names, ps->filters[i].name, j);
	    names += j;
	}
	
	/* fill in filter alias names */
	for (i = 0; i < ps->nfilterAliases; i++)
	{
	    j = strlen (ps->filterAliases[i].alias);
	    *names++ = j;
	    strncpy (names, ps->filterAliases[i].alias, j);
	    names += j;
	}
    }

    if (client->swapped)
    {
	register int n;

	for (i = 0; i < reply->numAliases; i++)
	{
	    swaps (&aliases[i], n);
	}
    	swaps(&reply->sequenceNumber, n);
    	swapl(&reply->length, n);
	swapl(&reply->numAliases, n);
	swapl(&reply->numFilters, n);
    }
    WriteToClient(client, total_bytes, (char *) reply);
    xfree (reply);
    
    return(client->noClientException);
}

static int
ProcRenderSetPictureFilter (ClientPtr client)
{
    REQUEST (xRenderSetPictureFilterReq);
    PicturePtr	pPicture;
    int		result;
    xFixed	*params;
    int		nparams;
    char	*name;
    
    REQUEST_AT_LEAST_SIZE (xRenderSetPictureFilterReq);
    VERIFY_PICTURE (pPicture, stuff->picture, client, SecurityWriteAccess,
		    RenderErrBase + BadPicture);
    name = (char *) (stuff + 1);
    params = (xFixed *) (name + ((stuff->nbytes + 3) & ~3));
    nparams = ((xFixed *) stuff + client->req_len) - params;
    result = SetPictureFilter (pPicture, name, stuff->nbytes, params, nparams);
    return result;
}

static int
ProcRenderCreateAnimCursor (ClientPtr client)
{
    REQUEST(xRenderCreateAnimCursorReq);
    CursorPtr	    *cursors;
    CARD32	    *deltas;
    CursorPtr	    pCursor;
    int		    ncursor;
    xAnimCursorElt  *elt;
    int		    i;
    int		    ret;

    REQUEST_AT_LEAST_SIZE(xRenderCreateAnimCursorReq);
    LEGAL_NEW_RESOURCE(stuff->cid, client);
    if (client->req_len & 1)
	return BadLength;
    ncursor = (client->req_len - (SIZEOF(xRenderCreateAnimCursorReq) >> 2)) >> 1;
    cursors = xalloc (ncursor * (sizeof (CursorPtr) + sizeof (CARD32)));
    if (!cursors)
	return BadAlloc;
    deltas = (CARD32 *) (cursors + ncursor);
    elt = (xAnimCursorElt *) (stuff + 1);
    for (i = 0; i < ncursor; i++)
    {
	cursors[i] = (CursorPtr)SecurityLookupIDByType(client, elt->cursor,
						       RT_CURSOR, SecurityReadAccess);
	if (!cursors[i])
	{
	    xfree (cursors);
	    client->errorValue = elt->cursor;
	    return BadCursor;
	}
	deltas[i] = elt->delay;
	elt++;
    }
    ret = AnimCursorCreate (cursors, deltas, ncursor, &pCursor);
    xfree (cursors);
    if (ret != Success)
	return ret;
    
    if (AddResource (stuff->cid, RT_CURSOR, (pointer)pCursor))
	return client->noClientException;
    return BadAlloc;
}

static int
ProcRenderDispatch (ClientPtr client)
{
    REQUEST(xReq);
    
    if (stuff->data < RenderNumberRequests)
	return (*ProcRenderVector[stuff->data]) (client);
    else
	return BadRequest;
}

static int
SProcRenderQueryVersion (ClientPtr client)
{
    register int n;
    REQUEST(xRenderQueryVersionReq);

    swaps(&stuff->length, n);
    swapl(&stuff->majorVersion, n);
    swapl(&stuff->minorVersion, n);
    return (*ProcRenderVector[stuff->renderReqType])(client);
}

static int
SProcRenderQueryPictFormats (ClientPtr client)
{
    register int n;
    REQUEST(xRenderQueryPictFormatsReq);
    swaps(&stuff->length, n);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}

static int
SProcRenderQueryPictIndexValues (ClientPtr client)
{
    register int n;
    REQUEST(xRenderQueryPictIndexValuesReq);
    swaps(&stuff->length, n);
    swapl(&stuff->format, n);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}

static int
SProcRenderQueryDithers (ClientPtr client)
{
    return BadImplementation;
}

static int
SProcRenderCreatePicture (ClientPtr client)
{
    register int n;
    REQUEST(xRenderCreatePictureReq);
    swaps(&stuff->length, n);
    swapl(&stuff->pid, n);
    swapl(&stuff->drawable, n);
    swapl(&stuff->format, n);
    swapl(&stuff->mask, n);
    SwapRestL(stuff);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}

static int
SProcRenderChangePicture (ClientPtr client)
{
    register int n;
    REQUEST(xRenderChangePictureReq);
    swaps(&stuff->length, n);
    swapl(&stuff->picture, n);
    swapl(&stuff->mask, n);
    SwapRestL(stuff);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}

static int
SProcRenderSetPictureClipRectangles (ClientPtr client)
{
    register int n;
    REQUEST(xRenderSetPictureClipRectanglesReq);
    swaps(&stuff->length, n);
    swapl(&stuff->picture, n);
    SwapRestS(stuff);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}

static int
SProcRenderFreePicture (ClientPtr client)
{
    register int n;
    REQUEST(xRenderFreePictureReq);
    swaps(&stuff->length, n);
    swapl(&stuff->picture, n);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}

static int
SProcRenderComposite (ClientPtr client)
{
    register int n;
    REQUEST(xRenderCompositeReq);
    swaps(&stuff->length, n);
    swapl(&stuff->src, n);
    swapl(&stuff->mask, n);
    swapl(&stuff->dst, n);
    swaps(&stuff->xSrc, n);
    swaps(&stuff->ySrc, n);
    swaps(&stuff->xMask, n);
    swaps(&stuff->yMask, n);
    swaps(&stuff->xDst, n);
    swaps(&stuff->yDst, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}

static int
SProcRenderScale (ClientPtr client)
{
    register int n;
    REQUEST(xRenderScaleReq);
    swaps(&stuff->length, n);
    swapl(&stuff->src, n);
    swapl(&stuff->dst, n);
    swapl(&stuff->colorScale, n);
    swapl(&stuff->alphaScale, n);
    swaps(&stuff->xSrc, n);
    swaps(&stuff->ySrc, n);
    swaps(&stuff->xDst, n);
    swaps(&stuff->yDst, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}

static int
SProcRenderTrapezoids (ClientPtr client)
{
    register int n;
    REQUEST(xRenderTrapezoidsReq);

    REQUEST_AT_LEAST_SIZE(xRenderTrapezoidsReq);
    swaps (&stuff->length, n);
    swapl (&stuff->src, n);
    swapl (&stuff->dst, n);
    swapl (&stuff->maskFormat, n);
    swaps (&stuff->xSrc, n);
    swaps (&stuff->ySrc, n);
    SwapRestL(stuff);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}

static int
SProcRenderTriangles (ClientPtr client)
{
    register int n;
    REQUEST(xRenderTrianglesReq);

    REQUEST_AT_LEAST_SIZE(xRenderTrianglesReq);
    swaps (&stuff->length, n);
    swapl (&stuff->src, n);
    swapl (&stuff->dst, n);
    swapl (&stuff->maskFormat, n);
    swaps (&stuff->xSrc, n);
    swaps (&stuff->ySrc, n);
    SwapRestL(stuff);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}

static int
SProcRenderTriStrip (ClientPtr client)
{
    register int n;
    REQUEST(xRenderTriStripReq);

    REQUEST_AT_LEAST_SIZE(xRenderTriStripReq);
    swaps (&stuff->length, n);
    swapl (&stuff->src, n);
    swapl (&stuff->dst, n);
    swapl (&stuff->maskFormat, n);
    swaps (&stuff->xSrc, n);
    swaps (&stuff->ySrc, n);
    SwapRestL(stuff);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}

static int
SProcRenderTriFan (ClientPtr client)
{
    register int n;
    REQUEST(xRenderTriFanReq);

    REQUEST_AT_LEAST_SIZE(xRenderTriFanReq);
    swaps (&stuff->length, n);
    swapl (&stuff->src, n);
    swapl (&stuff->dst, n);
    swapl (&stuff->maskFormat, n);
    swaps (&stuff->xSrc, n);
    swaps (&stuff->ySrc, n);
    SwapRestL(stuff);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}

static int
SProcRenderColorTrapezoids (ClientPtr client)
{
    return BadImplementation;
}

static int
SProcRenderColorTriangles (ClientPtr client)
{
    return BadImplementation;
}

static int
SProcRenderTransform (ClientPtr client)
{
    return BadImplementation;
}

static int
SProcRenderCreateGlyphSet (ClientPtr client)
{
    register int n;
    REQUEST(xRenderCreateGlyphSetReq);
    swaps(&stuff->length, n);
    swapl(&stuff->gsid, n);
    swapl(&stuff->format, n);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}

static int
SProcRenderReferenceGlyphSet (ClientPtr client)
{
    register int n;
    REQUEST(xRenderReferenceGlyphSetReq);
    swaps(&stuff->length, n);
    swapl(&stuff->gsid, n);
    swapl(&stuff->existing, n);
    return (*ProcRenderVector[stuff->renderReqType])  (client);
}

static int
SProcRenderFreeGlyphSet (ClientPtr client)
{
    register int n;
    REQUEST(xRenderFreeGlyphSetReq);
    swaps(&stuff->length, n);
    swapl(&stuff->glyphset, n);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}

static int
SProcRenderAddGlyphs (ClientPtr client)
{
    register int n;
    register int i;
    CARD32  *gids;
    void    *end;
    xGlyphInfo *gi;
    REQUEST(xRenderAddGlyphsReq);
    swaps(&stuff->length, n);
    swapl(&stuff->glyphset, n);
    swapl(&stuff->nglyphs, n);
    if (stuff->nglyphs & 0xe0000000)
	return BadLength;
    end = (CARD8 *) stuff + (client->req_len << 2);
    gids = (CARD32 *) (stuff + 1);
    gi = (xGlyphInfo *) (gids + stuff->nglyphs);
    if ((char *) end - (char *) (gids + stuff->nglyphs) < 0)
	return BadLength;
    if ((char *) end - (char *) (gi + stuff->nglyphs) < 0)
	return BadLength;
    for (i = 0; i < stuff->nglyphs; i++)
    {
	swapl (&gids[i], n);
	swaps (&gi[i].width, n);
	swaps (&gi[i].height, n);
	swaps (&gi[i].x, n);
	swaps (&gi[i].y, n);
	swaps (&gi[i].xOff, n);
	swaps (&gi[i].yOff, n);
    }
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}

static int
SProcRenderAddGlyphsFromPicture (ClientPtr client)
{
    return BadImplementation;
}

static int
SProcRenderFreeGlyphs (ClientPtr client)
{
    register int n;
    REQUEST(xRenderFreeGlyphsReq);
    swaps(&stuff->length, n);
    swapl(&stuff->glyphset, n);
    SwapRestL(stuff);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}

static int
SProcRenderCompositeGlyphs (ClientPtr client)
{
    register int n;
    xGlyphElt	*elt;
    CARD8	*buffer;
    CARD8	*end;
    int		space;
    int		i;
    int		size;
    
    REQUEST(xRenderCompositeGlyphsReq);
    
    switch (stuff->renderReqType) {
    default:			    size = 1; break;
    case X_RenderCompositeGlyphs16: size = 2; break;
    case X_RenderCompositeGlyphs32: size = 4; break;
    }
	    
    swaps(&stuff->length, n);
    swapl(&stuff->src, n);
    swapl(&stuff->dst, n);
    swapl(&stuff->maskFormat, n);
    swapl(&stuff->glyphset, n);
    swaps(&stuff->xSrc, n);
    swaps(&stuff->ySrc, n);
    buffer = (CARD8 *) (stuff + 1);
    end = (CARD8 *) stuff + (client->req_len << 2);
    while (buffer + sizeof (xGlyphElt) < end)
    {
	elt = (xGlyphElt *) buffer;
	buffer += sizeof (xGlyphElt);
	
	swaps (&elt->deltax, n);
	swaps (&elt->deltay, n);
	
	i = elt->len;
	if (i == 0xff)
	{
	    swapl (buffer, n);
	    buffer += 4;
	}
	else
	{
	    space = size * i;
	    switch (size) {
	    case 1:
		buffer += i;
		break;
	    case 2:
		while (i--)
		{
		    swaps (buffer, n);
		    buffer += 2;
		}
		break;
	    case 4:
		while (i--)
		{
		    swapl (buffer, n);
		    buffer += 4;
		}
		break;
	    }
	    if (space & 3)
		buffer += 4 - (space & 3);
	}
    }
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}

static int
SProcRenderFillRectangles (ClientPtr client)
{
    register int n;
    REQUEST(xRenderFillRectanglesReq);

    REQUEST_AT_LEAST_SIZE (xRenderFillRectanglesReq);
    swaps(&stuff->length, n);
    swapl(&stuff->dst, n);
    swaps(&stuff->color.red, n);
    swaps(&stuff->color.green, n);
    swaps(&stuff->color.blue, n);
    swaps(&stuff->color.alpha, n);
    SwapRestS(stuff);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}
    
static int
SProcRenderCreateCursor (ClientPtr client)
{
    register int n;
    REQUEST(xRenderCreateCursorReq);
    REQUEST_SIZE_MATCH (xRenderCreateCursorReq);
    
    swaps(&stuff->length, n);
    swapl(&stuff->cid, n);
    swapl(&stuff->src, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}
    
static int
SProcRenderSetPictureTransform (ClientPtr client)
{
    register int n;
    REQUEST(xRenderSetPictureTransformReq);
    REQUEST_SIZE_MATCH(xRenderSetPictureTransformReq);

    swaps(&stuff->length, n);
    swapl(&stuff->picture, n);
    swapl(&stuff->transform.matrix11, n);
    swapl(&stuff->transform.matrix12, n);
    swapl(&stuff->transform.matrix13, n);
    swapl(&stuff->transform.matrix21, n);
    swapl(&stuff->transform.matrix22, n);
    swapl(&stuff->transform.matrix23, n);
    swapl(&stuff->transform.matrix31, n);
    swapl(&stuff->transform.matrix32, n);
    swapl(&stuff->transform.matrix33, n);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}

static int
SProcRenderQueryFilters (ClientPtr client)
{
    register int n;
    REQUEST (xRenderQueryFiltersReq);
    REQUEST_SIZE_MATCH (xRenderQueryFiltersReq);

    swaps(&stuff->length, n);
    swapl(&stuff->drawable, n);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}
    
static int
SProcRenderSetPictureFilter (ClientPtr client)
{
    register int n;
    REQUEST (xRenderSetPictureFilterReq);
    REQUEST_AT_LEAST_SIZE (xRenderSetPictureFilterReq);

    swaps(&stuff->length, n);
    swapl(&stuff->picture, n);
    swaps(&stuff->nbytes, n);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}
    
static int
SProcRenderCreateAnimCursor (ClientPtr client)
{
    register int n;
    REQUEST (xRenderCreateAnimCursorReq);
    REQUEST_AT_LEAST_SIZE (xRenderCreateAnimCursorReq);

    swaps(&stuff->length, n);
    swapl(&stuff->cid, n);
    SwapRestL(stuff);
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}

static int
SProcRenderDispatch (ClientPtr client)
{
    REQUEST(xReq);
    
    if (stuff->data < RenderNumberRequests)
	return (*SProcRenderVector[stuff->data]) (client);
    else
	return BadRequest;
}

#ifdef PANORAMIX
#include "panoramiX.h"
#include "panoramiXsrv.h"

#define VERIFY_XIN_PICTURE(pPicture, pid, client, mode, err) {\
    pPicture = SecurityLookupIDByType(client, pid, XRT_PICTURE, mode);\
    if (!pPicture) { \
	client->errorValue = pid; \
	return err; \
    } \
}

#define VERIFY_XIN_ALPHA(pPicture, pid, client, mode, err) {\
    if (pid == None) \
	pPicture = 0; \
    else { \
	VERIFY_XIN_PICTURE(pPicture, pid, client, mode, err); \
    } \
} \

int	    (*PanoramiXSaveRenderVector[RenderNumberRequests])(ClientPtr);

unsigned long	XRT_PICTURE;

static int
PanoramiXRenderCreatePicture (ClientPtr client)
{
    REQUEST(xRenderCreatePictureReq);
    PanoramiXRes    *refDraw, *newPict;
    int		    result = Success, j;

    REQUEST_AT_LEAST_SIZE(xRenderCreatePictureReq);
    if(!(refDraw = (PanoramiXRes *)SecurityLookupIDByClass(
		client, stuff->drawable, XRC_DRAWABLE, SecurityWriteAccess)))
	return BadDrawable;
    if(!(newPict = (PanoramiXRes *) xalloc(sizeof(PanoramiXRes))))
	return BadAlloc;
    newPict->type = XRT_PICTURE;
    newPict->info[0].id = stuff->pid;
    
    if (refDraw->type == XRT_WINDOW &&
	stuff->drawable == WindowTable[0]->drawable.id)
    {
	newPict->u.pict.root = TRUE;
    }
    else
	newPict->u.pict.root = FALSE;

    for(j = 1; j < PanoramiXNumScreens; j++)
	newPict->info[j].id = FakeClientID(client->index);
    
    FOR_NSCREENS_BACKWARD(j) {
	stuff->pid = newPict->info[j].id;
	stuff->drawable = refDraw->info[j].id;
	result = (*PanoramiXSaveRenderVector[X_RenderCreatePicture]) (client);
	if(result != Success) break;
    }

    if (result == Success)
	AddResource(newPict->info[0].id, XRT_PICTURE, newPict);
    else 
	xfree(newPict);

    return (result);
}

static int
PanoramiXRenderChangePicture (ClientPtr client)
{
    PanoramiXRes    *pict;
    int		    result = Success, j;
    REQUEST(xRenderChangePictureReq);

    REQUEST_AT_LEAST_SIZE(xChangeWindowAttributesReq);
    
    VERIFY_XIN_PICTURE(pict, stuff->picture, client, SecurityWriteAccess,
		       RenderErrBase + BadPicture);
    
    FOR_NSCREENS_BACKWARD(j) {
        stuff->picture = pict->info[j].id;
        result = (*PanoramiXSaveRenderVector[X_RenderChangePicture]) (client);
        if(result != Success) break;
    }

    return (result);
}

static int
PanoramiXRenderSetPictureClipRectangles (ClientPtr client)
{
    REQUEST(xRenderSetPictureClipRectanglesReq);
    int		    result = Success, j;
    PanoramiXRes    *pict;

    REQUEST_AT_LEAST_SIZE(xRenderSetPictureClipRectanglesReq);
    
    VERIFY_XIN_PICTURE(pict, stuff->picture, client, SecurityWriteAccess,
		       RenderErrBase + BadPicture);
    
    FOR_NSCREENS_BACKWARD(j) {
        stuff->picture = pict->info[j].id;
        result = (*PanoramiXSaveRenderVector[X_RenderSetPictureClipRectangles]) (client);
        if(result != Success) break;
    }

    return (result);
}

static int
PanoramiXRenderSetPictureTransform (ClientPtr client)
{
    REQUEST(xRenderSetPictureTransformReq);
    int		    result = Success, j;
    PanoramiXRes    *pict;

    REQUEST_AT_LEAST_SIZE(xRenderSetPictureTransformReq);
    
    VERIFY_XIN_PICTURE(pict, stuff->picture, client, SecurityWriteAccess,
		       RenderErrBase + BadPicture);
    
    FOR_NSCREENS_BACKWARD(j) {
        stuff->picture = pict->info[j].id;
        result = (*PanoramiXSaveRenderVector[X_RenderSetPictureTransform]) (client);
        if(result != Success) break;
    }

    return (result);
}

static int
PanoramiXRenderSetPictureFilter (ClientPtr client)
{
    REQUEST(xRenderSetPictureFilterReq);
    int		    result = Success, j;
    PanoramiXRes    *pict;

    REQUEST_AT_LEAST_SIZE(xRenderSetPictureFilterReq);
    
    VERIFY_XIN_PICTURE(pict, stuff->picture, client, SecurityWriteAccess,
		       RenderErrBase + BadPicture);
    
    FOR_NSCREENS_BACKWARD(j) {
        stuff->picture = pict->info[j].id;
        result = (*PanoramiXSaveRenderVector[X_RenderSetPictureFilter]) (client);
        if(result != Success) break;
    }

    return (result);
}

static int
PanoramiXRenderFreePicture (ClientPtr client)
{
    PanoramiXRes *pict;
    int         result = Success, j;
    REQUEST(xRenderFreePictureReq);

    REQUEST_SIZE_MATCH(xRenderFreePictureReq);

    client->errorValue = stuff->picture;

    VERIFY_XIN_PICTURE(pict, stuff->picture, client, SecurityDestroyAccess,
		       RenderErrBase + BadPicture);
    

    FOR_NSCREENS_BACKWARD(j) {
	stuff->picture = pict->info[j].id;
	result = (*PanoramiXSaveRenderVector[X_RenderFreePicture]) (client);
	if(result != Success) break;
    }

    /* Since ProcRenderFreePicture is using FreeResource, it will free
	our resource for us on the last pass through the loop above */
 
    return (result);
}

static int
PanoramiXRenderComposite (ClientPtr client)
{
    PanoramiXRes	*src, *msk, *dst;
    int			result = Success, j;
    xRenderCompositeReq	orig;
    REQUEST(xRenderCompositeReq);

    REQUEST_SIZE_MATCH(xRenderCompositeReq);
    
    VERIFY_XIN_PICTURE (src, stuff->src, client, SecurityReadAccess, 
			RenderErrBase + BadPicture);
    VERIFY_XIN_ALPHA (msk, stuff->mask, client, SecurityReadAccess, 
		      RenderErrBase + BadPicture);
    VERIFY_XIN_PICTURE (dst, stuff->dst, client, SecurityWriteAccess, 
			RenderErrBase + BadPicture);
    
    orig = *stuff;
    
    FOR_NSCREENS_FORWARD(j) {
	stuff->src = src->info[j].id;
	if (src->u.pict.root)
	{
	    stuff->xSrc = orig.xSrc - panoramiXdataPtr[j].x;
	    stuff->ySrc = orig.ySrc - panoramiXdataPtr[j].y;
	}
	stuff->dst = dst->info[j].id;
	if (dst->u.pict.root)
	{
	    stuff->xDst = orig.xDst - panoramiXdataPtr[j].x;
	    stuff->yDst = orig.yDst - panoramiXdataPtr[j].y;
	}
	if (msk)
	{
	    stuff->mask = msk->info[j].id;
	    if (msk->u.pict.root)
	    {
		stuff->xMask = orig.xMask - panoramiXdataPtr[j].x;
		stuff->yMask = orig.yMask - panoramiXdataPtr[j].y;
	    }
	}
	result = (*PanoramiXSaveRenderVector[X_RenderComposite]) (client);
	if(result != Success) break;
    }

    return result;
}

static int
PanoramiXRenderCompositeGlyphs (ClientPtr client)
{
    PanoramiXRes    *src, *dst;
    int		    result = Success, j;
    REQUEST(xRenderCompositeGlyphsReq);
    xGlyphElt	    origElt, *elt;
    INT16	    xSrc, ySrc;

    REQUEST_AT_LEAST_SIZE(xRenderCompositeGlyphsReq);
    VERIFY_XIN_PICTURE (src, stuff->src, client, SecurityReadAccess,
			RenderErrBase + BadPicture);
    VERIFY_XIN_PICTURE (dst, stuff->dst, client, SecurityWriteAccess,
			RenderErrBase + BadPicture);

    if (client->req_len << 2 >= (sizeof (xRenderCompositeGlyphsReq) +
				 sizeof (xGlyphElt)))
    {
	elt = (xGlyphElt *) (stuff + 1);
	origElt = *elt;
	xSrc = stuff->xSrc;
	ySrc = stuff->ySrc;
	FOR_NSCREENS_FORWARD(j) {
	    stuff->src = src->info[j].id;
	    if (src->u.pict.root)
	    {
		stuff->xSrc = xSrc - panoramiXdataPtr[j].x;
		stuff->ySrc = ySrc - panoramiXdataPtr[j].y;
	    }
	    stuff->dst = dst->info[j].id;
	    if (dst->u.pict.root)
	    {
		elt->deltax = origElt.deltax - panoramiXdataPtr[j].x;
		elt->deltay = origElt.deltay - panoramiXdataPtr[j].y;
	    }
	    result = (*PanoramiXSaveRenderVector[stuff->renderReqType]) (client);
	    if(result != Success) break;
	}
    }

    return result;
}

static int
PanoramiXRenderFillRectangles (ClientPtr client)
{
    PanoramiXRes    *dst;
    int		    result = Success, j;
    REQUEST(xRenderFillRectanglesReq);
    char	    *extra;
    int		    extra_len;

    REQUEST_AT_LEAST_SIZE (xRenderFillRectanglesReq);
    VERIFY_XIN_PICTURE (dst, stuff->dst, client, SecurityWriteAccess, 
			RenderErrBase + BadPicture);
    extra_len = (client->req_len << 2) - sizeof (xRenderFillRectanglesReq);
    if (extra_len &&
	(extra = (char *) ALLOCATE_LOCAL (extra_len)))
    {
	memcpy (extra, stuff + 1, extra_len);
	FOR_NSCREENS_FORWARD(j) {
	    if (j) memcpy (stuff + 1, extra, extra_len);
	    if (dst->u.pict.root)
	    {
		int x_off = panoramiXdataPtr[j].x;
		int y_off = panoramiXdataPtr[j].y;

		if(x_off || y_off) {
		    xRectangle	*rects = (xRectangle *) (stuff + 1);
		    int		i = extra_len / sizeof (xRectangle);

		    while (i--)
		    {
			rects->x -= x_off;
			rects->y -= y_off;
			rects++;
		    }
		}
	    }
	    stuff->dst = dst->info[j].id;
	    result = (*PanoramiXSaveRenderVector[X_RenderFillRectangles]) (client);
	    if(result != Success) break;
	}
	DEALLOCATE_LOCAL(extra);
    }

    return result;
}

static int
PanoramiXRenderTrapezoids(ClientPtr client)
{
    PanoramiXRes        *src, *dst;
    int                 result = Success, j;
    REQUEST(xRenderTrapezoidsReq);
    char		*extra;
    int			extra_len;
    
    REQUEST_AT_LEAST_SIZE (xRenderTrapezoidsReq);
    
    VERIFY_XIN_PICTURE (src, stuff->src, client, SecurityReadAccess,
			RenderErrBase + BadPicture);
    VERIFY_XIN_PICTURE (dst, stuff->dst, client, SecurityWriteAccess,
			RenderErrBase + BadPicture);

    extra_len = (client->req_len << 2) - sizeof (xRenderTrapezoidsReq);

    if (extra_len &&
	(extra = (char *) ALLOCATE_LOCAL (extra_len))) {
	memcpy (extra, stuff + 1, extra_len);

	FOR_NSCREENS_FORWARD(j) {
	    if (j) memcpy (stuff + 1, extra, extra_len);
	    if (dst->u.pict.root) {
                int x_off = panoramiXdataPtr[j].x;
		int y_off = panoramiXdataPtr[j].y;

		if(x_off || y_off) {
                    xTrapezoid  *trap = (xTrapezoid *) (stuff + 1);
		    int         i = extra_len / sizeof (xTrapezoid);

		    while (i--) {
			trap->top -= y_off;
			trap->bottom -= y_off;
			trap->left.p1.x -= x_off;
			trap->left.p1.y -= y_off;
			trap->left.p2.x -= x_off;
			trap->left.p2.y -= y_off;
			trap->right.p1.x -= x_off;
			trap->right.p1.y -= y_off;
			trap->right.p2.x -= x_off;
			trap->right.p2.y -= y_off;
			trap++;
		    }
		}
	    }
	    
            stuff->src = src->info[j].id;
            stuff->dst = dst->info[j].id;
	    result =
		(*PanoramiXSaveRenderVector[X_RenderTrapezoids]) (client);

	    if(result != Success) break;
	}
	
        DEALLOCATE_LOCAL(extra);
    }

    return result;
}

static int
PanoramiXRenderTriangles(ClientPtr client)
{
    PanoramiXRes        *src, *dst;
    int                 result = Success, j;
    REQUEST(xRenderTrianglesReq);
    char		*extra;
    int			extra_len;
    
    REQUEST_AT_LEAST_SIZE (xRenderTrianglesReq);
    
    VERIFY_XIN_PICTURE (src, stuff->src, client, SecurityReadAccess,
			RenderErrBase + BadPicture);
    VERIFY_XIN_PICTURE (dst, stuff->dst, client, SecurityWriteAccess,
			RenderErrBase + BadPicture);

    extra_len = (client->req_len << 2) - sizeof (xRenderTrianglesReq);

    if (extra_len &&
	(extra = (char *) ALLOCATE_LOCAL (extra_len))) {
	memcpy (extra, stuff + 1, extra_len);

	FOR_NSCREENS_FORWARD(j) {
	    if (j) memcpy (stuff + 1, extra, extra_len);
	    if (dst->u.pict.root) {
                int x_off = panoramiXdataPtr[j].x;
		int y_off = panoramiXdataPtr[j].y;

		if(x_off || y_off) {
                    xTriangle  *tri = (xTriangle *) (stuff + 1);
		    int         i = extra_len / sizeof (xTriangle);

		    while (i--) {
			tri->p1.x -= x_off;
			tri->p1.y -= y_off;
			tri->p2.x -= x_off;
			tri->p2.y -= y_off;
			tri->p3.x -= x_off;
			tri->p3.y -= y_off;
			tri++;
		    }
		}
	    }
	    
            stuff->src = src->info[j].id;
            stuff->dst = dst->info[j].id;
	    result =
		(*PanoramiXSaveRenderVector[X_RenderTriangles]) (client);

	    if(result != Success) break;
	}
	
        DEALLOCATE_LOCAL(extra);
    }

    return result;
}

static int
PanoramiXRenderTriStrip(ClientPtr client)
{
    PanoramiXRes        *src, *dst;
    int                 result = Success, j;
    REQUEST(xRenderTriStripReq);
    char		*extra;
    int			extra_len;
    
    REQUEST_AT_LEAST_SIZE (xRenderTriStripReq);
    
    VERIFY_XIN_PICTURE (src, stuff->src, client, SecurityReadAccess,
			RenderErrBase + BadPicture);
    VERIFY_XIN_PICTURE (dst, stuff->dst, client, SecurityWriteAccess,
			RenderErrBase + BadPicture);

    extra_len = (client->req_len << 2) - sizeof (xRenderTriStripReq);

    if (extra_len &&
	(extra = (char *) ALLOCATE_LOCAL (extra_len))) {
	memcpy (extra, stuff + 1, extra_len);

	FOR_NSCREENS_FORWARD(j) {
	    if (j) memcpy (stuff + 1, extra, extra_len);
	    if (dst->u.pict.root) {
                int x_off = panoramiXdataPtr[j].x;
		int y_off = panoramiXdataPtr[j].y;

		if(x_off || y_off) {
                    xPointFixed  *fixed = (xPointFixed *) (stuff + 1);
		    int         i = extra_len / sizeof (xPointFixed);

		    while (i--) {
			fixed->x -= x_off;
			fixed->y -= y_off;
			fixed++;
		    }
		}
	    }
	    
            stuff->src = src->info[j].id;
            stuff->dst = dst->info[j].id;
	    result =
		(*PanoramiXSaveRenderVector[X_RenderTriStrip]) (client);

	    if(result != Success) break;
	}
	
        DEALLOCATE_LOCAL(extra);
    }

    return result;
}

static int
PanoramiXRenderTriFan(ClientPtr client)
{
    PanoramiXRes        *src, *dst;
    int                 result = Success, j;
    REQUEST(xRenderTriFanReq);
    char		*extra;
    int			extra_len;
    
    REQUEST_AT_LEAST_SIZE (xRenderTriFanReq);
    
    VERIFY_XIN_PICTURE (src, stuff->src, client, SecurityReadAccess,
			RenderErrBase + BadPicture);
    VERIFY_XIN_PICTURE (dst, stuff->dst, client, SecurityWriteAccess,
			RenderErrBase + BadPicture);

    extra_len = (client->req_len << 2) - sizeof (xRenderTriFanReq);

    if (extra_len &&
	(extra = (char *) ALLOCATE_LOCAL (extra_len))) {
	memcpy (extra, stuff + 1, extra_len);

	FOR_NSCREENS_FORWARD(j) {
	    if (j) memcpy (stuff + 1, extra, extra_len);
	    if (dst->u.pict.root) {
                int x_off = panoramiXdataPtr[j].x;
		int y_off = panoramiXdataPtr[j].y;

		if(x_off || y_off) {
                    xPointFixed  *fixed = (xPointFixed *) (stuff + 1);
		    int         i = extra_len / sizeof (xPointFixed);

		    while (i--) {
			fixed->x -= x_off;
			fixed->y -= y_off;
			fixed++;
		    }
		}
	    }
	    
            stuff->src = src->info[j].id;
            stuff->dst = dst->info[j].id;
	    result =
		(*PanoramiXSaveRenderVector[X_RenderTriFan]) (client);

	    if(result != Success) break;
	}
	
        DEALLOCATE_LOCAL(extra);
    }

    return result;
}

#if 0 /* Not implemented yet */

static int
PanoramiXRenderColorTrapezoids(ClientPtr client)
{
    PanoramiXRes        *src, *dst;
    int                 result = Success, j;
    REQUEST(xRenderColorTrapezoidsReq);
    char		*extra;
    int			extra_len;
    
    REQUEST_AT_LEAST_SIZE (xRenderColorTrapezoidsReq);
    
    VERIFY_XIN_PICTURE (dst, stuff->dst, client, SecurityWriteAccess,
			RenderErrBase + BadPicture);

    extra_len = (client->req_len << 2) - sizeof (xRenderColorTrapezoidsReq);

    if (extra_len &&
	(extra = (char *) ALLOCATE_LOCAL (extra_len))) {
	memcpy (extra, stuff + 1, extra_len);

	FOR_NSCREENS_FORWARD(j) {
	    if (j) memcpy (stuff + 1, extra, extra_len);
	    if (dst->u.pict.root) {
                int x_off = panoramiXdataPtr[j].x;
		int y_off = panoramiXdataPtr[j].y;

		if(x_off || y_off) {
			....; 
		}
	    }
	    
            stuff->dst = dst->info[j].id;
	    result =
		(*PanoramiXSaveRenderVector[X_RenderColorTrapezoids]) (client);

	    if(result != Success) break;
	}
	
        DEALLOCATE_LOCAL(extra);
    }

    return result;
}

static int
PanoramiXRenderColorTriangles(ClientPtr client)
{
    PanoramiXRes        *src, *dst;
    int                 result = Success, j;
    REQUEST(xRenderColorTrianglesReq);
    char		*extra;
    int			extra_len;
    
    REQUEST_AT_LEAST_SIZE (xRenderColorTrianglesReq);
    
    VERIFY_XIN_PICTURE (dst, stuff->dst, client, SecurityWriteAccess,
			RenderErrBase + BadPicture);

    extra_len = (client->req_len << 2) - sizeof (xRenderColorTrianglesReq);

    if (extra_len &&
	(extra = (char *) ALLOCATE_LOCAL (extra_len))) {
	memcpy (extra, stuff + 1, extra_len);

	FOR_NSCREENS_FORWARD(j) {
	    if (j) memcpy (stuff + 1, extra, extra_len);
	    if (dst->u.pict.root) {
                int x_off = panoramiXdataPtr[j].x;
		int y_off = panoramiXdataPtr[j].y;

		if(x_off || y_off) {
			....; 
		}
	    }
	    
            stuff->dst = dst->info[j].id;
	    result =
		(*PanoramiXSaveRenderVector[X_RenderColorTriangles]) (client);

	    if(result != Success) break;
	}
	
        DEALLOCATE_LOCAL(extra);
    }

    return result;
}

#endif

void
PanoramiXRenderInit (void)
{
    int	    i;
    
    XRT_PICTURE = CreateNewResourceType (XineramaDeleteResource);
    for (i = 0; i < RenderNumberRequests; i++)
	PanoramiXSaveRenderVector[i] = ProcRenderVector[i];
    /*
     * Stuff in Xinerama aware request processing hooks
     */
    ProcRenderVector[X_RenderCreatePicture] = PanoramiXRenderCreatePicture;
    ProcRenderVector[X_RenderChangePicture] = PanoramiXRenderChangePicture;
    ProcRenderVector[X_RenderSetPictureTransform] = PanoramiXRenderSetPictureTransform;
    ProcRenderVector[X_RenderSetPictureFilter] = PanoramiXRenderSetPictureFilter;
    ProcRenderVector[X_RenderSetPictureClipRectangles] = PanoramiXRenderSetPictureClipRectangles;
    ProcRenderVector[X_RenderFreePicture] = PanoramiXRenderFreePicture;
    ProcRenderVector[X_RenderComposite] = PanoramiXRenderComposite;
    ProcRenderVector[X_RenderCompositeGlyphs8] = PanoramiXRenderCompositeGlyphs;
    ProcRenderVector[X_RenderCompositeGlyphs16] = PanoramiXRenderCompositeGlyphs;
    ProcRenderVector[X_RenderCompositeGlyphs32] = PanoramiXRenderCompositeGlyphs;
    ProcRenderVector[X_RenderFillRectangles] = PanoramiXRenderFillRectangles;

    ProcRenderVector[X_RenderTrapezoids] = PanoramiXRenderTrapezoids;
    ProcRenderVector[X_RenderTriangles] = PanoramiXRenderTriangles;
    ProcRenderVector[X_RenderTriStrip] = PanoramiXRenderTriStrip;
    ProcRenderVector[X_RenderTriFan] = PanoramiXRenderTriFan;
}

void
PanoramiXRenderReset (void)
{
    int	    i;
    for (i = 0; i < RenderNumberRequests; i++)
	ProcRenderVector[i] = PanoramiXSaveRenderVector[i];
}

#endif	/* PANORAMIX */
