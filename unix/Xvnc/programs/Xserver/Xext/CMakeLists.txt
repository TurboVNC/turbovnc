include_directories(../miext/sync ../mi ../Xi)

# This code has a lot of type puns.  Disable strict aliasing just to be safe.
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
endif()

add_library(Xext STATIC
	bigreq.c
	dpms.c
	dpmsstubs.c
	geext.c
	saver.c
	security.c
	shape.c
	shm.c
	sleepuntil.c
	sync.c
	xace.c
	xcmisc.c
	xf86bigfont.c
	xres.c
	xtest.c
	xvdisp.c
	xvmain.c
	xvmc.c)
