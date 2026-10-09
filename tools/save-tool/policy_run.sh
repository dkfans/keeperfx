#!/bin/bash
# Builds kfx-savetool for the base and the head of a pull request and writes policy-out/result.json.
#
#   BASE_SHA=... HEAD_SHA=... PR=... LABELS=a,b policy_run.sh
#
# Run from the root of the checkout (the merge of the PR). Whatever can't be compared, such as a base
# without the tool yet, leaves a result with no findings: the check stays silent.
set -uo pipefail

out="$PWD/policy-out"
rm -rf "$out"
mkdir -p "$out/base" "$out/head"

empty() {
    printf '{"pr": %d, "head_sha": "%s", "findings": []}\n' "$PR" "$HEAD_SHA" > "$out/result.json"
    echo "$1"
    exit 0
}

# side <name> <tree>: the tool's output for one tree, in policy-out/<name>
side() {
    local name="$1" tree="$2"
    [ -f "$tree/tools/save-tool/build.sh" ] || return 1
    (cd "$tree" && bash tools/save-tool/build.sh "$out/savetool-$name") || return 1
    "$out/savetool-$name" schema --build > "$out/$name/schema.txt" || return 1
    "$out/savetool-$name" migrations > "$out/$name/migrations.txt" || return 1
    cp "$tree/src/kfx/save/core/save_codec.h" "$out/$name/save_codec.h" || return 1
}

git worktree add --detach "$out/base-tree" "$BASE_SHA" > /dev/null 2>&1 || empty "no base to compare with"
side base "$out/base-tree" || empty "the base can't be compared (no save tool yet, or it didn't build)"
side head "$PWD" || empty "the save tool didn't build for the head; the other checks report that"

python3 tools/save-tool/policy.py --base "$out/base" --head "$out/head" --pr "$PR" --head-sha "$HEAD_SHA" \
    --labels "${LABELS:-}" --out "$out/result.json" || empty "the comparison failed"
cat "$out/result.json"
