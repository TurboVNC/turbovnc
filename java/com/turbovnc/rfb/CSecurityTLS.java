/*
 * Copyright (C) 2004 Red Hat Inc.
 * Copyright (C) 2005 Martin Koegler
 * Copyright (C) 2010 m-privacy GmbH
 * Copyright (C) 2010 TigerVNC Team
 * Copyright (C) 2011-2012, 2015 Brian P. Hinz
 * Copyright (C) 2012, 2015-2018 D. R. Commander.  All Rights Reserved.
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
import javax.xml.bind.DatatypeConverter;

import com.turbovnc.rdr.*;
import com.turbovnc.network.*;
import com.turbovnc.vncviewer.*;

public class CSecurityTLS extends CSecurity {

  private void initGlobal() {
    boolean globalInitDone = false;

    if (!globalInitDone) {
      try {
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
    } catch (java.lang.Exception e) {
      if (e.getMessage().equals("X.509 certificate not trusted"))
        throw new WarningException(e.getMessage());
      else
        throw new SystemException(e.toString());
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
          new MyX509TrustManager()
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

    if (anon) {
      supported = engine.getSupportedCipherSuites();
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
      engine.setEnabledCipherSuites(engine.getSupportedCipherSuites());
    }
  }

  class MyX509TrustManager implements X509TrustManager {

    X509TrustManager tm;

    MyX509TrustManager() throws java.security.GeneralSecurityException {
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
              ks.setCertificateEntry(c.getSubjectX500Principal().getName(), c);
        File castore = new File(FileUtils.getVncHomeDir() +
                                "x509_savedcerts.pem");
        if (castore.exists() && castore.canRead()) {
          InputStream caStream = new MyFileInputStream(castore);
          Collection<? extends Certificate> cacerts =
            cf.generateCertificates(caStream);
          for (Certificate cert : cacerts) {
            String dn =
              ((X509Certificate)cert).getSubjectX500Principal().getName();
            ks.setCertificateEntry(dn, (X509Certificate)cert);
          }
        }
        File cacert = new File(SecurityClient.x509ca.getValue());
        vlog.debug("Using X.509 CA certificate " +
                   SecurityClient.x509ca.getValue());
        if (cacert.exists() && cacert.canRead()) {
          InputStream caStream = new MyFileInputStream(cacert);
          Collection<? extends Certificate> cacerts =
            cf.generateCertificates(caStream);
          for (Certificate cert : cacerts) {
            String dn =
              ((X509Certificate)cert).getSubjectX500Principal().getName();
            ks.setCertificateEntry(dn, (X509Certificate)cert);
          }
        }
        PKIXBuilderParameters params =
          new PKIXBuilderParameters(ks, new X509CertSelector());
        File crlcert = new File(SecurityClient.x509crl.getValue());
        if (!crlcert.exists() || !crlcert.canRead()) {
          vlog.debug("Not using X.509 CRL");
          params.setRevocationEnabled(false);
        } else {
          vlog.debug("Using X.509 CRL " + SecurityClient.x509crl.getValue());
          InputStream crlStream =
            new FileInputStream(SecurityClient.x509crl.getValue());
          Collection<? extends CRL> crls = cf.generateCRLs(crlStream);
          CertStoreParameters csp = new CollectionCertStoreParameters(crls);
          CertStore store = CertStore.getInstance("Collection", csp);
          params.addCertStore(store);
          params.setRevocationEnabled(true);
        }
        tmf = TrustManagerFactory.getInstance("PKIX");
        tmf.init(new CertPathTrustManagerParameters(params));
        tm = (X509TrustManager)tmf.getTrustManagers()[0];
      } catch (java.lang.Exception e) {
        throw new SystemException(e.getMessage());
      }
    }

    public void checkClientTrusted(X509Certificate[] chain, String authType)
      throws CertificateException {
      tm.checkClientTrusted(chain, authType);
    }

    public void checkServerTrusted(X509Certificate[] chain, String authType)
      throws CertificateException {
      MessageDigest md = null;
      try {
        md = MessageDigest.getInstance("SHA-1");
        verifyHostname(chain[0]);
        tm.checkServerTrusted(chain, authType);
      } catch (java.lang.Exception e) {
        if (e.getCause() instanceof CertPathBuilderException) {
          Object[] answer = { "YES", "NO" };
          X509Certificate cert = chain[0];
          md.update(cert.getEncoded());
          String thumbprint =
            DatatypeConverter.printHexBinary(md.digest());
          thumbprint = thumbprint.replaceAll("..(?!$)", "$0 ");
          int ret = JOptionPane.showOptionDialog(null,
            "This certificate has been signed by an unknown authority\n" +
            "\n" +
            "  Subject: " + cert.getSubjectX500Principal().getName() + "\n" +
            "  Issuer: " + cert.getIssuerX500Principal().getName() + "\n" +
            "  Serial Number: " + cert.getSerialNumber() + "\n" +
            "  Version: " + cert.getVersion() + "\n" +
            "  Signature Algorithm: " + cert.getPublicKey().getAlgorithm() +
            "\n" +
            "  Not Valid Before: " + cert.getNotBefore() + "\n" +
            "  Not Valid After: " + cert.getNotAfter() + "\n" +
            "  SHA1 Fingerprint: " + thumbprint + "\n" + "\n" +
            "Do you want to save it and continue?",
            "Certificate Issuer Unknown",
            JOptionPane.YES_NO_OPTION, JOptionPane.WARNING_MESSAGE,
            null, answer, answer[0]);
          if (ret == JOptionPane.YES_OPTION) {
            Collection<? extends X509Certificate> cacerts = null;
            File vncDir = new File(FileUtils.getVncHomeDir());
            File caFile = new File(vncDir, "x509_savedcerts.pem");
            try {
              if (!vncDir.exists())
                vncDir.mkdir();
              if (!caFile.createNewFile()) {
                vlog.error("Certificate save failed.");
                return;
              }
            } catch (java.lang.Exception e2) {
              // skip save if security settings prohibit access to filesystem
              vlog.error("Certificate save failed: " + e2.getMessage());
              return;
            }
            InputStream caStream = new MyFileInputStream(caFile);
            CertificateFactory cf =
              CertificateFactory.getInstance("X.509");
            cacerts = (Collection<? extends X509Certificate>)
              cf.generateCertificates(caStream);
            for (int i = 0; i < chain.length; i++) {
              if (cacerts == null || !cacerts.contains(chain[i])) {
                byte[] der = chain[i].getEncoded();
                String pem = DatatypeConverter.printBase64Binary(der);
                pem = pem.replaceAll("(.{64})", "$1\n");
                FileWriter fw = null;
                try {
                  fw = new FileWriter(caFile.getAbsolutePath(), true);
                  fw.write("-----BEGIN CERTIFICATE-----\n");
                  fw.write(pem + "\n");
                  fw.write("-----END CERTIFICATE-----\n");
                } catch (IOException ioe) {
                  throw new SystemException(ioe.getMessage());
                } finally {
                  try {
                    if (fw != null)
                      fw.close();
                  } catch (IOException ioe2) {
                    throw new SystemException(ioe2.getMessage());
                  }
                }
              }
            }
          } else {
            throw new WarningException("X.509 certificate not trusted");
          }
        } else {
          throw new SystemException(e.getMessage());
        }
      }
    }

    public X509Certificate[] getAcceptedIssuers() {
      return tm.getAcceptedIssuers();
    }

    private void verifyHostname(X509Certificate cert)
      throws CertificateParsingException {
      try {
        Collection sans = cert.getSubjectAlternativeNames();
        if (sans == null) {
          String dn = cert.getSubjectX500Principal().getName();
          LdapName ln = new LdapName(dn);
          for (Rdn rdn : ln.getRdns()) {
            if (rdn.getType().equalsIgnoreCase("CN")) {
              String peer =
                ((CConn)client).getSocket().getPeerName().toLowerCase();
              if (peer.equals(((String)rdn.getValue()).toLowerCase()))
                return;
            }
          }
        } else {
          Iterator i = sans.iterator();
          while (i.hasNext()) {
            List nxt = (List)i.next();
            if (((Integer)nxt.get(0)).intValue() == 2) {
              String peer =
                ((CConn)client).getSocket().getPeerName().toLowerCase();
              if (peer.equals(((String)nxt.get(1)).toLowerCase()))
                return;
            } else if (((Integer)nxt.get(0)).intValue() == 7) {
              String peer = ((CConn)client).getSocket().getPeerAddress();
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
        throw new SystemException(e.getMessage());
      } catch (InvalidNameException e) {
        throw new SystemException(e.getMessage());
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
        } catch (java.lang.Exception e) {
          throw new SystemException(e.toString());
        } finally {
          try {
            if (reader != null)
              reader.close();
          } catch (IOException ioe) {
            throw new SystemException(ioe.getMessage());
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
