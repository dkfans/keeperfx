/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_graphics.c
 *     Creature graphics support functions.
 * @par Purpose:
 *     Functions to maintain and use creature graphics sprites.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 23 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "creature_graphics.h"
#include "bflib_dernc.h"
#include "globals.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "player_data.h"
#include "thing_creature.h"
#include "config_creature.h"
#include "creature_instances.h"
#include "creature_states.h"
#include "engine_lenses.h"
#include "engine_arrays.h"
#include "gui_draw.h"
#include "game_legacy.h"
#include "vidfade.h"
#include "keeperfx.hpp"
#include "engine_render.h"
#include "player_instances.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

struct KeeperSprite * creature_table = NULL;
unsigned short * creature_list = NULL;
TbSpriteData * keepsprite = NULL;
TbFileHandle jty_file_handle;
int total_keepersprites = 0;
int total_keepersprite_animations = 0;

/******************************************************************************/
struct PickedUpOffset *get_creature_picked_up_offset(struct Thing *thing)
{
    ThingModel crmodel = thing->model;
    if ((crmodel < 1) || (crmodel >= game.conf.crtr_conf.model_count))
        crmodel = 0;
    struct CreatureModelConfig* crconf = creature_stats_get(crmodel);
    return &crconf->creature_picked_up_offset;
}

unsigned char keepersprite_frames(unsigned short n)
{
    if (n >= KEEPERSPRITE_ADD_OFFSET && n < KEEPERSPRITE_ADD_OFFSET + KEEPERSPRITE_ADD_NUM)
    {
        return creature_table_add[n - KEEPERSPRITE_ADD_OFFSET].FramesCount;
    }
    if (n < total_keepersprite_animations)
    {
        const unsigned short i = creature_list[n];
        if (i < total_keepersprites)
        {
            return creature_table[i].FramesCount;
        }
    }
    ERRORLOG("Frame %u out of range", n);
    return 0;
}

unsigned char keepersprite_rotable(unsigned short n)
{
    if (n >= KEEPERSPRITE_ADD_OFFSET && n < KEEPERSPRITE_ADD_OFFSET + KEEPERSPRITE_ADD_NUM)
    {
        return creature_table_add[n - KEEPERSPRITE_ADD_OFFSET].Rotable;
    }
    if (n < total_keepersprite_animations)
    {
        const unsigned short i = creature_list[n];
        if (i < total_keepersprites)
        {
            return creature_table[i].Rotable;
        }
    }
    ERRORLOG("Frame %u out of range", n);
    return 0;
}

struct KeeperSprite * keepersprite_array(unsigned short n)
{
    if (n >= KEEPERSPRITE_ADD_OFFSET && n < KEEPERSPRITE_ADD_OFFSET + KEEPERSPRITE_ADD_NUM)
    {
        return &creature_table_add[n - KEEPERSPRITE_ADD_OFFSET];
    }
    if (n < total_keepersprite_animations)
    {
        const unsigned short i = creature_list[n];
        if (i < total_keepersprites)
        {
            return &creature_table[i];
        }
    }
    ERRORLOG("Frame %u out of range", n);
    return NULL;
}

unsigned long keepersprite_index(unsigned short n)
{
    if (n >= KEEPERSPRITE_ADD_OFFSET && n < KEEPERSPRITE_ADD_OFFSET + KEEPERSPRITE_ADD_NUM)
    {
        return n;
    }
    if (n < total_keepersprite_animations)
    {
        return creature_list[n];
    }
    ERRORLOG("Frame %u out of range", n);
    return 0;
}

long get_lifespan_of_animation(long ani, long speed)
{
    if (speed == 0)
    {
        WARNLOG("Animation %ld has no speed value", ani);
        return keepersprite_frames(ani);
    }
    return (keepersprite_frames(ani) << 8) / speed;
}

static struct KeeperSprite* sprite_by_frame(long kspr_frame)
{
    if (kspr_frame >= KEEPERSPRITE_ADD_OFFSET &&  kspr_frame < KEEPERSPRITE_ADD_OFFSET + KEEPERSPRITE_ADD_NUM)
    {
        return &creature_table_add[kspr_frame - KEEPERSPRITE_ADD_OFFSET];
    }
    if (kspr_frame >= 0 && kspr_frame < total_keepersprite_animations)
    {
        const unsigned short i = creature_list[kspr_frame];
        if (i < total_keepersprites) {
            return &creature_table[i];
        }
    }
    ERRORLOG("Frame %ld out of range", kspr_frame);
    return NULL;
}

void get_keepsprite_unscaled_dimensions(long kspr_anim, long angle, long frame, short *orig_w, short *orig_h, short *unsc_w, short *unsc_h)
{
    TbBool val_in_range;
    struct KeeperSprite* kspr = sprite_by_frame(kspr_anim);
    if (kspr == NULL)
    {
        ERRORLOG("[md10 crash investigation] NULL sprite returned for anim=%ld angle=%ld frame=%ld", kspr_anim, angle, frame);
        *orig_w = 0;
        *orig_h = 0;
        *unsc_w = 0;
        *unsc_h = 0;
        return;
    }
    if (((angle & ANGLE_MASK) <= DEGREES_202_5) || ((angle & ANGLE_MASK) >= DEGREES_337_5) )
        val_in_range = 0;
    else
        val_in_range = 1;
    if ( val_in_range )
      lbDisplay.DrawFlags |= Lb_SPRITE_FLIP_HORIZ;
    else
      lbDisplay.DrawFlags &= ~Lb_SPRITE_FLIP_HORIZ;
    if (kspr->Rotable == 0)
    {
        kspr += frame;
        *orig_w = kspr->FrameWidth;
        *orig_h = kspr->FrameHeight;
        if ( val_in_range )
        {
          *unsc_w = *orig_w - (long)kspr->SWidth - (long)kspr->FrameOffsW;
          *unsc_h = kspr->FrameOffsH;
        }
        else
        {
          *unsc_w = kspr->FrameOffsW;
          *unsc_h = kspr->FrameOffsH;
        }
    }
    else if (kspr->Rotable == 2)
    {
        kspr += frame + abs(4 - (((angle + DEGREES_22_5) & ANGLE_MASK) >> 8)) * kspr->FramesCount;
        *orig_w = kspr->SWidth;
        *orig_h = kspr->SHeight;
        if ( val_in_range )
        {
          *unsc_w = (long)kspr->FrameWidth - (long)kspr->FrameOffsW - *orig_w;
          *unsc_h = kspr->FrameOffsH;
        }
        else
        {
          *unsc_w = kspr->FrameOffsW;
          *unsc_h = kspr->FrameOffsH;
        }
    }
    *unsc_w += kspr->offset_x;
    *unsc_h += kspr->offset_y;
}

short get_creature_model_graphics(long crmodel, unsigned short seq_idx)
{
    if (seq_idx >= CREATURE_GRAPHICS_INSTANCES)
    {
        ERRORLOG("Invalid model %ld graphics sequence %u", crmodel, seq_idx);
        seq_idx = 0;
    }
    if ((crmodel < 0) || (crmodel >= game.conf.crtr_conf.model_count))
    {
        ERRORLOG("Invalid model %ld graphics sequence %u", crmodel, seq_idx);
        crmodel = 0;
    }
    // Backward compatibility for custom creatures. Use the attack animation if the extra animation is undefined, return 0 if the attack animation is also undefined.
    if (game.conf.crtr_conf.creature_graphics[crmodel][seq_idx] < 0)
    {
        if ((seq_idx >= CGI_CastSpell) && (game.conf.crtr_conf.creature_graphics[crmodel][CGI_Attack] > 0))
        {
            return game.conf.crtr_conf.creature_graphics[crmodel][CGI_Attack];
        }
        return 0;
    }
    return game.conf.crtr_conf.creature_graphics[crmodel][seq_idx];
}

void set_creature_model_graphics(long crmodel, unsigned short seq_idx, unsigned long val)
{
    if (seq_idx >= CREATURE_GRAPHICS_INSTANCES)
    {
        ERRORLOG("Invalid model %ld graphics sequence %u", crmodel, seq_idx);
        return;
    }
    if ((crmodel < 0) || (crmodel >= game.conf.crtr_conf.model_count))
    {
        ERRORLOG("Invalid model %ld graphics sequence %u", crmodel, seq_idx);
        return;
    }
    game.conf.crtr_conf.creature_graphics[crmodel][seq_idx] = val;
}

short get_creature_anim(struct Thing *thing, unsigned short seq_idx)
{
    short idx = get_creature_model_graphics(thing->model, seq_idx);
    return convert_td_iso(idx);
}

void untint_thing(struct Thing *thing)
{
    thing->tint_colour = 0;
    thing->rendering_flags &= ~(TRF_Tint_1|TRF_Tint_2);
}

void tint_thing(struct Thing *thing, TbPixel colour, unsigned char tint)
{
    thing->rendering_flags ^= (thing->rendering_flags ^ (tint << 2)) & (TRF_Tint_1|TRF_Tint_2);
    thing->tint_colour = colour;
}

TbBool update_creature_anim(struct Thing *thing, long speed, long seq_idx)
{
    unsigned long i = get_creature_anim(thing, seq_idx);
    // Only update when it's a different sprite, or a different animation speed.
    if (i != thing->anim_sprite)
    {
        set_thing_draw(thing, i, speed, -1, -1, 0, ODC_Default);
        return true;
    }
    if ((speed != thing->anim_speed) && (speed != -1))
    {
        thing->anim_speed = speed;
        return true;
    }
    return false;
}

TbBool update_creature_anim_td(struct Thing *thing, long speed, long td_idx)
{
    unsigned long i = convert_td_iso(td_idx);
    // Only update when it's a different sprite, or a different animation speed.
    if ((i != thing->anim_sprite) || ((speed != thing->anim_speed) && (speed != -1)))
    {
        set_thing_draw(thing, i, speed, -1, -1, 0, ODC_Default);
        return true;
    }
    return false;
}

void update_creature_rendering_flags(struct Thing *thing)
{
    // Clear related flags
    thing->rendering_flags &= ~TRF_Invisible;
    thing->rendering_flags &= ~TRF_Transpar_Flags;
    thing->rendering_flags &= ~TRF_AnimateOnce;
    // Now set only those that should be
    if ( (is_thing_directly_controlled_by_player(thing, my_player_number)) || (is_thing_passenger_controlled_by_player(thing, my_player_number)) )
    {
        thing->rendering_flags |= TRF_Invisible;
    }
    if (thing_is_creature(thing))
    {
        struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
        if (crconf->transparency_flags != 0)
        {
            set_flag(thing->rendering_flags, crconf->transparency_flags);
        }
    }
    if (creature_is_invisible(thing))
    {
      if (is_my_player_number(thing->owner))
      {
          thing->rendering_flags &= ~TRF_Transpar_Flags;
          thing->rendering_flags |= TRF_Transpar_4;
      } else
      {
            thing->rendering_flags |= TRF_Invisible;
            struct PlayerInfo* player = get_my_player();
            struct Thing* creatng = thing_get(player->influenced_thing_idx);
            if ((creatng != thing) && (thing_is_creature(creatng)))
            {
                if ( (is_thing_directly_controlled_by_player(creatng, player->id_number)) || (is_thing_passenger_controlled_by_player(creatng, player->id_number)) )
                {
                    if (creature_can_see_invisible(creatng))
                    {
                        thing->rendering_flags &= ~TRF_Invisible;
                        thing->rendering_flags &= ~TRF_Transpar_Flags;
                        thing->rendering_flags |= TRF_Transpar_4;
                    }
                }
            }
      }
    }
}

void update_creature_graphic_anim(struct Thing *thing)
{
    long i;

    TRACE_THING(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);

    if ((thing->size_change & TSC_ChangeSize) != 0)
    {
      thing->size_change &= ~TSC_ChangeSize;
    } else
    if ((thing->active_state == CrSt_CreatureHeroEntering) && (cctrl->countdown >= 0))
    {
      thing->rendering_flags |= TRF_Invisible;
    } else
    if (!creature_under_spell_effect(thing, CSAfF_Chicken))
    {
        if (cctrl->instance_id != CrInst_NULL)
        {
          if (cctrl->instance_id == CrInst_TORTURED)
          {
              thing->rendering_flags &= ~(TRF_Transpar_Flags);
          }
          struct InstanceInfo* inst_inf = creature_instance_info_get(cctrl->instance_id);
          update_creature_anim(thing, cctrl->instance_anim_step_turns, inst_inf->graphics_idx);
        } else
        if ((cctrl->frozen_on_hit != 0) || creature_is_dying(thing) || creature_under_spell_effect(thing, CSAfF_Freeze))
        {
            update_creature_anim(thing, 256, CGI_GotHit);
        } else
        if (flag_is_set(cctrl->stateblock_flags, CCSpl_ChickenRel))
        {
            update_creature_anim(thing, 256, CGI_Stand);
        } else
        if (thing->active_state == CrSt_CreatureSlapCowers)
        {
            update_creature_anim(thing, 256, CGI_GotSlapped);
        } else
        if (thing->active_state == CrSt_CreatureRoar)
        {
            update_creature_anim(thing, 128, CGI_Roar);
        } else
        if (thing->active_state == CrSt_CreaturePiss)
        {
            update_creature_anim(thing, 128, CGI_Piss);
        } else
        if (thing->active_state == CrSt_CreatureUnconscious)
        {
            update_creature_anim(thing, 64, CGI_DropDead);
            thing->rendering_flags |= TRF_AnimateOnce;
        } else
        if (thing->active_state == CrSt_CreatureSleep)
        {
            thing->rendering_flags &= ~(TRF_Transpar_Flags);
            update_creature_anim(thing, 128, CGI_Sleep);
        } else
        if (cctrl->distance_to_destination == 0)
        {
            update_creature_anim(thing, crconf->walking_anim_speed, CGI_Stand);
        } else
        if (thing->floor_height < thing->mappos.z.val)
        {
            i = (((long)cctrl->distance_to_destination) << 8) / (crconf->walking_anim_speed + 1);
            update_creature_anim(thing, i, CGI_Stand);
        } else
        if ((cctrl->dragtng_idx != 0) && (thing_get(cctrl->dragtng_idx)->state_flags & TF1_IsDragged1))
        {
            i = (((long)cctrl->distance_to_destination) << 8) / (crconf->walking_anim_speed+1);
            update_creature_anim(thing, i, CGI_Drag);
        } else
        if (crconf->fixed_anim_speed)
        {
            update_creature_anim(thing, 256, CGI_Ambulate);
        } else
        {
            i = (((long)cctrl->distance_to_destination) << 8) / (crconf->walking_anim_speed + 1);
            if (!update_creature_anim(thing, i, CGI_Ambulate))
            {
                thing->anim_speed = i;
            }
        }
    } else // chickened
    {
        thing->rendering_flags &= ~(TRF_Transpar_Flags);
        if (cctrl->distance_to_destination == 0)
        {
            update_creature_anim_td(thing, 256, 820);
        } else
        if (thing->floor_height < thing->mappos.z.val)
        {
            update_creature_anim_td(thing, 256, 820);
        } else
        if (crconf->fixed_anim_speed)
        {
            update_creature_anim_td(thing, 256, 819);
        } else
        {
            i = (((long)cctrl->distance_to_destination) << 8) / (crconf->walking_anim_speed+1);
            if (!update_creature_anim_td(thing, i, 819))
            {
                thing->anim_speed = i;
            }
        }
    }
}


void update_creature_graphic_tint(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_under_spell_effect(thing, CSAfF_Freeze))
    {
        tint_thing(thing, colours[4][4][15], 1);
    } else
    if (((cctrl->combat_flags & CmbtF_Melee) == 0) && ((cctrl->combat_flags & CmbtF_Ranged) == 0))
    {
        untint_thing(thing);
    } else
    if (((game.play_gameturn % 3) == 0) || is_hero_thing(thing))
    {
        untint_thing(thing);
    } else
    {
        tint_thing(thing, possession_hit_colours[get_player_color_idx(thing->owner)], 1);
    }
}

void set_creature_graphic(struct Thing *thing)
{
    update_creature_rendering_flags(thing);
    update_creature_graphic_anim(thing);
    // Update tint
    update_creature_graphic_tint(thing);
}

void unload_keepersprites(void)
{
    SYNCDBG(8,"Starting");
    if (keepsprite) {
        for (int i = 0; i < total_keepersprites; ++i)
        {
            free(keepsprite[i]);
        }
        free(keepsprite);
        keepsprite = NULL;
    }
    free(creature_list);
    creature_list = NULL;
    free(creature_table);
    creature_table = NULL;
    LbFileClose(jty_file_handle);
    jty_file_handle = NULL;
    total_keepersprites = 0;
    total_keepersprite_animations = 0;
}

static int count_creature_animations(void)
{
    int rotation_idx = 0;
    int animation_count = 0;
    for (int frame_idx = 0; frame_idx < total_keepersprites;) {
        const struct KeeperSprite* kspr = &creature_table[frame_idx];
        if (rotation_idx == 0) {
            rotation_idx = (kspr->Rotable > 0) ? 5 : 0;
            animation_count++;
        }
        if (rotation_idx > 0) {
            rotation_idx--;
        }
        if (kspr->FramesCount == 0) {
            frame_idx++;
        } else {
            frame_idx += kspr->FramesCount;
        }
    }
    return animation_count;
}

static TbBool populate_creature_list(void)
{
    total_keepersprite_animations = count_creature_animations();
    if (total_keepersprite_animations == 0) {
        return true;
    }
    creature_list = calloc(sizeof(creature_list[0]), total_keepersprite_animations);
    if (creature_list == NULL) {
        ERRORLOG("Out of memory");
        return false;
    }
    int rotation_idx = 0;
    int animation_idx = 0;
    for (int frame_idx = 0; frame_idx < total_keepersprites;) {
        const struct KeeperSprite* kspr = &creature_table[frame_idx];
        if (rotation_idx == 0) {
            creature_list[animation_idx] = frame_idx;
            rotation_idx = (kspr->Rotable > 0) ? 5 : 0;
            animation_idx++;
        }
        if (rotation_idx > 0) {
            rotation_idx--;
        }
        if (kspr->FramesCount == 0) {
            frame_idx++;
        } else {
            frame_idx += kspr->FramesCount;
        }
    }
    return true;
}

static TbBool load_keepersprite_metadata(void)
{
#ifdef SPRITE_FORMAT_V2
    const char * tab_fname = prepare_file_path(FGrp_StdData, "thingspr-32.tab");
#else
    const char * tab_fname = prepare_file_path(FGrp_StdData, "creature.tab");
#endif
    int tab_fsize = LbFileLengthRnc(tab_fname);
    if (tab_fsize > 0) {
        void * tab_buffer = malloc(tab_fsize);
        if (tab_buffer) {
            if (LbFileLoadAt(tab_fname, tab_buffer) == tab_fsize) {
                total_keepersprites = tab_fsize / sizeof(struct KeeperSpriteDisk);
                creature_table = calloc(sizeof(creature_table[0]), total_keepersprites);
                if (creature_table) {
                    for (int i = 0; i < total_keepersprites; ++i)
                    {
                        struct KeeperSprite * kspr = &creature_table[i];
                        const struct KeeperSpriteDisk * kspr_disk = &((struct KeeperSpriteDisk *) tab_buffer)[i];
                        kspr->DataOffset = kspr_disk->DataOffset;
                        kspr->SWidth = kspr_disk->SWidth;
                        kspr->SHeight = kspr_disk->SHeight;
                        kspr->FrameWidth = kspr_disk->FrameWidth;
                        kspr->FrameHeight = kspr_disk->FrameHeight;
                        kspr->Rotable = kspr_disk->Rotable;
                        kspr->FramesCount = kspr_disk->FramesCount;
                        kspr->FrameOffsW = kspr_disk->FrameOffsW;
                        kspr->FrameOffsH = kspr_disk->FrameOffsH;
                        kspr->offset_x = kspr_disk->offset_x;
                        kspr->offset_y = kspr_disk->offset_y;
                    }
                    keepsprite = calloc(sizeof(keepsprite[0]), total_keepersprites);
                    if (keepsprite) {
                        if (populate_creature_list()) {
                            return true;
                        }
                    } else {
                        ERRORLOG("Out of memory");
                    }
                } else {
                    ERRORLOG("Out of memory");
                }
            } else {
                ERRORLOG("Cannot load \"%s\"", tab_fname);
            }
            free(tab_buffer);
        } else {
            ERRORLOG("Out of memory");
        }
    } else {
        ERRORLOG("Cannot open \"%s\"", tab_fname);
    }
    unload_keepersprites();
    return false;
}

void prepare_keepersprites(void)
{
    unload_keepersprites();
    if (load_keepersprite_metadata()) {
#ifdef SPRITE_FORMAT_V2
    const char * jty_fname = prepare_file_path(FGrp_StdData, "thingspr-32.jty");
#else
    const char * jty_fname = prepare_file_path(FGrp_StdData, "creature.jty");
#endif
        jty_file_handle = LbFileOpen(jty_fname, Lb_FILE_MODE_READ_ONLY);
        if (jty_file_handle) {
            // The last frame is a dummy used by load_single_frame to figure out how big the frame is
            total_keepersprites = max(total_keepersprites - 1, 0);
            total_keepersprite_animations = max(total_keepersprite_animations - 1, 0);
            JUSTLOG("Found %d keeper sprites, %d animations.",
                total_keepersprites, total_keepersprite_animations);
            return;
        } else {
            ERRORLOG("Cannot open \"%s\"", jty_fname);
        }
    }
    unload_keepersprites();
}

static TbBool load_single_frame(int kspr_idx)
{
    const struct KeeperSprite * kspr = &creature_table[kspr_idx];
    const struct KeeperSprite * next_kspr = &creature_table[kspr_idx + 1];
    uint32_t size = next_kspr->DataOffset - kspr->DataOffset;
    if (size > 0) {
        keepsprite[kspr_idx] = malloc(size);
        if (keepsprite[kspr_idx]) {
            LbFileSeek(jty_file_handle, kspr->DataOffset, 0);
            if (LbFileRead(jty_file_handle, keepsprite[kspr_idx], size) == size) {
                return true;
            } else {
                ERRORLOG("Error reading data for keeper frame %d", kspr_idx);
            }
            free(keepsprite[kspr_idx]);
            keepsprite[kspr_idx] = NULL;
        } else {
            ERRORLOG("Out of memory");
        }
    } else {
        ERRORLOG("Attempted to load zero-sized frame %d", kspr_idx);
    }
    return false;
}

static TbBool load_keepersprite_if_needed(unsigned short first_frame_idx)
{
    const struct KeeperSprite * kspr = &creature_table[first_frame_idx];
    int num_frames = kspr->FramesCount;
    if (kspr->Rotable) {
        num_frames *= 5;
    }
    for (int frame_idx = 0; frame_idx < num_frames; frame_idx++)
    {
        const int spridx = first_frame_idx + frame_idx;
        if (keepsprite[spridx] == NULL)
        {
            if (!load_single_frame(spridx))
            {
                return false;
            }
        }
    }
    return true;
}

TbBool heap_manage_keepersprite(unsigned short kspr_idx)
{
    if (kspr_idx >= KEEPERSPRITE_ADD_OFFSET) return true; // custom sprites are already loaded
    return load_keepersprite_if_needed(kspr_idx);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
