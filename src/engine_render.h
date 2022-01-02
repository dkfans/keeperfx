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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define BUCKETS_COUNT 704
#define KEEPSPRITE_LENGTH 9149

#define KEEPERSPRITE_ADD_OFFSET 16384
#define KEEPERSPRITE_ADD_NUM 2048

enum QKinds {
    QK_PolyTriangle = 0,
    QK_PolyTriangleSimp,
    QK_PolyMode0,
    QK_PolyMode4,
    QK_TrigMode2,
    QK_PolyMode5,
    QK_TrigMode3,
    QK_TrigMode6,
    QK_RotableSprite,
    QK_Unknown9,
    QK_Unknown10,
    QK_JontySprite,
    QK_KeeperSprite,
    QK_ClippedLine,
    QK_StatusSprites,
    QK_TextureQuad,
    QK_IntegerValue,
    QK_RoomFlagPole,
    QK_JontyISOSprite,
    QK_RoomFlagTop,
    QK_Unknown20,
};

struct MinMax;
struct Camera;
struct PlayerInfo;

typedef unsigned char QKind;

struct BasicQ { // sizeof = 5
  struct BasicQ *next;
  QKind kind;
};

struct BasicUnk00 {
    struct BasicQ b;
    unsigned char field_5;
    unsigned short block;
    struct PolyPoint p1;
    struct PolyPoint p2;
    struct PolyPoint p3;
};

struct BasicUnk01 {
    struct BasicQ b;
    unsigned char field_5;
    unsigned short block;
    struct PolyPoint p1;
    struct PolyPoint p2;
    struct PolyPoint p3;
};

struct BasicUnk02 {
    struct BasicQ b;
    unsigned char colour;
    unsigned short x1;
    unsigned short y1;
    unsigned short x2;
    unsigned short y2;
    unsigned short x3;
    unsigned short y3;
};

struct BasicUnk03 {
    struct BasicQ b;
    unsigned char colour;
    unsigned short x1;
    unsigned short y1;
    unsigned short x2;
    unsigned short y2;
    unsigned short x3;
    unsigned short y3;
    unsigned char vf1;
    unsigned char vf2;
    unsigned char vf3;
    unsigned char field_15[3];
    unsigned char field_18;
    unsigned char field_19[3];
};

struct BasicUnk04 {
    struct BasicQ b;
    unsigned char field_5;
    unsigned short x1;
    unsigned short y1;
    unsigned short x2;
    unsigned short y2;
    unsigned short x3;
    unsigned short y3;
    unsigned short field_12;
    unsigned char uf1;
    unsigned char vf1;
    unsigned char uf2;
    unsigned char vf2;
    unsigned char uf3;
    unsigned char vf3;
};

struct BasicUnk05 {
    struct BasicQ b;
    unsigned char field_5;
    unsigned short x1;
    unsigned short y1;
    unsigned short x2;
    unsigned short y2;
    unsigned short x3;
    unsigned short y3;
    unsigned short field_12;
    unsigned char uf1;
    unsigned char vf1;
    unsigned char uf2;
    unsigned char vf2;
    unsigned char uf3;
    unsigned char vf3;
    unsigned char wf1;
    unsigned char wf2;
    unsigned char wf3;
};

struct BasicUnk06 {
    struct BasicQ b;
    unsigned char field_5;
    unsigned short x1;
    unsigned short y1;
    unsigned short x2;
    unsigned short y2;
    unsigned short x3;
    unsigned short y3;
    unsigned short field_12;
    unsigned char uf1;
    unsigned char vf1;
    unsigned char uf2;
    unsigned char vf2;
    unsigned char uf3;
    unsigned char vf3;
};

struct BasicUnk07 {
    struct BasicQ b;
    unsigned char field_5;
    unsigned short x1;
    unsigned short y1;
    unsigned short x2;
    unsigned short y2;
    unsigned short x3;
    unsigned short y3;
    unsigned short field_12;
    unsigned char uf1;
    unsigned char vf1;
    unsigned char uf2;
    unsigned char vf2;
    unsigned char uf3;
    unsigned char vf3;
    unsigned char wf1;
    unsigned char wf2;
    unsigned char wf3;
};

struct RotoSpr {
    struct BasicQ b;
    unsigned char field_5[3];
    long field_8;
    long field_C;
    long field_10;
    long field_14;
    char field_18;
    unsigned char field_19[3];
};

struct BasicUnk09 {
    struct BasicQ b;
    unsigned char subtype;
    unsigned short block;
    struct PolyPoint p1;
    struct PolyPoint p2;
    struct PolyPoint p3;
    long field_44;
    long field_48;
    long field_4C;
    long field_50;
    long field_54;
    long field_58;
    long field_5C;
    long field_60;
    long field_64;
};

struct BasicUnk10 {
    struct BasicQ b;
    unsigned char field_5;
    unsigned char field_6;
    unsigned char field_7;
    struct PolyPoint p1;
    struct PolyPoint p2;
    struct PolyPoint p3;
};

struct JontySpr {  // BasicQ type 11,18
    struct BasicQ b;
    unsigned char field_5[3];
    struct Thing *thing;
    long scr_x;
    long scr_y;
    long field_14;
    unsigned char field_18;
    unsigned char field_19[3];
};

struct KeeperSpr {
    struct BasicQ b;
    unsigned char field_5;
    unsigned short field_6;
    struct PolyPoint p1;
    struct PolyPoint p2;
    struct PolyPoint p3;
    struct PolyPoint p4;
    long field_58;
    unsigned short field_5C;
    unsigned char field_5E;
};

struct BasicUnk13 {
    struct BasicQ b;
    unsigned char field_5;
    unsigned short field_6;
    struct PolyPoint p;
    unsigned char field_19[3];
};

struct BasicUnk14 { // sizeof = 24
    struct BasicQ b;
    unsigned char padding[3];
    struct Thing *thing;
    long x;
    long y;
    long z;
};

#define SHADOW_SOURCES_MAX_COUNT 4
struct NearestLights {
    struct Coord3d coord[SHADOW_SOURCES_MAX_COUNT];
};


struct TexturedQuad { // sizeof = 46
    struct BasicQ b;
    unsigned char field_5;
    long field_6;
    long field_A;
    long field_E;
    long field_12;
    long field_16;
    long field_1A;
    long field_1E;
    long field_22;
    long field_26;
    long field_2A;
};

struct Number { // BasicQ type 16
    struct BasicQ b;
    unsigned char field_5[3];
    long x;
    long y;
    long lvl;
};

struct RoomFlag { // BasicQ type 17,19
    struct BasicQ b;
    unsigned char field_5;
    unsigned short lvl;
    long x;
    long y;
};

struct EngineCoord { // sizeof = 28
  long view_width;
  long view_height;
  unsigned short field_8;
  unsigned short field_A;
  long field_C;
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

struct EngineCol {
    struct EngineCoord cors[16];
};

struct SideOri {
    unsigned char field_0;
    unsigned char field_1;
    unsigned char field_2;
    unsigned char field_3;
};

struct MapVolumeBox { // sizeof = 24
  unsigned char visible;
  unsigned char color;
  unsigned char field_2;
  long beg_x;
  long beg_y;
  long end_x;
  long end_y;
  long floor_height_z;
  unsigned char field_17;
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
    STRIPEY_LINE_COLOR_COUNT // Must always be the last entry (add new colours above this line)
};

struct stripey_line {
    TbPixel stripey_line_color_array[16];
    unsigned int line_color;
};

extern struct stripey_line colored_stripey_lines[];
/******************************************************************************/
DLLIMPORT unsigned char *_DK_getpoly;
#define getpoly _DK_getpoly
DLLIMPORT unsigned char _DK_poly_pool[0x40000];
#define poly_pool _DK_poly_pool
DLLIMPORT unsigned char *_DK_poly_pool_end;
#define poly_pool_end _DK_poly_pool_end
DLLIMPORT struct BasicQ *_DK_buckets[BUCKETS_COUNT];
#define buckets _DK_buckets
DLLIMPORT Offset _DK_vert_offset[3];
#define vert_offset _DK_vert_offset
DLLIMPORT Offset _DK_hori_offset[3];
#define hori_offset _DK_hori_offset
DLLIMPORT Offset _DK_high_offset[3];
#define high_offset _DK_high_offset
DLLIMPORT long _DK_x_init_off;
#define x_init_off _DK_x_init_off
DLLIMPORT long _DK_y_init_off;
#define y_init_off _DK_y_init_off
DLLIMPORT long _DK_floor_pointed_at_x;
#define floor_pointed_at_x _DK_floor_pointed_at_x
DLLIMPORT long _DK_floor_pointed_at_y;
#define floor_pointed_at_y _DK_floor_pointed_at_y
DLLIMPORT long _DK_cells_away;
#define cells_away _DK_cells_away
DLLIMPORT long _DK_fade_max;
#define fade_max _DK_fade_max
DLLIMPORT long _DK_fade_scaler;
#define fade_scaler _DK_fade_scaler
DLLIMPORT long _DK_fade_way_out;
#define fade_way_out _DK_fade_way_out
DLLIMPORT struct MapVolumeBox _DK_map_volume_box;
#define map_volume_box _DK_map_volume_box
DLLIMPORT long _DK_map_roll;
#define map_roll _DK_map_roll
DLLIMPORT long _DK_map_tilt;
#define map_tilt _DK_map_tilt
DLLIMPORT long _DK_view_alt;
#define view_alt _DK_view_alt
DLLIMPORT long _DK_fade_min;
#define fade_min _DK_fade_min
DLLIMPORT long _DK_split_1;
#define split_1 _DK_split_1
DLLIMPORT long _DK_split_2;
#define split_2 _DK_split_2
DLLIMPORT long _DK_fade_range;
#define fade_range _DK_fade_range
DLLIMPORT long _DK_depth_init_off;
#define depth_init_off _DK_depth_init_off
DLLIMPORT int _DK_normal_shade_left;
#define normal_shade_left _DK_normal_shade_left
DLLIMPORT int _DK_normal_shade_right;
#define normal_shade_right _DK_normal_shade_right
DLLIMPORT long _DK_apos;
#define apos _DK_apos
DLLIMPORT long _DK_bpos;
#define bpos _DK_bpos
DLLIMPORT long _DK_ScrCenterX;
#define ScrCenterX _DK_ScrCenterX
DLLIMPORT long _DK_ScrWidth;
#define ScrWidth _DK_ScrWidth
DLLIMPORT long _DK_ScrHeight;
#define ScrHeight _DK_ScrHeight
DLLIMPORT long _DK_ScrCenterY;
#define ScrCenterY _DK_ScrCenterY
DLLIMPORT long _DK_split1at;
#define split1at _DK_split1at
DLLIMPORT long _DK_split2at;
#define split2at _DK_split2at
DLLIMPORT long _DK_max_i_can_see;
#define max_i_can_see _DK_max_i_can_see
DLLIMPORT long _DK_view_height_over_2;
#define view_height_over_2 _DK_view_height_over_2
DLLIMPORT long _DK_view_width_over_2;
#define view_width_over_2 _DK_view_width_over_2
DLLIMPORT long _DK_map_x_pos;
#define map_x_pos _DK_map_x_pos
DLLIMPORT long _DK_map_y_pos;
#define map_y_pos _DK_map_y_pos
DLLIMPORT long _DK_map_z_pos;
#define map_z_pos _DK_map_z_pos
DLLIMPORT int _DK_normal_shade_front;
#define normal_shade_front _DK_normal_shade_front
DLLIMPORT int _DK_normal_shade_back;
#define normal_shade_back _DK_normal_shade_back
DLLIMPORT unsigned char _DK_temp_cluedo_mode; // This is true(1) if the "short wall" have been enabled in the graphics options
#define temp_cluedo_mode _DK_temp_cluedo_mode
DLLIMPORT long _DK_me_distance;
#define me_distance _DK_me_distance
DLLIMPORT short _DK_mx;
#define mx _DK_mx
DLLIMPORT short _DK_my;
#define my _DK_my
DLLIMPORT short _DK_mz;
#define mz _DK_mz
DLLIMPORT unsigned char _DK_engine_player_number;
#define engine_player_number _DK_engine_player_number
DLLIMPORT unsigned char _DK_player_bit;
#define player_bit _DK_player_bit
DLLIMPORT long _DK_UseFastBlockDraw;
#define UseFastBlockDraw _DK_UseFastBlockDraw
DLLIMPORT unsigned char *_DK_gtblock_screen_addr;
#define gtblock_screen_addr _DK_gtblock_screen_addr
DLLIMPORT long _DK_gtblock_clip_width;
#define gtblock_clip_width _DK_gtblock_clip_width
DLLIMPORT long _DK_gtblock_clip_height;
#define gtblock_clip_height _DK_gtblock_clip_height
DLLIMPORT long _DK_gtblock_screen_width;
#define gtblock_screen_width _DK_gtblock_screen_width
DLLIMPORT long _DK_thelens;
#define thelens _DK_thelens
DLLIMPORT long _DK_fade_mmm;
#define fade_mmm _DK_fade_mmm
DLLIMPORT long _DK_spr_map_angle;
#define spr_map_angle _DK_spr_map_angle
DLLIMPORT long _DK_lfade_max;
#define lfade_max _DK_lfade_max
DLLIMPORT long _DK_lfade_min;
#define lfade_min _DK_lfade_min
DLLIMPORT struct Thing *_DK_thing_being_displayed;
#define thing_being_displayed _DK_thing_being_displayed
DLLIMPORT unsigned char _DK_thing_being_displayed_is_creature;
#define thing_being_displayed_is_creature _DK_thing_being_displayed_is_creature
DLLIMPORT extern struct EngineCol _DK_ecs1[];
#define ecs1 _DK_ecs1
DLLIMPORT extern struct EngineCol _DK_ecs2[];
#define ecs2 _DK_ecs2
DLLIMPORT extern struct EngineCol *_DK_front_ec;
#define front_ec _DK_front_ec
DLLIMPORT extern struct EngineCol *_DK_back_ec;
#define back_ec _DK_back_ec
DLLIMPORT long _DK_global_scaler;
#define global_scaler _DK_global_scaler
DLLIMPORT long _DK_water_source_cutoff;
#define water_source_cutoff _DK_water_source_cutoff
DLLIMPORT long _DK_water_y_offset;
#define water_y_offset _DK_water_y_offset

DLLIMPORT TbSpriteData *_DK_keepsprite[KEEPSPRITE_LENGTH];
#define keepsprite _DK_keepsprite
DLLIMPORT struct HeapMgrHandle * _DK_heap_handle[KEEPSPRITE_LENGTH];
#define heap_handle _DK_heap_handle
DLLIMPORT struct HeapMgrHeader *_DK_graphics_heap;
#define graphics_heap _DK_graphics_heap
DLLIMPORT TbFileHandle _DK_file_handle;
#define file_handle _DK_file_handle
DLLIMPORT long _DK_cam_map_angle;
#define cam_map_angle _DK_cam_map_angle

extern TbSpriteData keepersprite_add[KEEPERSPRITE_ADD_NUM];
#pragma pack()
/******************************************************************************/
//extern unsigned char temp_cluedo_mode;
/******************************************************************************/
void do_a_plane_of_engine_columns_perspective(long a1, long a2, long a3, long a4);
void do_a_plane_of_engine_columns_cluedo(long a1, long a2, long a3, long a4);
void do_a_plane_of_engine_columns_isometric(long a1, long a2, long a3, long a4);
void find_gamut(void);
void fiddle_gamut(long a1, long a2);
int floor_height_for_volume_box(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y);
void process_isometric_map_volume_box(long x, long y, long z);
void process_frontview_map_volume_box(struct Camera *cam, unsigned char stl_width);
void rotpers_parallel_3(struct EngineCoord *epos, struct M33 *matx, long zoom);
void rotate_base_axis(struct M33 *matx, short a2, unsigned char a3);
void fill_in_points_perspective(long a1, long a2, struct MinMax *mm);
void fill_in_points_cluedo(long a1, long a2, struct MinMax *mm);
void fill_in_points_isometric(long a1, long a2, struct MinMax *mm);
void frame_wibble_generate(void);
void setup_rotate_stuff(long a1, long a2, long a3, long a4, long a5, long a6, long a7, long a8);

void process_keeper_sprite(short x, short y, unsigned short a3, short kspr_angle, unsigned char a5, long a6);
void draw_engine_number(struct Number *num);
void draw_engine_room_flagpole(struct RoomFlag *rflg);
void draw_status_sprites(long a1, long a2, struct Thing *thing, long a4);
void draw_keepsprite_unscaled_in_buffer(unsigned short kspr_n, short a2, unsigned char a3, unsigned char *a4);
void draw_mapwho_ariadne_path(struct Thing *thing);
void draw_jonty_mapwho(struct JontySpr *jspr);
void draw_map_volume_box(long cor1_x, long cor1_y, long cor2_x, long cor2_y, long floor_height_z, unsigned char color);
unsigned short choose_health_sprite(struct Thing *thing);

void update_engine_settings(struct PlayerInfo *player);
void display_drawlist(void);
void draw_view(struct Camera *cam, unsigned char a2);
void draw_frontview_engine(struct Camera *cam);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
