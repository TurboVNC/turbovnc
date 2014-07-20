set(HEADERS
	dpmsconst.h
	dpmsproto.h
	ge.h
	geproto.h
	secur.h
	securproto.h
	shapeconst.h
	shapeproto.h
	shapestr.h
	shm.h
	shmproto.h
	shmstr.h
	syncconst.h
	syncproto.h
	syncstr.h
	xtestconst.h
	xtestext1const.h
	xtestext1proto.h
	xtestproto.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11/extensions)
endforeach()
