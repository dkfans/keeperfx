#!/usr/bin/env python3
"""Merge KeeperFX gtext PO translations into a single translation.toml."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path
from typing import Dict, List, Optional

PO_REF_RE = re.compile(r"guitext:(\d+)")
QUOTED_RE = re.compile(r'^"(.*)"$')


def parse_po_file(path: Path, use_msgid_fallback: bool = False) -> Dict[int, str]:
    entries: Dict[int, str] = {}
    current_block: Optional[int] = None
    current_field: Optional[str] = None
    current_msgid: Optional[str] = None
    current_msgstr: str = ""
    current_refs: List[str] = []

    def finalize_entry() -> None:
        nonlocal current_block, current_msgid, current_msgstr, current_refs
        if current_block is None or current_msgid is None:
            return
        if current_msgstr != "":
            value = current_msgstr
        elif use_msgid_fallback:
            value = current_msgid
        else:
            value = ""
        entries[current_block] = value

    def parse_quoted(line: str) -> str:
        match = QUOTED_RE.match(line.strip())
        if not match:
            return ""
        raw = match.group(1)
        result: List[str] = []
        i = 0
        while i < len(raw):
            ch = raw[i]
            if ch != "\\":
                result.append(ch)
                i += 1
                continue
            i += 1
            if i >= len(raw):
                break
            esc = raw[i]
            i += 1
            if esc == "n":
                result.append("\n")
            elif esc == "t":
                result.append("\t")
            elif esc == "r":
                result.append("\r")
            elif esc == '"':
                result.append('"')
            elif esc == "'":
                result.append("'")
            elif esc == "\\":
                result.append("\\")
            elif esc == "u" and i + 4 <= len(raw):
                result.append(chr(int(raw[i:i + 4], 16)))
                i += 4
            elif esc == "U" and i + 8 <= len(raw):
                result.append(chr(int(raw[i:i + 8], 16)))
                i += 8
            elif esc == "x" and i + 2 <= len(raw):
                result.append(chr(int(raw[i:i + 2], 16)))
                i += 2
            else:
                result.append(esc)
        return "".join(result)

    with path.open("r", encoding="utf-8") as handle:
        for line in handle:
            line = line.rstrip("\n")
            if line.startswith("#:"):
                refs = line[2:].strip().split()
                current_refs.extend(refs)
                continue

            if line.startswith("msgid"):
                if current_msgid is not None and current_field == "msgstr":
                    finalize_entry()
                    current_refs = []
                current_field = "msgid"
                current_block = None
                current_msgid = ""
                current_msgstr = ""
                text = line[len("msgid"):].strip()
                if text:
                    current_msgid = parse_quoted(text)
                continue

            if line.startswith("msgstr"):
                if current_msgid is None:
                    continue
                current_field = "msgstr"
                current_msgstr = ""
                text = line[len("msgstr"):].strip()
                if text:
                    current_msgstr = parse_quoted(text)
                current_block = None
                for ref in current_refs:
                    match = PO_REF_RE.search(ref)
                    if match:
                        current_block = int(match.group(1))
                        break
                continue

            if line.startswith('"') and current_field in {"msgid", "msgstr"}:
                if current_field == "msgid":
                    current_msgid += parse_quoted(line)
                else:
                    current_msgstr += parse_quoted(line)
                continue

            if line.strip() == "" and current_field == "msgstr":
                finalize_entry()
                current_refs = []
                current_block = None
                current_field = None
                current_msgid = None
                current_msgstr = ""
                continue

    if current_field == "msgstr" and current_block is not None and current_msgid is not None:
        finalize_entry()

    return entries


def build_language_name(path: Path) -> str:
    name = path.stem
    if name.startswith("gtext_"):
        return name[len("gtext_"):].upper()
    return name.upper()


def toml_escape(value: str) -> str:
    text = value.replace("\\", "\\\\").replace("\"", "\\\"")
    text = text.replace("\t", "\\t").replace("\r", "\\r")
    text = text.replace("\n", "\\n")
    return text


def toml_literal_multiline(value: str) -> str:
    escaped = value.replace('"""', '\\"\"\"')
    return f'"""{escaped}"""'


def format_value(value: str) -> str:
    if value == "":
        return ''
    if "\n" in value or len(value) > 80 or '"' in value:
        escaped = toml_escape(value).replace('"""', '\\"\"\"')
        return f'"""{escaped}"""'
    return f'"{toml_escape(value)}"'


def write_translation_toml(out_path: Path, block_ids: List[int], language_codes: List[str], translations: Dict[str, Dict[int, str]]) -> None:
    lines: List[str] = []
    for idx in block_ids:
        lines.append(f"[{idx}]")
        for code in language_codes:
            value = translations.get(code, {}).get(idx, "")
            if value == "":
                lines.append(f"#{code} = ")
            else:
                lines.append(f"{code} = {format_value(value)}")
        lines.append("")
    out_path.write_text("\n".join(lines), encoding="utf-8")


def main(argv: Optional[List[str]] = None) -> int:
    parser = argparse.ArgumentParser(description="Merge gtext PO translations into translation.toml")
    parser.add_argument("--lang-dir", default="lang", help="Directory containing gtext_*.po files")
    parser.add_argument("--output", default="translation.toml", help="Output TOML file")
    parser.add_argument("--source", default="gtext_eng.pot", help="English source PO/POT file")
    args = parser.parse_args(argv)

    lang_dir = Path(args.lang_dir)
    if not lang_dir.is_dir():
        print(f"Error: language directory not found: {lang_dir}", file=sys.stderr)
        return 1

    source_path = lang_dir / args.source
    if not source_path.exists():
        print(f"Error: English source file not found: {source_path}", file=sys.stderr)
        return 2

    english_entries = parse_po_file(source_path, use_msgid_fallback=True)
    if not english_entries:
        print(f"Error: no entries parsed from {source_path}", file=sys.stderr)
        return 3

    translations: Dict[str, Dict[int, str]] = {}
    translations["ENG"] = english_entries

    for path in sorted(lang_dir.glob("gtext_*.po")):
        code = build_language_name(path)
        entries = parse_po_file(path)
        translations[code] = entries

    language_codes = ["ENG"] + [code for code in sorted(translations) if code != "ENG"]
    block_ids = sorted({idx for idx in english_entries})

    write_translation_toml(Path(args.output), block_ids, language_codes, translations)
    print(f"Wrote {args.output} with {len(block_ids)} blocks and {len(language_codes)} languages.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
