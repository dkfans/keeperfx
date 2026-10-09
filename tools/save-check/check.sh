#!/bin/bash
# Local checks for the save code, before submitting.
#
#   tools/save-check/check.sh [lint | valgrind | all]      (default: all)
#
#   lint      cppcheck and clang-tidy over src/kfx/save; both must report nothing
#   valgrind  builds the standalone save tests with debug info and runs them under valgrind: any leak or
#             memory error fails
#
# Runs in a Docker image built from tools/save-check/Dockerfile, so nothing has to be installed. Needs Docker.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
root="$(cd "$here/../.." && pwd)"
what="${1:-all}"
image=kfx-save-check

docker build -q -t "$image" "$here" > /dev/null

# Windows (Git Bash) needs the path in Windows form, and must not convert the paths inside the container
mount="$root"
if command -v cygpath > /dev/null 2>&1; then mount="$(cygpath -w "$root")"; fi

MSYS_NO_PATHCONV=1 docker run --rm -v "$mount:/src" -w /src "$image" bash -c '
set -uo pipefail
what="$1"
status=0
SDL="$(bash tools/save-layout/fetch_sdl_headers.sh /src /tmp/sdl)"
INC="-Itools/save-layout/stub -Itools/save-tool -Isrc -Ideps/centitoml $SDL -DBFDEBUG_LEVEL=0 -DDEBUG=0"
CORE="$(ls src/kfx/save/core/*.cpp)"

if [ "$what" = lint ] || [ "$what" = all ]; then
    echo "== cppcheck"
    cppcheck --enable=warning,style,performance,portability --std=c++17 --inline-suppr --quiet --error-exitcode=1 \
        --suppress=missingIncludeSystem --suppress=missingInclude --suppress=unusedFunction \
        --suppress=normalCheckLevelMaxBranches \
        -Isrc -Itools/save-layout/stub -DBFDEBUG_LEVEL=0 -DDEBUG=0 src/kfx/save || status=1

    echo "== clang-tidy (the portable core; the settings are in src/kfx/save/.clang-tidy)"
    tidy_output="$(clang-tidy -quiet --warnings-as-errors="*" $CORE -- -std=c++17 -x c++ $INC 2>&1)"
    if echo "$tidy_output" | grep -E ": (warning|error):"; then status=1; fi
fi

if [ "$what" = valgrind ] || [ "$what" = all ]; then
    echo "== valgrind"
    CFLAGS_EXTRA="-g -O0" bash tests/save/build.sh /tmp/kfx_save_tests > /dev/null
    for t in /tmp/kfx_save_tests /tmp/kfx_save_tests_records; do
        valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=9 -q "$t" > /dev/null || status=1
    done
fi

[ "$status" = 0 ] && echo "ALL CHECKS PASSED" || echo "CHECKS FAILED"
exit $status' _ "$what"
