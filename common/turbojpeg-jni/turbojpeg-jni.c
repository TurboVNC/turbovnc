/* Copyright (C) 2011-2012, 2014-2015, 2017-2019, 2021 D. R. Commander.
 *                                                     All Rights Reserved.
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

#ifdef __SUNPRO_C
/* Oracle Developer Studio sometimes erroneously detects the THROW() macro
   followed by a semicolon as an unreachable statement. */
#pragma error_messages(off, E_STATEMENT_NOT_REACHED)
#endif

#include "com_turbovnc_rfb_TightDecoder.h"
#include "turbojpeg.h"


#define BAILIF0(f) { \
  if (!(f) || (*env)->ExceptionCheck(env)) { \
    goto bailout; \
  } \
}

#define BAILIF0NOEC(f) { \
  if (!(f)) { \
    goto bailout; \
  } \
}

#define THROW(msg) {  \
  jclass _exccls = (*env)->FindClass(env, "java/lang/Exception");  \
  BAILIF0(_exccls);  \
  (*env)->ThrowNew(env, _exccls, msg);  \
  goto bailout;  \
}

#define SAFE_RELEASE(javaArray, cArray) {  \
  if (javaArray && cArray)  \
    (*env)->ReleasePrimitiveArrayCritical(env, javaArray, (void *)cArray,  \
                                          0);  \
  cArray = NULL;  \
}


JNIEXPORT jlong JNICALL Java_com_turbovnc_rfb_TightDecoder_tjInitDecompress
  (JNIEnv *env, jobject obj)
{
  tjhandle handle;

  if ((handle = tjInitDecompress()) == NULL)
    THROW(tjGetErrorStr());

bailout:
  return (jlong)handle;
}


static void decompress
  (JNIEnv *env, jobject obj, jlong handle, jbyteArray src, jint jpegSize,
   jarray dst, jint dstElementSize, jint x, jint y, jint width, jint pitch,
   jint height, jint pf, jint flags)
{
  jsize arraySize = 0, actualPitch;
  unsigned char *jpegBuf = NULL, *dstBuf = NULL;

  if (!handle || pf < 0 || pf >= TJ_NUMPF)
    THROW("Invalid argument in tjDecompress()");

  if ((*env)->GetArrayLength(env, src) < jpegSize)
    THROW("Source buffer is not large enough");
  actualPitch = (pitch == 0) ? width * tjPixelSize[pf] : pitch;
  arraySize = (y + height - 1) * actualPitch + (x + width) * tjPixelSize[pf];
  if ((*env)->GetArrayLength(env, dst) * dstElementSize < arraySize)
    THROW("Destination buffer is not large enough");

  BAILIF0NOEC(jpegBuf = (*env)->GetPrimitiveArrayCritical(env, src, 0));
  BAILIF0NOEC(dstBuf = (*env)->GetPrimitiveArrayCritical(env, dst, 0));

  if (tjDecompress2((tjhandle)handle, jpegBuf, (unsigned long)jpegSize,
                    &dstBuf[y * actualPitch + x * tjPixelSize[pf]], width,
                    pitch, height, pf, flags) == -1) {
    SAFE_RELEASE(dst, dstBuf);
    SAFE_RELEASE(src, jpegBuf);
    THROW(tjGetErrorStr());
  }

bailout:
  SAFE_RELEASE(dst, dstBuf);
  SAFE_RELEASE(src, jpegBuf);
}


JNIEXPORT void JNICALL Java_com_turbovnc_rfb_TightDecoder_tjDecompress__J_3BI_3BIIIIIII
  (JNIEnv *env, jobject obj, jlong handle, jbyteArray src, jint jpegSize,
   jbyteArray dst, jint x, jint y, jint width, jint pitch, jint height,
   jint pf, jint flags)
{
  decompress(env, obj, handle, src, jpegSize, dst, 1, x, y, width, pitch,
             height, pf, flags);
}


JNIEXPORT void JNICALL Java_com_turbovnc_rfb_TightDecoder_tjDecompress__J_3BI_3IIIIIIII
  (JNIEnv *env, jobject obj, jlong handle, jbyteArray src, jint jpegSize,
   jintArray dst, jint x, jint y, jint width, jint stride, jint height,
   jint pf, jint flags)
{
  decompress(env, obj, handle, src, jpegSize, dst, sizeof(jint), x, y, width,
             stride * sizeof(jint), height, pf, flags);
}


JNIEXPORT void JNICALL Java_com_turbovnc_rfb_TightDecoder_tjDestroy
  (JNIEnv *env, jobject obj, jlong handle)
{
  if (!handle)
    THROW("Invalid argument in tjDestroy()");

  if (tjDestroy((tjhandle)handle) == -1) THROW(tjGetErrorStr());

bailout:
  return;
}
