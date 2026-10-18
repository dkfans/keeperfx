| File | Purpose |
|---|---|
| `dump_types.py` | gdb script: lists every field-table entry each saved struct needs, from the compiler's debug info |
| `tables.py` | `gen` writes the first version of all tables; `check` compares the tables in `src/kfx/save/core/schema/` with the dump |
| `check_tables.sh` | Runs the dump and the check for the current checkout; used by CI |
