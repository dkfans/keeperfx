/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file SwZoomBoxTiles.h
 *     Software raster for the parchment zoom box's terrain tiles.
 */
/******************************************************************************/
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Draw each non-0xFFFF texture block of a tiles_x*tiles_y grid, row-major,
 *  into the software draw target at (dst_x, dst_y), tile_w*tile_h each. */
void SwDrawZoomBoxTiles(const uint16_t* tile_block_ids, int32_t tiles_x, int32_t tiles_y,
                        int32_t dst_x, int32_t dst_y, int32_t tile_w, int32_t tile_h);

#ifdef __cplusplus
}
#endif
