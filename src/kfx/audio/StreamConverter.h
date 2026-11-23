#pragma once
#include <cstdint>

struct SDL_AudioStream;

namespace kfx::audio {

// Converts decoded float audio of any rate (1 or 2 channels) to 48000 Hz float stereo.
class StreamConverter {
public:
    StreamConverter(uint32_t source_rate, uint8_t source_channels);
    ~StreamConverter();
    StreamConverter(const StreamConverter &) = delete;
    StreamConverter & operator=(const StreamConverter &) = delete;

    bool valid() const;
    void push(const float * frames, uint32_t frame_count);
    void flush();                                    // no more input
    uint32_t pull(float * out, uint32_t max_frames); // stereo frames available now
    uint32_t available() const;                     // stereo frames ready to pull

private:
    SDL_AudioStream * stream_ = nullptr;
    uint8_t source_channels_;
};

} // namespace kfx::audio
