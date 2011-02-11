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

if X86_64
CFGFLAGS = -x86_64
endif

if X86
CFGFLAGS = -x86
endif

if GCC
GCCFLAG = -gcc
endif

if WITH_MEDIALIB

JPEGCFLAGS = -I${abs_top_srcdir}/turbojpeg 	 
JPEGLDFLAGS = -lmlib ${abs_top_srcdir}/turbojpeg/libturbojpeg.a

else

JPEGCFLAGS = ${JPEG_CFLAGS}
JPEGLDFLAGS = ${JPEG_LDFLAGS}

endif

xserver: all
	if [ ! "${top_srcdir}" = "${top_builddir}" ]; then \
		echo Out-of-tree builds cannot be used when building Xvnc; \
	else \
		cd Xvnc; \
		rm -f config/imake/imake config/imake/*.o config/makedepend/makedepend \
			config/makedepend/*.o config/util/lndir config/util/*.o; \
		sh ./configure -jpeginc "$(JPEGCFLAGS)" -jpeglink "$(JPEGLDFLAGS)" \
			$(GCCFLAG) $(CFGFLAGS) CC="$(CC)" CDEBUGFLAGS="$(CFLAGS)" \
			EXTRA_LDOPTIONS="$(LDFLAGS)"; \
		$(MAKE) CC="$(CC)" CDEBUGFLAGS="$(CFLAGS)" EXTRA_LDOPTIONS="$(LDFLAGS)"; \
	fi

xserver-rebuild: all
	if [ ! "${top_srcdir}" = "${top_builddir}" ]; then \
		echo Out-of-tree builds cannot be used when building Xvnc; \
	else \
		cd Xvnc; \
		$(MAKE) CC="$(CC)" CDEBUGFLAGS="$(CFLAGS)" EXTRA_LDOPTIONS="$(LDFLAGS)"; \
	fi

xserver-clean: Xvnc/config/cf/tvnc_version.h
	if [ ! "${top_srcdir}" = "${top_builddir}" ]; then \
		echo Out-of-tree builds cannot be used when building Xvnc; \
	else \
		cd Xvnc; \
		rm -f config/imake/imake config/imake/*.o config/makedepend/makedepend \
			config/makedepend/*.o; \
		sh ./configure -jpeginc "$(JPEGCFLAGS)" -jpeglink "$(JPEGLDFLAGS)" \
			$(GCCFLAG) $(CFGFLAGS) CC="$(CC)" CDEBUGFLAGS="$(CFLAGS)" \
			EXTRA_LDOPTIONS="$(LDFLAGS)"; \
		$(MAKE) CC="$(CC)" CDEBUGFLAGS="$(CFLAGS)" EXTRA_LDOPTIONS="$(LDFLAGS)" clean; \
	fi

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
	rm -f ${PACKAGE_NAME}.${RPMARCH}.rpm; \
	rpmbuild -bb --define "_name ${PACKAGE_NAME}" \
		--define "_blddir $TMPDIR/buildroot" --define "_topdir $$TMPDIR" \
		--define "_bindir ${bindir}" --define "_mandir ${mandir}" \
		--define "_prefix ${prefix}" --define "_version ${VERSION}" \
		--define "_build ${BUILD}" --define "_srcdir ${srcdir}" \
		--target ${RPMARCH} ${srcdir}/${PACKAGE_NAME}.spec; \
	cp $$TMPDIR/RPMS/${RPMARCH}/${PACKAGE_NAME}-${VERSION}-${BUILD}.${RPMARCH}.rpm ${PACKAGE_NAME}.${RPMARCH}.rpm; \
	rm -rf $$TMPDIR

srpm:
	TMPDIR=`mktemp -d /tmp/${PACKAGE_NAME}-build.XXXXXX`; \
	mkdir -p $$TMPDIR/RPMS; \
	mkdir -p $$TMPDIR/SRPMS; \
	mkdir -p $$TMPDIR/BUILD; \
	mkdir -p $$TMPDIR/SOURCES; \
	mkdir -p $$TMPDIR/SPECS; \
	rm -f ${PACKAGE_NAME}.src.rpm; \
	cp vnc.tar.gz $$TMPDIR/SOURCES/${PACKAGE_NAME}-${VERSION}.tar.gz; \
	cat ${srcdir}/${PACKAGE_NAME}.spec | sed s@%{_name}@${PACKAGE_NAME}@g \
		| sed s@%{_version}@${VERSION}@g | sed s@%{_build}@${BUILD}@g \
		| sed s@%{_bindir}@${bindir}@g | sed s@%{_mandir}@${mandir}@g \
		| sed s@%{_prefix}@${prefix}@g | sed s@%{_srcdir}@${srcdir}@g \
		|sed s/#--\>//g > $$TMPDIR/SPECS/${PACKAGE_NAME}.spec; \
	rpmbuild -bs --define "_topdir $$TMPDIR" $$TMPDIR/SPECS/${PACKAGE_NAME}.spec; \
	cp $$TMPDIR/SRPMS/${PACKAGE_NAME}-${VERSION}-${BUILD}.src.rpm ${PACKAGE_NAME}.src.rpm; \
	rm -rf $$TMPDIR

deb: all
	sh $(srcdir)/makedpkg ${PACKAGE_NAME} ${VERSION} ${BUILD} ${PKGARCH} \
		${srcdir} ${prefix}


if X86_64

udmg: all
	sh $(srcdir)/makemacpkg ${VERSION} ${BUILD} ${srcdir} universal

endif

dmg: all
	sh $(srcdir)/makemacpkg ${VERSION} ${BUILD} ${srcdir}


if WITH_MEDIALIB

sunpkg: all
	sh $(srcdir)/makepkg $(VERSION) $(BUILD) $(PKGARCH) $(srcdir) medialib

else

sunpkg: all
	sh $(srcdir)/makepkg $(VERSION) $(BUILD) $(PKGARCH) $(srcdir)

endif
