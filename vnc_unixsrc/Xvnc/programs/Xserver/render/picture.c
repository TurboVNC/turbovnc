/*
 * $XFree86: xc/programs/Xserver/render/picture.c,v 1.30 2003/01/26 16:40:43 eich Exp $
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
#include "picturestr.h"
#define FB_OLD_GC
#define FB_OLD_SCREEN
#include "fb.h"

int		PictureScreenPrivateIndex = -1;
int		PictureWindowPrivateIndex;
int		PictureGeneration;
RESTYPE		PictureType;
RESTYPE		PictFormatType;
RESTYPE		GlyphSetType;
int		PictureCmapPolicy = PictureCmapPolicyDefault;

Bool
PictureDestroyWindow (WindowPtr pWindow)
{
    ScreenPtr		pScreen = pWindow->drawable.pScreen;
    PicturePtr		pPicture;
    PictureScreenPtr    ps = GetPictureScreen(pScreen);
    Bool		ret;

    while ((pPicture = GetPictureWindow(pWindow)))
    {
	SetPictureWindow(pWindow, pPicture->pNext);
	if (pPicture->id)
	    FreeResource (pPicture->id, PictureType);
	FreePicture ((pointer) pPicture, pPicture->id);
    }
    pScreen->DestroyWindow = ps->DestroyWindow;
    ret = (*pScreen->DestroyWindow) (pWindow);
    ps->DestroyWindow = pScreen->DestroyWindow;
    pScreen->DestroyWindow = PictureDestroyWindow;
    return ret;
}

Bool
PictureCloseScreen (int index, ScreenPtr pScreen)
{
    PictureScreenPtr    ps = GetPictureScreen(pScreen);
    Bool                ret;
    int			n;

    pScreen->CloseScreen = ps->CloseScreen;
    ret = (*pScreen->CloseScreen) (index, pScreen);
    PictureResetFilters (pScreen);
    for (n = 0; n < ps->nformats; n++)
	if (ps->formats[n].type == PictTypeIndexed)
	    (*ps->CloseIndexed) (pScreen, &ps->formats[n]);
    SetPictureScreen(pScreen, 0);
    xfree (ps->formats);
    xfree (ps);
    return ret;
}

void
PictureStoreColors (ColormapPtr pColormap, int ndef, xColorItem *pdef)
{
    ScreenPtr		pScreen = pColormap->pScreen;
    PictureScreenPtr    ps = GetPictureScreen(pScreen);

    pScreen->StoreColors = ps->StoreColors;
    (*pScreen->StoreColors) (pColormap, ndef, pdef);
    ps->StoreColors = pScreen->StoreColors;
    pScreen->StoreColors = PictureStoreColors;

    if (pColormap->class == PseudoColor || pColormap->class == GrayScale)
    {
	PictFormatPtr	format = ps->formats;
	int		nformats = ps->nformats;

	while (nformats--)
	{
	    if (format->type == PictTypeIndexed &&
		format->index.pColormap == pColormap)
	    {
		(*ps->UpdateIndexed) (pScreen, format, ndef, pdef);
		break;
	    }
	    format++;
	}
    }
}

static int
visualDepth (ScreenPtr pScreen, VisualPtr pVisual)
{
    int		d, v;
    DepthPtr	pDepth;

    for (d = 0; d < pScreen->numDepths; d++)
    {
	pDepth = &pScreen->allowedDepths[d];
	for (v = 0; v < pDepth->numVids; v++)
	    if (pDepth->vids[v] == pVisual->vid)
		return pDepth->depth;
    }
    return 0;
}

typedef struct _formatInit {
    CARD32  format;
    CARD8   depth;
} FormatInitRec, *FormatInitPtr;

static int
addFormat (FormatInitRec    formats[256],
	   int		    nformat,
	   CARD32	    format,
	   CARD8	    depth)
{
    int	n;

    for (n = 0; n < nformat; n++)
	if (formats[n].format == format && formats[n].depth == depth)
	    return nformat;
    formats[nformat].format = format;
    formats[nformat].depth = depth;
    return ++nformat;
}

#define Mask(n)	((n) == 32 ? 0xffffffff : ((1 << (n))-1))

PictFormatPtr
PictureCreateDefaultFormats (ScreenPtr pScreen, int *nformatp)
{
    int		    nformats, f;
    PictFormatPtr   pFormats;
    FormatInitRec   formats[1024];
    CARD32	    format;
    CARD8	    depth;
    VisualPtr	    pVisual;
    int		    v;
    int		    bpp;
    int		    type;
    int		    r, g, b;
    int		    d;
    DepthPtr	    pDepth;

    nformats = 0;
    /* formats required by protocol */
    formats[nformats].format = PICT_a1;
    formats[nformats].depth = 1;
    nformats++;
    formats[nformats].format = PICT_a8;
    formats[nformats].depth = 8;
    nformats++;
    formats[nformats].format = PICT_a4;
    formats[nformats].depth = 4;
    nformats++;
    formats[nformats].format = PICT_a8r8g8b8;
    formats[nformats].depth = 32;
    nformats++;
    formats[nformats].format = PICT_x8r8g8b8;
    formats[nformats].depth = 32;
    nformats++;

    /* now look through the depths and visuals adding other formats */
    for (v = 0; v < pScreen->numVisuals; v++)
    {
	pVisual = &pScreen->visuals[v];
	depth = visualDepth (pScreen, pVisual);
	if (!depth)
	    continue;
    	bpp = BitsPerPixel (depth);
	switch (pVisual->class) {
	case DirectColor:
	case TrueColor:
	    r = Ones (pVisual->redMask);
	    g = Ones (pVisual->greenMask);
	    b = Ones (pVisual->blueMask);
	    type = PICT_TYPE_OTHER;
	    /*
	     * Current rendering code supports only two direct formats,
	     * fields must be packed together at the bottom of the pixel
	     * and must be either RGB or BGR
	     */
	    if (pVisual->offsetBlue == 0 &&
		pVisual->offsetGreen == b &&
		pVisual->offsetRed == b + g)
	    {
		type = PICT_TYPE_ARGB;
	    }
	    else if (pVisual->offsetRed == 0 &&
		     pVisual->offsetGreen == r && 
		     pVisual->offsetBlue == r + g)
	    {
		type = PICT_TYPE_ABGR;
	    }
	    if (type != PICT_TYPE_OTHER)
	    {
		format = PICT_FORMAT(bpp, type, 0, r, g, b);
		nformats = addFormat (formats, nformats, format, depth);
	    }
	    break;
	case StaticColor:
	case PseudoColor:
	    format = PICT_VISFORMAT (bpp, PICT_TYPE_COLOR, v);
	    nformats = addFormat (formats, nformats, format, depth);
	    break;
	case StaticGray:
	case GrayScale:
	    format = PICT_VISFORMAT (bpp, PICT_TYPE_GRAY, v);
	    nformats = addFormat (formats, nformats, format, depth);
	    break;
	}
    }
    /*
     * Walk supported depths and add useful Direct formats
     */
    for (d = 0; d < pScreen->numDepths; d++)
    {
	pDepth = &pScreen->allowedDepths[d];
	bpp = BitsPerPixel (pDepth->depth);
	format = 0;
	switch (bpp) {
	case 16:
	    /* depth 12 formats */
	    if (pDepth->depth >= 12)
	    {
		nformats = addFormat (formats, nformats,
				      PICT_x4r4g4b4, pDepth->depth);
		nformats = addFormat (formats, nformats,
				      PICT_x4b4g4r4, pDepth->depth);
	    }
	    /* depth 15 formats */
	    if (pDepth->depth >= 15)
	    {
		nformats = addFormat (formats, nformats,
				      PICT_x1r5g5b5, pDepth->depth);
		nformats = addFormat (formats, nformats,
				      PICT_x1b5g5r5, pDepth->depth);
	    }
	    /* depth 16 formats */
	    if (pDepth->depth >= 16) 
	    {
		nformats = addFormat (formats, nformats,
				      PICT_a1r5g5b5, pDepth->depth);
		nformats = addFormat (formats, nformats,
				      PICT_a1b5g5r5, pDepth->depth);
		nformats = addFormat (formats, nformats,
				      PICT_r5g6b5, pDepth->depth);
		nformats = addFormat (formats, nformats,
				      PICT_b5g6r5, pDepth->depth);
		nformats = addFormat (formats, nformats,
				      PICT_a4r4g4b4, pDepth->depth);
		nformats = addFormat (formats, nformats,
				      PICT_a4b4g4r4, pDepth->depth);
	    }
	    break;
	case 24:
	    if (pDepth->depth >= 24)
	    {
		nformats = addFormat (formats, nformats,
				      PICT_r8g8b8, pDepth->depth);
		nformats = addFormat (formats, nformats,
				      PICT_b8g8r8, pDepth->depth);
	    }
	    break;
	case 32:
	    if (pDepth->depth >= 24)
	    {
		nformats = addFormat (formats, nformats,
				      PICT_x8r8g8b8, pDepth->depth);
		nformats = addFormat (formats, nformats,
				      PICT_x8b8g8r8, pDepth->depth);
	    }
	    break;
	}
    }
    

    pFormats = (PictFormatPtr) xalloc (nformats * sizeof (PictFormatRec));
    if (!pFormats)
	return 0;
    memset (pFormats, '\0', nformats * sizeof (PictFormatRec));
    for (f = 0; f < nformats; f++)
    {
        pFormats[f].id = FakeClientID (0);
	pFormats[f].depth = formats[f].depth;
	format = formats[f].format;
	pFormats[f].format = format;
	switch (PICT_FORMAT_TYPE(format)) {
	case PICT_TYPE_ARGB:
	    pFormats[f].type = PictTypeDirect;
	    
	    pFormats[f].direct.alphaMask = Mask(PICT_FORMAT_A(format));
	    if (pFormats[f].direct.alphaMask)
		pFormats[f].direct.alpha = (PICT_FORMAT_R(format) +
					    PICT_FORMAT_G(format) +
					    PICT_FORMAT_B(format));
	    
	    pFormats[f].direct.redMask = Mask(PICT_FORMAT_R(format));
	    pFormats[f].direct.red = (PICT_FORMAT_G(format) + 
				      PICT_FORMAT_B(format));
	    
	    pFormats[f].direct.greenMask = Mask(PICT_FORMAT_G(format));
	    pFormats[f].direct.green = PICT_FORMAT_B(format);
	    
	    pFormats[f].direct.blueMask = Mask(PICT_FORMAT_B(format));
	    pFormats[f].direct.blue = 0;
	    break;

	case PICT_TYPE_ABGR:
	    pFormats[f].type = PictTypeDirect;
	    
	    pFormats[f].direct.alphaMask = Mask(PICT_FORMAT_A(format));
	    if (pFormats[f].direct.alphaMask)
		pFormats[f].direct.alpha = (PICT_FORMAT_B(format) +
					    PICT_FORMAT_G(format) +
					    PICT_FORMAT_R(format));
	    
	    pFormats[f].direct.blueMask = Mask(PICT_FORMAT_B(format));
	    pFormats[f].direct.blue = (PICT_FORMAT_G(format) + 
				       PICT_FORMAT_R(format));
	    
	    pFormats[f].direct.greenMask = Mask(PICT_FORMAT_G(format));
	    pFormats[f].direct.green = PICT_FORMAT_R(format);
	    
	    pFormats[f].direct.redMask = Mask(PICT_FORMAT_R(format));
	    pFormats[f].direct.red = 0;
	    break;

	case PICT_TYPE_A:
	    pFormats[f].type = PictTypeDirect;

	    pFormats[f].direct.alpha = 0;
	    pFormats[f].direct.alphaMask = Mask(PICT_FORMAT_A(format));

	    /* remaining fields already set to zero */
	    break;
	    
	case PICT_TYPE_COLOR:
	case PICT_TYPE_GRAY:
	    pFormats[f].type = PictTypeIndexed;
	    pFormats[f].index.pVisual = &pScreen->visuals[PICT_FORMAT_VIS(format)];
	    break;
	}
    }
    *nformatp = nformats;
    return pFormats;
}

Bool
PictureInitIndexedFormats (ScreenPtr pScreen)
{
    PictureScreenPtr    ps = GetPictureScreenIfSet(pScreen);
    PictFormatPtr	format;
    int			nformat;

    if (!ps)
	return FALSE;
    format = ps->formats;
    nformat = ps->nformats;
    while (nformat--)
    {
	if (format->type == PictTypeIndexed && !format->index.pColormap)
	{
	    if (format->index.pVisual->vid == pScreen->rootVisual)
		format->index.pColormap = (ColormapPtr) LookupIDByType(pScreen->defColormap,
								       RT_COLORMAP);
	    else
	    {
		if (CreateColormap (FakeClientID (0), pScreen,
				    format->index.pVisual,
				    &format->index.pColormap, AllocNone,
				    0) != Success)
		{
		    return FALSE;
		}
	    }
	    if (!(*ps->InitIndexed) (pScreen, format))
		return FALSE;
	}
	format++;
    }
    return TRUE;
}

Bool
PictureFinishInit (void)
{
    int	    s;

    for (s = 0; s < screenInfo.numScreens; s++)
    {
	if (!PictureInitIndexedFormats (screenInfo.screens[s]))
	    return FALSE;
	(void) AnimCurInit (screenInfo.screens[s]);
    }

    return TRUE;
}

Bool
PictureSetSubpixelOrder (ScreenPtr pScreen, int subpixel)
{
    PictureScreenPtr    ps = GetPictureScreenIfSet(pScreen);

    if (!ps)
	return FALSE;
    ps->subpixel = subpixel;
    return TRUE;
    
}

int
PictureGetSubpixelOrder (ScreenPtr pScreen)
{
    PictureScreenPtr    ps = GetPictureScreenIfSet(pScreen);

    if (!ps)
	return SubPixelUnknown;
    return ps->subpixel;
}
    
PictFormatPtr
PictureMatchVisual (ScreenPtr pScreen, int depth, VisualPtr pVisual)
{
    PictureScreenPtr    ps = GetPictureScreenIfSet(pScreen);
    PictFormatPtr	format;
    int			nformat;
    int			type;

    if (!ps)
	return 0;
    format = ps->formats;
    nformat = ps->nformats;
    switch (pVisual->class) {
    case StaticGray:
    case GrayScale:
    case StaticColor:
    case PseudoColor:
	type = PictTypeIndexed;
	break;
    case TrueColor:
	type = PictTypeDirect;
	break;
    case DirectColor:
    default:
	return 0;
    }
    while (nformat--)
    {
	if (format->depth == depth && format->type == type)
	{
	    if (type == PictTypeIndexed)
	    {
		if (format->index.pVisual == pVisual)
		    return format;
	    }
	    else
	    {
		if (format->direct.redMask << format->direct.red == 
		    pVisual->redMask &&
		    format->direct.greenMask << format->direct.green == 
		    pVisual->greenMask &&
		    format->direct.blueMask << format->direct.blue == 
		    pVisual->blueMask)
		{
		    return format;
		}
	    }
	}
	format++;
    }
    return 0;
}

PictFormatPtr
PictureMatchFormat (ScreenPtr pScreen, int depth, CARD32 f)
{
    PictureScreenPtr    ps = GetPictureScreenIfSet(pScreen);
    PictFormatPtr	format;
    int			nformat;

    if (!ps)
	return 0;
    format = ps->formats;
    nformat = ps->nformats;
    while (nformat--)
    {
	if (format->depth == depth && format->format == (f & 0xffffff))
	    return format;
	format++;
    }
    return 0;
}

int
PictureParseCmapPolicy (const char *name)
{
    if ( strcmp (name, "default" ) == 0)
	return PictureCmapPolicyDefault;
    else if ( strcmp (name, "mono" ) == 0)
	return PictureCmapPolicyMono;
    else if ( strcmp (name, "gray" ) == 0)
	return PictureCmapPolicyGray;
    else if ( strcmp (name, "color" ) == 0)
	return PictureCmapPolicyColor;
    else if ( strcmp (name, "all" ) == 0)
	return PictureCmapPolicyAll;
    else
	return PictureCmapPolicyInvalid;
}

Bool
PictureInit (ScreenPtr pScreen, PictFormatPtr formats, int nformats)
{
    PictureScreenPtr	ps;
    int			n;
    CARD32		type, a, r, g, b;
    
    if (PictureGeneration != serverGeneration)
    {
	PictureType = CreateNewResourceType (FreePicture);
	if (!PictureType)
	    return FALSE;
	PictFormatType = CreateNewResourceType (FreePictFormat);
	if (!PictFormatType)
	    return FALSE;
	GlyphSetType = CreateNewResourceType (FreeGlyphSet);
	if (!GlyphSetType)
	    return FALSE;
	PictureScreenPrivateIndex = AllocateScreenPrivateIndex();
	if (PictureScreenPrivateIndex < 0)
	    return FALSE;
	PictureWindowPrivateIndex = AllocateWindowPrivateIndex();
	PictureGeneration = serverGeneration;
#ifdef XResExtension
	RegisterResourceName (PictureType, "PICTURE");
	RegisterResourceName (PictFormatType, "PICTFORMAT");
	RegisterResourceName (GlyphSetType, "GLYPHSET");
#endif
    }
    if (!AllocateWindowPrivate (pScreen, PictureWindowPrivateIndex, 0))
	return FALSE;
    
    if (!formats)
    {
	formats = PictureCreateDefaultFormats (pScreen, &nformats);
	if (!formats)
	    return FALSE;
    }
    for (n = 0; n < nformats; n++)
    {
	if (!AddResource (formats[n].id, PictFormatType, (pointer) (formats+n)))
	{
	    xfree (formats);
	    return FALSE;
	}
	if (formats[n].type == PictTypeIndexed)
	{
	    if ((formats[n].index.pVisual->class | DynamicClass) == PseudoColor)
		type = PICT_TYPE_COLOR;
	    else
		type = PICT_TYPE_GRAY;
	    a = r = g = b = 0;
	}
	else
	{
	    if ((formats[n].direct.redMask|
		 formats[n].direct.blueMask|
		 formats[n].direct.greenMask) == 0)
		type = PICT_TYPE_A;
	    else if (formats[n].direct.red > formats[n].direct.blue)
		type = PICT_TYPE_ARGB;
	    else
		type = PICT_TYPE_ABGR;
	    a = Ones (formats[n].direct.alphaMask);
	    r = Ones (formats[n].direct.redMask);
	    g = Ones (formats[n].direct.greenMask);
	    b = Ones (formats[n].direct.blueMask);
	}
	formats[n].format = PICT_FORMAT(0,type,a,r,g,b);
    }
    ps = (PictureScreenPtr) xalloc (sizeof (PictureScreenRec));
    if (!ps)
    {
	xfree (formats);
	return FALSE;
    }
    SetPictureScreen(pScreen, ps);
    if (!GlyphInit (pScreen))
    {
	SetPictureScreen(pScreen, 0);
	xfree (formats);
	xfree (ps);
	return FALSE;
    }

    ps->totalPictureSize = sizeof (PictureRec);
    ps->PicturePrivateSizes = 0;
    ps->PicturePrivateLen = 0;
    
    ps->formats = formats;
    ps->fallback = formats;
    ps->nformats = nformats;
    
    ps->filters = 0;
    ps->nfilters = 0;
    ps->filterAliases = 0;
    ps->nfilterAliases = 0;

    ps->subpixel = SubPixelUnknown;

    ps->CloseScreen = pScreen->CloseScreen;
    ps->DestroyWindow = pScreen->DestroyWindow;
    ps->StoreColors = pScreen->StoreColors;
    pScreen->DestroyWindow = PictureDestroyWindow;
    pScreen->CloseScreen = PictureCloseScreen;
    pScreen->StoreColors = PictureStoreColors;

    if (!PictureSetDefaultFilters (pScreen))
    {
	PictureResetFilters (pScreen);
	SetPictureScreen(pScreen, 0);
	xfree (formats);
	xfree (ps);
	return FALSE;
    }

    return TRUE;
}

void
SetPictureToDefaults (PicturePtr    pPicture)
{
    pPicture->refcnt = 1;
    pPicture->repeat = 0;
    pPicture->graphicsExposures = FALSE;
    pPicture->subWindowMode = ClipByChildren;
    pPicture->polyEdge = PolyEdgeSharp;
    pPicture->polyMode = PolyModePrecise;
    pPicture->freeCompClip = FALSE;
    pPicture->clientClipType = CT_NONE;
    pPicture->componentAlpha = FALSE;

    pPicture->alphaMap = 0;
    pPicture->alphaOrigin.x = 0;
    pPicture->alphaOrigin.y = 0;

    pPicture->clipOrigin.x = 0;
    pPicture->clipOrigin.y = 0;
    pPicture->clientClip = 0;

    pPicture->transform = 0;

    pPicture->dither = None;
    pPicture->filter = PictureGetFilterId (FilterNearest, -1, TRUE);
    pPicture->filter_params = 0;
    pPicture->filter_nparams = 0;

    pPicture->serialNumber = GC_CHANGE_SERIAL_BIT;
    pPicture->stateChanges = (1 << (CPLastBit+1)) - 1;
}

PicturePtr
AllocatePicture (ScreenPtr  pScreen)
{
    PictureScreenPtr	ps = GetPictureScreen(pScreen);
    PicturePtr		pPicture;
    char		*ptr;
    DevUnion		*ppriv;
    unsigned int    	*sizes;
    unsigned int    	size;
    int			i;

    pPicture = (PicturePtr) xalloc (ps->totalPictureSize);
    if (!pPicture)
	return 0;
    ppriv = (DevUnion *)(pPicture + 1);
    pPicture->devPrivates = ppriv;
    sizes = ps->PicturePrivateSizes;
    ptr = (char *)(ppriv + ps->PicturePrivateLen);
    for (i = ps->PicturePrivateLen; --i >= 0; ppriv++, sizes++)
    {
	if ( (size = *sizes) )
	{
	    ppriv->ptr = (pointer)ptr;
	    ptr += size;
	}
	else
	    ppriv->ptr = (pointer)NULL;
    }
    return pPicture;
}

PicturePtr
CreatePicture (Picture		pid,
	       DrawablePtr	pDrawable,
	       PictFormatPtr	pFormat,
	       Mask		vmask,
	       XID		*vlist,
	       ClientPtr	client,
	       int		*error)
{
    PicturePtr		pPicture;
    PictureScreenPtr	ps = GetPictureScreen(pDrawable->pScreen);

    pPicture = AllocatePicture (pDrawable->pScreen);
    if (!pPicture)
    {
	*error = BadAlloc;
	return 0;
    }

    pPicture->id = pid;
    pPicture->pDrawable = pDrawable;
    pPicture->pFormat = pFormat;
    pPicture->format = pFormat->format | (pDrawable->bitsPerPixel << 24);
    if (pDrawable->type == DRAWABLE_PIXMAP)
    {
	++((PixmapPtr)pDrawable)->refcnt;
	pPicture->pNext = 0;
    }
    else
    {
	pPicture->pNext = GetPictureWindow(((WindowPtr) pDrawable));
	SetPictureWindow(((WindowPtr) pDrawable), pPicture);
    }

    SetPictureToDefaults (pPicture);
    
    if (vmask)
	*error = ChangePicture (pPicture, vmask, vlist, 0, client);
    else
	*error = Success;
    if (*error == Success)
	*error = (*ps->CreatePicture) (pPicture);
    if (*error != Success)
    {
	FreePicture (pPicture, (XID) 0);
	pPicture = 0;
    }
    return pPicture;
}

#define NEXT_VAL(_type) (vlist ? (_type) *vlist++ : (_type) ulist++->val)

#define NEXT_PTR(_type) ((_type) ulist++->ptr)

int
ChangePicture (PicturePtr	pPicture,
	       Mask		vmask,
	       XID		*vlist,
	       DevUnion		*ulist,
	       ClientPtr	client)
{
    ScreenPtr		pScreen = pPicture->pDrawable->pScreen;
    PictureScreenPtr	ps = GetPictureScreen(pScreen);
    BITS32		index2;
    int			error = 0;
    BITS32		maskQ;
    
    pPicture->serialNumber |= GC_CHANGE_SERIAL_BIT;
    maskQ = vmask;
    while (vmask && !error)
    {
	index2 = (BITS32) lowbit (vmask);
	vmask &= ~index2;
	pPicture->stateChanges |= index2;
	switch (index2)
	{
	case CPRepeat:
	    {
		unsigned int	newr;
		newr = NEXT_VAL(unsigned int);
		if (newr <= xTrue)
		    pPicture->repeat = newr;
		else
		{
		    client->errorValue = newr;
		    error = BadValue;
		}
	    }
	    break;
	case CPAlphaMap:
	    {
		PicturePtr  pAlpha;
		
		if (vlist)
		{
		    Picture	pid = NEXT_VAL(Picture);

		    if (pid == None)
			pAlpha = 0;
		    else
		    {
			pAlpha = (PicturePtr) SecurityLookupIDByType(client,
								     pid, 
								     PictureType, 
								     SecurityWriteAccess|SecurityReadAccess);
			if (!pAlpha)
			{
			    client->errorValue = pid;
			    error = BadPixmap;
			    break;
			}
			if (pAlpha->pDrawable->type != DRAWABLE_PIXMAP)
			{
			    client->errorValue = pid;
			    error = BadMatch;
			    break;
			}
		    }
		}
		else
		    pAlpha = NEXT_PTR(PicturePtr);
		if (!error)
		{
		    if (pAlpha && pAlpha->pDrawable->type == DRAWABLE_PIXMAP)
			pAlpha->refcnt++;
		    if (pPicture->alphaMap)
			FreePicture ((pointer) pPicture->alphaMap, (XID) 0);
		    pPicture->alphaMap = pAlpha;
		}
	    }
	    break;
	case CPAlphaXOrigin:
	    pPicture->alphaOrigin.x = NEXT_VAL(INT16);
	    break;
	case CPAlphaYOrigin:
	    pPicture->alphaOrigin.y = NEXT_VAL(INT16);
	    break;
	case CPClipXOrigin:
	    pPicture->clipOrigin.x = NEXT_VAL(INT16);
	    break;
	case CPClipYOrigin:
	    pPicture->clipOrigin.y = NEXT_VAL(INT16);
	    break;
	case CPClipMask:
	    {
		Pixmap	    pid;
		PixmapPtr   pPixmap;
		int	    clipType;

		if (vlist)
		{
		    pid = NEXT_VAL(Pixmap);
		    if (pid == None)
		    {
			clipType = CT_NONE;
			pPixmap = NullPixmap;
		    }
		    else
		    {
			clipType = CT_PIXMAP;
			pPixmap = (PixmapPtr)SecurityLookupIDByType(client,
								    pid, 
								    RT_PIXMAP,
								    SecurityReadAccess);
			if (!pPixmap)
			{
			    client->errorValue = pid;
			    error = BadPixmap;
			    break;
			}
		    }
		}
		else
		{
		    pPixmap = NEXT_PTR(PixmapPtr);
		    if (pPixmap)
			clipType = CT_PIXMAP;
		    else
			clipType = CT_NONE;
		}

		if (pPixmap)
		{
		    if ((pPixmap->drawable.depth != 1) ||
			(pPixmap->drawable.pScreen != pScreen))
		    {
			error = BadMatch;
			break;
		    }
		    else
		    {
			clipType = CT_PIXMAP;
			pPixmap->refcnt++;
		    }
		}
		error = (*ps->ChangePictureClip)(pPicture, clipType,
						 (pointer)pPixmap, 0);
		break;
	    }
	case CPGraphicsExposure:
	    {
		unsigned int	newe;
		newe = NEXT_VAL(unsigned int);
		if (newe <= xTrue)
		    pPicture->graphicsExposures = newe;
		else
		{
		    client->errorValue = newe;
		    error = BadValue;
		}
	    }
	    break;
	case CPSubwindowMode:
	    {
		unsigned int	news;
		news = NEXT_VAL(unsigned int);
		if (news == ClipByChildren || news == IncludeInferiors)
		    pPicture->subWindowMode = news;
		else
		{
		    client->errorValue = news;
		    error = BadValue;
		}
	    }
	    break;
	case CPPolyEdge:
	    {
		unsigned int	newe;
		newe = NEXT_VAL(unsigned int);
		if (newe == PolyEdgeSharp || newe == PolyEdgeSmooth)
		    pPicture->polyEdge = newe;
		else
		{
		    client->errorValue = newe;
		    error = BadValue;
		}
	    }
	    break;
	case CPPolyMode:
	    {
		unsigned int	newm;
		newm = NEXT_VAL(unsigned int);
		if (newm == PolyModePrecise || newm == PolyModeImprecise)
		    pPicture->polyMode = newm;
		else
		{
		    client->errorValue = newm;
		    error = BadValue;
		}
	    }
	    break;
	case CPDither:
	    pPicture->dither = NEXT_VAL(Atom);
	    break;
	case CPComponentAlpha:
	    {
		unsigned int	newca;

		newca = NEXT_VAL (unsigned int);
		if (newca <= xTrue)
		    pPicture->componentAlpha = newca;
		else
		{
		    client->errorValue = newca;
		    error = BadValue;
		}
	    }
	    break;
	default:
	    client->errorValue = maskQ;
	    error = BadValue;
	    break;
	}
    }
    (*ps->ChangePicture) (pPicture, maskQ);
    return error;
}

int
SetPictureClipRects (PicturePtr	pPicture,
		     int	xOrigin,
		     int	yOrigin,
		     int	nRect,
		     xRectangle	*rects)
{
    ScreenPtr		pScreen = pPicture->pDrawable->pScreen;
    PictureScreenPtr	ps = GetPictureScreen(pScreen);
    RegionPtr		clientClip;
    int			result;

    clientClip = RECTS_TO_REGION(pScreen,
				 nRect, rects, CT_UNSORTED);
    if (!clientClip)
	return BadAlloc;
    result =(*ps->ChangePictureClip) (pPicture, CT_REGION, 
				      (pointer) clientClip, 0);
    if (result == Success)
    {
	pPicture->clipOrigin.x = xOrigin;
	pPicture->clipOrigin.y = yOrigin;
	pPicture->stateChanges |= CPClipXOrigin|CPClipYOrigin|CPClipMask;
	pPicture->serialNumber |= GC_CHANGE_SERIAL_BIT;
    }
    return result;
}

int
SetPictureTransform (PicturePtr	    pPicture,
		     PictTransform  *transform)
{
    static const PictTransform	identity = { {
	{ xFixed1, 0x00000, 0x00000 },
	{ 0x00000, xFixed1, 0x00000 },
	{ 0x00000, 0x00000, xFixed1 },
    } };

    if (transform && memcmp (transform, &identity, sizeof (PictTransform)) == 0)
	transform = 0;
    
    if (transform)
    {
	if (!pPicture->transform)
	{
	    pPicture->transform = (PictTransform *) xalloc (sizeof (PictTransform));
	    if (!pPicture->transform)
		return BadAlloc;
	}
	*pPicture->transform = *transform;
    }
    else
    {
	if (pPicture->transform)
	{
	    xfree (pPicture->transform);
	    pPicture->transform = 0;
	}
    }
    return Success;
}

static void
ValidateOnePicture (PicturePtr pPicture)
{
    if (pPicture->serialNumber != pPicture->pDrawable->serialNumber)
    {
	PictureScreenPtr    ps = GetPictureScreen(pPicture->pDrawable->pScreen);

	(*ps->ValidatePicture) (pPicture, pPicture->stateChanges);
	pPicture->stateChanges = 0;
	pPicture->serialNumber = pPicture->pDrawable->serialNumber;
    }
}

void
ValidatePicture(PicturePtr pPicture)
{
    ValidateOnePicture (pPicture);
    if (pPicture->alphaMap)
	ValidateOnePicture (pPicture->alphaMap);
}

int
FreePicture (pointer	value,
	     XID	pid)
{
    PicturePtr	pPicture = (PicturePtr) value;

    if (--pPicture->refcnt == 0)
    {
	ScreenPtr	    pScreen = pPicture->pDrawable->pScreen;
	PictureScreenPtr    ps = GetPictureScreen(pScreen);
	
	if (pPicture->alphaMap)
	    FreePicture ((pointer) pPicture->alphaMap, (XID) 0);
	(*ps->DestroyPicture) (pPicture);
	(*ps->DestroyPictureClip) (pPicture);
	if (pPicture->transform)
	    xfree (pPicture->transform);
	if (pPicture->pDrawable->type == DRAWABLE_WINDOW)
	{
	    WindowPtr	pWindow = (WindowPtr) pPicture->pDrawable;
	    PicturePtr	*pPrev;

	    for (pPrev = (PicturePtr *) &((pWindow)->devPrivates[PictureWindowPrivateIndex].ptr);
		 *pPrev;
		 pPrev = &(*pPrev)->pNext)
	    {
		if (*pPrev == pPicture)
		{
		    *pPrev = pPicture->pNext;
		    break;
		}
	    }
	}
	else if (pPicture->pDrawable->type == DRAWABLE_PIXMAP)
	{
	    (*pScreen->DestroyPixmap) ((PixmapPtr)pPicture->pDrawable);
	}
	xfree (pPicture);
    }
    return Success;
}

int
FreePictFormat (pointer	pPictFormat,
		XID     pid)
{
    return Success;
}

void
CompositePicture (CARD8		op,
		  PicturePtr	pSrc,
		  PicturePtr	pMask,
		  PicturePtr	pDst,
		  INT16		xSrc,
		  INT16		ySrc,
		  INT16		xMask,
		  INT16		yMask,
		  INT16		xDst,
		  INT16		yDst,
		  CARD16	width,
		  CARD16	height)
{
    PictureScreenPtr	ps = GetPictureScreen(pDst->pDrawable->pScreen);
    
    ValidatePicture (pSrc);
    if (pMask)
	ValidatePicture (pMask);
    ValidatePicture (pDst);
    (*ps->Composite) (op,
		       pSrc,
		       pMask,
		       pDst,
		       xSrc,
		       ySrc,
		       xMask,
		       yMask,
		       xDst,
		       yDst,
		       width,
		       height);
}

void
CompositeGlyphs (CARD8		op,
		 PicturePtr	pSrc,
		 PicturePtr	pDst,
		 PictFormatPtr	maskFormat,
		 INT16		xSrc,
		 INT16		ySrc,
		 int		nlist,
		 GlyphListPtr	lists,
		 GlyphPtr	*glyphs)
{
    PictureScreenPtr	ps = GetPictureScreen(pDst->pDrawable->pScreen);
    
    ValidatePicture (pSrc);
    ValidatePicture (pDst);
    (*ps->Glyphs) (op, pSrc, pDst, maskFormat, xSrc, ySrc, nlist, lists, glyphs);
}

void
CompositeRects (CARD8		op,
		PicturePtr	pDst,
		xRenderColor	*color,
		int		nRect,
		xRectangle      *rects)
{
    PictureScreenPtr	ps = GetPictureScreen(pDst->pDrawable->pScreen);
    
    ValidatePicture (pDst);
    (*ps->CompositeRects) (op, pDst, color, nRect, rects);
}

void
CompositeTrapezoids (CARD8	    op,
		     PicturePtr	    pSrc,
		     PicturePtr	    pDst,
		     PictFormatPtr  maskFormat,
		     INT16	    xSrc,
		     INT16	    ySrc,
		     int	    ntrap,
		     xTrapezoid	    *traps)
{
    PictureScreenPtr	ps = GetPictureScreen(pDst->pDrawable->pScreen);
    
    ValidatePicture (pSrc);
    ValidatePicture (pDst);
    (*ps->Trapezoids) (op, pSrc, pDst, maskFormat, xSrc, ySrc, ntrap, traps);
}

void
CompositeTriangles (CARD8	    op,
		    PicturePtr	    pSrc,
		    PicturePtr	    pDst,
		    PictFormatPtr   maskFormat,
		    INT16	    xSrc,
		    INT16	    ySrc,
		    int		    ntriangles,
		    xTriangle	    *triangles)
{
    PictureScreenPtr	ps = GetPictureScreen(pDst->pDrawable->pScreen);
    
    ValidatePicture (pSrc);
    ValidatePicture (pDst);
    (*ps->Triangles) (op, pSrc, pDst, maskFormat, xSrc, ySrc, ntriangles, triangles);
}

void
CompositeTriStrip (CARD8	    op,
		   PicturePtr	    pSrc,
		   PicturePtr	    pDst,
		   PictFormatPtr    maskFormat,
		   INT16	    xSrc,
		   INT16	    ySrc,
		   int		    npoints,
		   xPointFixed	    *points)
{
    PictureScreenPtr	ps = GetPictureScreen(pDst->pDrawable->pScreen);
    
    ValidatePicture (pSrc);
    ValidatePicture (pDst);
    (*ps->TriStrip) (op, pSrc, pDst, maskFormat, xSrc, ySrc, npoints, points);
}

void
CompositeTriFan (CARD8		op,
		 PicturePtr	pSrc,
		 PicturePtr	pDst,
		 PictFormatPtr	maskFormat,
		 INT16		xSrc,
		 INT16		ySrc,
		 int		npoints,
		 xPointFixed	*points)
{
    PictureScreenPtr	ps = GetPictureScreen(pDst->pDrawable->pScreen);
    
    ValidatePicture (pSrc);
    ValidatePicture (pDst);
    (*ps->TriFan) (op, pSrc, pDst, maskFormat, xSrc, ySrc, npoints, points);
}

typedef xFixed_32_32	xFixed_48_16;

#define MAX_FIXED_48_16	    ((xFixed_48_16) 0x7fffffff)
#define MIN_FIXED_48_16	    (-((xFixed_48_16) 1 << 31))

Bool
PictureTransformPoint (PictTransformPtr transform,
		       PictVectorPtr	vector)
{
    PictVector	    result;
    int		    i, j;
    xFixed_32_32    partial;
    xFixed_48_16    v;

    for (j = 0; j < 3; j++)
    {
	v = 0;
	for (i = 0; i < 3; i++)
	{
	    partial = ((xFixed_48_16) transform->matrix[j][i] * 
		       (xFixed_48_16) vector->vector[i]);
	    v += partial >> 16;
	}
	if (v > MAX_FIXED_48_16 || v < MIN_FIXED_48_16)
	    return FALSE;
	result.vector[j] = (xFixed) v;
    }
    if (!result.vector[2])
	return FALSE;
    for (j = 0; j < 2; j++)
    {
	partial = (xFixed_48_16) result.vector[j] << 16;
	v = partial / result.vector[2];
	if (v > MAX_FIXED_48_16 || v < MIN_FIXED_48_16)
	    return FALSE;
	vector->vector[j] = (xFixed) v;
    }
    vector->vector[2] = xFixed1;
    return TRUE;
}
