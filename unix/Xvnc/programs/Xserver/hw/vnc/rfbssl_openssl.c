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
#ifdef DLOPENSSL
#include <dlfcn.h>
#endif

static void rfbErr(char *format, ...);


#define BUFSIZE 1024


typedef void (*DH_free_type)(DH *);
typedef int (*DH_generate_key_type)(DH *);
typedef DH *(*DSA_dup_DH_type)(const DSA *);
typedef void (*DSA_free_type)(DSA *);
typedef int (*DSA_generate_parameters_ex_type)(DSA *, int, unsigned char *,
                                               int, int *, unsigned long *,
                                               BN_GENCB *);
typedef DSA *(*DSA_new_type)(void);
typedef unsigned long (*ERR_get_error_type)(void);
typedef char *(*ERR_error_string_type)(unsigned long, char *);

struct rfbcrypto_functions {
    DH_free_type DH_free;
    DH_generate_key_type DH_generate_key;
    DSA_dup_DH_type DSA_dup_DH;
    DSA_free_type DSA_free;
    DSA_generate_parameters_ex_type DSA_generate_parameters_ex;
    DSA_new_type DSA_new;
    ERR_get_error_type ERR_get_error;
    ERR_error_string_type ERR_error_string;
};

static struct rfbcrypto_functions crypto
#ifndef DLOPENSSL
    = { DH_free, DH_generate_key, DSA_dup_DH, DSA_free,
        DSA_generate_parameters_ex, DSA_new, ERR_get_error, ERR_error_string }
#endif
;

typedef int (*SSL_accept_type)(SSL *);
typedef int (*SSL_library_init_type)(void);
typedef void (*SSL_load_error_strings_type)(void);
typedef void (*SSL_free_type)(SSL *);
typedef int (*SSL_get_error_type)(const SSL *, int);
typedef SSL *(*SSL_new_type)(SSL_CTX *);
typedef int (*SSL_pending_type)(const SSL *);
typedef int (*SSL_read_type)(SSL *, void *, int);
typedef int (*SSL_set_fd_type)(SSL *, int);
typedef int (*SSL_shutdown_type)(SSL *);
typedef int (*SSL_write_type)(SSL *, const void *, int);
typedef long (*SSL_CTX_ctrl_type)(SSL_CTX *, int, long, void *);
typedef void (*SSL_CTX_free_type)(SSL_CTX *);
typedef SSL_CTX *(*SSL_CTX_new_type)(SSL_METHOD *);
typedef int (*SSL_CTX_set_cipher_list_type)(SSL_CTX *, const char *);
typedef int (*SSL_CTX_use_certificate_file_type)(SSL_CTX *, const char *, int);
typedef int (*SSL_CTX_use_PrivateKey_file_type)(SSL_CTX *, const char *, int);
typedef SSL_METHOD *(*TLSv1_server_method_type)(void);

struct rfbssl_functions {
    SSL_accept_type SSL_accept;
    SSL_library_init_type SSL_library_init;
    SSL_load_error_strings_type SSL_load_error_strings;
    SSL_free_type SSL_free;
    SSL_get_error_type SSL_get_error;
    SSL_new_type SSL_new;
    SSL_pending_type SSL_pending;
    SSL_read_type SSL_read;
    SSL_set_fd_type SSL_set_fd;
    SSL_shutdown_type SSL_shutdown;
    SSL_write_type SSL_write;
    SSL_CTX_ctrl_type SSL_CTX_ctrl;
    SSL_CTX_free_type SSL_CTX_free;
    SSL_CTX_new_type SSL_CTX_new;
    SSL_CTX_set_cipher_list_type SSL_CTX_set_cipher_list;
    SSL_CTX_use_certificate_file_type SSL_CTX_use_certificate_file;
    SSL_CTX_use_PrivateKey_file_type SSL_CTX_use_PrivateKey_file;
    TLSv1_server_method_type TLSv1_server_method;
};

static struct rfbssl_functions ssl
#ifndef DLOPENSSL
    = { SSL_accept, SSL_library_init, SSL_load_error_strings, SSL_free,
        SSL_get_error, SSL_new, SSL_pending, SSL_read, SSL_set_fd,
        SSL_shutdown, SSL_write, SSL_CTX_ctrl, SSL_CTX_free, SSL_CTX_new,
        SSL_CTX_set_cipher_list, SSL_CTX_use_certificate_file,
        SSL_CTX_use_PrivateKey_file, TLSv1_server_method }
#endif
;

#ifdef DLOPENSSL

static void *sslHandle = NULL, *cryptoHandle = NULL;

#ifndef __APPLE__
#define SUFFIXES 19
static const char *suffix[SUFFIXES] = { "0.9.8", "1.0.0", "1.0.1", "1.0.2",
    "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18",
    "19", "20" };
#endif

#define LOADSYM(lib, sym) {  \
    dlerror();  \
    if((lib.sym = (sym##_type)dlsym(lib##Handle, #sym)) == NULL) {  \
        char *err = dlerror();  \
        if (err)  \
            rfbErr("Could not load symbol: %s\n", err); \
        else  \
            rfbErr("Could not load symbol "#sym" from %s\n", libName);  \
        return -1;  \
    }  \
}

static int loadFunctions(void)
{
    if (sslHandle == NULL) {
#ifndef __APPLE__
        int i;
#endif
        char libName[80];

        memset(&ssl, 0, sizeof(ssl));
#ifdef __APPLE__
        snprintf(libName, 80, "libssl.dylib");
        if ((sslHandle = dlopen(libName, RTLD_NOW)) == NULL) {
            char *err = dlerror();
            if (err)
                rfbErr("Could not load libssl: %s\n", err);
            else
                rfbErr("Could not load libssl\n");
            return -1;
        }
#else
        for (i = 0; i < SUFFIXES; i++) {
            snprintf(libName, 80, "libssl.so.%s", suffix[i]);
            dlerror();  /* Clear error state */
            if ((sslHandle = dlopen(libName, RTLD_NOW)) != NULL)
                break;
        }
        if (i >= SUFFIXES) {
            rfbErr("Could not load libssl\n");
            return -1;
        }
#endif
        LOADSYM(ssl, SSL_accept);
        LOADSYM(ssl, SSL_library_init);
        LOADSYM(ssl, SSL_load_error_strings);
        LOADSYM(ssl, SSL_free);
        LOADSYM(ssl, SSL_get_error);
        LOADSYM(ssl, SSL_new);
        LOADSYM(ssl, SSL_pending);
        LOADSYM(ssl, SSL_read);
        LOADSYM(ssl, SSL_set_fd);
        LOADSYM(ssl, SSL_shutdown);
        LOADSYM(ssl, SSL_write);
        LOADSYM(ssl, SSL_CTX_ctrl);
        LOADSYM(ssl, SSL_CTX_free);
        LOADSYM(ssl, SSL_CTX_new);
        LOADSYM(ssl, SSL_CTX_set_cipher_list);
        LOADSYM(ssl, SSL_CTX_use_certificate_file);
        LOADSYM(ssl, SSL_CTX_use_PrivateKey_file);
        LOADSYM(ssl, TLSv1_server_method);
        rfbLog("Successfully loaded symbols from %s\n", libName);
    }

    if (cryptoHandle == NULL) {
#ifndef __APPLE__
        int i;
#endif
        char libName[80];

        memset(&crypto, 0, sizeof(crypto));
#ifdef __APPLE__
        snprintf(libName, 80, "libcrypto.dylib");
        if ((cryptoHandle = dlopen(libName, RTLD_NOW)) == NULL) {
            char *err = dlerror();
            if (err)
                rfbErr("Could not load libcrypto: %s\n", err);
            else
                rfbErr("Could not load libcrypto\n");
            return -1;
        }
#else
        for (i = 0; i < SUFFIXES; i++) {
            snprintf(libName, 80, "libcrypto.so.%s", suffix[i]);
            dlerror();  /* Clear error state */
            if ((cryptoHandle = dlopen(libName, RTLD_NOW)) != NULL)
                break;
        }
        if (i >= SUFFIXES) {
            rfbErr("Could not load libcrypto\n");
            return -1;
        }
#endif
        LOADSYM(crypto, DH_free);
        LOADSYM(crypto, DSA_dup_DH);
        LOADSYM(crypto, DSA_free);
        LOADSYM(crypto, DSA_generate_parameters_ex);
        LOADSYM(crypto, DSA_new);
        LOADSYM(crypto, DH_generate_key);
        LOADSYM(crypto, ERR_get_error);
        LOADSYM(crypto, ERR_error_string);
        rfbLog("Successfully loaded symbols from %s\n", libName);
    }

    return 0;
}

#endif


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
    unsigned long e = crypto.ERR_get_error();
    rfbErr("%s failed: %s (%d)\n", function, crypto.ERR_error_string(e, buf),
           e);
}


rfbSslCtx *rfbssl_init(rfbClientPtr cl, Bool anon)
{
    char *keyfile;
    struct rfbssl_ctx *ctx = NULL;
    DH *dh = NULL;
    DSA *dsa = NULL;

#ifdef DLOPENSSL
    if (loadFunctions() == -1)
        return NULL;
#endif

    ssl.SSL_library_init();
    ssl.SSL_load_error_strings();

    if (rfbAuthX509Key) {
      keyfile = rfbAuthX509Key;
    } else {
      keyfile = rfbAuthX509Cert;
    }

    if ((ctx = malloc(sizeof(struct rfbssl_ctx))) == NULL) {
        rfbErr("Out of memory\n");
        goto bailout;
    }
    memset(ctx, 0, sizeof(struct rfbssl_ctx));
    if ((ctx->ssl_ctx = ssl.SSL_CTX_new(ssl.TLSv1_server_method())) == NULL) {
        rfbssl_error("SSL_CTX_new()");
        goto bailout;
    }
    if (anon) {
        if ((dsa = crypto.DSA_new()) == NULL) {
            rfbssl_error("DSA_new()");
            goto bailout;
        }
        if (!crypto.DSA_generate_parameters_ex(dsa, 1024, NULL, 0, NULL, NULL,
                                               NULL)) {
            rfbssl_error("DSA_generate_paramters_ex()");
            goto bailout;
        }
        if (!(dh = crypto.DSA_dup_DH(dsa))) {
            rfbssl_error("DSA_dup_DH()");
            goto bailout;
        }
        crypto.DSA_free(dsa);  dsa = NULL;
        if (!crypto.DH_generate_key(dh)) {
            rfbssl_error("DH_generate_key()");
            goto bailout;
        }
        if (!ssl.SSL_CTX_ctrl(ctx->ssl_ctx, SSL_CTRL_SET_TMP_DH, 0,
                              (char *)dh)) {
            rfbssl_error("SSL_CTX_set_tmp_dh()");
            goto bailout;
        }
        if (!ssl.SSL_CTX_set_cipher_list(ctx->ssl_ctx, "aNULL")) {
            rfbssl_error("SSL_CTX_set_cipher_list()");
            goto bailout;
        }
    } else {
        if (!rfbAuthX509Cert || !rfbAuthX509Cert[0]) {
            rfbErr("No X.509 certificate specified\n");
            goto bailout;
        }
        if (ssl.SSL_CTX_use_certificate_file(ctx->ssl_ctx,
                rfbAuthX509Cert, SSL_FILETYPE_PEM) <= 0) {
            rfbErr("Unable to load X.509 certificate file %s\n",
                   rfbAuthX509Cert);
            goto bailout;
        }
        rfbLog("Using X.509 certificate file %s\n", rfbAuthX509Cert);
        if (ssl.SSL_CTX_use_PrivateKey_file(ctx->ssl_ctx, keyfile,
                                            SSL_FILETYPE_PEM) <= 0) {
            rfbErr("Unable to load X.509 private key file %s\n", keyfile);
            goto bailout;
        }
        rfbLog("Using X.509 private key file %s\n", keyfile);
    }
    if ((ctx->ssl = ssl.SSL_new(ctx->ssl_ctx)) == NULL) {
        rfbssl_error("SSL_new()");
        goto bailout;
    }
    if (!(ssl.SSL_set_fd(ctx->ssl, cl->sock))) {
        rfbssl_error("SSL_set_fd()");
        goto bailout;
    }

    return (rfbSslCtx *)ctx;

    bailout:
    if (dh) crypto.DH_free(dh);
    if (dsa) crypto.DSA_free(dsa);
    if (ctx) {
        if (ctx->ssl)
            ssl.SSL_free(ctx->ssl);
        if (ctx->ssl_ctx)
            ssl.SSL_CTX_free(ctx->ssl_ctx);
        free(ctx);
    }
    return NULL;
}


int rfbssl_accept(rfbClientPtr cl)
{
    int ret;
    struct rfbssl_ctx *ctx = (struct rfbssl_ctx *)cl->sslctx;

#ifdef DLOPENSSL
    if (loadFunctions() == -1)
        return -1;
#endif

    if ((ret = ssl.SSL_accept(ctx->ssl)) != 1) {
        int err = ssl.SSL_get_error(ctx->ssl, ret);
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

#ifdef DLOPENSSL
    if (loadFunctions() == -1)
        return -1;
#endif

    while ((ret = ssl.SSL_write(ctx->ssl, buf, bufsize)) <= 0) {
        if (ssl.SSL_get_error(ctx->ssl, ret) != SSL_ERROR_WANT_WRITE)
            break;
    }

    return ret;
}


int rfbssl_read(rfbClientPtr cl, char *buf, int bufsize)
{
    int ret;
    struct rfbssl_ctx *ctx = (struct rfbssl_ctx *)cl->sslctx;

#ifdef DLOPENSSL
    if (loadFunctions() == -1)
        return -1;
#endif

    while ((ret = ssl.SSL_read(ctx->ssl, buf, bufsize)) <= 0) {
        if (ssl.SSL_get_error(ctx->ssl, ret) != SSL_ERROR_WANT_READ)
            break;
    }

    return ret;
}


int rfbssl_pending(rfbClientPtr cl)
{
    struct rfbssl_ctx *ctx = (struct rfbssl_ctx *)cl->sslctx;

#ifdef DLOPENSSL
    if (loadFunctions() == -1)
        return -1;
#endif

    return ssl.SSL_pending(ctx->ssl);
}


void rfbssl_destroy(rfbClientPtr cl)
{
    struct rfbssl_ctx *ctx = (struct rfbssl_ctx *)cl->sslctx;

    if (!ctx) return;

    if (ctx->ssl) {
        ssl.SSL_shutdown(ctx->ssl);
        ssl.SSL_free(ctx->ssl);
    }
    if (ctx->ssl_ctx)
        ssl.SSL_CTX_free(ctx->ssl_ctx);

    free(ctx);
}


char *rfbssl_geterr(void)
{
    return errStr;
}
