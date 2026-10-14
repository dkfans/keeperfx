// State checksum for kfx-savetool digest. No game or SDL code.
#include "save_digest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#define DIGEST_MAX_DEPTH 16

/* One struct reached through one path. Every element of an array of structs, and every
   instance of the parent, feeds the same node, so each path ends up with a single CRC. */
struct DNode {
    const struct SaveFileStruct *fs;
    const struct SaveStructDesc *bs;
    char *path;
    int *bmatch;          /* per file field: index of the build field, -1 when the build has no such field */
    uint32_t *slot_off;   /* per file field: first slot (unions have one slot per file member) */
    uint32_t nslots;
    uint32_t *crc;
    uint8_t *seen;
    struct DNode **kid;
};

struct SaveDigest {
    const struct SaveFileSchema *schema;
    struct DNode **nodes;
    uint32_t node_count;
    uint32_t node_cap;
};

static char *dup_str(const char *s)
{
    size_t n = strlen(s) + 1;
    char *d = (char *)malloc(n);
    if (d != NULL)
        memcpy(d, s, n);
    return d;
}

static int name_matches(const struct SaveFieldDesc *bf, const char *name)
{
    if (strcmp(bf->name, name) == 0)
        return 1;
    if (bf->aliases != NULL)
        for (uint32_t i = 0; bf->aliases[i] != NULL; i++)
            if (strcmp(bf->aliases[i], name) == 0)
                return 1;
    return 0;
}

static const struct SaveFileStruct *find_struct(const struct SaveFileSchema *s, const char *name)
{
    for (uint32_t i = 0; i < s->struct_count; i++)
        if (strcmp(s->structs[i].name, name) == 0)
            return &s->structs[i];
    return NULL;
}

struct SaveDigest *save_digest_new(const struct SaveFileSchema *schema)
{
    struct SaveDigest *d = (struct SaveDigest *)calloc(1, sizeof(*d));
    if (d != NULL)
        d->schema = schema;
    return d;
}

static void node_free(struct DNode *n)
{
    free(n->path);
    free(n->bmatch);
    free(n->slot_off);
    free(n->crc);
    free(n->seen);
    free(n->kid);
    free(n);
}

void save_digest_free(struct SaveDigest *d)
{
    if (d == NULL)
        return;
    for (uint32_t i = 0; i < d->node_count; i++)
        node_free(d->nodes[i]);
    free(d->nodes);
    free(d);
}

static struct DNode *node_new(struct SaveDigest *d, const struct SaveFileStruct *fs, const struct SaveStructDesc *bs,
    const char *path)
{
    struct DNode *n = (struct DNode *)calloc(1, sizeof(*n));
    uint8_t *taken;
    if (n == NULL)
        return NULL;
    n->fs = fs;
    n->bs = bs;
    n->path = dup_str(path);
    n->bmatch = (int *)malloc((fs->field_count ? fs->field_count : 1) * sizeof(int));
    n->slot_off = (uint32_t *)malloc((fs->field_count ? fs->field_count : 1) * sizeof(uint32_t));
    taken = (uint8_t *)calloc(bs->field_count ? bs->field_count : 1, 1);
    if ((n->path == NULL) || (n->bmatch == NULL) || (n->slot_off == NULL) || (taken == NULL))
        goto fail;
    for (uint32_t i = 0; i < fs->field_count; i++)
    {
        const struct SaveFileField *ff = &fs->fields[i];
        n->bmatch[i] = -1;
        n->slot_off[i] = n->nslots;
        /* Same pairing rule as the decoder: first not-yet-taken build field with that name or alias. */
        for (uint32_t j = 0; j < bs->field_count; j++)
        {
            if (!name_matches(&bs->fields[j], ff->name) || taken[j])
                continue;
            if (save_field_included(&bs->fields[j], SVM_Save))
            {
                taken[j] = 1;
                n->bmatch[i] = (int)j;
            }
            break;
        }
        n->nslots += ((ff->stored_type == SV_UNION) && (ff->member_count > 0)) ? ff->member_count : 1;
    }
    n->crc = (uint32_t *)calloc(n->nslots ? n->nslots : 1, sizeof(uint32_t));
    n->seen = (uint8_t *)calloc(n->nslots ? n->nslots : 1, 1);
    n->kid = (struct DNode **)calloc(n->nslots ? n->nslots : 1, sizeof(struct DNode *));
    if ((n->crc == NULL) || (n->seen == NULL) || (n->kid == NULL))
        goto fail;
    if (d->node_count == d->node_cap)
    {
        uint32_t cap = d->node_cap ? d->node_cap * 2 : 64;
        struct DNode **mem = (struct DNode **)realloc((void *)d->nodes, cap * sizeof(*mem));
        if (mem == NULL)
            goto fail;
        d->nodes = mem;
        d->node_cap = cap;
    }
    d->nodes[d->node_count++] = n;
    free(taken);
    return n;
fail:
    free(taken);
    node_free(n);
    return NULL;
}

static char *join_path(const char *parent, const char *name, const char *suffix)
{
    size_t n = strlen(parent) + strlen(name) + strlen(suffix) + 2;
    char *p = (char *)malloc(n);
    if (p != NULL)
        snprintf(p, n, "%s.%s%s", parent, name, suffix);
    return p;
}

static struct DNode *kid_node(struct SaveDigest *d, struct DNode *n, uint32_t slot, const char *sub_type, const struct SaveStructDesc *bs,
    const char *path)
{
    const struct SaveFileStruct *fs;
    if (n->kid[slot] != NULL)
        return n->kid[slot];
    fs = find_struct(d->schema, sub_type);
    if (fs == NULL)
        return NULL;
    n->kid[slot] = node_new(d, fs, bs, path);
    return n->kid[slot];
}

static void crc_add(struct DNode *n, uint32_t slot, const void *bytes, uint32_t len)
{
    n->crc[slot] = (uint32_t)crc32(n->seen[slot] ? n->crc[slot] : 0, (const Bytef *)bytes, len);
    n->seen[slot] = 1;
}

static enum SaveResult visit(struct SaveDigest *d, struct DNode *n, const uint8_t *base, int depth, struct SaveError *err);

/* Walks the elements of a field the way the file lays them out; an element the build has no room for
   (the array shrank) is skipped. For each one that exists, fn gets the build element's address. */
typedef enum SaveResult (*ElemFn)(struct SaveDigest *d, struct DNode *n, const struct SaveFileField *ff,
    const struct SaveFieldDesc *bf, uint32_t slot, const uint8_t *elem, int depth, struct SaveError *err);

static enum SaveResult each_element(struct SaveDigest *d, struct DNode *n, const struct SaveFileField *ff,
    const struct SaveFieldDesc *bf, uint32_t slot, const uint8_t *base, int depth, ElemFn fn, struct SaveError *err)
{
    uint32_t file_count = 1;
    uint32_t build_count = save_field_count(bf);
    uint32_t belem = bf->mem_size / build_count;
    for (uint32_t k = 0; k < ff->dim_count; k++)
        file_count *= ff->dims[k];
    for (uint32_t lin = 0; lin < file_count; lin++)
    {
        uint32_t rest = lin;
        uint32_t idx[3] = { 0, 0, 0 };
        uint32_t blin = 0;
        int inside = 1;
        for (uint32_t k = ff->dim_count; k-- > 0;)
        {
            idx[k] = rest % ff->dims[k];
            rest /= ff->dims[k];
        }
        for (uint32_t k = 0; k < ff->dim_count; k++)
        {
            if (idx[k] >= bf->dims[k])
            {
                inside = 0;
                break;
            }
            blin = blin * bf->dims[k] + idx[k];
        }
        if (!inside)
            continue;
        enum SaveResult r = fn(d, n, ff, bf, slot, base + bf->offset + (size_t)blin * belem, depth, err);
        if (r != SVR_Ok)
            return r;
    }
    return SVR_Ok;
}

static enum SaveResult scalar_elem(struct SaveDigest *d, struct DNode *n, const struct SaveFileField *ff,
    const struct SaveFieldDesc *bf, uint32_t slot, const uint8_t *elem, int depth, struct SaveError *err)
{
    struct SaveValue v;
    uint8_t tmp[8];
    uint32_t belem = bf->mem_size / save_field_count(bf);
    (void)d;
    (void)depth;
    if (save_value_load_mem(elem, belem, bf->stored_type, &v) != 0)
        return save_fail(err, SVR_Unsupported, "%s.%s: can't read a %u byte member as %s", n->fs->name, ff->name,
            (unsigned)belem, save_stored_name(bf->stored_type));
    if (save_value_store_le(tmp, ff->stored_type, &v, 1) < 0)
        return save_fail(err, SVR_Unsupported, "%s.%s: value doesn't fit %s", n->fs->name, ff->name,
            save_stored_name(ff->stored_type));
    crc_add(n, slot, tmp, save_stored_size(ff->stored_type));
    return SVR_Ok;
}

static enum SaveResult struct_elem(struct SaveDigest *d, struct DNode *n, const struct SaveFileField *ff,
    const struct SaveFieldDesc *bf, uint32_t slot, const uint8_t *elem, int depth, struct SaveError *err)
{
    struct DNode *kid = n->kid[slot];
    (void)ff;
    (void)bf;
    if (kid == NULL)
        return save_fail(err, SVR_NoMemory, "out of memory");
    return visit(d, kid, elem, depth + 1, err);
}

static enum SaveResult visit_union(struct SaveDigest *d, struct DNode *n, uint32_t fi, const uint8_t *base, int depth,
    struct SaveError *err)
{
    const struct SaveFileField *ff = &n->fs->fields[fi];
    const struct SaveFieldDesc *bf = &n->bs->fields[n->bmatch[fi]];
    const struct SaveUnionDesc *u = bf->uni;
    const struct SaveUnionMember *bm;
    int pick = -1;
    enum SaveResult picked = save_union_pick(n->bs, bf, base, &pick, err);
    if (picked != SVR_Ok)
        return picked;
    if (pick < 0)
        return SVR_Ok;
    bm = &u->members[pick];
    for (uint32_t j = 0; j < ff->member_count; j++)
    {
        const struct SaveFileMember *fm = &ff->members[j];
        uint32_t slot = n->slot_off[fi] + j;
        if (strcmp(fm->name, bm->name) != 0)
            continue;
        if ((bm->sub != NULL) && (fm->sub_type[0] != 0))
        {
            char label[160];
            char *path;
            struct DNode *kid;
            snprintf(label, sizeof(label), "%s:%s", ff->name, fm->name);
            path = join_path(n->path, label, "");
            if (path == NULL)
                return save_fail(err, SVR_NoMemory, "out of memory");
            kid = kid_node(d, n, slot, fm->sub_type, bm->sub, path);
            free(path);
            if (kid == NULL)
                return save_fail(err, SVR_NoMemory, "out of memory");
            return visit(d, kid, base, depth + 1, err);
        }
        if ((bm->sub == NULL) && (fm->sub_type[0] == 0) && (bm->raw_size > 0) && (bm->raw_size == fm->raw_size))
            crc_add(n, slot, base + bf->offset, bm->raw_size);
        return SVR_Ok;
    }
    return SVR_Ok;
}

static enum SaveResult visit(struct SaveDigest *d, struct DNode *n, const uint8_t *base, int depth, struct SaveError *err)
{
    if (depth > DIGEST_MAX_DEPTH)
        return save_fail(err, SVR_Unsupported, "%s: structs nest too deeply", n->bs->name);
    for (uint32_t i = 0; i < n->fs->field_count; i++)
    {
        const struct SaveFileField *ff = &n->fs->fields[i];
        const struct SaveFieldDesc *bf;
        enum SaveResult r;
        if (n->bmatch[i] < 0)
            continue;
        bf = &n->bs->fields[n->bmatch[i]];
        if (((bf->stored_type == SV_STRUCT) != (ff->stored_type == SV_STRUCT)) ||
            ((bf->stored_type == SV_UNION) != (ff->stored_type == SV_UNION)) ||
            (save_field_dim_count(bf) != ff->dim_count))
            continue;
        if (ff->stored_type == SV_UNION)
        {
            r = visit_union(d, n, i, base, depth, err);
        } else if (ff->stored_type == SV_STRUCT)
        {
            uint32_t slot = n->slot_off[i];
            if (n->kid[slot] == NULL)
            {
                char *path = join_path(n->path, ff->name, ff->dim_count ? "[]" : "");
                if (path == NULL)
                    return save_fail(err, SVR_NoMemory, "out of memory");
                kid_node(d, n, slot, ff->sub_type, bf->sub, path);
                free(path);
            }
            r = each_element(d, n, ff, bf, slot, base, depth, struct_elem, err);
        } else
        {
            r = each_element(d, n, ff, bf, n->slot_off[i], base, depth, scalar_elem, err);
        }
        if (r != SVR_Ok)
            return r;
    }
    return SVR_Ok;
}

enum SaveResult save_digest_add(struct SaveDigest *d, const char *root_name, const struct SaveStructDesc *bs,
    const void *src, struct SaveError *err)
{
    const struct SaveFileStruct *fs = find_struct(d->schema, root_name);
    struct DNode *root;
    if (fs == NULL)
        return save_fail(err, SVR_Damaged, "schema has no struct %s", root_name);
    root = node_new(d, fs, bs, root_name);
    if (root == NULL)
        return save_fail(err, SVR_NoMemory, "out of memory");
    return visit(d, root, (const uint8_t *)src, 0, err);
}

struct Line {
    char *text;
};

static int cmp_line(const void *a, const void *b)
{
    return strcmp(((const struct Line *)a)->text, ((const struct Line *)b)->text);
}

enum SaveResult save_digest_text(const struct SaveDigest *d, struct SaveBuffer *out, struct SaveError *err)
{
    struct Line *lines = NULL;
    uint32_t count = 0;
    uint32_t cap = 0;
    enum SaveResult res = SVR_Ok;
    for (uint32_t ni = 0; (ni < d->node_count) && (res == SVR_Ok); ni++)
    {
        const struct DNode *n = d->nodes[ni];
        for (uint32_t i = 0; (i < n->fs->field_count) && (res == SVR_Ok); i++)
        {
            const struct SaveFileField *ff = &n->fs->fields[i];
            uint32_t nslots = ((ff->stored_type == SV_UNION) && (ff->member_count > 0)) ? ff->member_count : 1;
            for (uint32_t j = 0; j < nslots; j++)
            {
                uint32_t slot = n->slot_off[i] + j;
                char label[160];
                size_t len;
                if (!n->seen[slot])
                    continue;
                if (ff->stored_type == SV_UNION)
                    snprintf(label, sizeof(label), "%s:%s", ff->name, ff->members[j].name);
                else
                    snprintf(label, sizeof(label), "%s%s", ff->name, ff->dim_count ? "[]" : "");
                len = strlen(n->path) + strlen(label) + 16;
                if (count == cap)
                {
                    uint32_t ncap = cap ? cap * 2 : 1024;
                    struct Line *mem = (struct Line *)realloc((void *)lines, ncap * sizeof(*mem));
                    if (mem == NULL)
                    {
                        res = save_fail(err, SVR_NoMemory, "out of memory");
                        break;
                    }
                    lines = mem;
                    cap = ncap;
                }
                lines[count].text = (char *)malloc(len);
                if (lines[count].text == NULL)
                {
                    res = save_fail(err, SVR_NoMemory, "out of memory");
                    break;
                }
                snprintf(lines[count].text, len, "%s.%s  %08x", n->path, label, (unsigned)n->crc[slot]);
                count++;
            }
        }
    }
    if (res == SVR_Ok)
    {
        qsort(lines, count, sizeof(*lines), cmp_line);
        for (uint32_t i = 0; (i < count) && (res == SVR_Ok); i++)
            if ((save_buf_append(out, lines[i].text, (uint32_t)strlen(lines[i].text)) != 0) || (save_buf_u8(out, '\n') != 0))
                res = save_fail(err, SVR_NoMemory, "out of memory");
    }
    for (uint32_t i = 0; i < count; i++)
        free(lines[i].text);
    free(lines);
    return res;
}
