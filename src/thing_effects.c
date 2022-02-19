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
 * @date     01 Jan 2010 - 01 Feb 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_effects.h"
#include "globals.h"

#include "bflib_memory.h"
#include "bflib_math.h"
#include "bflib_sound.h"
#include "bflib_planar.h"

#include "thing_objects.h"
#include "thing_list.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_factory.h"
#include "thing_navigate.h"
#include "creature_senses.h"
#include "config_creature.h"
#include "front_simple.h"
#include "map_data.h"
#include "map_blocks.h"
#include "creature_graphics.h"
#include "gui_topmsg.h"
#include "game_legacy.h"
#include "engine_redraw.h"
#include "keeperfx.hpp"
#include "gui_soundmsgs.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_move_effect(struct Thing *efftng);

/******************************************************************************/
extern struct EffectElementStats _DK_effect_element_stats[95];
//DLLIMPORT struct InitEffect _DK_effect_info[];
//#define effect_info _DK_effect_info
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

//start_health;generation_type;accel_xy_min;accel_xy_max;accel_z_min;accel_z_max;size_yz;effect_sound;kind_min;kind_max;area_affect_type;field_11;struct InitLight ilght;affected_by_wind;
struct InitEffect effect_info[] = {
    { 0, 1,   0,   0,  0,    0,  0,   0,  0,  0,  AAffT_None, 0, {0}, 0},
    { 1, 1,  32,  32, -32,  32,  1,  47,  1,  1,  AAffT_None, 1, { 512, 45, 1, 0, 0, 0, {{0},{0},{0}}, 0, 0, 0}, 1},
    { 5, 1,  32,  32, -64,  64,  5,  47,  1,  1,  AAffT_None, 1, {1024, 45, 1, 0, 0, 0, {{0},{0},{0}}, 0, 0, 0}, 1},
    {10, 1, 128, 128,-128, 128, 10,  47,  1,  1,  AAffT_None, 1, {2048, 45, 1, 0, 0, 0, {{0},{0},{0}}, 0, 0, 0}, 1},
    {10, 1, 172, 172,-172, 172,  6,  47,  1,  1,  AAffT_None, 1, {2560, 45, 1, 0, 0, 0, {{0},{0},{0}}, 0, 0, 0}, 1},
    {20, 1, 256, 256,-256, 256, 10,  47,  1,  1,  AAffT_None, 1, {2560, 45, 1, 0, 0, 0, {{0},{0},{0}}, 0, 0, 0}, 1},
    { 1, 1,  32,  32, -96,  96,  2,   0, 84, 84,  AAffT_None, 1, {0}, 1},
    { 2, 1,  32,  32, -96,  96,  2,   0, 84, 84,  AAffT_None, 1, {0}, 1},
    { 2, 1,  64,  64, -96,  96,  4,   0, 84, 84,  AAffT_None, 1, {0}, 1},
    { 3, 1,  96,  96, -96,  96,  4,   0, 84, 84,  AAffT_None, 1, {0}, 1},
    { 4, 1,  96,  96, -96,  96,  5,   0, 84, 84,  AAffT_None, 1, {0}, 1}, // [10]
    {40, 1,  44,  44, -32,  32,  2,  52,  7,  7,  AAffT_GasDamage, 1, {0}, 1},
    {40, 1,  44,  44, -32,  32,  2,  52,  7,  7,  AAffT_GasDamage, 1, {0}, 1},
    {40, 1,  44,  44, -32,  32,  2,  52,  7,  7,  AAffT_GasDamage, 1, {0}, 1},
    {10, 1, 100, 100,   1,   1, 20, 178, 10, 10,  AAffT_WOPDamage, 1, {2560, 52, 0, 0, 0, 0, {{0},{0},{0}}, 0, 0, 0}, 1},
    { 1, 1,   1,   1,   1,   1,  1,   0, 11, 11,  AAffT_None, 1, {0}, 1},
    {40, 1,  64,  64, -64,  64,  2,  52, 21, 21,  AAffT_None, 1, {0}, 1},
    {40, 1,  64,  64, -64,  64,  2,  52, 21, 21,  AAffT_None, 1, {0}, 1},
    {40, 1,  64,  64, -64,  64,  2,  52, 21, 21,  AAffT_None, 1, {0}, 1},
    { 1, 1,   1,   1,   1,   1,  1,   0, 22, 22,  AAffT_None, 1, {0}, 1},
    { 1, 1,   1,   1,   1,   1,  1,   0, 22, 22,  AAffT_None, 1, {0}, 1}, // [20]
    { 1, 1,   1,   1,   1,   1,  1,   0, 22, 22,  AAffT_None, 1, {0}, 1},
    { 1, 1,  32,  32, -96,  96,  1,   0, 24, 24,  AAffT_None, 1, {0}, 1},
    { 2, 1,  64,  64, -96,  96,  4,   0, 24, 24,  AAffT_None, 1, {0}, 1},
    { 3, 1, 128, 128, -96,  96,  4,   0, 24, 24,  AAffT_None, 1, {0}, 1},
    { 2, 1,  64,  64, -96,  96,  2,   0, 26, 26,  AAffT_None, 1, {0}, 1},
    { 2, 1,  64,  64, -96,  96,  1,   0, 27, 28,  AAffT_None, 1, {0}, 1},
    { 3, 1,  64,  64, -96,  96, 10,   0, 26, 28,  AAffT_None, 1, {0}, 1},
    { 4, 1,  16,  16, -32,  64,  3,   0, 75, 75,  AAffT_None, 1, {0}, 1},
    { 1, 1,   1,   1,   1,   1,  1,   0, 40, 40,  AAffT_None, 1, {0}, 1},
    {80, 2,   1,   1,   1,   1,  1,   0, 21, 21,  AAffT_None, 0, {0}, 1}, // [30]
    { 8, 1,  64,  64, -64,  64,  1,   0, 47, 47,  AAffT_None, 1, {0}, 1},
    { 2, 1,  64,  64, -96,  96,  2,   0, 49, 49,  AAffT_None, 1, {0}, 1},
    { 2, 1,  64,  64, -96,  96,  1,   0, 49, 51,  AAffT_None, 1, {0}, 1},
    { 3, 1,  64,  64, -96,  96, 10,   0, 50, 51,  AAffT_None, 1, {0}, 1},
    { 8, 1,  16,  16, -16,  16,  1,   0, 29, 29,  AAffT_None, 1, {0}, 0},
    {32, 1,  32,  32, -32,  32,  2,   0, 26, 28,  AAffT_None, 1, {0}, 0},
    {40, 1,  44,  44, -32,  32,  2,  52,  7,  7,  AAffT_Unkn2, 1, {0}, 1},
    {40, 1,  44,  44, -32,  32,  2,  52,  7,  7,  AAffT_Unkn2, 1, {0}, 1},
    {40, 1,  44,  44, -32,  32,  2,  52,  7,  7,  AAffT_Unkn2, 1, {0}, 1},
    {40, 1,  44,  44, -32,  32,  2,  52,  7,  7,  AAffT_GasSlow, 1, {0}, 0}, // [40]
    {40, 1,  44,  44, -32,  32,  2,  52,  7,  7,  AAffT_GasSlow, 1, {0}, 0},
    {40, 1,  44,  44, -32,  32,  2,  52,  7,  7,  AAffT_GasSlow, 1, {0}, 0},
    {16, 1, 128, 128,-128, 128,  2,  47, 26, 32,  AAffT_None, 1, {2560, 45, 1, 0, 0, 0, {{0},{0},{0}}, 0, 0, 0}, 0},
    { 1, 1,  64,  64,-128, 128,  4,   0, 53, 53,  AAffT_None, 1, {0}, 0},
    {16, 1,  96,  96, -96,  96,  4,  47, 39, 39,  AAffT_GasDamage, 1, {2560, 45, 1, 0, 0, 0, {{0},{0},{0}}, 0, 0, 0}, 1},
    { 5, 1,  64,  64, -64,  64,  4,  39, 75, 75,  AAffT_None, 1, { 768, 20, 1, 0, 0, 0, {{0},{0},{0}}, 0, 0, 0}, 1},
    {60, 3,   1,   1,   1,   1,  2,  54, 55, 58,  AAffT_None, 1, {0}, 1},
    {20, 4,   1,   1,   1,   1,  1,  47,  0,  0,  AAffT_None, 1, {0}, 1},
    {50, 4,   1,   1,   1,   1,  1,   0,  0,  0,  AAffT_None, 0, {0}, 0}, // Unknown Damage effect
    {10, 1, 128, 128,-128, 128, 10,  47,  1,  1,  AAffT_None, 1, {4096, 50, 1, 0, 0, 0, {{0},{0},{0}}, 0, 0, 0}, 1}, // [50]
    { 1, 1,   1,   1,   1,   1,  1, 112, 61, 61,  AAffT_None, 1, {0}, 1},
    { 5, 1, 128, 128,-128, 128,  5,  47,  1,  1,  AAffT_None, 1, {2048, 45, 1, 0, 0, 0, {{0},{0},{0}}, 0, 0, 0}, 1},
    {96, 1, 256, 256,-256, 256,  1, 160, 63, 74,  AAffT_None, 1, {0}, 0},
    { 8, 1,  64,  64, -64,  64,  1, 159, 63, 66,  AAffT_None, 1, {0}, 0},
    { 8, 1,  64,  64, -64,  64,  1, 159, 67, 70,  AAffT_None, 1, {0}, 0},
    { 8, 1,  64,  64, -64,  64,  1, 159, 71, 74,  AAffT_None, 1, {0}, 0},
    { 4, 1,  16,  16, -32,  64,  3,   0, 76, 76,  AAffT_None, 1, {0}, 1},
    { 4, 1,  16,  16, -32,  64,  3,   0, 77, 77,  AAffT_None, 1, {0}, 1},
    { 4, 1,  16,  16, -32,  64,  3,   0, 78, 78,  AAffT_None, 1, {0}, 1},
    { 4, 1,  16,  16, -32,  64,  3,   0, 54, 54,  AAffT_None, 1, {0}, 1}, // [60]
    { 4, 1,  16,  16, -32,  64,  3,   0, 79, 79,  AAffT_None, 1, {0}, 1},
    { 4, 1,  16,  16, -32,  64,  3,   0, 80, 80,  AAffT_None, 1, {0}, 1},
    { 4, 1,  16,  16, -32,  64,  3,   0, 81, 81,  AAffT_None, 1, {0}, 1},
    { 4, 1,  16,  16, -32,  64,  3,   0, 82, 82,  AAffT_None, 1, {0}, 1},
    { 1, 1,  32,  32, 100, 100,  2,   0, 84, 84,  AAffT_None, 1, {0}, 1},
    { 1, 1,   1,   1,   1,   1,  2,   0, 85, 85,  AAffT_None, 1, {0}, 1},
    { 4, 1,  16,  16, -32,  64,  3,   0, 75, 78,  AAffT_None, 1, {0}, 1},
    {10, 1,  20, 150, -80,  80, 20,  36, 27, 29,  AAffT_None, 1, {2560, 52, 0, 0, 0, 0, {{0},{0},{0}}, 0, 0, 0}, 1}, // [68]
    { 0, 0,   0,   0,   0,   0,  0,   0,  0,  0,  0,          0, {0}, 0},
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
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
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
    0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // #41 Floating number then gold is spent.
   {2, 5, 0, 2, 4, 964, 128, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, -1, -1, 852, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
   {2, 5, 0, 4, 4, 852, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    // [45]
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
    // [50]
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
    // [55]
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
    // [60]  part of lightning (white?)
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
    // [65]
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
    // [70]
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
    // [75]
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
    // [80]
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
    // [85]
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
    // [90] // Lightning "dot" of blue player
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
    // [95]
   {2, 5, 0, 4, 4, 851, 172, 172, 1, 256, 256, 1, 1,
    3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 0,
    0, 0, 256, 0, 0, 0, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //Copy of 45, used for CREATURE DISEASE
};

long const bounce_table[] = { -160, -160, -120, -120, -80, -40, -20, 0, 20, 40, 80, 120, 120, 160, 160, 160 };
/** Effects used when creating new imps. Every player color has different index. */
const int birth_effect_element[] = { TngEffElm_RedPuff, TngEffElm_BluePuff, TngEffElm_GreenPuff, TngEffElm_YellowPuff, TngEffElm_WhitePuff, TngEffElm_WhitePuff, };
/******************************************************************************/
TbBool thing_is_effect(const struct Thing *thing)
{
  if (thing_is_invalid(thing))
    return false;
  if (thing->class_id != TCls_Effect)
    return false;
  return true;
}

struct InitEffect *get_effect_info(ThingModel effmodel)
{
    return &effect_info[effmodel];
}

struct InitEffect *get_effect_info_for_thing(const struct Thing *thing)
{
    return &effect_info[thing->model];
}

struct EffectElementStats *get_effect_element_model_stats(ThingModel tngmodel)
{
    if (tngmodel >= sizeof(effect_element_stats)/sizeof(effect_element_stats[0]))
        return &effect_element_stats[0];
    return &effect_element_stats[tngmodel];
}

struct Thing *create_effect_element(const struct Coord3d *pos, unsigned short eelmodel, unsigned short owner)
{
    long i;
    if (!i_can_allocate_free_thing_structure(FTAF_Default)) {
        return INVALID_THING;
    }
    if (!any_player_close_enough_to_see(pos)) {
        return INVALID_THING;
    }
    struct EffectElementStats* eestat = get_effect_element_model_stats(eelmodel);
    struct InitLight ilght;
    LbMemorySet(&ilght, 0, sizeof(struct InitLight));
    struct Thing* thing = allocate_free_thing_structure(FTAF_Default);
    if (thing->index == 0) {
        ERRORDBG(8,"Should be able to allocate effect element %d for player %d, but failed.",(int)eelmodel,(int)owner);
        return INVALID_THING;
    }
    thing->class_id = TCls_EffectElem;
    thing->model = eelmodel;
    thing->mappos.x.val = pos->x.val;
    thing->mappos.y.val = pos->y.val;
    thing->mappos.z.val = pos->z.val;
    thing->next_on_mapblk = 0;
    thing->parent_idx = thing->index;
    thing->owner = owner;
    thing->clipbox_size_xy = 1;
    thing->clipbox_size_yz = 1;
    thing->solid_size_xy = 1;
    thing->solid_size_yz = 1;

    if (eestat->sprite_idx != -1)
    {
        i = EFFECT_RANDOM(thing, eestat->sprite_size_max  - (int)eestat->sprite_size_min  + 1);
        long n = EFFECT_RANDOM(thing, eestat->sprite_speed_max - (int)eestat->sprite_speed_min + 1);
        set_thing_draw(thing, eestat->sprite_idx, eestat->sprite_speed_min + n, eestat->sprite_size_min + i, 0, 0, eestat->draw_class);
        set_flag_byte(&thing->field_4F,TF4F_Unknown02,eestat->field_13);
        thing->field_4F ^= (thing->field_4F ^ (0x10 * eestat->field_14)) & (TF4F_Transpar_Flags);
        set_flag_byte(&thing->field_4F,TF4F_Unknown40,eestat->field_D);
    } else
    {
        set_flag_byte(&thing->field_4F,TF4F_Unknown01,true);
    }

    thing->fall_acceleration = eestat->field_18;
    thing->field_23 = eestat->field_1A;
    thing->field_24 = eestat->field_1C;
    thing->movement_flags |= TMvF_Unknown08;
    set_flag_byte(&thing->movement_flags,TMvF_Unknown10,eestat->field_16);
    thing->creation_turn = game.play_gameturn;

    if (eestat->numfield_3 > 0)
    {
        i = EFFECT_RANDOM(thing, eestat->numfield_5 - (long)eestat->numfield_3 + 1);
        thing->health = eestat->numfield_3 + i;
    } else
    {
        thing->health = get_lifespan_of_animation(thing->anim_sprite, thing->anim_speed);
    }

    if (eestat->field_17 != 0)
    {
        thing->field_4B = eestat->sprite_size_min;
        thing->field_4D = eestat->sprite_size_max;
        if (eestat->field_17 == 2)
        {
            thing->field_4A = 2 * (eestat->sprite_size_max - (long)eestat->sprite_size_min) / thing->health;
            thing->field_50 |= 0x02;
        }
        else
        {
            thing->field_4A = (eestat->sprite_size_max - (long)eestat->sprite_size_min) / thing->health;
            thing->field_50 &= ~0x02;
        }
        thing->sprite_size = eestat->sprite_size_min;
    } else
    {
        thing->field_4A = 0;
    }

    if (eestat->field_3A != 0)
    {
        ilght.mappos.x.val = thing->mappos.x.val;
        ilght.mappos.y.val = thing->mappos.y.val;
        ilght.mappos.z.val = thing->mappos.z.val;
        ilght.radius = eestat->field_3A;
        ilght.intensity = eestat->field_3C;
        ilght.is_dynamic = 1;
        ilght.field_3 = eestat->field_3D;
        thing->light_id = light_create_light(&ilght);
        if (thing->light_id <= 0) {
            SYNCDBG(8,"Cannot allocate dynamic light to %s.",thing_model_name(thing));
        }
    }
    add_thing_to_its_class_list(thing);
    place_thing_in_mapwho(thing);
    return thing;
}

void process_spells_affected_by_effect_elements(struct Thing *thing)
{
    //_DK_process_spells_affected_by_effect_elements(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    GameTurnDelta dturn;
    long angle;
    MapCoordDelta shift_x;
    MapCoordDelta shift_y;
    struct Coord3d pos;
    struct Thing *effeltng;

    if ((cctrl->spell_flags & CSAfF_Rebound) != 0)
    {
        int diamtr = 4 * thing->clipbox_size_xy / 2;
        dturn = game.play_gameturn - thing->creation_turn;
        MapCoord cor_z_max = thing->clipbox_size_yz + (thing->clipbox_size_yz * gameadd.crtr_conf.exp.size_increase_on_exp * cctrl->explevel) / 80; //effect is 25% larger than unit

        struct EffectElementStats* eestat = get_effect_element_model_stats(TngEffElm_FlashBall1);
        unsigned short nframes = keepersprite_frames(eestat->sprite_idx);
        GameTurnDelta dtadd = 0;
        unsigned short cframe = game.play_gameturn % nframes;
        pos.z.val = thing->mappos.z.val;
        int radius = diamtr / 2;
        while (pos.z.val < cor_z_max + thing->mappos.z.val)
        {
            angle = (abs(dturn + dtadd) & 7) << 8;
            shift_x =  (radius * LbSinL(angle) >> 8) >> 8;
            shift_y = -(radius * LbCosL(angle) >> 8) >> 8;
            pos.x.val = thing->mappos.x.val + shift_x;
            pos.y.val = thing->mappos.y.val + shift_y;
            effeltng = create_thing(&pos, TCls_EffectElem, TngEffElm_FlashBall1, thing->owner, -1);
            if (thing_is_invalid(effeltng))
                break;
            set_thing_draw(effeltng, eestat->sprite_idx, 256, eestat->sprite_size_min, 0, cframe, 2);
            dtadd++;
            pos.z.val += 64;
            cframe = (cframe + 1) % nframes;
        }
    }

    if ((cctrl->spell_flags & CSAfF_Slow) != 0)
    {
        int diamtr = 4 * thing->clipbox_size_xy / 2;
        MapCoord cor_z_max = thing->clipbox_size_yz + (thing->clipbox_size_yz * gameadd.crtr_conf.exp.size_increase_on_exp * cctrl->explevel) / 80; //effect is 25% larger than unit
        int i = cor_z_max / 64;
        if (i <= 1)
          i = 1;
        dturn = game.play_gameturn - thing->creation_turn;
        int vrange = 2 * i / 2;
        if (dturn % (2 * i) < vrange)
            pos.z.val = thing->mappos.z.val + cor_z_max / vrange * dturn % vrange;
        else
            pos.z.val = thing->mappos.z.val + cor_z_max / vrange * (vrange - dturn % vrange);
        int radius = diamtr / 2;
        for (i=0; i < 16; i++)
        {
            angle = (abs(i) & 0xF) << 7;
            shift_x =  (radius * LbSinL(angle) >> 8) >> 8;
            shift_y = -(radius * LbCosL(angle) >> 8) >> 8;
            pos.x.val = thing->mappos.x.val + shift_x;
            pos.y.val = thing->mappos.y.val + shift_y;
            effeltng = create_thing(&pos, TCls_EffectElem, TngEffElm_RedFlash, thing->owner, -1);
        }
    }

    if ((cctrl->spell_flags & CSAfF_Flying) != 0)
    {
        effeltng = create_thing(&thing->mappos, TCls_EffectElem, TngEffElm_CloudDisperse, thing->owner, -1);
    }

    if ((cctrl->spell_flags & CSAfF_Speed) != 0)
    {
        effeltng = create_effect_element(&thing->mappos, TngEffElm_FlashBall2, thing->owner);
        if (!thing_is_invalid(effeltng))
        {
            // TODO: looks like some "struct AnimSpeed"
            memcpy(&effeltng->anim_speed, &thing->anim_speed, 20);
            effeltng->field_4F &= ~TF4F_Transpar_8;
            effeltng->field_4F |= TF4F_Transpar_4;
            effeltng->anim_speed = 0;
            effeltng->move_angle_xy = thing->move_angle_xy;
        }
    }

    if ((cctrl->stateblock_flags & CCSpl_Teleport) != 0)
    {
        dturn = get_spell_duration_left_on_thing(thing, SplK_Teleport);
        struct SpellConfig* splconf = &game.spells_config[SplK_Teleport];
        if (splconf->duration / 2 < dturn)
        {
            effeltng = create_effect_element(&thing->mappos, TngEffElm_FlashBall2, thing->owner);
            if (!thing_is_invalid(effeltng))
            {
                memcpy(&effeltng->anim_speed, &thing->anim_speed, 0x14u);
                effeltng->field_4F &= ~TF4F_Transpar_8;
                effeltng->field_4F |= TF4F_Transpar_4;
                effeltng->anim_speed = 0;
                effeltng->move_angle_xy = thing->move_angle_xy;
            }
        } else
        if (splconf->duration / 2 > dturn)
        {
            struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
            if ((dturn % 2) == 0) {
                effeltng = create_effect_element(&thing->mappos, birth_effect_element[thing->owner], thing->owner);
            }
            creature_turn_to_face_angle(thing, thing->move_angle_xy + crstat->max_angle_change);
        }
    }

    if ((cctrl->spell_flags & CSAfF_MagicFall) != 0)
    {
        dturn = game.play_gameturn - thing->creation_turn;
        if ((dturn & 1) == 0) {
            effeltng = create_effect_element(&thing->mappos, birth_effect_element[thing->owner], thing->owner);
        }
        struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
        creature_turn_to_face_angle(thing, thing->move_angle_xy + crstat->max_angle_change);
        if ((dturn > 32) || thing_touching_floor(thing)) {
            cctrl->spell_flags &= ~CSAfF_MagicFall;
        }
    }
}

void move_effect_blocked(struct Thing *thing, struct Coord3d *prev_pos, struct Coord3d *next_pos)
{
    struct EffectElementStats* eestat = get_effect_element_model_stats(thing->model);
    long blocked_flags = get_thing_blocked_flags_at(thing, next_pos);
    slide_thing_against_wall_at(thing, next_pos, blocked_flags);
    if ( ((blocked_flags & SlbBloF_WalledZ) != 0) && eestat->field_15 && eestat->field_22 )
    {
        struct Thing* efftng = thing;
        long cube_id = get_top_cube_at(next_pos->x.stl.num, next_pos->y.stl.num, NULL);
        unsigned short effmodel;
        if (cube_is_water(cube_id))
        {
          effmodel = eestat->water_effmodel;
          if (effmodel > 0) {
              efftng = create_effect(prev_pos, effmodel, thing->owner);
              TRACE_THING(efftng);
          }
          long sample_id = eestat->water_snd_smpid;
          if (sample_id > 0) {
              thing_play_sample(efftng, sample_id, NORMAL_PITCH, 0, 3, 0, 2, eestat->water_loudness);
          }
          if ( eestat->water_destroy_on_impact )
              thing->health = 0;
        } else
        if (cube_is_lava(cube_id))
        {
            effmodel = eestat->lava_effmodel;
            if (effmodel > 0) {
                efftng = create_effect(prev_pos, effmodel, thing->owner);
                TRACE_THING(efftng);
            }
            long sample_id = eestat->lava_snd_smpid;
            if (sample_id > 0) {
                thing_play_sample(efftng, sample_id, NORMAL_PITCH, 0, 3, 0, 2, eestat->lava_loudness);
            }
            if ( eestat->lava_destroy_on_impact )
                thing->health = 0;
        } else
        {
            effmodel = eestat->effmodel_23;
            if (effmodel > 0) {
                efftng = create_effect(prev_pos, effmodel, thing->owner);
                TRACE_THING(efftng);
            }
            long sample_id = eestat->solidgnd_snd_smpid;
            if (sample_id > 0) {
                thing_play_sample(efftng, sample_id, NORMAL_PITCH, 0, 3, 0, 2, eestat->solidgnd_loudness);
            }
            if ( eestat->solidgnd_destroy_on_impact )
                thing->health = 0;
        }
    }
    remove_relevant_forces_from_thing_after_slide(thing, next_pos, blocked_flags);
}

TngUpdateRet move_effect_element(struct Thing *thing)
{
    SYNCDBG(18,"Starting");
    TRACE_THING(thing);
    struct Coord3d pos;
    TbBool move_allowed = get_thing_next_position(&pos, thing);
    if ( positions_equivalent(&thing->mappos, &pos) ) {
        return TUFRet_Unchanged;
    }
    if ((thing->movement_flags & TMvF_Unknown10) == 0)
    {
        if (!move_allowed)
        {
            move_effect_blocked(thing, &thing->mappos, &pos);
        } else
        if ( !thing_covers_same_blocks_in_two_positions(thing, &thing->mappos, &pos) )
        {
            if ( thing_in_wall_at(thing, &pos) )
            {
                move_effect_blocked(thing, &thing->mappos, &pos);
            }
        }
    }
    move_thing_in_map(thing, &pos);
    return TUFRet_Modified;
}

void change_effect_element_into_another(struct Thing *thing, long nmodel)
{
    SYNCDBG(18,"Starting");
    //return _DK_change_effect_element_into_another(thing,nmodel);
    struct EffectElementStats* eestat = get_effect_element_model_stats(nmodel);
    int speed = eestat->sprite_speed_min + EFFECT_RANDOM(thing, eestat->sprite_speed_max - eestat->sprite_speed_min + 1);
    int scale = eestat->sprite_size_min + EFFECT_RANDOM(thing, eestat->sprite_size_max - eestat->sprite_size_min + 1);
    thing->model = nmodel;
    set_thing_draw(thing, eestat->sprite_idx, speed, scale, eestat->field_D, 0, 2);
    thing->field_4F ^= (thing->field_4F ^ 0x02 * eestat->field_13) & TF4F_Unknown02;
    thing->field_4F ^= (thing->field_4F ^ 0x10 * eestat->field_14) & (TF4F_Transpar_Flags);
    thing->fall_acceleration = eestat->field_18;
    thing->field_23 = eestat->field_1A;
    thing->field_24 = eestat->field_1C;
    if (eestat->numfield_3 <= 0) {
        thing->health = get_lifespan_of_animation(thing->anim_sprite, thing->anim_speed);
    } else {
        thing->health = EFFECT_RANDOM(thing, eestat->numfield_5 - eestat->numfield_3 + 1) + eestat->numfield_3;
    }
    thing->field_49 = keepersprite_frames(thing->anim_sprite);
}

TngUpdateRet update_effect_element(struct Thing *elemtng)
{
    long i;
    SYNCDBG(18,"Starting");
    TRACE_THING(elemtng);
    struct EffectElementStats* eestats = get_effect_element_model_stats(elemtng->model);
    // Check if effect health dropped to zero; delete it, or decrease health for the next check
    long health = elemtng->health;
    if (health <= 0)
    {
        if (eestats->transform_model != 0)
        {
            change_effect_element_into_another(elemtng, eestats->transform_model);
        } else
        {
            delete_thing_structure(elemtng, 0);
        }
        return TUFRet_Deleted;
    }
    elemtng->health = health-1;
    // Set dynamic properties of the effect
    if (!eestats->field_12)
    {
        if (elemtng->field_60 >= (int)elemtng->mappos.z.val)
          elemtng->anim_speed = 0;
    }
    if (eestats->field_15)
    {
        elemtng->movement_flags &= ~TMvF_IsOnWater;
        elemtng->movement_flags &= ~TMvF_IsOnLava;
        if (thing_touching_floor(elemtng))
        {
            i = get_top_cube_at(elemtng->mappos.x.stl.num, elemtng->mappos.y.stl.num, NULL);
            if (cube_is_water(i)) {
                elemtng->movement_flags |= TMvF_IsOnWater;
            } else
            if (cube_is_lava(i)) {
                elemtng->movement_flags |= TMvF_IsOnLava;
            }
        }
    }
    i = eestats->subeffect_delay;
    if (i > 0)
    {
      if (((elemtng->creation_turn - game.play_gameturn) % i) == 0) {
          create_effect_element(&elemtng->mappos, eestats->subeffect_model, elemtng->owner);
      }
    }
    switch (eestats->field_1)
    {
    case 1:
        move_effect_element(elemtng);
        break;
    case 2:
        i = elemtng->veloc_base.x.val;
        elemtng->veloc_base.x.val = 2*i/3;
        i = elemtng->veloc_base.y.val;
        elemtng->veloc_base.y.val = 2*i/3;
        i = elemtng->veloc_base.z.val;
        if (i > 32)
        {
          elemtng->veloc_base.z.val = 2*i/3;
        } else
        if (i > 16)
        {
          i = i-16;
          if (i < 16) i = 16;
          elemtng->veloc_base.z.val = i;
        } else
        if (i < -16)
        {
          elemtng->veloc_base.z.val = 2*i/3;
        } else
        {
            i = i+16;
            if (i > 16) i = 16;
            elemtng->veloc_base.z.val = i;
        }
        move_effect_element(elemtng);
        break;
    case 3:
        elemtng->veloc_base.z.val = 32;
        move_effect_element(elemtng);
        break;
    case 4:
        health = elemtng->health;
        if ((health >= 0) && (health < 16))
        {
            elemtng->veloc_base.z.val = bounce_table[health];
        } else
        {
            ERRORLOG("Illegal effect element bounce life: %d", (int)health);
        }
        move_effect_element(elemtng);
        break;
    case 5:
        break;
    default:
        ERRORLOG("Invalid effect element move type %d!",(int)eestats->field_1);
        move_effect_element(elemtng);
        break;
    }

    if (eestats->field_2 != 1)
      return TUFRet_Modified;
    i = get_angle_yz_to_vec(&elemtng->veloc_base);
    if (i > LbFPMath_PI)
      i -= LbFPMath_PI;
    long prop_val = i / (LbFPMath_PI / 8);
    elemtng->move_angle_xy = get_angle_xy_to_vec(&elemtng->veloc_base);
    elemtng->field_48 = prop_val;
    elemtng->anim_speed = 0;
    elemtng->field_40 = (prop_val & 0xff) << 8;
    SYNCDBG(18,"Finished");
    return TUFRet_Modified;
}

struct Thing *create_effect_generator(struct Coord3d *pos, unsigned short model, unsigned short range, unsigned short owner, long parent_idx)
{
  struct Thing *effgentng;

  if ( i_can_allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots) )
  {
    effgentng = allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots);
    effgentng->class_id = TCls_EffectGen;
    effgentng->model = model;
    effgentng->parent_idx = parent_idx;
    if ( parent_idx == -1 )
      effgentng->parent_idx = -1;
    effgentng->owner = owner;
    effgentng->effect_generator.range = range;
    effgentng->mappos = *pos;
    effgentng->creation_turn = game.play_gameturn;
    effgentng->health = -1;
    effgentng->field_4F |= TF4F_Unknown01;
    add_thing_to_list(effgentng, get_list_for_thing_class(TCls_EffectGen));
    place_thing_in_mapwho(effgentng);
    return effgentng;
  }
  else
  {
    ERRORLOG("Cannot create effect generator because there are too many fucking things allocated.");
    return 0;
  }
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
    if (thing->index == efftng->index)
    {
        SYNCDBG(18,"Effect tried to shoot itself; suicide not implemented");
        return false;
    }
    if (thing->index == efftng->parent_idx)
    {
        SYNCDBG(18,"Effect tried to shoot its maker; suicide not implemented");
        return false;
    }
    HitTargetFlags hit_targets = hit_type_to_hit_targets(efftng->hit_type);
    return area_effect_can_affect_thing(thing, hit_targets, efftng->owner);
}

void update_effect_light_intensity(struct Thing *thing)
{
  if (thing->light_id != 0)
  {
      if (thing->health < 4)
      {
          long i = light_get_light_intensity(thing->light_id);
          light_set_light_intensity(thing->light_id, (3*i)/4);
      }
  }
}

void effect_generate_effect_elements(const struct Thing *thing)
{
    const struct InitEffect* effnfo = get_effect_info_for_thing(thing);
    SYNCDBG(18,"Preparing Effect, Generation Type %d",(int)effnfo->generation_type);
    unsigned long arg;
    struct Thing* elemtng;
    switch (effnfo->generation_type)
    {
    case 1:
    {
        unsigned long argZ;
        for (long i = 0; i < effnfo->field_B; i++)
        {
            if (effnfo->kind_min <= 0)
                continue;
            long n = effnfo->kind_min + EFFECT_RANDOM(thing, effnfo->kind_max - effnfo->kind_min + 1);
            elemtng = create_effect_element(&thing->mappos, n, thing->owner);
            TRACE_THING(elemtng);
            if (thing_is_invalid(elemtng))
                break;
            arg = EFFECT_RANDOM(thing, 0x800);
            argZ = EFFECT_RANDOM(thing, 0x400);
            // Setting XY acceleration
            long k = abs(effnfo->accel_xy_max - effnfo->accel_xy_min);
            if (k <= 1) k = 1;
            long mag = effnfo->accel_xy_min + EFFECT_RANDOM(thing, k);
            elemtng->veloc_push_add.x.val += distance_with_angle_to_coord_x(mag,arg);
            elemtng->veloc_push_add.y.val += distance_with_angle_to_coord_y(mag,arg);
            // Setting Z acceleration
            k = abs(effnfo->accel_z_max - effnfo->accel_z_min);
            if (k <= 1) k = 1;
            mag = effnfo->accel_z_min + EFFECT_RANDOM(thing, k);
            elemtng->veloc_push_add.z.val += distance_with_angle_to_coord_z(mag,argZ);
            elemtng->state_flags |= TF1_PushAdd;
        }
        break;
    }
    case 2:
    {
        long k = 0;
        struct Coord3d pos;
        for (long i=0; i < effnfo->field_B; i++)
        {
            long n = effnfo->kind_min + EFFECT_RANDOM(thing, effnfo->kind_max - effnfo->kind_min + 1);
            long mag = effnfo->start_health - thing->health;
            arg = (mag << 7) + k/effnfo->field_B;
            set_coords_to_cylindric_shift(&pos, &thing->mappos, mag, arg, 0);
            elemtng = create_effect_element(&pos, n, thing->owner);
            TRACE_THING(elemtng);
            SYNCDBG(18,"Created %s",thing_model_name(elemtng));
            k += 2048;
        }
        break;
    }
    case 3:
    {
        long k = 0;
        struct Coord3d pos;
        for (long i=0; i < effnfo->field_B; i++)
        {
            long n = effnfo->kind_min + EFFECT_RANDOM(thing, effnfo->kind_max - effnfo->kind_min + 1);
            long mag = thing->health;
            arg = (mag << 7) + k/effnfo->field_B;
            set_coords_to_cylindric_shift(&pos, &thing->mappos, 16*mag, arg, 0);
            elemtng = create_effect_element(&pos, n, thing->owner);
            TRACE_THING(elemtng);
            k += 2048;
        }
        break;
    }
    case 4: // Lightning or CaveIn
    {
        if (thing->model != 48) // CaveIn only
            break;
        long i = effnfo->start_health / 2;
        struct PlayerInfo* player;
        if (thing->health == effnfo->start_health)
        {
            LbMemorySet(temp_pal, 63, PALETTE_SIZE);
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
            LbPaletteFade(engine_palette, 8, Lb_PALETTE_FADE_OPEN);
        } else
        {
            player = get_my_player();
            PaletteSetPlayerPalette(player, engine_palette);
            LbPaletteStopOpenFade();
        }
        break;
    }
    default:
        ERRORLOG("Unknown Effect Generation Type %d",(int)effnfo->generation_type);
        break;
    }
}

TngUpdateRet process_effect_generator(struct Thing *thing)
{
    SYNCDBG(18,"Starting");
    TRACE_THING(thing);
    if (thing->health > 0)
        thing->health--;
    if (thing->health == 0)
    {
        delete_thing_structure(thing, 0);
        return TUFRet_Deleted;
    }
    if ( !any_player_close_enough_to_see(&thing->mappos) )
    {
        SYNCDBG(18,"No player sees %s at (%d,%d,%d)",thing_model_name(thing),(int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num,(int)thing->mappos.z.stl.num);
        return TUFRet_Modified;
    }
    if (thing->effect_generator.generation_delay > 0)
        thing->effect_generator.generation_delay--;
    if (thing->effect_generator.generation_delay > 0)
    {
        return TUFRet_Modified;
    }
    struct EffectGeneratorStats* egenstat = &effect_generator_stats[thing->model];
    for (long i = 0; i < egenstat->genation_amount; i++)
    {
        long deviation_angle = EFFECT_RANDOM(thing, 0x800);
        long deviation_mag = EFFECT_RANDOM(thing, thing->belongs_to + 1);
        struct Coord3d pos;
        set_coords_to_cylindric_shift(&pos, &thing->mappos, deviation_mag, deviation_angle, 0);
        SYNCDBG(18,"The %s creates effect %d/%d at (%d,%d,%d)",thing_model_name(thing),(int)pos.x.val,(int)pos.y.val,(int)pos.z.val);
        struct Thing* elemtng = create_effect_element(&pos, egenstat->effect_sound, thing->owner);
        TRACE_THING(elemtng);
        if (thing_is_invalid(elemtng))
            break;
        elemtng->clipbox_size_xy = 20;
        elemtng->clipbox_size_yz = 20;
        long k;
        if (egenstat->field_10)
        {
            k = egenstat->field_11;
        } else
        if (egenstat->field_11 == -1)
        {
            elemtng->mappos.z.val = subtile_coord(8,0);
            k = get_next_gap_creature_can_fit_in_below_point(elemtng, &elemtng->mappos);
        } else
        {
            k = egenstat->field_11 + get_thing_height_at(elemtng, &elemtng->mappos);
        }
        elemtng->mappos.z.val = k;
        if ( thing_in_wall_at(elemtng, &elemtng->mappos) )
        {
            SYNCDBG(18,"The %s created effect %d/%d in wall, removing",thing_model_name(thing),(int)i,(int)egenstat->genation_amount);
            delete_thing_structure(elemtng, 0);
        } else
        {
            SYNCDBG(18,"The %s created effect %d/%d, index %d",thing_model_name(thing),(int)i,(int)egenstat->genation_amount,(int)elemtng->index);
            long acc_x = egenstat->acc_x_min + EFFECT_RANDOM(thing, egenstat->acc_x_max - egenstat->acc_x_min + 1);
            long acc_y = egenstat->acc_y_min + EFFECT_RANDOM(thing, egenstat->acc_y_max - egenstat->acc_y_min + 1);
            long acc_z = egenstat->acc_z_min + EFFECT_RANDOM(thing, egenstat->acc_z_max - egenstat->acc_z_min + 1);
            elemtng->veloc_push_add.x.val += acc_x;
            elemtng->veloc_push_add.y.val += acc_y;
            elemtng->veloc_push_add.z.val += acc_z;
            elemtng->state_flags |= TF1_PushAdd;
            if (egenstat->sound_sample_idx > 0)
            {
                struct Thing* sectng = create_effect(&elemtng->mappos, TngEff_DamageBlood, thing->owner);
                TRACE_THING(sectng);
                if (!thing_is_invalid(sectng)) {
                    thing_play_sample(sectng, egenstat->sound_sample_idx + EFFECT_RANDOM(thing, egenstat->sound_sample_rng), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
                }
            }
            if (egenstat->sound_sample_sec > 0) {
                thing_play_sample(elemtng, egenstat->sound_sample_sec, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
            }
        }
    }
    thing->long_15 = egenstat->genation_delay_min + EFFECT_RANDOM(thing, egenstat->genation_delay_max - egenstat->genation_delay_min + 1);
    return TUFRet_Modified;
}

struct Thing *create_effect(const struct Coord3d *pos, ThingModel effmodel, PlayerNumber owner)
{
    struct InitEffect* ieffect = &effect_info[effmodel];
    if (!i_can_allocate_free_thing_structure(1)) {
        return INVALID_THING;
    }
    struct Thing* thing = allocate_free_thing_structure(1);
    if (thing->index == 0) {
        ERRORDBG(8,"Should be able to allocate effect %d for player %d, but failed.",(int)effmodel,(int)owner);
        return INVALID_THING;
    }
    thing->creation_turn = game.play_gameturn;
    thing->class_id = TCls_Effect;
    thing->model = effmodel;
    thing->mappos.x.val = pos->x.val;
    thing->mappos.y.val = pos->y.val;
    thing->mappos.z.val = pos->z.val;
    thing->next_on_mapblk = 0;
    thing->owner = owner;
    thing->parent_idx = thing->index;
    thing->fall_acceleration = 0;
    thing->field_23 = 0;
    thing->field_24 = 0;
    thing->field_4F |= TF4F_Unknown01;
    thing->health = ieffect->start_health;
    if (ieffect->ilght.radius != 0)
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
    if (ieffect->effect_sound != 0)
    {
        thing_play_sample(thing, ieffect->effect_sound, NORMAL_PITCH, 0, 3, 0, 3, FULL_LOUDNESS);
    }
    return thing;
}

struct Thing *create_special_used_effect(const struct Coord3d *pos, long plyr_idx)
{
    struct Thing* efftng = create_effect(pos, TngEff_SpecialBox, plyr_idx);
    TRACE_THING(efftng);
    return efftng;
}

TbBool destroy_effect_thing(struct Thing *efftng)
{
    if (efftng->model == TngEff_Eruption)
    {
        place_slab_type_on_map(SlbT_LAVA, efftng->mappos.x.stl.num, efftng->mappos.y.stl.num, efftng->owner, 0);
        do_slab_efficiency_alteration(subtile_slab_fast(efftng->mappos.x.stl.num), subtile_slab_fast(efftng->mappos.y.stl.num));
    }
    if (efftng->snd_emitter_id != 0)
    {
        // In case of effect, don't stop any sound samples which are still playing
        S3DDestroySoundEmitter(efftng->snd_emitter_id);
        efftng->snd_emitter_id = 0;
    }
    delete_thing_structure(efftng, 0);
    return true;
}

/**
 * Affects a thing with explosion effect.
 *
 * @param tngsrc The thing which caused the affect.
 * @param tngdst The thing being affected by the effect.
 * @param pos Position of the effect epicenter.
 * @param max_dist Max distance at which creatures are affected, in map coordinates.
 * @param max_damage Damage at epicenter of the explosion.
 * @param blow_strength The strength of hitwave blowing creatures out of affected area.
 * @param damage_type Type of the damage inflicted.
 * @param owner The owner of the explosion.
 * @return Gives true if the target thing was affected by the spell, false otherwise.
 * @note If the function returns true, the effect might have caused death of the target.
 */
TbBool explosion_affecting_thing(struct Thing *tngsrc, struct Thing *tngdst, const struct Coord3d *pos,
    MapCoordDelta max_dist, HitPoints max_damage, long blow_strength, DamageType damage_type, PlayerNumber owner)
{
    TbBool affected = false;
    SYNCDBG(17,"Starting for %s, max damage %d, max blow %d, owner %d",thing_model_name(tngdst),(int)max_damage,(int)blow_strength,(int)owner);
    if (nowibble_line_of_sight_3d(pos, &tngdst->mappos))
    {
        // Friendly fire usually causes less damage and at smaller distance
        if ((tngdst->class_id == TCls_Creature) && (tngdst->owner == owner)) {
            max_dist = max_dist * gameadd.friendly_fight_area_range_permil / 1000;
            max_damage = max_damage * gameadd.friendly_fight_area_damage_permil / 1000;
        }
        MapCoordDelta distance = get_2d_distance(pos, &tngdst->mappos);
        if (distance < max_dist)
        {
            if (tngdst->class_id == TCls_Creature)
            {
                HitPoints damage = get_radially_decaying_value(max_damage, max_dist / 4, 3 * max_dist / 4, distance) + 1;
                SYNCDBG(7,"Causing %d damage to %s at distance %d",(int)damage,thing_model_name(tngdst),(int)distance);
                apply_damage_to_thing_and_display_health(tngdst, damage, damage_type, owner);
                affected = true;
                if (tngdst->health < 0)
                {
                    CrDeathFlags dieflags = CrDed_DiedInBattle;
                    // Explosions kill rather than only stun friendly creatures when imprison is on
                    if ((tngsrc->owner == tngdst->owner) &! (gameadd.classic_bugs_flags & ClscBug_FriendlyFaint))
                    {
                        dieflags |= CrDed_NoUnconscious;
                    }
                    kill_creature(tngdst, tngsrc, -1, dieflags);
                    affected = true;
                }
            }
            if (thing_is_dungeon_heart(tngdst))
            {
                HitPoints damage = get_radially_decaying_value(max_damage, max_dist / 4, 3 * max_dist / 4, distance) + 1;
                SYNCDBG(7,"Causing %d damage to %s at distance %d",(int)damage,thing_model_name(tngdst),(int)distance);
                apply_damage_to_thing(tngdst, damage, damage_type, -1);
                affected = true;
                event_create_event_or_update_nearby_existing_event(tngdst->mappos.x.val, tngdst->mappos.y.val,EvKind_HeartAttacked, tngdst->owner, 0);
                if (is_my_player_number(tngdst->owner))
                {
                    output_message(SMsg_HeartUnderAttack, 400, true);
                }
            } else // Explosions move creatures and other things
            {
                long move_angle = get_angle_xy_to(pos, &tngdst->mappos);
                long move_dist = get_radially_decaying_value(blow_strength, max_dist / 4, 3 * max_dist / 4, distance);
                if (move_dist > 0)
                {
                    tngdst->veloc_push_add.x.val += distance_with_angle_to_coord_x(move_dist, move_angle);
                    tngdst->veloc_push_add.y.val += distance_with_angle_to_coord_y(move_dist, move_angle);
                    tngdst->state_flags |= TF1_PushAdd;
                    affected = true;
                }
            } 
        }
    }
    return affected;
}
TbBool explosion_affecting_door(struct Thing *tngsrc, struct Thing *tngdst, const struct Coord3d *pos,
    MapCoordDelta max_dist, HitPoints max_damage, long blow_strength, DamageType damage_type, PlayerNumber owner)
{
    TbBool affected = false;
    SYNCDBG(17,"Starting for %s, max damage %d, max blow %d, owner %d",thing_model_name(tngdst),(int)max_damage,(int)blow_strength,(int)owner);
    if (tngdst->class_id != TCls_Door)
    {
        ERRORLOG("%s is trying to damage %s which is not a door",thing_model_name(tngsrc),thing_model_name(tngdst));
        return false;
    }
    if (line_of_sight_3d_ignoring_specific_door(&tngsrc->mappos, &tngdst->mappos,tngdst))
    {
        MapCoordDelta distance = get_2d_distance(pos, &tngdst->mappos);
        if (distance < max_dist)
        {
            HitPoints damage = get_radially_decaying_value(max_damage, max_dist / 4, 3 * max_dist / 4, distance) + 1;
            SYNCDBG(7,"Causing %d damage to %s at distance %d",(int)damage,thing_model_name(tngdst),(int)distance);
            apply_damage_to_thing(tngdst, damage, damage_type, -1);
            affected = true;
        }
    }
    return affected;
}

/**
 * Computes and applies damage the effect associated to a spell makes to things at given map block.
 * @param efftng The effect thing which represents the spell.
 * @param tngsrc The thing being source of the spell.
 * @param mapblk Map block on which all targets are to be affected by the spell.
 * @param max_dist Range of the spell on map, used to compute damage decaying with distance; in map coordinates.
 * @param max_damage Damage at epicenter of the explosion.
 * @param blow_strength The strength of hitwave blowing creatures out of affected area.
 * @param damage_type Type of the damage inflicted.
 */
long explosion_effect_affecting_map_block(struct Thing *efftng, struct Thing *tngsrc, struct Map *mapblk,
    MapCoordDelta max_dist, HitPoints max_damage, long blow_strength, DamageType damage_type)
{
    PlayerNumber owner;
    if (!thing_is_invalid(tngsrc))
        owner = tngsrc->owner;
    else
        owner = -1;
    long num_affected = 0;
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing processing block
        if ((thing->class_id == TCls_Door) && (efftng->shot.hit_type != 4)) //TODO: Find pretty way to say that WoP traps should not destroy doors. And make it configurable through configs.
        {
            if (explosion_affecting_door(tngsrc, thing, &efftng->mappos, max_dist, max_damage, blow_strength, damage_type, owner))
            {
                num_affected++;
            }
        } else
        if (effect_can_affect_thing(efftng, thing))
        {
            if (explosion_affecting_thing(tngsrc, thing, &efftng->mappos, max_dist, max_damage, blow_strength, damage_type, owner))
            {
                num_affected++;
            }
        }
        // Per thing processing block ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
    return num_affected;
}

/**
 * Applies damage the Word Of Power spell makes to all things in the area surrounding given position.
 * @param efftng The thing which is WOP effect originator.
 * @param owntng The thing being affected by the spell.
 * @param pos Position where the WOP effect center is.
 * @param max_dist Range of the WOP spell effect, in map coordinates.
 */
void word_of_power_affecting_area(struct Thing *efftng, struct Thing *owntng, struct Coord3d *pos)
{
    long stl_xmin;
    long stl_xmax;
    long stl_ymin;
    long stl_ymax;
    // Effect causes area damage only on its birth turn
    if (efftng->creation_turn != game.play_gameturn) {
        return;
    }
    struct ShotConfigStats* shotst;
    if (efftng->shot.hit_type == 4) // TODO: hit type seems hard coded. Find a better way to tell apart WoP traps from spells.
    {
        shotst = get_shot_model_stats(31); //SHOT_TRAP_WORD_OF_POWER
    }
    else
    {
        shotst = get_shot_model_stats(30); //SHOT_WORD_OF_POWER
    }
    if ((shotst->area_range <= 0) || ((shotst->area_damage == 0) && (shotst->area_blow == 0))) {
        ERRORLOG("Word of power shot configuration does not include area influence.");
        return;
    }
    SYNCDBG(8,"Starting for %s index %d owner %d",thing_model_name(efftng),(int)efftng->index,(int)efftng->owner);
    MapCoordDelta max_dist = shotst->area_range * COORD_PER_STL;
    {
        // Make sure the subtile is rounded up, unless the range is really close to lower value
        long stl_range = coord_subtile(max_dist + COORD_PER_STL * 9 / 10);
        // Position on subtile is not at its start, so add 1 to max values while ignoring the position
        stl_xmin = pos->x.stl.num - stl_range;
        stl_xmax = pos->x.stl.num + stl_range + 1;
        stl_ymin = pos->y.stl.num - stl_range;
        stl_ymax = pos->y.stl.num + stl_range + 1;
    }
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
    for (long stl_y = stl_ymin; stl_y <= stl_ymax; stl_y++)
    {
        for (long stl_x = stl_xmin; stl_x <= stl_xmax; stl_x++)
        {
            struct Map* mapblk = get_map_block_at(stl_x, stl_y);
            explosion_effect_affecting_map_block(efftng, owntng, mapblk, max_dist,
                shotst->area_damage, shotst->area_blow, shotst->damage_type);
        }
    }
}

/**
 * Determines if an explosion or other area effect, of given hit thing type and owner, can affect given thing.
 * Explosions can affect a lot more things than shots. If only the thing isn't invalid,
 * it is by default affected by explosions.
 */
TbBool area_effect_can_affect_thing(const struct Thing *thing, HitTargetFlags hit_targets, PlayerNumber shot_owner)
{
    if (thing_is_invalid(thing))
    {
        WARNLOG("Invalid thing tries to interact with explosion");
        return false;
    }
    return thing_is_shootable(thing, shot_owner, hit_targets);
}

/**
 * Affects things on a map blocks with explosion effect, if only they should be affected with given hit type.
 *
 * @param tngsrc The thing which caused the affect.
 * @param mapblk Map blocks on which are creatures should be affected.
 * @param pos Position of the effect epicenter.
 * @param max_dist Max distance at which creatures are affected, in map coordinates.
 * @param max_damage Damage at epicenter of the explosion.
 * @param blow_strength The strength of hitwave blowing creatures out of affected area.
 * @param hit_targets Defines which things are affected.
 * @param damage_type Type of the damage inflicted.
 */
long explosion_affecting_map_block(struct Thing *tngsrc, const struct Map *mapblk, const struct Coord3d *pos,
    MapCoord max_dist, HitPoints max_damage, long blow_strength, HitTargetFlags hit_targets, DamageType damage_type)
{
    PlayerNumber owner;
    if (!thing_is_invalid(tngsrc))
        owner = tngsrc->owner;
    else
        owner = -1;
    long num_affected = 0;
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            WARNLOG("Jump out of things array");
            break;
        }
        i = thing->next_on_mapblk;
        // Should never happen - only existing thing shall be in list
        if (!thing_exists(thing))
        {
            WARNLOG("Jump to non-existing thing");
            break;
        }
        // Per thing processing block
        if (area_effect_can_affect_thing(thing, hit_targets, owner))
        {
            if (explosion_affecting_thing(tngsrc, thing, pos, max_dist, max_damage, blow_strength, damage_type, owner))
                num_affected++;
        }
        // Per thing processing block ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
    return num_affected;
}

/**
 * Affects things on an area with explosion effect, if only they should be affected with given hit type.
 *
 * @param tngsrc The thing which caused the effect, usually spell caster.
 * @param pos Position of the effect epicenter.
 * @param range Range of the effect, in subtiles.
 * @param max_damage Damage at epicenter of the effect.
 * @param blow_strength The strength of hitwave blowing creatures out of affected area.
 * @param hit_targets Defines which things are affected.
 * @return Gives amount of things which were affected by the explosion.
 */
long explosion_affecting_area(struct Thing *tngsrc, const struct Coord3d *pos, MapCoord max_dist,
    HitPoints max_damage, long blow_strength, HitTargetFlags hit_targets, DamageType damage_type)
{
    MapSubtlCoord start_x;
    MapSubtlCoord start_y;
    if (hit_targets == HitTF_None)
    {
        ERRORLOG("The %s tries to affect area up to distance %d with invalid hit type %d",thing_model_name(tngsrc),(int)max_dist,(int)hit_targets);
        return 0;
    }
    MapSubtlCoord range_stl = (max_dist + 5 * COORD_PER_STL / 6) / COORD_PER_STL;
    if (pos->x.stl.num > range_stl)
      start_x = pos->x.stl.num - range_stl;
    else
      start_x = 0;
    if (pos->y.stl.num > range_stl)
      start_y = pos->y.stl.num - range_stl;
    else
      start_y = 0;
    MapSubtlCoord end_x = range_stl + pos->x.stl.num;
    if (end_x >= map_subtiles_x)
      end_x = map_subtiles_x;
    MapSubtlCoord end_y = range_stl + pos->y.stl.num;
    if (end_y > map_subtiles_y)
      end_y = map_subtiles_y;
#if (BFDEBUG_LEVEL > 0)
    if ((start_params.debug_flags & DFlg_ShotsDamage) != 0)
        create_price_effect(pos, my_player_number, max_damage);
#endif
    long num_affected = 0;
    for (MapSubtlCoord stl_y = start_y; stl_y <= end_y; stl_y++)
    {
        for (MapSubtlCoord stl_x = start_x; stl_x <= end_x; stl_x++)
        {
            const struct Map* mapblk = get_map_block_at(stl_x, stl_y);
            num_affected += explosion_affecting_map_block(tngsrc, mapblk, pos, max_dist, max_damage, blow_strength, hit_targets, damage_type);
        }
    }
    return num_affected;
}

TbBool poison_cloud_affecting_thing(struct Thing *tngsrc, struct Thing *tngdst, const struct Coord3d *pos,
    MapCoordDelta max_dist, HitPoints max_damage, long blow_strength, unsigned char area_affect_type, DamageType damage_type, PlayerNumber owner)
{
    TbBool affected = false;
    SYNCDBG(17,"Starting for %s, max damage %d, max blow %d, owner %d",thing_model_name(tngdst),(int)max_damage,(int)blow_strength,(int)owner);
    if (thing_is_creature(tngdst))
    {
        const struct CreatureStats* crstat = creature_stats_get_from_thing(tngdst);
        if (crstat->immune_to_gas) {
            return affected;
        }
    } else {
        return affected;
    }
    if (line_of_sight_3d(pos, &tngdst->mappos))
    {
        // Note that gas has no distinction over friendly and enemy fire
        MapCoordDelta distance = get_2d_distance(pos, &tngdst->mappos);
        if (distance < max_dist)
        {
            struct CreatureControl* cctrl = creature_control_get_from_thing(tngdst);
            cctrl->spell_flags |= CSAfF_PoisonCloud;
            switch (area_affect_type)
            {
            case AAffT_GasDamage:
                if (max_damage > 0) {
                    HitPoints damage;
                    damage = get_radially_decaying_value(max_damage,3*max_dist/4,max_dist/4,distance)+1;
                    SYNCDBG(7,"Causing %d damage to %s at distance %d",(int)damage,thing_model_name(tngdst),(int)distance);
                    apply_damage_to_thing_and_display_health(tngdst, damage, damage_type, tngsrc->owner);
                }
                break;
            case AAffT_GasSlow:
                if (!creature_affected_by_spell(tngdst,SplK_Slow)) {
                    struct CreatureControl *srcctrl;
                    srcctrl = creature_control_get_from_thing(tngsrc);
                    apply_spell_effect_to_thing(tngdst, SplK_Slow, srcctrl->explevel);
                }
                break;
            case AAffT_GasSlowDamage:
                if (max_damage > 0) {
                    HitPoints damage;
                    damage = get_radially_decaying_value(max_damage, 3 * max_dist / 4, max_dist / 4, distance) + 1;
                    SYNCDBG(7, "Causing %d damage to %s at distance %d", (int)damage, thing_model_name(tngdst), (int)distance);
                    apply_damage_to_thing_and_display_health(tngdst, damage, damage_type, tngsrc->owner);
                }
                if (!creature_affected_by_spell(tngdst, SplK_Slow)) {
                    struct CreatureControl* srcctrl;
                    srcctrl = creature_control_get_from_thing(tngsrc);
                    apply_spell_effect_to_thing(tngdst, SplK_Slow, srcctrl->explevel);
                }
                break;
            case AAffT_GasDisease:
                if (!creature_affected_by_spell(tngdst, SplK_Disease)) {
                    struct CreatureControl* srcctrl;
                    srcctrl = creature_control_get_from_thing(tngsrc);
                    apply_spell_effect_to_thing(tngdst, SplK_Disease, srcctrl->explevel);
                }
            }
            affected = true;
        }
    }
    return affected;
}

long poison_cloud_affecting_map_block(struct Thing *tngsrc, const struct Map *mapblk, const struct Coord3d *pos,
    MapCoord max_dist, HitPoints max_damage, long blow_strength, HitTargetFlags hit_targets, unsigned char area_affect_type, DamageType damage_type)
{
    PlayerNumber owner;
    if (!thing_is_invalid(tngsrc))
        owner = tngsrc->owner;
    else
        owner = -1;
    long num_affected = 0;
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            WARNLOG("Jump out of things array");
            break;
        }
        i = thing->next_on_mapblk;
        // Should never happen - only existing thing shall be in list
        if (!thing_exists(thing))
        {
            WARNLOG("Jump to non-existing thing");
            break;
        }
        // Per thing processing block
        if (area_effect_can_affect_thing(thing, hit_targets, owner))
        {
            if (poison_cloud_affecting_thing(tngsrc, thing, pos, max_dist, max_damage, blow_strength, area_affect_type, damage_type, owner))
                num_affected++;
        }
        // Per thing processing block ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
    return num_affected;
}

long poison_cloud_affecting_area(struct Thing *tngsrc, struct Coord3d *pos, long max_dist, long max_damage, unsigned char area_affect_type)
{
    int dmg_divider = 10;
    if (thing_is_effect(tngsrc)) {
        const struct InitEffect* effnfo = get_effect_info_for_thing(tngsrc);
        dmg_divider = max(effnfo->start_health,1);
    }
    MapSubtlCoord start_x = coord_subtile(pos->x.val - max_dist);
    MapSubtlCoord start_y = coord_subtile(pos->y.val - max_dist);
    MapSubtlCoord end_x = coord_subtile(pos->x.val + max_dist) + 1;
    MapSubtlCoord end_y = coord_subtile(pos->y.val + max_dist) + 1;
    if (start_x < 0) {
        start_x = 0;
    } else
    if (start_x > map_subtiles_x) {
        start_x = map_subtiles_x;
    }
    if (start_y < 0) {
        start_y = 0;
    } else
    if (start_y > map_subtiles_y) {
        start_y = map_subtiles_y;
    }
    if (end_x < 0) {
        end_x = 0;
    } else
    if (end_x > map_subtiles_x) {
        end_x = map_subtiles_x;
    }
    if (end_y < 0) {
        end_y = 0;
    } else
    if (end_y > map_subtiles_y) {
        end_y = map_subtiles_y;
    }
    long num_affected = 0;
    for (MapSubtlCoord stl_y = start_y; stl_y <= end_y; stl_y++)
    {
        for (MapSubtlCoord stl_x = start_x; stl_x <= end_x; stl_x++)
        {
            HitTargetFlags hit_targets = hit_type_to_hit_targets(tngsrc->hit_type);
            struct Map* mapblk = get_map_block_at(stl_x, stl_y);
            num_affected += poison_cloud_affecting_map_block(tngsrc, mapblk, pos, max_dist, max_damage/dmg_divider, 0, hit_targets, area_affect_type, DmgT_Respiratory);
        }
    }
    return num_affected;
}

TngUpdateRet update_effect(struct Thing *efftng)
{
    SYNCDBG(18,"Starting for %s",thing_model_name(efftng));
    TRACE_THING(efftng);
    struct Thing* subtng = NULL;
    const struct InitEffect* effnfo = get_effect_info_for_thing(efftng);
    if (efftng->parent_idx > 0) {
        subtng = thing_get(efftng->parent_idx);
        TRACE_THING(subtng);
    }
    if (efftng->health <= 0) {
        destroy_effect_thing(efftng);
        return TUFRet_Deleted;
    }
    update_effect_light_intensity(efftng);
    // Effect generators can be used to generate effect elements
    if ( (effnfo->field_11 == 0) || any_player_close_enough_to_see(&efftng->mappos) )
    {
        effect_generate_effect_elements(efftng);
    }
    // Let the effect affect area
    switch (effnfo->area_affect_type)
    {
    case AAffT_GasDamage:
    case AAffT_GasSlow:
    case AAffT_GasSlowDamage:
    case AAffT_GasDisease:
        poison_cloud_affecting_area(subtng, &efftng->mappos, 5*COORD_PER_STL, 120, effnfo->area_affect_type);
        break;
    case AAffT_WOPDamage:
        word_of_power_affecting_area(efftng, subtng, &efftng->mappos);
        break;
    }
    efftng->health--;
    return move_effect(efftng);
}

struct Thing *create_price_effect(const struct Coord3d *pos, long plyr_idx, long price)
{
    struct Thing* elemtng = create_effect_element(pos, TngEffElm_Price, plyr_idx);
    TRACE_THING(elemtng);
    if (!thing_is_invalid(elemtng)) {
        elemtng->price.number = abs(price);
    }
    return elemtng;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
