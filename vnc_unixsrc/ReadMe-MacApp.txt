The Mac TurboVNC Viewer is a Unix application which must be executed inside
the Mac X11 environment.  To do this,

-- Install the Mac X11 Application (this application is not installed by
default but is available on the OS X installation discs.)

-- Start the X11 application (in Applications->Utilities)

-- Start a new xterm (Command-N) if one isn't started already

-- In the xterm window, type

   /opt/TurboVNC/bin/vncviewer

-- If connecting to a TurboVNC server over a high-latency, low-bandwidth
   network, instead type

   /opt/TurboVNC/bin/vncvewer -broadband

-- Once connected, you can press the F8 key to bring up a configuration
   dialog that will allow you to adjust the image quality and other settings.

-- Type

   man -M /opt/TurboVNC/man vncviewer

   for more advanced usage
