/*
 * auth.c - deal with authentication.
 *
 * This file implements authentication when setting up an RFB connection.
 */

/*
 *  Copyright (C) 2010 D. R. Commander.  All Rights Reserved.
 *  Copyright (C) 2010 University Corporation for Atmospheric Research.
 *                     All Rights Reserved.
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
#include <sys/stat.h>
#include <errno.h>
#include "windowstr.h"
#include "rfb.h"


char *rfbAuthPasswdFile = NULL;

static void rfbSendSecurityType(rfbClientPtr cl, int securityType);
static void rfbSendSecurityTypeList(rfbClientPtr cl);
static void rfbSendTunnelingCaps(rfbClientPtr cl);
static void rfbSendAuthCaps(rfbClientPtr cl);

static void rfbVncAuthSendChallenge(rfbClientPtr cl);

#define AUTH_DEFAULT_CONF_FILE        "/etc/turbovncserver.auth"
#define AUTH_DEFAULT_AUTH_METHODS     "vncauth"
#ifdef XVNC_AuthPAM
#define AUTH_DEFAULT_PAM_SERVICE_NAME "turbovnc"
#endif

#define MAX_USER_LEN 64
#define MAX_PWD_LEN  64

char* rfbAuthConfigFile = AUTH_DEFAULT_CONF_FILE;
Bool  rfbAuthDisableRevCon;
Bool  rfbAuthOTP;
char* rfbAuthOTPValue;
int   rfbAuthOTPValueLen;

static void
AuthNoneStartFunc(rfbClientPtr cl)
{
    cl->state = RFB_INITIALISATION;
}

static void
AuthNoneRspFunc(rfbClientPtr cl)
{
}

#ifdef XVNC_AuthPAM

#include <pwd.h>

Bool rfbPAMAuthenticate(const char* svc, const char* host, const char* user,
                        const char* pwd, const char** emsg);

static char* pamServiceName = AUTH_DEFAULT_PAM_SERVICE_NAME;
typedef struct UserList {
    struct UserList* next;
    const char*      name;
    Bool             viewOnly;
} UserList;

static UserList* userACL;
Bool rfbAuthUserACL;

void
rfbAuthAddUser(const char* name, Bool viewOnly)
{
    UserList* p = (UserList*) xalloc(sizeof(UserList));

    rfbLog("rfbAuthAddUser: '%s'%s\n", name, viewOnly ? " view-only" : "");
    p->next = userACL;
    p->name = name;
    p->viewOnly = viewOnly;
    userACL = p;
}

void
rfbAuthRevokeUser(const char* name)
{
    UserList** prev = &userACL;
    UserList*  p;

    rfbLog("rfbAuthRevokeUser: '%s'\n", name);
    while (*prev != NULL) {
        p = *prev;
        if (!strcmp(p->name, name)) {
            *prev = p->next;
            xfree(p->name);
            xfree(p);
            return;
        }

        prev = &p->next;
    }
}

static void
AuthPAMUserPwdStartFunc(rfbClientPtr cl)
{       
    cl->state = RFB_AUTHENTICATION;
}

static void
AuthPAMUserPwdRspFunc(rfbClientPtr cl)
{       
    CARD32      userLen;
    CARD32      pwdLen;
    char        userBuf[MAX_USER_LEN + 1];
    char        pwdBuf[MAX_PWD_LEN + 1];
    int         n;
    const char* emsg;

    n = ReadExact(cl->sock, (char*) &userLen, sizeof(userLen));
    if (n <= 0) {
        if (n != 0)
            rfbLogPerror("AuthPAMUserPwdRspFunc: read");

        rfbCloseSock(cl->sock);
        return;
    }

    userLen = Swap32IfLE(userLen);
    n = ReadExact(cl->sock, (char*) &pwdLen, sizeof(pwdLen));
    if (n <= 0) {
        if (n != 0)
            rfbLogPerror("AuthPAMUserPwdRspFunc: read");

        rfbCloseSock(cl->sock);
        return;
    }

    pwdLen = Swap32IfLE(pwdLen);
    if ((userLen > MAX_USER_LEN) || (pwdLen > MAX_PWD_LEN)) {
        rfbLogPerror("AuthPAMUserPwdRspFunc: excessively large user or password in response");
        rfbCloseSock(cl->sock);
        return;
    }

    n = ReadExact(cl->sock, userBuf, userLen);
    if (n <= 0) {
        if (n != 0)
            rfbLogPerror("AuthPAMUserPwdRspFunc: user read");

        rfbCloseSock(cl->sock);
        return;
    }

    userBuf[userLen] = '\0';
    n = ReadExact(cl->sock, pwdBuf, pwdLen);
    if (n <= 0) {
        if (n != 0)
            rfbLogPerror("AuthPAMUserPwdRspFunc: pwd read");

        rfbCloseSock(cl->sock);
        return;
    }

    pwdBuf[pwdLen] = '\0';
    if (rfbAuthUserACL) {
        UserList* p = userACL;

        if (p == NULL)
            rfbLog("AuthPAMUserPwdRspFunc: WARNING: user ACL empty\n");

        while (p != NULL) {
            if (!strcmp(p->name, userBuf))
                break;

            p = p->next;
        }

        if (p == NULL) {
            rfbLog("AuthPAMUserPwdRspFunc: user '%s' denied access", userBuf);
            rfbClientAuthFailed(cl, "User denied access");
            return;
        }

        cl->viewOnly = p->viewOnly;
    }

    if (rfbPAMAuthenticate(pamServiceName, cl->host, userBuf, pwdBuf, &emsg)) {
        rfbClientAuthSucceeded(cl, rfbAuthUnixLogin);
    
    } else {
        rfbClientAuthFailed(cl, (char*)emsg);
    }
}
#endif

#define AD_NONE     0x01
#define AD_PWD      0x02
#define AD_USER_PWD 0x04
#define AD_OTP      0x08

typedef void (*AuthFunc)(rfbClientPtr cl);

typedef struct {
    const char* name;
    int         protocolMinorVer;
    Bool        enabled;
    int         requiredData;
    CARD8       securityType;
    int         authType;
    CARD8       vendorSignature[4];
    CARD8       nameSignature[8];
    AuthFunc    startFunc;
    AuthFunc    rspFunc;
} AuthMethodData;

/*
 * NOTE: Keep this list ordered by decreasing security.
 */
static AuthMethodData authMethods[] = {
#ifdef XVNC_AuthPAM
    { "pam-userpwd", 7, FALSE, AD_USER_PWD,
        rfbSecTypeTight, rfbAuthUnixLogin, rfbTightVncVendor, sig_rfbAuthUnixLogin,
        AuthPAMUserPwdStartFunc, AuthPAMUserPwdRspFunc},
#endif

    { "otp", 3, FALSE, AD_OTP,
        rfbSecTypeVncAuth, rfbAuthVNC, rfbStandardVendor, sig_rfbAuthVNC,
        rfbVncAuthSendChallenge, rfbVncAuthProcessResponse},

    { "vncauth", 3, FALSE, AD_PWD,
        rfbSecTypeVncAuth, rfbAuthVNC, rfbStandardVendor, sig_rfbAuthVNC,
        rfbVncAuthSendChallenge, rfbVncAuthProcessResponse},

    { "none", 3, FALSE, AD_NONE,
        rfbSecTypeNone, rfbAuthNone, rfbStandardVendor, sig_rfbAuthNone,
        AuthNoneStartFunc, AuthNoneRspFunc},

    { NULL }
};

static void
setMethods(char* buf)
{
    char*           saveptr;
    char*           p;
    AuthMethodData* a;

    for (a = authMethods; a->name != NULL; a++)
        a->enabled = FALSE;

    while (TRUE) {
        p = strtok_r(buf, ",", &saveptr);
        buf = NULL;
        if (p == NULL)
            break;

        for (a = authMethods; a->name != NULL; a++) {
            if (!strcmp(a->name, p))
                break;
        }

        if (a->name == NULL) {
            FatalError("rfbAuthInit unknown auth method name '%s'\n", p);
        }

        a->enabled = TRUE;
        if (a->requiredData & AD_OTP)
            rfbAuthOTP = TRUE;
    }
}

void
rfbAuthInit()
{
    FILE*           fp;
    char            buf[256];
    int             line;
    int             len;
    int             n;
    struct stat     sb;
    AuthMethodData* a;
    Bool            enabled = FALSE;

    setMethods(AUTH_DEFAULT_AUTH_METHODS);
    if ((fp = fopen(rfbAuthConfigFile, "r")) == NULL)
        return;

    if (fstat(fileno(fp), &sb) == -1) {
        FatalError("rfbAuthInit: ERROR: fstat %s: %s", rfbAuthConfigFile, strerror(errno));
    }

    if ((sb.st_uid != 0) && (sb.st_uid != getuid())) {
        FatalError("rfbAuthInit: ERROR: %s is not owned either by you or root\n", rfbAuthConfigFile);
    }

    if (sb.st_mode & (S_IWGRP | S_IWOTH)) {
        FatalError("rfbAuthInit: ERROR: %s is insecure\n", rfbAuthConfigFile);
    }

    rfbLog("rfbAuthInit: using configuration file %s\n", rfbAuthConfigFile);
    for (line = 0; fgets(buf, sizeof(buf), fp) != NULL; line++) {
        len = strlen(buf) - 1;
        if (buf[len] != '\n') {
            FatalError("rfbAuthInit: ERROR: Configuration file %s line %d too long!\n", rfbAuthConfigFile, line);
        }
        
        buf[len] = '\0';

        n = 21;
        if (!strncmp(buf, "no-reverse-connection", n)) {
            rfbAuthDisableRevCon = TRUE;
            continue;
        }

#ifdef XVNC_AuthPAM
        n = 15;
        if (!strncmp(buf, "enable-user-acl", n)) {
            rfbAuthUserACL = TRUE;
            continue;
        }

        n = 17;
        if (!strncmp(buf, "pam-service-name=", n)) {
            if (buf[n] == '\0') {
                FatalError("rfbAuthInit: ERROR: pam-service-name empty!");
            }

            if ((pamServiceName = strdup(&buf[n])) == NULL) {
                FatalError("rfbAuthInit strdup: %s", strerror(errno));
            }

            continue;
        }
#endif

        n = 23;
        if (!strncmp(buf, "permitted-auth-methods=", n)) {
            if (buf[n] == '\0') {
                FatalError("rfbAuthInit: ERROR: permitted-auth-methods empty!");
            }

            setMethods(&buf[n]);
            continue;
        }

        rfbLog("rfbAuthInit: WARNING: unrecognized auth config line '%s'\n", buf);
    }

    fclose(fp);
    for (a = authMethods; a->name != NULL; a++) {
        if (!a->enabled)
            continue;

        enabled = TRUE;
        rfbLog("rfbAuthInit: enabled method %s\n", a->name);
    }

    if (!enabled) {
        FatalError("rfbAuthInit: ERROR: no auth methods enabled!\n");
    }

    if (rfbAuthOTP && (rfbAuthPasswdFile != NULL)) {
        rfbLog("rfbAuthInit: WARNING: -rfbauth overridden to -otp by auth config file\n");
        rfbAuthPasswdFile = NULL;
    }

#ifdef XVNC_AuthPAM
    if (rfbAuthUserACL) {
        struct passwd  pbuf;
        struct passwd* pw;
        char           buf[256];
        char*          n;

        if (getpwuid_r(getuid(), &pbuf, buf, sizeof(buf), &pw) != 0) {
            FatalError("AuthPAMUserPwdRspFunc: limit-user enabled and getpwuid_r failed: %s",
                strerror(errno));
        }

        n = (char*) xalloc(strlen(pbuf.pw_name));
        strcpy(n, pbuf.pw_name);
        rfbAuthAddUser(n, FALSE);
    }
#endif
}

void
rfbAuthProcessResponse(rfbClientPtr cl)
{
    AuthMethodData* a;

    for (a = authMethods; a->name != NULL; a++) {
        if (cl->selectedAuthType == a->authType) {
            a->rspFunc(cl);
            return;
        }
    }

    rfbLog("rfbAuthProcessResponse: authType assertion failed");
    rfbCloseSock(cl->sock);
}


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
    AuthMethodData* a;

    if (rfbAuthIsBlocked()) {
        rfbLog("Too many authentication failures - client rejected\n");
        rfbClientConnFailed(cl, "Too many authentication failures");
        return;
    }

    if (cl->protocol_minor_ver >= 7) {
        rfbSendSecurityTypeList(cl);
        return;
    }

    /* Make sure we use only RFB 3.3 compatible security types,
       but we'll pick the first (most secure) one. */
    for (a = authMethods; a->name != NULL; a++) {
        if (a->enabled && (a->protocolMinorVer < 7))
            break;
    }

    if (a->name == NULL) {
        rfbLog("VNC authentication disabled - RFB 3.3 client rejected\n");
        rfbClientConnFailed(cl, "Your viewer cannot handle required authentication methods");
        return;
    }

    cl->selectedAuthType = a->securityType;
    rfbSendSecurityType(cl, a->securityType);
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
 */

static void
rfbSendSecurityTypeList(rfbClientPtr cl)
{
    int             n;
    AuthMethodData* a;
    Bool            tightAdvertised = FALSE;

    n = 0;
    for (a = authMethods; a->name != NULL; a++) {
        if (n > MAX_SECURITY_TYPES) {
            FatalError("rfbSendSecurityTypeList: # enabled security types > MAX_SECURITY_TYPES\n");
        }

        if (a->enabled && (cl->protocol_minor_ver >= a->protocolMinorVer)) {
            cl->securityTypes[++n] = a->securityType;
            if (a->securityType == rfbSecTypeTight)
                tightAdvertised = TRUE;
        }
    }

    if (!tightAdvertised) {
        /*
         * Make sure to advertise the Tight security type to allow the compatible
         * client to enable other the other non-auth Tight extensions.
         */
        if (n > MAX_SECURITY_TYPES) {
            FatalError("rfbSendSecurityTypeList: # enabled security types > MAX_SECURITY_TYPES\n");
        }

        cl->securityTypes[++n] = rfbSecTypeTight;
    }

    cl->securityTypes[0] = (CARD8)n;

    /* Send the list. */
    if (WriteExact(cl->sock, (char *)cl->securityTypes, n + 1) < 0) {
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

    cl->selectedAuthType = chosenType;
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

    AuthMethodData*    a;
    rfbCapabilityInfo* pcap;

    if (!cl->reverseConnection) {
        authRequired = TRUE;
        for (a = authMethods; a->name != NULL; a++) {
            if (count >= MAX_AUTH_CAPS) {
                FatalError("rfbSendAuthCaps: # enabled security types > MAX_AUTH_CAPS\n");
            }

            if (!a->enabled)
                continue;
                
            if (a->requiredData == AD_NONE) {
                authRequired = FALSE;
                break;
            }

            if ((a->requiredData & AD_OTP) && (rfbAuthOTPValue == NULL))
                continue;

            if ((a->requiredData & AD_PWD) && (rfbAuthPasswdFile == NULL))
                continue;

            pcap = &caplist[count];
            pcap->code = Swap32IfLE(a->authType);
            memcpy(pcap->vendorSignature, a->vendorSignature, sz_rfbCapabilityInfoVendor);
            memcpy(pcap->nameSignature, a->nameSignature, sz_rfbCapabilityInfoName);
            cl->authCaps[count] = a->authType;
            count++;
        }

        if (authRequired && (count == 0)) {
            if (rfbAuthOTP) {
                rfbLog("rfbSendAuthCaps: OTP authentication required but no OTP set.\n");

            } else {
                rfbLog("rfbSendAuthCaps: authentication required but no auth methods enabled.\n");
                rfbLog("    you may need to provide one of the -rfbauth or -otp command line options.\n");
            }

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
    AuthMethodData* a;

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

    for (a = authMethods; a->name != NULL; a++) {
        if (auth_type == a->authType) {
            cl->selectedAuthType = auth_type;
            a->startFunc(cl);
            return;
        }
    }

    rfbLog("rfbProcessClientAuthType: unknown authentication scheme\n");
    rfbCloseSock(cl->sock);
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
    char passwdFullControl[MAXPWLEN + 1];
    char passwdViewOnly[MAXPWLEN + 1];
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

    if (rfbAuthOTP) {
        if (rfbAuthOTPValue == NULL) {
            rfbClientAuthFailed(cl, "The one time password has not been set on the server");
            return;
        }

        memcpy(passwdFullControl, rfbAuthOTPValue, MAXPWLEN);
        passwdFullControl[MAXPWLEN] = '\0';
        numPasswords = rfbAuthOTPValueLen / MAXPWLEN;
        if (numPasswords > 1) {
            memcpy(passwdViewOnly, rfbAuthOTPValue + MAXPWLEN, MAXPWLEN);
            passwdViewOnly[MAXPWLEN] = '\0';
        }

        xfree(rfbAuthOTPValue);
        rfbAuthOTPValue = NULL;

    } else {
        numPasswords = vncDecryptPasswdFromFile2(rfbAuthPasswdFile,
                                                 passwdFullControl,
                                                 passwdViewOnly);
        if (numPasswords == 0) {
            rfbLog("rfbVncAuthProcessResponse: could not get password from %s\n",
                   rfbAuthPasswdFile);

            rfbClientAuthFailed(cl, "The server is not configured properly");
            return;
        }
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

