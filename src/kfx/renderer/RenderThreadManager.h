#ifndef RENDERER_RENDERTHREADMANAGER_H
#define RENDERER_RENDERTHREADMANAGER_H

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

// Owns the render-thread lifecycle: spawn, per-frame signal/wait, stop.
// Ported unwired (P5.3) -- no backend calls Start() yet; RendererOpenGL
// stays synchronous until P5.6 wires this in for real.
//
// Per-frame protocol from the game thread:
//   WaitForCompletion();  // block until the previous frame's work_fn finished
//   ... snapshot command buffers ...
//   Signal();             // wake the render thread for this frame

extern thread_local bool g_on_render_thread;

class RenderThreadManager
{
public:
    using Fn = std::function<void()>;

    RenderThreadManager() = default;
    ~RenderThreadManager();

    RenderThreadManager(const RenderThreadManager&)            = delete;
    RenderThreadManager& operator=(const RenderThreadManager&) = delete;

    /** Spawn the render thread and block until init_fn() completes. No-op if
     *  already active. */
    void Start(Fn init_fn, Fn work_fn, Fn cleanup_fn);

    /** Block until the render thread finishes work_fn(). First call returns
     *  immediately (nothing to wait for yet). */
    void WaitForCompletion();

    /** Wake the render thread to run work_fn() for this frame. */
    void Signal();

    /** Signal quit, join, and reset so a future Start() works again. */
    void Stop();

    bool IsActive() const { return m_active; }

private:
    void ThreadProc(Fn init_fn, Fn work_fn, Fn cleanup_fn);

    std::thread             m_thread;
    std::mutex              m_mutex;
    std::condition_variable m_cv;
    bool m_work_ready  = false;
    bool m_work_done   = true;
    bool m_quit        = false;
    bool m_initialized = false;
    bool m_active      = false;
};

#endif // RENDERER_RENDERTHREADMANAGER_H
