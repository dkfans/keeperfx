/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file WindowCompositorWin.cpp
 *     Windows DWM composition and display diagnostics for the game window.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/platform/WindowCompositorWin.h"
#include "bflib_basics.h"
#include <SDL3/SDL.h>
#include <dxgi1_6.h>
#include "post_inc.h"

// Extra width past the monitor's right edge. The right edge keeps the window's
// client origin on the monitor origin, so mouse coordinates and bottom-left
// anchored GL drawing stay aligned with the screen.
static const int kCompositedOverhangPx = 2;

static HWND window_hwnd(SDL_Window* window)
{
    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    return (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
}

void WinKeepFullscreenWindowComposited(SDL_Window* window)
{
    HWND hwnd = window_hwnd(window);
    if (hwnd == nullptr)
        return;
    MONITORINFO mi = {};
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), &mi))
        return;
    RECT target = mi.rcMonitor;
    target.right += kCompositedOverhangPx;
    RECT current = {};
    if (GetWindowRect(hwnd, &current) && EqualRect(&current, &target))
        return;
    SetWindowPos(hwnd, nullptr, target.left, target.top,
        target.right - target.left, target.bottom - target.top,
        SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
    LbJustLog("[display] kept composited: window rect (%ld,%ld)-(%ld,%ld)\n",
        target.left, target.top, target.right, target.bottom);
}

/******************************************************************************/

static const char *dxgi_colorspace_name(DXGI_COLOR_SPACE_TYPE cs)
{
    switch (cs) {
    case DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709:      return "sRGB (P709 G22 full)";
    case DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709:      return "linear RGB (P709 G10 full)";
    case DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P709:    return "P709 G22 studio";
    case DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P2020:   return "P2020 G22 studio";
    case DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020:   return "HDR10 P2020 PQ full";
    case DXGI_COLOR_SPACE_RGB_STUDIO_G2084_NONE_P2020: return "P2020 PQ studio";
    case DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P2020:     return "WCG SDR (P2020 G22 full)";
    default:                                            return "unknown";
    }
}

static void log_dxgi_outputs(HWND hwnd)
{
    IDXGIFactory1 *factory = nullptr;
    if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)&factory)))
    {
        LbJustLog("[display] CreateDXGIFactory1 failed\n");
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
            char devname[32] = {};
            WideCharToMultiByte(CP_UTF8, 0, odesc.DeviceName, -1, devname, sizeof(devname), nullptr, nullptr);
            IDXGIOutput6 *out6 = nullptr;
            DXGI_OUTPUT_DESC1 d1 = {};
            const bool have_desc1 = SUCCEEDED(output->QueryInterface(__uuidof(IDXGIOutput6), (void **)&out6))
                && SUCCEEDED(out6->GetDesc1(&d1));
            if (have_desc1)
                LbJustLog("[display] adapter[%u] output[%u] %s%s: %s, %u bpc, max %.0f nits\n",
                    ai, oi, devname, (odesc.Monitor == hMon) ? " (game window)" : "",
                    dxgi_colorspace_name(d1.ColorSpace), d1.BitsPerColor, d1.MaxLuminance);
            else
                LbJustLog("[display] adapter[%u] output[%u] %s%s\n",
                    ai, oi, devname, (odesc.Monitor == hMon) ? " (game window)" : "");
            if (out6 != nullptr)
                out6->Release();
            output->Release();
            ++oi;
        }
        adapter->Release();
        ++ai;
    }
    factory->Release();
}

void WinLogDisplayDiagnostics(SDL_Window* window)
{
    HWND hwnd = window_hwnd(window);
    if (hwnd == nullptr)
        return;

    log_dxgi_outputs(hwnd);

    HDC hdc = GetDC(hwnd);
    if (hdc)
    {
        int pfIdx = GetPixelFormat(hdc);
        PIXELFORMATDESCRIPTOR pfd = {};
        if (pfIdx > 0 && DescribePixelFormat(hdc, pfIdx, sizeof(pfd), &pfd))
            LbJustLog("[display] pixel format %d: R%u G%u B%u A%u depth=%u flags=0x%08lX\n",
                pfIdx, (unsigned)pfd.cRedBits, (unsigned)pfd.cGreenBits,
                (unsigned)pfd.cBlueBits, (unsigned)pfd.cAlphaBits,
                (unsigned)pfd.cDepthBits, pfd.dwFlags);
        ReleaseDC(hwnd, hdc);
    }

    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    LbJustLog("[display] window HDR_enabled=%d SDR_white=%.3f HDR_headroom=%.3f\n",
        SDL_GetBooleanProperty(props, SDL_PROP_WINDOW_HDR_ENABLED_BOOLEAN, false) ? 1 : 0,
        SDL_GetFloatProperty(props, SDL_PROP_WINDOW_SDR_WHITE_LEVEL_FLOAT, 1.0f),
        SDL_GetFloatProperty(props, SDL_PROP_WINDOW_HDR_HEADROOM_FLOAT, 1.0f));
}
