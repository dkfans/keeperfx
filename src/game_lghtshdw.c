/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file game_lghtshdw.c
 *     Module which consolidates level data related to lights and shadows.
 * @par Purpose:
 *     Allows storing the lights and shadows data as one struct.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     21 Oct 2009 - 23 Oct 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "game_lghtshdw.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"

#include "map_data.h"

/******************************************************************************/
long get_subtile_lightness(const struct LightsShadows * lish, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    if (stl_x > map_subtiles_x) stl_x = map_subtiles_x;
    if (stl_y > map_subtiles_y) stl_y = map_subtiles_y;
    if (stl_x < 0)  stl_x = 0;
    if (stl_y < 0) stl_y = 0;
    return lish->subtile_lightness[get_subtile_number(stl_x,stl_y)];
}

void clear_subtiles_lightness(struct LightsShadows * lish)
{
    for (MapSubtlCoord y = 0; y < (map_subtiles_y + 1); y++)
    {
        for (MapSubtlCoord x = 0; x < (map_subtiles_x + 1); x++)
        {
            unsigned short* wptr = &lish->subtile_lightness[get_subtile_number(x, y)];
            *wptr = MINIMUM_LIGHTNESS;
        }
    }
}

void create_shadow_limits(struct LightsShadows * lish, long start, long end)
{
    if (start <= end)
    {
        LbMemorySet(&lish->shadow_limits[start], 1, end-start);
    } else
    {
        LbMemorySet(&lish->shadow_limits[start], 1, SHADOW_LIMITS_COUNT-1-start);
        LbMemorySet(&lish->shadow_limits[0], 1, end);
    }
}

void clear_shadow_limits(struct LightsShadows * lish)
{
    LbMemorySet(lish->shadow_limits, 0, SHADOW_LIMITS_COUNT);
}

void clear_light_system(struct LightsShadows * lish)
{
    LbMemorySet(lish, 0, sizeof(struct LightsShadows));
}

/******************************************************************************/
