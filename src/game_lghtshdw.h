/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file game_merge.h
 *     Header file for game_lghtshdw.c.
 * @par Purpose:
 *     Consolidates level data related to lights and shadows.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     21 Oct 2009 - 23 Oct 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_LGHTSHDW_H
#define DK_LGHTSHDW_H

#include "bflib_basics.h"
#include "globals.h"

#include "light_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define SHADOW_LIMITS_COUNT  2048
#define SHADOW_CACHE_COUNT    512

/******************************************************************************/
#pragma pack(1)

struct LightingTable { // sizeof = 8
  TbBool is_populated;
  unsigned char distance; // 2 - 15
  char delta_x; // signed
  char delta_y; // signed
  uint32_t diagonal_length;
};

struct ShadowCache { // sizeof = 129
  unsigned char flags;
  unsigned int lighting_bitmask[32];
};

/**
 * Structure which stores data of lights and shadows system.
 */
struct LightsShadows {
    struct LightingTable lighting_tables[1024]; // only the first 700 elements are populated
    unsigned char shadow_limits[SHADOW_LIMITS_COUNT];
    struct Light lights[LIGHTS_COUNT];
    struct ShadowCache shadow_cache[SHADOW_CACHE_COUNT];
    unsigned short stat_light_map[MAX_SUBTILES_X*MAX_SUBTILES_Y];
    int32_t global_ambient_light;
    TbBool light_enabled;
    TbBool lighting_tables_initialised;
    int lighting_tables_count; // number of entries in lighting_tables
    unsigned short subtile_lightness[MAX_SUBTILES_X*MAX_SUBTILES_Y];
};

#pragma pack()
/******************************************************************************/
long get_subtile_lightness(const struct LightsShadows * lish, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
void clear_subtiles_lightness(struct LightsShadows * lish);

void create_shadow_limits(struct LightsShadows * lish, long start, long end);
void clear_shadow_limits(struct LightsShadows * lish);

void clear_light_system(struct LightsShadows * lish);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
