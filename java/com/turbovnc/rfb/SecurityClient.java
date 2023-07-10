/* Copyright (C) 2012, 2015-2016, 2018, 2020-2022 D. R. Commander.
 *                                                All Rights Reserved.
 * Copyright (C) 2011-2012 Brian P. Hinz
 * Copyright (C) 2010 TigerVNC Team
 * Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

package com.turbovnc.rfb;

import com.turbovnc.rdr.ErrorException;

public class SecurityClient {

  public CSecurity getCSecurity(Params params, int secType) {
    assert (msg != null);

    if (!params.secTypes.isSupported(secType))
      throw new ErrorException("Security type not supported");

    switch (secType) {
      case RFB.SECTYPE_NONE:      return (new CSecurityNone());
      case RFB.SECTYPE_VNCAUTH:   return (new CSecurityVncAuth());
      case RFB.SECTYPE_TIGHT:     return (new CSecurityTight(this));
      case RFB.SECTYPE_TLS:       return (new CSecurityRFBTLS(this));
      case RFB.SECTYPE_VENCRYPT:  return (new CSecurityVeNCrypt(this));
      case RFB.SECTYPE_PLAIN:     return (new CSecurityPlain());
      case RFB.SECTYPE_TLS_NONE:
        return (new CSecurityStack(RFB.SECTYPE_TLS_NONE, "TLSNone",
                new CSecurityTLS(true), null));
      case RFB.SECTYPE_TLS_VNC:
        return (new CSecurityStack(RFB.SECTYPE_TLS_VNC, "TLSVnc",
                new CSecurityTLS(true), new CSecurityVncAuth()));
      case RFB.SECTYPE_TLS_PLAIN:
        return (new CSecurityStack(RFB.SECTYPE_TLS_PLAIN, "TLSPlain",
                new CSecurityTLS(true), new CSecurityPlain()));
      case RFB.SECTYPE_X509_NONE:
        return (new CSecurityStack(RFB.SECTYPE_X509_NONE, "X509None",
                new CSecurityTLS(false), null));
      case RFB.SECTYPE_X509_VNC:
        return (new CSecurityStack(RFB.SECTYPE_X509_VNC, "X509Vnc",
                new CSecurityTLS(false), new CSecurityVncAuth()));
      case RFB.SECTYPE_X509_PLAIN:
        return (new CSecurityStack(RFB.SECTYPE_X509_PLAIN, "X509Plain",
                new CSecurityTLS(false), new CSecurityPlain()));
      default:
        throw new ErrorException("Security type not supported");
    }

  }

  String msg = null;
}
