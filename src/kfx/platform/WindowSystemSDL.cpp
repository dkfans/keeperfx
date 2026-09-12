/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file WindowSystemSDL.cpp
 *     SDL3 desktop window-system implementation of IWindowSystem.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/platform/WindowSystemSDL.h"
#include "kfx/platform/PlatformGLHdrWin.h"
#include "bflib_basics.h"
#include "bflib_video.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "post_inc.h"

#ifndef _WIN32
extern "C" const unsigned char kfx_window_icon_png[];
extern "C" const unsigned int kfx_window_icon_png_size;
#endif

static void ApplyWindowIcon(SDL_Window *window)
{
#ifndef _WIN32
    SDL_Surface *icon = IMG_Load_IO(SDL_IOFromConstMem(kfx_window_icon_png, kfx_window_icon_png_size), true);
    if (icon == nullptr) {
        WARNLOG("Failed to load window icon: %s", SDL_GetError());
        return;
    }
    if (!SDL_SetWindowIcon(window, icon)) {
        WARNLOG("Failed to window icon: %s", SDL_GetError());
    }
    SDL_DestroySurface(icon);
#else
    // MS Windows executable gets icon from .rc resource
#endif
}

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

bool WindowSystemSDL::IsAppActive() const { return m_appActive; }
void WindowSystemSDL::OnFocusGained()     { m_appActive = true;  ApplyOsCursorPolicy(); }
void WindowSystemSDL::OnFocusLost()       { m_appActive = false; ApplyOsCursorPolicy(); }

void WindowSystemSDL::ApplyOsCursorPolicy()
{
    if (!m_window)
        return;
    const bool focused = (SDL_GetWindowFlags(m_window) & SDL_WINDOW_INPUT_FOCUS) != 0;
    if (focused)
        SDL_HideCursor();
    else
        SDL_ShowCursor();
}

void WindowSystemSDL::SetCursorGrab(bool grab)
{
    if (!m_window)
        return;
    if (m_useRelativeMouse) {
        // deltas straight from the OS.
        SDL_SetWindowRelativeMouseMode(m_window, grab);
    }
    else{
        // confine the cursor to the window and re-center it.
        SDL_SetWindowMouseGrab(m_window, grab);
        if (grab)
        {
            int w = 0, h = 0;
            SDL_GetWindowSize(m_window, &w, &h);
            SDL_WarpMouseInWindow(m_window, w / 2.0f, h / 2.0f);
        }
    }
    ApplyOsCursorPolicy();
}

void WindowSystemSDL::SetUseRelativeMouse(bool relative)
{
    if (relative == m_useRelativeMouse)
        return;

    if (m_window)
    {
        if (m_useRelativeMouse)
            SDL_SetWindowRelativeMouseMode(m_window, false);
        else
            SDL_SetWindowMouseGrab(m_window, false);
    }
    m_useRelativeMouse = relative;
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
    if (!m_window)
        return;
    SDL_WarpMouseInWindow(m_window, (float)x, (float)y);
}

bool WindowSystemSDL::IsCursorInWindow() const
{
    if (!m_window)
        return false;
    float gx = 0.0f, gy = 0.0f;
    SDL_GetGlobalMouseState(&gx, &gy);
    int wx = 0, wy = 0, ww = 0, wh = 0;
    SDL_GetWindowPosition(m_window, &wx, &wy);
    SDL_GetWindowSize(m_window, &ww, &wh);
    return gx >= wx && gx < wx + ww && gy >= wy && gy < wy + wh;
}

// ----- Window management -----

bool WindowSystemSDL::HasWindow() const          { return m_window != nullptr; }
SDL_Window* WindowSystemSDL::GetSDLWindow() const { return m_window; }

unsigned int WindowSystemSDL::GetWindowFlags() const
{
    if (!m_window)
        return 0;
    SDL_WindowFlags sdl_flags = SDL_GetWindowFlags(m_window);
    unsigned int kfx_flags = 0;
    if (sdl_flags & SDL_WINDOW_FULLSCREEN)
    {
        // SDL3: a NULL fullscreen mode means desktop (borderless) fullscreen;
        // a non-NULL mode means an exclusive video mode.
        if (SDL_GetWindowFullscreenMode(m_window) == nullptr)
            kfx_flags |= KFX_WF_FULLSCREEN_DESKTOP;
        else
            kfx_flags |= KFX_WF_FULLSCREEN_EXCLUSIVE;
    }
    if (sdl_flags & SDL_WINDOW_BORDERLESS) kfx_flags |= KFX_WF_BORDERLESS;
    if (sdl_flags & SDL_WINDOW_HIDDEN)     kfx_flags |= KFX_WF_HIDDEN;
    return kfx_flags;
}

void WindowSystemSDL::GetWindowSize(int* out_w, int* out_h) const
{
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (m_window == nullptr)
        return;
    SDL_GetWindowSize(m_window, out_w, out_h);
}

int WindowSystemSDL::GetWindowDisplayIndex() const
{
    if (m_window)
        return (int)SDL_GetDisplayForWindow(m_window);

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
    if (m_window == nullptr)
        return -1;
    SDL_DisplayID disp_id = SDL_GetDisplayForWindow(m_window);
    SDL_DisplayMode dm = {};
    if (SDL_GetClosestFullscreenDisplayMode(disp_id, w, h, 0.0f, false, &dm))
        return SDL_SetWindowFullscreenMode(m_window, &dm) ? 0 : -1;
    // No matching exclusive mode — fall back to desktop (borderless) fullscreen.
    return SDL_SetWindowFullscreenMode(m_window, nullptr) ? 0 : -1;
}

void WindowSystemSDL::SetWindowSize(int w, int h)
{
    if (m_window != nullptr)
        SDL_SetWindowSize(m_window, w, h);
}

int WindowSystemSDL::SetWindowFullscreen(unsigned int flags)
{
    if (!m_window)
        return -1;
    if (flags == 0)
        return SDL_SetWindowFullscreen(m_window, false) ? 0 : -1; // windowed
    if (flags & KFX_WF_FULLSCREEN_DESKTOP)
    {
        // Desktop (borderless) fullscreen at the native resolution.
        SDL_SetWindowFullscreenMode(m_window, nullptr);
        int result = SDL_SetWindowFullscreen(m_window, true) ? 0 : -1;
#ifdef _WIN32
        // A desktop-fullscreen GL swapchain is exactly the shape DXGI's
        // Independent Flip targets (see PlatformGLHdrWin.h). RendererOpenGL::
        // Init() already tries this mitigation once, right after the GL
        // context is created. Re-applying here, on every fullscreen-state
        // change, is what actually matters for e.g. an in-game switch back
        // into desktop fullscreen after windowed play; the Init()-time call
        // and this one are both idempotent (safe no-ops if already applied),
        // so calling both is deliberate belt-and-suspenders, not redundant risk.
        if (result == 0 && (SDL_GetWindowFlags(m_window) & SDL_WINDOW_OPENGL))
            PlatformGLHdrWin_OnContextReady(m_window, true);
#endif
        return result;
    }
    // Exclusive fullscreen: the specific mode is applied via SetWindowDisplayMode();
    // here we just enter fullscreen.
    return SDL_SetWindowFullscreen(m_window, true) ? 0 : -1;
}

void WindowSystemSDL::SetWindowBordered(int bordered)
{
    if (m_window != nullptr)
        SDL_SetWindowBordered(m_window, bordered ? true : false);
}

void WindowSystemSDL::SetWindowPosition(int x, int y)
{
    if (m_window != nullptr)
        SDL_SetWindowPosition(m_window, x, y);
}

bool WindowSystemSDL::CreateWindow(const char* title, int x, int y, int w, int h, unsigned int flags)
{
    // Translate KfxWindowFlags to SDL3 window creation flags.
    SDL_WindowFlags sdl3_flags = 0;
    if (flags & (KFX_WF_FULLSCREEN_EXCLUSIVE | KFX_WF_FULLSCREEN_DESKTOP))
        sdl3_flags |= SDL_WINDOW_FULLSCREEN;
    if (flags & KFX_WF_BORDERLESS) sdl3_flags |= SDL_WINDOW_BORDERLESS;
    if (flags & KFX_WF_HIDDEN)     sdl3_flags |= SDL_WINDOW_HIDDEN;

    if (flags & KFX_WF_OPENGL)
    {
        sdl3_flags |= SDL_WINDOW_OPENGL;
        // Created hidden regardless of the caller's KFX_WF_HIDDEN request:
        // SDL3 sets the window's pixel format as part of SDL_CreateWindow,
        // and doing that on a visible window can trigger a DWM
        // composition-pipeline reconfiguration (a visible black flash on
        // HDR displays). RendererOpenGL::Init() shows the window once the
        // GL context is fully created and current -- matches develop's
        // platform_create_gl_context() sequencing.
        sdl3_flags |= SDL_WINDOW_HIDDEN;

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        // World geometry depth-tests tile/flat-poly draws for correct
        // near/far ordering -- request a depth buffer explicitly rather than
        // relying on a driver default, which may be none.
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        // Pin the colour channel sizes explicitly instead of letting SDL/the
        // driver pick a default pixel format. Without this a driver is free to
        // hand back a surface with a different bit layout (or, on an HDR-capable
        // display, an extended-precision one) -- matches develop's
        // platform_gl_sdl3.cpp, which calls out the 8-bit RGBA request as what
        // keeps an HDR display's DWM compositor tone-mapping the game as SDR
        // instead of applying an unwanted brightness boost.
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,   8);
    }

    m_window = SDL_CreateWindow(title, w, h, sdl3_flags);
    if (!m_window)
        return false;
    ApplyWindowIcon(lbWindow);

    // A window created with SDL_WINDOW_FULLSCREEN starts as desktop fullscreen,
    // which is exactly what a desktop-fullscreen mode wants; an exclusive mode is
    // applied later via SetWindowDisplayMode(). Windowed modes get their position.
    if (!(sdl3_flags & SDL_WINDOW_FULLSCREEN))
        SDL_SetWindowPosition(m_window, x, y);
    return true;
}

bool WindowSystemSDL::SetWindowTitle(const char* title)
{
    if (!m_window)
        return false;
    SDL_SetWindowTitle(m_window, title);
    return true;
}

int WindowSystemSDL::GetDisplayRefreshRate() const
{
    if (m_window == nullptr)
        return 0;
    SDL_DisplayID disp_id = SDL_GetDisplayForWindow(m_window);
    if (disp_id == 0)
        return 0;
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(disp_id);
    if (mode && mode->refresh_rate > 0)
        return (int)(mode->refresh_rate + 0.5f);
    return 0;
}
