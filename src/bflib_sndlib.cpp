#include "pre_inc.h"
#include "config_keeperfx.h"
#include "cdrom.h"
#include "bflib_sndlib.h"
#include "bflib_datetm.h"
#include "bflib_sound.h"
#include "bflib_fileio.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

// Single-header MP3 decoder (no external DLL dependency)
#define DR_MP3_IMPLEMENTATION
#include "../deps/dr_mp3.h"
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <utility>
#include <array>
#include <unordered_map>
#include <deque>
#include <mutex>
#include <atomic>
#include <cmath>

#include "post_inc.h"

namespace {

struct device_deleter {
	void operator()(ALCdevice * device) {
		alcCloseDevice(device);
	}
};

struct context_deleter {
	void operator()(ALCcontext * context) {
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(context);
	}
};

using ALCdevice_ptr = std::unique_ptr<ALCdevice, device_deleter>;
using ALCcontext_ptr = std::unique_ptr<ALCcontext, context_deleter>;

SoundVolume g_master_volume = 0;
SoundVolume g_music_volume = 0;
ALCdevice_ptr g_openal_device;
ALCcontext_ptr g_openal_context;
// SDL3_mixer replaces the SDL2 Mix_Music/Mix_Chunk + channel model with a
// mixer + per-purpose tracks. Music and speech each get their own track.
MIX_Mixer* g_mixer = nullptr;
MIX_Track* g_music_track = nullptr;
MIX_Audio* g_music_audio = nullptr;
MIX_Track* g_speech_track = nullptr;

bool g_bb_king_mode = false;

enum source_flags {
	bb_king_mode = 1,
};

const char * alErrorStr(ALenum code) {
	switch (code) {
		case AL_NO_ERROR: return "No error";
		case AL_INVALID_NAME: return "Invalid name";
		case AL_INVALID_ENUM: return "Invalid enum value";
		case AL_INVALID_VALUE: return "Invalid value";
		case AL_INVALID_OPERATION: return "Invalid operation";
		case AL_OUT_OF_MEMORY: return "Out of memory";
	}
	return "Unknown";
}

class openal_error : public std::runtime_error {
public:
	inline openal_error(const char * description, ALenum errcode = alGetError())
	: runtime_error(std::string("OpenAL error: ") + description + ": " + alErrorStr(errcode))
	{}
};

class openal_buffer {
public:
	ALuint id = 0;

	openal_buffer() {
		ALuint buffers[1];
		alGenBuffers(1, buffers);
		const auto errcode = alGetError();
		if (errcode != AL_NO_ERROR) {
			throw openal_error("Cannot create buffer", errcode);
		}
		id = buffers[0];
	}

	inline ~openal_buffer() noexcept {
		alDeleteBuffers(1, &id);
	}

	openal_buffer(const openal_buffer &) = delete;
	openal_buffer & operator=(const openal_buffer &) = delete;

	inline openal_buffer(openal_buffer && other)
	: id(std::exchange(other.id, 0)) {}

	inline openal_buffer & operator=(openal_buffer && other) {
		id = std::exchange(other.id, 0);
		return *this;
	}
};

class openal_source {
public:
	ALuint id = 0;
	SoundMilesID mss_id = 0;
	SoundEmitterID emit_id = 0;
	SoundSmplTblID smptbl_id = 0;
	int flags = 0;
	SoundVolume base_gain = 0; // requested (un-ducked) volume; used to recompute duck-scaled gain

	openal_source() {
		ALuint sources[1];
		alGenSources(1, sources);
		const auto errcode = alGetError();
		if (errcode != AL_NO_ERROR) {
			throw openal_error("Cannot create source", errcode);
		}
		id = sources[0];
	}

	inline ~openal_source() noexcept {
		alDeleteSources(1, &id);
	}

	void play(const openal_buffer & buffer) {
		alSourcei(id, AL_BUFFER, buffer.id);
		auto errcode = alGetError();
		if (errcode != AL_NO_ERROR) {
			throw openal_error("Cannot attach buffer", errcode);
		}
		alSourcePlay(id);
		errcode = alGetError();
		if (errcode != AL_NO_ERROR) {
			throw openal_error("Cannot play source", errcode);
		}
	}

	void stop() {
		alSourceStop(id);
		const auto errcode = alGetError();
		if (errcode != AL_NO_ERROR) {
			throw openal_error("Cannot stop source", errcode);
		}
	}

	void gain(SoundVolume volume) {
		alSourcef(id, AL_GAIN, float(volume) / FULL_LOUDNESS);
		const auto errcode = alGetError();
		if (errcode != AL_NO_ERROR) {
			throw openal_error("Cannot set volume", errcode);
		}
	}

	void gain_scaled(SoundVolume volume, float scale) {
		alSourcef(id, AL_GAIN, (float(volume) / FULL_LOUDNESS) * scale);
		const auto errcode = alGetError();
		if (errcode != AL_NO_ERROR) {
			throw openal_error("Cannot set volume", errcode);
		}
	}

	void pitch(SoundPitch pitch) {
		alSourcef(id, AL_PITCH, float(pitch) / NORMAL_PITCH);
		const auto errcode = alGetError();
		if (errcode != AL_NO_ERROR) {
			throw openal_error("Cannot set pitch", errcode);
		}
	}

	void pan(SoundPan pan) {
		// convert 0..128 (where 64 is center) to -1.0..1.0 and then reduce stereo separation by 50%
		const auto x = (-(float(64 - pan) / 64.0f)) * 0.5f;
		const auto z = -1.0f; // in front of listener
		alSource3f(id, AL_POSITION, x, 0, z);
		const auto errcode = alGetError();
		if (errcode != AL_NO_ERROR) {
			throw openal_error("Cannot set position", errcode);
		}
	}

	void repeat(bool value) {
		alSourcei(id, AL_LOOPING, value ? AL_TRUE : AL_FALSE);
		const auto errcode = alGetError();
		if (errcode != AL_NO_ERROR) {
			throw openal_error("Cannot toggle looping", errcode);
		}
	}

	bool is_playing() const {
		ALint state = 0;
		alGetSourcei(id, AL_SOURCE_STATE, &state);
		const auto errcode = alGetError();
		if (errcode != AL_NO_ERROR) {
			throw openal_error("Cannot get source state", errcode);
		}
		return state == AL_PLAYING;
	}

	openal_source(const openal_source &) = delete;
	openal_source & operator=(const openal_source &) = delete;

	inline openal_source(openal_source && other)
	: id(std::exchange(other.id, 0))
	, mss_id(std::exchange(other.mss_id, 0))
	, emit_id(std::exchange(other.emit_id, 0))
	, smptbl_id(std::exchange(other.smptbl_id, 0))
	, base_gain(std::exchange(other.base_gain, 0)){}

	inline openal_source & operator=(openal_source && other) {
		id = std::exchange(other.id, 0);
		mss_id = std::exchange(other.mss_id, 0);
		emit_id = std::exchange(other.emit_id, 0);
		smptbl_id = std::exchange(other.smptbl_id, 0);
		base_gain = std::exchange(other.base_gain, 0);
		return *this;
	}
};

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

protected:
	int m_samplerate = 0;
	ALenum m_format = 0;
	std::vector<uint8_t> m_pcm;
};

struct sound_sample {

	std::string name;
	SoundSFXID sfx_id;
	openal_buffer buffer;

	sound_sample(const char * _name, SoundSFXID _sfx_id, const wave_file & wav) {
		name = _name;
		sfx_id = _sfx_id;
		const auto & pcm = wav.pcm();
		const auto format = wav.format();
		if (format == AL_FORMAT_MONO_MSADPCM_SOFT) {
			// Needed for heart6a.wav
			std::vector<uint8_t> converted(pcm.size() * 2);
			for (size_t i = 0; i < pcm.size(); ++i) {
				converted[(i * 2) + 0] = (pcm[i] >> 4) * 2;
				converted[(i * 2) + 1] = (pcm[i] & 0x7) * 2;
			}
			alBufferData(buffer.id, AL_FORMAT_MONO8, converted.data(), converted.size(), wav.samplerate());
		} else if (format == AL_FORMAT_STEREO_MSADPCM_SOFT) {
			throw std::runtime_error("Format not implemented");
		} else {
			alBufferData(buffer.id, format, pcm.data(), pcm.size(), wav.samplerate());
		}
		const auto errcode = alGetError();
		if (errcode != AL_NO_ERROR) {
			throw openal_error("Cannot buffer sample data", errcode);
		}
	}

	sound_sample(const char * _name, SoundSFXID _sfx_id,
	             const std::vector<uint8_t> & pcm, ALenum format, int samplerate) {
		name = _name;
		sfx_id = _sfx_id;
		alBufferData(buffer.id, format, pcm.data(), (ALsizei)pcm.size(), samplerate);
		const auto errcode = alGetError();
		if (errcode != AL_NO_ERROR) {
			throw openal_error("Cannot buffer sample data", errcode);
		}
	}
};

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

std::vector<openal_source> g_sources;
std::array<std::vector<sound_sample>, 2> g_banks;
std::vector<sound_sample> g_custom_bank;  // Third bank for custom sounds loaded at runtime
SoundSmplTblID g_speech_offset = 0;  // Unified ID start of speech bank
SoundSmplTblID g_custom_offset = 0;  // Unified ID start of custom bank

static std::unordered_map<SoundSmplTblID, SoundSmplTblID> g_id_redirects;

struct SoundStackPolicy {
	unsigned char mode = SStack_Limit;
	short max_instances = 1;
};
static std::unordered_map<SoundSmplTblID, SoundStackPolicy> g_stack_policies;

// Tick-scoped gate reproducing the pre-Custom-Sounds behaviour exactly: a sample ID with
// no explicit STACK= policy may only start once per "tick", regardless of which emitter
// triggers it.
static unsigned long g_audio_tick_counter = 0; // tick != turn; ticks continue while navigating the frontend/main menu
static std::unordered_map<SoundSmplTblID, unsigned long> g_tick_samples_last_tick;

static SoundStackPolicy get_stack_policy(SoundSmplTblID smptbl_id) {
	const auto it = g_stack_policies.find(smptbl_id);
	if (it != g_stack_policies.end()) {
		return it->second;
	}
	return SoundStackPolicy{}; // default: Limit, max 1
}

// Recompute duck-scaled gain for every currently active instance of a sample.
// Called after a new instance starts, and after MonitorStreamedSoundTrack() prunes a
// finished one, so remaining instances' volume rises back up as concurrency drops.
static void apply_duck_gain(SoundSmplTblID smptbl_id) {
	int count = 0;
	for (const auto & source : g_sources) {
		if (source.emit_id != 0 && source.smptbl_id == smptbl_id) {
			++count;
		}
	}
	if (count == 0) {
		return;
	}
	const float scale = 1.0f / std::sqrt(float(count));
	for (auto & source : g_sources) {
		if (source.emit_id != 0 && source.smptbl_id == smptbl_id) {
			try {
				source.gain_scaled(source.base_gain, scale);
			} catch (const std::exception & e) {
				ERRORLOG("%s", e.what());
			}
		}
	}
}

void load_sound_banks() {
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
	g_speech_offset = (SoundSmplTblID)g_banks[0].size();
	g_custom_offset = g_speech_offset + (SoundSmplTblID)g_banks[1].size();
}

void print_device_info() {
	if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT")) {
		const auto devices = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);
		for (auto device = devices; device[0] != 0; device += strlen(device)) {
			// Device enumeration
		}
	} else if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT")) {
		const auto devices = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
		for (auto device = devices; device[0] != 0; device += strlen(device)) {
			// Device enumeration
		}
	} else {
		// Cannot enumerate devices :(
	}
}

// The currently-playing streamed speech sample (played on g_speech_track).
__attribute__((unused)) MIX_Audio * g_streamed_sample = nullptr;
std::mutex g_mix_mutex;

std::string g_current_music_fname; // empty if a numbered track (or nothing) is playing
int g_current_music_track = 0;     // 0 if a custom file (or nothing) is playing

struct queued_sample {
	std::string fname;
	SoundVolume volume;
};

} // local

extern "C" void FreeAudio() {
	SYNCDBG(6, "Starting audio cleanup");

	// Stop playback before freeing audio, so the mixer isn't reading it
	if (g_mixer != nullptr) {
		SYNCDBG(7, "SDL_mixer is open, halting playback");
		MIX_StopAllTracks(g_mixer, 0);
	}

	// Free SDL3_mixer resources
	{
		std::lock_guard<std::mutex> guard(g_mix_mutex);
		if (auto music = std::exchange(g_music_audio, (MIX_Audio*)nullptr)) {
			MIX_DestroyAudio(music);
			SYNCDBG(8, "Freed SDL_mixer music");
		}
		g_current_music_track = 0;
		g_current_music_fname.clear();
		if (g_streamed_sample) {
			MIX_DestroyAudio(g_streamed_sample);
			g_streamed_sample = nullptr;
			SYNCDBG(8, "Freed SDL_mixer streamed sample");
		}
	}

	// Destroy the mixer + tracks and quit SDL3_mixer (safe if never opened).
	ShutDownSDLAudio();
	SYNCDBG(7, "SDL_mixer shutdown complete");

	// Clear OpenAL sources and buffers while context is still current
	g_sources.clear();
	g_banks[0].clear();
	g_banks[1].clear();
	g_custom_bank.clear();  // Clear custom sounds when cleaning up audio
	g_id_redirects.clear(); // Clear raw-ID redirects alongside custom bank
	g_stack_policies.clear(); // Clear stacking policies alongside custom bank
	g_tick_samples_last_tick.clear(); // Clear per-tick stacking gate alongside custom bank
	SYNCDBG(7, "Cleared OpenAL sources and sound banks");

	// Now destroy OpenAL context and device (unique_ptr handles proper cleanup)
	g_openal_context = nullptr;
	g_openal_device = nullptr;
	SYNCDBG(6, "Audio cleanup complete");
}

extern "C" void custom_sound_bank_clear() {
	g_custom_bank.clear();
	g_id_redirects.clear();
	g_stack_policies.clear();
	g_tick_samples_last_tick.clear();
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
		g_custom_bank.erase(g_custom_bank.begin() + (ptrdiff_t)g_custom_bank_watermark, g_custom_bank.end());
	}
	SYNCDBG(7, "Restored sound snapshot: %" PRIuSIZE " redirects, %" PRIuSIZE " stack policies",
		SZCAST(g_id_redirects.size()), SZCAST(g_stack_policies.size()));
}

extern "C" void SetSoundMasterVolume(SoundVolume volume) {
	try {
		// Set OpenAL listener gain to maximum so we can split up the mentor speech volume slider from the sound effects volume slider
		alListenerf(AL_GAIN, 1.0f);
		const auto errcode = alGetError();
		if (errcode != AL_NO_ERROR) {
			throw openal_error("Cannot set master volume", errcode);
		}
		g_master_volume = volume;
	} catch (const std::exception & e) {
		ERRORLOG("%s", e.what());
	}
}

extern "C" void set_music_volume(SoundVolume value) {
	g_music_volume = value;
	SetRedbookVolume(value);
	// SDL3_mixer uses a per-track linear gain (0.0..1.0) rather than 0..128.
	if (g_music_track != nullptr) {
		MIX_SetTrackGain(g_music_track, float(value) / FULL_LOUDNESS);
	}
}

extern "C" TbBool play_music(const char * fname) {
	std::lock_guard<std::mutex> guard(g_mix_mutex);
	if (g_current_music_fname == fname) {
		return true;
	}
    game.music_track = -1;
	// Guard against fname aliasing game.music_fname itself — snprintf with overlapping
	// src/dest is undefined behaviour.
	if (fname != game.music_fname) {
		snprintf(game.music_fname, sizeof(game.music_fname), "%s", fname);
	}
	if (!g_mixer || !g_music_track) {
		return false;
	}
	// SDL3_mixer: load into a MIX_Audio and bind it to the persistent music track.
	MIX_Audio* new_audio = MIX_LoadAudio(g_mixer, game.music_fname, false);
	if (!new_audio) {
		WARNLOG("Cannot load music from %s: %s", game.music_fname, SDL_GetError());
		return false;
	}
	// MIX_SetTrackAudio replaces any currently-bound audio; the old audio is no
	// longer referenced by the track afterwards, so it is safe to destroy.
	MIX_SetTrackAudio(g_music_track, new_audio);
	MIX_Audio* old_audio = std::exchange(g_music_audio, new_audio);
	if (old_audio) {
		MIX_DestroyAudio(old_audio);
	}
	if (!MIX_PlayTrack(g_music_track, 0)) {
		WARNLOG("Cannot play music from %s: %s", game.music_fname, SDL_GetError());
		return false;
	}
	MIX_SetTrackLoops(g_music_track, -1); // loop forever (was Mix_PlayMusic(music, -1))
	g_current_music_fname = fname;
	g_current_music_track = 0;
	return true;
}

static const char * find_music_file_for_mod_list(short fgroup, const char * fname, const struct ModConfigItem *mod_items, long mod_cnt)
{
    if (fgroup != FGrp_CmpgMedia && fgroup != FGrp_Music)
        return NULL;

    // Since the path for FGrp_CmpgMedia is configurable/dynamic, the mod's designer cannot obtain it in advance.
    // it would make more sense to force-unify FGrp_CmpgMedia and FGrp_Music into FGrp_Music.
    fgroup = FGrp_Music;

    // Note that this is the reverse mods direction
    for (long i=mod_cnt-1; i>=0; i--)
    {
        const struct ModConfigItem *mod_item = mod_items + i;
        if (mod_item->state.mod_dir == 0)
            continue;

        if (mod_item->state.music == 0)
            continue;

        char mod_dir[256] = {0};
        sprintf(mod_dir, "%s/%s", MODS_DIR_NAME, mod_item->name);

        const char *fpath = prepare_file_path_mod(mod_dir, fgroup, fname);
        if (fpath[0] != 0 && LbFileExists(fpath))
            return fpath;
    }

    return NULL;
}

extern "C" TbBool play_music_fgroup(short fgroup, const char * fname) {
    const char * fpath = NULL;

    // Note that this is the reverse mods direction
    if (fpath == NULL && mods_conf.after_map_cnt > 0)
    {
        fpath = find_music_file_for_mod_list(fgroup, fname, mods_conf.after_map_item, mods_conf.after_map_cnt);
    }
    if (fpath == NULL && mods_conf.after_campaign_cnt > 0)
    {
        fpath = find_music_file_for_mod_list(fgroup, fname, mods_conf.after_campaign_item, mods_conf.after_campaign_cnt);
    }
    if (fpath == NULL && mods_conf.after_base_cnt > 0)
    {
        fpath = find_music_file_for_mod_list(fgroup, fname, mods_conf.after_base_item, mods_conf.after_base_cnt);
    }

    if (fpath == NULL)
        fpath = prepare_file_fmtpath(fgroup, "%s", fname);

    return play_music(fpath);
}

// Music container extensions, in order of preference. Files of other extensions
// (and non-audio files like MusicReadme.txt) are ignored, so they can't shift
// the track-to-file mapping.
// The preferred option is to preserve 100% of the original audio quality while
// using as little storage space as possible, that's why FLAC is first candidate.
static const char *const music_file_extensions[] = {
	".flac", ".wav", ".ogg", ".mp3"
};

static int music_extension_priority(const char *filename) {
	const char *ext = strrchr(filename, '.');
	if (ext == NULL) {
		return -1;
	}
	for (size_t i = 0; i < sizeof(music_file_extensions) / sizeof(music_file_extensions[0]); i++) {
		if (strcasecmp(ext, music_file_extensions[i]) == 0) {
			return (int)i;
		}
	}
	return -1;
}

// Resolve which music file plays for a given redbook track number
static TbBool resolve_track_music_path(int track, char *dst, int dst_size) {
	if (track < 2) {
		return false;
	}
	const int wanted = track - 2; // 0-based position within the chosen format's files

	char filespec[2048];
	prepare_file_path_buf(filespec, sizeof(filespec), FGrp_Music, "*");
	if (filespec[0] == '\0') {
		return false;
	}

	struct TbFileEntry fe;
	struct TbFileFind *ff = LbFileFindFirst(filespec, &fe);
	if (ff == NULL) {
		return false;
	}

	std::vector<std::pair<int, std::string>> files; // (priority rank, filename)
	int best_priority = -1;
	do {
		const int prio = music_extension_priority(fe.Filename);
		if (prio < 0) {
			continue; // not a recognized music file
		}
		files.emplace_back(prio, fe.Filename);
		if (best_priority < 0 || prio < best_priority) {
			best_priority = prio;
		}
	} while (LbFileFindNext(ff, &fe) >= 0);
	LbFileFindEnd(ff);

	if (best_priority < 0) {
		return false;
	}

	// Map the track within the winning format's files only, keeping sorted order
	int index = 0;
	for (const auto & f : files) {
		if (f.first != best_priority) {
			continue;
		}
		if (index == wanted) {
			prepare_file_path_buf(dst, dst_size, FGrp_Music, f.second.c_str());
			return (dst[0] != '\0');
		}
		index++;
	}
	return false;
}

extern "C" TbBool play_music_track(int track) {
	game.music_track = track;
	memset(game.music_fname, 0, sizeof(game.music_fname));
	if (game.music_track == 0) {
		stop_music(true);
		return true;
	} else if (features_enabled & Ft_NoCdMusic) {
		// play_music() itself skips restarting if this exact resolved file is
		// already the one actually playing (e.g. reloading a save for the same level).
		char fpath[2048];
		if (!resolve_track_music_path(track, fpath, sizeof(fpath))) {
			WARNLOG("No music file found for track %d in the music folder", track);
			return false;
		}
		LbJustLog("Playing track %d: %s\n", track, fpath);
		return play_music(fpath);
	} else {
		if (track == g_current_music_track) {
			// Already playing this exact numbered track — skip restarting it.
			return true;
		}
		if (PlayRedbookTrack(track)) {
			g_current_music_track = track;
			g_current_music_fname.clear();
			return true;
		} else {
			WARNLOG("Cannot play track %d", game.music_track);
			return false;
		}
	}
}

extern "C" void pause_music() {
	if (features_enabled & Ft_NoCdMusic) {
		if (g_music_track) MIX_PauseTrack(g_music_track);
	} else {
		PauseRedbookTrack();
	}
}

extern "C" void resume_music() {
	if (features_enabled & Ft_NoCdMusic) {
		if (g_music_track) MIX_ResumeTrack(g_music_track);
	} else {
		ResumeRedbookTrack();
	}
}

extern "C" void stop_music(TbBool fade_out) {
	game.music_track = 0;
	memset(game.music_fname, 0, sizeof(game.music_fname));
	g_current_music_track = 0;
	g_current_music_fname.clear();
	if (features_enabled & Ft_NoCdMusic) {
		if (g_music_track) {
			if (fade_out) {
				// SDL3_mixer expresses fades in sample frames, not milliseconds.
				Sint64 fade_frames = MIX_TrackMSToFrames(g_music_track, 1000);
				MIX_StopTrack(g_music_track, fade_frames);
			} else {
				MIX_StopTrack(g_music_track, 0);
			}
		}
	} else {
		StopRedbookTrack();
	}
}

extern "C" TbBool GetSoundInstalled() {
	return g_openal_device && g_openal_context;
}

// This function gets called every tick, both during active gameplay and while navigating
// the frontend/main menu, this should probably be buffer based and not tick based..
extern "C" void MonitorStreamedSoundTrack() {
	++g_audio_tick_counter;
	for (auto & source : g_sources) {
		try {
			if (source.emit_id > 0 && !source.is_playing()) {
				const SoundSmplTblID finished_smptbl_id = source.smptbl_id;
				source.emit_id = 0;
				source.smptbl_id = 0;
				// If this sample uses Duck-mode stacking, the remaining active instances
				// (if any) should return to a louder gain now that one has ended.
				if (get_stack_policy(finished_smptbl_id).mode == SStack_Duck) {
					apply_duck_gain(finished_smptbl_id);
				}
			}
		} catch (const std::exception & e) {
			ERRORLOG("%s", e.what());
		}
	}
}

extern "C" void * GetSoundDriver() {
	// This just needs to return any non-null pointer. FMV library appears to have standalone audio
	static int dummy = 0;
	return &dummy;
}

extern "C" void StopAllSamples() {
	for (auto & source : g_sources) {
		try {
			source.stop();
		} catch (const std::exception & e) {
			ERRORLOG("%s", e.what());
		}
	}
}

extern "C" TbBool InitAudio(const SoundSettings * settings) {
	try {
		if (game.easter_eggs_enabled == true) {
			TbDate date;
			LbDate(&date);
			g_bb_king_mode |= ((date.Day == 1) && (date.Month == 2));
		}
		if (SoundDisabled) {
			WARNLOG("Sound is disabled, skipping OpenAL initialization");
			return false;
		}
		if (g_openal_device || g_openal_context) {
			WARNLOG("OpenAL already initialized");
			return true;
		}
		print_device_info();
		ALCdevice_ptr device(alcOpenDevice(nullptr));
		if (!device) {
			throw openal_error("Cannot open default audio device");
		}
		ALCcontext_ptr context(alcCreateContext(device.get(), nullptr));
		if (!context) {
			throw openal_error("Cannot create context");
		} else if (!alcMakeContextCurrent(context.get())) {
			throw openal_error("Cannot make context current");
		}
		g_sources.resize(settings->max_number_of_samples);
		for (size_t i = 0; i < g_sources.size(); ++i) {
			g_sources[i].mss_id = i + 1;
		}
		load_sound_banks();
		g_openal_device = std::move(device);
		g_openal_context = std::move(context);
		return true;
	} catch (const std::exception & e) {
		ERRORLOG("%s", e.what());
	}
	SoundDisabled = true;
	return false;
}

extern "C" TbBool IsSamplePlaying(SoundMilesID mss_id) {
	try {
		for (const auto & source : g_sources) {
			if (source.mss_id == mss_id) {
				return source.is_playing();
			}
		}
	} catch (const std::exception & e) {
		ERRORLOG("%s", e.what());
	}
	return false;
}

extern "C" SoundVolume GetCurrentSoundMasterVolume() {
	return g_master_volume;
}

extern "C" void SetSampleVolume(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundVolume volume) {
	for (auto & source : g_sources) {
		if (source.emit_id == emit_id && source.smptbl_id == smptbl_id) {
			try {
				source.gain(volume);
			} catch (const std::exception & e) {
				ERRORLOG("%s", e.what());
			}
		}
	}
}

extern "C" void SetSamplePan(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundPan pan) {
	for (auto & source : g_sources) {
		if (source.emit_id == emit_id && source.smptbl_id == smptbl_id) {
			try {
				source.pan(pan);
			} catch (const std::exception & e) {
				ERRORLOG("%s", e.what());
			}
		}
	}
}

extern "C" void SetSamplePitch(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundPitch pitch) {
	for (auto & source : g_sources) {
		if (source.emit_id == emit_id && source.smptbl_id == smptbl_id) {
			try {
				if (source.flags & bb_king_mode) {
					return; // ben enjoyed dofi's stream so much I made random pitch an easter egg
				} else {
					source.pitch(pitch);
				}
			} catch (const std::exception & e) {
				ERRORLOG("%s", e.what());
			}
		}
	}
}

extern "C" SoundMilesID play_sample(
	SoundEmitterID emit_id,
	SoundSmplTblID smptbl_id,
	SoundVolume volume,
	SoundPan pan,
	SoundPitch pitch,
	char repeats, // possible values: -1, 0
	unsigned char ctype // possible values: 2, 3
) {
	if (emit_id <= 0) {
		ERRORLOG("Can't play sample %d, invalid emitter ID", smptbl_id);
		return 0;
	}
	// Apply raw-ID redirect before bank dispatch (only for effect-bank IDs)
	if (smptbl_id > 0 && smptbl_id < g_speech_offset) {
		auto redir = g_id_redirects.find(smptbl_id);
		if (redir != g_id_redirects.end()) {
			smptbl_id = redir->second;
		}
	}
	// Resolve sample data from unified ID space
	const openal_buffer * buf = nullptr;
	if (smptbl_id >= g_custom_offset) {
		const SoundSmplTblID idx = smptbl_id - g_custom_offset;
		if (idx < 0 || idx >= (SoundSmplTblID)g_custom_bank.size()) {
			ERRORLOG("Can't play custom sample %d, out of range", smptbl_id);
			return 0;
		}
		buf = &g_custom_bank[idx].buffer;
	} else if (smptbl_id >= g_speech_offset) {
		const SoundSmplTblID idx = smptbl_id - g_speech_offset;
		if (idx <= 0 || idx >= (SoundSmplTblID)g_banks[1].size()) {
			ERRORLOG("Can't play speech sample %d, out of range", smptbl_id);
			return 0;
		}
		buf = &g_banks[1][idx].buffer;
	} else {
		if (smptbl_id <= 0 || smptbl_id >= (SoundSmplTblID)g_banks[0].size()) {
			if (smptbl_id != 0) {
				ERRORLOG("Can't play effect sample %d, out of range", smptbl_id);
			}
			return 0;
		}
		buf = &g_banks[0][smptbl_id].buffer;
	}
	try {
		// Look up the stacking policy once — used by both the restart-in-place path below
		// and the new-voice-allocation path further down.
		const auto stack_policy_it = g_stack_policies.find(smptbl_id);
		const bool has_explicit_stack_policy = (stack_policy_it != g_stack_policies.end());
		SoundStackPolicy stack_policy{};
		if (has_explicit_stack_policy) {
			stack_policy = stack_policy_it->second;
		}

		// ctype 2/3: if this emitter is already playing the same sample, restart it in-place
		// rather than allocating a new source (mirrors MSS single-voice-per-slot behaviour and
		// prevents sounds from stacking — e.g. hailstorm projectiles all hitting the same target).
		if (ctype == 2 || ctype == 3) {
			for (auto & source : g_sources) {
				if (source.emit_id == emit_id && source.smptbl_id == smptbl_id) {
					source.stop();
					source.base_gain = volume;
					source.gain(volume);
					source.pan(pan);
					source.repeat(repeats == -1);
					source.pitch(pitch);
					source.play(*buf);
					if (has_explicit_stack_policy && stack_policy.mode == SStack_Duck) {
						// This source may be part of a Duck-mode group with other concurrently
						// playing instances (from other emitters) — rescale it (and them) back
						// down instead of leaving it at the full, un-ducked volume just set above.
						apply_duck_gain(smptbl_id);
					}
					return source.mss_id;
				}
			}
		}
		// Cross-emitter stacking policy: caps how many different emitters may play the
		// same sample concurrently (or ducks their combined volume). Only applies when
		// allocating a genuinely new voice, not on the same-emitter restart-in-place above.
		if (!has_explicit_stack_policy) {
			// No explicit STACK= policy: reproduce the OG behaviour exactly — this sample may
			// only start once per tick, regardless of which emitter triggers it.
			const auto tick_it = g_tick_samples_last_tick.find(smptbl_id);
			if (tick_it != g_tick_samples_last_tick.end() && tick_it->second == g_audio_tick_counter) {
				return 0; // dropped: already triggered this tick
			}
			g_tick_samples_last_tick[smptbl_id] = g_audio_tick_counter;
		} else {
			if (stack_policy.max_instances > 0) {
				int active_count = 0;
				for (const auto & source : g_sources) {
					if (source.emit_id != 0 && source.smptbl_id == smptbl_id) {
						++active_count;
					}
				}
				if (active_count >= stack_policy.max_instances) {
					return 0; // dropped: at this sample's concurrency cap
				}
			}
		}
		for (auto & source : g_sources) {
			if (source.emit_id == 0) {
				source.base_gain = volume;
				source.gain(volume);
				source.pan(pan);
				source.repeat(repeats == -1);
				if (g_bb_king_mode) {
					// ben enjoyed dofi's stream so much I made random pitch an easter egg
                    if (SOUND_RANDOM(10000) <= 3) { // ~0.03% of the time
						source.flags |= bb_king_mode;
						source.pitch((NORMAL_PITCH / 2) + SOUND_RANDOM(NORMAL_PITCH));
					} else {
						source.flags &= ~bb_king_mode;
						source.pitch(pitch);
					}
				} else {
					source.pitch(pitch);
				}
				source.play(*buf);
				source.emit_id = emit_id;
				source.smptbl_id = smptbl_id;
				if (has_explicit_stack_policy && stack_policy.mode == SStack_Duck) {
					apply_duck_gain(smptbl_id);
				}
				return source.mss_id;
			}
		}
		if (game.frame_skip < 2) {
			ERRORLOG("Can't play sample %d, too many samples playing at once", smptbl_id);
		}
		return 0;
	} catch (const std::exception & e) {
		ERRORLOG("%s", e.what());
	}
	return 0;
}

extern "C" void stop_sample(SoundEmitterID emit_id, SoundSmplTblID smptbl_id) {
	for (auto & source : g_sources) {
		if (emit_id == source.emit_id && smptbl_id == source.smptbl_id) {
			try {
				source.stop();
				source.emit_id = 0;
				source.smptbl_id = 0;
			} catch (const std::exception & e) {
				ERRORLOG("%s", e.what());
			}
		}
	}
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

extern "C" int InitialiseSDLAudio()
{
	if (!SDL_Init(SDL_INIT_AUDIO)) {
		ERRORLOG("Unable to initialise SDL audio subsystem: %s", SDL_GetError());
		return 0;
	}
	// SDL3_mixer: MIX_Init() is reference-counted and takes no format flags
	// (decoders are discovered automatically).
	if (!MIX_Init()) {
		ERRORLOG("Could not initialise SDL3_mixer: %s", SDL_GetError());
		return 0;
	}
	SDL_AudioSpec spec;
	spec.format   = SDL_AUDIO_S16;
	spec.channels = 2;
	spec.freq     = 44100;
	g_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
	if (!g_mixer) {
		ERRORLOG("Could not open audio device for SDL mixer: %s", SDL_GetError());
		MIX_Quit();
		return 0;
	}
	// One persistent track each for music and streamed speech (replaces the
	// SDL2 reserved-channel model).
	g_music_track = MIX_CreateTrack(g_mixer);
	g_speech_track = MIX_CreateTrack(g_mixer);
	if (!g_music_track || !g_speech_track) {
		ERRORLOG("Could not create SDL mixer tracks: %s", SDL_GetError());
		MIX_DestroyMixer(g_mixer);
		g_mixer = nullptr;
		g_music_track = nullptr;
		g_speech_track = nullptr;
		MIX_Quit();
		return 0;
	}
	return 1;
}

extern "C" void ShutDownSDLAudio()
{
	g_music_track = nullptr;
	g_speech_track = nullptr;
	if (g_mixer) {
		MIX_DestroyMixer(g_mixer); // also destroys all tracks created for this mixer
		g_mixer = nullptr;
	}
	MIX_Quit();
}

extern "C" TbBool play_streamed_sample(const char* fname, SoundVolume volume)
{
	if (SoundDisabled || fname == nullptr || strlen(fname) == 0) {
		return false;
	}
	if (!g_mixer || !g_speech_track) {
		return false;
	}
	// Predecode speech so short samples start with no I/O latency.
	MIX_Audio* sample = MIX_LoadAudio(g_mixer, fname, true);
	if (sample == nullptr) {
		ERRORLOG("Cannot load \"%s\": %s", fname, SDL_GetError());
		return false;
	}
	MIX_SetTrackAudio(g_speech_track, sample);
	// SDL3_mixer gain is linear 0.0..1.0 (SoundVolume ranges 0..FULL_LOUDNESS).
	MIX_SetTrackGain(g_speech_track, float(volume) / FULL_LOUDNESS);
	if (!MIX_PlayTrack(g_speech_track, 0)) {
		MIX_DestroyAudio(sample);
		ERRORLOG("Cannot play \"%s\": %s", fname, SDL_GetError());
		return false;
	}
	std::lock_guard<std::mutex> guard(g_mix_mutex);
	const auto old_sample = std::exchange(g_streamed_sample, sample);
	if (old_sample) {
		MIX_DestroyAudio(old_sample);
	}
	return true;
}

extern "C" void stop_streamed_samples()
{
	if (g_speech_track) MIX_StopTrack(g_speech_track, 0);
	std::lock_guard<std::mutex> guard(g_mix_mutex);
	const auto old_sample = std::exchange(g_streamed_sample, (MIX_Audio*)nullptr);
	if (old_sample) {
		MIX_DestroyAudio(old_sample);
	}
}

extern "C" void set_streamed_sample_volume(SoundVolume volume) {
	// SDL3_mixer gain is linear 0.0..1.0 (SoundVolume ranges 0..FULL_LOUDNESS).
	if (g_speech_track) MIX_SetTrackGain(g_speech_track, float(volume) / FULL_LOUDNESS);
}

// Replaces the SDL2 Mix_Playing(MIX_SPEECH_CHANNEL) query; keeps the MIX_ API
// contained to this translation unit.
extern "C" TbBool is_streamed_sample_playing(void) {
	return (g_speech_track != nullptr) && MIX_TrackPlaying(g_speech_track);
}

extern "C" void toggle_bbking_mode() {
	g_bb_king_mode = !g_bb_king_mode;
}

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
		ERRORLOG("Cannot decode MP3 data from %s", filepath);
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
//   - OGG / FLAC     : Mix_LoadWAV_RW (SDL_mixer; require MIX_INIT_OGG/MIX_INIT_FLAC).
//                       Converts to the mixer device's opened channel count/rate, so a
//                       mono OGG/FLAC source is force-upmixed to stereo — see the caveat
//                       comment below.
//   - Plain MP3      : dr_mp3 single-header decoder (compiled in, no external DLLs)
//   - BMU V1.0       : 8-byte wrapper used by some campaigns; contains a plain MP3
//                       stream after the header — stripped and decoded via dr_mp3.
static TbBool decode_audio_buffer_and_store(const char* logical_name, int sample_id,
	const uint8_t* data, size_t size)
{
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
	// This preserves the file's own channel count (mono stays mono). Otherwise this'll go to 
    // SDL's Mix_LoadWAV_RW path below, which converts to the mixer's output format (44100 Hz / Sint16 / stereo)
    // and blow your ears off if the source is mono. 
	if (size >= 12 && memcmp(data, "RIFF", 4) == 0 && memcmp(data + 8, "WAVE", 4) == 0) {
		try {
			std::string buf(reinterpret_cast<const char*>(data), size);
			std::istringstream stream(buf, std::ios::binary);
			g_custom_bank.emplace_back(logical_name, sample_id, wave_file(stream));
			return true;
		} catch (const std::exception & e) {
			ERRORLOG("Cannot parse WAV audio %s: %s", logical_name, e.what());
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
		ERRORLOG("Cannot create IOStream for %s: %s", logical_name, SDL_GetError());
		return false;
	}

	// closeio=true: the decoder takes ownership of the stream.
	MIX_AudioDecoder* decoder = MIX_CreateAudioDecoder_IO(io, true, 0);
	if (!decoder) {
		ERRORLOG("Cannot decode audio file %s: %s", logical_name, SDL_GetError());
		return false;
	}

	const SDL_AudioSpec out_spec = { SDL_AUDIO_S16, 2, 44100 };
	const ALenum al_fmt = AL_FORMAT_STEREO16;

	try {
		std::vector<uint8_t> pcm;
		uint8_t buffer[16384];
		for (;;) {
			int got = MIX_DecodeAudio(decoder, buffer, sizeof(buffer), &out_spec);
			if (got <= 0) break; // 0 == end of stream, <0 == error
			pcm.insert(pcm.end(), buffer, buffer + got);
		}
		MIX_DestroyAudioDecoder(decoder);
		g_custom_bank.emplace_back(logical_name, sample_id, pcm, al_fmt, out_spec.freq);
	} catch (...) {
		MIX_DestroyAudioDecoder(decoder);
		ERRORLOG("Out of memory buffering audio %s", logical_name);
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
			ERRORLOG("Cannot open audio file %s", filepath);
			return false;
		}
		const auto sz = f.tellg();
		if (sz <= 0) {
			ERRORLOG("Empty audio file %s", filepath);
			return false;
		}
		file_data.resize((size_t)sz);
		f.seekg(0);
		if (!f.read(reinterpret_cast<char*>(file_data.data()), sz)) {
			ERRORLOG("Cannot read audio file %s", filepath);
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
		ERRORLOG("Empty audio buffer for %s", logical_name);
		return false;
	}
	return decode_audio_buffer_and_store(logical_name, sample_id, data, size);
}
