include_directories(../composite ../damageext ../miext/damage ../miext/sync
	../mi ../render ../Xi ../xfixes)

handle_type_puns()

add_library(Xext-server STATIC
	bigreq.c
	dpms.c
	dpmsstubs.c
	geext.c
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
