/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file sounds.h
 *     Header file for sounds.c.
 * @par Purpose:
 *     Sound related functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     11 Jul 2010 - 05 Nov 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_SOUNDS_H
#define DK_SOUNDS_H

#include "bflib_basics.h"
#include "globals.h"

#include <SDL2/SDL_mixer.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FULL_LOUDNESS 256
#define NORMAL_PITCH 100

/******************************************************************************/
#pragma pack(1)

struct Thing;

struct SoundSettings {
  char *sound_data_path;
  char *music_data_path;
  char *dir3;
  unsigned short sound_type;
  unsigned short flags;
  unsigned char max_number_of_samples;
  unsigned char stereo;
  unsigned char field_12;
  unsigned char danger_music;
  unsigned char no_load_sounds;
  unsigned char no_load_music;
  unsigned char field_16;
  unsigned char sound_system;
  unsigned char field_18;
  unsigned char redbook_enable;
};

struct SoundBankHead { // sizeof = 18
  unsigned char field_0[14];
  unsigned long field_E;
};

struct SoundBankSample { // sizeof = 32
    /** Name of the sound file the sample comes from. */
    unsigned char filename[18];
    /** Offset of the sample data. */
    unsigned long field_12;
    unsigned long field_16;
    /** Size of the sample file. */
    unsigned long data_size;
    unsigned char sfxid;
    unsigned char field_1F;
};

struct SoundBankEntry { // sizeof = 16
  unsigned long field_0;
  unsigned long field_4;
  unsigned long field_8;
  unsigned long field_C;
};

enum SoundSettingsFlags {
    SndSetting_None    = 0x00,
    SndSetting_MIDI = 0x01,
    SndSetting_Sound = 0x02,
};

extern int atmos_sound_frequency;
extern int sdl_flags;
extern Mix_Chunk* streamed_sample;

#pragma pack()

/******************************************************************************/
TbBool init_sound_heap_two_banks(unsigned char *heap_mem, long heap_size, char *snd_fname, char *spc_fname, long a5);
TbBool init_sound(void);
void sound_reinit_after_load(void);

void update_player_sounds(void);
void process_3d_sounds(void);
void process_sound_heap(void);

void thing_play_sample(struct Thing *thing, short smptbl_idx, unsigned short a3, char a4, unsigned char a5, unsigned char a6, long a7, long loudness);
void stop_thing_playing_sample(struct Thing *thing, short smpl_idx);
void play_thing_walking(struct Thing *thing);

TbBool ambient_sound_prepare(void);
TbBool ambient_sound_stop(void);
struct Thing *create_ambient_sound(const struct Coord3d *pos, ThingModel model, PlayerNumber owner);

void mute_audio(TbBool mute);
void pause_music(TbBool pause);

void update_first_person_object_ambience(struct Thing *thing);

int InitialiseSDLAudio();
void ShutDownSDLAudio();
void free_sound_chunks();
void play_external_sound_sample(unsigned char smpl_id);
TbBool play_streamed_sample(char* fname, int volume, int loops);
void stop_streamed_sample();
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
