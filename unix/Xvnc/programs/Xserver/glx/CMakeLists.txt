include_directories(. ../composite ../randr ../render)

if(BITS EQUAL 64)
	add_definitions(-D__GLX_ALIGN64)
endif()

check_symbol_exists(backtrace execinfo.h HAVE_BACKTRACE)
if(HAVE_BACKTRACE)
	add_definitions(-DHAVE_BACKTRACE)
endif()

if(BITS EQUAL 64)
	set(DEFAULT_XORG_DRI_DRIVER_PATH "/usr/lib64/dri")
else()
	set(DEFAULT_XORG_DRI_DRIVER_PATH "/usr/lib/dri")
endif()
set(XORG_DRI_DRIVER_PATH ${DEFAULT_XORG_DRI_DRIVER_PATH} CACHE PATH
	"X.org DRI driver path, which contains swrast_dri.so (default: ${DEFAULT_XORG_DRI_DRIVER_PATH})")
message(STATUS "XORG_DRI_DRIVER_PATH = ${XORG_DRI_DRIVER_PATH}")
add_definitions(-DDRI_DRIVER_PATH=\"${XORG_DRI_DRIVER_PATH}\")

disable_compiler_warnings()

add_library(glx-server STATIC
	clientinfo.c
	createcontext.c
	extension_string.c
	glxcmds.c
	glxcmdsswap.c
	glxdricommon.c
	glxdriswrast.c
	glxext.c
	glxscreens.c
	indirect_dispatch.c
	indirect_dispatch_swap.c
	indirect_program.c
	indirect_reqsize.c
	indirect_size_get.c
	indirect_table.c
	indirect_texture_compression.c
	indirect_util.c
	render2.c
	render2swap.c
	renderpix.c
	renderpixswap.c
	rensize.c
	single2.c
	single2swap.c
	singlepix.c
	singlepixswap.c
	singlesize.c
	swap_interval.c
	vndcmds.c
	vndext.c
	vndservermapping.c
	vndservervendor.c
	xfont.c)
