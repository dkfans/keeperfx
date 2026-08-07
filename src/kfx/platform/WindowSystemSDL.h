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
    void SetCursorVisible(bool visible) override;
    void WarpCursor(int x, int y) override;

    bool HasWindow() const override;
    SDL_Window* GetSDLWindow() const;
    unsigned int GetWindowFlags() const override;
    void GetWindowSize(int* out_w, int* out_h) const override;
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
    bool RecreateForSoftwareRenderer() override;
    bool RecreateForVulkanRenderer() override;

    // ----- Display info -----
    int GetDisplayRefreshRate() const override;

    // PollInput is a no-op: SDL delivers mouse input via events.

private:
    bool m_appActive = true;
};

/** Shared singleton desktop window system. */
WindowSystemSDL* GetSDLWindowSystem();

#endif // WINDOW_SYSTEM_SDL_H
