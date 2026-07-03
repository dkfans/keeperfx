#!/usr/bin/env python3

import sys


def load_hex(path):
    entries = {}

    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()

            if not line or ":" not in line:
                continue

            codepoint, bitmap = line.split(":", 1)
            entries[codepoint.upper()] = bitmap.upper()

    return entries


def save_hex(path, entries):
    with open(path, "w", encoding="utf-8") as f:
        for codepoint in sorted(entries.keys(), key=lambda x: int(x, 16)):
            f.write(f"{codepoint}:{entries[codepoint]}\n")


def main():
    if len(sys.argv) != 4:
        print("Usage:")
        print("  merge_hex.py base.hex replacement.hex output.hex")
        sys.exit(1)

    base_path = sys.argv[1]
    replacement_path = sys.argv[2]
    output_path = sys.argv[3]

    print(f"Loading base: {base_path}")
    base = load_hex(base_path)

    print(f"Loading replacements: {replacement_path}")
    replacement = load_hex(replacement_path)

    replaced = 0

    for codepoint, bitmap in replacement.items():
        if codepoint in base:
            replaced += 1

        base[codepoint] = bitmap

    print(f"Writing output: {output_path}")
    save_hex(output_path, base)

    print(f"Done.")
    print(f"Total glyphs: {len(base)}")
    print(f"Replaced/added: {replaced}")


if __name__ == "__main__":
    main()