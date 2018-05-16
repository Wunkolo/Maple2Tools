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

### Optimizations
if( MSVC )
	add_compile_options( /arch:AVX2 )
	add_compile_options( /W3 )
elseif( CMAKE_COMPILER_IS_GNUCXX )
	add_compile_options( -m64 )
	add_compile_options( -march=native )
	add_compile_options( -Ofast )
	add_compile_options( -Wall )
	add_compile_options( -Wextra )
endif()

### Targets
add_executable(
	m2h
	source/m2h.cpp
	source/Maple2/Keys.cpp
)
target_link_libraries(
	m2h
	cryptopp
	z
)

