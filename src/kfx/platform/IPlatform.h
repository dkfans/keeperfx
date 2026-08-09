#ifndef IPLATFORM_H
#define IPLATFORM_H

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
