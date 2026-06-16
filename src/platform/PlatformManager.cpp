#include "../pre_inc.h"
#include "PlatformManager.h"
#include "../post_inc.h"

// ----- Singleton storage -----

IPlatform* PlatformManager::s_instance = nullptr;

IPlatform* PlatformManager::Get()
{
    return s_instance;
}

void PlatformManager::Set(IPlatform* platform)
{
    delete s_instance;
    s_instance = platform;
}

// ----- Default IWindowSystem (used by all desktop platforms) -----

static IWindowSystem s_defaultWindowSystem;

IWindowSystem* IPlatform::GetWindowSystem()
{
    return &s_defaultWindowSystem;
}

// ----- Basic platform-info wrappers (moved from windows.cpp / linux.cpp) -----

extern "C" const char* get_os_version()
{
    return PlatformManager::Get()->GetOSVersion();
}

extern "C" const void* get_image_base()
{
    return PlatformManager::Get()->GetImageBase();
}

extern "C" const char* get_wine_version()
{
    return PlatformManager::Get()->GetWineVersion();
}

extern "C" const char* get_wine_host()
{
    return PlatformManager::Get()->GetWineHost();
}

// ----- File-find wrappers (moved from windows.cpp / linux.cpp) -----

extern "C" TbFileFind* LbFileFindFirst(const char* filespec, TbFileEntry* fentry)
{
    return PlatformManager::Get()->FileFindFirst(filespec, fentry);
}

extern "C" int LbFileFindNext(TbFileFind* ffind, TbFileEntry* fentry)
{
    return PlatformManager::Get()->FileFindNext(ffind, fentry);
}

extern "C" void LbFileFindEnd(TbFileFind* ffind)
{
    PlatformManager::Get()->FileFindEnd(ffind);
}

// ----- C-compatible file-system / file-I/O wrappers -----

extern "C" TbBool PlatformManager_FileExists(const char* path)
{
    return PlatformManager::Get()->FileExists(path);
}

extern "C" int PlatformManager_MakeDirectory(const char* path)
{
    return PlatformManager::Get()->MakeDirectory(path);
}

extern "C" int PlatformManager_GetCurrentDirectory(char* buf, unsigned long buflen)
{
    return PlatformManager::Get()->GetCurrentDirectory(buf, buflen);
}

extern "C" TbFileHandle PlatformManager_FileOpen(const char* fname, unsigned char accmode)
{
    return PlatformManager::Get()->FileOpen(fname, accmode);
}

extern "C" int PlatformManager_FileClose(TbFileHandle handle)
{
    return PlatformManager::Get()->FileClose(handle);
}

extern "C" int PlatformManager_FileRead(TbFileHandle handle, void* buf, unsigned long len)
{
    return PlatformManager::Get()->FileRead(handle, buf, len);
}

extern "C" long PlatformManager_FileWrite(TbFileHandle handle, const void* buf, unsigned long len)
{
    return PlatformManager::Get()->FileWrite(handle, buf, len);
}

extern "C" int PlatformManager_FileSeek(TbFileHandle handle, long offset, unsigned char origin)
{
    return PlatformManager::Get()->FileSeek(handle, offset, origin);
}

extern "C" int PlatformManager_FilePosition(TbFileHandle handle)
{
    return PlatformManager::Get()->FilePosition(handle);
}

extern "C" TbBool PlatformManager_FileEof(TbFileHandle handle)
{
    return PlatformManager::Get()->FileEof(handle);
}

extern "C" short PlatformManager_FileFlush(TbFileHandle handle)
{
    return PlatformManager::Get()->FileFlush(handle);
}

extern "C" long PlatformManager_FileLength(const char* fname)
{
    return PlatformManager::Get()->FileLength(fname);
}

extern "C" int PlatformManager_FileDelete(const char* fname)
{
    return PlatformManager::Get()->FileDelete(fname);
}

extern "C" void PlatformManager_LogWrite(const char* message)
{
    IPlatform* p = PlatformManager::Get();
    if (p) p->LogWrite(message);
}
