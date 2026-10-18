#!/usr/bin/env python3
"""Field tables from the compiler's debug info.

  tables.py gen   <types.tsv> <roots.txt> <out dir>     bootstrap all tables
  tables.py check <types.tsv> <tables dir>              every member of every saved struct must be listed

types.tsv comes from dump_types.py (see check_tables.sh).
"""
import collections
import glob
import os
import re
import sys

AREAS = {
    "things": ["things_data", "cctrl_data", "thing_lists", "battles"],
    "world": ["columns_data", "slabset", "slabobjs", "lish", "map", "slabmap", "rooms", "event", "lightst",
              "fx_lines", "action_points", "gold_lookup", "texture_animation"],
    "dungeon": ["players", "user_states", "computer_task", "computer", "dungeon", "creature_scores", "bookmark",
                "pool"],
    "script": ["script", "script_variables"],
    "config": ["conf"],
}
AREA_FILE = {"game": "schema_game.c", "things": "schema_things.c", "world": "schema_world.c",
             "dungeon": "schema_dungeon.c", "script": "schema_script.c", "config": "schema_config.c",
             "misc": "schema_misc.c"}
ROOT_ORDER = ["Game", "CatalogueEntry", "FileChunkHeader", "IntralevelData", "PacketSaveHead", "Packet",
              "HighScore", "ConfigInfo"]

# Unions that have a discriminator field. A group with one member gets that member's own table; a group
# with several members stores the bytes of the largest; a group without members stores nothing. Values
# no group lists fall to a default that stores the whole union. Unions not listed here stay raw bytes.
UNIONS = {
    ("Thing", "union:valuable"): {
        "selector": "thing_arm",
        "groups": [
            {"label": "none", "values": ["STA_None"], "members": []},
            {"label": "object_bytes", "values": ["STA_ObjectBytes"], "shared": "heart",
             "members": ["custom_box", "call_to_arms_flag", "heart", "hero_gate", "lightning"]},
            {"label": "valuable", "values": ["STA_Valuable"], "members": ["valuable"]},
            {"label": "food", "values": ["STA_Food"], "members": ["food"]},
            {"label": "lair", "values": ["STA_Lair"], "shared": "lair", "members": ["lair", "torturer"]},
            {"label": "armor", "values": ["STA_Armor"], "shared": "armor", "members": ["armor", "disease"]},
            {"label": "roomflag", "values": ["STA_RoomFlag"], "members": ["roomflag"]},
            {"label": "shot", "values": ["STA_Shot"], "members": ["shot"]},
            {"label": "shot_lizard", "values": ["STA_ShotLizard"], "members": ["shot_lizard"]},
            {"label": "corpse", "values": ["STA_Corpse"], "members": ["corpse"]},
            {"label": "creature", "values": ["STA_Creature"], "members": ["creature"]},
            {"label": "shot_effect", "values": ["STA_ShotEffect"], "members": ["shot_effect"]},
            {"label": "price_effect", "values": ["STA_PriceEffect"], "members": ["price_effect"]},
            {"label": "effect_generator", "values": ["STA_EffectGenerator"], "members": ["effect_generator"]},
            {"label": "trap", "values": ["STA_Trap"], "members": ["trap"]},
            {"label": "door", "values": ["STA_Door"], "members": ["door"]},
            {"label": "cave_in", "values": ["STA_CaveIn"], "members": ["cave_in"]},
        ],
    },
    ("ComputerTask", "union:sell_traps_doors"): {
        "disc": "ttype",
        "groups": [
            {"label": "none", "values": ["CTT_None"], "members": []},
            {"label": "sell_traps_doors", "values": ["CTT_SellTrapsAndDoors"], "members": ["sell_traps_doors"]},
            {"label": "move_gold", "values": ["CTT_MoveGoldToTreasury"], "members": ["move_gold"]},
            {"label": "magic_cta", "values": ["CTT_MagicCallToArms"], "members": ["magic_cta"]},
            {"label": "attack_magic", "values": ["CTT_AttackMagic", "CTT_MagicSpeedUp"], "members": ["attack_magic"]},
            {"label": "slap_diggers", "values": ["CTT_SlapDiggers"], "members": ["slap_imps"]},
            {"label": "sacrifice_diggers", "values": ["CTT_SacrificeDiggers"], "members": ["sacrifice_imp"]},
            {"label": "move_to_room", "values": ["CTT_MoveCreatureToRoom"], "members": ["move_to_room"]},
            {"label": "move_to_defend", "values": ["CTT_MoveCreaturesToDefend"], "members": ["move_to_defend"]},
            {"label": "move_to_pos", "values": ["CTT_MoveCreatureToPos"], "members": ["move_to_pos"]},
            {"label": "pickup_for_attack", "values": ["CTT_PickupForAttack"], "members": ["pickup_for_attack"]},
            {"label": "dig_to_room", "values": ["CTT_DigToEntrance"], "members": ["dig_to_room"]},
            {"label": "dig_to_gold", "values": ["CTT_DigToGold"], "members": ["dig_to_gold"]},
            {"label": "dig_somewhere", "values": ["CTT_DigToAttack", "CTT_DigToNeutral"], "members": ["dig_somewhere"]},
            {"label": "create_room", "values": ["CTT_DigRoomPassage", "CTT_DigRoom", "CTT_CheckRoomDug", "CTT_PlaceRoom"],
             "members": ["create_room"]},
        ],
    },
}


def load(path):
    structs = collections.OrderedDict()
    for line in open(path, encoding="utf-8"):
        f = line.rstrip("\n").split("\t")
        if f[0] == "S":
            structs[f[1]] = {"expr": f[2], "size": int(f[3]), "entries": [],
                             "parent": f[5] if len(f) > 5 and f[4] == "M" else None}
        elif f[0] == "E":
            members = {}
            if len(f) > 10 and f[10] != "-":
                for m in f[10].split(","):
                    n, s = m.split(":")
                    members[n] = int(s)
            structs[f[1]]["entries"].append({
                "kind": f[2], "path": f[3], "elem": int(f[4]), "stored": None if f[5] == "-" else f[5],
                "dims": [] if f[6] == "-" else [int(d) for d in f[6].split(",")],
                "sub": None if f[7] == "-" else f[7], "notes": [] if f[8] == "-" else f[8].split(","),
                "at": f[9] if len(f) > 9 and f[9] != "-" else None, "members": members,
                "after": f[11] if len(f) > 11 and f[11] != "-" else None})
    return structs


def dim_expr(expr, path, n):
    inner = ["", "[0]", "[0][0]", "[0][0][0]"]
    return "(uint32_t)(sizeof(((%s *)0)->%s%s) / sizeof(((%s *)0)->%s%s))" % (
        expr, path, inner[n - 1], expr, path, inner[n])


def dims_arg(expr, e):
    return ".dims = { %s }" % ", ".join(dim_expr(expr, e["path"], i + 1) for i in range(len(e["dims"])))


def union_name(key, e):
    return "%s_%s" % (key, e["path"].replace("union:", "u_"))


def union_total_expr(expr, at, after):
    # The union itself is anonymous, so its size can't be named directly: measure the gap
    # to the next named sibling field, or to the end of the struct if it has none. This is
    # evaluated by whatever compiler builds the table, so it never bakes in one compiler's
    # widths (e.g. `long` is 4 bytes on i686, 8 on x86_64).
    if after:
        return "(uint32_t)(offsetof(%s, %s) - offsetof(%s, %s))" % (expr, after, expr, at)
    return "(uint32_t)(sizeof(%s) - offsetof(%s, %s))" % (expr, expr, at)


def member_size_expr(expr, name):
    return "(uint32_t)sizeof(((%s *)0)->%s)" % (expr, name)


def fmt(key, expr, e):
    p = e["path"]
    note = ("  /* %s */" % ", ".join(n for n in e["notes"] if n != "-")) if e["notes"] else ""
    if e["kind"] == "RUNTIME":
        return "    SAVE_RUNTIME(%s, %s),%s" % (expr, p, note)
    if e["kind"] == "OPAQUE":
        total = union_total_expr(expr, e["at"], e["after"])
        if (key, p) in UNIONS:
            return '    SAVE_UNION(%s, %s, "%s", %s, %s),%s' % (expr, e["at"], p, total, union_name(key, e), note)
        return '    SAVE_OPAQUE(%s, %s, "%s", %s),%s' % (expr, e["at"], p, total, note)
    nd = len(e["dims"])
    if e["kind"] == "SUB":
        if nd == 0:
            return "    SAVE_SUB(%s, %s, %s),%s" % (expr, p, e["sub"], note)
        if nd <= 2:
            return "    SAVE_SUB_ARRAY%s(%s, %s, %s),%s" % ("2" if nd == 2 else "", expr, p, e["sub"], note)
        return "    SAVE_SUB_EX(%s, %s, %s, %s),%s" % (expr, p, e["sub"], dims_arg(expr, e), note)
    if nd == 0:
        return "    SAVE_FIELD(%s, %s, %s),%s" % (expr, p, e["stored"], note)
    if nd == 1 and e["stored"] == "SV_STR":
        return "    SAVE_STRING(%s, %s),%s" % (expr, p, note)
    if nd <= 2:
        return "    SAVE_ARRAY%s(%s, %s, %s),%s" % ("2" if nd == 2 else "", expr, p, e["stored"], note)
    return "    SAVE_FIELD_EX(%s, %s, %s, %s),%s" % (expr, p, e["stored"], dims_arg(expr, e), note)


def union_def(structs, key, e):
    cfg = UNIONS[(key, e["path"])]
    expr = structs[key]["expr"]
    total = union_total_expr(expr, e["at"], e["after"])
    if "selector" in cfg:
        lines = ["SAVE_UNION_SELECTED_DEF(%s, \"%s\"," % (union_name(key, e), cfg["selector"])]
    else:
        lines = ["SAVE_UNION_DEF(%s, \"%s\"," % (union_name(key, e), cfg["disc"])]
    typed = []
    for g in cfg["groups"]:
        vals = ".values = { %s }, .nvalues = %d" % (", ".join(g["values"]), len(g["values"]))
        if not g["members"]:
            lines.append('    SAVE_UNION_MEMBER_NONE("%s", %s),' % (g["label"], vals))
        elif "shared" in g:
            mkey = "%s_%s" % (key, g["shared"])
            typed.append(mkey)
            lines.append('    SAVE_UNION_MEMBER_SHARED("%s", %s, "%s", %s),' % (g["label"], mkey, ",".join(g["members"]), vals))
        elif len(g["members"]) == 1:
            mkey = "%s_%s" % (key, g["members"][0])
            typed.append(mkey)
            lines.append('    SAVE_UNION_MEMBER_SUB("%s", %s, %s),' % (g["label"], mkey, vals))
        else:
            # The one-off dump picks which named member is largest; none of these members has a
            # variable-width field (checked by hand, 2026-09-27), so this ordering can't flip on
            # another compiler. If it ever did, the raw-member decode still refuses rather than
            # truncating live data (it requires any bytes past the smaller size to be zero).
            largest = max(g["members"], key=lambda m: e["members"][m])
            size_expr = member_size_expr(expr, largest)
            lines.append('    SAVE_UNION_MEMBER_RAW("%s", %s, "%s", %s),' % (g["label"], size_expr, ",".join(g["members"]), vals))
    lines.append('    SAVE_UNION_MEMBER_RAW("any", %s, "*", .is_default = 1)' % total)
    lines.append(");")
    return lines, typed


def classify(structs):
    game = structs["Game"]
    area_of_member = {}
    for area, names in AREAS.items():
        for n in names:
            area_of_member[n] = area
    reach = collections.defaultdict(set)
    children = collections.defaultdict(list)
    for k, s in structs.items():
        if s["parent"]:
            children[s["parent"]].append(k)

    def visit(key, area):
        if area in reach[key]:
            return
        reach[key].add(area)
        for e in structs[key]["entries"]:
            if e["kind"] == "SUB":
                visit(e["sub"], area)
        for c in children[key]:
            visit(c, area)

    for e in game["entries"]:
        if e["kind"] == "SUB":
            top = e["path"].split(".")[0]
            visit(e["sub"], area_of_member.get(top, "game"))
    for root in ROOT_ORDER[1:]:
        if root in structs:
            visit(root, "misc")
    result = {}
    for key in structs:
        areas = reach.get(key, {"misc"})
        result[key] = "game" if key == "Game" else (next(iter(areas)) if len(areas) == 1 else "misc")
        if key in ROOT_ORDER[1:]:
            result[key] = "misc"
    for key, s in structs.items():
        if s["parent"]:
            result[key] = result[s["parent"]]
    return result


def includes_from_roots(path):
    inc = []
    for line in open(path, encoding="utf-8"):
        m = re.match(r"include\s+(\S+)", line)
        if m:
            inc.append(m.group(1))
    return inc


def gen(types, roots, outdir):
    structs = load(types)
    areas = classify(structs)
    incs = includes_from_roots(roots)
    os.makedirs(outdir, exist_ok=True)
    # member tables only exist for the members a union group gives its own table
    typed_members = set()
    defs = collections.defaultdict(list)
    for key, s in structs.items():
        for e in s["entries"]:
            if e["kind"] == "OPAQUE" and (key, e["path"]) in UNIONS:
                lines, typed = union_def(structs, key, e)
                defs[key].append((union_name(key, e), lines))
                typed_members.update(typed)
    tables = {k for k, s in structs.items() if not s["parent"] or k in typed_members}
    head = ["/** @file %s", " *     Field tables for saved structs.", " */",
            '#include "pre_inc.h"'] + ['#include "%s"' % i for i in incs] + ['#include "post_inc.h"',
            '#include "kfx/save/core/save_schema.h"', '#include "kfx/save/core/schema/save_tables_decl.h"', ""]
    for area, fname in AREA_FILE.items():
        keys = sorted(k for k in tables if areas[k] == area)
        if not keys:
            continue
        lines = [head[0] % fname] + head[1:]
        for k in keys:
            s = structs[k]
            for _, dl in defs.get(k, []):
                lines += dl + [""]
            lines.append("SAVE_STRUCT(%s, %s," % (k, s["expr"]))
            lines += [fmt(k, s["expr"], e) for e in s["entries"]]
            lines.append(");")
            lines.append("")
        open(os.path.join(outdir, fname), "w", encoding="utf-8", newline="\n").write("\n".join(lines))
    decl = ["/** @file save_tables_decl.h", " *     Declarations of every struct table.", " */",
            "#ifndef KFX_SAVE_TABLES_DECL_H", "#define KFX_SAVE_TABLES_DECL_H", "",
            '#include "kfx/save/core/save_schema.h"', "", "#ifdef __cplusplus", 'extern "C" {', "#endif", ""] + ["SAVE_DECLARE(%s);" % k for k in sorted(tables)]
    decl += ["SAVE_DECLARE_UNION(%s);" % n for k in sorted(defs) for n, _ in defs[k]] + ["", "#ifdef __cplusplus", "}", "#endif", "", "#endif", ""]
    open(os.path.join(outdir, "save_tables_decl.h"), "w", encoding="utf-8", newline="\n").write("\n".join(decl))
    roots_c = ["/** @file schema_all.c", " *     Root struct tables of everything the game saves.", " */",
               '#include "kfx/save/core/save_tables.h"', '#include "kfx/save/core/schema/save_tables_decl.h"', "",
               "static const struct SaveStructDesc *const save_roots[] = {"]
    roots_c += ["    &save_desc_%s," % r for r in ROOT_ORDER if r in structs] + ["};", "",
        "const struct SaveStructDesc *const *save_root_structs(uint32_t *count)", "{",
        "    *count = (uint32_t)(sizeof(save_roots) / sizeof(save_roots[0]));", "    return save_roots;", "}", ""]
    open(os.path.join(outdir, "schema_all.c"), "w", encoding="utf-8", newline="\n").write("\n".join(roots_c))
    print("wrote %d struct tables, %d entries" % (len(tables), sum(len(structs[k]["entries"]) for k in tables)))


ENTRY = re.compile(r"SAVE_(FIELD_EX|FIELD|SUB_ARRAY2|SUB_ARRAY_EX|SUB_ARRAY|SUB_EX|SUB|ARRAY2_EX|ARRAY2|ARRAY_EX|ARRAY|STRING|RUNTIME|DERIVED)\(\s*([^,()]+?)\s*,\s*([A-Za-z_][\w.]*)\s*(?:,\s*(\w+))?")
# The byte-count params below are C expressions (sizeof/offsetof), not literals -- they're
# evaluated by the compiler that builds the table, so this tool never re-checks their value;
# it only checks which struct members and union arms are named, which stays exact regardless
# of the compiler's pointer/long width.
OPAQUE_ENTRY = re.compile(r'SAVE_OPAQUE\(\s*[^,()]+?\s*,\s*[\w.]+\s*,\s*"([^"]+)"\s*,')
UNION_DEF = re.compile(r'SAVE_UNION_(?:SELECTED_)?DEF\(\s*(\w+)\s*,\s*"(\w+)"')
UNION_SHARED = re.compile(r'SAVE_UNION_MEMBER_SHARED\(\s*"([^"]+)"\s*,\s*\w+\s*,\s*"([^"]*)"')
UNION_RAW = re.compile(r'SAVE_UNION_MEMBER_RAW\(\s*"([^"]+)"\s*,.*?,\s*"([^"]*)"')
UNION_SUB = re.compile(r'SAVE_UNION_MEMBER_SUB\(\s*"([^"]+)"\s*,\s*(\w+)')
BLOCK = re.compile(r"SAVE_STRUCT\(\s*(\w+)\s*,\s*([^,]+?)\s*,")
SIZES = {"SV_U8": 1, "SV_I8": 1, "SV_BOOL8": 1, "SV_STR": 1, "SV_U16": 2, "SV_I16": 2, "SV_U32": 4, "SV_I32": 4,
         "SV_F32": 4, "SV_U64": 8, "SV_I64": 8, "SV_F64": 8}


def call_args(text, open_pos):
    # Splits a macro call's arguments on top-level commas, respecting nested parens and quoted
    # strings, so a comma inside an embedded sizeof()/offsetof() expression doesn't end an arg
    # early. text[open_pos] must be the call's opening '('. Returns (args, pos after the ')').
    depth = 0
    in_str = False
    args = []
    cur = []
    i = open_pos
    while True:
        c = text[i]
        if in_str:
            cur.append(c)
            if c == '"' and text[i - 1] != "\\":
                in_str = False
        elif c == '"':
            in_str = True
            cur.append(c)
        elif c == "(":
            depth += 1
            if depth > 1:
                cur.append(c)
        elif c == ")":
            depth -= 1
            if depth == 0:
                args.append("".join(cur).strip())
                return args, i + 1
            cur.append(c)
        elif (c == ",") and (depth == 1):
            args.append("".join(cur).strip())
            cur = []
        else:
            cur.append(c)
        i += 1


def find_union_entries(text, start, end):
    # SAVE_UNION(...)'s bytes argument is an offsetof()-based expression with its own commas,
    # so unlike the other ENTRY regexes this one needs a real, paren-balanced argument split.
    out = {}
    for m in re.finditer(r"SAVE_UNION\(", text[start:end]):
        open_pos = start + m.end() - 1
        args, _ = call_args(text, open_pos)
        label = args[2].strip('"')
        out[label] = {"macro": "UNION", "union": args[4], "third": None}
    return out


def parse_tables(tdir):
    tables = {}
    unions = {}
    for path in sorted(glob.glob(os.path.join(tdir, "*.c"))):
        text = open(path, encoding="utf-8").read()
        text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
        for m in UNION_DEF.finditer(text):
            end = text.index(");", m.end())
            body = text[m.end():end]
            unions[m.group(1)] = {"disc": m.group(2), "raw": [(r.group(1), r.group(2))
                                                              for r in UNION_RAW.finditer(body)] +
                                  [(r.group(1), r.group(2)) for r in UNION_SHARED.finditer(body)],
                                  "subs": [(r.group(1), r.group(2)) for r in UNION_SUB.finditer(body)]}
        blocks = list(BLOCK.finditer(text))
        for i, b in enumerate(blocks):
            end = blocks[i + 1].start() if i + 1 < len(blocks) else len(text)
            entries = collections.OrderedDict()
            for m in ENTRY.finditer(text, b.end(), end):
                entries[m.group(3)] = {"macro": m.group(1), "third": m.group(4)}
            for m in OPAQUE_ENTRY.finditer(text, b.end(), end):
                entries[m.group(1)] = {"macro": "OPAQUE", "third": None}
            entries.update(find_union_entries(text, b.end(), end))
            tables[b.group(1)] = {"expr": b.group(2), "entries": entries, "file": os.path.basename(path)}
    return tables, unions


def check_union(key, e, m, unions, tables, errors):
    # Byte sizes in the generated table are sizeof()/offsetof() expressions the compiler
    # evaluates, so there's nothing for this tool to compare them against; it only checks that
    # every real union member is named by some group, and that no group names a member that
    # doesn't exist. A wrong or too-small raw size would be a codec bug (out of this tool's
    # reach), not a stale-generation bug -- the compiler always sees the true current size.
    u = unions.get(m["union"])
    if u is None:
        errors.append("%s.%s: no SAVE_UNION_DEF named %s" % (key, e["path"], m["union"]))
        return
    covered = set()
    for label, covers in u["raw"]:
        if covers == "*":
            continue
        names = covers.split(",")
        missing = [n for n in names if n not in e["members"]]
        if missing:
            errors.append("%s.%s: %s covers %s, which isn't a member" % (key, e["path"], label, ",".join(missing)))
            continue
        covered.update(names)
    for label, sub in u["subs"]:
        prefix = key + "_"
        covered.add(sub[len(prefix):] if sub.startswith(prefix) else label)
    for name in e["members"]:
        if name not in covered:
            errors.append("%s.%s: union member %s isn't covered by any group" % (key, e["path"], name))


def check(types, tdir):
    structs = load(types)
    tables, unions = parse_tables(tdir)
    errors = []
    warnings = []
    for key, s in structs.items():
        t = tables.get(key)
        if t is None:
            if s["parent"]:
                continue
            errors.append("%s: no table" % key)
            for e in s["entries"]:
                errors.append("  add " + fmt(key, s["expr"], e).strip())
            continue
        listed = t["entries"]
        want = collections.OrderedDict((e["path"], e) for e in s["entries"])
        for path, e in want.items():
            m = listed.get(path)
            if m is None:
                errors.append("%s.%s: not in the table; add:" % (key, path))
                errors.append("  " + fmt(key, s["expr"], e).strip())
                continue
            skip = m["macro"] in ("RUNTIME", "DERIVED")
            if e["kind"] == "RUNTIME" and not skip:
                errors.append("%s.%s: %s can't be stored; use SAVE_RUNTIME or SAVE_DERIVED" % (key, path, ",".join(e["notes"])))
            elif e["kind"] == "SUB" and not skip and not m["macro"].startswith("SUB"):
                errors.append("%s.%s: is a struct" % (key, path))
            elif e["kind"] in ("FIELD", "OPAQUE") and not skip:
                if m["macro"].startswith("SUB"):
                    errors.append("%s.%s: isn't a struct" % (key, path))
                elif m["third"] in SIZES and SIZES[m["third"]] < e["elem"] and m["third"] != "SV_STR":
                    errors.append("%s.%s: %s can't hold a %d byte member" % (key, path, m["third"], e["elem"]))
            if e["kind"] == "OPAQUE" and m["macro"] == "UNION":
                check_union(key, e, m, unions, tables, errors)
            elif e["kind"] == "OPAQUE" and not skip:
                warnings.append("%s.%s: raw bytes (%s); needs a real union table" % (key, path, ",".join(e["notes"])))
        for path in listed:
            if path not in want:
                errors.append("%s.%s: listed but not a member" % (key, path))
    for key in tables:
        if key not in structs:
            errors.append("%s: table for a struct that isn't saved" % key)
    return errors, warnings


def main(argv):
    if len(argv) >= 5 and argv[1] == "gen":
        gen(argv[2], argv[3], argv[4])
        return 0
    if len(argv) >= 4 and argv[1] == "check":
        errors, warnings = check(argv[2], argv[3])
        for w in warnings:
            print("warning: " + w)
        for e in errors:
            print("error: " + e if not e.startswith("  ") else e)
        n = sum(1 for e in errors if not e.startswith("  "))
        print("%d errors, %d warnings" % (n, len(warnings)))
        return 1 if n else 0
    print(__doc__)
    return 64


if __name__ == "__main__":
    sys.exit(main(sys.argv))
