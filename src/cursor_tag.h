#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

#pragma pack()
/******************************************************************************/
void tag_cursor_blocks_dig(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, TbBool full_slab);
void tag_cursor_blocks_thing_in_hand(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, TbBool is_special_digger, TbBool full_slab);
TbBool tag_cursor_blocks_sell_area(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, TbBool full_slab);
TbBool tag_cursor_blocks_place_door(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool tag_cursor_blocks_place_room(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, TbBool full_slab);
void tag_cursor_blocks_place_terrain(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool tag_cursor_blocks_place_thing(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool tag_cursor_blocks_order_creature(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct Thing* creatng);
TbBool tag_cursor_blocks_steal_slab(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
