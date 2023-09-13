/* Copyright (C) 2012-2015, 2017-2018, 2020-2023 D. R. Commander.
 *                                               All Rights Reserved.
 * Copyright (C) 2021 Steffen Kieß
 * Copyright (C) 2012, 2016 Brian P. Hinz.  All Rights Reserved.
 * Copyright (C) 2000 Const Kaplinsky.  All Rights Reserved.
 * Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
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

/*
 * Tunnel.java - SSH tunneling support
 */

package com.turbovnc.vncviewer;

import java.io.*;
import java.util.*;

import com.turbovnc.rfb.*;
import com.turbovnc.rdr.*;
import com.turbovnc.network.*;

import com.jcraft.jsch.agentproxy.*;
import com.jcraft.jsch.agentproxy.connector.*;
import com.jcraft.jsch.agentproxy.usocket.*;
import com.jcraft.jsch.Identity;
import com.jcraft.jsch.*;

public class Tunnel {

  public static void createTunnel(Params params) throws Exception {
    int localPort;
    int remotePort;
    String gatewayHost;
    String remoteHost;

    boolean tunnel = params.tunnel.get() ||
                     (params.sessMgrActive && params.sessMgrAuto.get());

    if (tunnel) {
      gatewayHost = Hostname.getHost(params.server.get());
      remoteHost = "localhost";
    } else {
      gatewayHost = params.via.get();
      remoteHost = Hostname.getHost(params.server.get());
    }
    if (params.server.get() != null && params.server.get().indexOf(':') < 0 &&
        params.port.get() > 0)
      remotePort = params.port.get();
    else
      remotePort = Hostname.getPort(params.server.get());

    String pattern = null;
    if (tunnel) {
      pattern = System.getProperty("turbovnc.tunnel");
      if (pattern == null)
        pattern = System.getenv("VNC_TUNNEL_CMD");
    } else {
      pattern = System.getProperty("turbovnc.via");
      if (pattern == null)
        pattern = System.getenv("VNC_VIA_CMD");
    }

    if (params.udsPath != null &&
        (pattern == null || pattern.indexOf("%L") < 0)) {
      // Connect to Unix domain socket using stdio-based forwarding
      params.stdioSocket = createTunnelExtUDS(gatewayHost, remoteHost, pattern,
                                              params, tunnel);
    } else {
      localPort = TcpSocket.findFreeTcpPort();
      if (localPort == 0)
        throw new ErrorException("Could not obtain free TCP port");

      if (params.extSSH.get() || (pattern != null && pattern.length() > 0))
        createTunnelExt(gatewayHost, remoteHost, remotePort, localPort,
                        pattern, params, tunnel);
      else {
        vlog.debug("Opening SSH tunnel through gateway " + gatewayHost);
        if (params.sshSession == null)
          createTunnelJSch(gatewayHost, params);
        vlog.debug("Forwarding local port " + localPort + " to " + remoteHost +
                   ":" + remotePort + " (relative to gateway)");
        params.sshSession.setPortForwardingL(localPort, remoteHost,
                                             remotePort);
      }
      params.server.set("localhost::" + localPort);
    }
    params.sshTunnelActive = true;
  }

  /* Create a tunnel using the builtin JSch SSH client */

  protected static void createTunnelJSch(String host, Params params)
                                         throws Exception {
    JSch jsch = new JSch();
    JSch.setLogger(LOGGER);
    String homeDir = new String("");
    try {
      homeDir = System.getProperty("user.home");
    } catch (Exception e) {
      System.err.println("Cannot access user.home system property");
    }

    // NOTE: JSch does not support all ciphers.  User may be prompted to accept
    //       the authenticity of the host key even if the key is in the
    //       known_hosts file.

    File knownHosts = new File(homeDir + "/.ssh/known_hosts");
    jsch.setKnownHosts(knownHosts.getAbsolutePath());

    if (Helper.isAvailable()) {
      Connector connector = null;
      try {
        if (Utils.isWindows())
          connector = new PageantConnector();
        else
          connector = new SSHAgentConnector(new JNIUSocketFactory());
        if (connector != null) {
          IdentityRepository repo = new RemoteIdentityRepository(connector);
          vlog.sshdebug("SSH private keys offered by agent:");
          Iterator<Identity> iter = repo.getIdentities().iterator();
          while (iter.hasNext()) {
            Identity id = iter.next();
            vlog.sshdebug("  " + id.getName());
            if (id.getFingerPrint() != null)
              vlog.sshdebug("    Fingerprint: " + id.getFingerPrint());
          }
          LocalIdentityRepository localRepo =
            (LocalIdentityRepository)jsch.getIdentityRepository();
          localRepo.copyFrom(repo);
        }
      } catch (Exception e) {
        if (Utils.isWindows())
          vlog.debug("Could not contact Pageant:\n        " + e.getMessage());
        else
          vlog.debug("Could not contact ssh-agent:\n        " +
                     e.getMessage());
      }
    }

    ArrayList<File> privateKeys = new ArrayList<File>();
    String sshKeyFile = params.sshKeyFile.get();
    String sshKey = params.sshKey.get();
    boolean useDefaultPrivateKeyFiles = false;
    if (sshKey != null) {
      String sshKeyPass = params.sshKeyPass.get();
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
      useDefaultPrivateKeyFiles = true;
    }

    // username and passphrase will be given via UserInfo interface.
    int port = params.sshPort.get();
    String user = params.sshUser.get();

    File sshConfigFile = new File(params.sshConfig.get());
    if (sshConfigFile.exists() && sshConfigFile.canRead()) {
      ConfigRepository repo =
        OpenSSHConfig.parseFile(sshConfigFile.getAbsolutePath());
      jsch.setConfigRepository(repo);
      vlog.debug("Read OpenSSH config file " + params.sshConfig.get());
      // This just ensures that the password dialog displays the correct
      // username.  JSch will ignore the username and port passed to
      // getSession() if the configuration has already been set using an
      // OpenSSH configuration file.
      String repoUser = repo.getConfig(host).getUser();
      if (repoUser != null && user == null)
        user = repoUser;
      String[] identityFiles = repo.getConfig(host).getValues("IdentityFile");
      if (identityFiles != null) {
        for (String file : identityFiles) {
          if (file != null && !file.isEmpty())
            useDefaultPrivateKeyFiles = false;
        }
      }
    } else {
      if (params.sshConfig.isDefault()) {
        vlog.debug("Could not parse SSH config file " +
                   params.sshConfig.get());
      } else {
        vlog.info("Could not parse SSH config file " + params.sshConfig.get());
      }
    }

    if (user == null) {
      user = (String)System.getProperties().get("user.name");
      if (params.localUsernameLC.get())
        user = user.toLowerCase();
    }

    if (useDefaultPrivateKeyFiles) {
      privateKeys.add(new File(homeDir + "/.ssh/id_rsa"));
      privateKeys.add(new File(homeDir + "/.ssh/id_dsa"));
      privateKeys.add(new File(homeDir + "/.ssh/id_ecdsa"));
    }

    for (Iterator<File> i = privateKeys.iterator(); i.hasNext();) {
      File privateKey = (File)i.next();
      try {
        if (privateKey.exists() && privateKey.canRead()) {
          if (params.sshKeyPass.get() != null)
            jsch.addIdentity(privateKey.getAbsolutePath(),
                             params.sshKeyPass.get());
          else
            jsch.addIdentity(privateKey.getAbsolutePath());
        }
      } catch (Exception e) {
        throw new ErrorException("Could not use SSH private key " +
                                 privateKey.getAbsolutePath() + ":\n" +
                                 e.getMessage());
      }
    }

    params.sshSession = jsch.getSession(user, host, port);

    vlog.sshdebug("Attempting to use the following SSH private keys:");
    Iterator<Identity> iter =
      params.sshSession.getIdentityRepository().getIdentities().iterator();
    while (iter.hasNext()) {
      Identity id = iter.next();
      vlog.sshdebug("  " + id.getName());
      if (id.getFingerPrint() != null)
        vlog.sshdebug("    Fingerprint: " + id.getFingerPrint());
    }

    // OpenSSHConfig doesn't recognize StrictHostKeyChecking
    if (params.sshSession.getConfig("StrictHostKeyChecking") == null)
      params.sshSession.setConfig("StrictHostKeyChecking", "ask");
    params.sshSession.setConfig("MaxAuthTries", "3");
    String auth = System.getProperty("turbovnc.sshauth");
    if (auth != null)
      params.sshSession.setConfig("PreferredAuthentications", auth);
    PasswdDialog dlg = new PasswdDialog(new String("SSH Authentication"),
                                        true, user, false, true, -1);
    if (!Utils.getBooleanProperty("turbovnc.sshkeytest", false))
      params.sshSession.setUserInfo(dlg);
    params.sshSession.connect();
  }

  /* Create a tunnel using an external SSH client.  This supports the same
     VNC_TUNNEL_CMD and VNC_VIA_CMD environment variables as the native viewers
     do. */

  private static final String DEFAULT_SSH_CMD =
    (Utils.isWindows() ? "ssh.exe" : "/usr/bin/env ssh");
  private static final String DEFAULT_TUNNEL_CMD =
    DEFAULT_SSH_CMD + " -axf -L %L:localhost:%R %H sleep 20";
  private static final String DEFAULT_VIA_CMD =
    DEFAULT_SSH_CMD + " -axf -L %L:%H:%R %G sleep 20";

  private static void createTunnelExt(String gatewayHost, String remoteHost,
                                      int remotePort, int localPort,
                                      String pattern, Params params,
                                      boolean tunnel)
                                      throws Exception {
    if (pattern == null || pattern.length() < 1)
      pattern = (tunnel ? DEFAULT_TUNNEL_CMD : DEFAULT_VIA_CMD);

    String command = fillCmdPattern(pattern, gatewayHost, remoteHost,
                                    remotePort, params.udsPath, localPort,
                                    params, tunnel);

    vlog.debug("SSH command line: " + command);
    List<String> args = ArgumentTokenizer.tokenize(command);
    ProcessBuilder pb = new ProcessBuilder(args);
    pb.redirectInput(ProcessBuilder.Redirect.INHERIT);
    pb.redirectOutput(ProcessBuilder.Redirect.INHERIT);
    pb.redirectError(ProcessBuilder.Redirect.INHERIT);
    Process p = pb.start();
    if (p == null || p.waitFor() != 0)
      throw new ErrorException("External SSH error");
  }

  private static final String DEFAULT_TUNNEL_CMD_UDS =
    DEFAULT_SSH_CMD + " -ax -- %H exec socat stdio unix-connect:%R";
  private static final String DEFAULT_VIA_CMD_UDS =
    DEFAULT_SSH_CMD + " -ax -J %G -- %H exec socat stdio unix-connect:%R";

  private static Socket createTunnelExtUDS(String gatewayHost,
                                           String remoteHost, String pattern,
                                           Params params, boolean tunnel)
                                           throws Exception {
    if (pattern == null || pattern.length() < 1)
      pattern = (tunnel ? DEFAULT_TUNNEL_CMD_UDS : DEFAULT_VIA_CMD_UDS);

    // Escape the Unix domain socket path twice, since it will be interpreted
    // once by ArgumentTokenizer.tokenize() and again by the remote shell.
    String udsPath = escapeUDSPath(params.udsPath, true);
    udsPath = escapeUDSPath(udsPath, false);

    String command = fillCmdPattern(pattern, gatewayHost, remoteHost, -1,
                                    udsPath, -1, params, tunnel);

    vlog.debug("SSH command line (stdio): " + command);
    List<String> args = ArgumentTokenizer.tokenize(command);
    ProcessBuilder pb = new ProcessBuilder(args);
    pb.redirectError(ProcessBuilder.Redirect.INHERIT);
    Process p = pb.start();
    if (p == null)
      throw new ErrorException("External SSH error");
    return new StreamSocket(p.getInputStream(), p.getOutputStream(), true);
  }

  protected static Socket connectUDSDirect(String udsPath) {
    udsPath = expandUDSPathLocal(udsPath);

    vlog.debug("Connecting to Unix domain socket: " + udsPath);
    try {
      ProcessBuilder pb =
        new ProcessBuilder("socat", "stdio",
                           "unix-connect:\"" + udsPath + "\"");
      pb.redirectError(ProcessBuilder.Redirect.INHERIT);
      Process p = pb.start();
      if (p == null)
        throw new ErrorException("socat error");
      return new StreamSocket(p.getInputStream(), p.getOutputStream(), true);
    } catch (Exception e) {
      throw new ErrorException("Could not start socat:\n" + e.getMessage());
    }
  }

  private static String fillCmdPattern(String pattern, String gatewayHost,
                                       String remoteHost, int remotePort,
                                       String udsPath, int localPort,
                                       Params params, boolean tunnel) {
    int i, j;
    boolean hFound = false, gFound = false, rFound = false, lFound = false;
    String command = "";

    if (params.sshUser.get() != null)
      gatewayHost = params.sshUser.get() + "@" + gatewayHost;

    for (i = 0; i < pattern.length(); i++) {
      if (pattern.charAt(i) == '%') {
        switch (pattern.charAt(++i)) {
          case 'H':
            command += (tunnel ? gatewayHost : remoteHost);
            hFound = true;
            continue;
          case 'G':
            command += gatewayHost;
            gFound = true;
            continue;
          case 'R':
            if (udsPath != null)
              command += udsPath;
            else
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

    if (!hFound || !rFound || (!lFound && udsPath == null))
      throw new ErrorException("%H, %R or %L absent in tunneling command template.");

    if (!tunnel && !gFound)
      throw new ErrorException("%G pattern absent in tunneling command template.");

    return command;
  }

  // Escape a remote Unix domain socket path so that it can survive being
  // parsed by a POSIX shell on the host or by ArgumentTokenizer.tokenize().
  // Expand ~ to the remote home directory, %h to the remote host name, %i to
  // the remote (numeric) user ID, and %u to the remote username.
  private static String escapeUDSPath(String udsPath, boolean expandTokens) {
    String result = "'";

    for (int i = 0; i < udsPath.length(); i++) {
      if (udsPath.charAt(i) == '\'') {
        // Replace ' with '"'"'
        result += "'\"'\"'";
        continue;
      }
      if (expandTokens) {
        if (i == 0 && udsPath.charAt(i) == '~') {
          result += "'\"$HOME\"'";
          continue;
        } else if (udsPath.charAt(i) == '%') {
          switch (udsPath.charAt(++i)) {
            case '%':
              break;
            case 'h':
              result += "'\"$(uname -n)\"'";
              continue;
            case 'i':
              result += "'\"$(id -u)\"'";
              continue;
            case 'u':
              result += "'\"$(id -u -n)\"'";
              continue;
            default:
              throw new ErrorException("Invalid % sequence (%" +
                                       udsPath.charAt(i) +
                                       ") in Unix domain socket path");
          }
        }
      }
      result += udsPath.charAt(i);
    }
    result += "'";

    return result;
  }

  // Expand a local (client-side) Unix domain socket path similarly to
  // escapeUDSPath()
  private static String expandUDSPathLocal(String udsPath) {
    String result = "";

    for (int i = 0; i < udsPath.length(); i++) {
      if (i == 0 && udsPath.charAt(i) == '~') {
        result += System.getProperty("user.home");
        continue;
      } else if (udsPath.charAt(i) == '%') {
        switch (udsPath.charAt(++i)) {
          case '%':
            break;
          case 'h':
            try {
              ProcessBuilder pb = new ProcessBuilder("uname", "-n");
              pb.redirectInput(ProcessBuilder.Redirect.INHERIT);
              pb.redirectError(ProcessBuilder.Redirect.INHERIT);
              Process p = pb.start();
              if (p == null)
                throw new ErrorException("error calling 'uname -n'");
              String id = new BufferedReader(
                new InputStreamReader(p.getInputStream())).readLine();
              p.getOutputStream().close();
              p.waitFor();
              result += id;
            } catch (Exception e) {
              throw new ErrorException("Could run 'uname -n':\n" +
                                       e.getMessage());
            }
            continue;
          case 'i':
            try {
              ProcessBuilder pb = new ProcessBuilder("id", "-u");
              pb.redirectInput(ProcessBuilder.Redirect.INHERIT);
              pb.redirectError(ProcessBuilder.Redirect.INHERIT);
              Process p = pb.start();
              if (p == null)
                throw new ErrorException("error calling 'id -u'");
              String id = new BufferedReader(
                new InputStreamReader(p.getInputStream())).readLine();
              p.getOutputStream().close();
              p.waitFor();
              result += id;
            } catch (Exception e) {
              throw new ErrorException("Could run 'id -u':\n" +
                                       e.getMessage());
            }
            continue;
          case 'u':
            result += System.getProperty("user.name");
            continue;
          default:
            throw new ErrorException("Invalid % sequence (%" +
                                     udsPath.charAt(i) +
                                     ") in Unix domain socket path");
        }
      }
      result += udsPath.charAt(i);
    }

    return result;
  }

  // JSch logging interface
  private static final Logger LOGGER = new Logger() {
    public boolean isEnabled(int level) {
      switch (level) {
        case Logger.DEBUG:
        case Logger.INFO:
        case Logger.WARN:
        case Logger.ERROR:
        case Logger.FATAL:
          return true;
        default:
          return false;
      }
    }

    public void log(int level, String message) {
      switch (level) {
        case Logger.DEBUG:
        case Logger.INFO:
          vlogSSH.sshdebug(message);
          return;
        case Logger.WARN:
          vlogSSH.status(message);
          return;
        case Logger.ERROR:
          vlogSSH.error(message);
          return;
        case Logger.FATAL:
          throw new ErrorException("JSch: " + message);
      }
    }
  };

  protected Tunnel() {}
  static LogWriter vlog = new LogWriter("Tunnel");
  static LogWriter vlogSSH = new LogWriter("JSch");
}
