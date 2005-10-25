/*
 * loginauth.c - deal with login-style Unix authentication.
 *
 * This file implements the UnixLogin authentication protocol when setting up
 * an RFB connection.
 */

/*
 *  Copyright (C) 2003 Constantin Kaplinsky.  All Rights Reserved.
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
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include "rfb.h"

#ifdef linux
#include <shadow.h>
#endif

Bool loginAuthEnabled = FALSE;

void rfbLoginAuthProcessClientMessage(rfbClientPtr cl)
{
    int n1 = 0, n2 = 0;
    CARD32 loginLen, passwdLen, authResult;
    char *loginBuf, *passwdBuf;
    struct passwd *ps;
    char *encPasswd1, *encPasswd2;
    Bool ok;

    if ((n1 = ReadExact(cl->sock, (char *)&loginLen,
			sizeof(loginLen))) <= 0 ||
	(n2 = ReadExact(cl->sock, (char *)&passwdLen,
			sizeof(passwdLen))) <= 0) {
	if (n1 != 0 || n2 != 0)
	    rfbLogPerror("rfbLoginAuthProcessClientMessage: read");
	rfbCloseSock(cl->sock);
	return;
    }
    loginLen = Swap32IfLE(loginLen);
    passwdLen = Swap32IfLE(passwdLen);
    loginBuf = (char *)xalloc(loginLen + 1);
    passwdBuf = (char *)xalloc(passwdLen + 1);

    n1 = n2 = 0;
    if ((n1 = ReadExact(cl->sock, loginBuf, loginLen)) <= 0 ||
	(n2 = ReadExact(cl->sock, passwdBuf, passwdLen)) <= 0) {
	if (n1 != 0 || n2 != 0)
	    rfbLogPerror("rfbLoginAuthProcessClientMessage: read");
	rfbCloseSock(cl->sock);
	return;
    }
    loginBuf[loginLen] = '\0';
    passwdBuf[passwdLen] = '\0';

    encPasswd1 = encPasswd2 = NULL;

    ps = getpwnam(loginBuf);
    if (ps == NULL) {
	rfbLog("rfbLoginAuthProcessClientMessage: "
	       "Cannot get password file entry for \"%s\"\n", loginBuf);
    } else {
	encPasswd1 = ps->pw_passwd;
#ifdef linux
	if (strlen(ps->pw_passwd) == 1) {
	    struct spwd *sps;

	    sps = getspnam(loginBuf);
	    if (sps == NULL) {
		rfbLog("rfbLoginAuthProcessClientMessage:"
		       " getspnam() failed for user \"%s\"\n", loginBuf);
	    } else {
		encPasswd1 = sps->sp_pwdp;
	    }
	}
#endif
	encPasswd2 = crypt(passwdBuf, encPasswd1);
	memset(passwdBuf, 0, strlen(passwdBuf));
    }

    ok = FALSE;
    if (encPasswd1 != NULL && encPasswd2 != NULL) {
	if (strcmp(encPasswd1, encPasswd2) == 0)
	    ok = TRUE;
    }

    if (!ok) {
	rfbLog("rfbAuthProcessClientMessage: authentication failed from %s\n",
	       cl->host);

	if (rfbAuthConsiderBlocking()) {
	    authResult = Swap32IfLE(rfbVncAuthTooMany);
	} else {
	    authResult = Swap32IfLE(rfbVncAuthFailed);
	}

	if (WriteExact(cl->sock, (char *)&authResult,
		       sizeof(authResult)) < 0) {
	    rfbLogPerror("rfbLoginAuthProcessClientMessage: write");
	}
	rfbCloseSock(cl->sock);
	return;
    }

    rfbAuthUnblock();

    cl->login = strdup(loginBuf);
    rfbLog("Login-style authentication passed for user %s at %s\n",
	   cl->login, cl->host);

    authResult = Swap32IfLE(rfbVncAuthOK);

    if (WriteExact(cl->sock, (char *)&authResult, sizeof(authResult)) < 0) {
	rfbLogPerror("rfbLoginAuthProcessClientMessage: write");
	rfbCloseSock(cl->sock);
	return;
    }

    /* Dispatch client input to rfbProcessClientInitMessage(). */
    cl->state = RFB_INITIALISATION;
}

