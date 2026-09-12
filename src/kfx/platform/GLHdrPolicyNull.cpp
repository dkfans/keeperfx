#include "pre_inc.h"
#include "kfx/platform/GLHdrPolicyNull.h"
#ifdef _WIN32
#include "kfx/platform/GLHdrPolicyWin.h" // LogGLHdrDiagnostics()
#endif
#include "post_inc.h"

void GLHdrPolicyNull::OnContextReady(SDL_Window* window, bool desktop_fullscreen)
{
#ifdef _WIN32
    // Reached only under Wine -- native Windows always gets the DXGI policy
    // (PlatformWindows::GetGLHdrPolicy()).
    (void)desktop_fullscreen;
    LogGLHdrDiagnostics(window);
#else
    (void)window;
    (void)desktop_fullscreen;
#endif
}

void GLHdrPolicyNull::OnPresent()
{
}

void GLHdrPolicyNull::Shutdown()
{
}

GLHdrPolicyNull g_hdr_policy_null;
