/* Copyright (C) 2012, 2015-2020, 2022 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2011-2012, 2015, 2017, 2019 Brian P. Hinz
 * Copyright (C) 2010 m-privacy GmbH
 * Copyright (C) 2010 TigerVNC Team
 * Copyright (C) 2005 Martin Koegler
 * Copyright (C) 2004 Red Hat Inc.
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

package com.turbovnc.rfb;

import javax.net.ssl.*;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.NoSuchAlgorithmException;
import java.security.MessageDigest;
import java.security.Security;
import java.security.cert.*;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.InputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import javax.naming.InvalidNameException;
import javax.naming.ldap.LdapName;
import javax.naming.ldap.Rdn;
import javax.swing.JOptionPane;
import org.spf4j.base.Base64;

import com.turbovnc.rdr.*;
import com.turbovnc.network.*;

public class CSecurityTLS extends CSecurity {

  static final String[] HEX_CODES = {
    "00", "01", "02", "03", "04", "05", "06", "07",
    "08", "09", "0A", "0B", "0C", "0D", "0E", "0F",
    "10", "11", "12", "13", "14", "15", "16", "17",
    "18", "19", "1A", "1B", "1C", "1D", "1E", "1F",
    "20", "21", "22", "23", "24", "25", "26", "27",
    "28", "29", "2A", "2B", "2C", "2D", "2E", "2F",
    "30", "31", "32", "33", "34", "35", "36", "37",
    "38", "39", "3A", "3B", "3C", "3D", "3E", "3F",
    "40", "41", "42", "43", "44", "45", "46", "47",
    "48", "49", "4A", "4B", "4C", "4D", "4E", "4F",
    "50", "51", "52", "53", "54", "55", "56", "57",
    "58", "59", "5A", "5B", "5C", "5D", "5E", "5F",
    "60", "61", "62", "63", "64", "65", "66", "67",
    "68", "69", "6A", "6B", "6C", "6D", "6E", "6F",
    "70", "71", "72", "73", "74", "75", "76", "77",
    "78", "79", "7A", "7B", "7C", "7D", "7E", "7F",
    "80", "81", "82", "83", "84", "85", "86", "87",
    "88", "89", "8A", "8B", "8C", "8D", "8E", "8F",
    "90", "91", "92", "93", "94", "95", "96", "97",
    "98", "99", "9A", "9B", "9C", "9D", "9E", "9F",
    "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7",
    "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF",
    "B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7",
    "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF",
    "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7",
    "C8", "C9", "CA", "CB", "CC", "CD", "CE", "CF",
    "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
    "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF",
    "E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7",
    "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF",
    "F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7",
    "F8", "F9", "FA", "FB", "FC", "FD", "FE", "FF"
  };

  private void initGlobal() {
    boolean globalInitDone = false;

    if (!globalInitDone) {
      try {
        // Java 7u211, 8u201, and 11.0.2 now disable anonymous cipher suites in
        // the java.security file, so we have to do this in order to re-enable
        // them.
        String disabledAlgorithms =
          Security.getProperty("jdk.tls.disabledAlgorithms");
        disabledAlgorithms = disabledAlgorithms.replaceAll(
          "(^\\s*ECDH_anon\\s*($|,))|((,|^)\\s*ECDH_anon\\s*)", "");
        disabledAlgorithms = disabledAlgorithms.replaceAll(
          "(^\\s*DH_anon\\s*($|,))|((,|^)\\s*DH_anon\\s*)", "");
        disabledAlgorithms = disabledAlgorithms.replaceAll(
          "(^\\s*anon\\s*($|,))|((,|^)\\s*anon\\s*)", "");
        Security.setProperty("jdk.tls.disabledAlgorithms", disabledAlgorithms);
        ctx = SSLContext.getInstance("TLS");
      } catch (NoSuchAlgorithmException e) {
        throw new ErrorException(e.getMessage());
      }

      globalInitDone = true;
    }
  }

  public CSecurityTLS(boolean anon_) {
    anon = anon_;
    manager = null;
  }

  // FIXME:
  // Need to shutdown the connection cleanly

  // FIXME?
  // add a finalizer method that calls shutdown

  public boolean processMsg(CConnection cc) {
    return processMsg(cc, true);
  }

  protected boolean processMsg(CConnection cc, boolean readResponse) {
    is = (FdInStream)cc.getInStream();
    os = (FdOutStream)cc.getOutStream();
    client = cc;

    initGlobal();

    if (manager == null) {
      if (readResponse) {
        if (!is.checkNoWait(1))
          return false;

        if (is.readU8() == 0) {
          int result = is.readU32();
          String reason;
          if (result == RFB.AUTH_FAILED || result == RFB.AUTH_TOO_MANY)
            reason = is.readString();
          else
            reason = new String("Authentication failure (protocol error)");
          throw new AuthFailureException(reason);
        }
      }

      setParam();
    }

    try {
      manager = new SSLEngineManager(engine, is, os);
      manager.doHandshake();
      vlog.debug("Negotiated cipher suite: " +
                 manager.getSession().getCipherSuite());
    } catch (Exception e) {
      SystemException.checkException(e);
      throw new SystemException(e);
    }

    cc.setStreams(new TLSInStream(is, manager),
                  new TLSOutStream(os, manager));
    return true;
  }

  private void setParam() {

    if (anon) {
      try {
        ctx.init(null, null, null);
      } catch (KeyManagementException e) {
        throw new AuthFailureException(e.getMessage());
      }
    } else {
      try {
        TrustManager[] myTM = new TrustManager[] {
          new MyX509TrustManager(client.params)
        };
        ctx.init(null, myTM, null);
      } catch (java.security.GeneralSecurityException e) {
        throw new AuthFailureException(e.getMessage());
      }
    }
    SSLSocketFactory sslfactory = ctx.getSocketFactory();
    engine = ctx.createSSLEngine(client.getServerName(),
                                 client.getServerPort());
    engine.setUseClientMode(true);

    String[] supported = engine.getSupportedProtocols();
    ArrayList<String> enabled = new ArrayList<String>();
    for (int i = 0; i < supported.length; i++)
      if (supported[i].matches("TLS.*"))
        enabled.add(supported[i]);
    engine.setEnabledProtocols(enabled.toArray(new String[0]));

    String cipherProperty = System.getProperty("turbovnc.ciphersuites");
    if (cipherProperty != null)
      supported = cipherProperty.split("\\s*,\\s*");
    else
      supported = engine.getSupportedCipherSuites();
    if (anon) {
      enabled = new ArrayList<String>();
      // prefer ECDH over DHE
      for (int i = 0; i < supported.length; i++)
        if (supported[i].matches("TLS_ECDH_anon.*"))
          enabled.add(supported[i]);
      for (int i = 0; i < supported.length; i++)
        if (supported[i].matches("TLS_DH_anon.*"))
          enabled.add(supported[i]);
      engine.setEnabledCipherSuites(enabled.toArray(new String[0]));
    } else {
      engine.setEnabledCipherSuites(supported);
    }

    supported = engine.getEnabledCipherSuites();
    StringBuilder sb = new StringBuilder();
    for (int i = 0; i < supported.length; i++) {
      sb.append(supported[i]);
      if (i < supported.length - 1)
        sb.append(", ");
    }
    vlog.debug("Available cipher suites: " + sb.toString());
    if (cipherProperty == null)
      vlog.debug("    (Set the turbovnc.ciphersuites system property to override.)");
  }

  class MyX509TrustManager implements X509TrustManager {

    X509TrustManager tm;

    MyX509TrustManager(Params params)
      throws java.security.GeneralSecurityException {
      KeyStore ks = KeyStore.getInstance("JKS");
      CertificateFactory cf = CertificateFactory.getInstance("X.509");
      try {
        ks.load(null, null);
        String a = TrustManagerFactory.getDefaultAlgorithm();
        TrustManagerFactory tmf = TrustManagerFactory.getInstance(a);
        tmf.init((KeyStore)null);
        for (TrustManager m : tmf.getTrustManagers())
          if (m instanceof X509TrustManager)
            for (X509Certificate c :
                 ((X509TrustManager)m).getAcceptedIssuers())
              ks.setCertificateEntry(getThumbprint((X509Certificate)c), c);
        File cacert = new File(params.x509ca.get());
        vlog.debug("Using X.509 CA certificate " + params.x509ca.get());
        if (cacert.exists() && cacert.canRead()) {
          InputStream caStream = new MyFileInputStream(cacert);
          Collection<? extends Certificate> cacerts =
            cf.generateCertificates(caStream);
          for (Certificate cert : cacerts) {
            String thumbprint = getThumbprint((X509Certificate)cert);
            ks.setCertificateEntry(thumbprint, (X509Certificate)cert);
          }
        }
        PKIXBuilderParameters pkixParams =
          new PKIXBuilderParameters(ks, new X509CertSelector());
        File crlcert = new File(params.x509crl.get());
        if (!crlcert.exists() || !crlcert.canRead()) {
          vlog.debug("Not using X.509 CRL");
          pkixParams.setRevocationEnabled(false);
        } else {
          vlog.debug("Using X.509 CRL " + params.x509crl.get());
          InputStream crlStream =
            new FileInputStream(params.x509crl.get());
          Collection<? extends CRL> crls = cf.generateCRLs(crlStream);
          CertStoreParameters csp = new CollectionCertStoreParameters(crls);
          CertStore store = CertStore.getInstance("Collection", csp);
          pkixParams.addCertStore(store);
          pkixParams.setRevocationEnabled(true);
        }
        tmf = TrustManagerFactory.getInstance("PKIX");
        tmf.init(new CertPathTrustManagerParameters(pkixParams));
        tm = (X509TrustManager)tmf.getTrustManagers()[0];
      } catch (Exception e) {
        SystemException.checkException(e);
        throw new SystemException(e);
      }
    }

    public void checkClientTrusted(X509Certificate[] chain, String authType)
      throws CertificateException {
      tm.checkClientTrusted(chain, authType);
    }

    private String getCertificateInfo(X509Certificate cert) {
      return "  Subject: " + cert.getSubjectX500Principal().getName() + "\n" +
             "  Issuer: " + cert.getIssuerX500Principal().getName() + "\n" +
             "  Serial Number: " + cert.getSerialNumber() + "\n" +
             "  Version: " + cert.getVersion() + "\n" +
             "  Signature Algorithm: " + cert.getPublicKey().getAlgorithm() +
             "\n" +
             "  Not Valid Before: " + cert.getNotBefore() + "\n" +
             "  Not Valid After: " + cert.getNotAfter() + "\n" +
             "  SHA1 Fingerprint: " + getThumbprint(cert);
    }

    public void checkServerTrusted(X509Certificate[] chain, String authType)
      throws CertificateException {
      Collection<? extends Certificate> certs = null;
      X509Certificate cert = chain[0];
      boolean expiredOK = false;

      try {
        cert.checkValidity();
      } catch (CertificateNotYetValidException e) {
        throw new ErrorException("X.509 certificate is not valid yet\n\n" +
                                 getCertificateInfo(cert));
      } catch (CertificateExpiredException e) {
        Object[] answer = { "YES", "NO" };
        int ret = JOptionPane.showOptionDialog(null,
          "X.509 certificate has expired\n\n" + getCertificateInfo(cert) +
          "\n\n" + "Do you want to continue?", "Certificate Expired",
          JOptionPane.YES_NO_OPTION, JOptionPane.WARNING_MESSAGE,
          null, answer, answer[1]);
        if (ret != JOptionPane.YES_OPTION)
          throw new WarningException("X.509 certificate not trusted");
        expiredOK = true;
      }
      String thumbprint = getThumbprint(cert);
      File vncDir = new File(Utils.getVncHomeDir());
      File certFile = new File(vncDir, "x509_savedcerts.pem");
      CertificateFactory cf = CertificateFactory.getInstance("X.509");
      if (vncDir.exists() && certFile.exists() && certFile.canRead()) {
        InputStream certStream = new MyFileInputStream(certFile);
        certs = cf.generateCertificates(certStream);
        for (Certificate c : certs)
          if (thumbprint.equals(getThumbprint((X509Certificate)c)))
            return;
      }
      try {
        verifyHostname(cert);
        tm.checkServerTrusted(chain, authType);
      } catch (Exception e) {
        if (e.getCause() instanceof CertPathBuilderException) {
          Object[] answer = { "YES", "NO" };
          int ret = JOptionPane.showOptionDialog(null,
            "X.509 certificate has been signed by an unknown authority\n\n" +
            getCertificateInfo(cert) + "\n\n" +
            "Do you want to save it and continue?",
            "Certificate Issuer Unknown",
            JOptionPane.YES_NO_OPTION, JOptionPane.WARNING_MESSAGE,
            null, answer, answer[0]);
          if (ret == JOptionPane.YES_OPTION) {
            if (certs == null || !certs.contains(cert)) {
              byte[] der = cert.getEncoded();
              String pem = Base64.encodeBase64(der);
              pem = pem.replaceAll("(.{64})", "$1\n");
              FileWriter fw = null;
              try {
                if (!vncDir.exists())
                  vncDir.mkdir();
                if (!certFile.exists() && !certFile.createNewFile())
                  vlog.error("Certificate save failed.");
                else {
                  fw = new FileWriter(certFile.getAbsolutePath(), true);
                  fw.write("-----BEGIN CERTIFICATE-----\n");
                  fw.write(pem + "\n");
                  fw.write("-----END CERTIFICATE-----\n");
                }
              } catch (IOException ioe) {
                throw new SystemException(ioe);
              } finally {
                try {
                  if (fw != null)
                    fw.close();
                } catch (IOException ioe2) {
                  throw new SystemException(ioe2);
                }
              }
            }
          } else {
            throw new WarningException("X.509 certificate not trusted");
          }
        } else {
          Throwable cause = e.getCause();
          if (cause == null ||
              !(cause instanceof CertPathValidatorException) || !expiredOK) {
            SystemException.checkException(e);
            throw new SystemException(e);
          }
        }
      }
    }

    public X509Certificate[] getAcceptedIssuers() {
      return tm.getAcceptedIssuers();
    }

    private String getThumbprint(X509Certificate cert) {
      String thumbprint = null;
      try {
        MessageDigest md = MessageDigest.getInstance("SHA-1");
        md.update(cert.getEncoded());
        StringBuffer hexBuffer = new StringBuffer();
        byte[] digest = md.digest();
        for (int i = 0; i < digest.length; i++)
          hexBuffer.append(HEX_CODES[digest[i] & 0xFF]);
        thumbprint = hexBuffer.toString();
        thumbprint = thumbprint.replaceAll("..(?!$)", "$0 ");
      } catch (CertificateEncodingException e) {
        throw new SystemException(e);
      } catch (NoSuchAlgorithmException e) {
        throw new SystemException(e);
      }
      return thumbprint;
    }

    private void verifyHostname(X509Certificate cert) {
      try {
        Collection sans = cert.getSubjectAlternativeNames();
        if (sans == null) {
          String dn = cert.getSubjectX500Principal().getName();
          LdapName ln = new LdapName(dn);
          for (Rdn rdn : ln.getRdns()) {
            if (rdn.getType().equalsIgnoreCase("CN")) {
              String peer = client.getServerName().toLowerCase();
              if (peer.equals(((String)rdn.getValue()).toLowerCase()))
                return;
            }
          }
        } else {
          Iterator i = sans.iterator();
          while (i.hasNext()) {
            List nxt = (List)i.next();
            if (((Integer)nxt.get(0)).intValue() == 2) {
              String peer = client.getServerName().toLowerCase();
              if (peer.equals(((String)nxt.get(1)).toLowerCase()))
                return;
            } else if (((Integer)nxt.get(0)).intValue() == 7) {
              String peer = client.getSocket().getPeerAddress();
              if (peer.equals(((String)nxt.get(1)).toLowerCase()))
                return;
            }
          }
        }
        Object[] answer = { "YES", "NO" };
        int ret = JOptionPane.showOptionDialog(null,
          "X.509 hostname verification failed.  Do you want to continue?",
          "Hostname Verification Failure",
          JOptionPane.YES_NO_OPTION, JOptionPane.WARNING_MESSAGE,
          null, answer, answer[0]);
        if (ret != JOptionPane.YES_OPTION)
          throw new WarningException("X.509 certificate not trusted");
      } catch (CertificateParsingException e) {
        throw new SystemException(e);
      } catch (InvalidNameException e) {
        throw new SystemException(e);
      }
    }

    private class MyFileInputStream extends InputStream {
      // Blank lines in a certificate file will cause Java 6 to throw a
      // "DerInputStream.getLength(): lengthTag=127, too big" exception.
      ByteBuffer buf;

      MyFileInputStream(String name) {
        this(new File(name));
      }

      MyFileInputStream(File file) {
        StringBuffer sb = new StringBuffer();
        BufferedReader reader = null;
        try {
          reader = new BufferedReader(new FileReader(file));
          String l;
          while ((l = reader.readLine()) != null) {
            if (l.trim().length() > 0)
              sb.append(l + "\n");
          }
        } catch (Exception e) {
          throw new SystemException(e);
        } finally {
          try {
            if (reader != null)
              reader.close();
          } catch (IOException ioe) {
            throw new SystemException(ioe);
          }
        }
        Charset utf8 = Charset.forName("UTF-8");
        buf = ByteBuffer.wrap(sb.toString().getBytes(utf8));
        buf.limit(buf.capacity());
      }

      @Override
      public int read(byte[] b) throws IOException {
        return this.read(b, 0, b.length);
      }

      @Override
      public int read(byte[] b, int off, int len) throws IOException {
        if (!buf.hasRemaining())
          return -1;
        len = Math.min(len, buf.remaining());
        buf.get(b, off, len);
        return len;
      }

      @Override
      public int read() throws IOException {
        if (!buf.hasRemaining())
          return -1;
        return buf.get() & 0xFF;
      }
    }
  }

  public int getType() {
    return anon ? RFB.SECTYPE_TLS_NONE : RFB.SECTYPE_X509_NONE;
  }

  public String getDescription() {
    return anon ? "TLSNone" : "X509None";
  }

  public final String getProtocol() {
    if (manager != null && manager.getSession() != null)
      return manager.getSession().getProtocol();
    return "Not initialized";
  }

  protected CConnection client;

  private SSLContext ctx;
  private SSLEngine engine;
  private SSLEngineManager manager;
  private boolean anon;

  private FdInStream is;
  private FdOutStream os;

  static LogWriter vlog = new LogWriter("CSecurityTLS");
}
