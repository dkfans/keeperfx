/******************************************************************************/
/** @file SaveCampaignProgress.cpp
 *     The per-campaign progress file through an IFileSystem. Uses nothing of the engine but the log
 *     functions, so it is tested standalone.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "game_merge.h"
#include "post_inc.h"

#include "kfx/save/SaveManager_Internal.h"

#include <cstring>

#include "kfx/save/core/save_bytes.h"

namespace campaignprogress {

/* Old format (alphas 1.4.0.5523~1.4.0.5589? [TBD/fixme after merge]): SCHM + RECS
 * New format: SCHM + ILVL [+ CILD] */
static enum SaveResult parse(const std::vector<uint8_t> &file, IntralevelData *ilvl, std::vector<uint8_t> *custom, SaveError *err)
{
    SaveReader reader;
    enum SaveResult result = save_reader_open(&reader, file.data(), static_cast<uint32_t>(file.size()), err);
    if (result != SVR_Ok)
        return result;
    if (reader.kind != SFK_Continue)
        return save_fail(err, SVR_Damaged, "this is not the kind of file expected here");
    savebytes::MallocBytes schema_bytes;
    savebytes::MallocBytes ilvl_bytes;
    savebytes::MallocBytes custom_bytes;
    uint32_t schema_len = 0;
    uint32_t ilvl_len = 0;
    uint32_t ilvl_flags = 0;
    uint32_t custom_len = 0;
    SaveChunk chunk;
    while ((result = save_reader_next(&reader, &chunk, err)) == SVR_Ok)
    {
        uint8_t *raw = nullptr;
        switch (chunk.id)
        {
        case SCID_Schema:
            result = save_chunk_unpack(&chunk, &raw, &schema_len, err);
            schema_bytes.reset(raw);
            break;
        case SCID_Level:
        case SCID_Records:
            result = save_chunk_unpack(&chunk, &raw, &ilvl_len, err);
            ilvl_bytes.reset(raw);
            ilvl_flags = chunk.flags;
            break;
        case SCID_CustomIntralevelData:
            result = save_chunk_unpack(&chunk, &raw, &custom_len, err);
            custom_bytes.reset(raw);
            break;
        }
        if (result != SVR_Ok)
            return result;
    }
    if (result != SVR_NotFound)
        return result;
    if (!schema_bytes || !ilvl_bytes)
        return save_fail(err, SVR_Damaged, "the file is missing a required chunk");
    SaveFileSchemaOwner file_schema;
    result = save_schema_decode(schema_bytes.get(), schema_len, &file_schema, err);
    if (result != SVR_Ok)
        return result;
    SaveDecoderOwner decoder;
    save_decoder_init(&decoder, &file_schema, SVM_Save);
    save_decoder_set_chunk_flags(&decoder, ilvl_flags);
    IntralevelData decoded;
    memset(&decoded, 0, sizeof(decoded));
    uint32_t used = 0;
    result = save_decode_record(&decoder, &save_desc_IntralevelData, ilvl_bytes.get(), ilvl_len, &used, &decoded, err);
    if (result != SVR_Ok)
        return result;
    memcpy(ilvl, &decoded, sizeof(decoded));
    if (custom != nullptr)
        custom->assign(custom_bytes.get(), custom_bytes.get() + custom_len);
    return SVR_Ok;
}

enum SaveResult Read(IFileSystem &filesystem, const char *fname, IntralevelData *ilvl, std::vector<uint8_t> *custom, SaveError *err)
{
    std::vector<uint8_t> file;
    const enum SaveResult result = savefile::ReadFile(filesystem, fname, file, err);
    if (result != SVR_Ok)
        return result;
    return parse(file, ilvl, custom, err);
}

bool Write(IFileSystem &filesystem, const char *fname, const IntralevelData *ilvl, const void *custom, uint32_t custom_len)
{
    SaveBufferOwner schema_buffer;
    SaveBufferOwner ilvl_buffer;
    SaveBufferOwner out;
    SaveError err;
    const SaveStructDesc *root = &save_desc_IntralevelData;
    enum SaveResult result = save_schema_encode(&schema_buffer, &root, 1, SVM_Save, &err);
    if (result == SVR_Ok)
        result = save_encode_record(&ilvl_buffer, &save_desc_IntralevelData, ilvl, SVM_Save, &err);
    if (result != SVR_Ok)
    {
        SAVEWARN("can't build \"%s\": %s", fname, err.message);
        return false;
    }
    if (save_file_begin(&out, SFK_Continue) != 0
     || save_chunk_add_buffer(&out, SCID_Schema, SCF_Required, &schema_buffer, 1) != 0
     || save_chunk_add_buffer(&out, SCID_Level, SCF_Required, &ilvl_buffer, 1) != 0
     || (custom_len > 0 && save_chunk_add(&out, SCID_CustomIntralevelData, 0, custom, custom_len, false) != 0))
    {
        SAVEWARN("can't build \"%s\": out of memory", fname);
        return false;
    }
    return savefile::WriteFile(filesystem, fname, out.data, out.len);
}

} // namespace campaignprogress
