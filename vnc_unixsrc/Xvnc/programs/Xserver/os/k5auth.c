/* $XConsortium: k5auth.c,v 1.9 95/04/06 16:10:29 mor Exp $ */
/* $XFree86: xc/programs/Xserver/os/k5auth.c,v 3.2 1996/05/10 07:02:15 dawes Exp $ */
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

/*
 * Kerberos V5 authentication scheme
 * Author: Tom Yu <tlyu@MIT.EDU>
 *
 * Mostly snarfed wholesale from the user_user demo in the
 * krb5 distribution. (At least the checking part)
 */

#include <sys/types.h>
#include <sys/socket.h>
#ifdef TCPCONN
#include <netinet/in.h>
#endif
#ifdef DNETCONN
#include <netdnet/dn.h>
#endif
#include <arpa/inet.h>
#include <krb5/krb5.h>
/* 9/93: krb5.h leaks some symbols */
#undef BITS32
#undef xfree
#include <krb5/los-proto.h>
#include "X.h"
#include "os.h"
#include "osdep.h"
#include "Xproto.h"
#include "Xfuncs.h"
#include "dixstruct.h"
#include <com_err.h>
#include "Xauth.h"

extern int (*k5_Vector[256])();
extern int SendConnSetup();
extern char *display;		/* need this to generate rcache name */

static XID krb5_id = ~0L;
static krb5_principal srvname = NULL; /* service name */
static char *ccname = NULL;
static char *ktname = NULL;	/* key table name */
static char kerror[256];

/*
 * tgt_keyproc:
 *
 * extract session key from a credentials struct
 */
krb5_error_code tgt_keyproc(keyprocarg, principal, vno, key)
    krb5_pointer keyprocarg;
    krb5_principal principal;
    krb5_kvno vno;
    krb5_keyblock **key;
{
    krb5_creds *creds = (krb5_creds *)keyprocarg;
    
    return krb5_copy_keyblock(&creds->keyblock, key);
}

/*
 * k5_cmpenc:
 *
 * compare "encoded" principals
 */
Bool k5_cmpenc(pname, plen, buf)
    unsigned char *pname;
    short plen;
    krb5_data *buf;
{
    return (plen == buf->length &&
	    memcmp(pname, buf->data, plen) == 0);
}

/*
 * K5Check:
 *
 * This is stage 0 of the krb5 authentication protocol.  It
 * goes through the current credentials cache and extracts the
 * primary principal and tgt to send to the client, or as
 * appropriate, extracts from a keytab.
 *
 * The packet sent to the client has the following format:
 *
 * CARD8	reqType	= 2
 * CARD8	data	= 0
 * CARD16	length	= total length of packet (in 32 bit units)
 * CARD16	plen	= length of encoded principal following
 * STRING8	princ	= encoded principal
 * STRING8	ticket	= server tgt
 *
 * For client-server authentication, the packet is as follows:
 *
 * CARD8	reqType	= 3
 * CARD8	data	= 0
 * CARD16	length	= total length
 * STRING8	princ	= encoded principal of server
 */
XID K5Check(data_length, data, client, reason)
    unsigned short data_length;
    char *data;
    ClientPtr client;
    char **reason;
{
    krb5_error_code retval;
    CARD16 tlen;
    krb5_principal sprinc, cprinc;
    krb5_ccache cc;
    krb5_creds *creds;
    char *outbuf, *cp;
    krb5_data princ;
    register char n;
    xReq prefix;

    if (krb5_id == ~0L)
	return ~0L;
    if (!ccname && !srvname)
	return ~0L;
    if (ccname)
    {
	if ((creds = (krb5_creds *)malloc(sizeof(krb5_creds))) == NULL)
	    return ~0L;
	if (retval = krb5_cc_resolve(ccname, &cc))
	    return ~0L;
	bzero((char*)creds, sizeof (krb5_creds));
	if (retval = krb5_cc_get_principal(cc, &cprinc))
	{
	    krb5_free_creds(creds);
	    krb5_cc_close(cc);
	    return ~0L;
	}
	creds->client = cprinc;
	if (retval =
	    krb5_build_principal_ext(&sprinc, 
				     krb5_princ_realm(creds->client)->length,
				     krb5_princ_realm(creds->client)->data,
				     6, "krbtgt",
				     krb5_princ_realm(creds->client)->length,
				     krb5_princ_realm(creds->client)->data,
				     0))
	{
	    krb5_free_creds(creds);
	    krb5_cc_close(cc);
	    return ~0L;
	}
	creds->server = sprinc;
	retval = krb5_get_credentials(KRB5_GC_CACHED, cc, creds);
	krb5_cc_close(cc);
	if (retval)
	{
	    krb5_free_creds(creds);
	    return ~0L;
	}
	if (retval = XauKrb5Encode(cprinc, &princ))
	{
	    krb5_free_creds(creds);
	    return ~0L;
	}
	tlen = sz_xReq + 2 + princ.length + creds->ticket.length;
	prefix.reqType = 2;	/* opcode = authenticate user-to-user */
    }
    else if (srvname)
    {
	if (retval = XauKrb5Encode(srvname, &princ))
	{
	    return ~0L;
	}
	tlen = sz_xReq + princ.length;
	prefix.reqType = 3;	/* opcode = authenticate client-server */
    }
    prefix.data = 0;		/* stage = 0 */
    prefix.length = (tlen + 3) >> 2; /* round up to nearest multiple
					of 4 bytes */
    if (client->swapped)
    {
	swaps(&prefix.length, n);
    }
    if ((cp = outbuf = (char *)malloc(tlen)) == NULL)
    {
	if (ccname)
	{
	    krb5_free_creds(creds);
	}
	free(princ.data);
	return ~0L;
    }
    memcpy(cp, &prefix, sz_xReq);
    cp += sz_xReq;
    if (ccname)
    {
	memcpy(cp, &princ.length, 2);
	if (client->swapped)
	{
	    swaps((CARD16 *)cp, n);
	}
	cp += 2;
    }
    memcpy(cp, princ.data, princ.length);
    cp += princ.length;
    free(princ.data);		/* we don't need that anymore */
    if (ccname)
	memcpy(cp, creds->ticket.data, creds->ticket.length);
    WriteToClient(client, tlen, outbuf);
    free(outbuf);
    client->requestVector = k5_Vector; /* hack in our dispatch vector */
    client->clientState = ClientStateAuthenticating;
    if (ccname)
    {
	((OsCommPtr)client->osPrivate)->authstate.srvcreds = (pointer)creds; /* save tgt creds */
	((OsCommPtr)client->osPrivate)->authstate.ktname = NULL;
	((OsCommPtr)client->osPrivate)->authstate.srvname = NULL;
    }
    if (srvname)
    {
	((OsCommPtr)client->osPrivate)->authstate.srvcreds = NULL;
	((OsCommPtr)client->osPrivate)->authstate.ktname = (pointer)ktname;
	((OsCommPtr)client->osPrivate)->authstate.srvname = (pointer)srvname;
    }
    ((OsCommPtr)client->osPrivate)->authstate.stageno = 1; /* next stage is 1 */
    return krb5_id;
}

/*
 * k5_stage1:
 *
 * This gets called out of the dispatcher after K5Check frobs with the
 * client->requestVector.  It accepts the ap_req from the client and verifies
 * it.  In addition, if the client has set AP_OPTS_MUTUAL_REQUIRED, it then
 * sends an ap_rep to the client to achieve mutual authentication.
 *
 * client stage1 packet format is as follows:
 *
 * CARD8	reqType	= 1
 * CARD8	data	= ignored
 * CARD16	length	= total length
 * STRING8	data	= the actual ap_req
 *
 * stage2 packet sent back to client for mutual authentication:
 *
 * CARD8	reqType	= 2
 * CARD8	data	= 2
 * CARD16	length	= total length
 * STRING8	data	= the ap_rep
 */
int k5_stage1(client)
    register ClientPtr client;
{
    long addrlen;
    krb5_error_code retval, retval2;
    register char n;
    struct sockaddr cli_net_addr;
    xReq prefix;
    krb5_principal cprinc;
    krb5_data buf;
    krb5_creds *creds = (krb5_creds *)((OsCommPtr)client->osPrivate)->authstate.srvcreds;
    krb5_keyblock *skey;
    krb5_address cli_addr, **localaddrs = NULL;
    krb5_tkt_authent *authdat;
    krb5_ap_rep_enc_part rep;
    krb5_int32 ctime, cusec;
    krb5_rcache rcache = NULL;
    char *cachename = NULL, *rc_type = NULL, *rc_base = "rcX", *kt = NULL;
    REQUEST(xReq);

    if (((OsCommPtr)client->osPrivate)->authstate.stageno != 1)
    {
	if (creds)
	    krb5_free_creds(creds);
	return(SendConnSetup(client, "expected Krb5 stage1 packet"));
    }
    addrlen = sizeof (cli_net_addr);
    if (getpeername(((OsCommPtr)client->osPrivate)->fd,
		    &cli_net_addr, &addrlen) == -1)
    {
	if (creds)
	    krb5_free_creds(creds);
	return(SendConnSetup(client, "Krb5 stage1: getpeername failed"));
    }
    if (cli_net_addr.sa_family == AF_UNSPEC
#if defined(UNIXCONN) || defined(LOCALCONN) || defined(OS2PIPECONN)
	|| cli_net_addr.sa_family == AF_UNIX
#endif
	)			/* assume local host */
    {
	krb5_os_localaddr(&localaddrs);
	if (!localaddrs || !localaddrs[0])
	{
	    if (creds)
		krb5_free_creds(creds);
	    return(SendConnSetup(client, "Krb5 failed to get localaddrs"));
	}
	cli_addr.addrtype = localaddrs[0]->addrtype;
	cli_addr.length = localaddrs[0]->length;
	cli_addr.contents = localaddrs[0]->contents;
    }
    else
    {
	cli_addr.addrtype = cli_net_addr.sa_family; /* the values
						       are compatible */
	switch (cli_net_addr.sa_family)
	{
#ifdef TCPCONN
	case AF_INET:
	    cli_addr.length = sizeof (struct in_addr);
	    cli_addr.contents =
		(krb5_octet *)&((struct sockaddr_in *)&cli_net_addr)->sin_addr;
	    break;
#endif
#ifdef DNETCONN
	case AF_DECnet:
	    cli_addr.length = sizeof (struct dn_naddr);
	    cli_addr.contents =
		(krb5_octet *)&((struct sockaddr_dn *)&cli_net_addr)->sdn_add;
	    break;
#endif
	default:
	    if (localaddrs)
		krb5_free_addresses(localaddrs);
	    if (creds)
		krb5_free_creds(creds);
	    sprintf(kerror, "Krb5 stage1: unknown address family %d from getpeername",
		    cli_net_addr.sa_family);    
	    return(SendConnSetup(client, kerror));
	}
    }
    if ((rcache = (krb5_rcache)malloc(sizeof(*rcache))) == NULL)
    {
	if (localaddrs)
	    krb5_free_addresses(localaddrs);
	if (creds)
	    krb5_free_creds(creds);
	return(SendConnSetup(client, "malloc bombed for krb5_rcache"));
    }
    if ((rc_type = krb5_rc_default_type()) == NULL)
	rc_type = "dfl";
    if (retval = krb5_rc_resolve_type(&rcache, rc_type))
    {
	if (localaddrs)
	    krb5_free_addresses(localaddrs);
	if (creds)
	    krb5_free_creds(creds);
	free(rcache);
	strcpy(kerror, "krb5_rc_resolve_type failed: ");
	strncat(kerror, error_message(retval), 231);
	return(SendConnSetup(client, kerror));
    }
    if ((cachename = (char *)malloc(strlen(rc_base) + strlen(display) + 1))
	== NULL)
    {
	if (localaddrs)
	    krb5_free_addresses(localaddrs);
	if (creds)
	    krb5_free_creds(creds);
	free(rcache);
	return(SendConnSetup(client, "Krb5: malloc bombed for cachename"));
    }
    strcpy(cachename, rc_base);
    strcat(cachename, display);
    if (retval = krb5_rc_resolve(rcache, cachename))
    {
	if (localaddrs)
	    krb5_free_addresses(localaddrs);
	if (creds)
	    krb5_free_creds(creds);
	free(rcache);
	free(cachename);
	strcpy(kerror, "krb5_rc_resolve failed: ");
	strncat(kerror, error_message(retval), 236);
	return(SendConnSetup(client, kerror));
    }
    free(cachename);
    if (krb5_rc_recover(rcache))
    {
	extern krb5_deltat krb5_clockskew;
	if (retval = krb5_rc_initialize(rcache, krb5_clockskew))
	{
	    if (localaddrs)
		krb5_free_addresses(localaddrs);
	    if (creds)
		krb5_free_creds(creds);
	    if (retval2 = krb5_rc_close(rcache))
	    {
		strcpy(kerror, "krb5_rc_close failed: ");
		strncat(kerror, error_message(retval2), 238);
		return(SendConnSetup(client, kerror));
	    }
	    free(rcache);
	    strcpy(kerror, "krb5_rc_initialize failed: ");
	    strncat(kerror, error_message(retval), 233);
	    return(SendConnSetup(client, kerror));
	}
    }
    buf.length = (stuff->length << 2) - sz_xReq;
    buf.data = (char *)stuff + sz_xReq;
    if (creds)
    {
	retval = krb5_rd_req(&buf,
			     NULL, /* don't bother with server name */
			     &cli_addr,
			     NULL, /* no fetchfrom */
			     tgt_keyproc,
			     creds, /* credentials as arg to
				       keyproc */
			     rcache,
			     &authdat);
	krb5_free_creds(creds);
    }
    else if (kt = (char *)((OsCommPtr)client->osPrivate)->authstate.ktname)
    {
	retval = krb5_rd_req(&buf, srvname, &cli_addr, kt, NULL, NULL,
			     rcache, &authdat);
	((OsCommPtr)client->osPrivate)->authstate.ktname = NULL;
    }
    else
    {
	if (localaddrs)
	    krb5_free_addresses(localaddrs);
	return(SendConnSetup(client, "Krb5: neither srvcreds nor ktname set"));
    }
    if (localaddrs)
	krb5_free_addresses(localaddrs);
    if (rcache)
    {
	if (retval2 = krb5_rc_close(rcache))
	{
	    strcpy(kerror, "krb5_rc_close failed (2): ");
	    strncat(kerror, error_message(retval2), 230);
	    return(SendConnSetup(client, kerror));
	}
	free(rcache);
    }
    if (retval)
    {
	strcpy(kerror, "Krb5: Bad application request: ");
	strncat(kerror, error_message(retval), 224);
	return(SendConnSetup(client, kerror));
    }
    cprinc = authdat->ticket->enc_part2->client;
    skey = authdat->ticket->enc_part2->session;
    if (XauKrb5Encode(cprinc, &buf))
    {
	krb5_free_tkt_authent(authdat);
	return(SendConnSetup(client, "XauKrb5Encode bombed"));
    }
    /*
     * Now check to see if the principal we got is one that we want to let in
     */
    if (ForEachHostInFamily(FamilyKrb5Principal, k5_cmpenc, (pointer)&buf))
    {
	free(buf.data);
	/*
	 * The following deals with sending an ap_rep to the client to
	 * achieve mutual authentication.  The client sends back a stage 3
	 * packet if all is ok.
	 */
	if (authdat->ap_options | AP_OPTS_MUTUAL_REQUIRED)
	{
	    /*
	     * stage 2: send ap_rep to client
	     */
	    if (retval = krb5_us_timeofday(&ctime, &cusec))
	    {
		krb5_free_tkt_authent(authdat);
		strcpy(kerror, "error in krb5_us_timeofday: ");
		strncat(kerror, error_message(retval), 234);
		return(SendConnSetup(client, kerror));
	    }
	    rep.ctime = ctime;
	    rep.cusec = cusec;
	    rep.subkey = NULL;
	    rep.seq_number = 0;
	    if (retval = krb5_mk_rep(&rep, skey, &buf))
	    {
		krb5_free_tkt_authent(authdat);
		strcpy(kerror, "error in krb5_mk_rep: ");
		strncat(kerror, error_message(retval), 238);
		return(SendConnSetup(client, kerror));
	    }
	    prefix.reqType = 2;	/* opcode = authenticate */
	    prefix.data = 2;	/* stage = 2 */
	    prefix.length = (buf.length + sz_xReq + 3) >> 2;
	    if (client->swapped)
	    {
		swaps(&prefix.length, n);
	    }
	    WriteToClient(client, sz_xReq, (char *)&prefix);
	    WriteToClient(client, buf.length, buf.data);
	    free(buf.data);
	    krb5_free_tkt_authent(authdat);
	    ((OsCommPtr)client->osPrivate)->authstate.stageno = 3; /* expect stage3 packet */
	    return(Success);
	}
	else
	{
	    free(buf.data);
	    krb5_free_tkt_authent(authdat);
	    return(SendConnSetup(client, NULL)); /* success! */
	}
    }
    else
    {
	char *kname;
	
	krb5_free_tkt_authent(authdat);
	free(buf.data);
	retval = krb5_unparse_name(cprinc, &kname);
	if (retval == 0)
	{
	    sprintf(kerror, "Principal \"%s\" is not authorized to connect",
		    kname);
	    if (kname)
		free(kname);
	    return(SendConnSetup(client, kerror));
	}
	else
	    return(SendConnSetup(client,"Principal is not authorized to connect to Server"));
    }
}

/*
 * k5_stage3:
 *
 * Get the short ack packet from the client.  This packet can conceivably
 * be expanded to allow for switching on end-to-end encryption.
 *
 * stage3 packet format:
 *
 * CARD8	reqType	= 3
 * CARD8	data	= ignored (for now)
 * CARD16	length	= should be zero
 */
int k5_stage3(client)
    register ClientPtr client;
{
    REQUEST(xReq);

    if (((OsCommPtr)client->osPrivate)->authstate.stageno != 3)
    {
	return(SendConnSetup(client, "expected Krb5 stage3 packet"));
    }
    else
	return(SendConnSetup(client, NULL)); /* success! */
}

k5_bad(client)
    register ClientPtr client;
{
    if (((OsCommPtr)client->osPrivate)->authstate.srvcreds)
	krb5_free_creds((krb5_creds *)((OsCommPtr)client->osPrivate)->authstate.srvcreds);
    sprintf(kerror, "unrecognized Krb5 auth packet %d, expecting %d",
	    ((xReq *)client->requestBuffer)->reqType,
	    ((OsCommPtr)client->osPrivate)->authstate.stageno);
    return(SendConnSetup(client, kerror));
}

/*
 * K5Add:
 *
 * Takes the name of a credentials cache and resolves it.  Also adds the
 * primary principal of the ccache to the acl.
 *
 * Now will also take a service name.
 */
int K5Add(data_length, data, id)
    unsigned short data_length;
    char *data;
    XID id;
{
    krb5_principal princ;
    krb5_error_code retval;
    krb5_keytab_entry tmp_entry;
    krb5_keytab keytab;
    krb5_kvno kvno = 0;
    krb5_ccache cc;
    char *nbuf, *cp;
    krb5_data kbuf;
    int i, ktlen;
    
    krb5_init_ets();		/* can't think of a better place to put it */
    krb5_id = ~0L;
    if (data_length < 3)
	return 0;
    if ((nbuf = (char *)malloc(data_length - 2)) == NULL)
	return 0;
    memcpy(nbuf, data + 3, data_length - 3);
    nbuf[data_length - 3] = '\0';
    if (ccname)
    {
	free(ccname);
	ccname = NULL;
    }
    if (srvname)
    {
	krb5_free_principal(srvname);
	srvname = NULL;
    }
    if (ktname)
    {
	free(ktname);
	ktname = NULL;
    }
    if (!strncmp(data, "UU:", 3))
    {
	if (retval = krb5_cc_resolve(nbuf, &cc))
	{
	    ErrorF("K5Add: krb5_cc_resolve of \"%s\" failed: %s\n",
		   nbuf, error_message(retval));
	    free(nbuf);
	    return 0;
	}
	if (cc && !(retval = krb5_cc_get_principal(cc, &princ)))
	{
	    if (XauKrb5Encode(princ, &kbuf))
	    {
		free(nbuf);
		krb5_free_principal(princ);
		krb5_cc_close(cc);
		return 0;
	    }
	    if (krb5_cc_close(cc))
		return 0;
	    AddHost(NULL, FamilyKrb5Principal, kbuf.length, kbuf.data);
	    krb5_free_principal(princ);
	    free(kbuf.data);
	    ccname = nbuf;
	    krb5_id = id;
	    return 1;
	}
	else
	{
	    ErrorF("K5Add: getting principal from cache \"%s\" failed: %s\n",
		   nbuf, error_message(retval));
	}
    }
    else if (!strncmp(data, "CS:", 3))
    {
	if ((cp = strchr(nbuf, ',')) == NULL)
	{
	    free(nbuf);
	    return 0;
	}
	*cp = '\0';		/* gross but it works :-) */
	ktlen = strlen(cp + 1);
	if ((ktname = (char *)malloc(ktlen + 1)) == NULL)
	{
	    free(nbuf);
	    return 0;
	}
	strcpy(ktname, cp + 1);
	retval = krb5_sname_to_principal(NULL, /* NULL for hostname uses
						  local host name*/
					 nbuf, KRB5_NT_SRV_HST,
					 &srvname);
	free(nbuf);
	if (retval)
	{
	    free(ktname);
	    ktname = NULL;
	    return 0;
	}
	if (retval = krb5_kt_resolve(ktname, &keytab))
	{
	    free(ktname);
	    ktname = NULL;
	    krb5_free_principal(srvname);
	    srvname = NULL;
	    return 0;
	}
	retval = krb5_kt_get_entry(keytab, srvname, kvno, &tmp_entry);
	krb5_kt_free_entry(&tmp_entry);
	if (retval)
	{
	    free(ktname);
	    ktname = NULL;
	    krb5_free_principal(srvname);
	    srvname = NULL;
	    return 0;
	}
	if (XauKrb5Encode(srvname, &kbuf))
	{
	    free(ktname);
	    ktname = NULL;
	    krb5_free_principal(srvname);
	    srvname = NULL;
	    return 0;
	}
	AddHost(NULL, FamilyKrb5Principal, kbuf.length, kbuf.data);
	krb5_id = id;
	return 1;
    }
    else
    {
	ErrorF("K5Add: credentials cache name \"%.*s\" in auth file: unknown type\n",
	       data_length, data);
    }
    return 0;
}

/*
 * K5Reset:
 *
 * Reset krb5_id, also nuke the current principal from the acl.
 */
int K5Reset()
{
    krb5_principal princ;
    krb5_error_code retval;
    krb5_ccache cc;
    krb5_data kbuf;
    int i;
    
    if (ccname)
    {
	if (retval = krb5_cc_resolve(ccname, &cc))
	{
	    free(ccname);
	    ccname = NULL;
	}
	if (cc && !(retval = krb5_cc_get_principal(cc, &princ)))
	{
	    if (XauKrb5Encode(princ, &kbuf))
		return 1;
	    RemoveHost(NULL, FamilyKrb5Principal, kbuf.length, kbuf.data);
	    krb5_free_principal(princ);
	    free(kbuf.data);
	    if (krb5_cc_close(cc))
		return 1;
	    free(ccname);
	    ccname = NULL;
	}
    }
    if (srvname)
    {
	if (XauKrb5Encode(srvname, &kbuf))
	    return 1;
	RemoveHost(NULL, FamilyKrb5Principal, kbuf.length, kbuf.data);
	krb5_free_principal(srvname);
	free(kbuf.data);
	srvname = NULL;
    }
    if (ktname)
    {
	free(ktname);
	ktname = NULL;
    }
    krb5_id = ~0L;
    return 0;
}

XID K5ToID(data_length, data)
    unsigned short data_length;
    char *data;
{
    return krb5_id;
}

int K5FromID(id, data_lenp, datap)
    XID id;
    unsigned short *data_lenp;
    char **datap;
{
    return 0;
}

int K5Remove(data_length, data)
    unsigned short data_length;
    char *data;
{
    return 0;
}
