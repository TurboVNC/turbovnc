include(CheckTypeSize)

set(CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE=1)
check_type_size(off64_t OFF64_T)
if(HAVE_OFF64_T)
	add_definitions(-D_LARGEFILE64_SOURCE=1)
endif()
set(CMAKE_REQUIRED_DEFINITIONS)

add_definitions(-DHAVE_UNISTD_H -DHAVE_STDARG_H)

set(ZLIB_PUBLIC_HDRS
	zconf.h
	zlib.h)
set(ZLIB_PRIVATE_HDRS
	crc32.h
	deflate.h
	gzguts.h
	inffast.h
	inffixed.h
	inflate.h
	inftrees.h
	trees.h
	x86.h
	zutil.h)
set(ZLIB_SRCS
	adler32.c
	compress.c
	crc32.c
	crc_folding.c
	deflate.c
	deflate_medium.c
	deflate_quick.c
	gzclose.c
	gzlib.c
	gzread.c
	gzwrite.c
	infback.c
	inffast.c
	inflate.c
	inftrees.c
	match.c
	slide_sse.c
	trees.c
	uncompr.c
	x86.c
	zutil.c)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
	set_source_files_properties(crc_folding.c PROPERTIES
		COMPILE_OPTIONS "-mpclmul;-msse4")
	set_source_files_properties(slide_sse.c PROPERTIES COMPILE_OPTIONS -msse2)
	set_source_files_properties(deflate_quick.c PROPERTIES
		COMPILE_OPTIONS -msse4)
endif()

disable_compiler_warnings()

add_library(zlib STATIC ${ZLIB_SRCS} ${ZLIB_PUBLIC_HDRS} ${ZLIB_PRIVATE_HDRS})
