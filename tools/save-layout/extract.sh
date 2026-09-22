#!/bin/bash
# Extract the layout of every saved type (roots.txt) from a source tree.
#
#   extract.sh <source root> <out.tsv> [extra compiler flags...]
#
# Needs: i686-w64-mingw32-gcc, i686-w64-mingw32-objcopy, gdb-multiarch.
# SDL_INCLUDES must hold the -I flags for the SDL3 and SDL3_mixer MinGW
# headers (see fetch_sdl_headers.sh). Only headers are compiled; no build
# directory is needed.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
src="$(cd "$1" && pwd)"
out="$2"
shift 2

work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT

roots=""
{
    echo '#include "pre_inc.h"'
    while read -r kw a _; do
        [ "$kw" = "include" ] && echo "#include \"$a\""
    done < "$here/roots.txt"
    echo '#include "post_inc.h"'
    n=0
    while read -r kw a _; do
        [ "$kw" = "root" ] || continue
        roots="$roots $a"
        if [[ "$a" == typedef:* ]]; then
            echo "${a#typedef:} probe_root_$n;"
        else
            echo "struct $a probe_root_$n;"
        fi
        n=$((n + 1))
    done < "$here/roots.txt"
    echo "$roots" > "$work/roots"
} > "$work/probe.c"
roots="$(cat "$work/roots")"

i686-w64-mingw32-gcc -g -c -w \
    -I"$here/stub" -I"$src/src" -I"$src/deps/centitoml" ${SDL_INCLUDES:-} \
    -DBFDEBUG_LEVEL=0 -DDEBUG=0 "$@" \
    "$work/probe.c" -o "$work/probe.o"

# gdb can't read the MinGW COFF object's debug info directly.
i686-w64-mingw32-objcopy -O elf32-i386 "$work/probe.o" "$work/probe.elf"

gdb-multiarch -batch -nx \
    -ex "python ROOTS='$roots'; OUTFILE='$out'" \
    -ex "source $here/walk.py" \
    "$work/probe.elf" > /dev/null

if grep -q $'\tmissing\t' "$out"; then
    echo "error: saved type not found in the headers:" >&2
    grep $'\tmissing\t' "$out" | cut -f1 >&2
    exit 1
fi
