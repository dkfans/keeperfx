#ifndef PLATFORM_WINDOWS_H
#define PLATFORM_WINDOWS_H

#include "kfx/platform/IPlatform.h"

/** Windows desktop platform. */
class PlatformWindows : public IPlatform {
public:
    bool VideoInit() override;
};

#endif // PLATFORM_WINDOWS_H
