/* $Xorg: XimTrans.h,v 1.3 2000/08/17 19:45:05 cpqbld Exp $ */
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
/* $XFree86: xc/lib/X11/XimTrans.h,v 1.3 2003/10/24 15:34:00 tsi Exp $ */

#ifndef _XIMTRANS_H
#define _XIMTRANS_H

typedef struct _TransIntrCallbackRec	*TransIntrCallbackPtr;

typedef struct _TransIntrCallbackRec {
    Bool			(*func)(
					Xim, INT16, XPointer, XPointer
					);
    XPointer			 call_data;
    TransIntrCallbackPtr	 next;
} TransIntrCallbackRec ;

typedef struct {
    TransIntrCallbackPtr	 intr_cb;
    struct _XtransConnInfo 	*trans_conn; /* transport connection object */
    int				 fd;
    char			*address;
    Window			 window;
    Bool			 is_putback;
} TransSpecRec;


/*
 * Prototypes
 */

extern Bool _XimTransRegisterDispatcher(
    Xim		 im,
    Bool	 (*callback)(
			     Xim, INT16, XPointer, XPointer
			     ),
    XPointer	 call_data
);


extern Bool _XimTransIntrCallback(
    Xim		 im,
    Bool	 (*callback)(
			     Xim, INT16, XPointer, XPointer
			     ),
    XPointer	 call_data
);

extern Bool _XimTransCallDispatcher(
    Xim		 im,
    INT16	 len,
    XPointer	 data
);

extern void _XimFreeTransIntrCallback(
    Xim		 im
);

extern Bool _XimTransIntrCallbackCheck(
    Xim		 im,
    INT16	 len,
    XPointer	 data
);

extern Bool _XimTransFilterWaitEvent(
    Display	*d,
    Window	 w,
    XEvent	*ev,
    XPointer	 arg
);

extern void _XimTransInternalConnection(
    Display	*d,
    int		 fd,
    XPointer	 arg
);

extern Bool _XimTransWrite(
    Xim		 im,
    INT16	 len,
    XPointer	 data
);

extern Bool _XimTransRead(
    Xim		 im,
    XPointer	 recv_buf,
    int		 buf_len,
    int		*ret_len
);

extern void _XimTransFlush(
    Xim		 im
);

#endif /* _XIMTRANS__H */
