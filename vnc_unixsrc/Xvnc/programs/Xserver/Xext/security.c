/* $XdotOrg: xc/programs/Xserver/Xext/security.c,v 1.5 2005/07/03 07:01:04 daniels Exp $ */
/* $Xorg: security.c,v 1.4 2001/02/09 02:04:32 xorgcvs Exp $ */
/*

Copyright 1996, 1998  The Open Group

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
/* $XFree86: xc/programs/Xserver/Xext/security.c,v 1.16tsi Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "dixstruct.h"
#include "extnsionst.h"
#include "windowstr.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "gcstruct.h"
#include "colormapst.h"
#include "propertyst.h"
#define _SECURITY_SERVER
#include <X11/extensions/securstr.h>
#include <assert.h>
#include <stdarg.h>
#ifdef LBX
#define _XLBX_SERVER_
#include <X11/extensions/XLbx.h>
extern unsigned char LbxReqCode;
#endif
#ifdef XAPPGROUP
#include <X11/extensions/Xagsrv.h>
#endif
#include <stdio.h>  /* for file reading operations */
#include <X11/Xatom.h>  /* for XA_STRING */

#ifndef DEFAULTPOLICYFILE
# define DEFAULTPOLICYFILE NULL
#endif
#if defined(WIN32) || defined(__CYGWIN__)
#include <X11/Xos.h>
#undef index
#endif

#include "modinit.h"

static int SecurityErrorBase;  /* first Security error number */
static int SecurityEventBase;  /* first Security event number */

CallbackListPtr SecurityValidateGroupCallback = NULL;  /* see security.h */

RESTYPE SecurityAuthorizationResType; /* resource type for authorizations */

static RESTYPE RTEventClient;

/* Proc vectors for untrusted clients, swapped and unswapped versions.
 * These are the same as the normal proc vectors except that extensions
 * that haven't declared themselves secure will have ProcBadRequest plugged
 * in for their major opcode dispatcher.  This prevents untrusted clients
 * from guessing extension major opcodes and using the extension even though
 * the extension can't be listed or queried.
 */
int (*UntrustedProcVector[256])(
    ClientPtr /*client*/
);
int (*SwappedUntrustedProcVector[256])(
    ClientPtr /*client*/
);

/* SecurityAudit
 *
 * Arguments:
 *	format is the formatting string to be used to interpret the
 *	  remaining arguments.
 *
 * Returns: nothing.
 *
 * Side Effects:
 *	Writes the message to the log file if security logging is on.
 */

void
SecurityAudit(char *format, ...)
{
    va_list args;

    if (auditTrailLevel < SECURITY_AUDIT_LEVEL)
	return;
    va_start(args, format);
    VAuditF(format, args);
    va_end(args);
} /* SecurityAudit */

#define rClient(obj) (clients[CLIENT_ID((obj)->resource)])

/* SecurityDeleteAuthorization
 *
 * Arguments:
 *	value is the authorization to delete.
 *	id is its resource ID.
 *
 * Returns: Success.
 *
 * Side Effects:
 *	Frees everything associated with the authorization.
 */

static int
SecurityDeleteAuthorization(
    pointer value,
    XID id)
{
    SecurityAuthorizationPtr pAuth = (SecurityAuthorizationPtr)value;
    unsigned short name_len, data_len;
    char *name, *data;
    int status;
    int i;
    OtherClientsPtr pEventClient;

    /* Remove the auth using the os layer auth manager */

    status = AuthorizationFromID(pAuth->id, &name_len, &name,
				 &data_len, &data);
    assert(status);
    status = RemoveAuthorization(name_len, name, data_len, data);
    assert(status);
    (void)status;

    /* free the auth timer if there is one */

    if (pAuth->timer) TimerFree(pAuth->timer);

    /* send revoke events */

    while ((pEventClient = pAuth->eventClients))
    {
	/* send revocation event event */
	ClientPtr client = rClient(pEventClient);

	if (!client->clientGone)
	{
	    xSecurityAuthorizationRevokedEvent are;
	    are.type = SecurityEventBase + XSecurityAuthorizationRevoked;
	    are.sequenceNumber = client->sequence;
	    are.authId = pAuth->id;
	    WriteEventsToClient(client, 1, (xEvent *)&are);
	}
	FreeResource(pEventClient->resource, RT_NONE);
    }

    /* kill all clients using this auth */

    for (i = 1; i<currentMaxClients; i++)
    {
	if (clients[i] && (clients[i]->authId == pAuth->id))
	    CloseDownClient(clients[i]);
    }

    SecurityAudit("revoked authorization ID %d\n", pAuth->id);
    xfree(pAuth);
    return Success;

} /* SecurityDeleteAuthorization */


/* resource delete function for RTEventClient */
static int
SecurityDeleteAuthorizationEventClient(
    pointer value,
    XID id)
{
    OtherClientsPtr pEventClient, prev = NULL;
    SecurityAuthorizationPtr pAuth = (SecurityAuthorizationPtr)value;

    for (pEventClient = pAuth->eventClients;
	 pEventClient;
	 pEventClient = pEventClient->next)
    {
	if (pEventClient->resource == id)
	{
	    if (prev)
		prev->next = pEventClient->next;
	    else
		pAuth->eventClients = pEventClient->next;
	    xfree(pEventClient);
	    return(Success);
	}
	prev = pEventClient;
    }
    /*NOTREACHED*/
    return -1; /* make compiler happy */
} /* SecurityDeleteAuthorizationEventClient */


/* SecurityComputeAuthorizationTimeout
 *
 * Arguments:
 *	pAuth is the authorization for which we are computing the timeout
 *	seconds is the number of seconds we want to wait
 *
 * Returns:
 *	the number of milliseconds that the auth timer should be set to
 *
 * Side Effects:
 *	Sets pAuth->secondsRemaining to any "overflow" amount of time
 *	that didn't fit in 32 bits worth of milliseconds
 */

static CARD32
SecurityComputeAuthorizationTimeout(
    SecurityAuthorizationPtr pAuth,
    unsigned int seconds)
{
    /* maxSecs is the number of full seconds that can be expressed in
     * 32 bits worth of milliseconds
     */
    CARD32 maxSecs = (CARD32)(~0) / (CARD32)MILLI_PER_SECOND;

    if (seconds > maxSecs)
    { /* only come here if we want to wait more than 49 days */
	pAuth->secondsRemaining = seconds - maxSecs;
	return maxSecs * MILLI_PER_SECOND;
    }
    else
    { /* by far the common case */
	pAuth->secondsRemaining = 0;
	return seconds * MILLI_PER_SECOND;
    }
} /* SecurityStartAuthorizationTimer */

/* SecurityAuthorizationExpired
 *
 * This function is passed as an argument to TimerSet and gets called from
 * the timer manager in the os layer when its time is up.
 *
 * Arguments:
 *	timer is the timer for this authorization.
 *	time is the current time.
 *	pval is the authorization whose time is up.
 *
 * Returns:
 *	A new time delay in milliseconds if the timer should wait some
 *	more, else zero.
 *
 * Side Effects:
 *	Frees the authorization resource if the timeout period is really
 *	over, otherwise recomputes pAuth->secondsRemaining.
 */

static CARD32
SecurityAuthorizationExpired(
    OsTimerPtr timer,
    CARD32 time,
    pointer pval)
{
    SecurityAuthorizationPtr pAuth = (SecurityAuthorizationPtr)pval;

    assert(pAuth->timer == timer);

    if (pAuth->secondsRemaining)
    {
	return SecurityComputeAuthorizationTimeout(pAuth,
						   pAuth->secondsRemaining);
    }
    else
    {
	FreeResource(pAuth->id, RT_NONE);
	return 0;
    }
} /* SecurityAuthorizationExpired */

/* SecurityStartAuthorizationTimer
 *
 * Arguments:
 *	pAuth is the authorization whose timer should be started.
 *
 * Returns: nothing.
 *
 * Side Effects:
 *	A timer is started, set to expire after the timeout period for
 *	this authorization.  When it expires, the function
 *	SecurityAuthorizationExpired will be called.
 */

static void
SecurityStartAuthorizationTimer(
    SecurityAuthorizationPtr pAuth)
{
    pAuth->timer = TimerSet(pAuth->timer, 0,
	SecurityComputeAuthorizationTimeout(pAuth, pAuth->timeout),
			    SecurityAuthorizationExpired, pAuth);
} /* SecurityStartAuthorizationTimer */


/* Proc functions all take a client argument, execute the request in
 * client->requestBuffer, and return a protocol error status.
 */

static int
ProcSecurityQueryVersion(
    ClientPtr client)
{
    /* REQUEST(xSecurityQueryVersionReq); */
    xSecurityQueryVersionReply 	rep;

    /* paranoia: this "can't happen" because this extension is hidden
     * from untrusted clients, but just in case...
     */
    if (client->trustLevel != XSecurityClientTrusted)
	return BadRequest;

    REQUEST_SIZE_MATCH(xSecurityQueryVersionReq);
    rep.type        	= X_Reply;
    rep.sequenceNumber 	= client->sequence;
    rep.length         	= 0;
    rep.majorVersion  	= SECURITY_MAJOR_VERSION;
    rep.minorVersion  	= SECURITY_MINOR_VERSION;
    if(client->swapped)
    {
	register char n;
    	swaps(&rep.sequenceNumber, n);
	swaps(&rep.majorVersion, n);
	swaps(&rep.minorVersion, n);
    }
    (void)WriteToClient(client, SIZEOF(xSecurityQueryVersionReply),
			(char *)&rep);
    return (client->noClientException);
} /* ProcSecurityQueryVersion */


static int
SecurityEventSelectForAuthorization(
    SecurityAuthorizationPtr pAuth,
    ClientPtr client,
    Mask mask)
{
    OtherClients *pEventClient;

    for (pEventClient = pAuth->eventClients;
	 pEventClient;
	 pEventClient = pEventClient->next)
    {
	if (SameClient(pEventClient, client))
	{
	    if (mask == 0)
		FreeResource(pEventClient->resource, RT_NONE);
	    else
		pEventClient->mask = mask;
	    return Success;
	}
    }
    
    pEventClient = (OtherClients *) xalloc(sizeof(OtherClients));
    if (!pEventClient)
	return BadAlloc;
    pEventClient->mask = mask;
    pEventClient->resource = FakeClientID(client->index);
    pEventClient->next = pAuth->eventClients;
    if (!AddResource(pEventClient->resource, RTEventClient,
		     (pointer)pAuth))
    {
	xfree(pEventClient);
	return BadAlloc;
    }
    pAuth->eventClients = pEventClient;

    return Success;
} /* SecurityEventSelectForAuthorization */


static int
ProcSecurityGenerateAuthorization(
    ClientPtr client)
{
    REQUEST(xSecurityGenerateAuthorizationReq);
    int len;			/* request length in CARD32s*/
    Bool removeAuth = FALSE;	/* if bailout, call RemoveAuthorization? */
    SecurityAuthorizationPtr pAuth = NULL;  /* auth we are creating */
    int err;			/* error to return from this function */
    XID authId;			/* authorization ID assigned by os layer */
    xSecurityGenerateAuthorizationReply rep; /* reply struct */
    unsigned int trustLevel;    /* trust level of new auth */
    XID group;			/* group of new auth */
    CARD32 timeout;		/* timeout of new auth */
    CARD32 *values;		/* list of supplied attributes */
    char *protoname;		/* auth proto name sent in request */
    char *protodata;		/* auth proto data sent in request */
    unsigned int authdata_len;  /* # bytes of generated auth data */
    char *pAuthdata;		/* generated auth data */
    Mask eventMask;		/* what events on this auth does client want */

    /* paranoia: this "can't happen" because this extension is hidden
     * from untrusted clients, but just in case...
     */
    if (client->trustLevel != XSecurityClientTrusted)
	return BadRequest;

    /* check request length */

    REQUEST_AT_LEAST_SIZE(xSecurityGenerateAuthorizationReq);
    len = SIZEOF(xSecurityGenerateAuthorizationReq) >> 2;
    len += (stuff->nbytesAuthProto + (unsigned)3) >> 2;
    len += (stuff->nbytesAuthData  + (unsigned)3) >> 2;
    values = ((CARD32 *)stuff) + len;
    len += Ones(stuff->valueMask);
    if (client->req_len != len)
	return BadLength;

    /* check valuemask */
    if (stuff->valueMask & ~XSecurityAllAuthorizationAttributes)
    {
	client->errorValue = stuff->valueMask;
	return BadValue;
    }

    /* check timeout */
    timeout = 60;
    if (stuff->valueMask & XSecurityTimeout)
    {
	timeout = *values++;
    }

    /* check trustLevel */
    trustLevel = XSecurityClientUntrusted;
    if (stuff->valueMask & XSecurityTrustLevel)
    {
	trustLevel = *values++;
	if (trustLevel != XSecurityClientTrusted &&
	    trustLevel != XSecurityClientUntrusted)
	{
	    client->errorValue = trustLevel;
	    return BadValue;
	}
    }

    /* check group */
    group = None;
    if (stuff->valueMask & XSecurityGroup)
    {
	group = *values++;
	if (SecurityValidateGroupCallback)
	{
	    SecurityValidateGroupInfoRec vgi;
	    vgi.group = group;
	    vgi.valid = FALSE;
	    CallCallbacks(&SecurityValidateGroupCallback, (pointer)&vgi);

	    /* if nobody said they recognized it, it's an error */

	    if (!vgi.valid)
	    {
		client->errorValue = group;
		return BadValue;
	    }
	}
    }

    /* check event mask */
    eventMask = 0;
    if (stuff->valueMask & XSecurityEventMask)
    {
	eventMask = *values++;
	if (eventMask & ~XSecurityAllEventMasks)
	{
	    client->errorValue = eventMask;
	    return BadValue;
	}
    }

    protoname = (char *)&stuff[1];
    protodata = protoname + ((stuff->nbytesAuthProto + (unsigned)3) >> 2);

    /* call os layer to generate the authorization */

    authId = GenerateAuthorization(stuff->nbytesAuthProto, protoname,
				   stuff->nbytesAuthData,  protodata,
				   &authdata_len, &pAuthdata);
    if ((XID) ~0L == authId)
    {
	err = SecurityErrorBase + XSecurityBadAuthorizationProtocol;
	goto bailout;
    }

    /* now that we've added the auth, remember to remove it if we have to
     * abort the request for some reason (like allocation failure)
     */
    removeAuth = TRUE;

    /* associate additional information with this auth ID */

    pAuth = (SecurityAuthorizationPtr)xalloc(sizeof(SecurityAuthorizationRec));
    if (!pAuth)
    {
	err = BadAlloc;
	goto bailout;
    }

    /* fill in the auth fields */

    pAuth->id = authId;
    pAuth->timeout = timeout;
    pAuth->group = group;
    pAuth->trustLevel = trustLevel;
    pAuth->refcnt = 0;	/* the auth was just created; nobody's using it yet */
    pAuth->secondsRemaining = 0;
    pAuth->timer = NULL;
    pAuth->eventClients = NULL;

    /* handle event selection */
    if (eventMask)
    {
	err = SecurityEventSelectForAuthorization(pAuth, client, eventMask);
	if (err != Success)
	    goto bailout;
    }

    if (!AddResource(authId, SecurityAuthorizationResType, pAuth))
    {
	err = BadAlloc;
	goto bailout;
    }

    /* start the timer ticking */

    if (pAuth->timeout != 0)
	SecurityStartAuthorizationTimer(pAuth);

    /* tell client the auth id and data */

    rep.type = X_Reply;
    rep.length = (authdata_len + 3) >> 2;
    rep.sequenceNumber = client->sequence;
    rep.authId = authId;
    rep.dataLength = authdata_len;

    if (client->swapped)
    {
	register char n;
    	swapl(&rep.length, n);
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.authId, n);
    	swaps(&rep.dataLength, n);
    }

    WriteToClient(client, SIZEOF(xSecurityGenerateAuthorizationReply),
		  (char *)&rep);
    WriteToClient(client, authdata_len, pAuthdata);

    SecurityAudit("client %d generated authorization %d trust %d timeout %d group %d events %d\n",
		  client->index, pAuth->id, pAuth->trustLevel, pAuth->timeout,
		  pAuth->group, eventMask);

    /* the request succeeded; don't call RemoveAuthorization or free pAuth */

    removeAuth = FALSE;
    pAuth = NULL;
    err = client->noClientException;

bailout:
    if (removeAuth)
	RemoveAuthorization(stuff->nbytesAuthProto, protoname,
			    authdata_len, pAuthdata);
    if (pAuth) xfree(pAuth);
    return err;

} /* ProcSecurityGenerateAuthorization */

static int
ProcSecurityRevokeAuthorization(
    ClientPtr client)
{
    REQUEST(xSecurityRevokeAuthorizationReq);
    SecurityAuthorizationPtr pAuth;

    /* paranoia: this "can't happen" because this extension is hidden
     * from untrusted clients, but just in case...
     */
    if (client->trustLevel != XSecurityClientTrusted)
	return BadRequest;

    REQUEST_SIZE_MATCH(xSecurityRevokeAuthorizationReq);

    pAuth = (SecurityAuthorizationPtr)SecurityLookupIDByType(client,
	stuff->authId, SecurityAuthorizationResType, SecurityDestroyAccess);
    if (!pAuth)
	return SecurityErrorBase + XSecurityBadAuthorization;

    FreeResource(stuff->authId, RT_NONE);
    return Success;
} /* ProcSecurityRevokeAuthorization */


static int
ProcSecurityDispatch(
    ClientPtr client)
{
    REQUEST(xReq);

    switch (stuff->data)
    {
	case X_SecurityQueryVersion:
	    return ProcSecurityQueryVersion(client);
	case X_SecurityGenerateAuthorization:
	    return ProcSecurityGenerateAuthorization(client);
	case X_SecurityRevokeAuthorization:
	    return ProcSecurityRevokeAuthorization(client);
	default:
	    return BadRequest;
    }
} /* ProcSecurityDispatch */

static int
SProcSecurityQueryVersion(
    ClientPtr client)
{
    REQUEST(xSecurityQueryVersionReq);
    register char 	n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xSecurityQueryVersionReq);
    swaps(&stuff->majorVersion, n);
    swaps(&stuff->minorVersion,n);
    return ProcSecurityQueryVersion(client);
} /* SProcSecurityQueryVersion */


static int
SProcSecurityGenerateAuthorization(
    ClientPtr client)
{
    REQUEST(xSecurityGenerateAuthorizationReq);
    register char 	n;
    CARD32 *values;
    unsigned long nvalues;

    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xSecurityGenerateAuthorizationReq);
    swaps(&stuff->nbytesAuthProto, n);
    swaps(&stuff->nbytesAuthData, n);
    swapl(&stuff->valueMask, n);
    values = (CARD32 *)(&stuff[1]) +
	((stuff->nbytesAuthProto + (unsigned)3) >> 2) +
	((stuff->nbytesAuthData + (unsigned)3) >> 2);
    nvalues = (((CARD32 *)stuff) + stuff->length) - values;
    SwapLongs(values, nvalues);
    return ProcSecurityGenerateAuthorization(client);
} /* SProcSecurityGenerateAuthorization */


static int
SProcSecurityRevokeAuthorization(
    ClientPtr client)
{
    REQUEST(xSecurityRevokeAuthorizationReq);
    register char 	n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xSecurityRevokeAuthorizationReq);
    swapl(&stuff->authId, n);
    return ProcSecurityRevokeAuthorization(client);
} /* SProcSecurityRevokeAuthorization */


static int
SProcSecurityDispatch(
    ClientPtr client)
{
    REQUEST(xReq);

    switch (stuff->data)
    {
	case X_SecurityQueryVersion:
	    return SProcSecurityQueryVersion(client);
	case X_SecurityGenerateAuthorization:
	    return SProcSecurityGenerateAuthorization(client);
	case X_SecurityRevokeAuthorization:
	    return SProcSecurityRevokeAuthorization(client);
	default:
	    return BadRequest;
    }
} /* SProcSecurityDispatch */

static void 
SwapSecurityAuthorizationRevokedEvent(
    xSecurityAuthorizationRevokedEvent *from,
    xSecurityAuthorizationRevokedEvent *to)
{
    to->type = from->type;
    to->detail = from->detail;
    cpswaps(from->sequenceNumber, to->sequenceNumber);
    cpswapl(from->authId, to->authId);
}

/* SecurityDetermineEventPropogationLimits
 *
 * This is a helper function for SecurityCheckDeviceAccess.
 *
 * Arguments:
 *	dev is the device for which the starting and stopping windows for
 *	event propogation should be determined.
 *	The values pointed to by ppWin and ppStopWin are not used.
 *
 * Returns:
 *	ppWin is filled in with a pointer to the window at which event
 *	propogation for the given device should start given the current
 *	state of the server (pointer position, window layout, etc.)
 *	ppStopWin is filled in with the window at which event propogation
 *	should stop; events should not go to ppStopWin.
 *
 * Side Effects: none.
 */

static void
SecurityDetermineEventPropogationLimits(
    DeviceIntPtr dev,
    WindowPtr *ppWin,
    WindowPtr *ppStopWin)
{
    WindowPtr pFocusWin = dev->focus ? dev->focus->win : NoneWin;

    if (pFocusWin == NoneWin)
    { /* no focus -- events don't go anywhere */
	*ppWin = *ppStopWin = NULL;
	return;
    }

    if (pFocusWin == PointerRootWin)
    { /* focus follows the pointer */
	*ppWin = GetSpriteWindow();
	*ppStopWin = NULL; /* propogate all the way to the root */
    }
    else
    { /* a real window is set for the focus */
	WindowPtr pSpriteWin = GetSpriteWindow();
	*ppStopWin = pFocusWin->parent; /* don't go past the focus window */

	/* if the pointer is in a subwindow of the focus window, start
	 * at that subwindow, else start at the focus window itself
	 */
	if (IsParent(pFocusWin, pSpriteWin))
	     *ppWin = pSpriteWin;
	else *ppWin = pFocusWin;
    }
} /* SecurityDetermineEventPropogationLimits */


/* SecurityCheckDeviceAccess
 *
 * Arguments:
 *	client is the client attempting to access a device.
 *	dev is the device being accessed.
 *	fromRequest is TRUE if the device access is a direct result of
 *	  the client executing some request and FALSE if it is a
 *	  result of the server trying to send an event (e.g. KeymapNotify)
 *	  to the client.
 * Returns:
 *	TRUE if the device access should be allowed, else FALSE.
 *
 * Side Effects:
 *	An audit message is generated if access is denied.
 */

Bool
SecurityCheckDeviceAccess(client, dev, fromRequest)
    ClientPtr client;
    DeviceIntPtr dev;
    Bool fromRequest;
{
    WindowPtr pWin, pStopWin;
    Bool untrusted_got_event;
    Bool found_event_window;
    Mask eventmask;
    int reqtype = 0;

    /* trusted clients always allowed to do anything */
    if (client->trustLevel == XSecurityClientTrusted)
	return TRUE;

    /* device security other than keyboard is not implemented yet */
    if (dev != inputInfo.keyboard)
	return TRUE;

    /* some untrusted client wants access */

    if (fromRequest)
    {
	reqtype = ((xReq *)client->requestBuffer)->reqType;
	switch (reqtype)
	{
	    /* never allow these */
	    case X_ChangeKeyboardMapping:
	    case X_ChangeKeyboardControl:
	    case X_SetModifierMapping:
		SecurityAudit("client %d attempted request %d\n",
			      client->index, reqtype);
		return FALSE;
	    default:
		break;
	}
    }

    untrusted_got_event = FALSE;
    found_event_window = FALSE;

    if (dev->grab)
    {
	untrusted_got_event =
	    ((rClient(dev->grab))->trustLevel != XSecurityClientTrusted);
    }
    else
    {
	SecurityDetermineEventPropogationLimits(dev, &pWin, &pStopWin);

	eventmask = KeyPressMask | KeyReleaseMask;
	while ( (pWin != pStopWin) && !found_event_window)
	{
	    OtherClients *other;

	    if (pWin->eventMask & eventmask)
	    {
		found_event_window = TRUE;
		client = wClient(pWin);
		if (client->trustLevel != XSecurityClientTrusted)
		{
		    untrusted_got_event = TRUE;
		}
	    }
	    if (wOtherEventMasks(pWin) & eventmask)
	    {
		found_event_window = TRUE;
		for (other = wOtherClients(pWin); other; other = other->next)
		{
		    if (other->mask & eventmask)
		    {
			client = rClient(other);
			if (client->trustLevel != XSecurityClientTrusted)
			{
			    untrusted_got_event = TRUE;
			    break;
			}
		    }
		}
	    }
	    if (wDontPropagateMask(pWin) & eventmask)
		break;
	    pWin = pWin->parent;
	} /* while propogating the event */
    }

    /* allow access by untrusted clients only if an event would have gone 
     * to an untrusted client
     */
    
    if (!untrusted_got_event)
    {
	char *devname = dev->name;
	if (!devname) devname = "unnamed";
	if (fromRequest)
	    SecurityAudit("client %d attempted request %d device %d (%s)\n",
			  client->index, reqtype, dev->id, devname);
	else
	    SecurityAudit("client %d attempted to access device %d (%s)\n",
			  client->index, dev->id, devname);
    }
    return untrusted_got_event;
} /* SecurityCheckDeviceAccess */



/* SecurityAuditResourceIDAccess
 *
 * Arguments:
 *	client is the client doing the resource access.
 *	id is the resource id.
 *
 * Returns: NULL
 *
 * Side Effects:
 *	An audit message is generated with details of the denied
 *	resource access.
 */

static pointer
SecurityAuditResourceIDAccess(
    ClientPtr client,
    XID id)
{
    int cid = CLIENT_ID(id);
    int reqtype = ((xReq *)client->requestBuffer)->reqType;
    switch (reqtype)
    {
	case X_ChangeProperty:
	case X_DeleteProperty:
	case X_GetProperty:
	{
	    xChangePropertyReq *req =
		(xChangePropertyReq *)client->requestBuffer;
	    int propertyatom = req->property;
	    char *propertyname = NameForAtom(propertyatom);

	    SecurityAudit("client %d attempted request %d with window 0x%x property %s of client %d\n",
		   client->index, reqtype, id, propertyname, cid);
	    break;
	}
	default:
	{
	    SecurityAudit("client %d attempted request %d with resource 0x%x of client %d\n",
		   client->index, reqtype, id, cid);
	    break;
	}   
    }
    return NULL;
} /* SecurityAuditResourceIDAccess */


/* SecurityCheckResourceIDAccess
 *
 * This function gets plugged into client->CheckAccess and is called from
 * SecurityLookupIDByType/Class to determine if the client can access the
 * resource.
 *
 * Arguments:
 *	client is the client doing the resource access.
 *	id is the resource id.
 *	rtype is its type or class.
 *	access_mode represents the intended use of the resource; see
 *	  resource.h.
 *	rval is a pointer to the resource structure for this resource.
 *
 * Returns:
 *	If access is granted, the value of rval that was passed in, else NULL.
 *
 * Side Effects:
 *	Disallowed resource accesses are audited.
 */

static pointer
SecurityCheckResourceIDAccess(
    ClientPtr client,
    XID id,
    RESTYPE rtype,
    Mask access_mode,
    pointer rval)
{
    int cid = CLIENT_ID(id);
    int reqtype = ((xReq *)client->requestBuffer)->reqType;

    if (SecurityUnknownAccess == access_mode)
	return rval;  /* for compatibility, we have to allow access */

    switch (reqtype)
    { /* these are always allowed */
	case X_QueryTree:
        case X_TranslateCoords:
        case X_GetGeometry:
	/* property access is controlled in SecurityCheckPropertyAccess */
	case X_GetProperty:
	case X_ChangeProperty:
	case X_DeleteProperty:
	case X_RotateProperties:
        case X_ListProperties:
	    return rval;
	default:
	    break;
    }

    if (cid != 0)
    { /* not a server-owned resource */
     /*
      * The following 'if' restricts clients to only access resources at
      * the same trustLevel.  Since there are currently only two trust levels,
      * and trusted clients never call this function, this degenerates into
      * saying that untrusted clients can only access resources of other
      * untrusted clients.  One way to add the notion of groups would be to
      * allow values other than Trusted (0) and Untrusted (1) for this field.
      * Clients at the same trust level would be able to use each other's
      * resources, but not those of clients at other trust levels.  I haven't
      * tried it, but this probably mostly works already.  The obvious
      * competing alternative for grouping clients for security purposes is to
      * use app groups.  dpw
      */
	if (client->trustLevel == clients[cid]->trustLevel
#ifdef XAPPGROUP
	    || (RT_COLORMAP == rtype && 
		XagDefaultColormap (client) == (Colormap) id)
#endif
	)
	    return rval;
	else
	    return SecurityAuditResourceIDAccess(client, id);
    }
    else /* server-owned resource - probably a default colormap or root window */
    {
	if (RT_WINDOW == rtype || RC_DRAWABLE == rtype)
	{
	    switch (reqtype)
	    {   /* the following operations are allowed on root windows */
	        case X_CreatePixmap:
	        case X_CreateGC:
	        case X_CreateWindow:
	        case X_CreateColormap:
		case X_ListProperties:
		case X_GrabPointer:
	        case X_UngrabButton:
		case X_QueryBestSize:
		case X_GetWindowAttributes:
		    break;
		case X_SendEvent:
		{ /* see if it is an event specified by the ICCCM */
		    xSendEventReq *req = (xSendEventReq *)
						(client->requestBuffer);
		    if (req->propagate == xTrue
			||
			  (req->eventMask != ColormapChangeMask &&
			   req->eventMask != StructureNotifyMask &&
			   req->eventMask !=
			      (SubstructureRedirectMask|SubstructureNotifyMask)
			  )
			||
			  (req->event.u.u.type != UnmapNotify &&
			   req->event.u.u.type != ConfigureRequest &&
			   req->event.u.u.type != ClientMessage
			  )
		       )
		    { /* not an ICCCM event */
			return SecurityAuditResourceIDAccess(client, id);
		    }
		    break;
		} /* case X_SendEvent on root */

		case X_ChangeWindowAttributes:
		{ /* Allow selection of PropertyNotify and StructureNotify
		   * events on the root.
		   */
		    xChangeWindowAttributesReq *req =
			(xChangeWindowAttributesReq *)(client->requestBuffer);
		    if (req->valueMask == CWEventMask)
		    {
			CARD32 value = *((CARD32 *)(req + 1));
			if ( (value &
			      ~(PropertyChangeMask|StructureNotifyMask)) == 0)
			    break;
		    }
		    return SecurityAuditResourceIDAccess(client, id);
		} /* case X_ChangeWindowAttributes on root */

		default:
		{
#ifdef LBX
		    /* XXX really need per extension dispatching */
		    if (reqtype == LbxReqCode) {
			switch (((xReq *)client->requestBuffer)->data) {
			case X_LbxGetProperty:
			case X_LbxChangeProperty:
			    return rval;
			default:
			    break;
			}
		    }
#endif
		    /* others not allowed */
		    return SecurityAuditResourceIDAccess(client, id);
		}
	    }
	} /* end server-owned window or drawable */
	else if (SecurityAuthorizationResType == rtype)
	{
	    SecurityAuthorizationPtr pAuth = (SecurityAuthorizationPtr)rval;
	    if (pAuth->trustLevel != client->trustLevel)
		return SecurityAuditResourceIDAccess(client, id);
	}
	else if (RT_COLORMAP != rtype)
	{ /* don't allow anything else besides colormaps */
	    return SecurityAuditResourceIDAccess(client, id);
	}
    }
    return rval;
} /* SecurityCheckResourceIDAccess */


/* SecurityClientStateCallback
 *
 * Arguments:
 *	pcbl is &ClientStateCallback.
 *	nullata is NULL.
 *	calldata is a pointer to a NewClientInfoRec (include/dixstruct.h)
 *	which contains information about client state changes.
 *
 * Returns: nothing.
 *
 * Side Effects:
 * 
 * If a new client is connecting, its authorization ID is copied to
 * client->authID.  If this is a generated authorization, its reference
 * count is bumped, its timer is cancelled if it was running, and its
 * trustlevel is copied to client->trustLevel.
 * 
 * If a client is disconnecting and the client was using a generated
 * authorization, the authorization's reference count is decremented, and
 * if it is now zero, the timer for this authorization is started.
 */

static void
SecurityClientStateCallback(
    CallbackListPtr *pcbl,
    pointer nulldata,
    pointer calldata)
{
    NewClientInfoRec *pci = (NewClientInfoRec *)calldata;
    ClientPtr client = pci->client;

    switch (client->clientState)
    {
	case ClientStateRunning:
	{ 
	    XID authId = AuthorizationIDOfClient(client);
	    SecurityAuthorizationPtr pAuth;

	    client->authId = authId;
	    pAuth = (SecurityAuthorizationPtr)LookupIDByType(authId,
						SecurityAuthorizationResType);
	    if (pAuth)
	    { /* it is a generated authorization */
		pAuth->refcnt++;
		if (pAuth->refcnt == 1)
		{
		    if (pAuth->timer) TimerCancel(pAuth->timer);
		}
		client->trustLevel = pAuth->trustLevel;
		if (client->trustLevel != XSecurityClientTrusted)
		{
		    client->CheckAccess = SecurityCheckResourceIDAccess;
		    client->requestVector = client->swapped ?
			SwappedUntrustedProcVector : UntrustedProcVector;
		}
	    }
	    break;
	}
	case ClientStateGone:
	case ClientStateRetained: /* client disconnected */
	{
	    XID authId = client->authId;
	    SecurityAuthorizationPtr pAuth;

	    pAuth = (SecurityAuthorizationPtr)LookupIDByType(authId,
						SecurityAuthorizationResType);
	    if (pAuth)
	    { /* it is a generated authorization */
		pAuth->refcnt--;
		if (pAuth->refcnt == 0)
		{
		    SecurityStartAuthorizationTimer(pAuth);
		}
	    }	    
	    break;
	}
	default: break; 
    }
} /* SecurityClientStateCallback */

#ifdef LBX
Bool
SecuritySameLevel(client, authId)
    ClientPtr client;
    XID authId;
{
    SecurityAuthorizationPtr pAuth;

    pAuth = (SecurityAuthorizationPtr)LookupIDByType(authId,
						SecurityAuthorizationResType);
    if (pAuth)
	return client->trustLevel == pAuth->trustLevel;
    return client->trustLevel == XSecurityClientTrusted;
}
#endif

/* SecurityCensorImage
 *
 * Called after pScreen->GetImage to prevent pieces or trusted windows from
 * being returned in image data from an untrusted window.
 *
 * Arguments:
 *	client is the client doing the GetImage.
 *      pVisibleRegion is the visible region of the window.
 *	widthBytesLine is the width in bytes of one horizontal line in pBuf.
 *	pDraw is the source window.
 *	x, y, w, h is the rectangle of image data from pDraw in pBuf.
 *	format is the format of the image data in pBuf: ZPixmap or XYPixmap.
 *	pBuf is the image data.
 *
 * Returns: nothing.
 *
 * Side Effects:
 *	Any part of the rectangle (x, y, w, h) that is outside the visible
 *	region of the window will be destroyed (overwritten) in pBuf.
 */
void
SecurityCensorImage(client, pVisibleRegion, widthBytesLine, pDraw, x, y, w, h,
		    format, pBuf)
    ClientPtr client;
    RegionPtr pVisibleRegion;
    long widthBytesLine;
    DrawablePtr pDraw;
    int x, y, w, h;
    unsigned int format;
    char * pBuf;
{
    ScreenPtr pScreen = pDraw->pScreen;
    RegionRec imageRegion;  /* region representing x,y,w,h */
    RegionRec censorRegion; /* region to obliterate */
    BoxRec imageBox;
    int nRects;

    imageBox.x1 = x;
    imageBox.y1 = y;
    imageBox.x2 = x + w;
    imageBox.y2 = y + h;
    REGION_INIT(pScreen, &imageRegion, &imageBox, 1);
    REGION_NULL(pScreen, &censorRegion);

    /* censorRegion = imageRegion - visibleRegion */
    REGION_SUBTRACT(pScreen, &censorRegion, &imageRegion, pVisibleRegion);
    nRects = REGION_NUM_RECTS(&censorRegion);
    if (nRects > 0)
    { /* we have something to censor */
	GCPtr pScratchGC = NULL;
	PixmapPtr pPix = NULL;
	xRectangle *pRects = NULL;
	Bool failed = FALSE;
	int depth = 1;
	int bitsPerPixel = 1;
	int i;
	BoxPtr pBox;

	/* convert region to list-of-rectangles for PolyFillRect */

	pRects = (xRectangle *)ALLOCATE_LOCAL(nRects * sizeof(xRectangle *));
	if (!pRects)
	{
	    failed = TRUE;
	    goto failSafe;
	}
	for (pBox = REGION_RECTS(&censorRegion), i = 0;
	     i < nRects;
	     i++, pBox++)
	{
	    pRects[i].x = pBox->x1;
	    pRects[i].y = pBox->y1 - imageBox.y1;
	    pRects[i].width  = pBox->x2 - pBox->x1;
	    pRects[i].height = pBox->y2 - pBox->y1;
	}

	/* use pBuf as a fake pixmap */

	if (format == ZPixmap)
	{
	    depth = pDraw->depth;
	    bitsPerPixel = pDraw->bitsPerPixel;
	}

	pPix = GetScratchPixmapHeader(pDraw->pScreen, w, h,
		    depth, bitsPerPixel,
		    widthBytesLine, (pointer)pBuf);
	if (!pPix)
	{
	    failed = TRUE;
	    goto failSafe;
	}

	pScratchGC = GetScratchGC(depth, pPix->drawable.pScreen);
	if (!pScratchGC)
	{
	    failed = TRUE;
	    goto failSafe;
	}

	ValidateGC(&pPix->drawable, pScratchGC);
	(* pScratchGC->ops->PolyFillRect)(&pPix->drawable,
			    pScratchGC, nRects, pRects);

    failSafe:
	if (failed)
	{
	    /* Censoring was not completed above.  To be safe, wipe out
	     * all the image data so that nothing trusted gets out.
	     */
	    bzero(pBuf, (int)(widthBytesLine * h));
	}
	if (pRects)     DEALLOCATE_LOCAL(pRects);
	if (pScratchGC) FreeScratchGC(pScratchGC);
	if (pPix)       FreeScratchPixmapHeader(pPix);
    }
    REGION_UNINIT(pScreen, &imageRegion);
    REGION_UNINIT(pScreen, &censorRegion);
} /* SecurityCensorImage */

/**********************************************************************/

typedef struct _PropertyAccessRec {
    ATOM name;
    ATOM mustHaveProperty;
    char *mustHaveValue;
    char windowRestriction;
#define SecurityAnyWindow          0
#define SecurityRootWindow         1
#define SecurityWindowWithProperty 2
    char readAction;
    char writeAction;
    char destroyAction;
    struct _PropertyAccessRec *next;
} PropertyAccessRec, *PropertyAccessPtr;

static PropertyAccessPtr PropertyAccessList = NULL;
static char SecurityDefaultAction = SecurityErrorOperation;
static char *SecurityPolicyFile = DEFAULTPOLICYFILE;
static ATOM SecurityMaxPropertyName = 0;

static char *SecurityKeywords[] = {
#define SecurityKeywordComment 0
    "#",
#define SecurityKeywordProperty 1
    "property",
#define SecurityKeywordSitePolicy 2
    "sitepolicy",
#define SecurityKeywordRoot 3
    "root",
#define SecurityKeywordAny 4
    "any"
};

#define NUMKEYWORDS (sizeof(SecurityKeywords) / sizeof(char *))

#undef PROPDEBUG
/*#define PROPDEBUG  1*/

static void
SecurityFreePropertyAccessList(void)
{
    while (PropertyAccessList)
    {
	PropertyAccessPtr freeit = PropertyAccessList;
	PropertyAccessList = PropertyAccessList->next;
	xfree(freeit);
    }
} /* SecurityFreePropertyAccessList */

#ifndef __UNIXOS2__
#define SecurityIsWhitespace(c) ( (c == ' ') || (c == '\t') || (c == '\n') )
#else
#define SecurityIsWhitespace(c) ( (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') )
#endif

static char *
SecuritySkipWhitespace(
    char *p)
{
    while (SecurityIsWhitespace(*p))
	p++;
    return p;
} /* SecuritySkipWhitespace */


static char *
SecurityParseString(
    char **rest)
{
    char *startOfString;
    char *s = *rest;
    char endChar = 0;

    s = SecuritySkipWhitespace(s);

    if (*s == '"' || *s == '\'')
    {
	endChar = *s++;
	startOfString = s;
	while (*s && (*s != endChar))
	    s++;
    }
    else
    {
	startOfString = s;
	while (*s && !SecurityIsWhitespace(*s))
	    s++;
    }
    if (*s)
    {
	*s = '\0';
	*rest = s + 1;
	return startOfString;
    }
    else
    {
	*rest = s;
	return (endChar) ? NULL : startOfString;
    }
} /* SecurityParseString */


static int
SecurityParseKeyword(
    char **p)
{
    int i;
    char *s = *p;
    s = SecuritySkipWhitespace(s);
    for (i = 0; i < NUMKEYWORDS; i++)
    {
	int len = strlen(SecurityKeywords[i]);
	if (strncmp(s, SecurityKeywords[i], len) == 0)
	{
	    *p = s + len;
	    return (i);
	}
    }
    *p = s;
    return -1;
} /* SecurityParseKeyword */


static Bool
SecurityParsePropertyAccessRule(
    char *p)
{
    char *propname;
    char c;
    char action = SecurityDefaultAction;
    char readAction, writeAction, destroyAction;
    PropertyAccessPtr pacl, prev, cur;
    char *mustHaveProperty = NULL;
    char *mustHaveValue = NULL;
    Bool invalid;
    char windowRestriction;
    int size;
    int keyword;

    /* get property name */
    propname = SecurityParseString(&p);
    if (!propname || (strlen(propname) == 0))
	return FALSE;

    /* get window on which property must reside for rule to apply */

    keyword = SecurityParseKeyword(&p);
    if (keyword == SecurityKeywordRoot)
	windowRestriction = SecurityRootWindow;
    else if (keyword == SecurityKeywordAny) 
	windowRestriction = SecurityAnyWindow;
    else /* not root or any, must be a property name */
    {
	mustHaveProperty = SecurityParseString(&p);
	if (!mustHaveProperty || (strlen(mustHaveProperty) == 0))
	    return FALSE;
	windowRestriction = SecurityWindowWithProperty;
	p = SecuritySkipWhitespace(p);
	if (*p == '=')
	{ /* property value is specified too */
	    p++; /* skip over '=' */
	    mustHaveValue = SecurityParseString(&p);
	    if (!mustHaveValue)
		return FALSE;
	}
    }

    /* get operations and actions */

    invalid = FALSE;
    readAction = writeAction = destroyAction = SecurityDefaultAction;
    while ( (c = *p++) && !invalid)
    {
	switch (c)
	{
	    case 'i': action = SecurityIgnoreOperation; break;
	    case 'a': action = SecurityAllowOperation;  break;
	    case 'e': action = SecurityErrorOperation;  break;

	    case 'r': readAction    = action; break;
	    case 'w': writeAction   = action; break;
	    case 'd': destroyAction = action; break;

	    default :
		if (!SecurityIsWhitespace(c))
		    invalid = TRUE;
	    break;
	}
    }
    if (invalid)
	return FALSE;

    /* We've successfully collected all the information needed for this
     * property access rule.  Now record it in a PropertyAccessRec.
     */
    size = sizeof(PropertyAccessRec);

    /* If there is a property value string, allocate space for it 
     * right after the PropertyAccessRec.
     */
    if (mustHaveValue)
	size += strlen(mustHaveValue) + 1;
    pacl = (PropertyAccessPtr)Xalloc(size);
    if (!pacl)
	return FALSE;

    pacl->name = MakeAtom(propname, strlen(propname), TRUE);
    if (pacl->name == BAD_RESOURCE)
    {
	Xfree(pacl);
	return FALSE;
    }
    if (mustHaveProperty)
    {
	pacl->mustHaveProperty = MakeAtom(mustHaveProperty,
					  strlen(mustHaveProperty), TRUE);
	if (pacl->mustHaveProperty == BAD_RESOURCE)
	{
	    Xfree(pacl);
	    return FALSE;
	}
    }
    else
	pacl->mustHaveProperty = 0;

    if (mustHaveValue)
    {
	pacl->mustHaveValue = (char *)(pacl + 1);
	strcpy(pacl->mustHaveValue, mustHaveValue);
    }
    else
	pacl->mustHaveValue = NULL;

    SecurityMaxPropertyName = max(SecurityMaxPropertyName, pacl->name);

    pacl->windowRestriction = windowRestriction;
    pacl->readAction  = readAction;
    pacl->writeAction = writeAction;
    pacl->destroyAction = destroyAction;

    /* link the new rule into the list of rules in order of increasing
     * property name (atom) value to make searching easier
     */

    for (prev = NULL,  cur = PropertyAccessList;
	 cur && cur->name <= pacl->name;
	 prev = cur, cur = cur->next)
	;
    if (!prev)
    {
	pacl->next = cur;
	PropertyAccessList = pacl;
    }
    else
    {
	prev->next = pacl;
	pacl->next = cur;
    }
    return TRUE;
} /* SecurityParsePropertyAccessRule */

static char **SecurityPolicyStrings = NULL;
static int nSecurityPolicyStrings = 0;

static Bool
SecurityParseSitePolicy(
    char *p)
{
    char *policyStr = SecurityParseString(&p);
    char *copyPolicyStr;
    char **newStrings;

    if (!policyStr)
	return FALSE;

    copyPolicyStr = (char *)Xalloc(strlen(policyStr) + 1);
    if (!copyPolicyStr)
	return TRUE;
    strcpy(copyPolicyStr, policyStr);
    newStrings = (char **)Xrealloc(SecurityPolicyStrings,
			  sizeof (char *) * (nSecurityPolicyStrings + 1));
    if (!newStrings)
    {
	Xfree(copyPolicyStr);
	return TRUE;
    }

    SecurityPolicyStrings = newStrings;
    SecurityPolicyStrings[nSecurityPolicyStrings++] = copyPolicyStr;

    return TRUE;

} /* SecurityParseSitePolicy */


char **
SecurityGetSitePolicyStrings(n)
    int *n;
{
    *n = nSecurityPolicyStrings;
    return SecurityPolicyStrings;
} /* SecurityGetSitePolicyStrings */

static void
SecurityFreeSitePolicyStrings(void)
{
    if (SecurityPolicyStrings)
    {
	assert(nSecurityPolicyStrings);
	while (nSecurityPolicyStrings--)
	{
	    Xfree(SecurityPolicyStrings[nSecurityPolicyStrings]);
	}
	Xfree(SecurityPolicyStrings);
	SecurityPolicyStrings = NULL;
	nSecurityPolicyStrings = 0;
    }
} /* SecurityFreeSitePolicyStrings */


static void
SecurityLoadPropertyAccessList(void)
{
    FILE *f;
    int lineNumber = 0;

    SecurityMaxPropertyName = 0;

    if (!SecurityPolicyFile)
	return;

#ifndef __UNIXOS2__
    f = fopen(SecurityPolicyFile, "r");
#else
    f = fopen((char*)__XOS2RedirRoot(SecurityPolicyFile), "r");
#endif    
    if (!f)
    {
	ErrorF("error opening security policy file %s\n",
	       SecurityPolicyFile);
	return;
    }

    while (!feof(f))
    {
	char buf[200];
	Bool validLine;
	char *p;

	if (!(p = fgets(buf, sizeof(buf), f)))
	    break;
	lineNumber++;

	/* if first line, check version number */
	if (lineNumber == 1)
	{
	    char *v = SecurityParseString(&p);
	    if (strcmp(v, SECURITY_POLICY_FILE_VERSION) != 0)
	    {
		ErrorF("%s: invalid security policy file version, ignoring file\n",
		       SecurityPolicyFile);
		break;
	    }
	    validLine = TRUE;
	}
	else
	{
	    switch (SecurityParseKeyword(&p))
	    {
		case SecurityKeywordComment:
		    validLine = TRUE;
		break;

		case SecurityKeywordProperty:
		    validLine = SecurityParsePropertyAccessRule(p);
		break;

		case SecurityKeywordSitePolicy:
		    validLine = SecurityParseSitePolicy(p);
		break;

		default:
		    validLine = (*p == '\0'); /* blank lines OK, others not */
		break;
	    }
	}

	if (!validLine)
	    ErrorF("Line %d of %s invalid, ignoring\n",
		   lineNumber, SecurityPolicyFile);
    } /* end while more input */

#ifdef PROPDEBUG
    {
	PropertyAccessPtr pacl;
	char *op = "aie";
	for (pacl = PropertyAccessList; pacl; pacl = pacl->next)
	{
	    ErrorF("property %s ", NameForAtom(pacl->name));
	    switch (pacl->windowRestriction)
	    {
		case SecurityAnyWindow: ErrorF("any "); break;
		case SecurityRootWindow: ErrorF("root "); break;
		case SecurityWindowWithProperty:
		{
		    ErrorF("%s ", NameForAtom(pacl->mustHaveProperty));
		    if (pacl->mustHaveValue)
			ErrorF(" = \"%s\" ", pacl->mustHaveValue);

		}
		break;
	    }
	    ErrorF("%cr %cw %cd\n", op[pacl->readAction],
		   op[pacl->writeAction], op[pacl->destroyAction]);
	}
    }
#endif /* PROPDEBUG */

    fclose(f);
} /* SecurityLoadPropertyAccessList */


static Bool
SecurityMatchString(
    char *ws,
    char *cs)
{
    while (*ws && *cs)
    {
	if (*ws == '*')
	{
	    Bool match = FALSE;
	    ws++;
	    while (!(match = SecurityMatchString(ws, cs)) && *cs)
	    {
		cs++;
	    }
	    return match;
	}
	else if (*ws == *cs)
	{
	    ws++;
	    cs++;
	}
	else break;
    }
    return ( ( (*ws == '\0') || ((*ws == '*') && *(ws+1) == '\0') )
	     && (*cs == '\0') );
} /* SecurityMatchString */

#ifdef PROPDEBUG
#include <sys/types.h>
#include <sys/stat.h>
#endif


char
SecurityCheckPropertyAccess(client, pWin, propertyName, access_mode)
    ClientPtr client;
    WindowPtr pWin;
    ATOM propertyName;
    Mask access_mode;
{
    PropertyAccessPtr pacl;
    char action = SecurityDefaultAction;

    /* if client trusted or window untrusted, allow operation */

    if ( (client->trustLevel == XSecurityClientTrusted) ||
	 (wClient(pWin)->trustLevel != XSecurityClientTrusted) )
	return SecurityAllowOperation;

#ifdef PROPDEBUG
    /* For testing, it's more convenient if the property rules file gets
     * reloaded whenever it changes, so we can rapidly try things without
     * having to reset the server.
     */
    {
	struct stat buf;
	static time_t lastmod = 0;
	int ret = stat(SecurityPolicyFile , &buf);
	if ( (ret == 0) && (buf.st_mtime > lastmod) )
	{
	    ErrorF("reloading property rules\n");
	    SecurityFreePropertyAccessList();
	    SecurityLoadPropertyAccessList();
	    lastmod = buf.st_mtime;
	}
    }
#endif

    /* If the property atom is bigger than any atoms on the list, 
     * we know we won't find it, so don't even bother looking.
     */
    if (propertyName <= SecurityMaxPropertyName)
    {
	/* untrusted client operating on trusted window; see if it's allowed */

	for (pacl = PropertyAccessList; pacl; pacl = pacl->next)
	{
	    if (pacl->name < propertyName)
		continue;
	    if (pacl->name > propertyName)
		break;

	    /* pacl->name == propertyName, so see if it applies to this window */

	    switch (pacl->windowRestriction)
	    {
		case SecurityAnyWindow: /* always applies */
		    break;

		case SecurityRootWindow:
		{
		    /* if not a root window, this rule doesn't apply */
		    if (pWin->parent)
			continue;
		}
		break;

		case SecurityWindowWithProperty:
		{
		    PropertyPtr pProp = wUserProps (pWin);
		    Bool match = FALSE;
		    char *p;
		    char *pEndData;

		    while (pProp)
		    {
			if (pProp->propertyName == pacl->mustHaveProperty)
			    break;
			pProp = pProp->next;
		    }
		    if (!pProp)
			continue;
		    if (!pacl->mustHaveValue)
			break;
		    if (pProp->type != XA_STRING || pProp->format != 8)
			continue;

		    p = pProp->data;
		    pEndData = ((char *)pProp->data) + pProp->size;
		    while (!match && p < pEndData)
		    {
			 if (SecurityMatchString(pacl->mustHaveValue, p))
			     match = TRUE;
			 else
			 { /* skip to the next string */
			     while (*p++ && p < pEndData)
				 ;
			 }
		    }
		    if (!match)
			continue;
		}
		break; /* end case SecurityWindowWithProperty */
	    } /* end switch on windowRestriction */

	    /* If we get here, the property access rule pacl applies.
	     * If pacl doesn't apply, something above should have
	     * executed a continue, which will skip the follwing code.
	     */
	    action = SecurityAllowOperation;
	    if (access_mode & SecurityReadAccess)
		action = max(action, pacl->readAction);
	    if (access_mode & SecurityWriteAccess)
		action = max(action, pacl->writeAction);
	    if (access_mode & SecurityDestroyAccess)
		action = max(action, pacl->destroyAction);
	    break;
	} /* end for each pacl */
    } /* end if propertyName <= SecurityMaxPropertyName */

    if (SecurityAllowOperation != action)
    { /* audit the access violation */
	int cid = CLIENT_ID(pWin->drawable.id);
	int reqtype = ((xReq *)client->requestBuffer)->reqType;
	char *actionstr = (SecurityIgnoreOperation == action) ?
							"ignored" : "error";
	SecurityAudit("client %d attempted request %d with window 0x%x property %s (atom 0x%x) of client %d, %s\n",
		client->index, reqtype, pWin->drawable.id,
		      NameForAtom(propertyName), propertyName, cid, actionstr);
    }
    return action;
} /* SecurityCheckPropertyAccess */


/* SecurityResetProc
 *
 * Arguments:
 *	extEntry is the extension information for the security extension.
 *
 * Returns: nothing.
 *
 * Side Effects:
 *	Performs any cleanup needed by Security at server shutdown time.
 */

static void
SecurityResetProc(
    ExtensionEntry *extEntry)
{
    SecurityFreePropertyAccessList();
    SecurityFreeSitePolicyStrings();
} /* SecurityResetProc */


int
XSecurityOptions(argc, argv, i)
    int argc;
    char **argv;
    int i;
{
    if (strcmp(argv[i], "-sp") == 0)
    {
	if (i < argc)
	    SecurityPolicyFile = argv[++i];
	return (i + 1);
    }
    return (i);
} /* XSecurityOptions */



/* SecurityExtensionInit
 *
 * Arguments: none.
 *
 * Returns: nothing.
 *
 * Side Effects:
 *	Enables the Security extension if possible.
 */

void
SecurityExtensionInit(INITARGS)
{
    ExtensionEntry	*extEntry;
    int i;

    SecurityAuthorizationResType =
	CreateNewResourceType(SecurityDeleteAuthorization);

    RTEventClient = CreateNewResourceType(
				SecurityDeleteAuthorizationEventClient);

    if (!SecurityAuthorizationResType || !RTEventClient)
	return;

    RTEventClient |= RC_NEVERRETAIN;

    if (!AddCallback(&ClientStateCallback, SecurityClientStateCallback, NULL))
	return;

    extEntry = AddExtension(SECURITY_EXTENSION_NAME,
			    XSecurityNumberEvents, XSecurityNumberErrors,
			    ProcSecurityDispatch, SProcSecurityDispatch,
                            SecurityResetProc, StandardMinorOpcode);

    SecurityErrorBase = extEntry->errorBase;
    SecurityEventBase = extEntry->eventBase;

    EventSwapVector[SecurityEventBase + XSecurityAuthorizationRevoked] =
	(EventSwapPtr)SwapSecurityAuthorizationRevokedEvent;

    /* initialize untrusted proc vectors */

    for (i = 0; i < 128; i++)
    {
	UntrustedProcVector[i] = ProcVector[i];
	SwappedUntrustedProcVector[i] = SwappedProcVector[i];
    }

    /* make sure insecure extensions are not allowed */

    for (i = 128; i < 256; i++)
    {
	if (!UntrustedProcVector[i])
	{
	    UntrustedProcVector[i] = ProcBadRequest;
	    SwappedUntrustedProcVector[i] = ProcBadRequest;
	}
    }

    SecurityLoadPropertyAccessList();

} /* SecurityExtensionInit */
