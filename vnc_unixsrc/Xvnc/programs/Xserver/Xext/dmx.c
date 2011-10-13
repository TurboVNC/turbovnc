/* $XFree86$ */
/*
 * Copyright 2002-2004 Red Hat Inc., Durham, North Carolina.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL RED HAT AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * Authors:
 *   Rickard E. (Rik) Faith <faith@redhat.com>
 *
 */

/** \file
 * This file implements the server-side part of the DMX protocol. A
 * vector of fucntions is provided at extension initialization time, so
 * most all of the useful functions in this file are declared static and
 * do not appear in the doxygen documentation.
 *
 * Much of the low-level work is done by functions in #dmxextension.c
 *
 * Please see the Client-to-Server DMX Extension to the X Protocol
 * document for details about the protocol.  */

#ifdef HAVE_DMX_CONFIG_H
#include <dmx-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#define EXTENSION_PROC_ARGS void *
#include "extnsionst.h"
#include "opaque.h"

#include "dmxextension.h"
#include <X11/extensions/dmxproto.h>

#define _DMX_SERVER_
#include <X11/extensions/dmxext.h>

#ifdef PANORAMIX
#include "panoramiX.h"
extern unsigned long XRT_WINDOW;
extern int           PanoramiXNumScreens;
#endif

extern void DMXExtensionInit(void);

static unsigned char DMXCode;

static DISPATCH_PROC(ProcDMXDispatch);
static DISPATCH_PROC(ProcDMXQueryVersion);
static DISPATCH_PROC(ProcDMXSync);
static DISPATCH_PROC(ProcDMXForceWindowCreation);
static DISPATCH_PROC(ProcDMXGetScreenCount);
static DISPATCH_PROC(ProcDMXGetScreenAttributes);
static DISPATCH_PROC(ProcDMXChangeScreensAttributes);
static DISPATCH_PROC(ProcDMXAddScreen);
static DISPATCH_PROC(ProcDMXRemoveScreen);
static DISPATCH_PROC(ProcDMXGetWindowAttributes);
static DISPATCH_PROC(ProcDMXGetDesktopAttributes);
static DISPATCH_PROC(ProcDMXChangeDesktopAttributes);
static DISPATCH_PROC(ProcDMXGetInputCount);
static DISPATCH_PROC(ProcDMXGetInputAttributes);
static DISPATCH_PROC(ProcDMXAddInput);
static DISPATCH_PROC(ProcDMXRemoveInput);

static DISPATCH_PROC(SProcDMXDispatch);
static DISPATCH_PROC(SProcDMXQueryVersion);
static DISPATCH_PROC(SProcDMXSync);
static DISPATCH_PROC(SProcDMXForceWindowCreation);
static DISPATCH_PROC(SProcDMXGetScreenCount);
static DISPATCH_PROC(SProcDMXGetScreenAttributes);
static DISPATCH_PROC(SProcDMXChangeScreensAttributes);
static DISPATCH_PROC(SProcDMXAddScreen);
static DISPATCH_PROC(SProcDMXRemoveScreen);
static DISPATCH_PROC(SProcDMXGetWindowAttributes);
static DISPATCH_PROC(SProcDMXGetDesktopAttributes);
static DISPATCH_PROC(SProcDMXChangeDesktopAttributes);
static DISPATCH_PROC(SProcDMXGetInputCount);
static DISPATCH_PROC(SProcDMXGetInputAttributes);
static DISPATCH_PROC(SProcDMXAddInput);
static DISPATCH_PROC(SProcDMXRemoveInput);

static int _DMXXineramaActive(void)
{
#ifdef PANORAMIX
    return !noPanoramiXExtension;
#endif
    return 0;
}

static void DMXResetProc(ExtensionEntry *extEntry)
{
}

/** Initialize the extension. */
void DMXExtensionInit(void)
{
    ExtensionEntry *extEntry;
    
    if ((extEntry = AddExtension(DMX_EXTENSION_NAME, 0, 0,
                                 ProcDMXDispatch, SProcDMXDispatch,
                                 DMXResetProc, StandardMinorOpcode)))
	DMXCode = extEntry->base;
}

static void dmxSetScreenAttribute(int bit, DMXScreenAttributesPtr attr,
                                  CARD32 value)
{
    switch (1 << bit) {
    case DMXScreenWindowWidth:   attr->screenWindowWidth   = value; break;
    case DMXScreenWindowHeight:  attr->screenWindowHeight  = value; break;
    case DMXScreenWindowXoffset: attr->screenWindowXoffset = value; break;
    case DMXScreenWindowYoffset: attr->screenWindowYoffset = value; break;
    case DMXRootWindowWidth:     attr->rootWindowWidth     = value; break;
    case DMXRootWindowHeight:    attr->rootWindowHeight    = value; break;
    case DMXRootWindowXoffset:   attr->rootWindowXoffset   = value; break;
    case DMXRootWindowYoffset:   attr->rootWindowYoffset   = value; break;
    case DMXRootWindowXorigin:   attr->rootWindowXorigin   = value; break;
    case DMXRootWindowYorigin:   attr->rootWindowYorigin   = value; break;
    }
}

static int dmxFetchScreenAttributes(unsigned int mask,
                                    DMXScreenAttributesPtr attr,
                                    CARD32 *value_list)
{
    int    i;
    CARD32 *value = value_list;
    int    count  = 0;
        
    for (i = 0; i < 32; i++) {
        if (mask & (1 << i)) {
            dmxSetScreenAttribute(i, attr, *value);
            ++value;
            ++count;
        }
    }
    return count;
}

static void dmxSetDesktopAttribute(int bit, DMXDesktopAttributesPtr attr,
                                   CARD32 value)
{
    switch (1 << bit) {
    case DMXDesktopWidth:  attr->width  = value; break;
    case DMXDesktopHeight: attr->height = value; break;
    case DMXDesktopShiftX: attr->shiftX = value; break;
    case DMXDesktopShiftY: attr->shiftY = value; break;
    }
}

static int dmxFetchDesktopAttributes(unsigned int mask,
                                     DMXDesktopAttributesPtr attr,
                                     CARD32 *value_list)
{
    int    i;
    CARD32 *value = value_list;
    int    count  = 0;
        
    for (i = 0; i < 32; i++) {
        if (mask & (1 << i)) {
            dmxSetDesktopAttribute(i, attr, *value);
	    ++value;
            ++count;
        }
    }
    return count;
}

static void dmxSetInputAttribute(int bit, DMXInputAttributesPtr attr,
                                 CARD32 value)
{
    switch (1 << bit) {
    case DMXInputType:           attr->inputType      = value;   break;
    case DMXInputPhysicalScreen: attr->physicalScreen = value;   break;
    case DMXInputSendsCore:      attr->sendsCore      = !!value; break;
    }
}

static int dmxFetchInputAttributes(unsigned int mask,
                                   DMXInputAttributesPtr attr,
                                   CARD32 *value_list)
{
    int    i;
    CARD32 *value = value_list;
    int    count  = 0;

    for (i = 0; i < 32; i++) {
        if (mask & (1 << i)) {
            dmxSetInputAttribute(i, attr, *value);
            ++value;
            ++count;
        }
    }
    return count;
}

static int ProcDMXQueryVersion(ClientPtr client)
{
    xDMXQueryVersionReply rep;
    int                   n;

    REQUEST_SIZE_MATCH(xDMXQueryVersionReq);

    rep.type           = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length         = 0;
    rep.majorVersion   = DMX_EXTENSION_MAJOR;
    rep.minorVersion   = DMX_EXTENSION_MINOR;
    rep.patchVersion   = DMX_EXTENSION_PATCH;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
	swapl(&rep.majorVersion, n);
	swapl(&rep.minorVersion, n);
	swapl(&rep.patchVersion, n);
    }
    WriteToClient(client, sizeof(xDMXQueryVersionReply), (char *)&rep);
    return client->noClientException;
}

static int ProcDMXSync(ClientPtr client)
{
    xDMXSyncReply rep;
    int           n;

    REQUEST_SIZE_MATCH(xDMXSyncReq);

    dmxFlushPendingSyncs();

    rep.type           = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length         = 0;
    rep.status         = 0;
    if (client->swapped) {
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
        swapl(&rep.status, n);
    }
    WriteToClient(client, sizeof(xDMXSyncReply), (char *)&rep);
    return client->noClientException;
}

static int ProcDMXForceWindowCreation(ClientPtr client)
{
    xDMXForceWindowCreationReply rep;
    REQUEST(xDMXForceWindowCreationReq);
    WindowPtr     pWin;
    int           n;

    REQUEST_SIZE_MATCH(xDMXForceWindowCreationReq);

#ifdef PANORAMIX
    if (!noPanoramiXExtension) {
        PanoramiXRes *win;
        int          i;

        if (!(win = SecurityLookupIDByType(client, stuff->window, XRT_WINDOW,
                                           SecurityReadAccess)))
            return -1;           /* BadWindow */

        FOR_NSCREENS(i) {
            if (!(pWin = SecurityLookupWindow(win->info[i].id, client,
                                              SecurityReadAccess)))
                return -1;       /* BadWindow */

            dmxForceWindowCreation(pWin);
        }
        goto doreply;
    }
#endif

    if (!(pWin = SecurityLookupWindow(stuff->window, client,
                                      SecurityReadAccess)))
        return -1;               /* BadWindow */

    dmxForceWindowCreation(pWin);
  doreply:
    dmxFlushPendingSyncs();
    rep.type           = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length         = 0;
    rep.status         = 0;
    if (client->swapped) {
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
        swapl(&rep.status, n);
    }
    WriteToClient(client, sizeof(xDMXForceWindowCreationReply), (char *)&rep);
    return Success;
}

static int ProcDMXGetScreenCount(ClientPtr client)
{
    xDMXGetScreenCountReply rep;
    int                     n;

    REQUEST_SIZE_MATCH(xDMXGetScreenCountReq);

    rep.type           = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length         = 0;
    rep.screenCount    = dmxGetNumScreens();
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
        swapl(&rep.screenCount, n);
    }
    WriteToClient(client, sizeof(xDMXGetScreenCountReply), (char *)&rep);
    return client->noClientException;
}

static int ProcDMXGetScreenAttributes(ClientPtr client)
{
    REQUEST(xDMXGetScreenAttributesReq);
    xDMXGetScreenAttributesReply rep;
    int                          n;
    int                          length;
    int                          paddedLength;
    DMXScreenAttributesRec       attr;

    REQUEST_SIZE_MATCH(xDMXGetScreenAttributesReq);

    if (stuff->physicalScreen < 0
        || stuff->physicalScreen >= dmxGetNumScreens()) return BadValue;

    if (!dmxGetScreenAttributes(stuff->physicalScreen, &attr))
        return BadValue;

    rep.logicalScreen       = attr.logicalScreen;
    rep.screenWindowWidth   = attr.screenWindowWidth;
    rep.screenWindowHeight  = attr.screenWindowHeight;
    rep.screenWindowXoffset = attr.screenWindowXoffset;
    rep.screenWindowYoffset = attr.screenWindowYoffset;
    rep.rootWindowWidth     = attr.rootWindowWidth;
    rep.rootWindowHeight    = attr.rootWindowHeight;
    rep.rootWindowXoffset   = attr.rootWindowXoffset;
    rep.rootWindowYoffset   = attr.rootWindowYoffset;
    rep.rootWindowXorigin   = attr.rootWindowXorigin;
    rep.rootWindowYorigin   = attr.rootWindowYorigin;
                                 
    length                  = attr.displayName ? strlen(attr.displayName) : 0;
    paddedLength            = (length + 3) & ~3;
    rep.type                = X_Reply;
    rep.sequenceNumber      = client->sequence;
    rep.length              = paddedLength >> 2;
    rep.displayNameLength   = length;

    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
        swapl(&rep.displayNameLength, n);
        swapl(&rep.logicalScreen, n);
        swaps(&rep.screenWindowWidth, n);
        swaps(&rep.screenWindowHeight, n);
        swaps(&rep.screenWindowXoffset, n);
        swaps(&rep.screenWindowYoffset, n);
        swaps(&rep.rootWindowWidth, n);
        swaps(&rep.rootWindowHeight, n);
        swaps(&rep.rootWindowXoffset, n);
        swaps(&rep.rootWindowYoffset, n);
        swaps(&rep.rootWindowXorigin, n);
        swaps(&rep.rootWindowYorigin, n);
    }
    WriteToClient(client, sizeof(xDMXGetScreenAttributesReply), (char *)&rep);
    if (length) WriteToClient(client, length, (char *)attr.displayName);
    return client->noClientException;
}

static int ProcDMXChangeScreensAttributes(ClientPtr client)
{
    REQUEST(xDMXChangeScreensAttributesReq);
    xDMXChangeScreensAttributesReply rep;
    int                              n;
    int                              status = DMX_BAD_XINERAMA;
    unsigned int                     mask   = 0;
    unsigned int                     i;
    CARD32                           *screen_list;
    CARD32                           *mask_list;
    CARD32                           *value_list;
    DMXScreenAttributesPtr           attribs;
    int                              errorScreen = 0;
    unsigned int                     len;
    int                              ones = 0;
    

    REQUEST_AT_LEAST_SIZE(xDMXChangeScreensAttributesReq);
    len = client->req_len - (sizeof(xDMXChangeScreensAttributesReq) >> 2);
    if (len < stuff->screenCount + stuff->maskCount)
        return BadLength;

    screen_list = (CARD32 *)(stuff + 1);
    mask_list   = &screen_list[stuff->screenCount];
    value_list  = &mask_list[stuff->maskCount];

    for (i = 0; i < stuff->maskCount; i++) ones += Ones(mask_list[i]);
    if (len != stuff->screenCount + stuff->maskCount + ones)
        return BadLength;
    
    if (!_DMXXineramaActive()) goto noxinerama;

    if (!(attribs = ALLOCATE_LOCAL(stuff->screenCount * sizeof(*attribs))))
        return BadAlloc;

    for (i = 0; i < stuff->screenCount; i++) {
        int count;
        
        if (i < stuff->maskCount) mask = mask_list[i];
        dmxGetScreenAttributes(screen_list[i], &attribs[i]);
        count = dmxFetchScreenAttributes(mask, &attribs[i], value_list);
        value_list += count;
    }

#if PANORAMIX
    status = dmxConfigureScreenWindows(stuff->screenCount,
				       screen_list,
				       attribs,
				       &errorScreen);
#endif

    DEALLOCATE_LOCAL(attribs);

    if (status == BadValue) return status;

  noxinerama:
    rep.type           = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length         = 0;
    rep.status         = status;
    rep.errorScreen    = errorScreen;
    if (client->swapped) {
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
        swapl(&rep.status, n);
        swapl(&rep.errorScreen, n);
    }
    WriteToClient(client,
                  sizeof(xDMXChangeScreensAttributesReply),
                  (char *)&rep);
    return client->noClientException;
}

static int ProcDMXAddScreen(ClientPtr client)
{
    REQUEST(xDMXAddScreenReq);
    xDMXAddScreenReply     rep;
    int                    n;
    int                    status = 0;
    CARD32                 *value_list;
    DMXScreenAttributesRec attr;
    int                    count;
    char                   *name;
    int                    len;
    int                    paddedLength;

    REQUEST_AT_LEAST_SIZE(xDMXAddScreenReq);
    paddedLength = (stuff->displayNameLength + 3) & ~3;
    len          = client->req_len - (sizeof(xDMXAddScreenReq) >> 2);
    if (len != Ones(stuff->valueMask) + paddedLength/4)
        return BadLength;

    memset(&attr, 0, sizeof(attr));
    dmxGetScreenAttributes(stuff->physicalScreen, &attr);
    value_list = (CARD32 *)(stuff + 1);
    count      = dmxFetchScreenAttributes(stuff->valueMask, &attr, value_list);
    
    if (!(name = ALLOCATE_LOCAL(stuff->displayNameLength + 1 + 4)))
        return BadAlloc;
    memcpy(name, &value_list[count], stuff->displayNameLength);
    name[stuff->displayNameLength] = '\0';
    attr.displayName = name;

    status = dmxAttachScreen(stuff->physicalScreen, &attr);

    DEALLOCATE_LOCAL(name);

    rep.type           = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length         = 0;
    rep.status         = status;
    rep.physicalScreen = stuff->physicalScreen;
    if (client->swapped) {
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
        swapl(&rep.status, n);
        swapl(&rep.physicalScreen, n);
    }
    WriteToClient(client,
                  sizeof(xDMXAddScreenReply),
                  (char *)&rep);
    return client->noClientException;
}

static int ProcDMXRemoveScreen(ClientPtr client)
{
    REQUEST(xDMXRemoveScreenReq);
    xDMXRemoveScreenReply rep;
    int                   n;
    int                   status = 0;

    REQUEST_SIZE_MATCH(xDMXRemoveScreenReq);

    status = dmxDetachScreen(stuff->physicalScreen);

    rep.type           = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length         = 0;
    rep.status         = status;
    if (client->swapped) {
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
        swapl(&rep.status, n);
    }
    WriteToClient(client,
                  sizeof(xDMXRemoveScreenReply),
                  (char *)&rep);
    return client->noClientException;
}


#ifdef PANORAMIX
static int dmxPopulatePanoramiX(ClientPtr client, Window window,
                                CARD32 *screens, CARD32 *windows,
                                xRectangle *pos, xRectangle *vis)
{
    WindowPtr              pWin;
    PanoramiXRes           *win;
    int                    i;
    int                    count = 0;
    DMXWindowAttributesRec attr;
    
    if (!(win = SecurityLookupIDByType(client, window, XRT_WINDOW,
                                       SecurityReadAccess)))
        return -1;               /* BadWindow */
    
    FOR_NSCREENS(i) {
        if (!(pWin = SecurityLookupWindow(win->info[i].id, client,
                                          SecurityReadAccess)))
            return -1;          /* BadWindow */
        if (dmxGetWindowAttributes(pWin, &attr)) {
            screens[count] = attr.screen;
            windows[count] = attr.window;
            pos[count]     = attr.pos;
            vis[count]     = attr.vis;
            ++count;            /* Only count existing windows */
        }
    }
    return count;
}
#endif

static int dmxPopulate(ClientPtr client, Window window, CARD32 *screens,
                       CARD32 *windows, xRectangle *pos, xRectangle *vis)
{
    WindowPtr              pWin;
    DMXWindowAttributesRec attr;

#ifdef PANORAMIX
    if (!noPanoramiXExtension)
        return dmxPopulatePanoramiX(client, window, screens, windows,
                                    pos, vis);
#endif
    
    if (!(pWin = SecurityLookupWindow(window, client, SecurityReadAccess)))
        return -1;               /* BadWindow */

    dmxGetWindowAttributes(pWin, &attr);
    *screens = attr.screen;
    *windows = attr.window;
    *pos     = attr.pos;
    *vis     = attr.vis;
    return 1;
}

static int dmxMaxNumScreens(void)
{
#ifdef PANORAMIX
    if (!noPanoramiXExtension) return PanoramiXNumScreens;
#endif
    return 1;
}

static int ProcDMXGetWindowAttributes(ClientPtr client)
{
    REQUEST(xDMXGetWindowAttributesReq);
    xDMXGetWindowAttributesReply rep;
    int                          i, n;
    CARD32                       *screens;
    CARD32                       *windows;
    xRectangle                   *pos, *vis;
    int                          count = dmxMaxNumScreens();

    REQUEST_SIZE_MATCH(xDMXGetWindowAttributesReq);

    if (!(screens = ALLOCATE_LOCAL(count * sizeof(*screens))))
        return BadAlloc;
    if (!(windows = ALLOCATE_LOCAL(count * sizeof(*windows)))) {
        DEALLOCATE_LOCAL(screens);
        return BadAlloc;
    }
    if (!(pos = ALLOCATE_LOCAL(count * sizeof(*pos)))) {
        DEALLOCATE_LOCAL(windows);
        DEALLOCATE_LOCAL(screens);
        return BadAlloc;
    }
    if (!(vis = ALLOCATE_LOCAL(count * sizeof(*vis)))) {
        DEALLOCATE_LOCAL(pos);
        DEALLOCATE_LOCAL(windows);
        DEALLOCATE_LOCAL(screens);
        return BadAlloc;
    }

    if ((count = dmxPopulate(client, stuff->window, screens, windows,
                             pos, vis)) < 0) {
        DEALLOCATE_LOCAL(vis);
        DEALLOCATE_LOCAL(pos);
        DEALLOCATE_LOCAL(windows);
        DEALLOCATE_LOCAL(screens);
        return BadWindow;
    }

    rep.type           = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length         = count * 6;
    rep.screenCount    = count;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
        swapl(&rep.screenCount, n);
        for (i = 0; i < count; i++) {
            swapl(&screens[i], n);
            swapl(&windows[i], n);
            
            swaps(&pos[i].x, n);
            swaps(&pos[i].y, n);
            swaps(&pos[i].width, n);
            swaps(&pos[i].height, n);
            
            swaps(&vis[i].x, n);
            swaps(&vis[i].y, n);
            swaps(&vis[i].width, n);
            swaps(&vis[i].height, n);
        }
    }

    dmxFlushPendingSyncs();

    WriteToClient(client, sizeof(xDMXGetWindowAttributesReply), (char *)&rep);
    if (count) {
        WriteToClient(client, count * sizeof(*screens), (char *)screens);
        WriteToClient(client, count * sizeof(*windows), (char *)windows);
        WriteToClient(client, count * sizeof(*pos),     (char *)pos);
        WriteToClient(client, count * sizeof(*vis),     (char *)vis);
    }

    DEALLOCATE_LOCAL(vis);
    DEALLOCATE_LOCAL(pos);
    DEALLOCATE_LOCAL(windows);
    DEALLOCATE_LOCAL(screens);

    return client->noClientException;
}

static int ProcDMXGetDesktopAttributes(ClientPtr client)
{
    xDMXGetDesktopAttributesReply rep;
    int                           n;
    DMXDesktopAttributesRec       attr;

    REQUEST_SIZE_MATCH(xDMXGetDesktopAttributesReq);

    dmxGetDesktopAttributes(&attr);

    rep.width               = attr.width;
    rep.height              = attr.height;
    rep.shiftX              = attr.shiftX;
    rep.shiftY              = attr.shiftY;

    rep.type                = X_Reply;
    rep.sequenceNumber      = client->sequence;
    rep.length              = 0;

    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
        swapl(&rep.width, n);
        swapl(&rep.height, n);
        swapl(&rep.shiftX, n);
        swapl(&rep.shiftY, n);
    }
    WriteToClient(client, sizeof(xDMXGetDesktopAttributesReply), (char *)&rep);
    return client->noClientException;
}

static int ProcDMXChangeDesktopAttributes(ClientPtr client)
{
    REQUEST(xDMXChangeDesktopAttributesReq);
    xDMXChangeDesktopAttributesReply rep;
    int                              n;
    int                              status = DMX_BAD_XINERAMA;
    CARD32                           *value_list;
    DMXDesktopAttributesRec          attr;
    int                              len;

    REQUEST_AT_LEAST_SIZE(xDMXChangeDesktopAttributesReq);
    len = client->req_len - (sizeof(xDMXChangeDesktopAttributesReq) >> 2);
    if (len != Ones(stuff->valueMask))
        return BadLength;

    if (!_DMXXineramaActive()) goto noxinerama;

    value_list = (CARD32 *)(stuff + 1);
    
    dmxGetDesktopAttributes(&attr);
    dmxFetchDesktopAttributes(stuff->valueMask, &attr, value_list);

#if PANORAMIX
    status = dmxConfigureDesktop(&attr);
#endif
    if (status == BadValue) return status;

  noxinerama:
    rep.type           = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length         = 0;
    rep.status         = status;
    if (client->swapped) {
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
        swapl(&rep.status, n);
    }
    WriteToClient(client,
                  sizeof(xDMXChangeDesktopAttributesReply),
                  (char *)&rep);
    return client->noClientException;
}

static int ProcDMXGetInputCount(ClientPtr client)
{
    xDMXGetInputCountReply rep;
    int                     n;

    REQUEST_SIZE_MATCH(xDMXGetInputCountReq);

    rep.type           = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length         = 0;
    rep.inputCount     = dmxGetInputCount();
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
        swapl(&rep.inputCount, n);
    }
    WriteToClient(client, sizeof(xDMXGetInputCountReply), (char *)&rep);
    return client->noClientException;
}

static int ProcDMXGetInputAttributes(ClientPtr client)
{
    REQUEST(xDMXGetInputAttributesReq);
    xDMXGetInputAttributesReply rep;
    int                          n;
    int                          length;
    int                          paddedLength;
    DMXInputAttributesRec        attr;

    REQUEST_SIZE_MATCH(xDMXGetInputAttributesReq);

    if (dmxGetInputAttributes(stuff->deviceId, &attr)) return BadValue;
    rep.inputType      = attr.inputType;
    rep.physicalScreen = attr.physicalScreen;
    rep.physicalId     = attr.physicalId;
    rep.isCore         = attr.isCore;
    rep.sendsCore      = attr.sendsCore;
    rep.detached       = attr.detached;
    
    length             = attr.name ? strlen(attr.name) : 0;
    paddedLength       = (length + 3) & ~3;
    rep.type           = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length         = paddedLength >> 2;
    rep.nameLength     = length;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
        swapl(&rep.inputType, n);
        swapl(&rep.physicalScreen, n);
        swapl(&rep.physicalId, n);
        swapl(&rep.nameLength, n);
    }
    WriteToClient(client, sizeof(xDMXGetInputAttributesReply), (char *)&rep);
    if (length) WriteToClient(client, length, (char *)attr.name);
    return client->noClientException;
}

static int ProcDMXAddInput(ClientPtr client)
{
    REQUEST(xDMXAddInputReq);
    xDMXAddInputReply      rep;
    int                    n;
    int                    status = 0;
    CARD32                 *value_list;
    DMXInputAttributesRec  attr;
    int                    count;
    char                   *name;
    int                    len;
    int                    paddedLength;
    int                    id     = -1;

    REQUEST_AT_LEAST_SIZE(xDMXAddInputReq);
    paddedLength = (stuff->displayNameLength + 3) & ~3;
    len          = client->req_len - (sizeof(xDMXAddInputReq) >> 2);
    if (len != Ones(stuff->valueMask) + paddedLength/4)
        return BadLength;

    memset(&attr, 0, sizeof(attr));
    value_list = (CARD32 *)(stuff + 1);
    count      = dmxFetchInputAttributes(stuff->valueMask, &attr, value_list);
    
    if (!(name = ALLOCATE_LOCAL(stuff->displayNameLength + 1 + 4)))
        return BadAlloc;
    memcpy(name, &value_list[count], stuff->displayNameLength);
    name[stuff->displayNameLength] = '\0';
    attr.name = name;

    status = dmxAddInput(&attr, &id);

    DEALLOCATE_LOCAL(name);

    if (status) return status;

    rep.type           = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length         = 0;
    rep.status         = status;
    rep.physicalId     = id;
    if (client->swapped) {
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
        swapl(&rep.status, n);
        swapl(&rep.physicalId, n);
    }
    WriteToClient(client, sizeof(xDMXAddInputReply), (char *)&rep);
    return client->noClientException;
}

static int ProcDMXRemoveInput(ClientPtr client)
{
    REQUEST(xDMXRemoveInputReq);
    xDMXRemoveInputReply     rep;
    int                      n;
    int                      status = 0;

    REQUEST_SIZE_MATCH(xDMXRemoveInputReq);

    status = dmxRemoveInput(stuff->physicalId);

    if (status) return status;

    rep.type           = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length         = 0;
    rep.status         = status;
    if (client->swapped) {
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
        swapl(&rep.status, n);
    }
    WriteToClient(client, sizeof(xDMXRemoveInputReply), (char *)&rep);
    return client->noClientException;
}

static int ProcDMXDispatch(ClientPtr client)
{
    REQUEST(xReq);

    switch (stuff->data) {
    case X_DMXQueryVersion:         return ProcDMXQueryVersion(client);
    case X_DMXSync:                 return ProcDMXSync(client);
    case X_DMXForceWindowCreation:  return ProcDMXForceWindowCreation(client);
    case X_DMXGetScreenCount:       return ProcDMXGetScreenCount(client);
    case X_DMXGetScreenAttributes:  return ProcDMXGetScreenAttributes(client);
    case X_DMXChangeScreensAttributes:
        return ProcDMXChangeScreensAttributes(client);
    case X_DMXAddScreen:            return ProcDMXAddScreen(client);
    case X_DMXRemoveScreen:         return ProcDMXRemoveScreen(client);
    case X_DMXGetWindowAttributes:  return ProcDMXGetWindowAttributes(client);
    case X_DMXGetDesktopAttributes: return ProcDMXGetDesktopAttributes(client);
    case X_DMXChangeDesktopAttributes:
        return ProcDMXChangeDesktopAttributes(client);
    case X_DMXGetInputCount:        return ProcDMXGetInputCount(client);
    case X_DMXGetInputAttributes:   return ProcDMXGetInputAttributes(client);
    case X_DMXAddInput:             return ProcDMXAddInput(client);
    case X_DMXRemoveInput:          return ProcDMXRemoveInput(client);
        
    case X_DMXGetScreenInformationDEPRECATED:
    case X_DMXForceWindowCreationDEPRECATED:
    case X_DMXReconfigureScreenDEPRECATED:
        return BadImplementation;

    default:                        return BadRequest;
    }
}

static int SProcDMXQueryVersion(ClientPtr client)
{
    int n;
    REQUEST(xDMXQueryVersionReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDMXQueryVersionReq);
    return ProcDMXQueryVersion(client);
}

static int SProcDMXSync(ClientPtr client)
{
    int n;
    REQUEST(xDMXSyncReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDMXSyncReq);
    return ProcDMXSync(client);
}

static int SProcDMXForceWindowCreation(ClientPtr client)
{
    int n;
    REQUEST(xDMXForceWindowCreationReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDMXForceWindowCreationReq);
    swaps(&stuff->window, n);
    return ProcDMXForceWindowCreation(client);
}

static int SProcDMXGetScreenCount(ClientPtr client)
{
    int n;
    REQUEST(xDMXGetScreenCountReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDMXGetScreenCountReq);
    return ProcDMXGetScreenCount(client);
}

static int SProcDMXGetScreenAttributes(ClientPtr client)
{
    int n;
    REQUEST(xDMXGetScreenAttributesReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDMXGetScreenAttributesReq);
    swapl(&stuff->physicalScreen, n);
    return ProcDMXGetScreenAttributes(client);
}

static int SProcDMXChangeScreensAttributes(ClientPtr client)
{
    int n;
    REQUEST(xDMXChangeScreensAttributesReq);

    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xDMXGetScreenAttributesReq);
    swapl(&stuff->screenCount, n);
    swapl(&stuff->maskCount, n);
    SwapRestL(stuff);
    return ProcDMXGetScreenAttributes(client);
}

static int SProcDMXAddScreen(ClientPtr client)
{
    int n;
    int paddedLength;
    REQUEST(xDMXAddScreenReq);

    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xDMXAddScreenReq);
    swapl(&stuff->displayNameLength, n);
    swapl(&stuff->valueMask, n);
    paddedLength = (stuff->displayNameLength + 3) & ~3;
    SwapLongs((CARD32 *)(stuff+1), LengthRestL(stuff) - paddedLength/4);
    return ProcDMXAddScreen(client);
}

static int SProcDMXRemoveScreen(ClientPtr client)
{
    int n;
    REQUEST(xDMXRemoveScreenReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDMXRemoveScreenReq);
    swapl(&stuff->physicalScreen, n);
    return ProcDMXRemoveScreen(client);
}

static int SProcDMXGetWindowAttributes(ClientPtr client)
{
    int n;
    REQUEST(xDMXGetWindowAttributesReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDMXGetWindowAttributesReq);
    swapl(&stuff->window, n);
    return ProcDMXGetWindowAttributes(client);
}

static int SProcDMXGetDesktopAttributes(ClientPtr client)
{
    int n;
    REQUEST(xDMXGetDesktopAttributesReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDMXGetDesktopAttributesReq);
    return ProcDMXGetDesktopAttributes(client);
}

static int SProcDMXChangeDesktopAttributes(ClientPtr client)
{
    int n;
    REQUEST(xDMXChangeDesktopAttributesReq);

    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xDMXChangeDesktopAttributesReq);
    swapl(&stuff->valueMask, n);
    SwapRestL(stuff);
    return ProcDMXChangeDesktopAttributes(client);
}

static int SProcDMXGetInputCount(ClientPtr client)
{
    int n;
    REQUEST(xDMXGetInputCountReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDMXGetInputCountReq);
    return ProcDMXGetInputCount(client);
}

static int SProcDMXGetInputAttributes(ClientPtr client)
{
    int n;
    REQUEST(xDMXGetInputAttributesReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDMXGetInputAttributesReq);
    swapl(&stuff->deviceId, n);
    return ProcDMXGetInputAttributes(client);
}

static int SProcDMXAddInput(ClientPtr client)
{
    int n;
    int paddedLength;
    REQUEST(xDMXAddInputReq);

    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xDMXAddInputReq);
    swapl(&stuff->displayNameLength, n);
    swapl(&stuff->valueMask, n);
    paddedLength = (stuff->displayNameLength + 3) & ~3;
    SwapLongs((CARD32 *)(stuff+1), LengthRestL(stuff) - paddedLength/4);
    return ProcDMXAddInput(client);
}

static int SProcDMXRemoveInput(ClientPtr client)
{
    int n;
    REQUEST(xDMXRemoveInputReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDMXRemoveInputReq);
    swapl(&stuff->physicalId, n);
    return ProcDMXRemoveInput(client);
}

static int SProcDMXDispatch (ClientPtr client)
{
    REQUEST(xReq);

    switch (stuff->data) {
    case X_DMXQueryVersion:         return SProcDMXQueryVersion(client);
    case X_DMXSync:                 return SProcDMXSync(client);
    case X_DMXForceWindowCreation:  return SProcDMXForceWindowCreation(client);
    case X_DMXGetScreenCount:       return SProcDMXGetScreenCount(client);
    case X_DMXGetScreenAttributes:  return SProcDMXGetScreenAttributes(client);
    case X_DMXChangeScreensAttributes:
        return SProcDMXChangeScreensAttributes(client);
    case X_DMXAddScreen:            return SProcDMXAddScreen(client);
    case X_DMXRemoveScreen:         return SProcDMXRemoveScreen(client);
    case X_DMXGetWindowAttributes:  return SProcDMXGetWindowAttributes(client);
    case X_DMXGetDesktopAttributes:
        return SProcDMXGetDesktopAttributes(client);
    case X_DMXChangeDesktopAttributes:
        return SProcDMXChangeDesktopAttributes(client);
    case X_DMXGetInputCount:        return SProcDMXGetInputCount(client);
    case X_DMXGetInputAttributes:   return SProcDMXGetInputAttributes(client);
    case X_DMXAddInput:             return SProcDMXAddInput(client);
    case X_DMXRemoveInput:          return SProcDMXRemoveInput(client);
        
    case X_DMXGetScreenInformationDEPRECATED:
    case X_DMXForceWindowCreationDEPRECATED:
    case X_DMXReconfigureScreenDEPRECATED:
        return BadImplementation;

    default:                        return BadRequest;
    }
}
