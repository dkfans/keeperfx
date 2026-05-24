/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file roomspace_prediction.h
 *     Client-side dig prediction overlay for roomspace highlighting.
 */
/******************************************************************************/
#ifndef DK_ROOMSPACE_PREDICTION_H
#define DK_ROOMSPACE_PREDICTION_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct RoomSpace;

TbBool local_dig_prediction_is_enabled(void);
void update_local_dig_tag_prediction(void);
unsigned char get_local_dig_prediction_render_flags(MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned char base_map_flags);
void update_local_dig_prediction_cursor_preview(void);
struct RoomSpace *get_local_dig_prediction_render_roomspace(struct RoomSpace *roomspace);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
