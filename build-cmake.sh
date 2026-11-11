#!/usr/bin/env bash
#
# Build keeperfx with CMake, at parity with the hand Makefiles.
#
#   KFX_OS=windows (default)  32-bit MinGW-w64 (i686) Windows binary, using the
#                             same compiler/flags/prebuilt deps as `make`.
#   KFX_OS=linux              native x86_64 Linux ELF (pkg-config deps + prebuilt lin64 static libs)
#                             (system/pkg-config deps + prebuilt lin64 static libs).
#
# Third-party deps are downloaded automatically on first run (into deps/); the
# Windows build needs no vcpkg, the Linux build needs the usual -dev packages.
#
# Usage:
#   ./build-cmake.sh                     # Windows keeperfx (standard log)
#   KFX_OS=linux ./build-cmake.sh        # Linux keeperfx
#   ./build-cmake.sh keeperfx_hvlog      # heavy-log variant
#   USE_DOCKER=1 ./build-cmake.sh        # build in an Ubuntu 24.04 container
#   BUILD_DIR=out ./build-cmake.sh       # override the build directory
#
# Requirements (native):
#   windows: a MinGW-w64 i686 toolchain (Ubuntu: g++-mingw-w64-i686), cmake, ninja
#   linux:   gcc/g++, cmake, ninja, pkg-config + the ffmpeg/openal/luajit/
#            spng/minizip/zlib/miniupnpc/natpmp/openssl/zstd -dev packages, plus
#            SDL3's build deps (X11/GL/audio + png/ogg/vorbis/flac/mpg123/opus)
#            since SDL3 is built from source on distros without libsdl3-dev.
#
set -euo pipefail

TARGET="${1:-keeperfx}"
KFX_OS="${KFX_OS:-windows}"
# Output goes to out/ (git-ignored); build/ holds tracked CMake modules.
BUILD_DIR="${BUILD_DIR:-out}"

# Run the whole thing inside a container that mirrors upstream CI (Ubuntu 24.04).
if [ "${USE_DOCKER:-0}" = "1" ]; then
    if [ "$KFX_OS" = "linux" ]; then
        PKGS="build-essential pkg-config cmake ninja-build git curl ca-certificates \
              libavcodec-dev libavformat-dev libavutil-dev libswresample-dev \
              libopenal-dev libluajit-5.1-dev libspng-dev libminizip-dev zlib1g-dev \
              libminiupnpc-dev libnatpmp-dev libssl-dev libzstd-dev \
              libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev \
              libxfixes-dev libxss-dev libxtst-dev libxkbcommon-dev libdrm-dev \
              libgbm-dev libgl1-mesa-dev libegl1-mesa-dev libasound2-dev \
              libpulse-dev libdbus-1-dev libudev-dev \
              libpng-dev libjpeg-dev libogg-dev libvorbis-dev libflac-dev \
              libmpg123-dev libopusfile-dev"
    else
        PKGS="g++-mingw-w64-i686 cmake ninja-build git curl ca-certificates"
    fi
    exec docker run --rm -v "$PWD:/src" -w /src ubuntu:24.04 bash -c "
        set -eux
        export DEBIAN_FRONTEND=noninteractive
        apt-get update -qq
        apt-get install -y -qq $PKGS
        git config --global --add safe.directory /src || true
        KFX_OS='$KFX_OS' BUILD_DIR='$BUILD_DIR' bash build-cmake.sh '$TARGET'
    "
fi

if [ "$KFX_OS" = "linux" ]; then
    cmake -S . -B "$BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
else
    if ! command -v i686-w64-mingw32-gcc >/dev/null 2>&1; then
        echo "error: i686-w64-mingw32-gcc not found on PATH." >&2
        echo "       Install a MinGW-w64 i686 toolchain (Ubuntu: 'sudo apt install g++-mingw-w64-i686')," >&2
        echo "       or re-run with USE_DOCKER=1, or KFX_OS=linux for a native Linux build." >&2
        exit 1
    fi
    cmake -S . -B "$BUILD_DIR" -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE=build/cmake/toolchains/mingw32.cmake \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo
fi

cmake --build "$BUILD_DIR" --target "$TARGET" -j"$(nproc 2>/dev/null || echo 4)"

echo
if [ "$KFX_OS" = "linux" ]; then
    echo "Built: $BUILD_DIR/$TARGET"
else
    echo "Built: $BUILD_DIR/$TARGET.exe"
fi
