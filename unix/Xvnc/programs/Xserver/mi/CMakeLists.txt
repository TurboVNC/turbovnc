include_directories(../fb ../render ../Xi ../miext/damage)

# This code has a lot of type puns.  Disable strict aliasing just to be safe.
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
endif()

add_library(mi STATIC
	miarc.c
	mibitblt.c
	micmap.c
	micopy.c
	micursor.c
	midash.c
	midispcur.c
	mieq.c
	miexpose.c
	mifillarc.c
	mifillrct.c
	mifpolycon.c
	migc.c
	miglblt.c
	miinitext.c
	mioverlay.c
	mipointer.c
	mipoly.c
	mipolycon.c
	mipolygen.c
	mipolypnt.c
	mipolyrect.c
	mipolyseg.c
	mipolytext.c
	mipolyutil.c
	mipushpxl.c
	miscrinit.c
	mispans.c
	misprite.c
	mivaltree.c
	miwideline.c
	miwindow.c
	mizerarc.c
	mizerclip.c
	mizerline.c)
