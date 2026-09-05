#ifndef PLATFORM_GL_HDR_WIN_H
#define PLATFORM_GL_HDR_WIN_H

struct SDL_Window;

#ifdef _WIN32
// Windows/DXGI HDR-compositor bookkeeping for the OpenGL backend.
//
// DXGI can engage "Independent Flip" (direct scanout) for a GL swapchain
// that covers the full primary display. Independent Flip bypasses the DWM
// HDR compositor, silently switching an HDR-enabled monitor to SDR at the
// first buffer swap -- which reads to a player as "the colours are wrong",
// with no error or log line to explain why. This ties a small always-on
// compositor anchor + colour-space watcher (ported from develop's
// platform_gl_sdl3.cpp) into this branch's own window/renderer split.
//
// Thread contract: OnContextReady()/Shutdown() must run on the thread that
// owns the SDL window (Win32 HWNDs are thread-affine -- DestroyWindow fails
// cross-thread with ERROR_ACCESS_DENIED if called from elsewhere). This
// codebase creates/destroys the GL context on the game thread, so call both
// from there. Tick() may run on any thread once OnContextReady() has
// returned -- DXGI factory calls are free-threaded -- and this codebase
// calls it from the GL render thread, next to SDL_GL_SwapWindow, matching
// develop's own platform_swap_gl_buffers().
void PlatformGLHdrWin_OnContextReady(SDL_Window* window, bool desktop_fullscreen);
void PlatformGLHdrWin_Tick();
void PlatformGLHdrWin_Shutdown();
#endif // _WIN32

#endif // PLATFORM_GL_HDR_WIN_H
