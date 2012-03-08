/*
 * authpam.c - deal with PAM authentication.
 */

/*
 *  Copyright (C) 2010 University Corporation for Atmospheric Research.
 *                     All Rights Reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#if defined(DARWIN)
#include <pam/pam_appl.h>
#else
#include <security/pam_appl.h>
#endif

#include "rfb.h"

#if defined(AIX) || defined(SOLARIS)
#  define MESSAGE_ARG_TYPE struct pam_message**
#else
#  define MESSAGE_ARG_TYPE const struct pam_message**
#endif

static const char* password;

static int
conv(int num_msg, MESSAGE_ARG_TYPE msg, struct pam_response** resp,
     void* appdata_ptr)
{
    const struct pam_message* m;
    struct pam_response*      rp;
    struct pam_response*      responses;
    int                       i;
    int                       pamRet = PAM_SUCCESS;
    int                       len;

    *resp = NULL;
    if (num_msg != 1) {
        rfbLog("PAMAuthenticate: conv num_msg invalid!\n");
        return(PAM_SERVICE_ERR);
    }

    if ((responses = (struct pam_response*) malloc(sizeof(struct pam_response)
        * num_msg)) == NULL) {
        rfbLogPerror("PAMAuthenticate: conv malloc");
        return(PAM_BUF_ERR);
    }

    memset(responses, 0, sizeof(struct pam_response) * num_msg);
    rp = responses;
    for (i = 0; (i < num_msg) && (pamRet == PAM_SUCCESS); i++, rp++) {
        m = msg[i];
        if (m->msg == NULL) {
            rfbLog("PAMAuthenticate: conv m->msg is NULL!\n");
            pamRet = PAM_SERVICE_ERR;
            break;
        }

        switch (m->msg_style) {
        case PAM_ERROR_MSG:
            rfbLog("PAMAuthenticate: PAM_ERROR_MSG '%s'\n", m->msg);
            break;

        case PAM_TEXT_INFO:
            rfbLog("PAMAuthenticate: PAM_TEXT_INFO '%s'\n", m->msg);
            break;

        case PAM_PROMPT_ECHO_OFF:
        case PAM_PROMPT_ECHO_ON:
            len = strlen(password) + 1;
            if ((rp->resp = (char*) malloc(len)) == NULL) {
                rfbLogPerror("PAMAuthenticate: conv malloc");
                pamRet = PAM_BUF_ERR;
                break;
            }

            memcpy(rp->resp, password, len);
            break;

        default:
            rfbLog("PAMAuthenticate: conv unknown msg_style!\n");
            pamRet = PAM_SERVICE_ERR;
            break;
        }
    }

    if (pamRet == PAM_SUCCESS) {
        *resp = responses;

    } else {
        rp = responses;
        for (i = 0; i < num_msg; i++) {
            if (rp->resp != 0)
                free(rp->resp);

            rp++;
        }

        free(responses);
    }

    return(pamRet);
}

Bool
rfbPAMAuthenticate(const char* svc, const char* host, const char* user,
                   const char* pwd, const char** emsg)
{
    pam_handle_t*    pamHandle;
    struct pam_conv    pamConv;
    int        r;
    int        authStatus;

    *emsg = "Failure encountered while initializing the authentication library";
    password = pwd;
    pamConv.conv = conv;
    pamConv.appdata_ptr = 0;
    if ((r = pam_start(svc, user, &pamConv, &pamHandle)) != PAM_SUCCESS) {
        rfbLog("PAMAuthenticate: pam_start: %s\n", pam_strerror(pamHandle, r));
        return(FALSE);
    }

    if ((r = pam_set_item(pamHandle, PAM_RHOST, host)) != PAM_SUCCESS) {
        rfbLog("PAMAuthenticate: pam_set_item PAM_RHOST: %s\n",
            pam_strerror(pamHandle, r));
        return(FALSE);
    }

    if ((authStatus = pam_authenticate(pamHandle, PAM_DISALLOW_NULL_AUTHTOK))
        != PAM_SUCCESS) {
        rfbLog("PAMAuthenticate: %s\n", pam_strerror(pamHandle, authStatus));
    }

    if ((r = pam_end(pamHandle, authStatus)) != PAM_SUCCESS) {
        rfbLog("PAMAuthenticate: pam_end: %s\n", pam_strerror(pamHandle, r));
    }

    switch (authStatus) {
    case PAM_SUCCESS:
        *emsg = NULL;
        return(TRUE);

    case PAM_AUTH_ERR:
    case PAM_USER_UNKNOWN:
    case PAM_MAXTRIES:
        *emsg = "Authentication failed";
        break;

    case PAM_AUTHINFO_UNAVAIL:
        *emsg = "Can not authenticate at this time, try again later";
        break;

    default:
        *emsg = "Unknown authentication failure";
        break;
    }

    return(FALSE);
}
