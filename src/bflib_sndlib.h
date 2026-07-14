/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_sndlib.h
 *     Header file for bflib_sndlib.c.
 * @par Purpose:
 *     Low-level sound and music related routines.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     16 Nov 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_SNDLIB_H
#define BFLIB_SNDLIB_H

#include "bflib_basics.h"
#include "bflib_sound.h"
#include "sounds.h"
#include "globals.h"

#define FIRST_REDBOOK_TRACK 2
#define LAST_REDBOOK_TRACK 7

#ifdef __cplusplus
extern "C" {
#endif

void FreeAudio(void);
void SetSoundMasterVolume(SoundVolume);
TbBool GetSoundInstalled(void);
void MonitorStreamedSoundTrack(void);
void * GetSoundDriver(void);
void StopAllSamples(void);
TbBool InitAudio(const struct SoundSettings *);
TbBool IsSamplePlaying(SoundMilesID);
SoundVolume GetCurrentSoundMasterVolume(void);
void SetSampleVolume(SoundEmitterID, SoundSmplTblID, SoundVolume);
void SetSamplePan(SoundEmitterID, SoundSmplTblID, SoundPan);
void SetSamplePitch(SoundEmitterID, SoundSmplTblID, SoundPitch);
void toggle_bbking_mode(void);

/**
 * @brief Register a raw sound.dat effect ID to be transparently redirected to a custom bank ID.
 *
 * Called by config_sounds.c when a numeric-key entry (e.g. "777 = custom/boom.wav") is parsed.
 * The redirect is applied in play_sample() before bank dispatch; it only affects IDs in the
 * effect bank (0..g_speech_offset-1) and has no effect on speech or already-custom IDs.
 *
 * @param from_id  Raw effect ID as it would appear in sound.dat (e.g. 777)
 * @param to_id    Unified custom bank ID returned by sound_manager_load_named_sound()
 */
void sound_register_id_redirect(SoundSmplTblID from_id, SoundSmplTblID to_id);

/**
 * @brief Clear all registered raw-ID redirects.
 *
 * Called during audio teardown and alongside custom_sound_bank_clear() on level reload.
 */
void sound_clear_id_redirects(void);

/**
 * @brief Save a snapshot of the ID-redirect table and custom-bank watermark.
 *
 * Call after campaign + mod sounds finish loading. Pair with
 * sound_restore_id_redirect_snapshot() at the start of each level load.
 */
void sound_save_id_redirect_snapshot(void);

/**
 * @brief Restore the ID-redirect table and truncate the custom bank to the
 * saved watermark, freeing any sounds added at level scope.
 */
void sound_restore_id_redirect_snapshot(void);

void set_music_volume(SoundVolume);
TbBool play_music(const char * fname);
TbBool play_music_fgroup(short fgroup, const char * fname);
TbBool play_music_track(int);
void pause_music(void);
void resume_music(void);
void stop_music(TbBool fade_out);

/**
 * @brief Concurrency policy applied to a unified sound sample ID when it is
 * triggered by more than one emitter (Thing/UI source) at the same time.
 *
 */
enum SoundStackMode {
	SStack_Limit = 0, // at most max_instances concurrent instances; extra triggers are dropped
	SStack_Duck  = 1, // gain of every active instance is scaled down as concurrency rises
};

/**
 * @brief Register the stacking policy for a unified sound sample ID.
 *
 * If a sample ID has no registered policy, it does NOT use this Limit/Duck struct at all —
 * it instead falls back to a separate once-per-tick gate in play_sample() (see
 * g_tick_samples_last_tick in bflib_sndlib.cpp), which reproduces the OG behaviour (at most
 * one instance of a given sample plays at a time, regardless of how many emitters trigger
 * it). The gate is keyed by an internal tick counter advanced in MonitorStreamedSoundTrack()
 * rather than the in-game turn counter, so it still works correctly for UI/menu sounds
 * triggered outside active gameplay (e.g. the frontend/main menu), where the game turn
 * counter never advances.
 * That legacy gate and this policy table are mutually exclusive per sample ID: registering
 * an explicit policy here (even {Limit, 1}) opts the sample out of the legacy gate and into
 * the duration-based concurrency tracking this struct describes instead.
 *
 * @param smptbl_id      Unified sample ID (effect, speech, or custom bank).
 * @param mode           SStack_Limit or SStack_Duck.
 * @param max_instances  For SStack_Limit: hard cap (clamped to >= 1).
 *                        For SStack_Duck: 0 means uncapped, >0 also caps concurrency.
 */
void sound_register_stack_policy(SoundSmplTblID smptbl_id, unsigned char mode, short max_instances);

/**
 * @brief Clear all registered stacking policies (samples with no policy revert to the
 * legacy once-per-turn gate described above).
 */
void sound_clear_stack_policies(void);

#ifdef __cplusplus
}
#endif
#endif
