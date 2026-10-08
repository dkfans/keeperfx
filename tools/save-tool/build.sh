#!/bin/bash
# Build kfx-savetool with the host compiler.
#
#   build.sh <output file>
#
# Run from the repository root. Needs gcc, g++ and zlib. The field tables include the game's struct
# headers, which are compiled but not linked, so the SDL headers are fetched the same way
# tools/save-layout does it.
#
# Only the portable codec (src/kfx/save/core) is linked, not the save manager, which uses the game's
# file and log functions.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
root="$(cd "$here/../.." && pwd)"
out="$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT

if [ -z "${SDL_INCLUDES:-}" ]; then
    SDL_INCLUDES="$(bash "$root/tools/save-layout/fetch_sdl_headers.sh" "$root" "$work/sdl")"
fi

cd "$root"
includes="-Itools/save-layout/stub -Itools/save-tool -Isrc -Ideps/centitoml $SDL_INCLUDES"
defs="-DBFDEBUG_LEVEL=0 -DDEBUG=0"
mkdir -p "$work/obj"
# shellcheck disable=SC2086
for f in tools/save-tool/*.c src/kfx/save/core/schema/*.c; do
    gcc -std=gnu11 -O2 -w $includes $defs -c "$f" -o "$work/obj/$(echo "$f" | tr / _).o"
done
# shellcheck disable=SC2086
for f in src/kfx/save/core/*.cpp; do
    g++ -std=c++17 -O2 -w $includes $defs -c "$f" -o "$work/obj/$(echo "$f" | tr / _).o"
done
g++ "$work"/obj/*.o -lz -o "$out"
