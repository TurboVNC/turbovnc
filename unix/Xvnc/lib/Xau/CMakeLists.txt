copy_X_header(Xauth.h X11)

add_definitions(-DXTHREADS -DXUSE_MTSAFE_API)
if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
	add_definitions(-DXUSE_NETDB_R_API)
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
