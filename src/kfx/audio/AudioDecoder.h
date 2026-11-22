#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace kfx::audio {

class AudioDecoder {
public:
    virtual ~AudioDecoder() = default;
    virtual uint32_t sample_rate() const = 0;
    virtual uint8_t channels() const = 0; // 1 or 2
    // Frames of interleaved float written to out; 0 at end of stream or on error.
    virtual uint32_t read(float * out, uint32_t max_frames) = 0;
    virtual bool rewind() = 0;
    // True when the last read returned 0 because of a read or decode error, not the end of the stream.
    bool failed() const { return failed_; }

protected:
    bool failed_ = false;
};

// The format is chosen from the file header, not the extension; error is set on failure.
std::unique_ptr<AudioDecoder> open_audio_file(const char * path, std::string & error);
// data must stay valid for the decoder's lifetime.
std::unique_ptr<AudioDecoder> open_audio_memory(const uint8_t * data, size_t size, std::string & error);

} // namespace kfx::audio
