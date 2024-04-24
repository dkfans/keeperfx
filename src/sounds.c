/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file sounds.c
 *     Sound related functions.
 * @par Purpose:
 *     Routines used for playing game-specific sounds.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     15 Jul 2010 - 05 Nov 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "sounds.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_sound.h"
#include "bflib_sndlib.h"
#include "bflib_fileio.h"
#include "bflib_memory.h"
#include "bflib_math.h"
#include "bflib_planar.h"
#include "bflib_bufrw.h"
#include "engine_render.h"
#include "map_utils.h"
#include "engine_camera.h"
#include "gui_soundmsgs.h"
#include "gui_topmsg.h"
#include "front_landview.h"
#include "frontmenu_ingame_evnt.h"
#include "thing_data.h"
#include "thing_navigate.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "game_legacy.h"
#include "config_settings.h"
#include "music_player.h"
#include "creature_senses.h"
#include "map_data.h"
#include "creature_states.h"
#include "thing_objects.h"
#include "config.h"
#include "lvl_script_commands.h"

#include "keeperfx.hpp"
#include "game_heap.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char foot_down_sound_sample_variant[] = {
   0,  1,  2,  3,  1,  0,  3,  2,  0,  3,  1,  2,  1,  2,  0,  3,
   1,  0,  1,  1,  1,  1,  0,  1,  0,  1, -1,  1, -1,  1, -1,  0,
  -1,  0, -1, -1, -1, -1,  0, -1,  0, -1,  1, -1,  1, -1,  1,  0,
};

char sound_dir[64] = "SOUND";
int atmos_sound_frequency = 800;
static char ambience_timer;
int sdl_flags = 0;
Mix_Chunk* streamed_sample;
/******************************************************************************/
void thing_play_sample(struct Thing *thing, short smptbl_idx, unsigned short pitch, char a4, unsigned char a5, unsigned char a6, long priority, long loudness)
{
    if (SoundDisabled)
        return;
    if (GetCurrentSoundMasterVolume() <= 0)
        return;
    if (thing_is_invalid(thing))
        return;
    struct Coord3d rcpos;
    rcpos.x.val = Receiver.pos.val_x;
    rcpos.y.val = Receiver.pos.val_y;
    rcpos.z.val = Receiver.pos.val_z;
    if (get_chessboard_3d_distance(&rcpos, &thing->mappos) < MaxSoundDistance)
    {
        long eidx = thing->snd_emitter_id;
        if (eidx > 0)
        {
            S3DAddSampleToEmitterPri(eidx, smptbl_idx, 0, pitch, loudness, a4, a5, a6 | 0x01, priority);
        } else
        {
            eidx = S3DCreateSoundEmitterPri(thing->mappos.x.val, thing->mappos.y.val, thing->mappos.z.val,
               smptbl_idx, 0, pitch, loudness, a4, a6 | 0x01, priority);
           thing->snd_emitter_id = eidx;
        }
    }
}

void play_sound_if_close_to_receiver(struct Coord3d *soundpos, short smptbl_idx)
{
    if (SoundDisabled)
        return;
    if (GetCurrentSoundMasterVolume() <= 0)
        return;
    struct Coord3d rcpos;
    rcpos.x.val = Receiver.pos.val_x;
    rcpos.y.val = Receiver.pos.val_y;
    rcpos.z.val = Receiver.pos.val_z;
    if (get_chessboard_3d_distance(&rcpos, soundpos) < MaxSoundDistance)
    {
        play_non_3d_sample(smptbl_idx);
    }
}

void play_thing_walking(struct Thing *thing)
{
    struct PlayerInfo* myplyr = get_my_player();
    struct Camera* cam = myplyr->acamera;
    struct CreatureStats* crstat;
    { // Skip the thing if its distance to camera is too big
        MapSubtlDelta dist_x = coord_subtile(abs(cam->mappos.x.val - (MapCoordDelta)thing->mappos.x.val));
        MapSubtlDelta dist_y = coord_subtile(abs(cam->mappos.y.val - (MapCoordDelta)thing->mappos.y.val));
        if (dist_x <= dist_y)
          dist_x = dist_y;
        if (dist_x >= 10) {
            return;
        }
    }
    if ((get_creature_model_flags(thing) & CMF_IsSpectator) != 0) {
        // Spectators don't do sounds
        return;
    }
    long loudness = (myplyr->view_mode == PVM_CreatureView) ? (FULL_LOUDNESS) : (FULL_LOUDNESS / 5);
    if (((thing->movement_flags & TMvF_Flying) != 0) && (thing->floor_height < (int)thing->mappos.z.val))
    {
        // Flying diptera has a buzzing noise sound
        if (get_creature_model_flags(thing) & CMF_IsDiptera)
        {
            if (!S3DEmitterIsPlayingSample(thing->snd_emitter_id, 25, 0))
            {
                thing_play_sample(thing, 25, 100, -1, 2, 0, 2, loudness);
            }
        }
    }
    else
    {
        if ( S3DEmitterIsPlayingSample(thing->snd_emitter_id, 25, 0) ) {
            S3DDeleteSampleFromEmitter(thing->snd_emitter_id, 25, 0);
        }
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if ((cctrl->distance_to_destination) && get_foot_creature_has_down(thing))
        {
            int smpl_variant = foot_down_sound_sample_variant[4 * ((cctrl->mood_flags & 0x1C) >> 2) + (cctrl->sound_flag & 0x1F)];
            long smpl_idx;
            if ((thing->movement_flags & TMvF_IsOnSnow) != 0) {
                smpl_idx = 181 + smpl_variant;
            } else {
                struct CreatureSound* crsound = get_creature_sound(thing, CrSnd_Foot);
                smpl_idx = crsound->index + smpl_variant;
            }
            cctrl->sound_flag = (cctrl->sound_flag ^ (cctrl->sound_flag ^ (cctrl->sound_flag + 1))) & 0x1F;
            if ((cctrl->sound_flag & 0x1F) >= 4)
            {
                cctrl->mood_flags &= ~0x1C;
                cctrl->mood_flags |=  (UNSYNC_RANDOM(4) << 2);
                cctrl->sound_flag &= ~0x1F;
            }
            crstat = creature_stats_get(thing->model);
            thing_play_sample(thing, smpl_idx, crstat->footstep_pitch, 0, 3, 3, 1, loudness);
            if ((thing->movement_flags & TMvF_IsOnWater) != 0) {
                thing_play_sample(thing, 21 + UNSYNC_RANDOM(4), 90 + UNSYNC_RANDOM(20), 0, 3, 3, 1, FULL_LOUDNESS);
            }
        }
    }
}

void set_room_playing_ambient_sound(struct Coord3d *pos, long sample_idx)
{
    long i;
    if (game.ambient_sound_thing_idx == 0)
    {
        ERRORLOG("No room ambient sound object");
        return;
    }
    struct Thing* thing = thing_get(game.ambient_sound_thing_idx);
    if ( thing_is_invalid(thing) )
    {
        ERRORLOG("Invalid room ambient sound object");
        return;
    }
    if (sample_idx != 0)
    {
        move_thing_in_map(thing, pos);
        i = thing->snd_emitter_id;
        if (i != 0)
        {
            if ( !S3DEmitterIsPlayingSample(i, sample_idx, 0) )
            {
                S3DDeleteAllSamplesFromEmitter(thing->snd_emitter_id);
                thing_play_sample(thing, sample_idx, NORMAL_PITCH, -1, 3, 0, 6, FULL_LOUDNESS);
            }
        } else
        {
            thing_play_sample(thing, sample_idx, NORMAL_PITCH, -1, 3, 0, 6, FULL_LOUDNESS);
        }
    } else
    {
        i = thing->snd_emitter_id;
        if (i != 0)
        {
            S3DDestroySoundEmitterAndSamples(i);
            thing->snd_emitter_id = 0;
        }
    }
}

void find_nearest_rooms_for_ambient_sound(void)
{
    SYNCDBG(8,"Starting");
    if ((SoundDisabled) || (GetCurrentSoundMasterVolume() <= 0))
        return;
    struct PlayerInfo* player = get_my_player();
    struct Camera* cam = player->acamera;
    if (cam == NULL || LbIsFrozenOrPaused())
    {
        if (cam == NULL)
        {
            ERRORLOG("No active camera");
        }
        set_room_playing_ambient_sound(NULL, 0);
        return;
    }
    long slb_x = subtile_slab(cam->mappos.x.stl.num);
    long slb_y = subtile_slab(cam->mappos.y.stl.num);
    for (long i = 0; i < 11 * 11; i++)
    {
        struct MapOffset* sstep = &spiral_step[i];
        MapSubtlCoord stl_x = slab_subtile_center(slb_x + sstep->h);
        MapSubtlCoord stl_y = slab_subtile_center(slb_y + sstep->v);
        if (subtile_is_player_room(player->id_number,stl_x,stl_y))
        {
            struct Room* room = subtile_room_get(stl_x, stl_y);
            if (room_is_invalid(room))
                continue;
            struct RoomConfigStats* roomst = &game.conf.slab_conf.room_cfgstats[room->kind];
            long k = roomst->ambient_snd_smp_id;
            if (k > 0)
            {
                SYNCDBG(8,"Playing ambient for %s at (%d,%d)",room_code_name(room->kind),(int)stl_x,(int)stl_y);
                struct Coord3d pos;
                pos.x.val = subtile_coord_center(stl_x);
                pos.y.val = subtile_coord_center(stl_y);
                pos.z.val = subtile_coord(1,0);
                set_room_playing_ambient_sound(&pos, k);
                return;
            }
        }
    }
    set_room_playing_ambient_sound(NULL, 0);
}

TbBool update_3d_sound_receiver(struct PlayerInfo* player)
{
    SYNCDBG(7, "Starting");
    struct Camera* cam = player->acamera;
    if (cam == NULL)
        return false;
    S3DSetSoundReceiverPosition(cam->mappos.x.val, cam->mappos.y.val, cam->mappos.z.val);
    S3DSetSoundReceiverOrientation(cam->orient_a, cam->orient_b, cam->orient_c);
    if (
        cam->view_mode == PVM_IsoWibbleView ||
        cam->view_mode == PVM_FrontView ||
        cam->view_mode == PVM_IsoStraightView
    ) {
        // Distance from center of camera that you can hear a sound
        S3DSetMaximumSoundDistance(lerp(5120, 27648, 1.0-hud_scale));
        // Quieten sounds when zoomed out
        float upper_range_only = min(hud_scale*2.0, 1.0);
        float rescale_audio = max(min(fastPow(upper_range_only, 1.25), 1.0), 0.0);
        S3DSetSoundReceiverSensitivity(lerp(2, 64, rescale_audio));
    } else {
        S3DSetMaximumSoundDistance(5120);
        S3DSetSoundReceiverSensitivity(64);
    }
    return true;
}

void update_player_sounds(void)
{
    SYNCDBG(7,"Starting");
    if ((game.operation_flags & GOF_Paused) == 0)
    {
        struct PlayerInfo* player = get_my_player();
        process_messages();
        if (!SoundDisabled)
        {
            PlayMusicPlayer(game.audiotrack);
            update_3d_sound_receiver(player);
        }
        game.play_gameturn++;
    }
    find_nearest_rooms_for_ambient_sound();
    process_3d_sounds();
    int k = (game.bonus_time - game.play_gameturn) / 2;
    if (bonus_timer_enabled())
    {
        if ((game.bonus_time == game.play_gameturn) ||
            ((game.bonus_time > game.play_gameturn) &&
            (   ((k <= 100)  && ((k % 10) == 0)) ||
                ((k <= 300)  && ((k % 50) == 0)) ||
                ((k <= 5000) && ((k % 250) == 0)) ||
                                ((k % 5000) == 0)    )  ))
        play_non_3d_sample(89);
    }
    if (game.play_gameturn != 0)
    {
        // Easter Egg Speeches

        // Interval for easter egg speeches. Original DK value was 20000 (16.6 minutes)
        if (game.conf.rules.game.easter_egg_speech_interval != 0 && (game.play_gameturn % game.conf.rules.game.easter_egg_speech_interval) == 0)
        {
            // The chance for the easter egg speech to trigger. Original DK value was 1/2000
            if (game.conf.rules.game.easter_egg_speech_chance != 0 && UNSYNC_RANDOM(game.conf.rules.game.easter_egg_speech_chance) == 0)
            {
                // Select a random Easter egg speech
                k = UNSYNC_RANDOM(10);
                SYNCDBG(9,"Rare message condition met, selected %d",(int)k);

                if (k == 7)
                {
                    // Replace SMsg_Glaagh with SMsg_PantsTooTight
                    // Most likely because 'Glaagh' is a bit negative in this scenario
                    output_message(SMsg_PantsTooTight, 0, true);
                }
                else
                {
                    // Play one of the speeches
                    output_message(SMsg_FunnyMessages+k, 0, true);
                }
            }

        }
        else
        {
            // Atmospheric background sound, replaces AWE soundfont
            if ( atmos_sounds_enabled() )
            {
                //Plays a sound on repeat, default sound sample 1013(water drops), with a small chance of a random other sound from the range.
                k = UNSYNC_RANDOM(atmos_sound_frequency);
                if (k == 1)
                {
                    // No atmos sounds the first 3 minutes
                    if (game.play_gameturn > 3600)
                    {
                        play_atmos_sound(AtmosStart + UNSYNC_RANDOM((AtmosEnd + 1) - AtmosStart));
                    }
                } else
                {
                    // No atmos drops the first 30 seconds
                    if (game.play_gameturn > 600)
                    {
                        // Roughly every 2 seconds drops sound
                        if ((k % 40) == 0)
                        {
                            play_atmos_sound(AtmosRepeat);
                        }
                    }
                }
            }
        }
    }

    // Music and sound control
    if ( !SoundDisabled ) {
        if ( (game.turns_fastforward == 0) && (!game.numfield_149F38) ) {
            MonitorStreamedSoundTrack();
            process_sound_heap();
        }
    }
    SYNCDBG(9,"Finished");
}

void process_3d_sounds(void)
{
    SYNCDBG(9,"Starting");
    increment_sample_times();
    process_sound_samples();
    process_sound_emitters();
}

// This function marks not playing samples (to remove them from heap MB)
void process_sound_heap(void)
{
    long i = 0;
    SYNCDBG(9,"Starting");
    struct SampleInfo* smpinfo_last = GetLastSampleInfoStructure();
    if (smpinfo_last == NULL) {
        return;
    }
    for (struct SampleInfo* smpinfo = GetFirstSampleInfoStructure(); smpinfo <= smpinfo_last; smpinfo++)
    {
      if ( (smpinfo->field_0 != 0) && ((smpinfo->flags_17 & 0x01) != 0) )
      {
          if ( IsSamplePlaying(0, 0, smpinfo->field_0) )
          {
              i++;
          } else
          {
              smpinfo->flags_17 &= ~0x01;
              smpinfo->flags_17 &= ~0x04;
          }
      }
    }
    SYNCDBG(9,"Done (%l playing yet)", i);
}

long parse_sound_file(TbFileHandle fileh, unsigned char *buf, long *nsamples, long buf_len, long a5)
{
    long k;

    // TODO SOUND use rewritten version when sound routines are rewritten

    switch ( a5 )
    {
    case 1610:
        k = 5;
        break;
    case 822:
        k = 6;
        break;
    case 811:
        k = 7;
        break;
    case 800:
        k = 8;
        break;
    case 1611:
        k = 4;
        break;
    case 1620:
        k = 3;
        break;
    case 1622:
        k = 2;
        break;
    case 1640:
        k = 1;
        break;
    case 1644:
        k = 0;
        break;
    default:
        return 0;
    }
    LbFileSeek(fileh, 0, Lb_FILE_SEEK_END);
    long fsize = LbFilePosition(fileh);
    LbFileSeek(fileh, fsize-4, Lb_FILE_SEEK_BEGINNING);
    unsigned char rbuf[8];
    LbFileRead(fileh, &rbuf, 4);
    long i = read_int32_le_buf(rbuf);
    LbFileSeek(fileh, i, Lb_FILE_SEEK_BEGINNING);
    struct SoundBankHead bhead;
    LbFileRead(fileh, &bhead, sizeof(bhead));
    struct SoundBankEntry bentries[9];
    LbFileRead(fileh, bentries, sizeof(bentries));
    struct SoundBankEntry* bentry = &bentries[k];
    if (bentry->field_0 == 0) {
        return 0;
    }
    if (bentry->field_8 == 0) {
        return 0;
    }
    i = bentry->field_8 / sizeof(struct SoundBankSample);
    *nsamples = i;
    if (sizeof(struct SampleTable) * (*nsamples) >= buf_len) {
        return 0;
    }
    LbFileSeek(fileh, bentry->field_0, Lb_FILE_SEEK_BEGINNING);
    struct SampleTable* smpl = (struct SampleTable*)buf;
    k = bentry->field_4;
    for (i=0; i < *nsamples; i++)
    {
        struct SoundBankSample bsample;
        LbFileRead(fileh, &bsample, sizeof(struct SoundBankSample));
        smpl->file_pos = k + bsample.field_12;
        smpl->data_size = bsample.data_size;
        smpl->sfxid = bsample.sfxid;
        he_free(smpl->snd_buf);
        smpl->snd_buf = NULL;
        smpl++;
    }
    //TODO SOUND Check why we're returning nsamples * 32 and not nsamples * 16
    return sizeof(struct SoundBankSample) * (*nsamples);
}

TbBool init_sound(void)
{
    SYNCDBG(8,"Starting");
    if (SoundDisabled)
      return false;
    struct SoundSettings* snd_settng = &game.sound_settings;
    SetupAudioOptionDefaults(snd_settng);
    snd_settng->flags = SndSetting_Sound;
    snd_settng->sound_type = 1622;
    snd_settng->sound_data_path = sound_dir;
    snd_settng->dir3 = sound_dir;
    snd_settng->field_12 = 1;
    snd_settng->stereo = 1;
    unsigned long i = get_best_sound_heap_size(mem_size);
    if (i < 1048576)
      snd_settng->max_number_of_samples = 10;
    else
      snd_settng->max_number_of_samples = 16;
    snd_settng->danger_music = 0;
    snd_settng->no_load_music = 1;
    snd_settng->no_load_sounds = 1;
    snd_settng->field_16 = 0;
    snd_settng->field_18 = 1;
    snd_settng->redbook_enable = IsRedbookMusicActive();
    snd_settng->sound_system = 0;
    InitAudio(snd_settng);
    sdl_flags = InitialiseSDLAudio();
    InitializeMusicPlayer();
    if (!GetSoundInstalled())
    {
      SoundDisabled = 1;
      return false;
    }
    S3DInit();
    S3DSetNumberOfSounds(snd_settng->max_number_of_samples);
    S3DSetMaximumSoundDistance(5120);
    return true;
}

TbBool init_sound_heap_two_banks(unsigned char *heap_mem, long heap_size, char *snd_fname, char *spc_fname, long a5)
{
    SYNCDBG(8,"Starting");
    LbMemorySet(heap_mem, 0, heap_size);
    using_two_banks = 0;
    // Open first sound bank and prepare sample table
    if (sound_file != -1)
        close_sound_bank(0);
    samples_in_bank = 0;
    sound_file = LbFileOpen(snd_fname,Lb_FILE_MODE_READ_ONLY);
    if (sound_file == -1)
    {
        ERRORLOG("Couldn't open primary sound bank file \"%s\"",snd_fname);
        return false;
    }
    unsigned char* buf = heap_mem;
    long buf_len = heap_size;
    long i = parse_sound_file(sound_file, buf, &samples_in_bank, buf_len, a5);
    if (i == 0)
    {
        ERRORLOG("Couldn't parse sound bank file \"%s\"",snd_fname);
        close_sound_heap();
        return false;
    }
    sample_table = (struct SampleTable *)buf;
    buf_len -= i;
    buf += i;
    if (buf_len <= 0)
    {
        ERRORLOG("Sound bank buffer too short");
        close_sound_heap();
        return false;
    }
    // Open second sound bank and prepare sample table
    if (sound_file2 != -1)
        close_sound_bank(1);
    samples_in_bank2 = 0;
    sound_file2 = LbFileOpen(spc_fname,Lb_FILE_MODE_READ_ONLY);
    if (sound_file2 == -1)
    {
        ERRORLOG("Couldn't open secondary sound bank file \"%s\"",spc_fname);
        return false;
    }
    i = parse_sound_file(sound_file2, buf, &samples_in_bank2, buf_len, a5);
    if (i == 0)
    {
        ERRORLOG("Couldn't parse sound bank file \"%s\"",spc_fname);
        close_sound_heap();
        return false;
    }
    sample_table2 = (struct SampleTable *)buf;
    buf_len -= i;
    buf += i;
    if (buf_len <= 0)
    {
        ERRORLOG("Sound bank buffer too short");
        close_sound_heap();
        return false;
    }
    SYNCLOG("Got sound buffer of %ld bytes, samples in banks: %d,%d",buf_len,(int)samples_in_bank,(int)samples_in_bank2);
    using_two_banks = 1;
    return true;
}

struct Thing *create_ambient_sound(const struct Coord3d *pos, ThingModel model, PlayerNumber owner)
{
    if ( !i_can_allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots) )
    {
        ERRORDBG(3,"Cannot create ambient sound %d for player %d. There are too many things allocated.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    struct Thing* thing = allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots);
    if (thing->index == 0) {
        ERRORDBG(3,"Should be able to allocate ambient sound %d for player %d, but failed.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing->class_id = TCls_AmbientSnd;
    thing->model = model;
    thing->parent_idx = thing->index;
    memcpy(&thing->mappos,pos,sizeof(struct Coord3d));
    thing->owner = owner;
    thing->rendering_flags |= TRF_Unknown01;
    add_thing_to_its_class_list(thing);
    return thing;
}

TbBool ambient_sound_prepare(void)
{
    struct Coord3d pos;
    memset(&pos,0,sizeof(struct Coord3d)); // ambient sound position
    struct Thing* thing = create_ambient_sound(&pos, 1, game.neutral_player_num);
    if (thing_is_invalid(thing))
    {
        game.ambient_sound_thing_idx = 0;
        ERRORLOG("Could not create ambient sound object");
        return false;
    }
    game.ambient_sound_thing_idx = thing->index;
    return true;
}

TbBool ambient_sound_stop(void)
{
    struct Thing* thing = thing_get(game.ambient_sound_thing_idx);
    if (thing_is_invalid(thing))
    {
        return false;
    }
    if (thing->snd_emitter_id != 0)
    {
        S3DDestroySoundEmitterAndSamples(thing->snd_emitter_id);
        thing->snd_emitter_id = 0;
    }
    return true;
}

void sound_reinit_after_load(void)
{
    stop_all_things_playing_samples();
    if (SpeechEmitter != 0)
    {
        S3DDestroySoundEmitterAndSamples(SpeechEmitter);
        SpeechEmitter = 0;
    }
    if (Non3DEmitter != 0)
    {
        S3DDestroySoundEmitterAndSamples(Non3DEmitter);
        Non3DEmitter = 0;
    }
    ambient_sound_stop();
    init_messages();
    free_sound_chunks();
    for (unsigned int sample = 0; sample <= EXTERNAL_SOUNDS_COUNT; sample++)
    {
        char *sound = &game.loaded_sound[sample][0];
        if (sound[0] != '\0')
        {
            char *fname = prepare_file_fmtpath(FGrp_CmpgMedia,"%s", sound);
            Ext_Sounds[sample] = Mix_LoadWAV(fname);
            if (Ext_Sounds[sample] != NULL)
            {
                Mix_VolumeChunk(Ext_Sounds[sample], settings.sound_volume);
                SYNCLOG("Loaded sound file %s into slot %u.", fname, sample);
                game.sounds_count++;
            }
            else
            {
                ERRORLOG("Could not reload sound %s (slot %u): %s", fname, sample, Mix_GetError());
            }
        }
    }
}

void stop_thing_playing_sample(struct Thing *thing, short smpl_idx)
{
    unsigned char eidx = thing->snd_emitter_id;
    if (eidx > 0)
    {
        if (S3DEmitterIsPlayingSample(eidx, smpl_idx, 0)) {
            S3DDeleteSampleFromEmitter(eidx, smpl_idx, 0);
        }
    }
}

void mute_audio(TbBool mute)
{
    if (!SoundDisabled)
    {
        if (mute)
        {
            SetSoundMasterVolume(0);
            SetMusicPlayerVolume(0);
            SetMusicMasterVolume(0);
            if (IsRedbookMusicActive())
            {
                PauseRedbookTrack(); // volume seems to have no effect on CD audio, so just pause/resume it
            }
        }
        else
        {
            SetMusicPlayerVolume(settings.redbook_volume);
            SetSoundMasterVolume(settings.sound_volume);
            SetMusicMasterVolume(settings.sound_volume);
            if (IsRedbookMusicActive())
            {
                ResumeRedbookTrack();
            }
        }
    }
}

void pause_music(TbBool pause)
{
    if (!SoundDisabled)
    {
        if (pause)
        {
            PauseMusicPlayer();
        }
        else
        {
            ResumeMusicPlayer();
        }
    }
}

void update_first_person_object_ambience(struct Thing *thing)
{
      if (thing_is_invalid(thing))
        return;
    struct Thing *objtng;
    MapCoordDelta new_distance;
    struct Thing *audtng;
    ThingIndex nearest_sounds[3];
    MapCoordDelta sound_distances[3];
    long hearing_range;
    struct ObjectConfigStats* objst;
    if (thing->class_id == TCls_Creature)
    {
        struct CreatureStats* crstat = creature_stats_get(thing->model);
        hearing_range = (long)subtile_coord(crstat->hearing, 0) / 2;
    }
    else
    {
        hearing_range = 2560;
    }
    sound_distances[0] = hearing_range;
    sound_distances[1] = hearing_range;
    sound_distances[2] = hearing_range;
    int i;
    if (ambience_timer)
    {
        memset(nearest_sounds, 0, sizeof(nearest_sounds));
        for (objtng = thing_get(get_list_for_thing_class(TCls_Object)->index);
             !thing_is_invalid(objtng);
             objtng = thing_get(objtng->next_of_class))
        {
            objst = get_object_model_stats(objtng->model);
            if ((objst->fp_smpl_idx != 0) && !thing_is_picked_up(objtng))
            {
                new_distance = get_chessboard_distance(&thing->mappos, &objtng->mappos);
                if (new_distance <= hearing_range)
                {
                    if (new_distance <= sound_distances[0])
                    {
                        for (i = 2; i > 0; i --)
                        {
                            MapCoordDelta dist = sound_distances[i-1];
                            nearest_sounds[i] = nearest_sounds[i-1];
                            sound_distances[i] = dist;
                        }
                        sound_distances[0] = new_distance;
                        nearest_sounds[0] = objtng->index;
                    }
                }
                else
                {
                    stop_thing_playing_sample(objtng, objst->fp_smpl_idx);
                }
            }
        }
        for (i = 0; i < (sizeof(nearest_sounds) / sizeof(nearest_sounds[0])); i++)
        {
            audtng = thing_get(nearest_sounds[i]);
            if (!thing_is_invalid(audtng))
            {
                objst = get_object_model_stats(audtng->model);
                if (!S3DEmitterIsPlayingSample(audtng->snd_emitter_id, objst->fp_smpl_idx, 0))
                {
                    long volume = line_of_sight_2d(&thing->mappos, &audtng->mappos) ? FULL_LOUDNESS : 128;
                    thing_play_sample(audtng, objst->fp_smpl_idx, NORMAL_PITCH, -1, 3, 1, 2, volume);
                }
            }
        }
    }
    ambience_timer = (ambience_timer + 1) % 4;
}

int InitialiseSDLAudio()
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
    return flags;
}

void ShutDownSDLAudio()
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

void free_sound_chunks()
{
    Mix_HaltChannel(-1);
    for (int i = 0; i <= EXTERNAL_SOUNDS_COUNT; i++)
    {
        if (Ext_Sounds[i] != NULL)
        {
            Mix_FreeChunk(Ext_Sounds[i]);
            Ext_Sounds[i] = NULL;
        }
    }
    game.sounds_count = 0;
}

void play_external_sound_sample(unsigned char smpl_id)
{
    if (Mix_PlayChannel(-1, Ext_Sounds[smpl_id], 0) == -1)
    {
        ERRORLOG("Could not play sound %s: %s", &game.loaded_sound[smpl_id][0], Mix_GetError());
    }
}

TbBool play_streamed_sample(char* fname, int volume, int loops)
{
    if (!SoundDisabled)
    {
        if (streamed_sample != NULL)
        {
            WARNLOG("Overwriting loaded sample.");
            stop_streamed_sample();
        }
        streamed_sample = Mix_LoadWAV(fname);
        if (streamed_sample != NULL)
        {
            Mix_VolumeChunk(streamed_sample, volume);
            if (Mix_PlayChannel(DESCRIPTION_CHANNEL, streamed_sample, loops) == -1)
            {
                ERRORLOG("Could not play sound %s: %s", fname, Mix_GetError());
                return false;
            }
        }
        else
        {
            ERRORLOG("Could not load sound %s: %s", fname, Mix_GetError());
            return false;
        }
        return true;
    }
    return false;
}

void stop_streamed_sample()
{
    Mix_HaltChannel(DESCRIPTION_CHANNEL);
    if (streamed_sample != NULL)
    {
        Mix_FreeChunk(streamed_sample);
        streamed_sample = NULL;
    }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
