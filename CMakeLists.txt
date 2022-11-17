cmake_minimum_required(VERSION 3.15)

# Do not allow building in root
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Do not build in the root. Create a bin directory and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif ()

project(keeperfx C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

set(VER_MAJOR       0)
set(VER_MINOR       4)
set(VER_RELEASE     9)
set(VER_BUILD       2762)
set(VER_STRING      "${VER_MAJOR}.${VER_MINOR}.${VER_RELEASE}.${VER_BUILD} ${PACKAGE_SUFFIX}")
set(PACKAGE_SUFFIX  "Prototype")
set(GIT_REVISION    "HEAD")

set(KEEPERFX_VER_DEFS_H_IN ${CMAKE_SOURCE_DIR}/ver_defs.h.in)
set(KEEPERFX_VER_DEFS_H_OUT ${CMAKE_SOURCE_DIR}/obj/ver_defs.h)
configure_file(${KEEPERFX_VER_DEFS_H_IN} ${KEEPERFX_VER_DEFS_H_OUT})

if (MSVC)
    find_package(unofficial-enet CONFIG REQUIRED)
    find_package(SDL2 REQUIRED CONFIG)
    find_package(SDL2_mixer CONFIG REQUIRED)
    find_package(SDL2_net CONFIG REQUIRED)
    find_package(ZLIB REQUIRED)

    set(OUR_C_FLAGS "/w")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OUR_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OUR_C_FLAGS}")

    add_compile_definitions(_CRT_NONSTDC_NO_WARNINGS _CRT_SECURE_NO_WARNINGS
                            KEEPERSPEECH_STATIC
                            SPNG_STATIC)

    file(GLOB_RECURSE KEEPERFX_SOURCES_C "src/*.c")
    file(GLOB_RECURSE KEEPERFX_SOURCES_CXX "src/*.cpp")
    file(GLOB_RECURSE LIBSPNG_SOURCES "deps/libspng/spng/*.c")
    file(GLOB_RECURSE CENTIJSON_SOURCES "deps/centijson/src/*.c")
    file(GLOB_RECURSE MINIZIP_SOURCES "deps/zlib/contrib/minizip/ioapi.c"
                                    "deps/zlib/contrib/minizip/unzip.c")

    # We will use precompiled obj instead to workaround incompatible assembly
    list(REMOVE_ITEM KEEPERFX_SOURCES_C "${CMAKE_SOURCE_DIR}/src/bflib_render_gpoly.c")

    add_executable(keeperfx ${KEEPERFX_SOURCES_C} ${KEEPERFX_SOURCES_CXX}
                            ${LIBSPNG_SOURCES}
                            ${CENTIJSON_SOURCES}
                            ${MINIZIP_SOURCES})

    # Original code import library
    target_link_libraries(keeperfx PRIVATE "${CMAKE_SOURCE_DIR}/lib/keeperfx.lib")
    target_link_libraries(keeperfx PRIVATE "${CMAKE_SOURCE_DIR}/lib/bflib_render_gpoly.obj")

    # System libraries
    target_link_libraries(keeperfx PRIVATE imagehlp)

    target_include_directories(keeperfx PRIVATE "${CMAKE_SOURCE_DIR}/deps/centijson/src")
    target_include_directories(keeperfx PRIVATE "${CMAKE_SOURCE_DIR}/deps/libspng/spng")

    target_link_libraries(keeperfx
        PRIVATE unofficial::enet::enet)
    target_link_libraries(keeperfx
        PRIVATE
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>)
    target_link_libraries(keeperfx
        PRIVATE $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>)
    target_link_libraries(keeperfx
        PRIVATE $<IF:$<TARGET_EXISTS:SDL2_net::SDL2_net>,SDL2_net::SDL2_net,SDL2_net::SDL2_net-static>)
    target_link_libraries(keeperfx PRIVATE ZLIB::ZLIB)

else ()
    find_package(SDL2 REQUIRED CONFIG)
    find_package(SDL2_mixer CONFIG REQUIRED)
    find_package(SDL2_net CONFIG REQUIRED)
    find_package(ZLIB REQUIRED)

    set(OUR_C_FLAGS "-w")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OUR_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OUR_C_FLAGS}")

    file(GLOB_RECURSE KEEPERFX_SOURCES_C "src/*.c")
    file(GLOB_RECURSE KEEPERFX_SOURCES_CXX "src/*.cpp")
    file(GLOB_RECURSE LIBSPNG_SOURCES "deps/libspng/spng/*.c")
    file(GLOB_RECURSE CENTIJSON_SOURCES "deps/centijson/src/*.c")
    file(GLOB_RECURSE MINIZIP_SOURCES "deps/zlib/contrib/minizip/ioapi.c"
                                      "deps/zlib/contrib/minizip/unzip.c")

    add_compile_definitions(KEEPERSPEECH_STATIC
                            SPNG_STATIC)

    add_executable(keeperfx ${KEEPERFX_SOURCES_C} ${KEEPERFX_SOURCES_CXX}
                            ${LIBSPNG_SOURCES}
                            ${CENTIJSON_SOURCES}
                            ${MINIZIP_SOURCES})

    target_include_directories(keeperfx PRIVATE "${CMAKE_SOURCE_DIR}/deps/centijson/src")
    target_include_directories(keeperfx PRIVATE "${CMAKE_SOURCE_DIR}/deps/libspng/spng")

    target_link_libraries(keeperfx PRIVATE enet)
    target_link_libraries(keeperfx
        PRIVATE
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>)
    target_link_libraries(keeperfx
        PRIVATE $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>)
    target_link_libraries(keeperfx
        PRIVATE $<IF:$<TARGET_EXISTS:SDL2_net::SDL2_net>,SDL2_net::SDL2_net,SDL2_net::SDL2_net-static>)
    target_link_libraries(keeperfx PRIVATE ZLIB::ZLIB)
endif ()
