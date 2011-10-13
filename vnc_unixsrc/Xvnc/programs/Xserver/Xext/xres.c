/*
   Copyright (c) 2002  XFree86 Inc
*/
/* $XFree86: xc/programs/Xserver/Xext/xres.c,v 1.7tsi Exp $ */
/* $XdotOrg: xc/programs/Xserver/Xext/xres.c,v 1.7 2005/07/03 08:53:36 daniels Exp $ */

#define NEED_EVENTS
#define NEED_REPLIES
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "swaprep.h"
#include <X11/extensions/XResproto.h>
#include "pixmapstr.h"
#include "modinit.h"

static int
ProcXResQueryVersion (ClientPtr client)
{
    REQUEST(xXResQueryVersionReq);
    xXResQueryVersionReply rep;
    CARD16 client_major, client_minor;  /* not used */

    REQUEST_SIZE_MATCH (xXResQueryVersionReq);

    client_major = stuff->client_major;
    client_minor = stuff->client_minor;
    (void) client_major;
    (void) client_minor;

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.server_major = XRES_MAJOR_VERSION;
    rep.server_minor = XRES_MINOR_VERSION;   
    if (client->swapped) { 
        int n;
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);     
        swaps(&rep.server_major, n);
        swaps(&rep.server_minor, n);
    }
    WriteToClient(client, sizeof (xXResQueryVersionReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXResQueryClients (ClientPtr client)
{
    /* REQUEST(xXResQueryClientsReq); */
    xXResQueryClientsReply rep;
    int *current_clients;
    int i, num_clients;

    REQUEST_SIZE_MATCH(xXResQueryClientsReq);

    current_clients = ALLOCATE_LOCAL((currentMaxClients - 1) * sizeof(int));

    num_clients = 0;
    for(i = 1; i < currentMaxClients; i++) {
       if(clients[i]) {
           current_clients[num_clients] = i;
           num_clients++;   
       }
    }

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.num_clients = num_clients;
    rep.length = rep.num_clients * sz_xXResClient >> 2;
    if (client->swapped) {
        int n;
        swaps (&rep.sequenceNumber, n);
        swapl (&rep.length, n);
        swapl (&rep.num_clients, n);
    }   
    WriteToClient (client, sizeof (xXResQueryClientsReply), (char *) &rep);

    if(num_clients) {
        xXResClient scratch;

        for(i = 0; i < num_clients; i++) {
            scratch.resource_base = clients[current_clients[i]]->clientAsMask;
            scratch.resource_mask = RESOURCE_ID_MASK;
        
            if(client->swapped) {
                register int n;
                swapl (&scratch.resource_base, n);
                swapl (&scratch.resource_mask, n);
            }
            WriteToClient (client, sz_xXResClient, (char *) &scratch);
        }
    }

    DEALLOCATE_LOCAL(current_clients);

    return (client->noClientException);
}


static void
ResFindAllRes (pointer value, XID id, RESTYPE type, pointer cdata)
{
    int *counts = (int *)cdata;

    counts[(type & TypeMask) - 1]++;
}

static int
ProcXResQueryClientResources (ClientPtr client)
{
    REQUEST(xXResQueryClientResourcesReq);
    xXResQueryClientResourcesReply rep;
    int i, clientID, num_types;
    int *counts;

    REQUEST_SIZE_MATCH(xXResQueryClientResourcesReq);

    clientID = CLIENT_ID(stuff->xid);

    /* we could remove the (clientID == 0) check if we wanted to allow
       probing the X-server's resource usage */
    if(!clientID || (clientID >= currentMaxClients) || !clients[clientID]) {
        client->errorValue = stuff->xid;
        return BadValue;
    }

    counts = ALLOCATE_LOCAL((lastResourceType + 1) * sizeof(int));

    memset(counts, 0, (lastResourceType + 1) * sizeof(int));

    FindAllClientResources(clients[clientID], ResFindAllRes, counts);

    num_types = 0;

    for(i = 0; i <= lastResourceType; i++) {
       if(counts[i]) num_types++;
    }

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.num_types = num_types;
    rep.length = rep.num_types * sz_xXResType >> 2;
    if (client->swapped) {
        int n;
        swaps (&rep.sequenceNumber, n);
        swapl (&rep.length, n);
        swapl (&rep.num_types, n);
    }   
    WriteToClient (client,sizeof(xXResQueryClientResourcesReply),(char*)&rep);

    if(num_types) {
        xXResType scratch;

        for(i = 0; i < lastResourceType; i++) {
            if(!counts[i]) continue;

            if(!ResourceNames[i + 1]) {
                char buf[40];
                snprintf(buf, sizeof(buf), "Unregistered resource %i", i + 1);
                RegisterResourceName(i + 1, buf);
            }

            scratch.resource_type = ResourceNames[i + 1];
            scratch.count = counts[i];

            if(client->swapped) {
                register int n;
                swapl (&scratch.resource_type, n);
                swapl (&scratch.count, n);
            }
            WriteToClient (client, sz_xXResType, (char *) &scratch);
        }
    }

    DEALLOCATE_LOCAL(counts);
    
    return (client->noClientException);
}

static void 
ResFindPixmaps (pointer value, XID id, pointer cdata)
{
   unsigned long *bytes = (unsigned long *)cdata;
   PixmapPtr pix = (PixmapPtr)value;

   *bytes += (pix->devKind * pix->drawable.height);
}

static int
ProcXResQueryClientPixmapBytes (ClientPtr client)
{
    REQUEST(xXResQueryClientPixmapBytesReq);
    xXResQueryClientPixmapBytesReply rep;
    int clientID;
    unsigned long bytes;

    REQUEST_SIZE_MATCH(xXResQueryClientPixmapBytesReq);

    clientID = CLIENT_ID(stuff->xid);

    /* we could remove the (clientID == 0) check if we wanted to allow
       probing the X-server's resource usage */
    if(!clientID || (clientID >= currentMaxClients) || !clients[clientID]) {
        client->errorValue = stuff->xid;
        return BadValue;
    }

    bytes = 0;

    FindClientResourcesByType(clients[clientID], RT_PIXMAP, ResFindPixmaps, 
                              (pointer)(&bytes));

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    rep.bytes = bytes;
#ifdef XSERVER64
    rep.bytes_overflow = bytes >> 32;
#else
    rep.bytes_overflow = 0;
#endif
    if (client->swapped) {
        int n;
        swaps (&rep.sequenceNumber, n);
        swapl (&rep.length, n);
        swapl (&rep.bytes, n);
        swapl (&rep.bytes_overflow, n);
    }
    WriteToClient (client,sizeof(xXResQueryClientPixmapBytesReply),(char*)&rep);

    return (client->noClientException);
}


static void
ResResetProc (ExtensionEntry *extEntry) { }

static int
ProcResDispatch (ClientPtr client)
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_XResQueryVersion:
        return ProcXResQueryVersion(client);
    case X_XResQueryClients:
        return ProcXResQueryClients(client);
    case X_XResQueryClientResources:
        return ProcXResQueryClientResources(client);
    case X_XResQueryClientPixmapBytes:
        return ProcXResQueryClientPixmapBytes(client);
    default: break;
    }

    return BadRequest;
}

static int
SProcXResQueryVersion (ClientPtr client)
{
    REQUEST(xXResQueryVersionReq);
    int n;

    REQUEST_SIZE_MATCH (xXResQueryVersionReq);
    swaps(&stuff->client_major,n);
    swaps(&stuff->client_minor,n);
    return ProcXResQueryVersion(client);
}

static int
SProcXResQueryClientResources (ClientPtr client)
{
    REQUEST(xXResQueryClientResourcesReq);
    int n;

    REQUEST_SIZE_MATCH (xXResQueryClientResourcesReq);
    swaps(&stuff->xid,n);
    return ProcXResQueryClientResources(client);
}

static int
SProcXResQueryClientPixmapBytes (ClientPtr client)
{
    REQUEST(xXResQueryClientPixmapBytesReq);
    int n;

    REQUEST_SIZE_MATCH (xXResQueryClientPixmapBytesReq);
    swaps(&stuff->xid,n);
    return ProcXResQueryClientPixmapBytes(client);
}

static int
SProcResDispatch (ClientPtr client)
{
    REQUEST(xReq);
    int n;

    swaps(&stuff->length,n);

    switch (stuff->data) {
    case X_XResQueryVersion:
        return SProcXResQueryVersion(client);
    case X_XResQueryClients:  /* nothing to swap */
        return ProcXResQueryClients(client);
    case X_XResQueryClientResources:
        return SProcXResQueryClientResources(client);
    case X_XResQueryClientPixmapBytes:
        return SProcXResQueryClientPixmapBytes(client);
    default: break;
    }

    return BadRequest;
}

void
ResExtensionInit(INITARGS)
{
    (void) AddExtension(XRES_NAME, 0, 0,
                            ProcResDispatch, SProcResDispatch,
                            ResResetProc, StandardMinorOpcode);

    RegisterResourceName(RT_NONE, "NONE");
    RegisterResourceName(RT_WINDOW, "WINDOW");
    RegisterResourceName(RT_PIXMAP, "PIXMAP");
    RegisterResourceName(RT_GC, "GC");
    RegisterResourceName(RT_FONT, "FONT");
    RegisterResourceName(RT_CURSOR, "CURSOR");
    RegisterResourceName(RT_COLORMAP, "COLORMAP");
    RegisterResourceName(RT_CMAPENTRY, "COLORMAP ENTRY");
    RegisterResourceName(RT_OTHERCLIENT, "OTHER CLIENT");
    RegisterResourceName(RT_PASSIVEGRAB, "PASSIVE GRAB");
}
