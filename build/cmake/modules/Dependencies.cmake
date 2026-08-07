# Dependencies.cmake - third-party libraries, per platform. Mirrors the hand
# Makefiles: Windows/MinGW uses prebuilt kfx-deps mingw32 static libs + SDL3 dev
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
    # --- SDL3 (prebuilt MinGW dev tarballs; each wraps <name>-<ver>/i686-w64-mingw32/*)
    # SDL_net is intentionally absent: api.c now uses a native Winsock socket
    # layer (SDL3_net is not reliably packaged). See docs SDL3-MIGRATION notes.
    set(SDL3_VER      3.4.12)
    set(SDL3_MIX_VER  3.2.4)
    set(SDL3_IMG_VER  3.4.4)

    kfx_fetch(sdl3       "https://github.com/libsdl-org/SDL/releases/download/release-${SDL3_VER}/SDL3-devel-${SDL3_VER}-mingw.tar.gz")
    kfx_fetch(sdl3_mixer "https://github.com/libsdl-org/SDL_mixer/releases/download/release-${SDL3_MIX_VER}/SDL3_mixer-devel-${SDL3_MIX_VER}-mingw.tar.gz")
    kfx_fetch(sdl3_image "https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL3_IMG_VER}/SDL3_image-devel-${SDL3_IMG_VER}-mingw.tar.gz")

    set(SDL3_PREFIX      "${D}/sdl3/SDL3-${SDL3_VER}/i686-w64-mingw32")
    set(SDL3_MIX_PREFIX  "${D}/sdl3_mixer/SDL3_mixer-${SDL3_MIX_VER}/i686-w64-mingw32")
    set(SDL3_IMG_PREFIX  "${D}/sdl3_image/SDL3_image-${SDL3_IMG_VER}/i686-w64-mingw32")

    add_library(kfx_sdl3 INTERFACE)
    # SDL3 headers live under include/SDL3/*.h; code uses <SDL3/SDL.h>,
    # <SDL3_mixer/SDL_mixer.h>, <SDL3_image/SDL_image.h>.
    target_include_directories(kfx_sdl3 INTERFACE
        "${SDL3_PREFIX}/include"
        "${SDL3_MIX_PREFIX}/include"
        "${SDL3_IMG_PREFIX}/include")
    target_link_libraries(kfx_sdl3 INTERFACE
        "${SDL3_PREFIX}/lib/libSDL3.dll.a"
        "${SDL3_MIX_PREFIX}/lib/libSDL3_mixer.dll.a"
        "${SDL3_IMG_PREFIX}/lib/libSDL3_image.dll.a")

    # SDL3 links dynamically, so ship its runtime DLLs (CPack picks these up).
    install(FILES
        "${SDL3_PREFIX}/bin/SDL3.dll"
        "${SDL3_MIX_PREFIX}/bin/SDL3_mixer.dll"
        "${SDL3_IMG_PREFIX}/bin/SDL3_image.dll"
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

    add_library(kfx_sdl3 INTERFACE)
    pkg_check_modules(SDL3       IMPORTED_TARGET sdl3)
    pkg_check_modules(SDL3_image IMPORTED_TARGET SDL3_image)
    pkg_check_modules(SDL3_mixer IMPORTED_TARGET SDL3_mixer)
    if(SDL3_FOUND AND SDL3_image_FOUND AND SDL3_mixer_FOUND)
        message(STATUS "SDL3: using system libraries (pkg-config)")
        target_link_libraries(kfx_sdl3 INTERFACE
            PkgConfig::SDL3 PkgConfig::SDL3_image PkgConfig::SDL3_mixer)
    else()
        message(STATUS "SDL3: system libraries not found; building from source (FetchContent)")
        include(FetchContent)
        set(SDL3_VER      3.4.12)
        set(SDL3_MIX_VER  3.2.4)
        set(SDL3_IMG_VER  3.4.4)
        # Shared libs, no tests/examples. Use system decoder libraries rather than
        # vendored ones: the release source tarballs do not bundle the external/
        # decoder submodules, so VENDORED would fail. Distros that hit this path
        # need libpng + the ogg/vorbis/flac/mpg123 -dev packages (see CI).
        set(SDL_TEST_LIBRARY   OFF CACHE BOOL "" FORCE)
        set(SDL_EXAMPLES       OFF CACHE BOOL "" FORCE)
        set(SDLIMAGE_SAMPLES   OFF CACHE BOOL "" FORCE)
        set(SDLIMAGE_VENDORED  OFF CACHE BOOL "" FORCE)
        set(SDLMIXER_SAMPLES   OFF CACHE BOOL "" FORCE)
        set(SDLMIXER_VENDORED  OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(SDL3
            URL "https://github.com/libsdl-org/SDL/releases/download/release-${SDL3_VER}/SDL3-${SDL3_VER}.tar.gz")
        FetchContent_Declare(SDL3_image
            URL "https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL3_IMG_VER}/SDL3_image-${SDL3_IMG_VER}.tar.gz")
        FetchContent_Declare(SDL3_mixer
            URL "https://github.com/libsdl-org/SDL_mixer/releases/download/release-${SDL3_MIX_VER}/SDL3_mixer-${SDL3_MIX_VER}.tar.gz")
        FetchContent_MakeAvailable(SDL3 SDL3_image SDL3_mixer)
        target_link_libraries(kfx_sdl3 INTERFACE
            SDL3::SDL3 SDL3_image::SDL3_image SDL3_mixer::SDL3_mixer)
    endif()

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
            kfx_sdl3 "$<LINK_GROUP:RESCAN,${_static}>" centitoml)
    else()
        target_link_libraries(${TARGET} PRIVATE
            kfx_sdl3
            PkgConfig::FFMPEG PkgConfig::OPENAL PkgConfig::LUAJIT
            PkgConfig::SPNG PkgConfig::MINIZIP PkgConfig::ZLIB
            astronomy_static centijson_static enet6_static curl_static
            centitoml
            miniupnpc natpmp dl)
    endif()
endfunction()
