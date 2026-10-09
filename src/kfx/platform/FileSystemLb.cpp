/******************************************************************************/
/** @file FileSystemLb.cpp
 *     IFileSystem over the game's LbFile functions.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/platform/FileSystemLb.h"
#include "bflib_fileio.h"
#include "post_inc.h"

#include <cstdio>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

/** Moves from over to, replacing a file that is already there in one step, so a reader sees the old file or the new one. */
bool ReplaceFile(const char* from, const char* to)
{
#ifdef _WIN32
    return MoveFileExA(from, to, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    return rename(from, to) == 0;
#endif
}

class LbFileReader final : public IFileReader {
public:
    LbFileReader(TbFileHandle fh, uint32_t size) : fh_(fh), size_(size) {}
    ~LbFileReader() override { LbFileClose(fh_); }
    uint32_t Size() const override { return size_; }
    bool ReadAt(uint32_t offset, void* dst, uint32_t len) override
    {
        if (LbFileSeek(fh_, (long)offset, Lb_FILE_SEEK_BEGINNING) < 0)
            return false;
        return LbFileRead(fh_, dst, len) == (int)len;
    }
private:
    TbFileHandle fh_;
    uint32_t size_;
};

} // namespace

bool FileSystemLb::Exists(const char* path) { return LbFileExists(path) != 0; }

FsStatus FileSystemLb::ReadAll(const char* path, std::vector<uint8_t>& out)
{
    out.clear();
    TbFileHandle fh = LbFileOpen(path, Lb_FILE_MODE_READ_ONLY);
    if (!fh)
        return FsStatus::NotFound;
    long size = LbFileLengthHandle(fh);
    if (size < 0) {
        LbFileClose(fh);
        return FsStatus::Failed;
    }
    out.resize((size_t)size);
    bool ok = (size == 0) || (LbFileRead(fh, out.data(), (unsigned long)size) == size);
    LbFileClose(fh);
    if (!ok) {
        out.clear();
        return FsStatus::Failed;
    }
    return FsStatus::Ok;
}

FsStatus FileSystemLb::WriteAll(const char* path, const void* data, size_t len)
{
    // Written beside the target and renamed over it, so a failed write leaves the old file alone.
    const std::string tmp = std::string(path) + ".tmp";
    TbFileHandle fh = LbFileOpen(tmp.c_str(), Lb_FILE_MODE_NEW);
    if (!fh)
        return FsStatus::Failed;
    bool ok = (len == 0) || (LbFileWrite(fh, data, (unsigned long)len) == (long)len);
    LbFileClose(fh);
    if (ok)
        ok = ReplaceFile(tmp.c_str(), path);
    if (!ok)
        LbFileDelete(tmp.c_str());
    return ok ? FsStatus::Ok : FsStatus::Failed;
}

bool FileSystemLb::Delete(const char* path) { return LbFileDelete(path) >= 0; }

bool FileSystemLb::Rename(const char* from, const char* to) { return LbFileRename(from, to) >= 0; }

std::unique_ptr<IFileReader> FileSystemLb::OpenReader(const char* path)
{
    TbFileHandle fh = LbFileOpen(path, Lb_FILE_MODE_READ_ONLY);
    if (!fh)
        return nullptr;
    long size = LbFileLengthHandle(fh);
    if (size < 0) {
        LbFileClose(fh);
        return nullptr;
    }
    return std::make_unique<LbFileReader>(fh, (uint32_t)size);
}

std::vector<std::string> FileSystemLb::Find(const char* filespec)
{
    std::vector<std::string> names;
    TbFileEntry fe;
    TbFileFind* ff = LbFileFindFirst(filespec, &fe);
    if (ff) {
        do {
            names.push_back(fe.Filename);
        } while (LbFileFindNext(ff, &fe) >= 0);
        LbFileFindEnd(ff);
    }
    return names;
}
