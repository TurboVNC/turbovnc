/* $TOG: fserve.c /main/49 1997/06/10 11:23:56 barstow $ */
/*

Copyright (c) 1990  X Consortium

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

*/
/* $XFree86: xc/lib/font/fc/fserve.c,v 3.4.2.2 1997/06/11 12:08:41 dawes Exp $ */

/*
 * Copyright 1990 Network Computing Devices
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, or Digital
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, AND DIGITAL AND DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES,
 * OR DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 *
 * Author:  	Dave Lemke, Network Computing Devices, Inc
 */
/*
 * font server specific font access
 */

#ifdef WIN32
#define _WILLWINSOCK_
#endif
#include	<X11/X.h>
#include	<X11/Xos.h>
#include	"X11/Xpoll.h"
#include	"FS.h"
#include	"FSproto.h"
#include	"fontmisc.h"
#include	"fontstruct.h"
#include	"fservestr.h"
#include	<errno.h>
#if defined(X_NOT_STDC_ENV) && !defined(__EMX__)
extern int errno;
#define Time_t long
extern Time_t time ();
#else
#include	<time.h>
#define Time_t time_t
#endif

#ifdef NCD
#include	<ncd/nvram.h>
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef MIN
#define MIN(a,b)    ((a)<(b)?(a):(b))
#endif

#define NONZEROMETRICS(pci) ((pci)->leftSideBearing || \
			     (pci)->rightSideBearing || \
			     (pci)->ascent || \
			     (pci)->descent || \
			     (pci)->characterWidth)


extern FontPtr find_old_font();

extern int  _fs_build_range();

static int  fs_read_glyphs();
static int  fs_read_list();
static int  fs_read_list_info();

static int  fs_font_type;
extern fd_set _fs_fd_mask;

static void fs_block_handler();
static int  fs_wakeup();

static FSFpePtr awaiting_reconnect;

void        _fs_connection_died();
static int  _fs_restart_connection();
static void _fs_try_reconnect();
static int  fs_send_query_info();
static int  fs_send_query_extents();
static int  fs_send_query_bitmaps();
static int  fs_send_close_font();
static void fs_client_died();
static void _fs_client_access();
static void _fs_client_resolution();

char _fs_glyph_undefined;
char _fs_glyph_requested;
char _fs_glyph_zero_length;

/*
 * Font server access
 *
 * the basic idea for the non-blocking access is to have the function
 * called multiple times until the actual data is returned, instead
 * of ClientBlocked.
 *
 * the first call to the function will cause the request to be sent to
 * the font server, and a block record to be stored in the fpe's list
 * of outstanding requests.  the FS block handler also sticks the
 * proper set of fd's into the select mask.  when data is ready to be
 * read in, the FS wakup handler will be hit.  this will read the
 * data off the wire into the proper block record, and then signal the
 * client that caused the block so that it can restart.  it will then
 * call the access function again, which will realize that the data has
 * arrived and return it.
 */


/* XXX this should probably be a macro once its fully debugged */
/* ARGSUSED */
static void
_fs_add_req_log(conn, opcode)
    FSFpePtr    conn;
    int         opcode;
{

#ifdef DEBUG
    conn->reqbuffer[conn->reqindex++] = opcode;
    if (conn->reqindex == REQUEST_LOG_SIZE)
	conn->reqindex = 0;
#endif

    conn->current_seq++;
}

static Bool
fs_name_check(name)
    char       *name;
{
#ifdef __EMX__
    /* OS/2 uses D:/XFree86/.... as fontfile pathnames, so check that
     * there is not only a protocol/ prefix, but also that the first chars
     * are not a drive letter
     */
    if (name && isalpha(*name) && name[1] == ':')
      return FALSE;
#endif
    /* Just make sure there is a protocol/ prefix */
    return (name && *name != '/' && strchr(name, '/'));
}

static void
_fs_client_resolution(conn)
    FSFpePtr    conn;
{
    fsSetResolutionReq srreq;
    int         num_res;
    FontResolutionPtr res;

    res = GetClientResolutions(&num_res);

    if (num_res) {
	srreq.reqType = FS_SetResolution;
	srreq.num_resolutions = num_res;
	srreq.length = (SIZEOF(fsSetResolutionReq) +
			(num_res * SIZEOF(fsResolution)) + 3) >> 2;

	_fs_add_req_log(conn, FS_SetResolution);
	if (_fs_write(conn, (char *) &srreq, SIZEOF(fsSetResolutionReq)) != -1)
	    (void)_fs_write_pad(conn, (char *) res,
				(num_res * SIZEOF(fsResolution)));
    }
}

/*
 * sends the stuff that's meaningful to a newly opened or reset FS
 */
static int
fs_send_init_packets(conn)
    FSFpePtr    conn;
{
    fsSetResolutionReq srreq;
    fsSetCataloguesReq screq;
    fsListCataloguesReq lcreq;
    fsListCataloguesReply lcreply;
    int         num_cats,
                clen,
                len;
    char       *client_cat = (char *) 0,
               *cp,
               *sp,
               *end;
    int         num_res;
    FontResolutionPtr res;
    int         err = Successful;

#define	CATALOGUE_SEP	'+'

    res = GetClientResolutions(&num_res);
    if (num_res) {
	srreq.reqType = FS_SetResolution;
	srreq.num_resolutions = num_res;
	srreq.length = (SIZEOF(fsSetResolutionReq) +
			(num_res * SIZEOF(fsResolution)) + 3) >> 2;

	_fs_add_req_log(conn, FS_SetResolution);
	if (_fs_write(conn, (char *) &srreq, SIZEOF(fsSetResolutionReq)) == -1)
	{
	    err = BadFontPath;
	    goto fail;
	}
	if (_fs_write_pad(conn, (char *) res, (num_res * SIZEOF(fsResolution))) == -1)
	{
	    err = BadFontPath;
	    goto fail;
	}
    }
    sp = strrchr(conn->servername, '/');

    /* don't get tricked by a non-existant catalogue list */
    if (sp == strchr(conn->servername, '/')) {
	/*
	 * try original name -- this might be an alternate with no catalogues
	 */
	sp = strrchr(conn->requestedname, '/');
	if (sp == strchr(conn->requestedname, '/'))
		sp = (char *) 0;
    }
    if (sp) {			/* turn cats into counted list */
	sp++;
	/* allocate more than enough room */
	cp = client_cat = (char *) xalloc(strlen(conn->servername));
	if (!cp) {
	    err = BadAlloc;
	    goto fail;
	}
	num_cats = 0;
	while (*sp) {
	    end = strchr(sp, CATALOGUE_SEP);
	    if (!end)
		end = sp + strlen(sp);
	    *cp++ = len = end - sp;
	    num_cats++;
	    memmove(cp, sp, len);
	    sp += len;
	    if (*sp == CATALOGUE_SEP)
		sp++;
	    cp += len;
	}
	clen = cp - client_cat;
	/* our list checked out, so send it */
	screq.reqType = FS_SetCatalogues;
	screq.num_catalogues = num_cats;
	screq.length = (SIZEOF(fsSetCataloguesReq) + clen + 3) >> 2;

	_fs_add_req_log(conn, FS_SetCatalogues);
	if (_fs_write(conn, (char *) &screq, SIZEOF(fsSetCataloguesReq)) == -1)
	{
	    err = BadFontPath;
	    goto fail;
	}
	if (_fs_write_pad(conn, (char *) client_cat, clen) == -1)
	{
	    err = BadFontPath;
	    goto fail;
	}

	/*
	 * now sync up with the font server, to see if an error was generated
	 * by a bogus catalogue
	 */
	lcreq.reqType = FS_ListCatalogues;
	lcreq.length = (SIZEOF(fsListCataloguesReq)) >> 2;
	lcreq.maxNames = 0;
	lcreq.nbytes = 0;
	_fs_add_req_log(conn, FS_SetCatalogues);
	if (_fs_write(conn, (char *) &lcreq, SIZEOF(fsListCataloguesReq)) == -1)
	{
	    err = BadFontPath;
	    goto fail;
	}

	/*
	 * next bit will either by the ListCats reply, or an error followed by
	 * the reply
	 */
	if (_fs_read(conn, (char *) &lcreply, SIZEOF(fsGenericReply)) == -1) {
	    err = BadFontPath;
	    goto fail;
	}
	if (lcreply.type == FS_Error &&
		((fsError *) & lcreply)->major_opcode == FS_SetCatalogues) {
	    _fs_eat_rest_of_error(conn, (fsError *) & lcreply);
	    /* get ListCats response */
	    (void) _fs_read(conn, (char *) &lcreply,
			    SIZEOF(fsListCataloguesReply));
	    err = BadFontPath;
	    goto fail;
	}
	/* must be reply, swallow the rest of it */
	_fs_eat_rest_of_error(conn, (fsError *) & lcreply);
    }
fail:
    xfree(client_cat);
    return err;
}

/* 
 * close font server and remove any state associated with
 * this connection - this includes any client records.
 */

static void
fs_close_conn(conn)
    FSFpePtr	conn;
{
    FSClientPtr	client, nclient;

    /* XXX - hack.  The right fix is to remember that the font server
       has gone away when we first discovered it. */
    if (conn->trans_conn)
        (void) _FontTransClose (conn->trans_conn);

    if (conn->fs_fd != -1)
        FD_CLR(conn->fs_fd, &_fs_fd_mask);

    for (client = conn->clients; client; client = nclient) 
    {
	nclient = client->next;
	xfree (client);
    }
    conn->clients = NULL;
}

/*
 * the wakeup handlers have to be set when the FPE is open, and not
 * removed until it is freed, in order to handle unexpected data, like
 * events
 */
/* ARGSUSED */
static int
fs_init_fpe(fpe)
    FontPathElementPtr fpe;
{
    FSFpePtr    conn;
    char       *name;
    int         err;

    /* open font server */
    /* create FS specific fpe info */
    errno = 0;

    name = fpe->name;

    /* hack for old style names */
    if (*name == ':')
	name++;			/* skip ':' */

    conn = _fs_open_server(name);
    if (conn) {
	conn->requestedname = fpe->name; /* stash this for later init use */
	fpe->private = (pointer) conn;
	err = fs_send_init_packets(conn);
	if (err != Successful) {
	    fs_close_conn(conn);
    	    xfree(conn->servername);
    	    xfree(conn->alts);
    	    xfree(conn);
	    return err;
	}
	if (init_fs_handlers(fpe, fs_block_handler) != Successful)
	    return AllocError;
	FD_SET(conn->fs_fd, &_fs_fd_mask);
	conn->attemptReconnect = TRUE;

#ifdef NCD
	if (configData.ExtendedFontDiags)
	    printf("Connected to font server \"%s\"\n", name);
#endif

	return err;
    }

#ifdef DEBUG
    fprintf(stderr, "failed to connect to FS \"%s\"\n", name);
#endif

#ifdef NCD
    if (configData.ExtendedFontDiags)
	printf("Failed to connect to font server \"%s\"\n", name);
#endif

    return (errno == ENOMEM) ? AllocError : BadFontPath;
}

static int
fs_reset_fpe(fpe)
    FontPathElementPtr fpe;
{
    (void) fs_send_init_packets((FSFpePtr) fpe->private);
    return Successful;
}

/*
 * this shouldn't be called till all refs to the FPE are gone
 */

static int
fs_free_fpe(fpe)
    FontPathElementPtr fpe;
{
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    FSFpePtr    recon,
               *prev;
    prev = &awaiting_reconnect;
    while (*prev) {
	recon = *prev;
	if (conn == recon) {
	    *prev = recon->next_reconnect;
	    break;
	}
	prev = &recon->next_reconnect;
    }

    fs_close_conn(conn);

    remove_fs_handlers(fpe, fs_block_handler,
		       !XFD_ANYSET(&_fs_fd_mask) && !awaiting_reconnect);

    xfree(conn->alts);
    xfree(conn->servername);
    xfree(conn);
    fpe->private = (pointer) 0;

#ifdef NCD
    if (configData.ExtendedFontDiags)
	printf("Disconnected from font server \"%s\"\n", fpe->name);
#endif

    return Successful;
}

static      FSBlockDataPtr
fs_new_block_rec(fpe, client, type)
    FontPathElementPtr fpe;
    pointer     client;
    int         type;
{
    FSBlockDataPtr blockrec,
                br;
    FSFpePtr    fsfpe = (FSFpePtr) fpe->private;
    int         size;

    blockrec = (FSBlockDataPtr) xalloc(sizeof(FSBlockDataRec));
    if (!blockrec)
	return (FSBlockDataPtr) 0;
    switch (type) {
    case FS_OPEN_FONT:
	size = sizeof(FSBlockedFontRec);
	break;
    case FS_LOAD_GLYPHS:
	size = sizeof(FSBlockedGlyphRec);
	break;
    case FS_LIST_FONTS:
	size = sizeof(FSBlockedListRec);
	break;
    case FS_LIST_WITH_INFO:
	size = sizeof(FSBlockedListInfoRec);
	break;
    default:
	break;
    }
    blockrec->data = (pointer) xalloc(size);
    if (!blockrec->data) {
	xfree(blockrec);
	return (FSBlockDataPtr) 0;
    }
    blockrec->client = client;
    blockrec->sequence_number = fsfpe->current_seq;
    blockrec->type = type;
    blockrec->depending = 0;
    blockrec->next = (FSBlockDataPtr) 0;

    /* stick it on the end of the list (since its expected last) */
    br = (FSBlockDataPtr) fsfpe->blocked_requests;
    if (!br) {
	fsfpe->blocked_requests = (pointer) blockrec;
    } else {
	while (br->next)
	    br = br->next;
	br->next = blockrec;
    }

    return blockrec;
}

static void
_fs_remove_block_rec(conn, blockrec)
    FSFpePtr    conn;
    FSBlockDataPtr blockrec;
{
    FSBlockDataPtr br,
                last;

    last = (FSBlockDataPtr) 0;
    br = (FSBlockDataPtr) conn->blocked_requests;
    while (br) {
	if (br == blockrec) {
	    if (last)
		last->next = br->next;
	    else
		conn->blocked_requests = (pointer) br->next;
	    if (br->type == FS_LOAD_GLYPHS)
	    {
		FSBlockedGlyphPtr bglyph = (FSBlockedGlyphPtr)br->data;
		if (bglyph->num_expected_ranges)
		    xfree(bglyph->expected_ranges);
	    }
	    xfree(br->data);
	    xfree(br);
	    return;
	}
	last = br;
	br = br->next;
    }
}

static void
signal_clients_depending(clients_depending)
FSClientsDependingPtr *clients_depending;
{
    FSClientsDependingPtr p = *clients_depending, p2;
    *clients_depending = (FSClientsDependingPtr)0;

    while (p != (FSClientsDependingPtr)0)
    {
	p2 = p;
	ClientSignal(p->client);
	p = p->next;
	xfree(p2);
    }
}

static int
add_clients_depending(clients_depending, client)
FSClientsDependingPtr *clients_depending;
pointer client;
{
    while (*clients_depending != (FSClientsDependingPtr)0)
    {
	if ((*clients_depending)->client == client) return Suspended;
	clients_depending = &(*clients_depending)->next;
    }
    *clients_depending = (FSClientsDependingPtr)xalloc(
			     sizeof(FSClientsDependingRec));
    if (!*clients_depending)
	return BadAlloc;

    (*clients_depending)->client = client;
    (*clients_depending)->next = 0;
    return Suspended;
}

static void
clean_aborted_blockrec(blockrec)
    FSBlockDataPtr blockrec;
{

    switch(blockrec->type)
    {
	case FS_LOAD_GLYPHS:
	{
	    FSBlockedGlyphPtr bglyph = (FSBlockedGlyphPtr)blockrec->data;
	    FontPtr pfont = bglyph->pfont;
	    int num_expected_ranges = bglyph->num_expected_ranges;
	    fsRange *expected_ranges = bglyph->expected_ranges;
	    _fs_clean_aborted_loadglyphs(pfont,
				     num_expected_ranges,
				     expected_ranges);
	    signal_clients_depending(&bglyph->clients_depending);
	    break;
	}
	case FS_OPEN_FONT:
	{
	    FSBlockedFontPtr bfont = (FSBlockedFontPtr)blockrec->data;
	    signal_clients_depending(&bfont->clients_depending);
	    break;
	}
	default:
	    break;
    }
}

static void
fs_abort_blockrec(conn, blockrec)
    FSFpePtr    conn;
    FSBlockDataPtr blockrec;
{
    clean_aborted_blockrec(blockrec);
    _fs_remove_block_rec(conn, blockrec);
}


static void
fs_free_font(bfont)
    FSBlockedFontPtr bfont;
{
    FontPtr     pfont;
    FSFontDataRec *fsd;

    pfont = bfont->pfont;
    fsd = (FSFontDataRec *) pfont->fpePrivate;

    /* xfree better be able to handle NULL */
    (*pfont->unload_font)(pfont);
    DeleteFontClientID(fsd->fontid);
    xfree(fsd->name);
    xfree(pfont->info.isStringProp);
    xfree(pfont->info.props);

    xfree(pfont);
    xfree(fsd);

    bfont->pfont = (FontPtr) 0;
}

static void
_fs_cleanup_font(bfont)
    FSBlockedFontPtr bfont;
{
    FSFontDataRec *fsd;

    if (bfont->pfont)
    {
    	fsd = (FSFontDataRec *) bfont->pfont->fpePrivate;
    
    	/* make sure the FS knows we choked on it */
    	fs_send_close_font(fsd->fpe, bfont->fontid);
    
    	fs_free_font(bfont);
    }
    bfont->errcode = AllocError;
}


static int
fs_read_open_font(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FSBlockedFontPtr bfont = (FSBlockedFontPtr) blockrec->data;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsOpenBitmapFontReply rep;
    FSBlockDataPtr blockOrig;
    FSBlockedFontPtr origBfont;

    /* pull out the OpenFont reply */
    memcpy(&rep, &blockrec->header, SIZEOF(fsGenericReply));

    if (rep.type == FS_Error) {
	_fs_eat_rest_of_error(conn, (fsError *) & rep);
	return BadFontName;
    } else {			/* get rest of reply */
	if (_fs_read(conn, (char *) &rep + SIZEOF(fsGenericReply),
	      SIZEOF(fsOpenBitmapFontReply) - SIZEOF(fsGenericReply)) == -1) {
	    /* If we're not reopening a font, we'll allocate the
	       structures again after connection is reestablished.  */
	    if (!(bfont->flags & FontReopen)) fs_free_font(bfont);
	    return StillWorking;
	}
    }

    /* If we're not reopening a font and FS detected a duplicate font
       open request, replace our reference to the new font with a
       reference to an existing font (possibly one not finished
       opening).  If this is a reopen, keep the new font reference...
       it's got the metrics and extents we read when the font was opened
       before.  This also gives us the freedom to easily close the font
       if we we decide (in fs_read_query_info()) that we don't like what
       we got. */

    if (rep.otherid && !(bfont->flags & FontReopen)) {
	(void) fs_send_close_font(fpe, bfont->fontid);

	/* Find old font if we're completely done getting it from server. */
	fs_free_font(bfont);
	bfont->pfont = find_old_font(rep.otherid);
	bfont->fontid = rep.otherid;
	bfont->state = FS_DONE_REPLY;
	/*
	 * look for a blocked request to open the same font
	 */
	for (blockOrig = (FSBlockDataPtr) conn->blocked_requests;
		blockOrig;
		blockOrig = blockOrig->next) {
	    if (blockOrig != blockrec && blockOrig->type == FS_OPEN_FONT) {
		origBfont = (FSBlockedFontPtr) blockOrig->data;
		if (origBfont->fontid == rep.otherid) {
		    blockrec->depending = blockOrig->depending;
		    blockOrig->depending = blockrec;
		    bfont->state = FS_DEPENDING;
		    bfont->pfont = origBfont->pfont;
		    break;
		}
	    }
	}
	if (bfont->pfont == NULL)
	{
	    /* XXX - something nasty happened */
	    return BadFontName;
	}
	return AccessDone;
    }

    bfont->pfont->info.cachable = rep.cachable != 0;
    bfont->state = FS_INFO_REPLY;
    /* ask for the next stage */
    (void) fs_send_query_info(fpe, blockrec);
    return StillWorking;
}


static int
fs_read_query_info(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FSBlockedFontPtr bfont = (FSBlockedFontPtr) blockrec->data;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsQueryXInfoReply rep;
    fsPropInfo  pi;
    fsPropOffset *po;
    pointer     pd;
    unsigned long prop_len;
    FSBlockedFontRec newbfont, *oldbfont;
    FontRec newpfont, *oldpfont;
    int err;

    /* If this is a reopen, accumulate the query info into a dummy
       font and compare to our original data. */
    if (bfont->flags & FontReopen)
    {
	newbfont = *(oldbfont = bfont);
	bfont = &newbfont;
	newpfont = *(oldpfont = oldbfont->pfont);
	newpfont.info.isStringProp = NULL;
	newpfont.info.props = NULL;
	newbfont.pfont = &newpfont;
	err = StillWorking;
    }

    /* pull out the QueryXInfo reply */
    memcpy(&rep, &blockrec->header, SIZEOF(fsGenericReply));
    if (_fs_read(conn, (char *) &rep + SIZEOF(fsGenericReply),
		 SIZEOF(fsQueryXInfoReply) - SIZEOF(fsGenericReply)) == -1) {
	if (bfont->flags & FontReopen) goto bail;
	fs_free_font(bfont);
	return StillWorking;
    }
    /* move the data over */
    fsUnpack_XFontInfoHeader(&rep, &bfont->pfont->info);
    _fs_init_fontinfo(conn, &bfont->pfont->info);

    if (bfont->pfont->info.terminalFont)
    {
	bfont->format =
	    (bfont->format & ~ (BitmapFormatImageRectMask)) |
	    BitmapFormatImageRectMax;
    }

    if (_fs_read(conn, (char *) &pi, SIZEOF(fsPropInfo)) == -1) {
	if (bfont->flags & FontReopen) goto bail;
	fs_free_font(bfont);
	return StillWorking;
    }
    prop_len = pi.num_offsets * SIZEOF(fsPropOffset);
    po = (fsPropOffset *) xalloc(prop_len);
    pd = (pointer) xalloc(pi.data_len);
    if (!po || !pd) {
	xfree(pd);
	xfree(po);
	/* clear the wire */
	(void) _fs_drain_bytes(conn, prop_len + pi.data_len);
	/* clean up the font */
	if (bfont->flags & FontReopen) { err = AllocError ; goto bail; }
	(void) _fs_cleanup_font(bfont);
	return AllocError;
    }
    if (_fs_read_pad(conn, (char *) po, prop_len) == -1 ||
	    _fs_read_pad(conn, (char *) pd, pi.data_len) == -1) {
	xfree(pd);
	xfree(po);
	if (bfont->flags & FontReopen) goto bail;
	fs_free_font(bfont);
	return StillWorking;
    }
    if (_fs_convert_props(&pi, po, pd, &bfont->pfont->info) == -1)
    {
    	xfree(po);
    	xfree(pd);
	if (bfont->flags & FontReopen) { err = AllocError ; goto bail; }
	(void) _fs_cleanup_font(bfont);
	return AllocError;
    }
    xfree(po);
    xfree(pd);

    if (bfont->flags & FontReopen)
    {
	int i;

	err = BadFontName;

	/* We're reopening a font that we lost because of a downed
	   connection.  In the interest of avoiding corruption from
	   opening a different font than the old one (we already have
	   its metrics, extents, and probably some of its glyphs),
	   verify that the metrics and properties all match.  */

	if (newpfont.info.firstCol != oldpfont->info.firstCol ||
	    newpfont.info.lastCol != oldpfont->info.lastCol ||
	    newpfont.info.firstRow != oldpfont->info.firstRow ||
	    newpfont.info.lastRow != oldpfont->info.lastRow ||
	    newpfont.info.defaultCh != oldpfont->info.defaultCh ||
	    newpfont.info.noOverlap != oldpfont->info.noOverlap ||
	    newpfont.info.terminalFont != oldpfont->info.terminalFont ||
	    newpfont.info.constantMetrics != oldpfont->info.constantMetrics ||
	    newpfont.info.constantWidth != oldpfont->info.constantWidth ||
	    newpfont.info.inkInside != oldpfont->info.inkInside ||
	    newpfont.info.inkMetrics != oldpfont->info.inkMetrics ||
	    newpfont.info.allExist != oldpfont->info.allExist ||
	    newpfont.info.drawDirection != oldpfont->info.drawDirection ||
	    newpfont.info.cachable != oldpfont->info.cachable ||
	    newpfont.info.anamorphic != oldpfont->info.anamorphic ||
	    newpfont.info.maxOverlap != oldpfont->info.maxOverlap ||
	    newpfont.info.fontAscent != oldpfont->info.fontAscent ||
	    newpfont.info.fontDescent != oldpfont->info.fontDescent ||
	    newpfont.info.nprops != oldpfont->info.nprops)
	    goto bail;

#define MATCH(xci1, xci2) \
	(((xci1).leftSideBearing == (xci2).leftSideBearing) && \
	 ((xci1).rightSideBearing == (xci2).rightSideBearing) && \
	 ((xci1).characterWidth == (xci2).characterWidth) && \
	 ((xci1).ascent == (xci2).ascent) && \
	 ((xci1).descent == (xci2).descent) && \
	 ((xci1).attributes == (xci2).attributes))

	if (!MATCH(newpfont.info.maxbounds, oldpfont->info.maxbounds) ||
	    !MATCH(newpfont.info.minbounds, oldpfont->info.minbounds) ||
	    !MATCH(newpfont.info.ink_maxbounds, oldpfont->info.ink_maxbounds) ||
	    !MATCH(newpfont.info.ink_minbounds, oldpfont->info.ink_minbounds))
	    goto bail;

#undef MATCH

	for (i = 0; i < newpfont.info.nprops; i++)
	    if (newpfont.info.isStringProp[i] !=
		    oldpfont->info.isStringProp[i] ||
		newpfont.info.props[i].name !=
		    oldpfont->info.props[i].name ||
		newpfont.info.props[i].value !=
		    oldpfont->info.props[i].value)
		goto bail;

	err = Successful;
    bail:
	if (err != Successful && err != StillWorking)
	{
	    /* Failure.  Close the font. */
    	    fs_send_close_font(((FSFontDataPtr)oldpfont->fpePrivate)->fpe,
			       bfont->fontid);
	    ((FSFontDataPtr)oldpfont->fpePrivate)->generation  = -1;
	}
	xfree(newpfont.info.isStringProp);
	xfree(newpfont.info.props);

	if (err == Successful) oldbfont->state = FS_DONE_REPLY;
	return err;
    }

    if (glyphCachingMode == CACHING_OFF ||
	glyphCachingMode == CACHE_16_BIT_GLYPHS && !bfont->pfont->info.lastRow)
	bfont->flags |= FontLoadAll;

    bfont->state = FS_EXTENT_REPLY;

    fs_send_query_extents(fpe, blockrec);
    return StillWorking;
}

static int
fs_read_extent_info(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FSBlockedFontPtr bfont = (FSBlockedFontPtr) blockrec->data;
    FSFontDataPtr fsd = (FSFontDataPtr) bfont->pfont->fpePrivate;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsQueryXExtents16Reply rep;
    int         i;
    int		numInfos;
    Bool	haveInk = FALSE; /* need separate ink metrics? */
    CharInfoPtr ci,
                pCI;
    FSFontPtr   fsfont = (FSFontPtr) bfont->pfont->fontPrivate;
    fsXCharInfo *fsci;
    fsXCharInfo fscilocal;
    pointer fscip;

    /* read the QueryXExtents reply */
    memcpy(&rep, &blockrec->header, SIZEOF(fsGenericReply));
    if (_fs_read(conn, (char *) &rep + SIZEOF(fsGenericReply),
	      SIZEOF(fsQueryXExtents16Reply) - SIZEOF(fsGenericReply)) == -1) {
	fs_free_font(bfont);
	return StillWorking;
    }
    /* move the data over */
    /* need separate inkMetrics for fixed font server protocol version */
    numInfos =  rep.num_extents;
    if (bfont->pfont->info.terminalFont && conn->fsMajorVersion > 1)
    {
	numInfos *= 2;
	haveInk = TRUE;
    }
    ci = pCI = (CharInfoPtr) xalloc(sizeof(CharInfoRec) * numInfos);
/* XXX this could be done with an ALLOCATE_LOCAL */
    fsci = (fsXCharInfo *) xalloc(SIZEOF(fsXCharInfo) * rep.num_extents);
    if (!pCI || !fsci) {
	xfree(pCI);
	xfree(fsci);
	/* clear the unusable data */
	_fs_drain_bytes(conn, SIZEOF(fsXCharInfo) * rep.num_extents);
	_fs_cleanup_font(bfont);
	return AllocError;
    }
    fsfont->encoding = pCI;
    if (haveInk)
	fsfont->inkMetrics = pCI + rep.num_extents;
    else
        fsfont->inkMetrics = pCI;

    if (_fs_read_pad(conn, (char *) fsci,
		     SIZEOF(fsXCharInfo) * rep.num_extents) == -1) {
	fs_free_font(bfont);
	xfree(fsci);
	return StillWorking;
    }
    fsd->glyphs_to_get = 0;
    fscip = (pointer) fsci;
    ci = fsfont->inkMetrics;
    for (i = 0; i < rep.num_extents; i++) {
	memcpy(&fscilocal, fscip, SIZEOF(fsXCharInfo)); /* align it */
	_fs_convert_char_info(&fscilocal, &ci->metrics);
	fscip += SIZEOF(fsXCharInfo);
	/* Initialize the bits field for later glyph-caching use */
	if (NONZEROMETRICS(&ci->metrics))
	{
	    if (!haveInk &&
		(ci->metrics.leftSideBearing == ci->metrics.rightSideBearing ||
		 ci->metrics.ascent == -ci->metrics.descent))
		pCI[i].bits = &_fs_glyph_zero_length;
	    else
	    {
		pCI[i].bits = &_fs_glyph_undefined;
		fsd->glyphs_to_get++;
	    }
	}
	else
	    pCI[i].bits = (char *)0;
	ci++;
    }

    xfree(fsci);

    /* build bitmap metrics, ImageRectMax style */
    if (haveInk)
    {
	FontInfoRec *fi = &bfont->pfont->info;
	CharInfoPtr ii;

	ci = fsfont->encoding;
	ii = fsfont->inkMetrics;
	for (i = 0; i < rep.num_extents; i++, ci++, ii++)
	{
	    if (NONZEROMETRICS(&ii->metrics))
	    {
		ci->metrics.leftSideBearing = FONT_MIN_LEFT(fi);
		ci->metrics.rightSideBearing = FONT_MAX_RIGHT(fi);
		ci->metrics.ascent = FONT_MAX_ASCENT(fi);
		ci->metrics.descent = FONT_MAX_DESCENT(fi);
		ci->metrics.characterWidth = FONT_MAX_WIDTH(fi);
		ci->metrics.attributes = ii->metrics.attributes;
	    }
	    else
	    {
		ci->metrics = ii->metrics;
	    }
	}
    }
    {
	unsigned int r, c, numCols, firstCol;

	firstCol = bfont->pfont->info.firstCol;
	numCols = bfont->pfont->info.lastCol - firstCol + 1;
	c = bfont->pfont->info.defaultCh;
	fsfont->pDefault = 0;
	if (bfont->pfont->info.lastRow)
	{
	    r = c >> 8;
	    r -= bfont->pfont->info.firstRow;
	    c &= 0xff;
	    c -= firstCol;
	    if (r < bfont->pfont->info.lastRow-bfont->pfont->info.firstRow+1 &&
		c < numCols)
		fsfont->pDefault = &pCI[r * numCols + c];
	}
	else
	{
	    c -= firstCol;
	    if (c < numCols)
		fsfont->pDefault = &pCI[c];
	}
    }
    bfont->state = FS_GLYPHS_REPLY;

    if (bfont->flags & FontLoadBitmaps) {
	fs_send_query_bitmaps(fpe, blockrec);
	return StillWorking;
    }
    return Successful;
}

/*
 * XXX should probably continue to read here if we can, but must be sure
 * it's our packet waiting, rather than another interspersed
 */
static int
fs_do_open_font(fpe, blockrec, readheader)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
    Bool        readheader;
{
    FSBlockedFontPtr bfont = (FSBlockedFontPtr) blockrec->data;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    int         err;

    switch (bfont->state) {
    case FS_OPEN_REPLY:
	if (readheader) {
	    /* get the next header */
	    if (_fs_read(conn, (char *) &blockrec->header,
			 SIZEOF(fsGenericReply)) == -1) {
		fs_free_font(bfont);
		err = StillWorking;
		break;
	    }
	}
	bfont->errcode = fs_read_open_font(fpe, blockrec);
	if (bfont->errcode != StillWorking) {	/* already loaded, or error */
	    /* if font's already loaded, massage error code */
	    switch (bfont->state) {
	    case FS_DONE_REPLY:
		bfont->errcode = Successful;
		break;
	    case FS_DEPENDING:
		bfont->errcode = StillWorking;
		break;
	    }
	    err = bfont->errcode;
	    break;
	}
	/* if more data to read or Sync, fall thru, else return */
	if (!(bfont->flags & FontOpenSync)) {
	    err = bfont->errcode;
	    break;
	} else {
	    if (_fs_read(conn, (char *) &blockrec->header,
			 SIZEOF(fsGenericReply)) == -1) {
		fs_free_font(bfont);
		err = StillWorking;
		break;
	    }
	}
	/* fall through */
    case FS_INFO_REPLY:
	bfont->errcode = fs_read_query_info(fpe, blockrec);
	if (bfont->errcode != StillWorking) {
	    err = bfont->errcode;
	    break;
	}
	if (!(bfont->flags & FontOpenSync)) {
	    err = bfont->errcode;
	    break;
	    /* if more data to read, fall thru, else return */
	} else {
	    if (_fs_read(conn, (char *) &blockrec->header,
			 SIZEOF(fsGenericReply))) {
		fs_free_font(bfont);
		err = StillWorking;
		break;
	    }
	}
	/* fall through */
    case FS_EXTENT_REPLY:
	bfont->errcode = fs_read_extent_info(fpe, blockrec);
	if (bfont->errcode != StillWorking) {
	    err = bfont->errcode;
	    break;
	}
	if (!(bfont->flags & FontOpenSync)) {
	    err = bfont->errcode;
	    break;
	} else if (bfont->flags & FontLoadBitmaps) {
	    if (_fs_read(conn, (char *) &blockrec->header,
			 SIZEOF(fsGenericReply))) {
		fs_free_font(bfont);
		err = StillWorking;
		break;
	    }
	}
	/* fall through */
    case FS_GLYPHS_REPLY:
	if (bfont->flags & FontLoadBitmaps) {
	    bfont->errcode = fs_read_glyphs(fpe, blockrec);
	}
	err = bfont->errcode;
	break;
    case FS_DEPENDING:		/* can't happen */
	err = bfont->errcode;
    default:
	err = bfont->errcode;
	break;
    }
    if (err != StillWorking) {
	bfont->state = FS_DONE_REPLY;	/* for _fs_load_glyphs() */
	while (blockrec = blockrec->depending) {
	    bfont = (FSBlockedFontPtr) blockrec->data;
	    bfont->errcode = err;
	    bfont->state = FS_DONE_REPLY;	/* for _fs_load_glyphs() */
	}
    }
    return err;
}

/* ARGSUSED */
static void
fs_block_handler(data, wt, LastSelectMask)
    pointer     data;
    struct timeval **wt;
    fd_set*      LastSelectMask;
{
    static struct timeval recon_timeout;
    Time_t      now,
                soonest;
    FSFpePtr    recon;

    XFD_ORSET(LastSelectMask, LastSelectMask, &_fs_fd_mask);
    if (recon = awaiting_reconnect) {
	now = time((Time_t *) 0);
	soonest = recon->time_to_try;
	while (recon = recon->next_reconnect) {
	    if (recon->time_to_try < soonest)
		soonest = recon->time_to_try;
	}
	if (soonest < now)
	    soonest = now;
	soonest = soonest - now;
	recon_timeout.tv_sec = soonest;
	recon_timeout.tv_usec = 0;
	if (*wt == (struct timeval *) 0) {
	    *wt = &recon_timeout;
	} else if ((*wt)->tv_sec > soonest) {
	    **wt = recon_timeout;
	}
    }
}

static void
fs_handle_unexpected(conn, rep)
    FSFpePtr    conn;
    fsGenericReply *rep;
{
    if (rep->type == FS_Event && rep->data1 == KeepAlive) {
	fsNoopReq   req;

	/* ping it back */
	req.reqType = FS_Noop;
	req.length = SIZEOF(fsNoopReq) >> 2;
	_fs_add_req_log(conn, FS_Noop);
	_fs_write(conn, (char *) &req, SIZEOF(fsNoopReq));
    }
    /* this should suck up unexpected replies and events */
    _fs_eat_rest_of_error(conn, (fsError *) rep);
}

static int
fs_wakeup(fpe, LastSelectMask)
    FontPathElementPtr fpe;
    fd_set* LastSelectMask;
{
    FSBlockDataPtr blockrec,
                br;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    int         err;
    fsGenericReply rep;

    if (awaiting_reconnect) {
	_fs_try_reconnect();
    }

    /* see if there's any data to be read */

    /* 
     * Don't continue if the fd is -1 (which will be true when the
     * font server terminates
     */
    if (conn->fs_fd == -1)
	return FALSE;

    if (FD_ISSET(conn->fs_fd, LastSelectMask)) {

#if defined(NOTDEF) || defined(__EMX__)		/* bogus - doesn't deal with EOF very well,
				 * now does it ... */
	/*
	 * make sure it isn't spurious - mouse events seem to trigger extra
	 * problems. Under OS/2, this is especially true ...
	 */
	if (_fs_data_ready(conn) <= 0) {
	    return FALSE;
	}
#endif

	/* get the header */
	if (_fs_read(conn, (char *) &rep, SIZEOF(fsGenericReply)) == -1)
	    return FALSE;

	/* find the matching block record */

	for (br = (FSBlockDataPtr) conn->blocked_requests; br; br = br->next) {
	    if ((CARD16)(br->sequence_number & 0xffff) ==
		(CARD16)(rep.sequenceNumber - 1))
		break;
	}
	if (!br) {
	    fs_handle_unexpected(conn, &rep);
	    return FALSE;
	}
	blockrec = br;

	memcpy(&blockrec->header, &rep, SIZEOF(fsGenericReply));

	/* go read it, and if we're done, wake up the appropriate client */
	switch (blockrec->type) {
	case FS_OPEN_FONT:
	    err = fs_do_open_font(fpe, blockrec, FALSE);
	    break;
	case FS_LOAD_GLYPHS:
	    err = fs_read_glyphs(fpe, blockrec);
	    break;
	case FS_LIST_FONTS:
	    err = fs_read_list(fpe, blockrec);
	    break;
	case FS_LIST_WITH_INFO:
	    err = fs_read_list_info(fpe, blockrec);
	    break;
	default:
	    break;
	}

	if (err != StillWorking) {
	    while (blockrec) {
		ClientSignal(blockrec->client);
		blockrec = blockrec->depending;
	    }
	}
	/*
	 * Xx we could loop here and eat any additional replies, but it should
	 * feel more responsive for other clients if we come back later
	 */
    } else if (awaiting_reconnect) {
	_fs_try_reconnect();
    }
    return FALSE;
}

/*
 * Reconnection code
 */

void
_fs_connection_died(conn)
    FSFpePtr    conn;
{
    if (!conn->attemptReconnect)
	return;
    conn->attemptReconnect = FALSE;
    fs_close_conn(conn);
    conn->time_to_try = time((Time_t *) 0) + FS_RECONNECT_WAIT;
    conn->reconnect_delay = FS_RECONNECT_WAIT;
    conn->fs_fd = -1;
    conn->trans_conn = NULL;
    conn->next_reconnect = awaiting_reconnect;
    awaiting_reconnect = conn;
}

static int
_fs_restart_connection(conn)
    FSFpePtr    conn;
{
    FSBlockDataPtr block;

    conn->current_seq = 0;
    FD_SET(conn->fs_fd, &_fs_fd_mask);
    if (!fs_send_init_packets(conn))
	return FALSE;
    while (block = (FSBlockDataPtr) conn->blocked_requests) {
	ClientSignal(block->client);
	fs_abort_blockrec(conn, block);
    }
    return TRUE;
}

static void
_fs_try_reconnect()
{
    FSFpePtr    conn,
               *prev;
    Time_t      now;

    prev = &awaiting_reconnect;
    now = time((Time_t *) 0);
    while (conn = *prev) {
	if (now - conn->time_to_try > 0) {
	    if (_fs_reopen_server(conn) && _fs_restart_connection(conn)) {
		conn->attemptReconnect = TRUE;
		*prev = conn->next_reconnect;
		if (prev == &awaiting_reconnect) continue;
	    } else {
		if (conn->reconnect_delay < FS_MAX_RECONNECT_WAIT)
		    conn->reconnect_delay *= 2;
		now = time((Time_t *) 0);
		conn->time_to_try = now + conn->reconnect_delay;
	    }
	}
	prev = &conn->next_reconnect;
    }
}

/*
 * sends the actual request out
 */
/* ARGSUSED */
static int
fs_send_open_font(client, fpe, flags, name, namelen, format, fmask, id, ppfont)
    pointer     client;
    FontPathElementPtr fpe;
    Mask        flags;
    char       *name;
    int         namelen;
    fsBitmapFormat format;
    fsBitmapFormatMask fmask;
    XID         id;
    FontPtr    *ppfont;
{
    FontPtr     newfont;
    FSBlockDataPtr blockrec = NULL;
    FSBlockedFontPtr blockedfont;
    FSFontDataPtr fsd;
    FSFontPtr   fsfont;
    FSFpePtr    conn;
    fsOpenBitmapFontReq openreq;
    int         err = Suspended;
    XID         newid;
    unsigned char buf[1024];
    char       *fontname;

    if (flags & FontReopen)
    {
	Atom nameatom, fn = None;
	int i;

	newfont = *ppfont;
	fsd = (FSFontDataPtr)newfont->fpePrivate;
	fsfont = (FSFontPtr)newfont->fontPrivate;
	fpe = newfont->fpe;
	format = fsd->format;
	fmask = fsd->fmask;
	newid = fsd->fontid;
	/* This is an attempt to reopen a font.  Did the font have a
	   NAME property? */
	if ((nameatom = MakeAtom("FONT", 4, 0)) != None)
	{
	    for (i = 0; i < newfont->info.nprops; i++)
		if (newfont->info.props[i].name == nameatom &&
		    newfont->info.isStringProp[i])
		{
		    fn = newfont->info.props[i].value;
		    break;
		}
	}
	if (fn == None || !(name = NameForAtom(fn)))
	{
	    name = fsd->name;
	    namelen = fsd->namelen;
	}
	else
	    namelen = strlen(name);
    }

    conn = (FSFpePtr) fpe->private;
    if (namelen > sizeof (buf) - 1)
	return BadFontName;
    _fs_client_access (conn, client, (flags & FontOpenSync) != 0);
    _fs_client_resolution(conn);


    if (!(flags & FontReopen))
    {

	newid = GetNewFontClientID();

	/* make the font */
	newfont = (FontPtr) xalloc(sizeof(FontRec));

	/* and the FS data */
	fsd = (FSFontDataPtr) xalloc(sizeof(FSFontDataRec));

	fsfont = (FSFontPtr) xalloc(sizeof(FSFontRec));

	fontname = (char *)xalloc(namelen);

	if (!newfont || !fsd || !fsfont || !fontname) {
lowmem:
	    if (!(flags & FontReopen))
	    {
		xfree((char *) newfont);
		xfree((char *) fsd);
		xfree((char *) fsfont);
		xfree((char *) fontname);
	    }
	    if (blockrec) fs_abort_blockrec(conn, blockrec);
	    return AllocError;
	}
	bzero((char *) newfont, sizeof(FontRec));
	bzero((char *) fsfont, sizeof(FSFontRec));
	bzero((char *) fsd, sizeof(FSFontDataRec));
    }

    /* make a new block record, and add it to the end of the list */
    blockrec = fs_new_block_rec(fpe, client, FS_OPEN_FONT);
    if (!blockrec) {
	goto lowmem;
    }

    if (!(flags & FontReopen))
    {
	int bit, byte, scan, glyph;

	newfont->refcnt = 0;
	newfont->maxPrivate = -1;
	newfont->devPrivates = (pointer *) 0;
	newfont->format = format;

	/* These font components will be needed in packGlyphs */
	CheckFSFormat(format, BitmapFormatMaskBit |
			      BitmapFormatMaskByte |
			      BitmapFormatMaskScanLineUnit |
			      BitmapFormatMaskScanLinePad,
		      &bit,
		      &byte,
		      &scan,
		      &glyph,
		      NULL);
	newfont->bit = bit;
	newfont->byte = byte;
	newfont->scan = scan;
	newfont->glyph = glyph;

	newfont->fpe = fpe;
	newfont->fpePrivate = (pointer) fsd;
	newfont->fontPrivate = (pointer) fsfont;
	_fs_init_font(newfont);

	fsd->fpe = fpe;
	fsd->name = fontname;
	fsd->namelen = namelen;
	memcpy(fontname, name, namelen);
	fsd->format = format;
	fsd->fmask = fmask;
    }
    fsd->fontid = newid;
    fsd->generation = conn->generation;

    blockedfont = (FSBlockedFontPtr) blockrec->data;
    blockedfont->fontid = newid;
    blockedfont->pfont = newfont;
    blockedfont->state = FS_OPEN_REPLY;
    blockedfont->flags = flags;
    blockedfont->format = format;
    blockedfont->clients_depending = (FSClientsDependingPtr)0;

    /* save the ID */
    if (!StoreFontClientFont(blockedfont->pfont, blockedfont->fontid)) {
	goto lowmem;
    }
    /* do an FS_OpenFont, FS_QueryXInfo and FS_QueryXExtents */
    buf[0] = (unsigned char) namelen;
    memcpy(&buf[1], name, namelen);
    namelen++;
    openreq.reqType = FS_OpenBitmapFont;
    openreq.fid = newid;
    openreq.format_hint = format;
    openreq.format_mask = fmask;
    openreq.length = (SIZEOF(fsOpenBitmapFontReq) + namelen + 3) >> 2;

    _fs_add_req_log(conn, FS_OpenBitmapFont);
    _fs_write(conn, (char *) &openreq, SIZEOF(fsOpenBitmapFontReq));
    _fs_write_pad(conn, (char *) buf, namelen);

#ifdef NCD
    if (configData.ExtendedFontDiags) {
	memcpy(buf, name, MIN(256, namelen));
	buf[MIN(256, namelen)] = '\0';
	printf("Requesting font \"%s\" from font server \"%s\"\n",
	       buf, fpe->name);
    }
#endif

    if (flags & FontOpenSync) {
	err = fs_do_open_font(fpe, blockrec, TRUE);
	if (blockedfont->errcode == Successful) {
	    *ppfont = blockedfont->pfont;
	} else {
	    _fs_cleanup_font(blockedfont);
	}
	_fs_remove_block_rec(conn, blockrec);
    }
    return err;
}

static int
fs_send_query_info(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FSBlockedFontPtr bfont;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsQueryXInfoReq inforeq;

    bfont = (FSBlockedFontPtr) blockrec->data;

    inforeq.reqType = FS_QueryXInfo;
    inforeq.id = bfont->fontid;
    inforeq.length = SIZEOF(fsQueryXInfoReq) >> 2;

    blockrec->sequence_number = conn->current_seq;
    _fs_add_req_log(conn, FS_QueryXInfo);
    _fs_write(conn, (char *) &inforeq, SIZEOF(fsQueryXInfoReq));

    return Successful;
}

static int
fs_send_query_extents(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FSBlockedFontPtr bfont;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsQueryXExtents16Req extreq;

    bfont = (FSBlockedFontPtr) blockrec->data;

    extreq.reqType = FS_QueryXExtents16;
    extreq.range = fsTrue;
    extreq.fid = bfont->fontid;
    extreq.num_ranges = 0;
    extreq.length = SIZEOF(fsQueryXExtents16Req) >> 2;

    blockrec->sequence_number = conn->current_seq;
    _fs_add_req_log(conn, FS_QueryXExtents16);
    _fs_write(conn, (char *) &extreq, SIZEOF(fsQueryXExtents16Req));

    return Successful;
}

static int
fs_send_query_bitmaps(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FSBlockedFontPtr bfont;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsQueryXBitmaps16Req bitreq;


    bfont = (FSBlockedFontPtr) blockrec->data;

    /* send the request */
    bitreq.reqType = FS_QueryXBitmaps16;
    bitreq.fid = bfont->fontid;
    bitreq.format = bfont->format;
    bitreq.range = TRUE;
    bitreq.length = SIZEOF(fsQueryXBitmaps16Req) >> 2;
    bitreq.num_ranges = 0;

    blockrec->sequence_number = conn->current_seq;
    _fs_add_req_log(conn, FS_QueryXBitmaps16);
    _fs_write(conn, (char *) &bitreq, SIZEOF(fsQueryXBitmaps16Req));

    return Successful;
}

/* ARGSUSED */
static int
fs_open_font(client, fpe, flags, name, namelen, format, fmask, id, ppfont,
	     alias, non_cachable_font)
    pointer     client;
    FontPathElementPtr fpe;
    Mask        flags;
    char       *name;
    fsBitmapFormat format;
    fsBitmapFormatMask fmask;
    int         namelen;
    XID         id;
    FontPtr    *ppfont;
    char      **alias;
    FontPtr     non_cachable_font;	/* Not used in this FPE */
{
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    FSBlockDataPtr blockrec;
    FSBlockedFontPtr blockedfont;
    int         err;

    /* libfont interface expects ImageRectMin glyphs */
    format = format & ~BitmapFormatImageRectMask | BitmapFormatImageRectMin;

    *alias = (char *) 0;
    /* XX if we find the blockrec for the font */
    blockrec = (FSBlockDataPtr) conn->blocked_requests;
    while (blockrec != (FSBlockDataPtr) 0) {
	if (blockrec->type == FS_OPEN_FONT &&
		blockrec->client == client) {
	    blockedfont = (FSBlockedFontPtr) blockrec->data;
	    err = blockedfont->errcode;
	    if (err == Successful) {
		*ppfont = blockedfont->pfont;
	    } else {
		_fs_cleanup_font(blockedfont);
	    }
	    /* cleanup */
	    _fs_remove_block_rec(conn, blockrec);
	    return err;
	}
	blockrec = blockrec->next;
    }
    return fs_send_open_font(client, fpe, flags, name, namelen, format, fmask,
			     id, ppfont);
}

/* ARGSUSED */
static int
fs_send_close_font(fpe, id)
    FontPathElementPtr fpe;
    Font        id;
{
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsCloseReq  req;

    /* tell the font server to close the font */
    req.reqType = FS_CloseFont;
    req.length = SIZEOF(fsCloseReq) >> 2;
    req.id = id;
    _fs_add_req_log(conn, FS_CloseFont);
    _fs_write(conn, (char *) &req, SIZEOF(fsCloseReq));

    return Successful;
}

/* ARGSUSED */
static int
fs_close_font(fpe, pfont)
    FontPathElementPtr fpe;
    FontPtr     pfont;
{
    FSFontDataPtr fsd = (FSFontDataPtr) pfont->fpePrivate;
    FSFpePtr    conn = (FSFpePtr) fpe->private;

    /* XXX we may get called after the resource DB has been cleaned out */
    if (find_old_font(fsd->fontid))
	DeleteFontClientID(fsd->fontid);
    if (conn->generation == fsd->generation)
	fs_send_close_font(fpe, fsd->fontid);
    (*pfont->unload_font) (pfont);


    xfree(fsd->name);
    xfree(fsd);
    xfree(pfont->info.isStringProp);
    xfree(pfont->info.props);
    xfree(pfont->devPrivates);
    xfree(pfont);


    return Successful;
}

static int
fs_read_glyphs(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FSBlockedGlyphPtr bglyph = (FSBlockedGlyphPtr) blockrec->data;
    FSBlockedFontPtr bfont = (FSBlockedFontPtr) blockrec->data;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    FontPtr pfont = bglyph->pfont;      /* works for either blocked font
					   or glyph rec...  pfont is at
					   the very beginning of both
					   blockrec->data structures */
    FSFontDataPtr fsd = (FSFontDataPtr) (pfont->fpePrivate);
    FSFontPtr   fsdata = (FSFontPtr) pfont->fontPrivate;
    FontInfoPtr	pfi = &pfont->info;
    fsQueryXBitmaps16Reply rep;
    fsOffset32   *ppbits;
    fsOffset32	local_off;
    char	*off_adr;
    pointer     pbitmaps;
    char	*bits;
    int         glyph_size,
                offset_size,
                i,
		err;
    int		nranges = 0;
    fsRange	*ranges, *nextrange;
    unsigned long minchar, maxchar;

    /* get reply header */
    memcpy(&rep, &blockrec->header, SIZEOF(fsGenericReply));
    if (rep.type == FS_Error) {
/* XXX -- translate FS error */
	_fs_eat_rest_of_error(conn, (fsError *) & rep);
	err = AllocError;
	goto bail;
    }
    if (_fs_read(conn, (char *) &rep + SIZEOF(fsGenericReply),
	      SIZEOF(fsQueryXBitmaps16Reply) - SIZEOF(fsGenericReply)) == -1) {
	if (blockrec->type == FS_OPEN_FONT)
	    fs_free_font(bfont);
	return StillWorking;
    }
    /* allocate space for glyphs */
    offset_size = SIZEOF(fsOffset32) * (rep.num_chars);
    glyph_size = (rep.length << 2) - SIZEOF(fsQueryXBitmaps16Reply)
	- offset_size;
    ppbits = (fsOffset32 *) xalloc(offset_size);
    pbitmaps = (pointer) xalloc(glyph_size);
    if (glyph_size && !pbitmaps || !ppbits)
    {
	xfree(pbitmaps);
	xfree(ppbits);

	/* clear wire */
	(void) _fs_drain_bytes_pad(conn, offset_size);
	(void) _fs_drain_bytes_pad(conn, glyph_size);

	if (blockrec->type == FS_OPEN_FONT)
	    _fs_cleanup_font(bfont);
	err = AllocError;
	goto bail;
    }

    /* read offsets */
    if (_fs_read_pad(conn, (char *) ppbits, offset_size) == -1) {
	if (blockrec->type == FS_OPEN_FONT)
	    fs_free_font(bfont);
	return StillWorking;
    }

    /* read glyphs */
    if (_fs_read_pad(conn, (char *) pbitmaps, glyph_size) == -1) {
	if (blockrec->type == FS_OPEN_FONT)
	    fs_free_font(bfont);
	return StillWorking;
    }

    if (blockrec->type == FS_LOAD_GLYPHS)
    {
	nranges = bglyph->num_expected_ranges;
	nextrange = ranges = bglyph->expected_ranges;
    }

    /* place the incoming glyphs */
    if (nranges)
    {
	/* We're operating under the assumption that the ranges
	   requested in the LoadGlyphs call were all legal for this
	   font, and that individual ranges do not cover multiple
	   rows...  fs_build_range() is designed to ensure this. */
	minchar = (nextrange->min_char_high - pfi->firstRow) *
		  (pfi->lastCol - pfi->firstCol + 1) +
		  nextrange->min_char_low - pfi->firstCol;
	maxchar = (nextrange->max_char_high - pfi->firstRow) *
		  (pfi->lastCol - pfi->firstCol + 1) +
		  nextrange->max_char_low - pfi->firstCol;
	nextrange++;
    }
    else
    {
	minchar = 0;
	maxchar = rep.num_chars;
    }

    off_adr = (char *)ppbits;
    for (i = 0; i < rep.num_chars; i++)
    {
	memcpy(&local_off, off_adr, SIZEOF(fsOffset32));	/* align it */
	if (blockrec->type == FS_OPEN_FONT ||
	    fsdata->encoding[minchar].bits == &_fs_glyph_requested)
	{
	    if (local_off.length)
	    {
		bits = (char *)xalloc(local_off.length);
		if (bits == NULL)
		{
		    xfree(ppbits);
		    xfree(pbitmaps);
		    err = AllocError;
		    goto bail;
		}
		memcpy(bits, pbitmaps + local_off.position,
		       local_off.length);
	    }
	    else if (NONZEROMETRICS(&fsdata->encoding[minchar].metrics))
		bits = &_fs_glyph_zero_length;
	    else
		bits = 0;
	    if (fsdata->encoding[minchar].bits == &_fs_glyph_requested)
		fsd->glyphs_to_get--;
	    fsdata->encoding[minchar].bits = bits;
	}
	if (minchar++ == maxchar)
	{
	    if (!--nranges) break;
	    minchar = (nextrange->min_char_high - pfi->firstRow) *
		      (pfi->lastCol - pfi->firstCol + 1) +
		      nextrange->min_char_low - pfi->firstCol;
	    maxchar = (nextrange->max_char_high - pfi->firstRow) *
		      (pfi->lastCol - pfi->firstCol + 1) +
		      nextrange->max_char_low - pfi->firstCol;
	    nextrange++;
	}
	off_adr += SIZEOF(fsOffset32);
    }

    xfree(ppbits);
    xfree(pbitmaps);

    if (blockrec->type == FS_OPEN_FONT)
    {
	fsd->glyphs_to_get = 0;
	bfont->state = FS_DONE_REPLY;
    }
    err = Successful;

bail:
    if (blockrec->type == FS_LOAD_GLYPHS)
    {
	bglyph->done = TRUE;
	bglyph->errcode = err;
    }

    return err;
}



static int
fs_send_load_glyphs(client, pfont, nranges, ranges)
    pointer     client;
    FontPtr     pfont;
    int		nranges;
    fsRange	*ranges;
{
    FSBlockedGlyphPtr blockedglyph;
    fsQueryXBitmaps16Req req;
    FSFontDataPtr fsd = (FSFontDataPtr) (pfont->fpePrivate);
    FontPathElementPtr fpe = fsd->fpe;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    FSBlockDataPtr blockrec;

    /* make a new block record, and add it to the end of the list */
    blockrec = fs_new_block_rec(fpe, client, FS_LOAD_GLYPHS);
    if (!blockrec)
	return AllocError;
    blockedglyph = (FSBlockedGlyphPtr) blockrec->data;
    blockedglyph->pfont = pfont;
    blockedglyph->num_expected_ranges = nranges;
    /* Assumption: it's our job to free ranges */
    blockedglyph->expected_ranges = ranges;
    blockedglyph->done = FALSE;
    blockedglyph->clients_depending = (FSClientsDependingPtr)0;

    blockrec->sequence_number = conn->current_seq;

    /* send the request */
    req.reqType = FS_QueryXBitmaps16;
    req.fid = ((FSFontDataPtr) pfont->fpePrivate)->fontid;
    req.format = pfont->format;
    if (pfont->info.terminalFont)
	req.format = req.format & ~(BitmapFormatImageRectMask) |
		     BitmapFormatImageRectMax;
    req.range = TRUE;
    /* each range takes up 4 bytes */
    req.length = (SIZEOF(fsQueryXBitmaps16Req) >> 2) + nranges;
    req.num_ranges = nranges * 2;	/* protocol wants count of fsChar2bs */
    _fs_add_req_log(conn, FS_QueryXBitmaps16);
    _fs_write(conn, (char *) &req, SIZEOF(fsQueryXBitmaps16Req));

    /* Send ranges to the server... pack into a char array by hand
       to avoid structure-packing portability problems and to
       handle swapping for version1 protocol */
    if (nranges)
    {
#define RANGE_BUFFER_SIZE 64
#define RANGE_BUFFER_SIZE_MASK 63
	int i;
	char range_buffer[RANGE_BUFFER_SIZE * 4];
	char *range_buffer_p;

	range_buffer_p = range_buffer;
	for (i = 0; i < nranges;)
	{
	    if (conn->fsMajorVersion > 1)
	    {
		*range_buffer_p++ = ranges[i].min_char_high;
		*range_buffer_p++ = ranges[i].min_char_low;
		*range_buffer_p++ = ranges[i].max_char_high;
		*range_buffer_p++ = ranges[i].max_char_low;
	    }
	    else
	    {
		*range_buffer_p++ = ranges[i].min_char_low;
		*range_buffer_p++ = ranges[i].min_char_high;
		*range_buffer_p++ = ranges[i].max_char_low;
		*range_buffer_p++ = ranges[i].max_char_high;
	    }

	    if (!(++i & RANGE_BUFFER_SIZE_MASK))
	    {
		_fs_write(conn, range_buffer, RANGE_BUFFER_SIZE * 4);
		range_buffer_p = range_buffer;
	    }
	}
	if (i &= RANGE_BUFFER_SIZE_MASK)
	    _fs_write(conn, range_buffer, i * 4);
    }

    return Suspended;
}


int
fs_load_all_glyphs(pfont)
    FontPtr	pfont;
{
    extern pointer serverClient;	/* This could be any number that
					   doesn't conflict with existing
					   client values. */
    int err;
    FSFpePtr conn = (FSFpePtr) pfont->fpe->private;

    /*
     * The purpose of this procedure is to load all glyphs in the event
     * that we're dealing with someone who doesn't understand the finer
     * points of glyph caching...  it is called from _fs_get_glyphs() if
     * the latter is called to get glyphs that have not yet been loaded.
     * We assume that the caller will not know how to handle a return
     * value of Suspended (usually the case for a GetGlyphs() caller),
     * so this procedure hangs around, freezing the server, for the
     * request to complete.  This is an unpleasant kluge called to
     * perform an unpleasant job that, we hope, will never be required.
     */

    while ((err = _fs_load_glyphs(serverClient, pfont, TRUE, 0, 0, NULL)) ==
	   Suspended)
    {
	fd_set TempSelectMask;
	if (_fs_wait_for_readable(conn) == -1)
	{
	    /* We lost our connection.  Don't wait to reestablish it;
	       just give up. */
	    _fs_connection_died(conn);

	    /* Get rid of blockrec */
	    fs_client_died(serverClient, pfont->fpe);

	    return BadCharRange;	/* As good an error as any other */
	}
	FD_SET(conn->fs_fd, &TempSelectMask);
	fs_wakeup(pfont->fpe, &TempSelectMask);
    }

    return err;
}


int
_fs_load_glyphs(client, pfont, range_flag, nchars, item_size, data)
    pointer     client;
    FontPtr     pfont;
    Bool	range_flag;
    unsigned int nchars;
    int         item_size;
    unsigned char *data;
{

    int		nranges = 0;
    fsRange     *ranges = NULL;
    int         res;
    FSBlockDataPtr blockrec;
    FSBlockedGlyphPtr blockedglyph;
    FSFpePtr    conn = (FSFpePtr) pfont->fpe->private;
    FSClientsDependingPtr *clients_depending = NULL;

    /* see if the result is already there */

    blockrec = (FSBlockDataPtr) conn->blocked_requests;
    while (blockrec) {
	if (blockrec->type == FS_LOAD_GLYPHS)
	{
	    blockedglyph = (FSBlockedGlyphPtr) blockrec->data;
	    if (blockedglyph->pfont == pfont)
	    {
		if (blockrec->client == client)
		{
		    if (blockedglyph->done)
		    {
			int errcode = blockedglyph->errcode;
			signal_clients_depending(&blockedglyph->
						 clients_depending);
			_fs_remove_block_rec(conn, blockrec);
			return errcode;
		    }
		    else return Suspended;
		}
		/* We've found an existing LoadGlyphs blockrec for this
		   font but for another client.  Rather than build a
		   blockrec for it now (which entails some complex
		   maintenance), we'll add it to a queue of clients to
		   be signalled when the existing LoadGlyphs is
		   completed.  */
		clients_depending = &blockedglyph->clients_depending;
		break;
	    }
	}
	else if (blockrec->type == FS_OPEN_FONT)
	{
	    FSBlockedFontPtr bfont;
	    bfont = (FSBlockedFontPtr) blockrec->data;
	    if (bfont->pfont == pfont)
	    {
		if (blockrec->client == client)
		{
		    if (bfont->state == FS_DONE_REPLY)
		    {
			int errcode = bfont->errcode;
			signal_clients_depending(&bfont->clients_depending);
			_fs_remove_block_rec(conn, blockrec);
			if (errcode == Successful) break;
			else return errcode;
		    }
		    else return Suspended;
		}
		/* We've found an existing OpenFont blockrec for this
		   font but for another client.  Rather than build a
		   blockrec for it now (which entails some complex
		   maintenance), we'll add it to a queue of clients to
		   be signalled when the existing OpenFont is
		   completed.  */
		if (bfont->state != FS_DONE_REPLY)
		{
		    clients_depending = &bfont->clients_depending;
		    break;
		}
	    }
	}
		 
	blockrec = blockrec->next;
    }

    /*
     * see if the desired glyphs already exist, and return Successful if they
     * do, otherwise build up character range/character string
     */
    res = fs_build_range(pfont, range_flag, nchars, item_size, data,
			 &nranges, &ranges);

    switch (res)
    {
	case AccessDone:
	    return Successful;

	case Successful:
	    break;

	default:
	    return res;
    }

    /*
     * If clients_depending is not null, this request must wait for
     * some prior request(s) to complete.
     */
    if (clients_depending)
    {
	/* Since we're not ready to send the load_glyphs request yet,
	   clean up the damage (if any) caused by the fs_build_range()
	   call. */
	if (nranges)
	{
	    _fs_clean_aborted_loadglyphs(pfont, nranges, ranges);
	    xfree(ranges);
	}
	return add_clients_depending(clients_depending, client);
    }

    /*
     * If fsd->generation != conn->generation, the font has been closed
     * due to a lost connection.  We will reopen it, which will result
     * in one of three things happening:
     *	 1) The open will succeed and obtain the same font.  Life
     *	    is wonderful.
     *	 2) The open will fail.  There is code above to recognize this
     *	    and flunk the LoadGlyphs request.  The client might not be
     *	    thrilled.
     *	 3) Worst case: the open will succeed but the font we open will
     *	    be different.  The fs_read_query_info() procedure attempts
     *	    to detect this by comparing the existing metrics and
     *	    properties against those of the reopened font... if they
     *	    don't match, we flunk the reopen, which eventually results
     *	    in flunking the LoadGlyphs request.  We could go a step
     *	    further and compare the extents, but this should be
     *	    sufficient.
     */
    if (((FSFontDataPtr)pfont->fpePrivate)->generation != conn->generation)
    {
	/* Since we're not ready to send the load_glyphs request yet,
	   clean up the damage caused by the fs_build_range() call. */
	_fs_clean_aborted_loadglyphs(pfont, nranges, ranges);
	xfree(ranges);

	/* Now try to reopen the font. */
	return fs_send_open_font(client, (FontPathElementPtr)0,
				 (Mask)FontReopen, (char *)0, 0,
				 (fsBitmapFormat)0, (fsBitmapFormatMask)0,
				 (XID)0, &pfont);
    }

    return fs_send_load_glyphs(client, pfont, nranges, ranges);
}



static int
fs_read_list(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FSBlockedListPtr blist = (FSBlockedListPtr) blockrec->data;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsListFontsReply rep;
    char       *data,
               *dp;
    int         length,
                i;

    blist->done = TRUE;

    /* read reply header */
    memcpy(&rep, &blockrec->header, SIZEOF(fsGenericReply));
    if (rep.type == FS_Error) {
/* XXX -- translate FS error */
	_fs_eat_rest_of_error(conn, (fsError *) & rep);
	return AllocError;
    }
    if (_fs_read(conn, (char *) &rep + SIZEOF(fsGenericReply),
		 SIZEOF(fsListFontsReply) - SIZEOF(fsGenericReply)) == -1) {
	/* nothing to free (i think) */
	return StillWorking;
    }
    length = (rep.length << 2) - SIZEOF(fsListFontsReply);
    data = (char *) xalloc(length);
    if (!data) {
	_fs_drain_bytes_pad(conn, length);
	return AllocError;
    }
    /* read the list */
    if (_fs_read_pad(conn, data, length) == -1) {
	/* nothing to free (i think) */
	return StillWorking;
    }
    /* copy data into FontPathRecord */
    dp = data;
    for (i = 0; i < rep.nFonts; i++) {
	length = *(unsigned char *)dp++;
	if (AddFontNamesName(blist->names, dp, length) != Successful) {
	    blist->errcode = AllocError;
	    break;
	}
	dp += length;
    }

    xfree(data);
    return Successful;
}

static int
fs_send_list_fonts(client, fpe, pattern, patlen, maxnames, newnames)
    pointer     client;
    FontPathElementPtr fpe;
    char       *pattern;
    int         patlen;
    int         maxnames;
    FontNamesPtr newnames;
{
    FSBlockDataPtr blockrec;
    FSBlockedListPtr blockedlist;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsListFontsReq req;

    _fs_client_access (conn, client, FALSE);
    _fs_client_resolution(conn);

    /* make a new block record, and add it to the end of the list */
    blockrec = fs_new_block_rec(fpe, client, FS_LIST_FONTS);
    if (!blockrec)
	return AllocError;
    blockedlist = (FSBlockedListPtr) blockrec->data;
    blockedlist->patlen = patlen;
    blockedlist->errcode = Successful;
    blockedlist->names = newnames;
    blockedlist->done = FALSE;

    /* send the request */
    req.reqType = FS_ListFonts;
    req.maxNames = maxnames;
    req.nbytes = patlen;
    req.length = (SIZEOF(fsListFontsReq) + patlen + 3) >> 2;
    _fs_add_req_log(conn, FS_ListFonts);
    _fs_write(conn, (char *) &req, SIZEOF(fsListFontsReq));
    _fs_write_pad(conn, (char *) pattern, patlen);

#ifdef NCD
    if (configData.ExtendedFontDiags) {
	char        buf[256];

	memcpy(buf, pattern, MIN(256, patlen));
	buf[MIN(256, patlen)] = '\0';
	printf("Listing fonts on pattern \"%s\" from font server \"%s\"\n",
	       buf, fpe->name);
    }
#endif

    return Suspended;
}

static int
fs_list_fonts(client, fpe, pattern, patlen, maxnames, newnames)
    pointer     client;
    FontPathElementPtr fpe;
    char       *pattern;
    int         patlen;
    int         maxnames;
    FontNamesPtr newnames;
{
    FSBlockDataPtr blockrec;
    FSBlockedListPtr blockedlist;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    int         err;

    /* see if the result is already there */
    blockrec = (FSBlockDataPtr) conn->blocked_requests;
    while (blockrec) {
	if (blockrec->type == FS_LIST_FONTS && blockrec->client == client) {
	    blockedlist = (FSBlockedListPtr) blockrec->data;
	    if (blockedlist->patlen == patlen && blockedlist->done) {
		err = blockedlist->errcode;
		_fs_remove_block_rec(conn, blockrec);
		return err;
	    }
	}
	blockrec = blockrec->next;
    }

    /* didn't find waiting record, so send a new one */
    return fs_send_list_fonts(client, fpe, pattern, patlen, maxnames, newnames);
}

static int  padlength[4] = {0, 3, 2, 1};

static int
fs_read_list_info(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FSBlockedListInfoPtr binfo = (FSBlockedListInfoPtr) blockrec->data;
    fsListFontsWithXInfoReply rep;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsPropInfo  pi;
    fsPropOffset *po;
    char       *name;
    pointer     pd;
    int		err;

    /* clean up anything from the last trip */
    if (binfo->name)
    {
	xfree(binfo->name);
	binfo->name = NULL;
    }
    if (binfo->pfi) {
	xfree(binfo->pfi->isStringProp);
	xfree(binfo->pfi->props);
	xfree(binfo->pfi);
	binfo->pfi = NULL;
    }
    /* get reply header */
    memcpy(&rep, &blockrec->header, SIZEOF(fsGenericReply));
    if (rep.type == FS_Error) {
/* XXX -- translate FS error */
	_fs_eat_rest_of_error(conn, (fsError *) & rep);
	binfo->errcode = AllocError;
	return AllocError;
    }
    if (conn->fsMajorVersion > 1)
	if (rep.nameLength == 0)
	    goto done;
    /* old protocol sent a full-length reply even for the last one */
    if (_fs_read(conn, (char *) &rep + SIZEOF(fsGenericReply),
	  SIZEOF(fsListFontsWithXInfoReply) - SIZEOF(fsGenericReply)) == -1) {
	goto done;
    }
    if (rep.nameLength == 0)
	goto done;

    /* read the data */
    name = (char *) xalloc(rep.nameLength);
    binfo->pfi = (FontInfoPtr) xalloc(sizeof(FontInfoRec));
    if (!name || !binfo->pfi) {
	xfree(name);
	xfree(binfo->pfi);
	binfo->pfi = NULL;
	_fs_drain_bytes(conn,
			rep.length - (SIZEOF(fsListFontsWithXInfoReply) -
				      SIZEOF(fsGenericReply)));
	binfo->errcode = AllocError;
	return AllocError;
    }
    if (conn->fsMajorVersion == 1)
	if (_fs_read_pad(conn, name, rep.nameLength) == -1)
	    goto done;
    if (_fs_read_pad(conn, (char *) &pi, SIZEOF(fsPropInfo)) == -1)
	    goto done;

    po = (fsPropOffset *) xalloc(SIZEOF(fsPropOffset) * pi.num_offsets);
    pd = (pointer) xalloc(pi.data_len);
    if (!po || !pd) {
	xfree(name);
	xfree(po);
	xfree(pd);
	xfree (binfo->pfi);
	binfo->pfi = NULL;
	binfo->errcode = AllocError;
	return AllocError;
    }
    err = _fs_read_pad(conn, (char *) po,
		       (pi.num_offsets * SIZEOF(fsPropOffset)));
    if (err != -1)
    {
	if (conn->fsMajorVersion > 1)
	    err = _fs_read(conn, (char *) pd, pi.data_len);
	else
	    err = _fs_read_pad(conn, (char *) pd, pi.data_len);
    }
    if (err != -1  &&  conn->fsMajorVersion != 1)
    {
	err = _fs_read(conn, name, rep.nameLength);
	if (err != -1)
	    err = _fs_drain_bytes(conn, padlength[(pi.data_len+rep.nameLength)&3]);
    }

    if (err == -1) {
	xfree(name);
	xfree(po);
	xfree(pd);
	xfree (binfo->pfi);
	binfo->pfi = NULL;
	goto done;
    }

    if (_fs_convert_lfwi_reply(conn, binfo->pfi, &rep, &pi, po, pd) != Successful)
    {
	xfree(name);
	xfree(po);
	xfree(pd);
	xfree (binfo->pfi);
	binfo->pfi = NULL;
	goto done;
    }
    xfree(po);
    xfree(pd);
    binfo->name = name;
    binfo->namelen = rep.nameLength;
    binfo->remaining = rep.nReplies;

    binfo->status = FS_LFWI_REPLY;
    binfo->errcode = Suspended;
    /* disable this font server until we've processed this response */
    FD_CLR(conn->fs_fd, &_fs_fd_mask);

    return Successful;

done:
    binfo->status = FS_LFWI_FINISHED;
    binfo->errcode = BadFontName;
    binfo->name = (char *) 0;
    return Successful;
}

/* ARGSUSED */
static int
fs_start_list_with_info(client, fpe, pattern, len, maxnames, pdata)
    pointer     client;
    FontPathElementPtr fpe;
    char       *pattern;
    int         len;
    int         maxnames;
    pointer    *pdata;
{
    FSBlockDataPtr blockrec;
    FSBlockedListInfoPtr blockedinfo;
    fsListFontsWithXInfoReq req;
    FSFpePtr    conn = (FSFpePtr) fpe->private;

    _fs_client_access (conn, client, FALSE);
    _fs_client_resolution(conn);

    /* make a new block record, and add it to the end of the list */
    blockrec = fs_new_block_rec(fpe, client, FS_LIST_WITH_INFO);
    if (!blockrec)
	return AllocError;
    blockedinfo = (FSBlockedListInfoPtr) blockrec->data;
    bzero((char *) blockedinfo, sizeof(FSBlockedListInfoRec));
    blockedinfo->status = FS_LFWI_WAITING;
    blockedinfo->errcode = Suspended;

    /* send the request */
    req.reqType = FS_ListFontsWithXInfo;
    req.maxNames = maxnames;
    req.nbytes = len;
    req.length = (SIZEOF(fsListFontsWithXInfoReq) + len + 3) >> 2;
    _fs_add_req_log(conn, FS_ListFontsWithXInfo);
    (void) _fs_write(conn, (char *) &req, SIZEOF(fsListFontsWithXInfoReq));
    (void) _fs_write_pad(conn, pattern, len);

#ifdef NCD
    if (configData.ExtendedFontDiags) {
	char        buf[256];

	memcpy(buf, pattern, MIN(256, len));
	buf[MIN(256, len)] = '\0';
	printf("Listing fonts with info on pattern \"%s\" from font server \"%s\"\n",
	       buf, fpe->name);
    }
#endif

    return Successful;
}

/* ARGSUSED */
static int
fs_next_list_with_info(client, fpe, namep, namelenp, pFontInfo, numFonts,
		       private)
    pointer     client;
    FontPathElementPtr fpe;
    char      **namep;
    int        *namelenp;
    FontInfoPtr *pFontInfo;
    int        *numFonts;
    pointer     private;
{
    FSBlockDataPtr blockrec;
    FSBlockedListInfoPtr blockedinfo;
    FSFpePtr    conn = (FSFpePtr) fpe->private;

    /* see if the result is already there */
    blockrec = (FSBlockDataPtr) conn->blocked_requests;
    while (blockrec) {
	if (blockrec->type == FS_LIST_WITH_INFO &&
		blockrec->client == client) {
	    blockedinfo = (FSBlockedListInfoPtr) blockrec->data;
	    break;
	}
	blockrec = blockrec->next;
    }

    if (!blockrec)
    {
	/* The only good reason for not finding a blockrec would be if
	   disconnect/reconnect to the font server wiped it out and the
	   code that called us didn't do the right thing to create
	   another one.  Under those circumstances, we need to return an
	   error to prevent that code from attempting to interpret the
	   information we don't return.  */
	return BadFontName;
    }

    if (blockedinfo->status == FS_LFWI_WAITING)
	return Suspended;

    *namep = blockedinfo->name;
    *namelenp = blockedinfo->namelen;
    *pFontInfo = blockedinfo->pfi;
    *numFonts = blockedinfo->remaining;
    FD_SET(conn->fs_fd, &_fs_fd_mask);
    if (blockedinfo->status == FS_LFWI_FINISHED) {
	int         err = blockedinfo->errcode;

	_fs_remove_block_rec(conn, blockrec);
	return err;
    }
    if (blockedinfo->status == FS_LFWI_REPLY) {
	blockedinfo->status = FS_LFWI_WAITING;
	return Successful;
    } else {
	return blockedinfo->errcode;
    }
}

/*
 * Called when client exits
 */

static void
fs_client_died(client, fpe)
    pointer     client;
    FontPathElementPtr fpe;
{
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    FSBlockDataPtr blockrec,
                depending;
    FSClientPtr	*prev, cur;
    fsFreeACReq	freeac;

    for (prev = &conn->clients; cur = *prev; prev = &cur->next)
    {
	if (cur->client == client) {
	    freeac.reqType = FS_FreeAC;
	    freeac.id = cur->acid;
	    freeac.length = sizeof (fsFreeACReq) >> 2;
	    _fs_add_req_log(conn, FS_FreeAC);
	    _fs_write (conn, (char *) &freeac, sizeof (fsFreeACReq));
	    *prev = cur->next;
	    xfree (cur);
	    break;
	}
    }
    /* see if the result is already there */
    blockrec = (FSBlockDataPtr) conn->blocked_requests;
    while (blockrec) {
	if (blockrec->client == client)
	    break;
	blockrec = blockrec->next;
    }
    if (!blockrec)
	return;
    if (blockrec->type == FS_LIST_WITH_INFO)
    {
	FSBlockedListInfoPtr binfo;
	binfo = (FSBlockedListInfoPtr) blockrec->data;
	if (binfo->status == FS_LFWI_REPLY)
	    FD_SET(conn->fs_fd, &_fs_fd_mask);
    	if (binfo->name)
	{
	    xfree(binfo->name);
	    binfo->name = NULL;
	}
    	if (binfo->pfi) 
	{
	    xfree(binfo->pfi->isStringProp);
	    xfree(binfo->pfi->props);
	    xfree(binfo->pfi);
	    binfo->pfi = NULL;
    	}
    }
    /* replace the client pointers in this block rec with the chained one */
    if (depending = blockrec->depending) {
	blockrec->client = depending->client;
	blockrec->depending = depending->depending;
	blockrec = depending;
    }
    fs_abort_blockrec(conn, blockrec);
}

static void
_fs_client_access (conn, client, sync)
    FSFpePtr	conn;
    pointer	client;
    Bool	sync;
{
    FSClientPtr	*prev,	    cur;
    fsCreateACReq	    crac;
    fsSetAuthorizationReq   setac;
    fsGenericReply	    rep;
    char		    *authorizations;
    int			    authlen;
    Bool		    new_cur = FALSE;

    for (prev = &conn->clients; cur = *prev; prev = &cur->next)
    {
	if (cur->client == client)
	{
	    if (prev != &conn->clients)
	    {
		*prev = cur->next;
		cur->next = conn->clients;
		conn->clients = cur;
	    }
	    break;
	}
    }
    if (!cur)
    {
	cur = (FSClientPtr) xalloc (sizeof (FSClientRec));
	if (!cur)
	    return;
	cur->client = client;
	cur->next = conn->clients;
	conn->clients = cur;
	cur->acid = GetNewFontClientID ();
	new_cur = TRUE;
    }
    if (new_cur || cur->auth_generation != client_auth_generation(client))
    {
	if (!new_cur)
	{
	    fsFreeACReq	freeac;
	    freeac.reqType = FS_FreeAC;
	    freeac.id = cur->acid;
	    freeac.length = sizeof (fsFreeACReq) >> 2;
	    _fs_add_req_log(conn, FS_FreeAC);
	    _fs_write (conn, (char *) &freeac, sizeof (fsFreeACReq));
	}
	crac.reqType = FS_CreateAC;
	crac.num_auths = set_font_authorizations(&authorizations, &authlen,
						 client);
	authlen = crac.num_auths ? (authlen + 3) & ~0x3 : 0;
	crac.length = (sizeof (fsCreateACReq) + authlen) >> 2;
	crac.acid = cur->acid;
	_fs_add_req_log(conn, FS_CreateAC);
	_fs_write(conn, (char *) &crac, sizeof (fsCreateACReq));
	_fs_write(conn, authorizations, authlen);
	/* if we're synchronous, open_font will be confused by
	 * the reply; eat it and continue
	 */
	if (sync)
	{
	    if (_fs_read(conn, (char *) &rep, sizeof (fsGenericReply)) == -1)
		return;
	    fs_handle_unexpected(conn, &rep);
	}
	/* ignore reply; we don't even care about it */
	conn->curacid = 0;
	cur->auth_generation = client_auth_generation(client);
    }
    if (conn->curacid != cur->acid)
    {
    	setac.reqType = FS_SetAuthorization;
    	setac.length = sizeof (fsSetAuthorizationReq) >> 2;
    	setac.id = cur->acid;
    	_fs_add_req_log(conn, FS_SetAuthorization);
    	_fs_write(conn, (char *) &setac, sizeof (fsSetAuthorizationReq));
	conn->curacid = cur->acid;
    }
}

/*
 * called at server init time
 */

void
fs_register_fpe_functions()
{
    fs_font_type = RegisterFPEFunctions(fs_name_check,
					fs_init_fpe,
					fs_free_fpe,
					fs_reset_fpe,
					fs_open_font,
					fs_close_font,
					fs_list_fonts,
					fs_start_list_with_info,
					fs_next_list_with_info,
					fs_wakeup,
					fs_client_died,
					_fs_load_glyphs,
					(int (*))0,
					(int (*))0,
					(void (*))0);
}

static int
check_fs_open_font(client, fpe, flags, name, namelen, format, fmask, id, ppfont,
	     alias, non_cachable_font)
    pointer     client;
    FontPathElementPtr fpe;
    Mask        flags;
    char       *name;
    fsBitmapFormat format;
    fsBitmapFormatMask fmask;
    int         namelen;
    XID         id;
    FontPtr    *ppfont;
    char      **alias;
    FontPtr     non_cachable_font;	/* Not used in this FPE */
{
    if (XpClientIsBitmapClient(client))
	return (fs_open_font(client, fpe, flags, name, namelen, format, 
			fmask, id, ppfont, alias, non_cachable_font) );
    return BadFontName;
}

static int
check_fs_list_fonts(client, fpe, pattern, patlen, maxnames, newnames)
    pointer     client;
    FontPathElementPtr fpe;
    char       *pattern;
    int         patlen;
    int         maxnames;
    FontNamesPtr newnames;
{
    if (XpClientIsBitmapClient(client))
	return (fs_list_fonts(client, fpe, pattern, patlen, maxnames, 
		newnames));
    return BadFontName;
}

static int
check_fs_start_list_with_info(client, fpe, pattern, len, maxnames, pdata)
    pointer     client;
    FontPathElementPtr fpe;
    char       *pattern;
    int         len;
    int         maxnames;
    pointer    *pdata;
{
    if (XpClientIsBitmapClient(client))
	return (fs_start_list_with_info(client, fpe, pattern, len, maxnames,
		pdata));
    return BadFontName;
}

static int
check_fs_next_list_with_info(client, fpe, namep, namelenp, pFontInfo, numFonts,
		       private)
    pointer     client;
    FontPathElementPtr fpe;
    char      **namep;
    int        *namelenp;
    FontInfoPtr *pFontInfo;
    int        *numFonts;
    pointer     private;
{
    if (XpClientIsBitmapClient(client))
	return (fs_next_list_with_info(client, fpe, namep, namelenp, pFontInfo, 
		numFonts,private));
    return BadFontName;
}

void
check_fs_register_fpe_functions()
{
    fs_font_type = RegisterFPEFunctions(fs_name_check,
					fs_init_fpe,
					fs_free_fpe,
					fs_reset_fpe,
					check_fs_open_font,
					fs_close_font,
					check_fs_list_fonts,
					check_fs_start_list_with_info,
					check_fs_next_list_with_info,
					fs_wakeup,
					fs_client_died,
					_fs_load_glyphs,
					(int (*))0,
					(int (*))0,
					(void (*))0);
}
