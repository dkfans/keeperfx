#!/usr/bin/env bash
#
#
# Usage:   scripts/build-deck-bundle.sh
# Output:  out/keeperfx-deck/   (deploy it with scripts/deploy-to-deck.sh)
#
# Requires Docker. On Windows, run from Git Bash. See docs/steam-deck.md.
#
set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
IMAGE="kfx-deck-build"

# Docker wants a Windows-style path on Git Bash; native path elsewhere.
case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*) HOSTREPO="$(cygpath -w "$REPO" | sed 's|\\\\|/|g')"; export MSYS_NO_PATHCONV=1 ;;
    *) HOSTREPO="$REPO" ;;
esac

echo ">> building pinned image '$IMAGE' (cached after the first run)"
docker build -q -t "$IMAGE" -f "$HOSTREPO/build/docker/deck.Dockerfile" "$HOSTREPO/build/docker" >/dev/null

mkdir -p "$REPO/out"

echo ">> configure + build + bundle in container"
docker run --rm \
    -v "${HOSTREPO}:/src:ro" \
    -v "${HOSTREPO}/out:/out" \
    "$IMAGE" bash -euc '
        mkdir -p /work
        find /src -maxdepth 1 -type f -exec cp {} /work/ \;
        cp -r /src/build /work/build
        cp -r /src/src   /work/src
        [ -d /src/tools ] && cp -r /src/tools /work/tools || true
        mkdir -p /work/deps && cp -r /src/deps/centitoml /work/deps/centitoml
        find /src/deps -maxdepth 1 -type f -name "*.h" -exec cp {} /work/deps/ \;
        cmake -S /work -B /work/out -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DKFX_DECK_BUNDLE=ON
        cmake --build /work/out --target keeperfx -j"$(nproc)"
        rm -rf /out/keeperfx-deck
        cmake --install /work/out --prefix /out/keeperfx-deck
    '

echo ">> bundle ready: $REPO/out/keeperfx-deck"
echo "   glibc floor: $(docker run --rm -v "${HOSTREPO}/out:/out" "$IMAGE" \
        bash -c "objdump -T /out/keeperfx-deck/keeperfx | grep -oE 'GLIBC_[0-9.]+' | sort -V | tail -1")"
echo "   deploy it:   scripts/deploy-to-deck.sh --bundle out/keeperfx-deck --assets <your-game-dir>"
