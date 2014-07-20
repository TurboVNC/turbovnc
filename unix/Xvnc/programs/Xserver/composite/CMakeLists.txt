include_directories(../damageext ../mi ../miext/damage ../render ../xfixes)

# This code has a lot of type puns.  Disable strict aliasing just to be safe.
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
endif()

add_library(composite STATIC
	compalloc.c
	compext.c
	compinit.c
	compoverlay.c
	compwindow.c)
