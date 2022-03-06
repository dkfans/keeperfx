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
#include "bflib_sound.h"
#include "bflib_memory.h"

#include "slab_data.h"
#include "room_data.h"
#include "map_utils.h"
#include "thing_effects.h"
#include "thing_objects.h"
#include "config_terrain.h"
#include "config_settings.h"
#include "config_creature.h"
#include "creature_senses.h"
#include "player_utils.h"
#include "ariadne_wallhug.h"
#include "spdigger_stack.h"
#include "frontmenu_ingame_map.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_set_slab_explored_flags(unsigned char flag, long tgslb_x, long tgslb_y);
DLLIMPORT long _DK_ceiling_partially_recompute_heights(long sx, long sy, long ex, long ey);
DLLIMPORT long _DK_element_top_face_texture(struct Map *map);
DLLIMPORT void _DK_shuffle_unattached_things_on_slab(long a1, long stl_x);
DLLIMPORT void _DK_delete_attached_things_on_slab(long slb_x, long slb_y);

const signed short slab_element_around_eight[] = {
    -3, -2, 1, 4, 3, 2, -1, -4
};

const signed short slab_primitive[] = {
    -1, 1, 0, 4, 3, -1, 7, -1, 2, 5, -1, -1, 6, -1, -1, 8
};

const signed short slab_element_to_corner[] = {
     6, -1, 0, -1, -1, -1, 4, -1, 2, 0, 0, 0
};

const unsigned char special_cases[][9] = {
    {1, 1, 3, 1, 1, 3, 1, 1, 3},
    {4, 4, 7, 4, 8, 7, 4, 8, 7},
    {2, 2, 2, 2, 2, 2, 0, 0, 0},
    {5, 5, 5, 8, 8, 5, 4, 4, 4},
    {5, 8, 6, 5, 8, 6, 5, 5, 6},
    {6, 6, 6, 6, 8, 8, 7, 7, 7},
    {0, 0, 0, 3, 3, 1, 2, 2, 2},
};

const unsigned short cube_bits[][6] = {
    {  0,   0,   0,   0,   0,   0},
    {160, 410, 413, 416, 419,  77},
    {161, 411, 414, 417, 420,  77},
    {162, 412, 415, 418, 421,  77},
    {382, 422, 423, 424, 426, 425},
    {393, 427, 428, 429, 431, 430},
    { 67,  68,  69,  70,  71,   4},
    {192, 193, 194, 195, 199, 198},
};

const unsigned char player_cube_group[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 16
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 32
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 48
    0, 0, 0, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, // 64
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 80
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 96
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 112
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 128
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 144
    1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 160
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 176
    7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, // 192
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 208
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 224
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 240
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 256
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 272
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 288
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 304
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 320
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 336
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 352
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, // 368
    0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, // 384
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 400
    0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, // 416
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 432
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 448
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 464
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 480
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 496
};

const unsigned char  *against_to_case[] = {
    NULL,            NULL,            NULL,            NULL,
    NULL,special_cases[0],            NULL,special_cases[1],
    NULL,            NULL,special_cases[2],special_cases[3],
    NULL,special_cases[4],special_cases[5],            NULL,
};

/******************************************************************************/
TbBool block_has_diggable_side(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
  long i;
  for (i = 0; i < SMALL_AROUND_SLAB_LENGTH; i++)
  {
    if (slab_is_safe_land(plyr_idx, slb_x + small_around[i].delta_x, slb_y + small_around[i].delta_y))
      return true;
  }
  return false;
}

int block_count_diggable_sides(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    int num_sides;
    num_sides = 0;
    long i;
    for (i = 0; i < SMALL_AROUND_SLAB_LENGTH; i++)
    {
        if (slab_is_safe_land(plyr_idx, slb_x + small_around[i].delta_x, slb_y + small_around[i].delta_y)) {
            num_sides++;
        }
    }
    return num_sides;
}

/**
 * Creates gold rubble effects when a gold block is destroyed.
 * @param stl_x
 * @param stl_y
 * @param stl_height
 * @param owner
 * @see create_dirt_rubble_for_dug_block()
 */
void create_gold_rubble_for_dug_block(MapSubtlCoord stl_x, MapSubtlCoord stl_y, MapSubtlCoord stl_height, PlayerNumber owner)
{
    struct Coord3d pos;
    MapCoord maxpos_z;
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = subtile_coord_center(1);
    maxpos_z = subtile_coord(stl_height,0);
    while (pos.z.val < maxpos_z)
    {
        create_effect(&pos, TngEff_DirtRubble, owner);
        create_effect(&pos, TngEff_GoldRubble2, owner);
        pos.z.val += COORD_PER_STL;
    }
}

/**
 * Creates dirt rubble effects when a gold block is destroyed.
 * Originally was create_rubble_for_dug_block().
 * @param stl_x
 * @param stl_y
 * @param stl_height
 * @param owner
 */
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
        create_effect(&pos, TngEff_DirtRubble, owner);
        pos.z.val += COORD_PER_STL;
    }
}

TbBool tag_blocks_for_digging_in_area(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    MapSubtlCoord x;
    MapSubtlCoord y;
    x = STL_PER_SLB * (stl_x/STL_PER_SLB);
    y = STL_PER_SLB * (stl_y/STL_PER_SLB);
    if ( (x < 0) || (x >= map_subtiles_x) || (y < 0) || (y >= map_subtiles_y) ) {
        ERRORLOG("Attempt to tag area outside of map");
        return 0;
    }
    //return _DK_tag_blocks_for_digging_in_area(stl_x, stl_y, plyr_idx);
    TbBool task_added;
    task_added = false;
    struct Map *mapblk;
    mapblk = get_map_block_at(x+1, y+1);
    struct SlabMap *slb;
    slb = get_slabmap_for_subtile(x+1, y+1);
    struct SlabAttr *slbattr;
    slbattr = get_slab_attrs(slb);
    long i;
    i = get_subtile_number(x+1,y+1);
    if ((find_from_task_list(plyr_idx, i) == -1)
      && (slbattr->is_diggable || !map_block_revealed(mapblk, plyr_idx))
      && (((mapblk->flags & SlbAtFlg_IsRoom) == 0) || slabmap_owner(slb) != plyr_idx)
      && (((mapblk->flags & SlbAtFlg_Filled) == 0) || slabmap_owner(slb) == plyr_idx || !map_block_revealed(mapblk, plyr_idx)) )
    {
      if ((mapblk->flags & SlbAtFlg_Valuable) != 0)
      {
          add_task_list_entry(plyr_idx, SDDigTask_MineGold, i);
          task_added = true;
      } else
      if (((mapblk->flags & SlbAtFlg_Digable) == 0) && (((mapblk->flags & SlbAtFlg_Filled) == 0) || (slabmap_owner(slb) != plyr_idx)))
      {
          add_task_list_entry(plyr_idx, SDDigTask_MineGems, i);
          task_added = true;
      } else
      {
          add_task_list_entry(plyr_idx, SDDigTask_DigEarth, i);
          task_added = true;
      }
      if (is_my_player_number(plyr_idx))
      {
          long dx;
          long dy;
          for (dy=0; dy < STL_PER_SLB; dy++)
          {
              for (dx=0; dx < STL_PER_SLB; dx++)
              {
                  mapblk = get_map_block_at(x+dx, y+dy);
                  slb = get_slabmap_for_subtile(x+dx, y+dy);
                  if ((mapblk->flags & (SlbAtFlg_TaggedValuable|SlbAtFlg_Unexplored)) != 0)
                      continue;
                  if (((mapblk->flags & SlbAtFlg_IsRoom) != 0) && (slabmap_owner(slb) == plyr_idx))
                      continue;
                  if (((mapblk->flags & SlbAtFlg_Filled) != 0) && (slabmap_owner(slb) != plyr_idx) && map_block_revealed(mapblk, plyr_idx))
                      continue;
                  if ((mapblk->flags & SlbAtFlg_Valuable) != 0)
                  {
                      mapblk->flags |= SlbAtFlg_TaggedValuable;
                      if (!map_block_revealed(mapblk, plyr_idx))
                          mapblk->flags |= SlbAtFlg_Unexplored;
                  } else
                  if (((mapblk->flags & (SlbAtFlg_Filled|SlbAtFlg_Digable)) != 0) || !map_block_revealed(mapblk, plyr_idx))
                  {
                      mapblk->flags |= SlbAtFlg_Unexplored;
                  }
              }
          }
      }
      pannel_map_update(x, y, STL_PER_SLB, STL_PER_SLB);
    }
    return task_added;
}

long untag_blocks_for_digging_in_area(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    MapSubtlCoord x;
    MapSubtlCoord y;
    long num_untagged;
    long task_idx;
    long i;
    x = STL_PER_SLB * (stl_x/STL_PER_SLB);
    y = STL_PER_SLB * (stl_y/STL_PER_SLB);
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
        long dx;
        long dy;
        for (dy=0; dy < STL_PER_SLB; dy++)
        {
            for (dx=0; dx < STL_PER_SLB; dx++)
            {
                struct Map *mapblk;
                mapblk = get_map_block_at(x+dx, y+dy);
                if (map_block_invalid(mapblk))
                    continue;
                if ( mapblk->flags & (SlbAtFlg_TaggedValuable|SlbAtFlg_Unexplored) )
                  num_untagged++;
                mapblk->flags &= ~SlbAtFlg_TaggedValuable;
                mapblk->flags &= ~SlbAtFlg_Unexplored;
            }
        }
    }
    pannel_map_update(x, y, STL_PER_SLB, STL_PER_SLB);
    return num_untagged;
}

long tag_blocks_for_digging_in_rectangle_around(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    long ret;
    ret = tag_blocks_for_digging_in_area(stl_x & ((stl_x < 0) - 1), stl_y & ((stl_y < 0) - 1), plyr_idx);
    if ((ret != 0) && is_my_player_number(plyr_idx))
        play_non_3d_sample(118);
    return ret;
}

void untag_blocks_for_digging_in_rectangle_around(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    long ret;
    ret = untag_blocks_for_digging_in_area(stl_x & ((stl_x < 0) - 1), stl_y & ((stl_y < 0) - 1), plyr_idx);
    if ((ret != 0) && is_my_player_number(plyr_idx))
        play_non_3d_sample(118);
}

void all_players_untag_blocks_for_digging_in_area(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct Map *mapblk;
    PlayerNumber plyr_idx;
    mapblk = get_map_block_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    for (plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        if ((plyr_idx == game.hero_player_num) || (plyr_idx == game.neutral_player_num))
            continue;
        struct PlayerInfo *player;
        player = get_player(plyr_idx);
        if (player_exists(player))
        {
            if (map_block_revealed(mapblk, plyr_idx))
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
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    SYNCDBG(16,"Starting");
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
    struct SlabMap* sslb1;
    struct SlabMap* sslb2;
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

/**
 * Deletes almost all of object things from a slab.
 * Leaves only those which are never bound to a slab.
 * @param slb_x
 * @param slb_y
 * @param rmeffect
 */
long delete_all_object_things_from_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, long rmeffect)
{
    SubtlCodedCoords stl_num;
    long removed_num;
    long n;
    stl_num = get_subtile_number(slab_subtile_center(slb_x),slab_subtile_center(slb_y));
    removed_num = 0;
    for (n=0; n < AROUND_MAP_LENGTH; n++)
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
          if ((thing->class_id == TCls_Object) && !object_is_unaffected_by_terrain_changes(thing))
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
              break_mapwho_infinite_chain(mapblk);
              break;
          }
        }
    }
    return removed_num;
}

long delete_unwanted_things_from_liquid_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, long rmeffect)
{
    SubtlCodedCoords stl_num;
    struct Thing *thing;
    struct Map *mapblk;
    struct Objects *objdat;
    struct Coord3d pos;
    long removed_num;
    unsigned long k;
    long i;
    long n;
    stl_num = get_subtile_number_at_slab_center(slb_x, slb_y);
    removed_num = 0;
    for (n=0; n < AROUND_MAP_LENGTH; n++)
    {
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
                objdat = get_objects_data_for_thing(thing);
                if (objdat->destroy_on_liquid)
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
                break_mapwho_infinite_chain(mapblk);
                break;
            }
        }
    }
    return removed_num;
}

void delete_attached_things_on_slab(long slb_x, long slb_y)
{
    _DK_delete_attached_things_on_slab(slb_x, slb_y); return;
}

unsigned char get_against(unsigned char agnst_plyr_idx, long agnst_slbkind, long slb_x, long slb_y)
{
    //return _DK_get_against(a1, a2, slb_x, slb_y);
    struct SlabMap *slb;
    slb = get_slabmap_block(slb_x, slb_y);
    if (slabmap_block_invalid(slb)) {
        return 1;
    }
    struct SlabAttr *slbattr;
    slbattr = get_slab_attrs(slb);
    struct SlabAttr *agnst_slbattr;
    agnst_slbattr = get_slab_kind_attrs(agnst_slbkind);
    return (slbattr->slb_id != agnst_slbattr->slb_id)
            || ((slabmap_owner(slb) != agnst_plyr_idx) && (slabmap_owner(slb)!= game.neutral_player_num));
}

void delete_column(long col_idx)
{
    game.field_14AB3F--;
    struct Column *col;
    col = &game.columns_data[col_idx];
    memcpy(col, &game.columns_data[0], sizeof(struct Column));
    col->use = 0;
}

void remove_block_from_map_element(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map *mapblk;
    mapblk = get_map_block_at(stl_x, stl_y);
    int col_idx;
    col_idx = get_mapblk_column_index(mapblk);
    struct Column *col;
    col = get_column(col_idx);
    col->use--;
    if (((col->bitfields & 0x01) == 0) && (col->use <= 0)) {
        delete_column(col_idx);
    }
    set_mapblk_column_index(mapblk, 0);
}

void place_column_on_map_element(struct Column *ncol, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    //void place_column_on_map_element(struct Column *col, unsigned short a2, unsigned short a3)
    remove_block_from_map_element(stl_x, stl_y);
    long col_idx;
    col_idx = find_column(ncol);
    if (col_idx <= 0)
    {
        col_idx = create_column(ncol);
        if (col_idx <= 0) {
          ERRORLOG("Cannot allocate column");
        }
    }
    {
        struct Map *mapblk;
        mapblk = get_map_block_at(stl_x, stl_y);
        set_mapblk_column_index(mapblk, col_idx);
        struct Column *col;
        col = get_column(col_idx);
        col->use++;
    }
}

void copy_block_with_cube_groups(short itm_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    //_DK_copy_block_with_cube_groups(a1, a2, a3);
    if (itm_idx >= 0) {
      ERRORLOG("We should only be dealing with columns now");
      return;
    }
    struct Column col;
    memcpy(&col, &game.columns_data[-itm_idx], sizeof(struct Column));
    col.use = 0;
    col.bitfields &= ~0x01;
    TbBool found;
    found = false;
    int i;
    for (i=0; i < COLUMN_STACK_HEIGHT; i++)
    {
        unsigned short *cube;
        cube = &col.cubes[i];
        if (*cube > 0)
        {
            unsigned char cube_grp;
            cube_grp = player_cube_group[*cube];
            if (cube_grp > 0) {
                found = true;
                struct SlabMap *slb;
                slb = get_slabmap_for_subtile(stl_x, stl_y);
                *cube = cube_bits[cube_grp][slabmap_owner(slb)];
            }
        }
    }
    if (found) {
        place_column_on_map_element(&col, stl_x, stl_y);
    } else
    {
        struct Map *mapblk;
        mapblk = get_map_block_at(stl_x, stl_y);
        set_mapblk_column_index(mapblk, -itm_idx);
    }
}

void set_alt_bit_based_on_slab(SlabKind slbkind, unsigned char stl_x, unsigned char stl_y)
{
    struct SlabAttr *slbattr;
    slbattr = get_slab_kind_attrs(slbkind);

    unsigned short sibling_flags;
    unsigned short edge_flags;
    unsigned long wibble;

    sibling_flags = 0;
    edge_flags = 0;
    wibble = slbattr->wibble;
    if (slab_kind_is_liquid(slbkind))
    {
        if ((stl_x % 3) == 0)
            edge_flags = 0x01;
        if ((stl_y % 3) == 0)
            edge_flags |= 0x02;
        MapSlabCoord slb_x;
        MapSlabCoord slb_y;
        slb_x = subtile_slab(stl_x);
        slb_y = subtile_slab(stl_y);
        struct SlabMap *slb;
        slb = get_slabmap_block(slb_x-1, slb_y);
        if (slab_kind_is_liquid(slb->kind))
            sibling_flags |= 0x01;
        slb = get_slabmap_block(slb_x, slb_y-1);
        if (slab_kind_is_liquid(slb->kind))
            sibling_flags |= 0x02;
        slb = get_slabmap_block(slb_x-1, slb_y-1);
        if (slab_kind_is_liquid(slb->kind))
            sibling_flags |= 0x04;

        if (edge_flags == 3)
        {
          if (sibling_flags == (0x01|0x02|0x04))
              wibble = 2;
        } else
        if (edge_flags == 1)
        {
          if (sibling_flags & 0x01)
              wibble = 2;
        } else
        if ((edge_flags != 2) || (sibling_flags & 0x02))
        {
            wibble = 2;
        }
    }
    struct Map *mapblk;
    mapblk = get_map_block_at(stl_x, stl_y);
    set_mapblk_wibble_value(mapblk, wibble);
}

void place_slab_columns(long slbkind, unsigned char stl_x, unsigned char stl_y, const ColumnIndex *col_idx)
{
    //_DK_place_slab_columns(slbkind, a2, a3, a4); return;
    struct SlabAttr *slbattr;
    slbattr = get_slab_kind_attrs(slbkind);
    if (slbattr->wlb_type != 3)
    {
        struct SlabMap *slb;
        slb = get_slabmap_for_subtile(stl_x, stl_y);
        slabmap_set_wlb(slb, slbattr->wlb_type);
    }
    int dx;
    int dy;

    const ColumnIndex *colid;
    colid = col_idx;
    for (dy=0; dy < STL_PER_SLB; dy++)
    {
        for (dx=0; dx  < STL_PER_SLB; dx++)
        {
            copy_block_with_cube_groups(*colid, stl_x+dx, stl_y+dy);
            int v10;
            v10 = -*colid;
            if ( v10 < 0 )
              ERRORLOG("BBlocks instead of columns");
            update_map_collide(slbkind, stl_x+dx, stl_y+dy);
            if (wibble_enabled() || (liquid_wibble_enabled() && slab_kind_is_liquid(slbkind)))
            {
                set_alt_bit_based_on_slab(slbkind, stl_x+dx, stl_y+dy);
            }
            colid++;
        }
    }
}

#define get_slabset_index(slbkind, style, pick) get_slabset_index_f(slbkind, style, pick, __func__)
unsigned short get_slabset_index_f(SlabKind slbkind, unsigned char style, unsigned char pick, const char *func_name)
{
    if (slbkind >= SLABSET_COUNT/(9*3+1)) {
        ERRORLOG("%s: Illegal animating slab kind: %d", func_name, (int)slbkind);
        slbkind = 0;
    }
    if (style > 3) {
        ERRORLOG("%s: Illegal animating slab style: %d", func_name, (int)style);
        style = 0;
    }
    if ((pick >= 9) || ((style == (SlbFillStl_Water+1)) && (pick >= 1)))
    {
        if (slab_kind_is_room_wall(slbkind) && (pick < 9))
        {
            style = SlbFillStl_Water;
            slbkind = SlbT_DAMAGEDWALL; // There's no columns for room walls next to water, so we're using a regular wall instead.
        }
        else
        {
            ERRORLOG("%s: Illegal animating slab pick: %d", func_name, (int)pick);
            pick = 0;
        }
    }
    return 28 * slbkind + 9 * style + pick;
}

void place_slab_object(unsigned short a1, long a2, long a3, unsigned short slabct_num, unsigned short slbelem, unsigned char a6)
{
    //_DK_place_slab_object(a1, a2, a3, a4, a5, a6); return;
    if (slabct_num >= SLABSET_COUNT) {
        ERRORLOG("Illegal animating slab number: %d", (int)slabct_num);
        return;
    }
    short sobj_idx;
    sobj_idx = game.slabobjs_idx[slabct_num];
    if (sobj_idx < 0) {
        return;
    }
    for (; sobj_idx < game.slabobjs_num; sobj_idx++)
    {
        struct SlabObj *sobj;
        sobj = &game.slabobjs[sobj_idx];
        if (sobj->field_1 != slabct_num) {
            break;
        }
        if (sobj->field_3 != slbelem) {
            continue;
        }
        struct Coord3d pos;
        pos.x.val = (a2 << 8) + sobj->field_4;
        pos.y.val = (a3 << 8) + sobj->field_6;
        pos.z.val = sobj->field_8;
        struct Map *mapblk;
        mapblk = get_map_block_at(coord_subtile(pos.x.val), coord_subtile(pos.y.val));
        if ((mapblk->flags & SlbAtFlg_Blocking) == 0)
        {
            if (sobj->field_0 == 1)
            {
                struct InitLight ilght;
                LbMemorySet(&ilght,0,sizeof(struct InitLight));
                ilght.mappos.x.val = pos.x.val;
                ilght.mappos.y.val = pos.y.val;
                ilght.mappos.z.val = pos.z.val;
                ilght.radius = sobj->sofield_C << 8;
                ilght.intensity = sobj->sofield_B;
                ilght.field_3 = 0;
                ilght.is_dynamic = 0;
                long lgt_id;
                lgt_id = light_create_light(&ilght);
                if (lgt_id != 0) {
                    struct Light *lgt;
                    lgt = &game.lish.lights[lgt_id];
                    lgt->field_12 = a1;
                } else {
                    WARNLOG("Cannot allocate light");
                    continue;
                }
            } else if (sobj->field_0 == 0)
            {
                if (sobj->field_A == TCls_Object)
                {
                    ThingModel tngmodel;
                    tngmodel = sobj->sofield_B;
                    if (tngmodel == dungeon_flame_objects[0]) {
                        tngmodel = dungeon_flame_objects[a6];
                    } else
                    if (tngmodel == player_guardflag_objects[0]) {
                        tngmodel = player_guardflag_objects[a6];
                    }
                    if (tngmodel <= 0)
                        continue;

                    TbBool needs_object;
                    int icorn;
                    int nfilled;
                    int nprison;

                    if ((tngmodel == 27) && (slbelem != 4))
                    {
                        MapSlabCoord slb_x;
                        MapSlabCoord slb_y;
                        slb_x = subtile_slab(a2);
                        slb_y = subtile_slab(a3);
                        nprison = 0;
                        nfilled = 0;
                        if ((slbelem & 1) != 0)
                        {
                            const struct SlabMap *slb;
                            slb = get_slabmap_block(slb_x + my_around_nine[slbelem].delta_x, slb_y + my_around_nine[slbelem].delta_y);
                            const struct SlabAttr *slbattr;
                            slbattr = get_slab_attrs(slb);
                            needs_object = ((slbattr->block_flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) == 0);
                        } else
                        {
                          icorn = slab_element_to_corner[slbelem];
                          if (icorn != -1)
                          {
                              struct SlabMap *slb;
                              slb = get_slabmap_block(slb_x + my_around_eight[icorn].delta_x, slb_y + my_around_eight[icorn].delta_y);
                              const struct SlabAttr *slbattr;
                              slbattr = get_slab_attrs(slb);
                              if ((slbattr->block_flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0)
                                  nfilled++;
                              if (slb->kind == SlbT_PRISON)
                                  nprison++;
                              slb = get_slabmap_block(slb_x + my_around_eight[(icorn + 2) & 7].delta_x, slb_y + my_around_eight[(icorn + 2) & 7].delta_y);
                              slbattr = get_slab_attrs(slb);
                              if ((slbattr->block_flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0)
                                  nfilled++;
                              if (slb->kind == SlbT_PRISON)
                                  nprison++;
                          }
                          needs_object = (nprison + nfilled < 2);
                        }
                        if ( !needs_object )
                            continue;
                      }
                      struct Thing *objtng;
                      objtng = create_object(&pos, tngmodel, a6, a1);
                      if (thing_is_invalid(objtng)) {
                          ERRORLOG("Cannot create object type %d", tngmodel);
                          continue;
                    }
                } else
                if (sobj->field_A == TCls_EffectGen)
                {
                    struct Thing *effgentng;
                    effgentng = create_effect_generator(&pos, sobj->sofield_B, sobj->sofield_C << 8, a6, a1);
                    if (thing_is_invalid(effgentng)) {
                        ERRORLOG("Cannot create effect generator, type %d", sobj->sofield_B);
                        continue;
                    }
                } else
                {
                    ERRORLOG("Stupid thing class %d", (int)sobj->field_A);
                    continue;
                }
            }
        }
    }
}

void place_slab_objects(MapSlabCoord slb_x, MapSlabCoord slb_y, const short * slab_number_list, PlayerNumber plyr_idx)
{
    SlabCodedCoords place_slbnum;
    place_slbnum = get_slab_number(slb_x, slb_y);
    int i;
    i = 0;
    MapSubtlDelta dx;
    MapSubtlDelta dy;
    for (dy=0; dy < STL_PER_SLB; dy++)
    {
        MapSubtlCoord sstl_x;
        MapSubtlCoord sstl_y;
        sstl_y = slab_subtile(slb_y,dy);
        for (dx=0; dx < STL_PER_SLB; dx++)
        {
            sstl_x = slab_subtile(slb_x,dx);
            place_slab_object(place_slbnum, sstl_x, sstl_y, slab_number_list[i], i, plyr_idx);
            i++;
        }
    }
}

void place_single_slab_fill_arrays_std(MapSlabCoord slb_x, MapSlabCoord slb_y, short *slab_type_list, short *room_pretty_list)
{
    int i;
    for (i = 0; i < AROUND_EIGHT_LENGTH; i+=2)
    {
        MapSlabCoord sslb_x;
        MapSlabCoord sslb_y;
        sslb_x = slb_x + (MapSlabCoord)my_around_eight[i].delta_x;
        sslb_y = slb_y + (MapSlabCoord)my_around_eight[i].delta_y;
        struct SlabMap *slb;
        slb = get_slabmap_block(sslb_x,sslb_y);
        if (!slabmap_block_invalid(slb))
        {
            if ((slb->kind != SlbT_GUARDPOST) && (slb->kind != SlbT_BRIDGE))
            {
                struct SlabAttr *slbattr;
                slbattr = get_slab_attrs(slb);
                MapSlabCoord sibslb_x;
                MapSlabCoord sibslb_y;
                struct SlabMap *sibslb1;
                sibslb_x = slb_x + (MapSlabCoord)my_around_eight[(i-1)&7].delta_x;
                sibslb_y = slb_y + (MapSlabCoord)my_around_eight[(i-1)&7].delta_y;
                sibslb1 = get_slabmap_block(sibslb_x,sibslb_y);
                struct SlabMap *sibslb2;
                sibslb_x = slb_x + (MapSlabCoord)my_around_eight[(i+1)&7].delta_x;
                sibslb_y = slb_y + (MapSlabCoord)my_around_eight[(i+1)&7].delta_y;
                sibslb2 = get_slabmap_block(sibslb_x,sibslb_y);
                short pretty_val;
                pretty_val = 0;
                if ((sibslb1->kind == slb->kind) && (sibslb2->kind == slb->kind))
                    pretty_val = 1;
                if (slbattr->category == SlbAtCtg_RoomInterior)
                {
                    int n;
                    for (n = -1; n <= 1; n++)
                    {
                        int neigh;
                        neigh = 4 + slab_element_around_eight[(i+n)&7];
                        slab_type_list[neigh] = slb->kind + 1;
                        room_pretty_list[neigh] = pretty_val;
                    }
                }
            }
        }
    }
}

void delete_attached_lights_on_slab(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    SlabCodedCoords place_slbnum;
    place_slbnum = get_slab_number(slb_x, slb_y);
    MapSubtlCoord start_stl_x;
    MapSubtlCoord start_stl_y;
    MapSubtlCoord end_stl_x;
    MapSubtlCoord end_stl_y;
    start_stl_x = 3 * slb_x - 1;
    if (start_stl_x <= 0)
      start_stl_x = 0;
    start_stl_y = 3 * slb_y - 1;
    if (start_stl_y <= 0)
      start_stl_y = 0;
    end_stl_x = 3 * slb_x + 4;
    if (end_stl_x >= 255)
      end_stl_x = 255;
    end_stl_y = 3 * slb_y + 4;
    if (end_stl_y >= 255)
      end_stl_y = 255;
    {
        long i;
        unsigned long k;
        i = game.thing_lists[TngList_StaticLights].index;
        k = 0;
        while (i > 0)
        {
            struct Light *lgt;
            lgt = &game.lish.lights[i];
            i = lgt->field_26;
            // Per-light code
            int lgtstl_x;
            int lgtstl_y;
            lgtstl_x = lgt->mappos.x.stl.num;
            lgtstl_y = lgt->mappos.y.stl.num;
            if (lgt->field_12 == place_slbnum)
            {
                if ((lgtstl_x >= start_stl_x) && (lgtstl_x <= end_stl_x) && (lgtstl_y >= start_stl_y) && (lgtstl_y <= end_stl_y))
                {
                    light_delete_light(lgt->index);
                }
            }
            // Per-light code ends
            k++;
            if (k > LIGHTS_COUNT)
            {
                ERRORLOG("Infinite loop detected when sweeping lights list");
                break;
            }
        }
    }
}

void place_single_slab_fill_style_array(MapSlabCoord slb_x, MapSlabCoord slb_y, short *style_set)
{
    int i;
    for (i=0; i < AROUND_EIGHT_LENGTH; i+=2)
    {
        MapSlabCoord sslb_x;
        MapSlabCoord sslb_y;
        sslb_x = slb_x + (MapSlabCoord)my_around_eight[i].delta_x;
        sslb_y = slb_y + (MapSlabCoord)my_around_eight[i].delta_y;
        int style_val;
        struct SlabMap *slb;
        slb = get_slabmap_block(sslb_x,sslb_y);
        if (!slabmap_block_invalid(slb)) {
            struct SlabAttr *slbattr;
            slbattr = get_slab_attrs(slb);
            style_val = slbattr->fill_style;
        } else {
            style_val = 0;
        }
        int n;
        for (n = -1; n <= 1; n++)
        {
            int neigh;
            neigh = 4 + slab_element_around_eight[(i+n)&7];
            if (style_set[neigh] < style_val)
              style_set[neigh] = style_val;
        }
    }
}

void place_single_slab_set_torch_places(SlabKind slbkind, MapSlabCoord slb_x, MapSlabCoord slb_y, short *slab_type_list)
{
    SlabKind undecorated_slbkind;
    unsigned short torch_flags;
    if (slbkind == SlbT_TORCHDIRT) {
        undecorated_slbkind = SlbT_EARTH;
    } else {
        undecorated_slbkind = slbkind + 4;
    }
    torch_flags = torch_flags_for_slab(slb_x, slb_y);
    if ((torch_flags & 0x01) != 0)
    {
        slab_type_list[3] = undecorated_slbkind;
        slab_type_list[5] = undecorated_slbkind;
        if ((slb_y + slb_x) & 1)
          slab_type_list[1] = undecorated_slbkind;
        else
          slab_type_list[7] = undecorated_slbkind;
    } else
    if ((torch_flags & 0x02) != 0)
    {
        slab_type_list[1] = undecorated_slbkind;
        slab_type_list[7] = undecorated_slbkind;
        if ((slb_y + slb_x) & 1)
          slab_type_list[3] = undecorated_slbkind;
        else
          slab_type_list[5] = undecorated_slbkind;
    }
}

void place_single_slab_prepare_column_index(SlabKind slbkind, MapSlabCoord slb_x, MapSlabCoord slb_y,
    PlayerNumber plyr_idx, short *slab_type_list, short *room_pretty_list, short *style_set, short *slab_number_list, ColumnIndex *col_idx)
{
    struct SlabAttr *place_slbattr;
    place_slbattr = get_slab_kind_attrs(slbkind);
    unsigned char against;
    signed short primitiv;
    against = 0;
    int i;
    for (i=0; i < AROUND_EIGHT_LENGTH; i+=2)
    {
        MapSlabCoord sslb_x;
        MapSlabCoord sslb_y;
        sslb_x = slb_x + (MapSlabCoord)my_around_eight[i].delta_x;
        sslb_y = slb_y + (MapSlabCoord)my_around_eight[i].delta_y;
        against = get_against(plyr_idx, slbkind, sslb_x, sslb_y) | 2 * against;
    }
    i = 0;
    int slabct_num;
    if ( against )
    {
        primitiv = slab_primitive[against];
        if (primitiv == -1)
        {
            const unsigned char *specase;
            specase = against_to_case[against];
            if (specase != NULL)
            {
                for (i=0; i < 9; i++)
                {
                    slabct_num = get_slabset_index( slab_type_list[i], style_set[i] + room_pretty_list[i], specase[i]);
                    slab_number_list[i] = slabct_num;
                    struct SlabSet *sset;
                    sset = &game.slabset[slabct_num];
                    col_idx[i] = sset->col_idx[i];
                }
            }
            else
            {
              ERRORLOG("Illegal special case!");
            }
        } else
        {
            for (i=0; i < 9; i++)
            {
                slabct_num = get_slabset_index( slab_type_list[i], style_set[i] + room_pretty_list[i], primitiv);
                slab_number_list[i] = slabct_num;
                struct SlabSet *sset;
                sset = &game.slabset[slabct_num];
                col_idx[i] = sset->col_idx[i];
            }
        }
    } else
    if (place_slbattr->category == SlbAtCtg_RoomInterior)
    {
        for (i=1; i < 8; i+=2)
        {
            MapSlabCoord sslb_x;
            MapSlabCoord sslb_y;
            sslb_x = slb_x + (MapSlabCoord)my_around_eight[i].delta_x;
            sslb_y = slb_y + (MapSlabCoord)my_around_eight[i].delta_y;
            against = get_against(plyr_idx, slbkind, sslb_x, sslb_y) | 2 * against;
        }
        if ( against )
        {
            const unsigned char *specase;
            specase = special_cases[6];
            for (i=0; i < 9; i++)
            {
                slabct_num = get_slabset_index( slab_type_list[i], style_set[i], specase[i]);
                slab_number_list[i] = slabct_num;
                struct SlabSet *sset;
                sset = &game.slabset[slabct_num];
                col_idx[i] = sset->col_idx[i];
            }
        } else
        {
            for (i=0; i < 9; i++)
            {
                slabct_num = get_slabset_index( slab_type_list[i], 3, 0);
                slab_number_list[i] = slabct_num;
                struct SlabSet *sset;
                sset = &game.slabset[slabct_num];
                col_idx[i] = sset->col_idx[i];
            }
        }
    } else
    {
        for (i=0; i < 9; i++)
        {
            slabct_num = get_slabset_index( slab_type_list[i], 3, 0);
            slab_number_list[i] = slabct_num;
            struct SlabSet *sset;
            sset = &game.slabset[slabct_num];
            col_idx[i] = sset->col_idx[i];
        }
    }
}

void place_single_slab_modify_column_near_liquid(SlabKind slbkind, MapSlabCoord slb_x, MapSlabCoord slb_y,
    PlayerNumber plyr_idx, short *slab_type_list, short *room_pretty_list, short *style_set, short *slab_number_list, ColumnIndex *col_idx)
{
    int neigh;
    int slabct_num;
    int i;
    for (i=0; i < AROUND_EIGHT_LENGTH; i+=2)
    {
        MapSlabCoord sslb_x;
        MapSlabCoord sslb_y;
        sslb_x = slb_x + (MapSlabCoord)my_around_eight[(i-1)&7].delta_x;
        sslb_y = slb_y + (MapSlabCoord)my_around_eight[(i-1)&7].delta_y;
        struct SlabMap *slb;
        slb = get_slabmap_block(sslb_x,sslb_y);
        struct SlabAttr *slbattra;
        slbattra = get_slab_attrs(slb);
        if ((slbattra->category == SlbAtCtg_FortifiedGround) || (slbattra->category == SlbAtCtg_RoomInterior) ||
            (slbattra->category == SlbAtCtg_Obstacle) || (slb->kind == SlbT_WATER) || (slb->kind == SlbT_LAVA))
        {
            sslb_x = slb_x + (MapSlabCoord)my_around_eight[(i-2)&7].delta_x;
            sslb_y = slb_y + (MapSlabCoord)my_around_eight[(i-2)&7].delta_y;
            slb = get_slabmap_block(sslb_x,sslb_y);
            struct SlabAttr *slbattrb;
            slbattrb = get_slab_attrs(slb);
            if (slbattrb->category == SlbAtCtg_FortifiedWall)
            {
                sslb_x = slb_x + (MapSlabCoord)my_around_eight[(i)&7].delta_x;
                sslb_y = slb_y + (MapSlabCoord)my_around_eight[(i)&7].delta_y;
                slb = get_slabmap_block(sslb_x,sslb_y);
                struct SlabAttr *slbattrc;
                slbattrc = get_slab_attrs(slb);
                if (slbattrc->category == SlbAtCtg_FortifiedWall)
                {
                  neigh = 4 + slab_element_around_eight[(i-1)&7];
                  slab_type_list[neigh] = SlbT_WALLWTWINS;
                  slabct_num = get_slabset_index(slab_type_list[neigh], style_set[neigh], 8);
                  slab_number_list[neigh] = slabct_num;
                  struct SlabSet *sset;
                  sset = &game.slabset[slabct_num];
                  col_idx[neigh] = sset->col_idx[neigh];
                }
            }
        }
    }
}

void place_single_slab_type_on_map(SlabKind slbkind, MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx)
{
    int i;
    short style_set[STL_PER_SLB*STL_PER_SLB];
    short slab_number_list[STL_PER_SLB*STL_PER_SLB];
    short room_pretty_list[STL_PER_SLB*STL_PER_SLB];
    short slab_type_list[STL_PER_SLB*STL_PER_SLB];
    for (i = 0; i < STL_PER_SLB*STL_PER_SLB; i++) {
        style_set[i] = 0;
        slab_number_list[i] = 0;
        room_pretty_list[i] = 0;
        slab_type_list[i] = slbkind;
    }
    struct SlabAttr *place_slbattr;
    place_slbattr = get_slab_kind_attrs(slbkind);
    if (place_slbattr->category == SlbAtCtg_FortifiedWall) {
        place_single_slab_fill_arrays_std(slb_x, slb_y, slab_type_list, room_pretty_list);
    }
    delete_attached_things_on_slab(slb_x, slb_y);
    delete_attached_lights_on_slab(slb_x, slb_y);

    ColumnIndex col_idx[STL_PER_SLB*STL_PER_SLB];
    {
        int slabct_num;
        slabct_num = get_slabset_index(slbkind, 3, 0);
        struct SlabSet *sset;
        sset = &game.slabset[slabct_num];
        for (i=0; i < STL_PER_SLB*STL_PER_SLB; i++)
        {
            col_idx[i] = sset->col_idx[i];
        }
    }
    place_single_slab_fill_style_array(slb_x, slb_y, style_set);

    if ((slbkind == SlbT_WALLTORCH) || (slbkind == SlbT_TORCHDIRT))
    {
        place_single_slab_set_torch_places(slbkind, slb_x, slb_y, slab_type_list);
    }
    place_single_slab_prepare_column_index(slbkind, slb_x, slb_y, plyr_idx, slab_type_list, room_pretty_list, style_set, slab_number_list, col_idx);
    if (place_slbattr->category == SlbAtCtg_FortifiedWall)
    {
        place_single_slab_modify_column_near_liquid(slbkind, slb_x, slb_y, plyr_idx, slab_type_list, room_pretty_list, style_set, slab_number_list, col_idx);
    }

    {
        struct SlabMap *slb;
        slb = get_slabmap_block(slb_x,slb_y);
        slb->health = game.block_health[place_slbattr->block_health_index];
    }
    place_slab_columns(slbkind, STL_PER_SLB * slb_x, STL_PER_SLB * slb_y, col_idx);
    place_slab_objects(slb_x, slb_y, slab_number_list, plyr_idx);
}

void shuffle_unattached_things_on_slab(long a1, long a2)
{
    _DK_shuffle_unattached_things_on_slab(a1, a2); return;
}

void dump_slab_on_map(SlabKind slbkind, long slabct_num, MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber owner)
{
    //_DK_dump_slab_on_map(slbkind, a2, stl_x, stl_y, owner); return;
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    slb_x = subtile_slab_fast(stl_x);
    slb_y = subtile_slab_fast(stl_y);
    MapSubtlCoord stl_xa;
    MapSubtlCoord stl_ya;
    stl_xa = STL_PER_SLB * slb_x;
    stl_ya = STL_PER_SLB * slb_y;
    if (slabct_num >= SLABSET_COUNT) {
        ERRORLOG("Illegal animating slab number: %d", slabct_num);
        slabct_num = 0;
    }
    struct SlabAttr *slbattr;
    slbattr = get_slab_kind_attrs(slbkind);
    struct SlabMap *slb;
    slb = get_slabmap_block(slb_x, slb_y);
    slb->health = game.block_health[slbattr->block_health_index];
    struct SlabSet *sset;
    sset = &game.slabset[slabct_num];
    place_slab_columns(slbkind, stl_xa, stl_ya, sset->col_idx);
    set_whole_slab_owner(slb_x, slb_y, owner);

    SlabCodedCoords place_slbnum;
    place_slbnum = get_slab_number(slb_x, slb_y);
    int n;
    n = 0;
    MapSubtlDelta dx;
    MapSubtlDelta dy;
    for (dy=0; dy < STL_PER_SLB; dy++)
    {
        MapSubtlCoord sstl_x;
        MapSubtlCoord sstl_y;
        sstl_y = slab_subtile(slb_y,dy);
        for (dx=0; dx < STL_PER_SLB; dx++)
        {
            sstl_x = slab_subtile(slb_x,dx);

            struct Map *mapblk;
            mapblk = get_map_block_at(sstl_x, sstl_y);
            long i;
            unsigned long k;
            k = 0;
            i = get_mapwho_thing_index(mapblk);
            while (i != 0)
            {
                struct Thing *thing;
                thing = thing_get(i);
                TRACE_THING(thing);
                if (thing_is_invalid(thing))
                {
                    ERRORLOG("Jump to invalid thing detected");
                    break;
                }
                i = thing->next_on_mapblk;
                // Per thing code start
                int floor_height;
                floor_height = get_map_floor_filled_subtiles(mapblk);
                //TODO this condition does not look consistent
                if ((thing->class_id != TCls_Creature) || (floor_height <= 4))
                {
                    if (thing->model != 2) {
                        thing->mappos.z.val = subtile_coord(floor_height,0);
                    }
                }
                // Per thing code end
                k++;
                if (k > THINGS_COUNT)
                {
                    ERRORLOG("Infinite loop detected when sweeping things list");
                    break_mapwho_infinite_chain(mapblk);
                    break;
                }
            }

            place_slab_object(place_slbnum, sstl_x, sstl_y, slabct_num, n, slabmap_owner(slb));
            n++;
        }
    }

    slb = get_slabmap_block(slb_x, slb_y);
    slb->kind = slbkind;
    pannel_map_update(stl_xa, stl_ya, STL_PER_SLB, STL_PER_SLB);
    if ((slbkind == SlbT_SLAB50) || (slbkind == SlbT_GUARDPOST) || (slbkind == SlbT_BRIDGE) || (slbkind == SlbT_GEMS) || (slbkind == SlbT_PURPLE))
    {
        MapSubtlCoord stl_xb;
        MapSubtlCoord stl_yb;
        stl_yb = stl_ya + STL_PER_SLB - 1;
        if (stl_yb > map_subtiles_y)
            stl_yb = map_subtiles_y;
        stl_xb = stl_xa + STL_PER_SLB - 1;
        if (stl_xb > map_subtiles_x)
            stl_xb = map_subtiles_x;
        update_blocks_in_area(stl_xa, stl_ya, stl_xb, stl_yb);
    }
}

void place_animating_slab_type_on_map(SlabKind slbkind, char ani_frame, MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber owner)
{
    SYNCDBG(7,"Starting");
    //_DK_place_animating_slab_type_on_map(slbkind,a2,a3,a4,owner);
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    slb_x = subtile_slab_fast(stl_x);
    slb_y = subtile_slab_fast(stl_y);
    if (!slab_kind_is_animated(slbkind))
    {
        ERRORLOG("Attempt to dump an invalid animating slab: %d", (int)slbkind);
        dump_slab_on_map(SlbT_LAVA, 0, stl_x, stl_y, game.neutral_player_num);
        return;
    }
    struct SlabMap *slbmap = get_slabmap_block(slb_x, slb_y);
    if (slbmap->kind != SlbT_GEMS)
    {
        all_players_untag_blocks_for_digging_in_area(slb_x, slb_y);
    }
    delete_attached_things_on_slab(slb_x, slb_y);
    dump_slab_on_map(slbkind, 840 + 8 * slbkind + ani_frame, stl_x, stl_y, owner);
    shuffle_unattached_things_on_slab(slb_x, slb_y);
    int i;
    for (i = 0; i < AROUND_EIGHT_LENGTH; i++)
    {
        MapSlabCoord sslb_x;
        MapSlabCoord sslb_y;
        sslb_x = slb_x + (MapSlabCoord)my_around_eight[i].delta_x;
        sslb_y = slb_y + (MapSlabCoord)my_around_eight[i].delta_y;
        struct SlabMap *slb;
        slb = get_slabmap_block(sslb_x, sslb_y);
        if (slabmap_block_invalid(slb)) {
            continue;
        }
        int ssub_x;
        int ssub_y;
        for (ssub_y=0; ssub_y < STL_PER_SLB; ssub_y++)
        {
            for (ssub_x=0; ssub_x < STL_PER_SLB; ssub_x++)
            {
                MapSubtlCoord sstl_x;
                MapSubtlCoord sstl_y;
                sstl_x = slab_subtile(sslb_x,ssub_x);
                sstl_y = slab_subtile(sslb_y,ssub_y);
                if (wibble_enabled())
                {
                    set_alt_bit_based_on_slab(slb->kind, sstl_x, sstl_y);
                }
            }
        }
    }
    if (slbkind == SlbT_GEMS)
    {
        delete_all_object_things_from_slab(slb_x, slb_y, 0);
    }
}

/**
 * For an unfortified dirt slab, select either "clean" slab type, or the one with place for torches.
 * @param slbkind
 * @param tgslb_x
 * @param tgslb_y
 * @param owner
 * @return
 */
SlabKind alter_rock_style(SlabKind slbkind, MapSlabCoord tgslb_x, MapSlabCoord tgslb_y, PlayerNumber owner)
{
    SlabKind retkind;
    retkind = slbkind;
    if (slbkind == SlbT_EARTH)
    {
        long i;
        for (i = 0; i < AROUND_EIGHT_LENGTH; i++)
        {
            MapSlabCoord slb_y;
            MapSlabCoord slb_x;
            unsigned char flags;
            slb_x = tgslb_x + my_around_eight[i].delta_x;
            slb_y = tgslb_y + my_around_eight[i].delta_y;
            struct SlabMap *slb;
            struct SlabAttr *slbattr;
            slb = get_slabmap_block(slb_x,slb_y);
            if (slabmap_block_invalid(slb))
                continue;
            slbattr = get_slab_attrs(slb);
            if ((slbattr->category == SlbAtCtg_FortifiedGround) || (slbattr->category == SlbAtCtg_RoomInterior) || (slbattr->category == SlbAtCtg_Obstacle))
            {
                flags = torch_flags_for_slab(tgslb_x, tgslb_y);
                retkind = (flags < 1) ? SlbT_EARTH : SlbT_TORCHDIRT;
                break;
            }
        }
        if (i == AROUND_EIGHT_LENGTH)
          retkind = SlbT_EARTH;
    }
    return retkind;
}

void place_slab_type_on_map_f(SlabKind nslab, MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber owner, unsigned char a5,const char *func_name)
{
    SlabKind previous_slab_types_around[8];
    struct SlabMap *slb;
    struct SlabAttr *slbattr;
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    MapSlabCoord spos_x;
    MapSlabCoord spos_y;
    int skind;
    long i;
    SYNCDBG(7,"%s: Starting for (%d,%d)",func_name,(int)stl_x,(int)stl_y);
    if (subtile_coords_invalid(stl_x, stl_y))
        return;
    slb_x = subtile_slab_fast(stl_x);
    slb_y = subtile_slab_fast(stl_y);
    if (slab_kind_is_animated(nslab))
    {
        ERRORLOG("%s: Placing animating slab %d as standard slab",func_name,(int)nslab);
    }
    for (i = 0; i < AROUND_EIGHT_LENGTH; i++)
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
    if ( (slb->kind >= SlbT_WALLDRAPE) && (slb->kind <= SlbT_WALLPAIRSHR) )
    {
        if ( (skind < SlbT_WALLDRAPE) || (skind > SlbT_WALLPAIRSHR) )
        {
            all_players_untag_blocks_for_digging_in_area(slb_x, slb_y);
        }
    }
    else if ( (slb->kind == SlbT_EARTH) || (slb->kind == SlbT_TORCHDIRT) )
    {
        if ( (skind != SlbT_EARTH) && (skind != SlbT_TORCHDIRT) )
        {
            all_players_untag_blocks_for_digging_in_area(slb_x, slb_y);
        }
    }
    else
    {
        if (slb->kind != skind)
        {
            all_players_untag_blocks_for_digging_in_area(slb_x, slb_y);
        }
    }
    slb->kind = skind;

    set_whole_slab_owner(slb_x, slb_y, owner);
    place_single_slab_type_on_map(skind, slb_x, slb_y, owner);
    shuffle_unattached_things_on_slab(slb_x, slb_y);

    slbattr = get_slab_kind_attrs(skind);
    if ((slbattr->category == SlbAtCtg_RoomInterior) || (slbattr->category == SlbAtCtg_FortifiedGround))
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

    pannel_map_update(slab_subtile(slb_x,0), slab_subtile(slb_y,0), STL_PER_SLB, STL_PER_SLB);

    for (i = 0; i < AROUND_EIGHT_LENGTH; i++)
    {
        spos_x = slb_x + (MapSlabCoord)my_around_eight[i].delta_x;
        spos_y = slb_y + (MapSlabCoord)my_around_eight[i].delta_y;
        slb = get_slabmap_block(spos_x,spos_y);
        if (slabmap_block_invalid(slb))
            continue;
        slbattr = get_slab_kind_attrs(slb->kind);
        if ((previous_slab_types_around[i] != slb->kind)
          || ((slbattr->category != SlbAtCtg_Obstacle) && (slbattr->category != SlbAtCtg_Unclaimed))
          || (game.game_kind == GKind_Unknown1))
        {
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
    SlabKind nslab;
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
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    long x;
    long y;
    long z;
    stl_x = STL_PER_SLB * slb_x;
    stl_y = STL_PER_SLB * slb_y;
    z = get_floor_filled_subtiles_at(stl_x, stl_y);
    for (y = stl_y; y < stl_y+STL_PER_SLB; y++)
    {
        for (x = stl_x; x < stl_x+STL_PER_SLB; x++)
        {
            if (z > 0) {
                create_gold_rubble_for_dug_block(x, y, z, game.neutral_player_num);
            }
        }
    }
}

/**
 * Updates given floor and ceiling height so that they are restricted to given subtile.
 * Can be used sequentially on adjacent subtiles to compute max height of a thing which could fit through.
 *
 * @param stl_x The subtile to be checked, X coord.
 * @param stl_y The subtile to be checked, Y coord.
 * @param floor_height Floor height value reference. Value is updated only if new one is larger.
 * @param ceiling_height Ceiling height value reference. Value is updated only if new one is smaller.
 */
void update_floor_and_ceiling_heights_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y,
    MapSubtlCoord *floor_height, MapSubtlCoord *ceiling_height)
{
    struct Map *mapblk;
    unsigned long height;
    unsigned long k;
    mapblk = get_map_block_at(stl_x, stl_y);
    k = get_map_floor_filled_subtiles(mapblk);
    if (k > 0) {
        height = k;
    } else {
        height = 0;
    }
    if (*floor_height < height) {
        *floor_height = height;
    }
    k = get_map_ceiling_filled_subtiles(mapblk);
    if (k > 0) {
        height = 8 - k;
    } else {
        height = get_mapblk_filled_subtiles(mapblk);
    }
    if (*ceiling_height > height) {
        *ceiling_height = height;
    }
}

TbBool point_in_map_is_solid(const struct Coord3d *pos)
{
    MapSubtlCoord floor_height;
    MapSubtlCoord ceiling_height;
    unsigned long check_h;
    check_h = pos->z.stl.num;
    struct Map *mapblk;
    mapblk = get_map_block_at(pos->x.stl.num, pos->y.stl.num);
    if (get_map_ceiling_filled_subtiles(mapblk) > 0)
    {
        floor_height = 0;
        ceiling_height = 15;
        update_floor_and_ceiling_heights_at(pos->x.stl.num, pos->y.stl.num, &floor_height, &ceiling_height);
    } else
    {
        floor_height = get_map_floor_filled_subtiles(mapblk);
        ceiling_height = get_mapblk_filled_subtiles(mapblk);
    }
    if ((ceiling_height <= check_h) || (floor_height > check_h)) {
        SYNCDBG(17, "Solid at (%d,%d,%d)",(int)pos->x.stl.num,(int)pos->y.stl.num,(int)pos->z.stl.num);
        return true;
    }
    return false;
}

/**
 * Destroys a tall gold slab, replacing it with neutral ground.
 * @param stl_x Slab subtile digged out, X coordinate.
 * @param stl_y Slab subtile digged out, Y coordinate.
 * @param plyr_idx Index of the player who does the digging.
 */
void mine_out_block(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
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
    // TODO MAP Maybe it should be cleared only if sibling non-gold and non-rock slabs are invisible
    set_slab_explored_flags(1 << plyr_idx, slb_x, slb_y);
}

TbBool dig_has_revealed_area(MapSubtlCoord rev_stl_x, MapSubtlCoord rev_stl_y, PlayerNumber plyr_idx)
{
    int i;
    for (i=0; i < SMALL_AROUND_LENGTH; i++)
    {
        MapSubtlCoord stl_x;
        MapSubtlCoord stl_y;
        stl_x = rev_stl_x + 3*small_around[i].delta_x;
        stl_y = rev_stl_y + 3*small_around[i].delta_y;
        if (!subtile_revealed(stl_x, stl_y, plyr_idx))
        {
            struct SlabMap *slb;
            struct SlabAttr *slbattr;
            slb = get_slabmap_for_subtile(stl_x, stl_y);
            slbattr = get_slab_attrs(slb);
            if ((slbattr->block_flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) == 0) {
                return true;
            }
        }
    }
    return false;
}

void create_dirt_rubble_for_dug_slab(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    long x;
    long y;
    long z;
    stl_x = STL_PER_SLB * slb_x;
    stl_y = STL_PER_SLB * slb_y;
    z = get_floor_filled_subtiles_at(stl_x, stl_y);
    for (y = stl_y; y < stl_y+STL_PER_SLB; y++)
    {
        for (x = stl_x; x < stl_x+STL_PER_SLB; x++)
        {
            if (z > 0) {
                create_dirt_rubble_for_dug_block(x, y, z, game.neutral_player_num);
            }
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
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
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

void clear_dig_and_set_explored_around(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx)
{
    struct SlabMap *slb;
    slb = get_slabmap_block(slb_x+1, slb_y);
    if (!slabmap_block_invalid(slb))
    {
        struct SlabAttr *slbattr;
        slbattr = get_slab_attrs(slb);
        if ((slbattr->block_flags & SlbAtFlg_IsDoor) != 0)
        {
            clear_slab_dig(slb_x+1, slb_y, plyr_idx);
            set_slab_explored(plyr_idx, slb_x+1, slb_y);
        }
    }
    slb = get_slabmap_block(slb_x-1, slb_y);
    if (!slabmap_block_invalid(slb))
    {
        struct SlabAttr *slbattr;
        slbattr = get_slab_attrs(slb);
        if ((slbattr->block_flags & SlbAtFlg_IsDoor) != 0)
        {
            clear_slab_dig(slb_x-1, slb_y, plyr_idx);
            set_slab_explored(plyr_idx, slb_x-1, slb_y);
        }
    }
    slb = get_slabmap_block(slb_x, slb_y+1);
    if (!slabmap_block_invalid(slb))
    {
        struct SlabAttr *slbattr;
        slbattr = get_slab_attrs(slb);
        if ((slbattr->block_flags & SlbAtFlg_IsDoor) != 0)
        {
            clear_slab_dig(slb_x, slb_y+1, plyr_idx);
            set_slab_explored(plyr_idx, slb_x, slb_y+1);
        }
    }
    slb = get_slabmap_block(slb_x, slb_y-1);
    if (!slabmap_block_invalid(slb))
    {
        struct SlabAttr *slbattr;
        slbattr = get_slab_attrs(slb);
        if ((slbattr->block_flags & SlbAtFlg_IsDoor) != 0)
        {
            clear_slab_dig(slb_x, slb_y-1, plyr_idx);
            set_slab_explored(plyr_idx, slb_x, slb_y-1);
        }
    }
}

void clear_dig_and_set_explored_can_see_x(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx, int can_see_slabs)
{
    int delta_see;
    for (delta_see = -can_see_slabs; delta_see <= can_see_slabs; delta_see++)
    {
        if ((delta_see + slb_x < 0) || (delta_see + slb_x >= 85)) {
            continue;
        }
        TbBool go_dir1;
        TbBool go_dir2;
        TbBool allow_next_dir1;
        TbBool allow_next_dir2;
        int delta_shift;
        int delta_x;
        int rad_y;
        int rad_x;
        delta_shift = 256 * delta_see;
        rad_x = 128;
        allow_next_dir1 = 0;
        allow_next_dir2 = 0;
        rad_y = 0;
        delta_x = delta_shift / can_see_slabs;
        go_dir1 = 1;
        go_dir2 = 1;
        while (rad_y < can_see_slabs<<8)
        {
            struct SlabMap *slb;
            struct SlabAttr *slbattr;
            MapSlabCoord lslb_y;
            MapSlabCoord hslb_x;
            MapSlabCoord hslb_y;
            if (!go_dir1 && !go_dir2)
              break;
            rad_x += delta_x;
            rad_y += 256;
            hslb_x = slb_x + (rad_x >> 8);
            lslb_y = slb_y - (rad_y >> 8);
            hslb_y = slb_y + (rad_y >> 8);
            if ((lslb_y < 0) || (hslb_y >= 85))
                continue;
            if ( go_dir1 )
            {
                if (delta_shift > 0)
                {
                    slb = get_slabmap_block(hslb_x, lslb_y-1);
                    slbattr = get_slab_attrs(slb);
                    if ((slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0)
                    {
                        slb = get_slabmap_block(hslb_x, lslb_y+1);
                        slbattr = get_slab_attrs(slb);
                        if ((slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0) {
                            allow_next_dir1 = 1;
                            go_dir1 = 0;
                        }
                    }
                }
                else
                if (delta_shift < 0)
                {
                    slb = get_slabmap_block(hslb_x+1, lslb_y);
                    slbattr = get_slab_attrs(slb);
                    if ((slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0)
                    {
                        slb = get_slabmap_block(hslb_x, lslb_y+1);
                        slbattr = get_slab_attrs(slb);
                        if ((slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0) {
                            allow_next_dir1 = 1;
                            go_dir1 = 0;
                        }
                    }
                }
                if ( go_dir1 )
                {
                  clear_slab_dig(hslb_x, lslb_y, plyr_idx);
                  slb = get_slabmap_block(hslb_x, lslb_y);
                  slbattr = get_slab_attrs(slb);
                  if (go_dir1 || (slbattr->block_flags & SlbAtFlg_Blocking)) {
                      set_slab_explored(plyr_idx, hslb_x, lslb_y);
                  }
                  if (slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) {
                      go_dir1 = 0;
                  }
                }
                else
                if ( allow_next_dir1 )
                {
                    allow_next_dir1 = 0;
                    slb = get_slabmap_block(hslb_x, lslb_y);
                    slbattr = get_slab_attrs(slb);
                    if (slbattr->block_flags & SlbAtFlg_Filled)
                    {
                      clear_slab_dig(hslb_x, lslb_y, plyr_idx);
                      if (go_dir1 || (slbattr->block_flags & SlbAtFlg_Blocking)) {
                          set_slab_explored(plyr_idx, hslb_x, lslb_y);
                      }
                    }
                }
            }
            if ( go_dir2 )
            {
              if (delta_shift > 0)
              {
                  slb = get_slabmap_block(hslb_x, hslb_y-1);
                  slbattr = get_slab_attrs(slb);
                  if (slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable))
                  {
                      slb = get_slabmap_block(hslb_x-1, hslb_y);
                      slbattr = get_slab_attrs(slb);
                      if (slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) {
                        allow_next_dir2 = 1;
                        go_dir2 = 0;
                      }
                  }
              }
              else
              if (delta_shift < 0)
              {
                  slb = get_slabmap_block(hslb_x, hslb_y-1);
                  slbattr = get_slab_attrs(slb);
                  if (slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable))
                  {
                      slb = get_slabmap_block(hslb_x+1, hslb_y);
                      slbattr = get_slab_attrs(slb);
                      if (slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) {
                        allow_next_dir2 = 1;
                        go_dir2 = 0;
                      }
                  }
              }
              if ( go_dir2 )
              {
                  clear_slab_dig(hslb_x, hslb_y, plyr_idx);
                  slb = get_slabmap_block(hslb_x, hslb_y);
                  slbattr = get_slab_attrs(slb);
                  if (go_dir2 || (slbattr->block_flags & SlbAtFlg_Blocking)) {
                      set_slab_explored(plyr_idx, hslb_x, hslb_y);
                  }
                  if (slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) {
                      go_dir2 = 0;
                  }
              }
              else
              if ( allow_next_dir2 )
              {
                  allow_next_dir2 = 0;
                  slb = get_slabmap_block(hslb_x, hslb_y);
                  slbattr = get_slab_attrs(slb);
                  if (slbattr->block_flags & SlbAtFlg_Filled)
                  {
                      clear_slab_dig(hslb_x, hslb_y, plyr_idx);
                      slb = get_slabmap_block(hslb_x, hslb_y);
                      slbattr = get_slab_attrs(slb);
                      if (go_dir2 || (slbattr->block_flags & SlbAtFlg_Blocking)) {
                          set_slab_explored(plyr_idx, hslb_x, hslb_y);
                      }
                  }
              }
            }
        }
    }
}

void clear_dig_and_set_explored_can_see_y(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx, int can_see_slabs)
{
    int delta_see;
    for (delta_see = -can_see_slabs; delta_see <= can_see_slabs; delta_see++)
    {
        if ((delta_see + slb_y < 0) || (delta_see + slb_y >= 85)) {
            continue;
        }
        TbBool go_dir1;
        TbBool go_dir2;
        TbBool allow_next_dir1;
        TbBool allow_next_dir2;
        int delta_shift;
        int delta_y;
        int rad_y;
        int rad_x;
        delta_shift = 256 * delta_see;
        rad_y = 128;
        allow_next_dir1 = 0;
        allow_next_dir2 = 0;
        rad_x = 0;
        delta_y = delta_shift / can_see_slabs;
        go_dir1 = 1;
        go_dir2 = 1;
        while (rad_x < can_see_slabs<<8)
        {
            struct SlabMap *slb;
            struct SlabAttr *slbattr;
            MapSlabCoord lslb_x;
            MapSlabCoord hslb_x;
            MapSlabCoord hslb_y;
            if (!go_dir1 && !go_dir2)
              break;
            rad_x += 256;
            rad_y += delta_y;
            lslb_x = slb_x - (rad_x >> 8);
            hslb_x = slb_x + (rad_x >> 8);
            hslb_y = slb_y + (rad_y >> 8);
            if ((lslb_x < 0) || (hslb_x >= 85))
                continue;
            if ( go_dir1 )
            {
                if (delta_shift > 0)
                {
                    slb = get_slabmap_block(lslb_x, hslb_y-1);
                    slbattr = get_slab_attrs(slb);
                    if ((slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0)
                    {
                        slb = get_slabmap_block(lslb_x+1, hslb_y);
                        slbattr = get_slab_attrs(slb);
                        if ((slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0) {
                            allow_next_dir1 = 1;
                            go_dir1 = 0;
                        }
                    }
                }
                else
                if (delta_shift < 0)
                {
                    slb = get_slabmap_block(lslb_x+1, hslb_y);
                    slbattr = get_slab_attrs(slb);
                    if ((slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0)
                    {
                        slb = get_slabmap_block(lslb_x, hslb_y+1);
                        slbattr = get_slab_attrs(slb);
                        if ((slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0) {
                            allow_next_dir1 = 1;
                            go_dir1 = 0;
                        }
                    }
                }
                if ( go_dir1 )
                {
                    clear_slab_dig(lslb_x, hslb_y, plyr_idx);
                    slb = get_slabmap_block(lslb_x, hslb_y);
                    slbattr = get_slab_attrs(slb);
                    if ( go_dir1 || (slbattr->block_flags & SlbAtFlg_Blocking)) {
                        set_slab_explored(plyr_idx, lslb_x, hslb_y);
                    }
                    if (slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) {
                        go_dir1 = 0;
                    }
                } else
                if ( allow_next_dir1 )
                {
                    allow_next_dir1 = 0;
                    slb = get_slabmap_block(lslb_x, hslb_y);
                    slbattr = get_slab_attrs(slb);
                    if (slbattr->block_flags & SlbAtFlg_Filled)
                    {
                        clear_slab_dig(lslb_x, hslb_y, plyr_idx);
                        if ( go_dir1 || (slbattr->block_flags & SlbAtFlg_Blocking)) {
                            set_slab_explored(plyr_idx, lslb_x, hslb_y);
                        }
                    }
                }
            }
            if ( go_dir2 )
            {
              if (delta_shift > 0)
              {
                  slb = get_slabmap_block(hslb_x-1, hslb_y);
                  slbattr = get_slab_attrs(slb);
                  if (slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable))
                  {
                      slb = get_slabmap_block(hslb_x, hslb_y-1);
                      slbattr = get_slab_attrs(slb);
                      if (slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) {
                          allow_next_dir2 = 0;
                          go_dir2 = 0;
                      }
                  }
              } else
              if (delta_shift < 0)
              {
                  slb = get_slabmap_block(hslb_x-1, hslb_y);
                  slbattr = get_slab_attrs(slb);
                  if (slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable))
                  {
                      slb = get_slabmap_block(hslb_x, hslb_y+1);
                      slbattr = get_slab_attrs(slb);
                      if (slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) {
                          allow_next_dir2 = 1;
                          go_dir2 = 0;
                      }
                  }
              }
              if ( go_dir2 )
              {
                clear_slab_dig(hslb_x, hslb_y, plyr_idx);
                slb = get_slabmap_block(hslb_x, hslb_y);
                slbattr = get_slab_attrs(slb);
                if (go_dir2 || (slbattr->block_flags & SlbAtFlg_Blocking))
                  set_slab_explored(plyr_idx, hslb_x, hslb_y);
                if (slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable))
                  go_dir2 = 0;
              } else
              if ( allow_next_dir2 )
              {
                  allow_next_dir2 = 0;
                  slb = get_slabmap_block(hslb_x, hslb_y);
                  slbattr = get_slab_attrs(slb);
                  if (slbattr->block_flags & SlbAtFlg_Filled)
                  {
                      clear_slab_dig(hslb_x, hslb_y, plyr_idx);
                      if (go_dir2 || (slbattr->block_flags & SlbAtFlg_Blocking)) {
                          set_slab_explored(plyr_idx, hslb_x, hslb_y);
                      }
                  }
              }
            }
        }
    }
}

void check_map_explored(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    if (is_neutral_thing(creatng) || is_hero_thing(creatng))
        return;
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = get_floor_height_at(&pos);
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    slb_x = map_to_slab[stl_x];
    slb_y = map_to_slab[stl_y];
    struct SlabMap *slb;
    struct SlabAttr *slbattr;
    slb = get_slabmap_block(slb_x, slb_y);
    slbattr = get_slab_attrs(slb);
    if (slbattr->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable))
    {
        if (slbattr->block_flags & SlbAtFlg_IsDoor)
        {
            clear_dig_and_set_explored_around(slb_x, slb_y, creatng->owner);
        }
        return;
    }

    int can_see_slabs;
    can_see_slabs = get_explore_sight_distance_in_slabs(creatng);
    if (!player_cannot_win(creatng->owner) && ((get_creature_model_flags(creatng) & CMF_IsSpectator) == 0)) {
        claim_neutral_creatures_in_sight(creatng, &pos, can_see_slabs);
    }
    clear_slab_dig(slb_x, slb_y, creatng->owner);
    set_slab_explored(creatng->owner, slb_x, slb_y);
    clear_dig_and_set_explored_can_see_x(slb_x, slb_y, creatng->owner, can_see_slabs);
    clear_dig_and_set_explored_can_see_y(slb_x, slb_y, creatng->owner, can_see_slabs);
}

long ceiling_partially_recompute_heights(long sx, long sy, long ex, long ey)
{
    return _DK_ceiling_partially_recompute_heights(sx, sy, ex, ey);
}

long element_top_face_texture(struct Map *map)
{
  return _DK_element_top_face_texture(map);
}

TbBool point_in_map_is_solid_ignoring_door(const struct Coord3d *pos, const struct Thing *doortng)
{
    struct Thing *thing;
    thing = get_door_for_position(pos->x.stl.num, pos->y.stl.num);
    if (!thing_is_invalid(thing)) {
        return (thing->index != doortng->index);
    } else {
        return point_in_map_is_solid(pos);
    }
}

unsigned short get_point_in_map_solid_flags_ignoring_door(const struct Coord3d *pos, const struct Thing *doortng)
{
    struct Thing *thing;
    unsigned short flags;
    thing = get_door_for_position(pos->x.stl.num, pos->y.stl.num);
    flags = 0;
    if (!thing_is_invalid(thing))
    {
        if (thing->index != doortng->index) {
            flags |= 0x01;
        }
    } else
    if (map_pos_is_lava(pos->x.stl.num, pos->y.stl.num))
    {
        flags |= 0x02;
    } else
    if (point_in_map_is_solid(pos))
    {
        flags |= 0x01;
    }
    return flags;
}

unsigned short get_point_in_map_solid_flags_ignoring_own_door(const struct Coord3d *pos, PlayerNumber plyr_idx)
{
    unsigned short flags;
    flags = 0;
    if (map_pos_is_lava(pos->x.stl.num, pos->y.stl.num))
    {
        flags |= 0x02;
    } else
    if (point_in_map_is_solid(pos))
    {
        struct Thing *thing;
        thing = get_door_for_position(pos->x.stl.num, pos->y.stl.num);
        if (!thing_is_invalid(thing))
        {
            if ((thing->owner != plyr_idx) || (thing->door.is_locked)) {
                flags |= 0x01;
            }
        } else
        {
            flags |= 0x01;
        }
    }
    return flags;
}

void fill_in_reinforced_corners(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    SYNCDBG(16,"Starting");
    // _DK_fill_in_reinforced_corners(plyr_idx, slb_x, slb_y); return;
  struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
  struct SlabAttr* slbattr = get_slab_attrs(slb);
  if ((slbattr->category != SlbAtCtg_FortifiedWall))
   return;
  if ( slabmap_owner(slb) != plyr_idx )
    return;
  for (long n = 0; n < SMALL_AROUND_LENGTH; n++)
  {
    MapSlabCoord x = slb_x + small_around[n].delta_x;
    MapSlabCoord y = slb_y + small_around[n].delta_y;
    struct SlabMap *slb2 = get_slabmap_block(x, y);
    struct SlabAttr* slbattr2 = get_slab_attrs(slb2);
    if ( (((slbattr2->category == SlbAtCtg_FortifiedGround) || (slbattr2->block_flags & SlbAtFlg_IsRoom) || ((slbattr2->block_flags & SlbAtFlg_IsDoor)) )) 
      && (slabmap_owner(slb2) == plyr_idx ) )
    {
      for (int k = -1; k < 2; k+=2)
      {
        int j = (k + n) & 3;
        MapSlabCoord x2 = x + small_around[j].delta_x;
        MapSlabCoord y2 = y + small_around[j].delta_y;
        struct SlabMap *slb3 = get_slabmap_block(x2, y2);
        struct SlabAttr* slbattr3 = get_slab_attrs(slb3);
        if ( (slbattr3->category == SlbAtCtg_FortifiedWall) 
          && (slabmap_owner(slb3) == plyr_idx ) )
        {
          int m = (k + j) & 3;
          MapSlabCoord x3 = x2 + small_around[m].delta_x;
          MapSlabCoord y3 = y2 + small_around[m].delta_y;
          struct SlabMap *slb4 = get_slabmap_block(x3, y3);
          struct SlabAttr* slbattr4 = get_slab_attrs(slb4);
          if ( (slbattr4->category == SlbAtCtg_FriableDirt) )
          {
            unsigned char pretty_type = choose_pretty_type(plyr_idx, x3, y3);
            place_slab_type_on_map(pretty_type, slab_subtile(x3, 0), slab_subtile(y3, 0), plyr_idx, 1);
            do_slab_efficiency_alteration(x3, y3);
            set_slab_explored(plyr_idx, x3, y3);
            remove_task_from_all_other_players_digger_stacks(plyr_idx, slab_subtile_center(x3), slab_subtile_center(y3));
          }
        }
      }
    }
  }
}

SlabKind choose_pretty_type(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap *pvslb, *nxslb;
    SYNCDBG(16,"Starting");
    // return _DK_choose_pretty_type(plyr_idx, slb_x, slb_y);
    // if x co-ordinate is divisible by 5
    if (slb_x % 5 == 0)
    {
        pvslb = get_slabmap_block(slb_x, slb_y - 1);
        nxslb = get_slabmap_block(slb_x, slb_y + 1);
        if ( (pvslb->kind == SlbT_CLAIMED) || (nxslb->kind == SlbT_CLAIMED) )  
        {
            return SlbT_WALLTORCH;
        }
    }
    // if y co-ordinate is divisible by 5
    if (slb_y % 5 == 0)
    {
        pvslb = get_slabmap_block(slb_x - 1, slb_y);
        nxslb = get_slabmap_block(slb_x + 1, slb_y);
        if ( (pvslb->kind == SlbT_CLAIMED) || (nxslb->kind == SlbT_CLAIMED) )
        {
            return SlbT_WALLTORCH;
        }
    }
    // if x co-ordinate is odd
    if ((slb_x & 1) != 0)
    {
        pvslb = get_slabmap_block(slb_x, slb_y - 1);
        nxslb = get_slabmap_block(slb_x, slb_y + 1);
        if ( (pvslb->kind == SlbT_CLAIMED) || (nxslb->kind == SlbT_CLAIMED) )
        {
        return SlbT_WALLDRAPE;
        }
    }
    // if y co-ordinate is odd
    if ((slb_y & 1) != 0)
    {
        pvslb = get_slabmap_block(slb_x - 1, slb_y);
        nxslb = get_slabmap_block(slb_x + 1, slb_y);
        if ( (pvslb->kind == SlbT_CLAIMED) || (nxslb->kind == SlbT_CLAIMED) )
        {
            return SlbT_WALLDRAPE;
        }
    }
    /* else, choose an art type, based on the tile's distance from its owner's Dungeon Heart,
    or the centre of the map if there's no heart */
    const SlabKind pretty_types[3] = {SlbT_WALLWTWINS, SlbT_WALLWWOMAN, SlbT_WALLPAIRSHR};
    struct Coord3d pos, pos2;
    struct Thing *heartng = get_player_soul_container(plyr_idx);
    // this function calculates distance in slabs, not subtiles
    if (thing_is_invalid(heartng))
    {
        pos.x.val = 42;
        pos.y.val = 42;
    }
    else
    {
        pos.x.val = heartng->mappos.x.stl.num / STL_PER_SLB;
        pos.y.val = heartng->mappos.y.stl.num / STL_PER_SLB;
    }
    pos2.x.val = slb_x;
    pos2.y.val = slb_y;
    MapCoordDelta dist = get_2d_distance(&pos, &pos2);
    return pretty_types[(dist / 4) % 3];
}

void pretty_map_remove_flags_and_update(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    long m;
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    stl_x = slab_subtile_center(slb_x);
    stl_y = slab_subtile_center(slb_y);
    for (m=0; m < STL_PER_SLB*STL_PER_SLB; m++)
    {
        MapSubtlCoord x;
        MapSubtlCoord y;
        x = stl_x + (m%STL_PER_SLB);
        y = stl_y + (m/STL_PER_SLB);
        struct Map *mapblk;
        mapblk = get_map_block_at(x,y);
        mapblk->flags &= ~SlbAtFlg_TaggedValuable;
        mapblk->flags &= ~SlbAtFlg_Unexplored;
    }
    pannel_map_update(stl_x, stl_y, STL_PER_SLB, STL_PER_SLB);
}

void place_and_process_pretty_wall_slab(struct Thing *creatng, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct CreatureControl *cctrl;
    SYNCDBG(16,"Starting");
    cctrl = creature_control_get_from_thing(creatng);
    unsigned char pretty_type;
    pretty_type = choose_pretty_type(creatng->owner, slb_x, slb_y);
    place_slab_type_on_map(pretty_type, slab_subtile_center(slb_x), slab_subtile_center(slb_y), creatng->owner, 0);
    EVM_MAP_EVENT("reinforced", creatng->owner, slb_x, slb_y, "");
    do_slab_efficiency_alteration(slb_x, slb_y);
    MapSubtlCoord wrkstl_x;
    MapSubtlCoord wrkstl_y;
    wrkstl_x = stl_num_decode_x(cctrl->digger.working_stl);
    wrkstl_y = stl_num_decode_y(cctrl->digger.working_stl);
    remove_task_from_all_other_players_digger_stacks(creatng->owner, wrkstl_x, wrkstl_y);
    fill_in_reinforced_corners(creatng->owner, slb_x, slb_y);
}

/*
char point_in_map_is_solid_including_lava_check_ignoring_door(struct Coord3d *pos, struct Thing *thing)
{

}*/
/******************************************************************************/
#ifdef __cplusplus
}
#endif
