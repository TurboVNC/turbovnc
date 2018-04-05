2.2 pre-beta
============

### Significant changes relative to 2.1.2:

1. Added a system menu option and hotkey (CTRL-ALT-SHIFT-V) that can be used to
quickly toggle view-only mode on and off.

2. Added system menu options and hotkeys that can be used to zoom in and out
(increase or decrease the scaling factor) or reset the view to 100% when
desktop scaling is enabled.

3. The TurboVNC Server now includes a GLX extension that uses the swrast DRI
driver installed on the system to perform unaccelerated OpenGL rendering.
Using VirtualGL for OpenGL rendering will still be much faster and more
compatible, but this feature allows the TurboVNC Server to be used for casual
OpenGL rendering if VirtualGL is not installed.  Passing `-extension GLX` to
vncserver disables the extension.  Note that your system must have Mesa 8.x or
later installed in order for the swrast DRI driver to be available.  On systems
that do not have vendor-specific GPU drivers installed, or on systems that
provide a libglvnd-enabled build of Mesa, the TurboVNC Server's GLX extension
will use direct rendering, which improves performance and compatibility.

4. The Java/Mac/Un*x TurboVNC Viewer is no longer supported with Java 6 and
earlier.  This increases the Mac system requirements to OS X 10.7 "Lion" or
later.  Apple's obsolete Java for OS X is no longer supported, and the
standalone "AppleJava" version of the TurboVNC Viewer is no longer provided.
Java SE 6 ceased receiving public updates in 2013 and, as of this writing,
critical bug fixes and security fixes for that platform are only available with
an Oracle Extended Support contract.  Furthermore, Red Hat ceased supporting
OpenJDK 6 in late 2016.  The TurboVNC Viewer JAR will continue to be built with
`-source 1.6 -target 1.6`, but it will only be tested with Oracle Java SE
releases that are currently receiving public updates and OpenJDK releases that
are actively supported by Red Hat.  The 2.1.x version of the TurboVNC Viewer
will continue to support Java 6 on a break/fix basis.

5. The ability to run the Java TurboVNC Viewer as an in-browser applet has been
removed.  Most popular web browsers have already abandoned the NPAPI plugin
standard around which the Java plugin was designed, because that plugin
standard grants full access to the client machine.  However, such access is
necessary in order to write a full-featured application (including a VNC
viewer), and absent any other cross-browser plugin standard, it would be
impossible for Oracle to make the Java plugin more secure without abandoning
the write-once-run-anywhere nature of the Java language.  Thus, they have
chosen not to include any kind of browser plugin in Java 9.  As of this
writing, NPAPI support can only be obtained on most platforms by installing an
older browser version.  The only advantage of using the TurboVNC Viewer as an
applet, as opposed to with Java Web Start, was the ability to embed the viewer
in a web page, but this mode of operation had some known usability issues and
was increasingly receiving little or no attention from the TurboVNC community.
The applet feature will continue to be supported in TurboVNC 2.1.x on a
break/fix basis.

6. The TurboVNC Server now has full support for the Xinerama extension, which
allows multiple virtual screens to be configured within the same remote
desktop.  This provides better usability for multi-screen environments, since
dialogs and maximized application windows are no longer split across screen
boundaries.  The server's screen layout can be configured in the following
ways:

    -  Multi-screen geometry descriptors, in the format
`W0xH0+X0+Y0[,W1xH1+X1+Y1,...,WnxHn+Xn+Yn]]`, are now accepted in these places:
        * The TurboVNC Server's `-geometry` command-line argument and the
`$geometry` variable in turbovncserver.conf (thus allowing the user to specify
a screen layout for the initial framebuffer)
        * The "Remote desktop size" combo box in the TurboVNC Viewer's Options
dialog, the TurboVNC Viewer's `-desktopsize` command-line argument, and the
`DesktopSize` parameter in the Java TurboVNC Viewer (thus allowing the user to
remotely request a specific screen layout)
        * A new .vnc connection info file directive (`desktopsize`) that can
be used instead of the `resizemode`, `desktopwidth`, and `desktopheight`
directives

    - The automatic desktop resize feature in the TurboVNC Viewer is now
multi-screen aware in both windowed and full-screen modes.  When the viewer
window is resized or reset to its default position, the viewer will now compute
and request a new server-side screen layout that matches not only the viewer
window boundaries but also the client's screen boundaries and the position of
the taskbar/menu bar on the client.  The single-screen automatic desktop
resizing behavior of the TurboVNC 2.1.x viewer can be restored by setting the
`TVNC_SINGLESCREEN` environment variable or the `turbovnc.singlescreen` Java
system property to `1`.

    - The X RandR extension can be used, either from the command line
(`xrandr`) or through the window manager's display configuration applet, to
modify the server's screen layout.

7. The TurboVNC Server is now based on xorg-xserver 1.19.5, which improves
compatibility with newer window managers.

8. By default, the TurboVNC Server will no longer listen on a TCP socket for
X11 protocol connections from X11 applications.  This mimics the behavior of
most modern X servers, which require X11 connections to be made using Unix
domain sockets.  To restore the behavior of previous versions of TurboVNC, pass
`-listen tcp` to vncserver (assuming that X11 TCP connections are not globally
disabled in the TurboVNC Server's security configuration file.)

9. The default xstartup.turbovnc script that the TurboVNC Server creates will
no longer start a 2D window manager by default on Ubuntu systems, since the
TurboVNC Server now has a software OpenGL implementation that supports running
Unity 3D without VirtualGL.  The xstartup.turbovnc script will now always
attempt to start the window manager specified by the system or user defaults,
unless the `TVNC_WM` environment variable is set.  Setting the `TVNC_WM`
environment variable to `2d` will start the GNOME fallback/flashback or Unity
2D session on Ubuntu systems instead of Unity 3D.  Setting the `TVNC_WM`
environment variable to `2d` will also start the GNOME classic session on RHEL
7 and recent Fedora releases.  To start MATE, set the `TVNC_WM` environment
variable to `mate-session`.

10. The `-3dwm` option to the vncserver script has been renamed to `-vgl`, to
reflect the fact that the TurboVNC Server is now able to run 3D window managers
without using VirtualGL.

11. Clipboard synchronization is now performed within the TurboVNC Server
process, so the tvncconfig utility is no longer needed or provided.

12. The TurboVNC Server now uses a 30-bit default visual (BGRX 10/10/10/2 on
little endian machines and XRGB 2/10/10/10 on big endian machines) when
launched with `-depth 30`.  This is mainly useful for application testing at
the moment, since the pixels are downsampled to 8-bit RGB prior to
transmission.

13. Introduced a new CMake variable (`TVNC_SYSTEMLIBS`) that, when enabled,
will cause the TurboVNC Server to be built against the system-supplied versions
of zlib, bzip2, and FreeType.

14. Introduced a new CMake variable (`TVNC_SYSTEMX11`) that, when enabled,
will cause the TurboVNC Server to be built against the system-supplied versions
of the X11 and OpenGL headers and libraries.  This will probably fail unless
the system is using xorg-server 1.19.x or later.

15. Fixed a bug in the TurboVNC Server's VeNCrypt implementation that prevented
it from working properly with LibVNCClient.

16. The TurboVNC Server and Windows TurboVNC Viewer, when built with
`TVNC_SYSTEMLIBS=0` (which is the default), now incorporate the Intel zlib
library, which accelerates zlib encoding and decoding significantly on
SSE2-equipped CPUs.  This improves the end-to-end performance of the Lossless
Tight + Zlib encoding method, and of non-JPEG (low-color-depth) subrectangles
encoded with one of the Tight + JPEG encoding methods, by approximately 20-40%.

17. The Mac/Java TurboVNC Viewer now includes remote drawing tablet support.
This feature is implemented using a TurboVNC Helper JNI library, which is
similar to the TurboVNC Helper included with the Un*x TurboVNC Viewer.  This
library connects the built-in drawing tablet drivers for OS X with the existing
remote X Input feature in the Java TurboVNC Viewer.

18. Fixed an issue in the Un*x/X11 TurboVNC Viewer and the Windows/Java
TurboVNC Viewer whereby keyboard grabbing was always initially disabled in the
second and subsequent connections initiated by the viewer, regardless of the
grab mode.


2.1.2
=====

### Significant changes relative to 2.1.1:

1. Improved the usability of multithreaded Tight encoding in the TurboVNC
Server:

    - The `TVNC_MT` and `TVNC_NTHREADS` environment variables now have
corresponding Xvnc command-line options (`-mt` and `-nthreads`), which makes it
easy to enable multithreaded Tight encoding in TurboVNC Server instances
spawned by init.d/systemd.
    - The turbovncserver.conf file, which is parsed by the vncserver script,
now includes two new variables (`$multiThread` and `$numThreads`) that can be
used to configure multithreading on a system-wide basis or for all TurboVNC
sessions started under a particular user account.
    - Previously, if multithreaded Tight encoding was enabled, the Tight
encoder would use as many threads as there were CPU cores on the server, up to
a maximum of 8.  However, because of limitations in the Tight encoding type,
using more than 4 threads requires the rfbTightNoZlib extension, which is only
supported by the TurboVNC Viewer.  To avoid confusion, the TurboVNC Server will
no longer use more than 4 threads (regardless of the number of CPU cores)
unless the thread count is explicitly specified.

2. Fixed an issue in the console version of the Windows TurboVNC Viewer
(cvncviewer.exe) whereby the console output of the viewer could not be
redirected to a file.

3. The Java/Mac/Un\*x TurboVNC Viewer now includes an option for reversing the
direction of mouse scroll wheel events that are sent to the VNC server.  This
is useful when connecting from clients that have "natural scrolling" enabled.

4. Fixed an issue whereby, under certain rare circumstances (for instance, if
the Xvnc binary was setuid root), the TurboVNC Server would allow any user of
the system (not just the session owner) to authenticate with a TurboVNC Server
session using PAM User/Password Authentication, if the user ACL feature was
disabled.

5. Fixed a BadMatch X11 error that occurred when attempting to resize the
TurboVNC Server desktop to a smaller size using the X RandR 1.2 API (for
instance, by executing `xrandr --output TurboVNC --mode {new_mode}`.)

6. Fixed an issue whereby the TurboVNC Server could not be built using GnuTLS
3.4.0 or later.

7. Improved the TLS encryption feature in the following ways:

     - The anonymous Elliptic Curve Diffie-Hellman (ECDH) key exchange
algorithm is now supported when the TurboVNC Server is built using GnuTLS 3.0.0
and later.  (The Java/Mac/Un\*x TurboVNC Viewer already supports ECDH.)
     - The TurboVNC Server will now use the most recent version of the TLS
protocol that both the server and the client support.
     - The "VNC connection info" dialog in the Java/Mac/Un*x TurboVNC Viewer
will now display the TLS protocol version currently in use.
     - The TurboVNC Server can now be built with OpenSSL 1.1, and if it is
built with `TVNC_DLOPENSSL=1` (the default), it can use OpenSSL 1.1 at run time
regardless of whether it was built on a machine that has OpenSSL 1.1 installed.
     - When using OpenSSL, the TurboVNC Server will now log a more meaningful
error message if the TLS handshake fails.

8. Fixed an issue in the Mac (Java) TurboVNC Viewer whereby the initial
non-full-screen window was positioned incorrectly if it spanned multiple
screens.

9. Fixed an issue in the Windows (native) TurboVNC Viewer whereby multi-screen
spanning did not work at all if the secondary display was to the left of the
primary display.

10. Multi-screen spanning now works with the Linux/Un\*x TurboVNC Viewer in
full-screen mode, if the viewer is using the TurboVNC Helper library.  (Due to
limitations in X11, it is still not possible to use multi-screen spanning with
the Linux/Un\*x TurboVNC Viewer in windowed mode.)  Also, the Linux/Un\*x
TurboVNC Viewer now falls back to Java's built-in full-screen window feature,
thus allowing full-screen mode to work with single-screen (Primary) spanning
even if the TurboVNC Helper library is not available.

11. The TurboVNC Server will now clamp the desktop dimensions to 32767x32767
regardless of the `max-desktop-size` setting in the security configuration
file.  This prevents two issues:

    - The server would fail to send framebuffer updates and would continuously
log messages of the form "WARNING: Framebuffer update at 0,0 with dimensions
0x0 has been clipped to the screen boundaries" if the desktop width or height
was set, by way of the `-geometry` command-line argument or a remote desktop
resize request, to a value greater than 32767.
    - The server would segfault if a mode with a width or height greater than
32767 was configured and enabled using the X RandR extension.

    Although TurboVNC can handle framebuffer dimensions larger than this, the
underlying X.org screen structure uses a signed short to represent width and
height, and thus values larger than 32767 overflow the data type and can
sometimes be interpreted as negative numbers.

12. Closed a loophole that allowed users to use X RandR functions to make the
desktop larger than the dimensions allowed by the `max-desktop-size` setting in
the security configuration file.

13. When automatic desktop resizing and automatic spanning are both enabled,
the TurboVNC Viewer will now use single-screen spanning ("Primary monitor
only") when in windowed mode and multi-screen spanning ("All monitors") when in
full-screen mode.

14. Fixed an issue whereby the Java TurboVNC Viewer, when launched from the
`vncviewer-java.bat` script, the Start Menu shortcut, or Java Web Start on
Windows clients with a 32-bit JRE, would fail with "Error: missing 'server'
JVM".  On Windows, the 32-bit JRE only provides the client VM, and the 64-bit
JRE only provides the server VM, so specifying `-server` in the launch scripts
was unnecessary.

15. Worked around an issue whereby, when the TurboVNC Viewer was running on
macOS 10.12 "Sierra", pressing and holding certain keys would cause the viewer
to stop processing subsequent keystrokes if ApplePressAndHoldEnabled was set to
true in the macOS user defaults (which it is by default in recent macOS
releases.) It is still necessary to set ApplePressAndHoldEnabled to false,
either in the global domain or the com.turbovnc.vncviewer.VncViewer domain, in
order for key repeat to work with the TurboVNC Viewer.

16. The Java/Mac/Un*x TurboVNC Viewer now supports the `user` and
`noremotecursor` directives in .vnc connection info files.

17. Fixed an issue that prevented the keyboard grabbing feature from working
properly in the Java TurboVNC Viewer on 32-bit Windows systems.

18. Fixed an issue whereby, when server-side cursor rendering was enabled, the
cursor would flicker on and off as the mouse was dragged.

19. Fixed an issue whereby the Java/Mac/Un*x TurboVNC Viewer would, under
certain circumstances (specifically, when receiving a desktop size update from
the server with automatic desktop resizing enabled, toggling on/off the
toolbar, or changing the scaling settings), always behave as if the
`CurrentMonitorIsPrimary` parameter was disabled.

20. The default xstartup.turbovnc script that the TurboVNC Server creates now
allows the window manager startup script/executable to be specified using
an environment variable (`TVNC_WM`.)  This facilitates using a different window
manager for local and remote desktop sessions on the same machine, or
temporarily switching the window manager used by TurboVNC.

21. The default xstartup.turbovnc script that the TurboVNC Server creates now
sets the `XDG_SESSION_TYPE` environment variable to `x11`.  This is necessary
when running GNOME 3 on Fedora 25 and later.  Otherwise, GNOME 3 will assume it
is operating in a Wayland environment.

22. Worked around an issue whereby desktop resizing in the TurboVNC Server
would fail on Ubuntu 12.04 LTS (and likely on Ubuntu 12.10 and 13.04 as well,
although those releases are EOL as of this writing) due to Ubuntu's inclusion
of an experimental extension to the XFixes protocol that was never accepted
upstream in X.org.

23. Fixed a bug in the X.org code that prevented the TurboVNC Server from
working properly on little endian PowerPC systems.

24. Fixed a couple of issues in the TurboVNC Server that prevented cursors from
being rendered and transmitted properly if the server was running on a big
endian system.

25. Fixed an issue in the Windows TurboVNC Viewer whereby, if keyboard grabbing
was disabled, using Alt-Tab to select another window would sometimes leave the
Alt key in a pressed state on the server.  Fixed a similar issue in the Mac
TurboVNC Viewer whereby, when quitting the viewer using Command-Q or closing
the connection using Command-W, the Super/Windows key would be left in a
pressed state on the server.

26. Fixed an issue in the Java TurboVNC Viewer whereby the client clipboard
contents would not be transferred to the server if text was selected in the
server session and the middle mouse button was clicked in the viewer window
without first bringing the window to the foreground.

27. Improved the zero-install Java Web Start feature in the following ways:

    - The built-in HTTP server in the TurboVNC Server now sends the
"Content-Length" and "Last-Modified" HTTP headers.  These headers are used by
Java Web Start to determine whether the server's copy of the TurboVNC Viewer
JAR is newer than the client's copy, and the addition of the headers prevents
an issue whereby Java Web Start would never update the cached copy of the
TurboVNC Viewer JAR on the client.
    - The JNLP file generated by the TurboVNC Server now contains directives
that disable background updating of the TurboVNC Viewer JAR.  Under certain
circumstances, background updating was causing the TurboVNC Server to become
unresponsive, since it is running the VNC, X11, and HTTP servers in the same
thread.

28. Worked around an issue in OS X whereby the TurboVNC Viewer window would not
be brought to the foreground if a mouse button other than the primary button
was clicked in it.  Because OS X sends only mouse press events to
non-foreground windows, this issue was causing mouse buttons other than the
primary button to become stuck in the pressed state on the server if one of
those buttons was clicked in the viewer window while the window was in the
background.

29. Fixed an issue in the TurboVNC Server whereby it would hang and eventually
time out when attempting to negotiate VeNCrypt capabilities with the viewer.
This issue was known to affect only Solaris 11 servers, but it might have also
affected other platforms under rare circumstances.

30. Worked around an issue in Java that caused Control-Underscore (the keyboard
shortcut for the Undo command in Emacs) to be transmitted incorrectly when
using the Java TurboVNC Viewer.  Worked around another issue in Java that
caused AltGr symbols associated with a dead key (for instance, the '|' symbol
on Danish keyboards) to be transmitted incorrectly.


2.1.1
=====

### Significant changes relative to 2.1:

1. Fixed an XML error ("The processing instruction target matching
'[xX][mM][lL]' is not allowed" or "Could not parse launch file") that occurred
when attempting to launch the Java TurboVNC Viewer using the built-in
zero-install Java Web Start feature in the TurboVNC Server.  Apparently recent
releases of Java did not like the fact that the JNLP file generated by the
TurboVNC Server began with an XML comment.

2. Arguments to the vncserver script are now case-insensitive.

3. The TurboVNC Server init.d script was erroneously checking for the existence
of ~/.vnc/passwd, even if the permitted and selected security types did not
require a VNC password file.  The init.d script now invokes vncserver with a
new argument (`-quiet`) that causes vncserver to fail if a VNC password file is
required but does not exist.

4. The Java TurboVNC Viewer now supports the rfbTLS security descriptor used by
Vino.  This should allow the viewer to connect to encrypted Vino sessions using
either the TLSVnc or TLSNone security types.

5. The default behavior of the TurboVNC Server is to refuse new non-shared
connections if a viewer is already connected.  Previously, the server
accomplished this by hard-coding the `-dontdisconnect` argument to Xvnc inside
of the vncserver wrapper script.  This release of TurboVNC instead makes
`-dontdisconnect` the default in Xvnc and introduces a new Xvnc argument
(`-disconnect`) to reverse the behavior.  This new option allows TurboVNC to
mimic the behavior of other VNC implementations that disconnect existing
viewers when a new non-shared connection is established.

6. Added a new parameter to the Java TurboVNC Viewer (`LocalCursor`) that, when
enabled, causes cursor shape updates to be ignored and the local cursor to
always be displayed.  This mimics the behavior of the Windows TurboVNC Viewer
when "Local cursor shape" is set to "Normal arrow" and "Mouse cursor" is set to
"Don't show remote cursor".  This option is useful when connecting to broken
VNC server implementations that do not properly support server-side cursor
rendering or cursor shape updates.

7. Fixed an issue in the Windows TurboVNC Viewer whereby the mouse scroll wheel
would stop working if the viewer was moved to a monitor above or to the left of
the Windows "main"/"primary" display.

8. Fixed an issue whereby zero-height PutImage requests (issued by XCB) would
crash the TurboVNC X server.  This was known to affect certain Qt5 applications
running under Debian Stretch but may have also affected other applications and
platforms.


2.1
===

### Significant changes relative to 2.1 beta2:

1. Added a new parameter to the Java TurboVNC Viewer (`LocalUsernameLC`) that,
when enabled along with the `SendLocalUsername` parameter, will cause the local
user name to be sent to the server in lowercase.  This can be useful for
Windows clients, since Windows allows mixed-case user names but Un*x machines
generally don't.

2. Fixed a regression introduced in 2.0 beta1[1] whereby connecting to the
TurboVNC Server with cursor shape updates (client-side cursor rendering)
enabled and continuous updates disabled (or with a viewer, such as the TurboVNC
1.1 Viewer, that doesn't support continuous updates) would cause the server and
viewer to get into an infinite ping-pong loop whenever the cursor shape
changed.  This infinite loop caused TurboVNC to consume an inordinate amount
of CPU and network resources until the next non-cursor-related framebuffer
update was sent.

3. Fixed several serious visual artifacts with server-side cursor rendering
(regression introduced in 2.0 beta1[1].)  Server-side cursor rendering is not
generally the default in TurboVNC, but it is useful for collaboration purposes.

4. Fixed an issue whereby the TurboVNC Server would become unresponsive to
input if a viewer connected while the MATE screensaver was active.

5. The default xstartup.turbovnc script that the TurboVNC Server creates now
includes a workaround for a bug in GNOME 3 whereby the pointer disappears when
mousing over the top bar.

6. Fixed an issue in the Java TurboVNC Viewer whereby, when the remote desktop
size was changed in the Options dialog, the new desktop size request was not
sent to the server until the next framebuffer update was received.

7. Fixed an issue in the TurboVNC Server's implementation of the X RANDR
extension that was causing viewer-initiated remote desktop resizes to fail or
otherwise behave incorrectly with GNOME 3.

8. The TurboVNC Server now ignores remote desktop resize requests from viewers
that authenticated with view-only credentials.

9. Fixed a regression introduced by 2.1 beta1\[2\] (but, for reasons
unexplained, not exposed until 2.1 beta1[4]) whereby the TurboVNC Server, when
built without TLS encryption, would sometimes segfault when a viewer
disconnected.  Oddly, this was not reproducible except on older platforms, such
as RHEL 4.

10. Fixed a regression introduced in 2.1 beta1[2] whereby the TurboVNC Server,
when built without TLS encryption, would fail to launch via the vncserver
script and display the following error: `Unrecognized option: -x509cert`.

11. Fixed an issue with the TurboVNC Server's 3D window manager feature that
was known to affect Red Hat- and Ubuntu-compatible systems (and may have
affected others as well.)  When running vncserver with the `-3dwm` option, the
window manager would behave as if VirtualGL was not active (thus causing the WM
to crash, if it required OpenGL.)  The underlying cause of this was code
executed from within /etc/X11/xinit/xinitrc that launches the WM using
ssh-agent if the `SSH_AGENT_PID` (Red Hat) or `SSH_AUTH_SOCK` (Ubuntu)
environment variable is unset.  ssh-agent clobbers the `LD_PRELOAD` environment
variable, so in order to make it work properly with VirtualGL, vglrun has to be
launched by ssh-agent, not vice versa.

    The default xstartup.turbovnc script that the TurboVNC Server creates now
launches the window manager in VirtualGL and launches VirtualGL in ssh-agent if
3D window manager support is enabled and there is no existing ssh-agent
session.  Furthermore, the `-3dwm` option to vncserver now simply sets the
`TVNC_3DWM` environment variable to `1` rather than invoking VirtualGL
directly.  The default xstartup.turbovnc script launches the WM in VirtualGL if
that environment variable is set, using a second environment variable
(`TVNC_VGLRUN`, which can be overridden by the user) to specify the VirtualGL
command line.  However, users can craft their own xstartup.turbovnc scripts
that handle the `TVNC_3DWM` environment variable differently, if desired.

12. The default xstartup.turbovnc script that the TurboVNC Server creates will
now properly launch the GNOME Flashback (Metacity) window manager on Ubuntu
16.04, if 3D window manager support is not activated.  This eliminates the need
to install MATE on that platform.

13. The default xtartup.turbovnc script that the TurboVNC Server creates now
contains a fix for an issue whereby Unity 7.2 on Ubuntu 14 would automatically
disable the compiz OpenGL plugin if the window manager was launched in a
broken OpenGL environment (such as if the user forgot to specify `-3dwm`.)
Symptomatically, this issue caused the menus and launcher to disappear, leaving
only a blank desktop.  When launching Unity, the xstartup.turbovnc script now
attempts to create an environment similar to the one created by GDM or LightDM.

    This fix also allows Unity 7.4 under Ubuntu 16.04 to run in TurboVNC, if
3D window manager support is enabled.

14. The "OracleJava" TurboVNC package is now the default Mac package, since
Oracle Java performs much better than Apple's (deprecated) Java distribution on
OS X 10.10 "Yosemite" and later.  Hence, the "OracleJava" suffix has been
removed from that package.  The "AppleJava" TurboVNC package is still provided,
but it should only be used on OS X 10.9 and earlier.

15. Fixed a regression introduced in 2.1 beta1[2] whereby Xvnc would segfault
if the `-rfbauth` argument was not specified and a client attempted to
authenticate using a VNC password.

16. Dead keys should now fully work when using Java 8 or later on Windows and
OS X.  They still do not work properly when using Java 6 or Linux, due to bugs
in Java.

17. Fixed two issues in the TurboVNC Viewer related to keyboard handling:

     - An issue whereby, when pressing Shift or AltGr, then pressing a
"printable" (alphanumeric or symbol) key, then releasing the keys in the same
order, the viewer would compute and send a different key symbol for the release
event than it did for the press event (because the key was no longer modified.)
This would confuse the XKEYBOARD handler in the server, which would ignore the
release event, thus causing the printable key to appear pressed from the point
of view of applications running in the TurboVNC Server session.
     - An issue whereby, when a key was pressed, the viewer window lost focus,
then the key was released in another window, the key would similarly continue
to appear pressed from the point of view of applications running in the
TurboVNC Server session.

18. Fixed a regression in the standalone Linux TurboVNC Viewer caused by
2.1 beta1[4] (remote X Input support) whereby some X Input devices other than
extended pointer devices (such as keyboards, regular mice, and webcams) were
mistakenly being cloned onto the TurboVNC Server, thus causing erratic pointer
behavior and other issues.  Some devices (even some keyboards) seem to report
that they are extended pointer devices when in fact they aren't.  The TurboVNC
Helper is now more discriminating and does not allow any X Input device whose
type Atom is "MOUSE" or "KEYBOARD", or any device with relative valuators, to
be cloned onto the server.  The server now also rejects any attempt by the
viewer to create an extended input device with relative valuators, since those
don't currently work.  Wacom tablets and other extended pointer devices with
absolute valuators should still work fine.

19. Worked around an issue in Java that was causing scrollbars to be
unnecessarily displayed in the Linux/Java TurboVNC Viewer when it was used
under GNOME 3 with automatic desktop resizing enabled.

20. The Windows TurboVNC Viewer will now pass keystrokes from Microsoft
extended keys to the VNC server when the keyboard is grabbed.

21. Fixed a regression introduced in 2.1 beta1[9] whereby the Linux TurboVNC
Viewer and the Windows Java TurboVNC Viewer would throw a NullPointerException
if the options were changed in the Options dialog prior to connecting to a VNC
server.  This issue occurred only in the standalone viewers, not when using
Java Web Start.


2.0.91 (2.1 beta2)
==================

### Significant changes relative to 2.1 beta1:

1. Fixed an issue in the Java TurboVNC Viewer whereby, when built against
libjpeg-turbo 1.5 or later, it would generate the following error:
`Class not found: org/libjpegturbo/turbojpeg/TJException` at run time and
subsequently fail to accelerate JPEG decompression.  TJException is a new class
in libjpeg-turbo 1.5, and due to an oversight, VncViewer.jar was not including
it.


2.0.90 (2.1 beta1)
==================

### Significant changes relative to 2.0.2:

1. The TurboVNC Server can now emulate a subset of the NV-CONTROL X11
extension, in order to support certain 3D applications that rely on this
extension to query and set low-level nVidia GPU attributes.  See the User's
Guide for more details.

2. The TurboVNC Server now provides full support for the VeNCrypt RFB
extensions, including TLS-encrypted security types.  TLS encryption is provided
by OpenSSL in the official TurboVNC binaries, but GnuTLS can be used instead
when building the TurboVNC Server from source.  Note that when using the Java
TurboVNC Viewer, you must use 2.0.1 or later when connecting to a TurboVNC 2.1
server.  Older versions of the Java TurboVNC Viewer had a bug that caused them
to lock up when connecting to a server that supports both the Tight and
VeNCrypt security extensions.

    Backward compatibility note:  because of the addition of this feature, it
was necessary to remove the `-noauth`, `-novncauth`, `-nootp`, and `-nopam`
parameters to vncserver, because these parameters could not be emulated
exactly using the new VeNCrypt security type selection mechanism.  Please use
the new `-securitytypes` parameter instead (see the Xvnc man page for more
details.)

3. TurboVNC's vncserver script now supports the `-autokill` option from
TigerVNC, which causes the server to be killed automatically whenever the
startup script finishes (which will usually happen as the result of logging out
of the window manager running in the TurboVNC session.)  It was possible to
accomplish the same thing in earlier versions of TurboVNC by running
`/opt/TurboVNC/bin/vncserver -fg </dev/null &`, but this is more intuitive.

4. The TurboVNC Server and the Java TurboVNC Viewer (when the latter is run in
standalone mode on Un*x/Linux machines, using the TurboVNC Helper library) now
support a remote X Input interface whereby extended pointer devices (such as
drawing tablets) on the client are cloned in the TurboVNC server session, and
the events from these devices (including pressure, tilt, etc.) are passed from
viewer to server.  This was specifically designed for Wacom tablets but should
work with other extended pointer devices as well.

    Additionally, the Windows TurboVNC Viewer provides specific support for
Wacom drawing tablets by interfacing between the Wacom drivers for Windows and
the afore-mentioned remote X Input interface.

5. Keyboard grabbing has been implemented in the Java TurboVNC Viewer for
Windows, thus allowing special keystrokes (such as Alt-Tab) to be sent to the
server.

6. Added a new option to Xvnc (`-pamsession`) that will open a PAM session for
every viewer that authenticates using the username/password of the user who
owns the TurboVNC session and will leave that PAM session open until the viewer
disconnects.  This feature was specifically implemented in order to allow
Kerberos tickets created by PAM to be reused within the TurboVNC session, but
the feature may benefit other use cases as well.

7. Added a new parameter to the Java TurboVNC Viewer (`NoReconnect`) that can
optionally be used to revert the behavior introduced in 2.0 beta1[17].

8. The controls in the Options dialog of the Windows (native) TurboVNC Viewer
have been re-organized into two tabs ("Encoding" and "Connection"), which makes
the layout of that dialog more similar to that of the Java viewer's Options
dialog.

9. The keyboard/pointer grabbing feature can now be configured from the Options
Dialog in both the Windows and Java TurboVNC Viewers.

10. Added a `-poll` option to tvncconfig, which works identically to the
`-poll` option in RealVNC's (and TigerVNC's) vncconfig program.  This causes
tvncconfig to poll for changes to the clipboard at a specified interval rather
than waiting for the X server to inform it of the changes.

11. The Windows TurboVNC Viewer package now includes a console version of the
viewer (cvncviewer.exe), which is useful when debugging or when using the
`-via` and `-tunnel` options.  This version of the viewer is built by default
when building TurboVNC on Windows, but it can be disabled by setting the
`TVNC_WINCONSOLE` CMake variable to `0`.


2.0.2
=====

### Significant changes relative to 2.0.1:

1. The default xstartup.turbovnc script that the TurboVNC Server creates now
includes `unset DBUS_SESSION_BUS_ADDRESS`.  This fixes an issue whereby
starting the TurboVNC Server from within an existing GNOME 2 (verified with
GNOME 2.28.x and later) or MATE session would cause the GNOME/MATE session
running in TurboVNC to fail with an error dialog: "Could not acquire name on
session bus".

2. Fixed an issue whereby the TurboVNC Server would not properly handle Caps
Lock when using the TurboVNC Viewer on OS X.  X11 generates both a key press
and a key release event when Caps Lock is toggled, but OS X generates a key
press when Caps Lock is toggled on and a key release when it is toggled off.
This was causing the Caps Lock state to become decoupled between client and
server.  The solution was to do what TigerVNC does and ignore Caps Lock (and
other lock modifiers) on the server side, thus allowing the client to handle
Caps Lock.  If for some reason this causes problems, then the old behavior can
be recovered by setting the `TVNC_XKBIGNORELOCK` environment variable to `0`
prior to launching the TurboVNC Server.

3. Worked around an issue whereby the Java TurboVNC Viewer on Windows, when
running under Java 7 and later, would take an inordinate amount of time to
create the viewer window after connecting to a VNC server.  This slowdown was
due to a slow Win32 system call-- `DescribePixelFormat()`-- which is used in
the implementation of `GraphicsDevice.getConfigurations()` in Windows JVMs.
This system call also forces the DirectDraw DLL to be loaded, which is
unnecessary (TurboVNC explicitly disables DirectDraw blitting for performance
reasons.)

    The Java TurboVNC Viewer now automatically sets the `sun.awt.nopixfmt`
system property to `true`, which should significantly speed up the creation of
the viewer window, particularly on multi-screen clients, by avoiding the slow
`DescribePixelFormat()` system call.  This causes Java 2D to always use the
default pixel format for the display, but since the TurboVNC Viewer will always
try to use GDI blitting (as opposed to OpenGL or DirectDraw blitting), the
default pixel format should always work.  If for some reason it doesn't,
however, then the old behavior of the Java TurboVNC Viewer can be restored by
passing `-Dsun.awt.nopixfmt=0` to java or adding `-Dsun.awt.nopixfmt=0` to the
`JAVA_TOOL_OPTIONS` environment variable.

4. "Dead" keys, which are used to add accents and other diacritics to
subsequent alphabetic keystrokes, should now work properly when using the
Windows TurboVNC Viewer.  Additionally, certain AltGr-modified symbols (such as
the Euro sign on European keyboards) did not previously work with the Windows
TurboVNC Viewer.  That has been fixed.

    Furthermore, dead keys should now mostly work when using the Java TurboVNC
Viewer on Windows under Java 8 and later.  Due to irregularities in how Java
translates dead keys into events on other platforms (for instance, on OS X,
Java uses a different key code for the press and release events of a particular
dead key, and on Linux, Java only communicates release events for dead keys),
dead key support on non-Windows platforms is still incomplete.

5. Fixed an issue in the Java TurboVNC Viewer whereby the Options, Clipboard,
and Profiling dialogs were not centered relative to the viewer window.

6. Fixed an issue in the TurboVNC Server (Xvnc) whereby, when running a
compositing window manager such as Gnome 3, activity in off-screen windows
(such as a scrolling xterm being displayed in a separate workspace) could
interfere with the display of on-screen windows that occupied the same screen
area as the off-screen windows.

7. The Windows TurboVNC Viewer will now resolve localhost to the IPv4 loopback
address (127.0.0.1) by default unless the `/ipv6` option is specified.  This
makes its behavior more consistent with the Java viewer (NOTE: connecting to
localhost on a Windows machine is an unusual case, but it can occur when using
virtualization software that has a built-in VNC server.)

8. Improved the parsing of display names in both TurboVNC viewers.  In
particular, the Windows viewer now properly parses the following display name
forms:

     -  `:n`  (same as `localhost:n`)
     -  `::port`  (same as `localhost::port`)
     -  `x:x:x::x:n`  (abbreviated IPv6 address)
     -  `x:x:x::x::port`  (abbreviated IPv6 address)
     -  `x:x:x:x:x:x:x:x:n`  (full IPv6 address)
     -  `[IPv6]:n`  (IPv6 address in brackets)
     -  `[IPv6]::port`  (IPv6 address in brackets)

    Both viewers now properly parse the following display name forms:

     -  `x:x:x::x`  (abbreviated IPv6 address, same as `x:x:x::x:0`)
     -  `[IPv6]`  (IPv6 address in brackets, same as `[IPv6]:0`)

9. The Windows TurboVNC Viewer should now build with Visual C++ 2015.

10. Fixed an issue in the Java TurboVNC Viewer whereby the viewer, when using
the "server" (manual) remote desktop resizing mode, would unnecessarily display
two scrollbars if one of the remote desktop dimensions was too large for the
local display but the other dimension wasn't.


2.0.1
=====

### Significant changes relative to 2.0:

1. When using an external SSH client with the `Via` or `Tunnel` parameters, the
Java TurboVNC Viewer was not passing the SSH user name to the external SSH
client, which caused SSH authentication to fail unless the remote and local
user names were the same.  This has been fixed.

2. Fixed an issue whereby the TurboVNC X server was erroneously generating X11
MotionNotify events every time a mouse button was pressed or released,
regardless of whether the pointer had actually moved.  This caused problems
with certain applications (for instance, in GNOME Terminal, text was selected
when single-clicking on it rather than double-clicking or click-dragging on
it.)

3. The `/desktopsize server` option now works properly with the Windows
TurboVNC Viewer.

4. Fixed an issue that prevented VeNCrypt authentication from working in the
Java TurboVNC Viewer when connecting to a server that supports both the
TightVNC and VeNCrypt security extensions.

5. Improved the GUI for specifying the X.509 CA certificate and CRL in the Java
viewer, and added new command-line/applet parameters (`X509CA` and `X509CRL`)
that can be used to specify these files without using the GUI.

6. The Java viewer will now report an error if one of the security types
specified in the `SecurityTypes` parameter is invalid.  This prevents
hard-to-diagnose issues (such as authentication errors or the wrong security
type being selected) if one of the security types is misspelled.

7. The TightVNC/TurboVNC-specific "Unix Login" authentication scheme can now be
enabled/disabled by using the Java TurboVNC Viewer's `SecurityTypes` parameter,
similarly to other authentication schemes and VeNCrypt security types.
Furthermore, the `NoUnixLogin` parameter now disables the VeNCrypt *Plain
security types as well as the Unix Login security type, and the `User` and
`SendLocalUserName` parameters now disable any of the VeNCrypt security types
that don't require a user name.

8. The Java TurboVNC Viewer now supports (and prefers) elliptic curve ciphers
when using the Anonymous TLS security types.  Additionally, the Java TurboVNC
Viewer now uses TLS v1.1+ by default instead of TLS v1.0.

9. Improvements to the handling of X.509 certificates in the Java TurboVNC
Viewer:

     - Fixed a NullPointerException that would occur when attempting to use one
of the X.509 security types without specifying an X.509 certificate authority
(CA) certificate.
     - If the server's certificate is signed by an unknown CA, then the viewer
will pop up a dialog that gives the user the option to trust the certificate
for future connections.
     - The viewer can now handle X.509 certificate files containing multiple
concatenated certificates.
     - The viewer now performs hostname verification using either the X.509 v3
subjectAltName (SAN) extension or, if SAN is not present, the subject CN field.

10. When entering full-screen mode or using the "Default Window Size/Position"
feature in the TurboVNC Viewer (both Java and Windows), the viewer will now use
the monitor containing the largest number of pixels from the viewer window as
the "primary" monitor, for the purposes of spanning.  Thus, if the viewer
window is mostly contained on a monitor other than the left/top one, entering
full-screen mode or setting the window to the default size/position will no
longer cause the window to jump to the left/top monitor.

    OS X Yosemite introduced an optional feature (enabled by default) called
"Spaces", whereby each monitor occupies its own independent space when in
full-screen mode (thus allowing full-screen mode to be enabled independently on
each monitor.)  Previously, when this feature was enabled, moving the TurboVNC
Viewer window to a monitor other than the left/top one and entering full-screen
mode would result in a black screen.  This bug fix allows Primary spanning to
work properly with Spaces, but note that no other spanning mode will work
properly unless the Spaces feature is disabled.

    Both viewers contain a new command-line option/parameter
(`-firstmonitorisprimary` for the Windows viewer, `CurrentMonitorIsPrimary=0`
for the Java viewer) that allows the previous behavior (always treating the
left/top monitor as the primary) to be restored.

11. The Java TurboVNC Viewer now supports the BGR555 pixel format, which is
reported to be necessary when connecting to some embedded devices.

12. The Java TurboVNC Viewer now encodes the clipboard as ISO-8859-1 when
sending it to the server, instead of UTF-8.  The RFB spec requires that the
clipboard be encoded as ISO-8859-1, so it is possible that attempting to encode
it as UTF-8 was causing problems with certain applications.

13. The TurboVNC Server now allows the maximum clipboard transfer size to be
specified, by way of a new Xvnc argument (`-maxclipboard`).  Previously the
limit was hard-coded to 1 MB.  Furthermore, the TurboVNC Server now truncates
clipboard updates larger than the max. clipboard size instead of ignoring them.

14. The Java TurboVNC Viewer now allows the maximum clipboard transfer size to
be specified, by way of a new parameter (`MaxClipboard`).  The Java TurboVNC
Viewer now also truncates any clipboard transfers greater than MaxClipboard
(which is set to 1 MB by default.)  This prevents the viewer from sending
clipboard data to the server that will ultimately be discarded.


2.0
===

### Significant changes relative to 2.0 beta1:

1. A new security configuration file directive (`no-httpd`) has been introduced
in order to disable the built-in HTTP server in the TurboVNC Server on a
system-wide basis.

2. A new security configuration file directive (`no-x11-tcp-connections`) has
been introduced in order to disable X11 TCP connections to the TurboVNC Server
on a system-wide basis.  This is the equivalent of passing `-nolisten tcp` to
every instance of the TurboVNC X server running on a particular server machine.

3. The Java TurboVNC Viewer now supports the `grabkeyboard`, `resizemode`,
`desktopwidth`, and `desktopheight` directives in .vnc connection info files.
These directives were added to the Windows TurboVNC Viewer in 2.0 beta but were
left out of the Java viewer due to an oversight.

4. The default xstartup.turbovnc script that the TurboVNC Server creates will
now launch the MATE desktop on Ubuntu 15, if it is installed and if 3D window
manager support is not activated.  The GNOME Flashback session under Ubuntu 15
cannot be made to work properly with TurboVNC, for unknown reasons, but MATE is
a better solution anyhow.

5. The drawing performance of the Java TurboVNC Viewer when running under
Oracle Java 7 and later on OS X has been dramatically improved (by 3-7x for 2D
application workloads and 35-50% for 3D application workloads.)  When running
the standalone TurboVNC Viewer, Apple Java will probably still perform better
on Macs running OS X 10.9 "Mavericks" and earlier, but on Macs running OS X
10.10 "Yosemite" and later, Oracle Java is now the fastest solution (Java 2D is
apparently not accelerated in Apple Java 6 on these more recent OS X
releases.)  See the User's Guide for more details.

6. Fixed an issue whereby pressing any of the extra buttons on mice with more
than 3 buttons (Microsoft calls these "X buttons") would cause the Java
TurboVNC Viewer to send a left button press event to the TurboVNC Server
without sending a corresponding button release event.  The Java TurboVNC Viewer
now ignores any X button events, since X11 doesn't support them.


1.2.90 (2.0 beta1)
==================

### Significant changes relative to 1.2.3:

1. The TurboVNC X server has been completely overhauled and is now based on the
unmodified Xorg 7.7 code base.  This overhaul enables support for the X
extensions needed by newer window managers (RANDR 1.2, Composite, XFIXES,
Damage, etc.)  In addition, the keyboard handler has been completely overhauled
and now uses the XKEYBOARD extension.  This fixes key mapping issues that
occurred when running TurboVNC with certain versions of GNOME (previously, it
was necessary to set an environment variable in xstartup.turbovnc to work
around those issues.)

2. Added the ability to dynamically resize the remote desktop, either through
the X RANDR extension on the server or remotely from a VNC client that supports
the RFB desktop size extensions.  Both TurboVNC viewers now also include an
"automatic desktop resize" feature that will resize the remote desktop so that
it always fits exactly in the viewer window without using scrollbars.  A new
`max-desktop-size` directive is provided in the TurboVNC security configuration
file in order to allow a maximum desktop size to be specified for all TurboVNC
sessions on a given server machine.

3. The X11 TurboVNC Viewer has been retired and replaced with the Java TurboVNC
Viewer.  The X11 viewer will continue to be maintained in the 1.2.x branch on a
break/fix basis only.  The Java viewer provides similar performance to the X11
viewer when remotely displaying 3D application workloads to a reasonably modern
client machine.

4. The Java TurboVNC Viewer, when run as an applet, can now be displayed to an
embedded frame in a web page rather than a dedicated window.  This restores a
feature of the TurboVNC 1.1 Java viewer that was lost in 1.2.  Full-screen mode
and scaling do not work when the viewer is run as an embedded applet.

5. vncconnect now uses the VNC X extension to establish a reverse connection,
rather than setting a root window property on the VNC X server.  This makes
TurboVNC's implementation of vncconnect compatible with RealVNC and TigerVNC
servers, and the return status of vncconnect now reflects whether the reverse
connection was successfully made.  vncconnect also now has a `-disconnect`
option, which can be used to disconnect all listening viewers.

6. Since Apple is discontinuing their distribution of Java in favor of Oracle's
(which doesn't work on Snow Leopard and earlier), we now provide two Mac
packages-- one that works with Leopard and later and uses Apple's version of
Java, and one that works with Lion and later and uses the Oracle Java plug-in.

7. When running in listen mode, the Java TurboVNC Viewer now displays a tray
icon with a popup menu similar to that of the Windows native viewer.  This
allows the listener to be shut down, for global options to be set for all
connections, and for new forward connections to be made without using the
command line or launching another viewer instance.

8. The default xstartup.turbovnc script that the TurboVNC Server creates will
now properly launch the GNOME fallback window manager on Ubuntu 14.04, if 3D
window manager support is not activated.

9. Interframe comparison and Compression Level 2 can now be selected in the
TurboVNC Viewer GUI.  Also, a new command-line option/parameter
(`CompatibleGUI`) can now be used to force the GUI to expose all 10 compression
levels (useful when connecting to non-TurboVNC servers.)

10. The Interframe Comparison Engine (ICE) now compares large framebuffer
update rectangles on a block-by-block basis, which prevents the entire
rectangle from being sent if only a small portion of it has changed.  The
default block size is 256x256 but can be changed using the `TVNC_ICEBLOCKSIZE`
environment variable (for instance, `TVNC_ICEBLOCKSIZE=128` would use 128x128
blocks.)

11. By default, the embedded HTTP server in the TurboVNC Server will now serve
up a JNLP (Java Web Start) file for the session instead of an applet.  You can
add /applet to the URL to instruct the HTTP server to serve up an applet
instead.  The official TurboVNC packages for Linux also include the native JAR
files necessary to deliver the libjpeg-turbo JNI library to Windows, Linux, and
OS X clients (when using Java Web Start.)

12. vncconnect can now be used to connect a TurboVNC Server session to an
instance of the UltraVNC Repeater in Mode II.

13. The `Via` and `Tunnel` parameters in the Java TurboVNC Viewer (which allow
specifying an SSH or UltraVNC Repeater gateway through which the VNC connection
should be tunneled) can now be configured using the Options dialog.

14. On Un*x/X11 platforms, a small JNI library (turbovnchelper) is now deployed
alongside the Java TurboVNC Viewer in order to work around full-screen mode
deficiencies in Java (specifically, under certain window managers, the taskbars
would appear on top of the full-screen window.)  Because the turbovnchelper
library depends on libjawt.so, it is unfortunately not easy to deploy it using
Java Web Start, so it is currently only used when the viewer is launched as a
standalone application (using the vncviewer script.)

15. The `NoNewConn` parameter in the Java TurboVNC Viewer will now disable the
"Close Connection" option in the F8 menu and the "Disconnect" button in the
toolbar as well (useful for web portals, particularly when using the new
embedded applet mode.)

16. On Un*x/X11 platforms, the Java TurboVNC Viewer can now grab the keyboard
when run as a standalone application.  This allows special key sequences, such
as Alt-Tab, to be sent to the server.  The pointer can also be optionally
grabbed, which allows special keyboard + pointer sequences (such as
Alt-{drag}), to be sent to the server as well.  This feature requires the
afore-mentioned TurboVNC Helper library.

17. The Java TurboVNC Viewer will now offer an option to reconnect if the
connection fails for any reason.

18. The Windows native TurboVNC Viewer no longer exposes the Double Buffering
option in its Options dialog.  The option was removed mainly to make room for
the Desktop Size combo box, but also, single buffering is rarely used and is
mostly a legacy feature.  Double buffering can still be disabled via the
`/singlebuffer` command-line switch or by specifying `doublebuffer=0` in a VNC
connection file.

19. /etc/turbovncserver-auth.conf (the "authentication configuration file") has
been renamed to /etc/turbovncserver-security.conf (the "security configuration
file") to reflect the fact that it allows configuration of more than just
authentication methods.


1.2.3
=====

### Significant changes relative to 1.2.2:

1. Fixed an issue whereby the Java TurboVNC Viewer would not enable the OS X
Lion full-screen mode feature on OS X 10.10 "Yosemite".

2. Fixed a regression introduced in 1.2.2 that caused a non-fatal
NullPointerException in the Java TurboVNC Viewer whenever the scaling factor
was changed in the options dialog.

3. Fixed an issue in the Java TurboVNC Viewer whereby the toolbar would
sometimes appear all black or be overwritten by the initial framebuffer update
when running under Java 7 and later.

4. Fixed an issue whereby the Java TurboVNC Viewer would unnecessarily add
scrollbars to the viewer window if the remote desktop was exactly large enough
to fit in the window without using scrollbars.

5. Fixed an issue whereby the `/grabkeyboard`, `/scale`, and `/span`
command-line parameters to the Windows TurboVNC Viewer could not be specified
in uppercase.

6. The Windows TurboVNC Viewer now supports scaling factors > 200.

7. Java 7 and later on Mac platforms no longer include a hardware-accelerated
2D blitter for Java 2D, opting instead to use only OpenGL for image drawing.
When using the default pixel format (BGRX), this is incredibly slow on some
Macs, because the OpenGL blitter is having to set all of the alpha values to
255 (opaque.)  Thus, the Java TurboVNC Viewer will now automatically use an
alpha-enabled pixel format (BGRA) for its back buffer if it detects that it is
running on a Mac with Java 7 or later (which will generally be the case when
the viewer is deployed as an applet or using Java Web Start.)  This improves
drawing performance by as much as 4-5x on certain Mac models, although the
drawing performance with Apple Java 6 is still going to be faster than with
Java 7 or later.  The Java TurboVNC Viewer will also now automatically use an
alpha-enabled pixel format on non-Mac platforms if it detects that OpenGL Java
2D blitting is being selected (normally accomplished by setting the
`sun.java2d.opengl` system property to `true`.)

    The `turbovnc.forcealpha` Java system property can be used to override the
default behavior described above (refer to User's Guide.)

8. Normally, Java 2D uses Direct3D by default for blitting (image drawing) on
Windows platforms, but GDI blitting is almost always faster (sometimes much
faster.)  Thus, the Java TurboVNC Viewer now attempts to disable Direct3D
blitting on Windows unless it is specifically requested (which can be
accomplished by setting the `sun.java2d.d3d` system property to `true`.)
Because Direct3D blitting can't be disabled programmatically under Java 6 and
earlier, the vncviewer-java.bat script and the Windows start menu shortcut now
pass `-Dsun.java2d.d3d=false` to java to ensure that Direct3D blitting is
always disabled when the Java TurboVNC Viewer is run as a standalone
application.

9. Improved the Tight decoding performance in the Windows TurboVNC Viewer by
5-15%.

10. Since the Java TurboVNC Viewer already has its own double buffering
mechanism, it now disables double buffering in Java 2D.  The primary advantage
of this is that MIT-SHM pixmap support is no longer required on Un*x platforms
in order to achieve optimal performance.  This also makes the viewer faster on
some systems.  On Windows, the Java viewer is now as fast as or faster than the
native viewer as a result of this change and [8].  You can set the
`turbovnc.swingdb` system property to `true` to restore the old behavior.

11. Fixed multiple issues in the Java TurboVNC Viewer related to full-screen
mode and desktop resizing and the interaction thereof.

      - When in full-screen mode, the viewer would sometimes abort with
"Destination buffer is not large enough" if the remote desktop size was
increased.
      - When in full-screen mode, the viewer would sometimes exit full-screen
mode without re-entering it if the remote desktop size was changed.  This was
particularly known to be a problem when using full-screen mode on OS X 10.7 or
later.
      - Fixed a couple of other timing-related issues (race conditions) that
could sometimes occur when resizing the window or entering/exiting full-screen
mode.  These issues would cause non-fatal exceptions or other unexpected
behavior.
      - When resizing the window to the default size, the viewer would
sometimes leave a border around the remote desktop if the remote desktop size
was less than the local desktop size.
      - The viewer window will now be automatically resized to the default
size/position whenever the span mode is changed.

12. The Java TurboVNC Viewer will no longer attempt to send a desktop resize
message to servers that don't support it.  This fixes a "Broken pipe" error
that occurred when the `-desktopsize` parameter was used when connecting to the
TurboVNC 1.2.x Server (or earlier versions.)

13. Fixed multiple issues in the Windows TurboVNC Viewer related to full-screen
mode, desktop resizing, spanning, scaling, and the interaction thereof.

     - When in full-screen mode, resizing the remote desktop sometimes produced
unexpected results, such as residual images of the old remote desktop appearing
around the borders of the new desktop.
     - The calculations for automatic spanning now take into account whether
scaling is enabled.  Previously, the decision as to whether to span the window
across one screen or multiple screens was based on the unscaled remote desktop
size.
     - When scaling is enabled, the default window position/size is now
computed correctly.  Previously, it was computed based on the unscaled remote
desktop size.
     - The viewer window will now be automatically resized to the default
size/position whenever the span mode or scaling factor is changed, or whenever
the remote desktop size changes.

14. On Un*x/X11 platforms, the Java TurboVNC Viewer will now send and set the
PRIMARY clipboard selection by default (thus enabling middle mouse button
copy/paste between the server and the client.)  This can be disabled by setting
the `turbovnc.primary` system property to `false`.

15. The Java TurboVNC Viewer can now optionally use an external SSH client when
the `ExtSSH` parameter is set to `1`.  The full SSH command line can also be
specified using the `VNC_VIA_CMD` and `VNC_TUNNEL_CMD` environment variables,
as with the native viewers.  See the TurboVNC User's Guide for more details.

16. All JAR files in the official TurboVNC packages for Linux are signed using
an official code signing certificate, to comply with applet/JWS security
requirements in Java 7u51 and later.

17. The vncviewer-java.bat and vncviewer-javaw.bat scripts, which are used to
launch the Java TurboVNC Viewer on Windows systems, did not work unless the
current directory was the TurboVNC install directory.  This has been fixed.

18. Sending Alt+{letter} key sequences to the server (for instance, to pop up
one of the menus in an application) did not work properly when using the Java
TurboVNC Viewer.  This has been fixed.


1.2.2
=====

### Significant changes relative to 1.2.1:

1. The Xvnc build system has been completely refactored and now uses CMake,
just like the rest of TurboVNC.  This makes the build somewhat more intuitive
(for instance, `make xserver` is no longer necessary when building Xvnc), and
it fixes various problems with cross-compiling.

2. Xvnc can now be built and run successfully on OS X and PowerPC platforms.

3. The default DPI of the X server has been changed to 96, to match that of
recent X.org releases and other VNC implementations.  This may make fonts
appear larger than in previous TurboVNC releases.  Passing an argument of
`-dpi 75` to vncserver restores the old behavior.

4. When the server is run with a depth of 8, PseudoColor visuals now work
properly (previously, the correct colormap was not always used.)  The server
now uses a PseudoColor visual as the default for depth=8, which matches the
behavior of other VNC solutions.

5. Added a security option to the Java TurboVNC Viewer that, when enabled, will
prevent the user from opening any new VNC connections from within the same
viewer instance.  In listen mode, this will further cause the viewer to exit
after the first connection is closed.

6. Added a profiling feature to the Java TurboVNC Viewer.

7. The libjpeg-turbo JNI libraries are now deployed with the Java TurboVNC
Viewer on Un*x and Windows, so it is no longer necessary to install the
libjpeg-turbo SDK on those platforms in order to get accelerated JPEG
decompression.

8. Fixed a couple of cosmetic issues with the automatic lossless refresh
feature:

     - If the source of a CopyRect operation was affected by lossy compression
(meaning that the destination of the operation will become lossy as well), then
the destination region is now ALR-eligible.  This can be overridden by
setting the environment variable `TVNC_ALRCOPYRECT` to `0`.
     - Previously, if `TVNC_ALRALL=0`, a PutImage operation would trigger an
ALR on all lossy regions of the display, even those that weren't drawn using a
PutImage operation.  This has been fixed.

9. The Mac packaging system now uses pkgbuild and productbuild rather than
PackageMaker (which is obsolete and no longer supported.)  This means that OS X
10.6 "Snow Leopard" or later must be used when packaging TurboVNC, although the
packages produced can be installed on OS X 10.5 "Leopard" or later.  OS X 10.4
"Tiger" is no longer supported.

10. Fixed various usability issues related to the display of dialogs in the
Java TurboVNC Viewer:

     - The ability to control the Options dialog with the keyboard was lost
after the first time the dialog was popped up and dismissed.
     - When running as an applet, clicking away from some of the dialogs would,
in some cases, send the dialog to the back of the window stack, making it
difficult to recover.
     - When running under Java 7 on Linux, dialogs that are not attached to a
window (including the New Connection, Authentication, and SSH dialogs) were not
showing up in the task switcher, so accidentally clicking away from one of
those dialogs would send it to the back of the window stack, and Alt-Tab could
not be used to recover it.
     - When running in full-screen mode on Linux/GNOME, popping up the Options
or Clipboard dialogs would cause the viewer to temporarily exit full-screen
mode.

11. Fixed an issue in the Java TurboVNC Viewer whereby, when running as an
applet, it would not read the applet parameters if an SSH connection error was
encountered and the applet was reloaded in the browser.

12. The `AlwaysShowConnectionDialog` parameter is now automatically disabled in
the Java TurboVNC Viewer whenever SSH tunneling is enabled.  This prevents an
issue whereby the "New TurboVNC Connection" dialog would pop up and display
"localhost:{some random port}" after SSH authentication was completed, thus
causing confusion.

13. The Java TurboVNC Viewer was not displaying dialogs from its built-in SSH
client, which caused the SSH client to fail if the server was not in the known
hosts file.  This has been fixed.

14. If the TurboVNC Server receives a clipboard update larger than 1 MB from a
connected viewer, it will now ignore the update instead of disconnecting the
viewer.

15. The Java TurboVNC Viewer can now be used to connect to UltraVNC Repeater
instances.  Refer to the documentation for the `Via` parameter for more
details.

16. Fixed an issue in the Java TurboVNC Viewer whereby, if a connection was
established using SSH tunneling and another connection was then opened, the
viewer would fail to exit after all of these connections were subsequently
closed.

17. Fixed an issue in the vncserver script whereby the TurboVNC Server would
give an "Unrecognized option" error and fail to launch if the server was built
without the optional Java viewer.

18. Fixed a regression in the "automatic" spanning mode of the Windows TurboVNC
Viewer introduced in TurboVNC 1.2.  Ever since that release, "automatic"
spanning mode has been behaving just like "primary" mode.

19. Fixed a segfault in Xvnc that would occur whenever a viewer that was using
ZRLE encoding disconnected.

20. The following improvements have been made to The TurboVNC Server init
script:

      - The script now looks for the tvncservers file under
`@TVNC_CONFDIR@/sysconfig` (where `@TVNC_CONFDIR@` is the directory specified
in the `TVNC_CONFDIR` CMake variable) in addition to under /etc/sysconfig.
This allows the script to work properly when installed in an arbitrary
directory.
      - When invoked with the `start` argument, the script now attempts to
start only those TurboVNC sessions specified in the tvncservers file that have
not yet been started.  If all sessions in the tvncservers file have been
started, then the script does nothing.  To be consistent with other init
scripts, a success message is not printed/logged unless one or more TurboVNC
sessions is actually started successfully, and a failure message is not
printed/logged unless one or more TurboVNC sessions fails to start.
      - When invoked with the `start` argument, the script will no longer abort
if a TurboVNC session fails to start.  It will attempt to start the other
sessions.
      - When invoked with the `reload` argument, the script will now stop any
running TurboVNC sessions not specified in the tvncservers file and start any
sessions specified in the tvncservers file that aren't running.
      - Fixed a bug whereby, if an error occurred when starting a TurboVNC
session, the error was not properly logged to the system error log.
      - The script should now work on Red Hat Enterprise/CentOS 7 and recent
Fedora releases.

21. The `restricted`, `nounixlogin`, and `user` options in the Windows TurboVNC
Viewer are no longer persistent from one session to the next.  Because those
options can only be specified on the command line or in a connection info file,
automatically restoring their previous values for subsequent connections to the
same host and display number led to confusing behavior.  The viewer would apply
the previous value of the option, thus changing the viewer's behavior, but
there was no way to visually confirm why the behavior had been changed or to
change it back (unless you knew the corresponding command-line counter-curse,
but `restricted` actually didn't have one.  Oops.)  The `encoding` and `8bit`
options have been non-persistent for quite some time, for this same reason.


1.2.1
=====

### Significant changes relative to 1.2:

1. Fixed two regressions introduced in TurboVNC 1.0, one of which prevented
older (RFB <= 3.3) VNC clients from connecting successfully to the TurboVNC
Server and the other of which prevented clients other than TurboVNC and
TightVNC from connecting to the TurboVNC Server using no authentication.

2. Added a new parameter (`EncPassword`) to the Java TurboVNC Viewer that
allows the password to be specified in encrypted ASCII hex.

3. The toolbar buttons in the Java TurboVNC Viewer that send keystrokes are now
disabled when view-only mode is selected.

4. Enabled the MIT-SCREEN-SAVER X extension in the TurboVNC Server.  Modern
screen savers don't actually use this extension, but it provides an easy way
for applications to query the idle time of the X server.

5. Fixed a regression introduced in 1.2 beta1 whereby the TurboVNC Viewer
desktop shortcut installed with the Linux RPM did not work properly.

6. Fixed a bug in the Java TurboVNC Viewer's RRE decoder that was causing
pixels to be displayed incorrectly.

7. Pressing F8 (or the chosen menu key) twice in the Java TurboVNC Viewer now
sends that keystroke to the VNC server.  This emulates the behavior of the X11
viewer.

8. Added an option to vncserver that allows the output of Xvnc to be redirected
to an arbitrary file.

9. Implemented the X RANDR extension in the TurboVNC Server.  The main purpose
of this at the moment is to placate applications that check for the extension
and refuse to start without it.  The extension currently can't be used to
change the screen size (that feature will be in TurboVNC 2.0.)

10. Fixed an issue in the Java TurboVNC Viewer whereby, when a mouse button was
pressed, pressing another button or activating the scroll wheel would cause the
viewer to send a release event for the first button.

11. Fixed an issue whereby the X11 TurboVNC Viewer would fail to authenticate
if the encrypted password stored in the connection info file started with "00".

12. Fixed an invalid memory access that occurred in the TurboVNC Server after a
client disconnected.  This had the visible effect of causing an error
(`Could not disable TCP corking: Bad file descriptor`) to be printed to the
TurboVNC Server's log, but it was not known to cause any other issues.

13. Fixed an issue that prevented clipboard transfer from working properly with
applications that request the clipboard selection in a non-ASCII format.  This
was specifically known to affect rxvt-unicode.

14. The Windows TurboVNC Viewer should now build properly with Visual Studio
2012.

15. The "Uninstall TurboVNC" app should once again work on OS X 10.5.

16. Added two new command-line options to PuTTY, `-L4` and `-L6`.  These work
just like `-L`, except that `-L4` forces PuTTY to use an IPv4 interface for the
client side of the SSH tunnel, and `-L6` forces PuTTY to use an IPv6 interface
for the client side of the SSH tunnel.

17. Worked around an issue whereby the Java/Mac TurboVNC Viewer would abort
with `InStream max string length exceeded` when connecting to recent versions
of the RealVNC Server.  This was due to a protocol conflict.  Apparently,
RealVNC uses pseudo-encoding number -311 for their CursorWithAlpha extension,
but TigerVNC uses -311 for ClientRedirect.  At least for now, ClientRedirect
has been disabled in the Java TurboVNC Viewer by default, but there is a new
hidden parameter (`ClientRedirect`) that can be set to `1` to re-enable it.

18. Due to an oversight, the 3-button mouse emulation feature in the Windows
TurboVNC Viewer was being enabled by default.  This has been fixed.  3-button
mouse emulation is largely unnecessary with modern systems, and the feature is
known to cause issues with certain applications.


1.2
===

### Significant changes relative to 1.2 beta1:

1. The Mac TurboVNC Viewer no longer has a separate menu for the "About" and
"Preferences" options.  As is the case with most Mac applications, these
options are now accessed from the application menu.

2. Opening VNC viewer connection info (.vnc) files in the OS X Finder or
dragging and dropping them onto the Mac TurboVNC Viewer icon now works
properly.  Additionally, if a connection is already open, dragging and dropping
a .vnc file onto the Mac TurboVNC Viewer icon will now open a new connection.

3. VNC viewer connection info (.vnc) files can now be opened in Windows by
dragging and dropping them onto the Windows TurboVNC Viewer icon.

4. The Java TurboVNC Viewer can now be built and run with Java 5.
Consequently, the Mac TurboVNC Viewer now works with the version of Java
shipped with OS X 10.4 and 10.5.

5. Previously, when using the Java TurboVNC Viewer on Mac platforms, the menu
bar was still visible in full-screen mode, and the dock was still visible if it
was not set to auto-hide.  The Java/Mac TurboVNC Viewer now takes advantage of
the OS X Lion full-screen feature, if available, to provide a "true"
full-screen mode on OS X 10.7 and later.  On OS X 10.6 and earlier, the
behavior is unchanged.

6. Fixed a regression in the new Java TurboVNC Viewer whereby, when used as an
applet, specifying a host other than the VNC server in the `Server` parameter
had no effect.

7. Fixed various key mapping issues in the Java TurboVNC Viewer:

     - The viewer now differentiates between the numeric keypad versions of the
navigation keys (Home, End, etc.) and their equivalents on the main keyboard.
     - The viewer now differentiates between the left and right versions of the
modifier keys (Alt, Ctrl, etc.)
     - The viewer now properly handles the AltGr key on international keyboards.
     - Left Alt key sequences on Mac clients can now be used to activate
pull-down menus on Linux/Windows servers.
     - The Apple keys on Macintosh keyboards can now be used as Windows keys
when connecting to Linux and Windows servers.

8. The Windows TurboVNC Viewer now properly differentiates between the numeric
keypad versions of the navigation keys (Home, End, etc.) and their equivalents
on the main keyboard.

9. Fixed a minor issue in the Windows TurboVNC Viewer whereby it would trigger
an Alt keypress on the remote desktop whenever an AltGr key symbol was typed
repeatedly.

10. Added LSB headers to the TurboVNC Server init.d script (tvncserver) in
order to avoid insserv errors/warnings with recent Debian releases.

11. The TurboVNC Server can now build its font path from a font catalogue, on
systems that support them (such as RHEL 6.)  vncserver will now check for the
existence of a font catalogue at /etc/X11/fontpath.d and use it if it exists.
For systems that do not support font catalogues, vncserver will now check for
the existence of the Liberation fonts (used by LibreOffice) and the Ghostscript
fonts and add them to the fontpath.

12. Fixed an error that occurred when running `/etc/init.d/tvncserver stop`
with an empty /etc/sysconfig/tvncservers file.

13. Fixed an issue whereby the X11 TurboVNC Viewer would fail with an error
message of `Password stored in connection info file is invalid` when loading a
connection info file in which the encrypted password contained "00".

14. Fixed an issue with the multi-monitor spanning feature in the Mac/Java
TurboVNC Viewer whereby the remote desktop would appear on the secondary
monitor instead of the primary monitor when the span mode was set to "Primary"
(or when the span mode was set to "Auto" and the remote desktop area was less
than the primary monitor area.)

15. Fixed Xvnc build errors on Solaris and FreeBSD.

16. The X11 TurboVNC Viewer will now temporarily ungrab the keyboard if the
viewer window loses focus while the keyboard is grabbed (when not in
full-screen mode, this can happen if the user clicks on another window, or if a
notification dialog from the window manager pops up.)

17. Fixed TurboVNC Server build issues that occurred under Ubuntu 13.04 (and
possibly other new Linux distros.)

18. The default xstartup.turbovnc script that the TurboVNC Server creates will
now use the GNOME fallback window manager on Ubuntu 12.10 and later, assuming
that the "gnome-session-fallback" package is installed.  TurboVNC cannot use
Unity 3D (yet), and Unity 2D was removed in Ubuntu 12.10 and later.

19. Fixed a segfault that occurred when starting the TurboVNC Server on Ubuntu
13.04 and Fedora 18 (and possibly other new Linux distros.)

20. The X11 TurboVNC Viewer will now automatically release the modifier keys if
the viewer window loses focus.  This fixes an issue whereby using Alt-Tab to
switch windows would leave the Alt key in a pressed state on the server.

21. The Java TurboVNC Viewer was ignoring the `PORT` parameter when SSH
tunneling was enabled.  This has been fixed.

22. Fixed an issue whereby the Java TurboVNC Viewer window would not close if
the server disconnected and the viewer was either running in listen mode or it
had more than one connection open.

23. Since there is no way to see the console output in a Java Web Start
environment or when starting the Java TurboVNC Viewer from an icon, it is
difficult or impossible in such cases to debug errors that are only displayed
to the console.  Thus, the Java TurboVNC Viewer now displays all errors both to
the console and in an error dialog.

24. Fixed an issue in the Java TurboVNC Viewer whereby pressing Alt-Tab would
change the keyboard focus to another window but would not bring the other
window to the top.  This was originally a feature, not a bug, and was meant to
be a stopgap substitute for a proper keyboard grabbing feature, but the
behavior causes confusion and does not achieve the primary goal that a keyboard
grabber should achieve (sending special keystrokes to the server.)

25. Fixed an issue in the TurboVNC Server whereby the value of `$desktopName`
was being ignored if it was present in the TurboVNC config file.



1.1.90 (1.2 beta1)
==================

### Significant changes relative to 1.1:

1. Modified the default xstartup.turbovnc script so that it loads the 2D rather
than the 3D version of the window manager on recent Ubuntu systems.  This
specifically fixes an issue whereby the Unity window manager in Ubuntu 12.04
would not display its menus.

2. Added a command-line switch (`-norender`) to Xvnc that can be used to
disable the X RENDER extension.

3. Added a command-line option (`-xstartup`) to vncserver that allows a custom
startup script to be specified.  This is useful along with the `-fg` switch,
because it allows a full-screen application to be launched without a window
manager and causes the TurboVNC session to terminate when the application
exits.

4. The Java TurboVNC Viewer has been completely rewritten and now supports most
of the features of the TurboVNC native viewers, as well as all of the features
of, and a rich GUI inspired by, the TigerVNC 1.2 Java viewer.  In addition, the
new Java TurboVNC Viewer has the ability to use libjpeg-turbo via JNI to
decompress JPEG images, giving it levels of performance approaching the native
viewers.  It also has an embedded SSH client and fully-integrated support for
SSH tunneling.  The new Java viewer now replaces the X11 TurboVNC Viewer on Mac
systems, since it has higher overall performance on that platform (due to
performance limitations of XQuartz) and much better usability.

5. IPv6 support

6. Implemented v0.10 of the X RENDER extension, to address compatibility
problems with newer applications that assumed v0.10 functionality was available
without checking for it.

7. Overhauled the build and packaging system.  All platforms now use CMake, and
the Java code can be built either as part of a Unix or Windows build or as a
stand-alone project.

8. Renamed the resource file for the X11 TurboVNC Viewer to "Tvncviewer" to
avoid conflicts with TightVNC.  This specifically fixes an issue whereby the
TurboVNC Viewer would display its menus and titlebar incorrectly when running
on a system that had the TightVNC Viewer installed.

9. The Windows TurboVNC Viewer now accepts a scaling factor of `fixedratio`
when using the `/scale` switch on the command line.  This was previously called
"Auto" in the GUI, but the name was changed to match the Java TurboVNC Viewer.

10. All default options in the X11 TurboVNC Viewer now have a command-line
equivalent, which is useful in case the defaults are overridden using a
resource file.

11. Added a keyboard grabbing feature to the Windows TurboVNC Viewer so that it
can optionally send special keystrokes (Alt-Tab, Ctrl-Esc, Menu key, etc.) to
the VNC server.  The default behavior of this option is to enable grabbing only
in full-screen mode (as the X11 TurboVNC Viewer already did), but a
command-line option (`-grabkeyboard`) can be used to configure keyboard
grabbing to be always on or always off.  Additionally, grabbing can always be
turned on/off by pressing CTRL-ALT-SHIFT-G.  The X11 TurboVNC Viewer has been
extended to support the same functionality.

12. Where possible, the naming of command-line options, resources, menu
options, and parameters has been reconciled among the Windows, X11, and Java
TurboVNC Viewers.

13. The multi-screen window spanning feature in the Windows TurboVNC Viewer
should now behave properly when fixed-ratio scaling is used.

14. Fixed a logic error in the "automatic" spanning mode of the Windows
TurboVNC Viewer, whereby it would try to extend the remote desktop window
horizontally across multiple screens if the remote desktop height was taller
than the local screen but the width was the same.

15. The Windows TurboVNC Viewer will now return a non-zero exit status if it
encounters an error.  This allows batch scripts to start the viewer with
`start /wait` and check its exit status.

16. The `/password` option in the Windows TurboVNC Viewer should now work
again.

17. Fixed an intermittent failure with the idle timeout feature in the TurboVNC
Server.  This failure was caused by the fact that the X server used a 32-bit
value to store the number of milliseconds since 1970, and this value was
wrapping around to 0 every 49 days.  If a TurboVNC Server session was started
near the end of one of these 49-day cycles and the idle timeout was set for
several days into the future, the expiration value for the timer would wrap
around and become lower than the current time, thus causing the TurboVNC Server
to exit.

18. Worked around a bug in the version of TigerVNC Server that ships with Red
Hat/CentOS 6, whereby dragging links from Firefox (running on the remote
desktop) to the remote desktop would cause the X11 TurboVNC Viewer to crash.

19. Added support for the RFB flow control extensions developed by the TigerVNC
Project.  Clients that support these extensions (including TurboVNC 1.2 and
later and TigerVNC 1.2 and later) can receive updates from the server without
having to explicitly request them, which improves performance on high-latency
networks.

20. The titlebar of all flavors of the TurboVNC Viewer now displays the last
encoding received from the server rather than the requested encoding.  This is
useful when connecting to RealVNC and other servers that do not support Tight
encoding.

21. Implemented an interframe comparison engine (ICE) in the TurboVNC Server,
which prevents duplicate framebuffer updates from being sent as a result of an
application drawing the same thing over and over again.  The ICE will normally
be enabled when Compression Level 5 or above is requested by a VNC viewer, but
you can also enable/disable it manually by passing command-line arguments to
Xvnc.

22. Added experimental (and currently undocumented) support for the `-via` and
`-tunnel` command-line options to the Windows TurboVNC Viewer.  These work the
same way as the equivalent options in the X11 TurboVNC Viewer.  Currently, they
require Cygwin SSH, since PLink does not have the ability to detach its process
after authentication.


1.1
===

### Significant changes relative to 1.1 beta1:

1. Improved the behavior of the automatic lossless refresh feature so that it
doesn't send an ALR for screen regions that were sent using JPEG but then
re-sent using a lossless subencoding (such as color index) prior to the ALR
being triggered.

2. Previously, empty cursors were not encoded correctly by the TurboVNC Server,
and this caused some VNC viewers (notably, TigerVNC) to render a system default
cursor instead of the empty cursor.  This has been fixed.

3. Fixed the rendering of empty cursors in the X11 TurboVNC Viewer.  This also
fixed an issue whereby the viewer would crash when opening recent versions of
xterm in the TurboVNC Server session.

4. Fixed a crash
(`xcb_io.c:507: _XReply: Assertion '!dpy->xcb->reply_data' failed`) that
occurred when running recent versions of twm in a TurboVNC Server session.
Fixing this also fixed an issue in openSUSE 12 whereby Xvnc would abort with
`could not open default font 'fixed'`.

5. The Windows TurboVNC Viewer will now properly switch into/out of full-screen
mode whenever the "Full-screen mode" check box is selected or de-selected in
the TurboVNC Viewer Options dialog and a connection is active.

6. Fixed an issue whereby the Windows TurboVNC Viewer window would remain on
top of other windows and dialogs (including dialogs raised by the TurboVNC
Viewer itself) when exiting full-screen mode.

7. Beginning with 1.1 beta1, whenever the Windows TurboVNC Viewer left
full-screen mode, it would set the window size and position to default values.
It now restores the window's size and position to the values they had before
the viewer was placed in full-screen mode.  An option has been added to the
system menu to manually resize and reposition the window to default values
(taking into account the spanning option.)

8. Added Ctrl-Alt-Shift-F and Ctrl-Alt-Shift-R hotkeys to the X11 TurboVNC
Viewer to toggle full-screen mode and request a refresh (respectively.)  These
emulate the behavior of the Windows TurboVNC Viewer.

9. Fixed several invalid reads/writes reported by valgrind.  These occurred
under certain circumstances when a Tight-compatible client disconnected from a
TurboVNC session, and the issues were known to cause one rare segfault when
disconnecting from a TurboVNC session that had multithreading and ALR enabled.

10. Fixed an issue whereby selecting text in xterm would sometimes cause the
Windows TurboVNC Viewer to abort with "Failed to open clipboard."

11. Fixed an issue whereby clipboard changes were being immediately sent back
to the viewer as soon as they were received by the server.  This specifically
caused copy/paste operations in some Windows applications to behave
incorrectly, since the clipboard contents were being converted from Unicode to
ASCII when they were bounced back.



1.0.90 (1.1 beta1)
==================

### Significant changes relative to 1.0.2:

1. autocutsel has been replaced with a more robust and configurable clipboard
transfer mechanism based on the VNC X11 extension and the vncconfig utility
used by RealVNC.

2. The Unix TurboVNC Server now implements the X RENDER extension, which
enables font anti-aliasing and animated cursors and fixes various issues with
KDE 4.  Additionally, the server now embeds the FreeType library for improved
font display and compatibility.

3. Replaced the broken "software cursor" implementation of rich (full-color)
cursor rendering in the X11 TurboVNC Viewer with a proper implementation that
uses the Xcursor library.  This specifically fixes pointer issues that occurred
when connecting to the Unix TigerVNC 1.2 Server.

4. The Unix TurboVNC Server now implements rich (full-color) cursor encoding,
so viewers that support this RFB extension (including TigerVNC 1.2 and later
and TurboVNC 1.1 and later) can display full-color animated cursors when
connecting to the TurboVNC Server.

5. Enhanced the screen scaling feature in the Java TurboVNC Viewer and added a
GUI option to control it.

6. Added an idle timeout feature to the Unix TurboVNC Server which, when
enabled, will cause it to exit automatically whenever a certain number of
seconds have elapsed since the last active VNC viewer connection.

7. Added a `-noxstartup` option to vncserver, which prevents it from executing
the xstartup.turbovnc script.  This is useful for those who may want to
occasionally run TurboVNC without a window manager.

8. Fixed image corruption that occurred when using the Lossless Tight or
Lossless Tight + Zlib protocols with the X11 TurboVNC Viewer running on a
16-bit-per-pixel X server.

9. Fixed TurboVNC Server build on ARM-based Linux systems

10. Added Xvnc command-line options and authentication configuration file
directives for disabling clipboard transfer.

11. Added two options (`-alrqual` and `-alrsamp`) to Xvnc that allow for
sending automatic lossless refreshes using JPEG rather than mathematically
lossless images.

12. Fixed an issue whereby the Windows TurboVNC Viewer, when running on Windows
7, would display an all-white image on one of the monitors in a multi-monitor
configuration the first time the viewer was switched to full-screen mode.

13. Changed the default full-screen behavior in the Windows TurboVNC Viewer
such that the window only spans multiple monitors if the remote desktop will
not fit on a single monitor.  Added options to the command line, GUI, and .vnc
file to allow users to override this behavior and force full-screen windows to
always occupy one monitor or all monitors.

14. Fixed numerous issues with full-screen mode in the X11 TurboVNC Viewer.

15. Implemented `-autopass` option in the Windows TurboVNC Viewer, which works
the same as the equivalent option in the X11 TurboVNC Viewer.  In both viewers,
`-autopass` can now be used with Unix Login Authentication as well.

16. Added an option to the F8 menu in the X11 TurboVNC Viewer that allows
view-only mode to be enabled/disabled on an active VNC connection.

17. The TurboVNC Server should now build and run properly on FreeBSD.

18. Added ZRLE encoding to the Unix TurboVNC Server, which may improve
performance with the RealVNC Viewer under certain circumstances.

19. The Windows TurboVNC Server (WinVNC) is no longer provided, as it was badly
broken, and the only way to fix it would have involved a complete
re-architecture.

20. The TurboVNC Server now includes a non-functional implementation of the X
Input extension, primarily to avoid crashes in recent versions of GNOME that
aren't smart enough to check for that extension's existence before using it.

21. Implemented an authentication plugin mechanism for the Java TurboVNC
Viewer, which allows sites to write their own external class for obtaining Unix
login authentication credentials.

22. Although there were no known issues with the implementation of the
automatic lossless refresh feature in TurboVNC 1.0.x, the feature was
re-implemented without the use of threads, in order to make it easier to
maintain.


1.0.2
=====

### Significant changes relative to 1.0.1:

1. Fixed seg fault in the Unix TurboVNC Server which occurred under certain
rare circumstances when disconnecting a viewer from a server that was using
multi- threaded compression.

2. If the user overrides the location of the password file (by passing
`-rfbauth {file}` to vncserver), then vncserver will no longer prompt the user
to create a new VNC password if ~/.vnc/passwd does not exist.

3. Fixed seg fault in the Unix TurboVNC Server which occurred under certain
rare circumstances when shutting down a server that was using multi-threaded
compression.

4. Fixed an issue in the Java TurboVNC Viewer whereby it was sending spurious
KeyPress/KeyRelease events in conjunction with certain mouse events.

5. Numerous build system changes, including the use of CMake for Windows builds
and support for out-of-tree Xvnc builds.

6. Added a `-config` option to the X11 TurboVNC Viewer which allows it to read
.vnc connection info files saved by the Windows TurboVNC Viewer.


1.0.1
=====

### Significant changes relative to 1.0:

1. Fixed seg fault in the Unix TurboVNC Server which occurred in the
`miRegionDestroy()` function under certain rare circumstances.

2. Fixed seg fault in the Unix TurboVNC Server which occurred in the
`cfb32ClippedLineCopy()` function under certain rare circumstances.

3. vncserver arguments can now be specified in the /etc/sysconfig/tvncservers
file using an array called `VNCSERVERARGS`.  This works the same as in RealVNC
4.

4. The `-via` option in the X11 TurboVNC Viewer now uses ephemeral ports to
bind the client side of the SSH tunnel.  This should prevent port conflicts
that could occur when multiple vncviewer instances tried to use the `-via`
option simultaneously.


1.0
===

### Significant changes relative to 1.0 beta1:

1. The Windows TurboVNC Viewer should now work properly with displays that use
a 24-bit (as opposed to a 32-bit) pixel format (this includes Remote Desktop
and Citrix sessions.)

2. Eliminated the use of the dangerous `alloca()` (stack-based allocation)
function in Xvnc and replaced it with heap-based allocation (`malloc()` and
`free()`.)

3. Eliminated a segfault in `pthread_join()` which would occur under certain
circumstances when using multithreading in the TurboVNC Server.

4. To maintain consistency with other versions of VNC, added `-nohttpd` option
to vncserver as an alias for `-nohttp`.

5. Fixed an issue whereby certain applications which draw hundreds of thousands
of tiny lines or points on the screen would cause the TurboVNC Viewer to abort
with an "unhandled message type" error or would cause the viewer to freeze for
several minutes.

6. Fixed a security loophole whereby RealVNC clients were able to connect with
a blank password if PAM and OTP authentication were enabled on the TurboVNC
Server and the OTP was not set.


0.6.90 (1.0 beta1)
==================

### Significant changes relative to 0.6:

1. Multi-threaded the Tight encoder in the Unix/Linux TurboVNC Server so that
it can take advantage of multi-core systems.  See documentation for further
details.

2. Added authentication extensions which allow clients to authenticate using
one-time passwords and Unix login credentials.  See documentation and man pages
for details.

3. Added scroll wheel support to the Java TurboVNC viewer (requires JRE 1.4 or
later.)

4. Added automatic lossless refresh (ALR) feature, which sends a mathematically
lossless copy of the screen during periods of inactivity.  See documentation
for details.

5. For Linux, Mac/Intel, Solaris/x86, and Windows systems, the default build of
TurboVNC no longer uses TurboJPEG/IPP (which was based on the proprietary Intel
Performance Primitives) or Sun mediaLib.  Instead, TurboVNC now uses
libjpeg-turbo, a fully open source vector-accelerated JPEG codec developed in
conjunction with the TigerVNC Project (and based on libjpeg/SIMD.)

    As a result of this, the TurboVNC RPMs and DEBs are now self-contained.

6. The uninstall script in the Mac distribution package should now work on OS X
10.6.

7. Added 64-bit Solaris and Windows binary packages, as well as a universal
32/64-bit binary package for OS X.

8. Fixed a number of interaction issues between turbovncserver.conf and the
vncserver script, and added configuration file options for the new TurboVNC
features, such as ALR.

9. Performance of the OS X viewer should generally be better (prior versions
were not being built with compiler optimizations, due to the use of the
antiquated imake build system.)

10. Generally robustified `vncserver -kill`.  In this release, if Xvnc aborts
unexpectedly for any reason, then attempting to run `vncserver -kill :N` for
the session will print a warning message about the process aborting and will
then try to clean up the orphaned X socket files.  If Xvnc deadlocks, then
attempting to run `vncserver -kill :N` will warn about the deadlock and forego
deleting the PID file (so you can `kill -9` the process yourself and then
re-run `vncserver -kill` to clean up the orphaned files.)

11. Integrated autocutsel into the TurboVNC Server distribution for Unix/Linux
in order to work around issues with the clipboard transfer feature.

12. In the Windows TurboVNC Viewer, full-screen mode will now span all monitors
in a multi-monitor configuration, not just the primary monitor.

13. Added automatic clipboard transfer to the Java TurboVNC Viewer.

14. Removed the warning dialog in the Windows viewer if full screen mode is
activated via a hotkey (the reasoning being that if the user knows the hotkey
to activate full screen mode, then they know how to deactivate it as well.)

15. Fixed an issue whereby the TurboVNC Server would crash when a VirtualGL
window was resized to larger than 2.75 megapixels.

16. Added double buffering option to Java TurboVNC Viewer.

17. "Login again" button now works when running the Java TurboVNC Viewer as a
standalone application.

18. Fix a couple of potential buffer overruns in Xvnc's embedded HTTP server.


0.6
===

### Significant changes relative to 0.5.1:

1. Integrated TightVNC 1.3.10 enhancements and fixes, including 64-bit support
for Linux and Solaris.  See the TightVNC Change Log (WhatsNew or WhatsNew.txt)
for more details.

2. When connecting a TigerVNC or TightVNC Viewer to a TurboVNC server, the
TurboVNC server will now translate JPEG quality levels into actual JPEG quality
and subsampling using the same translation table used by the TigerVNC Server.
See the TurboVNC User's Guide for more details.

3. Created the TurboVNC User's Guide, which mostly consists of the TurboVNC
chapters that were formerly in the VirtualGL User's Guide.

4. Fixed an issue on OpenSolaris whereby a TurboVNC session would start but
would display a blank X Windows screen with no window manager.  This was due to
the vncserver script passing an argument of `-nolisten local` to Xvnc on all
Sun platforms, and this doesn't work on OpenSolaris.  The `-nolisten local`
option was necessary to get Xvnc to work on earlier versions of Solaris, but it
is no longer necessary with Solaris 10.  If you are running TurboVNC on an
older Solaris release, then you can pass `-nolisten local` to
/opt/TurboVNC/bin/vncserver to get back the behavior of TurboVNC 0.5.x (or
uncomment that line in the vncserver script.)


0.5.1
=====

### Significant changes relative to 0.5:

1. Fixed a buffer overrun issue in TurboJPEG/mediaLib that may have caused
problems on Solaris/x86 TurboVNC servers.

2. Developed a proper uninstaller app for the Mac OS X TurboVNC package.

3. Modified `MAXINST` variable in SUNWtvnc Solaris package to prevent multiple
instances of this package from being installed simultaneously.


0.5
===

This release was historically part of the Sun Shared Visualization v1.1.1
product.

### Significant changes relative to 0.4:

1. The Windows TurboVNC Server now works properly (albeit more slowly) when the
server's graphics card is configured for a 16-bit pixel depth.

2. 0.4[12] was supposed to allow vncserver to work even if xauth was not in
the PATH, but unfortunately there was a bug in that patch.  This bug has been
fixed, so vncserver should now really work if xauth is not in the PATH.

3. It was discovered that pure JPEG encoding was not the most efficient method
of compression for all 3D image workloads.  Particularly, CAD applications and
other types of applications that generate images with sharp edges and reduced
color palettes compress much better with traditional Tight encoding.  However,
traditional Tight encoding is too slow to stream real-time full-screen 3D
images.  Thus, a hybrid scheme was developed which uses the fastest elements of
Tight encoding with minimal Zlib compression for low-color-depth image tiles
and continues to use TurboJPEG for high-color-depth image tiles.  The resulting
compression scheme should be both tighter and faster than the pure TurboJPEG
protocol used by previous versions of TurboVNC.

4. The Unix TurboVNC Server now executes ~/.vnc/xstartup.turbovnc instead of
~/.vnc/xstartup.  This is to avoid conflicts with other VNC flavors.

5. Shift-Click and Control-Click now work properly when using the X11 TurboVNC
Viewer.

6. Further optimized the Huffman encoder in the mediaLib implementation of
TurboJPEG.  This should decrease the CPU usage when running TurboVNC on Solaris
servers, particularly Solaris/x86 servers.

7. When running in OpenSolaris, the default xstartup.turbovnc file did not
launch JDS, since the JDS launch script is in a different location on that
platform.  This resulted in a TurboVNC session that had no window manager.
This has been fixed.

8. Increased TCP send buffer size to 64k in the Windows TurboVNC Server.  This
should improve performance significantly.

9. Generally improved compatibility with TightVNC and RealVNC.  This included
adding back in the Hextile decoder to the TurboVNC Viewer and adding back in
support for 8-bit and 16-bit color depths (in both the viewer and the server.)
See the TurboVNC Compatibility Guide for more information.

10. Changed default pixel format for Solaris TurboVNC servers to ARGB/BGRA.
This should improve performance by a bit on SPARC servers.

11. Added 200% scaling option to the Windows TurboVNC Viewer GUI.

12. The TurboVNC server script now tries to figure out an appropriate font path
for the system rather than using the X Font Server, since xfs is not
universally available.  If the script fails to figure out an appropriate font
path, then it will fall back to using xfs.

13. Changed the scrollbar behavior in the X11 TurboVNC Viewer to more closely
match that of "modern" windowing systems, in which the left mouse button is
used to scroll in both directions.

14. Fixed an issue whereby GNOME would fail to start on SuSE 10 machines if CSh
was the default shell.  The issue was that /opt/gnome/bin was not being added
to the PATH by /etc/csh.cshrc.  This was worked around by adding it to the PATH
in ~/.vnc/xstartup.turbovnc.

15. On Solaris machines, TurboVNC will attempt to load the window manager
startup script specified in ~/.dt/sessions/lastsession.  This caused problems
if the home directory was shared among multiple machines and the startup script
did not exist on some of the machines (for instance, if the user's default
windowing environment was JDS but they attempted to start TurboVNC on an older
Solaris machine that had only CDE.)  This has been fixed by adding a line to
xstartup.turbovnc which checks to make sure that the script specified in
~/.dt/sessions/lastsession exists and is executable before executing it.

16. Fixed a bug in the color conversion routines of TurboJPEG/mediaLib which
caused the Solaris TurboVNC Viewer to display spurious multi-colored vertical
lines when 2X or 4X subsampling was used.

17. When requesting a Lossless Refresh using the TurboVNC Viewer on a Linux or
Solaris/x86 client, the pixels obscured by the F8 dialog will be regenerated
using lossy compression after the lossless refresh has occurred.  This makes
the lossless refresh somewhat ineffective.  We were not able to find a fix for
this in 0.5, but as a workaround, you can now use the hotkey sequence
CTRL-ALT-SHIFT-L to request a Lossless Refresh without popping up the F8
dialog.


0.4
===

This release was historically part of the Sun Shared Visualization v1.1
product.

### Significant changes relative to 0.3.3:

1. Added relevant patches from TightVNC 1.3.9

2. Added lossless refresh feature, which instructs the server to send a
mathematically lossless (Zlib-encoded RGB) copy of the current screen.  This
feature does not currently work with the Windows TurboVNC Server, because the
Windows TurboVNC Server processes framebuffer update requests asynchronously.

3. Modified /opt/TurboVNC/bin/vncserver so that it invokes Xvnc with the
arguments `-deferupdate 1`.  This sets the deferred update timer to 1 ms rather
than its default value of 40 ms, which has two effects:

     - It improves the performance of Solaris TurboVNC servers dramatically
when connecting to them over a high-speed network.
     - It eliminates the need for the "High-Latency Network" switch in the
TurboVNC Viewer.  In prior versions of TurboVNC, leaving this switch on when
connecting over a high-speed network incurred a severe performance penalty.
Since this is no longer the case, the switch is left on all the time and is no
longer configurable.

    NOTE:  The afore-mentioned performance penalty will still be incurred
when connecting a TurboVNC 0.4 viewer to an older (pre-0.4) TurboVNC server
over a high-speed network.  Start the server with
`/opt/TurboVNC/bin/vncserver -deferupdate 1` to avoid this, or simply upgrade
the server to 0.4.

4. Added an option for lossless (uncompressed RGB) image encoding.  This is
useful for reducing CPU usage on the server and client (at the expense of
increased network usage) when connecting over a gigabit (or faster) network.

5. Added a "Medium Quality" connection profile to the Windows, X11, and Java
TurboVNC Viewers (and subsequently removed the "Broadband (favor image
quality)" profile, which is no longer necessary due to [3].)  The "Medium
Quality" profile sets the JPEG quality to 80 with 2X chrominance subsampling,
which (on average) should use about half the bandwidth of the "High Quality"
profile (quality=95, no subsampling) and twice the bandwidth of the "Low
Quality" profile (quality=30, 4x subsampling.)

6. Added an additional subsampling option to enable grayscale JPEG encoding.
This provides additional bandwidth savings over and above chrominance
subsampling, since grayscale throws away all chrominance pixels.  It is
potentially useful when working with applications that already render grayscale
images (medical imaging, etc.)

7. Fixed embedded Java viewer on Windows TurboVNC servers (.jar file did not
include all of the necessary classes)

8. Created symlink from /opt/TurboVNC to /opt/SUNWtvnc in the Solaris packages
so that Solaris and Linux would have a consistent interface.

9. Removed unnecessary pixel format translation when sending JPEG from a big
endian server to a little endian client (or vice versa.)  This improves
performance a bit when connecting x86 clients to Sparc servers or vice versa.

10. Included mediaLib Huffman encoding optimizations contributed by Sun.  This
boosts the performance of the Solaris TurboVNC server and client by as much as
30%.

11. Changed default geometry to 1240x900, an appropriate size for most
1280x1024 displays.

12. vncserver now looks for xauth in /usr/X11R6/bin and /usr/openwin/bin before
searching the PATH.  Those directories are sometimes not in the PATH on Linux
and Solaris systems.

13. Modified Mac package such that /opt/TurboVNC/bin/vncviewer links to
/opt/TurboVNC/lib/libturbojpeg.dylib rather than to
/opt/VirtualGL/lib/libturbojpeg.dylib (oops.)  This was causing the Mac
TurboVNC Viewer to fail unless VirtualGL was also installed.

14. Included an optimized version of PuTTY 0.60 in the Windows build (and
viewer package.)  It is recommended that this version be used when tunneling
TurboVNC connections over SSH, as it will perform as much as 4X as fast as the
stock version of PuTTY 0.60.

15. Fixed bug in vncserver script which was uncovered by running it with recent
versions of Perl.

16. Changed name of options registry key to avoid conflict with TightVNC

17. Open Java viewer in a new window (edit /opt/TurboVNC/vnc/classes/index.vnc
to change this back.)


0.3.3
=====

This release was historically part of the Sun Shared Visualization v1.0.x
product.

### Significant changes relative to 0.3.2:

1. Added a new preset to all VNC viewers which allows the user to select both
the WAN protocol optimizations and perceptually lossless image quality.

2. Added `-list` option to vncserver which lists all VNC sessions (not just
TurboVNC sessions) running under the current user account on the current
machine.  This new option is documented in the VGL/TVNC docs as well as the
TurboVNC man pages.

3. vncserver will no longer fail if the `USER` environment variable is unset.
That environment variable is unused in the script, so checking for its presence
was apparently a vestigial feature.

4. Modified Windows build to embed a proper version number in TurboVNC.exe.

5. Changed the fallback logic in the default ~/.vnc/xstartup file so that GNOME
is used as the window manager on Solaris if it is available and if
~/.dt/sessions/lastsession doesn't exist.  Otherwise, CDE is used.

6. Fixed an issue whereby GNOME would fail to start in TurboVNC if TurboVNC was
launched from within another X session.


0.3.2
=====

### Significant changes relative to 0.3.1:

1. Incorporated TightVNC 1.3.8 patches (where applicable)

2. Now using a single RPM to support multiple Linux distributions.

3. Fixed a couple of issues in the fallback logic of the default
~/.vnc/xstartup script.  It should now properly run fvwm2 or twm if KDE, CDE,
or GNOME are not available.

4. First pass at a Mac build of the TurboVNC Viewer.  The Mac version is an X11
Unix app and thus needs to be run inside an xterm.  It should otherwise behave
and perform identically to the Linux version.

5. Increased the size of the TurboJPEG compression holding buffer to account
for rare cases in which compressing very high-frequency image tiles
(specifically parts of the 3D Studio MAX Viewperf test) with high quality
levels (specifically Q99 or above) would produce JPEG images that are larger
than the uncompressed input.

    Linux users will need to upgrade to the TurboJPEG 1.04 RPM to get this fix.
For other platforms, the fix is included in the TurboVNC 0.3.2 packages.

6. Added `-fg` switch to vncserver to make it (optionally) run in the
foreground.  This allows you to kill the VNC server by pressing CTRL-C in the
shell you used to start it.  When in foreground mode, you can also kill the VNC
server by logging out of the window manager inside the VNC session.

7. The /etc/init.d/tvncserver script, which can be used to launch multiple
TurboVNC servers at boot time, should now work properly on SuSE systems.

8. Added TurboVNC protocol optimizations to the Java viewer and made its
configuration options match the other TurboVNC viewers.  The Java viewer still
uses a slower codec, so it is about 3X slower in a LAN environment than the
native viewer.  The Java viewer also lacks double buffering support.  But with
these new optimizations, the native and Java viewers should now perform
similarly over a wide-area network.

9. Added a `-password` option to the Windows TurboVNC Viewer to allow one to
pass the VNC password as plain text.


0.3.1
=====

### Significant changes relative to 0.3:

1. Automatically start Xvnc with `-nolisten local` on Solaris servers.  On
Solaris, /tmp/.X11-unix is not world writable by default, so it is necessary to
either start Xvnc with `-nolisten local` (which forces Xvnc to listen on a tcp
port rather than a local pipe) or to make /tmp/.X11-unix world writable.  The
former approach seemed like the lesser of two evils.

2. The vncserver startup script now sets VGL_COMPRESS to 0 automatically, so it
is no longer necessary to supply a `-c 0` argument to vglrun when running
inside a TurboVNC session.

3. The JPEG quality slider in the Unix client's F8 popup menu will now respond
to any mouse button, not just the middle one.

4. The WAN protocol optimizations can now be switched on and off.  It has been
discovered that these optimizations produce slower performance on a LAN, so it
is preferable only to use them on high-latency networks.

    On the Windows client, the "Broadband/T1" preset now enables the WAN
protocol optimizations in addition to setting quality=30 and subsampling=4:1:1.
Similarly, the "High-speed Network" preset disables WAN optimizations in
addition to setting quality=95 and subsampling=4:4:4.  WAN optimizations can
also be configured via an additional check box ("High-Latency Network") in the
Options dialog or through two new command line switches: `/lan` and `/wan`.

    On the Linux/Unix client (vncviewer), the default is no WAN optimizations,
quality=95, and subsampling=4:4:4.  You can specify an argument of `-wan` to
enable WAN optimizations or `-broadband` to enable WAN optimizations,
quality=30, and subsampling=4:1:1.  The F8 popup menu also contains a new
button for enabling/disabling WAN optimizations, and the Broadband and LAN
presets in this window will enable and disable WAN optimizations
(respectively.)


0.3
===

### Significant changes relative to 0.2:

1. Solaris/x86 support

2. Added a JPEG quality slider to the Unix/Linux F8 popup menu in vncviewer

3. Patches from TightVNC 1.3dev7, including a great many usability improvements
on the Windows client

4. Improved performance on broadband connections
