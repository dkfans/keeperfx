// Checks that a field table is consistent with itself: sizes, offsets, sub-structs, unions. The
// tables are constants, so this runs in the tests and in kfx-savetool, not in the game.
#include "save_validate.h"

#include <string.h>

#define SAVE_MAX_DEPTH 16

static enum SaveResult validate(const struct SaveStructDesc *d, struct SaveError *err, int depth);

static int is_integer_type(uint8_t t)
{
    return ((t >= SV_U8) && (t <= SV_I64)) || (t == SV_BOOL8);
}

static enum SaveResult validate_union(const struct SaveStructDesc *d, const struct SaveFieldDesc *f,
    struct SaveError *err, int depth)
{
    const struct SaveUnionDesc *u = f->uni;
    const struct SaveFieldDesc *df = NULL;
    int defaults = 0;
    if ((u == NULL) || ((u->disc == NULL) && (u->selector == NULL)) || (u->member_count == 0) || (u->member_count > SAVE_UNION_MAX_MEMBERS))
        return save_fail(err, SVR_Unsupported, "%s.%s: the union has no members", d->name, f->name);
    if (u->selector == NULL)
    {
        for (uint32_t i = 0; i < d->field_count; i++)
            if (strcmp(d->fields[i].name, u->disc) == 0)
                df = &d->fields[i];
        if ((df == NULL) || !is_integer_type(df->stored_type) || (save_field_count(df) != 1) ||
            (!save_field_included(df, SVM_Save) && !save_field_included(df, SVM_Sync)))
            return save_fail(err, SVR_Unsupported, "%s.%s: %s isn't a stored integer field", d->name, f->name, u->disc);
    }
    for (uint32_t i = 0; i < u->member_count; i++)
    {
        const struct SaveUnionMember *m = &u->members[i];
        if ((m->name == NULL) || (m->nvalues > SAVE_UNION_MAX_VALUES))
            return save_fail(err, SVR_Unsupported, "%s.%s: bad union member %u", d->name, f->name, (unsigned)i);
        defaults += m->is_default;
        for (uint32_t j = 0; j < i; j++)
        {
            if (strcmp(u->members[j].name, m->name) == 0)
                return save_fail(err, SVR_Unsupported, "%s.%s: member %s listed twice", d->name, f->name, m->name);
            for (uint32_t a = 0; a < m->nvalues; a++)
                for (uint32_t b = 0; b < u->members[j].nvalues; b++)
                    if (m->values[a] == u->members[j].values[b])
                        return save_fail(err, SVR_Unsupported, "%s.%s: value %d selects two members", d->name, f->name,
                            (int)m->values[a]);
        }
        if (m->sub != NULL)
        {
            if (m->sub->size != d->size)
                return save_fail(err, SVR_Unsupported, "%s.%s: member %s must be a table of %s", d->name, f->name,
                    m->name, d->name);
            enum SaveResult r = validate(m->sub, err, depth + 1);
            if (r != SVR_Ok)
                return r;
            for (uint32_t j = 0; j < m->sub->field_count; j++)
            {
                const struct SaveFieldDesc *g = &m->sub->fields[j];
                if (!save_field_included(g, SVM_Save) && !save_field_included(g, SVM_Sync))
                    continue;
                if ((g->offset < f->offset) || (g->offset + g->mem_size > f->offset + f->mem_size))
                    return save_fail(err, SVR_Unsupported, "%s.%s: member %s's field %s falls outside the union",
                        d->name, f->name, m->name, g->name);
            }
        } else if ((m->raw_size > f->mem_size) || (f->offset + m->raw_size > d->size))
        {
            return save_fail(err, SVR_Unsupported, "%s.%s: member %s is larger than the union", d->name, f->name, m->name);
        }
    }
    if (defaults > 1)
        return save_fail(err, SVR_Unsupported, "%s.%s: more than one default member", d->name, f->name);
    return SVR_Ok;
}

static enum SaveResult validate(const struct SaveStructDesc *d, struct SaveError *err, int depth)
{
    if (depth > SAVE_MAX_DEPTH)
        return save_fail(err, SVR_Unsupported, "%s: structs nest too deeply", d->name);
    for (uint32_t i = 0; i < d->field_count; i++)
    {
        const struct SaveFieldDesc *f = &d->fields[i];
        if (f->name == NULL)
            return save_fail(err, SVR_Unsupported, "%s: field %u has no name", d->name, (unsigned)i);
        for (uint32_t j = 0; j < i; j++)
            if (strcmp(d->fields[j].name, f->name) == 0)
                return save_fail(err, SVR_Unsupported, "%s.%s: listed twice", d->name, f->name);
        uint32_t count = save_field_count(f);
        if ((f->mem_size == 0) || (f->mem_size % count != 0) || (f->offset > d->size) || (f->mem_size > d->size - f->offset))
            return save_fail(err, SVR_Unsupported, "%s.%s: size or offset doesn't fit the struct", d->name, f->name);
        if (!save_field_included(f, SVM_Save) && !save_field_included(f, SVM_Sync))
            continue;
        uint32_t elem = f->mem_size / count;
        switch (f->stored_type)
        {
        case SV_STRUCT:
            if ((f->sub == NULL) || (elem != f->sub->size))
                return save_fail(err, SVR_Unsupported, "%s.%s: element size differs from struct %s", d->name, f->name,
                    (f->sub != NULL) ? f->sub->name : "?");
            {
                enum SaveResult r = validate(f->sub, err, depth + 1);
                if (r != SVR_Ok)
                    return r;
            }
            break;
        case SV_F32: case SV_F64:
            if ((elem != 4) && (elem != 8))
                return save_fail(err, SVR_Unsupported, "%s.%s: float must be 4 or 8 bytes", d->name, f->name);
            break;
        case SV_STR:
            if (elem != 1)
                return save_fail(err, SVR_Unsupported, "%s.%s: string must be a char array", d->name, f->name);
            break;
        case SV_UNION:
            {
                enum SaveResult r = validate_union(d, f, err, depth);
                if (r != SVR_Ok)
                    return r;
            }
            break;
        case SV_INVALID:
            return save_fail(err, SVR_Unsupported, "%s.%s: no stored type", d->name, f->name);
        default:
            if ((elem != 1) && (elem != 2) && (elem != 4) && (elem != 8))
                return save_fail(err, SVR_Unsupported, "%s.%s: integer must be 1, 2, 4 or 8 bytes", d->name, f->name);
            break;
        }
    }
    return SVR_Ok;
}

enum SaveResult save_desc_validate(const struct SaveStructDesc *d, struct SaveError *err)
{
    return validate(d, err, 0);
}
