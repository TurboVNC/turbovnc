bin_PROGRAMS = vncpasswd

man1_MANS = vncpasswd.man

vncpasswd_SOURCES = vncpasswd.c

vncpasswd_CFLAGS = @X_CFLAGS@ -I${top_srcdir}/include

vncpasswd_LDADD = @X_LIBS@ -lX11 ../libvncauth/libvncauth.a
