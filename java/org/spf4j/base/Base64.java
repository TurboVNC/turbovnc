/*
 * Copyright (c) 2001-2017, Zoltan Farkas All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 *
 * Additionally licensed with:
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package org.spf4j.base;

import java.io.IOException;
import java.nio.CharBuffer;

/**
 * "improved" implementation based on DataTypeConverterImpl performance should be same/slightly faster than the JDK
 * equivalent But most importantly you can encode/decode parts of a String, which should reduce the need of copying
 * objects and reduce the amount of garbage created.
 *
 * @author zoly
 */
public final class Base64 {

  private static final char[] ENCODE_MAP = initEncodeMap();

  private static final byte PADDING = 127;

  private Base64() {
  }

  private static char[] initEncodeMap() {
    char[] map = new char[64];
    int i;
    for (i = 0; i < 26; i++) {
      map[i] = (char) ('A' + i);
    }
    for (i = 26; i < 52; i++) {
      map[i] = (char) ('a' + (i - 26));
    }
    for (i = 52; i < 62; i++) {
      map[i] = (char) ('0' + (i - 52));
    }
    map[62] = '+';
    map[63] = '/';

    return map;
  }

  public static char encode(final int i) {
    return ENCODE_MAP[i & 0x3F];
  }

  public static byte encodeByte(final int i) {
    return (byte) ENCODE_MAP[i & 0x3F];
  }

  public static String encodeBase64(final byte[] input) {
    return encodeBase64(input, 0, input.length);
  }

  public static String encodeBase64(final byte[] input, final int offset, final int len) {
    char[] buf = new char[(((len + 2) / 3) * 4)];
    int ptr = encodeBase64(input, offset, len, buf, 0);
    return new String(buf, 0, ptr);
  }

  /**
   * Alternate implementation, should be better for large data.
   *
   * @param input - the byte array to encode
   * @param offset - the index of the first byte that is to be encoded.
   * @param len - the number of bytes to encode.
   * @return - the encoded String.
   */
  public static CharSequence encodeBase64V2(final byte[] input, final int offset, final int len) {
    char[] buf = new char[(((len + 2) / 3) * 4)];
    int ptr = encodeBase64(input, offset, len, buf, 0);
    assert ptr == buf.length;
    return CharBuffer.wrap(buf);
  }

  public static void encodeBase64(final byte[] input, final int offset, final int len, final Appendable result)
          throws IOException {
    for (int i = offset; i < len; i += 3) {
      switch (len - i) {
        case 1:
          result.append(encode(input[i] >> 2));
          result.append(encode(((input[i]) & 0x3) << 4));
          result.append("==");
          break;
        case 2:
          result.append(encode(input[i] >> 2));
          result.append(encode(
                  ((input[i] & 0x3) << 4)
                  | ((input[i + 1] >> 4) & 0xF)));
          result.append(encode((input[i + 1] & 0xF) << 2));
          result.append('=');
          break;
        default:
          result.append(encode(input[i] >> 2));
          result.append(encode(
                  ((input[i] & 0x3) << 4)
                  | ((input[i + 1] >> 4) & 0xF)));
          result.append(encode(
                  ((input[i + 1] & 0xF) << 2)
                  | ((input[i + 2] >> 6) & 0x3)));
          result.append(encode(input[i + 2] & 0x3F));
          break;
      }
    }
  }

  /**
   * Encodes a byte array into a char array by doing base64 encoding.
   *
   * The caller must supply a big enough buffer.
   *
   * @param input - the byte array to encode.
   * @param offset - the index of the first byte to encode.
   * @param len - the number of bytes to encode.
   * @param output - the destination character array to encode to.
   * @param cptr - the index of the first character to encode to.
   * @return the value of {@code ptr+((len+2)/3)*4}, which is the new offset in the output buffer where the further
   * bytes should be placed.
   */
  public static int encodeBase64(final byte[] input, final int offset,
          final int len, final char[] output, final int cptr) {
    int ptr = cptr;
    for (int i = offset; i < len; i += 3) {
      switch (len - i) {
        case 1:
          output[ptr++] = encode(input[i] >> 2);
          output[ptr++] = encode(((input[i]) & 0x3) << 4);
          output[ptr++] = '=';
          output[ptr++] = '=';
          break;
        case 2:
          output[ptr++] = encode(input[i] >> 2);
          output[ptr++] = encode(
                  ((input[i] & 0x3) << 4)
                  | ((input[i + 1] >> 4) & 0xF));
          output[ptr++] = encode((input[i + 1] & 0xF) << 2);
          output[ptr++] = '=';
          break;
        default:
          output[ptr++] = encode(input[i] >> 2);
          output[ptr++] = encode(
                  ((input[i] & 0x3) << 4)
                  | ((input[i + 1] >> 4) & 0xF));
          output[ptr++] = encode(
                  ((input[i + 1] & 0xF) << 2)
                  | ((input[i + 2] >> 6) & 0x3));
          output[ptr++] = encode(input[i + 2] & 0x3F);
          break;
      }
    }
    return ptr;
  }

  /**
   * Encodes a byte array into another byte array by first doing base64 encoding then encoding the result in ASCII.
   *
   * The caller must supply a big enough buffer.
   *
   * @param input - the byte array to encode.
   * @param offset - the index of the first byte to encode.
   * @param len - the number of bytes to encode.
   * @param out - the destination byte array that represents an ASCII string to encode to.
   * @param cptr - the index of the first byte in the destination array to encode to.
   * @return the value of {@code ptr+((len+2)/3)*4}, which is the new offset in the output buffer where the further
   * bytes should be placed.
   */
  public static int encodeBase64(final byte[] input, final int offset, final int len,
          final byte[] out, final int cptr) {
    int ptr = cptr;
    byte[] buf = out;
    int max = len + offset;
    for (int i = offset; i < max; i += 3) {
      switch (max - i) {
        case 1:
          buf[ptr++] = encodeByte(input[i] >> 2);
          buf[ptr++] = encodeByte(((input[i]) & 0x3) << 4);
          buf[ptr++] = '=';
          buf[ptr++] = '=';
          break;
        case 2:
          buf[ptr++] = encodeByte(input[i] >> 2);
          buf[ptr++] = encodeByte(
                  ((input[i] & 0x3) << 4)
                  | ((input[i + 1] >> 4) & 0xF));
          buf[ptr++] = encodeByte((input[i + 1] & 0xF) << 2);
          buf[ptr++] = '=';
          break;
        default:
          buf[ptr++] = encodeByte(input[i] >> 2);
          buf[ptr++] = encodeByte(
                  ((input[i] & 0x3) << 4)
                  | ((input[i + 1] >> 4) & 0xF));
          buf[ptr++] = encodeByte(
                  ((input[i + 1] & 0xF) << 2)
                  | ((input[i + 2] >> 6) & 0x3));
          buf[ptr++] = encodeByte(input[i + 2] & 0x3F);
          break;
      }
    }

    return ptr;
  }

}
