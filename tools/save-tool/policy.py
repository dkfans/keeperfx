#!/usr/bin/env python3
"""Decides what a pull request has to do about the save format.

    policy.py --base DIR --head DIR --pr N --head-sha SHA [--labels a,b] --out result.json

DIR holds what kfx-savetool printed for that side of the PR: schema.txt (schema --build),
migrations.txt (migrations) and save_codec.h (for SAVE_MIN_READABLE_MAJOR). The result is a list of
findings, empty when the PR needs nothing. policy_comment.py turns it into a PR comment.

A finding is {"kind": ..., "args": [...]}; the kinds and what they say are in MESSAGES below.
"""
import argparse
import json
import re
import sys

BREAK_LABEL = "save-break-accepted"

INT_BITS = {"u8": 8, "i8": 8, "u16": 16, "i16": 16, "u32": 32, "i32": 32, "u64": 64, "i64": 64, "bool8": 8}


def read(path):
    with open(path, encoding="utf-8", errors="replace") as f:
        return f.read()


def read_side(path):
    schema = read(path + "/schema.txt").splitlines()
    migrations = [l for l in read(path + "/migrations.txt").splitlines() if l]
    header = read(path + "/save_codec.h")
    m = re.match(r"format (\d+)\.(\d+)$", schema[0]) if schema else None
    if not m:
        raise ValueError("%s/schema.txt has no format line" % path)
    min_readable = re.search(r"#define\s+SAVE_MIN_READABLE_MAJOR\s+(\d+)", header)
    return {
        "major": int(m.group(1)),
        "minor": int(m.group(2)),
        "min_readable": int(min_readable.group(1)) if min_readable else 0,
        "lines": set(schema[1:]),
        "fields": parse_fields(schema[1:]),
        "migrations": set(migrations),
    }


def parse_fields(lines):
    """Struct.field -> {type, dims, flags, aliases}; union member lines are left out."""
    fields = {}
    for line in lines:
        parts = line.split()
        if len(parts) < 4 or "::" in parts[0]:
            continue
        extra = {}
        for p in parts[4:]:
            if "=" in p:
                k, v = p.split("=", 1)
                extra[k] = v
        fields[parts[0]] = {
            "type": parts[1],
            "dims": [int(d) for d in re.findall(r"\[(\d+)\]", parts[2])],
            "flags": parts[3].split(","),
            "aliases": extra.get("aliases", "").split(",") if "aliases" in extra else [],
        }
    return fields


def kind_of(t):
    if t in INT_BITS:
        return "int"
    return t.split(":")[0]


def struct_of(path):
    return path.split(".", 1)[0]


def compare(base, head, labels):
    findings = []
    removed = {p: f for p, f in base["fields"].items() if p not in head["fields"]}
    added = {p: f for p, f in head["fields"].items() if p not in base["fields"]}
    new_migrations = {l.split()[0] for l in head["migrations"] - base["migrations"]}

    # A removed field next to an added field of the same type, without an alias back, reads as a rename
    # that would lose the old value.
    for new_path, nf in sorted(added.items()):
        for old_path, of in sorted(removed.items()):
            if struct_of(new_path) != struct_of(old_path) or of["type"] != nf["type"]:
                continue
            old_name = old_path.split(".", 1)[1]
            if old_name in nf["aliases"]:
                continue
            findings.append({"kind": "missing_alias", "args": [old_path, new_path]})
            break

    for path, bf in sorted(base["fields"].items()):
        hf = head["fields"].get(path)
        if hf is None:
            continue
        migrated = struct_of(path) in new_migrations
        if bf["type"] != hf["type"]:
            bk, hk = kind_of(bf["type"]), kind_of(hf["type"])
            if bk == "int" and hk == "int":
                if INT_BITS[hf["type"]] < INT_BITS[bf["type"]] and not migrated and "clamp" not in hf["flags"]:
                    findings.append({"kind": "narrowed", "args": [path, bf["type"], hf["type"]]})
            elif not migrated:
                findings.append({"kind": "kind_changed", "args": [path, bf["type"], hf["type"]]})
        if bf["dims"] != hf["dims"] and not migrated:
            if len(bf["dims"]) != len(hf["dims"]):
                findings.append({"kind": "kind_changed", "args": [path, dims_text(bf["dims"]), dims_text(hf["dims"])]})
            elif any(h < b for b, h in zip(bf["dims"], hf["dims"])):
                findings.append({"kind": "shrunk", "args": [path, dims_text(bf["dims"]), dims_text(hf["dims"])]})

    breaks = []
    if head["major"] != base["major"]:
        breaks.append("SAVE_FORMAT_MAJOR")
    if head["min_readable"] != base["min_readable"]:
        breaks.append("SAVE_MIN_READABLE_MAJOR")
    if base["migrations"] - head["migrations"]:
        breaks.append("migration")
    if breaks and BREAK_LABEL not in labels:
        findings.append({"kind": "break", "args": breaks})

    if head["lines"] != base["lines"] and head["minor"] <= base["minor"] and head["major"] == base["major"]:
        findings.insert(0, {"kind": "minor_bump", "args": [str(base["minor"]), str(base["minor"] + 1)]})
    return findings


def dims_text(dims):
    return "".join("[%d]" % d for d in dims) or "-"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--base", required=True)
    ap.add_argument("--head", required=True)
    ap.add_argument("--pr", required=True, type=int)
    ap.add_argument("--head-sha", required=True)
    ap.add_argument("--labels", default="")
    ap.add_argument("--out", required=True)
    a = ap.parse_args()
    labels = {l for l in a.labels.split(",") if l}
    result = {"pr": a.pr, "head_sha": a.head_sha, "findings": []}
    result["findings"] = compare(read_side(a.base), read_side(a.head), labels)
    with open(a.out, "w", encoding="utf-8") as f:
        json.dump(result, f)
    return 0


if __name__ == "__main__":
    sys.exit(main())
