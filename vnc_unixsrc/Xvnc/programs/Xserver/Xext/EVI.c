/* $Xorg: EVI.c,v 1.3 2000/08/17 19:47:55 cpqbld Exp $ */
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
/* $XFree86: xc/programs/Xserver/Xext/EVI.c,v 3.10tsi Exp $ */

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
#include "modinit.h"

#if 0
static unsigned char XEVIReqCode = 0;
#endif
static EviPrivPtr eviPriv;

static int
ProcEVIQueryVersion(ClientPtr client)
{
    /* REQUEST(xEVIQueryVersionReq); */
    xEVIQueryVersionReply rep;
    register int n;
    REQUEST_SIZE_MATCH (xEVIQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = XEVI_MAJOR_VERSION;
    rep.minorVersion = XEVI_MAJOR_VERSION;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swaps(&rep.majorVersion, n);
	swaps(&rep.minorVersion, n);
    }
    WriteToClient(client, sizeof (xEVIQueryVersionReply), (char *)&rep);
    return (client->noClientException);
}
#define swapEviInfo(eviInfo, l)				\
{							\
   int l1 = l; 						\
   xExtendedVisualInfo *eviInfo1 = eviInfo;		\
   while (l1-- > 0) {					\
       swapl(&eviInfo1->core_visual_id, n);		\
       swapl(&eviInfo1->transparency_value, n);		\
       swaps(&eviInfo1->num_colormap_conflicts, n);	\
       eviInfo1++;					\
   }							\
}
#define swapVisual(visual, l)				\
{							\
    int l1 = l;						\
    VisualID32 *visual1 = visual;				\
    while (l1-- > 0) {					\
       swapl(visual1, n);				\
       visual1++;					\
    }							\
}

static int
ProcEVIGetVisualInfo(ClientPtr client)
{
    REQUEST(xEVIGetVisualInfoReq);
    xEVIGetVisualInfoReply rep;
    int n, n_conflict, n_info, sz_info, sz_conflict;
    VisualID32 *conflict;
    xExtendedVisualInfo *eviInfo;
    int status;
    REQUEST_FIXED_SIZE(xEVIGetVisualInfoReq, stuff->n_visual * sz_VisualID32);
    status = eviPriv->getVisualInfo((VisualID32 *)&stuff[1], (int)stuff->n_visual,
		&eviInfo, &n_info, &conflict, &n_conflict);
    if (status != Success)
	return status;
    sz_info = n_info * sz_xExtendedVisualInfo;
    sz_conflict = n_conflict * sz_VisualID32;
    rep.type = X_Reply;
    rep.n_info = n_info;
    rep.n_conflicts = n_conflict;
    rep.sequenceNumber = client->sequence;
    rep.length = (sz_info + sz_conflict) >> 2;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swapl(&rep.n_info, n);
    	swapl(&rep.n_conflicts, n);
	swapEviInfo(eviInfo, n_info);
	swapVisual(conflict, n_conflict);
    }
    WriteToClient(client, sz_xEVIGetVisualInfoReply, (char *)&rep);
    WriteToClient(client, sz_info, (char *)eviInfo);
    WriteToClient(client, sz_conflict, (char *)conflict);
    eviPriv->freeVisualInfo(eviInfo, conflict);
    return (client->noClientException);
}

static int
ProcEVIDispatch(ClientPtr client)
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_EVIQueryVersion:
	return ProcEVIQueryVersion (client);
    case X_EVIGetVisualInfo:
	return ProcEVIGetVisualInfo (client);
    default:
	return BadRequest;
    }
}

static int
SProcEVIQueryVersion(ClientPtr client)
{
   REQUEST(xEVIQueryVersionReq);
   int n;
   swaps(&stuff->length, n);
   return ProcEVIQueryVersion(client);
}

static int
SProcEVIGetVisualInfo(ClientPtr client)
{
    register int n;
    REQUEST(xEVIGetVisualInfoReq);
    swaps(&stuff->length, n);
    return ProcEVIGetVisualInfo(client);
}

static int
SProcEVIDispatch(ClientPtr client)
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_EVIQueryVersion:
	return SProcEVIQueryVersion (client);
    case X_EVIGetVisualInfo:
	return SProcEVIGetVisualInfo (client);
    default:
	return BadRequest;
    }
}

/*ARGSUSED*/
static void
EVIResetProc(ExtensionEntry *extEntry)
{
    eviDDXReset();
}

/****************
 * XEVIExtensionInit
 *
 * Called from InitExtensions in main() or from QueryExtension() if the
 * extension is dynamically loaded.
 *
 ****************/
void
EVIExtensionInit(INITARGS)
{
#if 0
    ExtensionEntry *extEntry;

    if ((extEntry = AddExtension(EVINAME, 0, 0,
				ProcEVIDispatch,
				SProcEVIDispatch,
				EVIResetProc, StandardMinorOpcode))) {
	XEVIReqCode = (unsigned char)extEntry->base;
#else
    if (AddExtension(EVINAME, 0, 0,
		     ProcEVIDispatch, SProcEVIDispatch,
		     EVIResetProc, StandardMinorOpcode)) {
#endif
	eviPriv = eviDDXInit();
    }
}
