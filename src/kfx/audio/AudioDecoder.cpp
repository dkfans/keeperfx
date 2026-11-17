#include "AudioDecoder.h"
#include "AudioFormats.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif
#define DR_WAV_IMPLEMENTATION
#include "../../../deps/dr_wav.h"
#define DR_FLAC_IMPLEMENTATION
#include "../../../deps/dr_flac.h"
#define DR_MP3_IMPLEMENTATION
#include "../../../deps/dr_mp3.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#define STB_VORBIS_HEADER_ONLY
#define STB_VORBIS_NO_PUSHDATA_API
#include "../../../deps/stb_vorbis.c"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace kfx::audio {
namespace {

enum class SeekFrom { Set, Cur, End };

// UTF-8 aware fopen so non-ASCII paths open on Windows.
FILE * open_file_utf8(const char * path) {
#ifdef _WIN32
    const int wide_len = MultiByteToWideChar(CP_UTF8, 0, path, -1, nullptr, 0);
    if (wide_len > 0) {
        std::vector<wchar_t> wide(wide_len);
        MultiByteToWideChar(CP_UTF8, 0, path, -1, wide.data(), wide_len);
        return _wfopen(wide.data(), L"rb");
    }
#endif
    return std::fopen(path, "rb");
}

struct FileCloser { void operator()(FILE * f) const { if (f) std::fclose(f); } };
using FilePtr = std::unique_ptr<FILE, FileCloser>;

// Byte source over a FILE or a memory block; offsets are relative to base.
struct ByteSource {
    FilePtr file;
    const uint8_t * mem = nullptr;
    size_t mem_size = 0;
    size_t mem_pos = 0;
    int64_t base = 0;

    size_t read(void * dst, size_t bytes) {
        if (file) return std::fread(dst, 1, bytes, file.get());
        const size_t n = (mem_pos < mem_size) ? std::min(bytes, mem_size - mem_pos) : 0;
        if (n) std::memcpy(dst, mem + mem_pos, n);
        mem_pos += n;
        return n;
    }
    bool seek(int64_t offset, SeekFrom from) {
        if (file) {
            if (from == SeekFrom::Set) return std::fseek(file.get(), (long)(base + offset), SEEK_SET) == 0;
            return std::fseek(file.get(), (long)offset, from == SeekFrom::Cur ? SEEK_CUR : SEEK_END) == 0;
        }
        int64_t target = offset;
        if (from == SeekFrom::Set) target = base + offset;
        else if (from == SeekFrom::Cur) target = (int64_t)mem_pos + offset;
        else target = (int64_t)mem_size + offset;
        if (target < base || (uint64_t)target > mem_size) return false;
        mem_pos = (size_t)target;
        return true;
    }
    bool tell(int64_t & out) {
        if (file) {
            const long pos = std::ftell(file.get());
            if (pos < 0) return false;
            out = (int64_t)pos - base;
        } else {
            out = (int64_t)mem_pos - base;
        }
        return true;
    }
};

SeekFrom seek_from(int origin) {
    return origin == 0 ? SeekFrom::Set : (origin == 1 ? SeekFrom::Cur : SeekFrom::End);
}

size_t src_read(void * user, void * dst, size_t bytes) {
    return static_cast<ByteSource *>(user)->read(dst, bytes);
}
bool src_seek(void * user, int offset, int origin) {
    return static_cast<ByteSource *>(user)->seek(offset, seek_from(origin));
}
bool src_tell(void * user, long long * cursor) {
    int64_t pos = 0;
    if (!static_cast<ByteSource *>(user)->tell(pos)) return false;
    *cursor = pos;
    return true;
}

class DrWavDecoder final : public AudioDecoder {
public:
    ByteSource src;
    drwav wav{};
    bool ok = false;
    ~DrWavDecoder() override { if (ok) drwav_uninit(&wav); }
    uint32_t sample_rate() const override { return wav.sampleRate; }
    uint8_t channels() const override { return (uint8_t)wav.channels; }
    uint32_t read(float * out, uint32_t max_frames) override {
        return (uint32_t)drwav_read_pcm_frames_f32(&wav, max_frames, out);
    }
    bool rewind() override { return drwav_seek_to_pcm_frame(&wav, 0) == DRWAV_TRUE; }
};

class DrFlacDecoder final : public AudioDecoder {
public:
    ByteSource src;
    drflac * flac = nullptr;
    ~DrFlacDecoder() override { if (flac) drflac_close(flac); }
    uint32_t sample_rate() const override { return flac->sampleRate; }
    uint8_t channels() const override { return (uint8_t)flac->channels; }
    uint32_t read(float * out, uint32_t max_frames) override {
        return (uint32_t)drflac_read_pcm_frames_f32(flac, max_frames, out);
    }
    bool rewind() override { return drflac_seek_to_pcm_frame(flac, 0) == DRFLAC_TRUE; }
};

class DrMp3Decoder final : public AudioDecoder {
public:
    ByteSource src;
    drmp3 mp3{};
    bool ok = false;
    ~DrMp3Decoder() override { if (ok) drmp3_uninit(&mp3); }
    uint32_t sample_rate() const override { return mp3.sampleRate; }
    uint8_t channels() const override { return (uint8_t)mp3.channels; }
    uint32_t read(float * out, uint32_t max_frames) override {
        return (uint32_t)drmp3_read_pcm_frames_f32(&mp3, max_frames, out);
    }
    bool rewind() override { return drmp3_seek_to_pcm_frame(&mp3, 0) == DRMP3_TRUE; }
};

// stb_vorbis owns its input: a FILE it closes, or the caller's memory block.
class VorbisDecoder final : public AudioDecoder {
public:
    stb_vorbis * vorbis = nullptr;
    uint32_t rate = 0;
    uint8_t chans = 0;
    ~VorbisDecoder() override { if (vorbis) stb_vorbis_close(vorbis); }
    uint32_t sample_rate() const override { return rate; }
    uint8_t channels() const override { return chans; }
    uint32_t read(float * out, uint32_t max_frames) override {
        const int got = stb_vorbis_get_samples_float_interleaved(vorbis, chans, out, (int)(max_frames * chans));
        return got > 0 ? (uint32_t)got : 0;
    }
    bool rewind() override { return stb_vorbis_seek_start(vorbis) != 0; }
};

std::unique_ptr<AudioDecoder> fail(std::string & error, const char * text) {
    error = text;
    return nullptr;
}

const char * const kChannelError = "only mono and stereo audio is supported";

std::unique_ptr<AudioDecoder> make_decoder(AudioFormat format, std::unique_ptr<ByteSource> src, std::string & error) {
    switch (format) {
    case AudioFormat::Ogg: {
        auto dec = std::make_unique<VorbisDecoder>();
        int vorbis_error = 0;
        if (src->file) {
            // stb_vorbis closes the FILE itself, including when the open fails.
            dec->vorbis = stb_vorbis_open_file(src->file.release(), 1, &vorbis_error, nullptr);
        } else {
            dec->vorbis = stb_vorbis_open_memory(src->mem, (int)src->mem_size, &vorbis_error, nullptr);
        }
        if (!dec->vorbis) return fail(error, "cannot open Ogg Vorbis data");
        const stb_vorbis_info info = stb_vorbis_get_info(dec->vorbis);
        if (info.channels < 1 || info.channels > 2) return fail(error, kChannelError);
        dec->rate = info.sample_rate;
        dec->chans = (uint8_t)info.channels;
        return dec;
    }
    case AudioFormat::Wav: {
        auto dec = std::make_unique<DrWavDecoder>();
        dec->src = std::move(*src);
        if (!drwav_init(&dec->wav,
                [](void * u, void * d, size_t n) { return src_read(u, d, n); },
                [](void * u, int off, drwav_seek_origin o) -> drwav_bool32 {
                    return src_seek(u, off, (int)o) ? DRWAV_TRUE : DRWAV_FALSE; },
                [](void * u, drwav_int64 * c) -> drwav_bool32 {
                    return src_tell(u, c) ? DRWAV_TRUE : DRWAV_FALSE; },
                &dec->src, nullptr)) return fail(error, "cannot open WAV data");
        dec->ok = true;
        if (dec->wav.channels < 1 || dec->wav.channels > 2) return fail(error, kChannelError);
        return dec;
    }
    case AudioFormat::Flac: {
        auto dec = std::make_unique<DrFlacDecoder>();
        dec->src = std::move(*src);
        dec->flac = drflac_open(
                [](void * u, void * d, size_t n) { return src_read(u, d, n); },
                [](void * u, int off, drflac_seek_origin o) -> drflac_bool32 {
                    return src_seek(u, off, (int)o) ? DRFLAC_TRUE : DRFLAC_FALSE; },
                [](void * u, drflac_int64 * c) -> drflac_bool32 {
                    return src_tell(u, c) ? DRFLAC_TRUE : DRFLAC_FALSE; },
                &dec->src, nullptr);
        if (!dec->flac) return fail(error, "cannot open FLAC data");
        if (dec->flac->channels < 1 || dec->flac->channels > 2) return fail(error, kChannelError);
        return dec;
    }
    case AudioFormat::Mp3: {
        auto dec = std::make_unique<DrMp3Decoder>();
        dec->src = std::move(*src);
        if (!drmp3_init(&dec->mp3,
                [](void * u, void * d, size_t n) { return src_read(u, d, n); },
                [](void * u, int off, drmp3_seek_origin o) -> drmp3_bool32 {
                    return src_seek(u, off, (int)o) ? DRMP3_TRUE : DRMP3_FALSE; },
                [](void * u, drmp3_int64 * c) -> drmp3_bool32 {
                    return src_tell(u, c) ? DRMP3_TRUE : DRMP3_FALSE; },
                nullptr, &dec->src, nullptr)) return fail(error, "cannot open MP3 data");
        dec->ok = true;
        if (dec->mp3.channels < 1 || dec->mp3.channels > 2) return fail(error, kChannelError);
        return dec;
    }
    default:
        break;
    }
    return fail(error, "unsupported audio data");
}

const size_t kBmuHeaderSize = 8;

bool has_bmu_header(const uint8_t * data, size_t size) {
    return size >= kBmuHeaderSize && std::memcmp(data, "BMU V1.0", kBmuHeaderSize) == 0;
}

} // namespace

std::unique_ptr<AudioDecoder> open_audio_file(const char * path, std::string & error) {
    auto source = std::make_unique<ByteSource>();
    source->file.reset(open_file_utf8(path));
    if (!source->file) return fail(error, "cannot open file");
    uint8_t header[12] = {};
    const size_t got = std::fread(header, 1, sizeof(header), source->file.get());
    const AudioFormat format = audio_format_from_header(header, got);
    if (format == AudioFormat::None) return fail(error, "not a supported audio file");
    if (format == AudioFormat::Mp3 && has_bmu_header(header, got)) source->base = (int64_t)kBmuHeaderSize;
    if (std::fseek(source->file.get(), (long)source->base, SEEK_SET) != 0) return fail(error, "cannot read file");
    return make_decoder(format, std::move(source), error);
}

std::unique_ptr<AudioDecoder> open_audio_memory(const uint8_t * data, size_t size, std::string & error) {
    auto source = std::make_unique<ByteSource>();
    source->mem = data;
    source->mem_size = size;
    const AudioFormat format = audio_format_from_header(data, size);
    if (format == AudioFormat::None) return fail(error, "not a supported audio file");
    if (format == AudioFormat::Mp3 && has_bmu_header(data, size)) source->base = (int64_t)kBmuHeaderSize;
    source->mem_pos = (size_t)source->base;
    return make_decoder(format, std::move(source), error);
}

} // namespace kfx::audio
