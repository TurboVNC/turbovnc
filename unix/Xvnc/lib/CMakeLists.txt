if(TVNC_ZLIBNG)
	include(ExternalProject)

	# WITH_NEW_STRATEGIES=0 makes zlib-ng compress as well as zlib when using
	# Compression Level 1.
	#
	# ZLIB_COMPAT=1 makes zlib-ng emulate the zlib API.
	#
	# The rest of the CMake arguments disable unneeded (unincluded, in the case
	# of testing) build features and build zlib-ng using the same compiler and
	# flags that are used to build the TurboVNC Server.

	ExternalProject_Add(zlib-ng BINARY_DIR zlib-ng
		SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib-ng
		CMAKE_ARGS
			-DBUILD_SHARED_LIBS=0
			-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
			"-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS} -w"
			-DCMAKE_C_FLAGS_DEBUG=${CMAKE_C_FLAGS_DEBUG}
			-DCMAKE_C_FLAGS_MINSIZEREL=${CMAKE_C_FLAGS_MINSIZEREL}
			-DCMAKE_C_FLAGS_RELEASE=${CMAKE_C_FLAGS_RELEASE}
			-DCMAKE_C_FLAGS_RELWITHDEBINFO=${CMAKE_C_FLAGS_RELWITHDEBINFO}
			-DWITH_GTEST=0
			-DWITH_GZFILEOP=0
			-DWITH_NEW_STRATEGIES=0
			-DWITH_SANITIZER=0
			-DZLIBNG_ENABLE_TESTS=0
			-DZLIB_COMPAT=1
			-DZLIB_ENABLE_TESTS=0
		INSTALL_COMMAND ""
		STEP_TARGETS build)
endif()

add_subdirectory(libsha1)
