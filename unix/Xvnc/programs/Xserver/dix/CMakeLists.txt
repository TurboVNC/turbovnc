include_directories(../Xi ../composite ../damageext ../mi ../miext/damage
	../render ../xfixes)

add_definitions(-DRGB_DB=\"/usr/share/X11/rgb\"
	-DCOMPILEDDEFAULTFONTPATH=\"/usr/share/X11/fonts/misc/,/usr/share/X11/fonts/Type1/,/usr/share/X11/fonts/75dpi/,/usr/share/X11/fonts/100dpi/\")

# This code has a lot of type puns.  Disable strict aliasing just to be safe.
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
endif()

add_library(main STATIC
	main.c)

add_library(dix STATIC
	atom.c
	colormap.c
	cursor.c
	devices.c
	dispatch.c
	dixfonts.c
	dixutils.c
	enterleave.c
	eventconvert.c
	events.c
	extension.c
	ffs.c
	gc.c
	getevents.c
	globals.c
	glyphcurs.c
	grabs.c
	initatoms.c
	inpututils.c
	pixmap.c
	privates.c
	property.c
	ptrveloc.c
	region.c
	registry.c
	resource.c
	selection.c
	swaprep.c
	swapreq.c
	tables.c
	touch.c
	window.c)
