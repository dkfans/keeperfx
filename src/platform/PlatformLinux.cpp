#include "pre_inc.h"
#include "platform/PlatformLinux.h"
#include "platform/PlatformManager.h"
#include "platform.h"
#include "bflib_crash.h"
#include "steam_api.hpp"
#include "cdrom.h"

#include <string>
#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fnmatch.h>
#include <errno.h>
#include <limits.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "post_inc.h"

// ============================================================
// OS information
// ============================================================

const char* PlatformLinux::GetOSVersion() const { return "Linux"; }
const void* PlatformLinux::GetImageBase() const { return nullptr; }
const char* PlatformLinux::GetWineVersion() const { return nullptr; }
const char* PlatformLinux::GetWineHost() const { return nullptr; }

// ============================================================
// Crash / error parachute
// ============================================================

void PlatformLinux::ErrorParachuteInstall()
{
    LbErrorParachuteInstall();
}

void PlatformLinux::ErrorParachuteUpdate() {}

// ============================================================
// File enumeration
// ============================================================

struct TbFileFind {
    std::string filespec;
    std::string path;
    std::string namebuf;
    DIR*        handle   = nullptr;
    bool        is_pattern = false;
};

static bool filespec_is_pattern(const char* filespec)
{
    return strchr(filespec, '*') != nullptr;
}

static std::string directory_from_filespec(const char* filespec)
{
    const auto sep = strrchr(filespec, '/');
    if (sep && sep != filespec)
        return std::string(filespec, sep - filespec);
    return ".";
}

static bool find_file(TbFileFind* ff, TbFileEntry* fe)
{
    while (true) {
        auto de = readdir(ff->handle);
        if (!de) return false;
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
        const std::string path = ff->path + "/" + de->d_name;
        ff->namebuf = de->d_name;
        fe->Filename = ff->namebuf.c_str();
        if (ff->is_pattern) {
            if (fnmatch(ff->filespec.c_str(), path.c_str(), FNM_FILE_NAME | FNM_CASEFOLD) != 0)
                continue;
        }
        struct stat sb;
        if (stat(path.c_str(), &sb) < 0 || !S_ISREG(sb.st_mode)) continue;
        return true;
    }
}

TbFileFind* PlatformLinux::FileFindFirst(const char* filespec, TbFileEntry* fe)
{
    try {
        auto ff = std::make_unique<TbFileFind>();
        ff->is_pattern = filespec_is_pattern(filespec);
        ff->filespec   = filespec;
        if (ff->is_pattern) {
            ff->path   = directory_from_filespec(filespec);
            ff->handle = opendir(ff->path.c_str());
        } else {
            ff->path   = filespec;
            ff->handle = opendir(filespec);
        }
        if (ff->handle && find_file(ff.get(), fe))
            return ff.release();
    } catch (...) {}
    return nullptr;
}

int32_t PlatformLinux::FileFindNext(TbFileFind* ff, TbFileEntry* fe)
{
    try {
        if (find_file(ff, fe)) return 1;
    } catch (...) {}
    return -1;
}

void PlatformLinux::FileFindEnd(TbFileFind* ff)
{
    if (ff) closedir(ff->handle);
    delete ff;
}

// ============================================================
// Case-insensitive file-name helper (Linux only)
// ============================================================

/** Finds the on-disk name of a file using case-insensitive matching.
 *  Returns 1 and fills actual_fname on success, 0 on failure. */
static int find_case_insensitive_file(const char* fname, char* actual_fname, size_t buflen)
{
    const char* last_slash = strrchr(fname, '/');
    const char* filename;
    char dir_path[PATH_MAX];
    size_t dir_len = 0;

    if (last_slash != nullptr) {
        dir_len = last_slash - fname;
        if (dir_len >= sizeof(dir_path)) return 0;
        if (dir_len > 0) {
            memcpy(dir_path, fname, dir_len);
            dir_path[dir_len] = '\0';
        } else {
            strcpy(dir_path, "/");
        }
        filename = last_slash + 1;
    } else {
        strcpy(dir_path, ".");
        filename = fname;
    }

    DIR* dir = opendir(dir_path);
    if (!dir) return 0;

    struct dirent* entry;
    int found = 0;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcasecmp(entry->d_name, filename) == 0) {
            if (last_slash != nullptr) {
                const size_t needed = dir_len + 1 + strlen(entry->d_name) + 1;
                if (needed > buflen) { closedir(dir); return 0; }
                if (dir_len > 0)
                    snprintf(actual_fname, buflen, "%s/%s", dir_path, entry->d_name);
                else
                    snprintf(actual_fname, buflen, "/%s", entry->d_name);
            } else {
                if (strlen(entry->d_name) + 1 > buflen) { closedir(dir); return 0; }
                strcpy(actual_fname, entry->d_name);
            }
            found = 1;
            break;
        }
    }
    closedir(dir);
    return found;
}

// ============================================================
// File system helpers
// ============================================================

TbBool PlatformLinux::FileExists(const char* path) const
{
    if (access(path, F_OK) == 0) return 1;
    char actual[PATH_MAX];
    return find_case_insensitive_file(path, actual, sizeof(actual)) ? 1 : 0;
}

int PlatformLinux::MakeDirectory(const char* path)
{
    if (mkdir(path, 0755) == 0 || errno == EEXIST) return 0;
    return -1;
}

int PlatformLinux::GetCurrentDirectory(char* buf, unsigned long buflen)
{
    if (getcwd(buf, buflen) != nullptr) {
        const int len = strlen(buf);
        if (len > 1 && buf[len - 1] == '/')
            buf[len - 1] = '\0';
        return 1;
    }
    return -1;
}

// ============================================================
// File I/O
// ============================================================

static int make_dirs_for_file(const char* fname)
{
    const int size = strlen(fname) + 1;
    char* tmp = static_cast<char*>(malloc(size));
    if (!tmp) return 0;
    const char* sep = strchr(fname, '/');
    while (sep != nullptr) {
        memcpy(tmp, fname, sep - fname);
        tmp[sep - fname] = '\0';
        if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
            free(tmp);
            return 0;
        }
        sep = strchr(sep + 1, '/');
    }
    free(tmp);
    return 1;
}

TbFileHandle PlatformLinux::FileOpen(const char* fname, unsigned char accmode)
{
    unsigned char mode = accmode;
    const char* open_fname = fname;
    char actual_fname[PATH_MAX];

    const int exists_exact = (access(fname, F_OK) == 0);
    if (!exists_exact && find_case_insensitive_file(fname, actual_fname, sizeof(actual_fname)))
        open_fname = actual_fname;

    const int file_exists = exists_exact || (open_fname != fname);
    if (!file_exists) {
        if (mode == Lb_FILE_MODE_READ_ONLY) return nullptr;
        if (mode == Lb_FILE_MODE_OLD)       mode = Lb_FILE_MODE_NEW;
    }

    TbFileHandle rc = nullptr;
    switch (mode) {
    case Lb_FILE_MODE_NEW:
        if (make_dirs_for_file(fname))
            rc = fopen(fname, "wb");
        break;
    case Lb_FILE_MODE_OLD:
        rc = fopen(open_fname, "r+b");
        break;
    case Lb_FILE_MODE_READ_ONLY:
        rc = fopen(open_fname, "rb");
        break;
    }
    return rc;
}

int PlatformLinux::FileClose(TbFileHandle handle)   { return fclose(handle) ? -1 : 1; }
int PlatformLinux::FileRead(TbFileHandle handle, void* buf, unsigned long len) { return fread(buf, 1, len, handle); }
long PlatformLinux::FileWrite(TbFileHandle handle, const void* buf, unsigned long len) { return fwrite(buf, 1, len, handle); }

int PlatformLinux::FileSeek(TbFileHandle handle, long offset, unsigned char origin)
{
    switch (origin) {
    case Lb_FILE_SEEK_BEGINNING: return fseek(handle, offset, SEEK_SET);
    case Lb_FILE_SEEK_CURRENT:   return fseek(handle, offset, SEEK_CUR);
    case Lb_FILE_SEEK_END:       return fseek(handle, offset, SEEK_END);
    }
    return -1;
}

int PlatformLinux::FilePosition(TbFileHandle handle) { return ftell(handle); }

TbBool PlatformLinux::FileEof(TbFileHandle handle)
{
    const long pos = ftell(handle);
    fseek(handle, 0, SEEK_END);
    const long end = ftell(handle);
    fseek(handle, pos, SEEK_SET);
    return (pos >= end) ? 1 : 0;
}

short PlatformLinux::FileFlush(TbFileHandle handle) { return (fflush(handle) == 0) ? 1 : 0; }

long PlatformLinux::FileLength(const char* fname)
{
    const char* open_fname = fname;
    char actual_fname[PATH_MAX];
    if (access(fname, F_OK) != 0 && find_case_insensitive_file(fname, actual_fname, sizeof(actual_fname)))
        open_fname = actual_fname;
    TbFileHandle h = fopen(open_fname, "rb");
    if (!h) return -1;
    fseek(h, 0, SEEK_END);
    const long result = ftell(h);
    fclose(h);
    return result;
}

int PlatformLinux::FileDelete(const char* fname)
{
    const char* del_fname = fname;
    char actual_fname[PATH_MAX];
    if (access(fname, F_OK) != 0 && find_case_insensitive_file(fname, actual_fname, sizeof(actual_fname)))
        del_fname = actual_fname;
    return remove(del_fname) ? -1 : 1;
}

// ============================================================
// Path provider stubs (not yet integrated)
// ============================================================

void        PlatformLinux::SetArgv(int, char**)  {}
const char* PlatformLinux::GetDataPath()  const  { return ""; }
const char* PlatformLinux::GetSavePath()  const  { return ""; }
const char* PlatformLinux::GetUserPrefDir()      { return ""; }

// ============================================================
// CDROM / Redbook audio — not supported on Linux
// ============================================================

void   PlatformLinux::SetRedbookVolume(SoundVolume) {}
TbBool PlatformLinux::PlayRedbookTrack(int)          { return 0; }
void   PlatformLinux::PauseRedbookTrack()            {}
void   PlatformLinux::ResumeRedbookTrack()           {}
void   PlatformLinux::StopRedbookTrack()             {}

// Free C functions called directly by bflib_sndlib.cpp (cdrom.cpp provides
// these on Windows; we supply no-op stubs for Linux).
extern "C" {
    void   SetRedbookVolume(SoundVolume)  {}
    TbBool PlayRedbookTrack(int)          { return 0; }
    void   PauseRedbookTrack()            {}
    void   ResumeRedbookTrack()           {}
    void   StopRedbookTrack()             {}
}

// ============================================================
// Steam API stubs (Steam not supported on Linux)
// ============================================================

extern "C" int steam_api_init()   { return 0; }
extern "C" void steam_api_shutdown() {}

// ============================================================
// Linux entry point
// ============================================================

extern "C" int main(int argc, char* argv[])
{
    PlatformManager::Set(new PlatformLinux());
    return kfxmain(argc, argv);
}
