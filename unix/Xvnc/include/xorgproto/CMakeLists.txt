if(TVNC_GLX)

set(HEADERS
	include/GL/glxint.h
	include/GL/glxmd.h
	include/GL/glxproto.h
	include/GL/glxtokens.h)

foreach(h ${HEADERS})
	copy_X_header(${h} GL)
endforeach()

endif()

configure_file(include/X11/Xpoll.h.in Xpoll.h)

set(HEADERS
	include/X11/X.h
	include/X11/XWDFile.h
	include/X11/Xalloca.h
	include/X11/Xarch.h
	include/X11/Xatom.h
	include/X11/Xdefs.h
	include/X11/Xfuncproto.h
	include/X11/Xfuncs.h
	include/X11/Xmd.h
	include/X11/Xos.h
	include/X11/Xos_r.h
	include/X11/Xosdefs.h
	${CMAKE_CURRENT_BINARY_DIR}/Xpoll.h
	include/X11/Xproto.h
	include/X11/Xprotostr.h
	include/X11/Xthreads.h
	include/X11/keysym.h
	include/X11/keysymdef.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11)
endforeach()

set(HEADERS
	include/X11/extensions/bigreqsproto.h
	include/X11/extensions/bigreqstr.h
	include/X11/extensions/composite.h
	include/X11/extensions/compositeproto.h
	include/X11/extensions/damageproto.h
	include/X11/extensions/damagewire.h
	include/X11/extensions/dpmsconst.h
	include/X11/extensions/dpmsproto.h
	include/X11/extensions/ge.h
	include/X11/extensions/geproto.h
	include/X11/extensions/panoramiXproto.h
	include/X11/extensions/presentproto.h
	include/X11/extensions/presenttokens.h
	include/X11/extensions/randr.h
	include/X11/extensions/randrproto.h
	include/X11/extensions/recordconst.h
	include/X11/extensions/recordproto.h
	include/X11/extensions/render.h
	include/X11/extensions/renderproto.h
	include/X11/extensions/saver.h
	include/X11/extensions/saverproto.h
	include/X11/extensions/secur.h
	include/X11/extensions/securproto.h
	include/X11/extensions/shapeconst.h
	include/X11/extensions/shapeproto.h
	include/X11/extensions/shapestr.h
	include/X11/extensions/shm.h
	include/X11/extensions/shmproto.h
	include/X11/extensions/shmstr.h
	include/X11/extensions/syncconst.h
	include/X11/extensions/syncproto.h
	include/X11/extensions/syncstr.h
	include/X11/extensions/xcmiscproto.h
	include/X11/extensions/xcmiscstr.h
	include/X11/extensions/xf86bigfont.h
	include/X11/extensions/xf86bigfproto.h
	include/X11/extensions/xf86bigfstr.h
	include/X11/extensions/xfixesproto.h
	include/X11/extensions/xfixeswire.h
	include/X11/extensions/XI.h
	include/X11/extensions/XI2.h
	include/X11/extensions/XI2proto.h
	include/X11/extensions/XIproto.h
	include/X11/extensions/XKB.h
	include/X11/extensions/XKBgeom.h
	include/X11/extensions/XKBproto.h
	include/X11/extensions/XKBsrv.h
	include/X11/extensions/XKBstr.h
	include/X11/extensions/XResproto.h
	include/X11/extensions/xtestconst.h
	include/X11/extensions/xtestext1const.h
	include/X11/extensions/xtestext1proto.h
	include/X11/extensions/xtestproto.h
	include/X11/extensions/Xv.h
	include/X11/extensions/XvMC.h
	include/X11/extensions/XvMCproto.h
	include/X11/extensions/Xvproto.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11/extensions)
endforeach()

set(HEADERS
	include/X11/fonts/FS.h
	include/X11/fonts/FSproto.h
	include/X11/fonts/font.h
	include/X11/fonts/fontproto.h
	include/X11/fonts/fontstruct.h
	include/X11/fonts/fsmasks.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11/fonts)
endforeach()
