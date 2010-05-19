/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file light_data.h
 *     Header file for light_data.c.
 * @par Purpose:
 *     light_data functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_LIGHT_DATA_H
#define DK_LIGHT_DATA_H

#include "globals.h"

#define LIGHT_MAX_RANGE        30
#define LIGHTS_COUNT          400

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif
/******************************************************************************/
struct StructureList;

struct Light { // sizeof = 46
  unsigned char field_0;
  unsigned char field_1;
  unsigned char field_2[3];
  unsigned char field_5;
  unsigned char field_6;
  unsigned short field_7;
  unsigned char field_9[5];
  unsigned short field_E;
  unsigned char field_10[22];
  unsigned short field_26;
  unsigned char field_28;
  unsigned char field_29;
  unsigned char field_2A;
  unsigned char field_2B;
  unsigned short field_2C;
};

struct InitLight { // sizeof=0x14
short field_0;
unsigned char field_2;
unsigned char field_3;
unsigned char field_4[6];
    struct Coord3d mappos;
unsigned char field_10;
unsigned char field_11;
unsigned char field_12[2];
};
/******************************************************************************/
DLLIMPORT long _DK_light_bitmask[32];
#define light_bitmask _DK_light_bitmask
DLLIMPORT long _DK_stat_light_needs_updating;
#define stat_light_needs_updating _DK_stat_light_needs_updating
/******************************************************************************/
#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
void clear_light_system(void);
void clear_stat_light_map(void);
void update_light_render_area(void);
void light_delete_light(long idx);
void light_initialise_lighting_tables(void);
void light_initialise(void);
void light_turn_light_off(long num);
void light_turn_light_on(long num);
long light_get_light_intensity(long idx);
long light_set_light_intensity(long a1, long a2);
long light_create_light(struct InitLight *ilght);
void light_set_light_never_cache(long idx);
long light_is_light_allocated(long lgt_id);
void light_set_light_position(long lgt_id, struct Coord3d *pos);
void light_set_lights_on(char state);
void light_set_light_minimum_size_to_cache(long a1, long a2, long a3);
void light_signal_update_in_area(long sx, long sy, long ex, long ey);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
