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

# --- Source list ------------------------------------------------------------
# The Makefiles build a curated object list, not a glob; match them.
file(GLOB_RECURSE KEEPERFX_SOURCES_C "src/*.c")
file(GLOB_RECURSE KEEPERFX_SOURCES_CXX "src/*.cpp")

# The functional-test harness is off by default (FTEST_DEBUG defaults to 0).
list(FILTER KEEPERFX_SOURCES_C   EXCLUDE REGEX "/src/ftests/")
list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX "/src/ftests/")
if(WIN32)
    # linux.cpp is the Linux entry point (POSIX headers, own main()); never
    # built on Windows (matches ../Makefile).
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX "/src/linux\\.cpp$")
else()
    # cdrom/steam_api/windows are Windows-only TUs (matches ../linux.mk).
    list(FILTER KEEPERFX_SOURCES_CXX EXCLUDE REGEX "/src/(cdrom|steam_api|windows)\\.cpp$")
endif()

# --- Targets ----------------------------------------------------------------
add_compile_definitions("DEBUG=$<IF:$<CONFIG:Debug>,1,0>")

add_executable(keeperfx       ${KEEPERFX_SOURCES_C} ${KEEPERFX_SOURCES_CXX})
add_executable(keeperfx_hvlog ${KEEPERFX_SOURCES_C} ${KEEPERFX_SOURCES_CXX})
target_compile_definitions(keeperfx       PUBLIC BFDEBUG_LEVEL=0)
target_compile_definitions(keeperfx_hvlog PUBLIC BFDEBUG_LEVEL=10)

message(STATUS "Building keeperfx with ${CMAKE_CXX_COMPILER_ID} for ${CMAKE_SYSTEM_NAME}")

# --- Platform compile/link setup --------------------------------------------
if(WIN32)
    # Windows / MinGW-w64 (i686) — mirrors ../Makefile.
    add_compile_definitions(_CRT_NONSTDC_NO_WARNINGS _CRT_SECURE_NO_WARNINGS)
    add_compile_definitions("SPNG_STATIC=1")     # spng linked statically
    add_compile_definitions("AL_LIBTYPE_STATIC") # OpenAL linked statically

    # Version/icon resource (needs windres + a generated .ico from tools/).
    target_sources(keeperfx       PRIVATE "res/keeperfx_stdres.rc")
    target_sources(keeperfx_hvlog PRIVATE "res/keeperfx_stdres.rc")

    # bfd is slow; prefer LLD when available (LINKER_TYPE needs CMake >= 3.29,
    # harmlessly ignored on older CMake, which then uses the default linker).
    set_property(TARGET keeperfx       PROPERTY LINKER_TYPE LLD)
    set_property(TARGET keeperfx_hvlog PROPERTY LINKER_TYPE LLD)

    set(WARNFLAGS -Wall -W -Wshadow -Wno-sign-compare -Wno-unused-parameter
        -Wno-maybe-uninitialized -Wno-strict-aliasing -Wno-unknown-pragmas
        -Werror -Wno-format-truncation)
    # -O3 matches the Makefile's release OPTFLAGS (RelWithDebInfo defaults to -O2).
    set(GNU_COMPILER_FLAG -march=x86-64 -fno-omit-frame-pointer -fmessage-length=0 -O3)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wimplicit")
    foreach(_t keeperfx keeperfx_hvlog)
        target_compile_options(${_t} PRIVATE ${WARNFLAGS} ${GNU_COMPILER_FLAG})
        target_link_options(${_t} PRIVATE -mwindows -Wl,--enable-auto-import -Wl,-Map,${_t}.map)
        target_link_libraries(${_t} PUBLIC -static stdc++ winpthread -dynamic)
    endforeach()
else()
    # Linux (x86_64) — mirrors ../linux.mk (native gcc, no -mwindows).
    set(WARNFLAGS -Wall -Wextra -Werror -Wno-unused-parameter
        -Wno-unknown-pragmas -Wno-format-truncation -Wno-sign-compare)
    set(GNU_COMPILER_FLAG -g -O3 -march=x86-64)
    foreach(_t keeperfx keeperfx_hvlog)
        target_compile_options(${_t} PRIVATE ${WARNFLAGS} ${GNU_COMPILER_FLAG})
        target_compile_options(${_t} PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wno-absolute-value>)
        target_link_options(${_t} PRIVATE -g -rdynamic)
    endforeach()
endif()

# deps/ links all third-party libraries onto the two targets (per platform).
add_subdirectory(deps)

# The Windows icon resource is generated by png2ico in tools/ (Windows only).
if(WIN32)
    add_subdirectory(tools)
endif()

# Distribution packaging (CPack). Combines the CMake-built keeperfx binary with
# the game data staged by "make pkg-assemble". See the module for the flow.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/build/cmake/modules)
include(Packaging)
