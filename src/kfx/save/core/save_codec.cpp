/******************************************************************************/
/** @file save_codec.cpp
 *     KFXS container: file header, chunks, CRC and zlib. No game dependencies.
 */
/******************************************************************************/
#include "save_codec.h"
#include "save_bytes.h"

#include <array>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <zlib.h>

using savebytes::LoadLE;
using savebytes::StoreLE;
/******************************************************************************/
static const char *fourcc_str(uint32_t id, char *text)
{
    for (int index = 0; index < 4; index++)
    {
        const auto byte = static_cast<uint8_t>(id >> (8 * index));
        text[index] = ((byte >= 32) && (byte < 127)) ? static_cast<char>(byte) : '?';
    }
    text[4] = 0;
    return text;
}

enum SaveResult save_fail(struct SaveError *err, enum SaveResult result, const char *fmt, ...)
{
    if (err != nullptr)
    {
        va_list args;
        err->result = result;
        va_start(args, fmt);
        vsnprintf(err->message, sizeof(err->message), fmt, args);
        va_end(args);
    }
    return result;
}

void save_buf_free(struct SaveBuffer *buffer)
{
    free(buffer->data);
    buffer->data = nullptr;
    buffer->len = buffer->cap = 0;
}

int save_buf_append(struct SaveBuffer *buffer, const void *data, uint32_t len)
{
    if (len > UINT32_MAX - buffer->len)
        return -1;
    if (buffer->len + len > buffer->cap)
    {
        uint64_t cap = buffer->cap ? buffer->cap : 256;
        while (cap < static_cast<uint64_t>(buffer->len) + len)
            cap *= 2;
        if (cap > UINT32_MAX)
            cap = UINT32_MAX;
        auto *grown = static_cast<uint8_t *>(realloc(buffer->data, static_cast<size_t>(cap)));
        if (grown == nullptr)
            return -1;
        buffer->data = grown;
        buffer->cap = static_cast<uint32_t>(cap);
    }
    if (len > 0)
        memcpy(buffer->data + buffer->len, data, len);
    buffer->len += len;
    return 0;
}

template <typename T>
static int buf_put(struct SaveBuffer *buffer, T value)
{
    std::array<uint8_t, sizeof(T)> bytes;
    StoreLE<T>(bytes.data(), value);
    return save_buf_append(buffer, bytes.data(), sizeof(T));
}

int save_buf_u8(struct SaveBuffer *buffer, uint8_t value) { return buf_put<uint8_t>(buffer, value); }
int save_buf_u16(struct SaveBuffer *buffer, uint16_t value) { return buf_put<uint16_t>(buffer, value); }
int save_buf_u32(struct SaveBuffer *buffer, uint32_t value) { return buf_put<uint32_t>(buffer, value); }

bool save_host_is_little_endian(void)
{
    const uint16_t one = 1;
    return *reinterpret_cast<const uint8_t *>(&one) == 1;
}

int save_chunk_add_buffer(struct SaveBuffer *buffer, uint32_t id, uint32_t flags, const struct SaveBuffer *payload,
    int compress)
{
    if (payload->has_raw_bytes)
    {
        flags |= SCF_RawBytes;
        if (!save_host_is_little_endian())
            flags |= SCF_RawBigEndian;
    }
    return save_chunk_add(buffer, id, flags, payload->data, payload->len, compress);
}

static constexpr std::array<uint8_t, SAVE_MAGIC_SIZE> save_magic = { 'K', 'F', 'X', 'S', 0x00, 0xFF, 0x1A, 0x0A };

bool save_magic_matches(const void *bytes, uint32_t len)
{
    return (len >= SAVE_MAGIC_SIZE) && (memcmp(bytes, save_magic.data(), SAVE_MAGIC_SIZE) == 0);
}

int save_file_begin(struct SaveBuffer *buffer, uint32_t kind)
{
    uint8_t head[SAVE_HEADER_SIZE - 4];
    memcpy(head, save_magic.data(), SAVE_MAGIC_SIZE);
    StoreLE<uint16_t>(head + 8, SAVE_FORMAT_MAJOR);
    StoreLE<uint16_t>(head + 10, SAVE_FORMAT_MINOR);
    StoreLE<uint32_t>(head + 12, kind);
    if (save_buf_append(buffer, head, sizeof(head)) != 0)
        return -1;
    return save_buf_u32(buffer, static_cast<uint32_t>(crc32(0, head, sizeof(head))));
}

int save_chunk_add(struct SaveBuffer *buffer, uint32_t id, uint32_t flags, const void *raw, uint32_t raw_len, int compress)
{
    const void *body = raw;
    uint32_t stored_len = raw_len;
    std::vector<uint8_t> packed;
    flags &= ~static_cast<uint32_t>(SCF_Zlib);
    if (compress && raw_len > 0)
    {
        uLongf plen = compressBound(raw_len);
        packed.resize(plen);
        if ((compress2(packed.data(), &plen, static_cast<const Bytef *>(raw), raw_len, 1) == Z_OK) && (plen < raw_len))
        {
            body = packed.data();
            stored_len = static_cast<uint32_t>(plen);
            flags |= SCF_Zlib;
        }
    }
    int failed = save_buf_u32(buffer, id);
    failed |= save_buf_u32(buffer, flags);
    failed |= save_buf_u32(buffer, stored_len);
    failed |= save_buf_u32(buffer, raw_len);
    failed |= save_buf_u32(buffer, static_cast<uint32_t>(crc32(0, static_cast<const Bytef *>(raw), raw_len)));
    failed |= save_buf_append(buffer, body, stored_len);
    return (failed != 0) ? -1 : 0;
}

enum SaveResult save_reader_open(struct SaveReader *reader, const void *data, uint32_t len, struct SaveError *err)
{
    const auto *bytes = static_cast<const uint8_t *>(data);
    memset(reader, 0, sizeof(*reader));
    if ((len < SAVE_HEADER_SIZE) || !save_magic_matches(bytes, len)) {
        // And let us never return to this place again, lest we be cursed with the wrath of the ancient save gods.
        return save_fail(err, SVR_TooOld, "not a KFXS file; it was made before the save format changed");
    }
    if (LoadLE<uint32_t>(bytes + 16) != static_cast<uint32_t>(crc32(0, bytes, 16)))
        return save_fail(err, SVR_Damaged, "file header is damaged");
    reader->major = LoadLE<uint16_t>(bytes + 8);
    reader->minor = LoadLE<uint16_t>(bytes + 10);
    reader->kind = LoadLE<uint32_t>(bytes + 12);
    if (reader->major > SAVE_FORMAT_MAJOR)
        return save_fail(err, SVR_TooNew, "made by a newer build (save format %u)", static_cast<unsigned>(reader->major));
    if (reader->major < SAVE_MIN_READABLE_MAJOR)
        return save_fail(err, SVR_TooOld, "made by an older build (save format %u)", static_cast<unsigned>(reader->major));
    reader->data = bytes;
    reader->len = len;
    reader->pos = SAVE_HEADER_SIZE;
    return SVR_Ok;
}

static bool chunk_id_known(uint32_t id)
{
    switch (id)
    {
    case SCID_Meta: case SCID_Schema: case SCID_Names: case SCID_Game: case SCID_Config: case SCID_Level:
    case SCID_Lua: case SCID_Records: case SCID_PacketHead: case SCID_TurnData:
        return true;
    default:
        return false;
    }
}

enum SaveResult save_chunk_head(const uint8_t *bytes, uint32_t after, struct SaveChunk *chunk, struct SaveError *err)
{
    char fcc[5];
    chunk->id = LoadLE<uint32_t>(bytes);
    chunk->flags = LoadLE<uint32_t>(bytes + 4);
    chunk->stored_len = LoadLE<uint32_t>(bytes + 8);
    chunk->raw_len = LoadLE<uint32_t>(bytes + 12);
    chunk->crc32 = LoadLE<uint32_t>(bytes + 16);
    chunk->body = nullptr;
    if (chunk->stored_len > after)
        return save_fail(err, SVR_Damaged, "chunk %s is longer than the file", fourcc_str(chunk->id, fcc));
    if (chunk->raw_len > SAVE_MAX_RAW_LEN)
        return save_fail(err, SVR_Damaged, "chunk %s is too large", fourcc_str(chunk->id, fcc));
    if (((chunk->flags & SCF_Zlib) == 0) && (chunk->raw_len != chunk->stored_len))
        return save_fail(err, SVR_Damaged, "chunk %s has inconsistent lengths", fourcc_str(chunk->id, fcc));
    /* A chunk this build doesn't know is skipped by the loaders, unless the writer marked it required. */
    if (((chunk->flags & SCF_Required) != 0) && !chunk_id_known(chunk->id))
        return save_fail(err, SVR_TooNew, "made by a newer build (chunk %s)", fourcc_str(chunk->id, fcc));
    return SVR_Ok;
}

enum SaveResult save_reader_next(struct SaveReader *reader, struct SaveChunk *chunk, struct SaveError *err)
{
    if (reader->pos == reader->len)
        return SVR_NotFound;
    if (reader->len - reader->pos < SAVE_CHUNK_HEADER_SIZE)
        return save_fail(err, SVR_Damaged, "file ends inside a chunk header");
    const enum SaveResult result = save_chunk_head(reader->data + reader->pos, reader->len - reader->pos - SAVE_CHUNK_HEADER_SIZE, chunk, err);
    reader->pos += SAVE_CHUNK_HEADER_SIZE;
    if (result != SVR_Ok)
        return result;
    chunk->body = reader->data + reader->pos;
    reader->pos += chunk->stored_len;
    return SVR_Ok;
}

enum SaveResult save_chunk_unpack(const struct SaveChunk *chunk, uint8_t **out, uint32_t *out_len, struct SaveError *err)
{
    char fcc[5];
    auto *raw = static_cast<uint8_t *>(malloc(chunk->raw_len ? chunk->raw_len : 1));
    if (raw == nullptr)
        return save_fail(err, SVR_NoMemory, "out of memory reading chunk %s", fourcc_str(chunk->id, fcc));
    if (chunk->flags & SCF_Zlib)
    {
        uLongf dlen = chunk->raw_len;
        if ((uncompress(raw, &dlen, chunk->body, chunk->stored_len) != Z_OK) || (dlen != chunk->raw_len))
        {
            free(raw);
            return save_fail(err, SVR_Damaged, "chunk %s can't be decompressed", fourcc_str(chunk->id, fcc));
        }
    } else
    {
        memcpy(raw, chunk->body, chunk->raw_len);
    }
    if (static_cast<uint32_t>(crc32(0, raw, chunk->raw_len)) != chunk->crc32)
    {
        free(raw);
        return save_fail(err, SVR_Damaged, "chunk %s failed its checksum", fourcc_str(chunk->id, fcc));
    }
    *out = raw;
    *out_len = chunk->raw_len;
    return SVR_Ok;
}
