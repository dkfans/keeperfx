/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_complookup.c
 *     Computer player lookups definitions.
 * @par Purpose:
 *     Implements lookups used for computer player tasks.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Mar 2009 - 20 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "player_complookup.h"

#include <limits.h>
#include <string.h>

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"

#include "map_data.h"
#include "slab_data.h"
#include "game_legacy.h"
#include "front_simple.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct GoldLookup *get_gold_lookup(long idx)
{
    return &game.gold_lookup[idx];
}

long gold_lookup_index(const struct GoldLookup *gldlook)
{
    long i;
    i = ((char *)gldlook - (char *)&game.gold_lookup[0]);
    if ( (i < 0) || (i >= GOLD_LOOKUP_COUNT*sizeof(struct GoldLookup)) )
        return 0;
    return i / sizeof(struct GoldLookup);
}

/** Finds a gold vein with smaller amount of gold and gem slabs than given values.
 *  Gems slabs count has higher priority than gold slabs count.
 *
 * @param higher_gold_slabs
 * @param higher_gem_slabs
 * @return
 */
long smaller_gold_vein_lookup_idx(long higher_gold_slabs, long higher_gem_slabs)
{
    struct GoldLookup *gldlook;
    long gold_slabs, gem_slabs;
    long gold_idx;
    long i;
    gold_slabs = higher_gold_slabs;
    gem_slabs = higher_gem_slabs;
    gold_idx = -1;
    for (i=0; i < GOLD_LOOKUP_COUNT; i++)
    {
        gldlook = get_gold_lookup(i);
        if (gldlook->field_10 == gem_slabs)
        {
            if (gldlook->field_A < gold_slabs)
            {
              gold_slabs = gldlook->field_A;
              gold_idx = i;
            }
        } else
        if (gldlook->field_10 < gem_slabs)
        {
            gem_slabs = gldlook->field_10;
            gold_slabs = gldlook->field_A;
            gold_idx = i;
        }
    }
    return gold_idx;
}

void check_treasure_map(unsigned char *treasure_map, unsigned short *vein_list, long *gold_next_idx, MapSlabCoord veinslb_x, MapSlabCoord veinslb_y)
{
    struct GoldLookup *gldlook;
    struct SlabMap *slb;
    SlabCodedCoords slb_num,slb_around;
    MapSlabCoord slb_x,slb_y;
    long gold_slabs,gem_slabs;
    long vein_total,vein_idx;
    long gld_v1,gld_v2,gld_v3;
    long gold_idx;
    // First, find a vein
    vein_total = 0;
    slb_x = veinslb_x;
    slb_y = veinslb_y;
    gld_v1 = 0;
    gld_v2 = 0;
    gld_v3 = 0;
    gem_slabs = 0;
    gold_slabs = 0;
    slb_num = get_slab_number(slb_x, slb_y);
    treasure_map[slb_num] |= 0x02;
    for (vein_idx=0; vein_idx <= vein_total; vein_idx++)
    {
        gld_v1 += slb_x;
        gld_v2 += slb_y;
        gld_v3++;
        slb_around = get_slab_number(slb_x, slb_y);
        slb = get_slabmap_direct(slb_around);
        if (slb->kind == SlbT_GEMS)
        {
            gem_slabs++;
        } else
        {
            gold_slabs++;
            slb_around = get_slab_number(slb_x-1, slb_y);
            if ((treasure_map[slb_around] & 0x03) == 0)
            {
                treasure_map[slb_around] |= 0x02;
                vein_list[vein_total] = slb_around;
                vein_total++;
            }
            slb_around = get_slab_number(slb_x+1, slb_y);
            if ((treasure_map[slb_around] & 0x03) == 0)
            {
                treasure_map[slb_around] |= 0x02;
                vein_list[vein_total] = slb_around;
                vein_total++;
            }
            slb_around = get_slab_number(slb_x, slb_y-1);
            if ((treasure_map[slb_around] & 0x03) == 0)
            {
                treasure_map[slb_around] |= 0x02;
                vein_list[vein_total] = slb_around;
                vein_total++;
            }
            slb_around = get_slab_number(slb_x, slb_y+1);
            if ((treasure_map[slb_around] & 0x03) == 0)
            {
                treasure_map[slb_around] |= 0x02;
                vein_list[vein_total] = slb_around;
                vein_total++;
            }
        }
        // Move to next slab in list
        slb_x = slb_num_decode_x(vein_list[vein_idx]);
        slb_y = slb_num_decode_y(vein_list[vein_idx]);
    }
    // Now get a GoldLookup struct to put the vein into
    if (*gold_next_idx < GOLD_LOOKUP_COUNT)
    {
        gold_idx = *gold_next_idx;
        (*gold_next_idx)++;
    } else
    {
        gold_idx = smaller_gold_vein_lookup_idx(gold_slabs, gem_slabs);
    }
    // Write the vein to GoldLookup item
    if (gold_idx != -1)
    {
        gldlook = get_gold_lookup(gold_idx);
        LbMemorySet(gldlook, 0, sizeof(struct GoldLookup));
        gldlook->field_0 |= 0x01;
        gldlook->x_stl_num = slab_subtile_center(gld_v1 / gld_v3);
        gldlook->y_stl_num = slab_subtile_center(gld_v2 / gld_v3);
        gldlook->field_A = gold_slabs;
        gldlook->field_C = 0;
        gldlook->field_E = gold_slabs;
        gldlook->field_10 = gem_slabs;
    }
}

void check_map_for_gold(void)
{
    MapSlabCoord slb_x,slb_y;
    struct SlabMap *slb;
    SlabCodedCoords slb_num;
    unsigned char *treasure_map;
    unsigned short *vein_list;
    long gold_next_idx;
    long i;
    SYNCDBG(8,"Starting");
    //_DK_check_map_for_gold();
    for (i=0; i < GOLD_LOOKUP_COUNT; i++) {
        LbMemorySet(&game.gold_lookup[i], 0, sizeof(struct GoldLookup));
    }

    treasure_map = (unsigned char *)scratch;
    vein_list = (unsigned short *)&scratch[map_tiles_x*map_tiles_y];
    for (slb_y = 0; slb_y < map_tiles_y; slb_y++) {
        for (slb_x = 0; slb_x < map_tiles_x; slb_x++) {
            slb_num = get_slab_number(slb_x, slb_y);
            slb = get_slabmap_direct(slb_num);
            treasure_map[slb_num] = 0;
            if ( (slb->kind != SlbT_GOLD) && (slb->kind != SlbT_GEMS) ) {
                treasure_map[slb_num] |= 0x01;
            }
        }
    }
    gold_next_idx = 0;
    for (slb_y = 0; slb_y < map_tiles_y; slb_y++) {
        for (slb_x = 0; slb_x < map_tiles_x; slb_x++) {
            slb_num = get_slab_number(slb_x, slb_y);
            if ( ((treasure_map[slb_num] & 0x01) == 0) && ((treasure_map[slb_num] & 0x02) == 0) )
            {
                check_treasure_map(treasure_map, vein_list, &gold_next_idx, slb_x, slb_y);
            }
        }
    }
    SYNCDBG(8,"Found %ld possible digging locations",gold_next_idx);
}
/******************************************************************************/
