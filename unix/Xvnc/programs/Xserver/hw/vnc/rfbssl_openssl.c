/*
 * rfbssl_openssl.c - Secure socket funtions (openssl version)
 */

/*
 *  Copyright (C) 2011 Gernot Tenchio
 *  Copyright (C) 2015 D. R. Commander
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
#include <openssl/ssl.h>
#include <openssl/err.h>


#define BUFSIZE 1024


static char errStr[BUFSIZE] = "No error";

struct rfbssl_ctx {
    SSL_CTX *ssl_ctx;
    SSL     *ssl;
};


static void rfbErr(char *format, ...)
{
    va_list args;

    va_start(args, format);
    snprintf(errStr, BUFSIZE, "Server TLS ERROR: ");
    vsnprintf(&errStr[strlen(errStr)], BUFSIZE - strlen(errStr), format, args);
    vrfbLog(format, args);
    va_end(args);
}


static void rfbssl_error(const char *function)
{
    char buf[1024];
    unsigned long e = ERR_get_error();
    rfbErr("%s failed: %s (%d)\n", function, ERR_error_string(e, buf), e);
}


rfbSslCtx *rfbssl_init(rfbClientPtr cl, Bool anon)
{
    char *keyfile;
    struct rfbssl_ctx *ctx = NULL;
    DH *dh = NULL;

    SSL_library_init();
    SSL_load_error_strings();

    if (rfbAuthX509Key) {
      keyfile = rfbAuthX509Key;
    } else {
      keyfile = rfbAuthX509Cert;
    }

    if ((ctx = malloc(sizeof(struct rfbssl_ctx))) == NULL) {
        rfbErr("Out of memory");
        goto bailout;
    }
    memset(ctx, 0, sizeof(struct rfbssl_ctx));
    if ((ctx->ssl_ctx = SSL_CTX_new(TLSv1_server_method())) == NULL) {
        rfbssl_error("SSL_CTX_new()");
        goto bailout;
    }
    if (anon) {
        int codes = 0;
        if ((dh = DH_new()) == NULL) {
            rfbssl_error("DH_new()");
            goto bailout;
        }
        if (!DH_generate_parameters_ex(dh, 512, DH_GENERATOR_2, 0)) {
            rfbssl_error("DH_generate_parameters()");
            goto bailout;
        }
        if (!DH_check(dh, &codes) || codes) {
            if (codes)
                rfbErr("DH_check() failed.  codes = 0x%.8x\n", codes);
            else
                rfbssl_error("DH_check()");
            goto bailout;
        }
        if (!DH_generate_key(dh)) {
            rfbssl_error("DH_generate_key()");
            goto bailout;
        }
        if (!SSL_CTX_set_tmp_dh(ctx->ssl_ctx, dh)) {
            rfbssl_error("SSL_CTX_set_tmp_dh()");
            goto bailout;
        }
        if (!SSL_CTX_set_cipher_list(ctx->ssl_ctx, "aNULL")) {
            rfbssl_error("SSL_CTX_set_cipher_list()");
            goto bailout;
        }
    } else {
        if (!rfbAuthX509Cert || !rfbAuthX509Cert[0]) {
            rfbErr("No X.509 certificate specified\n");
            goto bailout;
        }
        if (SSL_CTX_use_certificate_file(ctx->ssl_ctx,
              rfbAuthX509Cert, SSL_FILETYPE_PEM) <= 0) {
            rfbErr("Unable to load X.509 certificate file %s\n",
                   rfbAuthX509Cert);
            goto bailout;
        }
        rfbLog("Using X.509 certificate file %s\n", rfbAuthX509Cert);
        if (SSL_CTX_use_PrivateKey_file(ctx->ssl_ctx, keyfile,
                                        SSL_FILETYPE_PEM) <= 0) {
            rfbErr("Unable to load X.509 private key file %s\n", keyfile);
            goto bailout;
        }
        rfbLog("Using X.509 private key file %s\n", keyfile);
    }
    if ((ctx->ssl = SSL_new(ctx->ssl_ctx)) == NULL) {
        rfbssl_error("SSL_new()");
        goto bailout;
    }
    if (!(SSL_set_fd(ctx->ssl, cl->sock))) {
        rfbssl_error("SSL_set_fd()");
        goto bailout;
    }

    return (rfbSslCtx *)ctx;

    bailout:
    if (dh) DH_free(dh);
    if (ctx) {
        if (ctx->ssl)
            SSL_free(ctx->ssl);
        if (ctx->ssl_ctx)
            SSL_CTX_free(ctx->ssl_ctx);
        free(ctx);
    }
    return NULL;
}


int rfbssl_accept(rfbClientPtr cl)
{
    int ret;
    struct rfbssl_ctx *ctx = (struct rfbssl_ctx *)cl->sslctx;

    if ((ret = SSL_accept(ctx->ssl)) != 1) {
        int err = SSL_get_error(ctx->ssl, ret);
        if (err == SSL_ERROR_WANT_READ)
            return 1;
        else if (err == SSL_ERROR_SYSCALL)
            rfbErr("SSL_accept() failed: errno=%d\n", errno);
        else
            rfbErr("SSL_accept() failed: %d\n", err);
        return -1;
    }

    return 0;
}


int rfbssl_write(rfbClientPtr cl, const char *buf, int bufsize)
{
    int ret;
    struct rfbssl_ctx *ctx = (struct rfbssl_ctx *)cl->sslctx;

    while ((ret = SSL_write(ctx->ssl, buf, bufsize)) <= 0) {
        if (SSL_get_error(ctx->ssl, ret) != SSL_ERROR_WANT_WRITE)
            break;
    }

    return ret;
}


int rfbssl_read(rfbClientPtr cl, char *buf, int bufsize)
{
    int ret;
    struct rfbssl_ctx *ctx = (struct rfbssl_ctx *)cl->sslctx;

    while ((ret = SSL_read(ctx->ssl, buf, bufsize)) <= 0) {
        if (SSL_get_error(ctx->ssl, ret) != SSL_ERROR_WANT_READ)
            break;
    }

    return ret;
}


int rfbssl_pending(rfbClientPtr cl)
{
    struct rfbssl_ctx *ctx = (struct rfbssl_ctx *)cl->sslctx;

    return SSL_pending(ctx->ssl);
}


void rfbssl_destroy(rfbClientPtr cl)
{
    struct rfbssl_ctx *ctx = (struct rfbssl_ctx *)cl->sslctx;

    if (!ctx) return;

    if (ctx->ssl) {
        SSL_shutdown(ctx->ssl);
        SSL_free(ctx->ssl);
    }
    if (ctx->ssl_ctx)
        SSL_CTX_free(ctx->ssl_ctx);

    free(ctx);
}


char *rfbssl_geterr(void)
{
    return errStr;
}
