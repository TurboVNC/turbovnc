bin_PROGRAMS = tvncconfig

tvncconfig_SOURCES = tvncconfig.c vncExt.c

tvncconfig_CFLAGS = @X_CFLAGS@ -I${top_srcdir}/include

tvncconfig_LDADD = @X_LIBS@ -lX11 ../libvncauth/libvncauth.a
