/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright 2011 Pierre Ossman <ossman@cendio.se> for Cendio AB
 * Copyright (C) 2011-2016 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2011-2013 Brian P. Hinz
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
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Image;
import java.io.*;
import java.util.jar.Attributes;
import java.util.jar.Manifest;
import java.util.List;
import javax.swing.*;
import java.lang.reflect.*;

import com.turbovnc.rdr.*;
import com.turbovnc.rfb.*;
import com.turbovnc.network.*;

public class VncViewer extends javax.swing.JApplet
  implements Runnable, ActionListener, OptionsDialogCallback {
  public static final String PRODUCT_NAME = "TurboVNC Viewer";
  public static String copyrightYear = null;
  public static String copyright = null;
  public static String url = null;
  public static String version = null;
  public static String build = null;
  public static String pkgDate = null;
  public static String pkgTime = null;

  private static final ImageIcon frameIcon =
    new ImageIcon(VncViewer.class.getResource("turbovnc-sm.png"));
  public static final Image frameImage = frameIcon.getImage();
  public static final ImageIcon logoIcon =
    new ImageIcon(VncViewer.class.getResource("turbovnc.png"));
  public static final ImageIcon logoIcon128 =
    new ImageIcon(VncViewer.class.getResource("turbovnc-128.png"));

  void setVersion() {
    if (version == null || build == null || copyrightYear == null ||
        copyright == null) {
      ClassLoader cl = getClass().getClassLoader();
      InputStream stream =
        cl.getResourceAsStream("com/turbovnc/vncviewer/timestamp");
      if (stream == null) {
        vlog.error("WARNING: Could not read JAR timestamp.  Version information will be");
        vlog.error("         incorrect.");
        version = "NOVERSION";
        build = "NOBUILD";
        copyrightYear = "NOYEAR";
        copyright = "NOCOPYRIGHT";
        url = "NOURL";
        pkgDate = "NODATE";
        pkgTime = "NOTIME";
        return;
      }
      try {
        Manifest manifest = new Manifest(stream);
        Attributes attributes = manifest.getMainAttributes();
        version = attributes.getValue("Version");
        build = attributes.getValue("Build");
        copyrightYear = attributes.getValue("Copyright-Year");
        copyright = attributes.getValue("Copyright");
        url = attributes.getValue("URL");
        pkgDate = attributes.getValue("Package-Date");
        pkgTime = attributes.getValue("Package-Time");
      } catch (IOException e) {}
    }
  }

  static final double getTime() {
    return (double)System.nanoTime() / 1.0e9;
  }

  public static final boolean getBooleanProperty(String key, boolean def) {
    String prop = System.getProperty(key, def ? "True" : "False");
    if (prop != null && prop.length() > 0) {
      if (prop.equalsIgnoreCase("true") || prop.equalsIgnoreCase("yes"))
        return true;
      if (prop.equalsIgnoreCase("false") || prop.equalsIgnoreCase("no"))
        return false;
      int i = -1;
      try {
        i = Integer.parseInt(prop);
      } catch (NumberFormatException e) {};
      if (i == 1)
        return true;
      if (i == 0)
        return false;
    }
    return def;
  }

  public static final String os = System.getProperty("os.name").toLowerCase();

  public static boolean isX11() {
    return !os.startsWith("mac os x") && !os.startsWith("windows");
  }

  public static boolean osEID() {
    return !os.startsWith("mac os x") && !os.startsWith("windows");
  }

  public static boolean osGrab() {
    return !os.startsWith("mac os x");
  }

  // This allows the Mac app to handle .vnc files opened or dragged onto its
  // icon from the Finder.

  static String fileName;

  static class MyInvocationHandler implements InvocationHandler {
    public Object invoke(Object proxy, Method method, Object[] args) {
      try {
        if (method.getName().equals("openFiles") && args[0] != null) {
          synchronized(VncViewer.class) {
            Class ofEventClass =
              Class.forName("com.apple.eawt.AppEvent$OpenFilesEvent");
            Method getFiles = ofEventClass.getMethod("getFiles",
                                                     (Class[])null);
            List<File> files =(List<File>)getFiles.invoke(args[0]);
            String fName = files.iterator().next().getAbsolutePath();
            if (nViewers == 0)
              fileName = fName;
            else {
              VncViewer viewer = new VncViewer(new String[]{});
              try {
                Configuration.load(fName);
              } catch (Exception e) {
                viewer.reportException(e);
                return null;
              }
              viewer.setGlobalOptions();
              viewer.start();
            }
          }
        }
      } catch (Exception e) {
        vlog.error("Invocation handler failed:");
        vlog.error("  " + e.toString());
      }
      return null;
    }
  }

  static void enableFileHandler() throws Exception {
    Class appClass = Class.forName("com.apple.eawt.Application");
    Method getApplication = appClass.getMethod("getApplication",
                                               (Class[])null);
    Object app = getApplication.invoke(appClass);

    Class fileHandlerClass = Class.forName("com.apple.eawt.OpenFilesHandler");
    InvocationHandler handler = new MyInvocationHandler();
    Object proxy = Proxy.newProxyInstance(fileHandlerClass.getClassLoader(),
                                          new Class[]{fileHandlerClass},
                                          handler);
    Method setOpenFileHandler =
      appClass.getMethod("setOpenFileHandler", fileHandlerClass);
    setOpenFileHandler.invoke(app, new Object[]{proxy});
  }

  static void setLookAndFeel() {
    try {
      if (os.startsWith("windows")) {
        String laf = "com.sun.java.swing.plaf.windows.WindowsLookAndFeel";
        UIManager.setLookAndFeel(laf);
      } else {
        UIManager.put("swing.boldMetal", Boolean.FALSE);
        javax.swing.plaf.FontUIResource f = new
          javax.swing.plaf.FontUIResource("SansSerif", Font.PLAIN, 11);
        java.util.Enumeration<Object> keys = UIManager.getDefaults().keys();
        while (keys.hasMoreElements()) {
          Object key = keys.nextElement();
          Object value = UIManager.get(key);
          if (value instanceof javax.swing.plaf.FontUIResource)
            UIManager.put(key, f);
        }
      }
      UIManager.put("TitledBorder.titleColor", Color.blue);
      if (os.startsWith("mac os x")) {
        System.setProperty("apple.laf.useScreenMenuBar", "true");
        enableFileHandler();

        try {
          Class appClass = Class.forName("com.apple.eawt.Application");
          Method getApplication =
            appClass.getMethod("getApplication", (Class[])null);
          Object app = getApplication.invoke(appClass);
          Class paramTypes[] = new Class[1];
          paramTypes[0] = Image.class;
          Method setDockIconImage =
            appClass.getMethod("setDockIconImage", paramTypes);
          setDockIconImage.invoke(app, logoIcon128.getImage());
        } catch (Exception e) {
          vlog.debug("Could not set OS X dock icon:");
          vlog.debug("  " + e.toString());
        }
      }

      // Set the shared frame's icon, which will be inherited by any ownerless
      // dialogs that do not have a null owner.
      JDialog dlg = new JDialog();
      Object owner = dlg.getOwner();
      if (owner instanceof Frame && owner != null)
        ((Frame)owner).setIconImage(frameImage);
      dlg.dispose();

    } catch (Exception e) {
      vlog.error("Could not set look & feel:");
      vlog.error("  " + e.toString());
    }
  }

  static void setGlobalInsets() {
    try {
      // To make a short story long:
      // -- Swing can't determine the proper inset values for a JFrame until it
      //    is visible.
      // -- We need to know the inset values in order to initially size the
      //    JFrame properly.
      // -- Making the JFrame visible prior to sizing it causes issues on OS X
      //    (clicking the Maximize gadget reduces the window to nothing.)
      // -- On Linux, the inset values sometimes change by 1 pixel upon
      //    subsequent calls to getInsets().
      // Thus, we have to compute the insets globally using a dummy JFrame.
      // Dear Swing, eff ewe.
      if (!embed.getValue()) {
        final JFrame frame = new JFrame();
        // Under certain Linux WM's, the insets aren't valid until
        // componentResized() is called (see above.)
        if (isX11()) {
          frame.addComponentListener(new ComponentAdapter() {
            public void componentResized(ComponentEvent e) {
              synchronized(frame) {
                if (frame.isVisible() && frame.getExtendedState() == JFrame.NORMAL) {
                  insets = frame.getInsets();
                  frame.notifyAll();
                }
              }
            }
          });
          frame.setExtendedState(JFrame.NORMAL);
          frame.setVisible(true);
          synchronized(frame) {
            while (insets == null)
              frame.wait();
          }
          frame.setVisible(false);
        } else {
          frame.setVisible(true);
          insets = frame.getInsets();
          frame.setVisible(false);
        }
        frame.dispose();
      }
    } catch (Exception e) {
      vlog.error("Could not set insets:");
      vlog.error("  " + e.toString());
    }
  }
  public static void setBlitterDefaults() {
    // Java 1.7 and later do not include hardware-accelerated 2D blitting
    // routines on Mac platforms.  They only support OpenGL blitting, and using
    // TYPE_INT_ARGB_PRE BufferedImages with OpenGL blitting is much faster
    // than using TYPE_INT_RGB BufferedImages on some Macs (about 4-5X as fast
    // on certain models.)
    boolean defForceAlpha = false;

    if (os.startsWith("mac os x")) {
      int minorVersion =
        Integer.parseInt(System.getProperty("java.version").split("\\.")[1]);
      if (minorVersion >= 7)
        defForceAlpha = true;
    }
    // TYPE_INT_ARGB_PRE images are also faster when using OpenGL blitting on
    // other platforms, so attempt to detect that.
    boolean useOpenGL =
      Boolean.parseBoolean(System.getProperty("sun.java2d.opengl"));
    if (useOpenGL)
      defForceAlpha = true;

    forceAlpha = getBooleanProperty("turbovnc.forcealpha", defForceAlpha);

    // Disable Direct3D Java 2D blitting unless the user specifically requests
    // it (by setting the sun.java2d.d3d property to true.)  GDI Java 2D
    // blitting is almost always faster than D3D, but D3D is normally the
    // default.  Note that this doesn't work with Java 1.6 and earlier, for
    // unknown reasons.  Apparently it reads the Java 2D system properties
    // before our code can influence them.
    if (os.startsWith("windows")) {
      String prop = System.getProperty("sun.java2d.d3d");
      if (prop == null || prop.length() < 1 || !Boolean.parseBoolean(prop))
        System.setProperty("sun.java2d.d3d", "false");
      prop = System.getProperty("sun.awt.nopixfmt");
      if (prop == null || prop.length() < 1 || Boolean.parseBoolean(prop))
        System.setProperty("sun.awt.nopixfmt", "true");
      else
        System.clearProperty("sun.awt.nopixfmt");
    }
  }

  static {
    setBlitterDefaults();
  }

  public static void main(String[] argv) {
    setLookAndFeel();
    VncViewer viewer = new VncViewer(argv);
    if (os.startsWith("mac os x")) {
      synchronized(VncViewer.class) {
        if (fileName != null) {
          try {
            Configuration.load(fileName);
          } catch (Exception e) {
            viewer.reportException(e);
            return;
          }
          viewer.setGlobalOptions();
          fileName = null;
        }
      }
    }
    viewer.start();
  }

  public VncViewer(String[] argv) {
    applet = false;

    UserPreferences.load("global");

    setVersion();

    // Override defaults with command-line options
    for (int i = 0; i < argv.length; i++) {
      if (argv[i].length() == 0)
        continue;

      // The following is primarily included so we can ensure that we're
      // running either a 32-bit or a 64-bit JRE from the Windows .bat file
      if (argv[i].equalsIgnoreCase("-reqarch")) {
        if (++i < argv.length) {
          if (!argv[i].equalsIgnoreCase(System.getProperty("os.arch"))) {
            reportException(new WarningException("You must use a " +
              argv[i] + " Java Runtime Environment with this version of TurboVNC."));
            exit(1);
          }
        }
        continue;
      }

      if (argv[i].equalsIgnoreCase("-config")) {
        if (++i >= argv.length) usage();
        try {
          Configuration.load(argv[i]);
        } catch (Exception e) {
          reportException(e);
          exit(1);
        }
        continue;
      }

      if (argv[i].equalsIgnoreCase("-loglevel")) {
        if (++i >= argv.length) usage();
        System.err.println("Log setting: " + argv[i]);
        LogWriter.setLogParams(argv[i]);
        continue;
      }

      if (argv[i].equalsIgnoreCase("-bench")) {
        if (i < argv.length - 1) {
          try {
            benchFile = new FileInStream(argv[++i]);
          } catch (Exception e) {
            reportException(new WarningException("Could not open session capture:\n" +
                                                 e.getMessage()));
            exit(1);
          }
        }
        continue;
      }

      if (argv[i].equalsIgnoreCase("-benchiter")) {
        if (i < argv.length - 1) {
          int iter = Integer.parseInt(argv[++i]);
          if (iter > 0) benchIter = iter;
        }
        continue;
      }

      if (argv[i].equalsIgnoreCase("-benchwarmup")) {
        if (i < argv.length - 1) {
          int warmup = Integer.parseInt(argv[++i]);
          if (warmup > 0) benchWarmup = warmup;
        }
        continue;
      }

      if (Configuration.setParam(argv[i]))
        continue;

      if (argv[i].charAt(0) == '-') {
        if (i + 1 < argv.length) {
          if (Configuration.setParam(argv[i].substring(1), argv[i + 1])) {
            i++;
            continue;
          }
        }
        usage();
      }

      if (argv[i].toLowerCase().endsWith(".vnc")) {
        try {
          Configuration.load(argv[i]);
        } catch (Exception e) {
          reportException(e);
          exit(1);
        }
        continue;
      }

      if (vncServerName.getValue() != null)
        usage();
      vncServerName.setParam(argv[i]);
    }

    setGlobalOptions();

    embed.setParam(false);
  }

  public static void usage() {
    String usage = ("\n" +
                    "USAGE: VncViewer [options/parameters] [host:displayNum] [options/parameters]\n" +
                    "       VncViewer [options/parameters] [host::port] [options/parameters]\n" +
                    "       VncViewer [options/parameters] -listen [port] [options/parameters]\n" +
                    "\n" +
                    "Options:\n" +
                    "  -loglevel <level>   configure logging level\n" +
                    "                      0 = errors only\n" +
                    "                      10 = status messages\n" +
                    "                      30 = informational messages (default)\n" +
                    "                      100 = debugging messages\n" +
                    "\n" +
                    "Specifying boolean parameters:\n" +
                    "  On:   -<param>=1 or -<param>\n" +
                    "  Off:  -<param>=0 or -no<param>\n" +
                    "Parameters that take a value can be specified as:\n" +
                    "  -<param> <value> or <param>=<value> or -<param>=<value> or --<param>=<value>\n" +
                    "Parameter names and values are case-insensitive (except for the value of\n" +
                    "Password.)\n\n" +
                    "The parameters are:\n\n");
    System.err.println("\nTurboVNC Viewer v" + version + " (build " + build +
                       ") [JVM: " + System.getProperty("os.arch") + "]");
    System.err.println("Copyright (C) " + copyrightYear + " " + copyright);
    System.err.println(url);
    System.err.print(usage);
    Configuration.listParams(80);
    System.exit(1);
  }

  public VncViewer() {
    applet = true;
    UserPreferences.load("global");
    setVersion();
    setGlobalOptions();
  }

  public VncViewer(Socket sock_) {
    sock = sock_;
    UserPreferences.load("global");
    opts.serverName = null;
    opts.port = -1;
  }

  public static void newViewer(VncViewer oldViewer, Socket sock,
                               boolean close) {
    VncViewer viewer = new VncViewer(sock);
    viewer.start();
    if (close)
      oldViewer.exit(0);
  }

  public static void newViewer(VncViewer oldViewer) {
    if (!noNewConn.getValue())
      newViewer(oldViewer, null, false);
  }

  public void init() {
    vlog.debug("init called");
    Container parent = getParent();
    while (!parent.isFocusCycleRoot()) {
      parent = parent.getParent();
    }
    parent.setFocusable(false);
    parent.setFocusTraversalKeysEnabled(false);
    setLookAndFeel();
    setBackground(Color.white);
  }

  public void start() {
    vlog.debug("start called");
    String host = null;
    if (applet && nViewers == 0) {
      Configuration.readAppletParams(this);
      if (embed.getValue()) {
        fullScreen.setParam(false);
        noNewConn.setParam(true);
        scalingFactor.setParam("100");
      }
      String str = getParameter("LogLevel");
      if (str != null)
        LogWriter.setLogParams(str);
      setGlobalOptions();
      host = opts.serverName;
    } else if (!applet)
      host = opts.serverName;
    if (host != null && host.indexOf(':') < 0 &&
        opts.port > 0) {
      opts.serverName = host + ((opts.port >= 5900 && opts.port <= 5999) ?
                         (":" + (opts.port - 5900)) : ("::" + opts.port));
    }
    nViewers++;
    thread = new Thread(this);
    thread.start();
  }

  public void exit(int n) {
    if (nViewers > 0)
      nViewers--;
    if (nViewers > 0 || embed.getValue())
      return;
    if (applet) {
      destroy();
    } else {
      System.exit(n);
    }
  }

  // If "Reconnect" button is pressed
  public void actionPerformed(ActionEvent e) {
    getContentPane().removeAll();
    start();
  }

  void reportException(Exception e) {
    reportException(e, false);
  }
  void reportException(Exception e, boolean reconnect) {
    String title, msg = e.getMessage();
    int msgType = JOptionPane.ERROR_MESSAGE;
    if (e instanceof WarningException) {
      msgType = JOptionPane.WARNING_MESSAGE;
      title = "TurboVNC Viewer";
      System.out.println(msg);
    } else if (e instanceof ErrorException) {
      title = "TurboVNC Viewer : Error";
      System.out.println(msg);
    } else {
      if (!(e instanceof SystemException))
        msg = e.toString();
      title = "TurboVNC Viewer : Unexpected Error";
      e.printStackTrace();
    }
    if (embed.getValue()) {
      getContentPane().removeAll();
      JLabel label = new JLabel("<html><center><b>" + title + "</b><p><i>" +
                                msg + "</i></center></html>", JLabel.CENTER);
      label.setFont(new Font("Helvetica", Font.PLAIN, 24));
      label.setMaximumSize(new Dimension(getSize().width, 100));
      label.setVerticalAlignment(JLabel.CENTER);
      label.setAlignmentX(Component.CENTER_ALIGNMENT);
      JButton button = new JButton("Reconnect");
      button.addActionListener(this);
      button.setMaximumSize(new Dimension(200, 30));
      button.setAlignmentX(Component.CENTER_ALIGNMENT);
      setLayout(new BoxLayout(getContentPane(), BoxLayout.Y_AXIS));
      add(label);
      add(button);
      validate();
      repaint();
    } else {
      JOptionPane pane;
      Object[] options = { UIManager.getString("OptionPane.yesButtonText"),
                           UIManager.getString("OptionPane.noButtonText") };
      if (reconnect)
        pane = new JOptionPane(msg + "\nReconnect?", msgType,
                               JOptionPane.YES_NO_OPTION, null, options,
                               options[1]);
      else
        pane = new JOptionPane(msg, msgType);
      JDialog dlg = pane.createDialog(null, title);
      dlg.setAlwaysOnTop(true);
      dlg.setVisible(true);
      if (reconnect && pane.getValue() == options[0])
        start();
    }
  }

  static void showAbout(Component comp) {
    JOptionPane pane = new JOptionPane(
      VncViewer.PRODUCT_NAME + " v" + VncViewer.version +
        " (" + VncViewer.build + ")\n" +
      "[JVM: " + System.getProperty("java.vm.name") + " " +
        System.getProperty("java.version") + " " +
        System.getProperty("os.arch") + "]\n" +
      "Built on " + VncViewer.pkgDate + " at " + VncViewer.pkgTime + "\n" +
      "Copyright (C) " + VncViewer.copyrightYear + " " + VncViewer.copyright +
        "\n" +
      VncViewer.url, JOptionPane.INFORMATION_MESSAGE);
    pane.setIcon(VncViewer.logoIcon128);
    JDialog dlg = pane.createDialog(comp, "About TurboVNC Viewer");
    if (VncViewer.embed.getValue())
      dlg.setAlwaysOnTop(true);
    dlg.setVisible(true);
  }

  void showOptions() {
    options = new OptionsDialog(this);
    options.initDialog();
    options.showDialog();
  }

  public void setTightOptions() {
    options.setTightOptions(opts.preferredEncoding);
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

    options.shared.setSelected(opts.shared);
    options.sendLocalUsername.setSelected(opts.sendLocalUsername);
    options.setSecurityOptions();

    options.fullScreen.setSelected(opts.fullScreen);
    options.span.setSelectedIndex(opts.span);
    options.cursorShape.setSelected(opts.cursorShape);
    options.acceptBell.setSelected(opts.acceptBell);
    options.showToolbar.setSelected(VncViewer.showToolbar.getValue());
    if (opts.scalingFactor == Options.SCALE_AUTO) {
      options.scalingFactor.setSelectedItem("Auto");
    } else if (opts.scalingFactor == Options.SCALE_FIXEDRATIO) {
      options.scalingFactor.setSelectedItem("Fixed Aspect Ratio");
    } else {
      options.scalingFactor.setSelectedItem(opts.scalingFactor + "%");
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

    options.gateway.setEnabled(false);
    options.sshUser.setEnabled(false);
    options.tunnel.setEnabled(false);
    if (SecurityClient.x509ca.getValue() != null)
      options.x509ca.setText(SecurityClient.x509ca.getValue());
    if (SecurityClient.x509crl.getValue() != null)
      options.x509crl.setText(SecurityClient.x509crl.getValue());
  }

  public void getOptions() {
    opts.allowJpeg = options.allowJpeg.isSelected();
    opts.quality = options.jpegQualityLevel.getValue();
    opts.compressLevel = options.getCompressionLevel();
    opts.subsampling = options.getSubsamplingLevel();
    opts.sendLocalUsername = options.sendLocalUsername.isSelected();
    opts.viewOnly = options.viewOnly.isSelected();
    opts.acceptClipboard = options.acceptClipboard.isSelected();
    opts.sendClipboard = options.sendClipboard.isSelected();
    opts.acceptBell = options.acceptBell.isSelected();
    VncViewer.showToolbar.setParam(options.showToolbar.isSelected());

    opts.setScalingFactor(options.scalingFactor.getSelectedItem().toString());
    opts.setDesktopSize(options.desktopSize.getSelectedItem().toString());

    int index = options.span.getSelectedIndex();
    if (index >= 0 && index < Options.NUMSPANOPT)
      opts.span = index;

    VncViewer.menuKey.setParam(
      MenuKey.getMenuKeySymbols()[options.menuKey.getSelectedIndex()].name);

    if (VncViewer.osGrab() && Viewport.isHelperAvailable())
      opts.grabKeyboard = options.grabKeyboard.getSelectedIndex();

    opts.shared = options.shared.isSelected();
    opts.cursorShape = options.cursorShape.isSelected();

    options.getSecurityOptions();
    SecurityClient.x509ca.setParam(options.x509ca.getText());
    SecurityClient.x509crl.setParam(options.x509crl.getText());
    options = null;
  }

  public boolean supportsSetDesktopSize() {
    return true;
  }

  public void killListener() {
    if (listenThread != null)
      listenThread.interrupt();
  }

  public void run() {
    CConn cc = null;
    int exitStatus = 0;

    if (listenMode.getValue()) {
      int port = 5500;

      listenMode.setParam(false);
      if (opts.serverName != null &&
          Character.isDigit(opts.serverName.charAt(0)))
        port = Integer.parseInt(opts.serverName);
      else if (opts.port > 0)
        port = opts.port;

      if (TrayMenu.isSupported()) {
        try {
          trayMenu = new TrayMenu(this);
        } catch (Exception e) {
          reportException(e);
          exit(1);
        }
      }
      TcpListener listener = null;
      try {
        listener = new TcpListener(null, port);
      } catch (Exception e) {
        reportException(e);
        exit(1);
      }
      listenThread = Thread.currentThread();

      vlog.info("Listening on port " + port);

      while (true) {
        Socket newSock = listener.accept();
        if (newSock != null)
          newViewer(this, newSock, noNewConn.getValue());
        else {
          listener.shutdown();
          vlog.info("Listener exiting ...");
          return;
        }
      }
    }

    double tAvg = 0.0, tAvgDecode = 0.0, tAvgBlit = 0.0;
    if (benchFile == null) { benchIter = 1;  benchWarmup = 0; }

    for (int i = 0; i < benchIter + benchWarmup; i++) {
      double tStart = 0.0, tTotal;

      try {
        if (cc == null)
          cc = new CConn(this, sock);
        if (benchFile != null) {
          if (i < benchWarmup)
            System.out.format("Benchmark warmup run %d\n", i + 1);
          else
            System.out.format("Benchmark run %d:\n", i + 1 - benchWarmup);
          tStart = getTime();
          try {
            while (!cc.shuttingDown)
              cc.processMsg(true);
          } catch (EndOfStream e) {}
          tTotal = getTime() - tStart - benchFile.getReadTime();
          if (i >= benchWarmup) {
            System.out.format("%f s (Decode = %f, Blit = %f)\n", tTotal,
                              cc.tDecode, cc.tBlit);
            System.out.println("     Decode statistics:");
            System.out.format("     %.3f Mpixels, %.3f Mpixels/sec, %d rect, %.0f pixels/rect,\n",
                              (double)cc.decodePixels / 1000000.,
                              (double)cc.decodePixels / 1000000. / cc.tDecode,
                              cc.decodeRect,
                              (double)cc.decodePixels / (double)cc.decodeRect);
            System.out.format("       %.0f rects/update\n",
                              (double)cc.decodeRect / (double)cc.blits);
            System.out.println("     Blit statistics:");
            System.out.format("     %.3f Mpixels, %.3f Mpixels/sec, %d updates, %.0f pixels/update\n",
                              (double)cc.blitPixels / 1000000.,
                              (double)cc.blitPixels / 1000000. / cc.tBlit,
                              cc.blits,
                              (double)cc.blitPixels / (double)cc.blits);
            tAvg += tTotal;
            tAvgDecode += cc.tDecode;
            tAvgBlit += cc.tBlit;
          }
          System.out.print("\n");
          cc.tDecode = cc.tBlit = 0.0;
          cc.decodePixels = cc.decodeRect = cc.blitPixels = cc.blits = 0;
          benchFile.reset();
          benchFile.resetReadTime();
          cc.reset();
          System.gc();
        } else {
          while (!cc.shuttingDown)
            cc.processMsg(false);
        }
      } catch (Exception e) {
        if (cc == null || !cc.shuttingDown) {
          reportException(e, cc != null &&
                          cc.state() == CConnection.RFBSTATE_NORMAL &&
                          !VncViewer.noReconnect.getValue());
          exitStatus = 1;
          if (cc != null) cc.deleteWindow();
        } else if (cc.shuttingDown && embed.getValue()) {
          reportException(new WarningException("Connection closed"));
        } else {
          cc = null;
        }
      }
    }

    if (benchFile != null && benchIter > 1)
      System.out.format("Average          :  %f s (Decode = %f, Blit = %f)\n",
                        tAvg / (double)benchIter,
                        tAvgDecode / (double)benchIter,
                        tAvgBlit / (double)benchIter);

    exit(exitStatus);
  }

  void setGlobalOptions() {
    try {

    if (opts == null)
      opts = new Options();

    opts.port = vncServerPort.getValue();
    vncServerPort.setValue(-1);

    opts.shared = shared.getValue();
    opts.viewOnly = viewOnly.getValue();
    opts.fullScreen = fullScreen.getValue();

    if (osGrab()) {
      if (grabKeyboard.getValue().toLowerCase().startsWith("f"))
        opts.grabKeyboard = Options.GRAB_FS;
      else if (grabKeyboard.getValue().toLowerCase().startsWith("a"))
        opts.grabKeyboard = Options.GRAB_ALWAYS;
      else if (grabKeyboard.getValue().toLowerCase().startsWith("m"))
        opts.grabKeyboard = Options.GRAB_MANUAL;
    }

    if (span.getValue().toLowerCase().startsWith("p"))
      opts.span = Options.SPAN_PRIMARY;
    else if (span.getValue().toLowerCase().startsWith("al"))
      opts.span = Options.SPAN_ALL;
    else
      opts.span = Options.SPAN_AUTO;

    opts.scalingFactor = Integer.parseInt(scalingFactor.getDefaultStr());

    if (benchFile != null)
      opts.desktopSize.mode = Options.SIZE_SERVER;
    else {
      Options.DesktopSize size =
        Options.parseDesktopSize(desktopSize.getValue());
      if (size == null)
        throw new ErrorException("DesktopSize parameter is incorrect");
      opts.desktopSize = size;
    }

    opts.setScalingFactor(scalingFactor.getValue());
    if (opts.scalingFactor != 100 &&
        opts.desktopSize.mode == Options.SIZE_AUTO) {
      vlog.info("Desktop scaling enabled.  Disabling automatic desktop resizing.");
      opts.desktopSize.mode = Options.SIZE_SERVER;
    }

    opts.acceptClipboard = acceptClipboard.getValue();
    opts.sendClipboard = sendClipboard.getValue();
    opts.acceptBell = acceptBell.getValue();

    String encStr = preferredEncoding.getValue();
    int encNum = Encodings.encodingNum(encStr);
    if (encNum != -1)
      opts.preferredEncoding = encNum;
    else
      opts.preferredEncoding =
        Encodings.encodingNum(preferredEncoding.getDefaultStr());

    opts.allowJpeg = allowJpeg.getValue();
    opts.quality = quality.getValue();

    opts.subsampling = Options.SUBSAMP_NONE;
    switch (subsampling.getValue().toUpperCase().charAt(0)) {
      case '2':
        opts.subsampling = Options.SUBSAMP_2X;
        break;
      case '4':
        opts.subsampling = Options.SUBSAMP_4X;
        break;
      case 'G':
        opts.subsampling = Options.SUBSAMP_GRAY;
        break;
    }

    opts.compressLevel = compressLevel.getValue();

    opts.colors = -1;
    switch (colors.getValue()) {
      case 8:  case 64:  case 256:  case 32768:  case 65536:
        opts.colors = colors.getValue();
        break;
    }

    opts.cursorShape = cursorShape.getValue();
    opts.continuousUpdates = continuousUpdates.getValue();
    opts.copyRect = copyRect.getValue();
    if (user.getValue() != null) opts.user = new String(user.getValue());
    opts.sendLocalUsername = sendLocalUsername.getValue();

    String v = via.getValue();
    if (v != null && !v.isEmpty()) {
      int atIndex = v.indexOf('@');
      if (atIndex >= 0) {
        opts.via = v.substring(atIndex + 1);
        opts.sshUser = v.substring(0, atIndex);
      } else {
        opts.via = new String(v);
      }
    }
    opts.tunnel = tunnel.getValue();
    opts.extSSH = extSSH.getValue();

    String s = vncServerName.getValue();
    if (s != null) {
      int atIndex = s.indexOf('@');
      if (atIndex >= 0 && opts.tunnel) {
        opts.serverName = s.substring(atIndex + 1);
        opts.sshUser = s.substring(0, atIndex);
      } else {
        opts.serverName = new String(s);
      }
    }
    vncServerName.setParam(null);

    } catch (Exception e) {
      reportException(new WarningException("Could not set global options:\n" +
                                           e.getMessage()));
      exit(1);
    }

    setGlobalInsets();
  }

  static StringParameter vncServerName
  = new StringParameter("Server",
  "The VNC server to which to connect.  This can be specified in the format " +
  "<host>[:<display number>] or <host>::<port>, where <host> is the host name " +
  "or IP address of the machine on which the VNC server is running, " +
  "<display number> is an optional X display number (default: 0), and <port> " +
  "is a TCP port.", null);

  static BoolParameter alwaysShowConnectionDialog
  = new BoolParameter("AlwaysShowConnectionDialog",
  "Always show the \"New TurboVNC Connection\" dialog even if the server has " +
  "been specified in an applet parameter or on the command line.  This defaults " +
  "to 1 if the viewer is being run as an applet.  This parameter has no effect " +
  "if SSH tunneling is enabled (the \"New TurboVNC Connection\" dialog is " +
  "never shown in that case.)", false);

  static IntParameter vncServerPort
  = new IntParameter("Port",
  "The TCP port number on which the VNC server session is listening.  For Unix " +
  "VNC servers, this is typically 5900 + the X display number of the VNC " +
  "session (example: 5901 if connecting to display :1.)  For Windows and Mac " +
  "VNC servers, this is typically 5900.  (default = 5900)\n " +
  "If listen mode is enabled, this parameter specifies the TCP port on which " +
  "the viewer will listen for connections from a VNC server.  (default = 5500)",
  -1);

  static BoolParameter listenMode
  = new BoolParameter("Listen",
  "Start the viewer in \"listen mode.\"  The viewer will listen on port 5500 " +
  "(or on the port specified by the Port parameter) for reverse connections " +
  "from a VNC server.  To connect to a listening viewer from the Unix/Linux " +
  "TurboVNC Server, use the vncconnect program.", false);

  static BoolParameter shared
  = new BoolParameter("Shared",
  "When connecting, request a shared session.  When the session is shared, " +
  "other users can connect to the session (assuming they have the correct " +
  "authentication credentials) and collaborate with the user who started the " +
  "session.  If this option is disabled and the TurboVNC Server is using " +
  "default settings, then you will only be able to connect to the server if " +
  "no one else is already connected.", true);

  static BoolParameter viewOnly
  = new BoolParameter("ViewOnly",
  "Ignore all keyboard and mouse events in the viewer window and do not pass " +
  "these events to the VNC server.", false);

  // Set to 0 to disable the view-only checkbox in the Options dialog
  static BoolParameter viewOnlyControl
  = new BoolParameter("ViewOnlyControl", null, true);

  // Prevent the viewer from sending Ctrl-Alt-Del and Ctrl-Esc to the server
  static BoolParameter restricted
  = new BoolParameter("Restricted", null, false);

  static BoolParameter noReconnect
  = new BoolParameter("NoReconnect",
  "Normally, if the viewer is disconnected from the server unexpectedly, " +
  "the viewer will ask whether you want to reconnect.  Setting this option " +
  "disables that behavior.", false);

  static StringParameter grabKeyboard
  = new StringParameter("GrabKeyboard",
  osGrab() ? "When the keyboard is grabbed, special key sequences (such as " +
  "Alt-Tab) that are used to switch windows and perform other window " +
  "management functions are passed to the VNC server instead of being " +
  "handled by the local window manager.  The default is to grab the " +
  "keyboard when switching to full-screen mode and ungrab it when exiting " +
  "full-screen mode.  Setting this parameter to \"Always\" grabs the " +
  "keyboard when the viewer starts up and does not automatically ungrab it. " +
  "When this parameter is set to \"Manual\", the keyboard is only grabbed " +
  "or ungrabbed when the \"Grab Keyboard\" option is selected in the F8 " +
  "menu, or when the Ctrl-Alt-Shift-G hotkey is pressed.  Regardless of the " +
  "grabbing mode, the F8 menu option and hotkey can always be used to " +
  "grab or ungrab the keyboard." : null, "FS", "Always, FS, Manual");

  static BoolParameter grabPointer
  = new BoolParameter("GrabPointer",
  isX11() ? "If this option is enabled, then the pointer will be grabbed " +
  "whenever the keyboard is grabbed.  This allows certain keyboard + " +
  "pointer sequences, such as Alt-{drag}, to be passed to the server.  The " +
  "downside, however, is that grabbing the pointer prevents any interaction " +
  "with the local window manager whatsoever (for instance, the window can " +
  "no longer be maximized or closed, and you cannot switch to other running " +
  "applications.)  Thus, this option is primarily useful in conjunction " +
  "with GrabKeyboard=FS." : null, true);

  static BoolParameter noNewConn
  = new BoolParameter("NoNewConn",
  "Always exit after the first connection closes, and do not allow new " +
  "connections to be made without restarting the viewer.  This is useful in " +
  "portal environments that need to control when and how the viewer is " +
  "launched.  This option also disables the \"Close Connection\" option in " +
  "the F8 menu and the \"Disconnect\" button in the toolbar.", false);

  static BoolParameter fullScreen
  = new BoolParameter("FullScreen",
  "Start the viewer in full-screen mode.", false);

  static BoolParameter fsAltEnter
  = new BoolParameter("FSAltEnter",
  "Normally, the viewer will switch into and out of full-screen mode when " +
  "Ctrl-Alt-Shift-F is pressed or \"Full screen\" is selected from the popup " +
  "menu.  Setting this parameter will additionally cause the viewer to switch " +
  "into and out of full-screen mode when Alt-Enter is pressed.", false);

  static StringParameter span
  = new StringParameter("Span",
  "This option specifies whether the viewer window should span all monitors, " +
  "only the primary monitor, or whether it should span all monitors only if it " +
  "cannot fit on the primary monitor (Auto.)  When using automatic desktop " +
  "resizing, Auto has the same effect as Primary.  Due to general issues " +
  "with spanning windows across multiple monitors in X11, this option is " +
  "not available on Un*x/X11 platforms.", "Auto", "Primary, All, Auto");

  static BoolParameter currentMonitorIsPrimary
  = new BoolParameter("CurrentMonitorIsPrimary",
  "If this option is enabled, then the monitor that contains the largest " +
  "number of pixels from the viewer window will be treated as the primary " +
  "monitor for the purposes of spanning.  Otherwise, the left-most and " +
  "top-most monitor will always be the primary monitor (as was the case in " +
  "prior versions of TurboVNC.)", true);

  static BoolParameter showToolbar
  = new BoolParameter("Toolbar",
  "Show the toolbar by default.", true);

  static BoolParameter embed
  = new BoolParameter("Embed",
  "If the TurboVNC Viewer is being run as an applet, display its output to " +
  "an embedded frame in the browser window rather than to a dedicated " +
  "window.  This also has the effect of setting FullScreen=0, " +
  "NoNewConn=1, and Scale=100.", false);

  static StringParameter menuKey
  = new StringParameter("MenuKey",
  "The key used to display the popup menu", "F8",
  MenuKey.getMenuKeyValueStr());

  static StringParameter scalingFactor
  = new StringParameter("Scale",
  "Reduce or enlarge the remote desktop image.  The value is interpreted as a " +
  "scaling factor in percent.  The default value of 100% corresponds to the " +
  "original remote desktop size.  Values below 100 reduce the image size, " +
  "whereas values above 100 enlarge the image proportionally.  If the " +
  "parameter is set to \"Auto\", then automatic scaling is performed. " +
  "Automatic scaling tries to choose a scaling factor in such a way that the " +
  "whole remote desktop will fit on the local screen.  If the parameter is set " +
  "to \"FixedRatio\", then automatic scaling is performed, but the original " +
  "aspect ratio is preserved.  Enabling scaling disables automatic desktop " +
  "resizing.", "100", "1-1000, Auto, or FixedRatio");

  static StringParameter desktopSize
  = new StringParameter("DesktopSize",
  "If the VNC server supports remote desktop resizing, then attempt to " +
  "resize the remote desktop to the specified size (example: 1920x1200)." +
  "Setting this parameter to \"Auto\" causes the remote desktop to be " +
  "resized to fit in the local window without using scrollbars (this is the " +
  "default behavior.)  Setting this parameter to \"Server\" or \"0\" " +
  "disables remote desktop resizing and uses the desktop size set by the " +
  "server.", "Auto", "WxH, Auto, or Server");

  static BoolParameter acceptClipboard
  = new BoolParameter("RecvClipboard",
  "Synchronize the local clipboard with the clipboard of the TurboVNC session " +
  "whenever the latter changes.", true);

  static BoolParameter sendClipboard
  = new BoolParameter("SendClipboard",
  "Synchronize the TurboVNC session clipboard with the local clipboard " +
  "whenever the latter changes.", true);

  static IntParameter maxClipboard
  = new IntParameter("MaxClipboard",
  "Maximum permitted length of an outgoing clipboard update (in bytes)",
  1048576);

  static BoolParameter acceptBell
  = new BoolParameter("AcceptBell",
  "Produce a system beep when a \"bell\" event is received from the server.",
  true);

  static StringParameter preferredEncoding
  = new StringParameter("Encoding",
  "Preferred encoding type to use.  If the server does not support the " +
  "preferred encoding type, then the next best one will be chosen.  There " +
  "should be no reason to use an encoding type other than Tight when " +
  "connecting to a TurboVNC server, but this option can be useful when " +
  "connecting to other types of VNC servers, such as RealVNC.",
  "Tight", "Tight, ZRLE, Hextile, Raw, RRE");

  static BoolParameter allowJpeg
  = new BoolParameter("JPEG",
  "Enable the JPEG subencoding type when using Tight encoding.  This causes " +
  "the Tight encoder to use JPEG compression for subrectangles that have a " +
  "high number of unique colors and indexed color subencoding for " +
  "subrectangles that have a low number of unique colors.  If this option is " +
  "disabled, then the Tight encoder will select between indexed color or raw " +
  "subencoding, depending on the size of the subrectangle and its color count.",
  true);

  static IntParameter quality
  = new IntParameter("Quality",
  "Specifies the JPEG quality to use when compressing JPEG images with the " +
  "Tight+JPEG encoding methods.  Lower quality values produce grainier JPEG " +
  "images with more noticeable compression artifacts, but lower quality " +
  "values also use less network bandwidth and CPU time.  The default value of " +
  "" + Options.DEFQUAL + " should be perceptually lossless (that is, any image compression " +
  "artifacts it produces should be imperceptible to the human eye under most " +
  "viewing conditions.)", Options.DEFQUAL, 1, 100);

  static StringParameter subsampling
  = new StringParameter("Subsampling",
  "When compressing an image using JPEG, the RGB pixels are first converted " +
  "to the YCbCr colorspace, a colorspace in which each pixel is represented as a " +
  "brightness (Y, or \"luminance\") value and a pair of color (Cb & Cr, or " +
  "\"chrominance\") values.  After this colorspace conversion, chrominance " +
  "subsampling can be used to discard some of the chrominance components in " +
  "order to save bandwidth.  1X subsampling retains the chrominance components " +
  "for all pixels, and thus it provides the best image quality but also uses " +
  "the most network bandwidth and CPU time.  2X subsampling retains the " +
  "chrominance components for every other pixel, and 4X subsampling retains " +
  "the chrominance components for every fourth pixel (this is typically " +
  "implemented as 2X subsampling in both X and Y directions.)  Grayscale " +
  "throws out all of the chrominance components, leaving only luminance.  2X " +
  "and 4X subsampling will typically produce noticeable aliasing of lines and " +
  "other sharp features, but with photographic or other \"smooth\" image " +
  "content, it may be difficult to detect any difference between 1X, 2X, and " +
  "4X.", "1X", "1X, 2X, 4X, Gray");

  static AliasParameter samp
  = new AliasParameter("Samp",
  "Alias for Subsampling", subsampling);

  static IntParameter compressLevel
  = new IntParameter("CompressLevel",
  "When Tight encoding is used, the compression level specifies the amount of " +
  "zlib compression to apply to subrectangles encoded using the indexed color, " +
  "mono, and raw subencoding types.  If the JPEG subencoding type is enabled, " +
  "then the compression level also defines the \"palette threshold\", or the " +
  "minimum number of colors that a subrectangle must have before it is encoded " +
  "using JPEG.  Higher compression levels have higher palette thresholds and " +
  "thus favor the use of indexed color subencoding, whereas lower compression " +
  "levels favor the use of JPEG.\n " +
  "Compression Level 1 is usually the default whenever JPEG is enabled, because " +
  "extensive experimentation has revealed little or no benefit to using higher " +
  "compression levels with most 3D and video workloads.  However, v1.1 and " +
  "later of the TurboVNC Server also supports Compression Level 2 when JPEG is " +
  "enabled.  Compression Level 2 can be shown to reduce the bandwidth of " +
  "certain types of low-color workloads by typically 20-40% (with a " +
  "commensurate increase in CPU usage.)\n " +
  "In v1.2 or later of the TurboVNC Server, compression levels 5-7 map to " +
  "compression levels 0-2, but they also enable the interframe comparison engine " +
  "in the server.  Interframe comparison maintains a copy of the remote " +
  "framebuffer for each connected viewer and compares each framebuffer update " +
  "with the copy to ensure that redundant updates are not sent to the viewer.  " +
  "This prevents unnecessary network traffic if an ill-behaved application " +
  "draws the same thing over and over again, but interframe comparison also " +
  "causes the TurboVNC Server to use more CPU time and much more memory.",
  1, 0, 9);

  public static BoolParameter compatibleGUI
  = new BoolParameter("CompatibleGUI",
  "Normally, the TurboVNC Viewer GUI exposes only the settings that are " +
  "useful for TurboVNC servers.  Enabling this option will change the " +
  "compression level slider such that it can be used to select any " +
  "compression level from 0-9, which is useful when connecting to other " +
  "types of VNC servers.  This option is enabled automatically when using " +
  "any encoding type other than Tight or when selecting a compression level " +
  "that the GUI normally does not expose.", false);

  static IntParameter colors
  = new IntParameter("Colors",
  "The color depth to use for the viewer's window.  Specifying 8 will use a " +
  "BGR111 pixel format (1 bit for each red, green, and blue component.) " +
  "Specifying 64 will use a BGR222 pixel format, specifying 256 will use a " +
  "BGR233 pixel format, specifying 32768 will use a BGR555 pixel format, " +
  "and specifying 65536 will use a BGR565 pixel format.  Lowering the color " +
  "depth can significantly reduce bandwidth when using encoding types other " +
  "than Tight or when using Tight encoding without JPEG.  However, colors " +
  "will not be represented accurately, and CPU usage will increase " +
  "substantially (causing a corresponding decrease in performance on fast " +
  "networks.)  The default is to use the native color depth of the display " +
  "on which the viewer is running, which is usually true color (8 bits per " +
  "component.)", -1);

  static BoolParameter cursorShape
  = new BoolParameter("CursorShape",
  "Normally, TurboVNC and compatible servers will send only changes to the " +
  "remote mouse cursor's shape and position.  This results in the best mouse " +
  "responsiveness.  Disabling this option causes the server to instead draw " +
  "the mouse cursor and send it to the viewer as an image every time the " +
  "cursor moves.  Thus, using a remote cursor can increase network \"chatter\" " +
  "between server and client significantly, which may cause performance " +
  "problems on slow networks.  However, using a remote cursor can be " +
  "advantageous with shared sessions, since it will allow you to see the " +
  "cursor movements of other connected users.", true);

  static BoolParameter continuousUpdates
  = new BoolParameter("CU", null, true);

  static BoolParameter copyRect
  = new BoolParameter("CopyRect", null, true);

  static StringParameter secTypes = SecurityClient.secTypes;

  static StringParameter user
  = new StringParameter("User",
  "The user name to use for Unix Login authentication (TightVNC-compatible " +
  "servers) or for Plain and Ident authentication (VeNCrypt-compatible " +
  "servers.)  Specifying this option has the effect of removing any types " +
  "from the SecurityTypes parameter except for \"Plain\" and \"Ident\" " +
  "(and their encrypted derivatives) and \"UnixLogin\", thus allowing only " +
  "authentication schemes that require a user name.", null);

  static BoolParameter noUnixLogin
  = new BoolParameter("NoUnixLogin",
  "This disables the use of Unix Login authentication when connecting to " +
  "TightVNC-compatible servers and Plain authentication when connecting to " +
  "VeNCrypt-compatible servers.  Setting this parameter has the effect of " +
  "removing \"Plain\" (and its encrypted derivatives) and \"UnixLogin\" " +
  "from the SecurityTypes parameter.  This is useful if the server is " +
  "configured to prefer an authentication method that supports " +
  "Unix Login/Plain authentication and you want to override that preference " +
  "for a particular connection (for instance, to use a one-time password.)",
  false);

  static BoolParameter sendLocalUsername
  = new BoolParameter("SendLocalUsername",
  "Send the local user name when using user/password authentication schemes " +
  "(Unix Login, Plain, Ident) rather than prompting for it.  As with the " +
  "\"User\" parameter, setting this parameter has the effect of disabling " +
  "any authentication schemes that don't require a user name.", false);

  static BoolParameter localUsernameLC
  = new BoolParameter("LocalUsernameLC",
  "When the SendLocalUsername option is enabled, enabling this option will " +
  "cause the local user name to be sent in lowercase, which may be useful " +
  "when using the viewer on Windows machines (Windows allows mixed-case " +
  "user names, whereas Un*x generally doesn't.)", false);

  static StringParameter passwordFile
  = new StringParameter("PasswordFile",
  "Password file from which to read the password for Standard VNC " +
  "authentication.  This is useful if your home directory is shared between " +
  "the client and server machines.", null);

  static AliasParameter passwd
  = new AliasParameter("passwd",
  "Alias for PasswordFile", passwordFile);

  static StringParameter password
  = new StringParameter("Password",
  "Plain-text password to use when authenticating with the VNC server.  It is " +
  "strongly recommended that this parameter be used only in conjunction with a " +
  "one-time password or other disposable token.", null);

  static BoolParameter autoPass
  = new BoolParameter("AutoPass",
  "Read a plain-text password from stdin and use this password when " +
  "authenticating with the VNC server.  It is strongly recommended that this " +
  "parameter be used only in conjunction with a one-time password or other " +
  "disposable token.", false);

  static StringParameter encPassword
  = new StringParameter("EncPassword",
  "Encrypted password to use when authenticating with the VNC server.  The " +
  "encrypted password should be in the same ASCII hex format used by " +
  "TurboVNC connection info (.vnc) files.  For instance, you can generate " +
  "an ASCII hex VNC password on the TurboVNC server machine by executing\n " +
  "'cat {VNC_password_file} | xxd -c 256 -ps' or\n " +
  "'echo {unencrypted_password} | /opt/TurboVNC/bin/vncpasswd -f | xxd -c 256 -ps'\n " +
  "This parameter is provided mainly so that web portals can embed a " +
  "password in automatically-generated Java Web Start (JNLP) files without " +
  "exposing the password as plain text.  However, the encryption scheme " +
  "(DES3) used for VNC passwords is not particularly strong, so encrypting " +
  "the password guards against only the most casual of attacks.  It is thus " +
  "recommended that this parameter be used only in conjunction with a " +
  "one-time password or other disposable token.", null);

  static StringParameter via
  = new StringParameter("Via",
  "This parameter specifies an SSH server or UltraVNC repeater " +
  "(\"gateway\") through which the VNC connection should be tunneled.  Note " +
  "that when using the Via parameter, the VNC server host should be " +
  "specified from the point of view of the gateway.  For example, " +
  "specifying Via=gateway_machine Server=localhost:1 will connect to " +
  "display :1 on gateway_machine via the SSH server running on that same " +
  "machine.  Similarly, specifying Via=gateway_machine:0 Server=localhost:1 " +
  "will connect to display :1 on gateway_machine via the UltraVNC repeater " +
  "running on that same machine and listening on port 5900 (VNC display " +
  ":0.)  The VNC server must be specified on the command line or in the " +
  "Server parameter when using the Via parameter.  If using the UltraVNC " +
  "Repeater in \"Mode II\", then specify ID:xxxx as the VNC server name, " +
  "where xxxx is the ID number of the VNC server to which you want to " +
  "connect.  If using an SSH server, then the Via parameter can be prefixed " +
  "by <user>@ to indicate that user name <user> (default = local user name) " +
  "should be used when authenticating with the SSH server.", null);

  static BoolParameter tunnel
  = new BoolParameter("Tunnel",
  "This is the same as using Via with an SSH gateway, except that the " +
  "gateway is assumed to be the same as the VNC server host, so you do not " +
  "need to specify it separately.  The VNC server must be specified on the " +
  "command line or in the Server parameter when using the Tunnel parameter. " +
  "When using the Tunnel parameter, the VNC server host can be prefixed by " +
  "<user>@ to indicate that user name <user> (default = local user name) " +
  "should be used when authenticating with the SSH server.", false);

  static BoolParameter extSSH
  = new BoolParameter("ExtSSH",
  "Use an external SSH client on Un*x systems instead of the built-in SSH " +
  "client.  The external client defaults to /usr/bin/ssh, but you can use " +
  "the VNC_VIA_CMD and VNC_TUNNEL_CMD environment variables or the " +
  "turbovnc.via and turbovnc.tunnel system properties to specify the exact " +
  "command line to use when creating the tunnel.  If one of those " +
  "environment variables or system properties is set, then an external SSH " +
  "client is automatically used.  See the TurboVNC User's Guide for more " +
  "details.", false);

  static IntParameter sshPort
  = new IntParameter("SSHPort",
  "When using the Via or Tunnel options with the built-in SSH client, this " +
  "parameter specifies the TCP port on which the SSH server is " +
  "listening.", 22);

  static StringParameter sshKey
  = new StringParameter("SSHKey",
  "When using the Via or Tunnel options with the built-in SSH client, this " +
  "parameter specifies the text of the SSH private key to use when " +
  "authenticating with the SSH server.  You can use \\n within the string " +
  "to specify a new line.", null);

  static StringParameter sshKeyFile
  = new StringParameter("SSHKeyFile",
  "When using the Via or Tunnel options with the built-in SSH client, this " +
  "parameter specifies a file that contains an SSH private key (or keys) to " +
  "use when authenticating with the SSH server.  If not specified, then the " +
  "built-in SSH client will attempt to read private keys from ~/.ssh/id_dsa " +
  "and ~/.ssh/id_rsa.  It will fall back to asking for an SSH password if " +
  "private key authentication fails.", null);

  static StringParameter sshKeyPass
  = new StringParameter("SSHKeyPass",
  "When using the Via or Tunnel options with the built-in SSH client, this " +
  "parameter specifies the passphrase for the SSH key.", null);

  static StringParameter config
  = new StringParameter("Config",
  "File from which to read connection information.  This file can be generated " +
  "by selecting \"Save connection info as...\" in the system menu of the Windows " +
  "TurboVNC Viewer.", null);

  static IntParameter profileInt
  = new IntParameter("ProfileInterval",
  "TurboVNC includes an internal profiling system that can be used to display " +
  "performance statistics about the connection, such as how many updates per second " +
  "are being received and how much network bandwidth is being used.  Profiling " +
  "is activated by selecting \"Performance Info...\" in the F8 menu, which pops " +
  "up a dialog that displays the statistics.  Profiling can also be enabled on " +
  "the console only by setting the environment variable TVNC_PROFILE to 1.  The " +
  "ProfileInterval parameter specifies how often (in seconds) that the performance " +
  "statistics are updated in the dialog or on the console.  The statistics are " +
  "averaged over this interval.", 5);

  static BoolParameter clientRedirect
  = new BoolParameter("ClientRedirect", null, false);

  Thread thread;
  Socket sock;
  static boolean applet;
  static int nViewers;
  static LogWriter vlog = new LogWriter("main");
  FileInStream benchFile;
  int benchIter = 1;
  int benchWarmup = 0;
  static Options opts;
  static boolean forceAlpha;
  OptionsDialog options;
  TrayMenu trayMenu;
  Thread listenThread;
  static Insets insets;
}
