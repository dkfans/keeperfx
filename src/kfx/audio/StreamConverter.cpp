#include "StreamConverter.h"
#include "StreamConfig.h"
#include <SDL3/SDL.h>

namespace kfx::audio {

StreamConverter::StreamConverter(uint32_t source_rate, uint8_t source_channels)
    : source_channels_(source_channels) {
    const SDL_AudioSpec src{SDL_AUDIO_F32, int(source_channels), int(source_rate)};
    const SDL_AudioSpec dst{SDL_AUDIO_F32, 2, int(kCoreSampleRate)};
    stream_ = SDL_CreateAudioStream(&src, &dst);
}

StreamConverter::~StreamConverter() {
    if (stream_) SDL_DestroyAudioStream(stream_);
}

bool StreamConverter::valid() const { return stream_ != nullptr; }

void StreamConverter::push(const float * frames, uint32_t frame_count) {
    if (stream_ && frame_count) {
        SDL_PutAudioStreamData(stream_, frames, int(frame_count * source_channels_ * sizeof(float)));
    }
}

void StreamConverter::flush() {
    if (stream_) SDL_FlushAudioStream(stream_);
}

uint32_t StreamConverter::pull(float * out, uint32_t max_frames) {
    if (!stream_) return 0;
    const int bytes = SDL_GetAudioStreamData(stream_, out, int(max_frames * 2 * sizeof(float)));
    return bytes > 0 ? uint32_t(bytes) / (2 * sizeof(float)) : 0;
}

uint32_t StreamConverter::available() const {
    const int bytes = stream_ ? SDL_GetAudioStreamAvailable(stream_) : 0;
    return bytes > 0 ? uint32_t(bytes) / (2 * sizeof(float)) : 0;
}

} // namespace kfx::audio
