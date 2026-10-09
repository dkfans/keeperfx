/******************************************************************************/
/** @file save_decode.cpp
 *     Decodes a record into a struct, matching the file's schema to the field table by name.
 */
/******************************************************************************/
#include "save_schema.h"
#include "save_internal.h"
#include "save_bytes.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>
#include <optional>
#include <variant>
#include <vector>

using savebytes::ByteReader;
/******************************************************************************/
struct Plan;

/** What the file holds for a field. */
struct FileShape {
    const SaveFileField *field = nullptr;
    uint32_t count = 0;                             /* elements in all dimensions */
    uint32_t elem_bytes = 0;                        /* stored bytes of a scalar element */
    std::array<uint32_t, SAVE_MAX_DIMS> dims{};
    uint32_t dim_count = 0;
    uint32_t struct_index = 0;                      /* the field's struct in the file, for struct fields */
};

/** How this build lays out the same field. */
struct BuildShape {
    const SaveFieldDesc *field = nullptr;
    uint32_t count = 0;
    uint32_t elem = 0;                              /* bytes of one element in memory */
    std::array<uint32_t, SAVE_MAX_DIMS> dims{};
    bool same_dims = false;                         /* the file's dimensions are this build's */
};

/** The file has the field and this build doesn't: step over it, and keep it for the migrations. */
struct SkipOp {
    FileShape file;
};

/** This build has the field and the file doesn't: give it its default value. */
struct DefaultOp {
    const SaveFieldDesc *field;
    uint32_t count;
    uint32_t elem;
};

/** Both have a scalar or string array: read it and convert it to this build's type. */
struct CopyOp {
    FileShape file;
    BuildShape build;
    bool block;                                     /* the file's bytes are this build's bytes: one copy */
};

/** Both have an array of structs, each read with a plan of its own. */
struct SubOp {
    FileShape file;
    BuildShape build;
    Plan *plan;
};

/** Both have a union: read the member the file used, if this build still has it. */
struct UnionOp {
    const SaveFileField *file_field;
    const SaveFieldDesc *build_field;
    std::vector<const SaveUnionMember *> build_members;    /* per file member: this build's member, or null */
    std::vector<Plan *> plans;                              /* per file member: the plan for a struct member */
};

/** One step of a plan: what to do with one field of the file, or of this build. */
using PlanOp = std::variant<SkipOp, DefaultOp, CopyOp, SubOp, UnionOp>;

/** How to read one struct of the file into one struct of this build: one op per field, made once per
 *  (file struct, build struct) pair and used for every record of it. */
struct Plan {
    const SaveFileStruct *file_struct;
    const SaveStructDesc *build_struct;
    std::vector<PlanOp> ops;
};

struct DecoderState {
    std::vector<std::unique_ptr<Plan>> plans;
};

void save_decoder_init(struct SaveDecoder *decoder, const struct SaveFileSchema *schema, enum SaveMode mode)
{
    memset(decoder, 0, sizeof(*decoder));
    decoder->schema = schema;
    decoder->mode = mode;
    decoder->priv = new (std::nothrow) DecoderState();
}

void save_decoder_set_chunk_flags(struct SaveDecoder *decoder, uint32_t chunk_flags)
{
    const bool written_big_endian = (chunk_flags & SCF_RawBigEndian) != 0;
    decoder->foreign_raw_bytes = ((chunk_flags & SCF_RawBytes) != 0) && (written_big_endian == (save_host_is_little_endian() != 0));
}

void save_decoder_free(struct SaveDecoder *decoder)
{
    delete static_cast<DecoderState *>(decoder->priv);
    for (uint32_t stash_index = 0; stash_index < decoder->stash_count; stash_index++)
    {
        free(decoder->stash[stash_index].path);
        free(decoder->stash[stash_index].bytes);
    }
    free(decoder->stash);
    memset(decoder, 0, sizeof(*decoder));
}

static std::optional<uint32_t> find_file_struct(const SaveFileSchema *schema, const char *name)
{
    for (uint32_t struct_index = 0; struct_index < schema->struct_count; struct_index++)
        if (strcmp(schema->structs[struct_index].name, name) == 0)
            return struct_index;
    return std::nullopt;
}

static bool alias_matches(const SaveFieldDesc *build_field, const char *name)
{
    if (strcmp(build_field->name, name) == 0)
        return true;
    if (build_field->aliases == nullptr)
        return false;
    for (const char *const *alias = build_field->aliases; *alias != nullptr; alias++)
        if (strcmp(*alias, name) == 0)
            return true;
    return false;
}

static bool bytes_zero(const uint8_t *bytes, uint32_t count)
{
    for (uint32_t index = 0; index < count; index++)
        if (bytes[index] != 0)
            return false;
    return true;
}
/******************************************************************************/
static enum SaveResult build_plan(SaveDecoder *decoder, uint32_t file_struct_index, const SaveStructDesc *build_struct,
    Plan **out, SaveError *err, int depth);

static enum SaveResult describe_file_field(const SaveDecoder *decoder, const SaveFileField *file_field, FileShape &shape,
    SaveError *err)
{
    shape.field = file_field;
    shape.dim_count = file_field->dim_count;
    shape.count = 1;
    for (uint32_t dim_index = 0; dim_index < file_field->dim_count; dim_index++)
    {
        shape.dims[dim_index] = file_field->dims[dim_index];
        shape.count *= file_field->dims[dim_index];
    }
    if (file_field->stored_type == SV_STRUCT)
    {
        const std::optional<uint32_t> struct_index = find_file_struct(decoder->schema, file_field->sub_type);
        if (!struct_index)
            return save_fail(err, SVR_Damaged, "schema has no struct %s", file_field->sub_type);
        shape.struct_index = *struct_index;
    } else if (file_field->stored_type != SV_UNION)
    {
        shape.elem_bytes = save_stored_size(file_field->stored_type);
    }
    return SVR_Ok;
}

static BuildShape describe_build_field(const SaveFieldDesc *build_field, const FileShape &file)
{
    BuildShape shape;
    shape.field = build_field;
    shape.count = save_field_count(build_field);
    shape.elem = build_field->mem_size / shape.count;
    shape.same_dims = true;
    for (uint32_t dim_index = 0; dim_index < file.dim_count; dim_index++)
    {
        shape.dims[dim_index] = build_field->dims[dim_index];
        if (build_field->dims[dim_index] != file.dims[dim_index])
            shape.same_dims = false;
    }
    return shape;
}

/** Which of the file's union members this build still has, and the plan to read each one that is a struct. */
static enum SaveResult build_union_op(SaveDecoder *decoder, const SaveFileField *file_field, const SaveFieldDesc *build_field,
    UnionOp &op, SaveError *err, int depth)
{
    const SaveUnionDesc *union_desc = build_field->uni;
    op.file_field = file_field;
    op.build_field = build_field;
    op.build_members.assign(file_field->member_count, nullptr);
    op.plans.assign(file_field->member_count, nullptr);
    for (uint32_t file_member_index = 0; file_member_index < file_field->member_count; file_member_index++)
    {
        const SaveFileMember *file_member = &file_field->members[file_member_index];
        const SaveUnionMember *build_member = nullptr;
        for (uint32_t build_member_index = 0; build_member_index < union_desc->member_count; build_member_index++)
            if (strcmp(union_desc->members[build_member_index].name, file_member->name) == 0)
                build_member = &union_desc->members[build_member_index];
        if ((build_member == nullptr) || ((file_member->sub_type[0] != 0) != (build_member->sub != nullptr)))
            continue;
        if (build_member->sub != nullptr)
        {
            const std::optional<uint32_t> struct_index = find_file_struct(decoder->schema, file_member->sub_type);
            if (!struct_index)
                return save_fail(err, SVR_Damaged, "schema has no struct %s", file_member->sub_type);
            const enum SaveResult result = build_plan(decoder, *struct_index, build_member->sub, &op.plans[file_member_index], err, depth + 1);
            if (result != SVR_Ok)
                return result;
        }
        op.build_members[file_member_index] = build_member;
    }
    return SVR_Ok;
}

/** The op for a field the file and this build both have. struct_name is only for messages. */
static enum SaveResult make_field_op(SaveDecoder *decoder, const SaveFileField *file_field, const SaveFieldDesc *build_field,
    const char *struct_name, PlanOp &op, SaveError *err, int depth)
{
    FileShape file;
    enum SaveResult result = describe_file_field(decoder, file_field, file, err);
    if (result != SVR_Ok)
        return result;
    if (((file_field->stored_type == SV_STRUCT) != (build_field->stored_type == SV_STRUCT)) ||
        ((file_field->stored_type == SV_UNION) != (build_field->stored_type == SV_UNION)))
        return save_fail(err, SVR_Unsupported, "%s.%s: was %s in the file, is %s now", struct_name, file_field->name,
            save_stored_name(file_field->stored_type), save_stored_name(build_field->stored_type));
    if (save_field_dim_count(build_field) != file_field->dim_count)
        return save_fail(err, SVR_Unsupported, "%s.%s: number of array dimensions changed", struct_name, file_field->name);

    if (file_field->stored_type == SV_UNION)
    {
        UnionOp union_op;
        result = build_union_op(decoder, file_field, build_field, union_op, err, depth);
        op = std::move(union_op);
        return result;
    }
    const BuildShape build = describe_build_field(build_field, file);
    if (file_field->stored_type == SV_STRUCT)
    {
        Plan *plan = nullptr;
        result = build_plan(decoder, file.struct_index, build_field->sub, &plan, err, depth + 1);
        op = SubOp{ file, build, plan };
        return result;
    }
    const bool block = build.same_dims && (file_field->stored_type == build_field->stored_type) &&
        save_fast_copy_ok(build_field->stored_type, build.elem);
    op = CopyOp{ file, build, block };
    return SVR_Ok;
}

/** One op per field of the file (read it, or skip it when this build doesn't have it), then a default op
 *  for each field of this build that the file lacks. */
static enum SaveResult fill_plan(SaveDecoder *decoder, Plan &plan, SaveError *err, int depth)
{
    const SaveFileStruct *file_struct = plan.file_struct;
    const SaveStructDesc *build_struct = plan.build_struct;
    std::vector<bool> matched(build_struct->field_count, false);

    for (uint32_t file_field_index = 0; file_field_index < file_struct->field_count; file_field_index++)
    {
        const SaveFileField *file_field = &file_struct->fields[file_field_index];
        std::optional<uint32_t> matched_index;
        for (uint32_t build_field_index = 0; build_field_index < build_struct->field_count; build_field_index++)
        {
            const SaveFieldDesc *build_field = &build_struct->fields[build_field_index];
            if (!alias_matches(build_field, file_field->name) || matched[build_field_index])
                continue;
            if (save_field_included(build_field, decoder->mode))
                matched_index = build_field_index;
            break;
        }
        PlanOp &op = plan.ops.emplace_back();
        enum SaveResult result;
        if (!matched_index)
        {
            SkipOp skip;
            result = describe_file_field(decoder, file_field, skip.file, err);
            op = skip;
        } else
        {
            matched[*matched_index] = true;
            result = make_field_op(decoder, file_field, &build_struct->fields[*matched_index], file_struct->name, op, err, depth);
        }
        if (result != SVR_Ok)
            return result;
    }
    for (uint32_t build_field_index = 0; build_field_index < build_struct->field_count; build_field_index++)
    {
        const SaveFieldDesc *build_field = &build_struct->fields[build_field_index];
        if (matched[build_field_index] || !save_field_included(build_field, decoder->mode) ||
            (build_field->stored_type == SV_STRUCT) || (build_field->stored_type == SV_STR) || (build_field->default_value == 0))
            continue;
        const uint32_t count = save_field_count(build_field);
        plan.ops.emplace_back(DefaultOp{ build_field, count, build_field->mem_size / count });
    }
    return SVR_Ok;
}

static enum SaveResult build_plan(SaveDecoder *decoder, uint32_t file_struct_index, const SaveStructDesc *build_struct,
    Plan **out, SaveError *err, int depth)
{
    auto *state = static_cast<DecoderState *>(decoder->priv);
    const SaveFileStruct *file_struct = &decoder->schema->structs[file_struct_index];
    if (depth > SAVE_MAX_DEPTH)
        return save_fail(err, SVR_Unsupported, "%s: structs nest too deeply", build_struct->name);
    const auto existing = std::find_if(state->plans.begin(), state->plans.end(), [&](const std::unique_ptr<Plan> &candidate) {
        return (candidate->file_struct == file_struct) && (candidate->build_struct == build_struct);
    });
    if (existing != state->plans.end())
    {
        *out = existing->get();
        return SVR_Ok;
    }
    // Registered before it is filled, so a struct that contains itself finds its own plan.
    state->plans.push_back(std::make_unique<Plan>(Plan{ file_struct, build_struct, {} }));
    Plan &plan = *state->plans.back();
    *out = &plan;
    return fill_plan(decoder, plan, err, depth);
}
/******************************************************************************/
static enum SaveResult skip_struct(SaveDecoder *decoder, uint32_t struct_index, ByteReader &reader, SaveError *err, int depth);

/** Steps over a field of the file without reading it. */
static enum SaveResult skip_field(SaveDecoder *decoder, const SaveFileField *file_field, ByteReader &reader, SaveError *err, int depth)
{
    uint64_t count = 1;
    if (depth > SAVE_MAX_DEPTH)
        return save_fail(err, SVR_Damaged, "the schema nests too deeply");
    for (uint32_t dim_index = 0; dim_index < file_field->dim_count; dim_index++)
    {
        count *= file_field->dims[dim_index];
        if (count > SAVE_MAX_RAW_LEN)
            return save_fail(err, SVR_Damaged, "%s: array is too large", file_field->name);
    }
    if (file_field->stored_type == SV_STRUCT)
    {
        const std::optional<uint32_t> struct_index = find_file_struct(decoder->schema, file_field->sub_type);
        if (!struct_index)
            return save_fail(err, SVR_Damaged, "schema has no struct %s", file_field->sub_type);
        for (uint64_t index = 0; index < count; index++)
        {
            const enum SaveResult result = skip_struct(decoder, *struct_index, reader, err, depth + 1);
            if (result != SVR_Ok)
                return result;
        }
        return SVR_Ok;
    }
    if (file_field->stored_type == SV_UNION)
    {
        const auto code = reader.Get<uint8_t>();
        if (!reader.ok())
            return save_fail(err, SVR_Damaged, "record ends inside %s", file_field->name);
        if (code == 0)
            return SVR_Ok;
        if (code > file_field->member_count)
            return save_fail(err, SVR_Damaged, "%s: union member out of range", file_field->name);
        const SaveFileMember *file_member = &file_field->members[code - 1];
        if (file_member->sub_type[0] == 0)
            return (reader.Take(file_member->raw_size) != nullptr) ? SVR_Ok : save_fail(err, SVR_Damaged, "record ends inside %s", file_field->name);
        const std::optional<uint32_t> struct_index = find_file_struct(decoder->schema, file_member->sub_type);
        if (!struct_index)
            return save_fail(err, SVR_Damaged, "schema has no struct %s", file_member->sub_type);
        const enum SaveResult result = skip_struct(decoder, *struct_index, reader, err, depth + 1);
        if (result != SVR_Ok)
            return result;
        const auto tail_length = reader.Get<uint16_t>();
        if ((tail_length > 0) && (reader.Take(2U + tail_length) == nullptr))
            return save_fail(err, SVR_Damaged, "record ends inside %s", file_field->name);
        return reader.ok() ? SVR_Ok : save_fail(err, SVR_Damaged, "record ends inside %s", file_field->name);
    }
    if (reader.Take(count * save_stored_size(file_field->stored_type)) == nullptr)
        return save_fail(err, SVR_Damaged, "record ends inside %s", file_field->name);
    return SVR_Ok;
}

static enum SaveResult skip_struct(SaveDecoder *decoder, uint32_t struct_index, ByteReader &reader, SaveError *err, int depth)
{
    const SaveFileStruct *file_struct = &decoder->schema->structs[struct_index];
    for (uint32_t field_index = 0; field_index < file_struct->field_count; field_index++)
    {
        const enum SaveResult result = skip_field(decoder, &file_struct->fields[field_index], reader, err, depth);
        if (result != SVR_Ok)
            return result;
    }
    return SVR_Ok;
}
/******************************************************************************/
/** Where file element file_element_index goes in this build's array. Returns false when this build's array is smaller
 *  and the element falls off its end. */
static bool build_index(const FileShape &file, const BuildShape &build, uint32_t file_element_index, uint32_t &build_element_index)
{
    if (build.same_dims)
    {
        build_element_index = file_element_index;
        return true;
    }
    std::array<uint32_t, SAVE_MAX_DIMS> indices{};
    uint32_t rest = file_element_index;
    bool in_range = true;
    for (uint32_t index = file.dim_count; index-- > 0;)
    {
        indices[index] = rest % file.dims[index];
        rest /= file.dims[index];
        if (indices[index] >= build.dims[index])
            in_range = false;
    }
    build_element_index = 0;
    for (uint32_t dim_index = 0; dim_index < file.dim_count; dim_index++)
        build_element_index = (build_element_index * build.dims[dim_index]) + indices[dim_index];
    return in_range;
}

/** Makes the last byte of every string of a string field zero, whatever the file held, so a string read from a
 *  file is always terminated. */
static void terminate_strings(const BuildShape &build, uint8_t *base)
{
    const uint32_t dim_count = save_field_dim_count(build.field);
    if ((build.field->stored_type != SV_STR) || (dim_count == 0))
        return;
    const uint32_t row = build.dims[dim_count - 1];
    for (uint32_t row_index = 0; row_index < build.count / row; row_index++)
        base[(static_cast<size_t>(row_index) * row) + row - 1] = 0;
}

static enum SaveResult apply_default(const DefaultOp &op, uint8_t *base, SaveError *err)
{
    SaveValue value{};
    value.neg = op.field->default_value < 0;
    value.mag = value.neg ? static_cast<uint64_t>(-static_cast<int64_t>(op.field->default_value)) : static_cast<uint64_t>(op.field->default_value);
    for (uint32_t element_index = 0; element_index < op.count; element_index++)
        if (save_value_store_mem(base + (static_cast<size_t>(element_index) * op.elem), op.elem, op.field->stored_type, &value, 1) < 0)
            return save_fail(err, SVR_Unsupported, "%s: default value doesn't fit", op.field->name);
    return SVR_Ok;
}

/** Keeps the bytes of a field this build dropped, for the migrations to read. Only the top-level structs'
 *  fields are kept. */
static enum SaveResult stash_add(SaveDecoder *decoder, const char *struct_name, const SaveFileField *file_field,
    const uint8_t *bytes, uint32_t len, SaveError *err)
{
    auto *grown = static_cast<SaveStashEntry *>(realloc(decoder->stash, (decoder->stash_count + 1) * sizeof(SaveStashEntry)));
    if (grown == nullptr)
        return save_fail(err, SVR_NoMemory, "out of memory");
    decoder->stash = grown;
    SaveStashEntry *entry = &decoder->stash[decoder->stash_count];
    const size_t path_length = strlen(struct_name) + strlen(file_field->name) + 2;
    entry->path = static_cast<char *>(malloc(path_length));
    entry->bytes = static_cast<uint8_t *>(malloc(len ? static_cast<size_t>(len) : 1));
    if ((entry->path == nullptr) || (entry->bytes == nullptr))
    {
        free(entry->path);
        free(entry->bytes);
        return save_fail(err, SVR_NoMemory, "out of memory");
    }
    snprintf(entry->path, path_length, "%s.%s", struct_name, file_field->name);
    memcpy(entry->bytes, bytes, static_cast<size_t>(len));
    entry->len = len;
    entry->field = file_field;
    entry->used = 0;
    decoder->stash_count++;
    return SVR_Ok;
}

/** Reads one record by running a plan against it. */
class PlanRunner {
public:
    PlanRunner(SaveDecoder *decoder, ByteReader &reader, SaveError *err) : decoder_(decoder), reader_(reader), err_(err) {}

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    enum SaveResult Run(const Plan &plan, uint8_t *dst, int depth)
    {
        const Target target{ plan, dst, depth };
        for (const PlanOp &op : plan.ops)
        {
            const enum SaveResult result = std::visit([&](const auto &typed_op) { return Apply(target, typed_op); }, op);
            if (result != SVR_Ok)
                return result;
        }
        return SVR_Ok;
    }

private:
    struct Target {
        const Plan &plan;
        uint8_t *dst;       /* the struct being filled */
        int depth;
    };

    static const char *StructName(const Target &target) { return target.plan.file_struct->name; }

    enum SaveResult Apply(const Target &target, const DefaultOp &op)
    {
        return apply_default(op, target.dst + op.field->offset, err_);
    }

    enum SaveResult Apply(const Target &target, const SkipOp &op)
    {
        const uint32_t start = reader_.pos();
        const enum SaveResult result = skip_field(decoder_, op.file.field, reader_, err_, 0);
        if ((result != SVR_Ok) || (target.depth != 0))
            return result;
        return stash_add(decoder_, StructName(target), op.file.field, reader_.data() + start, reader_.pos() - start, err_);
    }

    enum SaveResult Apply(const Target &target, const CopyOp &op)
    {
        const SaveFieldDesc *build_field = op.build.field;
        uint8_t *base = target.dst + build_field->offset;
        const uint64_t total = static_cast<uint64_t>(op.file.count) * op.file.elem_bytes;
        if ((build_field->flags & SVF_RAW) && decoder_->foreign_raw_bytes)
        {
            if ((total <= reader_.left()) && !bytes_zero(reader_.data() + reader_.pos(), static_cast<uint32_t>(total)))
                return save_fail(err_, SVR_Unsupported,
                    "%s.%s: is raw memory made by a machine with the other byte order, which this one can't read",
                    StructName(target), op.file.field->name);
        }
        if (op.block)
        {
            const uint8_t *bytes = reader_.Take(total);
            if (bytes == nullptr)
                return save_fail(err_, SVR_Damaged, "record ends inside %s.%s", StructName(target), op.file.field->name);
            memcpy(base, bytes, static_cast<size_t>(total));
            terminate_strings(op.build, base);
            return SVR_Ok;
        }
        if (!op.build.same_dims && (build_field->stored_type != SV_STR) && (build_field->default_value != 0))
        {
            const enum SaveResult result = apply_default(DefaultOp{ build_field, op.build.count, op.build.elem }, base, err_);
            if (result != SVR_Ok)
                return result;
        }
        for (uint32_t element_index = 0; element_index < op.file.count; element_index++)
        {
            uint32_t build_element_index;
            const bool in_range = build_index(op.file, op.build, element_index, build_element_index);
            const enum SaveResult result = ReadScalar(target, op, base, build_element_index, in_range);
            if (result != SVR_Ok)
                return result;
        }
        terminate_strings(op.build, base);
        return SVR_Ok;
    }

    enum SaveResult Apply(const Target &target, const SubOp &op)
    {
        uint8_t *base = target.dst + op.build.field->offset;
        for (uint32_t element_index = 0; element_index < op.file.count; element_index++)
        {
            uint32_t build_element_index;
            const bool in_range = build_index(op.file, op.build, element_index, build_element_index);
            const enum SaveResult result = ReadStruct(target, op, base, build_element_index, in_range);
            if (result != SVR_Ok)
                return result;
        }
        return SVR_Ok;
    }

    /** One scalar element of an array, converted to this build's type. Strings may be cut short. */
    enum SaveResult ReadScalar(const Target &target, const CopyOp &op, uint8_t *base, uint32_t build_element_index, bool in_range)
    {
        const SaveFieldDesc *build_field = op.build.field;
        const uint8_t *bytes = reader_.Take(op.file.elem_bytes);
        SaveValue value;
        if (bytes == nullptr)
            return save_fail(err_, SVR_Damaged, "record ends inside %s.%s", StructName(target), op.file.field->name);
        if (save_value_load_le(bytes, op.file.field->stored_type, &value) != 0)
            return save_fail(err_, SVR_Damaged, "%s.%s: bad stored type", StructName(target), op.file.field->name);
        if (!in_range)
        {
            if ((build_field->stored_type != SV_STR) && !save_value_is_zero(&value))
                return save_fail(err_, SVR_Unsupported, "%s.%s: the save uses more entries than this build supports",
                    StructName(target), op.file.field->name);
            return SVR_Ok;
        }
        if (save_value_store_mem(base + (static_cast<size_t>(build_element_index) * op.build.elem), op.build.elem,
                build_field->stored_type, &value, (build_field->flags & SVF_CLAMP) != 0) < 0)
            return save_fail(err_, SVR_Unsupported, "%s.%s: a saved value doesn't fit %s in this build",
                StructName(target), op.file.field->name, save_stored_name(build_field->stored_type));
        return SVR_Ok;
    }

    /** One struct element of an array. One that falls off the end of this build's array is only allowed to be
     *  all zero. */
    enum SaveResult ReadStruct(const Target &target, const SubOp &op, uint8_t *base, uint32_t build_element_index, bool in_range)
    {
        if (in_range)
            return Run(*op.plan, base + (static_cast<size_t>(build_element_index) * op.build.elem), target.depth + 1);
        const uint32_t start = reader_.pos();
        const enum SaveResult result = skip_struct(decoder_, op.file.struct_index, reader_, err_, target.depth + 1);
        if (result != SVR_Ok)
            return result;
        if (!bytes_zero(reader_.data() + start, reader_.pos() - start))
            return save_fail(err_, SVR_Unsupported, "%s.%s: the save uses more entries than this build supports",
                StructName(target), op.file.field->name);
        return SVR_Ok;
    }

    enum SaveResult Apply(const Target &target, const UnionOp &op)
    {
        const auto code = reader_.Get<uint8_t>();
        if (!reader_.ok())
            return save_fail(err_, SVR_Damaged, "record ends inside %s.%s", StructName(target), op.file_field->name);
        if (code == 0)
            return SVR_Ok;
        if (code > op.file_field->member_count)
            return save_fail(err_, SVR_Damaged, "%s.%s: union member out of range", StructName(target), op.file_field->name);
        const uint32_t member_index = static_cast<uint32_t>(code) - 1;
        const SaveFileMember *file_member = &op.file_field->members[member_index];
        const SaveUnionMember *build_member = op.build_members[member_index];
        if (build_member == nullptr)
            return save_fail(err_, SVR_Unsupported, "%s.%s: the save uses %s, which this build doesn't have",
                StructName(target), op.file_field->name, file_member->name);
        if (op.plans[member_index] != nullptr)
        {
            const enum SaveResult result = Run(*op.plans[member_index], target.dst, target.depth + 1);
            if (result != SVR_Ok)
                return result;
            return ReadMemberTail(target, op, build_member);
        }
        return ReadRawMember(target, op, file_member, build_member);
    }

    /** Puts back the bytes a member left over (see encode_member_tail), except those a field of this build's member
     *  covers. */
    enum SaveResult ReadMemberTail(const Target &target, const UnionOp &op, const SaveUnionMember *build_member)
    {
        const uint32_t tail_length = reader_.Get<uint16_t>();
        if (!reader_.ok())
            return save_fail(err_, SVR_Damaged, "record ends inside %s.%s", StructName(target), op.file_field->name);
        if (tail_length == 0)
            return SVR_Ok;
        const uint32_t tail_offset = reader_.Get<uint16_t>();
        const uint8_t *bytes = reader_.Take(tail_length);
        if (bytes == nullptr)
            return save_fail(err_, SVR_Damaged, "record ends inside %s.%s", StructName(target), op.file_field->name);
        if (decoder_->foreign_raw_bytes)
            return save_fail(err_, SVR_Unsupported,
                "%s.%s: has raw memory made by a machine with the other byte order, which this one can't read",
                StructName(target), op.file_field->name);
        const SaveFieldDesc *build_field = op.build_field;
        const uint32_t member_end = save_struct_extent(build_member->sub);
        const uint32_t covered = (member_end > build_field->offset) ? (member_end - build_field->offset) : 0;
        for (uint32_t index = 0; index < tail_length; index++)
        {
            const uint32_t position = tail_offset + index;
            if (position < covered)
                continue;
            if (position < build_field->mem_size)
                target.dst[build_field->offset + position] = bytes[index];
            else if (bytes[index] != 0)
                return save_fail(err_, SVR_Unsupported, "%s.%s: holds more bytes than this build has room for",
                    StructName(target), op.file_field->name);
        }
        return SVR_Ok;
    }

    enum SaveResult ReadRawMember(const Target &target, const UnionOp &op, const SaveFileMember *file_member,
        const SaveUnionMember *build_member)
    {
        const uint8_t *bytes = reader_.Take(file_member->raw_size);
        const uint32_t keep = (file_member->raw_size < build_member->raw_size) ? file_member->raw_size : build_member->raw_size;
        if (bytes == nullptr)
            return save_fail(err_, SVR_Damaged, "record ends inside %s.%s", StructName(target), op.file_field->name);
        if (decoder_->foreign_raw_bytes && !bytes_zero(bytes, file_member->raw_size))
            return save_fail(err_, SVR_Unsupported,
                "%s.%s: %s is raw memory made by a machine with the other byte order, which this one can't read",
                StructName(target), op.file_field->name, file_member->name);
        if (!bytes_zero(bytes + keep, file_member->raw_size - keep))
            return save_fail(err_, SVR_Unsupported, "%s.%s: %s holds more bytes than this build has room for",
                StructName(target), op.file_field->name, file_member->name);
        memcpy(target.dst + op.build_field->offset, bytes, keep);
        return SVR_Ok;
    }

    SaveDecoder *decoder_;
    ByteReader &reader_;
    SaveError *err_;
};

enum SaveResult save_decode_record(struct SaveDecoder *decoder, const struct SaveStructDesc *desc,
    const uint8_t *data, uint32_t len, uint32_t *used, void *dst, struct SaveError *err)
{
    if (decoder->priv == nullptr)
        return save_fail(err, SVR_NoMemory, "out of memory");
    const std::optional<uint32_t> file_index = find_file_struct(decoder->schema, desc->name);
    if (!file_index)
        return save_fail(err, SVR_Damaged, "the file has no schema for %s", desc->name);
    try
    {
        Plan *plan = nullptr;
        enum SaveResult result = build_plan(decoder, *file_index, desc, &plan, err, 0);
        if (result != SVR_Ok)
        {
            static_cast<DecoderState *>(decoder->priv)->plans.clear();     // a plan that failed is not complete
            return result;
        }
        ByteReader reader(data, len);
        PlanRunner runner(decoder, reader, err);
        result = runner.Run(*plan, static_cast<uint8_t *>(dst), 0);
        if ((result == SVR_Ok) && (used != nullptr))
            *used = reader.pos();
        return result;
    } catch (const std::bad_alloc &)
    {
        return save_fail(err, SVR_NoMemory, "out of memory");
    }
}
