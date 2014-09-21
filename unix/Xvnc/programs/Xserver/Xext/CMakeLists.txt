# This code has some type puns.  Disable strict aliasing just to be safe.
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
endif()

add_library(Xext STATIC
	shape.c
	shm.c
	mbuf.c
	mitmisc.c
	xtest.c
	xtest1di.c
	xtest1dd.c
	sleepuntil.c
	bigreq.c
	sync.c
	saver.c
	xcmisc.c
	xprint.c)
