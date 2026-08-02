#!/usr/bin/env bash
#
# Build keeperfx.exe with CMake, at parity with the Makefile build.
#
# Produces a 32-bit MinGW-w64 (i686) Windows binary using the same compiler,
# flags and prebuilt dependencies as `make standard`. The dependencies are
# downloaded automatically on first run (into deps/), so no vcpkg is required.
#
# Usage:
#   ./build-cmake.sh                     # build keeperfx        (standard log)
#   ./build-cmake.sh keeperfx_hvlog      # build keeperfx_hvlog  (heavy log)
#   USE_DOCKER=1 ./build-cmake.sh        # build in an Ubuntu 24.04 container,
#                                        #   no local MinGW toolchain needed
#   BUILD_DIR=out ./build-cmake.sh       # override the build directory
#
# Requirements (native build): a MinGW-w64 i686 toolchain (Debian/Ubuntu:
# `g++-mingw-w64-i686`; MSYS2: the mingw32 toolchain), plus cmake and ninja.
#
set -euo pipefail

# Output goes to out/ (git-ignored); build/ holds tracked CMake modules.
TARGET="${1:-keeperfx}"
BUILD_DIR="${BUILD_DIR:-out}"

# Run the whole thing inside a container that mirrors upstream CI (Ubuntu 24.04
# + g++-mingw-w64-i686 == GCC 13 MinGW). Handy for testers without a toolchain.
if [ "${USE_DOCKER:-0}" = "1" ]; then
    exec docker run --rm -v "$PWD:/src" -w /src ubuntu:24.04 bash -c "
        set -eux
        export DEBIAN_FRONTEND=noninteractive
        apt-get update -qq
        apt-get install -y -qq g++-mingw-w64-i686 cmake ninja-build git curl ca-certificates
        git config --global --add safe.directory /src || true
        BUILD_DIR='$BUILD_DIR' bash build-cmake.sh '$TARGET'
    "
fi

if ! command -v i686-w64-mingw32-gcc >/dev/null 2>&1; then
    echo "error: i686-w64-mingw32-gcc not found on PATH." >&2
    echo "       Install a MinGW-w64 i686 toolchain (Ubuntu: 'sudo apt install g++-mingw-w64-i686')," >&2
    echo "       or re-run with USE_DOCKER=1 to build in a container." >&2
    exit 1
fi

cmake -S . -B "$BUILD_DIR" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=mingw32.cmake \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo

cmake --build "$BUILD_DIR" --target "$TARGET" -j"$(nproc 2>/dev/null || echo 4)"

echo
echo "Built: $BUILD_DIR/$TARGET.exe"
