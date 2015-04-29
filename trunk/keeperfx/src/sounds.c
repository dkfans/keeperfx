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
#include "sounds.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_sound.h"
#include "bflib_sndlib.h"
#include "bflib_fileio.h"
#include "bflib_memory.h"
#include "bflib_math.h"
#include "bflib_bufrw.h"
#include "bflib_heapmgr.h"
#include "map_utils.h"
#include "engine_camera.h"
#include "gui_soundmsgs.h"
#include "gui_topmsg.h"
#include "frontmenu_ingame_evnt.h"
#include "thing_data.h"
#include "thing_navigate.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "game_legacy.h"

#include "music_player.h"

#include "keeperfx.hpp"

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
/******************************************************************************/
DLLIMPORT TbFileHandle _DK_LbFileOpen(const char *fname, int mode);
DLLIMPORT int _DK_LbFileClose(TbFileHandle handle);
DLLIMPORT int _DK_LbFileSeek(TbFileHandle handle, long offset, int origin);
DLLIMPORT int _DK_LbFileRead(TbFileHandle handle, void *buffer, unsigned long len);
DLLIMPORT int _DK_LbFilePosition(TbFileHandle handle);
/******************************************************************************/
void thing_play_sample(struct Thing *thing, short smptbl_idx, unsigned short pitch, char a4, unsigned char a5, unsigned char a6, long a7, long loudness)
{
    struct Coord3d rcpos;
    long eidx;
    if (SoundDisabled)
        return;
    if (GetCurrentSoundMasterVolume() <= 0)
        return;
    if (thing_is_invalid(thing))
        return;
    rcpos.x.val = Receiver.pos.val_x;
    rcpos.y.val = Receiver.pos.val_y;
    rcpos.z.val = Receiver.pos.val_z;
    if (get_3d_box_distance(&rcpos, &thing->mappos) < MaxSoundDistance)
    {
        eidx = thing->snd_emitter_id;
        if (eidx > 0)
        {
            S3DAddSampleToEmitterPri(eidx, smptbl_idx, 0, pitch, loudness, a4, a5, a6 | 0x01, a7);
        } else
        {
            eidx = S3DCreateSoundEmitterPri(thing->mappos.x.val, thing->mappos.y.val, thing->mappos.z.val,
               smptbl_idx, 0, pitch, loudness, a4, a6 | 0x01, a7);
           thing->snd_emitter_id = eidx;
        }
    }
}

void play_thing_walking(struct Thing *thing)
{
    struct PlayerInfo *myplyr;
    myplyr = get_my_player();
    struct Camera *cam;
    cam = myplyr->acamera;
    { // Skip the thing if its distance to camera is too big
        MapSubtlDelta dist_x, dist_y;
        dist_x = coord_subtile(abs(cam->mappos.x.val - (MapCoordDelta)thing->mappos.x.val));
        dist_y = coord_subtile(abs(cam->mappos.y.val - (MapCoordDelta)thing->mappos.y.val));
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
    long loudness;
    loudness = ((myplyr->view_mode - 1) < 1u) ? (FULL_LOUDNESS) : (FULL_LOUDNESS/5);
    // Flying diptera has a buzzing noise sound
    if ((get_creature_model_flags(thing) & CMF_IsDiptera) && ((thing->movement_flags & TMvF_Flying) != 0) && (thing->field_60 < (int)thing->mappos.z.val))
    {
        if ( !S3DEmitterIsPlayingSample(thing->snd_emitter_id, 25, 0) ) {
            thing_play_sample(thing, 25, 100, -1, 2, 0, 2, loudness);
        }
    }
    else
    {
        if ( S3DEmitterIsPlayingSample(thing->snd_emitter_id, 25, 0) ) {
            S3DDeleteSampleFromEmitter(thing->snd_emitter_id, 25, 0);
        }
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        if ((cctrl->field_9) && get_foot_creature_has_down(thing))
        {
            int smpl_variant;
            smpl_variant = foot_down_sound_sample_variant[4 * ((cctrl->mood_flags & 0x1C) >> 2) + (cctrl->field_67 & 0x1F)];
            long smpl_idx;
            if ((thing->movement_flags & TMvF_Unknown80) != 0) {
                smpl_idx = 181 + smpl_variant;
            } else {
                struct CreatureSound *crsound;
                crsound = get_creature_sound(thing, CrSnd_Foot);
                smpl_idx = crsound->index + smpl_variant;
            }
            cctrl->field_67 = (cctrl->field_67 ^ (cctrl->field_67 ^ (cctrl->field_67 + 1))) & 0x1F;
            if ((cctrl->field_67 & 0x1F) >= 4)
            {
                cctrl->mood_flags &= ~0x1C;
                cctrl->mood_flags |=  (UNSYNC_RANDOM(4) << 2);
                cctrl->field_67 &= ~0x1F;
            }

            int v15;
            unsigned short smpl_delay;
            //TODO CONFIG creature model dependency; remove, add config file option for this
            v15 = thing->model;
            if ( v15 == 19 || v15 == 24 ) {//FLY or BUG
                smpl_delay = 400;
            } else
            if ( v15 == 27 ) {//HELL_HOUND
                smpl_delay = 300;
            } else {
                smpl_delay = 100;
            }
            thing_play_sample(thing, smpl_idx, smpl_delay, 0, 3, 3, 1, loudness);
            if ((thing->movement_flags & TMvF_IsOnWater) != 0) {
                thing_play_sample(thing, 21 + SOUND_RANDOM(4), 90 + SOUND_RANDOM(20), 0, 3, 3, 1, FULL_LOUDNESS);
            }
        }
    }
}

void set_room_playing_ambient_sound(struct Coord3d *pos, long sample_idx)
{
    struct Thing *thing;
    long i;
    if (game.ambient_sound_thing_idx == 0)
    {
        ERRORLOG("No room ambient sound object");
        return;
    }
    thing = thing_get(game.ambient_sound_thing_idx);
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
    struct PlayerInfo *player;
    struct Room *room;
    struct MapOffset *sstep;
    struct Coord3d pos;
    long slb_x,slb_y;
    MapSubtlCoord stl_x,stl_y;
    long i,k;
    SYNCDBG(8,"Starting");
    if ((SoundDisabled) || (GetCurrentSoundMasterVolume() <= 0))
        return;
    player = get_my_player();
    struct Camera *cam;
    cam = player->acamera;
    if (cam == NULL)
    {
        ERRORLOG("No active camera");
        set_room_playing_ambient_sound(NULL, 0);
        return;
    }
    slb_x = subtile_slab(cam->mappos.x.stl.num);
    slb_y = subtile_slab(cam->mappos.y.stl.num);
    for (i = 0; i < 11*11; i++)
    {
        sstep = &spiral_step[i];
        stl_x = slab_subtile_center(slb_x + sstep->h);
        stl_y = slab_subtile_center(slb_y + sstep->v);
        if (subtile_is_player_room(player->id_number,stl_x,stl_y))
        {
            room = subtile_room_get(stl_x, stl_y);
            if (room_is_invalid(room))
                continue;
            struct RoomConfigStats *roomst;
            roomst = &slab_conf.room_cfgstats[room->kind];
            k = roomst->ambient_snd_smp_id;
            if (k > 0)
            {
                SYNCDBG(8,"Playing ambient for %s at (%d,%d)",room_code_name(room->kind),(int)stl_x,(int)stl_y);
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

TbBool update_3d_sound_receiver(struct PlayerInfo *player)
{
    struct Camera *cam;
    SYNCDBG(7,"Starting");
    cam = player->acamera;
    if (cam == NULL)
        return false;
    S3DSetSoundReceiverPosition(cam->mappos.x.val,cam->mappos.y.val,cam->mappos.z.val);
    S3DSetSoundReceiverOrientation(cam->orient_a,cam->orient_b,cam->orient_c);
    return true;
}

void update_player_sounds(void)
{
    int k;
    struct PlayerInfo *player;
    SYNCDBG(7,"Starting");
    if (!(game.status_flags & Status_Paused))
    {
        player = get_my_player();
        process_messages();
        if (!SoundDisabled)
        {
            if (game.audiotrack > 0)
            {
                PlayMusicPlayer(game.audiotrack);
            }
            update_3d_sound_receiver(player);
        }
        game.play_gameturn++;
    }
    find_nearest_rooms_for_ambient_sound();
    process_3d_sounds();
    k = (game.bonus_time-game.play_gameturn) / 2;
    if (bonus_timer_enabled())
    {
      if ((game.bonus_time == game.play_gameturn) ||
         ((game.bonus_time > game.play_gameturn) && (((k <= 100) && ((k % 10) == 0)) ||
          ((k<=300) && ((k % 50) == 0)) || ((k % 250) == 0))) )
        play_non_3d_sample(89);
    }
    // Rare message easter egg
    if ((game.play_gameturn != 0) && ((game.play_gameturn % 20000) == 0))
    {
        if (ACTION_RANDOM(2000) == 0)
        {
          k = UNSYNC_RANDOM(10);
          SYNCDBG(9,"Rare message condition met, selected %d",(int)k);
          if (k == 7)
          {
            output_message(SMsg_PantsTooTight, 0, true);
          } else
          {
            output_message(SMsg_FunnyMessages+k, 0, true);
          }
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

void process_sound_heap(void)
{
    struct SampleInfo *smpinfo;
    struct SampleInfo *smpinfo_last;
    struct SampleTable *satab;
    struct HeapMgrHandle *hmhndl;
    long i;
    SYNCDBG(9,"Starting");
    for (i = 0; i < samples_in_bank; i++)
    {
        satab = &sample_table[i];
        hmhndl = satab->hmhandle;
        if (hmhndl != NULL) {
            hmhndl->flags &= ~0x0004;
            hmhndl->flags &= ~0x0002;
        }
    }
    if (using_two_banks)
    {
        for (i = 0; i < samples_in_bank2; i++)
        {
            satab = &sample_table2[i];
            hmhndl = satab->hmhandle;
            if (hmhndl != NULL) {
                hmhndl->flags &= ~0x0004;
                hmhndl->flags &= ~0x0002;
            }
        }
    }
    smpinfo_last = GetLastSampleInfoStructure();
    for (smpinfo = GetFirstSampleInfoStructure(); smpinfo <= smpinfo_last; smpinfo++)
    {
      if ( (smpinfo->field_0 != 0) && ((smpinfo->flags_17 & 0x01) != 0) )
      {
          if ( IsSamplePlaying(0, 0, smpinfo->field_0) )
          {
            if ( (using_two_banks) && ((smpinfo->flags_17 & 0x04) != 0) )
            {
                satab = &sample_table2[smpinfo->field_12];
                hmhndl = satab->hmhandle;
                if (hmhndl != NULL) {
                    hmhndl->flags |= 0x0004;
                    hmhndl->flags |= 0x0002;
                }
            } else
            {
                satab = &sample_table[smpinfo->field_12];
                hmhndl = satab->hmhandle;
                if (hmhndl != NULL) {
                    hmhndl->flags |= 0x0004;
                    hmhndl->flags |= 0x0002;
                }
            }
          } else
          {
              smpinfo->flags_17 &= ~0x01;
              smpinfo->flags_17 &= ~0x04;
          }
      }
    }
}

long parse_sound_file(TbFileHandle fileh, unsigned char *buf, long *nsamples, long buf_len, long a5)
{
    struct SoundBankHead bhead;
    struct SoundBankEntry bentries[9];
    struct SoundBankSample bsample;

    unsigned char rbuf[8];
    struct SampleTable *smpl;
    struct SoundBankEntry *bentry;
    long fsize;
    long i,k;

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
    _DK_LbFileSeek(fileh, 0, Lb_FILE_SEEK_END);
    fsize = _DK_LbFilePosition(fileh);
    _DK_LbFileSeek(fileh, fsize-4, Lb_FILE_SEEK_BEGINNING);
    _DK_LbFileRead(fileh, &rbuf, 4);
    i = read_int32_le_buf(rbuf);
    _DK_LbFileSeek(fileh, i, Lb_FILE_SEEK_BEGINNING);
    _DK_LbFileRead(fileh, &bhead, sizeof(bhead));
    _DK_LbFileRead(fileh, bentries, sizeof(bentries));
    bentry = &bentries[k];
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
    _DK_LbFileSeek(fileh, bentry->field_0, Lb_FILE_SEEK_BEGINNING);
    smpl = (struct SampleTable *)buf;
    k = bentry->field_4;
    for (i=0; i < *nsamples; i++)
    {
        _DK_LbFileRead(fileh, &bsample, sizeof(struct SoundBankSample));
        smpl->file_pos = k + bsample.field_12;
        smpl->data_size = bsample.data_size;
        smpl->sfxid = bsample.sfxid;
        smpl->hmhandle = NULL;
        smpl++;
    }
    //TODO SOUND Check why we're returning nsamples * 32 and not nsamples * 16
    return sizeof(struct SoundBankSample) * (*nsamples);
}

TbBool init_sound(void)
{
    struct SoundSettings *snd_settng;
    unsigned long i;
    SYNCDBG(8,"Starting");
    if (SoundDisabled)
      return false;
    snd_settng = &game.sound_settings;
    SetupAudioOptionDefaults(snd_settng);
    snd_settng->field_E = 3;
    snd_settng->sound_type = 1622;
    snd_settng->sound_data_path = sound_dir;
    snd_settng->dir3 = sound_dir;
    snd_settng->field_12 = 1;
    snd_settng->stereo = 1;
    i = get_best_sound_heap_size(mem_size);
    if (i < 1048576)
      snd_settng->max_number_of_samples = 10;
    else
      snd_settng->max_number_of_samples = 16;
    snd_settng->danger_music = 0;
    snd_settng->no_load_music = 0;
    snd_settng->no_load_sounds = 1;
    snd_settng->field_16 = 1;
    if ((game.flags_font & FFlg_UsrSndFont) == 0)
      snd_settng->field_16 = 0;
    snd_settng->field_18 = 1;
    snd_settng->redbook_enable = ((game.flags_cd & MFlg_NoCdMusic) == 0);
    snd_settng->sound_system = 0;
    InitAudio(snd_settng);
    LoadMusic(0);
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
    long i;
    long buf_len;
    unsigned char *buf;
    SYNCDBG(8,"Starting");
    LbMemorySet(heap_mem, 0, heap_size);
    using_two_banks = 0;
    // Open first sound bank and prepare sample table
    if (sound_file != -1)
        close_sound_bank(0);
    samples_in_bank = 0;
    sound_file = _DK_LbFileOpen(snd_fname,Lb_FILE_MODE_READ_ONLY);
    if (sound_file == -1)
    {
        ERRORLOG("Couldn't open primary sound bank file \"%s\"",snd_fname);
        return false;
    }
    buf = heap_mem;
    buf_len = heap_size;
    i = parse_sound_file(sound_file, buf, &samples_in_bank, buf_len, a5);
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
    sound_file2 = _DK_LbFileOpen(spc_fname,Lb_FILE_MODE_READ_ONLY);
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
    sndheap = heapmgr_init(buf, buf_len, samples_in_bank2 + samples_in_bank);
    if (sndheap == NULL)
    {
        ERRORLOG("Sound heap manager init error");
        close_sound_heap();
        return false;
    }
    using_two_banks = 1;
    return true;
}

void randomize_sound_font(void)
{
    StopMusic();
    switch (UNSYNC_RANDOM(3))
    {
    case 0:
        if (LoadAwe32Soundfont("bullfrog"))
          StartMusic(1, 127);
        break;
    case 1:
        if (LoadAwe32Soundfont("atmos1"))
          StartMusic(1, 127);
        break;
    case 2:
        if (LoadAwe32Soundfont("atmos2"))
          StartMusic(1, 127);
        break;
    }
}

struct Thing *create_ambient_sound(const struct Coord3d *pos, ThingModel model, PlayerNumber owner)
{
    struct Thing *thing;
    if ( !i_can_allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots) )
    {
        ERRORDBG(3,"Cannot create ambient sound %d for player %d. There are too many things allocated.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing = allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots);
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
    thing->field_4F |= 0x01;
    add_thing_to_its_class_list(thing);
    return thing;
}

TbBool ambient_sound_prepare(void)
{
    struct Thing *thing;
    struct Coord3d pos;
    memset(&pos,0,sizeof(struct Coord3d)); // ambient sound position
    thing = create_ambient_sound(&pos, 1, game.neutral_player_num);
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
    struct Thing *thing;
    thing = thing_get(game.ambient_sound_thing_idx);
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
    StartMusic(1, 127);
    init_messages();
    randomize_sound_font();
}

void stop_thing_playing_sample(struct Thing *heartng, short a2)
{
    unsigned char eidx;
    eidx = heartng->snd_emitter_id;
    if (eidx > 0)
    {
        if (S3DEmitterIsPlayingSample(eidx, a2, 0)) {
            S3DDeleteSampleFromEmitter(eidx, a2, 0);
        }
    }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
