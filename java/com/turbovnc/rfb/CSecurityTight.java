/* Copyright (C) 2012, 2015, 2017-2018, 2020-2022 D. R. Commander.
 *                                                All Rights Reserved.
 * Copyright (C) 2012 Brian P. Hinz
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

import java.util.*;
import com.turbovnc.rdr.*;

public class CSecurityTight extends CSecurity {

  static final String VENDOR_STDV = "STDV";
  static final String VENDOR_VENC = "VENC";
  static final String VENDOR_GTKV = "GTKV";
  static final String VENDOR_TGHT = "TGHT";
  static final int NOTUNNEL = 0;
  static final int CODE_NOAUTH = 1;
  static final int CODE_VNCAUTH = 2;
  static final int CODE_VENCRYPT = 19;
  static final int CODE_SASL = 20;
  static final int CODE_ULGNAUTH = 129;
  static final String SIG_NOAUTH = "NOAUTH__";
  static final String SIG_VNCAUTH = "VNCAUTH_";
  static final String SIG_VENCRYPT = "VENCRYPT";
  static final String SIG_SASL = "SASL____";
  static final String SIG_ULGNAUTH = "ULGNAUTH";

  public CSecurityTight(SecurityClient sec) {
    security = sec;
  }

  public boolean processMsg(CConnection cc) {
    InStream is = cc.getInStream();
    OutStream os = cc.getOutStream();
    List<Integer> supportedAuthTypes = new ArrayList<Integer>();

    if (cs instanceof CSecurityVeNCrypt) {
      // VeNCrypt has already been selected using the Tight security selector,
      // so defer any further handshaking to the VeNCrypt handler.
      return cs.processMsg(cc);
    }

    int nTunnels = is.readU32();

    if (nTunnels > 0) {
      vlog.info("Tunneling not supported!");
      os.writeU32(NOTUNNEL);
    }

    int nAuthTypes = is.readU32();
    if (nAuthTypes == 0) {
      cs = new CSecurityNone();
      return (cs.processMsg(cc));
    }

    for (int i = 0; i < nAuthTypes; i++) {
      int code = is.readS32();
      byte[] v = new byte[4];
      is.readBytes(v, 0, 4);
      String vendor = new String(v);
      byte[] s = new byte[8];
      is.readBytes(s, 0, 8);
      String signature = new String(s);

      Params params = cc.params;
      switch (code) {
        case CODE_NOAUTH:
          if (!params.secTypes.getEnabled().contains(RFB.SECTYPE_NONE))
            break;
          if (vendor.equals(VENDOR_STDV) && signature.equals(SIG_NOAUTH))
            supportedAuthTypes.add(code);
          break;
        case CODE_VNCAUTH:
          if (!params.secTypes.getEnabled().contains(RFB.SECTYPE_VNCAUTH))
            break;
          if (vendor.equals(VENDOR_STDV) && signature.equals(SIG_VNCAUTH))
            supportedAuthTypes.add(code);
          break;
        case CODE_ULGNAUTH:
          if (!params.secTypes.getEnabledTight().contains(RFB.SECTYPE_UNIX_LOGIN))
            break;
          if (vendor.equals(VENDOR_TGHT) && signature.equals(SIG_ULGNAUTH))
            supportedAuthTypes.add(code);
          break;
        case CODE_VENCRYPT:
          if (!params.secTypes.getEnabled().contains(RFB.SECTYPE_VENCRYPT))
            break;
          if (vendor.equals(VENDOR_VENC) && signature.equals(SIG_VENCRYPT))
            supportedAuthTypes.add(code);
          break;
        default:
          vlog.info("Unsupported auth type: " + code + " " + vendor + " " +
                    signature);
          break;
      }
    }

    if (supportedAuthTypes.isEmpty())
      throw new WarningException("No supported auth types!");

    for (Iterator<Integer> i = supportedAuthTypes.iterator(); i.hasNext();) {
      int authType = (Integer)i.next();
      if (authType == CODE_ULGNAUTH || authType == CODE_NOAUTH ||
          authType == CODE_VNCAUTH || authType == CODE_VENCRYPT)
        vlog.debug("Choosing security type " + RFB.secTypeName(authType) +
                   "(" + authType + ")");
      switch (authType) {
        case CODE_ULGNAUTH:
          os.writeU32(CODE_ULGNAUTH);

          StringBuffer username = new StringBuffer();
          StringBuffer password = new StringBuffer();

          cc.getUserPasswd(username, password);

          // Return the response to the server
          os.writeU32(username.length());
          os.writeU32(password.length());
          byte[] utf8str;
          try {
            utf8str = username.toString().getBytes("UTF8");
            os.writeBytes(utf8str, 0, username.length());
            utf8str = password.toString().getBytes("UTF8");
            os.writeBytes(utf8str, 0, password.length());
          } catch (java.io.UnsupportedEncodingException e) {
            e.printStackTrace();
          }
          os.flush();
          return true;
        case CODE_NOAUTH:
          os.writeU32(CODE_NOAUTH);
          os.flush();
          cs = new CSecurityNone();
          return (cs.processMsg(cc));
        case CODE_VNCAUTH:
          os.writeU32(CODE_VNCAUTH);
          os.flush();
          cs = new CSecurityVncAuth();
          return (cs.processMsg(cc));
        case CODE_VENCRYPT:
          os.writeU32(CODE_VENCRYPT);
          os.flush();
          cs = new CSecurityVeNCrypt(security);
          return (cs.processMsg(cc));
      }
    }
    return false;
  }

  public final int getType() { return RFB.SECTYPE_TIGHT; }

  public final int getChosenType() {
    return (cs != null ? cs.getType() : RFB.SECTYPE_UNIX_LOGIN);
  }

  public final String getDescription() {
    return (cs != null ? cs.getDescription() : "UnixLogin");
  }

  public final String getProtocol() {
    return (cs != null ? cs.getProtocol() : "None");
  }

  SecurityClient security;
  CSecurity cs;

  static LogWriter vlog = new LogWriter("Tight");
}
