/* $XConsortium: xdmcp.c /main/34 1996/12/02 10:23:29 lehors $ */
/* $XFree86: xc/programs/Xserver/os/xdmcp.c,v 3.9 1997/01/18 06:58:04 dawes Exp $ */
/*
 * Copyright 1989 Network Computing Devices, Inc., Mountain View, California.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of N.C.D. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  N.C.D. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 */

#ifdef WIN32
/* avoid conflicting definitions */
#define BOOL wBOOL
#define ATOM wATOM
#define FreeResource wFreeResource
#include <winsock.h>
#undef BOOL
#undef ATOM
#undef FreeResource
#undef CreateWindowA
#undef RT_FONT
#undef RT_CURSOR
#endif
#include "Xos.h"
#if !defined(MINIX) && !defined(WIN32)
#ifndef Lynx
#include <sys/param.h>
#include <sys/socket.h>
#else
#include <socket.h>
#endif
#include <netinet/in.h>
#include <netdb.h>
#else
#if defined(MINIX)
#include <net/hton.h>
#include <net/netlib.h>
#include <net/gen/netdb.h>
#include <net/gen/udp.h>
#include <net/gen/udp_io.h>
#include <sys/nbio.h>
#include <sys/ioctl.h>
#endif
#endif
#include <stdio.h>
#include "X.h"
#include "Xmd.h"
#include "misc.h"
#include "Xpoll.h"
#include "osdep.h"
#include "input.h"
#include "dixstruct.h"
#include "opaque.h"

#ifdef STREAMSCONN
#include <tiuser.h>
#include <netconfig.h>
#include <netdir.h>
#endif

#ifdef XDMCP
#undef REQUEST
#include "Xdmcp.h"

extern char *display;
extern fd_set EnabledDevices;
extern fd_set AllClients;
extern char *defaultDisplayClass;

static int		    xdmcpSocket, sessionSocket;
static xdmcp_states	    state;
static struct sockaddr_in   req_sockaddr;
static int		    req_socklen;
static CARD32		    SessionID;
static CARD32		    timeOutTime;
static int		    timeOutRtx;
static CARD32		    defaultKeepaliveDormancy = XDM_DEF_DORMANCY;
static CARD32		    keepaliveDormancy = XDM_DEF_DORMANCY;
static CARD16		    DisplayNumber;
static xdmcp_states	    XDM_INIT_STATE = XDM_OFF;
#ifdef HASXDMAUTH
static char		    *xdmAuthCookie;
#endif

static XdmcpBuffer	    buffer;

static struct sockaddr_in   ManagerAddress;

static void get_xdmcp_sock(
#if NeedFunctionPrototypes
    void
#endif
);

static void send_query_msg(
#if NeedFunctionPrototypes
    void
#endif
);

static void recv_willing_msg(
#if NeedFunctionPrototypes
    struct sockaddr_in */*from*/,
    int /*fromlen*/,
    unsigned /*length*/
#endif
);

static void send_request_msg(
#if NeedFunctionPrototypes
    void
#endif
);

static void recv_accept_msg(
#if NeedFunctionPrototypes
    unsigned /*length*/
#endif
);

static void recv_decline_msg(
#if NeedFunctionPrototypes
    unsigned /*length*/
#endif
);

static void send_manage_msg(
#if NeedFunctionPrototypes
    void
#endif
);

static void recv_refuse_msg(
#if NeedFunctionPrototypes
    unsigned /*length*/
#endif
);

static void recv_failed_msg(
#if NeedFunctionPrototypes
    unsigned /*length*/
#endif
);

static void send_keepalive_msg(
#if NeedFunctionPrototypes
    void
#endif
);

static void recv_alive_msg(
#if NeedFunctionPrototypes
    unsigned /*length*/
#endif
);

static XdmcpFatal(
#if NeedFunctionPrototypes
    char */*type*/,
    ARRAY8Ptr /*status*/
#endif
);

static XdmcpWarning(
#if NeedFunctionPrototypes
    char */*str*/
#endif
);

static get_manager_by_name(
#if NeedFunctionPrototypes
    int /*argc*/,
    char **/*argv*/,
    int /*i*/
#endif
);

static void receive_packet(
#if NeedFunctionPrototypes
    void
#endif
);

static send_packet(
#if NeedFunctionPrototypes
    void
#endif
);

extern int XdmcpDeadSession(
#if NeedFunctionPrototypes
    char */*reason*/
#endif
);

static void timeout(
#if NeedFunctionPrototypes
    void
#endif
);

static restart(
#if NeedFunctionPrototypes
    void
#endif
);

static void XdmcpBlockHandler(
#if NeedFunctionPrototypes
    pointer /*data*/,
    struct timeval **/*wt*/,
    pointer /*LastSelectMask*/
#endif
);

static void XdmcpWakeupHandler(
#if NeedFunctionPrototypes
    pointer /*data*/,
    int /*i*/,
    pointer /*LastSelectMask*/
#endif
);

void XdmcpRegisterManufacturerDisplayID(
#if NeedFunctionPrototypes
    char    * /*name*/,
    int	    /*length*/
#endif
);

#ifdef MINIX
static void read_cb(
#if NeedFunctionPrototypes
    nbio_ref_t	/*ref*/,
    int		/*res*/,
    int		/*err*/
#endif
);
#endif

static short	xdm_udp_port = XDM_UDP_PORT;
static Bool	OneSession = FALSE;

XdmcpUseMsg ()
{
    ErrorF("-query host-name       contact named host for XDMCP\n");
    ErrorF("-broadcast             broadcast for XDMCP\n");
    ErrorF("-indirect host-name    contact named host for indirect XDMCP\n");
    ErrorF("-port port-num         UDP port number to send messages to\n");
    ErrorF("-once                  Terminate server after one session\n");
    ErrorF("-class display-class   specify display class to send in manage\n");
#ifdef HASXDMAUTH
    ErrorF("-cookie xdm-auth-bits  specify the magic cookie for XDMCP\n");
#endif
    ErrorF("-displayID display-id  manufacturer display ID for request\n");
}

int 
XdmcpOptions(argc, argv, i)
    int	    argc, i;
    char    **argv;
{
    if (strcmp(argv[i], "-query") == 0) {
	get_manager_by_name(argc, argv, ++i);
	XDM_INIT_STATE = XDM_QUERY;
	AccessUsingXdmcp ();
	return (i + 1);
    }
    if (strcmp(argv[i], "-broadcast") == 0) {
	XDM_INIT_STATE = XDM_BROADCAST;
	AccessUsingXdmcp ();
	return (i + 1);
    }
    if (strcmp(argv[i], "-indirect") == 0) {
	get_manager_by_name(argc, argv, ++i);
	XDM_INIT_STATE = XDM_INDIRECT;
	AccessUsingXdmcp ();
	return (i + 1);
    }
    if (strcmp(argv[i], "-port") == 0) {
	++i;
	xdm_udp_port = atoi(argv[i]);
	return (i + 1);
    }
    if (strcmp(argv[i], "-once") == 0) {
	OneSession = TRUE;
	return (i + 1);
    }
    if (strcmp(argv[i], "-class") == 0) {
	++i;
	defaultDisplayClass = argv[i];
	return (i + 1);
    }
#ifdef HASXDMAUTH
    if (strcmp(argv[i], "-cookie") == 0) {
	++i;
	xdmAuthCookie = argv[i];
	return (i + 1);
    }
#endif
    if (strcmp(argv[i], "-displayID") == 0) {
	++i;
	XdmcpRegisterManufacturerDisplayID (argv[i], strlen (argv[i]));
	return (i + 1);
    }
    return (i);
}

/*
 * This section is a collection of routines for
 * registering server-specific data with the XDMCP
 * state machine.
 */


/*
 * Save all broadcast addresses away so BroadcastQuery
 * packets get sent everywhere
 */

#define MAX_BROADCAST	10

static struct sockaddr_in   BroadcastAddresses[MAX_BROADCAST];
static int		    NumBroadcastAddresses;

void
XdmcpRegisterBroadcastAddress (addr)
    struct sockaddr_in	*addr;
{
    struct sockaddr_in	*bcast;
    if (NumBroadcastAddresses >= MAX_BROADCAST)
	return;
    bcast = &BroadcastAddresses[NumBroadcastAddresses++];
    bzero (bcast, sizeof (struct sockaddr_in));
#ifdef BSD44SOCKETS
    bcast->sin_len = addr->sin_len;
#endif
    bcast->sin_family = addr->sin_family;
    bcast->sin_port = htons (xdm_udp_port);
    bcast->sin_addr = addr->sin_addr;
}

/*
 * Each authentication type is registered here; Validator
 * will be called to check all access attempts using
 * the specified authentication type
 */

static ARRAYofARRAY8	AuthenticationNames, AuthenticationDatas;
typedef struct _AuthenticationFuncs {
    Bool    (*Validator)();
    Bool    (*Generator)();
    Bool    (*AddAuth)();
} AuthenticationFuncsRec, *AuthenticationFuncsPtr;

static AuthenticationFuncsPtr	AuthenticationFuncsList;

void
XdmcpRegisterAuthentication (name, namelen, data, datalen, Validator, Generator, AddAuth)
    char    *name;
    int	    namelen;
    char    *data;
    int	    datalen;
    Bool    (*Validator)();
    Bool    (*Generator)();
    Bool    (*AddAuth)();
{
    int	    i;
    ARRAY8  AuthenticationName, AuthenticationData;
    static AuthenticationFuncsPtr	newFuncs;

    if (!XdmcpAllocARRAY8 (&AuthenticationName, namelen))
	return;
    if (!XdmcpAllocARRAY8 (&AuthenticationData, datalen))
    {
	XdmcpDisposeARRAY8 (&AuthenticationName);
	return;
    }
    for (i = 0; i < namelen; i++)
	AuthenticationName.data[i] = name[i];
    for (i = 0; i < datalen; i++)
	AuthenticationData.data[i] = data[i];
    if (!(XdmcpReallocARRAYofARRAY8 (&AuthenticationNames,
				     AuthenticationNames.length + 1) &&
	  XdmcpReallocARRAYofARRAY8 (&AuthenticationDatas,
				     AuthenticationDatas.length + 1) &&
	  (newFuncs = (AuthenticationFuncsPtr) xalloc (
			(AuthenticationNames.length + 1) * sizeof (AuthenticationFuncsRec)))))
    {
	XdmcpDisposeARRAY8 (&AuthenticationName);
	XdmcpDisposeARRAY8 (&AuthenticationData);
	return;
    }
    for (i = 0; i < AuthenticationNames.length - 1; i++)
	newFuncs[i] = AuthenticationFuncsList[i];
    newFuncs[AuthenticationNames.length-1].Validator = Validator;
    newFuncs[AuthenticationNames.length-1].Generator = Generator;
    newFuncs[AuthenticationNames.length-1].AddAuth = AddAuth;
    xfree (AuthenticationFuncsList);
    AuthenticationFuncsList = newFuncs;
    AuthenticationNames.data[AuthenticationNames.length-1] = AuthenticationName;
    AuthenticationDatas.data[AuthenticationDatas.length-1] = AuthenticationData;
}

/*
 * Select the authentication type to be used; this is
 * set by the manager of the host to be connected to.
 */

ARRAY8		noAuthenticationName = {(CARD16) 0, (CARD8Ptr) 0};
ARRAY8		noAuthenticationData = {(CARD16) 0, (CARD8Ptr) 0};
ARRAY8Ptr	AuthenticationName = &noAuthenticationName;
ARRAY8Ptr	AuthenticationData = &noAuthenticationData;
AuthenticationFuncsPtr	AuthenticationFuncs;

void
XdmcpSetAuthentication (name)
    ARRAY8Ptr	name;
{
    int	i;

    for (i = 0; i < AuthenticationNames.length; i++)
	if (XdmcpARRAY8Equal (&AuthenticationNames.data[i], name))
	{
	    AuthenticationName = &AuthenticationNames.data[i];
	    AuthenticationData = &AuthenticationDatas.data[i];
	    AuthenticationFuncs = &AuthenticationFuncsList[i];
	    break;
	}
}

/*
 * Register the host address for the display
 */

static ARRAY16		ConnectionTypes;
static ARRAYofARRAY8	ConnectionAddresses;
static long		xdmcpGeneration;

void
XdmcpRegisterConnection (type, address, addrlen)
    int	    type;
    char    *address;
    int	    addrlen;
{
    int	    i;
    CARD8   *newAddress;

    if (xdmcpGeneration != serverGeneration)
    {
	XdmcpDisposeARRAY16 (&ConnectionTypes);
	XdmcpDisposeARRAYofARRAY8 (&ConnectionAddresses);
	xdmcpGeneration = serverGeneration;
    }
    newAddress = (CARD8 *) xalloc (addrlen * sizeof (CARD8));
    if (!newAddress)
	return;
    if (!XdmcpReallocARRAY16 (&ConnectionTypes, ConnectionTypes.length + 1))
    {
	xfree (newAddress);
	return;
    }
    if (!XdmcpReallocARRAYofARRAY8 (&ConnectionAddresses,
				    ConnectionAddresses.length +  1))
    {
	xfree (newAddress);
	return;
    }
    ConnectionTypes.data[ConnectionTypes.length - 1] = (CARD16) type;
    for (i = 0; i < addrlen; i++)
	newAddress[i] = address[i];
    ConnectionAddresses.data[ConnectionAddresses.length-1].data = newAddress;
    ConnectionAddresses.data[ConnectionAddresses.length-1].length = addrlen;
}

/*
 * Register an Authorization Name.  XDMCP advertises this list
 * to the manager.
 */

static ARRAYofARRAY8	AuthorizationNames;

void
XdmcpRegisterAuthorizations ()
{
    XdmcpDisposeARRAYofARRAY8 (&AuthorizationNames);
    RegisterAuthorizations ();
}

void
XdmcpRegisterAuthorization (name, namelen)
    char    *name;
    int	    namelen;
{
    ARRAY8  authName;
    int	    i;

    authName.data = (CARD8 *) xalloc (namelen * sizeof (CARD8));
    if (!authName.data)
	return;
    if (!XdmcpReallocARRAYofARRAY8 (&AuthorizationNames, AuthorizationNames.length +1))
    {
	xfree (authName.data);
	return;
    }
    for (i = 0; i < namelen; i++)
	authName.data[i] = (CARD8) name[i];
    authName.length = namelen;
    AuthorizationNames.data[AuthorizationNames.length-1] = authName;
}

/*
 * Register the DisplayClass string
 */

ARRAY8	DisplayClass;

void
XdmcpRegisterDisplayClass (name, length)
    char    *name;
    int	    length;
{
    int	    i;

    XdmcpDisposeARRAY8 (&DisplayClass);
    if (!XdmcpAllocARRAY8 (&DisplayClass, length))
	return;
    for (i = 0; i < length; i++)
	DisplayClass.data[i] = (CARD8) name[i];
}

/*
 * Register the Manufacturer display ID
 */

ARRAY8 ManufacturerDisplayID;

void
XdmcpRegisterManufacturerDisplayID (name, length)
    char    *name;
    int	    length;
{
    int	    i;

    XdmcpDisposeARRAY8 (&ManufacturerDisplayID);
    if (!XdmcpAllocARRAY8 (&ManufacturerDisplayID, length))
	return;
    for (i = 0; i < length; i++)
	ManufacturerDisplayID.data[i] = (CARD8) name[i];
}

/* 
 * initialize XDMCP; create the socket, compute the display
 * number, set up the state machine
 */

void 
XdmcpInit()
{
    state = XDM_INIT_STATE;
#ifdef HASXDMAUTH
    if (xdmAuthCookie)
	XdmAuthenticationInit (xdmAuthCookie, strlen (xdmAuthCookie));
#endif
    if (state != XDM_OFF)
    {
	XdmcpRegisterAuthorizations();
	XdmcpRegisterDisplayClass (defaultDisplayClass, strlen (defaultDisplayClass));
	AccessUsingXdmcp();
	RegisterBlockAndWakeupHandlers (XdmcpBlockHandler, XdmcpWakeupHandler,
				        (pointer) 0);
    	timeOutRtx = 0;
    	DisplayNumber = (CARD16) atoi(display);
    	get_xdmcp_sock();
    	send_packet();
    }
}

void
XdmcpReset ()
{
    state = XDM_INIT_STATE;
    if (state != XDM_OFF)
    {
	RegisterBlockAndWakeupHandlers (XdmcpBlockHandler, XdmcpWakeupHandler,
				        (pointer) 0);
    	timeOutRtx = 0;
    	send_packet();
    }
}

/*
 * Called whenever a new connection is created; notices the
 * first connection and saves it to terminate the session
 * when it is closed
 */

void
XdmcpOpenDisplay(sock)
    int	sock;
{
    if (state != XDM_AWAIT_MANAGE_RESPONSE)
	return;
    state = XDM_RUN_SESSION;
    sessionSocket = sock;
}

void 
XdmcpCloseDisplay(sock)
    int	sock;
{
    if ((state != XDM_RUN_SESSION && state != XDM_AWAIT_ALIVE_RESPONSE)
	|| sessionSocket != sock)
	    return;
    state = XDM_INIT_STATE;
    if (OneSession)
	dispatchException |= DE_TERMINATE;
    else
	dispatchException |= DE_RESET;
    isItTimeToYield = TRUE;
}

/*
 * called before going to sleep, this routine
 * may modify the timeout value about to be sent
 * to select; in this way XDMCP can do appropriate things
 * dynamically while starting up
 */

/*ARGSUSED*/
static void
XdmcpBlockHandler(data, wt, pReadmask)
    pointer	    data;   /* unused */
    struct timeval  **wt;
    pointer	    pReadmask;
{
    fd_set *LastSelectMask = (fd_set*)pReadmask;
    CARD32 millisToGo, wtMillis;
    static struct timeval waittime;

    if (state == XDM_OFF)
	return;
    FD_SET(xdmcpSocket, LastSelectMask);
    if (timeOutTime == 0)
	return;
    millisToGo = GetTimeInMillis();
    if (millisToGo < timeOutTime)
	millisToGo = timeOutTime - millisToGo;
    else
	millisToGo = 0;
    if (*wt == NULL)
    {
	waittime.tv_sec = (millisToGo) / 1000;
	waittime.tv_usec = 1000 * (millisToGo % 1000);
	*wt = &waittime;
    }
    else
    {
	wtMillis = (*wt)->tv_sec * 1000 + (*wt)->tv_usec / 1000;
	if (millisToGo < wtMillis)
 	{
	    (*wt)->tv_sec = (millisToGo) / 1000;
	    (*wt)->tv_usec = 1000 * (millisToGo % 1000);
	}
    }
}

/*
 * called after select returns; this routine will
 * recognise when XDMCP packets await and
 * process them appropriately
 */

/*ARGSUSED*/
static void
XdmcpWakeupHandler(data, i, pReadmask)
    pointer data;   /* unused */
    int	    i;
    pointer pReadmask;
{
    fd_set* LastSelectMask = (fd_set*)pReadmask;
    fd_set   devicesReadable;

    if (state == XDM_OFF)
	return;
    if (i > 0)
    {
	if (FD_ISSET(xdmcpSocket, LastSelectMask))
	{
	    receive_packet();
	    FD_CLR(xdmcpSocket, LastSelectMask);
	} 
	XFD_ANDSET(&devicesReadable, LastSelectMask, &EnabledDevices);
	if (XFD_ANYSET(&devicesReadable))
	{
	    if (state == XDM_AWAIT_USER_INPUT)
		restart();
	    else if (state == XDM_RUN_SESSION)
		keepaliveDormancy = defaultKeepaliveDormancy;
	}
	if (XFD_ANYSET(&AllClients) && state == XDM_RUN_SESSION)
	    timeOutTime = GetTimeInMillis() +  keepaliveDormancy * 1000;
    }
    else if (timeOutTime && GetTimeInMillis() >= timeOutTime)
    {
    	if (state == XDM_RUN_SESSION)
    	{
	    state = XDM_KEEPALIVE;
	    send_packet();
    	}
    	else
	    timeout();
    }
}

/*
 * This routine should be called from the routine that drives the
 * user's host menu when the user selects a host
 */

XdmcpSelectHost(host_sockaddr, host_len, AuthenticationName)
    struct sockaddr_in	*host_sockaddr;
    int			host_len;
    ARRAY8Ptr		AuthenticationName;
{
    state = XDM_START_CONNECTION;
    memmove(&req_sockaddr, host_sockaddr, host_len);
    req_socklen = host_len;
    XdmcpSetAuthentication (AuthenticationName);
    send_packet();
}

/*
 * !!! this routine should be replaced by a routine that adds
 * the host to the user's host menu. the current version just
 * selects the first host to respond with willing message.
 */

/*ARGSUSED*/
XdmcpAddHost(from, fromlen, AuthenticationName, hostname, status)
    struct sockaddr_in  *from;
    ARRAY8Ptr		AuthenticationName, hostname, status;
{
    XdmcpSelectHost(from, fromlen, AuthenticationName);
}

/*
 * A message is queued on the socket; read it and
 * do the appropriate thing
 */

ARRAY8	UnwillingMessage = { (CARD8) 14, (CARD8 *) "Host unwilling" };

static void
receive_packet()
{
    struct sockaddr_in from;
    int fromlen = sizeof(struct sockaddr_in);
    XdmcpHeader	header;

    /* read message off socket */
    if (!XdmcpFill (xdmcpSocket, &buffer, (struct sockaddr *) &from, &fromlen))
	return;

    /* reset retransmission backoff */
    timeOutRtx = 0;

    if (!XdmcpReadHeader (&buffer, &header))
	return;

    if (header.version != XDM_PROTOCOL_VERSION)
	return;

    switch (header.opcode) {
    case WILLING:
	recv_willing_msg(&from, fromlen, header.length);
	break;
    case UNWILLING:
	XdmcpFatal("Manager unwilling", &UnwillingMessage);
	break;
    case ACCEPT:
	recv_accept_msg(header.length);
	break;
    case DECLINE:
	recv_decline_msg(header.length);
	break;
    case REFUSE:
	recv_refuse_msg(header.length);
	break;
    case FAILED:
	recv_failed_msg(header.length);
	break;
    case ALIVE:
	recv_alive_msg(header.length);
	break;
    }
}

/*
 * send the appropriate message given the current state
 */

static
send_packet()
{
    int rtx;
    switch (state) {
    case XDM_QUERY:
    case XDM_BROADCAST:
    case XDM_INDIRECT:
	send_query_msg();
	break;
    case XDM_START_CONNECTION:
	send_request_msg();
	break;
    case XDM_MANAGE:
	send_manage_msg();
	break;
    case XDM_KEEPALIVE:
	send_keepalive_msg();
	break;
    }
    rtx = (XDM_MIN_RTX << timeOutRtx);
    if (rtx > XDM_MAX_RTX)
	rtx = XDM_MAX_RTX;
    timeOutTime = GetTimeInMillis() + rtx * 1000;
}

/*
 * The session is declared dead for some reason; too many
 * timeouts, or Keepalive failure.
 */

XdmcpDeadSession (reason)
    char *reason;
{
    ErrorF ("XDM: %s, declaring session dead\n", reason);
    state = XDM_INIT_STATE;
    isItTimeToYield = TRUE;
    dispatchException |= DE_RESET;
    timeOutTime = 0;
    timeOutRtx = 0;
    send_packet();
}

/*
 * Timeout waiting for an XDMCP response.
 */

static void
timeout()
{
    timeOutRtx++;
    if (state == XDM_AWAIT_ALIVE_RESPONSE && timeOutRtx >= XDM_KA_RTX_LIMIT )
    {
	XdmcpDeadSession ("too many keepalive retransmissions");
	return;
    }
    else if (timeOutRtx >= XDM_RTX_LIMIT)
    {
	ErrorF("XDM: too many retransmissions\n");
	state = XDM_AWAIT_USER_INPUT;
	timeOutTime = 0;
	timeOutRtx = 0;
	return;
    }

    switch (state) {
    case XDM_COLLECT_QUERY:
	state = XDM_QUERY;
	break;
    case XDM_COLLECT_BROADCAST_QUERY:
	state = XDM_BROADCAST;
	break;
    case XDM_COLLECT_INDIRECT_QUERY:
	state = XDM_INDIRECT;
	break;
    case XDM_AWAIT_REQUEST_RESPONSE:
	state = XDM_START_CONNECTION;
	break;
    case XDM_AWAIT_MANAGE_RESPONSE:
	state = XDM_MANAGE;
	break;
    case XDM_AWAIT_ALIVE_RESPONSE:
	state = XDM_KEEPALIVE;
	break;
    }
    send_packet();
}

static
restart()
{
    state = XDM_INIT_STATE;
    timeOutRtx = 0;
    send_packet();
}

XdmcpCheckAuthentication (Name, Data, packet_type)
    ARRAY8Ptr	Name, Data;
    int	packet_type;
{
    return (XdmcpARRAY8Equal (Name, AuthenticationName) &&
	    (AuthenticationName->length == 0 ||
	     (*AuthenticationFuncs->Validator) (AuthenticationData, Data, packet_type)));
}

XdmcpAddAuthorization (name, data)
    ARRAY8Ptr	name, data;
{
    Bool    (*AddAuth)(), AddAuthorization();

    if (AuthenticationFuncs && AuthenticationFuncs->AddAuth)
	AddAuth = AuthenticationFuncs->AddAuth;
    else
	AddAuth = AddAuthorization;
    return (*AddAuth) ((unsigned short)name->length,
		       (char *)name->data,
		       (unsigned short)data->length,
		       (char *)data->data);
}

/*
 * from here to the end of this file are routines private
 * to the state machine.
 */

static void
get_xdmcp_sock()
{
#ifdef STREAMSCONN
    struct netconfig *nconf;

    if ((xdmcpSocket = t_open("/dev/udp", O_RDWR, 0)) < 0) {
	XdmcpWarning("t_open() of /dev/udp failed");
	return;
    }

    if( t_bind(xdmcpSocket,NULL,NULL) < 0 ) {
	XdmcpWarning("UDP socket creation failed");
	t_error("t_bind(xdmcpSocket) failed" );
	t_close(xdmcpSocket);
	return;
    }

    /*
     * This part of the code looks contrived. It will actually fit in nicely
     * when the CLTS part of Xtrans is implemented.
     */
 
    if( (nconf=getnetconfigent("udp")) == NULL ) {
	XdmcpWarning("UDP socket creation failed: getnetconfigent()");
	t_unbind(xdmcpSocket);
	t_close(xdmcpSocket);
	return;
    }
 
    if( netdir_options(nconf, ND_SET_BROADCAST, xdmcpSocket, NULL) ) {
	XdmcpWarning("UDP set broadcast option failed: netdir_options()");
	freenetconfigent(nconf);
	t_unbind(xdmcpSocket);
	t_close(xdmcpSocket);
	return;
    }
 
    freenetconfigent(nconf);
#else
#ifndef _MINIX
    int soopts = 1;

    if ((xdmcpSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
#else /* MINIX */
    char *udp_device;
    int r, s_errno;
    nwio_udpopt_t udpopt;
    nbio_ref_t ref;

    udp_device= getenv("UDP_DEVICE");
    if (udp_device == NULL)
    	udp_device= UDP_DEVICE;
    xdmcpSocket= open(udp_device, O_RDWR);
    if (xdmcpSocket != -1)
    {
    	udpopt.nwuo_flags= NWUO_COPY | NWUO_LP_SEL | NWUO_EN_LOC | 
    		NWUO_DI_BROAD | NWUO_RP_ANY | NWUO_RA_ANY | NWUO_RWDATALL |
    		NWUO_DI_IPOPT;
    	r= ioctl(xdmcpSocket, NWIOSUDPOPT, &udpopt);
    	if (r == -1)
    	{
    		s_errno= errno;
    		close(xdmcpSocket);
    		xdmcpSocket= -1;
    		errno= s_errno;
    	}
    	ioctl(xdmcpSocket, NWIOGUDPOPT, &udpopt);
    	ErrorF("0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", 
    		udpopt.nwuo_flags,
    		udpopt.nwuo_locport,
    		udpopt.nwuo_remport,
    		udpopt.nwuo_locaddr,
    		udpopt.nwuo_remaddr);
    }
    if (xdmcpSocket != -1)
    {
	fcntl(xdmcpSocket, F_SETFD, fcntl(xdmcpSocket, F_GETFD) | 
    								FD_ASYNCHIO);
	nbio_register(xdmcpSocket);
	ref.ref_int= xdmcpSocket;
	nbio_setcallback(xdmcpSocket, ASIO_READ, read_cb, ref);
    }
    if (xdmcpSocket == -1)
#endif /* !MINIX */
	XdmcpWarning("UDP socket creation failed");
#ifdef SO_BROADCAST
    else if (setsockopt(xdmcpSocket, SOL_SOCKET, SO_BROADCAST, (char *)&soopts,
	sizeof(soopts)) < 0)
	    XdmcpWarning("UDP set broadcast socket-option failed");
#endif /* SO_BROADCAST */
#endif /* STREAMSCONN */
}

static void
send_query_msg()
{
    XdmcpHeader	header;
    Bool	broadcast = FALSE;
    int		i;

    header.version = XDM_PROTOCOL_VERSION;
    switch(state){
    case XDM_QUERY:
	header.opcode = (CARD16) QUERY; 
	state = XDM_COLLECT_QUERY;
	break;
    case XDM_BROADCAST:
	header.opcode = (CARD16) BROADCAST_QUERY;
	state = XDM_COLLECT_BROADCAST_QUERY;
	broadcast = TRUE;
	break;
    case XDM_INDIRECT:
	header.opcode = (CARD16) INDIRECT_QUERY;
	state = XDM_COLLECT_INDIRECT_QUERY;
	break;
    }
    header.length = 1;
    for (i = 0; i < AuthenticationNames.length; i++)
	header.length += 2 + AuthenticationNames.data[i].length;

    XdmcpWriteHeader (&buffer, &header);
    XdmcpWriteARRAYofARRAY8 (&buffer, &AuthenticationNames);
    if (broadcast)
    {
	int i;

	for (i = 0; i < NumBroadcastAddresses; i++)
	    XdmcpFlush (xdmcpSocket, &buffer, &BroadcastAddresses[i],
			sizeof (struct sockaddr_in));
    }
    else
    {
	XdmcpFlush (xdmcpSocket, &buffer, &ManagerAddress,
		    sizeof (ManagerAddress));
    }
}

static void
recv_willing_msg(from, fromlen, length)
    struct sockaddr_in	*from;
    int			fromlen;
    unsigned		length;
{
    ARRAY8	authenticationName;
    ARRAY8	hostname;
    ARRAY8	status;

    authenticationName.data = 0;
    hostname.data = 0;
    status.data = 0;
    if (XdmcpReadARRAY8 (&buffer, &authenticationName) &&
	XdmcpReadARRAY8 (&buffer, &hostname) &&
	XdmcpReadARRAY8 (&buffer, &status))
    {
    	if (length == 6 + authenticationName.length +
		      hostname.length + status.length)
    	{
	    switch (state)
	    {
	    case XDM_COLLECT_QUERY:
	    	XdmcpSelectHost(from, fromlen, &authenticationName);
	    	break;
	    case XDM_COLLECT_BROADCAST_QUERY:
	    case XDM_COLLECT_INDIRECT_QUERY:
	    	XdmcpAddHost(from, fromlen, &authenticationName, &hostname, &status);
	    	break;
    	    }
    	}
    }
    XdmcpDisposeARRAY8 (&authenticationName);
    XdmcpDisposeARRAY8 (&hostname);
    XdmcpDisposeARRAY8 (&status);
}

static void
send_request_msg()
{
    XdmcpHeader	    header;
    int		    length;
    int		    i;
    ARRAY8	    authenticationData;

    header.version = XDM_PROTOCOL_VERSION;
    header.opcode = (CARD16) REQUEST;

    length = 2;					    /* display number */
    length += 1 + 2 * ConnectionTypes.length;	    /* connection types */
    length += 1;				    /* connection addresses */
    for (i = 0; i < ConnectionAddresses.length; i++)
	length += 2 + ConnectionAddresses.data[i].length;
    authenticationData.length = 0;
    authenticationData.data = 0;
    if (AuthenticationFuncs)
    {
	(*AuthenticationFuncs->Generator) (AuthenticationData,
					   &authenticationData,
 					   REQUEST);
    }
    length += 2 + AuthenticationName->length;	    /* authentication name */
    length += 2 + authenticationData.length;	    /* authentication data */
    length += 1;				    /* authorization names */
    for (i = 0; i < AuthorizationNames.length; i++)
	length += 2 + AuthorizationNames.data[i].length;
    length += 2 + ManufacturerDisplayID.length;	    /* display ID */
    header.length = length;

    if (!XdmcpWriteHeader (&buffer, &header))
    {
	XdmcpDisposeARRAY8 (&authenticationData);
	return;
    }
    XdmcpWriteCARD16 (&buffer, DisplayNumber);
    XdmcpWriteARRAY16 (&buffer, &ConnectionTypes);
    XdmcpWriteARRAYofARRAY8 (&buffer, &ConnectionAddresses);

    XdmcpWriteARRAY8 (&buffer, AuthenticationName);
    XdmcpWriteARRAY8 (&buffer, &authenticationData);
    XdmcpDisposeARRAY8 (&authenticationData);
    XdmcpWriteARRAYofARRAY8 (&buffer, &AuthorizationNames);
    XdmcpWriteARRAY8 (&buffer, &ManufacturerDisplayID);
    if (XdmcpFlush (xdmcpSocket, &buffer, &req_sockaddr, req_socklen))
	state = XDM_AWAIT_REQUEST_RESPONSE;
}

static void
recv_accept_msg(length)
    unsigned		length;
{
    CARD32  AcceptSessionID;
    ARRAY8  AcceptAuthenticationName, AcceptAuthenticationData;
    ARRAY8  AcceptAuthorizationName, AcceptAuthorizationData;

    if (state != XDM_AWAIT_REQUEST_RESPONSE)
	return;
    AcceptAuthenticationName.data = 0;
    AcceptAuthenticationData.data = 0;
    AcceptAuthorizationName.data = 0;
    AcceptAuthorizationData.data = 0;
    if (XdmcpReadCARD32 (&buffer, &AcceptSessionID) &&
	XdmcpReadARRAY8 (&buffer, &AcceptAuthenticationName) &&
	XdmcpReadARRAY8 (&buffer, &AcceptAuthenticationData) &&
	XdmcpReadARRAY8 (&buffer, &AcceptAuthorizationName) &&
	XdmcpReadARRAY8 (&buffer, &AcceptAuthorizationData))
    {
    	if (length == 12 + AcceptAuthenticationName.length +
		      	   AcceptAuthenticationData.length +
		      	   AcceptAuthorizationName.length +
 		      	   AcceptAuthorizationData.length)
    	{
	    if (!XdmcpCheckAuthentication (&AcceptAuthenticationName,
				      &AcceptAuthenticationData, ACCEPT))
	    {
		XdmcpFatal ("Authentication Failure", &AcceptAuthenticationName);
	    }
	    /* permit access control manipulations from this host */
	    AugmentSelf (&req_sockaddr, req_socklen);
	    /* if the authorization specified in the packet fails
	     * to be acceptable, enable the local addresses
	     */
	    if (!XdmcpAddAuthorization (&AcceptAuthorizationName,
					&AcceptAuthorizationData))
	    {
		AddLocalHosts ();
	    }
	    SessionID = AcceptSessionID;
    	    state = XDM_MANAGE;
    	    send_packet();
    	}
    }
    XdmcpDisposeARRAY8 (&AcceptAuthenticationName);
    XdmcpDisposeARRAY8 (&AcceptAuthenticationData);
    XdmcpDisposeARRAY8 (&AcceptAuthorizationName);
    XdmcpDisposeARRAY8 (&AcceptAuthorizationData);
}

static void
recv_decline_msg(length)
    unsigned		length;
{
    ARRAY8  status, DeclineAuthenticationName, DeclineAuthenticationData;

    status.data = 0;
    DeclineAuthenticationName.data = 0;
    DeclineAuthenticationData.data = 0;
    if (XdmcpReadARRAY8 (&buffer, &status) &&
	XdmcpReadARRAY8 (&buffer, &DeclineAuthenticationName) &&
	XdmcpReadARRAY8 (&buffer, &DeclineAuthenticationData))
    {
    	if (length == 6 + status.length +
		      	  DeclineAuthenticationName.length +
 		      	  DeclineAuthenticationData.length &&
	    XdmcpCheckAuthentication (&DeclineAuthenticationName,
				      &DeclineAuthenticationData, DECLINE))
    	{
	    XdmcpFatal ("Session declined", &status);
    	}
    }
    XdmcpDisposeARRAY8 (&status);
    XdmcpDisposeARRAY8 (&DeclineAuthenticationName);
    XdmcpDisposeARRAY8 (&DeclineAuthenticationData);
}

static void
send_manage_msg()
{
    XdmcpHeader	header;

    header.version = XDM_PROTOCOL_VERSION;
    header.opcode = (CARD16) MANAGE;
    header.length = 8 + DisplayClass.length;

    if (!XdmcpWriteHeader (&buffer, &header))
	return;
    XdmcpWriteCARD32 (&buffer, SessionID);
    XdmcpWriteCARD16 (&buffer, DisplayNumber);
    XdmcpWriteARRAY8 (&buffer, &DisplayClass);
    state = XDM_AWAIT_MANAGE_RESPONSE;
    XdmcpFlush (xdmcpSocket, &buffer, &req_sockaddr, req_socklen);
}

static void
recv_refuse_msg(length)
    unsigned		length;
{
    CARD32  RefusedSessionID;

    if (state != XDM_AWAIT_MANAGE_RESPONSE)
	return;
    if (length != 4)
	return;
    if (XdmcpReadCARD32 (&buffer, &RefusedSessionID))
    {
	if (RefusedSessionID == SessionID)
	{
    	    state = XDM_START_CONNECTION;
    	    send_packet();
	}
    }
}

static void
recv_failed_msg(length)
    unsigned		length;
{
    CARD32  FailedSessionID;
    ARRAY8  status;

    if (state != XDM_AWAIT_MANAGE_RESPONSE)
	return;
    status.data = 0;
    if (XdmcpReadCARD32 (&buffer, &FailedSessionID) &&
	XdmcpReadARRAY8 (&buffer, &status))
    {
    	if (length == 6 + status.length &&
	    SessionID == FailedSessionID)
	{
	    XdmcpFatal ("Session failed", &status);
	}
    }
    XdmcpDisposeARRAY8 (&status);
}

static void
send_keepalive_msg()
{
    XdmcpHeader	header;

    header.version = XDM_PROTOCOL_VERSION;
    header.opcode = (CARD16) KEEPALIVE;
    header.length = 6;

    XdmcpWriteHeader (&buffer, &header);
    XdmcpWriteCARD16 (&buffer, DisplayNumber);
    XdmcpWriteCARD32 (&buffer, SessionID);

    state = XDM_AWAIT_ALIVE_RESPONSE;
    XdmcpFlush (xdmcpSocket, &buffer, &req_sockaddr, req_socklen);
}

static void
recv_alive_msg (length)
    unsigned		length;
{
    CARD8   SessionRunning;
    CARD32  AliveSessionID;
    int	    dormancy;

    if (state != XDM_AWAIT_ALIVE_RESPONSE)
	return;
    if (length != 5)
	return;
    if (XdmcpReadCARD8 (&buffer, &SessionRunning) &&
	XdmcpReadCARD32 (&buffer, &AliveSessionID))
    {
    	if (SessionRunning && AliveSessionID == SessionID)
    	{
	    /* backoff dormancy period */
	    state = XDM_RUN_SESSION;
	    if ((GetTimeInMillis() - lastDeviceEventTime.milliseconds) >
		keepaliveDormancy * 1000)
	    {
		keepaliveDormancy <<= 1;
		if (keepaliveDormancy > XDM_MAX_DORMANCY)
		    keepaliveDormancy = XDM_MAX_DORMANCY;
	    }
	    timeOutTime = GetTimeInMillis() + keepaliveDormancy * 1000;
    	}
	else
    	{
	    XdmcpDeadSession ("Alive respose indicates session dead");
    	}
    }
}

static 
XdmcpFatal (type, status)
    char	*type;
    ARRAY8Ptr	status;
{
    FatalError ("XDMCP fatal error: %s %*.*s\n", type,
	   status->length, status->length, status->data);
}

static 
XdmcpWarning(str)
    char *str;
{
    ErrorF("XDMCP warning: %s\n", str);
}

static
get_manager_by_name(argc, argv, i)
    int	    argc, i;
    char    **argv;
{
    struct hostent *hep;

    if (i == argc)
    {
	ErrorF("Xserver: missing host name in command line\n");
	exit(1);
    }
    if (!(hep = gethostbyname(argv[i])))
    {
	ErrorF("Xserver: unknown host: %s\n", argv[i]);
	exit(1);
    }
#ifndef _MINIX
    if (hep->h_length == sizeof (struct in_addr))
#else
    if (hep->h_length == sizeof (ipaddr_t))
#endif
    {
	memmove(&ManagerAddress.sin_addr, hep->h_addr, hep->h_length);
#ifdef BSD44SOCKETS
	ManagerAddress.sin_len = sizeof(ManagerAddress);
#endif
	ManagerAddress.sin_family = AF_INET;
	ManagerAddress.sin_port = htons (xdm_udp_port);
    }
    else
    {
	ErrorF ("Xserver: host on strange network %s\n", argv[i]);
	exit (1);
    }
}

#ifdef MINIX
static char read_buffer[XDM_MAX_MSGLEN+sizeof(udp_io_hdr_t)];
static int read_inprogress;
static int read_size;

int
XdmcpFill (fd, buffer, from, fromlen)
int             fd;
XdmcpBufferPtr  buffer;
XdmcpNetaddr    from;       /* return */
int             *fromlen;   /* return */
{
	int r;

	if (read_inprogress)
		return 0;

	if (read_size != 0)
	{
		r= read_size;
		read_size= 0;
		return MNX_XdmcpFill(fd, buffer, from, fromlen, read_buffer,
			r);
	}

	r= read(fd, read_buffer, sizeof(read_buffer));
	if (r > 0)
	{
		return MNX_XdmcpFill(fd, buffer, from, fromlen, read_buffer,
			r);
	}
	else if (r == -1 && errno == EINPROGRESS)
	{
		read_inprogress= 1;
		nbio_inprogress(fd, ASIO_READ, 1 /* read */, 0 /* write */,
			0 /* except */);
		return 0;
	}
	else
		FatalError("XdmcpFill: read failed: %s\n",
			r == 0 ? "EOF" : strerror(errno));
	return 0;
}

static void read_cb(ref, res, err)
nbio_ref_t ref;
int res;
int err;
{
	if (res <= 0)
	{
		FatalError("xdmcp'read_cb: read failed: %s\n",
			res == 0 ? "EOF" : strerror(err));
	}
	read_inprogress= 0;
	read_size= res;
}
#endif

#else
static int xdmcp_non_empty; /* avoid complaint by ranlib */
#endif /* XDMCP */
