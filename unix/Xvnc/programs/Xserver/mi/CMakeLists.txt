include_directories(. ../fb ../render)

add_definitions(${EXT_DEFINES})

add_library(mi STATIC
	cbrt.c
	mivaltree.c
	mipolyseg.c
	mipolyrect.c
	mipoly.c
	mipolycon.c
	mipolygen.c
	mipolyutil.c
	mifillrct.c
	miwideline.c
	mispans.c
	miarc.c
	mizerarc.c
	mifillarc.c
	miwindow.c
	micursor.c
	miregion.c
	mipolytext.c
	mibitblt.c
	mipolypnt.c
	mipushpxl.c
	miexpose.c
	miglblt.c
	mizerline.c
	mifpolycon.c
	midash.c
	mibstore.c
	miinitext.c
	mieq.c
	mipointer.c
	misprite.c
	midispcur.c
	miscrinit.c
	miclipn.c
	migc.c
	micmap.c)
