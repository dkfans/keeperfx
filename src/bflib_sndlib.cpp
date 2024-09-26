#include "pre_inc.h"
#include "config.h"
#include "bflib_sndlib.h"
#include "bflib_sound.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <fstream>
#include <string>
#include <filesystem>
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
SoundVolume g_redbook_volume = 0;
SoundVolume g_music_volume = 0;
ALCdevice_ptr g_openal_device;
ALCcontext_ptr g_openal_context;

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
		alSource3f(id, AL_POSITION, -(float(64 - pan) / 64), 0, 0);
		const auto errcode = alGetError();
		if (errcode != AL_NO_ERROR) {
			throw openal_error("Cannot set position", errcode);
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
		const auto pcm = wav.pcm();
		auto format = wav.format();
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
	uint8_t field_0[14];
	uint32_t field_E;
};
#pragma pack()

#pragma pack(1)
struct SoundBankSample { // sizeof = 32
	/** Name of the sound file the sample comes from. */
	char filename[18];
	/** Offset of the sample data. */
	uint32_t field_12;
	uint32_t field_16;
	/** Size of the sample file. */
	uint32_t data_size;
	SoundSFXID sfxid;
	uint8_t field_1F;
};
#pragma pack()

#pragma pack(1)
struct SoundBankEntry { // sizeof = 16
	uint32_t first_sample_offset;
	uint32_t first_data_offset;
	uint32_t total_samples_size;
	uint32_t field_C;
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
		stream.seekg(directory.first_data_offset + sample.field_12, std::ios::beg);
		buffers.emplace_back(sample.filename, sample.sfxid, wave_file(stream));
	}
	LbJustLog("Loaded %d sound samples from %s", buffers.size(), filename);
	return buffers;
}

std::vector<openal_source> g_sources;
std::vector<sound_sample> g_banks[2];

void load_sound_banks() {
	char snd_fname[2048];
	prepare_file_path_buf(snd_fname, FGrp_LrgSound, "sound.dat");
	// language-specific speech file
	char * spc_fname = prepare_file_fmtpath(FGrp_LrgSound, "speech_%s.dat", get_language_lwrstr(install_info.lang_id));
	// default speech file
	if (!std::filesystem::exists(spc_fname)) {
		spc_fname = prepare_file_path(FGrp_LrgSound, "speech.dat");
	}
	// speech file for english
	if (!std::filesystem::exists(spc_fname)) {
		spc_fname = prepare_file_fmtpath(FGrp_LrgSound, "speech_%s.dat", get_language_lwrstr(1));
	}
	g_banks[0] = load_sound_bank(snd_fname);
	g_banks[1] = load_sound_bank(spc_fname);
}

} // local

extern "C" void FreeAudio() {
	g_sources.clear();
	g_banks[0].clear();
	g_banks[1].clear();
	g_openal_context = nullptr;
	g_openal_device = nullptr;
}

extern "C" void SetRedbookVolume(SoundVolume value) {
	g_redbook_volume = value;
}

extern "C" void SetSoundMasterVolume(SoundVolume value) {
	g_master_volume = value;
}

extern "C" void SetMusicMasterVolume(SoundVolume value) {
	g_music_volume = value;
}

extern "C" TbBool GetSoundInstalled() {
	return g_openal_device && g_openal_context;
}

extern "C" void PlayRedbookTrack(int) {
	// TODO
}

extern "C" void PauseRedbookTrack() {
	// TODO
}

extern "C" void ResumeRedbookTrack() {
	// TODO
}

extern "C" void MonitorStreamedSoundTrack() {
	for (auto & source : g_sources) {
		try {
			if (source.emit_id > 0 && !source.is_playing()) {
				source.emit_id = 0;
				source.smptbl_id = 0;
				source.bank_id = 0;
			}
		} catch (const std::exception & e) {
			LbErrorLog("%s", e.what());
		}
	}
}

extern "C" void StopRedbookTrack() {
	// TODO
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
			LbErrorLog("%s", e.what());
		}
	}
}

extern "C" TbBool InitAudio(const SoundSettings * settings) {
	try {
		if (SoundDisabled) {
			LbWarnLog("Sound is disabled, skipping OpenAL initialization");
			return false;
		}
		if (g_openal_device || g_openal_context) {
			LbWarnLog("OpenAL already initialized");
			return true;
		}
		ALCdevice_ptr device(alcOpenDevice(nullptr));
		if (!device) {
			throw openal_error("Cannot open device");
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
		LbErrorLog("%s", e.what());
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
		LbErrorLog("%s", e.what());
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
				LbErrorLog("%s", e.what());
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
				LbErrorLog("%s", e.what());
			}
		}
	}
}

extern "C" void SetSamplePitch(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundPitch pitch) {
	for (auto & source : g_sources) {
		if (source.emit_id == emit_id && source.smptbl_id == smptbl_id) {
			try {
				source.pitch(pitch);
			} catch (const std::exception & e) {
				LbErrorLog("%s", e.what());
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
	char fild1D, // possible values: -1, 0
	unsigned char ctype, // possible values: 2, 3
	SoundBankID bank_id
) {
	if (emit_id <= 0) {
		LbErrorLog("Can't play sample %d from bank %u, invalid emitter ID", smptbl_id, bank_id);
		return 0;
	} else if (bank_id > 1) {
		LbErrorLog("Can't play sample %d from bank %u, invalid bank ID", smptbl_id, bank_id);
		return 0;
	} else if (smptbl_id <= 0 || smptbl_id >= g_banks[bank_id].size()) {
		LbErrorLog("Can't play sample %d from bank %u, invalid sample ID", smptbl_id, bank_id);
		return 0;
	}
	try {
		for (auto & source : g_sources) {
			if (source.emit_id == 0) {
				source.play(g_banks[bank_id][smptbl_id].buffer);
				source.emit_id = emit_id;
				source.smptbl_id = smptbl_id;
				source.bank_id = bank_id;
				return source.mss_id;
			}
		}
		LbErrorLog("Can't play sample %d from bank %u, too many samples playing at once", smptbl_id, bank_id);
		return 0;
	} catch (const std::exception & e) {
		LbErrorLog("%s", e.what());
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
				LbErrorLog("%s", e.what());
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
