/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file WindowSystemSDL.cpp
 *     SDL3 desktop window-system implementation of IWindowSystem.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "platform/WindowSystemSDL.h"
#include "bflib_basics.h"
#include "bflib_video.h"
#include <SDL3/SDL.h>
#include "post_inc.h"

/******************************************************************************/

static WindowSystemSDL s_sdlWindowSystem;

WindowSystemSDL* GetSDLWindowSystem()
{
    return &s_sdlWindowSystem;
}

// Translate a 0-based display index (as used by the span-all-displays loop in
// bflib_video.c) into an SDL3 display ID. Falls back to the primary display.
static SDL_DisplayID display_index_to_id(int index)
{
    int count = 0;
    SDL_DisplayID* displays = SDL_GetDisplays(&count);
    SDL_DisplayID id = (displays && index >= 0 && index < count)
                       ? displays[index]
                       : SDL_GetPrimaryDisplay();
    if (displays) SDL_free(displays);
    return id;
}

bool WindowSystemSDL::InitVideo()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        return false;
    atexit(SDL_Quit);
    return true;
}

bool WindowSystemSDL::IsAppActive() const { return m_appActive; }
void WindowSystemSDL::OnFocusGained()     { m_appActive = true; }
void WindowSystemSDL::OnFocusLost()       { m_appActive = false; }

void WindowSystemSDL::SetCursorGrab(bool grab)
{
    if (SDL_getenv("NO_RELATIVE_MOUSE") == nullptr)
        SDL_SetWindowRelativeMouseMode(lbWindow, grab);
}

void WindowSystemSDL::SetCursorVisible(bool visible)
{
    if (visible)
        SDL_ShowCursor();
    else
        SDL_HideCursor();
}

void WindowSystemSDL::WarpCursor(int x, int y)
{
    SDL_WarpMouseInWindow(lbWindow, (float)x, (float)y);
}

// ----- Window management -----

bool WindowSystemSDL::HasWindow() const          { return lbWindow != nullptr; }
SDL_Window* WindowSystemSDL::GetSDLWindow() const { return lbWindow; }

unsigned int WindowSystemSDL::GetWindowFlags() const
{
    if (!lbWindow)
        return 0;
    SDL_WindowFlags sdl_flags = SDL_GetWindowFlags(lbWindow);
    unsigned int kfx_flags = 0;
    if (sdl_flags & SDL_WINDOW_FULLSCREEN)
    {
        // SDL3: a NULL fullscreen mode means desktop (borderless) fullscreen;
        // a non-NULL mode means an exclusive video mode.
        if (SDL_GetWindowFullscreenMode(lbWindow) == nullptr)
            kfx_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        else
            kfx_flags |= (unsigned int)SDL_WINDOW_FULLSCREEN;
    }
    if (sdl_flags & SDL_WINDOW_BORDERLESS) kfx_flags |= SDL_WINDOW_BORDERLESS;
    if (sdl_flags & SDL_WINDOW_HIDDEN)     kfx_flags |= SDL_WINDOW_HIDDEN;
    return kfx_flags;
}

void WindowSystemSDL::GetWindowSize(int* out_w, int* out_h) const
{
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (lbWindow == nullptr)
        return;
    SDL_GetWindowSize(lbWindow, out_w, out_h);
}

int WindowSystemSDL::GetWindowDisplayIndex() const
{
    if (lbWindow)
        return (int)SDL_GetDisplayForWindow(lbWindow);

    return (int)SDL_GetPrimaryDisplay();
}

int WindowSystemSDL::GetNumVideoDisplays() const
{
    int count = 0;
    SDL_DisplayID* displays = SDL_GetDisplays(&count);
    if (displays)
        SDL_free(displays);
    return count;
}

int WindowSystemSDL::GetDesktopDisplayMode(int display, int* out_w, int* out_h) const
{
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    // SDL3 display IDs start at 1; 0 means "not set yet" — fall back to primary.
    SDL_DisplayID disp_id = (display > 0) ? (SDL_DisplayID)display : SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(disp_id);
    if (!mode)
        return -1;
    if (out_w) *out_w = mode->w;
    if (out_h) *out_h = mode->h;
    return 0;
}

int WindowSystemSDL::GetDisplayBounds(int display, int* out_x, int* out_y, int* out_w, int* out_h) const
{
    if (out_x) *out_x = 0;
    if (out_y) *out_y = 0;
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    SDL_DisplayID disp_id = display_index_to_id(display);
    SDL_Rect rect = {0, 0, 0, 0};
    if (!SDL_GetDisplayBounds(disp_id, &rect))
        return -1;
    if (out_x) *out_x = rect.x;
    if (out_y) *out_y = rect.y;
    if (out_w) *out_w = rect.w;
    if (out_h) *out_h = rect.h;
    return 0;
}

int WindowSystemSDL::GetClosestDisplayMode(int display, int desired_w, int desired_h, int* out_w, int* out_h) const
{
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    SDL_DisplayID disp_id = (display > 0) ? (SDL_DisplayID)display : SDL_GetPrimaryDisplay();
    SDL_DisplayMode closest = {};
    if (!SDL_GetClosestFullscreenDisplayMode(disp_id, desired_w, desired_h, 0.0f, false, &closest))
        return 0;
    if (out_w) *out_w = closest.w;
    if (out_h) *out_h = closest.h;
    return 1;
}

int WindowSystemSDL::SetWindowDisplayMode(int w, int h)
{
    if (lbWindow == nullptr)
        return -1;
    SDL_DisplayID disp_id = SDL_GetDisplayForWindow(lbWindow);
    SDL_DisplayMode dm = {};
    if (SDL_GetClosestFullscreenDisplayMode(disp_id, w, h, 0.0f, false, &dm))
        return SDL_SetWindowFullscreenMode(lbWindow, &dm) ? 0 : -1;
    // No matching exclusive mode — fall back to desktop (borderless) fullscreen.
    return SDL_SetWindowFullscreenMode(lbWindow, nullptr) ? 0 : -1;
}

void WindowSystemSDL::SetWindowSize(int w, int h)
{
    if (lbWindow != nullptr)
        SDL_SetWindowSize(lbWindow, w, h);
}

int WindowSystemSDL::SetWindowFullscreen(unsigned int flags)
{
    if (!lbWindow)
        return -1;
    if (flags == 0)
        return SDL_SetWindowFullscreen(lbWindow, false) ? 0 : -1; // windowed
    if ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP)
    {
        // Desktop (borderless) fullscreen at the native resolution.
        SDL_SetWindowFullscreenMode(lbWindow, nullptr);
        return SDL_SetWindowFullscreen(lbWindow, true) ? 0 : -1;
    }
    // Exclusive fullscreen: the specific mode is applied via SetWindowDisplayMode();
    // here we just enter fullscreen.
    return SDL_SetWindowFullscreen(lbWindow, true) ? 0 : -1;
}

void WindowSystemSDL::SetWindowBordered(int bordered)
{
    if (lbWindow != nullptr)
        SDL_SetWindowBordered(lbWindow, bordered ? true : false);
}

void WindowSystemSDL::SetWindowPosition(int x, int y)
{
    if (lbWindow != nullptr)
        SDL_SetWindowPosition(lbWindow, x, y);
}

bool WindowSystemSDL::CreateWindow(const char* title, int x, int y, int w, int h, unsigned int flags)
{
    // Translate KFX/SDL2-compat flags to SDL3 window creation flags.
    SDL_WindowFlags sdl3_flags = 0;
    const bool is_desktop_fs = (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP;
    const bool is_fullscreen  = is_desktop_fs || (flags & (unsigned int)SDL_WINDOW_FULLSCREEN);
    if (is_fullscreen)                  sdl3_flags |= SDL_WINDOW_FULLSCREEN;
    if (flags & SDL_WINDOW_BORDERLESS)  sdl3_flags |= SDL_WINDOW_BORDERLESS;
    if (flags & SDL_WINDOW_HIDDEN)      sdl3_flags |= SDL_WINDOW_HIDDEN;

    lbWindow = SDL_CreateWindow(title, w, h, sdl3_flags);
    if (!lbWindow)
        return false;

    // A window created with SDL_WINDOW_FULLSCREEN starts as desktop fullscreen,
    // which is exactly what a desktop-fullscreen mode wants; an exclusive mode is
    // applied later via SetWindowDisplayMode(). Windowed modes get their position.
    if (!(sdl3_flags & SDL_WINDOW_FULLSCREEN))
        SDL_SetWindowPosition(lbWindow, x, y);
    return true;
}

bool WindowSystemSDL::RecreateForSoftwareRenderer()
{
    if (lbWindow == nullptr)
        return false;
    SDL_WindowFlags cur_flags = SDL_GetWindowFlags(lbWindow);
    if (!(cur_flags & (SDL_WINDOW_OPENGL | SDL_WINDOW_VULKAN)))
        return true;

    int w = 0, h = 0;
    SDL_GetWindowSize(lbWindow, &w, &h);
    int x = SDL_WINDOWPOS_UNDEFINED, y = SDL_WINDOWPOS_UNDEFINED;
    SDL_GetWindowPosition(lbWindow, &x, &y);
    char title[256];
    { const char* cur = SDL_GetWindowTitle(lbWindow); snprintf(title, sizeof(title), "%s", cur ? cur : ""); }
    SDL_WindowFlags new_flags = cur_flags & ~(SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_VULKAN);

    SDL_DestroyWindow(lbWindow);
    lbWindow = SDL_CreateWindow(title, w, h, new_flags);
    if (!lbWindow) {
        ERRORLOG("WindowSystemSDL::RecreateForSoftwareRenderer failed: %s", SDL_GetError());
        return false;
    }
    SDL_SetWindowPosition(lbWindow, x, y);
    SDL_ShowWindow(lbWindow);
    return true;
}

bool WindowSystemSDL::RecreateForVulkanRenderer()
{
    // No Vulkan window in this build; kept for interface parity with develop.
    if (lbWindow == nullptr)
        return false;
    if (!(SDL_GetWindowFlags(lbWindow) & SDL_WINDOW_VULKAN))
        return true;
    return true;
}

int WindowSystemSDL::GetDisplayRefreshRate() const
{
    if (lbWindow == nullptr)
        return 0;
    SDL_DisplayID disp_id = SDL_GetDisplayForWindow(lbWindow);
    if (disp_id == 0)
        return 0;
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(disp_id);
    if (mode && mode->refresh_rate > 0)
        return (int)(mode->refresh_rate + 0.5f);
    return 0;
}
