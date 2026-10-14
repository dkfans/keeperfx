/******************************************************************************/
/** @file SaveRecordFiles.cpp
 *     Reading and writing whole files and files of records through an IFileSystem. Uses nothing of the
 *     engine but the log functions, so it is tested standalone.
 */
/******************************************************************************/
#include "kfx/save/SaveManager_Internal.h"

#include <cstdlib>
#include <cstring>

#include "kfx/save/core/save_recfile.h"

namespace savefile {

enum SaveResult ReadFile(IFileSystem &filesystem, const char *fname, std::vector<uint8_t> &out, SaveError *err)
{
    switch (filesystem.ReadAll(fname, out)) {
    case FsStatus::Ok:
        return SVR_Ok;
    case FsStatus::NotFound:
        return save_fail(err, SVR_NotFound, "can't open \"%s\"", fname);
    default:
        return save_fail(err, SVR_Damaged, "can't read \"%s\"", fname);
    }
}

bool WriteFile(IFileSystem &filesystem, const char *fname, const void *data, size_t len)
{
    if (filesystem.WriteAll(fname, data, len) == FsStatus::Ok)
        return true;
    SAVEWARN("can't write \"%s\"", fname);
    return false;
}

namespace detail {

enum SaveResult ReadRecords(IFileSystem &filesystem, const char *fname, uint32_t kind, const SaveStructDesc *desc,
    uint32_t elem_size, void **records, uint32_t *count, SaveError *err)
{
    *records = nullptr;
    *count = 0;
    std::vector<uint8_t> file;
    const enum SaveResult result = ReadFile(filesystem, fname, file, err);
    if (result != SVR_Ok)
        return result;
    return save_recfile_parse(file.data(), static_cast<uint32_t>(file.size()), kind, desc, elem_size, records, count, err);
}

bool WriteRecords(IFileSystem &filesystem, const char *fname, uint32_t kind, const SaveStructDesc *desc,
    const void *records, uint32_t elem_size, uint32_t count)
{
    SaveBufferOwner out;
    SaveError err;
    const enum SaveResult result = save_recfile_build(&out, kind, desc, records, elem_size, count, &err);
    if (result != SVR_Ok) {
        SAVEWARN("can't build \"%s\": %s", fname, err.message);
        return false;
    }
    return WriteFile(filesystem, fname, out.data, out.len);
}

} // namespace detail

} // namespace savefile
