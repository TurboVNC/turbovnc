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
#ifndef AFM_H
#define AFM_H

/* Bounding box definition. Used for the Font BBox as well as the 
 * Character BBox.
 */
typedef struct
{ 
   int llx;	/* lower left x-position  */
   int lly;	/* lower left y-position  */
   int urx;	/* upper right x-position */
   int ury;	/* upper right y-position */
} BBox;

/* Character Metric Information. This structure is used only if ALL 
 * character metric information is requested. If only the character
 * widths is requested, then only an array of the character x-widths
 * is returned.
 *
 * The key that each field is associated with is in comments. For an 
 * explanation about each key and its value please refer to the 
 * Character Metrics section of the AFM documentation (full title
 * & version given above). 
 */
typedef struct
{
    long code;		/* CID code */
    int wx;		/* key: WX or W0X */
    BBox charBBox;	/* key: B */
} Metrics;

typedef struct
{ 
    int nChars;		        /* number of entries in char metrics array */
    Metrics *metrics;	        /* ptr to char metrics array */
} FontInfo;

int CIDAFM(FILE *, FontInfo **);
#endif /* AFM_H */
#endif
