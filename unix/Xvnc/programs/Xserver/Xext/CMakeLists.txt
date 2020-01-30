include_directories(../Xi ../composite ../damageext ../mi ../miext/sync
	../render ../xfixes)

foreach(dir /run/shm /dev/shm /var/tmp /tmp)
	if(EXISTS ${dir})
		add_definitions(-DSHMDIR=\"${dir}\")
		break()
	endif()
endforeach()

disable_compiler_warnings()

add_library(Xext-server STATIC
	bigreq.c
	dpms.c
	dpmsstubs.c
	geext.c
	hashtable.c
	panoramiX.c
	panoramiXprocs.c
	panoramiXSwap.c
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
