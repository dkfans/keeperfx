#include "pre_inc.h"
// Sound bank loading, custom sounds, and the sample id tables.
#include "config_keeperfx.h"
#include "bflib_sndlib.h"
#include "bflib_sound.h"
#include "bflib_fileio.h"
#include "game_legacy.h"
#include "kfx/profiling/KfxProfiling.h"
#include "kfx/audio/AudioLog.h"
#include "kfx/audio/SoundInternal.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

// Single-header MP3 decoder (no external DLL dependency)
#include "../../../deps/dr_mp3.h"
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <utility>
#include <array>
#include <unordered_map>

#include "keeperfx.hpp"
#include "post_inc.h"

namespace {

inline uint32_t make_fourcc(const char (& code)[5]) {
	return
		(uint32_t(code[0]) << 0) |
		(uint32_t(code[1]) << 8) |
		(uint32_t(code[2]) << 16) |
		(uint32_t(code[3]) << 24);
}

#define WAVE_FORMAT_PCM 1
#define WAVE_FORMAT_ADPCM 2

#pragma pack(1)
struct riff_chunk_t {
	uint32_t tag;
	uint32_t size;
	// zero or more bytes of data
	// padding byte if data size not a multiple of two
};
#pragma pack()

#pragma pack(1)
struct WAVEFORMATEX {
	uint16_t wFormatTag;
	uint16_t nChannels;
	uint32_t nSamplesPerSec;
	uint32_t nAvgBytesPerSec;
	uint16_t nBlockAlign;
	uint16_t wBitsPerSample;
	// uint16_t cbSize;
};
#pragma pack()

} // local

class wave_file {
public:
	wave_file(std::istream & stream) {
		riff_chunk_t riff_header;
		stream.read(reinterpret_cast<char *>(&riff_header), sizeof(riff_header));
		if (riff_header.tag != make_fourcc("RIFF")) {
			throw std::runtime_error("Expected RIFF chunk");
		}
		uint32_t filetype;
		stream.read(reinterpret_cast<char *>(&filetype), sizeof(filetype));
		if (filetype != make_fourcc("WAVE")) {
			throw std::runtime_error("Expected WAVE chunk");
		}
		riff_chunk_t chunk;
		for (bool have_format = false, have_data = false; !(have_format && have_data);) {
			stream.read(reinterpret_cast<char *>(&chunk), sizeof(chunk));
			if (chunk.tag == make_fourcc("fmt ")) {
				if (chunk.size < sizeof(WAVEFORMATEX)) {
					throw std::runtime_error("Expected WAVEFORMATEX struct");
				}
				WAVEFORMATEX formatex;
				stream.read(reinterpret_cast<char *>(&formatex), sizeof(formatex));
				if (!(formatex.wFormatTag == WAVE_FORMAT_PCM || formatex.wFormatTag == WAVE_FORMAT_ADPCM)) {
					throw std::runtime_error("Unsupported format");
				} else if (formatex.nChannels == 1 && formatex.wBitsPerSample == 4) {
					m_format = AL_FORMAT_MONO_MSADPCM_SOFT;
				} else if (formatex.nChannels == 1 && formatex.wBitsPerSample == 8) {
					m_format = AL_FORMAT_MONO8;
				} else if (formatex.nChannels == 1 && formatex.wBitsPerSample == 16) {
					m_format = AL_FORMAT_MONO16;
				} else if (formatex.nChannels == 2 && formatex.wBitsPerSample == 4) {
					m_format = AL_FORMAT_STEREO_MSADPCM_SOFT;
				} else if (formatex.nChannels == 2 && formatex.wBitsPerSample == 8) {
					m_format = AL_FORMAT_STEREO8;
				} else if (formatex.nChannels == 2 && formatex.wBitsPerSample == 16) {
					m_format = AL_FORMAT_STEREO16;
				} else {
					throw std::runtime_error("Unsupported format");
				}
				m_samplerate = formatex.nSamplesPerSec;
				m_block_align = formatex.nBlockAlign;
				if (chunk.size > sizeof(formatex)) {
					stream.seekg(chunk.size - sizeof(formatex), std::ios::cur);
				}
				have_format = true;
			} else if (chunk.tag == make_fourcc("data")) {
				m_pcm.resize(chunk.size);
				stream.read(reinterpret_cast<char *>(m_pcm.data()), m_pcm.size());
				have_data = true;
			} else {
				stream.seekg(chunk.size, std::ios::cur);
			}
		}
	}

	inline const std::vector<uint8_t> & pcm() const {
		return m_pcm;
	}

	inline int samplerate() const {
		return m_samplerate;
	}

	inline ALenum format() const {
		return m_format;
	}

	inline int block_align() const {
		return m_block_align;
	}

protected:
	int m_samplerate = 0;
	int m_block_align = 0;
	ALenum m_format = 0;
	std::vector<uint8_t> m_pcm;
};

sound_sample::sound_sample(const char * _name, SoundSFXID _sfx_id, const wave_file & wav) {
	name = _name;
	sfx_id = _sfx_id;
	const auto & pcm = wav.pcm();
	const auto format = wav.format();
	if (format == AL_FORMAT_MONO_MSADPCM_SOFT || format == AL_FORMAT_STEREO_MSADPCM_SOFT) {
		buffer_msadpcm(wav);
	} else {
		alBufferData(buffer.id, format, pcm.data(), pcm.size(), wav.samplerate());
	}
	const auto errcode = alGetError();
	if (errcode != AL_NO_ERROR) {
		throw openal_error("Cannot buffer sample data", errcode);
	}
}

// OpenAL decodes MS-ADPCM itself; it needs the block size in sample frames and
// only accepts whole blocks. A sample it rejects is replaced with silence so one
// bad entry can't fail the whole bank load.
void sound_sample::buffer_msadpcm(const wave_file & wav) {
	const auto & data = wav.pcm();
	const int channels = (wav.format() == AL_FORMAT_MONO_MSADPCM_SOFT) ? 1 : 2;
	const int block_align = wav.block_align();
	if (block_align > 7 * channels) {
		const ALint frames_per_block = (block_align - 7 * channels) * 2 / channels + 2;
		const size_t whole_blocks = data.size() - (data.size() % block_align);
		alBufferi(buffer.id, AL_UNPACK_BLOCK_ALIGNMENT_SOFT, frames_per_block);
		alBufferData(buffer.id, wav.format(), data.data(), (ALsizei)whole_blocks, wav.samplerate());
		if (alGetError() == AL_NO_ERROR) {
			return;
		}
	}
	AUDIOWARN("Cannot decode MS-ADPCM sample %s, using silence", name.c_str());
	const int16_t silence = 0;
	alBufferData(buffer.id, AL_FORMAT_MONO16, &silence, sizeof(silence), wav.samplerate());
}

sound_sample::sound_sample(const char * _name, SoundSFXID _sfx_id,
                           const std::vector<uint8_t> & pcm, ALenum format, int samplerate) {
	name = _name;
	sfx_id = _sfx_id;
	alBufferData(buffer.id, format, pcm.data(), (ALsizei)pcm.size(), samplerate);
	const auto errcode = alGetError();
	if (errcode != AL_NO_ERROR) {
		throw openal_error("Cannot buffer sample data", errcode);
	}
}

namespace {

#pragma pack(1)
struct SoundBankHead { // sizeof = 18
	uint8_t signature[14];
	uint32_t version;
};
#pragma pack()

#pragma pack(1)
struct SoundBankSample { // sizeof = 32
	/** Name of the sound file the sample comes from. */
	char filename[18];
	/** Offset of the sample data. */
	uint32_t data_offset;
	uint32_t sample_rate;
	/** Size of the sample file. */
	uint32_t data_size;
	SoundSFXID sfxid;
	uint8_t format_flags;
};
#pragma pack()

#pragma pack(1)
struct SoundBankEntry { // sizeof = 16
	uint32_t first_sample_offset;
	uint32_t first_data_offset;
	uint32_t total_samples_size;
	uint32_t entries_count;
};
#pragma pack()

std::vector<sound_sample> load_sound_bank(const char * filename) {
	const int directory_index = 2; // a5 was always 1622
	std::ifstream stream(filename, std::ios::in | std::ios::binary);
	if (!stream.is_open()) {
		throw std::runtime_error("Cannot open sound bank file");
	}
	stream.seekg(-4, std::ios::end);
	uint32_t head_offset;
	stream.read(reinterpret_cast<char *>(&head_offset), sizeof(head_offset));
	stream.seekg(head_offset, std::ios::beg);
	SoundBankHead bhead;
	stream.read(reinterpret_cast<char *>(&bhead), sizeof(bhead));
	SoundBankEntry bentries[9];
	stream.read(reinterpret_cast<char *>(bentries), sizeof(bentries));
	const auto & directory = bentries[directory_index];
	if (directory.first_sample_offset == 0) {
		throw std::runtime_error("Invalid sample offset");
	} else if (directory.total_samples_size < sizeof(SoundBankSample)) {
		throw std::runtime_error("Invalid samples size");
	}
	const int sample_count = directory.total_samples_size / sizeof(SoundBankSample);
	stream.seekg(directory.first_sample_offset, std::ios::beg);
	std::vector<sound_sample> buffers;
	buffers.reserve(sample_count);
	SoundBankSample sample;
	for (int i = 0; i < sample_count; ++i) {
		stream.seekg(directory.first_sample_offset + (sizeof(sample) * i), std::ios::beg);
		stream.read(reinterpret_cast<char *>(&sample), sizeof(sample));
		stream.seekg(directory.first_data_offset + sample.data_offset, std::ios::beg);
		buffers.emplace_back(sample.filename, sample.sfxid, wave_file(stream));
	}
	return buffers;
}

// Stops and flushes every voice using samples >= first_smptbl_id; false means don't delete the buffers.
bool stop_voices_using_samples_from(SoundSmplTblID first_smptbl_id) {
	if (!g_audio_callback.initialized()) {
		return true;
	}
	if (!g_audio_callback.flush()) { // empties the ring so every Stop below fits
		return false;
	}
	g_voice_mirror.stop_samples_from(first_smptbl_id, g_command_ring);
	return g_audio_callback.flush();
}

} // local

std::array<std::vector<sound_sample>, 2> g_banks;
std::vector<sound_sample> g_custom_bank;  // Third bank for custom sounds loaded at runtime
SoundSmplTblID g_speech_offset = 0;  // Unified ID start of speech bank
SoundSmplTblID g_custom_offset = 0;  // Unified ID start of custom bank

std::unordered_map<SoundSmplTblID, SoundSmplTblID> g_id_redirects;

std::unordered_map<SoundSmplTblID, SoundStackPolicy> g_stack_policies;

// Feeds g_stack_policies to the VoiceMirror, which can't see this file's tables.
bool lookup_stack_policy(SoundSmplTblID smptbl_id, kfx::audio::StackPolicy & out) {
	const auto it = g_stack_policies.find(smptbl_id);
	if (it == g_stack_policies.end()) {
		return false;
	}
	out.mode = it->second.mode;
	out.max_instances = it->second.max_instances;
	return true;
}

// For when buffers can't be deleted safely: a later InitAudio must not delete reused ids.
void forget_bank_buffers(std::vector<sound_sample> & bank) {
	for (auto & sample : bank) {
		sample.buffer.id = 0;
	}
	bank.clear();
}

void load_sound_banks() {
	KFX_ZONE_COLOR("load_sound_banks", KFX_COLOR_AUDIO);
	char snd_fname[2048];
	prepare_file_path_buf(snd_fname, sizeof(snd_fname), FGrp_LrgSound, "sound.dat");
	// language-specific speech file
	char * spc_fname = prepare_file_fmtpath(FGrp_LrgSound, "speech_%s.dat", get_language_lwrstr(install_info.lang_id));
	// default speech file
	if (!LbFileExists(spc_fname)) {
		spc_fname = prepare_file_path(FGrp_LrgSound, "speech.dat");
	}
	// speech file for english
	if (!LbFileExists(spc_fname)) {
		spc_fname = prepare_file_fmtpath(FGrp_LrgSound, "speech_%s.dat", get_language_lwrstr(1));
	}
	g_banks[0] = load_sound_bank(snd_fname);
	g_banks[1] = load_sound_bank(spc_fname);
	AUDIOLOG("Loaded %s (%d samples), %s (%d samples)",
		snd_fname, (int)g_banks[0].size(), spc_fname, (int)g_banks[1].size());
	g_speech_offset = (SoundSmplTblID)g_banks[0].size();
	g_custom_offset = g_speech_offset + (SoundSmplTblID)g_banks[1].size();
}

extern "C" void custom_sound_bank_clear() {
	if (stop_voices_using_samples_from(g_custom_offset)) {
		g_custom_bank.clear();
	} else {
		AUDIOERR("Audio flush failed, leaving custom sound buffers undeleted");
	}
	g_id_redirects.clear();
	g_stack_policies.clear();
	g_voice_mirror.clear_tick_gate();
}

extern "C" void sound_register_id_redirect(SoundSmplTblID from_id, SoundSmplTblID to_id) {
	g_id_redirects[from_id] = to_id;
	SYNCDBG(7, "Registered ID redirect: %d -> %d", from_id, to_id);
}

extern "C" void sound_clear_id_redirects(void) {
	g_id_redirects.clear();
}

extern "C" void sound_register_stack_policy(SoundSmplTblID smptbl_id, unsigned char mode, short max_instances) {
	SoundStackPolicy policy;
	policy.mode = mode;
	policy.max_instances = (mode == SStack_Limit) ? std::max<short>(max_instances, 1) : std::max<short>(max_instances, 0);
	g_stack_policies[smptbl_id] = policy;
	SYNCDBG(7, "Registered stack policy for sample %d: mode %d, max %d", smptbl_id, mode, policy.max_instances);
}

extern "C" void sound_clear_stack_policies(void) {
	g_stack_policies.clear();
}

static std::unordered_map<SoundSmplTblID, SoundSmplTblID> g_id_redirects_snapshot;
static std::unordered_map<SoundSmplTblID, SoundStackPolicy> g_stack_policies_snapshot;
static size_t g_custom_bank_watermark = 0;

extern "C" void sound_save_id_redirect_snapshot(void) {
	g_id_redirects_snapshot = g_id_redirects;
	g_stack_policies_snapshot = g_stack_policies;
	g_custom_bank_watermark = g_custom_bank.size();
	SYNCDBG(7, "Saved sound snapshot: %" PRIuSIZE " redirects, %" PRIuSIZE " stack policies, %" PRIuSIZE " custom bank entries",
		SZCAST(g_id_redirects_snapshot.size()), SZCAST(g_stack_policies_snapshot.size()), SZCAST(g_custom_bank_watermark));
}

extern "C" void sound_restore_id_redirect_snapshot(void) {
	g_id_redirects = g_id_redirects_snapshot;
	g_stack_policies = g_stack_policies_snapshot;
	if (g_custom_bank.size() > g_custom_bank_watermark) {
		SYNCDBG(7, "Trimming custom bank from %" PRIuSIZE " to %" PRIuSIZE " entries",
			SZCAST(g_custom_bank.size()), SZCAST(g_custom_bank_watermark));
		if (stop_voices_using_samples_from(g_custom_offset + (SoundSmplTblID)g_custom_bank_watermark)) {
			g_custom_bank.erase(g_custom_bank.begin() + (ptrdiff_t)g_custom_bank_watermark, g_custom_bank.end());
		} else {
			AUDIOERR("Audio flush failed, leaving custom sound buffers undeleted");
		}
	}
	SYNCDBG(7, "Restored sound snapshot: %" PRIuSIZE " redirects, %" PRIuSIZE " stack policies",
		SZCAST(g_id_redirects.size()), SZCAST(g_stack_policies.size()));
}

extern "C" SoundSFXID get_sample_sfxid(SoundSmplTblID smptbl_id) {
	if (smptbl_id >= g_custom_offset) {
		return 0;
	} else if (smptbl_id >= g_speech_offset) {
		const SoundSmplTblID idx = smptbl_id - g_speech_offset;
		if (idx <= 0 || idx >= (SoundSmplTblID)g_banks[1].size()) return 0;
		return g_banks[1][idx].sfx_id;
	} else {
		if (smptbl_id <= 0 || smptbl_id >= (SoundSmplTblID)g_banks[0].size()) return 0;
		return g_banks[0][smptbl_id].sfx_id;
	}
}

extern "C" SoundSmplTblID get_speech_offset(void) { return g_speech_offset; }
extern "C" SoundSmplTblID get_custom_offset(void) { return g_custom_offset; }

// Bridge functions for custom sound loading from C++ sound_manager
extern "C" int custom_sound_bank_size() {
	return g_custom_bank.size();
}

// Decode data as MP3 (via dr_mp3) and push it into g_custom_bank.
// data/size may point into a larger buffer (e.g. after stripping a BMU header).
static TbBool decode_mp3_and_store(const char* filepath, int sample_id,
	const uint8_t* data, size_t size)
{
	drmp3_config cfg = {};
	drmp3_uint64 frame_count = 0;
	drmp3_int16* mp3_pcm = drmp3_open_memory_and_read_pcm_frames_s16(
		data, size, &cfg, &frame_count, nullptr);
	if (!mp3_pcm || frame_count == 0) {
		if (mp3_pcm) drmp3_free(mp3_pcm, nullptr);
		AUDIOERR("Cannot decode MP3 data from %s", filepath);
		return false;
	}

	const ALenum al_fmt = (cfg.channels == 1)
		? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
	const size_t byte_count =
		(size_t)frame_count * cfg.channels * sizeof(drmp3_int16);

	try {
		const std::vector<uint8_t> pcm(
			reinterpret_cast<const uint8_t*>(mp3_pcm),
			reinterpret_cast<const uint8_t*>(mp3_pcm) + byte_count);
		g_custom_bank.emplace_back(filepath, sample_id, pcm,
			al_fmt, (int)cfg.sampleRate);
	} catch (...) {
		drmp3_free(mp3_pcm, nullptr);
		return false;
	}
	drmp3_free(mp3_pcm, nullptr);
	return true;
}

// Decode an in-memory WAV/OGG/FLAC/MP3/BMU buffer and append it to g_custom_bank as a
// signed-16-bit OpenAL buffer. Shared by the disk-file and in-memory (e.g. zip-sourced)
// loading entry points below.
//   - Plain WAV/RIFF : wave_file (same in-process header parser the base sound.dat bank
//                       uses) — preserves the source's own channel count/format exactly.
//   - OGG / FLAC     : SDL3_mixer MIX_AudioDecoder, keeping the source's channel count
//                       (mono or stereo) and rate.
//   - Plain MP3      : dr_mp3 single-header decoder (compiled in, no external DLLs)
//   - BMU V1.0       : 8-byte wrapper used by some campaigns; contains a plain MP3
//                       stream after the header — stripped and decoded via dr_mp3.
static TbBool decode_audio_buffer_and_store(const char* logical_name, int sample_id,
	const uint8_t* data, size_t size)
{
	KFX_ZONE_COLOR("decode_custom_sound", KFX_COLOR_AUDIO);
	// Detect BMU V1.0 wrapper (8-byte ASCII prefix used by some campaigns).
	// After the prefix the payload is a standard MP3 (often with an ID3 tag).
	static const uint8_t bmu_magic[8] = { 'B','M','U',' ','V','1','.','0' };
	if (size > 8 && memcmp(data, bmu_magic, 8) == 0) {
		return decode_mp3_and_store(logical_name, sample_id, data + 8, size - 8);
	}

	// Detect MP3 by ID3v2 tag or sync-word (0xFF 0xE? / 0xFF 0xF?).
	// Also handle plain .mp3 extension as a hint.
	const bool looks_like_mp3 =
		(size >= 3 && data[0] == 'I' && data[1] == 'D' && data[2] == '3') ||
		(size >= 2 && data[0] == 0xFF && (data[1] & 0xE0) == 0xE0);
	if (looks_like_mp3) {
		return decode_mp3_and_store(logical_name, sample_id, data, size);
	}

	// --- Plain WAV/RIFF path: parse with wave_file, same as the base sound.dat bank uses.
	// This preserves the file's own format exactly, including ADPCM.
	if (size >= 12 && memcmp(data, "RIFF", 4) == 0 && memcmp(data + 8, "WAVE", 4) == 0) {
		try {
			std::string buf(reinterpret_cast<const char*>(data), size);
			std::istringstream stream(buf, std::ios::binary);
			g_custom_bank.emplace_back(logical_name, sample_id, wave_file(stream));
			return true;
		} catch (const std::exception & e) {
			AUDIOERR("Cannot parse WAV audio %s: %s", logical_name, e.what());
			return false;
		}
	}

	// --- OGG / FLAC (and anything else the mixer decodes) path ---
	// SDL3_mixer no longer exposes raw PCM from a loaded MIX_Audio (as SDL2's
	// Mix_Chunk::abuf did), so use a MIX_AudioDecoder to decode the buffer to
	// the same output format the old mixer produced (44100 Hz / S16 / stereo)
	// and hand that PCM to OpenAL. This keeps OGG/FLAC custom-sound support.
	SDL_IOStream* io = SDL_IOFromConstMem(data, size);
	if (!io) {
		AUDIOERR("Cannot create IOStream for %s: %s", logical_name, SDL_GetError());
		return false;
	}

	// closeio=true: the decoder takes ownership of the stream.
	MIX_AudioDecoder* decoder = MIX_CreateAudioDecoder_IO(io, true, 0);
	if (!decoder) {
		AUDIOERR("Cannot decode audio file %s: %s", logical_name, SDL_GetError());
		return false;
	}

	// Keep the source's channel count and rate: forcing stereo stops OpenAL from positioning
	// the sound, and upmixing mono makes it louder. More than two channels is downmixed.
	SDL_AudioSpec src_spec;
	if (!MIX_GetAudioDecoderFormat(decoder, &src_spec)) {
		AUDIOERR("Cannot read audio format of %s: %s", logical_name, SDL_GetError());
		MIX_DestroyAudioDecoder(decoder);
		return false;
	}
	const SDL_AudioSpec out_spec = { SDL_AUDIO_S16, (src_spec.channels == 1) ? 1 : 2, src_spec.freq };
	const ALenum al_fmt = (out_spec.channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

	std::vector<uint8_t> pcm;
	try {
		uint8_t buffer[16384];
		for (;;) {
			int got = MIX_DecodeAudio(decoder, buffer, sizeof(buffer), &out_spec);
			if (got <= 0) break; // 0 == end of stream, <0 == error
			pcm.insert(pcm.end(), buffer, buffer + got);
		}
	} catch (...) {
		MIX_DestroyAudioDecoder(decoder);
		AUDIOERR("Out of memory buffering audio %s", logical_name);
		return false;
	}
	MIX_DestroyAudioDecoder(decoder);
	try {
		g_custom_bank.emplace_back(logical_name, sample_id, pcm, al_fmt, out_spec.freq);
	} catch (const std::exception & e) {
		AUDIOERR("Cannot store audio %s: %s", logical_name, e.what());
		return false;
	}
	return true;
}

// Load a WAV, OGG, FLAC, or MP3 file from disk and append it to g_custom_bank.
extern "C" TbBool custom_sound_load_wav(const char* filepath, int sample_id)
{
	// Resolve to absolute path so the decoders can find the file regardless of
	// process CWD (keeperfx changes directories at startup).
	char abs_buf[4096];
#ifdef _WIN32
	if (_fullpath(abs_buf, filepath, sizeof(abs_buf)) == nullptr)
		snprintf(abs_buf, sizeof(abs_buf), "%s", filepath);
#else
	if (realpath(filepath, abs_buf) == nullptr)
		snprintf(abs_buf, sizeof(abs_buf), "%s", filepath);
#endif

	// Read the whole file once so we can inspect the magic bytes and route to
	// the right decoder without reopening.
	std::vector<uint8_t> file_data;
	{
		std::ifstream f(abs_buf, std::ios::binary | std::ios::ate);
		if (!f) {
			AUDIOERR("Cannot open audio file %s", filepath);
			return false;
		}
		const auto sz = f.tellg();
		if (sz <= 0) {
			AUDIOERR("Empty audio file %s", filepath);
			return false;
		}
		file_data.resize((size_t)sz);
		f.seekg(0);
		if (!f.read(reinterpret_cast<char*>(file_data.data()), sz)) {
			AUDIOERR("Cannot read audio file %s", filepath);
			return false;
		}
	}

	return decode_audio_buffer_and_store(filepath, sample_id, file_data.data(), file_data.size());
}

// Load a WAV, OGG, FLAC, or MP3 buffer already in memory (e.g. read out of a map's
// zip bundle) and append it to g_custom_bank. logical_name is only used for logging.
extern "C" TbBool custom_sound_load_wav_mem(const unsigned char* data, size_t size,
	const char* logical_name, int sample_id)
{
	if (data == nullptr || size == 0) {
		AUDIOERR("Empty audio buffer for %s", logical_name);
		return false;
	}
	return decode_audio_buffer_and_store(logical_name, sample_id, data, size);
}
