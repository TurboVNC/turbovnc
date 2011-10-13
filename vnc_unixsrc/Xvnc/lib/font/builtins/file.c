/*
 * Id: file.c,v 1.2 1999/11/02 06:16:47 keithp Exp $
 *
 * Copyright 1999 SuSE, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of SuSE not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  SuSE makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * SuSE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL SuSE
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, SuSE, Inc.
 */
/* $XFree86: xc/lib/font/builtins/file.c,v 1.3 1999/12/30 02:29:49 robin Exp $ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "builtin.h"

typedef struct _BuiltinIO {
    int		    offset;
    BuiltinFilePtr  file;
} BuiltinIORec, *BuiltinIOPtr;

static int
BuiltinFill (f)
    BufFilePtr	f;
{
    int	    left, len;
    BuiltinIOPtr    io = ((BuiltinIOPtr) f->private);

    left = io->file->len - io->offset;
    if (left <= 0)
    {
	f->left = 0;
	return BUFFILEEOF;
    }
    len = BUFFILESIZE;
    if (len > left)
	len = left;
    bcopy (io->file->bits + io->offset, f->buffer, len);
    io->offset += len;
    f->left = len - 1;
    f->bufp = f->buffer + 1;
    return f->buffer[0];
}

static int
BuiltinSkip (f, count)
    BufFilePtr	f;
    int		count;
{
    BuiltinIOPtr    io = ((BuiltinIOPtr) f->private);
    int	    curoff;
    int	    fileoff;
    int	    todo;
    int	    left;

    curoff = f->bufp - f->buffer;
    fileoff = curoff + f->left;
    if (curoff + count <= fileoff) {
	f->bufp += count;
	f->left -= count;
    } else {
	todo = count - (fileoff - curoff);
	io->offset += todo;
	if (io->offset > io->file->len)
	    io->offset = io->file->len;
	if (io->offset < 0)
	    io->offset = 0;
	f->left = 0;
    }
    return count;
}

static int
BuiltinClose (f, doClose)
    BufFilePtr	f;
{
    BuiltinIOPtr    io = ((BuiltinIOPtr) f->private);
    
    xfree (io);
    return 1;
}


FontFilePtr
BuiltinFileOpen (name)
    char    *name;
{
    int		    i;
    BuiltinIOPtr    io;
    BufFilePtr	    raw, cooked;

    if (*name == '/') name++;
    for (i = 0; i < builtin_files_count; i++)
	if (!strcmp (name, builtin_files[i].name))
	    break;
    if (i == builtin_files_count)
	return NULL;
    io = (BuiltinIOPtr) xalloc (sizeof (BuiltinIORec));
    if (!io)
	return NULL;
    io->offset = 0;
    io->file = (void *) &builtin_files[i];
    raw = BufFileCreate ((char *) io, BuiltinFill, 0, BuiltinSkip, BuiltinClose);
    if (!raw)
    {
	xfree (io);
	return NULL;
    }
    if (cooked = BufFilePushCompressed (raw))
	raw = cooked;
    else
    {
	raw->left += raw->bufp - raw->buffer;
	raw->bufp = raw->buffer;
    }
    return (FontFilePtr) raw;
}

int
BuiltinFileClose (f)
    FontFilePtr	f;
{
    return BufFileClose ((BufFilePtr) f, TRUE);
}
