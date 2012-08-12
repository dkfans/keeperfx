/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file map_blocks.c
 *     Map blocks support functions.
 * @par Purpose:
 *     Functions to manage map blocks.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "map_blocks.h"

#include "globals.h"
#include "bflib_basics.h"

#include "slab_data.h"
#include "room_data.h"
#include "thing_effects.h"
#include "thing_objects.h"
#include "config_terrain.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_mine_out_block(long a1, long a2, long stl_height);
DLLIMPORT unsigned char _DK_dig_has_revealed_area(long a1, long a2, unsigned char stl_height);
DLLIMPORT void _DK_dig_out_block(long a1, long a2, long stl_height);
DLLIMPORT void _DK_check_map_explored(struct Thing *thing, long a2, long stl_height);
DLLIMPORT void _DK_create_gold_rubble_for_dug_block(long x, long y, unsigned char stl_height, unsigned char a4);
DLLIMPORT long _DK_untag_blocks_for_digging_in_area(long slb_x, long slb_y, signed char stl_height);
DLLIMPORT void _DK_set_slab_explored_flags(unsigned char flag, long slb_x, long slb_y);
DLLIMPORT long _DK_ceiling_partially_recompute_heights(long sx, long sy, long ex, long ey);
DLLIMPORT long _DK_element_top_face_texture(struct Map *map);
DLLIMPORT void _DK_place_single_slab_type_on_map(long a1, unsigned char a2, unsigned char a3, unsigned char a4);
DLLIMPORT void _DK_shuffle_unattached_things_on_slab(long a1, long a2);

/******************************************************************************/
TbBool block_has_diggable_side(long plyr_idx, long slb_x, long slb_y)
{
  long i;
  for (i = 0; i < SMALL_AROUND_SLAB_LENGTH; i++)
  {
    if (slab_is_safe_land(plyr_idx, slb_x + small_around[i].delta_x, slb_y + small_around[i].delta_y))
      return true;
  }
  return false;
}

void create_gold_rubble_for_dug_block(MapSubtlCoord stl_x, MapSubtlCoord stl_y, MapSubtlCoord stl_height, PlayerNumber owner)
{
    _DK_create_gold_rubble_for_dug_block(stl_x, stl_y, stl_height, owner);
}

void create_dirt_rubble_for_dug_block(MapSubtlCoord stl_x, MapSubtlCoord stl_y, MapSubtlCoord stl_height, PlayerNumber owner)
{
    struct Coord3d pos;
    MapCoord maxpos_z;
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = subtile_coord_center(1);
    maxpos_z = subtile_coord(stl_height,0);
    while (pos.z.val < maxpos_z)
    {
        create_effect(&pos, 26, owner);
        pos.z.val += subtile_coord(1,0);
    }
}

long untag_blocks_for_digging_in_area(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    struct Map *mapblk;
    MapSubtlCoord x, y;
    long num_untagged;
    long task_idx;
    long i;
    //return _DK_untag_blocks_for_digging_in_area(slb_x, slb_y, plyr_idx);
    x = 3 * (stl_x/3);
    y = 3 * (stl_y/3);
    if ( (x < 0) || (x >= map_subtiles_x) || (y < 0) || (y >= map_subtiles_y) ) {
        ERRORLOG("Attempt to tag area outside of map");
        return 0;
    }
    i = get_subtile_number(x+1,y+1);
    task_idx = find_from_task_list(plyr_idx, i);
    if (task_idx != -1) {
        remove_from_task_list(plyr_idx, task_idx);
    }
    num_untagged = 0;
    if (is_my_player_number(plyr_idx))
    {
        long dx,dy;
        for (dy=0; dy < 3; dy++)
        {
            for (dx=0; dx < 3; dx++)
            {
                mapblk = get_map_block_at(x+dx, y+dy);
                if (map_block_invalid(mapblk))
                    continue;
                if ( mapblk->flags & (MapFlg_Unkn80|MapFlg_Unkn04) )
                  num_untagged++;
                mapblk->flags &= ~MapFlg_Unkn80;
                mapblk->flags &= ~MapFlg_Unkn04;
            }
        }
    }
    pannel_map_update(x, y, 3, 3);
    return num_untagged;
}

void all_players_untag_blocks_for_digging_in_area(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct PlayerInfo *player;
    struct Map *map;
    PlayerNumber plyr_idx;
    map = get_map_block_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    for (plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        if ((plyr_idx == hero_player_number) || (plyr_idx == game.neutral_player_num))
            continue;
        player = get_player(plyr_idx);
        if (player_exists(player))
        {
            if (map_block_revealed(map, plyr_idx))
            {
                untag_blocks_for_digging_in_area(slab_subtile(slb_x,0), slab_subtile(slb_y,0), plyr_idx);
            }
        }
    }
}

TbBool set_slab_explored(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    if ( (plyr_idx == game.neutral_player_num) || subtile_revealed(slab_subtile_center(slb_x), slab_subtile_center(slb_y), plyr_idx) )
    {
        return false;
    }
    reveal_map_subtile(slab_subtile(slb_x,0), slab_subtile(slb_y,0), plyr_idx);
    reveal_map_subtile(slab_subtile(slb_x,1), slab_subtile(slb_y,0), plyr_idx);
    reveal_map_subtile(slab_subtile(slb_x,2), slab_subtile(slb_y,0), plyr_idx);
    reveal_map_subtile(slab_subtile(slb_x,0), slab_subtile(slb_y,1), plyr_idx);
    reveal_map_subtile(slab_subtile(slb_x,1), slab_subtile(slb_y,1), plyr_idx);
    reveal_map_subtile(slab_subtile(slb_x,2), slab_subtile(slb_y,1), plyr_idx);
    reveal_map_subtile(slab_subtile(slb_x,0), slab_subtile(slb_y,2), plyr_idx);
    reveal_map_subtile(slab_subtile(slb_x,1), slab_subtile(slb_y,2), plyr_idx);
    reveal_map_subtile(slab_subtile(slb_x,2), slab_subtile(slb_y,2), plyr_idx);
    pannel_map_update(slab_subtile(slb_x,0), slab_subtile(slb_y,0), slab_subtile(1,0), slab_subtile(1,0));
    return true;
}

void set_slab_explored_flags(unsigned char flag, long slb_x, long slb_y)
{
    _DK_set_slab_explored_flags(flag, slb_x, slb_y);
}

void neutralise_enemy_block(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber domn_plyr_idx)
{
    struct SlabMap *slb;
    MapSlabCoord slb_x,slb_y;
    slb_x = subtile_slab_fast(stl_x);
    slb_y = subtile_slab_fast(stl_y);
    slb = get_slabmap_block(slb_x, slb_y);
    switch (slabmap_wlb(slb))
    {
    case 1:
        place_slab_type_on_map(SlbT_LAVA,  slab_subtile(slb_x,0), slab_subtile(slb_y,0), game.neutral_player_num, 0);
        break;
    case 2:
        place_slab_type_on_map(SlbT_WATER, slab_subtile(slb_x,0), slab_subtile(slb_y,0), game.neutral_player_num, 0);
        break;
    default:
        place_slab_type_on_map(SlbT_PATH,  slab_subtile(slb_x,0), slab_subtile(slb_y,0), game.neutral_player_num, 1);
        break;
    }
    do_slab_efficiency_alteration(slb_x, slb_y);
}

unsigned short torch_flags_for_slab(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap *sslb1,*sslb2;
    unsigned short tflag;
    tflag = 0;
    if ((slb_x % 5) == 0)
    {
        sslb1 = get_slabmap_block(slb_x,slb_y+1);
        sslb2 = get_slabmap_block(slb_x,slb_y-1);
        if ((sslb1->kind == SlbT_CLAIMED) || (sslb2->kind == SlbT_CLAIMED))
            tflag |= 0x01;
    }
    if ((slb_y % 5) == 0)
    {
        sslb1 = get_slabmap_block(slb_x+1,slb_y);
        sslb2 = get_slabmap_block(slb_x-1,slb_y);
        if ((sslb1->kind == SlbT_CLAIMED) || (sslb2->kind == SlbT_CLAIMED))
            tflag |= 0x02;
    }
    return tflag;
}

long delete_all_object_things_from_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, long rmeffect)
{
    long stl_num;
    long removed_num;
    long n;
    stl_num = get_subtile_number(slab_subtile_center(slb_x),slab_subtile_center(slb_y));
    removed_num = 0;
    for (n=0; n < 9; n++)
    {
        struct Thing *thing;
        struct Map *mapblk;
        struct Coord3d pos;
        unsigned long k;
        long i;
        mapblk = get_map_block_at_pos(stl_num+around_map[n]);
        k = 0;
        i = get_mapwho_thing_index(mapblk);
        while (i != 0)
        {
          thing = thing_get(i);
          if (thing_is_invalid(thing))
          {
            WARNLOG("Jump out of things array");
            break;
          }
          i = thing->next_on_mapblk;
          // Per thing code
          if (thing->class_id == TCls_Object)
          {
              if (rmeffect > 0)
              {
                  set_coords_to_slab_center(&pos,slb_x,slb_y);
                  pos.z.val = get_floor_height_at(&pos);
                  create_effect(&pos, rmeffect, thing->owner);
              }
              delete_thing_structure(thing, 0);
              removed_num++;
          }
          // Per thing code ends
          k++;
          if (k > THINGS_COUNT)
          {
              ERRORLOG("Infinite loop detected when sweeping things list");
              break;
          }
        }
    }
    return removed_num;
}

long delete_unwanted_things_from_liquid_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, long rmeffect)
{
    struct Thing *thing;
    struct Map *mapblk;
    struct Objects *objdat;
    struct Coord3d pos;
    long removed_num;
    unsigned long k;
    long i,n;
    removed_num = 0;
    for (n=0; n < 9; n++)
    {
      mapblk = get_map_block_at_pos(get_subtile_number(slab_subtile_center(slb_x),slab_subtile_center(slb_y))+around_map[n]);
      k = 0;
      i = get_mapwho_thing_index(mapblk);
      while (i != 0)
      {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            WARNLOG("Jump out of things array");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code
        if (thing->class_id == TCls_Object)
        {
            objdat = get_objects_data_for_thing(thing);
            if (objdat->field_15)
            {
                if (rmeffect > 0)
                {
                    set_coords_to_slab_center(&pos,slb_x,slb_y);
                    pos.z.val = get_floor_height_at(&pos);
                    create_effect(&pos, rmeffect, thing->owner);
                }
                delete_thing_structure(thing, 0);
                removed_num++;
            }
        }
        // Per thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
      }
    }
    return removed_num;
}

void place_single_slab_type_on_map(long a1, unsigned char a2, unsigned char a3, unsigned char a4)
{
    _DK_place_single_slab_type_on_map(a1, a2, a3, a4);
}

void shuffle_unattached_things_on_slab(long a1, long a2)
{
    _DK_shuffle_unattached_things_on_slab(a1, a2);
}

void place_slab_type_on_map(SlabType nslab, MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber owner, unsigned char a5)
{
    SlabType previous_slab_types_around[8];
    struct SlabMap *slb;
    struct SlabAttr *slbattr;
    MapSlabCoord slb_x,slb_y;
    MapSlabCoord spos_x,spos_y;
    int skind;
    long i;
    SYNCDBG(7,"Starting");
    if ((stl_x < 0) || (stl_x > map_subtiles_x))
        return;
    if ((stl_y < 0) || (stl_y > map_subtiles_y))
        return;
    slb_x = subtile_slab_fast(stl_x);
    slb_y = subtile_slab_fast(stl_y);
    if (slab_kind_is_animated(nslab))
    {
        ERRORLOG("Placing animating slab %d as standard slab",(int)nslab);
    }
    for (i = 0; i < 8; i++)
    {
        spos_x = slb_x + (MapSlabCoord)my_around_eight[i].delta_x;
        spos_y = slb_y + (MapSlabCoord)my_around_eight[i].delta_y;
        slb = get_slabmap_block(spos_x,spos_y);
        if (slabmap_block_invalid(slb))
        {
            previous_slab_types_around[i] = SlbT_ROCK;
            continue;
        }
        previous_slab_types_around[i] = slb->kind;
    }

    skind = alter_rock_style(nslab, slb_x, slb_y, owner);
    slb = get_slabmap_block(slb_x,slb_y);
    slb->kind = skind;

    set_whole_slab_owner(slb_x, slb_y, owner);
    place_single_slab_type_on_map(skind, slb_x, slb_y, owner);
    shuffle_unattached_things_on_slab(slb_x, slb_y);

    slbattr = get_slab_kind_attrs(skind);
    if ((slbattr->field_F == 4) || (slbattr->field_F == 2))
    {
      for (i = 0; i < 8; i++)
      {
          spos_x = slb_x + (MapSlabCoord)my_around_eight[i].delta_x;
          spos_y = slb_y + (MapSlabCoord)my_around_eight[i].delta_y;
          slb = get_slabmap_block(spos_x,spos_y);
          if (slabmap_block_invalid(slb))
              continue;
          if (slb->kind == SlbT_EARTH)
          {
              if (torch_flags_for_slab(spos_x, spos_y) == 0)
                  slb->kind = SlbT_EARTH;
              else
                  slb->kind = SlbT_TORCHDIRT;
          }
      }
    } else
    {
      for (i = 0; i < 8; i++)
      {
          spos_x = slb_x + (MapSlabCoord)my_around_eight[i].delta_x;
          spos_y = slb_y + (MapSlabCoord)my_around_eight[i].delta_y;
          slb = get_slabmap_block(spos_x,spos_y);
          if (slabmap_block_invalid(slb))
              continue;
          if (!slab_kind_is_animated(slb->kind))
          {
              slb->kind = alter_rock_style(slb->kind, spos_x, spos_y, owner);
          }
      }
    }

    pannel_map_update(slab_subtile(slb_x,0), slab_subtile(slb_y,0), slab_subtile(1,0), slab_subtile(1,0));

    for (i = 0; i < 8; i++)
    {
        spos_x = slb_x + (MapSlabCoord)my_around_eight[i].delta_x;
        spos_y = slb_y + (MapSlabCoord)my_around_eight[i].delta_y;
        slb = get_slabmap_block(spos_x,spos_y);
        if (slabmap_block_invalid(slb))
            continue;
        if ((previous_slab_types_around[i] != slb->kind)
          || ((slb->kind != SlbT_GOLD) && (slb->kind != SlbT_ROCK))
          || (game.game_kind == GKind_Unknown1))
        {
            slbattr = get_slab_kind_attrs(slb->kind);
            if (slbattr->field_F != 5)
                place_single_slab_type_on_map(slb->kind, spos_x, spos_y, slabmap_owner(slb));
        }
    }

    if (!a5)
      update_blocks_around_slab(slb_x,slb_y);
    switch (nslab)
    {
    case SlbT_EARTH:
    case SlbT_TORCHDIRT:
    case SlbT_ROCK:
    case SlbT_GOLD:
    case SlbT_GEMS:
    case SlbT_WALLDRAPE:
    case SlbT_WALLTORCH:
    case SlbT_WALLWTWINS:
    case SlbT_WALLWWOMAN:
    case SlbT_WALLPAIRSHR:
        delete_all_object_things_from_slab(slb_x, slb_y, 0);
        break;
    case SlbT_LAVA:
        delete_unwanted_things_from_liquid_slab(slb_x, slb_y, 17);
        break;
    case SlbT_WATER:
        delete_unwanted_things_from_liquid_slab(slb_x, slb_y, 21);
        break;
    }

}

void replace_map_slab_when_destroyed(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    SlabType nslab;
    struct SlabMap *slb;
    slb = get_slabmap_block(slb_x, slb_y);
    switch (slabmap_wlb(slb))
    {
    case 1:
        nslab = SlbT_LAVA;
        break;
    case 2:
        nslab = SlbT_WATER;
        break;
    default:
        nslab = SlbT_PATH;
        break;
    }
    place_slab_type_on_map(nslab, slab_subtile(slb_x,0), slab_subtile(slb_y,0), game.neutral_player_num, 0);
}

void create_gold_rubble_for_dug_slab(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct Column *col;
    MapSubtlCoord stl_x,stl_y;
    long x,y,z;
    stl_x = 3 * slb_x;
    stl_y = 3 * slb_y;
    col = get_column_at(stl_x, stl_y);
    if (column_invalid(col))
        z = 0;
    else
        z = (col->bitfileds >> 4) & 0x0F;
    for (y = stl_y; y < stl_y+3; y++)
    {
      for (x = stl_x; x < stl_x+3; x++)
      {
        if (z > 0)
          create_gold_rubble_for_dug_block(x, y, z, game.neutral_player_num);
      }
    }
}

void get_floor_and_ceiling_heights_at(const struct Coord3d *pos, unsigned long *heights)
{
    struct Column *col;
    struct Map *mapblk;
    long i;
    unsigned long height,k;
    heights[0] = 0;
    heights[1] = 15;
    mapblk = get_map_block_at(pos->x.stl.num, pos->y.stl.num);
    i = get_mapblk_column_index(mapblk);
    col = get_column(i);
    if (col->bitfileds & 0xF0)
        heights[0] = col->bitfileds >> 4;
    k = col->bitfileds & 0xE;
    if (k)
    {
        height = 8 - (k >> 1);
        if (height >= 15)
            height = 15;
        heights[1] = height;
    } else
    {
        height = get_mapblk_filled_subtiles(mapblk);
        if (height >= 15)
            height = 15;
        heights[1] = height;
    }
}

TbBool point_in_map_is_solid(const struct Coord3d *pos)
{
    struct Map *mapblk;
    struct Column *col;
    unsigned long heights[2];
    unsigned long check_h;
    col = get_column_at(pos->x.stl.num, pos->y.stl.num);
    check_h = pos->z.stl.num;
    if (col->bitfileds & 0xE)
    {
        get_floor_and_ceiling_heights_at(pos, heights);
    } else
    {
        mapblk = get_map_block_at(pos->x.stl.num, pos->y.stl.num);
        heights[0] = col->bitfileds >> 4;
        heights[1] = get_mapblk_filled_subtiles(mapblk);
    }
    if ((heights[1] <= check_h) || (heights[0] > check_h))
        return 1;
    return 0;
}

/**
 * Destroys a tall gold slab, replacing it with neutral ground.
 * @param stl_x Slab subtile digged out, X coordinate.
 * @param stl_y Slab subtile digged out, Y coordinate.
 * @param plyr_idx Index of the player who does the digging.
 */
void mine_out_block(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    MapSlabCoord slb_x,slb_y;
    //_DK_mine_out_block(a1, a2, plyr_idx);
    if (!subtile_has_slab(stl_x, stl_y))
    {
        ERRORLOG("Attempt to mine on invalid coordinates.");
        return;
    }
    slb_x = subtile_slab_fast(stl_x);
    slb_y = subtile_slab_fast(stl_y);
    create_gold_rubble_for_dug_slab(slb_x, slb_y);
    all_players_untag_blocks_for_digging_in_area(slb_x, slb_y);
    replace_map_slab_when_destroyed(slb_x, slb_y);
    do_slab_efficiency_alteration(slb_x, slb_y);
    // Gold slabs are normally visible to all players,
    // so sine we're destroying it - make it invisible
    // TODO MAP Maybe it should be cleared only if sibling non-gold slab are invisible
    set_slab_explored_flags(1 << plyr_idx, slb_x, slb_y);
}

unsigned char dig_has_revealed_area(long a1, long a2, unsigned char a3)
{
    return _DK_dig_has_revealed_area(a1, a2, a3);
}

void create_dirt_rubble_for_dug_slab(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct Column *col;
    MapSubtlCoord stl_x,stl_y;
    long x,y,z;
    stl_x = 3 * slb_x;
    stl_y = 3 * slb_y;
    col = get_column_at(stl_x, stl_y);
    if (column_invalid(col))
        z = 0;
    else
        z = (col->bitfileds >> 4) & 0x0F;
    for (y = stl_y; y < stl_y+3; y++)
    {
        for (x = stl_x; x < stl_x+3; x++)
        {
            if (z > 0)
                create_dirt_rubble_for_dug_block(x, y, z, game.neutral_player_num);
        }
    }
}

/**
 * Destroys a tall dirt or wall slab, replacing it with neutral ground.
 * @param stl_x Slab subtile digged out, X coordinate.
 * @param stl_y Slab subtile digged out, Y coordinate.
 * @param plyr_idx Index of the player who does the digging.
 */
void dig_out_block(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    MapSlabCoord slb_x,slb_y;
    //_DK_dig_out_block(stl_x, stl_y, plyr_idx);
    if (!subtile_has_slab(stl_x, stl_y))
    {
        ERRORLOG("Attempt to dig on invalid coordinates.");
        return;
    }
    slb_x = subtile_slab_fast(stl_x);
    slb_y = subtile_slab_fast(stl_y);
    create_dirt_rubble_for_dug_slab(slb_x, slb_y);
    all_players_untag_blocks_for_digging_in_area(slb_x, slb_y);
    replace_map_slab_when_destroyed(slb_x, slb_y);
    do_slab_efficiency_alteration(slb_x, slb_y);
    {
        struct Dungeon *dungeon;
        dungeon = get_dungeon(plyr_idx);
        if (!dungeon_invalid(dungeon)) {
            dungeon->lvstats.rock_dug_out++;
        }
    }
}

void check_map_explored(struct Thing *thing, long a2, long a3)
{
    _DK_check_map_explored(thing, a2, a3);
}

long ceiling_partially_recompute_heights(long sx, long sy, long ex, long ey)
{
    return _DK_ceiling_partially_recompute_heights(sx, sy, ex, ey);
}

long element_top_face_texture(struct Map *map)
{
  return _DK_element_top_face_texture(map);
}

/*
char point_in_map_is_solid_including_lava_check_ignoring_door(struct Coord3d *pos, struct Thing *thing)
{

}*/
/******************************************************************************/
#ifdef __cplusplus
}
#endif
