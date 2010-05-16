if WITH_MEDIALIB
SUBDIRS = turbojpeg libvncauth vncconnect vncpasswd vncviewer
else
SUBDIRS = libvncauth vncconnect vncpasswd vncviewer
endif

if X86_64

udmg: all
	sh $(srcdir)/release/makemacpkg ${PACKAGE_NAME} ${VERSION} ${BUILD} \
		${srcdir} universal

endif

dmg: all
	sh $(srcdir)/release/makemacpkg ${PACKAGE_NAME} ${VERSION} ${BUILD} ${srcdir}
