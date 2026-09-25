# gdb script: write the layout of every saved type as TSV.
# Run as: gdb -batch -nx -ex "python ROOTS='...'; OUTFILE='...'" -ex "source walk.py" probe.elf
# Each row: path <TAB> offset <TAB> size <TAB> kind <TAB> declared type
import gdb

KINDS = {gdb.TYPE_CODE_PTR: "ptr", gdb.TYPE_CODE_ENUM: "enum", gdb.TYPE_CODE_FLT: "float",
         gdb.TYPE_CODE_BOOL: "bool", gdb.TYPE_CODE_INT: "int", gdb.TYPE_CODE_CHAR: "char"}
out = open(OUTFILE, "w")


def row(path, off, size, kind, tname):
    out.write("%s\t%s\t%s\t%s\t%s\n" % (path, off, size, kind, tname))


def walk(t, path, base):
    t0 = t
    t = t.strip_typedefs()
    code = t.code
    if code in (gdb.TYPE_CODE_STRUCT, gdb.TYPE_CODE_UNION):
        row(path, base, t.sizeof, "struct" if code == gdb.TYPE_CODE_STRUCT else "union", str(t0))
        anon = 0
        for f in t.fields():
            nm = f.name
            if not nm:
                nm = "<anon%d>" % anon
                anon += 1
            name = "%s.%s" % (path, nm)
            if f.bitsize:
                row(name, "%d.%d" % (base + f.bitpos // 8, f.bitpos % 8), "%db" % f.bitsize, "bitfield", str(f.type))
                continue
            walk(f.type, name, base + f.bitpos // 8)
    elif code == gdb.TYPE_CODE_ARRAY:
        el = t.target()
        n = t.sizeof // max(el.sizeof, 1)
        if el.strip_typedefs().code in (gdb.TYPE_CODE_STRUCT, gdb.TYPE_CODE_UNION, gdb.TYPE_CODE_ARRAY):
            row(path, base, t.sizeof, "array[%d]" % n, str(t0))
            walk(el, path + "[]", base)
        else:
            row(path, base, t.sizeof, "leafarray[%d]" % n, str(t0))
    else:
        row(path, base, t.sizeof, KINDS.get(code, "code%d" % code), str(t0))


for r in ROOTS.split():
    if r.startswith("typedef:"):
        name, lookup = r[len("typedef:"):], r[len("typedef:"):]
    else:
        name, lookup = r, "struct " + r
    try:
        t = gdb.lookup_type(lookup)
    except gdb.error:
        row(name, 0, -1, "missing", lookup)
        continue
    walk(t, name, 0)
out.close()
