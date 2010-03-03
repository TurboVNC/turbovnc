Summary:   An accelerated version of TightVNC designed for video and 3D applications
Name:      turbovnc
Version:   %{_version}
Release:   %{_build}
URL:       http://www.virtualgl.org
#-->Source0: http://prdownloads.sourceforge.net/virtualgl/turbovnc-%{version}.tar.gz
License:   GPL
Group:     User Interface/Desktops
Requires:  bash >= 2.0
Prereq:    /sbin/chkconfig /etc/init.d turbojpeg >= 1.11
BuildRoot: %{_tmppath}/%{name}-%{version}-root
BuildPrereq: /usr/bin/perl turbojpeg
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
#-->xmkmf
#-->make World
#-->cd Xvnc
#-->./configure
#-->make
#-->cd ..


%install
rm -rf %{buildroot}

mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_mandir}/man1
sh ./vncinstall %{buildroot}%{_bindir} %{buildroot}%{_mandir}
install -m 644 Xvnc/programs/Xserver/Xserver.man %{buildroot}%{_mandir}/man1/Xserver.1
mkdir -p %{buildroot}/etc
install -m 644 turbovncserver-auth.conf %{buildroot}/etc/

mkdir -p %{buildroot}%{_datadir}/vnc/classes
for i in classes/*.class; do install -m 644 $i %{buildroot}%{_datadir}/vnc/classes; done
for i in classes/*.jar; do install -m 644 $i %{buildroot}%{_datadir}/vnc/classes; done
for i in classes/*.vnc; do install -m 644 $i %{buildroot}%{_datadir}/vnc/classes; done

strip %{buildroot}%{_bindir}/* || :

mkdir -p %{buildroot}/etc/init.d

cat vncserver.init   | \
sed -e 's@vncserver :${display\%\%:@%{_bindir}/vncserver :${display\%\%:@g' | \
sed -e 's@vncserver -kill :${display\%\%:@%{_bindir}/vncserver -kill :${display\%\%:@g' \
 > %{buildroot}/etc/init.d/tvncserver
chmod 755 %{buildroot}/etc/init.d/tvncserver

mkdir -p %{buildroot}/etc/sysconfig
cat > %{buildroot}/etc/sysconfig/tvncservers << EOF
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
chmod 644 %{buildroot}/etc/sysconfig/tvncservers

mkdir -p %{buildroot}/usr/share/applications
cat > %{buildroot}/usr/share/applications/tvncviewer.desktop << EOF
[Desktop Entry]
Name=TurboVNC Viewer
Comment=TurboVNC client application
Exec=%{_bindir}/vncviewer
Terminal=0
Type=Application
Categories=Application;Utility;X-Red-Hat-Extra;
EOF

chmod 644 LICENCE.TXT WhatsNew ChangeLog TurboVNC-ChangeLog.txt ../vnc_docs/LICEN*.txt ../vnc_docs/*.html ../vnc_docs/*.png ../vnc_docs/*.css

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
%config(noreplace) /etc/turbovncserver-auth.conf
%doc LICENCE.TXT WhatsNew ChangeLog TurboVNC-ChangeLog.txt ../vnc_docs/LICEN*.txt ../vnc_docs/*.html ../vnc_docs/*.png ../vnc_docs/*.css

%dir %{_bindir}
%dir %{_mandir}
%dir %{_mandir}/man1
%dir %{_datadir}
%dir %{_datadir}/vnc

%{_bindir}/vncviewer
%config(noreplace) /usr/share/applications/tvncviewer.desktop
%{_mandir}/man1/vncviewer.1*
%{_bindir}/Xvnc
%{_bindir}/vncserver
%{_bindir}/vncpasswd
%{_bindir}/vncconnect
%{_datadir}/vnc/*
%{_mandir}/man1/Xvnc.1*
%{_mandir}/man1/Xserver.1*
%{_mandir}/man1/vncserver.1*
%{_mandir}/man1/vncconnect.1*
%{_mandir}/man1/vncpasswd.1*

%changelog
