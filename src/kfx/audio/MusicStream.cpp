#include "pre_inc.h"
// Music and streamed speech through the stream player.
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
#include "kfx/audio/StreamPlayer.h"
#include <SDL3/SDL.h>
#include <chrono>
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
std::mutex g_mix_mutex;

std::string g_current_music_fname; // empty if a numbered track (or nothing) is playing
int g_current_music_track = 0;     // 0 if a custom file (or nothing) is playing

} // local

void free_music_state() {
	kfx::audio::stream_player().shutdown();
	{
		std::lock_guard<std::mutex> guard(g_mix_mutex);
		g_current_music_track = 0;
		g_current_music_fname.clear();
	}
	SYNCDBG(7, "Music state freed");
}

extern "C" void set_music_volume(SoundVolume value) {
	g_music_volume = value;
	SetRedbookVolume(value);
	kfx::audio::stream_player().set_volume(kfx::audio::PlayerChannel::Music, float(value) / FULL_LOUDNESS);
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
	KFX_ZONE_COLOR("play_music", KFX_COLOR_AUDIO);
	if (SoundDisabled) {
		return false;
	}
	if (!kfx_audio_path_supported(game.music_fname)) {
		AUDIOWARN("unsupported audio format '%s', supported: %s", game.music_fname, kfx_audio_format_list_text());
		return false;
	}
	if (!kfx::audio::stream_player().play(kfx::audio::PlayerChannel::Music, game.music_fname,
			float(g_music_volume) / FULL_LOUDNESS, true)) {
		return false;
	}
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
		kfx::audio::stream_player().set_paused(kfx::audio::PlayerChannel::Music, true);
	} else {
		PauseRedbookTrack();
	}
}

extern "C" void resume_music() {
	if (features_enabled & Ft_NoCdMusic) {
		kfx::audio::stream_player().set_paused(kfx::audio::PlayerChannel::Music, false);
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
		if (fade_out) {
			kfx::audio::stream_player().fade_out(kfx::audio::PlayerChannel::Music, 1000);
		} else {
			kfx::audio::stream_player().stop(kfx::audio::PlayerChannel::Music);
		}
	} else {
		StopRedbookTrack();
	}
}

// With KFX_AUDIO_STATS=1 set, logs the sample drop counts summed over each second (only when non-zero).
static void log_drop_totals(uint32_t stolen, uint32_t tick_gate, uint32_t stack_cap, uint32_t no_voice,
	uint32_t commands_dropped, bool events_overflowed)
{
	static const bool enabled = (getenv("KFX_AUDIO_STATS") != nullptr);
	if (!enabled) {
		return;
	}
	static uint64_t totals[10] = {};
	static auto window_start = std::chrono::steady_clock::now();
	const uint64_t add[10] = {stolen, tick_gate, stack_cap, no_voice, commands_dropped, events_overflowed ? 1u : 0u,
		S3DDropsPriority, S3DDropsEviction, S3DDropsVolumeZero, S3DDropsOutOfRange};
	for (int i = 0; i < 10; ++i) {
		totals[i] += add[i];
	}
	const auto now = std::chrono::steady_clock::now();
	if (now - window_start < std::chrono::seconds(1)) {
		return;
	}
	window_start = now;
	uint64_t sum = 0;
	for (uint64_t t : totals) {
		sum += t;
	}
	if (sum != 0) {
		AUDIOLOG("drops/s: S3D list full %llu, S3D evicted %llu, silent %llu, out of range %llu | tick gate %llu, "
			"stack cap %llu, no voice %llu, stolen %llu, command ring full %llu, event ring overflow %llu",
			(unsigned long long)totals[6], (unsigned long long)totals[7], (unsigned long long)totals[8],
			(unsigned long long)totals[9], (unsigned long long)totals[1], (unsigned long long)totals[2],
			(unsigned long long)totals[3], (unsigned long long)totals[0], (unsigned long long)totals[4],
			(unsigned long long)totals[5]);
	}
	for (uint64_t & t : totals) {
		t = 0;
	}
}

// This function gets called every tick, both during active gameplay and while navigating
// the frontend/main menu, this should probably be buffer based and not tick based..
extern "C" void MonitorStreamedSoundTrack() {
	kfx::audio::stream_player().update();
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
	log_drop_totals(voices_stolen, drops_tick_gate, drops_stack_cap, drops_no_voice, commands_dropped, events_overflowed);
	S3DDropsPriority = 0;
	S3DDropsEviction = 0;
	S3DDropsVolumeZero = 0;
	S3DDropsOutOfRange = 0;
}

extern "C" TbBool play_streamed_sample(const char* fname, SoundVolume volume)
{
	if (SoundDisabled || fname == nullptr || strlen(fname) == 0) {
		return false;
	}
	KFX_ZONE_COLOR("play_streamed_sample", KFX_COLOR_AUDIO);
	if (!kfx_audio_path_supported(fname)) {
		AUDIOWARN("unsupported audio format '%s', supported: %s", fname, kfx_audio_format_list_text());
		return false;
	}
	return kfx::audio::stream_player().play(kfx::audio::PlayerChannel::Speech, fname,
		float(volume) / FULL_LOUDNESS, false);
}

extern "C" void stop_streamed_samples()
{
	kfx::audio::stream_player().stop(kfx::audio::PlayerChannel::Speech);
}

extern "C" void set_streamed_sample_volume(SoundVolume volume) {
	kfx::audio::stream_player().set_volume(kfx::audio::PlayerChannel::Speech, float(volume) / FULL_LOUDNESS);
}

extern "C" TbBool is_streamed_sample_playing(void) {
	return kfx::audio::stream_player().is_playing(kfx::audio::PlayerChannel::Speech);
}
