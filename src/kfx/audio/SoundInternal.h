#pragma once
// Internal state shared by the sound units; not part of the public API.
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include "bflib_sound.h"
#include "bflib_sndlib.h"
#include "AudioCallback.h"
#include "Commands.h"
#include "VoiceMirror.h"

extern kfx::audio::VoiceMirror g_voice_mirror;
extern kfx::audio::AudioCallback g_audio_callback;
extern kfx::audio::CommandRing g_command_ring;
extern kfx::audio::EventRing g_event_ring;
extern bool g_bb_king_mode;

inline const char * alErrorStr(ALenum code) {
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

class wave_file;

struct sound_sample {

	std::string name;
	SoundSFXID sfx_id;
	openal_buffer buffer;

	sound_sample(const char * _name, SoundSFXID _sfx_id, const wave_file & wav);

	void buffer_msadpcm(const wave_file & wav);

	sound_sample(const char * _name, SoundSFXID _sfx_id,
	             const std::vector<uint8_t> & pcm, ALenum format, int samplerate);
};

struct SoundStackPolicy {
	unsigned char mode = SStack_Limit;
	short max_instances = 1;
};

extern std::array<std::vector<sound_sample>, 2> g_banks;
extern std::vector<sound_sample> g_custom_bank;
extern SoundSmplTblID g_speech_offset;
extern SoundSmplTblID g_custom_offset;
extern std::unordered_map<SoundSmplTblID, SoundSmplTblID> g_id_redirects;
extern std::unordered_map<SoundSmplTblID, SoundStackPolicy> g_stack_policies;

bool lookup_stack_policy(SoundSmplTblID smptbl_id, kfx::audio::StackPolicy & out);
void forget_bank_buffers(std::vector<sound_sample> & bank);
void load_sound_banks();
void free_music_state();
