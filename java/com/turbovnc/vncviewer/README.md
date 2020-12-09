Java TurboVNC Viewer
====================

The Java TurboVNC Viewer was originally based on the Java TigerVNC Viewer, but
it contains numerous additional features and GUI modifications that make it
behave and perform like a native VNC viewer.


Accelerated JPEG Decoding
-------------------------

The Java TurboVNC Viewer can access libjpeg-turbo through JNI to accelerate
JPEG decoding, which gives the viewer similar performance to the native viewer
in most cases.  In fact, the TurboVNC Viewer on Mac and Linux/Un\*x platforms
is simply the Java TurboVNC Viewer packaged in such a way that it behaves like
a native viewer.  The libjpeg-turbo JNI library is bundled with the official
TurboVNC packages and will automatically be loaded if the Java TurboVNC Viewer
is launched using the __vncviewer__ script (Linux/Un*x), the
__vncviewer-java.bat__ script or the Start Menu shortcut (Windows), or the
standalone TurboVNC Viewer app (Mac).  If the viewer is launched with Java Web
Start using the recommended procedure or the TurboVNC Server's built-in web
server (refer to the TurboVNC User's Guide), then the appropriate libjpeg-turbo
JNI library will be downloaded along with it, assuming that the client machine
is running Windows, OS X/macOS, or Linux.  For other deployment scenarios, the
Java TurboVNC Viewer will find the libjpeg-turbo JNI library if
[one of the official libjpeg-turbo packages](http://www.sourceforge.net/projects/libjpeg-turbo/files)
is installed on the client machine.

If you suspect for whatever reason that JPEG decoding is not being accelerated,
then the easiest way to check is to open the "Connection Info" dialog (after
the connection to the server has been established) and verify that the "JPEG
decompression" field says "Turbo".  If you are launching the Java TurboVNC
Viewer from the command line, then it will also print a warning if it is unable
to load libjpeg-turbo.


The TurboVNC Helper
-------------------

When run as a standalone application, the Java TurboVNC Viewer implements some
low-level platform-specific features by way of a bundled JNI library called the
"TurboVNC Helper."  The following features require the TurboVNC Helper:

### Windows

* Keyboard grabbing (passing Alt-Tab and other special keystrokes to the
  server)

### Mac

* Drawing tablet support

### Linux/Un*x

* Keyboard grabbing
* Remote X Input/extended input device support
* Multi-screen spanning in full-screen mode


Blitting Performance
--------------------

The Java TurboVNC Viewer includes a benchmark that can be used to diagnose
performance problems with Java 2D.  In order to run the benchmark, execute the
following command in a Command Prompt/terminal window:

### Windows

    java -Dsun.java2d.trace=count -cp "c:\Program Files\TurboVNC\Java\VncViewer.jar" com.turbovnc.vncviewer.ImageDrawTest

### Mac

    java -Dsun.java2d.trace=count -cp /Applications/TurboVNC/TurboVNC\ Viewer.app/Contents/Resources/Java/VncViewer.jar com.turbovnc.vncviewer.ImageDrawTest

### Linux/Un*x

    java -Dsun.java2d.trace=count -cp /opt/TurboVNC/java/VncViewer.jar com.turbovnc.vncviewer.ImageDrawTest

Let the benchmark run until it produces stable results, then abort it with
CTRL-C.  Looking at the Java 2D trace output will reveal which underlying API
is being used to draw the images.  This should generally be OpenGL on Mac
platforms and Windows GDI on Windows platforms.
