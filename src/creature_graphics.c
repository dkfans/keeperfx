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

#include "globals.h"
#include "bflib_basics.h"

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

struct CreaturePickedUpOffset creature_picked_up_offset[] = {
  {  0,   0,  0,  0},
  {  6, 122,  0,  0},
  { 38, 134,  0,  0},
  {  0,  82,  0,  0},
  { -1,  69,  0,  0},
  { 14,  44,  0,  0},
  {  8,  64,  0,  0},
  { 14,  76,  0,  0},
  { 12,  50,  0,  0},
  {  6,  74,  0,  0},
  { 10,  90,  0,  0},
  {  8, 116,  0,  0},
  { 10, 102,  0,  0},
  {  4, 104,  0,  0},
  {  4, 128,  0,  0},
  { -5,  54,  0,  0},
  {  4,  96,  0,  0},
  { 14, 120,  0,  0},
  {  0,  50,  0,  0},
  { 14,  68,  0,  0},
  { -6, 126,  0,  0},
  { -8,  84,  0,  0},
  { -8,  76,  0,  0},
  { -2,  46,  0,  0},
  { 22,  60,  0,  0},
  {  0,  70,  0,  0},
  {  2,  44,  0,  0},
  {-12,  80,  0,  0},
  { -8,  60,  0,  0},
  {  0,  74,  0,  0},
  {  5, 121,  0,  0},
  {  0,   0,  0,  0},
};

/******************************************************************************/
DLLIMPORT unsigned short _DK_creature_list[CREATURE_FRAMELIST_LENGTH];
/******************************************************************************/
extern struct CreaturePickedUpOffset creature_picked_up_offset[];
/******************************************************************************/
struct CreaturePickedUpOffset *get_creature_picked_up_offset(struct Thing *thing)
{
    int crmodel = thing->model;
    if ((crmodel < 1) || (crmodel >= gameadd.crtr_conf.model_count))
        crmodel = 0;
    return &creature_picked_up_offset[crmodel];
}

unsigned char keepersprite_frames(unsigned short n)
{
  if ((n >= CREATURE_FRAMELIST_LENGTH && n < KEEPERSPRITE_ADD_OFFSET)
        || (n > KEEPERSPRITE_ADD_OFFSET + KEEPERSPRITE_ADD_NUM)
        )
  {
      ERRORLOG("Frame %d out of range",(int)n);
      n = 0;
  }
  else if (n >= KEEPERSPRITE_ADD_OFFSET)
  {
      return creature_table_add[n - KEEPERSPRITE_ADD_OFFSET].FramesCount;
  }
  unsigned long i = _DK_creature_list[n];
  return creature_table[i].FramesCount;
}

unsigned char keepersprite_rotable(unsigned short n)
{
    if ((n >= CREATURE_FRAMELIST_LENGTH && n < KEEPERSPRITE_ADD_OFFSET)
        || (n > KEEPERSPRITE_ADD_OFFSET + KEEPERSPRITE_ADD_NUM)
        )
    {
      ERRORLOG("Frame %d out of range",(int)n);
      n = 0;
    }
    else if (n >= KEEPERSPRITE_ADD_OFFSET)
    {
        return creature_table_add[n - KEEPERSPRITE_ADD_OFFSET].Rotable;
    }
    unsigned long i = _DK_creature_list[n];
    return creature_table[i].Rotable;
}

struct KeeperSprite * keepersprite_array(unsigned short n)
{
    if ((n >= CREATURE_FRAMELIST_LENGTH && n < KEEPERSPRITE_ADD_OFFSET)
        || (n > KEEPERSPRITE_ADD_OFFSET + KEEPERSPRITE_ADD_NUM)
        )
    {
        ERRORLOG("Frame %d out of range",(int)n);
        n = 0;
    }
    else if (n >= KEEPERSPRITE_ADD_OFFSET)
    {
        return &creature_table_add[n - KEEPERSPRITE_ADD_OFFSET];
    }
    unsigned long i = _DK_creature_list[n];
    return &creature_table[i];
}

unsigned long keepersprite_index(unsigned short n)
{
    if ((n >= CREATURE_FRAMELIST_LENGTH && n < KEEPERSPRITE_ADD_OFFSET)
        || (n > KEEPERSPRITE_ADD_OFFSET + KEEPERSPRITE_ADD_NUM)
        )
    {
      ERRORLOG("Frame %d out of range",(int)n);
      n = 0;
    }
    else if (n >= KEEPERSPRITE_ADD_OFFSET)
    {
        return n;
    }
    return _DK_creature_list[n];
}

long get_lifespan_of_animation(long ani, long frameskip)
{
    return (keepersprite_frames(ani) << 8) / frameskip;
}

static struct KeeperSprite* sprite_by_frame(long kspr_frame)
{
    if (kspr_frame >= KEEPERSPRITE_ADD_OFFSET)
    {
        return &creature_table_add[kspr_frame - KEEPERSPRITE_ADD_OFFSET];
    }
    unsigned long i = _DK_creature_list[kspr_frame];
    return &creature_table[i];
}

void get_keepsprite_unscaled_dimensions(long kspr_anim, long angle, long frame, short *orig_w, short *orig_h, short *unsc_w, short *unsc_h)
{
    TbBool val_in_range;
    struct KeeperSprite* kspr = sprite_by_frame(kspr_anim);
    if (((angle & 0x7FF) <= 1151) || ((angle & 0x7FF) >= 1919) )
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
    } else
    if (kspr->Rotable == 2)
    {
        kspr += frame + abs(4 - (((angle + 128) & 0x7FF) >> 8)) * kspr->FramesCount;
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
  if (seq_idx >= CREATURE_GRAPHICS_INSTANCES) {
      ERRORLOG("Invalid model %d graphics sequence %d",crmodel,seq_idx);
      seq_idx = 0;
  }
  if ((crmodel < 0) || (crmodel >= gameadd.crtr_conf.model_count)) {
      ERRORLOG("Invalid model %d graphics sequence %d",crmodel,seq_idx);
      crmodel = 0;
  }
  return gameadd.crtr_conf.creature_graphics[crmodel][seq_idx];
}

void set_creature_model_graphics(long crmodel, unsigned short seq_idx, unsigned long val)
{
    if (seq_idx >= CREATURE_GRAPHICS_INSTANCES) {
        ERRORLOG("Invalid model %d graphics sequence %d",crmodel,seq_idx);
        return;
    }
    if ((crmodel < 0) || (crmodel >= gameadd.crtr_conf.model_count)) {
        ERRORLOG("Invalid model %d graphics sequence %d",crmodel,seq_idx);
        return;
    }
    gameadd.crtr_conf.creature_graphics[crmodel][seq_idx] = val;
}

short get_creature_anim(struct Thing *thing, unsigned short seq_idx)
{
    short idx = get_creature_model_graphics(thing->model, seq_idx);
    return convert_td_iso(idx);
}

void untint_thing(struct Thing *thing)
{
    thing->tint_colour = 0;
    thing->rendering_flags &= ~(TRF_Unknown04|TRF_Unknown08);
}

void tint_thing(struct Thing *thing, TbPixel colour, unsigned char tint)
{
    thing->rendering_flags ^= (thing->rendering_flags ^ (tint << 2)) & (TRF_Unknown04|TRF_Unknown08);
    thing->tint_colour = colour;
}

TbBool update_creature_anim(struct Thing *thing, long speed, long seq_idx)
{
    unsigned long i = get_creature_anim(thing, seq_idx);
    if (i != thing->anim_sprite)
    {
        set_thing_draw(thing, i, speed, -1, -1, 0, 2);
        return true;
    }
    return false;
}

TbBool update_creature_anim_td(struct Thing *thing, long speed, long td_idx)
{
    unsigned long i = convert_td_iso(td_idx);
    if (i != thing->anim_sprite)
    {
        set_thing_draw(thing, i, speed, -1, -1, 0, 2);
        return true;
    }
    return false;
}

void update_creature_rendering_flags(struct Thing *thing)
{
    // Clear related flags
    thing->rendering_flags &= ~TRF_Unknown01;
    thing->rendering_flags &= ~TRF_Transpar_Flags;
    thing->rendering_flags &= ~TRF_Unmoving;
    // Now set only those that should be
    if ( (is_thing_directly_controlled_by_player(thing, my_player_number)) || (is_thing_passenger_controlled_by_player(thing, my_player_number)) )
    {
        thing->rendering_flags |= TRF_Unknown01;
    }
    if (creatures[thing->model].field_7)
    {
        thing->rendering_flags |= TRF_Transpar_Alpha;
    }
    if (creature_is_invisible(thing))
    {
      if (is_my_player_number(thing->owner))
      {
          thing->rendering_flags &= ~TRF_Transpar_Flags;
          thing->rendering_flags |= TRF_Transpar_4;
      } else
      {
            thing->rendering_flags |= TRF_Unknown01;
            struct PlayerInfo* player = get_my_player();
            struct Thing* creatng = thing_get(player->influenced_thing_idx);
            if (creatng != thing)
            {
                if ( (is_thing_directly_controlled_by_player(creatng, player->id_number)) || (is_thing_passenger_controlled_by_player(creatng, player->id_number)) )
                {
                    if (creature_can_see_invisible(creatng))
                    {
                        thing->rendering_flags &= ~TRF_Unknown01;
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
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);

    if ((thing->field_50 & 0x01) != 0)
    {
      thing->field_50 &= ~0x01;
    } else
    if ((thing->active_state == CrSt_CreatureHeroEntering) && (cctrl->countdown_282 >= 0))
    {
      thing->rendering_flags |= TRF_Unknown01;
    } else
    if (!creature_affected_by_spell(thing, SplK_Chicken))
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
        if ((cctrl->target_frozen_on_hit != 0) || creature_is_dying(thing) || creature_affected_by_spell(thing, SplK_Freeze))
        {
            update_creature_anim(thing, 256, 8);
        } else
        if ((cctrl->stateblock_flags & CCSpl_ChickenRel) != 0)
        {
            update_creature_anim(thing, 256, 0);
        } else
        if (thing->active_state == CrSt_CreatureSlapCowers)
        {
            update_creature_anim(thing, 256, 10);
        } else
        if ((thing->active_state == CrSt_CreaturePiss) || (thing->active_state == CrSt_CreatureRoar))
        {
            update_creature_anim(thing, 128, 4);
        } else
        if (thing->active_state == CrSt_CreatureUnconscious)
        {
            update_creature_anim(thing, 64, 16);
            thing->rendering_flags |= TRF_Unmoving;
        } else
        if (thing->active_state == CrSt_CreatureSleep)
        {
            thing->rendering_flags &= ~(TRF_Transpar_Flags);
            update_creature_anim(thing, 128, 12);
        } else
        if (cctrl->distance_to_destination == 0)
        {
            update_creature_anim(thing, 256, 0);
        } else
        if (thing->floor_height < thing->mappos.z.val)
        {
            update_creature_anim(thing, 256, 0);
        } else
        if ((cctrl->dragtng_idx != 0) && (thing_get(cctrl->dragtng_idx)->state_flags & TF1_IsDragged1))
        {
            i = (((long)cctrl->distance_to_destination) << 8) / (crstat->walking_anim_speed+1);
            update_creature_anim(thing, i, 2);
        } else
        if (creatures[thing->model].field_6 == 4)
        {
            update_creature_anim(thing, 256, 1);
        } else
        {
            i = (((long)cctrl->distance_to_destination) << 8) / (crstat->walking_anim_speed+1);
            if (!update_creature_anim(thing, i, 1))
            {
                thing->anim_speed = i;
            }
        }
    } else
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
        if (creatures[thing->model].field_6 == 4)
        {
            update_creature_anim_td(thing, 256, 819);
        } else
        {
            i = (((long)cctrl->distance_to_destination) << 8) / (crstat->walking_anim_speed+1);
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
    if (creature_affected_by_spell(thing, SplK_Freeze))
    {
        tint_thing(thing, colours[4][4][15], 1);
    } else
    if (((cctrl->combat_flags & CmbtF_Melee) == 0) && ((cctrl->combat_flags & CmbtF_Ranged) == 0))
    {
        untint_thing(thing);
    } else
    if ((game.play_gameturn % 3) == 0)
    {
        untint_thing(thing);
    } else
    {
        switch (thing->owner) //TODO: move player colors to array
        {
        case 0:
            tint_thing(thing, colours[15][0][0], 1);
            break;
        case 1:
            tint_thing(thing, colours[0][0][15], 1);
            break;
        case 2:
            tint_thing(thing, colours[0][15][0], 1);
            break;
        case 3:
            tint_thing(thing, colours[13][13][2], 1);
            break;
        default:
            untint_thing(thing);
            break;
        }
    }
}

void set_creature_graphic(struct Thing *thing)
{
    update_creature_rendering_flags(thing);
    update_creature_graphic_anim(thing);
    // Update tint
    update_creature_graphic_tint(thing);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
