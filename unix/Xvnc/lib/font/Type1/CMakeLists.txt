include_directories(../../../programs/Xserver/include)

# This code has some type puns.  Disable strict aliasing just to be safe.
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
endif()

add_definitions(-DBUILDCID -DHAVE_CFM)

add_library(Type1 OBJECT
	arith.c
	curves.c
	fontfcn.c
	hints.c
	lines.c
	objects.c
	paths.c
	regions.c
	scanfont.c
	spaces.c
	t1funcs.c
	t1info.c
	t1io.c
	t1malloc.c
	t1snap.c
	t1stub.c
 	token.c
	type1.c
	util.c
	cidchar.c
	afm.c
 	t1unicode.c)
