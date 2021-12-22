/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright 2011 Pierre Ossman <ossman@cendio.se> for Cendio AB
 * Copyright (C) 2011-2018, 2020-2021 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2011-2013, 2016 Brian P. Hinz
 * Copyright (C) 2021 Steffen Kieß
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
import java.awt.Image;
import java.io.*;
import java.util.jar.Attributes;
import java.util.jar.Manifest;
import java.util.ArrayList;
import java.util.List;
import javax.swing.*;
import java.lang.reflect.*;

import com.turbovnc.rdr.*;
import com.turbovnc.rfb.*;
import com.turbovnc.network.*;

public class VncViewer implements Runnable, OptionsDialogCallback {
  static final String PRODUCT_NAME = "TurboVNC Viewer";
  static String copyrightYear = null;
  static String copyright = null;
  static String url = null;
  static String version = null;
  static String build = null;
  static String pkgDate = null;
  static String pkgTime = null;

  private static final ImageIcon FRAME_ICON =
    new ImageIcon(VncViewer.class.getResource("turbovnc-sm.png"));
  public static final Image FRAME_IMAGE = FRAME_ICON.getImage();
  public static final ImageIcon LOGO_ICON =
    new ImageIcon(VncViewer.class.getResource("turbovnc.png"));
  public static final ImageIcon LOGO_ICON128 =
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

  public static int getMenuShortcutKeyMask() {
    Method getMenuShortcutKeyMask;
    Integer mask = null;

    try {
      if (Utils.JAVA_VERSION >= 10)
        getMenuShortcutKeyMask =
          Toolkit.class.getMethod("getMenuShortcutKeyMaskEx");
      else
        getMenuShortcutKeyMask =
          Toolkit.class.getMethod("getMenuShortcutKeyMask");
      mask =
        (Integer)getMenuShortcutKeyMask.invoke(Toolkit.getDefaultToolkit());
    } catch (Exception e) {
      vlog.error("Could not get menu shortcut key mask:");
      vlog.error("  " + e.toString());
    }

    return (mask != null ? mask.intValue() : 0);
  }

  // This allows the Mac app to handle .vnc files opened or dragged onto its
  // icon from the Finder.

  static String fileName;

  static class MyInvocationHandler implements InvocationHandler {
    public Object invoke(Object proxy, Method method, Object[] args) {
      try {
        if (method.getName().equals("openFiles") && args[0] != null) {
          synchronized(VncViewer.class) {
            Class ofEventClass = Utils.JAVA_VERSION >= 9 ?
                Class.forName("java.awt.desktop.OpenFilesEvent") :
                Class.forName("com.apple.eawt.AppEvent$OpenFilesEvent");
            Method getFiles = ofEventClass.getMethod("getFiles",
                                                     (Class[])null);
            List<File> files = (List<File>)getFiles.invoke(args[0]);
            String fName = files.iterator().next().getAbsolutePath();
            if (nViewers == 0)
              fileName = fName;
            else {
              VncViewer viewer = new VncViewer(new String[]{});
              try {
                Params.load(fName);
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
    Class appClass, fileHandlerClass;
    Object obj;

    if (Utils.JAVA_VERSION >= 9) {
      appClass = Desktop.class;
      obj = Desktop.getDesktop();
      fileHandlerClass = Class.forName("java.awt.desktop.OpenFilesHandler");
    } else {
      appClass = Class.forName("com.apple.eawt.Application");
      Method getApplication = appClass.getMethod("getApplication",
                                                 (Class[])null);
      obj = getApplication.invoke(appClass);
      fileHandlerClass = Class.forName("com.apple.eawt.OpenFilesHandler");
    }

    InvocationHandler handler = new MyInvocationHandler();
    Object proxy = Proxy.newProxyInstance(fileHandlerClass.getClassLoader(),
                                          new Class[]{ fileHandlerClass },
                                          handler);
    Method setOpenFileHandler =
      appClass.getMethod("setOpenFileHandler", fileHandlerClass);
    setOpenFileHandler.invoke(obj, new Object[]{ proxy });
  }

  static void setLookAndFeel() {
    try {
      if (Utils.isWindows()) {
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
      if (Utils.isMac()) {
        System.setProperty("apple.laf.useScreenMenuBar", "true");
        enableFileHandler();

        try {
          Class appClass;
          Object obj;

          if (Utils.JAVA_VERSION >= 9) {
            appClass = Class.forName("java.awt.Taskbar");
            Method getTaskbar =
              appClass.getMethod("getTaskbar", (Class[])null);
            obj = getTaskbar.invoke(appClass);
          } else {
            appClass = Class.forName("com.apple.eawt.Application");
            Method getApplication =
              appClass.getMethod("getApplication", (Class[])null);
            obj = getApplication.invoke(appClass);
          }

          Class[] paramTypes = new Class[1];
          paramTypes[0] = Image.class;
          Method setDockIconImage = Utils.JAVA_VERSION >= 9 ?
            appClass.getMethod("setIconImage", paramTypes) :
            appClass.getMethod("setDockIconImage", paramTypes);
          setDockIconImage.invoke(obj, LOGO_ICON128.getImage());
        } catch (Exception e) {
          vlog.debug("Could not set dock icon:");
          vlog.debug("  " + e.toString());
        }
        // This allows us to trap Command-Q and shut things down properly.
        Runtime.getRuntime().addShutdownHook(new Thread() {
          public void run() {
            synchronized(VncViewer.conns) {
              for (CConn cc : VncViewer.conns)
                cc.close(false);
              VncViewer.conns.clear();
            }
          }
        });
      }

      // Set the shared frame's icon, which will be inherited by any ownerless
      // dialogs that do not have a null owner.
      JDialog dlg = new JDialog();
      Object owner = dlg.getOwner();
      if (owner instanceof Frame && owner != null)
        ((Frame)owner).setIconImage(FRAME_IMAGE);
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
      final JFrame frame = new JFrame();
      // Under certain Linux WM's, the insets aren't valid until
      // componentResized() is called (see above.)
      if (Utils.isX11()) {
        frame.addComponentListener(new ComponentAdapter() {
          public void componentResized(ComponentEvent e) {
            synchronized(frame) {
              if (frame.isVisible() &&
                  frame.getExtendedState() == JFrame.NORMAL) {
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

    if (Utils.isMac()) {
      if (Utils.JAVA_VERSION >= 7)
        defForceAlpha = true;
    }
    // TYPE_INT_ARGB_PRE images are also faster when using OpenGL blitting on
    // other platforms, so attempt to detect that.
    boolean useOpenGL =
      Boolean.parseBoolean(System.getProperty("sun.java2d.opengl"));
    if (useOpenGL)
      defForceAlpha = true;

    forceAlpha =
      Utils.getBooleanProperty("turbovnc.forcealpha", defForceAlpha);

    // Disable Direct3D Java 2D blitting unless the user specifically requests
    // it (by setting the sun.java2d.d3d property to true.)  GDI Java 2D
    // blitting is almost always faster than D3D, but D3D is normally the
    // default.  Note that this doesn't work with Java 1.6 and earlier, for
    // unknown reasons.  Apparently it reads the Java 2D system properties
    // before our code can influence them.
    if (Utils.isWindows()) {
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

  static void startViewer(String[] argv) {
    VncViewer viewer = new VncViewer(argv);
    if (Utils.isMac()) {
      synchronized(VncViewer.class) {
        if (fileName != null) {
          try {
            Params.load(fileName);
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
    try {
      synchronized(viewer) {
        viewer.wait();
      }
    } catch (InterruptedException e) {
    }
  }

  public static void main(String[] argv) {
    setLookAndFeel();

    // Split argument array at "--" to allow multiple connections to be
    // specified
    int index = 0, i;
    String[] newArgv = null;

    for (i = 0; i < argv.length; i++) {
      if (argv[i].equals("--")) {
        newArgv = new String[i - index];
        System.arraycopy(argv, index, newArgv, 0, i - index);
        if (index > 0)
          Params.reset();
        startViewer(newArgv);
        index = i + 1;
      }
    }

    if (index < argv.length || argv.length == 0) {
      if (index != 0) {
        newArgv = new String[i - index];
        System.arraycopy(argv, index, newArgv, 0, i - index);
        argv = newArgv;
        Params.reset();
      }
      startViewer(argv);
    }
  }

  public VncViewer(String[] argv) {
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
              argv[i] +
              " Java Runtime Environment with this version of TurboVNC."));
            exit(1);
          }
        }
        continue;
      }

      if (argv[i].equalsIgnoreCase("-config")) {
        if (++i >= argv.length) usage();
        try {
          Params.load(argv[i]);
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
            reportException(
              new WarningException("Could not open session capture:\n" +
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

      if (Params.set(argv[i]))
        continue;

      if (argv[i].charAt(0) == '-') {
        if (i + 1 < argv.length) {
          if (Params.set(argv[i].substring(1), argv[i + 1])) {
            i++;
            continue;
          }
        }
        usage();
      }

      if (argv[i].toLowerCase().endsWith(".vnc")) {
        try {
          Params.load(argv[i]);
        } catch (Exception e) {
          reportException(e);
          exit(1);
        }
        continue;
      }

      if (Params.vncServerName.getValue() != null)
        usage();
      Params.vncServerName.setParam(argv[i]);
    }

    setGlobalOptions();
  }

  public static void usage() {
    String usage = "\n" +
      "USAGE: VncViewer [options/parameters] [host] [options/parameters]\n" +
      "       VncViewer [options/parameters] [host:displayNum] [options/parameters]\n" +
      "       VncViewer [options/parameters] [host::port] [options/parameters]\n" +
      "       VncViewer [options/parameters] -listen [port] [options/parameters]\n" +
      "\n" +
      "Multiple VNC servers and associated options/parameters can be specified by\n" +
      "separating the command-line arguments for each server with --.  The TurboVNC\n" +
      "Viewer will connect to the VNC servers serially and in the specified order.\n" +
      "\n" +
      "Options:\n" +
      "  -loglevel <level>   configure logging level\n" +
      "                      0 = errors only\n" +
      "                      10 = status messages\n" +
      "                      30 = informational messages (default)\n" +
      "                      100 = debugging messages\n" +
      "                      110 = SSH debugging messages\n" +
      "                      150 = extended input device debugging messages\n" +
      "\n" +
      "Specifying boolean parameters:\n" +
      "  On:   -<param>=1 or -<param>\n" +
      "  Off:  -<param>=0 or -no<param>\n" +
      "Parameters that take a value can be specified as:\n" +
      "  -<param> <value> or <param>=<value> or -<param>=<value> or --<param>=<value>\n" +
      "Parameter names and values are case-insensitive (except for the value of\n" +
      "Password.)\n\n" +
      "The parameters are:\n\n";
    System.out.println("\nTurboVNC Viewer v" + version + " (build " + build +
                       ") [JVM: " + System.getProperty("os.arch") + "]");
    System.out.println("Copyright (C) " + copyrightYear + " " + copyright);
    System.out.println(url);
    System.out.print(usage);
    Params.list(80);
    System.exit(1);
  }

  public VncViewer(Socket sock_) {
    sock = sock_;
    opts.serverName = null;
    opts.port = -1;
    opts.unixDomainPath = null;
  }

  public static void newViewer(VncViewer oldViewer, Socket sock,
                               boolean close) {
    VncViewer viewer = new VncViewer(sock);
    viewer.start();
    if (close)
      oldViewer.exit(0);
  }

  public static void newViewer(VncViewer oldViewer) {
    if (!Params.noNewConn.getValue())
      newViewer(oldViewer, null, false);
  }

  public static void tileWindows() {
    Rectangle workArea = CConn.getMaxSpannedSize(true);
    int nTilesX, nTilesY;

    synchronized(conns) {
      nTilesX = nTilesY = (int)Math.sqrt(conns.size());
      if (nTilesX * nTilesY < conns.size()) {
        nTilesX++;
        if (nTilesX * nTilesY < conns.size())
          nTilesY++;
      }

      int x = workArea.x, y = workArea.y;
      for (int i = 0; i < conns.size(); i++) {
        CConn cc = conns.get(i);
        int w = workArea.width / nTilesX;
        int h = workArea.height / nTilesY;

        cc.resize(x, y, w, h);
        x += w;
        if (x >= workArea.width) {
          x = 0;  y += h;
        }
      }
    }
  }

  public void start() {
    vlog.debug("start called");
    String host = opts.serverName;
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
    if (nViewers > 0)
      return;
    System.exit(n);
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
      System.err.println(msg);
    } else if (e instanceof ErrorException) {
      title = "TurboVNC Viewer : Error";
      System.err.println(msg);
    } else if (e instanceof SystemException) {
      Throwable cause = e.getCause();
      while (cause instanceof SystemException && cause.getCause() != null)
        cause = cause.getCause();
      msg = cause.toString();
      title = "TurboVNC Viewer : Unexpected Error";
      cause.printStackTrace();
    } else {
      msg = e.toString();
      title = "TurboVNC Viewer : Unexpected Error";
      e.printStackTrace();
    }
    if (VncViewer.noExceptionDialog) return;
    JOptionPane pane;
    Object[] dlgOptions = { UIManager.getString("OptionPane.yesButtonText"),
                            UIManager.getString("OptionPane.noButtonText") };
    if (reconnect)
      pane = new JOptionPane(msg + "\nReconnect?", msgType,
                             JOptionPane.YES_NO_OPTION, null, dlgOptions,
                             dlgOptions[1]);
    else
      pane = new JOptionPane(msg, msgType);
    JDialog dlg = pane.createDialog(null, title);
    dlg.setAlwaysOnTop(true);
    dlg.setVisible(true);
    if (reconnect && pane.getValue() == dlgOptions[0])
      start();
    else {
      synchronized(this) {
        this.notify();
      }
    }
  }

  static void showAbout(Component comp) {
    JOptionPane.showMessageDialog(comp,
      VncViewer.PRODUCT_NAME + " v" + VncViewer.version +
        " (" + VncViewer.build + ")\n" +
      "[JVM: " + System.getProperty("java.vm.name") + " " +
        System.getProperty("java.version") + " " +
        System.getProperty("os.arch") + "]\n" +
      "Built on " + VncViewer.pkgDate + " at " + VncViewer.pkgTime + "\n" +
      "Copyright (C) " + VncViewer.copyrightYear + " " + VncViewer.copyright +
        "\n" +
      VncViewer.url,
      "About TurboVNC Viewer", JOptionPane.INFORMATION_MESSAGE, LOGO_ICON128);
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
    options.setX509Enabled(false);
    options.setOptions(opts, true, true, false, true);
    setTightOptions();
  }

  public void getOptions() {
    options.getOptions(opts);
    opts.save();
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

    if (Params.listenMode.getValue()) {
      int port = 5500;

      Params.listenMode.setParam(false);
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
          newViewer(this, newSock, Params.noNewConn.getValue());
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
        if (cc == null) {
          cc = new CConn(this, sock);
          if (benchFile == null) {
            synchronized(conns) {
              conns.add(cc);
            }
          }
        }
        if (benchFile != null) {
          if (i < benchWarmup)
            System.out.format("Benchmark warmup run %d\n", i + 1);
          else
            System.out.format("Benchmark run %d:\n", i + 1 - benchWarmup);
          tStart = Utils.getTime();
          try {
            while (!cc.shuttingDown)
              cc.processMsg(true);
          } catch (EndOfStream e) {}
          tTotal = Utils.getTime() - tStart - benchFile.getReadTime();
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
          synchronized(conns) {
            conns.remove(cc);
          }
        }
      } catch (Exception e) {
        if (cc == null || !cc.shuttingDown) {
          reportException(e, cc != null &&
                          cc.state() == CConnection.RFBSTATE_NORMAL &&
                          !Params.noReconnect.getValue());
          exitStatus = 1;
          if (cc != null) {
            cc.deleteWindow(true);
            cc.closeSocket();
            synchronized(conns) {
              conns.remove(cc);
            }
          }
        } else {
          cc.closeSocket();
          synchronized(conns) {
            conns.remove(cc);
          }
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

      // CONNECTION OPTIONS
      opts.port = Params.vncServerPort.getValue();
      Params.vncServerPort.setValue(-1);
      opts.recvClipboard = Params.recvClipboard.getValue();
      opts.sendClipboard = Params.sendClipboard.getValue();
      opts.shared = Params.shared.getValue();

      // INPUT OPTIONS
      opts.fsAltEnter = Params.fsAltEnter.getValue();
      if (Utils.osGrab()) {
        if (Params.grabKeyboard.getValue().toLowerCase().startsWith("f"))
          opts.grabKeyboard = Options.GRAB_FS;
        else if (Params.grabKeyboard.getValue().toLowerCase().startsWith("a"))
          opts.grabKeyboard = Options.GRAB_ALWAYS;
        else if (Params.grabKeyboard.getValue().toLowerCase().startsWith("m"))
          opts.grabKeyboard = Options.GRAB_MANUAL;
      }
      opts.menuKeyCode = MenuKey.getMenuKeyCode(Params.menuKey.getValue());
      opts.menuKeySym = MenuKey.getMenuKeySym(Params.menuKey.getValue());
      opts.reverseScroll = Params.reverseScroll.getValue();
      opts.viewOnly = Params.viewOnly.getValue();

      // DISPLAY OPTIONS
      opts.acceptBell = Params.acceptBell.getValue();
      opts.cursorShape = Params.cursorShape.getValue();

      if (benchFile != null)
        opts.desktopSize.mode = Options.SIZE_SERVER;
      else {
        Options.DesktopSize size =
          Options.parseDesktopSize(Params.desktopSize.getValue());
        if (size == null)
          throw new ErrorException("DesktopSize parameter is incorrect");
        opts.desktopSize = size;
      }

      opts.fullScreen = Params.fullScreen.getValue();

      opts.scalingFactor =
        Integer.parseInt(Params.scalingFactor.getDefaultStr());
      opts.setScalingFactor(Params.scalingFactor.getValue());
      if (opts.scalingFactor != 100 &&
          opts.desktopSize.mode == Options.SIZE_AUTO) {
        vlog.info("Desktop scaling enabled.  Disabling automatic desktop resizing.");
        opts.desktopSize.mode = Options.SIZE_SERVER;
      }

      if (Params.span.getValue().toLowerCase().startsWith("p"))
        opts.span = Options.SPAN_PRIMARY;
      else if (Params.span.getValue().toLowerCase().startsWith("al"))
        opts.span = Options.SPAN_ALL;
      else
        opts.span = Options.SPAN_AUTO;

      opts.showToolbar = Params.showToolbar.getValue() && (benchFile == null);

      // ENCODING OPTIONS
      opts.compressLevel = Params.compressLevel.getValue();

      String encStr = Params.preferredEncoding.getValue();
      int encNum = RFB.encodingNum(encStr);
      if (encNum != -1)
        opts.preferredEncoding = encNum;
      else
        opts.preferredEncoding =
          RFB.encodingNum(Params.preferredEncoding.getDefaultStr());

      opts.allowJpeg = Params.allowJpeg.getValue();
      opts.quality = Params.quality.getValue();

      opts.subsampling = Options.SUBSAMP_NONE;
      switch (Params.subsampling.getValue().toUpperCase().charAt(0)) {
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

      // SECURITY AND AUTHENTICATION OPTIONS
      opts.setSecurityTypes(Params.secTypes.getValue());
      opts.sendLocalUsername = Params.sendLocalUsername.getValue();
      opts.tunnel = Params.tunnel.getValue();
      if (Params.user.getValue() != null)
        opts.user = new String(Params.user.getValue());

      String v = Params.via.getValue();
      if (v != null && !v.isEmpty()) {
        int atIndex = v.lastIndexOf('@');
        if (atIndex >= 0) {
          opts.via = v.substring(atIndex + 1);
          opts.sshUser = v.substring(0, atIndex);
        } else {
          opts.via = new String(v);
        }
        if (opts.via != null) {
          opts.via = opts.via.replaceAll("\\s", "");
          if (opts.via.length() == 0)
            opts.via = null;
        }
      }

      if (Params.x509ca.getValue() != null)
        opts.x509ca = new String(Params.x509ca.getValue());
      if (Params.x509crl.getValue() != null)
        opts.x509crl = new String(Params.x509crl.getValue());

      String s = Params.vncServerName.getValue();
      if (s != null) {
        int atIndex = s.lastIndexOf('@');
        if (atIndex >= 0) {
          opts.serverName = s.substring(atIndex + 1);
          opts.sshUser = s.substring(0, atIndex);
        } else {
          opts.serverName = new String(s);
        }
        if (opts.serverName != null) {
          opts.serverName = opts.serverName.replaceAll("\\s", "");
          if (opts.serverName.length() == 0)
            opts.serverName = null;
        }
      }
      Params.vncServerName.setParam(null);

    } catch (Exception e) {
      reportException(new WarningException("Could not set global options:\n" +
                                           e.getMessage()));
      exit(1);
    }

    setGlobalInsets();
  }

  // Is the keyboard grabbed by any TurboVNC Viewer window?
  public static boolean isKeyboardGrabbed() {
    synchronized(VncViewer.class) {
      return grabOwner != null;
    }
  }

  // Is the keyboard grabbed by a specific TurboVNC Viewer window?
  public static boolean isKeyboardGrabbed(Viewport viewport) {
    synchronized(VncViewer.class) {
      return grabOwner == viewport;
    }
  }

  public static void setGrabOwner(Viewport viewport) {
    synchronized(VncViewer.class) {
      grabOwner = viewport;
    }
  }

  Thread thread;
  Socket sock;
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
  static ArrayList<CConn> conns = new ArrayList<CConn>();
  static Insets insets;
  static Viewport grabOwner;
  static boolean noExceptionDialog;
}
