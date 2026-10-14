/******************************************************************************/
/** @file SaveManager_Internal.h
 *     Shared by the files that make up the save manager. Not for use outside src/kfx/save.
 */
/******************************************************************************/
#ifndef KFX_SAVEMANAGER_INTERNAL_H
#define KFX_SAVEMANAGER_INTERNAL_H

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <vector>

#include "kfx/platform/IFileSystem.h"
#include "kfx/save/SaveManager.h"
#include "kfx/save/core/save_schema.h"
#include "kfx/save/core/schema/save_tables_decl.h"

/** Log lines of the save manager carry one prefix, like the [display] lines, so they can be found together. */
#define SAVELOG(format, ...) LbJustLog("[save-manager] " format "\n", ##__VA_ARGS__)
#define SAVEWARN(format, ...) LbWarnLog("[save-manager] " format "\n", ##__VA_ARGS__)
#define SAVEERR(format, ...) LbErrorLog("[save-manager] " format "\n", ##__VA_ARGS__)

#define SAVED_GAME_FILENAME "fx1g%04d.sav"

struct Game;
struct IntralevelData;
struct Configs;
struct ConfigInfo;
struct HighScore;

namespace savefile {

/** Reads a whole file. A missing file is SVR_NotFound, anything else that goes wrong SVR_Damaged. */
enum SaveResult ReadFile(IFileSystem &filesystem, const char *fname, std::vector<uint8_t> &out, SaveError *err);

/** Writes data as the whole file, replacing any file there. Failures are logged. */
bool WriteFile(IFileSystem &filesystem, const char *fname, const void *data, size_t len);

/** The file kind and field table of each type kept in a file of records. */
template <typename T> struct RecordTraits;
template <> struct RecordTraits<ConfigInfo> {
    static constexpr uint32_t kind = SFK_NetConfig;
    static const SaveStructDesc &Desc() { return save_desc_ConfigInfo; }
};
template <> struct RecordTraits<HighScore> {
    static constexpr uint32_t kind = SFK_HighScore;
    static const SaveStructDesc &Desc() { return save_desc_HighScore; }
};
template <> struct RecordTraits<IntralevelData> {
    static constexpr uint32_t kind = SFK_Continue;
    static const SaveStructDesc &Desc() { return save_desc_IntralevelData; }
};
template <> struct RecordTraits<ContinueData> {
    static constexpr uint32_t kind = SFK_Continue;
    static const SaveStructDesc &Desc() { return save_desc_ContinueData; }
};

/** The records of a file made by WriteRecords. out is empty when the file holds none and on any error. */
template <typename T>
enum SaveResult ReadRecords(IFileSystem &filesystem, const char *fname, std::vector<T> &out, SaveError *err);

/** A file that holds exactly one record. record is untouched on any failure. */
template <typename T>
enum SaveResult ReadRecord(IFileSystem &filesystem, const char *fname, T &record, SaveError *err);

template <typename T>
bool WriteRecords(IFileSystem &filesystem, const char *fname, const T *records, uint32_t count);
template <typename T>
bool WriteRecord(IFileSystem &filesystem, const char *fname, const T &record) { return WriteRecords(filesystem, fname, &record, 1); }

namespace detail {
/** The same without the types. The record array of ReadRecords is malloc'd; it is NULL when empty or on any error. */
enum SaveResult ReadRecords(IFileSystem &filesystem, const char *fname, uint32_t kind, const SaveStructDesc *desc,
    uint32_t elem_size, void **records, uint32_t *count, SaveError *err);
bool WriteRecords(IFileSystem &filesystem, const char *fname, uint32_t kind, const SaveStructDesc *desc,
    const void *records, uint32_t elem_size, uint32_t count);
} // namespace detail

template <typename T>
enum SaveResult ReadRecords(IFileSystem &filesystem, const char *fname, std::vector<T> &out, SaveError *err)
{
    static_assert(std::is_trivially_copyable<T>::value, "records are read and written as plain memory");
    out.clear();
    void *records = nullptr;
    uint32_t count = 0;
    enum SaveResult result = detail::ReadRecords(filesystem, fname, RecordTraits<T>::kind, &RecordTraits<T>::Desc(), sizeof(T),
        &records, &count, err);
    if (result == SVR_Ok && count > 0)
        out.assign(static_cast<const T *>(records), static_cast<const T *>(records) + count);
    free(records);
    return result;
}

template <typename T>
enum SaveResult ReadRecord(IFileSystem &filesystem, const char *fname, T &record, SaveError *err)
{
    std::vector<T> records;
    enum SaveResult result = ReadRecords(filesystem, fname, records, err);
    if (result != SVR_Ok)
        return result;
    if (records.size() != 1)
        return save_fail(err, SVR_Damaged, "\"%s\" holds %u records, expected one", fname, (unsigned)records.size());
    record = records[0];
    return SVR_Ok;
}

template <typename T>
bool WriteRecords(IFileSystem &filesystem, const char *fname, const T *records, uint32_t count)
{
    return detail::WriteRecords(filesystem, fname, RecordTraits<T>::kind, &RecordTraits<T>::Desc(), records, sizeof(T), count);
}

} // namespace savefile

/** Savegame files (SaveGameFile.cpp): the KFXS container built from and read into the game state. */
namespace savegame {

/** The raw NAMS/CONF/LUA chunk bytes of a read save, for the caller to use after its own config loading:
 *  they only make sense once game.conf is loaded fresh. Free with FreeReadResult. */
struct ReadResult {
    uint8_t *nams_data; uint32_t nams_len;
    uint8_t *conf_data; uint32_t conf_len;
    char *lua_data; uint32_t lua_len;
};

/** Encodes a full KFXS buffer (META + SCHM + NAMS + CONF + GAME + ILVL + LUA). baseline may be NULL (no
 *  level set up yet); the CONF chunk is then an empty overlay. */
enum SaveResult Build(const Game *game_state, const IntralevelData *ilvl, const Configs *baseline,
    const char *lua_data, uint32_t lua_len, const CatalogueEntry *centry, SaveBuffer *out, SaveError *err);

/** Builds the save from the running game and writes it. */
bool Write(IFileSystem &filesystem, const char *fname, const CatalogueEntry *centry);

/** Reads the file header, the chunk headers and the title, not the rest. *title_ok says whether *centry was
 *  filled in, which can be so even when the result is an error. */
enum SaveResult Inspect(IFileSystem &filesystem, const char *fname, CatalogueEntry *centry, int *title_ok, SaveError *err);

/** Reads and decodes a save the way a load does, every chunk checksum-checked, into scratch memory. */
enum SaveResult Verify(IFileSystem &filesystem, const char *fname, SaveError *err);

/** Decodes META into *centry and GAME/ILVL into g/ilvl (both zeroed first). conf is not part of GAME, so
 *  g->conf comes back zeroed. */
enum SaveResult Read(IFileSystem &filesystem, const char *fname, CatalogueEntry *centry, Game *game_state, IntralevelData *ilvl,
    ReadResult *out, SaveError *err);
void FreeReadResult(ReadResult *out);

/** Maps a codec result to what the menus need to know: can this build load the file. */
enum SaveCheckResult Classify(enum SaveResult result);

/** Checks one savegame file without loading it. *title_ok says whether centry (may be NULL) was filled in,
 *  which can be so even for a save that can't be loaded. detail (may be NULL) gets the reason in plain text. */
enum SaveCheckResult CheckFile(IFileSystem &filesystem, const char *fname, CatalogueEntry *centry, int *title_ok,
    char *detail, size_t detail_len);

/** Slot number of a file named fx1gNNNN.sav, or -1. */
int SlotFromFilename(const char *fname);

/** Reads the title of a savegame written before the KFXS container, into *centry. */
bool ReadLegacyTitle(IFileSystem &filesystem, const char *fname, CatalogueEntry *centry);

} // namespace savegame

#endif
