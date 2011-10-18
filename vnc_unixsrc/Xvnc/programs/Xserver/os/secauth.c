/* $XConsortium: secauth.c /main/4 1996/11/27 16:57:14 swick $ */
/*
Copyright (c) 1996  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.
*/

#include "X.h"
#include "os.h"
#include "osdep.h"
#include "dixstruct.h"

/*================================================================
   BEGIN ORL VNC modification
   Need function prototype for SecurityGetSitePolicyStrings even when
   XCSECURITY isn't defined */

/* REMOVE
#ifdef XCSECURITY
*/
#define _SECURITY_SERVER
#include "extensions/security.h"
/* REMOVE
#endif
*/

/* END ORL VNC modification
================================================================*/

static char InvalidPolicyReason[] = "invalid policy specification";
static char PolicyViolationReason[] = "policy violation";

static Bool
AuthCheckSitePolicy(data_lengthP, dataP, client, reason)
    unsigned short *data_lengthP;
    char	**dataP;
    ClientPtr	client;
    char	**reason;
{
    char	*policy = *dataP;
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
    nPolicies = *policy++;

    length -= 2;

    sitePolicies = SecurityGetSitePolicyStrings(&nSitePolicies);

    while (nPolicies) {
	int strLen, sitePolicy;

	if (length == 0) {
	    *reason = InvalidPolicyReason;
	    return FALSE;
	}

	strLen = *policy++;
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
		    (strncmp(policy, testPolicy, strLen) == 0))
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
    *dataP = policy;
    return TRUE;
}

XID
AuthSecurityCheck (data_length, data, client, reason)
    unsigned short	data_length;
    char	*data;
    ClientPtr client;
    char	**reason;
{
#ifdef XCSECURITY
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    register ConnectionInputPtr oci = oc->input;
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
