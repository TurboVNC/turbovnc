set(HEADERS
	include/GL/gl.h
	include/GL/glext.h)

foreach(h ${HEADERS})
	copy_X_header(${h} GL)
endforeach()

copy_X_header(include/GL/internal/dri_interface.h GL/internal)

include_directories(include src src/mapi src/mapi/glapi)

add_definitions(-DGLX_USE_TLS -DHAVE_PTHREAD -DMAPI_MODE_UTIL
	-DGLX_INDIRECT_RENDERING)
if(APPLE)
	add_definitions(-DGLX_ALIAS_UNSUPPORTED)
endif()

# This code has a lot of type puns.  Disable strict aliasing just to be safe.
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
endif()

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

add_library(glapi STATIC
	src/mapi/glapi/glapi.c
	src/mapi/glapi/glapi_dispatch.c
	src/mapi/glapi/glapi_entrypoint.c
	src/mapi/glapi/glapi_getproc.c
	src/mapi/glapi/glapi_nop.c
	src/mapi/u_current.c
	src/mapi/u_execmem.c)

add_library(glx STATIC
	src/glx/clientattrib.c
	src/glx/clientinfo.c
	src/glx/compsize.c
	src/glx/create_context.c
	src/glx/eval.c
	src/glx/glxcmds.c
	src/glx/glxconfig.c
	src/glx/glxcurrent.c
	src/glx/glxext.c
	src/glx/glxextensions.c
	src/glx/glxhash.c
	src/glx/glx_error.c
	src/glx/glx_pbuffer.c
	src/glx/glx_query.c
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
	src/glx/vertarr.c)
