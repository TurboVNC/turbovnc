TurboVNC 0.5 Binary Distribution for Windows platforms
======================================================

TurboVNC is a highly-optimized version of TightVNC 1.3 designed for real-time
video applications (specifically VirtualGL, which performs real-time 3D
rendering on a dedicated server and outputs the results as an interactive video
stream.)  TurboVNC supports only tight encoding and true color displays, and
the tight encoding features are accelerated greatly via. the use of TurboJPEG,
the high-performance JPEG codec provided by The VirtualGL Project.

In addition to an optimized codec, TurboVNC also adds double buffering support
to the client as well as minor protocol tweaks which allow interactive
performance over broadband without breaking compatibility with TightVNC.
TurboVNC is currently the best performing solution for enabling the use of
VirtualGL on high-latency, low-bandwidth networks.  On high-speed local
networks, VirtualGL's "direct" mode is a bit faster, but TurboVNC is easier to
deploy, since it does not require an X server to be running on the client
machine.  TurboVNC also provides rudimentary collaboration capabilities.

TurboVNC can be used as a more general-purpose remote access solution for 2D
applications, and in particular it is a good choice for applications that
require high performance with high image quality.  But it should be understood
that TurboVNC is designed to support only one fast path of operation, thus it
is not as "general-purpose" as other VNC distributions.

For more information on VirtualGL and TurboVNC, visit:

http://www.virtualgl.org


TightVNC 1.3.9 Binary Distribution for Windows platforms
========================================================

This distribution is based on the standard VNC source and includes new
TightVNC-specific features and fixes, such as additional low-bandwidth
optimizations, major GUI improvements, file transfers, and more.

Executable files included in the release:

   Complete TightVNC server:  winvnc.exe, VNCHooks.dll
   TightVNC Viewer:           vncviewer.exe

TightVNC is available under the terms of the GNU General Public
License (GPL), inclided in the file LICENCE.txt. You can freely use
the software for any legal purpose, but we're asking for donations to
the TightVNC project from commercial users, as well as from motivated
individuals, who like the software. You can donate any amount of your
choice:

   http://www.tightvnc.com/donate.html

Your support ensures we can add more new features and fix more bugs,
making TightVNC a better software for you. Thank you in advance!

Please visit the project homepage at the following URL for more info:

   http://www.tightvnc.com/

