#!/bin/bash
# Compare the saved layout of the current checkout with a base commit.
#
#   check_pr.sh <base commit> <output dir> [pr|build]
#
# Run from the repository root. The last argument words the summary for a
# pull request (default) or for a master build. Writes <output dir>/result.json and
# summary.md, and prints the verdict (identical, compatible, refused, silent).
# The exit status is 0 unless the tooling itself fails; the caller applies
# the pass/fail policy.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
base="$1"
out="$(mkdir -p "$2" && cd "$2" && pwd)"
subject="${3:-pr}"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT

if [ -z "${SDL_INCLUDES:-}" ]; then
    SDL_INCLUDES="$(bash "$here/fetch_sdl_headers.sh" . "$work/sdl")"
    export SDL_INCLUDES
fi

# Base: headers only, straight from git.
mkdir -p "$work/base"
git archive --format=tar "$base" -- ':(glob)src/**/*.h' ':(glob)src/**/*.hpp' ':(glob)src/*.h' \
    ':(glob)deps/centitoml/*.h' | tar x -C "$work/base"

# The base's own roots.txt may not exist yet (first run), so both sides use
# the head's list; a root missing on one side is reported as a change.
bash "$here/extract.sh" "$work/base" "$out/base.tsv"
bash "$here/extract.sh" . "$out/head.tsv"
# Saved layout must not depend on the heavy-logging / functional-test flags.
bash "$here/extract.sh" . "$out/head_flags.tsv" -DBFDEBUG_LEVEL=10 -DFUNCTESTING=1

lua=()
if ! git diff --quiet "$base" -- config/fxdata/lua/core/serialisation.lua config/fxdata/lua/external/binser.lua; then
    lua=(--lua-changed)
fi

python3 "$here/compare.py" "$here/roots.txt" "$out/base.tsv" "$out/head.tsv" \
    "$out/result.json" "$out/summary.md" --flags-tsv "$out/head_flags.tsv" --subject "$subject" "${lua[@]}"
