# KeeperFX on Linux

KeeperFX runs natively on 64-bit Linux (x86_64). The game data is the same as on
Windows; only the executables differ.

## Playing

The Linux release is a *patch* archive, like the Windows patches: it holds the
native `keeperfx` and `keeperfx_hvlog` executables, the libraries they need (in
`lib/`), and the updated game data. It goes on top of a full KeeperFX install.

1. **Get a full KeeperFX install.** Download the latest complete release from
   [keeperfx.net](https://keeperfx.net) or the
   [GitHub releases](https://github.com/dkfans/keeperfx/releases) and unpack it
   into a folder of your choice, e.g. `~/Games/keeperfx`.
2. **Add the original Dungeon Keeper files.** Copy the files listed in
   [files_required_from_original_dk.txt](files_required_from_original_dk.txt)
   (`data/*.dat|pal|col` and `sound/*.sbk`) from your Dungeon Keeper CD, or
   GOG/Steam/EA install, into the same folders of the KeeperFX install. File
   name case does not matter: KeeperFX finds `DATA/BLUEPAL.DAT` as well as
   `data/bluepal.dat`.
3. **Unpack the Linux patch over it.** Download
   `keeperfx-linux_x86_64-<version>-patch.tar.gz` and extract it into the
   KeeperFX folder, replacing existing files:

   ```bash
   tar -xzf keeperfx-linux_x86_64-*-patch.tar.gz -C ~/Games/keeperfx
   ```
4. **Run it** from the KeeperFX folder:

   ```bash
   cd ~/Games/keeperfx && ./keeperfx
   ```

   Command line options are the same as on Windows, see
   [Command Line Options](https://github.com/dkfans/keeperfx/wiki/Command-Line-Options).
   `./keeperfx_hvlog` is the heavy-log version for bug reports. Both write
   `keeperfx.log` in the KeeperFX folder.

### Requirements

- 64-bit x86 Linux with glibc 2.39 or newer: Ubuntu 24.04, Debian 13, Fedora 40,
  current SteamOS, Arch and anything more recent.
- The system's own graphics (OpenGL/EGL, X11 or Wayland) and sound (ALSA or
  PulseAudio/PipeWire) libraries. Everything else ships in `lib/`.

If the game does not start, run `ldd ./keeperfx | grep "not found"` to see
which system library is missing, and read `keeperfx.log`.

For the Steam Deck, see also [steam-deck.md](steam-deck.md).

## Building from source

You need GCC (or Clang), CMake, Ninja, pkg-config and these libraries with their
headers: FFmpeg (libavcodec, libavformat, libavutil, libswresample), OpenAL,
LuaJIT 2.1, libspng, minizip, zlib, miniupnpc, libnatpmp, OpenSSL, zstd and
SDL3 with SDL3_image and SDL3_mixer. Astronomy, centijson, ENet6 and libcurl are
downloaded as prebuilt static libraries during configure.

**Arch Linux:**

```bash
sudo pacman -S --needed base-devel cmake ninja pkgconf ffmpeg openal luajit \
    libspng minizip zlib miniupnpc libnatpmp openssl zstd sdl3 sdl3_image sdl3_mixer
```

**Ubuntu 24.04 / Debian:** see the package list in
[build-linux-patch.yml](../.github/workflows/build-linux-patch.yml). Ubuntu 24.04
has no SDL3 packages, so the configure step builds SDL3, SDL3_image and
SDL3_mixer from source, which needs their X11/Wayland/GL/audio and image/audio
codec `-dev` packages from that list.

Then build:

```bash
cmake -S . -B out -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build out --target keeperfx
```

The result is `out/keeperfx`. To try it, copy it into a KeeperFX install (step 3
above) or run it with that folder as the working directory.

### Building the release archive

The CI job in `build-linux-patch.yml` makes the patch archive. Locally:

```bash
make pkg-enginegfx                       # engine graphics into pkg/
cmake -S . -B out/release -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DKFX_DECK_BUNDLE=ON -DKFX_SPLIT_DEBUG=ON -DBUILD_NUMBER=<n>
cmake --build out/release --target keeperfx keeperfx_hvlog
make pkg-assemble                        # game data into pkg/
cmake --build out/release --target package
```

This writes `pkg/keeperfx-linux_x86_64-<version>-patch.tar.gz`.
`KFX_DECK_BUNDLE` copies the non-system libraries into `lib/` and points the
executables there, and `KFX_SPLIT_DEBUG` moves the debug info into
`out/release/*.debug`. A library bundle only runs on systems with the same or a
newer glibc than the build machine, so build releases on the oldest system you
want to support.
