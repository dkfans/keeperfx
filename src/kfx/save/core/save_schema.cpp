/******************************************************************************/
/** @file save_schema.cpp
 *     Field table helpers: sizes, flags, and the list of structs reachable from a set of roots.
 *     definition of the schema as structs also prevents fuckup when the field tables are changed and the schema is not updated.
 *     You'll see it a mile away.
 */
/******************************************************************************/
#include "save_schema.h"
#include "save_internal.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <vector>

/******************************************************************************/
bool save_fast_copy_ok(uint8_t type, uint32_t elem)
{
    return save_host_is_little_endian() && (type != SV_BOOL8) && (type != SV_STRUCT) && (elem == save_stored_size(type));
}

namespace {
struct SelectorEntry {
    const char *name;
    SaveUnionSelector selector;
};

std::vector<SelectorEntry> &selector_registry()
{
    static std::vector<SelectorEntry> registry;
    return registry;
}
} // namespace

int save_register_union_selector(const char *name, SaveUnionSelector selector)
{
    std::vector<SelectorEntry> &registry = selector_registry();
    const auto existing = std::find_if(registry.begin(), registry.end(),
        [name](const SelectorEntry &entry) { return strcmp(entry.name, name) == 0; });
    if (existing != registry.end())
        existing->selector = selector;
    else
        registry.push_back({ name, selector });
    return 0;
}

enum SaveResult save_union_pick(const struct SaveStructDesc *desc, const struct SaveFieldDesc *field, const void *owner,
    int *pick, struct SaveError *err)
{
    const SaveUnionDesc *union_desc = field->uni;
    int64_t value = INT64_MIN;      /* a value no member lists */
    if (union_desc->selector != nullptr)
    {
        const std::vector<SelectorEntry> &registry = selector_registry();
        const auto found = std::find_if(registry.begin(), registry.end(),
            [union_desc](const SelectorEntry &entry) { return strcmp(entry.name, union_desc->selector) == 0; });
        if (found != registry.end())
            value = found->selector(owner);
    } else
    {
        const SaveFieldDesc *disc_field = nullptr;
        SaveValue discriminator;
        for (uint32_t field_index = 0; field_index < desc->field_count; field_index++)
            if (strcmp(desc->fields[field_index].name, union_desc->disc) == 0)
                disc_field = &desc->fields[field_index];
        if ((disc_field == nullptr) || (save_value_load_mem(static_cast<const uint8_t *>(owner) + disc_field->offset, disc_field->mem_size,
                disc_field->stored_type, &discriminator) != 0))
            return save_fail(err, SVR_Unsupported, "%s.%s: can't read %s", desc->name, field->name, union_desc->disc);
        value = discriminator.neg ? -static_cast<int64_t>(discriminator.mag) : static_cast<int64_t>(discriminator.mag);
    }
    *pick = -1;
    for (uint32_t member_index = 0; member_index < union_desc->member_count; member_index++)
    {
        const SaveUnionMember *member = &union_desc->members[member_index];
        if (member->is_default && (*pick < 0))
            *pick = static_cast<int>(member_index);
        for (uint32_t value_index = 0; value_index < member->nvalues; value_index++)
            if (member->values[value_index] == value)
                *pick = static_cast<int>(member_index);
    }
    return SVR_Ok;
}

uint32_t save_struct_extent(const struct SaveStructDesc *desc)
{
    uint32_t extent = 0;
    for (uint32_t field_index = 0; field_index < desc->field_count; field_index++)
    {
        const uint32_t end = desc->fields[field_index].offset + desc->fields[field_index].mem_size;
        extent = std::max(end, extent);
    }
    return extent;
}

uint32_t save_stored_size(uint8_t type)
{
    switch (type)
    {
    case SV_U8: case SV_I8: case SV_BOOL8: case SV_STR: return 1;
    case SV_U16: case SV_I16: return 2;
    case SV_U32: case SV_I32: case SV_F32: return 4;
    case SV_U64: case SV_I64: case SV_F64: return 8;
    default: return 0;
    }
}

const char *save_stored_name(uint8_t type)
{
    static const char *const names[] = { "invalid", "u8", "i8", "u16", "i16", "u32", "i32", "u64", "i64",
        "f32", "f64", "bool8", "str", "struct", "union" };
    return (type <= SV_UNION) ? names[type] : "invalid";
}

uint32_t save_field_dim_count(const struct SaveFieldDesc *field)
{
    uint32_t count = 0;
    while ((count < SAVE_MAX_DIMS) && (field->dims[count] != 0))
        count++;
    return count;
}

uint32_t save_field_count(const struct SaveFieldDesc *field)
{
    uint32_t count = 1;
    for (uint32_t index = 0; (index < SAVE_MAX_DIMS) && (field->dims[index] != 0); index++)
        count *= field->dims[index];
    return count;
}

uint8_t save_field_flags(const struct SaveFieldDesc *field)
{
    if ((field->flags & (SVF_SAVE | SVF_SYNC | SVF_DERIVED | SVF_RUNTIME)) == 0)
        return static_cast<uint8_t>(field->flags | SVF_SAVE | SVF_SYNC);
    return field->flags;
}

bool save_field_included(const struct SaveFieldDesc *field, enum SaveMode mode)
{
    const uint8_t flags = save_field_flags(field);
    if (flags & (SVF_DERIVED | SVF_RUNTIME))
        return false;
    return (flags & ((mode == SVM_Save) ? SVF_SAVE : SVF_SYNC)) != 0;
}

void save_copy_runtime(const struct SaveStructDesc *desc, void *dst, const void *src)
{
    for (uint32_t field_index = 0; field_index < desc->field_count; field_index++)
    {
        const struct SaveFieldDesc *field = &desc->fields[field_index];
        if (field->flags & SVF_RUNTIME)
        {
            memcpy(static_cast<uint8_t *>(dst) + field->offset, static_cast<const uint8_t *>(src) + field->offset, field->mem_size);
        } else if ((field->stored_type == SV_STRUCT) && (field->sub != nullptr))
        {
            const uint32_t count = save_field_count(field);
            const uint32_t elem = field->mem_size / count;
            for (uint32_t k = 0; k < count; k++)
                save_copy_runtime(field->sub, static_cast<uint8_t *>(dst) + field->offset + (static_cast<size_t>(k) * elem),
                    static_cast<const uint8_t *>(src) + field->offset + (static_cast<size_t>(k) * elem));
        }
    }
}

/******************************************************************************/
static int cmp_desc(const void *left, const void *right)
{
    return strcmp((*static_cast<const struct SaveStructDesc *const *>(left))->name, (*static_cast<const struct SaveStructDesc *const *>(right))->name);
}

static enum SaveResult collect(struct SaveStructList *list, const struct SaveStructDesc *desc, enum SaveMode mode, struct SaveError *err, int depth)
{
    if (depth > SAVE_MAX_DEPTH)
        return save_fail(err, SVR_Unsupported, "%s: structs nest too deeply", desc->name);
    for (uint32_t index = 0; index < list->count; index++)
        if (list->structs[index] == desc)
            return SVR_Ok;
    if (list->count == list->capacity)
    {
        const uint32_t capacity = list->capacity ? list->capacity * 2 : 16;
        const auto **grown = static_cast<const struct SaveStructDesc **>(realloc(static_cast<void *>(list->structs), capacity * sizeof(const SaveStructDesc *)));
        if (grown == nullptr)
            return save_fail(err, SVR_NoMemory, "out of memory");
        list->structs = grown;
        list->capacity = capacity;
    }
    list->structs[list->count++] = desc;
    for (uint32_t field_index = 0; field_index < desc->field_count; field_index++)
    {
        const struct SaveFieldDesc *field = &desc->fields[field_index];
        if (save_field_included(field, mode) && (field->stored_type == SV_STRUCT) && (field->sub != nullptr))
        {
            const enum SaveResult result = collect(list, field->sub, mode, err, depth + 1);
            if (result != SVR_Ok)
                return result;
        }
        if (save_field_included(field, mode) && (field->stored_type == SV_UNION) && (field->uni != nullptr))
        {
            for (uint32_t member_index = 0; member_index < field->uni->member_count; member_index++)
            {
                if (field->uni->members[member_index].sub == nullptr)
                    continue;
                const enum SaveResult result = collect(list, field->uni->members[member_index].sub, mode, err, depth + 1);
                if (result != SVR_Ok)
                    return result;
            }
        }
    }
    return SVR_Ok;
}

enum SaveResult save_schema_collect(struct SaveStructList *list, const struct SaveStructDesc *const *roots, uint32_t count,
    enum SaveMode mode, struct SaveError *err)
{
    memset(list, 0, sizeof(*list));
    for (uint32_t index = 0; index < count; index++)
    {
        const enum SaveResult result = collect(list, roots[index], mode, err, 0);
        if (result != SVR_Ok)
        {
            free(static_cast<void *>(list->structs));
            return result;
        }
    }
    if (list->count > 1)
        qsort(static_cast<void *>(list->structs), list->count, sizeof(*list->structs), cmp_desc);
    for (uint32_t index = 1; index < list->count; index++)
        if (strcmp(list->structs[index - 1]->name, list->structs[index]->name) == 0)
        {
            const enum SaveResult result = save_fail(err, SVR_Unsupported, "two different structs are named %s", list->structs[index]->name);
            free(static_cast<void *>(list->structs));
            return result;
        }
    return SVR_Ok;
}
