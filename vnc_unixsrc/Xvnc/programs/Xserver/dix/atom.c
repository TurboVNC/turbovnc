/* $XFree86: xc/programs/Xserver/dix/atom.c,v 3.3 2001/12/14 19:59:29 dawes Exp $ */
/***********************************************************

Copyright 1987, 1998  The Open Group

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


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* $Xorg: atom.c,v 1.4 2001/02/09 02:04:39 xorgcvs Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xatom.h>
#include "misc.h"
#include "resource.h"
#include "dix.h"

#define InitialTableSize 100

typedef struct _Node {
    struct _Node   *left,   *right;
    Atom a;
    unsigned int fingerPrint;
    char   *string;
} NodeRec, *NodePtr;

static Atom lastAtom = None;
static NodePtr atomRoot = (NodePtr)NULL;
static unsigned long tableLength;
static NodePtr *nodeTable;

void FreeAtom(NodePtr patom);

Atom 
MakeAtom(char *string, unsigned len, Bool makeit)
{
    register    NodePtr * np;
    unsigned i;
    int     comp;
    register unsigned int   fp = 0;

    np = &atomRoot;
    for (i = 0; i < (len+1)/2; i++)
    {
	fp = fp * 27 + string[i];
	fp = fp * 27 + string[len - 1 - i];
    }
    while (*np != (NodePtr) NULL)
    {
	if (fp < (*np)->fingerPrint)
	    np = &((*np)->left);
	else if (fp > (*np)->fingerPrint)
	    np = &((*np)->right);
	else
	{			       /* now start testing the strings */
	    comp = strncmp(string, (*np)->string, (int)len);
	    if ((comp < 0) || ((comp == 0) && (len < strlen((*np)->string))))
		np = &((*np)->left);
	    else if (comp > 0)
		np = &((*np)->right);
	    else
		return(*np)->a;
	    }
    }
    if (makeit)
    {
	register NodePtr nd;

	nd = (NodePtr) xalloc(sizeof(NodeRec));
	if (!nd)
	    return BAD_RESOURCE;
	if (lastAtom < XA_LAST_PREDEFINED)
	{
	    nd->string = string;
	}
	else
	{
	    nd->string = (char *) xalloc(len + 1);
	    if (!nd->string) {
		xfree(nd);
		return BAD_RESOURCE;
	    }
	    strncpy(nd->string, string, (int)len);
	    nd->string[len] = 0;
	}
	if ((lastAtom + 1) >= tableLength) {
	    NodePtr *table;

	    table = (NodePtr *) xrealloc(nodeTable,
					 tableLength * (2 * sizeof(NodePtr)));
	    if (!table) {
		if (nd->string != string)
		    xfree(nd->string);
		xfree(nd);
		return BAD_RESOURCE;
	    }
	    tableLength <<= 1;
	    nodeTable = table;
	}
	*np = nd;
	nd->left = nd->right = (NodePtr) NULL;
	nd->fingerPrint = fp;
	nd->a = (++lastAtom);
	*(nodeTable+lastAtom) = nd;
	return nd->a;
    }
    else
	return None;
}

Bool
ValidAtom(Atom atom)
{
    return (atom != None) && (atom <= lastAtom);
}

char *
NameForAtom(Atom atom)
{
    NodePtr node;
    if (atom > lastAtom) return 0;
    if ((node = nodeTable[atom]) == (NodePtr)NULL) return 0;
    return node->string;
}

void
AtomError()
{
    FatalError("initializing atoms");
}

void
FreeAtom(NodePtr patom)
{
    if(patom->left)
	FreeAtom(patom->left);
    if(patom->right)
	FreeAtom(patom->right);
    if (patom->a > XA_LAST_PREDEFINED)
	xfree(patom->string);
    xfree(patom);
}

void
FreeAllAtoms()
{
    if(atomRoot == (NodePtr)NULL)
	return;
    FreeAtom(atomRoot);
    atomRoot = (NodePtr)NULL;
    xfree(nodeTable);
    nodeTable = (NodePtr *)NULL;
    lastAtom = None;
}

void
InitAtoms()
{
    FreeAllAtoms();
    tableLength = InitialTableSize;
    nodeTable = (NodePtr *)xalloc(InitialTableSize*sizeof(NodePtr));
    if (!nodeTable)
	AtomError();
    nodeTable[None] = (NodePtr)NULL;
    MakePredeclaredAtoms();
    if (lastAtom != XA_LAST_PREDEFINED)
	AtomError ();
}

    
