cmake_minimum_required( VERSION 3.2.2 )
project( MapleTools )

### Standard
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS ON )

### Verbosity
set( CMAKE_COLOR_MAKEFILE ON )
set( CMAKE_VERBOSE_MAKEFILE ON )

# Generate 'compile_commands.json' for clang_complete
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

### Global includes
include_directories(
	include
)

### Flags
if( MSVC )
	add_compile_options( /W3 )
elseif( CMAKE_COMPILER_IS_GNUCXX )
	add_compile_options( -Wall )
	add_compile_options( -Wextra )
endif()


## Threads
find_package( Threads REQUIRED )

## mio: Multiplatform memory mapped IO
add_subdirectory(
	dependencies/mio
)

### Targets
add_library(
	Maple2
	STATIC
	source/Maple2/Keys.cpp
	source/Maple2/Util.cpp
)
target_link_libraries(
	Maple2
	PRIVATE
	cryptopp
)

## MapleStory2 Tools

# Expands .m2h/.m2d files into folder and "flattens" the virtual file system
add_executable(
	Expand
	tools/Expand.cpp
)
target_include_directories(
	Expand
	PRIVATE
	dependencies/Clara/include
	mio
)
target_link_libraries(
	Expand
	PRIVATE
	Maple2
	${CMAKE_THREAD_LIBS_INIT}
)

# Packs a folder into a .m2h/.m2d pair
add_executable(
	Pack
	tools/Pack.cpp
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
