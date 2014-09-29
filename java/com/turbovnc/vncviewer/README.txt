
Java TurboVNC Viewer
====================

The Java TurboVNC Viewer is based largely on the Java TigerVNC Viewer, but it
contains additional features and GUI modifications that make it behave and
perform as much like the TurboVNC native viewers as possible.  One of the
most notable of these features is the ability to use the libjpeg-turbo library
(through JNI) to accelerate JPEG decoding, giving the Java TurboVNC Viewer,
when run as a standalone application, similar levels of performance to the
native TurboVNC viewers.

The Java TurboVNC Viewer is:

        Copyright (C) 2000-2003 Constantin Kaplinsky
        Copyright (C) 2004 Red Hat, Inc.
        Copyright (C) 2004-2005 Cendio AB
        Copyright (C) 2002-2005 RealVNC Ltd.
        Copyright (C) 2005-2006 Martin Koegler
        Copyright (C) 2006 OCCAM Financial Technology
        Copyright (C) 2009-2011 Pierre Ossman for Cendio AB
        Copyright (C) 2010-2012 TigerVNC Team
        Copyright (C) 2010 m-privacy GmbH
        Copyright (C) 2011-2013 Brian P. Hinz
        Copyright (C) 2011-2014 D. R. Commander
        All rights reserved.

This software is distributed under the GNU General Public Licence as
published by the Free Software Foundation.  See the file LICENSE.txt for the
conditions under which this software is made available.  TurboVNC also
contains code from other sources.  See the Acknowledgements section below and
the individual files for details of the conditions under which they are made
available.


NOTE: On Mac platforms, the Java TurboVNC Viewer is bundled as a self-contained
app and can be launched by opening the "TurboVNC Viewer" app located in the
"TurboVNC" Applications folder or by running /opt/TurboVNC/bin/vncviewer from a
Terminal window.  On Linux/Un*x platforms, the Java TurboVNC Viewer can be
launched by running /opt/TurboVNC/bin/vncviewer-java from a command prompt.  On
Windows platforms, the Java TurboVNC Viewer can be launched by selecting "Java
TurboVNC Viewer" in the "TurboVNC" Start Menu group or by running
c:\Program Files\TurboVNC\vncviewer-java.bat from a command prompt.  If using
the Java TurboVNC Viewer in this manner, then the instructions below do not
apply.  See the TurboVNC User's Guide for more information.


Installation
============

There are three basic ways to use the Java TurboVNC Viewer:

  1. Running the applet as part of a TurboVNC Server installation.

     The TurboVNC Server includes a small built-in HTTP server that can serve
     the Java TurboVNC Viewer to web clients.  This enables easy access to the
     remote desktop without the need to install any software on the client
     machine.

     The TurboVNC Server (Xvnc) is able to serve up any set of files that
     are present in a particular directory, which is specified in the -httpd
     argument to Xvnc.  In the default version of the vncserver script, this
     argument is set to ../java, relative to the directory containing the
     vncserver script.  Thus, one can easily deploy a modified version of
     the Java TurboVNC Viewer by simply copying a new JAR file into this
     directory.

  2. Running the applet from a standalone web server.

     Another possibility for using the Java TurboVNC Viewer is to install it
     under a fully-functional HTTP server, such as Apache or IIS.  Due to Java
     security restrictions, the applet must be signed in order for it to
     connect to a VNC server running on a different machine from the HTTP
     server.

     One can install the Java TurboVNC Viewer by simply copying the .jar file
     into a directory that is under the control of the HTTP server.  Also, an
     HTML page should be created to act as a the base document for the Java
     TurboVNC Viewer applet (see the "Parameters" section below for example
     HTML code.)

  3. Running the viewer as a standalone application.

     Finally, the Java TurboVNC Viewer can be executed locally on the client
     machine, but this method requires installation of either a JRE (Java
     Runtime Environment) or a JDK (Java Development Kit).  If VncViewer.jar is
     in the current directory, then the Java TurboVNC Viewer can be launched
     with the following command line:

         java -jar VncViewer.jar [parameters]

     Add an argument of -? to the above command line to print a list of
     optional parameters supported by VncViewer.


Parameters
==========

The Java TurboVNC Viewer accepts a number of optional parameters, allowing you
to customize its behavior.

Parameters can be specified in one of the two ways, depending on how the Java
TurboVNC Viewer is used:

  1. When the Java TurboVNC Viewer is run as an applet (embedded within an HTML
     document), parameters should be specified using the <PARAM> HTML tags
     within the appropriate <APPLET> section.  Example:

     <APPLET CODE=com.turbovnc.vncviewer.VncViewer ARCHIVE=VncViewer.jar
       WIDTH=400 HEIGHT=300>
       <PARAM NAME="PORT" VALUE=5901>
       <PARAM NAME="Scale" VALUE=50>
     </APPLET>

  2. When run as a standalone application, the Java TurboVNC Viewer reads
     parameters from the command line.  Example:

     java -jar VncViewer.jar Port=5901 Scale=50

     or

     java -jar VncViewer.jar -port 5901 -scale 50

Both parameter names and their values are case-insensitive.  The only
exception is the "Password" parameter, as VNC passwords are case-sensitive.

For a complete list of parameters and their descriptions, execute:

     java -jar VncViewer.jar -?


ACKNOWLEDGEMENTS
================

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
