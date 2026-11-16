#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
namespace kfx::audio {

// Audio file kinds the engine can play. None means unsupported.
enum class AudioFormat : uint8_t { None, Wav, Mp3, Flac, Ogg };

AudioFormat audio_format_from_path(const char * path);
AudioFormat audio_format_from_header(const uint8_t * data, size_t size);
int audio_format_music_priority(AudioFormat format);
const char * audio_format_list_text();

} // namespace kfx::audio

extern "C" {
#endif
int kfx_audio_path_supported(const char * path);
const char * kfx_audio_format_list_text(void);
#ifdef __cplusplus
}
#endif
