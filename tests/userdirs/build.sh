#!/bin/bash
# Build the user folder tests with any C++ compiler.
#
#   build.sh <output file>
#
# Run from anywhere. Environment:
#   CXX            C++ compiler (default g++), e.g. i686-w64-mingw32-g++
#   CFLAGS_EXTRA   more compile flags, e.g. -fsanitize=address,undefined
#   LDFLAGS_EXTRA  more link flags, e.g. -static
#
# The tests link the folder rules, the file helpers and the migration (src/kfx/platform/UserDirs*.cpp
# except UserDirs.cpp, which is the game's side) and no other game code.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
root="$(cd "$here/../.." && pwd)"
out="$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
cxx="${CXX:-g++}"
cd "$root"
# shellcheck disable=SC2086
$cxx -std=c++17 -O1 -g -Wall -W -Wshadow -Werror -Wno-sign-compare -Wno-unused-parameter -Isrc ${CFLAGS_EXTRA:-} \
    tests/userdirs/kfx_userdirs_tests.cpp \
    src/kfx/platform/UserDirsPolicy.cpp src/kfx/platform/UserDirsFs.cpp src/kfx/platform/UserDirsMigrate.cpp \
    ${LDFLAGS_EXTRA:-} -o "$out"
