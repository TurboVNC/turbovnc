/* Copyright (C)2004 Landmark Graphics Corporation
 * Copyright (C)2005, 2006 Sun Microsystems, Inc.
 * Copyright (C)2010 D. R. Commander
 *
 * The VNC system is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

// This library abstracts fast frame buffer access
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

#if defined(FBXWIN32) || defined(XWIN32)

 static char __lasterror[1024]="No error";
 #define _throw(m) {strncpy(__lasterror, m, 1023);  __line=__LINE__;  goto finally;}
 #define w32(f) {if(!(f)) {FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),  \
	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)__lasterror, 1024, NULL);  \
	__line=__LINE__;  goto finally;}}
 #define x11(f) if(!(f)) {snprintf(__lasterror, 1023, "X11 Error (window may have disappeared)");  __line=__LINE__;  goto finally;}

#else

 static char *__lasterror="No error";
 #define _throw(m) {__lasterror=m;  __line=__LINE__;  goto finally;}
 #define x11(f) if(!(f)) {__lasterror="X11 Error (window may have disappeared)";  __line=__LINE__;  goto finally;}
 #ifndef X_ShmAttach
 #define X_ShmAttach 1
 #endif

#endif

#ifdef FBXWIN32

 typedef struct _BMINFO {BITMAPINFO bmi;  RGBQUAD cmap[256];} BMINFO;

#else

 #include <errno.h>
 #include "x11err.h"

 #ifdef USESHM
 #ifdef XDK
 static int fbx_checkdlls(void);
 static int fbx_checkdll(char *, int *, int *, int *, int *);
 #endif

 static unsigned long serial=0;  static int __extok=1;
 static XErrorHandler prevhandler=NULL;

 int _fbx_xhandler(Display *dpy, XErrorEvent *e)
 {
	if(e->serial==serial && (e->minor_code==X_ShmAttach && e->error_code==BadAccess))
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
	#ifdef FBXWIN32
	BMINFO bminfo;  HBITMAP hmembmp=0;  RECT rect;  HDC hdc=NULL;
	#else
	XWindowAttributes xwinattrib;  int shmok=1, alphafirst;
	#ifdef XWIN32
	static int seqnum=1;  char temps[80];
	#endif
	#endif

	if(!s) _throw("Invalid argument");

	#ifdef FBXWIN32

	if(!wh) _throw("Invalid argument");
	w32(GetClientRect(wh, &rect));
	if(width>0) w=width;  else {w=rect.right-rect.left;  if(w<=0) w=MINWIDTH;}
	if(height>0) h=height;  else {h=rect.bottom-rect.top;  if(h<=0) h=MINHEIGHT;}
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
	w32(DeleteObject(hmembmp));  hmembmp=0;  // We only needed it to get the screen properties
	ps=bminfo.bmi.bmiHeader.biBitCount/8;
	if(width>0) bminfo.bmi.bmiHeader.biWidth=width;
	if(height>0) bminfo.bmi.bmiHeader.biHeight=height;
	s->width=bminfo.bmi.bmiHeader.biWidth;
	s->height=bminfo.bmi.bmiHeader.biHeight;

	if(ps<3)
	{
		// Make the buffer BGRA
		bminfo.bmi.bmiHeader.biCompression=BI_BITFIELDS;
		bminfo.bmi.bmiHeader.biBitCount=32;
		ps=4;
		(*(DWORD *)&bminfo.bmi.bmiColors[0])=0xFF0000;
		(*(DWORD *)&bminfo.bmi.bmiColors[1])=0xFF00;
		(*(DWORD *)&bminfo.bmi.bmiColors[2])=0xFF;
	}

	s->pitch=BMPPAD(s->width*ps);  // Windoze bitmaps are always padded

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

	bminfo.bmi.bmiHeader.biHeight=-bminfo.bmi.bmiHeader.biHeight;  // Our convention is top down
	w32(s->hdib=CreateDIBSection(hdc, &bminfo.bmi, DIB_RGB_COLORS, (void **)&s->bits, NULL, 0));
	w32(SelectObject(s->hmdc, s->hdib));
	ReleaseDC(s->wh, hdc);
	return 0;

	finally:
	if(hmembmp) DeleteObject(hmembmp);
	if(hdc) ReleaseDC(s->wh, hdc);

	#else

	if(!wh.dpy || !wh.win) _throw("Invalid argument");
	x11(XGetWindowAttributes(wh.dpy, wh.win, &xwinattrib));
	if(width>0) w=width;  else w=xwinattrib.width;
	if(height>0) h=height;  else h=xwinattrib.height;
	if(s->wh.dpy==wh.dpy && s->wh.win==wh.win)
	{
		if(w==s->width && h==s->height && s->xi && s->xgc && s->bits) return 0;
		else if(fbx_term(s)==-1) return -1;
	}
	memset(s, 0, sizeof(fbx_struct));
	s->wh.dpy=wh.dpy;  s->wh.win=wh.win;

	#ifdef USESHM
	#ifdef XDK
	if(!fbx_checkdlls()) useshm=0;
	#endif
	if(useshm && XShmQueryExtension(s->wh.dpy))
	{
		static int alreadywarned=0;
		s->shminfo.shmid=-1;
		if(!(s->xi=XShmCreateImage(s->wh.dpy, xwinattrib.visual, xwinattrib.depth, ZPixmap, NULL,
			&s->shminfo, w, h)))
		{
			useshm=0;  goto noshm;
		}
		#ifdef XWIN32
		s->shminfo.shmid=seqnum;
		sprintf(temps, "X11-MIT-SHM-%d", s->shminfo.shmid);
		seqnum++;
		w32(s->filemap=CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
			0, s->xi->bytes_per_line*s->xi->height+1, temps));
		if((s->shminfo.shmaddr=s->xi->data=(char *)MapViewOfFile(s->filemap,
			FILE_MAP_WRITE, 0, 0, 0))==NULL)
		{
			useshm=0;  XDestroyImage(s->xi);  CloseHandle(s->filemap);  goto noshm;
		}
		#else
		if((s->shminfo.shmid=shmget(IPC_PRIVATE, s->xi->bytes_per_line*s->xi->height+1, IPC_CREAT|0777))==-1)
		{
			useshm=0;  XDestroyImage(s->xi);  goto noshm;
		}
		if((s->shminfo.shmaddr=s->xi->data=(char *)shmat(s->shminfo.shmid, 0, 0))
			==(char *)-1)
		{
			useshm=0;  XDestroyImage(s->xi);  shmctl(s->shminfo.shmid, IPC_RMID, 0);  goto noshm;
		}
		#endif
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
			fprintf(__warningfile, "[FBX] WARNING: MIT-SHM extension failed to initialize (this is normal on a\n");
			fprintf(__warningfile, "[FBX]    remote connection.)  Will use X Pixmap drawing instead.\n");
			alreadywarned=1;
		}
		XUnlockDisplay(s->wh.dpy);
		#ifndef XDK
		shmctl(s->shminfo.shmid, IPC_RMID, 0);
		#endif
		#ifdef XWIN32
		if(!shmok)
		{
			useshm=0;  XDestroyImage(s->xi);  if(s->filemap) CloseHandle(s->filemap);  goto noshm;
		}
		#else
		if(!shmok)
		{
			useshm=0;  XDestroyImage(s->xi);  shmdt(s->shminfo.shmaddr);
			shmctl(s->shminfo.shmid, IPC_RMID, 0);  goto noshm;
		}
		#endif
		s->xattach=1;  s->shm=1;
	}
	else if(useshm)
	{
		static int alreadywarned=0;
		if(!alreadywarned && __warningfile)
		{
			fprintf(__warningfile, "[FBX] WARNING: MIT-SHM extension not available.  Will use X pixmap\n");
			fprintf(__warningfile, "[FBX]    drawing instead.\n");
			alreadywarned=1;
		}
		useshm=0;
	}
	noshm:
	if(!useshm)
	#endif
	{
		x11(s->pm=XCreatePixmap(s->wh.dpy, s->wh.win, xwinattrib.width, xwinattrib.height,
			xwinattrib.depth));
		x11(s->xi=XCreateImage(s->wh.dpy, xwinattrib.visual, xwinattrib.depth, ZPixmap, 0, NULL,
			w, h, 8, 0));
		if((s->xi->data=(char *)malloc(s->xi->bytes_per_line*s->xi->height+1))==NULL)
			_throw("Memory allocation error");
	}
	ps=s->xi->bits_per_pixel/8;
	s->width=s->xi->width;
	s->height=s->xi->height;
	s->pitch=s->xi->bytes_per_line;
	if(s->width!=w || s->height!=h) _throw("Bitmap returned does not match requested size");
	rmask=s->xi->red_mask;  gmask=s->xi->green_mask;  bmask=s->xi->blue_mask;
	alphafirst=0;
	if(s->xi->byte_order==MSBFirst)
	{
		if(ps<4) {rmask=s->xi->blue_mask;  gmask=s->xi->green_mask;  bmask=s->xi->red_mask;}
		else alphafirst=1;
	}

	s->format=-1;
	for(i=0; i<FBX_FORMATS; i++)
		if(rmask==fbx_rmask[i] && gmask==fbx_gmask[i] && bmask==fbx_bmask[i]
			&& ps==fbx_ps[i] && fbx_alphafirst[i]==alphafirst) s->format=i;
	if(s->format==-1) _throw("Display has unsupported pixel format");

	s->bits=s->xi->data;
	x11(s->xgc=XCreateGC(s->wh.dpy, s->pm? s->pm:s->wh.win, 0, NULL));
	return 0;

	finally:

	#endif

	fbx_term(s);
	return -1;
}

int fbx_read(fbx_struct *s, int winx, int winy)
{
	int wx, wy;
	#ifdef FBXWIN32
	fbx_gc gc;
	#elif !defined(__APPLE__)
	int x=0, y=0;  XWindowAttributes xwinattrib, rwinattrib;  Window dummy;
	#endif
	if(!s) _throw("Invalid argument");
	wx=winx>=0?winx:0;  wy=winy>=0?winy:0;

	#ifdef FBXWIN32
	if(!s->hmdc || s->width<=0 || s->height<=0 || !s->bits || !s->wh) _throw("Not initialized");
	w32(gc=GetDC(s->wh));
	w32(BitBlt(s->hmdc, 0, 0, s->width, s->height, gc, wx, wy, SRCCOPY));
	w32(ReleaseDC(s->wh, gc));
	return 0;
	#else
	if(!s->wh.dpy || !s->wh.win || !s->xi || !s->bits) _throw("Not initialized");
	#ifdef USESHM
	if(!s->xattach && s->shm) {x11(XShmAttach(s->wh.dpy, &s->shminfo));  s->xattach=1;}
	#endif

	#ifndef __APPLE__
	x11(XGetWindowAttributes(s->wh.dpy, s->wh.win, &xwinattrib));
	if(s->wh.win!=xwinattrib.root && s->format!=FBX_INDEX)
	{
		x11(XGetWindowAttributes(s->wh.dpy, xwinattrib.root, &rwinattrib));
		XTranslateCoordinates(s->wh.dpy, s->wh.win, xwinattrib.root, 0, 0, &x, &y, &dummy);
		x=wx+x;  y=wy+y;
		if(x<0) x=0;  if(y<0) y=0;
		if(x>rwinattrib.width-s->width) x=rwinattrib.width-s->width;
		if(y>rwinattrib.height-s->height) y=rwinattrib.height-s->height;
		#ifdef USESHM
		if(s->shm)
		{
			x11(XShmGetImage(s->wh.dpy, xwinattrib.root, s->xi, x, y, AllPlanes));
		}
		else
		#endif
		{
			x11(XGetSubImage(s->wh.dpy, xwinattrib.root, x, y, s->width, s->height, AllPlanes, ZPixmap, s->xi, 0, 0));
		}
	}
	else
	#endif
	{
		#ifdef USESHM
		if(s->shm)
		{
			x11(XShmGetImage(s->wh.dpy, s->wh.win, s->xi, wx, wy, AllPlanes));
		}
		else
		#endif
		{
			x11(XGetSubImage(s->wh.dpy, s->wh.win, wx, wy, s->width, s->height, AllPlanes, ZPixmap, s->xi, 0, 0));
		}
	}
	return 0;
	#endif

	finally:
	return -1;
}

int fbx_write(fbx_struct *s, int bmpx, int bmpy, int winx, int winy, int w, int h)
{
	int bx, by, wx, wy, bw, bh;
	#ifdef FBXWIN32
	BITMAPINFO bmi;  fbx_gc gc;
	#endif
	if(!s) _throw("Invalid argument");

	bx=bmpx>=0?bmpx:0;  by=bmpy>=0?bmpy:0;  bw=w>0?w:s->width;  bh=h>0?h:s->height;
	wx=winx>=0?winx:0;  wy=winy>=0?winy:0;
	if(bw>s->width) bw=s->width;  if(bh>s->height) bh=s->height;
	if(bx+bw>s->width) bw=s->width-bx;  if(by+bh>s->height) bh=s->height-by;

	#ifdef FBXWIN32
	if(!s->wh || s->width<=0 || s->height<=0 || !s->bits) _throw("Not initialized");
	memset(&bmi, 0, sizeof(bmi));
	bmi.bmiHeader.biSize=sizeof(bmi);
	bmi.bmiHeader.biWidth=s->width;
	bmi.bmiHeader.biHeight=-s->height;
	bmi.bmiHeader.biPlanes=1;
	bmi.bmiHeader.biBitCount=fbx_ps[s->format]*8;
	bmi.bmiHeader.biCompression=BI_RGB;
	w32(gc=GetDC(s->wh));
	w32(SetDIBitsToDevice(gc, wx, wy, bw, bh, bx, 0, 0, bh, &s->bits[by*s->pitch], &bmi,
		DIB_RGB_COLORS));
	w32(ReleaseDC(s->wh, gc));
	return 0;
	#else
	if(fbx_awrite(s, bmpx, bmpy, winx, winy, w, h)==-1) return -1;
	if(s->pm)
	{
		XCopyArea(s->wh.dpy, s->pm, s->wh.win, s->xgc, wx, wy, bw, bh, wx, wy);
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

	bx=bmpx>=0?bmpx:0;  by=bmpy>=0?bmpy:0;  bw=w>0?w:s->width;  bh=h>0?h:s->height;
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

#ifndef FBXWIN32
int fbx_awrite(fbx_struct *s, int bmpx, int bmpy, int winx, int winy, int w, int h)
{
	int bx, by, wx, wy, bw, bh;
	if(!s) _throw("Invalid argument");
	bx=bmpx>=0?bmpx:0;  by=bmpy>=0?bmpy:0;  bw=w>0?w:s->width;  bh=h>0?h:s->height;
	wx=winx>=0?winx:0;  wy=winy>=0?winy:0;
	if(bw>s->width) bw=s->width;  if(bh>s->height) bh=s->height;
	if(bx+bw>s->width) bw=s->width-bx;  if(by+bh>s->height) bh=s->height-by;
	if(!s->wh.dpy || !s->wh.win || !s->xi || !s->bits) _throw("Not initialized");
	#ifdef USESHM
	if(s->shm)
	{
		if(!s->xattach) {x11(XShmAttach(s->wh.dpy, &s->shminfo));  s->xattach=1;}
		x11(XShmPutImage(s->wh.dpy, s->wh.win, s->xgc, s->xi, bx, by, wx, wy, bw, bh, False));
	}
	else
	#endif
	XPutImage(s->wh.dpy, s->pm, s->xgc, s->xi, bx, by, wx, wy, bw, bh);
	return 0;

	finally:
	return -1;
}
#endif

int fbx_sync(fbx_struct *s)
{
	#ifdef FBXWIN32
	return 0;
	#else
	if(!s) _throw("Invalid argument");
	if(s->pm)
	{
		XCopyArea(s->wh.dpy, s->pm, s->wh.win, s->xgc, 0, 0, s->width, s->height, 0, 0);
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
	#ifdef FBXWIN32
	if(s->hdib) DeleteObject(s->hdib);
	if(s->hmdc) DeleteDC(s->hmdc);
	#else
	if(s->pm) {XFreePixmap(s->wh.dpy, s->pm);  s->pm=0;}
	if(s->xi) 
	{
		if(s->xi->data && !s->shm) {free(s->xi->data);  s->xi->data=NULL;}
		XDestroyImage(s->xi);
	}
	#ifdef USESHM
	if(s->shm)
	{
		if(s->xattach) {XShmDetach(s->wh.dpy, &s->shminfo);  XSync(s->wh.dpy, False);}
		#ifdef XWIN32
		if(s->shminfo.shmaddr!=NULL) UnmapViewOfFile(s->shminfo.shmaddr);
		if(s->filemap) CloseHandle(s->filemap);
		#else
		if(s->shminfo.shmaddr!=NULL) shmdt(s->shminfo.shmaddr);
		if(s->shminfo.shmid!=-1) shmctl(s->shminfo.shmid, IPC_RMID, 0);
		#endif
	}
	#endif
	if(s->xgc) XFreeGC(s->wh.dpy, s->xgc);
	#endif
	memset(s, 0, sizeof(fbx_struct));
	return 0;

	finally:
	return -1;
}

#if defined(XDK) && defined(USESHM) && !defined(FBXWIN32)
static int fbx_checkdlls(void)
{
	int retval=1, v1, v2, v3, v4;
	static int alreadywarned=0;
	if(fbx_checkdll("hclshm.dll", &v1, &v2, &v3, &v4))
	{
		if(v1<9 || (v1==9 && v2==0 && v3==0 && v4<1))
		{
			if(!alreadywarned && __warningfile)
			{
				fprintf(__warningfile, "[FBX] WARNING: Installed version of hclshm.dll is %d.%d.%d.%d.\n",
					v1, v2, v3, v4);
				fprintf(__warningfile, "[FBX]    Need version >= 9.0.0.1 for shared memory drawing\n");
			}
			retval=0;
		}
	}
	if(fbx_checkdll("xlib.dll", &v1, &v2, &v3, &v4))
	{
		if(v1<9 || (v1==9 && v2==0 && v3==0 && v4<3))
		{
			if(!alreadywarned && __warningfile)
			{
				fprintf(__warningfile, "[FBX] WARNING: Installed version of xlib.dll is %d.%d.%d.%d.\n",
					v1, v2, v3, v4);
				fprintf(__warningfile, "[FBX]    Need version >= 9.0.0.3 for shared memory drawing\n");
			}
			retval=0;
		}
	}
	if(fbx_checkdll("exceed.exe", &v1, &v2, &v3, &v4))
	{
		if(v1<8 || (v1==8 && v2==0 && v3==0 && v4<28))
		{
			if(!alreadywarned && __warningfile)
			{
				fprintf(__warningfile, "[FBX] WARNING: Installed version of exceed.exe is %d.%d.%d.%d.\n",
					v1, v2, v3, v4);
				fprintf(__warningfile, "[FBX]    Need version >= 8.0.0.28 for shared memory drawing\n");
			}
			retval=0;
		}
		if(v1==9 && v2==0 && v3==0 && v4<9)
		{
			if(!alreadywarned && __warningfile)
			{
				fprintf(__warningfile, "[FBX] WARNING: Installed version of exceed.exe is %d.%d.%d.%d.\n",
					v1, v2, v3, v4);
				fprintf(__warningfile, "[FBX]    Need version >= 9.0.0.9 for shared memory drawing\n");
			}
			retval=0;
		}
	}
	if(!retval && !alreadywarned && __warningfile)
		fprintf(__warningfile, "[FBX] WARNING: Exceed patches not installed.  Disabling shared memory drawing\n");
	if(!alreadywarned) alreadywarned=1;
	return retval;
}

static int fbx_checkdll(char *filename, int *v1, int *v2, int *v3, int *v4)
{
	unsigned long vinfolen, vinfohnd=0;
	char *vinfo=NULL, temps[256], *p;
	void *buf=NULL;
	unsigned int len;
	int retval=1;

	vinfolen=GetFileVersionInfoSize(filename, &vinfohnd);
	if(!vinfolen) {retval=0;  goto ret;}
	if((vinfo=(char *)malloc(vinfolen))==NULL) {retval=0;  goto ret;}
	if(!GetFileVersionInfo(filename, vinfohnd, vinfolen, vinfo))
		{retval=0;  goto ret;}
	if(VerQueryValue(vinfo, "\\VarFileInfo\\Translation", &buf, &len)
		&& len==4)
	{
		p=(char *)buf;
		sprintf(temps, "\\StringFileInfo\\%04X%04X\\FileVersion",
			*((WORD *)p), *((WORD *)&p[2]));
	}
	else sprintf(temps, "\\StringFileInfo\\%04X04B0\\FileVersion",
		GetUserDefaultLangID());
	if(VerQueryValue(vinfo, temps, &buf, &len) && len>0)
	{
		if(sscanf((char *)buf, "%d.%d.%d.%d", v1, v2, v3, v4)!=4)
			retval=0;
		else retval=1;
	}
	else retval=0;

	ret:
	if(vinfo) free(vinfo);
	return retval;
}
#endif
