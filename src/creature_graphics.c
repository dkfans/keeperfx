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

struct KeeperSprite *creature_table;
size_t creature_table_length = 0;

/******************************************************************************/
static const unsigned short creature_list[CREATURE_FRAMELIST_LENGTH] = {
    0, 30, 60, 65, 70, 95, 120, 125, 130, 134, 138, 140,
    142, 148, 154, 158, 162, 169, 176, 191, 206, 214, 222,
    232, 242, 272, 302, 307, 312, 337, 362, 367, 372, 376,
    380, 382, 384, 390, 396, 400, 404, 412, 420, 425, 430,
    438, 446, 447, 448, 456, 464, 472, 480, 488, 496, 504,
    512, 520, 528, 536, 544, 552, 560, 568, 576, 584, 592,
    600, 608, 616, 624, 630, 636, 640, 644, 652, 660, 668,
    676, 684, 692, 700, 708, 716, 724, 732, 740, 748, 756,
    764, 772, 780, 788, 796, 804, 808, 812, 862, 870, 878,
    886, 887, 888, 896, 904, 912, 920, 928, 936, 944, 952,
    973, 994, 998, 1006, 1022, 1038, 1054, 1070, 1086, 1102,
    1110, 1118, 1124, 1130, 1134, 1138, 1139, 1140, 1141,
    1142, 1150, 1158, 1159, 1160, 1161, 1162, 1166, 1170,
    1171, 1172, 1180, 1188, 1196, 1204, 1208, 1212, 1216,
    1220, 1224, 1228, 1229, 1230, 1234, 1238, 1239, 1240,
    1244, 1248, 1252, 1256, 1316, 1376, 1381, 1386, 1456,
    1526, 1531, 1536, 1540, 1544, 1569, 1594, 1600, 1606,
    1612, 1618, 1622, 1626, 1631, 1636, 1642, 1648, 1678,
    1708, 1713, 1718, 1743, 1768, 1773, 1778, 1779, 1780,
    1782, 1784, 1790, 1796, 1800, 1804, 1812, 1820, 1825,
    1830, 1834, 1838, 1868, 1898, 1903, 1908, 1933, 1958,
    1963, 1968, 1972, 1976, 1978, 1980, 1986, 1992, 1996,
    2000, 2008, 2016, 2021, 2026, 2030, 2034, 2064, 2094,
    2099, 2104, 2139, 2174, 2179, 2184, 2208, 2232, 2244,
    2256, 2262, 2268, 2272, 2276, 2284, 2292, 2297, 2302,
    2310, 2318, 2348, 2378, 2383, 2388, 2413, 2438, 2443,
    2448, 2452, 2456, 2458, 2460, 2466, 2472, 2476, 2480,
    2488, 2496, 2501, 2506, 2514, 2522, 2552, 2582, 2612,
    2642, 2667, 2692, 2697, 2702, 2706, 2710, 2712, 2714,
    2720, 2726, 2730, 2734, 2742, 2750, 2755, 2760, 2768,
    2776, 2806, 2836, 2841, 2846, 2871, 2896, 2901, 2906,
    2910, 2914, 2916, 2918, 2924, 2930, 2934, 2938, 2946,
    2954, 2959, 2964, 2972, 2980, 3010, 3040, 3045, 3050,
    3080, 3110, 3115, 3120, 3124, 3128, 3130, 3132, 3138,
    3144, 3148, 3152, 3158, 3164, 3169, 3174, 3182, 3190,
    3220, 3250, 3255, 3260, 3285, 3310, 3315, 3320, 3324,
    3328, 3330, 3332, 3338, 3344, 3349, 3354, 3362, 3370,
    3375, 3380, 3388, 3396, 3426, 3456, 3461, 3466, 3491,
    3516, 3521, 3526, 3530, 3534, 3536, 3538, 3544, 3550,
    3555, 3560, 3568, 3576, 3581, 3586, 3594, 3602, 3632,
    3662, 3667, 3672, 3702, 3732, 3737, 3742, 3746, 3750,
    3752, 3754, 3760, 3766, 3770, 3774, 3782, 3790, 3795,
    3800, 3808, 3816, 3846, 3876, 3881, 3886, 3911, 3936,
    3941, 3946, 3950, 3954, 3956, 3958, 3964, 3970, 3974,
    3978, 3986, 3994, 3999, 4004, 4012, 4020, 4050, 4080,
    4085, 4090, 4115, 4140, 4145, 4150, 4154, 4158, 4160,
    4162, 4168, 4174, 4178, 4182, 4190, 4198, 4203, 4208,
    4216, 4224, 4254, 4284, 4289, 4294, 4324, 4354, 4359,
    4364, 4368, 4372, 4374, 4376, 4382, 4388, 4392, 4396,
    4404, 4412, 4417, 4422, 4430, 4438, 4468, 4498, 4528,
    4558, 4588, 4618, 4623, 4628, 4634, 4640, 4646, 4652,
    4658, 4664, 4670, 4676, 4680, 4684, 4690, 4696, 4726,
    4756, 4761, 4766, 4796, 4826, 4831, 4836, 4840, 4844,
    4846, 4848, 4854, 4860, 4864, 4868, 4872, 4876, 4881,
    4886, 4894, 4902, 4932, 4962, 4967, 4972, 4997, 5022,
    5027, 5032, 5036, 5040, 5042, 5044, 5050, 5056, 5061,
    5066, 5070, 5074, 5079, 5084, 5092, 5100, 5125, 5150,
    5155, 5160, 5180, 5200, 5205, 5210, 5214, 5218, 5220,
    5222, 5227, 5232, 5236, 5240, 5248, 5256, 5281, 5306,
    5314, 5322, 5352, 5382, 5387, 5392, 5422, 5452, 5457,
    5462, 5472, 5482, 5512, 5542, 5572, 5602, 5642, 5682,
    5686, 5690, 5692, 5694, 5700, 5706, 5711, 5716, 5746,
    5776, 5781, 5786, 5826, 5866, 5871, 5876, 5880, 5884,
    5886, 5888, 5894, 5900, 5904, 5908, 5916, 5924, 5929,
    5934, 5942, 5950, 5980, 6010, 6015, 6020, 6045, 6070,
    6075, 6080, 6084, 6088, 6090, 6092, 6098, 6104, 6108,
    6112, 6120, 6128, 6133, 6138, 6146, 6154, 6184, 6214,
    6219, 6224, 6249, 6274, 6279, 6284, 6288, 6292, 6294,
    6296, 6302, 6308, 6312, 6316, 6324, 6332, 6337, 6342,
    6350, 6358, 6368, 6378, 6388, 6398, 6403, 6408, 6412,
    6416, 6418, 6420, 6426, 6432, 6436, 6440, 6448, 6456,
    6486, 6516, 6520, 6524, 6554, 6584, 6589, 6594, 6624,
    6654, 6659, 6664, 6668, 6672, 6674, 6676, 6682, 6688,
    6692, 6696, 6704, 6712, 6717, 6722, 6730, 6738, 6768,
    6798, 6803, 6808, 6833, 6858, 6863, 6868, 6872, 6876,
    6879, 6882, 6888, 6894, 6898, 6902, 6910, 6918, 6923,
    6928, 6936, 6944, 6974, 7004, 7009, 7014, 7039, 7064,
    7069, 7074, 7078, 7082, 7084, 7086, 7092, 7098, 7102,
    7106, 7114, 7122, 7127, 7132, 7140, 7148, 7178, 7208,
    7213, 7218, 7243, 7268, 7273, 7278, 7282, 7286, 7288,
    7290, 7296, 7302, 7306, 7310, 7311, 7312, 7317, 7322,
    7330, 7338, 7368, 7398, 7403, 7408, 7433, 7458, 7463,
    7468, 7472, 7476, 7478, 7480, 7486, 7492, 7512, 7532,
    7536, 7540, 7545, 7550, 7558, 7566, 7571, 7576, 7592,
    7608, 7624, 7640, 7644, 7648, 7656, 7660, 7661, 7669,
    7673, 7677, 7678, 7679, 7683, 7687, 7688, 7689, 7701,
    7713, 7714, 7722, 7738, 7754, 7770, 7786, 7798, 7799,
    7800, 7801, 7802, 7803, 7804, 7820, 7836, 7837, 7838,
    7839, 7840, 7841, 7842, 7858, 7859, 7889, 7904, 7934,
    7949, 7953, 7961, 7967, 7973, 7974, 7980, 7981, 7982,
    7986, 7994, 8002, 8010, 8025, 8030, 8035, 8040, 8045,
    8046, 8047, 8048, 8049, 8057, 8065, 8073, 8081, 8089,
    8097, 8103, 8105, 8117, 8129, 8141, 8153, 8161, 8169,
    8177, 8185, 8193, 8233, 8273, 8281, 8289, 8297, 8305,
    8312, 8320, 8329, 8336, 8344, 8353, 8360, 8368, 8377,
    8384, 8392, 8401, 8408, 8416, 8425, 8432, 8440, 8449,
    8456, 8464, 8473, 8480, 8488, 8497, 8513, 8529, 8534,
    8535, 8539, 8560, 8565, 8566, 8570, 8591, 8597, 8603,
    8611, 8619, 8620, 8621, 8629, 8633, 8641, 8645, 8654,
    8658, 8666, 8674, 8682, 8686, 8687, 8691, 8695, 8699,
    8703, 8743, 8783, 8803, 8823, 8839, 8847, 8855, 8859,
    8860, 8861, 8862, 8863, 8869, 8875, 8879, 8883, 8887,
    8891, 8895, 8899, 8903, 8907, 8911, 8915, 8916, 8917,
    8929, 8941, 8945, 8949, 8950, 8951, 8952, 8953, 8954,
    8955, 8956, 8957, 8958, 8959, 8960, 8961, 8965, 8969,
    8990, 9011, 9032, 9053, 9061, 9069, 9077, 9085, 9093,
    9101, 9109, 9117, 9125, 9133, 9141
};
/******************************************************************************/

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
    if (n < CREATURE_FRAMELIST_LENGTH)
    {
        const unsigned short i = creature_list[n];
        if (i < creature_table_length)
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
    if (n < CREATURE_FRAMELIST_LENGTH)
    {
        const unsigned short i = creature_list[n];
        if (i < creature_table_length)
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
    if (n < CREATURE_FRAMELIST_LENGTH)
    {
        const unsigned short i = creature_list[n];
        if (i < creature_table_length)
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
    if (n < CREATURE_FRAMELIST_LENGTH)
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
    if (kspr_frame >= 0 && kspr_frame < CREATURE_FRAMELIST_LENGTH)
    {
        const unsigned short i = creature_list[kspr_frame];
        if (i < creature_table_length) {
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

size_t creature_table_load_get_size(size_t disk_size)
{
    size_t items = disk_size / sizeof(struct KeeperSpriteDisk);
    if (items * sizeof(struct KeeperSpriteDisk) != disk_size)
    {
        ERRORLOG("Unexpected creature.tab");
    }
    creature_table_length = items;
    return items * sizeof(struct KeeperSprite);
}

void creature_table_load_unpack(unsigned char *src_buf, size_t disk_size)
{
    size_t items = disk_size / sizeof(struct KeeperSpriteDisk);
    struct KeeperSpriteDisk* src = (struct KeeperSpriteDisk*)src_buf;
    struct KeeperSprite *tmp = malloc(items * sizeof(struct KeeperSprite));
    for (int i = 0; i < items; i++, src++)
    {
        tmp[i].DataOffset = src->DataOffset;
        tmp[i].SWidth = src->SWidth;
        tmp[i].SHeight = src->SHeight;
        tmp[i].FrameWidth = src->FrameWidth;
        tmp[i].FrameHeight = src->FrameHeight;
        tmp[i].Rotable = src->Rotable;
        tmp[i].FramesCount = src->FramesCount;
        tmp[i].FrameOffsW = src->FrameOffsW;
        tmp[i].FrameOffsH = src->FrameOffsH;
        tmp[i].offset_x = src->offset_x;
        tmp[i].offset_y = src->offset_y;
        tmp[i].shadow_offset = 0;
        tmp[i].frame_flags = 0;
    }
    memcpy(src_buf, tmp, items * sizeof(struct KeeperSprite));
    free(tmp);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
