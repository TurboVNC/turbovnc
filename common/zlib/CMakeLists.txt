if(WIN32)
	add_definitions(-wd4996)
else()
	add_definitions(-DHAVE_UNISTD_H)
endif()
add_definitions(-DHAVE_STDARG_H)

if(CPU_TYPE STREQUAL "x86_64")
	if(NOT WIN32)
		add_definitions(-DUNALIGNED_OK -DADLER32_UNROLL_LESS -DCRC32_UNROLL_LESS
			-DHAVE_SSE2 -DUSE_SSE4_2_CRC_HASH -DUSE_MEDIUM)
	endif()
	set(X86_SOURCES fill_window_sse.c x86.c)
elseif(CPU_TYPE STREQUAL "i386")
	if(NOT WIN32)
		add_definitions(-DX86 -DUNALIGNED_OK -DADLER32_UNROLL_LESS
			-DCRC32_UNROLL_LESS -DCHECK_SSE2 -DHAVE_SSE2 -DUSE_SSE4_2_CRC_HASH
			-DUSE_MEDIUM)
		set_source_files_properties(fill_window_sse.c PROPERTIES COMPILE_FLAGS
			-msse2)
	endif()
	set(X86_SOURCES fill_window_sse.c x86.c)
endif()

disable_compiler_warnings()

add_library(zlib STATIC
	adler32.c
	compress.c
	crc32.c
	crc_folding.c
	deflate.c
	gzclose.c
	gzlib.c
	gzread.c
	gzwrite.c
	infback.c
	inffast.c
	inflate.c
	inftrees.c
	trees.c
	uncompr.c
	zutil.c
	${X86_SOURCES})
