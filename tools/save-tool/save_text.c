// Schema as text, for kfx-savetool schema (and the CI schema diff). Not part of the game.
#include "save_text.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *dup_str(const char *s)
{
    size_t n = strlen(s) + 1;
    char *d = (char *)malloc(n);
    if (d != NULL)
        memcpy(d, s, n);
    return d;
}
/******************************************************************************/
struct Lines {
    char **v;
    uint32_t n;
    uint32_t cap;
};

static int lines_add(struct Lines *l, const char *s)
{
    if (l->n == l->cap)
    {
        uint32_t cap = l->cap ? l->cap * 2 : 64;
        char **mem = (char **)realloc((void *)l->v, cap * sizeof(char *));
        if (mem == NULL)
            return -1;
        l->v = mem;
        l->cap = cap;
    }
    l->v[l->n] = dup_str(s);
    if (l->v[l->n] == NULL)
        return -1;
    l->n++;
    return 0;
}

static int cmp_line(const void *a, const void *b)
{
    return strcmp(*(char *const *)a, *(char *const *)b);
}

static enum SaveResult lines_flush(struct Lines *l, struct SaveBuffer *out, struct SaveError *err)
{
    int rc = 0;
    if (l->n > 1)
        qsort((void *)l->v, l->n, sizeof(char *), cmp_line);
    for (uint32_t i = 0; i < l->n; i++)
    {
        rc |= save_buf_append(out, l->v[i], (uint32_t)strlen(l->v[i]));
        rc |= save_buf_u8(out, '\n');
        free(l->v[i]);
    }
    free((void *)l->v);
    return (rc != 0) ? save_fail(err, SVR_NoMemory, "out of memory") : SVR_Ok;
}

static void fmt_type(char *dst, size_t n, uint8_t type, const char *sub)
{
    if (type == SV_STRUCT)
        snprintf(dst, n, "struct:%s", sub ? sub : "?");
    else
        snprintf(dst, n, "%s", save_stored_name(type));
}

static void fmt_dims(char *dst, size_t n, const uint32_t *dims, uint32_t nd)
{
    size_t used = 0;
    dst[0] = 0;
    if (nd == 0)
    {
        snprintf(dst, n, "-");
        return;
    }
    for (uint32_t i = 0; i < nd && used < n; i++)
        used += (size_t)snprintf(dst + used, n - used, "[%u]", (unsigned)dims[i]);
}

static void fmt_flags(char *dst, size_t n, uint8_t flags)
{
    static const struct { uint8_t bit; const char *name; } names[] = {
        { SVF_SAVE, "save" }, { SVF_SYNC, "sync" }, { SVF_CLAMP, "clamp" } };
    size_t used = 0;
    dst[0] = 0;
    for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++)
    {
        if ((flags & names[i].bit) && (used < n))
            used += (size_t)snprintf(dst + used, n - used, "%s%s", used ? "," : "", names[i].name);
    }
}

enum SaveResult save_schema_text_build(struct SaveBuffer *out, const struct SaveStructDesc *const *roots,
    uint32_t root_count, enum SaveMode mode, struct SaveError *err)
{
    struct SaveStructList c;
    struct Lines l = { NULL, 0, 0 };
    enum SaveResult r = save_schema_collect(&c, roots, root_count, mode, err);
    if (r != SVR_Ok)
        return r;
    for (uint32_t i = 0; i < c.count; i++)
    {
        for (uint32_t j = 0; j < c.structs[i]->field_count; j++)
        {
            const struct SaveFieldDesc *f = &c.structs[i]->fields[j];
            char type[96], dims[64], flags[16], line[600];
            if (!save_field_included(f, mode))
                continue;
            fmt_type(type, sizeof(type), f->stored_type, f->sub ? f->sub->name : NULL);
            fmt_dims(dims, sizeof(dims), f->dims, save_field_dim_count(f));
            fmt_flags(flags, sizeof(flags), save_field_flags(f));
            int len = snprintf(line, sizeof(line), "%s.%s %s %s %s", c.structs[i]->name, f->name, type, dims, flags);
            if ((f->aliases != NULL) && (f->aliases[0] != NULL))
            {
                len += snprintf(line + len, sizeof(line) - (size_t)len, " aliases=");
                for (uint32_t a = 0; f->aliases[a] != NULL; a++)
                    len += snprintf(line + len, sizeof(line) - (size_t)len, "%s%s", a ? "," : "", f->aliases[a]);
            }
            if (f->stored_type == SV_UNION)
                snprintf(line + len, sizeof(line) - (size_t)len, " disc=%s", (f->uni->selector != NULL) ? f->uni->selector : f->uni->disc);
            if (lines_add(&l, line) != 0)
            {
                free((void *)c.structs);
                return save_fail(err, SVR_NoMemory, "out of memory");
            }
            for (uint32_t k = 0; (f->stored_type == SV_UNION) && (k < f->uni->member_count); k++)
            {
                const struct SaveUnionMember *m = &f->uni->members[k];
                char what[96], vals[128];
                size_t vl = 0;
                if (m->sub != NULL)
                    snprintf(what, sizeof(what), "struct:%s", m->sub->name);
                else
                    snprintf(what, sizeof(what), m->raw_size ? "raw:%u" : "none", (unsigned)m->raw_size);
                vals[0] = 0;
                for (uint32_t v = 0; v < m->nvalues; v++)
                    vl += (size_t)snprintf(vals + vl, sizeof(vals) - vl, "%s%d", v ? "," : "", (int)m->values[v]);
                snprintf(line, sizeof(line), "%s.%s::%s %s%s%s%s", c.structs[i]->name, f->name, m->name, what,
                    m->nvalues ? " values=" : "", vals, m->is_default ? " default" : "");
                if (lines_add(&l, line) != 0)
                {
                    free((void *)c.structs);
                    return save_fail(err, SVR_NoMemory, "out of memory");
                }
            }
        }
    }
    free((void *)c.structs);
    return lines_flush(&l, out, err);
}
