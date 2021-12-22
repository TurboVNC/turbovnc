/*  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
 *  Copyright (C) 2000 Const Kaplinsky.  All Rights Reserved.
 *  Copyright (C) 2012-2015, 2017-2018, 2020-2021 D. R. Commander.
 *                                                All Rights Reserved.
 *  Copyright (C) 2012, 2016 Brian P. Hinz.  All Rights Reserved.
 *  Copyright (C) 2021 Steffen Kieß
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

import java.io.*;
import java.util.*;

import com.turbovnc.rfb.*;
import com.turbovnc.rdr.*;
import com.turbovnc.network.*;

import com.jcraft.jsch.agentproxy.*;
import com.jcraft.jsch.agentproxy.connector.*;
import com.jcraft.jsch.agentproxy.usocket.*;
import com.jcraft.jsch.*;

public class Tunnel {

  public static void createTunnel(Options opts) throws Exception {
    int localPort;
    int remotePort;
    String gatewayHost;
    String remoteHost;

    boolean tunnel = opts.tunnel ||
                     (opts.sessMgrActive && Params.sessMgrAuto.getValue());

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

    if (tunnel) {
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

    if (opts.unixDomainPath != null &&
        (pattern == null || pattern.indexOf("%L") < 0)) {
      // Connect to unix domain socket using stdio-based forwarding

      if (Params.extSSH.getValue() || (pattern != null && pattern.length() > 0))
        opts.stdioSocket = createTunnelExtStdio(gatewayHost, remoteHost,
                                                opts.unixDomainPath, pattern,
                                                opts);
      else {
        vlog.debug("Opening SSH stdio tunnel through gateway " + gatewayHost);
        opts.stdioSocket = createTunnelJSchStdio(opts.serverName, opts);
      }
    } else {
      // Forward to a local TCP port and connect to this port

      String unixDomainPath = null;
      if (opts.unixDomainPath != null) {
        // Escape the unix domain path once: It will only be interpreted by
        // ArgumentTokenizer.tokenize().
        // Expansions are not supported in this case
        unixDomainPath = escapeUnixDomainPath(opts.unixDomainPath, false);
      }

      localPort = TcpSocket.findFreeTcpPort();
      if (localPort == 0)
        throw new ErrorException("Could not obtain free TCP port");

      if (Params.extSSH.getValue() || (pattern != null && pattern.length() > 0))
        createTunnelExt(gatewayHost, remoteHost, remotePort, unixDomainPath,
                        localPort, pattern, opts);
      else {
        vlog.debug("Opening SSH tunnel through gateway " + gatewayHost);
        if (opts.sshSession == null)
          createTunnelJSch(gatewayHost, opts);
        vlog.debug("Forwarding local port " + localPort + " to " + remoteHost +
                   ":" + remotePort + " (relative to gateway)");
        opts.sshSession.setPortForwardingL(localPort, remoteHost, remotePort);
      }
      opts.serverName = "localhost::" + localPort;
    }
    opts.sshTunnelActive = true;
  }

  /* Create a tunnel using the builtin JSch SSH client */

  protected static void createTunnelJSch(String host, Options opts)
                                         throws Exception {
    opts.sshSession = openJSchConnection(host, opts, null);
  }

  static Socket createTunnelJSchStdio(String remoteHost, Options opts)
                                      throws Exception {
    String command = "exec socat stdio unix-connect:\\\"" +
      escapeUnixDomainPath(opts.unixDomainPath, true) + "\\\"";

    JSchProxy proxy = null;
    if (opts.via != null)
      proxy = new JSchProxy(opts.via, opts);

    vlog.debug("Opening SSH tunnel to " + remoteHost);

    Session session = openJSchConnection(remoteHost, opts, proxy);

    ChannelExec channelExec = (ChannelExec)session.openChannel("exec");
    channelExec.setCommand(command);
    final InputStream stdout = channelExec.getInputStream();
    final InputStream stderr = channelExec.getErrStream();
    final OutputStream stdin = channelExec.getOutputStream();
    channelExec.connect();

    // Copy stderr from remote process to stderr
    Thread thread = new Thread() {
        public void run() {
          byte[] buffer = new byte[1024];
          try {
            while (true) {
              int len = stderr.read(buffer, 0, buffer.length);
              if (len < 0)
                return;
              System.err.write(buffer, 0, len);
            }
          } catch (IOException e) {
            throw new ErrorException("Error reading error stream: " + e.getMessage());
          }
        }
      };
    thread.start();

    return new StreamSocket(stdout, stdin, true);
  }

  private static Session openJSchConnection(String host, Options opts,
                                            Proxy proxy) throws Exception {
    JSch jsch = new JSch();
    JSch.setLogger(LOGGER);
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
          Iterator<com.jcraft.jsch.Identity> iter =
            repo.getIdentities().iterator();
          while (iter.hasNext()) {
            com.jcraft.jsch.Identity id = iter.next();
            vlog.sshdebug("  " + id.getName());
            if (id.getFingerPrint() != null)
              vlog.sshdebug("    Fingerprint: " + id.getFingerPrint());
          }
          jsch.setIdentityRepository(repo);
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
    String sshKeyFile = Params.sshKeyFile.getValue();
    String sshKey = Params.sshKey.getValue();
    boolean useDefaultPrivateKeyFiles = false;
    if (sshKey != null) {
      String sshKeyPass = Params.sshKeyPass.getValue();
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
    int port = Params.sshPort.getValue();
    String user = opts.sshUser;
    if (user == null)
      user = (String)System.getProperties().get("user.name");

    File sshConfigFile = new File(Params.sshConfig.getValue());
    if (sshConfigFile.exists() && sshConfigFile.canRead()) {
      ConfigRepository repo =
        OpenSSHConfig.parseFile(sshConfigFile.getAbsolutePath());
      jsch.setConfigRepository(repo);
      vlog.debug("Read OpenSSH config file " + Params.sshConfig.getValue());
      // This just ensures that the password dialog displays the correct
      // username.  JSch will ignore the username and port passed to
      // getSession() if the configuration has already been set using an
      // OpenSSH configuration file.
      String repoUser = repo.getConfig(host).getUser();
      if (repoUser != null)
        user = repoUser;
      String[] identityFiles = repo.getConfig(host).getValues("IdentityFile");
      if (identityFiles != null) {
        for (String file : identityFiles) {
          if (file != null && !file.isEmpty())
            useDefaultPrivateKeyFiles = false;
        }
      }
    } else {
      if (Params.sshConfig.isDefault()) {
        vlog.debug("Could not parse SSH config file " +
                   Params.sshConfig.getValue());
      } else {
        vlog.info("Could not parse SSH config file " +
                  Params.sshConfig.getValue());
      }
    }

    if (useDefaultPrivateKeyFiles) {
      privateKeys.add(new File(homeDir + "/.ssh/id_rsa"));
      privateKeys.add(new File(homeDir + "/.ssh/id_dsa"));
    }

    for (Iterator<File> i = privateKeys.iterator(); i.hasNext();) {
      File privateKey = (File)i.next();
      try {
        if (privateKey.exists() && privateKey.canRead()) {
          if (Params.sshKeyPass.getValue() != null)
            jsch.addIdentity(privateKey.getAbsolutePath(),
                             Params.sshKeyPass.getValue());
          else
            jsch.addIdentity(privateKey.getAbsolutePath());
        }
      } catch (Exception e) {
        throw new ErrorException("Could not use SSH private key " +
                                 privateKey.getAbsolutePath() + ":\n" +
                                 e.getMessage());
      }
    }

    Session session = jsch.getSession(user, host, port);
    if (proxy != null)
      session.setProxy(proxy);
    // OpenSSHConfig doesn't recognize StrictHostKeyChecking
    if (session.getConfig("StrictHostKeyChecking") == null)
      session.setConfig("StrictHostKeyChecking", "ask");
    session.setConfig("MaxAuthTries", "3");
    String auth = System.getProperty("turbovnc.sshauth");
    if (auth == null)
      auth = "publickey,keyboard-interactive,password";
    session.setConfig("PreferredAuthentications", auth);
    PasswdDialog dlg = new PasswdDialog(new String("SSH Authentication"),
                                        true, user, false, true, -1);
    session.setUserInfo(dlg);
    session.connect();

    return session;
  }

  /* Create a tunnel using an external SSH client.  This supports the same
     VNC_TUNNEL_CMD and VNC_VIA_CMD environment variables as the native viewers
     do. */

  private static final String DEFAULT_SSH_CMD =
    (Utils.isWindows() ? "ssh.exe" : "/usr/bin/ssh");
  private static final String DEFAULT_TUNNEL_CMD =
    DEFAULT_SSH_CMD + " -ax -f -L %L:localhost:%R %H sleep 20";
  private static final String DEFAULT_VIA_CMD =
    DEFAULT_SSH_CMD + " -ax -f -L %L:%H:%R %G sleep 20";
  private static final String DEFAULT_TUNNEL_CMD_UNIX =
    DEFAULT_SSH_CMD + " -ax -- %H exec socat stdio unix-connect:%R";
  private static final String DEFAULT_VIA_CMD_UNIX =
    DEFAULT_SSH_CMD + " -ax -J %G -- %H exec socat stdio unix-connect:%R";

  public static void createTunnelExt(String gatewayHost, String remoteHost,
                                     int remotePort, String unixDomainPath,
                                     int localPort, String pattern,
                                     Options opts) throws Exception {
    if (pattern == null || pattern.length() < 1)
      pattern = (opts.tunnel ? DEFAULT_TUNNEL_CMD : DEFAULT_VIA_CMD);

    String command = fillCmdPattern(pattern, gatewayHost, remoteHost,
                                    remotePort, unixDomainPath, localPort,
                                    opts, false);

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

  static Socket createTunnelExtStdio(String gatewayHost, String remoteHost,
                                     String unixDomainPath, String pattern,
                                     Options opts) throws Exception {
    if (pattern == null || pattern.length() < 1)
      pattern = (opts.tunnel ? DEFAULT_TUNNEL_CMD_UNIX : DEFAULT_VIA_CMD_UNIX);

    // Escape the unix domain path twice: Once it will be interpreted by
    // ArgumentTokenizer.tokenize(), once by the remote shell
    String path = escapeUnixDomainPath(unixDomainPath, true);
    path = escapeUnixDomainPath(path, false);

    String command = fillCmdPattern(pattern, gatewayHost, remoteHost, -1,
                                    path, -1, opts, true);

    vlog.debug("SSH command line (stdio): " + command);
    List<String> args = ArgumentTokenizer.tokenize(command);
    ProcessBuilder pb = new ProcessBuilder(args);
    pb.redirectError(ProcessBuilder.Redirect.INHERIT);
    Process p = pb.start();
    if (p == null)
      throw new ErrorException("External SSH error");
    return new StreamSocket(p.getInputStream(), p.getOutputStream(), true);
  }

  static Socket connectUnixDirect(String unixDomainPath) {
    String socketPath = expandUnixDomainPathLocal(unixDomainPath);

    vlog.debug("Connecting to socket: " + socketPath);
    try {
      ProcessBuilder pb = new ProcessBuilder("socat", "stdio", "unix-connect:\"" + socketPath + "\"");
      pb.redirectError(ProcessBuilder.Redirect.INHERIT);
      Process p = pb.start();
      if (p == null)
        throw new ErrorException("socat error");
      return new StreamSocket(p.getInputStream(), p.getOutputStream(), true);
    } catch (Exception e) {
      throw new ErrorException("Could start socat:\n" + e.getMessage());
    }
  }

  private static String fillCmdPattern(String pattern, String gatewayHost,
                                       String remoteHost, int remotePort,
                                       String unixDomainPath, int localPort,
                                       Options opts, boolean stdio) {
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
            if (unixDomainPath != null)
              command += unixDomainPath;
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

    if (!hFound || !rFound || (!lFound && !stdio))
      throw new ErrorException("%H, %R or %L absent in tunneling command template.");

    if (!opts.tunnel && !gFound)
      throw new ErrorException("%G pattern absent in tunneling command template.");

    return command;
  }

  // Escape the string for a posix shell or for ArgumentTokenizer.tokenize().
  // Expand %d to the remote home directory, %i to the remote (numeric) user ID
  // and %u to the remote username.
  private static String escapeUnixDomainPath(String s, boolean expandTokens) {
    String result = "'";

    for (int i = 0; i < s.length(); i++) {
      if (s.charAt(i) == '\'') {
        // Replace ' by '"'"'
        result += "'\"'\"'";
        continue;
      }
      if (expandTokens && s.charAt(i) == '%') {
        switch (s.charAt(++i)) {
          case '%':
            break;
          case 'd':
            result += "'\"$HOME\"'";
            continue;
          case 'i':
            result += "'\"$(id -u)\"'";
            continue;
          case 'u':
            result += "'\"$(id -u -n)\"'";
            continue;
          default:
            throw new ErrorException("Invalid % sequence in unix domain path: %" + s.charAt(i));
        }
      }
      result += s.charAt(i);
    }

    result += "'";
    return result;
  }

  // Expand a unix domain path similar to escapeUnixDomainPath, but do the
  // expansion locally
  private static String expandUnixDomainPathLocal(String s) {
    String result = "";

    for (int i = 0; i < s.length(); i++) {
      if (s.charAt(i) == '%') {
        switch (s.charAt(++i)) {
          case '%':
            break;
          case 'd':
            result += System.getProperty("user.home");
            continue;
          case 'i':
            try {
              ProcessBuilder pb = new ProcessBuilder("id", "-u");
              pb.redirectInput(ProcessBuilder.Redirect.INHERIT);
              pb.redirectError(ProcessBuilder.Redirect.INHERIT);
              Process p = pb.start();
              if (p == null)
                throw new ErrorException("error calling 'id -u'");
              String id = new BufferedReader(new InputStreamReader(p.getInputStream())).readLine();
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
            throw new ErrorException("Invalid % sequence in unix domain path: %" + s.charAt(i));
        }
      }
      result += s.charAt(i);
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

  static class JSchProxy implements com.jcraft.jsch.Proxy {
    Session outerSession;
    ChannelDirectTCPIP channel;
    PipedOutputStream toRemotePipeOut;
    PipedInputStream fromRemotePipeIn;

    public JSchProxy(String gatewayHost, Options opts) throws Exception {
      vlog.debug("Opening outer SSH tunnel to " + gatewayHost);
      outerSession = Tunnel.openJSchConnection(gatewayHost, opts, null);
    }

    public void connect(SocketFactory socket_factory, String host, int port, int timeout) throws Exception {
      vlog.debug("Connecting over tunnel to " + host + ":" + port);
      channel = (ChannelDirectTCPIP)outerSession.openChannel("direct-tcpip");
      channel.setHost(host);
      channel.setPort(port);
      PipedInputStream toRemotePipeIn = new PipedInputStream();
      toRemotePipeOut = new PipedOutputStream(toRemotePipeIn);
      fromRemotePipeIn = new PipedInputStream();
      PipedOutputStream fromRemotePipeOut = new PipedOutputStream(fromRemotePipeIn);
      channel.setInputStream(toRemotePipeIn);
      channel.setOutputStream(fromRemotePipeOut);
      channel.connect();
    }

    public InputStream getInputStream() {
      return fromRemotePipeIn;
    }

    public OutputStream getOutputStream() {
      return toRemotePipeOut;
    }

    public java.net.Socket getSocket() {
      return null;
    }

    public void close() {
      if (channel != null)
        channel.disconnect();
      channel = null;
      fromRemotePipeIn = null;
      toRemotePipeOut = null;
    }

    public void closeOuter() {
      outerSession.disconnect();
    }
  }

  protected Tunnel() {}
  static LogWriter vlog = new LogWriter("Tunnel");
  static LogWriter vlogSSH = new LogWriter("JSch");
}
