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
#include "pre_inc.h"
#include "map_blocks.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_sound.h"

#include "slab_data.h"
#include "room_data.h"
#include "map_utils.h"
#include "thing_effects.h"
#include "thing_objects.h"
#include "thing_physics.h"
#include "config_terrain.h"
#include "config_settings.h"
#include "config_creature.h"
#include "creature_senses.h"
#include "player_utils.h"
#include "ariadne_wallhug.h"
#include "spdigger_stack.h"
#include "frontmenu_ingame_map.h"
#include "game_legacy.h"
#include "engine_render.h"
#include "thing_navigate.h"
#include "thing_physics.h"
#include "config_spritecolors.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

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

const unsigned char  *against_to_case[] = {
    NULL,            NULL,            NULL,            NULL,
    NULL,special_cases[0],            NULL,special_cases[1],
    NULL,            NULL,special_cases[2],special_cases[3],
    NULL,special_cases[4],special_cases[5],            NULL,
};

/******************************************************************************/
TbBool block_has_diggable_side(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
  long i;
  for (i = 0; i < SMALL_AROUND_SLAB_LENGTH; i++)
  {
    // slab_is_safe_land looks at the slab owner. We don't want that here.
    struct SlabMap* slb = get_slabmap_block(slb_x + small_around[i].delta_x, slb_y + small_around[i].delta_y);
    struct SlabConfigStats* slabst = get_slab_stats(slb);
    if (slabst->is_safe_land)
      return true;
  }
  return false;
}

int block_count_diggable_sides(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    int num_sides = 0;
    for (long i = 0; i < SMALL_AROUND_SLAB_LENGTH; i++)
    {
        // slab_is_safe_land looks at the slab owner. We don't want that here.
        struct SlabMap* slb = get_slabmap_block(slb_x + small_around[i].delta_x, slb_y + small_around[i].delta_y);
        struct SlabConfigStats* slabst = get_slab_stats(slb);
        if (slabst->is_safe_land) {
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
    if ( (x < 0) || (x >= game.map_subtiles_x) || (y < 0) || (y >= game.map_subtiles_y) ) {
        ERRORLOG("Attempt to tag area outside of map");
        return 0;
    }
    TbBool task_added;
    task_added = false;
    struct Map *mapblk;
    mapblk = get_map_block_at(x+1, y+1);
    struct SlabMap *slb;
    slb = get_slabmap_for_subtile(x+1, y+1);
    struct SlabConfigStats *slabst;
    slabst = get_slab_stats(slb);
    long i;
    i = get_subtile_number(x+1,y+1);
    if ((find_from_task_list(plyr_idx, i) == -1)
      && (slabst->is_diggable || !map_block_revealed(mapblk, plyr_idx))
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
      panel_map_update(x, y, STL_PER_SLB, STL_PER_SLB);
    }
    return task_added;
}

TbBool untag_blocks_for_digging_in_area(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    MapSubtlCoord x;
    MapSubtlCoord y;
    long num_untagged;
    long task_idx;
    long i;
    x = STL_PER_SLB * (stl_x/STL_PER_SLB);
    y = STL_PER_SLB * (stl_y/STL_PER_SLB);
    if ( (x < 0) || (x > game.map_subtiles_x) || (y < 0) || (y > game.map_subtiles_y) ) {
        ERRORLOG("Attempt to tag (%d,%d), which is outside of map",x,y);
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
    panel_map_update(x, y, STL_PER_SLB, STL_PER_SLB);
    return num_untagged > 0;
}

void all_players_untag_blocks_for_digging_in_area(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct Map *mapblk;
    PlayerNumber plyr_idx;
    mapblk = get_map_block_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    for (plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        if (!player_is_keeper(plyr_idx))
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
    if ( (plyr_idx == game.neutral_player_num) || subtile_revealed_directly(slab_subtile_center(slb_x), slab_subtile_center(slb_y), plyr_idx) )
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
    panel_map_update(slab_subtile(slb_x,0), slab_subtile(slb_y,0), STL_PER_SLB, STL_PER_SLB);
    return true;
}

// only used by mine_out_block
void set_slab_explored_flags(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    MapSubtlCoord stl_y = STL_PER_SLB * slb_y;
    MapSubtlCoord stl_x = STL_PER_SLB * slb_x;
    PlayerBitFlags flag = to_flag(plyr_idx);
    struct Map *mapblk = get_map_block_at(stl_x, stl_y);

    if (mapblk->revealed != flag)
    {
        get_map_block_at(stl_x,     stl_y    )->revealed = flag;
        get_map_block_at(stl_x + 1, stl_y    )->revealed = flag;
        get_map_block_at(stl_x + 2, stl_y    )->revealed = flag;
        get_map_block_at(stl_x,     stl_y + 1)->revealed = flag;
        get_map_block_at(stl_x + 1, stl_y + 1)->revealed = flag;
        get_map_block_at(stl_x + 2, stl_y + 1)->revealed = flag;
        get_map_block_at(stl_x,     stl_y + 2)->revealed = flag;
        get_map_block_at(stl_x + 1, stl_y + 2)->revealed = flag;
        get_map_block_at(stl_x + 2, stl_y + 2)->revealed = flag;

        panel_map_update(stl_x, stl_y, STL_PER_SLB, STL_PER_SLB);
    }
}

void neutralise_enemy_block(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber domn_plyr_idx)
{
    struct SlabMap *slb;
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    SYNCDBG(16,"Starting");
    slb_x = subtile_slab(stl_x);
    slb_y = subtile_slab(stl_y);
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

unsigned long delete_unwanted_things_from_liquid_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, long rmeffect)
{
    SubtlCodedCoords stl_num;
    struct Thing *thing;
    struct Map *mapblk;
    struct Coord3d pos;
    unsigned long removed_num;
    unsigned long k;
    long i;
    long n;
    stl_num = get_subtile_number_at_slab_center(slb_x, slb_y);
    removed_num = 0;
    for (n=0; n < AROUND_MAP_LENGTH; n++)
    {
        mapblk = get_map_block_at_pos(stl_num+game.around_map[n]);
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
                struct ObjectConfigStats *objst = get_object_model_stats(thing->model);
                if (objst->destroy_on_liquid)
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
            else if (thing->class_id == TCls_Door)
            {
                remove_key_on_door(thing);
                delete_thing_structure(thing, 0);
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

unsigned long remove_unwanted_things_from_wall_slab(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    SubtlCodedCoords stl_num = get_subtile_number_at_slab_center(slb_x, slb_y);
    unsigned long removed_num = 0;
    for (long n=0; n < AROUND_MAP_LENGTH; n++)
    {
        struct Map *mapblk = get_map_block_at_pos(stl_num+game.around_map[n]);
        unsigned long k = 0;
        long i = get_mapwho_thing_index(mapblk);
        while (i != 0)
        {
            struct Thing * thing = thing_get(i);
            if (thing_is_invalid(thing))
            {
                WARNLOG("Jump out of things array");
                break;
            }
            i = thing->next_on_mapblk;
            // Per thing code
            if (thing_in_wall_at(thing, &thing->mappos))
            {
                switch(thing->class_id)
                {
                    case TCls_Door:
                    {
                        // using destroy_door places claimed path, which we don't want
                        remove_key_on_door(thing);
                        delete_thing_structure(thing, 0);
                        removed_num++;
                        break;
                    }
                    case TCls_Effect:
                    {
                        destroy_effect_thing(thing);
                        removed_num++;
                        break;
                    }
                    case TCls_Object:
                    {
                        struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
                        if ((objst->model_flags & OMF_DestroyedOnRoomPlace) != 0)
                        {
                            destroy_object(thing);
                            removed_num++;
                        }
                        else
                        {
                            move_creature_to_nearest_valid_position(thing);
                        }
                        break;
                    }
                    case TCls_Creature:
                    case TCls_DeadCreature:
                    {
                        move_creature_to_nearest_valid_position(thing);
                        break;
                    }
                    case TCls_EffectElem:
                    {
                        delete_thing_structure(thing, 0);
                        removed_num++;
                        break;
                    }
                    case TCls_Trap:
                    {
                        removed_num += remove_trap(thing, NULL);
                        break;
                    }
                    default:
                    {
                        break;
                    }
                    break;
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

unsigned long remove_unwanted_things_from_floor_slab(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    SubtlCodedCoords stl_num = get_subtile_number_at_slab_center(slb_x, slb_y);
    struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
    unsigned long removed_num = 0;
    for (long n=0; n < AROUND_MAP_LENGTH; n++)
    {
        struct Map *mapblk = get_map_block_at_pos(stl_num+game.around_map[n]);
        unsigned long k = 0;
        long i = get_mapwho_thing_index(mapblk);
        while (i != 0)
        {
            struct Thing *thing = thing_get(i);
            if (thing_is_invalid(thing))
            {
                WARNLOG("Jump out of things array");
                break;
            }
            i = thing->next_on_mapblk;
            // Per thing code
            switch(thing->class_id)
            {
                case TCls_Door:
                {
                    // using destroy_door places claimed path, which we don't want
                    remove_key_on_door(thing);
                    delete_thing_structure(thing, 0);
                    removed_num++;
                    break;
                }
                case TCls_Trap:
                {
                    if (thing->owner != slabmap_owner(slb))
                    {
                        removed_num += remove_trap(thing, NULL);
                    }
                    break;
                }
                default:
                {
                    break;
                }
                break;
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
    return removed_num;
}

static void delete_attached_things_on_slab(long slb_x, long slb_y)
{
    MapSubtlCoord stl_x = slab_subtile(slb_x,-1);
    MapSubtlCoord stl_y = slab_subtile(slb_y,-1);

    unsigned long k = 0;
    for (MapSubtlCoord y = stl_y; y < stl_y+STL_PER_SLB+2; y++)
    {
        for (MapSubtlCoord x = stl_x; x < stl_x+STL_PER_SLB+2; x++)
        {
            struct Map *mapblk = get_map_block_at(x,y);
            if (mapblk == INVALID_MAP_BLOCK)
            {
                continue;
            }
            struct Thing *thing = thing_get(get_mapwho_thing_index(mapblk));
            if ( !thing_is_invalid(thing))
            {
                struct Thing *next_thing;
                do
                {
                    next_thing = thing_get(thing->next_on_mapblk);
                    if (thing->parent_idx == get_slab_number(slb_x,slb_y))
                    {
                        char class_id = thing->class_id;
                        if (class_id == TCls_Object || class_id == TCls_EffectGen)
                            delete_thing_structure(thing, 0);
                    }
                    thing = next_thing;
                    k++;
                    if (k > THINGS_COUNT)
                    {
                        ERRORLOG("Infinite loop detected when sweeping things list");
                        break_mapwho_infinite_chain(mapblk);
                        break;
                    }
                } while (!thing_is_invalid(next_thing));
            }
        }
    }
}

static TbBool get_against(PlayerNumber agnst_plyr_idx, SlabKind agnst_slbkind, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
    if (slabmap_block_invalid(slb)) {
        return 1;
    }
    struct SlabConfigStats *slabst = get_slab_stats(slb);
    struct SlabConfigStats *agnst_slabst = get_slab_kind_stats(agnst_slbkind);
    return (slabst->slb_id != agnst_slabst->slb_id)
    || ((slabmap_owner(slb) != agnst_plyr_idx) && ((slabmap_owner(slb) != game.neutral_player_num) || (slb->kind == SlbT_CLAIMED) ));
}

void delete_column(ColumnIndex col_idx)
{
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
    if (((col->bitfields & CLF_ACTIVE) == 0) && (col->use <= 0)) {
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
        if (col.cubes[i] > 0)
        {
            struct CubeConfigStats* cubed = get_cube_model_stats(col.cubes[i]);
            if (cubed->ownershipGroup > 0) {
                found = true;
                struct SlabMap *slb;
                slb = get_slabmap_for_subtile(stl_x, stl_y);
                col.cubes[i] = game.conf.cube_conf.cube_bits[cubed->ownershipGroup][get_player_color_idx(slabmap_owner(slb))];
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

void set_alt_bit_based_on_slab(SlabKind slbkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct SlabConfigStats *slabst;
    slabst = get_slab_kind_stats(slbkind);

    unsigned short sibling_flags;
    unsigned short edge_flags;
    unsigned long wibble;

    sibling_flags = 0;
    edge_flags = 0;
    wibble = slabst->wibble;
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

void place_slab_columns(SlabKind slbkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y, const ColumnIndex *col_idx)
{
    struct SlabConfigStats *slabst;
    slabst = get_slab_kind_stats(slbkind);
    if (slabst->wlb_type != WlbT_Bridge)
    {
        struct SlabMap *slb;
        slb = get_slabmap_for_subtile(stl_x, stl_y);
        slabmap_set_wlb(slb, slabst->wlb_type);
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
            int column_index_check;
            column_index_check = -*colid;
            if ( column_index_check < 0 )
              ERRORLOG("BBlocks instead of columns");
            update_map_collide(slbkind, stl_x+dx, stl_y+dy);
            set_alt_bit_based_on_slab(slbkind, stl_x+dx, stl_y+dy);
            colid++;
        }
    }
}

#define get_slabset_index(slbkind, style, pick) get_slabset_index_f(slbkind, style, pick, __func__)
unsigned short get_slabset_index_f(SlabKind slbkind, unsigned char style, unsigned char pick, const char *func_name)
{
    if (slbkind >= game.conf.slab_conf.slab_types_count) {
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
    return SLABSETS_PER_SLAB * slbkind + 9 * style + pick;
}

void place_slab_object(SlabCodedCoords slb_num, MapSubtlCoord stl_x,MapSubtlCoord stl_y, unsigned short slabset_id, unsigned short stl_id, PlayerNumber plyr_idx)
{
    if (slabset_id >= SLABSET_COUNT) {
        ERRORLOG("Illegal animating slab number: %d", (int)slabset_id);
        return;
    }
    short sobj_idx;
    sobj_idx = game.slabobjs_idx[slabset_id];
    if (sobj_idx < 0) {
        return;
    }
    for (; sobj_idx < game.slabobjs_num; sobj_idx++)
    {
        struct SlabObj *sobj;
        sobj = &game.slabobjs[sobj_idx];
        if (sobj->slabset_id != slabset_id) {
            break;
        }
        if (sobj->stl_id != stl_id) {
            continue;
        }
        struct Coord3d pos;
        pos.x.val = (stl_x << 8) + sobj->offset_x;
        pos.y.val = (stl_y << 8) + sobj->offset_y;
        pos.z.val = sobj->offset_z;
        struct Map *mapblk;
        mapblk = get_map_block_at(coord_subtile(pos.x.val), coord_subtile(pos.y.val));
        if ((mapblk->flags & SlbAtFlg_Blocking) == 0)
        {
            if (sobj->isLight == 1)
            {
                struct InitLight ilght;
                memset(&ilght,0,sizeof(struct InitLight));
                ilght.mappos.x.val = pos.x.val;
                ilght.mappos.y.val = pos.y.val;
                ilght.mappos.z.val = pos.z.val;
                ilght.radius = sobj->range * COORD_PER_STL;
                ilght.intensity = sobj->model;
                ilght.flags = 0;
                ilght.is_dynamic = 0;
                long lgt_id;
                lgt_id = light_create_light(&ilght);
                if (lgt_id != 0) {
                    struct Light *lgt;
                    lgt = &game.lish.lights[lgt_id];
                    lgt->attached_slb = slb_num;
                } else {
                    WARNLOG("Cannot allocate light");
                    continue;
                }
            } else if (sobj->isLight == 0)
            {
                if (sobj->class_id == TCls_Object)
                {
                    ThingModel tngmodel;
                    tngmodel = sobj->model;

                    ThingModel base_model = get_coloured_object_base_model(tngmodel);
                    if(base_model != 0)
                    {
                        tngmodel = get_player_colored_object_model(base_model,plyr_idx);
                    }
                    if (tngmodel <= 0)
                        continue;

                    TbBool needs_object;
                    int icorn;
                    int nfilled;
                    int nprison;

                    if ((tngmodel == ObjMdl_PrisonBar) && (stl_id != 4))
                    {
                        MapSlabCoord slb_x;
                        MapSlabCoord slb_y;
                        slb_x = subtile_slab(stl_x);
                        slb_y = subtile_slab(stl_y);
                        nprison = 0;
                        nfilled = 0;
                        if ((stl_id & 1) != 0)
                        {
                            const struct SlabMap *slb;
                            slb = get_slabmap_block(slb_x + my_around_nine[stl_id].delta_x, slb_y + my_around_nine[stl_id].delta_y);
                            const struct SlabConfigStats *slabst;
                            slabst = get_slab_stats(slb);
                            needs_object = ((slabst->block_flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) == 0);
                        } else
                        {
                          icorn = slab_element_to_corner[stl_id];
                          if (icorn != -1)
                          {
                              struct SlabMap *slb;
                              slb = get_slabmap_block(slb_x + my_around_eight[icorn].delta_x, slb_y + my_around_eight[icorn].delta_y);
                              const struct SlabConfigStats *slabst;
                              slabst = get_slab_stats(slb);
                              if ((slabst->block_flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0)
                                  nfilled++;
                              if (slb->kind == SlbT_PRISON)
                                  nprison++;
                              slb = get_slabmap_block(slb_x + my_around_eight[(icorn + 2) & 7].delta_x, slb_y + my_around_eight[(icorn + 2) & 7].delta_y);
                              slabst = get_slab_stats(slb);
                              if ((slabst->block_flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0)
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
                    objtng = create_object(&pos, tngmodel, plyr_idx, slb_num);
                    if (thing_is_invalid(objtng))
                    {
                        ERRORLOG("Cannot create object type %d", tngmodel);
                        continue;
                    }
                    if (thing_is_dungeon_heart(objtng))
                    {
                        struct Dungeon* dungeon = get_dungeon(objtng->owner);
                        if (dungeon->backup_heart_idx == 0)
                        {
                            dungeon->backup_heart_idx = objtng->index;
                        }
                    }
                } else
                if (sobj->class_id == TCls_EffectGen)
                {
                    struct Thing *effgentng;
                    effgentng = create_effect_generator(&pos, sobj->model, (sobj->range * COORD_PER_STL), plyr_idx, slb_num);
                    if (thing_is_invalid(effgentng)) {
                        ERRORLOG("Cannot create effect generator, type %d", sobj->model);
                        continue;
                    }
                } else
                {
                    ERRORLOG("Stupid thing class %d", (int)sobj->class_id);
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
                struct SlabConfigStats *slabst;
                slabst = get_slab_stats(slb);
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
                if (slabst->category == SlbAtCtg_RoomInterior)
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
    if (end_stl_x >= game.map_subtiles_x)
      end_stl_x = game.map_subtiles_x;
    end_stl_y = 3 * slb_y + 4;
    if (end_stl_y >= game.map_subtiles_y)
      end_stl_y = game.map_subtiles_y;
    {
        long i;
        unsigned long k;
        i = game.thing_lists[TngList_StaticLights].index;
        k = 0;
        while (i > 0)
        {
            struct Light *lgt;
            lgt = &game.lish.lights[i];
            i = lgt->next_in_list;
            // Per-light code
            int lgtstl_x;
            int lgtstl_y;
            lgtstl_x = lgt->mappos.x.stl.num;
            lgtstl_y = lgt->mappos.y.stl.num;
            if (lgt->attached_slb == place_slbnum)
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
            struct SlabConfigStats *slabst;
            slabst = get_slab_stats(slb);
            style_val = slabst->fill_style;
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
    struct SlabMap* slb;
    struct SlabConfigStats* slabst;
    SlabKind undecorated_slbkind;
    unsigned short torch_flags;
    if (slbkind == SlbT_TORCHDIRT) {
        undecorated_slbkind = SlbT_EARTH;
    }
    else {
        undecorated_slbkind = slbkind + 4;
    }
    torch_flags = torch_flags_for_slab(slb_x, slb_y);
    if ((torch_flags & 0x01) != 0)
    {
        slab_type_list[3] = undecorated_slbkind;
        slab_type_list[5] = undecorated_slbkind;
        if ((slb_y + slb_x) & 1)
        {
            slb = get_slabmap_block(slb_x, slb_y-1);
            slabst = get_slab_stats(slb);
            if (slabst->category != SlbAtCtg_RoomInterior)
            {
                slab_type_list[1] = undecorated_slbkind;
            }
        }
        else
        {
            slb = get_slabmap_block(slb_x, slb_y+1);
            slabst = get_slab_stats(slb);
            if (slabst->category != SlbAtCtg_RoomInterior)
            {
                slab_type_list[7] = undecorated_slbkind;
            }
        }
    } else
    if ((torch_flags & 0x02) != 0)
    {
        slab_type_list[1] = undecorated_slbkind;
        slab_type_list[7] = undecorated_slbkind;
        if ((slb_y + slb_x) & 1)
        {
            slb = get_slabmap_block(slb_x-1, slb_y);
            slabst = get_slab_stats(slb);
            if (slabst->category != SlbAtCtg_RoomInterior)
            {
                slab_type_list[3] = undecorated_slbkind;
            }
        }
        else
        {
            slb = get_slabmap_block(slb_x+1, slb_y);
            slabst = get_slab_stats(slb);
            if (slabst->category != SlbAtCtg_RoomInterior)
            {
                slab_type_list[5] = undecorated_slbkind;
            }
        }
    }
}

void place_single_slab_prepare_column_index(SlabKind slbkind, MapSlabCoord slb_x, MapSlabCoord slb_y,
    PlayerNumber plyr_idx, short *slab_type_list, short *room_pretty_list, short *style_set, short *slab_number_list, ColumnIndex *col_idx)
{
    struct SlabConfigStats *place_slabst = get_slab_kind_stats(slbkind);
    unsigned char against = 0;
    signed short primitiv;
    int i;
    // Test non diagonal neighbours
    for (i=0; i < AROUND_EIGHT_LENGTH; i+=2)
    {
        MapSlabCoord sslb_x = slb_x + (MapSlabCoord)my_around_eight[i].delta_x;
        MapSlabCoord sslb_y = slb_y + (MapSlabCoord)my_around_eight[i].delta_y;
        // Collecting bitmask
        against <<= 1;
        against |= get_against(plyr_idx, slbkind, sslb_x, sslb_y);
    }
    i = 0;
    int slabset_id;
    if ( against )
    {
        primitiv = slab_primitive[against];
        if (primitiv == -1)
        {
            const unsigned char *specase = against_to_case[against];
            if (specase != NULL)
            {
                for (i=0; i < STL_PER_SLB*STL_PER_SLB; i++)
                {
                    slabset_id = get_slabset_index(slab_type_list[i], style_set[i] + room_pretty_list[i], specase[i]);
                    slab_number_list[i] = slabset_id;
                    struct SlabSet *sset = &game.slabset[slabset_id];
                    col_idx[i] = sset->col_idx[i];
                }
            }
            else
            {
                ERRORLOG("Illegal special case!");
            }
        } else
        {
            for (i=0; i < STL_PER_SLB*STL_PER_SLB; i++)
            {
                slabset_id = get_slabset_index(slab_type_list[i], style_set[i] + room_pretty_list[i], primitiv);
                slab_number_list[i] = slabset_id;
                struct SlabSet *sset = &game.slabset[slabset_id];
                col_idx[i] = sset->col_idx[i];
            }
        }
    } else
    if (place_slabst->category == SlbAtCtg_RoomInterior)
    {
        // Test diagonal neighbours
        for (i=1; i < AROUND_EIGHT_LENGTH; i+=2)
        {
            MapSlabCoord sslb_x = slb_x + (MapSlabCoord)my_around_eight[i].delta_x;
            MapSlabCoord sslb_y = slb_y + (MapSlabCoord)my_around_eight[i].delta_y;
            // Collecting bitmask
            against <<= 1;
            against |= get_against(plyr_idx, slbkind, sslb_x, sslb_y);
        }
        if ( against )
        {
            const unsigned char *specase;
            specase = special_cases[6];
            for (i=0; i < STL_PER_SLB*STL_PER_SLB; i++)
            {
                slabset_id = get_slabset_index( slab_type_list[i], style_set[i], specase[i]);
                slab_number_list[i] = slabset_id;
                struct SlabSet *sset = &game.slabset[slabset_id];
                col_idx[i] = sset->col_idx[i];
            }
        } else
        {
            for (i=0; i < STL_PER_SLB*STL_PER_SLB; i++)
            {
                slabset_id = get_slabset_index( slab_type_list[i], 3, 0);
                slab_number_list[i] = slabset_id;
                struct SlabSet *sset = &game.slabset[slabset_id];
                col_idx[i] = sset->col_idx[i];
            }
        }
    } else
    {
        for (i=0; i < STL_PER_SLB*STL_PER_SLB; i++)
        {
            slabset_id = get_slabset_index( slab_type_list[i], 3, 0);
            slab_number_list[i] = slabset_id;
            struct SlabSet *sset = &game.slabset[slabset_id];
            col_idx[i] = sset->col_idx[i];
        }
    }
}

void place_single_slab_modify_column_near_liquid(SlabKind slbkind, MapSlabCoord slb_x, MapSlabCoord slb_y,
    PlayerNumber plyr_idx, short *slab_type_list, short *room_pretty_list, short *style_set, short *slab_number_list, ColumnIndex *col_idx)
{
    int neigh;
    int slabset_id;
    int i;
    for (i=0; i < AROUND_EIGHT_LENGTH; i+=2)
    {
        MapSlabCoord sslb_x;
        MapSlabCoord sslb_y;
        sslb_x = slb_x + (MapSlabCoord)my_around_eight[(i-1)&7].delta_x;
        sslb_y = slb_y + (MapSlabCoord)my_around_eight[(i-1)&7].delta_y;
        struct SlabMap *slb;
        slb = get_slabmap_block(sslb_x,sslb_y);
        struct SlabConfigStats *slabsta;
        slabsta = get_slab_stats(slb);
        if ((slabsta->category == SlbAtCtg_FortifiedGround) || (slabsta->category == SlbAtCtg_RoomInterior) ||
            (slabsta->category == SlbAtCtg_Obstacle) || (slb->kind == SlbT_WATER) || (slb->kind == SlbT_LAVA))
        {
            sslb_x = slb_x + (MapSlabCoord)my_around_eight[(i-2)&7].delta_x;
            sslb_y = slb_y + (MapSlabCoord)my_around_eight[(i-2)&7].delta_y;
            slb = get_slabmap_block(sslb_x,sslb_y);
            struct SlabConfigStats *slabstb;
            slabstb = get_slab_stats(slb);
            if (slabstb->category == SlbAtCtg_FortifiedWall)
            {
                sslb_x = slb_x + (MapSlabCoord)my_around_eight[(i)&7].delta_x;
                sslb_y = slb_y + (MapSlabCoord)my_around_eight[(i)&7].delta_y;
                slb = get_slabmap_block(sslb_x,sslb_y);
                struct SlabConfigStats *slabstc;
                slabstc = get_slab_stats(slb);
                if (slabstc->category == SlbAtCtg_FortifiedWall)
                {
                  neigh = 4 + slab_element_around_eight[(i-1)&7];
                  slab_type_list[neigh] = SlbT_WALLWTWINS;
                  slabset_id = get_slabset_index(slab_type_list[neigh], style_set[neigh], 8);
                  slab_number_list[neigh] = slabset_id;
                  struct SlabSet *sset;
                  sset = &game.slabset[slabset_id];
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
    struct SlabConfigStats *place_slabst = get_slab_kind_stats(slbkind);
    if (place_slabst->category == SlbAtCtg_FortifiedWall) {
        place_single_slab_fill_arrays_std(slb_x, slb_y, slab_type_list, room_pretty_list);
    }
    delete_attached_things_on_slab(slb_x, slb_y);
    delete_attached_lights_on_slab(slb_x, slb_y);

    ColumnIndex col_idx[STL_PER_SLB*STL_PER_SLB];
    {
        int slabset_id = get_slabset_index(slbkind, 3, 0);
        struct SlabSet *sset = &game.slabset[slabset_id];
        for (i=0; i < STL_PER_SLB*STL_PER_SLB; i++)
        {
            col_idx[i] = sset->col_idx[i];
        }
    }
    place_single_slab_fill_style_array(slb_x, slb_y, style_set);

    if (slab_kind_has_torches(slbkind))
    {
        place_single_slab_set_torch_places(slbkind, slb_x, slb_y, slab_type_list);
    }
    place_single_slab_prepare_column_index(slbkind, slb_x, slb_y, plyr_idx, slab_type_list, room_pretty_list, style_set, slab_number_list, col_idx);
    if (place_slabst->category == SlbAtCtg_FortifiedWall)
    {
        place_single_slab_modify_column_near_liquid(slbkind, slb_x, slb_y, plyr_idx, slab_type_list, room_pretty_list, style_set, slab_number_list, col_idx);
    }

    {
        struct SlabMap *slb = get_slabmap_block(slb_x,slb_y);
        slb->health = game.block_health[place_slabst->block_health_index];
    }
    place_slab_columns(slbkind, STL_PER_SLB * slb_x, STL_PER_SLB * slb_y, col_idx);
    place_slab_objects(slb_x, slb_y, slab_number_list, plyr_idx);
}


static void shuffle_unattached_things_on_slab(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct Thing *next_thing;
    int persistence;
    unsigned long k = 0;

    MapSubtlCoord start_stl_x = slab_subtile(slb_x, 0);
    MapSubtlCoord start_stl_y = slab_subtile(slb_y, 0);

    for (MapSubtlCoord stl_x = start_stl_x; stl_x < start_stl_x + STL_PER_SLB; stl_x++)
    {
        for (MapSubtlDelta stl_y = start_stl_y; stl_y < start_stl_y + STL_PER_SLB; stl_y++)
        {
            struct Map *mapblk = get_map_block_at(stl_x, stl_y);
            struct Thing *thing = thing_get(get_mapwho_thing_index(mapblk));
            while (!thing_is_invalid(thing))
            {
                next_thing = thing_get(thing->next_on_mapblk);
                if (thing->parent_idx != get_slab_number(slb_x,slb_y))
                {
                    TbBool delete_thing = true;
                    if (thing_is_object(thing))
                    {
                        struct ObjectConfigStats* objst = get_object_model_stats(thing->model);

                        persistence = objst->persistence;
                        if (persistence == ObPer_Move)
                        {
                            if ((get_map_floor_filled_subtiles(mapblk) <= 4) || move_object_to_nearest_free_position(thing))
                            {
                                delete_thing = false;
                            }
                        }
                        else if (persistence != ObPer_Persist)
                        {
                            delete_thing = false;
                        }
                    }
                    else if (thing->class_id != TCls_EffectGen)
                    {
                        delete_thing = false;
                    }
                    if (delete_thing)
                    {
                        delete_thing_structure(thing, 0);
                    }
                }
                thing = next_thing;
                k++;
                if (k > THINGS_COUNT)
                {
                    ERRORLOG("Infinite loop detected when sweeping things list");
                    break_mapwho_infinite_chain(mapblk);
                    break;
                }
            }
        }
    }
}

void set_alt_bit_on_slabs_around(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    for (int i = 0; i < AROUND_EIGHT_LENGTH; i++)
    {
        MapSlabCoord sslb_x;
        MapSlabCoord sslb_y;
        sslb_x = slb_x + (MapSlabCoord)my_around_eight[i].delta_x;
        sslb_y = slb_y + (MapSlabCoord)my_around_eight[i].delta_y;
        struct SlabMap* slb;
        slb = get_slabmap_block(sslb_x, sslb_y);
        if (slabmap_block_invalid(slb)) {
            continue;
        }
        int ssub_x;
        int ssub_y;
        for (ssub_y = 0; ssub_y < STL_PER_SLB; ssub_y++)
        {
            for (ssub_x = 0; ssub_x < STL_PER_SLB; ssub_x++)
            {
                MapSubtlCoord sstl_x;
                MapSubtlCoord sstl_y;
                sstl_x = slab_subtile(sslb_x, ssub_x);
                sstl_y = slab_subtile(sslb_y, ssub_y);
                set_alt_bit_based_on_slab(slb->kind, sstl_x, sstl_y);
            }
        }
    }
}

void dump_slab_on_map(SlabKind slbkind, long slabset_id, MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber owner)
{
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    slb_x = subtile_slab(stl_x);
    slb_y = subtile_slab(stl_y);
    MapSubtlCoord stl_xa;
    MapSubtlCoord stl_ya;
    stl_xa = STL_PER_SLB * slb_x;
    stl_ya = STL_PER_SLB * slb_y;
    if (slabset_id >= SLABSET_COUNT) {
        ERRORLOG("Illegal animating slab number: %ld", slabset_id);
        slabset_id = 0;
    }
    struct SlabConfigStats *slabst;
    slabst = get_slab_kind_stats(slbkind);
    struct SlabMap *slb;
    slb = get_slabmap_block(slb_x, slb_y);
    slb->health = game.block_health[slabst->block_health_index];
    struct SlabSet *sset;
    sset = &game.slabset[slabset_id];
    place_slab_columns(slbkind, stl_xa, stl_ya, sset->col_idx);
    set_slab_owner(slb_x, slb_y, owner);

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
                    //Suspect this has to do with object 2, torch, that needs to stay 4 cubes heigh. TCls_Door added later for braced door(model 2) key bug.
                    //TODO investigate if doors need height 1 or 5, below code drops them to the floor
                    if ((thing->model != 2) || (thing->class_id == TCls_Door))
                    {
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

            place_slab_object(place_slbnum, sstl_x, sstl_y, slabset_id, n, slabmap_owner(slb));
            n++;
        }
    }

    slb = get_slabmap_block(slb_x, slb_y);
    slb->kind = slbkind;
    panel_map_update(stl_xa, stl_ya, STL_PER_SLB, STL_PER_SLB);
    if (slab_kind_is_animated(slbkind) && !slab_kind_is_door(slbkind))
    {
        MapSubtlCoord stl_xb;
        MapSubtlCoord stl_yb;
        stl_yb = stl_ya + STL_PER_SLB - 1;
        if (stl_yb > game.map_subtiles_y)
            stl_yb = game.map_subtiles_y;
        stl_xb = stl_xa + STL_PER_SLB - 1;
        if (stl_xb > game.map_subtiles_x)
            stl_xb = game.map_subtiles_x;
        update_blocks_in_area(stl_xa, stl_ya, stl_xb, stl_yb);
    }
}

void place_animating_slab_type_on_map(SlabKind slbkind, char ani_frame, MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber owner)
{
    SYNCDBG(7,"Starting");
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    slb_x = subtile_slab(stl_x);
    slb_y = subtile_slab(stl_y);
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
    dump_slab_on_map(slbkind, SLABSETS_PER_SLAB * slbkind + ani_frame, stl_x, stl_y, owner);
    shuffle_unattached_things_on_slab(slb_x, slb_y);
    set_alt_bit_on_slabs_around(slb_x, slb_y);
    if (slbkind == SlbT_GEMS)
    {
        remove_unwanted_things_from_wall_slab(slb_x, slb_y);
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
            struct SlabConfigStats *slabst;
            slb = get_slabmap_block(slb_x,slb_y);
            if (slabmap_block_invalid(slb))
                continue;
            slabst = get_slab_stats(slb);
            if ((slabst->category == SlbAtCtg_FortifiedGround) || (slabst->category == SlbAtCtg_RoomInterior) || (slabst->category == SlbAtCtg_Obstacle))
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

void place_slab_type_on_map_f(SlabKind nslab, MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber owner, unsigned char keep_blocks_around,const char *func_name)
{
    SlabKind previous_slab_types_around[8];
    struct SlabMap *slb;
    struct SlabConfigStats *slabst;
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    MapSlabCoord spos_x;
    MapSlabCoord spos_y;
    int skind;
    long i;
    SYNCDBG(7,"%s: Starting for (%d,%d)",func_name,(int)stl_x,(int)stl_y);
    if (subtile_coords_invalid(stl_x, stl_y))
        return;
    slb_x = subtile_slab(stl_x);
    slb_y = subtile_slab(stl_y);
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

    set_slab_owner(slb_x, slb_y, owner);
    place_single_slab_type_on_map(skind, slb_x, slb_y, owner);
    shuffle_unattached_things_on_slab(slb_x, slb_y);

    slabst = get_slab_kind_stats(skind);
    if ((slabst->category == SlbAtCtg_RoomInterior) || (slabst->category == SlbAtCtg_FortifiedGround))
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

    panel_map_update(slab_subtile(slb_x,0), slab_subtile(slb_y,0), STL_PER_SLB, STL_PER_SLB);

    for (i = 0; i < AROUND_EIGHT_LENGTH; i++)
    {
        spos_x = slb_x + (MapSlabCoord)my_around_eight[i].delta_x;
        spos_y = slb_y + (MapSlabCoord)my_around_eight[i].delta_y;
        slb = get_slabmap_block(spos_x,spos_y);
        if (slabmap_block_invalid(slb))
            continue;
        slabst = get_slab_kind_stats(slb->kind);
        if ((previous_slab_types_around[i] != slb->kind)
          || ((slabst->category != SlbAtCtg_Obstacle) && (slabst->category != SlbAtCtg_Unclaimed))
          || (game.game_kind == GKind_NonInteractiveState))
        {
            place_single_slab_type_on_map(slb->kind, spos_x, spos_y, slabmap_owner(slb));
        }
    }

    if (!keep_blocks_around)
      update_blocks_around_slab(slb_x,slb_y);
    switch (nslab)
    {
        case SlbT_ROCK:
        case SlbT_GOLD:
        case SlbT_DENSEGOLD:
        case SlbT_EARTH:
        case SlbT_TORCHDIRT:
        case SlbT_WALLDRAPE:
        case SlbT_WALLTORCH:
        case SlbT_WALLWTWINS:
        case SlbT_WALLWWOMAN:
        case SlbT_WALLPAIRSHR:
        case SlbT_DAMAGEDWALL:
        case SlbT_GEMS:
        case SlbT_ENTRANCE_WALL:
        case SlbT_TREASURE_WALL:
        case SlbT_LIBRARY_WALL:
        case SlbT_PRISON_WALL:
        case SlbT_TORTURE_WALL:
        case SlbT_TRAINING_WALL:
        case SlbT_DUNGHEART_WALL:
        case SlbT_WORKSHOP_WALL:
        case SlbT_SCAVENGER_WALL:
        case SlbT_TEMPLE_WALL:
        case SlbT_GRAVEYARD_WALL:
        case SlbT_GARDEN_WALL:
        case SlbT_LAIR_WALL:
        case SlbT_BARRACKS_WALL:
            remove_unwanted_things_from_wall_slab(slb_x, slb_y);
            break;
        case SlbT_LAVA:
            delete_unwanted_things_from_liquid_slab(slb_x, slb_y, TngEff_HarmlessGas2);
            break;
        case SlbT_WATER:
            delete_unwanted_things_from_liquid_slab(slb_x, slb_y, TngEff_Drip3);
            break;
        case SlbT_PATH:
        case SlbT_CLAIMED:
        case SlbT_PURPLE:
            remove_unwanted_things_from_floor_slab(slb_x, slb_y);
            break;
        default:
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
    for (y = stl_y; y < stl_y+STL_PER_SLB; y++)
    {
        for (x = stl_x; x < stl_x+STL_PER_SLB; x++)
        {
            z = get_floor_filled_subtiles_at(x, y);
            if (z > 0)
            {
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
    slb_x = subtile_slab(stl_x);
    slb_y = subtile_slab(stl_y);
    create_gold_rubble_for_dug_slab(slb_x, slb_y);
    all_players_untag_blocks_for_digging_in_area(slb_x, slb_y);
    replace_map_slab_when_destroyed(slb_x, slb_y);
    do_slab_efficiency_alteration(slb_x, slb_y);
    // Gold slabs are normally visible to all players,
    // so sine we're destroying it - make it invisible
    // TODO MAP Maybe it should be cleared only if sibling non-gold and non-rock slabs are invisible
    set_slab_explored_flags(plyr_idx, slb_x, slb_y);
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
            struct SlabConfigStats *slabst;
            slb = get_slabmap_for_subtile(stl_x, stl_y);
            slabst = get_slab_stats(slb);
            if ((slabst->block_flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) == 0) {
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
    for (y = stl_y; y < stl_y+STL_PER_SLB; y++)
    {
        for (x = stl_x; x < stl_x+STL_PER_SLB; x++)
        {
            z = get_floor_filled_subtiles_at(x, y);
            if (z > 0)
            {
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
    slb_x = subtile_slab(stl_x);
    slb_y = subtile_slab(stl_y);
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


void set_explored_around(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx)
{
    struct SlabMap* slb;
    slb = get_slabmap_block(slb_x + 1, slb_y);
    if (!slabmap_block_invalid(slb))
    {
        set_slab_explored(plyr_idx, slb_x + 1, slb_y);
    }
    slb = get_slabmap_block(slb_x - 1, slb_y);
    if (!slabmap_block_invalid(slb))
    {
        set_slab_explored(plyr_idx, slb_x - 1, slb_y);
    }
    slb = get_slabmap_block(slb_x, slb_y + 1);
    if (!slabmap_block_invalid(slb))
    {
        set_slab_explored(plyr_idx, slb_x, slb_y + 1);
    }
    slb = get_slabmap_block(slb_x, slb_y - 1);
    if (!slabmap_block_invalid(slb))
    {
        set_slab_explored(plyr_idx, slb_x, slb_y - 1);
    }
}

void clear_dig_and_set_explored_around(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx)
{
    struct SlabMap *slb;
    slb = get_slabmap_block(slb_x+1, slb_y);
    if (!slabmap_block_invalid(slb))
    {
        struct SlabConfigStats *slabst;
        slabst = get_slab_stats(slb);
        if ((slabst->block_flags & SlbAtFlg_IsDoor) != 0)
        {
            clear_slab_dig(slb_x+1, slb_y, plyr_idx);
            set_slab_explored(plyr_idx, slb_x+1, slb_y);
        }
    }
    slb = get_slabmap_block(slb_x-1, slb_y);
    if (!slabmap_block_invalid(slb))
    {
        struct SlabConfigStats *slabst;
        slabst = get_slab_stats(slb);
        if ((slabst->block_flags & SlbAtFlg_IsDoor) != 0)
        {
            clear_slab_dig(slb_x-1, slb_y, plyr_idx);
            set_slab_explored(plyr_idx, slb_x-1, slb_y);
        }
    }
    slb = get_slabmap_block(slb_x, slb_y+1);
    if (!slabmap_block_invalid(slb))
    {
        struct SlabConfigStats *slabst;
        slabst = get_slab_stats(slb);
        if ((slabst->block_flags & SlbAtFlg_IsDoor) != 0)
        {
            clear_slab_dig(slb_x, slb_y+1, plyr_idx);
            set_slab_explored(plyr_idx, slb_x, slb_y+1);
        }
    }
    slb = get_slabmap_block(slb_x, slb_y-1);
    if (!slabmap_block_invalid(slb))
    {
        struct SlabConfigStats *slabst;
        slabst = get_slab_stats(slb);
        if ((slabst->block_flags & SlbAtFlg_IsDoor) != 0)
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
        if ((delta_see + slb_x < 0) || (delta_see + slb_x >= game.map_tiles_x)) {
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
            struct SlabConfigStats *slabst;
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
            if ((lslb_y < 0) || (hslb_y >= game.map_tiles_y))
                continue;
            if ( go_dir1 )
            {
                if (delta_shift > 0)
                {
                    slb = get_slabmap_block(hslb_x-1, lslb_y);
                    slabst = get_slab_stats(slb);
                    if ((slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0)
                    {
                        slb = get_slabmap_block(hslb_x, lslb_y+1);
                        slabst = get_slab_stats(slb);
                        if ((slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0) {
                            allow_next_dir1 = 1;
                            go_dir1 = 0;
                        }
                    }
                }
                else
                if (delta_shift < 0)
                {
                    slb = get_slabmap_block(hslb_x+1, lslb_y);
                    slabst = get_slab_stats(slb);
                    if ((slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0)
                    {
                        slb = get_slabmap_block(hslb_x, lslb_y+1);
                        slabst = get_slab_stats(slb);
                        if ((slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0) {
                            allow_next_dir1 = 1;
                            go_dir1 = 0;
                        }
                    }
                }
                if ( go_dir1 )
                {
                  clear_slab_dig(hslb_x, lslb_y, plyr_idx);
                  slb = get_slabmap_block(hslb_x, lslb_y);
                  slabst = get_slab_stats(slb);
                  if (go_dir1 || (slabst->block_flags & SlbAtFlg_Blocking)) {
                      set_slab_explored(plyr_idx, hslb_x, lslb_y);
                  }
                  if (slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) {
                      go_dir1 = 0;
                  }
                }
                else
                if ( allow_next_dir1 )
                {
                    allow_next_dir1 = 0;
                    slb = get_slabmap_block(hslb_x, lslb_y);
                    slabst = get_slab_stats(slb);
                    if (slabst->block_flags & SlbAtFlg_Filled)
                    {
                      clear_slab_dig(hslb_x, lslb_y, plyr_idx);
                      if (go_dir1 || (slabst->block_flags & SlbAtFlg_Blocking)) {
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
                  slabst = get_slab_stats(slb);
                  if (slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable))
                  {
                      slb = get_slabmap_block(hslb_x-1, hslb_y);
                      slabst = get_slab_stats(slb);
                      if (slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) {
                        allow_next_dir2 = 1;
                        go_dir2 = 0;
                      }
                  }
              }
              else
              if (delta_shift < 0)
              {
                  slb = get_slabmap_block(hslb_x, hslb_y-1);
                  slabst = get_slab_stats(slb);
                  if (slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable))
                  {
                      slb = get_slabmap_block(hslb_x+1, hslb_y);
                      slabst = get_slab_stats(slb);
                      if (slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) {
                        allow_next_dir2 = 1;
                        go_dir2 = 0;
                      }
                  }
              }
              if ( go_dir2 )
              {
                  clear_slab_dig(hslb_x, hslb_y, plyr_idx);
                  slb = get_slabmap_block(hslb_x, hslb_y);
                  slabst = get_slab_stats(slb);
                  if (go_dir2 || (slabst->block_flags & SlbAtFlg_Blocking)) {
                      set_slab_explored(plyr_idx, hslb_x, hslb_y);
                  }
                  if (slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) {
                      go_dir2 = 0;
                  }
              }
              else
              if ( allow_next_dir2 )
              {
                  allow_next_dir2 = 0;
                  slb = get_slabmap_block(hslb_x, hslb_y);
                  slabst = get_slab_stats(slb);
                  if (slabst->block_flags & SlbAtFlg_Filled)
                  {
                      clear_slab_dig(hslb_x, hslb_y, plyr_idx);
                      slb = get_slabmap_block(hslb_x, hslb_y);
                      slabst = get_slab_stats(slb);
                      if (go_dir2 || (slabst->block_flags & SlbAtFlg_Blocking)) {
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
        if ((delta_see + slb_y < 0) || (delta_see + slb_y >= game.map_tiles_y)) {
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
            struct SlabConfigStats *slabst;
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
            if ((lslb_x < 0) || (hslb_x >= game.map_tiles_x))
                continue;
            if ( go_dir1 )
            {
                if (delta_shift > 0)
                {
                    slb = get_slabmap_block(lslb_x, hslb_y-1);
                    slabst = get_slab_stats(slb);
                    if ((slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0)
                    {
                        slb = get_slabmap_block(lslb_x+1, hslb_y);
                        slabst = get_slab_stats(slb);
                        if ((slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0) {
                            allow_next_dir1 = 1;
                            go_dir1 = 0;
                        }
                    }
                }
                else
                if (delta_shift < 0)
                {
                    slb = get_slabmap_block(lslb_x+1, hslb_y);
                    slabst = get_slab_stats(slb);
                    if ((slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0)
                    {
                        slb = get_slabmap_block(lslb_x, hslb_y+1);
                        slabst = get_slab_stats(slb);
                        if ((slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0) {
                            allow_next_dir1 = 1;
                            go_dir1 = 0;
                        }
                    }
                }
                if ( go_dir1 )
                {
                    clear_slab_dig(lslb_x, hslb_y, plyr_idx);
                    slb = get_slabmap_block(lslb_x, hslb_y);
                    slabst = get_slab_stats(slb);
                    if ( go_dir1 || (slabst->block_flags & SlbAtFlg_Blocking)) {
                        set_slab_explored(plyr_idx, lslb_x, hslb_y);
                    }
                    if (slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) {
                        go_dir1 = 0;
                    }
                } else
                if ( allow_next_dir1 )
                {
                    allow_next_dir1 = 0;
                    slb = get_slabmap_block(lslb_x, hslb_y);
                    slabst = get_slab_stats(slb);
                    if (slabst->block_flags & SlbAtFlg_Filled)
                    {
                        clear_slab_dig(lslb_x, hslb_y, plyr_idx);
                        if ( go_dir1 || (slabst->block_flags & SlbAtFlg_Blocking)) {
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
                  slabst = get_slab_stats(slb);
                  if (slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable))
                  {
                      slb = get_slabmap_block(hslb_x, hslb_y-1);
                      slabst = get_slab_stats(slb);
                      if (slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) {
                          allow_next_dir2 = 0;
                          go_dir2 = 0;
                      }
                  }
              } else
              if (delta_shift < 0)
              {
                  slb = get_slabmap_block(hslb_x-1, hslb_y);
                  slabst = get_slab_stats(slb);
                  if (slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable))
                  {
                      slb = get_slabmap_block(hslb_x, hslb_y+1);
                      slabst = get_slab_stats(slb);
                      if (slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) {
                          allow_next_dir2 = 1;
                          go_dir2 = 0;
                      }
                  }
              }
              if ( go_dir2 )
              {
                clear_slab_dig(hslb_x, hslb_y, plyr_idx);
                slb = get_slabmap_block(hslb_x, hslb_y);
                slabst = get_slab_stats(slb);
                if (go_dir2 || (slabst->block_flags & SlbAtFlg_Blocking))
                  set_slab_explored(plyr_idx, hslb_x, hslb_y);
                if (slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable))
                  go_dir2 = 0;
              } else
              if ( allow_next_dir2 )
              {
                  allow_next_dir2 = 0;
                  slb = get_slabmap_block(hslb_x, hslb_y);
                  slabst = get_slab_stats(slb);
                  if (slabst->block_flags & SlbAtFlg_Filled)
                  {
                      clear_slab_dig(hslb_x, hslb_y, plyr_idx);
                      if (go_dir2 || (slabst->block_flags & SlbAtFlg_Blocking)) {
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
    if (is_neutral_thing(creatng) || thing_is_invalid(creatng)) //heroes do explore, so mapmakers can cast hero powers
        return;
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = get_floor_height_at(&pos);
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    slb_x = subtile_slab(stl_x);
    slb_y = subtile_slab(stl_y);
    struct SlabMap *slb;
    struct SlabConfigStats *slabst;
    slb = get_slabmap_block(slb_x, slb_y);
    slabst = get_slab_stats(slb);
    if (slabst->block_flags & (SlbAtFlg_IsDoor|SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable))
    {
        if (slabst->block_flags & SlbAtFlg_IsDoor)
        {
            clear_dig_and_set_explored_around(slb_x, slb_y, creatng->owner);
        }
        return;
    }

    int can_see_slabs;
    can_see_slabs = get_explore_sight_distance_in_slabs(creatng);
    if (can_see_slabs > 0)
    {
        clear_dig_and_set_explored_can_see_x(slb_x, slb_y, creatng->owner, can_see_slabs);
        clear_dig_and_set_explored_can_see_y(slb_x, slb_y, creatng->owner, can_see_slabs);
        if (!player_cannot_win(creatng->owner) && (!flag_is_set(get_creature_model_flags(creatng),CMF_IsSpectator)) && (!player_is_roaming(creatng->owner)))
        {
            claim_neutral_creatures_in_sight(creatng, &pos, can_see_slabs);
        }
    }
    clear_slab_dig(slb_x, slb_y, creatng->owner);
    set_slab_explored(creatng->owner, slb_x, slb_y);
}

long element_top_face_texture(struct Map *mapblk)
{
    struct Column *col;
    struct CubeConfigStats* cubed;
    TbBool visible = map_block_revealed(mapblk, my_player_number);
    int result = mapblk->col_idx;

    if ( !visible || (result != 0) )
    {
        if ( visible )
        {
            col = get_map_column(mapblk);
        }
        else
        {
            col = get_column(game.unrevealed_column_idx);
        }
        if ( (col->bitfields & CLF_CEILING_MASK) != 0 )
        {
            cubed = get_cube_model_stats(col->cubes[COLUMN_STACK_HEIGHT-get_column_ceiling_filled_subtiles(col)-1]);
            return cubed->texture_id[4];
        }
        else if ((col->bitfields & CLF_FLOOR_MASK) != 0)
        {
            cubed = get_cube_model_stats(col->cubes[get_column_floor_filled_subtiles(col) - 1]);
            return cubed->texture_id[4];
        }
        else
        {
            return col->floor_texture;
        }
    }
    return result;
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
        struct Thing *doortng;
        doortng = get_door_for_position(pos->x.stl.num, pos->y.stl.num);
        if (!thing_is_invalid(doortng))
        {
            if (!players_are_mutual_allies(doortng->owner,plyr_idx) || (doortng->door.is_locked)) {
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
  struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
  struct SlabConfigStats* slabst = get_slab_stats(slb);
  if ((slabst->category != SlbAtCtg_FortifiedWall))
   return;
  if ( slabmap_owner(slb) != plyr_idx )
    return;
  for (long n = 0; n < SMALL_AROUND_LENGTH; n++)
  {
    MapSlabCoord x = slb_x + small_around[n].delta_x;
    MapSlabCoord y = slb_y + small_around[n].delta_y;
    struct SlabMap *slb2 = get_slabmap_block(x, y);
    struct SlabConfigStats* slabst2 = get_slab_stats(slb2);
    if ( (((slabst2->category == SlbAtCtg_FortifiedGround) || (slabst2->block_flags & SlbAtFlg_IsRoom) || ((slabst2->block_flags & SlbAtFlg_IsDoor)) ))
      && (slabmap_owner(slb2) == plyr_idx ) )
    {
      for (int k = -1; k < 2; k+=2)
      {
        int j = (k + n) & 3;
        MapSlabCoord x2 = x + small_around[j].delta_x;
        MapSlabCoord y2 = y + small_around[j].delta_y;
        struct SlabMap *slb3 = get_slabmap_block(x2, y2);
        struct SlabConfigStats* slabst3 = get_slab_stats(slb3);
        if ( (slabst3->category == SlbAtCtg_FortifiedWall)
          && (slabmap_owner(slb3) == plyr_idx ) )
        {
          int m = (k + j) & 3;
          MapSlabCoord x3 = x2 + small_around[m].delta_x;
          MapSlabCoord y3 = y2 + small_around[m].delta_y;
          struct SlabMap *slb4 = get_slabmap_block(x3, y3);
          struct SlabConfigStats* slabst4 = get_slab_stats(slb4);
          if (slabst4->category == SlbAtCtg_FriableDirt)
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
    if (!thing_exists(heartng))
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
    panel_map_update(stl_x, stl_y, STL_PER_SLB, STL_PER_SLB);
}

void place_and_process_pretty_wall_slab(struct Thing *creatng, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct CreatureControl *cctrl;
    SYNCDBG(16,"Starting");
    cctrl = creature_control_get_from_thing(creatng);
    unsigned char pretty_type;
    pretty_type = choose_pretty_type(creatng->owner, slb_x, slb_y);
    place_slab_type_on_map(pretty_type, slab_subtile_center(slb_x), slab_subtile_center(slb_y), creatng->owner, 0);
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

TbBool subtile_is_diggable_at_diagonal_angle(struct Thing *thing, unsigned short angle, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    if ( (subtile_slab(stl_x) == subtile_slab(thing->mappos.x.stl.num)) && (subtile_slab(stl_y) == subtile_slab(thing->mappos.y.stl.num)) )
    {
        return true;
    }
    MapSubtlCoord check_stl_x, check_stl_y;
    switch(angle)
    {
        case ANGLE_NORTHEAST:
        {
            check_stl_x = stl_x - 1;
            check_stl_y = stl_y + 1;
            break;
        }
        case ANGLE_SOUTHEAST:
        {
            check_stl_x = stl_x - 1;
            check_stl_y = stl_y - 1;
            break;
        }
        case ANGLE_SOUTHWEST:
        {
            check_stl_x = stl_x + 1;
            check_stl_y = stl_y - 1;
            break;
        }
        case ANGLE_NORTHWEST:
        {
            check_stl_x = stl_x + 1;
            check_stl_y = stl_y + 1;
            break;
        }
        default:
        {
            check_stl_x = stl_x;
            check_stl_y = stl_y;
            break;
        }
        break;
    }
    if ( (!slab_is_wall(subtile_slab(stl_x), subtile_slab(check_stl_y))) || (!slab_is_wall(subtile_slab(check_stl_x), subtile_slab(stl_y))) )
    {
        return true;
    }
    return false;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
