#ifndef KFX_IFILESYSTEM_H
#define KFX_IFILESYSTEM_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

/** The outcome of a whole-file read or write. */
enum class FsStatus {
    /** It worked. */
    Ok,
    /** There is no such file. */
    NotFound,
    /** The file exists but couldn't be read, or couldn't be written. */
    Failed
};

/** Ranged reads of one open file. */
class IFileReader {
public:
    virtual ~IFileReader() = default;
    virtual uint32_t Size() const = 0;
    virtual bool ReadAt(uint32_t offset, void* dst, uint32_t len) = 0;
};

/** Whole-file storage for saves and other small data files. Paths are already resolved by the
 *  caller. Seekable streaming handles are not part of this interface. */
class IFileSystem {
public:
    virtual ~IFileSystem() = default;
    virtual bool Exists(const char* path) = 0;
    virtual FsStatus ReadAll(const char* path, std::vector<uint8_t>& out) = 0;
    /** Replaces any file at path. A failed write leaves the old file as it was. */
    virtual FsStatus WriteAll(const char* path, const void* data, size_t len) = 0;
    virtual bool Delete(const char* path) = 0;
    virtual bool Rename(const char* from, const char* to) = 0;
    virtual std::unique_ptr<IFileReader> OpenReader(const char* path) = 0;
    /** File names (no directory) matching a wildcard spec such as ".../fx1g*.sav". */
    virtual std::vector<std::string> Find(const char* filespec) = 0;
};

#endif // KFX_IFILESYSTEM_H
