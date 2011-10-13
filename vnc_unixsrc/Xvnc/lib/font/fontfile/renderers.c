/* $Xorg: renderers.c,v 1.4 2001/02/09 02:04:03 xorgcvs Exp $ */

/*

Copyright 1991, 1998  The Open Group

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
/* $XFree86: xc/lib/font/fontfile/renderers.c,v 1.7 2002/12/09 17:30:00 dawes Exp $ */

/*
 * Author:  Keith Packard, MIT X Consortium
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <X11/fonts/fntfilst.h>
extern void ErrorF(const char *f, ...);

static FontRenderersRec	renderers;

/*
 * XXX Maybe should allow unregistering renders. For now, just clear the
 * list at each new generation.
 */
extern unsigned long serverGeneration;
static unsigned long rendererGeneration = 0;

Bool
FontFileRegisterRenderer (FontRendererPtr renderer)
{
    return FontFilePriorityRegisterRenderer(renderer, 0);
}

Bool
FontFilePriorityRegisterRenderer (FontRendererPtr renderer, int priority)
{
    int		    i;
    struct _FontRenderersElement *new;

    if (rendererGeneration != serverGeneration) {
	rendererGeneration = serverGeneration;
	renderers.number = 0;
	if (renderers.renderers)
	   xfree(renderers.renderers);
	renderers.renderers = NULL;
    }

    for (i = 0; i < renderers.number; i++) {
	if (!strcmp (renderers.renderers[i].renderer->fileSuffix, 
                     renderer->fileSuffix)) {
            if(renderers.renderers[i].priority >= priority) {
                if(renderers.renderers[i].priority == priority) {
                    if (rendererGeneration == 1)
                        ErrorF("Warning: font renderer for \"%s\" "
                               "already registered at priority %d\n",
                               renderer->fileSuffix, priority);
                }
                return TRUE;
            } else {
                break;
            }
        }
    }

    if(i >= renderers.number) {
        new = xrealloc (renderers.renderers, sizeof(*new) * (i + 1));
        if (!new)
            return FALSE;
        renderers.renderers = new;
        renderers.number = i + 1;
    }
    renderer->number = i;
    renderers.renderers[i].renderer = renderer;
    renderers.renderers[i].priority = priority;
    return TRUE;
}

FontRendererPtr
FontFileMatchRenderer (char *fileName)
{
    int			i;
    int			fileLen;
    FontRendererPtr	r;
    
    fileLen = strlen (fileName);
    for (i = 0; i < renderers.number; i++)
    {
	r = renderers.renderers[i].renderer;
	if (fileLen >= r->fileSuffixLen &&
	    !strcmp (fileName + fileLen - r->fileSuffixLen, r->fileSuffix))
	{
	    return r;
	}
    }
    return 0;
}
