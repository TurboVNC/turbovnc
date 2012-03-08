/* $Xorg: t1test.c,v 1.3 2000/08/17 19:46:34 cpqbld Exp $ */
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
 
#include "fntfilst.h"
#include "FSproto.h"
 
void Display();
 
#define DECIPOINTSPERINCH 722.7
#define DEFAULTRES 75
#define DEFAULTPOINTSIZE 120
 
FontScalableRec vals;
FontEntryRec entry;
 
int main(argc, argv)
       int argc;
       char *argv[];
{
       int h;
       char temp[80];
       char file[80];
       char glyphcode[1];
       FontPtr fontptr;
       CharInfoRec *glyphs[1];
       int count;
       int code;
       int rc = -1;
 
       T1FillVals(&vals);
       Type1RegisterFontFileFunctions();
       entry.name.name = "-adobe-utopia-medium-r-normal--0-0-0-0-p-0-iso8859-1";
 
       for (;;) {
               printf("T1TEST: ");
               gets(temp);
               glyphcode[0] = '\0';
 
               switch(temp[0]) {
 
                   case 'c':
                       if (1 != sscanf(&temp[2], "%c", glyphcode))
                               printf("glyph code?\n");
                       break;
 
                   case 'x':
                       if (1 != sscanf(&temp[2], "%x", &code))
                               printf("glyph code?\n");
                       else
                               glyphcode[0] = code;
                       break;
 
                   case 'd':
                       if (1 != sscanf(&temp[2], "%d", &code))
                               printf("glyph code?\n");
                       else
                               glyphcode[0] = code;
                       break;
 
                   case 'h':
                       if (1 != sscanf(&temp[2], "%d", &h))
                               printf("height?\n");
                       vals.pixel = h;
                       rc = Type1OpenScalable(NULL, &fontptr, 0, &entry, file, &vals, 0, 0);
                       break;
 
                   case 'f':
                       if (1 != sscanf(&temp[2], "%s", file))
                               printf("file name?\n");
                       rc = Type1OpenScalable(NULL, &fontptr, 0, &entry, file, &vals, 0, 0);
                       break;
 
                   case 't':
                       if (1 != sscanf(&temp[2], "%s", file))
                               printf("file name?\n");
                       vals.pixel = 8;
                       rc = Type1OpenScalable(NULL, &fontptr, 0, &entry, file, &vals, 0, 0);
                       if (rc != Successful) break;
                       vals.pixel = 20;
                       rc = Type1OpenScalable(NULL, &fontptr, 0, &entry, file, &vals, 0, 0);
                       if (rc != Successful) break;
                       vals.pixel = 50;
                       rc = Type1OpenScalable(NULL, &fontptr, 0, &entry, file, &vals, 0, 0);
                       glyphcode[0] = 'A';
                       printf("From font '%s':\n", file);
                       break;
 
                   case 'q':
                       return 0;
 
                   default:
                       printf("unknown command '%c', must one of 'qfchdxt'\n", temp[0]);
 
               }
               if (rc == Successful) {
                      if (glyphcode[0] != '\0') {
                              (*fontptr->get_glyphs)(fontptr, 1, glyphcode, 0, &count, glyphs);
                              if (count > 0)
                                      Display(glyphs[0]);
                              else
                                      printf("Code %x not valid in this font\n", glyphcode[0]);
                      }
               }
               else
                      printf("Bad font (rc = %d, file='%s')\n", rc, file);
       }
}
 
static void Display(glyph)
       CharInfoRec *glyph;
{
       int h,w;
       unsigned char *p;
       int data;
       int i;
 
       p = glyph->bits;
 
       printf("Metrics: left=%d, right=%d, w=%d, above=%d, below=%d\n",
               glyph->metrics.leftSideBearing,
               glyph->metrics.rightSideBearing,
               glyph->metrics.characterWidth,
               glyph->metrics.ascent,
               glyph->metrics.descent);
 
       for (h=glyph->metrics.ascent + glyph->metrics.descent; --h >= 0;) {
               w = glyph->metrics.rightSideBearing - glyph->metrics.leftSideBearing;
               while (w > 0) {
                       data = *p++;
                       for (i=0; i<8; i++) {
                               if (--w < 0)
                                       break;
                               if (data & 0x80)
                                       printf("X");
                               else
                                       printf(".");
                               data <<= 1;
                       }
               }
               printf("\n");
       }
}
 
T1FillVals(vals)
    FontScalablePtr vals;
{
    FontResolutionPtr res;
    int         x_res = DEFAULTRES;
    int         y_res = DEFAULTRES;
    int         pointsize = DEFAULTPOINTSIZE;  /* decipoints */
    int         num_res;
 
    /* Must have x, y, and pixel */
    if (!vals->x || !vals->y || !vals->pixel) {
        res = GetClientResolutions(&num_res);
        if (num_res) {
            if (res->x_resolution)
                x_res = res->x_resolution;
            if (res->y_resolution)
                y_res = res->y_resolution;
            if (res->point_size)
                pointsize = res->point_size;
        }
        if (!vals->x)
            vals->x = x_res;
        if (!vals->y)
            vals->y = y_res;
        if (!vals->point) {
            if (!vals->pixel) vals->point = pointsize;
            else vals->point = (vals->pixel * DECIPOINTSPERINCH) / vals->y;
        }
        if (!vals->pixel)
            vals->pixel = (vals->point * vals->y) / DECIPOINTSPERINCH;
        /* Make sure above arithmetic is normally in range and will
           round properly. +++ */
    }
}
 
int CheckFSFormat(format, fmask, bit, byte, scan, glyph, image)
       int format,fmask,*bit,*byte,*scan,*glyph,*image;
{
       *bit = *byte = 1;
       *glyph = *scan = *image = 1;
       return Successful;
 
}
 
char *MakeAtom(p)
       char *p;
{
       return p;
}


FontResolutionPtr GetClientResolutions(resP)
       int *resP;
{
       *resP = 0;
};
 
char *Xalloc(size)
       int size;
{
       extern char *malloc();
       return(malloc(size));
}
 
void Xfree()
{
       free();
}
 
FontDefaultFormat() { ; }
 
FontFileRegisterRenderer() { ; }
 
GenericGetBitmaps() { ; }
GenericGetExtents() { ; }
 
FontParseXLFDName() { ; }
FontComputeInfoAccelerators() { ; }
