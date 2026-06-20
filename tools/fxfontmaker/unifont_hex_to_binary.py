#!/usr/bin/env python3
"""Convert UniFont hex to a simple binary form.

Output format:
- 65,536 index entries (one per BMP codepoint 0x0000..0xFFFF)
- Each entry is 6 bytes: <width:uint16><offset:uint32>
- Offset is relative to the start of the glyph data block, which follows the index.
- Glyph data is raw bytes converted from the UniFont hex payload.
- Missing codepoints remain with width=0, offset=0.

Example:
  001F:AAAA000180... -> width=16, data bytes length=32
  005A:000000007E... -> width=8, data bytes length=16
"""

import argparse
import os
import struct
import sys

INDEX_COUNT = 65536
INDEX_FORMAT = "<HI"  # width:uint16, offset:uint32
INDEX_ENTRY_SIZE = struct.calcsize(INDEX_FORMAT)
HEIGHT = 16


def parse_unifont_hex_line(line, line_no, height):
    line = line.strip()
    if not line or line.startswith("#"):
        return None

    if ":" not in line:
        raise ValueError(f"Invalid UniFont line {line_no}: missing ':'")

    code_hex, data_hex = line.split(":", 1)
    code_hex = code_hex.strip()
    data_hex = data_hex.strip()

    if not code_hex:
        raise ValueError(f"Invalid UniFont line {line_no}: empty codepoint")

    try:
        codepoint = int(code_hex, 16)
    except ValueError:
        raise ValueError(f"Invalid codepoint on line {line_no}: {code_hex}")

    if codepoint < 0 or codepoint > 0xFFFF:
        raise ValueError(f"Codepoint out of BMP range on line {line_no}: {code_hex}")

    if len(data_hex) == 0:
        raise ValueError(f"Empty glyph data on line {line_no}: {line}")

    if len(data_hex) % 2 != 0:
        raise ValueError(f"Odd number of hex digits on line {line_no}")

    data_bytes = bytes.fromhex(data_hex)

    if len(data_bytes) % height != 0:
        raise ValueError(
            f"Glyph data length must be a multiple of {height} on line {line_no}: got {len(data_bytes)} bytes"
        )

    row_size = len(data_bytes) // height
    width = row_size * 8
    return codepoint, width, data_bytes


def build_index_and_data(glyphs):
    index_entries = [(0, 0)] * INDEX_COUNT
    data_blocks = [b""] * INDEX_COUNT

    for codepoint, (width, data_bytes) in glyphs.items():
        index_entries[codepoint] = (width, 0)  # offset assigned later
        data_blocks[codepoint] = data_bytes

    offset = 0
    for codepoint in range(INDEX_COUNT):
        width, _ = index_entries[codepoint]
        if width == 0:
            continue
        index_entries[codepoint] = (width, offset)
        offset += len(data_blocks[codepoint])

    data_block = b"".join(data_blocks)
    return index_entries, data_block


def write_output(path, index_entries, data_block):
    with open(path, "wb") as out_file:
        for width, offset in index_entries:
            out_file.write(struct.pack(INDEX_FORMAT, width, offset))
        out_file.write(data_block)


def read_unifont_hex(path, height):
    glyphs = {}
    with open(path, "r", encoding="utf-8") as f:
        for line_no, line in enumerate(f, start=1):
            parsed = parse_unifont_hex_line(line, line_no, height)
            if parsed is None:
                continue
            codepoint, width, data_bytes = parsed
            if codepoint in glyphs:
                raise ValueError(
                    f"Duplicate glyph definition for U+{codepoint:04X} on line {line_no}"
                )
            glyphs[codepoint] = (width, data_bytes)
    return glyphs


def main():
    parser = argparse.ArgumentParser(
        description="Convert UniFont hex to a binary lookup table + glyph data block."
    )
    parser.add_argument("input", help="Input UniFont .hex file")
    parser.add_argument("output", help="Output binary file")
    parser.add_argument("height", type=int, help="Glyph height in pixels")
    args = parser.parse_args()

    if not os.path.isfile(args.input):
        print(f"Input file not found: {args.input}", file=sys.stderr)
        return 1

    glyphs = read_unifont_hex(args.input, args.height)
    index_entries, data_block = build_index_and_data(glyphs)
    write_output(args.output, index_entries, data_block)

    print(f"Wrote {args.output}")
    print(f"Index entries: {INDEX_COUNT} x {INDEX_ENTRY_SIZE} bytes = {INDEX_COUNT * INDEX_ENTRY_SIZE}")
    print(f"Glyph data size: {len(data_block)} bytes")
    print(f"Total output size: {INDEX_COUNT * INDEX_ENTRY_SIZE + len(data_block)} bytes")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
