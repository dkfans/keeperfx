# Native macOS (Apple Silicon / arm64) build

KeeperFX builds and runs **natively on Apple Silicon** — a Mach-O arm64 binary,
no Rosetta, no emulation. It has been verified end to end: it compiles, launches,
loads the original Dungeon Keeper data, and runs into live gameplay.

The macOS build reuses the native **Linux** build (`linux.mk`), which already does
almost all of the cross-platform work; `macos.mk` is the Linux build adapted for
arm64 + Homebrew.

## Why this is tractable

The hard part of porting KeeperFX (the historic hooking into the original 32-bit
`DK.exe` at fixed addresses) is **gone** — the engine is fully decompiled and
standalone. Concretely:

- **64-bit clean.** The Linux build compiles with `-march=x86-64`; there is no
  32-bit lock-in. (Critical, since macOS has had zero 32-bit support since
  Catalina.)
- **SDL2 is the platform layer** for video, input, audio and networking. The
  renderer is software/surface based — no OpenGL or DirectX in the hot path.
- The platform-specific spots are already guarded: `src/bflib_cpu.c` (`cpuid`
  asm behind `__i386__ || __x86_64__`), `src/bflib_crash.c` (POSIX path with a
  non-Linux fallback), `src/linux.cpp` (pure POSIX, reused as-is), and every
  `#include <windows.h>` sits behind `#ifdef _WIN32` / `__MINGW32__`.

## Source changes that *were* needed

Compiling on arm64 did require a few small edits to shared headers (they also
help a native Linux build and change no behaviour on Windows):

- **Un-packing in-memory-only structs.** Several structs were declared inside
  `#pragma pack(1)` blocks (`GuiButton`/`GuiMenu` in `bflib_guibtns.h`;
  `NamedCommand`/`NamedField`/`ConfigFileData` in `config.h`; `GamekeySettings`
  in `front_input.h`; `DemoItem` in `frontend.h`). None of these are serialized
  to disk — they are runtime helpers full of pointers and function pointers. On
  arm64, statically-initialized packed structs place those pointers at unaligned
  offsets that the linker rejects. They are now left at natural alignment.
- **`src/globals.h`** — the POSIX `<unistd.h>`/`<signal.h>` include path now also
  fires on `__APPLE__` / `__unix__`, not just the legacy `unix` macro.

## Prerequisites

1. **Xcode Command Line Tools:** `xcode-select --install`
2. **Homebrew:** https://brew.sh
3. Homebrew libs: `sdl2 sdl2_image sdl2_mixer sdl2_net ffmpeg luajit openal-soft
   libspng minizip miniupnpc libnatpmp zlib curl` (plus `innoextract`/`sevenzip`
   if you extract original DK data from a GOG installer).

## Build

```sh
# from the repo root
./tools/build_macos_deps.sh                       # one-time (and after dep bumps)
make -f macos.mk -j"$(sysctl -n hw.ncpu)"
```

`build_macos_deps.sh` builds the three deps Homebrew doesn't publish for arm64
(`centijson`, `astronomy`, `enet6`) from source into `deps/`, where `macos.mk`
expects them.

Output binary: `bin/keeperfx` (Mach-O arm64).

## Running

Run the binary from a directory populated with a KeeperFX install **plus the
files listed in `docs/files_required_from_original_dk.txt`** copied from an
original Dungeon Keeper (CD / GOG / Steam / Origin). KeeperFX's install gate is
simply a check for `data/bluepal.dat`.

> Tip: on a machine with the GOG "Dungeon Keeper Gold" app installed, all 16
> required files ship **uncompressed** inside the app bundle under
> `Contents/Resources/game/{DATA,SOUND}/` — no `game.gog` CD-image extraction is
> needed. Copy them in with lowercase names.

Benign log noise on first run: missing `font12/16.fxfont` (optional Unicode
fonts; it falls back to the `.fon` bitmap fonts) and `colours.col` / `tables.dat`
/ `alpha.col` "errors" that are regenerated at runtime from the palette.

## Packaging a self-contained `KeeperFX.app`

`tools/make_macos_app.sh` turns `bin/keeperfx` into a redistributable
`dist/KeeperFX.app` that runs on any Apple Silicon Mac with **no Homebrew**:

```sh
make -f macos.mk && tools/make_macos_app.sh
```

What it does, and the non-obvious bits it handles:

- **Bundles the dylibs.** `dylibbundler` copies every non-system dylib into
  `Contents/libs` and rewrites the engine's load paths to `@executable_path/../libs`.
- **SDL3, which `dylibbundler` can't see.** Homebrew's `sdl2` is actually
  `sdl2-compat` — a thin `libSDL2` that **`dlopen`s SDL3 at runtime**, so SDL3 is
  not a link-time dependency and the bundling pass misses it (the shim then dies
  in its initializer). The shim's first search path is `@loader_path/libSDL3.dylib`
  (note: *unversioned*), so the script copies `libSDL3.0.dylib` in as
  `Contents/libs/libSDL3.dylib` and the shim finds it with no env vars or rpath.
- **A drop-in, self-locating engine.** Finder starts apps with `cwd=/`, but the
  game data lives in the folder the user dropped the `.app` into. The engine is
  the bundle's main executable and, on startup, `macos_chdir_to_bundle_parent()`
  (in `src/linux.cpp`) resolves its own path via `_NSGetExecutablePath`; if it is
  running from inside a `*.app/Contents/MacOS/`, it `chdir`s to the folder
  containing the `.app`. Run outside a bundle (a developer launching
  `bin/keeperfx` directly) and the working directory is left untouched. This
  replaces the older bash-wrapper launcher.
- **Folder-access permission prompt.** Because the engine is a real signed main
  executable (not a script) and `Info.plist` declares `NSDesktopFolderUsage`/
  `NSDocumentsFolderUsage`/`NSDownloadsFolderUsageDescription`, macOS shows a
  normal privacy prompt the first time the app reads data from a protected folder
  (Desktop/Documents/Downloads). The user clicks **Allow** once and it works from
  any location. (A bash-wrapper main executable is *silently denied* instead of
  prompted, which is why the wrapper approach failed on the Desktop.)
- **Ad-hoc code signing.** arm64 requires every Mach-O to be signed to run. The
  dylibs are signed first (inside-out), then the whole bundle — sealing the
  engine and giving macOS a stable app identity to attach the folder-access grant
  to. Users still clear Gatekeeper once (right-click → Open) since it isn't
  notarized.

CI runs this and uploads the `.app` as an artifact
(`.github/workflows/build-macos.yml`).

## Optional enhancements

- **Crash register dump.** Add an `#elif defined(__APPLE__) && defined(__aarch64__)`
  branch to `log_posix_context()` in `src/bflib_crash.c` reading
  `((ucontext_t*)context)->uc_mcontext->__ss.__pc` / `__sp`. Nicer crash logs;
  not required.
- **Notarization.** Replace the ad-hoc signature with a Developer ID signature +
  `notarytool` so the `.app` opens without the Gatekeeper right-click dance.
- **Share the source list.** Factor the common `KFX_SOURCES` out of `linux.mk`
  so `macos.mk` can include it instead of duplicating (keeps them from drifting
  when upstream adds/removes a source file).

## Staying in sync with upstream

The port adds only new files (`macos.mk`, `tools/build_macos_deps.sh`, this doc)
plus a handful of guarded header edits, so pulling from upstream rarely
conflicts. If upstream adds or removes a file in `linux.mk`'s `KFX_SOURCES`,
mirror that one line into `macos.mk`.
