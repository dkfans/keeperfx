#ifndef WINDOW_SYSTEM_SDL_H
#define WINDOW_SYSTEM_SDL_H

#include "kfx/platform/IWindowSystem.h"

struct SDL_Window;  // forward declaration; full type in WindowSystemSDL.cpp

/** SDL3 desktop window-system implementation.
 *
 */
class WindowSystemSDL : public IWindowSystem {
public:
    bool IsAppActive() const override;
    void OnFocusGained() override;
    void OnFocusLost() override;

    bool HasOSCursor() const override { return true; }
    void SetCursorGrab(bool grab) override;
    void SetUseRelativeMouse(bool relative) override;
    void SetCursorVisible(bool visible) override;
    void WarpCursor(int x, int y) override;
    void RecenterCursor() override;
    bool GetCursorPosition(int* out_x, int* out_y) const override;
    void GetCursorScale(float* out_sx, float* out_sy) const override;
    bool IsCursorInWindow() const override;

    bool HasWindow() const override;
    SDL_Window* GetSDLWindow() const;
    unsigned int GetWindowFlags() const override;
    void GetWindowSize(int* out_w, int* out_h) const override;
    void GetDrawableSize(int* out_w, int* out_h) const override;
    void SetGameSurfaceSize(int w, int h) override;
    void GetPresentRect(int* out_x, int* out_y, int* out_w, int* out_h) const override;
    int GetWindowDisplayIndex() const override;
    int GetNumVideoDisplays() const override;
    int GetDesktopDisplayMode(int display, int* out_w, int* out_h) const override;
    int GetDisplayBounds(int display, int* out_x, int* out_y, int* out_w, int* out_h) const override;
    int GetClosestDisplayMode(int display, int desired_w, int desired_h, int* out_w, int* out_h) const override;
    int SetWindowDisplayMode(int w, int h) override;
    void SetWindowSize(int w, int h) override;
    int SetWindowFullscreen(unsigned int flags) override;
    void SetWindowBordered(int bordered) override;
    void SetWindowPosition(int x, int y) override;
    bool CreateWindow(const char* title, int x, int y, int w, int h, unsigned int flags) override;

    bool SetWindowTitle(const char* /*title*/) override;
    void ShowWindow() override;
    std::unique_ptr<IGLContext> CreateGLContext() override;

    // ----- Display info -----
    int GetDisplayRefreshRate() const override;

    // PollInput is a no-op: SDL delivers mouse input via events.

    /** Called for every SDL window event. */
    void HandleWindowEvent(const union SDL_Event* ev);

private:
    void ApplyOsCursorPolicy();
    void KeepFullscreenComposited();
    void GetVisibleArea(int* out_x, int* out_y, int* out_w, int* out_h) const;
    void GetPixelsPerWindowUnit(float* out_sx, float* out_sy) const;

    SDL_Window* m_window = nullptr;
    bool m_appActive = true;
    bool m_useRelativeMouse = true;
    bool m_keepComposited = false;
    bool m_desktopFullscreenOnly = false;
    int m_gameW = 0;
    int m_gameH = 0;
};

/** Shared singleton desktop window system. */
WindowSystemSDL* GetSDLWindowSystem();

#endif // WINDOW_SYSTEM_SDL_H
