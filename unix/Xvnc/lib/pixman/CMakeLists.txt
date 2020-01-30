include_directories(pixman)

set(BUILTIN_CLZ_SOURCE "\n
	unsigned int x = 11;\n
	int main (void) {\n
		return __builtin_clz(x);\n
	}")
check_c_source_compiles("${BUILTIN_CLZ_SOURCE}" HAVE_BUILTIN_CLZ)
if(HAVE_BUILTIN_CLZ)
	add_definitions(-DHAVE_BUILTIN_CLZ)
endif()

add_definitions(-DHAVE_CONFIG_H -DPIXMAN_NO_TLS)

disable_compiler_warnings()

add_library(pixman STATIC
	pixman/pixman-access-accessors.c
	pixman/pixman-access.c
	pixman/pixman-arm.c
	pixman/pixman-bits-image.c
	pixman/pixman-combine-float.c
	pixman/pixman-combine32.c
	pixman/pixman-conical-gradient.c
	pixman/pixman-edge.c
	pixman/pixman-edge-accessors.c
	pixman/pixman-fast-path.c
	pixman/pixman-general.c
	pixman/pixman-glyph.c
	pixman/pixman-gradient-walker.c
	pixman/pixman-image.c
	pixman/pixman-implementation.c
	pixman/pixman-linear-gradient.c
	pixman/pixman-matrix.c
	pixman/pixman-mips.c
	pixman/pixman-noop.c
	pixman/pixman-ppc.c
	pixman/pixman-radial-gradient.c
	pixman/pixman-region16.c
	pixman/pixman-region32.c
	pixman/pixman-solid-fill.c
	pixman/pixman-trap.c
	pixman/pixman-utils.c
	pixman/pixman-x86.c
	pixman/pixman.c)
