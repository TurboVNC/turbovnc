/*
 * cutpaste.c - routines to deal with cut & paste buffers / selection.
 */

/* Copyright (C) 2014, 2017-2019, 2022 D. R. Commander.  All Rights Reserved.
 * Copyright 2016-2017, 2019-2021 Pierre Ossman for Cendio AB
 * Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
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

/************************************************************

Copyright 1987, 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

Copyright 1987, 1989 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include <X11/X.h>
#include <X11/Xproto.h>
#include "rfb.h"
#include "selection.h"
#include "propertyst.h"
#include "xace.h"
#include <X11/Xatom.h>
#include <errno.h>

Bool rfbSyncPrimary = TRUE;

static Bool probing;
char *cachedData = NULL;
static WindowPtr pSelectionWin = NULL;
static Window win = 0;
static Atom xaPRIMARY, xaCLIPBOARD, xaTARGETS, xaTIMESTAMP, xaSTRING, xaTEXT,
  xaUTF8_STRING, activeSelection = None;
static rfbClientPtr clipboardClient = NULL;
struct xorg_list clipboardRequestors;

/*
 * Linked list of X11 clients that should receive clipboard updates
 */

struct VncDataTarget {
  ClientPtr client;
  Atom selection;
  Atom target;
  Atom property;
  Window requestor;
  CARD32 time;
  struct VncDataTarget *next;
};

static struct VncDataTarget *vncDataTargetHead;


/*
 * Naming convention:
 *
 * rfb*():  Client-specific functions that are called by the RFB server
 * vnc*():  Non-client-specific functions
 * All others:  Client-specific functions that are called only by other
 *              clipboard functions
 */

static void HandleClipboardRequest(rfbClientPtr cl, CARD32 flags);
static void SendClipboardMessage(rfbClientPtr cl, CARD32 msgType,
                                 CARD32 flags);
static void SendClipboardProvide(rfbClientPtr cl, CARD32 flags,
                                 const CARD32 *lengths, const char *data[]);
static void vncClientStateCallback(CallbackListPtr *l, void *d, void *p);
static void vncHandleClipboardAnnounce(Bool available);
static void vncHandleClipboardData(const char *data);
static void vncHandleClipboardRequest(void);
static Bool vncHasAtom(Atom atom, const Atom list[], size_t size);
static void vncMaybeRequestCache(void);
static int vncOwnSelection(Atom selection);
static void vncRequestClipboard(void);
static void vncSelectionCallback(CallbackListPtr *callbacks, pointer data,
                                 pointer args);
static void vncSelectionRequest(Atom selection, Atom target);
static void vncSendClipboardData(const char *data);
static Bool vncWeAreOwner(Atom selection);


#define ZLIB_ERROR(zlibStream, zlibErr, zlibFunc) {  \
  if (zlibStream.msg)  \
    rfbLog("%s (%d): zlib %s error (%s)\n", __FUNCTION__, __LINE__,  \
           #zlibFunc, zlibStream.msg);  \
  else  \
    rfbLog("%s (%d): zlib %s error (%d)\n", __FUNCTION__, __LINE__,  \
           #zlibFunc, zlibErr);  \
}


/*
 * Text conversion functions.  The returned strings are always NULL-terminated
 * and must be freed using free().
 */

static char *ConvertCRLF(const char *src, size_t bytes)
{
  char *buffer, *out;
  const char *in;
  size_t in_len, sz;

  /* Always allow space for a NULL */
  sz = 1;

  /* Compute output size */
  in = src;
  in_len = bytes;
  while ((in_len > 0) && (*in != '\0')) {
    sz++;

    if (*in == '\r') {
      if ((in_len < 2) || (*(in + 1) != '\n'))
        sz++;
    } else if (*in == '\n') {
      if ((in == src) || (*(in - 1) != '\r'))
        sz++;
    }

    in++;
    in_len--;
  }

  /* Allocate ... */
  buffer = rfbAlloc0(sz);

  /* ... and convert */
  out = buffer;
  in = src;
  in_len = bytes;
  while ((in_len > 0) && (*in != '\0')) {
    if (*in == '\n') {
      if ((in == src) || (*(in - 1) != '\r'))
        *out++ = '\r';
    }

    *out = *in;

    if (*in == '\r') {
      if ((in_len < 2) || (*(in + 1) != '\n')) {
        out++;
        *out = '\n';
      }
    }

    out++;
    in++;
    in_len--;
  }

  return buffer;
}


static char *ConvertLF(const char *src, size_t bytes)
{
  char *buffer, *out;
  const char *in;
  size_t in_len, sz;

  /* Always allow space for a NULL */
  sz = 1;

  /* Compute output size */
  in = src;
  in_len = bytes;
  while ((in_len > 0) && (*in != '\0')) {
    if (*in != '\r') {
      sz++;
      in++;
      in_len--;
      continue;
    }

    if ((in_len < 2) || (*(in + 1) != '\n'))
      sz++;

    in++;
    in_len--;
  }

  /* Allocate ... */
  buffer = rfbAlloc0(sz);

  /* ... and convert */
  out = buffer;
  in = src;
  in_len = bytes;
  while ((in_len > 0) && (*in != '\0')) {
    if (*in != '\r') {
      *out++ = *in++;
      in_len--;
      continue;
    }

    if ((in_len < 2) || (*(in + 1) != '\n'))
      *out++ = '\n';

    in++;
    in_len--;
  }

  return buffer;
}


static size_t UCS4ToUTF8(unsigned src, char *dst)
{
  if (src < 0x80) {
    *dst++ = src;
    *dst++ = '\0';
    return 1;
  } else if (src < 0x800) {
    *dst++ = 0xc0 | (src >> 6);
    *dst++ = 0x80 | (src & 0x3f);
    *dst++ = '\0';
    return 2;
  } else if (src < 0x10000) {
    *dst++ = 0xe0 | (src >> 12);
    *dst++ = 0x80 | ((src >> 6) & 0x3f);
    *dst++ = 0x80 | (src & 0x3f);
    *dst++ = '\0';
    return 3;
  } else if (src < 0x110000) {
    *dst++ = 0xf0 | (src >> 18);
    *dst++ = 0x80 | ((src >> 12) & 0x3f);
    *dst++ = 0x80 | ((src >> 6) & 0x3f);
    *dst++ = 0x80 | (src & 0x3f);
    *dst++ = '\0';
    return 4;
  } else {
    return UCS4ToUTF8(0xfffd, dst);
  }
}


static char *Latin1ToUTF8(const char *src, size_t bytes)
{
  char *buffer, *out;
  const char *in;
  size_t in_len, sz;

  /* Always allow space for a NULL */
  sz = 1;

  /* Compute output size */
  in = src;
  in_len = bytes;
  while ((in_len > 0) && (*in != '\0')) {
    char buf[5];

    sz += UCS4ToUTF8(*(const unsigned char *)in, buf);
    in++;
    in_len--;
  }

  /* Allocate ... */
  buffer = rfbAlloc0(sz);

  /* ... and convert */
  out = buffer;
  in = src;
  in_len = bytes;
  while ((in_len > 0) && (*in != '\0')) {
    out += UCS4ToUTF8(*(const unsigned char *)in, out);
    in++;
    in_len--;
  }

  return buffer;
}


static size_t UTF8ToUCS4(const char *src, size_t max, unsigned *dst)
{
  size_t count, consumed;

  *dst = 0xfffd;

  if (max == 0)
    return 0;

  consumed = 1;

  if ((*src & 0x80) == 0) {
    *dst = *src;
    count = 0;
  } else if ((*src & 0xe0) == 0xc0) {
    *dst = *src & 0x1f;
    count = 1;
  } else if ((*src & 0xf0) == 0xe0) {
    *dst = *src & 0x0f;
    count = 2;
  } else if ((*src & 0xf8) == 0xf0) {
    *dst = *src & 0x07;
    count = 3;
  } else {
    /* Invalid sequence.  Consume all continuation characters. */
    src++;
    max--;
    while ((max-- > 0) && ((*src++ & 0xc0) == 0x80))
      consumed++;
    return consumed;
  }

  src++;
  max--;

  while (count--) {
    consumed++;

    /* Invalid or truncated sequence? */
    if ((max == 0) || ((*src & 0xc0) != 0x80)) {
      *dst = 0xfffd;
      return consumed;
    }

    *dst <<= 6;
    *dst |= *src & 0x3f;

    src++;
    max--;
  }

  return consumed;
}


static char *UTF8ToLatin1(const char *src, size_t bytes)
{
  char *buffer, *out;
  const char *in;
  size_t in_len, sz;

  /* Always allow space for a NULL */
  sz = 1;

  /* Compute output size */
  in = src;
  in_len = bytes;
  while ((in_len > 0) && (*in != '\0')) {
    size_t len;
    unsigned ucs;

    len = UTF8ToUCS4(in, in_len, &ucs);
    in += len;
    in_len -= len;
    sz++;
  }

  /* Allocate ... */
  buffer = rfbAlloc0(sz);

  /* ... and convert */
  out = buffer;
  in = src;
  in_len = bytes;
  while ((in_len > 0) && (*in != '\0')) {
    size_t len;
    unsigned ucs;

    len = UTF8ToUCS4(in, in_len, &ucs);
    in += len;
    in_len -= len;

    if (ucs > 0xff)
      *out++ = '?';
    else
      *out++ = (unsigned char)ucs;
  }

  return buffer;
}


/*
 * Handle an Extended Clipboard capabilities message from an RFB client.  This
 * function is called only by rfbReadExtClipboard().
 */

static void HandleClipboardCaps(rfbClientPtr cl, CARD32 flags, CARD32 *lengths)
{
  int i, num;

  rfbLog("Client clipboard capabilities:\n");
  num = 0;
  for (i = 0; i < 16; i++) {
    if (flags & (1 << i)) {
      switch (1 << i) {
        case rfbExtClipUTF8:
          rfbLog("- Plain text (limit = %d bytes)\n", lengths[num]);
          break;
        case rfbExtClipRTF:
          rfbLog("- Rich text (limit = %d bytes)\n", lengths[num]);
          break;
        case rfbExtClipHTML:
          rfbLog("- HTML (limit = %d bytes)\n", lengths[num]);
          break;
        case rfbExtClipDIB:
          rfbLog("- Images (limit = %d bytes)\n", lengths[num]);
          break;
        case rfbExtClipFiles:
          rfbLog("- Files (limit = %d bytes)\n", lengths[num]);
          break;
        default:
          rfbLog("- Unknown format 0x%x\n", 1 << i);
          continue;
      }
      num++;
    }
  }

  cl->clipFlags = flags;

  num = 0;
  for (i = 0; i < 16; i++) {
    if (!(flags & (1 << i)))
      continue;
    cl->clipSizes[i] = lengths[num++];
  }
}


/*
 * HandleClipboardData() is called whenever an RFB client has sent its
 * clipboard data in response to a previous call to vncRequestClipboard().
 * Note that this function might never be called if the clipboard data was no
 * longer available when the client received the request.  This function is
 * called by HandleClipboardProvide() and vncRequestClipboard().
 */

static void HandleClipboardData(rfbClientPtr cl, const char *data)
{
  if (rfbViewOnly || cl->viewOnly || rfbAuthDisableCBRecv || !data)
    return;

  if (cl != clipboardClient) {
    LogMessage(X_DEBUG, "Ignoring unexpected clipboard data\n");
    return;
  }
  vncHandleClipboardData(data);
}


/*
 * Handle an Extended Clipboard notify message from an RFB client.  This
 * function is called only by rfbReadExtClipboard().
 */

static void HandleClipboardNotify(rfbClientPtr cl, CARD32 flags)
{
  if (rfbViewOnly || cl->viewOnly || rfbAuthDisableCBRecv)
    return;

  free(cl->clientClipboard);
  cl->clientClipboard = NULL;

  if (flags & rfbExtClipUTF8) {
    cl->hasLocalClipboard = FALSE;
    rfbHandleClipboardAnnounce(cl, TRUE);
  } else {
    rfbHandleClipboardAnnounce(cl, FALSE);
  }
}


/*
 * Handle an Extended Clipboard peek message from an RFB client.  This function
 * is called only by rfbReadExtClipboard().
 */

static void HandleClipboardPeek(rfbClientPtr cl)
{
  if (rfbViewOnly || cl->viewOnly || rfbAuthDisableCBSend)
    return;

  if (cl->clipFlags & rfbExtClipNotify)
    SendClipboardMessage(cl, rfbExtClipNotify,
                         cl->hasLocalClipboard ? rfbExtClipUTF8 : 0);
}


/*
 * Handle an Extended Clipboard provide message from an RFB client.  This
 * function is called only by rfbReadExtClipboard().
 */

static void HandleClipboardProvide(rfbClientPtr cl, CARD32 flags,
                                   const CARD32 *lengths, char *buffers[])
{
  if (!(flags & rfbExtClipUTF8)) {
    LogMessage(X_DEBUG, "Ignoring Extended Clipboard provide message with unsupported formats 0x%x\n",
               flags);
    return;
  }

  if (rfbViewOnly || cl->viewOnly || rfbAuthDisableCBRecv)
    return;

  free(cl->clientClipboard);

  cl->clientClipboard = ConvertLF(buffers[0], lengths[0]);

  LogMessage(X_DEBUG, "Received client clipboard: '%.*s%s' (%u bytes)\n",
             lengths[0] <= 20 ? (int)lengths[0] : 20, cl->clientClipboard,
             lengths[0] <= 20 ? "" : "...", lengths[0]);

  /* FIXME: Should probably verify that this data was actually requested */
  HandleClipboardData(cl, cl->clientClipboard);
}


/*
 * HandleClipboardRequest() is called whenever an RFB client requests clipboard
 * data from the server.  It will only be called after the server has first
 * announced a clipboard change via vncAnnounceClipboard().  This function is
 * called by rfbReadExtClipboard() and vncAnnounceClipboard().
 */

static void HandleClipboardRequest(rfbClientPtr cl, CARD32 flags)
{
  rfbClientPtr client;
  int size = 0;
  Bool alreadyInList = FALSE;

  if (flags) {
    if (!(flags & rfbExtClipUTF8)) {
      LogMessage(X_DEBUG, "Ignoring clipboard request for unsupported formats 0x%x\n",
                 flags);
      return;
    }
    if (!cl->hasLocalClipboard) {
      LogMessage(X_DEBUG, "Ignoring unexpected clipboard request\n");
      return;
    }
  }

  if (rfbViewOnly || cl->viewOnly || rfbAuthDisableCBSend)
    return;

  xorg_list_for_each_entry(client, &clipboardRequestors, entry) {
    if (client == cl) alreadyInList = TRUE;
  }
  if (!alreadyInList)
    xorg_list_append(&cl->entry, &clipboardRequestors);
  xorg_list_for_each_entry(client, &clipboardRequestors, entry)
    size++;
  if (size == 1)
    vncHandleClipboardRequest();
}


/*
 * Handle a standard RFB ClientCutText message from an RFB client.  Per the RFB
 * specification, only the ISO 8859-1 (AKA Latin-1) character set is supported.
 * This function is called only by the RFB server.
 */

void rfbHandleClientCutText(rfbClientPtr cl, const char *str, int len)
{
  char *filtered = ConvertLF(str, len);

  cl->hasLocalClipboard = FALSE;

  free(cl->clientClipboard);

  cl->clientClipboard = Latin1ToUTF8(filtered, (size_t)-1);
  free(filtered);

  if (cl->clientClipboard) {
    LogMessage(X_DEBUG, "Received client clipboard: '%.*s%s' (%d bytes)\n",
               len <= 20 ? len : 20, cl->clientClipboard,
               len <= 20 ? "" : "...", len);
    rfbHandleClipboardAnnounce(cl, TRUE);
  }
}


/*
 * rfbHandleClipboardAnnounce() is called whenever an RFB client's clipboard
 * changes.  vncRequestClipboard() should be called in order to access the
 * actual data.  This function is called by HandleClipboardNotify() and
 * rfbHandleClientCutText() and by the RFB server when a client disconnects.
 */

void rfbHandleClipboardAnnounce(rfbClientPtr cl, Bool available)
{
  if (rfbViewOnly || cl->viewOnly || rfbAuthDisableCBRecv)
    return;

  if (available)
    clipboardClient = cl;
  else {
    if (cl != clipboardClient)
      return;
    clipboardClient = NULL;
  }
  vncHandleClipboardAnnounce(available);
}


/*
 * rfbReadExtClipboard receives an Extended Clipboard message from a specific
 * RFB client.  This function is called only by the RFB server.
 */

void rfbReadExtClipboard(rfbClientPtr cl, int len)
{
  CARD32 action, flags, lengths[16];
  int err, i, num;
  z_stream zs;
  Bool zlibInit = FALSE;
  char *compressedBuf = NULL;
  char *uncompressedBufs[16] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  if (len < 4) {
    rfbLog("ERROR: Malformed Extended Clipboard message\n");
    goto bailout;
  }

  READ32_OR_CLOSE(flags, goto bailout);
  len -= sizeof(CARD32);
  action = flags & 0xFF000000;

  /* Because clipboard updates are zlib-compressed, we cannot truncate them
     without first reading and decompressing them.  Thus, as a first line of
     defense, we discard any Extended Clipboard messages that are larger than
     rfbMaxClipboard bytes.  We can safely assume that, if a compressed
     clipboard update is larger than rfbMaxClipboard bytes, then the
     uncompressed clipboard update will be larger still. */
  if (len > rfbMaxClipboard) {
    rfbLog("Ignoring %d-byte Extended Clipboard message (limit = %d bytes)\n",
           len, rfbMaxClipboard);
    SKIP_OR_CLOSE(len, goto bailout);
    return;
  }

  if (action & rfbExtClipCaps) {

    num = 0;
    for (i = 0; i < 16; i++) {
      if (flags & (1 << i))
        num++;
    }

    if (len < 4 * num) {
      rfbLog("ERROR: Malformed Extended Clipboard message\n");
      goto bailout;
    }

    num = 0;
    for (i = 0; i < 16; i++) {
      if (flags & (1 << i))
        READ32_OR_CLOSE(lengths[num++], goto bailout);
    }

    HandleClipboardCaps(cl, flags, lengths);
  } else if (action == rfbExtClipProvide) {
    int ignoredBytes = 0;

    if (len <= 0)
      goto bailout;

    memset(&zs, 0, sizeof(z_stream));
    if ((err = inflateInit(&zs)) != Z_OK) {
      ZLIB_ERROR(zs, err, inflateInit);
      goto bailout;
    }
    zlibInit = TRUE;

    zs.avail_in = len;
    compressedBuf = (char *)rfbAlloc(len);
    zs.next_in = (unsigned char *)compressedBuf;
    READ_OR_CLOSE(compressedBuf, len, goto bailout);

    num = 0;
    for (i = 0; i < 16; i++) {
      if (!(flags & 1 << i))
        continue;

      zs.avail_out = sizeof(CARD32);
      zs.next_out = (unsigned char *)&lengths[num];
      if ((err = inflate(&zs, Z_SYNC_FLUSH)) < 0) {
        ZLIB_ERROR(zs, err, inflate);
        goto bailout;
      }
      lengths[num] = Swap32IfLE(lengths[num]);

      if (lengths[num] <= 0) continue;

      /* As a second line of defense, if the overall size of the message is
         <= rfbMaxClipboard but the uncompressed length is > rfbMaxClipboard,
         then we decompress the clipboard update but truncate it to
         rfbMaxClipboard bytes.  If the clipboard update contains repeated
         character sequences, then it may have a compressed length
         < rfbMaxClipboard and an uncompressed length >> rfbMaxClipboard.
         Thus, to be safe, we decompress only the zlib data we need and discard
         the rest.  That effectively discards any non-UTF8 formats that might
         be contained in the same clipboard update, but we don't currently
         support those formats anyhow. */
      if (lengths[num] > rfbMaxClipboard) {
        rfbLog("Truncating %d-byte incoming clipboard update to %d bytes.\n",
               lengths[num], rfbMaxClipboard);
        ignoredBytes = lengths[num] - rfbMaxClipboard;
        lengths[num] = rfbMaxClipboard;
      }

      uncompressedBufs[num] = (char *)rfbAlloc(lengths[num]);

      zs.avail_out = lengths[num];
      zs.next_out = (unsigned char *)uncompressedBufs[num];
      if ((err = inflate(&zs, Z_SYNC_FLUSH)) < 0) {
        ZLIB_ERROR(zs, err, inflate);
        goto bailout;
      }

      if (ignoredBytes > 0) break;

      num++;
    }

    inflateEnd(&zs);  zlibInit = FALSE;
    free(compressedBuf);  compressedBuf = NULL;

    HandleClipboardProvide(cl, flags, lengths, uncompressedBufs);

    for (i = 0; i < 16; i++) {
      free(uncompressedBufs[i]);  uncompressedBufs[i] = NULL;
    }
  } else {
    switch (action) {
      case rfbExtClipRequest:
        HandleClipboardRequest(cl, flags);
        break;
      case rfbExtClipPeek:
        HandleClipboardPeek(cl);
        break;
      case rfbExtClipNotify:
        HandleClipboardNotify(cl, flags);
        break;
      default:
        rfbLog("ERROR: Invalid Extended Clipboard action\n");
    }
  }
  return;

  bailout:
  if (zlibInit) inflateEnd(&zs);
  free(compressedBuf);
  for (i = 0; i < 16; i++) free(uncompressedBufs[i]);
  rfbCloseClient(cl);
}


/*
 * Send the server's clipboard capabilities to an RFB client.  This function is
 * called only by the RFB server.
 */

void rfbSendClipboardCaps(rfbClientPtr cl, CARD32 caps, const CARD32 *lengths)
{
  size_t i, count;
  rfbServerCutTextMsg sct;

  if (!cl->enableExtClipboard)
    return;

  count = 0;
  for (i = 0; i < 16; i++) {
    if (caps & (1 << i))
      count++;
  }

  memset(&sct, 0, sz_rfbServerCutTextMsg);
  sct.type = rfbServerCutText;
  sct.length = Swap32IfLE((CARD32)(-(4 + 4 * count)));

  WRITE_OR_CLOSE(&sct, sz_rfbServerCutTextMsg, return);
  WRITE32_OR_CLOSE(caps | rfbExtClipCaps, return);

  count = 0;
  for (i = 0; i < 16; i++) {
    if (caps & (1 << i))
      WRITE32_OR_CLOSE(lengths[count++], return);
  }
}


/*
 * SendClipboardData() transfers the server's clipboard data to an RFB client
 * and should be called whenever the client has requested the clipboard data
 * via HandleClipboardRequest().  This function is called only by
 * vncSendClipboardData().
 */

static void SendClipboardData(rfbClientPtr cl, const char *data)
{
  rfbServerCutTextMsg sct;
  char *latin1;

  if (rfbViewOnly || cl->viewOnly || rfbAuthDisableCBSend ||
      cl->state != RFB_NORMAL || !data)
    return;

  if (cl->enableExtClipboard && cl->clipFlags & rfbExtClipProvide) {
    char *filtered;
    CARD32 lengths[1];
    const char *datas[1];

    filtered = ConvertCRLF(data, (size_t)-1);
    lengths[0] = strlen(filtered) + 1;
    datas[0] = filtered;

    if (cl->unsolicitedClipboardAttempt) {
      cl->unsolicitedClipboardAttempt = FALSE;
      if (lengths[0] > cl->clipSizes[rfbExtClipUTF8]) {
        LogMessage(X_DEBUG, "%u-byte clipboard was too large for unsolicited transfer\n",
                   lengths[0]);
        if (cl->clipFlags & rfbExtClipNotify)
          SendClipboardMessage(cl, rfbExtClipNotify, rfbExtClipUTF8);
        free(filtered);
        return;
      }
    }
    SendClipboardProvide(cl, rfbExtClipUTF8, lengths, datas);
    free(filtered);
    return;
  }

  latin1 = UTF8ToLatin1(data, (size_t)-1);

  memset(&sct, 0, sz_rfbServerCutTextMsg);
  sct.type = rfbServerCutText;
  sct.length = Swap32IfLE(strlen(latin1));

  WRITE_OR_CLOSE(&sct, sz_rfbServerCutTextMsg, return);
  WRITE_OR_CLOSE(latin1, strlen(latin1), return);
  if (cl->captureFD >= 0)
    rfbWriteCapture(cl->captureFD, latin1, strlen(latin1));

  free(latin1);
}


/*
 * SendClipboardMessage sends a clipboard notify, peek, or request message to a
 * specific RFB client.  This function is called by HandleClipboardPeek()
 * [notify], SendClipboardData() [notify], vncAnnounceClipboard() [notify], and
 * vncRequestClipboard() [request].
 */

static void SendClipboardMessage(rfbClientPtr cl, CARD32 msgType, CARD32 flags)
{
  rfbServerCutTextMsg sct;

  if (msgType != rfbExtClipNotify && msgType != rfbExtClipPeek &&
      msgType != rfbExtClipRequest)
    return;

  if (!cl->enableExtClipboard || !(cl->clipFlags & msgType))
    return;

  memset(&sct, 0, sz_rfbServerCutTextMsg);
  sct.type = rfbServerCutText;
  sct.length = Swap32IfLE((CARD32)(-4));
  WRITE_OR_CLOSE(&sct, sz_rfbServerCutTextMsg, return);
  WRITE32_OR_CLOSE(flags | msgType, return);
}


/*
 * SendClipboardProvide sends a clipboard provide message to a specific RFB
 * client.  This function is called only by SendClipboardData().
 */

static void SendClipboardProvide(rfbClientPtr cl, CARD32 flags,
                                 const CARD32 *lengths, const char *data[])
{
  int err, i, count;
  rfbServerCutTextMsg sct;
  Bool zlibInit = FALSE;
  z_stream zs;
  CARD32 totalLength = 0;
  char *compressedBuf = NULL;

  if (!cl->enableExtClipboard || !(cl->clipFlags & rfbExtClipProvide))
    return;

  count = 0;
  for (i = 0; i < 16; i++) {
    if (!(flags & (1 << i)))
      continue;
    totalLength += sizeof(CARD32) + lengths[count];
    count++;
  }

  memset(&zs, 0, sizeof(z_stream));
  if ((err = deflateInit(&zs, cl->zlibCompressLevel)) != Z_OK) {
    ZLIB_ERROR(zs, err, deflateInit);
    goto bailout;
  }

  compressedBuf = (char *)rfbAlloc(compressBound(totalLength));

  zs.next_out = (unsigned char *)compressedBuf;
  zs.avail_out = compressBound(totalLength);

  count = 0;
  for (i = 0; i < 16; i++) {
    CARD32 len;

    if (!(flags & (1 << i)))
      continue;

    len = Swap32IfLE(lengths[count]);
    zs.next_in = (unsigned char *)&len;
    zs.avail_in = sizeof(CARD32);

    if ((err = deflate(&zs, Z_SYNC_FLUSH)) < 0) {
      ZLIB_ERROR(zs, err, deflate);
      goto bailout;
    }

    zs.next_in = (unsigned char *)data[count];
    zs.avail_in = lengths[count];

    if ((err = deflate(&zs, Z_SYNC_FLUSH)) < 0) {
      ZLIB_ERROR(zs, err, deflate);
      goto bailout;
    }

    count++;
  }

  deflateEnd(&zs);  zlibInit = FALSE;

  memset(&sct, 0, sz_rfbServerCutTextMsg);
  sct.type = rfbServerCutText;
  sct.length =
    Swap32IfLE((CARD32)(-(4 + compressBound(totalLength) - zs.avail_out)));
  WRITE_OR_CLOSE(&sct, sz_rfbServerCutTextMsg, goto bailout);
  WRITE32_OR_CLOSE(flags | rfbExtClipProvide, goto bailout);
  WRITE_OR_CLOSE(compressedBuf, compressBound(totalLength) - zs.avail_out,
                 goto bailout);

  free(compressedBuf);
  return;

  bailout:
  free(compressedBuf);
  if (zlibInit) deflateEnd(&zs);
  rfbCloseClient(cl);
}


/*
 * This function is called only by vncExtensionInit() when initializing the VNC
 * X11 extension.
 */

void vncSelectionInit(void)
{
  xaPRIMARY = MakeAtom("PRIMARY", 7, TRUE);
  xaCLIPBOARD = MakeAtom("CLIPBOARD", 9, TRUE);
  xaTARGETS = MakeAtom("TARGETS", 7, TRUE);
  xaTIMESTAMP = MakeAtom("TIMESTAMP", 9, TRUE);
  xaSTRING = MakeAtom("STRING", 6, TRUE);
  xaTEXT = MakeAtom("TEXT", 4, TRUE);
  xaUTF8_STRING = MakeAtom("UTF8_STRING", 11, TRUE);

  if (!AddCallback(&SelectionCallback, vncSelectionCallback, 0))
    FatalError("Add SelectionCallback failed");
  if (!AddCallback(&ClientStateCallback, vncClientStateCallback, 0))
    FatalError("Add ClientStateCallback failed");

  xorg_list_init(&clipboardRequestors);
}


/*
 * vncAnnounceClipboard() informs all RFB clients that the server's clipboard
 * has changed.  A client may later request the clipboard data via
 * HandleClipboardRequest().  This function is called by vncHandleSelection()
 * and vncSelectionCallback().
 */

static void vncAnnounceClipboard(Bool available)
{
  rfbClientPtr cl, nextCl;

  xorg_list_del(&clipboardRequestors);

  if (rfbViewOnly || rfbAuthDisableCBSend)
    return;

  for (cl = rfbClientHead; cl; cl = nextCl) {
    nextCl = cl->next;

    if (cl->state != RFB_NORMAL || cl->viewOnly)
      continue;

    cl->hasLocalClipboard = available;
    cl->unsolicitedClipboardAttempt = FALSE;

    if (cl->enableExtClipboard) {
      /* Attempt an unsolicited transfer? */
      if (available && cl->clipSizes[rfbExtClipUTF8] > 0 &&
          cl->clipFlags & rfbExtClipProvide) {
        LogMessage(X_DEBUG, "Attempting unsolicited clipboard transfer...\n");
        cl->unsolicitedClipboardAttempt = TRUE;
        HandleClipboardRequest(cl, 0);
        continue;
      }

      if (cl->clipFlags & rfbExtClipNotify) {
        SendClipboardMessage(cl, rfbExtClipNotify,
                             available ? rfbExtClipUTF8 : 0);
        continue;
      }
    }

    if (available) HandleClipboardRequest(cl, 0);
  }
}


/*
 * Callback function that periodically updates the linked list of X11 clients
 * that should receive clipboard updates
 */

static void vncClientStateCallback(CallbackListPtr *l, void *d, void *p)
{
  ClientPtr client = ((NewClientInfoRec *)p)->client;

  if (client->clientState == ClientStateGone) {
    struct VncDataTarget **nextPtr = &vncDataTargetHead, *cur;

    for (cur = vncDataTargetHead; cur; cur = *nextPtr) {
      if (cur->client == client) {
        *nextPtr = cur->next;
        free(cur);
        continue;
      }
      nextPtr = &cur->next;
    }
  }
}


/*
 * This function is called by the X server whenever an X11 ConvertSelection
 * request is received from an X11 client and the VNC server owns the
 * selection.  It is also called by vncHandleClipboardData().
 */

int vncConvertSelection(ClientPtr client, Atom selection, Atom target,
                        Atom property, Window requestor, CARD32 time,
                        const char *data)
{
  Selection *pSel;
  WindowPtr pWin;
  int rc;
  Atom realProperty;
  xEvent event;

  if (data == NULL)
    LogMessage(X_DEBUG, "Selection request for %s (type %s)\n",
               NameForAtom(selection), NameForAtom(target));
  else
    LogMessage(X_DEBUG, "Sending data for selection request for %s (type %s)\n",
               NameForAtom(selection), NameForAtom(target));

  rc = dixLookupSelection(&pSel, selection, client, DixGetAttrAccess);
  if (rc != Success) return rc;

  /* We do not validate the time argument, because neither does
     dix/selection.c, and some clients (e.g. Qt) rely on this lenient
     behavior. */

  rc = dixLookupWindow(&pWin, requestor, client, DixSetAttrAccess);
  if (rc != Success) return rc;

  if (property != None) realProperty = property;
  else realProperty = target;

  if (target == xaTARGETS) {
    Atom targets[] = { xaTARGETS, xaTIMESTAMP, xaSTRING, xaTEXT,
                       xaUTF8_STRING };

    rc = dixChangeWindowProperty(serverClient, pWin, realProperty, XA_ATOM, 32,
                                 PropModeReplace,
                                 sizeof(targets) / sizeof(targets[0]), targets,
                                 TRUE);
    if (rc != Success) return rc;
  } else if (target == xaTIMESTAMP) {
    rc = dixChangeWindowProperty(serverClient, pWin, realProperty, XA_INTEGER,
                                 32, PropModeReplace, 1,
                                 &pSel->lastTimeChanged.milliseconds, TRUE);
    if (rc != Success) return rc;
  } else {
    if (data == NULL) {
      struct VncDataTarget *vdt;

      if (target != xaSTRING && target != xaTEXT && target != xaUTF8_STRING)
        return BadMatch;

      vdt = (struct VncDataTarget *)rfbAlloc0(sizeof(struct VncDataTarget));

      vdt->client = client;
      vdt->selection = selection;
      vdt->target = target;
      vdt->property = property;
      vdt->requestor = requestor;
      vdt->time = time;

      vdt->next = vncDataTargetHead;
      vncDataTargetHead = vdt;

      LogMessage(X_DEBUG, "Requesting clipboard data from client\n");

      vncRequestClipboard();

      return Success;
    } else {
      if (target == xaSTRING || target == xaTEXT) {
        char *latin1 = UTF8ToLatin1(data, (size_t)-1);

        rc = dixChangeWindowProperty(serverClient, pWin, realProperty,
                                     XA_STRING, 8, PropModeReplace,
                                     strlen(latin1), latin1, TRUE);
        free(latin1);
        if (rc != Success) return rc;
      } else if (target == xaUTF8_STRING) {
        rc = dixChangeWindowProperty(serverClient, pWin, realProperty,
                                     xaUTF8_STRING, 8, PropModeReplace,
                                     strlen(data), (char *)data, TRUE);
        if (rc != Success) return rc;
      } else {
        return BadMatch;
      }
    }
  }

  event.u.u.type = SelectionNotify;
  event.u.selectionNotify.time = time;
  event.u.selectionNotify.requestor = requestor;
  event.u.selectionNotify.selection = selection;
  event.u.selectionNotify.target = target;
  event.u.selectionNotify.property = property;
  WriteEventsToClient(client, 1, &event);
  return Success;
}


/*
 * This function is called by vncOwnSelection() and vncSelectionRequest().  It
 * creates a hidden X window that is used by the VNC server to transfer
 * clipboard updates from/to the X server.
 */

static int vncCreateSelectionWindow(void)
{
  ScreenPtr pScreen;
  int result;

  if (pSelectionWin) return Success;

  pScreen = screenInfo.screens[0];
  win = FakeClientID(0);
  pSelectionWin = CreateWindow(win, pScreen->root, 0, 0, 1, 1, 0, InputOnly, 0,
                               NULL, 0, serverClient, CopyFromParent, &result);
  if (!pSelectionWin) return result;

  if (!AddResource(pSelectionWin->drawable.id, RT_WINDOW, pSelectionWin))
    return BadAlloc;

  LogMessage(X_DEBUG, "Created selection window\n");

  return Success;
}


/*
 * This function is called only by the X server.  It returns the handle of the
 * VNC server's hidden selection window.
 */

Window vncGetSelectionWindow(void) { return win; }


/*
 * This function is called only by rfbHandleClipboardAnnounce().  It prepares
 * the X server to receive a clipboard update from an RFB client.
 */

static void vncHandleClipboardAnnounce(Bool available)
{
  /* The data has changed in some way, so whatever is in our cache is now
     stale. */
  free(cachedData);
  cachedData = NULL;

  if (available) {
    LogMessage(X_DEBUG, "Remote clipboard announced.  Grabbing local ownership.\n");

    if (rfbSyncPrimary) {
      if (vncOwnSelection(xaPRIMARY) != Success)
        LogMessage(X_ERROR, "Could not grab PRIMARY selection\n");
    }
    if (vncOwnSelection(xaCLIPBOARD) != Success)
      LogMessage(X_ERROR, "Could not grab CLIPBOARD selection\n");
  } else {
    struct VncDataTarget *next;

    if (pSelectionWin == NULL)
      return;

    LogMessage(X_DEBUG, "Remote clipboard lost.  Removing local ownership.\n");

    DeleteWindowFromAnySelections(pSelectionWin);

    /* Abort any pending transfer */
    while (vncDataTargetHead != NULL) {
      xEvent event;

      event.u.u.type = SelectionNotify;
      event.u.selectionNotify.time = vncDataTargetHead->time;
      event.u.selectionNotify.requestor = vncDataTargetHead->requestor;
      event.u.selectionNotify.selection = vncDataTargetHead->selection;
      event.u.selectionNotify.target = vncDataTargetHead->target;
      event.u.selectionNotify.property = None;
      WriteEventsToClient(vncDataTargetHead->client, 1, &event);

      next = vncDataTargetHead->next;
      free(vncDataTargetHead);
      vncDataTargetHead = next;
    }
  }
}


/*
 * This function is called only by HandleClipboardData().  It transfers a
 * clipboard update from an RFB client to multiple X11 clients.
 */

static void vncHandleClipboardData(const char *data)
{
  struct VncDataTarget *next;

  LogMessage(X_DEBUG, "Got remote clipboard data.  Sending to X11 clients.\n");

  free(cachedData);
  cachedData = strdup(data);

  while (vncDataTargetHead != NULL) {
    int rc;
    xEvent event;

    rc = vncConvertSelection(vncDataTargetHead->client,
                             vncDataTargetHead->selection,
                             vncDataTargetHead->target,
                             vncDataTargetHead->property,
                             vncDataTargetHead->requestor,
                             vncDataTargetHead->time,
                             cachedData);
    if (rc != Success) {
      event.u.u.type = SelectionNotify;
      event.u.selectionNotify.time = vncDataTargetHead->time;
      event.u.selectionNotify.requestor = vncDataTargetHead->requestor;
      event.u.selectionNotify.selection = vncDataTargetHead->selection;
      event.u.selectionNotify.target = vncDataTargetHead->target;
      event.u.selectionNotify.property = None;
      WriteEventsToClient(vncDataTargetHead->client, 1, &event);
    }

    next = vncDataTargetHead->next;
    free(vncDataTargetHead);
    vncDataTargetHead = next;
  }
}


/*
 * This function is called only by HandleClipboardRequest().
 */

static void vncHandleClipboardRequest(void)
{
  if (activeSelection == None) {
    LogMessage(X_DEBUG, "Got request for local clipboard, although no clipboard is active.\n");
    return;
  }

  LogMessage(X_DEBUG, "Got request for local clipboard.  Re-probing formats.\n");

  probing = FALSE;
  vncSelectionRequest(activeSelection, xaTARGETS);
}


/*
 * This function is called only by the X server when an X11 SelectionNotify
 * event is received for the VNC server's selection window.  It transfers a
 * clipboard update from an X11 client to multiple RFB clients.
 */

void vncHandleSelection(Atom selection, Atom target, Atom property,
                        Atom requestor, TimeStamp time)
{
  PropertyPtr prop;
  int rc;

  if ((rc = dixLookupProperty(&prop, pSelectionWin, property, serverClient,
                              DixReadAccess)) != Success) {
    LogMessage(X_ERROR, "dixLookupProperty() failed: %d\n", rc);
    return;
  }

  LogMessage(X_DEBUG, "Selection notification for %s (target %s, property %s, type %s)\n",
             NameForAtom(selection), NameForAtom(target),
             NameForAtom(property), NameForAtom(prop->type));

  if (target != property) return;

  if (target == xaTARGETS) {
    if (prop->format != 32 || prop->type != XA_ATOM)
      return;

    if (probing) {
      if (vncHasAtom(xaSTRING, (const Atom *)prop->data, prop->size) ||
          vncHasAtom(xaUTF8_STRING, (const Atom *)prop->data, prop->size)) {
        vncMaybeRequestCache();
        LogMessage(X_DEBUG, "Compatible format found.  Notifying clients.\n");
        activeSelection = selection;
        vncAnnounceClipboard(TRUE);
      }
    } else {
      if (vncHasAtom(xaUTF8_STRING, (const Atom *)prop->data, prop->size))
        vncSelectionRequest(selection, xaUTF8_STRING);
      else if (vncHasAtom(xaSTRING, (const Atom *)prop->data, prop->size))
        vncSelectionRequest(selection, xaSTRING);
    }
  } else if (target == xaSTRING) {
    char *filtered, *utf8;
    int len = prop->size;

    if (prop->format != 8 || prop->type != xaSTRING)
      return;

    if (len > rfbMaxClipboard) {
      rfbLog("Truncating %d-byte outgoing clipboard update to %d bytes.\n", len,
             rfbMaxClipboard);
      len = rfbMaxClipboard;
    }

    filtered = ConvertLF(prop->data, len);
    utf8 = Latin1ToUTF8(filtered, (size_t)-1);
    free(filtered);

    vncSendClipboardData(utf8);

    free(utf8);
  } else if (target == xaUTF8_STRING) {
    char *filtered;
    int len = prop->size;

    if (prop->format != 8 || prop->type != xaUTF8_STRING)
      return;

    if (len > rfbMaxClipboard) {
      rfbLog("Truncating %d-byte outgoing clipboard update to %d bytes.\n", len,
             rfbMaxClipboard);
      len = rfbMaxClipboard;
    }

    filtered = ConvertLF(prop->data, len);

    vncSendClipboardData(filtered);

    free(filtered);
  }
}


static Bool vncHasAtom(Atom atom, const Atom list[], size_t size)
{
  size_t i;

  for (i = 0; i < size; i++) {
    if (list[i] == atom)
      return TRUE;
  }

  return FALSE;
}


/*
 * This function is called by vncHandleSelection() and vncSelectionCallback().
 */

static void vncMaybeRequestCache(void)
{
  /* Telling an RFB client that we have clipboard data will likely mean that we
     can no longer request its clipboard data.  This is a problem, as we might
     initially own multiple selections, we now just lost one, and we still want
     to be able to service the other one.  Solve this by requesting the data
     from the client and caching it when we can't afford to lose it. */

  /* Already cached? */
  if (cachedData != NULL)
    return;

  if (!vncWeAreOwner(xaCLIPBOARD)) {
    if (!rfbSyncPrimary || !vncWeAreOwner(xaPRIMARY))
      return;
  }

  LogMessage(X_DEBUG, "Requesting clipboard data from client for caching\n");

  vncRequestClipboard();
}


/*
 * This function attempts to grab the specified X11 clipboard selection in
 * anticipation of receiving a clipboard update from an RFB client.  The code
 * in this function is based on ProcSetSelectionOwner() in dix/selection.c.
 * This function is called only by vncHandleClipboardAnnounce().
 */

static int vncOwnSelection(Atom selection)
{
  Selection *pSel;
  SelectionInfoRec info;
  int rc;

  if ((rc = vncCreateSelectionWindow()) != Success) {
    LogMessage(X_ERROR, "Could not create selection window: %d\n", rc);
    return rc;
  }

  /*
   * First, see if the selection is already set...
   */
  rc = dixLookupSelection(&pSel, selection, serverClient, DixSetAttrAccess);

  if (rc == Success) {
    /* If the timestamp in client's request is in the past relative
       to the time stamp indicating the last time the owner of the
       selection was set, do not set the selection, just return
       success. */
    if (pSel->client && (pSel->client != serverClient)) {
      xEvent event = {
        .u.selectionClear.time = currentTime.milliseconds,
        .u.selectionClear.window = pSel->window,
        .u.selectionClear.atom = pSel->selection
      };
      event.u.u.type = SelectionClear;
      WriteEventsToClient(pSel->client, 1, &event);
    }
  } else if (rc == BadMatch) {
    /*
     * It doesn't exist, so add it...
     */
    pSel = dixAllocateObjectWithPrivates(Selection, PRIVATE_SELECTION);
    if (!pSel)
      return BadAlloc;

    pSel->selection = selection;

    /* security creation/labeling check */
    rc = XaceHookSelectionAccess(serverClient, &pSel,
                                 DixCreateAccess | DixSetAttrAccess);
    if (rc != Success) {
      free(pSel);
      return rc;
    }

    pSel->next = CurrentSelections;
    CurrentSelections = pSel;
  } else
    return rc;

  pSel->lastTimeChanged = currentTime;
  pSel->window = win;
  pSel->pWin = pSelectionWin;
  pSel->client = serverClient;

  info.selection = pSel;
  info.client = serverClient;
  info.kind = SelectionSetOwner;
  CallCallbacks(&SelectionCallback, &info);

  LogMessage(X_DEBUG, "Grabbed %s selection\n", NameForAtom(selection));

  return Success;
}


/*
 * vncRequestClipboard() requests clipboard data from the clipboard client.
 * HandleClipboardData() will be called once the data is available.  This
 * function is called by vncConvertSelection() and vncMaybeRequestCache().
 */

static void vncRequestClipboard(void)
{
  rfbClientPtr cl;

  if (clipboardClient == NULL) {
    LogMessage(X_DEBUG, "Got request for client clipboard, but no client currently owns the clipboard.\n");
    return;
  }

  cl = clipboardClient;

  if (rfbViewOnly || cl->viewOnly || rfbAuthDisableCBRecv ||
      cl->state != RFB_NORMAL)
    return;

  if (cl->clientClipboard != NULL) {
    HandleClipboardData(cl, cl->clientClipboard);
    return;
  }

  if (cl->enableExtClipboard && cl->clipFlags & rfbExtClipRequest)
    SendClipboardMessage(cl, rfbExtClipRequest, rfbExtClipUTF8);
}


/*
 * Callback function that periodically checks for X11 clipboard ownership
 * changes.  If the VNC server's selection window loses ownership, then all RFB
 * clients are notified that the server's clipboard has changed.  For all other
 * ownership changes, a clipboard update is requested from the X server.
 */

static void vncSelectionCallback(CallbackListPtr *callbacks, pointer data,
                                 pointer args)
{
  SelectionInfoRec *info = (SelectionInfoRec *)args;

  if (info->selection->selection == activeSelection) {
    vncMaybeRequestCache();
    LogMessage(X_DEBUG, "Local clipboard lost.  Notifying clients.\n");
    activeSelection = None;
    vncAnnounceClipboard(FALSE);
  }

  if (info->kind != SelectionSetOwner || info->client == serverClient)
    return;

  LogMessage(X_DEBUG, "Selection owner change for %s\n",
             NameForAtom(info->selection->selection));

  if (info->selection->selection == xaCLIPBOARD ||
      (info->selection->selection == xaPRIMARY && rfbSyncPrimary)) {
    LogMessage(X_DEBUG, "Got clipboard notification.  Probing for formats.\n");
    probing = TRUE;
    vncSelectionRequest(info->selection->selection, xaTARGETS);
  }
}


/*
 * This function is called by vncHandleClipboardRequest(),
 * vncHandleSelection(), and vncSelectionCallback().  It requests a clipboard
 * update from the X server via the VNC server's selection window, creating the
 * window if necessary.
 */

static void vncSelectionRequest(Atom selection, Atom target)
{
  Selection *pSel;
  xEvent event;
  int rc;

  if ((rc = vncCreateSelectionWindow()) != Success) {
    LogMessage(X_ERROR, "Could not create selection window: %d\n", rc);
    return;
  }

  LogMessage(X_DEBUG, "Requesting %s for %s selection\n", NameForAtom(target),
             NameForAtom(selection));

  rc = dixLookupSelection(&pSel, selection, serverClient, DixGetAttrAccess);
  if (rc != Success) {
    LogMessage(X_ERROR, "dixLookupSelection() failed: %d\n", rc);
    return;
  }

  event.u.u.type = SelectionRequest;
  event.u.selectionRequest.owner = pSel->window;
  event.u.selectionRequest.time = currentTime.milliseconds;
  event.u.selectionRequest.requestor = win;
  event.u.selectionRequest.selection = selection;
  event.u.selectionRequest.target = target;
  event.u.selectionRequest.property = target;
  WriteEventsToClient(pSel->client, 1, &event);
}


/*
 * This function is called only by vncHandleSelection().  It transfers the
 * server's clipboard data to all RFB clients that have requested it.
 */

static void vncSendClipboardData(const char *data)
{
  rfbClientPtr client;
  int len;

  xorg_list_for_each_entry(client, &clipboardRequestors, entry)
    SendClipboardData(client, data);

  xorg_list_del(&clipboardRequestors);

  len = strlen(data) + 1;
  LogMessage(X_DEBUG, "Sent server clipboard: '%.*s%s' (%d bytes)\n",
             len <= 20 ? (int)len : 20, data, len <= 20 ? "" : "...", len);
}


/*
 * Does the VNC server own the specified X11 clipboard selection?
 */

static Bool vncWeAreOwner(Atom selection)
{
  Selection *pSel;
  int rc;

  rc = dixLookupSelection(&pSel, selection, serverClient, DixReadAccess);
  if (rc != Success) return FALSE;

  if (pSel->client != serverClient || pSel->window != win)
    return FALSE;

  return TRUE;
}
