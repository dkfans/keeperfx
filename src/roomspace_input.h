/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file roomspace_input.h
 *     Turns local mouse input into dig tag packet actions, and shows those tags and the cursor box before the game state catches up.
 */
/******************************************************************************/
#ifndef DK_ROOMSPACE_INPUT_H
#define DK_ROOMSPACE_INPUT_H

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct RoomSpace;

void reset_dig_prediction(void);
void update_dig_tag_input(void);
void update_dig_prediction_cursor_preview(void);
unsigned char get_dig_prediction_render_flags(MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned char base_map_flags);
const struct RoomSpace *get_dig_prediction_render_roomspace(const struct RoomSpace *roomspace);
TbBool local_cursor_is_locked(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
