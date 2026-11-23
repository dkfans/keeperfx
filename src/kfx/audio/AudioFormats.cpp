#include "AudioFormats.h"
#include <cstring>

namespace kfx::audio {

static bool equals_ignore_case(const char * a, const char * b) {
    for (; *a != '\0' && *b != '\0'; ++a, ++b) {
        const char ca = (*a >= 'A' && *a <= 'Z') ? char(*a - 'A' + 'a') : *a;
        const char cb = (*b >= 'A' && *b <= 'Z') ? char(*b - 'A' + 'a') : *b;
        if (ca != cb) {
            return false;
        }
    }
    return *a == *b;
}

static const char * path_extension(const char * path) {
    if (path == nullptr) {
        return nullptr;
    }
    const char * dot = strrchr(path, '.');
    const char * slash = strrchr(path, '/');
    const char * backslash = strrchr(path, '\\');
    if (dot == nullptr || dot < slash || dot < backslash) {
        return nullptr;
    }
    return dot;
}

AudioFormat audio_format_from_path(const char * path) {
    const char * dot = path_extension(path);
    if (dot == nullptr) {
        return AudioFormat::None;
    }
    if (equals_ignore_case(dot, ".wav")) return AudioFormat::Wav;
    if (equals_ignore_case(dot, ".mp3") || equals_ignore_case(dot, ".bmu")) return AudioFormat::Mp3;
    if (equals_ignore_case(dot, ".flac")) return AudioFormat::Flac;
    if (equals_ignore_case(dot, ".ogg")) return AudioFormat::Ogg;
    return AudioFormat::None;
}

AudioFormat audio_format_from_header(const uint8_t * data, size_t size) {
    if (size >= 12 && memcmp(data, "RIFF", 4) == 0 && memcmp(data + 8, "WAVE", 4) == 0) return AudioFormat::Wav;
    if (size >= 4 && memcmp(data, "OggS", 4) == 0) return AudioFormat::Ogg;
    if (size >= 4 && memcmp(data, "fLaC", 4) == 0) return AudioFormat::Flac;
    if (size >= 8 && memcmp(data, "BMU V1.0", 8) == 0) return AudioFormat::Mp3;
    if (size >= 3 && memcmp(data, "ID3", 3) == 0) return AudioFormat::Mp3;
    if (size >= 2 && data[0] == 0xFF && (data[1] & 0xE0) == 0xE0) return AudioFormat::Mp3;
    return AudioFormat::None;
}

int audio_format_music_priority(AudioFormat format) {
    switch (format) {
        case AudioFormat::Flac: return 0;
        case AudioFormat::Wav: return 1;
        case AudioFormat::Ogg: return 2;
        case AudioFormat::Mp3: return 3;
        case AudioFormat::None: break;
    }
    return -1;
}

const char * audio_format_list_text() {
    return "wav, mp3, flac, ogg";
}

bool audio_path_has_extension(const char * path) {
    return path_extension(path) != nullptr;
}

const char * const * audio_format_extensions() {
    static const char * const extensions[] = {".wav", ".mp3", ".ogg", ".flac", nullptr};
    return extensions;
}

} // namespace kfx::audio

extern "C" const char * const * kfx_audio_format_extensions(void) {
    return kfx::audio::audio_format_extensions();
}

extern "C" int kfx_audio_path_supported(const char * path) {
    return kfx::audio::audio_format_from_path(path) != kfx::audio::AudioFormat::None ? 1 : 0;
}

extern "C" const char * kfx_audio_format_list_text(void) {
    return kfx::audio::audio_format_list_text();
}

extern "C" int kfx_audio_path_has_extension(const char * path) {
    return kfx::audio::audio_path_has_extension(path) ? 1 : 0;
}
