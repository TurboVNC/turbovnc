/*
 * Copyright (c) 2011 ymnk, JCraft,Inc. All rights reserved.
 * Copyright (c) 2018, 2025 D. R. Commander. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions
 * and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided with
 * the distribution.
 *
 * 3. The names of the authors may not be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL JCRAFT, INC. OR ANY CONTRIBUTORS TO THIS SOFTWARE BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package com.jcraft.jsch;

class AgentIdentity implements Identity {

  private AgentProxy agent;
  private byte[] blob;
  private String comment;
  private String algname;
  private HASH hash;

  AgentIdentity(AgentProxy agent, byte[] blob, String comment) {
    this.agent = agent;
    this.blob = blob;
    this.comment = comment;
    algname = Util.byte2str((new Buffer(blob)).getString());
  }

  private HASH genHash() {
    try {
      Class c = Class.forName(JSch.getConfig("md5"));
      hash = (HASH)(c.getDeclaredConstructor().newInstance());
      hash.init();
    } catch (Exception e) {
    }
    return hash;
  }

  @Override
  public boolean setPassphrase(byte[] passphrase) throws JSchException {
    return true;
  }

  @Override
  public byte[] getPublicKeyBlob() {
    return blob;
  }

  @Override
  public String getFingerPrint() {
    if (hash == null) hash = genHash();
    if (blob == null) return null;
    return Util.getFingerPrint(hash, blob, false, true);
  }

  @Override
  public byte[] getSignature(byte[] data) {
    return agent.sign(blob, data, null);
  }

  @Override
  public byte[] getSignature(byte[] data, String alg) {
    return agent.sign(blob, data, alg);
  }

  @Override
  public String getAlgName() {
    return algname;
  }

  @Override
  public String getName() {
    return comment;
  }

  @Override
  public boolean isEncrypted() {
    return false;
  }

  @Override
  public void clear() {}
}
