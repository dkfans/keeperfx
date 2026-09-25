# Save layout check

## Results

| Result | Meaning |
|---|---|
| identical | No change to saved data |
| compatible | Only union members added or removed; existing data kept its place |
| refused | A saved type changed size; older files won't load |
| silent | Same size, but existing data moved; older files load **wrong values** |

## Files

| File | Purpose |
|---|---|
| `roots.txt` | Saved types, the headers that declare them, and the files they end up in |
| `extract.sh` | Compiles a probe of the headers and writes every field's offset and size |
| `walk.py` | gdb script used by `extract.sh` to walk the types |
| `stub/ver_defs.h` | Stand-in for the header CMake generates |
| `fetch_sdl_headers.sh` | Downloads the SDL3 and SDL3_mixer MinGW headers the probe needs |
| `compare.py` | Classifies the change and writes `result.json` and a Markdown summary |
| `check_pr.sh` | Runs all of the above for the current checkout against a base commit |
