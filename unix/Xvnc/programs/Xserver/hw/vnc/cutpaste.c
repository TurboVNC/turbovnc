/*
 * cutpaste.c - routines to deal with cut & paste buffers / selection.
 */

/*
 *  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
 *  Copyright (C) 2014, 2017-2019 D. R. Commander.  All Rights Reserved.
 *  Copyright 2016-2017 Pierre Ossman for Cendio AB
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

/* The iconv() prototype in Solaris 10 expects const char **inbuf.  The iconv()
   prototype in Solaris 11, Linux, and FreeBSD expects char **inbuf. */
#ifdef __SUNPRO_C
#pragma error_messages(off, E_ARG_INCOMPATIBLE_WITH_ARG_L)
#elif defined(sun) && defined(__GNUC__)
/* No way to disable just the incompatible pointer type warning with GCC 4.x,
   so we have to disable all of them. */
#pragma GCC diagnostic warning "-w"
#endif

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
#include <iconv.h>
#include <errno.h>

/* #define CLIPBOARD_DEBUG */

extern int NumCurrentSelections;

Bool rfbSyncPrimary = TRUE;

static Bool inSetXCutText = FALSE;
static char *clientCutText = 0;
static int clientCutTextLen = 0;
static WindowPtr pWin = NULL;
static Window win = 0;
static Atom xaPRIMARY, xaCLIPBOARD, xaTARGETS, xaTIMESTAMP, xaSTRING, xaTEXT,
  xaUTF8_STRING;


static Bool vncHasAtom(Atom atom, const Atom list[], size_t size);
static int vncOwnSelection(Atom selection);
static void vncSelectionCallback(CallbackListPtr *callbacks, pointer data,
                                 pointer args);
static void vncSelectionRequest(Atom selection, Atom target);


/*
 * rfbSetXCutText sets the cut buffer to be the given string.  We also clear
 * the primary selection.  Ideally we'd like to set it to the same thing, but I
 * can't work out how to do that without some kind of helper X client.
 */

void rfbSetXCutText(char *str, int len)
{
  inSetXCutText = TRUE;
  dixChangeWindowProperty(serverClient, screenInfo.screens[0]->root,
                          XA_CUT_BUFFER0, XA_STRING, 8, PropModeReplace, len,
                          (pointer)str, TRUE);
  inSetXCutText = FALSE;
}


void rfbGotXCutText(char *str, int len)
{
  if (!inSetXCutText)
    rfbSendServerCutText(str, len);
}


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
}


void vncClientCutText(const char *str, int len)
{
  free(clientCutText);
  clientCutText = (char *)rfbAlloc(len);
  memcpy(clientCutText, str, len);
  clientCutTextLen = len;

  LogMessage(X_DEBUG, "Received client clipboard: '%.*s%s' (%d bytes)\n",
             clientCutTextLen <= 20 ? clientCutTextLen : 20, clientCutText,
             clientCutTextLen <= 20 ? "" : "...", clientCutTextLen);

  if (rfbSyncPrimary) {
    if (vncOwnSelection(XA_PRIMARY) != Success)
      LogMessage(X_ERROR, "Could not grab PRIMARY selection\n");
  }
  if (vncOwnSelection(xaCLIPBOARD) != Success)
    LogMessage(X_ERROR, "Could not grab CLIPBOARD selection\n");
}


int vncConvertSelection(ClientPtr client, Atom selection, Atom target,
                        Atom property, Window requestor, CARD32 time)
{
  Selection *pSel;
  WindowPtr pWin;
  int rc;
  Atom realProperty;
  xEvent event;

#ifdef CLIPBOARD_DEBUG
  LogMessage(X_DEBUG, "Selection request for %s (type %s)\n",
             NameForAtom(selection), NameForAtom(target));
#endif

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
                                 PropModeReplace, 5, targets, TRUE);
    if (rc != Success) return rc;
  } else if (target == xaTIMESTAMP) {
    rc = dixChangeWindowProperty(serverClient, pWin, realProperty, XA_INTEGER,
                                 32, PropModeReplace, 1,
                                 &pSel->lastTimeChanged.milliseconds, TRUE);
    if (rc != Success) return rc;
  } else if ((target == xaSTRING) || (target == xaTEXT)) {
    rc = dixChangeWindowProperty(serverClient, pWin, realProperty, XA_STRING,
                                 8, PropModeReplace, clientCutTextLen,
                                 clientCutText, TRUE);
    if (rc != Success) return rc;
  } else if (target == xaUTF8_STRING) {
    size_t inbytesleft = clientCutTextLen, outbytesleft = clientCutTextLen * 2;
    char *in = clientCutText, *buffer = rfbAlloc(outbytesleft), *out = buffer;
    iconv_t cd = (iconv_t)-1;

    if ((cd = iconv_open("UTF-8", "ISO-8859-1")) == (iconv_t)-1 ||
        iconv(cd, &in, &inbytesleft, &out, &outbytesleft) == (size_t)-1)
      LogMessage(X_ERROR, "ISO-8859-1 to UTF-8 conversion failed: %s\n",
                 strerror(errno));
    if (cd != (iconv_t)-1)
      iconv_close(cd);

    rc = dixChangeWindowProperty(serverClient, pWin, realProperty,
                                 xaUTF8_STRING, 8, PropModeReplace,
                                 clientCutTextLen * 2 - outbytesleft, buffer,
                                 TRUE);
    free(buffer);
    if (rc != Success) return rc;
  } else
    return BadMatch;

  event.u.u.type = SelectionNotify;
  event.u.selectionNotify.time = time;
  event.u.selectionNotify.requestor = requestor;
  event.u.selectionNotify.selection = selection;
  event.u.selectionNotify.target = target;
  event.u.selectionNotify.property = property;
  WriteEventsToClient(client, 1, &event);
  return Success;
}


static int vncCreateSelectionWindow(void)
{
  ScreenPtr pScreen;
  int result;

  if (pWin) return Success;

  pScreen = screenInfo.screens[0];
  win = FakeClientID(0);
  pWin = CreateWindow(win, pScreen->root, 0, 0, 1, 1, 0, InputOnly, 0, NULL, 0,
                      serverClient, CopyFromParent, &result);
  if (!pWin) return result;

  if (!AddResource(pWin->drawable.id, RT_WINDOW, pWin))
    return BadAlloc;

  return Success;
}


Window vncGetSelectionWindow(void) { return win; }


void vncHandleSelection(Atom selection, Atom target, Atom property,
                        Atom requestor, TimeStamp time)
{
  PropertyPtr prop;
  int rc;

  if ((rc = dixLookupProperty(&prop, pWin, property, serverClient,
                              DixReadAccess)) != Success) {
    LogMessage(X_ERROR, "dixLookupProperty() failed: %d\n", rc);
    return;
  }

#ifdef CLIPBOARD_DEBUG
  LogMessage(X_DEBUG, "Selection notification for %s (target %s, property %s, type %s)\n",
             NameForAtom(selection), NameForAtom(target),
             NameForAtom(property), NameForAtom(prop->type));
#endif

  if (target != property) return;

  if (target == xaTARGETS) {
    if (prop->format != 32 || prop->type != XA_ATOM)
      return;

    if (vncHasAtom(xaSTRING, (const Atom *)prop->data, prop->size))
      vncSelectionRequest(selection, xaSTRING);
    else if (vncHasAtom(xaUTF8_STRING, (const Atom *)prop->data, prop->size))
      vncSelectionRequest(selection, xaUTF8_STRING);
  } else if (target == xaSTRING) {
    if (prop->format != 8 || prop->type != xaSTRING)
      return;

    rfbSendServerCutText(prop->data, prop->size);
  } else if (target == xaUTF8_STRING) {
    size_t inbytesleft = prop->size, outbytesleft = prop->size;
    char *in = prop->data, *buffer, *out;
    iconv_t cd;

    if (prop->format != 8 || prop->type != xaUTF8_STRING)
      return;

    buffer = rfbAlloc(outbytesleft);
    out = buffer;
    if ((cd = iconv_open("ISO-8859-1", "UTF-8")) == (iconv_t)-1 ||
        iconv(cd, &in, &inbytesleft, &out, &outbytesleft) == (size_t)-1)
      LogMessage(X_ERROR, "UTF-8 to ISO-8859-1 conversion failed: %s\n",
                 strerror(errno));

    rfbSendServerCutText(buffer, prop->size - outbytesleft);
    free(buffer);
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


/* The code in this function is based on ProcSetSelectionOwner() in
   dix/selection.c. */

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
  pSel->pWin = pWin;
  pSel->client = serverClient;

  info.selection = pSel;
  info.client = serverClient;
  info.kind = SelectionSetOwner;
  CallCallbacks(&SelectionCallback, &info);

#ifdef CLIPBOARD_DEBUG
  LogMessage(X_DEBUG, "Grabbed %s selection\n", NameForAtom(selection));
#endif

  return Success;
}


static void vncSelectionCallback(CallbackListPtr *callbacks, pointer data,
                                 pointer args)
{
  SelectionInfoRec *info = (SelectionInfoRec *)args;

  if (info->kind != SelectionSetOwner || info->client == serverClient)
    return;

  if (info->selection->selection == xaCLIPBOARD ||
      (info->selection->selection == XA_PRIMARY && rfbSyncPrimary))
    vncSelectionRequest(info->selection->selection, xaTARGETS);
}


static void vncSelectionRequest(Atom selection, Atom target)
{
  Selection *pSel;
  xEvent event;
  int rc;

  if ((rc = vncCreateSelectionWindow()) != Success) {
    LogMessage(X_ERROR, "Could not create selection window: %d\n", rc);
    return;
  }

#ifdef CLIPBOARD_DEBUG
  LogMessage(X_DEBUG, "Requesting %s for %s selection\n", NameForAtom(target),
             NameForAtom(selection));
#endif

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
