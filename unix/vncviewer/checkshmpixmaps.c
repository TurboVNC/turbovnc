/*  Copyright (C) 2013, 2015, 2018-2019 D. R. Commander.  All Rights Reserved.
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 *  USA.
 */

#ifdef __SUNPRO_C
/* Oracle Developer Studio sometimes erroneously detects the _error() or
   _warning*() macro followed by a semicolon as an unreachable statement. */
#pragma error_messages(off, E_STATEMENT_NOT_REACHED)
#endif

#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/Xutil.h>


#define _error(m) {  \
  fprintf(stderr, "ERROR when testing for MIT-SHM Pixmap support:\n%s\n", m);  \
  goto bailout;  \
}

#define _warning(m) {  \
  fprintf(stderr, "WARNING: %s\nPerformance of TurboVNC Viewer may suffer.\n", m);  \
  goto bailout;  \
}

#define _warningdb(m) {  \
  fprintf(stderr, "WARNING: %s\nPerformance of TurboVNC Viewer may suffer if Swing double buffering is enabled.\n", m);  \
  goto bailout;  \
}


unsigned long serial = 0;
int shmok = 1;
XErrorHandler prevhandler = NULL;


#ifndef X_ShmAttach
#define X_ShmAttach 1
#endif

#ifndef X_ShmCreatePixmap
#define X_ShmCreatePixmap 5
#endif


static int xhandler(Display *dpy, XErrorEvent *e)
{
  if (e->serial == serial && (e->minor_code == X_ShmAttach &&
                              e->error_code == BadAccess)) {
    shmok = 0;
    return 0;
  }
  if (e->serial == serial && (e->minor_code == X_ShmCreatePixmap &&
                              e->error_code == BadImplementation)) {
    shmok = 0;
    return 0;
  }
  if (prevhandler && prevhandler != xhandler)
    return prevhandler(dpy, e);
  else
    return 0;
}


#define trapx11(f)  \
  XSync(dpy, False);  \
  prevhandler = XSetErrorHandler(xhandler);  \
  shmok = 1;  \
  serial = NextRequest(dpy);  \
  f;  \
  XSync(dpy, False);  \
  XSetErrorHandler(prevhandler);


int main(void)
{
  Display *dpy = NULL;
  Window win = 0;
  XSetWindowAttributes wattrs;
  Colormap cmap;
  XShmSegmentInfo shminfo;
  int attached = 0, screen, depth;
  Pixmap pm = 0;

  memset(&shminfo, 0, sizeof(shminfo));
  shminfo.shmid = -1;

  dpy = XOpenDisplay(0);
  if (!dpy)
    _error("Could not open X display.");
  screen = DefaultScreen(dpy);
  depth = DefaultDepth(dpy, screen);

  cmap = XCreateColormap(dpy, DefaultRootWindow(dpy),
                         DefaultVisual(dpy, DefaultScreen(dpy)), AllocNone);
  wattrs.background_pixel = 0;
  wattrs.border_pixel = 0;
  wattrs.colormap = cmap;
  wattrs.event_mask = 0;
  win = XCreateWindow(dpy, DefaultRootWindow(dpy), 0, 0, 1, 1, 1, depth,
                      InputOutput, DefaultVisual(dpy, screen),
                      CWBackPixel | CWBorderPixel | CWEventMask | CWColormap,
                      &wattrs);
  if (!win)
    _error("Could not create X window");

  if (!XShmQueryExtension(dpy))
    _warning("MIT-SHM X extension is not available.");

  shminfo.readOnly = False;
  shminfo.shmid = shmget(IPC_PRIVATE, 10, IPC_CREAT | 0777);
  if (shminfo.shmid == -1)
    _warning("MIT-SHM extension is not working properly.");
  shminfo.shmaddr = (char *)shmat(shminfo.shmid, 0, 0);
  if (shminfo.shmaddr == (char *)-1)
    _warning("MIT-SHM extension is not working properly.");

  trapx11(XShmAttach(dpy, &shminfo))
  if (!shmok)
    _warning("MIT-SHM X extension failed to initialize (this is normal on remote\n         connections.)");
  attached = 1;

  trapx11(pm = XShmCreatePixmap(dpy, win, shminfo.shmaddr, &shminfo, 1, 1,
                                DefaultDepth(dpy, DefaultScreen(dpy))))
  if (!shmok) {
    pm = 0;
    _warningdb("MIT-SHM Pixmaps are not enabled.");
  }

  fprintf(stderr, "MIT-SHM Pixmaps are available and working properly.\n");

  bailout:
  if (pm) XFreePixmap(dpy, pm);
  if (attached) {
    XShmDetach(dpy, &shminfo);
    XSync(dpy, False);
  }
  if (shminfo.shmaddr != NULL) shmdt(shminfo.shmaddr);
  if (shminfo.shmid != -1) shmctl(shminfo.shmid, IPC_RMID, 0);
  if (win) XDestroyWindow(dpy, win);
  if (dpy) XCloseDisplay(dpy);
  return 0;
}
