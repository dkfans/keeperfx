#ifndef IGLHDRPOLICY_H
#define IGLHDRPOLICY_H

struct SDL_Window;

/** Per-host policy for keeping a fullscreen GL swapchain on the desktop
 *  compositor's HDR path. Windows/DXGI implements it; every other host
 *  no-ops and lets the native compositor decide. */
class IGLHdrPolicy {
public:
    virtual ~IGLHdrPolicy() = default;
    virtual void OnContextReady(SDL_Window* window, bool desktop_fullscreen) = 0;
    virtual void OnPresent() = 0;
    virtual void Shutdown() = 0;
};

#endif // IGLHDRPOLICY_H
