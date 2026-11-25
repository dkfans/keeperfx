/** @file TileSetLoader.cpp
 *     Background loading of tile texture sets. The worker reads files only;
 *     it never logs and never touches game state. */
#include "pre_inc.h"
#include "kfx/tilesets/TileSetLoader.h"
#include "bflib_dernc.h"

#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <mutex>
#include <thread>
#include "post_inc.h"

namespace {

class TileSetLoader
{
public:
    ~TileSetLoader() { Shutdown(); }

    void Queue(const TileSetJob& job)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_jobs.push_back(job);
            if (!m_worker.joinable())
            {
                m_quit = false;
                m_worker = std::thread(&TileSetLoader::WorkerMain, this);
            }
        }
        m_wake.notify_one();
    }

    bool TakeDone(TileSetResult* out)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_done.empty())
            return false;
        *out = m_done.front();
        m_done.pop_front();
        return true;
    }

    void WaitIdle()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_idle.wait(lock, [this] { return m_jobs.empty() && !m_busy; });
    }

    void Shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_quit = true;
        }
        m_wake.notify_one();
        if (m_worker.joinable())
            m_worker.join();
        std::lock_guard<std::mutex> lock(m_mutex);
        m_jobs.clear();
        for (TileSetResult& res : m_done)
            free(res.buffer);
        m_done.clear();
    }

private:
    void WorkerMain()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        for (;;)
        {
            m_wake.wait(lock, [this] { return m_quit || !m_jobs.empty(); });
            if (m_quit)
                return;
            TileSetJob job = m_jobs.front();
            m_jobs.pop_front();
            m_busy = true;
            lock.unlock();

            TileSetResult res = Load(job);

            lock.lock();
            m_done.push_back(res);
            m_busy = false;
            if (m_jobs.empty())
                m_idle.notify_all();
        }
    }

    static TileSetResult Load(const TileSetJob& job)
    {
        TileSetResult res{};
        res.set = job.set;
        res.level_serial = job.level_serial;
        res.result_a = LbLoadAtMax_ReadError;
        res.result_b = LbLoadAtMax_ReadError;
        res.buffer = static_cast<unsigned char*>(malloc(TILE_SET_BYTES));
        if (res.buffer == nullptr)
            return res;
        memset(res.buffer, TILE_SET_FILL_COLOUR, TILE_SET_BYTES);
        res.result_a = (int32_t)LbFileLoadAtMax(job.path_a, res.buffer, TILE_SET_LETTER_BYTES);
        res.result_b = (int32_t)LbFileLoadAtMax(job.path_b, res.buffer + TILE_SET_LETTER_BYTES, TILE_SET_LETTER_BYTES);
        return res;
    }

    std::mutex m_mutex;
    std::condition_variable m_wake;
    std::condition_variable m_idle;
    std::deque<TileSetJob> m_jobs;
    std::deque<TileSetResult> m_done;
    std::thread m_worker;
    bool m_busy = false;
    bool m_quit = false;
};

TileSetLoader g_loader;

} // namespace

extern "C" void TileSetLoader_Queue(const TileSetJob* job) { g_loader.Queue(*job); }
extern "C" int  TileSetLoader_TakeDone(TileSetResult* out) { return g_loader.TakeDone(out) ? 1 : 0; }
extern "C" void TileSetLoader_WaitIdle(void) { g_loader.WaitIdle(); }
extern "C" void TileSetLoader_Shutdown(void) { g_loader.Shutdown(); }
