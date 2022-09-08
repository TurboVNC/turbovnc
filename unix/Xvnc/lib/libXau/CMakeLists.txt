copy_X_header(include/X11/Xauth.h X11)

add_definitions(-DXTHREADS -DXUSE_MTSAFE_API)

check_symbol_exists(pathconf unistd.h HAVE_PATHCONF)
if(HAVE_PATHCONF)
	add_definitions(-DHAVE_PATHCONF)
endif()

add_library(Xau STATIC
	AuDispose.c
	AuFileName.c
	AuGetAddr.c
	AuGetBest.c
	AuLock.c
	AuRead.c
	AuUnlock.c
	AuWrite.c)
