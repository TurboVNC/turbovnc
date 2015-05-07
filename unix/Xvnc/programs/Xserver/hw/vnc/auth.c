/*
 * auth.c - deal with authentication.
 *
 * This file implements authentication when setting up an RFB connection.
 */

/*
 *  Copyright (C) 2010, 2012-2015 D. R. Commander.  All Rights Reserved.
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 *  USA.
 */

#ifndef _POSIX_PTHREAD_SEMANTICS
#define _POSIX_PTHREAD_SEMANTICS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "rfb.h"
#include "windowstr.h"


char *rfbAuthPasswdFile = NULL;

static void rfbSendSecurityType(rfbClientPtr cl, int securityType);
static void rfbSendSecurityTypeList(rfbClientPtr cl);
static void rfbSendTunnelingCaps(rfbClientPtr cl);
static void rfbSendAuthCaps(rfbClientPtr cl);

static void rfbVncAuthSendChallenge(rfbClientPtr cl);

#define AUTH_DEFAULT_CONF_FILE "/etc/turbovncserver-security.conf"
#ifdef XVNC_AuthPAM
#define AUTH_DEFAULT_PAM_SERVICE_NAME "turbovnc"
#endif

#define MAX_USER_LEN 64
#define MAX_PWD_LEN 64

char *rfbAuthConfigFile = AUTH_DEFAULT_CONF_FILE;
Bool rfbAuthDisableRevCon = FALSE;
Bool rfbAuthDisableCBSend = FALSE;
Bool rfbAuthDisableCBRecv = FALSE;
Bool rfbAuthDisableHTTP   = FALSE;
Bool rfbAuthDisableX11TCP = FALSE;

static int nAuthMethodsEnabled = 0;
static int preferenceLimit = 1; /* Force one iteration of the loop in
                                   rfbSendAuthCaps() */

Bool rfbOptNoauth = FALSE;
Bool rfbOptOtpauth = FALSE;
Bool rfbOptPamauth = FALSE;
Bool rfbOptRfbauth = FALSE;

char *rfbAuthOTPValue = NULL;
int rfbAuthOTPValueLen = 0;


static void
AuthNoneStartFunc(rfbClientPtr cl)
{
    rfbClientAuthSucceeded(cl, rfbAuthNone);
}

static void
AuthNoneRspFunc(rfbClientPtr cl)
{
}


#ifdef XVNC_AuthPAM

#include <pwd.h>

Bool rfbPAMAuthenticate(const char *svc, const char *host, const char *user,
                        const char *pwd, const char **emsg);

static char *pamServiceName = AUTH_DEFAULT_PAM_SERVICE_NAME;

typedef struct UserList {
    struct UserList *next;
    const char *name;
    Bool viewOnly;
} UserList;

static UserList *userACL = NULL;
Bool rfbAuthUserACL = FALSE;


void
rfbAuthAddUser(const char *name, Bool viewOnly)
{
    UserList *p = (UserList *)malloc(sizeof(UserList));

    if (p == NULL) {
        FatalError("rfbAuthAddUser: out of memory");
    }

    rfbLog("Adding user '%s' to ACL with %s privileges\n", name,
           viewOnly ? " view-only" : "full control");
    p->next = userACL;
    p->name = name;
    p->viewOnly = viewOnly;
    userACL = p;
}


void
rfbAuthRevokeUser(const char *name)
{
    UserList **prev = &userACL;
    UserList *p;

    rfbLog("Removing user '%s' from ACL\n", name);
    while (*prev != NULL) {
        p = *prev;
        if (!strcmp(p->name, name)) {
            *prev = p->next;
            free((void *)p->name);
            free(p);
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
    CARD32 userLen;
    CARD32 pwdLen;
    char userBuf[MAX_USER_LEN + 1];
    char pwdBuf[MAX_PWD_LEN + 1];
    int n;
    const char *emsg;

    n = ReadExact(cl, (char *)&userLen, sizeof(userLen));
    if (n <= 0) {
        if (n != 0)
            rfbLogPerror("AuthPAMUserPwdRspFunc: read error");
        rfbCloseClient(cl);
        return;
    }

    userLen = Swap32IfLE(userLen);
    n = ReadExact(cl, (char *)&pwdLen, sizeof(pwdLen));
    if (n <= 0) {
        if (n != 0)
            rfbLogPerror("AuthPAMUserPwdRspFunc: read error");
        rfbCloseClient(cl);
        return;
    }

    pwdLen = Swap32IfLE(pwdLen);
    if ((userLen > MAX_USER_LEN) || (pwdLen > MAX_PWD_LEN)) {
        rfbLogPerror("AuthPAMUserPwdRspFunc: excessively large user name or password in response");
        rfbCloseClient(cl);
        return;
    }

    n = ReadExact(cl, userBuf, userLen);
    if (n <= 0) {
        if (n != 0)
            rfbLogPerror("AuthPAMUserPwdRspFunc: error reading user name");
        rfbCloseClient(cl);
        return;
    }

    userBuf[userLen] = '\0';
    n = ReadExact(cl, pwdBuf, pwdLen);
    if (n <= 0) {
        if (n != 0)
            rfbLogPerror("AuthPAMUserPwdRspFunc: error reading password");
        rfbCloseClient(cl);
        return;
    }

    pwdBuf[pwdLen] = '\0';
    if (rfbAuthUserACL) {
        UserList *p = userACL;

        if (p == NULL)
            rfbLog("WARNING: User ACL is empty.  No users will be allowed to log in with Unix login authentication.\n");

        while (p != NULL) {
            if (!strcmp(p->name, userBuf))
                break;
            p = p->next;
        }

        if (p == NULL) {
            rfbLog("User '%s' is not in the ACL and has been denied access\n",
                   userBuf);
            rfbClientAuthFailed(cl, "User denied access");
            return;
        }

        cl->viewOnly = p->viewOnly;
    }

    if (rfbPAMAuthenticate(pamServiceName, cl->host, userBuf, pwdBuf, &emsg)) {
        rfbClientAuthSucceeded(cl, rfbAuthUnixLogin);
    } else {
        rfbClientAuthFailed(cl, (char *)emsg);
    }
}

#endif


typedef struct {
    const char *name;
    int protocolMinorVer;
    Bool advertise;
    CARD8 securityType;
} SecTypeData;

static SecTypeData secTypeNone    = { "none",    3, TRUE, rfbSecTypeNone };
static SecTypeData secTypeVncauth = { "vncauth", 3, TRUE, rfbSecTypeVncAuth };
static SecTypeData secTypeTight   = { "tight",   7, TRUE, rfbSecTypeTight };

static SecTypeData *secTypes[] = {
    &secTypeNone, &secTypeVncauth, &secTypeTight, NULL
};

typedef void (*AuthFunc)(rfbClientPtr cl);

typedef struct {
    int authType;
    CARD8 vendorSignature[4];
    CARD8 nameSignature[8];
    AuthFunc startFunc;
    AuthFunc rspFunc;
} AuthCapData;

static AuthCapData authCapNone =
    { rfbAuthNone, rfbStandardVendor, sig_rfbAuthNone,
      AuthNoneStartFunc, AuthNoneRspFunc };

static AuthCapData authCapVncauth =
    { rfbAuthVNC, rfbStandardVendor, sig_rfbAuthVNC,
      rfbVncAuthSendChallenge, rfbVncAuthProcessResponse };

#ifdef XVNC_AuthPAM
static AuthCapData authCapUnixLogin =
    { rfbAuthUnixLogin, rfbTightVncVendor, sig_rfbAuthUnixLogin,
      AuthPAMUserPwdStartFunc, AuthPAMUserPwdRspFunc };
#endif

static AuthCapData *authCaps[] = {
    &authCapNone, &authCapVncauth,
#ifdef XVNC_AuthPAM
    &authCapUnixLogin,
#endif
    NULL
};

typedef struct {
    const char *name;
    Bool enabled;
    Bool permitted;
    int preference;
    Bool *optionSet;
    const char *optionName;
    Bool requiredData;
    SecTypeData *secType;
    AuthCapData *authCap;
} AuthMethodData;

/*
 * Set the "permitted" member to TRUE if you want the auth method to be
 * available by default.  The value of the "permitted-auth-methods" config file
 * option will take precedence over the defaults below.
 *
 * We permit the rfbAuthNone auth method by default for backward compatibility
 * and only enable it when either explicitly told to do so or if it is
 * permitted and no other auth methods were specified on the command line.
 */
static AuthMethodData authMethods[] = {
    { "none", FALSE, TRUE, -1, &rfbOptNoauth, "", FALSE,
      &secTypeNone, &authCapNone },

    { "vnc", FALSE, TRUE, -1, &rfbOptRfbauth, "-rfbauth", TRUE,
      &secTypeVncauth, &authCapVncauth },

    { "otp", FALSE, TRUE, -1, &rfbOptOtpauth, "-otpauth", TRUE,
      &secTypeVncauth, &authCapVncauth },

#ifdef XVNC_AuthPAM
    { "pam-userpwd", FALSE, TRUE, -1, &rfbOptPamauth, "-pamauth", TRUE,
      &secTypeTight, &authCapUnixLogin },
#endif

    { NULL }
};


static void
setMethods(char *buf)
{
    char *saveptr;
    char *p;
    AuthMethodData *a;

    preferenceLimit = 0;
    for (a = authMethods; a->name != NULL; a++) {
        a->permitted = FALSE;
        a->enabled = FALSE;
        a->preference = -1;
        a->secType->advertise = FALSE;
    }

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
            FatalError("ERROR: Unknown auth method name '%s'\n", p);
        }

        a->permitted = TRUE;
        a->preference = preferenceLimit++;
    }
}


static void
ReadConfigFile(void)
{
    FILE *fp;
    char buf[256], buf2[256];
    int line;
    int len;
    int n, i, j;
    struct stat sb;

    if ((fp = fopen(rfbAuthConfigFile, "r")) == NULL)
        return;

    if (fstat(fileno(fp), &sb) == -1) {
        FatalError("rfbAuthInit: ERROR: fstat %s: %s", rfbAuthConfigFile,
                   strerror(errno));
    }

    if ((sb.st_uid != 0) && (sb.st_uid != getuid())) {
        FatalError("ERROR: %s must be owned by you or by root\n",
                   rfbAuthConfigFile);
    }

    if (sb.st_mode & (S_IWGRP | S_IWOTH)) {
        FatalError("ERROR: %s cannot have group or global write permissions\n",
                   rfbAuthConfigFile);
    }

    rfbLog("Using auth configuration file %s\n", rfbAuthConfigFile);
    for (line = 0; fgets(buf, sizeof(buf), fp) != NULL; line++) {
        len = strlen(buf) - 1;
        if (buf[len] != '\n' && strlen(buf) == 256) {
            FatalError("ERROR in %s: line %d is too long!\n",
                       rfbAuthConfigFile, line + 1);
        }

        buf[len] = '\0';

        for (i = 0, j = 0; i < len; i++) {
            if (buf[i] != ' ' && buf[i] != '\t')
                 buf2[j++] = buf[i];
        }
        len = j;
        buf2[len] = '\0';
        if (len < 1) continue;

        if (!strcmp(buf2, "no-reverse-connections")) {
            rfbAuthDisableRevCon = TRUE;
            continue;
        }

        if (!strcmp(buf2, "no-remote-connections")) {
            interface.s_addr = htonl (INADDR_LOOPBACK);
            interface6 = in6addr_loopback;
            continue;
        }

        if (!strcmp(buf2, "no-clipboard-send")) {
            rfbAuthDisableCBSend = TRUE;
            continue;
        }

        if (!strcmp(buf2, "no-clipboard-recv")) {
            rfbAuthDisableCBRecv = TRUE;
            continue;
        }

        if (!strcmp(buf2, "no-httpd")) {
            rfbAuthDisableHTTP = TRUE;
            continue;
        }

        if (!strcmp(buf2, "no-x11-tcp-connections")) {
            rfbAuthDisableX11TCP = TRUE;
            continue;
        }


#ifdef XVNC_AuthPAM
        if (!strcmp(buf2, "enable-user-acl")) {
            rfbAuthUserACL = TRUE;
            continue;
        }

        n = 17;
        if (!strncmp(buf2, "pam-service-name=", n)) {
            if (buf2[n] == '\0') {
                FatalError("ERROR in %s: pam-service-name is empty!",
                           rfbAuthConfigFile);
            }

            if ((pamServiceName = strdup(&buf2[n])) == NULL) {
                FatalError("rfbAuthInit strdup: %s", strerror(errno));
            }

            continue;
        }
#endif

        n = 23;
        if (!strncmp(buf2, "permitted-auth-methods=", n)) {
            if (buf2[n] == '\0') {
                FatalError("ERROR in %s: permitted-auth-methods is empty!",
                           rfbAuthConfigFile);
            }

            setMethods(&buf2[n]);
            continue;
        }

        n = 17;
        if (!strncmp(buf2, "max-idle-timeout=", n)) {
            int t;

            if (buf2[n] == '\0') {
                FatalError("ERROR in %s: max-idle-timeout is empty!",
                           rfbAuthConfigFile);
            }

            if (sscanf(&buf2[n], "%d", &t) < 1 || t <= 0) {
                FatalError("ERROR in %s: max-idle-timeout value must be > 0!",
                           rfbAuthConfigFile);
            }

            rfbMaxIdleTimeout = (CARD32)t;
            continue;
        }

        n = 17;
        if (!strncmp(buf2, "max-desktop-size=", n)) {
            int w = -1, h = -1;

            if (buf2[n] == '\0') {
                FatalError("ERROR in %s: max-desktop-size is empty!",
                           rfbAuthConfigFile);
            }

            if (sscanf(&buf2[n], "%dx%d", &w, &h) < 2 || w <= 0 || h <= 0) {
                FatalError("ERROR in %s: max-desktop-size value is incorrect.",
                           rfbAuthConfigFile);
            }

            rfbMaxWidth = (CARD32)w;
            rfbMaxHeight = (CARD32)h;
            continue;
        }

        if (buf2[0] != '#')
            rfbLog("WARNING: unrecognized auth config line '%s'\n", buf);
    }

    fclose(fp);
}


void
rfbAuthInit()
{
    AuthMethodData *a;
    int nSelected = 0;

    ReadConfigFile();

    for (a = authMethods; a->name != NULL; a++) {
        if (*(a->optionSet) == TRUE) {
            nSelected++;
            if (!a->permitted) {
                if (a->optionName != NULL) {
                    rfbLog("WARNING: authentication option '%s' is not permitted\n",
                            a->optionName);
                }

                *(a->optionSet) = FALSE;
                continue;
            }

            a->enabled = TRUE;
        }

        if (a->enabled) {
            nAuthMethodsEnabled++;
            rfbLog("Enabled authentication method '%s'\n", a->name);
            if (!a->secType->advertise) {
                a->secType->advertise = TRUE;
                rfbLog("Advertising security type '%s' to viewers\n",
                       a->secType->name);
            }
        }
    }

    if (nSelected == 0) {
        /* No auth method was selected.  See if we should enable the
           rfbAuthNone authentication method. */
        for (a = authMethods; a->name != NULL; a++) {
            if (!a->requiredData) {
                if (a->permitted) {
                    nAuthMethodsEnabled++;
                    a->enabled = TRUE;
                    a->secType->advertise = TRUE;
                    rfbLog("Enabled authentication method '%s'\n", a->name);
                    rfbLog("Advertising security type '%s' to viewers\n",
                           a->secType->name);
                }
            } else {
                a->secType->advertise = FALSE;
            }
        }
    }

#ifndef XVNC_AuthPAM
    if (rfbOptPamauth)
        rfbLog("WARNING: PAM support is not compiled in.\n");
#endif

    if (nAuthMethodsEnabled == 0) {
        for (a = authMethods; a->name != NULL; a++) {
            if (a->permitted)
                rfbLog("NOTICE: %s is a permitted auth method\n", a->name);
        }

        FatalError("ERROR: no authentication methods enabled!\n");
    } else {
        /* Do not advertise rfbAuthNone if any other auth method is enabled */
        for (a = authMethods; a->name != NULL; a++) {
            if (a->enabled && strcmp(a->name, "none"))
                secTypeNone.advertise = FALSE;
        }
    }

#ifdef XVNC_AuthPAM
    if (rfbOptPamauth && rfbAuthUserACL) {
        struct passwd pbuf;
        struct passwd *pw;
        char buf[256];
        char *n;

        if (getpwuid_r(getuid(), &pbuf, buf, sizeof(buf), &pw) != 0) {
            FatalError("AuthPAMUserPwdRspFunc: limit-user enabled and getpwuid_r failed: %s",
                strerror(errno));
        }

        n = (char *)malloc(strlen(pbuf.pw_name));
        if (n == NULL) {
            FatalError("AuthPAMUserPwdRspFunc: out of memory");
        }
        strcpy(n, pbuf.pw_name);
        rfbAuthAddUser(n, FALSE);
    }
#endif
}


void
rfbAuthProcessResponse(rfbClientPtr cl)
{
    AuthCapData **p;
    AuthCapData *c;

    for (p = authCaps; *p != NULL; p++) {
        c = *p;
        if (cl->selectedAuthType == c->authType) {
            c->rspFunc(cl);
            return;
        }
    }

    rfbLog("rfbAuthProcessResponse: authType assertion failed");
    rfbCloseClient(cl);
}


/*
 * rfbAuthNewClient is called right after negotiating the protocol version.
 * Depending on the protocol version, we send either a code for the
 * authentication scheme to be used (protocol 3.3) or a list of possible
 * "security types" (protocol 3.7 and above.)
 */

void
rfbAuthNewClient(rfbClientPtr cl)
{
    SecTypeData **p;
    SecTypeData *s;

    if (rfbAuthIsBlocked()) {
        rfbLog("Too many authentication failures - client rejected\n");
        rfbClientConnFailed(cl, "Too many authentication failures");
        return;
    }

    if (cl->protocol_minor_ver >= 7) {
        rfbSendSecurityTypeList(cl);
        return;
    }

    /* Make sure we use only RFB 3.3-compatible security types */
    for (p = secTypes; *p != NULL; p++) {
        s = *p;
        if (s->advertise && (s->protocolMinorVer < 7))
            break;
    }

    if (*p == NULL) {
        rfbLog("VNC authentication disabled - RFB 3.3 client rejected\n");
        rfbClientConnFailed(cl, "Your viewer cannot handle required authentication methods");
        return;
    }

    cl->selectedAuthType = s->securityType;
    rfbSendSecurityType(cl, s->securityType);
}


/*
 * Tell the client which security type will be used (protocol 3.3)
 */

static void
rfbSendSecurityType(rfbClientPtr cl, int securityType)
{
    CARD32 value32;

    value32 = Swap32IfLE(securityType);
    if (WriteExact(cl, (char *)&value32, 4) < 0) {
        rfbLogPerror("rfbSendSecurityType: write");
        rfbCloseClient(cl);
        return;
    }

    switch (securityType) {
    case rfbSecTypeNone:
        /* Dispatch client input to rfbProcessClientInitMessage() */
        cl->state = RFB_INITIALISATION;
        break;
    case rfbSecTypeVncAuth:
        /* Begin the standard VNC authentication procedure */
        rfbVncAuthSendChallenge(cl);
        break;
    default:
        rfbLogPerror("rfbSendSecurityType: assertion failed");
        rfbCloseClient(cl);
    }
}


/*
 * Advertise our supported security types (protocol 3.7 and above)
 */

static void
rfbSendSecurityTypeList(rfbClientPtr cl)
{
    int n;
    SecTypeData **p;
    SecTypeData *s;
    Bool tightAdvertised = FALSE;

    n = 0;
    for (p = secTypes; *p != NULL; p++) {
        if (n > MAX_SECURITY_TYPES) {
            FatalError("rfbSendSecurityTypeList: # enabled security types > MAX_SECURITY_TYPES\n");
        }

        s = *p;
        if (s->advertise && (cl->protocol_minor_ver >= s->protocolMinorVer)) {
            cl->securityTypes[++n] = s->securityType;
            if (s->securityType == rfbSecTypeTight)
                tightAdvertised = TRUE;
        }
    }

    if (!tightAdvertised) {
        /*
         * Make sure to advertise the Tight security type, in order to allow
         * TightVNC-compatible clients to enable other (non-auth) Tight
         * extensions.
         */
        if (n > MAX_SECURITY_TYPES) {
            FatalError("rfbSendSecurityTypeList: # enabled security types > MAX_SECURITY_TYPES\n");
        }

        rfbLog("rfbSendSecurityTypeList: advertise sectype tight\n");
        cl->securityTypes[++n] = rfbSecTypeTight;
    }

    cl->securityTypes[0] = (CARD8)n;

    /* Send the list */
    if (WriteExact(cl, (char *)cl->securityTypes, n + 1) < 0) {
        rfbLogPerror("rfbSendSecurityTypeList: write");
        rfbCloseClient(cl);
        return;
    }

    /* Dispatch client input to rfbProcessClientSecurityType() */
    cl->state = RFB_SECURITY_TYPE;
}


/*
 * Read the security type chosen by the client (protocol 3.7 and above)
 */

void
rfbProcessClientSecurityType(rfbClientPtr cl)
{
    int n, count, i;
    CARD8 chosenType;

    /* Read the security type */
    n = ReadExact(cl, (char *)&chosenType, 1);
    if (n <= 0) {
        if (n == 0)
            rfbLog("rfbProcessClientSecurityType: client gone\n");
        else
            rfbLogPerror("rfbProcessClientSecurityType: read");
        rfbCloseClient(cl);
        return;
    }

    /* Make sure it was present in the list sent by the server */
    count = (int)cl->securityTypes[0];
    for (i = 1; i <= count; i++) {
        if (chosenType == cl->securityTypes[i])
            break;
    }
    if (i > count) {
        rfbLog("rfbProcessClientSecurityType: "
               "wrong security type requested\n");
        rfbCloseClient(cl);
        return;
    }

    cl->selectedAuthType = chosenType;
    switch (chosenType) {
    case rfbSecTypeNone:
        /* No authentication needed */
        rfbClientAuthSucceeded(cl, rfbAuthNone);
        break;
    case rfbSecTypeVncAuth:
        /* Begin the standard VNC authentication procedure */
        rfbVncAuthSendChallenge(cl);
        break;
    case rfbSecTypeTight:
        /* The viewer supports TightVNC extensions */
        rfbLog("Enabling TightVNC protocol extensions\n");
        /* Switch to protocol 3.7t/3.8t */
        cl->protocol_tightvnc = TRUE;
        /* Advertise our tunneling capabilities */
        rfbSendTunnelingCaps(cl);
        break;
    default:
        rfbLog("rfbProcessClientSecurityType: unknown authentication scheme\n");
        rfbCloseClient(cl);
        break;
   }
}


/*
 * Send the list of our tunneling capabilities (protocol 3.7t/3.8t)
 */

static void
rfbSendTunnelingCaps(rfbClientPtr cl)
{
    rfbTunnelingCapsMsg caps;
    CARD32 nTypes = 0;          /* We don't support tunneling yet */

    caps.nTunnelTypes = Swap32IfLE(nTypes);
    if (WriteExact(cl, (char *)&caps, sz_rfbTunnelingCapsMsg) < 0) {
        rfbLogPerror("rfbSendTunnelingCaps: write");
        rfbCloseClient(cl);
        return;
    }

    if (nTypes) {
        /* Dispatch client input to rfbProcessClientTunnelingType() */
        cl->state = RFB_TUNNELING_TYPE;
    } else {
        rfbSendAuthCaps(cl);
    }
}


/*
 * Read tunneling type requested by the client (protocol 3.7t/3.8t)
 *
 * NOTE: Currently we don't support tunneling, and this function can never be
 * called.
 */

void
rfbProcessClientTunnelingType(rfbClientPtr cl)
{
    /* If we were called, then something's really wrong. */
    rfbLog("rfbProcessClientTunnelingType: not implemented\n");
    rfbCloseClient(cl);
    return;
}


/*
 * Send the list of our authentication capabilities to the client
 * (protocol 3.7t/3.8t)
 */

static void
rfbSendAuthCaps(rfbClientPtr cl)
{
    rfbAuthenticationCapsMsg caps;
    rfbCapabilityInfo caplist[MAX_AUTH_CAPS];
    int count = 0;

    int j;
    AuthMethodData *a;
    AuthCapData *c;
    rfbCapabilityInfo *pcap;

    if (!cl->reverseConnection) {
        int i;

        /*
         * When no preference order was set using "permitted-auth-methods", the
         * default value of preferenceLimit (1) will cause us to execute the
         * outer loop once.  In this case, the a->preference members will all
         * be the default value (-1), and we skip the order testing.
         */
        for (i = 0; i < preferenceLimit; i++) {
            for (a = authMethods; a->name != NULL; a++) {
                if (((a->preference != -1) && (i != a->preference)) ||
                    !a->enabled)
                    continue;

                c = a->authCap;
                /*
                 * Check to see if we have already advertised this auth cap.
                 * VNC password and OTP both use the VNC authentication cap.
                 */
                for (j = 0; j < count; j++) {
                    if (cl->authCaps[j] == c->authType)
                        break;
                }

                if (j < count)
                    continue;

                if (count > MAX_AUTH_CAPS) {
                    FatalError("rfbSendAuthCaps: # enabled security types > MAX_AUTH_CAPS\n");
                }

                pcap = &caplist[count];
                pcap->code = Swap32IfLE(c->authType);
                memcpy(pcap->vendorSignature, c->vendorSignature,
                       sz_rfbCapabilityInfoVendor);
                memcpy(pcap->nameSignature, c->nameSignature,
                       sz_rfbCapabilityInfoName);
                cl->authCaps[count] = c->authType;
                count++;
            }
        }

        if (count == 0) {
            FatalError("rfbSendAuthCaps: authentication required but no auth methods enabled! This should not have happened!\n");
        }
    }

    cl->nAuthCaps = count;
    caps.nAuthTypes = Swap32IfLE((CARD32)count);
    if (WriteExact(cl, (char *)&caps, sz_rfbAuthenticationCapsMsg) < 0) {
        rfbLogPerror("rfbSendAuthCaps: write");
        rfbCloseClient(cl);
        return;
    }

    if (count) {
        if (WriteExact(cl, (char *)&caplist[0],
                       count *sz_rfbCapabilityInfo) < 0) {
            rfbLogPerror("rfbSendAuthCaps: write");
            rfbCloseClient(cl);
            return;
        }
        /* Dispatch client input to rfbProcessClientAuthType() */
        cl->state = RFB_AUTH_TYPE;
    } else {
        /* No authentication needed */
        rfbClientAuthSucceeded(cl, rfbAuthNone);
        cl->state = RFB_INITIALISATION;
    }
}


/*
 * Read client's preferred authentication type (protocol 3.7t/3.8t)
 */

void
rfbProcessClientAuthType(rfbClientPtr cl)
{
    CARD32 auth_type;
    int n, i;
    AuthCapData **p;
    AuthCapData *c;

    /* Read authentication type selected by the client */
    n = ReadExact(cl, (char *)&auth_type, sizeof(auth_type));
    if (n <= 0) {
        if (n == 0)
            rfbLog("rfbProcessClientAuthType: client gone\n");
        else
            rfbLogPerror("rfbProcessClientAuthType: read");
        rfbCloseClient(cl);
        return;
    }
    auth_type = Swap32IfLE(auth_type);

    /* Make sure it was present in the list sent by the server */
    for (i = 0; i < cl->nAuthCaps; i++) {
        if (auth_type == cl->authCaps[i])
            break;
    }
    if (i >= cl->nAuthCaps) {
        rfbLog("rfbProcessClientAuthType: "
               "wrong authentication type requested\n");
        rfbCloseClient(cl);
        return;
    }

    for (p = authCaps; *p != NULL; p++) {
        c = *p;
        if (auth_type == c->authType) {
            cl->selectedAuthType = auth_type;
            c->startFunc(cl);
            return;
        }
    }

    rfbLog("rfbProcessClientAuthType: unknown authentication scheme\n");
    rfbCloseClient(cl);
}


/*
 * Send the authentication challenge
 */

static void
rfbVncAuthSendChallenge(rfbClientPtr cl)
{
    vncRandomBytes(cl->authChallenge);
    if (WriteExact(cl, (char *)cl->authChallenge, CHALLENGESIZE) < 0) {
        rfbLogPerror("rfbVncAuthSendChallenge: write");
        rfbCloseClient(cl);
        return;
    }

    /* Dispatch client input to rfbVncAuthProcessResponse() */
    cl->state = RFB_AUTHENTICATION;
}


static Bool
CheckResponse(rfbClientPtr cl, int numPasswords, char *passwdFullControl,
              char *passwdViewOnly, CARD8 *response)
{
    Bool ok = FALSE;
    CARD8 encryptedChallenge1[CHALLENGESIZE];
    CARD8 encryptedChallenge2[CHALLENGESIZE];

    memcpy(encryptedChallenge1, cl->authChallenge, CHALLENGESIZE);
    vncEncryptBytes(encryptedChallenge1, passwdFullControl);
    memcpy(encryptedChallenge2, cl->authChallenge, CHALLENGESIZE);
    vncEncryptBytes(encryptedChallenge2, (numPasswords == 2) ?
                    passwdViewOnly : passwdFullControl);

    /* Delete the passwords from memory */
    memset(passwdFullControl, 0, MAXPWLEN + 1);
    memset(passwdViewOnly, 0, MAXPWLEN + 1);

    if (memcmp(encryptedChallenge1, response, CHALLENGESIZE) == 0) {
        rfbLog("Full-control authentication enabled for %s\n", cl->host);
        ok = TRUE;
        cl->viewOnly = FALSE;

    } else if (memcmp(encryptedChallenge2, response, CHALLENGESIZE) == 0) {
        rfbLog("View-only authentication enabled for %s\n", cl->host);
        ok = TRUE;
        cl->viewOnly = TRUE;
    }

    return(ok);
}


/*
 * rfbVncAuthProcessResponse is called when the client sends its
 * authentication response.
 */

void
rfbVncAuthProcessResponse(rfbClientPtr cl)
{
    char passwdFullControl[MAXPWLEN + 1] = "\0";
    char passwdViewOnly[MAXPWLEN + 1] = "\0";
    int numPasswords;
    Bool ok;
    int n;
    CARD8 response[CHALLENGESIZE];

    n = ReadExact(cl, (char *)response, CHALLENGESIZE);
    if (n <= 0) {
        if (n != 0)
            rfbLogPerror("rfbVncAuthProcessResponse: read");
        rfbCloseClient(cl);
        return;
    }

    ok = FALSE;
    if (rfbOptOtpauth) {
        if (rfbAuthOTPValue == NULL) {
            if (nAuthMethodsEnabled == 1) {
                rfbClientAuthFailed(cl, "The one-time password has not been set on the server");
                return;
            }

        } else {
            memcpy(passwdFullControl, rfbAuthOTPValue, MAXPWLEN);
            passwdFullControl[MAXPWLEN] = '\0';
            numPasswords = rfbAuthOTPValueLen / MAXPWLEN;
            if (numPasswords > 1) {
                memcpy(passwdViewOnly, rfbAuthOTPValue + MAXPWLEN, MAXPWLEN);
                passwdViewOnly[MAXPWLEN] = '\0';
            }

            ok = CheckResponse(cl, numPasswords, passwdFullControl,
                               passwdViewOnly, response);
            if (ok) {
                memset(rfbAuthOTPValue, 0, rfbAuthOTPValueLen);
                free(rfbAuthOTPValue);
                rfbAuthOTPValue = NULL;
            }
        }
    }

    if ((ok == FALSE) && rfbOptRfbauth) {
        numPasswords = vncDecryptPasswdFromFile2(rfbAuthPasswdFile,
                                                 passwdFullControl,
                                                 passwdViewOnly);
        if (numPasswords == 0) {
            rfbLog("rfbVncAuthProcessResponse: could not get password from %s\n",
                   rfbAuthPasswdFile);

            if (nAuthMethodsEnabled == 1) {
                rfbClientAuthFailed(cl, "The server could not read the VNC password file");
                return;
            }
        }

        ok = CheckResponse(cl, numPasswords, passwdFullControl, passwdViewOnly,
                           response);
    }

    if (ok) {
        rfbAuthUnblock();
        rfbClientAuthSucceeded(cl, rfbAuthVNC);
    } else {
        rfbLog("rfbVncAuthProcessResponse: authentication failed from %s\n",
               cl->host);
        if (rfbAuthConsiderBlocking()) {
            rfbClientAuthFailed(cl, "Authentication failed.  Too many tries");
        } else {
            rfbClientAuthFailed(cl, "Authentication failed");
        }
    }
}


/*
 * rfbClientConnFailed is called when a client connection has failed before
 * the authentication stage.
 */

void
rfbClientConnFailed(rfbClientPtr cl, char *reason)
{
    int headerLen, reasonLen;
    char buf[8];
    CARD32 *buf32 = (CARD32 *)buf;

    headerLen = (cl->protocol_minor_ver >= 7) ? 1 : 4;
    reasonLen = strlen(reason);
    buf32[0] = 0;
    buf32[1] = Swap32IfLE(reasonLen);

    if (WriteExact(cl, buf, headerLen) < 0 ||
        WriteExact(cl, buf + 4, 4) < 0 ||
        WriteExact(cl, reason, reasonLen) < 0) {
        rfbLogPerror("rfbClientConnFailed: write");
    }

    rfbCloseClient(cl);
}


/*
 * rfbClientAuthFailed is called on authentication failure.  Sending a reason
 * string is defined in RFB 3.8 and above.
 */

void
rfbClientAuthFailed(rfbClientPtr cl, char *reason)
{
    int reasonLen;
    char buf[8];
    CARD32 *buf32=(CARD32 *)buf;

    if (cl->protocol_minor_ver < 8)
        reason = NULL;          /* invalidate the pointer */

    reasonLen = (reason == NULL) ? 0 : strlen(reason);
    buf32[0] = Swap32IfLE(rfbAuthFailed);
    buf32[1] = Swap32IfLE(reasonLen);

    if (reasonLen == 0) {
        if (WriteExact(cl, buf, 4) < 0) {
            rfbLogPerror("rfbClientAuthFailed: write");
        }
    } else {
        if (WriteExact(cl, buf, 8) < 0 ||
            WriteExact(cl, reason, reasonLen) < 0) {
            rfbLogPerror("rfbClientAuthFailed: write");
        }
    }

    rfbCloseClient(cl);
}


/*
 * rfbClientAuthSucceeded is called on successful authentication.  It just
 * sends rfbAuthOK and dispatches client input to
 * rfbProcessClientInitMessage().  However, the rfbAuthOK message is not sent
 * if authentication was not required and the protocol version is 3.7 or lower.
 */

void
rfbClientAuthSucceeded(rfbClientPtr cl, CARD32 authType)
{
    CARD32 authResult;

    if (cl->protocol_minor_ver >= 8 || authType == rfbAuthVNC) {
        authResult = Swap32IfLE(rfbAuthOK);
        if (WriteExact(cl, (char *)&authResult, 4) < 0) {
            rfbLogPerror("rfbClientAuthSucceeded: write");
            rfbCloseClient(cl);
            return;
        }
    }

    /* Dispatch client input to rfbProcessClientInitMessage() */
    cl->state = RFB_INITIALISATION;
}


/*********************************************************************
 * Functions to prevent too many successive authentication failures.
 *
 * FIXME: This should be performed separately for each client.
 */

/* Maximum authentication failures before blocking connections */
#define MAX_AUTH_TRIES 5

/* Delay in ms.  This doubles for each failure over MAX_AUTH_TRIES. */
#define AUTH_TOO_MANY_BASE_DELAY 10 * 1000

static int rfbAuthTries = 0;
static Bool rfbAuthTooManyTries = FALSE;
static OsTimerPtr timer = NULL;


/*
 * This function should not be called directly.  It is called by setting a
 * timer in rfbAuthConsiderBlocking().
 */

static CARD32
rfbAuthReenable(OsTimerPtr timer, CARD32 now, pointer arg)
{
    rfbAuthTooManyTries = FALSE;
    return 0;
}


/*
 * This function should be called after each authentication failure.  The
 * return value will be true if there were too many failures.
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
 * This function should be called after a successful authentication.  It
 * resets the counter of authentication failures.  Note that it's not necessary
 * to clear the rfbAuthTooManyTries flag, as it will be reset by the timer
 * function.
 */

void
rfbAuthUnblock(void)
{
    rfbAuthTries = 0;
}


/*
 * This function should be called before authentication.  The return value will
 * be true if there were too many authentication failures, and the server
 * should not allow another try.
 */

Bool
rfbAuthIsBlocked(void)
{
    return rfbAuthTooManyTries;
}
