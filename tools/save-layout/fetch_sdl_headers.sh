#!/bin/bash
# Download the SDL3 and SDL3_mixer MinGW headers that the saved-struct headers
# include. Versions are read from build/cmake/modules/Dependencies.cmake so
# they always match the real build.
#
#   fetch_sdl_headers.sh <source root> <dest dir>
#
# Prints the -I flags to use as SDL_INCLUDES.
set -euo pipefail

src="$1"
dest="$2"
deps="$src/build/cmake/modules/Dependencies.cmake"

ver() {
    tr -d '\r' < "$deps" | sed -n "s/^ *set($1 *\([0-9.]*\))/\1/p" | head -1
}
sdl3="$(ver SDL3_VER)"
mix="$(ver SDL3_MIX_VER)"
if [ -z "$sdl3" ] || [ -z "$mix" ]; then
    echo "error: SDL3 versions not found in $deps" >&2
    exit 1
fi

mkdir -p "$dest"
fetch() {
    [ -d "$dest/$2" ] || curl -fsSL "$1" | tar xz -C "$dest"
}
fetch "https://github.com/libsdl-org/SDL/releases/download/release-$sdl3/SDL3-devel-$sdl3-mingw.tar.gz" "SDL3-$sdl3"
fetch "https://github.com/libsdl-org/SDL_mixer/releases/download/release-$mix/SDL3_mixer-devel-$mix-mingw.tar.gz" "SDL3_mixer-$mix"

echo "-I$dest/SDL3-$sdl3/i686-w64-mingw32/include -I$dest/SDL3_mixer-$mix/i686-w64-mingw32/include"
