# gdb script: write the field-table entries every saved struct needs, one row per entry.
# Run through tools/save-layout/extract.sh with KFX_WALK=this file. Uses ROOTS and OUTFILE.
# S <key> <c type> <size> [M <parent key>]   (M: a struct member of a union, offsets are from the parent)
# E <key> <kind> <path> <elem size> <stored> <dims> <sub key> <notes> <at> <members> <after>
# OPAQUE rows: path is the label, at is the first member of the union (its offset), dims is the byte size,
# members lists the struct members as name:size, and after is the next named sibling field of the union
# in its containing struct (empty if the union is the last member) -- used to compute the union's real
# size as offsetof(after) - offsetof(at), or sizeof(struct) - offsetof(at) when there is no after, so the
# generated table never bakes in a byte count measured on one compiler (a long changes width)
# kind: FIELD (scalar or array), SUB (named struct), RUNTIME (never stored), OPAQUE (raw bytes)
import gdb

out = open(OUTFILE, "w")
queue = []
seen = set()
expr_of = {}


def typedef_names(t):
    names = []
    while t.code == gdb.TYPE_CODE_TYPEDEF:
        names.append(t.name)
        t = t.target()
    return names


def type_key(t):
    # (key, C type expression) for a named struct or union type, else None.
    names = typedef_names(t)
    s = t.strip_typedefs()
    if s.code == gdb.TYPE_CODE_UNION:
        tag = ("union " + s.tag) if s.tag else None
    else:
        tag = ("struct " + s.tag) if s.tag else None
    if tag:
        return s.tag, tag
    if names:
        return names[0], names[0]
    return None


def stored_for(t):
    names = typedef_names(t)
    s = t.strip_typedefs()
    code = s.code
    size = s.sizeof
    notes = []
    if code == gdb.TYPE_CODE_BOOL:
        return "SV_BOOL8", notes
    if code == gdb.TYPE_CODE_ENUM:
        return "SV_I32", ["enum%d" % size]
    if code in (gdb.TYPE_CODE_INT, gdb.TYPE_CODE_CHAR):
        if size == 1 and "TbBool" in names:
            return "SV_BOOL8", notes
        if s.name == "char":
            notes.append("plain-char")
        if s.name in ("long", "unsigned long", "long int", "long unsigned int"):
            notes.append("long")
        signed = s.is_signed
        return "SV_%s%d" % ("I" if signed else "U", size * 8), notes
    if code == gdb.TYPE_CODE_FLT:
        if size == 4:
            return "SV_F32", notes
        if size == 8:
            return "SV_F64", notes
        return None, ["long-double"]
    if code == gdb.TYPE_CODE_PTR:
        return None, ["pointer"]
    return None, ["code%d" % code]


def emit(key, kind, path, elem, stored, dims, sub, notes, at=None, extra=None, after=None):
    out.write("E\t%s\t%s\t%s\t%d\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n" % (
        key, kind, path, elem, stored or "-", ",".join(str(d) for d in dims) or "-", sub or "-", ",".join(notes) or "-",
        at or "-", extra or "-", after or "-"))


def split_array(t):
    dims = []
    s = t.strip_typedefs()
    while s.code == gdb.TYPE_CODE_ARRAY:
        lo, hi = s.range()
        dims.append(hi - lo + 1)
        t = s.target()
        s = t.strip_typedefs()
    return t, dims


def enqueue(t):
    k = type_key(t)
    if k and k[0] not in seen:
        seen.add(k[0])
        queue.append((k, t.strip_typedefs()))


def first_named(s, prefix):
    for f in s.fields():
        if f.name:
            return prefix + f.name
        inner = first_named(f.type.strip_typedefs(), prefix)
        if inner:
            return inner
    return None


def union_entry(key, path, prefix, s, after=None):
    # A member that spans the whole union carries every byte; otherwise store raw bytes.
    base = path if path else prefix.rstrip(".")
    for f in s.fields():
        ft = f.type.strip_typedefs()
        if f.name and f.bitsize == 0 and ft.sizeof == s.sizeof and ft.code in (
                gdb.TYPE_CODE_INT, gdb.TYPE_CODE_CHAR, gdb.TYPE_CODE_FLT, gdb.TYPE_CODE_ENUM, gdb.TYPE_CODE_BOOL):
            stored, notes = stored_for(f.type)
            if stored:
                emit(key, "FIELD", (base + "." + f.name) if base else f.name, ft.sizeof, stored, [], None,
                     notes + ["union-view"])
                return
    at = first_named(s, (base + ".") if base else "")
    msizes = []
    for f in s.fields():
        ft = f.type.strip_typedefs()
        if f.name and ft.code == gdb.TYPE_CODE_STRUCT:
            mkey = key + "_" + f.name
            expr_of[mkey] = expr_of[key]
            out.write("S\t%s\t%s\t%d\tM\t%s\n" % (mkey, expr_of[key], ft.sizeof, key))
            members(mkey, ((base + ".") if base else "") + f.name + ".", ft)
            msizes.append("%s:%d" % (f.name, ft.sizeof))
    emit(key, "OPAQUE", path if path else "union:" + at, 1, "SV_U8", [s.sizeof], None, ["union"], at, ",".join(msizes), after)


def members(key, prefix, s):
    flds = list(s.fields())
    for i, f in enumerate(flds):
        if f.bitsize:
            emit(key, "OPAQUE", prefix + (f.name or "?"), 1, "SV_U8", [s.sizeof], None, ["BITFIELD"], prefix + (f.name or "?"))
            continue
        path = prefix + f.name if f.name else prefix
        base, dims = split_array(f.type)
        bs = base.strip_typedefs()
        if bs.code in (gdb.TYPE_CODE_STRUCT, gdb.TYPE_CODE_UNION):
            named = type_key(base)
            if bs.code == gdb.TYPE_CODE_UNION:
                if dims:
                    emit(key, "OPAQUE", path, 1, "SV_U8", [f.type.strip_typedefs().sizeof], None, ["union-array"], path)
                else:
                    after = None
                    for nxt in flds[i + 1:]:
                        if nxt.name:
                            after = nxt.name
                            break
                    union_entry(key, path if f.name else None, prefix, bs, after)
            elif named:
                enqueue(base)
                emit(key, "SUB", path, bs.sizeof, "SV_STRUCT", dims, named[0], [])
            elif dims:
                emit(key, "OPAQUE", path, 1, "SV_U8", [f.type.strip_typedefs().sizeof], None, ["anon-array"], path)
            else:
                members(key, path + "." if f.name else prefix, bs)
            continue
        stored, notes = stored_for(base)
        if stored is None:
            emit(key, "RUNTIME", path, bs.sizeof, None, dims, None, notes)
            continue
        if stored.startswith("SV_") and "plain-char" in notes and dims:
            stored = "SV_STR"
        emit(key, "FIELD", path, bs.sizeof, stored, dims, None, notes)


for r in ROOTS.split():
    if r.startswith("typedef:"):
        continue
    enqueue(gdb.lookup_type("struct " + r))

while queue:
    (key, expr), s = queue.pop(0)
    expr_of[key] = expr
    out.write("S\t%s\t%s\t%d\n" % (key, expr, s.sizeof))
    members(key, "", s)
out.close()
