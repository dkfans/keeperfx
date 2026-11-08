#pragma once
#include <cstddef>
#include <cstdint>

#ifndef KFX_AUDIO_CORE_RATE
#define KFX_AUDIO_CORE_RATE 48000
#endif
#ifndef KFX_AUDIO_STREAM_SLOTS
#define KFX_AUDIO_STREAM_SLOTS 4
#endif
#ifndef KFX_AUDIO_STREAM_RING_MS
#define KFX_AUDIO_STREAM_RING_MS 500
#endif
#ifndef KFX_AUDIO_STREAM_MIN_FILL_MS
#define KFX_AUDIO_STREAM_MIN_FILL_MS 100
#endif
#ifndef KFX_AUDIO_STREAM_FLOAT
#define KFX_AUDIO_STREAM_FLOAT 1
#endif

namespace kfx::audio {

// Sample rate of the shared mix; streams must be written at exactly this rate.
constexpr uint32_t kCoreSampleRate = KFX_AUDIO_CORE_RATE;
// Number of simultaneously open stream slots.
constexpr size_t kStreamSlots = KFX_AUDIO_STREAM_SLOTS;
// Ring length per slot, in milliseconds of audio.
constexpr uint32_t kStreamRingMs = KFX_AUDIO_STREAM_RING_MS;
// Buffered audio a slot needs before it starts playing, in milliseconds.
constexpr uint32_t kStreamMinFillMs = KFX_AUDIO_STREAM_MIN_FILL_MS;
// Whether 32-bit float streams are accepted.
constexpr bool kStreamFloatEnabled = KFX_AUDIO_STREAM_FLOAT != 0;

} // namespace kfx::audio
