#include "pre_inc.h"
// SDL3_mixer: music, streamed speech, and the mixer lifetime.
#include "config_keeperfx.h"
#include "cdrom.h"
#include "bflib_sndlib.h"
#include "bflib_sound.h"
#include "bflib_fileio.h"
#include "game_legacy.h"
#include "kfx/profiling/KfxProfiling.h"
#include "kfx/audio/AudioLog.h"
#include "kfx/audio/AudioFormats.h"
#include "kfx/audio/SoundInternal.h"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#include <utility>
#include <mutex>
#include <algorithm>

#include "keeperfx.hpp"
#include "post_inc.h"

namespace {

SoundVolume g_music_volume = 0;
// SDL3_mixer replaces the SDL2 Mix_Music/Mix_Chunk + channel model with a
// mixer + per-purpose tracks. Music and speech each get their own track.
MIX_Mixer* g_mixer = nullptr;
MIX_Track* g_music_track = nullptr;
MIX_Track* g_speech_track = nullptr;

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

void free_music_state() {
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
		const int prio = kfx::audio::audio_format_music_priority(kfx::audio::audio_format_from_path(fe.Filename));
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

