/******************************************************************************/
/** @file kfx_save_tests.c
 *     Standalone tests for the save codec.
 *     Links only src/kfx/save/*, the schema tables and zlib; no game logic, no game data.
 *
 *     kfx_save_tests                     run every test
 *     kfx_save_tests --write-sample F    write the deterministic sample save F
 *     kfx_save_tests --digest F          print what a sample file holds, as this build reads it
 *
 *     The sample is built from every saved field of the real struct Game, filled with values
 *     derived from the field's path, so each platform produces the same logical state. Comparing
 *     the outputs of --digest across platforms checks that the format doesn't depend on the
 *     compiler, word size or char signedness.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "kfx/save/SaveTypes.h"
#include "post_inc.h"
#include "kfx/save/SaveUnionArms.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include "kfx/save/core/save_codec.h"
#include "kfx/save/core/save_config_overlay.h"
#include "kfx/save/core/save_inspect.h"
#include "kfx/save/core/save_migrate.h"
#include "kfx/save/core/save_recfile.h"
#include "kfx/save/core/save_names.h"
#include "kfx/save/core/save_schema.h"
#include "kfx/save/core/save_tables.h"
#include "kfx/save/core/schema/save_tables_decl.h"
#include "save_digest.h"
#include "save_validate.h"
/******************************************************************************/
static int g_checks;
static int g_fails;

#define CHECK(cond, ...) \
    do { \
        g_checks++; \
        if (!(cond)) { \
            g_fails++; \
            printf("FAIL %s:%d: ", __FILE__, __LINE__); \
            printf(__VA_ARGS__); \
            printf("\n"); \
        } \
    } while (0)

static void section(const char *name)
{
    printf("-- %s\n", name);
}
/******************************************************************************/
/* Tables for synthetic structs: same struct name in the file, different shape in the build. */
#define TDESC(var, str, T, ...) \
    static const struct SaveFieldDesc var##_f[] = { __VA_ARGS__ }; \
    static const struct SaveStructDesc var = { str, (uint32_t)sizeof(T), \
        (uint32_t)(sizeof(var##_f) / sizeof(var##_f[0])), var##_f }

struct Xcode {
    struct SaveBuffer schema;
    struct SaveFileSchema fs;
    struct SaveDecoder dec;
    int have_fs;
};

static enum SaveResult xcode_open(struct Xcode *x, const struct SaveStructDesc *const *roots, uint32_t n,
    enum SaveMode enc_mode, enum SaveMode dec_mode, struct SaveError *err)
{
    memset(x, 0, sizeof(*x));
    enum SaveResult r = save_schema_encode(&x->schema, roots, n, enc_mode, err);
    if (r != SVR_Ok)
        return r;
    r = save_schema_decode(x->schema.data, x->schema.len, &x->fs, err);
    if (r != SVR_Ok)
        return r;
    x->have_fs = 1;
    save_decoder_init(&x->dec, &x->fs, dec_mode);
    return SVR_Ok;
}

static void xcode_close(struct Xcode *x)
{
    save_decoder_free(&x->dec);
    if (x->have_fs)
        save_schema_free(&x->fs);
    save_buf_free(&x->schema);
    memset(x, 0, sizeof(*x));
}

/** Encodes ov with od's schema, then decodes it into nv (zeroed by the caller) using nd. */
static enum SaveResult convert1(const struct SaveStructDesc *od, const void *ov, const struct SaveStructDesc *nd,
    void *nv, struct SaveError *err)
{
    struct SaveBuffer rec = { NULL, 0, 0 };
    struct Xcode x;
    const struct SaveStructDesc *roots[1] = { od };
    uint32_t used = 0;
    enum SaveResult r = save_encode_record(&rec, od, ov, SVM_Save, err);
    memset(&x, 0, sizeof(x));
    if (r == SVR_Ok)
        r = xcode_open(&x, roots, 1, SVM_Save, SVM_Save, err);
    if (r == SVR_Ok)
        r = save_decode_record(&x.dec, nd, rec.data, rec.len, &used, nv, err);
    if (r == SVR_Ok)
        CHECK(used == rec.len, "decoder used %u of %u bytes", (unsigned)used, (unsigned)rec.len);
    xcode_close(&x);
    save_buf_free(&rec);
    return r;
}
/******************************************************************************/
/* Scalar conversions: widen, sign change, narrow, clamp, floats. */
struct CvOld { uint8_t a; int16_t b; uint32_t c; int32_t d; int32_t e; uint8_t flag; };
struct CvNew { uint32_t a; int32_t b; int32_t c; int16_t d; int16_t e; uint8_t flag; };

TDESC(cv_old, "Cv", struct CvOld,
    SAVE_FIELD(struct CvOld, a, SV_U8), SAVE_FIELD(struct CvOld, b, SV_I16), SAVE_FIELD(struct CvOld, c, SV_U32),
    SAVE_FIELD(struct CvOld, d, SV_I32), SAVE_FIELD(struct CvOld, e, SV_I32), SAVE_FIELD(struct CvOld, flag, SV_BOOL8));
TDESC(cv_new, "Cv", struct CvNew,
    SAVE_FIELD(struct CvNew, a, SV_U32), SAVE_FIELD(struct CvNew, b, SV_I32), SAVE_FIELD(struct CvNew, c, SV_I32),
    SAVE_FIELD(struct CvNew, d, SV_I16), SAVE_FIELD(struct CvNew, e, SV_I16), SAVE_FIELD(struct CvNew, flag, SV_BOOL8));
TDESC(cv_clamp, "Cv", struct CvNew,
    SAVE_FIELD(struct CvNew, a, SV_U32), SAVE_FIELD(struct CvNew, b, SV_I32),
    SAVE_FIELD_EX(struct CvNew, c, SV_I32, .flags = SVF_CLAMP), SAVE_FIELD_EX(struct CvNew, d, SV_I16, .flags = SVF_CLAMP),
    SAVE_FIELD_EX(struct CvNew, e, SV_I16, .flags = SVF_CLAMP), SAVE_FIELD(struct CvNew, flag, SV_BOOL8));

struct FOld { float x; };
struct FNew { double x; };
struct FInt { uint32_t x; };
TDESC(f_old, "F", struct FOld, SAVE_FIELD(struct FOld, x, SV_F32));
TDESC(f_new, "F", struct FNew, SAVE_FIELD(struct FNew, x, SV_F64));
TDESC(f_int, "F", struct FInt, SAVE_FIELD(struct FInt, x, SV_U32));

static void test_conversions(void)
{
    struct SaveError err;
    struct CvOld o = { 200, -5, 5, 100, 1, 1 };
    struct CvNew n;

    section("conversions");
    memset(&n, 0, sizeof(n));
    CHECK(convert1(&cv_old, &o, &cv_new, &n, &err) == SVR_Ok, "in-range conversion refused: %s", err.message);
    CHECK((n.a == 200) && (n.b == -5) && (n.c == 5) && (n.d == 100) && (n.e == 1) && (n.flag == 1),
        "widen / sign change / narrow in range changed a value");

    o.c = 3000000000u;
    memset(&n, 0, sizeof(n));
    CHECK(convert1(&cv_old, &o, &cv_new, &n, &err) == SVR_Unsupported, "u32 above INT32_MAX was accepted as i32");
    memset(&n, 0, sizeof(n));
    CHECK(convert1(&cv_old, &o, &cv_clamp, &n, &err) == SVR_Ok, "clamped conversion refused");
    CHECK(n.c == INT32_MAX, "clamp gave %d, wanted INT32_MAX", (int)n.c);

    o.c = 5;
    o.d = 70000;
    memset(&n, 0, sizeof(n));
    CHECK(convert1(&cv_old, &o, &cv_new, &n, &err) == SVR_Unsupported, "70000 was accepted as i16");
    memset(&n, 0, sizeof(n));
    CHECK(convert1(&cv_old, &o, &cv_clamp, &n, &err) == SVR_Ok, "clamped narrowing refused");
    CHECK(n.d == 32767, "clamp gave %d, wanted 32767", (int)n.d);
    o.d = -70000;
    memset(&n, 0, sizeof(n));
    CHECK((convert1(&cv_old, &o, &cv_clamp, &n, &err) == SVR_Ok) && (n.d == -32768), "negative clamp wrong");

    o.d = 1;
    o.flag = 7; /* any non-zero bool reads back as 1 */
    memset(&n, 0, sizeof(n));
    CHECK((convert1(&cv_old, &o, &cv_new, &n, &err) == SVR_Ok) && (n.flag == 1), "bool8 didn't normalise to 1");

    {
        struct FOld fo = { 0.1f };
        struct FNew fn = { 0 };
        struct FInt fi = { 0 };
        CHECK((convert1(&f_old, &fo, &f_new, &fn, &err) == SVR_Ok) && (fn.x == (double)0.1f), "f32 -> f64 changed the value");
        CHECK(convert1(&f_old, &fo, &f_int, &fi, &err) == SVR_Unsupported, "float -> integer was accepted");
    }
}
/******************************************************************************/
/* Names: alias, default for a new field, unknown field stashed; order doesn't matter. */
struct NmOld { uint16_t hp; uint16_t removed; uint16_t keep; };
struct NmNew { uint16_t keep; uint16_t added; uint16_t health; };
static const char *const hp_aliases[] = { "hp", NULL };

TDESC(nm_old, "Nm", struct NmOld,
    SAVE_FIELD(struct NmOld, hp, SV_U16), SAVE_FIELD(struct NmOld, removed, SV_U16), SAVE_FIELD(struct NmOld, keep, SV_U16));
TDESC(nm_new, "Nm", struct NmNew,
    SAVE_FIELD(struct NmNew, keep, SV_U16), SAVE_FIELD_EX(struct NmNew, added, SV_U16, .default_value = 77),
    SAVE_FIELD_EX(struct NmNew, health, SV_U16, .aliases = hp_aliases));

static void test_names(void)
{
    struct SaveError err;
    struct NmOld o = { 11, 22, 33 };
    struct NmNew n;
    struct SaveBuffer rec = { NULL, 0, 0 };
    struct Xcode x;
    const struct SaveStructDesc *roots[1] = { &nm_old };
    uint32_t used;

    section("names, defaults, stash");
    memset(&n, 0, sizeof(n));
    CHECK(save_encode_record(&rec, &nm_old, &o, SVM_Save, &err) == SVR_Ok, "encode failed");
    CHECK(xcode_open(&x, roots, 1, SVM_Save, SVM_Save, &err) == SVR_Ok, "schema failed");
    CHECK(save_decode_record(&x.dec, &nm_new, rec.data, rec.len, &used, &n, &err) == SVR_Ok, "decode: %s", err.message);
    CHECK(n.health == 11, "alias hp -> health gave %u", (unsigned)n.health);
    CHECK(n.keep == 33, "matched field gave %u", (unsigned)n.keep);
    CHECK(n.added == 77, "new field got %u instead of its default", (unsigned)n.added);
    CHECK((x.dec.stash_count == 1) && (strcmp(x.dec.stash[0].path, "Nm.removed") == 0), "the dropped field wasn't stashed");
    xcode_close(&x);
    save_buf_free(&rec);
}
/******************************************************************************/
/* A load replaces the live struct with the decoded one; the fields the format leaves to the running process
   have to come across from the live one, at any depth, and nothing else. */
struct RtIn { uint8_t keep; uint8_t live; };
struct Rt { uint8_t a; uint8_t rt; struct RtIn in[2]; uint8_t derived; uint8_t wide[3]; };
SAVE_STRUCT(RtIn, struct RtIn, SAVE_FIELD(struct RtIn, keep, SV_U8), SAVE_RUNTIME(struct RtIn, live));
TDESC(rt_desc, "Rt", struct Rt,
    SAVE_FIELD(struct Rt, a, SV_U8),
    SAVE_RUNTIME(struct Rt, rt),
    SAVE_SUB_EX(struct Rt, in, RtIn, .dims = { 2 }),
    SAVE_DERIVED(struct Rt, derived, "rebuilt elsewhere"),
    SAVE_RUNTIME(struct Rt, wide));

static void test_copy_runtime(void)
{
    struct Rt dst = { 1, 2, { { 3, 4 }, { 5, 6 } }, 7, { 8, 8, 8 } };
    const struct Rt src = { 9, 10, { { 11, 12 }, { 13, 14 } }, 15, { 16, 17, 18 } };
    section("runtime fields across a load");
    save_copy_runtime(&rt_desc, &dst, &src);
    CHECK(dst.a == 1, "a saved field was overwritten");
    CHECK(dst.rt == 10, "a runtime field wasn't copied");
    CHECK((dst.in[0].keep == 3) && (dst.in[1].keep == 5), "a saved field in a struct array was overwritten");
    CHECK((dst.in[0].live == 12) && (dst.in[1].live == 14), "a runtime field in a struct array wasn't copied");
    CHECK(dst.derived == 7, "a derived field was overwritten (it is rebuilt, not carried)");
    CHECK((dst.wide[0] == 16) && (dst.wide[1] == 17) && (dst.wide[2] == 18), "a runtime array wasn't copied whole");
}
/******************************************************************************/
/* The name table of a save, written and checked the way the game does it: some config loaders never
   keep their table's count field, so a table can have names and a count of zero. */
static void name_case_fill(struct Game *g, int keep_count)
{
    const char *const names[3] = { "A0", "A1", "A2" };
    for (int i = 0; i < 3; i++)
    {
        strcpy(g->conf.crtr_conf.model[i].name, names[i]);
        strcpy(g->conf.crtr_conf.instances[i].name, names[i]);
        strcpy(g->conf.slab_conf.room_cfgstats[i].code_name, names[i]);
        strcpy(g->conf.trapdoor_conf.trap_cfgstats[i].code_name, names[i]);
        strcpy(g->conf.trapdoor_conf.door_cfgstats[i].code_name, names[i]);
        strcpy(g->conf.object_conf.object_cfgstats[i].code_name, names[i]);
        strcpy(g->conf.magic_conf.spell_cfgstats[i].code_name, names[i]);
        strcpy(g->conf.magic_conf.power_cfgstats[i].code_name, names[i]);
        strcpy(g->conf.magic_conf.shot_cfgstats[i].code_name, names[i]);
        strcpy(g->conf.effects_conf.effect_cfgstats[i].code_name, names[i]);
        strcpy(g->conf.effects_conf.effectelement_cfgstats[i].code_name, names[i]);
        strcpy(g->conf.effects_conf.effectgen_cfgstats[i].code_name, names[i]);
    }
    if (keep_count)
    {
        g->conf.crtr_conf.model_count = 3;
        g->conf.crtr_conf.instances_count = 3;
        g->conf.slab_conf.room_types_count = 3;
        g->conf.trapdoor_conf.trap_types_count = 3;
        g->conf.trapdoor_conf.door_types_count = 3;
        g->conf.object_conf.object_types_count = 3;
        g->conf.magic_conf.spell_types_count = 3;
        g->conf.magic_conf.power_types_count = 3;
        g->conf.magic_conf.shot_types_count = 3;
        g->conf.effects_conf.effectgen_cfgstats_count = 3;
    }
}

static void test_name_tables(void)
{
    static const uint8_t classes[] = { TCls_Object, TCls_Shot, TCls_EffectElem, TCls_Creature, TCls_Effect,
        TCls_EffectGen, TCls_Trap, TCls_Door };
    struct Game *g = (struct Game *)calloc(1, sizeof(struct Game));
    struct SaveError err;
    section("name table");
    CHECK(g != NULL, "out of memory");
    for (unsigned i = 0; (g != NULL) && (i < sizeof(classes)); i++)
    {
        for (int keep_count = 0; keep_count <= 1; keep_count++)
        {
            struct SaveBuffer nams = { NULL, 0, 0 };
            memset(g, 0, sizeof(*g));
            name_case_fill(g, keep_count);
            g->things_data[5].class_id = classes[i];
            g->things_data[5].model = 2;
            CHECK(save_names_write(g, &nams, &err) == SVR_Ok, "class %u: write failed", (unsigned)classes[i]);
            CHECK(save_names_check(nams.data, nams.len, g, &err) == SVR_Ok,
                "class %u, counts %s: a thing with model 2 was refused: %s", (unsigned)classes[i],
                keep_count ? "kept" : "never set", err.message);
            /* the same save in a build whose table has a different name at that position */
            strcpy(g->conf.crtr_conf.model[2].name, "OTHER");
            if (classes[i] == TCls_Creature)
                CHECK(save_names_check(nams.data, nams.len, g, &err) == SVR_Unsupported, "a renamed model was accepted");
            save_buf_free(&nams);
        }
    }
    free(g);
}
/******************************************************************************/
/* Arrays: grow, shrink (with and without live entries), 2D resize, strings, struct arrays. */
struct Ar4 { uint8_t v[4]; };
struct Ar2 { uint8_t v[2]; };
struct Ar8 { uint8_t v[8]; };
TDESC(ar4, "Ar", struct Ar4, SAVE_FIELD_EX(struct Ar4, v, SV_U8, .dims = { 4 }));
TDESC(ar2, "Ar", struct Ar2, SAVE_FIELD_EX(struct Ar2, v, SV_U8, .dims = { 2 }));
TDESC(ar8d, "Ar", struct Ar8, SAVE_FIELD_EX(struct Ar8, v, SV_U8, .dims = { 8 }, .default_value = 9));

struct G23 { uint8_t m[2][3]; };
struct G34 { uint8_t m[3][4]; };
TDESC(g23, "G", struct G23, SAVE_FIELD_EX(struct G23, m, SV_U8, .dims = { 2, 3 }));
TDESC(g34, "G", struct G34, SAVE_FIELD_EX(struct G34, m, SV_U8, .dims = { 3, 4 }));

struct S8 { char s[8]; };
struct S4 { char s[4]; };
TDESC(s8, "S", struct S8, SAVE_FIELD_EX(struct S8, s, SV_STR, .dims = { 8 }));
TDESC(s4, "S", struct S4, SAVE_FIELD_EX(struct S4, s, SV_STR, .dims = { 4 }));

struct El { uint8_t v; };
struct Sa3 { struct El e[3]; };
struct Sa2 { struct El e[2]; };
SAVE_STRUCT(El, struct El, SAVE_FIELD(struct El, v, SV_U8));
TDESC(sa3, "Sa", struct Sa3, SAVE_SUB_EX(struct Sa3, e, El, .dims = { 3 }));
TDESC(sa2, "Sa", struct Sa2, SAVE_SUB_EX(struct Sa2, e, El, .dims = { 2 }));

static void test_arrays(void)
{
    struct SaveError err;

    section("arrays");
    {
        struct Ar2 o = { { 1, 2 } };
        struct Ar8 n;
        memset(&n, 0, sizeof(n));
        CHECK(convert1(&ar2, &o, &ar8d, &n, &err) == SVR_Ok, "grow refused");
        CHECK((n.v[0] == 1) && (n.v[1] == 2) && (n.v[2] == 9) && (n.v[7] == 9), "grown entries didn't get the default");
    }
    {
        struct Ar4 live = { { 1, 2, 3, 4 } };
        struct Ar4 spare = { { 1, 2, 0, 0 } };
        struct Ar2 n;
        memset(&n, 0, sizeof(n));
        CHECK(convert1(&ar4, &live, &ar2, &n, &err) == SVR_Unsupported, "shrinking over live entries wasn't refused");
        memset(&n, 0, sizeof(n));
        CHECK((convert1(&ar4, &spare, &ar2, &n, &err) == SVR_Ok) && (n.v[0] == 1) && (n.v[1] == 2), "shrinking over empty entries failed");
    }
    {
        struct G23 o = { { { 1, 2, 3 }, { 4, 5, 6 } } };
        struct G34 n;
        memset(&n, 0, sizeof(n));
        CHECK(convert1(&g23, &o, &g34, &n, &err) == SVR_Ok, "2D grow refused");
        CHECK((n.m[0][0] == 1) && (n.m[0][2] == 3) && (n.m[1][0] == 4) && (n.m[1][2] == 6) && (n.m[0][3] == 0) && (n.m[2][0] == 0),
            "2D grow moved entries");
        {
            struct G34 big = { { { 1, 2, 0, 0 }, { 5, 6, 0, 0 }, { 0, 0, 0, 0 } } };
            struct G23 small;
            memset(&small, 0, sizeof(small));
            CHECK((convert1(&g34, &big, &g23, &small, &err) == SVR_Ok) && (small.m[1][1] == 6), "2D shrink over empty entries failed");
            big.m[2][1] = 1;
            memset(&small, 0, sizeof(small));
            CHECK(convert1(&g34, &big, &g23, &small, &err) == SVR_Unsupported, "2D shrink over a live row wasn't refused");
        }
    }
    {
        struct S8 o;
        struct S4 n;
        memset(&o, 0, sizeof(o));
        memset(&n, 0, sizeof(n));
        memcpy(o.s, "hello", 6);
        CHECK(convert1(&s8, &o, &s4, &n, &err) == SVR_Ok, "string truncation refused");
        CHECK(strcmp(n.s, "hel") == 0, "truncated string is \"%.4s\", wanted \"hel\" with a terminator", n.s);
    }
    {
        struct Sa3 live = { { { 1 }, { 2 }, { 3 } } };
        struct Sa3 spare = { { { 1 }, { 2 }, { 0 } } };
        struct Sa2 n;
        memset(&n, 0, sizeof(n));
        CHECK(convert1(&sa3, &live, &sa2, &n, &err) == SVR_Unsupported, "struct array shrink over a live entry wasn't refused");
        memset(&n, 0, sizeof(n));
        CHECK((convert1(&sa3, &spare, &sa2, &n, &err) == SVR_Ok) && (n.e[1].v == 2), "struct array shrink over an empty entry failed");
    }
}
/******************************************************************************/
/* Unions: member chosen by the discriminator, by name; a member this build lacks is refused. */
struct Ut { uint8_t kind; union { struct { uint16_t a; } m1; struct { uint32_t b; } m2; } u; };
SAVE_STRUCT(Um1, struct Ut, SAVE_FIELD(struct Ut, u.m1.a, SV_U16));
SAVE_STRUCT(Um2, struct Ut, SAVE_FIELD(struct Ut, u.m2.b, SV_U32));
SAVE_UNION_DEF(UtBoth, "kind",
    SAVE_UNION_MEMBER_SUB("m1", Um1, .nvalues = 1, .values = { 1 }),
    SAVE_UNION_MEMBER_SUB("m2", Um2, .nvalues = 1, .values = { 2 }));
SAVE_UNION_DEF(UtOne, "kind",
    SAVE_UNION_MEMBER_SUB("m1", Um1, .nvalues = 1, .values = { 1 }));
TDESC(ut_both, "Ut", struct Ut, SAVE_FIELD(struct Ut, kind, SV_U8),
    SAVE_UNION(struct Ut, u, "u", sizeof(((struct Ut *)0)->u), UtBoth));
TDESC(ut_one, "Ut", struct Ut, SAVE_FIELD(struct Ut, kind, SV_U8),
    SAVE_UNION(struct Ut, u, "u", sizeof(((struct Ut *)0)->u), UtOne));

static void test_unions(void)
{
    struct SaveError err;
    struct Ut o, n;

    section("unions");
    memset(&o, 0, sizeof(o));
    memset(&n, 0, sizeof(n));
    o.kind = 1;
    o.u.m1.a = 0x1234;
    CHECK((convert1(&ut_both, &o, &ut_both, &n, &err) == SVR_Ok) && (n.kind == 1) && (n.u.m1.a == 0x1234), "member 1 didn't round-trip");
    memset(&o, 0, sizeof(o));
    memset(&n, 0, sizeof(n));
    o.kind = 2;
    o.u.m2.b = 0xDEADBEEFu;
    CHECK((convert1(&ut_both, &o, &ut_both, &n, &err) == SVR_Ok) && (n.kind == 2) && (n.u.m2.b == 0xDEADBEEFu), "member 2 didn't round-trip");
    memset(&n, 0, sizeof(n));
    CHECK(convert1(&ut_both, &o, &ut_one, &n, &err) == SVR_Unsupported, "a union member this build lacks wasn't refused");
    memset(&o, 0, sizeof(o));
    memset(&n, 0, sizeof(n));
    o.kind = 9; /* selects no member: nothing stored, nothing restored */
    CHECK((convert1(&ut_both, &o, &ut_both, &n, &err) == SVR_Ok) && (n.kind == 9) && (n.u.m2.b == 0), "an unselected union wasn't empty");
}
/******************************************************************************/
/* Raw memory (a union member with no table, an opaque field): flagged when it isn't all zero, and refused when it
   was made by a machine with the other byte order. */
struct Rw { uint8_t kind; uint32_t raw_u; uint8_t opaque[4]; };
SAVE_UNION_DEF(RwU, "kind",
    SAVE_UNION_MEMBER_RAW("raw", 4, "raw_u", .nvalues = 1, .values = { 1 }));
TDESC(rw, "Rw", struct Rw, SAVE_FIELD(struct Rw, kind, SV_U8),
    SAVE_UNION(struct Rw, raw_u, "raw", 4, RwU), SAVE_OPAQUE(struct Rw, opaque, "opaque", 4));

static enum SaveResult decode_rw(uint32_t chunk_flags, const struct SaveBuffer *rec, struct Rw *out, struct SaveError *err)
{
    struct Xcode x;
    const struct SaveStructDesc *roots[1] = { &rw };
    uint32_t used;
    enum SaveResult r = xcode_open(&x, roots, 1, SVM_Save, SVM_Save, err);
    if (r != SVR_Ok)
        return r;
    save_decoder_set_chunk_flags(&x.dec, chunk_flags);
    r = save_decode_record(&x.dec, &rw, rec->data, rec->len, &used, out, err);
    xcode_close(&x);
    return r;
}

static void test_raw_bytes(void)
{
    struct SaveError err;
    struct SaveBuffer rec;
    struct Rw in, out;
    int host_big = !save_host_is_little_endian();
    uint32_t same_order = SCF_RawBytes | (host_big ? SCF_RawBigEndian : 0);
    uint32_t other_order = SCF_RawBytes | (host_big ? 0 : SCF_RawBigEndian);

    section("raw memory");
    memset(&in, 0, sizeof(in));
    memset(&rec, 0, sizeof(rec));
    CHECK(save_encode_record(&rec, &rw, &in, SVM_Save, &err) == SVR_Ok, "encode failed");
    CHECK(!rec.has_raw_bytes, "zero raw memory was flagged");
    save_buf_free(&rec);

    in.kind = 1;
    in.raw_u = 0x01020304u;
    CHECK(save_encode_record(&rec, &rw, &in, SVM_Save, &err) == SVR_Ok, "encode failed");
    CHECK(rec.has_raw_bytes, "a raw union member wasn't flagged");
    memset(&out, 0, sizeof(out));
    CHECK((decode_rw(same_order, &rec, &out, &err) == SVR_Ok) && (out.raw_u == 0x01020304u), "same byte order was refused");
    CHECK(decode_rw(other_order, &rec, &out, &err) == SVR_Unsupported, "raw memory of the other byte order wasn't refused");
    CHECK(decode_rw(0, &rec, &out, &err) == SVR_Ok, "a chunk with no raw memory flag was refused");
    save_buf_free(&rec);

    memset(&in, 0, sizeof(in));
    in.opaque[2] = 7;
    CHECK(save_encode_record(&rec, &rw, &in, SVM_Save, &err) == SVR_Ok, "encode failed");
    CHECK(rec.has_raw_bytes, "an opaque field wasn't flagged");
    CHECK(decode_rw(other_order, &rec, &out, &err) == SVR_Unsupported, "an opaque field of the other byte order wasn't refused");
    save_buf_free(&rec);

    memset(&in, 0, sizeof(in));
    in.kind = 1;
    CHECK(save_encode_record(&rec, &rw, &in, SVM_Save, &err) == SVR_Ok, "encode failed");
    CHECK(decode_rw(other_order, &rec, &out, &err) == SVR_Ok, "all-zero raw memory of the other byte order was refused");
    save_buf_free(&rec);

    /* The chunk gets the flags from the buffer. */
    {
        struct SaveBuffer file, payload;
        struct SaveReader reader;
        struct SaveChunk chunk;
        memset(&file, 0, sizeof(file));
        memset(&payload, 0, sizeof(payload));
        payload.has_raw_bytes = 1;
        CHECK((save_file_begin(&file, SFK_Save) == 0) && (save_chunk_add_buffer(&file, SCID_Game, SCF_Required, &payload, 0) == 0), "chunk add failed");
        CHECK((save_reader_open(&reader, file.data, file.len, &err) == SVR_Ok) && (save_reader_next(&reader, &chunk, &err) == SVR_Ok), "chunk read failed");
        CHECK(((chunk.flags & SCF_RawBytes) != 0) && (((chunk.flags & SCF_RawBigEndian) != 0) == host_big), "the chunk's raw memory flags are wrong");
        save_buf_free(&file);
    }
}
/******************************************************************************/
/* Selected unions: a registered function tells which member is in use, and the bytes the member doesn't cover are
   kept (so a member that was picked wrongly loses nothing) and flagged as raw memory. */
struct Sel { uint8_t kind; union { struct { uint32_t wide; } w; struct { uint8_t narrow; } n; uint8_t bytes[8]; } u; };
SAVE_STRUCT(SelWide, struct Sel, SAVE_FIELD(struct Sel, u.w.wide, SV_U32));
SAVE_STRUCT(SelNarrow, struct Sel, SAVE_FIELD(struct Sel, u.n.narrow, SV_U8));
SAVE_UNION_SELECTED_DEF(SelU, "test_member",
    SAVE_UNION_MEMBER_SUB("wide", SelWide, .nvalues = 1, .values = { 11 }),
    SAVE_UNION_MEMBER_SUB("narrow", SelNarrow, .nvalues = 1, .values = { 22 }),
    SAVE_UNION_MEMBER_RAW("any", 8, "*", .is_default = 1));
SAVE_UNION_SELECTED_DEF(SelNone, "never_registered",
    SAVE_UNION_MEMBER_SUB("wide", SelWide, .nvalues = 1, .values = { 11 }),
    SAVE_UNION_MEMBER_RAW("any", 8, "*", .is_default = 1));
TDESC(sel, "Sel", struct Sel, SAVE_FIELD(struct Sel, kind, SV_U8),
    SAVE_UNION(struct Sel, u, "u", sizeof(((struct Sel *)0)->u), SelU));
TDESC(sel_none, "Sel", struct Sel, SAVE_FIELD(struct Sel, kind, SV_U8),
    SAVE_UNION(struct Sel, u, "u", sizeof(((struct Sel *)0)->u), SelNone));

static int64_t test_member(const void *owner)
{
    return ((const struct Sel *)owner)->kind == 1 ? 11 : 22;
}

static enum SaveResult decode_sel(const struct SaveStructDesc *d, uint32_t chunk_flags, const struct SaveBuffer *rec,
    struct Sel *out, struct SaveError *err)
{
    struct Xcode x;
    const struct SaveStructDesc *roots[1] = { d };
    uint32_t used;
    enum SaveResult r = xcode_open(&x, roots, 1, SVM_Save, SVM_Save, err);
    if (r != SVR_Ok)
        return r;
    save_decoder_set_chunk_flags(&x.dec, chunk_flags);
    r = save_decode_record(&x.dec, d, rec->data, rec->len, &used, out, err);
    xcode_close(&x);
    return r;
}

static void test_selected_unions(void)
{
    struct SaveError err;
    struct SaveBuffer rec;
    struct Sel in, out;
    int host_big = !save_host_is_little_endian();
    uint32_t same_order = SCF_RawBytes | (host_big ? SCF_RawBigEndian : 0);
    uint32_t other_order = SCF_RawBytes | (host_big ? 0 : SCF_RawBigEndian);

    section("selected unions");
    save_register_union_selector("test_member", test_member);

    /* the member covers everything in use: nothing raw */
    memset(&in, 0, sizeof(in));
    memset(&rec, 0, sizeof(rec));
    in.kind = 1;
    in.u.w.wide = 0x11223344u;
    CHECK(save_encode_record(&rec, &sel, &in, SVM_Save, &err) == SVR_Ok, "encode failed");
    CHECK(!rec.has_raw_bytes, "a member that covers its data was flagged raw");
    memset(&out, 0, sizeof(out));
    CHECK((decode_sel(&sel, other_order, &rec, &out, &err) == SVR_Ok) && (out.u.w.wide == 0x11223344u), "the wide member didn't round-trip");
    save_buf_free(&rec);

    /* a member picked wrongly: nothing is lost, and the leftover is flagged */
    memset(&in, 0, sizeof(in));
    in.kind = 2;
    in.u.w.wide = 0x11223344u;
    CHECK(save_encode_record(&rec, &sel, &in, SVM_Save, &err) == SVR_Ok, "encode failed");
    CHECK(rec.has_raw_bytes, "the bytes past a member weren't flagged");
    memset(&out, 0, sizeof(out));
    CHECK((decode_sel(&sel, same_order, &rec, &out, &err) == SVR_Ok) && (memcmp(&in, &out, sizeof(in)) == 0), "bytes past a member were lost");
    CHECK(decode_sel(&sel, other_order, &rec, &out, &err) == SVR_Unsupported, "bytes past a member of the other byte order weren't refused");
    save_buf_free(&rec);

    /* no registered selector: the default member keeps the bytes */
    memset(&in, 0, sizeof(in));
    in.kind = 1;
    in.u.w.wide = 0x01020304u;
    CHECK(save_encode_record(&rec, &sel_none, &in, SVM_Save, &err) == SVR_Ok, "encode failed");
    CHECK(rec.has_raw_bytes, "the default member wasn't flagged");
    memset(&out, 0, sizeof(out));
    CHECK((decode_sel(&sel_none, same_order, &rec, &out, &err) == SVR_Ok) && (memcmp(&in, &out, sizeof(in)) == 0), "the default member lost bytes");
    save_buf_free(&rec);
}
/******************************************************************************/
/* Modes: SAVE and SYNC pick different fields, and a SYNC load changes a live struct in place. */
struct Sy { uint8_t both; uint8_t save_only; uint8_t sync_only; uint8_t derived; uint8_t runtime; };
TDESC(sy, "Sy", struct Sy,
    SAVE_FIELD(struct Sy, both, SV_U8), SAVE_FIELD_EX(struct Sy, save_only, SV_U8, .flags = SVF_SAVE),
    SAVE_FIELD_EX(struct Sy, sync_only, SV_U8, .flags = SVF_SYNC), SAVE_DERIVED(struct Sy, derived, "rebuild"),
    SAVE_RUNTIME(struct Sy, runtime));

static void test_modes(void)
{
    struct SaveError err;
    struct Sy src = { 1, 2, 3, 4, 5 };
    struct Sy dst = { 90, 91, 92, 93, 94 };
    struct SaveBuffer rec = { NULL, 0, 0 };
    struct Xcode x;
    const struct SaveStructDesc *roots[1] = { &sy };
    uint32_t used;

    section("save and sync modes");
    CHECK(save_encode_record(&rec, &sy, &src, SVM_Sync, &err) == SVR_Ok, "sync encode failed");
    CHECK(rec.len == 2, "a sync record holds %u bytes, wanted 2 (both + sync_only)", (unsigned)rec.len);
    CHECK(xcode_open(&x, roots, 1, SVM_Sync, SVM_Sync, &err) == SVR_Ok, "sync schema failed");
    CHECK(save_decode_record(&x.dec, &sy, rec.data, rec.len, &used, &dst, &err) == SVR_Ok, "sync decode failed");
    CHECK((dst.both == 1) && (dst.sync_only == 3), "synced fields weren't updated");
    CHECK((dst.save_only == 91) && (dst.derived == 93) && (dst.runtime == 94), "a sync load changed a field it must leave alone");
    xcode_close(&x);
    save_buf_free(&rec);

    CHECK(save_encode_record(&rec, &sy, &src, SVM_Save, &err) == SVR_Ok, "save encode failed");
    CHECK(rec.len == 2, "a save record holds %u bytes, wanted 2 (both + save_only)", (unsigned)rec.len);
    save_buf_free(&rec);
}
/******************************************************************************/
/* Migrations run in from_minor order, ties in table order, and only for files at or below from_minor. */
static char mig_log[16];
static int mig_n;
static int mig_a(struct SaveLoadCtx *c) { (void)c; mig_log[mig_n++] = 'A'; return 0; }
static int mig_b(struct SaveLoadCtx *c) { (void)c; mig_log[mig_n++] = 'B'; return 0; }
static int mig_c(struct SaveLoadCtx *c) { (void)c; mig_log[mig_n++] = 'C'; return 0; }
static int mig_d(struct SaveLoadCtx *c) { (void)c; mig_log[mig_n++] = 'D'; return 0; }
static int mig_fail(struct SaveLoadCtx *c) { (void)c; mig_log[mig_n++] = 'F'; return 1; }

static void test_migrations(void)
{
    static const struct SaveMigration table[] = {
        { "S", 5, "A", mig_a }, { "S", 3, "B", mig_b }, { "S", 5, "C", mig_c }, { "S", 1, "D", mig_d } };
    static const struct SaveMigration failing[] = { { "S", 2, "F", mig_fail }, { "S", 3, "B", mig_b } };
    struct SaveLoadCtx ctx;
    struct SaveError err;

    section("migrations");
    memset(&ctx, 0, sizeof(ctx));
    ctx.file_minor = 2;
    mig_n = 0;
    CHECK(save_migrations_run_table(&ctx, "S", table, 4, &err) == SVR_Ok, "migrations failed");
    mig_log[mig_n] = 0;
    CHECK(strcmp(mig_log, "BAC") == 0, "ran \"%s\", wanted \"BAC\" (D is for older files)", mig_log);
    mig_n = 0;
    CHECK((save_migrations_run_table(&ctx, "Other", table, 4, &err) == SVR_Ok) && (mig_n == 0), "migrations of another struct ran");
    CHECK(save_migrations_run_table(&ctx, "S", failing, 2, &err) == SVR_Unsupported, "a failing migration wasn't reported");
    mig_log[mig_n] = 0;
    CHECK(strcmp(mig_log, "F") == 0, "ran \"%s\" after a failure, wanted it to stop", mig_log);
    CHECK(save_migration_count() == 0, "the real registry isn't empty: add a test for the new migration");
}
/******************************************************************************/
/* Container: header, chunks, compression, checksum; and damaged files. */
struct Rec { uint32_t a; char name[8]; uint16_t arr[4]; };
TDESC(rec_desc, "Rec", struct Rec, SAVE_FIELD(struct Rec, a, SV_U32),
    SAVE_FIELD_EX(struct Rec, name, SV_STR, .dims = { 8 }), SAVE_FIELD_EX(struct Rec, arr, SV_U16, .dims = { 4 }));

static const struct Rec rec_meta = { 0xCAFEBABEu, "meta", { 1, 2, 3, 4 } };
static const struct Rec rec_game = { 0x12345678u, "game", { 500, 600, 700, 800 } };

#define CID(a, b, c, d) SAVE_FOURCC(a, b, c, d)

/** Builds a small, valid file: META (plain), SCHM, GAME and ILVL, then an optional LUA chunk. */
static void build_small(struct SaveBuffer *out, int with_lua, uint32_t extra_id, uint32_t extra_flags, int drop_schema)
{
    struct SaveBuffer meta = { NULL, 0, 0 }, game = { NULL, 0, 0 }, sch = { NULL, 0, 0 };
    struct SaveError err;
    const struct SaveStructDesc *roots[1] = { &rec_desc };
    save_encode_record(&meta, &rec_desc, &rec_meta, SVM_Save, &err);
    save_encode_record(&game, &rec_desc, &rec_game, SVM_Save, &err);
    save_schema_encode(&sch, roots, 1, SVM_Save, &err);
    memset(out, 0, sizeof(*out));
    save_file_begin(out, SFK_Save);
    save_chunk_add(out, SCID_Meta, SCF_Required, meta.data, meta.len, 0);
    if (!drop_schema)
        save_chunk_add(out, SCID_Schema, SCF_Required, sch.data, sch.len, 1);
    if (extra_id != 0)
        save_chunk_add(out, extra_id, extra_flags, "xyz", 3, 0);
    save_chunk_add(out, SCID_Game, SCF_Required, game.data, game.len, 1);
    save_chunk_add(out, SCID_Level, SCF_Required, game.data, game.len, 1);
    if (with_lua)
        save_chunk_add(out, SCID_Lua, 0, "LUABYTES", 8, 1);
    save_buf_free(&meta);
    save_buf_free(&game);
    save_buf_free(&sch);
}

/** Loads a file the way the game's loaders do: every chunk read and checked, then META and GAME decoded. */
static enum SaveResult load_small(const uint8_t *data, uint32_t len, struct Rec *meta_out, struct Rec *game_out, struct SaveError *err)
{
    struct SaveReader rd;
    struct SaveChunk ch;
    uint8_t *meta = NULL, *sch = NULL, *game = NULL, *ilvl = NULL, *lua = NULL;
    uint32_t meta_len = 0, sch_len = 0, game_len = 0, ilvl_len = 0, lua_len = 0;
    enum SaveResult r = save_reader_open(&rd, data, len, err);
    while (r == SVR_Ok)
    {
        uint8_t **dst = NULL;
        uint32_t *dlen = NULL;
        r = save_reader_next(&rd, &ch, err);
        if (r != SVR_Ok)
            break;
        if (ch.id == SCID_Meta) { dst = &meta; dlen = &meta_len; }
        else if (ch.id == SCID_Schema) { dst = &sch; dlen = &sch_len; }
        else if (ch.id == SCID_Game) { dst = &game; dlen = &game_len; }
        else if (ch.id == SCID_Level) { dst = &ilvl; dlen = &ilvl_len; }
        else if (ch.id == SCID_Lua) { dst = &lua; dlen = &lua_len; }
        if (dst != NULL)
        {
            free(*dst);
            *dst = NULL;
            r = save_chunk_unpack(&ch, dst, dlen, err);
        }
    }
    if (r == SVR_NotFound)
        r = SVR_Ok;
    if ((r == SVR_Ok) && ((meta == NULL) || (sch == NULL) || (game == NULL) || (ilvl == NULL)))
        r = save_fail(err, SVR_Damaged, "the file is missing a required chunk");
    if (r == SVR_Ok)
    {
        struct SaveFileSchema fs;
        r = save_schema_decode(sch, sch_len, &fs, err);
        if (r == SVR_Ok)
        {
            struct SaveDecoder dec;
            uint32_t used;
            save_decoder_init(&dec, &fs, SVM_Save);
            memset(meta_out, 0, sizeof(*meta_out));
            memset(game_out, 0, sizeof(*game_out));
            r = save_decode_record(&dec, &rec_desc, meta, meta_len, &used, meta_out, err);
            if (r == SVR_Ok)
                r = save_decode_record(&dec, &rec_desc, game, game_len, &used, game_out, err);
            save_decoder_free(&dec);
            save_schema_free(&fs);
        }
    }
    free(meta);
    free(sch);
    free(game);
    free(ilvl);
    free(lua);
    return r;
}

static int rec_equal(const struct Rec *a, const struct Rec *b)
{
    return (a->a == b->a) && (memcmp(a->name, b->name, sizeof(a->name)) == 0) && (memcmp(a->arr, b->arr, sizeof(a->arr)) == 0);
}

static uint32_t rd32(const uint8_t *p) { return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }
static void wr32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }

/** Offsets of every chunk header; returns the count. */
static uint32_t chunk_offsets(const struct SaveBuffer *f, uint32_t *offs, uint32_t max)
{
    uint32_t n = 0;
    uint32_t pos = SAVE_HEADER_SIZE;
    while ((pos + SAVE_CHUNK_HEADER_SIZE <= f->len) && (n < max))
    {
        offs[n++] = pos;
        pos += SAVE_CHUNK_HEADER_SIZE + rd32(f->data + pos + 8);
    }
    return n;
}

static void fix_header_crc(uint8_t *f)
{
    wr32(f + 12, (uint32_t)crc32(0, f, 12));
}

static uint64_t g_rng = 0x2545F4914F6CDD1DULL;
static uint32_t rnd32(void)
{
    g_rng ^= g_rng << 13;
    g_rng ^= g_rng >> 7;
    g_rng ^= g_rng << 17;
    return (uint32_t)(g_rng >> 11);
}

static void test_container(void)
{
    struct SaveBuffer f;
    struct SaveError err;
    struct Rec m, g;
    uint32_t offs[8];
    uint32_t n;

    section("container");
    build_small(&f, 1, 0, 0, 0);
    CHECK(load_small(f.data, f.len, &m, &g, &err) == SVR_Ok, "a good file was refused: %s", err.message);
    CHECK(rec_equal(&m, &rec_meta) && rec_equal(&g, &rec_game), "a good file decoded to other values");
    n = chunk_offsets(&f, offs, 8);
    CHECK(n == 5, "expected 5 chunks, found %u", (unsigned)n);
    CHECK((rd32(f.data + offs[0] + 4) & SCF_Zlib) == 0, "META is compressed; it has to stay plain so the load menu can read it");
    save_buf_free(&f);

    /* Compression: a compressible chunk is stored smaller, flagged, and comes back identical. */
    {
        static uint8_t zeros[4096];
        struct SaveReader rd;
        struct SaveChunk ch;
        uint8_t *raw = NULL;
        uint32_t raw_len = 0;
        memset(zeros, 0, sizeof(zeros));
        memset(&f, 0, sizeof(f));
        CHECK((save_file_begin(&f, SFK_Save) == 0) && (save_chunk_add(&f, SCID_Game, 0, zeros, sizeof(zeros), 1) == 0), "writing failed");
        CHECK(save_reader_open(&rd, f.data, f.len, &err) == SVR_Ok, "open failed");
        CHECK(save_reader_next(&rd, &ch, &err) == SVR_Ok, "next failed");
        CHECK(((ch.flags & SCF_Zlib) != 0) && (ch.stored_len < ch.raw_len) && (ch.raw_len == sizeof(zeros)), "chunk wasn't compressed");
        CHECK((save_chunk_unpack(&ch, &raw, &raw_len, &err) == SVR_Ok) && (raw_len == sizeof(zeros)) && (memcmp(raw, zeros, sizeof(zeros)) == 0),
            "compressed chunk came back different");
        free(raw);
        save_buf_free(&f);
    }

    section("damaged files");
    build_small(&f, 1, 0, 0, 0);
    n = chunk_offsets(&f, offs, 8);
    {
        struct SaveBuffer w;

        /* Header. */
        w = f; w.data = (uint8_t *)malloc(f.len); memcpy(w.data, f.data, f.len);
        w.data[0] = 0;
        CHECK(load_small(w.data, w.len, &m, &g, &err) == SVR_TooOld, "a file without the magic gave %d", (int)err.result);
        memcpy(w.data, f.data, f.len);
        w.data[12] ^= 0xFF;
        CHECK(load_small(w.data, w.len, &m, &g, &err) == SVR_Damaged, "a bad header checksum wasn't Damaged");
        memcpy(w.data, f.data, f.len);
        w.data[4] = (uint8_t)(SAVE_FORMAT_MAJOR + 1);
        fix_header_crc(w.data);
        CHECK(load_small(w.data, w.len, &m, &g, &err) == SVR_TooNew, "a newer major version wasn't TooNew");
        memcpy(w.data, f.data, f.len);
        w.data[4] = 0;
        w.data[5] = 0;
        fix_header_crc(w.data);
        CHECK(load_small(w.data, w.len, &m, &g, &err) == SVR_TooOld, "major version 0 wasn't TooOld");

        /* Lengths. */
        for (uint32_t c = 0; c < n; c++)
        {
            memcpy(w.data, f.data, f.len);
            wr32(w.data + offs[c] + 8, 0xFFFFFFFFu);
            CHECK(load_small(w.data, w.len, &m, &g, &err) == SVR_Damaged, "chunk %u: a huge stored length wasn't Damaged", (unsigned)c);
            memcpy(w.data, f.data, f.len);
            wr32(w.data + offs[c] + 12, 0xFFFFFFFFu);
            CHECK(load_small(w.data, w.len, &m, &g, &err) == SVR_Damaged, "chunk %u: a huge raw length wasn't Damaged", (unsigned)c);
            memcpy(w.data, f.data, f.len);
            wr32(w.data + offs[c] + 12, rd32(w.data + offs[c] + 12) + 1);
            CHECK(load_small(w.data, w.len, &m, &g, &err) == SVR_Damaged, "chunk %u: a wrong raw length wasn't Damaged", (unsigned)c);
            memcpy(w.data, f.data, f.len);
            w.data[offs[c] + SAVE_CHUNK_HEADER_SIZE] ^= 0x55; /* first body byte */
            CHECK(load_small(w.data, w.len, &m, &g, &err) == SVR_Damaged, "chunk %u: a changed body wasn't Damaged", (unsigned)c);
        }
        free(w.data);
    }

    /* Truncation at every length. The only cut that leaves a usable file is the one right after ILVL
       (LUA is optional); any other cut leaves a half chunk or a missing required chunk. */
    {
        uint32_t lvl_end = offs[3] + SAVE_CHUNK_HEADER_SIZE + rd32(f.data + offs[3] + 8);
        for (uint32_t len = 0; len < f.len; len++)
        {
            enum SaveResult r = load_small(f.data, len, &m, &g, &err);
            if (len == lvl_end)
                CHECK(r == SVR_Ok, "a cut right after ILVL was refused: %s", err.message);
            else
                CHECK(r != SVR_Ok, "cut at %u was accepted", (unsigned)len);
        }
    }
    save_buf_free(&f);

    /* Chunk sets. */
    build_small(&f, 0, 0, 0, 1);
    CHECK(load_small(f.data, f.len, &m, &g, &err) == SVR_Damaged, "a file without SCHM wasn't Damaged");
    save_buf_free(&f);
    build_small(&f, 1, CID('Z', 'Z', 'Z', 'Z'), 0, 0);
    CHECK((load_small(f.data, f.len, &m, &g, &err) == SVR_Ok) && rec_equal(&g, &rec_game), "an unknown optional chunk wasn't skipped");
    save_buf_free(&f);
    build_small(&f, 1, CID('Z', 'Z', 'Z', 'Z'), SCF_Required, 0);
    CHECK(load_small(f.data, f.len, &m, &g, &err) == SVR_TooNew, "an unknown required chunk wasn't TooNew");
    CHECK(strstr(err.message, "ZZZZ") != NULL, "the refusal doesn't name the chunk: %s", err.message);
    save_buf_free(&f);
    build_small(&f, 0, CID('G', 'A', 'M', 'E'), SCF_Required, 0); /* a second chunk with the same id */
    CHECK(load_small(f.data, f.len, &m, &g, &err) != SVR_NoMemory, "duplicate chunks crashed or ran out of memory");
    save_buf_free(&f);

    /* Random damage. Whatever happens, nothing crashes; a file that still loads holds the original values. */
    build_small(&f, 1, 0, 0, 0);
    {
        struct SaveBuffer w;
        int accepted = 0;
        w.len = f.len;
        w.cap = f.len;
        w.data = (uint8_t *)malloc(f.len);
        for (int it = 0; it < 6000; it++)
        {
            uint32_t len = f.len;
            uint32_t edits = 1 + (rnd32() % 4);
            enum SaveResult r;
            memcpy(w.data, f.data, f.len);
            for (uint32_t e = 0; e < edits; e++)
                w.data[rnd32() % f.len] = (uint8_t)rnd32();
            if ((rnd32() % 4) == 0)
                len = rnd32() % (f.len + 1);
            r = load_small(w.data, len, &m, &g, &err);
            CHECK((r >= SVR_Ok) && (r <= SVR_NoMemory), "result %d isn't a SaveResult", (int)r);
            if (r == SVR_Ok)
            {
                accepted++;
                CHECK(rec_equal(&m, &rec_meta) && rec_equal(&g, &rec_game), "a damaged file loaded with other values (iteration %d)", it);
            }
        }
        printf("   %d of 6000 damaged files still loaded (header flag edits and untouched copies), none with changed values\n", accepted);
        free(w.data);
    }
    save_buf_free(&f);
}
/******************************************************************************/
/* save_inspect_game: the cheap check the load menus use. Same structural cases as the full loader. */
struct MemSrc { const uint8_t *data; uint32_t len; };

static int mem_read_at(void *ctx, uint32_t off, void *dst, uint32_t len)
{
    const struct MemSrc *m = (const struct MemSrc *)ctx;
    if ((off > m->len) || (len > m->len - off))
        return -1;
    memcpy(dst, m->data + off, len);
    return 0;
}

static enum SaveResult inspect_small(const uint8_t *data, uint32_t len, struct Rec *meta, int *ok, struct SaveError *err)
{
    struct MemSrc m = { data, len };
    memset(meta, 0, sizeof(*meta));
    return save_inspect_game(mem_read_at, &m, len, &rec_desc, meta, ok, err);
}

static void test_inspect(void)
{
    struct SaveBuffer f;
    struct SaveError err;
    struct Rec m, g;
    uint32_t offs[8];
    int ok;

    section("save inspection");
    build_small(&f, 1, 0, 0, 0);
    chunk_offsets(&f, offs, 8);
    CHECK((inspect_small(f.data, f.len, &m, &ok, &err) == SVR_Ok) && ok && rec_equal(&m, &rec_meta), "a good file wasn't read: %s", err.message);
    {
        uint32_t lvl_end = offs[3] + SAVE_CHUNK_HEADER_SIZE + rd32(f.data + offs[3] + 8);
        for (uint32_t len = 0; len < f.len; len++)
        {
            enum SaveResult r = inspect_small(f.data, len, &m, &ok, &err);
            CHECK((r == SVR_Ok) == (len == lvl_end), "cut at %u gave result %d", (unsigned)len, (int)r);
            /* The title survives as long as META and SCHM are whole, so a cut file is still listed by name. */
            CHECK((ok != 0) == (len >= offs[2]), "cut at %u: title read = %d", (unsigned)len, ok);
            if (ok)
                CHECK(rec_equal(&m, &rec_meta), "cut at %u: wrong title", (unsigned)len);
        }
    }
    {
        /* Only headers are checked for the big chunks: a changed GAME body passes here and fails the full load. */
        struct SaveBuffer w = f;
        w.data = (uint8_t *)malloc(f.len);
        memcpy(w.data, f.data, f.len);
        w.data[offs[2] + SAVE_CHUNK_HEADER_SIZE] ^= 0x55;
        CHECK(inspect_small(w.data, w.len, &m, &ok, &err) == SVR_Ok, "a changed GAME body was refused by the cheap check");
        CHECK(load_small(w.data, w.len, &m, &g, &err) == SVR_Damaged, "a changed GAME body wasn't refused by the full load");
        w.data[0] = 0;
        CHECK((inspect_small(w.data, w.len, &m, &ok, &err) == SVR_TooOld) && !ok, "a file without the magic wasn't TooOld");
        free(w.data);
    }
    save_buf_free(&f);

    build_small(&f, 1, CID('Z', 'Z', 'Z', 'Z'), SCF_Required, 0);
    CHECK((inspect_small(f.data, f.len, &m, &ok, &err) == SVR_TooNew) && ok, "an unknown required chunk: result %d, title read %d", (int)err.result, ok);
    save_buf_free(&f);
    build_small(&f, 1, CID('Z', 'Z', 'Z', 'Z'), 0, 0);
    CHECK((inspect_small(f.data, f.len, &m, &ok, &err) == SVR_Ok) && ok, "an unknown optional chunk wasn't skipped");
    save_buf_free(&f);

    /* Random damage: nothing crashes, and a file that reads a title has the right one. */
    build_small(&f, 1, 0, 0, 0);
    {
        struct SaveBuffer w;
        w.len = f.len;
        w.cap = f.len;
        w.data = (uint8_t *)malloc(f.len);
        for (int it = 0; it < 6000; it++)
        {
            uint32_t len = f.len;
            uint32_t edits = 1 + (rnd32() % 4);
            enum SaveResult r;
            memcpy(w.data, f.data, f.len);
            for (uint32_t e = 0; e < edits; e++)
                w.data[rnd32() % f.len] = (uint8_t)rnd32();
            if ((rnd32() % 4) == 0)
                len = rnd32() % (f.len + 1);
            r = inspect_small(w.data, len, &m, &ok, &err);
            CHECK((r >= SVR_Ok) && (r <= SVR_NoMemory), "result %d isn't a SaveResult", (int)r);
            if (ok)
                CHECK(rec_equal(&m, &rec_meta), "a damaged file gave a wrong title (iteration %d)", it);
        }
        free(w.data);
    }
    save_buf_free(&f);
}
/******************************************************************************/
/* save_recfile: the shared shape of the high score, net config and continue files. */
static void make_recs(struct Rec *r, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++)
    {
        memset(&r[i], 0, sizeof(r[i]));
        r[i].a = 1000 + i;
        snprintf(r[i].name, sizeof(r[i].name), "rec%u", (unsigned)i);
        for (uint32_t k = 0; k < 4; k++)
            r[i].arr[k] = (uint16_t)(i * 4 + k);
    }
}

static int recs_equal(const struct Rec *a, const struct Rec *b, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++)
        if (!rec_equal(&a[i], &b[i]))
            return 0;
    return 1;
}

/** A string that is not the first member of its struct is diffed and applied at its own place in the struct. */
static void test_config_overlay_strings(void)
{
    struct SaveError err;
    struct SaveBuffer overlay = { NULL, 0, 0 };
    struct Configs *baseline = (struct Configs *)calloc(1, sizeof(struct Configs));
    struct Configs *current = (struct Configs *)calloc(1, sizeof(struct Configs));
    struct Configs *loaded = (struct Configs *)calloc(1, sizeof(struct Configs));
    section("config overlay strings");
    if ((baseline == NULL) || (current == NULL) || (loaded == NULL))
    {
        CHECK(0, "out of memory");
        free(baseline); free(current); free(loaded);
        return;
    }
    /* Only the number in front of the string changes: the string must not appear in the overlay. */
    current->slab_conf.room_cfgstats[1].msg_needed.id = 7;
    CHECK(save_config_overlay_write(current, baseline, &overlay, &err) == SVR_Ok, "overlay write: %s", err.message);
    CHECK(save_config_overlay_apply(overlay.data, overlay.len, loaded, &err) == SVR_Ok, "overlay apply: %s", err.message);
    CHECK(loaded->slab_conf.room_cfgstats[1].msg_needed.id == 7, "the number was not applied");
    CHECK(loaded->slab_conf.room_cfgstats[1].msg_needed.path[0] == '\0', "an unchanged string came back as \"%s\"",
        loaded->slab_conf.room_cfgstats[1].msg_needed.path);
    save_buf_free(&overlay);
    memset(loaded, 0, sizeof(*loaded));

    /* Only the string changes: it must be applied. */
    memset(current, 0, sizeof(*current));
    snprintf(current->slab_conf.room_cfgstats[1].msg_needed.path, sizeof(current->slab_conf.room_cfgstats[1].msg_needed.path), "needed.wav");
    CHECK(save_config_overlay_write(current, baseline, &overlay, &err) == SVR_Ok, "overlay write: %s", err.message);
    CHECK(save_config_overlay_apply(overlay.data, overlay.len, loaded, &err) == SVR_Ok, "overlay apply: %s", err.message);
    CHECK(strcmp(loaded->slab_conf.room_cfgstats[1].msg_needed.path, "needed.wav") == 0, "the string came back as \"%s\"",
        loaded->slab_conf.room_cfgstats[1].msg_needed.path);
    save_buf_free(&overlay);
    free(baseline); free(current); free(loaded);
}

static void test_recfile(void)
{
    struct SaveError err;
    struct SaveBuffer f = { NULL, 0, 0 };
    struct Rec src[40];
    void *out = NULL;
    uint32_t count = 99;
    enum { KIND_A = SFK_HighScore, KIND_B = SFK_NetConfig };

    section("record files");
    make_recs(src, 40);
    for (uint32_t n = 0; n <= 40; n += (n < 3) ? 1 : 37)
    {
        memset(&f, 0, sizeof(f));
        CHECK(save_recfile_build(&f, KIND_A, &rec_desc, src, sizeof(src[0]), n, &err) == SVR_Ok, "build of %u records failed: %s", (unsigned)n, err.message);
        CHECK(save_recfile_parse(f.data, f.len, KIND_A, &rec_desc, sizeof(struct Rec), &out, &count, &err) == SVR_Ok, "parse of %u records failed: %s", (unsigned)n, err.message);
        CHECK(count == n, "%u records came back as %u", (unsigned)n, (unsigned)count);
        CHECK((n == 0) == (out == NULL), "records pointer is %s for %u records", out ? "set" : "NULL", (unsigned)n);
        if (out != NULL)
            CHECK(recs_equal((const struct Rec *)out, src, n), "%u records came back different", (unsigned)n);
        free(out);
        out = NULL;
        save_buf_free(&f);
    }

    CHECK(save_recfile_build(&f, KIND_A, &rec_desc, src, sizeof(src[0]), 3, &err) == SVR_Ok, "build failed");
    CHECK(save_recfile_parse(f.data, f.len, KIND_B, &rec_desc, sizeof(struct Rec), &out, &count, &err) == SVR_Damaged, "a file of another kind was accepted");
    CHECK((out == NULL) && (count == 0), "a refused file still returned records");

    /* Cut at every length: only the whole file parses. */
    for (uint32_t len = 0; len < f.len; len++)
    {
        enum SaveResult r = save_recfile_parse(f.data, len, KIND_A, &rec_desc, sizeof(struct Rec), &out, &count, &err);
        CHECK(r != SVR_Ok, "cut at %u was accepted", (unsigned)len);
        free(out);
        out = NULL;
    }

    /* Random damage: nothing crashes, and a file that still parses holds the original records. */
    {
        uint8_t *w = (uint8_t *)malloc(f.len);
        for (int it = 0; it < 3000; it++)
        {
            uint32_t edits = 1 + (rnd32() % 4);
            enum SaveResult r;
            memcpy(w, f.data, f.len);
            for (uint32_t e = 0; e < edits; e++)
                w[rnd32() % f.len] = (uint8_t)rnd32();
            r = save_recfile_parse(w, f.len, KIND_A, &rec_desc, sizeof(struct Rec), &out, &count, &err);
            if (r == SVR_Ok)
                CHECK((count == 3) && recs_equal((const struct Rec *)out, src, 3), "a damaged file parsed to other records (iteration %d)", it);
            free(out);
            out = NULL;
        }
        free(w);
    }
    save_buf_free(&f);

    /* A file with the right chunk layout for a savegame isn't a record file. */
    build_small(&f, 0, 0, 0, 0);
    CHECK(save_recfile_parse(f.data, f.len, SFK_Save, &rec_desc, sizeof(struct Rec), &out, &count, &err) == SVR_Damaged, "a save was accepted as a record file");
    save_buf_free(&f);
}
/******************************************************************************/
/* Table validation. The game doesn't validate its tables when it saves or loads, so this is where a
   wrong table is caught: every real table, every synthetic one above, and some deliberately wrong ones. */
struct Bd { uint32_t a; uint32_t b; };
TDESC(bad_dup, "Bd", struct Bd, SAVE_FIELD(struct Bd, a, SV_U32), SAVE_FIELD(struct Bd, a, SV_U32));
TDESC(bad_offset, "Bd", struct Bd, SAVE_FIELD(struct Bd, a, SV_U32), { .name = "x", .offset = 100, .mem_size = 4, .stored_type = SV_U32 });
TDESC(bad_type, "Bd", struct Bd, SAVE_FIELD(struct Bd, a, SV_U32), { .name = "x", .offset = 4, .mem_size = 4, .stored_type = SV_INVALID });
TDESC(bad_width, "Bd", struct Bd, SAVE_FIELD(struct Bd, a, SV_U32), { .name = "x", .offset = 4, .mem_size = 3, .stored_type = SV_U16 });
SAVE_UNION_DEF(UtNoDisc, "nope", SAVE_UNION_MEMBER_SUB("m1", Um1, .nvalues = 1, .values = { 1 }));
TDESC(bad_union, "Ut", struct Ut, SAVE_FIELD(struct Ut, kind, SV_U8),
    SAVE_UNION(struct Ut, u, "u", sizeof(((struct Ut *)0)->u), UtNoDisc));

static void test_tables(void)
{
    struct SaveError err;
    uint32_t count;
    const struct SaveStructDesc *const *roots = save_root_structs(&count);
    const struct SaveStructDesc *good[] = { &cv_old, &cv_new, &cv_clamp, &f_old, &f_new, &nm_old, &nm_new, &ar4, &ar2, &ar8d, &g23, &g34, &s8, &s4, &sa3, &sa2, &ut_both, &ut_one, &sy, &rec_desc };
    const struct SaveStructDesc *bad[] = { &bad_dup, &bad_offset, &bad_type, &bad_width, &bad_union };

    section("table validation");
    for (uint32_t i = 0; i < count; i++)
        CHECK(save_desc_validate(roots[i], &err) == SVR_Ok, "table %s: %s", roots[i]->name, err.message);
    for (size_t i = 0; i < sizeof(good) / sizeof(good[0]); i++)
        CHECK(save_desc_validate(good[i], &err) == SVR_Ok, "synthetic table %s: %s", good[i]->name, err.message);
    for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); i++)
        CHECK(save_desc_validate(bad[i], &err) == SVR_Unsupported, "wrong table %u was accepted", (unsigned)i);
}
/******************************************************************************/
/* The real state: every saved field of struct Game filled from its own path. */
static uint64_t mix64(uint64_t x)
{
    x += 0x9E3779B97F4A7C15ULL;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
    return x ^ (x >> 31);
}

static uint64_t fnv(const char *s)
{
    uint64_t h = 1469598103934665603ULL;
    while (*s)
    {
        h ^= (uint8_t)*s++;
        h *= 1099511628211ULL;
    }
    return h;
}

#define FILL_PATH_MAX 512

static void fill_struct(const struct SaveStructDesc *d, uint8_t *base, char *path, int depth);

static void fill_scalar(const struct SaveFieldDesc *f, uint8_t *p, uint32_t elem, uint64_t h)
{
    struct SaveValue v;
    memset(&v, 0, sizeof(v));
    if ((f->stored_type == SV_F32) || (f->stored_type == SV_F64))
    {
        v.is_float = 1;
        v.real = (double)(int32_t)((h >> 40) & 0xFFFFF) / 64.0 - 8192.0;
    } else if (f->stored_type == SV_BOOL8)
    {
        v.mag = h & 1;
    } else if ((h & 15) == 0)
    {
        v.mag = mix64(h);
        v.neg = (int)((h >> 20) & 1);
    } else
    {
        v.mag = (h >> 8) % 6;
        v.neg = (((h >> 5) & 7) == 0) ? 1 : 0;
    }
    save_value_store_mem(p, elem, f->stored_type, &v, 1);
}

static void fill_struct(const struct SaveStructDesc *d, uint8_t *base, char *path, int depth)
{
    size_t plen = strlen(path);
    for (int pass = 0; pass < 2; pass++)
    {
        for (uint32_t i = 0; i < d->field_count; i++)
        {
            const struct SaveFieldDesc *f = &d->fields[i];
            uint32_t count, elem;
            if (!save_field_included(f, SVM_Save) || ((f->stored_type == SV_UNION) != (pass == 1)))
                continue;
            count = save_field_count(f);
            elem = f->mem_size / count;
            snprintf(path + plen, FILL_PATH_MAX - plen, ".%s", f->name);
            if (f->stored_type == SV_UNION)
            {
                /* The whole region gets bytes, then every member's own fields are laid over it. */
                uint64_t h = fnv(path);
                for (uint32_t b = 0; b < f->mem_size; b++)
                    base[f->offset + b] = (uint8_t)(mix64(h + b / 8) >> ((b % 8) * 8));
                for (uint32_t m = 0; m < f->uni->member_count; m++)
                    if (f->uni->members[m].sub != NULL)
                    {
                        size_t mlen = strlen(path);
                        snprintf(path + mlen, FILL_PATH_MAX - mlen, ":%s", f->uni->members[m].name);
                        if (depth < 12)
                            fill_struct(f->uni->members[m].sub, base, path, depth + 1);
                        path[mlen] = 0;
                    }
            } else if (f->stored_type == SV_STRUCT)
            {
                for (uint32_t k = 0; k < count; k++)
                {
                    size_t flen = strlen(path);
                    snprintf(path + flen, FILL_PATH_MAX - flen, "[%u]", (unsigned)k);
                    if (depth < 12)
                        fill_struct(f->sub, base + f->offset + (size_t)k * elem, path, depth + 1);
                    path[flen] = 0;
                }
            } else if (f->stored_type == SV_STR)
            {
                uint64_t h = fnv(path);
                for (uint32_t k = 0; k < count; k++)
                {
                    uint64_t x = mix64(h + k);
                    base[f->offset + k] = ((x & 3) == 0) ? 0 : (uint8_t)('a' + (x >> 8) % 26);
                }
            } else
            {
                uint64_t h = fnv(path);
                for (uint32_t k = 0; k < count; k++)
                    fill_scalar(f, base + f->offset + (size_t)k * elem, elem, mix64(h + k));
            }
            path[plen] = 0;
        }
    }
    path[plen] = 0;
}

static void fill_record(const struct SaveStructDesc *d, void *rec)
{
    char path[FILL_PATH_MAX];
    snprintf(path, sizeof(path), "%s", d->name);
    fill_struct(d, (uint8_t *)rec, path, 0);
}

/** Builds the sample save: the same chunks, in the same order, as save_build_game(). */
static enum SaveResult build_sample(struct SaveBuffer *out, struct SaveError *err)
{
    struct Game *g = (struct Game *)calloc(1, sizeof(struct Game));
    struct IntralevelData *ilvl = (struct IntralevelData *)calloc(1, sizeof(struct IntralevelData));
    struct Configs *baseline = (struct Configs *)calloc(1, sizeof(struct Configs));
    struct CatalogueEntry *centry = (struct CatalogueEntry *)calloc(1, sizeof(struct CatalogueEntry));
    struct SaveBuffer meta = { NULL, 0, 0 }, sch = { NULL, 0, 0 }, nams = { NULL, 0, 0 }, conf = { NULL, 0, 0 };
    struct SaveBuffer gamebuf = { NULL, 0, 0 }, ilvlbuf = { NULL, 0, 0 };
    uint32_t root_count;
    const struct SaveStructDesc *const *roots = save_root_structs(&root_count);
    enum SaveResult r = SVR_Ok;
    int bad = 0;
    if ((g == NULL) || (ilvl == NULL) || (baseline == NULL) || (centry == NULL))
    {
        free(g); free(ilvl); free(baseline); free(centry);
        return save_fail(err, SVR_NoMemory, "out of memory");
    }
    fill_record(&save_desc_Game, g);
    fill_record(&save_desc_IntralevelData, ilvl);
    fill_record(&save_desc_CatalogueEntry, centry);
    /* conf is runtime state, not part of GAME; it travels as an overlay over a baseline. */
    g->conf.crtr_conf.model_count = 4;
    strcpy(g->conf.crtr_conf.model[1].name, "IMP");
    strcpy(g->conf.crtr_conf.model[2].name, "DRAGON");
    g->conf.crtr_conf.model[2].health = 321;
    r = save_encode_record(&meta, &save_desc_CatalogueEntry, centry, SVM_Save, err);
    if (r == SVR_Ok)
        r = save_schema_encode(&sch, roots, root_count, SVM_Save, err);
    if (r == SVR_Ok)
        r = save_names_write(g, &nams, err);
    if (r == SVR_Ok)
        r = save_config_overlay_write(&g->conf, baseline, &conf, err);
    if (r == SVR_Ok)
        r = save_encode_record(&gamebuf, &save_desc_Game, g, SVM_Save, err);
    if (r == SVR_Ok)
        r = save_encode_record(&ilvlbuf, &save_desc_IntralevelData, ilvl, SVM_Save, err);
    if (r == SVR_Ok)
    {
        memset(out, 0, sizeof(*out));
        bad |= save_file_begin(out, SFK_Save);
        bad |= save_chunk_add(out, SCID_Meta, SCF_Required, meta.data, meta.len, 0);
        bad |= save_chunk_add(out, SCID_Schema, SCF_Required, sch.data, sch.len, 1);
        bad |= save_chunk_add(out, SCID_Names, SCF_Required, nams.data, nams.len, 1);
        bad |= save_chunk_add(out, SCID_Config, SCF_Required, conf.data, conf.len, 1);
        bad |= save_chunk_add(out, SCID_Game, SCF_Required, gamebuf.data, gamebuf.len, 1);
        bad |= save_chunk_add(out, SCID_Level, SCF_Required, ilvlbuf.data, ilvlbuf.len, 1);
        bad |= save_chunk_add(out, SCID_Lua, 0, "LUABYTES", 8, 1);
        if (bad)
            r = save_fail(err, SVR_NoMemory, "out of memory");
    }
    save_buf_free(&meta);
    save_buf_free(&sch);
    save_buf_free(&nams);
    save_buf_free(&conf);
    save_buf_free(&gamebuf);
    save_buf_free(&ilvlbuf);
    free(g); free(ilvl); free(baseline); free(centry);
    return r;
}

/** The chunks of a sample, unpacked. */
struct Unpacked {
    uint8_t *meta, *sch, *nams, *conf, *game, *ilvl, *lua;
    uint32_t meta_len, sch_len, nams_len, conf_len, game_len, ilvl_len, lua_len;
};

static void unpacked_free(struct Unpacked *u)
{
    free(u->meta); free(u->sch); free(u->nams); free(u->conf); free(u->game); free(u->ilvl); free(u->lua);
    memset(u, 0, sizeof(*u));
}

static enum SaveResult unpack_sample(const uint8_t *data, uint32_t len, struct Unpacked *u, struct SaveError *err)
{
    struct SaveReader rd;
    struct SaveChunk ch;
    enum SaveResult r = save_reader_open(&rd, data, len, err);
    memset(u, 0, sizeof(*u));
    while (r == SVR_Ok)
    {
        uint8_t **dst = NULL;
        uint32_t *dlen = NULL;
        r = save_reader_next(&rd, &ch, err);
        if (r != SVR_Ok)
            break;
        switch (ch.id)
        {
        case SCID_Meta:   dst = &u->meta; dlen = &u->meta_len; break;
        case SCID_Schema: dst = &u->sch;  dlen = &u->sch_len;  break;
        case SCID_Names:  dst = &u->nams; dlen = &u->nams_len; break;
        case SCID_Config: dst = &u->conf; dlen = &u->conf_len; break;
        case SCID_Game:   dst = &u->game; dlen = &u->game_len; break;
        case SCID_Level:  dst = &u->ilvl; dlen = &u->ilvl_len; break;
        case SCID_Lua:    dst = &u->lua;  dlen = &u->lua_len;  break;
        default: break;
        }
        if (dst != NULL)
            r = save_chunk_unpack(&ch, dst, dlen, err);
    }
    if (r == SVR_NotFound)
        r = SVR_Ok;
    if ((r == SVR_Ok) && ((u->meta == NULL) || (u->sch == NULL) || (u->game == NULL) || (u->ilvl == NULL)))
        r = save_fail(err, SVR_Damaged, "the sample is missing a required chunk");
    if (r != SVR_Ok)
        unpacked_free(u);
    return r;
}

struct Decoded {
    struct SaveFileSchema fs;
    struct CatalogueEntry *centry;
    struct Game *game;
    struct IntralevelData *ilvl;
};

static void decoded_free(struct Decoded *d)
{
    save_schema_free(&d->fs);
    free(d->centry);
    free(d->game);
    free(d->ilvl);
    memset(d, 0, sizeof(*d));
}

static enum SaveResult decode_sample(const struct Unpacked *u, struct Decoded *d, struct SaveError *err)
{
    struct SaveDecoder dec;
    uint32_t used;
    enum SaveResult r;
    memset(d, 0, sizeof(*d));
    r = save_schema_decode(u->sch, u->sch_len, &d->fs, err);
    if (r != SVR_Ok)
        return r;
    d->centry = (struct CatalogueEntry *)calloc(1, sizeof(struct CatalogueEntry));
    d->game = (struct Game *)calloc(1, sizeof(struct Game));
    d->ilvl = (struct IntralevelData *)calloc(1, sizeof(struct IntralevelData));
    if ((d->centry == NULL) || (d->game == NULL) || (d->ilvl == NULL))
    {
        decoded_free(d);
        return save_fail(err, SVR_NoMemory, "out of memory");
    }
    save_decoder_init(&dec, &d->fs, SVM_Save);
    r = save_decode_record(&dec, &save_desc_CatalogueEntry, u->meta, u->meta_len, &used, d->centry, err);
    if (r == SVR_Ok)
        r = save_decode_record(&dec, &save_desc_Game, u->game, u->game_len, &used, d->game, err);
    if (r == SVR_Ok)
        CHECK(used == u->game_len, "GAME: decoder used %u of %u bytes", (unsigned)used, (unsigned)u->game_len);
    if (r == SVR_Ok)
        r = save_decode_record(&dec, &save_desc_IntralevelData, u->ilvl, u->ilvl_len, &used, d->ilvl, err);
    CHECK(dec.stash_count == 0, "reading a file from this build dropped %u fields (first: %s)", (unsigned)dec.stash_count,
        dec.stash_count ? dec.stash[0].path : "");
    save_decoder_free(&dec);
    if (r != SVR_Ok)
        decoded_free(d);
    return r;
}

static void test_real_state(void)
{
    struct SaveBuffer file = { NULL, 0, 0 };
    struct SaveError err;
    struct Unpacked u;
    struct Decoded d;

    section("real state: struct Game round trip");
    CHECK(build_sample(&file, &err) == SVR_Ok, "building the sample failed: %s", err.message);
    if (file.data == NULL)
        return;
    printf("   sample: %u bytes\n", (unsigned)file.len);
    CHECK(unpack_sample(file.data, file.len, &u, &err) == SVR_Ok, "unpacking the sample failed: %s", err.message);
    if (u.game != NULL)
    {
        CHECK(decode_sample(&u, &d, &err) == SVR_Ok, "decoding the sample failed: %s", err.message);
        if (d.game != NULL)
        {
            struct SaveBuffer again = { NULL, 0, 0 };
            CHECK(save_encode_record(&again, &save_desc_Game, d.game, SVM_Save, &err) == SVR_Ok, "re-encoding Game failed");
            CHECK((again.len == u.game_len) && (memcmp(again.data, u.game, u.game_len) == 0),
                "Game: encode -> decode -> encode gave different bytes");
            save_buf_free(&again);
            CHECK(save_encode_record(&again, &save_desc_IntralevelData, d.ilvl, SVM_Save, &err) == SVR_Ok, "re-encoding IntralevelData failed");
            CHECK((again.len == u.ilvl_len) && (memcmp(again.data, u.ilvl, u.ilvl_len) == 0),
                "IntralevelData: encode -> decode -> encode gave different bytes");
            save_buf_free(&again);
            CHECK(save_encode_record(&again, &save_desc_CatalogueEntry, d.centry, SVM_Save, &err) == SVR_Ok, "re-encoding CatalogueEntry failed");
            CHECK((again.len == u.meta_len) && (memcmp(again.data, u.meta, u.meta_len) == 0),
                "CatalogueEntry: encode -> decode -> encode gave different bytes");
            save_buf_free(&again);
            CHECK(d.game->conf.crtr_conf.model_count == 0, "conf is runtime state and must come back empty");
            decoded_free(&d);
        }
    }
    unpacked_free(&u);
    save_buf_free(&file);
}
/******************************************************************************/
/** What the file holds, as this build reads it: raw chunk sizes and checksums (the encoding must be
 *  the same everywhere), then one checksum per field path (the state must be the same everywhere). */
static int print_digest(const char *path)
{
    FILE *f = fopen(path, "rb");
    uint8_t *data;
    long size;
    struct SaveError err;
    struct Unpacked u;
    struct Decoded d;
    struct SaveDigest *dg;
    struct SaveBuffer text = { NULL, 0, 0 };
    enum SaveResult r;
    if (f == NULL)
    {
        fprintf(stderr, "can't open %s\n", path);
        return 2;
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = (uint8_t *)malloc((size_t)size + 1);
    if ((data == NULL) || (fread(data, 1, (size_t)size, f) != (size_t)size))
    {
        fprintf(stderr, "can't read %s\n", path);
        fclose(f);
        free(data);
        return 2;
    }
    fclose(f);
    r = unpack_sample(data, (uint32_t)size, &u, &err);
    free(data);
    if (r != SVR_Ok)
    {
        fprintf(stderr, "%s\n", err.message);
        return (int)r;
    }
#define RAWLINE(name, buf, len) printf("chunk %s %u %08x\n", name, (unsigned)(len), (unsigned)crc32(0, (buf) != NULL ? (buf) : (const uint8_t *)"", (len)))
    RAWLINE("META", u.meta, u.meta_len);
    RAWLINE("SCHM", u.sch, u.sch_len);
    RAWLINE("NAMS", u.nams, u.nams_len);
    RAWLINE("CONF", u.conf, u.conf_len);
    RAWLINE("GAME", u.game, u.game_len);
    RAWLINE("ILVL", u.ilvl, u.ilvl_len);
    RAWLINE("LUA ", u.lua, u.lua_len);
    r = decode_sample(&u, &d, &err);
    if (r != SVR_Ok)
    {
        fprintf(stderr, "%s\n", err.message);
        unpacked_free(&u);
        return (int)r;
    }
    dg = save_digest_new(&d.fs);
    r = save_digest_add(dg, "CatalogueEntry", &save_desc_CatalogueEntry, d.centry, &err);
    if (r == SVR_Ok)
        r = save_digest_add(dg, "Game", &save_desc_Game, d.game, &err);
    if (r == SVR_Ok)
        r = save_digest_add(dg, "IntralevelData", &save_desc_IntralevelData, d.ilvl, &err);
    if (r == SVR_Ok)
        r = save_digest_text(dg, &text, &err);
    if (r == SVR_Ok)
        fwrite(text.data, 1, text.len, stdout);
    else
        fprintf(stderr, "%s\n", err.message);
    save_buf_free(&text);
    save_digest_free(dg);
    decoded_free(&d);
    unpacked_free(&u);
    return (r == SVR_Ok) ? 0 : (int)r;
}

/* Things with the union data of every member, written by one machine and checked on another of either byte order. */
#define ARM_THINGS 20

static void fill_arm_things(struct Thing *things)
{
    int n = 0;
    memset(things, 0, sizeof(struct Thing) * ARM_THINGS);
    things[n].class_id = TCls_Object; things[n].model = ObjMdl_GoldPot;
    things[n].valuable.gold_stored = 0x12345678; things[n].valuable.unusedparam = 0x1234; n++;
    things[n].class_id = TCls_Object; things[n].model = ObjMdl_ChickenMature;
    things[n].food.life_remaining = 0x1122; things[n].food.freshness_state = -3; things[n].food.possession_startup_timer = 9;
    things[n].food.some_chicken_was_sacrificed = 1; things[n].food.angle = 0x3344; n++;
    things[n].class_id = TCls_Object; things[n].model = ObjMdl_Torturer;
    things[n].lair.belongs_to = 0x1122; things[n].lair.cssize = 0x3344; things[n].lair.spr_size = 0x5566; n++;
    things[n].class_id = TCls_Object; things[n].model = ObjMdl_LightBall;
    things[n].armor.belongs_to = 0x2233; things[n].armor.shspeed = 7; n++;
    things[n].class_id = TCls_Object; things[n].model = ObjMdl_Disease;
    things[n].disease.belongs_to = 0x4455; things[n].disease.effect_slot = 3; n++;
    things[n].class_id = TCls_Object; things[n].model = ObjMdl_RoomFlag;
    things[n].roomflag.room_idx = 0x01020304; things[n].roomflag.last_turn_drawn = 0x05060708; things[n].roomflag.display_timer = 9; n++;
    things[n].class_id = TCls_Object; things[n].model = ObjMdl_SoulCountainer;
    things[n].heart.countdown_UNUSED = 1; things[n].heart.beat_direction = 255; n++;
    things[n].class_id = TCls_Shot; things[n].model = ShM_Fireball;
    things[n].shot.dexterity = 5; things[n].shot.damage = 0x0A0B; things[n].shot.hit_type = 2; things[n].shot.target_idx = 0x0C0D;
    things[n].shot.shot_level = 4; things[n].shot.originpos.x.val = 0x0102; things[n].shot.originpos.y.val = 0x0304;
    things[n].shot.num_wind_affected = 0x01020304; things[n].shot.wind_affected_creature[0] = 0x1234;
    things[n].shot.wind_affected_creature[CREATURES_COUNT - 1] = 0xABCD; n++;
    things[n].class_id = TCls_Shot; things[n].model = ShM_Lizard;
    things[n].shot_lizard.x = 0x01020304; things[n].shot_lizard.target_idx = 0x0708; things[n].shot_lizard.posint = 5;
    things[n].shot_lizard.range = 6; n++;
    things[n].class_id = TCls_Creature; things[n].model = 3;
    things[n].creature.gold_carried = 0x0A0B0C0D; things[n].creature.health_bar_turns = 0x0102; things[n].creature.volley_repeat = 0x0304;
    things[n].creature.volley_fire = 1; n++;
    things[n].class_id = TCls_DeadCreature; things[n].model = 3;
    things[n].corpse.exp_level = 4; things[n].corpse.laid_to_rest = 1; n++;
    things[n].class_id = TCls_Effect; things[n].model = 2;
    things[n].shot_effect.parent_class_id = 0x01020304; things[n].shot_effect.parent_model = 0x0506; things[n].shot_effect.hit_type = 3; n++;
    things[n].class_id = TCls_EffectElem; things[n].model = TngEffElm_Price;
    things[n].price_effect.number = 0x01020304; n++;
    things[n].class_id = TCls_EffectGen; things[n].model = 1;
    things[n].effect_generator.range = 0x0102; things[n].effect_generator.generation_delay = 0x03040506; n++;
    things[n].class_id = TCls_Trap; things[n].model = 1;
    things[n].trap.num_shots = 2; things[n].trap.revealed = 1; things[n].trap.rearm_turn = 0x01020304;
    things[n].trap.shooting_finished_turn = 0x05060708; things[n].trap.volley_repeat = 0x0102; things[n].trap.volley_delay = 0x0304;
    things[n].trap.firing_at = 0x0506; things[n].trap.flag_number = 9; n++;
    things[n].class_id = TCls_Door; things[n].model = 1;
    things[n].door.orientation = 0x0102; things[n].door.opening_counter = 3; things[n].door.closing_counter = 0x0405;
    things[n].door.is_locked = 1; things[n].door.revealed = 0x0607; n++;
    things[n].class_id = TCls_CaveIn; things[n].model = 1;
    things[n].cave_in.x = 1; things[n].cave_in.y = 2; things[n].cave_in.time = 0x0304; things[n].cave_in.model = 0x0506; n++;
    things[n].class_id = TCls_Object; things[n].model = ObjMdl_GoldHoard3;
    things[n].valuable.gold_stored = -5; n++;
    things[n].class_id = TCls_Empty; n++;
    things[n].class_id = TCls_AmbientSnd; n++;
}

static int write_arms(const char *path)
{
    struct Thing things[ARM_THINGS];
    struct SaveBuffer file = { NULL, 0, 0 };
    struct SaveError err;
    struct SaveReader reader;
    struct SaveChunk chunk;
    FILE *f;
    fill_arm_things(things);
    if (save_recfile_build(&file, SFK_Resync, &save_desc_Thing, things, sizeof(struct Thing), ARM_THINGS, &err) != SVR_Ok)
    {
        fprintf(stderr, "%s\n", err.message);
        return 1;
    }
    /* every member covers its data, so none of it is raw memory */
    save_reader_open(&reader, file.data, file.len, &err);
    while (save_reader_next(&reader, &chunk, &err) == SVR_Ok)
    {
        if (chunk.flags & SCF_RawBytes)
        {
            fprintf(stderr, "the things have raw memory in chunk %08x\n", (unsigned)chunk.id);
            return 1;
        }
    }
    f = fopen(path, "wb");
    if ((f == NULL) || (fwrite(file.data, 1, file.len, f) != file.len))
    {
        fprintf(stderr, "can't write %s\n", path);
        return 1;
    }
    fclose(f);
    save_buf_free(&file);
    return 0;
}

static int check_arms(const char *path)
{
    struct Thing expected[ARM_THINGS];
    uint8_t *data = NULL;
    void *records = NULL;
    uint32_t count = 0;
    struct SaveError err;
    int bad = 0;
    long size;
    FILE *f = fopen(path, "rb");
    if (f == NULL)
        return 1;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = (uint8_t *)malloc((size_t)size);
    if ((data == NULL) || (fread(data, 1, (size_t)size, f) != (size_t)size))
        return 1;
    fclose(f);
    if (save_recfile_parse(data, (uint32_t)size, SFK_Resync, &save_desc_Thing, sizeof(struct Thing), &records, &count, &err) != SVR_Ok)
    {
        fprintf(stderr, "%s\n", err.message);
        return 1;
    }
    fill_arm_things(expected);
    if (count != ARM_THINGS)
    {
        fprintf(stderr, "read %u things, wanted %d\n", (unsigned)count, ARM_THINGS);
        return 1;
    }
    for (int i = 0; i < ARM_THINGS; i++)
    {
        if (memcmp(&expected[i], (struct Thing *)records + i, sizeof(struct Thing)) != 0)
        {
            fprintf(stderr, "thing %d (class %d, model %d) read back differently\n", i, expected[i].class_id, expected[i].model);
            bad = 1;
        }
    }
    free(records);
    free(data);
    if (!bad)
        printf("%d things read back identically\n", ARM_THINGS);
    return bad;
}

static int write_sample(const char *path)
{
    struct SaveBuffer file = { NULL, 0, 0 };
    struct SaveError err;
    FILE *f;
    if (build_sample(&file, &err) != SVR_Ok)
    {
        fprintf(stderr, "%s\n", err.message);
        return 1;
    }
    f = fopen(path, "wb");
    if ((f == NULL) || (fwrite(file.data, 1, file.len, f) != file.len))
    {
        fprintf(stderr, "can't write %s\n", path);
        return 2;
    }
    fclose(f);
    save_buf_free(&file);
    return 0;
}

int main(int argc, char **argv)
{
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY); /* digest text has to be the same on every platform */
#endif
    save_register_standalone_selectors();
    if ((argc == 3) && (strcmp(argv[1], "--write-arms") == 0))
        return write_arms(argv[2]);
    if ((argc == 3) && (strcmp(argv[1], "--check-arms") == 0))
        return check_arms(argv[2]);
    if ((argc == 3) && (strcmp(argv[1], "--write-sample") == 0))
        return write_sample(argv[2]);
    if ((argc == 3) && (strcmp(argv[1], "--digest") == 0))
        return print_digest(argv[2]);
    if (argc != 1)
    {
        fprintf(stderr, "usage: kfx_save_tests [--write-sample <file> | --digest <file>]\n");
        return 64;
    }
    test_tables();
    test_conversions();
    test_names();
    test_name_tables();
    test_copy_runtime();
    test_arrays();
    test_unions();
    test_raw_bytes();
    test_selected_unions();
    test_modes();
    test_migrations();
    test_container();
    test_inspect();
    test_recfile();
    test_config_overlay_strings();
    test_real_state();
    printf("\n%d checks, %d failed\n", g_checks, g_fails);
    return (g_fails == 0) ? 0 : 1;
}
