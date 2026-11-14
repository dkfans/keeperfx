/** @file DaiWorker.h
 *  The threads Daimon's planning turns run on (12 section 5): a fixed set of workers, each running the work posted for
 *  its keys in posting order; a key always goes to the same worker (key mod the count), so keepers past the count share
 *  workers round-robin and only the timing changes. With no worker started, Post runs the work at once on the caller's
 *  thread (the inline path). WaitFor returns once a key's work up to a posting has run, Wait once all of it has, with
 *  what an exception said if one ended any. A test mode delays each piece of work a random time first: results must not
 *  change. */
#ifndef DAIMON_DAIWORKER_H
#define DAIMON_DAIWORKER_H

#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class DaiWorkers {
public:
    DaiWorkers() = default;
    ~DaiWorkers() { Stop(); }
    DaiWorkers(const DaiWorkers &) = delete;
    DaiWorkers &operator=(const DaiWorkers &) = delete;

    /** How many workers a setting asks for: `configured` > 0 is used as given, 0 none (inline), -1 the logical cores
     *  less `reserved` (main thread and renderer), at most `keepers` either way. */
    static int64_t CountFor(int64_t configured, int64_t reserved, int64_t keepers);
    /** Starts `n` workers (after Stop); 0 leaves the inline path. Fewer may start (a thread the system refuses): the
     *  number that did is Count(). */
    void Start(int64_t n, int64_t jitter_us = 0);
    /** Runs everything posted, then ends the workers. */
    void Stop();
    int64_t Count() const { return (int64_t)m_threads.size(); }

    /** The posting's number for its key (1, 2, ...; 0: run already, inline). */
    int64_t Post(int64_t key, std::function<void()> work);
    /** Waits until the key's postings up to `seq` have run; "" or what an exception in the key's work said (cleared). */
    std::string WaitFor(int64_t key, int64_t seq);
    /** Waits for all the key's work. */
    std::string Wait(int64_t key);
private:
    struct Item {
        int64_t key = 0;
        std::function<void()> work;
    };
    struct Queue {
        std::deque<Item> items;
    };
    void run(size_t worker);
    void done(int64_t key, const std::string &error);

    std::mutex m_mutex;
    std::condition_variable m_work_cv, m_done_cv;
    std::vector<Queue> m_queues;
    std::vector<std::thread> m_threads;
    std::map<int64_t, int64_t> m_posted, m_done;    /**< by key: postings made, postings run */
    std::map<int64_t, std::string> m_errors;
    bool m_stopping = false;
    int64_t m_jitter_us = 0;
};

#endif
