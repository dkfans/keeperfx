#ifndef KFX_FILESYSTEMLB_H
#define KFX_FILESYSTEMLB_H

#include "kfx/platform/IFileSystem.h"

/** IFileSystem over the game's LbFile functions. */
class FileSystemLb final : public IFileSystem {
public:
    bool Exists(const char* path) override;
    FsStatus ReadAll(const char* path, std::vector<uint8_t>& out) override;
    FsStatus WriteAll(const char* path, const void* data, size_t len) override;
    bool Delete(const char* path) override;
    bool Rename(const char* from, const char* to) override;
    std::unique_ptr<IFileReader> OpenReader(const char* path) override;
    std::vector<std::string> Find(const char* filespec) override;
};

#endif // KFX_FILESYSTEMLB_H
