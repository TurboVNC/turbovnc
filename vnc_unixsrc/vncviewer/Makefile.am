bin_PROGRAMS = vncviewer

man1_MANS = vncviewer.man

vncviewer_SOURCES = \
argsresources.c \
caps.c \
colour.c \
cursor.c \
desktop.c \
dialogs.c	\
fullscreen.c \
listen.c \
misc.c \
popup.c \
rfbproto.c \
selection.c \
shm.c \
sockets.c \
tunnel.c \
vncviewer.c \
caps.h \
vncviewer.h

EXTRA_DIST = README

vncviewer_CFLAGS = -I${top_srcdir}/include @X_CFLAGS@

vncviewer_LDADD = @X_LIBS@ -lX11 -lXt -lXaw -lXmu @X_EXTRA_LIBS@ \
	$(top_builddir)/libvncauth/libvncauth.a -lz -lpthread

if WITH_MEDIALIB

vncviewer_CFLAGS += -I${top_srcdir}/turbojpeg

vncviewer_LDADD += -lmlib ${top_builddir}/turbojpeg/libturbojpeg.a

else

vncviewer_CFLAGS += ${LJTINC}

vncviewer_LDADD += ${LJTLIB}

endif
