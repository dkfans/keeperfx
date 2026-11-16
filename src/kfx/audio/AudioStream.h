#pragma once
#include <cstdint>

namespace kfx::audio {

class AudioCallback;
class StreamPool;

enum class AudioSampleType : uint8_t { Int16, Float32 };

struct AudioStreamFormat {
    uint32_t sample_rate;
    uint8_t channels; // 1 or 2
    AudioSampleType type;
};

using AudioStreamHandle = uint32_t; // 0 = invalid

StreamPool & stream_pool();

// Rate every stream must be written at.
uint32_t audio_stream_output_rate();
// 0 if no slot is free, the format is unsupported, or audio is not running.
AudioStreamHandle audio_stream_open(const AudioStreamFormat & format);
// Whole frames; returns frames accepted (0 when the ring is full). Never blocks.
uint32_t audio_stream_write(AudioStreamHandle handle, const void * frames, uint32_t frame_count);
// Play now even if below the minimum fill.
void audio_stream_start(AudioStreamHandle handle);
// No more data will be written; the stream ends once it drains.
void audio_stream_finish(AudioStreamHandle handle);
void audio_stream_set_volume(AudioStreamHandle handle, float volume);
// A paused stream is silent and keeps its buffered data.
void audio_stream_set_paused(AudioStreamHandle handle, bool paused);
// Frames the audio callback has consumed; divide by the sample rate for the audio clock.
uint64_t audio_stream_frames_consumed(AudioStreamHandle handle);
uint32_t audio_stream_underruns(AudioStreamHandle handle);
bool audio_stream_is_finished(AudioStreamHandle handle);
// Stops the stream and frees its slot; stop the producer first, never call from the audio callback.
void audio_stream_close(AudioStreamHandle handle);

// Called by the audio system when the callback starts and stops.
void audio_stream_bind(AudioCallback * callback);
void audio_stream_unbind();

} // namespace kfx::audio
