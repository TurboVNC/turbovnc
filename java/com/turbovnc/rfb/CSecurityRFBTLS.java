/* Copyright (C) 2012, 2016-2018, 2021-2022 D. R. Commander.
 *                                          All Rights Reserved.
 * Copyright (C) 2011 Brian P Hinz
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

// This implements the rfbTLS security descriptor used by Vino.  It is
// basically just a repeat of the RFB 3.7 security negotiation process, only
// performed inside of an encrypted socket.  VncAuth and None are the only
// valid sub-types.

package com.turbovnc.rfb;

import java.util.*;

import com.turbovnc.rdr.*;

public class CSecurityRFBTLS extends CSecurityTLS {

  public CSecurityRFBTLS(SecurityClient sec) {
    super(true);
    security = sec;
  }

  public boolean processMsg(CConnection cc) {
    super.processMsg(cc, false);

    InStream is = cc.getInStream();
    OutStream os = cc.getOutStream();

    // Shamelessly horked from CConnection.processSecurityTypesMsg()
    vlog.debug("processing TLS security types message");

    int secType = RFB.SECTYPE_INVALID;

    List<Integer> secTypes = new ArrayList<Integer>();
    secTypes = cc.params.secTypes.getEnabledExt();

    int nServerSecTypes = is.readU8();
    if (nServerSecTypes == 0)
      throw new ErrorException("Server reported no TLS sub-types");

    for (int i = 0; i < nServerSecTypes; i++) {
      int serverSecType = is.readU8();
      vlog.debug("Server offers TLS security type " +
                 RFB.secTypeName(serverSecType) + "(" + serverSecType + ")");

      /*
       * Use the first type sent by server which matches client's type.
       * It means server's order specifies priority.
       */
      if (secType == RFB.SECTYPE_INVALID) {
        for (Iterator<Integer> j = secTypes.iterator(); j.hasNext();) {
          int refType = (Integer)j.next();
          if ((refType == RFB.SECTYPE_TLS_VNC &&
               serverSecType == RFB.SECTYPE_VNCAUTH) ||
              (refType == RFB.SECTYPE_TLS_NONE &&
               serverSecType == RFB.SECTYPE_NONE)) {
            secType = serverSecType;
            chosenType = refType;
            break;
          }
        }
      }
    }

    if (secType == RFB.SECTYPE_INVALID) {
      cc.state = CConnection.RFBSTATE_INVALID;
      vlog.error("No matching security types");
      throw new ErrorException("No matching security types");
    }

    // Inform the server of our decision
    os.writeU8(secType);
    os.flush();
    vlog.debug("Choosing TLS security type " +
               RFB.secTypeName(secType) + "(" + secType + ")");

    csecurity = security.getCSecurity(cc.params, secType);
    return csecurity.processMsg(cc);
  }

  public final int getType() { return chosenType; }

  public final String getDescription() {
    return RFB.secTypeName(chosenType);
  }

  private CSecurity csecurity;
  SecurityClient security;
  private int chosenType;

  static LogWriter vlog = new LogWriter("CSecurityRFBTLS");
}
