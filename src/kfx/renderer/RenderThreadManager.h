#ifndef RENDERER_RENDERTHREADMANAGER_H
#define RENDERER_RENDERTHREADMANAGER_H

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

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

    void WaitForCompletion();
    bool IsWorkDone() const;

    /** Wake the render thread to run work_fn() for this frame. */
    void Signal();

    /** Signal quit, join, and reset so a future Start() works again. */
    void Stop();

    bool IsActive() const { return m_active; }

private:
    void ThreadProc(Fn init_fn, Fn work_fn, Fn cleanup_fn);

    std::thread             m_thread;
    mutable std::mutex      m_mutex;
    std::condition_variable m_cv;
    bool m_work_ready  = false;
    bool m_work_done   = true;
    bool m_quit        = false;
    bool m_initialized = false;
    bool m_active      = false;
};

#endif // RENDERER_RENDERTHREADMANAGER_H
