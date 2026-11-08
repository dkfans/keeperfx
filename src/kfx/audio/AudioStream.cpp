#include "AudioStream.h"
#include "AudioCallback.h"
#include "AudioLog.h"
#include "StreamPool.h"

namespace kfx::audio {

namespace {
StreamPool g_pool;
AudioCallback * g_callback = nullptr;

StreamFormat to_pool_format(const AudioStreamFormat & format) {
    return StreamFormat{format.sample_rate, format.channels,
        format.type == AudioSampleType::Int16 ? StreamSampleType::Int16 : StreamSampleType::Float32};
}
}

StreamPool & stream_pool() {
    return g_pool;
}

uint32_t audio_stream_output_rate() {
    return kCoreSampleRate;
}

AudioStreamHandle audio_stream_open(const AudioStreamFormat & format) {
    if (!g_callback) {
        return 0;
    }
    const AudioStreamHandle handle = g_pool.open(to_pool_format(format));
    if (handle == 0) {
        AUDIOWARN("Cannot open audio stream (rate %u, %u channels): no free slot or unsupported format",
                  format.sample_rate, unsigned(format.channels));
    }
    return handle;
}

uint32_t audio_stream_write(AudioStreamHandle handle, const void * frames, uint32_t frame_count) {
    return g_pool.write(handle, frames, frame_count);
}

void audio_stream_start(AudioStreamHandle handle) {
    g_pool.start(handle);
}

void audio_stream_finish(AudioStreamHandle handle) {
    g_pool.finish(handle);
}

void audio_stream_set_volume(AudioStreamHandle handle, float volume) {
    g_pool.set_volume(handle, volume);
}

uint64_t audio_stream_frames_consumed(AudioStreamHandle handle) {
    return g_pool.frames_consumed(handle);
}

uint32_t audio_stream_underruns(AudioStreamHandle handle) {
    return g_pool.underruns(handle);
}

bool audio_stream_is_finished(AudioStreamHandle handle) {
    return g_pool.is_finished(handle);
}

void audio_stream_close(AudioStreamHandle handle) {
    g_pool.request_close(handle);
    if (g_callback) {
        g_callback->reclaim_streams();
    } else {
        g_pool.reclaim();
    }
}

void audio_stream_bind(AudioCallback * callback) {
    g_callback = callback;
}

void audio_stream_unbind() {
    g_callback = nullptr;
    g_pool.reset_all();
}

} // namespace kfx::audio
