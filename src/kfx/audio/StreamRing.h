#pragma once
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace kfx::audio {

// Byte ring for one writer and one reader; write()/read() never block. Capacity is rounded up to a power of two.
class StreamRing {
public:
    bool init(size_t capacity_bytes) {
        size_t capacity = 16;
        while (capacity < capacity_bytes) {
            capacity <<= 1;
        }
        buffer_.assign(capacity, 0);
        mask_ = capacity - 1;
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
        return true;
    }

    // Only when neither side is running.
    void release() {
        std::vector<uint8_t>().swap(buffer_);
        mask_ = 0;
    }

    // Returns the number of bytes accepted (may be less than requested).
    size_t write(const void * data, size_t bytes) {
        const size_t head = head_.load(std::memory_order_relaxed);
        const size_t used = head - tail_.load(std::memory_order_acquire);
        const size_t count = std::min(bytes, buffer_.size() - used);
        copy_in(head, static_cast<const uint8_t *>(data), count);
        head_.store(head + count, std::memory_order_release);
        return count;
    }

    // Returns the number of bytes read (may be less than requested).
    size_t read(void * out, size_t bytes) {
        const size_t tail = tail_.load(std::memory_order_relaxed);
        const size_t available = head_.load(std::memory_order_acquire) - tail;
        const size_t count = std::min(bytes, available);
        copy_out(tail, static_cast<uint8_t *>(out), count);
        tail_.store(tail + count, std::memory_order_release);
        return count;
    }

    size_t readable() const {
        return head_.load(std::memory_order_acquire) - tail_.load(std::memory_order_acquire);
    }
    size_t writable() const { return buffer_.size() - readable(); }
    size_t capacity() const { return buffer_.size(); }

private:
    void copy_in(size_t position, const uint8_t * src, size_t count) {
        const size_t start = position & mask_;
        const size_t first = std::min(count, buffer_.size() - start);
        std::memcpy(buffer_.data() + start, src, first);
        std::memcpy(buffer_.data(), src + first, count - first);
    }
    void copy_out(size_t position, uint8_t * dst, size_t count) const {
        const size_t start = position & mask_;
        const size_t first = std::min(count, buffer_.size() - start);
        std::memcpy(dst, buffer_.data() + start, first);
        std::memcpy(dst + first, buffer_.data(), count - first);
    }

    std::vector<uint8_t> buffer_;
    size_t mask_ = 0;
    std::atomic<size_t> head_{0};
    std::atomic<size_t> tail_{0};
};

} // namespace kfx::audio
