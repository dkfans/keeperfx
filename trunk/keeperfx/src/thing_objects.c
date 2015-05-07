/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_objects.c
 *     Things of class 'object' handling functions.
 * @par Purpose:
 *     Functions to maintain object things in the game.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     05 Nov 2009 - 01 Jan 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_objects.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_math.h"
#include "bflib_sound.h"
#include "bflib_planar.h"

#include "config_strings.h"
#include "config_objects.h"
#include "config_terrain.h"
#include "config_creature.h"
#include "config_effects.h"
#include "thing_stats.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "thing_physics.h"
#include "power_hand.h"
#include "player_instances.h"
#include "map_data.h"
#include "map_columns.h"
#include "room_entrance.h"
#include "gui_topmsg.h"
#include "gui_soundmsgs.h"
#include "engine_arrays.h"
#include "sounds.h"
#include "creature_states_pray.h"
#include "game_legacy.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
long food_moves(struct Thing *objtng);
long food_grows(struct Thing *objtng);
long object_being_dropped(struct Thing *objtng);
TngUpdateRet object_update_dungeon_heart(struct Thing *heartng);
TngUpdateRet object_update_call_to_arms(struct Thing *objtng);
TngUpdateRet object_update_armour(struct Thing *objtng);
TngUpdateRet object_update_object_scale(struct Thing *objtng);
TngUpdateRet object_update_armour2(struct Thing *objtng);
TngUpdateRet object_update_power_sight(struct Thing *objtng);
TngUpdateRet object_update_power_lightning(struct Thing *objtng);

Thing_State_Func object_state_functions[] = {
    NULL,
    food_moves,
    food_grows,
    NULL,
    object_being_dropped,
    NULL,
};

Thing_Class_Func object_update_functions[] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    object_update_dungeon_heart,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    object_update_call_to_arms,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    object_update_armour,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    object_update_object_scale,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    object_update_armour2,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    object_update_power_sight,
    object_update_power_lightning,
    object_update_object_scale,
    object_update_object_scale,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

/**
 * Objects config array.
 *
 * Originally was named objects[].
 */
struct Objects objects_data[] = {
  {0, 0, 0, 0, 0,   0, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown0, 0}, //0
  {0, 0, 0, 0, 0, 930, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown3, 1}, //1 BARREL
  {0, 0, 1, 0, 1, 962, 0x0100,    0,    0, 300, 0, 1, 2, 0,  0, ObOC_Unknown2, 1}, //2 TORCH
  {0, 0, 0, 0, 0, 934, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 1}, //3 GOLD_CHEST
  {0, 0, 0, 0, 0, 950, 0x0100,    0,    0, 300, 0, 1, 2, 0,  0, ObOC_Unknown3, 1}, //4
  {0, 0, 0, 0, 0, 948, 0x0100,0x200,0x200, 525, 0, 0, 2, 0,  0, ObOC_Unknown0, 0}, //5 SOUL_CONTAINER
  {0, 0, 0, 0, 0, 934, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 1}, //6
  {0, 0, 0, 0, 1, 962, 0x0100,    0,    0, 300, 0, 1, 2, 0,  0, ObOC_Unknown2, 1}, //7 TORCHUN
  {0, 0, 0, 0, 0, 950, 0x0100,    0,    0, 300, 0, 1, 2, 0,  0, ObOC_Unknown3, 1}, //8
  {2, 0, 0, 0, 0, 893, 0x0008, 0x80, 0x80, 300, 0, 0, 2, 1,  0, ObOC_Unknown2, 1}, //9 CHICKEN_GRW
  {1, 0, 0, 0, 0, 819, 0x0100, 0x80, 0x80, 300, 0, 0, 2, 0,  0, ObOC_Unknown2, 1}, //10 CHICKEN_MAT
  {0, 0, 0, 0, 1, 777, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0}, //11
  {0, 0, 0, 0, 1, 777, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0}, //12
  {0, 0, 0, 0, 1, 777, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0}, //13
  {0, 0, 0, 0, 1, 777, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0}, //14
  {0, 0, 0, 0, 1, 777, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0}, //15
  {0, 0, 0, 0, 1, 777, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0}, //16
  {0, 0, 0, 0, 1, 777, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0}, //17
  {0, 0, 0, 0, 1, 777, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0}, //18
  {0, 0, 0, 0, 1, 777, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0}, //19
  {0, 0, 0, 0, 1, 777, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0}, //20
  {0, 0, 0, 0, 1, 777, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0}, //21
  {0, 0, 0, 0, 1, 777, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0},
  {0, 0, 0, 0, 1, 777, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0},
  {0, 0, 0, 0, 0,   0, 0x0100,    0,    0, 375, 0, 0, 2, 0,  0, ObOC_Unknown1, 0}, //24
  {0, 0, 0, 0, 0,   0, 0x0100,    0,    0, 300, 0, 0, 5, 0,  0, ObOC_Unknown0, 0}, //25
  {0, 0, 0, 0, 0, 789, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown2, 1}, //26 ANVIL
  {0, 0, 0, 0, 0, 796, 0x0100,    0,    0, 200, 0, 0, 2, 1,  0, ObOC_Unknown2, 1}, //27 PRISON_BAR
  {0, 0, 1, 0, 0, 791, 0x0100,    0,    0, 300, 0, 1, 2, 0,  0, ObOC_Unknown3, 1}, //28
  {0, 0, 0, 0, 0, 793, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown2, 1}, //29 GRAVE_STONE
  {0, 0, 0, 0, 0, 905, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown3, 1}, //30
  {0, 0, 1, 0, 0, 795, 0x0100,    0,    0, 375, 0, 0, 2, 1,  0, ObOC_Unknown2, 1}, //31 TRAINING_POST
  {0, 0, 1, 0, 0, 892, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown2, 1}, //32 TORTURE_SPIKE
  {0, 0, 0, 0, 0, 797, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown2, 1}, //33 TEMPLE_SPANGLE
  {0, 0, 0, 0, 0, 804, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown2, 1}, //34 POTION1
  {0, 0, 0, 0, 0, 806, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown2, 1}, //35
  {0, 0, 0, 0, 0, 808, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown2, 1}, //36
  {0, 0, 0, 0, 0, 782, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown2, 0}, //37 POWER_HAND
  {0, 0, 0, 0, 0, 783, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown2, 0}, //38 POWER_HAND_GRAB
  {0, 0, 0, 0, 0, 785, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown2, 0}, //39 POWER_HAND_WHIP
  {2, 0, 0, 0, 0, 894, 0x0100, 0x80, 0x80, 300, 0, 0, 2, 1,  0, ObOC_Unknown2, 1}, //40 CHICKEN_STB
  {2, 0, 0, 0, 0, 895, 0x00C0, 0x80, 0x80, 300, 0, 0, 2, 1,  0, ObOC_Unknown2, 1}, //41 CHICKEN_WOB
  {2, 0, 0, 0, 0, 896, 0x00C0, 0x80, 0x80, 300, 0, 0, 2, 1,  0, ObOC_Unknown2, 1}, //42 CHICKEN_CRK
  {0, 0, 0, 0, 0, 936, 0x0055, 0x80, 0x40, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 1}, //43 GOLDL
  {0, 0, 0, 0, 0, 810, 0x0100,    0,    0, 300, 0, 0, 6, 0,  0, ObOC_Unknown2, 0}, //44 SPINNING_KEY
  {0, 0, 0, 0, 0, 777, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0}, //45
  {0, 0, 0, 0, 0, 777, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0},
  {0, 0, 0, 0, 0, 777, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0},
  {0, 0, 0, 0, 0, 777, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0},
  {0, 0, 0, 0, 0, 776, 0x0100,    0,    0, 300, 3, 0, 2, 0,  0, ObOC_Unknown1, 0}, //49
  {0, 0, 0, 0, 0, 818, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown0, 0}, //50
  {0, 0, 1, 0, 0, 850, 0x0100,    0,    0, 144, 3, 0, 2, 0,  0, ObOC_Unknown0, 0}, //51
  {0, 0, 0, 0, 0, 936, 0x0080,    0,    0, 375, 0, 0, 2, 0,  0, ObOC_Unknown1, 1}, //52
  {0, 0, 0, 0, 0, 937, 0x0080,    0,    0, 375, 0, 0, 2, 0,  0, ObOC_Unknown1, 1}, //53
  {0, 0, 0, 0, 0, 938, 0x0080,    0,    0, 375, 0, 0, 2, 0,  0, ObOC_Unknown1, 1}, //54
  {0, 0, 0, 0, 0, 939, 0x0080,    0,    0, 375, 0, 0, 2, 0,  0, ObOC_Unknown1, 1}, //55
  {0, 0, 0, 0, 0, 940, 0x0080,    0,    0, 375, 0, 0, 2, 0,  0, ObOC_Unknown1, 1}, //56
  {0, 0, 0, 0, 0, 124, 0x0100,    0,    0, 300, 0, 0, 2, 0,  1, ObOC_Unknown2, 0}, //57 LAIR_WIZRD
  {0, 0, 0, 0, 0, 124, 0x0100,    0,    0, 300, 0, 0, 2, 0,  2, ObOC_Unknown2, 0}, //58 LAIR_BARBR
  {0, 0, 0, 0, 0, 124, 0x0100,    0,    0, 300, 0, 0, 2, 0,  3, ObOC_Unknown2, 0}, //59
  {0, 0, 0, 0, 0, 124, 0x0100,    0,    0, 300, 0, 0, 2, 0,  4, ObOC_Unknown2, 0}, //60
  {0, 0, 0, 0, 0, 124, 0x0100,    0,    0, 300, 0, 0, 2, 0,  5, ObOC_Unknown2, 0}, //61
  {0, 0, 0, 0, 0, 124, 0x0100,    0,    0, 300, 0, 0, 2, 0,  6, ObOC_Unknown2, 0}, //62
  {0, 0, 0, 0, 0, 124, 0x0100,    0,    0, 300, 0, 0, 2, 0,  7, ObOC_Unknown2, 0}, //63
  {0, 0, 0, 0, 0, 124, 0x0100,    0,    0, 300, 0, 0, 2, 0,  8, ObOC_Unknown2, 0}, //64
  {0, 0, 0, 0, 0, 124, 0x0100,    0,    0, 300, 0, 0, 2, 0,  9, ObOC_Unknown2, 0}, //65
  {0, 0, 0, 0, 0, 124, 0x0100,    0,    0, 300, 0, 0, 2, 0, 10, ObOC_Unknown2, 0}, //66
  {0, 0, 0, 0, 0, 124, 0x0100,    0,    0, 300, 0, 0, 2, 0, 11, ObOC_Unknown2, 0}, //67
  {0, 0, 0, 0, 0, 124, 0x0100,    0,    0, 300, 0, 0, 2, 0, 12, ObOC_Unknown2, 0}, //68
  {0, 0, 0, 0, 0, 124, 0x0100,    0,    0, 300, 0, 0, 2, 0, 13, ObOC_Unknown2, 0}, //69
  {0, 0, 0, 0, 0, 158, 0x0100,    0,    0, 300, 0, 0, 2, 0, 14, ObOC_Unknown2, 0}, //70
  {0, 0, 0, 0, 0, 156, 0x0100,    0,    0, 300, 0, 0, 2, 0, 15, ObOC_Unknown2, 0}, //71
  {0, 0, 0, 0, 0, 154, 0x0100,    0,    0, 300, 0, 0, 2, 0, 16, ObOC_Unknown2, 0}, //72
  {0, 0, 0, 0, 0, 152, 0x0100,    0,    0, 300, 0, 0, 2, 0, 17, ObOC_Unknown2, 0}, //73
  {0, 0, 0, 0, 0, 150, 0x0100,    0,    0, 300, 0, 0, 2, 0, 18, ObOC_Unknown2, 0}, //74
  {0, 0, 0, 0, 0, 148, 0x0100,    0,    0, 300, 0, 0, 2, 0, 19, ObOC_Unknown2, 0}, //75
  {0, 0, 0, 0, 0, 146, 0x0100,    0,    0, 300, 0, 0, 2, 0, 20, ObOC_Unknown2, 0}, //76
  {0, 0, 0, 0, 0, 144, 0x0100,    0,    0, 300, 0, 0, 2, 0, 21, ObOC_Unknown2, 0}, //77
  {0, 0, 0, 0, 0, 142, 0x0100,    0,    0, 300, 0, 0, 2, 0, 22, ObOC_Unknown2, 0}, //78
  {0, 0, 0, 0, 0, 152, 0x0100,    0,    0, 300, 0, 0, 2, 0, 23, ObOC_Unknown2, 0}, //79
  {0, 0, 0, 0, 0, 140, 0x0100,    0,    0, 300, 0, 0, 2, 0, 24, ObOC_Unknown2, 0}, //80
  {0, 0, 0, 0, 0, 138, 0x0100,    0,    0, 300, 0, 0, 2, 0, 25, ObOC_Unknown2, 0}, //81
  {0, 0, 0, 0, 0, 136, 0x0100,    0,    0, 300, 0, 0, 2, 0, 26, ObOC_Unknown2, 0}, //82
  {0, 0, 0, 0, 0, 134, 0x0100,    0,    0, 300, 0, 0, 2, 0, 27, ObOC_Unknown2, 0}, //83
  {0, 0, 0, 0, 0, 132, 0x0100,    0,    0, 300, 0, 0, 2, 0, 28, ObOC_Unknown2, 0}, //84 LAIR_GHOST
  {0, 0, 0, 0, 0, 128, 0x0100,    0,    0, 300, 0, 0, 2, 0, 29, ObOC_Unknown2, 0}, //85 LAIR_TENTC
  {0, 0, 1, 0, 0, 901, 0x0080,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown1, 0}, //86 SPECBOX_REVMAP
  {0, 0, 1, 0, 0, 901, 0x0080,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown1, 0},
  {0, 0, 1, 0, 0, 901, 0x0080,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown1, 0},
  {0, 0, 1, 0, 0, 901, 0x0080,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown1, 0},
  {0, 0, 1, 0, 0, 901, 0x0080,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown1, 0},
  {0, 0, 1, 0, 0, 901, 0x0080,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown1, 0},
  {0, 0, 1, 0, 0, 901, 0x0080,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown1, 0},
  {0, 0, 1, 0, 0, 901, 0x0080,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown1, 0},
  {0, 0, 1, 0, 0, 114, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0}, //94
  {0, 0, 1, 0, 0, 114, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0},
  {0, 0, 1, 0, 0, 114, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0},
  {0, 0, 1, 0, 0, 114, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0},
  {0, 0, 1, 0, 0, 114, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0},
  {0, 0, 1, 0, 0, 114, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0},
  {0, 0, 1, 0, 0, 114, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0}, //100
  {0, 0, 1, 0, 0, 114, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0},
  {0, 0, 1, 0, 0, 114, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0},
  {0, 0, 1, 0, 0, 114, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0},
  {0, 0, 1, 0, 0, 114, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0},
  {0, 0, 1, 0, 0, 114, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0},
  {0, 0, 1, 0, 0, 114, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown1, 0}, //106
  {0, 0, 1, 0, 0, 114, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown1, 0},
  {0, 0, 1, 0, 0, 114, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown1, 0},
  {0, 0, 1, 0, 0, 114, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown1, 0},
  {0, 0, 0, 0, 0, 789, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown1, 0}, //110
  {0, 0, 1, 0, 0, 798, 0x0100,    0,    0, 300, 3, 0, 2, 0,  0, ObOC_Unknown2, 0}, //111 HEARTFLAME_RED
  {0, 0, 1, 0, 0, 851, 0x0100,    0,    0, 300, 3, 0, 2, 0,  0, ObOC_Unknown0, 0}, //112 DISEASE
  {0, 0, 1, 0, 0, 130, 0x0080,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown2, 0}, //113 SCAVENGE_EYE
  {0, 0, 1, 0, 0,  98, 0x0080,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown2, 0}, //114 WORKSHOP_MACHINE
  {0, 0, 1, 0, 0, 102, 0x0080,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown2, 0}, //115 GURDFLAG_RED
  {0, 0, 1, 0, 0, 104, 0x0080,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown2, 0}, //116
  {0, 0, 1, 0, 0, 106, 0x0080,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown2, 0}, //117
  {0, 0, 1, 0, 0, 108, 0x0080,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown2, 0}, //118
  {0, 0, 1, 0, 0, 100, 0x0080,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown2, 0}, //119
  {0, 0, 1, 0, 0, 799, 0x0100,    0,    0, 300, 3, 0, 2, 0,  0, ObOC_Unknown2, 0}, //120 HEARTFLAME_BLUE
  {0, 0, 1, 0, 0, 800, 0x0100,    0,    0, 300, 3, 0, 2, 0,  0, ObOC_Unknown2, 0}, //121
  {0, 0, 1, 0, 0, 801, 0x0100,    0,    0, 300, 3, 0, 2, 0,  0, ObOC_Unknown2, 0}, //122 HEARTFLAME_YELLOW
  {0, 0, 0, 0, 0,   0, 0x0000,    0,    0,   0, 0, 0, 0, 0,  0, ObOC_Unknown0, 0}, //123 POWER_SIGHT
  {0, 0, 0, 0, 0,   0, 0x0000,    0,    0,   0, 0, 0, 0, 0,  0, ObOC_Unknown0, 0}, //124 POWER_LIGHTNG
  {0, 0, 0, 0, 0,  46, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown2, 0}, //125 TORTURER
  {0, 0, 0, 0, 0, 126, 0x0100,    0,    0, 300, 0, 0, 2, 0, 30, ObOC_Unknown2, 0}, //126 LAIR_ORC
  {0, 0, 0, 0, 1, 781, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown0, 0}, //127
  {4, 0, 1, 0, 1, 780, 0x0100,    0,    0, 500, 0, 0, 2, 1,  0, ObOC_Unknown0, 0}, //128
  {0, 0, 0, 0, 0, 952, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown3, 1}, //129
  {0, 0, 0, 0, 0, 954, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown3, 1}, //130
  {0, 0, 0, 0, 0, 956, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown3, 1}, //131
  {0, 0, 0, 0, 0, 958, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown3, 1}, //132
  {0, 0, 0, 0, 0, 960, 0x0100,    0,    0, 300, 0, 0, 2, 0,  0, ObOC_Unknown3, 1}, //133
  {0, 0, 0, 0, 1, 777, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0}, //134
  {0, 0, 0, 0, 1, 777, 0x0100,    0,    0, 300, 0, 0, 2, 1,  0, ObOC_Unknown1, 0}, //135
  {0, 0, 0, 0, 0,   0, 0x0000,    0,    0,   0, 0, 0, 0, 0,  0, ObOC_Unknown0, 0},
};

ThingModel object_to_special[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};

/** Guard flag objects model per player index. Originally named guard_post_objects.
 */
unsigned short player_guardflag_objects[] = {115, 116, 117, 118,  0, 119};
/** Dungeon Heart flame objects model per player index.
 */
unsigned short dungeon_flame_objects[] =    {111, 120, 121, 122,  0,   0};
unsigned short lightning_spangles[] = {83, 90, 91, 92, 0, 0};
unsigned short gold_hoard_objects[] = {52, 52, 53, 54, 55, 56};

struct CallToArmsGraphics call_to_arms_graphics[] = {
    {867, 868, 869},
    {873, 874, 875},
    {879, 880, 881},
    {885, 886, 887},
    {  0,   0,   0}
};

/******************************************************************************/
DLLIMPORT long _DK_food_moves(struct Thing *objtng);
DLLIMPORT long _DK_food_grows(struct Thing *objtng);
DLLIMPORT long _DK_object_update_armour(struct Thing *objtng);
DLLIMPORT long _DK_object_update_object_scale(struct Thing *objtng);
DLLIMPORT long _DK_object_update_armour2(struct Thing *objtng);
DLLIMPORT long _DK_object_update_power_sight(struct Thing *objtng);
DLLIMPORT long _DK_remove_gold_from_hoarde(struct Thing *objtng, struct Room *room, long amount);
DLLIMPORT long _DK_add_gold_to_hoarde(struct Thing *objtng, struct Room *room, long amount);
DLLIMPORT void _DK_set_call_to_arms_as_rebirthing(struct Thing *objtng);
DLLIMPORT void _DK_set_call_to_arms_as_dying(struct Thing *objtng);
DLLIMPORT void _DK_process_object_sacrifice(struct Thing *gldtng, long sacowner);
DLLIMPORT struct Thing * _DK_find_base_thing_on_mapwho_excluding_self(struct Thing *gldtng);
/******************************************************************************/
struct Thing *create_object(const struct Coord3d *pos, unsigned short model, unsigned short owner, long parent_idx)
{
    struct Objects *objdat;
    struct InitLight ilight;
    struct Thing *thing;
    long i,k;

    if (!i_can_allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots))
    {
        ERRORDBG(3,"Cannot create object model %d for player %d. There are too many things allocated.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing = allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots);
    if (thing->index == 0) {
        ERRORDBG(3,"Should be able to allocate object %d for player %d, but failed.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing->class_id = TCls_Object;
    thing->model = model;
    if (parent_idx == -1)
      thing->parent_idx = -1;
    else
      thing->parent_idx = parent_idx;
    LbMemoryCopy(&thing->mappos, pos, sizeof(struct Coord3d));
    struct ObjectConfig *objconf;
    objconf = get_object_model_stats2(model);
    objdat = get_objects_data_for_thing(thing);
    thing->clipbox_size_xy = objdat->size_xy;
    thing->clipbox_size_yz = objdat->size_yz;
    thing->solid_size_xy = objdat->size_xy;
    thing->field_5C = objdat->size_yz;
    thing->health = saturate_set_signed(objconf->health,16);
    thing->field_20 = objconf->field_4;
    thing->field_23 = 204;
    thing->field_24 = 51;
    thing->field_22 = 0;
    thing->movement_flags |= TMvF_Unknown08;

    set_flag_byte(&thing->movement_flags, TMvF_Unknown40, objconf->field_8);
    thing->owner = owner;
    thing->creation_turn = game.play_gameturn;

    if (!objdat->field_2)
    {
      i = convert_td_iso(objdat->field_5);
      k = 0;
    } else
    {
      i = convert_td_iso(objdat->field_5);
      k = -1;
    }
    set_thing_draw(thing, i, objdat->anim_speed, objdat->sprite_size_max, 0, k, objdat->field_11);
    set_flag_byte(&thing->field_4F, 0x02, objconf->field_5);
    set_flag_byte(&thing->field_4F, 0x01, objdat->field_3 & 0x01);
    set_flag_byte(&thing->field_4F, 0x10, objdat->field_F & 0x01);
    set_flag_byte(&thing->field_4F, 0x20, objdat->field_F & 0x02);
    thing->active_state = objdat->initial_state;
    if (objconf->ilght.field_0 != 0)
    {
        LbMemorySet(&ilight, 0, sizeof(struct InitLight));
        LbMemoryCopy(&ilight.mappos, &thing->mappos, sizeof(struct Coord3d));
        ilight.field_0 = objconf->ilght.field_0;
        ilight.field_2 = objconf->ilght.field_2;
        ilight.field_3 = objconf->ilght.field_3;
        ilight.is_dynamic = objconf->ilght.is_dynamic;
        thing->light_id = light_create_light(&ilight);
        if (thing->light_id == 0) {
            SYNCDBG(8,"Cannot allocate light to %s",thing_model_name(thing));
        }
    } else {
        thing->light_id = 0;
    }
    switch (thing->model)
    {
      case 5:
        thing->byte_14 = 1;
        light_set_light_minimum_size_to_cache(thing->light_id, 0, 56);
        break;
      case 33:
        set_flag_byte(&thing->field_4F, 0x10, false);
        set_flag_byte(&thing->field_4F, 0x20, true);
        break;
      case 3:
      case 6:
      case 43:
        thing->valuable.gold_stored = gold_object_typical_value(thing->model);
        break;
      case 49:
        i = get_free_hero_gate_number();
        if (i > 0)
        {
            thing->byte_13 = i;
        } else
        {
            thing->byte_13 = 0;
            ERRORLOG("Could not allocate number for hero gate");
        }
        break;
      default:
        break;
    }
    add_thing_to_its_class_list(thing);
    place_thing_in_mapwho(thing);
    return thing;
}

void destroy_food(struct Thing *thing)
{
    struct Room *room;
    struct Thing *efftng;
    struct Coord3d pos;
    PlayerNumber plyr_idx;
    SYNCDBG(8,"Starting");
    plyr_idx = thing->owner;
    if (game.neutral_player_num != plyr_idx) {
        struct Dungeon *dungeon;
        dungeon = get_dungeon(plyr_idx);
        dungeon->lvstats.chickens_wasted++;
    }
    efftng = create_effect(&thing->mappos, TngEff_Unknown49, plyr_idx);
    if (!thing_is_invalid(efftng)) {
        thing_play_sample(efftng, 112+UNSYNC_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    }
    pos.x.val = thing->mappos.x.val;
    pos.y.val = thing->mappos.y.val;
    pos.z.val = thing->mappos.z.val + 256;
    create_effect(&thing->mappos, TngEff_Unknown51, plyr_idx);
    create_effect(&pos, TngEff_Unknown07, plyr_idx);
    if (!is_neutral_thing(thing))
    {
        if (thing->word_13 == -1)
        {
          room = get_room_thing_is_on(thing);
          if (!room_is_invalid(room))
          {
            if ((room->kind == RoK_GARDEN) && (room->owner == thing->owner))
            {
                if (room->used_capacity > 0)
                  room->used_capacity--;
                thing->word_13 = game.food_life_out_of_hatchery;
            }
          }
        }
    }
    delete_thing_structure(thing, 0);
}

void destroy_object(struct Thing *thing)
{
    if (object_is_mature_food(thing))
    {
        destroy_food(thing);
    } else
    {
        delete_thing_structure(thing, 0);
    }
}

TbBool thing_is_object(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
      return false;
    if (thing->class_id != TCls_Object)
      return false;
    return true;
}

void change_object_owner(struct Thing *objtng, PlayerNumber nowner)
{
    //TODO make this function more advanced - switch object types and update dungeon and rooms for spellbook/workshop box/lair
    SYNCDBG(6,"Starting for %s, owner %d to %d",thing_model_name(objtng),(int)objtng->owner,(int)nowner);
    objtng->owner = nowner;
}

struct Objects *get_objects_data_for_thing(struct Thing *thing)
{
    unsigned int tmodel;
    tmodel = thing->model;
    if (tmodel >= OBJECT_TYPES_COUNT)
      return &objects_data[0];
    return &objects_data[tmodel];
}

struct Objects *get_objects_data(unsigned int tmodel)
{
    if (tmodel >= OBJECT_TYPES_COUNT)
        return &objects_data[0];
    return &objects_data[tmodel];
}

SpecialKind box_thing_to_special(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return 0;
    if ( (thing->class_id != TCls_Object) || (thing->model >= object_conf.object_types_count) )
        return 0;
    return object_conf.object_to_special_artifact[thing->model];
}

/**
 * Gives power kind associated with given spellbook thing.
 * @param thing The spellbook object thing.
 * @return Power kind, or 0 if the thing is not a spellbook object.
 */
PowerKind book_thing_to_power_kind(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return 0;
    if ( (thing->class_id != TCls_Object) || (thing->model >= object_conf.object_types_count) )
        return 0;
    return object_conf.object_to_power_artifact[thing->model];
}

TbBool thing_is_special_box(const struct Thing *thing)
{
    if (!thing_is_object(thing))
        return false;
    struct ObjectConfigStats *objst;
    objst = get_object_model_stats(thing->model);
    return (objst->genre == OCtg_SpecialBox);
}

TbBool thing_is_workshop_crate(const struct Thing *thing)
{
    if (!thing_is_object(thing))
        return false;
    struct ObjectConfigStats *objst;
    objst = get_object_model_stats(thing->model);
    return (objst->genre == OCtg_WrkshpBox);
}

TbBool thing_is_trap_crate(const struct Thing *thing)
{
    return (crate_thing_to_workshop_item_class(thing) == TCls_Trap);
}

TbBool thing_is_door_crate(const struct Thing *thing)
{
    return (crate_thing_to_workshop_item_class(thing) == TCls_Door);
}

TbBool thing_is_dungeon_heart(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return false;
    if (thing->class_id != TCls_Object)
        return false;
    struct ObjectConfig *objconf;
    objconf = get_object_model_stats2(thing->model);
    return (objconf->is_heart) != 0;
}

TbBool thing_is_mature_food(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return false;
    return (thing->class_id == TCls_Object) && (thing->model == 10);
}

TbBool thing_is_spellbook(const struct Thing *thing)
{
    if (!thing_is_object(thing))
        return false;
    struct ObjectConfigStats *objst;
    objst = get_object_model_stats(thing->model);
    return (objst->genre == OCtg_Spellbook);
}

TbBool thing_is_lair_totem(const struct Thing *thing)
{
    if (!thing_is_object(thing))
        return false;
    struct ObjectConfigStats *objst;
    objst = get_object_model_stats(thing->model);
    return (objst->genre == OCtg_LairTotem);
}

TbBool object_is_hero_gate(const struct Thing *thing)
{
  return (thing->model == 49);
}

TbBool object_is_infant_food(const struct Thing *thing)
{
  return (thing->model == 40) || (thing->model == 41) || (thing->model == 42);
}

TbBool object_is_growing_food(const struct Thing *thing)
{
  return (thing->model == 9);
}

TbBool object_is_mature_food(const struct Thing *thing)
{
  return (thing->model == 10);
}

TbBool object_is_gold(const struct Thing *thing)
{
    return object_is_gold_pile(thing) || object_is_gold_hoard(thing);
}

/**
 * Returns if given thing is a gold hoard.
 * Gold hoards may only exist in treasure rooms.
 * @param thing
 * @return
 */
TbBool object_is_gold_hoard(const struct Thing *thing)
{
    struct ObjectConfigStats *objst;
    objst = get_object_model_stats(thing->model);
    return (objst->genre == OCtg_GoldHoard);
}

TbBool object_is_gold_pile(const struct Thing *thing)
{
    switch (thing->model)
    {
      case 3: // Chest of gold
      case 6: // Pot of gold
      case 43: // Gold laying on the ground
      case 128: // Spinning coin
          return true;
      default:
          return false;
    }
}

TbBool object_is_gold_laying_on_ground(const struct Thing *thing)
{
    return (thing->model == 43);
}

/**
 * Returns if given thing is a guardpost flag.
 * @param thing
 * @return
 */
TbBool object_is_guard_flag(const struct Thing *thing)
{
    switch (thing->model)
    {
      case 115:
      case 116:
      case 117:
      case 118:
      case 119:
          return true;
      default:
          return false;
    }
}

/**
 * Returns if given object thing is a room equipment.
 * Equipment are the objects which room always has, put there for the lifetime of a room.
 * @param thing
 * @return
 */
TbBool object_is_room_equipment(const struct Thing *thing, RoomKind rkind)
{
    switch (rkind)
    {
    case RoK_ENTRANCE:
        // No objects
        return false;
    case RoK_TREASURE:
        // Candlestick
        return (thing->model == 28);
    case RoK_LIBRARY:
        // No objects
        return false;
    case RoK_PRISON:
        // Prison bar
        return (thing->model == 27);
    case RoK_TORTURE:
        // Spike and torturer
        return (thing->model == 32) || (thing->model == 125);
    case RoK_TRAINING:
        // Training post and torch
        return (thing->model == 31) || (thing->model == 2);
    case RoK_DUNGHEART:
        // Heart flames
        return (thing->model == 111) || (thing->model == 120) || (thing->model == 121) || (thing->model == 122);
    case RoK_WORKSHOP:
        // Workshop machine and anvil
        return (thing->model == 114) || (thing->model == 26);
    case RoK_SCAVENGER:
        // Scavenge eye and torch
        return (thing->model == 113) || (thing->model == 2);
    case RoK_TEMPLE:
        // Temple statue
        return (thing->model == 4);
    case RoK_GRAVEYARD:
        // Gravestone and torch
        return (thing->model == 29) || (thing->model == 2);
    case RoK_BARRACKS:
        // No break
    case RoK_GARDEN:
        // Torch
        return (thing->model == 2);
    case RoK_LAIR:
        // No objects
        return false;
    case RoK_BRIDGE:
        // No objects
        return false;
    case RoK_GUARDPOST:
        // Guard flags
        return object_is_guard_flag(thing);
    default:
        return false;
    }
}

/**
 * Returns if given object thing is a room inventory.
 * Inventory are the objects which can be stored in a room, but are movable and optional.
 * @param thing
 * @return
 */
TbBool object_is_room_inventory(const struct Thing *thing, RoomKind rkind)
{
    switch (rkind)
    {
    case RoK_TREASURE:
        return object_is_gold_hoard(thing);
    case RoK_LIBRARY:
        return thing_is_spellbook(thing) || thing_is_special_box(thing);
    case RoK_DUNGHEART:
        return thing_is_dungeon_heart(thing);
    case RoK_WORKSHOP:
        return thing_is_workshop_crate(thing);
    case RoK_GARDEN:
        return object_is_infant_food(thing) || object_is_growing_food(thing) || object_is_mature_food(thing);
    case RoK_LAIR:
        return thing_is_lair_totem(thing);
    default:
        return false;
    }
}

TbBool object_is_unaffected_by_terrain_changes(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return false;
    if (thing->class_id != TCls_Object)
        return false;
    struct ObjectConfigStats *objst;
    objst = get_object_model_stats(thing->model);
    return (objst->genre == OCtg_Power);
}

/**
 * Finds spellbook in a 3x3 subtiles area around given position.
 * Selects the spellbook which is nearest to center of given subtile.
 * @param stl_x Central search subtile X coord.
 * @param stl_y Central search subtile Y coord.
 * @return The nearest thing, or invalid thing if no match was found.
 */
struct Thing *get_spellbook_at_position(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    return get_object_around_owned_by_and_matching_bool_filter(
        subtile_coord_center(stl_x), subtile_coord_center(stl_y), -1, thing_is_spellbook);
}

/**
 * Finds special box in a 3x3 subtiles area around given position.
 * Selects the box which is nearest to center of given subtile.
 * @param stl_x Central search subtile X coord.
 * @param stl_y Central search subtile Y coord.
 * @return The nearest thing, or invalid thing if no match was found.
 */
struct Thing *get_special_at_position(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    return get_object_around_owned_by_and_matching_bool_filter(
        subtile_coord_center(stl_x), subtile_coord_center(stl_y), -1, thing_is_special_box);
}

/**
 * Finds crate box in a 3x3 subtiles area around given position.
 * Selects the box which is nearest to center of given subtile.
 * @param stl_x Central search subtile X coord.
 * @param stl_y Central search subtile Y coord.
 * @return The nearest thing, or invalid thing if no match was found.
 */
struct Thing *get_crate_at_position(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    return get_object_around_owned_by_and_matching_bool_filter(
        subtile_coord_center(stl_x), subtile_coord_center(stl_y), -1, thing_is_workshop_crate);
}

TbBool creature_remove_lair_totem_from_room(struct Thing *creatng, struct Room *room)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->lair_room_id != room->index)
    {
        ERRORLOG("Attempt to remove a lair which belongs to %s index %d from room index %d he didn't think he was in",thing_model_name(creatng),(int)creatng->index,(int)room->index);
        return false;
    }
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    TbBool result;
    result = true;
    // Remove lair from room capacity
    if (room->content_per_model[creatng->model] <= 0)
    {
        ERRORLOG("Attempt to remove a lair which belongs to %s index %d from room index %d not containing this creature model",thing_model_name(creatng),(int)creatng->index,(int)room->index);
        result = false;
    } else
    if ( room->used_capacity < crstat->lair_size)
    {
        ERRORLOG("Attempt to remove creature lair from room with too little used space");
        result = false;
    } else
    {
        room->used_capacity -= crstat->lair_size;
        room->content_per_model[creatng->model]--;
    }
    cctrl->lair_room_id = 0;
    //Remove the totem thing
    if (cctrl->lairtng_idx > 0)
    {
        struct Thing *lairtng;
        lairtng = thing_get(cctrl->lairtng_idx);
        TRACE_THING(lairtng);
        create_effect(&lairtng->mappos, imp_spangle_effects[creatng->owner], creatng->owner);
        delete_lair_totem(lairtng);
    }
    return result;
}

TbBool delete_lair_totem(struct Thing *lairtng)
{
    struct Thing *creatng;
    creatng = thing_get(lairtng->word_13);
    if (thing_is_creature(creatng)) {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(creatng);
        cctrl->lair_room_id = 0;
        cctrl->lairtng_idx = 0;
    } else {
        ERRORLOG("No totem owner");
    }
    delete_thing_structure(lairtng, 0);
    return true;
}

long food_moves(struct Thing *objtng)
{
    //return _DK_food_moves(objtng);
    struct Thing *near_creatng;
    struct Coord3d pos;
    pos.x.val = objtng->mappos.x.val;
    pos.y.val = objtng->mappos.y.val;
    pos.z.val = objtng->mappos.z.val;
    unsigned int snd_smplidx;
    snd_smplidx = 0;
    if (objtng->food.byte_17)
    {
        destroy_food(objtng);
        return -1;
    }
    TbBool dirct_ctrl;
    dirct_ctrl = is_thing_directly_controlled(objtng);
    if (dirct_ctrl)
    {
        if (objtng->food.byte_16 > 0)
        {
            objtng->food.byte_16--;
            return 1;
        }
    }
    struct Room *room;
    room = get_room_thing_is_on(objtng);
    if (!dirct_ctrl)
    {
      if (objtng->food.word_13 >= 0)
      {
        if (!room_is_invalid(room) && !is_neutral_thing(objtng) && (objtng->food.word_13 != -1))
        {
            if ((room->kind == RoK_GARDEN) && (room->owner == objtng->owner) && (room->total_capacity > room->used_capacity))
            {
                room->used_capacity++;
                objtng->food.word_13 = -1;
            }
        }
      }
      if (objtng->food.word_13 >= 0)
      {
        objtng->food.word_13--;
        if (objtng->food.word_13 <= 0)
        {
            struct Dungeon *dungeon;
            dungeon = get_dungeon(objtng->owner);
            dungeon->lvstats.chickens_wasted++;
            create_effect(&objtng->mappos, 0x33u, objtng->owner);
            create_effect(&objtng->mappos, 7u, objtng->owner);
            delete_thing_structure(objtng, 0);
            return -1;
        }
      }
    }
    TbBool has_near_creature;
    has_near_creature = false;
    if (!room_is_invalid(room) && (room->kind == 13) && (objtng->food.word_13 < 0))
    {
        if (room->hatch_gameturn == game.play_gameturn)
        {
            near_creatng = thing_get(room->hatchfield_1B);
        } else
        {
            room->hatch_gameturn = game.play_gameturn;
            near_creatng = get_nearest_thing_of_class_and_model_owned_by(pos.x.val, pos.y.val, -1, TCls_Creature, -1);
            if (!thing_is_invalid(near_creatng))
                room->hatchfield_1B = near_creatng->index;
        }
        has_near_creature = (thing_exists(near_creatng) && (get_2d_box_distance(&objtng->mappos, &near_creatng->mappos) < 768));
        if (has_near_creature)
        {
            objtng->food.word_18 = get_angle_xy_to(&near_creatng->mappos, &pos);
            if (objtng->snd_emitter_id == 0)
            {
                if (UNSYNC_RANDOM(16) == 0) {
                  snd_smplidx = 109 + UNSYNC_RANDOM(3);
                }
            }
        }
    }
    if (objtng->food.byte_15 <= 0)
    {
        if (objtng->food.byte_15 == 0)
        {
            objtng->food.byte_15 = -1;
            set_thing_draw(objtng, 820, -1, -1, -1, 0, 2u);
            if (dirct_ctrl) {
                objtng->food.byte_16 = 6;
            } else {
                objtng->food.byte_16 = ACTION_RANDOM(4) + 1;
            }
        }
        if ((has_near_creature && (objtng->food.byte_16 < 5)) || (objtng->food.byte_16 == 0))
        {
            set_thing_draw(objtng, 819, -1, -1, -1, 0, 2u);
            objtng->food.byte_15 = ACTION_RANDOM(0x39);
            objtng->food.word_18 = ACTION_RANDOM(0x7FF);
            objtng->food.byte_16 = 0;
        } else
        if ((objtng->field_3E * objtng->field_49 <= objtng->field_3E + objtng->field_40) && (objtng->food.byte_16 < 5))
        {
            objtng->food.byte_16--;
        }
    }
    else
    {
        int vel_x, vel_y;
        vel_x = 32 * LbSinL(objtng->food.word_18) >> 16;
        pos.x.val += vel_x;
        vel_y = -(32 * LbCosL(objtng->food.word_18) >> 8) >> 8;
        pos.y.val += vel_y;
        if (thing_in_wall_at(objtng, &pos))
        {
            objtng->food.word_18 = ACTION_RANDOM(0x7FF);
        }
        int sangle;
        long dangle;
        dangle = get_angle_difference(objtng->move_angle_xy, objtng->food.word_18);
        sangle = get_angle_sign(objtng->move_angle_xy, objtng->food.word_18);
        if (dangle > 62)
            dangle = 62;
        objtng->move_angle_xy = (objtng->move_angle_xy + dangle * sangle) & LbFPMath_AngleMask;
        if (get_angle_difference(objtng->move_angle_xy, objtng->food.word_18) < 284)
        {
            struct ComponentVector cvec;
            cvec.x = vel_x;
            cvec.y = vel_y;
            cvec.z = 0;
            objtng->food.byte_15--;
            apply_transitive_velocity_to_thing(objtng, &cvec);
        }
        if (objtng->snd_emitter_id == 0)
        {
            if (snd_smplidx > 0) {
              thing_play_sample(objtng, snd_smplidx, 100, 0, 3u, 0, 1, 256);
              return 1;
            }
            if (UNSYNC_RANDOM(0x50) == 0)
            {
              snd_smplidx = 100 + UNSYNC_RANDOM(9);
            }
        }
    }
    if (snd_smplidx > 0) {
        thing_play_sample(objtng, snd_smplidx, 100, 0, 3u, 0, 1, 256);
        return 1;
    }
    return 1;
}

long food_grows(struct Thing *objtng)
{
    //return _DK_food_grows(objtng);
    if (objtng->food.word_13 > 0)
    {
        objtng->food.word_13--;
        return 1;
    }
    struct Coord3d pos;
    pos.x.val = objtng->mappos.x.val;
    pos.y.val = objtng->mappos.y.val;
    pos.z.val = objtng->mappos.z.val;
    struct Thing *nobjtng;
    long ret;
    ret = 0;
    PlayerNumber tngowner;
    tngowner = objtng->owner;
    switch (objtng->anim_sprite)
    {
      case 893:
      case 897:
        delete_thing_structure(objtng, 0);
        nobjtng = create_object(&pos, 0x28u, tngowner, -1);
        if (!thing_is_invalid(nobjtng)) {
            nobjtng->food.word_13 = (nobjtng->field_49 << 8) / nobjtng->field_3E - 1;
        }
        ret = -1;
        break;
      case 894:
      case 898:
        delete_thing_structure(objtng, 0);
        nobjtng = create_object(&pos, 0x29u, tngowner, -1);
        if (!thing_is_invalid(nobjtng)) {
            nobjtng->food.word_13 = 3 * ((nobjtng->field_49 << 8) / nobjtng->field_3E - 1);
        }
        ret = -1;
        break;
      case 895:
      case 899:
        delete_thing_structure(objtng, 0);
        nobjtng = create_object(&pos, 0x2Au, tngowner, -1);
        if (!thing_is_invalid(nobjtng)) {
            nobjtng->food.word_13 = (nobjtng->field_49 << 8) / nobjtng->field_3E - 1;
        }
        ret = -1;
        break;
      case 896:
      case 900:
        delete_thing_structure(objtng, 0);
        nobjtng = create_object(&pos, 0xAu, tngowner, -1);
        if (!thing_is_invalid(nobjtng)) {
            nobjtng->move_angle_xy = ACTION_RANDOM(0x800);
            nobjtng->food.byte_15 = ACTION_RANDOM(0x6FF);
            nobjtng->food.byte_16 = 0;
          thing_play_sample(nobjtng, 80 + UNSYNC_RANDOM(3), 100, 0, 3u, 0, 1, 256);
          if (!is_neutral_thing(nobjtng)) {
              struct Dungeon *dungeon;
              dungeon = get_dungeon(nobjtng->owner);
              dungeon->lvstats.chickens_hatched++;
          }
          nobjtng->food.word_13 = -1;
        }
        ret = -1;
        break;
      default:
        break;
    }
    return ret;
}

GoldAmount add_gold_to_treasure_room_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, GoldAmount gold_store)
{
    struct Room *room;
    room = subtile_room_get(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    struct Thing *gldtng;
    gldtng = find_gold_hoard_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    if (thing_is_invalid(gldtng))
    {
        struct Coord3d pos;
        pos.x.val = subtile_coord_center(slab_subtile_center(slb_x));
        pos.y.val = subtile_coord_center(slab_subtile_center(slb_y));
        pos.z.val = get_floor_height_at(&pos);
        gldtng = create_gold_hoarde(room, &pos, gold_store);
        if (!thing_is_invalid(gldtng)) {
            gold_store -= gldtng->valuable.gold_stored;
        }
    } else
    {
        gold_store -= add_gold_to_hoarde(gldtng, room, gold_store);
    }
    return gold_store;
}

long gold_being_dropped_at_treasury(struct Thing *thing, struct Room *room)
{
    GoldAmount gold_store;
    gold_store = thing->valuable.gold_stored;
    MapSlabCoord slb_x, slb_y;
    {
        slb_x = coord_slab(thing->mappos.x.val);
        slb_y = coord_slab(thing->mappos.y.val);
        gold_store = add_gold_to_treasure_room_slab(slb_x, slb_y, gold_store);
    }
    SlabCodedCoords slbnum;
    long n;
    unsigned long k;
    n = ACTION_RANDOM(room->slabs_count);
    slbnum = room->slabs_list;
    for (k = n; k > 0; k--)
    {
        if (slbnum == 0)
            break;
        slbnum = get_next_slab_number_in_room(slbnum);
    }
    if (slbnum == 0) {
        ERRORLOG("Taking random slab (%d/%d) in %s index %d failed - internal inconsistency.",(int)n,(int)room->slabs_count,room_code_name(room->kind),(int)room->index);
        slbnum = room->slabs_list;
    }
    k = 0;
    while (1)
    {
        MapSlabCoord slb_x,slb_y;
        slb_x = slb_num_decode_x(slbnum);
        slb_y = slb_num_decode_y(slbnum);
        // Per slab code
        if (gold_store <= 0)
            break;
        gold_store = add_gold_to_treasure_room_slab(slb_x, slb_y, gold_store);
        // Per slab code ends
        slbnum = get_next_slab_number_in_room(slbnum);
        if (slbnum == 0) {
            slbnum = room->slabs_list;
        }
        k++;
        if (k >= room->slabs_count) {
            break;
        }
    }
    thing->valuable.gold_stored = gold_store;
    if (thing->valuable.gold_stored <= 0)
    {
        delete_thing_structure(thing, 0);
        return -1;
    }
    return 0;
}

TbBool temple_check_for_arachnid_join_dungeon(struct Dungeon *dungeon)
{
    if ((dungeon->chickens_sacrificed % 16) == 0)
    {
        ThingModel crmodel, spdigmodel;
        crmodel = get_creature_model_with_model_flags(CMF_IsArachnid);
        spdigmodel = get_players_special_digger_model(dungeon->owner);
        if ((dungeon->gold_piles_sacrificed == 4) &&
            (dungeon->creature_sacrifice[spdigmodel] == 4) &&
            (dungeon->owned_creatures_of_model[crmodel] < 4))
        {
            SYNCLOG("Conditions to trigger arachnid met");
            struct Room *room;
            room = pick_random_room(dungeon->owner, RoK_ENTRANCE);
            if (room_is_invalid(room))
            {
                ERRORLOG("Could not get a random entrance for player %d",(int)dungeon->owner);
                return false;
            }
            struct Thing *ncreatng;
            ncreatng = create_creature_at_entrance(room, crmodel);
            set_creature_level(ncreatng, ACTION_RANDOM(CREATURE_MAX_LEVEL));
            return true;
        }
    }
    return false;
}

long process_temple_special(struct Thing *thing, long sacowner)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(sacowner);
    if (object_is_mature_food(thing))
    {
        dungeon->chickens_sacrificed++;
        if (temple_check_for_arachnid_join_dungeon(dungeon))
            return true;
    } else
    {
        dungeon->gold_piles_sacrificed++;
    }
    return false;
}

void process_object_sacrifice(struct Thing *thing, long sacowner)
{
    //_DK_process_object_sacrifice(thing, sacowner); return;
    PlayerNumber slbowner;
    {
        struct SlabMap *slb;
        slb = get_slabmap_thing_is_on(thing);
        slbowner = slabmap_owner(slb);
    }
    if (object_is_mature_food(thing))
    {
        process_temple_special(thing, sacowner);
        kill_all_players_chickens(thing->owner);
        if (is_my_player_number(sacowner))
            output_message(SMsg_SacrificePunish, 0, true);
    } else
    if (object_is_gold_pile(thing))
    {
        if (thing->valuable.gold_stored > 0)
        {
            process_temple_special(thing, sacowner);
            int num_allies;
            num_allies = 0;
            PlayerNumber plyr_idx;
            for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
            {
                if ((slbowner != plyr_idx) && players_are_mutual_allies(slbowner, plyr_idx))
                {
                    num_allies++;
                }
            }
            if (num_allies > 0)
            {
                GoldAmount value;
                value = thing->valuable.gold_stored / num_allies;
                for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
                {
                    if ((slbowner != plyr_idx) && players_are_mutual_allies(slbowner, plyr_idx))
                    {
                        player_add_offmap_gold(plyr_idx, value);
                    }
                }
            } else
            {
                if (is_my_player_number(sacowner))
                    output_message(SMsg_SacrificeWishing, 0, true);
            }
        }
    }
}

struct Thing *find_base_thing_on_mapwho_excluding_self(struct Thing *thing)
{
    return _DK_find_base_thing_on_mapwho_excluding_self(thing);
}

long object_being_dropped(struct Thing *thing)
{
    if (!thing_touching_floor(thing)) {
        return 1;
    }
    if (subtile_has_sacrificial_on_top(thing->mappos.x.stl.num, thing->mappos.y.stl.num))
    {
        struct Room *room;
        room = get_room_thing_is_on(thing);
        process_object_sacrifice(thing, room->owner);
        delete_thing_structure(thing, 0);
        return -1;
    }
    if (object_is_gold_pile(thing))
    {
        if (thing->valuable.gold_stored <= 0)
        {
            delete_thing_structure(thing, 0);
            return -1;
        }
        struct Room *room;
        room = get_room_thing_is_on(thing);
        if (!room_is_invalid(room) && (room->kind == RoK_TREASURE))
        {
            if ((thing->owner == room->owner) || is_neutral_thing(thing))
            {
                if (gold_being_dropped_at_treasury(thing, room) == -1) {
                    return -1;
                }
            }
        }
        if (thing->model == 128)
        {
            drop_gold_pile(thing->valuable.gold_stored, &thing->mappos);
            delete_thing_structure(thing, 0);
            return -1;
        }
        struct Thing *gldtng;
        gldtng = find_base_thing_on_mapwho_excluding_self(thing);
        if (!thing_is_invalid(gldtng))
        {
            add_gold_to_pile(gldtng, thing->valuable.gold_stored);
            delete_thing_structure(thing, 0);
            return -1;
        }
    }
    thing->active_state = thing->continue_state;
    return 1;
}

void update_dungeon_heart_beat(struct Thing *heartng)
{
    long i;
    long long k;
    const long base_heart_beat_rate = 2304;
    static long bounce = 0;
    if (heartng->active_state != ObSt_BeingDestroyed)
    {
        i = (char)heartng->byte_14;
        heartng->field_3E = 0;
        struct ObjectConfig *objconf;
        objconf = get_object_model_stats2(5);
        k = 384 * (long)(objconf->health - heartng->health) / objconf->health;
        k = base_heart_beat_rate / (k + 128);
        light_set_light_intensity(heartng->light_id, light_get_light_intensity(heartng->light_id) + (i*36/k));
        heartng->field_40 += (i*base_heart_beat_rate/k);
        if (heartng->field_40 < 0)
        {
            heartng->field_40 = 0;
            light_set_light_intensity(heartng->light_id, 20);
            heartng->byte_14 = 1;
        }
        if (heartng->field_40 > base_heart_beat_rate-1)
        {
            heartng->field_40 = base_heart_beat_rate-1;
            light_set_light_intensity(heartng->light_id, 56);
            heartng->byte_14 = (unsigned char)-1;
            if ( bounce )
            {
                thing_play_sample(heartng, 151, NORMAL_PITCH, 0, 3, 1, 6, FULL_LOUDNESS);
            } else
            {
                thing_play_sample(heartng, 150, NORMAL_PITCH, 0, 3, 1, 6, FULL_LOUDNESS);
            }
            bounce = !bounce;
        }
        k = (((unsigned long long)heartng->field_40 >> 32) & 0xFF) + heartng->field_40;
        heartng->field_48 = (k >> 8) & 0xFF;
        if ( !S3DEmitterIsPlayingSample(heartng->snd_emitter_id, 93, 0) )
          thing_play_sample(heartng, 93, NORMAL_PITCH, -1, 3, 1, 6, FULL_LOUDNESS);
    }
}

TngUpdateRet object_update_dungeon_heart(struct Thing *heartng)
{
    struct Dungeon *dungeon;
    long i;
    long long k;
    SYNCDBG(18,"Starting");
    if ((heartng->health > 0) && (game.dungeon_heart_heal_time != 0))
    {
        struct ObjectConfig *objconf;
        objconf = get_object_model_stats2(5);
        if ((game.play_gameturn % game.dungeon_heart_heal_time) == 0)
        {
            heartng->health += game.dungeon_heart_heal_health;
            if (heartng->health < 0)
            {
              heartng->health = 0;
            } else
            if (heartng->health > objconf->health)
            {
              heartng->health = objconf->health;
            }
        }
        k = ((heartng->health << 8) / objconf->health) << 7;
        i = (saturate_set_signed(k,32) >> 8) + 128;
        struct Objects *objdat;
        objdat = get_objects_data_for_thing(heartng);
        heartng->sprite_size = i * (long)objdat->sprite_size_max >> 8;
        heartng->clipbox_size_xy = i * (long)objdat->size_xy >> 8;
    } else
    if (heartng->owner != game.neutral_player_num)
    {
        dungeon = get_players_num_dungeon(heartng->owner);
        if (dungeon->heart_destroy_state == 0)
        {
            dungeon->heart_destroy_turn = 0;
            dungeon->heart_destroy_state = 1;
            dungeon->essential_pos.x.val = heartng->mappos.x.val;
            dungeon->essential_pos.y.val = heartng->mappos.y.val;
            dungeon->essential_pos.z.val = heartng->mappos.z.val;
        }
    }
    process_dungeon_destroy(heartng);
    SYNCDBG(18,"Beat update");
    if ((heartng->alloc_flags & TAlF_Exists) == 0)
      return 0;
    if ( heartng->byte_13 )
      heartng->byte_13--;
    update_dungeon_heart_beat(heartng);
    return TUFRet_Modified;
}

void set_call_to_arms_as_birthing(struct Thing *objtng)
{
    int frame;
    switch (objtng->byte_13)
    {
    case CTAOL_Birthing:
        frame = objtng->field_48;
        break;
    case CTAOL_Alive:
        frame = 0;
        break;
    case CTAOL_Dying:
    case CTAOL_Rebirthing:
        frame = objtng->field_49 - (int)objtng->field_48;
        break;
    default:
        ERRORLOG("Invalid CTA object life state %d",(int)objtng->byte_13);
        frame = 0;
        break;
    }
    struct CallToArmsGraphics *ctagfx;
    ctagfx = &call_to_arms_graphics[objtng->owner];
    struct Objects *objdat;
    objdat = get_objects_data_for_thing(objtng);
    set_thing_draw(objtng, ctagfx->birth_spr_idx, 256, objdat->sprite_size_max, 0, frame, 2);
    objtng->byte_13 = CTAOL_Birthing;
    stop_thing_playing_sample(objtng, 83);
    thing_play_sample(objtng, 83, NORMAL_PITCH, 0, 3, 0, 6, FULL_LOUDNESS);
}

void set_call_to_arms_as_dying(struct Thing *objtng)
{
    //_DK_set_call_to_arms_as_dying(objtng); return;
    int frame;
    switch (objtng->byte_13)
    {
    case CTAOL_Birthing:
        frame = objtng->field_49 - (int)objtng->field_48;
        break;
    case CTAOL_Alive:
        frame = 0;
        break;
    case CTAOL_Dying:
    case CTAOL_Rebirthing:
        frame = objtng->field_48;
        break;
    default:
        ERRORLOG("Invalid CTA object life state %d",(int)objtng->byte_13);
        frame = 0;
        break;
    }
    struct CallToArmsGraphics *ctagfx;
    ctagfx = &call_to_arms_graphics[objtng->owner];
    struct Objects *objdat;
    objdat = get_objects_data_for_thing(objtng);
    set_thing_draw(objtng, ctagfx->field_8, 256, objdat->sprite_size_max, 0, frame, 2);
    objtng->byte_13 = CTAOL_Dying;
}

void set_call_to_arms_as_rebirthing(struct Thing *objtng)
{
    //_DK_set_call_to_arms_as_rebirthing(objtng); return;
    int frame;
    switch (objtng->byte_13)
    {
    case CTAOL_Birthing:
        frame = objtng->field_49 - (int)objtng->field_48;
        break;
    case CTAOL_Alive:
        frame = 0;
        break;
    case CTAOL_Dying:
    case CTAOL_Rebirthing:
        frame = objtng->field_48;
        break;
    default:
        ERRORLOG("Invalid CTA object life state %d",(int)objtng->byte_13);
        frame = 0;
        break;
    }
    struct CallToArmsGraphics *ctagfx;
    ctagfx = &call_to_arms_graphics[objtng->owner];
    struct Objects *objdat;
    objdat = get_objects_data_for_thing(objtng);
    set_thing_draw(objtng, ctagfx->field_8, 256, objdat->sprite_size_max, 0, frame, 2);
    objtng->byte_13 = CTAOL_Rebirthing;
}

TngUpdateRet object_update_call_to_arms(struct Thing *thing)
{
    struct PlayerInfo *player;
    player = get_player(thing->owner);
    if (thing->index != player->field_43C)
    {
        delete_thing_structure(thing, 0);
        return -1;
    }
    struct Dungeon *dungeon;
    dungeon = get_players_dungeon(player);
    struct CallToArmsGraphics *ctagfx;
    ctagfx = &call_to_arms_graphics[player->id_number];
    struct Objects *objdat;
    objdat = get_objects_data_for_thing(thing);
    struct Coord3d pos;

    switch (thing->byte_13)
    {
    case CTAOL_Birthing:
        if (thing->field_49 - 1 <= thing->field_48)
        {
            thing->byte_13 = CTAOL_Alive;
            set_thing_draw(thing, ctagfx->field_4, 256, objdat->sprite_size_max, 0, 0, 2);
            return 1;
        }
        break;
    case CTAOL_Alive:
        break;
    case CTAOL_Dying:
        if (thing->field_49 - 1 == thing->field_48)
        {
            player->field_43C = 0;
            delete_thing_structure(thing, 0);
            return -1;
        }
        break;
    case CTAOL_Rebirthing:
        if (thing->field_49 - 1 == thing->field_48)
        {
            pos.x.val = subtile_coord_center(dungeon->cta_stl_x);
            pos.y.val = subtile_coord_center(dungeon->cta_stl_y);
            pos.z.val = get_thing_height_at(thing, &pos);
            move_thing_in_map(thing, &pos);
            set_thing_draw(thing, ctagfx->birth_spr_idx, 256, objdat->sprite_size_max, 0, 0, 2);
            thing->byte_13 = CTAOL_Birthing;
            stop_thing_playing_sample(thing, 83);
            thing_play_sample(thing, 83, NORMAL_PITCH, 0, 3, 0, 6, FULL_LOUDNESS);
        }
        break;
    default:
        break;
    }
    return 1;
}

TngUpdateRet object_update_armour(struct Thing *objtng)
{
    //return _DK_object_update_armour(objtng);
    struct Thing *thing;
    thing = thing_get(objtng->word_13);
    if (thing_is_picked_up(thing))
    {
        objtng->field_4F |= 0x01;
        return 1;
    }
    long cvect_len;
    short shspeed;
    struct Coord3d pos;
    struct ComponentVector cvect;
    pos.x.val = thing->mappos.x.val;
    pos.y.val = thing->mappos.y.val;
    pos.z.val = thing->mappos.z.val;
    if ((abs(objtng->mappos.x.val - pos.x.val) > 512)
     || (abs(objtng->mappos.y.val - pos.y.val) > 512)
     || (abs(objtng->mappos.z.val - pos.z.val) > 512))
    {
        shspeed = objtng->byte_15;
        pos.x.val += 32 * LbSinL(682 * shspeed) >> 16;
        pos.y.val += -(32 * LbCosL(682 * shspeed) >> 8) >> 8;
        pos.z.val += shspeed * (thing->clipbox_size_yz >> 1);
        move_thing_in_map(objtng, &pos);
        objtng->move_angle_xy = thing->move_angle_xy;
        objtng->move_angle_z = thing->move_angle_z;
        angles_to_vector(thing->move_angle_xy, thing->move_angle_z, 32, &cvect);
    }
    else
    {
        pos.z.val += (thing->clipbox_size_yz >> 1);
        objtng->move_angle_xy = get_angle_xy_to(&objtng->mappos, &pos);
        objtng->move_angle_z = get_angle_yz_to(&objtng->mappos, &pos);
        angles_to_vector(objtng->move_angle_xy, objtng->move_angle_z, 32, &cvect);
        cvect_len = LbSqrL(cvect.x * cvect.x + cvect.z * cvect.z + cvect.y * cvect.y);
        if (cvect_len > 128)
        {
          pos.x.val = (cvect.x << 7) / cvect_len;
          pos.y.val = (cvect.y << 7) / cvect_len;
          pos.z.val = (cvect.z << 7) / cvect_len;
          cvect.x = pos.x.val;
          cvect.y = pos.y.val;
          cvect.z = pos.z.val;
        }
    }
    objtng->state_flags |= 0x04;
    objtng->veloc_push_add.x.val += cvect.x;
    objtng->veloc_push_add.y.val += cvect.y;
    objtng->veloc_push_add.z.val += cvect.z;
    objtng->field_4F &= ~0x01;
    return 1;
}

TngUpdateRet object_update_object_scale(struct Thing *objtng)
{
    //return _DK_object_update_object_scale(objtng);
    struct Thing *creatng;
    creatng = thing_get(objtng->word_13);
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    struct Objects *objdat;
    objdat = get_objects_data_for_thing(objtng);
    int start_frame;
    int spr_size;
    start_frame = objtng->field_48;
    if (objtng->word_13) {
        spr_size = 300 * cctrl->explevel / 20 + 300;
    } else {
        spr_size = objdat->sprite_size_max;
    }
    int cssize;
    cssize = objtng->word_15;
    objtng->word_17 = spr_size;
    long i;
    if (cssize+32 < spr_size)
    {
        objtng->word_15 = cssize+32;
        i = convert_td_iso(objdat->field_5);
    } else
    if (cssize-32 > spr_size)
    {
        objtng->word_15 = cssize-32;
        i = convert_td_iso(objdat->field_5);
    } else
    {
        objtng->word_15 = spr_size;
        i = convert_td_iso(objdat->field_5);
    }
    if ((i & 0x8000u) != 0) {
        i = objdat->field_5;
    }
    set_thing_draw(objtng, i, objdat->anim_speed, objtng->word_15, 0, start_frame, objdat->field_11);
    return 1;
}

TngUpdateRet object_update_armour2(struct Thing *objtng)
{
    return _DK_object_update_armour2(objtng);
}

TngUpdateRet object_update_power_sight(struct Thing *objtng)
{
    return _DK_object_update_power_sight(objtng);
}

#define NUM_ANGLES 16
TngUpdateRet object_update_power_lightning(struct Thing *objtng)
{
    long i;
    unsigned long exist_turns;
    long variation;
    objtng->health = 2;
    exist_turns = game.play_gameturn - objtng->creation_turn;
    variation = NUM_ANGLES * exist_turns;
    for (i=0; i < NUM_ANGLES; i++)
    {
        struct Coord3d pos;
        int angle;
        angle = (variation % NUM_ANGLES) * 2*LbFPMath_PI / NUM_ANGLES;
        if (set_coords_to_cylindric_shift(&pos, &objtng->mappos, 8*variation, angle, 0))
        {
            struct Map *mapblk;
            mapblk = get_map_block_at(pos.x.stl.num, pos.y.stl.num);
            if ((mapblk->flags & SlbAtFlg_Blocking) == 0)
            {
                pos.z.val = get_floor_height_at(&pos) + 128;
                create_effect_element(&pos, lightning_spangles[objtng->owner], objtng->owner);
            }
        }
        variation++;
    }
    const struct MagicStats *pwrdynst;
    pwrdynst = get_power_dynamic_stats(PwrK_LIGHTNING);
    if (exist_turns > abs(pwrdynst->strength[objtng->byte_13]))
    {
        delete_thing_structure(objtng, 0);
        return TUFRet_Deleted;
    }
    return TUFRet_Modified;
}
#undef NUM_ANGLES

TngUpdateRet move_object(struct Thing *thing)
{
    struct Coord3d pos;
    TbBool move_allowed;
    SYNCDBG(18,"Starting");
    TRACE_THING(thing);
    move_allowed = get_thing_next_position(&pos, thing);
    if ( !positions_equivalent(&thing->mappos, &pos) )
    {
        if ((!move_allowed) || thing_in_wall_at(thing, &pos))
        {
            long blocked_flags;
            blocked_flags = get_thing_blocked_flags_at(thing, &pos);
            slide_thing_against_wall_at(thing, &pos, blocked_flags);
            remove_relevant_forces_from_thing_after_slide(thing, &pos, blocked_flags);
            if (thing->model == 6)
              thing_play_sample(thing, 79, NORMAL_PITCH, 0, 3, 0, 1, FULL_LOUDNESS);
        }
        move_thing_in_map(thing, &pos);
    }
    thing->field_60 = get_thing_height_at(thing, &thing->mappos);
    return TUFRet_Modified;
}

TngUpdateRet update_object(struct Thing *thing)
{
    Thing_Class_Func upcallback;
    Thing_State_Func stcallback;
    struct Objects *objdat;
    SYNCDBG(18,"Starting for %s",thing_model_name(thing));
    TRACE_THING(thing);

    upcallback = NULL;
    if (thing->model < sizeof(object_update_functions)/sizeof(object_update_functions[0])) {
        upcallback = object_update_functions[thing->model];
    } else {
        ERRORLOG("Object model %d exceeds update_functions dimensions",(int)thing->model);
    }
    if (upcallback != NULL)
    {
        if (upcallback(thing) <= 0) {
            return TUFRet_Deleted;
        }
    }
    stcallback = NULL;
    if (thing->active_state < sizeof(object_state_functions)/sizeof(object_state_functions[0])) {
        stcallback = object_state_functions[thing->active_state];
    } else {
        ERRORLOG("The %s state %d exceeds state_functions dimensions",thing_model_name(thing),(int)thing->active_state);
    }
    if (stcallback != NULL)
    {
        SYNCDBG(18,"Updating state");
        if (stcallback(thing) <= 0) {
            return TUFRet_Deleted;
        }
    }
    SYNCDBG(18,"Updating position");
    thing->movement_flags &= ~TMvF_IsOnWater;
    thing->movement_flags &= ~TMvF_IsOnLava;
    if ( ((thing->movement_flags & TMvF_Unknown40) == 0) && thing_touching_floor(thing) )
    {
      if (subtile_has_lava_on_top(thing->mappos.x.stl.num, thing->mappos.y.stl.num))
      {
        thing->movement_flags |= TMvF_IsOnLava;
        objdat = get_objects_data_for_thing(thing);
        if ( (objdat->destroy_on_lava) && !thing_is_dragged_or_pulled(thing) )
        {
            destroy_object(thing);
            return TUFRet_Deleted;
        }
      } else
      if (subtile_has_water_on_top(thing->mappos.x.stl.num, thing->mappos.y.stl.num))
      {
        thing->movement_flags |= TMvF_IsOnWater;
      }
    }
    if ((thing->movement_flags & TMvF_Unknown40) != 0)
        return TUFRet_Modified;
    return move_object(thing);
}

/**
 * Creates a guard post flag object.
 * @param pos Position where the guard post flag is to be created.
 * @param plyr_idx Player who will own the flag.
 * @param parent_idx Slab number associated with the flag.
 * @return Guard flag object thing.
 */
struct Thing *create_guard_flag_object(const struct Coord3d *pos, PlayerNumber plyr_idx, long parent_idx)
{
    struct Thing *thing;
    ThingModel grdflag_kind;
    if (plyr_idx >= sizeof(player_guardflag_objects)/sizeof(player_guardflag_objects[0]))
        grdflag_kind = player_guardflag_objects[NEUTRAL_PLAYER];
    else
        grdflag_kind = player_guardflag_objects[(int)plyr_idx];
    if (grdflag_kind <= 0)
        return INVALID_THING;
    // Guard posts have slab number set as parent
    thing = create_object(pos, grdflag_kind, plyr_idx, parent_idx);
    if (thing_is_invalid(thing))
        return INVALID_THING;
    return thing;
}

struct Thing *create_gold_pot_at(long pos_x, long pos_y, PlayerNumber plyr_idx)
{
    struct Thing *gldtng;
    struct Coord3d pos;
    pos.x.val = pos_x;
    pos.y.val = pos_y;
    pos.z.val = subtile_coord(3,0);
    gldtng = create_object(&pos, 6, plyr_idx, -1);
    if (thing_is_invalid(gldtng))
        return INVALID_THING;
    gldtng->valuable.gold_stored = gold_object_typical_value(6);
    // Update size of the gold object
    add_gold_to_pile(gldtng, 0);
    return gldtng;
}

/**
 * For given gold hoard thing, returns the wealth size, scaled 0..max_size.
 */
int get_wealth_size_of_gold_hoard_object(const struct Thing *objtng)
{
    // Find position of the hoard size
    int i;
    for (i = get_wealth_size_types_count(); i > 0; i--)
    {
        if (gold_hoard_objects[i] == objtng->model)
            return i;
    }
    return 0;
}

/**
 * For given gold amount, returns ceiling wealth size which would fit it, scaled 0..max_size+1.
 */
int get_wealth_size_of_gold_amount(GoldAmount value)
{
    long wealth_size_holds;
    wealth_size_holds = gold_per_hoard / get_wealth_size_types_count();
    int wealth_size;
    wealth_size = (value + wealth_size_holds - 1) / wealth_size_holds;
    if (wealth_size > get_wealth_size_types_count()) {
        WARNLOG("Gold hoard with %d gold would be oversized",(int)value);
        wealth_size = get_wealth_size_types_count();
    }
    return wealth_size;
}

/**
 * Gives amount of possible wealth sizes of gold hoard.
 */
int get_wealth_size_types_count(void)
{
    return sizeof(gold_hoard_objects)/sizeof(gold_hoard_objects[0])-1;
}

/**
 * Creates a gold hoard object.
 * Note that this function does not create a fully operable object - gold hoard requires room
 * association to be fully functional. This is just a utility sub-function.
 * @param pos Position where the hoard is to be created.
 * @param plyr_idx Player who will own the hoard.
 * @param value The max amount of gold to be stored inside the hoard.
 * @return Hoard object thing, which still require to be associated to room.
 */
struct Thing *create_gold_hoard_object(const struct Coord3d *pos, PlayerNumber plyr_idx, GoldAmount value)
{
    if (value >= gold_per_hoard)
        value = gold_per_hoard;
    int wealth_size;
    wealth_size = get_wealth_size_of_gold_amount(value);
    struct Thing *gldtng;
    gldtng = create_object(pos, gold_hoard_objects[wealth_size], plyr_idx, -1);
    if (thing_is_invalid(gldtng))
        return INVALID_THING;
    gldtng->valuable.gold_stored = value;
    return gldtng;
}

struct Thing *create_gold_hoarde(struct Room *room, const struct Coord3d *pos, GoldAmount value)
{
    struct Thing *thing;
    GoldAmount wealth_size_holds;
    wealth_size_holds = gold_per_hoard / get_wealth_size_types_count();
    if ((value <= 0) || (room->slabs_count < 1)) {
        ERRORLOG("Attempt to create a gold hoard with %ld gold", (long)value);
        return INVALID_THING;
    }
    GoldAmount max_hoard_size_in_room;
    max_hoard_size_in_room = wealth_size_holds * room->total_capacity / room->slabs_count;
    if (value > max_hoard_size_in_room)
        value = max_hoard_size_in_room;
    thing = create_gold_hoard_object(pos, room->owner, value);
    if (!thing_is_invalid(thing))
    {
        struct Dungeon *dungeon;
        room->capacity_used_for_storage += thing->valuable.gold_stored;
        dungeon = get_dungeon(room->owner);
        if (!dungeon_invalid(dungeon)) {
            dungeon->total_money_owned += thing->valuable.gold_stored;
        }
        int wealth_size;
        wealth_size = get_wealth_size_of_gold_amount(thing->valuable.gold_stored);
        room->used_capacity += wealth_size;
    }
    return thing;
}

/**
 * Adds gold to hoard stored in room.
 *
 * @param gldtng The gold hoard thing.
 * @param room The room which stores this hoard.
 * @param amount Amount of gold to be added.
 * @return Gives amount really added to the hoard.
 */
long add_gold_to_hoarde(struct Thing *gldtng, struct Room *room, GoldAmount amount)
{
    //return _DK_add_gold_to_hoarde(gldtng, room, amount);
    GoldAmount wealth_size_holds;
    wealth_size_holds = gold_per_hoard / get_wealth_size_types_count();
    GoldAmount max_hoard_size_in_room;
    max_hoard_size_in_room = wealth_size_holds * room->total_capacity / room->slabs_count;
    // Fix amount
    if (gldtng->valuable.gold_stored + amount > max_hoard_size_in_room)
        amount = max_hoard_size_in_room - gldtng->valuable.gold_stored;
    if (amount <= 0) {
        return 0;
    }
    // Remove prev wealth size
    int wealth_size;
    wealth_size = get_wealth_size_of_gold_amount(gldtng->valuable.gold_stored);
    if (wealth_size > room->used_capacity) {
        ERRORLOG("Room %s index %d has used capacity %d but stores gold hoard of wealth size %d (%ld gold)",
            room_code_name(room->kind),(int)room->index,(int)room->used_capacity,(int)wealth_size,(long)gldtng->valuable.gold_stored);
        wealth_size = room->used_capacity;
    }
    room->used_capacity -= wealth_size;
    // Add amount of gold
    gldtng->valuable.gold_stored += amount;
    room->capacity_used_for_storage += amount;
    if (room->owner != game.neutral_player_num)
    {
        struct Dungeon *dungeon;
        dungeon = get_dungeon(room->owner);
        if (!dungeon_invalid(dungeon)) {
            dungeon->total_money_owned += amount;
        }
    }
    // Add new wealth size
    wealth_size = get_wealth_size_of_gold_amount(gldtng->valuable.gold_stored);
    room->used_capacity += wealth_size;
    // switch hoard object model
    gldtng->model = gold_hoard_objects[wealth_size];
    // Set visual appearance
    struct Objects *objdat;
    objdat = get_objects_data_for_thing(gldtng);
    unsigned short i, n;
    i = objdat->field_5;
    n = convert_td_iso(i);
    if ((n & 0x8000u) == 0) {
      i = n;
    }
    set_thing_draw(gldtng, i, objdat->anim_speed, objdat->sprite_size_max, 0, 0, objdat->field_11);
    return amount;
}

/**
 * Removes gold from hoard stored in room.
 *
 * @param gldtng The gold hoard thing.
 * @param room The room which stores this hoard.
 * @param amount Amount of gold to be taken.
 * @return Gives amount really taken from the hoard.
 */
long remove_gold_from_hoarde(struct Thing *gldtng, struct Room *room, GoldAmount amount)
{
    //return _DK_remove_gold_from_hoarde(gldtng, room, amount);
    if (amount <= 0) {
        return 0;
    }
    if (amount > gldtng->valuable.gold_stored)
        amount = gldtng->valuable.gold_stored;
    // Remove prev wealth size
    int wealth_size;
    wealth_size = get_wealth_size_of_gold_amount(gldtng->valuable.gold_stored);
    if (wealth_size > room->used_capacity) {
        ERRORLOG("Room %s index %d has used capacity %d but stores gold hoard of wealth size %d (%ld gold)",
            room_code_name(room->kind),(int)room->index,(int)room->used_capacity,(int)wealth_size,(long)gldtng->valuable.gold_stored);
        wealth_size = room->used_capacity;
    }
    room->used_capacity -= wealth_size;
    // Add amount of gold
    gldtng->valuable.gold_stored -= amount;
    room->capacity_used_for_storage -= amount;
    struct Dungeon *dungeon;
    dungeon = get_dungeon(gldtng->owner);
    if (!dungeon_invalid(dungeon)) {
        dungeon->total_money_owned -= amount;
    }

    if (gldtng->valuable.gold_stored <= 0)
    {
        delete_thing_structure(gldtng, 0);
        return amount;
    }
    // Add new wealth size
    wealth_size = get_wealth_size_of_gold_amount(gldtng->valuable.gold_stored);
    room->used_capacity += wealth_size;
    // switch hoard object model
    gldtng->model = gold_hoard_objects[wealth_size];
    // Set visual appearance
    struct Objects *objdat;
    objdat = get_objects_data_for_thing(gldtng);
    unsigned short i, n;
    i = objdat->field_5;
    n = convert_td_iso(i);
    if ((n & 0x8000u) == 0) {
      i = n;
    }
    set_thing_draw(gldtng, i, objdat->anim_speed, objdat->sprite_size_max, 0, 0, objdat->field_11);
    return amount;
}

/**
 * Returns if given thing is a hoard of gold.
 * @note originally was thing_is_gold_hoarde().
 * @param thing
 * @return
 */
TbBool thing_is_gold_hoard(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return false;
    if (thing->class_id != TCls_Object)
      return false;
    return object_is_gold_hoard(thing);
}

struct Thing *find_gold_hoard_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Thing *thing;
    struct Map *mapblk;
    long i;
    unsigned long k;
    k = 0;
    mapblk = get_map_block_at(stl_x,stl_y);
    i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
      thing = thing_get(i);
      if (thing_is_invalid(thing))
      {
        WARNLOG("Jump out of things array");
        break;
      }
      i = thing->next_on_mapblk;
      // Per-thing block
      if (thing_is_gold_hoard(thing))
          return thing;
      // Per-thing block ends
      k++;
      if (k > THINGS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping things list");
        break;
      }
    }
    return INVALID_THING;
}

GoldAmount gold_object_typical_value(ThingModel tngmodel)
{
    switch (tngmodel)
    {
      case 3:
        return game.chest_gold_hold;
      case 6:
          return game.pot_of_gold_holds;
      case 43:
          return game.gold_pile_value;
      case 128:
          return game.gold_pile_maximum;
      default:
        break;
    }
    return 0;
}

/**
 * Adds given amount of gold to gold pile, gold pot or gold chest.
 * Scales size of the pile or pot accordingly.
 *
 * @param thing
 * @param value
 * @return
 */
TbBool add_gold_to_pile(struct Thing *thing, long value)
{
    long scaled_val;
    if (thing_is_invalid(thing)) {
        return false;
    }
    GoldAmount typical_value;
    typical_value = gold_object_typical_value(thing->model);
    if (typical_value <= 0) {
        return false;
    }
    thing->valuable.gold_stored += value;
    if (thing->valuable.gold_stored < 0)
        thing->valuable.gold_stored = LONG_MAX;
    if (thing->valuable.gold_stored < typical_value)
        scaled_val = 196 * thing->valuable.gold_stored / typical_value + 128;
    else
        scaled_val = 196 + (24 * (thing->valuable.gold_stored-typical_value) / typical_value) + 128;
    if (scaled_val > 640)
      scaled_val = 640;
    thing->sprite_size = scaled_val;
    return true;
}

struct Thing *create_gold_pile(struct Coord3d *pos, PlayerNumber plyr_idx, long value)
{
    struct Thing *gldtng;
    gldtng = create_object(pos, 43, plyr_idx, -1);
    if (thing_is_invalid(gldtng)) {
        return INVALID_THING;
    }
    gldtng->valuable.gold_stored = 0;
    add_gold_to_pile(gldtng, value);
    return gldtng;
}

struct Thing *drop_gold_pile(long value, struct Coord3d *pos)
{
    struct Thing *thing;
    thing = smallest_gold_pile_at_xy(pos->x.stl.num, pos->y.stl.num);
    if (thing_is_invalid(thing)) {
        thing = create_gold_pile(pos, game.neutral_player_num, value);
    } else {
        add_gold_to_pile(thing, value);
    }
    return thing;
}

void create_rubble_for_dug_block(MapSubtlCoord stl_x, MapSubtlCoord stl_y, MapSubtlCoord stl_z_max, PlayerNumber plyr_idx)
{
    int zmax;
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = subtile_coord_center(1);
    zmax = subtile_coord(stl_z_max,0);
    while (pos.z.val < zmax)
    {
        create_effect(&pos, 26, plyr_idx);
        pos.z.val += 256;
    }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
