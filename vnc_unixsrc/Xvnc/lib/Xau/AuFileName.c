/* $Xorg: AuFileName.c,v 1.5 2001/02/09 02:03:42 xorgcvs Exp $ */

/*

Copyright 1988, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/
/* $XFree86: xc/lib/Xau/AuFileName.c,v 3.6 2001/07/25 15:04:48 dawes Exp $ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <X11/Xauth.h>
#include <X11/Xos.h>
#include <stdlib.h>

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

    if ((name = getenv ("XAUTHORITY")))
	return name;
    name = getenv ("HOME");
    if (!name) {
#ifdef WIN32
	(void) strcpy (dir, "/users/");
	if ((name = getenv("USERNAME"))) {
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
