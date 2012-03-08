/*
 * $Xorg: RAofA8.c,v 1.6 2001/02/09 02:03:48 xorgcvs Exp $
 *
 * 
Copyright 1989, 1998  The Open Group

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
 * *
 * Author:  Keith Packard, MIT X Consortium
 */

#include <X11/Xos.h>
#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xdmcp.h>

int
XdmcpReadARRAYofARRAY8 (buffer, array)
    XdmcpBufferPtr	buffer;
    ARRAYofARRAY8Ptr	array;
{
    int	    i;

    if (!XdmcpReadCARD8 (buffer, &array->length)) {

	/* Must set array->data to NULL to guarantee safe call of
 	 * XdmcpDisposeARRAY*(array) (which calls Xfree(array->data));
         * see defect 7329 */
	array->data = 0;
	return FALSE;
    }
    if (!array->length)
    {
	array->data = NULL;
	return TRUE;
    }
    array->data = (ARRAY8 *) Xalloc (array->length * sizeof (ARRAY8));
    if (!array->data)
	return FALSE;
    for (i = 0; i < (int)array->length; i++)
    {
	if (!XdmcpReadARRAY8 (buffer, &array->data[i]))
	{

	    /* All arrays allocated thus far in the loop must be freed
	     * if there is an error in the read.
             * See Defect 7328 */
	    array->length = i;
	    XdmcpDisposeARRAYofARRAY8(array);
	    return FALSE;
	}
    }
    return TRUE;
}
