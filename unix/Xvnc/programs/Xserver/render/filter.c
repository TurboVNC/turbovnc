/*
 * $Id$
 *
 * Copyright © 2002 Keith Packard
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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

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

/*
 * standard but not required filters don't have constant indices
 */

int
PictureGetFilterId (char *filter, int len, Bool makeit)
{
    int	    i;
    char    *name;
    char    **names;

    if (len < 0)
	len = strlen (filter);
    for (i = 0; i < nfilterNames; i++)
	if (!CompareISOLatin1Lowered ((unsigned char *) filterNames[i], -1, (unsigned char *) filter, len))
	    return i;
    if (!makeit)
	return -1;
    name = xalloc (len + 1);
    if (!name)
	return -1;
    memcpy (name, filter, len);
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

    if (PictureGetFilterId (FilterConvolution, -1, TRUE) != PictFilterConvolution)
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

_X_EXPORT int
PictureAddFilter (ScreenPtr			    pScreen,
		  char				    *filter,
		  PictFilterValidateParamsProcPtr   ValidateParams)
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
    ps->filters[i].id = id;
    ps->filters[i].ValidateParams = ValidateParams;
    return id;
}

_X_EXPORT Bool
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

static Bool
convolutionFilterValidateParams (PicturePtr pPicture,
                                 int	   filter,
                                 xFixed	   *params,
                                 int	   nparams)
{
    if (nparams < 3)
        return FALSE;

    if (xFixedFrac (params[0]) || xFixedFrac (params[1]))
        return FALSE;

    nparams -= 2;
    if ((xFixedToInt (params[0]) * xFixedToInt (params[1])) > nparams)
        return FALSE;

    return TRUE;
}


Bool
PictureSetDefaultFilters (ScreenPtr pScreen)
{
    if (!filterNames)
	if (!PictureSetDefaultIds ())
	    return FALSE;
    if (PictureAddFilter (pScreen, FilterNearest, 0) < 0)
	return FALSE;
    if (PictureAddFilter (pScreen, FilterBilinear, 0) < 0)
	return FALSE;

    if (!PictureSetFilterAlias (pScreen, FilterNearest, FilterFast))
	return FALSE;
    if (!PictureSetFilterAlias (pScreen, FilterBilinear, FilterGood))
	return FALSE;
    if (!PictureSetFilterAlias (pScreen, FilterBilinear, FilterBest))
	return FALSE;

    if (PictureAddFilter (pScreen, FilterConvolution, convolutionFilterValidateParams) < 0)
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
    PictFilterPtr	pFilter;
    xFixed		*new_params;
    int			i, s, result;

    pFilter = PictureFindFilter (screenInfo.screens[0], name, len);

    if (pPicture->pDrawable == NULL) {
	/* For source pictures, the picture isn't tied to a screen.  So, ensure
	 * that all screens can handle a filter we set for the picture.
	 */
	for (s = 0; s < screenInfo.numScreens; s++) {
	    if (PictureFindFilter (screenInfo.screens[s], name, len)->id !=
		pFilter->id)
	    {
		return BadMatch;
	    }
	}
    }

    if (!pFilter)
	return BadName;
    if (pFilter->ValidateParams)
    {
	if (!(*pFilter->ValidateParams) (pPicture, pFilter->id, params, nparams))
	    return BadMatch;
    }
    else if (nparams)
	return BadMatch;

    if (nparams != pPicture->filter_nparams)
    {
	new_params = xalloc (nparams * sizeof (xFixed));
	if (!new_params)
	    return BadAlloc;
	xfree (pPicture->filter_params);
	pPicture->filter_params = new_params;
	pPicture->filter_nparams = nparams;
    }
    for (i = 0; i < nparams; i++)
	pPicture->filter_params[i] = params[i];
    pPicture->filter = pFilter->id;

    if (pPicture->pDrawable) {
	ScreenPtr pScreen = pPicture->pDrawable->pScreen;
	PictureScreenPtr ps = GetPictureScreen(pScreen);

	result = (*ps->ChangePictureFilter) (pPicture, pPicture->filter,
					     params, nparams);
	return result;
    }
    return Success;
}
