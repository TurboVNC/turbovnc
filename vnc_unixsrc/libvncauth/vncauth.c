/*
 *  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 *  USA.
 */

/*
 * vncauth.c - Functions for VNC password management and authentication.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vncauth.h>
#include <d3des.h>


/*
 * Make sure we call srandom() only once.
 */

static int s_srandom_called = 0;

/*
 * We use a fixed key to store passwords, since we assume that our local
 * file system is secure but nonetheless don't want to store passwords
 * as plaintext.
 */

static unsigned char s_fixedkey[8] = {23,82,107,6,35,78,88,7};


/*
 * Encrypt a password and store it in a file.  Returns 0 if successful,
 * 1 if the file could not be written.
 *
 * NOTE: This function is preserved only for compatibility with the original
 * AT&T VNC software.  Use vncEncryptAndStorePasswd2() instead.
 */

int
vncEncryptAndStorePasswd(char *passwd, char *fname)
{
    return (vncEncryptAndStorePasswd2(passwd, NULL, fname) == 0);
}

/*
 * Encrypt one or two passwords and store them in a file.  Returns 1 if
 * successful, 0 if the file could not be written (note that the original
 * vncEncryptAndStorePasswd() function returns inverse values).  The
 * passwdViewOnly pointer may be NULL.
 *
 * NOTE: The file name of "-" denotes stdout.
 */

int
vncEncryptAndStorePasswd2(char *passwd, char *passwdViewOnly, char *fname)
{
    FILE *fp;
    int i, bytesToWrite, bytesWrote;
    unsigned char encryptedPasswd[16] = {
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0
    };

    if (strcmp(fname, "-") != 0) {
      fp = fopen(fname, "w");
      if (fp == NULL) {
	return 0;
      }
      chmod(fname, S_IRUSR|S_IWUSR);
    } else {
      fp = stdout;
    }

    strncpy(encryptedPasswd, passwd, 8);
    if (passwdViewOnly != NULL)
	strncpy(encryptedPasswd + 8, passwdViewOnly, 8);

    /* Do encryption in-place - this way we overwrite our copies of
       plaintext passwords. */

    deskey(s_fixedkey, EN0);
    des(encryptedPasswd, encryptedPasswd);
    if (passwdViewOnly != NULL)
	des(encryptedPasswd + 8, encryptedPasswd + 8);

    bytesToWrite = (passwdViewOnly == NULL) ? 8 : 16;
    bytesWrote = fwrite(encryptedPasswd, 1, bytesToWrite, fp);
  
    if (fp != stdout) {
      fclose(fp);
    }
    return (bytesWrote == bytesToWrite);
}


/*
 * Decrypt a password from a file.  Returns a pointer to a newly allocated
 * string containing the password or a null pointer if the password could
 * not be retrieved for some reason.
 *
 * NOTE: This function is preserved only for compatibility with the original
 * AT&T VNC software.  Use vncDecryptPasswdFromFile2() instead.
 */

char *
vncDecryptPasswdFromFile(char *fname)
{
    char *passwd;

    passwd = malloc(9);

    if (passwd != NULL) {
	if (vncDecryptPasswdFromFile2(fname, passwd, NULL) == 0) {
	    free(passwd);
	    passwd = NULL;
	}
    }

    return passwd;
}

/*
 * Decrypt one or two passwords from a file.  Returns the number of
 * passwords read (1, 2, or 0 on error).  On success, the passwords are
 * written into buffers passwdFullControl[] and passwdViewOnly[] if
 * they are not NULL.  If the pointers to buffers are not NULL, then
 * the buffers should be at least of 9 bytes length.
 */

int
vncDecryptPasswdFromFile2(char *fname,
			  char *passwdFullControl, char *passwdViewOnly)
{
    FILE *fp;
    int i, ch;
    char passwd[16];

    if (strcmp(fname, "-") != 0) {
	if ((fp = fopen(fname,"r")) == NULL)
	    return 0;		/* Could not open the file */
    } else {
	fp = stdin;
    }

    for (i = 0; i < 16; i++) {
	ch = getc(fp);
	if (ch == EOF)
	    break;
	passwd[i] = ch;
    }

    if (fp != stdin)
	fclose(fp);

    if (i < 8)
	return 0;		/* Could not read eight bytes */

    deskey(s_fixedkey, DE1);

    /* Decoding first (full-control) password */
    if (passwdFullControl != NULL) {
	des(passwd, passwd);
	memcpy(passwdFullControl, passwd, 8);
	passwdFullControl[8] = '\0';
    }

    /* Decoding second (view-only) password if available */
    if (i == 16 && passwdViewOnly != NULL) {
	des(&passwd[8], &passwd[8]);
	memcpy(passwdViewOnly, &passwd[8], 8);
	passwdViewOnly[8] = '\0';
    }

    /* Destroying our copy of clear-text passwords */
    memset(passwd, 0, 16);

    return (i < 16) ? 1 : 2;
}


/*
 * Generate CHALLENGESIZE random bytes for use in challenge-response
 * authentication.
 */

void
vncRandomBytes(unsigned char *bytes)
{
    int i;
    unsigned int seed;

    if (!s_srandom_called) {
      seed = (unsigned int)time(0) ^ (unsigned int)getpid();
      srandom(seed);
      s_srandom_called = 1;
    }

    for (i = 0; i < CHALLENGESIZE; i++) {
	bytes[i] = (unsigned char)(random() & 255);    
    }
}


/*
 * Encrypt CHALLENGESIZE bytes in memory using a password.
 */

void
vncEncryptBytes(unsigned char *bytes, char *passwd)
{
    unsigned char key[8];
    int i;

    /* key is simply password padded with nulls */

    for (i = 0; i < 8; i++) {
	if (i < strlen(passwd)) {
	    key[i] = passwd[i];
	} else {
	    key[i] = 0;
	}
    }

    deskey(key, EN0);

    for (i = 0; i < CHALLENGESIZE; i += 8) {
	des(bytes+i, bytes+i);
    }
}
