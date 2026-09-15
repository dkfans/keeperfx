# IDE build & debug setup: Make vs CMake

KeeperFX has two independent build paths. Pick either one - they don't need to
agree, and most contributors will only ever touch one:

- **Makefile** - the canonical build. CI and releases use it. GCC/MinGW-w64
  (i686), same as it's always been.
- **CMake** - an alternative build for the same code, via `CMakeLists.txt` +
  `CMakePresets.json`. Same GCC/MinGW-w64 toolchain is the default preset
  (`x86-MinGW32-Debug`); MSVC and Clang-cl presets are also available for
  local convenience/debugging.

Neither is required to use the other. This page covers wiring each one into
VS Code and Visual Studio 2026.

## VS Code - Make (unchanged)

This is the existing workflow: `Ctrl+Shift+B` runs the Makefile via WSL, `F5`
builds + copies the binary into your configured game folder and launches gdb.
First-time setup happens automatically on folder open (see
`.vscode/create_settings_and_launch_files.ps1`) and prompts you to locate your
installed `keeperfx.exe`, which seeds `program`/`cwd` in your personal
(gitignored) `.vscode/launch.json`.

## VS Code - CMake

1. Install the recommended `ms-vscode.cmake-tools` extension (VS Code will
   prompt you; it's listed in `.vscode/extensions.json`).
2. Pick a configure preset from the CMake Tools status bar (or Command
   Palette -> "CMake: Select Configure Preset"). `x86-MinGW32-Debug` is the
   fork's standard toolchain and closest to the Makefile build; `Debug-MSVC`
   / `Debug-Clang` are also available.
3. Configure once (CMake Tools does this automatically, or run
   `cmake --preset <name>` yourself).
4. Set `cmake.debugConfig` once in your (personal, gitignored)
   `.vscode/settings.json` - a template is in `.vscode/defaultsettings`. At
   minimum it needs `cwd` pointed at your installed game folder; `args` mirror
   what the Makefile debug config uses (`-level 00001 -campaign keeporig
   -nointro -alex`).
5. Build and debug:
   - `Ctrl+Shift+B` still prompts "Choose what Ctrl+Shift+B builds" with both
     Make and CMake options (`CMake: x86-MinGW32-Debug`, etc.) for a
     compile-only build of a specific preset.
   - To build, launch, and attach a debugger, don't use F5 - F5 always runs
     whichever launch.json entry is selected, which can't switch debugger
     backends (gdb vs the native Visual Studio debugger) depending on which
     CMake preset happens to be active. Instead use **CMake Tools' own Debug
     action**: the status bar's Debug button, Command Palette ->
     "CMake: Debug", or right-click a target in the CMake sidebar. It builds
     the active target if needed, and automatically picks the matching
     debugger for whichever preset/kit is currently selected (gdb for
     MinGW32, the native debugger for MSVC/Clang-cl) - so switching presets
     in the CMake Tools status bar is the only thing you need to do, no
     launch.json changes required.
   - The build runs the exe straight from its CMake output directory (no copy
     step): vcpkg's applocal deployment already places runtime DLLs beside
     the exe there, and `cwd` from `cmake.debugConfig` points it at your game
     data, exactly like Visual Studio's own `launch.vs.json` already does.

## Visual Studio 2026

Use **File -> Open -> Folder...** on the repository root, not a `.sln`. VS
reads `CMakeLists.txt`/`CMakePresets.json` directly and exposes the same
presets (MinGW32, MSVC, Clang-cl) in its own configuration dropdown. The
legacy `keeperfx_vs2010.sln` has been removed - it predated this and is
superseded by Open Folder.

For debugging, use VS's own Debug Target dropdown (set it to `keeperfx.exe`)
and configure the working directory / command-line arguments for the
`keeperfx` target under **Project -> keeperfx Debug Configuration** (or
`launch.vs.json`) to point at your DK game folder, matching the `args` used
by the VS Code configs above (`-level 00001 -campaign keeporig -nointro
-alex`).

To deploy a build into your actual game folder from VS (equivalent to the VS
Code deploy step), either use VS's CMake install support, or run from a
Developer Command Prompt:

```
cmake --install out/build/<preset> --prefix "D:/Games/DungeonKeeper"
```

For a path you don't want to type every time, copy
`CMakeUserPresets.json.example` to `CMakeUserPresets.json` (gitignored,
personal) and set `CMAKE_INSTALL_PREFIX` there instead.

## What's out of scope here

- CI still builds via the Makefile only; this doesn't change that.
- No new CMake toolchains were added - only VS Code / VS 2026 wiring for the
  presets that already existed (MSVC x86, Clang-cl x86, MinGW32 x86).
- Make and CMake are not unified into one build; both stay independent,
  first-class paths.
