TurboVNC 0.1.1 Binary Distribution for Windows platforms
======================================================

This software is a stripped-down version of TightVNC 1.2.9, supporting only
tight encoding and true color displays.  The tight encoding features are
accelerated greatly via. the use of the high-performance JPEG codec provided by
the VirtualGL project.

TurboVNC is intended primarily as a high-performance X proxy for Linux
visualization applications.  It is designed to be used in conjunction with
VirtualGL's "compression-less" mode to provide excellent remote display
performance for 3D applications.  The VirtualGL/TurboVNC combination will not
perform as well in a 100 Mbit LAN environment as "standard" VirtualGL, but it
should perform much better over high-latency networks such as broadband or
satellite.  It also provides rudimentary collaboration capabilities, which are
not available in "standard" VirtualGL.

TurboVNC can be used as a more general-purpose remote access solution for 2D
applications, and in particular it is suitable for applications that require
high performance and image quality.  But it should be understood that TurboVNC
is designed to support only one fast path of operation, thus it is not as
"general-purpose" as other VNC distributions.

For more information on VirtualGL and TurboVNC, visit:

http://VirtualGL.sourceforge.net


TightVNC 1.2.9 Binary Distribution for Windows platforms
========================================================

This distribution is based on the standard VNC source with
modifications introduced in TridiaVNC 1.4.0, and includes new
TightVNC-specific features and fixes, such as additional low-bandwidth
optimizations ("Tight" encoding with optional JPEG compression, "local
cursor" feature), improved WinVNC advanced settings, and much more.

Executable files included in the release:

   Complete TightVNC server:  winvnc.exe, VNCHooks.dll
   TightVNC Viewer:           vncviewer.exe

TightVNC is available under the terms of the GNU General Public
License (GPL), inclided in the file LICENCE.txt. You can freely use
the software for any legal purpose, but we're asking for a donation to
the TightVNC project from commercial users, as well as from motivated
individuals, who like the software. You can donate any amount of your
choice starting at 10 U.S. dollars. See the details here:

   http://www.tightvnc.com/contribute.html

Your support ensures we can add more new features and fix more bugs,
making TightVNC a better software for you. Thank you in advance!
