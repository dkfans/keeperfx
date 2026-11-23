#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>
#include "StreamConfig.h"
#include "StreamRing.h"

namespace kfx::audio {

enum class StreamSampleType : uint8_t { Int16, Float32 };

struct StreamFormat {
    uint32_t sample_rate;
    uint8_t channels; // 1 or 2
    StreamSampleType type;
};

// Slot index in the low 8 bits, slot generation above; 0 is never valid.
using StreamHandle = uint32_t;

// Game thread: open/close. Producer thread: write/start/finish; stop it before request_close.
// Audio thread: mix_into only.
class StreamPool {
public:
    StreamHandle open(const StreamFormat & format);
    // Whole frames only; returns the number of frames accepted. Never blocks.
    uint32_t write(StreamHandle handle, const void * frames, uint32_t frame_count);
    // Play now even if below the minimum fill.
    void start(StreamHandle handle);
    // No more data will be written; the slot ends once its ring drains.
    void finish(StreamHandle handle);
    void set_volume(StreamHandle handle, float volume);
    // Silent and keeps its buffered data.
    void set_paused(StreamHandle handle, bool paused);
    uint64_t frames_consumed(StreamHandle handle) const;
    uint32_t underruns(StreamHandle handle) const;
    bool is_finished(StreamHandle handle) const;
    // Stops mixing the slot; reclaim() then frees it.
    void request_close(StreamHandle handle);
    // Frees closing slots. Call with the audio callback excluded (stream lock held) or stopped.
    void reclaim();
    // Adds every playing slot into interleaved stereo float output. Returns true if any slot contributed.
    bool mix_into(float * out, size_t frame_count);
    // For shutdown, with the audio callback stopped.
    void reset_all();

private:
    enum SlotState : uint32_t { Free = 0, Claimed, Active, Closing };

    struct Slot {
        static_assert(std::atomic<uint64_t>::is_always_lock_free, "stream slots need lock-free 64-bit atomics");
        std::atomic<uint32_t> state{Free};
        uint32_t generation = 0;           // game thread; bumped on every open
        StreamFormat format{};
        uint32_t frame_bytes = 0;
        uint32_t min_fill_bytes = 0;
        StreamRing ring;
        std::atomic<bool> start_requested{false};
        std::atomic<bool> finish_requested{false};
        std::atomic<bool> playing{false};  // audio thread
        std::atomic<bool> finished{false}; // audio thread
        std::atomic<float> volume{1.0f};
        std::atomic<bool> paused{false};
        std::atomic<uint64_t> consumed{0};
        std::atomic<uint32_t> underrun_count{0};
    };

    Slot * find(StreamHandle handle);
    const Slot * find(StreamHandle handle) const;
    size_t mix_slot(Slot & slot, float * out, size_t frame_count);

    Slot slots_[kStreamSlots];
};

} // namespace kfx::audio
