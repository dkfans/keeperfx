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

#define LIGHT_MAX_RANGE       256 // Large enough to cover the whole map
#define LIGHTS_COUNT         2048

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
    LgtF_NeedRemoval  = 0x02,
    LgtF_Dynamic      = 0x04,
    LgtF_NeedUpdate   = 0x08,
    LgtF_RadiusOscillation = 0x10,
    LgtF_IntensityAnimation = 0x20,
    LgtF_NeverCached  = 0x40,
    LgtF_OutOfDate    = 0x80,
};

enum LightFlags2 {
    LgtF2_InList    = 0x01,
};

struct Light {
  unsigned char flags;
  unsigned char flags2;
  unsigned char intensity;
  unsigned char intensity_toggling_field;//toggles between 1 and 2 when flags has LgtF_IntensityAnimation
  unsigned char intensity_delta;//seems never assigned
  unsigned char range;
  unsigned char radius_oscillation_direction;
  unsigned char max_intensity;//seems never assigned
  unsigned char min_radius;
  unsigned short index;
  unsigned short shadow_index;
  SlabCodedCoords attached_slb;
  unsigned short radius;
  unsigned short force_render_update;
  unsigned short radius_delta;//seems never assigned
  unsigned short max_radius;//seems never assigned
  unsigned short min_radius2;//seems never assigned
  unsigned short min_intensity;
  unsigned short next_in_list;
  struct Coord3d mappos;
  TbBool interp_has_been_initialized;
  struct Coord3d previous_mappos;
  struct Coord3d interp_mappos;
  GameTurn last_turn_drawn;
  GameTurnDelta disable_interp_for_turns;
};

struct InitLight { // sizeof=0x14
    short radius;
    unsigned char intensity;
    unsigned char flags;
    struct Coord3d mappos;
    unsigned char is_dynamic;
    SlabCodedCoords attached_slb;
};

struct LightSystemState {
    int32_t bitmask[32];
    int32_t static_light_needs_updating;
    int32_t total_dynamic_lights;
    int32_t total_stat_lights;
    int32_t rendered_dynamic_lights;
    int32_t rendered_optimised_dynamic_lights;
    int32_t updated_stat_lights;
    int32_t out_of_date_stat_lights;
};

/******************************************************************************/

#pragma pack()

typedef struct VALUE VALUE;

/******************************************************************************/
void clear_stat_light_map(void);
void update_light_render_area(void);
void light_delete_light(long idx);
void light_initialise(void);
void light_turn_light_off(long num);
void light_turn_light_on(long num);
unsigned char light_get_light_intensity(long idx);
void light_set_light_intensity(long idx, unsigned char intensity);
long light_create_light(struct InitLight *ilght);
TbBool light_create_light_adv(VALUE *init_data);
void light_set_light_never_cache(long lgt_id);
TbBool light_is_invalid(const struct Light *lgt);
long light_is_light_allocated(long lgt_id);
void light_set_light_position(long lgt_id, struct Coord3d *pos);
void light_stat_refresh();
void light_set_lights_on(char state);
void light_set_light_minimum_size_to_cache(long lgt_id, long a2, long a3);
void light_signal_update_in_area(long sx, long sy, long ex, long ey);
void light_export_system_state(struct LightSystemState *lightst);
void light_import_system_state(const struct LightSystemState *lightst);
TbBool lights_stats_debug_dump(void);
void light_signal_stat_light_update_in_area(long x1, long y1, long x2, long y2);

int light_count_lights();
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
