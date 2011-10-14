if WITH_MEDIALIB
SUBDIRS = turbojpeg libvncauth autocutsel vncconnect vncpasswd vncviewer
else
SUBDIRS = libvncauth autocutsel vncconnect vncpasswd vncviewer
endif

mandir = @MANDIR@
datadir = @DATADIR@

noinst_SCRIPTS = vncserver
noinst_MANS = vncserver.man
noinst_HEADERS = Xvnc/config/cf/tvnc_version.h

CLEANFILES = $(noinst_SCRIPTS) $(noinst_HEADERS)

xserver: all Xvnc/config/cf/tvnc_version.h Xvnc-configure
	if [ ! "${top_srcdir}" = "${top_builddir}" ]; then \
		rsync -av ${top_srcdir}/Xvnc ${top_builddir}; \
	fi; \
	cd Xvnc; \
	sh ../Xvnc-configure; \
	$(MAKE) CC="$(CC)" CDEBUGFLAGS="$(CFLAGS)" EXTRA_LDOPTIONS="$(LDFLAGS)" World

xserver-rebuild: all Xvnc/config/cf/tvnc_version.h
	if [ ! "${top_srcdir}" = "${top_builddir}" ]; then \
		rsync -av ${top_srcdir}/Xvnc ${top_builddir}; \
	fi; \
	cd Xvnc; \
	$(MAKE) CC="$(CC)" CDEBUGFLAGS="$(CFLAGS)" EXTRA_LDOPTIONS="$(LDFLAGS)"

xserver-clean: Xvnc/config/cf/tvnc_version.h
	if [ ! "${top_srcdir}" = "${top_builddir}" ]; then \
		rsync -av ${top_srcdir}/Xvnc ${top_builddir}; \
	fi; \
	cd Xvnc; \
	sh ../Xvnc-configure; \
	$(MAKE) CC="$(CC)" CDEBUGFLAGS="$(CFLAGS)" EXTRA_LDOPTIONS="$(LDFLAGS)" distclean

xserver-install: vncserver
	$(INSTALL) -m 755 ./vncserver $(DESTDIR)/$(bindir)/
	$(INSTALL) -m 755 ./Xvnc/programs/Xserver/Xvnc $(DESTDIR)/$(bindir)/
	$(INSTALL) -m 644 $(srcdir)/vncserver.man $(DESTDIR)/$(mandir)/man1/vncserver.1
	$(INSTALL) -m 644 $(srcdir)/Xvnc/programs/Xserver/Xvnc.man $(DESTDIR)/$(mandir)/man1/Xvnc.1
	$(INSTALL) -m 644 $(srcdir)/Xvnc/programs/Xserver/Xserver.man $(DESTDIR)/$(mandir)/man1/Xserver.1
	mkdir -p $(DESTDIR)/$(prefix)/vnc/classes/
	$(INSTALL) -m 644 $(srcdir)/classes/index.vnc $(DESTDIR)/$(prefix)/vnc/classes/
	$(INSTALL) -m 644 $(srcdir)/classes/VncViewer.jar $(DESTDIR)/$(prefix)/vnc/classes/
	$(INSTALL) -m 644 $(srcdir)/classes/README $(DESTDIR)/$(prefix)/vnc/classes/
	mkdir -p $(DESTDIR)/$(sysconfdir)/
	$(INSTALL) -m 644 $(srcdir)/turbovncserver.conf $(DESTDIR)/$(sysconfdir)/
	$(INSTALL) -m 644 $(srcdir)/turbovncserver-auth.conf $(DESTDIR)/$(sysconfdir)/

xserver-uninstall:
	rm -f $(DESTDIR)/$(bindir)/vncserver
	rm -f $(DESTDIR)/$(bindir)/Xvnc
	rm -f $(DESTDIR)/$(mandir)/man1/vncserver.1
	rm -f $(DESTDIR)/$(mandir)/man1/Xvnc.1
	rm -f $(DESTDIR)/$(mandir)/man1/Xserver.1
	rm -f $(DESTDIR)/$(prefix)/vnc/classes/index.vnc
	rm -f $(DESTDIR)/$(prefix)/vnc/classes/VncViewer.jar
	rm -f $(DESTDIR)/$(prefix)/vnc/classes/README
	rmdir $(DESTDIR)/$(prefix)/vnc/classes/
	rmdir $(DESTDIR)/$(prefix)/vnc/
	rm -f $(DESTDIR)/$(sysconfdir)/turbovncserver.conf
	rm -f $(DESTDIR)/$(sysconfdir)/turbovncserver-auth.conf


rpm: all
	TMPDIR=`mktemp -d /tmp/${PACKAGE_NAME}-build.XXXXXX`; \
	mkdir -p $$TMPDIR/RPMS; \
	ln -fs `pwd` $$TMPDIR/BUILD; \
	rm -f ${PACKAGE_NAME}-${VERSION}.${RPMARCH}.rpm; \
	rpmbuild -bb --define "_blddir $$TMPDIR/buildroot"  \
		--define "_srcdir ${srcdir}" --define "_topdir $$TMPDIR" \
		--target ${RPMARCH} pkgscripts/${PACKAGE_NAME}.spec; \
	cp $$TMPDIR/RPMS/${RPMARCH}/${PACKAGE_NAME}-${VERSION}-${BUILD}.${RPMARCH}.rpm ${PACKAGE_NAME}-${VERSION}.${RPMARCH}.rpm; \
	rm -rf $$TMPDIR

srpm:
	TMPDIR=`mktemp -d /tmp/${PACKAGE_NAME}-build.XXXXXX`; \
	mkdir -p $$TMPDIR/RPMS; \
	mkdir -p $$TMPDIR/SRPMS; \
	mkdir -p $$TMPDIR/BUILD; \
	mkdir -p $$TMPDIR/SOURCES; \
	mkdir -p $$TMPDIR/SPECS; \
	rm -f ${PACKAGE_NAME}-${VERSION}.src.rpm; \
	cp vnc.tar.gz $$TMPDIR/SOURCES/${PACKAGE_NAME}-${VERSION}.tar.gz; \
	cat pkgscripts/${PACKAGE_NAME}.spec | sed s/%{_blddir}/%{_tmppath}/g \
		| sed s@%{_srcdir}/@@g | sed s/#--\>//g \
		> $$TMPDIR/SPECS/${PACKAGE_NAME}.spec; \
	rpmbuild -bs --define "_topdir $$TMPDIR" $$TMPDIR/SPECS/${PACKAGE_NAME}.spec; \
	cp $$TMPDIR/SRPMS/${PACKAGE_NAME}-${VERSION}-${BUILD}.src.rpm ${PACKAGE_NAME}-${VERSION}.src.rpm; \
	rm -rf $$TMPDIR

deb: all
	sh pkgscripts/makedpkg

udmg: all
	sh pkgscripts/makemacpkg universal ${BUILDDIR32}

dmg: all
	sh pkgscripts/makemacpkg

solarispkg: all
	sh pkgscripts/makesolarispkg
