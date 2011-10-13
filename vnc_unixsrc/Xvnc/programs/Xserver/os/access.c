/* $Xorg: access.c,v 1.5 2001/02/09 02:05:23 xorgcvs Exp $ */
/* $XdotOrg: xc/programs/Xserver/os/access.c,v 1.13 2005/11/08 06:33:30 jkj Exp $ */
/***********************************************************

Copyright 1987, 1998  The Open Group
Copyright 2004 Sun Microsystems, Inc.

All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, provided that the above
copyright notice(s) and this permission notice appear in all copies of
the Software and that both the above copyright notice(s) and this
permission notice appear in supporting documentation.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

Except as contained in this notice, the name of a copyright holder
shall not be used in advertising or otherwise to promote the sale, use
or other dealings in this Software without prior written authorization
of the copyright holder.

X Window System is a trademark of The Open Group.

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

******************************************************************/
/* $XFree86: xc/programs/Xserver/os/access.c,v 3.53 2004/01/02 18:23:19 tsi Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifdef WIN32
#include <X11/Xwinsock.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xtrans/Xtrans.h>
#include <X11/Xauth.h>
#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "site.h"
#include <errno.h>
#include <sys/types.h>
#ifndef WIN32
#ifndef Lynx
#include <sys/socket.h>
#else
#include <socket.h>
#endif
#include <sys/ioctl.h>
#include <ctype.h>

#if defined(TCPCONN) || defined(STREAMSCONN) || defined(ISC) || defined(__SCO__)
#include <netinet/in.h>
#endif /* TCPCONN || STREAMSCONN || ISC || __SCO__ */
#ifdef DNETCONN
#include <netdnet/dn.h>
#include <netdnet/dnetdb.h>
#endif

#ifdef HAS_GETPEERUCRED
# include <ucred.h>
# ifdef sun
#  include <zone.h>
# endif
#endif

#if defined(DGUX)
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <ctype.h>
#include <sys/utsname.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/param.h>
#include <sys/sockio.h>
#endif


#if defined(hpux) || defined(QNX4)
# include <sys/utsname.h>
# ifdef HAS_IFREQ
#  include <net/if.h>
# endif
#else
#if defined(SVR4) ||  (defined(SYSV) && defined(i386)) || defined(__GNU__)
# include <sys/utsname.h>
#endif
#if defined(SYSV) &&  defined(i386)
# include <sys/stream.h>
# ifdef ISC
#  include <sys/stropts.h>
#  include <sys/sioctl.h>
# endif /* ISC */
#endif
#ifdef __GNU__
#undef SIOCGIFCONF
#include <netdb.h>
#else /*!__GNU__*/
# include <net/if.h>
#endif /*__GNU__ */
#endif /* hpux */

#ifdef SVR4
#include <sys/sockio.h>
#include <sys/stropts.h>
#endif

#include <netdb.h>

#ifdef CSRG_BASED
#include <sys/param.h>
#if (BSD >= 199103)
#define VARIABLE_IFREQ
#endif
#endif

#ifdef BSD44SOCKETS
#ifndef VARIABLE_IFREQ
#define VARIABLE_IFREQ
#endif
#endif

#ifdef HAS_GETIFADDRS
#include <ifaddrs.h>
#endif

/* Solaris provides an extended interface SIOCGLIFCONF.  Other systems
 * may have this as well, but the code has only been tested on Solaris
 * so far, so we only enable it there.  Other platforms may be added as
 * needed.
 *
 * Test for Solaris commented out  --  TSI @ UQV  2003.06.13
 */
#ifdef SIOCGLIFCONF
/* #if defined(sun) */
#define USE_SIOCGLIFCONF
/* #endif */
#endif

#endif /* WIN32 */

#ifndef PATH_MAX
#ifndef Lynx
#include <sys/param.h>
#else
#include <param.h>
#endif 
#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 1024
#endif
#endif
#endif 

#ifdef __SCO__
/* The system defined value is wrong. MAXPATHLEN is set in sco5.cf. */
#undef PATH_MAX
#endif

#define X_INCLUDE_NETDB_H
#include <X11/Xos_r.h>

#include "dixstruct.h"
#include "osdep.h"

#ifdef XCSECURITY
#define _SECURITY_SERVER
#include <X11/extensions/security.h>
#endif

#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 1024
#endif
#endif

Bool defeatAccessControl = FALSE;

#define acmp(a1, a2, len) memcmp((char *)(a1), (char *)(a2), len)
#define acopy(a1, a2, len) memmove((char *)(a2), (char *)(a1), len)
#define addrEqual(fam, address, length, host) \
			 ((fam) == (host)->family &&\
			  (length) == (host)->len &&\
			  !acmp (address, (host)->addr, length))

static int ConvertAddr(struct sockaddr * /*saddr*/,
		       int * /*len*/,
		       pointer * /*addr*/);

static int CheckAddr(int /*family*/,
		     pointer /*pAddr*/,
		     unsigned /*length*/);

static Bool NewHost(int /*family*/,
		    pointer /*addr*/,
		    int /*len*/,
		    int /* addingLocalHosts */);

int LocalClientCredAndGroups(ClientPtr client, int *pUid, int *pGid, 
                             int **pSuppGids, int *nSuppGids);


/* XFree86 bug #156: To keep track of which hosts were explicitly requested in
   /etc/X<display>.hosts, we've added a requested field to the HOST struct,
   and a LocalHostRequested variable.  These default to FALSE, but are set
   to TRUE in ResetHosts when reading in /etc/X<display>.hosts.  They are
   checked in DisableLocalHost(), which is called to disable the default 
   local host entries when stronger authentication is turned on. */

typedef struct _host {
	short		family;
	short		len;
	unsigned char	*addr;
	struct _host *next;
	int		requested;
} HOST;

#define MakeHost(h,l)	(h)=(HOST *) xalloc(sizeof *(h)+(l));\
			if (h) { \
			   (h)->addr=(unsigned char *) ((h) + 1);\
			   (h)->requested = FALSE; \
			}
#define FreeHost(h)	xfree(h)
static HOST *selfhosts = NULL;
static HOST *validhosts = NULL;
static int AccessEnabled = DEFAULT_ACCESS_CONTROL;
static int LocalHostEnabled = FALSE;
static int LocalHostRequested = FALSE;
static int UsingXdmcp = FALSE;

/* FamilyServerInterpreted implementation */
static Bool siAddrMatch(int family, pointer addr, int len, HOST *host, 
	ClientPtr client);
static int  siCheckAddr(const char *addrString, int length);
static void siTypesInitialize(void);

/*
 * called when authorization is not enabled to add the
 * local host to the access list
 */

void
EnableLocalHost (void)
{
    if (!UsingXdmcp)
    {
	LocalHostEnabled = TRUE;
	AddLocalHosts ();
    }
}

/*
 * called when authorization is enabled to keep us secure
 */
void
DisableLocalHost (void)
{
    HOST *self;

    if (!LocalHostRequested)		/* Fix for XFree86 bug #156 */
	LocalHostEnabled = FALSE;
    for (self = selfhosts; self; self = self->next) {
      if (!self->requested)		/* Fix for XFree86 bug #156 */
	(void) RemoveHost ((ClientPtr)NULL, self->family, self->len, (pointer)self->addr);
    }
}

/*
 * called at init time when XDMCP will be used; xdmcp always
 * adds local hosts manually when needed
 */

void
AccessUsingXdmcp (void)
{
    UsingXdmcp = TRUE;
    LocalHostEnabled = FALSE;
}


#if ((defined(SVR4) && !defined(DGUX) && !defined(SCO325) && !defined(sun) && !defined(NCR)) || defined(ISC)) && !defined(__sgi) && defined(SIOCGIFCONF) && !defined(USE_SIOCGLIFCONF)

/* Deal with different SIOCGIFCONF ioctl semantics on these OSs */

static int
ifioctl (int fd, int cmd, char *arg)
{
    struct strioctl ioc;
    int ret;

    bzero((char *) &ioc, sizeof(ioc));
    ioc.ic_cmd = cmd;
    ioc.ic_timout = 0;
    if (cmd == SIOCGIFCONF)
    {
	ioc.ic_len = ((struct ifconf *) arg)->ifc_len;
	ioc.ic_dp = ((struct ifconf *) arg)->ifc_buf;
#ifdef ISC
	/* SIOCGIFCONF is somewhat brain damaged on ISC. The argument
	 * buffer must contain the ifconf structure as header. Ifc_req
	 * is also not a pointer but a one element array of ifreq
	 * structures. On return this array is extended by enough
	 * ifreq fields to hold all interfaces. The return buffer length
	 * is placed in the buffer header.
	 */
        ((struct ifconf *) ioc.ic_dp)->ifc_len =
                                         ioc.ic_len - sizeof(struct ifconf);
#endif
    }
    else
    {
	ioc.ic_len = sizeof(struct ifreq);
	ioc.ic_dp = arg;
    }
    ret = ioctl(fd, I_STR, (char *) &ioc);
    if (ret >= 0 && cmd == SIOCGIFCONF)
#ifdef SVR4
	((struct ifconf *) arg)->ifc_len = ioc.ic_len;
#endif
#ifdef ISC
    {
	((struct ifconf *) arg)->ifc_len =
				 ((struct ifconf *)ioc.ic_dp)->ifc_len;
	((struct ifconf *) arg)->ifc_buf = 
			(caddr_t)((struct ifconf *)ioc.ic_dp)->ifc_req;
    }
#endif
    return(ret);
}
#else /* Case DGUX, sun, SCO325 NCR and others  */
#define ifioctl ioctl
#endif /* ((SVR4 && !DGUX !sun !SCO325 !NCR) || ISC) && SIOCGIFCONF */

/*
 * DefineSelf (fd):
 *
 * Define this host for access control.  Find all the hosts the OS knows about 
 * for this fd and add them to the selfhosts list.
 */

#ifdef WINTCP /* NCR Wollongong based TCP */

#include <sys/un.h>
#include <stropts.h>
#include <tiuser.h>

#include <sys/stream.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/in.h>
#include <netinet/in_var.h>

void
DefineSelf (int fd)
{
    /*
     * The Wolongong drivers used by NCR SVR4/MP-RAS don't understand the
     * socket IO calls that most other drivers seem to like. Because of
     * this, this routine must be special cased for NCR. Eventually,
     * this will be cleared up.
     */

    struct ipb ifnet;
    struct in_ifaddr ifaddr;
    struct strioctl str;
    unsigned char *addr;
    register HOST *host;
    int	family, len;

    if ((fd = open ("/dev/ip", O_RDWR, 0 )) < 0)
        Error ("Getting interface configuration (1)");

    /* Indicate that we want to start at the begining */
    ifnet.ib_next = (struct ipb *) 1;

    while (ifnet.ib_next)
    {
	str.ic_cmd = IPIOC_GETIPB;
	str.ic_timout = 0;
	str.ic_len = sizeof (struct ipb);
	str.ic_dp = (char *) &ifnet;

	if (ioctl (fd, (int) I_STR, (char *) &str) < 0)
	{
	    close (fd);
	    Error ("Getting interface configuration (2)");
	}

	ifaddr.ia_next = (struct in_ifaddr *) ifnet.if_addrlist;
	str.ic_cmd = IPIOC_GETINADDR;
	str.ic_timout = 0;
	str.ic_len = sizeof (struct in_ifaddr);
	str.ic_dp = (char *) &ifaddr;

	if (ioctl (fd, (int) I_STR, (char *) &str) < 0)
	{
	    close (fd);
	    Error ("Getting interface configuration (3)");
	}

	len = sizeof(struct sockaddr_in);
	family = ConvertAddr (IA_SIN(&ifaddr), &len, (pointer *)&addr);
        if (family == -1 || family == FamilyLocal)
	    continue;
        for (host = selfhosts;
 	     host && !addrEqual (family, addr, len, host);
	     host = host->next)
	    ;
        if (host)
	    continue;
	MakeHost(host,len)
	if (host)
	{
	    host->family = family;
	    host->len = len;
	    acopy(addr, host->addr, len);
	    host->next = selfhosts;
	    selfhosts = host;
	}
#ifdef XDMCP
        {
	    struct sockaddr broad_addr;

	    /*
	     * If this isn't an Internet Address, don't register it.
	     */
	    if (family != FamilyInternet)
		continue;

	    /*
 	     * Ignore 'localhost' entries as they're not useful
	     * on the other end of the wire.
	     */
	    if (len == 4 &&
		addr[0] == 127 && addr[1] == 0 &&
		addr[2] == 0 && addr[3] == 1)
		continue;

	    /*
	     * Ignore '0.0.0.0' entries as they are
	     * returned by some OSes for unconfigured NICs but they are
	     * not useful on the other end of the wire.
	     */
	    if (len == 4 &&
		addr[0] == 0 && addr[1] == 0 &&
		addr[2] == 0 && addr[3] == 0)
		continue;

	    XdmcpRegisterConnection (family, (char *)addr, len);


#define IA_BROADADDR(ia) ((struct sockaddr_in *)(&((struct in_ifaddr *)ia)->ia_broadaddr))

	    XdmcpRegisterBroadcastAddress (
		(struct sockaddr_in *) IA_BROADADDR(&ifaddr));

#undef IA_BROADADDR
	}
#endif /* XDMCP */
    }

    close(fd);

    /*
     * add something of FamilyLocalHost
     */
    for (host = selfhosts;
	 host && !addrEqual(FamilyLocalHost, "", 0, host);
	 host = host->next);
    if (!host)
    {
	MakeHost(host, 0);
	if (host)
	{
	    host->family = FamilyLocalHost;
	    host->len = 0;
	    acopy("", host->addr, 0);
	    host->next = selfhosts;
	    selfhosts = host;
	}
    }
}

#else /* WINTCP */

#if !defined(SIOCGIFCONF) || (defined (hpux) && ! defined (HAS_IFREQ)) || defined(QNX4)
void
DefineSelf (int fd)
{
#if !defined(TCPCONN) && !defined(STREAMSCONN) && !defined(UNIXCONN) && !defined(MNX_TCPCONN)
    return;
#else
    register int n;
    int	len;
    caddr_t	addr;
    int		family;
    register HOST	*host;

#ifndef WIN32
    struct utsname name;
#else
    struct {
        char  nodename[512];	    
    } name;
#endif

    register struct hostent  *hp;

    union {
	struct  sockaddr   sa;
	struct  sockaddr_in  in;
#if defined(IPv6) && defined(AF_INET6)
	struct  sockaddr_in6  in6;
#endif
    } saddr;
	
    struct	sockaddr_in	*inetaddr;
    struct	sockaddr_in6	*inet6addr;
    struct sockaddr_in broad_addr;
#ifdef XTHREADS_NEEDS_BYNAMEPARAMS
    _Xgethostbynameparams hparams;
#endif

    /* Why not use gethostname()?  Well, at least on my system, I've had to
     * make an ugly kernel patch to get a name longer than 8 characters, and
     * uname() lets me access to the whole string (it smashes release, you
     * see), whereas gethostname() kindly truncates it for me.
     */
#ifndef QNX4
#ifndef WIN32
    uname(&name);
#else
    gethostname(name.nodename, sizeof(name.nodename));
#endif
#else
    /* QNX4's uname returns node number in name.nodename, not the hostname
       have to overwrite it */
    char hname[1024];
    gethostname(hname, 1024);
    name.nodename = hname;
#endif

    hp = _XGethostbyname(name.nodename, hparams);
    if (hp != NULL)
    {
	saddr.sa.sa_family = hp->h_addrtype;
	switch (hp->h_addrtype) {
	case AF_INET:
	    inetaddr = (struct sockaddr_in *) (&(saddr.sa));
	    acopy ( hp->h_addr, &(inetaddr->sin_addr), hp->h_length);
	    len = sizeof(saddr.sa);
	    break;
#if defined(IPv6) && defined(AF_INET6)
	case AF_INET6:
	    inet6addr = (struct sockaddr_in6 *) (&(saddr.sa));
	    acopy ( hp->h_addr, &(inet6addr->sin6_addr), hp->h_length);
	    len = sizeof(saddr.in6);
	    break;
#endif
	default:
	    goto DefineLocalHost;
	}
	family = ConvertAddr ( &(saddr.sa), &len, (pointer *)&addr);
	if ( family != -1 && family != FamilyLocal )
	{
	    for (host = selfhosts;
		 host && !addrEqual (family, addr, len, host);
		 host = host->next) ;
	    if (!host)
	    {
		/* add this host to the host list.	*/
		MakeHost(host,len)
		if (host)
		{
		    host->family = family;
		    host->len = len;
		    acopy ( addr, host->addr, len);
		    host->next = selfhosts;
		    selfhosts = host;
		}
#ifdef XDMCP
		/*
		 *  If this is an Internet Address, but not the localhost
		 *  address (127.0.0.1), nor the bogus address (0.0.0.0),
		 *  register it.
		 */
		if (family == FamilyInternet &&
		    !(len == 4 &&
		      ((addr[0] == 127 && addr[1] == 0 &&
			addr[2] == 0 && addr[3] == 1) ||
		       (addr[0] == 0 && addr[1] == 0 &&
			addr[2] == 0 && addr[3] == 0)))
		      )
		{
		    XdmcpRegisterConnection (family, (char *)addr, len);
		    broad_addr = *inetaddr;
		    ((struct sockaddr_in *) &broad_addr)->sin_addr.s_addr =
			htonl (INADDR_BROADCAST);
		    XdmcpRegisterBroadcastAddress ((struct sockaddr_in *)
						   &broad_addr);
		}
#if defined(IPv6) && defined(AF_INET6)
		else if (family == FamilyInternet6 &&
		  !(IN6_IS_ADDR_LOOPBACK((struct in6_addr *)addr)))
		{
		    XdmcpRegisterConnection (family, (char *)addr, len);
		}
#endif

#endif /* XDMCP */
	    }
	}
    }
    /*
     * now add a host of family FamilyLocalHost...
     */
DefineLocalHost:
    for (host = selfhosts;
	 host && !addrEqual(FamilyLocalHost, "", 0, host);
	 host = host->next);
    if (!host)
    {
	MakeHost(host, 0);
	if (host)
	{
	    host->family = FamilyLocalHost;
	    host->len = 0;
	    acopy("", host->addr, 0);
	    host->next = selfhosts;
	    selfhosts = host;
	}
    }
#endif /* !TCPCONN && !STREAMSCONN && !UNIXCONN && !MNX_TCPCONN */
}

#else

#ifdef USE_SIOCGLIFCONF
#define ifr_type    struct lifreq
#else
#define ifr_type    struct ifreq
#endif

#ifdef VARIABLE_IFREQ
#define ifr_size(p) (sizeof (struct ifreq) + \
		     (p->ifr_addr.sa_len > sizeof (p->ifr_addr) ? \
		      p->ifr_addr.sa_len - sizeof (p->ifr_addr) : 0))
#define ifraddr_size(a) (a.sa_len)
#else
#ifdef QNX4
#define ifr_size(p) (p->ifr_addr.sa_len + IFNAMSIZ)
#define ifraddr_size(a) (a.sa_len)
#else
#define ifr_size(p) (sizeof (ifr_type))
#define ifraddr_size(a) (sizeof (a))
#endif
#endif

#if defined(DEF_SELF_DEBUG) || (defined(IPv6) && defined(AF_INET6))
#include <arpa/inet.h>
#endif

#if defined(IPv6) && defined(AF_INET6)
static void
in6_fillscopeid(struct sockaddr_in6 *sin6)
{
#if defined(__KAME__)
	if (IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr)) {
		sin6->sin6_scope_id =
			ntohs(*(u_int16_t *)&sin6->sin6_addr.s6_addr[2]);
		sin6->sin6_addr.s6_addr[2] = sin6->sin6_addr.s6_addr[3] = 0;
	}
#endif
}
#endif

void
DefineSelf (int fd)
{
#ifndef HAS_GETIFADDRS
    char		buf[2048], *cp, *cplim;
    void *		bufptr = buf;   
#ifdef USE_SIOCGLIFCONF
    struct lifconf	ifc;
    register struct lifreq *ifr;
#ifdef SIOCGLIFNUM
    struct lifnum	ifn;
#endif
#else
    struct ifconf	ifc;
    register struct ifreq *ifr;
#endif 
#else 
    struct ifaddrs *	ifap, *ifr;
#endif
    int 		len;
    unsigned char *	addr;
    int 		family;
    register HOST 	*host;
    
#ifdef DNETCONN
    struct dn_naddr *dnaddr = getnodeadd();
    /*
     * AF_DECnet may not be listed in the interface list.  Instead use
     * the supported library call to find out the local address (if any).
     */
    if (dnaddr)
    {    
	addr = (unsigned char *) dnaddr;
	len = dnaddr->a_len + sizeof(dnaddr->a_len);
	family = FamilyDECnet;
	for (host = selfhosts;
	     host && !addrEqual (family, addr, len, host);
	     host = host->next)
	    ;
        if (!host)
	{
	    MakeHost(host,len)
	    if (host)
	    {
		host->family = family;
		host->len = len;
		acopy(addr, host->addr, len);
		host->next = selfhosts;
		selfhosts = host;
	    }
	}
    }
#endif /* DNETCONN */
#ifndef HAS_GETIFADDRS

    len = sizeof(buf);

#ifdef USE_SIOCGLIFCONF
    
#ifdef SIOCGLIFNUM
    ifn.lifn_family = AF_UNSPEC;
    ifn.lifn_flags = 0;
    if (ioctl (fd, SIOCGLIFNUM, (char *) &ifn) < 0)
        Error ("Getting interface count");    
    if (len < (ifn.lifn_count * sizeof(struct lifreq))) {
	len = ifn.lifn_count * sizeof(struct lifreq);
	bufptr = xalloc(len);
    }
#endif
    
    ifc.lifc_family = AF_UNSPEC;
    ifc.lifc_flags = 0;
    ifc.lifc_len = len;
    ifc.lifc_buf = bufptr;

#define IFC_IOCTL_REQ SIOCGLIFCONF
#define IFC_IFC_REQ ifc.lifc_req
#define IFC_IFC_LEN ifc.lifc_len
#define IFR_IFR_ADDR ifr->lifr_addr
#define IFR_IFR_NAME ifr->lifr_name

#else /* Use SIOCGIFCONF */
    ifc.ifc_len = len;
    ifc.ifc_buf = bufptr;

#define IFC_IOCTL_REQ SIOCGIFCONF
#ifdef ISC
#define IFC_IFC_REQ (struct ifreq *) ifc.ifc_buf
#else
#define IFC_IFC_REQ ifc.ifc_req
#endif /* ISC */
#define IFC_IFC_LEN ifc.ifc_len
#define IFR_IFR_ADDR ifr->ifr_addr
#define IFR_IFR_NAME ifr->ifr_name
#endif

    if (ifioctl (fd, IFC_IOCTL_REQ, (pointer) &ifc) < 0)
        Error ("Getting interface configuration (4)");

    cplim = (char *) IFC_IFC_REQ + IFC_IFC_LEN;
    
    for (cp = (char *) IFC_IFC_REQ; cp < cplim; cp += ifr_size (ifr))
    {
	ifr = (ifr_type *) cp;
	len = ifraddr_size (IFR_IFR_ADDR);
	family = ConvertAddr ((struct sockaddr *) &IFR_IFR_ADDR, 
	  			&len, (pointer *)&addr);
#ifdef DNETCONN
	/*
	 * DECnet was handled up above.
	 */
	if (family == AF_DECnet)
	    continue;
#endif /* DNETCONN */
        if (family == -1 || family == FamilyLocal)
	    continue;
#if defined(IPv6) && defined(AF_INET6)
	if (family == FamilyInternet6) 
	    in6_fillscopeid((struct sockaddr_in6 *)&IFR_IFR_ADDR);
#endif
#ifdef DEF_SELF_DEBUG
	if (family == FamilyInternet) 
	    ErrorF("Xserver: DefineSelf(): ifname = %s, addr = %d.%d.%d.%d\n",
		   IFR_IFR_NAME, addr[0], addr[1], addr[2], addr[3]);
#if defined(IPv6) && defined(AF_INET6)
	else if (family == FamilyInternet6) {
	    char cp[INET6_ADDRSTRLEN] = "";
	    inet_ntop(AF_INET6, addr, cp, sizeof(cp));
	    ErrorF("Xserver: DefineSelf(): ifname = %s, addr = %s\n",
		   IFR_IFR_NAME,  cp);
	}
#endif
#endif /* DEF_SELF_DEBUG */
        for (host = selfhosts;
 	     host && !addrEqual (family, addr, len, host);
	     host = host->next)
	    ;
        if (host)
	    continue;
	MakeHost(host,len)
	if (host)
	{
	    host->family = family;
	    host->len = len;
	    acopy(addr, host->addr, len);
	    host->next = selfhosts;
	    selfhosts = host;
	}
#ifdef XDMCP
	{
#ifdef USE_SIOCGLIFCONF
	    struct sockaddr_storage broad_addr;
#else
	    struct sockaddr broad_addr;
#endif

	    /*
	     * If this isn't an Internet Address, don't register it.
	     */
	    if (family != FamilyInternet
#if defined(IPv6) && defined(AF_INET6)
	      && family != FamilyInternet6
#endif
		)
		continue;

	    /*
 	     * ignore 'localhost' entries as they're not useful
	     * on the other end of the wire
	     */
	    if (family == FamilyInternet &&
		addr[0] == 127 && addr[1] == 0 &&
		addr[2] == 0 && addr[3] == 1)
		continue;
#if defined(IPv6) && defined(AF_INET6)
	    else if (family == FamilyInternet6 && 
	      IN6_IS_ADDR_LOOPBACK((struct in6_addr *)addr))
		continue;
#endif

	    /*
	     * Ignore '0.0.0.0' entries as they are
	     * returned by some OSes for unconfigured NICs but they are
	     * not useful on the other end of the wire.
	     */
	    if (len == 4 &&
		addr[0] == 0 && addr[1] == 0 &&
		addr[2] == 0 && addr[3] == 0)
		continue;

	    XdmcpRegisterConnection (family, (char *)addr, len);

#if defined(IPv6) && defined(AF_INET6)
	    /* IPv6 doesn't support broadcasting, so we drop out here */
	    if (family == FamilyInternet6)
		continue;
#endif

	    broad_addr = IFR_IFR_ADDR;

	    ((struct sockaddr_in *) &broad_addr)->sin_addr.s_addr =
		htonl (INADDR_BROADCAST);
#if defined(USE_SIOCGLIFCONF) && defined(SIOCGLIFBRDADDR)
	    {
	    	struct lifreq    broad_req;
    
	    	broad_req = *ifr;
		if (ioctl (fd, SIOCGLIFFLAGS, (char *) &broad_req) != -1 &&
		    (broad_req.lifr_flags & IFF_BROADCAST) &&
		    (broad_req.lifr_flags & IFF_UP)
		    )
		{
		    broad_req = *ifr;
		    if (ioctl (fd, SIOCGLIFBRDADDR, &broad_req) != -1)
			broad_addr = broad_req.lifr_broadaddr;
		    else
			continue;
		}
		else
		    continue;
	    }

#elif defined(SIOCGIFBRDADDR)
	    {
	    	struct ifreq    broad_req;
    
	    	broad_req = *ifr;
		if (ifioctl (fd, SIOCGIFFLAGS, (pointer) &broad_req) != -1 &&
		    (broad_req.ifr_flags & IFF_BROADCAST) &&
		    (broad_req.ifr_flags & IFF_UP)
		    )
		{
		    broad_req = *ifr;
		    if (ifioctl (fd, SIOCGIFBRDADDR, (pointer) &broad_req) != -1)
			broad_addr = broad_req.ifr_addr;
		    else
			continue;
		}
		else
		    continue;
	    }
#endif /* SIOCGIFBRDADDR */
#ifdef DEF_SELF_DEBUG
	    ErrorF("Xserver: DefineSelf(): ifname = %s, baddr = %s\n",
		   IFR_IFR_NAME,
	           inet_ntoa(((struct sockaddr_in *) &broad_addr)->sin_addr));
#endif /* DEF_SELF_DEBUG */
	    XdmcpRegisterBroadcastAddress ((struct sockaddr_in *) &broad_addr);
	}
#endif /* XDMCP */
    }
    if (bufptr != buf)
        free(bufptr);    
#else /* HAS_GETIFADDRS */
    if (getifaddrs(&ifap) < 0) {
	ErrorF("Warning: getifaddrs returns %s\n", strerror(errno));
	return;
    }
    for (ifr = ifap; ifr != NULL; ifr = ifr->ifa_next) {
#ifdef DNETCONN
	if (ifr->ifa_addr.sa_family == AF_DECnet) 
	    continue;
#endif /* DNETCONN */
	len = sizeof(*(ifr->ifa_addr));
	family = ConvertAddr(ifr->ifa_addr, &len, (pointer *)&addr);
	if (family == -1 || family == FamilyLocal) 
	    continue;
#if defined(IPv6) && defined(AF_INET6)
	if (family == FamilyInternet6) 
	    in6_fillscopeid((struct sockaddr_in6 *)ifr->ifa_addr);
#endif

#ifdef DEF_SELF_DEBUG
	if (family == FamilyInternet) 
	    ErrorF("Xserver: DefineSelf(): ifname = %s, addr = %d.%d.%d.%d\n",
		   ifr->ifa_name, addr[0], addr[1], addr[2], addr[3]);
#if defined(IPv6) && defined(AF_INET6)
	else if (family == FamilyInternet6) {
		char cp[INET6_ADDRSTRLEN];

		inet_ntop(AF_INET6, addr, cp, sizeof(cp));
		ErrorF("Xserver: DefineSelf(): ifname = %s addr = %s\n",
		    ifr->ifa_name, cp);
	}
#endif
#endif /* DEF_SELF_DEBUG */
	for (host = selfhosts; 
	     host != NULL && !addrEqual(family, addr, len, host);
	     host = host->next) 
	    ;
	if (host != NULL) 
	    continue;
	MakeHost(host, len);
	if (host != NULL) {
	    host->family = family;
	    host->len = len;
	    acopy(addr, host->addr, len);
	    host->next = selfhosts;
	    selfhosts = host;
	}
#ifdef XDMCP
	{
	    struct sockaddr broad_addr;
	    /*
	     * If this isn't an Internet Address, don't register it.
	     */
	    if (family != FamilyInternet
#if defined(IPv6) && defined(AF_INET6)
		&& family != FamilyInternet6
#endif
	    )
		continue;

	    /* 
	     * ignore 'localhost' entries as they're not useful
	     * on the other end of the wire
	     */
	    if (ifr->ifa_flags & IFF_LOOPBACK) 
		    continue;

	    if (family == FamilyInternet && 
		addr[0] == 127 && addr[1] == 0 &&
		addr[2] == 0 && addr[3] == 1) 
		continue;

	    /*
	     * Ignore '0.0.0.0' entries as they are
	     * returned by some OSes for unconfigured NICs but they are
	     * not useful on the other end of the wire.
	     */
	    if (len == 4 &&
		addr[0] == 0 && addr[1] == 0 &&
		addr[2] == 0 && addr[3] == 0)
		continue;
#if defined(IPv6) && defined(AF_INET6)
	    else if (family == FamilyInternet6 && 
	      IN6_IS_ADDR_LOOPBACK((struct in6_addr *)addr))
		continue;
#endif
	    XdmcpRegisterConnection(family, (char *)addr, len);
#if defined(IPv6) && defined(AF_INET6)
	    if (family == FamilyInternet6) 
		/* IPv6 doesn't support broadcasting, so we drop out here */
		continue;
#endif
	    if ((ifr->ifa_flags & IFF_BROADCAST) &&
		(ifr->ifa_flags & IFF_UP))
		broad_addr = *ifr->ifa_broadaddr;
	    else
		continue;
#ifdef DEF_SELF_DEBUG
	    ErrorF("Xserver: DefineSelf(): ifname = %s, baddr = %s\n",
		   ifr->ifa_name,
	           inet_ntoa(((struct sockaddr_in *) &broad_addr)->sin_addr));
#endif /* DEF_SELF_DEBUG */
	    XdmcpRegisterBroadcastAddress((struct sockaddr_in *)
					  &broad_addr);
	}
#endif /* XDMCP */
		
    } /* for */
    freeifaddrs(ifap);
#endif /* HAS_GETIFADDRS */

    /*
     * add something of FamilyLocalHost
     */
    for (host = selfhosts;
	 host && !addrEqual(FamilyLocalHost, "", 0, host);
	 host = host->next);
    if (!host)
    {
	MakeHost(host, 0);
	if (host)
	{
	    host->family = FamilyLocalHost;
	    host->len = 0;
	    acopy("", host->addr, 0);
	    host->next = selfhosts;
	    selfhosts = host;
	}
    }
}
#endif /* hpux && !HAS_IFREQ */
#endif /* WINTCP */

#ifdef XDMCP
void
AugmentSelf(pointer from, int len)
{
    int family;
    pointer addr;
    register HOST *host;

    family = ConvertAddr(from, &len, (pointer *)&addr);
    if (family == -1 || family == FamilyLocal)
	return;
    for (host = selfhosts; host; host = host->next)
    {
	if (addrEqual(family, addr, len, host))
	    return;
    }
    MakeHost(host,len)
    if (!host)
	return;
    host->family = family;
    host->len = len;
    acopy(addr, host->addr, len);
    host->next = selfhosts;
    selfhosts = host;
}
#endif

void
AddLocalHosts (void)
{
    HOST    *self;

    for (self = selfhosts; self; self = self->next)
	    /* Fix for XFree86 bug #156: pass addingLocal = TRUE to
	     * NewHost to tell that we are adding the default local
	     * host entries and not to flag the entries as being
	     * explicitely requested */
	(void) NewHost (self->family, self->addr, self->len, TRUE);
}

/* Reset access control list to initial hosts */
void
ResetHosts (char *display)
{
    register HOST	*host;
    char                lhostname[120], ohostname[120];
    char 		*hostname = ohostname;
    char		fname[PATH_MAX + 1];
    int			fnamelen;
    FILE		*fd;
    char		*ptr;
    int                 i, hostlen;
#if ((defined(TCPCONN) || defined(STREAMSCONN) || defined(MNX_TCPCONN)) && \
     (!defined(IPv6) || !defined(AF_INET6))) || defined(DNETCONN)
    union {
        struct sockaddr	sa;
#if defined(TCPCONN) || defined(STREAMSCONN) || defined(MNX_TCPCONN)
	struct sockaddr_in in;
#endif /* TCPCONN || STREAMSCONN */
#ifdef DNETCONN
	struct sockaddr_dn dn;
#endif
    }			saddr;
#endif
#ifdef DNETCONN
    struct nodeent 	*np;
    struct dn_naddr 	dnaddr, *dnaddrp, *dnet_addr();
#endif
#ifdef K5AUTH
    krb5_principal      princ;
    krb5_data		kbuf;
#endif
    int			family = 0;
    pointer		addr;
    int 		len;

    siTypesInitialize();
    AccessEnabled = defeatAccessControl ? FALSE : DEFAULT_ACCESS_CONTROL;
    LocalHostEnabled = FALSE;
    while ((host = validhosts) != 0)
    {
        validhosts = host->next;
        FreeHost (host);
    }

#define ETC_HOST_PREFIX "/etc/X"
#define ETC_HOST_SUFFIX ".hosts"
    fnamelen = strlen(ETC_HOST_PREFIX) + strlen(ETC_HOST_SUFFIX) +
		strlen(display) + 1;
    if (fnamelen > sizeof(fname))
	FatalError("Display name `%s' is too long\n", display);
    sprintf(fname, ETC_HOST_PREFIX "%s" ETC_HOST_SUFFIX, display);
#ifdef __UNIXOS2__
    strcpy(fname, (char*)__XOS2RedirRoot(fname));
#endif /* __UNIXOS2__ */

    if ((fd = fopen (fname, "r")) != 0)
    {
        while (fgets (ohostname, sizeof (ohostname), fd))
	{
	family = FamilyWild;
	if (*ohostname == '#')
	    continue;
    	if ((ptr = strchr(ohostname, '\n')) != 0)
    	    *ptr = 0;
#ifdef __UNIXOS2__
    	if ((ptr = strchr(ohostname, '\r')) != 0)
    	    *ptr = 0;
#endif
        hostlen = strlen(ohostname) + 1;
        for (i = 0; i < hostlen; i++)
	    lhostname[i] = tolower(ohostname[i]);
	hostname = ohostname;
	if (!strncmp("local:", lhostname, 6))
	{
	    family = FamilyLocalHost;
	    NewHost(family, "", 0, FALSE);
	    LocalHostRequested = TRUE;	/* Fix for XFree86 bug #156 */
	}
#if defined(TCPCONN) || defined(STREAMSCONN) || defined(MNX_TCPCONN)
	else if (!strncmp("inet:", lhostname, 5))
	{
	    family = FamilyInternet;
	    hostname = ohostname + 5;
	}
#if defined(IPv6) && defined(AF_INET6)
	else if (!strncmp("inet6:", lhostname, 6))
	{
	    family = FamilyInternet6;
	    hostname = ohostname + 6;
	}
#endif
#endif
#ifdef DNETCONN
	else if (!strncmp("dnet:", lhostname, 5))
	{
	    family = FamilyDECnet;
	    hostname = ohostname + 5;
	}
#endif
#ifdef SECURE_RPC
	else if (!strncmp("nis:", lhostname, 4))
	{
	    family = FamilyNetname;
	    hostname = ohostname + 4;
	}
#endif
#ifdef K5AUTH
	else if (!strncmp("krb:", lhostname, 4))
	{
	    family = FamilyKrb5Principal;
	    hostname = ohostname + 4;
	}
#endif
	else if (!strncmp("si:", lhostname, 3))
	{
	    family = FamilyServerInterpreted;
	    hostname = ohostname + 3;
	    hostlen -= 3;
	}


	if (family == FamilyServerInterpreted) 
	{
	    len = siCheckAddr(hostname, hostlen);
	    if (len >= 0) {
		NewHost(family, hostname, len, FALSE);
	    }
	}
	else
#ifdef DNETCONN
    	if ((family == FamilyDECnet) || ((family == FamilyWild) &&
	    (ptr = strchr(hostname, ':')) && (*(ptr + 1) == ':') &&
	    !(*ptr = '\0')))	/* bash trailing colons if necessary */
	{
    	    /* node name (DECnet names end in "::") */
	    dnaddrp = dnet_addr(hostname);
    	    if (!dnaddrp && (np = getnodebyname (hostname)))
	    {
		/* node was specified by name */
		saddr.sa.sa_family = np->n_addrtype;
		len = sizeof(saddr.sa);
		if (ConvertAddr (&saddr.sa, &len, (pointer *)&addr) == FamilyDECnet)
		{
		    bzero ((char *) &dnaddr, sizeof (dnaddr));
		    dnaddr.a_len = np->n_length;
		    acopy (np->n_addr, dnaddr.a_addr, np->n_length);
		    dnaddrp = &dnaddr;
		}
    	    }
	    if (dnaddrp)
		(void) NewHost(FamilyDECnet, (pointer)dnaddrp,
			(int)(dnaddrp->a_len + sizeof(dnaddrp->a_len)), FALSE);
    	}
	else
#endif /* DNETCONN */
#ifdef K5AUTH
	if (family == FamilyKrb5Principal)
	{
            krb5_parse_name(hostname, &princ);
	    XauKrb5Encode(princ, &kbuf);
	    (void) NewHost(FamilyKrb5Principal, kbuf.data, kbuf.length, FALSE);
	    krb5_free_principal(princ);
        }
	else
#endif
#ifdef SECURE_RPC
	if ((family == FamilyNetname) || (strchr(hostname, '@')))
	{
	    SecureRPCInit ();
	    (void) NewHost (FamilyNetname, hostname, strlen (hostname), FALSE);
	}
	else
#endif /* SECURE_RPC */
#if defined(TCPCONN) || defined(STREAMSCONN) || defined(MNX_TCPCONN)
	{
#if defined(IPv6) && defined(AF_INET6)
	    if ( (family == FamilyInternet) || (family == FamilyInternet6) ||
		 (family == FamilyWild) ) 
            {
		struct addrinfo *addresses;
		struct addrinfo *a;
		int f;
	    
		if (getaddrinfo(hostname, NULL, NULL, &addresses) == 0) {
		    for (a = addresses ; a != NULL ; a = a->ai_next) {
			len = a->ai_addrlen;
			f = ConvertAddr(a->ai_addr,&len,(pointer *)&addr);
			if ( (family == f) || 
			     ((family == FamilyWild) && (f != -1)) ) {
			    NewHost(f, addr, len, FALSE);
			}			
		    }
		    freeaddrinfo(addresses);
		}
	    }
#else
#ifdef XTHREADS_NEEDS_BYNAMEPARAMS
	    _Xgethostbynameparams hparams;
#endif
	    register struct hostent *hp;

    	    /* host name */
    	    if ((family == FamilyInternet &&
		 ((hp = _XGethostbyname(hostname, hparams)) != 0)) ||
		((hp = _XGethostbyname(hostname, hparams)) != 0))
	    {
    		saddr.sa.sa_family = hp->h_addrtype;
		len = sizeof(saddr.sa);
    		if ((family = ConvertAddr (&saddr.sa, &len, (pointer *)&addr)) != -1)
		{
#ifdef h_addr				/* new 4.3bsd version of gethostent */
		    char **list;

		    /* iterate over the addresses */
		    for (list = hp->h_addr_list; *list; list++)
			(void) NewHost (family, (pointer)*list, len, FALSE);
#else
    		    (void) NewHost (family, (pointer)hp->h_addr, len, FALSE);
#endif
		}
    	    }
#endif /* IPv6 */
        }
#endif /* TCPCONN || STREAMSCONN */
	family = FamilyWild;
        }
        fclose (fd);
    }
}

/* Is client on the local host */
Bool LocalClient(ClientPtr client)
{
    int    		alen, family, notused;
    Xtransaddr		*from = NULL;
    pointer		addr;
    register HOST	*host;

#ifdef XCSECURITY
    /* untrusted clients can't change host access */
    if (client->trustLevel != XSecurityClientTrusted)
    {
	SecurityAudit("client %d attempted to change host access\n",
		      client->index);
	return FALSE;
    }
#endif
#ifdef LBX
    if (!((OsCommPtr)client->osPrivate)->trans_conn)
	return FALSE;
#endif
    if (!_XSERVTransGetPeerAddr (((OsCommPtr)client->osPrivate)->trans_conn,
	&notused, &alen, &from))
    {
	family = ConvertAddr ((struct sockaddr *) from,
	    &alen, (pointer *)&addr);
	if (family == -1)
	{
	    xfree ((char *) from);
	    return FALSE;
	}
	if (family == FamilyLocal)
	{
	    xfree ((char *) from);
	    return TRUE;
	}
	for (host = selfhosts; host; host = host->next)
	{
	    if (addrEqual (family, addr, alen, host))
		return TRUE;
	}
	xfree ((char *) from);
    }
    return FALSE;
}

/*
 * Return the uid and gid of a connected local client
 * or the uid/gid for nobody those ids cannot be determined
 * 
 * Used by XShm to test access rights to shared memory segments
 */
int
LocalClientCred(ClientPtr client, int *pUid, int *pGid)
{
    return LocalClientCredAndGroups(client, pUid, pGid, NULL, NULL);
}

/*
 * Return the uid and all gids of a connected local client
 * or the uid/gid for nobody those ids cannot be determined
 * 
 * If the caller passes non-NULL values for pSuppGids & nSuppGids,
 * they are responsible for calling XFree(*pSuppGids) to release the
 * memory allocated for the supplemental group ids list.
 *
 * Used by localuser & localgroup ServerInterpreted access control forms below
 */
int
LocalClientCredAndGroups(ClientPtr client, int *pUid, int *pGid, 
			 int **pSuppGids, int *nSuppGids)
{
#if defined(HAS_GETPEEREID) || defined(HAS_GETPEERUCRED) || defined(SO_PEERCRED)
    int fd;
    XtransConnInfo ci;
#ifdef HAS_GETPEEREID
    uid_t uid;
    gid_t gid;
#elif defined(HAS_GETPEERUCRED)
    ucred_t *peercred = NULL;
#elif defined(SO_PEERCRED)
    struct ucred peercred;
    socklen_t so_len = sizeof(peercred);
#endif

    if (client == NULL)
	return -1;
    ci = ((OsCommPtr)client->osPrivate)->trans_conn;
#if !(defined(sun) && defined(HAS_GETPEERUCRED))
    /* Most implementations can only determine peer credentials for Unix 
     * domain sockets - Solaris getpeerucred can work with a bit more, so 
     * we just let it tell us if the connection type is supported or not
     */
    if (!_XSERVTransIsLocal(ci)) {
	return -1;
    }
#endif

    if (pSuppGids != NULL)
	*pSuppGids = NULL;
    if (nSuppGids != NULL)
	*nSuppGids = 0;

    fd = _XSERVTransGetConnectionNumber(ci);
#ifdef HAS_GETPEEREID
    if (getpeereid(fd, &uid, &gid) == -1) 
	    return -1;
    if (pUid != NULL)
	    *pUid = uid;
    if (pGid != NULL)
	    *pGid = gid;
    return 0;
#elif defined(HAS_GETPEERUCRED)
    if (getpeerucred(fd, &peercred) < 0)
    	return -1;
#ifdef sun /* Ensure process is in the same zone */
    if (getzoneid() != ucred_getzoneid(peercred)) {
	ucred_free(peercred);
	return -1;
    }
#endif
    if (pUid != NULL)
	*pUid = ucred_geteuid(peercred);
    if (pGid != NULL)
	*pGid = ucred_getegid(peercred);
    if (pSuppGids != NULL && nSuppGids != NULL) {
	const gid_t *gids;
	*nSuppGids = ucred_getgroups(peercred, &gids);
	if (*nSuppGids > 0) {
	    *pSuppGids = xalloc(sizeof(int) * (*nSuppGids));
	    if (*pSuppGids == NULL) {
		*nSuppGids = 0;
	    } else {
		int i;
		for (i = 0 ; i < *nSuppGids; i++) {
		    (*pSuppGids)[i] = (int) gids[i];
		}
	    }
	}
    }
    ucred_free(peercred);
    return 0;
#elif defined(SO_PEERCRED)
    if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &peercred, &so_len) == -1) 
	    return -1;
    if (pUid != NULL)
	    *pUid = peercred.uid;
    if (pGid != NULL)
	    *pGid = peercred.gid;
    return 0;
#endif
#else
    /* No system call available to get the credentials of the peer */
#define NO_LOCAL_CLIENT_CRED
    return -1;
#endif
}

static Bool
AuthorizedClient(ClientPtr client)
{
    if (!client || defeatAccessControl)
	return TRUE;
    return LocalClient(client);
}

/* Add a host to the access control list.  This is the external interface
 * called from the dispatcher */

int
AddHost (ClientPtr	client,
	 int            family,
	 unsigned       length,        /* of bytes in pAddr */
	 pointer        pAddr)
{
    int			len;

    if (!AuthorizedClient(client))
	return(BadAccess);
    switch (family) {
    case FamilyLocalHost:
	len = length;
	LocalHostEnabled = TRUE;
	break;
#ifdef K5AUTH
    case FamilyKrb5Principal:
        len = length;
        break;
#endif
#ifdef SECURE_RPC
    case FamilyNetname:
	len = length;
	SecureRPCInit ();
	break;
#endif
    case FamilyInternet:
#if defined(IPv6) && defined(AF_INET6)
    case FamilyInternet6:
#endif
    case FamilyDECnet:
    case FamilyChaos:
    case FamilyServerInterpreted:
	if ((len = CheckAddr (family, pAddr, length)) < 0)
	{
	    client->errorValue = length;
	    return (BadValue);
	}
	break;
    case FamilyLocal:
    default:
	client->errorValue = family;
	return (BadValue);
    }
    if (NewHost (family, pAddr, len, FALSE))
	return Success;
    return BadAlloc;
}

Bool
ForEachHostInFamily (int	    family,
		     Bool    (*func)(
			 unsigned char * /* addr */,
			 short           /* len */,
			 pointer         /* closure */),
		     pointer closure)
{
    HOST    *host;

    for (host = validhosts; host; host = host->next)
	if (family == host->family && func (host->addr, host->len, closure))
	    return TRUE;
    return FALSE;
}

/* Add a host to the access control list. This is the internal interface 
 * called when starting or resetting the server */
static Bool
NewHost (int		family,
	 pointer	addr,
	 int		len,
	 int		addingLocalHosts)
{
    register HOST *host;

    for (host = validhosts; host; host = host->next)
    {
        if (addrEqual (family, addr, len, host))
	    return TRUE;
    }
    if (!addingLocalHosts) {			/* Fix for XFree86 bug #156 */
	for (host = selfhosts; host; host = host->next) {
	    if (addrEqual (family, addr, len, host)) {
		host->requested = TRUE;
		break;
	    }	    
	}
    }
    MakeHost(host,len)
    if (!host)
	return FALSE;
    host->family = family;
    host->len = len;
    acopy(addr, host->addr, len);
    host->next = validhosts;
    validhosts = host;
    return TRUE;
}

/* Remove a host from the access control list */

int
RemoveHost (
    ClientPtr		client,
    int                 family,
    unsigned            length,        /* of bytes in pAddr */
    pointer             pAddr)
{
    int			len;
    register HOST	*host, **prev;

    if (!AuthorizedClient(client))
	return(BadAccess);
    switch (family) {
    case FamilyLocalHost:
	len = length;
	LocalHostEnabled = FALSE;
	break;
#ifdef K5AUTH
    case FamilyKrb5Principal:
        len = length;
	break;
#endif
#ifdef SECURE_RPC
    case FamilyNetname:
	len = length;
	break;
#endif
    case FamilyInternet:
#if defined(IPv6) && defined(AF_INET6)
    case FamilyInternet6:
#endif
    case FamilyDECnet:
    case FamilyChaos:
    case FamilyServerInterpreted:
    	if ((len = CheckAddr (family, pAddr, length)) < 0)
    	{
	    client->errorValue = length;
            return(BadValue);
    	}
	break;
    case FamilyLocal:
    default:
	client->errorValue = family;
        return(BadValue);
    }
    for (prev = &validhosts;
         (host = *prev) && (!addrEqual (family, pAddr, len, host));
         prev = &host->next)
        ;
    if (host)
    {
        *prev = host->next;
        FreeHost (host);
    }
    return (Success);
}

/* Get all hosts in the access control list */
int
GetHosts (
    pointer		*data,
    int			*pnHosts,
    int			*pLen,
    BOOL		*pEnabled)
{
    int			len;
    register int 	n = 0;
    register unsigned char *ptr;
    register HOST	*host;
    int			nHosts = 0;

    *pEnabled = AccessEnabled ? EnableAccess : DisableAccess;
    for (host = validhosts; host; host = host->next)
    {
	nHosts++;
	n += (((host->len + 3) >> 2) << 2) + sizeof(xHostEntry);
    }
    if (n)
    {
        *data = ptr = (pointer) xalloc (n);
	if (!ptr)
	{
	    return(BadAlloc);
	}
        for (host = validhosts; host; host = host->next)
	{
	    len = host->len;
	    ((xHostEntry *)ptr)->family = host->family;
	    ((xHostEntry *)ptr)->length = len;
	    ptr += sizeof(xHostEntry);
	    acopy (host->addr, ptr, len);
	    ptr += ((len + 3) >> 2) << 2;
        }
    } else {
	*data = NULL;
    }
    *pnHosts = nHosts;
    *pLen = n;
    return(Success);
}

/* Check for valid address family and length, and return address length. */

/*ARGSUSED*/
static int
CheckAddr (
    int			family,
    pointer		pAddr,
    unsigned		length)
{
    int	len;

    switch (family)
    {
#if defined(TCPCONN) || defined(STREAMSCONN) || defined(MNX_TCPCONN)
      case FamilyInternet:
	if (length == sizeof (struct in_addr))
	    len = length;
	else
	    len = -1;
        break;
#if defined(IPv6) && defined(AF_INET6)
      case FamilyInternet6:
	if (length == sizeof (struct in6_addr))
	    len = length;
	else
	    len = -1;
        break;
#endif
#endif 
#ifdef DNETCONN
      case FamilyDECnet:
        {
	    struct dn_naddr *dnaddr = (struct dn_naddr *) pAddr;

	    if ((length < sizeof(dnaddr->a_len)) ||
		(length < dnaddr->a_len + sizeof(dnaddr->a_len)))
		len = -1;
	    else
		len = dnaddr->a_len + sizeof(dnaddr->a_len);
	    if (len > sizeof(struct dn_naddr))
		len = -1;
	}
        break;
#endif
      case FamilyServerInterpreted:
	len = siCheckAddr(pAddr, length);
	break;
      default:
        len = -1;
    }
    return (len);
}

/* Check if a host is not in the access control list. 
 * Returns 1 if host is invalid, 0 if we've found it. */

int
InvalidHost (
    register struct sockaddr	*saddr,
    int				len,
    ClientPtr			client)
{
    int 			family;
    pointer			addr;
    register HOST 		*selfhost, *host;

    if (!AccessEnabled)   /* just let them in */
        return(0);    
    family = ConvertAddr (saddr, &len, (pointer *)&addr);
    if (family == -1)
        return 1;
    if (family == FamilyLocal)
    {
	if (!LocalHostEnabled)
 	{
	    /*
	     * check to see if any local address is enabled.  This 
	     * implicitly enables local connections.
	     */
	    for (selfhost = selfhosts; selfhost; selfhost=selfhost->next)
 	    {
		for (host = validhosts; host; host=host->next)
		{
		    if (addrEqual (selfhost->family, selfhost->addr,
				   selfhost->len, host))
			return 0;
		}
	    }
	} else
	    return 0;
    }
    for (host = validhosts; host; host = host->next)
    {
	if ((host->family == FamilyServerInterpreted)) {
	    if (siAddrMatch (family, addr, len, host, client)) {
		return (0);
	    }
	} else {
	    if (addrEqual (family, addr, len, host))
		return (0);
	}

    }
    return (1);
}

static int
ConvertAddr (
    register struct sockaddr	*saddr,
    int				*len,
    pointer			*addr)
{
    if (*len == 0)
        return (FamilyLocal);
    switch (saddr->sa_family)
    {
    case AF_UNSPEC:
#if defined(UNIXCONN) || defined(LOCALCONN) || defined(OS2PIPECONN)
    case AF_UNIX:
#endif
        return FamilyLocal;
#if defined(TCPCONN) || defined(STREAMSCONN) || defined(MNX_TCPCONN)
    case AF_INET:
#ifdef WIN32
        if (16777343 == *(long*)&((struct sockaddr_in *) saddr)->sin_addr)
            return FamilyLocal;
#endif
        *len = sizeof (struct in_addr);
        *addr = (pointer) &(((struct sockaddr_in *) saddr)->sin_addr);
        return FamilyInternet;
#if defined(IPv6) && defined(AF_INET6)
    case AF_INET6: 
    {
	struct sockaddr_in6 *saddr6 = (struct sockaddr_in6 *) saddr;
	if (IN6_IS_ADDR_V4MAPPED(&(saddr6->sin6_addr))) {
	    *len = sizeof (struct in_addr);
	    *addr = (pointer) &(saddr6->sin6_addr.s6_addr[12]);
	    return FamilyInternet;
	} else {
	    *len = sizeof (struct in6_addr);
	    *addr = (pointer) &(saddr6->sin6_addr);
	    return FamilyInternet6;
	}
    }
#endif
#endif
#ifdef DNETCONN
    case AF_DECnet:
	{
	    struct sockaddr_dn *sdn = (struct sockaddr_dn *) saddr;
	    *len = sdn->sdn_nodeaddrl + sizeof(sdn->sdn_nodeaddrl);
	    *addr = (pointer) &(sdn->sdn_add);
	}
        return FamilyDECnet;
#endif
#ifdef CHAOSCONN
    case AF_CHAOS:
	{
	    not implemented
	}
	return FamilyChaos;
#endif
    default:
        return -1;
    }
}

int
ChangeAccessControl(
    ClientPtr client,
    int fEnabled)
{
    if (!AuthorizedClient(client))
	return BadAccess;
    AccessEnabled = fEnabled;
    return Success;
}

/* returns FALSE if xhost + in effect, else TRUE */
int
GetAccessControl(void)
{
    return AccessEnabled;
}

/*****************************************************************************
 * FamilyServerInterpreted host entry implementation
 *
 * Supports an extensible system of host types which the server can interpret
 * See the IPv6 extensions to the X11 protocol spec for the definition.
 *
 * Currently supported schemes:
 *
 * hostname	- hostname as defined in IETF RFC 2396
 * ipv6		- IPv6 literal address as defined in IETF RFC's 3513 and <TBD>
 *
 * See xc/doc/specs/SIAddresses for formal definitions of each type.
 */

/* These definitions and the siTypeAdd function could be exported in the 
 * future to enable loading additional host types, but that was not done for
 * the initial implementation.
 */
typedef Bool (*siAddrMatchFunc)(int family, pointer addr, int len, 
  const char *siAddr, int siAddrlen, ClientPtr client, void *siTypePriv);
typedef int  (*siCheckAddrFunc)(const char *addrString, int length, 
  void *siTypePriv);

struct siType {
    struct siType *	next;
    const char *	typeName;
    siAddrMatchFunc	addrMatch;
    siCheckAddrFunc	checkAddr;
    void *		typePriv;	/* Private data for type routines */
};

static struct siType *siTypeList;

static int
siTypeAdd(const char *typeName, siAddrMatchFunc addrMatch,
  siCheckAddrFunc checkAddr, void *typePriv)
{
    struct siType *s, *p;

    if ((typeName == NULL) || (addrMatch == NULL) || (checkAddr == NULL))
	return BadValue;

    for (s = siTypeList, p = NULL; s != NULL ; p = s, s = s->next) {
	if (strcmp(typeName, s->typeName) == 0) {
	    s->addrMatch = addrMatch;
	    s->checkAddr = checkAddr;
	    s->typePriv = typePriv;
	    return Success;
	}
    }

    s = (struct siType *) xalloc(sizeof(struct siType));
    if (s == NULL)
	return BadAlloc;

    if (p == NULL)
	siTypeList = s;
    else
	p->next = s;

    s->next = NULL;
    s->typeName = typeName;
    s->addrMatch = addrMatch;
    s->checkAddr = checkAddr;
    s->typePriv = typePriv;
    return Success;
}

/* Checks to see if a host matches a server-interpreted host entry */
static Bool 
siAddrMatch(int family, pointer addr, int len, HOST *host, ClientPtr client)
{
    Bool matches = FALSE;
    struct siType *s;
    const char *valueString;
    int addrlen;

    valueString = (const char *) memchr(host->addr, '\0', host->len);
    if (valueString != NULL) {
	for (s = siTypeList; s != NULL ; s = s->next) {
	    if (strcmp((char *) host->addr, s->typeName) == 0) {
		addrlen = host->len - (strlen((char *)host->addr) + 1);
		matches = s->addrMatch(family, addr, len, 
		  valueString + 1, addrlen, client, s->typePriv);
		break;
	    }
	}
#ifdef FAMILY_SI_DEBUG
	ErrorF(
	    "Xserver: siAddrMatch(): type = %s, value = %*.*s -- %s\n",
	      host->addr, addrlen, addrlen, valueString + 1,
	      (matches) ? "accepted" : "rejected");
#endif
    }
    return matches;
}

static int
siCheckAddr(const char *addrString, int length)
{
    const char *valueString;
    int addrlen, typelen;
    int len = -1;
    struct siType *s;

    /* Make sure there is a \0 byte inside the specified length
       to separate the address type from the address value. */
    valueString = (const char *) memchr(addrString, '\0', length);
    if (valueString != NULL) {
	/* Make sure the first string is a recognized address type,
	 * and the second string is a valid address of that type. 
	 */
	typelen = strlen(addrString) + 1;
	addrlen = length - typelen;

	for (s = siTypeList; s != NULL ; s = s->next) {
	    if (strcmp(addrString, s->typeName) == 0) {
		len = s->checkAddr(valueString + 1, addrlen, s->typePriv);
		if (len >= 0) {
		    len += typelen;
		}
		break;
	    }
	}
#ifdef FAMILY_SI_DEBUG
	{
	    const char *resultMsg;

	    if (s == NULL) {
		resultMsg = "type not registered";
	    } else {
		if (len == -1) 
		    resultMsg = "rejected";
		else
		    resultMsg = "accepted";
	    }

	    ErrorF("Xserver: siCheckAddr(): type = %s, value = %*.*s, len = %d -- %s\n",
	      addrString, addrlen, addrlen, valueString + 1, len, resultMsg);
	}
#endif
    }
    return len;
}


/***
 * Hostname server-interpreted host type
 *
 * Stored as hostname string, explicitly defined to be resolved ONLY
 * at access check time, to allow for hosts with dynamic addresses
 * but static hostnames, such as found in some DHCP & mobile setups.
 *
 * Hostname must conform to IETF RFC 2396 sec. 3.2.2, which defines it as:
 * 	hostname     = *( domainlabel "." ) toplabel [ "." ]
 *	domainlabel  = alphanum | alphanum *( alphanum | "-" ) alphanum
 *	toplabel     = alpha | alpha *( alphanum | "-" ) alphanum
 */

#ifdef NI_MAXHOST
# define SI_HOSTNAME_MAXLEN NI_MAXHOST
#else
# ifdef MAXHOSTNAMELEN
#  define SI_HOSTNAME_MAXLEN MAXHOSTNAMELEN
# else
#  define SI_HOSTNAME_MAXLEN 256
# endif
#endif

static Bool 
siHostnameAddrMatch(int family, pointer addr, int len,
  const char *siAddr, int siAddrLen, ClientPtr client, void *typePriv)
{
    Bool res = FALSE;

/* Currently only supports checking against IPv4 & IPv6 connections, but 
 * support for other address families, such as DECnet, could be added if 
 * desired.
 */
#if defined(IPv6) && defined(AF_INET6)
    if ((family == FamilyInternet) || (family == FamilyInternet6)) {
	char hostname[SI_HOSTNAME_MAXLEN];
	struct addrinfo *addresses;
	struct addrinfo *a;
	int f, hostaddrlen;
	pointer hostaddr;

	if (siAddrLen >= sizeof(hostname)) 
	    return FALSE;

	strncpy(hostname, siAddr, siAddrLen);
	hostname[siAddrLen] = '\0';

	if (getaddrinfo(hostname, NULL, NULL, &addresses) == 0) {
	    for (a = addresses ; a != NULL ; a = a->ai_next) {
		hostaddrlen = a->ai_addrlen;
		f = ConvertAddr(a->ai_addr,&hostaddrlen,&hostaddr);
		if ((f == family) && (len == hostaddrlen) &&
		  (acmp (addr, hostaddr, len) == 0) ) {
		    res = TRUE;
		    break;
		}
	    }
	    freeaddrinfo(addresses);
	}
    }
#else /* IPv6 not supported, use gethostbyname instead for IPv4 */
    if (family == FamilyInternet) {
	register struct hostent *hp;
#ifdef XTHREADS_NEEDS_BYNAMEPARAMS
	_Xgethostbynameparams hparams;
#endif
	char hostname[SI_HOSTNAME_MAXLEN];
	int f, hostaddrlen;
	pointer hostaddr;
	const char **addrlist;

	if (siAddrLen >= sizeof(hostname)) 
	    return FALSE;

	strncpy(hostname, siAddr, siAddrLen);
	hostname[siAddrLen] = '\0';

	if ((hp = _XGethostbyname(hostname, hparams)) != NULL) {
#ifdef h_addr				/* new 4.3bsd version of gethostent */
	    /* iterate over the addresses */
	    for (addrlist = hp->h_addr_list; *addrlist; addrlist++)
#else
	    addrlist = &hp->h_addr;
#endif
	    {
		struct  sockaddr_in  sin;

    		sin.sin_family = hp->h_addrtype;
		acopy ( *addrlist, &(sin.sin_addr), hp->h_length);
		hostaddrlen = sizeof(sin);
    		f = ConvertAddr ((struct sockaddr *)&sin, 
		  &hostaddrlen, &hostaddr);
		if ((f == family) && (len == hostaddrlen) &&
		  (acmp (addr, hostaddr, len) == 0) ) {
		    res = TRUE;
		    break;
		}
    	    }
        }
    }
#endif
    return res;
}


static int
siHostnameCheckAddr(const char *valueString, int length, void *typePriv)
{
    /* Check conformance of hostname to RFC 2396 sec. 3.2.2 definition.
     * We do not use ctype functions here to avoid locale-specific
     * character sets.  Hostnames must be pure ASCII.  
     */
    int len = length;
    int i;
    Bool dotAllowed = FALSE;
    Bool dashAllowed = FALSE;
		    
    if ((length <= 0) || (length >= SI_HOSTNAME_MAXLEN)) {
	len = -1;
    } else {
	for (i = 0; i < length; i++) {
	    char c = valueString[i];

	    if (c == 0x2E) { /* '.' */
		if (dotAllowed == FALSE) {
		    len = -1;
		    break;
		} else {
		    dotAllowed = FALSE;
		    dashAllowed = FALSE;
		}
	    } else if (c == 0x2D) { /* '-' */
		if (dashAllowed == FALSE) {
		    len = -1;
		    break;
		} else {
		    dotAllowed = FALSE;
		}
	    } else if (((c >= 0x30) && (c <= 0x3A)) /* 0-9 */ ||
		       ((c >= 0x61) && (c <= 0x7A)) /* a-z */ ||
		       ((c >= 0x41) && (c <= 0x5A)) /* A-Z */) {
		dotAllowed = TRUE;
		dashAllowed = TRUE;
	    } else { /* Invalid character */
		len = -1;
		break;
	    }
	}
    }
    return len;
}

#if defined(IPv6) && defined(AF_INET6)
/***
 * "ipv6" server interpreted type
 *
 * Currently supports only IPv6 literal address as specified in IETF RFC 3513
 *
 * Once draft-ietf-ipv6-scoping-arch-00.txt becomes an RFC, support will be 
 * added for the scoped address format it specifies.
 */

/* Maximum length of an IPv6 address string - increase when adding support 
 * for scoped address qualifiers.  Includes room for trailing NUL byte. 
 */
#define SI_IPv6_MAXLEN INET6_ADDRSTRLEN

static Bool 
siIPv6AddrMatch(int family, pointer addr, int len,
  const char *siAddr, int siAddrlen, ClientPtr client, void *typePriv)
{
    struct in6_addr addr6;
    char addrbuf[SI_IPv6_MAXLEN];

    if ((family != FamilyInternet6) || (len != sizeof(addr6)))
	return FALSE;

    memcpy(addrbuf, siAddr, siAddrlen);
    addrbuf[siAddrlen] = '\0';

    if (inet_pton(AF_INET6, addrbuf, &addr6) != 1) {
	perror("inet_pton");
	return FALSE;
    }

    if (memcmp(addr, &addr6, len) == 0) {
	return TRUE;
    } else {
	return FALSE;
    }
}

static int
siIPv6CheckAddr(const char *addrString, int length, void *typePriv)
{
    int len;

    /* Minimum length is 3 (smallest legal address is "::1") */
    if (length < 3) {
	/* Address is too short! */
	len = -1;
    } else if (length >= SI_IPv6_MAXLEN) {
	/* Address is too long! */
	len = -1;
    } else {
	/* Assume inet_pton is sufficient validation */
	struct in6_addr addr6;
	char addrbuf[SI_IPv6_MAXLEN];

	memcpy(addrbuf, addrString, length);
	addrbuf[length] = '\0';

	if (inet_pton(AF_INET6, addrbuf, &addr6) != 1) {
	    perror("inet_pton");
	    len = -1;
	} else {
	    len = length;
	}
    }
    return len;
}
#endif /* IPv6 */

#if !defined(NO_LOCAL_CLIENT_CRED)
/***
 * "localuser" & "localgroup" server interpreted types
 *
 * Allows local connections from a given local user or group
 */

#include <pwd.h>
#include <grp.h>

#define LOCAL_USER 1
#define LOCAL_GROUP 2

typedef struct {
    int credType;
} siLocalCredPrivRec, *siLocalCredPrivPtr;

static siLocalCredPrivRec siLocalUserPriv = { LOCAL_USER };
static siLocalCredPrivRec siLocalGroupPriv = { LOCAL_GROUP };

static Bool
siLocalCredGetId(const char *addr, int len, siLocalCredPrivPtr lcPriv, int *id)
{
    Bool parsedOK = FALSE;
    char *addrbuf = xalloc(len + 1);

    if (addrbuf == NULL) {
	return FALSE;
    }

    memcpy(addrbuf, addr, len);
    addrbuf[len] = '\0';

    if (addr[0] == '#') { /* numeric id */
	char *cp;
	errno = 0;
	*id = strtol(addrbuf + 1, &cp, 0);
	if ((errno == 0) && (cp != (addrbuf+1))) {
	    parsedOK = TRUE;
	}
    } else { /* non-numeric name */
	if (lcPriv->credType == LOCAL_USER) {
	    struct passwd *pw = getpwnam(addrbuf);

	    if (pw != NULL) {
		*id = (int) pw->pw_uid;
		parsedOK = TRUE;
	    }
	} else { /* group */
	    struct group *gr = getgrnam(addrbuf);

	    if (gr != NULL) {
		*id = (int) gr->gr_gid;
		parsedOK = TRUE;
	    }
	}
    }

    xfree(addrbuf);
    return parsedOK;
}

static Bool 
siLocalCredAddrMatch(int family, pointer addr, int len,
  const char *siAddr, int siAddrlen, ClientPtr client, void *typePriv)
{
    int connUid, connGid, *connSuppGids, connNumSuppGids, siAddrId;
    siLocalCredPrivPtr lcPriv = (siLocalCredPrivPtr) typePriv;

    if (LocalClientCredAndGroups(client, &connUid, &connGid,
      &connSuppGids, &connNumSuppGids) == -1) {
	return FALSE;
    }

    if (siLocalCredGetId(siAddr, siAddrlen, lcPriv, &siAddrId) == FALSE) {
	return FALSE;
    }

    if (lcPriv->credType == LOCAL_USER) {
	if (connUid == siAddrId) {
	    return TRUE;
	}
    } else {
	if (connGid == siAddrId) {
	    return TRUE;
	}
	if (connSuppGids != NULL) {
	    int i;

	    for (i = 0 ; i < connNumSuppGids; i++) {
		if (connSuppGids[i] == siAddrId) {
		    xfree(connSuppGids);
		    return TRUE;
		}
	    }
	    xfree(connSuppGids);
	}
    }
    return FALSE;
}

static int
siLocalCredCheckAddr(const char *addrString, int length, void *typePriv)
{
    int len = length;
    int id;

    if (siLocalCredGetId(addrString, length, 
	(siLocalCredPrivPtr)typePriv, &id) == FALSE) {
	len = -1;
    }
    return len;
}
#endif /* localuser */

static void
siTypesInitialize(void)
{
    siTypeAdd("hostname", siHostnameAddrMatch, siHostnameCheckAddr, NULL);
#if defined(IPv6) && defined(AF_INET6)
    siTypeAdd("ipv6", siIPv6AddrMatch, siIPv6CheckAddr, NULL);
#endif
#if !defined(NO_LOCAL_CLIENT_CRED)
    siTypeAdd("localuser", siLocalCredAddrMatch, siLocalCredCheckAddr, 
      &siLocalUserPriv);
    siTypeAdd("localgroup", siLocalCredAddrMatch, siLocalCredCheckAddr, 
      &siLocalGroupPriv);
#endif
}
