cmake_minimum_required( VERSION 3.8 )
project( MapleTools VERSION 1.0.0 LANGUAGES C CXX )

### Verbosity
set( CMAKE_COLOR_MAKEFILE ON )
set( CMAKE_VERBOSE_MAKEFILE ON )

# Generate 'compile_commands.json' for clang_complete
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

## Threads
find_package( Threads REQUIRED )

## mio: Multiplatform memory mapped IO
add_subdirectory(
	dependencies/mio
)

### Targets

## Maple 2 library
add_library(
	Maple2
	STATIC
	source/Maple2/Keys.cpp
	source/Maple2/Util.cpp
)
target_include_directories(
	Maple2
	PUBLIC
	include
)
set_target_properties(
	Maple2
	PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS ON
)

if(MSVC)
	include(dependencies/DownloadProject/DownloadProject.cmake)
	download_project(
		PROJ cryptopp
		GIT_REPOSITORY git@github.com:weidai11/cryptopp.git
		GIT_TAG master
		PREFIX cryptopp
	)
	# Copy files from cryptopp-cmake to the working sources dir
	configure_file(
		dependencies/cryptopp-cmake/CMakeLists.txt
		${CMAKE_BINARY_DIR}/cryptopp/cryptopp-src/CMakeLists.txt COPYONLY
	)
	configure_file(
		dependencies/cryptopp-cmake/cryptopp-config.cmake
		${CMAKE_BINARY_DIR}/cryptopp/cryptopp-src/cryptopp-config.cmake COPYONLY
	)
	add_subdirectory(
		${CMAKE_BINARY_DIR}/cryptopp/cryptopp-src ${CMAKE_BINARY_DIR}/cryptopp/cryptopp-build
	)
	# create includes
	file(
		GLOB cryptopp_HEADERS
		"${CMAKE_BINARY_DIR}/cryptopp/cryptopp-src/*.h"
	)
	file(
		COPY ${cryptopp_HEADERS}
		DESTINATION ${CMAKE_BINARY_DIR}/cryptopp/include/cryptopp
	)
	target_include_directories(
		Maple2
		PRIVATE
		${CMAKE_BINARY_DIR}/cryptopp/include/
	)
	target_link_libraries(
		Maple2
		PRIVATE
		cryptopp-static
	)
else()
	target_link_libraries(
		Maple2
		PRIVATE
		cryptopp
	)
endif()

## Expands .m2h/.m2d files into folder and "flattens" the virtual file system
add_executable(
	Expand
	tools/Expand.cpp
)
set_target_properties(
	Expand
	PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS ON
)
target_include_directories(
	Expand
	PRIVATE
	dependencies/Clara/include
)
target_link_libraries(
	Expand
	PRIVATE
	Maple2
	mio
	${CMAKE_THREAD_LIBS_INIT}
)

# Packs a folder into a .m2h/.m2d pair
add_executable(
	Pack
	tools/Pack.cpp
)
set_target_properties(
	Pack
	PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS ON
)
target_link_libraries(
	Pack
	PRIVATE
	Maple2
	${CMAKE_THREAD_LIBS_INIT}
)

# Link filesystem libs for GCC/Clang
if( CMAKE_COMPILER_IS_GNUCXX )
	target_link_libraries(
		Maple2
		PRIVATE
		stdc++fs
	)
	target_link_libraries(
		Pack
		PRIVATE
		stdc++fs
	)
	target_link_libraries(
		Expand
		PRIVATE
		stdc++fs
	)
endif()
