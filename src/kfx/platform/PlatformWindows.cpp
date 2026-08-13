#include "pre_inc.h"
#include "kfx/platform/PlatformWindows.h"
#include "kfx/platform/FileFind.h"
#include "platform.h"
#include "bflib_fileio.h"
#include <SDL3/SDL.h>
#include <cstdlib>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <shellapi.h>
#include <stdio.h>
#include <algorithm>
#include <cctype>
#include <memory>
#include <vector>
#include "keeperfx.hpp" // is_running_under_wine (Steam), todo : maybe cleanup, meh
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

namespace {

MCIDEVICEID g_redbook_device = 0;
SoundVolume g_redbook_volume = 0;

MCIDEVICEID mci_open(const char * drive) {
    MCI_OPEN_PARMS params = {};
    params.lpstrElementName = drive;
    params.lpstrDeviceType = "cdaudio";
    const auto flags = MCI_OPEN_TYPE | MCI_OPEN_ELEMENT | MCI_OPEN_SHAREABLE;
    mciSendCommand(0, MCI_OPEN, flags, (DWORD_PTR) &params);
    return params.wDeviceID; // will be zero on error
}

bool mci_close(MCIDEVICEID device_id) {
    MCI_GENERIC_PARMS params = {};
    const auto result = mciSendCommand(device_id, MCI_CLOSE, 0, (DWORD_PTR) &params);
    return result == 0;
}

bool mci_set_time_format(MCIDEVICEID device_id) {
    MCI_SET_PARMS params = {};
    params.dwTimeFormat = MCI_FORMAT_TMSF;
    const auto flags = MCI_SET_TIME_FORMAT;
    const auto result = mciSendCommand(device_id, MCI_SET, flags, (DWORD_PTR) &params);
    return result == 0;
}

bool mci_play(MCIDEVICEID device_id, int track) {
    MCI_PLAY_PARMS params = {};
    params.dwFrom = MCI_MAKE_TMSF(track, 0, 0, 0);
    params.dwTo = MCI_MAKE_TMSF(track + 1, 0, 0, 0);
    params.dwCallback = (DWORD_PTR) GetDesktopWindow();
    const auto flags = MCI_FROM | MCI_TO | MCI_NOTIFY;
    const auto result = mciSendCommand(device_id, MCI_PLAY, flags, (DWORD_PTR) &params);
    return result == 0;
}

bool mci_pause(MCIDEVICEID device_id) {
    MCI_GENERIC_PARMS params;
    const auto result = mciSendCommand(device_id, MCI_PAUSE, 0, (DWORD_PTR) &params);
    return result == 0;
}

bool mci_resume(MCIDEVICEID device_id) {
    MCI_GENERIC_PARMS params;
    const auto result = mciSendCommand(device_id, MCI_RESUME, 0, (DWORD_PTR) &params);
    return result == 0;
}

bool mci_stop(MCIDEVICEID device_id) {
    MCI_GENERIC_PARMS params = {};
    const auto result = mciSendCommand(device_id, MCI_STOP, 0, (DWORD_PTR) &params);
    return result == 0;
}

int mci_status(MCIDEVICEID device_id, int what) {
    MCI_STATUS_PARMS params = {};
    params.dwItem = what;
    const auto flags = MCI_STATUS_ITEM;
    mciSendCommand(device_id, MCI_STATUS, flags, (DWORD_PTR) &params);
    return params.dwReturn; // returns zero on error
}

bool open_redbook_device() {
    if (g_redbook_device == 0) {
        // find first cdrom device that has a disk in it
        char drive[] = "C:\\";
        for (char letter = 'C'; letter <= 'Z'; ++letter) {
            drive[0] = letter;
            if (GetDriveType(drive) != DRIVE_CDROM) {
                continue;
            }
            if (const auto device_id = mci_open(drive)) {
                const auto num_tracks = mci_status(device_id, MCI_STATUS_NUMBER_OF_TRACKS);
                if (num_tracks > 0) {
                    JUSTLOG("Using cdrom drive %s for music", drive);
                    g_redbook_device = device_id;
                    mci_set_time_format(device_id);
                    return true;
                }
                mci_close(device_id);
            }
        }
        return false;
    }
    return true;
}

} // namespace

void PlatformWindows::SetRedbookVolume(SoundVolume value) {
    // TODO: Not implemented
    g_redbook_volume = value;
}

TbBool PlatformWindows::PlayRedbookTrack(int track) {
    // The original disk only had 7 tracks (the first one being data).
    // However, any kind of disk can be inserted so just play whatever track we're told to play.
    if (open_redbook_device()) {
        const auto mode = mci_status(g_redbook_device, MCI_STATUS_MODE);
        if (mode == MCI_MODE_OPEN || mode == MCI_MODE_NOT_READY) {
            return false; // door open or no disk
        }
        const auto current_track = mci_status(g_redbook_device, MCI_STATUS_CURRENT_TRACK);
        if (current_track == track && (mode == MCI_MODE_PLAY || mode == MCI_MODE_SEEK)) {
            return false; // already playing or seeking to requested track
        }
        return mci_play(g_redbook_device, track);
    }
    return false;
}

void PlatformWindows::PauseRedbookTrack() {
    if (open_redbook_device()) {
        const auto mode = mci_status(g_redbook_device, MCI_STATUS_MODE);
        if (!(mode == MCI_MODE_PLAY || MCI_MODE_SEEK)) {
            return; // not currently playing or about to play
        }
        mci_pause(g_redbook_device);
    }
}

void PlatformWindows::ResumeRedbookTrack() {
    if (open_redbook_device()) {
        const auto mode = mci_status(g_redbook_device, MCI_STATUS_MODE);
        if (!(mode == MCI_MODE_PAUSE)) {
            return; // not currently paused
        }
        mci_resume(g_redbook_device);
    }
}

void PlatformWindows::StopRedbookTrack() {
    if (open_redbook_device()) {
        const auto mode = mci_status(g_redbook_device, MCI_STATUS_MODE);
        if (!(mode == MCI_MODE_PLAY || mode == MCI_MODE_PAUSE || mode == MCI_MODE_SEEK)) {
            return; // not currently playing, paused or about to play
        }
        mci_stop(g_redbook_device);
    }
}

/******************************************************************************/
// Steam — dynamically loads steam_api.dll (needs steam_api.dll + steam_appid.txt).

namespace {

HMODULE steam_lib = nullptr;

// Result from the dynamically loaded SteamAPI_Init function
enum ESteamAPIInitResult
{
    k_ESteamAPIInitResult_OK = 0,
    k_ESteamAPIInitResult_FailedGeneric = 1,   // Some other failure
    k_ESteamAPIInitResult_NoSteamClient = 2,   // Cannot connect to Steam, probably not running
    k_ESteamAPIInitResult_VersionMismatch = 3, // Steam client out of date
};

typedef char SteamErrMsg[1024];
typedef ESteamAPIInitResult(__cdecl *SteamApiInitFunc)(SteamErrMsg *err);
typedef void(__cdecl *SteamApiShutdownFunc)();

// Type-punning union to go from FARPROC to the __cdecl init function type
union SteamApiInitUnion
{
    FARPROC farProc;
    SteamApiInitFunc steamApiInitFunc;
};

SteamApiInitFunc SteamAPI_Init = nullptr;
SteamApiShutdownFunc SteamAPI_Shutdown = nullptr;

} // namespace

int PlatformWindows::InitSteam()
{
    // Make sure the steam API is not initialized multiple times
    if (steam_lib != NULL || SteamAPI_Init != NULL)
    {
        WARNLOG("Steam API already initialized");
        return 1;
    }

    // Make sure both files are present
    if (LbFileExists("steam_api.dll") == false || LbFileExists("steam_appid.txt") == false)
    {
        // If only one of the 2 required files is present, log it for the user.
        if (
            (LbFileExists("steam_api.dll") == true && LbFileExists("steam_appid.txt") == false) ||
            (LbFileExists("steam_api.dll") == false && LbFileExists("steam_appid.txt") == true))
        {
            ERRORLOG("The Steam API requires both the 'steam_api.dll' and 'steam_appid.txt' files to be present");
        }
        return 1;
    }

    JUSTLOG("'steam_api.dll' and 'steam_appid.txt' found");

    // Loading 'libsteam_api.so' under Wine can't reach the host's Steam binary,
    // so the Steam API is unsupported under Wine. Checked after the file check so
    // this only logs when the user is actually trying to enable it.
    if (is_running_under_wine == true)
    {
        WARNLOG("Using the Steam API under Wine is not supported");
        return -1;
    }

    // Load the Steam API library
    steam_lib = LoadLibraryA("steam_api.dll");
    if (!steam_lib)
    {
        ERRORLOG("Unable to load 'steam_api.dll' library");
        return 1;
    }

    JUSTLOG("'steam_api.dll' library loaded");

    // The 'Flat' version is used instead of SteamAPI_Init when dynamically linking
    SteamApiInitUnion SteamApiInit;
    SteamApiInit.farProc = GetProcAddress(steam_lib, "SteamAPI_InitFlat");
    if (SteamApiInit.farProc == NULL)
    {
        ERRORLOG("Failed to get proc address for 'SteamAPI_InitFlat' in 'steam_api.dll'");
        FreeLibrary(steam_lib);
        return 1;
    }
    SteamAPI_Init = SteamApiInit.steamApiInitFunc;

    SteamAPI_Shutdown = reinterpret_cast<SteamApiShutdownFunc>(GetProcAddress(steam_lib, "SteamAPI_Shutdown"));
    if (SteamAPI_Shutdown == NULL)
    {
        ERRORLOG("Failed to get proc address for 'SteamAPI_Shutdown' in 'steam_api.dll'");
        FreeLibrary(steam_lib);
        return 1;
    }

    // Initialize the Steam API (notifies Steam that we are running the game)
    SteamErrMsg error;
    ESteamAPIInitResult result = SteamAPI_Init(&error);

    if (result == k_ESteamAPIInitResult_OK) {
        JUSTLOG("Steam API connected");
    } else {
        if (result == k_ESteamAPIInitResult_NoSteamClient) {
            JUSTLOG("Cannot connect to the Steam client. Steam is probably not running");
        } else if (result == k_ESteamAPIInitResult_VersionMismatch) {
            WARNLOG("Steam API version mismatch. Steam client appears to be out of date");
        } else {
            ERRORLOG("Steam API Failure: %s", error);
        }
        FreeLibrary(steam_lib);
        steam_lib = nullptr;
        SteamAPI_Init = nullptr;
        SteamAPI_Shutdown = nullptr;
        return 1;
    }

    return 0;
}

void PlatformWindows::ShutdownSteam()
{
    if (SteamAPI_Shutdown != nullptr) {
        JUSTLOG("Shutting down Steam API");
        SteamAPI_Shutdown();
    }
    if (steam_lib != nullptr) {
        FreeLibrary(steam_lib);
        steam_lib = nullptr;
    }
    SteamAPI_Init = nullptr;
    SteamAPI_Shutdown = nullptr;
}

/******************************************************************************/
// Process entry point and vectored exception logger (the crash-parachute piece
// that must live in the Windows translation unit).

namespace {

const char * exception_name(DWORD exception_code)
{
    switch (exception_code) {
        case EXCEPTION_ACCESS_VIOLATION: return "EXCEPTION_ACCESS_VIOLATION";
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
        case EXCEPTION_BREAKPOINT: return "EXCEPTION_BREAKPOINT";
        case EXCEPTION_DATATYPE_MISALIGNMENT: return "EXCEPTION_DATATYPE_MISALIGNMENT";
        case EXCEPTION_FLT_DENORMAL_OPERAND: return "EXCEPTION_FLT_DENORMAL_OPERAND";
        case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
        case EXCEPTION_FLT_INEXACT_RESULT: return "EXCEPTION_FLT_INEXACT_RESULT";
        case EXCEPTION_FLT_INVALID_OPERATION: return "EXCEPTION_FLT_INVALID_OPERATION";
        case EXCEPTION_FLT_OVERFLOW: return "EXCEPTION_FLT_OVERFLOW";
        case EXCEPTION_FLT_STACK_CHECK: return "EXCEPTION_FLT_STACK_CHECK";
        case EXCEPTION_FLT_UNDERFLOW: return "EXCEPTION_FLT_UNDERFLOW";
        case EXCEPTION_ILLEGAL_INSTRUCTION: return "EXCEPTION_ILLEGAL_INSTRUCTION";
        case EXCEPTION_IN_PAGE_ERROR: return "EXCEPTION_IN_PAGE_ERROR";
        case EXCEPTION_INT_DIVIDE_BY_ZERO: return "EXCEPTION_INT_DIVIDE_BY_ZERO";
        case EXCEPTION_INT_OVERFLOW: return "EXCEPTION_INT_OVERFLOW";
        case EXCEPTION_INVALID_DISPOSITION: return "EXCEPTION_INVALID_DISPOSITION";
        case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
        case EXCEPTION_PRIV_INSTRUCTION: return "EXCEPTION_PRIV_INSTRUCTION";
        case EXCEPTION_SINGLE_STEP: return "EXCEPTION_SINGLE_STEP";
        case EXCEPTION_STACK_OVERFLOW: return "EXCEPTION_STACK_OVERFLOW";
    }
    return "Unknown";
}

LONG __stdcall Vex_handler(_EXCEPTION_POINTERS *ExceptionInfo)
{
    const auto exception_code = ExceptionInfo->ExceptionRecord->ExceptionCode;
    if (exception_code == DBG_PRINTEXCEPTION_WIDE_C) {
        return EXCEPTION_CONTINUE_EXECUTION; // Thrown by OutputDebugStringW, intended for debugger
    } else if (exception_code == DBG_PRINTEXCEPTION_C) {
        return EXCEPTION_CONTINUE_EXECUTION; // Thrown by OutputDebugStringA, intended for debugger
    } else if (exception_code == 0xe24c4a02) {
        return EXCEPTION_EXECUTE_HANDLER; // Thrown by luaJIT for some reason
    }
    LbJustLog("Exception 0x%08lx thrown: %s\n", exception_code, exception_name(exception_code));
    return EXCEPTION_CONTINUE_SEARCH;
}

} // namespace

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    AddVectoredExceptionHandler(0, &Vex_handler);
    // Construct argc/argv from Unicode command line
    int argc = 0;
    auto szArglist = CommandLineToArgvW(GetCommandLineW(), &argc);
    std::vector<char *> argv(argc);
    std::vector<std::vector<char>> args(argc);
    for (int i = 0; i < argc; ++i) {
        const auto arg_size = WideCharToMultiByte(CP_UTF8, 0, szArglist[i], -1, nullptr, 0, nullptr, nullptr);
        if (arg_size > 0) {
            args[i] = std::vector<char>(arg_size);
            WideCharToMultiByte(CP_UTF8, 0, szArglist[i], -1, args[i].data(), arg_size, nullptr, nullptr);
        } else {
            args[i] = std::vector<char>(1);
        }
        argv[i] = args[i].data();
    }
    LocalFree(szArglist);
    return kfxmain(argc, argv.data());
}
