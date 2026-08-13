#ifndef IPLATFORM_H
#define IPLATFORM_H

#include "bflib_basics.h" // TbBool
#include "bflib_sound.h"  // SoundVolume

class IWindowSystem;
struct TbFileFind;
struct TbFileEntry;

/** Per-OS platform services. PlatformManager selects the concrete
 *  implementation (PlatformWindows / PlatformLinux) for the build target;
 *  engine code reaches it only through the PlatformManager_* facade. */
class IPlatform {
public:
    virtual ~IPlatform() = default;

    // ----- OS information -----
    virtual const char* GetOSVersion() const = 0;
    virtual const void* GetImageBase() const = 0;
    virtual const char* GetWineVersion() const = 0; // nullptr when not running under Wine
    virtual const char* GetWineHost() const = 0;    // nullptr when not running under Wine

    virtual TbFileFind* FileFindFirst(const char* filespec, TbFileEntry* entry) = 0;

    // ----- Redbook (CD) audio -----
    virtual void   SetRedbookVolume(SoundVolume vol) = 0;
    virtual TbBool PlayRedbookTrack(int track) = 0;
    virtual void   PauseRedbookTrack() = 0;
    virtual void   ResumeRedbookTrack() = 0;
    virtual void   StopRedbookTrack() = 0;

    // ----- Steam ----- (Linux stubbed; real libsteam_api.so is future work)
    virtual int  InitSteam() = 0;   // 0 ok, -1 unsupported, >0 failure
    virtual void ShutdownSteam() = 0;

    /** Initialise the display subsystem. Per-OS: Windows adjusts SDL hints
     *  before SDL_Init, Linux initialises plainly. Returns false on failure. */
    virtual bool VideoInit() = 0;

    /** True on consoles that own the display exclusively. Desktop: false. */
    virtual bool OwnsDisplay() const { return false; }

    /** True where all registered video modes are reported available without
     *  querying SDL (consoles that own the display). Desktop: false. */
    virtual bool ForcesAllModesAvailable() const { return false; }

    /** The window system backing this platform (SDL desktop backend). */
    virtual IWindowSystem* GetWindowSystem();
};

/** The platform implementation selected for this build target. */
IPlatform* GetPlatform();

#endif // IPLATFORM_H
