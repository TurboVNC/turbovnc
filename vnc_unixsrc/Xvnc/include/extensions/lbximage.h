/* $XConsortium: lbximage.h /main/6 1996/11/04 16:48:27 rws $ */

/******************************************************************************

Copyright (c) 1994  X Consortium

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
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

******************************************************************************/

#ifndef _LBX_IMAGE_H_
#define _LBX_IMAGE_H_

typedef struct _LbxBitmapCompMethod {

    char *methodName;
    int inited;
    int methodOpCode;	/* to be filled on reply from server */

    int (*compInit)(
#if NeedFunctionPrototypes
	void
#endif
    );

    int (*compFunc)(
#if NeedFunctionPrototypes
	unsigned char *		/* inbuf */,
	unsigned char *		/* outbuf */,
	int			/* outbufSize */,
	int			/* image_bytes */,
	int			/* pixels_per_line */,
	int			/* padded_bytes_per_scanline */,
	int			/* reverse_bits */,
	int *			/* bytesCompressed */
#endif
    );

    int (*decompFunc)(
#if NeedFunctionPrototypes
	unsigned char *		/* inbuf */,
	unsigned char *		/* outbuf */,
	int			/* image_bytes */,
	int			/* pixels_per_line */,
	int			/* padded_bytes_per_scanline */,
	int			/* reverse_bits */
#endif
    );

} LbxBitmapCompMethod;


#define LBX_MAX_DEPTHS 5

typedef struct _LbxPixmapCompMethod {

    char *methodName;
    unsigned formatMask;
    int depthCount;
    int depths[LBX_MAX_DEPTHS];
    int inited;
    int methodOpCode;	/* to be filled on reply from server */

    int (*compInit)(
#if NeedFunctionPrototypes
	void
#endif
    );

    int (*compFunc)(
#if NeedFunctionPrototypes
	char *			/* inbuf */,
	char *			/* outbuf */,
	int			/* outbufSize */,
	int			/* format */,
	int			/* depth */,
	int			/* num_scan_lines */,
	int			/* scan_line_size */,
	int *			/* bytesCompressed */
#endif
    );

    int (*decompFunc)(
#if NeedFunctionPrototypes
	char *			/* inbuf */,
	char *			/* outbuf */,
	int			/* num_scan_lines */,
	int			/* scan_line_size */
#endif
    );

} LbxPixmapCompMethod;



extern int LbxImageEncodePackBits (
#if NeedFunctionPrototypes
char *			/* inbuf */,
char *			/* outbuf */,
int			/* outbufSize */,
int			/* format */,
int			/* depth */,
int			/* num_scan_lines */,
int			/* scan_line_size */,
int *			/* bytesCompressed */
#endif
);

extern int LbxImageEncodeFaxG42D (
#if NeedFunctionPrototypes
unsigned char *		/* inbuf */,
unsigned char *		/* outbuf */,
int			/* outbufSize */,
int			/* image_bytes */,
int			/* pixels_per_line */,
int			/* padded_bytes_per_scanline */,
int			/* reverse_bits */,
int *			/* bytesCompressed */
#endif
);

extern int LbxImageDecodePackBits (
#if NeedFunctionPrototypes
char *			/* inbuf */,
char *			/* outbuf */,
int			/* num_scan_lines */,
int			/* scan_line_size */
#endif
);

extern int LbxImageDecodeFaxG42D (
#if NeedFunctionPrototypes
unsigned char *		/* inbuf */,
unsigned char *		/* outbuf */,
int			/* image_bytes */,
int			/* pixels_per_line */,
int			/* padded_bytes_per_scanline */,
int			/* reverse_bits */
#endif
);


#define LBX_IMAGE_COMPRESS_SUCCESS		0
#define LBX_IMAGE_COMPRESS_NO_SUPPORT		1
#define LBX_IMAGE_COMPRESS_BAD_MALLOC		2
#define LBX_IMAGE_COMPRESS_NOT_WORTH_IT		3

#endif /* _LBX_IMAGE_H_ */
