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
#include "bflib_basics.h"

#define LIGHT_MAX_RANGE        30
#define LIGHTS_COUNT          400
#define MINIMUM_LIGHTNESS    8192

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct StructureList;

enum ShadowCacheFlags {
    ShCF_Allocated = 0x01,
};

enum LightFlags {
    LgtF_Allocated    = 0x01,
    LgtF_Unkn02       = 0x02,
    LgtF_Dynamic      = 0x04,
    LgtF_Unkn08       = 0x08,
};

struct Light { // sizeof = 46
  unsigned char flags;
  unsigned char field_1;
  unsigned char intensity;
  unsigned char field_3[2];
  unsigned char range;
  unsigned char field_6;
  unsigned short field_7;
  unsigned char field_9[5];
  unsigned short index;
  unsigned short shadow_index;
  long field_12;
  unsigned short radius;
  short field_18;
  short field_1A;
  unsigned char field_1C[10];
  unsigned short field_26;
  struct Coord3d mappos;
};

struct InitLight { // sizeof=0x14
short radius;
unsigned char intensity;
unsigned char field_3;
short field_4;
short field_6;
short field_8;
    struct Coord3d mappos;
unsigned char field_10;
    unsigned char is_dynamic;
short field_12;
};

struct LightSystemState {
    long bitmask[32];
    long static_light_needs_updating;
    long total_dynamic_lights;
    long total_stat_lights;
    long rendered_dynamic_lights;
    long rendered_optimised_dynamic_lights;
    long updated_stat_lights;
    long out_of_date_stat_lights;
};

/******************************************************************************/
DLLIMPORT long _DK_light_bitmask[32];
#define light_bitmask _DK_light_bitmask
DLLIMPORT long _DK_stat_light_needs_updating;
#define stat_light_needs_updating _DK_stat_light_needs_updating
DLLIMPORT long _DK_light_total_dynamic_lights;
#define light_total_dynamic_lights _DK_light_total_dynamic_lights
DLLIMPORT long _DK_light_total_stat_lights;
#define light_total_stat_lights _DK_light_total_stat_lights
DLLIMPORT long _DK_light_rendered_dynamic_lights;
#define light_rendered_dynamic_lights _DK_light_rendered_dynamic_lights
DLLIMPORT long _DK_light_rendered_optimised_dynamic_lights;
#define light_rendered_optimised_dynamic_lights _DK_light_rendered_optimised_dynamic_lights
DLLIMPORT long _DK_light_updated_stat_lights;
#define light_updated_stat_lights _DK_light_updated_stat_lights
DLLIMPORT long _DK_light_out_of_date_stat_lights;
#define light_out_of_date_stat_lights _DK_light_out_of_date_stat_lights

#pragma pack()
/******************************************************************************/
void clear_stat_light_map(void);
void update_light_render_area(void);
void light_delete_light(long idx);
void light_initialise_lighting_tables(void);
void light_initialise(void);
void light_turn_light_off(long num);
void light_turn_light_on(long num);
unsigned char light_get_light_intensity(long idx);
void light_set_light_intensity(long idx, unsigned char intensity);
long light_create_light(struct InitLight *ilght);
void light_set_light_never_cache(long lgt_id);
TbBool light_is_invalid(const struct Light *lgt);
long light_is_light_allocated(long lgt_id);
void light_set_light_position(long lgt_id, struct Coord3d *pos);
void light_set_lights_on(char state);
void light_set_light_minimum_size_to_cache(long a1, long a2, long a3);
void light_signal_update_in_area(long sx, long sy, long ex, long ey);
long light_get_total_dynamic_lights(void);
void light_export_system_state(struct LightSystemState *lightst);
void light_import_system_state(const struct LightSystemState *lightst);
TbBool lights_stats_debug_dump(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
