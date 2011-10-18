/* $XConsortium: AuFileName.c /main/8 1996/09/28 16:43:20 rws $ */
/* $XFree86: xc/lib/Xau/AuFileName.c,v 3.2 1996/12/24 08:46:53 dawes Exp $ */

/*

Copyright (c) 1988  X Consortium

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

#include <X11/Xauth.h>
#include <X11/Xos.h>

#ifdef X_NOT_STDC_ENV
char *malloc (), *getenv ();
#else
#include <stdlib.h>
#endif

char *
XauFileName ()
{
    char *slashDotXauthority = "/.Xauthority";
    char    *name;
    static char	*buf;
    static int	bsize;
#ifdef WIN32
    char    dir[128];
#endif
    int	    size;

    if (name = getenv ("XAUTHORITY"))
	return name;
    name = getenv ("HOME");
    if (!name) {
#ifdef WIN32
	(void) strcpy (dir, "/users/");
	if (name = getenv("USERNAME")) {
	    (void) strcat (dir, name);
	    name = dir;
	}
	if (!name)
#endif
	return 0;
    }
    size = strlen (name) + strlen(&slashDotXauthority[1]) + 2;
    if (size > bsize) {
	if (buf)
	    free (buf);
	buf = malloc ((unsigned) size);
	if (!buf)
	    return 0;
	bsize = size;
    }
    strcpy (buf, name);
    strcat (buf, slashDotXauthority + (name[1] == '\0' ? 1 : 0));
    return buf;
}
