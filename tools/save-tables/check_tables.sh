#!/bin/bash
# Check that the field tables list every member of every saved struct.
#
#   check_tables.sh
#
# Run from anywhere. Prints paste-ready lines for anything missing and exits
# non-zero when a table is wrong. Needs the same tools as tools/save-layout.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
root="$(cd "$here/../.." && pwd)"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT

if [ -z "${SDL_INCLUDES:-}" ]; then
    SDL_INCLUDES="$(bash "$root/tools/save-layout/fetch_sdl_headers.sh" "$root" "$work/sdl")"
fi
export SDL_INCLUDES KFX_WALK="$here/dump_types.py"

bash "$root/tools/save-layout/extract.sh" "$root" "$work/types.tsv"
python3 "$here/tables.py" check "$work/types.tsv" "$root/src/kfx/save/core/schema"
