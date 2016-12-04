include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(BITS EQUAL 64)
	add_definitions(-D__GLX_ALIGN64)
endif()
add_definitions(-DGLX_USE_TLS -DPTHREADS)

# This code has a lot of type puns.  Disable strict aliasing just to be safe.
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
endif()

check_include_files(execinfo.h HAVE_EXECINFO_H)
check_function_exists(backtrace HAVE_BACKTRACE)
if(HAVE_EXECINFO_H AND HAVE_BACKTRACE)
	add_definitions(-DHAVE_EXECINFO_H -DHAVE_BACKTRACE)
endif()

if(BITS EQUAL 64)
	set(DEFAULT_DRI_DRIVER_PATH "/usr/lib64/dri")
else()
	set(DEFAULT_DRI_DRIVER_PATH "/usr/lib/dri")
endif()
set(DRI_DRIVER_PATH ${DEFAULT_DRI_DRIVER_PATH} CACHE PATH
	"Path to DRI drivers (default: ${DEFAULT_DRI_DRIVER_PATH})")
message(STATUS "DRI_DRIVER_PATH = ${DRI_DRIVER_PATH}")
add_definitions(-DDRI_DRIVER_PATH=\"${DRI_DRIVER_PATH}\")

check_c_source_compiles("int __thread test;  int main(void) { return 0; }"
	TLS_WORKS)
if(TLS_WORKS)
	set(TLS __thread)
	check_c_source_compiles("int ${TLS} __attribute__((tls_model(\"initial-exec\"))) test;  int main(void) { return 0; }"
		TLS_INITIAL_EXEC_WORKS)
	if(TLS_INITIAL_EXEC_WORKS)
		set(TLS "${TLS} __attribute__((tls_model(\"initial-exec\")))")
	endif()
	add_definitions(-DTLS=${TLS})
else()
	message(FATAL_ERROR "Could not figure out how to use thread-local storage with this compiler.")
endif()

message(STATUS "Thread-local storage directive: ${TLS}")

add_library(glx STATIC
	clientinfo.c
	createcontext.c
	extension_string.c
	glapi.c
	glapi_gentable.c
	glthread.c
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
	xfont.c)
