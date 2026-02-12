#include "pre_inc.h"
#include "platform.h"
#include "bflib_fileio.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <shellapi.h>
#include <vector>
#include "post_inc.h"

extern "C" const char * get_os_version()
{
    static char buffer[256];
    OSVERSIONINFO v;
    v.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (GetVersionEx(&v)) {
        snprintf(buffer, sizeof(buffer), "%s %ld.%ld.%ld",
            (v.dwPlatformId == VER_PLATFORM_WIN32_NT) ? "Windows NT" : "Windows",
            v.dwMajorVersion, v.dwMinorVersion, v.dwBuildNumber);
        return buffer;
    } else {
        return "unknown";
    }
}

extern "C" const void * get_image_base()
{
    return GetModuleHandle(NULL);
}


extern "C" const char * get_wine_version()
{
    const auto module = GetModuleHandle("ntdll.dll");
    if (module) {
        const auto wine_get_version = (const char * (WINAPI *)()) (void *) GetProcAddress(module, "wine_get_version");
        if (wine_get_version) {
            return wine_get_version();
        }
    }
    return nullptr;
}

extern "C" const char * get_wine_host()
{
    const auto module = GetModuleHandle("ntdll.dll");
    static char buffer[256];
    if (module) {
        const auto wine_get_host_version = (void (WINAPI *)(const char **, const char **)) (void *) GetProcAddress(module, "wine_get_host_version");
        if (wine_get_host_version) {
            const char * sys_name = nullptr;
            const char * release_name = nullptr;
            wine_get_host_version(&sys_name, &release_name);
            snprintf(buffer, sizeof(buffer), "%s %s", sys_name ? sys_name : "unknown", release_name ? release_name : "unknown");
            return buffer;
        }
    }
    return nullptr;
}

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
    }else if (exception_code == 0xe24c4a02) {
        return EXCEPTION_EXECUTE_HANDLER; //Thrown by luaJIT for some reason
    }
    LbJustLog("Exception 0x%08lx thrown: %s\n", exception_code, exception_name(exception_code));
    return EXCEPTION_CONTINUE_SEARCH;
}

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

struct TbFileFind {
    HANDLE handle;
    char * namebuf;
    int namebuflen;
};

extern "C" TbFileFind * LbFileFindFirst(const char * filespec, struct TbFileEntry * fentry)
{
    auto ffind = static_cast<TbFileFind *>(malloc(sizeof(struct TbFileFind)));
    if (!ffind) {
        return nullptr;
    }
    WIN32_FIND_DATA fd;
    ffind->handle = FindFirstFile(filespec, &fd);
    if (ffind->handle == INVALID_HANDLE_VALUE) {
        free(ffind);
        return nullptr;
    }
    const auto namelen = strlen(fd.cFileName);
    ffind->namebuf = static_cast<char *>(malloc(namelen + 1));
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

extern "C" int LbFileFindNext(struct TbFileFind * ffind, struct TbFileEntry * fentry)
{
    if (!ffind) {
        return -1;
    }
    WIN32_FIND_DATA fd;
    if (!FindNextFile(ffind->handle, &fd)) {
        return -1;
    }
    const int namelen = strlen(fd.cFileName);
    if (namelen > ffind->namebuflen) {
        auto buf = static_cast<char *>(realloc(ffind->namebuf, namelen + 1));
        if (!buf) {
            return -1;
        }
        ffind->namebuf = buf;
        ffind->namebuflen = namelen;
    }
    memcpy(ffind->namebuf, fd.cFileName, namelen + 1);
    fentry->Filename = ffind->namebuf;
    return 1;
}

extern "C" void LbFileFindEnd(struct TbFileFind * ffind)
{
    if (ffind) {
        FindClose(ffind->handle);
        free(ffind->namebuf);
        free(ffind);
    }
}
