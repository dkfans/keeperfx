#include "../pre_inc.h"
#include "PlatformWindows.h"
#include "PlatformManager.h"
#include "../platform.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// windows.h maps GetCurrentDirectory -> GetCurrentDirectoryA; undefine to avoid
// mangling the IPlatform virtual method name.
#undef GetCurrentDirectory
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <direct.h>
#include <errno.h>
#include <shellapi.h>
#include <vector>
#include "../post_inc.h"

// ============================================================
// OS information
// ============================================================

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
        const auto fn = (const char * (WINAPI *)()) (void *) GetProcAddress(module, "wine_get_version");
        if (fn) return fn();
    }
    return nullptr;
}

const char* PlatformWindows::GetWineHost() const
{
    const auto module = GetModuleHandle("ntdll.dll");
    static char buffer[256];
    if (module) {
        const auto fn = (void (WINAPI *)(const char **, const char **)) (void *) GetProcAddress(module, "wine_get_host_version");
        if (fn) {
            const char* sys_name = nullptr;
            const char* release_name = nullptr;
            fn(&sys_name, &release_name);
            snprintf(buffer, sizeof(buffer), "%s %s",
                sys_name     ? sys_name     : "unknown",
                release_name ? release_name : "unknown");
            return buffer;
        }
    }
    return nullptr;
}

// ============================================================
// Crash / error parachute
// The VEH handler is installed in WinMain() below; no further setup needed.
// ============================================================

void PlatformWindows::ErrorParachuteInstall() {}
void PlatformWindows::ErrorParachuteUpdate()  {}

// ============================================================
// File enumeration
// ============================================================

struct TbFileFind {
    HANDLE handle;
    char*  namebuf;
    int    namebuflen;
};

TbFileFind* PlatformWindows::FileFindFirst(const char* filespec, TbFileEntry* fentry)
{
    auto ffind = static_cast<TbFileFind*>(malloc(sizeof(TbFileFind)));
    if (!ffind) return nullptr;

    WIN32_FIND_DATA fd;
    ffind->handle = FindFirstFile(filespec, &fd);
    if (ffind->handle == INVALID_HANDLE_VALUE) {
        free(ffind);
        return nullptr;
    }
    const int namelen = strlen(fd.cFileName);
    ffind->namebuf = static_cast<char*>(malloc(namelen + 1));
    if (!ffind->namebuf) {
        FindClose(ffind->handle);
        free(ffind);
        return nullptr;
    }
    memcpy(ffind->namebuf, fd.cFileName, namelen + 1);
    ffind->namebuflen = namelen;
    fentry->Filename = ffind->namebuf;
    return ffind;
}

int32_t PlatformWindows::FileFindNext(TbFileFind* ffind, TbFileEntry* fentry)
{
    if (!ffind) return -1;
    WIN32_FIND_DATA fd;
    if (!FindNextFile(ffind->handle, &fd)) return -1;
    const int namelen = strlen(fd.cFileName);
    if (namelen > ffind->namebuflen) {
        auto buf = static_cast<char*>(realloc(ffind->namebuf, namelen + 1));
        if (!buf) return -1;
        ffind->namebuf = buf;
        ffind->namebuflen = namelen;
    }
    memcpy(ffind->namebuf, fd.cFileName, namelen + 1);
    fentry->Filename = ffind->namebuf;
    return 1;
}

void PlatformWindows::FileFindEnd(TbFileFind* ffind)
{
    if (ffind) {
        FindClose(ffind->handle);
        free(ffind->namebuf);
        free(ffind);
    }
}

// ============================================================
// File system helpers
// ============================================================

TbBool PlatformWindows::FileExists(const char* path) const
{
    return (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES) ? 1 : 0;
}

int PlatformWindows::MakeDirectory(const char* path)
{
    if (mkdir(path) == 0 || errno == EEXIST) return 0;
    return -1;
}

int PlatformWindows::GetCurrentDirectory(char* buf, unsigned long buflen)
{
    if (getcwd(buf, buflen) != nullptr) {
        // Strip leading drive letter (e.g. "C:" -> "")
        if (buf[1] == ':')
            memmove(buf, buf + 2, strlen(buf + 2) + 1);
        const int len = strlen(buf);
        if (len > 1 && buf[len - 1] == '\\')
            buf[len - 1] = '\0';
        return 1;
    }
    return -1;
}

// ============================================================
// File I/O
// ============================================================

/** Creates all intermediate directories for fname if they do not exist. */
static int make_dirs_for_file(const char* fname)
{
    const int size = strlen(fname) + 1;
    char* tmp = static_cast<char*>(malloc(size));
    if (!tmp) return 0;
    const char* sep = strchr(fname, '/');
    while (sep != nullptr) {
        memcpy(tmp, fname, sep - fname);
        tmp[sep - fname] = '\0';
        if (mkdir(tmp) != 0 && errno != EEXIST) {
            free(tmp);
            return 0;
        }
        sep = strchr(sep + 1, '/');
    }
    free(tmp);
    return 1;
}

TbFileHandle PlatformWindows::FileOpen(const char* fname, unsigned char accmode)
{
    unsigned char mode = accmode;
    const int file_exists = (GetFileAttributesA(fname) != INVALID_FILE_ATTRIBUTES) ? 1 : 0;

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
        rc = fopen(fname, "r+b");
        break;
    case Lb_FILE_MODE_READ_ONLY:
        rc = fopen(fname, "rb");
        break;
    }
    return rc;
}

int PlatformWindows::FileClose(TbFileHandle handle)
{
    return fclose(handle) ? -1 : 1;
}

int PlatformWindows::FileRead(TbFileHandle handle, void* buf, unsigned long len)
{
    return fread(buf, 1, len, handle);
}

long PlatformWindows::FileWrite(TbFileHandle handle, const void* buf, unsigned long len)
{
    return fwrite(buf, 1, len, handle);
}

int PlatformWindows::FileSeek(TbFileHandle handle, long offset, unsigned char origin)
{
    switch (origin) {
    case Lb_FILE_SEEK_BEGINNING: return fseek(handle, offset, SEEK_SET);
    case Lb_FILE_SEEK_CURRENT:   return fseek(handle, offset, SEEK_CUR);
    case Lb_FILE_SEEK_END:       return fseek(handle, offset, SEEK_END);
    }
    return -1;
}

int PlatformWindows::FilePosition(TbFileHandle handle)
{
    return ftell(handle);
}

TbBool PlatformWindows::FileEof(TbFileHandle handle)
{
    const long pos = ftell(handle);
    fseek(handle, 0, SEEK_END);
    const long end = ftell(handle);
    fseek(handle, pos, SEEK_SET);
    return (pos >= end) ? 1 : 0;
}

short PlatformWindows::FileFlush(TbFileHandle handle)
{
    return (fflush(handle) == 0) ? 1 : 0;
}

long PlatformWindows::FileLength(const char* fname)
{
    TbFileHandle h = fopen(fname, "rb");
    if (!h) return -1;
    fseek(h, 0, SEEK_END);
    const long result = ftell(h);
    fclose(h);
    return result;
}

int PlatformWindows::FileDelete(const char* fname)
{
    return remove(fname) ? -1 : 1;
}

// ============================================================
// Path provider stubs (not yet integrated)
// ============================================================

void        PlatformWindows::SetArgv(int, char**) {}
const char* PlatformWindows::GetDataPath()  const { return ""; }
const char* PlatformWindows::GetSavePath()  const { return ""; }
const char* PlatformWindows::GetUserPrefDir()     { return ""; }

// ============================================================
// CDROM / Redbook audio — handled by cdrom.cpp in the current build
// ============================================================

void   PlatformWindows::SetRedbookVolume(SoundVolume) {}
TbBool PlatformWindows::PlayRedbookTrack(int)         { return 0; }
void   PlatformWindows::PauseRedbookTrack()           {}
void   PlatformWindows::ResumeRedbookTrack()          {}
void   PlatformWindows::StopRedbookTrack()            {}

// ============================================================
// Exception-name helper and VEH handler (from windows.cpp)
// ============================================================

static const char* exception_name(DWORD exception_code)
{
    switch (exception_code) {
        case EXCEPTION_ACCESS_VIOLATION:         return "EXCEPTION_ACCESS_VIOLATION";
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
        case EXCEPTION_BREAKPOINT:               return "EXCEPTION_BREAKPOINT";
        case EXCEPTION_DATATYPE_MISALIGNMENT:    return "EXCEPTION_DATATYPE_MISALIGNMENT";
        case EXCEPTION_FLT_DENORMAL_OPERAND:     return "EXCEPTION_FLT_DENORMAL_OPERAND";
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
        case EXCEPTION_FLT_INEXACT_RESULT:       return "EXCEPTION_FLT_INEXACT_RESULT";
        case EXCEPTION_FLT_INVALID_OPERATION:    return "EXCEPTION_FLT_INVALID_OPERATION";
        case EXCEPTION_FLT_OVERFLOW:             return "EXCEPTION_FLT_OVERFLOW";
        case EXCEPTION_FLT_STACK_CHECK:          return "EXCEPTION_FLT_STACK_CHECK";
        case EXCEPTION_FLT_UNDERFLOW:            return "EXCEPTION_FLT_UNDERFLOW";
        case EXCEPTION_ILLEGAL_INSTRUCTION:      return "EXCEPTION_ILLEGAL_INSTRUCTION";
        case EXCEPTION_IN_PAGE_ERROR:            return "EXCEPTION_IN_PAGE_ERROR";
        case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "EXCEPTION_INT_DIVIDE_BY_ZERO";
        case EXCEPTION_INT_OVERFLOW:             return "EXCEPTION_INT_OVERFLOW";
        case EXCEPTION_INVALID_DISPOSITION:      return "EXCEPTION_INVALID_DISPOSITION";
        case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
        case EXCEPTION_PRIV_INSTRUCTION:         return "EXCEPTION_PRIV_INSTRUCTION";
        case EXCEPTION_SINGLE_STEP:              return "EXCEPTION_SINGLE_STEP";
        case EXCEPTION_STACK_OVERFLOW:           return "EXCEPTION_STACK_OVERFLOW";
    }
    return "Unknown";
}

static LONG __stdcall Vex_handler(_EXCEPTION_POINTERS* ExceptionInfo)
{
    const auto exception_code = ExceptionInfo->ExceptionRecord->ExceptionCode;
    if (exception_code == DBG_PRINTEXCEPTION_WIDE_C)
        return EXCEPTION_CONTINUE_EXECUTION;
    if (exception_code == DBG_PRINTEXCEPTION_C)
        return EXCEPTION_CONTINUE_EXECUTION;
    if (exception_code == 0xe24c4a02)
        return EXCEPTION_EXECUTE_HANDLER; // LuaJIT internal
    LbJustLog("Exception 0x%08lx thrown: %s\n", exception_code, exception_name(exception_code));
    return EXCEPTION_CONTINUE_SEARCH;
}

// ============================================================
// Windows entry point
// ============================================================

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    PlatformManager::Set(new PlatformWindows());
    AddVectoredExceptionHandler(0, &Vex_handler);

    // Construct argc/argv from Unicode command line
    int argc = 0;
    auto szArglist = CommandLineToArgvW(GetCommandLineW(), &argc);
    std::vector<char*> argv(argc);
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
