import argparse
import os
import sys


def hexrow_to_bits(hexrow):
    row_bytes = bytes.fromhex(hexrow)
    bits = []
    for byte in row_bytes:
        for bit in range(8):
            bits.append((byte >> (7 - bit)) & 1)
    return bits


def bits_to_hexrow(bits):
    row_size = (len(bits) + 7) // 8
    row_bytes = bytearray(row_size)
    for x, bit in enumerate(bits):
        if bit:
            byte_index = x // 8
            bit_index = 7 - (x % 8)
            row_bytes[byte_index] |= 1 << bit_index
    return row_bytes.hex().upper()


def bdf_to_hex(bdf_path, out_path, target_height=12):
    font_descent = 0
    entries = {}

    encoding = None
    dwidth = None
    bbx = None
    bitmap_lines = []
    in_bitmap = False

    with open(bdf_path, "r", encoding="ascii", errors="ignore") as f:
        for raw_line in f:
            line = raw_line.rstrip("\n\r")
            stripped = line.strip()

            if stripped.startswith("FONTBOUNDINGBOX"):
                parts = stripped.split()
                if len(parts) == 5:
                    _, _, _, _, yoff = parts
                    font_descent = -int(yoff)
                continue

            if stripped.startswith("ENCODING"):
                encoding = int(stripped.split()[1])
                dwidth = None
                bbx = None
                bitmap_lines = []
                in_bitmap = False
                continue

            if stripped.startswith("DWIDTH"):
                parts = stripped.split()
                if len(parts) >= 2:
                    dwidth = int(parts[1])
                continue

            if stripped.startswith("BBX"):
                parts = stripped.split()
                if len(parts) == 5:
                    bbx = tuple(int(x) for x in parts[1:])
                continue

            if stripped == "BITMAP":
                bitmap_lines = []
                in_bitmap = True
                continue

            if stripped == "ENDCHAR":
                if encoding is not None:
                    width = dwidth if dwidth and dwidth > 0 else (bbx[0] if bbx else 8)
                    width = max(1, width)
                    rows = [[0] * width for _ in range(target_height)]

                    if bbx is not None:
                        bbx_width, bbx_height, bbx_xoff, bbx_yoff = bbx
                        top_y = bbx_yoff + bbx_height - 1
                        x_start = max(0, bbx_xoff)
                        bitmap_width = bbx_width
                    else:
                        top_y = target_height - 1
                        x_start = 0
                        bitmap_width = width

                    baseline_row = target_height - font_descent - 1
                    for row_index, row_hex in enumerate(bitmap_lines):
                        row_bits = hexrow_to_bits(row_hex)
                        y = top_y - row_index
                        out_row = baseline_row - y
                        if 0 <= out_row < target_height:
                            for bit_index in range(min(bitmap_width, len(row_bits))):
                                if row_bits[bit_index]:
                                    x = x_start + bit_index
                                    if 0 <= x < width:
                                        rows[out_row][x] = 1

                    hexrows = "".join(bits_to_hexrow(row) for row in rows)
                    entries[f"{encoding:04X}"] = hexrows

                encoding = None
                dwidth = None
                bbx = None
                bitmap_lines = []
                in_bitmap = False
                continue

            if in_bitmap and stripped:
                bitmap_lines.append(stripped)

    with open(out_path, "w", encoding="utf-8") as f:
        for codepoint in sorted(entries.keys(), key=lambda x: int(x, 16)):
            f.write(f"{codepoint}:{entries[codepoint]}\n")


def main():
    parser = argparse.ArgumentParser(
        description="Convert a BDF font to UniFont-style hex with a fixed target height."
    )
    parser.add_argument("input", help="Input BDF file")
    parser.add_argument("output", help="Output UniFont .hex file")
    parser.add_argument("--height", type=int, default=12, help="Target glyph height in pixels")
    args = parser.parse_args()

    if not os.path.isfile(args.input):
        print(f"Input file not found: {args.input}", file=sys.stderr)
        return 1

    bdf_to_hex(args.input, args.output, target_height=args.height)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
