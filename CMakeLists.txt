cmake_minimum_required(VERSION 3.20)

# Do not allow building in root
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Do not build in the root. Create a bin directory and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif ()

project(keeperfx C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# Get the abbreviated commit Id of the head.
find_package(Git REQUIRED)
execute_process(COMMAND "${GIT_EXECUTABLE}" describe --always OUTPUT_VARIABLE COMMIT_ID OUTPUT_STRIP_TRAILING_WHITESPACE)

set(VER_MAJOR       1)
set(VER_MINOR       2)
set(VER_RELEASE     0)
set(VER_BUILD       0)
# Packaging parameters (override via -DBUILD_NUMBER=<n> -DPACKAGE_SUFFIX=<s>).
# Used for the CPack archive name so it matches the Makefile's convention.
set(BUILD_NUMBER    "${VER_BUILD}" CACHE STRING "Monotonic build number")
set(PACKAGE_SUFFIX  ""             CACHE STRING "Distribution suffix (e.g. Alpha, Release)")
set(VER_STRING      "${VER_MAJOR}.${VER_MINOR}.${VER_RELEASE}.${BUILD_NUMBER} ${PACKAGE_SUFFIX}")
set(GIT_REVISION    "${COMMIT_ID}")

# Generate the version header. src/version.h does #include "ver_defs.h", which
# resolves relative to src/, so the generated header must land in src/ (this is
# exactly where the Makefile puts it: GENSRC = src/ver_defs.h).
set(KEEPERFX_VER_DEFS_H_IN ${CMAKE_SOURCE_DIR}/ver_defs.h.in)
set(KEEPERFX_VER_DEFS_H_OUT ${CMAKE_SOURCE_DIR}/src/ver_defs.h)
configure_file(${KEEPERFX_VER_DEFS_H_IN} ${KEEPERFX_VER_DEFS_H_OUT})

# SDL2 and all other dependencies are prebuilt MinGW-w64 (i686) libraries
# provided by deps/ (add_subdirectory below), mirroring the hand Makefile.

# Global definitions.
add_compile_definitions(_CRT_NONSTDC_NO_WARNINGS _CRT_SECURE_NO_WARNINGS)

file(GLOB_RECURSE KEEPERFX_SOURCES_C "src/*.c")
file(GLOB_RECURSE KEEPERFX_SOURCES_CXX "src/*.cpp")

# The Makefile builds a curated object list, not a glob. Match it:
#  - the functional test harness is off by default (FTEST_DEBUG defaults to 0);
#  - src/linux.cpp is the Linux entry point (POSIX headers, its own main()) and
#    is only built by linux.mk, never the Windows/MinGW build.
list(FILTER KEEPERFX_SOURCES_C   EXCLUDE REGEX "/src/ftests/")
list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX "/src/ftests/")
list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX "/src/linux\\.cpp$")

# Global definitions for all targets.
add_compile_definitions("DEBUG=$<IF:$<CONFIG:Debug>,1,0>")
# Match the Makefile's dependency-related defines (DEPFLAGS + CFLAGS).
add_compile_definitions("SPNG_STATIC=1")
add_compile_definitions("AL_LIBTYPE_STATIC")

# Add two executable targets: keeperfx and keeperfx_hvlog.
add_executable(keeperfx ${KEEPERFX_SOURCES_C} ${KEEPERFX_SOURCES_CXX})

target_compile_definitions(keeperfx PUBLIC BFDEBUG_LEVEL=0)
target_sources(keeperfx PRIVATE "res/keeperfx_stdres.rc")

add_executable(keeperfx_hvlog ${KEEPERFX_SOURCES_C} ${KEEPERFX_SOURCES_CXX})

target_compile_definitions(keeperfx_hvlog PUBLIC BFDEBUG_LEVEL=10)
target_sources(keeperfx_hvlog PRIVATE "res/keeperfx_stdres.rc")

message(STATUS "We are using ${CMAKE_CXX_COMPILER_ID}")

# The default bfd linker in MinGW is extremely slow. LLVM linker (LLD) is much much faster.
set_property(TARGET keeperfx PROPERTY LINKER_TYPE LLD)
set_property(TARGET keeperfx_hvlog PROPERTY LINKER_TYPE LLD)

set(WARNFLAGS -Wall -W -Wshadow -Wno-sign-compare -Wno-unused-parameter -Wno-maybe-uninitialized -Wno-strict-aliasing -Wno-unknown-pragmas -Werror -Wno-format-truncation)
# -O3 matches the Makefile's release OPTFLAGS (CMake's RelWithDebInfo defaults to
# -O2); appended last so it wins over the build-type default.
set(GNU_COMPILER_FLAG -march=x86-64 -fno-omit-frame-pointer -fmessage-length=0 -O3)
set(GNU_LINK_FLAG -mwindows -Wl,--enable-auto-import)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wimplicit")
target_compile_options(keeperfx PRIVATE ${WARNFLAGS} ${GNU_COMPILER_FLAG})
target_compile_options(keeperfx_hvlog PRIVATE ${WARNFLAGS} ${GNU_COMPILER_FLAG})
target_link_options(keeperfx PRIVATE ${GNU_LINK_FLAG} -Wl,-Map,keeperfx.map)
target_link_options(keeperfx_hvlog PRIVATE ${GNU_LINK_FLAG} -Wl,-Map,keeperfx_hvlog.map)
target_link_libraries (keeperfx PUBLIC -static stdc++ winpthread -dynamic)
target_link_libraries (keeperfx_hvlog PUBLIC -static stdc++ winpthread -dynamic)

# Go into submodules. deps/ defines the prebuilt dependency targets used below.
add_subdirectory(deps)
add_subdirectory(tools)

# All external dependencies (mirroring the Makefile's LINKLIB). The static
# archives have circular references (e.g. curl<->zlib, ffmpeg internals), so
# they are wrapped in a linker group (RESCAN == --start-group/--end-group)
# and ordering between them is irrelevant. SDL2, centitoml, and the trailing
# Windows system libraries are linked normally.
set(KFX_STATIC_DEPS
    libavformat_static libavcodec_static libswresample_static libavutil_static
    openal_static astronomy_static enet6_static miniupnpc_static natpmp_static
    curl_static spng_static centijson_static minizip_static zlib_static
    luajit_static)

foreach(_tgt keeperfx keeperfx_hvlog)
    target_link_libraries(${_tgt} PRIVATE
        kfx_sdl2
        "$<LINK_GROUP:RESCAN,${KFX_STATIC_DEPS}>"
        centitoml
        # Windows system libs (matches the Makefile LINKLIB trailer).
        winmm mingw32 imagehlp ws2_32 dbghelp bcrypt ole32 uuid)
endforeach()

# Distribution packaging (CPack). Combines the CMake-built keeperfx binary with
# the game data staged by "make pkg-assemble". See the module for the flow.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/build/cmake/modules)
include(Packaging)
