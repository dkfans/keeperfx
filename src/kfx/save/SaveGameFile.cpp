/******************************************************************************/
/** @file SaveGameFile.cpp
 *     Savegame files: the KFXS container built from and read into the game state.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_basics.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "kfx/platform/IPlatform.h"
#include "lua_base.h"
#include "post_inc.h"

#include <cstdlib>
#include <cstring>

#include "kfx/save/SaveManager_Internal.h"
#include "kfx/save/core/save_bytes.h"
#include "kfx/save/core/save_inspect.h"
#include "kfx/save/core/save_migrate.h"
#include "kfx/save/core/save_config_overlay.h"
#include "kfx/save/core/save_names.h"
#include "kfx/save/core/save_schema.h"
#include "kfx/save/core/save_tables.h"
#include "kfx/save/core/schema/save_tables_decl.h"

namespace savegame {

enum SaveResult Build(const Game *game_state, const IntralevelData *ilvl, const Configs *baseline,
    const char *lua_data, uint32_t lua_len, const CatalogueEntry *centry, SaveBuffer *out, SaveError *err)
{
    SaveBufferOwner meta, schema_buffer, nams, conf, gamebuf, ilvlbuf;
    uint32_t root_count;
    const SaveStructDesc *const *roots = save_root_structs(&root_count);
    enum SaveResult result = save_encode_record(&meta, &save_desc_CatalogueEntry, centry, SVM_Save, err);
    if (result == SVR_Ok)
        result = save_schema_encode(&schema_buffer, roots, root_count, SVM_Save, err);
    if (result == SVR_Ok)
        result = save_names_write(game_state, &nams, err);
    if (result == SVR_Ok)
    {
        if (baseline != nullptr)
            result = save_config_overlay_write(&game_state->conf, baseline, &conf, err);
        else if (save_buf_u32(&conf, 0) != 0) /* no level set up yet: an overlay with no entries */
            result = save_fail(err, SVR_NoMemory, "out of memory");
    }
    if (result == SVR_Ok)
        result = save_encode_record(&gamebuf, &save_desc_Game, game_state, SVM_Save, err);
    if (result == SVR_Ok)
        result = save_encode_record(&ilvlbuf, &save_desc_IntralevelData, ilvl, SVM_Save, err);

    // The container: the header, then the chunks in this order.
    bool succeeded = (result == SVR_Ok) && (save_file_begin(out, SFK_Save) == 0);
    auto add = [&](uint32_t id, const SaveBuffer &payload, bool compress) {
        succeeded = succeeded && (save_chunk_add_buffer(out, id, SCF_Required, &payload, compress) == 0);
    };
    add(SCID_Meta, meta, false);
    add(SCID_Schema, schema_buffer, true);
    add(SCID_Names, nams, true);
    add(SCID_Config, conf, true);
    add(SCID_Game, gamebuf, true);
    add(SCID_Level, ilvlbuf, true);
    if (lua_len > 0)
        succeeded = succeeded && (save_chunk_add(out, SCID_Lua, 0, lua_data, lua_len, true) == 0);
    if ((result == SVR_Ok) && !succeeded)
        result = save_fail(err, SVR_NoMemory, "out of memory");
    if (result != SVR_Ok)
        save_buf_free(out);
    return result;
}

bool Write(IFileSystem &filesystem, const char *fname, const CatalogueEntry *centry)
{
    SaveBufferOwner out;
    SaveError err;
    size_t lua_len = 0;
    const char *lua_data = lua_get_serialised_data(&lua_len);
    enum SaveResult result = Build(&game, &intralvl, conf_baseline, lua_data, (uint32_t)lua_len, centry, &out, &err);
    cleanup_serialized_data();
    if (result != SVR_Ok)
    {
        SAVEWARN("can't build the save: %s", err.message);
        return false;
    }
    return savefile::WriteFile(filesystem, fname, out.data, out.len);
}

/******************************************************************************/
namespace {

/** One unpacked chunk of a save, and whether the file had it. */
struct RawChunk {
    savebytes::MallocBytes data;
    uint32_t len = 0;
    uint32_t flags = 0;     /* the chunk's flags in the file */
    bool present = false;
};

struct RawChunks {
    RawChunk meta, schema, nams, conf, game, ilvl, lua;
    uint16_t minor = 0;     /* the file's schema revision */
};

RawChunk *chunk_for(RawChunks &chunks, uint32_t id)
{
    switch (id)
    {
    case SCID_Meta:   return &chunks.meta;
    case SCID_Schema: return &chunks.schema;
    case SCID_Names:  return &chunks.nams;
    case SCID_Config: return &chunks.conf;
    case SCID_Game:   return &chunks.game;
    case SCID_Level:  return &chunks.ilvl;
    case SCID_Lua:    return &chunks.lua;
    default:          return nullptr;   /* a chunk from a newer build that isn't required: skipped */
    }
}

enum SaveResult read_raw_chunks(const uint8_t *filebuf, uint32_t filelen, RawChunks &chunks, SaveError *err)
{
    SaveReader reader;
    enum SaveResult result = save_reader_open(&reader, filebuf, filelen, err);
    if (result != SVR_Ok)
        return result;
    chunks.minor = reader.minor;
    SaveChunk chunk;
    while ((result = save_reader_next(&reader, &chunk, err)) == SVR_Ok)
    {
        RawChunk *dst = chunk_for(chunks, chunk.id);
        if (dst == nullptr)
            continue;
        uint8_t *raw = nullptr;
        result = save_chunk_unpack(&chunk, &raw, &dst->len, err);
        dst->data.reset(raw);
        if (result != SVR_Ok)
            return result;
        dst->flags = chunk.flags;
        dst->present = true;
    }
    return (result == SVR_NotFound) ? SVR_Ok : result;   /* SVR_NotFound from save_reader_next is a clean end of file */
}

int reader_read_at(void *context, uint32_t offset, void *dst, uint32_t len)
{
    return static_cast<IFileReader *>(context)->ReadAt(offset, dst, len) ? 0 : -1;
}

} // namespace

enum SaveResult Inspect(IFileSystem &filesystem, const char *fname, CatalogueEntry *centry, int *title_ok, SaveError *err)
{
    *title_ok = 0;
    std::unique_ptr<IFileReader> reader = filesystem.OpenReader(fname);
    if (!reader)
        return save_fail(err, SVR_NotFound, "can't open \"%s\"", fname);
    return save_inspect_game(reader_read_at, reader.get(), reader->Size(), &save_desc_CatalogueEntry, centry,
        title_ok, err);
}

enum SaveResult Verify(IFileSystem &filesystem, const char *fname, SaveError *err)
{
    std::unique_ptr<Game, savebytes::FreeDeleter> game_state(static_cast<Game *>(calloc(1, sizeof(Game))));
    std::unique_ptr<IntralevelData, savebytes::FreeDeleter> ilvl(static_cast<IntralevelData *>(calloc(1, sizeof(IntralevelData))));
    if (!game_state || !ilvl)
        return save_fail(err, SVR_NoMemory, "out of memory");
    CatalogueEntry centry;
    ReadResult read_result;
    enum SaveResult result = Read(filesystem, fname, &centry, game_state.get(), ilvl.get(), &read_result, err);
    FreeReadResult(&read_result);
    return result;
}

enum SaveResult Read(IFileSystem &filesystem, const char *fname, CatalogueEntry *centry, Game *game_state, IntralevelData *ilvl,
    ReadResult *out, SaveError *err)
{
    memset(out, 0, sizeof(*out));
    std::vector<uint8_t> filebuf;
    enum SaveResult result = savefile::ReadFile(filesystem, fname, filebuf, err);
    if (result != SVR_Ok)
        return result;
    RawChunks chunks;
    result = read_raw_chunks(filebuf.data(), (uint32_t)filebuf.size(), chunks, err);
    if (result != SVR_Ok)
        return result;
    if (!chunks.meta.present || !chunks.schema.present || !chunks.game.present || !chunks.ilvl.present)
        return save_fail(err, SVR_Damaged, "the save is missing a required chunk");

    SaveFileSchemaOwner file_schema;
    result = save_schema_decode(chunks.schema.data.get(), chunks.schema.len, &file_schema, err);
    if (result != SVR_Ok)
        return result;
    {
        SaveDecoderOwner decoder;
        save_decoder_init(&decoder, &file_schema, SVM_Save);
        uint32_t used;
        memset(centry, 0, sizeof(*centry));
        // cppcheck-suppress memsetClassFloat
        memset(game_state, 0, sizeof(*game_state));       /* conf is SVF_RUNTIME: stays zero, caller loads it fresh */
        memset(ilvl, 0, sizeof(*ilvl));
        save_decoder_set_chunk_flags(&decoder, chunks.meta.flags);
        result = save_decode_record(&decoder, &save_desc_CatalogueEntry, chunks.meta.data.get(), chunks.meta.len, &used, centry, err);
        if (result == SVR_Ok)
        {
            save_decoder_set_chunk_flags(&decoder, chunks.game.flags);
            result = save_decode_record(&decoder, &save_desc_Game, chunks.game.data.get(), chunks.game.len, &used, game_state, err);
        }
        if (result == SVR_Ok)
        {
            save_decoder_set_chunk_flags(&decoder, chunks.ilvl.flags);
            result = save_decode_record(&decoder, &save_desc_IntralevelData, chunks.ilvl.data.get(), chunks.ilvl.len, &used, ilvl, err);
        }
        if (result == SVR_Ok)
        {
            SaveLoadCtx context = { game_state, &decoder, chunks.minor };
            result = save_migrations_run(&context, "Game", err);
            context.state = ilvl;
            if (result == SVR_Ok)
                result = save_migrations_run(&context, "IntralevelData", err);
        }
    }
    if (result == SVR_Ok)
    {
        /* Hand the raw NAMS/CONF/LUA bytes to the caller: applying them needs
           config-loading side effects only the load knows how to sequence. */
        out->nams_data = chunks.nams.data.release(); out->nams_len = chunks.nams.len;
        out->conf_data = chunks.conf.data.release(); out->conf_len = chunks.conf.len;
        out->lua_data = reinterpret_cast<char *>(chunks.lua.data.release()); out->lua_len = chunks.lua.len;
    }
    return result;
}

void FreeReadResult(ReadResult *out)
{
    free(out->nams_data);
    free(out->conf_data);
    free(out->lua_data);
    memset(out, 0, sizeof(*out));
}

/******************************************************************************/

enum SaveCheckResult Classify(enum SaveResult result)
{
    switch (result)
    {
    case SVR_Ok:
        return SvChk_Loadable;
    case SVR_TooNew:
    case SVR_TooOld:
    case SVR_Unsupported:
        return SvChk_Incompatible;
    default:
        return SvChk_Damaged;
    }
}

static void set_detail(char *detail, size_t detail_len, const char *text)
{
    if ((detail != nullptr) && (detail_len > 0))
        snprintf(detail, detail_len, "%s", text);
}

bool ReadLegacyTitle(IFileSystem &filesystem, const char *fname, CatalogueEntry *centry)
{
    std::unique_ptr<IFileReader> reader = filesystem.OpenReader(fname);
    FileChunkHeader header;
    if (!reader || !reader->ReadAt(0, &header, sizeof(header)))
        return false;
    clear_flag(centry->flags, CEF_InUse);
    if ((header.id == SGC_InfoBlock) && (header.len == sizeof(CatalogueEntry)) && reader->ReadAt(sizeof(header), centry, sizeof(CatalogueEntry)))
        set_flag(centry->flags, CEF_InUse);
    centry->textname[SAVE_TEXTNAME_LEN-1] = '\0';
    centry->campaign_name[LINEMSG_SIZE-1] = '\0';
    centry->campaign_fname[DISKPATH_SIZE-1] = '\0';
    centry->player_name[PLAYER_NAME_LENGTH-1] = '\0';
    return ((centry->flags & CEF_InUse) != 0);
}

enum SaveCheckResult CheckFile(IFileSystem &filesystem, const char *fname, CatalogueEntry *centry, int *title_ok,
    char *detail, size_t detail_len)
{
    *title_ok = 0;
    set_detail(detail, detail_len, "");
    std::unique_ptr<IFileReader> reader = filesystem.OpenReader(fname);
    if (!reader)
    {
        set_detail(detail, detail_len, "no such file");
        return SvChk_Missing;
    }
    char magic[4];
    bool is_kfxs = reader->ReadAt(0, magic, sizeof(magic)) && (memcmp(magic, "KFXS", sizeof(magic)) == 0);
    reader.reset();
    if (!is_kfxs)
    {
        // Written before the KFXS container. It is listed by the title it carries, if it has one, but can't be loaded.
        if ((centry != nullptr) && ReadLegacyTitle(filesystem, fname, centry))
            *title_ok = 1;
        set_detail(detail, detail_len, "made before the save format changed");
        return SvChk_Incompatible;
    }
    SaveError err;
    CatalogueEntry unused_title;
    if (centry == nullptr)
    {
        memset(&unused_title, 0, sizeof(unused_title));
        centry = &unused_title;
    }
    enum SaveResult result = Inspect(filesystem, fname, centry, title_ok, &err);
    if (*title_ok)
        centry->textname[SAVE_TEXTNAME_LEN-1] = '\0';
    if (result != SVR_Ok)
        set_detail(detail, detail_len, err.message);
    return Classify(result);
}

int SlotFromFilename(const char *fname)
{
    if (strncasecmp(fname, "fx1g", 4) != 0)
        return -1;
    const char *text = fname + 4;
    if ((*text < '0') || (*text > '9'))
        return -1;
    long index = atol(text);
    while ((*text >= '0') && (*text <= '9'))
        text++;
    if (strcasecmp(text, ".sav") != 0)
        return -1;
    if ((index < 0) || (index >= SAVE_SLOTS_LIMIT))
        return -1;
    return (int)index;
}

} // namespace savegame
