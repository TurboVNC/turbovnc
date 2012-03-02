/* $Xorg: htest.c,v 1.3 2000/08/17 19:46:25 cpqbld Exp $ */

/*

Copyright 1989-1991, Bitstream Inc., Cambridge, MA.
You are hereby granted permission under all Bitstream propriety rights to
use, copy, modify, sublicense, sell, and redistribute the Bitstream Speedo
software and the Bitstream Charter outline font for any purpose and without
restrictions; provided, that this notice is left intact on all copies of such
software or font and that Bitstream's trademark is acknowledged as shown below
on all unmodified copies of such font.

BITSTREAM CHARTER is a registered trademark of Bitstream Inc.


BITSTREAM INC. DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING
WITHOUT LIMITATION THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE.  BITSTREAM SHALL NOT BE LIABLE FOR ANY DIRECT OR INDIRECT
DAMAGES, INCLUDING BUT NOT LIMITED TO LOST PROFITS, LOST DATA, OR ANY OTHER
INCIDENTAL OR CONSEQUENTIAL DAMAGES, ARISING OUT OF OR IN ANY WAY CONNECTED
WITH THE SPEEDO SOFTWARE OR THE BITSTREAM CHARTER OUTLINE FONT.

*/

/****************************** H T E S T . C ********************************
 *                                                                           *
 *              SPEEDO FONT HEADER TEST MODULE                               *
 *                                                                           *
 ****************************************************************************/


#include "speedo.h"                 /* General definition for make_bmap */
#include <stdio.h>

#define DEBUG  0

#if DEBUG
#define SHOW(X) printf("X = %d\n", X)
#else
#define SHOW(X)
#endif

#define FONT_BUFFER_SIZE  1000

/***** EXTERNAL FUNCTIONS *****/

/***** STATIC VARIABLES *****/
static  char   pathname[100]; /* Name of font file to be output */
static  ufix8  font_buffer[FONT_BUFFER_SIZE]; /* Font buffer */
static  FILE  *fdescr;             /* Speedo outline file descriptor */



FUNCTION main(argc,argv)
int argc;
char *argv[];
{
int     bytes_read;           /* Number of bytes read from font file */
ufix8   tmpufix8;             /* Temporary workspace */
fix15   tmpfix15;             /* Temporary workspace */
ufix16  tmpufix16;            /* Temporary workspace */
ufix32  tmpufix32;            /* Temporary workspace */
ufix8  *pvt_header_org;       /* Origin of provate header data */

ufix8   read_1b();            /* Read 1 byte field from font header */
fix15   read_2b();            /* Read 2 byte field from font header */
fix31   read_4b();            /* Read 4 byte field from font header */

if (argc != 2) 
    {
    fprintf(stderr,"Usage: htest {fontfile}\n\n"); 
    exit (1);
    }

sprintf(pathname, argv[1]);

/* Initialization */
printf("\n      SPEEDO FONT FILE HEADER DATA\n");
printf("      -------------------------\n\n");
/* Load Speedo outline file */
fdescr = fopen (pathname, "rb");
if (fdescr == NULL)
    {
    printf("****** Cannot open file %s\n", pathname);
    return;                  
    }

bytes_read = fread(font_buffer, sizeof(ufix8), sizeof(font_buffer), fdescr);
if (bytes_read == 0)
    {
    printf("****** Error on reading %s: %x\n", pathname, bytes_read);
    fclose(fdescr);     
    return;
    }

printf("Format Identifier ...................... %.4s\n", font_buffer + FH_FMVER);

tmpufix32 = (ufix32)read_4b(font_buffer + FH_FMVER + 4);
printf("CR-LF-NULL-NULL data ............... %8.8lx %s\n", tmpufix32, (tmpufix32 != 0x0d0a0000)? "(incorrect)": " ");

printf("Font Size .............................. %4ld\n", (ufix32)read_4b(font_buffer + FH_FNTSZ));

printf("Minimum Font Buffer Size ............... %4ld\n", (ufix32)read_4b(font_buffer + FH_FBFSZ));

printf("Minimum Character Buffer Size .......... %4d\n", (ufix16)read_2b(font_buffer + FH_CBFSZ));

printf("Header Size ............................ %4d\n", (ufix16)read_2b(font_buffer + FH_HEDSZ));

printf("Font ID ................................ %4.4d\n", (ufix16)read_2b(font_buffer + FH_FNTID));

printf("Font Version Number .................... %4d\n", (ufix16)read_1b(font_buffer + FH_SFVNR));

printf("Font Full Name:\n    %.70s\n", font_buffer + FH_FNTNM);

printf("Manufacturing Date ................ %10.10s\n", font_buffer + FH_MDATE);

printf("Character Set Name:\n    %s\n", font_buffer + FH_LAYNM);

printf("Character Set ID: ...................... %.4s\n", font_buffer + FH_LAYNM + 66);

printf("Copyright Notice:\n    %.70s\n", font_buffer + FH_CPYRT);

printf("Number of Char. Indexes in Char. Set ... %4d\n", (ufix16)read_2b(font_buffer + FH_NCHRL));

printf("Total number of Char. Indexes in Font .. %4d\n", (ufix16)read_2b(font_buffer + FH_NCHRF));

printf("Index of First Character ............... %4d\n", (ufix16)read_2b(font_buffer + FH_FCHRF));

printf("Number of Kerning Tracks ............... %4d\n", (ufix16)read_2b(font_buffer + FH_NKTKS));

printf("Number of Kerning Pairs ................ %4d\n", (ufix16)read_2b(font_buffer + FH_NKPRS));

printf("Font Flags:\n");
tmpufix8 = read_1b(font_buffer + FH_FLAGS);
printf("    Extended font ...................... %s\n", (tmpufix8 & BIT0)? " Yes": "  No");

printf("Classification Flags:\n");
tmpufix8 = read_1b(font_buffer + FH_CLFGS);
printf("    Italic ............................. %s\n", (tmpufix8 & BIT0)? " Yes": "  No");
printf("    Monospace .......................... %s\n", (tmpufix8 & BIT1)? " Yes": "  No");
printf("    Serif .............................. %s\n", (tmpufix8 & BIT2)? " Yes": "  No");
printf("    Display ............................ %s\n", (tmpufix8 & BIT3)? " Yes": "  No");

tmpufix8 = read_1b(font_buffer + FH_FAMCL);
printf("Family Classification .................. %4d ", tmpufix8);
switch (tmpufix8)
    {
case 0:
    printf("(Don't care)\n");
    break;
case 1:
    printf("(Serif)\n");
    break;
case 2:
    printf("(Sans serif)\n");
    break;
case 3:
    printf("(Monospace)\n");
    break;
case 4:
    printf("(Script or calligraphic)\n");
    break;
case 5:
    printf("(Decorative)\n");
    break;
default:
    printf("\n");
    break;
    }

printf("Font Form Classification:\n");
tmpufix8 = read_1b(font_buffer + FH_FRMCL);
printf("    Width Type ......................... %4d ", (tmpufix8 & 0x0f));
switch (tmpufix8 & 0x0f)
    {
case 4:
    printf("(Condensed)\n");
    break;
case 6:
    printf("(Semi-condensed)\n");
    break;
case 8:
    printf("(Normal)\n");
    break;
case 10:
    printf("(Semi-expanded)\n");
    break;
case 12:
    printf("(Expanded)\n");
    break;
default:
    printf("\n");
    break;
    }
printf("    Weight ............................. %4d ", (tmpufix8 >> 4));
switch (tmpufix8 >> 4)
    {
case 1:
    printf("(Thin)\n");
    break;
case 2:
    printf("(Ultralight)\n");
    break;
case 3:
    printf("(Extra light)\n");
    break;
case 4:
    printf("(Light)\n");
    break;
case 5:
    printf("(Book)\n");
    break;
case 6:
    printf("(Normal)\n");
    break;
case 7:
    printf("(Medium)\n");
    break;
case 8:
    printf("(Semibold)\n");
    break;
case 9:
    printf("(Demibold)\n");
    break;
case 10:
    printf("(Bold)\n");
    break;
case 11:
    printf("(Extrabold)\n");
    break;
case 12:
    printf("(Ultrabold)\n");
    break;
case 13:
    printf("(Heavy)\n");
    break;
case 14:
    printf("(Black)\n");
    break;
default:
    printf("\n");
    break;
    }

printf("Short Font Name ........................ %.16s\n", font_buffer + FH_SFNTN);

printf("Short Face Name ........................ %.16s\n", font_buffer + FH_SFACN);

printf("Font Form .............................. %.14s\n", font_buffer + FH_FNTFM);

printf("Italic Angle ........................... %7.2f\n", ((real)read_2b(font_buffer + FH_ITANG) / 256.0));

printf("ORUs per Em ............................ %4d\n", (ufix16)read_2b(font_buffer + FH_ORUPM));

printf("Width of Word Space .................... %4d\n", (ufix16)read_2b(font_buffer + FH_WDWTH));

printf("Width of Em Space ...................... %4d\n", (ufix16)read_2b(font_buffer + FH_EMWTH));

printf("Width of En Space ...................... %4d\n", (ufix16)read_2b(font_buffer + FH_ENWTH));

printf("Width of Thin Space .................... %4d\n", (ufix16)read_2b(font_buffer + FH_TNWTH));

printf("Width of Figure Space .................. %4d\n", (ufix16)read_2b(font_buffer + FH_FGWTH));

printf("Min X coordinate in font ............... %4d\n", (fix15)read_2b(font_buffer + FH_FXMIN));

printf("Min Y coordinate in font ............... %4d\n", (fix15)read_2b(font_buffer + FH_FYMIN));

printf("Max X coordinate in font ............... %4d\n", (fix15)read_2b(font_buffer + FH_FXMAX));

printf("Max Y coordinate in font ............... %4d\n", (fix15)read_2b(font_buffer + FH_FYMAX));

printf("Underline Position ..................... %4d\n", (fix15)read_2b(font_buffer + FH_ULPOS));

printf("Underline Thickness .................... %4d\n", (fix15)read_2b(font_buffer + FH_ULTHK));

printf("Small Caps Y position .................. %4d\n", (fix15)read_2b(font_buffer + FH_SMCTR));
printf("Small Caps X scale ..................... %7.2f\n", ((real)read_2b(font_buffer + FH_SMCTR + 2) / 4096.0));
printf("Small Caps Y scale ..................... %7.2f\n", ((real)(fix15)read_2b(font_buffer + FH_SMCTR + 4) / 4096.0));

printf("Display Superiors Y position ........... %4d\n", (fix15)read_2b(font_buffer + FH_SMCTR));
printf("Display Superiors X scale .............. %7.2f\n", ((real)read_2b(font_buffer + FH_SMCTR + 2) / 4096.0));
printf("Display Superiors Y scale .............. %7.2f\n", ((real)read_2b(font_buffer + FH_SMCTR + 4) / 4096.0));

printf("Footnote Superiors Y position .......... %4d\n", (fix15)read_2b(font_buffer + FH_FNSTR));
printf("Footnote Superiors X scale ............. %7.2f\n", ((real)read_2b(font_buffer + FH_FNSTR + 2) / 4096.0));
printf("Footnote Superiors Y scale ............. %7.2f\n", ((real)read_2b(font_buffer + FH_FNSTR + 4) / 4096.0));

printf("Alpha Superiors Y position ............. %4d\n", (fix15)read_2b(font_buffer + FH_ALSTR));
printf("Alpha Superiors X scale ................ %7.2f\n", ((real)read_2b(font_buffer + FH_ALSTR + 2) / 4096.0));
printf("Alpha Superiors Y scale ................ %7.2f\n", ((real)read_2b(font_buffer + FH_ALSTR + 4) / 4096.0));

printf("Chemical Inferiors Y position .......... %4d\n", (fix15)read_2b(font_buffer + FH_CMITR));
printf("Chemical Inferiors X scale ............. %7.2f\n", ((real)read_2b(font_buffer + FH_CMITR + 2) / 4096.0));
printf("Chemical Inferiors Y scale ............. %7.2f\n", ((real)read_2b(font_buffer + FH_CMITR + 4) / 4096.0));

printf("Small Numerators Y position ............ %4d\n", (fix15)read_2b(font_buffer + FH_SNMTR));
printf("Small Numerators X scale ............... %7.2f\n", ((real)read_2b(font_buffer + FH_SNMTR + 2) / 4096.0));
printf("Small Numerators Y scale ............... %7.2f\n", ((real)read_2b(font_buffer + FH_SNMTR + 4) / 4096.0));

printf("Small Denominators Y position .......... %4d\n", (fix15)read_2b(font_buffer + FH_SDNTR));
printf("Small Denominators X scale ............. %7.2f\n", ((real)read_2b(font_buffer + FH_SDNTR + 2) / 4096.0));
printf("Small Denominators Y scale ............. %7.2f\n", ((real)read_2b(font_buffer + FH_SDNTR + 4) / 4096.0));

printf("Medium Numerators Y position ........... %4d\n", (fix15)read_2b(font_buffer + FH_MNMTR));
printf("Medium Numerators X scale .............. %7.2f\n", ((real)read_2b(font_buffer + FH_MNMTR + 2) / 4096.0));
printf("Medium Numerators Y scale .............. %7.2f\n", ((real)read_2b(font_buffer + FH_MNMTR + 4) / 4096.0));

printf("Medium Denominators Y position ......... %4d\n", (fix15)read_2b(font_buffer + FH_MDNTR));
printf("Medium Denominators X scale ............ %7.2f\n", ((real)read_2b(font_buffer + FH_MDNTR + 2) / 4096.0));
printf("Medium Denominators Y scale ............ %7.2f\n", ((real)read_2b(font_buffer + FH_MDNTR + 4) / 4096.0));

printf("Large Numerators Y position ............ %4d\n", (fix15)read_2b(font_buffer + FH_LNMTR));
printf("Large Numerators X scale ............... %7.2f\n", ((real)read_2b(font_buffer + FH_LNMTR + 2) / 4096.0));
printf("Large Numerators Y scale ............... %7.2f\n", ((real)read_2b(font_buffer + FH_LNMTR + 4) / 4096.0));

printf("Large Denominators Y position .......... %4d\n", (fix15)read_2b(font_buffer + FH_LDNTR));
printf("Large Denominators X scale ............. %7.2f\n", ((real)read_2b(font_buffer + FH_LDNTR + 2) / 4096.0));
printf("Large Denominators Y scale ............. %7.2f\n", ((real)read_2b(font_buffer + FH_LDNTR + 4) / 4096.0));

fclose(fdescr);     
}


FUNCTION ufix8 read_1b(pointer)
ufix8 *pointer;
/*
 * Reads 1-byte field from font buffer 
 */
{
return *pointer;
}

FUNCTION fix15 read_2b(pointer)
ufix8 *pointer;
/*
 * Reads 2-byte field from font buffer 
 */
{
fix31 temp;

temp = *pointer++;
temp = (temp << 8) + *(pointer);
return temp;
}

FUNCTION fix31 read_4b(pointer)
ufix8 *pointer;
/*
 * Reads 4-byte field from font buffer 
 */
{
fix31 temp;

temp = *pointer++;
temp = (temp << 8) + *(pointer++);
temp = (temp << 8) + *(pointer++);
temp = (temp << 8) + *(pointer);
return temp;
}

