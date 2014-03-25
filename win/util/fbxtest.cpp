/* Copyright (C)2004 Landmark Graphics Corporation
 * Copyright (C)2005, 2006 Sun Microsystems, Inc.
 * Copyright (C)2011, 2013-2014 D. R. Commander
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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "Thread.h"
#include "vglutil.h"
#include "Timer.h"
#include "fbx.h"
#ifndef _WIN32
 #include <errno.h>
 #include "x11err.h"
#endif
#include "bmp.h"

using namespace vglutil;

#ifdef _MSC_VER
#define snprintf(str, n, format, ...) \
	_snprintf_s(str, n, _TRUNCATE, format, __VA_ARGS__)
#endif


#ifndef _WIN32
extern "C" {
int xhandler(Display *dpy, XErrorEvent *xe)
{
	fprintf(stderr, "X11 Error: %s\n", x11error(xe->error_code));
	return 0;
}
} // extern "C"
#endif


#define bench_name		"FBXtest"

#define MIN_SCREEN_WIDTH  1024
#define MIN_SCREEN_HEIGHT 768
#define WIDTH             1240
#define HEIGHT            900
#define N                 2

int width, height;
int checkdb=0, doshm=1, dofs=0, dovid=0, dodisplay=0, interactive=0,
	advance=0, dostress=0, offset;
#ifndef _WIN32
int dopixmap=0;
Window win=0;
#endif
fbx_wh wh;
Timer timer, timer2;
#ifdef _WIN32
#define fg() SetForegroundWindow(wh)
#else
#define fg()
#endif

const BMPPIXELFORMAT fb2bmpformat[FBX_FORMATS]=
	{BMP_RGB, BMP_RGBX, BMP_BGR, BMP_BGRX, BMP_XBGR, BMP_XRGB, BMP_RGB};

void nativeread(int), nativewrite(int);


void initbuf(int x, int y, int w, int pitch, int h, int format,
	unsigned char *buf, int offset)
{
	int i, j, ps=fbx_ps[format];
	for(i=0; i<h; i++)
	{
		for(j=0; j<w; j++)
		{
			memset(&buf[i*pitch+j*ps], 0, fbx_ps[format]);
			if(format==FBX_INDEX)
				buf[i*pitch+j]=(j+x+i+y+offset)%32;
			else
			{
				buf[i*pitch+j*ps+fbx_roffset[format]]=(j+x+offset)%256;
				buf[i*pitch+j*ps+fbx_goffset[format]]=(i+y+offset)%256;
				buf[i*pitch+j*ps+fbx_boffset[format]]=(i+y+j+x+offset)%256;
			}
		}
	}
}


int cmpbuf(int x, int y, int w, int pitch, int h, int format,
	unsigned char *buf, int offset)
{
	int i, j, ps=fbx_ps[format];
	for(i=0; i<h; i++)
	{
		for(j=0; j<w; j++)
		{
			if(format==FBX_INDEX)
			{
				if(buf[i*pitch+j]!=(j+x+i+y+offset)%32) return 0;
			}
			else
			{
				if(buf[i*pitch+j*ps+fbx_roffset[format]]!=(j+x+offset)%256)
					return 0;
				if(buf[i*pitch+j*ps+fbx_goffset[format]]!=(i+y+offset)%256)
					return 0;
				if(buf[i*pitch+j*ps+fbx_boffset[format]]!=(i+y+j+x+offset)%256)
					return 0;
			}
		}
	}
	return 1;
}


// Makes sure the frame buffer has been cleared prior to a write
void clearfb(void)
{
	#ifdef _WIN32
	if(wh)
	{
		HDC hdc=0;  RECT rect;
		tryw32(hdc=GetDC(wh));
		tryw32(GetClientRect(wh, &rect));
		tryw32(PatBlt(hdc, 0, 0, rect.right, rect.bottom, BLACKNESS));
		tryw32(ReleaseDC(wh, hdc));
	}
	#else
	if(wh.dpy && wh.d && !dopixmap)
	{
		XSetWindowBackground(wh.dpy, wh.d, BlackPixel(wh.dpy, DefaultScreen(wh.dpy)));
		XClearWindow(wh.dpy, wh.d);
		XSync(wh.dpy, False);
	}
	#endif
	return;
}


// Platform-specific write test
void nativewrite(int useshm)
{
	fbx_struct s;  int i=0;  double rbtime;
	memset(&s, 0, sizeof(s));

	try {

	fbx(fbx_init(&s, wh, 0, 0, useshm));
	if(useshm && !s.shm) _throw("MIT-SHM not available");
	fprintf(stderr, "Native Pixel Format:  %s\n", fbx_formatname(s.format));
	if(s.width!=width || s.height!=height)
		_throw("The benchmark window lost input focus or was obscured, or the display\nresolution is not large enough.  Skipping native write test\n");

	clearfb();
	if(useshm)
		fprintf(stderr, "FBX bottom-up write [SHM]:        ");
	else
		fprintf(stderr, "FBX bottom-up write:              ");
	i=0;  rbtime=0;  timer2.start();
	do
	{
		if(checkdb)
		{
			memset(s.bits, 255, s.pitch*s.height);
			fbx(fbx_awrite(&s, 0, 0, 0, 0, 0, 0));
		}
		initbuf(0, 0, width, s.pitch, height, s.format, (unsigned char *)s.bits,
			i);
		timer.start();
		fbx(fbx_flip(&s, 0, 0, 0, 0));
		fbx(fbx_write(&s, 0, 0, 0, 0, 0, 0));
		rbtime+=timer.elapsed();
		i++;
	} while(timer2.elapsed()<5.);
	fprintf(stderr, "%f Mpixels/sec\n",
		(double)i*(double)(width*height)/((double)1000000.*rbtime));

	clearfb();
	if(useshm)
		fprintf(stderr, "FBX 1/4 top-down write [SHM]:     ");
	else
		fprintf(stderr, "FBX 1/4 top-down write:           ");
	i=0;  rbtime=0.;  timer2.start();
	do
	{
		if(checkdb)
		{
			memset(s.bits, 255, s.pitch*s.height);
			fbx(fbx_awrite(&s, 0, 0, 0, 0, 0, 0));
		}
		initbuf(0, 0, width, s.pitch, height, s.format, (unsigned char *)s.bits,
			i);
		timer.start();
		fbx(fbx_write(&s, 0, 0, WIDTH/2, HEIGHT/2, WIDTH/2, HEIGHT/2));
		rbtime+=timer.elapsed();
		i++;
	} while(timer2.elapsed()<5.);
	fprintf(stderr, "%f Mpixels/sec\n",
		(double)i*(double)(width*height)/((double)4000000.*rbtime));

	clearfb();
	if(useshm)
		fprintf(stderr, "FBX top-down write [SHM]:         ");
	else
		fprintf(stderr, "FBX top-down write:               ");
	i=0;  rbtime=0.;  timer2.start();
	do
	{
		if(checkdb)
		{
			memset(s.bits, 255, s.pitch*s.height);
			fbx(fbx_awrite(&s, 0, 0, 0, 0, 0, 0));
		}
		initbuf(0, 0, width, s.pitch, height, s.format, (unsigned char *)s.bits,
			i);
		timer.start();
		fbx(fbx_write(&s, 0, 0, 0, 0, 0, 0));
		rbtime+=timer.elapsed();
		i++;
	} while(timer2.elapsed()<5.);
	fprintf(stderr, "%f Mpixels/sec\n",
		(double)i*(double)(width*height)/((double)1000000.*rbtime));

	} catch(Error &e) { fprintf(stderr, "%s\n", e.getMessage()); }

	offset=i-1;

	fbx_term(&s);
}


// Platform-specific readback test
void nativeread(int useshm)
{
	fbx_struct s;  int i;  double rbtime;
	memset(&s, 0, sizeof(s));

	try {

	fbx(fbx_init(&s, wh, 0, 0, useshm));
	int ps=fbx_ps[s.format];
	if(useshm && !s.shm) _throw("MIT-SHM not available");
	if(s.width!=width || s.height!=height)
		_throw("The benchmark window lost input focus or was obscured, or the display\nresolution is not large enough.  Skipping native read test\n");
	if(useshm)
		fprintf(stderr, "FBX read [SHM]:                   ");
	else
		fprintf(stderr, "FBX read:                         ");
	memset(s.bits, 0, width*height*ps);
	i=0;  rbtime=0.;  timer2.start();
	do
	{
		timer.start();
		fbx(fbx_read(&s, 0, 0));
		rbtime+=timer.elapsed();
		if(!cmpbuf(0, 0, width, s.pitch, height, s.format, (unsigned char *)s.bits,
			offset))
			_throw("ERROR: Bogus data read back.");
		i++;
	} while(timer2.elapsed()<5.);
	fprintf(stderr, "%f Mpixels/sec\n",
		(double)i*(double)(width*height)/((double)1000000.*rbtime));

	} catch(Error &e) { fprintf(stderr, "%s\n", e.getMessage()); }

	fbx_term(&s);
}


// This serves as a unit test for the FBX library
class WriteThread : public Runnable
{
	public:
		WriteThread(int myrank_, int iter_, int useshm_) : myrank(myrank_),
			iter(iter_), useshm(useshm_) {}

		void run(void)
		{
			int i;  fbx_struct stressfb;
			memset(&stressfb, 0, sizeof(stressfb));

			try {

			int mywidth, myheight, myx=0, myy=0;
			if(myrank<2) { mywidth=width/2;  myx=0; }
			else { mywidth=width-width/2;  myx=width/2; }
			if(myrank%2==0) { myheight=height/2;  myy=0; }
			else { myheight=height-height/2;  myy=height/2; }
			fbx(fbx_init(&stressfb, wh, mywidth, myheight, useshm));
			if(useshm && !stressfb.shm) _throw("MIT-SHM not available");
			initbuf(myx, myy, mywidth, stressfb.pitch, myheight, stressfb.format,
				(unsigned char *)stressfb.bits, 0);
			for(i=0; i<iter; i++)
				fbx(fbx_write(&stressfb, 0, 0, myx, myy, mywidth, myheight));

			} catch(...) { fbx_term(&stressfb);  throw; }
		}

	private:
		int myrank, iter, useshm;
};


class ReadThread : public Runnable
{
	public:
		ReadThread(int myrank_, int iter_, int useshm_) : myrank(myrank_),
			iter(iter_), useshm(useshm_) {}

		void run(void)
		{
			fbx_struct stressfb;
			memset(&stressfb, 0, sizeof(stressfb));

			try {

			int i, mywidth, myheight, myx=0, myy=0;
			if(myrank<2) { mywidth=width/2;  myx=0; }
			else { mywidth=width-width/2;  myx=width/2; }
			if(myrank%2==0) { myheight=height/2;  myy=0; }
			else { myheight=height-height/2;  myy=height/2; }
			fbx(fbx_init(&stressfb, wh, mywidth, myheight, useshm));
			if(useshm && !stressfb.shm) _throw("MIT-SHM not available");
			int ps=fbx_ps[stressfb.format];
			memset(stressfb.bits, 0, mywidth*myheight*ps);
			for(i=0; i<iter; i++)
				fbx(fbx_read(&stressfb, myx, myy));
			if(!cmpbuf(myx, myy, mywidth, stressfb.pitch, myheight, stressfb.format,
				(unsigned char *)stressfb.bits, 0))
				_throw("ERROR: Bogus data read back.");

			} catch(...) { fbx_term(&stressfb);  throw; }
		}

	private:
		int myrank, iter, useshm;
};


void nativestress(int useshm)
{
	int i, n;  double rbtime;
	Thread *t[4];

	try {

	clearfb();
	if(useshm)
		fprintf(stderr, "FBX write [multi-threaded SHM]:   ");
	else
		fprintf(stderr, "FBX write [multi-threaded]:       ");
	n=N;
	do
	{
		n+=n;
		timer.start();
		WriteThread *wt[4];
		for(i=0; i<4; i++)
		{
			wt[i]=new WriteThread(i, n, useshm);
			t[i]=new Thread(wt[i]);
			t[i]->start();
		}
		for(i=0; i<4; i++) t[i]->stop();
		for(i=0; i<4; i++) t[i]->checkError();
		for(i=0; i<4; i++)
		{
			delete t[i];  delete wt[i];
		}
		rbtime=timer.elapsed();
	} while(rbtime<1.);
	fprintf(stderr, "%f Mpixels/sec\n",
		(double)n*(double)(width*height)/((double)1000000.*rbtime));

	} catch(Error &e) { fprintf(stderr, "%s\n", e.getMessage()); }

	try {

	if(useshm)
		fprintf(stderr, "FBX read [multi-threaded SHM]:    ");
	else
		fprintf(stderr, "FBX read [multi-threaded]:        ");
	n=N;
	do
	{
		n+=n;
		timer.start();
		ReadThread *rt[4];
		for(i=0; i<4; i++)
		{
			rt[i]=new ReadThread(i, n, useshm);
			t[i]=new Thread(rt[i]);
			t[i]->start();
		}
		for(i=0; i<4; i++) t[i]->stop();
		for(i=0; i<4; i++) t[i]->checkError();
		for(i=0; i<4; i++)
		{
			delete t[i];  delete rt[i];
		}
		rbtime=timer.elapsed();
	} while(rbtime<1.);
	fprintf(stderr, "%f Mpixels/sec\n",
		(double)n*(double)(width*height)/((double)1000000.*rbtime));

	} catch(Error &e) { fprintf(stderr, "%s\n", e.getMessage()); }

	return;
}


void display(void)
{
	if(dostress)
	{
		fprintf(stderr, "-- Stress tests --\n");
		#ifndef _WIN32
		if(doshm)
		{
			fg();  nativestress(1);
		}
		#endif
		fg();  nativestress(0);
		fprintf(stderr, "\n");
		return;
	}

	fprintf(stderr, "-- Performance tests --\n");
	#ifndef _WIN32
	if(doshm)
	{
		fg();  nativewrite(1);
		fg();  nativeread(1);
	}
	#endif
	fg();  nativewrite(0);
	fg();  nativeread(0);
	fprintf(stderr, "\n");
}


#ifdef _WIN32
LRESULT CALLBACK WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg)
	{
		case WM_CREATE:
			return 0;
		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;
		case WM_CHAR:
			if((wParam==27 || wParam=='q' || wParam=='Q') && dovid)
			{
				PostQuitMessage(0);
				return 0;
			}
			break;
		case WM_PAINT:
			if(!dovid)
			{
				display();
				PostQuitMessage(0);
			}
			else
			{
				if(interactive) dodisplay=1;
				return 0;
			}
			break;
		case WM_MOUSEMOVE:
			if((wParam & MK_LBUTTON) && dovid && interactive)
			{
				dodisplay=advance=1;
				return 0;
			}
			break;
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}
#endif


void event_loop(void)
{
	fbx_struct fb[10];
	int frame=0, inc=-1, first=1;
	unsigned long frames=0;
	Timer timer;
	double elapsed, mpixels=0.;
	char temps[256];

	for(int i=0; i<10; i++)
	{
		memset(&fb[i], 0, sizeof(fb[i]));
	}

	try {

	for(int i=0; i<10; i++)
	{
		fbx(fbx_init(&fb[i], wh, 0, 0, doshm));
		snprintf(temps, 256, "frame%d.ppm", i);
		unsigned char *buf=NULL;  int tempw=0, temph=0;
		if(loadbmp(temps, &buf, &tempw, &temph, fb2bmpformat[fb[i].format],
			1, 0)==-1)
			_throw(bmpgeterr());
		int ps=fbx_ps[fb[i].format];
		for(int j=0; j<min(temph, fb[i].height); j++)
			memcpy(&fb[i].bits[fb[i].pitch*j], &buf[tempw*ps*j],
				min(tempw, fb[i].width)*ps);
		free(buf);
	}

	timer.start();
	while(1)
	{
		advance=0;  dodisplay=0;
		if(first)
		{
			dodisplay=1;  first=0;
		}

		#ifdef _WIN32

		int ret;  MSG msg;
		if((ret=GetMessage(&msg, NULL, 0, 0))==-1) { _throww32(); }
		else if(ret==0) break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		#else

		while(1)
		{
			XEvent event;
			if(XPending(wh.dpy)>0) XNextEvent(wh.dpy, &event);
			else break;
			switch (event.type)
			{
				case Expose:
					dodisplay=1;
					break;
				case KeyPress:
				{
					char buf[10];  int key;
					key=XLookupString(&event.xkey, buf, sizeof(buf), NULL, NULL);
					switch(buf[0])
					{
						case 27: case 'q': case 'Q':
							return;
					}
					break;
				}
				case MotionNotify:
					if(event.xmotion.state & Button1Mask) dodisplay=advance=1;
					break;
			}
		}

		#endif

		if(!interactive || dodisplay)
		{
			fbx(fbx_write(&fb[frame], 0, 0, 0, 0, 0, 0));
			if(!interactive || advance)
			{
				if(frame==0 || frame==9) inc=-1*inc;
				frame+=inc;  frames++;
				mpixels+=(double)fb[frame].width*(double)fb[frame].height/1000000.;

				if((elapsed=timer.elapsed())>2.0)
				{
					snprintf(temps, 256, "%f frames/sec - %f Mpixels/sec",
						(double)frames/elapsed, mpixels/elapsed);
					printf("%s\n", temps);
					timer.start();  mpixels=0.;  frames=0;
				}
			}
		}
	}

	for(int i=0; i<10; i++) fbx_term(&fb[i]);

	} catch(...)
	{
		for(int i=0; i<10; i++) fbx_term(&fb[i]);
		throw;
	}

}


void usage(char *progname)
{
	fprintf(stderr, "USAGE: %s [options]\n\n", progname);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "-checkdb = Verify that double buffering is working correctly\n");
	fprintf(stderr, "-noshm = Do not use MIT-SHM extension to accelerate blitting\n");
	fprintf(stderr, "-mt = Run multi-threaded stress tests\n");
	fprintf(stderr, "-pm = Blit to a pixmap rather than to a window\n");
	fprintf(stderr, "-v = Print all warnings and informational messages from FBX\n");
	fprintf(stderr, "-fs = Full-screen mode\n\n");
	exit(1);
}


int main(int argc, char **argv)
{
	#ifdef _WIN32
	int winstyle=WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_VISIBLE;
	#endif
	int i;

	fprintf(stderr, "\n%s v%s (Build %s)\n\n", bench_name, __VERSION, __BUILD);

	if(argc>1) for(i=1; i<argc; i++)
	{
		if(!stricmp(argv[i], "-checkdb"))
		{
			checkdb=1;
			fprintf(stderr, "Checking double buffering.  Watch for flashing to indicate that it is\n");
			fprintf(stderr, "not enabled.  Performance will be sub-optimal.\n");
		}
		if(!stricmp(argv[i], "-noshm"))
		{
			doshm=0;
		}
		if(!stricmp(argv[i], "-vid")) dovid=1;
		else if(!strnicmp(argv[i], "-v", 2))
		{
			fbx_printwarnings(stderr);
		}
		if(!stricmp(argv[i], "-i")) interactive=1;
		#ifndef _WIN32
		if(!stricmp(argv[i], "-pm"))
		{
			dopixmap=1;  doshm=0;
		}
		#endif
		if(!stricmp(argv[i], "-mt")) dostress=1;
		if(!stricmp(argv[i], "-fs"))
		{
			dofs=1;
			#ifdef _WIN32
			winstyle=WS_EX_TOPMOST | WS_POPUP | WS_VISIBLE;
			#endif
		}
		if(!strnicmp(argv[i], "-h", 2) || !stricmp(argv[i], "-?")) usage(argv[0]);
	}

	try {

	#ifdef _WIN32

	WNDCLASSEX wndclass;  MSG msg;
	wndclass.cbSize=sizeof(WNDCLASSEX);
	wndclass.style=CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc=WndProc;
	wndclass.cbClsExtra=0;
	wndclass.cbWndExtra=0;
	wndclass.hInstance=GetModuleHandle(NULL);
	wndclass.hIcon=LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor=LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName=NULL;
	wndclass.lpszClassName=bench_name;
	wndclass.hIconSm=LoadIcon(NULL, IDI_WINLOGO);
	tryw32(RegisterClassEx(&wndclass));
	width=GetSystemMetrics(SM_CXSCREEN);
	height=GetSystemMetrics(SM_CYSCREEN);

	#else

	if(!XInitThreads())
	{
		fprintf(stderr, "ERROR: Could not initialize Xlib thread safety\n");
		exit(1);
	}
	XSetErrorHandler(xhandler);
	if(!(wh.dpy=XOpenDisplay(0)))
	{
		fprintf(stderr, "Could not open display %s\n", XDisplayName(0));
		exit(1);
	}
	width=DisplayWidth(wh.dpy, DefaultScreen(wh.dpy));
	height=DisplayHeight(wh.dpy, DefaultScreen(wh.dpy));

	#endif

	if(width<MIN_SCREEN_WIDTH && height<MIN_SCREEN_HEIGHT)
	{
		fprintf(stderr,
			"ERROR: Please switch to a screen resolution of at least %d x %d.\n",
			MIN_SCREEN_WIDTH, MIN_SCREEN_HEIGHT);
		exit(1);
	}
	if(!dofs)
	{
		width=WIDTH;
		height=HEIGHT;
	}

	#ifdef _WIN32

	int bw=GetSystemMetrics(SM_CXFIXEDFRAME)*2;
	int bh=GetSystemMetrics(SM_CYFIXEDFRAME)*2+GetSystemMetrics(SM_CYCAPTION);
	tryw32(wh=CreateWindowEx(0, bench_name, bench_name, winstyle, 0, 0, width+bw,
		height+bh, NULL, NULL, GetModuleHandle(NULL), NULL));
	UpdateWindow(wh);
	BOOL ret;
	if(dovid)
	{
		event_loop();  return 0;
	}
	while(1)
	{
		if((ret=GetMessage(&msg, NULL, 0, 0))==-1) _throww32();
		else if(ret==0) break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;

	#else

	XVisualInfo vtemp, *v=NULL;  int n=0;
	XSetWindowAttributes swa;
	Window root=DefaultRootWindow(wh.dpy);

	vtemp.depth=24;  vtemp.c_class=TrueColor;
	if((v=XGetVisualInfo(wh.dpy, VisualDepthMask|VisualClassMask, &vtemp,
		&n))!=NULL && n!=0)
	{
		int mask=CWBorderPixel|CWColormap|CWEventMask;
		swa.colormap=XCreateColormap(wh.dpy, root, v->visual, AllocNone);
		swa.border_pixel=0;
		swa.event_mask=0;
		if(dofs)
		{
			mask|=CWOverrideRedirect;  swa.override_redirect=True;
		}
		if(dovid)
		{
			if(interactive)
				swa.event_mask|=PointerMotionMask|ButtonPressMask|ExposureMask;
			swa.event_mask|=KeyPressMask;
		}
		if(dopixmap)
		{
			errifnot(win=XCreateWindow(wh.dpy, root, 0, 0, 1, 1, 0, v->depth,
				InputOutput, v->visual, mask, &swa));
			errifnot(wh.d=XCreatePixmap(wh.dpy, win, width, height, v->depth));
			wh.v=v->visual;
		}
		else
		{
			errifnot(wh.d=XCreateWindow(wh.dpy, root, 0, 0, width, height, 0,
				v->depth, InputOutput, v->visual, mask, &swa));
			errifnot(XMapRaised(wh.dpy, wh.d));
		}
		if(dofs) XSetInputFocus(wh.dpy, wh.d, RevertToParent, CurrentTime);
		XSync(wh.dpy, False);
		if(dovid) event_loop();
		else display();
		if(dopixmap)
		{
			XFreePixmap(wh.dpy, wh.d);
			XDestroyWindow(wh.dpy, win);
		}
		else XDestroyWindow(wh.dpy, wh.d);
		XFree(v);  v=NULL;
	}
	else fprintf(stderr, "No RGB visuals available.  Skipping those tests.\n\n");

	if(dovid) return 0;

	vtemp.depth=8;  vtemp.c_class=PseudoColor;
	if((v=XGetVisualInfo(wh.dpy, VisualDepthMask|VisualClassMask, &vtemp,
		&n))!=NULL && n!=0)
	{
		swa.colormap=XCreateColormap(wh.dpy, root, v->visual, AllocAll);
		swa.border_pixel=0;
		swa.event_mask=0;
		XColor xc[32];  int i;
		errifnot(v->colormap_size==256);
		for(i=0; i<32; i++)
		{
			xc[i].red=(i<16? i*16:255)<<8;
			xc[i].green=(i<16? i*16:255-(i-16)*16)<<8;
			xc[i].blue=(i<16? 255:255-(i-16)*16)<<8;
			xc[i].flags=DoRed | DoGreen | DoBlue;
			xc[i].pixel=i;
		}
		XStoreColors(wh.dpy, swa.colormap, xc, 32);
		if(dopixmap)
		{
			errifnot(win=XCreateWindow(wh.dpy, root, 0, 0, 1, 1, 0,
				v->depth, InputOutput, v->visual,
				CWBorderPixel|CWColormap|CWEventMask, &swa));
			errifnot(wh.d=XCreatePixmap(wh.dpy, win, width, height, v->depth));
			wh.v=v->visual;
		}
		else
		{
			errifnot(wh.d=XCreateWindow(wh.dpy, root, 0, 0, width, height, 0,
				v->depth, InputOutput, v->visual,
				CWBorderPixel|CWColormap|CWEventMask, &swa));
			errifnot(XMapRaised(wh.dpy, wh.d));
		}
		XSync(wh.dpy, False);
		display();
		if(dopixmap)
		{
			XFreePixmap(wh.dpy, wh.d);
			XDestroyWindow(wh.dpy, win);
		}
		else XDestroyWindow(wh.dpy, wh.d);
		XFreeColormap(wh.dpy, swa.colormap);
		XFree(v);  v=NULL;
	}
	else fprintf(stderr, "No Pseudocolor visuals available.  Skipping those tests.\n\n");

	return 0;

	#endif

	}	catch(Error &e) { fprintf(stderr, "%s\n", e.getMessage()); }
}
