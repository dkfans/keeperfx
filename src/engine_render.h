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
#pragma pack(1)

#define POLY_POOL_SIZE 16777216 // Originally 262144, adjusted for view distance
#define Z_DRAW_DISTANCE_MAX 65536 // Originally 11232, adjusted for view distance
#define BUCKETS_COUNT 4098 // Originally 704, adjusted for view distance. (65536/16)+2

#define KEEPSPRITE_LENGTH 9149
#define KEEPERSPRITE_ADD_OFFSET 16384
#define KEEPERSPRITE_ADD_NUM 2048

enum QKinds {
    QK_PolygonStandard = 0,
    QK_PolygonSimple,
    QK_PolyMode0,
    QK_PolyMode4,
    QK_TrigMode2,
    QK_PolyMode5,
    QK_TrigMode3,
    QK_TrigMode6,
    QK_RotableSprite, // 8
    QK_PolygonNearFP,
    QK_Unknown10,
    QK_JontySprite,
    QK_CreatureShadow,
    QK_SlabSelector,
    QK_CreatureStatus,
    QK_TextureQuad,
    QK_FloatingGoldText, // 16
    QK_RoomFlagBottomPole,
    QK_JontyISOSprite,
    QK_RoomFlagStatusBox,
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

struct BucketKindPolygonStandard {
    struct BasicQ b;
    unsigned char field_5;
    unsigned short block;
    struct PolyPoint p1;
    struct PolyPoint p2;
    struct PolyPoint p3;
};

struct BucketKindPolygonSimple {
    struct BasicQ b;
    unsigned char field_5;
    unsigned short block;
    struct PolyPoint p1;
    struct PolyPoint p2;
    struct PolyPoint p3;
};

struct BucketKindPolyMode0 {
    struct BasicQ b;
    unsigned char colour;
    unsigned short x1;
    unsigned short y1;
    unsigned short x2;
    unsigned short y2;
    unsigned short x3;
    unsigned short y3;
};

struct BucketKindPolyMode4 {
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

struct BucketKindTrigMode2 {
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

struct BucketKindPolyMode5 {
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

struct BucketKindTrigMode3 {
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

struct BucketKindTrigMode6 {
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

struct BucketKindRotableSprite {
    struct BasicQ b;
    unsigned char field_5[3];
    long field_8;
    long field_C;
    long field_10;
    long field_14;
    char field_18;
    unsigned char field_19[3];
};

struct BucketKindPolygonNearFP {
    struct BasicQ b;
    unsigned char subtype;
    unsigned short block;
    struct PolyPoint p1;
    struct PolyPoint p2;
    struct PolyPoint p3;
    struct XYZ c1;
    struct XYZ c2;
    struct XYZ c3;
};

struct BucketKindBasicUnk10 {
    struct BasicQ b;
    unsigned char field_5;
    unsigned char field_6;
    unsigned char field_7;
    struct PolyPoint p1;
    struct PolyPoint p2;
    struct PolyPoint p3;
};

struct BucketKindJontySprite {  // BasicQ type 11,18
    struct BasicQ b;
    unsigned char field_5[3];
    struct Thing *thing;
    long scr_x;
    long scr_y;
    long field_14;
    unsigned char field_18;
    unsigned char field_19[3];
};

struct BucketKindCreatureShadow {
    struct BasicQ b;
    unsigned char field_5;
    unsigned short field_6;
    struct PolyPoint p1;
    struct PolyPoint p2;
    struct PolyPoint p3;
    struct PolyPoint p4;
    long angle;
    unsigned short anim_sprite;
    unsigned char thing_field48;
};

struct BucketKindSlabSelector {
    struct BasicQ b;
    unsigned char field_5;
    unsigned short field_6;
    struct PolyPoint p;
    unsigned char field_19[3];
};

struct BucketKindCreatureStatus { // sizeof = 24
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


struct BucketKindTexturedQuad { // sizeof = 46
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

struct BucketKindFloatingGoldText { // BasicQ type 16
    struct BasicQ b;
    unsigned char field_5[3];
    long x;
    long y;
    long lvl;
};

struct BucketKindRoomFlag { // BasicQ type 17,19
    struct BasicQ b;
    unsigned char field_5;
    unsigned short lvl;
    long x;
    long y;
};

struct EngineCoord { // sizeof = 28
  long view_width; // X screen position, probably not a width
  long view_height; // Y screen position, probably not a height
  unsigned short field_8; // Affects the drawing of offscreen triangles and something to do with Splittypes
  unsigned short field_A; // Lightness
  long field_C; // Distance to camera
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
extern unsigned char poly_pool[POLY_POOL_SIZE];
extern unsigned char *poly_pool_end;
extern long cells_away;
extern float hud_scale;
extern int creature_status_size;

extern struct MapVolumeBox map_volume_box;
extern long view_height_over_2;
extern long view_width_over_2;
extern long split_1;
extern long split_2;
extern long fade_max;

extern long ScrWidth;
extern long ScrHeight;
extern long ScrCenterX;
extern long ScrCenterY;

extern short mx;
extern short my;
extern short mz;

extern long floor_pointed_at_x;
extern long floor_pointed_at_y;
extern Offset vert_offset[3];
extern Offset hori_offset[3];
extern Offset high_offset[3];

extern unsigned char player_bit;

extern TbSpriteData *keepsprite[KEEPSPRITE_LENGTH];
extern struct HeapMgrHandle * heap_handle[KEEPSPRITE_LENGTH];
extern struct HeapMgrHeader *graphics_heap;
extern TbFileHandle file_handle;

extern long x_init_off;
extern long y_init_off;
extern struct Thing *thing_being_displayed;

extern unsigned char temp_cluedo_mode;
/******************************************************************************/

extern TbSpriteData keepersprite_add[KEEPERSPRITE_ADD_NUM];
#pragma pack()
/*****************************************************************************/
long interpolate(long variable_to_interpolate, long previous, long current);
long interpolate_angle(long variable_to_interpolate, long previous, long current);

int floor_height_for_volume_box(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y);
void frame_wibble_generate(void);
void setup_rotate_stuff(long a1, long a2, long a3, long a4, long a5, long a6, long a7, long a8);

void process_keeper_sprite(short x, short y, unsigned short a3, short kspr_angle, unsigned char a5, long a6);
void draw_status_sprites(long a1, long a2, struct Thing *thing);
void draw_map_volume_box(long cor1_x, long cor1_y, long cor2_x, long cor2_y, long floor_height_z, unsigned char color);

void update_engine_settings(struct PlayerInfo *player);
void draw_view(struct Camera *cam, unsigned char a2);
void draw_frontview_engine(struct Camera *cam);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
