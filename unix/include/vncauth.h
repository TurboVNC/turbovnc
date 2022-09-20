/* Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
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

#define MAXPWLEN 8
#define CHALLENGESIZE 16

extern int vncEncryptAndStorePasswd(char *passwd, char *fname);
extern int vncEncryptAndStorePasswd2(char *passwd, char *passwdViewOnly,
                                     char *fname);
extern char *vncDecryptPasswdFromFile(char *fname);
extern int vncDecryptPasswdFromFile2(char *fname, char *passwdFullControl,
                                     char *passwdViewOnly);
extern int vncDecryptPasswd(char *encryptedPasswd, char *decryptedPasswd);
extern void vncRandomBytes(unsigned char *bytes);
extern void vncEncryptBytes(unsigned char *bytes, char *passwd);
