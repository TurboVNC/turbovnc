/* $XConsortium: bufio.h /main/5 1996/11/03 19:32:07 kaleb $ */

/*

Copyright (c) 1993  X Consortium

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

#ifdef TEST

#define xalloc(s)   malloc(s)
#define xfree(s)    free(s)

#endif

#define BUFFILESIZE	8192
#define BUFFILEEOF	-1

typedef unsigned char BufChar;

typedef struct _buffile {
    BufChar *bufp;
    int	    left;
    BufChar buffer[BUFFILESIZE];
    int	    (*io)(/* BufFilePtr f */);
    int	    (*skip)(/* BufFilePtr f, int count */);
    int	    (*close)(/* BufFilePtr f */);
    char    *private;
} BufFileRec, *BufFilePtr;

extern BufFilePtr   BufFileCreate ();
extern BufFilePtr   BufFileOpenRead (), BufFileOpenWrite ();
extern BufFilePtr   BufFilePushCompressed ();
#ifdef X_GZIP_FONT_COMPRESSION
extern BufFilePtr   BufFilePushZIP ();
#endif
extern int	    BufFileClose ();
extern int	    BufFileFlush ();
#define BufFileGet(f)	((f)->left-- ? *(f)->bufp++ : (*(f)->io) (f))
#define BufFilePut(c,f)	(--(f)->left ? *(f)->bufp++ = (c) : (*(f)->io) (c,f))
#define BufFileSkip(f,c)    ((*(f)->skip) (f, c))

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
