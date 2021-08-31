set(HEADERS
	include/GL/gl.h
	include/GL/glext.h)

foreach(h ${HEADERS})
	copy_X_header(${h} GL)
endforeach()

copy_X_header(include/GL/internal/dri_interface.h GL/internal)
copy_X_header(include/KHR/khrplatform.h KHR)

include_directories(include src src/mapi src/mapi/glapi)

check_symbol_exists(random_r stdlib.h HAVE_RANDOM_R)
if(HAVE_RANDOM_R)
	add_definitions(-DHAVE_RANDOM_R)
endif()

add_definitions(-DUSE_ELF_TLS -DHAVE_PTHREAD -DHAVE_TIMESPEC_GET
	-DMAPI_MODE_UTIL -DGLX_INDIRECT_RENDERING)

disable_compiler_warnings()

add_library(glapi-server STATIC
	src/mapi/glapi/glapi_dispatch.c
	src/mapi/glapi/glapi_entrypoint.c
	src/mapi/glapi/glapi_getproc.c
	src/mapi/glapi/glapi_nop.c
	src/mapi/glapi/glapi.c
	src/mapi/u_current.c
	src/mapi/u_execmem.c)

add_library(glx STATIC
	src/glx/clientattrib.c
	src/glx/clientinfo.c
	src/glx/compsize.c
	src/glx/create_context.c
	src/glx/eval.c
	src/glx/glx_error.c
	src/glx/glx_pbuffer.c
	src/glx/glx_query.c
	src/glx/glxcmds.c
	src/glx/glxconfig.c
	src/glx/glxcurrent.c
	src/glx/glxext.c
	src/glx/glxextensions.c
	src/glx/glxhash.c
	src/glx/indirect.c
	src/glx/indirect_glx.c
	src/glx/indirect_init.c
	src/glx/indirect_texture_compression.c
	src/glx/indirect_transpose_matrix.c
	src/glx/indirect_vertex_array.c
	src/glx/indirect_vertex_program.c
	src/glx/indirect_window_pos.c
	src/glx/pixel.c
	src/glx/pixelstore.c
	src/glx/query_renderer.c
	src/glx/render2.c
	src/glx/renderpix.c
	src/glx/single2.c
	src/glx/singlepix.c
	src/glx/vertarr.c
	src/util/debug.c)
