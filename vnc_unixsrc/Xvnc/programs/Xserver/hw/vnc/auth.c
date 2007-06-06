/*
 * auth.c - deal with authentication.
 *
 * This file implements authentication when setting up an RFB connection.
 */

/*
 *  Copyright (C) 2003-2006 Constantin Kaplinsky.  All Rights Reserved.
 *  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 *  USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include "windowstr.h"
#include "rfb.h"


char *rfbAuthPasswdFile = NULL;

static void rfbSendSecurityType(rfbClientPtr cl, int securityType);
static void rfbSendSecurityTypeList(rfbClientPtr cl, int primaryType);
static void rfbSendTunnelingCaps(rfbClientPtr cl);
static void rfbSendAuthCaps(rfbClientPtr cl);

static void rfbVncAuthSendChallenge(rfbClientPtr cl);


/*
 * rfbAuthNewClient is called right after negotiating the protocol
 * version. Depending on the protocol version, we send either a code
 * for authentication scheme to be used (protocol 3.3), or a list of
 * possible "security types" (protocol 3.7 and above).
 */

void
rfbAuthNewClient(cl)
    rfbClientPtr cl;
{
    int securityType = rfbSecTypeInvalid;

    if (!rfbAuthPasswdFile || cl->reverseConnection) {
	securityType = rfbSecTypeNone;
    } else {
	if (rfbAuthIsBlocked()) {
	    rfbLog("Too many authentication failures - client rejected\n");
	    rfbClientConnFailed(cl, "Too many authentication failures");
	    return;
	}
	if (rfbAuthPasswdFile)
	    securityType = rfbSecTypeVncAuth;
    }

    if (cl->protocol_minor_ver < 7) {
	/* Make sure we use only RFB 3.3 compatible security types. */
	if (securityType == rfbSecTypeInvalid) {
	    rfbLog("VNC authentication disabled - RFB 3.3 client rejected\n");
	    rfbClientConnFailed(cl, "Your viewer cannot handle required "
				"authentication methods");
	    return;
	}
	rfbSendSecurityType(cl, securityType);
    } else {
	/* Here it's ok when securityType is set to rfbSecTypeInvalid. */
	rfbSendSecurityTypeList(cl, securityType);
    }
}


/*
 * Tell the client what security type will be used (protocol 3.3).
 */

static void
rfbSendSecurityType(cl, securityType)
    rfbClientPtr cl;
    int securityType;
{
    CARD32 value32;

    /* Send the value. */
    value32 = Swap32IfLE(securityType);
    if (WriteExact(cl->sock, (char *)&value32, 4) < 0) {
	rfbLogPerror("rfbSendSecurityType: write");
	rfbCloseSock(cl->sock);
	return;
    }

    /* Decide what to do next. */
    switch (securityType) {
    case rfbSecTypeNone:
	/* Dispatch client input to rfbProcessClientInitMessage. */
	cl->state = RFB_INITIALISATION;
	break;
    case rfbSecTypeVncAuth:
	/* Begin the standard VNC authentication procedure. */
	rfbVncAuthSendChallenge(cl);
	break;
    default:
	/* Impossible case (hopefully). */
	rfbLogPerror("rfbSendSecurityType: assertion failed");
	rfbCloseSock(cl->sock);
    }
}


/*
 * Advertise our supported security types (protocol 3.7 and above).
 * The list includes one standard security type (if primaryType is not
 * set to rfbSecTypeInvalid), and then one more value telling the
 * client that we support TightVNC protocol extensions. Thus,
 * currently, there will be either 1 or 2 items in the list.
 */

static void
rfbSendSecurityTypeList(cl, primaryType)
    rfbClientPtr cl;
    int primaryType;
{
    int count = 1;

    /* Fill in the list of security types in the client structure. */
    if (primaryType != rfbSecTypeInvalid) {
	cl->securityTypes[count++] = (CARD8)primaryType;
    }
    cl->securityTypes[count] = (CARD8)rfbSecTypeTight;
    cl->securityTypes[0] = (CARD8)count++;

    /* Send the list. */
    if (WriteExact(cl->sock, (char *)cl->securityTypes, count) < 0) {
	rfbLogPerror("rfbSendSecurityTypeList: write");
	rfbCloseSock(cl->sock);
	return;
    }

    /* Dispatch client input to rfbProcessClientSecurityType. */
    cl->state = RFB_SECURITY_TYPE;
}


/*
 * Read the security type chosen by the client (protocol 3.7 and
 * above).
 */

void
rfbProcessClientSecurityType(cl)
    rfbClientPtr cl;
{
    int n, count, i;
    CARD8 chosenType;

    /* Read the security type. */
    n = ReadExact(cl->sock, (char *)&chosenType, 1);
    if (n <= 0) {
	if (n == 0)
	    rfbLog("rfbProcessClientSecurityType: client gone\n");
	else
	    rfbLogPerror("rfbProcessClientSecurityType: read");
	rfbCloseSock(cl->sock);
	return;
    }

    /* Make sure it was present in the list sent by the server. */
    count = (int)cl->securityTypes[0];
    for (i = 1; i <= count; i++) {
	if (chosenType == cl->securityTypes[i])
	    break;
    }
    if (i > count) {
	rfbLog("rfbProcessClientSecurityType: "
	       "wrong security type requested\n");
	rfbCloseSock(cl->sock);
	return;
    }

    /* Now go to the proper authentication procedure. */
    switch (chosenType) {
    case rfbSecTypeNone:
	/* No authentication needed. */
	rfbClientAuthSucceeded(cl, rfbAuthNone);
	break;
    case rfbSecTypeVncAuth:
	/* Begin the standard VNC authentication procedure. */
	rfbVncAuthSendChallenge(cl);
	break;
    case rfbSecTypeTight:
	/* We are lucky: the viewer supports TightVNC extensions. */
	rfbLog("Enabling TightVNC protocol extensions\n");
	/* Switch to the protocol 3.7t/3.8t. */
	cl->protocol_tightvnc = TRUE;
	/* Advertise our tunneling capabilities. */
	rfbSendTunnelingCaps(cl);
	break;
    default:
	/* Impossible case (hopefully). */
	rfbLog("rfbProcessClientSecurityType: "
	       "unknown authentication scheme\n");
	rfbCloseSock(cl->sock);
    }
}


/*
 * Send the list of our tunneling capabilities (protocol 3.7t/3.8t).
 */

static void
rfbSendTunnelingCaps(cl)
    rfbClientPtr cl;
{
    rfbTunnelingCapsMsg caps;
    CARD32 nTypes = 0;		/* we don't support tunneling yet */

    caps.nTunnelTypes = Swap32IfLE(nTypes);
    if (WriteExact(cl->sock, (char *)&caps, sz_rfbTunnelingCapsMsg) < 0) {
	rfbLogPerror("rfbSendTunnelingCaps: write");
	rfbCloseSock(cl->sock);
	return;
    }

    if (nTypes) {
	/* Dispatch client input to rfbProcessClientTunnelingType(). */
	cl->state = RFB_TUNNELING_TYPE;
    } else {
	rfbSendAuthCaps(cl);
    }
}


/*
 * Read tunneling type requested by the client (protocol 3.7t/3.8t).
 * NOTE: Currently, we don't support tunneling, and this function
 *       can never be called.
 */

void
rfbProcessClientTunnelingType(cl)
    rfbClientPtr cl;
{
    /* If we were called, then something's really wrong. */
    rfbLog("rfbProcessClientTunnelingType: not implemented\n");
    rfbCloseSock(cl->sock);
    return;
}


/*
 * Send the list of our authentication capabilities to the client
 * (protocol 3.7t/3.8t).
 */

static void
rfbSendAuthCaps(cl)
    rfbClientPtr cl;
{
    Bool authRequired;
    rfbAuthenticationCapsMsg caps;
    rfbCapabilityInfo caplist[MAX_AUTH_CAPS];
    int count = 0;

    authRequired = (rfbAuthPasswdFile != NULL && !cl->reverseConnection);

    if (authRequired) {
	if (rfbAuthPasswdFile != NULL) {
	    SetCapInfo(&caplist[count], rfbAuthVNC, rfbStandardVendor);
	    cl->authCaps[count++] = rfbAuthVNC;
	}
	if (count == 0) {
	    /* Should never happen. */
	    rfbLog("rfbSendAuthCaps: assertion failed\n");
	    rfbCloseSock(cl->sock);
	    return;
	}
    }

    cl->nAuthCaps = count;
    caps.nAuthTypes = Swap32IfLE((CARD32)count);
    if (WriteExact(cl->sock, (char *)&caps, sz_rfbAuthenticationCapsMsg) < 0) {
	rfbLogPerror("rfbSendAuthCaps: write");
	rfbCloseSock(cl->sock);
	return;
    }

    if (count) {
	if (WriteExact(cl->sock, (char *)&caplist[0],
		       count * sz_rfbCapabilityInfo) < 0) {
	    rfbLogPerror("rfbSendAuthCaps: write");
	    rfbCloseSock(cl->sock);
	    return;
	}
	/* Dispatch client input to rfbProcessClientAuthType. */
	cl->state = RFB_AUTH_TYPE;
    } else {
	/* No authentication needed. */
        rfbClientAuthSucceeded(cl, rfbAuthNone);
    }
}


/*
 * Read client's preferred authentication type (protocol 3.7t/3.8t).
 */

void
rfbProcessClientAuthType(cl)
    rfbClientPtr cl;
{
    CARD32 auth_type;
    int n, i;

    /* Read authentication type selected by the client. */
    n = ReadExact(cl->sock, (char *)&auth_type, sizeof(auth_type));
    if (n <= 0) {
	if (n == 0)
	    rfbLog("rfbProcessClientAuthType: client gone\n");
	else
	    rfbLogPerror("rfbProcessClientAuthType: read");
	rfbCloseSock(cl->sock);
	return;
    }
    auth_type = Swap32IfLE(auth_type);

    /* Make sure it was present in the list sent by the server. */
    for (i = 0; i < cl->nAuthCaps; i++) {
	if (auth_type == cl->authCaps[i])
	    break;
    }
    if (i >= cl->nAuthCaps) {
	rfbLog("rfbProcessClientAuthType: "
	       "wrong authentication type requested\n");
	rfbCloseSock(cl->sock);
	return;
    }

    switch (auth_type) {
    case rfbAuthNone:
	/* Dispatch client input to rfbProcessClientInitMessage. */
	cl->state = RFB_INITIALISATION;
	break;
    case rfbAuthVNC:
	rfbVncAuthSendChallenge(cl);
	break;
    default:
	rfbLog("rfbProcessClientAuthType: unknown authentication scheme\n");
	rfbCloseSock(cl->sock);
    }
}


/*
 * Send the authentication challenge.
 */

static void
rfbVncAuthSendChallenge(cl)
    rfbClientPtr cl;
{
    vncRandomBytes(cl->authChallenge);
    if (WriteExact(cl->sock, (char *)cl->authChallenge, CHALLENGESIZE) < 0) {
	rfbLogPerror("rfbVncAuthSendChallenge: write");
	rfbCloseSock(cl->sock);
	return;
    }

    /* Dispatch client input to rfbVncAuthProcessResponse. */
    cl->state = RFB_AUTHENTICATION;
}


/*
 * rfbVncAuthProcessResponse is called when the client sends its
 * authentication response.
 */

void
rfbVncAuthProcessResponse(cl)
    rfbClientPtr cl;
{
    char passwdFullControl[9];
    char passwdViewOnly[9];
    int numPasswords;
    Bool ok;
    int n;
    CARD8 encryptedChallenge1[CHALLENGESIZE];
    CARD8 encryptedChallenge2[CHALLENGESIZE];
    CARD8 response[CHALLENGESIZE];

    n = ReadExact(cl->sock, (char *)response, CHALLENGESIZE);
    if (n <= 0) {
	if (n != 0)
	    rfbLogPerror("rfbVncAuthProcessResponse: read");
	rfbCloseSock(cl->sock);
	return;
    }

    numPasswords = vncDecryptPasswdFromFile2(rfbAuthPasswdFile,
					     passwdFullControl,
					     passwdViewOnly);
    if (numPasswords == 0) {
	rfbLog("rfbVncAuthProcessResponse: could not get password from %s\n",
	       rfbAuthPasswdFile);

	rfbClientAuthFailed(cl, "The server is not configured properly");
	return;
    }

    memcpy(encryptedChallenge1, cl->authChallenge, CHALLENGESIZE);
    vncEncryptBytes(encryptedChallenge1, passwdFullControl);
    memcpy(encryptedChallenge2, cl->authChallenge, CHALLENGESIZE);
    vncEncryptBytes(encryptedChallenge2,
		    (numPasswords == 2) ? passwdViewOnly : passwdFullControl);

    /* Lose the passwords from memory */
    memset(passwdFullControl, 0, 9);
    memset(passwdViewOnly, 0, 9);

    ok = FALSE;
    if (memcmp(encryptedChallenge1, response, CHALLENGESIZE) == 0) {
	rfbLog("Full-control authentication passed by %s\n", cl->host);
	ok = TRUE;
	cl->viewOnly = FALSE;
    } else if (memcmp(encryptedChallenge2, response, CHALLENGESIZE) == 0) {
	rfbLog("View-only authentication passed by %s\n", cl->host);
	ok = TRUE;
	cl->viewOnly = TRUE;
    }

    if (ok) {
	rfbAuthUnblock();
	rfbClientAuthSucceeded(cl, rfbAuthVNC);
    } else {
	rfbLog("rfbVncAuthProcessResponse: authentication failed from %s\n",
	       cl->host);
	if (rfbAuthConsiderBlocking()) {
	    rfbClientAuthFailed(cl, "Authentication failed, too many tries");
	} else {
	    rfbClientAuthFailed(cl, "Authentication failed");
	}
    }
}


/*
 * rfbClientConnFailed is called when a client connection has failed
 * before the authentication stage.
 */

void
rfbClientConnFailed(cl, reason)
    rfbClientPtr cl;
    char *reason;
{
    int headerLen, reasonLen;
    char buf[8];

    headerLen = (cl->protocol_minor_ver >= 7) ? 1 : 4;
    reasonLen = strlen(reason);
    ((CARD32 *)buf)[0] = 0;
    ((CARD32 *)buf)[1] = Swap32IfLE(reasonLen);

    if ( WriteExact(cl->sock, buf, headerLen) < 0 ||
	 WriteExact(cl->sock, buf + 4, 4) < 0 ||
	 WriteExact(cl->sock, reason, reasonLen) < 0 ) {
	rfbLogPerror("rfbClientConnFailed: write");
    }

    rfbCloseSock(cl->sock);
}


/*
 * rfbClientAuthFailed is called on authentication failure. Sending a
 * reason string is defined in the RFB protocol 3.8 and above.
 */

void
rfbClientAuthFailed(cl, reason)
    rfbClientPtr cl;
    char *reason;
{
    int reasonLen;
    char buf[8];

    if (cl->protocol_minor_ver < 8)
	reason = NULL;		/* invalidate the pointer */

    reasonLen = (reason == NULL) ? 0 : strlen(reason);
    ((CARD32 *)buf)[0] = Swap32IfLE(rfbAuthFailed);
    ((CARD32 *)buf)[1] = Swap32IfLE(reasonLen);

    if (reasonLen == 0) {
	if (WriteExact(cl->sock, buf, 4) < 0) {
	    rfbLogPerror("rfbClientAuthFailed: write");
	}
    } else {
	if ( WriteExact(cl->sock, buf, 8) < 0 ||
	     WriteExact(cl->sock, reason, reasonLen) < 0 ) {
	    rfbLogPerror("rfbClientAuthFailed: write");
	}
    }

    rfbCloseSock(cl->sock);
}


/*
 * rfbClientAuthSucceeded is called on successful authentication.
 * It just sends rfbAuthOK and dispatches client input to
 * rfbProcessClientInitMessage(). However, rfbAuthOK message is
 * not sent if authentication was not required and the protocol
 * version is 3.7 or lower.
 */

void
rfbClientAuthSucceeded(cl, authType)
    rfbClientPtr cl;
    CARD32 authType;
{
    CARD32 authResult;

    if (cl->protocol_minor_ver >= 8 || authType != rfbAuthNone) {
        authResult = Swap32IfLE(rfbAuthOK);
        if (WriteExact(cl->sock, (char *)&authResult, 4) < 0) {
            rfbLogPerror("rfbClientAuthSucceeded: write");
            rfbCloseSock(cl->sock);
            return;
        }
    }

    /* Dispatch client input to rfbProcessClientInitMessage(). */
    cl->state = RFB_INITIALISATION;
}


/*********************************************************************
 * Functions to prevent too many successive authentication failures.
 * FIXME: This should be performed separately per each client IP.
 */

/* Maximum authentication failures before blocking connections */
#define MAX_AUTH_TRIES 5

/* Delay in ms, doubles for each failure over MAX_AUTH_TRIES */
#define AUTH_TOO_MANY_BASE_DELAY 10 * 1000

static int rfbAuthTries = 0;
static Bool rfbAuthTooManyTries = FALSE;
static OsTimerPtr timer = NULL;

/*
 * This function should not be called directly, it is called by
 * setting a timer in rfbAuthConsiderBlocking().
 */

static CARD32
rfbAuthReenable(OsTimerPtr timer, CARD32 now, pointer arg)
{
    rfbAuthTooManyTries = FALSE;
    return 0;
}

/*
 * This function should be called after each authentication failure.
 * The return value will be true if there was too many failures.
 */

Bool
rfbAuthConsiderBlocking(void)
{
    int i;

    rfbAuthTries++;

    if (rfbAuthTries >= MAX_AUTH_TRIES) {
	CARD32 delay = AUTH_TOO_MANY_BASE_DELAY;
	for (i = MAX_AUTH_TRIES; i < rfbAuthTries; i++)
	    delay *= 2;
	timer = TimerSet(timer, 0, delay, rfbAuthReenable, NULL);
	rfbAuthTooManyTries = TRUE;
	return TRUE;
    }

    return FALSE;
}

/*
 * This function should be called after successful authentication.
 * It resets the counter of authentication failures. Note that it's
 * not necessary to clear the rfbAuthTooManyTries flag as it will be
 * reset by the timer function.
 */

void
rfbAuthUnblock(void)
{
    rfbAuthTries = 0;
}

/*
 * This function should be called before authentication process.
 * The return value will be true if there was too many authentication
 * failures, and the server should not allow another try.
 */

Bool
rfbAuthIsBlocked(void)
{
    return rfbAuthTooManyTries;
}

