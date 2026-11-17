#include "pre_inc.h"
// Audio device lifetime: InitAudio/FreeAudio, device logging, master volume.
#include "config_keeperfx.h"
#include "bflib_sndlib.h"
#include "bflib_datetm.h"
#include "bflib_sound.h"
#include "kfx/audio/AudioLog.h"
#include "kfx/audio/AudioStream.h"
#include "kfx/audio/SoundInternal.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <SDL3/SDL.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "keeperfx.hpp"
#include "post_inc.h"

// Sound effects: game-thread voice bookkeeping and the audio-thread OpenAL renderer.
kfx::audio::VoiceMirror g_voice_mirror;
kfx::audio::AudioCallback g_audio_callback;
kfx::audio::CommandRing g_command_ring;
kfx::audio::EventRing g_event_ring;
bool g_bb_king_mode = false;

namespace {

SoundVolume g_master_volume = 0;
// InitAudio holds its own reference on the SDL audio subsystem for the SDL3 stream.
bool g_sdl_audio_subsystem_held = false;

#ifdef AL_EXT_debug
LPALDEBUGMESSAGECALLBACKEXT g_al_debug_message_callback = nullptr;
#endif

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

} // local

extern "C" void FreeAudio() {
	SYNCDBG(6, "Starting audio cleanup");

	free_music_state();

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

extern "C" TbBool GetSoundInstalled() {
	return g_audio_callback.initialized();
}

extern "C" void * GetSoundDriver() {
	// This just needs to return any non-null pointer. FMV library appears to have standalone audio
	static int dummy = 0;
	return &dummy;
}

extern "C" void StopAllSamples() {
	g_voice_mirror.stop_all(g_command_ring);
}

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
		// Own SDL audio reference, dropped in release_audio_output().
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

extern "C" SoundVolume GetCurrentSoundMasterVolume() {
	return g_master_volume;
}

