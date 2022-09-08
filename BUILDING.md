Building TurboVNC
=================


Build Requirements
------------------


### All Systems

- [CMake](http://www.cmake.org) v2.8.12 or later
  * CMake 3.12 or later is required if it is desirable or necessary to use
    Python 3 to run the simple web server for noVNC (part of the TurboVNC
    Server.)

- libjpeg-turbo SDK v1.2 or later
  * The libjpeg-turbo SDK binary packages can be downloaded from the "Files"
    area of <http://sourceforge.net/projects/libjpeg-turbo>.
  * The TurboVNC build system will search for the TurboJPEG header and library
    under __/opt/libjpeg-turbo__ on Un\*x or __c:\libjpeg-turbo[64]__ (Visual
    C++) or __c:\libjpeg-turbo-gcc[64]__ (MinGW) on Windows, but you can
    override this by setting the `TJPEG_INCLUDE_DIR` CMake variable to the
    directory containing __turbojpeg.h__ and the `TJPEG_LIBRARY` CMake variable
    to either the full path of the TurboJPEG library against which you want to
    link or a set of link flags needed to link with the TurboJPEG library (for
    instance, `-DTJPEG_LIBRARY="-L/opt/libjpeg-turbo/lib64 -lturbojpeg"` or
    `-DTJPEG_LIBRARY="-libpath:c:/libjpeg-turbo64/lib turbojpeg.lib"`.)


### Linux and other Un*x O/S's (except Mac)

- GCC (or a GCC-compatible compiler)

- X11 development kit

- PAM development kit [if building the TurboVNC Server]

- JDK 8 or OpenJDK 1.7 or later [if building the TurboVNC Viewer]
  * For systems that do not provide a JDK, download the
    [Oracle Java Development Kit](http://www.oracle.com/technetwork/java/javase/downloads)
    or [OpenJDK](https://jdk.java.net)
  * [OpenJDK](https://jdk.java.net) 11 or later must be used if building
    an installer with a custom JRE (if the `TVNC_INCLUDEJRE` CMake variable is
    set to `1`)
  * If using JDK 11 or later, CMake 3.11.x or later must also be used


### Windows

- Microsoft Visual C++ 2005 or later

  If you don't already have Visual C++, then the easiest way to get it is by
  installing
  [Visual Studio Community Edition](https://visualstudio.microsoft.com),
  which includes everything necessary to build TurboVNC.

  * You can also download and install the standalone Windows SDK (for Windows 7
    or later), which includes command-line versions of the 32-bit and 64-bit
    Visual C++ compilers.
  * If you intend to build TurboVNC from the command line, then add the
    appropriate compiler and SDK directories to the `INCLUDE`, `LIB`, and
    `PATH` environment variables.  This is generally accomplished by executing
    `vcvars32.bat` or `vcvars64.bat`, which are located in the same directory
    as the compiler.

   ... OR ...

- MinGW

  [MSYS2](http://msys2.github.io/) or [tdm-gcc](http://tdm-gcc.tdragon.net/)
  recommended if building on a Windows machine.  Both distributions install a
  Start Menu link that can be used to launch a command prompt with the
  appropriate compiler paths automatically set.

- JDK 8 or OpenJDK 1.8 or later
  * Download the
    [Oracle Java Development Kit](http://www.oracle.com/technetwork/java/javase/downloads)
    or [OpenJDK](https://jdk.java.net)
  * [OpenJDK](https://jdk.java.net) 11 or later must be used if building
    an installer with a custom JRE (if the `TVNC_INCLUDEJRE` CMake variable is
    set to `1`)
  * If using JDK 11 or later, CMake 3.11.x or later must also be used


### Mac

- Xcode 4.1 or later (OS X/macOS 10.7.x or later SDK required)

- JDK 8 or OpenJDK 8 or later
  * Download the
    [Oracle Java Development Kit](http://www.oracle.com/technetwork/java/javase/downloads)
    or [OpenJDK](https://jdk.java.net)
  * [OpenJDK](https://jdk.java.net) 11 or later must be used if building
    a Mac package/disk image with a custom JRE (if the `TVNC_INCLUDEJRE` CMake
    variable is set to `1`)
  * If using JDK 11 or later, CMake 3.11.x or later must also be used


Out-of-Tree Builds
------------------

Binary objects, libraries, and executables are generated in the directory from
which CMake is executed (the "binary directory"), and this directory need not
necessarily be the same as the TurboVNC source directory.  You can create
multiple independent binary directories, in which different versions of
TurboVNC can be built from the same source tree using different compilers or
settings.  In the sections below, __{build_directory}__ refers to the binary
directory, whereas __{source_directory}__ refers to the TurboVNC source
directory.  For in-tree builds, these directories are the same.


Build Procedure
---------------


### Un*x (including Mac)

The following procedure will build the TurboVNC Viewer and some native glueware
to support running it as a standalone application.  Additionally, if the
TurboVNC Server build is enabled (which is the default on Un*x platforms other
than Mac), then this procedure will build the TurboVNC Server and a handful of
C applications that are used to interface with it.  On most 64-bit systems
(Solaris being a notable exception), this will build a 64-bit version of
TurboVNC.  (See "Build Recipes" for specific instructions on how to build a
64-bit version of TurboVNC on Solaris.)

    cd {build_directory}
    cmake -G"Unix Makefiles" [additional CMake flags] {source_directory}
    make

Replace `make` with `ninja` and `Unix Makefiles` with `Ninja` if using Ninja.


### Visual C++ (Command Line)

    cd {build_directory}
    cmake -G"NMake Makefiles" -DCMAKE_BUILD_TYPE=Release [additional CMake flags] {source_directory}
    nmake

This will build either a 32-bit or a 64-bit version of TurboVNC, depending on
which version of __cl.exe__ is in the `PATH`.

Replace `nmake` with `ninja` and `NMake Makefiles` with `Ninja` if using Ninja.


### Visual C++ (IDE)

Choose the appropriate CMake generator option for your version of Visual Studio
(run `cmake` with no arguments for a list of available generators.)  For
instance:

    cd {build_directory}
    cmake -G"Visual Studio 10" [additional CMake flags] {source_directory}

NOTE: Add `Win64` to the generator name (for example, `Visual Studio 10 Win64`)
to build a 64-bit version of TurboVNC.  A separate build directory must be
used for 32-bit and 64-bit builds.

You can then open __ALL_BUILD.vcproj__ in Visual Studio and build one of the
configurations in that project ("Debug", "Release", etc.) to generate a full
build of TurboVNC.


### Debug Build

When building the native TurboVNC Viewer or the TurboVNC Server, add
`-DCMAKE_BUILD_TYPE=Debug` to the CMake command line.  Or, if building with
NMake, remove `-DCMAKE_BUILD_TYPE=Release` (Debug builds are the default with
NMake.)


### TLS Encryption Support

The TurboVNC Server provides TLS encryption using OpenSSL.  Add
`-DTVNC_USETLS=0` to the CMake command line to disable TLS encryption.

The default is to dynamically load the OpenSSL symbols from libssl and
libcrypto using `dlopen()` and `dlsym()`.  This ensures maximum compatibility
across different O/S distributions.  You can disable this behavior and link
directly with libssl and libcrypto by adding `-DTVNC_DLOPENSSL=0` to the CMake
command line.


### Distribution-Specific Build

By default, the build system builds TurboVNC binaries that can run on multiple
O/S distributions.  This involves building some of the X.org dependencies,
which are included in the TurboVNC source tree, and statically linking TurboVNC
with those and other dependencies.  Distribution-specific dynamically-linked
TurboVNC binaries can instead be built by changing the values of the following
CMake variables:

- `TJPEG_INCLUDE_DIR`
- `TJPEG_LIBRARY`
- `TVNC_DLOPENSSL`
- `TVNC_STATIC_XORG_PATHS`
- `TVNC_SYSTEMLIBS`
- `TVNC_SYSTEMX11`
- `XKB_BASE_DIRECTORY`
- `XKB_BIN_DIRECTORY`
- `XORG_DRI_DRIVER_PATH`
- `XORG_FONT_PATH`
- `XORG_REGISTRY_PATH`

Use `ccmake` or `cmake-gui`, as described below, to view documentation for
those variables.


Build Recipes
-------------


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
installer package.)  To do this, run `make install` or `ninja install` or
`nmake install` (or build the "install" target in the Visual Studio IDE.)
Running `make uninstall` or `ninja uninstall` or `nmake uninstall` (or building
the "uninstall" target in the Visual Studio IDE) will uninstall TurboVNC.

The `CMAKE_INSTALL_PREFIX` CMake variable can be modified in order to install
TurboVNC into a directory of your choosing.  If you don't specify
`CMAKE_INSTALL_PREFIX`, then the default is:

__c:\Program Files\TurboVNC__<br>
Windows

__c:\Program Files (x86)\TurboVNC__<br>
32-bit build on 64-bit Windows

__/opt/TurboVNC__<br>
Un*x

The default value of `CMAKE_INSTALL_PREFIX` causes the TurboVNC files to be
installed with a directory structure resembling that of the official TurboVNC
binary packages.  Changing the value of `CMAKE_INSTALL_PREFIX` (for instance,
to `/usr/local`) causes the TurboVNC files to be installed with a directory
structure that conforms to GNU standards.

The `CMAKE_INSTALL_BINDIR`, `CMAKE_INSTALL_DATAROOTDIR`,
`CMAKE_INSTALL_DOCDIR`, `CMAKE_INSTALL_JAVADIR`, `CMAKE_INSTALL_MANDIR`, and
`CMAKE_INSTALL_SYSCONFDIR` CMake variables allow a finer degree of control over
where specific files in the TurboVNC distribution should be installed.  These
directory variables can either be specified as absolute paths or as paths
relative to `CMAKE_INSTALL_PREFIX` (for instance, setting
`CMAKE_INSTALL_DOCDIR` to `doc` would cause the documentation to be installed
in __${CMAKE\_INSTALL\_PREFIX}/doc__.)  If a directory variable contains the
name of another directory variable in angle brackets, then its final value will
depend on the final value of that other variable.  For instance, the default
value of `CMAKE_INSTALL_MANDIR` is `<CMAKE_INSTALL_DATAROOTDIR>/man`.

NOTE: If setting one of these directory variables to a relative path using the
CMake command line, you must specify that the variable is of type `PATH`.
For example:

    cmake -G"{generator type}" -DCMAKE_INSTALL_JAVADIR:PATH=java {source_directory}

Otherwise, CMake will assume that the path is relative to the build directory
rather than the install directory.


Creating Distribution Packages
==============================

The following commands can be used to create various types of distribution
packages (replace `make` with `ninja` if using Ninja):


Linux
-----

    make rpm

Create Red Hat-style binary RPM package.  Requires RPM v4 or later.

    make deb

Create Debian-style binary package.  Requires dpkg.

A custom JRE based on OpenJDK can be included in the Linux installer packages
by setting the `TVNC_INCLUDEJRE` CMake variable to `1`.  If this variable is
not set, then the TurboVNC Viewer will rely on a separate installation of
Oracle Java or OpenJDK.  OpenJDK must be used when including a custom JRE, in
order to avoid legal restrictions regarding the redistribution of Oracle JDK
components.


Mac
---

    make dmg

Create Mac package/disk image.  This requires __pkgbuild__ and
__productbuild__, which are installed by default on OS X/macOS 10.7 and later.
This command generates a package containing a TurboVNC Viewer app bundle that,
depending on the value of the `TVNC_INCLUDEJRE` CMake variable, includes a
custom JRE based on OpenJDK or relies on a separate installation of Oracle Java
or OpenJDK.  The DMG built with this command can be installed on OS X/macOS
10.7 and later, but if a custom JRE is included, it requires OS X/macOS 10.9 or
later in order to run.  OpenJDK must be used when including a custom JRE, in
order to avoid legal restrictions regarding the redistribution of Oracle JDK
components.


Windows
-------

If using NMake:

    cd {build_directory}
    nmake installer

If using Ninja:

    cd {build_directory}
    ninja installer

If using the Visual Studio IDE, build the "installer" target.

The installer package (__TurboVNC[64]-{version}.exe__) will be located under
__{build_directory}__.  If building using the Visual Studio IDE, then the
installer package will be located in a subdirectory with the same name as the
configuration you built (such as __{build_directory}\Debug__ or
__{build_directory}\Release__).

Building a Windows installer requires
[Inno Setup](http://www.jrsoftware.org/isinfo.php).
__iscc.exe__ should be in your `PATH`.

A custom JRE based on OpenJDK can be included in the Windows installer package
by setting the `TVNC_INCLUDEJRE` CMake variable to `1`.  If this variable is
not set, then the TurboVNC Viewer will rely on a separate installation of
Oracle Java or OpenJDK.  OpenJDK must be used when including a custom JRE, in
order to avoid legal restrictions regarding the redistribution of Oracle JDK
components.
