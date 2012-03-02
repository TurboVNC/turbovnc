/* Copyright (c) 1994-1999 Silicon Graphics, Inc. All Rights Reserved.
 *
 * The contents of this file are subject to the CID Font Code Public Licence
 * Version 1.0 (the "License"). You may not use this file except in compliance
 * with the Licence. You may obtain a copy of the License at Silicon Graphics,
 * Inc., attn: Legal Services, 2011 N. Shoreline Blvd., Mountain View, CA
 * 94043 or at http://www.sgi.com/software/opensource/cid/license.html.
 *
 * Software distributed under the License is distributed on an "AS IS" basis.
 * ALL WARRANTIES ARE DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED
 * WARRANTIES OF MERCHANTABILITY, OF FITNESS FOR A PARTICULAR PURPOSE OR OF
 * NON-INFRINGEMENT. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Software is CID font code that was developed by Silicon
 * Graphics, Inc.
 */
#ifdef BUILDCID
#define CID_NAME_MAX 255       /* max # of characters in a file name */
#define CID_PATH_MAX 1024      /* max # of characters in a path name */

typedef struct spacerange_code {
    unsigned int srcCodeLo;
    unsigned int srcCodeHi;
} spacerangecode;

typedef struct space_range {
    struct space_range *next;
    int rangecnt;
    struct spacerange_code *spacecode;
} spacerange;

typedef struct cidrange_code {
    unsigned int srcCodeLo;
    unsigned int srcCodeHi;
    unsigned int dstCIDLo;
} cidrangecode;

typedef struct cid_range {
    struct cid_range *next;
    int rangecnt;
    struct cidrange_code *range;
} cidrange;
#endif
