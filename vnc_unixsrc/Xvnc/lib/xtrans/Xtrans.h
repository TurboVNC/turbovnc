/* $XConsortium: Xtrans.h,v 1.29 95/06/08 23:20:39 gildea Exp $ */
/* $XFree86: xc/lib/xtrans/Xtrans.h,v 3.9 1997/01/18 06:52:39 dawes Exp $ */
/*

Copyright (c) 1993, 1994  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

/* Copyright (c) 1993, 1994 NCR Corporation - Dayton, Ohio, USA
 *
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name NCR not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  NCR makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NCR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NCR BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _XTRANS_H_
#define _XTRANS_H_

#include <X11/Xfuncproto.h>
#include <X11/Xos.h>


/*
 * Set the functions names according to where this code is being compiled.
 */

#ifdef X11_t
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _X11Trans##func
#else
#define TRANS(func) _X11Trans/**/func
#endif
static char* __xtransname = "_X11Trans";
#endif /* X11_t */

#ifdef XSERV_t
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _XSERVTrans##func
#else
#define TRANS(func) _XSERVTrans/**/func
#endif
static char* __xtransname = "_XSERVTrans";
#define X11_t
#endif /* X11_t */

#ifdef XIM_t
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _XimXTrans##func
#else
#define TRANS(func) _XimXTrans/**/func
#endif
static char* __xtransname = "_XimTrans";
#endif /* XIM_t */

#ifdef FS_t
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _FSTrans##func
#else
#define TRANS(func) _FSTrans/**/func
#endif
static char* __xtransname = "_FSTrans";
#endif /* FS_t */

#ifdef FONT_t
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _FontTrans##func
#else
#define TRANS(func) _FontTrans/**/func
#endif
static char* __xtransname = "_FontTrans";
#endif /* FONT_t */

#ifdef ICE_t
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _IceTrans##func
#else
#define TRANS(func) _IceTrans/**/func
#endif
static char* __xtransname = "_IceTrans";
#endif /* ICE_t */

#ifdef TEST_t
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _TESTTrans##func
#else
#define TRANS(func) _TESTTrans/**/func
#endif
static char* __xtransname = "_TESTTrans";
#endif /* TEST_t */

#if !defined(TRANS)
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _XTrans##func
#else
#define TRANS(func) _XTrans/**/func
#endif
static char* __xtransname = "_XTrans";
#endif /* !TRANS */


/*
 * Create a single address structure that can be used wherever
 * an address structure is needed. struct sockaddr is not big enough
 * to hold a sockadd_un, so we create this definition to have a single
 * structure that is big enough for all the structures we might need.
 *
 * This structure needs to be independent of the socket/TLI interface used.
 */

#define XTRANS_MAX_ADDR_LEN	128	/* large enough to hold sun_path */

typedef	struct {
    unsigned char	addr[XTRANS_MAX_ADDR_LEN];
} Xtransaddr;


#ifdef LONG64
typedef int BytesReadable_t;
#else
typedef long BytesReadable_t;
#endif


#if defined(WIN32) || (defined(USG) && !defined(CRAY) && !defined(umips) && !defined(MOTOROLA) && !defined(uniosu) && !defined(__sxg__)) || defined(MINIX)

/*
 *      TRANS(Readv) and TRANS(Writev) use struct iovec, normally found
 *      in Berkeley systems in <sys/uio.h>.  See the readv(2) and writev(2)
 *      manual pages for details.
 */

struct iovec {
    caddr_t iov_base;
    int iov_len;
};

#else
#ifndef Lynx
#include <sys/uio.h>
#else
#include <uio.h>
#endif
#endif

typedef struct _XtransConnInfo *XtransConnInfo;


/*
 * Transport Option definitions
 */

#define TRANS_NONBLOCKING	1
#define	TRANS_CLOSEONEXEC	2


/*
 * Return values of Connect (0 is success)
 */

#define TRANS_CONNECT_FAILED 	-1
#define TRANS_TRY_CONNECT_AGAIN -2


/*
 * Return values of CreateListener (0 is success)
 */

#define TRANS_CREATE_LISTENER_FAILED 	-1
#define TRANS_ADDR_IN_USE		-2


/*
 * Return values of Accept (0 is success)
 */

#define TRANS_ACCEPT_BAD_MALLOC			-1
#define TRANS_ACCEPT_FAILED 			-2
#define TRANS_ACCEPT_MISC_ERROR			-3


/*
 * ResetListener return values
 */

#define TRANS_RESET_NOOP	1
#define TRANS_RESET_NEW_FD	2
#define TRANS_RESET_FAILURE	3


/*
 * Function prototypes for the exposed interface
 */

#ifdef TRANS_CLIENT

XtransConnInfo TRANS(OpenCOTSClient)(
#if NeedFunctionPrototypes
    char *		/* address */
#endif
);

#endif /* TRANS_CLIENT */

#ifdef TRANS_SERVER

XtransConnInfo TRANS(OpenCOTSServer)(
#if NeedFunctionPrototypes
    char *		/* address */
#endif
);

#endif /* TRANS_SERVER */

#ifdef TRANS_CLIENT

XtransConnInfo TRANS(OpenCLTSClient)(
#if NeedFunctionPrototypes
    char *		/* address */
#endif
);

#endif /* TRANS_CLIENT */

#ifdef TRANS_SERVER

XtransConnInfo TRANS(OpenCLTSServer)(
#if NeedFunctionPrototypes
    char *		/* address */
#endif
);

#endif /* TRANS_SERVER */

#ifdef TRANS_REOPEN

XtransConnInfo TRANS(ReopenCOTSServer)(
#if NeedFunctionPrototypes
    int,		/* trans_id */
    int,		/* fd */
    char *		/* port */
#endif
);

XtransConnInfo TRANS(ReopenCLTSServer)(
#if NeedFunctionPrototypes
    int,		/* trans_id */
    int,		/* fd */
    char *		/* port */
#endif
);

int TRANS(GetReopenInfo)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    int *,		/* trans_id */
    int *,		/* fd */
    char **		/* port */
#endif
);

#endif /* TRANS_REOPEN */


int TRANS(SetOption)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    int,		/* option */
    int			/* arg */
#endif
);

#ifdef TRANS_SERVER

int TRANS(CreateListener)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    char *		/* port */
#endif
);

int TRANS(NoListen) (
#if NeedFunctionPrototypes
    char*               /* protocol*/
#endif
);

int TRANS(ResetListener)(
#if NeedFunctionPrototypes
    XtransConnInfo	/* ciptr */
#endif
);

XtransConnInfo TRANS(Accept)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    int *		/* status */
#endif
);

#endif /* TRANS_SERVER */

#ifdef TRANS_CLIENT

int TRANS(Connect)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    char *		/* address */
#endif
);

#endif /* TRANS_CLIENT */

int TRANS(BytesReadable)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    BytesReadable_t *	/* pend */
#endif
);

int TRANS(Read)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    char *,		/* buf */
    int			/* size */
#endif
);

int TRANS(Write)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    char *,		/* buf */
    int			/* size */
#endif
);

int TRANS(Readv)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    struct iovec *,	/* buf */
    int			/* size */
#endif
);

int TRANS(Writev)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    struct iovec *,	/* buf */
    int			/* size */
#endif
);

int TRANS(Disconnect)(
#if NeedFunctionPrototypes
    XtransConnInfo	/* ciptr */
#endif
);

int TRANS(Close)(
#if NeedFunctionPrototypes
    XtransConnInfo	/* ciptr */
#endif
);

int TRANS(CloseForCloning)(
#if NeedFunctionPrototypes
    XtransConnInfo	/* ciptr */
#endif
);

int TRANS(IsLocal)(
#if NeedFunctionPrototypes
    XtransConnInfo	/* ciptr */
#endif
);

int TRANS(GetMyAddr)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    int *,		/* familyp */
    int *,		/* addrlenp */
    Xtransaddr **	/* addrp */
#endif
);

int TRANS(GetPeerAddr)(
#if NeedFunctionPrototypes
    XtransConnInfo,	/* ciptr */
    int *,		/* familyp */
    int *,		/* addrlenp */
    Xtransaddr **	/* addrp */
#endif
);

int TRANS(GetConnectionNumber)(
#if NeedFunctionPrototypes
    XtransConnInfo	/* ciptr */
#endif
);

#ifdef TRANS_SERVER

int TRANS(MakeAllCOTSServerListeners)(
#if NeedFunctionPrototypes
    char *,		/* port */
    int *,		/* partial */
    int *,		/* count_ret */
    XtransConnInfo **	/* ciptrs_ret */
#endif
);

int TRANS(MakeAllCLTSServerListeners)(
#if NeedFunctionPrototypes
    char *,		/* port */
    int *,		/* partial */
    int *,		/* count_ret */
    XtransConnInfo **	/* ciptrs_ret */
#endif
);

#endif /* TRANS_SERVER */


/*
 * Function Prototypes for Utility Functions.
 */

#ifdef X11_t

int TRANS(ConvertAddress)(
#if NeedFunctionPrototypes
    int *,		/* familyp */
    int *,		/* addrlenp */
    Xtransaddr **	/* addrp */
#endif
);

#endif /* X11_t */

#ifdef ICE_t

char *
TRANS(GetMyNetworkId)(
#if NeedFunctionPrototypes
    XtransConnInfo	/* ciptr */
#endif
);

char *
TRANS(GetPeerNetworkId)(
#if NeedFunctionPrototypes
    XtransConnInfo	/* ciptr */
#endif
);

#endif /* ICE_t */

#if defined(WIN32) && (defined(TCPCONN) || defined(DNETCONN))
int TRANS(WSAStartup)();
#endif

#endif /* _XTRANS_H_ */
