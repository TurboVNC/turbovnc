include_directories(. ../fb ../mi)

add_library(render STATIC
	animcur.c
	filter.c
	glyph.c
	miglyph.c
	miindex.c
	mipict.c
	mirect.c
	mitrap.c
	mitri.c
	picture.c
	render.c
	renderedge.c)
