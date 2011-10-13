/* $Xorg: cup.c,v 1.4 2001/02/09 02:04:32 xorgcvs Exp $ */
/*

Copyright 1997, 1998  The Open Group

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

*/
/* $XFree86: xc/programs/Xserver/Xext/cup.c,v 1.11tsi Exp $ */

#define NEED_REPLIES
#define NEED_EVENTS
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "colormapst.h"
#include "scrnintstr.h"
#include "servermd.h"
#include "swapreq.h"
#define _XCUP_SERVER_
#include <X11/extensions/Xcupstr.h>
#include <X11/Xfuncproto.h>

#ifndef EXTMODULE
#include "../os/osdep.h"
#else
#include "xf86_ansic.h"
#endif

#include "modinit.h"

static int		ProcDispatch(ClientPtr client);
static int              SProcDispatch(ClientPtr client);
static void		ResetProc(ExtensionEntry* extEntry);

#if 0
static unsigned char	ReqCode = 0;
static int		ErrorBase;
#endif

#if defined(WIN32) || defined(TESTWIN32)
#define HAVE_SPECIAL_DESKTOP_COLORS
#endif

static xColorItem citems[] = {
#ifndef HAVE_SPECIAL_DESKTOP_COLORS
#define CUP_BLACK_PIXEL 0
#define CUP_WHITE_PIXEL 1
  /*  pix     red   green    blue        */ 
    {   0,      0,      0,      0, 0, 0 },
    {   1, 0xffff, 0xffff, 0xffff, 0, 0 }
#else
#ifndef WIN32
    /* 
	This approximates the MS-Windows desktop colormap for testing 
        purposes but has black and white pixels in the typical Unix 
        locations, which should be switched if necessary if your system
        has blackPixel and whitePixel swapped. No entries are provided
        for colormap entries 254 and 255 because AllocColor/FindColor
        will reuse entries zero and one.
    */
    {   0,      0,      0,      0, 0, 0 },
    {   1, 0xffff, 0xffff, 0xffff, 0, 0 },
    {   2, 0x8000,      0,      0, 0, 0 },
    {   3,      0, 0x8000,      0, 0, 0 },
    {   4, 0x8000, 0x8000,      0, 0, 0 },
    {   5,      0,      0, 0x8000, 0, 0 },
    {   6, 0x8000,      0, 0x8000, 0, 0 },
    {   7,      0, 0x8000, 0x8000, 0, 0 },
    {   8, 0xc000, 0xc000, 0xc000, 0, 0 },
    {   9, 0xc000, 0xdc00, 0xc000, 0, 0 },
    { 246, 0xa000, 0xa000, 0xa000, 0, 0 },
    { 247, 0x8000, 0x8000, 0x8000, 0, 0 },
    { 248, 0xffff,      0,      0, 0, 0 },
    { 249,      0, 0xffff,      0, 0, 0 },
    { 250, 0xffff, 0xffff,      0, 0, 0 },
    { 251,      0,      0, 0xffff, 0, 0 },
    { 252, 0xffff,      0, 0xffff, 0, 0 },
    { 253,      0, 0xffff, 0xffff, 0, 0 }
#else
    /* 
	this is the MS-Windows desktop, adjusted for X's 16-bit color
	specifications.
    */
    {   0,      0,      0,      0, 0, 0 },
    {   1, 0x8000,      0,      0, 0, 0 },
    {   2,      0, 0x8000,      0, 0, 0 },
    {   3, 0x8000, 0x8000,      0, 0, 0 },
    {   4,      0,      0, 0x8000, 0, 0 },
    {   5, 0x8000,      0, 0x8000, 0, 0 },
    {   6,      0, 0x8000, 0x8000, 0, 0 },
    {   7, 0xc000, 0xc000, 0xc000, 0, 0 },
    {   8, 0xc000, 0xdc00, 0xc000, 0, 0 },
    {   9, 0xa600, 0xca00, 0xf000, 0, 0 },
    { 246, 0xff00, 0xfb00, 0xf000, 0, 0 },
    { 247, 0xa000, 0xa000, 0xa400, 0, 0 },
    { 248, 0x8000, 0x8000, 0x8000, 0, 0 },
    { 249, 0xff00,      0,      0, 0, 0 },
    { 250,      0, 0xff00,      0, 0, 0 },
    { 251, 0xff00, 0xff00,      0, 0, 0 },
    { 252,      0,      0, 0xff00, 0, 0 },
    { 253, 0xff00,      0, 0xff00, 0, 0 },
    { 254,      0, 0xff00, 0xff00, 0, 0 },
    { 255, 0xff00, 0xff00, 0xff00, 0, 0 }
#endif
#endif
};
#define NUM_DESKTOP_COLORS (sizeof citems / sizeof citems[0])

void
XcupExtensionInit (INITARGS)
{
#if 0
    ExtensionEntry* extEntry;

    if ((extEntry = AddExtension (XCUPNAME,
				0,
				XcupNumberErrors,
				ProcDispatch,
				SProcDispatch,
				ResetProc,
				StandardMinorOpcode))) {
	ReqCode = (unsigned char)extEntry->base;
	ErrorBase = extEntry->errorBase;
    }
#else
    (void) AddExtension (XCUPNAME,
			0,
			XcupNumberErrors,
			ProcDispatch,
			SProcDispatch,
			ResetProc,
			StandardMinorOpcode);
#endif

    /* PC servers initialize the desktop colors (citems) here! */
}

/*ARGSUSED*/
static 
void ResetProc(
    ExtensionEntry* extEntry)
{
}

static 
int ProcQueryVersion(
    register ClientPtr client)
{
    /* REQUEST (xXcupQueryVersionReq); */
    xXcupQueryVersionReply rep;
    register int n;

    REQUEST_SIZE_MATCH (xXcupQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequence_number = client->sequence;
    rep.server_major_version = XCUP_MAJOR_VERSION;
    rep.server_minor_version = XCUP_MINOR_VERSION;
    if (client->swapped) {
    	swaps (&rep.sequence_number, n);
    	swapl (&rep.length, n);
    	swaps (&rep.server_major_version, n);
    	swaps (&rep.server_minor_version, n);
    }
    WriteToClient (client, sizeof (xXcupQueryVersionReply), (char *)&rep);
    return client->noClientException;
}

static
int ProcGetReservedColormapEntries(
    register ClientPtr client)
{
    REQUEST (xXcupGetReservedColormapEntriesReq);
    xXcupGetReservedColormapEntriesReply rep;
    xColorItem* cptr;
    register int n;

    REQUEST_SIZE_MATCH (xXcupGetReservedColormapEntriesReq);

#ifndef HAVE_SPECIAL_DESKTOP_COLORS
    citems[CUP_BLACK_PIXEL].pixel = 
	screenInfo.screens[stuff->screen]->blackPixel;
    citems[CUP_WHITE_PIXEL].pixel = 
	screenInfo.screens[stuff->screen]->whitePixel;
#endif

    rep.type = X_Reply;
    rep.sequence_number = client->sequence;
    rep.length = NUM_DESKTOP_COLORS * 3;
    if (client->swapped) {
    	swaps (&rep.sequence_number, n);
    	swapl (&rep.length, n);
    }
    WriteToClient (client, sizeof (xXcupGetReservedColormapEntriesReply), (char *)&rep);
    for (n = 0, cptr = citems; n < NUM_DESKTOP_COLORS; n++, cptr++) {
	if (client->swapped) SwapColorItem (cptr);
	WriteToClient (client, SIZEOF(xColorItem), (char *)cptr);
    }
    return client->noClientException;
}

static
int ProcStoreColors(
    register ClientPtr client)
{
    REQUEST (xXcupStoreColorsReq);
    ColormapPtr pcmp;

    REQUEST_AT_LEAST_SIZE (xXcupStoreColorsReq);
    pcmp = (ColormapPtr) SecurityLookupIDByType (client, stuff->cmap,
						 RT_COLORMAP, SecurityWriteAccess);

    if (pcmp) {
	int ncolors, n;
	xXcupStoreColorsReply rep;
	xColorItem* cptr;

	if (!(pcmp->class & DynamicClass))
	    return BadMatch;

	ncolors = (client->req_len << 2) - SIZEOF (xXcupStoreColorsReq);
	if (ncolors % SIZEOF(xColorItem))
	    return BadLength;

	ncolors /= SIZEOF (xColorItem);


	for (n = 0, cptr = (xColorItem*) &stuff[1]; n < ncolors; n++) {
	    Pixel pixel = cptr->pixel;

	    if (AllocColor (pcmp,
			    &cptr->red, &cptr->green, &cptr->blue,
			    &pixel, client->index) == Success) {
		cptr->pixel = pixel;
		cptr->flags = 0x08;
	    } else
		cptr->flags = 0;
	    cptr = (xColorItem*) (((char*)cptr) + SIZEOF(xColorItem));
	}

	rep.type = X_Reply;
	rep.sequence_number = client->sequence;
	rep.length = ncolors * 3;
	if (client->swapped) {
    	    swaps (&rep.sequence_number, n);
    	    swapl (&rep.length, n);
	}
	WriteToClient (client, sizeof (xXcupGetReservedColormapEntriesReply), (char *)&rep);
	for (n = 0, cptr = (xColorItem*) &stuff[1]; n < ncolors; n++) {
	    if (client->swapped) SwapColorItem (cptr);
	    WriteToClient (client, SIZEOF(xColorItem), (char *)cptr);
	    cptr = (xColorItem*) (((char*)cptr) + SIZEOF(xColorItem));
	}
	return client->noClientException;
    } else {
	client->errorValue = stuff->cmap;
	return BadColor;
    }
}

static 
int ProcDispatch(
    register ClientPtr client)
{
    REQUEST (xReq);
    switch (stuff->data)
    {
    case X_XcupQueryVersion:
	return ProcQueryVersion (client);
    case X_XcupGetReservedColormapEntries:
	return ProcGetReservedColormapEntries (client);
    case X_XcupStoreColors:
	return ProcStoreColors (client);
    default:
	return BadRequest;
    }
}

static 
int SProcQueryVersion(
    register ClientPtr client)
{
    register int n;

    REQUEST(xXcupQueryVersionReq);
    swaps(&stuff->length, n);
    return ProcQueryVersion(client);
}

static 
int SProcGetReservedColormapEntries(
    ClientPtr client)
{
    register int n;

    REQUEST (xXcupGetReservedColormapEntriesReq);
    swaps (&stuff->length, n);
    swapl (&stuff->screen, n);
    REQUEST_AT_LEAST_SIZE (xXcupGetReservedColormapEntriesReq);
    return ProcGetReservedColormapEntries (client);
}

static 
int SProcXcupStoreColors(
    ClientPtr client)
{
    register int n;
    int count;
    xColorItem* pItem;

    REQUEST (xXcupStoreColorsReq);
    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE (xXcupStoreColorsReq);
    swapl(&stuff->cmap, n);
    pItem = (xColorItem*) &stuff[1];
    for(count = LengthRestB(stuff)/sizeof(xColorItem); --count >= 0; )
        SwapColorItem(pItem++);
    return ProcStoreColors (client);
}

static 
int SProcDispatch(
    register ClientPtr client)
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_XcupQueryVersion:
	return SProcQueryVersion (client);
    case X_XcupGetReservedColormapEntries:
	return SProcGetReservedColormapEntries (client);
    case X_XcupStoreColors:
	return SProcXcupStoreColors (client);
    default:
	return BadRequest;
    }
}


