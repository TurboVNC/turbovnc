/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright 2011 Pierre Ossman <ossman@cendio.se> for Cendio AB
 * Copyright (C) 2011-2013 D. R. Commander.  All Rights Reserved.
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

//
// VncViewer - the VNC viewer applet.  It can also be run from the
// command-line, when it behaves as much as possibly like the windows and unix
// viewers.
//
// Unfortunately, because of the way Java classes are loaded on demand, only
// configuration parameters defined in this file can be set from the command
// line or in applet parameters.

package com.turbovnc.vncviewer;

import java.awt.*;
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

public class VncViewer extends java.applet.Applet implements Runnable {
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
  public static final Image logoImage = logoIcon.getImage();

  void setVersion() {
    if (version == null || build == null || copyrightYear == null ||
        copyright == null) {
      ClassLoader cl = getClass().getClassLoader();
      InputStream stream =
        cl.getResourceAsStream("com/turbovnc/vncviewer/timestamp");
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
      } catch(IOException e) {}
    }
  }

  static final double getTime() {
    return (double)System.nanoTime() / 1.0e9;
  }

  public static final String os = System.getProperty("os.name").toLowerCase();

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
              } catch(Exception e) {
                viewer.reportException(e);
                return null;
              }
              viewer.setGlobalOptions();
              viewer.start();
            }
          }
        }
      } catch(Exception e) {
        vlog.info(e.getMessage());
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
      }
    } catch(Exception e) {
      vlog.info(e.getMessage());
    }
  }

  public static void main(String[] argv) {
    setLookAndFeel();
    VncViewer viewer = new VncViewer(argv);
    if (os.startsWith("mac os x")) {
      synchronized(VncViewer.class) {
        if (fileName != null) {
          try {
            Configuration.load(fileName);
          } catch(Exception e) {
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

    // load user preferences
    UserPreferences.load("global");

    setVersion();

    // Override defaults with command-line options
    for (int i = 0; i < argv.length; i++) {
      if (argv[i].length() == 0)
        continue;

      if (argv[i].equalsIgnoreCase("-config")) {
        if (++i >= argv.length) usage();
        try {
          Configuration.load(argv[i]);
        } catch(Exception e) {
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
          } catch(Exception e) {
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
        } catch(Exception e) {
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

    if (opts.via != null || opts.tunnel) {
      if (opts.serverName == null)
        usage();
      try {
        Tunnel.createTunnel(opts);
      } catch(Exception e) {
        reportException(new WarningException("Could not create SSH tunnel:\n" +
                                             e.getMessage()));
        exit(1);
      }
    }

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
    // Technically, we shouldn't use System.exit here but if there is a parameter
    // error then the problem is in the index/html file anyway.
    System.exit(1);
  }

  public VncViewer() {
    applet = true;
    UserPreferences.load("global");
    setVersion();
    setGlobalOptions();
  }

  public static void newViewer(VncViewer oldViewer, Socket sock,
                               boolean close) {
    VncViewer viewer = new VncViewer();
    viewer.applet = oldViewer.applet;
    viewer.sock = sock;
    viewer.start();
    if (close)
      oldViewer.exit(0);
  }

  public static void newViewer(VncViewer oldViewer, Socket sock) {
    newViewer(oldViewer, sock, false);
  }

  public static void newViewer(VncViewer oldViewer) {
    newViewer(oldViewer, null);
  }

  public void init() {
    vlog.debug("init called");
    setLookAndFeel();
    setBackground(Color.white);
  }

  public void start() {
    vlog.debug("start called");
    String host = null;
    if (applet && nViewers == 0) {
      alwaysShowConnectionDialog.setParam(true);
      Configuration.readAppletParams(this);
      String str = getParameter("LogLevel");
      if (str != null)
        LogWriter.setLogParams(str);
      setGlobalOptions();
      host = opts.serverName;
      if ((opts.via != null || opts.tunnel) && opts.serverName != null) {
        try {
          Tunnel.createTunnel(opts);
        } catch(Exception e) {
          reportException(new WarningException("Could not create SSH tunnel:\n" +
                                               e.getMessage()));
          exit(1);
          return;
        }
        host = null;
      }
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
    nViewers--;
    if (nViewers > 0)
      return;
    if (applet) {
      destroy();
    } else {
      System.exit(n);
    }
  }

  public void paint(Graphics g) {
    g.drawImage(logoImage, 0, 0, this);
    int h = logoImage.getHeight(this) + 20;
    g.drawString(PRODUCT_NAME + " v" + version + " (" + build + ")", 0, h);
    h += g.getFontMetrics().getHeight();
    g.drawString("Copyright (C) " + copyrightYear + " " + copyright, 0, h);
    h += g.getFontMetrics().getHeight();
    g.drawString(url, 0, h);
  }

  void reportException(Exception e) {
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
    JOptionPane.showMessageDialog(null, msg, title, msgType);
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

      TcpListener listener = null;
      try {
        listener = new TcpListener(null, port);
      } catch(Exception e) {
        reportException(e);
        exit(1);
      }

      vlog.info("Listening on port " + port);

      while (true) {
        Socket newSock = listener.accept();
        if (newSock != null)
          newViewer(this, newSock);
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
          } catch(EndOfStream e) {}
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
      } catch(Exception e) {
        if (cc == null || !cc.shuttingDown) {
          reportException(e);
          exitStatus = 1;
          if (cc != null) cc.deleteWindow();
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
    if (opts == null)
      opts = new Options();

    opts.port = vncServerPort.getValue();
    vncServerPort.setValue(-1);

    opts.shared = shared.getValue();
    opts.viewOnly = viewOnly.getValue();
    opts.fullScreen = fullScreen.getValue();

    if (span.getValue().toLowerCase().startsWith("p"))
      opts.span = Options.SPAN_PRIMARY;
    else if (span.getValue().toLowerCase().startsWith("al"))
      opts.span = Options.SPAN_ALL;
    else
      opts.span = Options.SPAN_AUTO;

    opts.scalingFactor = Integer.parseInt(scalingFactor.getDefaultStr());
    opts.setScalingFactor(scalingFactor.getValue());

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
      case 8:  case 64:  case 256:  case 65536:
        opts.colors = colors.getValue();
        break;
    }

    opts.cursorShape = cursorShape.getValue();
    opts.continuousUpdates = continuousUpdates.getValue();
    if (user.getValue() != null) opts.user = new String(user.getValue());
    opts.noUnixLogin = noUnixLogin.getValue();
    opts.sendLocalUsername = sendLocalUsername.getValue();

    String v = via.getValue();
    if (v != null) {
      int atIndex = v.indexOf('@');
      if (atIndex >= 0) {
        opts.via = v.substring(atIndex + 1);
        opts.sshUser = v.substring(0, atIndex);
      } else {
        opts.via = new String(v);
      }
    }
    opts.tunnel = tunnel.getValue();
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
  "to 1 if the viewer is being run as an applet.", false);

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
  "cannot fit on the primary monitor (Auto.)", "Auto",
  "Primary, All, Auto");

  // On Mac systems, setting this parameter will force the use of the old
  // (pre-Lion) full-screen mode, even if the viewer is running on OS X 10.7
  // "Lion" or later.
  static BoolParameter noLionFS
  = new BoolParameter("NoLionFS", null, false);

  static BoolParameter showToolbar
  = new BoolParameter("Toolbar",
  "Show the toolbar by default.", true);

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
  "Auto-scaling tries to choose a scaling factor in such a way that the whole " +
  "remote desktop will fit on the local screen.  If the parameter is set to " +
  "\"FixedRatio\", then automatic scaling is performed, but the original aspect " +
  "ratio is preserved.", "100", "1-1000, Auto, or FixedRatio");

  static StringParameter desktopSize
  = new StringParameter("DesktopSize",
  "If the VNC server supports desktop resizing, attempt to resize the remote " +
  "desktop to the specified size (example: 1920x1200).", null);

  static BoolParameter acceptClipboard
  = new BoolParameter("RecvClipboard",
  "Synchronize the local clipboard with the clipboard of the TurboVNC session " +
  "whenever the latter changes.", true);

  static BoolParameter sendClipboard
  = new BoolParameter("SendClipboard",
  "Synchronize the TurboVNC session clipboard with the local clipboard " +
  "whenever the latter changes.", true);

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
  "Compression Level 1 is always the default whenever JPEG is enabled, because " +
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

  static IntParameter colors
  = new IntParameter("Colors",
  "The color depth to use for the viewer's window.  Specifying 8 will use a " +
  "BGR111 pixel format (1 bit for each red, green, and blue component.) " +
  "Specifying 64 will use a BGR222 pixel format, specifying 256 will use a " +
  "BGR233 pixel format, and specifying 65536 will use a BGR565 pixel format. " +
  "Lowering the color depth can significantly reduce bandwidth when using " +
  "encoding types other than Tight or when using Tight encoding without JPEG. " +
  "However, colors will not be represented accurately, and CPU usage will " +
  "increase substantially (causing a corresponding decrease in performance on " +
  "fast networks.)  The default is to use the native color depth of the display " +
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

  static StringParameter secTypes = SecurityClient.secTypes;

  static StringParameter user
  = new StringParameter("User",
  "The user name to use for Unix Login authentication (TurboVNC and TightVNC " +
  "servers) or for Plain and Ident authentication (VeNCrypt-compatible " +
  "servers.)  If connecting to a TightVNC/TurboVNC server, this also forces " +
  "Unix login authentication to be used, if an authentication method that " +
  "supports it is enabled in the VNC server.", null);

  static BoolParameter noUnixLogin
  = new BoolParameter("NoUnixLogin",
  "When connecting to TightVNC/TurboVNC servers, this disables the use of " +
  "Unix login authentication.  This is useful if the server is configured to " +
  "prefer an authentication method that supports Unix login authentication and " +
  "you want to override this preference for a particular connection (for " +
  "instance, to use a one-time password.)", false);

  static BoolParameter sendLocalUsername
  = new BoolParameter("SendLocalUsername",
  "Send the local user name when using user/password authentication schemes " +
  "(Unix login, Plain, Ident) rather than prompting for it.  If connecting to " +
  "a TightVNC/TurboVNC server, this also forces Unix login authentication to " +
  "be used, if an authentication method that supports it is enabled in the VNC " +
  "server.", false);

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
  "This parameter specifies an SSH server (\"gateway\") through which the VNC " +
  "connection should be tunneled.  Note that when using the Via parameter, " +
  "the VNC server host should be specified from the point of view of the " +
  "gateway.  For example, specifying Via=gateway_machine Server=localhost:1 " +
  "will connect to display :1 on gateway_machine.  The VNC server must be " +
  "specified on the command line or in the Server parameter when using the " +
  "Via parameter.  The Via parameter can be prefixed by <user>@ to indicate " +
  "that user name <user> (default = local user name) should be used when " +
  "authenticating with the SSH server.", null);

  static BoolParameter tunnel
  = new BoolParameter("Tunnel",
  "Same as Via, except that the gateway is assumed to be the same as the VNC " +
  "server host, so you do not need to specify it separately.  The VNC server " +
  "must be specified on the command line or in the Server parameter when " +
  "using the Tunnel parameter.  When using the Tunnel parameter, the VNC server " +
  "host can be prefixed by <user>@ to indicate that user name <user> " +
  "(default = local user name) should be used when authenticating with the SSH " +
  "server.", false);

  static IntParameter sshPort
  = new IntParameter("SSHPort",
  "When using the Via or Tunnel options, this parameter specifies the TCP " +
  "port on which the SSH server is listening.", 22);

  static StringParameter sshKey
  = new StringParameter("SSHKey",
  "When using the Via or Tunnel options, this parameter specifies the text " +
  "of the SSH private key to use when authenticating with the SSH server.  " +
  "You can use \\n within the string to specify a new line.", null);

  static StringParameter sshKeyFile
  = new StringParameter("SSHKeyFile",
  "When using the Via or Tunnel options, this parameter specifies a file " +
  "that contains an SSH private key (or keys) to use when authenticating " +
  "with the SSH server.  If not specified, then the built-in SSH client will " +
  "attempt to read private keys from ~/.ssh/id_dsa and ~/.ssh/id_rsa.  It " +
  "will fall back to asking for an SSH password if private key " +
  "authentication fails.", null);

  static StringParameter sshKeyPass
  = new StringParameter("SSHKeyPass",
  "When using the Via or Tunnel options, this parameter specifies the " +
  "passphrase for the SSH key.", null);

  static StringParameter config
  = new StringParameter("Config",
  "File from which to read connection information.  This file can be generated " +
  "by selecting \"Save connection info as...\" in the system menu of the Windows " +
  "TurboVNC Viewer.", null);

  Thread thread;
  Socket sock;
  boolean applet;
  static int nViewers;
  static LogWriter vlog = new LogWriter("main");
  FileInStream benchFile;
  int benchIter = 1;
  int benchWarmup = 0;
  Options opts;
}
