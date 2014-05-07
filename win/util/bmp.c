/* Copyright (C)2004 Landmark Graphics Corporation
 * Copyright (C)2005 Sun Microsystems, Inc.
 * Copyright (C)2014 D. R. Commander
 *
 * This library is free software and may be redistributed and/or modified under
 * the terms of the wxWindows Library License, Version 3.1 or (at your option)
 * any later version.  The full license is in the LICENSE.txt file included
 * with this distribution.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * wxWindows Library License for more details.
 */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
 #include <io.h>
#else
 #include <unistd.h>
#endif
#include "vglutil.h"
#include "bmp.h"


#ifndef BI_BITFIELDS
#define BI_BITFIELDS 3L
#endif
#ifndef BI_RGB
#define BI_RGB 0L
#endif


#define BMPHDRSIZE 54
typedef struct
{
	unsigned short bfType;
	unsigned int bfSize;
	unsigned short bfReserved1, bfReserved2;
	unsigned int bfOffBits;

	unsigned int biSize;
	int biWidth, biHeight;
	unsigned short biPlanes, biBitCount;
	unsigned int biCompression, biSizeImage;
	int biXPelsPerMeter, biYPelsPerMeter;
	unsigned int biClrUsed, biClrImportant;
} BitmapHeader;


static const char *errorStr="No error";


#define _throw(m) {  \
	errorStr=m;  ret=-1;  goto finally;  \
}
#define _unix(f) {  \
	if((f)==-1) _throw(strerror(errno));  \
}
#define _catch(f) {  \
	if((f)==-1) {  \
		ret=-1;  goto finally;  \
	}  \
}


#define READ(fd, addr, size) \
	if((bytesRead=read(fd, addr, (size)))==-1) _throw(strerror(errno)); \
	if(bytesRead!=(size)) _throw("Read error");

#define WRITE(fd, addr, size) \
	if((bytesWritten=write(fd, addr, (size)))==-1) _throw(strerror(errno)); \
	if(bytesWritten!=(size)) _throw("Write error");


static void pixelConvert(unsigned char *srcbuf, int width, int srcPitch,
	int height, enum BMPPF srcFormat, unsigned char *dstbuf, int dstPitch,
	enum BMPPF dstFormat, int flip)
{
	unsigned char *srcptr, *srcptr0, *dstptr, *dstptr0;
	int i, j;

	srcptr=flip? &srcbuf[srcPitch*(height-1)]:srcbuf;
	for(j=0, dstptr=dstbuf; j<height; j++,
		srcptr+=flip? -srcPitch:srcPitch, dstptr+=dstPitch)
	{
		for(i=0, srcptr0=srcptr, dstptr0=dstptr; i<width; i++,
			srcptr0+=bmp_ps[srcFormat], dstptr0+=bmp_ps[dstFormat])
		{
			dstptr0[bmp_roffset[dstFormat]]=srcptr0[bmp_roffset[srcFormat]];
			dstptr0[bmp_goffset[dstFormat]]=srcptr0[bmp_goffset[srcFormat]];
			dstptr0[bmp_boffset[dstFormat]]=srcptr0[bmp_boffset[srcFormat]];
		}
		if(width*bmp_ps[dstFormat]!=dstPitch)
			memset(&dstptr[width*bmp_ps[dstFormat]], 0,
				dstPitch-width*bmp_ps[dstFormat]);
	}
}


static int ppm_load(int *fd, unsigned char **buf, int *width, int align,
	int *height, enum BMPPF format, enum BMPORN orientation, int ascii)
{
	FILE *file=NULL;  int ret=0, scaleFactor, dstPitch;
	unsigned char *tempbuf=NULL;  char temps[255], temps2[255];
	int numRead=0, totalRead=0, pixel[3], i, j;

	if((file=fdopen(*fd, "r"))==NULL) _throw(strerror(errno));

	do
	{
		if(!fgets(temps, 255, file)) _throw("Read error");
		if(strlen(temps)==0 || temps[0]=='\n') continue;
		if(sscanf(temps, "%s", temps2)==1 && temps2[1]=='#') continue;
		switch(totalRead)
		{
			case 0:
				if((numRead=sscanf(temps, "%d %d %d", width, height,
					&scaleFactor))==EOF)
					_throw("Read error");
				break;
			case 1:
				if((numRead=sscanf(temps, "%d %d", height, &scaleFactor))==EOF)
					_throw("Read error");
				break;
			case 2:
				if((numRead=sscanf(temps, "%d", &scaleFactor))==EOF)
					_throw("Read error");
				break;
		}
		totalRead+=numRead;
	} while(totalRead<3);
	if((*width)<1 || (*height)<1 || scaleFactor<1) _throw("Corrupt PPM header");

	dstPitch=(((*width)*bmp_ps[format])+(align-1))&(~(align-1));
	if((*buf=(unsigned char *)malloc(dstPitch*(*height)))==NULL)
		_throw("Memory allocation error");
	if(ascii)
	{
		for(j=0; j<*height; j++)
		{
			for(i=0; i<*width; i++)
			{
				if(fscanf(file, "%d%d%d", &pixel[0], &pixel[1], &pixel[2])!=3)
					_throw("Read error");
				(*buf)[j*dstPitch+i*bmp_ps[format]+bmp_roffset[format]]=
					(unsigned char)(pixel[0]*255/scaleFactor);
				(*buf)[j*dstPitch+i*bmp_ps[format]+bmp_goffset[format]]=
					(unsigned char)(pixel[1]*255/scaleFactor);
				(*buf)[j*dstPitch+i*bmp_ps[format]+bmp_boffset[format]]=
					(unsigned char)(pixel[2]*255/scaleFactor);
			}
		}
	}
	else
	{
		if(scaleFactor!=255)
			_throw("Binary PPMs must have 8-bit components");
		if((tempbuf=(unsigned char *)malloc((*width)*(*height)*3))==NULL)
			_throw("Memory allocation error");
		if(fread(tempbuf, (*width)*(*height)*3, 1, file)!=1)
			_throw("Read error");
		pixelConvert(tempbuf, *width, (*width)*3, *height, BMPPF_RGB, *buf,
			dstPitch, format, orientation==BMPORN_BOTTOMUP);
	}

	finally:
	if(file) { fclose(file);  *fd=-1; }
	if(tempbuf) free(tempbuf);
	return ret;
}


int bmp_load(char *filename, unsigned char **buf, int *width, int align,
	int *height, enum BMPPF format, enum BMPORN orientation)
{
	int fd=-1, bytesRead, srcPitch, srcOrientation=BMPORN_BOTTOMUP,
		srcPixelSize, dstPitch, ret=0;
	unsigned char *tempbuf=NULL;
	BitmapHeader bh;  int flags=O_RDONLY;

	#ifdef _WIN32
	flags|=O_BINARY;
	#endif
	if(!filename || !buf || !width || !height || format<0	|| format>BMP_NUMPF-1
		|| align<1)
		_throw("invalid argument to loadbmp()");
	if((align&(align-1))!=0)
		_throw("Alignment must be a power of 2");
	_unix(fd=open(filename, flags));

	READ(fd, &bh.bfType, sizeof(unsigned short));
	if(!littleendian())	bh.bfType=byteswap16(bh.bfType);

	if(bh.bfType==0x3650)
	{
		_catch(ppm_load(&fd, buf, width, align, height, format, orientation, 0));
		goto finally;
	}
	if(bh.bfType==0x3350)
	{
		_catch(ppm_load(&fd, buf, width, align, height, format, orientation, 1));
		goto finally;
	}

	READ(fd, &bh.bfSize, sizeof(unsigned int));
	READ(fd, &bh.bfReserved1, sizeof(unsigned short));
	READ(fd, &bh.bfReserved2, sizeof(unsigned short));
	READ(fd, &bh.bfOffBits, sizeof(unsigned int));
	READ(fd, &bh.biSize, sizeof(unsigned int));
	READ(fd, &bh.biWidth, sizeof(int));
	READ(fd, &bh.biHeight, sizeof(int));
	READ(fd, &bh.biPlanes, sizeof(unsigned short));
	READ(fd, &bh.biBitCount, sizeof(unsigned short));
	READ(fd, &bh.biCompression, sizeof(unsigned int));
	READ(fd, &bh.biSizeImage, sizeof(unsigned int));
	READ(fd, &bh.biXPelsPerMeter, sizeof(int));
	READ(fd, &bh.biYPelsPerMeter, sizeof(int));
	READ(fd, &bh.biClrUsed, sizeof(unsigned int));
	READ(fd, &bh.biClrImportant, sizeof(unsigned int));

	if(!littleendian())
	{
		bh.bfSize=byteswap(bh.bfSize);
		bh.bfOffBits=byteswap(bh.bfOffBits);
		bh.biSize=byteswap(bh.biSize);
		bh.biWidth=byteswap(bh.biWidth);
		bh.biHeight=byteswap(bh.biHeight);
		bh.biPlanes=byteswap16(bh.biPlanes);
		bh.biBitCount=byteswap16(bh.biBitCount);
		bh.biCompression=byteswap(bh.biCompression);
		bh.biSizeImage=byteswap(bh.biSizeImage);
		bh.biXPelsPerMeter=byteswap(bh.biXPelsPerMeter);
		bh.biYPelsPerMeter=byteswap(bh.biYPelsPerMeter);
		bh.biClrUsed=byteswap(bh.biClrUsed);
		bh.biClrImportant=byteswap(bh.biClrImportant);
	}

	if(bh.bfType!=0x4d42 || bh.bfOffBits<BMPHDRSIZE || bh.biWidth<1
		|| bh.biHeight==0)
		_throw("Corrupt bitmap header");
	if((bh.biBitCount!=24 && bh.biBitCount!=32) || bh.biCompression!=BI_RGB)
		_throw("Only uncompessed RGB bitmaps are supported");

	*width=bh.biWidth;  *height=bh.biHeight;  srcPixelSize=bh.biBitCount/8;
	if(*height<0) { *height=-(*height);  srcOrientation=BMPORN_TOPDOWN; }
	srcPitch=(((*width)*srcPixelSize)+3)&(~3);
	dstPitch=(((*width)*bmp_ps[format])+(align-1))&(~(align-1));

	if(srcPitch*(*height)+bh.bfOffBits!=bh.bfSize)
		_throw("Corrupt bitmap header");
	if((tempbuf=(unsigned char *)malloc(srcPitch*(*height)))==NULL
		|| (*buf=(unsigned char *)malloc(dstPitch*(*height)))==NULL)
		_throw("Memory allocation error");
	if(lseek(fd, (long)bh.bfOffBits, SEEK_SET)!=(long)bh.bfOffBits)
		_throw(strerror(errno));
	_unix(bytesRead=read(fd, tempbuf, srcPitch*(*height)));
	if(bytesRead!=srcPitch*(*height)) _throw("Read error");

	pixelConvert(tempbuf, *width, srcPitch, *height, BMPPF_BGR, *buf, dstPitch,
		format, srcOrientation!=orientation);

	finally:
	if(tempbuf) free(tempbuf);
	if(fd!=-1) close(fd);
	return ret;
}


static int ppm_save(char *filename, unsigned char *buf, int width, int pitch,
	int height, enum BMPPF format, enum BMPORN orientation)
{
	FILE *file=NULL;  int ret=0;
	unsigned char *tempbuf=NULL;

	if((file=fopen(filename, "wb"))==NULL) _throw(strerror(errno));
	if(fprintf(file, "P6\n")<1) _throw("Write error");
	if(fprintf(file, "%d %d\n", width, height)<1) _throw("Write error");
	if(fprintf(file, "255\n")<1) _throw("Write error");

	if((tempbuf=(unsigned char *)malloc(width*height*3))==NULL)
		_throw("Memory allocation error");

	pixelConvert(buf, width, pitch, height, format, tempbuf, width*3, BMPPF_RGB,
		orientation==BMPORN_BOTTOMUP);

	if((fwrite(tempbuf, width*height*3, 1, file))!=1) _throw("Write error");

	finally:
	if(tempbuf) free(tempbuf);
	if(file) fclose(file);
	return ret;
}


int bmp_save(char *filename, unsigned char *buf, int width, int pitch,
	int height, enum BMPPF format, enum BMPORN orientation)
{
	int fd=-1, bytesWritten, dstPitch, ret=0;
	int flags=O_RDWR|O_CREAT|O_TRUNC;
	unsigned char *tempbuf=NULL;  char *temp;
	BitmapHeader bh;  int mode;

	#ifdef _WIN32
	flags|=O_BINARY;  mode=_S_IREAD|_S_IWRITE;
	#else
	mode=S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
	#endif
	if(!filename || !buf || width<1 || height<1 || format<0 || format>BMP_NUMPF-1
		|| pitch<0)
		_throw("bad argument to bmp_save()");

	if(pitch==0) pitch=width*bmp_ps[format];

	if((temp=strrchr(filename, '.'))!=NULL)
	{
		if(!stricmp(temp, ".ppm"))
			return ppm_save(filename, buf, width, pitch, height, format,
				orientation);
	}

	_unix(fd=open(filename, flags, mode));
	dstPitch=((width*3)+3)&(~3);

	bh.bfType=0x4d42;
	bh.bfSize=BMPHDRSIZE+dstPitch*height;
	bh.bfReserved1=0;  bh.bfReserved2=0;
	bh.bfOffBits=BMPHDRSIZE;
	bh.biSize=40;
	bh.biWidth=width;  bh.biHeight=height;
	bh.biPlanes=0;  bh.biBitCount=24;
	bh.biCompression=BI_RGB;  bh.biSizeImage=0;
	bh.biXPelsPerMeter=0;  bh.biYPelsPerMeter=0;
	bh.biClrUsed=0;  bh.biClrImportant=0;

	if(!littleendian())
	{
		bh.bfType=byteswap16(bh.bfType);
		bh.bfSize=byteswap(bh.bfSize);
		bh.bfOffBits=byteswap(bh.bfOffBits);
		bh.biSize=byteswap(bh.biSize);
		bh.biWidth=byteswap(bh.biWidth);
		bh.biHeight=byteswap(bh.biHeight);
		bh.biPlanes=byteswap16(bh.biPlanes);
		bh.biBitCount=byteswap16(bh.biBitCount);
		bh.biCompression=byteswap(bh.biCompression);
		bh.biSizeImage=byteswap(bh.biSizeImage);
		bh.biXPelsPerMeter=byteswap(bh.biXPelsPerMeter);
		bh.biYPelsPerMeter=byteswap(bh.biYPelsPerMeter);
		bh.biClrUsed=byteswap(bh.biClrUsed);
		bh.biClrImportant=byteswap(bh.biClrImportant);
	}

	WRITE(fd, &bh.bfType, sizeof(unsigned short));
	WRITE(fd, &bh.bfSize, sizeof(unsigned int));
	WRITE(fd, &bh.bfReserved1, sizeof(unsigned short));
	WRITE(fd, &bh.bfReserved2, sizeof(unsigned short));
	WRITE(fd, &bh.bfOffBits, sizeof(unsigned int));
	WRITE(fd, &bh.biSize, sizeof(unsigned int));
	WRITE(fd, &bh.biWidth, sizeof(int));
	WRITE(fd, &bh.biHeight, sizeof(int));
	WRITE(fd, &bh.biPlanes, sizeof(unsigned short));
	WRITE(fd, &bh.biBitCount, sizeof(unsigned short));
	WRITE(fd, &bh.biCompression, sizeof(unsigned int));
	WRITE(fd, &bh.biSizeImage, sizeof(unsigned int));
	WRITE(fd, &bh.biXPelsPerMeter, sizeof(int));
	WRITE(fd, &bh.biYPelsPerMeter, sizeof(int));
	WRITE(fd, &bh.biClrUsed, sizeof(unsigned int));
	WRITE(fd, &bh.biClrImportant, sizeof(unsigned int));

	if((tempbuf=(unsigned char *)malloc(dstPitch*height))==NULL)
		_throw("Memory allocation error");

	pixelConvert(buf, width, pitch, height, format, tempbuf, dstPitch,
		BMPPF_BGR, orientation!=BMPORN_BOTTOMUP);

	if((bytesWritten=write(fd, tempbuf, dstPitch*height))!=dstPitch*height)
		_throw(strerror(errno));

	finally:
	if(tempbuf) free(tempbuf);
	if(fd!=-1) close(fd);
	return ret;
}


const char *bmp_geterr(void)
{
	return errorStr;
}
