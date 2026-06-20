import argparse
import os
import sys

def bdf_to_hex(bdf_path, out_path):
    with open(bdf_path, "r", encoding="ascii", errors="ignore") as f:
        lines = [l.strip() for l in f]

    out = []

    codepoint = None
    bitmap = []
    in_bitmap = False

    for line in lines:
        if line.startswith("ENCODING"):
            codepoint = int(line.split()[1])

        elif line == "BITMAP":
            bitmap = []
            in_bitmap = True

        elif line == "ENDCHAR":
            if codepoint is not None:
                hexrows = "".join(bitmap)
                out.append(f"{codepoint:04X}:{hexrows}")
            in_bitmap = False

        elif in_bitmap:
            bitmap.append(line)

    with open(out_path, "w") as f:
        f.write("\n".join(out))

def main():
    parser = argparse.ArgumentParser(
        description="Convert UniFont hex to a binary lookup table + glyph data block."
    )
    parser.add_argument("input", help="Input UniFont .hex file")
    parser.add_argument("output", help="Output binary file")
    args = parser.parse_args()

    if not os.path.isfile(args.input):
        print(f"Input file not found: {args.input}", file=sys.stderr)
        return 1

    bdf_to_hex(args.input, args.output)

    return 0

if __name__ == "__main__":
    raise SystemExit(main())