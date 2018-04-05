Building TurboVNC
=================


Build Requirements
------------------


### All Systems

- [CMake](http://www.cmake.org) v2.8.11 or later

- libjpeg-turbo SDK v1.0.0 or later (v1.4 or later required if building the
  Java viewer.)
  * The libjpeg-turbo SDK binary packages can be downloaded from the "Files"
    area of <http://sourceforge.net/projects/libjpeg-turbo>.
  * The TurboVNC build system will search for the TurboJPEG header and
    library under **/opt/libjpeg-turbo** on Unix or **c:\libjpeg-turbo[64]** on
    Windows, but you can override this by setting the `TJPEG_INCLUDE_DIR` CMake
    variable to the directory containing turbojpeg.h and the `TJPEG_LIBRARY`
    CMake variable to either the full path of the TurboJPEG library against
    which you want to link or a set of link flags needed to link with the
    TurboJPEG library (for instance,
    `-DTJPEG_LIBRARY="-L/opt/libjpeg-turbo/lib64 -lturbojpeg"` or
    `-DTJPEG_LIBRARY="-libpath:c:/libjpeg-turbo64/lib turbojpeg.lib"`.)


### Linux and other Un*x O/S's (except Mac)

- GCC (or a GCC-compatible compiler)

- X11 development kit

- PAM development kit [if building the TurboVNC Server]

- JDK 8 or OpenJDK 1.7 or later [if building the TurboVNC Viewer]
  * For systems that do not provide a JDK, download the
    [Oracle Java Development Kit](http://www.oracle.com/technetwork/java/javase/downloads)


### Windows

- Microsoft Visual C++ 2005 or later

  If you don't already have Visual C++, then the easiest way to get it is by
  installing the
  [Windows SDK](http://msdn.microsoft.com/en-us/windows/bb980924.aspx).
  The Windows SDK includes both 32-bit and 64-bit Visual C++ compilers and
  everything necessary to build TurboVNC.

  * You can also use Microsoft Visual Studio Express/Community Edition, which
    is a free download.  (NOTE: versions prior to 2012 can only be used to
    build 32-bit code.)
  * If you intend to build TurboVNC from the command line, then add the
    appropriate compiler and SDK directories to the `INCLUDE`, `LIB`, and
    `PATH` environment variables.  This is generally accomplished by executing
    `vcvars32.bat` or `vcvars64.bat` and `SetEnv.cmd`.  `vcvars32.bat` and
    `vcvars64.bat` are part of Visual C++ and are located in the same directory
    as the compiler.  `SetEnv.cmd` is part of the Windows SDK.  You can pass
    optional arguments to `SetEnv.cmd` to specify a 32-bit or 64-bit build
    environment.


### Mac

- Xcode 4.1 or later (OS X 10.7.x or later SDK required)

- JDK 8 or later
  * Download the
    [Oracle Java Development Kit](http://www.oracle.com/technetwork/java/javase/downloads)


Out-of-Tree Builds
------------------

Binary objects, libraries, and executables are generated in the directory from
which CMake is executed (the "binary directory"), and this directory need not
necessarily be the same as the TurboVNC source directory.  You can create
multiple independent binary directories, in which different versions of
TurboVNC can be built from the same source tree using different compilers or
settings.  In the sections below, *{build_directory}* refers to the binary
directory, whereas *{source_directory}* refers to the TurboVNC source
directory.  For in-tree builds, these directories are the same.


Build Procedure
---------------


### Un*x (including Mac)

The following procedure will build the Java TurboVNC Viewer and some native
glueware to support running it as a standalone application.  Additionally, if
the TurboVNC Server build is enabled (which is the default on Un*x platforms
other than Mac), then this procedure will build the TurboVNC Server and a
handful of C applications that are used to interface with it.  On most 64-bit
systems (Solaris being a notable exception), this will build a 64-bit version
of TurboVNC.  See "Build Recipes" for specific instructions on how to build a
32-bit or 64-bit version of TurboVNC on systems that support both.

    cd {build_directory}
    cmake -G"Unix Makefiles" [additional CMake flags] {source_directory}
    make


### Visual C++ (Command Line)

    cd {build_directory}
    cmake -G"NMake Makefiles" -DCMAKE_BUILD_TYPE=Release [additional CMake flags] {source_directory}
    nmake

This will build either a 32-bit or a 64-bit version of TurboVNC, depending on
which version of **cl.exe** is in the `PATH`.


### Visual C++ (IDE)

Choose the appropriate CMake generator option for your version of Visual Studio
(run `cmake` with no arguments for a list of available generators.)  For
instance:

    cd {build_directory}
    cmake -G"Visual Studio 10" [additional CMake flags] {source_directory}

NOTE: Add "Win64" to the generator name (for example, "Visual Studio 10 Win64")
to build a 64-bit version of TurboVNC.  A separate build directory must be
used for 32-bit and 64-bit builds.

You can then open **ALL_BUILD.vcproj** in Visual Studio and build one of the
configurations in that project ("Debug", "Release", etc.) to generate a full
build of TurboVNC.


### Java

Adding `-DTVNC_BUILDJAVA=1` to the CMake command line will build the Java
TurboVNC Viewer, as well as include it when installing or packaging the native
TurboVNC Viewer or the TurboVNC Server.  This is the default on Un*x and Mac
platforms.

On Un*x systems, the default is to also build native/X11 infrastructure to
support running the Java TurboVNC Viewer as a standalone application.  You can,
however, disable this by adding `-DTVNC_BUILDNATIVE=0` to the CMake command
line.  This will build only the Java classes without the native infrastructure.
You can add `-DTVNC_BUILDNATIVE=0 -DTVNC_BUILDSERVER=0` to the CMake command
line to avoid building anything but the pure Java code.


### Debug Build

When building the native TurboVNC Viewer or the TurboVNC Server, add
`-DCMAKE_BUILD_TYPE=Debug` to the CMake command line.  Or, if building with
NMake, remove `-DCMAKE_BUILD_TYPE=Release` (Debug builds are the default with
NMake.)


### TLS Encryption Support

The TurboVNC Server provides TLS encryption using either OpenSSL or GnuTLS.
OpenSSL is the default because it generally performs the best, but it may be
desirable to use GnuTLS under certain circumstances.  To do this, add
`-DTVNC_USETLS=GnuTLS` to the CMake command line when configuring the TurboVNC
Server.  Add `-DTVNC_USETLS=0` to the CMake command line to disable TLS
encryption.

The default when using OpenSSL is to dynamically load the OpenSSL symbols from
libssl and libcrypto using dlopen() and dlsym().  This ensures maximum
compatibility across different O/S distributions.  You can disable this
behavior and link directly with libssl and libcrypto by adding
`-DTVNC_DLOPENSSL=0` to the CMake command line.


Build Recipes
-------------


### 32-bit Build on 64-bit Linux/Unix (including Mac)

Use export/setenv to set the following environment variables before running
CMake:

    CFLAGS=-m32
    LDFLAGS=-m32


### 64-bit Build on Solaris

Use export/setenv to set the following environment variables before running
CMake:

    CFLAGS=-m64
    LDFLAGS=-m64


### Other Compilers

On Un*x systems, prior to running CMake, you can set the `CC` environment
variable to the command used to invoke the C compiler.


Advanced CMake Options
----------------------

To list and configure other CMake options not specifically mentioned in this
guide, run

    ccmake {source_directory}

or

    cmake-gui {source_directory}

from the build directory after initially configuring the build.  CCMake is a
text-based interactive version of CMake, and CMake-GUI is a GUI version.  Both
will display all variables that are relevant to the TurboVNC build, their
current values, and a help string describing what they do.


Installing TurboVNC
===================

You can use the build system to install TurboVNC (as opposed to creating an
installer package.)  To do this, run `make install` or `nmake install` (or
build the "install" target in the Visual Studio IDE.)  Running `make uninstall`
or `nmake uninstall` (or building the "uninstall" target in the Visual Studio
IDE) will uninstall TurboVNC.

The `CMAKE_INSTALL_PREFIX` CMake variable can be modified in order to install
TurboVNC into a directory of your choosing.  If you don't specify
`CMAKE_INSTALL_PREFIX`, then the default is:

**c:\Program Files\TurboVNC**<br>
Windows

**c:\Program Files (x86)\TurboVNC**<br>
32-bit build on 64-bit Windows

**/opt/TurboVNC**<br>
Un*x

The default value of `CMAKE_INSTALL_PREFIX` causes the TurboVNC files to be
installed with a directory structure resembling that of the official TurboVNC
binary packages.  Changing the value of `CMAKE_INSTALL_PREFIX` (for instance,
to **/usr/local**) causes the TurboVNC files to be installed with a directory
structure that conforms to GNU standards.

The `CMAKE_INSTALL_BINDIR`, `CMAKE_INSTALL_DATAROOTDIR`,
`CMAKE_INSTALL_DOCDIR`, `CMAKE_INSTALL_JAVADIR`, `CMAKE_INSTALL_MANDIR`, and
`CMAKE_INSTALL_SYSCONFDIR` CMake variables allow a finer degree of control over
where specific files in the TurboVNC distribution should be installed.  These
directory variables can either be specified as absolute paths or as paths
relative to `CMAKE_INSTALL_PREFIX` (for instance, setting
`CMAKE_INSTALL_DOCDIR` to **doc** would cause the documentation to be installed
in **${CMAKE\_INSTALL\_PREFIX}/doc**.)  If a directory variable contains the
name of another directory variable in angle brackets, then its final value will
depend on the final value of that other variable.  For instance, the default
value of `CMAKE_INSTALL_MANDIR` is **\<CMAKE\_INSTALL\_DATAROOTDIR\>/man**.

NOTE: If setting one of these directory variables to a relative path using the
CMake command line, you must specify that the variable is of type `PATH`.
For example:

    cmake -G"{generator type}" -DCMAKE_INSTALL_JAVADIR:PATH=java {source_directory}

Otherwise, CMake will assume that the path is relative to the build directory
rather than the install directory.


Creating Distribution Packages
==============================

The following commands can be used to create various types of distribution
packages:


Linux
-----

    make rpm

Create Red Hat-style binary RPM package.  Requires RPM v4 or later.

    make deb

Create Debian-style binary package.  Requires dpkg.


Mac
---

    make dmg

Create Mac package/disk image.  This requires pkgbuild and productbuild, which
are installed by default on OS X 10.7 and later.  This command generates a
package containing a Java app bundle that relies on Oracle Java.  The DMG built
with this command can be installed on OS X 10.7 and later.


Windows
-------

If using NMake:

    cd {build_directory}
    nmake installer

If using the Visual Studio IDE, build the "installer" target.

The installer package (TurboVNC[64]-{version}.exe) will be located under
*{build_directory}*.  If building using the Visual Studio IDE, then the
installer package will be located in a subdirectory with the same name as the
configuration you built (such as *{build_directory}*\Debug\ or
*{build_directory}*\Release\).

Building a Windows installer requires
[Inno Setup](http://www.jrsoftware.org/isinfo.php).
iscc.exe should be in your `PATH`.
