/* Copyright (C)2004 Landmark Graphics Corporation
 * Copyright (C)2006-2007 Sun Microsystems, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "vglutil.h"
#include "Timer.h"
#include "fbx.h"

#ifndef _WIN32
extern "C" {
#define NeedFunctionPrototypes 1
#include "dsimple.h"
}
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

using namespace vglutil;


#define _throw(f, l, m) {  \
	fprintf(stderr, "%s (%d):\n%s\n", f, l, m);  fflush(stderr);  exit(1);  \
}
#define _fbx(a) {  \
	if((a)==-1) _throw("fbx.c", fbx_geterrline(), fbx_geterrmsg());  \
}

#ifdef _WIN32
char errMsg[256];
#define _throww32(f) {  \
	if(!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),  \
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errMsg, 255, NULL))  \
		strncpy(errMsg, "Error in FormatMessage()", 256);  \
	_throw(f, __LINE__, errMsg);  \
 }
#endif


#define DEFSAMPLERATE 100
#define DEFBENCHTIME 30.0


#ifdef _WIN32
char *program_name;
#endif


void usage(void)
{
	printf("\n");
	printf("USAGE: %s [-h|-?]\n", program_name);
	printf("       [-t <seconds>] [-s <samples per second>]\n");
	printf("       [-wh <window handle in hex>] [-x <x offset>] [-y <y offset>]\n\n");
	printf("-h or -? = This help screen\n");
	printf("-t = Run the benchmark for this many seconds (default: %.1f)\n",
		DEFBENCHTIME);
	printf("-s = Sample the window this many times per second (default: %d)\n",
		DEFSAMPLERATE);
	printf("-wh = Explicitly specify a window (if auto-detect fails)\n");
	printf("-x = x coordinate (relative to window) of the sampling block\n");
	printf("-y = y coordinate (relative to window) of the sampling block\n\n");
	exit(1);
}


int main(int argc, char **argv)
{
	int i;  fbx_wh wh;
	double benchTime=DEFBENCHTIME, elapsed;  Timer timer;
	int sampleRate=DEFSAMPLERATE, x=-1, y=-1;

	program_name=argv[0];
	memset(&wh, 0, sizeof(wh));

	if(argc>1) for(i=1; i<argc; i++)
	{
		double tempf;  int temp;
		if(!stricmp(argv[i], "-h") || !stricmp(argv[i], "-?")) usage();
		if(!stricmp(argv[i], "-t") && i<argc-1)
		{
			if((tempf=atof(argv[++i]))>0.) benchTime=tempf;
		}
		if(!stricmp(argv[i], "-s") && i<argc-1)
		{
			if((temp=atoi(argv[++i]))>1) sampleRate=temp;
		}
		if(!stricmp(argv[i], "-x") && i<argc-1)
		{
			if((temp=atoi(argv[++i]))>0) x=temp;
		}
		if(!stricmp(argv[i], "-y") && i<argc-1)
		{
			if((temp=atoi(argv[++i]))>0) y=temp;
		}
		if(!stricmp(argv[i], "-wh") && i<argc-1)
		{
			#ifdef _WIN32
			fbx_wh temp;
			memset(&temp, 0, sizeof(temp));
			if(sscanf(argv[++i], "%x", &temp)==1) wh=temp;
			#else
			unsigned int temp=0;
			if(sscanf(argv[++i], "%x", &temp)==1) wh.d=(Drawable)temp;
			#endif
		}
	}

	printf("\nThin Client Benchmark\n");

	#ifdef _WIN32

	if(!wh)
	{
		printf("Click the mouse in the window that you wish to monitor ... ");
		wh=GetForegroundWindow();
		int t, tOld=-1;
		timer.start();
		do
		{
			elapsed=timer.elapsed();
			t=(int)(10.-elapsed);
			if(t!=tOld) { tOld=t;  printf("%.2d\b\b", t); }
			Sleep(50);
		} while(wh==GetForegroundWindow() && elapsed<10.);
		if((wh=GetForegroundWindow())==0)
			_throww32("Could not get foreground window")
		FlashWindow(wh, TRUE);
		printf("  \n");
	}
	char temps[1024];
	GetWindowText(wh, temps, 1024);
	printf("Monitoring window 0x%.8x (%s)\n", wh, temps);

	#else

	if(!XInitThreads())
	{
		fprintf(stderr, "XInitThreads() failed\n");
		exit(1);
	}
	if(!(wh.dpy=XOpenDisplay(0)))
	{
		fprintf(stderr, "Could not open display %s\n", XDisplayName(0));
		exit(1);
	}
	if(!wh.d)
	{
		printf("Click the mouse in the window that you wish to monitor ...\n");
		wh.d=Select_Window(wh.dpy);
		XSetInputFocus(wh.dpy, wh.d, RevertToNone, CurrentTime);
	}

	#endif

	fbx_struct fb;
	memset(&fb, 0, sizeof(fb));
	_fbx(fbx_init(&fb, wh, 0, 0, 1));
	int width=fb.width, height=fb.height;
	fbx_term(&fb);
	memset(&fb, 0, sizeof(fb));
	_fbx(fbx_init(&fb, wh, 32, 32, 1));

	int frames=0, samples=0;
	if(x<0) x=width/2-16;  if(x<0) x=0;
	if(y<0) y=height/2-16;  if(y<0) y=0;
	unsigned char buf[32*32*4];
	int first=1;

	printf("Sample block location: %d, %d\n", x, y);
	#ifdef _WIN32
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	#endif

	timer.start();
	do
	{
		_fbx(fbx_read(&fb, x, y));
		samples++;
		if(first)
		{
			first=0;  frames++;
		}
		else
		{
			if(memcmp(buf, fb.bits, fbx_ps[fb.format]*32*32)) frames++;
		}
		memcpy(buf, fb.bits, fbx_ps[fb.format]*32*32);
		elapsed=timer.elapsed();
		#ifdef _WIN32
		int sleepTime=(int)(1000.*((double)samples/(double)sampleRate-elapsed));
		if(sleepTime>0) Sleep(sleepTime);
		#else
		int sleepTime=(int)(1000000.*((double)samples/(double)sampleRate-elapsed));
		if(sleepTime>0) usleep(sleepTime);
		#endif
	} while((elapsed=timer.elapsed())<benchTime);
	#ifdef _WIN32
	SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
	#endif

	printf("Samples: %d  Frames: %d  Time: %f s  Frames/sec: %f\n", samples,
		frames, elapsed, (double)frames/elapsed);

	return 0;
}
