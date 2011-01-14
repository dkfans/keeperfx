/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_effects.c
 *     Effect generators and effect elements support functions.
 * @par Purpose:
 *     Functions to create and maintain effect generators and single effect elements.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     01 Jan 2010 - 12 Jan 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_effects.h"
#include "globals.h"

#include "bflib_math.h"
#include "bflib_sound.h"
#include "thing_objects.h"
#include "thing_list.h"
#include "thing_stats.h"
#include "front_simple.h"
#include "gui_topmsg.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT struct Thing *_DK_create_effect_element(const struct Coord3d *pos, unsigned short a2, unsigned short a3);
DLLIMPORT struct Thing *_DK_create_effect_generator(struct Coord3d *pos, unsigned short a1, unsigned short a2, unsigned short a3, long a4);
DLLIMPORT void _DK_poison_cloud_affecting_area(struct Thing *thing, struct Coord3d *pos, long a3, long a4, unsigned char a5);
DLLIMPORT void _DK_process_spells_affected_by_effect_elements(struct Thing *thing);
DLLIMPORT void _DK_process_thing_spell_effects(struct Thing *thing);
DLLIMPORT long _DK_update_effect_element(struct Thing *thing);
DLLIMPORT long _DK_update_effect(struct Thing *thing);
DLLIMPORT long _DK_process_effect_generator(struct Thing *thing);
DLLIMPORT struct Thing *_DK_create_effect(const struct Coord3d *pos, unsigned short a2, unsigned char a3);
DLLIMPORT long _DK_move_effect(struct Thing *thing);
DLLIMPORT long _DK_move_effect_element(struct Thing *thing);
DLLIMPORT void _DK_change_effect_element_into_another(struct Thing *thing, long nmodel);

/******************************************************************************/
extern struct EffectElementStats _DK_effect_element_stats[95];
//#define effect_element_stats _DK_effect_element_stats
/******************************************************************************/
struct EffectElementStats effect_element_stats[] = {
   {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 2, 0, -1, -1, 926, 152, 192, 1, 192, 256, 1, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1},
   {2, 1, 0, 30, 40, 96, 122, 142, 0, 192, 256, 0,
    0, 0, 1, 0, 0, 10, 0, 102, 0, 0, 0, 0, 0, 1,
    0, 0, 256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1},
   {2, 1, 0, 30, 40, 96, 122, 142, 0, 192, 256, 0,
    0, 0, 1, 0, 0, 10, 0, 102, 0, 0, 0, 0, 0, 1,
    0, 0, 256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1},
   {2, 1, 0, 30, 40, 96, 122, 142, 0, 192, 256, 0,
    0, 0, 1, 0, 0, 10, 0, 102, 0, 0, 0, 0, 0, 1,
    0, 0, 256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1},
   {2, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, 5, 5, 916, 110, 128, 0, 256, 256, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0,
    0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 1, 0, 30, 40, 929, 320, 374, 0, 128, 128, 1, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1},
   {2, 1, 0, 20, 20, 981, 172, 192, 0, 256, 256, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 2, 0, -1, -1, 927, 50, 256, 1, 256, 320, 1, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1},
   {2, 1, 0, -1, -1, 798, 50, 256, 1, 256, 320, 1, 1,
    3, 0, 0, 2, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256,
    0, 0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1},
   {2, 1, 0, 15, 15, 908, 256, 256, 1, 256, 256, 1, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1},
   {2, 1, 0, 15, 15, 913, 226, 256, 0, 256, 256, 0, 1,
    0, 1, 0, 0, 8, 0, 102, 0, 0, 0, 0, 0, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 1, 0, 15, 15, 914, 226, 256, 0, 256, 256, 0, 1,
    0, 1, 0, 0, 8, 0, 102, 0, 0, 0, 0, 0, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 1, 0, 15, 15, 915, 226, 256, 0, 256, 256, 0, 1,
    0, 1, 0, 0, 8, 0, 102, 0, 0, 0, 0, 0, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 5, 0, 1, 1, 964, 24, 24, 0, 256, 256, 1, 1, 3, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
   {2, 5, 0, -1, -1, 918, 96, 96, 0, 256, 256, 1, 1,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 5, 0, 1, 1, 917, 96, 96, 0, 256, 256, 1, 1, 3, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, 6, 6, 964, 64, 64, 0, 256, 256, 1, 1, 3, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, 1, 1, 981, 192, 192, 0, 256, 256, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0,
    0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, 20000, 20000, 907, 192, 192, 0, 256, 256,
    1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 1, 0, 40, 40, 919, 320, 374, 0, 128, 128, 1, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1},
   {2, 5, 0, -1, -1, 909, 250, 300, 1, 128, 128, 1, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1},
   {2, 5, 0, -1, -1, 836, 200, 256, 1, 16, 32, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0,
    0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 1, 0, 40, 50, 910, 150, 180, 0, 256, 256, 0, 0,
    2, 1, 0, 0, 8, 0, 102, 0, 0, 0, 0, 0, 1, 0, 0,
    256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 25, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 1, 0, -1, -1, 911, 150, 180, 1, 16, 32, 1, 0,
    2, 1, 0, 0, 8, 0, 102, 0, 0, 0, 0, 0, 1, 0, 0,
    256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 1, 0, 30, 40, 834, 122, 142, 0, 192, 256, 0, 0,
    0, 1, 0, 0, 40, 0, 102, 0, 0, 0, 0, 0, 1, 0, 0,
    256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 1, 0, 30, 40, 833, 122, 142, 0, 192, 256, 0, 0,
    0, 1, 0, 0, 40, 0, 102, 0, 0, 0, 0, 0, 1, 0, 0,
    256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 1, 0, 30, 40, 832, 122, 142, 0, 192, 256, 0, 0,
    0, 1, 0, 0, 40, 0, 102, 0, 0, 0, 0, 0, 1, 0, 0,
    256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 1, 0, -1, -1, 828, 250, 300, 1, 128, 128, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 1, 0, 30, 40, 823, 122, 142, 0, 192, 256, 0, 0,
    3, 1, 0, 0, 10, 0, 16, 0, 0, 0, 32, 1, 1, 0, 0,
    256, 0, 19, 36, 80, 1, 0, 0, 256, 1, 0, 1280, 52, 0,
    0, 0, 0, 0, 1},
   {2, 1, 0, 30, 40, 827, 122, 142, 0, 192, 256, 0, 0,
    2, 1, 0, 0, 10, 0, 102, 0, 0, 0, 0, 0, 1, 19, 36,
    80, 1, 19, 36, 80, 1, 0, 0, 256, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 5, 0, -1, -1, 824, 122, 142, 1, 192, 256, 0, 0,
    3, 1, 0, 0, 10, 0, 102, 0, 0, 0, 0, 0, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0},
   {2, 1, 0, 10, 15, 832, 122, 142, 0, 192, 256, 0, 0,
    0, 1, 0, 0, 10, 0, 102, 0, 0, 0, 0, 0, 1, 0, 0,
    256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 5, 0, -1, -1, 0, 450, 450, 1, 256, 256, 1, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0,
    0, 256, 0, 0, 0, 256, 0, 35, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, 20000, 20000, 0, 450, 450, 0, 256, 256,
    1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0},
   {2, 5, 0, -1, -1, 0, 450, 450, 1, 256, 256, 1, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0,
    0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 1, 0, -1, -1, 110, 225, 270, 1, 85, 85, 1, 0,
    2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1},
   {2, 5, 0, -1, -1, 825, 256, 256, 1, 256, 256, 1, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1},
   {2, 1, 0, -1, -1, 926, 122, 192, 0, 192, 256, 1, 0,
    1, 0, 0, 0, 10, 0, 102, 0, 0, 0, 0, 0, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0},
   {2, 5, 0, -1, -1, 828, 250, 300, 1, 128, 128, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1},
   {4, 4, 0, 16, 16, 0, 256, 256, 0, 256, 256, 1, 1, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0,
    0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, 2, 4, 964, 128, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 5, 0, -1, -1, 852, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 5, 0, 4, 4, 852, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 5, 0, -1, -1, 802, 172, 196, 0, 256, 256, 1, 1,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 5, 0, 100, 100, -1, 122, 142, 0, 192, 256, 0, 0,
    0, 1, 0, 0, 40, 0, 102, 0, 0, 0, 0, 0, 1, 0, 0,
    256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 1, 0, 20, 30, 919, 320, 374, 0, 128, 128, 1, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1},
   {2, 1, 0, 10, 15, 832, 122, 142, 0, 192, 256, 0, 0,
    0, 1, 0, 0, 40, 0, 102, 0, 0, 0, 0, 0, 1, 0, 0,
    256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 1, 0, 30, 40, 831, 130, 180, 0, 192, 256, 0, 0,
    0, 1, 0, 0, 40, 0, 102, 0, 0, 0, 0, 0, 1, 0, 0,
    256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 1, 0, 30, 40, 831, 180, 250, 0, 192, 256, 0, 0,
    0, 1, 0, 0, 40, 0, 102, 0, 0, 0, 0, 0, 1, 0, 0,
    256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 1, 0, 30, 40, 831, 250, 350, 0, 192, 256, 0, 0,
    0, 1, 0, 0, 40, 0, 102, 0, 0, 0, 0, 0, 1, 0, 0,
    256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 5, 0, 1, 1, 964, 64, 64, 0, 256, 256, 1, 1, 3, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 1, 0, 6, 12, 964, 64, 96, 0, 256, 256, 1, 1, 3,
    0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0,
    0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 1, 0, -1, -1, 856, 128, 128, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 5, 0, -1, -1, 863, 320, 374, 1, 256, 256, 0, 0,
    3, 1, 0, 0, 10, 0, 102, 0, 0, 0, 0, 0, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0},
   {2, 5, 0, -1, -1, 864, 320, 374, 1, 256, 256, 0, 0,
    3, 1, 0, 0, 10, 0, 102, 0, 0, 0, 0, 0, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0},
   {2, 5, 0, -1, -1, 865, 320, 374, 1, 256, 256, 0, 0,
    3, 1, 0, 0, 10, 0, 102, 0, 0, 0, 0, 0, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0},
   {2, 5, 0, -1, -1, 866, 320, 374, 1, 256, 256, 0, 0,
    3, 1, 0, 0, 10, 0, 102, 0, 0, 0, 0, 0, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0},
   {2, 5, 0, 8, 8, 819, 256, 256, 1, 256, 256, 1, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 1, 0, 5, 5, 964, 96, 160, 1, 85, 85, 1, 1, 3, 0,
    0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, -1, -1, 120, 256, 256, 1, 256, 256, 1, 1,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1},
   {2, 5, 0, 1, 1, -1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
   {2, 2, 0, -1, -1, 116, 256, 256, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 2, 0, -1, -1, 117, 256, 256, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 2, 0, -1, -1, 118, 256, 256, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 2, 0, -1, -1, 119, 256, 256, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 2, 0, -1, -1, 116, 358, 358, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 2, 0, -1, -1, 117, 358, 358, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 2, 0, -1, -1, 118, 358, 358, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 2, 0, -1, -1, 119, 358, 358, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 2, 0, -1, -1, 116, 460, 460, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 2, 0, -1, -1, 117, 460, 460, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 2, 0, -1, -1, 118, 460, 460, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 2, 0, -1, -1, 119, 460, 460, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 1, 0, -1, -1, 966, 172, 255, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 1, 0, -1, -1, 967, 172, 255, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 1, 0, -1, -1, 968, 172, 255, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 1, 0, -1, -1, 969, 172, 255, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 1, 0, -1, -1, 857, 128, 128, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 1, 0, -1, -1, 858, 128, 128, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 1, 0, -1, -1, 859, 128, 128, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 1, 0, -1, -1, 860, 128, 128, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 5, 0, -1, -1, 852, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 1, 1, 30, 40, 96, 160, 256, 0, 192, 256, 0, 0,
    1, 1, 0, 0, 10, 0, 102, 0, 0, 0, 0, 0, 1, 66, 0,
    256, 1, 66, 36, 100, 1, 0, 0, 256, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 5, 0, -1, -1, 97, 250, 300, 1, 128, 128, 1, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1},
   {2, 5, 0, -1, -1, 853, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 5, 0, -1, -1, 854, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 5, 0, -1, -1, 855, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 1, 0, -1, -1, 110, 225, 270, 1, 1024, 1024, 1, 0,
    2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
   {2, 5, 0, -1, -1, 853, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 5, 0, -1, -1, 854, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 5, 0, -1, -1, 855, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
   {2, 1, 0, 5, 5, 917, 96, 160, 1, 85, 85, 1, 1, 3, 0,
    0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, -1, -1, 837, 200, 256, 1, 16, 32, 1, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0},
};

long const bounce_table[] = { -160, -160, -120, -120, -80, -40, -20, 0, 20, 40, 80, 120, 120, 160, 160, 160 };
/******************************************************************************/
struct Thing *create_effect_element(const struct Coord3d *pos, unsigned short a2, unsigned short a3)
{
  return _DK_create_effect_element(pos, a2, a3);
}

void process_spells_affected_by_effect_elements(struct Thing *thing)
{
  _DK_process_spells_affected_by_effect_elements(thing);
}

void process_thing_spell_effects(struct Thing *thing)
{
  _DK_process_thing_spell_effects(thing);
}

long move_effect_element(struct Thing *thing)
{
  SYNCDBG(18,"Starting");
  return _DK_move_effect_element(thing);
}

void change_effect_element_into_another(struct Thing *thing, long nmodel)
{
    SYNCDBG(18,"Starting");
    return _DK_change_effect_element_into_another(thing,nmodel);
}

long update_effect_element(struct Thing *thing)
{
    struct EffectElementStats *eestats;
    long health;
    long abs_x,abs_y;
    long prop_factor,prop_val;
    long i;
    SYNCDBG(18,"Starting");
    //return _DK_update_effect_element(thing);
    if (thing->model < sizeof(effect_element_stats)/sizeof(effect_element_stats[0])) {
        eestats = &effect_element_stats[thing->model];
    } else {
        ERRORLOG("Outranged model %d",(int)thing->model);
        eestats = &effect_element_stats[0];
    }
    // Check if effect health dropped to zero; delete it, or decrease health for the next check
    health = thing->health;
    if (health <= 0)
    {
        if (eestats->transform_model != 0)
        {
            change_effect_element_into_another(thing, eestats->transform_model);
        } else
        {
            delete_thing_structure(thing, 0);
        }
        return 0;
    }
    thing->health = health-1;
    // Set dynamic properties of the effect
    if (!eestats->field_12)
    {
        if (thing->field_60 >= (int)thing->mappos.z.val)
          thing->field_3E = 0;
    }
    if (eestats->field_15)
    {
        thing->field_25 &= ~0x01;
        thing->field_25 &= ~0x02;
        if (thing_touching_floor(thing))
        {
            i = get_top_cube_at(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
            switch (i)
            {
            case 39:
                thing->field_25 |= 0x01;
                break;
            case 40:
            case 41:
                thing->field_25 |= 0x02;
                break;
            }
        }
    }
    i = eestats->subeffect_delay;
    if (i > 0)
    {
      if (((thing->field_9 - game.play_gameturn) % i) == 0) {
          create_effect_element(&thing->mappos, eestats->subeffect_model, thing->owner);
      }
    }
    switch (eestats->field_1)
    {
    case 1:
        move_effect_element(thing);
        break;
    case 2:
        i = thing->pos_2C.x.val;
        thing->pos_2C.x.val = 2*i/3;
        i = thing->pos_2C.y.val;
        thing->pos_2C.y.val = 2*i/3;
        i = thing->pos_2C.z.val;
        if (i > 32)
        {
          thing->pos_2C.z.val = 2*i/3;
        } else
        if (i > 16)
        {
          i = i-16;
          if (i < 16) i = 16;
          thing->pos_2C.z.val = i;
        } else
        if (i < -16)
        {
          thing->pos_2C.z.val = 2*i/3;
        } else
        {
            i = i+16;
            if (i > 16) i = 16;
            thing->pos_2C.z.val = i;
        }
        move_effect_element(thing);
        break;
    case 3:
        thing->pos_2C.z.val = 32;
        move_effect_element(thing);
        break;
    case 4:
        health = thing->health;
        if ((health >= 0) && (health < 16))
        {
            thing->pos_2C.z.val = bounce_table[health];
        } else
        {
            ERRORLOG("Illegal effect element bounce life: %ld", health);
        }
        move_effect_element(thing);
        break;
    case 5:
        break;
    default:
        ERRORLOG("Invalid effect element move type %d!",(int)eestats->field_1);
        move_effect_element(thing);
        break;
    }

    if (eestats->field_2 != 1)
      return 1;
    abs_x = abs(thing->pos_2C.x.val);
    abs_y = abs(thing->pos_2C.y.val);
    prop_factor = LbProportion(abs_x, abs_y);
    i = ((LbArcTan(thing->pos_2C.z.val, prop_factor) & 0x7FF) - 512) & 0x7FF;
    if (i > 1024)
      i -= 1024;
    prop_val = i / 128;
    thing->field_52 = LbArcTan(thing->pos_2C.x.val, thing->pos_2C.y.val) & 0x7FF;
    thing->field_48 = prop_val;
    thing->field_3E = 0;
    thing->field_40 = (prop_val & 0xff) << 8;
    SYNCDBG(18,"Finished");
    return 1;
}

struct Thing *create_effect_generator(struct Coord3d *pos, unsigned short a1, unsigned short a2, unsigned short a3, long a4)
{
  return _DK_create_effect_generator(pos, a1, a2, a3, a4);
}

long move_effect(struct Thing *thing)
{
  return _DK_move_effect(thing);
}

TbBool effect_can_affect_thing(struct Thing *efftng, struct Thing *thing)
{
  if (thing_is_invalid(efftng) || thing_is_invalid(thing))
  {
    WARNLOG("Invalid thing tries to interact with other things");
    return false;
  }
  switch (efftng->byte_16)
  {
  case 1:
      return thing_is_shootable_by_any_player_including_objects(thing);
  case 2:
      return thing_is_shootable_by_any_player_excluding_objects(thing);
  case 3:
      return thing_is_shootable_by_any_player_except_own_including_objects(efftng, thing);
  case 4:
      return thing_is_shootable_by_any_player_except_own_excluding_objects(efftng, thing);
  case 7:
      if (thing_is_dungeon_heart(thing) && (thing->owner != efftng->owner))
        return true;
      return false;
  case 8:
      return false;
  default:
      WARNLOG("Thing has no hit thing type");
      return false;
  }
}

void update_effect_light_intensity(struct Thing *thing)
{
  long i;
  if (thing->field_62 != 0)
  {
      if (thing->health < 4)
      {
        i = light_get_light_intensity(thing->field_62);
        light_set_light_intensity(thing->field_62, (3*i)/4);
      }
  }
}

void effect_generate_effect_elements(const struct Thing *thing)
{
  const struct InitEffect *effnfo;
  struct PlayerInfo *player;
  struct Thing *elemtng;
  struct Coord3d pos;
  long i,k,n;
  long mag;
  unsigned long arg,argZ;
  effnfo = &effect_info[thing->model];
  SYNCDBG(18,"Preparing Effect, Generation Type %d",(int)effnfo->generation_type);
  switch (effnfo->generation_type)
  {
  case 1:
        for (i=0; i < effnfo->field_B; i++)
        {
          if (effnfo->kind_min > 0)
          {
            n = effnfo->kind_min + ACTION_RANDOM(effnfo->kind_max - effnfo->kind_min + 1);
            elemtng = create_effect_element(&thing->mappos, n, thing->owner);
            if (thing_is_invalid(elemtng))
              break;
            arg = ACTION_RANDOM(0x800);
            argZ = ACTION_RANDOM(0x400);
            // Setting XY acceleration
            k = abs(effnfo->accel_xy_max - effnfo->accel_xy_min);
            if (k <= 1) k = 1;
            mag = effnfo->accel_xy_min + ACTION_RANDOM(k);
            elemtng->pos_32.x.val += (mag*LbSinL(arg)) >> 16;
            elemtng->pos_32.y.val -= (mag*LbCosL(arg)) >> 16;
            // Setting Z acceleration
            k = abs(effnfo->accel_z_max - effnfo->accel_z_min);
            if (k <= 1) k = 1;
            mag = effnfo->accel_z_min + ACTION_RANDOM(k);
            elemtng->pos_32.z.val += (mag*LbSinL(argZ)) >> 16;
            elemtng->field_1 |= 0x04;
          }
        }
        break;
  case 2:
        k = 0;
        for (i=0; i < effnfo->field_B; i++)
        {
            n = effnfo->kind_min + ACTION_RANDOM(effnfo->kind_max - effnfo->kind_min + 1);
            mag = effnfo->numfield_0 - thing->health;
            arg = (mag << 7) + k/effnfo->field_B;
            pos.x.val = thing->mappos.x.val + ((16*mag*LbSinL(arg)) >> 16);
            pos.y.val = thing->mappos.y.val - ((16*mag*LbCosL(arg)) >> 16);
            pos.z.val = thing->mappos.z.val;
            elemtng = create_effect_element(&pos, n, thing->owner);
            k += 2048;
        }
        break;
  case 3:
        k = 0;
        for (i=0; i < effnfo->field_B; i++)
        {
            n = effnfo->kind_min + ACTION_RANDOM(effnfo->kind_max - effnfo->kind_min + 1);
            mag = thing->health;
            arg = (mag << 7) + k/effnfo->field_B;
            pos.x.val = thing->mappos.x.val + ((16*mag*LbSinL(arg)) >> 16);
            pos.y.val = thing->mappos.y.val - ((16*mag*LbCosL(arg)) >> 16);
            pos.z.val = thing->mappos.z.val;
            elemtng = create_effect_element(&pos, n, thing->owner);
            k += 2048;
        }
        break;
  case 4:
        if (thing->model != 48)
          break;
        i = effnfo->numfield_0 / 2;
        if (thing->health == effnfo->numfield_0)
        {
          memset(temp_pal, 63, PALETTE_SIZE);
        } else
        if (thing->health > i)
        {
          LbPaletteFade(temp_pal, i, Lb_PALETTE_FADE_OPEN);
        } else
        if (thing->health == i)
        {
          LbPaletteStopOpenFade();
          LbPaletteSet(temp_pal);
        } else
        if (thing->health > 0)
        {
          LbPaletteFade(_DK_palette, 8, Lb_PALETTE_FADE_OPEN);
        } else
        {
          player = get_my_player();
          PaletteSetPlayerPalette(player, _DK_palette);
          LbPaletteStopOpenFade();
        }
        break;
  default:
        ERRORLOG("Unknown Effect Generation Type %d",(int)effnfo->generation_type);
        break;
  }
}

long process_effect_generator(struct Thing *thing)
{
  SYNCDBG(18,"Starting");
  return _DK_process_effect_generator(thing);
}

struct Thing *create_effect(const struct Coord3d *pos, unsigned short a2, unsigned char a3)
{
  return _DK_create_effect(pos, a2, a3);
}

void create_special_used_effect(const struct Coord3d *pos, long plyr_idx)
{
  create_effect(pos, 67, plyr_idx);
}

TbBool destroy_effect_generator(struct Thing *thing)
{
  if (thing->model == 43)
  {
      place_slab_type_on_map(12, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing->owner, 0);
      do_slab_efficiency_alteration(map_to_slab[thing->mappos.x.stl.num], map_to_slab[thing->mappos.y.stl.num]);
  }
  if (thing->field_66 != 0)
  {
      S3DDestroySoundEmitter(thing->field_66);
      thing->field_66 = 0;
  }
  delete_thing_structure(thing, 0);
  return true;
}

/**
 * Computes damage the Word Of Power spell should make to given thing.
 * @param efftng The thing being source of the spell.
 * @param dsttng The target thing to be affected by the spell.
 */
long get_word_of_power_damage(const struct Thing *efftng, const struct Thing *dsttng)
{
  long distance;
  distance = get_2d_box_distance(&dsttng->mappos, &efftng->mappos);
  // TODO: SPELLS the damage and the distance should be in config files.
  return get_radially_decaying_value(150,640,640,distance);
}

/**
 * Computes and applies damage the Word Of Power spell makes to things at given map block.
 */
void word_of_power_affecting_map_block(struct Thing *efftng, struct Thing *owntng, struct Map *mapblk)
{
  struct Thing *thing;
  long damage;
  long i;
  unsigned long k;
  k = 0;
  i = get_mapwho_thing_index(mapblk);
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
    i = thing->field_2;
    if (effect_can_affect_thing(efftng, thing)
      || ((thing->class_id == TCls_Door) && (thing->owner != owntng->owner)))
    {
      damage = get_word_of_power_damage(efftng, thing);
      apply_damage_to_thing_and_display_health(thing, damage, owntng->owner);
    }
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
}

/**
 * Applies damage the Word Of Power spell makes to all things in the area surrounding given position.
 */
void word_of_power_affecting_area(struct Thing *efftng, struct Thing *owntng, struct Coord3d *pos)
{
  struct Map *mapblk;
  long stl_xmin,stl_xmax;
  long stl_ymin,stl_ymax;
  long stl_x,stl_y;
  if (efftng->field_9 != game.play_gameturn)
    return;
  stl_xmin = pos->x.stl.num - 5;
  stl_xmax = pos->x.stl.num + 6;
  stl_ymin = pos->y.stl.num - 5;
  stl_ymax = pos->y.stl.num + 6;
  if (stl_xmin < 0)
  {
    stl_xmin = 0;
  } else
  if (stl_xmin > map_subtiles_x)
  {
    stl_xmin = map_subtiles_x;
  }
  if (stl_ymin < 0)
  {
    stl_ymin = 0;
  } else
  if (stl_ymin > map_subtiles_y)
  {
    stl_ymin = map_subtiles_y;
  }
  if (stl_xmax < 0)
  {
    stl_xmax = 0;
  } else
  if (stl_xmax > map_subtiles_x)
  {
    stl_xmax = map_subtiles_x;
  }
  if (stl_ymax < 0)
  {
    stl_ymax = 0;
  } else
  if (stl_ymax > map_subtiles_y)
  {
    stl_ymax = map_subtiles_y;
  }
  for (stl_y=stl_ymin; stl_y <= stl_ymax; stl_y++)
  {
    for (stl_x=stl_xmin; stl_x <= stl_xmax; stl_x++)
    {
      mapblk = get_map_block_at(stl_x, stl_y);
      word_of_power_affecting_map_block(efftng, owntng, mapblk);
    }
  }
}

void poison_cloud_affecting_area(struct Thing *owntng, struct Coord3d *pos, long a3, long a4, unsigned char a5)
{
  _DK_poison_cloud_affecting_area(owntng, pos, a3, a4, a5);
}

long update_effect(struct Thing *thing)
{
  struct InitEffect *effnfo;
  struct Thing *subtng;
  SYNCDBG(18,"Starting for %s",thing_model_name(thing));
  //return _DK_update_effect(thing);
  subtng = NULL;
  effnfo = &effect_info[thing->model];
  if ( thing->field_1D )
    subtng = thing_get(thing->field_1D);
  if (thing->health <= 0)
  {
    destroy_effect_generator(thing);
    return 0;
  }
  update_effect_light_intensity(thing);
  // Effect generators can be used to generate effect elements
  if ( (effnfo->field_11 == 0) || any_player_close_enough_to_see(&thing->mappos) )
  {
    effect_generate_effect_elements(thing);
  }
  // Let the effect affect area
  switch (effnfo->area_affect_type)
  {
  case 1:
  case 3:
      poison_cloud_affecting_area(subtng, &thing->mappos, 1280, 60, effnfo->area_affect_type);
      break;
  case 4:
      word_of_power_affecting_area(thing, subtng, &thing->mappos);
      break;
  }
  thing->health--;
  return move_effect(thing);
}

struct Thing *create_price_effect(const struct Coord3d *pos, long plyr_idx, long price)
{
  struct Thing *thing;
  thing = create_effect_element(pos, 41, plyr_idx);
  if (!thing_is_invalid(thing))
    thing->long_13 = price;
  return thing;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
