/*
Copyright (c) 1998 by Juliusz Chroboczek

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/* $XFree86$ */

/* These data are very dodgy. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "bics-unicode.h"

static short table_160[]=
{0, 128, 98, 97, 278, 274, 277, 110, 135, 503, 538, 125, 309, 191, 504,
 230, 339, 286, 160, 161, 129, 325, 279, 102, 141, 159, 544, 126, 151,
 153, 155, 127, 259, 261, 257, 253, 255, 113, 114, 148, 249, 251, 247,
 245, 239, 241, 237, 235, 169, 196, 202, 200, 204, 208, 206, 284, 115,
 212, 210, 214, 216, 224, 271, 121, 260, 262, 258, 254, 256, 117, 118,
 149, 250, 252, 248, 246, 240, 242, 238, 236, 273, 195, 201, 199, 203,
 207, 205, 285, 119, 211, 209, 213, 215, 223, 272, 221, 477, 476, 374,
 373, 171, 177, 376, 375, -1, -1, -1, -1, 378, 377, 379, -1, 169, 173,
 383, 382, -1, -1, -1, -1, 172, 178, 243, 244, -1, -1, -1, -1, -1, -1,
 385, -1, -1, -1, -1, -1, 233, 234, 387, 386, -1, -1, 391, 390, 389,
 122, 276, 275, -1, -1, 393, 392, -1, 395, 394, 399, 398, -1, -1, -1,
 -1, 170, 174, 194, 193, 402, 401, 198, 197, 263, -1, -1, -1, -1, -1,
 -1, 404, 403, 116, 120, -1, -1, 408, 407, 406, 405, 410, 409, -1, -1,
 486, 485, 412, 411, 419, 418, 364, 363, -1, -1, 218, 217, 421, 420, -1,
 -1, 220, 219, 423, 422, -1, 268, 425, 424, -1, -1, 222, 368, 367, 372,
 371, 370, 369};

static short table_728[]={144, 181, 146, 731, 137, 183};

static short table_915[]=
{313, 314, -1, -1, -1, 315, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 316, -1, -1, 317, -1, -1, 318, -1, -1, -1, -1, -1, -1, -1, 319, 320,
 -1, 321, 322, -1, 323, 324, -1, -1, -1, 325, -1, -1, -1, 326, -1, -1,
 327, 328, -1, 329};

static short table_8211[]=
{111, 112, -1, -1, -1, -1, -1, 106, -1, 103, 105, 107, 104, 108, 109,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 100, -1, -1,
 -1, -1, -1, -1, -1, -1, 123, 124, -1, 265};

static short table_8319[]=
{543, 475, 466, 467, 468, 469, 470, 471, 472, 473, 474, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, 491, -1, -1, -1, 266};

static short table_8592[]={293, 295, 294, 292, 297, 296};

static short table_8712[]=
{298, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 492, -1, -1, -1, -1, -1,
 -1, 302, -1, -1, -1, 303, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 299,
 -1, -1, -1, -1, 428};


static short table_8800[]={288, -1, -1, -1, 291, 290};

static short table_9600[]=
{304, -1, -1, -1, 305, -1, -1, -1, 308, -1, -1, -1, 306, -1, -1, -1,
 307, 357, 358, 359, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 335, 336, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, 348, -1, -1, -1, 345, -1, -1, -1, -1, -1, 347, -1, -1, -1, 346,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 339, -1, -1, -1, 342, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, 344};

static short table_9784[]={360, -1, 361, 362, -1, -1, -1, -1, 350, -1, 349};

static short table_9824[]=
{354, -1, -1, 352, -1, 351, 353, -1, -1, -1, 330, 331};

static short table_64256[]={282, 95, 96, 281};

int 
unicode_to_bics(unsigned code)
{
  if(code<32) return -1;
  else if(code<127) return code-32;
  else if(code<160) return -1;
  else if(code<383) return table_160[code-160];
  else if(code==402) return 99;
  else if(code==486) return 480;
  else if(code==487) return 379;
  else if(code==501) return 384;
  else if(code==711) return 139;
  else if(code<728) return -1;
  else if(code<734) return table_728[code-728];
  else if(code<915) return -1;
  else if(code<967) return table_915[code-915];
  else if(code<8211) return -1;
  else if(code<8253) return table_8211[code-8211];
  else if(code<8319) return -1;
  else if(code<8360) return table_8319[code-8319];
  else if(code<8592) return -1;
  else if(code<8598) return table_8592[code-8592];
  else if(code==8616) return 340;
  else if(code<8712) return -1;
  else if(code<8751) return table_8712[code-8712];
  else if(code<8800) return -1;
  else if(code<8806) return table_8800[code-8800];
  else if(code==8976) return 310;
  else if(code==8992) return 300;
  else if(code==8993) return 301;
  else if(code==9400) return 332;
  else if(code==9415) return 333;
  else if(code==9473) return 355;
  else if(code==9475) return 356;
  else if(code<9600) return -1;
  else if(code<9690) return table_9600[code-9600];
  else if(code==9711) return 343;
  else if(code<9784) return -1;
  else if(code<9795) return table_9784[code-9784];
  else if(code<9824) return -1;
  else if(code<9836) return table_9824[code-9824];
  else if(code<64256) return -1;
  else if(code<64261) return table_64256[code-64256];
  else return -1;
}

