/*
 * vncconnect.c
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

static char *programName;

static void usage()
{
    fprintf(stderr, "usage: %s [-display Xvnc-display] host[:port]\n"
	    "Tells Xvnc to connect to a listening VNC viewer on the given"
							    " host and port\n",
	    programName);
    exit(1);
}

int main(argc, argv)
    int argc;
    char **argv;
{
    char *displayname = NULL;
    Display *dpy;
    int i;
    Atom prop;

    programName = argv[0];
    
    for (i = 1; i < argc; i++) {
	if (argv[i][0] != '-')
	    break;

	switch (argv[i][1]) {
	case 'd':			/* -display dpyname */
	    if (++i >= argc) usage();
	    displayname = argv[i];
	    break;
	default:
	    usage();
	}
    }

    if (argc != i+1)
	usage();

    if (!(dpy = XOpenDisplay(displayname))) {
	fprintf(stderr,"%s: unable to open display \"%s\"\n",
		programName, XDisplayName (displayname));
	exit(1);
    }

    prop = XInternAtom(dpy, "VNC_CONNECT", False);

    XChangeProperty(dpy, DefaultRootWindow(dpy), prop, XA_STRING, 8,
		    PropModeReplace, (unsigned char *)argv[i],
		    strlen(argv[i]));

    XCloseDisplay(dpy);

    return 0;
}
