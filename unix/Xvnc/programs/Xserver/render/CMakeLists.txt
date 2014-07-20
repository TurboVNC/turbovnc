include_directories(. ../fb ../mi)

# This code has a lot of type puns.  Disable strict aliasing just to be safe.
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
endif()

add_library(render STATIC
	animcur.c
	filter.c
	glyph.c
	matrix.c
	miindex.c
	mipict.c
	mirect.c
	mitrap.c
	mitri.c
	picture.c
	render.c)
