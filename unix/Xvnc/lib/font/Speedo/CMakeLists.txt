include_directories(../../../programs/Xserver/include)

add_library(Speedo OBJECT
	spfuncs.c
	spfile.c
	spinfo.c
	sperr.c
	spfont.c
	spglyph.c
	spencode.c
	bics-unicode.c
	do_char.c
	do_trns.c
	out_bl2d.c
	out_blk.c
	out_scrn.c
	out_util.c
	reset.c
	set_spcs.c
	set_trns.c)
