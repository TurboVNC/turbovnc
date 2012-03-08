/*
 * $XFree86: xc/programs/Xserver/render/filter.c,v 1.1 2002/09/26 02:56:52 keithp Exp $
 *
 * Copyright © 2002 Keith Packard, member of The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
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

static char **filterNames;
static int  nfilterNames;

int
PictureGetFilterId (char *filter, int len, Bool makeit)
{
    int	    i;
    char    *name;
    char    **names;

    if (len < 0)
	len = strlen (filter);
    for (i = 0; i < nfilterNames; i++)
	if (len == strlen (filterNames[i]) && 
	    !strncmp (filterNames[i], filter, len))
	    return i;
    if (!makeit)
	return -1;
    name = xalloc (strlen (filter) + 1);
    if (!name)
	return -1;
    strncpy (name, filter, len);
    name[len] = '\0';
    if (filterNames)
	names = xrealloc (filterNames, (nfilterNames + 1) * sizeof (char *));
    else
	names = xalloc (sizeof (char *));
    if (!names)
    {
	xfree (name);
	return -1;
    }
    filterNames = names;
    i = nfilterNames++;
    filterNames[i] = name;
    return i;
}

static Bool
PictureSetDefaultIds (void)
{
    /* careful here -- this list must match the #define values */
    
    if (PictureGetFilterId (FilterNearest, -1, TRUE) != PictFilterNearest)
	return FALSE;
    if (PictureGetFilterId (FilterBilinear, -1, TRUE) != PictFilterBilinear)
	return FALSE;
    
    if (PictureGetFilterId (FilterFast, -1, TRUE) != PictFilterFast)
	return FALSE;
    if (PictureGetFilterId (FilterGood, -1, TRUE) != PictFilterGood)
	return FALSE;
    if (PictureGetFilterId (FilterBest, -1, TRUE) != PictFilterBest)
	return FALSE;
    return TRUE;
}

char *
PictureGetFilterName (int id)
{
    if (0 <= id && id < nfilterNames)
	return filterNames[id];
    else
	return 0;
}

static void
PictureFreeFilterIds (void)
{
    int	    i;

    for (i = 0; i < nfilterNames; i++)
	xfree (filterNames[i]);
    xfree (filterNames);
    nfilterNames = 0;
    filterNames = 0;
}

int
PictureAddFilter (ScreenPtr pScreen, char *filter, xFixed *params, int nparams)
{
    PictureScreenPtr    ps = GetPictureScreen(pScreen);
    int			id = PictureGetFilterId (filter, -1,  TRUE);
    int			i;
    PictFilterPtr	filters;

    if (id < 0)
	return -1;
    /*
     * It's an error to attempt to reregister a filter
     */
    for (i = 0; i < ps->nfilters; i++)
	if (ps->filters[i].id == id)
	    return -1;
    if (ps->filters)
	filters = xrealloc (ps->filters, (ps->nfilters + 1) * sizeof (PictFilterRec));
    else
	filters = xalloc (sizeof (PictFilterRec));
    if (!filters)
	return -1;
    ps->filters = filters;
    i = ps->nfilters++;
    ps->filters[i].name = PictureGetFilterName (id);
    ps->filters[i].params = params;
    ps->filters[i].nparams = nparams;
    ps->filters[i].id = id;
    return id;
}

Bool
PictureSetFilterAlias (ScreenPtr pScreen, char *filter, char *alias)
{
    PictureScreenPtr    ps = GetPictureScreen(pScreen);
    int			filter_id = PictureGetFilterId (filter, -1, FALSE);
    int			alias_id = PictureGetFilterId (alias, -1, TRUE);
    int			i;

    if (filter_id < 0 || alias_id < 0)
	return FALSE;
    for (i = 0; i < ps->nfilterAliases; i++)
	if (ps->filterAliases[i].alias_id == alias_id)
	    break;
    if (i == ps->nfilterAliases)
    {
	PictFilterAliasPtr  aliases;

	if (ps->filterAliases)
	    aliases = xrealloc (ps->filterAliases,
				(ps->nfilterAliases + 1) * 
				sizeof (PictFilterAliasRec));
	else
	    aliases = xalloc (sizeof (PictFilterAliasRec));
	if (!aliases)
	    return FALSE;
	ps->filterAliases = aliases;
	ps->filterAliases[i].alias = PictureGetFilterName (alias_id);
	ps->filterAliases[i].alias_id = alias_id;
	ps->nfilterAliases++;
    }
    ps->filterAliases[i].filter_id = filter_id;
    return TRUE;
}

PictFilterPtr
PictureFindFilter (ScreenPtr pScreen, char *name, int len)
{
    PictureScreenPtr    ps = GetPictureScreen(pScreen);
    int			id = PictureGetFilterId (name, len, FALSE);
    int			i;

    if (id < 0)
	return 0;
    /* Check for an alias, allow them to recurse */
    for (i = 0; i < ps->nfilterAliases; i++)
	if (ps->filterAliases[i].alias_id == id)
	{
	    id = ps->filterAliases[i].filter_id;
	    i = 0;
	}
    /* find the filter */
    for (i = 0; i < ps->nfilters; i++)
	if (ps->filters[i].id == id)
	    return &ps->filters[i];
    return 0;
}

Bool
PictureSetDefaultFilters (ScreenPtr pScreen)
{
    if (!filterNames)
	if (!PictureSetDefaultIds ())
	    return FALSE;
    if (PictureAddFilter (pScreen, FilterNearest, 0, 0) < 0)
	return FALSE;
    if (PictureAddFilter (pScreen, FilterBilinear, 0, 0) < 0)
	return FALSE;

    if (!PictureSetFilterAlias (pScreen, FilterNearest, FilterFast))
	return FALSE;
    if (!PictureSetFilterAlias (pScreen, FilterBilinear, FilterGood))
	return FALSE;
    if (!PictureSetFilterAlias (pScreen, FilterBilinear, FilterBest))
	return FALSE;
    return TRUE;
}

void
PictureResetFilters (ScreenPtr pScreen)
{
    PictureScreenPtr    ps = GetPictureScreen(pScreen);

    xfree (ps->filters);
    xfree (ps->filterAliases);
    PictureFreeFilterIds ();
}

int
SetPictureFilter (PicturePtr pPicture, char *name, int len, xFixed *params, int nparams)
{
    ScreenPtr		pScreen = pPicture->pDrawable->pScreen;
    PictFilterPtr	pFilter = PictureFindFilter (pScreen, name, len);
    xFixed		*new_params;
    int			i;

    if (!pFilter)
	return BadName;
    if (nparams > pFilter->nparams)
	return BadMatch;
    if (pFilter->nparams != pPicture->filter_nparams)
    {
	new_params = xalloc (pFilter->nparams * sizeof (xFixed));
	if (!new_params)
	    return BadAlloc;
	xfree (pPicture->filter_params);
	pPicture->filter_params = new_params;
	pPicture->filter_nparams = pFilter->nparams;
    }
    for (i = 0; i < nparams; i++)
	pPicture->filter_params[i] = params[i];
    for (; i < pFilter->nparams; i++)
	pPicture->filter_params[i] = pFilter->params[i];
    pPicture->filter = pFilter->id;
    return Success;
}
