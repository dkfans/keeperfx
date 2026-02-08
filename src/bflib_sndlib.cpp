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
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <fstream>
#include <string>
#include <utility>
#include <array>
#include <deque>
#include <mutex>
#include <atomic>
#include <set>
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
std::atomic<Mix_Music *> g_mix_music;
std::set<uint32_t> g_tick_samples;
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
	SoundBankID bank_id = 0;
	int flags = 0;

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
	, bank_id(std::exchange(other.bank_id, 0)){}

	inline openal_source & operator=(openal_source && other) {
		id = std::exchange(other.id, 0);
		mss_id = std::exchange(other.mss_id, 0);
		emit_id = std::exchange(other.emit_id, 0);
		smptbl_id = std::exchange(other.smptbl_id, 0);
		bank_id = std::exchange(other.bank_id, 0);
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
	wave_file(std::ifstream & stream) {
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
	JUSTLOG("Loaded %d sound samples from %s", (int) buffers.size(), filename);
	return buffers;
}

std::vector<openal_source> g_sources;
std::array<std::vector<sound_sample>, 2> g_banks;

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
}

void print_device_info() {
	if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT")) {
		const auto devices = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);
		JUSTLOG("Available audio devices:");
		for (auto device = devices; device[0] != 0; device += strlen(device)) {
			JUSTLOG("  %s", device);
		}
		const auto default_device = alcGetString(nullptr, ALC_DEFAULT_ALL_DEVICES_SPECIFIER);
		JUSTLOG("Default audio device: %s", default_device);
	} else if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT")) {
		const auto devices = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
		JUSTLOG("Available audio devices:");
		for (auto device = devices; device[0] != 0; device += strlen(device)) {
			JUSTLOG("  %s", device);
		}
		const auto default_device = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
		JUSTLOG("Default audio device: %s", default_device);
	} else {
		// Cannot enumerate devices :(
	}
}

Mix_Chunk * g_streamed_sample = nullptr;
std::mutex g_mix_mutex;

struct queued_sample {
	std::string fname;
	SoundVolume volume;
};

void SDLCALL on_music_finished() {
	// don't grab mutex or we'll deadlock, just free memory
	Mix_FreeMusic(g_mix_music.exchange(nullptr));
}

} // local

extern "C" void FreeAudio() {
	g_sources.clear();
	g_banks[0].clear();
	g_banks[1].clear();
	g_openal_context = nullptr;
	g_openal_device = nullptr;
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
	// convert 0..256 to 0..128
	Mix_VolumeMusic(LbLerp(0, MIX_MAX_VOLUME, float(value) / FULL_LOUDNESS));
}

extern "C" TbBool play_music(const char * fname) {
	std::lock_guard<std::mutex> guard(g_mix_mutex);
	game.music_track = -1;
	snprintf(game.music_fname, sizeof(game.music_fname), "%s", fname);
	// Mix_PlayMusic will stop anything currently playing and eventually
	// calls on_music_finished so theres no need to call Mix_FreeMusic first.
	const auto music = Mix_LoadMUS(game.music_fname);
	if (!music) {
		WARNLOG("Cannot load music from %s: %s", game.music_fname, Mix_GetError());
		return false;
	} else if (Mix_PlayMusic(music, -1) != 0) {
		Mix_FreeMusic(music);
		WARNLOG("Cannot play music from %s: %s", game.music_fname, Mix_GetError());
		return false;
	}
	// g_mix_music will be null here as Mix_PlayMusic ends up calling on_music_finished
	g_mix_music = music;
	JUSTLOG("Playing %s", game.music_fname);
	return true;
}

extern "C" TbBool play_music_track(int track) {
	game.music_track = track;
	memset(game.music_fname, 0, sizeof(game.music_fname));
	if (game.music_track == 0) {
		stop_music();
		return true;
	} else if (features_enabled & Ft_NoCdMusic) {
		return play_music(prepare_file_fmtpath(FGrp_Music, "keeper%02d.ogg", track));
	} else {
		if (PlayRedbookTrack(track)) {
			JUSTLOG("Playing track %d", game.music_track);
			return true;
		} else {
			WARNLOG("Cannot play track %d", game.music_track);
			return false;
		}
	}
}

extern "C" void pause_music() {
	JUSTLOG("Pausing music");
	if (features_enabled & Ft_NoCdMusic) {
		Mix_PauseMusic();
	} else {
		PauseRedbookTrack();
	}
}

extern "C" void resume_music() {
	JUSTLOG("Resuming music");
	if (features_enabled & Ft_NoCdMusic) {
		Mix_ResumeMusic();
	} else {
		ResumeRedbookTrack();
	}
}

extern "C" void stop_music() {
	JUSTLOG("Stopping music");
	game.music_track = 0;
	memset(game.music_fname, 0, sizeof(game.music_fname));
	if (features_enabled & Ft_NoCdMusic) {
		if (Mix_FadingMusic() != MIX_FADING_OUT) {
			Mix_FadeOutMusic(1000);
		}
	} else {
		StopRedbookTrack();
	}
}

extern "C" TbBool GetSoundInstalled() {
	return g_openal_device && g_openal_context;
}

// This function gets called every tick
extern "C" void MonitorStreamedSoundTrack() {
	for (auto & source : g_sources) {
		try {
			if (source.emit_id > 0 && !source.is_playing()) {
				source.emit_id = 0;
				source.smptbl_id = 0;
				source.bank_id = 0;
			}
		} catch (const std::exception & e) {
			ERRORLOG("%s", e.what());
		}
	}
	g_tick_samples.clear();
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
	unsigned char ctype, // possible values: 2, 3
	SoundBankID bank_id
) {
	if (emit_id <= 0) {
		ERRORLOG("Can't play sample %d from bank %u, invalid emitter ID", smptbl_id, bank_id);
		return 0;
	} else if (bank_id > g_banks.size()) {
		ERRORLOG("Can't play sample %d from bank %u, invalid bank ID", smptbl_id, bank_id);
		return 0;
	} else if (smptbl_id == 0) {
		return 0; // silently ignore
	} else if (smptbl_id <= 0 || smptbl_id >= g_banks[bank_id].size()) {
		ERRORLOG("Can't play sample %d from bank %u, invalid sample ID", smptbl_id, bank_id);
		return 0;
	}
	// (ab)use the fact that bank_id and smptbl_id are currently 8- and 16-bits wide respectively.
	const uint32_t tick_sample_key = (uint32_t(bank_id) << 16) | (smptbl_id & 0xffff);
	if (g_tick_samples.count(tick_sample_key) > 0) {
		return 0; // don't play the same sample multiple times on the same tick
	}
	try {
		g_tick_samples.emplace(tick_sample_key);
		for (auto & source : g_sources) {
			if (source.emit_id == 0) {
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
				source.play(g_banks[bank_id][smptbl_id].buffer);
				source.emit_id = emit_id;
				source.smptbl_id = smptbl_id;
				source.bank_id = bank_id;
				return source.mss_id;
			}
		}
        if (game.frame_skip < 2)
        {
            ERRORLOG("Can't play sample %d from bank %u, too many samples playing at once", smptbl_id, bank_id);
        }
		return 0;
	} catch (const std::exception & e) {
		ERRORLOG("%s", e.what());
	}
	return 0;
}

extern "C" void stop_sample(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundBankID bank_id) {
	for (auto & source : g_sources) {
		if (emit_id == source.emit_id && smptbl_id == source.smptbl_id && bank_id == source.bank_id) {
			try {
				source.stop();
				source.emit_id = 0;
				source.smptbl_id = 0;
				source.bank_id = 0;
			} catch (const std::exception & e) {
				ERRORLOG("%s", e.what());
			}
		}
	}
}

extern "C" SoundSFXID get_sample_sfxid(SoundSmplTblID smptbl_id, SoundBankID bank_id) {
	if (bank_id > 1) {
		return 0;
	} else if (smptbl_id < 0 || smptbl_id >= g_banks[bank_id].size()) {
		return 0;
	}
	return g_banks[bank_id][smptbl_id].sfx_id;
}

extern "C" int InitialiseSDLAudio()
{
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		ERRORLOG("Unable to initialise SDL audio subsystem: %s", SDL_GetError());
		return 0;
	}
	int flags = Mix_Init(MIX_INIT_OGG|MIX_INIT_MP3);
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0)
	{
		ERRORLOG("Could not open audio device for SDL mixer: %s", Mix_GetError());
		Mix_Quit();
		return 0;
	}
	Mix_ReserveChannels(1); // reserve for external speech samples
	Mix_HookMusicFinished(on_music_finished); // register callback so we can do things
	return flags;
}

extern "C" void ShutDownSDLAudio()
{
	int frequency, channels;
	unsigned short format;
	int i = Mix_QuerySpec(&frequency, &format, &channels);
	if (i == 0)
	{
		ERRORLOG("Could not query SDL mixer: %s", Mix_GetError());
	}
	while (i > 0)
	{
		Mix_CloseAudio();
		i--;
	}
	while (Mix_Init(0))
	{
		Mix_Quit();
	}
}

extern "C" TbBool play_streamed_sample(const char* fname, SoundVolume volume)
{
	if (SoundDisabled || fname == nullptr || strlen(fname) == 0) {
		return false;
	}
	const auto sample = Mix_LoadWAV(fname);
	if (sample == nullptr) {
		ERRORLOG("Cannot load \"%s\": %s", fname, Mix_GetError());
		return false;
	}
	// SoundVolume ranges 0..255 but MIX_MAX_VOLUME ranges 0..128
	Mix_VolumeChunk(sample, volume / 2);
	if (Mix_PlayChannel(MIX_SPEECH_CHANNEL, sample, 0) != 0) {
		Mix_FreeChunk(sample);
		ERRORLOG("Cannot play \"%s\": %s", fname, Mix_GetError());
		return false;
	}
	std::lock_guard<std::mutex> guard(g_mix_mutex);
	const auto old_sample = std::exchange(g_streamed_sample, sample);
	if (old_sample) {
		Mix_FreeChunk(old_sample);
	}
	return true;
}

extern "C" void stop_streamed_samples()
{
	Mix_HaltChannel(MIX_SPEECH_CHANNEL);
	std::lock_guard<std::mutex> guard(g_mix_mutex);
	const auto old_sample = std::exchange(g_streamed_sample, nullptr);
	if (old_sample) {
		Mix_FreeChunk(g_streamed_sample);
	}
}

extern "C" void set_streamed_sample_volume(SoundVolume volume) {
	// SoundVolume ranges 0..255 but MIX_MAX_VOLUME ranges 0..128
	Mix_VolumeChunk(g_streamed_sample, volume / 2);
}

extern "C" void toggle_bbking_mode() {
	g_bb_king_mode = !g_bb_king_mode;
}
