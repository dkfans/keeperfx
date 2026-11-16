#include "StreamPlayer.h"
#include "AudioDecoder.h"
#include "AudioLog.h"
#include "AudioStream.h"
#include "StreamConfig.h"
#include "StreamConverter.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <string>
#include <vector>

namespace kfx::audio {

namespace {
using Clock = std::chrono::steady_clock;

constexpr uint32_t kPassFrames = 4096;    // stereo frames written per job per pass
constexpr uint32_t kLowWaterFrames = 1024; // converted frames kept ready
constexpr uint32_t kReadFrames = 2048;     // source frames per decoder read
constexpr auto kWorkerPeriod = std::chrono::milliseconds(10);
}

// Game thread and worker share a job only under its mutex.
struct StreamJob {
    std::mutex mutex;
    std::unique_ptr<AudioDecoder> decoder;
    std::unique_ptr<StreamConverter> converter;
    AudioStreamHandle handle = 0;
    bool active = false;
    bool loop = false;
    bool paused = false;
    bool decoder_done = false;
    bool finish_sent = false;
    bool decoded_any = false;  // since open
    bool decoded_lap = false;  // since open or the last rewind
    float volume = 1.0f;
    bool fading = false;
    Clock::time_point fade_start;
    uint32_t fade_ms = 0;
    float fade_from = 1.0f;
    std::atomic<bool> done{false};
    std::string path;
    std::vector<float> source;  // kReadFrames source frames
    std::vector<float> output;  // kPassFrames stereo frames
    std::vector<float> pending; // converted frames the slot had no room for
};

namespace {

void release(StreamJob & job) {
    if (job.handle != 0) {
        audio_stream_close(job.handle);
    }
    job.handle = 0;
    job.decoder.reset();
    job.converter.reset();
    job.active = false;
    job.fading = false;
    job.done.store(false, std::memory_order_relaxed);
}

bool write_pending(StreamJob & job) {
    if (job.pending.empty()) {
        return true;
    }
    const uint32_t written = audio_stream_write(job.handle, job.pending.data(), uint32_t(job.pending.size() / 2));
    job.pending.erase(job.pending.begin(), job.pending.begin() + ptrdiff_t(written) * 2);
    return job.pending.empty();
}

void fill_converter(StreamJob & job) {
    while (!job.decoder_done && job.converter->available() < kLowWaterFrames) {
        const uint32_t got = job.decoder->read(job.source.data(), kReadFrames);
        if (got > 0) {
            job.converter->push(job.source.data(), got);
            job.decoded_any = true;
            job.decoded_lap = true;
            continue;
        }
        if (job.loop && job.decoded_lap && job.decoder->rewind()) {
            job.decoded_lap = false;
            continue;
        }
        if (!job.decoded_any) {
            AUDIOWARN("Cannot play \"%s\": %s", job.path.c_str(), "no audio could be decoded");
        }
        job.decoder_done = true;
        job.converter->flush();
    }
}

void pump(StreamJob & job) {
    if (!write_pending(job)) {
        return;
    }
    uint32_t budget = kPassFrames;
    while (budget > 0) {
        fill_converter(job);
        const uint32_t pulled = job.converter->pull(job.output.data(), budget);
        if (pulled == 0) {
            break;
        }
        budget -= pulled;
        const uint32_t written = audio_stream_write(job.handle, job.output.data(), pulled);
        if (written < pulled) {
            job.pending.assign(job.output.begin() + ptrdiff_t(written) * 2, job.output.begin() + ptrdiff_t(pulled) * 2);
            return;
        }
    }
    if (job.decoder_done && job.converter->available() == 0) {
        audio_stream_finish(job.handle);
        audio_stream_start(job.handle);
        job.finish_sent = true;
    }
}

void service(StreamJob & job) {
    std::lock_guard<std::mutex> lock(job.mutex);
    if (!job.active || job.done.load(std::memory_order_relaxed)) {
        return;
    }
    if (job.fading) {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - job.fade_start).count();
        if (elapsed >= int64_t(job.fade_ms)) {
            audio_stream_set_volume(job.handle, 0.0f);
            job.done.store(true, std::memory_order_relaxed);
            return;
        }
        audio_stream_set_volume(job.handle, job.fade_from * (1.0f - float(elapsed) / float(job.fade_ms)));
    }
    if (job.paused) {
        return;
    }
    if (job.finish_sent) {
        if (audio_stream_is_finished(job.handle)) {
            job.done.store(true, std::memory_order_relaxed);
        }
        return;
    }
    pump(job);
}

} // namespace

StreamPlayer::StreamPlayer() {
    for (auto & job : jobs_) {
        job = std::make_unique<StreamJob>();
    }
}

StreamPlayer::~StreamPlayer() {
    shutdown();
}

StreamJob & StreamPlayer::job_for(PlayerChannel channel) {
    return *jobs_[channel == PlayerChannel::Music ? 0 : 1];
}

bool StreamPlayer::play(PlayerChannel channel, const char * path, float volume, bool loop) {
    const char * name = path ? path : "";
    std::string error = "no file given";
    std::unique_ptr<AudioDecoder> decoder = path ? open_audio_file(path, error) : nullptr;
    std::unique_ptr<StreamConverter> converter;
    AudioStreamHandle handle = 0;
    if (decoder) {
        converter = std::make_unique<StreamConverter>(decoder->sample_rate(), decoder->channels());
        if (!converter->valid()) {
            error = "cannot convert the audio";
        } else {
            handle = audio_stream_open(AudioStreamFormat{kCoreSampleRate, 2, AudioSampleType::Float32});
            if (handle == 0) {
                error = "no free audio stream";
            }
        }
    }
    if (handle == 0) {
        AUDIOWARN("Cannot play \"%s\": %s", name, error.c_str());
        return false;
    }
    StreamJob & job = job_for(channel);
    {
        std::lock_guard<std::mutex> lock(job.mutex);
        release(job);
        job.source.resize(size_t(kReadFrames) * decoder->channels());
        job.output.resize(size_t(kPassFrames) * 2);
        job.pending.clear();
        job.pending.reserve(size_t(kPassFrames) * 2);
        job.decoder = std::move(decoder);
        job.converter = std::move(converter);
        job.handle = handle;
        job.loop = loop;
        job.paused = false;
        job.decoder_done = false;
        job.finish_sent = false;
        job.decoded_any = false;
        job.decoded_lap = false;
        job.volume = std::clamp(volume, 0.0f, 1.0f);
        job.path = name;
        job.active = true;
        audio_stream_set_volume(handle, job.volume);
    }
    if (!worker_.joinable()) {
        worker_ = std::thread(&StreamPlayer::worker_main, this);
    }
    wake_worker();
    return true;
}

void StreamPlayer::stop(PlayerChannel channel) {
    StreamJob & job = job_for(channel);
    std::lock_guard<std::mutex> lock(job.mutex);
    release(job);
}

void StreamPlayer::fade_out(PlayerChannel channel, uint32_t milliseconds) {
    StreamJob & job = job_for(channel);
    std::lock_guard<std::mutex> lock(job.mutex);
    if (!job.active || job.fading || job.done.load(std::memory_order_relaxed)) {
        return;
    }
    if (milliseconds == 0) {
        audio_stream_set_volume(job.handle, 0.0f);
        job.done.store(true, std::memory_order_relaxed);
        return;
    }
    job.fading = true;
    job.fade_start = Clock::now();
    job.fade_ms = milliseconds;
    job.fade_from = job.volume;
}

void StreamPlayer::set_paused(PlayerChannel channel, bool paused) {
    StreamJob & job = job_for(channel);
    std::lock_guard<std::mutex> lock(job.mutex);
    job.paused = paused;
    if (job.active) {
        audio_stream_set_paused(job.handle, paused);
    }
}

void StreamPlayer::set_volume(PlayerChannel channel, float volume) {
    StreamJob & job = job_for(channel);
    std::lock_guard<std::mutex> lock(job.mutex);
    job.volume = std::clamp(volume, 0.0f, 1.0f);
    if (job.active && !job.fading) {
        audio_stream_set_volume(job.handle, job.volume);
    }
}

bool StreamPlayer::is_playing(PlayerChannel channel) {
    StreamJob & job = job_for(channel);
    std::lock_guard<std::mutex> lock(job.mutex);
    return job.active && !job.done.load(std::memory_order_relaxed);
}

void StreamPlayer::update() {
    for (auto & job : jobs_) {
        std::lock_guard<std::mutex> lock(job->mutex);
        if (job->active && job->done.load(std::memory_order_relaxed)) {
            release(*job);
        }
    }
}

void StreamPlayer::shutdown() {
    stop(PlayerChannel::Music);
    stop(PlayerChannel::Speech);
    if (!worker_.joinable()) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(wake_mutex_);
        quit_ = true;
    }
    wake_.notify_one();
    worker_.join();
    std::lock_guard<std::mutex> lock(wake_mutex_);
    quit_ = false;
    woken_ = false;
}

void StreamPlayer::wake_worker() {
    {
        std::lock_guard<std::mutex> lock(wake_mutex_);
        woken_ = true;
    }
    wake_.notify_one();
}

void StreamPlayer::worker_main() {
    for (;;) {
        {
            std::unique_lock<std::mutex> lock(wake_mutex_);
            wake_.wait_for(lock, kWorkerPeriod, [this] { return quit_ || woken_; });
            if (quit_) {
                return;
            }
            woken_ = false;
        }
        for (auto & job : jobs_) {
            service(*job);
        }
    }
}

StreamPlayer & stream_player() {
    static StreamPlayer player;
    return player;
}

} // namespace kfx::audio
