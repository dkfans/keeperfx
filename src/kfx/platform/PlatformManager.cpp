/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file PlatformManager.cpp
 *     C-callable windowing facade delegating to the desktop window system.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/platform/PlatformManager.h"
#include "kfx/platform/WindowSystemSDL.h"
#include "kfx/platform/IPlatform.h"
#include "kfx/platform/PlatformWindows.h"
#include "kfx/platform/PlatformLinux.h"
#include "post_inc.h"

/******************************************************************************/

IWindowSystem* IPlatform::GetWindowSystem() { return GetSDLWindowSystem(); }

IPlatform* GetPlatform()
{
#if defined(_WIN32)
    static PlatformWindows s_platform;
#else
    static PlatformLinux s_platform;
#endif
    return &s_platform;
}

/******************************************************************************/

extern "C" int PlatformManager_InitVideo(void)
{
    return GetPlatform()->VideoInit() ? 1 : 0;
}

extern "C" int PlatformManager_HasWindow(void)
{
    IWindowSystem* ws = GetSDLWindowSystem();
    return (ws && ws->HasWindow()) ? 1 : 0;
}

extern "C" int PlatformManager_GetIsAppActive(void)
{
    IWindowSystem* ws = GetSDLWindowSystem();
    return (ws && ws->IsAppActive()) ? 1 : 0;
}

extern "C" int PlatformManager_OwnsDisplay(void)             { return GetPlatform()->OwnsDisplay() ? 1 : 0; }
extern "C" int PlatformManager_ForcesAllModesAvailable(void) { return GetPlatform()->ForcesAllModesAvailable() ? 1 : 0; }

extern "C" unsigned int PlatformManager_GetWindowFlags(void)
{
    IWindowSystem* ws = GetSDLWindowSystem();
    return ws ? ws->GetWindowFlags() : 0;
}

extern "C" void PlatformManager_GetWindowSize(int* out_w, int* out_h)
{
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    IWindowSystem* ws = GetSDLWindowSystem();
    if (ws) ws->GetWindowSize(out_w, out_h);
}

extern "C" int PlatformManager_GetWindowDisplayIndex(void)
{
    IWindowSystem* ws = GetSDLWindowSystem();
    return ws ? ws->GetWindowDisplayIndex() : -1;
}

extern "C" int PlatformManager_GetNumVideoDisplays(void)
{
    IWindowSystem* ws = GetSDLWindowSystem();
    return ws ? ws->GetNumVideoDisplays() : 0;
}

extern "C" int PlatformManager_GetDesktopDisplayMode(int display, int* out_w, int* out_h)
{
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    IWindowSystem* ws = GetSDLWindowSystem();
    return ws ? ws->GetDesktopDisplayMode(display, out_w, out_h) : -1;
}

extern "C" int PlatformManager_GetDisplayBounds(int display, int* out_x, int* out_y, int* out_w, int* out_h)
{
    if (out_x) *out_x = 0;
    if (out_y) *out_y = 0;
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    IWindowSystem* ws = GetSDLWindowSystem();
    return ws ? ws->GetDisplayBounds(display, out_x, out_y, out_w, out_h) : -1;
}

extern "C" int PlatformManager_GetClosestDisplayMode(int display, int desired_w, int desired_h, int* out_w, int* out_h)
{
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    IWindowSystem* ws = GetSDLWindowSystem();
    return ws ? ws->GetClosestDisplayMode(display, desired_w, desired_h, out_w, out_h) : 0;
}

extern "C" int PlatformManager_SetWindowDisplayMode(int w, int h)
{
    IWindowSystem* ws = GetSDLWindowSystem();
    return ws ? ws->SetWindowDisplayMode(w, h) : -1;
}

extern "C" void PlatformManager_SetWindowSize(int w, int h)
{
    IWindowSystem* ws = GetSDLWindowSystem();
    if (ws) ws->SetWindowSize(w, h);
}

extern "C" int PlatformManager_SetWindowFullscreen(unsigned int flags)
{
    IWindowSystem* ws = GetSDLWindowSystem();
    return ws ? ws->SetWindowFullscreen(flags) : -1;
}

extern "C" void PlatformManager_SetWindowBordered(int bordered)
{
    IWindowSystem* ws = GetSDLWindowSystem();
    if (ws) ws->SetWindowBordered(bordered);
}

extern "C" void PlatformManager_SetWindowPosition(int x, int y)
{
    IWindowSystem* ws = GetSDLWindowSystem();
    if (ws) ws->SetWindowPosition(x, y);
}

extern "C" int PlatformManager_CreateWindow(const char* title, int x, int y, int w, int h, unsigned int flags)
{
    IWindowSystem* ws = GetSDLWindowSystem();
    return (ws && ws->CreateWindow(title, x, y, w, h, flags)) ? 1 : 0;
}

extern "C" void PlatformManager_WarpCursor(int x, int y)
{
    IWindowSystem* ws = GetSDLWindowSystem();
    if (ws) ws->WarpCursor(x, y);
}

extern "C" int PlatformManager_RecreateWindowForSoftwareRenderer(void)
{
    IWindowSystem* ws = GetSDLWindowSystem();
    return (ws && ws->RecreateForSoftwareRenderer()) ? 1 : 0;
}

extern "C" int PlatformManager_GetDisplayRefreshRate(void)
{
    IWindowSystem* ws = GetSDLWindowSystem();
    return ws ? ws->GetDisplayRefreshRate() : 0;
}
