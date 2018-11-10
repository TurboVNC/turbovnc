TurboVNC Viewer
===============

The TurboVNC Viewer was originally based on the Java TigerVNC Viewer, but it
contains numerous additional features and GUI modifications that make it behave
and perform like a native VNC viewer.


The TurboVNC Helper
-------------------

When run as a standalone application, the TurboVNC Viewer implements some
low-level platform-specific features by way of a bundled JNI library called the
"TurboVNC Helper."  The following features require the TurboVNC Helper:

### All Platforms

* Accelerated JPEG decompression using libjpeg-turbo

### Windows

* Keyboard grabbing (passing Alt-Tab and other special keystrokes to the
  server)
* Pageant support (for password-less SSH public key authentication)

### Mac

* Drawing tablet support
* ssh-agent support (for password-less SSH public key authentication)

### Linux/Un*x

* Keyboard grabbing
* Remote X Input/extended input device support
* Multi-screen spanning in full-screen mode
* ssh-agent support (for password-less SSH public key authentication)

You can verify that the TurboVNC Helper has been loaded by checking the
"Connection Info" dialog (after the connection to the server has been
established.)  If you are launching the TurboVNC Viewer from the command line,
then it will also print a warning if it is unable to load the helper.


Blitting Performance
--------------------

The TurboVNC Viewer includes a benchmark that can be used to diagnose
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


Acknowledgements
----------------

Screenshot icon in the TurboVNC Viewer toolbar is by VisualPharm and is used
under the terms of the Creative Commons Attributon license:
<https://www.shareicon.net/screenshot-586081>

Lock/unlock icons in the TurboVNC Viewer authentication dialog are from
[Open Iconic](https://useiconic.com/open), which is:

    Copyright (c) 2014 Waybury

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
