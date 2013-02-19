/*
 *  Copyright (C) 2012 Brian P. Hinz.  All Rights Reserved.
 *  Copyright (C) 2012-2013 D. R. Commander.  All Rights Reserved.
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

public class Tunnel {

  private static final Integer SERVER_PORT_OFFSET = 5900;

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
    remotePort = Hostname.getPort(opts.serverName);

    JSch jsch = new JSch();
    String homeDir = new String("");
    try {
      homeDir = System.getProperty("user.home");
    } catch(java.security.AccessControlException e) {
      System.out.println("Cannot access user.home system property");
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
        session.connect();
      } catch(com.jcraft.jsch.JSchException e) {
        System.out.println("Could not authenticate using SSH private key.  Falling back to user/password.");
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
      session.connect();
    }
    vlog.debug("Forwarding local port " + localPort + " to " + remoteHost +
               ":" + remotePort + " (relative to gateway)");
    session.setPortForwardingL(localPort, remoteHost, remotePort);
    opts.serverName = "localhost::" + localPort;
  }

  static LogWriter vlog = new LogWriter("Tunnel");
}
