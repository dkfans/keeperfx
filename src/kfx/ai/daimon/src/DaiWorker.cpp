#include "pre_inc.h"
#include "daimon/DaiWorker.h"

#include <algorithm>
#include <chrono>
#include <exception>
#include <random>

#include "post_inc.h"

int64_t DaiWorkers::CountFor(int64_t configured, int64_t reserved, int64_t keepers)
{
    int64_t n = configured;
    if (configured < 0) n = (int64_t)std::thread::hardware_concurrency() - reserved;
    return std::clamp<int64_t>(n, 0, std::max<int64_t>(keepers, 0));
}

void DaiWorkers::Start(int64_t n, int64_t jitter_us)
{
    Stop();
    std::lock_guard<std::mutex> lock(m_mutex);
    m_stopping = false;
    m_jitter_us = jitter_us;
    m_queues.assign((size_t)std::max<int64_t>(n, 0), Queue());
    for (size_t i = 0; i < m_queues.size(); i++) {
        try {
            m_threads.emplace_back([this, i] { run(i); });
        } catch (const std::exception &) {
            break;                          // the system refused a thread: the ones started share the keys
        }
    }
    m_queues.resize(m_threads.size());
}

void DaiWorkers::Stop()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stopping = true;
    }
    m_work_cv.notify_all();
    for (std::thread &t : m_threads) t.join();
    std::lock_guard<std::mutex> lock(m_mutex);
    m_threads.clear();
    m_queues.clear();
    m_stopping = false;
}

int64_t DaiWorkers::Post(int64_t key, std::function<void()> work)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_threads.empty()) {
            const int64_t seq = ++m_posted[key];
            const int64_t n = (int64_t)m_queues.size();
            m_queues[(size_t)(((key % n) + n) % n)].items.push_back(Item{ key, std::move(work) });
            m_work_cv.notify_all();
            return seq;
        }
    }
    // The inline path: now, on this thread.
    std::string error;
    try {
        work();
    } catch (const std::exception &e) {
        error = e.what();
    } catch (...) {
        error = "unknown";
    }
    if (!error.empty()) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_errors[key] = error;
    }
    return 0;
}

std::string DaiWorkers::Wait(int64_t key)
{
    int64_t seq = 0;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        seq = m_posted[key];
    }
    return WaitFor(key, seq);
}

std::string DaiWorkers::WaitFor(int64_t key, int64_t seq)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_done_cv.wait(lock, [&] { return m_done[key] >= seq; });
    std::string error;
    const auto it = m_errors.find(key);
    if (it != m_errors.end()) {
        error = it->second;
        m_errors.erase(it);
    }
    return error;
}

void DaiWorkers::done(int64_t key, const std::string &error)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_done[key]++;
        if (!error.empty()) m_errors[key] = error;
    }
    m_done_cv.notify_all();
}

void DaiWorkers::run(size_t worker)
{
    std::mt19937_64 jitter_rng((uint64_t)std::chrono::steady_clock::now().time_since_epoch().count() + worker);
    for (;;) {
        Item item;
        int64_t jitter = 0;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_work_cv.wait(lock, [&] { return m_stopping || !m_queues[worker].items.empty(); });
            if (m_queues[worker].items.empty()) return;     // stopping, and nothing left
            item = std::move(m_queues[worker].items.front());
            m_queues[worker].items.pop_front();
            jitter = m_jitter_us;
        }
        if (jitter > 0) std::this_thread::sleep_for(std::chrono::microseconds((int64_t)(jitter_rng() % (uint64_t)(jitter + 1))));
        std::string error;
        try {
            item.work();
        } catch (const std::exception &e) {
            error = e.what();
        } catch (...) {
            error = "unknown";
        }
        done(item.key, error);
    }
}
