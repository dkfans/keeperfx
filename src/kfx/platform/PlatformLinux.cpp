#include "pre_inc.h"
#include "kfx/platform/PlatformLinux.h"
#include "kfx/platform/FileFind.h"
#include "platform.h" // kfxmain
#include "bflib_fileio.h"
#include "config.h" // keeper_runtime_directory (GetUserPrefDir() SDL-less fallback)
#include <SDL3/SDL.h>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <algorithm>
#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fnmatch.h>
#include "post_inc.h"

static bool filespec_is_pattern(const char* filespec)
{
    return strchr(filespec, '*') != nullptr;
}

static std::string directory_from_filespec(const char* filespec)
{
    const auto sep = strrchr(filespec, '/');
    if (sep && sep != filespec) {
        return std::string(filespec, sep - filespec);
    }
    return ".";
}

const char* PlatformLinux::GetOSVersion() const { return "Linux"; }
const void* PlatformLinux::GetImageBase() const { return nullptr; }
const char* PlatformLinux::GetWineVersion() const { return nullptr; } // running native
const char* PlatformLinux::GetWineHost() const { return nullptr; }    // running native

const char* PlatformLinux::GetUserPrefDir()
{
    static char pref_path[512] = {};
    if (pref_path[0] != '\0')
        return pref_path;
    char* sdl_path = SDL_GetPrefPath("keeperfx", "keeperfx");
    if (sdl_path)
    {
        snprintf(pref_path, sizeof(pref_path), "%s", sdl_path);
        size_t len = strlen(pref_path);
        if (len > 0 && pref_path[len - 1] == '/')
            pref_path[len - 1] = '\0';
        SDL_free(sdl_path);
    }
    else
    {
        // Fall back to the game's own runtime directory (where keeperfx.cfg
        // already lives) rather than an SDL-less user-pref concept this
        // branch doesn't otherwise have.
        snprintf(pref_path, sizeof(pref_path), "%s", keeper_runtime_directory);
    }
    return pref_path;
}

TbFileFind* PlatformLinux::FileFindFirst(const char* filespec, TbFileEntry* entry)
{
    try {
        auto ff = std::make_unique<TbFileFind>();
        bool is_pattern = filespec_is_pattern(filespec);
        std::string path = is_pattern ? directory_from_filespec(filespec) : filespec;
        DIR* handle = opendir(path.c_str());
        if (handle) {
            while (auto de = readdir(handle)) {
                if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
                    continue;
                }
                const std::string file_path = path + "/" + de->d_name;
                if (is_pattern && fnmatch(filespec, file_path.c_str(), FNM_FILE_NAME | FNM_CASEFOLD) != 0) {
                    continue;
                }
                struct stat sb;
                if (stat(file_path.c_str(), &sb) < 0 || !S_ISREG(sb.st_mode)) {
                    continue;
                }
                std::string key = de->d_name;
                for (size_t i = 0; i < key.size(); i++) {
                    key[i] = (char)tolower((unsigned char)key[i]);
                }
                ff->names.emplace_back(key, de->d_name);
            }
            closedir(handle);
        }
        if (!ff->names.empty()) {
            std::sort(ff->names.begin(), ff->names.end());
            entry->Filename = ff->names[0].second.c_str();
            return ff.release();
        }
    } catch (...) {}
    return nullptr;
}

bool PlatformLinux::VideoInit()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        return false;
    atexit(SDL_Quit);
    return true;
}

void   PlatformLinux::SetRedbookVolume(SoundVolume) {}
TbBool PlatformLinux::PlayRedbookTrack(int) { return false; }
void   PlatformLinux::PauseRedbookTrack() {}
void   PlatformLinux::ResumeRedbookTrack() {}
void   PlatformLinux::StopRedbookTrack() {}

int  PlatformLinux::InitSteam() { return -1; }
void PlatformLinux::ShutdownSteam() {}

/******************************************************************************/
// Process entry point.

extern "C" int main(int argc, char *argv[])
{
    return kfxmain(argc, argv);
}
