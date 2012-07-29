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
#include "thing_physics.h"
#include "thing_navigate.h"
#include "front_simple.h"
#include "map_data.h"
#include "creature_graphics.h"
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
//extern struct EffectGeneratorStats _DK_effect_generator_stats[6];
//#define effect_element_stats _DK_effect_element_stats
/******************************************************************************/
struct EffectGeneratorStats effect_generator_stats[] = {
    { 0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0},
    {10, 20,  1, 30, 1,  0,-40, 40,-40, 40, 80,150,147,  3, 0},
    {10, 20,  1, 31, 0, -1,  0,  0,  0,  0,  0,  0,  0,  0, 0},
    { 0,  0,  5, 33, 0, -1,  0,  0,  0,  0,  0,  0,  0,  0, 0},
    { 0,  2,  1, 37, 0,256,-15, 15,-15, 15,  0,  0,  0,  0, 0},
    { 2,  5,  1, 37, 0,  0,-15, 15,-15, 15,  0,  0,  0,  0, 0}
};

struct EffectElementStats effect_element_stats[] = {
   // [0]
   {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 2, 0, -1, -1, 926, 152, 192, 1, 192, 256, 1, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
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
    // [5]
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
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    // [10]
   {2, 1, 0,    -1,    -1, 798, 50, 256, 1, 256, 320, 1, 1,
    3, 0, 0, 2, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256,
    0, 0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1},
   {2, 1, 0,    15,    15, 908, 256, 256, 1, 256, 256, 1, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
   {2, 1, 0,    15,    15, 913, 226, 256, 0, 256, 256, 0, 1,
    0, 1, 0, 0, 8, 0, 102, 0, 0, 0, 0, 0, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 1, 0,    15,    15, 914, 226, 256, 0, 256, 256, 0, 1,
    0, 1, 0, 0, 8, 0, 102, 0, 0, 0, 0, 0, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 1, 0,    15,    15, 915, 226, 256, 0, 256, 256, 0, 1,
    0, 1, 0, 0, 8, 0, 102, 0, 0, 0, 0, 0, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
    // [15]
   {2, 5, 0,     1,     1, 964, 24, 24, 0, 256, 256, 1, 1, 3, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
   {2, 5, 0,    -1,    -1, 918, 96, 96, 0, 256, 256, 1, 1,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0,     1,     1, 917, 96, 96, 0, 256, 256, 1, 1, 3, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0,     6,     6, 964, 64, 64, 0, 256, 256, 1, 1, 3, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0,     1,     1, 981, 192, 192, 0, 256, 256, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0,
    0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    // [20]
   {2, 5, 0, 20000, 20000, 907, 192, 192, 0, 256, 256,
    1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
   {2, 1, 0,    40,    40, 919, 320, 374, 0, 128, 128, 1, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
   {2, 5, 0,    -1,    -1, 909, 250, 300, 1, 128, 128, 1, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
   {2, 5, 0,    -1,    -1, 836, 200, 256, 1, 16, 32, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0,
    0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 1, 0,    40,    50, 910, 150, 180, 0, 256, 256, 0, 0,
    2, 1, 0, 0, 8, 0, 102, 0, 0, 0, 0, 0, 1, 0, 0,
    256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 25, 0, 0, 0, 0, 0, 0, 0, 1},
    // [25]
   {2, 1, 0,    -1,    -1, 911, 150, 180, 1, 16, 32, 1, 0,
    2, 1, 0, 0, 8, 0, 102, 0, 0, 0, 0, 0, 1, 0, 0,
    256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 1, 0,    30,    40, 834, 122, 142, 0, 192, 256, 0, 0,
    0, 1, 0, 0, 40, 0, 102, 0, 0, 0, 0, 0, 1, 0, 0,
    256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 1, 0,    30,    40, 833, 122, 142, 0, 192, 256, 0, 0,
    0, 1, 0, 0, 40, 0, 102, 0, 0, 0, 0, 0, 1, 0, 0,
    256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 1, 0,    30,    40, 832, 122, 142, 0, 192, 256, 0, 0,
    0, 1, 0, 0, 40, 0, 102, 0, 0, 0, 0, 0, 1, 0, 0,
    256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 1, 0,    -1,    -1, 828, 250, 300, 1, 128, 128, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    // [30]
   {2, 1, 0,    30,    40, 823, 122, 142, 0, 192, 256, 0, 0,
    3, 1, 0, 0, 10, 0, 16, 0, 0, 0, 32, 1, 1, 0, 0,
    256, 0, 19, 36, 80, 1, 0, 0, 256, 1, 0, 1280, 52, 0,
    0, 0, 0, 0, 1},
   {2, 1, 0,    30,    40, 827, 122, 142, 0, 192, 256, 0, 0,
    2, 1, 0, 0, 10, 0, 102, 0, 0, 0, 0, 0, 1, 19, 36,
    80, 1, 19, 36, 80, 1, 0, 0, 256, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 5, 0,    -1,    -1, 824, 122, 142, 1, 192, 256, 0, 0,
    3, 1, 0, 0, 10, 0, 102, 0, 0, 0, 0, 0, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0},
   {2, 1, 0,    10,    15, 832, 122, 142, 0, 192, 256, 0, 0,
    0, 1, 0, 0, 10, 0, 102, 0, 0, 0, 0, 0, 1, 0, 0,
    256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 5, 0,    -1,    -1, 0, 450, 450, 1, 256, 256, 1, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0,
    0, 256, 0, 0, 0, 256, 0, 35, 0, 0, 0, 0, 0, 0, 0, 0},
    // [35]
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
    // [40]
   {2, 5, 0, -1, -1, 828, 250, 300, 1, 128, 128, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
   {4, 4, 0, 16, 16, 0, 256, 256, 0, 256, 256, 1, 1, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0,
    0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, 2, 4, 964, 128, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, -1, -1, 852, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, 4, 4, 852, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, -1, -1, 802, 172, 196, 0, 256, 256, 1, 1,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, 100, 100, -1, 122, 142, 0, 192, 256, 0, 0,
    0, 1, 0, 0, 40, 0, 102, 0, 0, 0, 0, 0, 1, 0, 0,
    256, 0, 19, 36, 80, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 1, 0, 20, 30, 919, 320, 374, 0, 128, 128, 1, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
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
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
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
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 1, 0, 5, 5, 964, 96, 160, 1, 85, 85, 1, 1, 3, 0,
    0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, -1, -1, 120, 256, 256, 1, 256, 256, 1, 1,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
   {2, 5, 0, 1, 1, -1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
   {2, 2, 0, -1, -1, 116, 256, 256, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 2, 0, -1, -1, 117, 256, 256, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 2, 0, -1, -1, 118, 256, 256, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 2, 0, -1, -1, 119, 256, 256, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 2, 0, -1, -1, 116, 358, 358, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 2, 0, -1, -1, 117, 358, 358, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 2, 0, -1, -1, 118, 358, 358, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 2, 0, -1, -1, 119, 358, 358, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 2, 0, -1, -1, 116, 460, 460, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 2, 0, -1, -1, 117, 460, 460, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 2, 0, -1, -1, 118, 460, 460, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 2, 0, -1, -1, 119, 460, 460, 0, 128, 128, 1, 1,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 1, 0, -1, -1, 966, 172, 255, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 1, 0, -1, -1, 967, 172, 255, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 1, 0, -1, -1, 968, 172, 255, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 1, 0, -1, -1, 969, 172, 255, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 1, 0, -1, -1, 857, 128, 128, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 1, 0, -1, -1, 858, 128, 128, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 1, 0, -1, -1, 859, 128, 128, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 1, 0, -1, -1, 860, 128, 128, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, -1, -1, 852, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 1, 1, 30, 40, 96, 160, 256, 0, 192, 256, 0, 0,
    1, 1, 0, 0, 10, 0, 102, 0, 0, 0, 0, 0, 1, 66, 0,
    256, 1, 66, 36, 100, 1, 0, 0, 256, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 1},
   {2, 5, 0, -1, -1, 97, 250, 300, 1, 128, 128, 1, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
   {2, 5, 0, -1, -1, 853, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, -1, -1, 854, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, -1, -1, 855, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 1, 0, -1, -1, 110, 225, 270, 1, 1024, 1024, 1, 0,
    2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
   {2, 5, 0, -1, -1, 853, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, -1, -1, 854, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, -1, -1, 855, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 1, 0, 5, 5, 917, 96, 160, 1, 85, 85, 1, 1, 3, 0,
    0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0,
    256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, -1, -1, 837, 200, 256, 1, 16, 32, 1, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

long const bounce_table[] = { -160, -160, -120, -120, -80, -40, -20, 0, 20, 40, 80, 120, 120, 160, 160, 160 };
/******************************************************************************/
struct Thing *create_effect_element(const struct Coord3d *pos, unsigned short eelmodel, unsigned short owner)
{
    struct InitLight ilght;
    struct EffectElementStats *eestat;
    struct Thing *thing;
    long i,n;
    //return _DK_create_effect_element(pos, eelmodel, owner);

    if (!i_can_allocate_free_thing_structure(FTAF_Default)) {
        return INVALID_THING;
    }
    if (!any_player_close_enough_to_see(pos)) {
        return INVALID_THING;
    }
    eestat = &effect_element_stats[eelmodel];
    memset(&ilght, 0, sizeof(struct InitLight));
    thing = allocate_free_thing_structure(FTAF_Default);
    if (thing->index == 0) {
        ERRORDBG(8,"Should be able to allocate effect element %d for player %d, but failed.",(int)eelmodel,(int)owner);
        return INVALID_THING;
    }
    thing->class_id = TCls_EffectElem;
    thing->model = eelmodel;
    thing->mappos.x.val = pos->x.val;
    thing->mappos.y.val = pos->y.val;
    thing->mappos.z.val = pos->z.val;
    thing->field_2 = 0;
    thing->parent_thing_idx = thing->index;
    thing->owner = owner;
    thing->sizexy = 1;
    thing->field_58 = 1;
    thing->field_5A = 1;
    thing->field_5C = 1;

    if (eestat->numfield_7 != -1)
    {
        i = ACTION_RANDOM(eestat->numfield_B - (long)eestat->numfield_9 + 1);
        n = ACTION_RANDOM(eestat->field_10   - (long)eestat->field_E    + 1);
        set_thing_draw(thing, eestat->numfield_7, eestat->field_E + n, eestat->numfield_9 + i, 0, 0, eestat->field_0);
        set_flag_byte(&thing->field_4F,0x02,eestat->field_13);
        thing->field_4F ^= (thing->field_4F ^ (0x10 * eestat->field_14)) & 0x30;
        set_flag_byte(&thing->field_4F,0x40,eestat->field_D);
    } else
    {
        set_flag_byte(&thing->field_4F,0x01,true);
    }

    thing->field_20 = eestat->field_18;
    thing->field_23 = eestat->field_1A;
    thing->field_24 = eestat->field_1C;
    thing->movement_flags |= TMvF_Unknown08;
    set_flag_byte(&thing->movement_flags,TMvF_Unknown10,eestat->field_16);
    thing->field_9 = game.play_gameturn;

    if (eestat->numfield_3 > 0)
    {
        i = ACTION_RANDOM(eestat->numfield_5 - (long)eestat->numfield_3 + 1);
        thing->health = eestat->numfield_3 + i;
    } else
    {
        thing->health = get_lifespan_of_animation(thing->field_44, thing->field_3E);
    }

    if (eestat->field_17 != 0)
    {
        thing->field_4B = eestat->numfield_9;
        thing->field_4D = eestat->numfield_B;
        if (eestat->field_17 == 2)
        {
            thing->field_4A = 2 * (eestat->numfield_B - (long)eestat->numfield_9) / thing->health;
            thing->field_50 |= 0x02;
        }
        else
        {
            thing->field_4A = (eestat->numfield_B - (long)eestat->numfield_9) / thing->health;
            thing->field_50 &= ~0x02;
        }
        thing->field_46 = eestat->numfield_9;
    } else
    {
        thing->field_4A = 0;
    }

    if (eestat->field_3A != 0)
    {
        ilght.mappos.x.val = thing->mappos.x.val;
        ilght.mappos.y.val = thing->mappos.y.val;
        ilght.mappos.z.val = thing->mappos.z.val;
        ilght.field_0 = eestat->field_3A;
        ilght.field_2 = eestat->field_3C;
        ilght.is_dynamic = 1;
        ilght.field_3 = eestat->field_3D;
        thing->light_id = light_create_light(&ilght);
        if (thing->light_id <= 0) {
            SYNCDBG(8,"Cannot allocate dynamic light to %s.",thing_model_name(thing));
        }
    }
    add_thing_to_list(thing, &game.thing_lists[TngList_EffectElems]);
    place_thing_in_mapwho(thing);
    return thing;
}

void process_spells_affected_by_effect_elements(struct Thing *thing)
{
  _DK_process_spells_affected_by_effect_elements(thing);
}

void process_thing_spell_effects(struct Thing *thing)
{
  _DK_process_thing_spell_effects(thing);
}

void move_effect_blocked(struct Thing *thing, struct Coord3d *prev_pos, struct Coord3d *next_pos)
{
    struct EffectElementStats *effstat;
    long cube_id,sample_id;
    unsigned short effmodel;
    unsigned long blocked_flags;
    effstat = &effect_element_stats[thing->model];
    blocked_flags = get_thing_blocked_flags_at(thing, next_pos);
    slide_thing_against_wall_at(thing, next_pos, blocked_flags);
    if ( ((blocked_flags & 0x04) != 0) && effstat->field_15 && effstat->field_22 )
    {
        struct Thing *efftng;
        efftng = thing;
        cube_id = get_top_cube_at(next_pos->x.stl.num, next_pos->y.stl.num);
        if (cube_id == 39)
        {
          effmodel = effstat->field_2A;
          if (effmodel > 0) {
              efftng = create_effect(prev_pos, effmodel, thing->owner);
          }
          sample_id = effstat->field_2C;
          if (sample_id > 0) {
              thing_play_sample(efftng, sample_id, 100, 0, 3, 0, 2, effstat->field_2E);
          }
          if ( effstat->field_30 )
              thing->health = 0;
        } else
        if ( (cube_id == 40) || (cube_id == 41) )
        {
            effmodel = effstat->field_31;
            if (effmodel > 0) {
                efftng = create_effect(prev_pos, effmodel, thing->owner);
            }
            sample_id = effstat->field_33;
            if (sample_id > 0) {
                thing_play_sample(efftng, sample_id, 100, 0, 3, 0, 2, effstat->field_35);
            }
            if ( effstat->field_37 )
                thing->health = 0;
        } else
        {
            effmodel = effstat->field_23;
            if (effmodel > 0) {
                efftng = create_effect(prev_pos, effmodel, thing->owner);
            }
            sample_id = effstat->field_25;
            if (sample_id > 0) {
                thing_play_sample(efftng, sample_id, 100, 0, 3, 0, 2, effstat->field_27);
            }
            if ( effstat->field_29 )
                thing->health = 0;
        }
    }
    remove_relevant_forces_from_thing_after_slide(thing, next_pos, blocked_flags);
}

long move_effect_element(struct Thing *thing)
{
    struct Coord3d pos;
    SYNCDBG(18,"Starting");
    //return _DK_move_effect_element(thing);
    set_coords_add_velocity(&pos, &thing->mappos, &thing->velocity, false);
    if ( positions_equivalent(&thing->mappos, &pos) ) {
        return 1;
    }
    if ((thing->movement_flags & 0x10) == 0)
    {
       if ( !thing_covers_same_blocks_in_two_positions(thing, &thing->mappos, &pos) )
       {
           if ( thing_in_wall_at(thing, &pos) )
           {
               move_effect_blocked(thing, &thing->mappos, &pos);
           }
       }
    }
    move_thing_in_map(thing, &pos);
    return 1;
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
        thing->movement_flags &= ~TMvF_Unknown01;
        thing->movement_flags &= ~TMvF_Unknown02;
        if (thing_touching_floor(thing))
        {
            i = get_top_cube_at(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
            switch (i)
            {
            case 39:
                thing->movement_flags |= TMvF_Unknown01;
                break;
            case 40:
            case 41:
                thing->movement_flags |= TMvF_Unknown02;
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
    prop_factor = LbDiagonalLength(abs_x, abs_y);
    i = ((LbArcTanAngle(thing->pos_2C.z.val, prop_factor) & 0x7FF) - 512) & 0x7FF;
    if (i > 1024)
      i -= 1024;
    prop_val = i / 128;
    thing->field_52 = LbArcTanAngle(thing->pos_2C.x.val, thing->pos_2C.y.val) & 0x7FF;
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
  if (thing->light_id != 0)
  {
      if (thing->health < 4)
      {
          i = light_get_light_intensity(thing->light_id);
          light_set_light_intensity(thing->light_id, (3*i)/4);
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
              if (effnfo->kind_min <= 0)
                  continue;
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
              elemtng->acceleration.x.val += (mag*LbSinL(arg)) >> 16;
              elemtng->acceleration.y.val -= (mag*LbCosL(arg)) >> 16;
              // Setting Z acceleration
              k = abs(effnfo->accel_z_max - effnfo->accel_z_min);
              if (k <= 1) k = 1;
              mag = effnfo->accel_z_min + ACTION_RANDOM(k);
              elemtng->acceleration.z.val += (mag*LbSinL(argZ)) >> 16;
              elemtng->field_1 |= 0x04;
          }
          break;
    case 2:
          k = 0;
          for (i=0; i < effnfo->field_B; i++)
          {
              n = effnfo->kind_min + ACTION_RANDOM(effnfo->kind_max - effnfo->kind_min + 1);
              mag = effnfo->start_health - thing->health;
              arg = (mag << 7) + k/effnfo->field_B;
              set_coords_to_cylindric_shift(&pos, &thing->mappos, mag, arg, 0);
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
              set_coords_to_cylindric_shift(&pos, &thing->mappos, 16*mag, arg, 0);
              elemtng = create_effect_element(&pos, n, thing->owner);
              k += 2048;
          }
          break;
    case 4:
        if (thing->model != 48)
            break;
        i = effnfo->start_health / 2;
        if (thing->health == effnfo->start_health)
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
    struct EffectGeneratorStats *egenstat;
    struct Thing *efftng;
    struct Coord3d pos;
    long deviation_angle,deviation_mag;
    long i,k;
    SYNCDBG(18,"Starting");
    //return _DK_process_effect_generator(thing);
    if (thing->health > 0)
        thing->health--;
    if (thing->health == 0)
    {
        delete_thing_structure(thing, 0);
        return 0;
    }
    if ( !any_player_close_enough_to_see(&thing->mappos) )
    {
        SYNCDBG(18,"No player sees %s at (%d,%d,%d)",thing_model_name(thing),(int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num,(int)thing->mappos.z.stl.num);
        return 1;
    }
    if (thing->long_15 > 0)
        thing->long_15--;
    if (thing->long_15 > 0)
    {
        return 1;
    }
    egenstat = &effect_generator_stats[thing->model];
    for (i=0; i < egenstat->genation_amount; i++)
    {
        deviation_angle = ACTION_RANDOM(0x800);
        deviation_mag = ACTION_RANDOM(thing->word_13 + 1);
        set_coords_to_cylindric_shift(&pos, &thing->mappos, deviation_mag, deviation_angle, 0);
        SYNCDBG(18,"The %s creates effect %d/%d at (%d,%d,%d)",thing_model_name(thing),(int)pos.x.val,(int)pos.y.val,(int)pos.z.val);
        efftng = create_effect_element(&pos, egenstat->field_C, thing->owner);
        if (thing_is_invalid(efftng))
            break;
        efftng->sizexy = 20;
        efftng->field_58 = 20;
        if (egenstat->field_10)
        {
            k = egenstat->field_11;
        } else
        if (egenstat->field_11 == -1)
        {
            efftng->mappos.z.val = (8 << 8);
            k = get_next_gap_creature_can_fit_in_below_point(efftng, &efftng->mappos);
        } else
        {
            k = egenstat->field_11 + get_thing_height_at(efftng, &efftng->mappos);
        }
        efftng->mappos.z.val = k;
        if ( thing_in_wall_at(efftng, &efftng->mappos) )
        {
            SYNCDBG(18,"The %s created effect %d/%d in wall, removing",thing_model_name(thing),(int)i,(int)egenstat->genation_amount);
            delete_thing_structure(efftng, 0);
        } else
        {
            SYNCDBG(18,"The %s created effect %d/%d, index %d",thing_model_name(thing),(int)i,(int)egenstat->genation_amount,(int)efftng->index);
            long acc_x,acc_y,acc_z;
            struct Thing *sectng;
            acc_x = egenstat->acc_x_min + ACTION_RANDOM(egenstat->acc_x_max - egenstat->acc_x_min + 1);
            acc_y = egenstat->acc_y_min + ACTION_RANDOM(egenstat->acc_y_max - egenstat->acc_y_min + 1);
            acc_z = egenstat->acc_z_min + ACTION_RANDOM(egenstat->acc_z_max - egenstat->acc_z_min + 1);
            efftng->acceleration.x.val += acc_x;
            efftng->acceleration.y.val += acc_y;
            efftng->acceleration.z.val += acc_z;
            efftng->field_1 |= 0x04;
            if (egenstat->sound_sample_idx > 0)
            {
                sectng = create_effect(&efftng->mappos, 49, thing->owner);
                if (!thing_is_invalid(sectng)) {
                    thing_play_sample(sectng, egenstat->sound_sample_idx + ACTION_RANDOM(egenstat->sound_sample_rng), 100, 0, 3, 0, 2, 256);
                }
            }
            if (egenstat->sound_sample_sec > 0) {
                thing_play_sample(efftng, egenstat->sound_sample_sec, 100, 0, 3, 0, 2, 256);
            }
        }
    }
    thing->long_15 = egenstat->genation_delay_min + ACTION_RANDOM(egenstat->genation_delay_max - egenstat->genation_delay_min + 1);
    return 1;
}

struct Thing *create_effect(const struct Coord3d *pos, unsigned short effmodel, unsigned char owner)
{
    struct Thing *thing;
    struct InitEffect *ieffect;
    //return _DK_create_effect(pos, effmodel, owner);
    ieffect = &effect_info[effmodel];
    if (!i_can_allocate_free_thing_structure(1)) {
        return INVALID_THING;
    }
    thing = allocate_free_thing_structure(1);
    if (thing->index == 0) {
        ERRORDBG(8,"Should be able to allocate effect %d for player %d, but failed.",(int)effmodel,(int)owner);
        return INVALID_THING;
    }
    thing->field_9 = game.play_gameturn;
    thing->class_id = TCls_Effect;
    thing->model = effmodel;
    thing->mappos.x.val = pos->x.val;
    thing->mappos.y.val = pos->y.val;
    thing->mappos.z.val = pos->z.val;
    thing->field_2 = 0;
    thing->owner = owner;
    thing->parent_thing_idx = thing->index;
    thing->field_20 = 0;
    thing->field_23 = 0;
    thing->field_24 = 0;
    thing->field_4F |= 0x01;
    thing->health = ieffect->start_health;
    if (ieffect->ilght.field_0 != 0)
    {
        struct InitLight ilght;
        memcpy(&ilght, &ieffect->ilght, sizeof(struct InitLight));
        ilght.is_dynamic = 1;
        ilght.mappos.x.val = thing->mappos.x.val;
        ilght.mappos.y.val = thing->mappos.y.val;
        ilght.mappos.z.val = thing->mappos.z.val;
        thing->light_id = light_create_light(&ilght);
        if (thing->light_id == 0) {
            // Note that there's an error here in original DK, and it makes unusable Thing entries if cannot allocate light.
            SYNCDBG(8,"Cannot allocate dynamic light to %s.",thing_model_name(thing));
        }
    }
    add_thing_to_its_class_list(thing);
    place_thing_in_mapwho(thing);
    if (ieffect->field_C != 0)
      thing_play_sample(thing, ieffect->field_C, 100, 0, 3, 0, 3, 256);
    return thing;
}

void create_special_used_effect(const struct Coord3d *pos, long plyr_idx)
{
    create_effect(pos, TngEff_Unknown67, plyr_idx);
}

TbBool destroy_effect_generator(struct Thing *thing)
{
  if (thing->model == 43)
  {
      place_slab_type_on_map(12, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing->owner, 0);
      do_slab_efficiency_alteration(map_to_slab[thing->mappos.x.stl.num], map_to_slab[thing->mappos.y.stl.num]);
  }
  if (thing->snd_emitter_id != 0)
  {
      // In case of effect, don't stop any sound samples which are still playing
      S3DDestroySoundEmitter(thing->snd_emitter_id);
      thing->snd_emitter_id = 0;
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
    if (stl_xmin < 0) {
        stl_xmin = 0;
    } else
    if (stl_xmin > map_subtiles_x) {
        stl_xmin = map_subtiles_x;
    }
    if (stl_ymin < 0) {
      stl_ymin = 0;
    } else
    if (stl_ymin > map_subtiles_y) {
      stl_ymin = map_subtiles_y;
    }
    if (stl_xmax < 0) {
      stl_xmax = 0;
    } else
    if (stl_xmax > map_subtiles_x) {
      stl_xmax = map_subtiles_x;
    }
    if (stl_ymax < 0) {
      stl_ymax = 0;
    } else
    if (stl_ymax > map_subtiles_y) {
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
    if ( thing->parent_thing_idx ) {
        subtng = thing_get(thing->parent_thing_idx);
    }
    if (thing->health <= 0) {
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
    thing = create_effect_element(pos, TngEff_Unknown41, plyr_idx);
    if (!thing_is_invalid(thing)) {
        thing->creature.gold_carried = price;
    }
    return thing;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
