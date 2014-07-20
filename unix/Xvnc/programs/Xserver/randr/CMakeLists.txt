include_directories(../mi ../render)

# This code has a lot of type puns.  Disable strict aliasing just to be safe.
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
endif()

add_library(randr STATIC
	randr.c
	rrcrtc.c
	rrdispatch.c
	rrinfo.c
	rrmode.c
	rroutput.c
	rrpointer.c
	rrproperty.c
	rrscreen.c
	rrsdispatch.c
	rrtransform.c)
