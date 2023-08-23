include_directories(../../render)

if(TVNC_DRI3)
	set(MISYNCSHMSRC misyncshm.c)
endif()

add_library(sync STATIC
	misync.c
	misyncfd.c
	${MISYNCSHMSRC})
