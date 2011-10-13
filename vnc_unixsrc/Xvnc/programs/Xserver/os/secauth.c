/* $Xorg: secauth.c,v 1.4 2001/02/09 02:05:23 xorgcvs Exp $ */
/*
Copyright 1996, 1998  The Open Group

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
/* $XFree86: xc/programs/Xserver/os/secauth.c,v 1.10 2001/08/01 00:44:59 tsi Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include "os.h"
#include "osdep.h"
#include "dixstruct.h"
#include "swaprep.h"

/*================================================================
   BEGIN ORL VNC modification
   Need function prototype for SecurityGetSitePolicyStrings even when
   XCSECURITY isn't defined */

/* REMOVE
#ifdef XCSECURITY
*/
#define _SECURITY_SERVER
#include <X11/extensions/security.h>
/* REMOVE
#endif
*/

/* END ORL VNC modification
================================================================*/

static char InvalidPolicyReason[] = "invalid policy specification";
static char PolicyViolationReason[] = "policy violation";

static Bool
AuthCheckSitePolicy(
    unsigned short *data_lengthP,
    char	**dataP,
    ClientPtr	client,
    char	**reason)
{
    CARD8	*policy = *(CARD8 **)dataP;
    int		length;
    Bool	permit;
    int		nPolicies;
    char	**sitePolicies;
    int		nSitePolicies;
    Bool	found = FALSE;

    if ((length = *data_lengthP) < 2) {
	*reason = InvalidPolicyReason;
	return FALSE;
    }

    permit = (*policy++ == 0);
    nPolicies = (CARD8) *policy++;

    length -= 2;

    sitePolicies = SecurityGetSitePolicyStrings(&nSitePolicies);

    while (nPolicies) {
	int strLen, sitePolicy;

	if (length == 0) {
	    *reason = InvalidPolicyReason;
	    return FALSE;
	}

	strLen = (CARD8) *policy++;
	if (--length < strLen) {
	    *reason = InvalidPolicyReason;
	    return FALSE;
	}

	if (!found)
	{
	    for (sitePolicy = 0; sitePolicy < nSitePolicies; sitePolicy++)
	    {
		char *testPolicy = sitePolicies[sitePolicy];
		if ((strLen == strlen(testPolicy)) &&
		    (strncmp((char *)policy, testPolicy, strLen) == 0))
		{
		    found = TRUE; /* need to continue parsing the policy... */
		    break;
		}
	    }
	}

	policy += strLen;
	length -= strLen;
	nPolicies--;
    }

    if (found != permit)
    {
	*reason = PolicyViolationReason;
	return FALSE;
    }

    *data_lengthP = length;
    *dataP = (char *)policy;
    return TRUE;
}

XID
AuthSecurityCheck (
    unsigned short	data_length,
    char		*data,
    ClientPtr		client,
    char		**reason)
{
#ifdef XCSECURITY
    xConnSetupPrefix csp;
    xReq freq;

    if (client->clientState == ClientStateCheckedSecurity)
    {
	*reason = "repeated security check not permitted";
	return (XID) -1;
    }
    else if (data_length > 0)
    {
	char policy_mask = *data++;

	if (--data_length == 1) {
	    *reason = InvalidPolicyReason;
	    return (XID) -1;
	}

	if (policy_mask & 0x01)	/* Extensions policy */
	{
	 /* AuthCheckExtensionPolicy(&data_length, &data, client, reason) */
	    *reason = "security policy not implemented";
	    return (XID) -1;
	}

	if (policy_mask & 0x02)	/* Site policy */
	{
	    if (!AuthCheckSitePolicy(&data_length, &data, client, reason))
		return (XID) -1;
	}

	if (data_length > 0) {	/* did we consume the whole policy? */
	    *reason = InvalidPolicyReason;
	    return (XID) -1;
	}

    }
    else if (!GetAccessControl())
    {
	/*
	 * The client - possibly the X FireWall Proxy - gave
	 * no auth data and host-based authorization is turned
	 * off.  In this case, the client should be denied
	 * access to the X server.
	 */
	*reason = "server host access control is disabled";
	return (XID) -1;
    }

    client->clientState = ClientStateCheckingSecurity;

    csp.success = 2 /* Authenticate */;
    csp.lengthReason = 0;
    csp.length = 0;
    csp.majorVersion = X_PROTOCOL;
    csp.minorVersion = X_PROTOCOL_REVISION;
    if (client->swapped)
	WriteSConnSetupPrefix(client, &csp);
    else
	(void)WriteToClient(client, sz_xConnSetupPrefix, (char *) &csp);

    /*
     * Next time the client sends the real auth data, we want
     * ProcEstablishConnection to be called.
     */

    freq.reqType = 1;
    freq.length = (sz_xReq + sz_xConnClientPrefix) >> 2;
    client->swapped = FALSE;
    if (!InsertFakeRequest(client, (char *)&freq, sz_xReq))
    {
	*reason = "internal error";
	return (XID) -1;
    }

    return (XID) 0;
#else
    *reason = "method not supported";
    return (XID) -1;
#endif
}
