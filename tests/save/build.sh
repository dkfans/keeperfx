#!/bin/bash
# Build the save tests with any C/C++ compiler pair.
#
#   build.sh <output file>
#
# Writes <output file> (the codec tests) and <output file>_records (the file tests; with a .exe
# suffix it goes before the suffix). Run from anywhere. Environment:
#   CC             C compiler (default gcc), e.g. i686-w64-mingw32-gcc or aarch64-linux-gnu-gcc
#   CXX            C++ compiler (default: CC with gcc replaced by g++)
#   CFLAGS_EXTRA   more compile flags, e.g. -m32 or -fsanitize=address,undefined
#   LDFLAGS_EXTRA  more link flags, e.g. -static
#   SDL_INCLUDES   -I flags for the SDL3 headers; fetched when not set
#
# The tests link the codec (src/kfx/save/core, C++17), the field tables (C) and the digest walker. They include
# the game's struct headers (SDL headers are needed at compile time only) and no game code.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
root="$(cd "$here/../.." && pwd)"
out="$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
case "$out" in
    *.exe) records_out="${out%.exe}_records.exe" ;;
    *) records_out="${out}_records" ;;
esac
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT

if [ -z "${SDL_INCLUDES:-}" ]; then
    SDL_INCLUDES="$(bash "$root/tools/save-layout/fetch_sdl_headers.sh" "$root" "$work/sdl")"
fi

cc="${CC:-gcc}"
cxx="${CXX:-${cc%gcc}g++}"
cd "$root"
mkdir -p "$work/core" "$work/tests"

defs="-DBFDEBUG_LEVEL=0 -DDEBUG=0"
includes="-Itools/save-layout/stub -Itools/save-tool -Isrc -Itests/save -Ideps/centitoml $SDL_INCLUDES"
# shellcheck disable=SC2086
compile_c() { $cc -std=gnu11 -O1 -g -w $includes $defs ${CFLAGS_EXTRA:-} -c "$1" -o "$2"; }

# shellcheck disable=SC2086
compile_cxx() { $cxx -std=c++17 -O1 -g -Wall -W -Wshadow -Wno-sign-compare -Wno-unused-parameter $includes $defs ${CFLAGS_EXTRA:-} -c "$1" -o "$2"; }

for f in src/kfx/save/core/*.cpp; do
    compile_cxx "$f" "$work/core/$(basename "$f").o"
done
for f in src/kfx/save/core/schema/*.c; do
    compile_c "$f" "$work/core/schema_$(basename "$f").o"
done
for f in tests/save/kfx_save_tests.c tools/save-tool/save_digest.c tools/save-tool/save_validate.c; do
    compile_c "$f" "$work/tests/$(basename "$f").o"
done

# shellcheck disable=SC2086
$cxx ${CFLAGS_EXTRA:-} "$work"/tests/*.o "$work"/core/*.o ${LDFLAGS_EXTRA:-} -lz -o "$out"
# shellcheck disable=SC2086
$cxx -std=c++17 -O1 -g -w $includes $defs ${CFLAGS_EXTRA:-} \
    tests/save/kfx_record_file_tests.cpp src/kfx/save/SaveRecordFiles.cpp "$work"/core/*.o \
    ${LDFLAGS_EXTRA:-} -lz -o "$records_out"
