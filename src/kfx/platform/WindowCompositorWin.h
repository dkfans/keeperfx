#ifndef WINDOWCOMPOSITORWIN_H
#define WINDOWCOMPOSITORWIN_H

struct SDL_Window;

/** Grow a desktop-fullscreen window past the right edge of its monitor. A
 *  window that exactly covers a monitor can have its OpenGL presents taken
 *  straight to the display by the driver, bypassing DWM: an HDR monitor
 *  drops to SDR, and alt-tab previews and capture see stale content. */
void WinKeepFullscreenWindowComposited(SDL_Window* window);

/** Log DXGI output, pixel format and SDL HDR properties for the window. */
void WinLogDisplayDiagnostics(SDL_Window* window);

#endif // WINDOWCOMPOSITORWIN_H
