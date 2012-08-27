/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright 2011 Pierre Ossman <ossman@cendio.se> for Cendio AB
 * Copyright (C) 2011-2012 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2011-2012 Brian P. Hinz
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
import java.io.InputStream;
import java.io.IOException;
import java.lang.Character;
import java.util.jar.Attributes;
import java.util.jar.Manifest;
import javax.swing.*;

import com.turbovnc.rdr.*;
import com.turbovnc.rfb.*;
import com.turbovnc.network.*;

public class VncViewer extends java.applet.Applet implements Runnable
{
  public static final String about1 = "TurboVNC Viewer";
  public static final String about2 = "Copyright (C) 2000-2012 "+
                                      "The VirtualGL Project and many others (see README.txt)";
  public static final String about3 = "Visit http://www.virtualgl.org "+
                                      "for more information on TurboVNC.";
  public static String version = null;
  public static String build = null;

  public static final String os = System.getProperty("os.name").toLowerCase();

  public static void setLookAndFeel() {
    try {
      if (os.startsWith("windows")) {
        String laf = "com.sun.java.swing.plaf.windows.WindowsLookAndFeel";
        UIManager.setLookAndFeel(laf);
      } else {
        UIManager.put("swing.boldMetal", Boolean.FALSE);
        javax.swing.plaf.FontUIResource f = new
          javax.swing.plaf.FontUIResource("SansSerif", Font.PLAIN, 11);
        java.util.Enumeration keys = UIManager.getDefaults().keys();
        while (keys.hasMoreElements()) {
          Object key = keys.nextElement();
          Object value = UIManager.get (key);
          if (value instanceof javax.swing.plaf.FontUIResource)
            UIManager.put(key, f);
        }
      }
      UIManager.put("TitledBorder.titleColor",Color.blue);
      if (os.startsWith("mac os x"))
        System.setProperty("apple.laf.useScreenMenuBar", "true");
    } catch (java.lang.Exception e) { 
      vlog.info(e.toString());
    }
  }

  public static void main(String[] argv) {
    setLookAndFeel();
    VncViewer viewer = new VncViewer(argv);
    viewer.start();
  }

  
  public VncViewer(String[] argv) {
    applet = false;
    
    // Override defaults with command-line options
    for (int i = 0; i < argv.length; i++) {
      if (argv[i].length() == 0)
        continue;

      if (argv[i].equalsIgnoreCase("-loglevel")) {
        if (++i >= argv.length) usage();
        System.err.println("Log setting: "+argv[i]);
        LogWriter.setLogParams(argv[i]);
        continue;
      }

      if (Configuration.setParam(argv[i]))
        continue;

      if (argv[i].charAt(0) == '-') {
        if (i+1 < argv.length) {
          if (Configuration.setParam(argv[i].substring(1), argv[i+1])) {
            i++;
            continue;
          }
        }
        usage();
      }

      if (vncServerName.getValue() != null)
        usage();
      vncServerName.setParam(argv[i]);
    }

    if (via.getValue() != null || tunnel.getValue()) {
      if (vncServerName.getValue() == null)
        usage();
      try {
        Tunnel.createTunnel(this);
      } catch (java.lang.Exception e) {
        System.out.println("Could not create SSH tunnel:\n"+e.toString());
        exit(1);
      }
    }

  }

  public static void usage() {
    String usage = ("\n"+
                    "USAGE: vncviewer [options/parameters] [host:displayNum] [options/parameters]\n"+
                    "       vncviewer [options/parameters] [host::port] [options/parameters]\n"+
                    "       vncviewer [options/parameters] -listen [port] [options/parameters]\n"+
                    "\n"+
                    "Options:\n"+
                    "  -loglevel <level>   configure logging level\n"+
                    "                      0 = errors only\n"+
                    "                      10 = status messages\n"+
                    "                      30 = informational messages (default)\n"+
                    "                      100 = debugging messages\n"+
                    "\n"+
                    "Specifying boolean parameters:\n"+
                    "  On:   -<param>=1 or -<param>\n"+
                    "  Off:  -<param>=0 or -no<param>\n"+
                    "Parameters that take a value can be specified as:\n"+
                    "  -<param> <value> or <param>=<value> or -<param>=<value> or --<param>=<value>\n"+
                    "Parameter names and values are case-insensitive.  The parameters are:\n\n");
    System.err.print(usage);
    Configuration.listParams(80);
    // Technically, we shouldn't use System.exit here but if there is a parameter
    // error then the problem is in the index/html file anyway.
    System.exit(1);
  }

  public VncViewer() {
    applet = true;
  }

  public static void newViewer(VncViewer oldViewer, Socket sock, boolean close) {
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
    ClassLoader cl = this.getClass().getClassLoader();
    ImageIcon icon = new ImageIcon(cl.getResource("com/turbovnc/vncviewer/turbovnc.png"));
    logo = icon.getImage();
  }

  public void start() {
    vlog.debug("start called");
    if (version == null || build == null) {
      ClassLoader cl = this.getClass().getClassLoader();
      InputStream stream = cl.getResourceAsStream("com/turbovnc/vncviewer/timestamp");
      try {
        Manifest manifest = new Manifest(stream);
        Attributes attributes = manifest.getMainAttributes();
        version = attributes.getValue("Version");
        build = attributes.getValue("Build");
      } catch (java.io.IOException e) { }
    }
    String host = null;
    if (applet && nViewers == 0) {
      alwaysShowConnectionDialog.setParam(true);
      Configuration.readAppletParams(this);
      host = getCodeBase().getHost();
    }
    else if (!applet)
      host = vncServerName.getValue();
    if (host != null && host.indexOf(':') < 0 &&
        vncServerPort.getValue() > 0) {
      int port = vncServerPort.getValue();
      vncServerName.setParam(host + ((port >= 5900 && port <= 5999)
                                     ? (":"+(port-5900))
                                     : ("::"+port)));
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
    g.drawImage(logo, 0, 0, this);
    int h = logo.getHeight(this)+20;
    g.drawString(about1+" v"+version+" ("+build+")", 0, h);
    h += g.getFontMetrics().getHeight();
    g.drawString(about2, 0, h);
    h += g.getFontMetrics().getHeight();
    g.drawString(about3, 0, h);
  }

  public void run() {
    CConn cc = null;
    int exitStatus = 0;

    if (listenMode.getValue()) {
      int port = 5500;

      if (vncServerName.getValue() != null && 
          Character.isDigit(vncServerName.getValue().charAt(0)))
        port = Integer.parseInt(vncServerName.getValue());
      else if (vncServerPort.getValue() > 0)
        port = vncServerPort.getValue();

      TcpListener listener = null;
      try {
        listener = new TcpListener(null, port);
      } catch (java.lang.Exception e) {
        System.out.println(e.toString());
        exit(1);
      }

      vlog.info("Listening on port "+port);

      while (true) {
        Socket new_sock = listener.accept();
        if (new_sock != null)
          newViewer(this, new_sock);
      }
    }

    try {
      cc = new CConn(this, sock, vncServerName.getValue());
      while (!cc.shuttingDown)
        cc.processMsg();
    } catch (java.lang.Exception e) {
      if (cc == null || !cc.shuttingDown) {
        e.printStackTrace();
        JOptionPane.showMessageDialog(null,
          e.toString(),
          "VNC Viewer : Error",
          JOptionPane.ERROR_MESSAGE);
        exitStatus = 1;
      } else {
        if (!cc.shuttingDown) {
          vlog.info(e.toString());
          exitStatus = 1;
        }
        cc = null;
      }
    }
    exit(exitStatus);
  }

  StringParameter vncServerName
  = new StringParameter("Server",
  "The VNC server to which to connect.  This can be specified in the format "+
  "<host>[:<display number>] or <host>::<port>, where <host> is the host name "+
  "or IP address of the machine on which the VNC server is running, "+
  "<display number> is an optional X display number (default: 0), and <port> "+
  "is a TCP port.", null);

  BoolParameter alwaysShowConnectionDialog
  = new BoolParameter("AlwaysShowConnectionDialog",
  "Always show the \"New TurboVNC Connection\" dialog even if the server has "+
  "been specified in an applet parameter or on the command line.", false);

  IntParameter vncServerPort
  = new IntParameter("Port",
  "The TCP port number on which the VNC server session is listening.  For Unix "+
  "VNC servers, this is typically 5900 + the X display number of the VNC "+
  "session (example: 5901 if connecting to display :1.)  For Windows and Mac "+
  "VNC servers, this is typically 5900.  (default = 5900)\n "+
  "If listen mode is enabled, this parameter specifies the TCP port on which "+
  "the viewer will listen for connections from a VNC server.  (default = 5500)",
  -1);

  BoolParameter listenMode
  = new BoolParameter("Listen",
  "Start the viewer in \"listen mode.\"  The viewer will listen on port 5500 "+
  "(or on the port specified by the Port parameter) for reverse connections "+
  "from a VNC server.  To connect to a listening viewer from the Unix/Linux "+
  "TurboVNC Server, use the vncconnect program.", false);

  BoolParameter shared
  = new BoolParameter("Shared",
  "When connecting, request a shared session.  When the session is shared, "+
  "other users can connect to the session (assuming they have the correct "+
  "authentication credentials) and collaborate with the user who started the "+
  "session.  If this option is disabled and the TurboVNC Server is using "+
  "default settings, then you will only be able to connect to the server if "+
  "no one else is already connected.", true);

  BoolParameter viewOnly
  = new BoolParameter("ViewOnly",
  "Ignore all keyboard and mouse events in the viewer window and do not pass "+
  "these events to the VNC server.", false);

  BoolParameter fullScreen
  = new BoolParameter("FullScreen",
  "Start the viewer in full-screen mode.", false);

  StringParameter menuKey
  = new StringParameter("MenuKey",
  "The key used to display the popup menu", "F8",
  MenuKey.getMenuKeyValueStr());

  StringParameter scalingFactor
  = new StringParameter("Scale",
  "Reduce or enlarge the remote desktop image.  The value is interpreted as a "+
  "scaling factor in percent.  The default value of 100% corresponds to the "+
  "original remote desktop size.  Values below 100 reduce the image size, "+
  "whereas values above 100 enlarge the image proportionally.  If the "+
  "parameter is set to \"Auto\", then automatic scaling is performed. "+
  "Auto-scaling tries to choose a scaling factor in such a way that the whole "+
  "remote desktop will fit on the local screen.  If the parameter is set to "+
  "\"FixedRatio\", then automatic scaling is performed, but the original aspect "+
  "ratio is preserved.", "100", "1-1000, Auto, or FixedRatio");

  StringParameter desktopSize
  = new StringParameter("DesktopSize",
  "If the VNC server supports desktop resizing, attempt to resize the remote "+
  "desktop to the specified size (example: 1920x1200).", null);

  BoolParameter acceptClipboard
  = new BoolParameter("RecvClipboard",
  "Synchronize the local clipboard with the clipboard of the TurboVNC session "+
  "whenever the latter changes.", true);

  BoolParameter sendClipboard
  = new BoolParameter("SendClipboard",
  "Synchronize the TurboVNC session clipboard with the local clipboard "+
  "whenever the latter changes.", true);

  BoolParameter acceptBell
  = new BoolParameter("AcceptBell",
  "Produce a system beep when a \"bell\" event is received from the server.",
  true);

  StringParameter preferredEncoding
  = new StringParameter("Encoding",
  "Preferred encoding type to use.  If the server does not support the "+
  "preferred encoding type, then the next best one will be chosen.  There "+
  "should be no reason to use an encoding type other than Tight when "+
  "connecting to a TurboVNC server, but this option can be useful when "+
  "connecting to other types of VNC servers, such as RealVNC.",
  "Tight", "Tight, ZRLE, Hextile, Raw");

  BoolParameter allowJpeg
  = new BoolParameter("JPEG",
  "Enable the JPEG subencoding type when using Tight encoding.  This causes "+
  "the Tight encoder to use JPEG compression for subrectangles that have a "+
  "high number of unique colors and indexed color subencoding for "+
  "subrectangles that have a low number of unique colors.  If this option is "+
  "disabled, then the Tight encoder will select between indexed color or raw "+
  "subencoding, depending on the size of the subrectangle and its color count.",
  true);

  IntParameter quality
  = new IntParameter("Quality",
  "Specifies the JPEG quality to use when compressing JPEG images with the "+
  "Tight+JPEG encoding methods.  Lower quality values produce grainier JPEG "+
  "images with more noticeable compression artifacts, but lower quality "+
  "values also use less network bandwidth and CPU time.  The default value of "+
  "" + ConnParams.DEFQUAL + " should be perceptually lossless (that is, any image compression "+
  "artifacts it produces should be imperceptible to the human eye under most "+
  "viewing conditions.)", ConnParams.DEFQUAL, 1, 100);

  StringParameter subsampling
  = new StringParameter("Subsampling",
  "When compressing an image using JPEG, the RGB pixels are first converted "+
  "to the YUV colorspace, a colorspace in which each pixel is represented as a "+
  "brightness (Y, or \"luminance\") value and a pair of color (U & V, or "+
  "\"chrominance\") values.  After this colorspace conversion, chrominance "+
  "subsampling can be used to discard some of the chrominance components in "+
  "order to save bandwidth.  1X subsampling retains the chrominance components "+
  "for all pixels, and thus it provides the best image quality but also uses "+
  "the most network bandwidth and CPU time.  2X subsampling retains the "+
  "chrominance components for every other pixel, and 4X subsampling retains "+
  "the chrominance components for every fourth pixel (this is typically "+
  "implemented as 2X subsampling in both X and Y directions.)  Grayscale "+
  "throws out all of the chrominance components, leaving only luminance.  2X "+
  "and 4X subsampling will typically produce noticeable aliasing of lines and "+
  "other sharp features, but with photographic or other \"smooth\" image "+
  "content, it may be difficult to detect any difference between 1X, 2X, and "+
  "4X.", "1X", "1X, 2X, 4X, Gray");

  AliasParameter samp
  = new AliasParameter("Samp",
  "Alias for Subsampling", subsampling);

  IntParameter compressLevel
  = new IntParameter("CompressLevel",
  "When Tight encoding is used, the compression level specifies the amount of "+
  "Zlib compression to apply to subrectangles encoded using the indexed color, "+
  "mono, and raw subencoding types.  If the JPEG subencoding type is enabled, "+
  "then the compression level also defines the \"palette threshold\", or the "+
  "minimum number of colors that a subrectangle must have before it is encoded "+
  "using JPEG.  Higher compression levels have higher palette thresholds and "+
  "thus favor the use of indexed color subencoding, whereas lower compression "+
  "levels favor the use of JPEG.\n "+
  "Compression Level 1 is always the default whenever JPEG is enabled, because "+
  "extensive experimentation has revealed little or no benefit to using higher "+
  "compression levels with most 3D and video workloads.  However, v1.1 and "+
  "later of the TurboVNC Server also supports Compression Level 2 when JPEG is "+
  "enabled.  Compression Level 2 can be shown to reduce the bandwidth of "+
  "certain types of low-color workloads by typically 20-40% (with a "+
  "commensurate increase in CPU usage.)", 1, 0, 9);

  IntParameter colors
  = new IntParameter("Colors",
  "The color depth to use for the viewer's window.  Specifying 8 will use a "+
  "BGR111 pixel format (1 bit for each red, green, and blue component.) "+
  "Specifying 64 will use a BGR222 pixel format, and specifying 256 will use "+
  "an 8-bit indexed color pixel format.  Lowering the color depth can "+
  "significantly reduce bandwidth, particularly when using encoding types "+
  "other than Tight or when using Tight encoding without JPEG.  However, "+
  "colors will not be represented accurately.  The default is to use the "+
  "native color depth of the display on which the viewer is running, which is "+
  "usually true color (8 bits per component.)", -1);

  BoolParameter useLocalCursor
  = new BoolParameter("CursorShape",
  "Normally, TurboVNC and compatible servers will send only changes to the "+
  "remote mouse cursor's shape and position.  This results in the best mouse "+
  "responsiveness.  Disabling this option causes the server to instead draw "+
  "the mouse cursor and send it to the viewer as an image every time the "+
  "cursor moves.  Thus, using a remote cursor can increase network \"chatter\" "+
  "between server and client significantly, which may cause performance "+
  "problems on slow networks.  However, using a remote cursor can be "+
  "advantageous with shared sessions, since it will allow you to see the "+
  "cursor movements of other connected users.", true);

  StringParameter secTypes = SecurityClient.secTypes;

  StringParameter user
  = new StringParameter("User",
  "The user name to use for Unix Login authentication (TurboVNC and TightVNC "+
  "servers) or for Plain and Ident authentication (VeNCrypt-compatible "+
  "servers.)  If connecting to a TightVNC/TurboVNC server, this also forces "+
  "Unix login authentication to be used, if an authentication method that "+
  "supports it is enabled in the VNC server.", null);

  BoolParameter noUnixLogin
  = new BoolParameter("NoUnixLogin",
  "When connecting to TightVNC/TurboVNC servers, this disables the use of "+
  "Unix login authentication.  This is useful if the server is configured to "+
  "prefer an authentication method that supports Unix login authentication and "+
  "you want to override this preference for a particular connection (for "+
  "instance, to use a one-time password.)", false);

  BoolParameter sendLocalUsername
  = new BoolParameter("SendLocalUsername",
  "Send the local username when using user/password authentication schemes "+
  "(Unix login, Plain, Ident) rather than prompting for it.  If connecting to "+
  "a TightVNC/TurboVNC server, this also forces Unix login authentication to "+
  "be used, if an authentication method that supports it is enabled in the VNC "+
  "server.", false);

  StringParameter passwordFile
  = new StringParameter("PasswordFile",
  "Password file from which to read the password for Standard VNC "+
  "authentication.  This is useful if your home directory is shared between "+
  "the client and server machines.", null);

  AliasParameter passwd
  = new AliasParameter("passwd",
  "Alias for PasswordFile", passwordFile);

  StringParameter via
  = new StringParameter("Via",
  "This parameter specifies an SSH server (\"gateway\") through which the VNC "+
  "connection should be tunneled.  Note that when using the \"Via\" parameter, "+
  "the VNC server host should be specified from the point of view of the "+
  "gateway.  For example, specifying Via=gateway_machine Server=localhost:1 "+
  "will connect to display :1 on gateway_machine.  The VNC server must be "+
  "specified on the command line or in the \"Server\" parameter when using the "+
  "Via parameter.", null);

  BoolParameter tunnel
  = new BoolParameter("Tunnel",
  "Same as Via, except that the gateway is assumed to be the same as the VNC "+
  "server host, so you do not need to specify it separately.  The VNC server "+
  "must be specified on the command line or in the \"Server\" parameter when "+
  "using the Tunnel parameter.", false);

  Thread thread;
  Socket sock;
  boolean applet;
  Image logo;
  static int nViewers;
  static LogWriter vlog = new LogWriter("main");
}
