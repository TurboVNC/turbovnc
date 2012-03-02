/* $Xorg: t1imager.h,v 1.3 2000/08/17 19:46:33 cpqbld Exp $ */
/* Copyright International Business Machines,Corp. 1991
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is
 * hereby granted, provided that the above copyright notice
 * appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation,
 * and that the name of IBM not be used in advertising or
 * publicity pertaining to distribution of the software without
 * specific, written prior permission.
 *
 * IBM PROVIDES THIS SOFTWARE "AS IS", WITHOUT ANY WARRANTIES
 * OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT
 * LIMITED TO ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS.  THE ENTIRE RISK AS TO THE QUALITY AND
 * PERFORMANCE OF THE SOFTWARE, INCLUDING ANY DUTY TO SUPPORT
 * OR MAINTAIN, BELONGS TO THE LICENSEE.  SHOULD ANY PORTION OF
 * THE SOFTWARE PROVE DEFECTIVE, THE LICENSEE (NOT IBM) ASSUMES
 * THE ENTIRE COST OF ALL SERVICING, REPAIR AND CORRECTION.  IN
 * NO EVENT SHALL IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
/* $XFree86: xc/lib/font/Type1/t1imager.h,v 1.5 2001/07/25 15:04:55 dawes Exp $ */


#include "objects.h"
#include "spaces.h"
#include "paths.h"
#include "regions.h"

typedef  struct xobject *xobject;
typedef  struct segment *path;
typedef  struct region *region;
typedef  struct XYspace *XYspace;
 

#ifndef   NULL
#include <stddef.h>
#endif
 
#ifndef   TRUE
#define   TRUE          1
#endif
 
#ifndef   FALSE
#define   FALSE         0
#endif
  

#define   WINDINGRULE -2
#define   EVENODDRULE -3
 
#define   CONTINUITY  0x80   /* can be added to above rules; e.g. WINDINGRULE+CONTINUITY */
 
 
/*
Generic null object definition:
*/
#define    NULLOBJECT   ((xobject)NULL)
 
/*
Null path definition:
*/
#define    NULLPATH     NULLOBJECT
 
/*
Full page and null region definition:
*/
#define    INFINITY     t1_Infinity
#ifndef NOEXTERNS
extern     region       *INFINITY;
#endif
#define    NULLREGION   NULLOBJECT
 
#define    FF_PARSE_ERROR  5
#define    FF_PATH         1
 
