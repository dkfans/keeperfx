cmake_minimum_required(VERSION 3.20)

## libspng supports CMake, let's use it.
# Adjust options of libspng.
option(SPNG_SHARED "" OFF)
option(BUILD_EXAMPLES "" OFF)
add_subdirectory(libspng)
if ("${CMAKE_CXX_COMPILER_ID};${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "Clang;MSVC") # clang-cl
    target_compile_options(spng_static PRIVATE -w)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(spng_static PRIVATE /W0)
else()
    target_compile_options(spng_static PRIVATE -w)
endif()
target_include_directories(spng_static INTERFACE "libspng/spng")
target_link_libraries(keeperfx PRIVATE spng_static)
target_link_libraries(keeperfx_hvlog PRIVATE spng_static)

## centijson supports CMake, let's use it.
add_subdirectory(centijson)
target_include_directories(json INTERFACE "centijson/src")
target_link_libraries(keeperfx PRIVATE json)
target_link_libraries(keeperfx_hvlog PRIVATE json)

## centitoml does not support CMake. It requires centijson.
add_library(centitoml OBJECT "centitoml/toml_api.c")
target_link_libraries(centitoml PUBLIC json)
target_include_directories(centitoml INTERFACE "centitoml")
target_link_libraries(keeperfx PRIVATE centitoml)
target_link_libraries(keeperfx_hvlog PRIVATE centitoml)

## astronomy does not support CMake.
add_library(astronomy OBJECT "astronomy/astronomy.c")
target_include_directories(astronomy INTERFACE "astronomy")
target_link_libraries(keeperfx PRIVATE astronomy)
target_link_libraries(keeperfx_hvlog PRIVATE astronomy)

## zlib supports CMake. However, the contrib/minizip does not.

#[[
Do not build zlib from the submodule because it causes inconvenience (zconf.h issue) to the original Makefile build.
# Add zlib
add_subdirectory(zlib)
if ("${CMAKE_CXX_COMPILER_ID};${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "Clang;MSVC") # clang-cl
    target_compile_options(zlib PRIVATE -w)
else()
    target_compile_options(zlib PRIVATE /W0)
endif()
target_include_directories(zlib INTERFACE "zlib" "${CMAKE_CURRENT_BINARY_DIR}/zlib")
target_link_libraries(keeperfx PUBLIC zlib)
target_link_libraries(keeperfx_hvlog PUBLIC zlib)
]]

# Add minizip
set(MINIZIP_SOURCES "zlib/contrib/minizip/ioapi.c"
                    "zlib/contrib/minizip/unzip.c")

add_library(minizip OBJECT ${MINIZIP_SOURCES})
if ("${CMAKE_CXX_COMPILER_ID};${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "Clang;MSVC") # clang-cl
    target_compile_options(minizip PRIVATE -w)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(minizip PRIVATE /W0)
else()
    target_compile_options(minizip PRIVATE -w)
endif()
# minizip requires zlib.
#[[
Do not build zlib from the submodule because it causes inconvenience (zconf.h issue) to the original Makefile build.
target_link_libraries(minizip PUBLIC zlib)
]]
target_link_libraries(minizip PUBLIC ZLIB::ZLIB)
target_include_directories(minizip INTERFACE "zlib/contrib/minizip")
target_link_libraries(keeperfx PRIVATE minizip)
target_link_libraries(keeperfx_hvlog PRIVATE minizip)
