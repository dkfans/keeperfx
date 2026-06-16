#ifndef PLATFORM_LINUX_H
#define PLATFORM_LINUX_H

#include "IPlatform.h"

/** Linux (POSIX) implementation of IPlatform.
 *
 *  Handles POSIX file I/O with case-insensitive file-name matching, dirent-
 *  based directory enumeration, and Linux OS query functions.
 *  Instantiated once at the start of main() and registered with
 *  PlatformManager::Set() before kfxmain() is called.
 */
class PlatformLinux : public IPlatform {
public:
    // ----- OS information -----
    const char* GetOSVersion() const override;
    const void* GetImageBase() const override;
    const char* GetWineVersion() const override;
    const char* GetWineHost() const override;

    // ----- Crash / error parachute -----
    void ErrorParachuteInstall() override;
    void ErrorParachuteUpdate() override;

    // ----- File enumeration -----
    TbFileFind* FileFindFirst(const char* filespec, TbFileEntry* entry) override;
    int32_t     FileFindNext(TbFileFind* handle, TbFileEntry* entry) override;
    void        FileFindEnd(TbFileFind* handle) override;

    // ----- File system helpers -----
    TbBool FileExists(const char* path) const override;
    int    MakeDirectory(const char* path) override;
    int    GetCurrentDirectory(char* buf, unsigned long buflen) override;

    // ----- File I/O -----
    TbFileHandle FileOpen(const char* fname, unsigned char accmode) override;
    int          FileClose(TbFileHandle handle) override;
    int          FileRead(TbFileHandle handle, void* buf, unsigned long len) override;
    long         FileWrite(TbFileHandle handle, const void* buf, unsigned long len) override;
    int          FileSeek(TbFileHandle handle, long offset, unsigned char origin) override;
    int          FilePosition(TbFileHandle handle) override;
    TbBool       FileEof(TbFileHandle handle) override;
    short        FileFlush(TbFileHandle handle) override;
    long         FileLength(const char* fname) override;
    int          FileDelete(const char* fname) override;

    // ----- Path provider (stubs — not yet integrated) -----
    void        SetArgv(int argc, char** argv) override;
    const char* GetDataPath() const override;
    const char* GetSavePath() const override;
    const char* GetUserPrefDir() override;

    // ----- CDROM / Redbook audio (stubs — no CDROM support on Linux) -----
    void   SetRedbookVolume(SoundVolume vol) override;
    TbBool PlayRedbookTrack(int track) override;
    void   PauseRedbookTrack() override;
    void   ResumeRedbookTrack() override;
    void   StopRedbookTrack() override;
};

#endif // PLATFORM_LINUX_H
