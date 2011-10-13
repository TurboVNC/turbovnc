/* $Xorg: sampleEVI.c,v 1.3 2000/08/17 19:47:58 cpqbld Exp $ */
/************************************************************
Copyright (c) 1997 by Silicon Graphics Computer Systems, Inc.
Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of Silicon Graphics not be
used in advertising or publicity pertaining to distribution
of the software without specific prior written permission.
Silicon Graphics makes no representation about the suitability
of this software for any purpose. It is provided "as is"
without any express or implied warranty.
SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.
********************************************************/
/* $XFree86$ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "dixstruct.h"
#include "extnsionst.h"
#include "dix.h"
#define _XEVI_SERVER_
#include <X11/extensions/XEVIstr.h>
#include "EVIstruct.h"
#include "scrnintstr.h"
static int sampleGetVisualInfo(
    VisualID32 *visual,
    int n_visual,
    xExtendedVisualInfo **evi_rn,
    int *n_info_rn,
    VisualID32 **conflict_rn,
    int *n_conflict_rn)
{
    int max_sz_evi = n_visual * sz_xExtendedVisualInfo * screenInfo.numScreens;
    VisualID32 *temp_conflict;
    xExtendedVisualInfo *evi;
    int max_visuals = 0, max_sz_conflict, sz_conflict = 0;
    register int visualI, scrI, sz_evi = 0, conflictI, n_conflict;
    *evi_rn = evi = (xExtendedVisualInfo *)xalloc(max_sz_evi);
    if (!*evi_rn)
         return BadAlloc;
    for (scrI = 0; scrI < screenInfo.numScreens; scrI++) {
        if (screenInfo.screens[scrI]->numVisuals > max_visuals)
            max_visuals = screenInfo.screens[scrI]->numVisuals;
    }
    max_sz_conflict = n_visual * sz_VisualID32 * screenInfo.numScreens * max_visuals;
    temp_conflict = (VisualID32 *)xalloc(max_sz_conflict);
    if (!temp_conflict) {
        xfree(*evi_rn);
        return BadAlloc;
    }
    for (scrI = 0; scrI < screenInfo.numScreens; scrI++) {
        for (visualI = 0; visualI < n_visual; visualI++) {
	    evi[sz_evi].core_visual_id = visual[visualI];
	    evi[sz_evi].screen = scrI;
	    evi[sz_evi].level = 0;
	    evi[sz_evi].transparency_type = XEVI_TRANSPARENCY_NONE;
	    evi[sz_evi].transparency_value = 0;
	    evi[sz_evi].min_hw_colormaps = 1;
	    evi[sz_evi].max_hw_colormaps = 1;
	    evi[sz_evi].num_colormap_conflicts = n_conflict = 0;
	    for (conflictI = 0; conflictI < n_conflict; conflictI++)
		temp_conflict[sz_conflict++] = visual[visualI];
	    sz_evi++;
	}
    }
    *conflict_rn = temp_conflict;
    *n_conflict_rn = sz_conflict;
    *n_info_rn = sz_evi;
    return Success;
}

static void sampleFreeVisualInfo(
    xExtendedVisualInfo *evi,
    VisualID32 *conflict)
{
    if (evi)
        xfree(evi);
    if (conflict)
    	xfree(conflict);
}

EviPrivPtr eviDDXInit(void)
{
    static EviPrivRec eviPriv;
    eviPriv.getVisualInfo = sampleGetVisualInfo;
    eviPriv.freeVisualInfo = sampleFreeVisualInfo;
    return &eviPriv;
}

void eviDDXReset(void)
{
}
