/* $XConsortium: cfbcmap.c,v 4.19 94/04/17 20:28:46 dpw Exp $ */
/* $XFree86: xc/programs/Xserver/cfb/cfbcmap.c,v 3.1.8.2 1997/05/11 05:04:17 dawes Exp $ */
/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or X Consortium
not be used in advertising or publicity pertaining to 
distribution  of  the software  without specific prior 
written permission. Sun and X Consortium make no 
representations about the suitability of this software for 
any purpose. It is provided "as is" without any express or 
implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/


#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "colormapst.h"
#include "resource.h"

#ifdef GLXEXT
#ifdef GLX_MODULE
Bool (*GlxInitVisualsPtr)(
#else
extern Bool GlxInitVisuals(
#endif
#if NeedFunctionPrototypes
    VisualPtr *         /*visualp*/,
    DepthPtr *          /*depthp*/,
    int *               /*nvisualp*/,
    int *               /*ndepthp*/,
    int *               /*rootDepthp*/,
    VisualID *          /*defaultVisp*/,
    unsigned long       /*sizes*/,
    int                 /*bitsPerRGB*/
#endif
#ifdef GLX_MODULE
) = NULL;
#else
);
#endif
#endif

#ifdef	STATIC_COLOR

static ColormapPtr InstalledMaps[MAXSCREENS];

int
cfbListInstalledColormaps(pScreen, pmaps)
    ScreenPtr	pScreen;
    Colormap	*pmaps;
{
    /* By the time we are processing requests, we can guarantee that there
     * is always a colormap installed */
    *pmaps = InstalledMaps[pScreen->myNum]->mid;
    return (1);
}


void
cfbInstallColormap(pmap)
    ColormapPtr	pmap;
{
    int index = pmap->pScreen->myNum;
    ColormapPtr oldpmap = InstalledMaps[index];

    if(pmap != oldpmap)
    {
	/* Uninstall pInstalledMap. No hardware changes required, just
	 * notify all interested parties. */
	if(oldpmap != (ColormapPtr)None)
	    WalkTree(pmap->pScreen, TellLostMap, (char *)&oldpmap->mid);
	/* Install pmap */
	InstalledMaps[index] = pmap;
	WalkTree(pmap->pScreen, TellGainedMap, (char *)&pmap->mid);

    }
}

void
cfbUninstallColormap(pmap)
    ColormapPtr	pmap;
{
    int index = pmap->pScreen->myNum;
    ColormapPtr curpmap = InstalledMaps[index];

    if(pmap == curpmap)
    {
	if (pmap->mid != pmap->pScreen->defColormap)
	{
	    curpmap = (ColormapPtr) LookupIDByType(pmap->pScreen->defColormap,
						   RT_COLORMAP);
	    (*pmap->pScreen->InstallColormap)(curpmap);
	}
    }
}

#endif

void
cfbResolveColor(pred, pgreen, pblue, pVisual)
    unsigned short	*pred, *pgreen, *pblue;
    register VisualPtr	pVisual;
{
    int shift = 16 - pVisual->bitsPerRGBValue;
    unsigned lim = (1 << pVisual->bitsPerRGBValue) - 1;

    if ((pVisual->class == PseudoColor) || (pVisual->class == DirectColor))
    {
	/* rescale to rgb bits */
	*pred = ((*pred >> shift) * 65535) / lim;
	*pgreen = ((*pgreen >> shift) * 65535) / lim;
	*pblue = ((*pblue >> shift) * 65535) / lim;
    }
    else if (pVisual->class == GrayScale)
    {
	/* rescale to gray then rgb bits */
	*pred = (30L * *pred + 59L * *pgreen + 11L * *pblue) / 100;
	*pblue = *pgreen = *pred = ((*pred >> shift) * 65535) / lim;
    }
    else if (pVisual->class == StaticGray)
    {
	unsigned limg = pVisual->ColormapEntries - 1;
	/* rescale to gray then [0..limg] then [0..65535] then rgb bits */
	*pred = (30L * *pred + 59L * *pgreen + 11L * *pblue) / 100;
	*pred = ((((*pred * (limg + 1))) >> 16) * 65535) / limg;
	*pblue = *pgreen = *pred = ((*pred >> shift) * 65535) / lim;
    }
    else
    {
	unsigned limr, limg, limb;

	limr = pVisual->redMask >> pVisual->offsetRed;
	limg = pVisual->greenMask >> pVisual->offsetGreen;
	limb = pVisual->blueMask >> pVisual->offsetBlue;
	/* rescale to [0..limN] then [0..65535] then rgb bits */
	*pred = ((((((*pred * (limr + 1)) >> 16) *
		    65535) / limr) >> shift) * 65535) / lim;
	*pgreen = ((((((*pgreen * (limg + 1)) >> 16) *
		      65535) / limg) >> shift) * 65535) / lim;
	*pblue = ((((((*pblue * (limb + 1)) >> 16) *
		     65535) / limb) >> shift) * 65535) / lim;
    }
}

Bool
cfbInitializeColormap(pmap)
    register ColormapPtr	pmap;
{
    register unsigned i;
    register VisualPtr pVisual;
    unsigned lim, maxent, shift;

    pVisual = pmap->pVisual;
    lim = (1 << pVisual->bitsPerRGBValue) - 1;
    shift = 16 - pVisual->bitsPerRGBValue;
    maxent = pVisual->ColormapEntries - 1;
    if (pVisual->class == TrueColor)
    {
	unsigned limr, limg, limb;

	limr = pVisual->redMask >> pVisual->offsetRed;
	limg = pVisual->greenMask >> pVisual->offsetGreen;
	limb = pVisual->blueMask >> pVisual->offsetBlue;
	for(i = 0; i <= maxent; i++)
	{
	    /* rescale to [0..65535] then rgb bits */
	    pmap->red[i].co.local.red =
		((((i * 65535) / limr) >> shift) * 65535) / lim;
	    pmap->green[i].co.local.green =
		((((i * 65535) / limg) >> shift) * 65535) / lim;
	    pmap->blue[i].co.local.blue =
		((((i * 65535) / limb) >> shift) * 65535) / lim;
	}
    }
    else if (pVisual->class == StaticColor)
    {
	unsigned limr, limg, limb;

	limr = pVisual->redMask >> pVisual->offsetRed;
	limg = pVisual->greenMask >> pVisual->offsetGreen;
	limb = pVisual->blueMask >> pVisual->offsetBlue;
	for(i = 0; i <= maxent; i++)
	{
	    /* rescale to [0..65535] then rgb bits */
	    pmap->red[i].co.local.red =
		((((((i & pVisual->redMask) >> pVisual->offsetRed)
		    * 65535) / limr) >> shift) * 65535) / lim;
	    pmap->red[i].co.local.green =
		((((((i & pVisual->greenMask) >> pVisual->offsetGreen)
		    * 65535) / limg) >> shift) * 65535) / lim;
	    pmap->red[i].co.local.blue =
		((((((i & pVisual->blueMask) >> pVisual->offsetBlue)
		    * 65535) / limb) >> shift) * 65535) / lim;
	}
    }
    else if (pVisual->class == StaticGray)
    {
	for(i = 0; i <= maxent; i++)
	{
	    /* rescale to [0..65535] then rgb bits */
	    pmap->red[i].co.local.red = ((((i * 65535) / maxent) >> shift)
					 * 65535) / lim;
	    pmap->red[i].co.local.green = pmap->red[i].co.local.red;
	    pmap->red[i].co.local.blue = pmap->red[i].co.local.red;
	}
    }
    return TRUE;
}

/* When simulating DirectColor on PseudoColor hardware, multiple
   entries of the colormap must be updated
 */

#define AddElement(mask) { \
    pixel = red | green | blue; \
    for (i = 0; i < nresult; i++) \
  	if (outdefs[i].pixel == pixel) \
    	    break; \
    if (i == nresult) \
    { \
   	nresult++; \
	outdefs[i].pixel = pixel; \
	outdefs[i].flags = 0; \
    } \
    outdefs[i].flags |= (mask); \
    outdefs[i].red = pmap->red[red >> pVisual->offsetRed].co.local.red; \
    outdefs[i].green = pmap->green[green >> pVisual->offsetGreen].co.local.green; \
    outdefs[i].blue = pmap->blue[blue >> pVisual->offsetBlue].co.local.blue; \
}

cfbExpandDirectColors (pmap, ndef, indefs, outdefs)
    ColormapPtr	pmap;
    int		ndef;
    xColorItem	*indefs, *outdefs;
{
    int		    minred, mingreen, minblue;
    register int    red, green, blue;
    int		    maxred, maxgreen, maxblue;
    int		    stepred, stepgreen, stepblue;
    VisualPtr	    pVisual;
    register int    pixel;
    register int    nresult;
    register int    i;

    pVisual = pmap->pVisual;

    stepred = 1 << pVisual->offsetRed;
    stepgreen = 1 << pVisual->offsetGreen;
    stepblue = 1 << pVisual->offsetBlue;
    maxred = pVisual->redMask;
    maxgreen = pVisual->greenMask;
    maxblue = pVisual->blueMask;
    nresult = 0;
    for (;ndef--; indefs++)
    {
	if (indefs->flags & DoRed)
	{
	    red = indefs->pixel & pVisual->redMask;
    	    for (green = 0; green <= maxgreen; green += stepgreen)
    	    {
	    	for (blue = 0; blue <= maxblue; blue += stepblue)
	    	{
		    AddElement (DoRed)
	    	}
    	    }
	}
	if (indefs->flags & DoGreen)
	{
	    green = indefs->pixel & pVisual->greenMask;
    	    for (red = 0; red <= maxred; red += stepred)
    	    {
	    	for (blue = 0; blue <= maxblue; blue += stepblue)
	    	{
		    AddElement (DoGreen)
	    	}
    	    }
	}
	if (indefs->flags & DoBlue)
	{
	    blue = indefs->pixel & pVisual->blueMask;
    	    for (red = 0; red <= maxred; red += stepred)
    	    {
	    	for (green = 0; green <= maxgreen; green += stepgreen)
	    	{
		    AddElement (DoBlue)
	    	}
    	    }
	}
    }
    return nresult;
}

Bool
cfbCreateDefColormap(pScreen)
    ScreenPtr pScreen;
{
    unsigned short	zero = 0, ones = 0xFFFF;
    VisualPtr	pVisual;
    ColormapPtr	cmap;
    Pixel wp, bp;
    
    for (pVisual = pScreen->visuals;
	 pVisual->vid != pScreen->rootVisual;
	 pVisual++)
	;

    if (CreateColormap(pScreen->defColormap, pScreen, pVisual, &cmap,
		       (pVisual->class & DynamicClass) ? AllocNone : AllocAll,
		       0)
	!= Success)
	return FALSE;
    wp = pScreen->whitePixel;
    bp = pScreen->blackPixel;
    if ((AllocColor(cmap, &ones, &ones, &ones, &wp, 0) !=
       	   Success) ||
    	(AllocColor(cmap, &zero, &zero, &zero, &bp, 0) !=
       	   Success))
    	return FALSE;
    pScreen->whitePixel = wp;
    pScreen->blackPixel = bp;
    (*pScreen->InstallColormap)(cmap);
    return TRUE;
}

extern int defaultColorVisualClass;

#define _BZ(d) (d / 3)
#define _BS(d) 0
#define _BM(d) ((1 << _BZ(d)) - 1)
#define _GZ(d) ((d - _BZ(d) + 1) / 2)
#define _GS(d) _BZ(d)
#define _GM(d) (((1 << _GZ(d)) - 1) << _GS(d))
#define _RZ(d) (d - _BZ(d) - _GZ(d))
#define _RS(d) (_BZ(d) + _GZ(d))
#define _RM(d) (((1 << _RZ(d)) - 1) << _RS(d))
#define _CE(d) (1 << _GZ(d))

#define MAX_PSEUDO_DEPTH    10	    /* largest DAC size I know */

#define StaticGrayMask	(1 << StaticGray)
#define GrayScaleMask	(1 << GrayScale)
#define StaticColorMask	(1 << StaticColor)
#define PseudoColorMask	(1 << PseudoColor)
#define TrueColorMask	(1 << TrueColor)
#define DirectColorMask (1 << DirectColor)

#define ALL_VISUALS	(StaticGrayMask|\
			 GrayScaleMask|\
			 StaticColorMask|\
			 PseudoColorMask|\
			 TrueColorMask|\
			 DirectColorMask)

#define LARGE_VISUALS	(TrueColorMask|\
			 DirectColorMask)

typedef struct _cfbVisuals {
    struct _cfbVisuals	*next;
    int			depth;
    int			bitsPerRGB;
    int			visuals;
    int			count;
} cfbVisualsRec, *cfbVisualsPtr;

static int  cfbVisualPriority[] = {
    PseudoColor, DirectColor, GrayScale, StaticColor, TrueColor, StaticGray
};

#define NUM_PRIORITY	6

static cfbVisualsPtr	cfbVisuals;

Bool
cfbSetVisualTypes (depth, visuals, bitsPerRGB)
    int	    depth;
    int	    visuals;
{
    cfbVisualsPtr   new, *prev, v;
    int		    count;

    new = (cfbVisualsPtr) xalloc (sizeof *new);
    if (!new)
	return FALSE;
    new->next = 0;
    new->depth = depth;
    new->visuals = visuals;
    new->bitsPerRGB = bitsPerRGB;
    count = (visuals >> 1) & 033333333333;
    count = visuals - count - ((count >> 1) & 033333333333);
    count = (((count + (count >> 3)) & 030707070707) % 077);	/* HAKMEM 169 */
    new->count = count;
    for (prev = &cfbVisuals; v = *prev; prev = &v->next);
    *prev = new;
    return TRUE;
}

/*
 * Given a list of formats for a screen, create a list
 * of visuals and depths for the screen which coorespond to
 * the set which can be used with this version of cfb.
 */

Bool
cfbInitVisuals (visualp, depthp, nvisualp, ndepthp, rootDepthp, defaultVisp, sizes, bitsPerRGB)
    VisualPtr	*visualp;
    DepthPtr	*depthp;
    int		*nvisualp, *ndepthp;
    int		*rootDepthp;
    VisualID	*defaultVisp;
    unsigned long   sizes;
    int		bitsPerRGB;
{
    int		i, j, k;
    VisualPtr	visual;
    DepthPtr	depth;
    VisualID	*vid;
    int		d, b;
    int		f;
    int		ndepth, nvisual;
    int		nvtype;
    int		vtype;
    VisualID	defaultVisual;
    cfbVisualsPtr   visuals, nextVisuals;

    /* none specified, we'll guess from pixmap formats */
    if (!cfbVisuals) 
    {
    	for (f = 0; f < screenInfo.numPixmapFormats; f++) 
    	{
	    d = screenInfo.formats[f].depth;
	    b = screenInfo.formats[f].bitsPerPixel;
	    if (sizes & (1 << (b - 1)))
	    {
	    	if (d > MAX_PSEUDO_DEPTH)
		    vtype = LARGE_VISUALS;
	    	else if (d == 1)
		    vtype = StaticGrayMask;
		else
		    vtype = ALL_VISUALS;
	    }
	    else
		vtype = 0;
	    if (!cfbSetVisualTypes (d, vtype, bitsPerRGB))
		return FALSE;
    	}
    }
    nvisual = 0;
    ndepth = 0;
    for (visuals = cfbVisuals; visuals; visuals = nextVisuals) 
    {
	nextVisuals = visuals->next;
	ndepth++;
	nvisual += visuals->count;
    }
    depth = (DepthPtr) xalloc (ndepth * sizeof (DepthRec));
    visual = (VisualPtr) xalloc (nvisual * sizeof (VisualRec));
    if (!depth || !visual)
    {
	xfree (depth);
	xfree (visual);
	return FALSE;
    }
    *depthp = depth;
    *visualp = visual;
    *ndepthp = ndepth;
    *nvisualp = nvisual;
    for (visuals = cfbVisuals; visuals; visuals = nextVisuals) 
    {
	nextVisuals = visuals->next;
	d = visuals->depth;
	vtype = visuals->visuals;
	nvtype = visuals->count;
	vid = NULL;
	if (nvtype)
	{
	    vid = (VisualID *) xalloc (nvtype * sizeof (VisualID));
	    if (!vid)
		return FALSE;
	}
	depth->depth = d;
	depth->numVids = nvtype;
	depth->vids = vid;
	depth++;
	for (i = 0; i < NUM_PRIORITY; i++) {
	    if (! (vtype & (1 << cfbVisualPriority[i])))
		continue;
	    visual->class = cfbVisualPriority[i];
	    visual->bitsPerRGBValue = visuals->bitsPerRGB;
	    visual->ColormapEntries = 1 << d;
	    visual->nplanes = d;
	    visual->vid = *vid = FakeClientID (0);
	    switch (visual->class) {
	    case PseudoColor:
	    case GrayScale:
	    case StaticGray:
		visual->redMask = 0;
		visual->greenMask =  0;
		visual->blueMask =  0;
		visual->offsetRed  =  0;
		visual->offsetGreen = 0;
		visual->offsetBlue =  0;
		break;
	    case DirectColor:
	    case TrueColor:
		visual->ColormapEntries = _CE(d);
		/* fall through */
	    case StaticColor:
		if (d == 8) {
		    visual->redMask = 0x07;
		    visual->greenMask = 0x38;
		    visual->blueMask =  0xC0;
		    visual->offsetRed  =  0;
		    visual->offsetGreen = 3;
		    visual->offsetBlue =  6;
		} else {
		    visual->redMask =  _RM(d);
		    visual->greenMask =  _GM(d);
		    visual->blueMask =  _BM(d);
		    visual->offsetRed  =  _RS(d);
		    visual->offsetGreen = _GS(d);
		    visual->offsetBlue =  _BS(d);
		}
	    }
	    vid++;
	    visual++;
	}
	xfree (visuals);
    }
    cfbVisuals = NULL;
    visual = *visualp;
    depth = *depthp;
    for (i = 0; i < ndepth; i++)
    {
	if (*rootDepthp && *rootDepthp != depth[i].depth)
	    continue;
	for (j = 0; j < depth[i].numVids; j++)
	{
	    for (k = 0; k < nvisual; k++)
		if (visual[k].vid == depth[i].vids[j])
		    break;
	    if (k == nvisual)
		continue;
	    if (defaultColorVisualClass < 0 ||
		visual[k].class == defaultColorVisualClass)
		break;
	}
	if (j != depth[i].numVids)
	    break;
    }
    if (i == ndepth) {
	i = 0;
	j = 0;
    }
    *rootDepthp = depth[i].depth;
    *defaultVisp = depth[i].vids[j];

#ifdef GLXEXT
#ifdef GLX_MODULE
    if( GlxInitVisualsPtr != NULL ) 
       return (*GlxInitVisualsPtr)
#else
    return GlxInitVisuals
#endif
	   (
               visualp, 
               depthp,
               nvisualp,
               ndepthp,
               rootDepthp,
               defaultVisp,
               sizes,
               bitsPerRGB
           );
#else
    return TRUE;
#endif

}
