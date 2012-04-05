/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2011-2012 Brian P Hinz
 * Copyright (C) 2012 D. R. Commander.  All Rights Reserved.
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

//import com.jcraft.jsch.*;

public class VncViewer extends java.applet.Applet implements Runnable
{
  public static final String about1 = "TurboVNC Viewer";
  public static final String about2 = "Copyright (C) 2000-2012 "+
                                      "The VirtualGL Project and many others (see README)";
  public static final String about3 = "Visit http://www.virtualgl.org "+
                                      "for more information on TurboVNC.";
  public static String version = null;
  public static String build = null;

  private static int SERVER_PORT_OFFSET = 5900;

  public static void main(String[] argv) {
    try {
      String os = System.getProperty("os.name");
      if (os.startsWith("Windows")) {
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
    } catch (java.lang.Exception exc) { }
    VncViewer viewer = new VncViewer(argv);
    viewer.start();
  }

  
  public VncViewer(String[] argv) {
    applet = false;
    
    // Override defaults with command-line options
    for (int i = 0; i < argv.length; i++) {
      if (argv[i].equalsIgnoreCase("-log")) {
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

  }

  public static void usage() {
    String usage = ("\n"+
                    "USAGE: vncviewer [options/parameters] [host:displayNum] [options/parameters]\n"+
                    "       vncviewer [options/parameters] [host::port] [options/parameters]\n"+
                    "       vncviewer [options/parameters] -listen [port] [options/parameters]\n"+
                    "\n"+
                    "Options:\n"+
                    "  -log <level>    configure logging level\n"+
                    "                  0 = errors only\n"+
                    "                  10 = status messages\n"+
                    "                  30 = informational messages (default)\n"+
                    "                  100 = debugging messages\n"+
                    "\n"+
                    "Parameters can be turned on with -<param> or off with "+
                    "-<param>=0\n"+
                    "Parameters that take a value can be specified as "+
                    "-<param> <value>\n"+
                    "Other valid forms are <param>=<value> -<param>=<value> "+
                    "--<param>=<value>\n"+
                    "Parameter names are case-insensitive.  The parameters "+
                    "are:\n\n"+
                    Configuration.listParams());
    System.err.print(usage);
    System.exit(1);
  }

  /* Tunnelling support. */
//  private void
//  interpretViaParam(String gatewayHost, String remoteHost,
//  		   Integer remotePort, StringParameter vncServerName,
//  		   Integer localPort)
//  {
//    int pos = vncServerName.getValueStr().indexOf(":");
//    if (pos == -1)
//      remotePort = new Integer(SERVER_PORT_OFFSET);
//    else {
//      int portOffset = SERVER_PORT_OFFSET;
//      int len;
//      len =  vncServerName.getValueStr().substring(pos).length();
//      if (vncServerName.getValueStr().substring(pos).startsWith(":")) {
//        /* Two colons is an absolute port number, not an offset. */
//        pos++;
//        len--;
//        portOffset = 0;
//      }
//      try {
//        if (len <= 0)
//          usage();
//        remotePort = 
//          new Integer(vncServerName.getValueStr().substring(pos) + portOffset);
//      } catch (java.lang.NumberFormatException e) {
//        usage();
//      }
//    }
//  
//    if (vncServerName != null)
//      remoteHost = vncServerName.getValueStr();
//  
//    gatewayHost = new String(via.getValueStr());
//    vncServerName.setParam("localhost::"+localPort);
//  }

//  private void
//  createTunnel(String gatewayHost, String remoteHost,
//  	      Integer remotePort, Integer localPort)
//  {
//    //char *cmd = getenv ("VNC_VIA_CMD");
//    //char *percent;
//    //char lport[10], rport[10];
//    //sprintf (lport, "%d", localPort);
//    //sprintf (rport, "%d", remotePort);
//    //setenv ("G", gatewayHost, 1);
//    //setenv ("H", remoteHost, 1);
//    //setenv ("R", rport, 1);
//    //setenv ("L", lport, 1);
//    //if (!cmd)
//      //cmd = "/usr/bin/ssh -f -L \"$L\":\"$H\":\"$R\" \"$G\" sleep 20";
//    try{
//      JSch jsch=new JSch();
//      Session session=jsch.getSession("user", remoteHost, 22);
//      // username and passphrase will be given via UserInfo interface.
//      //UserInfo ui=new MyUserInfo();
//      //session.setUserInfo(ui);
//      session.connect();
//
//      session.setPortForwardingL(localPort, remoteHost, remotePort);
//    } catch (java.lang.Exception e) {
//      System.out.println(e);
//    }
////    /* Compatibility with TigerVNC's method. */
////    while ((percent = strchr (cmd, '%')) != NULL)
////     *percent = '$';
////    system (cmd);
//  }

  public VncViewer() {
    applet = true;
    firstApplet = true;
  }

  public static void newViewer(VncViewer oldViewer) {
    VncViewer viewer = new VncViewer();
    viewer.applet = oldViewer.applet;
    viewer.firstApplet = false;
    viewer.start();
  }


  public void init() {
    vlog.debug("init called");
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
    nViewers++;
    String host = null;
    if (firstApplet) {
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
    thread = new Thread(this);
    thread.start();
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
    Socket sock = null;

    /* Tunnelling support. */
//    if (via.getValueStr() != null) {
//      String gatewayHost = new String("");
//      String remoteHost = new String("localhost");
//      Integer localPort = new Integer(TcpSocket.findFreeTcpPort());
//      Integer remotePort = new Integer(SERVER_PORT_OFFSET);
//      if (vncServerName.getValueStr() == null)
//        usage();
//      interpretViaParam(gatewayHost, remoteHost, remotePort,
//        vncServerName, localPort);
//      createTunnel(gatewayHost, remoteHost, remotePort, localPort);
//    }

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
        System.exit(1);
      }

      vlog.info("Listening on port "+port);

      while (true) {
        sock = listener.accept();
        if (sock != null)
          break;
        //listener.close();
      }
    }

    try {
      cc = new CConn(this, sock, vncServerName.getValue());
      while (true)
        cc.processMsg();
    } catch (EndOfStream e) {
      vlog.info(e.toString());
    } catch (java.lang.Exception e) {
      if (cc != null) cc.deleteWindow();
      if (cc == null || !cc.shuttingDown) {
        e.printStackTrace();
        JOptionPane.showMessageDialog(null,
          e.toString(),
          "VNC Viewer : Error",
          JOptionPane.ERROR_MESSAGE);
      }
    }
    if (cc != null) cc.deleteWindow();
    nViewers--;
    if (!applet && nViewers == 0) {
      System.exit(0);
    }
  }

  StringParameter vncServerName
  = new StringParameter("Server",
  "    The VNC server to which to connect.  This can be specified in the format\n"+
  "    <host>[:<display number>] or <host>::<port>, where <host> is the host name\n"+
  "    or IP address of the machine on which the VNC server is running,\n"+
  "    <display number> is an optional X display number (default: 0), and <port>\n"+
  "    is a TCP port.", null);

  BoolParameter alwaysShowConnectionDialog
  = new BoolParameter("AlwaysShowConnectionDialog",
  "    Always show the \"New TurboVNC Connection\" dialog even if the server has\n"+
  "    been specified in an applet parameter or on the command line.", false);

  IntParameter vncServerPort
  = new IntParameter("Port",
  "    The TCP port number on which the VNC server session is listening.  For Unix\n"+
  "    VNC servers, this is typically 5900 + the X display number of the VNC\n"+
  "    session (example: 5901 if connecting to display :1.)  For Windows and Mac\n"+
  "    VNC servers, this is typically 5900.  (default = 5900)\n\n"+
  "    If listen mode is enabled, this parameter specifies the TCP port on which\n"+
  "    the viewer will listen for connections from a VNC server.  (default = 5500)",
  -1);

  BoolParameter listenMode
  = new BoolParameter("Listen",
  "    Start the viewer in \"listen mode.\"  The viewer will listen on port 5500\n"+
  "    (or on the port specified by the Port parameter) for reverse connections\n"+
  "    from a VNC server.  To connect to a listening viewer from the Unix/Linux\n"+
  "    TurboVNC Server, use the vncconnect program.", false);

  BoolParameter shared
  = new BoolParameter("Shared",
  "    When connecting, request a shared session.  When the session is shared,\n"+
  "    other users can connect to the session (assuming they have the correct\n"+
  "    authentication credentials) and collaborate with the user who started the\n"+
  "    session.  If this option is disabled and the TurboVNC Server is using\n"+
  "    default settings, then you will only be able to connect to the server if\n"+
  "    no one else is already connected.", true);

  BoolParameter viewOnly
  = new BoolParameter("ViewOnly",
  "    Ignore all keyboard and mouse events in the viewer window and do not pass\n"+
  "    these events to the VNC server.", false);

  BoolParameter fullScreen
  = new BoolParameter("FullScreen",
  "    Start the viewer in full-screen mode.", false);

  StringParameter scalingFactor
  = new StringParameter("ScalingFactor",
  "    Reduce or enlarge the remote desktop image.  The value is interpreted as a\n"+
  "    scaling factor in percent.  The default value of 100% corresponds to the\n"+
  "    original remote desktop size.  Values below 100 reduce the image size,\n"+
  "    whereas values above 100 enlarge the image proportionally.  If the\n"+
  "    parameter is set to \"Auto\", then automatic scaling is performed.\n"+
  "    Auto-scaling tries to choose a scaling factor in such a way that the whole\n"+
  "    remote desktop will fit on the local screen.  If the parameter is set to\n"+
  "    \"FixedRatio\", then automatic scaling is performed, but the original aspect\n"+
  "    ratio is preserved.", "100", "1-1000, Auto, or FixedRatio");

  StringParameter desktopSize
  = new StringParameter("DesktopSize",
  "    If the VNC server supports desktop resizing, attempt to resize the remote\n"+
  "    desktop to the specified size (example: 1920x1200).", null);

  BoolParameter acceptClipboard
  = new BoolParameter("RecvClipboard",
  "    Synchronize the local clipboard with the clipboard of the TurboVNC session\n"+
  "    whenever the latter changes.", true);

  BoolParameter sendClipboard
  = new BoolParameter("SendClipboard",
  "    Synchronize the TurboVNC session clipboard with the local clipboard\n"+
  "    whenever the latter changes.", true);

  BoolParameter acceptBell
  = new BoolParameter("AcceptBell",
  "    Produce a system beep when a \"bell\" event is received from the server.",
  true);

  StringParameter preferredEncoding
  = new StringParameter("PreferredEncoding",
  "    Preferred encoding type to use.  If the server does not support the\n"+
  "    preferred encoding type, then the next best one will be chosen.  There\n"+
  "    should be no reason to use an encoding type other than Tight when\n"+
  "    connecting to a TurboVNC server, but this option can be useful when\n"+
  "    connecting to other types of VNC servers, such as RealVNC.",
  "Tight", "Tight, ZRLE, Hextile, Raw");

  BoolParameter allowJpeg
  = new BoolParameter("AllowJPEG",
  "    Enable the JPEG subencoding type when using Tight encoding.  This causes\n"+
  "    the Tight encoder to use JPEG compression for subrectangles that have a\n"+
  "    high number of unique colors and indexed color subencoding for\n"+
  "    subrectangles that have a low number of unique colors.  If this option is\n"+
  "    disabled, then the Tight encoder will select between indexed color or raw\n"+
  "    subencoding, depending on the size of the subrectangle and its color count.",
  true);

  IntParameter quality
  = new IntParameter("Quality",
  "    Specifies the JPEG quality to use when compressing JPEG images with the\n"+
  "    Tight+JPEG encoding methods.  Lower quality values produce grainier JPEG\n"+
  "    images with more noticeable compression artifacts, but lower quality\n"+
  "    values also use less network bandwidth and CPU time.  The default value of\n"+
  "    " + ConnParams.DEFQUAL + " should be perceptually lossless (that is, any image compression\n"+
  "    artifacts it produces should be imperceptible to the human eye under most\n"+
  "    viewing conditions.)", ConnParams.DEFQUAL, 1, 100);

  StringParameter subsampling
  = new StringParameter("Subsampling",
  "    When compressing an image using JPEG, the RGB pixels are first converted\n"+
  "    to the YUV colorspace, a colorspace in which each pixel is represented as a\n"+
  "    brightness (Y, or \"luminance\") value and a pair of color (U & V, or\n"+
  "    \"chrominance\") values.  After this colorspace conversion, chrominance\n"+
  "    subsampling can be used to discard some of the chrominance components in\n"+
  "    order to save bandwidth.  1X subsampling retains the chrominance components\n"+
  "    for all pixels, and thus it provides the best image quality but also uses\n"+
  "    the most network bandwidth and CPU time.  2X subsampling retains the\n"+
  "    chrominance components for every other pixel, and 4X subsampling retains\n"+
  "    the chrominance components for every fourth pixel (this is typically\n"+
  "    implemented as 2X subsampling in both X and Y directions.)  Grayscale\n"+
  "    throws out all of the chrominance components, leaving only luminance.  2X\n"+
  "    and 4X subsampling will typically produce noticeable aliasing of lines and\n"+
  "    other sharp features, but with photographic or other \"smooth\" image\n"+
  "    content, it may be difficult to detect any difference between 1X, 2X, and\n"+
  "    4X.", "1X", "1X, 2X, 4X, Gray");

  IntParameter compressLevel
  = new IntParameter("CompressLevel",
  "    When Tight encoding is used, the compression level specifies the amount of\n"+
  "    Zlib compression to apply to subrectangles encoded using the indexed color,\n"+
  "    mono, and raw subencoding types.  If the JPEG subencoding type is enabled,\n"+
  "    then the compression level also defines the \"palette threshold\", or the\n"+
  "    minimum number of colors that a subrectangle must have before it is encoded\n"+
  "    using JPEG.  Higher compression levels have higher palette thresholds and\n"+
  "    thus favor the use of indexed color subencoding, whereas lower compression\n"+
  "    levels favor the use of JPEG.\n\n"+
  "    Compression Level 1 is always the default whenever JPEG is enabled, because\n"+
  "    extensive experimentation has revealed little or no benefit to using higher\n"+
  "    compression levels with most 3D and video workloads.  However, v1.1 and\n"+
  "    later of the TurboVNC Server also supports Compression Level 2 when JPEG is\n"+
  "    enabled.  Compression Level 2 can be shown to reduce the bandwidth of\n"+
  "    certain types of low-color workloads by typically 20-40% (with a\n"+
  "    commensurate increase in CPU usage.)", 1, 0, 9);

  IntParameter colors
  = new IntParameter("Colors",
  "    The color depth to use for the viewer's window.  Specifying 8 will use a\n"+
  "    BGR111 pixel format (1 bit for each red, green, and blue component.)\n"+
  "    Specifying 64 will use a BGR222 pixel format, and specifying 256 will use\n"+
  "    an 8-bit indexed color pixel format.  Lowering the color depth can\n"+
  "    significantly reduce bandwidth, particularly when using encoding types\n"+
  "    other than Tight or when using Tight encoding without JPEG.  However,\n"+
  "    colors will not be represented accurately.  The default is to use the\n"+
  "    native color depth of the display on which the viewer is running, which is\n"+
  "    usually true color (8 bits per component.)", -1);

  BoolParameter fastCopyRect
  = new BoolParameter("FastCopyRect",
  "    Use fast CopyRect.  Turn this off if you get screen corruption when\n"+
  "    copying from off-screen.", true);

  BoolParameter useLocalCursor
  = new BoolParameter("UseLocalCursor",
  "    Normally, TurboVNC and compatible servers will send only changes to the\n"+
  "    remote mouse cursor's shape and position.  This results in the best mouse\n"+
  "    responsiveness.  Disabling this option causes the server to instead draw\n"+
  "    the mouse cursor and send it to the viewer as an image every time the\n"+
  "    cursor moves.  Thus, using a remote cursor can increase network \"chatter\"\n"+
  "    between server and client significantly, which may cause performance\n"+
  "    problems on slow networks.  However, using a remote cursor can be\n"+
  "    advantageous with shared sessions, since it will allow you to see the\n"+
  "    cursor movements of other connected users.", true);

  StringParameter secTypes = SecurityClient.secTypes;

  StringParameter user
  = new StringParameter("User",
  "    The user name to use for Unix Login authentication (TurboVNC and TightVNC\n"+
  "    servers) or for Plain and Ident authentication (VeNCrypt-compatible\n"+
  "    servers.)  If connecting to a TightVNC/TurboVNC server, this also forces\n"+
  "    Unix login authentication to be used, if an authentication method that\n"+
  "    supports it is enabled in the VNC server.", null);

  BoolParameter noUnixLogin
  = new BoolParameter("NoUnixLogin",
  "    When connecting to TightVNC/TurboVNC servers, this disables the use of\n"+
  "    Unix login authentication.  This is useful if the server is configured to\n"+
  "    prefer an authentication method that supports Unix login authentication and\n"+
  "    you want to override this preference for a particular connection (for\n"+
  "    instance, to use a one-time password.)", false);

  BoolParameter sendLocalUsername
  = new BoolParameter("SendLocalUsername",
  "    Send the local username when using user/password authentication schemes\n"+
  "    (Unix login, Plain, Ident) rather than prompting for it.  If connecting to\n"+
  "    a TightVNC/TurboVNC server, this also forces Unix login authentication to\n"+
  "    be used, if an authentication method that supports it is enabled in the VNC\n"+
  "    server.", false);

  StringParameter passwordFile
  = new StringParameter("PasswordFile",
  "    Password file from which to read the password for Standard VNC\n"+
  "    authentication.  This is useful if your home directory is shared between\n"+
  "    the client and server machines.", null);

  AliasParameter passwd
  = new AliasParameter("passwd",
  "    Alias for PasswordFile", passwordFile);

//  StringParameter via
//  = new StringParameter("via", "Gateway to tunnel via", null);

  Thread thread;
  boolean applet, firstApplet;
  Image logo;
  static int nViewers;
  static LogWriter vlog = new LogWriter("main");
}
