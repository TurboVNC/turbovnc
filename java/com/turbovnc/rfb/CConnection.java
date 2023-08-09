/* Copyright (C) 2012, 2014, 2016, 2018, 2020-2023 D. R. Commander.
 *                                                 All Rights Reserved.
 * Copyright 2019 Pierre Ossman for Cendio AB
 * Copyright (C) 2011-2012 Brian P. Hinz
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

import java.nio.*;
import java.nio.charset.*;
import java.util.*;

import com.turbovnc.network.*;
import com.turbovnc.rdr.*;

public abstract class CConnection extends CMsgHandler {

  public CConnection() {
    csecurity = null;  is = null;  os = null;  reader = null;
    writer = null;  shared = false;
    state = RFBSTATE_UNINITIALISED;
    security = new SecurityClient();
  }

  // deleteReaderAndWriter() deletes the reader and writer associated with
  // this connection.  This may be useful if you want to delete the streams
  // before deleting the SConnection to make sure that no attempt by the
  // SConnection is made to read or write.
  // XXX Do we really need this at all???
  public void deleteReaderAndWriter() {
    reader = null;
    writer = null;
  }

  // initialiseProtocol() should be called once the streams and security
  // types are set.  Subsequently, processMsg() should be called whenever
  // there is data to read on the InStream.
  public final void initialiseProtocol() {
    state = RFBSTATE_PROTOCOL_VERSION;
  }

  // processMsg() should be called whenever there is data to read on the
  // InStream.  You must have called initialiseProtocol() first.
  public synchronized void processMsg(boolean benchmark) {
    switch (state) {
      case RFBSTATE_PROTOCOL_VERSION:  processVersionMsg();         break;
      case RFBSTATE_SECURITY_TYPES:    processSecurityTypesMsg();   break;
      case RFBSTATE_SECURITY:          processSecurityMsg();        break;
      case RFBSTATE_SECURITY_RESULT:   processSecurityResultMsg();  break;
      case RFBSTATE_INITIALISATION:    processInitMsg(benchmark);   break;
      case RFBSTATE_NORMAL:            reader.readMsg(params);      break;
      case RFBSTATE_UNINITIALISED:
        throw new ErrorException("CConnection.processMsg: not initialised yet?");
      default:
        throw new ErrorException("CConnection.processMsg: invalid state");
    }
  }

  private void processVersionMsg() {
    if (!alreadyPrintedVersion) {
      vlog.debug("reading protocol version");
      alreadyPrintedVersion = true;
    }

    if (!cp.readVersion(is)) {
      state = RFBSTATE_INVALID;
      throw new ErrorException("Reading version failed: not an RFB server?");
    }
    if (!cp.done) return;

    vlog.info("Server supports RFB protocol version " +
              cp.majorVersion + "." + cp.minorVersion);

    // The UltraVNC Repeater sends a version number of 000:000, which
    // indicates that it is waiting for either a VNC server name (Mode I) or
    // a VNC server ID (Mode II).
    if (cp.majorVersion == 0 && cp.minorVersion == 0) {
      if (serverName == null)
        throw new ErrorException("UltraVNC Repeater detected but VNC server name has not been specified");
      vlog.info("Connecting to " + serverName + " via UltraVNC repeater");
      os.writeBytes(serverName.getBytes(), 0,
                    Math.min(serverName.length(), 250));
      if (serverName.length() < 250) {
        byte[] pad = new byte[250 - serverName.length()];
        os.writeBytes(pad, 0, pad.length);
      }
      os.flush();
      return;
    }

    // The only official RFB protocol versions are currently 3.3, 3.7 and 3.8
    if (cp.beforeVersion(3, 3)) {
      String msg = ("Server gave unsupported RFB protocol version " +
                    cp.majorVersion + "." + cp.minorVersion);
      vlog.error(msg);
      state = RFBSTATE_INVALID;
      throw new ErrorException(msg);
    } else if (cp.beforeVersion(3, 7)) {
      cp.setVersion(3, 3);
    } else if (cp.afterVersion(3, 8)) {
      cp.setVersion(3, 8);
    }

    cp.writeVersion(os);
    state = RFBSTATE_SECURITY_TYPES;

    vlog.info("Using RFB protocol version " +
              cp.majorVersion + "." + cp.minorVersion);
  }

  private void processSecurityTypesMsg() {
    vlog.debug("processing security types message");

    int secType = RFB.SECTYPE_INVALID;

    List<Integer> secTypes = new ArrayList<Integer>();
    secTypes = params.secTypes.getEnabled();

    if (cp.isVersion(3, 3)) {

      // legacy 3.3 server may only offer "vnc authentication" or "none"

      secType = is.readU32();
      if (secType == RFB.SECTYPE_INVALID) {
        throwConnFailedException();

      } else if (secType == RFB.SECTYPE_NONE ||
                 secType == RFB.SECTYPE_VNCAUTH) {
        Iterator<Integer> i;
        for (i = secTypes.iterator(); i.hasNext();) {
          int refType = (Integer)i.next();
          if (refType == secType) {
            secType = refType;
            break;
          }
        }

        if (!secTypes.contains(secType))
          secType = RFB.SECTYPE_INVALID;
      } else {
        vlog.error("Unknown RFB 3.3 security type " + secType);
        throw new ErrorException("Unknown RFB 3.3 security type " + secType);
      }

    } else {

      // >=3.7 server will offer us a list

      int nServerSecTypes = is.readU8();
      if (nServerSecTypes == 0)
        throwConnFailedException();

      for (int i = 0; i < nServerSecTypes; i++) {
        int serverSecType = is.readU8();
        vlog.debug("Server offers security type " +
                   RFB.secTypeName(serverSecType) + "(" + serverSecType + ")");

        /*
         * TurboVNC specific - use secTypeTight if the server supports it.
         */
        if (serverSecType == RFB.SECTYPE_TIGHT)
          secType = RFB.SECTYPE_TIGHT;

        if (params.sessMgrActive && params.sessMgrAuto.get())
          secType = RFB.SECTYPE_VNCAUTH;

        /*
         * Use the first type sent by server which matches client's type.
         * It means server's order specifies priority.
         */
        if (secType == RFB.SECTYPE_INVALID && secType != RFB.SECTYPE_TIGHT) {
          for (Iterator<Integer> j = secTypes.iterator(); j.hasNext();) {
            int refType = (Integer)j.next();
            if (refType == serverSecType) {
              secType = refType;
              break;
            }
          }
        }
      }

      // Inform the server of our decision
      if (secType != RFB.SECTYPE_INVALID) {
        os.writeU8(secType);
        os.flush();
        vlog.debug("Choosing security type " + RFB.secTypeName(secType) +
                   "(" + secType + ")");
      }
    }

    if (secType == RFB.SECTYPE_INVALID) {
      state = RFBSTATE_INVALID;
      vlog.error("No matching security types");
      throw new ErrorException("No matching security types");
    }

    state = RFBSTATE_SECURITY;
    csecurity = security.getCSecurity(params, secType);
    processSecurityMsg();
  }

  private void processSecurityMsg() {
    if (!alreadyPrintedSecurity) {
      vlog.debug("processing security message");
      alreadyPrintedSecurity = true;
    }

    if (csecurity.processMsg(this)) {
      state = RFBSTATE_SECURITY_RESULT;
      processSecurityResultMsg();
    }
  }

  private void processSecurityResultMsg() {
    if (!alreadyPrintedSecurityResult) {
      vlog.debug("processing security result message");
      alreadyPrintedSecurityResult = true;
    }
    int result;
    if (cp.beforeVersion(3, 8) &&
        (csecurity.getType() == RFB.SECTYPE_NONE ||
         (csecurity instanceof CSecurityRFBTLS &&
          csecurity.getType() == RFB.SECTYPE_TLS_NONE))) {
      result = RFB.AUTH_OK;
    } else {
      if (!is.checkNoWait(1)) return;
      result = is.readU32();
    }
    switch (result) {
      case RFB.AUTH_OK:
        securityCompleted();
        return;
      case RFB.AUTH_FAILED:
        vlog.debug("auth failed");
        break;
      case RFB.AUTH_TOO_MANY:
        vlog.debug("auth failed - too many tries");
        break;
      default:
        throw new ErrorException("Unknown security result from server");
    }
    String reason;
    if (cp.beforeVersion(3, 8))
      reason = "Authentication failure";
    else
      reason = is.readString();
    state = RFBSTATE_INVALID;
    throw new AuthFailureException(reason);
  }

  private void processInitMsg(boolean benchmark) {
    vlog.debug("reading server initialisation");
    reader.readServerInit(benchmark);
  }

  private void throwConnFailedException() {
    state = RFBSTATE_INVALID;
    String reason;
    reason = is.readString();
    throw new ConnFailedException(reason);
  }

  private void securityCompleted() {
    state = RFBSTATE_INITIALISATION;
    reader = new CMsgReader(this, is);
    writer = new CMsgWriter(cp, os);
    vlog.debug("Authentication success!");
    //authSuccess();
    writer.writeClientInit(shared);
  }

  // Methods to initialise the connection

  // setServerName() is used to provide a unique(ish) name for the server to
  // which we are connected.  This might be the result of getPeerEndpoint on
  // a TcpSocket, for example, or a host specified by DNS name & port.
  // The serverName is used when verifying the Identity of a host (see RA2).
  public final void setServerName(String name) {
    serverName = name;
  }

  // setStreams() sets the streams to be used for the connection.  These must
  // be set before initialiseProtocol() and processMsg() are called.  The
  // CSecurity object may call setStreams() again to provide alternative
  // streams over which the RFB protocol is sent (i.e. encrypting/decrypting
  // streams).  Ownership of the streams remains with the caller
  // (i.e. SConnection will not delete them).
  public final void setStreams(InStream is_, OutStream os_) {
    is = is_;
    os = os_;
  }

  // setShared sets the value of the shared flag which will be sent to the
  // server upon initialisation.
  public final void setShared(boolean s) { shared = s; }

  public void setServerPort(int port) {
    serverPort = port;
  }

  public void initSecTypes() {
    nSecTypes = 0;
  }

  // Methods to be overridden in a derived class

  // getIdVerifier() returns the identity verifier associated with the
  // connection.  Ownership of the IdentityVerifier is retained by the
  // CConnection instance.
  //public IdentityVerifier getIdentityVerifier() { return 0; }

  // authSuccess() is called when authentication has succeeded.
  //public void authSuccess() {}

  // serverInit() is called when the ServerInit message is received.  The
  // derived class must call on to CConnection::serverInit().
  public void serverInit() {
    state = RFBSTATE_NORMAL;
    vlog.debug("initialisation done");
  }

  // getCSecurity() gets the CSecurity object for the given type.  The type
  // is guaranteed to be one of the secTypes passed in to addSecType().  The
  // CSecurity object's destroy() method will be called by the CConnection
  // from its destructor.
  //abstract public CSecurity getCSecurity(int secType);

  // getCurrentCSecurity() gets the CSecurity instance used for this
  // connection.
  public CSecurity getCurrentCSecurity() { return csecurity; }

  // setClientSecTypeOrder() determines whether the client should obey the
  // server's security type preference, by picking the first server security
  // type that the client supports, or whether it should pick the first type
  // that the server supports, from the client-supported list of types.
  public void setClientSecTypeOrder(boolean csto) {
    clientSecTypeOrder = csto;
  }

  // handleClipboardAnnounce() is called whenever the server's clipboard
  // changes.  requestClipboard() should be called in order to access the
  // actual data.

  public abstract void handleClipboardAnnounce(boolean available);

  // handleClipboardData() is called whenever the server has sent its clipboard
  // data in response to a previous call to requestClipboard().  Note that this
  // function might never be called if the clipboard data was no longer
  // available when the server received the request.

  public abstract void handleClipboardData(String data);

  // handleClipboardRequest() is called whenever the server requests clipboard
  // data from the client.  It will only be called after the client has first
  // announced a clipboard change via announceClipboard().

  public abstract void handleClipboardRequest();

  // Other methods

  public CMsgReader reader() { return reader; }
  public CMsgWriter writer() { return writer; }

  public InStream getInStream() { return is; }
  public OutStream getOutStream() { return os; }

  public String getServerName() { return serverName; }
  public int getServerPort() { return serverPort; }

  public static final int RFBSTATE_UNINITIALISED = 0;
  public static final int RFBSTATE_PROTOCOL_VERSION = 1;
  public static final int RFBSTATE_SECURITY_TYPES = 2;
  public static final int RFBSTATE_SECURITY = 3;
  public static final int RFBSTATE_SECURITY_RESULT = 4;
  public static final int RFBSTATE_INITIALISATION = 5;
  public static final int RFBSTATE_NORMAL = 6;
  public static final int RFBSTATE_INVALID = 7;

  public int state() { return state; }

  protected final void setState(int s) { state = s; }

  public void fence(int flags, int len, byte[] data) {
    super.fence(flags, len, data);

    if ((flags & RFB.FENCE_FLAG_REQUEST) != 0)
      return;

    // We cannot guarantee any synchronisation at this level
    flags = 0;

    synchronized(this) {
      writer().writeFence(flags, len, data);
    }
  }

  private void throwAuthFailureException() {
    String reason;
    vlog.debug("state=" + state() + ", ver=" + cp.majorVersion + "." +
               cp.minorVersion);
    if (state() == RFBSTATE_SECURITY_RESULT && !cp.beforeVersion(3, 8)) {
      reason = is.readString();
    } else {
      reason = "Authentication failure";
    }
    state = RFBSTATE_INVALID;
    vlog.error(reason);
    throw new AuthFailureException(reason);
  }

  public Socket getSocket() {
    return sock;
  }

  public boolean getUserPasswd(StringBuffer user, StringBuffer password) {
    throw new ErrorException("getUserPasswd() called in base class (this shouldn't happen.)");
  }

  // announceClipboard() informs the server that the client's clipboard has
  // changed.  The server may later request the clipboard data via
  // handleClipboardRequest().

  public void announceClipboard(boolean available)
  {
    if (!params.sendClipboard.get())
      return;

    hasLocalClipboard = available;
    unsolicitedClipboardAttempt = false;

    // Attempt an unsolicited transfer?
    if (available && cp.clipboardSize(RFB.EXTCLIP_FORMAT_UTF8) > 0 &&
        (cp.clipboardFlags() & RFB.EXTCLIP_ACTION_PROVIDE) != 0) {
      vlog.debug("Attempting unsolicited clipboard transfer...");
      unsolicitedClipboardAttempt = true;
      handleClipboardRequest();
      return;
    }

    if ((cp.clipboardFlags() & RFB.EXTCLIP_ACTION_NOTIFY) != 0) {
      writer().writeClipboardNotify(available ? RFB.EXTCLIP_FORMAT_UTF8 : 0);
      return;
    }

    if (available)
      handleClipboardRequest();
  }

  void handleClipboardCaps(int flags, int[] lengths)
  {
    int[] sizes = new int[]{ 0 };

    super.handleClipboardCaps(flags, lengths);

    writer().writeClipboardCaps(RFB.EXTCLIP_FORMAT_UTF8 |
                                RFB.EXTCLIP_ACTION_REQUEST |
                                RFB.EXTCLIP_ACTION_PEEK |
                                RFB.EXTCLIP_ACTION_NOTIFY |
                                RFB.EXTCLIP_ACTION_PROVIDE, sizes);
  }

  void handleClipboardNotify(int flags)
  {
    if (!params.recvClipboard.get())
      return;

    serverClipboard = null;

    if ((flags & RFB.EXTCLIP_FORMAT_UTF8) != 0) {
      hasLocalClipboard = false;
      handleClipboardAnnounce(true);
    } else {
      handleClipboardAnnounce(false);
    }
  }

  void handleClipboardPeek(int flags)
  {
    if (!params.sendClipboard.get())
      return;

    if ((cp.clipboardFlags() & RFB.EXTCLIP_ACTION_NOTIFY) != 0)
      writer().writeClipboardNotify(hasLocalClipboard ?
                                    RFB.EXTCLIP_FORMAT_UTF8 : 0);
  }

  void handleClipboardProvide(int flags, int[] lengths, byte[][] buffers)
  {
    if (!params.recvClipboard.get())
      return;

    if ((flags & RFB.EXTCLIP_FORMAT_UTF8) == 0) {
      vlog.debug("Ignoring Extended Clipboard provide message with " +
                 "unsupported formats 0x" + Integer.toHexString(flags));
      return;
    }

    if (buffers[0][lengths[0] - 1] == 0)
      lengths[0]--;
    serverClipboard =
      Utils.convertLF(new String(buffers[0], 0, lengths[0],
                                 StandardCharsets.UTF_8));

    // FIXME: Should probably verify that this data was actually requested
    handleClipboardData(serverClipboard);
  }

  void handleClipboardRequest(int flags)
  {
    if (!params.sendClipboard.get())
      return;

    if ((flags & RFB.EXTCLIP_FORMAT_UTF8) == 0) {
      vlog.debug("Ignoring Extended Clipboard request with unsupported " +
                 "formats 0x" + Integer.toHexString(flags));
      return;
    }
    if (!hasLocalClipboard) {
      vlog.debug("Ignoring unexpected clipboard request");
      return;
    }
    handleClipboardRequest();
  }

  // requestClipboard() requests clipboard data from the server.
  // handleClipboardData() will be called once the data is available.

  public void requestClipboard()
  {
    if (!params.recvClipboard.get())
      return;

    if (serverClipboard != null) {
      handleClipboardData(serverClipboard);
      return;
    }

    if ((cp.clipboardFlags() & RFB.EXTCLIP_ACTION_REQUEST) != 0)
      writer().writeClipboardRequest(RFB.EXTCLIP_FORMAT_UTF8);
  }

  // sendClipboardData() transfers the client's clipboard data to the server
  // and should be called whenever the server has requested the clipboard data
  // via handleClipboardRequest().

  public void sendClipboardData(String data)
  {
    if (!params.sendClipboard.get())
      return;

    if ((cp.clipboardFlags() & RFB.EXTCLIP_ACTION_PROVIDE) != 0) {
      String filtered = Utils.convertCRLF(data);
      int[] lengths = new int[1];
      byte[][] datas = new byte[1][];

      byte[] filteredBytes = filtered.getBytes(StandardCharsets.UTF_8);
      lengths[0] = filteredBytes.length + 1;
      datas[0] = new byte[filteredBytes.length + 1];
      System.arraycopy(filteredBytes, 0, datas[0], 0, filteredBytes.length);

      if (unsolicitedClipboardAttempt) {
        unsolicitedClipboardAttempt = false;
        if (lengths[0] > cp.clipboardSize(RFB.EXTCLIP_FORMAT_UTF8)) {
          vlog.debug(lengths[0] +
                     "-byte clipboard was too large for unsolicited transfer");
          if ((cp.clipboardFlags() & RFB.EXTCLIP_ACTION_NOTIFY) != 0)
            writer().writeClipboardNotify(RFB.EXTCLIP_FORMAT_UTF8);
          return;
        }
      }

      writer().writeClipboardProvide(RFB.EXTCLIP_FORMAT_UTF8, lengths, datas);
    } else {
      writer().writeClientCutText(data);
    }
  }

  public void serverCutText(String str)
  {
    if (!params.recvClipboard.get())
      return;

    hasLocalClipboard = false;

    serverClipboard = str;

    handleClipboardAnnounce(true);
  }

  InStream is;
  OutStream os;
  protected CMsgReader reader;
  CMsgWriter writer;
  boolean shared;
  protected CSecurity csecurity;
  SecurityClient security;
  int nSecTypes;
  protected int state = RFBSTATE_UNINITIALISED;
  String serverName;
  int serverPort;
  boolean clientSecTypeOrder;
  protected Socket sock;
  boolean alreadyPrintedVersion, alreadyPrintedSecurity;
  boolean alreadyPrintedSecurityResult;

  private boolean hasLocalClipboard, unsolicitedClipboardAttempt;
  private String serverClipboard;

  // CHECKSTYLE VisibilityModifier:OFF
  public Params params;
  // CHECKSTYLE VisibilityModifier:ON

  static LogWriter vlog = new LogWriter("CConnection");
}
