/* -*-mode:java; c-basic-offset:2; indent-tabs-mode:nil -*- */
/*
Copyright (c) 2020-2021 Matthias Wiedemann. All rights reserved.
Copyright (c) 2022 D. R. Commander. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright 
     notice, this list of conditions and the following disclaimer in 
     the documentation and/or other materials provided with the distribution.

  3. The names of the authors may not be used to endorse or promote products
     derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL JCRAFT,
INC. OR ANY CONTRIBUTORS TO THIS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

package com.jcraft.jsch;

import org.mindrot.jbcrypt.BCrypt;

import java.util.Arrays;

/**
 * A {@link KeyPair} which can only reveal its type and content after it was decrypted using {@link com.jcraft.jsch.KeyPairDeferred#decrypt(byte[])}.
 * This is needed for openssh-v1-private-key format.
 */
public class KeyPairDeferred extends KeyPair {

    private KeyPair delegate;

    KeyPairDeferred(JSch jsch) {
        super(jsch);
    }

    @Override
    public boolean decrypt(byte[] _passphrase) {
        try {
            if (!encrypted) {
                return true;
            }
            if (_passphrase == null) {
                JSch.getLogger().log(Logger.ERROR, "no passphrase set.");
                return false;
            }

            initCipher(_passphrase);

            byte[] plain = new byte[data.length];
            cipher.update(data, 0, data.length, plain, 0);

            // now we have decrypted key and can determine type
            int type = readOpenSSHKeyv1(plain);

            delegate = getKeyPair(jsch, null, null, null, false, plain, getPublicKeyBlob(), type, VENDOR_OPENSSH_V1, publicKeyComment, cipher, null, null);

            return delegate != null;


        } catch (Exception e) {
            throw new IllegalArgumentException("Could not sucessfully decrypt openssh v1 key", e);
        }

    }

    private void initCipher(byte[] _passphrase) throws Exception {

        // the encrypted private key is here:
        if ("bcrypt".equals(kdfName)) {
            Buffer opts = new Buffer(kdfOptions);

            byte[] keyiv = new byte[48];

            new BCrypt().pbkdf(_passphrase, opts.getString(), opts.getInt(), keyiv);

            Arrays.fill(_passphrase, (byte) 0);
            byte[] key = Arrays.copyOfRange(keyiv, 0, 32);
            byte[] iv = Arrays.copyOfRange(keyiv, 32, 48);
            cipher.init(Cipher.DECRYPT_MODE, key, iv);
        } else {
            throw new IllegalStateException("No support for KDF '" + kdfName + "'.");
        }
    }

    @Override
    void generate(int key_size) throws JSchException {
        throw new UnsupportedOperationException();
    }

    @Override
    byte[] getBegin() {
        return requireDecrypted(delegate).getBegin();
    }

    @Override
    byte[] getEnd() {
        return requireDecrypted(delegate).getEnd();
    }

    @Override
    int getKeySize() {
        return requireDecrypted(delegate).getKeySize();
    }

    @Override
    public byte[] getSignature(byte[] data) {
        return requireDecrypted(delegate).getSignature(data);
    }

    @Override
    public byte[] getSignature(byte[] data, String alg) {
        return delegate.getSignature(data, alg);
    }

    @Override
    public Signature getVerifier() {
        return requireDecrypted(delegate).getVerifier();
    }

    @Override
    public Signature getVerifier(String alg) {
        return delegate.getVerifier(alg);
    }

    @Override
    public byte[] forSSHAgent() throws JSchException {
        return requireDecrypted(delegate).forSSHAgent();
    }

    @Override
    byte[] getPrivateKey() {
        return requireDecrypted(delegate).getPrivateKey();
    }

    @Override
    byte[] getKeyTypeName() {
        return requireDecrypted(delegate).getKeyTypeName();
    }

    @Override
    public int getKeyType() {
        return requireDecrypted(delegate).getKeyType();
    }

    @Override
    boolean parse(byte[] data) {
        return requireDecrypted(delegate).parse(data);
    }

    @Override
    public byte[] getPublicKeyBlob() {
        return delegate != null ? delegate.getPublicKeyBlob() :
                                  super.getPublicKeyBlob();
    }

    @Override
    public String getPublicKeyComment() {
        return requireDecrypted(delegate).getPublicKeyComment();
    }

    @Override
    public String getFingerPrint() {
        return delegate != null ? delegate.getFingerPrint() :
                                  super.getFingerPrint();
    }

    @Override
    public boolean isEncrypted() {
        return delegate != null ? delegate.isEncrypted() : super.isEncrypted();
    }

    private <T> T requireDecrypted(T obj) {
        if (obj == null)
            throw new IllegalStateException("encrypted key has not been decrypted yet.");
        return obj;
    }
}
