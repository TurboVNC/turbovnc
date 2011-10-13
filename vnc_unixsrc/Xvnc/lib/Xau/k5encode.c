/* $Xorg: k5encode.c,v 1.4 2001/02/09 02:03:42 xorgcvs Exp $ */

/*

Copyright 1993, 1994, 1998  The Open Group

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

*/

/*
 * functions to encode/decode Kerberos V5 principals
 * into something that can be reasonable spewed over
 * the wire
 *
 * Author: Tom Yu <tlyu@MIT.EDU>
 *
 * Still needs to be fixed up wrt signed/unsigned lengths, but we'll worry
 * about that later.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <krb5/krb5.h>
/* 9/93: krb5.h leaks some symbols */
#undef BITS32
#undef xfree

#include <X11/X.h>
#include <X11/Xos.h>
#include <X11/Xmd.h>
#include <X11/Xfuncs.h>

/*
 * XauKrb5Encode
 *
 * this function encodes the principal passed to it in a format that can
 * easily be dealt with by stuffing it into an X packet.  Encoding is as
 * follows:
 *   length count of the realm name
 *   realm
 *   component count
 *   length of component
 *   actual principal component
 *   etc....
 *
 * Note that this function allocates a hunk of memory, which must be
 * freed to avoid nasty memory leak type things.  All counts are
 * byte-swapped if needed. (except for the total length returned)
 *
 * nevermind.... stuffing the encoded packet in net byte order just to
 * always do the right thing.  Don't have to frob with alignment that way.
 */
int
XauKrb5Encode(princ, outbuf)
    krb5_principal princ;	/* principal to encode */
    krb5_data *outbuf;		/* output buffer */
{
    CARD16 i, numparts, totlen = 0, plen, rlen;
    char *cp, *pdata;

    rlen = krb5_princ_realm(princ)->length;
    numparts = krb5_princ_size(princ);
    totlen = 2 + rlen + 2;	/* include room for realm length
				   and component count */
    for (i = 0; i < numparts; i++)
	totlen += krb5_princ_component(princ, i)->length + 2;
    /* add 2 bytes each time for length */
    if ((outbuf->data = (char *)malloc(totlen)) == NULL)
	return -1;
    cp = outbuf->data;
    *cp++ = (char)((int)(0xff00 & rlen) >> 8);
    *cp++ = (char)(0x00ff & rlen);
    memcpy(cp, krb5_princ_realm(princ)->data, rlen);
    cp += rlen;
    *cp++ = (char)((int)(0xff00 & numparts) >> 8);
    *cp++ = (char)(0x00ff & numparts);
    for (i = 0; i < numparts; i++)
    {
	plen = krb5_princ_component(princ, i)->length;
	pdata = krb5_princ_component(princ, i)->data;
	*cp++ = (char)((int)(0xff00 & plen) >> 8);
	*cp++ = (char)(0x00ff & plen);
	memcpy(cp, pdata, plen);
	cp += plen;
    }
    outbuf->length = totlen;
    return 0;
}

/*
 * XauKrb5Decode
 *
 * This function essentially reverses what XauKrb5Encode does.
 * return value: 0 if okay, -1 if malloc fails, -2 if inbuf format bad
 */
int
XauKrb5Decode(inbuf, princ)
    krb5_data inbuf;
    krb5_principal *princ;
{
    CARD16 i, numparts, plen, rlen;
    CARD8 *cp, *pdata;
    
    if (inbuf.length < 4)
    {
	return -2;
    }
    *princ = (krb5_principal)malloc(sizeof (krb5_principal_data));
    if (*princ == NULL)
	return -1;
    bzero(*princ, sizeof (krb5_principal_data));
    cp = (CARD8 *)inbuf.data;
    rlen = *cp++ << 8;
    rlen |= *cp++;
    if (inbuf.length < 4 + (int)rlen + 2)
    {
	krb5_free_principal(*princ);
	return -2;
    }
    krb5_princ_realm(*princ)->data = (char *)malloc(rlen);
    if (krb5_princ_realm(*princ)->data == NULL)
    {
	krb5_free_principal(*princ);
	return -1;
    }
    krb5_princ_realm(*princ)->length = rlen;
    memcpy(krb5_princ_realm(*princ)->data, cp, rlen);
    cp += rlen;
    numparts = *cp++ << 8;
    numparts |= *cp++;
    krb5_princ_name(*princ) =
	(krb5_data *)malloc(numparts * sizeof (krb5_data));
    if (krb5_princ_name(*princ) == NULL)
    {
	krb5_free_principal(*princ);
	return -1;
    }
    krb5_princ_size(*princ) = 0;
    for (i = 0; i < numparts; i++)
    {
	if (cp + 2 > (CARD8 *)inbuf.data + inbuf.length)
	{
	    krb5_free_principal(*princ);
	    return -2;
	}
	plen = *cp++ << 8;
	plen |= *cp++;
	if (cp + plen > (CARD8 *)inbuf.data + inbuf.length)
	{
	    krb5_free_principal(*princ);
	    return -2;
	}
	pdata = (CARD8 *)malloc(plen);
	if (pdata == NULL)
	{
	    krb5_free_principal(*princ);
	    return -1;
	}
	krb5_princ_component(*princ, i)->data = (char *)pdata;
	krb5_princ_component(*princ, i)->length = plen;
	memcpy(pdata, cp, plen);
	cp += plen;
	krb5_princ_size(*princ)++;
    }
    return 0;
}
