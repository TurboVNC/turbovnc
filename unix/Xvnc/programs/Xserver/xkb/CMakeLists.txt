include_directories(../Xi ../mi ../render)

add_definitions(-DXKM_OUTPUT_DIR=\"compiled/\")

disable_compiler_warnings()

add_library(xkb STATIC
	XKBAlloc.c
	XKBGAlloc.c
	XKBMAlloc.c
	XKBMisc.c
	ddxBeep.c
	ddxCtrls.c
	ddxKillSrv.c
	ddxLEDs.c
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
