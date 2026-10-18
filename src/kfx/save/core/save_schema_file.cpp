/******************************************************************************/
/** @file save_schema_file.cpp
 *     The schema stored in a file (the SCHM chunk): writing it from the field tables and reading it back.
 *     But who reads this anyway.
 */
/******************************************************************************/
#include "save_schema.h"
#include "save_internal.h"
#include "save_bytes.h"

#include <cstdlib>
#include <cstring>

using savebytes::ByteReader;
/******************************************************************************/
static int put_str(struct SaveBuffer *buffer, const char *text)
{
    const size_t length = strlen(text);
    if (length > 0xFFFF)
        return -1;
    return save_buf_u16(buffer, static_cast<uint16_t>(length)) | save_buf_append(buffer, text, static_cast<uint32_t>(length));
}

enum SaveResult save_schema_encode(struct SaveBuffer *out, const struct SaveStructDesc *const *roots,
    uint32_t root_count, enum SaveMode mode, struct SaveError *err)
{
    SaveStructListOwner list;
    const enum SaveResult result = save_schema_collect(&list, roots, root_count, mode, err);
    if (result != SVR_Ok)
        return result;
    int failed = save_buf_u32(out, list.count);
    for (uint32_t index = 0; index < list.count; index++)
    {
        const SaveStructDesc *desc = list.structs[index];
        uint32_t field_count = 0;
        for (uint32_t field_index = 0; field_index < desc->field_count; field_index++)
            field_count += save_field_included(&desc->fields[field_index], mode);
        failed |= put_str(out, desc->name);
        failed |= save_buf_u16(out, static_cast<uint16_t>(field_count));
        for (uint32_t field_index = 0; field_index < desc->field_count; field_index++)
        {
            const SaveFieldDesc *field = &desc->fields[field_index];
            if (!save_field_included(field, mode))
                continue;
            const uint32_t dim_count = save_field_dim_count(field);
            failed |= put_str(out, field->name);
            failed |= save_buf_u8(out, field->stored_type);
            failed |= save_buf_u8(out, static_cast<uint8_t>(save_field_flags(field) & (SVF_SAVE | SVF_SYNC)));
            failed |= save_buf_u8(out, static_cast<uint8_t>(dim_count));
            for (uint32_t dim_index = 0; dim_index < dim_count; dim_index++)
                failed |= save_buf_u32(out, field->dims[dim_index]);
            failed |= put_str(out, (field->sub != nullptr) ? field->sub->name : "");
            if (field->stored_type == SV_UNION)
            {
                failed |= save_buf_u16(out, static_cast<uint16_t>(field->uni->member_count));
                for (uint32_t member_index = 0; member_index < field->uni->member_count; member_index++)
                {
                    const SaveUnionMember *member = &field->uni->members[member_index];
                    failed |= put_str(out, member->name);
                    failed |= put_str(out, (member->sub != nullptr) ? member->sub->name : "");
                    failed |= save_buf_u32(out, member->raw_size);
                }
            }
        }
    }
    return (failed != 0) ? save_fail(err, SVR_NoMemory, "out of memory writing the schema") : SVR_Ok;
}
/******************************************************************************/
void save_schema_free(struct SaveFileSchema *schema)
{
    for (uint32_t struct_index = 0; struct_index < schema->struct_count; struct_index++)
    {
        SaveFileStruct *file_struct = &schema->structs[struct_index];
        for (uint32_t field_index = 0; field_index < file_struct->field_count; field_index++)
        {
            SaveFileField *file_field = &file_struct->fields[field_index];
            free(file_field->name);
            free(file_field->sub_type);
            for (uint32_t member_index = 0; member_index < file_field->member_count; member_index++)
            {
                free(file_field->members[member_index].name);
                free(file_field->members[member_index].sub_type);
            }
            free(file_field->members);
        }
        free(file_struct->fields);
        free(file_struct->name);
    }
    free(schema->structs);
    memset(schema, 0, sizeof(*schema));
}

/** A malloc'd, terminated copy of the next string. NULL on a short read, or when out of memory. */
static char *read_string(ByteReader &reader, bool &oom)
{
    uint16_t length;
    const uint8_t *bytes = reader.TakeString(&length);
    if (bytes == nullptr)
        return nullptr;
    char *copy = static_cast<char *>(malloc(static_cast<size_t>(length) + 1));
    if (copy == nullptr)
    {
        oom = true;
        return nullptr;
    }
    memcpy(copy, bytes, length);
    copy[length] = 0;
    return copy;
}

static void read_field(ByteReader &reader, bool &oom, SaveFileField *file_field)
{
    file_field->name = read_string(reader, oom);
    file_field->stored_type = reader.Get<uint8_t>();
    file_field->flags = reader.Get<uint8_t>();
    file_field->dim_count = reader.Get<uint8_t>();
    if (file_field->dim_count > SAVE_MAX_DIMS)
    {
        reader.Fail();
        return;
    }
    for (uint32_t dim_index = 0; dim_index < file_field->dim_count; dim_index++)
        file_field->dims[dim_index] = reader.Get<uint32_t>();
    file_field->sub_type = read_string(reader, oom);
    if (file_field->stored_type > SV_UNION)
        reader.Fail();
    if ((file_field->stored_type != SV_UNION) || !reader.ok())
        return;

    const uint32_t member_count = reader.Get<uint16_t>();
    if (!reader.ok() || (file_field->dim_count != 0) || (member_count > SAVE_UNION_MAX_MEMBERS) || (member_count > reader.left() / 8))
    {
        reader.Fail();
        return;
    }
    file_field->members = static_cast<SaveFileMember *>(calloc(member_count ? member_count : 1, sizeof(SaveFileMember)));
    if (file_field->members == nullptr)
    {
        oom = true;
        return;
    }
    file_field->member_count = member_count;
    for (uint32_t member_index = 0; (member_index < member_count) && reader.ok(); member_index++)
    {
        file_field->members[member_index].name = read_string(reader, oom);
        file_field->members[member_index].sub_type = read_string(reader, oom);
        file_field->members[member_index].raw_size = reader.Get<uint32_t>();
    }
}

static void read_struct(ByteReader &reader, bool &oom, SaveFileStruct *file_struct)
{
    file_struct->name = read_string(reader, oom);
    const uint32_t field_count = reader.Get<uint16_t>();
    if (!reader.ok() || (field_count > reader.left() / 8))
    {
        reader.Fail();
        return;
    }
    file_struct->fields = static_cast<SaveFileField *>(calloc(field_count ? field_count : 1, sizeof(SaveFileField)));
    if (file_struct->fields == nullptr)
    {
        oom = true;
        return;
    }
    file_struct->field_count = field_count;
    for (uint32_t field_index = 0; (field_index < field_count) && reader.ok() && !oom; field_index++)
        read_field(reader, oom, &file_struct->fields[field_index]);
}

enum SaveResult save_schema_decode(const uint8_t *data, uint32_t len, struct SaveFileSchema *schema, struct SaveError *err)
{
    ByteReader reader(data, len);
    bool oom = false;
    memset(schema, 0, sizeof(*schema));
    const auto count = reader.Get<uint32_t>();
    if (!reader.ok() || (count > reader.left() / 4))
        return save_fail(err, SVR_Damaged, "schema is damaged");
    schema->structs = static_cast<SaveFileStruct *>(calloc(count ? count : 1, sizeof(SaveFileStruct)));
    if (schema->structs == nullptr)
        return save_fail(err, SVR_NoMemory, "out of memory reading the schema");
    schema->struct_count = count;
    for (uint32_t index = 0; (index < count) && reader.ok() && !oom; index++)
        read_struct(reader, oom, &schema->structs[index]);
    if (!reader.ok() || oom || (reader.left() != 0))
    {
        save_schema_free(schema);
        return save_fail(err, oom ? SVR_NoMemory : SVR_Damaged, "schema is damaged");
    }
    return SVR_Ok;
}
