if WITH_MEDIALIB
SUBDIRS = turbojpeg libvncauth vncconnect vncpasswd vncviewer
else
SUBDIRS = libvncauth vncconnect vncpasswd vncviewer
endif

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
JPEGLDADD = -lmlib ${abs_top_srcdir}/turbojpeg/libturbojpeg.a

else

JPEGCFLAGS = ${LJTINC}
JPEGLDADD = ${LJTLIB}

endif

xserver:
	if [ ! "${top_srcdir}" = "${top_builddir}" ]; then \
		echo Out-of-tree builds cannot be used when building Xvnc; \
	else \
		cd Xvnc; \
		$(RM) config/imake/imake config/imake/*.o config/makedepend/makedepend \
			config/makedepend/*.o config/util/lndir config/util/*.o; \
		sh ./configure -jpeginc "$(JPEGCFLAGS)" -jpeglink "$(JPEGLDADD)" \
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
		$(RM) config/imake/imake config/imake/*.o config/makedepend/makedepend \
			config/makedepend/*.o; \
		sh ./configure -jpeginc "$(JPEGCFLAGS)" -jpeglink "$(JPEGLDADD)" \
			$(GCCFLAG) $(CFGFLAGS) CC="$(CC)" CDEBUGFLAGS="$(CFLAGS)" \
			EXTRA_LDOPTIONS="$(LDFLAGS)"; \
		$(MAKE) CC="$(CC)" CDEBUGFLAGS="$(CFLAGS)" EXTRA_LDOPTIONS="$(LDFLAGS)" clean; \
	fi

if X86_64

udmg: all
	sh $(srcdir)/release/makemacpkg ${PACKAGE_NAME} ${VERSION} ${BUILD} \
		${srcdir} universal

endif

dmg: all
	sh $(srcdir)/release/makemacpkg ${PACKAGE_NAME} ${VERSION} ${BUILD} ${srcdir}
