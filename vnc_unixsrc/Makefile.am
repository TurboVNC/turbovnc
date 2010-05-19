if WITH_MEDIALIB
SUBDIRS = turbojpeg libvncauth vncconnect vncpasswd vncviewer
else
SUBDIRS = libvncauth vncconnect vncpasswd vncviewer
endif

noinst_SCRIPTS = vncserver
CLEANFILES = $(noinst_SCRIPTS)
noinst_MANS = vncserver.man

if X86_64
CFGFLAGS = -x86_64
endif

if X86
CFGFLAGS = -x86
endif

if GCC
GCCFLAG = -gcc
endif

xserver:
	if [ ! "${top_srcdir}" = "${top_builddir}" ]; then \
		echo Out-of-tree builds cannot be used when building Xvnc; \
	else \
		cd Xvnc; \
		rm -f config/imake/imake config/imake/*.o config/makedepend/makedepend \
			config/makedepend/*.o config/util/lndir config/util/*.o; \
		sh ./configure -jpeginc "$(JPEG_CFLAGS)" -jpeglink "$(JPEG_LDFLAGS)" \
			$(GCCFLAG) $(CFGFLAGS) CC="$(CC)" CDEBUGFLAGS="$(CFLAGS)" \
			EXTRA_LDOPTIONS="$(LDFLAGS)"; \
		$(MAKE) CC="$(CC)" CDEBUGFLAGS="$(CFLAGS)" EXTRA_LDOPTIONS="$(LDFLAGS)"; \
	fi

xserver-rebuild:
	if [ ! "${top_srcdir}" = "${top_builddir}" ]; then \
		echo Out-of-tree builds cannot be used when building Xvnc; \
	else \
		cd Xvnc; \
		$(MAKE) CC="$(CC)" CDEBUGFLAGS="$(CFLAGS)" EXTRA_LDOPTIONS="$(LDFLAGS)"; \
	fi

xserver-clean:
	if [ ! "${top_srcdir}" = "${top_builddir}" ]; then \
		echo Out-of-tree builds cannot be used when building Xvnc; \
	else \
		cd Xvnc; \
		rm -f config/imake/imake config/imake/*.o config/makedepend/makedepend \
			config/makedepend/*.o; \
		sh ./configure -jpeginc "$(JPEG_CFLAGS)" -jpeglink "$(JPEG_LDFLAGS)" \
			$(GCCFLAG) $(CFGFLAGS) CC="$(CC)" CDEBUGFLAGS="$(CFLAGS)" \
			EXTRA_LDOPTIONS="$(LDFLAGS)"; \
		$(MAKE) CC="$(CC)" CDEBUGFLAGS="$(CFLAGS)" EXTRA_LDOPTIONS="$(LDFLAGS)" clean; \
	fi

xserver-install:
	$(INSTALL) -m 755 $(builddir)/vncserver $(DESTDIR)/$(bindir)/
	$(INSTALL) -m 755 $(builddir)/Xvnc/programs/Xserver/Xvnc $(DESTDIR)/$(bindir)/
	$(INSTALL) -m 755 $(srcdir)/vncserver.man $(DESTDIR)/$(mandir)/man1/vncserver.1
	$(INSTALL) -m 755 $(srcdir)/Xvnc/programs/Xserver/Xvnc.man $(DESTDIR)/$(mandir)/man1/Xvnc.1
	$(INSTALL) -m 755 $(srcdir)/Xvnc/programs/Xserver/Xserver.man $(DESTDIR)/$(mandir)/man1/Xserver.1
	mkdir -p $(DESTDIR)/$(prefix)/vnc/classes/
	$(INSTALL) -m 755 $(srcdir)/classes/VncViewer.jar $(DESTDIR)/$(prefix)/vnc/classes/


rpm: all
	sh $(srcdir)/makerpm ${PACKAGE_NAME} ${VERSION} ${BUILD} \
		${RPMARCH} ${srcdir}

deb: all
	sh $(srcdir)/release/makedpkg ${PACKAGE_NAME} ${VERSION} ${BUILD} \
		${PKGARCH} ${srcdir}


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
