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

struct EngineCoord { // sizeof = 28
  long view_width; // X screen position, probably not a width
  long view_height; // Y screen position, probably not a height
  unsigned short clip_flags; // Clipping and culling flags for frustum culling
  unsigned short shade_intensity; // Shading intensity for vertex lighting
  long render_distance; // Distance used for rendering calculations
  long x;
  long y;
  long z;
};

struct M31 {
    long v[4];
};

struct M33 { // sizeof = 48
    struct M31 r[3];
};

struct MapVolumeBox { // sizeof = 24
  unsigned char visible;
  unsigned char color;
  long beg_x;
  long beg_y;
  long end_x;
  long end_y;
  long floor_height_z;
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
extern long cells_away;
extern float hud_scale;
extern int creature_status_size;
extern int line_box_size;

extern struct MapVolumeBox map_volume_box;
extern long view_height_over_2;
extern long view_width_over_2;
extern long z_threshold_near;
extern long split_2;
extern long fade_max;

extern short mx;
extern short my;
extern short mz;

extern long floor_pointed_at_x;
extern long floor_pointed_at_y;

// ToDo : this is fucking garbage, remove it and use the proper camera position instead
// Front-view textured-quad UV corner tables (draw_texturedquad_block(), and
// GLWorldViewRenderer::append_frontview_quad() -- P5.7.2a).
extern long const orient_to_mapU1[];
extern long const orient_to_mapU2[];
extern long const orient_to_mapU3[];
extern long const orient_to_mapU4[];
extern long const orient_to_mapV1[];
extern long const orient_to_mapV2[];
extern long const orient_to_mapV3[];
extern long const orient_to_mapV4[];
extern long box_lag_compensation_x;
extern long box_lag_compensation_y;
extern Offset vert_offset[3];
extern Offset hori_offset[3];
extern Offset high_offset[3];

extern TbSpriteData *keepsprite[KEEPSPRITE_LENGTH];
extern TbSpriteData sprite_heap_handle[KEEPSPRITE_LENGTH];
extern struct HeapMgrHeader *graphics_heap;
extern TbFileHandle jty_file_handle;

extern long x_init_off;
extern long y_init_off;
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
void setup_rotate_stuff(long a1, long a2, long a3, long a4, long a5, long a6, long a7, long a8);

void process_keeper_sprite(short x, short y, unsigned short a3, short kspr_angle, unsigned char a5, long a6);
void draw_status_sprites(long a1, long a2, struct Thing *thing);

// Room-flag pole/top and floating gold/damage text raster. No longer static:
// GLWorldViewRenderer's own bucket walk calls these directly (same reason as
// draw_jonty_mapwho() below -- reuse the exact resolution+raster code
// software uses; these already submit through LbSpriteDrawScaled()/
// LbDrawBox(), which route through the renderer bridge on both backends).
struct BucketKindRoomFlag;
struct BucketKindFloatingGoldText;
void draw_engine_room_flagpole(struct BucketKindRoomFlag *rflg);
void draw_engine_room_flag_top(struct BucketKindRoomFlag *rflg);
void draw_engine_number(struct BucketKindFloatingGoldText *num);

// Slab/dig selection outline raster, same reason: submits through LbDrawBox().
void draw_clipped_line(long x1, long y1, long x2, long y2, TbPixel color);

// Walk-time keeper-sprite resolution+raster for one bucket entry (isometric
// view: draw_jonty_mapwho(); front view: draw_fastview_mapwho()/
// draw_iso_only_fastview_mapwho() for the spinning-key case). No longer
// static: GLWorldViewRenderer's own bucket walk (P5.7.3b) calls these
// directly so GL reuses the exact same resolution code software does --
// only the final raster call inside draw_keepersprite() diverges per backend.
// Forward-declared (full definition lives in engine_buckets.h, which this
// header deliberately doesn't pull in) so the pointer-only use below doesn't
// trigger gcc's "declared inside parameter list" -Werror.
struct BucketKindJontySprite;
void draw_jonty_mapwho(struct BucketKindJontySprite *jspr);
void draw_fastview_mapwho(struct Camera *cam, struct BucketKindJontySprite *jspr);
void draw_iso_only_fastview_mapwho(struct Camera *cam, struct BucketKindJontySprite *spr);

// Resolve a keeper-sprite frame's atlas cache key (draw_idx) + raw RLE data
// pointer + content dims, without any draw-flag/positioning side effects.
// GLWorldViewRenderer's shadow case (P5.7.4) calls this directly to reuse
// the exact same GPU atlas layer a real sprite draw of that frame would use.
// out_kspr (optional, may be NULL) returns the resolved per-frame entry --
// resolve_keepersprite_cursor_geometry() (below) needs it for FrameOffsW/H.
struct KeeperSprite;
TbBool resolve_keepersprite_draw_data(unsigned short anim_sprite, short angle,
    unsigned char current_frame, int32_t *out_draw_idx,
    const unsigned char **out_data, int *out_src_w, int *out_src_h,
    const struct KeeperSprite **out_kspr);

// Cursor/power-hand geometry resolve (P5.7.5): the destination-rect twin of
// process_keeper_sprite()'s own xflip/scaled-position math, minus the
// water-cutoff logic (not applicable to a UI overlay sprite) and minus its
// ambient draw-flag side effect. GLCursorLayer::SubmitKeeperHandSprite()
// (game thread) calls this to resolve everything it needs up front before
// submitting through GLWorldViewRenderer::BeginCursorCapture()/
// SubmitKeeperSprite()/EndCursorCapture() (Beat 3), so the render thread
// never touches game-thread-only state (keepersprite_array(), the sprite
// heap) directly.
TbBool resolve_keepersprite_cursor_geometry(short x, short y, unsigned short kspr_base,
    short kspr_angle, unsigned char sprgroup, long scale,
    int32_t *out_dst_x, int32_t *out_dst_y, int32_t *out_dst_w, int32_t *out_dst_h,
    int32_t *out_draw_idx, const unsigned char **out_data, int *out_src_w, int *out_src_h);
void draw_map_volume_box(long cor1_x, long cor1_y, long cor2_x, long cor2_y, long floor_height_z, unsigned char color);

void update_engine_settings(struct PlayerInfo *player);
void draw_view(struct Camera *cam, unsigned char a2);
void draw_frontview_engine(struct Camera *cam);

// Rasterize the bucket list built by the most recent draw_view()/
// draw_frontview_engine() walk. Called (via SoftwareWorldViewRenderer) at
// deferred-resolve time, not immediately after the walk -- see
// software_execute_world_from_ir()'s comment in engine_render.c.
void display_drawlist(void);
void display_fast_drawlist(struct Camera *cam);
void software_execute_world_from_ir(int win_x, int win_y, int win_w, int win_h,
                                    int is_frontview, struct Camera *cam);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
