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

target_link_libraries(
	Maple2
	PRIVATE
	cryptopp
)

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
