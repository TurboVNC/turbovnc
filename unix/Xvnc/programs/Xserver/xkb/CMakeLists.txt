include_directories(../Xi ../mi)

# This code has a lot of type puns.  Disable strict aliasing just to be safe.
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
endif()

add_library(xkb STATIC
	XKBAlloc.c
	XKBGAlloc.c
	XKBMAlloc.c
	XKBMisc.c
	ddxBeep.c
	ddxCtrls.c
	ddxKillSrv.c
	ddxLEDs.c
	ddxList.c
	ddxLoad.c
	ddxPrivate.c
	ddxVT.c
	maprules.c
	xkb.c
	xkbAccessX.c
	xkbActions.c
	xkbEvents.c
	xkbInit.c
	xkbLEDs.c
	xkbPrKeyEv.c
	xkbSwap.c
	xkbUtils.c
	xkbfmisc.c
	xkbout.c
	xkbtext.c
	xkmread.c)
