/* $XConsortium: bufio.c,v 1.8 94/04/17 20:17:00 gildea Exp $ */
/* $XFree86: xc/lib/font/fontfile/bufio.c,v 3.0 1994/12/17 09:41:39 dawes Exp $ */

/*

Copyright (c) 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

/*
 * Author:  Keith Packard, MIT X Consortium
 */


#include <X11/Xos.h>
#include <fontmisc.h>
#include <bufio.h>
#include <errno.h>
#ifdef X_NOT_STDC_ENV
extern int errno;
#endif

BufFilePtr
BufFileCreate (private, io, skip, close)
    char    *private;
    int	    (*io)();
    int	    (*skip)();
    int	    (*close)();
{
    BufFilePtr	f;

    f = (BufFilePtr) xalloc (sizeof *f);
    if (!f)
	return 0;
    f->private = private;
    f->bufp = f->buffer;
    f->left = 0;
    f->io = io;
    f->skip = skip;
    f->close = close;
    return f;
}

#define FileDes(f)  ((int) (f)->private)

static int
BufFileRawFill (f)
    BufFilePtr	f;
{
    int	left;

    left = read (FileDes(f), (char *)f->buffer, BUFFILESIZE);
    if (left <= 0) {
	f->left = 0;
	return BUFFILEEOF;
    }
    f->left = left - 1;
    f->bufp = f->buffer + 1;
    return f->buffer[0];
}

static int
BufFileRawSkip (f, count)
    BufFilePtr	f;
    int		count;
{
    int	    curoff;
    int	    fileoff;
    int	    todo;

    curoff = f->bufp - f->buffer;
    fileoff = curoff + f->left;
    if (curoff + count <= fileoff) {
	f->bufp += count;
	f->left -= count;
    } else {
	todo = count - (fileoff - curoff);
	if (lseek (FileDes(f), todo, 1) == -1) {
	    if (errno != ESPIPE)
		return BUFFILEEOF;
	    while (todo) {
		curoff = BUFFILESIZE;
		if (curoff > todo)
		    curoff = todo;
		fileoff = read (FileDes(f), (char *)f->buffer, curoff);
		if (fileoff <= 0)
		    return BUFFILEEOF;
		todo -= fileoff;
	    }
	}
	f->left = 0;
    }
    return count;
}

static int
BufFileRawClose (f, doClose)
    BufFilePtr	f;
{
    if (doClose)
	close (FileDes (f));
    return 1;
}

BufFilePtr
BufFileOpenRead (fd)
    int	fd;
{
#ifdef __EMX__
    /* hv: I'd bet WIN32 has the same effect here */
    setmode(fd,O_BINARY);
#endif
    return BufFileCreate ((char *) fd, BufFileRawFill, BufFileRawSkip, BufFileRawClose);
}

static
BufFileRawFlush (c, f)
    int		c;
    BufFilePtr	f;
{
    int	cnt;

    if (c != BUFFILEEOF)
	*f->bufp++ = c;
    cnt = f->bufp - f->buffer;
    f->bufp = f->buffer;
    f->left = BUFFILESIZE;
    if (write (FileDes(f), (char *)f->buffer, cnt) != cnt)
	return BUFFILEEOF;
    return c;
}

BufFilePtr
BufFileOpenWrite (fd)
    int	fd;
{
    BufFilePtr	f;

#ifdef __EMX__
    /* hv: I'd bet WIN32 has the same effect here */
    setmode(fd,O_BINARY);
#endif
    f = BufFileCreate ((char *) fd, BufFileRawFlush, 0, BufFileFlush);
    f->bufp = f->buffer;
    f->left = BUFFILESIZE;
    return f;
}

BufFileRead (f, b, n)
    BufFilePtr	f;
    char	*b;
    int		n;
{
    int	    c, cnt;
    cnt = n;
    while (cnt--) {
	c = BufFileGet (f);
	if (c == BUFFILEEOF)
	    break;
	*b++ = c;
    }
    return n - cnt - 1;
}

BufFileWrite (f, b, n)
    BufFilePtr	f;
    char	*b;
    int		n;
{
    int	    cnt;
    cnt = n;
    while (cnt--) {
	if (BufFilePut (*b++, f) == BUFFILEEOF)
	    return BUFFILEEOF;
    }
    return n;
}

int
BufFileFlush (f)
    BufFilePtr	f;
{
    if (f->bufp != f->buffer)
	(*f->io) (BUFFILEEOF, f);

    return 0;
}

int
BufFileClose (f, doClose)
    BufFilePtr	f;
{
    (void) (*f->close) (f, doClose);
    xfree (f);

    return 0;
}

int
BufFileFree (f)
    BufFilePtr	f;
{
    xfree (f);

    return 0;
}
