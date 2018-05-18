/*  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
 *  Copyright (C) 2000 Const Kaplinsky.  All Rights Reserved.
 *  Copyright (C) 2012-2015, 2017-2018 D. R. Commander.  All Rights Reserved.
 *  Copyright (C) 2012 Brian P. Hinz.  All Rights Reserved.
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 *  USA.
 */

/*
 * Tunnel.java - SSH tunneling support
 */

package com.turbovnc.vncviewer;

import java.io.File;
import java.util.*;

import com.turbovnc.rfb.*;
import com.turbovnc.rdr.*;
import com.turbovnc.network.*;

import com.jcraft.jsch.JSch;
import com.jcraft.jsch.Session;

public final class Tunnel {

  public static void createTunnel(Options opts) throws Exception {
    int localPort;
    int remotePort;
    String gatewayHost;
    String remoteHost;

    localPort = TcpSocket.findFreeTcpPort();
    if (localPort == 0)
      throw new ErrorException("Could not obtain free TCP port");

    if (opts.tunnel) {
      gatewayHost = Hostname.getHost(opts.serverName);
      remoteHost = "localhost";
    } else {
      gatewayHost = opts.via;
      remoteHost = Hostname.getHost(opts.serverName);
    }
    if (opts.serverName != null && opts.serverName.indexOf(':') < 0 &&
        opts.port > 0)
      remotePort = opts.port;
    else
      remotePort = Hostname.getPort(opts.serverName);

    String pattern = null;
    if (opts.tunnel) {
      pattern = System.getProperty("turbovnc.tunnel");
      if (pattern == null)
        pattern = System.getenv("VNC_TUNNEL_CMD");
    } else {
      pattern = System.getProperty("turbovnc.via");
      if (pattern == null)
        pattern = System.getenv("VNC_VIA_CMD");
    }

    if (opts.extSSH || (pattern != null && pattern.length() > 0))
      createTunnelExt(gatewayHost, remoteHost, remotePort, localPort, pattern,
                      opts);
    else
      createTunnelJSch(gatewayHost, remoteHost, remotePort, localPort, opts);
    opts.serverName = "localhost::" + localPort;
  }

  /* Create a tunnel using the builtin JSch SSH client */

  private static void createTunnelJSch(String gatewayHost, String remoteHost,
                                       int remotePort, int localPort,
                                       Options opts) throws Exception {
    JSch jsch = new JSch();
    String homeDir = new String("");
    try {
      homeDir = System.getProperty("user.home");
    } catch (java.security.AccessControlException e) {
      System.err.println("Cannot access user.home system property");
    }

    // NOTE: JSch does not support all ciphers.  User may be prompted to accept
    //       the authenticity of the host key even if the key is in the
    //       known_hosts file.

    File knownHosts = new File(homeDir + "/.ssh/known_hosts");
    if (knownHosts.exists() && knownHosts.canRead())
      jsch.setKnownHosts(knownHosts.getAbsolutePath());
    ArrayList<File> privateKeys = new ArrayList<File>();
    String sshKeyFile = VncViewer.sshKeyFile.getValue();
    String sshKey = VncViewer.sshKey.getValue();
    if (sshKey != null) {
      String sshKeyPass = VncViewer.sshKeyPass.getValue();
      byte[] keyPass = null, key;
      if (sshKeyPass != null)
        keyPass = sshKeyPass.getBytes();
      sshKey = sshKey.replaceAll("\\\\n", "\n");
      key = sshKey.getBytes();
      jsch.addIdentity("TurboVNC", key, null, keyPass);
    } else if (sshKeyFile != null) {
      File f = new File(sshKeyFile);
      if (!f.exists() || !f.canRead())
        throw new ErrorException("Cannot access private SSH key file " +
                                 sshKeyFile);
      privateKeys.add(f);
    } else {
      privateKeys.add(new File(homeDir + "/.ssh/id_rsa"));
      privateKeys.add(new File(homeDir + "/.ssh/id_dsa"));
    }
    for (Iterator<File> i = privateKeys.iterator(); i.hasNext();) {
      File privateKey = (File)i.next();
      if (privateKey.exists() && privateKey.canRead()) {
        if (VncViewer.sshKeyPass.getValue() != null)
          jsch.addIdentity(privateKey.getAbsolutePath(),
                           VncViewer.sshKeyPass.getValue());
        else
          jsch.addIdentity(privateKey.getAbsolutePath());
      }
    }

    // username and passphrase will be given via UserInfo interface.
    vlog.debug("Opening SSH tunnel through gateway " + gatewayHost);
    String user = opts.sshUser;
    if (user == null)
      user = (String)System.getProperties().get("user.name");
    Session session = null;
    if (user != null && jsch.getIdentityNames().size() > 0) {
      session = jsch.getSession(user, gatewayHost,
                                VncViewer.sshPort.getValue());
      try {
        PasswdDialog dlg = new PasswdDialog(new String("SSH Authentication"),
                                            false, user, false);
        session.setUserInfo(dlg);
        session.connect();
      } catch (com.jcraft.jsch.JSchException e) {
        System.err.println("Could not authenticate using SSH private key.  Falling back to user/password.");
        jsch.removeAllIdentity();
        session = null;
      }
    }
    if (session == null) {
      PasswdDialog dlg = new PasswdDialog(new String("SSH Authentication"),
                                          false, user, false);
      dlg.promptPassword(new String("SSH Authentication"));
      session = jsch.getSession(dlg.userEntry.getText(), gatewayHost,
                                VncViewer.sshPort.getValue());
      session.setPassword(new String(dlg.passwdEntry.getPassword()));
      session.setUserInfo(dlg);
      session.connect();
    }
    vlog.debug("Forwarding local port " + localPort + " to " + remoteHost +
               ":" + remotePort + " (relative to gateway)");
    session.setPortForwardingL(localPort, remoteHost, remotePort);
  }

  /* Create a tunnel using an external SSH client.  This supports the same
     VNC_TUNNEL_CMD and VNC_VIA_CMD environment variables as the native viewers
     do. */

  private static final String DEFAULT_SSH_CMD = "/usr/bin/ssh";
  private static final String DEFAULT_TUNNEL_CMD =
    DEFAULT_SSH_CMD + " -f -L %L:localhost:%R %H sleep 20";
  private static final String DEFAULT_VIA_CMD =
    DEFAULT_SSH_CMD + " -f -L %L:%H:%R %G sleep 20";

  public static void createTunnelExt(String gatewayHost, String remoteHost,
                                     int remotePort, int localPort,
                                     String pattern, Options opts)
                                     throws Exception {
    if (pattern == null || pattern.length() < 1)
      pattern = (opts.tunnel ? DEFAULT_TUNNEL_CMD : DEFAULT_VIA_CMD);

    String command = fillCmdPattern(pattern, gatewayHost, remoteHost,
                                    remotePort, localPort, opts);

    vlog.debug("SSH command line: " + command);
    Process p = Runtime.getRuntime().exec(command);
    if (p != null)
      p.waitFor();
  }

  private static String fillCmdPattern(String pattern, String gatewayHost,
                                       String remoteHost, int remotePort,
                                       int localPort, Options opts) {
    int i, j;
    boolean hFound = false, gFound = false, rFound = false, lFound = false;
    String command = "";

    if (opts.sshUser != null)
      gatewayHost = opts.sshUser + "@" + gatewayHost;

    for (i = 0; i < pattern.length(); i++) {
      if (pattern.charAt(i) == '%') {
        switch (pattern.charAt(++i)) {
          case 'H':
            command += (opts.tunnel ? gatewayHost : remoteHost);
            hFound = true;
            continue;
          case 'G':
            command += gatewayHost;
            gFound = true;
            continue;
          case 'R':
            command += remotePort;
            rFound = true;
            continue;
          case 'L':
            command += localPort;
            lFound = true;
            continue;
        }
      }
      command += pattern.charAt(i);
    }

    if (!hFound || !rFound || !lFound)
      throw new ErrorException("%H, %R or %L absent in tunneling command template.");

    if (!opts.tunnel && !gFound)
      throw new ErrorException("%G pattern absent in tunneling command template.");

    return command;
  }

  private Tunnel() {}
  static LogWriter vlog = new LogWriter("Tunnel");
}
