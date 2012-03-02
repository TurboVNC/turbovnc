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
/* $XFree86: xc/lib/font/Type1/afm.c,v 1.3 2001/08/27 19:49:52 dawes Exp $ */

#ifdef BUILDCID
#ifndef FONTMODULE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#else
#include "Xmd.h"        /* For INT32 declaration */
#include "Xdefs.h"      /* For Bool */
#include "xf86_ansic.h"
#endif
#include "fontmisc.h"			/* for xalloc/xfree */
#include "AFM.h"

#define PBUF 256
#define KBUF 20

char *gettoken(FILE *);
  
static char *afmbuf = NULL;

char *gettoken(FILE *fd) {
    char *bp;
    int c, found;

    bp = afmbuf;
    found = 0;

    while((c = getc(fd)) != EOF) {
        if (found == 0 && (c == ' ' || c == '\t' || c == '\n' || c == '\r' || 
            c == ';' || c == ',')) continue;
        found = 1;
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != ';') {
            *bp++ = c;
            if (bp - afmbuf >= PBUF) {
                bp = afmbuf;
                break;
            }             
        } else
            break;
    } 

    *bp = 0;
    return(afmbuf);
}

int CIDAFM(FILE *fd, FontInfo **pfi) {
    char *p = 0;
    int i, j, k = 0, found = 0;
    FontInfo *fi;

    if (fd == NULL || pfi == NULL) return(1);

    *pfi = NULL;

    if ((afmbuf = (char *)xalloc(PBUF)) == NULL) 
        return(1);

    while(1) {
        if (!(p = gettoken(fd))) {
            xfree(afmbuf);
            return(1);
        }

        if (strncmp(p, "StartFontMetrics", 16) == 0) {
            if (!(p = gettoken(fd))) {
                xfree(afmbuf);
                return(1);
            }
            if (strncmp(p, "4", 1) < 0) {
                free(afmbuf);
                return(1);
            }
            found = 1;
        } else if (strncmp(p, "StartCharMetrics", 16) == 0) {
            if (!found) {
                xfree(afmbuf);
                return(1);
            }

            if (!(p = gettoken(fd))) {
                xfree(afmbuf);
                return(1);
            } 

            fi = (FontInfo *)xalloc(sizeof(FontInfo));
            
            if (fi == NULL) {
                xfree(afmbuf);
                return(1);
            }
            bzero(fi, sizeof(FontInfo));
            
            fi->nChars = atoi(p);

            fi->metrics = (Metrics *)xalloc(fi->nChars * 
                sizeof(Metrics));
            if (fi->metrics == NULL) {
                xfree(afmbuf);
                xfree(fi);
                return(1);
            }

            j = 0;
            for (i = 0; i < fi->nChars; i++) {
                k = 0;
                while(1) { 
                    if (!(p = gettoken(fd))) {
                        k = KBUF;
                        break;
                    }
                    if (strncmp(p, "W0X", 3) == 0) {
                        if (!(p = gettoken(fd))) {
                            k = KBUF;
                            break;
                        }
                        fi->metrics[j].wx = atoi(p);
                    } else if (strncmp(p, "N", 1) == 0) {
                        if (!(p = gettoken(fd))) {
                            k = KBUF;
                            break;
                        }
                        fi->metrics[j].code = (long)atoi(p);
                    } else if (strncmp(p, "B", 1) == 0) {
                        if (!(p = gettoken(fd))) {
                            k = KBUF;
                            break;
                        }
                        fi->metrics[j].charBBox.llx = atoi(p);
                        if (!(p = gettoken(fd))) {
                            k = KBUF;
                            break;
                        }
                        fi->metrics[j].charBBox.lly = atoi(p);
                        if (!(p = gettoken(fd))) {
                            k = KBUF;
                            break;
                        }
                        fi->metrics[j].charBBox.urx = atoi(p);
                        if (!(p = gettoken(fd))) {
                            k = KBUF;
                            break;
                        }
                        fi->metrics[j].charBBox.ury = atoi(p);
                        j++;
                        break;
                    } 
                    k++;
                    if (k >= KBUF) break;
                }
                if (k >= KBUF) break;
            }
            if (k >= KBUF || j != fi->nChars) {
                xfree(fi->metrics);
                xfree(fi);
                xfree(afmbuf);
                return(1);
            } else {
                *pfi = fi;
                xfree(afmbuf);
                return(0);
            }
        }
    } 
    
    xfree(afmbuf);
    return(1);
}
#endif
