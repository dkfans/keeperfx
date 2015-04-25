/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file slab_data.c
 *     Map Slabs support functions.
 * @par Purpose:
 *     Definitions and functions to maintain map slabs.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 Apr 2009 - 12 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "slab_data.h"
#include "globals.h"

#include "bflib_memory.h"
#include "player_instances.h"
#include "config_terrain.h"
#include "map_blocks.h"
#include "ariadne.h"
#include "ariadne_wallhug.h"
#include "map_utils.h"
#include "frontmenu_ingame_map.h"
#include "game_legacy.h"
#include "creature_states.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const short around_slab[] = {-86, -85, -84,  -1,   0,   1,  84,  85,  86};
const short small_around_slab[] = {-85,   1,  85,  -1};
struct SlabMap bad_slabmap_block;
/******************************************************************************/
DLLIMPORT long _DK_calculate_effeciency_score_for_room_slab(long a1, long plyr_idx);
DLLIMPORT void _DK_unfill_reinforced_corners(unsigned char plyr_idx, unsigned char base_slb_x, unsigned char base_slb_y);
/******************************************************************************/
/**
 * Returns slab number, which stores both X and Y coords in one number.
 */
SlabCodedCoords get_slab_number(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
  if (slb_x > map_tiles_x) slb_x = map_tiles_x;
  if (slb_y > map_tiles_y) slb_y = map_tiles_y;
  if (slb_x < 0)  slb_x = 0;
  if (slb_y < 0) slb_y = 0;
  return slb_y*(map_tiles_x) + (SlabCodedCoords)slb_x;
}

/**
 * Decodes X coordinate from slab number.
 */
MapSlabCoord slb_num_decode_x(SlabCodedCoords slb_num)
{
  return slb_num % (map_tiles_x);
}

/**
 * Decodes Y coordinate from slab number.
 */
MapSlabCoord slb_num_decode_y(SlabCodedCoords slb_num)
{
  return (slb_num/(map_tiles_x))%map_tiles_y;
}

/**
 * Returns SlabMap struct for given slab number.
 */
struct SlabMap *get_slabmap_direct(SlabCodedCoords slab_num)
{
  if ((slab_num < 0) || (slab_num >= map_tiles_x*map_tiles_y))
      return INVALID_SLABMAP_BLOCK;
  return &game.slabmap[slab_num];
}

/**
 * Returns SlabMap struct for given (X,Y) slab coords.
 */
struct SlabMap *get_slabmap_block(MapSlabCoord slab_x, MapSlabCoord slab_y)
{
  if ((slab_x < 0) || (slab_x >= map_tiles_x))
      return INVALID_SLABMAP_BLOCK;
  if ((slab_y < 0) || (slab_y >= map_tiles_y))
      return INVALID_SLABMAP_BLOCK;
  return &game.slabmap[slab_y*(map_tiles_x) + slab_x];
}

/**
 * Gives SlabMap struct for given (X,Y) subtile coords.
 * @param stl_x
 * @param stl_y
 */
struct SlabMap *get_slabmap_for_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    if ((stl_x < 0) || (stl_x >= map_subtiles_x))
        return INVALID_SLABMAP_BLOCK;
    if ((stl_y < 0) || (stl_y >= map_subtiles_y))
        return INVALID_SLABMAP_BLOCK;
    return &game.slabmap[subtile_slab(stl_y)*(map_tiles_x) + subtile_slab(stl_x)];
}

/**
 * Gives SlabMap struct for slab on which given thing is placed.
 * @param thing The thing which coordinates are used to retrieve SlabMap.
 */
struct SlabMap *get_slabmap_thing_is_on(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return INVALID_SLABMAP_BLOCK;
    return get_slabmap_for_subtile(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
}

PlayerNumber get_slab_owner_thing_is_on(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return game.neutral_player_num;
    struct SlabMap *slb;
    slb = get_slabmap_for_subtile(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    return slabmap_owner(slb);
}

/**
 * Returns if given SlabMap is not a part of the map.
 */
TbBool slabmap_block_invalid(const struct SlabMap *slb)
{
    if (slb == NULL)
        return true;
    if (slb == INVALID_SLABMAP_BLOCK)
        return true;
    return (slb < &game.slabmap[0]);
}

/**
 * Returns if the slab coords are in range of existing map slabs.
 */
TbBool slab_coords_invalid(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
  if ((slb_x < 0) || (slb_x >= map_tiles_x))
      return true;
  if ((slb_y < 0) || (slb_y >= map_tiles_y))
      return true;
  return false;
}

/**
 * Returns owner index of given SlabMap.
 */
long slabmap_owner(const struct SlabMap *slb)
{
    if (slabmap_block_invalid(slb))
        return 5;
    return slb->field_5 & 0x07;
}

/**
 * Sets owner of given SlabMap.
 */
void slabmap_set_owner(struct SlabMap *slb, PlayerNumber owner)
{
    if (slabmap_block_invalid(slb))
        return;
    slb->field_5 ^= (slb->field_5 ^ owner) & 0x07;
}

/**
 * Sets owner of a slab on given position.
 */
void set_whole_slab_owner(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber owner)
{
    struct SlabMap *slb;
    MapSubtlCoord stl_x,stl_y;
    long i,k;
    stl_x = 3 * slb_x;
    stl_y = 3 * slb_y;
    for (i = 0; i < 3; i++)
    {
        for (k = 0; k < 3; k++)
        {
            slb = get_slabmap_for_subtile(stl_x + k, stl_y + i);
            slabmap_set_owner(slb, owner);
        }
    }
}

/**
 * Returns Water-Lava under Bridge flags for given SlabMap.
 */
unsigned long slabmap_wlb(struct SlabMap *slb)
{
    if (slabmap_block_invalid(slb))
        return 0;
    return (slb->field_5 >> 3) & 0x03;
}

/**
 * Sets Water-Lava under Bridge flags for given SlabMap.
 */
void slabmap_set_wlb(struct SlabMap *slb, unsigned long wlbflag)
{
    if (slabmap_block_invalid(slb))
        return;
    slb->field_5 ^= (slb->field_5 ^ (wlbflag << 3)) & 0x18;
}

/**
 * Returns slab number of the next tile in a room, after the given one.
 */
long get_next_slab_number_in_room(SlabCodedCoords slab_num)
{
    if ((slab_num < 0) || (slab_num >= map_tiles_x*map_tiles_y))
        return 0;
    return game.slabmap[slab_num].next_in_room;
}

TbBool slab_is_safe_land(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap *slb;
    struct SlabAttr *slbattr;
    int slb_owner;
    slb = get_slabmap_block(slb_x, slb_y);
    slbattr = get_slab_attrs(slb);
    slb_owner = slabmap_owner(slb);
    if ((slb_owner == plyr_idx) || (slb_owner == game.neutral_player_num))
    {
        return slbattr->is_safe_land;
    }
    return false;
}

TbBool slab_is_door(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap *slb;
    slb = get_slabmap_block(slb_x, slb_y);
    return slab_kind_is_door(slb->kind);
}

TbBool slab_is_liquid(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap *slb;
    slb = get_slabmap_block(slb_x, slb_y);
    return slab_kind_is_liquid(slb->kind);
}

TbBool slab_kind_is_animated(SlabKind slbkind)
{
    if (slab_kind_is_door(slbkind))
        return true;
    if ((slbkind == SlbT_GUARDPOST) || (slbkind == SlbT_BRIDGE))
        return true;
    return false;
}

TbBool can_build_room_at_slab(PlayerNumber plyr_idx, RoomKind rkind,
    MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    if (!subtile_revealed(slab_subtile_center(slb_x), slab_subtile_center(slb_y), plyr_idx)) {
        SYNCDBG(7,"Cannot place %s owner %d as slab (%d,%d) is not revealed",room_code_name(rkind),(int)plyr_idx,(int)slb_x,(int)slb_y);
        return false;
    }
    struct SlabMap *slb;
    slb = get_slabmap_block(slb_x, slb_y);
    if (slb->room_index > 0) {
        SYNCDBG(7,"Cannot place %s owner %d as slab (%d,%d) has room index %d",room_code_name(rkind),(int)plyr_idx,(int)slb_x,(int)slb_y,(int)slb->room_index);
        return false;
    }
    if (slab_has_trap_on(slb_x, slb_y) || slab_has_door_thing_on(slb_x, slb_y)) {
        SYNCDBG(7,"Cannot place %s owner %d as slab (%d,%d) has blocking thing on it",room_code_name(rkind),(int)plyr_idx,(int)slb_x,(int)slb_y);
        return false;
    }
    if (rkind == RoK_BRIDGE) {
        return slab_kind_is_liquid(slb->kind) && slab_by_players_land(plyr_idx, slb_x, slb_y);
    }
    if (slabmap_owner(slb) != plyr_idx) {
        return false;
    }
    return (slb->kind == SlbT_CLAIMED);
}

/**
 * Clears all SlabMap structures in the map.
 */
void clear_slabs(void)
{
    struct SlabMap *slb;
    unsigned long x,y;
    for (y=0; y < map_tiles_y; y++)
    {
        for (x=0; x < map_tiles_x; x++)
        {
          slb = &game.slabmap[y*map_tiles_x + x];
          LbMemorySet(slb, 0, sizeof(struct SlabMap));
          slb->kind = SlbT_ROCK;
        }
    }
}

long calculate_effeciency_score_for_room_slab(SlabCodedCoords slab_num, PlayerNumber plyr_idx)
{
    return _DK_calculate_effeciency_score_for_room_slab(slab_num, plyr_idx);
}

/**
 * Reveals the whole map for specific player.
 */
void reveal_whole_map(struct PlayerInfo *player)
{
    clear_dig_for_map_rect(player->id_number,0,map_tiles_x,0,map_tiles_y);
    reveal_map_rect(player->id_number,1,map_subtiles_x,1,map_subtiles_y);
    pannel_map_update(0, 0, map_subtiles_x+1, map_subtiles_y+1);
}

void update_blocks_in_area(MapSubtlCoord sx, MapSubtlCoord sy, MapSubtlCoord ex, MapSubtlCoord ey)
{
    update_navigation_triangulation(sx, sy, ex, ey);
    ceiling_partially_recompute_heights(sx, sy, ex, ey);
    light_signal_update_in_area(sx, sy, ex, ey);
}

void update_blocks_around_slab(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    MapSubtlCoord stl_x,stl_y;
    MapSubtlCoord sx,sy,ex,ey;
    SYNCDBG(7,"Starting");
    stl_x = STL_PER_SLB * slb_x;
    stl_y = STL_PER_SLB * slb_y;

    ey = stl_y + 5;
    if (ey > map_subtiles_y)
        ey = map_subtiles_y;
    ex = stl_x + 5;
    if (ex > map_subtiles_x)
        ex = map_subtiles_x;
    sy = stl_y - STL_PER_SLB;
    if (sy <= 0)
        sy = 0;
    sx = stl_x - STL_PER_SLB;
    if (sx <= 0)
        sx = 0;
    update_blocks_in_area(sx, sy, ex, ey);
}

void update_map_collide(SlabKind slbkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map *mapblk;
    mapblk = get_map_block_at(stl_x, stl_y);
    struct Column *colmn;
    colmn = get_map_column(mapblk);
    if (column_invalid(colmn)) {
        ERRORLOG("Invalid column at (%d,%d)",(int)stl_x,(int)stl_y);
    }
    unsigned long smask;
    smask = colmn->solidmask;
    MapSubtlCoord stl_z;
    for (stl_z=0; stl_z < map_subtiles_z; stl_z++)
    {
        if ((smask & 0x01) == 0)
            break;
        smask >>= 1;
    }
    struct SlabAttr *slbattr;
    slbattr = get_slab_kind_attrs(slbkind);
    unsigned long nflags;
    if (slbattr->field_2 < stl_z) {
      nflags = slbattr->block_flags;
    } else {
      nflags = slbattr->noblck_flags;
    }
    mapblk->flags &= (SlbAtFlg_Unk80|SlbAtFlg_Unk04);
    mapblk->flags |= nflags;
}

void do_slab_efficiency_alteration(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    long n;
    for (n=0; n < SMALL_AROUND_SLAB_LENGTH; n++)
    {
        MapSlabCoord sslb_x, sslb_y;
        sslb_x = slb_x + small_around[n].delta_x;
        sslb_y = slb_y + small_around[n].delta_y;
        struct SlabMap *slb;
        struct SlabAttr *slbattr;
        slb = get_slabmap_block(sslb_x, sslb_y);
        if (slabmap_block_invalid(slb)) {
            continue;
        }
        slbattr = get_slab_attrs(slb);
        if (slbattr->category == SlbAtCtg_RoomInterior)
        {
            struct Room *room;
            room = slab_room_get(sslb_x, sslb_y);
            set_room_efficiency(room);
            set_room_capacity(room, 1);
        }
    }
}

SlabKind choose_rock_type(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    unsigned char flags;
    flags = 0;
    if ((slb_x % 5) == 0)
    {
        struct SlabMap *pvslb;
        struct SlabMap *nxslb;
        pvslb = get_slabmap_block(slb_x, slb_y-1);
        nxslb = get_slabmap_block(slb_x, slb_y+1);
        if ((pvslb->kind == SlbT_CLAIMED) || (nxslb->kind == SlbT_CLAIMED)) {
            flags |= 0x01;
        }
    }
    if ((slb_y % 5) == 0)
    {
        struct SlabMap *pvslb;
        struct SlabMap *nxslb;
        pvslb = get_slabmap_block(slb_x-1, slb_y);
        nxslb = get_slabmap_block(slb_x+1, slb_y);
        if ((pvslb->kind == SlbT_CLAIMED) || (nxslb->kind == SlbT_CLAIMED)) {
            flags |= 0x02;
        }
    }
    if (flags < 1)
        return SlbT_EARTH;
    else
        return SlbT_TORCHDIRT;
}

void unfill_reinforced_corners(PlayerNumber plyr_idx, MapSlabCoord base_slb_x, MapSlabCoord base_slb_y)
{
    //_DK_unfill_reinforced_corners(plyr_idx, base_slb_x, base_slb_y); return;
    int i;
    for (i = 0; i < SMALL_AROUND_MID_LENGTH; i++)
    {
        MapSlabCoord slb_x, slb_y;
        slb_x = base_slb_x + small_around_mid[i].delta_x;
        slb_y = base_slb_y + small_around_mid[i].delta_y;
        struct SlabMap *slb;
        slb = get_slabmap_for_subtile(slb_x, slb_y);
        struct SlabAttr *slbattr;
        slbattr = get_slab_attrs(slb);
        if ((slbattr->category == SlbAtCtg_FortifiedWall) && (slabmap_owner(slb) != plyr_idx))
        {
            int n, num_owned_around;
            num_owned_around = 0;
            for (n=0; n < SMALL_AROUND_SLAB_LENGTH; n++)
            {
                MapSlabCoord sslb_x, sslb_y;
                sslb_x = slb_x + small_around[n].delta_x;
                sslb_y = slb_y + small_around[n].delta_y;
                struct SlabMap *sslb;
                sslb = get_slabmap_for_subtile(sslb_x, sslb_y);
                if (slabmap_owner(sslb) == slabmap_owner(sslb))
                {
                    struct SlabAttr *sslbattr;
                    sslbattr = get_slab_attrs(sslb);
                    if ((sslbattr->category == SlbAtCtg_FortifiedGround) || (sslbattr->category == SlbAtCtg_RoomInterior)) {
                        num_owned_around = 4;
                    } else {
                        num_owned_around++;
                    }
                }

            }
            if (num_owned_around < 2)
            {
                SlabKind slbkind;
                slbkind = alter_rock_style(SlbT_EARTH, slb_x, slb_y, game.neutral_player_num);
                place_slab_type_on_map(slbkind, slab_subtile_center(slb_x), slab_subtile_center(slb_y), game.neutral_player_num, 0);
                do_slab_efficiency_alteration(slb_x, slb_y);
            }
        }
    }
}

void do_unprettying(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    long n;
    for (n=0; n < 4; n++)
    {
        long sslb_x,sslb_y;
        struct SlabMap *slb;
        sslb_x = slb_x + (long)small_around[n].delta_x;
        sslb_y = slb_y + (long)small_around[n].delta_y;
        slb = get_slabmap_block(sslb_x, sslb_y);
        struct SlabAttr *slbattr;
        slbattr = get_slab_attrs(slb);
        if ((slbattr->category == SlbAtCtg_FortifiedWall) && (slabmap_owner(slb) != plyr_idx))
        {
            if (!slab_by_players_land(slabmap_owner(slb), sslb_x, sslb_y))
            {
                SlabKind newslab;
                newslab = choose_rock_type(plyr_idx, sslb_x, sslb_y);
                place_slab_type_on_map(newslab, slab_subtile_center(sslb_x), slab_subtile_center(sslb_y), game.neutral_player_num, 0);
                unfill_reinforced_corners(plyr_idx, sslb_x, sslb_y);
                do_slab_efficiency_alteration(sslb_x, sslb_y);
            }
        }
    }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
