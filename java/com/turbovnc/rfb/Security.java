/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2010 TigerVNC Team
 * Copyright (C) 2011 Brian P. Hinz
 * Copyright (C) 2012, 2015-2016, 2018 D. R. Commander.  All Rights Reserved.
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

public class Security {

  public Security(StringParameter secTypes) {
    if (setInUserPrefs && secTypes.isDefault) return;

    String secTypesStr;

    secTypesStr = secTypes.getData();
    enabledSecTypes = parseSecTypes(secTypesStr);

    secTypesStr = null;
  }

  static List<Integer> enabledSecTypes = new ArrayList<Integer>();

  public static final List<Integer> getEnabledSecTypes() {
    List<Integer> result = new ArrayList<Integer>();

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

    return (result);
  }

  public static final List<Integer> getEnabledExtSecTypes() {
    List<Integer> result = new ArrayList<Integer>();

    for (Iterator<Integer> i = enabledSecTypes.iterator(); i.hasNext();) {
      int refType = (Integer)i.next();
      if (refType != RFB.SECTYPE_VENCRYPT && refType != RFB.SECTYPE_UNIX_LOGIN)
        /* ^^ Do not include VeNCrypt to avoid loops */
        result.add(refType);
    }

    return (result);
  }

  public static final List<Integer> getEnabledTightSecTypes() {
    List<Integer> result = new ArrayList<Integer>();

    for (Iterator<Integer> i = enabledSecTypes.iterator(); i.hasNext();) {
      int refType = (Integer)i.next();
      if (refType < 0x100 && refType != RFB.SECTYPE_VENCRYPT &&
          refType != RFB.SECTYPE_TIGHT && refType != RFB.SECTYPE_TLS)
        result.add(refType);
    }

    return (result);
  }

  public static final void enableSecType(int secType) {
    for (Iterator<Integer> i = enabledSecTypes.iterator(); i.hasNext();)
      if ((Integer)i.next() == secType)
        return;

    enabledSecTypes.add(secType);
  }

  public boolean isSupported(int secType) {
    Iterator<Integer> i;

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

  public static void disableSecType(int secType) {
    enabledSecTypes.remove((Object)secType);
  }

  public static final List<Integer> parseSecTypes(String types_) {
    List<Integer> result = new ArrayList<Integer>();
    String[] types = types_.split(",");
    for (int i = 0; i < types.length; i++) {
      int typeNum = RFB.secTypeNum(types[i]);
      if (typeNum != RFB.SECTYPE_INVALID)
        result.add(typeNum);
      else
        throw new WarningException("Security type \'" + types[i] +
                                   "\' is not valid");
    }
    return (result);
  }

  public final void setSecTypes(List<Integer> secTypes) {
    enabledSecTypes = secTypes;
  }

  @SuppressWarnings("checkstyle:VisibilityModifier")
  public static boolean setInUserPrefs;

  static LogWriter vlog = new LogWriter("Security");
}
