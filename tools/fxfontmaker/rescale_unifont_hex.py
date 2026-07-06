#!/usr/bin/env python3
"""Rescale UniFont .hex glyphs from 16px height to 12px height.

This script reads a UniFont-style .hex file where each glyph is stored as
height*row_bytes raw bitmap bytes, then writes a new .hex file with each
glyph scaled to 12 rows and an appropriately reduced width.

Usage:
  python rescale_unifont_hex.py input.hex output.hex
"""

import argparse
import os
import sys

SOURCE_HEIGHT = 16
TARGET_HEIGHT = 12


def parse_unifont_hex_line(line, line_no, source_height):
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

    if len(data_hex) == 0:
        raise ValueError(f"Empty glyph data on line {line_no}: {line}")

    if len(data_hex) % 2 != 0:
        raise ValueError(f"Odd number of hex digits on line {line_no}")

    data_bytes = bytes.fromhex(data_hex)
    if len(data_bytes) % source_height != 0:
        raise ValueError(
            f"Glyph data length must be a multiple of {source_height} on line {line_no}: got {len(data_bytes)} bytes"
        )

    row_size = len(data_bytes) // source_height
    width = row_size * 8
    return codepoint, width, row_size, data_bytes


def bitmap_from_bytes(data_bytes, height, row_size):
    width = row_size * 8
    bitmap = [[0] * width for _ in range(height)]
    for y in range(height):
        row_bytes = data_bytes[y * row_size : (y + 1) * row_size]
        for bx, b in enumerate(row_bytes):
            for bit in range(8):
                x = bx * 8 + bit
                bitmap[y][x] = (b >> (7 - bit)) & 1
    return bitmap


def bytes_from_bitmap(bitmap, row_size):
    height = len(bitmap)
    width = len(bitmap[0])
    assert width == row_size * 8
    out = bytearray()
    for row in bitmap:
        for bx in range(row_size):
            byte = 0
            for bit in range(8):
                byte = (byte << 1) | (1 if row[bx * 8 + bit] else 0)
            out.append(byte)
    return bytes(out)


def rescale_bitmap(source, target_width, target_height):
    source_height = len(source)
    source_width = len(source[0])
    if target_width <= 0 or target_height <= 0:
        raise ValueError("Target dimensions must be positive")

    if source_width == target_width and source_height == target_height:
        return [row[:] for row in source]

    x_scale = (source_width - 1) / max(1, target_width - 1)
    y_scale = (source_height - 1) / max(1, target_height - 1)

    dest = [[0] * target_width for _ in range(target_height)]
    for ty in range(target_height):
        sy = int(round(ty * y_scale))
        sy = min(source_height - 1, sy)
        for tx in range(target_width):
            sx = int(round(tx * x_scale))
            sx = min(source_width - 1, sx)
            dest[ty][tx] = source[sy][sx]
    return dest


def read_unifont_hex(path, source_height):
    glyphs = []
    with open(path, "r", encoding="utf-8") as f:
        for line_no, line in enumerate(f, start=1):
            parsed = parse_unifont_hex_line(line, line_no, source_height)
            if parsed is None:
                continue
            codepoint, width, row_size, data_bytes = parsed
            glyphs.append((codepoint, width, row_size, data_bytes))
    return glyphs


def write_unifont_hex(path, glyphs):
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        for codepoint, data_bytes in glyphs:
            code_hex = f"{codepoint:04X}"
            f.write(f"{code_hex}:{data_bytes.hex().upper()}\n")


def main():
    parser = argparse.ArgumentParser(
        description="Rescale UniFont .hex glyphs from 16px height to 12px height."
    )
    parser.add_argument("input", help="Input UniFont .hex file")
    parser.add_argument("output", help="Output UniFont .hex file")
    parser.add_argument(
        "--source-height",
        type=int,
        default=SOURCE_HEIGHT,
        help=f"Source glyph height in pixels (default: {SOURCE_HEIGHT})",
    )
    parser.add_argument(
        "--target-height",
        type=int,
        default=TARGET_HEIGHT,
        help=f"Target glyph height in pixels (default: {TARGET_HEIGHT})",
    )
    args = parser.parse_args()

    if not os.path.isfile(args.input):
        print(f"Input file not found: {args.input}", file=sys.stderr)
        return 1

    if args.source_height <= 0 or args.target_height <= 0:
        print("Source and target heights must be positive integers.", file=sys.stderr)
        return 1

    glyphs = read_unifont_hex(args.input, args.source_height)
    scaled_glyphs = []

    for codepoint, width, row_size, data_bytes in glyphs:
        source_bitmap = bitmap_from_bytes(data_bytes, args.source_height, row_size)
        target_row_size = max(1, int(round(row_size * args.target_height / args.source_height)))
        target_width = target_row_size * 8
        scaled_bitmap = rescale_bitmap(source_bitmap, target_width, args.target_height)
        scaled_bytes = bytes_from_bitmap(scaled_bitmap, target_row_size)
        scaled_glyphs.append((codepoint, scaled_bytes))

    write_unifont_hex(args.output, scaled_glyphs)
    print(f"Wrote rescaled file: {args.output}")
    print(f"Source height: {args.source_height}, target height: {args.target_height}")
    print(f"Glyphs processed: {len(scaled_glyphs)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
