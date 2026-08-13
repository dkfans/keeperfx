#ifndef PLATFORM_WINDOWS_H
#define PLATFORM_WINDOWS_H

#include "kfx/platform/IPlatform.h"

/** Windows desktop platform. */
class PlatformWindows : public IPlatform {
public:
    const char* GetOSVersion() const override;
    const void* GetImageBase() const override;
    const char* GetWineVersion() const override;
    const char* GetWineHost() const override;

    TbFileFind* FileFindFirst(const char* filespec, TbFileEntry* entry) override;

    void   SetRedbookVolume(SoundVolume vol) override;
    TbBool PlayRedbookTrack(int track) override;
    void   PauseRedbookTrack() override;
    void   ResumeRedbookTrack() override;
    void   StopRedbookTrack() override;

    int  InitSteam() override;
    void ShutdownSteam() override;

    bool VideoInit() override;
};

#endif // PLATFORM_WINDOWS_H
