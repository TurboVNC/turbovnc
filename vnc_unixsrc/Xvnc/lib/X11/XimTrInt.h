/* $Xorg: XimTrInt.h,v 1.3 2000/08/17 19:45:05 cpqbld Exp $ */
/******************************************************************

              Copyright 1992 by Sun Microsystems, Inc.
	      Copyright 1993, 1994 by FUJITSU LIMITED

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of Sun Microsystems, Inc.
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.
Sun Microsystems, Inc. makes no representations about the suitability of
this software for any purpose.  It is provided "as is" without
express or implied warranty.

Sun Microsystems Inc. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL Sun Microsystems, Inc. BE LIABLE FOR ANY SPECIAL, INDIRECT
OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
OR PERFORMANCE OF THIS SOFTWARE.

  Author: Hideki Hiura (hhiura@Sun.COM) Sun Microsystems, Inc.
	  Takashi Fujiwara     FUJITSU LIMITED
			       fujiwara@a80.tech.yk.fujitsu.co.jp

******************************************************************/
/* $XFree86$ */

#ifndef _XIMTRINT_H
#define _XIMTRINT_H

#include "Ximint.h"

typedef struct {
    char	*transportname;
    Bool	 (*config)(
			Xim,
			char *
    );
} TransportSW;

extern TransportSW _XimTransportRec[];

/*
 * Global symbols
 */

extern Bool	_XimXConf(
    Xim		 im,
    char	*address
);

#if defined(TCPCONN) || defined(UNIXCONN) || defined(DNETCONN) || defined(STREAMSCONN)

extern Bool	_XimTransConf(
    Xim		 im,
    char	*address
);

#endif

#endif /* _XIMTRINT_H */
