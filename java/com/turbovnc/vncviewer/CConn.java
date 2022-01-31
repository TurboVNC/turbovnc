/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright 2009-2011 Pierre Ossman <ossman@cendio.se> for Cendio AB
 * Copyright (C) 2011-2021 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2011-2015 Brian P. Hinz
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

// CConn
//
// Methods in CConn are called from both the Swing Event Dispatch Thread (EDT)
// and the thread that processes incoming RFB messages ("the RFB thread").
// This means that we need to be careful with synchronization here.
//
// Any access to writer() must not only be synchronized, but we must also make
// sure that the connection is in RFBSTATE_NORMAL.  We are guaranteed this for
// any code called after serverInit() has been called.  Since the DesktopWindow
// isn't created until then, any methods called only from DesktopWindow can
// assume that we are in RFBSTATE_NORMAL.

package com.turbovnc.vncviewer;

import java.awt.*;
import java.awt.event.*;

import java.io.*;
import java.lang.reflect.*;
import javax.swing.*;
import java.text.*;
import java.util.*;

import com.turbovnc.rdr.*;
import com.turbovnc.rfb.*;
import com.turbovnc.rfb.Point;
import com.turbovnc.network.Socket;
import com.turbovnc.network.TcpSocket;

public class CConn extends CConnection implements UserPasswdGetter,
  OptionsDialogCallback, FdInStreamBlockCallback {

  public final PixelFormat getPreferredPF() { return fullColourPF; }
  static final PixelFormat VERY_LOW_COLOR_PF =
    new PixelFormat(8, 3, false, true, 1, 1, 1, 2, 1, 0);
  static final PixelFormat LOW_COLOR_PF =
    new PixelFormat(8, 6, false, true, 3, 3, 3, 4, 2, 0);
  static final PixelFormat MEDIUM_COLOR_PF =
    new PixelFormat(8, 8, false, true, 7, 7, 3, 5, 2, 0);
  static final PixelFormat MEDIUMHIGH_COLOR_PF =
    new PixelFormat(16, 15, false, true, 31, 31, 31, 10, 5, 0);
  static final PixelFormat HIGH_COLOR_PF =
    new PixelFormat(16, 16, false, true, 31, 63, 31, 11, 5, 0);
  static final int SUPER_MASK = 1 << 16;

  // RFB thread
  public CConn(VncViewer viewer_, Socket sock_) {
    sock = sock_;  viewer = viewer_;
    benchmark = viewer.benchFile != null;
    pendingPFChange = false;
    lastServerEncoding = -1;

    opts = new Options(VncViewer.opts);

    formatChange = false;  encodingChange = false;
    currentEncoding = opts.preferredEncoding;
    options = new OptionsDialog(this);
    options.initDialog();
    clipboardDialog = new ClipboardDialog(this);
    profileDialog = new ProfileDialog(this);
    String env = System.getenv("TVNC_PROFILE");
    if (env != null && env.equals("1"))
      alwaysProfile = true;
    firstUpdate = true;  pendingUpdate = false;  continuousUpdates = false;
    forceNonincremental = true;  supportsSyncFence = false;
    pressedKeys = new HashMap<Integer, Integer>();

    setShared(opts.shared);

    cp.supportsDesktopResize = true;
    cp.supportsExtendedDesktopSize = true;
    cp.supportsClientRedirect = Params.clientRedirect.getValue();
    cp.supportsDesktopRename = true;
    menu = new F8Menu(this);

    if (sock != null) {
      String name = sock.getPeerEndpoint();
      vlog.info("Accepted connection from " + name);
    } else if (!benchmark) {
      String serverName = null;
      int port = -1;

      if (opts.serverName != null &&
          !Params.alwaysShowConnectionDialog.getValue()) {
        if (opts.via == null || opts.via.indexOf(':') < 0) {
          port = opts.port = Hostname.getPort(opts.serverName);
          serverName = opts.serverName = Hostname.getHost(opts.serverName);
        }
      } else {
        ServerDialog dlg = new ServerDialog(options, opts, this);
        boolean ret = dlg.showDialog();
        if (!ret) {
          close();
          return;
        }
        port = opts.port;
        serverName = opts.serverName;
      }

      if (opts.via != null && opts.via.indexOf(':') >= 0) {
        port = Hostname.getPort(opts.via);
        serverName = Hostname.getHost(opts.via);
      } else if (opts.via != null || opts.tunnel ||
                 (opts.port == 0 && Params.sessMgrAuto.getValue())) {
        if (opts.port == 0) {
          try {
            // TurboVNC Session Manager
            String session = SessionManager.createSession(opts);
            if (session == null) {
              close();
              return;
            }
            opts.sessMgrActive = true;
            opts.port = Hostname.getPort(session);
          } catch (Exception e) {
            throw new ErrorException("Session Manager Error:\n" +
                                     e.getMessage());
          }
        }
        try {
          Tunnel.createTunnel(opts);
          port = Hostname.getPort(opts.serverName);
          serverName = Hostname.getHost(opts.serverName);
        } catch (Exception e) {
          throw new ErrorException("Could not create SSH tunnel:\n" +
                                   e.getMessage());
        }
      }

      if (port == 0) {
        try {
          // TurboVNC Session Manager
          String session = SessionManager.createSession(opts);
          if (session == null) {
            close();
            return;
          }
          port = Hostname.getPort(session);
          opts.sshSession.disconnect();
        } catch (Exception e) {
          throw new ErrorException("Session Manager Error:\n" +
                                   e.getMessage());
        }
      }

      sock = new TcpSocket(serverName, port);
      vlog.info("connected to host " + serverName + " port " + port);
    }

    if (benchmark) {
      state = RFBSTATE_INITIALISATION;
      reader = new CMsgReaderV3(this, viewer.benchFile);
    } else {
      sock.inStream().setBlockCallback(this);
      setServerName(opts.serverName);
      setStreams(sock.inStream(), sock.outStream());
      initialiseProtocol();
    }
  }

  // RFB thread
  public void reset() {
    if (reader != null)
      reader.reset();
    state = RFBSTATE_INITIALISATION;
  }

  // EDT: deleteWindow() is called when the user closes the window or selects
  // "Close Connection" from the F8 menu.
  void deleteWindow(boolean disposeViewport) {
    if (viewport != null) {
      if (viewport.timer != null)
        viewport.timer.stop();
      if (disposeViewport)
        viewport.dispose();
    }
    releasePressedKeys();
    viewport = null;
  }

  // RFB thread: blockCallback() is called when reading from the socket would
  // block.
  public void blockCallback() {
    try {
      synchronized(this) {
        wait(0, 50000);
      }
    } catch (InterruptedException e) {
      throw new SystemException(e);
    }
  }

  // RFB thread: getUserPasswd() is called by the CSecurity object when it
  // needs us to read a password from the user.
  @Override
  public final boolean getUserPasswd(StringBuffer user, StringBuffer passwd) {
    String title = ((user == null ? "Standard VNC Authentication" :
                                    "Unix Login Authentication") +
                    " [" + csecurity.getDescription() + "]");
    String passwordFileStr = Params.passwordFile.getValue();
    PasswdDialog dlg = null;
    String autoPass;

    if (Params.encPassword.getValue() != null) {
      byte[] encryptedPassword = new byte[8];
      String passwordString = Params.encPassword.getValue();
      if (passwordString.length() != 16)
        throw new ErrorException("Password specified in EncPassword parameter is invalid.");
      for (int c = 0; c < 16; c += 2) {
        int temp = -1;
        try {
          temp = Integer.parseInt(passwordString.substring(c, c + 2), 16);
        } catch (NumberFormatException e) {}
        if (temp >= 0)
          encryptedPassword[c / 2] = (byte)temp;
        else break;
      }
      autoPass = VncAuth.unobfuscatePasswd(encryptedPassword);
    } else if (Params.autoPass.getValue()) {
      BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
      try {
        autoPass = in.readLine();
      } catch (IOException e) {
        throw new SystemException(e);
      }
      Params.autoPass.setParam("0");
    } else
      autoPass = Params.password.getValue();

    if (autoPass != null && passwd != null) {
      passwd.append(autoPass);
      passwd.setLength(autoPass.length());
      Params.password.setParam(null);
    }

    if (user == null && passwordFileStr != null && autoPass == null) {
      InputStream fp = null;
      try {
        fp = new FileInputStream(passwordFileStr);
      } catch (FileNotFoundException e) {
        throw new WarningException("Could not open password file");
      }
      byte[] obfPwd = new byte[256];
      try {
        fp.read(obfPwd);
        fp.close();
      } catch (IOException e) {
        throw new ErrorException("Could not read password file");
      }
      String plainPasswd = VncAuth.unobfuscatePasswd(obfPwd);
      passwd.append(plainPasswd);
      passwd.setLength(plainPasswd.length());
      return true;
    }

    if (user == null) {
      if (autoPass == null)
        dlg = new PasswdDialog(title, (user == null), null, (passwd == null),
                               opts.sshTunnelActive,
                               csecurity.getChosenType());
    } else {
      String userName = opts.user;
      if (opts.sendLocalUsername) {
        userName = (String)System.getProperties().get("user.name");
        if (Params.localUsernameLC.getValue())
          userName = userName.toLowerCase();
        if (passwd == null)
          return true;
      }
      if (autoPass == null)
        dlg = new PasswdDialog(title, (userName != null), userName,
                               (passwd == null), opts.sshTunnelActive,
                               csecurity.getChosenType());
      else
        user.append(userName);
    }

    if (dlg != null) {
      if (!dlg.showDialog()) return false;
      if (user != null)
        user.append(dlg.userEntry.getText());
      if (passwd != null)
        passwd.append(dlg.passwdEntry.getPassword());
    }

    return true;
  }

  // CConnection callback methods

  // RFB thread: serverInit() is called when the serverInit message has been
  // received.  At this point, we create the desktop window and display it.  We
  // also tell the server which pixel format and encodings to use and request
  // the first update.
  public void serverInit() {
    super.serverInit();

    serverPF = cp.pf();

    desktop = new DesktopWindow(cp.width, cp.height, serverPF, this);
    fullColourPF = desktop.getPreferredPF();

    // Force a switch to our preferred format and encoding.
    formatChange = true;  encodingChange = true;

    // And kick off the update cycle
    if (!benchmark)
      requestNewUpdate();
    else {
      if (Params.colors.getValue() == 8) {
        pendingPF = VERY_LOW_COLOR_PF;
      } else if (Params.colors.getValue() == 64) {
        pendingPF = LOW_COLOR_PF;
      } else if (Params.colors.getValue() == 256) {
        pendingPF = MEDIUM_COLOR_PF;
      } else if (Params.colors.getValue() == 32768) {
        pendingPF = MEDIUMHIGH_COLOR_PF;
      } else if (Params.colors.getValue() == 65536) {
        pendingPF = HIGH_COLOR_PF;
      } else {
        pendingPF = fullColourPF;
      }
      pendingPFChange = true;
    }

    // This initial update request is a bit of a corner case, so we need to
    // help by setting the correct format here.
    assert(pendingPFChange);
    desktop.setServerPF(pendingPF);
    cp.setPF(pendingPF);
    pendingPFChange = false;

    try {
      SwingUtilities.invokeAndWait(new Runnable() {
        public void run() {
          recreateViewport();
        }
      });
    } catch (InterruptedException e) {
    } catch (InvocationTargetException e) {
      SystemException.checkException(e);
      throw new SystemException(e);
    }
    synchronized(viewer) {
      viewer.notify();
    }
  }

  // RFB thread: setDesktopSize() is called when the desktop size changes
  // (including when it is set initially.)
  public void setDesktopSize(int w, int h) {
    super.setDesktopSize(w, h);
    resizeFramebuffer();
    if (opts.desktopSize.mode == Options.SIZE_MANUAL && !firstUpdate) {
      opts.desktopSize.width = w;
      opts.desktopSize.height = h;
    }
  }

  // RFB thread: setExtendedDesktopSize() is a more advanced version of
  // setDesktopSize().
  public void setExtendedDesktopSize(int reason, int result, int w, int h,
                                     ScreenSet layout) {
    super.setExtendedDesktopSize(reason, result, w, h, layout);

    if (reason == RFB.EDS_REASON_CLIENT && result != RFB.EDS_RESULT_SUCCESS) {
      vlog.error("SetDesktopSize failed: " + result);
      return;
    }

    resizeFramebuffer();
    if (opts.desktopSize.mode == Options.SIZE_MANUAL && !firstUpdate) {
      opts.desktopSize.width = w;
      opts.desktopSize.height = h;
    }

    // Normally, the TurboVNC Viewer will not create a multi-screen viewer
    // window that extends beyond the unshared boundary of any physical screen
    // on the client.  This ensures that the window position and scrollbars
    // will be correct, regardless of the monitor layout.  However, if
    // automatic desktop resizing is enabled and the server supports Xinerama,
    // then the full-screen multi-screen viewer window should extend to the
    // bounding box of all physical screens, even if it extends beyond the
    // unshared boundary of some of them.  This ensures correct behavior if the
    // screens have different resolutions or are offset.  TurboVNC 2.0.x-2.1.x
    // supported the RFB extended desktop size message but not Xinerama, and
    // those versions also sent a screen ID of 0.  Thus, we do not assume that
    // the server supports Xinerama unless it sends a multi-screen layout or a
    // non-zero screen ID.
    if (layout.numScreens() > 1 ||
        (layout.numScreens() > 0 && layout.screens.get(0).id != 0))
      serverXinerama = true;
  }

  // RFB thread: clientRedirect() migrates the client to another host/port.
  public void clientRedirect(int port, String host,
                             String x509subject) {
    sock.close();
    setServerPort(port);
    sock = new TcpSocket(host, port);
    vlog.info("Redirected to " + host + ":" + port);
    VncViewer.newViewer(viewer, sock, true);
  }

  // RFB thread: setName() is called when the desktop name changes.
  public void setName(String name) {
    super.setName(name);

    if (viewport != null) {
      viewport.setTitle(name + " - TurboVNC");
    }
  }

  // RFB thread: framebufferUpdateStart() is called at the beginning of an
  // update.  Here we try to send out a new framebuffer update request so that
  // the next update can be sent while we decode the current one.
  public void framebufferUpdateStart() {
    tUpdateStart = Utils.getTime();
    if (tStart < 0.) tStart = tUpdateStart;

    // Note: This might not be true if sync fences are supported.
    pendingUpdate = false;

    if (!benchmark) requestNewUpdate();
  }

  // RFB thread
  public void framebufferUpdateEnd() {

    desktop.updateWindow();

    if (firstUpdate) {
      int width, height;

      // We need fences to make extra update requests and continuous updates
      // "safe".  See fence() for the next step.
      if (cp.supportsFence)
        writer().writeFence(RFB.FENCE_FLAG_REQUEST | RFB.FENCE_FLAG_SYNC_NEXT,
                            0, null);

      if (!cp.supportsSetDesktopSize) {
        if (opts.desktopSize.mode == Options.SIZE_AUTO)
          vlog.info("Disabling automatic desktop resizing because the server doesn't support it.");
        if (opts.desktopSize.mode == Options.SIZE_MANUAL)
          vlog.info("Ignoring desktop resize request because the server doesn't support it.");
        opts.desktopSize.mode = Options.SIZE_SERVER;
      }

      if (opts.desktopSize.mode == Options.SIZE_MANUAL)
        sendDesktopSize(opts.desktopSize.width, opts.desktopSize.height,
                        false);
      else if (opts.desktopSize.mode == Options.SIZE_AUTO) {
        Dimension availableSize = viewport.getAvailableSize();
        sendDesktopSize(availableSize.width, availableSize.height, false);
      }

      firstUpdate = false;
    }

    // A format change has been scheduled, and we have finished decoding and
    // displaying the last framebuffer update that used the old format, so
    // activate the new format.
    if (pendingPFChange) {
      desktop.setServerPF(pendingPF);
      cp.setPF(pendingPF);
      pendingPFChange = false;
    }

    tUpdate += Utils.getTime() - tUpdateStart;
    updates++;
    tElapsed = Utils.getTime() - tStart;

    if (tElapsed > (double)Params.profileInt.getValue() && !benchmark) {
      if (profileDialog.isVisible()) {
        String str;
        str = String.format("%.3f", (double)updates / tElapsed);
        profileDialog.upsVal.setText(str);
        str = String.format("%.3f", (double)sock.inStream().getBytesRead() /
                            125000. / tElapsed);
        profileDialog.tpVal.setText(str);

        str = String.format("%.3f", sock.inStream().getReadTime() /
                            (double)updates * 1000.);
        profileDialog.tpuRecvVal.setText(str);
        str = String.format("%.3f", tDecode / (double)updates * 1000.);
        profileDialog.tpuDecodeVal.setText(str);
        str = String.format("%.3f", tBlit / (double)updates * 1000.);
        profileDialog.tpuBlitVal.setText(str);
        str = String.format("%.3f", tUpdate / (double)updates * 1000.);
        profileDialog.tpuTotalVal.setText(str);

        str = String.format("%.3f", (double)decodePixels / 1000000.);
        profileDialog.mpDecodeVal.setText(str);
        str = String.format("%.3f", (double)blitPixels / 1000000.);
        profileDialog.mpBlitVal.setText(str);

        str = String.format("%.3f", (double)decodePixels / 1000000. / tDecode);
        profileDialog.mpsDecodeVal.setText(str);
        str = String.format("%.3f", (double)blitPixels / 1000000. / tBlit);
        profileDialog.mpsBlitVal.setText(str);
        str = String.format("%.3f", (double)decodePixels / 1000000. /
                            tElapsed);
        profileDialog.mpsTotalVal.setText(str);

        str = String.format("%d", decodeRect);
        profileDialog.rectDecodeVal.setText(str);
        str = String.format("%d", blits);
        profileDialog.rectBlitVal.setText(str);

        str = String.format("%.0f", (double)decodePixels / (double)decodeRect);
        profileDialog.pprDecodeVal.setText(str);
        str = String.format("%.0f", (double)blitPixels / (double)blits);
        profileDialog.pprBlitVal.setText(str);

        str = String.format("%.0f", (double)decodeRect / (double)updates);
        profileDialog.rpuDecodeVal.setText(str);
      }
      if (profileDialog.isVisible() || alwaysProfile) {
        System.out.format("-------------------------------------------------------------------------------\n");
        System.out.format("Total:   %.3f updates/sec,  %.3f Mpixels/sec,  %.3f Mbits/sec\n",
                          (double)updates / tElapsed,
                          (double)decodePixels / 1000000. / tElapsed,
                          (double)sock.inStream().getBytesRead() / 125000. /
                            tElapsed);
        System.out.format("Decode:  %.3f Mpixels,  %.3f Mpixels/sec,  %d rect,\n",
                          (double)decodePixels / 1000000.,
                          (double)decodePixels / 1000000. / tDecode,
                          decodeRect);
        System.out.format("         %.0f pixels/rect,  %.0f rect/update\n",
                          (double)decodePixels / (double)decodeRect,
                          (double)decodeRect / (double)updates);
        System.out.format("Blit:    %.3f Mpixels,  %.3f Mpixels/sec,  %d updates,\n",
                          (double)blitPixels / 1000000.,
                          (double)blitPixels / 1000000. / tBlit,
                          blits);
        System.out.format("         %.0f pixels/update\n",
                          (double)blitPixels / (double)blits);
        System.out.format("Time/update:  Recv = %.3f ms,  Decode = %.3f ms,  Blit = %.3f ms\n",
                          sock.inStream().getReadTime() / (double)updates *
                            1000.,
                          tDecode / (double)updates * 1000.,
                          tBlit / (double)updates * 1000.);
        System.out.format("              Total = %.3f ms  +  Overhead = %.3f ms\n",
                          tUpdate / (double)updates * 1000.,
                          (tElapsed - tUpdate) / (double)updates * 1000.);
      }
      tUpdate = tDecode = tBlit = 0.0;
      sock.inStream().resetReadTime();
      sock.inStream().resetBytesRead();
      decodePixels = decodeRect = blitPixels = blits = updates = 0;
      tStart = Utils.getTime();
    }
  }

  public final ScreenSet computeScreenLayout(int width, int height) {
    java.awt.Point vpPos = viewport.getContentPane().getLocationOnScreen();
    Rectangle vpRect = viewport.getContentPane().getBounds();
    ScreenSet layout;

    String env = System.getenv("TVNC_SINGLESCREEN");
    if (Utils.getBooleanProperty("turbovnc.singlescreen", false) ||
        (env != null && env.equals("1"))) {
      layout = cp.screenLayout;

      if (layout.numScreens() == 0)
        layout.addScreen(new Screen());
      else if (layout.numScreens() != 1) {
        int i = 0;

        for (Iterator<Screen> iter = layout.screens.iterator(); iter.hasNext();
             i++) {
          Screen screen = (Screen)iter.next();
          if (i > 0)
            iter.remove();
        }
      }

      Screen screen0 = (Screen)layout.screens.iterator().next();
      screen0.dimensions.tl.x = 0;
      screen0.dimensions.tl.y = 0;
      screen0.dimensions.br.x = width;
      screen0.dimensions.br.y = height;

      return layout;
    }

    layout = new ScreenSet();

    vpRect.x = vpPos.x;
    vpRect.y = vpPos.y;
    if (opts.showToolbar && !opts.fullScreen) {
      vpRect.y += 22;
      vpRect.height -= 22;
    }

    GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
    GraphicsDevice[] gsList = ge.getScreenDevices();
    for (GraphicsDevice gs : gsList) {
      GraphicsConfiguration[] gcList = gs.getConfigurations();
      for (GraphicsConfiguration gc : gcList) {
        Rectangle s = gc.getBounds();
        if (gc == gcList[0]) {
          Rectangle screenRect = s.intersection(vpRect);

          if (screenRect.isEmpty() || screenRect.width < 1 ||
              screenRect.height < 1)
            continue;

          screenRect.x -= vpRect.x;
          screenRect.y -= vpRect.y;

          Screen screen = new Screen(0, screenRect.x, screenRect.y,
                                     screenRect.width, screenRect.height, 0);

          // We map client screens to server screens in the server's preferred
          // order (which, in the case of the TurboVNC Server, is always the
          // order of RANDR outputs), so we send the "primary" screen (the
          // screen containing 0, 0) first.  This ensures that the window
          // manager taskbar on the server will follow the taskbar on the
          // client.
          if (s.contains(0, 0))
            layout.addScreen0(screen);
          else
            layout.addScreen(screen);
        }
      }
    }

    layout.assignIDs(cp.screenLayout);

    return layout;
  }

  // RFB thread when sending initial desktop size, EDT when sending desktop
  // size from component listener (writer() is synchronized.)
  public void sendDesktopSize(int width, int height, boolean fromListener) {
    ScreenSet layout;

    if (!cp.supportsSetDesktopSize)
      return;

    if (opts.desktopSize.mode == Options.SIZE_AUTO)
      layout = computeScreenLayout(width, height);
    else {
      if (opts.desktopSize.mode != Options.SIZE_MANUAL ||
          opts.desktopSize.layout == null) {
        vlog.error("ERROR: Unexpected desktop size configuration");
        return;
      }
      layout = opts.desktopSize.layout;
      // Map client screens to server screen IDs in the server's preferred
      // order.  This allows us to control the server's screen order from the
      // client.
      layout.assignIDs(cp.screenLayout);
    }

    sendDesktopSize(width, height, layout, fromListener);
  }

  public void sendDesktopSize(int width, int height, ScreenSet layout,
                              boolean fromListener) {
    if (!cp.supportsSetDesktopSize)
      return;

    if (!layout.validate(width, height, true)) {
      vlog.error("Invalid screen layout");
      return;
    }

    writer().writeSetDesktopSize(width, height, layout);
    if (fromListener)
      pendingAutoResize.setXYWH(0, 0, width, height);
  }

  // The rest of the callbacks are fairly self-explanatory.  These are all
  // called from the RFB thread.

  public void setColourMapEntries(int firstColour, int nColours, int[] rgbs) {
    desktop.setColourMapEntries(firstColour, nColours, rgbs);
  }

  public void bell() {
    if (opts.acceptBell)
      desktop.getToolkit().beep();
  }

  public void serverCutText(String str, int len) {
    if (opts.recvClipboard)
      clipboardDialog.serverCutText(str, len);
  }

  public void startDecodeTimer() {
    tDecodeStart = Utils.getTime();
    if (benchmark)
      tReadOld = viewer.benchFile.getReadTime();
    else
      tReadOld = sock.inStream().getReadTime();
  }

  public void stopDecodeTimer() {
    double tRead = tReadOld;
    if (benchmark)
      tRead = viewer.benchFile.getReadTime();
    else
      tRead = sock.inStream().getReadTime();
    tDecode += Utils.getTime() - tDecodeStart - (tRead - tReadOld);
  }

  public void beginRect(Rect r, int encoding) {
    if (!benchmark)
      sock.inStream().startTiming();
    if (encoding != RFB.ENCODING_COPYRECT) {
      boolean updateTitle = false;
      if (encoding != lastServerEncoding)
        updateTitle = true;
      lastServerEncoding = encoding;
      if (updateTitle && viewport != null)
        viewport.updateTitle();
    }
  }

  public void endRect(Rect r, int encoding) {
    if (!benchmark)
      sock.inStream().stopTiming();
    decodePixels += r.width() * r.height();
    decodeRect++;
  }

  public void fillRect(Rect r, int p) {
    desktop.fillRect(r.tl.x, r.tl.y, r.width(), r.height(), p);
  }

  public void imageRect(Rect r, Object p) {
    desktop.imageRect(r.tl.x, r.tl.y, r.width(), r.height(), p);
  }

  public void copyRect(Rect r, int sx, int sy) {
    desktop.copyRect(r.tl.x, r.tl.y, r.width(), r.height(), sx, sy);
  }

  public Object getRawPixelsRW(int[] stride) {
    return desktop.getRawPixelsRW(stride);
  }

  public void releaseRawPixels(Rect r) {
    desktop.releaseRawPixels(r);
  }

  // RFB thread
  public void setCursor(int width, int height, Point hotspot,
                        int[] data, byte[] mask) {
    if (viewport != null && (viewport.dx > 0 || viewport.dy > 0))
      hotspot.translate(new Point(viewport.dx, viewport.dy));
    desktop.setCursor(width, height, hotspot, data, mask);
  }

  // RFB thread
  public void fence(int flags, int len, byte[] data) {
    // can't call super.super.fence(flags, len, data);
    cp.supportsFence = true;

    if ((flags & RFB.FENCE_FLAG_REQUEST) != 0) {
      // We handle everything synchronously, so we trivially honor these modes.
      flags = flags & (RFB.FENCE_FLAG_BLOCK_BEFORE |
                       RFB.FENCE_FLAG_BLOCK_AFTER);

      writer().writeFence(flags, len, data);
      return;
    }

    if (len == 0) {
      // Initial probe
      if ((flags & RFB.FENCE_FLAG_SYNC_NEXT) != 0) {
        supportsSyncFence = true;

        if (cp.supportsContinuousUpdates) {
          vlog.info("Enabling continuous updates");
          continuousUpdates = true;
          writer().writeEnableContinuousUpdates(true, 0, 0, cp.width,
                                                cp.height);
        }
      }
    } else {
      // Pixel format change
      MemInStream memStream = new MemInStream(data, 0, len);
      PixelFormat pf = new PixelFormat();

      pf.read(memStream);
      if (pf.is888() && VncViewer.forceAlpha)
        pf.alpha = true;

      desktop.setServerPF(pf);
      cp.setPF(pf);
    }
  }

  // RFB thread
  public void enableGII() {
    cp.supportsGII = true;
    if (viewport != null && !benchmark) {
      vlog.info("Enabling GII");
      writer().writeGIIVersion();
      if (Utils.osEID())
        viewport.setupExtInputHelper();
    }
  }

  // RFB thread
  public void giiDeviceCreated(int deviceOrigin) {
    if (viewport != null && !benchmark)
      viewport.assignInputDevice(deviceOrigin);
  }

  // Helper thread
  public void giiDeviceCreate(ExtInputDevice dev) {
    if (viewport != null && !benchmark)
      writer().writeGIIDeviceCreate(dev);
  }

  // Helper thread
  public void giiSendEvent(ExtInputDevice dev, ExtInputEvent e) {
    if (viewport == null || benchmark)
      return;

    Rectangle screenArea = getMaxSpannedSize(false);
    java.awt.Point spOffset = viewport.sp.getViewport().getViewPosition();
    java.awt.Point winOffset = viewport.sp.getLocationOnScreen();

    // Here's where things get dicey.  We assume valuators 0 and 1 are X and
    // Y, which means we need to translate them so they will make sense
    // relative to the remote desktop.
    if (dev.valuators.size() > 0) {
      for (int i = e.firstValuator; i < e.firstValuator + e.numValuators;
           i++) {
        ExtInputDevice.Valuator v =
          (ExtInputDevice.Valuator)dev.valuators.get(i);
        if (i == 0) {
          double x = (double)(e.valuators[i] - v.rangeMin) /
                     (double)(v.rangeMax - v.rangeMin) *
                     (double)(screenArea.width - 1) - (double)winOffset.x;
          if (viewport.dx > 0)
            x -= (double)viewport.dx;
          x += spOffset.x;
          if (cp.width != desktop.scaledWidth) {
            x = (desktop.scaleWidthRatio == 1.00) ? x :
                x / desktop.scaleWidthRatio;
          }
          e.valuators[i] = (int)(x / (double)(cp.width - 1) *
                                 (double)(v.rangeMax - v.rangeMin) +
                                 (double)v.rangeMin + 0.5);
          if (e.valuators[i] > v.rangeMax)
            e.valuators[i] = v.rangeMax;
          else if (e.valuators[i] < v.rangeMin)
            e.valuators[i] = v.rangeMin;
        } else if (i == 1) {
          double y = (double)(e.valuators[i - e.firstValuator] - v.rangeMin) /
                     (double)(v.rangeMax - v.rangeMin) *
                     (double)(screenArea.height - 1) - (double)winOffset.y;
          if (viewport.dy > 0)
            y -= (double)viewport.dy;
          y += spOffset.y;
          if (cp.height != desktop.scaledHeight) {
            y = (desktop.scaleHeightRatio == 1.00) ? y :
                y / desktop.scaleHeightRatio;
          }
          e.valuators[i - e.firstValuator] =
            (int)((double)y / (double)(cp.height - 1) *
                  (double)(v.rangeMax - v.rangeMin) +
                  (double)v.rangeMin + 0.5);
          if (e.valuators[i - e.firstValuator] > v.rangeMax)
            e.valuators[i - e.firstValuator] = v.rangeMax;
          else if (e.valuators[i - e.firstValuator] < v.rangeMin)
            e.valuators[i - e.firstValuator] = v.rangeMin;
        }
      }
    }

    e.print();
    writer().writeGIIEvent(dev, e);
  }

  // RFB thread
  private void resizeFramebuffer() {
    if (desktop == null)
      return;

    if (continuousUpdates)
      writer().writeEnableContinuousUpdates(true, 0, 0, cp.width, cp.height);

    if ((cp.width == 0) && (cp.height == 0))
      return;

    if (pendingAutoResize.width() == cp.width &&
        pendingAutoResize.height() == cp.height) {
      desktop.setScaledSize();
      desktop.resize();
      pendingAutoResize.setXYWH(0, 0, 0, 0);
      return;
    }
    pendingAutoResize.setXYWH(0, 0, 0, 0);

    int w, h;
    w = desktop.width();
    h = desktop.height();

    if ((w == cp.width) && (h == cp.height) &&
        (desktop.im.width() == cp.width) &&
        (desktop.im.height() == cp.height)) {
      desktop.setScaledSize();
      return;
    }

    try {
      SwingUtilities.invokeAndWait(new Runnable() {
        public void run() {
          // Normally, when automatic desktop resizing is enabled, the viewer
          // will attempt to keep the remote desktop size synced with the local
          // viewport size.  However, in full-screen mode, the viewport cannot
          // change size.  Thus, we cannot force the remote desktop size to
          // always be equal to the viewport size in that case, because it
          // would create an endless game of ping pong if two connected viewers
          // both had automatic desktop resizing enabled, one of them entered
          // full-screen mode, and the other (non-full-screen) viewer could not
          // accommodate the new size.  In that case, we let the
          // non-full-screen viewer win, and the full-screen viewer will
          // display a full-screen viewport that is only partially used.  The
          // following code thus sets a flag to temporarily disable the
          // component resize handler, then it resizes the full-screen viewport
          // accordingly.
          if (opts.desktopSize.mode == Options.SIZE_AUTO && opts.fullScreen)
            pendingServerResize = true;
          desktop.resize();
          if (opts.desktopSize.mode == Options.SIZE_AUTO && !opts.fullScreen)
            // Have to do this in order to trigger the resize handler, even if
            // we're not actually changing the component size.
            recreateViewport(false);
          else
            reconfigureViewport(false);
          if (opts.desktopSize.mode == Options.SIZE_AUTO && opts.fullScreen)
            pendingServerResize = false;
        }
      });
    } catch (InterruptedException e) {
    } catch (InvocationTargetException e) {
      SystemException.checkException(e);
      throw new SystemException(e);
    }
  }

  // EDT: recreateViewport() recreates our top-level window.

  static Rectangle savedRect = new Rectangle(-1, -1, 0, 0);
  static int savedState = -1;

  private void recreateViewport() { recreateViewport(false); }

  private void recreateViewport(boolean restore) {
    if (desktop == null) return;

    if (viewport != null) {
      if (opts.fullScreen) {
        savedState = viewport.getExtendedState();
        viewport.setExtendedState(JFrame.NORMAL);
        savedRect = viewport.getBounds();
      }
      if (viewport.timer != null)
        viewport.timer.stop();
      viewport.grabKeyboardHelper(false);
      if (Params.currentMonitorIsPrimary.getValue())
        oldViewportBounds = viewport.getBounds();
      viewport.dispose();
    }
    viewport = new Viewport(this);
    // When in Lion full-screen mode, we need to create the viewport as if
    // full-screen mode was disabled.
    viewport.setUndecorated(opts.fullScreen);
    desktop.setViewport(viewport);
    reconfigureViewport(restore);
    if ((cp.width > 0) && (cp.height > 0))
      viewport.setVisible(true);
    if (Utils.isX11())
      viewport.x11FullScreenHelper(opts.fullScreen);
    if (opts.fullScreen && viewport.lionFSSupported())
      viewport.toggleLionFS();
    desktop.requestFocusInWindow();
    if (shouldGrab())
      viewport.grabKeyboardHelper(true);
    selectGrab(VncViewer.isKeyboardGrabbed(viewport));
    if (Utils.osEID())
      viewport.setupExtInputHelper();
  }

  public static Rectangle getMaxSpannedSize(boolean workArea) {
    GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
    GraphicsDevice[] gsList = ge.getScreenDevices();
    Rectangle s0 = null;
    Rectangle span = new Rectangle(-1, -1, 0, 0);
    Insets in = new Insets(0, 0, 0, 0);
    int tLeft = 0, tTop = 0, tRight = 0, tBottom = 0;
    boolean equal = true;

    Toolkit tk = Toolkit.getDefaultToolkit();

    for (GraphicsDevice gs : gsList) {
      GraphicsConfiguration[] gcList = gs.getConfigurations();
      for (GraphicsConfiguration gc : gcList) {
        Rectangle s = gc.getBounds();
        if (workArea) {
          if (gc == gcList[0])
            in = tk.getScreenInsets(gc);
          s.setBounds(s.x + in.left, s.y + in.top,
                      s.width - in.left - in.right,
                      s.height - in.top - in.bottom);
        }
        if (s0 == null) {
          s0 = s;
          span.setBounds(s);
          tLeft = s.x;  tTop = s.y;
          tRight = s.x + s.width;  tBottom = s.y + s.height;
        }

        tLeft = Math.min(tLeft, s.x);
        tRight = Math.max(tRight, s.x + s.width);
        tTop = Math.min(tTop, s.y);
        tBottom = Math.max(tBottom, s.y + s.height);

        // If any monitors aren't equal in resolution to and evenly offset from
        // the primary, then we can't use the simple path.
        if (s.width != s0.width ||
            s.height != s0.height ||
            (Math.abs(s.y - s0.y) % s0.height) != 0 ||
            (Math.abs(s.x - s0.x) % s0.width) != 0)
            equal = false;

        // If the screen area of this monitor overlaps vertically with the
        // multi-screen span area, then allow the window to extend horizontally
        // to this monitor, and constrain it vertically, if necessary, to fit
        // within this monitor's dimensions.
        if (Math.min(s.y + s.height, span.y + span.height) -
            Math.max(s.y, span.y) > 0 &&
            (s.x + s.width == span.x || span.x + span.width == s.x)) {
          int right = Math.max(s.x + s.width, span.x + span.width);
          int bottom = Math.min(s.y + s.height, span.y + span.height);
          span.x = Math.min(s.x, span.x);
          span.y = Math.max(s.y, span.y);
          span.width = right - span.x;
          span.height = bottom - span.y;
        }
      }
    }

    if (equal)
      span = new Rectangle(tLeft, tTop, tRight - tLeft, tBottom - tTop);

    return span;
  }


  // EDT
  public Rectangle getSpannedSize() {
    boolean fullScreenWindow = opts.fullScreen &&
                               (!Utils.isMac() || viewport.lionFSSupported());
    GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
    GraphicsDevice[] gsList = ge.getScreenDevices();
    Rectangle primary = null, s0 = null;
    Rectangle span = new Rectangle(-1, -1, 0, 0);
    Insets in = new Insets(0, 0, 0, 0);
    int tLeft = Integer.MAX_VALUE, tTop = Integer.MAX_VALUE,
      tRight = Integer.MIN_VALUE, tBottom = Integer.MIN_VALUE;
    int primaryID = 0;
    boolean equal = true;
    int sw = desktop.scaledWidth;
    int sh = desktop.scaledHeight;

    viewport.leftMon = viewport.rightMon = viewport.topMon =
      viewport.bottomMon = 0;

    if (opts.scalingFactor == Options.SCALE_AUTO ||
        opts.scalingFactor == Options.SCALE_FIXEDRATIO) {
      sw = cp.width;
      sh = cp.height;
    }

    Toolkit tk = Toolkit.getDefaultToolkit();

    int i = 0, maxArea = 0;
    boolean primaryIsCurrent = false;
    for (GraphicsDevice gs : gsList) {
      GraphicsConfiguration[] gcList = gs.getConfigurations();
      for (GraphicsConfiguration gc : gcList) {
        Rectangle s = gc.getBounds();
        if (!fullScreenWindow) {
          if (gc == gcList[0])
            in = tk.getScreenInsets(gc);
          s.setBounds(s.x + in.left, s.y + in.top,
                      s.width - in.left - in.right,
                      s.height - in.top - in.bottom);
        }
        if (s0 == null) {
          s0 = s;
          span.setBounds(s);
          tLeft = s.x;  tTop = s.y;
          tRight = s.x + s.width;  tBottom = s.y + s.height;
        }
        if (s.x >= 0 && s.y >= 0 && !primaryIsCurrent &&
            (primary == null ||
             (gc == gcList[0] && ((s.y < primary.y &&
                                   s.x < primary.x + primary.width) ||
                                  (s.x < primary.x &&
                                   s.y < primary.y + primary.height))))) {
          primary = s;
          primaryGD = gs;
          primaryID = i;
        }
        if (Params.currentMonitorIsPrimary.getValue() && viewport != null) {
          Rectangle vpRect = oldViewportBounds != null ? oldViewportBounds :
                             viewport.getBounds();
          if (opts.fullScreen && savedRect.width > 0 && savedRect.height > 0)
            vpRect = savedRect;
          vpRect = s.intersection(vpRect);
          int area = vpRect.isEmpty() ? 0 : vpRect.width * vpRect.height;
          if (area > maxArea) {
            maxArea = area;
            primary = s;
            primaryGD = gs;
            primaryID = i;
            primaryIsCurrent = true;
          }
        }

        if (s.x < tLeft) {
          tLeft = s.x;
          viewport.leftMon = i;
        }
        if (s.x + s.width > tRight) {
          tRight = s.x + s.width;
          viewport.rightMon = i;
        }
        if (s.y < tTop) {
          tTop = s.y;
          viewport.topMon = i;
        }
        if (s.y + s.height > tBottom) {
          tBottom = s.y + s.height;
          viewport.bottomMon = i;
        }

        if (gc == gcList[0])
          vlog.debug("Screen " + i++ + (fullScreenWindow ? " FS " : " work ") +
                     "area: " + s.x + ", " + s.y + " " + s.width + " x " +
                     s.height);

        // If any monitors aren't equal in resolution to and evenly offset from
        // the primary, then we can't use the simple path.
        if (s.width != s0.width ||
            s.height != s0.height ||
            (Math.abs(s.y - s0.y) % s0.height) != 0 ||
            (Math.abs(s.x - s0.x) % s0.width) != 0)
          equal = false;

        // If the screen/work area of this monitor overlaps vertically with the
        // multi-screen span area, then allow the window to extend horizontally
        // to this monitor, and constrain it vertically, if necessary, to fit
        // within this monitor's dimensions.
        if (Math.min(s.y + s.height, span.y + span.height) -
            Math.max(s.y, span.y) > 0 &&
            (s.x + s.width == span.x || span.x + span.width == s.x)) {
          int right = Math.max(s.x + s.width, span.x + span.width);
          int bottom = Math.min(s.y + s.height, span.y + span.height);
          span.x = Math.min(s.x, span.x);
          span.y = Math.max(s.y, span.y);
          span.width = right - span.x;
          span.height = bottom - span.y;
        }
      }
    }

    // Enable Primary spanning if explicitly selected, or ...
    if (opts.span == Options.SPAN_PRIMARY ||
        // Automatic spanning + Manual or Server resizing is enabled and the
        // server desktop fits on the primary monitor, or ...
        (opts.span == Options.SPAN_AUTO &&
         opts.desktopSize.mode != Options.SIZE_AUTO &&
         (sw <= primary.width || span.width <= primary.width) &&
         (sh <= primary.height || span.height <= primary.height)) ||
        // Automatic spanning + Auto resizing is enabled and we're in windowed
        // mode, or ...
        (opts.span == Options.SPAN_AUTO &&
         opts.desktopSize.mode == Options.SIZE_AUTO && !opts.fullScreen) ||
        // We're using X11, and we're in windowed mode or the helper library
        // isn't available (multi-screen spanning doesn't even pretend to work
        // under X11 except for full-screen windows, and even then, the
        // appropriate WM hints must be set using C.)
        (Utils.isX11() && (!opts.fullScreen || !Helper.isAvailable()))) {
      span = primary;
      viewport.leftMon = viewport.rightMon = viewport.topMon =
        viewport.bottomMon = primaryID;
    } else if (equal ||
               (fullScreenWindow && serverXinerama &&
                opts.desktopSize.mode == Options.SIZE_AUTO && !Utils.isX11()))
      span = new Rectangle(tLeft, tTop, tRight - tLeft, tBottom - tTop);

    vlog.debug("Spanned " + (fullScreenWindow ? "FS " : "work ") + "area: " +
               span.x + ", " + span.y + " " + span.width + " x " +
               span.height);

    oldViewportBounds = null;

    return span;
  }

  // EDT: Resize window based on the spanning option
  public void sizeWindow() { sizeWindow(true); }

  public void sizeWindow(boolean manual) {
    int w = desktop.scaledWidth;
    int h = desktop.scaledHeight;
    Rectangle span = getSpannedSize();
    Dimension vpSize;

    if ((opts.scalingFactor == Options.SCALE_AUTO ||
         opts.scalingFactor == Options.SCALE_FIXEDRATIO) &&
        !opts.fullScreen) {
      w = cp.width;
      h = cp.height;
    }

    if (opts.desktopSize.mode == Options.SIZE_AUTO && manual) {
      w = span.width;
      h = span.height;
    }

    if (w >= span.width)
      w = span.width;
    if (h >= span.height)
      h = span.height;

    if (viewport.getExtendedState() != JFrame.ICONIFIED && !Utils.isMac())
      viewport.setExtendedState(JFrame.NORMAL);
    int x = (span.width - w) / 2 + span.x;
    int y = (span.height - h) / 2 + span.y;
    if (opts.fullScreen) {
      java.awt.Point vpPos = viewport.getLocation();
      boolean checkLayoutNow = false;
      vpSize = viewport.getSize();

      // If the window size is unchanged, check whether we need to force a
      // desktop resize message to inform the server of a new screen layout
      if (opts.desktopSize.mode == Options.SIZE_AUTO && manual &&
          span.width == vpSize.width && span.height == vpSize.height) {
        if (vpPos.x != span.x || vpPos.y != span.y)
          checkLayout = true;
        else
          // The component listener is unlikely to be called, so we need to
          // check the layout immediately.
          checkLayoutNow = true;
      }

      viewport.setGeometry(span.x, span.y, span.width, span.height);
      viewport.dx = x - span.x;
      viewport.dy = y - span.y;

      if (checkLayoutNow) {
        ScreenSet layout = computeScreenLayout(span.width, span.height);
        if (!layout.equals(cp.screenLayout))
          sendDesktopSize(span.width, span.height, layout, false);
      }

      return;
    }

    viewport.dx = viewport.dy = 0;
    viewport.adjustWidth = viewport.adjustHeight = 0;

    Dimension vpBorder = viewport.getBorderSize();
    if (vpBorder.width > 0 || vpBorder.height > 0) {
      w += vpBorder.width;
      h += vpBorder.height;
      if (w >= span.width)
        w = span.width;
      if (h >= span.height)
        h = span.height;
      x = (span.width - w) / 2 + span.x;
      y = (span.height - h) / 2 + span.y;
    }

    // Attempt to prevent cases in which the presence of one scrollbar will
    // unnecessarily cause the other scrollbar to appear
    if (opts.desktopSize.mode != Options.SIZE_AUTO &&
        opts.scalingFactor != Options.SCALE_FIXEDRATIO &&
        opts.scalingFactor != Options.SCALE_AUTO) {
      int clientw = w - vpBorder.width, clienth = h - vpBorder.height;
      int sbWidth = UIManager.getInt("ScrollBar.width");
      if (desktop.scaledWidth > clientw && h + sbWidth <= span.height) {
        h += sbWidth;
        y = (span.height - h) / 2 + span.y;
        viewport.adjustHeight = sbWidth;
      } else if (desktop.scaledHeight > clienth && w + sbWidth <= span.width) {
        w += sbWidth;
        x = (span.width - w) / 2 + span.x;
        viewport.adjustWidth = sbWidth;
      }
    }

    vpSize = viewport.getSize();
    // If the window size is unchanged, check whether we need to force a
    // desktop resize message to inform the server of a new screen layout
    if (opts.desktopSize.mode == Options.SIZE_AUTO && manual &&
        w == vpSize.width && h == vpSize.height)
      checkLayout = true;

    viewport.setGeometry(x, y, w, h);
  }

  // EDT
  private void reconfigureViewport(boolean restore) {
    desktop.setScaledSize();
    if (!opts.fullScreen && savedRect.width > 0 && savedRect.height > 0 &&
        restore) {
      if (savedState >= 0)
        viewport.setExtendedState(savedState);
      viewport.setGeometry(savedRect.x, savedRect.y, savedRect.width,
                           savedRect.height);
    } else {
      sizeWindow(false);
    }
  }

  private void reconfigureAndRepaintViewport(boolean restore) {
    reconfigureViewport(false);
    // The viewport's componentResized() method isn't guaranteed to be called
    // when reconfiguring the viewport, and it generally won't be called if
    // only the scaling factor has changed while in full-screen mode.  Thus, we
    // need to ensure that the appropriate scrollbar policy is set and the
    // viewport is repainted.
    if (desktop != null) {
      if (opts.scalingFactor == Options.SCALE_AUTO ||
          opts.scalingFactor == Options.SCALE_FIXEDRATIO) {
        viewport.sp.setHorizontalScrollBarPolicy(
          ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
        viewport.sp.setVerticalScrollBarPolicy(
          ScrollPaneConstants.VERTICAL_SCROLLBAR_NEVER);
      } else {
        viewport.sp.setHorizontalScrollBarPolicy(
          ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        viewport.sp.setVerticalScrollBarPolicy(
          ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED);
      }
      viewport.sp.validate();
      desktop.resize();
      desktop.paintImmediately(desktop.getBounds());
    }
  }

  // RFB thread: requestNewUpdate() requests an update from the server, having
  // set the format and encoding appropriately.
  private void requestNewUpdate() {
    if (formatChange) {
      PixelFormat pf;

      // Catch incorrect requestNewUpdate calls
      assert(!pendingUpdate || supportsSyncFence);

      if (Params.colors.getValue() == 8) {
        pf = VERY_LOW_COLOR_PF;
      } else if (Params.colors.getValue() == 64) {
        pf = LOW_COLOR_PF;
      } else if (Params.colors.getValue() == 256) {
        pf = MEDIUM_COLOR_PF;
      } else if (Params.colors.getValue() == 32768) {
        pf = MEDIUMHIGH_COLOR_PF;
      } else if (Params.colors.getValue() == 65536) {
        pf = HIGH_COLOR_PF;
      } else {
        pf = fullColourPF;
      }

      if (supportsSyncFence) {
        // We let the fence carry the pixel format change and make the switch
        // once we get the response back.  That way, we will be synchronised
        // with the format change on the server end.
        MemOutStream memStream = new MemOutStream();

        pf.write(memStream);

        writer().writeFence(RFB.FENCE_FLAG_REQUEST | RFB.FENCE_FLAG_SYNC_NEXT,
                            memStream.length(), (byte[])memStream.data());
      } else {
        // New update requests are sent out before processing the last update,
        // so we cannot switch our internal format right now (doing so would
        // mean incorrectly decoding the current update.)
        pendingPFChange = true;
        pendingPF = pf;
      }

      String str = pf.print();
      vlog.info("Using pixel format " + str);
      writer().writeSetPixelFormat(pf);

      formatChange = false;
    }

    checkEncodings();

    if (forceNonincremental || !continuousUpdates) {
      pendingUpdate = true;
      writer().writeFramebufferUpdateRequest(
        new Rect(0, 0, cp.width, cp.height),
        !formatChange && !forceNonincremental);
    }

    forceNonincremental = false;
  }


  ////////////////////////////////////////////////////////////////////
  // The following methods are all called from the EDT.

  public boolean confirmClose() {
    if (Params.confirmClose.getValue() && state() == RFBSTATE_NORMAL &&
        !shuttingDown && sock != null) {
      JOptionPane pane;
      Object[] dlgOptions = { UIManager.getString("OptionPane.yesButtonText"),
                              UIManager.getString("OptionPane.noButtonText") };

      int port = sock.getPeerPort();
      String name = (port >= 5900 && port <= 5999 ?
                     sock.getPeerName() + ":" + (port - 5900) :
                     sock.getPeerName() + "::" + port);
      pane = new JOptionPane("Are you sure you want to close the\n" +
                             "connection to " + name + "?",
                             JOptionPane.WARNING_MESSAGE,
                             JOptionPane.YES_NO_OPTION, null, dlgOptions,
                             dlgOptions[1]);
      JDialog dlg = pane.createDialog(null, "TurboVNC Viewer");
      dlg.setAlwaysOnTop(true);
      dlg.setVisible(true);
      if (pane.getValue() == dlgOptions[1])
        return false;
    }
    return true;
  }

  // close() shuts down the socket, thus waking up the RFB thread.
  public void close() { close(true); }

  public void close(boolean disposeViewport) {
    if (disposeViewport && !confirmClose()) return;
    deleteWindow(disposeViewport);
    shuttingDown = true;
    if (sock != null)
      sock.shutdown();
    if (opts.sshSession != null) {
      opts.sshSession.disconnect();
      opts.sshSession = null;
    }
    if (reader != null)
      reader.close();
  }

  public void closeSocket() {
    if (sock != null) {
      sock.close();
      sock = null;
    }
  }

  // Menu callbacks.  These are guaranteed only to be called after serverInit()
  // has been called, since the menu is only accessible from the DesktopWindow.

  void showMenu(int x, int y) {
    if (Utils.isWindows())
      UIManager.put("Button.showMnemonics", true);
    if (viewport != null && (viewport.dx > 0 || viewport.dy > 0)) {
      x += viewport.dx;
      y += viewport.dy;
    }
    menu.show(desktop, x, y);
  }

  void showAbout() {
    VncViewer.showAbout(viewport);
  }

  String getEncryptionProtocol() {
    String protocol = csecurity.getProtocol();
    if (protocol.equals("None") && opts.sshTunnelActive)
      return "SSH";
    else
      return protocol + (opts.sshTunnelActive ? " (+ SSH)" : "");
  }

  void showInfo() {
    JOptionPane.showMessageDialog(viewport,
      "Desktop name:  " + cp.name() + "\n" +
      "Host:  " + sock.getPeerName() + "::" + sock.getPeerPort() + "\n" +
      "Size:  " + cp.width + "x" + cp.height + "\n" +
      "Pixel format:  " + desktop.getPF().print() + "\n" +
      "(server default " + serverPF.print() + ")\n" +
      "Requested encoding:  " + RFB.encodingName(currentEncoding) + "\n" +
      "Last used encoding:  " + RFB.encodingName(lastServerEncoding) + "\n" +
      "Protocol version:  " + cp.majorVersion + "." + cp.minorVersion + "\n" +
      "Security type:  " + RFB.secTypeName(csecurity.getType()) +
        " [" + csecurity.getDescription() + "]\n" +
      "Encryption protocol:  " + getEncryptionProtocol() + "\n" +
      "JPEG decompression:  " +
        (reader.isTurboJPEG() ? "Turbo" : "Unaccelerated") +
      (Utils.osGrab() || Utils.osEID() ? "\nTurboVNC Helper:  " +
        (Helper.isAvailable() ? "Loaded" : "Not found") : ""),
      "VNC connection info", JOptionPane.PLAIN_MESSAGE);
  }

  public void refresh() {
    writer().writeFramebufferUpdateRequest(new Rect(0, 0, cp.width, cp.height),
                                           false);
    pendingUpdate = true;
  }

  public void losslessRefresh() {
    int currentEncodingSave = currentEncoding;
    int compressLevelSave = opts.compressLevel;
    int qualitySave = opts.quality;
    boolean allowJpegSave = opts.allowJpeg;
    boolean alreadyLossless = false;

    if (currentEncoding == RFB.ENCODING_TIGHT && opts.compressLevel == 1 &&
        opts.quality == -1 && !opts.allowJpeg)
      alreadyLossless = true;

    if (!alreadyLossless) {
      currentEncoding = RFB.ENCODING_TIGHT;
      opts.compressLevel = 1;
      opts.quality = -1;
      opts.allowJpeg = false;
      encodingChange = true;
      checkEncodings();
    }
    refresh();
    if (!alreadyLossless) {
      currentEncoding = currentEncodingSave;
      opts.compressLevel = compressLevelSave;
      opts.quality = qualitySave;
      opts.allowJpeg = allowJpegSave;
      encodingChange = true;
      checkEncodings();
    }
  }


  // OptionsDialogCallback.  setOptions() and getOptions() are both called from
  // the EDT.

  public boolean isUnixLoginForced() {
    return (opts.user != null || opts.sendLocalUsername);
  }

  public void setTightOptions() {
    int encoding = currentEncoding;
    if (lastServerEncoding != RFB.ENCODING_TIGHT && lastServerEncoding >= 0)
      encoding = lastServerEncoding;
    options.setTightOptions(encoding);
  }

  public void setOptions() {
    options.setOptions(opts, cp.supportsSetDesktopSize || firstUpdate,
                       state() == RFBSTATE_NORMAL, state() == RFBSTATE_NORMAL,
                       state() == RFBSTATE_NORMAL);
    setTightOptions();
    if (opts.scalingFactor != Options.SCALE_AUTO &&
        opts.scalingFactor != Options.SCALE_FIXEDRATIO && desktop != null)
      desktop.setScaledSize();
  }

  public void getOptions() {
    boolean recreate = false, reconfigure = false, deleteRestore = false;

    Options oldOpts = new Options(opts);

    options.getOptions(opts);

    if (opts.allowJpeg != oldOpts.allowJpeg ||
        opts.quality != oldOpts.quality ||
        opts.compressLevel != oldOpts.compressLevel ||
        opts.subsampling != oldOpts.subsampling)
      encodingChange = true;

    if (opts.viewOnly != oldOpts.viewOnly && opts.showToolbar &&
        !opts.fullScreen)
      recreate = true;

    if (state() != RFBSTATE_NORMAL)
      opts.showToolbar = opts.showToolbar && !benchmark;

    if (opts.showToolbar != oldOpts.showToolbar && !opts.fullScreen)
      recreate = true;

    if (desktop != null && opts.scalingFactor != oldOpts.scalingFactor) {
      deleteRestore = true;
      savedState = -1;
      savedRect = new Rectangle(-1, -1, 0, 0);
      // Ideally we could recreate the viewport on all platforms, but due to a
      // Java bug, doing so on macOS causes the viewer to exit full-screen
      // mode.
      if (!viewport.lionFSSupported() || !opts.fullScreen)
        recreate = true;
      else
        reconfigure = true;
    }

    if (desktop != null &&
        !opts.desktopSize.equalsIgnoreID(oldOpts.desktopSize)) {
      deleteRestore = true;
      savedState = -1;
      savedRect = new Rectangle(-1, -1, 0, 0);
      if (opts.desktopSize.mode != Options.SIZE_SERVER) {
        if (!viewport.lionFSSupported() || !opts.fullScreen)
          recreate = true;
        else
          reconfigure = true;
        firstUpdate = true;
      }
    }

    if (desktop != null && opts.span != oldOpts.span) {
      deleteRestore = true;
      savedState = -1;
      savedRect = new Rectangle(-1, -1, 0, 0);
      if (!viewport.lionFSSupported() || !opts.fullScreen)
        recreate = true;
      else
        reconfigure = true;
    }

    clipboardDialog.setSendingEnabled(opts.sendClipboard);
    menu.updateMenuKey(opts.menuKeyCode);

    if (Utils.osGrab() && Helper.isAvailable()) {
      boolean isGrabbed = VncViewer.isKeyboardGrabbed(viewport);
      if (viewport != null &&
          ((opts.grabKeyboard == Options.GRAB_ALWAYS && !isGrabbed) ||
           (opts.grabKeyboard == Options.GRAB_FS &&
            opts.fullScreen != isGrabbed))) {
        viewport.grabKeyboardHelper(!isGrabbed);
        selectGrab(!isGrabbed);
      }
    }

    setShared(opts.shared);
    if (opts.cursorShape != oldOpts.cursorShape) {
      encodingChange = true;
      if (desktop != null)
        desktop.resetLocalCursor();
    }

    checkEncodings();

    if (opts.fullScreen != oldOpts.fullScreen) {
      opts.fullScreen = !opts.fullScreen;
      toggleFullScreen();
    } else if (recreate)
      recreateViewport();
    else if (reconfigure)
      reconfigureAndRepaintViewport(false);
    if (opts.showToolbar != oldOpts.showToolbar) {
      if (viewport != null)
        viewport.showToolbar(opts.showToolbar);
      menu.showToolbar.setSelected(opts.showToolbar);
    }
    if (opts.viewOnly != oldOpts.viewOnly) {
      if (viewport != null)
        viewport.updateMacMenuViewOnly();
      menu.viewOnly.setSelected(opts.viewOnly);
    }
    if (opts.scalingFactor != oldOpts.scalingFactor ||
        !opts.desktopSize.equalsIgnoreID(oldOpts.desktopSize)) {
      if (viewport != null)
        viewport.updateMacMenuZoom();
      menu.updateZoom();
    }
    if (deleteRestore) {
      savedState = -1;
      savedRect = new Rectangle(-1, -1, 0, 0);
    }
    // Force a framebuffer update if we're initiating a manual or auto remote
    // desktop resize.  Otherwise, it won't occur until the mouse is moved or
    // something changes on the server (manual) or until the window is resized
    // (auto.)
    if ((encodingChange || firstUpdate) && state() == RFBSTATE_NORMAL) {
      forceNonincremental = true;
      requestNewUpdate();
    }

    opts.save();
    VncViewer.opts = new Options(opts);
  }

  public boolean supportsSetDesktopSize() {
    return cp.supportsSetDesktopSize || firstUpdate;
  }

  // EDT
  public void zoomIn() {
    if (opts.desktopSize.mode == Options.SIZE_AUTO ||
        opts.scalingFactor == Options.SCALE_AUTO ||
        opts.scalingFactor == Options.SCALE_FIXEDRATIO)
      return;

    int sf = opts.scalingFactor;
    if (sf < 100)
      sf = ((sf / 10) + 1) * 10;
    else if (sf >= 100 && sf <= 200)
      sf = ((sf / 25) + 1) * 25;
    else
      sf = ((sf / 50) + 1) * 50;
    if (sf > 400) sf = 400;
    opts.scalingFactor = sf;

    savedState = -1;
    savedRect = new Rectangle(-1, -1, 0, 0);
    if (!viewport.lionFSSupported() || !opts.fullScreen)
      recreateViewport();
    else
      reconfigureAndRepaintViewport(false);
    savedState = -1;
    savedRect = new Rectangle(-1, -1, 0, 0);
  }

  // EDT
  public void zoomOut() {
    if (opts.desktopSize.mode == Options.SIZE_AUTO ||
        opts.scalingFactor == Options.SCALE_AUTO ||
        opts.scalingFactor == Options.SCALE_FIXEDRATIO)
      return;

    int sf = opts.scalingFactor;
    if (sf <= 100)
      sf = (((sf + 9) / 10) - 1) * 10;
    else if (sf >= 100 && sf <= 200)
      sf = (((sf + 24) / 25) - 1) * 25;
    else
      sf = (((sf + 49) / 50) - 1) * 50;
    if (sf < 10) sf = 10;
    if (sf > 400) sf = 400;
    opts.scalingFactor = sf;

    savedState = -1;
    savedRect = new Rectangle(-1, -1, 0, 0);
    if (!viewport.lionFSSupported() || !opts.fullScreen)
      recreateViewport();
    else
      reconfigureAndRepaintViewport(false);
    savedState = -1;
    savedRect = new Rectangle(-1, -1, 0, 0);
  }

  // EDT
  public void zoom100() {
    if (opts.desktopSize.mode == Options.SIZE_AUTO ||
        opts.scalingFactor == Options.SCALE_AUTO ||
        opts.scalingFactor == Options.SCALE_FIXEDRATIO)
      return;

    opts.scalingFactor = 100;

    savedState = -1;
    savedRect = new Rectangle(-1, -1, 0, 0);
    if (!viewport.lionFSSupported() || !opts.fullScreen)
      recreateViewport();
    else
      reconfigureAndRepaintViewport(false);
    savedState = -1;
    savedRect = new Rectangle(-1, -1, 0, 0);
  }

  // EDT
  public void toggleToolbar() {
    if (opts.fullScreen)
      return;
    opts.showToolbar = !opts.showToolbar;
    if (viewport != null) {
      recreateViewport();
      viewport.showToolbar(opts.showToolbar);
    }
    menu.showToolbar.setSelected(opts.showToolbar);
  }

  // EDT
  public void toggleFullScreen() {
    opts.fullScreen = !opts.fullScreen;
    menu.fullScreen.setSelected(opts.fullScreen);
    if (viewport != null)
      recreateViewport(true);
  }

  // EDT
  public void toggleViewOnly() {
    opts.viewOnly = !opts.viewOnly;
    menu.viewOnly.setSelected(opts.viewOnly);
    if (viewport != null) {
      viewport.updateMacMenuViewOnly();
      if (opts.showToolbar && !opts.fullScreen)
        recreateViewport(true);
    }
  }

  // EDT
  public void resize(int x, int y, int w, int h) {
    if (viewport != null) {
      if (opts.fullScreen)
        toggleFullScreen();

      Dimension vpSize = viewport.getSize();

      // If the window size is unchanged, check whether we need to force a
      // desktop resize message to inform the server of a new screen layout
      if (opts.desktopSize.mode == Options.SIZE_AUTO && w == vpSize.width &&
          h == vpSize.height)
        checkLayout = true;
      viewport.setGeometry(x, y, w, h);
    }
  }

  // EDT
  public void screenshot() {
    JFileChooser fc =
      new JFileChooser(System.getProperty("user.home") + "/Desktop");
    SimpleDateFormat df =
      new SimpleDateFormat("yyyy-MM-dd_HH.mm.ss");
    fc.setDialogTitle("Save Remote Desktop Image");
    fc.setSelectedFile(new File("TurboVNC_Screenshot_" +
                       df.format(Calendar.getInstance().getTime()) + ".png"));
    int ret = fc.showSaveDialog(null);
    if (ret == JFileChooser.APPROVE_OPTION)
      desktop.screenshot(fc.getSelectedFile());
  }

  public boolean shouldGrab() {
    return Utils.osGrab() &&
           (opts.grabKeyboard == Options.GRAB_ALWAYS ||
            (opts.grabKeyboard == Options.GRAB_MANUAL && isGrabSelected()) ||
            (opts.grabKeyboard == Options.GRAB_FS && opts.fullScreen));
  }

  public void selectGrab(boolean on) {
    if (menu.grabKeyboard != null)
      menu.grabKeyboard.setSelected(on);
  }

  public boolean isGrabSelected() {
    if (menu.grabKeyboard != null)
      return menu.grabKeyboard.isSelected();
    return false;
  }

  // EDT
  public void toggleProfile() {
    menu.profile.setSelected(profileDialog.isVisible());
    if (viewport != null)
      viewport.updateMacMenuProfile();
  }

  // EDT: writeClientCutText() is called from the clipboard dialog.
  public void writeClientCutText(String str, int len) {
    if (state() != RFBSTATE_NORMAL || shuttingDown || benchmark)
      return;
    writer().writeClientCutText(str, len);
  }

  // EDT
  public void writeKeyEvent(int keysym, boolean down) {
    if (state() != RFBSTATE_NORMAL || shuttingDown || benchmark)
      return;
    try {
      writer().writeKeyEvent(keysym, down);
    } catch (Exception e) {
      if (!shuttingDown) {
        vlog.error("Error writing key event:");
        vlog.error("  " + e.toString());
      }
    }
  }

  // KeyEvent.getKeyModifiersText() is unfortunately broken on some platforms.
  String getKeyModifiersText() {
    String str = "";
    if (pressedKeys.containsValue(Keysyms.SHIFT_L))
      str += " LShift";
    if (pressedKeys.containsValue(Keysyms.SHIFT_R))
      str += " RShift";
    if (pressedKeys.containsValue(Keysyms.CONTROL_L))
      str += " LCtrl";
    if (pressedKeys.containsValue(Keysyms.CONTROL_R))
      str += " RCtrl";
    if (pressedKeys.containsValue(Keysyms.ALT_L))
      str += " LAlt";
    if (pressedKeys.containsValue(Keysyms.ALT_R))
      str += " RAlt";
    if (pressedKeys.containsValue(Keysyms.META_L))
      str += " LMeta";
    if (pressedKeys.containsValue(Keysyms.META_R))
      str += " RMeta";
    if (pressedKeys.containsValue(Keysyms.SUPER_L))
      str += " LSuper";
    if (pressedKeys.containsValue(Keysyms.SUPER_R))
      str += " RSuper";
    return str;
  }

  String getLocationText(int location) {
    switch (location) {
      case KeyEvent.KEY_LOCATION_LEFT:      return "LEFT";
      case KeyEvent.KEY_LOCATION_NUMPAD:    return "NUMPAD";
      case KeyEvent.KEY_LOCATION_RIGHT:     return "RIGHT";
      case KeyEvent.KEY_LOCATION_STANDARD:  return "STANDARD";
      case KeyEvent.KEY_LOCATION_UNKNOWN:   return "UNKNOWN";
      default:                              return Integer.toString(location);
    }
  }

  // EDT
  public void writeKeyEvent(KeyEvent ev) {
    int keysym = -1, keycode, key, location;
    boolean winAltGr = false;
    String debugStr;

    if (shuttingDown || benchmark)
      return;

    boolean down = (ev.getID() == KeyEvent.KEY_PRESSED);

    keycode = ev.getKeyCode();
    key = ev.getKeyChar();
    location = ev.getKeyLocation();

    debugStr = ((ev.isActionKey() ? "action " : "") + "key " +
                  (down ? "PRESS" : "release") +
                ", code " + KeyEvent.getKeyText(keycode) +
                  " (" + keycode + ")" +
                ", loc " + getLocationText(location) +
                ", char " +
                  (key >= 32 && key <= 126 ? "'" + (char)key + "'" : key) +
                getKeyModifiersText() + (ev.isAltGraphDown() ? " AltGr" : ""));

    // If neither the key code nor key char is defined, then there's really
    // nothing we can do with this.  The fn key on OS X fires events like this
    // when pressed but does not fire a corresponding release event.
    if (keycode == 0 && ev.getKeyChar() == KeyEvent.CHAR_UNDEFINED) {
      debugStr += " IGNORED";
      vlog.debug(debugStr);
      return;
    }

    if (!down) {
      Integer hashedKey = keycode;
      if (Utils.isMac()) {
        if (hashedKey == KeyEvent.VK_ALT_GRAPH)
          hashedKey = KeyEvent.VK_ALT;
      } else
        hashedKey |= (location << 16);
      Integer sym = pressedKeys.get(hashedKey);

      if (sym == null) {
        // Note that dead keys will raise this sort of error falsely
        // See https://bugs.openjdk.java.net/browse/JDK-6534883
        debugStr += " UNEXPECTED/IGNORED";
        vlog.debug(debugStr);
        return;
      }

      // Work around a Windows bug whereby the O/S does not send a WM_KEYUP
      // message if both Shift keys are pressed and only one is released.  If
      // we receive a key release event for one Shift key and the other is in
      // the pressed keys hash, we release both of them.
      if (Utils.isWindows()) {
        if (sym == Keysyms.SHIFT_R &&
            pressedKeys.containsValue(Keysyms.SHIFT_L)) {
          writeKeyEvent(Keysyms.SHIFT_L, down);
          pressedKeys.remove(Keysyms.SHIFT_L);
        }
        if (sym == Keysyms.SHIFT_L &&
            pressedKeys.containsValue(Keysyms.SHIFT_R)) {
          writeKeyEvent(Keysyms.SHIFT_R, down);
          pressedKeys.remove(Keysyms.SHIFT_R);
        }
      }

      writeKeyEvent(sym, false);
      pressedKeys.remove(hashedKey);
      debugStr += String.format(" => 0x%04x", sym);
      vlog.debug(debugStr);
      return;
    }

    if (!ev.isActionKey()) {
      if (keycode >= KeyEvent.VK_0 && keycode <= KeyEvent.VK_9 &&
        location == KeyEvent.KEY_LOCATION_NUMPAD)
        keysym = Keysyms.KP_0 + keycode - KeyEvent.VK_0;

      switch (keycode) {
        case KeyEvent.VK_BACK_SPACE:  keysym = Keysyms.BACKSPACE;  break;
        case KeyEvent.VK_TAB:         keysym = Keysyms.TAB;  break;
        case KeyEvent.VK_ENTER:
          if (location == KeyEvent.KEY_LOCATION_NUMPAD)
            keysym = Keysyms.KP_ENTER;
          else
            keysym = Keysyms.RETURN;
          break;
        case KeyEvent.VK_ESCAPE:      keysym = Keysyms.ESCAPE;  break;
        case KeyEvent.VK_NUMPAD0:     keysym = Keysyms.KP_0;  break;
        case KeyEvent.VK_NUMPAD1:     keysym = Keysyms.KP_1;  break;
        case KeyEvent.VK_NUMPAD2:     keysym = Keysyms.KP_2;  break;
        case KeyEvent.VK_NUMPAD3:     keysym = Keysyms.KP_3;  break;
        case KeyEvent.VK_NUMPAD4:     keysym = Keysyms.KP_4;  break;
        case KeyEvent.VK_NUMPAD5:     keysym = Keysyms.KP_5;  break;
        case KeyEvent.VK_NUMPAD6:     keysym = Keysyms.KP_6;  break;
        case KeyEvent.VK_NUMPAD7:     keysym = Keysyms.KP_7;  break;
        case KeyEvent.VK_NUMPAD8:     keysym = Keysyms.KP_8;  break;
        case KeyEvent.VK_NUMPAD9:     keysym = Keysyms.KP_9;  break;
        case KeyEvent.VK_SEPARATOR:   keysym = Keysyms.KP_SEPARATOR;  break;
        case KeyEvent.VK_DECIMAL:
          // Use XK_KP_Separator instead of XK_KP_Decimal if the current key
          // map uses a comma rather than period as a decimal symbol.
          if (key == ',')
            keysym = Keysyms.KP_SEPARATOR;
          else
            keysym = Keysyms.KP_DECIMAL;
          break;
        case KeyEvent.VK_ADD:         keysym = Keysyms.KP_ADD;  break;
        case KeyEvent.VK_SUBTRACT:    keysym = Keysyms.KP_SUBTRACT;  break;
        case KeyEvent.VK_MULTIPLY:    keysym = Keysyms.KP_MULTIPLY;  break;
        case KeyEvent.VK_DIVIDE:      keysym = Keysyms.KP_DIVIDE;  break;
        case KeyEvent.VK_DELETE:
          if (location == KeyEvent.KEY_LOCATION_NUMPAD)
            keysym = Keysyms.KP_DELETE;
          else
            keysym = Keysyms.DELETE;
          break;
        case KeyEvent.VK_CLEAR:
          if (location == KeyEvent.KEY_LOCATION_NUMPAD)
            keysym = Keysyms.KP_BEGIN;
          else
            keysym = Keysyms.CLEAR;
          break;
        case KeyEvent.VK_CONTROL:
          if (location == KeyEvent.KEY_LOCATION_RIGHT)
            keysym = Keysyms.CONTROL_R;
          else
            keysym = Keysyms.CONTROL_L;
          break;
        case KeyEvent.VK_ALT:
          if (location == KeyEvent.KEY_LOCATION_RIGHT) {
            // Mac has no AltGr key, but the Option/Alt keys serve the same
            // purpose.  Thus, we allow RAlt to be used as AltGr and LAlt to be
            // used as a regular Alt key.
            if (Utils.isMac())
              keysym = Keysyms.ISO_LEVEL3_SHIFT;
            else
              keysym = Keysyms.ALT_R;
          } else {
            keysym = Keysyms.ALT_L;
          }
          break;
        case KeyEvent.VK_SHIFT:
          if (location == KeyEvent.KEY_LOCATION_RIGHT)
            keysym = Keysyms.SHIFT_R;
          else
            keysym = Keysyms.SHIFT_L;
          break;
        case KeyEvent.VK_META:
          if (location == KeyEvent.KEY_LOCATION_RIGHT)
            keysym = Keysyms.SUPER_R;
          else
            keysym = Keysyms.SUPER_L;
          break;
        case KeyEvent.VK_ALT_GRAPH:
          keysym = Keysyms.ISO_LEVEL3_SHIFT;
          break;
        default:
          // On Windows, pressing AltGr has the same effect as pressing LCtrl +
          // RAlt, so we have to send fake key release events for those
          // modifiers (and any other Ctrl and Alt modifiers that are pressed),
          // then send the key event for the modified key, then send fake key
          // press events for the same modifiers.
          if (pressedKeys.containsValue(Keysyms.ALT_R) &&
              pressedKeys.containsValue(Keysyms.CONTROL_L) &&
              Utils.isWindows()) {
            winAltGr = true;
          } else if (ev.isControlDown()) {
            // For CTRL-<letter>, CTRL is sent separately, so just send
            // <letter>.
            if ((key >= 1 && key <= 26 && !ev.isShiftDown()) ||
                // CTRL-{, CTRL-|, CTRL-} also map to ASCII 96-127
                (key >= 27 && key <= 29 && ev.isShiftDown()))
              key += 96;
            // For CTRL-SHIFT-<letter>, send capital <letter> to emulate the
            // behavior of Linux.  For CTRL-@, send @.  For CTRL-_, send _.
            // For CTRL-^, send ^.
            else if (key < 32)
              key += 64;
            // Windows and Mac sometimes return CHAR_UNDEFINED with CTRL-SHIFT
            // combinations.
            else if (key == KeyEvent.CHAR_UNDEFINED) {
              // CTRL-SHIFT-Minus should generate an underscore key character
              // in almost all keyboard layouts.  Emacs, in particular, uses
              // CTRL-SHIFT-Underscore for its undo function.
              if (keycode == KeyEvent.VK_MINUS && ev.isShiftDown())
                key = '_';
              // The best we can do for other keys is to send the key code if
              // it is a valid ASCII symbol.
              else if (keycode >= 0 && keycode <= 127)
                key = keycode;
            }
          } else if (pressedKeys.containsValue(Keysyms.ALT_L) &&
                     Utils.isMac() && key > 127) {
            // Un*x and Windows servers expect that, if Alt + an ASCII key is
            // pressed, the key event for the ASCII key will be the same as if
            // Alt had not been pressed.  On OS X, however, the Alt/Option keys
            // act like AltGr keys, so if Alt + an ASCII key is pressed, the
            // key code is the ASCII key symbol, but the key char is the code
            // for the alternate graphics symbol.
            if (keycode >= 65 && keycode <= 90 &&
                !pressedKeys.containsValue(Keysyms.SHIFT_L) &&
                !pressedKeys.containsValue(Keysyms.SHIFT_R))
              key = keycode + 32;
            else if (keycode == KeyEvent.VK_QUOTE)
              key = '\'';
            else if (keycode >= 32 && keycode <= 126)
              key = keycode;
          }
          switch (keycode) {
            // NOTE: For keyboard layouts that produce a different symbol when
            // AltGr+{a dead key} is pressed, Java tends to send us the key
            // code for the dead key.  It is difficult to distinguish those key
            // events from key events in which the dead key itself is pressed.
            // Fortunately, it seems that Java usually accompanies actual dead
            // key events with a key character corresponding to the equivalent
            // ASCII or ISO-8859-1 symbol (e.g. '^' for a dead circumflex) or
            // KeyEvent.CHAR_UNDEFINED.  We assume that, if we receive the key
            // code for a dead key, the key character is an ASCII/ISO-8859-1
            // symbol, and it doesn't match the equivalent ASCII/ISO-8859-1
            // symbol for the dead key, we should send the keysym for the key
            // character instead of the keysym for the dead key.  We only do
            // this for the dead keys that are known to have AltGr symbols
            // associated with them in various QWERTY layouts.
            case KeyEvent.VK_DEAD_ABOVEDOT:
              if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
                keysym = Keysyms.DEAD_ABOVEDOT;
              break;
            case KeyEvent.VK_DEAD_ABOVERING:
              if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
                keysym = Keysyms.DEAD_ABOVERING;
              break;
            case KeyEvent.VK_DEAD_ACUTE:
              if (location != KeyEvent.KEY_LOCATION_UNKNOWN &&
                  (key == '\'' || key == 180 || key > 255))
                keysym = Keysyms.DEAD_ACUTE;
              break;
            case KeyEvent.VK_DEAD_BREVE:
              if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
                keysym = Keysyms.DEAD_BREVE;
              break;
            case KeyEvent.VK_DEAD_CARON:
              if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
                keysym = Keysyms.DEAD_CARON;
              break;
            case KeyEvent.VK_DEAD_CEDILLA:
              if (location != KeyEvent.KEY_LOCATION_UNKNOWN &&
                  (key == 184 || key > 255))
                keysym = Keysyms.DEAD_CEDILLA;
              break;
            case KeyEvent.VK_DEAD_CIRCUMFLEX:
              if (location != KeyEvent.KEY_LOCATION_UNKNOWN &&
                  (key == '^' || key > 255))
                keysym = Keysyms.DEAD_CIRCUMFLEX;
              break;
            case KeyEvent.VK_DEAD_DIAERESIS:
              if (location != KeyEvent.KEY_LOCATION_UNKNOWN &&
                  (key == '\"' || key == 168 || key > 255))
                keysym = Keysyms.DEAD_DIAERESIS;
              break;
            case KeyEvent.VK_DEAD_DOUBLEACUTE:
              if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
                keysym = Keysyms.DEAD_DOUBLEACUTE;
              break;
            case KeyEvent.VK_DEAD_GRAVE:
              if (location != KeyEvent.KEY_LOCATION_UNKNOWN &&
                  (key == '`' || key > 255))
                keysym = Keysyms.DEAD_GRAVE;
              break;
            case KeyEvent.VK_DEAD_IOTA:
              if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
                keysym = Keysyms.DEAD_IOTA;
              break;
            case KeyEvent.VK_DEAD_MACRON:
              if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
                keysym = Keysyms.DEAD_MACRON;
              break;
            case KeyEvent.VK_DEAD_OGONEK:
              if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
                keysym = Keysyms.DEAD_OGONEK;
              break;
            case KeyEvent.VK_DEAD_SEMIVOICED_SOUND:
              if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
                keysym = Keysyms.DEAD_SEMIVOICED_SOUND;
              break;
            case KeyEvent.VK_DEAD_TILDE:
              if (location != KeyEvent.KEY_LOCATION_UNKNOWN &&
                  (key == '~' || key > 255))
                keysym = Keysyms.DEAD_TILDE;
              break;
            case KeyEvent.VK_DEAD_VOICED_SOUND:
              if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
                keysym = Keysyms.DEAD_VOICED_SOUND;
              break;
          }
          if (keysym == -1)
            keysym = UnicodeToKeysym.ucs2keysym(key);
          if (keysym == -1) {
            debugStr += " NO KEYSYM";
            vlog.debug(debugStr);
            return;
          }
      }
    } else {
      // KEY_ACTION
      switch (keycode) {
        case KeyEvent.VK_HOME:
          if (location == KeyEvent.KEY_LOCATION_NUMPAD)
            keysym = Keysyms.KP_HOME;
          else
            keysym = Keysyms.HOME;
          break;
        case KeyEvent.VK_END:
          if (location == KeyEvent.KEY_LOCATION_NUMPAD)
            keysym = Keysyms.KP_END;
          else
            keysym = Keysyms.END;
          break;
        case KeyEvent.VK_PAGE_UP:
          if (location == KeyEvent.KEY_LOCATION_NUMPAD)
            keysym = Keysyms.KP_PAGE_UP;
          else
            keysym = Keysyms.PAGE_UP;
          break;
        case KeyEvent.VK_PAGE_DOWN:
          if (location == KeyEvent.KEY_LOCATION_NUMPAD)
            keysym = Keysyms.KP_PAGE_DOWN;
          else
            keysym = Keysyms.PAGE_DOWN;
          break;
        case KeyEvent.VK_UP:
          if (location == KeyEvent.KEY_LOCATION_NUMPAD)
            keysym = Keysyms.KP_UP;
          else
            keysym = Keysyms.UP;
          break;
        case KeyEvent.VK_DOWN:
          if (location == KeyEvent.KEY_LOCATION_NUMPAD)
            keysym = Keysyms.KP_DOWN;
          else
            keysym = Keysyms.DOWN;
          break;
        case KeyEvent.VK_LEFT:
          if (location == KeyEvent.KEY_LOCATION_NUMPAD)
            keysym = Keysyms.KP_LEFT;
          else
            keysym = Keysyms.LEFT;
          break;
        case KeyEvent.VK_RIGHT:
          if (location == KeyEvent.KEY_LOCATION_NUMPAD)
            keysym = Keysyms.KP_RIGHT;
          else
            keysym = Keysyms.RIGHT;
          break;
        case KeyEvent.VK_F1:            keysym = Keysyms.F1;  break;
        case KeyEvent.VK_F2:            keysym = Keysyms.F2;  break;
        case KeyEvent.VK_F3:            keysym = Keysyms.F3;  break;
        case KeyEvent.VK_F4:            keysym = Keysyms.F4;  break;
        case KeyEvent.VK_F5:            keysym = Keysyms.F5;  break;
        case KeyEvent.VK_F6:            keysym = Keysyms.F6;  break;
        case KeyEvent.VK_F7:            keysym = Keysyms.F7;  break;
        case KeyEvent.VK_F8:            keysym = Keysyms.F8;  break;
        case KeyEvent.VK_F9:            keysym = Keysyms.F9;  break;
        case KeyEvent.VK_F10:           keysym = Keysyms.F10;  break;
        case KeyEvent.VK_F11:           keysym = Keysyms.F11;  break;
        case KeyEvent.VK_F12:           keysym = Keysyms.F12;  break;
        case KeyEvent.VK_F13:           keysym = Keysyms.F13;  break;
        case KeyEvent.VK_HELP:          keysym = Keysyms.HELP;  break;
        case KeyEvent.VK_UNDO:          keysym = Keysyms.UNDO;  break;
        case KeyEvent.VK_AGAIN:         keysym = Keysyms.REDO;  break;
        case KeyEvent.VK_PRINTSCREEN:   keysym = Keysyms.PRINT;  break;
        case KeyEvent.VK_PAUSE:
          if (ev.isControlDown())
            keysym = Keysyms.BREAK;
          else
            keysym = Keysyms.PAUSE;
          break;
        case KeyEvent.VK_INSERT:
          if (location == KeyEvent.KEY_LOCATION_NUMPAD)
            keysym = Keysyms.KP_INSERT;
          else
            keysym = Keysyms.INSERT;
          break;
        case KeyEvent.VK_KP_DOWN:       keysym = Keysyms.KP_DOWN;  break;
        case KeyEvent.VK_KP_LEFT:       keysym = Keysyms.KP_LEFT;  break;
        case KeyEvent.VK_KP_RIGHT:      keysym = Keysyms.KP_RIGHT;  break;
        case KeyEvent.VK_KP_UP:         keysym = Keysyms.KP_UP;  break;
        case KeyEvent.VK_NUM_LOCK:      keysym = Keysyms.NUM_LOCK;  break;
        case KeyEvent.VK_WINDOWS:
          if (location == KeyEvent.KEY_LOCATION_RIGHT)
            keysym = Keysyms.SUPER_R;
          else
            keysym = Keysyms.SUPER_L;
          break;
        case KeyEvent.VK_CONTEXT_MENU:  keysym = Keysyms.MENU;  break;
        case KeyEvent.VK_SCROLL_LOCK:   keysym = Keysyms.SCROLL_LOCK;  break;
        case KeyEvent.VK_CAPS_LOCK:     keysym = Keysyms.CAPS_LOCK;  break;
        case KeyEvent.VK_BEGIN:
          if (location == KeyEvent.KEY_LOCATION_NUMPAD)
            keysym = Keysyms.KP_BEGIN;
          else
            keysym = Keysyms.BEGIN;
          break;
        default:
          debugStr += " NO KEYSYM";
          vlog.debug(debugStr);
          return;
      }
    }

    if (winAltGr) {
      if (pressedKeys.containsValue(Keysyms.CONTROL_L)) {
        vlog.debug("Fake L Ctrl released");
        writeKeyEvent(Keysyms.CONTROL_L, false);
      }
      if (pressedKeys.containsValue(Keysyms.ALT_L)) {
        vlog.debug("Fake L Alt released");
        writeKeyEvent(Keysyms.ALT_L, false);
      }
      if (pressedKeys.containsValue(Keysyms.CONTROL_R)) {
        vlog.debug("Fake R Ctrl released");
        writeKeyEvent(Keysyms.CONTROL_R, false);
      }
      if (pressedKeys.containsValue(Keysyms.ALT_R)) {
        vlog.debug("Fake R Alt released");
        writeKeyEvent(Keysyms.ALT_R, false);
      }
    }
    debugStr += String.format(" => 0x%04x", keysym);
    vlog.debug(debugStr);
    Integer hashedKey = keycode;
    // On Mac platforms, Java 11 assigns KeyEvent.VK_ALT_GRAPH to the right Alt
    // key, whereas previous versions of Java assigned KeyEvent.VK_ALT to the
    // same key.  However, Java 11 still exhibits the behavior described below
    // when LAlt and RAlt are pressed simultaneously, so for the purposes of
    // matching key release events to key press events, it is necessary to
    // treat the RAlt key as an Alt key rather than an AltGr key.
    if (Utils.isMac()) {
      if (hashedKey == KeyEvent.VK_ALT_GRAPH)
        hashedKey = KeyEvent.VK_ALT;
    // On Mac platforms, modifier key press/release events have a key code of 0
    // if another location of the same modifier key is already pressed, so for
    // the purposes of matching key release events to key press events, all
    // locations of a modifier key have to be treated as if they are the same
    // key.  On other platforms, we hash both the key code and location.
    } else
      hashedKey |= (location << 16);
    pressedKeys.put(hashedKey, keysym);
    writeKeyEvent(keysym, down);
    if (winAltGr) {
      if (pressedKeys.containsValue(Keysyms.CONTROL_L)) {
        vlog.debug("Fake L Ctrl pressed");
        writeKeyEvent(Keysyms.CONTROL_L, true);
      }
      if (pressedKeys.containsValue(Keysyms.ALT_L)) {
        vlog.debug("Fake L Alt pressed");
        writeKeyEvent(Keysyms.ALT_L, true);
      }
      if (pressedKeys.containsValue(Keysyms.CONTROL_R)) {
        vlog.debug("Fake R Ctrl pressed");
        writeKeyEvent(Keysyms.CONTROL_R, true);
      }
      if (pressedKeys.containsValue(Keysyms.ALT_R)) {
        vlog.debug("Fake R Alt pressed");
        writeKeyEvent(Keysyms.ALT_R, true);
      }
    }
  }


  // EDT
  public void writePointerEvent(MouseEvent ev) {
    if (state() != RFBSTATE_NORMAL || shuttingDown || benchmark)
      return;

    switch (ev.getID()) {
      case MouseEvent.MOUSE_PRESSED:
        switch (ev.getButton()) {
          case 1:
            buttonMask |= RFB.BUTTON1_MASK;  break;
          case 2:
            buttonMask |= RFB.BUTTON2_MASK;  break;
          case 3:
            buttonMask |= RFB.BUTTON3_MASK;  break;
          default:
            return;
        }
        vlog.debug("mouse PRESS, button " + ev.getButton() +
                   ", coords " + ev.getX() + "," + ev.getY());
        break;
      case MouseEvent.MOUSE_RELEASED:
        switch (ev.getButton()) {
          case 1:
            buttonMask &= ~RFB.BUTTON1_MASK;  break;
          case 2:
            buttonMask &= ~RFB.BUTTON2_MASK;  break;
          case 3:
            buttonMask &= ~RFB.BUTTON3_MASK;  break;
          default:
            return;
        }
        vlog.debug("mouse release, button " + ev.getButton() +
                   ", coords " + ev.getX() + "," + ev.getY());
        break;
    }

    if (cp.width != desktop.scaledWidth ||
        cp.height != desktop.scaledHeight) {
      int sx = (desktop.scaleWidthRatio == 1.00) ?
        ev.getX() : (int)Math.floor(ev.getX() / desktop.scaleWidthRatio);
      int sy = (desktop.scaleHeightRatio == 1.00) ?
        ev.getY() : (int)Math.floor(ev.getY() / desktop.scaleHeightRatio);
      ev.translatePoint(sx - ev.getX(), sy - ev.getY());
    }
    if (viewport != null && (viewport.dx > 0 || viewport.dy > 0)) {
      int dx = (int)Math.floor(viewport.dx / desktop.scaleWidthRatio);
      int dy = (int)Math.floor(viewport.dy / desktop.scaleHeightRatio);
      ev.translatePoint(-dx, -dy);
    }

    try {
      writer().writePointerEvent(new Point(ev.getX(), ev.getY()), buttonMask);
    } catch (Exception e) {
      if (!shuttingDown) {
        vlog.error("Error writing pointer event:");
        vlog.error("  " + e.toString());
      }
    }
  }


  // EDT
  public void writeWheelEvent(MouseWheelEvent ev) {
    if (state() != RFBSTATE_NORMAL || shuttingDown || benchmark)
      return;
    int x, y, wheelMask;
    int clicks = ev.getWheelRotation();
    if (opts.reverseScroll) {
      clicks = -clicks;
    }
    if (clicks < 0) {
      wheelMask = buttonMask | RFB.BUTTON4_MASK;
    } else {
      wheelMask = buttonMask | RFB.BUTTON5_MASK;
    }

    if (cp.width != desktop.scaledWidth ||
        cp.height != desktop.scaledHeight) {
      int sx = (desktop.scaleWidthRatio == 1.00) ?
        ev.getX() : (int)Math.floor(ev.getX() / desktop.scaleWidthRatio);
      int sy = (desktop.scaleHeightRatio == 1.00) ?
        ev.getY() : (int)Math.floor(ev.getY() / desktop.scaleHeightRatio);
      ev.translatePoint(sx - ev.getX(), sy - ev.getY());
    }
    if (viewport != null && (viewport.dx > 0 || viewport.dy > 0)) {
      int dx = (int)Math.floor(viewport.dx / desktop.scaleWidthRatio);
      int dy = (int)Math.floor(viewport.dy / desktop.scaleHeightRatio);
      ev.translatePoint(-dx, -dy);
    }
    for (int i = 0; i < Math.abs(clicks); i++) {
      x = ev.getX();
      y = ev.getY();
      try {
        writer().writePointerEvent(new Point(x, y), wheelMask);
        writer().writePointerEvent(new Point(x, y), buttonMask);
      } catch (Exception e) {
        if (!shuttingDown) {
          vlog.error("Error writing wheel event:");
          vlog.error("  " + e.toString());
        }
      }
    }
  }


  // EDT
  synchronized void releasePressedKeys() {
    for (Map.Entry<Integer, Integer> entry : pressedKeys.entrySet()) {
      vlog.debug(String.format("Lost focus.  Releasing key symbol 0x%04x",
                 entry.getValue()));
      writeKeyEvent(entry.getValue(), false);
    }
    pressedKeys.clear();
  }


  ////////////////////////////////////////////////////////////////////
  // The following methods are called from both the RFB thread and EDT.

  // checkEncodings() sends a setEncodings message if one is needed.
  private void checkEncodings() {
    if (encodingChange && (writer() != null)) {
      vlog.info("Requesting " + RFB.encodingName(currentEncoding) +
                " encoding");
      writer().writeSetEncodings(currentEncoding, lastServerEncoding, opts);
      encodingChange = false;
      if (viewport != null)
        viewport.updateTitle();
    }
  }

  // The following need no synchronization:
  VncViewer viewer;

  // shuttingDown is set in the EDT and is only ever tested by the RFB thread
  // after the window has been destroyed.
  boolean shuttingDown = false;

  // All menu, options, about and info stuff is done in the EDT (apart from
  // initial construction.)
  F8Menu menu;
  OptionsDialog options;

  // clipboard sync issues?
  ClipboardDialog clipboardDialog;

  int buttonMask;  // EDT only

  protected DesktopWindow desktop;

  PixelFormat serverPF;
  private PixelFormat fullColourPF;

  private boolean pendingPFChange;
  private PixelFormat pendingPF;
  boolean pendingServerResize;
  Rect pendingAutoResize = new Rect();

  int currentEncoding, lastServerEncoding;

  private boolean formatChange;
  private boolean encodingChange;

  boolean firstUpdate;
  private boolean pendingUpdate;
  private boolean continuousUpdates;
  boolean checkLayout;
  boolean serverXinerama;

  private boolean forceNonincremental;

  private boolean supportsSyncFence;

  private HashMap<Integer, Integer> pressedKeys;
  Viewport viewport;
  Rectangle oldViewportBounds;
  boolean keyboardGrabbed;
  GraphicsDevice primaryGD;

  double tDecode, tBlit;
  long decodePixels, decodeRect, blitPixels, blits;
  double tDecodeStart, tReadOld;
  boolean benchmark;

  double tStart = -1.0, tElapsed, tUpdateStart, tUpdate;
  long updates;
  ProfileDialog profileDialog;
  boolean alwaysProfile;

  static LogWriter vlog = new LogWriter("CConn");
}
