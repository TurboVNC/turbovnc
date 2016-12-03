/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2010 TigerVNC Team
 * Copyright (C) 2011-2012 Brian P. Hinz
 * Copyright (C) 2012, 2015-2016 D. R. Commander.  All Rights Reserved.
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

import com.turbovnc.vncviewer.CConn;
import com.turbovnc.rdr.ErrorException;

import com.turbovnc.vncviewer.*;

public class SecurityClient extends Security {

  public SecurityClient() { super(secTypes); }

  public CSecurity getCSecurity(int secType) {
    assert (CConn.upg != null); /* (upg == null) means bug in the viewer */
    assert (msg != null);

    if (!isSupported(secType))
      throw new ErrorException("Security type not supported");

    switch (secType) {
    case Security.secTypeNone: return (new CSecurityNone());
    case Security.secTypeVncAuth: return (new CSecurityVncAuth());
    case Security.secTypeTight: return (new CSecurityTight(this));
    case Security.secTypeTLS: return (new CSecurityRFBTLS(this));
    case Security.secTypeVeNCrypt: return (new CSecurityVeNCrypt(this));
    case Security.secTypePlain: return (new CSecurityPlain());
    case Security.secTypeIdent: return (new CSecurityIdent());
    case Security.secTypeTLSNone:
      return (new CSecurityStack(secTypeTLSNone, "TLSNone",
              new CSecurityTLS(true), null));
    case Security.secTypeTLSVnc:
      return (new CSecurityStack(secTypeTLSVnc, "TLSVnc",
              new CSecurityTLS(true), new CSecurityVncAuth()));
    case Security.secTypeTLSPlain:
      return (new CSecurityStack(secTypeTLSPlain, "TLSPlain",
              new CSecurityTLS(true), new CSecurityPlain()));
    case Security.secTypeTLSIdent:
      return (new CSecurityStack(secTypeTLSIdent, "TLSIdent",
              new CSecurityTLS(true), new CSecurityIdent()));
    case Security.secTypeX509None:
      return (new CSecurityStack(secTypeX509None, "X509None",
              new CSecurityTLS(false), null));
    case Security.secTypeX509Vnc:
      return (new CSecurityStack(secTypeX509Vnc, "X509Vnc",
              new CSecurityTLS(false), new CSecurityVncAuth()));
    case Security.secTypeX509Plain:
      return (new CSecurityStack(secTypeX509Plain, "X509Plain",
              new CSecurityTLS(false), new CSecurityPlain()));
    case Security.secTypeX509Ident:
      return (new CSecurityStack(secTypeX509Ident, "X509Ident",
              new CSecurityTLS(false), new CSecurityIdent()));
    default:
      throw new ErrorException("Security type not supported");
    }

  }

  String msg = null;

  public static StringParameter secTypes
  = new StringParameter("SecurityTypes",
  "A comma-separated list of the security types that can be used, if the " +
  "server supports them.  \"VNC\" and \"None\" are the standard VNC " +
  "password and no-password authentication schemes supported by all VNC " +
  "servers.  The 10 supported VeNCrypt security types (Plain, Ident, " +
  "TLSNone, TLSVnc, TLSPlain, TLSIdent, X509None, X509Vnc, X509Plain, and " +
  "X509Ident) are combinations of three encryption methods (None, Anonymous " +
  "TLS, and TLS with X.509 certificates) and four authentication schemes " +
  "(None, Standard VNC, Plain, and Ident.)  The \"UnixLogin\" security type " +
  "enables user/password authentication using the TightVNC security " +
  "extensions rather than VeNCrypt.  \"Plain\" and \"UnixLogin\" " +
  "authenticate using a plain-text user name and password, so it is " +
  "strongly recommended that those types only be used with either TLS " +
  "encryption or SSH tunneling.  \"Ident\", which is designed for use by " +
  "VNC proxies, authenticates using only a user name.  The order of this " +
  "list does not matter, since the server's preferred order is always used.",
  "X509Plain,X509Ident,X509Vnc,X509None,TLSPlain,TLSIdent,TLSVnc,TLSNone,VNC,Ident,Plain,UnixLogin,None");

  public static StringParameter x509ca
  = new StringParameter("X509CA",
  "X.509 Certificate Authority certificate to use with the X509* security " +
  "types.  This is used to check the validity of the server's X.509 " +
  "certificate.", FileUtils.getVncHomeDir() + "x509_ca.pem");

  public static StringParameter x509crl
  = new StringParameter("X509CRL",
  "X.509 Certificate Revocation List to use with the X509* security types. " +
  "This is used to check the validity of the server's X.509 " +
  "certificate.", FileUtils.getVncHomeDir() + "x509_crl.pem");
}
