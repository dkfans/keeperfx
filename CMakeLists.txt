cmake_minimum_required(VERSION 3.20)

# Do not allow building in root
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Do not build in the root. Create a bin directory and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif ()

project(keeperfx LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 26)

include(GNUInstallDirs)
include(FindPkgConfig)
include(FindGit)

# Get the abbreviated commit Id of the head.
find_package(Git REQUIRED)
execute_process(COMMAND "${GIT_EXECUTABLE}" describe --always OUTPUT_VARIABLE COMMIT_ID OUTPUT_STRIP_TRAILING_WHITESPACE)

set(VER_MAJOR       1)
set(VER_MINOR       2)
set(VER_RELEASE     0)
set(VER_BUILD       0)
set(VER_STRING      "${VER_MAJOR}.${VER_MINOR}.${VER_RELEASE}.${VER_BUILD} ${PACKAGE_SUFFIX}")
set(PACKAGE_SUFFIX  "")
set(GIT_REVISION    "${COMMIT_ID}")

# CMAKE_BINARY_DIR is defined in CMakePresets.json.
set(KEEPERFX_VER_DEFS_H_IN ${CMAKE_SOURCE_DIR}/ver_defs.h.in)
set(KEEPERFX_VER_DEFS_H_OUT ${CMAKE_BINARY_DIR}/ver_defs.h)
configure_file(${KEEPERFX_VER_DEFS_H_IN} ${KEEPERFX_VER_DEFS_H_OUT})

# Add binary dir to the include paths, so ver_defs.h can be found.
include_directories($<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

# Enable Heavy Log only for -DCMAKE_BUILD_TYPE=Debug and -DDEBUG_LEVEL=7 or more
set(DEBUG_LEVEL 0 CACHE STRING "Debug level")
if(NOT DEBUG_LEVEL MATCHES "^[0-9]+$")
	message(FATAL_ERROR "DEBUG_LEVEL must be number from 0 to 10")
endif()

# Global definitions for all targets.
add_compile_definitions("DEBUG=$<IF:$<CONFIG:Debug>,1,0>")
add_compile_definitions(_CRT_NONSTDC_NO_WARNINGS _CRT_SECURE_NO_WARNINGS)

message(STATUS "We are using ${CMAKE_CXX_COMPILER_ID}")
set(WARNFLAGS -Wall -W -Wshadow -Werror -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter -Wno-error=attributes -Wno-error=int-conversion -Wno-error=format-truncation)
set(GNU_COMPILER_FLAG -march=native -O3 -g -fno-omit-frame-pointer -fmessage-length=0)
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -Wimplicit)

option(BFLIB_BACKEND_SWRAST "The Software Rasterizer framework to use" SDL2)
option(BFLIB_BACKEND_OPENGL "The OpenGL platform framework to use"     SDL2)

find_package(PkgConfig REQUIRED)
find_package(ZLIB REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(SDL2_net CONFIG REQUIRED)
pkg_check_modules(MINIZIP IMPORTED_TARGET REQUIRED minizip)
pkg_check_modules(SPNG IMPORTED_TARGET REQUIRED spng)
pkg_check_modules(AVFORMAT IMPORTED_TARGET REQUIRED libavformat)
pkg_check_modules(AVCODEC IMPORTED_TARGET REQUIRED libavcodec)
pkg_check_modules(AVUTIL IMPORTED_TARGET REQUIRED libavutil)
pkg_check_modules(SWRESAMPLE IMPORTED_TARGET REQUIRED libswresample)
pkg_check_modules(ENET IMPORTED_TARGET REQUIRED libenet)

file(GLOB_RECURSE KEEPERFX_SOURCES_C "src/*.c")
file(GLOB_RECURSE KEEPERFX_SOURCES_CXX "src/*.cpp")

# Add two executable targets: keeperfx.
add_executable(keeperfx ${KEEPERFX_SOURCES_C} ${KEEPERFX_SOURCES_CXX})
target_compile_definitions(keeperfx PUBLIC "BFDEBUG_LEVEL=$<IF:$<CONFIG:Debug>,${DEBUG_LEVEL}, 0>")
target_sources(keeperfx PRIVATE "res/keeperfx_stdres.rc")
target_compile_options(keeperfx PRIVATE ${WARNFLAGS} ${GNU_COMPILER_FLAG})
target_include_directories(keeperfx PUBLIC ZLIB PkgConfig::MINIZIP SDL2::SDL2 SDL2_mixer SDL2_net::SDL2_net SDL2_image::SDL2_image PkgConfig::ENET PkgConfig::AVUTIL PkgConfig::AVFORMAT PkgConfig::SWRESAMPLE PkgConfig::AVCODEC PkgConfig::SPNG)
target_link_options(keeperfx PRIVATE ${GNU_LINK_FLAG} -Wl,-Map,keeperfx.map)
target_link_libraries(keeperfx PUBLIC ZLIB::ZLIB PkgConfig::MINIZIP SDL2::SDL2 SDL2_mixer::SDL2_mixer SDL2_net::SDL2_net SDL2_image::SDL2_image PkgConfig::ENET PkgConfig::AVUTIL PkgConfig::AVFORMAT PkgConfig::SWRESAMPLE PkgConfig::AVCODEC PkgConfig::SPNG)
target_link_libraries (keeperfx PUBLIC -dynamic)

add_subdirectory(deps)
add_subdirectory(tools)

