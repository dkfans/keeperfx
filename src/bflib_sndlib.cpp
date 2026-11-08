#include "pre_inc.h"
#include "config_keeperfx.h"
#include "cdrom.h"
#include "bflib_sndlib.h"
#include "bflib_datetm.h"
#include "game_legacy.h"
#include "bflib_sound.h"
#include "bflib_fileio.h"
#include "kfx/profiling/KfxProfiling.h"
#include "kfx/audio/AudioCallback.h"
#include "kfx/audio/AudioLog.h"
#include "kfx/audio/AudioStream.h"
#include "kfx/audio/VoiceMirror.h"
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

#include "keeperfx.hpp"
#include "post_inc.h"

namespace {

SoundVolume g_master_volume = 0;
SoundVolume g_music_volume = 0;
// Sound effects: game-thread voice bookkeeping and the audio-thread OpenAL renderer.
kfx::audio::VoiceMirror g_voice_mirror;
kfx::audio::AudioCallback g_audio_callback;
kfx::audio::CommandRing g_command_ring;
kfx::audio::EventRing g_event_ring;
// InitAudio holds its own reference on the SDL audio subsystem for the SDL3 stream.
bool g_sdl_audio_subsystem_held = false;
// SDL3_mixer replaces the SDL2 Mix_Music/Mix_Chunk + channel model with a
// mixer + per-purpose tracks. Music and speech each get their own track.
MIX_Mixer* g_mixer = nullptr;
MIX_Track* g_music_track = nullptr;
MIX_Track* g_speech_track = nullptr;

#ifdef AL_EXT_debug
LPALDEBUGMESSAGECALLBACKEXT g_al_debug_message_callback = nullptr;
#endif

bool g_bb_king_mode = false;

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

struct sound_sample {

	std::string name;
	SoundSFXID sfx_id;
	openal_buffer buffer;

	sound_sample(const char * _name, SoundSFXID _sfx_id, const wave_file & wav) {
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
	void buffer_msadpcm(const wave_file & wav) {
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

// For when buffers can't be deleted safely: a later InitAudio must not delete reused ids.
void forget_bank_buffers(std::vector<sound_sample> & bank) {
	for (auto & sample : bank) {
		sample.buffer.id = 0;
	}
	bank.clear();
}

// Closes the SDL3 stream and the loopback device, then drops InitAudio's SDL audio reference.
void release_audio_output() {
	g_audio_callback.shutdown();
	kfx::audio::audio_stream_unbind();
	kfx::audio::Command command;
	while (g_command_ring.pop(command)) {} // the callback is gone, so nothing else consumes these
	kfx::audio::VoiceFinishedEvent event;
	while (g_event_ring.pop(event)) {} // would otherwise match generations after a re-init
	g_voice_mirror.init(0);
	if (g_sdl_audio_subsystem_held) {
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		g_sdl_audio_subsystem_held = false;
	}
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

void print_device_info() {
	const char * devices;
	if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT")) {
		devices = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);
	} else if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT")) {
		devices = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
	} else {
		return;
	}
	if (devices == nullptr || devices[0] == 0) {
		AUDIOLOG("Audio devices: none");
		return;
	}
	std::string list = "Audio devices:";
	const char * separator = " ";
	for (auto device = devices; device[0] != 0; device += strlen(device) + 1) {
		list += separator;
		list += device;
		separator = ", ";
	}
	AUDIOLOG("%s", list.c_str());
}

const char * output_mode_str(ALCint mode) {
	switch (mode) {
		case ALC_ANY_SOFT: return "any";
		case ALC_STEREO_BASIC_SOFT: return "stereo (basic)";
		case ALC_STEREO_UHJ_SOFT: return "stereo (UHJ)";
		case ALC_STEREO_HRTF_SOFT: return "stereo (HRTF)";
		case ALC_SURROUND_5_1_SOFT: return "5.1 surround";
		case ALC_SURROUND_6_1_SOFT: return "6.1 surround";
		case ALC_SURROUND_7_1_SOFT: return "7.1 surround";
	}
	return "unknown";
}

const char * hrtf_status_str(ALCint status) {
	switch (status) {
		case ALC_HRTF_DISABLED_SOFT: return "disabled";
		case ALC_HRTF_ENABLED_SOFT: return "enabled";
		case ALC_HRTF_DENIED_SOFT: return "denied (unsupported by device)";
		case ALC_HRTF_REQUIRED_SOFT: return "required by device";
		case ALC_HRTF_HEADPHONES_DETECTED_SOFT: return "enabled (headphones detected)";
		case ALC_HRTF_UNSUPPORTED_FORMAT_SOFT: return "unsupported format";
	}
	return "unknown";
}

void log_openal_status(ALCdevice * device) {
	AUDIOLOG("OpenAL version: %s, vendor: %s, renderer: %s",
		alGetString(AL_VERSION), alGetString(AL_VENDOR), alGetString(AL_RENDERER));
	const char * device_name = alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT")
		? alcGetString(device, ALC_ALL_DEVICES_SPECIFIER)
		: alcGetString(device, ALC_DEVICE_SPECIFIER);
	AUDIOLOG("OpenAL device in use: %s", (device_name != nullptr) ? device_name : "unknown");
	if (alcIsExtensionPresent(device, "ALC_SOFT_output_mode")) {
		ALCint mode = 0;
		alcGetIntegerv(device, ALC_OUTPUT_MODE_SOFT, 1, &mode);
		AUDIOLOG("OpenAL output mode: %s", output_mode_str(mode));
	}
	if (alcIsExtensionPresent(device, "ALC_SOFT_HRTF")) {
		ALCint status = 0;
		alcGetIntegerv(device, ALC_HRTF_STATUS_SOFT, 1, &status);
		AUDIOLOG("OpenAL HRTF status: %s", hrtf_status_str(status));
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

	// Stop every voice so no source holds a buffer, then delete while the loopback context is current.
	bool buffers_safe = true;
	if (g_audio_callback.initialized()) {
		buffers_safe = g_audio_callback.flush();
		if (buffers_safe) {
			g_voice_mirror.stop_all(g_command_ring);
			buffers_safe = g_audio_callback.flush();
		}
	}
	if (buffers_safe) {
		g_banks[0].clear();
		g_banks[1].clear();
		g_custom_bank.clear();  // Clear custom sounds when cleaning up audio
	} else {
		AUDIOERR("Audio flush failed, leaving sound buffers to the device close");
		forget_bank_buffers(g_banks[0]);
		forget_bank_buffers(g_banks[1]);
		forget_bank_buffers(g_custom_bank);
	}
	g_id_redirects.clear(); // Clear raw-ID redirects alongside custom bank
	g_stack_policies.clear(); // Clear stacking policies alongside custom bank
	g_voice_mirror.clear_tick_gate(); // Clear per-tick stacking gate alongside custom bank
	SYNCDBG(7, "Stopped voices and cleared sound banks");

	// Now destroy the SDL3 stream, the sources and the loopback context/device
#ifdef AL_EXT_debug
	g_al_debug_message_callback = nullptr;
#endif
	release_audio_output();
	SYNCDBG(6, "Audio cleanup complete");
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
		AUDIOERR("%s", e.what());
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
	KFX_ZONE_COLOR("play_music", KFX_COLOR_AUDIO);
	// Stream from disk: MIX_LoadAudio would read the whole file on the game thread.
	// On failure the stream is closed and the current music keeps playing.
	SDL_IOStream* io = SDL_IOFromFile(game.music_fname, "rb");
	if (!io || !MIX_SetTrackIOStream(g_music_track, io, true)) {
		AUDIOWARN("Cannot load music from %s: %s", game.music_fname, SDL_GetError());
		return false;
	}
	if (!MIX_PlayTrack(g_music_track, 0)) {
		AUDIOWARN("Cannot play music from %s: %s", game.music_fname, SDL_GetError());
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
	KFX_ZONE_COLOR("play_music_track", KFX_COLOR_AUDIO);
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
			AUDIOWARN("No music file found for track %d in the music folder", track);
			return false;
		}
		AUDIOLOG("Playing track %d: %s", track, fpath);
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
			AUDIOWARN("Cannot play track %d", game.music_track);
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
	return g_audio_callback.initialized();
}

// This function gets called every tick, both during active gameplay and while navigating
// the frontend/main menu, this should probably be buffer based and not tick based..
extern "C" void MonitorStreamedSoundTrack() {
	KFX_ZONE_COLOR("MonitorStreamedSoundTrack", KFX_COLOR_AUDIO);
	g_voice_mirror.advance_tick(); // must precede process_events(): its gates compare against the new tick
	g_voice_mirror.process_events(g_event_ring, g_command_ring);
	[[maybe_unused]] const size_t active_voices = g_voice_mirror.active_count();
	const uint32_t voices_stolen = g_voice_mirror.take_voices_stolen();
	const uint32_t drops_tick_gate = g_voice_mirror.take_drops_tick_gate();
	const uint32_t drops_stack_cap = g_voice_mirror.take_drops_stack_cap();
	const uint32_t drops_no_voice = g_voice_mirror.take_drops_no_voice();
	const uint32_t commands_dropped = g_voice_mirror.take_commands_dropped();
	const bool events_overflowed = g_audio_callback.take_event_ring_overflowed();
	KFX_PLOT("Audio voices", active_voices);
	KFX_PLOT("Audio voices stolen", voices_stolen);
	KFX_PLOT("Audio drops: tick gate", drops_tick_gate);
	KFX_PLOT("Audio drops: stack cap", drops_stack_cap);
	KFX_PLOT("Audio drops: no voice", drops_no_voice);
	KFX_PLOT("Audio drops: command ring full", commands_dropped);
	KFX_PLOT("Audio event ring overflowed", events_overflowed ? 1 : 0);
	KFX_PLOT("Audio drops: S3D priority", S3DDropsPriority);
	KFX_PLOT("Audio drops: S3D eviction", S3DDropsEviction);
	KFX_PLOT("Audio drops: S3D volume zero", S3DDropsVolumeZero);
	KFX_PLOT("Audio drops: S3D out of range", S3DDropsOutOfRange);
	if (voices_stolen || drops_tick_gate || drops_stack_cap || drops_no_voice || commands_dropped || events_overflowed
	 || S3DDropsPriority || S3DDropsEviction || S3DDropsVolumeZero || S3DDropsOutOfRange) {
		SYNCDBG(7, "Audio sample drops this tick: stolen %u, tick-gate %u, stack-cap %u, no-voice %u, "
			"command-ring-full %u, event-ring-overflow %d, "
			"S3D priority %lu, S3D eviction %lu, S3D volume-zero %lu, S3D out-of-range %lu",
			voices_stolen, drops_tick_gate, drops_stack_cap, drops_no_voice,
			commands_dropped, events_overflowed ? 1 : 0,
			S3DDropsPriority, S3DDropsEviction, S3DDropsVolumeZero, S3DDropsOutOfRange);
	}
	S3DDropsPriority = 0;
	S3DDropsEviction = 0;
	S3DDropsVolumeZero = 0;
	S3DDropsOutOfRange = 0;
}

extern "C" void * GetSoundDriver() {
	// This just needs to return any non-null pointer. FMV library appears to have standalone audio
	static int dummy = 0;
	return &dummy;
}

extern "C" void StopAllSamples() {
	g_voice_mirror.stop_all(g_command_ring);
}

#ifdef AL_EXT_debug
void AL_APIENTRY on_al_debug_message(ALenum source, ALenum type, ALuint id, ALenum severity, ALsizei length, const ALchar * message, void * userParam) noexcept {
	(void)source; (void)id; (void)length; (void)userParam;
	if (type == AL_DEBUG_TYPE_ERROR_EXT || severity == AL_DEBUG_SEVERITY_HIGH_EXT) {
		AUDIOERR("OpenAL: %s", message);
	} else {
		AUDIOWARN("OpenAL: %s", message);
	}
}
#endif

extern "C" TbBool InitAudio(const SoundSettings * settings) {
	try {
		if (start_params.easter_egg == true) {
			TbDate date;
			LbDate(&date);
			g_bb_king_mode |= ((date.Day == 1) && (date.Month == 2));
		}
		if (SoundDisabled) {
			AUDIOWARN("Sound is disabled, skipping OpenAL initialization");
			return false;
		}
		if (GetSoundInstalled()) {
			AUDIOWARN("OpenAL already initialized");
			return true;
		}
		print_device_info();
		// sounds.c calls InitAudio before InitialiseSDLAudio, so take our own SDL audio reference.
		if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
			throw std::runtime_error(std::string("Cannot initialise SDL audio subsystem: ") + SDL_GetError());
		}
		g_sdl_audio_subsystem_held = true;
		// Makes the loopback context process-current; nothing else may change it.
		if (!g_audio_callback.init(settings->max_number_of_samples, g_command_ring, g_event_ring, &kfx::audio::stream_pool())) {
			throw std::runtime_error("Cannot open audio output");
		}
		kfx::audio::audio_stream_bind(&g_audio_callback);
		AUDIOLOG("OpenAL renders into a loopback device, played through an SDL3 audio stream");
		const char * output_device = g_audio_callback.physical_device_name();
		AUDIOLOG("SDL3 audio output device: %s", (output_device != nullptr) ? output_device : "unknown");
		log_openal_status(g_audio_callback.loopback_device());
#ifdef AL_EXT_debug
		if (alIsExtensionPresent("AL_EXT_debug")) {
			g_al_debug_message_callback = reinterpret_cast<LPALDEBUGMESSAGECALLBACKEXT>(alGetProcAddress("alDebugMessageCallbackEXT"));
			if (g_al_debug_message_callback) {
				g_al_debug_message_callback(on_al_debug_message, nullptr);
				alEnable(AL_DEBUG_OUTPUT_EXT);
			}
		}
#endif
		g_voice_mirror.init(settings->max_number_of_samples);
		g_voice_mirror.set_stack_policy_lookup(lookup_stack_policy);
		g_voice_mirror.set_bb_king_mode(g_bb_king_mode);
		// Must run after g_audio_callback.init(): buffers must live on the loopback context.
		load_sound_banks();
		return true;
	} catch (const std::exception & e) {
		AUDIOERR("%s", e.what());
	}
	// No voice has played yet, so the partly loaded banks hold no attached buffers.
	g_banks[0].clear();
	g_banks[1].clear();
#ifdef AL_EXT_debug
	g_al_debug_message_callback = nullptr;
#endif
	release_audio_output();
	SoundDisabled = true;
	return false;
}

extern "C" TbBool IsSamplePlaying(SoundMilesID mss_id) {
	return g_voice_mirror.is_playing(mss_id);
}

extern "C" SoundVolume GetCurrentSoundMasterVolume() {
	return g_master_volume;
}

extern "C" void SetSampleVolume(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundVolume volume) {
	g_voice_mirror.set_volume(emit_id, smptbl_id, volume, g_command_ring);
}

extern "C" void SetSamplePan(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundPan pan) {
	g_voice_mirror.set_pan(emit_id, smptbl_id, pan, g_command_ring);
}

extern "C" void SetSamplePitch(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundPitch pitch) {
	g_voice_mirror.set_pitch(emit_id, smptbl_id, pitch, g_command_ring);
}

// No-ops: updates are queued for the audio thread.
extern "C" void begin_sample_updates() {
}

extern "C" void end_sample_updates() {
}

extern "C" SoundMilesID play_sample(
	SoundEmitterID emit_id,
	SoundSmplTblID smptbl_id,
	SoundVolume volume,
	SoundPan pan,
	SoundPitch pitch,
	char repeats, // possible values: -1, 0
	unsigned char ctype, // possible values: 2, 3
	long priority // higher = more important; matches bflib_sound.c's find_slot() convention
) {
	if (emit_id <= 0) {
		AUDIOERR("Can't play sample %d, invalid emitter ID", smptbl_id);
		return 0;
	}
	// Only positional (S3D) emitters are spatialised, not ambient loops or speech.
	const bool is_positional = (emit_id != Non3DEmitter) && (emit_id != SpeechEmitter);
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
			AUDIOERR("Can't play custom sample %d, out of range", smptbl_id);
			return 0;
		}
		buf = &g_custom_bank[idx].buffer;
	} else if (smptbl_id >= g_speech_offset) {
		const SoundSmplTblID idx = smptbl_id - g_speech_offset;
		if (idx <= 0 || idx >= (SoundSmplTblID)g_banks[1].size()) {
			AUDIOERR("Can't play speech sample %d, out of range", smptbl_id);
			return 0;
		}
		buf = &g_banks[1][idx].buffer;
	} else {
		if (smptbl_id <= 0 || smptbl_id >= (SoundSmplTblID)g_banks[0].size()) {
			if (smptbl_id != 0) {
				AUDIOERR("Can't play effect sample %d, out of range", smptbl_id);
			}
			return 0;
		}
		buf = &g_banks[0][smptbl_id].buffer;
	}
	try {
		return g_voice_mirror.allocate(emit_id, buf->id, smptbl_id, volume, pan, pitch, repeats, ctype,
		                               priority, is_positional, g_command_ring);
	} catch (const std::exception & e) {
		AUDIOERR("%s", e.what());
	}
	return 0;
}

extern "C" void stop_sample(SoundEmitterID emit_id, SoundSmplTblID smptbl_id) {
	g_voice_mirror.stop(emit_id, smptbl_id, g_command_ring);
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
		AUDIOERR("Unable to initialise SDL audio subsystem: %s", SDL_GetError());
		return 0;
	}
	// SDL3_mixer: MIX_Init() is reference-counted and takes no format flags
	// (decoders are discovered automatically).
	if (!MIX_Init()) {
		AUDIOERR("Could not initialise SDL3_mixer: %s", SDL_GetError());
		return 0;
	}
	SDL_AudioSpec spec;
	spec.format   = SDL_AUDIO_S16;
	spec.channels = 2;
	spec.freq     = 44100;
	g_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
	if (!g_mixer) {
		AUDIOERR("Could not open audio device for SDL mixer: %s", SDL_GetError());
		MIX_Quit();
		return 0;
	}
	// One persistent track each for music and streamed speech (replaces the
	// SDL2 reserved-channel model).
	g_music_track = MIX_CreateTrack(g_mixer);
	g_speech_track = MIX_CreateTrack(g_mixer);
	if (!g_music_track || !g_speech_track) {
		AUDIOERR("Could not create SDL mixer tracks: %s", SDL_GetError());
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
	KFX_ZONE_COLOR("play_streamed_sample", KFX_COLOR_AUDIO);
	// No predecode: decoding happens on the mixer thread, only the file read stays here.
	MIX_Audio* sample = MIX_LoadAudio(g_mixer, fname, false);
	if (sample == nullptr) {
		AUDIOERR("Cannot load \"%s\": %s", fname, SDL_GetError());
		return false;
	}
	MIX_SetTrackAudio(g_speech_track, sample);
	// SDL3_mixer gain is linear 0.0..1.0 (SoundVolume ranges 0..FULL_LOUDNESS).
	MIX_SetTrackGain(g_speech_track, float(volume) / FULL_LOUDNESS);
	if (!MIX_PlayTrack(g_speech_track, 0)) {
		MIX_DestroyAudio(sample);
		AUDIOERR("Cannot play \"%s\": %s", fname, SDL_GetError());
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
	g_voice_mirror.set_bb_king_mode(g_bb_king_mode);
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
