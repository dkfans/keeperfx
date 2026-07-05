#!/bin/sh
# Build the source-only KeeperFX dependencies as arm64 static libraries for a
# native Apple Silicon macOS build. These libraries have no Homebrew formula, so
# we compile them from upstream source into deps/ where macos.mk expects them.
#
#   deps/astronomy/{include/astronomy.h, libastronomy.a}
#   deps/centijson/{include/*.h, libjson.a}
#   deps/enet6/{include/enet6/*.h, libenet6.a}
#
# The Homebrew-provided deps (SDL2, ffmpeg, luajit, openal-soft, libspng,
# minizip, miniupnpc, libnatpmp, zlib, curl) are resolved via pkg-config.
set -e

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WORK="$(mktemp -d)"
CFLAGS="-O3 -arch arm64"
echo "Working in $WORK"

echo "== centijson =="
git clone --depth 1 https://github.com/mity/centijson.git "$WORK/centijson"
mkdir -p "$ROOT/deps/centijson/include"
cp "$WORK"/centijson/src/*.h "$ROOT/deps/centijson/include/"
( cd "$WORK/centijson/src" && clang $CFLAGS -c json.c json-dom.c json-ptr.c value.c )
ar rcs "$ROOT/deps/centijson/libjson.a" "$WORK"/centijson/src/*.o

echo "== astronomy =="
git clone --depth 1 https://github.com/cosinekitty/astronomy.git "$WORK/astronomy"
mkdir -p "$ROOT/deps/astronomy/include"
cp "$WORK/astronomy/source/c/astronomy.h" "$ROOT/deps/astronomy/include/"
( cd "$WORK/astronomy/source/c" && clang $CFLAGS -c astronomy.c )
ar rcs "$ROOT/deps/astronomy/libastronomy.a" "$WORK"/astronomy/source/c/astronomy.o

echo "== enet6 =="
git clone --depth 1 https://github.com/SirLynix/enet6.git "$WORK/enet6"
mkdir -p "$ROOT/deps/enet6/include/enet6"
cp -r "$WORK"/enet6/include/enet6/* "$ROOT/deps/enet6/include/enet6/"
( cd "$WORK/enet6/src" && clang $CFLAGS -DHAS_SOCKLEN_T=1 -I"$WORK/enet6/include" \
    -c address.c callbacks.c compress.c host.c list.c packet.c peer.c protocol.c unix.c )
ar rcs "$ROOT/deps/enet6/libenet6.a" "$WORK"/enet6/src/*.o

rm -rf "$WORK"
echo "Done. Built deps/{astronomy,centijson,enet6} for arm64."
