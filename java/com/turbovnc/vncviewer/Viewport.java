/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2011-2013 Brian P. Hinz
 * Copyright (C) 2012-2013, 2015 D. R. Commander.  All Rights Reserved.
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

package com.turbovnc.vncviewer;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

import java.lang.reflect.*;
import java.io.*;
import java.util.ArrayList;
import java.util.Iterator;

import com.turbovnc.rdr.*;
import com.turbovnc.rfb.*;
import com.turbovnc.rfb.Cursor;

public class Viewport implements Runnable {

  private JComponent content = new JPanel(new BorderLayout());
  JFrame frame;
  boolean useFrame;
  
  public Container getContainer(){
    if(frame != null){
      return frame;
    }
    
    return content;
  }
  
  public Viewport(CConn cc_) {
    this(cc_, true);
  }
  
  public Viewport(CConn cc_, boolean useFrame) {
    cc = cc_;

    if(useFrame){
      frame = new JFrame();
      frame.setIconImage(VncViewer.frameImage);
    }
    
    updateTitle();
    getContainer().setFocusable(false);
    getContainer().setFocusTraversalKeysEnabled(false);
    UIManager.getDefaults().put("ScrollPane.ancestorInputMap",
      new UIDefaults.LazyInputMap(new Object[]{}));
    sp = new JScrollPane();
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
    tb = new Toolbar(cc);
    getContainer().add(tb, BorderLayout.PAGE_START);
    
    getContainer().add(sp, BorderLayout.CENTER);
    if(useFrame){
      frame.getContentPane().add(getContainer());
    }
    
    if (VncViewer.os.startsWith("mac os x") && useFrame) {
      macMenu = new MacMenuBar(cc);
      frame.setJMenuBar(macMenu);
      if (VncViewer.getBooleanProperty("turbovnc.lionfs", true))
        enableLionFS();
    }
    // NOTE: If Lion FS mode is enabled, then the viewport is only created once
    // as a non-full-screen viewport, so we tell showToolbar() to ignore the
    // full-screen state.
    showToolbar(cc.showToolbar, canDoLionFS);

    if(useFrame){
      frame.addWindowFocusListener(new WindowAdapter() {
        public void windowGainedFocus(WindowEvent e) {
          if (sp.getViewport().getView() != null)
            sp.getViewport().getView().requestFocusInWindow();
          if (getContainer().isVisible() && keyboardTempUngrabbed) {
            System.out.println("Keyboard focus regained. Re-grabbing keyboard.");
            grabKeyboardHelper(true);
            keyboardTempUngrabbed = false;
          }
        }
        public void windowLostFocus(WindowEvent e) {
          if (cc.keyboardGrabbed && getContainer().isVisible()) {
            vlog.info("Keyboard focus lost. Temporarily ungrabbing keyboard.");
            grabKeyboardHelper(false);
            keyboardTempUngrabbed = true;
          }
        }
      });

      frame.addWindowListener(new WindowAdapter() {
        public void windowClosing(WindowEvent e) {
          cc.close();
        }
      });
    }

    getContainer().addComponentListener(new ComponentAdapter() {
      public void componentResized(ComponentEvent e) {
        if (cc.opts.scalingFactor == Options.SCALE_AUTO ||
            cc.opts.scalingFactor == Options.SCALE_FIXEDRATIO) {
          if ((sp.getSize().width != cc.desktop.scaledWidth) ||
              (sp.getSize().height != cc.desktop.scaledHeight)) {
            cc.desktop.setScaledSize();
            sp.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
            sp.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_NEVER);
            sp.validate();
            if (getExtendedState() != JFrame.MAXIMIZED_BOTH &&
                !cc.opts.fullScreen) {
              sp.setSize(new Dimension(cc.desktop.scaledWidth,
                                       cc.desktop.scaledHeight));
              int w = cc.desktop.scaledWidth + VncViewer.insets.left +
                      VncViewer.insets.right;
              int h = cc.desktop.scaledHeight + VncViewer.insets.top +
                      VncViewer.insets.bottom;
              if (tb.isVisible())
                h += tb.getHeight();
              if (cc.opts.scalingFactor == Options.SCALE_FIXEDRATIO)
                getContainer().setSize(w, h);
            }
          }
        } else if (cc.opts.desktopSize.mode == Options.SIZE_AUTO &&
                   !cc.firstUpdate && !cc.pendingServerResize) {
          Dimension availableSize = cc.viewport.getAvailableSize();
          if (availableSize.width >= 1 && availableSize.height >= 1 &&
              (availableSize.width != cc.desktop.scaledWidth ||
               availableSize.height != cc.desktop.scaledHeight)) {
            sp.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);
            sp.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED);
            sp.validate();
            if (timer != null)
              timer.stop();
            ActionListener actionListener = new ActionListener() {
              public void actionPerformed(ActionEvent e) {
                Dimension availableSize = cc.viewport.getAvailableSize();
                if (availableSize.width < 1 || availableSize.height < 1)
                  throw new ErrorException("Unexpected zero-size component");
                cc.sendDesktopSize(availableSize.width, availableSize.height, true);
              }
            };
            timer = new Timer(500, actionListener);
            timer.setRepeats(false);
            timer.start();
          }
        } else {
          sp.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);
          sp.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED);
          sp.validate();
        }
        if (cc.desktop.cursor != null) {
          Cursor cursor = cc.desktop.cursor;
          if (cursor.hotspot != null)
            // hotspot will be null until the first cursor update is received
            // from the server.
            cc.setCursor(cursor.width(), cursor.height(), cursor.hotspot,
                         (int[])cursor.data, cursor.mask);
        }
        if (((sp.getSize().width > cc.desktop.scaledWidth) ||
             (sp.getSize().height > cc.desktop.scaledHeight)) &&
            cc.opts.desktopSize.mode != Options.SIZE_AUTO) {
          dx = (sp.getSize().width <= cc.desktop.scaledWidth) ? 0 :
            (int)Math.floor((sp.getSize().width - cc.desktop.scaledWidth) / 2);
          dy = (sp.getSize().height <= cc.desktop.scaledHeight) ? 0 :
            (int)Math.floor((sp.getSize().height - cc.desktop.scaledHeight) / 2);
        } else {
          dx = dy = 0;
        }
        getContainer().repaint();
      }
    });
  }

  public int getExtendedState() {
    if(frame != null){
      return frame.getExtendedState();
    }
    
    return JFrame.NORMAL;
  }
  
  public void setExtendedState(int state){
    if(frame != null){
      frame.setExtendedState(state);
    }
  }
  
  public Dimension getAvailableSize() {
    Dimension availableSize = getContainer().getSize();
    if (!cc.opts.fullScreen) {
      Insets vpInsets = VncViewer.insets;
      availableSize.width -= vpInsets.left + vpInsets.right;
      availableSize.height -= vpInsets.top + vpInsets.bottom;
    }
    if (tb.isVisible())
      availableSize.height -= tb.getHeight();
    if (availableSize.width < 0)
      availableSize.width = 0;
    if (availableSize.height < 0)
      availableSize.height = 0;
    return availableSize;
  }

  public Dimension getBorderSize() {
    if (cc.opts.fullScreen)
      return new Dimension(0, 0);
    Insets vpInsets = VncViewer.insets;
    Dimension borderSize = new Dimension(vpInsets.left + vpInsets.right,
                                         vpInsets.top + vpInsets.bottom);
    if (cc.showToolbar)
      borderSize.height += 22;
    return borderSize;
  }

  boolean lionFSSupported() { return canDoLionFS; }

  class MyInvocationHandler implements InvocationHandler {
    MyInvocationHandler(CConn cc_) { cc = cc_; }

    public Object invoke(Object proxy, Method method, Object[] args) {
      if (method.getName().equals("windowEnteringFullScreen")) {
        cc.opts.fullScreen = true;
        cc.menu.fullScreen.setSelected(cc.opts.fullScreen);
        updateMacMenuFS();
        showToolbar(cc.showToolbar);
      } else if (method.getName().equals("windowExitingFullScreen")) {
        cc.opts.fullScreen = false;
        cc.menu.fullScreen.setSelected(cc.opts.fullScreen);
        updateMacMenuFS();
        showToolbar(cc.showToolbar);
      } else if (method.getName().equals("windowEnteredFullScreen")) {
        cc.sizeWindow();
      }
      return null;
    }

    CConn cc;
  }

  void enableLionFS() {
    try {
      String version = System.getProperty("os.version");
      String[] tokens = version.split("\\.");
      int major = Integer.parseInt(tokens[0]), minor = 0;
      if (tokens.length > 1)
        minor = Integer.parseInt(tokens[1]);
      if (major < 10 || (major == 10 && minor < 7))
        throw new Exception("Operating system version is " + version);

      Class fsuClass = Class.forName("com.apple.eawt.FullScreenUtilities");
      Class argClasses[] = new Class[]{Window.class, Boolean.TYPE};
      Method setWindowCanFullScreen =
        fsuClass.getMethod("setWindowCanFullScreen", argClasses);
      setWindowCanFullScreen.invoke(fsuClass, this, true);

      Class fsListenerClass =
        Class.forName("com.apple.eawt.FullScreenListener");
      InvocationHandler fsHandler = new MyInvocationHandler(cc);
      Object proxy = Proxy.newProxyInstance(fsListenerClass.getClassLoader(),
                                            new Class[]{fsListenerClass},
                                            fsHandler);
      argClasses = new Class[]{Window.class, fsListenerClass};
      Method addFullScreenListenerTo =
        fsuClass.getMethod("addFullScreenListenerTo", argClasses);
      addFullScreenListenerTo.invoke(fsuClass, this, proxy);

      canDoLionFS = true;
    } catch (Exception e) {
      vlog.debug("Could not enable OS X 10.7+ full-screen mode:");
      vlog.debug("  " + e.toString());
    }
  }

  public void toggleLionFS() {
    try {
      Class appClass = Class.forName("com.apple.eawt.Application");
      Method getApplication = appClass.getMethod("getApplication",
                                                 (Class[])null);
      Object app = getApplication.invoke(appClass);
      Method requestToggleFullScreen =
        appClass.getMethod("requestToggleFullScreen", Window.class);
      requestToggleFullScreen.invoke(app, this);
    } catch (Exception e) {
      vlog.debug("Could not toggle OS X 10.7+ full-screen mode:");
      vlog.debug("  " + e.toString());
    }
  }

  public void updateMacMenuFS() {
    if (macMenu != null)
      macMenu.updateFullScreen();
  }

  public void updateMacMenuProfile() {
    if (macMenu != null)
      macMenu.updateProfile();
  }

  public void setChild(DesktopWindow child) {
    sp.getViewport().setView(child);
  }

  public void setGeometry(int x, int y, int w, int h) {
    getContainer().setSize(w, h);
    getContainer().setLocation(x, y);
    vlog.debug("Set geometry to " + x + ", " + y + " " + w + " x " + h);
  }

  public void showToolbar(boolean show) { showToolbar(show, false); }

  private void showToolbar(boolean show, boolean force) {
    tb.setVisible(show && (!cc.opts.fullScreen || force));
  }

  public void updateTitle() {
    if(frame == null){
      return;
    }
    
    int enc = cc.lastServerEncoding;
    if (enc < 0) enc = cc.currentEncoding;
    if (enc == Encodings.encodingTight) {
      if (cc.opts.allowJpeg) {
        String[] subsampStr = { "1X", "4X", "2X", "Gray" };
        frame.setTitle(cc.cp.name() + " [Tight + JPEG " +
                 subsampStr[cc.opts.subsampling] + " Q" + cc.opts.quality +
                 (cc.opts.compressLevel > 1 ? " + CL " + cc.opts.compressLevel : "") +
                 "]");
      } else {
        frame.setTitle(cc.cp.name() + " [Lossless Tight" +
                 (cc.opts.compressLevel == 1 ? " + Zlib" : "") +
                 (cc.opts.compressLevel > 1 ? " + CL " + cc.opts.compressLevel : "") +
                 "]");
      }
    } else {
      frame.setTitle(cc.cp.name() + " [" + Encodings.encodingName(enc) + "]");
    }
  }

  public static boolean isHelperAvailable() {
    if (!triedHelperInit) {
      try {
        System.loadLibrary("turbovnchelper");
        helperAvailable = true;
      } catch (java.lang.UnsatisfiedLinkError e) {
        vlog.info("WARNING: Could not find TurboVNC Helper JNI library.  If it is in a");
        vlog.info("  non-standard location, then add -Djava.library.path=<dir>");
        vlog.info("  to the Java command line to specify its location.");
        vlog.info("  Full-screen mode may not work correctly.");
        if (VncViewer.isX11())
          vlog.info("  Keyboard grabbing and extended input device support will be disabled.");
      } catch (java.lang.Exception e) {
        vlog.info("WARNING: Could not initialize TurboVNC Helper JNI library:");
        vlog.info("  " + e.toString());
        vlog.info("  Full-screen mode may not work correctly.");
        if (VncViewer.isX11())
          vlog.info("  Keyboard grabbing and extended input device support will be disabled.");
      }
    }
    triedHelperInit = true;
    return helperAvailable;
  }

  public void x11FullScreenHelper(boolean on) {
    if (isHelperAvailable()) {
      try {
        x11FullScreen(on);
      } catch (java.lang.UnsatisfiedLinkError e) {
        vlog.info("WARNING: Could not invoke x11FullScreen() from TurboVNC Helper.");
        vlog.info("  Full-screen mode may not work correctly.");
        helperAvailable = false;
      } catch (java.lang.Exception e) {
        vlog.info("WARNING: Could not invoke x11FullScreen() from TurboVNC Helper:");
        vlog.info("  " + e.toString());
        vlog.info("  Full-screen mode may not work correctly.");
      }
    }
  }

  public void grabKeyboardHelper(boolean on) {
    grabKeyboardHelper(on, false);
  }

  public void grabKeyboardHelper(boolean on, boolean force) {
    if (isHelperAvailable()) {
      try {
        if (cc.keyboardGrabbed == on && !force)
          return;
        grabKeyboard(on, VncViewer.grabPointer.getValue());
        cc.keyboardGrabbed = on;
        cc.menu.grabKeyboard.setSelected(cc.keyboardGrabbed);
      } catch (java.lang.UnsatisfiedLinkError e) {
        vlog.info("WARNING: Could not invoke grabKeyboard() from TurboVNC Helper.");
        vlog.info("  Keyboard grabbing will be disabled.");
        helperAvailable = false;
      } catch (java.lang.Exception e) {
        vlog.info("WARNING: Could not invoke grabKeyboard() from TurboVNC Helper:");
        vlog.info("  " + e.toString());
        vlog.info("  Keyboard grabbing may not work correctly.");
      }
    }
  }

  public void extInputHelper() {
    if (isHelperAvailable() && cc.cp.supportsGII) {
      if (thread == null) {
        thread = new Thread(this);
        thread.start();
      }
    }
  }

  public void run() {
    try {
      threadRunning = true;
      extInputEventLoop();
    } catch (java.lang.UnsatisfiedLinkError e) {
      vlog.info("WARNING: Could not invoke extInputEventLoop() from TurboVNC Helper.");
      vlog.info("  Extended input device support will be disabled.");
      helperAvailable = false;
    } catch (java.lang.Exception e) {
      vlog.info("WARNING: Could not invoke extInputEventLoop() from TurboVNC Helper:");
      vlog.info("  " + e.toString());
      vlog.info("  Extended input device support may not work correctly.");
      thread = null;
    }
  }

  synchronized void addInputDevice(ExtInputDevice dev) {
    if (devices == null)
      devices = new ArrayList<ExtInputDevice>();
    devices.add(dev);
    dev.print();
    cc.giiDeviceCreate(dev);
  }

  synchronized void assignInputDevice(int deviceOrigin) {
    if (devices == null) {
      vlog.eidebug("Attempted to assign GII device ID " + deviceOrigin +
                   " to non-existent device");
      return;
    }
    for (Iterator<ExtInputDevice> i = devices.iterator(); i.hasNext();) {
      ExtInputDevice dev = (ExtInputDevice)i.next();
      if (dev.remoteID == 0) {
        dev.remoteID = deviceOrigin;
        vlog.info("Successfully created device " + deviceOrigin + " ("
                  + dev.name + ")");
        break;
      }
    }
  }

  synchronized void sendInputEvent(ExtInputEvent e) {
    if (devices == null) {
      vlog.error("ERROR: Attempted to send extended input event when no devices exist");
      return;
    }
    for (Iterator<ExtInputDevice> i = devices.iterator(); i.hasNext();) {
      ExtInputDevice dev = (ExtInputDevice)i.next();
      if (e.deviceID == dev.id && dev.remoteID != 0) {
        if (dev.absolute && e.type == giiTypes.giiValuatorRelative)
          e.type = giiTypes.giiValuatorAbsolute;
        cc.giiSendEvent(dev, e);
      }
    }
  }

  private native void x11FullScreen(boolean on);
  private native void grabKeyboard(boolean on, boolean pointer);
  private native void extInputEventLoop();

  public void dispose() {
    if(frame != null){
      frame.dispose();
    }
    if (thread != null) {
      threadRunning = false;
      try {
        thread.join();
      } catch (InterruptedException e) {
        vlog.error("InterruptedException while joining thread: " +
                   e.getMessage());
      }
      thread = null;
    }
  }

  CConn cc;
  JScrollPane sp;
  public Toolbar tb;
  public int dx, dy = 0;
  MacMenuBar macMenu;
  boolean canDoLionFS;
  boolean keyboardTempUngrabbed;
  static boolean triedHelperInit, helperAvailable;
  Timer timer;
  Thread thread;
  boolean threadRunning;
  private long x11win;
  ArrayList<ExtInputDevice> devices;

  static LogWriter vlog = new LogWriter("Viewport");
}
