include_directories(../Xi ../mi ../render)

disable_compiler_warnings()
handle_type_puns(
	XKBGAlloc.c
	xkb.c
	xkbEvents.c
	xkbSwap.c
	xkbUtils.c)

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
