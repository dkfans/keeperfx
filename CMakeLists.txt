cmake_minimum_required(VERSION 3.20)

# Do not allow building in root
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Do not build in the root. Create a bin directory and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif ()

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(keeperfx C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# Get the abbreviated commit Id of the head.
find_package(Git REQUIRED)
execute_process(COMMAND "${GIT_EXECUTABLE}" describe --always OUTPUT_VARIABLE COMMIT_ID OUTPUT_STRIP_TRAILING_WHITESPACE)

set(VER_MAJOR       1)
set(VER_MINOR       1)
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
include_directories($(${CMAKE_BINARY_DIR}))

find_package(unofficial-enet CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(SDL2_net CONFIG REQUIRED)
find_package(cpuid CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

# Global definitions.
add_compile_definitions(_CRT_NONSTDC_NO_WARNINGS _CRT_SECURE_NO_WARNINGS)

file(GLOB_RECURSE KEEPERFX_SOURCES_C "src/*.c")
file(GLOB_RECURSE KEEPERFX_SOURCES_CXX "src/*.cpp")

# Global definitions for all targets.
add_compile_definitions("DEBUG=$<IF:$<CONFIG:Debug>,1,0>")

# Add two executable targets: keeperfx and keeperfx_hvlog.
add_executable(keeperfx ${KEEPERFX_SOURCES_C} ${KEEPERFX_SOURCES_CXX})

target_compile_definitions(keeperfx PUBLIC BFDEBUG_LEVEL=0)
target_sources(keeperfx PRIVATE "res/keeperfx_stdres.rc")

add_executable(keeperfx_hvlog ${KEEPERFX_SOURCES_C} ${KEEPERFX_SOURCES_CXX})

target_compile_definitions(keeperfx_hvlog PUBLIC BFDEBUG_LEVEL=10)
target_sources(keeperfx_hvlog PRIVATE "res/keeperfx_stdres.rc")

# todo: Ideally we should not suppress so many kinds of warning.
# Some of them should be fixed in code.
if ("${CMAKE_CXX_COMPILER_ID};${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "Clang;MSVC") # clang-cl
    message(STATUS "We are using clang-cl")
    set(CLANG_DISABLED_WARNINGS_LIST
    "-Wno-microsoft-fixed-enum"
    "-Wno-unused-parameter"
    "-Wno-unused-but-set-variable"
    "-Wno-strict-aliasing"
    "-Wno-unknown-pragmas"
    "-Wno-undef"
    "-Wno-missing-prototypes"
    "-Wno-sign-compare"
    "-Wno-sign-conversion"
    "-Wno-implicit-int-conversion"
    "-Wno-strict-prototypes"
    "-Wno-unsafe-buffer-usage"
    "-Wno-gnu-zero-variadic-macro-arguments"
    "-Wno-zero-as-null-pointer-constant"
    "-Wno-switch-enum"
    "-Wno-unused-macros"
    "-Wno-double-promotion"
    "-Wno-comma"
    "-Wno-extra-semi"
    "-Wno-extra-semi-stmt"
    "-Wno-declaration-after-statement"
    "-Wno-documentation"
    "-Wno-c++98-compat"
    "-Wno-c++98-compat-pedantic"
    "-Wno-old-style-cast"
    "-Wno-missing-variable-declarations"
    "-Wno-cast-align"
    "-Wno-keyword-macro"
    "-Wno-nested-anon-types"
    "-Wno-shorten-64-to-32"
    "-Wno-implicit-fallthrough"
    "-Wno-gnu-anonymous-struct"
    "-Wno-zero-length-array"
    "-Wno-nonportable-system-include-path"
    "-Wno-newline-eof"
    "-Wno-deprecated-declarations"
    "-Wno-ignored-attributes"
    "-Wno-cast-function-type"
    "-Wno-cast-function-type-strict"
    "-Wno-conditional-uninitialized"
    "-Wno-reserved-identifier"
    "-Wno-float-equal"
    "-Wno-bad-function-cast"
    "-Wno-covered-switch-default"
    "-Wno-implicit-int-float-conversion"
    "-Wno-implicit-float-conversion"
    "-Wno-float-conversion"
    "-Wno-unreachable-code-break"
    "-Wno-language-extension-token"
    "-Wno-pointer-to-int-cast"
    "-Wno-global-constructors"
    "-Wno-cast-qual"
    "-Wno-absolute-value"
    "-Wno-unreachable-code-return"
    "-Wno-tautological-value-range-compare"
    "-Wno-int-to-pointer-cast"
    "-Wno-constant-conversion"
    "-Wno-format-nonliteral"
    "-Wno-incompatible-function-pointer-types-strict"
    "-Wno-empty-translation-unit"
    )
    # -Wextra will give us too many warnings.
    target_compile_options(keeperfx PRIVATE -Wall -Wshadow -Wmicrosoft-enum-value)
    target_compile_options(keeperfx_hvlog PRIVATE -Wall -Wshadow -Wmicrosoft-enum-value)
    target_compile_options(keeperfx PRIVATE ${CLANG_DISABLED_WARNINGS_LIST})
    target_compile_options(keeperfx_hvlog PRIVATE ${CLANG_DISABLED_WARNINGS_LIST})

    target_link_options(keeperfx PUBLIC "/MANIFEST:NO" "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
    target_link_options(keeperfx_hvlog PUBLIC "/MANIFEST:NO" "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC") # MSVC
    message(STATUS "We are using MSVC")
    set(MSVC_DISABLED_WARNINGS_LIST
    "/wd4018" # 'token' : signed/unsigned mismatch.
    "/wd4100" # unreferenced formal parameter
    "/wd4127" # conditional expression is constant
    "/wd4200" # nonstandard extension used : zero-sized array in
            # struct/union.
    "/wd4201" # nonstandard extension used : nameless struct/union
    "/wd4242" # 'function' : conversion from 'int' to 'uint8_t',
            # possible loss of data
    "/wd4244" # 'function' : conversion from 'int' to 'uint8_t',
            # possible loss of data
    "/wd4245" # 'initializing' : conversion from 'long' to
            # 'unsigned long', signed/unsigned mismatch
    "/wd4267" # conversion from 'size_t' to 'int', possible loss of data
    "/wd4388" # signed/unsigned mismatch
    "/wd4296" # '>=' : expression is always true
    "/wd4366" # The result of the unary '&' operator may be unaligned
    "/wd4365" # '=' : conversion from 'size_t' to 'int',
            # signed/unsigned mismatch
    "/wd4389" # '!=' : signed/unsigned mismatch
    "/wd4800" # 'int' : forcing value to bool 'true' or 'false'
            # (performance warning)
    "/wd4820" # 'bytes' bytes padding added after construct 'member_name'
    )
    target_compile_options(keeperfx PRIVATE /Zc:enumTypes /W4)
    target_compile_options(keeperfx_hvlog PRIVATE /Zc:enumTypes /W4)
    target_compile_options(keeperfx PRIVATE ${MSVC_DISABLED_WARNINGS_LIST})
    target_compile_options(keeperfx_hvlog PRIVATE ${MSVC_DISABLED_WARNINGS_LIST})
    target_link_options(keeperfx PUBLIC "/MANIFEST:NO" "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
    target_link_options(keeperfx_hvlog PUBLIC "/MANIFEST:NO" "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
else()
    message(STATUS "We are using ${CMAKE_CXX_COMPILER_ID}")
    set(WARNFLAGS -Wall -W -Wshadow -Wno-sign-compare -Wno-unused-parameter -Wno-strict-aliasing -Wno-unknown-pragmas)
    set(GNU_COMPILER_FLAG -march=x86-64 -fno-omit-frame-pointer -fmessage-length=0)
    set(GNU_LINK_FLAG -mwindows -Wl,--enable-auto-import)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=implicit")
    target_compile_options(keeperfx PRIVATE ${WARNFLAGS} ${GNU_COMPILER_FLAG})
    target_compile_options(keeperfx_hvlog PRIVATE ${WARNFLAGS} ${GNU_COMPILER_FLAG})
    target_link_options(keeperfx PRIVATE ${GNU_LINK_FLAG} -Wl,-Map,keeperfx.map)
    target_link_options(keeperfx_hvlog PRIVATE ${GNU_LINK_FLAG} -Wl,-Map,keeperfx_hvlog.map)
    target_link_libraries (keeperfx PUBLIC -static gcc stdc++ winpthread -dynamic)
    target_link_libraries (keeperfx_hvlog PUBLIC -static gcc stdc++ winpthread -dynamic)
endif()

# Go into submodules.
add_subdirectory(deps)

# System libraries.
# It seems that dbghelp and imagehlp cannot co-exist. We choose dbghelp.
target_link_libraries(keeperfx PRIVATE dbghelp)
target_link_libraries(keeperfx_hvlog PRIVATE dbghelp)

# External libraries.
target_link_libraries(keeperfx
    PRIVATE unofficial::enet::enet)
target_link_libraries(keeperfx
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>)
target_link_libraries(keeperfx
    PRIVATE $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>)
target_link_libraries(keeperfx
    PRIVATE $<IF:$<TARGET_EXISTS:SDL2_net::SDL2_net>,SDL2_net::SDL2_net,SDL2_net::SDL2_net-static>)
target_link_libraries(keeperfx
    PRIVATE $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>)
target_link_libraries(keeperfx PRIVATE cpuid::cpuid)
target_link_libraries(keeperfx PUBLIC ZLIB::ZLIB)

target_link_libraries(keeperfx_hvlog
    PRIVATE unofficial::enet::enet)
target_link_libraries(keeperfx_hvlog
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>)
target_link_libraries(keeperfx_hvlog
    PRIVATE $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>)
target_link_libraries(keeperfx_hvlog
    PRIVATE $<IF:$<TARGET_EXISTS:SDL2_net::SDL2_net>,SDL2_net::SDL2_net,SDL2_net::SDL2_net-static>)
target_link_libraries(keeperfx_hvlog
    PRIVATE $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>)
target_link_libraries(keeperfx_hvlog PRIVATE cpuid::cpuid)
target_link_libraries(keeperfx PUBLIC ZLIB::ZLIB)
