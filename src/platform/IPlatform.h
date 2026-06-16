#ifndef IPLATFORM_H
#define IPLATFORM_H

#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_sound.h"
#include "IAudioPlatform.h"
#include "IWindowSystem.h"

/** Abstract interface for OS/platform-specific operations.
 *
 *  Implement this class for each target platform and register it with
 *  PlatformManager::Set() at startup.  All callers access functionality
 *  through the C-compatible wrappers in PlatformManager.cpp, keeping
 *  existing C-code call-sites unchanged.
 */
class IPlatform {
public:
    virtual ~IPlatform() = default;

    // ----- OS information -----
    virtual const char* GetOSVersion() const = 0;
    virtual const void* GetImageBase() const = 0;
    virtual const char* GetWineVersion() const = 0;
    virtual const char* GetWineHost() const = 0;

    // ----- Crash / error parachute -----
    /** Install platform-specific crash handlers (SEH on Windows, extra POSIX
     *  signals on Linux/Vita).  Called from LbErrorParachuteInstall() after
     *  the ANSI signal set has been registered. */
    virtual void ErrorParachuteInstall() = 0;
    /** Re-arm the exception filter after any SDL/third-party library resets it. */
    virtual void ErrorParachuteUpdate() = 0;

    // ----- File enumeration -----
    virtual TbFileFind* FileFindFirst(const char* filespec, TbFileEntry* entry) = 0;
    virtual int32_t     FileFindNext(TbFileFind* handle, TbFileEntry* entry) = 0;
    virtual void        FileFindEnd(TbFileFind* handle) = 0;

    // ----- File system helpers -----
    /** Returns true if the file or directory at `path` exists and is accessible. */
    virtual TbBool FileExists(const char* path) const = 0;
    /** Creates a single directory component.  Returns 0 on success, -1 on failure
     *  (EEXIST is NOT treated as failure). */
    virtual int MakeDirectory(const char* path) = 0;
    /** Fills `buf` with the current working directory.  Returns 1 on success,
     *  -1 on failure.  Strips a leading drive letter on platforms that use one
     *  (Windows/Vita ux0: style). */
    virtual int GetCurrentDirectory(char* buf, unsigned long buflen) = 0;

    // ----- File I/O -----
    /** Opens a file.  accmode is one of TbFileMode.  Returns NULL on failure. */
    virtual TbFileHandle FileOpen(const char* fname, unsigned char accmode) = 0;
    /** Closes a file handle.  Returns 0 on success, -1 on failure. */
    virtual int          FileClose(TbFileHandle handle) = 0;
    /** Reads up to `len` bytes.  Returns bytes read, or -1 on error. */
    virtual int          FileRead(TbFileHandle handle, void* buf, unsigned long len) = 0;
    /** Writes `len` bytes.  Returns bytes written, or -1 on error. */
    virtual long         FileWrite(TbFileHandle handle, const void* buf, unsigned long len) = 0;
    /** Seeks within the file (origin: Lb_FILE_SEEK_*).  Returns new position, -1 on error. */
    virtual int          FileSeek(TbFileHandle handle, long offset, unsigned char origin) = 0;
    /** Returns the current file position, or -1 on error. */
    virtual int          FilePosition(TbFileHandle handle) = 0;
    /** Returns non-zero if at end of file. */
    virtual TbBool       FileEof(TbFileHandle handle) = 0;
    /** Flushes buffered writes.  Returns 1 on success, 0 on error. */
    virtual short        FileFlush(TbFileHandle handle) = 0;
    /** Returns the byte length of the named file, or -1 on error. */
    virtual long         FileLength(const char* fname) = 0;
    /** Deletes a file.  Returns 1 on success, -1 on failure. */
    virtual int          FileDelete(const char* fname) = 0;

    // ----- Path provider -----
    /** Called once at startup with the raw argc/argv before any path queries.
     *  Desktop platforms extract the executable directory from argv[0] here.
     *  Homebrew platforms can ignore this. */
    virtual void        SetArgv(int argc, char** argv) = 0;

    /** Root directory where game data files live.
     *  Desktop: directory containing the executable (argv[0]-relative).
     *  Vita:    "ux0:data/keeperfx"
     *  3DS:     "sdmc:/keeperfx"
     *  Switch:  "sdmc:/keeperfx"
     *  Returned pointer is valid for the lifetime of the platform object. */
    virtual const char* GetDataPath() const = 0;

    /** Directory where save files are written.
     *  Desktop: <data_path>/save
     *  Homebrew: same convention under the data path. */
    virtual const char* GetSavePath() const = 0;

    /** OS-appropriate directory for per-user preference files.
     *  Desktop (via SDL_GetPrefPath("keeperfx","keeperfx")):
     *    Windows : %APPDATA%\keeperfx\keeperfx
     *    Linux   : $XDG_DATA_HOME/keeperfx/keeperfx  (~/.local/share/...)
     *    macOS   : ~/Library/Application Support/keeperfx/keeperfx
     *  Vita     : ux0:data/keeperfx
     *  3DS/Switch: sdmc:/keeperfx
     *  Directory is created by the implementation if it does not exist.
     *  Returned pointer is valid for the lifetime of the platform object. */
    virtual const char* GetUserPrefDir() = 0;

    // ----- CDROM / Redbook audio -----
    virtual void   SetRedbookVolume(SoundVolume vol) = 0;
    virtual TbBool PlayRedbookTrack(int track) = 0;
    virtual void   PauseRedbookTrack() = 0;
    virtual void   ResumeRedbookTrack() = 0;
    virtual void   StopRedbookTrack() = 0;

    // ----- Lifecycle init hooks (called from main() before kfxmain()) -----
    /** Initialise the display/GPU subsystem.
     *  SDL desktop: SDL_Init(VIDEO|JOYSTICK) + atexit(SDL_Quit).
     *  Vita: vglInitExtended (vitaGL owns GXM) + SDL_Init(JOYSTICK|GAMECONTROLLER).
     *  Console stubs: no-op. */
    virtual void VideoInit() {}

    /** Initialise the platform input layer.
     *  Vita: calls input_vita_initialize().
     *  SDL desktop: no-op (input arrives via SDL_PollEvent). */
    virtual void InputInit() {}

    /** Initialise the platform audio layer.
     *  Vita: calls audio_vita_initialize().
     *  SDL desktop: no-op. */
    virtual void AudioInit() {}

    /** Return true if all registered video modes should be reported as available
     *  without querying SDL display capabilities.
     *  Vita (vitaGL owns display, SDL VIDEO not initialised): true.
     *  SDL desktop: false. */
    virtual TbBool ForcesAllModesAvailable() const { return false; }

    /** Return true if the platform owns the display directly and bflib should
     *  not create an SDL window.
     *  Vita: true (vitaGL renders directly to GXM).
     *  SDL desktop: false. */
    virtual TbBool OwnsDisplay() const { return false; }

    // ----- Log output routing -----
    /** Called for every formatted log line (prefix already prepended).
     *  The default is a no-op — override to mirror output to a platform
     *  debug channel (e.g. sceClibPrintf on Vita, debugnet on 3DS). */
    virtual void LogWrite(const char* /*message*/) {}

    // ----- Hardware / OS initialisation -----
    /** Called once at the very start of main(), before any other subsystem
     *  is brought up.  Implementations use this for hardware-level setup:
     *  clock maximisation, FPU exception masking, etc.  Default is a no-op. */
    virtual void SystemInit() {}

    /** Called once per rendered frame.  Implementations may use this to
     *  prevent the system from blanking the screen (e.g. sceKernelPowerTick
     *  on Vita).  Default is a no-op. */
    virtual void FrameTick() {}

    /** Called during long loading operations (no frames rendered).
     *  Implementations should reset the auto-suspend/blank timer.
     *  Default is a no-op. */
    virtual void WorkTick() {}

    /** Size of the scratch/arena allocator pool in bytes.
     *  Called once at startup by KfxMemInit().
     *  Default (desktop): 4 MB.  Override to tune for constrained platforms. */
    virtual size_t GetScratchSize() const { return 4 * 1024 * 1024; }

    /** Size of the poly rendering pool in bytes.
     *  Default (desktop): 16 MB.  Override for constrained platforms. */
    virtual size_t GetPolyPoolSize() const { return 16 * 1024 * 1024; }

    // ----- Audio sub-interface -----
    /** Returns the platform audio implementation, or nullptr if the platform
     *  has no native audio path (desktop CI, headless builds).
     *  When nullptr, callers fall back to SDL audio or silent playback. */
    virtual IAudioPlatform* GetAudio() { return nullptr; }

    /** Returns the platform window-system implementation.
     *  Desktop SDL platforms return a WindowSystemSDL instance.
     *  Console platforms (Vita, 3DS, Switch) return a platform-specific or
     *  default IWindowSystem whose HasOSCursor() returns false.
     *  Never returns nullptr — the base implementation returns a static
     *  default IWindowSystem instance with safe no-op behaviour. */
    virtual IWindowSystem* GetWindowSystem();
};

#endif // IPLATFORM_H
