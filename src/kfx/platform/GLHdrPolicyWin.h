#ifndef GLHDRPOLICYWIN_H
#define GLHDRPOLICYWIN_H

#include "kfx/platform/IGLHdrPolicy.h"

struct SDL_Window;

/** Windows/DXGI HDR-compositor bookkeeping for the GL backend. Stops DXGI
 *  engaging Independent Flip on a fullscreen GL swapchain, because that
 *  bypasses the DWM HDR compositor and drops an HDR monitor to SDR at the
 *  first buffer swap. Does it with a 1x1 anchor window plus
 *  DwmExtendFrameIntoClientArea, a watcher, and DXGI colour-space
 *  diagnostics. */
class GLHdrPolicyWin : public IGLHdrPolicy {
public:
    void OnContextReady(SDL_Window* window, bool desktop_fullscreen) override;
    void OnPresent() override;
    void Shutdown() override;
};

extern GLHdrPolicyWin g_hdr_policy_dxgi;

/** DXGI colour-space/pixel-format diagnostics only, no compositor
 *  mutations. Wine answers DXGI queries even though it has no DWM, so this
 *  is reused by the null policy to keep useful logging under Wine. */
void LogGLHdrDiagnostics(SDL_Window* window);

#endif // GLHDRPOLICYWIN_H
