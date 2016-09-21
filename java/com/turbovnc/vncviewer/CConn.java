/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright 2009-2011 Pierre Ossman <ossman@cendio.se> for Cendio AB
 * Copyright (C) 2011-2016 D. R. Commander.  All Rights Reserved.
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
import java.lang.Exception;
import java.util.jar.Attributes;
import java.util.jar.Manifest;
import javax.swing.*;
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
  static final int SUPER_MASK = 1<<16;

  static final double getTime() {
    return (double)System.nanoTime() / 1.0e9;
  }

  // RFB thread
  public CConn(VncViewer viewer_, Socket sock_) {
    sock = sock_;  viewer = viewer_;
    benchmark = viewer.benchFile != null;
    pendingPFChange = false;
    lastServerEncoding = -1;

    opts = new Options(VncViewer.opts);

    formatChange = false; encodingChange = false;
    currentEncoding = opts.preferredEncoding;
    showToolbar = VncViewer.showToolbar.getValue() && !benchmark;
    options = new OptionsDialog(this);
    options.initDialog();
    clipboardDialog = new ClipboardDialog(this);
    profileDialog = new ProfileDialog(this);
    String env = System.getenv("TVNC_PROFILE");
    if (env != null && env.equals("1"))
      alwaysProfile = true;
    firstUpdate = true; pendingUpdate = false; continuousUpdates = false;
    forceNonincremental = true; supportsSyncFence = false;
    pressedKeys = new HashMap<Integer, Integer>();

    setShared(opts.shared);
    upg = this;

    cp.supportsDesktopResize = true;
    cp.supportsExtendedDesktopSize = true;
    cp.supportsClientRedirect = VncViewer.clientRedirect.getValue();
    cp.supportsDesktopRename = true;
    menu = new F8Menu(this);

    if (VncViewer.noUnixLogin.getValue()) {
      Security.disableSecType(Security.secTypePlain);
      Security.disableSecType(Security.secTypeTLSPlain);
      Security.disableSecType(Security.secTypeX509Plain);
      Security.disableSecType(Security.secTypeUnixLogin);
    } else if (isUnixLoginForced()) {
      Security.disableSecType(Security.secTypeVncAuth);
      Security.disableSecType(Security.secTypeTLSVnc);
      Security.disableSecType(Security.secTypeX509Vnc);
      Security.disableSecType(Security.secTypeNone);
      Security.disableSecType(Security.secTypeTLSNone);
      Security.disableSecType(Security.secTypeX509None);
    }

    if (sock != null) {
      String name = sock.getPeerEndpoint();
      vlog.info("Accepted connection from " + name);
    } else if (!benchmark) {
      String serverName = null;
      int port = -1;

      if (opts.serverName != null &&
          !VncViewer.alwaysShowConnectionDialog.getValue()) {
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
      } else if (opts.via != null || opts.tunnel) {
        try {
          Tunnel.createTunnel(opts);
          port = Hostname.getPort(opts.serverName);
          serverName = Hostname.getHost(opts.serverName);
        } catch (Exception e) {
          throw new ErrorException("Could not create SSH tunnel:\n" +
                                   e.getMessage());
        }
      }

      sock = new TcpSocket(serverName, port);
      vlog.info("connected to host " + serverName + " port " + port);
    }

    if (benchmark) {
      state_ = RFBSTATE_INITIALISATION;
      reader_ = new CMsgReaderV3(this, viewer.benchFile);
    } else {
      sock.inStream().setBlockCallback(this);
      setServerName(opts.serverName);
      setStreams(sock.inStream(), sock.outStream());
      initialiseProtocol();
    }
  }

  // RFB thread
  public void reset() {
    if (reader_ != null)
      reader_.reset();
    state_ = RFBSTATE_INITIALISATION;
  }

  // EDT: deleteWindow() is called when the user closes the window or selects
  // "Close Connection" from the F8 menu.
  void deleteWindow() {
    if (viewport != null) {
      if (viewport.timer != null)
        viewport.timer.stop();
      viewport.dispose();
    }
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
      throw new SystemException(e.toString());
    }
  }

  // RFB thread: getUserPasswd() is called by the CSecurity object when it
  // needs us to read a password from the user.
  public final boolean getUserPasswd(StringBuffer user, StringBuffer passwd) {
    String title = ((user == null ? "Standard VNC Authentication" :
                                    "Unix Login Authentication") +
                    " [" + csecurity.description() + "]");
    String passwordFileStr = VncViewer.passwordFile.getValue();
    PasswdDialog dlg = null;
    String autoPass;

    if (VncViewer.encPassword.getValue() != null) {
      byte[] encryptedPassword = new byte[8];
      String passwordString = VncViewer.encPassword.getValue();
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
    } else if (VncViewer.autoPass.getValue()) {
      BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
      try {
        autoPass = in.readLine();
      } catch (IOException e) {
        throw new SystemException(e.toString());
      }
      VncViewer.autoPass.setParam("0");
    } else
      autoPass = VncViewer.password.getValue();

    if (autoPass != null && passwd != null) {
      passwd.append(autoPass);
      passwd.setLength(autoPass.length());
      VncViewer.password.setParam(null);
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
        dlg = new PasswdDialog(title, (user == null), null, (passwd == null));
    } else {
      String userName = opts.user;
      if (opts.sendLocalUsername) {
        userName = (String)System.getProperties().get("user.name");
        if (VncViewer.localUsernameLC.getValue())
          userName = userName.toLowerCase();
        if (passwd == null)
          return true;
      }
      if (autoPass == null)
        dlg = new PasswdDialog(title, (userName != null), userName,
                               (passwd == null));
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
      if (opts.colors < 0) {
        pendingPF = fullColourPF;
      } else {
        if (opts.colors == 8) {
          pendingPF = VERY_LOW_COLOR_PF;
        } else if (opts.colors == 64) {
          pendingPF = LOW_COLOR_PF;
        } else if (opts.colors == 256) {
          pendingPF = MEDIUM_COLOR_PF;
        } else if (opts.colors == 32768) {
          pendingPF = MEDIUMHIGH_COLOR_PF;
        } else {
          pendingPF = HIGH_COLOR_PF;
        }
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
          if (VncViewer.embed.getValue()) {
            desktop.setScaledSize();
            setupEmbeddedFrame();
          } else {
            recreateViewport();
          }
        }
      });
    } catch (InterruptedException e) {
    } catch (java.lang.reflect.InvocationTargetException e) {
      Throwable cause = e.getCause();
      if (cause instanceof ErrorException)
        throw (ErrorException)cause;
      if (cause instanceof WarningException)
        throw (WarningException)cause;
      else if (cause != null)
        throw new SystemException(cause.toString());
    }
  }

  // EDT
  void setupEmbeddedFrame() {
    UIManager.getDefaults().put("ScrollPane.ancestorInputMap",
      new UIDefaults.LazyInputMap(new Object[]{}));
    JScrollPane sp = new JScrollPane();
    sp.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 0));
    sp.getViewport().setBackground(Color.BLACK);
    InputMap im = sp.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW);
    int ctrlAltShiftMask = Event.SHIFT_MASK | Event.CTRL_MASK | Event.ALT_MASK;
    if (im != null) {
      im.put(KeyStroke.getKeyStroke(KeyEvent.VK_UP, ctrlAltShiftMask),
             "unitScrollUp");
      im.put(KeyStroke.getKeyStroke(KeyEvent.VK_DOWN, ctrlAltShiftMask),
             "unitScrollDown");
      im.put(KeyStroke.getKeyStroke(KeyEvent.VK_LEFT, ctrlAltShiftMask),
             "unitScrollLeft");
      im.put(KeyStroke.getKeyStroke(KeyEvent.VK_RIGHT, ctrlAltShiftMask),
             "unitScrollRight");
      im.put(KeyStroke.getKeyStroke(KeyEvent.VK_PAGE_UP, ctrlAltShiftMask),
             "scrollUp");
      im.put(KeyStroke.getKeyStroke(KeyEvent.VK_PAGE_DOWN, ctrlAltShiftMask),
             "scrollDown");
      im.put(KeyStroke.getKeyStroke(KeyEvent.VK_HOME, ctrlAltShiftMask),
             "scrollLeft");
      im.put(KeyStroke.getKeyStroke(KeyEvent.VK_END, ctrlAltShiftMask),
             "scrollRight");
    }
    sp.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);
    sp.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED);
    sp.getViewport().setView(desktop);
    viewer.getContentPane().removeAll();
    if (showToolbar) {
      Toolbar tb = new Toolbar(this);
      viewer.add(tb, BorderLayout.PAGE_START);
    }
    viewer.add(sp);
    viewer.addFocusListener(new FocusAdapter() {
      public void focusGained(FocusEvent e) {
        if (desktop.isAncestorOf(viewer))
          desktop.requestFocus();
      }
      public void focusLost(FocusEvent e) {
        releasePressedKeys();
      }
    });
    viewer.validate();
    viewer.repaint();
    desktop.requestFocus();
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

    if ((reason == screenTypes.reasonClient) &&
        (result != screenTypes.resultSuccess)) {
      vlog.error("SetDesktopSize failed: " + result);
      return;
    }

    resizeFramebuffer();
    if (opts.desktopSize.mode == Options.SIZE_MANUAL && !firstUpdate) {
      opts.desktopSize.width = w;
      opts.desktopSize.height = h;
    }
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
    tUpdateStart = getTime();
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
        writer().writeFence(
          fenceTypes.fenceFlagRequest | fenceTypes.fenceFlagSyncNext, 0, null);

      if (!cp.supportsSetDesktopSize) {
        if (opts.desktopSize.mode == Options.SIZE_AUTO)
          vlog.info("Disabling automatic desktop resizing because the server doesn't support it.");
        if (opts.desktopSize.mode == Options.SIZE_MANUAL)
          vlog.info("Ignoring desktop resize request because the server doesn't support it.");
        opts.desktopSize.mode = Options.SIZE_SERVER;
      }

      if (opts.desktopSize.mode == Options.SIZE_MANUAL)
        sendDesktopSize(opts.desktopSize.width, opts.desktopSize.height);
      else if (opts.desktopSize.mode == Options.SIZE_AUTO) {
        if (VncViewer.embed.getValue()) {
          Dimension size = viewer.getSize();
          if (showToolbar)
            size.height -= 22;
          sendDesktopSize(size.width, size.height);
        } else {
          Dimension availableSize = viewport.getAvailableSize();
          sendDesktopSize(availableSize.width, availableSize.height);
        }
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

    tUpdate += getTime() - tUpdateStart;
    updates++;
    tElapsed = getTime() - tStart;

    if (tElapsed > (double)VncViewer.profileInt.getValue() && !benchmark) {
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
        str = String.format("%.3f", (double)decodePixels / 1000000. / tElapsed);
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
                          (double)sock.inStream().getBytesRead() / 125000. / tElapsed);
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
                          sock.inStream().getReadTime() / (double)updates * 1000.,
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
      tStart = getTime();
    }
  }

  public void sendDesktopSize(int width, int height) {
    sendDesktopSize(width, height, false);
  }

  // RFB thread when sending initial desktop size, EDT when sending automatic
  // desktop size (writer() is synchronized.)
  public void sendDesktopSize(int width, int height, boolean automatic) {
    ScreenSet layout;

    layout = cp.screenLayout;

    if (!cp.supportsSetDesktopSize)
      return;

    if (layout.numScreens() == 0)
      layout.addScreen(new Screen());
    else if (layout.numScreens() != 1) {

      while (true) {
        Iterator<Screen> iter = layout.screens.iterator();
        Screen screen = (Screen)iter.next();

        if (!iter.hasNext())
          break;

        layout.removeScreen(screen.id);
      }
    }

    Screen screen0 = (Screen)layout.screens.iterator().next();
    screen0.dimensions.tl.x = 0;
    screen0.dimensions.tl.y = 0;
    screen0.dimensions.br.x = width;
    screen0.dimensions.br.y = height;

    writer().writeSetDesktopSize(width, height, layout);
    if (automatic)
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
    if (opts.acceptClipboard)
      clipboardDialog.serverCutText(str, len);
  }

  public void startDecodeTimer() {
    tDecodeStart = getTime();
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
    tDecode += getTime() - tDecodeStart - (tRead - tReadOld);
  }

  public void beginRect(Rect r, int encoding) {
    if (!benchmark)
      sock.inStream().startTiming();
    if (encoding != Encodings.encodingCopyRect) {
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

  // EDT
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

    if ((flags & fenceTypes.fenceFlagRequest) != 0) {
      // We handle everything synchronously, so we trivially honor these modes.
      flags = flags & (fenceTypes.fenceFlagBlockBefore |
                       fenceTypes.fenceFlagBlockAfter);

      writer().writeFence(flags, len, data);
      return;
    }

    if (len == 0) {
      // Initial probe
      if ((flags & fenceTypes.fenceFlagSyncNext) != 0) {
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
      if (VncViewer.osEID())
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

    Rectangle screenArea = getMaxSpannedSize();
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
    if (VncViewer.embed.getValue()) {
      w = desktop.scaledWidth;
      h = desktop.scaledHeight;
    } else {
      w = desktop.width();
      h = desktop.height();
    }

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
          if (VncViewer.embed.getValue()) {
            desktop.setScaledSize();
            setupEmbeddedFrame();
          } else if (opts.desktopSize.mode == Options.SIZE_AUTO &&
                     !opts.fullScreen)
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
    } catch (java.lang.reflect.InvocationTargetException e) {
      Throwable cause = e.getCause();
      if (cause instanceof ErrorException)
        throw (ErrorException)cause;
      if (cause instanceof WarningException)
        throw (WarningException)cause;
      else if (cause != null)
        throw new SystemException(cause.toString());
    }
  }

  // EDT: recreateViewport() recreates our top-level window.

  static Rectangle savedRect = new Rectangle(-1, -1, 0, 0);
  static int savedState = -1;

  private void recreateViewport() { recreateViewport(false); }

  private void recreateViewport(boolean restore) {
    boolean keyboardTempUngrabbed = false;
    if (VncViewer.embed.getValue())
      return;
    if (viewport != null) {
      if (opts.fullScreen) {
        savedState = viewport.getExtendedState();
        viewport.setExtendedState(JFrame.NORMAL);
        savedRect = viewport.getBounds();
      }
      if (viewport.timer != null)
        viewport.timer.stop();
      if (VncViewer.osGrab() && keyboardGrabbed) {
        viewport.grabKeyboardHelper(false);
        if (opts.grabKeyboard == Options.GRAB_MANUAL)
          keyboardTempUngrabbed = true;
      }
      viewport.dispose();
    }
    viewport = new Viewport(this);
    // When in Lion full-screen mode, we need to create the viewport as if
    // full-screen mode was disabled.
    boolean fullScreen = opts.fullScreen && !viewport.lionFSSupported();
    viewport.setUndecorated(fullScreen);
    desktop.setViewport(viewport);
    reconfigureViewport(restore);
    if ((cp.width > 0) && (cp.height > 0))
      viewport.setVisible(true);
    if (VncViewer.isX11())
      viewport.x11FullScreenHelper(fullScreen);
    if (opts.fullScreen && viewport.lionFSSupported())
      viewport.toggleLionFS();
    desktop.requestFocusInWindow();
    if (VncViewer.osGrab()) {
      if (opts.grabKeyboard == Options.GRAB_ALWAYS ||
          (opts.grabKeyboard == Options.GRAB_MANUAL && keyboardTempUngrabbed) ||
          (opts.grabKeyboard == Options.GRAB_FS && fullScreen))
        viewport.grabKeyboardHelper(true);
    }
    if (VncViewer.osEID())
      viewport.setupExtInputHelper();
  }

  public Rectangle getMaxSpannedSize() {
    GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
    GraphicsDevice[] gsList = ge.getScreenDevices();
    Rectangle s0 = null;
    Rectangle span = new Rectangle(-1, -1, 0, 0);
    int tLeft = 0, tTop = 0, tRight = 0, tBottom = 0;
    boolean equal = true;

    for (GraphicsDevice gs : gsList) {
      GraphicsConfiguration[] gcList = gs.getConfigurations();
      for (GraphicsConfiguration gc : gcList) {
        Rectangle s = gc.getBounds();
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

        // If the screen areas of the primary monitor and this monitor overlap
        // vertically, then allow the full-screen window to extend horizontally
        // to this monitor, and constrain it vertically, if necessary, to fit
        // within this monitor's dimensions.
        if (Math.min(s.y + s.height, s0.y + s0.height) -
            Math.max(s.y, s0.y) > 0) {
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
    boolean fullScreen = opts.fullScreen &&
                         (!VncViewer.os.startsWith("mac os x") ||
                          viewport.lionFSSupported());
    GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
    GraphicsDevice[] gsList = ge.getScreenDevices();
    Rectangle primary = null, s0 = null;
    Rectangle span = new Rectangle(-1, -1, 0, 0);
    Insets in = new Insets(0, 0, 0, 0);
    int tLeft = 0, tTop = 0, tRight = 0, tBottom = 0;
    boolean equal = true;
    int sw = desktop.scaledWidth;
    int sh = desktop.scaledHeight;

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
        if (!fullScreen) {
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
                                   s.y < primary.y + primary.height)))))
          primary = s;
        if (VncViewer.currentMonitorIsPrimary.getValue() && viewport != null) {
          Rectangle vpRect = viewport.getBounds();
          if (opts.fullScreen && savedRect.width > 0 && savedRect.height > 0)
            vpRect = savedRect;
          vpRect = s.intersection(vpRect);
          int area = vpRect.isEmpty() ? 0 : vpRect.width * vpRect.height;
          if (area > maxArea) {
            maxArea = area;
            primary = s;
            primaryIsCurrent = true;
          }
        }
        if (gc == gcList[0])
          vlog.debug("Screen " + i++ + (fullScreen ? " FS " : " work ") +
                     "area: " + s.x + ", " + s.y + " " + s.width + " x " +
                     s.height);

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

        // If the screen areas of the primary monitor and this monitor overlap
        // vertically, then allow the full-screen window to extend horizontally
        // to this monitor, and constrain it vertically, if necessary, to fit
        // within this monitor's dimensions.
        if (Math.min(s.y + s.height, s0.y + s0.height) -
            Math.max(s.y, s0.y) > 0) {
          int right = Math.max(s.x + s.width, span.x + span.width);
          int bottom = Math.min(s.y + s.height, span.y + span.height);
          span.x = Math.min(s.x, span.x);
          span.y = Math.max(s.y, span.y);
          span.width = right - span.x;
          span.height = bottom - span.y;
        }
      }
    }

    if (opts.span == Options.SPAN_PRIMARY ||
        (opts.span == Options.SPAN_AUTO &&
         (sw <= primary.width || span.width <= primary.width) &&
         (sh <= primary.height || span.height <= primary.height)) ||
        (opts.span == Options.SPAN_AUTO &&
         opts.desktopSize.mode == Options.SIZE_AUTO) ||
        VncViewer.isX11())
        // ^^ Multi-screen spanning doesn't even pretend to work under X11.
      span = primary;
    else if (equal && fullScreen)
      span = new Rectangle(tLeft, tTop, tRight - tLeft, tBottom - tTop);

    vlog.debug("Spanned " + (fullScreen ? "FS " : "work ") + "area: " +
               span.x + ", " + span.y + " " + span.width + " x " +
               span.height);
    return span;
  }

  // EDT: Resize window based on the spanning option
  public void sizeWindow() { sizeWindow(true); }

  public void sizeWindow(boolean manual) {
    if (VncViewer.embed.getValue())
      return;
    boolean fullScreen = opts.fullScreen && !viewport.lionFSSupported();
    int w = desktop.scaledWidth;
    int h = desktop.scaledHeight;
    Rectangle span = getSpannedSize();

    if ((opts.scalingFactor == Options.SCALE_AUTO ||
         opts.scalingFactor == Options.SCALE_FIXEDRATIO) && !fullScreen) {
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

    if (viewport.getExtendedState() != JFrame.ICONIFIED &&
        !VncViewer.os.startsWith("mac os x"))
      viewport.setExtendedState(JFrame.NORMAL);
    int x = (span.width - w) / 2 + span.x;
    int y = (span.height - h) / 2 + span.y;
    if (fullScreen) {
      viewport.setGeometry(span.x, span.y, span.width, span.height);
      viewport.dx = x - span.x;
      viewport.dy = y - span.y;
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

    viewport.setGeometry(x, y, w, h);
  }

  // EDT
  private void reconfigureViewport(boolean restore) {
    boolean fullScreen = opts.fullScreen && !viewport.lionFSSupported();
    desktop.setScaledSize();
    if (!fullScreen && savedRect.width > 0 && savedRect.height > 0 &&
        restore) {
      if (savedState >= 0)
        viewport.setExtendedState(savedState);
      viewport.setGeometry(savedRect.x, savedRect.y, savedRect.width,
                           savedRect.height);
    } else {
      sizeWindow(false);
    }
  }

  // RFB thread: requestNewUpdate() requests an update from the server, having
  // set the format and encoding appropriately.
  private void requestNewUpdate() {
    if (formatChange) {
      PixelFormat pf;

      // Catch incorrect requestNewUpdate calls
      assert(!pendingUpdate || supportsSyncFence);

      if (opts.colors < 0) {
        pf = fullColourPF;
      } else {
        if (opts.colors == 8) {
          pf = VERY_LOW_COLOR_PF;
        } else if (opts.colors == 64) {
          pf = LOW_COLOR_PF;
        } else if (opts.colors == 256) {
          pf = MEDIUM_COLOR_PF;
        } else if (opts.colors == 32768) {
          pf = MEDIUMHIGH_COLOR_PF;
        } else {
          pf = HIGH_COLOR_PF;
        }
      }

      if (supportsSyncFence) {
        // We let the fence carry the pixel format change and make the switch
        // once we get the response back.  That way, we will be synchronised
        // with the format change on the server end.
        MemOutStream memStream = new MemOutStream();

        pf.write(memStream);

        writer().writeFence(fenceTypes.fenceFlagRequest |
                            fenceTypes.fenceFlagSyncNext, memStream.length(),
                            (byte[])memStream.data());
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
      writer().writeFramebufferUpdateRequest(new Rect(0, 0, cp.width, cp.height),
                                             !formatChange && !forceNonincremental);
    }

    forceNonincremental = false;
  }


  ////////////////////////////////////////////////////////////////////
  // The following methods are all called from the EDT.

  // close() shuts down the socket, thus waking up the RFB thread.
  public void close() {
    deleteWindow();
    shuttingDown = true;
    if (sock != null)
      sock.shutdown();
  }

  // Menu callbacks.  These are guaranteed only to be called after serverInit()
  // has been called, since the menu is only accessible from the DesktopWindow.

  void showMenu(int x, int y) {
    String os = System.getProperty("os.name");
    if (os.startsWith("Windows"))
      com.sun.java.swing.plaf.windows.WindowsLookAndFeel.setMnemonicHidden(false);
    if (viewport != null && (viewport.dx > 0 || viewport.dy > 0)) {
      x += viewport.dx;
      y += viewport.dy;
    }
    menu.show(desktop, x, y);
  }

  void showAbout() {
    VncViewer.showAbout(viewport);
  }

  void showInfo() {
    JOptionPane pane = new JOptionPane(
      "Desktop name:  " + cp.name() + "\n" +
      "Host:  " + sock.getPeerName() + ":" + sock.getPeerPort() + "\n" +
      "Size:  " + cp.width + "x" + cp.height + "\n" +
      "Pixel format:  " + desktop.getPF().print() + "\n" +
      "(server default " + serverPF.print() + ")\n" +
      "Requested encoding:  " + Encodings.encodingName(currentEncoding) +
        "\n" +
      "Last used encoding:  " + Encodings.encodingName(lastServerEncoding) +
        "\n" +
      "Protocol version:  " + cp.majorVersion + "." + cp.minorVersion + "\n" +
      "Security type:  " + Security.secTypeName(csecurity.getType()) +
        " [" + csecurity.description() + "]\n" +
      "JPEG decompression:  " +
        (reader_.isTurboJPEG() ? "Turbo" : "Unaccelerated") +
      (VncViewer.osGrab() ? "\nTurboVNC Helper:  " +
        (Viewport.isHelperAvailable() ? "Loaded" : "Not found") : ""),
      JOptionPane.PLAIN_MESSAGE);
    JDialog dlg = pane.createDialog(viewport, "VNC connection info");
    if (VncViewer.embed.getValue())
      dlg.setAlwaysOnTop(true);
    dlg.setVisible(true);
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

    if (currentEncoding == Encodings.encodingTight &&
        opts.compressLevel == 1 && opts.quality == -1 && !opts.allowJpeg)
      alreadyLossless = true;

    if (!alreadyLossless) {
      currentEncoding = Encodings.encodingTight;
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
    if (lastServerEncoding != Encodings.encodingTight &&
        lastServerEncoding >= 0)
      encoding = lastServerEncoding;
    options.setTightOptions(encoding);
  }

  public void setOptions() {
    options.allowJpeg.setSelected(opts.allowJpeg);
    options.subsamplingLevel.setValue(opts.getSubsamplingOrdinal());
    options.jpegQualityLevel.setValue(opts.quality);
    options.setCompressionLevel(opts.compressLevel);

    setTightOptions();

    options.viewOnly.setSelected(opts.viewOnly);
    options.acceptClipboard.setSelected(opts.acceptClipboard);
    options.sendClipboard.setSelected(opts.sendClipboard);
    options.menuKey.setSelectedItem(KeyEvent.getKeyText(MenuKey.getMenuKeyCode()));
    if (VncViewer.osGrab() && Viewport.isHelperAvailable())
      options.grabKeyboard.setSelectedIndex(opts.grabKeyboard);

    if (state() == RFBSTATE_NORMAL) {
      options.shared.setEnabled(false);
      options.secVeNCrypt.setEnabled(false);
      options.encNone.setEnabled(false);
      options.encTLS.setEnabled(false);
      options.encX509.setEnabled(false);
      options.x509ca.setEnabled(false);
      options.x509caButton.setEnabled(false);
      options.x509crl.setEnabled(false);
      options.x509crlButton.setEnabled(false);
      options.secIdent.setEnabled(false);
      options.secNone.setEnabled(false);
      options.secVnc.setEnabled(false);
      options.secUnixLogin.setEnabled(false);
      options.secPlain.setEnabled(false);
      options.sendLocalUsername.setEnabled(false);
      options.gateway.setEnabled(false);
      options.sshUser.setEnabled(false);
      options.tunnel.setEnabled(false);
    } else {
      options.shared.setSelected(opts.shared);
      options.sendLocalUsername.setSelected(opts.sendLocalUsername);
      options.setSecurityOptions();
      if (opts.via != null)
        options.gateway.setText(opts.via);
      if (opts.sshUser != null)
        options.sshUser.setText(opts.sshUser);
      options.tunnel.setSelected(opts.tunnel);
      if (SecurityClient.x509ca.getValue() != null)
        options.x509ca.setText(SecurityClient.x509ca.getValue());
      if (SecurityClient.x509crl.getValue() != null)
        options.x509crl.setText(SecurityClient.x509crl.getValue());
    }

    options.fullScreen.setSelected(opts.fullScreen);
    options.span.setSelectedIndex(opts.span);
    options.cursorShape.setSelected(opts.cursorShape);
    options.acceptBell.setSelected(opts.acceptBell);
    options.showToolbar.setSelected(VncViewer.showToolbar.getValue());
    options.desktopSize.setEnabled(cp.supportsSetDesktopSize || firstUpdate);
    if (opts.scalingFactor == Options.SCALE_AUTO) {
      options.scalingFactor.setSelectedItem("Auto");
    } else if (opts.scalingFactor == Options.SCALE_FIXEDRATIO) {
      options.scalingFactor.setSelectedItem("Fixed Aspect Ratio");
    } else {
      options.scalingFactor.setSelectedItem(opts.scalingFactor + "%");
      if (desktop != null)
        desktop.setScaledSize();
    }
    if (opts.desktopSize.mode == Options.SIZE_AUTO) {
      options.desktopSize.setSelectedItem("Auto");
      options.scalingFactor.setEnabled(false);
    } else if (opts.desktopSize.mode == Options.SIZE_SERVER) {
      options.desktopSize.setSelectedItem("Server");
      options.scalingFactor.setEnabled(!VncViewer.embed.getValue());
    } else {
      options.desktopSize.setSelectedItem(opts.desktopSize.width + "x" +
                                          opts.desktopSize.height);
      options.scalingFactor.setEnabled(!VncViewer.embed.getValue());
    }
  }

  public void getOptions() {
    boolean recreate = false, reconfigure = false, defaultSize = false;

    if (opts.allowJpeg != options.allowJpeg.isSelected())
      encodingChange = true;
    opts.allowJpeg = options.allowJpeg.isSelected();

    if (opts.quality != options.jpegQualityLevel.getValue())
      encodingChange = true;
    opts.quality = options.jpegQualityLevel.getValue();

    if (opts.compressLevel != options.getCompressionLevel())
      encodingChange = true;
    opts.compressLevel = options.getCompressionLevel();

    if (opts.subsampling != options.getSubsamplingLevel())
      encodingChange = true;
    opts.subsampling = options.getSubsamplingLevel();

    opts.sendLocalUsername = options.sendLocalUsername.isSelected();
    if (opts.viewOnly != options.viewOnly.isSelected() && showToolbar)
      recreate = true;
    opts.viewOnly = options.viewOnly.isSelected();
    opts.acceptClipboard = options.acceptClipboard.isSelected();
    opts.sendClipboard = options.sendClipboard.isSelected();
    opts.acceptBell = options.acceptBell.isSelected();
    VncViewer.showToolbar.setParam(options.showToolbar.isSelected());

    int oldScalingFactor = opts.scalingFactor;
    opts.setScalingFactor(options.scalingFactor.getSelectedItem().toString());
    if (desktop != null && opts.scalingFactor != oldScalingFactor)
      recreate = true;

    Options.DesktopSize oldDesktopSize = opts.desktopSize;
    opts.setDesktopSize(options.desktopSize.getSelectedItem().toString());
    if (desktop != null && !opts.desktopSize.isEqual(oldDesktopSize)) {
      if (oldDesktopSize.mode != Options.SIZE_AUTO &&
          opts.desktopSize.mode == Options.SIZE_AUTO &&
          viewport.lionFSSupported() && opts.fullScreen &&
          options.fullScreen.isSelected())
        defaultSize = true;
      if (opts.desktopSize.mode != Options.SIZE_SERVER) {
        reconfigure = true;
        firstUpdate = true;
      }
    }

    int oldSpan = opts.span;
    int index = options.span.getSelectedIndex();
    if (index >= 0 && index < Options.NUMSPANOPT)
      opts.span = index;
    if (desktop != null && opts.span != oldSpan) {
      if (opts.fullScreen && !viewport.lionFSSupported())
        recreate = true;
      else
        reconfigure = true;
    }

    clipboardDialog.setSendingEnabled(opts.sendClipboard);
    VncViewer.menuKey.setParam(
      MenuKey.getMenuKeySymbols()[options.menuKey.getSelectedIndex()].name);
    menu.updateMenuKey(MenuKey.getMenuKeyCode());

    if (VncViewer.osGrab() && Viewport.isHelperAvailable()) {
      opts.grabKeyboard = options.grabKeyboard.getSelectedIndex();
      if (viewport != null &&
          ((opts.grabKeyboard == Options.GRAB_ALWAYS &&
           !viewport.keyboardTempUngrabbed) ||
          (opts.grabKeyboard == Options.GRAB_FS &&
           opts.fullScreen != viewport.keyboardTempUngrabbed))) {
        viewport.keyboardTempUngrabbed = !viewport.keyboardTempUngrabbed;
      }
    }

    opts.shared = options.shared.isSelected();
    setShared(opts.shared);
    if (opts.cursorShape != options.cursorShape.isSelected()) {
      opts.cursorShape = options.cursorShape.isSelected();
      encodingChange = true;
      if (desktop != null)
        desktop.resetLocalCursor();
    }

    checkEncodings();

    if (state() != RFBSTATE_NORMAL) {
      options.getSecurityOptions();
      String gateway = options.gateway.getText();
      opts.via = (gateway.isEmpty() ? null : gateway);
      String sshUser = options.sshUser.getText();
      opts.sshUser = (sshUser.isEmpty() ? null : sshUser);
      opts.tunnel = options.tunnel.isSelected();
      SecurityClient.x509ca.setParam(options.x509ca.getText());
      SecurityClient.x509crl.setParam(options.x509crl.getText());
    }

    if (options.fullScreen.isSelected() != opts.fullScreen)
      toggleFullScreen();
    else if ((recreate || reconfigure) && VncViewer.embed.getValue())
      setupEmbeddedFrame();
    else if (recreate)
      recreateViewport();
    else if (reconfigure) {
      if (defaultSize)
        sizeWindow(true);
      else
        reconfigureViewport(false);
    }
    // Force a framebuffer update if we're initiating a manual or auto remote
    // desktop resize.  Otherwise, it won't occur until the mouse is moved or
    // something changes on the server (manual) or until the window is resized
    // (auto.)
    if (firstUpdate && state() == RFBSTATE_NORMAL) {
      forceNonincremental = true;
      requestNewUpdate();
    }
  }

  public boolean supportsSetDesktopSize() {
    return cp.supportsSetDesktopSize || firstUpdate;
  }

  // EDT
  public void toggleToolbar() {
    if (opts.fullScreen)
      return;
    showToolbar = !showToolbar;
    if (viewport != null && !VncViewer.embed.getValue()) {
      recreateViewport();
      viewport.showToolbar(showToolbar);
    } else {
      setupEmbeddedFrame();
      if (opts.desktopSize.mode == Options.SIZE_AUTO) {
        Dimension size = viewer.getSize();
        if (showToolbar)
          size.height -= 22;
        sendDesktopSize(size.width, size.height);
      }
    }
    menu.showToolbar.setSelected(showToolbar);
  }

  // EDT
  public void toggleFullScreen() {
    if (VncViewer.embed.getValue())
      return;
    opts.fullScreen = !opts.fullScreen;
    menu.fullScreen.setSelected(opts.fullScreen);
    if (viewport != null) {
      if (!viewport.lionFSSupported()) {
        recreateViewport(true);
      } else {
        viewport.toggleLionFS();
      }
    }
  }

  // EDT
  public void toggleKeyboardGrab() {
    if (VncViewer.embed.getValue() || !VncViewer.osGrab())
      return;
    if (viewport != null)
      viewport.grabKeyboardHelper(!keyboardGrabbed);
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
    if (pressedKeys.containsValue(Keysyms.Shift_L))
      str += " LShift";
    if (pressedKeys.containsValue(Keysyms.Shift_R))
      str += " RShift";
    if (pressedKeys.containsValue(Keysyms.Control_L))
      str += " LCtrl";
    if (pressedKeys.containsValue(Keysyms.Control_R))
      str += " RCtrl";
    if (pressedKeys.containsValue(Keysyms.Alt_L))
      str += " LAlt";
    if (pressedKeys.containsValue(Keysyms.Alt_R))
      str += " RAlt";
    if (pressedKeys.containsValue(Keysyms.Meta_L))
      str += " LMeta";
    if (pressedKeys.containsValue(Keysyms.Meta_R))
      str += " RMeta";
    if (pressedKeys.containsValue(Keysyms.Super_L))
      str += " LSuper";
    if (pressedKeys.containsValue(Keysyms.Super_R))
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
                ", code " + KeyEvent.getKeyText(keycode) + " (" + keycode + ")" +
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
      Integer sym = pressedKeys.get(keycode);

      if (sym == null) {
        // Note that dead keys will raise this sort of error falsely
        // See https://bugs.openjdk.java.net/browse/JDK-6534883
        debugStr += " UNEXPECTED/IGNORED";
        vlog.debug(debugStr);
        return;
      }

      writeKeyEvent(sym, false);
      pressedKeys.remove(keycode);
      debugStr += String.format(" => 0x%04x", sym);
      vlog.debug(debugStr);
      return;
    }

    if (!ev.isActionKey()) {
      if (keycode >= KeyEvent.VK_0 && keycode <= KeyEvent.VK_9 &&
        location == KeyEvent.KEY_LOCATION_NUMPAD)
        keysym = Keysyms.KP_0 + keycode - KeyEvent.VK_0;

      switch (keycode) {
      case KeyEvent.VK_BACK_SPACE: keysym = Keysyms.BackSpace;  break;
      case KeyEvent.VK_TAB:        keysym = Keysyms.Tab;  break;
      case KeyEvent.VK_ENTER:
        if (location == KeyEvent.KEY_LOCATION_NUMPAD)
          keysym = Keysyms.KP_Enter;
        else
          keysym = Keysyms.Return;  break;
      case KeyEvent.VK_ESCAPE:     keysym = Keysyms.Escape;  break;
      case KeyEvent.VK_NUMPAD0:    keysym = Keysyms.KP_0;  break;
      case KeyEvent.VK_NUMPAD1:    keysym = Keysyms.KP_1;  break;
      case KeyEvent.VK_NUMPAD2:    keysym = Keysyms.KP_2;  break;
      case KeyEvent.VK_NUMPAD3:    keysym = Keysyms.KP_3;  break;
      case KeyEvent.VK_NUMPAD4:    keysym = Keysyms.KP_4;  break;
      case KeyEvent.VK_NUMPAD5:    keysym = Keysyms.KP_5;  break;
      case KeyEvent.VK_NUMPAD6:    keysym = Keysyms.KP_6;  break;
      case KeyEvent.VK_NUMPAD7:    keysym = Keysyms.KP_7;  break;
      case KeyEvent.VK_NUMPAD8:    keysym = Keysyms.KP_8;  break;
      case KeyEvent.VK_NUMPAD9:    keysym = Keysyms.KP_9;  break;
      case KeyEvent.VK_DECIMAL:    keysym = Keysyms.KP_Decimal;  break;
      case KeyEvent.VK_ADD:        keysym = Keysyms.KP_Add;  break;
      case KeyEvent.VK_SUBTRACT:   keysym = Keysyms.KP_Subtract;  break;
      case KeyEvent.VK_MULTIPLY:   keysym = Keysyms.KP_Multiply;  break;
      case KeyEvent.VK_DIVIDE:     keysym = Keysyms.KP_Divide;  break;
      case KeyEvent.VK_DELETE:
        if (location == KeyEvent.KEY_LOCATION_NUMPAD)
          keysym = Keysyms.KP_Delete;
        else
          keysym = Keysyms.Delete;  break;
      case KeyEvent.VK_CLEAR:
        if (location == KeyEvent.KEY_LOCATION_NUMPAD)
          keysym = Keysyms.KP_Begin;
        else
          keysym = Keysyms.Clear;  break;
      case KeyEvent.VK_CONTROL:
        if (location == KeyEvent.KEY_LOCATION_RIGHT)
          keysym = Keysyms.Control_R;
        else
          keysym = Keysyms.Control_L;  break;
      case KeyEvent.VK_ALT:
        if (location == KeyEvent.KEY_LOCATION_RIGHT) {
          // Mac has no AltGr key, but the Option/Alt keys serve the same
          // purpose.  Thus, we allow RAlt to be used as AltGr and LAlt to be
          // used as a regular Alt key.
          if (VncViewer.os.startsWith("mac os x"))
            keysym = Keysyms.ISO_Level3_Shift;
          else
            keysym = Keysyms.Alt_R;
        } else {
          keysym = Keysyms.Alt_L;
        }
        break;
      case KeyEvent.VK_SHIFT:
        if (location == KeyEvent.KEY_LOCATION_RIGHT)
          keysym = Keysyms.Shift_R;
        else
          keysym = Keysyms.Shift_L;  break;
      case KeyEvent.VK_META:
        if (location == KeyEvent.KEY_LOCATION_RIGHT)
          keysym = Keysyms.Super_R;
        else
          keysym = Keysyms.Super_L;  break;
      case KeyEvent.VK_ALT_GRAPH:
        keysym = Keysyms.ISO_Level3_Shift;
        break;
      default:
        // On Windows, pressing AltGr has the same effect as pressing LCtrl +
        // RAlt, so we have to send fake key release events for those modifiers
        // (and any other Ctrl and Alt modifiers that are pressed), then send
        // the key event for the modified key, then send fake key press events
        // for the same modifiers.
        if (pressedKeys.containsValue(Keysyms.Alt_R) &&
            pressedKeys.containsValue(Keysyms.Control_L) &&
            VncViewer.os.startsWith("windows")) {
          winAltGr = true;
        } else if (ev.isControlDown()) {
          // For CTRL-<letter>, CTRL is sent separately, so just send <letter>.
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
          // combinations, so the best we can do is send the key code if it is
          // a valid ASCII symbol.
          else if (key == KeyEvent.CHAR_UNDEFINED && keycode >= 0 &&
                   keycode <= 127)
            key = keycode;
        } else if (pressedKeys.containsValue(Keysyms.Alt_L) &&
                   VncViewer.os.startsWith("mac os x") && key > 127) {
          // Un*x and Windows servers expect that, if Alt + an ASCII key is
          // pressed, the key event for the ASCII key will be the same as if
          // Alt had not been pressed.  On OS X, however, the Alt/Option keys
          // act like AltGr keys, so if Alt + an ASCII key is pressed, the key
          // code is the ASCII key symbol, but the key char is the code for the
          // alternate graphics symbol.
          if (keycode >= 65 && keycode <= 90 &&
              !pressedKeys.containsValue(Keysyms.Shift_L) &&
              !pressedKeys.containsValue(Keysyms.Shift_R))
            key = keycode + 32;
          else if (keycode == KeyEvent.VK_QUOTE)
            key = '\'';
          else if (keycode >= 32 && keycode <= 126)
            key = keycode;
        }
        switch (keycode) {
        case KeyEvent.VK_DEAD_ABOVEDOT:
          if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
            keysym = Keysyms.Dead_AboveDot;
          break;
        case KeyEvent.VK_DEAD_ABOVERING:
          if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
            keysym = Keysyms.Dead_AboveRing;
          break;
        case KeyEvent.VK_DEAD_ACUTE:
          if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
            keysym = Keysyms.Dead_Acute;
          break;
        case KeyEvent.VK_DEAD_BREVE:
          if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
            keysym = Keysyms.Dead_Breve;
          break;
        case KeyEvent.VK_DEAD_CARON:
          if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
            keysym = Keysyms.Dead_Caron;
          break;
        case KeyEvent.VK_DEAD_CEDILLA:
          if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
            keysym = Keysyms.Dead_Cedilla;
          break;
        case KeyEvent.VK_DEAD_CIRCUMFLEX:
          if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
            keysym = Keysyms.Dead_Circumflex;
          break;
        case KeyEvent.VK_DEAD_DIAERESIS:
          if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
            keysym = Keysyms.Dead_Diaeresis;
          break;
        case KeyEvent.VK_DEAD_DOUBLEACUTE:
          if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
            keysym = Keysyms.Dead_DoubleAcute;
          break;
        case KeyEvent.VK_DEAD_GRAVE:
          if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
            keysym = Keysyms.Dead_Grave;
          break;
        case KeyEvent.VK_DEAD_IOTA:
          if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
            keysym = Keysyms.Dead_Iota;
          break;
        case KeyEvent.VK_DEAD_MACRON:
          if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
            keysym = Keysyms.Dead_Macron;
          break;
        case KeyEvent.VK_DEAD_OGONEK:
          if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
            keysym = Keysyms.Dead_Ogonek;
          break;
        case KeyEvent.VK_DEAD_SEMIVOICED_SOUND:
          if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
            keysym = Keysyms.Dead_Semivoiced_Sound;
          break;
        case KeyEvent.VK_DEAD_TILDE:
          if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
            keysym = Keysyms.Dead_Tilde;
          break;
        case KeyEvent.VK_DEAD_VOICED_SOUND:
          if (location != KeyEvent.KEY_LOCATION_UNKNOWN)
            keysym = Keysyms.Dead_Voiced_Sound;
          break;
        default:
          keysym = UnicodeToKeysym.ucs2keysym(key);
        }
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
          keysym = Keysyms.KP_Home;
        else
          keysym = Keysyms.Home;  break;
      case KeyEvent.VK_END:
        if (location == KeyEvent.KEY_LOCATION_NUMPAD)
          keysym = Keysyms.KP_End;
        else
          keysym = Keysyms.End;  break;
      case KeyEvent.VK_PAGE_UP:
        if (location == KeyEvent.KEY_LOCATION_NUMPAD)
          keysym = Keysyms.KP_Page_Up;
        else
          keysym = Keysyms.Page_Up;  break;
      case KeyEvent.VK_PAGE_DOWN:
        if (location == KeyEvent.KEY_LOCATION_NUMPAD)
          keysym = Keysyms.KP_Page_Down;
        else
          keysym = Keysyms.Page_Down;  break;
      case KeyEvent.VK_UP:
        if (location == KeyEvent.KEY_LOCATION_NUMPAD)
          keysym = Keysyms.KP_Up;
        else
          keysym = Keysyms.Up;  break;
      case KeyEvent.VK_DOWN:
        if (location == KeyEvent.KEY_LOCATION_NUMPAD)
          keysym = Keysyms.KP_Down;
        else
         keysym = Keysyms.Down;  break;
      case KeyEvent.VK_LEFT:
        if (location == KeyEvent.KEY_LOCATION_NUMPAD)
          keysym = Keysyms.KP_Left;
        else
         keysym = Keysyms.Left;  break;
      case KeyEvent.VK_RIGHT:
        if (location == KeyEvent.KEY_LOCATION_NUMPAD)
          keysym = Keysyms.KP_Right;
        else
          keysym = Keysyms.Right;  break;
      case KeyEvent.VK_F1:           keysym = Keysyms.F1;  break;
      case KeyEvent.VK_F2:           keysym = Keysyms.F2;  break;
      case KeyEvent.VK_F3:           keysym = Keysyms.F3;  break;
      case KeyEvent.VK_F4:           keysym = Keysyms.F4;  break;
      case KeyEvent.VK_F5:           keysym = Keysyms.F5;  break;
      case KeyEvent.VK_F6:           keysym = Keysyms.F6;  break;
      case KeyEvent.VK_F7:           keysym = Keysyms.F7;  break;
      case KeyEvent.VK_F8:           keysym = Keysyms.F8;  break;
      case KeyEvent.VK_F9:           keysym = Keysyms.F9;  break;
      case KeyEvent.VK_F10:          keysym = Keysyms.F10;  break;
      case KeyEvent.VK_F11:          keysym = Keysyms.F11;  break;
      case KeyEvent.VK_F12:          keysym = Keysyms.F12;  break;
      case KeyEvent.VK_F13:          keysym = Keysyms.F13;  break;
      case KeyEvent.VK_HELP:         keysym = Keysyms.Help;  break;
      case KeyEvent.VK_UNDO:         keysym = Keysyms.Undo;  break;
      case KeyEvent.VK_AGAIN:        keysym = Keysyms.Redo;  break;
      case KeyEvent.VK_PRINTSCREEN:  keysym = Keysyms.Print;  break;
      case KeyEvent.VK_PAUSE:
        if (ev.isControlDown())
          keysym = Keysyms.Break;
        else
          keysym = Keysyms.Pause;
        break;
      case KeyEvent.VK_INSERT:
        if (location == KeyEvent.KEY_LOCATION_NUMPAD)
          keysym = Keysyms.KP_Insert;
        else
          keysym = Keysyms.Insert;  break;
      case KeyEvent.VK_KP_DOWN:      keysym = Keysyms.KP_Down;  break;
      case KeyEvent.VK_KP_LEFT:      keysym = Keysyms.KP_Left;  break;
      case KeyEvent.VK_KP_RIGHT:     keysym = Keysyms.KP_Right;  break;
      case KeyEvent.VK_KP_UP:        keysym = Keysyms.KP_Up;  break;
      case KeyEvent.VK_NUM_LOCK:     keysym = Keysyms.Num_Lock;  break;
      case KeyEvent.VK_WINDOWS:
        if (location == KeyEvent.KEY_LOCATION_RIGHT)
          keysym = Keysyms.Super_R;
        else
          keysym = Keysyms.Super_L;  break;
      case KeyEvent.VK_CONTEXT_MENU: keysym = Keysyms.Menu;  break;
      case KeyEvent.VK_SCROLL_LOCK:  keysym = Keysyms.Scroll_Lock;  break;
      case KeyEvent.VK_CAPS_LOCK:    keysym = Keysyms.Caps_Lock;  break;
      case KeyEvent.VK_BEGIN:
        if (location == KeyEvent.KEY_LOCATION_NUMPAD)
          keysym = Keysyms.KP_Begin;
        else
          keysym = Keysyms.Begin;  break;
      default:
        debugStr += " NO KEYSYM";
        vlog.debug(debugStr);
        return;
      }
    }

    if (winAltGr) {
      if (pressedKeys.containsValue(Keysyms.Control_L)) {
        vlog.debug("Fake L Ctrl released");
        writeKeyEvent(Keysyms.Control_L, false);
      }
      if (pressedKeys.containsValue(Keysyms.Alt_L)) {
        vlog.debug("Fake L Alt released");
        writeKeyEvent(Keysyms.Alt_L, false);
      }
      if (pressedKeys.containsValue(Keysyms.Control_R)) {
        vlog.debug("Fake R Ctrl released");
        writeKeyEvent(Keysyms.Control_R, false);
      }
      if (pressedKeys.containsValue(Keysyms.Alt_R)) {
        vlog.debug("Fake R Alt released");
        writeKeyEvent(Keysyms.Alt_R, false);
      }
    }
    debugStr += String.format(" => 0x%04x", keysym);
    vlog.debug(debugStr);
    pressedKeys.put(keycode, keysym);
    writeKeyEvent(keysym, down);
    if (winAltGr) {
      if (pressedKeys.containsValue(Keysyms.Control_L)) {
        vlog.debug("Fake L Ctrl pressed");
        writeKeyEvent(Keysyms.Control_L, true);
      }
      if (pressedKeys.containsValue(Keysyms.Alt_L)) {
        vlog.debug("Fake L Alt pressed");
        writeKeyEvent(Keysyms.Alt_L, true);
      }
      if (pressedKeys.containsValue(Keysyms.Control_R)) {
        vlog.debug("Fake R Ctrl pressed");
        writeKeyEvent(Keysyms.Control_R, true);
      }
      if (pressedKeys.containsValue(Keysyms.Alt_R)) {
        vlog.debug("Fake R Alt pressed");
        writeKeyEvent(Keysyms.Alt_R, true);
      }
    }
  }


  public static final int rfbButton1Mask = 1;
  public static final int rfbButton2Mask = 2;
  public static final int rfbButton3Mask = 4;
  public static final int rfbButton4Mask = 8;
  public static final int rfbButton5Mask = 16;

  // EDT
  public void writePointerEvent(MouseEvent ev) {
    if (state() != RFBSTATE_NORMAL || shuttingDown || benchmark)
      return;

    switch (ev.getID()) {
    case MouseEvent.MOUSE_PRESSED:
      switch(ev.getButton()) {
      case 1:
        buttonMask |= rfbButton1Mask;  break;
      case 2:
        buttonMask |= rfbButton2Mask;  break;
      case 3:
        buttonMask |= rfbButton3Mask;  break;
      default:
        return;
      }
      vlog.debug("mouse PRESS, button " + ev.getButton() +
                 ", coords " + ev.getX() + "," + ev.getY());
      break;
    case MouseEvent.MOUSE_RELEASED:
      switch(ev.getButton()) {
      case 1:
        buttonMask &= ~rfbButton1Mask;  break;
      case 2:
        buttonMask &= ~rfbButton2Mask;  break;
      case 3:
        buttonMask &= ~rfbButton3Mask;  break;
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
    if (clicks < 0) {
      wheelMask = buttonMask | rfbButton4Mask;
    } else {
      wheelMask = buttonMask | rfbButton5Mask;
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


  public Socket getSocket() {
    return sock;
  }


  ////////////////////////////////////////////////////////////////////
  // The following methods are called from both the RFB thread and EDT.

  // checkEncodings() sends a setEncodings message if one is needed.
  private void checkEncodings() {
    if (encodingChange && (writer() != null)) {
      vlog.info("Requesting " + Encodings.encodingName(currentEncoding) +
        " encoding");
      writer().writeSetEncodings(currentEncoding, lastServerEncoding, opts);
      encodingChange = false;
      if (viewport != null)
        viewport.updateTitle();
    }
  }

  // The following need no synchronization:
  VncViewer viewer;
  public static UserPasswdGetter upg;

  // shuttingDown is set in the EDT and is only ever tested by the RFB thread
  // after the window has been destroyed.
  boolean shuttingDown = false;

  // All menu, options, about and info stuff is done in the EDT (apart from
  // initial construction.)
  F8Menu menu;
  OptionsDialog options;

  // clipboard sync issues?
  ClipboardDialog clipboardDialog;

  Options opts;

  int buttonMask;  // EDT only

  private Socket sock;

  protected DesktopWindow desktop;

  // FIXME: should be private
  public PixelFormat serverPF;
  private PixelFormat fullColourPF;

  private boolean pendingPFChange;
  private PixelFormat pendingPF;
  public boolean pendingServerResize;
  Rect pendingAutoResize = new Rect();

  public int currentEncoding, lastServerEncoding;

  private boolean formatChange;
  private boolean encodingChange;

  public boolean firstUpdate;
  private boolean pendingUpdate;
  private boolean continuousUpdates;

  private boolean forceNonincremental;

  private boolean supportsSyncFence;

  private HashMap<Integer, Integer> pressedKeys;
  Viewport viewport;
  boolean showToolbar;
  boolean keyboardGrabbed;

  public double tDecode, tBlit;
  public long decodePixels, decodeRect, blitPixels, blits;
  double tDecodeStart, tReadOld;
  boolean benchmark;

  double tStart = -1.0, tElapsed, tUpdateStart, tUpdate;
  long updates;
  ProfileDialog profileDialog;
  boolean alwaysProfile;

  static LogWriter vlog = new LogWriter("CConn");
}
