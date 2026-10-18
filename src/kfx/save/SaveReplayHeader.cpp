/******************************************************************************/
/** @file SaveReplayHeader.cpp
 *     The KFXS header of a replay (.pck) file: a small, bounded container (SCHM + META + PHDR + optionally
 *     GAME, then a zero-length TURN marker). Everything after the marker is the streamed per-turn packet
 *     codec, read and written directly against the file handle, so a long replay is never buffered whole.
 *     So the replay file is read and written through a raw TbFileHandle, not through IFileSystem.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "globals.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "replay.h"
#include "game_legacy.h"
#include "post_inc.h"

#include <cstdlib>
#include <cstring>

#include "kfx/save/SaveManager_Internal.h"
#include "kfx/save/core/save_bytes.h"
#include "kfx/save/core/schema/save_tables_decl.h"
/******************************************************************************/
enum SaveResult SaveManager::WriteReplayHeader(TbFileHandle file, const CatalogueEntry *centry,
    const PacketSaveHead *phead, const Game *game_state, SaveError *err)
{
    SaveBufferOwner schema_buffer;
    SaveBufferOwner meta;
    SaveBufferOwner phdr;
    SaveBufferOwner gamebuf;
    SaveBufferOwner out;
    const SaveStructDesc *roots4[4] = {
        &save_desc_CatalogueEntry, &save_desc_PacketSaveHead, &save_desc_Packet, &save_desc_Game };
    const SaveStructDesc *roots3[3] = {
        &save_desc_CatalogueEntry, &save_desc_PacketSaveHead, &save_desc_Packet };
    const bool with_game = (game_state != nullptr);
    enum SaveResult result = save_schema_encode(&schema_buffer, with_game ? roots4 : roots3, with_game ? 4u : 3u, SVM_Save, err);
    if (result == SVR_Ok)
        result = save_encode_record(&meta, &save_desc_CatalogueEntry, centry, SVM_Save, err);
    if (result == SVR_Ok)
        result = save_encode_record(&phdr, &save_desc_PacketSaveHead, phead, SVM_Save, err);
    if ((result == SVR_Ok) && with_game)
        result = save_encode_record(&gamebuf, &save_desc_Game, game_state, SVM_Save, err);
    if (result != SVR_Ok)
        return result;

    // The container: the header, then the chunks in this order, ended by the empty turn marker.
    unsigned char empty = 0;
    bool succeeded = (save_file_begin(&out, SFK_Replay) == 0);
    auto add = [&](uint32_t id, const SaveBuffer &payload, bool compress) {
        succeeded = succeeded && (save_chunk_add_buffer(&out, id, SCF_Required, &payload, compress) == 0);
    };
    add(SCID_Schema, schema_buffer, true);
    add(SCID_Meta, meta, false);
    add(SCID_PacketHead, phdr, false);
    if (with_game)
        add(SCID_Game, gamebuf, true);
    succeeded = succeeded && (save_chunk_add(&out, SCID_TurnData, SCF_Required, &empty, 0, 0) == 0);
    if (!succeeded)
        return save_fail(err, SVR_NoMemory, "out of memory");
    if (LbFileWrite(file, out.data, out.len) != static_cast<long>(out.len))
        return save_fail(err, SVR_Damaged, "can't write replay header");
    return SVR_Ok;
}
/******************************************************************************/
/* Reads one KFXS chunk directly off an open file handle: a chunk header (id/flags/stored_len/raw_len/crc32,
   the same layout save_reader_next parses from memory) followed by stored_len bytes, decompressed via the
   shared codec. Used instead of save_reader_open/save_reader_next, which need the whole file in memory. */
static enum SaveResult read_one_chunk(TbFileHandle file, uint32_t &id, uint32_t &flags, savebytes::MallocBytes &data,
    uint32_t &len, SaveError *err)
{
    uint8_t hdrbuf[SAVE_CHUNK_HEADER_SIZE];
    if (LbFileRead(file, hdrbuf, sizeof(hdrbuf)) != static_cast<int>(sizeof(hdrbuf)))
        return save_fail(err, SVR_Damaged, "replay file ends inside a chunk header");
    SaveChunk chunk;
    chunk.id = savebytes::LoadLE<uint32_t>(hdrbuf);
    chunk.flags = savebytes::LoadLE<uint32_t>(hdrbuf + 4);
    chunk.stored_len = savebytes::LoadLE<uint32_t>(hdrbuf + 8);
    chunk.raw_len = savebytes::LoadLE<uint32_t>(hdrbuf + 12);
    chunk.crc32 = savebytes::LoadLE<uint32_t>(hdrbuf + 16);
    if (chunk.raw_len > SAVE_MAX_RAW_LEN)
        return save_fail(err, SVR_Damaged, "replay chunk is too large");
    std::vector<uint8_t> stored(chunk.stored_len ? chunk.stored_len : 1);
    if ((chunk.stored_len > 0) && (LbFileRead(file, stored.data(), chunk.stored_len) != static_cast<int>(chunk.stored_len)))
        return save_fail(err, SVR_Damaged, "replay file ends inside a chunk body");
    chunk.body = stored.data();
    uint8_t *raw = nullptr;
    const enum SaveResult result = save_chunk_unpack(&chunk, &raw, &len, err);
    if (result != SVR_Ok)
        return result;
    id = chunk.id;
    flags = chunk.flags;
    data.reset(raw);
    return SVR_Ok;
}

static enum SaveResult fail_freeing_decoder(SaveDecoder *decoder, SaveFileSchema *schema, enum SaveResult result)
{
    save_decoder_free(decoder);
    save_schema_free(schema);
    return result;
}

enum SaveResult SaveManager::ReadReplayHeader(TbFileHandle file, CatalogueEntry *centry, PacketSaveHead *phead,
    Game *game_state, bool *g_present, SaveFileSchema *out_schema, SaveDecoder *out_dec, SaveError *err)
{
    *g_present = false;
    memset(out_schema, 0, sizeof(*out_schema));
    uint8_t filehdr[SAVE_HEADER_SIZE];
    if (LbFileRead(file, filehdr, sizeof(filehdr)) != static_cast<int>(sizeof(filehdr)))
        return save_fail(err, SVR_TooOld, "not a KFXS file; it was made before the save format changed");
    SaveReader reader;
    enum SaveResult result = save_reader_open(&reader, filehdr, sizeof(filehdr), err);
    if (result != SVR_Ok)
        return result;
    if (reader.kind != SFK_Replay)
        return save_fail(err, SVR_Damaged, "not a replay file");

    bool have_schema = false;
    bool have_meta = false;
    bool have_phdr = false;
    for (bool done = false; !done;)
    {
        uint32_t id = 0;
        uint32_t len = 0;
        uint32_t flags = 0;
        savebytes::MallocBytes data;
        result = read_one_chunk(file, id, flags, data, len, err);
        if (result != SVR_Ok)
            return have_schema ? fail_freeing_decoder(out_dec, out_schema, result) : result;
        switch (id)
        {
        case SCID_Schema:
            result = save_schema_decode(data.get(), len, out_schema, err);
            if (result != SVR_Ok)
                return result;
            have_schema = true;
            save_decoder_init(out_dec, out_schema, SVM_Save);
            break;
        case SCID_Meta:
        case SCID_PacketHead:
        case SCID_Game:
        {
            if (!have_schema)
                return save_fail(err, SVR_Damaged, "replay chunk before its schema");
            const SaveStructDesc *desc = (id == SCID_Meta) ? &save_desc_CatalogueEntry
                : (id == SCID_PacketHead) ? &save_desc_PacketSaveHead : &save_desc_Game;
            void *dst = (id == SCID_Meta) ? static_cast<void *>(centry) : (id == SCID_PacketHead) ? static_cast<void *>(phead) : static_cast<void *>(game_state);
            memset(dst, 0, (id == SCID_Meta) ? sizeof(*centry) : (id == SCID_PacketHead) ? sizeof(*phead) : sizeof(*game_state));
            uint32_t used;
            save_decoder_set_chunk_flags(out_dec, flags);
            result = save_decode_record(out_dec, desc, data.get(), len, &used, dst, err);
            if (result != SVR_Ok)
                return fail_freeing_decoder(out_dec, out_schema, result);
            if (id == SCID_Meta) have_meta = true;
            else if (id == SCID_PacketHead) have_phdr = true;
            else *g_present = true;
            break;
        }
        case SCID_TurnData:
            done = true;
            break;
        default:
            break;
        }
    }
    if (!have_schema || !have_meta || !have_phdr)
    {
        save_fail(err, SVR_Damaged, "replay file is missing a required chunk");
        return fail_freeing_decoder(out_dec, out_schema, SVR_Damaged);
    }
    return SVR_Ok;
}
/******************************************************************************/
extern "C" enum SaveResult SaveManager_WriteReplayHeader(TbFileHandle file, const struct CatalogueEntry *centry,
    const struct PacketSaveHead *phead, const struct Game *game_state, struct SaveError *err)
{
    return SaveManager::WriteReplayHeader(file, centry, phead, game_state, err);
}

extern "C" enum SaveResult SaveManager_ReadReplayHeader(TbFileHandle file, struct CatalogueEntry *centry,
    struct PacketSaveHead *phead, struct Game *game_state, TbBool *g_present, struct SaveFileSchema *out_schema,
    struct SaveDecoder *out_dec, struct SaveError *err)
{
    bool present = false;
    const enum SaveResult result = SaveManager::ReadReplayHeader(file, centry, phead, game_state, &present, out_schema, out_dec, err);
    *g_present = present;
    return result;
}
