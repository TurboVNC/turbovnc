/* $Xorg: XimTrX.h,v 1.3 2000/08/17 19:45:05 cpqbld Exp $ */
/******************************************************************

           Copyright 1992 by Sun Microsystems, Inc.
           Copyright 1992, 1993, 1994 by FUJITSU LIMITED

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of Sun Microsystems, Inc.
and FUJITSU LIMITED not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Sun Microsystems, Inc. and FUJITSU LIMITED makes no representations about
the suitability of this software for any purpose.
It is provided "as is" without express or implied warranty.

Sun Microsystems Inc. AND FUJITSU LIMITED DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS, IN NO EVENT SHALL Sun Microsystems, Inc. AND FUJITSU LIMITED
BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

  Author: Hideki Hiura (hhiura@Sun.COM) Sun Microsystems, Inc.
          Takashi Fujiwara     FUJITSU LIMITED 
                               fujiwara@a80.tech.yk.fujitsu.co.jp

******************************************************************/
/* $XFree86$ */

#ifndef _XIMTRX_H
#define _XIMTRX_H

typedef struct _XIntrCallbackRec	*XIntrCallbackPtr;

typedef struct _XIntrCallbackRec {
    Bool		 (*func)(
				 Xim, INT16, XPointer, XPointer
);
    XPointer		 call_data;
    XIntrCallbackPtr	 next;
} XIntrCallbackRec ;

typedef struct _XSpecRec {
    XIntrCallbackPtr	 intr_cb;
    Atom		 imconnectid;
    Atom		 improtocolid;
    Atom		 immoredataid;
    Window		 lib_connect_wid;
    Window		 ims_connect_wid;
    XPointer		 ev;
    CARD32		 major_code;
    CARD32		 minor_code;
    CARD32		 BoundarySize;
} XSpecRec;

#define _XIM_PROTOCOL		"_XIM_PROTOCOL"
#define _XIM_XCONNECT		"_XIM_XCONNECT"
#define _XIM_MOREDATA		"_XIM_MOREDATA"

#define MAJOR_TRANSPORT_VERSION         0
#define MINOR_TRANSPORT_VERSION         0

#endif /* _XIMTRX_H */
