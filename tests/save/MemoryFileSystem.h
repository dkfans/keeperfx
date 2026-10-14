/** @file MemoryFileSystem.h
 *     An IFileSystem that keeps its files in memory, for tests.
 */
#ifndef KFX_TEST_MEMORYFILESYSTEM_H
#define KFX_TEST_MEMORYFILESYSTEM_H

#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "kfx/platform/IFileSystem.h"

class MemoryFileSystem final : public IFileSystem {
public:
    std::map<std::string, std::vector<uint8_t>> files;
    bool fail_writes = false;

    bool Exists(const char *path) override { return files.count(path) != 0; }

    FsStatus ReadAll(const char *path, std::vector<uint8_t> &out) override
    {
        out.clear();
        auto it = files.find(path);
        if (it == files.end())
            return FsStatus::NotFound;
        out = it->second;
        return FsStatus::Ok;
    }

    FsStatus WriteAll(const char *path, const void *data, size_t len) override
    {
        if (fail_writes)
            return FsStatus::Failed;
        const uint8_t *p = static_cast<const uint8_t *>(data);
        files[path].assign(p, p + len);
        return FsStatus::Ok;
    }

    bool Delete(const char *path) override { return files.erase(path) != 0; }

    bool Rename(const char *from, const char *to) override
    {
        auto it = files.find(from);
        if (it == files.end())
            return false;
        files[to] = it->second;
        files.erase(from);
        return true;
    }

    std::unique_ptr<IFileReader> OpenReader(const char *path) override
    {
        auto it = files.find(path);
        if (it == files.end())
            return nullptr;
        return std::unique_ptr<IFileReader>(new Reader(it->second));
    }

    std::vector<std::string> Find(const char *filespec) override
    {
        std::string spec = filespec;
        size_t slash = spec.find_last_of("/\\");
        std::string dir = (slash == std::string::npos) ? "" : spec.substr(0, slash + 1);
        std::string pattern = spec.substr(dir.size());
        std::vector<std::string> names;
        for (const auto &f : files) {
            if (f.first.compare(0, dir.size(), dir) != 0)
                continue;
            std::string name = f.first.substr(dir.size());
            if (Match(pattern.c_str(), name.c_str()))
                names.push_back(name);
        }
        return names;
    }

private:
    class Reader final : public IFileReader {
    public:
        explicit Reader(const std::vector<uint8_t> &data) : data_(data) {}
        uint32_t Size() const override { return (uint32_t)data_.size(); }
        bool ReadAt(uint32_t offset, void *dst, uint32_t len) override
        {
            if ((uint64_t)offset + len > data_.size())
                return false;
            if (len)
                memcpy(dst, data_.data() + offset, len);
            return true;
        }
    private:
        std::vector<uint8_t> data_;
    };

    static bool Match(const char *pat, const char *s)
    {
        if (*pat == '\0')
            return *s == '\0';
        if (*pat == '*')
            return Match(pat + 1, s) || ((*s != '\0') && Match(pat, s + 1));
        return (*s != '\0') && ((*pat == '?') || (*pat == *s)) && Match(pat + 1, s + 1);
    }
};

#endif
