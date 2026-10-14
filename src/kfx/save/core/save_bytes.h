/******************************************************************************/
/** @file save_bytes.h
 *     Little-endian integers in byte ranges
 */
/******************************************************************************/
#ifndef KFX_SAVE_BYTES_H
#define KFX_SAVE_BYTES_H

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>

namespace savebytes {

struct FreeDeleter {
    void operator()(void *memory) const { free(memory); }
};
using MallocBytes = std::unique_ptr<uint8_t, FreeDeleter>;

/** The unsigned integer of type T stored little endian at p. */
template <typename T>
inline T LoadLE(const uint8_t *bytes)
{
    T value = 0;
    for (size_t index = 0; index < sizeof(T); index++)
        value = static_cast<T>(value | (static_cast<T>(bytes[index]) << (8 * index)));
    return value;
}

template <typename T>
inline void StoreLE(uint8_t *bytes, T value)
{
    for (size_t index = 0; index < sizeof(T); index++)
        bytes[index] = static_cast<uint8_t>(value >> (8 * index));
}

/** Reads from a byte range without going past its end. Once a read has asked for more than is left,
 *  the reader stays failed, and every read after it gives nothing. */
class ByteReader {
public:
    ByteReader(const uint8_t *data, uint32_t len) : data_(data), len_(len) {}

    /** The next n bytes, or NULL when fewer are left. */
    const uint8_t *Take(uint64_t count)
    {
        if (failed_ || (count > static_cast<uint64_t>(len_ - pos_)))
        {
            failed_ = true;
            return nullptr;
        }
        const uint8_t *bytes = data_ + pos_;
        pos_ += static_cast<uint32_t>(count);
        return bytes;
    }

    /** The next integer of type T, or 0 when too few bytes are left. */
    template <typename T>
    T Get()
    {
        const uint8_t *bytes = Take(sizeof(T));
        return bytes ? LoadLE<T>(bytes) : static_cast<T>(0);
    }

    const uint8_t *TakeString(uint16_t *len)
    {
        *len = Get<uint16_t>();
        return Take(*len);
    }

    /** Marks the data as bad, for a length that can't be right. */
    void Fail() { failed_ = true; }
    bool ok() const { return !failed_; }
    uint32_t pos() const { return pos_; }
    uint32_t left() const { return len_ - pos_; }
    const uint8_t *data() const { return data_; }

private:
    const uint8_t *data_;
    uint32_t len_;
    uint32_t pos_ = 0;
    bool failed_ = false;
};

} // namespace savebytes

#endif
