/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_render.h
 *     Header file for engine_render.c.
 * @par Purpose:
 *     Rendering the 3D view functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     20 Mar 2009 - 30 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ENGNREND_H
#define DK_ENGNREND_H

#include "bflib_basics.h"
#include "globals.h"
#include "game_legacy.h"
#include "bflib_render.h"
#include "bflib_sprite.h"
#include "engine_lenses.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define POLY_POOL_SIZE 16777216 // Originally 262144, adjusted for view distance
#define Z_DRAW_DISTANCE_MAX 65536 // Originally 11232, adjusted for view distance
#define BUCKETS_COUNT 4098 // Originally 704, adjusted for view distance. (65536/16)+2
#define BUCKETS_STEP 16 // Bucket size in Z steps

#define KEEPSPRITE_LENGTH 9149
#define KEEPERSPRITE_ADD_OFFSET 16384
#define KEEPERSPRITE_ADD_NUM 16383

#pragma pack(push,1)
struct EngineCoord { // sizeof = 28
  int32_t view_width; // X screen position, probably not a width
  int32_t view_height; // Y screen position, probably not a height
  uint16_t clip_flags; // Clipping and culling flags for frustum culling
  uint16_t shade_intensity; // Shading intensity for vertex lighting
  int32_t render_distance; // Distance used for rendering calculations
  int32_t x;
  int32_t y;
  int32_t z;
};
#pragma pack(pop)

struct M31 {
    int32_t v[4];
};

struct M33 { // sizeof = 48
    struct M31 r[3];
};

struct MapVolumeBox { // sizeof = 24
  unsigned char visible;
  unsigned char color;
  int32_t beg_x;
  int32_t beg_y;
  int32_t end_x;
  int32_t end_y;
  int32_t floor_height_z;
};

struct ThingInterpolateResult
{
    struct Coord3d mappos;
    int32_t floor_height;
};

/******************************************************************************/
// Stripey Line Color Arrays

enum stripey_line_colors {
    SLC_RED = 0, // INVALID SELECTION
    SLC_GREEN = 1, // VALID SELECTION
    SLC_YELLOW,
    SLC_BROWN,
    SLC_GREY,
    SLC_REDYELLOW,
    SLC_GREENFLASH,
    SLC_REDFLASH,
    SLC_PURPLE,
    SLC_BLUE,
    SLC_ORANGE,
    SLC_WHITE,
    SLC_GREEN2,
    SLC_DARKGREEN,
    SLC_MIXEDGREEN,
    STRIPEY_LINE_COLOR_COUNT // Must always be the last entry (add new colours above this line)
};

struct stripey_line {
    TbPixel stripey_line_color_array[16];
    unsigned int line_color;
};

extern struct stripey_line colored_stripey_lines[];
extern unsigned char poly_pool[POLY_POOL_SIZE];
extern unsigned char *poly_pool_end;
extern int32_t cells_away;
extern float hud_scale;
extern int creature_status_size;
extern int line_box_size;

extern struct MapVolumeBox map_volume_box;
extern int32_t view_height_over_2;
extern int32_t view_width_over_2;
extern int32_t z_threshold_near;
extern int32_t split_2;
extern int32_t fade_max;

extern int16_t mx;
extern int16_t my;
extern int16_t mz;

extern int32_t floor_pointed_at_x;
extern int32_t floor_pointed_at_y;
extern int32_t box_lag_compensation_x;
extern int32_t box_lag_compensation_y;
extern Offset vert_offset[3];
extern Offset hori_offset[3];
extern Offset high_offset[3];

extern TbSpriteData *keepsprite[KEEPSPRITE_LENGTH];
extern TbSpriteData sprite_heap_handle[KEEPSPRITE_LENGTH];
extern struct HeapMgrHeader *graphics_heap;
extern TbFileHandle jty_file_handle;

extern int32_t x_init_off;
extern int32_t y_init_off;
extern struct Thing *thing_being_displayed;

extern unsigned char temp_cluedo_mode;
/******************************************************************************/

extern TbSpriteData keepersprite_add[KEEPERSPRITE_ADD_NUM];
/*****************************************************************************/
float interpolate(float previous, float current);
float interpolate_angle(float previous, float current);
float interpolate_synced(float previous, float current);
struct ThingInterpolateResult interpolate_thing(struct Thing *thing);

int floor_height_for_volume_box(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y);
void frame_wibble_generate(void);
void setup_rotate_stuff(int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6, int32_t a7, int32_t a8);

void process_keeper_sprite(int16_t x, int16_t y, uint16_t a3, int16_t kspr_angle, unsigned char a5, int32_t a6);
void draw_status_sprites(int32_t a1, int32_t a2, struct Thing *thing);
void draw_map_volume_box(int32_t cor1_x, int32_t cor1_y, int32_t cor2_x, int32_t cor2_y, int32_t floor_height_z, unsigned char color);

void update_engine_settings(struct PlayerInfo *player);
void draw_view(struct Camera *cam, unsigned char a2);
void draw_frontview_engine(struct Camera *cam);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
