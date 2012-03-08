/* $XConsortium: chownxterm.c,v 1.3 94/04/17 20:10:41 gildea Exp $ */
/*

Copyright (c) 1993, 1994  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/

/*
 * chownxterm --- make xterm suid root
 *
 * By Stephen Gildea, December 1993
 */


#define XTERM_PATH "/x11/programs/xterm/xterm"

#include <stdio.h>
#include <errno.h>

char *prog_name;

void help()
{
    setgid(getgid());
    setuid(getuid());
    printf("chown-xterm makes %s suid root\n", XTERM_PATH);
    printf("This is necessary on Ultrix for /dev/tty operation.\n");
    exit(0);
}

void print_error(err_string)
    char *err_string;
{
    setgid(getgid());
    setuid(getuid());
    fprintf(stderr, "%s: \"%s\"", prog_name, err_string);
    perror(" failed");
    exit(1);
}

main(argc, argv)
    int argc;
    char **argv;
{
    prog_name = argv[0];
    if (argc >= 2 && strcmp(argv[1], "-help") == 0) {
	help();
    } else {
	if (chown(XTERM_PATH, 0, -1) != 0)
	    print_error("chown root " XTERM_PATH);
	if (chmod(XTERM_PATH, 04555) != 0)
	    print_error("chmod 4555 " XTERM_PATH);
    }
    exit(0);
}
