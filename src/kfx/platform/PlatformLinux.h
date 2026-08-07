#ifndef PLATFORM_LINUX_H
#define PLATFORM_LINUX_H

#include "kfx/platform/IPlatform.h"

/** Linux desktop platform. */
class PlatformLinux : public IPlatform {
public:
    bool VideoInit() override;
};

#endif // PLATFORM_LINUX_H
