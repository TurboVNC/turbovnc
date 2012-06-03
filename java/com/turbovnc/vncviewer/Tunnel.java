/*
 *  Copyright (C) 2012 Brian P. Hinz.  All Rights Reserved.
 *  Copyright (C) 2012 D. R. Commander.  All Rights Reserved.
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
import java.util.ArrayList;
import java.util.Iterator;

import com.turbovnc.rfb.*;
import com.turbovnc.network.*;

import com.jcraft.jsch.JSch;
import com.jcraft.jsch.Session;

public class Tunnel 
{
  private final static Integer SERVER_PORT_OFFSET = 5900;

  public static void
  createTunnel(VncViewer viewer) throws java.lang.Exception
  {
    int localPort;
    int remotePort;
    String gatewayHost;
    String remoteHost;
  
    localPort = TcpSocket.findFreeTcpPort();
    if (localPort == 0)
      throw new java.lang.Exception("Could not obtain free TCP port");
  
    if (viewer.tunnel.getValue()) {
      gatewayHost = Hostname.getHost(viewer.vncServerName.getValue());
      remoteHost = "localhost";
    } else {
      gatewayHost = viewer.via.getValue();
      remoteHost = Hostname.getHost(viewer.vncServerName.getValue());
    }
    remotePort = Hostname.getPort(viewer.vncServerName.getValue());
  
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
    privateKeys.add(new File(homeDir + "/.ssh/id_rsa"));
    privateKeys.add(new File(homeDir + "/.ssh/id_dsa"));
    for (Iterator i = privateKeys.iterator(); i.hasNext();) {
      File privateKey = (File)i.next();
      if (privateKey.exists() && privateKey.canRead())
        jsch.addIdentity(privateKey.getAbsolutePath());
    }

    // username and passphrase will be given via UserInfo interface.
    vlog.debug("Opening SSH tunnel through gateway " + gatewayHost);
    PasswdDialog dlg = new PasswdDialog(new String("SSH Authentication"),
                                        false, null, false);
    dlg.promptPassword(new String("SSH Authentication"));

    Session session = jsch.getSession(dlg.userEntry.getText(), gatewayHost,
                                      22);
    session.setPassword(new String(dlg.passwdEntry.getPassword()));
    session.connect();
    vlog.debug("Forwarding local port " + localPort + " to " + remoteHost
               + ":" + remotePort + " (relative to gateway)");
    session.setPortForwardingL(localPort, remoteHost, remotePort);
    viewer.vncServerName.setParam("localhost::" + localPort);
  }
  
  static LogWriter vlog = new LogWriter("Tunnel");
}
