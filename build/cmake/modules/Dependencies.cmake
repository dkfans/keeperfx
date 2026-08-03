# Dependencies.cmake - third-party libraries, per platform. Mirrors the hand
# Makefiles: Windows/MinGW uses prebuilt kfx-deps mingw32 static libs + SDL2 dev
# tarballs (../Makefile); Linux uses system/pkg-config libs + a few prebuilt
# lin64 static libs (../linux.mk). Dep URLs/tags track those Makefiles.
#
# Targets are defined here; kfx_link_dependencies(<target>) links them onto the
# game executables (called from BuildTargets, once they exist).

set(KFX_DEPS_BASE "https://github.com/dkfans/kfx-deps/releases/download")

# Downloaded deps live under the BUILD dir, not the source tree, so Windows and
# Linux builds in one checkout get their own deps and never collide.
set(D "${CMAKE_BINARY_DIR}/deps")
set(KFX_CENTITOML_SRC "${CMAKE_SOURCE_DIR}/deps/centitoml")

# kfx_fetch(<dir> <url>): download + extract into <builddir>/deps/<dir>/ once.
function(kfx_fetch dir url)
    set(_tgz "${D}/${dir}.tar.gz")
    set(_dest "${D}/${dir}")
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

macro(kfx_imported name lib incdir)
    add_library(${name} STATIC IMPORTED GLOBAL)
    set_target_properties(${name} PROPERTIES
        IMPORTED_LOCATION "${lib}"
        INTERFACE_INCLUDE_DIRECTORIES "${incdir}")
endmacro()

if(WIN32)
    # --- SDL2 (prebuilt MinGW dev tarballs; each wraps <name>-<ver>/i686-w64-mingw32/*)
    set(SDL2_VER      2.30.7)
    set(SDL2_NET_VER  2.2.0)
    set(SDL2_MIX_VER  2.8.0)
    set(SDL2_IMG_VER  2.8.2)

    kfx_fetch(sdl2       "${KFX_DEPS_BASE}/20260608/SDL2-devel-${SDL2_VER}-mingw.tar.gz")
    kfx_fetch(sdl2_net   "https://github.com/libsdl-org/SDL_net/releases/download/release-${SDL2_NET_VER}/SDL2_net-devel-${SDL2_NET_VER}-mingw.tar.gz")
    kfx_fetch(sdl2_mixer "https://github.com/libsdl-org/SDL_mixer/releases/download/release-${SDL2_MIX_VER}/SDL2_mixer-devel-${SDL2_MIX_VER}-mingw.tar.gz")
    kfx_fetch(sdl2_image "https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMG_VER}/SDL2_image-devel-${SDL2_IMG_VER}-mingw.tar.gz")

    set(SDL2_PREFIX      "${D}/sdl2/SDL2-${SDL2_VER}/i686-w64-mingw32")
    set(SDL2_NET_PREFIX  "${D}/sdl2_net/SDL2_net-${SDL2_NET_VER}/i686-w64-mingw32")
    set(SDL2_MIX_PREFIX  "${D}/sdl2_mixer/SDL2_mixer-${SDL2_MIX_VER}/i686-w64-mingw32")
    set(SDL2_IMG_PREFIX  "${D}/sdl2_image/SDL2_image-${SDL2_IMG_VER}/i686-w64-mingw32")

    add_library(kfx_sdl2 INTERFACE)
    # Code uses both <SDL.h> (include/SDL2) and <SDL2/SDL_mixer.h> (parent include/).
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

    # SDL2 links dynamically, so ship its runtime DLLs (CPack picks these up).
    install(FILES
        "${SDL2_PREFIX}/bin/SDL2.dll"
        "${SDL2_MIX_PREFIX}/bin/SDL2_mixer.dll"
        "${SDL2_NET_PREFIX}/bin/SDL2_net.dll"
        "${SDL2_IMG_PREFIX}/bin/SDL2_image.dll"
        DESTINATION .)

    # --- Static libs from kfx-deps (mirror ../Makefile URLs/tags)
    kfx_fetch(enet6      "${KFX_DEPS_BASE}/20260212/enet6-mingw32.tar.gz")
    kfx_fetch(zlib       "${KFX_DEPS_BASE}/initial/zlib-mingw32.tar.gz")
    kfx_fetch(spng       "${KFX_DEPS_BASE}/initial/spng-mingw32.tar.gz")
    kfx_fetch(astronomy  "${KFX_DEPS_BASE}/astronomy_fix/astronomy-mingw32.tar.gz")
    kfx_fetch(centijson  "${KFX_DEPS_BASE}/initial/centijson-mingw32.tar.gz")
    kfx_fetch(ffmpeg     "${KFX_DEPS_BASE}/initial/ffmpeg-mingw32.tar.gz")
    kfx_fetch(openal     "${KFX_DEPS_BASE}/2024-11-14/openal-mingw32.tar.gz")
    kfx_fetch(luajit     "${KFX_DEPS_BASE}/20250418/luajit-mingw32.tar.gz")
    kfx_fetch(miniupnpc  "${KFX_DEPS_BASE}/20260102/miniupnpc-mingw32.tar.gz")
    kfx_fetch(libnatpmp  "${KFX_DEPS_BASE}/20260102/libnatpmp-mingw32.tar.gz")
    kfx_fetch(libcurl    "${KFX_DEPS_BASE}/20260310/libcurl-mingw32.tar.gz")

    kfx_imported(enet6_static      "${D}/enet6/lib/libenet6.a"       "${D}/enet6/include")
    target_link_libraries(enet6_static INTERFACE ws2_32 winmm)
    kfx_imported(spng_static       "${D}/spng/libspng.a"             "${D}/spng/include")
    kfx_imported(centijson_static  "${D}/centijson/libjson.a"        "${D}/centijson/include")
    kfx_imported(astronomy_static  "${D}/astronomy/libastronomy.a"   "${D}/astronomy/include")
    kfx_imported(zlib_static       "${D}/zlib/libz.a"                "${D}/zlib/include")
    kfx_imported(minizip_static    "${D}/zlib/libminizip.a"          "${D}/zlib/include")
    target_link_libraries(minizip_static INTERFACE zlib_static)
    kfx_imported(openal_static     "${D}/openal/libOpenAL32.a"       "${D}/openal/include")
    target_link_libraries(openal_static INTERFACE winmm ole32 uuid)
    kfx_imported(luajit_static     "${D}/luajit/lib/libluajit.a"     "${D}/luajit/include")
    kfx_imported(miniupnpc_static  "${D}/miniupnpc/libminiupnpc.a"   "${D}/miniupnpc/include")
    target_link_libraries(miniupnpc_static INTERFACE ws2_32 iphlpapi)
    kfx_imported(natpmp_static     "${D}/libnatpmp/libnatpmp.a"      "${D}/libnatpmp/include")
    target_link_libraries(natpmp_static INTERFACE ws2_32 iphlpapi)
    kfx_imported(curl_static       "${D}/libcurl/lib/libcurl.a"      "${D}/libcurl/include")
    target_compile_definitions(curl_static INTERFACE CURL_STATICLIB)
    target_link_libraries(curl_static INTERFACE zlib_static wldap32 crypt32 secur32 bcrypt ws2_32 iphlpapi)

    kfx_imported(libavcodec_static     "${D}/ffmpeg/libavcodec/libavcodec.a"         "${D}/ffmpeg")
    kfx_imported(libavformat_static    "${D}/ffmpeg/libavformat/libavformat.a"       "${D}/ffmpeg")
    kfx_imported(libavutil_static      "${D}/ffmpeg/libavutil/libavutil.a"           "${D}/ffmpeg")
    kfx_imported(libswresample_static  "${D}/ffmpeg/libswresample/libswresample.a"   "${D}/ffmpeg")

    add_library(centitoml OBJECT "${KFX_CENTITOML_SRC}/toml_api.c")
    target_link_libraries(centitoml PUBLIC centijson_static)
    target_include_directories(centitoml INTERFACE "${KFX_CENTITOML_SRC}")

else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2       REQUIRED IMPORTED_TARGET sdl2)
    pkg_check_modules(SDL2_mixer REQUIRED IMPORTED_TARGET SDL2_mixer)
    pkg_check_modules(SDL2_net   REQUIRED IMPORTED_TARGET SDL2_net)
    pkg_check_modules(SDL2_image REQUIRED IMPORTED_TARGET SDL2_image)
    pkg_check_modules(FFMPEG     REQUIRED IMPORTED_TARGET libavformat libavcodec libswresample libavutil)
    pkg_check_modules(OPENAL     REQUIRED IMPORTED_TARGET openal)
    pkg_check_modules(LUAJIT     REQUIRED IMPORTED_TARGET luajit)
    pkg_check_modules(SPNG       REQUIRED IMPORTED_TARGET spng)
    pkg_check_modules(MINIZIP    REQUIRED IMPORTED_TARGET minizip)
    pkg_check_modules(ZLIB       REQUIRED IMPORTED_TARGET zlib)

    # Not reliably packaged; use the prebuilt lin64 static libs (as linux.mk does).
    kfx_fetch(astronomy "${KFX_DEPS_BASE}/20250418/astronomy-lin64.tar.gz")
    kfx_fetch(centijson "${KFX_DEPS_BASE}/20250418/centijson-lin64.tar.gz")
    kfx_fetch(enet6     "${KFX_DEPS_BASE}/20260213/enet6-lin64.tar.gz")
    kfx_fetch(libcurl   "${KFX_DEPS_BASE}/20260310/libcurl-lin64.tar.gz")

    kfx_imported(astronomy_static "${D}/astronomy/libastronomy.a" "${D}/astronomy/include")
    kfx_imported(centijson_static "${D}/centijson/libjson.a"      "${D}/centijson/include")
    kfx_imported(enet6_static     "${D}/enet6/libenet6.a"         "${D}/enet6/include")
    kfx_imported(curl_static      "${D}/libcurl/lib/libcurl.a"    "${D}/libcurl/include")
    target_link_libraries(curl_static INTERFACE ssl crypto zstd)

    add_library(centitoml OBJECT "${KFX_CENTITOML_SRC}/toml_api.c")
    target_link_libraries(centitoml PUBLIC centijson_static)
    target_include_directories(centitoml INTERFACE "${KFX_CENTITOML_SRC}")
endif()

# Link every dependency onto TARGET.
function(kfx_link_dependencies TARGET)
    if(WIN32)
        # Static archives have circular refs (curl<->zlib, ffmpeg internals), so
        # link them in a group (RESCAN == --start-group/--end-group).
        set(_static
            libavformat_static libavcodec_static libswresample_static libavutil_static
            openal_static astronomy_static enet6_static miniupnpc_static natpmp_static
            curl_static spng_static centijson_static minizip_static zlib_static
            luajit_static)
        target_link_libraries(${TARGET} PRIVATE
            kfx_sdl2 "$<LINK_GROUP:RESCAN,${_static}>" centitoml)
    else()
        target_link_libraries(${TARGET} PRIVATE
            PkgConfig::SDL2 PkgConfig::SDL2_mixer PkgConfig::SDL2_net PkgConfig::SDL2_image
            PkgConfig::FFMPEG PkgConfig::OPENAL PkgConfig::LUAJIT
            PkgConfig::SPNG PkgConfig::MINIZIP PkgConfig::ZLIB
            astronomy_static centijson_static enet6_static curl_static
            centitoml
            miniupnpc natpmp dl)
    endif()
endfunction()
