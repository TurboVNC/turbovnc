/*
 * rfbssl_gnutls.c - Secure socket funtions (gnutls version)
 */

/*
 *  Copyright (C) 2011 Gernot Tenchio
 *  Copyright (C) 2015, 2017, 2019 D. R. Commander
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

#include "rfb.h"
#include <gnutls/gnutls.h>
#include <errno.h>

CARD32 rfbTLSKeyLength = 2048;


#define BUFSIZE 1024


static char errStr[BUFSIZE] = "No error";

struct rfbssl_ctx {
  gnutls_session_t session;
  gnutls_anon_server_credentials_t anon_cred;
  gnutls_certificate_credentials_t x509_cred;
  gnutls_dh_params_t dh_params;
};


static void rfbErr(const char *format, ...)
{
  va_list args;

  va_start(args, format);
  snprintf(errStr, BUFSIZE, "Server TLS ERROR: ");
  vsnprintf(&errStr[strlen(errStr)], BUFSIZE - strlen(errStr), format, args);
  va_end(args);
}


static void rfbssl_error(const char *function, int e)
{
  rfbErr("%s failed: %s (%d)\n", function, gnutls_strerror(e), e);
  rfbLog("%s failed: %s (%d)\n", function, gnutls_strerror(e), e);
}


static void rfbssl_log_func(int level, const char *msg)
{
  rfbLog("GnuTLS: %s\n", msg);
}


rfbSslCtx *rfbssl_init(rfbClientPtr cl, Bool anon)
{
  struct rfbssl_ctx *ctx = NULL;
  char *keyfile;
  static Bool globalInit = FALSE;
  int ret;

  ctx = rfbAlloc0(sizeof(struct rfbssl_ctx));
  if (!globalInit) {
    if ((ret = gnutls_global_init()) != GNUTLS_E_SUCCESS) {
      rfbssl_error("gnutls_global_init()", ret);
      goto bailout;
    }
    globalInit = TRUE;
  }
  if ((ret = gnutls_init(&ctx->session, GNUTLS_SERVER)) != GNUTLS_E_SUCCESS) {
    rfbssl_error("gnutls_init()", ret);
    goto bailout;
  }
  if ((ret = gnutls_dh_params_init(&ctx->dh_params)) != GNUTLS_E_SUCCESS) {
    rfbssl_error("gnutls_dh_params_init()", ret);
    goto bailout;
  }
  if ((ret = gnutls_dh_params_generate2(ctx->dh_params, rfbTLSKeyLength)) !=
      GNUTLS_E_SUCCESS) {
    rfbssl_error("gnutls_dh_params_generate2()", ret);
    goto bailout;
  }
  if ((ret = gnutls_set_default_priority(ctx->session)) != GNUTLS_E_SUCCESS) {
    rfbssl_error("gnutls_set_default_priority()", ret);
    goto bailout;
  }
  if (anon) {
#if GNUTLS_VERSION_NUMBER < 0x030400
    static const int priority[] = { GNUTLS_KX_ANON_DH,
#if GNUTLS_VERSION_NUMBER >= 0x030000
                                    GNUTLS_KX_ANON_ECDH,
#endif
                                    0 };
    if ((ret = gnutls_kx_set_priority(ctx->session, priority)) !=
        GNUTLS_E_SUCCESS) {
      rfbssl_error("gnutls_kx_set_priority()", ret);
      goto bailout;
    }
#else
    static const char priority[] = "NORMAL:+ANON-ECDH:+ANON-DH", *err;
    if ((ret = gnutls_priority_set_direct(ctx->session, priority, &err)) !=
        GNUTLS_E_SUCCESS) {
      rfbssl_error("gnutls_priority_set_direct()", ret);
      goto bailout;
    }
#endif
    if ((ret = gnutls_anon_allocate_server_credentials(&ctx->anon_cred)) !=
        GNUTLS_E_SUCCESS) {
      rfbssl_error("gnutls_anon_allocate_server_credentials()", ret);
      goto bailout;
    }
    gnutls_anon_set_server_dh_params(ctx->anon_cred, ctx->dh_params);
    if ((ret = gnutls_credentials_set(ctx->session, GNUTLS_CRD_ANON,
                                      ctx->anon_cred)) != GNUTLS_E_SUCCESS) {
      rfbssl_error("gnutls_credentials_set()", ret);
      goto bailout;
    }
  } else {
    if (rfbAuthX509Key)
      keyfile = rfbAuthX509Key;
    else
      keyfile = rfbAuthX509Cert;

    if (!rfbAuthX509Cert || !rfbAuthX509Cert[0]) {
      rfbErr("No X.509 certificate specified\n");
      rfbLog("No X.509 certificate specified\n");
      goto bailout;
    }
    if ((ret = gnutls_certificate_allocate_credentials(&ctx->x509_cred)) !=
        GNUTLS_E_SUCCESS) {
      rfbssl_error("gnutls_certificate_allocate_credentials()", ret);
      goto bailout;
    }
    gnutls_certificate_set_dh_params(ctx->x509_cred, ctx->dh_params);
    if ((ret =
         gnutls_certificate_set_x509_key_file(ctx->x509_cred, rfbAuthX509Cert,
                                              keyfile, GNUTLS_X509_FMT_PEM)) !=
         GNUTLS_E_SUCCESS) {
      rfbssl_error("gnutls_certificate_set_x509_key_file()", ret);
      goto bailout;
    }
    rfbLog("Using X.509 certificate file %s\n", rfbAuthX509Cert);
    rfbLog("Using X.509 private key file %s\n", keyfile);
    if ((ret = gnutls_credentials_set(ctx->session, GNUTLS_CRD_CERTIFICATE,
                                      ctx->x509_cred)) != GNUTLS_E_SUCCESS) {
      rfbssl_error("gnutls_credentials_set()", ret);
      goto bailout;
    }
  }
  gnutls_transport_set_ptr(ctx->session,
                           (gnutls_transport_ptr_t)(uintptr_t)cl->sock);
  gnutls_global_set_log_function(rfbssl_log_func);
  gnutls_global_set_log_level(1);

  rfbLog("%s protocol initialized\n",
         gnutls_protocol_get_name(gnutls_protocol_get_version(ctx->session)));

  return (rfbSslCtx *)ctx;

  bailout:
  if (ctx) {
    if (ctx->session)
      gnutls_deinit(ctx->session);
    if (ctx->dh_params)
      gnutls_dh_params_deinit(ctx->dh_params);
    if (ctx->anon_cred)
      gnutls_anon_free_server_credentials(ctx->anon_cred);
    if (ctx->x509_cred)
      gnutls_certificate_free_credentials(ctx->x509_cred);
    free(ctx);
  }
  return NULL;
}


int rfbssl_accept(rfbClientPtr cl)
{
  int ret;
  struct rfbssl_ctx *ctx = (struct rfbssl_ctx *)cl->sslctx;

  if ((ret = gnutls_handshake(ctx->session)) != GNUTLS_E_SUCCESS) {
    if (!gnutls_error_is_fatal(ret))
      return 1;
    rfbssl_error("gnutls_handshake()", ret);
    return -1;
  }

  return 0;
}


int rfbssl_write(rfbClientPtr cl, const char *buf, int bufsize)
{
  struct rfbssl_ctx *ctx = (struct rfbssl_ctx *)cl->sslctx;
  int ret;

  while ((ret = gnutls_record_send(ctx->session, buf, bufsize)) < 0) {
    if (gnutls_error_is_fatal(ret))
      break;
  }
  if (ret < 0) {
    rfbssl_error("gnutls_record_send()", ret);
    errno = EIO;
    return -1;
  }

  return ret;
}


int rfbssl_read(rfbClientPtr cl, char *buf, int bufsize)
{
  struct rfbssl_ctx *ctx = (struct rfbssl_ctx *)cl->sslctx;
  int ret;

  while ((ret = gnutls_record_recv(ctx->session, buf, bufsize)) < 0) {
    if (gnutls_error_is_fatal(ret))
      break;
  }
  if (ret < 0) {
    rfbssl_error("gnutls_record_recv()", ret);
    errno = EIO;
    return -1;
  }

  return ret;
}


int rfbssl_pending(rfbClientPtr cl)
{
  struct rfbssl_ctx *ctx = (struct rfbssl_ctx *)cl->sslctx;

  return gnutls_record_check_pending(ctx->session);
}


void rfbssl_destroy(rfbClientPtr cl)
{
  struct rfbssl_ctx *ctx = (struct rfbssl_ctx *)cl->sslctx;

  if (!ctx) return;

  if (ctx->session) {
    gnutls_bye(ctx->session, GNUTLS_SHUT_WR);
    gnutls_deinit(ctx->session);
  }
  if (ctx->dh_params)
    gnutls_dh_params_deinit(ctx->dh_params);
  if (ctx->anon_cred)
    gnutls_anon_free_server_credentials(ctx->anon_cred);
  if (ctx->x509_cred)
    gnutls_certificate_free_credentials(ctx->x509_cred);

  free(ctx);
}


char *rfbssl_geterr(void)
{
  return errStr;
}
