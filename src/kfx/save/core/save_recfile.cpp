/******************************************************************************/
/** @file save_recfile.cpp
 *     Files that hold a run of records of one type: a schema chunk and a records chunk.
 */
/******************************************************************************/
#include "save_recfile.h"
#include "save_bytes.h"

#include <cstdlib>
#include <cstring>
#include <vector>

using savebytes::MallocBytes;
/******************************************************************************/
enum SaveResult save_recfile_build(struct SaveBuffer *out, uint32_t kind, const struct SaveStructDesc *desc,
    const void *records, uint32_t elem_size, uint32_t count, struct SaveError *err)
{
    SaveBufferOwner schema_buffer;
    SaveBufferOwner recs;
    const SaveStructDesc *roots[1] = { desc };
    enum SaveResult result = save_schema_encode(&schema_buffer, roots, 1, SVM_Save, err);
    for (uint32_t i = 0; (result == SVR_Ok) && (i < count); i++)
        result = save_encode_record(&recs, desc, static_cast<const uint8_t *>(records) + (static_cast<size_t>(i) * elem_size), SVM_Save, err);
    if ((result == SVR_Ok) && ((save_file_begin(out, kind) != 0) ||
            (save_chunk_add_buffer(out, SCID_Schema, SCF_Required, &schema_buffer, 1) != 0) ||
            (save_chunk_add_buffer(out, SCID_Records, SCF_Required, &recs, 1) != 0)))
        result = save_fail(err, SVR_NoMemory, "out of memory");
    if (result != SVR_Ok)
        save_buf_free(out);
    return result;
}

/** Decodes records back to back until the chunk's bytes run out: the count isn't stored, because a
 *  record's own decoded length already tells where the next one starts. */
static enum SaveResult decode_records(const uint8_t *sch_data, uint32_t sch_len, const uint8_t *recs, uint32_t recs_len,
    const SaveStructDesc *desc, uint32_t elem_size, std::vector<uint8_t> &array, uint32_t *count, SaveError *err)
{
    SaveFileSchemaOwner file_schema;
    enum SaveResult result = save_schema_decode(sch_data, sch_len, &file_schema, err);
    if (result != SVR_Ok)
        return result;
    SaveDecoderOwner decoder;
    save_decoder_init(&decoder, &file_schema, SVM_Save);
    uint32_t record_count = 0;
    uint32_t pos = 0;
    while (pos < recs_len)
    {
        uint32_t used = 0;
        array.resize(static_cast<size_t>(record_count + 1) * elem_size);
        uint8_t *record = array.data() + (static_cast<size_t>(record_count) * elem_size);
        memset(record, 0, elem_size);
        result = save_decode_record(&decoder, desc, recs + pos, recs_len - pos, &used, record, err);
        if ((result == SVR_Ok) && (used == 0))
            result = save_fail(err, SVR_Damaged, "a record is empty");
        if (result != SVR_Ok)
            return result;
        pos += used;
        record_count++;
    }
    *count = record_count;
    return SVR_Ok;
}

enum SaveResult save_recfile_parse(const uint8_t *file, uint32_t len, uint32_t kind, const struct SaveStructDesc *desc,
    uint32_t elem_size, void **records, uint32_t *count, struct SaveError *err)
{
    *records = nullptr;
    *count = 0;
    SaveReader reader;
    enum SaveResult result = save_reader_open(&reader, file, len, err);
    if (result != SVR_Ok)
        return result;
    if (reader.kind != kind)
        return save_fail(err, SVR_Damaged, "this is not the kind of file expected here");

    MallocBytes schema_bytes;
    MallocBytes recs;
    uint32_t sch_len = 0;
    uint32_t recs_len = 0;
    for (;;)
    {
        SaveChunk chunk;
        result = save_reader_next(&reader, &chunk, err);
        if (result == SVR_NotFound) /* a clean end of file */
            break;
        if (result != SVR_Ok)
            return result;
        uint8_t *raw = nullptr;
        if (chunk.id == SCID_Schema)
        {
            result = save_chunk_unpack(&chunk, &raw, &sch_len, err);
            schema_bytes.reset(raw);
        } else if (chunk.id == SCID_Records)
        {
            result = save_chunk_unpack(&chunk, &raw, &recs_len, err);
            recs.reset(raw);
        }
        if (result != SVR_Ok)
            return result;
    }
    if (!schema_bytes || !recs)
        return save_fail(err, SVR_Damaged, "the file is missing a required chunk");

    std::vector<uint8_t> array;
    result = decode_records(schema_bytes.get(), sch_len, recs.get(), recs_len, desc, elem_size, array, count, err);
    if ((result != SVR_Ok) || (*count == 0))
    {
        *count = 0;
        return result;
    }
    *records = malloc(array.size());
    if (*records == nullptr)
    {
        *count = 0;
        return save_fail(err, SVR_NoMemory, "out of memory");
    }
    memcpy(*records, array.data(), array.size());
    return SVR_Ok;
}
