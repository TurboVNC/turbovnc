/* $XConsortium: securstr.h /main/4 1996/11/12 12:17:47 swick $ */
/*
Copyright (c) 1996  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and sell copies of the Software, and to
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

#ifndef _SECURSTR_H
#define _SECURSTR_H

#include "security.h"

#define SECURITY_EXTENSION_NAME		"SECURITY"
#define SECURITY_MAJOR_VERSION		1
#define SECURITY_MINOR_VERSION		0

#define X_SecurityQueryVersion		0
#define X_SecurityGenerateAuthorization 1
#define X_SecurityRevokeAuthorization   2

typedef struct {
    CARD8       reqType;
    CARD8       securityReqType;
    CARD16      length B16;
    CARD16      majorVersion B16;
    CARD16      minorVersion B16;
} xSecurityQueryVersionReq;
#define sz_xSecurityQueryVersionReq 	8

typedef struct {
    CARD8   type;
    CARD8   pad0;
    CARD16  sequenceNumber B16;
    CARD32  length	 B32;
    CARD16  majorVersion B16;
    CARD16  minorVersion B16;
    CARD32  pad1	 B32;
    CARD32  pad2	 B32;
    CARD32  pad3	 B32;
    CARD32  pad4	 B32;
    CARD32  pad5	 B32;
 } xSecurityQueryVersionReply;
#define sz_xSecurityQueryVersionReply  	32

typedef struct {
    CARD8       reqType;
    CARD8       securityReqType;
    CARD16      length B16;
    CARD16	nbytesAuthProto B16;
    CARD16	nbytesAuthData B16;
    CARD32	valueMask B32; 
    /* auth protocol name padded to 4 bytes */
    /* auth protocol data padded to 4 bytes */
    /* list of CARD32 values, if any */
} xSecurityGenerateAuthorizationReq;
#define sz_xSecurityGenerateAuthorizationReq 12

typedef struct {
    CARD8   type;
    CARD8   pad0;
    CARD16  sequenceNumber B16;
    CARD32  length	 B32;
    CARD32  authId	 B32;
    CARD16  dataLength   B16;
    CARD16  pad1	 B16;
    CARD32  pad2	 B32;
    CARD32  pad3	 B32;
    CARD32  pad4	 B32;
    CARD32  pad5	 B32;
 } xSecurityGenerateAuthorizationReply;
#define sz_xSecurityGenerateAuthorizationReply  	32

typedef struct {
    CARD8       reqType;
    CARD8       securityReqType;
    CARD16      length B16;
    CARD32	authId B32;
} xSecurityRevokeAuthorizationReq;
#define sz_xSecurityRevokeAuthorizationReq 8

typedef struct _xSecurityAuthorizationRevokedEvent {
    BYTE	type;
    BYTE	detail;
    CARD16	sequenceNumber B16;
    CARD32	authId B32;
    CARD32	pad0	 B32;
    CARD32	pad1	 B32;
    CARD32	pad2	 B32;
    CARD32	pad3	 B32;
    CARD32	pad4	 B32;
    CARD32	pad5	 B32;
} xSecurityAuthorizationRevokedEvent;
#define sz_xSecurityAuthorizationRevokedEvent 32

#endif /* _SECURSTR_H */
