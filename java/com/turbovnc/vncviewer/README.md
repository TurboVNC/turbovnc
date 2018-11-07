TurboVNC Viewer
===============

The TurboVNC Viewer was originally based on the Java TigerVNC Viewer, but it
contains numerous additional features and GUI modifications that make it behave
and perform like a native VNC viewer.


Accelerated JPEG Decoding
-------------------------

The TurboVNC Viewer can access libjpeg-turbo through JNI to accelerate JPEG
decoding, which gives the viewer similar performance to the native viewer in
most cases.  The libjpeg-turbo JNI library is bundled with the official
TurboVNC packages and will automatically be loaded if the TurboVNC Viewer is
launched using the `vncviewer` script (Linux/Un*x), the `vncviewer.bat` script
or the Start Menu shortcut (Windows), or the TurboVNC Viewer app (Mac).  If the
viewer is launched with Java Web Start using the recommended procedure or the
TurboVNC Server's built-in web server (refer to the TurboVNC User's Guide),
then the appropriate libjpeg-turbo JNI library will be downloaded along with
it, assuming that the client machine is running Windows, OS X, or Linux.  For
other deployment scenarios, the TurboVNC Viewer will find the libjpeg-turbo JNI
library if [one of the official libjpeg-turbo packages]
(http://www.sourceforge.net/projects/libjpeg-turbo/files) is installed on the
client machine.

If you suspect for whatever reason that JPEG decoding is not being accelerated,
then the easiest way to check is to open the "Connection Info" dialog (after
the connection to the server has been established) and verify that the "JPEG
decompression" field says "Turbo".  If you are launching the TurboVNC Viewer
from the command line, then it will also print a warning if it is unable to
load libjpeg-turbo.


The TurboVNC Helper
-------------------

When run as a standalone application, the TurboVNC Viewer implements some
low-level platform-specific features by way of a bundled JNI library called the
"TurboVNC Helper."  The following features require the TurboVNC Helper:

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


Launching the TurboVNC Viewer from a Web Browser Using Java Web Start
---------------------------------------------------------------------

__NOTE:__ [Oracle Java](http://www.java.com) 8 or [IcedTea-Web]
(https://icedtea.classpath.org/wiki/IcedTea-Web) is required on the client if
using Java Web Start.

__NOTE:__ This feature is deprecated and may be removed in a future release of
TurboVNC.

When a TurboVNC session is created, it can optionally start a miniature web
server that serves up the TurboVNC Viewer as a Java Web Start app.  This allows
you to connect to the TurboVNC session from a machine that does not have the
TurboVNC Viewer installed locally.  If one of the official TurboVNC binary
packages is installed on the host, then the miniature web server will
automatically send the appropriate x86 or x86-64 libjpeg-turbo JNI library for
Linux, OS X, or Windows clients when launching the TurboVNC Viewer. This
provides most of the advantages of the standalone TurboVNC viewers, including
native levels of performance on popular client platforms.

To launch the TurboVNC Viewer from a web browser, pass `-httpd` to
`/opt/TurboVNC/bin/vncserver` when starting a TurboVNC session, then point
your web browser to:

    http://{host}:{5800+n}

where `{host}` is the hostname or IP address of the TurboVNC host, and `n` is
the X display number of the TurboVNC session.

__Example:__ If the TurboVNC session is occupying X display `my_host:1`, then
point your web browser to:

    http://my_host:5801

This will download a JNLP file to your computer, which you can open in Java
Web Start.

You can add viewer parameters to the URL using the following format:

    http://{host}:{5800+n}?{param1}={value1}&{param2}={value2}

Example:

    http://my_host:5801?tunnel=1&samp=2x&quality=80

will tunnel the VNC connection through SSH and enable Medium-Quality JPEG.

__NOTE:__ As of Java 7 Update 51, self-signed JARs are not allowed to run in
Java Web Start by default.  This is not an issue if you are using the official
TurboVNC binary packages, but if you are building a self-signed version of the
TurboVNC Viewer for development purposes, then you will need to add
`http://{host}:{http_port}` (for example, `http://my_host:5801`) to Java's
Exception Site List, which can be found under the "Security" tab in the Java
Control Panel.

__NOTE:__ On some newer OS X systems, downloading a JNLP file may result in an
error: "xxxxxxxx.jnlp can't be opened because it it from an unidentified
developer."  To work around this, you can either open the JNLP file directly
from your Downloads folder, or you can change the application security settings
in the "Security & Privacy" section of System Preferences to allow applications
downloaded from anywhere.


Deploying the TurboVNC Viewer Using Java Web Start
--------------------------------------------------

__NOTE:__ [Oracle Java](http://www.java.com) 8 or [IcedTea-Web]
(https://icedtea.classpath.org/wiki/IcedTea-Web) is required on the client if
using Java Web Start.

Accessing the TurboVNC Viewer through TurboVNC's built-in HTTP server, as
described above, is a straightforward way of running the TurboVNC Viewer on
machines that have Java but not a VNC viewer installed (for instance, for the
purpose of collaborating with colleagues who don't normally use TurboVNC.)

To set up a large-scale deployment of the TurboVNC Viewer, however, it is
desirable to serve up the JAR files from a dedicated web server.

For the purposes of this guide, it is assumed that the reader has some
knowledge of web server administration.

* Copy the TurboVNC Viewer JAR file (`VncViewer.jar`) into a directory on
  your web server.

* Copy the libjpeg-turbo JNI JAR files into that same directory.  You can
  obtain these from one of the official TurboVNC 2.0 or later binary packages
  for Linux, or you can download `libjpeg-turbo-{version}-jws.zip` from
  [libjpeg-turbo 1.4.0 or later]
  (http://sourceforge.net/projects/libjpeg-turbo/files).  Note that only
  the JARs included in the official TurboVNC packages are signed using an
  official code signing certificate.

* __OPTIONAL:__ Copy the TurboVNC Helper JAR files into that same directory.
  You can obtain these from `turbovnc-{version}-jws.zip`, which is supplied
  with official releases of [TurboVNC 2.1.2 and later]
  (http://sourceforge.net/projects/turbovnc/files).

* __OPTIONAL:__ For large organizations, it may be desirable to obtain your
  own code signing certificate from a trusted certificate authority and use
  `jarsigner` to sign all of the JARs with that certificate.  The specifics of
  this are left as an exercise for the reader.

* Create a file called `TurboVNC.jnlp` in the same directory as
  `VncViewer.jar` on the web server, and give it the following contents:

        <?xml version="1.0" encoding="utf-8"?>
        <jnlp codebase="{turbovnc_url}">
          <information>
            <title>TurboVNC Viewer</title>
            <vendor>The VirtualGL Project</vendor>
          </information>

          <resources>
            <jar href="VncViewer.jar"/>
          </resources>

          <security>
            <all-permissions/>
          </security>

          <resources os="Mac OS X">
            <j2se version="1.8+" java-vm-args="-server"/>
            <nativelib href="ljtosx.jar"/>
            <!-- Enable drawing tablet support for OS X clients -->
            <nativelib href="tvnchelper-osx.jar"/>
          </resources>

          <resources os="Windows" arch="x86">
            <j2se version="1.8+" java-vm-args="-Dsun.java2d.d3d=false"/>
            <nativelib href="ljtwin32.jar"/>
            <!-- Enable keyboard grabbing for 32-bit Windows clients -->
            <nativelib href="tvnchelper-win32.jar"/>
          </resources>

          <resources os="Windows" arch="amd64">
            <j2se version="1.8+" java-vm-args="-Dsun.java2d.d3d=false"/>
            <nativelib href="ljtwin64.jar"/>
            <!-- Enable keyboard grabbing for 64-bit Windows clients -->
            <nativelib href="tvnchelper-win64.jar"/>
          </resources>

          <resources os="Linux" arch="i386">
            <j2se version="1.8+" java-vm-args="-server"/>
            <nativelib href="ljtlinux32.jar"/>
            <!-- Enable keyboard grabbing, multi-screen spanning, and extended
                 input device support for 32-bit Linux clients -->
            <nativelib href="tvnchelper-linux32.jar"/>
          </resources>

          <resources os="Linux" arch="amd64">
            <j2se version="1.8+"/>
            <nativelib href="ljtlinux64.jar"/>
            <!-- Enable keyboard grabbing, multi-screen spanning, and extended
                 input device support for 64-bit Linux clients -->
            <nativelib href="tvnchelper-linux64.jar"/>
          </resources>

          <application-desc main-class="com.turbovnc.vncviewer.VncViewer"/>
        </jnlp>

    __NOTE:__ `{turbovnc_url}` should be the absolute URL of the TurboVNC
    Viewer directory on the web server, e.g. `http://my_host/turbovnc`.

    __NOTE:__ Leave out the lines referring to `tvnchelper-*.jar` if you have
    not installed the TurboVNC Helper JARs.

    This is just a minimal example.  Refer to the [JNLP file syntax]
    (https://docs.oracle.com/javase/8/docs/technotes/guides/javaws/developersguide/syntax.html)
    for additional fields that you might want to add.

* You should now be able to access `{turbovnc_url}/TurboVNC.jnlp` in your
  browser to launch the TurboVNC Viewer with full performance.


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
