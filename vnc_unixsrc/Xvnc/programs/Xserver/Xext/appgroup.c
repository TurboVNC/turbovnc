/* $XFree86: xc/programs/Xserver/Xext/appgroup.c,v 1.10tsi Exp $ */
/*
Copyright 1996, 1998, 2001  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.
*/
/* $Xorg: appgroup.c,v 1.6 2001/02/09 02:04:32 xorgcvs Exp $ */

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
#include "scrnintstr.h"
#include "windowstr.h"
#include "colormapst.h"
#include "servermd.h"
#define _XAG_SERVER_
#include <X11/extensions/Xagstr.h>
#include <X11/extensions/Xagsrv.h>
#define _SECURITY_SERVER
#include <X11/extensions/security.h>
#include <X11/Xfuncproto.h>

#define XSERV_t
#include <X11/Xtrans/Xtrans.h>
#include "../os/osdep.h"

#include <stdio.h>

#include "modinit.h"
#include "appgroup.h"

typedef struct _AppGroupRec {
    struct _AppGroupRec* next;
    XID			appgroupId;
    ClientPtr*		clients;
    int			nclients;
    ClientPtr		leader;
    Bool		single_screen;
    Window		default_root;
    VisualID		root_visual;
    Colormap		default_colormap;    
    Pixel		black_pixel;
    Pixel		white_pixel;
    xConnSetupPrefix	connSetupPrefix;
    char*		ConnectionInfo;
} AppGroupRec, *AppGroupPtr;

static int		ProcXagDispatch(ClientPtr client);
static int              SProcXagDispatch(ClientPtr client);
static void		XagResetProc(ExtensionEntry* extEntry);

#if 0
static unsigned char	XagReqCode = 0;
static int		XagErrorBase;
#endif
static int		XagCallbackRefCount = 0;

static RESTYPE		RT_APPGROUP;
static AppGroupPtr	appGrpList = NULL;

extern xConnSetupPrefix connSetupPrefix;
extern char* ConnectionInfo;
extern int connBlockScreenStart;

static 
int XagAppGroupFree(
    pointer what,
    XID id) /* unused */
{
    int i;
    AppGroupPtr pAppGrp = (AppGroupPtr) what;

    if (pAppGrp->leader)
	for (i = 0; i < pAppGrp->nclients; i++) {
	    pAppGrp->clients[i]->appgroup = NULL;
	    CloseDownClient (pAppGrp->clients[i]);
	}

    if (pAppGrp == appGrpList)
	appGrpList = appGrpList->next;
    else {
	AppGroupPtr tpAppGrp;
	for (tpAppGrp = appGrpList; 
	    tpAppGrp->next != NULL; 
	    tpAppGrp = tpAppGrp->next) {
	    if (tpAppGrp->next == pAppGrp) {
		tpAppGrp->next = tpAppGrp->next->next;
		break;
	    }
	}
    }
    (void) xfree (pAppGrp->clients);
    (void) xfree (pAppGrp->ConnectionInfo);
    (void) xfree (what);
    return Success;
}

/* static */
void XagClientStateChange(
    CallbackListPtr* pcbl,
    pointer nulldata,
    pointer calldata)
{
    SecurityAuthorizationPtr pAuth;
    NewClientInfoRec* pci = (NewClientInfoRec*) calldata;
    ClientPtr pClient = pci->client;
    AppGroupPtr pAppGrp;
    XID authId = 0;

    if (!pClient->appgroup) {
	switch (pClient->clientState) {

	case ClientStateAuthenticating:
	case ClientStateRunning: 
	case ClientStateCheckingSecurity:
	    return;

	case ClientStateInitial: 
	case ClientStateCheckedSecurity:
	    /* 
	     * If the client is connecting via a firewall proxy (which
	     * uses XC-QUERY-SECURITY-1, then the authId is available
	     * during ClientStateCheckedSecurity, otherwise it's
	     * available during ClientStateInitial.
	     *
	     * Don't get it from pClient because can't guarantee the order
	     * of the callbacks and the security extension might not have
	     * plugged it in yet.
	     */
	    authId = AuthorizationIDOfClient(pClient);
	    break;

	case ClientStateGone:
	case ClientStateRetained:
	    /*
	     * Don't get if from AuthorizationIDOfClient because can't
	     * guarantee the order of the callbacks and the security
	     * extension may have torn down the client's private data
	     */
	    authId = pClient->authId;
	    break;
	}

	if (authId == None)
	    return;

	pAuth = (SecurityAuthorizationPtr)SecurityLookupIDByType(pClient,
		authId, SecurityAuthorizationResType, SecurityReadAccess);

	if (pAuth == NULL)
	    return;

	for (pAppGrp = appGrpList; pAppGrp != NULL; pAppGrp = pAppGrp->next)
	    if (pAppGrp->appgroupId == pAuth->group) break;
    } else {
	pAppGrp = pClient->appgroup;
    }

    if (!pAppGrp)
	return;

    switch (pClient->clientState) {
    case ClientStateAuthenticating:
    case ClientStateRunning: 
    case ClientStateCheckingSecurity:
	break;

    case ClientStateInitial: 
    case ClientStateCheckedSecurity:
	/* see the comment above about Initial vs. CheckedSecurity */
	{
	    /* if this client already in AppGroup, don't add it again */
	    int i;
	    for (i = 0; i < pAppGrp->nclients; i++)
		if (pClient == pAppGrp->clients[i]) return;
	}
	pAppGrp->clients = (ClientPtr*) xrealloc (pAppGrp->clients, 
				++pAppGrp->nclients * sizeof (ClientPtr));
	pAppGrp->clients[pAppGrp->nclients - 1] = pClient;
	pClient->appgroup = pAppGrp;
	break;

    case ClientStateGone:
    case ClientStateRetained: /* client disconnected, dump it */
	{
	    int i;
	    for (i = 0; i < pAppGrp->nclients; i++)
		if (pAppGrp->clients[i] == pClient) {
		    pAppGrp->clients[i] = NULL;
		    break;
		}
	    for (i = 0; i < pAppGrp->nclients; i++)
		if (pAppGrp->clients[i] == NULL && i + 1 < pAppGrp->nclients)
		    pAppGrp->clients[i] = pAppGrp->clients[i + 1];
	    pAppGrp->nclients--;
	}
	pClient->appgroup = NULL; /* redundant, pClient will be freed */
	break;
    }
}

void
XagExtensionInit(INITARGS)
{
#if 0
    ExtensionEntry* extEntry;

    if ((extEntry = AddExtension (XAGNAME,
				0,
				XagNumberErrors,
				ProcXagDispatch,
				SProcXagDispatch,
				XagResetProc,
				StandardMinorOpcode))) {
	XagReqCode = (unsigned char)extEntry->base;
	XagErrorBase = extEntry->errorBase;
#else
    if (AddExtension (XAGNAME,
		      0,
		      XagNumberErrors,
		      ProcXagDispatch,
		      SProcXagDispatch,
		      XagResetProc,
		      StandardMinorOpcode)) {
#endif
	RT_APPGROUP = CreateNewResourceType (XagAppGroupFree);
    }
}

/*ARGSUSED*/
static 
void XagResetProc(
    ExtensionEntry* extEntry)
{
    DeleteCallback (&ClientStateCallback, XagClientStateChange, NULL);
    XagCallbackRefCount = 0;
    while (appGrpList) XagAppGroupFree ((pointer) appGrpList, 0);
}

static 
int ProcXagQueryVersion(
    register ClientPtr client)
{
    /* REQUEST (xXagQueryVersionReq); */
    xXagQueryVersionReply rep;
    register int n;

    REQUEST_SIZE_MATCH (xXagQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequence_number = client->sequence;
    rep.server_major_version = XAG_MAJOR_VERSION;
    rep.server_minor_version = XAG_MINOR_VERSION;
    if (client->swapped) {
    	swaps (&rep.sequence_number, n);
    	swapl (&rep.length, n);
    	swaps (&rep.server_major_version, n);
    	swaps (&rep.server_minor_version, n);
    }
    WriteToClient (client, sizeof (xXagQueryVersionReply), (char *)&rep);
    return client->noClientException;
}

static 
void ProcessAttr(
    AppGroupPtr pAppGrp,
    ClientPtr client,
    unsigned int attrib_mask,
    CARD32* attribs)
{
    int i;

    for (i = 0; i <= XagNappGroupLeader; i++) {
	switch (attrib_mask & (1 << i)) {
	case XagSingleScreenMask:
	    pAppGrp->single_screen = *attribs;
	    break;
	case XagDefaultRootMask:
	    pAppGrp->default_root = *attribs;
	    break;
	case XagRootVisualMask:
	    pAppGrp->root_visual = *attribs;
	    break;
	case XagDefaultColormapMask:
	    pAppGrp->default_colormap = *attribs;
	    break;
	case XagBlackPixelMask:
	    pAppGrp->black_pixel = *attribs;
	    break;
	case XagWhitePixelMask:
	    pAppGrp->white_pixel = *attribs;
	    break;
	case XagAppGroupLeaderMask:
	    pAppGrp->leader = client;
	    break;
	default: continue;
	}
	attribs++;
    }
}

static 
void CreateConnectionInfo(
    AppGroupPtr pAppGrp)
{
    xWindowRoot* rootp;
    xWindowRoot* roots[MAXSCREENS];
    unsigned int rootlens[MAXSCREENS];
    xDepth* depth;
    int olen;
    int snum, i;

    rootp = (xWindowRoot*) (ConnectionInfo + connBlockScreenStart);
    for (snum = 0; snum < screenInfo.numScreens; snum++) {

	rootlens[snum] = sizeof (xWindowRoot);
	roots[snum] = rootp;

	depth = (xDepth*) (rootp + 1);
	for (i = 0; i < rootp->nDepths; i++) {
	    rootlens[snum] += sizeof (xDepth) + 
			      depth->nVisuals * sizeof (xVisualType);
	    depth = (xDepth *)(((char*)(depth + 1)) +
		depth->nVisuals * sizeof (xVisualType));
	}
	rootp = (xWindowRoot*) depth;
    }
    snum = 0;
    if (pAppGrp->default_root) {
	for (; snum < screenInfo.numVideoScreens; snum++) {
	    if (roots[snum]->windowId == pAppGrp->default_root)
		break;
        }
    }
    olen = connBlockScreenStart + rootlens[snum];
    for (i = screenInfo.numVideoScreens; i < screenInfo.numScreens; i++)
	olen += rootlens[i];
    pAppGrp->ConnectionInfo = (char*) xalloc (olen);
    if (!pAppGrp->ConnectionInfo)
	return;
    memmove (pAppGrp->ConnectionInfo, ConnectionInfo, connBlockScreenStart);
    ((xConnSetup*) (pAppGrp->ConnectionInfo))->numRoots = 
	1 + screenInfo.numScreens - screenInfo.numVideoScreens;
    memmove (pAppGrp->ConnectionInfo + connBlockScreenStart,
	     (void*) roots[snum], rootlens[snum]);
    rootp = (xWindowRoot*) (pAppGrp->ConnectionInfo + connBlockScreenStart);
    if (pAppGrp->default_colormap) {
	rootp->defaultColormap = pAppGrp->default_colormap;
	rootp->whitePixel = pAppGrp->white_pixel;
	rootp->blackPixel = pAppGrp->black_pixel;
    }
    if (pAppGrp->root_visual)
	rootp->rootVisualID = pAppGrp->root_visual;
    rootp = (xWindowRoot*) (((char*)rootp) + rootlens[snum]);
    for (i = screenInfo.numVideoScreens; i < screenInfo.numScreens; i++) {
	memmove ((void*) rootp, (void*) roots[i], rootlens[i]);
	rootp = (xWindowRoot*) (((char*) rootp) + rootlens[i]);
    }
    pAppGrp->connSetupPrefix = connSetupPrefix;
    pAppGrp->connSetupPrefix.length = olen >> 2;
}

static 
AppGroupPtr CreateAppGroup(
    ClientPtr client,
    XID appgroupId,
    unsigned int attrib_mask,
    CARD32* attribs)
{
    AppGroupPtr pAppGrp;

    pAppGrp = (AppGroupPtr) xalloc (sizeof(AppGroupRec));
    if (pAppGrp) {
	pAppGrp->next = appGrpList;
	appGrpList = pAppGrp;
	pAppGrp->appgroupId = appgroupId;
	pAppGrp->clients = (ClientPtr*) xalloc (0);
	pAppGrp->nclients = 0;
	pAppGrp->leader = NULL;
	pAppGrp->default_root = 0;
	pAppGrp->root_visual = 0;
	pAppGrp->default_colormap = 0;
	pAppGrp->black_pixel = -1;
	pAppGrp->white_pixel = -1;
	pAppGrp->ConnectionInfo = NULL;
	ProcessAttr (pAppGrp, client, attrib_mask, attribs);
    }
    return pAppGrp;
}

static 
int AttrValidate(
    ClientPtr client,
    int attrib_mask,
    AppGroupPtr pAppGrp)
{
    WindowPtr pWin;
    int idepth, ivids, found;
    ScreenPtr pScreen;
    DepthPtr pDepth;
    ColormapPtr pColormap;

    pWin = LookupWindow (pAppGrp->default_root, client);
    /* XXX check that pWin is not NULL */
    pScreen = pWin->drawable.pScreen;
    if (WindowTable[pScreen->myNum]->drawable.id != pAppGrp->default_root)
	return BadWindow;
    pDepth = pScreen->allowedDepths;
    if (pAppGrp->root_visual) {
	found = FALSE;
	for (idepth = 0; idepth < pScreen->numDepths; idepth++, pDepth++) {
	    for (ivids = 0; ivids < pDepth->numVids; ivids++) {
		if (pAppGrp->root_visual == pDepth->vids[ivids]) {
		    found = TRUE;
		    break;
		}
	    }
	}
	if (!found)
	    return BadMatch;
    }
    if (pAppGrp->default_colormap) {

	pColormap = (ColormapPtr)LookupIDByType (pAppGrp->default_colormap, RT_COLORMAP);
	/* XXX check that pColormap is not NULL */
	if (pColormap->pScreen != pScreen)
	    return BadColor;
	if (pColormap->pVisual->vid != (pAppGrp->root_visual ? pAppGrp->root_visual : pScreen->rootVisual))
	    return BadMatch;
    }
    return client->noClientException;
}

/* static */
int ProcXagCreate (
    register ClientPtr client)
{
    REQUEST (xXagCreateReq);
    AppGroupPtr pAppGrp;
    int ret;

    REQUEST_AT_LEAST_SIZE (xXagCreateReq);

    LEGAL_NEW_RESOURCE (stuff->app_group, client);
    pAppGrp = CreateAppGroup (client, stuff->app_group, 
		stuff->attrib_mask, (CARD32*) &stuff[1]);
    if (!pAppGrp)
	return BadAlloc;
    ret = AttrValidate (client, stuff->attrib_mask, pAppGrp);
    if (ret != Success) {
	XagAppGroupFree ((pointer)pAppGrp, (XID)0);
	return ret;
    }
    if (pAppGrp->single_screen) {
	CreateConnectionInfo (pAppGrp);
	if (!pAppGrp->ConnectionInfo)
	    return BadAlloc;
    }
    if (!AddResource (stuff->app_group, RT_APPGROUP, (pointer)pAppGrp))
	return BadAlloc;
    if (XagCallbackRefCount++ == 0)
	(void) AddCallback (&ClientStateCallback, XagClientStateChange, NULL);
    return client->noClientException;
}

/* static */
int ProcXagDestroy(
    register ClientPtr client)
{
    AppGroupPtr pAppGrp;
    REQUEST (xXagDestroyReq);

    REQUEST_SIZE_MATCH (xXagDestroyReq);
    pAppGrp = (AppGroupPtr)SecurityLookupIDByType (client, 
		(XID)stuff->app_group, RT_APPGROUP, SecurityReadAccess);
    if (!pAppGrp) return XagBadAppGroup;
    FreeResource ((XID)stuff->app_group, RT_NONE);
    if (--XagCallbackRefCount == 0)
	(void) DeleteCallback (&ClientStateCallback, XagClientStateChange, NULL);
    return client->noClientException;
}

static 
int ProcXagGetAttr(
    register ClientPtr client)
{
    AppGroupPtr pAppGrp;
    REQUEST (xXagGetAttrReq);
    xXagGetAttrReply rep;
    int n;

    REQUEST_SIZE_MATCH (xXagGetAttrReq);
    pAppGrp = (AppGroupPtr)SecurityLookupIDByType (client, 
		(XID)stuff->app_group, RT_APPGROUP, SecurityReadAccess);
    if (!pAppGrp) return XagBadAppGroup;
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequence_number = client->sequence;
    rep.default_root = pAppGrp->default_root;
    rep.root_visual = pAppGrp->root_visual;
    rep.default_colormap = pAppGrp->default_colormap;
    rep.black_pixel = pAppGrp->black_pixel;
    rep.white_pixel = pAppGrp->white_pixel;
    rep.single_screen = pAppGrp->single_screen;
    rep.app_group_leader = (pAppGrp->leader) ? 1 : 0;
    if (client->swapped) {
    	swaps (&rep.sequence_number, n);
    	swapl (&rep.length, n);
    	swapl (&rep.default_root, n);
    	swapl (&rep.root_visual, n);
    	swapl (&rep.default_colormap, n);
    	swapl (&rep.black_pixel, n);
    	swapl (&rep.white_pixel, n);
    }
    WriteToClient (client, sizeof (xXagGetAttrReply), (char *)&rep);
    return client->noClientException;
}

static 
int ProcXagQuery(
    register ClientPtr client)
{
    ClientPtr pClient;
    AppGroupPtr pAppGrp;
    REQUEST (xXagQueryReq);
    int n;

    REQUEST_SIZE_MATCH (xXagQueryReq);
    pClient = LookupClient (stuff->resource, client);
    for (pAppGrp = appGrpList; pAppGrp != NULL; pAppGrp = pAppGrp->next)
	for (n = 0; n < pAppGrp->nclients; n++)
	    if (pAppGrp->clients[n] == pClient) {
		xXagQueryReply rep;

		rep.type = X_Reply;
		rep.length = 0;
		rep.sequence_number = client->sequence;
		rep.app_group = pAppGrp->appgroupId;
		if (client->swapped) {
		    swaps (&rep.sequence_number, n);
		    swapl (&rep.length, n);
		    swapl (&rep.app_group, n);
		}
		WriteToClient (client, sizeof (xXagQueryReply), (char *)&rep);
		return client->noClientException;
	    }

    return BadMatch;
}

static 
int ProcXagCreateAssoc(
    register ClientPtr client)
{
    REQUEST (xXagCreateAssocReq);

    REQUEST_SIZE_MATCH (xXagCreateAssocReq);
#ifdef WIN32
    if (stuff->window_type != XagWindowTypeWin32)
#else
    if (stuff->window_type != XagWindowTypeX11)
#endif
	return BadMatch;
#if defined(WIN32) || defined(__CYGWIN__) /* and Mac, etc */
    if (!LocalClient (client))
	return BadAccess;
#endif

/* Macintosh, OS/2, and MS-Windows servers have some work to do here */

    return client->noClientException;
}

static 
int ProcXagDestroyAssoc(
    register ClientPtr client)
{
    /* REQUEST (xXagDestroyAssocReq); */

    REQUEST_SIZE_MATCH (xXagDestroyAssocReq);
/* Macintosh, OS/2, and MS-Windows servers have some work to do here */
    return client->noClientException;
}

static 
int ProcXagDispatch (
    register ClientPtr client)
{
    REQUEST (xReq);
    switch (stuff->data)
    {
    case X_XagQueryVersion:
	return ProcXagQueryVersion (client);
    case X_XagCreate:
	return ProcXagCreate (client);
    case X_XagDestroy:
	return ProcXagDestroy (client);
    case X_XagGetAttr:
	return ProcXagGetAttr (client);
    case X_XagQuery:
	return ProcXagQuery (client);
    case X_XagCreateAssoc:
	return ProcXagCreateAssoc (client);
    case X_XagDestroyAssoc:
	return ProcXagDestroyAssoc (client);
    default:
	return BadRequest;
    }
}

static 
int SProcXagQueryVersion(
    register ClientPtr client)
{
    register int n;
    REQUEST(xXagQueryVersionReq);
    swaps(&stuff->length, n);
    return ProcXagQueryVersion(client);
}

static 
int SProcXagCreate(
    ClientPtr client)
{
    register int n;
    REQUEST (xXagCreateReq);
    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE (xXagCreateReq);
    swapl (&stuff->app_group, n);
    swapl (&stuff->attrib_mask, n);
    SwapRestL (stuff);
    return ProcXagCreate (client);
}

static 
int SProcXagDestroy(
    ClientPtr client)
{
    register int n;
    REQUEST (xXagDestroyReq);
    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xXagDestroyReq);
    swapl (&stuff->app_group, n);
    return ProcXagDestroy (client);
}

static 
int SProcXagGetAttr(
    ClientPtr client)
{
    register int n;
    REQUEST (xXagGetAttrReq);
    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xXagGetAttrReq);
    swapl (&stuff->app_group, n);
    return ProcXagGetAttr (client);
}

static 
int SProcXagQuery(
    ClientPtr client)
{
    register int n;
    REQUEST (xXagQueryReq);
    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xXagQueryReq);
    swapl (&stuff->resource, n);
    return ProcXagQuery (client);
}

static 
int SProcXagCreateAssoc(
    ClientPtr client)
{
    register int n;
    REQUEST (xXagCreateAssocReq);
    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xXagCreateAssocReq);
    swapl (&stuff->window, n);
    swapl (&stuff->window_type, n);
    swaps (&stuff->system_window_len, n);
    return ProcXagCreateAssoc (client);
}

static 
int SProcXagDestroyAssoc(
    ClientPtr client)
{
    register int n;
    REQUEST (xXagDestroyAssocReq);
    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xXagDestroyAssocReq);
    swapl (&stuff->window, n);
    return ProcXagDestroyAssoc (client);
}

static 
int SProcXagDispatch(
    register ClientPtr client)
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_XagQueryVersion:
	return SProcXagQueryVersion (client);
    case X_XagCreate:
	return SProcXagCreate (client);
    case X_XagDestroy:
	return SProcXagDestroy (client);
    case X_XagGetAttr:
	return SProcXagGetAttr (client);
    case X_XagQuery:
	return SProcXagQuery (client);
    case X_XagCreateAssoc:
	return SProcXagCreateAssoc (client);
    case X_XagDestroyAssoc:
	return SProcXagDestroyAssoc (client);
    default:
	return BadRequest;
    }
}

Colormap XagDefaultColormap(
    ClientPtr client)
{
    return (client->appgroup ? client->appgroup->default_colormap : None);
}

VisualID XagRootVisual(
    ClientPtr client)
{
    return (client->appgroup ? client->appgroup->root_visual : 0);
}

ClientPtr XagLeader(
    ClientPtr client)
{
    return (client->appgroup ? client->appgroup->leader : NULL);
}

/*
 * Return whether the Map request event should be sent to the appgroup leader.
 * We don't want to send it to the leader when the window is on a different
 * screen, e.g. a print screen.
 */
Bool XagIsControlledRoot(
    ClientPtr client,
    WindowPtr pParent)
{
    if (client->appgroup) {
	if (client->appgroup->single_screen && 
	    pParent->drawable.id == client->appgroup->default_root)
	    return TRUE;
	else if (!pParent->parent)
	    return TRUE;
	else
	    return FALSE;
    }
    return FALSE; 
}

void XagConnectionInfo(
    ClientPtr client,
    xConnSetupPrefix** conn_prefix,
    char** conn_info,
    int* num_screen)
{
    if (client->appgroup && client->appgroup->ConnectionInfo) {
	*conn_prefix = &client->appgroup->connSetupPrefix;
	*conn_info = client->appgroup->ConnectionInfo;
	*num_screen = ((xConnSetup*)(client->appgroup->ConnectionInfo))->numRoots;
    } 
}

XID XagId(
    ClientPtr client)
{
    return (client->appgroup ? client->appgroup->appgroupId : 0);
}

void XagGetDeltaInfo(
    ClientPtr client,
    CARD32* buf)
{
    *buf++ = (CARD32) client->appgroup->default_root;
    *buf++ = (CARD32) client->appgroup->root_visual;
    *buf++ = (CARD32) client->appgroup->default_colormap;
    *buf++ = (CARD32) client->appgroup->black_pixel;
    *buf = (CARD32) client->appgroup->white_pixel;
}

void XagCallClientStateChange(
    ClientPtr client)
{
    if (appGrpList) {
	NewClientInfoRec clientinfo;

	clientinfo.client = client;
	XagClientStateChange (NULL, NULL, (pointer)&clientinfo);
    }
}
