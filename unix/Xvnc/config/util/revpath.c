/*
 * Copyright 1999 by The XFree86 Project, Inc.
 */
/* $XFree86: xc/config/util/revpath.c,v 1.2 1999/02/01 11:55:49 dawes Exp $ */

/*
 * Reverse a pathname.  It returns a relative path that can be used to undo
 * 'cd argv[1]'.
 *
 * It is impossible to do this in general, but this handles the cases that
 * come up in imake.  Maybe imake should use an absolute path for $(TOP)
 * instead of a relative path so that this problem can be avoided?
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int
main(int argc, char *argv[])
{
    int levels = 0;
    char *p;

    /* Silently ignore invalid usage */
    if (argc != 2)
	exit(0);

    /* Split the path and count the levels */
    p = strtok(argv[1], "/");
    while (p) {
	if (strcmp(p, ".") == 0)
	    ;
	else if (strcmp(p, "..") == 0)
	    levels--;
	else
	    levels++;
	p = strtok(NULL, "/");
    }

    while (levels-- > 0)
	printf("../");

    printf("\n");

    exit(0);
}
