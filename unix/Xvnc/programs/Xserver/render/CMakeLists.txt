include_directories(. ../fb ../mi)

handle_type_puns()

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
