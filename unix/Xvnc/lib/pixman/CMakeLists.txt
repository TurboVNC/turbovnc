include_directories(.)

add_definitions(-DHAVE_CONFIG_H -DPIXMAN_NO_TLS)

# This code has a lot of type puns.  Disable strict aliasing just to be safe.
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
endif()

add_library(pixman STATIC
	pixman-access-accessors.c
	pixman-access.c
	pixman-arm.c
	pixman-bits-image.c
	pixman-combine-float.c
	pixman-combine32.c
	pixman-conical-gradient.c
	pixman-edge.c
	pixman-edge-accessors.c
	pixman-fast-path.c
	pixman-general.c
	pixman-gradient-walker.c
	pixman-image.c
	pixman-implementation.c
	pixman-linear-gradient.c
	pixman-matrix.c
	pixman-mips.c
	pixman-noop.c
	pixman-ppc.c
	pixman-radial-gradient.c
	pixman-region16.c
	pixman-region32.c
	pixman-solid-fill.c
	pixman-trap.c
	pixman-utils.c
	pixman-x86.c
	pixman.c)
