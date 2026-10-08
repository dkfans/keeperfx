/******************************************************************************/
/** @file save_config_overlay.cpp
 *     Diffs struct Configs against a baseline by field path (write), and
 *     applies a parsed diff back onto a freshly-loaded Configs by field path
 *     (apply). A save keeps only what a level actually changed, so a config
 *     struct layout change never breaks it.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/save/SaveTypes.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "packets.h"
#include "config_campaigns.h"
#include "front_network.h"
#include "post_inc.h"

#include <array>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "kfx/save/core/save_bytes.h"
#include "kfx/save/core/save_config_overlay.h"
#include "kfx/save/core/save_schema.h"
#include "kfx/save/core/schema/save_tables_decl.h"
/******************************************************************************/
using savebytes::ByteReader;

constexpr size_t PATH_MAX_LEN = 255;

static int put_str(struct SaveBuffer *buffer, const char *text, size_t len)
{
    if (len > 0xFFFF)
        return -1;
    return save_buf_u16(buffer, static_cast<uint16_t>(len)) | save_buf_append(buffer, text, static_cast<uint32_t>(len));
}

static void path_append(char *path, size_t cap, const char *name)
{
    size_t path_length = strlen(path);
    if (path_length > 0)
    {
        if (path_length + 1 < cap) { path[path_length] = '.'; path[path_length + 1] = 0; path_length++; }
    }
    const size_t name_length = strlen(name);
    if (path_length + name_length < cap)
        memcpy(path + path_length, name, name_length + 1);
}

static void path_append_index(char *path, size_t cap, uint32_t index)
{
    std::array<char, 16> text;
    const size_t path_length = strlen(path);
    const int text_length = snprintf(text.data(), text.size(), "[%u]", index);
    if ((text_length > 0) && (path_length + static_cast<size_t>(text_length) < cap))
        memcpy(path + path_length, text.data(), static_cast<size_t>(text_length) + 1);
}
/******************************************************************************/
static int diff_struct(const struct SaveStructDesc *desc, const uint8_t *current_bytes, const uint8_t *base,
    char *path, size_t pathcap, struct SaveBuffer *out, uint32_t *diff_count)
{
    int failed = 0;
    const size_t base_len = strlen(path);
    for (uint32_t field_index = 0; field_index < desc->field_count; field_index++)
    {
        const struct SaveFieldDesc *field = &desc->fields[field_index];
        if (field->stored_type == SV_UNION)
            continue;
        path[base_len] = 0;
        path_append(path, pathcap, field->name);
        const uint32_t count = save_field_count(field);
        const uint32_t elem = field->mem_size / count;
        if (field->stored_type == SV_STR)
        {
            /* One field is one leaf: the whole char array, not per-character. */
            const uint8_t *current_string = current_bytes + field->offset;
            if (memcmp(current_string, base + field->offset, field->mem_size) != 0)
            {
                const size_t string_length = strnlen(reinterpret_cast<const char *>(current_string), field->mem_size);
                failed |= put_str(out, path, strlen(path));
                failed |= save_buf_u8(out, SV_STR);
                failed |= put_str(out, reinterpret_cast<const char *>(current_string), string_length);
                (*diff_count)++;
            }
            continue;
        }
        const size_t field_len = strlen(path);
        for (uint32_t index = 0; index < count; index++)
        {
            const uint8_t *current_element = current_bytes + field->offset + (static_cast<size_t>(index) * elem);
            const uint8_t *baseline_element = base + field->offset + (static_cast<size_t>(index) * elem);
            if (field->stored_type == SV_STRUCT)
            {
                path[field_len] = 0;
                if (count > 1)
                    path_append_index(path, pathcap, index);
                failed |= diff_struct(field->sub, current_element, baseline_element, path, pathcap, out, diff_count);
                continue;
            }
            if (memcmp(current_element, baseline_element, elem) != 0)
            {
                path[field_len] = 0;
                if (count > 1)
                    path_append_index(path, pathcap, index);
                struct SaveValue value;
                save_value_load_mem(current_element, elem, field->stored_type, &value);
                std::array<uint8_t, 8> bytes;
                if (save_value_store_le(bytes.data(), field->stored_type, &value, 1) < 0)
                    continue; /* not a plain scalar type; nothing sane to store */
                failed |= put_str(out, path, strlen(path));
                failed |= save_buf_u8(out, field->stored_type);
                failed |= save_buf_append(out, bytes.data(), save_stored_size(field->stored_type));
                (*diff_count)++;
            }
        }
    }
    path[base_len] = 0;
    return failed;
}

enum SaveResult save_config_overlay_write(const struct Configs *current, const struct Configs *baseline,
    struct SaveBuffer *out, struct SaveError *err)
{
    int failed = 0;
    SaveBufferOwner diffs;
    char path[PATH_MAX_LEN + 1];
    path[0] = 0;
    uint32_t diff_count = 0;
    failed |= diff_struct(&save_desc_Configs, reinterpret_cast<const uint8_t *>(current), reinterpret_cast<const uint8_t *>(baseline),
        path, sizeof(path), &diffs, &diff_count);
    failed |= save_buf_u32(out, diff_count);
    failed |= save_buf_append(out, diffs.data, diffs.len);
    return (failed != 0) ? save_fail(err, SVR_NoMemory, "out of memory writing the config overlay") : SVR_Ok;
}

static uint8_t *resolve_path(const struct SaveStructDesc *desc, uint8_t *base,
    const char *path, uint8_t *out_stored_type, uint32_t *out_elem_size)
{
    const char *text = path;
    for (;;)
    {
        std::array<char, 64> name;
        size_t name_length = 0;
        while ((text[name_length] != 0) && (text[name_length] != '.') && (text[name_length] != '[') && (name_length < name.size() - 1))
        {
            name[name_length] = text[name_length];
            name_length++;
        }
        name[name_length] = 0;
        text += name_length;
        uint32_t index = 0;
        int has_index = 0;
        if (*text == '[')
        {
            has_index = 1;
            text++;
            while ((*text >= '0') && (*text <= '9')) { index = index * 10 + static_cast<uint32_t>(*text - '0'); text++; }
            if (*text == ']')
                text++;
        }
        const struct SaveFieldDesc *field = nullptr;
        for (uint32_t field_index = 0; field_index < desc->field_count; field_index++)
        {
            const struct SaveFieldDesc *cand = &desc->fields[field_index];
            if (strcmp(cand->name, name.data()) == 0) { field = cand; break; }
            if (cand->aliases != nullptr)
            {
                for (const char *const *alias = cand->aliases; *alias != nullptr; alias++)
                    if (strcmp(*alias, name.data()) == 0) { field = cand; break; }
                if (field != nullptr)
                    break;
            }
        }
        if (field == nullptr)
            return nullptr;
        const uint32_t count = save_field_count(field);
        const uint32_t elem = field->mem_size / count;
        if (field->stored_type == SV_STR)
        {
            if (*text != 0)
                return nullptr; /* a string leaf has no further segments */
            *out_stored_type = SV_STR;
            *out_elem_size = field->mem_size;
            return base + field->offset;
        }
        const uint32_t flat = has_index ? index : 0;
        if (flat >= count)
            return nullptr;
        uint8_t *elem_ptr = base + field->offset + (static_cast<size_t>(flat) * elem);
        if (field->stored_type == SV_STRUCT)
        {
            if (*text == '.')
            {
                text++;
                desc = field->sub;
                base = elem_ptr;
                continue;
            }
            return nullptr; /* a struct leaf needs a further segment */
        }
        if (*text != 0)
            return nullptr; /* a scalar leaf has no further segments */
        *out_stored_type = field->stored_type;
        *out_elem_size = elem;
        return elem_ptr;
    }
}

enum SaveResult save_config_overlay_apply(const uint8_t *conf_data, uint32_t conf_len,
    struct Configs *current, struct SaveError *err)
{
    ByteReader reader(conf_data, conf_len);
    const auto count = reader.Get<uint32_t>();
    for (uint32_t index = 0; (index < count) && reader.ok(); index++)
    {
        uint16_t path_len;
        const uint8_t *path_bytes = reader.TakeString(&path_len);
        const auto stored_type = reader.Get<uint8_t>();
        if (!reader.ok())
            break;
        char path[PATH_MAX_LEN + 1];
        const size_t path_length = (path_len < PATH_MAX_LEN) ? path_len : PATH_MAX_LEN;
        memcpy(path, path_bytes, path_length);
        path[path_length] = 0;
        if (stored_type == SV_STR)
        {
            uint16_t slen;
            const uint8_t *sbytes = reader.TakeString(&slen);
            if (!reader.ok())
                break;
            uint8_t out_type;
            uint32_t out_size;
            uint8_t *dst = resolve_path(&save_desc_Configs, reinterpret_cast<uint8_t *>(current), path, &out_type, &out_size);
            if ((dst == nullptr) || (out_type != SV_STR))
                continue;
            const uint32_t copy_len = (slen < out_size) ? slen : out_size;
            memset(dst, 0, out_size);
            memcpy(dst, sbytes, copy_len);
            continue;
        }
        const uint32_t size = save_stored_size(stored_type);
        const uint8_t *raw = reader.Take(size);
        if (!reader.ok())
            break;
        uint8_t out_type;
        uint32_t out_size;
        uint8_t *dst = resolve_path(&save_desc_Configs, reinterpret_cast<uint8_t *>(current), path, &out_type, &out_size);
        if ((dst == nullptr) || (out_type == SV_STR) || (out_type == SV_STRUCT))
            continue;
        struct SaveValue value;
        if (save_value_load_le(raw, stored_type, &value) < 0)
            continue;
        save_value_store_mem(dst, out_size, out_type, &value, 1);
    }
    if (!reader.ok())
        return save_fail(err, SVR_Damaged, "the config overlay is damaged");
    return SVR_Ok;
}
