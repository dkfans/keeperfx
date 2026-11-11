#ifndef RENDERER_RENDERERTHREAD_H
#define RENDERER_RENDERERTHREAD_H

#include <thread>

// Debug-mode thread-ownership assertions. ASSERT_GAME_THREAD()/
// ASSERT_RENDER_THREAD() are no-ops in Release.

void RendererThread_RegisterGameThread();
void RendererThread_RegisterRenderThread();
bool RendererThread_IsGameThread();
bool RendererThread_IsRenderThread();

#if DEBUG
#  include <cassert>
#  define ASSERT_GAME_THREAD()   do { assert(RendererThread_IsGameThread());   } while (0)
#  define ASSERT_RENDER_THREAD() do { assert(RendererThread_IsRenderThread()); } while (0)
#else
#  define ASSERT_GAME_THREAD()   do {} while (0)
#  define ASSERT_RENDER_THREAD() do {} while (0)
#endif

#endif // RENDERER_RENDERERTHREAD_H
