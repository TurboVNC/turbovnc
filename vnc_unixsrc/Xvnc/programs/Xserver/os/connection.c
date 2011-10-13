/* $Xorg: connection.c,v 1.6 2001/02/09 02:05:23 xorgcvs Exp $ */
/***********************************************************

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

******************************************************************/
/* $XFree86: xc/programs/Xserver/os/connection.c,v 3.64 2003/10/07 22:50:42 herrb Exp $ */
/*****************************************************************
 *  Stuff to create connections --- OS dependent
 *
 *      EstablishNewConnections, CreateWellKnownSockets, ResetWellKnownSockets,
 *      CloseDownConnection, CheckConnections, AddEnabledDevice,
 *	RemoveEnabledDevice, OnlyListToOneClient,
 *      ListenToAllClients,
 *
 *      (WaitForSomething is in its own file)
 *
 *      In this implementation, a client socket table is not kept.
 *      Instead, what would be the index into the table is just the
 *      file descriptor of the socket.  This won't work for if the
 *      socket ids aren't small nums (0 - 2^8)
 *
 *****************************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifdef WIN32
#include <X11/Xwinsock.h>
#endif
#include <X11/X.h>
#include <X11/Xproto.h>
#define XSERV_t
#define TRANS_SERVER
#define TRANS_REOPEN
#include <X11/Xtrans/Xtrans.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32
#if defined(Lynx)
#include <socket.h>
#else
#include <sys/socket.h>
#endif

#ifdef hpux
#include <sys/utsname.h>
#include <sys/ioctl.h>
#endif

#if defined(DGUX)
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/param.h>
#include <unistd.h>
#endif


#ifdef AIXV3
#include <sys/ioctl.h>
#endif

#ifdef __UNIXOS2__
#define select(n,r,w,x,t) os2PseudoSelect(n,r,w,x,t)
extern __const__ int _nfiles;
#endif

#if defined(TCPCONN) || defined(STREAMSCONN)
# include <netinet/in.h>
# include <arpa/inet.h>
# if !defined(hpux)
#  ifdef apollo
#   ifndef NO_TCP_H
#    include <netinet/tcp.h>
#   endif
#  else
#   ifdef CSRG_BASED
#    include <sys/param.h>
#   endif
#    ifndef __UNIXOS2__
#     include <netinet/tcp.h>
#    endif
#  endif
# endif
# include <arpa/inet.h>
#endif

#if !defined(__UNIXOS2__)
#ifndef Lynx
#include <sys/uio.h>
#else
#include <uio.h>
#endif
#endif
#endif /* WIN32 */
#include "misc.h"		/* for typedef of pointer */
#include "osdep.h"
#include <X11/Xpoll.h>
#include "opaque.h"
#include "dixstruct.h"
#ifdef XAPPGROUP
#include <X11/extensions/Xagsrv.h>
#endif
#ifdef XCSECURITY
#define _SECURITY_SERVER
#include <X11/extensions/security.h>
#endif
#ifdef LBX
#include "colormapst.h"
#include "propertyst.h"
#include "lbxserve.h"
#include "osdep.h"
#endif

#ifdef X_NOT_POSIX
#define Pid_t int
#else
#define Pid_t pid_t
#endif

#ifdef DNETCONN
#include <netdnet/dn.h>
#endif /* DNETCONN */

int lastfdesc;			/* maximum file descriptor */

fd_set WellKnownConnections;	/* Listener mask */
fd_set EnabledDevices;		/* mask for input devices that are on */
fd_set AllSockets;		/* select on this */
fd_set AllClients;		/* available clients */
fd_set LastSelectMask;		/* mask returned from last select call */
fd_set ClientsWithInput;	/* clients with FULL requests in buffer */
fd_set ClientsWriteBlocked;	/* clients who cannot receive output */
fd_set OutputPending;		/* clients with reply/event data ready to go */
int MaxClients = 0;
Bool NewOutputPending;		/* not yet attempted to write some new output */
Bool AnyClientsWriteBlocked;	/* true if some client blocked on write */

Bool RunFromSmartParent;	/* send SIGUSR1 to parent process */
Bool PartialNetwork;		/* continue even if unable to bind all addrs */
static Pid_t ParentProcess;
#ifdef __UNIXOS2__
Pid_t GetPPID(Pid_t pid);
#endif

static Bool debug_conns = FALSE;

fd_set IgnoredClientsWithInput;
static fd_set GrabImperviousClients;
static fd_set SavedAllClients;
static fd_set SavedAllSockets;
static fd_set SavedClientsWithInput;
int GrabInProgress = 0;

#if !defined(WIN32)
int *ConnectionTranslation = NULL;
#else
/*
 * On NT fds are not between 0 and MAXSOCKS, they are unrelated, and there is
 * not even a known maximum value, so use something quite arbitrary for now.
 * Do storage is a hash table of size 256. Collisions are handled in a linked
 * list.
 */

#undef MAXSOCKS
#define MAXSOCKS 500
#undef MAXSELECT
#define MAXSELECT 500
#define MAXFD 500

struct _ct_node {
    struct _ct_node *next;
    int key;
    int value;
};

struct _ct_node *ct_head[256];

void InitConnectionTranslation(void)
{
    bzero(ct_head, sizeof(ct_head));
}

int GetConnectionTranslation(int conn)
{
    struct _ct_node *node = ct_head[conn & 0xff];
    while (node != NULL)
    {
        if (node->key == conn)
            return node->value;
        node = node->next;
    }
    return 0;
}

void SetConnectionTranslation(int conn, int client)
{
    struct _ct_node **node = ct_head + (conn & 0xff);
    if (client == 0) /* remove entry */
    {
        while (*node != NULL)
        {
            if ((*node)->key == conn)
            {
                struct _ct_node *temp = *node;
                *node = (*node)->next;
                free(temp);
                return;
            }
            node = &((*node)->next);
        }
        return;
    } else 
    {
        while (*node != NULL)
        {
            if ((*node)->key == conn)
            {
                (*node)->value = client;
                return;
            }
            node = &((*node)->next);
        }
        *node = (struct _ct_node*)xalloc(sizeof(struct _ct_node));
        (*node)->next = NULL;
        (*node)->key = conn;
        (*node)->value = client;
        return;
    }
}

void ClearConnectionTranslation(void)
{
    unsigned i;
    for (i = 0; i < 256; i++)
    {
        struct _ct_node *node = ct_head[i];
        while (node != NULL)
        {
            struct _ct_node *temp = node;
            node = node->next;
            xfree(temp);
        }
    }
}
#endif

XtransConnInfo 	*ListenTransConns = NULL;
int	       	*ListenTransFds = NULL;
int		ListenTransCount;

static void ErrorConnMax(XtransConnInfo /* trans_conn */);

#ifndef LBX
static
void CloseDownFileDescriptor(
    OsCommPtr /*oc*/
);
#endif


static XtransConnInfo
lookup_trans_conn (int fd)
{
    if (ListenTransFds)
    {
	int i;
	for (i = 0; i < ListenTransCount; i++)
	    if (ListenTransFds[i] == fd)
		return ListenTransConns[i];
    }

    return (NULL);
}

/* Set MaxClients and lastfdesc, and allocate ConnectionTranslation */

void
InitConnectionLimits(void)
{
    lastfdesc = -1;

#ifndef __CYGWIN__

#ifndef __UNIXOS2__

#if !defined(XNO_SYSCONF) && defined(_SC_OPEN_MAX)
    lastfdesc = sysconf(_SC_OPEN_MAX) - 1;
#endif

#ifdef HAS_GETDTABLESIZE
    if (lastfdesc < 0)
	lastfdesc = getdtablesize() - 1;
#endif

#ifdef _NFILE
    if (lastfdesc < 0)
	lastfdesc = _NFILE - 1;
#endif

#else /* __UNIXOS2__ */
    lastfdesc = _nfiles - 1;
#endif

#endif /* __CYGWIN__ */

    /* This is the fallback */
    if (lastfdesc < 0)
	lastfdesc = MAXSOCKS;

    if (lastfdesc > MAXSELECT)
	lastfdesc = MAXSELECT;

    if (lastfdesc > MAXCLIENTS)
    {
	lastfdesc = MAXCLIENTS;
	if (debug_conns)
	    ErrorF( "REACHED MAXIMUM CLIENTS LIMIT %d\n", MAXCLIENTS);
    }
    MaxClients = lastfdesc;

#ifdef DEBUG
    ErrorF("InitConnectionLimits: MaxClients = %d\n", MaxClients);
#endif

#if !defined(WIN32)
    ConnectionTranslation = (int *)xnfalloc(sizeof(int)*(lastfdesc + 1));
#else
    InitConnectionTranslation();
#endif
}


/*****************
 * CreateWellKnownSockets
 *    At initialization, create the sockets to listen on for new clients.
 *****************/

void
CreateWellKnownSockets(void)
{
    int		i;
    int		partial;
    char 	port[20];
    OsSigHandlerPtr handler;

    FD_ZERO(&AllSockets);
    FD_ZERO(&AllClients);
    FD_ZERO(&LastSelectMask);
    FD_ZERO(&ClientsWithInput);

#if !defined(WIN32)
    for (i=0; i<MaxClients; i++) ConnectionTranslation[i] = 0;
#else
    ClearConnectionTranslation();
#endif

    FD_ZERO (&WellKnownConnections);

    sprintf (port, "%d", atoi (display));

    if ((_XSERVTransMakeAllCOTSServerListeners (port, &partial,
	&ListenTransCount, &ListenTransConns) >= 0) &&
	(ListenTransCount >= 1))
    {
	if (!PartialNetwork && partial)
	{
	    FatalError ("Failed to establish all listening sockets");
	}
	else
	{
	    ListenTransFds = (int *) xalloc (ListenTransCount * sizeof (int));

	    for (i = 0; i < ListenTransCount; i++)
	    {
		int fd = _XSERVTransGetConnectionNumber (ListenTransConns[i]);
		
		ListenTransFds[i] = fd;
		FD_SET (fd, &WellKnownConnections);

		if (!_XSERVTransIsLocal (ListenTransConns[i]))
		{
		    DefineSelf (fd);
		}
	    }
	}
    }

    if (!XFD_ANYSET (&WellKnownConnections))
        FatalError ("Cannot establish any listening sockets - Make sure an X server isn't already running");
#if !defined(WIN32)
    OsSignal (SIGPIPE, SIG_IGN);
    OsSignal (SIGHUP, AutoResetServer);
#endif
    OsSignal (SIGINT, GiveUp);
    OsSignal (SIGTERM, GiveUp);
    XFD_COPYSET (&WellKnownConnections, &AllSockets);
    ResetHosts(display);
    /*
     * Magic:  If SIGUSR1 was set to SIG_IGN when
     * the server started, assume that either
     *
     *  a- The parent process is ignoring SIGUSR1
     *
     * or
     *
     *  b- The parent process is expecting a SIGUSR1
     *     when the server is ready to accept connections
     *
     * In the first case, the signal will be harmless,
     * in the second case, the signal will be quite
     * useful
     */
#if !defined(WIN32)
    handler = OsSignal (SIGUSR1, SIG_IGN);
    if ( handler == SIG_IGN)
	RunFromSmartParent = TRUE;
    OsSignal(SIGUSR1, handler);
    ParentProcess = getppid ();
#ifdef __UNIXOS2__
    /*
     * fg030505: under OS/2, xinit is not the parent process but
     * the "grant parent" process of the server because execvpe()
     * presents us an additional process number;
     * GetPPID(pid) is part of libemxfix
     */
    ParentProcess = GetPPID (ParentProcess);
#endif /* __UNIXOS2__ */
    if (RunFromSmartParent) {
	if (ParentProcess > 1) {
	    kill (ParentProcess, SIGUSR1);
	}
    }
#endif
#ifdef XDMCP
    XdmcpInit ();
#endif
}

void
ResetWellKnownSockets (void)
{
    int i;

    ResetOsBuffers();

    for (i = 0; i < ListenTransCount; i++)
    {
	int status = _XSERVTransResetListener (ListenTransConns[i]);

	if (status != TRANS_RESET_NOOP)
	{
	    if (status == TRANS_RESET_FAILURE)
	    {
		/*
		 * ListenTransConns[i] freed by xtrans.
		 * Remove it from out list.
		 */

		FD_CLR (ListenTransFds[i], &WellKnownConnections);
		ListenTransFds[i] = ListenTransFds[ListenTransCount - 1];
		ListenTransConns[i] = ListenTransConns[ListenTransCount - 1];
		ListenTransCount -= 1;
		i -= 1;
	    }
	    else if (status == TRANS_RESET_NEW_FD)
	    {
		/*
		 * A new file descriptor was allocated (the old one was closed)
		 */

		int newfd = _XSERVTransGetConnectionNumber (ListenTransConns[i]);

		FD_CLR (ListenTransFds[i], &WellKnownConnections);
		ListenTransFds[i] = newfd;
		FD_SET(newfd, &WellKnownConnections);
	    }
	}
    }

    ResetAuthorization ();
    ResetHosts(display);
    /*
     * See above in CreateWellKnownSockets about SIGUSR1
     */
#if !defined(WIN32)
    if (RunFromSmartParent) {
	if (ParentProcess > 1) {
	    kill (ParentProcess, SIGUSR1);
	}
    }
#endif
    /*
     * restart XDMCP
     */
#ifdef XDMCP
    XdmcpReset ();
#endif
}

void
CloseWellKnownConnections(void)
{
    int i;

    for (i = 0; i < ListenTransCount; i++)
	_XSERVTransClose (ListenTransConns[i]);
}

static void
AuthAudit (ClientPtr client, Bool letin, 
    struct sockaddr *saddr, int len, 
    unsigned int proto_n, char *auth_proto, int auth_id)
{
    char addr[128];
    char *out = addr;

    if (!((OsCommPtr)client->osPrivate)->trans_conn) {
	strcpy(addr, "LBX proxy at ");
	out += strlen(addr);
    }
    if (!len)
        strcpy(out, "local host");
    else
	switch (saddr->sa_family)
	{
	case AF_UNSPEC:
#if defined(UNIXCONN) || defined(LOCALCONN) || defined(OS2PIPECONN)
	case AF_UNIX:
#endif
	    strcpy(out, "local host");
	    break;
#if defined(TCPCONN) || defined(STREAMSCONN) || defined(MNX_TCPCONN)
	case AF_INET:
	    sprintf(out, "IP %s",
		inet_ntoa(((struct sockaddr_in *) saddr)->sin_addr));
	    break;
#if defined(IPv6) && defined(AF_INET6)
	case AF_INET6: {
	    char ipaddr[INET6_ADDRSTRLEN];
	    inet_ntop(AF_INET6, &((struct sockaddr_in6 *) saddr)->sin6_addr,
	      ipaddr, sizeof(ipaddr));
	    sprintf(out, "IP %s", ipaddr);
	}
	    break;
#endif
#endif
#ifdef DNETCONN
	case AF_DECnet:
	    sprintf(out, "DN %s",
		    dnet_ntoa(&((struct sockaddr_dn *) saddr)->sdn_add));
	    break;
#endif
	default:
	    strcpy(out, "unknown address");
	}
    
    if (proto_n)
	AuditF("client %d %s from %s\n  Auth name: %.*s ID: %d\n", 
	       client->index, letin ? "connected" : "rejected", addr,
	       (int)proto_n, auth_proto, auth_id);
    else 
	AuditF("client %d %s from %s\n", 
	       client->index, letin ? "connected" : "rejected", addr);
}

XID
AuthorizationIDOfClient(ClientPtr client)
{
    if (client->osPrivate)
	return ((OsCommPtr)client->osPrivate)->auth_id;
    else
	return None;
}


/*****************************************************************
 * ClientAuthorized
 *
 *    Sent by the client at connection setup:
 *                typedef struct _xConnClientPrefix {
 *                   CARD8	byteOrder;
 *                   BYTE	pad;
 *                   CARD16	majorVersion, minorVersion;
 *                   CARD16	nbytesAuthProto;    
 *                   CARD16	nbytesAuthString;   
 *                 } xConnClientPrefix;
 *
 *     	It is hoped that eventually one protocol will be agreed upon.  In the
 *        mean time, a server that implements a different protocol than the
 *        client expects, or a server that only implements the host-based
 *        mechanism, will simply ignore this information.
 *
 *****************************************************************/

char * 
ClientAuthorized(ClientPtr client, 
    unsigned int proto_n, char *auth_proto, 
    unsigned int string_n, char *auth_string)
{
    OsCommPtr 		priv;
    Xtransaddr		*from = NULL;
    int 		family;
    int			fromlen;
    XID	 		auth_id;
    char	 	*reason = NULL;
    XtransConnInfo	trans_conn;
    int			restore_trans_conn = 0;
    ClientPtr           lbxpc = NULL;

    priv = (OsCommPtr)client->osPrivate;
    trans_conn = priv->trans_conn;

#ifdef LBX
    if (!trans_conn) {
	/*
	 * Since trans_conn is NULL, this must be a proxy's client for
	 * which we have NO address.  Therefore, we will temporarily
	 * set the client's trans_conn to the proxy's trans_conn and
	 * after CheckAuthorization the client's trans_conn will be
	 * restored. 
	 *
	 * If XDM-AUTHORIZATION-1 is being used, CheckAuthorization
	 * will eventually call XdmAuthorizationValidate and this
	 * later function may use the client's trans_conn to get the 
	 * client's address.  Since a XDM-AUTH-1 auth string includes 
	 * the client's address, this address is compared to the address 
	 * in the client's trans_conn.  If the proxy and client are 
	 * on the same host, the comparison will fail; otherwise the
	 * comparison will fail and the client will not be authorized
	 * to connect to the server.
	 *
	 * The basis for this additional code is to prevent a
	 * NULL pointer dereference of the client's trans_conn.
	 * The fundamental problem - the fact that the client's
	 * trans_conn is NULL - is because the NewClient
	 * request in version 1.0 of the LBX protocol does not
	 * send the client's address to the server.  When the
	 * spec is changed and the client's address is sent to
	 * server in the NewClient request, this additional code
	 * should be removed.
	 *
	 * See defect number XWSog08218 for more information.
	 */
	lbxpc = LbxProxyClient(priv->proxy);
	trans_conn = ((OsCommPtr)lbxpc->osPrivate)->trans_conn;
        priv->trans_conn = trans_conn;
	restore_trans_conn = 1;
    }
#endif

    auth_id = CheckAuthorization (proto_n, auth_proto,
				  string_n, auth_string, client, &reason);

#ifdef LBX
    if (! priv->trans_conn) {
	if (auth_id == (XID) ~0L && !GetAccessControl())
	    auth_id = ((OsCommPtr)lbxpc->osPrivate)->auth_id;
#ifdef XCSECURITY
	else if (auth_id != (XID) ~0L && !SecuritySameLevel(lbxpc, auth_id)) {
	    auth_id = (XID) ~0L;
	    reason = "Client trust level differs from that of LBX Proxy";
	}
#endif
    }
#endif
    if (auth_id == (XID) ~0L)
    {
	if (
#ifdef XCSECURITY	    
	    (proto_n == 0 ||
	    strncmp (auth_proto, XSecurityAuthorizationName, proto_n) != 0) &&
#endif
	    _XSERVTransGetPeerAddr (trans_conn,
	        &family, &fromlen, &from) != -1)
	{
	    if (
#ifdef LBX
		!trans_conn ||
#endif
		InvalidHost ((struct sockaddr *) from, fromlen, client))
		AuthAudit(client, FALSE, (struct sockaddr *) from,
			  fromlen, proto_n, auth_proto, auth_id);
	    else
	    {
		auth_id = (XID) 0;
		if (auditTrailLevel > 1)
		    AuthAudit(client, TRUE,
			(struct sockaddr *) from, fromlen,
			proto_n, auth_proto, auth_id);
	    }

	    xfree ((char *) from);
	}

	if (auth_id == (XID) ~0L) {
#ifdef LBX
	  /*
	   * Restore client's trans_conn state
	   */
	  if (restore_trans_conn) {
		priv->trans_conn = NULL;
	  }
#endif
	    if (reason)
		return reason;
	    else
		return "Client is not authorized to connect to Server";
	}
    }
    else if (auditTrailLevel > 1)
    {
	if (_XSERVTransGetPeerAddr (trans_conn,
	    &family, &fromlen, &from) != -1)
	{
	    AuthAudit(client, TRUE, (struct sockaddr *) from, fromlen,
		      proto_n, auth_proto, auth_id);

	    xfree ((char *) from);
	}
    }
    priv->auth_id = auth_id;
    priv->conn_time = 0;

#ifdef XDMCP
    /* indicate to Xdmcp protocol that we've opened new client */
    XdmcpOpenDisplay(priv->fd);
#endif /* XDMCP */
#ifdef XAPPGROUP
    if (ClientStateCallback)
        XagCallClientStateChange (client);
#endif
    /* At this point, if the client is authorized to change the access control
     * list, we should getpeername() information, and add the client to
     * the selfhosts list.  It's not really the host machine, but the
     * true purpose of the selfhosts list is to see who may change the
     * access control list.
     */
#ifdef LBX
     if (restore_trans_conn) {
	priv->trans_conn = NULL;
     }
#endif
    return((char *)NULL);
}

static ClientPtr
#ifdef LBX
AllocNewConnection (XtransConnInfo trans_conn, int fd, CARD32 conn_time, 
    int (*Flush)(
        ClientPtr /*who*/, OsCommPtr /*oc*/,
        char * /*extraBuf*/, int /*extraCount*/),
    void (*Close)(
        ClientPtr /*client*/),
    LbxProxyPtr proxy)
#else
AllocNewConnection (XtransConnInfo trans_conn, int fd, CARD32 conn_time)
#endif
{
    OsCommPtr	oc;
    ClientPtr	client;
    
    if (
#ifdef LBX
	trans_conn &&
#endif
#ifndef WIN32
	fd >= lastfdesc
#else
	XFD_SETCOUNT(&AllClients) >= MaxClients
#endif
	)
	return NullClient;
    oc = (OsCommPtr)xalloc(sizeof(OsCommRec));
    if (!oc)
	return NullClient;
    oc->trans_conn = trans_conn;
    oc->fd = fd;
    oc->input = (ConnectionInputPtr)NULL;
    oc->output = (ConnectionOutputPtr)NULL;
    oc->auth_id = None;
    oc->conn_time = conn_time;
#ifdef LBX
    oc->proxy = proxy;
    oc->Flush = Flush;
    oc->Close = Close;
    oc->largereq = (ConnectionInputPtr) NULL;
#endif
    if (!(client = NextAvailableClient((pointer)oc)))
    {
	xfree (oc);
	return NullClient;
    }
#ifdef LBX
    if (trans_conn)
#endif
    {
#if !defined(WIN32)
	ConnectionTranslation[fd] = client->index;
#else
	SetConnectionTranslation(fd, client->index);
#endif
	if (GrabInProgress)
	{
	    FD_SET(fd, &SavedAllClients);
	    FD_SET(fd, &SavedAllSockets);
	}
	else
	{
	    FD_SET(fd, &AllClients);
	    FD_SET(fd, &AllSockets);
	}
    }

#ifdef DEBUG
    ErrorF("AllocNewConnection: client index = %d, socket fd = %d\n",
	   client->index, fd);
#endif

    return client;
}

#ifdef LBX

int
ClientConnectionNumber (ClientPtr client)
{
    OsCommPtr oc = (OsCommPtr) client->osPrivate;

    return oc->fd;
}

ClientPtr
AllocLbxClientConnection (ClientPtr client, LbxProxyPtr proxy)
{
    OsCommPtr oc = (OsCommPtr) client->osPrivate;

    return AllocNewConnection ((XtransConnInfo)NULL, oc->fd, GetTimeInMillis(),
			       LbxFlushClient, LbxCloseClient, proxy);
}

void
LbxProxyConnection (ClientPtr client, LbxProxyPtr proxy)
{
    OsCommPtr	oc = (OsCommPtr) client->osPrivate;

    FlushClient(client, oc, (char *)NULL, 0);
    oc->proxy = proxy;
    oc->Flush = LbxFlushClient;
    oc->Close = LbxCloseClient;
    LbxPrimeInput(client, proxy);
}

#endif

/*****************
 * EstablishNewConnections
 *    If anyone is waiting on listened sockets, accept them.
 *    Returns a mask with indices of new clients.  Updates AllClients
 *    and AllSockets.
 *****************/

/*ARGSUSED*/
Bool
EstablishNewConnections(ClientPtr clientUnused, pointer closure)
{
    fd_set  readyconnections;     /* set of listeners that are ready */
    int curconn;                  /* fd of listener that's ready */
    register int newconn;         /* fd of new client */
    CARD32 connect_time;
    register int i;
    register ClientPtr client;
    register OsCommPtr oc;
    fd_set tmask;

    XFD_ANDSET (&tmask, (fd_set*)closure, &WellKnownConnections);
    XFD_COPYSET(&tmask, &readyconnections);
    if (!XFD_ANYSET(&readyconnections))
	return TRUE;
    connect_time = GetTimeInMillis();
    /* kill off stragglers */
    for (i=1; i<currentMaxClients; i++)
    {
	if ((client = clients[i]))
	{
	    oc = (OsCommPtr)(client->osPrivate);
	    if ((oc && (oc->conn_time != 0) &&
		(connect_time - oc->conn_time) >= TimeOutValue) || 
		(client->noClientException != Success && !client->clientGone))
		CloseDownClient(client);     
	}
    }
#ifndef WIN32
    for (i = 0; i < howmany(XFD_SETSIZE, NFDBITS); i++)
    {
      while (readyconnections.fds_bits[i])
#else
      for (i = 0; i < XFD_SETCOUNT(&readyconnections); i++) 
#endif
      {
	XtransConnInfo trans_conn, new_trans_conn;
	int status;

#ifndef WIN32
	curconn = ffs (readyconnections.fds_bits[i]) - 1;
	readyconnections.fds_bits[i] &= ~((fd_mask)1 << curconn);
	curconn += (i * (sizeof(fd_mask)*8));
#else
	curconn = XFD_FD(&readyconnections, i);
#endif

	if ((trans_conn = lookup_trans_conn (curconn)) == NULL)
	    continue;

	if ((new_trans_conn = _XSERVTransAccept (trans_conn, &status)) == NULL)
	    continue;

	newconn = _XSERVTransGetConnectionNumber (new_trans_conn);

	if (newconn < lastfdesc)
	{
		int clientid;
#if !defined(WIN32)
  		clientid = ConnectionTranslation[newconn];
#else
  		clientid = GetConnectionTranslation(newconn);
#endif
		if(clientid && (client = clients[clientid]))
 			CloseDownClient(client);
	}

	_XSERVTransSetOption(new_trans_conn, TRANS_NONBLOCKING, 1);

	if (!AllocNewConnection (new_trans_conn, newconn, connect_time
#ifdef LBX
				 , StandardFlushClient,
				 CloseDownFileDescriptor, (LbxProxyPtr)NULL
#endif
				))
	{
	    ErrorConnMax(new_trans_conn);
	    _XSERVTransClose(new_trans_conn);
	}
      }
#ifndef WIN32
    }
#endif
    return TRUE;
}

#define NOROOM "Maximum number of clients reached"

/************
 *   ErrorConnMax
 *     Fail a connection due to lack of client or file descriptor space
 ************/

static void
ErrorConnMax(XtransConnInfo trans_conn)
{
    int fd = _XSERVTransGetConnectionNumber (trans_conn);
    xConnSetupPrefix csp;
    char pad[3];
    struct iovec iov[3];
    char byteOrder = 0;
    int whichbyte = 1;
    struct timeval waittime;
    fd_set mask;

    /* if these seems like a lot of trouble to go to, it probably is */
    waittime.tv_sec = BOTIMEOUT / MILLI_PER_SECOND;
    waittime.tv_usec = (BOTIMEOUT % MILLI_PER_SECOND) *
		       (1000000 / MILLI_PER_SECOND);
    FD_ZERO(&mask);
    FD_SET(fd, &mask);
    (void)Select(fd + 1, &mask, NULL, NULL, &waittime);
    /* try to read the byte-order of the connection */
    (void)_XSERVTransRead(trans_conn, &byteOrder, 1);
    if ((byteOrder == 'l') || (byteOrder == 'B'))
    {
	csp.success = xFalse;
	csp.lengthReason = sizeof(NOROOM) - 1;
	csp.length = (sizeof(NOROOM) + 2) >> 2;
	csp.majorVersion = X_PROTOCOL;
	csp.minorVersion = X_PROTOCOL_REVISION;
	if (((*(char *) &whichbyte) && (byteOrder == 'B')) ||
	    (!(*(char *) &whichbyte) && (byteOrder == 'l')))
	{
	    swaps(&csp.majorVersion, whichbyte);
	    swaps(&csp.minorVersion, whichbyte);
	    swaps(&csp.length, whichbyte);
	}
	iov[0].iov_len = sz_xConnSetupPrefix;
	iov[0].iov_base = (char *) &csp;
	iov[1].iov_len = csp.lengthReason;
	iov[1].iov_base = NOROOM;
	iov[2].iov_len = (4 - (csp.lengthReason & 3)) & 3;
	iov[2].iov_base = pad;
	(void)_XSERVTransWritev(trans_conn, iov, 3);
    }
}

/************
 *   CloseDownFileDescriptor:
 *     Remove this file descriptor and it's I/O buffers, etc.
 ************/

#ifdef LBX
void
CloseDownFileDescriptor(ClientPtr client)
#else
static void
CloseDownFileDescriptor(OsCommPtr oc)
#endif
{
#ifdef LBX
    OsCommPtr oc = (OsCommPtr) client->osPrivate;
#endif
    int connection = oc->fd;

    if (oc->trans_conn) {
	_XSERVTransDisconnect(oc->trans_conn);
	_XSERVTransClose(oc->trans_conn);
    }
#ifndef LBX
    FreeOsBuffers(oc);
    xfree(oc);
#endif
#ifndef WIN32
    ConnectionTranslation[connection] = 0;
#else
    SetConnectionTranslation(connection, 0);
#endif    
    FD_CLR(connection, &AllSockets);
    FD_CLR(connection, &AllClients);
    FD_CLR(connection, &ClientsWithInput);
    FD_CLR(connection, &GrabImperviousClients);
    if (GrabInProgress)
    {
	FD_CLR(connection, &SavedAllSockets);
	FD_CLR(connection, &SavedAllClients);
	FD_CLR(connection, &SavedClientsWithInput);
    }
    FD_CLR(connection, &ClientsWriteBlocked);
    if (!XFD_ANYSET(&ClientsWriteBlocked))
    	AnyClientsWriteBlocked = FALSE;
    FD_CLR(connection, &OutputPending);
}

/*****************
 * CheckConnections
 *    Some connection has died, go find which one and shut it down 
 *    The file descriptor has been closed, but is still in AllClients.
 *    If would truly be wonderful if select() would put the bogus
 *    file descriptors in the exception mask, but nooooo.  So we have
 *    to check each and every socket individually.
 *****************/

void
CheckConnections(void)
{
#ifndef WIN32
    fd_mask		mask;
#endif
    fd_set		tmask; 
    int			curclient, curoff;
    int			i;
    struct timeval	notime;
    int r;
#ifdef WIN32
    fd_set savedAllClients;
#endif

    notime.tv_sec = 0;
    notime.tv_usec = 0;

#ifndef WIN32
    for (i=0; i<howmany(XFD_SETSIZE, NFDBITS); i++)
    {
	mask = AllClients.fds_bits[i];
        while (mask)
    	{
	    curoff = ffs (mask) - 1;
	    curclient = curoff + (i * (sizeof(fd_mask)*8));
            FD_ZERO(&tmask);
            FD_SET(curclient, &tmask);
            r = Select (curclient + 1, &tmask, NULL, NULL, &notime);
            if (r < 0)
		CloseDownClient(clients[ConnectionTranslation[curclient]]);
	    mask &= ~((fd_mask)1 << curoff);
	}
    }	
#else
    XFD_COPYSET(&AllClients, &savedAllClients);
    for (i = 0; i < XFD_SETCOUNT(&savedAllClients); i++)
    {
	curclient = XFD_FD(&savedAllClients, i);
	FD_ZERO(&tmask);
	FD_SET(curclient, &tmask);
	r = Select (curclient + 1, &tmask, NULL, NULL, &notime);
	if (r < 0)
	    CloseDownClient(clients[GetConnectionTranslation(curclient)]);
    }	
#endif
}


/*****************
 * CloseDownConnection
 *    Delete client from AllClients and free resources 
 *****************/

void
CloseDownConnection(ClientPtr client)
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;

    if (oc->output && oc->output->count)
	FlushClient(client, oc, (char *)NULL, 0);
#ifdef XDMCP
    XdmcpCloseDisplay(oc->fd);
#endif
#ifndef LBX
    CloseDownFileDescriptor(oc);
#else
    (*oc->Close) (client);
    FreeOsBuffers(oc);
    xfree(oc);
#endif
    client->osPrivate = (pointer)NULL;
    if (auditTrailLevel > 1)
	AuditF("client %d disconnected\n", client->index);
}

void
AddEnabledDevice(int fd)
{
    FD_SET(fd, &EnabledDevices);
    FD_SET(fd, &AllSockets);
    if (GrabInProgress)
	FD_SET(fd, &SavedAllSockets);
}

void
RemoveEnabledDevice(int fd)
{
    FD_CLR(fd, &EnabledDevices);
    FD_CLR(fd, &AllSockets);
    if (GrabInProgress)
	FD_CLR(fd, &SavedAllSockets);
}

/*****************
 * OnlyListenToOneClient:
 *    Only accept requests from  one client.  Continue to handle new
 *    connections, but don't take any protocol requests from the new
 *    ones.  Note that if GrabInProgress is set, EstablishNewConnections
 *    needs to put new clients into SavedAllSockets and SavedAllClients.
 *    Note also that there is no timeout for this in the protocol.
 *    This routine is "undone" by ListenToAllClients()
 *****************/

void
OnlyListenToOneClient(ClientPtr client)
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    int connection = oc->fd;

    if (! GrabInProgress)
    {
	XFD_COPYSET(&ClientsWithInput, &SavedClientsWithInput);
	XFD_ANDSET(&ClientsWithInput,
		       &ClientsWithInput, &GrabImperviousClients);
	if (FD_ISSET(connection, &SavedClientsWithInput))
	{
	    FD_CLR(connection, &SavedClientsWithInput);
	    FD_SET(connection, &ClientsWithInput);
	}
	XFD_UNSET(&SavedClientsWithInput, &GrabImperviousClients);
	XFD_COPYSET(&AllSockets, &SavedAllSockets);
	XFD_COPYSET(&AllClients, &SavedAllClients);
	XFD_UNSET(&AllSockets, &AllClients);
	XFD_ANDSET(&AllClients, &AllClients, &GrabImperviousClients);
	FD_SET(connection, &AllClients);
	XFD_ORSET(&AllSockets, &AllSockets, &AllClients);
	GrabInProgress = client->index;
    }
}

/****************
 * ListenToAllClients:
 *    Undoes OnlyListentToOneClient()
 ****************/

void
ListenToAllClients(void)
{
    if (GrabInProgress)
    {
	XFD_ORSET(&AllSockets, &AllSockets, &SavedAllSockets);
	XFD_ORSET(&AllClients, &AllClients, &SavedAllClients);
	XFD_ORSET(&ClientsWithInput, &ClientsWithInput, &SavedClientsWithInput);
	GrabInProgress = 0;
    }	
}

/****************
 * IgnoreClient
 *    Removes one client from input masks.
 *    Must have cooresponding call to AttendClient.
 ****************/

void
IgnoreClient (ClientPtr client)
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    int connection = oc->fd;
#ifdef LBX
    LbxClientPtr lbxClient = LbxClient(client);
#endif

    isItTimeToYield = TRUE;
#ifdef LBX
    if (lbxClient) {
	lbxClient->ignored = TRUE;
	return;
    }
#endif
    if (!GrabInProgress || FD_ISSET(connection, &AllClients))
    {
    	if (FD_ISSET (connection, &ClientsWithInput))
	    FD_SET(connection, &IgnoredClientsWithInput);
    	else
	    FD_CLR(connection, &IgnoredClientsWithInput);
    	FD_CLR(connection, &ClientsWithInput);
    	FD_CLR(connection, &AllSockets);
    	FD_CLR(connection, &AllClients);
	FD_CLR(connection, &LastSelectMask);
    }
    else
    {
    	if (FD_ISSET (connection, &SavedClientsWithInput))
	    FD_SET(connection, &IgnoredClientsWithInput);
    	else
	    FD_CLR(connection, &IgnoredClientsWithInput);
	FD_CLR(connection, &SavedClientsWithInput);
	FD_CLR(connection, &SavedAllSockets);
	FD_CLR(connection, &SavedAllClients);
    }
}

/****************
 * AttendClient
 *    Adds one client back into the input masks.
 ****************/

void
AttendClient (ClientPtr client)
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    int connection = oc->fd;
#ifdef LBX
    LbxClientPtr lbxClient = LbxClient(client);

    if (lbxClient) {
	lbxClient->ignored = FALSE;
	return;
    }
#endif
    if (!GrabInProgress || GrabInProgress == client->index ||
	FD_ISSET(connection, &GrabImperviousClients))
    {
    	FD_SET(connection, &AllClients);
    	FD_SET(connection, &AllSockets);
	FD_SET(connection, &LastSelectMask);
    	if (FD_ISSET (connection, &IgnoredClientsWithInput))
	    FD_SET(connection, &ClientsWithInput);
    }
    else
    {
	FD_SET(connection, &SavedAllClients);
	FD_SET(connection, &SavedAllSockets);
	if (FD_ISSET(connection, &IgnoredClientsWithInput))
	    FD_SET(connection, &SavedClientsWithInput);
    }
}

/* make client impervious to grabs; assume only executing client calls this */

void
MakeClientGrabImpervious(ClientPtr client)
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    int connection = oc->fd;

    FD_SET(connection, &GrabImperviousClients);

    if (ServerGrabCallback)
    {
	ServerGrabInfoRec grabinfo;
	grabinfo.client = client;
	grabinfo.grabstate  = CLIENT_IMPERVIOUS;
	CallCallbacks(&ServerGrabCallback, &grabinfo);
    }
}

/* make client pervious to grabs; assume only executing client calls this */

void
MakeClientGrabPervious(ClientPtr client)
{
    OsCommPtr oc = (OsCommPtr)client->osPrivate;
    int connection = oc->fd;

    FD_CLR(connection, &GrabImperviousClients);
    if (GrabInProgress && (GrabInProgress != client->index))
    {
	if (FD_ISSET(connection, &ClientsWithInput))
	{
	    FD_SET(connection, &SavedClientsWithInput);
	    FD_CLR(connection, &ClientsWithInput);
	}
	FD_CLR(connection, &AllSockets);
	FD_CLR(connection, &AllClients);
	isItTimeToYield = TRUE;
    }

    if (ServerGrabCallback)
    {
	ServerGrabInfoRec grabinfo;
	grabinfo.client = client;
	grabinfo.grabstate  = CLIENT_PERVIOUS;
	CallCallbacks(&ServerGrabCallback, &grabinfo);
    }
}

