/* Copyright (C) 2012, 2015, 2018, 2021-2023 D. R. Commander.
 *                                           All Rights Reserved.
 * Copyright (C) 2011 Brian P. Hinz
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

import java.util.*;

import com.turbovnc.rdr.*;

public final class SecTypesParameter extends VoidParameter {

  public SecTypesParameter(String name, Params params, boolean isGUI,
                           String desc, String defValue_) {
    super(name, params, isGUI, desc);
    defValue = defValue_;
    // NOTE: We defer initialization of enabledSecTypes until first use,
    // because the initialization depends on other parameters that may not yet
    // be constructed at the time that this parameter is constructed.
  }

  public synchronized boolean set(String secTypesString) {
    if (secTypesString == null)
      enabledSecTypes = new ArrayList<Integer>();
    else
      enabledSecTypes = parse(secTypesString, false);
    setCommandLine(false);
    return true;
  }

  public synchronized void reset() { enabledSecTypes = null; }

  public synchronized boolean setDefault(String secTypesString) {
    if (secTypesString != null) {
      try {
        List<Integer> result = parse(secTypesString, true);
        if (result.size() > 0)
          defValue = getStr(result);
        else
          defValue = null;
      } catch (Exception e) {
        return false;
      }
    } else
      defValue = null;
    return true;
  }

  public synchronized List<Integer> getEnabled() {
    List<Integer> result = new ArrayList<Integer>();

    if (enabledSecTypes == null)
      set(defValue);

    for (Iterator<Integer> i = enabledSecTypes.iterator(); i.hasNext();) {
      int refType = (Integer)i.next();
      if (refType >= 0x100) {
        result.add(RFB.SECTYPE_VENCRYPT);
        break;
      }
    }
    result.add(RFB.SECTYPE_TIGHT);
    result.add(RFB.SECTYPE_TLS);
    for (Iterator<Integer> i = enabledSecTypes.iterator(); i.hasNext();) {
      int refType = (Integer)i.next();
      if (refType < 0x100 && refType != RFB.SECTYPE_TIGHT &&
          refType != RFB.SECTYPE_TLS)
        result.add(refType);
    }

    return result;
  }

  public synchronized List<Integer> getEnabledExt() {
    List<Integer> result = new ArrayList<Integer>();

    if (enabledSecTypes == null)
      set(defValue);

    for (Iterator<Integer> i = enabledSecTypes.iterator(); i.hasNext();) {
      int refType = (Integer)i.next();
      if (refType != RFB.SECTYPE_VENCRYPT && refType != RFB.SECTYPE_UNIX_LOGIN)
        /* ^^ Do not include VeNCrypt to avoid loops */
        result.add(refType);
    }

    return result;
  }

  public synchronized List<Integer> getEnabledTight() {
    List<Integer> result = new ArrayList<Integer>();

    if (enabledSecTypes == null)
      set(defValue);

    for (Iterator<Integer> i = enabledSecTypes.iterator(); i.hasNext();) {
      int refType = (Integer)i.next();
      if (refType < 0x100 && refType != RFB.SECTYPE_VENCRYPT &&
          refType != RFB.SECTYPE_TIGHT && refType != RFB.SECTYPE_TLS)
        result.add(refType);
    }

    return result;
  }

  public synchronized void enable(int secType) {
    if (enabledSecTypes == null)
      set(defValue);

    for (Iterator<Integer> i = enabledSecTypes.iterator(); i.hasNext();)
      if ((Integer)i.next() == secType)
        return;

    if (isAllowed(secType))
      enabledSecTypes.add(secType);

    setCommandLine(false);
  }

  public synchronized void disable(int secType) {
    if (enabledSecTypes == null)
      set(defValue);

    enabledSecTypes.remove((Object)secType);

    setCommandLine(false);
  }

  private boolean isAllowed(int secType) {
    if (params.noUnixLogin.get() &&
        (secType == RFB.SECTYPE_PLAIN || secType == RFB.SECTYPE_TLS_PLAIN ||
         secType == RFB.SECTYPE_X509_PLAIN ||
         secType == RFB.SECTYPE_UNIX_LOGIN))
      return false;
    if ((params.user.get() != null || params.sendLocalUsername.get()) &&
        (secType == RFB.SECTYPE_VNCAUTH || secType == RFB.SECTYPE_TLS_VNC ||
         secType == RFB.SECTYPE_X509_VNC || secType == RFB.SECTYPE_NONE ||
         secType == RFB.SECTYPE_TLS_NONE || secType == RFB.SECTYPE_X509_NONE))
      return false;
    return true;
  }

  public synchronized boolean isSupported(int secType) {
    Iterator<Integer> i;

    if (enabledSecTypes == null)
      set(defValue);

    if (params.sessMgrActive && params.sessMgrAuto.get() &&
        secType == RFB.SECTYPE_VNCAUTH)
      return true;

    for (i = enabledSecTypes.iterator(); i.hasNext();)
      if ((Integer)i.next() == secType)
        return true;
    if (secType == RFB.SECTYPE_VENCRYPT)
      return true;
    if (secType == RFB.SECTYPE_TIGHT)
      return true;
    if (secType == RFB.SECTYPE_TLS)
      return true;

    return false;
  }

  private List<Integer> parse(String types_, boolean force) {
    List<Integer> result = new ArrayList<Integer>();
    String[] types = types_.split(",");
    for (int i = 0; i < types.length; i++) {
      if (types[i].length() < 1) continue;
      int typeNum = RFB.secTypeNum(types[i]);
      if (typeNum != RFB.SECTYPE_INVALID) {
        if (isAllowed(typeNum) || force)
          result.add(typeNum);
      } else if (!types[i].equalsIgnoreCase("Ident") &&
                 !types[i].equalsIgnoreCase("TLSIdent") &&
                 !types[i].equalsIgnoreCase("X509Ident"))
        throw new WarningException("Security type \'" + types[i] +
                                   "\' is not valid");
    }
    return result;
  }

  public synchronized String getDefaultStr() { return defValue; }

  private String getStr(List<Integer> secTypes) {
    StringBuilder sb = new StringBuilder();
    for (Iterator<Integer> i = secTypes.iterator(); i.hasNext();) {
      int refType = ((Integer)i.next());
      if (refType != RFB.SECTYPE_VENCRYPT && refType != RFB.SECTYPE_TIGHT &&
          refType != RFB.SECTYPE_TLS) {
        sb.append(RFB.secTypeName(refType));
        if (i.hasNext()) sb.append(',');
      }
    }
    return sb.toString();
  }

  public synchronized String getStr() {
    if (enabledSecTypes == null)
      set(defValue);

    return getStr(enabledSecTypes);
  }

  public String getValues() { return null; }

  private List<Integer> enabledSecTypes;
  private String defValue;
}
