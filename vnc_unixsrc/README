
  TightVNC version 1.3.10
  Source distribution for Unix platforms

======================================================================

This distribution is based on the standard VNC source and includes new
TightVNC-specific features and fixes, such as additional low-bandwidth
optimizations, major GUI improvements, and more.

	Copyright (C) 1999 AT&T Laboratories Cambridge.
	Copyright (C) 2000 Tridia Corp.
	Copyright (C) 2002-2003 RealVNC Ltd.
	Copyright (C) 2001-2004 HorizonLive.com, Inc.
	Copyright (C) 2000-2006 Constantin Kaplinsky
	Copyright (C) 2000-2009 TightVNC Group
	All rights reserved.

This software is distributed under the GNU General Public Licence as published
by the Free Software Foundation.  See the file LICENCE.TXT for the conditions
under which this software is made available.  VNC also contains code from other
sources.  See the Acknowledgements section below, and the individual files for
details of the conditions under which they are made available.


There are five programs here:

	vncviewer - this is the VNC viewer, or client, program for X.

	vncserver - this is a wrapper script which makes starting an X VNC
		    server (i.e. desktop) more convenient.  It is written in
		    Perl, so to use the script you need that.

	vncpasswd - this program allows you to change the password used to
		    access your X VNC desktops.  The vncserver script uses
		    this program when you first start a VNC server.

	vncconnect - this program tells a running instance of Xvnc to connect
		     to a listening VNC viewer (normally the connection is made
		     the other way round i.e. the viewer connects to Xvnc).

	Xvnc - this is the X VNC server - it is both an X server and a VNC
	       server.  You normally use the vncserver script to start Xvnc.


First you must have a reasonably recent version of X installed (this includes
/usr/openwin on Solaris machines).  Also, TightVNC requires JPEG and zlib
libraries installed in the system (e.g. under /usr/local).  To build
everything but Xvnc, do:

	% xmkmf
	% make World

This should build first the vncauth library which is used by each of the
programs, then vncviewer, vncpasswd and vncconnect.

Xvnc differs from the other programs in that it is built inside a cut-down
version of the X build tree.  This is based around the XFree86 3.3.2 "server
only" distribution, which in turn is based on the X11R6.3 distribution from
the X consortium.  To build Xvnc, do:

	% cd Xvnc
	% ./configure
	% make

If you have trouble building Xvnc, see the Xvnc/README file for more details.

If it all builds OK you should copy the programs to some directory which
is in your PATH environment variable, such as /usr/local/bin.  Also, it's
handy to install manual pages in a directory where the man utility can find
them.  You can use the vncinstall script to do this for you (man path is
optional):

	% cd ..
	% ./vncinstall /usr/local/bin /usr/local/man

If you want to use the Java VNC viewer, you should copy the class files from
the classes directory to some suitable installation directory such as
/usr/local/vnc/classes:

	% mkdir -p /usr/local/vnc/classes
	% cp classes/* /usr/local/vnc/classes

We recommend that you use the vncserver script to run Xvnc for you.  You can
edit the script as appropriate for your site.  Things you may need to change
include:

 * The location of Perl - if Perl is not installed in /usr/bin you'll need
   to edit the "#!/usr/bin/perl" first line of vncserver.

 * $vncClasses - this specifies the location of the Java classes for
   the VNC viewer applet.  The default is /usr/local/vnc/classes.

 * Xvnc's font path and color database.  If you have an installation of
   X which is not in the standard place you may need to add arguments to the
   Xvnc command line to set these.  These should be appended to the $cmd
   variable at the comment "# Add font path and color database...".



ACKNOWLEDGEMENTS
================

This distribution contains public domain DES software by Richard Outerbridge.
This is:

    Copyright (c) 1988,1989,1990,1991,1992 by Richard Outerbridge.
    (GEnie : OUTER; CIS : [71755,204]) Graven Imagery, 1992.


This distribution contains software from the X Window System, Version 11,
Release 6.3.  This is:

    Copyright c 1996 X Consortium
    
    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish, dis-
    tribute, sublicense, and/or sell copies of the Software, and to permit
    persons to whom the Software is furnished to do so, subject to the fol-
    lowing conditions:
    
    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABIL-
    ITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT
    SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABIL-
    ITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
    
    Except as contained in this notice, the name of the X Consortium shall
    not be used in advertising or otherwise to promote the sale, use or
    other dealings in this Software without prior written authorization from
    the X Consortium.
    
    X Window System is a trademark of X Consortium, Inc.


This distribution contains Java DES software by Dave Zimmerman
<dzimm@widget.com> and Jef Poskanzer <jef@acme.com>.  This is:

    Copyright (c) 1996 Widget Workshop, Inc. All Rights Reserved.

    Permission to use, copy, modify, and distribute this software and its
    documentation for NON-COMMERCIAL or COMMERCIAL purposes and without fee
    is hereby granted, provided that this copyright notice is kept intact.
    
    WIDGET WORKSHOP MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE
    SUITABILITY OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT
    NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
    PARTICULAR PURPOSE, OR NON-INFRINGEMENT. WIDGET WORKSHOP SHALL NOT BE
    LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING,
    MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
    
    THIS SOFTWARE IS NOT DESIGNED OR INTENDED FOR USE OR RESALE AS ON-LINE
    CONTROL EQUIPMENT IN HAZARDOUS ENVIRONMENTS REQUIRING FAIL-SAFE
    PERFORMANCE, SUCH AS IN THE OPERATION OF NUCLEAR FACILITIES, AIRCRAFT
    NAVIGATION OR COMMUNICATION SYSTEMS, AIR TRAFFIC CONTROL, DIRECT LIFE
    SUPPORT MACHINES, OR WEAPONS SYSTEMS, IN WHICH THE FAILURE OF THE
    SOFTWARE COULD LEAD DIRECTLY TO DEATH, PERSONAL INJURY, OR SEVERE
    PHYSICAL OR ENVIRONMENTAL DAMAGE ("HIGH RISK ACTIVITIES").  WIDGET
    WORKSHOP SPECIFICALLY DISCLAIMS ANY EXPRESS OR IMPLIED WARRANTY OF
    FITNESS FOR HIGH RISK ACTIVITIES.

    Copyright (C) 1996 by Jef Poskanzer <jef@acme.com>.  All rights
    reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS
    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
    OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Visit the ACME Labs Java page for up-to-date versions of this and other
    fine Java utilities: http://www.acme.com/java/
