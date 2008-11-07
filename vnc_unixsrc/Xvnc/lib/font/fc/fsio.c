/* $XConsortium: fsio.c,v 1.37 95/04/05 19:58:13 kaleb Exp $ */
/* $XFree86: xc/lib/font/fc/fsio.c,v 3.5.2.1 1998/02/15 16:08:40 hohndel Exp $ */
/*
 * Copyright 1990 Network Computing Devices
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Network Computing Devices not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  Network Computing
 * Devices makes no representations about the suitability of this software
 * for any purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * NETWORK COMPUTING DEVICES DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL NETWORK COMPUTING DEVICES BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  	Dave Lemke, Network Computing Devices, Inc
 */
/*
 * font server i/o routines
 */

#ifdef WIN32
#define _WILLWINSOCK_
#endif

#include	"FS.h"
#include	"FSproto.h"

#include 	"X11/Xtrans.h"
#include	"X11/Xpoll.h"
#include	"fontmisc.h"
#include	"fsio.h"

#include	<stdio.h>
#include	<signal.h>
#include	<sys/types.h>
#if !defined(WIN32) && !defined(AMOEBA) && !defined(_MINIX)
#ifndef Lynx
#include	<sys/socket.h>
#else
#include	<socket.h>
#endif
#endif
#include	<errno.h>
#ifdef X_NOT_STDC_ENV
extern int errno;
#endif 
#ifdef WIN32
#define EWOULDBLOCK WSAEWOULDBLOCK
#undef EINTR
#define EINTR WSAEINTR
#endif

#ifdef MINIX
#include <sys/nbio.h>
#define select(n,r,w,x,t) nbio_select(n,r,w,x,t)
#endif

#ifdef __EMX__
#define select(n,r,w,x,t) os2PseudoSelect(n,r,w,x,t)
#endif

/* check for both EAGAIN and EWOULDBLOCK, because some supposedly POSIX
 * systems are broken and return EWOULDBLOCK when they should return EAGAIN
 */
#ifdef WIN32
#define ETEST() (WSAGetLastError() == WSAEWOULDBLOCK)
#else
#if defined(EAGAIN) && defined(EWOULDBLOCK)
#define ETEST() (errno == EAGAIN || errno == EWOULDBLOCK)
#else
#ifdef EAGAIN
#define ETEST() (errno == EAGAIN)
#else
#define ETEST() (errno == EWOULDBLOCK)
#endif
#endif
#endif
#ifdef WIN32
#define ECHECK(err) (WSAGetLastError() == err)
#define ESET(val) WSASetLastError(val)
#else
#ifdef ISC
#define ECHECK(err) ((errno == err) || ETEST())
#else
#define ECHECK(err) (errno == err)
#endif
#define ESET(val) errno = val
#endif

static int  padlength[4] = {0, 3, 2, 1};
fd_set _fs_fd_mask;

int  _fs_wait_for_readable();

#ifdef SIGNALRETURNSINT
#define SIGNAL_T int
#else
#define SIGNAL_T void
#endif

/* ARGSUSED */
static      SIGNAL_T
_fs_alarm(foo)
    int         foo;
{
    return;
}

static XtransConnInfo
_fs_connect(servername, timeout)
    char       *servername;
    int         timeout;
{
    XtransConnInfo trans_conn;		/* transport connection object */
    int         ret = -1;
#ifdef SIGALRM
    unsigned    oldTime;

    SIGNAL_T(*oldAlarm) ();
#endif

    /*
     * Open the network connection.
     */
    if( (trans_conn=_FontTransOpenCOTSClient(servername)) == NULL )
	{
	return (NULL);
	}

#ifdef SIGALRM
    oldTime = alarm((unsigned) 0);
    oldAlarm = signal(SIGALRM, _fs_alarm);
    alarm((unsigned) timeout);
#endif

    ret = _FontTransConnect(trans_conn,servername);

#ifdef SIGALRM
    alarm((unsigned) 0);
    signal(SIGALRM, oldAlarm);
    alarm(oldTime);
#endif

    if (ret < 0)
	{
	_FontTransClose(trans_conn);
	return (NULL);
	}

    /*
     * Set the connection non-blocking since we use select() to block.
     */

    _FontTransSetOption(trans_conn, TRANS_NONBLOCKING, 1);

    return trans_conn;
}

static int  generationCount;

/* ARGSUSED */
static Bool
_fs_setup_connection(conn, servername, timeout, copy_name_p)
    FSFpePtr    conn;
    char       *servername;
    int         timeout;
    Bool	copy_name_p;
{
    fsConnClientPrefix prefix;
    fsConnSetup rep;
    int         setuplength;
    fsConnSetupAccept conn_accept;
    int         endian;
    int         i;
    int         alt_len;
    char       *auth_data = NULL,
               *vendor_string = NULL,
               *alt_data = NULL,
               *alt_dst;
    FSFpeAltPtr alts;
    int         nalts;

    if ((conn->trans_conn = _fs_connect(servername, 5)) == NULL)
	return FALSE;

    conn->fs_fd = _FontTransGetConnectionNumber (conn->trans_conn);

    conn->generation = ++generationCount;

    /* send setup prefix */
    endian = 1;
    if (*(char *) &endian)
	prefix.byteOrder = 'l';
    else
	prefix.byteOrder = 'B';

    prefix.major_version = FS_PROTOCOL;
    prefix.minor_version = FS_PROTOCOL_MINOR;

/* XXX add some auth info here */
    prefix.num_auths = 0;
    prefix.auth_len = 0;

    if (_fs_write(conn, (char *) &prefix, SIZEOF(fsConnClientPrefix)) == -1)
	return FALSE;

    /* read setup info */
    if (_fs_read(conn, (char *) &rep, SIZEOF(fsConnSetup)) == -1)
	return FALSE;

    conn->fsMajorVersion = rep.major_version;
    if (rep.major_version > FS_PROTOCOL)
	return FALSE;

    alts = 0;
    /* parse alternate list */
    if (nalts = rep.num_alternates) {
	setuplength = rep.alternate_len << 2;
	alts = (FSFpeAltPtr) xalloc(nalts * sizeof(FSFpeAltRec) +
				    setuplength);
	if (!alts) {
	    _FontTransClose(conn->trans_conn);
	    errno = ENOMEM;
	    return FALSE;
	}
	alt_data = (char *) (alts + nalts);
	if (_fs_read(conn, (char *) alt_data, setuplength) == -1) {
	    xfree(alts);
	    return FALSE;
	}
	alt_dst = alt_data;
	for (i = 0; i < nalts; i++) {
	    alts[i].subset = alt_data[0];
	    alt_len = alt_data[1];
	    alts[i].name = alt_dst;
	    memmove(alt_dst, alt_data + 2, alt_len);
	    alt_dst[alt_len] = '\0';
	    alt_dst += (alt_len + 1);
	    alt_data += (2 + alt_len + padlength[(2 + alt_len) & 3]);
	}
    }
    if (conn->alts)
	xfree(conn->alts);
    conn->alts = alts;
    conn->numAlts = nalts;

    setuplength = rep.auth_len << 2;
    if (setuplength &&
	    !(auth_data = (char *) xalloc((unsigned int) setuplength))) {
	_FontTransClose(conn->trans_conn);
	errno = ENOMEM;
	return FALSE;
    }
    if (_fs_read(conn, (char *) auth_data, setuplength) == -1) {
	xfree(auth_data);
	return FALSE;
    }
    if (rep.status != AuthSuccess) {
	xfree(auth_data);
	_FontTransClose(conn->trans_conn);
	errno = EPERM;
	return FALSE;
    }
    /* get rest */
    if (_fs_read(conn, (char *) &conn_accept, (long) SIZEOF(fsConnSetupAccept)) == -1) {
	xfree(auth_data);
	return FALSE;
    }
    if ((vendor_string = (char *)
	 xalloc((unsigned) conn_accept.vendor_len + 1)) == NULL) {
	xfree(auth_data);
	_FontTransClose(conn->trans_conn);
	errno = ENOMEM;
	return FALSE;
    }
    if (_fs_read_pad(conn, (char *) vendor_string, conn_accept.vendor_len) == -1) {
	xfree(vendor_string);
	xfree(auth_data);
	return FALSE;
    }
    xfree(auth_data);
    xfree(vendor_string);

    if (copy_name_p)
    {
        conn->servername = (char *) xalloc(strlen(servername) + 1);
        if (conn->servername == NULL)
	    return FALSE;
        strcpy(conn->servername, servername);
    }
    else
        conn->servername = servername;

    return TRUE;
}

static Bool
_fs_try_alternates(conn, timeout)
    FSFpePtr    conn;
    int         timeout;
{
    int         i;

    for (i = 0; i < conn->numAlts; i++)
	if (_fs_setup_connection(conn, conn->alts[i].name, timeout, TRUE))
	    return TRUE;
    return FALSE;
}

#define FS_OPEN_TIMEOUT	    30
#define FS_REOPEN_TIMEOUT   10

FSFpePtr
_fs_open_server(servername)
    char       *servername;
{
    FSFpePtr    conn;

    conn = (FSFpePtr) xalloc(sizeof(FSFpeRec));
    if (!conn) {
	errno = ENOMEM;
	return (FSFpePtr) NULL;
    }
    bzero((char *) conn, sizeof(FSFpeRec));
    if (!_fs_setup_connection(conn, servername, FS_OPEN_TIMEOUT, TRUE)) {
	if (!_fs_try_alternates(conn, FS_OPEN_TIMEOUT)) {
	    xfree(conn->alts);
	    xfree(conn);
	    return (FSFpePtr) NULL;
	}
    }
    return conn;
}

Bool
_fs_reopen_server(conn)
    FSFpePtr    conn;
{
    if (_fs_setup_connection(conn, conn->servername, FS_REOPEN_TIMEOUT, FALSE))
	return TRUE;
    if (_fs_try_alternates(conn, FS_REOPEN_TIMEOUT))
	return TRUE;
    return FALSE;
}

/*
 * expects everything to be here.  *not* to be called when reading huge
 * numbers of replies, but rather to get each chunk
 */
_fs_read(conn, data, size)
    FSFpePtr    conn;
    char       *data;
    unsigned long size;
{
    long        bytes_read;
#if defined(SVR4) && defined(i386)
    int		num_failed_reads = 0;
#endif

    if (size == 0) {

#ifdef DEBUG
	fprintf(stderr, "tried to read 0 bytes \n");
#endif

	return 0;
    }
    ESET(0);
    /*
     * For SVR4 with a unix-domain connection, ETEST() after selecting
     * readable means the server has died.  To do this here, we look for
     * two consecutive reads returning ETEST().
     */
    while ((bytes_read = _FontTransRead(conn->trans_conn,
	data, (int) size)) != size) {
	if (bytes_read > 0) {
	    size -= bytes_read;
	    data += bytes_read;
#if defined(SVR4) && defined(i386)
	    num_failed_reads = 0;
#endif
	} else if (ETEST()) {
	    /* in a perfect world, this shouldn't happen */
	    /* ... but then, its less than perfect... */
	    if (_fs_wait_for_readable(conn) == -1) {	/* check for error */
		_fs_connection_died(conn);
		ESET(EPIPE);
		return -1;
	    }
#if defined(SVR4) && defined(i386)
	    num_failed_reads++;
	    if (num_failed_reads > 1) {
		_fs_connection_died(conn);
		ESET(EPIPE);
		return -1;
	    }
#endif
	    ESET(0);
	} else if (ECHECK(EINTR)) {
#if defined(SVR4) && defined(i386)
	    num_failed_reads = 0;
#endif
	    continue;
	} else {		/* something bad happened */
	    if (conn->fs_fd > 0)
		_fs_connection_died(conn);
	    ESET(EPIPE);
	    return -1;
	}
    }
    return 0;
}

_fs_write(conn, data, size)
    FSFpePtr    conn;
    char       *data;
    unsigned long size;
{
    long        bytes_written;

    if (size == 0) {

#ifdef DEBUG
	fprintf(stderr, "tried to write 0 bytes \n");
#endif

	return 0;
    }

    /* XXX - hack.  The right fix is to remember that the font server
       has gone away when we first discovered it. */
    if (!conn->trans_conn)
	return -1;

    ESET(0);
    while ((bytes_written = _FontTransWrite(conn->trans_conn,
	data, (int) size)) != size) {
	if (bytes_written > 0) {
	    size -= bytes_written;
	    data += bytes_written;
	} else if (ETEST()) {
	    /* XXX -- we assume this can't happen */

#ifdef DEBUG
	    fprintf(stderr, "fs_write blocking\n");
#endif
	} else if (ECHECK(EINTR)) {
	    continue;
	} else {		/* something bad happened */
	    _fs_connection_died(conn);
	    ESET(EPIPE);
	    return -1;
	}
    }
    return 0;
}

_fs_read_pad(conn, data, len)
    FSFpePtr    conn;
    char       *data;
    int         len;
{
    char        pad[3];

    if (_fs_read(conn, data, len) == -1)
	return -1;

    /* read the junk */
    if (padlength[len & 3]) {
	return _fs_read(conn, pad, padlength[len & 3]);
    }
    return 0;
}

_fs_write_pad(conn, data, len)
    FSFpePtr    conn;
    char       *data;
    int         len;
{
    static char pad[3];

    if (_fs_write(conn, data, len) == -1)
	return -1;

    /* write the pad */
    if (padlength[len & 3]) {
	return _fs_write(conn, pad, padlength[len & 3]);
    }
    return 0;
}

/*
 * returns the amount of data waiting to be read
 */
int
_fs_data_ready(conn)
    FSFpePtr    conn;
{
    BytesReadable_t readable;

    if (_FontTransBytesReadable(conn->trans_conn, &readable) < 0)
	return -1;
    return readable;
}

int
_fs_wait_for_readable(conn)
    FSFpePtr    conn;
{
#ifndef AMOEBA
    fd_set r_mask;
    fd_set e_mask;
    int         result;

#ifdef DEBUG
    fprintf(stderr, "read would block\n");
#endif

    do {
	FD_ZERO(&r_mask);
#ifndef MINIX
	FD_ZERO(&e_mask);
#endif
	FD_SET(conn->fs_fd, &r_mask);
	FD_SET(conn->fs_fd, &e_mask);
	result = Select(conn->fs_fd + 1, &r_mask, NULL, &e_mask, NULL);
	if (result == -1) {
	    if (ECHECK(EINTR) || ECHECK(EAGAIN))
		continue;
	    else
		return -1;
	}
	if (result && FD_ISSET(conn->fs_fd, &e_mask))
	    return -1;
    } while (result <= 0);

    return 0;
#else
    printf("fs_wait_for_readable(): fail\n");
    return -1;
#endif
}

int
_fs_set_bit(mask, fd)
    fd_set* mask;
    int         fd;
{
    FD_SET(fd, mask);
    return fd;
}

int
_fs_is_bit_set(mask, fd)
    fd_set* mask;
    int         fd;
{
    return FD_ISSET(fd, mask);
}

void
_fs_bit_clear(mask, fd)
    fd_set* mask;
    int         fd;
{
    FD_CLR(fd, mask);
}

int
_fs_any_bit_set(mask)
    fd_set* mask;
{
    return XFD_ANYSET(mask);
}

int
_fs_or_bits(dst, m1, m2)
    fd_set* dst;
    fd_set* m1;
    fd_set* m2;
{
#ifdef WIN32
    int i;
    if (dst != m1) {
	for (i = m1->fd_count; --i >= 0; ) {
	    if (!FD_ISSET(m1->fd_array[i], dst))
		FD_SET(m1->fd_array[i], dst);
	}
    }
    if (dst != m2) {
	for (i = m2->fd_count; --i >= 0; ) {
	    if (!FD_ISSET(m2->fd_array[i], dst))
		FD_SET(m2->fd_array[i], dst);
	}
    }
#else
    XFD_ORSET(dst, m1, m2);
#endif

    return 0;
}

_fs_drain_bytes(conn, len)
    FSFpePtr    conn;
    int         len;
{
    char        buf[128];

#ifdef DEBUG
    fprintf(stderr, "draining wire\n");
#endif

    while (len > 0) {
	if (_fs_read(conn, buf, (len < 128) ? len : 128) < 0)
	    return -1;
	len -= 128;
    }
    return 0;
}

_fs_drain_bytes_pad(conn, len)
    FSFpePtr    conn;
    int         len;
{
    _fs_drain_bytes(conn, len);

    /* read the junk */
    if (padlength[len & 3]) {
	_fs_drain_bytes(conn, padlength[len & 3]);
    }
}

_fs_eat_rest_of_error(conn, err)
    FSFpePtr    conn;
    fsError    *err;
{
    int         len = (err->length - (SIZEOF(fsGenericReply) >> 2)) << 2;

#ifdef DEBUG
    fprintf(stderr, "clearing error\n");
#endif

    _fs_drain_bytes(conn, len);
}
