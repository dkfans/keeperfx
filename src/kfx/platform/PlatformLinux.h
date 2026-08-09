#ifndef PLATFORM_LINUX_H
#define PLATFORM_LINUX_H

#include "kfx/platform/IPlatform.h"

/** Linux desktop platform. */
class PlatformLinux : public IPlatform {
public:
    const char* GetOSVersion() const override;
    const void* GetImageBase() const override;
    const char* GetWineVersion() const override;
    const char* GetWineHost() const override;

    bool VideoInit() override;
};

#endif // PLATFORM_LINUX_H
