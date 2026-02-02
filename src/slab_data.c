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
#include "pre_inc.h"
#include "slab_data.h"
#include "globals.h"

#include "player_instances.h"
#include "config_terrain.h"
#include "map_blocks.h"
#include "map_ceiling.h"
#include "ariadne.h"
#include "ariadne_wallhug.h"
#include "map_utils.h"
#include "frontmenu_ingame_map.h"
#include "game_legacy.h"
#include "creature_states.h"
#include "map_data.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct SlabMap bad_slabmap_block;
/******************************************************************************/
/**
 * Returns slab number, which stores both X and Y coords in one number.
 */
SlabCodedCoords get_slab_number(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
  if (slb_x > game.map_tiles_x) slb_x = game.map_tiles_x;
  if (slb_y > game.map_tiles_y) slb_y = game.map_tiles_y;
  if (slb_x < 0)  slb_x = 0;
  if (slb_y < 0) slb_y = 0;
  return slb_y*(game.map_tiles_x) + (SlabCodedCoords)slb_x;
}

/**
 * Decodes X coordinate from slab number.
 */
MapSlabCoord slb_num_decode_x(SlabCodedCoords slb_num)
{
  return slb_num % (game.map_tiles_x);
}

/**
 * Decodes Y coordinate from slab number.
 */
MapSlabCoord slb_num_decode_y(SlabCodedCoords slb_num)
{
  return (slb_num/(game.map_tiles_x))%game.map_tiles_y;
}

/**
 * Returns SlabMap struct for given slab number.
 */
struct SlabMap *get_slabmap_direct(SlabCodedCoords slab_num)
{
  if (slab_num >= game.map_tiles_x*game.map_tiles_y)
      return INVALID_SLABMAP_BLOCK;
  return &game.slabmap[slab_num];
}

/**
 * Returns SlabMap struct for given (X,Y) slab coords.
 */
struct SlabMap *get_slabmap_block(MapSlabCoord slab_x, MapSlabCoord slab_y)
{
  if ((slab_x < 0) || (slab_x >= game.map_tiles_x))
      return INVALID_SLABMAP_BLOCK;
  if ((slab_y < 0) || (slab_y >= game.map_tiles_y))
      return INVALID_SLABMAP_BLOCK;
  return &game.slabmap[slab_y*(game.map_tiles_x) + slab_x];
}

/**
 * Gives SlabMap struct for given (X,Y) subtile coords.
 * @param stl_x
 * @param stl_y
 */
struct SlabMap *get_slabmap_for_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    if ((stl_x < 0) || (stl_x >= game.map_subtiles_x))
        return INVALID_SLABMAP_BLOCK;
    if ((stl_y < 0) || (stl_y >= game.map_subtiles_y))
        return INVALID_SLABMAP_BLOCK;
    return &game.slabmap[subtile_slab(stl_y)*(game.map_tiles_x) + subtile_slab(stl_x)];
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
    struct SlabMap* slb = get_slabmap_for_subtile(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
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
  if ((slb_x < 0) || (slb_x >= game.map_tiles_x))
      return true;
  if ((slb_y < 0) || (slb_y >= game.map_tiles_y))
      return true;
  return false;
}

/**
 * Returns owner index of given SlabMap.
 */
long slabmap_owner(const struct SlabMap *slb)
{
    if (slabmap_block_invalid(slb))
        return PLAYER_NEUTRAL;
    return slb->owner;
}

/**
 * Sets owner of a slab on given position.
 */
void set_slab_owner(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber owner)
{
    struct SlabMap* slb = get_slabmap_block(slb_x,slb_y);
    if (slabmap_block_invalid(slb))
    {
        return;
    }
    struct Dungeon *dungeon = get_dungeon(owner);
    if (dungeon->texture_pack == 0)
    {
        game.slab_ext_data[get_slab_number(slb_x,slb_y)] = game.slab_ext_data_initial[get_slab_number(slb_x,slb_y)];
    }
    else
    {
        game.slab_ext_data[get_slab_number(slb_x,slb_y)] = dungeon->texture_pack;
    }
    slb->owner = owner;
}

/**
 * Returns Water-Lava under Bridge flags for given SlabMap.
 */
unsigned long slabmap_wlb(struct SlabMap *slb)
{
    if (slabmap_block_invalid(slb))
        return WlbT_None;
    return slb->wlb_type;
}

/**
 * Sets Water-Lava under Bridge flags for given SlabMap.
 */
void slabmap_set_wlb(struct SlabMap *slb, unsigned long wlb_type)
{
    if (slabmap_block_invalid(slb))
        return;
    slb->wlb_type = wlb_type;
}

/**
 * Returns slab number of the next tile in a room, after the given one.
 */
SlabCodedCoords get_next_slab_number_in_room(SlabCodedCoords slab_num)
{
    if (slab_num >= game.map_tiles_x * game.map_tiles_y)
    {
        ERRORLOG("Slabnumber %u exceeds map dimensions %d*%d", slab_num, game.map_tiles_x, game.map_tiles_y);
        return 0;
    }
    return game.slabmap[slab_num].next_in_room;
}

TbBool slab_is_safe_land(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    struct SlabConfigStats* slabst = get_slab_stats(slb);
    int slb_owner = slabmap_owner(slb);
    if ((slb_owner == plyr_idx) || (slb_owner == game.neutral_player_num))
    {
        return slabst->is_safe_land;
    }
    return false;
}

TbBool slab_is_door(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    return slab_kind_is_door(slb->kind);
}

TbBool slab_is_liquid(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    return slab_kind_is_liquid(slb->kind);
}

TbBool slab_is_wall(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    MapSubtlCoord stl_x = slab_subtile_center(slb_x);
    MapSubtlCoord stl_y = slab_subtile_center(slb_y);
    for (int i = 0; i < SMALL_AROUND_LENGTH; i++)
    {
        MapSubtlCoord astl_x = stl_x + small_around[i].delta_x;
        MapSubtlCoord astl_y = stl_y + small_around[i].delta_y;
        if (!subtile_is_wall(astl_x, astl_y))
        {
            return false;
        }
    }
    return true;
}

TbBool is_slab_type_walkable(SlabKind slbkind)
{
    struct SlabConfigStats* slabst = get_slab_kind_stats(slbkind);
    return (slabst->block_flags & (SlbAtFlg_Blocking | SlbAtFlg_Digable | SlbAtFlg_Valuable)) == 0;
}

TbBool slab_kind_is_animated(SlabKind slbkind)
{
    struct SlabConfigStats* slabst = get_slab_kind_stats(slbkind);
    return slabst->animated;
}

TbBool slab_good_for_computer_dig_path(const struct SlabMap *slb)
{
    const struct SlabConfigStats* slabst = get_slab_stats(slb);
    if ( any_flag_is_set(slabst->block_flags, (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) || (slb->kind == SlbT_LAVA) )
        return true;
    return false;
}

TbBool is_valid_hug_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
    const struct SlabConfigStats* slabst = get_slab_stats(slb);
    if ((slabst->is_diggable) && !slab_kind_is_indestructible(slb->kind))
    {
        struct Map* mapblk = get_map_block_at(stl_x, stl_y);
        if (!flag_is_set(mapblk->flags, SlbAtFlg_Filled) || (slabmap_owner(slb) == plyr_idx)) {
            SYNCDBG(17,"Subtile (%d,%d) rejected based on attrs",(int)stl_x,(int)stl_y);
            return false;
        }
    }
    if (!slab_good_for_computer_dig_path(slb)) {
        SYNCDBG(17,"Subtile (%d,%d) rejected as not good for dig",(int)stl_x,(int)stl_y);
        return false;
    }
    return true;
}

TbBool can_build_room_at_slab(PlayerNumber plyr_idx, RoomKind rkind,
    MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    if (!subtile_revealed(slab_subtile_center(slb_x), slab_subtile_center(slb_y), plyr_idx)) {
        SYNCDBG(7,"Cannot place %s owner %d as slab (%d,%d) is not revealed",room_code_name(rkind),(int)plyr_idx,(int)slb_x,(int)slb_y);
        return false;
    }
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    if (slb->room_index > 0) {
        SYNCDBG(7,"Cannot place %s owner %d as slab (%d,%d) has room index %d",room_code_name(rkind),(int)plyr_idx,(int)slb_x,(int)slb_y,(int)slb->room_index);
        return false;
    }
    if (slab_has_trap_on(slb_x, slb_y) || slab_has_door_thing_on(slb_x, slb_y)) {
        SYNCDBG(7,"Cannot place %s owner %d as slab (%d,%d) has blocking thing on it",room_code_name(rkind),(int)plyr_idx,(int)slb_x,(int)slb_y);
        return false;
    }
    if (room_role_matches(rkind,RoRoF_PassWater|RoRoF_PassLava)) {
        return ((room_role_matches(rkind,RoRoF_PassWater) && (slb->kind == SlbT_WATER && slab_by_players_land(plyr_idx, slb_x, slb_y))) ||
                (room_role_matches(rkind,RoRoF_PassLava)  && (slb->kind == SlbT_LAVA  && slab_by_players_land(plyr_idx, slb_x, slb_y))));
    }
    if (slabmap_owner(slb) != plyr_idx) {
        return false;
    }
    return (slb->kind == SlbT_CLAIMED);
}

TbBool can_build_room_at_slab_fast(PlayerNumber plyr_idx, RoomKind rkind, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    if (room_role_matches(rkind,RoRoF_PassWater|RoRoF_PassLava))
    {
        return ((room_role_matches(rkind,RoRoF_PassWater) && (slb->kind == SlbT_WATER && slab_by_players_land(plyr_idx, slb_x, slb_y))) ||
                (room_role_matches(rkind,RoRoF_PassLava)  && (slb->kind == SlbT_LAVA  && slab_by_players_land(plyr_idx, slb_x, slb_y))));
    }
    else
    {
        if (slb->kind == SlbT_CLAIMED)
        {
            return (slabmap_owner(slb) == plyr_idx);
        }
    }
    return false;
}

int check_room_at_slab_loose(PlayerNumber plyr_idx, RoomKind rkind, MapSlabCoord slb_x, MapSlabCoord slb_y, int looseness)
{
    // looseness:
    // don't allow tile = 0
    // valid tile to place room = 1 (i.e. tile owned by current player, and is claimed path)
    // allow same room type = 2
    // allow other room types = 3
    // allow gems = 4
    // allow gold = 5
    // allow liquid = 6
    // allow rock = 7
    // allow path = 8
    // allow path claimed by others = 9

    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    int result = 0;
    if (room_role_matches(rkind,RoRoF_PassWater|RoRoF_PassLava))
    {
        result = ((room_role_matches(rkind,RoRoF_PassWater) && (slb->kind == SlbT_WATER && slab_by_players_land(plyr_idx, slb_x, slb_y))) ||
                  (room_role_matches(rkind,RoRoF_PassLava)  && (slb->kind == SlbT_LAVA  && slab_by_players_land(plyr_idx, slb_x, slb_y)))); // 0 or 1
    }
    else
    {
        if (slb->kind == SlbT_CLAIMED)
        {
            if (slabmap_owner(slb) == plyr_idx)
            {
                result = 1; // valid tile
            }
            else
            {
                result = 9; // claimed dirt owned by other player
            }
        }
        else
        {
            if (slab_is_wall(slb_x, slb_y))
            {
                if (slb->kind == SlbT_GEMS)
                {
                    result = 4;
                }
                else if ((slb->kind == SlbT_GOLD) || (slb->kind == SlbT_DENSEGOLD))
                {
                    result = 5;
                }
                else if (slb->kind == SlbT_ROCK)
                {
                    result = 7; // is unbreakable rock
                }
            }
            else if (slab_kind_is_liquid(slb->kind))
            {
                result = 6; // is water or lava
            }
            else if (slb->kind == SlbT_PATH)
            {
                result = 8; //unclaimed path
            }
            else if (slabmap_owner(slb) == plyr_idx)
            {
                int slab_type_from_room_kind = room_corresponding_slab(rkind);

                if (slab_type_from_room_kind == slb->kind)
                {
                    result = 2; // same room type
                }
                else if (slab_type_from_room_kind > 0)
                {
                    result = 3; // different room type
                }

            }
        }
    }
    if (result > looseness)
    {
        // adjusting the "looseness" that is passed to this function allows different slab types to be considered "valid" tiles to place a room, for the purposes of finding a room.
        // A room is checked for valid/invalid tiles later in the process, before it is shown to the user with the bounding box.
        result = 0;
    }
    return result;
}

/**
 * Clears all SlabMap structures in the map.
 */
void clear_slabs(void)
{
    for (unsigned long y = 0; y < game.map_tiles_y; y++)
    {
        for (unsigned long x = 0; x < game.map_tiles_x; x++)
        {
            struct SlabMap* slb = &game.slabmap[y * game.map_tiles_x + x];
            memset(slb, 0, sizeof(struct SlabMap));
            slb->kind = SlbT_ROCK;
        }
    }
}

SlabKind find_core_slab_type(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    struct SlabConfigStats* slabst = get_slab_stats(slb);
    SlabKind corekind;
    switch (slabst->category)
    {
    case SlbAtCtg_FriableDirt:
        corekind = SlbT_EARTH;
        break;
    case SlbAtCtg_FortifiedWall:
        corekind = SlbT_WALLDRAPE;
        break;
    case SlbAtCtg_Obstacle:
        // originally, 99 was returned by this case, without further conditions
        if ((slabst->block_flags & SlbAtFlg_IsRoom) != 0)
            corekind = SlbT_BRIDGE;
        else
            corekind = SlbT_DOORWOOD1;
        break;
    default:
        corekind = slb->kind;
        break;
    }
    return corekind;
}

long calculate_effeciency_score_for_room_slab(SlabCodedCoords slab_num, PlayerNumber plyr_idx, short synergy_slab_num)
{
    TbBool is_room_inside = true;
    long eff_score = 0;
    struct SlabMap* slb = get_slabmap_direct(slab_num);
    long n;
    for (n=1; n < AROUND_SLAB_LENGTH; n+=2)
    {
        long round_slab_num = slab_num + game.around_slab[n];
        struct SlabMap* round_slb = get_slabmap_direct(round_slab_num);
        if (!slabmap_block_invalid(round_slb))
        {
            MapSlabCoord slb_x = slb_num_decode_x(round_slab_num);
            MapSlabCoord slb_y = slb_num_decode_y(round_slab_num);
            // Per slab code
            if ((slabmap_owner(round_slb) == slabmap_owner(slb)) && (round_slb->kind == slb->kind))
            {
                eff_score += 2;
            } else if (((slabmap_owner(round_slb) == slabmap_owner(slb)) || !(get_slab_kind_stats(synergy_slab_num)->is_ownable)) && ((round_slb->kind == synergy_slab_num) && (synergy_slab_num >= 0)))
            {
                eff_score += 2;
            } else
            {
                is_room_inside = false;
                switch (find_core_slab_type(slb_x, slb_y))
                {
                  case SlbT_ROCK:
                  case SlbT_GOLD:
                  case SlbT_DENSEGOLD:
                  case SlbT_EARTH:
                  case SlbT_GEMS:
                    eff_score++;
                    break;
                  case SlbT_WALLDRAPE:
                      if (slabmap_owner(round_slb) == slabmap_owner(slb))
                      {
                          eff_score += 2;
                      }
                      else
                      {
                          eff_score++;
                      }
                    break;
                  case SlbT_DOORWOOD1:
                    if (slabmap_owner(round_slb) == slabmap_owner(slb))
                        eff_score += 2;
                    break;
                  default:
                    break;
                }
            }
            // Per slab code ends
        }
    }
    // If we already know this is not an inside - finish
    if (!is_room_inside) {
        return eff_score;
    }
    // Make sure this is room inside by checking corners
    for (n=0; n < AROUND_SLAB_LENGTH; n+=2)
    {
        long round_slab_num = slab_num + game.around_slab[n];
        struct SlabMap* round_slb = get_slabmap_direct(round_slab_num);
        if (!slabmap_block_invalid(round_slb))
        {
            // Per slab code
            if ((slabmap_owner(round_slb) != slabmap_owner(slb)) || (round_slb->kind != slb->kind))
            {
                is_room_inside = 0;
                break;
            }
            // Per slab code ends
        }
    }
    if (is_room_inside) {
        eff_score += 2;
    }
    return eff_score;
}

/**
 * Reveals the whole map for specific player.
 */
void reveal_whole_map(struct PlayerInfo *player)
{
    clear_dig_for_map_rect(player->id_number,0,game.map_tiles_x,0,game.map_tiles_y);
    reveal_map_rect(player->id_number,1,game.map_subtiles_x,1,game.map_subtiles_y);
    panel_map_update(0, 0, game.map_subtiles_x+1, game.map_subtiles_y+1);
}

void update_blocks_in_area(MapSubtlCoord sx, MapSubtlCoord sy, MapSubtlCoord ex, MapSubtlCoord ey)
{
    update_navigation_triangulation(sx, sy, ex, ey);
    ceiling_partially_recompute_heights(sx, sy, ex, ey);
    light_signal_update_in_area(sx, sy, ex, ey);
}

void update_blocks_around_slab(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    SYNCDBG(7,"Starting");
    MapSubtlCoord stl_x = STL_PER_SLB * slb_x;
    MapSubtlCoord stl_y = STL_PER_SLB * slb_y;

    MapSubtlCoord ey = stl_y + 5;
    if (ey > game.map_subtiles_y)
        ey = game.map_subtiles_y;
    MapSubtlCoord ex = stl_x + 5;
    if (ex > game.map_subtiles_x)
        ex = game.map_subtiles_x;
    MapSubtlCoord sy = stl_y - STL_PER_SLB;
    if (sy <= 0)
        sy = 0;
    MapSubtlCoord sx = stl_x - STL_PER_SLB;
    if (sx <= 0)
        sx = 0;
    update_blocks_in_area(sx, sy, ex, ey);
}

void update_map_collide(SlabKind slbkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    struct Column* colmn = get_map_column(mapblk);
    if (column_invalid(colmn)) {
        ERRORLOG("Invalid column at (%d,%d)",(int)stl_x,(int)stl_y);
    }
    unsigned long smask = colmn->solidmask;
    MapSubtlCoord stl_z;
    for (stl_z=0; stl_z < map_subtiles_z; stl_z++)
    {
        if ((smask & 0x01) == 0)
            break;
        smask >>= 1;
    }
    struct SlabConfigStats* slabst = get_slab_kind_stats(slbkind);
    unsigned long nflags;
    if (slabst->block_flags_height < stl_z) {
      nflags = slabst->block_flags;
    } else {
      nflags = slabst->noblck_flags;
    }
    mapblk->flags &= (SlbAtFlg_TaggedValuable|SlbAtFlg_Unexplored);
    mapblk->flags |= nflags;
}

void do_slab_efficiency_alteration(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    for (long n = 0; n < SMALL_AROUND_SLAB_LENGTH; n++)
    {
        MapSlabCoord sslb_x = slb_x + small_around[n].delta_x;
        MapSlabCoord sslb_y = slb_y + small_around[n].delta_y;
        struct SlabMap* slb = get_slabmap_block(sslb_x, sslb_y);
        if (slabmap_block_invalid(slb)) {
            continue;
        }
        struct SlabConfigStats* slabst = get_slab_stats(slb);
        if (slabst->category == SlbAtCtg_RoomInterior)
        {
            struct Room* room = slab_room_get(sslb_x, sslb_y);
            do_room_recalculation(room);
        }
    }
}

SlabKind choose_rock_type(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    unsigned char flags = 0;
    if ((slb_x % 5) == 0)
    {
        struct SlabMap* pvslb = get_slabmap_block(slb_x, slb_y - 1);
        struct SlabMap* nxslb = get_slabmap_block(slb_x, slb_y + 1);
        if ((pvslb->kind == SlbT_CLAIMED) || (nxslb->kind == SlbT_CLAIMED)) {
            flags |= 0x01;
        }
    }
    if ((slb_y % 5) == 0)
    {
        struct SlabMap* pvslb = get_slabmap_block(slb_x - 1, slb_y);
        struct SlabMap* nxslb = get_slabmap_block(slb_x + 1, slb_y);
        if ((pvslb->kind == SlbT_CLAIMED) || (nxslb->kind == SlbT_CLAIMED)) {
            flags |= 0x02;
        }
    }
    if (flags < 1)
        return SlbT_EARTH;
    else
        return SlbT_TORCHDIRT;
}

/**
 * Counts number of tiles owned by given player around given slab.
 * @param plyr_idx Owning player to be checked.
 * @param slb_x Target slab to check around, X coord.
 * @param slb_y Target slab to check around, Y coord.
 * @return Number of owned slabs.
 */
int count_owned_ground_around(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y, TbBool IncludeDiagonals)
{
    int num_owned = 0;
    int i;
    MapSlabCoord sslb_x;
    MapSlabCoord sslb_y;
    for (i = 0; i < SMALL_AROUND_SLAB_LENGTH; i++)
    {
        sslb_x = slb_x + small_around[i].delta_x;
        sslb_y = slb_y + small_around[i].delta_y;
        struct SlabMap* slb = get_slabmap_block(sslb_x, sslb_y);
        if (slabmap_owner(slb) == plyr_idx)
        {
            struct SlabConfigStats* slabst = get_slab_stats(slb);
            if ((slabst->category == SlbAtCtg_FortifiedGround) || (slabst->block_flags & SlbAtFlg_IsRoom) || (slabst->block_flags & SlbAtFlg_IsDoor))
            {
                num_owned++;
            }
        }
    }
    if (IncludeDiagonals)
    {
        for (i = 5; i < MID_AROUND_LENGTH; i++)
        {
            sslb_x = slb_x + mid_around[i].delta_x;
            sslb_y = slb_y + mid_around[i].delta_y;
            struct SlabMap* slb = get_slabmap_block(sslb_x, sslb_y);
            if (slabmap_owner(slb) == plyr_idx)
            {
                struct SlabConfigStats* slabst = get_slab_stats(slb);
                if ((slabst->category == SlbAtCtg_FortifiedGround) || (slabst->block_flags & SlbAtFlg_IsRoom) || (slabst->block_flags & SlbAtFlg_IsDoor))
                {
                    num_owned++;
                }
            }
        }
    }
    return num_owned;
}

void unfill_reinforced_corners(PlayerNumber keep_plyr_idx, MapSlabCoord base_slb_x, MapSlabCoord base_slb_y)
{
    for (long n = 0; n < SMALL_AROUND_LENGTH; n++)
    {
        MapSlabCoord x = base_slb_x + small_around[n].delta_x;
        MapSlabCoord y = base_slb_y + small_around[n].delta_y;
        struct SlabMap *slb = get_slabmap_block(x, y);
        struct SlabConfigStats* slabst = get_slab_stats(slb);
        if ( ( (((slabst->category == SlbAtCtg_FortifiedGround) || (slabst->block_flags & SlbAtFlg_IsRoom) || ((slabst->block_flags & SlbAtFlg_IsDoor)) ))
      && (slabmap_owner(slb) == keep_plyr_idx) ) || (slabst->category == SlbAtCtg_Unclaimed) )
        {
            for (int k = -1; k < 2; k+=2)
            {
                int j = (k + n) & 3;
                MapSlabCoord x2 = x + small_around[j].delta_x;
                MapSlabCoord y2 = y + small_around[j].delta_y;
                struct SlabMap *slb2 = get_slabmap_block(x2, y2);
                struct SlabConfigStats* slabst2 = get_slab_stats(slb2);
                if ( (slabst2->category == SlbAtCtg_FortifiedWall) || (slabst2->category == SlbAtCtg_FriableDirt) )
                {
                    int m = (k + j) & 3;
                    MapSlabCoord x3 = x2 + small_around[m].delta_x;
                    MapSlabCoord y3 = y2 + small_around[m].delta_y;
                    struct SlabMap *slb3 = get_slabmap_block(x3, y3);
                    struct SlabConfigStats* slabst3 = get_slab_stats(slb3);
                    if ( (slabst3->category == SlbAtCtg_FortifiedWall) && (slabmap_owner(slb3) != keep_plyr_idx) )
                    {
                        if (count_owned_ground_around(slabmap_owner(slb3), x3, y3, true) == 0)
                        {
                            SlabKind slbkind = alter_rock_style(SlbT_EARTH, x3, y3, game.neutral_player_num);
                            place_slab_type_on_map(slbkind, slab_subtile_center(x3), slab_subtile_center(y3), game.neutral_player_num, 0);
                            do_slab_efficiency_alteration(x3, y3);
                        }
                    }
                }
            }
        }
    }
}

/**
 * Removes reinforces walls from tiles around given slab, except given owner.
 * @param keep_plyr_idx The owning player whose walls are not to be affected.
 * @param slb_x Central slab for the unprettying effect, X coord.
 * @param slb_y Central slab for the unprettying effect, Y coord.
 */
void do_unprettying(PlayerNumber keep_plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    for (long n = 0; n < SMALL_AROUND_SLAB_LENGTH; n++)
    {
        long sslb_x = slb_x + (long)small_around[n].delta_x;
        long sslb_y = slb_y + (long)small_around[n].delta_y;
        struct SlabMap* slb = get_slabmap_block(sslb_x, sslb_y);
        struct SlabConfigStats* slabst = get_slab_stats(slb);
        if ((slabst->category == SlbAtCtg_FortifiedWall) && (slabmap_owner(slb) != keep_plyr_idx))
        {
            if (!slab_by_players_land(slabmap_owner(slb), sslb_x, sslb_y))
            {
                SlabKind newslab = choose_rock_type(keep_plyr_idx, sslb_x, sslb_y);
                place_slab_type_on_map(newslab, slab_subtile_center(sslb_x), slab_subtile_center(sslb_y), game.neutral_player_num, 0);
                unfill_reinforced_corners(keep_plyr_idx, sslb_x, sslb_y);
                do_slab_efficiency_alteration(sslb_x, sslb_y);
            }
        }
    }
}

TbBool slab_kind_has_no_ownership(SlabKind slbkind)
{
    struct SlabConfigStats* slabst = get_slab_kind_stats(slbkind);
    return (slabst->is_ownable == 0);
}

TbBool players_land_by_slab_kind(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y, SlabKind slbkind)
{
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    if (slabmap_owner(slb) == plyr_idx)
    {
        for (long n = 0; n < SMALL_AROUND_LENGTH; n++)
        {
            MapSlabCoord aslb_x = slb_x + small_around[n].delta_x;
            MapSlabCoord aslb_y = slb_y + small_around[n].delta_y;
            struct SlabMap* aslb = get_slabmap_block(aslb_x, aslb_y);
            if (aslb->kind == slbkind)
            {
                return true;
            }
        }
    }
    return false;
}


/**
 * Returns whether the given slab has an adjacent slab owned by given player.
 * @param plyr_idx
 * @param slb_x
 * @param slb_y
 * @return
 */
TbBool slab_by_players_land(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    for (long n = 0; n < SMALL_AROUND_LENGTH; n++)
    {
        MapSlabCoord aslb_x = slb_x + small_around[n].delta_x;
        MapSlabCoord aslb_y = slb_y + small_around[n].delta_y;
        struct SlabMap* slb = get_slabmap_block(aslb_x, aslb_y);
        if (slabmap_owner(slb) == plyr_idx)
        {
            if (slab_is_safe_land(plyr_idx, aslb_x, aslb_y) && !slab_is_liquid(aslb_x, aslb_y)) {
                return true;
            }
        }
    }
    return false;
}

TbBool player_can_claim_slab(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
    PlayerNumber prev_owner = slabmap_owner(slb);
    if (prev_owner == plyr_idx)
        return false;
    if (players_are_mutual_allies(plyr_idx, prev_owner)) {
        SYNCDBG(8,"The slab %d,%d is owned by ally, so cannot be converted",(int)slb_x, (int)slb_y);
        return false;
    }

    struct Room *room = room_get(slb->room_index);
    if ((slb->kind != SlbT_CLAIMED) && (room_is_invalid(room) || (flag_is_set(get_room_kind_stats(room->kind)->flags, RoCFlg_CannotBeClaimed)))) {
        SYNCDBG(8,"The slab %d,%d is not a valid kind %d to be converted",(int)slb_x, (int)slb_y, (int)slb->kind);
        return false;
    }
    struct Map *mapblk = get_map_block_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    if (!map_block_revealed(mapblk, plyr_idx)) {
        SYNCDBG(8,"The slab %d,%d is not revealed",(int)slb_x, (int)slb_y);
        return false;
    }
    if (!slab_by_players_land(plyr_idx, slb_x, slb_y)) {
        SYNCDBG(8,"The slab %d,%d is not by players land",(int)slb_x, (int)slb_y);
        return false;
    }
    return true;
}

void set_player_texture(PlayerNumber plyr_idx, long texture_id)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    dungeon->texture_pack = texture_id;

    for (MapSlabCoord slb_y=0; slb_y < game.map_tiles_y; slb_y++)
    {
        for (MapSlabCoord slb_x=0; slb_x < game.map_tiles_x; slb_x++)
        {
            struct SlabMap* slb = get_slabmap_block(slb_x,slb_y);
            if (slabmap_owner(slb) == plyr_idx)
            {
                if (texture_id == 0)
                {
                    game.slab_ext_data[get_slab_number(slb_x,slb_y)] = game.slab_ext_data_initial[get_slab_number(slb_x,slb_y)];
                }
                else
                {
                    game.slab_ext_data[get_slab_number(slb_x,slb_y)] = texture_id;
                }
            }
        }
    }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
