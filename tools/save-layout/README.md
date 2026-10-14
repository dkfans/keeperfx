# Save layout probe

Reads the layout of every saved type from the compiler's debug info. The save tables check
(`tools/save-tables`) uses it to confirm that every member of every saved struct is in a field table.
Layout differences between builds no longer matter to saves, so nothing here compares layouts.

| File | Purpose |
|---|---|
| `roots.txt` | Saved types, the headers that declare them, and the files they end up in |
| `extract.sh` | Compiles a probe of the headers and writes every field's offset and size |
| `walk.py` | gdb script used by `extract.sh` to walk the types |
| `stub/ver_defs.h` | Stand-in for the header CMake generates |
| `fetch_sdl_headers.sh` | Downloads the SDL3 and SDL3_mixer MinGW headers the headers need at compile time |
