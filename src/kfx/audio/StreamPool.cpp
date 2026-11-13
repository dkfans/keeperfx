#include "StreamPool.h"
#include <cstring>
#include <algorithm>

namespace kfx::audio {

namespace {
constexpr size_t kMixChunkFrames = 256;
constexpr uint32_t kIndexBits = 8;
constexpr uint32_t kIndexMask = (1u << kIndexBits) - 1;
static_assert(kStreamSlots <= (1u << kIndexBits), "stream handles use 8 index bits");

size_t sample_bytes(StreamSampleType type) {
    return type == StreamSampleType::Int16 ? sizeof(int16_t) : sizeof(float);
}
}

StreamHandle StreamPool::open(const StreamFormat & format) {
    if (format.sample_rate != kCoreSampleRate || format.channels < 1 || format.channels > 2) {
        return 0;
    }
    if (format.type == StreamSampleType::Float32 && !kStreamFloatEnabled) {
        return 0;
    }
    for (size_t i = 0; i < kStreamSlots; ++i) {
        Slot & slot = slots_[i];
        uint32_t expected = Free;
        if (!slot.state.compare_exchange_strong(expected, Claimed, std::memory_order_acquire)) {
            continue;
        }
        slot.format = format;
        slot.frame_bytes = uint32_t(format.channels * sample_bytes(format.type));
        slot.min_fill_bytes = uint32_t(uint64_t(kCoreSampleRate) * kStreamMinFillMs / 1000) * slot.frame_bytes;
        slot.ring.init(size_t(uint64_t(kCoreSampleRate) * kStreamRingMs / 1000) * slot.frame_bytes);
        slot.start_requested.store(false, std::memory_order_relaxed);
        slot.finish_requested.store(false, std::memory_order_relaxed);
        slot.playing.store(false, std::memory_order_relaxed);
        slot.finished.store(false, std::memory_order_relaxed);
        slot.volume.store(1.0f, std::memory_order_relaxed);
        slot.consumed.store(0, std::memory_order_relaxed);
        slot.underrun_count.store(0, std::memory_order_relaxed);
        slot.generation = (slot.generation + 1) & 0xFFFFFF;
        if (slot.generation == 0) {
            slot.generation = 1;
        }
        slot.state.store(Active, std::memory_order_release);
        return (slot.generation << kIndexBits) | uint32_t(i);
    }
    return 0;
}

StreamPool::Slot * StreamPool::find(StreamHandle handle) {
    return const_cast<Slot *>(static_cast<const StreamPool *>(this)->find(handle));
}

const StreamPool::Slot * StreamPool::find(StreamHandle handle) const {
    const uint32_t index = handle & kIndexMask;
    if (handle == 0 || index >= kStreamSlots) {
        return nullptr;
    }
    const Slot & slot = slots_[index];
    if (slot.state.load(std::memory_order_acquire) != Active || (slot.generation << kIndexBits) != (handle & ~kIndexMask)) {
        return nullptr;
    }
    return &slot;
}

uint32_t StreamPool::write(StreamHandle handle, const void * frames, uint32_t frame_count) {
    Slot * slot = find(handle);
    if (!slot || frame_count == 0 || slot->finish_requested.load(std::memory_order_relaxed)) {
        return 0;
    }
    const size_t room = slot->ring.writable() / slot->frame_bytes;
    const size_t take = std::min<size_t>(room, frame_count);
    slot->ring.write(frames, take * slot->frame_bytes);
    return uint32_t(take);
}

void StreamPool::start(StreamHandle handle) {
    if (Slot * slot = find(handle)) {
        slot->start_requested.store(true, std::memory_order_release);
    }
}

void StreamPool::finish(StreamHandle handle) {
    if (Slot * slot = find(handle)) {
        slot->finish_requested.store(true, std::memory_order_release);
    }
}

void StreamPool::set_volume(StreamHandle handle, float volume) {
    if (Slot * slot = find(handle)) {
        slot->volume.store(std::max(volume, 0.0f), std::memory_order_relaxed);
    }
}

uint64_t StreamPool::frames_consumed(StreamHandle handle) const {
    const Slot * slot = find(handle);
    return slot ? slot->consumed.load(std::memory_order_relaxed) : 0;
}

uint32_t StreamPool::underruns(StreamHandle handle) const {
    const Slot * slot = find(handle);
    return slot ? slot->underrun_count.load(std::memory_order_relaxed) : 0;
}

bool StreamPool::is_finished(StreamHandle handle) const {
    const Slot * slot = find(handle);
    return slot ? slot->finished.load(std::memory_order_acquire) : true;
}

void StreamPool::request_close(StreamHandle handle) {
    if (Slot * slot = find(handle)) {
        slot->state.store(Closing, std::memory_order_release);
    }
}

void StreamPool::reclaim() {
    for (Slot & slot : slots_) {
        if (slot.state.load(std::memory_order_acquire) == Closing) {
            slot.ring.release();
            slot.state.store(Free, std::memory_order_release);
        }
    }
}

void StreamPool::reset_all() {
    for (Slot & slot : slots_) {
        slot.ring.release();
        slot.state.store(Free, std::memory_order_release);
    }
}

size_t StreamPool::mix_slot(Slot & slot, float * out, size_t frame_count) {
    // read finish first: the producer's last write precedes it
    const bool finishing = slot.finish_requested.load(std::memory_order_acquire);
    const float volume = slot.volume.load(std::memory_order_relaxed);
    uint8_t chunk[kMixChunkFrames * 2 * sizeof(float)];
    size_t done = 0;
    while (done < frame_count) {
        const size_t want = std::min(kMixChunkFrames, frame_count - done);
        const size_t got = slot.ring.read(chunk, want * slot.frame_bytes) / slot.frame_bytes;
        float * dst = out + done * 2;
        if (slot.format.type == StreamSampleType::Int16) {
            for (size_t i = 0; i < got; ++i) {
                int16_t l16, r16;
                std::memcpy(&l16, chunk + i * slot.frame_bytes, sizeof(int16_t));
                if (slot.format.channels == 2) {
                    std::memcpy(&r16, chunk + i * slot.frame_bytes + sizeof(int16_t), sizeof(int16_t));
                } else {
                    r16 = l16;
                }
                const float left = float(l16) * (1.0f / 32768.0f) * volume;
                const float right = float(r16) * (1.0f / 32768.0f) * volume;
                dst[i * 2] += left;
                dst[i * 2 + 1] += right;
            }
        } else {
            for (size_t i = 0; i < got; ++i) {
                float l32, r32;
                std::memcpy(&l32, chunk + i * slot.frame_bytes, sizeof(float));
                if (slot.format.channels == 2) {
                    std::memcpy(&r32, chunk + i * slot.frame_bytes + sizeof(float), sizeof(float));
                } else {
                    r32 = l32;
                }
                const float left = l32 * volume;
                const float right = r32 * volume;
                dst[i * 2] += left;
                dst[i * 2 + 1] += right;
            }
        }
        done += got;
        slot.consumed.fetch_add(got, std::memory_order_relaxed);
        if (got < want) {
            if (finishing && slot.ring.readable() == 0) {
                slot.finished.store(true, std::memory_order_release);
            } else {
                slot.underrun_count.fetch_add(1, std::memory_order_relaxed);
            }
            return done;
        }
    }
    if (finishing && slot.ring.readable() == 0) {
        slot.finished.store(true, std::memory_order_release);
    }
    return done;
}

bool StreamPool::mix_into(float * out, size_t frame_count) {
    bool contributed = false;
    for (Slot & slot : slots_) {
        if (slot.state.load(std::memory_order_acquire) != Active || slot.finished.load(std::memory_order_acquire)) {
            continue;
        }
        if (!slot.playing.load(std::memory_order_relaxed)) {
            const bool ready = slot.start_requested.load(std::memory_order_acquire)
                || slot.finish_requested.load(std::memory_order_acquire)
                || slot.ring.readable() >= slot.min_fill_bytes;
            if (!ready) {
                continue;
            }
            slot.playing.store(true, std::memory_order_relaxed);
        }
        if (mix_slot(slot, out, frame_count) > 0) {
            contributed = true;
        }
    }
    return contributed;
}

} // namespace kfx::audio
