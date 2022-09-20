/*
 * rfbssl_openssl.c - Secure socket funtions (openssl version)
 */

/* Copyright (C) 2015, 2017-2020, 2022 D. R. Commander
 * Copyright (C) 2011 Gernot Tenchio
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

#include "rfb.h"
#ifdef DLOPENSSL
#define OPENSSL_API_COMPAT 0x10100000L
#endif
#include <openssl/ssl.h>
#include <openssl/err.h>
#ifdef DLOPENSSL
#include <dlfcn.h>
#endif

static void rfbErr(const char *format, ...);

CARD32 rfbTLSKeyLength = 2048;


#define BUFSIZE 1024

#ifndef SSL_CTRL_OPTIONS
#define SSL_CTRL_OPTIONS 32
#endif
#ifndef SSL_CTRL_SET_ECDH_AUTO
#define SSL_CTRL_SET_ECDH_AUTO 94
#endif
#ifndef OPENSSL_INIT_LOAD_CRYPTO_STRINGS
#define OPENSSL_INIT_LOAD_CRYPTO_STRINGS 0x00000002L
#endif
#ifndef OPENSSL_INIT_LOAD_SSL_STRINGS
#define OPENSSL_INIT_LOAD_SSL_STRINGS 0x00200000L
#endif

#if OPENSSL_VERSION_NUMBER >= 0x10000000L
#define CONST const
#else
#define CONST
#endif
#if OPENSSL_VERSION_NUMBER < 0x10100000L
typedef struct ossl_init_settings_st OPENSSL_INIT_SETTINGS;
#endif

typedef void (*DH_free_type) (DH *);
typedef int (*DH_generate_key_type) (DH *);
typedef int (*DH_size_type) (const DH *dh);
typedef DH *(*DSA_dup_DH_type) (const DSA *);
typedef void (*DSA_free_type) (DSA *);
typedef int (*DSA_generate_parameters_ex_type) (DSA *, int,
                                                CONST unsigned char *, int,
                                                int *, unsigned long *,
                                                BN_GENCB *);
typedef DSA *(*DSA_new_type) (void);
typedef unsigned long (*ERR_get_error_type) (void);
typedef char *(*ERR_error_string_type) (unsigned long, char *);

struct rfbcrypto_functions {
  DH_free_type DH_free;
  DH_generate_key_type DH_generate_key;
  DH_size_type DH_size;
  DSA_dup_DH_type DSA_dup_DH;
  DSA_free_type DSA_free;
  DSA_generate_parameters_ex_type DSA_generate_parameters_ex;
  DSA_new_type DSA_new;
  ERR_get_error_type ERR_get_error;
  ERR_error_string_type ERR_error_string;
};

static struct rfbcrypto_functions crypto = {
#ifdef DLOPENSSL
  NULL
#else
  DH_free, DH_generate_key, DH_size, DSA_dup_DH, DSA_free,
  DSA_generate_parameters_ex, DSA_new, ERR_get_error, ERR_error_string
#endif
};

typedef int (*SSL_accept_type) (SSL *);
typedef int (*OPENSSL_init_ssl_type) (uint64_t, const OPENSSL_INIT_SETTINGS *);
typedef int (*SSL_library_init_type) (void);
typedef void (*SSL_load_error_strings_type) (void);
typedef void (*SSL_free_type) (SSL *);
typedef const char *(*SSL_get_cipher_list_type) (const SSL *, int);
typedef CONST SSL_CIPHER *(*SSL_get_current_cipher_type) (const SSL *);
typedef int (*SSL_get_error_type) (const SSL *, int);
typedef SSL *(*SSL_new_type) (SSL_CTX *);
typedef int (*SSL_peek_type) (SSL *, void *, int);
typedef int (*SSL_pending_type) (const SSL *);
typedef int (*SSL_read_type) (SSL *, void *, int);
typedef int (*SSL_set_fd_type) (SSL *, int);
typedef int (*SSL_shutdown_type) (SSL *);
typedef int (*SSL_write_type) (SSL *, const void *, int);
typedef const char *(*SSL_CIPHER_get_name_type) (const SSL_CIPHER *);
typedef long (*SSL_CTX_ctrl_type) (SSL_CTX *, int, long, void *);
typedef void (*SSL_CTX_free_type) (SSL_CTX *);
typedef SSL_CTX *(*SSL_CTX_new_type) (CONST SSL_METHOD *);
typedef int (*SSL_CTX_set_cipher_list_type) (SSL_CTX *, const char *);
typedef void (*SSL_CTX_set_security_level_type) (SSL_CTX *, int);
typedef int (*SSL_CTX_use_certificate_file_type) (SSL_CTX *, const char *,
                                                  int);
typedef int (*SSL_CTX_use_PrivateKey_file_type) (SSL_CTX *, const char *, int);
typedef CONST SSL_METHOD *(*SSLv23_server_method_type) (void);

struct rfbssl_functions {
  SSL_accept_type SSL_accept;
  OPENSSL_init_ssl_type OPENSSL_init_ssl;
  SSL_library_init_type SSL_library_init;
  SSL_load_error_strings_type SSL_load_error_strings;
  SSL_free_type SSL_free;
  SSL_get_cipher_list_type SSL_get_cipher_list;
  SSL_get_current_cipher_type SSL_get_current_cipher;
  SSL_get_error_type SSL_get_error;
  SSL_new_type SSL_new;
  SSL_peek_type SSL_peek;
  SSL_pending_type SSL_pending;
  SSL_read_type SSL_read;
  SSL_set_fd_type SSL_set_fd;
  SSL_shutdown_type SSL_shutdown;
  SSL_write_type SSL_write;
  SSL_CIPHER_get_name_type SSL_CIPHER_get_name;
  SSL_CTX_ctrl_type SSL_CTX_ctrl;
  SSL_CTX_free_type SSL_CTX_free;
  SSL_CTX_new_type SSL_CTX_new;
  SSL_CTX_set_cipher_list_type SSL_CTX_set_cipher_list;
  SSL_CTX_set_security_level_type SSL_CTX_set_security_level;
  SSL_CTX_use_certificate_file_type SSL_CTX_use_certificate_file;
  SSL_CTX_use_PrivateKey_file_type SSL_CTX_use_PrivateKey_file;
  SSLv23_server_method_type SSLv23_server_method;
};

static struct rfbssl_functions ssl = {
#ifdef DLOPENSSL
  NULL
#else
  SSL_accept,
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
  OPENSSL_init_ssl, NULL, NULL,
#else
  NULL, SSL_library_init, SSL_load_error_strings,
#endif
  SSL_free, SSL_get_cipher_list, SSL_get_current_cipher, SSL_get_error,
  SSL_new, SSL_peek, SSL_pending, SSL_read, SSL_set_fd, SSL_shutdown,
  SSL_write, SSL_CIPHER_get_name, SSL_CTX_ctrl, SSL_CTX_free, SSL_CTX_new,
  SSL_CTX_set_cipher_list,
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
  SSL_CTX_set_security_level,
#else
  NULL,
#endif
  SSL_CTX_use_certificate_file, SSL_CTX_use_PrivateKey_file,
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
  TLS_server_method
#else
  SSLv23_server_method
#endif
#endif
};

#ifdef DLOPENSSL

static void *sslHandle = NULL, *cryptoHandle = NULL;

#ifndef __APPLE__
#define SUFFIXES 22
static const char *suffix[SUFFIXES] = { "3", "1.1", "111", "1.0.2", "1.0.1",
  "1.0.0", "0.9.8", "20", "19", "18", "17", "16", "15", "14", "13", "12", "11",
  "10", "9", "8", "7", "6" };
#endif

#define LOADSYM(lib, sym) {  \
    dlerror();  \
    if ((lib.sym = (sym##_type)dlsym(lib##Handle, #sym)) == NULL) {  \
      char *err = dlerror();  \
      if (err)  \
        rfbErr("Could not load symbol: %s\n", err);  \
      else  \
        rfbErr("Could not load symbol "#sym" from %s\n", libName);  \
      return -1;  \
    }  \
}

#define LOADSYMOPT(lib, sym, name) {  \
    dlerror();  \
    lib.sym = (sym##_type)dlsym(lib##Handle, name);  \
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
    LOADSYMOPT(ssl, OPENSSL_init_ssl, "OPENSSL_init_ssl");
    if (!ssl.OPENSSL_init_ssl) {
      LOADSYM(ssl, SSL_library_init);
      LOADSYM(ssl, SSL_load_error_strings);
    }
    LOADSYM(ssl, SSL_free);
    LOADSYM(ssl, SSL_get_cipher_list);
    LOADSYM(ssl, SSL_get_current_cipher);
    LOADSYM(ssl, SSL_get_error);
    LOADSYM(ssl, SSL_new);
    LOADSYM(ssl, SSL_peek);
    LOADSYM(ssl, SSL_pending);
    LOADSYM(ssl, SSL_read);
    LOADSYM(ssl, SSL_set_fd);
    LOADSYM(ssl, SSL_shutdown);
    LOADSYM(ssl, SSL_write);
    LOADSYM(ssl, SSL_CIPHER_get_name);
    LOADSYM(ssl, SSL_CTX_ctrl);
    LOADSYM(ssl, SSL_CTX_free);
    LOADSYM(ssl, SSL_CTX_new);
    LOADSYM(ssl, SSL_CTX_set_cipher_list);
    LOADSYMOPT(ssl, SSL_CTX_set_security_level, "SSL_CTX_set_security_level");
    LOADSYM(ssl, SSL_CTX_use_certificate_file);
    LOADSYM(ssl, SSL_CTX_use_PrivateKey_file);
    LOADSYMOPT(ssl, SSLv23_server_method, "TLS_server_method");
    if (!ssl.SSLv23_server_method) {
      LOADSYM(ssl, SSLv23_server_method);
    }
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
    LOADSYM(crypto, DH_size);
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


static void rfbErr(const char *format, ...)
{
  va_list args;

  va_start(args, format);
  snprintf(errStr, BUFSIZE, "Server TLS ERROR: ");
  vsnprintf(&errStr[strlen(errStr)], BUFSIZE - strlen(errStr), format, args);
  va_end(args);
  rfbLog("%s", &errStr[18]);
}


static void rfbssl_error(const char *function)
{
  char buf[1024];
  unsigned long e = crypto.ERR_get_error();

  while (e) {
    rfbErr("%s failed: %s (%d)\n", function,
           crypto.ERR_error_string(e, buf), e);
    e = crypto.ERR_get_error();
  }
}


rfbSslCtx *rfbssl_init(rfbClientPtr cl, Bool anon)
{
  char *keyfile;
  struct rfbssl_ctx *ctx = NULL;
  DH *dh = NULL;
  DSA *dsa = NULL;
  int flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3, priority = 0;
  const char *list = NULL;

#ifdef DLOPENSSL
  if (loadFunctions() == -1)
    return NULL;
#endif

  if (ssl.OPENSSL_init_ssl)
    ssl.OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS |
                         OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);
  else {
    ssl.SSL_library_init();
    ssl.SSL_load_error_strings();
  }

  if (rfbAuthX509Key) {
    keyfile = rfbAuthX509Key;
  } else {
    keyfile = rfbAuthX509Cert;
  }

  ctx = rfbAlloc0(sizeof(struct rfbssl_ctx));
  if ((ctx->ssl_ctx = ssl.SSL_CTX_new(ssl.SSLv23_server_method())) == NULL) {
    rfbssl_error("SSL_CTX_new()");
    goto bailout;
  }
  ssl.SSL_CTX_ctrl(ctx->ssl_ctx, SSL_CTRL_OPTIONS, flags, NULL);
  if (anon) {
    if ((dsa = crypto.DSA_new()) == NULL) {
      rfbssl_error("DSA_new()");
      goto bailout;
    }
    if (!crypto.DSA_generate_parameters_ex(dsa, rfbTLSKeyLength, NULL, 0, NULL,
                                           NULL, NULL)) {
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
    rfbLog("Anonymous TLS key length: %d bits\n", crypto.DH_size(dh) * 8);
    if (!ssl.SSL_CTX_ctrl(ctx->ssl_ctx, SSL_CTRL_SET_TMP_DH, 0, (char *)dh)) {
      rfbssl_error("SSL_CTX_set_tmp_dh()");
      goto bailout;
    }
    if (!ssl.SSL_CTX_set_cipher_list(ctx->ssl_ctx, rfbAuthCipherSuites ?
                                                   rfbAuthCipherSuites :
                                                   "aNULL")) {
      rfbssl_error("SSL_CTX_set_cipher_list()");
      goto bailout;
    }
    if (ssl.SSL_CTX_set_security_level)
      ssl.SSL_CTX_set_security_level(ctx->ssl_ctx, 0);
  } else {
    if (!rfbAuthX509Cert || !rfbAuthX509Cert[0]) {
      rfbErr("No X.509 certificate specified\n");
      goto bailout;
    }
    if (ssl.SSL_CTX_use_certificate_file(ctx->ssl_ctx, rfbAuthX509Cert,
                                         SSL_FILETYPE_PEM) <= 0) {
      rfbErr("Unable to load X.509 certificate file %s\n", rfbAuthX509Cert);
      goto bailout;
    }
    rfbLog("Using X.509 certificate file %s\n", rfbAuthX509Cert);
    if (ssl.SSL_CTX_use_PrivateKey_file(ctx->ssl_ctx, keyfile,
                                        SSL_FILETYPE_PEM) <= 0) {
      rfbErr("Unable to load X.509 private key file %s\n", keyfile);
      goto bailout;
    }
    rfbLog("Using X.509 private key file %s\n", keyfile);
    if (rfbAuthCipherSuites) {
      if (!ssl.SSL_CTX_set_cipher_list(ctx->ssl_ctx, rfbAuthCipherSuites)) {
        rfbssl_error("SSL_CTX_set_cipher_list()");
        goto bailout;
      }
    }
  }
  ssl.SSL_CTX_ctrl(ctx->ssl_ctx, SSL_CTRL_SET_ECDH_AUTO, 1, NULL);
  if ((ctx->ssl = ssl.SSL_new(ctx->ssl_ctx)) == NULL) {
    rfbssl_error("SSL_new()");
    goto bailout;
  }
  rfbLog("Available cipher suites: ");
  list = ssl.SSL_get_cipher_list(ctx->ssl, priority++);
  while (list) {
    fprintf(stderr, "%s", list);
    list = ssl.SSL_get_cipher_list(ctx->ssl, priority++);
    if (list) fprintf(stderr, ":");
  }
  fprintf(stderr, "\n");
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
      rfbssl_error("SSL_accept()");
    return -1;
  }
  rfbLog("Negotiated cipher suite: %s\n",
         ssl.SSL_CIPHER_get_name(ssl.SSL_get_current_cipher(ctx->ssl)));

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


int rfbssl_peek(rfbClientPtr cl, char *buf, int bufsize)
{
  int ret;
  struct rfbssl_ctx *ctx = (struct rfbssl_ctx *)cl->sslctx;

#ifdef DLOPENSSL
  if (loadFunctions() == -1)
    return -1;
#endif

  while ((ret = ssl.SSL_peek(ctx->ssl, buf, bufsize)) <= 0) {
    if (ssl.SSL_get_error(ctx->ssl, ret) != SSL_ERROR_WANT_READ)
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
