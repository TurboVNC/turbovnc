/* Copyright (C) 2012 Brian P. Hinz
 * Copyright (C) 2012 D. R. Commander.  All Rights Reserved.
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
import com.turbovnc.vncviewer.*;

public class CSecurityTight extends CSecurity {

  final static String VENDOR_STDV = "STDV";
  final static String VENDOR_VENC = "VENC";
  final static String VENDOR_GTKV = "GTKV";
  final static String VENDOR_TGHT = "TGHT";
  final static int NOTUNNEL = 0;
  final static int CODE_NOAUTH = 1;
  final static int CODE_VNCAUTH = 2;
  final static int CODE_VENCRYPT = 19;
  final static int CODE_SASL = 20;
  final static int CODE_ULGNAUTH = 129;
  final static String SIG_NOAUTH = "NOAUTH__";
  final static String SIG_VNCAUTH = "VNCAUTH_";
  final static String SIG_VENCRYPT = "VENCRYPT";
  final static String SIG_SASL = "SASL____";
  final static String SIG_ULGNAUTH = "ULGNAUTH";

  public CSecurityTight(SecurityClient sec) { 
    security = sec;  
  }

  public boolean processMsg(CConnection cc) 
  {
    InStream is = cc.getInStream();
    OutStream os = cc.getOutStream();
    List<Integer> SupportedAuthTypes = new ArrayList<Integer>();

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
      // FIXME:
      // Add functions comparable to GetEnabledSecTypes for checking
      // Tight auth caps.
      switch (code) {
      case CODE_NOAUTH:
        if (!Security.GetEnabledSecTypes().contains(Security.secTypeNone))
          break;
        if (vendor.equals(VENDOR_STDV) && signature.equals(SIG_NOAUTH))
          SupportedAuthTypes.add(code);
        break;
      case CODE_VNCAUTH:
        if (!Security.GetEnabledSecTypes().contains(Security.secTypeVncAuth))
          break;
        if (vendor.equals(VENDOR_STDV) && signature.equals(SIG_VNCAUTH))
          SupportedAuthTypes.add(code);
        break;
      case CODE_ULGNAUTH:
        if (!((CConn)cc).isUnixLoginSelected())
          break;

        if (vendor.equals(VENDOR_TGHT) && signature.equals(SIG_ULGNAUTH)) {
          if (((CConn)cc).isUnixLoginForced())
            SupportedAuthTypes.add(0, code);
          else
            SupportedAuthTypes.add(code);
        }
        break;
      case CODE_VENCRYPT:
        if (!Security.GetEnabledSecTypes().contains(Security.secTypeVeNCrypt))
          break;
        if (vendor.equals(VENDOR_VENC) && signature.equals(SIG_VENCRYPT))
          SupportedAuthTypes.add(code);
        break;
      default:
        vlog.info("Unsupported auth type: "+code+" "+vendor+" "+signature);
        break;
      }
    }

    if (SupportedAuthTypes.isEmpty())
      throw new Exception("No supported auth types!");

    for (Iterator<Integer> i = SupportedAuthTypes.iterator(); i.hasNext(); ) {
      int authType = (Integer)i.next();
      switch (authType) {
      case CODE_ULGNAUTH: 
        os.writeU32(CODE_ULGNAUTH);
  
        StringBuffer username = new StringBuffer();
        StringBuffer password = new StringBuffer();
  
        CConn.upg.getUserPasswd(username, password);
  
        // Return the response to the server
        os.writeU32(username.length());
        os.writeU32(password.length());
        byte[] utf8str;
        try {
          utf8str = username.toString().getBytes("UTF8");
          os.writeBytes(utf8str, 0, username.length());
          utf8str = password.toString().getBytes("UTF8");
          os.writeBytes(utf8str, 0, password.length());
        } catch(java.io.UnsupportedEncodingException e) {
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

  SecurityClient security;
  CSecurity cs;
  public int getType() { return Security.secTypeTight; }
  public String description() { 
    if (cs != null)
      return cs.description();
    return "Tight"; 
  }

  static LogWriter vlog = new LogWriter("Tight");
}
