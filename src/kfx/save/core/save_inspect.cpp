/******************************************************************************/
/** @file save_inspect.cpp
 *     Cheap check of a savegame file: structure and title, without reading the whole file.
 */
/******************************************************************************/
#include "save_inspect.h"
#include "save_bytes.h"

#include <vector>

using savebytes::MallocBytes;
/******************************************************************************/
/** Reads and unpacks the chunk whose header is at offset pos. */
static enum SaveResult read_chunk(SaveReadAtFn read_at, void *context, uint32_t pos, const SaveChunk *head,
    MallocBytes &raw, uint32_t *raw_len, SaveError *err)
{
    SaveChunk chunk = *head;
    std::vector<uint8_t> body(head->stored_len ? head->stored_len : 1);
    if (read_at(context, pos + SAVE_CHUNK_HEADER_SIZE, body.data(), head->stored_len) != 0)
        return save_fail(err, SVR_Damaged, "can't read the file");
    chunk.body = body.data();
    uint8_t *out = nullptr;
    const enum SaveResult result = save_chunk_unpack(&chunk, &out, raw_len, err);
    raw.reset(out);
    return result;
}

enum SaveResult save_inspect_game(SaveReadAtFn read_at, void *context, uint32_t file_len,
    const struct SaveStructDesc *meta_desc, void *meta, int *meta_ok, struct SaveError *err)
{
    uint8_t head[SAVE_HEADER_SIZE];
    *meta_ok = 0;
    if ((file_len < SAVE_HEADER_SIZE) || (read_at(context, 0, head, SAVE_HEADER_SIZE) != 0))
        return save_fail(err, SVR_TooOld, "not a KFXS file; it was made before the save format changed");

    MallocBytes meta_raw;
    MallocBytes sch_raw;
    uint32_t meta_len = 0;
    uint32_t sch_len = 0;
    bool has_game = false;
    bool has_ilvl = false;
    uint32_t pos = SAVE_HEADER_SIZE;
    SaveReader reader;
    enum SaveResult result = save_reader_open(&reader, head, SAVE_HEADER_SIZE, err);
    while ((result == SVR_Ok) && (pos < file_len))
    {
        uint8_t chunk_header[SAVE_CHUNK_HEADER_SIZE];
        SaveChunk chunk;
        if (file_len - pos < SAVE_CHUNK_HEADER_SIZE)
        {
            result = save_fail(err, SVR_Damaged, "file ends inside a chunk header");
            break;
        }
        if (read_at(context, pos, chunk_header, sizeof(chunk_header)) != 0)
        {
            result = save_fail(err, SVR_Damaged, "can't read the file");
            break;
        }
        result = save_chunk_head(chunk_header, file_len - pos - SAVE_CHUNK_HEADER_SIZE, &chunk, err);
        if (result != SVR_Ok)
            break;
        if ((chunk.id == SCID_Meta) && !meta_raw)
            result = read_chunk(read_at, context, pos, &chunk, meta_raw, &meta_len, err);
        else if ((chunk.id == SCID_Schema) && !sch_raw)
            result = read_chunk(read_at, context, pos, &chunk, sch_raw, &sch_len, err);
        else if (chunk.id == SCID_Game)
            has_game = true;
        else if (chunk.id == SCID_Level)
            has_ilvl = true;
        pos += SAVE_CHUNK_HEADER_SIZE + chunk.stored_len;
    }
    if ((result == SVR_Ok) && (!meta_raw || !sch_raw || !has_game || !has_ilvl))
        result = save_fail(err, SVR_Damaged, "the save is missing a required chunk");

    if (meta_raw && sch_raw)
    {
        SaveError meta_err;
        SaveFileSchemaOwner file_schema;
        enum SaveResult meta_result = save_schema_decode(sch_raw.get(), sch_len, &file_schema, &meta_err);
        if (meta_result == SVR_Ok)
        {
            SaveDecoderOwner decoder;
            uint32_t used;
            save_decoder_init(&decoder, &file_schema, SVM_Save);
            meta_result = save_decode_record(&decoder, meta_desc, meta_raw.get(), meta_len, &used, meta, &meta_err);
        }
        if (meta_result == SVR_Ok)
            *meta_ok = 1;
        else if (result == SVR_Ok)
            result = save_fail(err, meta_result, "%s", meta_err.message);
    }
    return result;
}
