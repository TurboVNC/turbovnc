Summary:   A remote display system.
Name:      tightvnc
Version:   1.2.9
Release:   1
URL:       http://www.tightvnc.com/
Source0:   tightvnc-%{version}_unixsrc.tar.bz2
License:   GPL
Group:     User Interface/Desktops
Packager:  Constantin Kaplinsky <const@ce.cctpu.edu.ru>
Obsoletes: vnc
BuildRoot: %{_tmppath}/%{name}-%{version}-root
BuildPrereq: /usr/bin/perl tcp_wrappers
BuildRequires: zlib-devel libjpeg-devel
ExclusiveArch: i386 alpha sparc ppc s390 s390x

%description
Virtual Network Computing (VNC) is a remote display system which
allows you to view a computing 'desktop' environment not only on the
machine where it is running, but from anywhere on the Internet and
from a wide variety of machine architectures. TightVNC is an enhanced
VNC distribution. This package contains a client which will allow you
to connect to other desktops running a VNC or a TightVNC server.

%package server
Summary: TightVNC server
Obsoletes: vnc-server
Requires: XFree86 bash >= 2.0
Group: User Interface/X
Prereq: /sbin/chkconfig /sbin/service /etc/init.d

%description server
The VNC system allows you to access the same desktop from a wide
variety of platforms. TightVNC is an enhanced VNC distribution. This
package is a TightVNC server, allowing others to access the desktop on
your machine.

%prep
%setup -q -n vnc_unixsrc

perl -pi -e "s|/usr/local/vnc/classes|%{_datadir}/vnc/classes|" vncserver

%build
# Use xinit's Xclients script to start the session (bug #52711)
patch < vnc-xclients.patch

xmkmf -a
make CDEBUGFLAGS="$RPM_OPT_FLAGS" World
cd Xvnc
./configure
make EXTRA_LIBRARIES="-lwrap -lnss_nis" CDEBUGFLAGS="$RPM_OPT_FLAGS" \
     EXTRA_DEFINES="-DUSE_LIBWRAP=1"

%install
rm -rf %{buildroot}

mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_mandir}/man1
./vncinstall %{buildroot}%{_bindir} %{buildroot}%{_mandir}

mkdir -p %{buildroot}%{_datadir}/vnc
cp -aR classes %{buildroot}%{_datadir}/vnc

strip %{buildroot}%{_bindir}/* || :

mkdir -p %{buildroot}/etc/rc.d/init.d
install -m755 vncserver.init %{buildroot}/etc/rc.d/init.d/vncserver

mkdir -p %{buildroot}/etc/sysconfig
cat > %{buildroot}/etc/sysconfig/vncservers << EOF
# The VNCSERVERS variable is a list of display:user pairs.
#
# Uncomment the line below to start a VNC server on display :1
# as my 'myusername' (adjust this to your own).  You will also
# need to set a VNC password; run 'man vncpasswd' to see how
# to do that.  
#
# DO NOT RUN THIS SERVICE if your local area network is
# untrusted!  For a secure way of using VNC, see
# <URL:http://www.uk.research.att.com/vnc/sshvnc.html>.

# VNCSERVERS="1:myusername"
EOF
chmod 644 %{buildroot}/etc/sysconfig/vncservers

mkdir -p %{buildroot}/etc/X11/applnk/Applications
cat > %{buildroot}/etc/X11/applnk/Applications/vncviewer.desktop << EOF
[Desktop Entry]
Name=VNC Viewer
Comment=VNC client application
Exec=/usr/bin/vncviewer
Terminal=0
Type=Application
EOF

%clean
rm -rf %{buildroot}

%post server
if [ "$1" = 1 ]; then
  /sbin/chkconfig --add vncserver
fi

%preun server
if [ "$1" = 0 ]; then
  /sbin/service vncserver stop >/dev/null 2>&1
  /sbin/chkconfig --del vncserver
fi

%postun server
if [ "$1" -ge "1" ]; then
  /sbin/service vncserver condrestart >/dev/null 2>&1
fi

%files
%defattr(-,root,root)
%doc LICENCE.TXT README WhatsNew ChangeLog
%{_bindir}/vncviewer
%config(noreplace) /etc/X11/applnk/Applications/vncviewer.desktop
%{_mandir}/man1/vncviewer.1*

%files server
%defattr(-,root,root)
%doc LICENCE.TXT README WhatsNew ChangeLog
%attr(0755,root,root) %config /etc/rc.d/init.d/vncserver
%config(noreplace) /etc/sysconfig/vncservers
%{_bindir}/Xvnc
%{_bindir}/vncserver
%{_bindir}/vncpasswd
%{_bindir}/vncconnect
%{_datadir}/vnc
%{_mandir}/man1/Xvnc.1*
%{_mandir}/man1/vncserver.1*
%{_mandir}/man1/vncconnect.1*
%{_mandir}/man1/vncpasswd.1*

%changelog
* Thu Jul 31 2003 Constantin Kaplinsky <const@ce.cctpu.edu.ru>
- TightVNC 1.2.9 update.

* Wed Jan 29 2003 Constantin Kaplinsky <const@ce.cctpu.edu.ru>
- TightVNC 1.2.8 update.
- Dependencies on /sbin/chkconfig and /sbin/service has been removed
  for the viewer part.

* Thu Nov 14 2002 Constantin Kaplinsky <const@ce.cctpu.edu.ru>
- TightVNC 1.2.7 update.

* Sat Aug 10 2002 Constantin Kaplinsky <const@ce.cctpu.edu.ru>
- TightVNC 1.2.5 update.

* Tue May 21 2002 Constantin Kaplinsky <const@ce.cctpu.edu.ru>
- TightVNC 1.2.4 update.

* Fri Mar 22 2002 Constantin Kaplinsky <const@ce.cctpu.edu.ru>
- TightVNC 1.2.3 update.

* Tue Mar 12 2002 Tim Waugh <twaugh@redhat.com> 3.3.3r2-27
- Don't block on partial HTTP requests (bug #58066).
- Use the system-provided zlib instead of the bundled one in Xvnc.
- Link to libz and libjpeg dynamically instead of statically in
  vncviewer.
- Fix docs permissions (bug #60783).

* Tue Jan 29 2002 Tim Waugh <twaugh@redhat.com> 3.3.3r2-26
- Updated vncserver fp patch (see bug #58990):
  - Check that Xvnc actually started.
  - Try omitting font path specification if not.

* Wed Jan 09 2002 Tim Powers <timp@redhat.com> 3.3.3r2-25
- automated rebuild

* Wed Dec 05 2001 David Sainty <dsainty@redhat.com> 3.3.3r2-24
- Added support for s390 and s390x architectures.

* Thu Nov 29 2001 Tim Waugh <twaugh@redhat.com> 3.3.3r2-23
- TightVNC 1.2.2.

* Mon Nov  5 2001 Tim Waugh <twaugh@redhat.com> 3.3.3r2-22
- Don't use bash redirections except in bash (bug #55686).

* Fri Oct 26 2001 Tim Waugh <twaugh@redhat.com> 3.3.3r2-21
- Run /etc/X11/xinit/xinitrc in xstartup, not Xclients (bug #52711).
- Don't segfault when disconnecting from font server (bug #55135).

* Wed Sep 26 2001 Tim Waugh <twaugh@redhat.com> 3.3.3r2-20
- TightVNC 1.2.1, which incorporates several patches we had.

* Mon Sep  3 2001 Tim Waugh <twaugh@redhat.com> 3.3.3r2-19
- Use xinit's Xclients script to start the session (bug #52711).
- Make vncpasswd create ~/.vnc if it doesn't exist (bug #52547).
- Mention the eight character password limit in vncpasswd.1.

* Tue Jul 24 2001 Tim Waugh <twaugh@redhat.com> 3.3.3r2-18
- Build requires zlib-devel, libjpeg-devel (bug #49731).

* Fri Jul 13 2001 Tim Waugh <twaugh@redhat.com> 3.3.3r2-17
- Rebuild to fix file ownership problems.

* Tue Jun  5 2001 Tim Waugh <twaugh@redhat.com> 3.3.3r2-16
- Really fix bug #28318.
- Use tcp_wrappers (bug #41052).
- Fix build error when using XFree86 4.1.0's imakefile config.

* Fri May  4 2001 Tim Waugh <twaugh@redhat.com> 3.3.3r2-15
- Add tight encoding patch (includes corre and typo patches).
- Const's Java viewer with tight encoding.
- Update man pages.
- Use the font server (bug #35913).

* Mon Feb 19 2001 Tim Waugh <twaugh@redhat.com> 3.3.3r2-14
- Make initscript change to user's home directory before starting
  vncserver (bug #28318).
- Make initscript stop properly.

* Thu Feb  8 2001 Tim Waugh <twaugh@redhat.com>
- Change initscript usage string.
- Resync descriptions and summaries to match those from specspo.

* Tue Jan 23 2001 Tim Waugh <twaugh@redhat.com>
- Redo initscript internationalisation.
- Initscript uses bash not sh.

* Fri Jan 19 2001 Tim Waugh <twaugh@redhat.com>
- Xvnc man page: put a pointer to tunnelling through ssh.

* Thu Jan 18 2001 Tim Waugh <twaugh@redhat.com>
- Warn about VNC being insecure in /etc/sysconfig/vncservers.
- CoRRE fix from Const Kaplinsky (bug #24285).

* Mon Jan 15 2001 Tim Waugh <twaugh@redhat.com>
- Initscript internationalisation.

* Tue Dec 12 2000 Tim Waugh <twaugh@redhat.com>
- Service is off by default (bug #22076).
- Don't start VNC server in initscript if user has not set passwd yet
  (bug #22076).

* Thu Dec 07 2000 Tim Waugh <twaugh@redhat.com>
- Mark vncviewer.desktop noreplace.
- Mark vncservers and /etc/rc.d/init.d/vncserver config.
- Add reload to vncserver initscript.
- chkconfig in post.

* Fri Dec 01 2000 Tim Waugh <twaugh@redhat.com>
- Rebuilt because of fileutils bug.

* Mon Nov 06 2000 Tim Waugh <twaugh@redhat.com>
- Include vncpasswd man page in manifest.

* Mon Nov 06 2000 Tim Waugh <twaugh@redhat.com>
- Add vncviewer.desktop.
- Correct typo in vncconnect.c.
- Add man pages.

* Mon Oct 30 2000 Tim Waugh <twaugh@redhat.com>
- set XAUTHORITY before running xstartup.  Fixes vncserver in ssh
  sessions.
- revamp initscript so that it allows for non-root users.

* Thu Oct 26 2000 Tim Waugh <twaugh@redhat.com>
- update to 3.3.3r2.  Patch from bug #19146 no longer needed.
- include new vncconnect program in file list

* Thu Oct 26 2000 Tim Waugh <twaugh@redhat.com>
- fix initscript (bug #19698)

* Tue Oct 17 2000 Than Ngo <than@redhat.com>
- fixed VNC crashes, patch from Shinji Hattori (Bug #10528)

* Mon Oct 16 2000 Bernhard Rosenkraenzer <bero@redhat.com>
- Fix authentication, Bug #19146, Patch from Tim Waugh <twaugh@redhat.com>

* Fri Aug  4 2000 Bernhard Rosenkraenzer <bero@redhat.com>
- Remove ,v file from vnc-doc, Bug #12443

* Thu Jul 20 2000 Bill Nottingham <notting@redhat.com>
- move initscript back

* Thu Jul 13 2000 Prospector <bugzilla@redhat.com>
- automatic rebuild

* Mon Jul 10 2000 Preston Brown <pbrown@redhat.com>
- move initscript

* Thu Jun 15 2000 Preston Brown <pbrown@redhat.com>
- move post/postun scripts from main package to server subpackage

* Mon Jun 12 2000 Preston Brown <pbrown@redhat.com>
- preun and postun sections

* Fri Jun 09 2000 Preston Brown <pbrown@redhat.com>
- initial package, from Sean P. Kane package and Mandrake package
- rewrote init script, added condrestart mode
