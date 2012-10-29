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
#define SHADOW_CACHE_COUNT     40

/******************************************************************************/
#pragma pack(1)

struct UnkStruc6 { // sizeof = 8
  unsigned char field_0;
  unsigned char field_1;
  unsigned char field_2;
  unsigned char field_3;
  unsigned long field_4;
};

struct ShadowCache { // sizeof = 129
  unsigned char flags;
  unsigned char field_1[127];
  unsigned char field_80;
};

/**
 * Structure which stores data of lights and shadows system.
 */
struct LightsShadows { // sizeof = 164886
    struct UnkStruc6 field_1DD41[1024];
    unsigned char shadow_limits[SHADOW_LIMITS_COUNT];
    struct Light lights[LIGHTS_COUNT];
    struct ShadowCache shadow_cache[SHADOW_CACHE_COUNT];
    unsigned short stat_light_map[256*256];
    long field_46149;
    char field_4614D;
    char field_4614E;
    int field_4614F;
    int field_46153;
    unsigned short subtile_lightness[256*256];
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
