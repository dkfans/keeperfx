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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
short creature_graphics[][22] = {
  {   0,   0,   0,   0,   0,   0,   0,  0,   0,  0,   0,
      0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,},
  { 426, 424, 424, 428,   0,   0,   0,  0, 430, 436, 442,
    438, 440, 444,  52, 432, 434, 946, 20, 164, 178,  20,},
  { 404, 402, 402, 406,   0,   0,   0,  0, 408, 414, 420,
    416, 418, 422,  62, 410, 412, 946, 21, 165, 180,  21,},
  { 382, 380, 380, 384,   0,   0,   0,  0, 386, 392, 398,
    394, 396, 400,  54, 388, 390, 946, 15, 168, 182,  24,},
  { 206, 204, 204, 208,   0,   0,   0,  0, 210, 216, 222,
    218, 220, 224,  48, 212, 214, 946, 15, 172, 184,  28,},
  { 360, 358, 358, 362,   0,   0,   0,  0, 364, 370, 376,
    372, 374, 378,  48, 366, 368, 946, 22, 166, 186,  22,},
  { 228, 226, 226, 230,   0,   0,   0,  0, 232, 238, 244,
    240, 242, 246,  60, 234, 236, 946, 18, 162, 188,  18,},
  { 162, 160, 160, 164,   0,   0,   0,  0, 166, 172, 178,
    174, 176, 180,  48, 168, 170, 946, 18, 214, 190,  18,},
  { 338, 336, 336, 340,   0,   0,   0,350, 342, 348, 354,
    350, 352, 356,  48, 344, 346, 946, 22, 167, 192,  23,},
  { 316, 314, 314, 318,   0,   0,   0,  0, 320, 326, 332,
    328, 330, 334,  48, 322, 324, 946, 26, 170, 194,  26,},
  { 294, 292, 292, 296,   0,   0,   0,  0, 298, 304, 310,
    306, 308, 312,  48, 300, 302, 946, 15, 169, 196,  25,},
  { 272, 270, 270, 274,   0,   0,   0,  0, 276, 282, 288,
    284, 286, 290,  58, 278, 280, 946, 15, 161, 198,  17,},
  { 250, 248, 248, 252,   0,   0,   0,  0, 254, 260, 266,
    262, 264, 268,  64, 256, 258, 946, 19, 163, 200,  19,},
  {  26,  24,  24,  28,   0,   0,   0,  0,  30,  36,  42,
     38,  40,  44,  48,  32,  34, 946, 15, 171, 202,  27,},
  { 754, 752, 752, 756, 766,   0,   0,  0, 758, 764, 770,
    766, 768, 772,  48, 760, 762, 946,  1, 145, 204,   1,},
  { 732, 730, 730, 734,   0,   0,   0,  0, 736, 742, 748,
    744, 746, 750,  92, 738, 740, 946,  2, 146, 206,   2,},
  { 710, 708, 708, 712,   0,   0,   0,  0, 714, 720, 726,
    722, 724, 728,  90, 716, 718, 946,  3, 147, 208,   3,},
  { 688, 686, 686, 690,   0,   0,   0,  0, 692, 698, 704,
    700, 702, 706,  88, 694, 696, 946,  4, 148, 210,   4,},
  { 666, 664, 664, 668,   0,   0,   0,  0, 670, 676, 682,
    678, 680, 684,  86, 672, 674, 946,  5, 149, 212,   5,},
  { 644, 644, 644, 646,   0,   0,   0,  0, 648, 654, 660,
    656, 658, 662,  84, 650, 652, 946,  6, 150, 214,   6,},
  { 624, 622, 622, 626,   0,   0,   0,  0, 628, 634, 640,
    636, 638, 642,  82, 630, 632, 946,  7, 151, 216,   7,},
  { 602, 600, 600, 604,   0,   0,   0,  0, 606, 612, 618,
    614, 616, 620,  80, 608, 610, 946,  8, 152, 218,   8,},
  { 580, 578, 578, 582,   0,   0,   0,  0, 584, 590, 596,
    592, 594, 598,  78, 586, 588, 946,  9, 153, 220,   9,},
  { 556, 554, 566, 558, 558, 568, 562,564, 560, 574, 576,
    564, 556, 556,  56, 570, 572, 946, 10, 154, 222,  10,},
  { 534, 532, 532, 536,   0,   0,   0,  0, 538, 544, 550,
    546, 548, 552,  76, 540, 542, 946, 11, 155, 224,  11,},
  { 512, 510, 510, 514,   0,   0,   0,  0, 516, 522, 528,
    524, 526, 530,  50, 518, 520, 946, 12, 156, 226,  12,},
  { 490, 488, 488, 492,   0,   0,   0,  0, 494, 500, 506,
    502, 504, 508,  74, 496, 498, 946, 13, 157, 228,  13,},
  {   2,   0,   0,   4,  22,   0,   0,  0,   6,  12,  18,
     14,  16,  20,  48,   8,  10, 946, 15, 159, 230,  15,},
  { 470, 468, 468, 472,   0,   0,   0,  0, 474, 480, 486,
    482, 484, 470,  48, 476, 478, 946, 16, 160, 232,  16,},
  { 448, 446, 446, 450,   0,   0,   0,  0, 452, 458, 464,
    460, 462, 466,  68, 454, 456, 946, 14, 158, 234,  14,},
  { 184, 182, 182, 186,   0,   0,   0,  0, 188, 194, 200,
    196, 198, 202,  66, 190, 192, 946, 19, 173, 496,  29,},
  { 980, 980, 980, 980, 980,   0,   0,  0, 980, 980, 980,
    980, 980, 980, 980, 980, 980, 980,  0,   0,   0,   0,},
};

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
    if ((crmodel < 1) || (crmodel >= CREATURE_TYPES_COUNT))
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
        return &creature_table_add[kspr_frame];
    }
    unsigned long i = _DK_creature_list[kspr_frame];
    return &creature_table[i];
}

void get_keepsprite_unscaled_dimensions(long kspr_frame, long a2, long a3, short *orig_w, short *orig_h, short *unsc_w, short *unsc_h)
{
    TbBool val_in_range;
    struct KeeperSprite* kspr = sprite_by_frame(kspr_frame);
    if ( ((a2 & 0x7FF) <= 1151) || ((a2 & 0x7FF) >= 1919) )
        val_in_range = 0;
    else
        val_in_range = 1;
    if ( val_in_range )
      lbDisplay.DrawFlags |= Lb_SPRITE_FLIP_HORIZ;
    else
      lbDisplay.DrawFlags &= ~Lb_SPRITE_FLIP_HORIZ;
    if (kspr->Rotable == 0)
    {
        kspr += a3;
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
        kspr += a3 + abs(4 - (((a2 + 128) & 0x7FF) >> 8)) * kspr->FramesCount;
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
    *unsc_w += kspr->field_C;
    *unsc_h += kspr->field_E;

}

short get_creature_model_graphics(long crmodel, unsigned short seq_idx)
{
  if (seq_idx >= CREATURE_GRAPHICS_INSTANCES) {
      ERRORLOG("Invalid model %d graphics sequence %d",crmodel,seq_idx);
      seq_idx = 0;
  }
  if ((crmodel < 0) || (crmodel >= CREATURE_TYPES_COUNT)) {
      ERRORLOG("Invalid model %d graphics sequence %d",crmodel,seq_idx);
      crmodel = 0;
  }
  return creature_graphics[crmodel][seq_idx];
}

void set_creature_model_graphics(long crmodel, unsigned short seq_idx, unsigned long val)
{
    if (seq_idx >= CREATURE_GRAPHICS_INSTANCES) {
        ERRORLOG("Invalid model %d graphics sequence %d",crmodel,seq_idx);
        return;
    }
    if ((crmodel < 0) || (crmodel >= CREATURE_TYPES_COUNT)) {
        ERRORLOG("Invalid model %d graphics sequence %d",crmodel,seq_idx);
        return;
    }
    creature_graphics[crmodel][seq_idx] = val;
}

short get_creature_anim(struct Thing *thing, unsigned short seq_idx)
{
    short idx = get_creature_model_graphics(thing->model, seq_idx);
    return convert_td_iso(idx);
}

void untint_thing(struct Thing *thing)
{
    thing->field_51 = 0;
    thing->field_4F &= ~(TF4F_Unknown04|TF4F_Unknown08);
}

void tint_thing(struct Thing *thing, TbPixel colour, unsigned char tint)
{
    thing->field_4F ^= (thing->field_4F ^ (tint << 2)) & (TF4F_Unknown04|TF4F_Unknown08);
    thing->field_51 = colour;
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

void update_creature_graphic_field_4F(struct Thing *thing)
{
    // Clear related flags
    thing->field_4F &= ~TF4F_Unknown01;
    thing->field_4F &= ~TF4F_Transpar_Flags;
    thing->field_4F &= ~TF4F_Unknown40;
    // Now set only those that should be
    if ( (is_thing_directly_controlled_by_player(thing, my_player_number)) || (is_thing_passenger_controlled_by_player(thing, my_player_number)) )
    {
        thing->field_4F |= TF4F_Unknown01;
    }
    if (creatures[thing->model].field_7)
    {
        thing->field_4F |= TF4F_Transpar_Alpha;
    }
    if (creature_is_invisible(thing))
    {
      if (is_my_player_number(thing->owner))
      {
          thing->field_4F &= ~TF4F_Transpar_Flags;
          thing->field_4F |= TF4F_Transpar_4;
      } else
      {
            thing->field_4F |= TF4F_Unknown01;
            struct PlayerInfo* player = get_my_player();
            struct Thing* creatng = thing_get(player->influenced_thing_idx);
            if (creatng != thing)
            {
                if ( (is_thing_directly_controlled_by_player(creatng, player->id_number)) || (is_thing_passenger_controlled_by_player(creatng, player->id_number)) )
                {
                    if (creature_can_see_invisible(creatng))
                    {
                        thing->field_4F &= ~TF4F_Unknown01;
                        thing->field_4F &= ~TF4F_Transpar_Flags;
                        thing->field_4F |= TF4F_Transpar_4;
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
      thing->field_4F |= TF4F_Unknown01;
    } else
    if (!creature_affected_by_spell(thing, SplK_Chicken))
    {
        if (cctrl->instance_id != CrInst_NULL)
        {
          if (cctrl->instance_id == CrInst_TORTURED)
          {
              thing->field_4F &= ~(TF4F_Transpar_Flags);
          }
          struct InstanceInfo* inst_inf = creature_instance_info_get(cctrl->instance_id);
          update_creature_anim(thing, cctrl->instance_anim_step_turns, inst_inf->graphics_idx);
        } else
        if ((cctrl->field_B1 != 0) || creature_is_dying(thing) || creature_affected_by_spell(thing, SplK_Freeze))
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
            thing->field_4F |= TF4F_Unknown40;
        } else
        if (thing->active_state == CrSt_CreatureSleep)
        {
            thing->field_4F &= ~(TF4F_Transpar_Flags);
            update_creature_anim(thing, 128, 12);
        } else
        if (cctrl->distance_to_destination == 0)
        {
            update_creature_anim(thing, 256, 0);
        } else
        if (thing->field_60 < thing->mappos.z.val)
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
        thing->field_4F &= ~(TF4F_Transpar_Flags);
        if (cctrl->distance_to_destination == 0)
        {
            update_creature_anim_td(thing, 256, 820);
        } else
        if (thing->field_60 < thing->mappos.z.val)
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
    update_creature_graphic_field_4F(thing);
    update_creature_graphic_anim(thing);
    // Update tint
    update_creature_graphic_tint(thing);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
