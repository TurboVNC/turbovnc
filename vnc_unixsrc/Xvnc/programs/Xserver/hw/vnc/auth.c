/*
 * auth.c - deal with authentication.
 *
 * This file implements the VNC authentication protocol when setting up an RFB
 * connection.
 */

/*
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


#define MAX_AUTH_TRIES 5
#define AUTH_TOO_MANY_BASE_DELAY 10 * 1000 /* in ms, doubles for each failure
					      over MAX_AUTH_TRIES */

static CARD32 rfbAuthReenable(OsTimerPtr timer, CARD32 now, pointer arg);

char *rfbAuthPasswdFile = NULL;
int rfbAuthTries = 0;
Bool rfbAuthTooManyTries = FALSE;
static OsTimerPtr timer = NULL;


/*
 * rfbAuthNewClient is called when we reach the point of authenticating
 * a new client.  If authentication isn't being used then we simply send
 * rfbNoAuth.  Otherwise we send rfbVncAuth plus the challenge.
 */

void
rfbAuthNewClient(cl)
    rfbClientPtr cl;
{
    char buf[4 + CHALLENGESIZE];
    int len;

    cl->state = RFB_AUTHENTICATION;

    if (rfbAuthPasswdFile && !cl->reverseConnection) {

	if (rfbAuthTooManyTries) {
	    rfbClientConnFailed(cl, "Too many authentication failures");
	    return;
	}

	*(CARD32 *)buf = Swap32IfLE(rfbVncAuth);
	vncRandomBytes(cl->authChallenge);
	memcpy(&buf[4], (char *)cl->authChallenge, CHALLENGESIZE);
	len = 4 + CHALLENGESIZE;

    } else {

	*(CARD32 *)buf = Swap32IfLE(rfbNoAuth);
	len = 4;
	cl->state = RFB_INITIALISATION;
    }

    if (WriteExact(cl->sock, buf, len) < 0) {
	rfbLogPerror("rfbAuthNewClient: write");
	rfbCloseSock(cl->sock);
	return;
    }
}


/*
 * rfbAuthProcessClientMessage is called when the client sends its
 * authentication response.
 */

void
rfbAuthProcessClientMessage(cl)
    rfbClientPtr cl;
{
    char passwdFullControl[9];
    char passwdViewOnly[9];
    int numPasswords;
    Bool ok;
    int i, n;
    CARD8 encryptedChallenge1[CHALLENGESIZE];
    CARD8 encryptedChallenge2[CHALLENGESIZE];
    CARD8 response[CHALLENGESIZE];
    CARD32 authResult;

    if ((n = ReadExact(cl->sock, (char *)response, CHALLENGESIZE)) <= 0) {
	if (n != 0)
	    rfbLogPerror("rfbAuthProcessClientMessage: read");
	rfbCloseSock(cl->sock);
	return;
    }

    numPasswords = vncDecryptPasswdFromFile2(rfbAuthPasswdFile,
					     passwdFullControl,
					     passwdViewOnly);
    if (numPasswords == 0) {
	rfbLog("rfbAuthProcessClientMessage: could not get password from %s\n",
	       rfbAuthPasswdFile);

	authResult = Swap32IfLE(rfbVncAuthFailed);

	if (WriteExact(cl->sock, (char *)&authResult, 4) < 0) {
	    rfbLogPerror("rfbAuthProcessClientMessage: write");
	}
	rfbCloseSock(cl->sock);
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

    if (!ok) {
	rfbLog("rfbAuthProcessClientMessage: authentication failed from %s\n",
	       cl->host);

	rfbAuthTries++;

	if (rfbAuthTries >= MAX_AUTH_TRIES) {

	    CARD32 delay = AUTH_TOO_MANY_BASE_DELAY;
	    for (i = MAX_AUTH_TRIES; i < rfbAuthTries; i++)
		delay *= 2;
	    timer = TimerSet(timer, 0, delay, rfbAuthReenable, NULL);

	    rfbAuthTooManyTries = TRUE;
	    authResult = Swap32IfLE(rfbVncAuthTooMany);

	} else {
	    authResult = Swap32IfLE(rfbVncAuthFailed);
	}

	if (WriteExact(cl->sock, (char *)&authResult, 4) < 0) {
	    rfbLogPerror("rfbAuthProcessClientMessage: write");
	}
	rfbCloseSock(cl->sock);
	return;
    }

    rfbAuthTries = 0;

    authResult = Swap32IfLE(rfbVncAuthOK);

    if (WriteExact(cl->sock, (char *)&authResult, 4) < 0) {
	rfbLogPerror("rfbAuthProcessClientMessage: write");
	rfbCloseSock(cl->sock);
	return;
    }

    cl->state = RFB_INITIALISATION;
}

static CARD32
rfbAuthReenable(OsTimerPtr timer, CARD32 now, pointer arg)
{
    rfbAuthTooManyTries = FALSE;
    return 0;
}
