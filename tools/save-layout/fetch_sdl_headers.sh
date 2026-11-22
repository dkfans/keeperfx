#!/bin/bash
# Download the SDL3 MinGW headers that the saved-struct headers
# include. The version is read from build/cmake/modules/Dependencies.cmake so
# it always matches the real build.
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
if [ -z "$sdl3" ]; then
    echo "error: SDL3 version not found in $deps" >&2
    exit 1
fi

mkdir -p "$dest"
fetch() {
    [ -d "$dest/$2" ] || curl -fsSL "$1" | tar xz -C "$dest"
}
fetch "https://github.com/libsdl-org/SDL/releases/download/release-$sdl3/SDL3-devel-$sdl3-mingw.tar.gz" "SDL3-$sdl3"

echo "-I$dest/SDL3-$sdl3/i686-w64-mingw32/include"
