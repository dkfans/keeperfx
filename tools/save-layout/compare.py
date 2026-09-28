#!/usr/bin/env python3
"""Compare two saved-type layouts (from extract.sh) and classify the change.

    compare.py <roots.txt> <base.tsv> <head.tsv> <result.json> <summary.md>
               [--flags-tsv <head built with debug flags>] [--lua-changed]

Per root type the result is one of:
    identical   no layout change
    compatible  only union members added or removed; every existing field kept
                its offset and size
    refused     the type's size changed; today's loader refuses such files
    silent      same size but existing data moved; old files load wrong values
"""
import argparse
import collections
import json
import re
import sys

STATE_ORDER = ["identical", "compatible", "refused", "silent"]

FILE_NAMES = {
    "savegame": "Saved games (fx1g*.sav)",
    "continue": "Continue file (fx1contn.sav)",
    "replay": "Replays (*.pck)",
    "resync": "Multiplayer resync",
    "highscores": "High scores",
    "netconfig": "Network config (fxconfig.net)",
}

AREAS = [
    ("config snapshot (Game.conf)", r"^Game\.conf\."),
    ("level script (Game.script)", r"^Game\.script\."),
    ("things / creatures", r"^Game\.(things_data|cctrl_data)"),
    ("dungeons / players / AI", r"^Game\.(dungeon|players|computer|computer_task)\b"),
    ("per-user UI / network / debug", r"^Game\.(user_states|packets|packet_|host_checksums|log_snapshot|input_lag)|^Packet\b"),
    ("map / rooms / world", r"^Game\.(map|slabmap|rooms|columns_data|lish|navigation|event)\b"),
    ("other Game fields", r"^Game\."),
    ("other saved types", r"."),
]


def load_roots(path):
    roots = collections.OrderedDict()
    for line in open(path, encoding="utf-8"):
        parts = line.split()
        if len(parts) >= 3 and parts[0] == "root":
            name = parts[1][len("typedef:"):] if parts[1].startswith("typedef:") else parts[1]
            roots[name] = parts[2:]
    return roots


def load_layout(path):
    rows = collections.OrderedDict()
    for line in open(path, encoding="utf-8"):
        p, off, size, kind, ty = line.rstrip("\n").split("\t")
        rows[p] = (off, size, kind, ty)
    return rows


def root_of(p):
    return re.split(r"[.\[]", p, maxsplit=1)[0]


def is_leaf(kind):
    return not (kind in ("struct", "union") or kind.startswith("array["))


def area(p):
    for name, rx in AREAS:
        if re.search(rx, p):
            return name
    return "?"


def shallow(paths):
    out = []
    for p in sorted(paths):
        if not any(p.startswith(q + ".") or p.startswith(q + "[]") for q in out):
            out.append(p)
    return out


def in_union(rows, p):
    q = p
    while "." in q:
        q = q.rsplit(".", 1)[0]
        base = q[:-2] if q.endswith("[]") else q
        if rows.get(base, ("", "", ""))[2] == "union" or rows.get(q, ("", "", ""))[2] == "union":
            return True
    return False


def root_state(old, new, r):
    if old[r][1] != new[r][1]:
        return "refused"
    sig_o = [(v[0], v[1]) for p, v in old.items() if root_of(p) == r and is_leaf(v[2])]
    sig_n = [(v[0], v[1]) for p, v in new.items() if root_of(p) == r and is_leaf(v[2])]
    # Same bytes in the same places: a raw-memory file still loads correctly,
    # even if fields were renamed or retyped at the same size.
    if sig_o == sig_n:
        return "identical"
    for p in old:
        if root_of(p) == r and p in new and old[p][:2] != new[p][:2]:
            return "silent"
    changed = [p for p in set(old) ^ set(new) if root_of(p) == r]
    if all(in_union(old if p in old else new, p) for p in changed):
        return "compatible"
    return "silent"


def describe(old, new):
    d = {"added": [], "removed": [], "renamed": [], "moved": [], "resized_arrays": [], "type_changes": []}
    added = shallow(set(new) - set(old))
    removed = shallow(set(old) - set(new))
    for p in old:
        if p not in new:
            continue
        ok, nk = old[p][2], new[p][2]
        if ok.startswith(("array[", "leafarray[")) and nk.startswith(("array[", "leafarray[")) and ok != nk:
            d["resized_arrays"].append("%s: %s -> %s" % (p, ok.split("[")[1][:-1], nk.split("[")[1][:-1]))
        elif is_leaf(ok) and (old[p][1] != new[p][1] or old[p][3] != new[p][3]):
            d["type_changes"].append("%s: %s (%s bytes) -> %s (%s bytes)" % (p, old[p][3], old[p][1], new[p][3], new[p][1]))
    left = list(added)
    for r in list(removed):
        par = r.rsplit(".", 1)[0]
        for a in left:
            if a.rsplit(".", 1)[0] == par and old[r][:2] == new[a][:2]:
                d["renamed"].append("%s -> %s" % (r, a.rsplit(".", 1)[1]))
                removed.remove(r)
                left.remove(a)
                break
    for r in list(removed):
        nm = r.rsplit(".", 1)[-1]
        for a in left:
            if a.rsplit(".", 1)[-1] == nm:
                d["moved"].append("%s -> %s" % (r, a))
                removed.remove(r)
                left.remove(a)
                break
    d["added"] = left
    d["removed"] = removed
    return d


def change_paths(d):
    paths = list(d["added"]) + list(d["removed"])
    paths += [x.split(":")[0] for x in d["resized_arrays"] + d["type_changes"]]
    for x in d["renamed"] + d["moved"]:
        paths += [x.split(" -> ")[0]]
    for x in d["moved"]:
        paths += [x.split(" -> ")[1]]
    return paths


def worst(states):
    return max(states, key=STATE_ORDER.index) if states else "identical"


def analyse(roots, old, new, flags_rows=None, lua_changed=False):
    states = collections.OrderedDict()
    for r in roots:
        if r not in old or r not in new:
            states[r] = "identical" if r not in old and r not in new else "refused"
        else:
            states[r] = root_state(old, new, r)
    files = collections.OrderedDict()
    for r, fl in roots.items():
        for f in fl:
            files.setdefault(f, []).append(states[r])
    files = collections.OrderedDict((f, worst(s)) for f, s in files.items())

    detail = describe(old, new)
    by_area = collections.OrderedDict()
    for p in change_paths(detail):
        by_area.setdefault(area(p), set()).add(p)

    flag_diff = []
    if flags_rows is not None:
        flag_diff = sorted(p for p in set(flags_rows) | set(new)
                           if flags_rows.get(p, ("",) * 4)[:3] != new.get(p, ("",) * 4)[:3])

    verdict = worst(list(states.values()))
    return {
        "verdict": verdict,
        "roots": {r: {"state": s, "base_size": int(old[r][1]) if r in old else None,
                      "head_size": int(new[r][1]) if r in new else None} for r, s in states.items()},
        "files": files,
        "changes": detail,
        "areas": {k: sorted(v) for k, v in by_area.items()},
        "lua_changed": lua_changed,
        "flag_dependent_fields": flag_diff,
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("roots")
    ap.add_argument("base")
    ap.add_argument("head")
    ap.add_argument("result_json")
    ap.add_argument("summary_md")
    ap.add_argument("--flags-tsv")
    ap.add_argument("--lua-changed", action="store_true")
    ap.add_argument("--subject", choices=["pr", "build"], default="pr",
                    help="word the summary for a pull request or for a master build")
    a = ap.parse_args()

    result = analyse(load_roots(a.roots), load_layout(a.base), load_layout(a.head),
                     load_layout(a.flags_tsv) if a.flags_tsv else None, a.lua_changed)
    verdict = result["verdict"]
    json.dump(result, open(a.result_json, "w", encoding="utf-8"), indent=1)
    open(a.summary_md, "w", encoding="utf-8").write(render(result, a.subject))
    print(verdict)


def render(r, subject="pr"):
    this = "This PR" if subject == "pr" else "This build"
    L = ["<!-- save-format-check -->", "## Save format check", ""]
    v = r["verdict"]
    if v == "identical":
        L.append("No change to the layout of saved data.")
    elif v == "compatible":
        L.append("The layout of saved data changed, but old files still load correctly (union members only).")
    elif v == "refused":
        L.append("**%s changes the save format.** Saves, and the other files listed below, "
                 "made by earlier builds will **not load** after it." % this)
    else:
        L.append(("**%s changes the save format without changing its size.** Files made by earlier "
                  "builds will **load with wrong values, with no error**. Avoid this: keep existing fields "
                  "where they are, or change the size so the loader refuses old files.") % this)
    if r["lua_changed"]:
        L += ["", "The Lua serialiser changed (`serialisation.lua` / `binser.lua`). "
              "Data saved by Lua scripts can't be checked automatically; review it for compatibility."]
    if r["flag_dependent_fields"]:
        L += ["", "**The saved layout depends on build flags** (`BFDEBUG_LEVEL`, `FUNCTESTING`): "
              "the heavy-logging build would write files the normal build can't read. Fields:", ""]
        L += ["- `%s`" % p for p in r["flag_dependent_fields"][:20]]
    if v == "identical" and not r["flag_dependent_fields"]:
        return "\n".join(L) + "\n"

    L += ["", "| File | Result |", "|---|---|"]
    label = {"identical": "unchanged", "compatible": "compatible", "refused": "**won't load**",
             "silent": "**loads wrong data**"}
    for f, s in r["files"].items():
        L.append("| %s | %s |" % (FILE_NAMES.get(f, f), label[s]))
    L += ["", "| Type | Base size | Head size | Result |", "|---|---|---|---|"]
    for t, x in r["roots"].items():
        if x["state"] != "identical":
            L.append("| `%s` | %s | %s | %s |" % (t, x["base_size"], x["head_size"], label[x["state"]]))

    titles = [("added", "Added"), ("removed", "Removed"), ("renamed", "Renamed"), ("moved", "Moved to another struct"),
              ("resized_arrays", "Array resized"), ("type_changes", "Type changed")]
    L += ["", "### What changed", ""]
    for k, t in titles:
        items = r["changes"][k]
        if not items:
            continue
        L.append("**%s** (%d)" % (t, len(items)))
        L.append("")
        for i in items[:40]:
            L.append("- `%s`" % i)
        if len(items) > 40:
            L.append("- … and %d more (see the job artifact)" % (len(items) - 40))
        L.append("")
    if r["areas"]:
        L += ["### Where", "", "| Area | Fields |", "|---|---|"]
        for k, v2 in r["areas"].items():
            L.append("| %s | %d |" % (k, len(v2)))
        L.append("")
    if v in ("refused", "silent") and subject == "pr":
        L += ["If this break is intended, a maintainer can accept it with the `save-break-accepted` label. "
              "Consider merging it together with other open PRs that break saves, so players lose their saves once."]
    return "\n".join(L) + "\n"


if __name__ == "__main__":
    sys.exit(main())
