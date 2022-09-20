/*
 * authpam.c - deal with PAM authentication.
 */

/* Copyright (C) 2015, 2017-2018, 2020 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2020 Andrew Yoder.  All Rights Reserved.
 * Copyright (C) 2010 University Corporation for Atmospheric Research.
 *                    All Rights Reserved.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>

#include "rfb.h"

#if defined(AIX) || defined(sun)
# define MESSAGE_ARG_TYPE struct pam_message **
#else
# define MESSAGE_ARG_TYPE const struct pam_message **
#endif

Bool rfbAuthPAMSession = FALSE;
Bool rfbAuthDisablePAMSession = FALSE;

static const char *password;

static int conv(int num_msg, MESSAGE_ARG_TYPE msg, struct pam_response **resp,
                void *appdata_ptr)
{
  const struct pam_message *m;
  struct pam_response *rp;
  struct pam_response *responses;
  int i;
  int pamRet = PAM_SUCCESS;
  int len;

  *resp = NULL;
  if (num_msg != 1) {
    rfbLog("PAMAuthenticate: conv num_msg invalid!\n");
    return PAM_SERVICE_ERR;
  }

  responses =
    (struct pam_response *)rfbAlloc0(sizeof(struct pam_response) * num_msg);

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
        rp->resp = (char *)rfbAlloc(len);

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
      free(rp->resp);
      rp++;
    }

    free(responses);
  }

  return pamRet;
}


Bool rfbPAMAuthenticate(rfbClientPtr cl, const char *svc, const char *user,
                        const char *pwd, const char **emsg)
{
  pam_handle_t *pamHandle;
  struct pam_conv pamConv;
  int r;
  int authStatus;
  struct passwd *pw = getpwuid(geteuid());
  Bool pamSession = rfbAuthPAMSession && !cl->viewOnly &&
                    !rfbAuthDisablePAMSession && pw &&
                    !strcmp(user, pw->pw_name);

  *emsg = "Failure encountered while initializing the authentication library";
  password = pwd;
  pamConv.conv = conv;
  pamConv.appdata_ptr = 0;
  if ((r = pam_start(svc, user, &pamConv, &pamHandle)) != PAM_SUCCESS) {
    rfbLog("PAMAuthenticate: pam_start: %s\n", pam_strerror(pamHandle, r));
    return FALSE;
  }

  if ((r = pam_set_item(pamHandle, PAM_RHOST, cl->host)) != PAM_SUCCESS) {
    rfbLog("PAMAuthenticate: pam_set_item PAM_RHOST: %s\n",
           pam_strerror(pamHandle, r));
    return FALSE;
  }

  authStatus = pam_authenticate(pamHandle, PAM_DISALLOW_NULL_AUTHTOK);
  if (authStatus != PAM_SUCCESS) {
    rfbLog("PAMAuthenticate: pam_authenticate: %s\n",
           pam_strerror(pamHandle, authStatus));
  } else {
    /* Authentication was successful.  Validate the user's account status. */
    authStatus = pam_acct_mgmt(pamHandle, PAM_DISALLOW_NULL_AUTHTOK);
    if (authStatus != PAM_SUCCESS) {
      rfbLog("PAMAuthenticate: pam_acct_mgmt: %s\n",
             pam_strerror(pamHandle, authStatus));
    } else if (pamSession) {
      if ((authStatus = pam_open_session(pamHandle, 0)) != PAM_SUCCESS) {
        rfbLog("PAMAuthenticate: pam_open_session: %s\n",
               pam_strerror(pamHandle, authStatus));
      } else {
        rfbLog("Opened PAM session for client %s\n", cl->host);
      }
    }
  }

  if (authStatus != PAM_SUCCESS || !pamSession) {
    if ((r = pam_end(pamHandle, authStatus)) != PAM_SUCCESS)
      rfbLog("PAMAuthenticate: pam_end: %s\n", pam_strerror(pamHandle, r));
  } else
    cl->pamHandle = pamHandle;

  switch (authStatus) {
    case PAM_SUCCESS:
      *emsg = NULL;
      return TRUE;

    case PAM_AUTH_ERR:
    case PAM_USER_UNKNOWN:
    case PAM_MAXTRIES:
      *emsg = "Authentication failed";
      break;

    case PAM_SESSION_ERR:
      *emsg = "Could not open PAM session on server";
      break;

    case PAM_AUTHINFO_UNAVAIL:
      *emsg = "Cannot authenticate at this time.  Try again later.";
      break;

    case PAM_ACCT_EXPIRED:
      *emsg = "User account is expired";
      break;

    case PAM_NEW_AUTHTOK_REQD:
      *emsg = "Authentication token/password is expired";
      break;

    default:
      *emsg = "Unknown authentication failure";
      break;
  }

  return FALSE;
}


void rfbPAMEnd(rfbClientPtr cl)
{
  int r;

  if (cl->pamHandle) {
    if ((r = pam_close_session(cl->pamHandle, 0)) != PAM_SUCCESS)
      rfbLog("PAMEnd: pam_close_session: %s\n",
             pam_strerror(cl->pamHandle, r));

    if ((r = pam_end(cl->pamHandle, PAM_SUCCESS)) != PAM_SUCCESS)
      rfbLog("PAMEnd: pam_end: %s\n", pam_strerror(cl->pamHandle, r));

    rfbLog("Closed PAM session for client %s\n", cl->host);
    cl->pamHandle = 0;
  }
}
