/* Copyright (C)2004 Landmark Graphics Corporation
 * Copyright (C)2005, 2006 Sun Microsystems, Inc.
 * Copyright (C)2010-2013 D. R. Commander
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

/* This library abstracts fast frame buffer access */

#include <string.h>
#include <stdlib.h>
#include "fbx.h"


#define MINWIDTH  160
#define MINHEIGHT 24


static int __line=-1;
static FILE *__warningfile=NULL;

const int fbx_rmask[FBX_FORMATS]=
	{0x0000FF, 0x0000FF, 0xFF0000, 0xFF0000, 0x0000FF, 0xFF0000, 0};
const int fbx_gmask[FBX_FORMATS]=
	{0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0};
const int fbx_bmask[FBX_FORMATS]=
	{0xFF0000, 0xFF0000, 0x0000FF, 0x0000FF, 0xFF0000, 0x0000FF, 0};
const char *_fbx_formatname[FBX_FORMATS]=
	{"RGB", "RGBA", "BGR", "BGRA", "ABGR", "ARGB", "INDEX"};


#if defined(_WIN32)

static char __lasterror[1024]="No error";
#define _throw(m) {  \
	strncpy(__lasterror, m, 1023);  __line=__LINE__;  \
	goto finally;  \
}
#define w32(f) {  \
	if(!(f)) {  \
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),  \
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)__lasterror, 1024,  \
			NULL);  \
		__line=__LINE__;  goto finally;  \
	}  \
}
#define x11(f) if(!(f)) {  \
	snprintf(__lasterror, 1023, "X11 Error (window may have disappeared)");  \
		__line=__LINE__;  goto finally;  \
}

#else

static char *__lasterror="No error";
#define _throw(m) { __lasterror=m;  __line=__LINE__;  goto finally; }
#define x11(f) if(!(f)) {  \
	__lasterror="X11 Error (window may have disappeared)";  \
	__line=__LINE__;  goto finally;  \
}
#ifndef X_ShmAttach
#define X_ShmAttach 1
#endif

#endif


#ifdef _WIN32

typedef struct _BMINFO
{
	BITMAPINFO bmi;  RGBQUAD cmap[256];
} BMINFO;

#else

#include <errno.h>

#ifdef USESHM

static unsigned long serial=0;  static int __extok=1;
static XErrorHandler prevhandler=NULL;

int _fbx_xhandler(Display *dpy, XErrorEvent *e)
{
	if(e->serial==serial && (e->minor_code==X_ShmAttach
		&& e->error_code==BadAccess))
	{
		__extok=0;  return 0;
	}
	if(prevhandler && prevhandler!=_fbx_xhandler) return prevhandler(dpy, e);
	else return 0;
}
#endif

#endif


char *fbx_geterrmsg(void)
{
	return __lasterror;
}


int fbx_geterrline(void)
{
	return __line;
}


void fbx_printwarnings(FILE *stream)
{
	__warningfile=stream;
}


const char *fbx_formatname(int format)
{
	if(format>=0 && format<=FBX_FORMATS-1) return _fbx_formatname[format];
	else return "Invalid format";
}


int fbx_init(fbx_struct *s, fbx_wh wh, int width, int height, int useshm)
{
	int w, h;
	int rmask, gmask, bmask, ps, i;
	#ifdef _WIN32
	BMINFO bminfo;  HBITMAP hmembmp=0;  RECT rect;  HDC hdc=NULL;
	#else
	XWindowAttributes xwinattrib;  int shmok=1, alphafirst, pixmap=0;
	#endif

	if(!s) _throw("Invalid argument");

	#ifdef _WIN32

	if(!wh) _throw("Invalid argument");
	w32(GetClientRect(wh, &rect));
	if(width>0) w=width;
	else
	{
		w=rect.right-rect.left;  if(w<=0) w=MINWIDTH;
	}
	if(height>0) h=height;
	else
	{
		h=rect.bottom-rect.top;  if(h<=0) h=MINHEIGHT;
	}
	if(s->wh==wh)
	{
		if(w==s->width && h==s->height && s->hmdc && s->hdib && s->bits) return 0;
		else if(fbx_term(s)==-1) return -1;
	}
	memset(s, 0, sizeof(fbx_struct));
	s->wh=wh;

	w32(hdc=GetDC(s->wh));
	w32(s->hmdc=CreateCompatibleDC(hdc));
	w32(hmembmp=CreateCompatibleBitmap(hdc, w, h));
	w32(GetDeviceCaps(hdc, RASTERCAPS)&RC_BITBLT);
	w32(GetDeviceCaps(s->hmdc, RASTERCAPS)&RC_DI_BITMAP);
	bminfo.bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	bminfo.bmi.bmiHeader.biBitCount=0;
	w32(GetDIBits(s->hmdc, hmembmp, 0, 1, NULL, &bminfo.bmi, DIB_RGB_COLORS));
	w32(GetDIBits(s->hmdc, hmembmp, 0, 1, NULL, &bminfo.bmi, DIB_RGB_COLORS));
	w32(DeleteObject(hmembmp));  hmembmp=0;
		/* (we only needed it to get the screen properties) */
	ps=bminfo.bmi.bmiHeader.biBitCount/8;
	if(width>0) bminfo.bmi.bmiHeader.biWidth=width;
	if(height>0) bminfo.bmi.bmiHeader.biHeight=height;
	s->width=bminfo.bmi.bmiHeader.biWidth;
	s->height=bminfo.bmi.bmiHeader.biHeight;

	if(ps<3)
	{
		/* Make the buffer BGRA */
		bminfo.bmi.bmiHeader.biCompression=BI_BITFIELDS;
		bminfo.bmi.bmiHeader.biBitCount=32;
		ps=4;
		(*(DWORD *)&bminfo.bmi.bmiColors[0])=0xFF0000;
		(*(DWORD *)&bminfo.bmi.bmiColors[1])=0xFF00;
		(*(DWORD *)&bminfo.bmi.bmiColors[2])=0xFF;
	}

	s->pitch=BMPPAD(s->width*ps);  /* Windoze bitmaps are always padded */

	if(bminfo.bmi.bmiHeader.biCompression==BI_BITFIELDS)
	{
		rmask=(*(DWORD *)&bminfo.bmi.bmiColors[0]);
		gmask=(*(DWORD *)&bminfo.bmi.bmiColors[1]);
		bmask=(*(DWORD *)&bminfo.bmi.bmiColors[2]);
	}
	else
	{
		rmask=0xFF0000;
		gmask=0xFF00;
		bmask=0xFF;
	}

	s->format=-1;
	for(i=0; i<FBX_FORMATS; i++)
		if(rmask==fbx_rmask[i] && gmask==fbx_gmask[i] && bmask==fbx_bmask[i]
			&& ps==fbx_ps[i] && fbx_alphafirst[i]==0) s->format=i;
	if(s->format==-1) _throw("Display has unsupported pixel format");

	bminfo.bmi.bmiHeader.biHeight=-bminfo.bmi.bmiHeader.biHeight;
		/* (our convention is top-down) */
	w32(s->hdib=CreateDIBSection(hdc, &bminfo.bmi, DIB_RGB_COLORS,
		(void **)&s->bits, NULL, 0));
	w32(SelectObject(s->hmdc, s->hdib));
	ReleaseDC(s->wh, hdc);
	return 0;

	finally:
	if(hmembmp) DeleteObject(hmembmp);
	if(hdc) ReleaseDC(s->wh, hdc);

	#else

	if(!wh.dpy || !wh.d) _throw("Invalid argument");
	if(wh.v)
	{
		x11(XGetGeometry(wh.dpy, wh.d, &xwinattrib.root, &xwinattrib.x,
			&xwinattrib.y, (unsigned int *)&xwinattrib.width,
			(unsigned int *)&xwinattrib.height,
			(unsigned int *)&xwinattrib.border_width,
			(unsigned int *)&xwinattrib.depth));
		xwinattrib.visual=wh.v;
		useshm=0;
		pixmap=1;
	}
	else
	{
		x11(XGetWindowAttributes(wh.dpy, wh.d, &xwinattrib));
	}
	if(width>0) w=width;  else w=xwinattrib.width;
	if(height>0) h=height;  else h=xwinattrib.height;
	if(s->wh.dpy==wh.dpy && s->wh.d==wh.d)
	{
		if(w==s->width && h==s->height && s->xi && s->xgc && s->bits) return 0;
		else if(fbx_term(s)==-1) return -1;
	}
	memset(s, 0, sizeof(fbx_struct));
	s->wh.dpy=wh.dpy;  s->wh.d=wh.d;

	#ifdef USESHM
	if(!useshm)
	{
		static int alreadywarned=0;
		if(!alreadywarned && __warningfile)
		{
			fprintf(__warningfile, "[FBX] Disabling shared memory blitting\n");
			alreadywarned=1;
		}
	}
	if(useshm && XShmQueryExtension(s->wh.dpy))
	{
		static int alreadywarned=0;
		s->shminfo.shmid=-1;
		if(!(s->xi=XShmCreateImage(s->wh.dpy, xwinattrib.visual, xwinattrib.depth,
			ZPixmap, NULL, &s->shminfo, w, h)))
		{
			useshm=0;  goto noshm;
		}
		if((s->shminfo.shmid=shmget(IPC_PRIVATE,
			s->xi->bytes_per_line*s->xi->height+1, IPC_CREAT|0777))==-1)
		{
			useshm=0;  XDestroyImage(s->xi);  goto noshm;
		}
		if((s->shminfo.shmaddr=s->xi->data=(char *)shmat(s->shminfo.shmid, 0, 0))
			==(char *)-1)
		{
			useshm=0;  XDestroyImage(s->xi);
			shmctl(s->shminfo.shmid, IPC_RMID, 0);  goto noshm;
		}
		s->shminfo.readOnly=False;
		XLockDisplay(s->wh.dpy);
		XSync(s->wh.dpy, False);
		prevhandler=XSetErrorHandler(_fbx_xhandler);
		__extok=1;
		serial=NextRequest(s->wh.dpy);
		XShmAttach(s->wh.dpy, &s->shminfo);
		XSync(s->wh.dpy, False);
		XSetErrorHandler(prevhandler);
		shmok=__extok;
		if(!alreadywarned && !shmok && __warningfile)
		{
			fprintf(__warningfile,
				"[FBX] WARNING: MIT-SHM extension failed to initialize (this is normal on a\n");
			fprintf(__warningfile,
				"[FBX]    remote connection.)  Will use X Pixmap drawing instead.\n");
			alreadywarned=1;
		}
		XUnlockDisplay(s->wh.dpy);
		if(shmok)
		{
			char *env=getenv("FBX_USESHMPIXMAPS");
			if(env && !strcmp(env, "1"))
			{
				static int alreadywarned=0;
				if(!alreadywarned && __warningfile)
				{
					fprintf(__warningfile, "[FBX] Using MIT-SHM pixmaps\n");
					alreadywarned=1;
				}
				s->pm=XShmCreatePixmap(s->wh.dpy, s->wh.d, s->shminfo.shmaddr,
					&s->shminfo, w, h, xwinattrib.depth);
				if(!s->pm) shmok=0;
			}
		}
		shmctl(s->shminfo.shmid, IPC_RMID, 0);
		if(!shmok)
		{
			useshm=0;  XDestroyImage(s->xi);  shmdt(s->shminfo.shmaddr);
			shmctl(s->shminfo.shmid, IPC_RMID, 0);  goto noshm;
		}
		s->xattach=1;  s->shm=1;
	}
	else if(useshm)
	{
		static int alreadywarned=0;
		if(!alreadywarned && __warningfile)
		{
			fprintf(__warningfile,
				"[FBX] WARNING: MIT-SHM extension not available.  Will use X pixmap\n");
			fprintf(__warningfile, "[FBX]    drawing instead.\n");
			alreadywarned=1;
		}
		useshm=0;
	}
	noshm:
	if(!useshm)
	#endif
	{
		if(!pixmap)
			x11(s->pm=XCreatePixmap(s->wh.dpy, s->wh.d, w, h, xwinattrib.depth));
		x11(s->xi=XCreateImage(s->wh.dpy, xwinattrib.visual, xwinattrib.depth,
			ZPixmap, 0, NULL, w, h, 8, 0));
		if((s->xi->data=(char *)malloc(s->xi->bytes_per_line*s->xi->height+1))
			==NULL)
			_throw("Memory allocation error");
	}
	ps=s->xi->bits_per_pixel/8;
	s->width=s->xi->width;
	s->height=s->xi->height;
	s->pitch=s->xi->bytes_per_line;
	if(s->width!=w || s->height!=h)
		_throw("Bitmap returned does not match requested size");
	rmask=s->xi->red_mask;  gmask=s->xi->green_mask;  bmask=s->xi->blue_mask;
	alphafirst=0;
	if(s->xi->byte_order==MSBFirst)
	{
		if(ps<4)
		{
			rmask=s->xi->blue_mask;  gmask=s->xi->green_mask;  bmask=s->xi->red_mask;
		}
		else alphafirst=1;
	}

	s->format=-1;
	for(i=0; i<FBX_FORMATS; i++)
		if(rmask==fbx_rmask[i] && gmask==fbx_gmask[i] && bmask==fbx_bmask[i]
			&& ps==fbx_ps[i] && fbx_alphafirst[i]==alphafirst) s->format=i;
	if(s->format==-1) _throw("Display has unsupported pixel format");

	s->bits=s->xi->data;
	s->pixmap=pixmap;
	x11(s->xgc=XCreateGC(s->wh.dpy, s->pm? s->pm:s->wh.d, 0, NULL));
	return 0;

	finally:

	#endif

	fbx_term(s);
	return -1;
}


int fbx_read(fbx_struct *s, int winx, int winy)
{
	int wx, wy;
	#ifdef _WIN32
	fbx_gc gc;
	#endif
	if(!s) _throw("Invalid argument");
	wx=winx>=0? winx:0;  wy=winy>=0? winy:0;

	#ifdef _WIN32

	if(!s->hmdc || s->width<=0 || s->height<=0 || !s->bits || !s->wh)
		_throw("Not initialized");
	w32(gc=GetDC(s->wh));
	w32(BitBlt(s->hmdc, 0, 0, s->width, s->height, gc, wx, wy, SRCCOPY));
	w32(ReleaseDC(s->wh, gc));
	return 0;

	#else

	if(!s->wh.dpy || !s->wh.d || !s->xi || !s->bits) _throw("Not initialized");
	#ifdef USESHM
	if(!s->xattach && s->shm)
	{
		x11(XShmAttach(s->wh.dpy, &s->shminfo));  s->xattach=1;
	}
	#endif

	#ifdef USESHM
	if(s->shm)
	{
		x11(XShmGetImage(s->wh.dpy, s->wh.d, s->xi, wx, wy, AllPlanes));
	}
	else
	#endif
	{
		x11(XGetSubImage(s->wh.dpy, s->wh.d, wx, wy, s->width, s->height,
			AllPlanes, ZPixmap, s->xi, 0, 0));
	}
	return 0;

	#endif

	finally:
	return -1;
}


int fbx_write(fbx_struct *s, int bmpx, int bmpy, int winx, int winy, int w,
	int h)
{
	int bx, by, wx, wy, bw, bh;
	#ifdef _WIN32
	BITMAPINFO bmi;  fbx_gc gc;
	#endif
	if(!s) _throw("Invalid argument");

	bx=bmpx>=0? bmpx:0;  by=bmpy>=0? bmpy:0;  bw=w>0? w:s->width;
	bh=h>0? h:s->height;
	wx=winx>=0? winx:0;  wy=winy>=0? winy:0;
	if(bw>s->width) bw=s->width;  if(bh>s->height) bh=s->height;
	if(bx+bw>s->width) bw=s->width-bx;  if(by+bh>s->height) bh=s->height-by;

	#ifdef _WIN32

	if(!s->wh || s->width<=0 || s->height<=0 || !s->bits)
		_throw("Not initialized");
	memset(&bmi, 0, sizeof(bmi));
	bmi.bmiHeader.biSize=sizeof(bmi);
	bmi.bmiHeader.biWidth=s->width;
	bmi.bmiHeader.biHeight=-s->height;
	bmi.bmiHeader.biPlanes=1;
	bmi.bmiHeader.biBitCount=fbx_ps[s->format]*8;
	bmi.bmiHeader.biCompression=BI_RGB;
	w32(gc=GetDC(s->wh));
	w32(SetDIBitsToDevice(gc, wx, wy, bw, bh, bx, 0, 0, bh,
		&s->bits[by*s->pitch], &bmi, DIB_RGB_COLORS));
	w32(ReleaseDC(s->wh, gc));
	return 0;

	#else

	if(!s->pm || !s->shm)
		if(fbx_awrite(s, bmpx, bmpy, winx, winy, w, h)==-1) return -1;
	if(s->pm)
	{
		XCopyArea(s->wh.dpy, s->pm, s->wh.d, s->xgc, bx, by, bw, bh, wx, wy);
	}
	XFlush(s->wh.dpy);
	XSync(s->wh.dpy, False);
	return 0;

	#endif

	finally:
	return -1;
}


int fbx_flip(fbx_struct *s, int bmpx, int bmpy, int w, int h)
{
	int i, bx, by, bw, bh, ps, pitch;
	char *tmpbuf=NULL, *srcptr, *dstptr;
	if(!s) _throw("Invalid argument");

	bx=bmpx>=0? bmpx:0;  by=bmpy>=0? bmpy:0;  bw=w>0? w:s->width;
	bh=h>0? h:s->height;
	if(bw>s->width) bw=s->width;  if(bh>s->height) bh=s->height;
	if(bx+bw>s->width) bw=s->width-bx;  if(by+bh>s->height) bh=s->height-by;
	ps=fbx_ps[s->format];  pitch=s->pitch;

	if(!(tmpbuf=(char *)malloc(bw*ps)))
		_throw("Memory allocation error");
	srcptr=&s->bits[pitch*by+ps*bx];
	dstptr=&s->bits[pitch*(by+bh-1)+ps*bx];
	for(i=0; i<bh/2; i++, srcptr+=pitch, dstptr-=pitch)
	{
		memcpy(tmpbuf, srcptr, bw*ps);
		memcpy(srcptr, dstptr, bw*ps);
		memcpy(dstptr, tmpbuf, bw*ps);
	}
	free(tmpbuf);
	return 0;

	finally:
	if(tmpbuf) free(tmpbuf);
	return -1;
}


#ifndef _WIN32

int fbx_awrite(fbx_struct *s, int bmpx, int bmpy, int winx, int winy, int w,
	int h)
{
	int bx, by, wx, wy, bw, bh;
	if(!s) _throw("Invalid argument");
	bx=bmpx>=0? bmpx:0;  by=bmpy>=0? bmpy:0;  bw=w>0? w:s->width;
	bh=h>0? h:s->height;
	wx=winx>=0? winx:0;  wy=winy>=0? winy:0;
	if(bw>s->width) bw=s->width;  if(bh>s->height) bh=s->height;
	if(bx+bw>s->width) bw=s->width-bx;  if(by+bh>s->height) bh=s->height-by;
	if(!s->wh.dpy || !s->wh.d || !s->xi || !s->bits) _throw("Not initialized");
	#ifdef USESHM
	if(s->shm)
	{
		if(!s->xattach)
		{
			x11(XShmAttach(s->wh.dpy, &s->shminfo));  s->xattach=1;
		}
		x11(XShmPutImage(s->wh.dpy, s->wh.d, s->xgc, s->xi, bx, by, wx, wy, bw,
			bh, False));
	}
	else
	#endif
	{
		Drawable d=s->pixmap? s->wh.d:s->pm;
		if(d==s->pm) wx=wy=0;
		XPutImage(s->wh.dpy, d, s->xgc, s->xi, bx, by, wx, wy, bw, bh);
	}
	return 0;

	finally:
	return -1;
}

#endif


int fbx_sync(fbx_struct *s)
{
	#ifdef _WIN32

	return 0;

	#else

	if(!s) _throw("Invalid argument");
	if(s->pm)
	{
		XCopyArea(s->wh.dpy, s->pm, s->wh.d, s->xgc, 0, 0, s->width, s->height,
			0, 0);
	}
	XFlush(s->wh.dpy);
	XSync(s->wh.dpy, False);
	return 0;

	finally:
	return -1;

	#endif
}


int fbx_term(fbx_struct *s)
{
	if(!s) _throw("Invalid argument");

	#ifdef _WIN32

	if(s->hdib) DeleteObject(s->hdib);
	if(s->hmdc) DeleteDC(s->hmdc);

	#else

	if(s->pm)
	{
		XFreePixmap(s->wh.dpy, s->pm);  s->pm=0;
	}
	if(s->xi) 
	{
		if(s->xi->data && !s->shm)
		{
			free(s->xi->data);  s->xi->data=NULL;
		}
		XDestroyImage(s->xi);
	}
	#ifdef USESHM
	if(s->shm)
	{
		if(s->xattach)
		{
			XShmDetach(s->wh.dpy, &s->shminfo);  XSync(s->wh.dpy, False);
		}
		if(s->shminfo.shmaddr!=NULL) shmdt(s->shminfo.shmaddr);
		if(s->shminfo.shmid!=-1) shmctl(s->shminfo.shmid, IPC_RMID, 0);
	}
	#endif
	if(s->xgc) XFreeGC(s->wh.dpy, s->xgc);

	#endif

	memset(s, 0, sizeof(fbx_struct));
	return 0;

	finally:
	return -1;
}
