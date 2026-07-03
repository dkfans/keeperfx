/**
 * @file ftest_util.h
 * @author demonds1
 * @brief Helper functions for use in Functional Tests
 * @version 0.1
 * @date 2023-11-30
 * 
 * @copyright Copyright (c) 2023
 */

#pragma once

#ifdef FUNCTESTING

#include "../globals.h"

#include "ftest.h"

#include "../thing_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Replaces slabs in the given area.
 * If the owner exists, it will claim the slab first.
 * 
 * @param slb_x_from 
 * @param slb_y_from 
 * @param slb_x_to 
 * @param slb_y_to 
 * @param slab_kind 
 * @param owner
 * @return true, false if any slab replacements failed
 */
TbBool ftest_util_replace_slabs(MapSlabCoord slb_x_from, MapSlabCoord slb_y_from, MapSlabCoord slb_x_to, MapSlabCoord slb_y_to, SlabKind slab_kind, PlayerNumber owner);

/**
 * @brief Checks if the player owns any slabs in the given area.
 * 
 * @param slb_x_from 
 * @param slb_y_from 
 * @param slb_x_to 
 * @param slb_y_to 
 * @param owner 
 * @return true if any slabs owned, false otherwise
 */
TbBool ftest_util_does_player_own_any_slabs(MapSlabCoord slb_x_from, MapSlabCoord slb_y_from, MapSlabCoord slb_x_to, MapSlabCoord slb_y_to, PlayerNumber owner);

/**
 * @brief Checks if any slabs match the specified kind
 * 
 * @param slb_x_from 
 * @param slb_y_from 
 * @param slb_x_to 
 * @param slb_y_to 
 * @param slab 
 * @return true if any match, false otherwise 
 */
TbBool ftest_util_do_any_slabs_match(MapSlabCoord slb_x_from, MapSlabCoord slb_y_from, MapSlabCoord slb_x_to, MapSlabCoord slb_y_to, SlabKind slab);

/**
 * @brief Reveals the map for the given player
 * 
 * @param plyr_idx 
 * @return bool
 */
TbBool ftest_util_reveal_map(PlayerNumber plyr_idx);

/**
 * @brief 
 * 
 * @param slb_x 
 * @param slb_y 
 * @param subtile_index 
 * @param slab_base_type 
 * @param column_type 
 * @return bool
 */
TbBool ftest_util_replace_slab_columns(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber owner, SlabKind slab_base_type, ColumnIndex column0_type, ColumnIndex column1_type, ColumnIndex column2_type, ColumnIndex column3_type, ColumnIndex column4_type, ColumnIndex column5_type, ColumnIndex column6_type, ColumnIndex column7_type, ColumnIndex column8_type);

/**
 * @brief 
 * 
 * @param x mappos.x.val
 * @param y mappos.y.val
 * @param plyr_idx 
 * @return bool
 */
TbBool ftest_util_move_camera(long x, long y, PlayerNumber plyr_idx);

/**
 * @brief 
 * 
 * @param thing 
 * @param plyr_idx 
 * @return bool 
 */
TbBool ftest_util_move_camera_to_thing(struct Thing* const thing, PlayerNumber plyr_idx);

/**
 * @brief 
 * 
 * @param slb_x 
 * @param slb_y 
 * @param plyr_idx 
 * @return bool 
 */
TbBool ftest_util_move_camera_to_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx);

/**
 * Creates creature of random kind, and with random experience level.
 * @param x
 * @param y
 * @param owner
 * @param max_level
 * @return
 */
struct Thing* ftest_util_create_random_creature(MapCoord x, MapCoord y, PlayerNumber owner, CrtrExpLevel max_level);

/**
 * @brief Creates creature of kind creature_model with random experience level
 * 
 * @param x 
 * @param y 
 * @param owner 
 * @param max_level 
 * @param creature_model 
 * @return
 */
struct Thing* ftest_util_create_creature(MapCoord x, MapCoord y, PlayerNumber owner, CrtrExpLevel max_level, ThingModel creature_model);

/**
 * @brief Centers the players cursor over the dungeon view (useful for picking up creatures by moving camera to/near unit pos)
 * 
 */
void ftest_util_center_cursor_over_dungeon_view();

/**
 * @brief Replaces slabs with dungeon hearts, separated by paths so that destroying one won't destroy the others
 * 
 * @param slb_x_from 
 * @param slb_y_from 
 * @param slb_x_to 
 * @param slb_y_to 
 * @param owner 
 * @return  
 */
TbBool ftest_util_replace_slabs_with_dungeon_hearts(MapSlabCoord slb_x_from, MapSlabCoord slb_y_from, MapSlabCoord slb_x_to, MapSlabCoord slb_y_to, PlayerNumber owner);

/**
 * @brief Marks the slab with an X pattern that is only used visually (for testing purposes, eg: to visualize areas on the map)
 * 
 * @param slb_x 
 * @param slb_y 
 * @param plyr_idx 
 * @return TbBool 
 */
TbBool ftest_util_mark_slab_for_highlight(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx);

/**
 * @brief test action to create a torture room for player and fill it with assigned torture creatures
 * 
 * @param args 
 * @return TbBool 
 */
TbBool ftest_util_action__create_and_fill_torture_room(struct FTestActionArgs* const args);
struct ftest_util_action__create_and_fill_torture_room__variables
{
    const MapSlabCoord room_slb_x_start;
    const MapSlabCoord room_slb_y_start;

    const MapSlabCoord room_width;
    const MapSlabCoord room_height;

    PlayerNumber room_owner;

    ThingModel victim_creature_model;
    PlayerNumber victim_player_owner;
    CrtrExpLevel victim_max_level;

    TbBool only_run_once;
};


#ifdef __cplusplus
}
#endif

#endif // FUNCTESTING
