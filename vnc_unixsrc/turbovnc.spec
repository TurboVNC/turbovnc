Summary:   A highly-optimized version of VNC that can be used with real-time video applications
Name:      %{_name}
Version:   %{_version}
Release:   %{_build}
URL:       http://www.virtualgl.org
#-->Source0: http://prdownloads.sourceforge.net/virtualgl/%{_name}-%{version}.tar.gz
License:   GPL
Group:     User Interface/Desktops
Requires:  bash >= 2.0
Prereq:    /sbin/chkconfig /etc/init.d
BuildRoot: %{_tmppath}/%{name}-%{version}-root
BuildPrereq: /usr/bin/perl libjpeg-turbo
BuildRequires: zlib-devel

%description
Virtual Network Computing (VNC) is a remote display system which
allows you to view a computing 'desktop' environment not only on the
machine where it is running, but from anywhere on the Internet and
from a wide variety of machine architectures.  TurboVNC is a sleek and
fast VNC distribution, containing a high-performance implementation of
Tight encoding designed to work in conjunction with VirtualGL.

#-->%prep
#-->%setup -q -n vnc/vnc_unixsrc

#-->%build
#-->configure --prefix=%{_prefix} --sysconfdir=/etc --mandir=%{_mandir}
#-->make DESTDIR=%{buildroot}
#-->make xserver DESTDIR=%{buildroot}

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot} sysconfdir=/etc
make xserver-install DESTDIR=%{buildroot} sysconfdir=/etc

strip %{buildroot}%{_prefix}/bin/* || :

mkdir -p %{buildroot}/etc/init.d

cat vncserver.init   | \
sed -e 's@vncserver :${display\%\%:@%{_prefix}/bin/vncserver :${display\%\%:@g' | \
sed -e 's@vncserver -kill :${display\%\%:@%{_prefix}/bin/vncserver -kill :${display\%\%:@g' \
 > %{buildroot}/etc/init.d/tvncserver
chmod 755 %{buildroot}/etc/init.d/tvncserver

mkdir -p %{buildroot}/etc/sysconfig
install -m 644 tvncservers %{buildroot}/etc/sysconfig/tvncservers

mkdir -p %{buildroot}/usr/share/applications
cat > %{buildroot}/usr/share/applications/tvncviewer.desktop << EOF
[Desktop Entry]
Name=TurboVNC Viewer
Comment=TurboVNC client application
Exec=%{_prefix}/bin/vncviewer
Terminal=0
Type=Application
Categories=Application;Utility;X-Red-Hat-Extra;
EOF

chmod 644 %{_srcdir}/LICENCE.TXT %{_srcdir}/TurboVNC-ChangeLog.txt %{_srcdir}/../vnc_docs/LICEN*.txt %{_srcdir}/../vnc_docs/*.html %{_srcdir}/../vnc_docs/*.png %{_srcdir}/../vnc_docs/*.css

%clean
rm -rf %{buildroot}

%post
if [ "$1" = 1 ]; then
  if [ -f /etc/redhat-release ]; then /sbin/chkconfig --add tvncserver; fi
fi

%preun
if [ "$1" = 0 ]; then
  /etc/init.d/tvncserver stop >/dev/null 2>&1
  if [ -f /etc/redhat-release ]; then /sbin/chkconfig --del tvncserver; fi
fi

%postun
if [ "$1" -ge "1" ]; then
  /etc/init.d/tvncserver condrestart >/dev/null 2>&1
fi

%files
%defattr(-,root,root)
%attr(0755,root,root) %config /etc/init.d/tvncserver
%config(noreplace) /etc/sysconfig/tvncservers
%config(noreplace) /etc/turbovncserver.conf
%config(noreplace) /etc/turbovncserver-auth.conf
%doc %{_srcdir}/LICENCE.TXT  %{_srcdir}/TurboVNC-ChangeLog.txt %{_srcdir}/../vnc_docs/LICEN*.txt %{_srcdir}/../vnc_docs/*.html %{_srcdir}/../vnc_docs/*.png %{_srcdir}/../vnc_docs/*.css

%dir %{_prefix}/bin
%dir %{_mandir}
%dir %{_mandir}/man1
%dir %{_prefix}/vnc
%dir %{_prefix}/vnc/classes

%{_prefix}/bin/vncviewer
%config(noreplace) /usr/share/applications/tvncviewer.desktop
%{_mandir}/man1/vncviewer.1*
%{_prefix}/bin/Xvnc
%{_prefix}/bin/vncserver
%{_prefix}/bin/vncpasswd
%{_prefix}/bin/vncconnect
%{_prefix}/bin/autocutsel
%{_prefix}/vnc/classes/index.vnc
%{_prefix}/vnc/classes/VncViewer.jar
%{_mandir}/man1/Xvnc.1*
%{_mandir}/man1/Xserver.1*
%{_mandir}/man1/vncserver.1*
%{_mandir}/man1/vncconnect.1*
%{_mandir}/man1/vncpasswd.1*

%changelog
