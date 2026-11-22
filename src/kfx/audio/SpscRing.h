#pragma once
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>

namespace kfx::audio {

// Fixed-capacity SPSC ring of POD values; push()/pop() never block or allocate.
// Capacity must be a power of two. Debug builds assert that calls on one side never overlap.
template <typename T, size_t Capacity>
class SpscRing {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of two");
public:
    bool push(const T & value) {
#ifndef NDEBUG
        const SideGuard guard(producer_busy_);
#endif
        const uint32_t head = head_.load(std::memory_order_relaxed);
        if (head - tail_.load(std::memory_order_acquire) >= Capacity) {
            return false; // full
        }
        slots_[head & kMask] = value;
        head_.store(head + 1, std::memory_order_release);
        return true;
    }

    bool pop(T & out) {
#ifndef NDEBUG
        const SideGuard guard(consumer_busy_);
#endif
        const uint32_t tail = tail_.load(std::memory_order_relaxed);
        if (tail == head_.load(std::memory_order_acquire)) {
            return false; // empty
        }
        out = slots_[tail & kMask];
        tail_.store(tail + 1, std::memory_order_release);
        return true;
    }

private:
    static constexpr uint32_t kMask = static_cast<uint32_t>(Capacity - 1);

    // Marks one side busy for the duration of a push()/pop() call.
    class SideGuard {
    public:
        explicit SideGuard(std::atomic<bool> & busy) : busy_(busy) {
            const bool was_busy = busy_.exchange(true, std::memory_order_acquire);
            assert(!was_busy && "SpscRing: concurrent push()/pop() calls on the same side");
            (void)was_busy;
        }
        ~SideGuard() { busy_.store(false, std::memory_order_release); }
        SideGuard(const SideGuard &) = delete;
        SideGuard & operator=(const SideGuard &) = delete;
    private:
        std::atomic<bool> & busy_;
    };

    std::atomic<uint32_t> head_{0};
    std::atomic<uint32_t> tail_{0};
    T slots_[Capacity]{};
    // Present in every build mode so the layout doesn't change; only used when !NDEBUG.
    std::atomic<bool> producer_busy_{false};
    std::atomic<bool> consumer_busy_{false};
};

} // namespace kfx::audio
