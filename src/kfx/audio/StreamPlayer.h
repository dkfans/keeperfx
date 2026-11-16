#pragma once
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>

namespace kfx::audio {

enum class PlayerChannel : uint8_t { Music, Speech }; // one job each

struct StreamJob;

// Decodes music and speech files into stream slots on a worker thread. Call every method from the game thread.
class StreamPlayer {
public:
    StreamPlayer();
    ~StreamPlayer();
    StreamPlayer(const StreamPlayer &) = delete;
    StreamPlayer & operator=(const StreamPlayer &) = delete;

    // Opens the file; false (and the current job keeps playing) on failure.
    bool play(PlayerChannel channel, const char * path, float volume, bool loop);
    void stop(PlayerChannel channel);                       // immediate
    void fade_out(PlayerChannel channel, uint32_t milliseconds);
    void set_paused(PlayerChannel channel, bool paused);
    void set_volume(PlayerChannel channel, float volume);   // 0..1
    bool is_playing(PlayerChannel channel);                 // active and not finished
    void update();                                          // every tick: frees finished and faded-out jobs
    void shutdown();                                        // stops both jobs and joins the worker

private:
    static constexpr size_t kJobCount = 2;

    StreamJob & job_for(PlayerChannel channel);
    void wake_worker();
    void worker_main();

    std::unique_ptr<StreamJob> jobs_[kJobCount];
    std::thread worker_;
    std::mutex wake_mutex_;
    std::condition_variable wake_;
    bool quit_ = false;  // under wake_mutex_
    bool woken_ = false; // under wake_mutex_
};

StreamPlayer & stream_player();

} // namespace kfx::audio
