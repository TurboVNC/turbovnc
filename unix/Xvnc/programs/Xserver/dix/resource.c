/************************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

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

/* $XConsortium: resource.c /main/39 1996/10/30 11:17:56 rws $ */
/* $XFree86: xc/programs/Xserver/dix/resource.c,v 3.1 1996/12/23 06:29:51 dawes Exp $ */

/*	Routines to manage various kinds of resources:
 *
 *	CreateNewResourceType, CreateNewResourceClass, InitClientResources,
 *	FakeClientID, AddResource, FreeResource, FreeClientResources,
 *	FreeAllResources, LookupIDByType, LookupIDByClass, GetXIDRange
 */

/* 
 *      A resource ID is a 32 bit quantity, the upper 3 bits of which are
 *	off-limits for client-visible resources.  The next 7 bits are
 *      used as client ID, and the low 22 bits come from the client.
 *	A resource ID is "hashed" by extracting and xoring subfields
 *      (varying with the size of the hash table).
 *
 *      It is sometimes necessary for the server to create an ID that looks
 *      like it belongs to a client.  This ID, however,  must not be one
 *      the client actually can create, or we have the potential for conflict.
 *      The 30th bit of the ID is reserved for the server's use for this
 *      purpose.  By setting CLIENT_ID(id) to the client, the SERVER_BIT to
 *      1, and an otherwise arbitrary ID in the low 22 bits, we can create a
 *      resource "owned" by the client.
 */

#define NEED_EVENTS
#include "X.h"
#include "misc.h"
#include "os.h"
#include "resource.h"
#include "dixstruct.h" 
#include "opaque.h"
#include "windowstr.h"
#include "inputstr.h"
#include "dixfont.h"
#include "dixevents.h"
#include "dixgrabs.h"
#include "colormap.h"
#include "cursor.h"
#include <assert.h>

extern WindowPtr *WindowTable;

static void RebuildTable(
#if NeedFunctionPrototypes
    int /*client*/
#endif
);

#define SERVER_MINID 32

#define INITBUCKETS 64
#define INITHASHSIZE 6
#define MAXHASHSIZE 11

typedef struct _Resource {
    struct _Resource	*next;
    XID			id;
    RESTYPE		type;
    pointer		value;
} ResourceRec, *ResourcePtr;
#define NullResource ((ResourcePtr)NULL)

typedef struct _ClientResource {
    ResourcePtr *resources;
    int		elements;
    int		buckets;
    int		hashsize;	/* log(2)(buckets) */
    XID		fakeID;
    XID		endFakeID;
    XID		expectID;
} ClientResourceRec;

static RESTYPE lastResourceType;
static RESTYPE lastResourceClass;
static RESTYPE TypeMask;

static DeleteType *DeleteFuncs = (DeleteType *)NULL;

RESTYPE
CreateNewResourceType(deleteFunc)
    DeleteType deleteFunc;
{
    RESTYPE next = lastResourceType + 1;
    DeleteType *funcs;

    if (next & lastResourceClass)
	return 0;
    funcs = (DeleteType *)xrealloc(DeleteFuncs,
				   (next + 1) * sizeof(DeleteType));
    if (!funcs)
	return 0;
    lastResourceType = next;
    DeleteFuncs = funcs;
    DeleteFuncs[next] = deleteFunc;
    return next;
}

RESTYPE
CreateNewResourceClass()
{
    RESTYPE next = lastResourceClass >> 1;

    if (next & lastResourceType)
	return 0;
    lastResourceClass = next;
    TypeMask = next - 1;
    return next;
}

ClientResourceRec clientTable[MAXCLIENTS];

/*****************
 * InitClientResources
 *    When a new client is created, call this to allocate space
 *    in resource table
 *****************/

Bool
InitClientResources(client)
    ClientPtr client;
{
    register int i, j;
 
    if (client == serverClient)
    {
	lastResourceType = RT_LASTPREDEF;
	lastResourceClass = RC_LASTPREDEF;
	TypeMask = RC_LASTPREDEF - 1;
	if (DeleteFuncs)
	    xfree(DeleteFuncs);
	DeleteFuncs = (DeleteType *)xalloc((lastResourceType + 1) *
					   sizeof(DeleteType));
	if (!DeleteFuncs)
	    return FALSE;
	DeleteFuncs[RT_NONE & TypeMask] = (DeleteType)NoopDDA;
	DeleteFuncs[RT_WINDOW & TypeMask] = DeleteWindow;
	DeleteFuncs[RT_PIXMAP & TypeMask] = dixDestroyPixmap;
	DeleteFuncs[RT_GC & TypeMask] = FreeGC;
	DeleteFuncs[RT_FONT & TypeMask] = CloseFont;
	DeleteFuncs[RT_CURSOR & TypeMask] = FreeCursor;
	DeleteFuncs[RT_COLORMAP & TypeMask] = FreeColormap;
	DeleteFuncs[RT_CMAPENTRY & TypeMask] = FreeClientPixels;
	DeleteFuncs[RT_OTHERCLIENT & TypeMask] = OtherClientGone;
	DeleteFuncs[RT_PASSIVEGRAB & TypeMask] = DeletePassiveGrab;
    }
    clientTable[i = client->index].resources =
	(ResourcePtr *)xalloc(INITBUCKETS*sizeof(ResourcePtr));
    if (!clientTable[i].resources)
	return FALSE;
    clientTable[i].buckets = INITBUCKETS;
    clientTable[i].elements = 0;
    clientTable[i].hashsize = INITHASHSIZE;
    /* Many IDs allocated from the server client are visible to clients,
     * so we don't use the SERVER_BIT for them, but we have to start
     * past the magic value constants used in the protocol.  For normal
     * clients, we can start from zero, with SERVER_BIT set.
     */
    clientTable[i].fakeID = client->clientAsMask |
			    (client->index ? SERVER_BIT : SERVER_MINID);
    clientTable[i].endFakeID = (clientTable[i].fakeID | RESOURCE_ID_MASK) + 1;
    clientTable[i].expectID = client->clientAsMask;
    for (j=0; j<INITBUCKETS; j++) 
    {
        clientTable[i].resources[j] = NullResource;
    }
    return TRUE;
}

static int
#if NeedFunctionPrototypes
Hash(int client, register XID id)
#else
Hash(client, id)
    int client;
    register XID id;
#endif
{
    id &= RESOURCE_ID_MASK;
    switch (clientTable[client].hashsize)
    {
	case 6:
	    return ((int)(0x03F & (id ^ (id>>6) ^ (id>>12))));
	case 7:
	    return ((int)(0x07F & (id ^ (id>>7) ^ (id>>13))));
	case 8:
	    return ((int)(0x0FF & (id ^ (id>>8) ^ (id>>16))));
	case 9:
	    return ((int)(0x1FF & (id ^ (id>>9))));
	case 10:
	    return ((int)(0x3FF & (id ^ (id>>10))));
	case 11:
	    return ((int)(0x7FF & (id ^ (id>>11))));
    }
    return -1;
}

static XID
#if NeedFunctionPrototypes
AvailableID(
    register int client,
    register XID id,
    register XID maxid,
    register XID goodid)
#else
AvailableID(client, id, maxid, goodid)
    register int client;
    register XID id, maxid, goodid;
#endif
{
    register ResourcePtr res;

    if ((goodid >= id) && (goodid <= maxid))
	return goodid;
    for (; id <= maxid; id++)
    {
	res = clientTable[client].resources[Hash(client, id)];
	while (res && (res->id != id))
	    res = res->next;
	if (!res)
	    return id;
    }
    return 0;
}

void
GetXIDRange(client, server, minp, maxp)
    int client;
    Bool server;
    XID *minp, *maxp;
{
    register XID id, maxid;
    register ResourcePtr *resp;
    register ResourcePtr res;
    register int i;
    XID goodid;

    id = (Mask)client << CLIENTOFFSET;
    if (server)
	id |= client ? SERVER_BIT : SERVER_MINID;
    maxid = id | RESOURCE_ID_MASK;
    goodid = 0;
    for (resp = clientTable[client].resources, i = clientTable[client].buckets;
	 --i >= 0;)
    {
	for (res = *resp++; res; res = res->next)
	{
	    if ((res->id < id) || (res->id > maxid))
		continue;
	    if (((res->id - id) >= (maxid - res->id)) ?
		(goodid = AvailableID(client, id, res->id - 1, goodid)) :
		!(goodid = AvailableID(client, res->id + 1, maxid, goodid)))
		maxid = res->id - 1;
	    else
		id = res->id + 1;
	}
    }
    if (id > maxid)
	id = maxid = 0;
    *minp = id;
    *maxp = maxid;
}

/*  GetXIDList is called by the XC-MISC extension's MiscGetXIDList function.
 *  This function tries to find count unused XIDs for the given client.  It 
 *  puts the IDs in the array pids and returns the number found, which should
 *  almost always be the number requested.
 *
 *  The circumstances that lead to a call to this function are very rare.
 *  Xlib must run out of IDs while trying to generate a request that wants
 *  multiple ID's, like the Multi-buffering CreateImageBuffers request.
 *
 *  No rocket science in the implementation; just iterate over all
 *  possible IDs for the given client and pick the first count IDs
 *  that aren't in use.  A more efficient algorithm could probably be
 *  invented, but this will be used so rarely that this should suffice.
 */

unsigned int
GetXIDList(pClient, count, pids)
    ClientPtr pClient;
    unsigned int count;
    XID *pids;
{
    unsigned int found = 0;
    XID id = pClient->clientAsMask;
    XID maxid;

    maxid = id | RESOURCE_ID_MASK;
    while ( (found < count) && (id <= maxid) )
    {
	if (!LookupIDByClass(id, RC_ANY))
	{
	    pids[found++] = id;
	}
	id++;
    }
    return found;
}

/*
 * Return the next usable fake client ID.
 *
 * Normally this is just the next one in line, but if we've used the last
 * in the range, we need to find a new range of safe IDs to avoid
 * over-running another client.
 */

XID
FakeClientID(client)
    register int client;
{
    XID id, maxid;

    id = clientTable[client].fakeID++;
    if (id != clientTable[client].endFakeID)
	return id;
    GetXIDRange(client, TRUE, &id, &maxid);
    if (!id) {
	if (!client)
	    FatalError("FakeClientID: server internal ids exhausted\n");
	MarkClientException(clients[client]);
	id = ((Mask)client << CLIENTOFFSET) | (SERVER_BIT * 3);
	maxid = id | RESOURCE_ID_MASK;
    }
    clientTable[client].fakeID = id + 1;
    clientTable[client].endFakeID = maxid + 1;
    return id;
}

Bool
AddResource(id, type, value)
    XID id;
    RESTYPE type;
    pointer value;
{
    int client;
    register ClientResourceRec *rrec;
    register ResourcePtr res, *head;
    	
    client = CLIENT_ID(id);
    rrec = &clientTable[client];
    if (!rrec->buckets)
    {
	ErrorF("AddResource(%x, %x, %x), client=%d \n",
		id, type, (unsigned long)value, client);
        FatalError("client not in use\n");
    }
    if ((rrec->elements >= 4*rrec->buckets) &&
	(rrec->hashsize < MAXHASHSIZE))
	RebuildTable(client);
    head = &rrec->resources[Hash(client, id)];
    res = (ResourcePtr)xalloc(sizeof(ResourceRec));
    if (!res)
    {
	(*DeleteFuncs[type & TypeMask])(value, id);
	return FALSE;
    }
    res->next = *head;
    res->id = id;
    res->type = type;
    res->value = value;
    *head = res;
    rrec->elements++;
    if (!(id & SERVER_BIT) && (id >= rrec->expectID))
	rrec->expectID = id + 1;
    return TRUE;
}

static void
RebuildTable(client)
    int client;
{
    register int j;
    register ResourcePtr res, next;
    ResourcePtr **tails, *resources;
    register ResourcePtr **tptr, *rptr;

    /*
     * For now, preserve insertion order, since some ddx layers depend
     * on resources being free in the opposite order they are added.
     */

    j = 2 * clientTable[client].buckets;
    tails = (ResourcePtr **)ALLOCATE_LOCAL(j * sizeof(ResourcePtr *));
    if (!tails)
	return;
    resources = (ResourcePtr *)xalloc(j * sizeof(ResourcePtr));
    if (!resources)
    {
	DEALLOCATE_LOCAL(tails);
	return;
    }
    for (rptr = resources, tptr = tails; --j >= 0; rptr++, tptr++)
    {
	*rptr = NullResource;
	*tptr = rptr;
    }
    clientTable[client].hashsize++;
    for (j = clientTable[client].buckets,
	 rptr = clientTable[client].resources;
	 --j >= 0;
	 rptr++)
    {
	for (res = *rptr; res; res = next)
	{
	    next = res->next;
	    res->next = NullResource;
	    tptr = &tails[Hash(client, res->id)];
	    **tptr = res;
	    *tptr = &res->next;
	}
    }
    DEALLOCATE_LOCAL(tails);
    clientTable[client].buckets *= 2;
    xfree(clientTable[client].resources);
    clientTable[client].resources = resources;
}

void
FreeResource(id, skipDeleteFuncType)
    XID id;
    RESTYPE skipDeleteFuncType;
{
    int		cid;
    register    ResourcePtr res;
    register	ResourcePtr *prev, *head;
    register	int *eltptr;
    int		elements;
    Bool	gotOne = FALSE;

    if (((cid = CLIENT_ID(id)) < MAXCLIENTS) && clientTable[cid].buckets)
    {
	head = &clientTable[cid].resources[Hash(cid, id)];
	eltptr = &clientTable[cid].elements;

	prev = head;
	while ( (res = *prev) )
	{
	    if (res->id == id)
	    {
		RESTYPE rtype = res->type;
		*prev = res->next;
		elements = --*eltptr;
		if (rtype & RC_CACHED)
		    FlushClientCaches(res->id);
		if (rtype != skipDeleteFuncType)
		    (*DeleteFuncs[rtype & TypeMask])(res->value, res->id);
		xfree(res);
		if (*eltptr != elements)
		    prev = head; /* prev may no longer be valid */
		gotOne = TRUE;
	    }
	    else
		prev = &res->next;
        }
	if(clients[cid] && (id == clients[cid]->lastDrawableID))
	{
	    clients[cid]->lastDrawable = (DrawablePtr)WindowTable[0];
	    clients[cid]->lastDrawableID = WindowTable[0]->drawable.id;
	}
    }
    if (!gotOne)
	FatalError("Freeing resource id=%X which isn't there", id);
}


void
FreeResourceByType(id, type, skipFree)
    XID id;
    RESTYPE type;
    Bool    skipFree;
{
    int		cid;
    register    ResourcePtr res;
    register	ResourcePtr *prev, *head;

    if (((cid = CLIENT_ID(id)) < MAXCLIENTS) && clientTable[cid].buckets)
    {
	head = &clientTable[cid].resources[Hash(cid, id)];

	prev = head;
	while ( (res = *prev) )
	{
	    if (res->id == id && res->type == type)
	    {
		*prev = res->next;
		if (type & RC_CACHED)
		    FlushClientCaches(res->id);
		if (!skipFree)
		    (*DeleteFuncs[type & TypeMask])(res->value, res->id);
		xfree(res);
		break;
	    }
	    else
		prev = &res->next;
        }
	if(clients[cid] && (id == clients[cid]->lastDrawableID))
	{
	    clients[cid]->lastDrawable = (DrawablePtr)WindowTable[0];
	    clients[cid]->lastDrawableID = WindowTable[0]->drawable.id;
	}
    }
}

/*
 * Change the value associated with a resource id.  Caller
 * is responsible for "doing the right thing" with the old
 * data
 */

Bool
ChangeResourceValue (id, rtype, value)
    XID	id;
    RESTYPE rtype;
    pointer value;
{
    int    cid;
    register    ResourcePtr res;

    if (((cid = CLIENT_ID(id)) < MAXCLIENTS) && clientTable[cid].buckets)
    {
	res = clientTable[cid].resources[Hash(cid, id)];

	for (; res; res = res->next)
	    if ((res->id == id) && (res->type == rtype))
	    {
		if (rtype & RC_CACHED)
		    FlushClientCaches(res->id);
		res->value = value;
		return TRUE;
	    }
    }
    return FALSE;
}

/* Note: if func adds or deletes resources, then func can get called
 * more than once for some resources.  If func adds new resources,
 * func might or might not get called for them.  func cannot both
 * add and delete an equal number of resources!
 */

void
FindClientResourcesByType(client, type, func, cdata)
    ClientPtr client;
    RESTYPE type;
    FindResType func;
    pointer cdata;
{
    register ResourcePtr *resources;
    register ResourcePtr this, next;
    int i, elements;
    register int *eltptr;

    if (!client)
	client = serverClient;

    resources = clientTable[client->index].resources;
    eltptr = &clientTable[client->index].elements;
    for (i = 0; i < clientTable[client->index].buckets; i++) 
    {
        for (this = resources[i]; this; this = next)
	{
	    next = this->next;
	    if (!type || this->type == type) {
		elements = *eltptr;
		(*func)(this->value, this->id, cdata);
		if (*eltptr != elements)
		    next = resources[i]; /* start over */
	    }
	}
    }
}

void
FreeClientNeverRetainResources(client)
    ClientPtr client;
{
    ResourcePtr *resources;
    ResourcePtr this;
    ResourcePtr *prev;
    int j;

    if (!client)
	return;

    resources = clientTable[client->index].resources;
    for (j=0; j < clientTable[client->index].buckets; j++) 
    {
	prev = &resources[j];
        while ( (this = *prev) )
	{
	    RESTYPE rtype = this->type;
	    if (rtype & RC_NEVERRETAIN)
	    {
		*prev = this->next;
		if (rtype & RC_CACHED)
		    FlushClientCaches(this->id);
		(*DeleteFuncs[rtype & TypeMask])(this->value, this->id);
		xfree(this);	    
	    }
	    else
		prev = &this->next;
	}
    }
}

void
FreeClientResources(client)
    ClientPtr client;
{
    register ResourcePtr *resources;
    register ResourcePtr this;
    int j;

    /* This routine shouldn't be called with a null client, but just in
	case ... */

    if (!client)
	return;

    HandleSaveSet(client);

    resources = clientTable[client->index].resources;
    for (j=0; j < clientTable[client->index].buckets; j++) 
    {
        /* It may seem silly to update the head of this resource list as
	we delete the members, since the entire list will be deleted any way, 
	but there are some resource deletion functions "FreeClientPixels" for 
	one which do a LookupID on another resource id (a Colormap id in this
	case), so the resource list must be kept valid up to the point that
	it is deleted, so every time we delete a resource, we must update the
	head, just like in FreeResource. I hope that this doesn't slow down
	mass deletion appreciably. PRH */

	ResourcePtr *head;

	head = &resources[j];

        for (this = *head; this; this = *head)
	{
	    RESTYPE rtype = this->type;
	    *head = this->next;
	    if (rtype & RC_CACHED)
		FlushClientCaches(this->id);
	    (*DeleteFuncs[rtype & TypeMask])(this->value, this->id);
	    xfree(this);	    
	}
    }
    xfree(clientTable[client->index].resources);
    clientTable[client->index].buckets = 0;
}

void
FreeAllResources()
{
    int	i;

    for (i = currentMaxClients; --i >= 0; ) 
    {
        if (clientTable[i].buckets) 
	    FreeClientResources(clients[i]);
    }
}

Bool
LegalNewID(id, client)
    XID id;
    register ClientPtr client;
{
    return ((client->clientAsMask == (id & ~RESOURCE_ID_MASK)) &&
	    ((clientTable[client->index].expectID <= id) ||
	     !LookupIDByClass(id, RC_ANY)));
}

#ifdef XCSECURITY

/* SecurityLookupIDByType and SecurityLookupIDByClass:
 * These are the heart of the resource ID security system.  They take
 * two additional arguments compared to the old LookupID functions:
 * the client doing the lookup, and the access mode (see resource.h).
 * The resource is returned if it exists and the client is allowed access,
 * else NULL is returned.
 */

pointer
SecurityLookupIDByType(client, id, rtype, mode)
    ClientPtr client;
    XID id;
    RESTYPE rtype;
    Mask mode;
{
    int    cid;
    register    ResourcePtr res;
    pointer retval = NULL;

    assert(client == NullClient ||
     (client->index <= currentMaxClients && clients[client->index] == client));
    assert( (rtype & TypeMask) <= lastResourceType);

    if (((cid = CLIENT_ID(id)) < MAXCLIENTS) &&
	clientTable[cid].buckets)
    {
	res = clientTable[cid].resources[Hash(cid, id)];

	for (; res; res = res->next)
	    if ((res->id == id) && (res->type == rtype))
	    {
		retval = res->value;
		break;
	    }
    }
    if (retval && client && client->CheckAccess)
	retval = (* client->CheckAccess)(client, id, rtype, mode, retval);
    return retval;
}


pointer
SecurityLookupIDByClass(client, id, classes, mode)
    ClientPtr client;
    XID id;
    RESTYPE classes;
    Mask mode;
{
    int    cid;
    register    ResourcePtr res;
    pointer retval = NULL;

    assert(client == NullClient ||
     (client->index <= currentMaxClients && clients[client->index] == client));
    assert (classes >= lastResourceClass);

    if (((cid = CLIENT_ID(id)) < MAXCLIENTS) &&
	clientTable[cid].buckets)
    {
	res = clientTable[cid].resources[Hash(cid, id)];

	for (; res; res = res->next)
	    if ((res->id == id) && (res->type & classes))
	    {
		retval = res->value;
		break;
	    }
    }
    if (retval && client && client->CheckAccess)
	retval = (* client->CheckAccess)(client, id, classes, mode, retval);
    return retval;
}

/* We can't replace the LookupIDByType and LookupIDByClass functions with
 * macros because of compatibility with loadable servers.
 */

pointer
LookupIDByType(id, rtype)
    XID id;
    RESTYPE rtype;
{
    return SecurityLookupIDByType(NullClient, id, rtype,
				  SecurityUnknownAccess);
}

pointer
LookupIDByClass(id, classes)
    XID id;
    RESTYPE classes;
{
    return SecurityLookupIDByClass(NullClient, id, classes,
				   SecurityUnknownAccess);
}

#else /* not XCSECURITY */

/*
 *  LookupIDByType returns the object with the given id and type, else NULL.
 */ 
pointer
LookupIDByType(id, rtype)
    XID id;
    RESTYPE rtype;
{
    int    cid;
    register    ResourcePtr res;

    if (((cid = CLIENT_ID(id)) < MAXCLIENTS) &&
	clientTable[cid].buckets)
    {
	res = clientTable[cid].resources[Hash(cid, id)];

	for (; res; res = res->next)
	    if ((res->id == id) && (res->type == rtype))
		return res->value;
    }
    return (pointer)NULL;
}

/*
 *  LookupIDByClass returns the object with the given id and any one of the
 *  given classes, else NULL.
 */ 
pointer
LookupIDByClass(id, classes)
    XID id;
    RESTYPE classes;
{
    int    cid;
    register    ResourcePtr res;

    if (((cid = CLIENT_ID(id)) < MAXCLIENTS) &&
	clientTable[cid].buckets)
    {
	res = clientTable[cid].resources[Hash(cid, id)];

	for (; res; res = res->next)
	    if ((res->id == id) && (res->type & classes))
		return res->value;
    }
    return (pointer)NULL;
}

#endif /* XCSECURITY */
