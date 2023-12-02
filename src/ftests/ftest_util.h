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

#ifdef __cplusplus
}
#endif

#endif // FUNCTESTING