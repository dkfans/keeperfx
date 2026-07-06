#!/bin/bash
#
# Build a portable KeeperFX AppImage.
#
# Must be run inside a Debian 12 (bookworm) environment - glibc 2.36 + ffmpeg 5.1 -
# for broad distro compatibility (Ubuntu 22.10+/24.04, Fedora, Mint, Arch, ...).
# Run from the repository root, e.g. in CI:
#   docker run --rm -v "$PWD":/src -w /src debian:12 bash dist/linux/build-appimage.sh /src/dist/out
#
# The resulting AppImage bundles the engine, all libraries (incl. a modern SDL2)
# and the freely-redistributable KeeperFX data. It NEVER contains the original
# Dungeon Keeper files (copyright EA/Bullfrog) - the user supplies those at runtime.
#
# Usage: dist/linux/build-appimage.sh [OUTPUT_DIR]
set -euo pipefail
export DEBIAN_FRONTEND=noninteractive
export APPIMAGE_EXTRACT_AND_RUN=1   # run the tool AppImages without FUSE (works in containers)

REPO="$(cd "$(dirname "$0")/../.." && pwd)"
OUT_DIR="${1:-$REPO/dist/out}"

# Version of this tree, from version.mk (e.g. 1.4.0). Used both to name the output
# AppImage and, by default, to pick which release's free data to bundle.
VER="$(grep -E '^VER_MAJOR=' "$REPO/version.mk" | cut -d= -f2).$(grep -E '^VER_MINOR=' "$REPO/version.mk" | cut -d= -f2).$(grep -E '^VER_RELEASE=' "$REPO/version.mk" | cut -d= -f2)"

# Free KeeperFX data bundled into the AppImage. Defaults to THIS tree's version so
# the bundled data always matches the engine being built. Override with
# KFX_DATA_VERSION when building from a tree whose release has not been published
# yet (the data is pulled from that version's GitHub release; a missing release is
# reported with a clear error in step 5 rather than shipping mismatched data).
KFX_DATA_VERSION="${KFX_DATA_VERSION:-$VER}"
mkdir -p "$OUT_DIR"
cd "$REPO"

echo "######## 1. build dependencies ########"
apt-get update -qq
apt-get install -y -qq build-essential cmake ninja-build git pkg-config \
  p7zip-full curl ca-certificates file wget squashfs-tools patchelf zsync jq \
  libsdl2-dev libsdl2-mixer-dev libsdl2-net-dev libsdl2-image-dev \
  libavformat-dev libavcodec-dev libavutil-dev libswresample-dev \
  libopenal-dev libluajit-5.1-dev libspng-dev libminizip-dev \
  libssl-dev libzstd-dev libminiupnpc-dev libnatpmp-dev zlib1g-dev \
  libwayland-dev wayland-protocols libxkbcommon-dev libdecor-0-dev \
  libx11-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libxfixes-dev \
  libxss-dev libgl1-mesa-dev libegl1-mesa-dev libgles2-mesa-dev \
  libdbus-1-dev libudev-dev libpulse-dev libasound2-dev libibus-1.0-dev >/dev/null
echo "glibc: $(ldd --version | head -1) | ffmpeg: $(pkg-config --modversion libavcodec)"

# Modern SDL2 built from source: Debian 12 ships 2.26 (broken Wayland fullscreen).
# Pinned to a fixed SDL2 release tag for reproducible builds - bump deliberately,
# don't track the rolling SDL2 branch (which self-reports as an unreleased 2.33.x-dev).
SDL2_TAG="${SDL2_TAG:-release-2.32.10}"
echo "######## 2. modern SDL2 ($SDL2_TAG) ########"
git config --global --add safe.directory '*' || true
git clone -q --depth 1 -b "$SDL2_TAG" https://github.com/libsdl-org/SDL.git /tmp/sdl2-src
cmake -S /tmp/sdl2-src -B /tmp/sdl2-build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_LIBDIR=lib/x86_64-linux-gnu \
  -DSDL_WAYLAND=ON -DSDL_X11=ON -DSDL_WAYLAND_LIBDECOR=ON -DSDL_STATIC=OFF >/dev/null
cmake --build /tmp/sdl2-build >/dev/null
cmake --install /tmp/sdl2-build >/dev/null
ldconfig
echo "bundled SDL2: $(pkg-config --modversion sdl2)"

echo "######## 3. build engine (linux.mk) ########"
# Drop any stale objects (e.g. when the working tree was previously built on a
# different toolchain via a bind mount); a fresh CI checkout has none anyway.
rm -rf obj bin src/ver_defs.h
make -f linux.mk -j"$(nproc)" all
file bin/keeperfx

echo "######## 4. AppDir skeleton ########"
APPDIR="$REPO/AppDir"
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/share/keeperfx" \
         "$APPDIR/usr/share/applications" "$APPDIR/usr/share/icons/hicolor/256x256/apps"
cp bin/keeperfx "$APPDIR/usr/bin/keeperfx"

echo "######## 5. free FX data ($KFX_DATA_VERSION; official release, already free of the 16 copyright files) ########"
DATA_7Z="keeperfx_${KFX_DATA_VERSION//./_}_complete.7z"
DATA_URL="https://github.com/dkfans/keeperfx/releases/download/v${KFX_DATA_VERSION}/${DATA_7Z}"
# Expected sha256, resolved from the GitHub release API. Best-effort: if the API is
# unreachable / rate-limited the digest is empty and verification is skipped with a
# warning, but a real mismatch always fails the build.
DATA_DIGEST="$(curl -fsSL "https://api.github.com/repos/dkfans/keeperfx/releases/tags/v${KFX_DATA_VERSION}" 2>/dev/null \
  | jq -r --arg n "$DATA_7Z" '.assets[] | select(.name==$n) | .digest // empty' 2>/dev/null || true)"
# Download (fail hard and clearly on a missing asset / 404, e.g. a tree ahead of any release).
if ! curl -fLso "/tmp/$DATA_7Z" "$DATA_URL"; then
  echo "ERROR: could not download free data for KeeperFX v${KFX_DATA_VERSION}:" >&2
  echo "  $DATA_URL" >&2
  echo "Set KFX_DATA_VERSION to a released version that ships a *_complete.7z (e.g. 1.4.0)." >&2
  exit 1
fi
if [ -n "$DATA_DIGEST" ]; then
  if echo "${DATA_DIGEST#sha256:}  /tmp/$DATA_7Z" | sha256sum -c - >/dev/null; then
    echo "data checksum OK ($DATA_DIGEST)"
  else
    echo "ERROR: checksum mismatch for $DATA_7Z (expected $DATA_DIGEST)" >&2
    exit 1
  fi
else
  echo "WARNING: no digest published for $DATA_7Z; skipping checksum verification." >&2
fi
mkdir -p /tmp/fxdata && (cd /tmp/fxdata && 7z x -y "/tmp/$DATA_7Z" >/dev/null)
for d in data fxdata ldata levels sound campgns creatrs mods music; do
  [ -d "/tmp/fxdata/$d" ] && cp -r "/tmp/fxdata/$d" "$APPDIR/usr/share/keeperfx/"
done
[ -f /tmp/fxdata/keeperfx.cfg ] && cp /tmp/fxdata/keeperfx.cfg "$APPDIR/usr/share/keeperfx/"
mkdir -p "$APPDIR/usr/share/keeperfx/save"

echo "######## 6. icon + desktop ########"
cp res/keeperfx_icon256-24bpp.png "$APPDIR/usr/share/icons/hicolor/256x256/apps/keeperfx.png"
cp res/keeperfx_icon256-24bpp.png "$APPDIR/keeperfx.png"
cp dist/linux/keeperfx.desktop "$APPDIR/usr/share/applications/keeperfx.desktop"

echo "######## 7. bundle libraries (linuxdeploy) ########"
cd /tmp
wget -q https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage
./linuxdeploy-x86_64.AppImage --appdir "$APPDIR" \
  -e "$APPDIR/usr/bin/keeperfx" \
  -d "$APPDIR/usr/share/applications/keeperfx.desktop" \
  -i "$APPDIR/keeperfx.png" 2>&1 | tail -15

echo "######## 8. custom AppRun (data setup; original DK files supplied by the user) ########"
rm -f "$APPDIR/AppRun"   # linuxdeploy leaves a symlink -> usr/bin/keeperfx; replace with ours
cp "$REPO/dist/linux/AppRun" "$APPDIR/AppRun"
chmod +x "$APPDIR/AppRun"

echo "######## 9. appimagetool ########"
cd /tmp
wget -q https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
chmod +x appimagetool-x86_64.AppImage
OUT_FILE="$OUT_DIR/KeeperFX-${VER}-linux-x86_64.AppImage"
ARCH=x86_64 ./appimagetool-x86_64.AppImage "$APPDIR" "$OUT_FILE" 2>&1 | tail -10
chmod -R a+rwX "$OUT_DIR" || true
echo "######## DONE ########"
ls -la "$OUT_FILE"
