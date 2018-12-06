/*
 * NOTE: The original file (from LibVNCServer) lacks a copyright header but is
 * assumed, from Git logs and context, to have been authored by Jocelyn Le Sage
 * and made available either under the same license as base64.c or under the
 * same license as LibVNCServer (GPL v2+).
 */

#ifndef _BASE64_H
#define _BASE64_H

extern int __b64_ntop(u_char const *src, size_t srclength, char *target, size_t targsize);
extern int __b64_pton(char const *src, u_char *target, size_t targsize);

#define rfbBase64NtoP __b64_ntop
#define rfbBase64PtoN __b64_pton

#endif /* _BASE64_H */
