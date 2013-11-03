include_directories(../../../programs/Xserver/include)

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
