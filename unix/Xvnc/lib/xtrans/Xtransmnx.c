/* $XFree86: xc/lib/xtrans/Xtransmnx.c,v 3.3 1996/05/10 06:55:50 dawes Exp $ */

/*
Xtransmnx.c

Created:	11 April 1994 by Philip Homburg <philip@cs.vu.nl>
*/


#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/nbio.h>
#include <net/hton.h>
#include <net/netlib.h>
#include <net/gen/in.h>
#include <net/gen/netdb.h>
#include <net/gen/tcp.h>
#include <net/gen/tcp_io.h>

struct private
{
	int nonblocking;

	int read_inprogress;
	char *read_buffer;
	size_t read_bufsize;
	size_t read_size;
	size_t read_offset;

	int write_inprogress;
	char *write_buffer;
	size_t write_bufsize;
	size_t write_size;
	size_t write_offset;
	int write_errno;

	int listen_completed;
	u16_t listen_port;
	XtransConnInfo listen_list;
};
#define RD_BUFSIZE	1024
#define WR_BUFSIZE	1024

static XtransConnInfo listen_list= NULL;

static XtransConnInfo alloc_ConnInfo(Xtransport *thistrans);
static void free_ConnInfo(XtransConnInfo ciptr);
static struct private *alloc_private(size_t rd_size, size_t wr_size);
static void free_private(struct private *priv);
static void read_cb(nbio_ref_t ref, int res, int err);
static void write_cb(nbio_ref_t ref, int res, int err);
static void listen_cb(nbio_ref_t ref, int res, int err);
static int restart_listen(XtransConnInfo ciptr);

#ifdef TRANS_CLIENT
static XtransConnInfo
TRANS(MnxTcpOpenCOTSClient) (thistrans, protocol, host, port)

Xtransport *thistrans;
char 	   *protocol;
char 	   *host;
char       *port;

{
	XtransConnInfo	ciptr;
	char *tcp_device;
	int s_errno;
	int fd;
	nbio_ref_t ref;


	PRMSG(2, "MnxTcpOpenCOTSClient(%s,%s,%s)\n",
		protocol, host, port);

	if ((ciptr= alloc_ConnInfo(thistrans)) == NULL)
	{
		PRMSG(1,
			"MnxTcpOpenCOTSClient: alloc_ConnInfo failed\n",
			0, 0, 0);
		return NULL;
	}
	if ((ciptr->priv= (char *)alloc_private(RD_BUFSIZE, WR_BUFSIZE)) ==
		NULL)
	{
		PRMSG(1,
			"MnxTcpOpenCOTSClient: alloc_private() failed\n",
			0, 0, 0);
		s_errno= errno;
		free_ConnInfo(ciptr);
		errno= s_errno;
		return NULL;
	}

	if ((tcp_device= getenv("TCP_DEVICE")) == NULL)
		tcp_device= TCP_DEVICE;
	PRMSG(4, "MnxTcpOpenCOTSClient: tcp_device= '%s'\n",
		tcp_device, 0, 0);

	if ((fd= open(tcp_device, O_RDWR)) == -1)
	{
		PRMSG(1,
			"MnxTcpOpenCOTSClient: open '%s' failed: %s\n",
			tcp_device, strerror(errno), 0);
		s_errno= errno;
		free_ConnInfo(ciptr);
		errno= s_errno;
		return NULL;
	}
	ciptr->fd= fd;
	ref.ref_ptr= ciptr;
	nbio_register(fd);
	nbio_setcallback(fd, ASIO_READ, read_cb, ref);
	nbio_setcallback(fd, ASIO_WRITE, write_cb, ref);
	return ciptr;
}
#endif /* TRANS_CLIENT */

#ifdef TRANS_SERVER
static XtransConnInfo
TRANS(MnxTcpOpenCOTSServer) (thistrans, protocol, host, port)

Xtransport *thistrans;
char 	   *protocol;
char 	   *host;
char 	   *port;

{
	XtransConnInfo	ciptr;
	char *tcp_device;
	int s_errno;
	int fd;
	nbio_ref_t ref;


	PRMSG(2, "MnxTcpOpenCOTSServer(%s,%s,%s)\n",
		protocol, host, port);

	if ((ciptr= alloc_ConnInfo(thistrans)) == NULL)
	{
		PRMSG(1,
			"MnxTcpOpenCOTSServer: alloc_ConnInfo failed\n",
			0, 0, 0);
		return NULL;
	}
	if ((ciptr->priv= (char *)alloc_private(RD_BUFSIZE, WR_BUFSIZE)) ==
		NULL)
	{
		PRMSG(1,
			"MnxTcpOpenCOTSServer: alloc_private() failed\n",
			0, 0, 0);
		s_errno= errno;
		free_ConnInfo(ciptr);
		errno= s_errno;
		return NULL;
	}

	if ((tcp_device= getenv("TCP_DEVICE")) == NULL)
		tcp_device= TCP_DEVICE;
	PRMSG(4, "MnxTcpOpenCOTSServer: tcp_device= '%s'\n",
		tcp_device, 0, 0);

	if ((fd= open(tcp_device, O_RDWR)) == -1)
	{
		PRMSG(1,
			"MnxTcpOpenCOTSServer: open '%s' failed: %s\n",
			tcp_device, strerror(errno), 0);
		s_errno= errno;
		free_ConnInfo(ciptr);
		errno= s_errno;
		return NULL;
	}
	PRMSG(5, "MnxTcpOpenCOTSServer: fd= '%d'\n", fd, 0, 0);
	ciptr->fd= fd;
	ref.ref_ptr= ciptr;
	nbio_register(fd);
	nbio_setcallback(fd, ASIO_IOCTL, listen_cb, ref);
	return ciptr;
}
#endif /* TRANS_SERVER */

#ifdef TRANS_CLIENT
static XtransConnInfo
TRANS(MnxTcpOpenCLTSClient) (thistrans, protocol, host, port)

Xtransport *thistrans;
char 	   *protocol;
char 	   *host;
char 	   *port;

{
	abort();
}
#endif /* TRANS_CLIENT */

#ifdef TRANS_SERVER
static XtransConnInfo
TRANS(MnxTcpOpenCLTSServer) (thistrans, protocol, host, port)

Xtransport *thistrans;
char 	   *protocol;
char 	   *host;
char 	   *port;

{
	abort();
}
#endif /* TRANS_SERVER */

#ifdef TRANS_REOPEN

static XtransConnInfo
TRANS(MnxTcpReopenCOTSServer) (thistrans, fd, port)

Xtransport *thistrans;
int	   fd;
char	   *port;

{
    XtransConnInfo	ciptr;
    int			i;

    PRMSG (2,
	"MnxTcpReopenCOTSServer(%d, %s)\n", fd, port, 0);

    abort();
}

static XtransConnInfo
TRANS(MnxTcpReopenCLTSServer) (thistrans, fd, port)

Xtransport *thistrans;
int	   fd;
char	   *port;

{
    XtransConnInfo	ciptr;
    int			i;


    PRMSG (2,
	"MnxTcpReopenCLTSServer(%d, %s)\n", fd, port, 0);

    abort();
}

#endif /* TRANS_REOPEN */



static int
TRANS(MnxTcpSetOption) (ciptr, option, arg)

XtransConnInfo 	ciptr;
int 		option;
int 		arg;

{
	int flags;
	struct private *priv;

	PRMSG(2, "MnxTcpSetOption(%d,%d,%d)\n",
		ciptr->fd, option, arg);

	priv= (struct private *)ciptr->priv;
	switch(option)
	{
	case TRANS_NONBLOCKING:
		flags= fcntl(ciptr->fd, F_GETFD);
		if (flags == -1)
		{
			PRMSG(1,
			"MnxTcpSetOption: fcntl F_GETFD failed: %s\n",
				strerror(errno), 0, 0);
			return -1;
		}
		if (arg == 0)
			flags &= ~FD_ASYNCHIO;
		else if (arg == 1)
			flags |= FD_ASYNCHIO;
		else
		{
			PRMSG(1,
		"MnxTcpSetOption: bad arg for TRANS_NONBLOCKING: %d\n",
				arg, 0, 0);
			return -1;
		}
		if (fcntl(ciptr->fd, F_SETFD, flags) == -1)
		{
			PRMSG(1,
			"MnxTcpSetOption: fcntl F_SETFD failed: %s\n",
				strerror(errno), 0, 0);
			return -1;
		}
		priv->nonblocking= arg;
		return 0;
	case TRANS_CLOSEONEXEC:
		flags= fcntl(ciptr->fd, F_GETFD);
		if (flags == -1)
		{
			PRMSG(1,
			"MnxTcpSetOption: fcntl F_GETFD failed: %s\n",
				strerror(errno), 0, 0);
			return -1;
		}
		if (arg == 0)
			flags &= ~FD_CLOEXEC;
		else if (arg == 1)
			flags |= FD_CLOEXEC;
		else
		{
			PRMSG(1,
		"MnxTcpSetOption: bad arg for TRANS_CLOSEONEXEC: %d\n",
				arg, 0, 0);
			return -1;
		}
		if (fcntl(ciptr->fd, F_SETFD, flags) == -1)
		{
			PRMSG(1,
			"MnxTcpSetOption: fcntl F_SETFD failed: %s\n",
				strerror(errno), 0, 0);
			return -1;
		}
		return 0;
	default:
		PRMSG(1, "MnxTcpSetOption: unknown option '%d'\n",
			option, 0, 0);
		errno= EINVAL;
		return -1;
	}
}


#ifdef TRANS_SERVER
static int
TRANS(MnxTcpCreateListener) (ciptr, port)

XtransConnInfo	ciptr;
char		*port;

{
	struct servent *servp;
	tcpport_t num_port;
	char *check;
	nwio_tcpconf_t tcpconf;
	nwio_tcpcl_t tcpcl;
	int r, s_errno, flags;
	struct private *priv;
	struct sockaddr_in *addr;

	PRMSG(2, "MnxTcpCreateListener(%d,%s)\n", ciptr->fd, port, 0);

	priv= (struct private *)ciptr->priv;

	if (port == NULL)
		num_port= 0;
	else
	{
		num_port= strtol(port, &check, 10);
		num_port= htons(num_port);
		if (check[0] == '\0')
			port= NULL;
	}

#ifdef X11_t
	/*
	 * X has a well known port, that is transport dependent. It is easier
	 * to handle it here, than try and come up with a transport independent
	 * representation that can be passed in and resolved the usual way.
	 *
	 * The port that is passed here is really a string containing the
	 * idisplay from ConnectDisplay().
	 */
	if (port == NULL)
		num_port= htons(ntohs(num_port) + X_TCP_PORT);
#endif
	if (port != NULL)
	{
		if ((servp = getservbyname (port, "tcp")) == NULL)
		{
			PRMSG(1,
		"MnxTcpCreateListener: can't get service for %s\n",
				port, 0, 0);
			errno= EINVAL;
			return TRANS_CREATE_LISTENER_FAILED;
		}
		num_port= servp->s_port;
	}

	tcpconf.nwtc_flags= NWTC_SHARED | NWTC_UNSET_RA | NWTC_UNSET_RP;
	if (num_port != 0)
	{
		tcpconf.nwtc_locport= num_port;
		tcpconf.nwtc_flags |= NWTC_LP_SET;
	}
	else
		tcpconf.nwtc_flags |= NWTC_LP_SEL;

	if (ioctl(ciptr->fd, NWIOSTCPCONF, &tcpconf) == -1)
	{
		PRMSG(1,
		"MnxTcpCreateListener: NWIOSTCPCONF failed: %s\n",
			strerror(errno),0, 0);
		return TRANS_CREATE_LISTENER_FAILED;
	}

	if (ioctl(ciptr->fd, NWIOGTCPCONF, &tcpconf) == -1)
	{
		PRMSG(1,
		"MnxTcpListen: NWIOGTCPCONF failed: %s\n",
			strerror(errno),0, 0);
		return TRANS_CREATE_LISTENER_FAILED;
	}

	priv->listen_port= tcpconf.nwtc_locport;

	if ((addr= (struct sockaddr_in *)xalloc(sizeof(struct sockaddr_in)))
		== NULL)
	{
		PRMSG(1, "MnxTcpAccept: malloc failed\n", 0, 0, 0);
		return TRANS_CREATE_LISTENER_FAILED;
	}
	addr->sin_family= AF_INET;
	addr->sin_addr.s_addr= tcpconf.nwtc_locaddr;
	addr->sin_port= tcpconf.nwtc_locport;
	if (ciptr->addr)
		xfree(ciptr->addr);
	ciptr->addr= (char *)addr;
	ciptr->addrlen= sizeof(struct sockaddr_in);

	flags= fcntl(ciptr->fd, F_GETFD);
	if (flags == -1)
	{
		PRMSG(1,
		"MnxTcpCreateListener: fcntl F_GETFD failed: %s\n",
			strerror(errno), 0, 0);
		return TRANS_CREATE_LISTENER_FAILED;
	}
	if (fcntl(ciptr->fd, F_SETFD, flags | FD_ASYNCHIO) == -1)
	{
		PRMSG(1,
		"MnxTcpCreateListener: fcntl F_SETFD failed: %s\n",
			strerror(errno), 0, 0);
		return TRANS_CREATE_LISTENER_FAILED;
	}

	tcpcl.nwtcl_flags= 0;
	r= ioctl(ciptr->fd, NWIOTCPLISTEN, &tcpcl);
	s_errno= errno;

	if (fcntl(ciptr->fd, F_SETFD, flags) == -1)
	{
		PRMSG(1,
		"MnxTcpCreateListener: fcntl F_SETFD failed: %s\n",
			strerror(errno), 0, 0);
		return TRANS_CREATE_LISTENER_FAILED;
	}

	if (r == -1 && s_errno == EINPROGRESS)
	{
		nbio_inprogress(ciptr->fd, ASIO_IOCTL, 1 /* read */,
			1 /* write */, 0 /* exception */);
		return 0;
	}
	if (r == 0)
	{
		priv->listen_completed= 1;
		return 0;
	}

	errno= s_errno;
	PRMSG(1, "MnxTcpCreateListener: NWIOTCPLISTEN failed: %s\n",
		strerror(errno), 0, 0);
	return TRANS_CREATE_LISTENER_FAILED;
}
#endif /* TRANS_SERVER */


#ifdef TRANS_SERVER
static int
TRANS(MnxTcpResetListener) (ciptr)

XtransConnInfo	ciptr;
{
	PRMSG(2, "MnxTcpResetListener(%d)\n", ciptr->fd, 0, 0);
	return TRANS_RESET_NOOP;
}
#endif /* TRANS_SERVER */

#ifdef TRANS_SERVER
static XtransConnInfo
TRANS(MnxTcpAccept) (ciptr_listen, status)

XtransConnInfo ciptr_listen;
int	       *status;

{
	XtransConnInfo	ciptr;
	int s_errno;
	int fd;
	nbio_ref_t ref;
	struct private *priv;
	nwio_tcpconf_t tcpconf;
	struct sockaddr_in *addr;

	PRMSG(2, "MnxTcpAccept(%d,%p)\n", ciptr_listen->fd, status, 0);

	priv= (struct private *)ciptr_listen->priv;
	*status= TRANS_ACCEPT_MISC_ERROR;

	if (!priv->listen_completed)
	{
		PRMSG(1, "MnxTcpAccept: listen is not completed\n",
			0, 0, 0);
		*status= TRANS_ACCEPT_FAILED;
		return NULL;
	}
	priv->listen_completed= 0;

	if ((ciptr= alloc_ConnInfo(ciptr_listen->transptr)) == NULL)
	{
		PRMSG(1,
			"MnxTcpAccept: alloc_ConnInfo failed\n",
			0, 0, 0);
		*status= TRANS_ACCEPT_BAD_MALLOC;
		return NULL;
	}
	if ((ciptr->priv= (char *)alloc_private(RD_BUFSIZE, WR_BUFSIZE)) ==
		NULL)
	{
		PRMSG(1,
			"MnxTcpAccept: alloc_private() failed\n",
			0, 0, 0);
		s_errno= errno;
		free_ConnInfo(ciptr);
		errno= s_errno;
		*status= TRANS_ACCEPT_BAD_MALLOC;
		return NULL;
	}

	fd= dup(ciptr_listen->fd);
	if (fd == -1)
	{
		s_errno= errno;
		PRMSG(1, "MnxTcpAccept: dup failed: %s\n",
			strerror(errno), 0, 0);
		free_ConnInfo(ciptr);
		*status= TRANS_ACCEPT_FAILED;
		return NULL;
	}
	if (restart_listen(ciptr_listen) == -1)
	{
		priv->listen_list= listen_list;
		listen_list= ciptr_listen;
		PRMSG(1, "MnxTcpAccept: unable to restart listen\n",
			0, 0, 0);
	}
	ciptr->fd= fd;
	ref.ref_ptr= ciptr;
	nbio_register(fd);
	nbio_setcallback(fd, ASIO_WRITE, write_cb, ref);
	nbio_setcallback(fd, ASIO_READ, read_cb, ref);

	if (ioctl(ciptr->fd, NWIOGTCPCONF, &tcpconf) == -1)
	{
		PRMSG(1, "MnxTcpAccept: NWIOGTCPCONF failed: %s\n",
			strerror(errno),0, 0);
		close(fd);
		free_ConnInfo(ciptr);
		*status= TRANS_ACCEPT_MISC_ERROR;
		return NULL;
	}
	if ((addr= (struct sockaddr_in *)xalloc(sizeof(struct sockaddr_in)))
		== NULL)
	{
		PRMSG(1, "MnxTcpAccept: malloc failed\n", 0, 0, 0);
		close(fd);
		free_ConnInfo(ciptr);
		*status= TRANS_ACCEPT_BAD_MALLOC;
		return NULL;
	}
	addr->sin_family= AF_INET;
	addr->sin_addr.s_addr= tcpconf.nwtc_locaddr;
	addr->sin_port= tcpconf.nwtc_locport;
	if (ciptr->addr)
		xfree(ciptr->addr);
	ciptr->addr= (char *)addr;
	ciptr->addrlen= sizeof(struct sockaddr_in);
	if (*(u8_t *)&tcpconf.nwtc_remaddr == 127)
	{
		/* Make ConvertAddress return FamilyLocal */
		addr->sin_addr.s_addr= tcpconf.nwtc_remaddr;
	}

	if ((addr= (struct sockaddr_in *)xalloc(sizeof(struct sockaddr_in)))
		== NULL)
	{
		PRMSG(1, "MnxTcpConnect: malloc failed\n", 0, 0, 0);
		close(fd);
		free_ConnInfo(ciptr);
		*status= TRANS_ACCEPT_BAD_MALLOC;
		return NULL;
	}
	addr->sin_family= AF_INET;
	addr->sin_addr.s_addr= tcpconf.nwtc_remaddr;
	addr->sin_port= tcpconf.nwtc_remport;
	ciptr->peeraddr= (char *)addr;
	ciptr->peeraddrlen= sizeof(struct sockaddr_in);
	*status= 0;
	return ciptr;
}
#endif /* TRANS_SERVER */

TRANS(MnxTcpConnect) (ciptr, host, port)

XtransConnInfo 	ciptr;
char 		*host;
char 		*port;

{
	struct hostent *hostp;
	struct servent *servp;
	char hostnamebuf[256];		/* tmp space */
	tcpport_t num_port;
	ipaddr_t num_addr;
	char *check;
	nwio_tcpconf_t tcpconf;
	nwio_tcpcl_t tcpcl;
	struct sockaddr_in *addr;

	PRMSG(2, "MnxTcpConnect(%d,%s,%s)\n", ciptr->fd, host, port);

	if (!host)
	{
		hostnamebuf[0] = '\0';
		(void) TRANS(GetHostname) (hostnamebuf, sizeof hostnamebuf);
		host = hostnamebuf;
	}


	num_port= strtol(port, &check, 10);
	num_port= htons(num_port);
	if (check[0] == '\0')
		port= NULL;
#ifdef X11_t
	/*
	 * X has a well known port, that is transport dependent. It is easier
	 * to handle it here, than try and come up with a transport independent
	 * representation that can be passed in and resolved the usual way.
	 *
	 * The port that is passed here is really a string containing the
	 * idisplay from ConnectDisplay().
	 */
	if (port == NULL)
		num_port= htons(ntohs(num_port) + X_TCP_PORT);
#endif

	num_addr= inet_addr(host);
	if (num_addr != -1)
		host= NULL;

	if (host != NULL)
	{
		if ((hostp = gethostbyname(host)) == NULL)
		{
			PRMSG(1,
			"MnxTcpConnect: can't get address for %s\n",
				host, 0, 0);
			errno= EINVAL;
			return TRANS_CONNECT_FAILED;
		}
		if (hostp->h_addrtype != AF_INET)  /* is IP host? */
		{
		    PRMSG(1, "MnxTcpConnect: %s in not an INET host\n",
			  host, 0, 0);
		    errno= EINVAL;
		    return TRANS_CONNECT_FAILED;
		}
		num_addr= *(ipaddr_t *)hostp->h_addr;
	}

	if (port != NULL)
	{
		if ((servp = getservbyname (port, "tcp")) == NULL)
		{
			PRMSG(1,
			"MnxTcpConnect: can't get service for %s\n",
				port, 0, 0);
			errno= EINVAL;
			return TRANS_CONNECT_FAILED;
		}
		num_port= servp->s_port;
	}

	tcpconf.nwtc_flags= NWTC_EXCL | NWTC_LP_SEL | NWTC_SET_RA |
		NWTC_SET_RP;
	tcpconf.nwtc_remaddr= num_addr;
	tcpconf.nwtc_remport= num_port;
	if (ioctl(ciptr->fd, NWIOSTCPCONF, &tcpconf) == -1)
	{
		PRMSG(1, "MnxTcpConnect: NWIOSTCPCONF failed: %s\n",
			strerror(errno),0, 0);
		return TRANS_CONNECT_FAILED;
	}

	tcpcl.nwtcl_flags= 0;
	if (ioctl(ciptr->fd, NWIOTCPCONN, &tcpcl) == -1)
	{
		PRMSG(1, "MnxTcpConnect: connect failed: %s\n",
			strerror(errno),0, 0);
		if (errno == ECONNREFUSED || errno == EINTR)
			return TRANS_TRY_CONNECT_AGAIN;
		else
			return TRANS_CONNECT_FAILED;
	}

	if (ioctl(ciptr->fd, NWIOGTCPCONF, &tcpconf) == -1)
	{
		PRMSG(1, "MnxTcpConnect: NWIOGTCPCONF failed: %s\n",
			strerror(errno),0, 0);
		return TRANS_CONNECT_FAILED;
	}
	if ((addr= (struct sockaddr_in *)xalloc(sizeof(struct sockaddr_in)))
		== NULL)
	{
		PRMSG(1, "MnxTcpConnect: malloc failed\n", 0, 0, 0);
		return TRANS_CONNECT_FAILED;
	}
	addr->sin_family= AF_INET;
	addr->sin_addr.s_addr= tcpconf.nwtc_locaddr;
	addr->sin_port= tcpconf.nwtc_locport;
	ciptr->addr= (char *)addr;
	ciptr->addrlen= sizeof(struct sockaddr_in);
	if (*(u8_t *)&tcpconf.nwtc_remaddr == 127)
	{
		/* Make ConvertAddress return FamilyLocal */
		addr->sin_addr.s_addr= tcpconf.nwtc_remaddr;
	}

	if ((addr= (struct sockaddr_in *)xalloc(sizeof(struct sockaddr_in)))
		== NULL)
	{
		PRMSG(1, "MnxTcpConnect: malloc failed\n", 0, 0, 0);
		return TRANS_CONNECT_FAILED;
	}
	addr->sin_family= AF_INET;
	addr->sin_addr.s_addr= tcpconf.nwtc_remaddr;
	addr->sin_port= tcpconf.nwtc_remport;
	ciptr->peeraddr= (char *)addr;
	ciptr->peeraddrlen= sizeof(struct sockaddr_in);

	return 0;
}

static int
TRANS(MnxTcpBytesReadable) (ciptr, pend)

XtransConnInfo ciptr;
BytesReadable_t *pend;

{
	struct private *priv;
	int r;

	PRMSG(2, "MnxTcpBytesReadable(%x,%d,%x)\n",
		ciptr, ciptr->fd, pend);

	*pend= 0;

	priv= (struct private *)ciptr->priv;
	if (priv->read_inprogress)
	{
		PRMSG(5, "MnxTcpBytesReadable: read inprogress, %d\n",
			*pend, 0, 0);
		return *pend;
	}
	if (priv->read_offset < priv->read_size)
	{
		*pend= priv->read_size-priv->read_offset;
		PRMSG(5, "MnxTcpBytesReadable: %d\n",
			*pend, 0, 0);
		return *pend;
	}
	priv->read_offset= 0;
	r= read(ciptr->fd, priv->read_buffer, priv->read_bufsize);
	if (r >= 0)
	{
		if (r == 0)
			r= 1;	/* Signal EOF condition */

		priv->read_size= r;
		PRMSG(5, "MnxTcpBytesReadable: %d\n",
			*pend, 0, 0);
		*pend= r;
	}
	else if (r == -1 && errno == EINPROGRESS)
	{
		priv->read_inprogress= 1;
		nbio_inprogress(ciptr->fd, ASIO_READ, 1 /* read */,
			0 /* write */, 0 /* exception */);
	}
	else
	{
		PRMSG(1, "MnxTcpBytesReadable: read failed: %s\n",
			strerror(errno), 0, 0);
		return -1;
	}
	PRMSG(5, "MnxTcpBytesReadable: %d\n", *pend, 0, 0);
	return *pend;
}


static int
TRANS(MnxTcpRead) (ciptr, buf, size)

XtransConnInfo	ciptr;
char		*buf;
int		size;

{
	int len, r, ret, s_errno;
	int offset;
	struct private *priv;
	asio_fd_set_t fd_set;
	fwait_t fw;

	PRMSG(2, "MnxTcpRead(%d,%x,%d)\n", ciptr->fd, buf, size);

	priv= (struct private *)ciptr->priv;
	offset= 0;

	if (priv->read_inprogress)
	{
		PRMSG(5, "MnxTcpRead: EAGAIN\n", 0, 0, 0);
		errno= EAGAIN;
		return -1;
	}

	/* Copy any data left in the buffer */
	if (priv->read_offset < priv->read_size)
	{
		len= priv->read_size-priv->read_offset;
		if (len > size-offset)
			len= size-offset;
		PRMSG(5, "MnxTcpRead: copying %d bytes\n", len, 0, 0);

		memcpy(buf+offset, priv->read_buffer + priv->read_offset,
			len);
		offset += len;
		priv->read_offset += len;
		if (priv->read_offset < priv->read_size)
			return offset;
	}

	/* Try to read directly into the user's buffer. */
	ret= 0;
	s_errno= 0;
	while(offset < size)
	{
		r= read(ciptr->fd, buf+offset, size-offset);
		if (r == -1 && errno == EINPROGRESS)
		{
			r= fcancel(ciptr->fd, ASIO_READ);
			if (r == -1)
				abort();
			ASIO_FD_ZERO(&fd_set);
			ASIO_FD_SET(ciptr->fd, ASIO_READ, &fd_set);
			fw.fw_flags= FWF_NONBLOCK;
			fw.fw_bits= fd_set.afds_bits;
			fw.fw_maxfd= ASIO_FD_SETSIZE;
			r= fwait(&fw);
			if (r == -1 || fw.fw_fd != ciptr->fd ||
				fw.fw_operation != ASIO_READ)
			{
				abort();
			}
			r= fw.fw_result;
			errno= fw.fw_errno;
		}

		if (r > 0)
		{
			PRMSG(5, "MnxTcpRead: read %d bytes\n", r,
				0, 0);
			offset += r;
			continue;
		}
		else if (r == 0)
		{
			PRMSG(5, "MnxTcpRead: read EOF\n", 0, 0, 0);
			break;
		}
		else
		{
			if (errno == EINTR)
			{
				PRMSG(5, "MnxTcpRead: EINTR\n",
					0, 0, 0);
				errno= EAGAIN;
			}
			else
			{
				PRMSG(1, "MnxTcpRead: read error %s\n",
					strerror(errno), 0, 0);
			}
			s_errno= errno;
			ret= -1;
			break;
		}
	}
	if (offset != 0)
		ret= offset;

	if (priv->read_offset != priv->read_size)
		abort();
	priv->read_offset= 0;
	priv->read_size= 0;
	if (priv->nonblocking)
	{
		r= read(ciptr->fd, priv->read_buffer, priv->read_bufsize);
		if (r >= 0)
		{
			PRMSG(5, "MnxTcpRead: buffered %d bytes\n",
				r, 0, 0);
			priv->read_size= r;
		}
		else if (r == -1 && errno == EINPROGRESS)
		{
			priv->read_inprogress= 1;
			nbio_inprogress(ciptr->fd, ASIO_READ, 1 /* read */,
				0 /* write */, 0 /* exception */);
		}
		else
		{
			PRMSG(1, "MnxTcpRead: read failed: %s\n",
				strerror(errno), 0, 0);
		}
	}
	errno= s_errno;
	return ret;
}


static int
TRANS(MnxTcpWrite) (ciptr, buf, size)

XtransConnInfo ciptr;
char 	       *buf;
int 	       size;

{
	int len, r, ret, s_errno;
	int offset;
	struct private *priv;
	asio_fd_set_t fd_set;
	fwait_t fw;

	PRMSG(2, "MnxTcpWrite(%d,%x,%d)\n", ciptr->fd, buf, size);

	priv= (struct private *)ciptr->priv;
	offset= 0;

	if (priv->write_errno)
	{
		PRMSG(5, "MnxTcpWrite: write_errno %d\n",
			priv->write_errno, 0, 0);
		errno= priv->write_errno;
		return -1;
	}

	if (priv->write_inprogress)
	{
		PRMSG(5, "MnxTcpWrite: EAGAIN\n", 0, 0, 0);
		errno= EAGAIN;
		return -1;
	}

	/* Try to write directly out of the user's buffer. */
	ret= 0;
	s_errno= 0;
	while(offset < size)
	{
		r= write(ciptr->fd, buf+offset, size-offset);
		if (r == -1 && errno == EINPROGRESS)
		{
			r= fcancel(ciptr->fd, ASIO_WRITE);
			if (r == -1)
				abort();
			ASIO_FD_ZERO(&fd_set);
			ASIO_FD_SET(ciptr->fd, ASIO_WRITE, &fd_set);
			fw.fw_flags= FWF_NONBLOCK;
			fw.fw_bits= fd_set.afds_bits;
			fw.fw_maxfd= ASIO_FD_SETSIZE;
			r= fwait(&fw);
			if (r == -1 || fw.fw_fd != ciptr->fd ||
				fw.fw_operation != ASIO_WRITE)
			{
				abort();
			}
			r= fw.fw_result;
			errno= fw.fw_errno;
		}
		if (r > 0)
		{
			PRMSG(5, "MnxTcpWrite: wrote %d bytes\n", r,
				0, 0);
			offset += r;
			continue;
		}
		else if (r == 0)
			abort();
		else
		{
			if (errno == EINTR)
			{
				PRMSG(5, "MnxTcpWrite: EINTR\n",
					0, 0, 0);
				errno= EAGAIN;
			}
			else
			{
				PRMSG(1,
				"MnxTcpWrite: write error: %s\n",
					strerror(errno), 0, 0);
			}
			s_errno= errno;
			ret= -1;
			break;
		}
	}

	/* Copy any data to the buffer */
	if (offset < size)
	{
		len= priv->write_bufsize;
		if (len > size-offset)
			len= size-offset;
		PRMSG(5, "MnxTcpWrite: copying %d bytes\n", len, 0, 0);

		memcpy(priv->write_buffer, buf+offset, len);
		offset += len;
		priv->write_offset= 0;
		priv->write_size= len;
	}
	if (offset != 0)
		ret= offset;

	while (priv->write_offset < priv->write_size)
	{
		r= write(ciptr->fd, priv->write_buffer+priv->write_offset,
			priv->write_size-priv->write_offset);
		if (r > 0)
		{
			PRMSG(5, "MnxTcpWrite: wrote %d bytes\n",
				r, 0, 0);
			priv->write_offset += r;
			continue;
		}
		else if (r == -1 && errno == EINPROGRESS)
		{
			priv->write_inprogress= 1;
			nbio_inprogress(ciptr->fd, ASIO_WRITE, 0 /* read */,
				1 /* write */, 0 /* exception */);
		}
		else
		{
			PRMSG(1, "MnxTcpWrite: write failed: %s\n",
				strerror(errno), 0, 0);
			priv->write_errno= errno;
		}
		break;
	}

	errno= s_errno;
	return ret;
}


static int
TRANS(MnxTcpReadv) (ciptr, buf, size)

XtransConnInfo	ciptr;
struct iovec 	*buf;
int 		size;

{
	int i, offset, total, len, r;

	PRMSG(2, "MnxTcpReadv(%d,%x,%d)\n", ciptr->fd, buf, size);

	/* Simply call read a number of times. */
	total= 0;
	offset= 0;
	i= 0;
	while(i<size)
	{
		PRMSG(5, "MnxTcpReadv: [%d] size %d-%d\n",
			i, buf[i].iov_len, offset);
		if (offset >= buf[i].iov_len)
		{
			offset= 0;
			i++;
			continue;
		}
		len= buf[i].iov_len-offset;
		r= TRANS(MnxTcpRead)(ciptr, buf[i].iov_base+offset, len);
		if (r == -1)
		{
			if (errno == EAGAIN)
			{
				PRMSG(5,
				"MnxTcpReadv: read returned: %s\n",
					strerror(errno), 0, 0);
			}
			else
			{
				PRMSG(1,
				"MnxTcpReadv: read failed: %s\n",
					strerror(errno), 0, 0);
			}
			if (total != 0)
				return total;
			else
				return -1;
		}
		if (r == 0)
			break;
		if (r > len)
			abort();
		total += r;
		offset += r;
	}
	return total;
}

static int
TRANS(MnxTcpWritev) (ciptr, buf, size)

XtransConnInfo 	ciptr;
struct iovec 	*buf;
int 		size;

{
	int i, offset, total, len, r;

	PRMSG(2, "MnxTcpWritev(%d,%x,%d)\n", ciptr->fd, buf, size);

	/* Simply call write a number of times. */
	total= 0;
	offset= 0;
	i= 0;
	while(i<size)
	{
		if (offset >= buf[i].iov_len)
		{
			offset= 0;
			i++;
			continue;
		}
		len= buf[i].iov_len-offset;
		r= TRANS(MnxTcpWrite)(ciptr, buf[i].iov_base+offset, len);
		if (r == -1)
		{
			if (errno == EAGAIN)
			{
				PRMSG(5, "MnxTcpWritev: AGAIN\n",
					0, 0, 0);
			}
			else
			{
				PRMSG(1,
				"MnxTcpWritev: write failed: %s\n",
					strerror(errno), 0, 0);
			}
			if (total != 0)
				return total;
			else
				return -1;
		}
		if (r == 0 || r > len)
			abort();
		total += r;
		offset += r;
	}
	return total;
}


static int
TRANS(MnxTcpDisconnect) (ciptr)

XtransConnInfo ciptr;

{
	PRMSG(2, "MnxTcpDisconnect(%x,%d)\n", ciptr, ciptr->fd, 0);

	return ioctl(ciptr->fd, NWIOTCPSHUTDOWN, NULL);
}

static int
TRANS(MnxTcpClose) (ciptr)

XtransConnInfo ciptr;

{
	XtransConnInfo list, t_ciptr;
	struct private *priv;

	PRMSG(2, "MnxTcpClose(%x,%d)\n", ciptr, ciptr->fd, 0);


	if (listen_list)
	{
		list= listen_list;
		listen_list= NULL;
		while(list)
		{
			t_ciptr= list;
			priv= (struct private *)t_ciptr->priv;
			list= priv->listen_list;
			if (t_ciptr == ciptr)
				continue;
			if (restart_listen(t_ciptr) == -1)
			{
				priv->listen_list= listen_list;
				listen_list= t_ciptr;
			}
		}
	}

	free_private((struct private *)ciptr->priv);
	nbio_unregister(ciptr->fd);
	return close (ciptr->fd);
}


static XtransConnInfo
alloc_ConnInfo(thistrans)
Xtransport *thistrans;
{
	XtransConnInfo ciptr;

	PRMSG(2, " alloc_ConnInfo(%p)\n", thistrans, 0, 0);

	if ((ciptr= (XtransConnInfo) xalloc(sizeof(struct _XtransConnInfo)))
		== NULL)
	{
		PRMSG(1, " alloc_ConnInfo: malloc failed\n", 0, 0, 0);
		return NULL;
	}
	ciptr->transptr= thistrans;
	ciptr->priv= NULL;
	ciptr->flags= 0;
	ciptr->fd= -1;
	ciptr->port= NULL;
	ciptr->family= AF_INET;
	ciptr->addr= NULL;
	ciptr->addrlen= 0;
	ciptr->peeraddr= NULL;
	ciptr->peeraddrlen= 0;
	return ciptr;
}

static void
free_ConnInfo(ciptr)
XtransConnInfo ciptr;
{
	if (ciptr == NULL)
		return;
	free_private((struct private *)ciptr->priv);
	xfree(ciptr);
}

static struct private *
alloc_private(rd_size, wr_size)
size_t rd_size;
size_t wr_size;
{
	struct private *priv;
	int s_errno;
	char *buf;

	PRMSG(2, ":alloc_private(%d, %d)\n", rd_size, wr_size, 0);

	if ((priv= (struct private *)xalloc(sizeof(struct private))) == NULL)
	{
		PRMSG(1, ":alloc_private: malloc failed\n", 0, 0, 0);
		return NULL;
	}
	priv->nonblocking= 0;
	priv->read_inprogress= 0;
	priv->read_buffer= NULL;
	priv->read_bufsize= rd_size;
	priv->read_size= 0;
	priv->read_offset= 0;

	if (rd_size != 0)
	{
		if ((buf= xalloc(rd_size)) == NULL)
		{
			PRMSG(1, ":alloc_private: malloc failed\n", 0, 0, 0);
			s_errno= errno;
			free_private(priv);
			errno= s_errno;
			return NULL;
		}
		priv->read_buffer= buf;
	}

	priv->write_inprogress= 0;
	priv->write_buffer= NULL;
	priv->write_bufsize= rd_size;
	priv->write_size= 0;
	priv->write_offset= 0;
	priv->write_errno= 0;

	if (wr_size != 0)
	{
		if ((buf= xalloc(wr_size)) == NULL)
		{
			PRMSG(1, ":alloc_private: malloc failed\n", 0, 0, 0);
			s_errno= errno;
			free_private(priv);
			errno= s_errno;
			return NULL;
		}
		priv->write_buffer= buf;
	}

	priv->listen_completed= 0;
	priv->listen_port= 0;
	priv->listen_list= NULL;

	return priv;
}

static void
free_private(priv)
struct private *priv;
{
	if (priv == NULL)
		return;
	xfree(priv->read_buffer);
	xfree(priv->write_buffer);
	xfree(priv);
}

static void
read_cb(ref, res, err)
nbio_ref_t ref;
int res;
int err;
{
	XtransConnInfo ciptr;
	struct private *priv;

	PRMSG(2, ":read_cb(%x,%d,%d)\n", ref.ref_ptr, res, err);

	ciptr= ref.ref_ptr;
	priv= (struct private *)ciptr->priv;
	if (res > 0)
		priv->read_size= res;
	priv->read_inprogress= 0;
}

static void
write_cb(ref, res, err)
nbio_ref_t ref;
int res;
int err;
{
	XtransConnInfo ciptr;
	struct private *priv;
	int r;

	PRMSG(2, ":write_cb(%x,%d,%d)\n", ref.ref_ptr, res, err);

	ciptr= ref.ref_ptr;
	priv= (struct private *)ciptr->priv;
	if (res > 0)
		priv->write_offset += res;
	else if (res == 0)
		abort();
	else
	{
		priv->write_errno= err;
		return;
	}
	priv->write_inprogress= 0;

	while (priv->write_offset < priv->write_size)
	{
		r= write(ciptr->fd, priv->write_buffer+priv->write_offset,
			priv->write_size-priv->write_offset);
		if (r > 0)
		{
			PRMSG(5, "MnxTcpWrite: wrote %d bytes\n",
				r, 0, 0);
			priv->write_offset += r;
			continue;
		}
		else if (r == -1 && errno == EINPROGRESS)
		{
			priv->write_inprogress= 1;
			nbio_inprogress(ciptr->fd, ASIO_WRITE, 0 /* read */,
				1 /* write */, 0 /* exception */);
		}
		else
		{
			PRMSG(1, "MnxTcpWrite: write failed: %s\n",
				strerror(errno), 0, 0);
			priv->write_errno= errno;
		}
		break;
	}
}

static void
listen_cb(ref, res, err)
nbio_ref_t ref;
int res;
int err;
{
	XtransConnInfo ciptr;
	struct private *priv;
	struct sockaddr_in *addr;
	nwio_tcpconf_t tcpconf;

	PRMSG(2, ":listen_cb(%x,%d,%d)\n", ref.ref_ptr, res, err);

	ciptr= ref.ref_ptr;
	priv= (struct private *)ciptr->priv;
	if (res == 0)
	{
		if (ioctl(ciptr->fd, NWIOGTCPCONF, &tcpconf) == -1)
		{
			PRMSG(1,
			":listen_cb: NWIOGTCPCONF failed: %s\n",
				strerror(errno),0, 0);
			return;
		}
		if ((addr= (struct sockaddr_in *)xalloc(sizeof(struct sockaddr_in)))
			== NULL)
		{
			PRMSG(1, ":listen_cb: malloc failed\n", 0, 0, 0);
			return;
		}
		addr->sin_family= AF_INET;
		addr->sin_addr.s_addr= tcpconf.nwtc_locaddr;
		addr->sin_port= tcpconf.nwtc_locport;
		if (ciptr->addr)
			xfree(ciptr->addr);
		ciptr->addr= (char *)addr;
		ciptr->addrlen= sizeof(struct sockaddr_in);
		priv->listen_completed= 1;
		return;
	}
	PRMSG(2, ":listen_cb: listen failed: %s\n", strerror(err), 0, 0);
	if (restart_listen(ciptr) == -1)
	{
		priv->listen_list= listen_list;
		listen_list= ciptr;
	}
}

static int
restart_listen(ciptr)
XtransConnInfo	ciptr;
{
	char *tcp_device;
	nwio_tcpconf_t tcpconf;
	nwio_tcpcl_t tcpcl;
	int fd, r, s_errno, flags;
	struct private *priv;
	nbio_ref_t ref;

	PRMSG(2, ":restart_listen(%d)\n", ciptr->fd, 0, 0);

	nbio_unregister(ciptr->fd);

	if ((tcp_device= getenv("TCP_DEVICE")) == NULL)
		tcp_device= TCP_DEVICE;

	if ((fd= open(tcp_device, O_RDWR)) == -1)
	{
		PRMSG(1, ":restart_listen: open '%s' failed: %s\n",
			tcp_device, strerror(errno), 0);
		return -1;
	}
	PRMSG(5, ":restart_listen: fd= '%d'\n", fd, 0, 0);
	if (fd != ciptr->fd)
	{
		if (dup2(fd, ciptr->fd) == -1)
			abort();	/* no way to recover */
		close(fd);
	}
	fd= ciptr->fd;
	ref.ref_ptr= ciptr;
	nbio_register(fd);
	nbio_setcallback(fd, ASIO_IOCTL, listen_cb, ref);

	priv= (struct private *)ciptr->priv;

	tcpconf.nwtc_flags= NWTC_SHARED | NWTC_UNSET_RA | NWTC_UNSET_RP;
	tcpconf.nwtc_locport= priv->listen_port;
	tcpconf.nwtc_flags |= NWTC_LP_SET;

	if (ioctl(ciptr->fd, NWIOSTCPCONF, &tcpconf) == -1)
	{
		PRMSG(1,
		":restart_listen: NWIOSTCPCONF failed: %s\n",
			strerror(errno),0, 0);
		return -1;
	}

	flags= fcntl(ciptr->fd, F_GETFD);
	if (flags == -1)
	{
		PRMSG(1,
		":restart_listen: fcntl F_GETFD failed: %s\n",
			strerror(errno), 0, 0);
		return -1;
	}
	if (fcntl(ciptr->fd, F_SETFD, flags | FD_ASYNCHIO) == -1)
	{
		PRMSG(1,
		":restart_listen: fcntl F_SETFD failed: %s\n",
			strerror(errno), 0, 0);
		return -1;
	}

	tcpcl.nwtcl_flags= 0;
	r= ioctl(ciptr->fd, NWIOTCPLISTEN, &tcpcl);
	s_errno= errno;

	if (fcntl(ciptr->fd, F_SETFD, flags) == -1)
	{
		PRMSG(1,
		":restart_listen: fcntl F_SETFD failed: %s\n",
			strerror(errno), 0, 0);
		return -1;
	}

	if (r == -1 && s_errno == EINPROGRESS)
	{
		nbio_inprogress(ciptr->fd, ASIO_IOCTL, 1 /* read */,
			1 /* write */, 0 /* exception */);
		return 0;
	}
	if (r == 0)
	{
		priv->listen_completed= 1;
		return 0;
	}
	errno= s_errno;
	PRMSG(1, ":restart_listen: NWIOTCPLISTEN failed: %s\n",
		strerror(errno), 0, 0);
	return -1;
}


Xtransport	TRANS(MnxINETFuncs) =
{
	/* Minix TCP Interface */
	"inet",
	0,
#ifdef TRANS_CLIENT
	TRANS(MnxTcpOpenCOTSClient),
#endif /* TRANS_CLIENT */
#ifdef TRANS_SERVER
	TRANS(MnxTcpOpenCOTSServer),
#endif /* TRANS_SERVER */
#ifdef TRANS_CLIENT
	TRANS(MnxTcpOpenCLTSClient),
#endif /* TRANS_CLIENT */
#ifdef TRANS_SERVER
	TRANS(MnxTcpOpenCLTSServer),
#endif /* TRANS_SERVER */
#ifdef TRANS_REOPEN
	TRANS(MnxTcpReopenCOTSServer),
	TRANS(MnxTcpReopenCLTSServer),
#endif
	TRANS(MnxTcpSetOption),
#ifdef TRANS_SERVER
	TRANS(MnxTcpCreateListener),
	TRANS(MnxTcpResetListener),
	TRANS(MnxTcpAccept),
#endif /* TRANS_SERVER */
#ifdef TRANS_CLIENT
	TRANS(MnxTcpConnect),
#endif /* TRANS_CLIENT */
	TRANS(MnxTcpBytesReadable),
	TRANS(MnxTcpRead),
	TRANS(MnxTcpWrite),
	TRANS(MnxTcpReadv),
	TRANS(MnxTcpWritev),
	TRANS(MnxTcpDisconnect),
	TRANS(MnxTcpClose),
	TRANS(MnxTcpClose),
};

Xtransport	TRANS(MnxTCPFuncs) =
{
	/* Minix TCP Interface */
	"tcp",
	TRANS_ALIAS,
#ifdef TRANS_CLIENT
	TRANS(MnxTcpOpenCOTSClient),
#endif /* TRANS_CLIENT */
#ifdef TRANS_SERVER
	TRANS(MnxTcpOpenCOTSServer),
#endif /* TRANS_SERVER */
#ifdef TRANS_CLIENT
	TRANS(MnxTcpOpenCLTSClient),
#endif /* TRANS_CLIENT */
#ifdef TRANS_SERVER
	TRANS(MnxTcpOpenCLTSServer),
#endif /* TRANS_SERVER */
#ifdef TRANS_REOPEN
	TRANS(MnxTcpReopenCOTSServer),
	TRANS(MnxTcpReopenCLTSServer),
#endif
	TRANS(MnxTcpSetOption),
#ifdef TRANS_SERVER
	TRANS(MnxTcpCreateListener),
	TRANS(MnxTcpResetListener),
	TRANS(MnxTcpAccept),
#endif /* TRANS_SERVER */
#ifdef TRANS_CLIENT
	TRANS(MnxTcpConnect),
#endif /* TRANS_CLIENT */
	TRANS(MnxTcpBytesReadable),
	TRANS(MnxTcpRead),
	TRANS(MnxTcpWrite),
	TRANS(MnxTcpReadv),
	TRANS(MnxTcpWritev),
	TRANS(MnxTcpDisconnect),
	TRANS(MnxTcpClose),
	TRANS(MnxTcpClose),
};
