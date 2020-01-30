include_directories(. ../mi)

disable_compiler_warnings()

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
