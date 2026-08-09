#include "pre_inc.h"
#include "kfx/platform/PlatformWindows.h"
#include "kfx/platform/FileFind.h"
#include "bflib_fileio.h"
#include <SDL3/SDL.h>
#include <cstdlib>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <algorithm>
#include <cctype>
#include <memory>
#include "post_inc.h"

const char* PlatformWindows::GetOSVersion() const
{
    static char buffer[256];
    OSVERSIONINFO v;
    v.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (GetVersionEx(&v)) {
        snprintf(buffer, sizeof(buffer), "%s %ld.%ld.%ld",
            (v.dwPlatformId == VER_PLATFORM_WIN32_NT) ? "Windows NT" : "Windows",
            v.dwMajorVersion, v.dwMinorVersion, v.dwBuildNumber);
        return buffer;
    }
    return "unknown";
}

const void* PlatformWindows::GetImageBase() const
{
    return GetModuleHandle(NULL);
}

const char* PlatformWindows::GetWineVersion() const
{
    const auto module = GetModuleHandle("ntdll.dll");
    if (module) {
        const auto wine_get_version = (const char* (WINAPI*)()) (void*) GetProcAddress(module, "wine_get_version");
        if (wine_get_version) {
            return wine_get_version();
        }
    }
    return nullptr;
}

const char* PlatformWindows::GetWineHost() const
{
    const auto module = GetModuleHandle("ntdll.dll");
    static char buffer[256];
    if (module) {
        const auto wine_get_host_version = (void (WINAPI*)(const char**, const char**)) (void*) GetProcAddress(module, "wine_get_host_version");
        if (wine_get_host_version) {
            const char* sys_name = nullptr;
            const char* release_name = nullptr;
            wine_get_host_version(&sys_name, &release_name);
            snprintf(buffer, sizeof(buffer), "%s %s", sys_name ? sys_name : "unknown", release_name ? release_name : "unknown");
            return buffer;
        }
    }
    return nullptr;
}

TbFileFind* PlatformWindows::FileFindFirst(const char* filespec, TbFileEntry* entry)
{
    auto ffind = std::make_unique<TbFileFind>();
    WIN32_FIND_DATA fd;
    HANDLE handle = FindFirstFile(filespec, &fd);
    if (handle == INVALID_HANDLE_VALUE) {
        return nullptr;
    }
    do {
        // Regular files only. Skipping directories also drops "." and ".." so a
        // plain "*" enumerates the same set as the Linux implementation.
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }
        std::string key = fd.cFileName;
        for (size_t i = 0; i < key.size(); i++) {
            key[i] = (char)tolower((unsigned char)key[i]);
        }
        ffind->names.emplace_back(key, fd.cFileName);
    } while (FindNextFile(handle, &fd));
    FindClose(handle);
    if (ffind->names.empty()) {
        return nullptr;
    }
    std::sort(ffind->names.begin(), ffind->names.end());
    entry->Filename = ffind->names[0].second.c_str();
    return ffind.release();
}

bool PlatformWindows::VideoInit()
{
    // SDL disables the screensaver by default, which can disrupt the HDR
    // compositor; re-allow it before initialising video.
    SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1");
    if (!SDL_Init(SDL_INIT_VIDEO))
        return false;
    atexit(SDL_Quit);
    return true;
}
