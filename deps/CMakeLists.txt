cmake_minimum_required(VERSION 3.20)

# ---------------------------------------------------------------------------
# Prebuilt MinGW-w64 (i686) dependencies.
#
# These mirror EXACTLY the tarballs the hand Makefile downloads (see the
# `deps/<name>-mingw32.tar.gz` rules in ../Makefile), so a CMake build links
# the same binary artifacts as the authoritative Makefile build. Tarballs are
# extracted into deps/<name>/ with the same on-disk layout the Makefile uses,
# i.e. the include/ and lib/ paths referenced below match the Makefile's
# -I / -L flags one-for-one.
#
# URLs/tags are kept in lockstep with ../Makefile. If you bump a dep there,
# bump it here too.
# ---------------------------------------------------------------------------

set(KFX_DEPS_BASE "https://github.com/dkfans/kfx-deps/releases/download")

# fetch_dep(<dir> <url>): download <url> to deps/<dir>.tar.gz (once) and extract
# it into deps/<dir>/ (once).
function(fetch_dep dir url)
    set(_tgz "${CMAKE_CURRENT_SOURCE_DIR}/${dir}-mingw32.tar.gz")
    set(_dest "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
    if(NOT EXISTS "${_dest}")
        file(MAKE_DIRECTORY "${_dest}")
        if(NOT EXISTS "${_tgz}")
            message(STATUS "Downloading dep: ${dir}  <-  ${url}")
            file(DOWNLOAD "${url}" "${_tgz}" SHOW_PROGRESS STATUS _st)
            list(GET _st 0 _code)
            if(NOT _code EQUAL 0)
                message(FATAL_ERROR "Failed to download ${url}: ${_st}")
            endif()
        endif()
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xzf "${_tgz}"
            WORKING_DIRECTORY "${_dest}"
            RESULT_VARIABLE _rc)
        if(NOT _rc EQUAL 0)
            message(FATAL_ERROR "Failed to extract ${_tgz}")
        endif()
    endif()
endfunction()

# --- SDL2 (prebuilt MinGW dev tarballs, same versions as prebuilds.mk) -------
# SDL2 core comes from kfx-deps; the extension libs from libsdl-org. Each dev
# tarball wraps its content in <name>-<ver>/i686-w64-mingw32/{bin,include,lib},
# so we locate that prefix after extraction rather than hardcoding versions.
function(fetch_sdl dir url)
    set(_tgz "${CMAKE_CURRENT_SOURCE_DIR}/${dir}.tar.gz")
    set(_dest "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
    if(NOT EXISTS "${_dest}")
        file(MAKE_DIRECTORY "${_dest}")
        if(NOT EXISTS "${_tgz}")
            message(STATUS "Downloading SDL dep: ${dir}  <-  ${url}")
            file(DOWNLOAD "${url}" "${_tgz}" SHOW_PROGRESS STATUS _st)
            list(GET _st 0 _code)
            if(NOT _code EQUAL 0)
                message(FATAL_ERROR "Failed to download ${url}: ${_st}")
            endif()
        endif()
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xzf "${_tgz}"
            WORKING_DIRECTORY "${_dest}"
            RESULT_VARIABLE _rc)
        if(NOT _rc EQUAL 0)
            message(FATAL_ERROR "Failed to extract ${_tgz}")
        endif()
    endif()
endfunction()

set(SDL2_VER      2.30.7)
set(SDL2_NET_VER  2.2.0)
set(SDL2_MIX_VER  2.8.0)
set(SDL2_IMG_VER  2.8.2)

fetch_sdl(sdl2       "${KFX_DEPS_BASE}/20260608/SDL2-devel-${SDL2_VER}-mingw.tar.gz")
fetch_sdl(sdl2_net   "https://github.com/libsdl-org/SDL_net/releases/download/release-${SDL2_NET_VER}/SDL2_net-devel-${SDL2_NET_VER}-mingw.tar.gz")
fetch_sdl(sdl2_mixer "https://github.com/libsdl-org/SDL_mixer/releases/download/release-${SDL2_MIX_VER}/SDL2_mixer-devel-${SDL2_MIX_VER}-mingw.tar.gz")
fetch_sdl(sdl2_image "https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMG_VER}/SDL2_image-devel-${SDL2_IMG_VER}-mingw.tar.gz")

set(SDL2_PREFIX      "${CMAKE_CURRENT_SOURCE_DIR}/sdl2/SDL2-${SDL2_VER}/i686-w64-mingw32")
set(SDL2_NET_PREFIX  "${CMAKE_CURRENT_SOURCE_DIR}/sdl2_net/SDL2_net-${SDL2_NET_VER}/i686-w64-mingw32")
set(SDL2_MIX_PREFIX  "${CMAKE_CURRENT_SOURCE_DIR}/sdl2_mixer/SDL2_mixer-${SDL2_MIX_VER}/i686-w64-mingw32")
set(SDL2_IMG_PREFIX  "${CMAKE_CURRENT_SOURCE_DIR}/sdl2_image/SDL2_image-${SDL2_IMG_VER}/i686-w64-mingw32")

# --- Static libs from kfx-deps (mirror ../Makefile URLs/tags exactly) -------
fetch_dep(enet6      "${KFX_DEPS_BASE}/20260212/enet6-mingw32.tar.gz")
fetch_dep(zlib       "${KFX_DEPS_BASE}/initial/zlib-mingw32.tar.gz")
fetch_dep(spng       "${KFX_DEPS_BASE}/initial/spng-mingw32.tar.gz")
fetch_dep(astronomy  "${KFX_DEPS_BASE}/astronomy_fix/astronomy-mingw32.tar.gz")
fetch_dep(centijson  "${KFX_DEPS_BASE}/initial/centijson-mingw32.tar.gz")
fetch_dep(ffmpeg     "${KFX_DEPS_BASE}/initial/ffmpeg-mingw32.tar.gz")
fetch_dep(openal     "${KFX_DEPS_BASE}/2024-11-14/openal-mingw32.tar.gz")
fetch_dep(luajit     "${KFX_DEPS_BASE}/20250418/luajit-mingw32.tar.gz")
fetch_dep(miniupnpc  "${KFX_DEPS_BASE}/20260102/miniupnpc-mingw32.tar.gz")
fetch_dep(libnatpmp  "${KFX_DEPS_BASE}/20260102/libnatpmp-mingw32.tar.gz")
fetch_dep(libcurl    "${KFX_DEPS_BASE}/20260310/libcurl-mingw32.tar.gz")

set(D "${CMAKE_CURRENT_SOURCE_DIR}")

# ---------------------------------------------------------------------------
# Imported targets. Each carries its own include dir + any Windows system libs
# it needs, matching the Makefile's LINKLIB. Link order is made robust with a
# linker group in the parent CMakeLists (static archives have circular refs,
# e.g. curl<->zlib), so we do not rely on declaration order here.
# ---------------------------------------------------------------------------

## SDL2 (dynamic import libs; SDL2*.dll shipped alongside the exe)
add_library(kfx_sdl2 INTERFACE)
# The code includes both <SDL.h> (via include/SDL2) and <SDL2/SDL_mixer.h> etc.
# (via the parent include/), so expose both levels for core and each extension.
target_include_directories(kfx_sdl2 INTERFACE
    "${SDL2_PREFIX}/include" "${SDL2_PREFIX}/include/SDL2"
    "${SDL2_NET_PREFIX}/include"
    "${SDL2_MIX_PREFIX}/include"
    "${SDL2_IMG_PREFIX}/include")
target_link_libraries(kfx_sdl2 INTERFACE
    "${SDL2_PREFIX}/lib/libSDL2.dll.a"
    "${SDL2_MIX_PREFIX}/lib/libSDL2_mixer.dll.a"
    "${SDL2_NET_PREFIX}/lib/libSDL2_net.dll.a"
    "${SDL2_IMG_PREFIX}/lib/libSDL2_image.dll.a")

## enet6
add_library(enet6_static STATIC IMPORTED GLOBAL)
set_target_properties(enet6_static PROPERTIES
    IMPORTED_LOCATION "${D}/enet6/lib/libenet6.a"
    INTERFACE_INCLUDE_DIRECTORIES "${D}/enet6/include")
target_link_libraries(enet6_static INTERFACE ws2_32 winmm)

## spng
add_library(spng_static STATIC IMPORTED GLOBAL)
set_target_properties(spng_static PROPERTIES
    IMPORTED_LOCATION "${D}/spng/libspng.a"
    INTERFACE_INCLUDE_DIRECTORIES "${D}/spng/include")

## centijson
add_library(centijson_static STATIC IMPORTED GLOBAL)
set_target_properties(centijson_static PROPERTIES
    IMPORTED_LOCATION "${D}/centijson/libjson.a"
    INTERFACE_INCLUDE_DIRECTORIES "${D}/centijson/include")

## astronomy
add_library(astronomy_static STATIC IMPORTED GLOBAL)
set_target_properties(astronomy_static PROPERTIES
    IMPORTED_LOCATION "${D}/astronomy/libastronomy.a"
    INTERFACE_INCLUDE_DIRECTORIES "${D}/astronomy/include")

## zlib
add_library(zlib_static STATIC IMPORTED GLOBAL)
set_target_properties(zlib_static PROPERTIES
    IMPORTED_LOCATION "${D}/zlib/libz.a"
    INTERFACE_INCLUDE_DIRECTORIES "${D}/zlib/include")

## minizip (from the zlib tarball)
add_library(minizip_static STATIC IMPORTED GLOBAL)
set_target_properties(minizip_static PROPERTIES
    IMPORTED_LOCATION "${D}/zlib/libminizip.a"
    INTERFACE_INCLUDE_DIRECTORIES "${D}/zlib/include")
target_link_libraries(minizip_static INTERFACE zlib_static)

## openal (static OpenAL32; Makefile links -lwinmm -lole32 -luuid globally)
add_library(openal_static STATIC IMPORTED GLOBAL)
set_target_properties(openal_static PROPERTIES
    IMPORTED_LOCATION "${D}/openal/libOpenAL32.a"
    INTERFACE_INCLUDE_DIRECTORIES "${D}/openal/include")
target_link_libraries(openal_static INTERFACE winmm ole32 uuid)

## luajit
add_library(luajit_static STATIC IMPORTED GLOBAL)
set_target_properties(luajit_static PROPERTIES
    IMPORTED_LOCATION "${D}/luajit/lib/libluajit.a"
    INTERFACE_INCLUDE_DIRECTORIES "${D}/luajit/include")

## miniupnpc
add_library(miniupnpc_static STATIC IMPORTED GLOBAL)
set_target_properties(miniupnpc_static PROPERTIES
    IMPORTED_LOCATION "${D}/miniupnpc/libminiupnpc.a"
    INTERFACE_INCLUDE_DIRECTORIES "${D}/miniupnpc/include")
target_link_libraries(miniupnpc_static INTERFACE ws2_32 iphlpapi)

## libnatpmp (Makefile: -lnatpmp -liphlpapi)
add_library(natpmp_static STATIC IMPORTED GLOBAL)
set_target_properties(natpmp_static PROPERTIES
    IMPORTED_LOCATION "${D}/libnatpmp/libnatpmp.a"
    INTERFACE_INCLUDE_DIRECTORIES "${D}/libnatpmp/include")
target_link_libraries(natpmp_static INTERFACE ws2_32 iphlpapi)

## libcurl (static; Makefile: -lcurl -lwldap32 -lcrypt32 -lsecur32 -liphlpapi
## plus global -lbcrypt -lws2_32). Consumers compile with -DCURL_STATICLIB.
add_library(curl_static STATIC IMPORTED GLOBAL)
set_target_properties(curl_static PROPERTIES
    IMPORTED_LOCATION "${D}/libcurl/lib/libcurl.a"
    INTERFACE_INCLUDE_DIRECTORIES "${D}/libcurl/include")
target_compile_definitions(curl_static INTERFACE CURL_STATICLIB)
target_link_libraries(curl_static INTERFACE
    zlib_static wldap32 crypt32 secur32 bcrypt ws2_32 iphlpapi)

## ffmpeg
add_library(libavcodec_static STATIC IMPORTED GLOBAL)
set_target_properties(libavcodec_static PROPERTIES
    IMPORTED_LOCATION "${D}/ffmpeg/libavcodec/libavcodec.a"
    INTERFACE_INCLUDE_DIRECTORIES "${D}/ffmpeg")
add_library(libavformat_static STATIC IMPORTED GLOBAL)
set_target_properties(libavformat_static PROPERTIES
    IMPORTED_LOCATION "${D}/ffmpeg/libavformat/libavformat.a"
    INTERFACE_INCLUDE_DIRECTORIES "${D}/ffmpeg")
add_library(libavutil_static STATIC IMPORTED GLOBAL)
set_target_properties(libavutil_static PROPERTIES
    IMPORTED_LOCATION "${D}/ffmpeg/libavutil/libavutil.a"
    INTERFACE_INCLUDE_DIRECTORIES "${D}/ffmpeg")
add_library(libswresample_static STATIC IMPORTED GLOBAL)
set_target_properties(libswresample_static PROPERTIES
    IMPORTED_LOCATION "${D}/ffmpeg/libswresample/libswresample.a"
    INTERFACE_INCLUDE_DIRECTORIES "${D}/ffmpeg")

## centitoml (compiled from source in-tree; depends on centijson)
add_library(centitoml OBJECT "centitoml/toml_api.c")
target_link_libraries(centitoml PUBLIC centijson_static)
target_include_directories(centitoml INTERFACE "centitoml")

# The concrete static archives are linked by the parent inside a linker group
# (see ../CMakeLists.txt, KFX_STATIC_DEPS) so their circular references resolve
# regardless of order. Each target above already propagates its include dirs
# and any Windows system libs it needs as usage requirements.
