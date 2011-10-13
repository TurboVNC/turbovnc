/*-
 * Copyright (c) 1998-1999 Shunsuke Akiyama <akiyama@jp.FreeBSD.org>.
 * All rights reserved.
 * Copyright (c) 1998-1999 X-TrueType Server Project, All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	Id: fontcache.c,v 1.12 1999/01/31 13:47:45 akiyama Exp $
 */
/* $XFree86: xc/programs/Xserver/Xext/fontcache.c,v 1.7 2003/10/28 23:08:43 tsi Exp $ */

/* THIS IS NOT AN X CONSORTIUM STANDARD */

#define NEED_REPLIES
#define NEED_EVENTS
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "scrnintstr.h"
#include "inputstr.h"
#include "servermd.h"
#define _FONTCACHE_SERVER_
#include "fontcacheP.h"
#include "fontcachstr.h"
#include <X11/Xfuncproto.h>

#include "swaprep.h"
#include "modinit.h"

static int miscErrorBase;

static void FontCacheResetProc(
    ExtensionEntry* /* extEntry */
);

static DISPATCH_PROC(ProcFontCacheDispatch);
static DISPATCH_PROC(ProcFontCacheGetCacheSettings);
static DISPATCH_PROC(ProcFontCacheGetCacheStatistics);
static DISPATCH_PROC(ProcFontCacheQueryVersion);
static DISPATCH_PROC(ProcFontCacheChangeCacheSettings);
static DISPATCH_PROC(SProcFontCacheDispatch);
static DISPATCH_PROC(SProcFontCacheGetCacheSettings);
static DISPATCH_PROC(SProcFontCacheGetCacheStatistics);
static DISPATCH_PROC(SProcFontCacheQueryVersion);
static DISPATCH_PROC(SProcFontCacheChangeCacheSettings);

#if 0
static unsigned char FontCacheReqCode = 0;
#endif

void
FontCacheExtensionInit(INITARGS)
{
    ExtensionEntry* extEntry;

    if (
	(extEntry = AddExtension(FONTCACHENAME,
				FontCacheNumberEvents,
				FontCacheNumberErrors,
				ProcFontCacheDispatch,
				SProcFontCacheDispatch,
				FontCacheResetProc,
				StandardMinorOpcode))) {
#if 0
	FontCacheReqCode = (unsigned char)extEntry->base;
#endif
	miscErrorBase = extEntry->errorBase;
    }
}

/*ARGSUSED*/
static void
FontCacheResetProc (extEntry)
    ExtensionEntry* extEntry;
{
}

static int
ProcFontCacheQueryVersion(client)
    register ClientPtr client;
{
    xFontCacheQueryVersionReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xFontCacheQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = FONTCACHE_MAJOR_VERSION;
    rep.minorVersion = FONTCACHE_MINOR_VERSION;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    	swaps(&rep.majorVersion, n);
    	swaps(&rep.minorVersion, n);
    }
    WriteToClient(client, SIZEOF(xFontCacheQueryVersionReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcFontCacheGetCacheSettings(client)
    register ClientPtr client;
{
    xFontCacheGetCacheSettingsReply rep;
    FontCacheSettings cinfo;
    register int n;

    REQUEST_SIZE_MATCH(xFontCacheGetCacheSettingsReq);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;

    /* XXX */
    FontCacheGetSettings(&cinfo);
    rep.himark = cinfo.himark;
    rep.lowmark = cinfo.lowmark;
    rep.balance = cinfo.balance;
    rep.reserve0 = 0;
    rep.reserve1 = 0;
    rep.reserve2 = 0;

    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.himark, n);
    	swapl(&rep.lowmark, n);
    	swapl(&rep.balance, n);
    	swapl(&rep.reserve0, n);
    	swapl(&rep.reserve1, n);
    	swapl(&rep.reserve2, n);
    }
    /* XXX */

    WriteToClient(client, SIZEOF(xFontCacheGetCacheSettingsReply),
		  (char *)&rep);
    return (client->noClientException);
}

static int
ProcFontCacheGetCacheStatistics(client)
    register ClientPtr client;
{
    xFontCacheGetCacheStatisticsReply rep;
    FontCacheStatistics cstats;
    register int n;

    REQUEST_SIZE_MATCH(xFontCacheGetCacheStatisticsReq);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = (sz_xFontCacheGetCacheStatisticsReply - 32) >> 2;

    /* XXX */
    FontCacheGetStatistics(&cstats);
    rep.purge_runs = cstats.purge_runs;
    rep.purge_stat = cstats.purge_stat;
    rep.balance = cstats.balance;
    rep.reserve0 = 0;
    rep.f_hits = cstats.f.hits;
    rep.f_misshits = cstats.f.misshits;
    rep.f_purged = cstats.f.purged;
    rep.f_usage = cstats.f.usage;
    rep.f_reserve0 = 0;
    rep.v_hits = cstats.v.hits;
    rep.v_misshits = cstats.v.misshits;
    rep.v_purged = cstats.v.purged;
    rep.v_usage = cstats.v.usage;
    rep.v_reserve0 = 0;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swapl(&rep.purge_runs, n);
	swapl(&rep.purge_stat, n);
    	swapl(&rep.balance, n);
	swapl(&rep.reserve0, n);
    	swapl(&rep.f_hits, n);
    	swapl(&rep.f_misshits, n);
	swapl(&rep.f_purged, n);
    	swapl(&rep.f_usage, n);
	swapl(&rep.f_reserve0, n);
	swapl(&rep.v_hits, n);
	swapl(&rep.v_misshits, n);
	swapl(&rep.v_purged, n);
	swapl(&rep.v_usage, n);
	swapl(&rep.v_reserve0, n);
    }
    /* XXX */
    WriteToClient(client, SIZEOF(xFontCacheGetCacheStatisticsReply),
		  (char *)&rep);
    return (client->noClientException);
}

static int
ProcFontCacheChangeCacheSettings(client)
    register ClientPtr client;
{
    FontCacheSettings cs;

    REQUEST(xFontCacheChangeCacheSettingsReq);

    REQUEST_SIZE_MATCH(xFontCacheChangeCacheSettingsReq);

    /* XXX */
    cs.himark = stuff->himark;
    cs.lowmark = stuff->lowmark;
    cs.balance = stuff->balance;

    if (cs.himark < 0 || cs.lowmark < 0)
        return BadValue;
    if (cs.himark <= cs.lowmark)
	return BadValue;
    if (!(10 <= cs.balance && cs.balance <= 90))
	return BadValue;

    if (FontCacheChangeSettings(&cs) == 0)
	return miscErrorBase + FontCacheCannotAllocMemory;
    /* XXX */

    return (client->noClientException);
}

static int
ProcFontCacheDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_FontCacheQueryVersion:
	return ProcFontCacheQueryVersion(client);
    case X_FontCacheGetCacheSettings:
	return ProcFontCacheGetCacheSettings(client);
    case X_FontCacheGetCacheStatistics:
	return ProcFontCacheGetCacheStatistics(client);
    case X_FontCacheChangeCacheSettings:
	return ProcFontCacheChangeCacheSettings(client);
    default:
	return miscErrorBase + FontCacheBadProtocol;
    }
}

static int
SProcFontCacheQueryVersion(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xFontCacheQueryVersionReq);
    swaps(&stuff->length, n);
    return ProcFontCacheQueryVersion(client);
}

static int
SProcFontCacheGetCacheSettings(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xFontCacheGetCacheSettingsReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xFontCacheGetCacheSettingsReq);
    return ProcFontCacheGetCacheSettings(client);
}

static int
SProcFontCacheGetCacheStatistics(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xFontCacheGetCacheStatisticsReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xFontCacheGetCacheStatisticsReq);
    return ProcFontCacheGetCacheStatistics(client);
}

static int
SProcFontCacheChangeCacheSettings(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xFontCacheChangeCacheSettingsReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xFontCacheChangeCacheSettingsReq);
    /* XXX */
    swapl(&stuff->himark, n);
    swapl(&stuff->lowmark, n);
    swapl(&stuff->balance, n);
    /* XXX */
    return ProcFontCacheChangeCacheSettings(client);
}

static int
SProcFontCacheDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_FontCacheQueryVersion:
	return SProcFontCacheQueryVersion(client);
    case X_FontCacheGetCacheSettings:
	return SProcFontCacheGetCacheSettings(client);
    case X_FontCacheGetCacheStatistics:
	return SProcFontCacheGetCacheStatistics(client);
    case X_FontCacheChangeCacheSettings:
	return SProcFontCacheChangeCacheSettings(client);
    default:
	return miscErrorBase + FontCacheBadProtocol;
    }
}
