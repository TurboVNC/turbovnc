include_directories(../mi ../render ../Xext)

# This code has a lot of type puns.  Disable strict aliasing just to be safe.
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
endif()

add_library(xfixes STATIC
	cursor.c
	region.c
	saveset.c
	select.c
	xfixes.c)
