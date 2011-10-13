/* $Xorg: xdmauth.c,v 1.4 2001/02/09 02:05:24 xorgcvs Exp $ */
/*

Copyright 1988, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xdmauth.c,v 1.7 2002/11/05 05:50:34 keithp Exp $ */

/*
 * XDM-AUTHENTICATION-1 (XDMCP authentication) and
 * XDM-AUTHORIZATION-1 (client authorization) protocols
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include <X11/X.h>
#include <X11/Xtrans/Xtrans.h>
#include "os.h"
#include "osdep.h"
#include "dixstruct.h"

#ifdef HASXDMAUTH

static Bool authFromXDMCP;

#ifdef XDMCP
#include <X11/Xmd.h>
#undef REQUEST
#include <X11/Xdmcp.h>

/* XDM-AUTHENTICATION-1 */

static XdmAuthKeyRec	privateKey;
static char XdmAuthenticationName[] = "XDM-AUTHENTICATION-1";
#define XdmAuthenticationNameLen (sizeof XdmAuthenticationName - 1)
static XdmAuthKeyRec	rho;

static Bool 
XdmAuthenticationValidator (ARRAY8Ptr privateData, ARRAY8Ptr incomingData, 
    xdmOpCode packet_type)
{
    XdmAuthKeyPtr	incoming;

    XdmcpUnwrap (incomingData->data, &privateKey,
			      incomingData->data,incomingData->length);
    switch (packet_type)
    {
    case ACCEPT:
    	if (incomingData->length != 8)
	    return FALSE;
    	incoming = (XdmAuthKeyPtr) incomingData->data;
    	XdmcpDecrementKey (incoming);
    	return XdmcpCompareKeys (incoming, &rho);
    }
    return FALSE;
}

static Bool
XdmAuthenticationGenerator (ARRAY8Ptr privateData, ARRAY8Ptr outgoingData, 
    xdmOpCode packet_type)
{
    outgoingData->length = 0;
    outgoingData->data = 0;
    switch (packet_type)
    {
    case REQUEST:
	if (XdmcpAllocARRAY8 (outgoingData, 8))
	    XdmcpWrap (&rho, &privateKey, outgoingData->data, 8);
    }
    return TRUE;
}

static Bool
XdmAuthenticationAddAuth (int name_len, char *name, 
    int data_len, char *data)
{
    Bool    ret;
    XdmcpUnwrap (data, (unsigned char *)&privateKey, data, data_len);
    authFromXDMCP = TRUE;
    ret = AddAuthorization (name_len, name, data_len, data);
    authFromXDMCP = FALSE;
    return ret;
}


#define atox(c)	('0' <= c && c <= '9' ? c - '0' : \
		 'a' <= c && c <= 'f' ? c - 'a' + 10 : \
		 'A' <= c && c <= 'F' ? c - 'A' + 10 : -1)

static int
HexToBinary (char *in, char *out, int len)
{
    int	    top, bottom;

    while (len > 0)
    {
	top = atox(in[0]);
	if (top == -1)
	    return 0;
	bottom = atox(in[1]);
	if (bottom == -1)
	    return 0;
	*out++ = (top << 4) | bottom;
	in += 2;
	len -= 2;
    }
    if (len)
	return 0;
    *out++ = '\0';
    return 1;
}

void
XdmAuthenticationInit (char *cookie, int cookie_len)
{
    bzero (privateKey.data, 8);
    if (!strncmp (cookie, "0x", 2) || !strncmp (cookie, "0X", 2))
    {
	if (cookie_len > 2 + 2 * 8)
	    cookie_len = 2 + 2 * 8;
	HexToBinary (cookie + 2, (char *)privateKey.data, cookie_len - 2);
    }
    else
    {
    	if (cookie_len > 7)
	    cookie_len = 7;
    	memmove (privateKey.data + 1, cookie, cookie_len);
    }
    XdmcpGenerateKey (&rho);
    XdmcpRegisterAuthentication (XdmAuthenticationName, XdmAuthenticationNameLen,
				 (unsigned char *)&rho,
				 sizeof (rho),
				 XdmAuthenticationValidator,
				 XdmAuthenticationGenerator,
				 XdmAuthenticationAddAuth);
}

#endif /* XDMCP */

/* XDM-AUTHORIZATION-1 */
typedef struct _XdmAuthorization {
    struct _XdmAuthorization	*next;
    XdmAuthKeyRec		rho;
    XdmAuthKeyRec		key;
    XID				id;
} XdmAuthorizationRec, *XdmAuthorizationPtr;

static XdmAuthorizationPtr xdmAuth;

typedef struct _XdmClientAuth {
    struct _XdmClientAuth   *next;
    XdmAuthKeyRec	    rho;
    char		    client[6];
    long		    time;
} XdmClientAuthRec, *XdmClientAuthPtr;

static XdmClientAuthPtr    xdmClients;
static long	    clockOffset;
static Bool	    gotClock;

#define TwentyMinutes	(20 * 60)
#define TwentyFiveMinutes (25 * 60)

static Bool
XdmClientAuthCompare (XdmClientAuthPtr a, XdmClientAuthPtr b)
{
    int	i;

    if (!XdmcpCompareKeys (&a->rho, &b->rho))
	return FALSE;
    for (i = 0; i < 6; i++)
	if (a->client[i] != b->client[i])
	    return FALSE;
    return a->time == b->time;
}

static void
XdmClientAuthDecode (unsigned char *plain, XdmClientAuthPtr auth)
{
    int	    i, j;

    j = 0;
    for (i = 0; i < 8; i++)
    {
	auth->rho.data[i] = plain[j];
	++j;
    }
    for (i = 0; i < 6; i++)
    {
	auth->client[i] = plain[j];
	++j;
    }
    auth->time = 0;
    for (i = 0; i < 4; i++)
    {
	auth->time |= plain[j] << ((3 - i) << 3);
	j++;
    }
}

static void
XdmClientAuthTimeout (long now)
{
    XdmClientAuthPtr	client, next, prev;

    prev = 0;
    for (client = xdmClients; client; client=next)
    {
	next = client->next;
	if (abs (now - client->time) > TwentyFiveMinutes)
	{
	    if (prev)
		prev->next = next;
	    else
		xdmClients = next;
	    xfree (client);
	}
	else
	    prev = client;
    }
}

static XdmClientAuthPtr
XdmAuthorizationValidate (unsigned char *plain, int length, 
    XdmAuthKeyPtr rho, ClientPtr xclient, char **reason)
{
    XdmClientAuthPtr	client, existing;
    long		now;
    int			i;

    if (length != (192 / 8)) {
	if (reason)
	    *reason = "Bad XDM authorization key length";
	return NULL;
    }
    client = (XdmClientAuthPtr) xalloc (sizeof (XdmClientAuthRec));
    if (!client)
	return NULL;
    XdmClientAuthDecode (plain, client);
    if (!XdmcpCompareKeys (&client->rho, rho))
    {
	xfree (client);
	if (reason)
	    *reason = "Invalid XDM-AUTHORIZATION-1 key (failed key comparison)";
	return NULL;
    }
    for (i = 18; i < 24; i++)
	if (plain[i] != 0) {
	    xfree (client);
	    if (reason)
		*reason = "Invalid XDM-AUTHORIZATION-1 key (failed NULL check)";
	    return NULL;
	}
    if (xclient) {
	int family, addr_len;
	Xtransaddr *addr;

	if (_XSERVTransGetPeerAddr(((OsCommPtr)xclient->osPrivate)->trans_conn,
				   &family, &addr_len, &addr) == 0
	    && _XSERVTransConvertAddress(&family, &addr_len, &addr) == 0) {
#if defined(TCPCONN) || defined(STREAMSCONN)
	    if (family == FamilyInternet &&
		memcmp((char *)addr, client->client, 4) != 0) {
		xfree (client);
		xfree (addr);
		if (reason)
		    *reason = "Invalid XDM-AUTHORIZATION-1 key (failed address comparison)";
		return NULL;

	    }
#endif
	    xfree (addr);
	}
    }
    now = time(0);
    if (!gotClock)
    {
	clockOffset = client->time - now;
	gotClock = TRUE;
    }
    now += clockOffset;
    XdmClientAuthTimeout (now);
    if (abs (client->time - now) > TwentyMinutes)
    {
	xfree (client);
	if (reason)
	    *reason = "Excessive XDM-AUTHORIZATION-1 time offset";
	return NULL;
    }
    for (existing = xdmClients; existing; existing=existing->next)
    {
	if (XdmClientAuthCompare (existing, client))
	{
	    xfree (client);
	    if (reason)
		*reason = "XDM authorization key matches an existing client!";
	    return NULL;
	}
    }
    return client;
}

int
XdmAddCookie (unsigned short data_length, char *data, XID id)
{
    XdmAuthorizationPtr	new;
    unsigned char	*rho_bits, *key_bits;

    switch (data_length)
    {
    case 16:		    /* auth from files is 16 bytes long */
#ifdef XDMCP
	if (authFromXDMCP)
	{
	    /* R5 xdm sent bogus authorization data in the accept packet,
	     * but we can recover */
	    rho_bits = rho.data;
	    key_bits = (unsigned char *) data;
	    key_bits[0] = '\0';
	}
	else
#endif
	{
	    rho_bits = (unsigned char *) data;
	    key_bits = (unsigned char *) (data + 8);
	}
	break;
#ifdef XDMCP
    case 8:		    /* auth from XDMCP is 8 bytes long */
	rho_bits = rho.data;
	key_bits = (unsigned char *) data;
	break;
#endif
    default:
	return 0;
    }
    /* the first octet of the key must be zero */
    if (key_bits[0] != '\0')
	return 0;
    new = (XdmAuthorizationPtr) xalloc (sizeof (XdmAuthorizationRec));
    if (!new)
	return 0;
    new->next = xdmAuth;
    xdmAuth = new;
    memmove (new->key.data, key_bits, (int) 8);
    memmove (new->rho.data, rho_bits, (int) 8);
    new->id = id;
    return 1;
}

XID
XdmCheckCookie (unsigned short cookie_length, char *cookie, 
    ClientPtr xclient, char **reason)
{
    XdmAuthorizationPtr	auth;
    XdmClientAuthPtr	client;
    unsigned char	*plain;

    /* Auth packets must be a multiple of 8 bytes long */
    if (cookie_length & 7)
	return (XID) -1;
    plain = (unsigned char *) xalloc (cookie_length);
    if (!plain)
	return (XID) -1;
    for (auth = xdmAuth; auth; auth=auth->next) {
	XdmcpUnwrap (cookie, (unsigned char *)&auth->key, plain, cookie_length);
	if ((client = XdmAuthorizationValidate (plain, cookie_length, &auth->rho, xclient, reason)) != NULL)
	{
	    client->next = xdmClients;
	    xdmClients = client;
	    xfree (plain);
	    return auth->id;
	}
    }
    xfree (plain);
    return (XID) -1;
}

int
XdmResetCookie (void)
{
    XdmAuthorizationPtr	auth, next_auth;
    XdmClientAuthPtr	client, next_client;

    for (auth = xdmAuth; auth; auth=next_auth)
    {
	next_auth = auth->next;
	xfree (auth);
    }
    xdmAuth = 0;
    for (client = xdmClients; client; client=next_client)
    {
	next_client = client->next;
	xfree (client);
    }
    xdmClients = (XdmClientAuthPtr) 0;
    return 1;
}

XID
XdmToID (unsigned short cookie_length, char *cookie)
{
    XdmAuthorizationPtr	auth;
    XdmClientAuthPtr	client;
    unsigned char	*plain;

    plain = (unsigned char *) xalloc (cookie_length);
    if (!plain)
	return (XID) -1;
    for (auth = xdmAuth; auth; auth=auth->next) {
	XdmcpUnwrap (cookie, (unsigned char *)&auth->key, plain, cookie_length);
	if ((client = XdmAuthorizationValidate (plain, cookie_length, &auth->rho, NULL, NULL)) != NULL)
	{
	    xfree (client);
	    xfree (cookie);
	    return auth->id;
	}
    }
    xfree (cookie);
    return (XID) -1;
}

int
XdmFromID (XID id, unsigned short *data_lenp, char **datap)
{
    XdmAuthorizationPtr	auth;

    for (auth = xdmAuth; auth; auth=auth->next) {
	if (id == auth->id) {
	    *data_lenp = 16;
	    *datap = (char *) &auth->rho;
	    return 1;
	}
    }
    return 0;
}

int
XdmRemoveCookie (unsigned short data_length, char *data)
{
    XdmAuthorizationPtr	auth, prev;
    XdmAuthKeyPtr	key_bits, rho_bits;

    prev = 0;
    switch (data_length)
    {
    case 16:
	rho_bits = (XdmAuthKeyPtr) data;
	key_bits = (XdmAuthKeyPtr) (data + 8);
	break;
#ifdef XDMCP
    case 8:
	rho_bits = &rho;
	key_bits = (XdmAuthKeyPtr) data;
	break;
#endif
    default:
	return 0;
    }
    for (auth = xdmAuth; auth; auth=auth->next) {
	if (XdmcpCompareKeys (rho_bits, &auth->rho) &&
	    XdmcpCompareKeys (key_bits, &auth->key))
 	{
	    if (prev)
		prev->next = auth->next;
	    else
		xdmAuth = auth->next;
	    xfree (auth);
	    return 1;
	}
    }
    return 0;
}

#endif
