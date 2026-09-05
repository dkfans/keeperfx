/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file PlatformGLHdrWin.cpp
 *     Windows/DXGI HDR-compositor bookkeeping for the OpenGL backend.
 *     See PlatformGLHdrWin.h for why this exists. Ported from develop's
 *     platform_gl_sdl3.cpp (hdr_anchor_.../hdr_watcher_... + DXGI diagnostics),
 *     adapted to this branch's own window/renderer split.
 */
/******************************************************************************/
#ifdef _WIN32
#include "pre_inc.h"
#include "kfx/platform/PlatformGLHdrWin.h"
#include "bflib_basics.h"
#include <SDL3/SDL.h>
#include <dxgi1_6.h>
#include <dwmapi.h>
#include "post_inc.h"

/******************************************************************************/
// DXGI / pixel-format diagnostics
// ---------------------------------------------------------------------------

static const char *dxgi_colorspace_name(DXGI_COLOR_SPACE_TYPE cs)
{
    switch (cs) {
    case DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709:      return "sRGB (P709 G22 full)";
    case DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709:      return "linear RGB (P709 G10 full)";
    case DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P709:    return "P709 G22 studio";
    case DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P2020:   return "P2020 G22 studio";
    case DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020:   return "HDR10 P2020 PQ full [HDR ACTIVE]";
    case DXGI_COLOR_SPACE_RGB_STUDIO_G2084_NONE_P2020: return "P2020 PQ studio";
    case DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P2020:     return "WCG SDR (P2020 G22 full)";
    default:                                            return "unknown";
    }
}

static void log_dxgi_hdr_info(HWND hwnd)
{
    LbJustLog("[GL-diag] --- DXGI display info ---\n");
    IDXGIFactory1 *factory = nullptr;
    if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)&factory)))
    {
        LbJustLog("[GL-diag]   CreateDXGIFactory1 failed\n");
        return;
    }
    HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    UINT ai = 0;
    IDXGIAdapter *adapter = nullptr;
    while (factory->EnumAdapters(ai, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        UINT oi = 0;
        IDXGIOutput *output = nullptr;
        while (adapter->EnumOutputs(oi, &output) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_OUTPUT_DESC odesc;
            output->GetDesc(&odesc);
            const bool isTarget = (odesc.Monitor == hMon);
            char devname[32] = {};
            WideCharToMultiByte(CP_UTF8, 0, odesc.DeviceName, -1, devname, sizeof(devname), nullptr, nullptr);
            LbJustLog("[GL-diag]   adapter[%u] output[%u] %-14s%s\n",
                ai, oi, devname, isTarget ? "  <<< game window" : "");
            IDXGIOutput6 *out6 = nullptr;
            if (SUCCEEDED(output->QueryInterface(__uuidof(IDXGIOutput6), (void **)&out6)))
            {
                DXGI_OUTPUT_DESC1 d1;
                if (SUCCEEDED(out6->GetDesc1(&d1)))
                {
                    LbJustLog("[GL-diag]     ColorSpace    : %d (%s)\n",
                        (int)d1.ColorSpace, dxgi_colorspace_name(d1.ColorSpace));
                    LbJustLog("[GL-diag]     BitsPerColor  : %u\n", d1.BitsPerColor);
                    LbJustLog("[GL-diag]     Luminance     : max=%.1f  min=%.4f  maxFF=%.1f  (nits)\n",
                        d1.MaxLuminance, d1.MinLuminance, d1.MaxFullFrameLuminance);
                }
                out6->Release();
            }
            else
            {
                LbJustLog("[GL-diag]     (IDXGIOutput6 unavailable)\n");
            }
            output->Release();
            ++oi;
        }
        adapter->Release();
        ++ai;
    }
    factory->Release();
}

static void log_window_pixelformat(SDL_Window *window)
{
    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    HWND hwnd = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
    if (!hwnd)
        return;

    log_dxgi_hdr_info(hwnd);

    HDC hdc = GetDC(hwnd);
    if (hdc)
    {
        int pfIdx = GetPixelFormat(hdc);
        PIXELFORMATDESCRIPTOR pfd = {};
        DescribePixelFormat(hdc, pfIdx, sizeof(pfd), &pfd);
        LbJustLog("[GL-diag] SDL3 pixel format %d: R%u G%u B%u A%u  depth=%u  flags=0x%08lX\n",
            pfIdx,
            (unsigned)pfd.cRedBits, (unsigned)pfd.cGreenBits,
            (unsigned)pfd.cBlueBits, (unsigned)pfd.cAlphaBits,
            (unsigned)pfd.cDepthBits, pfd.dwFlags);
        ReleaseDC(hwnd, hdc);
    }

    // Per-window HDR state -- different from the display's own DXGI
    // colorspace logged above. SDL_PROP_WINDOW_HDR_ENABLED_BOOLEAN is true
    // when the DWM HDR compositor is actively treating this window as an
    // HDR surface; SDR_white_level/HDR_headroom govern how DWM tone-maps an
    // SDR (this game's) surface into the HDR10 desktop. A game that looks
    // "washed out"/oversaturated on an HDR desktop despite a correct 8-bit
    // sRGB swapchain is DWM's SDR-in-HDR tone-mapping, not a swapchain bug --
    // this is what would show that, so log it every time, not just on
    // request.
    bool  winHdr      = SDL_GetBooleanProperty(props, SDL_PROP_WINDOW_HDR_ENABLED_BOOLEAN, false);
    float sdrWhite    = SDL_GetFloatProperty(props, SDL_PROP_WINDOW_SDR_WHITE_LEVEL_FLOAT, 1.0f);
    float hdrHeadroom = SDL_GetFloatProperty(props, SDL_PROP_WINDOW_HDR_HEADROOM_FLOAT, 1.0f);
    LbJustLog("[GL-diag] window HDR_enabled=%d  SDR_white=%.3f  HDR_headroom=%.3f%s\n",
        winHdr ? 1 : 0, sdrWhite, hdrHeadroom,
        winHdr ? "  [HDR ACTIVE for this window]" : "  [SDR mode for this window]");
}

// ---------------------------------------------------------------------------
// HDR compositor anchor
// ---------------------------------------------------------------------------
// DXGI can engage "Independent Flip" (direct scanout) when it detects an OpenGL
// swap chain covering the full primary display. Independent Flip bypasses the
// DWM HDR compositor, switching the monitor from HDR to SDR at the first
// buffer swap. With a second DWM-composited window present, DXGI cannot
// switch to Independent Flip (it requires the game to be the sole composited
// surface), so the DWM HDR compositor remains active.
//
// The anchor is a 1x1 transparent WS_EX_TOPMOST window with alpha=1 -- nearly
// invisible but sufficient for DWM to include it in the composition pass.
// WS_EX_TRANSPARENT lets mouse events fall through to the game window.

static HWND s_hdr_anchor = nullptr;

static LRESULT CALLBACK hdr_anchor_wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    return DefWindowProcA(hwnd, msg, wp, lp);
}

static void hdr_anchor_create(void)
{
    if (s_hdr_anchor)
        return;
    HINSTANCE hInst = GetModuleHandleA(nullptr);
    WNDCLASSEXA wc = {};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = hdr_anchor_wndproc;
    wc.hInstance     = hInst;
    wc.lpszClassName = "KFX_HDR_Anchor";
    RegisterClassExA(&wc);
    s_hdr_anchor = CreateWindowExA(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        "KFX_HDR_Anchor", nullptr, WS_POPUP,
        0, 0, 1, 1, nullptr, nullptr, hInst, nullptr);
    if (s_hdr_anchor)
    {
        SetLayeredWindowAttributes(s_hdr_anchor, 0, 1, LWA_ALPHA);
        ShowWindow(s_hdr_anchor, SW_SHOWNA);
        LbJustLog("[GL-diag] HDR anchor: created 1x1 compositor anchor to prevent Independent Flip\n");
    }
    else
    {
        LbJustLog("[GL-diag] HDR anchor: CreateWindowExA failed (error %lu)\n", GetLastError());
    }
}

static void hdr_anchor_destroy(void)
{
    if (s_hdr_anchor)
    {
        DestroyWindow(s_hdr_anchor);
        s_hdr_anchor = nullptr;
    }
}

// ---------------------------------------------------------------------------
// HDR compositor watcher -- polls DXGI color space every 250ms during rendering
// ---------------------------------------------------------------------------
// Independent of SDL's HDR event system: catches any DXGI-level color space
// change even if SDL_EVENT_WINDOW_HDR_STATE_CHANGED doesn't fire.
static IDXGIFactory1 *s_watcher_factory = nullptr;
static int            s_watcher_last_cs  = -99;   // -99 = uninitialized
static ULONGLONG      s_watcher_next_ms  = 0;

static void hdr_watcher_init(void)
{
    if (s_watcher_factory) return;
    if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)&s_watcher_factory)))
        LbJustLog("[HDR-watcher] CreateDXGIFactory1 failed -- watcher disabled\n");
    else
        LbJustLog("[HDR-watcher] started: polling DXGI ColorSpace every 250ms\n");
}

static void hdr_watcher_tick(void)
{
    if (!s_watcher_factory) return;
    ULONGLONG now = GetTickCount64();
    if (now < s_watcher_next_ms) return;
    s_watcher_next_ms = now + 250;

    IDXGIAdapter *adapter = nullptr;
    if (FAILED(s_watcher_factory->EnumAdapters(0, &adapter))) return;
    IDXGIOutput *output = nullptr;
    int cs = -1;
    if (SUCCEEDED(adapter->EnumOutputs(0, &output)))
    {
        IDXGIOutput6 *out6 = nullptr;
        if (SUCCEEDED(output->QueryInterface(__uuidof(IDXGIOutput6), (void **)&out6)))
        {
            DXGI_OUTPUT_DESC1 d1 = {};
            if (SUCCEEDED(out6->GetDesc1(&d1)))
                cs = (int)d1.ColorSpace;
            out6->Release();
        }
        output->Release();
    }
    adapter->Release();

    if (s_watcher_last_cs != -99 && cs != s_watcher_last_cs)
        LbJustLog("[HDR-watcher] @%llums  ColorSpace  %d (%s)  ->  %d (%s)\n",
            (unsigned long long)now,
            s_watcher_last_cs, dxgi_colorspace_name((DXGI_COLOR_SPACE_TYPE)s_watcher_last_cs),
            cs,               dxgi_colorspace_name((DXGI_COLOR_SPACE_TYPE)cs));
    s_watcher_last_cs = cs;
}

static void hdr_watcher_shutdown(void)
{
    if (!s_watcher_factory) return;
    s_watcher_factory->Release();
    s_watcher_factory = nullptr;
    s_watcher_last_cs = -99;
    LbJustLog("[HDR-watcher] stopped\n");
}

/******************************************************************************/

void PlatformGLHdrWin_OnContextReady(SDL_Window* window, bool desktop_fullscreen)
{
    log_window_pixelformat(window);

    // Independent Flip only bypasses the DWM HDR compositor for a swapchain
    // that covers the full display -- i.e. desktop-fullscreen/borderless
    // mode. A bordered window is already DWM-composited, so the anchor and
    // frame-extension trick are unnecessary (and DwmExtendFrameIntoClientArea
    // on a bordered window would fight the window manager's own frame).
    if (desktop_fullscreen)
    {
        SDL_PropertiesID props = SDL_GetWindowProperties(window);
        HWND hwnd = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
        if (hwnd)
        {
            MARGINS m = { -1, -1, -1, -1 };
            HRESULT hr = DwmExtendFrameIntoClientArea(hwnd, &m);
            LbJustLog("[GL-diag] HDR: DwmExtendFrameIntoClientArea -> 0x%08lX (%s)\n",
                (unsigned long)hr,
                SUCCEEDED(hr) ? "forcing DWM compositing" : "failed");
        }
        hdr_anchor_create();
    }
    hdr_watcher_init();
}

void PlatformGLHdrWin_Tick()
{
    hdr_watcher_tick();
}

void PlatformGLHdrWin_Shutdown()
{
    hdr_watcher_shutdown();
    hdr_anchor_destroy();
}

#endif // _WIN32
