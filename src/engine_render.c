/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_render.c
 *     Rendering the 3D view functions.
 * @par Purpose:
 *     Functions for displaying drawlist elements on screen.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     20 Mar 2009 - 20 May 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include <stddef.h>

#include "engine_render.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_math.h"
#include "bflib_planar.h"
#include "bflib_render.h"
#include "bflib_sprite.h"
#include "bflib_video.h"
#include "bflib_vidraw.h"
#include "config_creature.h"
#include "config_players.h"
#include "config_settings.h"
#include "config_spritecolors.h"
#include "config_terrain.h"
#include "config_keeperfx.h"
#include "creature_graphics.h"
#include "creature_states.h"
#include "creature_states_combt.h"
#include "creature_states_gardn.h"
#include "creature_states_lair.h"
#include "creature_states_mood.h"
#include "custom_sprites.h"
#include "engine_arrays.h"
#include "engine_camera.h"
#include "engine_lenses.h"
#include "engine_redraw.h"
#include "engine_textures.h"
#include "local_camera.h"
#include "front_simple.h"
#include "frontend.h"
#include "game_heap.h"
#include "game_lghtshdw.h"
#include "gui_draw.h"
#include "keeperfx.hpp"
#include "kjm_input.h"
#include "player_instances.h"
#include "sprites.h"
#include "thing_stats.h"
#include "thing_traps.h"
#include "vidfade.h"
#include "vidmode.h"

#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TO_FIXED(x)    ((x) << 16)
#define FROM_FIXED(x)    ((x) >> 16)

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
    QK_BasicPolygon,
    QK_JontySprite,
    QK_CreatureShadow,
    QK_SlabSelector,
    QK_CreatureStatus,
    QK_TextureQuad,
    QK_FloatingGoldText, // 16
    QK_RoomFlagBottomPole,
    QK_JontyISOSprite,
    QK_RoomFlagStatusBox,
    QK_ListEnd,
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
    unsigned short block;
    struct PolyPoint vertex_first;
    struct PolyPoint vertex_second;
    struct PolyPoint vertex_third;
};

struct BucketKindPolygonSimple {
    struct BasicQ b;
    unsigned short block;
    struct PolyPoint vertex_first;
    struct PolyPoint vertex_second;
    struct PolyPoint vertex_third;
};

struct BucketKindPolyMode0 {
    struct BasicQ b;
    unsigned char colour;
    unsigned short vertex_first_x;
    unsigned short vertex_first_y;
    unsigned short vertex_second_x;
    unsigned short vertex_second_y;
    unsigned short vertex_third_x;
    unsigned short vertex_third_y;
};

struct BucketKindPolyMode4 {
    struct BasicQ b;
    unsigned char colour;
    unsigned short vertex_first_x;
    unsigned short vertex_first_y;
    unsigned short vertex_second_x;
    unsigned short vertex_second_y;
    unsigned short vertex_third_x;
    unsigned short vertex_third_y;
    unsigned char texture_vertex_first;
    unsigned char texture_vertex_second;
    unsigned char texture_vertex_third;
};

struct BucketKindTrigMode2 {
    struct BasicQ b;
    unsigned short vertex_first_x;
    unsigned short vertex_first_y;
    unsigned short vertex_second_x;
    unsigned short vertex_second_y;
    unsigned short vertex_third_x;
    unsigned short vertex_third_y;
    unsigned char texture_u_first;
    unsigned char texture_v_first;
    unsigned char texture_u_second;
    unsigned char texture_v_second;
    unsigned char texture_u_third;
    unsigned char texture_v_third;
};

struct BucketKindPolyMode5 {
    struct BasicQ b;
    unsigned short vertex_first_x;
    unsigned short vertex_first_y;
    unsigned short vertex_second_x;
    unsigned short vertex_second_y;
    unsigned short vertex_third_x;
    unsigned short vertex_third_y;
    unsigned char texture_u_first;
    unsigned char texture_v_first;
    unsigned char texture_u_second;
    unsigned char texture_v_second;
    unsigned char texture_u_third;
    unsigned char texture_v_third;
    unsigned char texture_w_first;
    unsigned char texture_w_second;
    unsigned char texture_w_third;
};

struct BucketKindTrigMode3 {
    struct BasicQ b;
    unsigned short vertex_first_x;
    unsigned short vertex_first_y;
    unsigned short vertex_second_x;
    unsigned short vertex_second_y;
    unsigned short vertex_third_x;
    unsigned short vertex_third_y;
    unsigned char texture_u_first;
    unsigned char texture_v_first;
    unsigned char texture_u_second;
    unsigned char texture_v_second;
    unsigned char texture_u_third;
    unsigned char texture_v_third;
};

struct BucketKindTrigMode6 {
    struct BasicQ b;
    unsigned short vertex_first_x;
    unsigned short vertex_first_y;
    unsigned short vertex_second_x;
    unsigned short vertex_second_y;
    unsigned short vertex_third_x;
    unsigned short vertex_third_y;
    unsigned char texture_u_first;
    unsigned char texture_v_first;
    unsigned char texture_u_second;
    unsigned char texture_v_second;
    unsigned char texture_u_third;
    unsigned char texture_v_third;
    unsigned char texture_w_first;
    unsigned char texture_w_second;
    unsigned char texture_w_third;
};

struct BucketKindRotableSprite {
    struct BasicQ b;
    long clip_flags;
    long depth_fade;
};

struct BucketKindPolygonNearFP {
    struct BasicQ b;
    unsigned char subtype;
    unsigned short block;
    struct PolyPoint vertex_first;
    struct PolyPoint vertex_second;
    struct PolyPoint vertex_third;
    struct XYZ coordinate_first;
    struct XYZ coordinate_second;
    struct XYZ coordinate_third;
};

struct BucketKindBasicUnk10 {
    struct BasicQ b;
    unsigned char color_value;
    struct PolyPoint vertex_first;
    struct PolyPoint vertex_second;
    struct PolyPoint vertex_third;
};

struct BucketKindJontySprite {  // BasicQ type 11,18
    struct BasicQ b;
    struct Thing *thing;
    long scr_x;
    long scr_y;
    long depth_fade;
};

struct BucketKindCreatureShadow {
    struct BasicQ b;
    unsigned short color_value;
    struct PolyPoint vertex_first;
    struct PolyPoint vertex_second;
    struct PolyPoint vertex_third;
    struct PolyPoint vertex_fourth;
    long angle;
    unsigned short anim_sprite;
    unsigned char current_frame;
};

struct BucketKindSlabSelector {
    struct BasicQ b;
    unsigned short color_value;
    struct PolyPoint p;
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
    unsigned char orient;
    long texture_idx;
    long texture_x;
    long texture_y;
    long zoom_x;
    long zoom_y;
    long shade_intensity0;
    long shade_intensity1;
    long shade_intensity2;
    long shade_intensity3;
    long marked_mode;
};

struct BucketKindFloatingGoldText { // BasicQ type 16
    struct BasicQ b;
    long x;
    long y;
    long lvl;
};

struct BucketKindRoomFlag { // BasicQ type 17,19
    struct BasicQ b;
    unsigned short lvl;
    long x;
    long y;
};



struct EngineCol {
    struct EngineCoord cors[16];
};

struct SideOri {
    unsigned char back_texture_index;
    unsigned char top_texture_index;
    unsigned char front_texture_index;
    unsigned char bottom_texture_index;
};

/******************************************************************************/
static const struct SideOri sideoris[] = {
    { 0,  1,  2,  3},
    { 0,  0,  3,  2},
    { 1,128,  2,  1},
    { 0,  3,128,  2},
    { 3,  0,  1,  0},
    { 3,  2,  1,  0},
    {128, 3,  0,  1},
    { 2,  0,  1,  2},
    { 3,  0,  0,  1},
    { 0,  3,  2,128},
};

long const x_offs[] =  { 0, 1, 1, 0};
long const y_offs[] =  { 0, 0, 1, 1};
long const x_step1[] = { 0,-1, 0, 1};
long const y_step1[] = { 1, 0,-1, 0};
long const x_step2[] = { 1, 0,-1, 0};
long const y_step2[] = { 0, 1, 0,-1};
long const orient_table_xflip[] =  {0, 0, 1, 1};
long const orient_table_yflip[] =  {0, 1, 1, 0};
long const orient_table_rotate[] = {0, 1, 0, 1};
long const orient_to_mapU1[] = { 0x00, 0x1F0000, 0x1F0000, 0x00 };
long const orient_to_mapU2[] = { 0x1F0000, 0x1F0000, 0x00, 0x00 };
long const orient_to_mapU3[] = { 0x1F0000, 0x00, 0x00, 0x1F0000 };
long const orient_to_mapU4[] = { 0x00, 0x00, 0x1F0000, 0x1F0000 };
long const orient_to_mapV1[] = { 0x00, 0x00, 0x1F0000, 0x1F0000 };
long const orient_to_mapV2[] = { 0x00, 0x1F0000, 0x1F0000, 0x00 };
long const orient_to_mapV3[] = { 0x1F0000, 0x1F0000, 0x00, 0x00 };
long const orient_to_mapV4[] = { 0x1F0000, 0x00, 0x00, 0x1F0000 };

unsigned char const height_masks[] = {
  0, 1, 2, 2, 3, 3, 3, 3,
  4, 4, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6,
  7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
};

// View distance related
struct MinMax minmaxs[MINMAX_LENGTH];
unsigned char *getpoly;
unsigned char poly_pool[POLY_POOL_SIZE];
unsigned char *poly_pool_end;
struct BasicQ *buckets[BUCKETS_COUNT];
long cells_away;
long max_i_can_see;
const int MAX_I_CAN_SEE_OVERHEAD = (MINMAX_LENGTH/2)-2;
struct EngineCol ecs1[MINMAX_LENGTH-1];
struct EngineCol ecs2[MINMAX_LENGTH-1];
struct EngineCol *front_ec;
struct EngineCol *back_ec;
float hud_scale;

int line_box_size = 150; // Default value, overwritten by cfg setting
int creature_status_size = 16; // Default value, overwritten by cfg setting
static int water_wibble_angle = 0;
static float render_water_wibble = 0; // Rendering float
static unsigned long render_problems;
static long render_prob_kind;

Offset vert_offset[3];
Offset hori_offset[3];
Offset high_offset[3];
long x_init_off;
long y_init_off;
long floor_pointed_at_x;
long floor_pointed_at_y;
long box_lag_compensation_x;
long box_lag_compensation_y;

static long fade_scaler;
static long fade_way_out;
static long map_roll;
static long map_tilt;
static long view_alt;
static long fade_min;
static long fade_range;
static long depth_init_off;
static int normal_shade_left;
static int normal_shade_right;
static long apos;
static long bpos;
static long split1at;
static long split2at;
static long map_x_pos;
static long map_y_pos;
static long map_z_pos;
static int normal_shade_front;
static int normal_shade_back;
static long me_distance;
static long thelens;
static long fade_mmm;
static long spr_map_angle;
static long lfade_max;
static long lfade_min;
static unsigned char thing_being_displayed_is_creature;
static long global_scaler;
static long water_source_cutoff;
static long water_y_offset;
static long cam_map_angle;

static struct M33 camera_matrix;
struct EngineCoord object_origin;

short mx;
short my;
short mz;
unsigned char temp_cluedo_mode; // This is true(1) if the "short wall" have been enabled in the graphics options
struct Thing *thing_being_displayed;

TbSpriteData *keepsprite[KEEPSPRITE_LENGTH];
TbSpriteData sprite_heap_handle[KEEPSPRITE_LENGTH];
struct HeapMgrHeader *graphics_heap;
TbFileHandle jty_file_handle;

struct MapVolumeBox map_volume_box;
long view_height_over_2;
long view_width_over_2;
long z_threshold_near;
long split_2;
long fade_max;

static const char splittypes[64] = {
    0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 5, 5, 0, 1, 5, 5,
    0, 2, 2, 2, 3, 4, 4, 4, 3, 4, 8, 8, 3, 4, 8, 8,
    0, 2, 6, 6, 3, 4, 9, 9, 7, 10, 11, 11, 7, 10, 11, 11,
    0, 2, 6, 6, 3, 4, 9, 9, 7, 10, 11, 11, 7, 10, 11, 11
};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
static void do_map_who(short tnglist_idx);
static void (*render_sprite_debug_fn) (struct Thing*, long scrpos_x, long scrpos_y) = NULL;
static int render_sprite_debug_level = 0;
static void draw_keepsprite_unscaled_in_buffer(unsigned short kspr_n, short angle, unsigned char current_frame, unsigned char *outbuf);
static void draw_jonty_mapwho(struct BucketKindJontySprite *jspr);
/******************************************************************************/

static void calculate_hud_scale(struct Camera *cam) {
    // hud_scale is the current camera zoom converted to a percentage that ranges between base level zoom and fully zoomed out.
    // HUD items: creature status flowers, room flags, popup gold numbers. They scale with the zoom.
    float range_input = cam->zoom;
    float range_min;
    float range_max;
    switch (cam->view_mode) {
        case PVM_IsoWibbleView:
        case PVM_IsoStraightView:
            range_min = CAMERA_ZOOM_MIN; // Fully zoomed out
            range_max = 4100; // Base zoom level
            break;
        case PVM_FrontView:
            range_min = FRONTVIEW_CAMERA_ZOOM_MIN; // Fully zoomed out
            range_max = 32768; // Base zoom level
            break;
        default:
            hud_scale = 0;
            return;
    }
    if (range_input < range_min) {
        range_input = range_min;
    } else if (range_input > range_max) {
        range_input = range_max;
    }
    hud_scale = ((range_input - range_min)) / (range_max - range_min);
}

float interpolate(float variable_to_interpolate, long previous, long current)
{
    if (is_feature_on(Ft_DeltaTime) == false || game.frame_skip > 0) {
        return current;
    }
    // future: by using the predicted future position in the interpolation calculation, we can remove input lag (or visual lag).
    long future = current + (current - previous);
    // 0.5 is definitely accurate. Tested by rotating the camera while comparing the minimap's rotation with the camera's rotation in a video recording.
    float desired_value = LbLerp(current, future, 0.5);
    return LbLerp(variable_to_interpolate, desired_value, game.delta_time);
}

float interpolate_angle(float variable_to_interpolate, float previous, float current)
{
    if (is_feature_on(Ft_DeltaTime) == false || game.frame_skip > 0) {
        return current;
    }
    float future = current + (current - previous);
    float desired_value = lerp_angle(current, future, 0.5);
    float result = lerp_angle(variable_to_interpolate, desired_value, game.delta_time);
    float result_change = LbFmodf((result - current) + DEGREES_180, DEGREES_360) - DEGREES_180;
    if (result_change > -0.5f && result_change < 0.5f) {
        return current;
    }
    return result;
}

void interpolate_thing(struct Thing *thing)
{
    // Note: if delta_time is off the interpolated position will also reflect that

    if (thing->creation_turn == game.play_gameturn-1 || game.play_gameturn - thing->last_turn_drawn > 1 ) {
        // Set initial interp position when either Thing has just been created or goes off camera then comes back on camera
        thing->interp_mappos = thing->mappos;
        thing->interp_floor_height = thing->floor_height;

        if (thing->interp_mappos.z.val == 65534) { // Fixes an odd bug where thing->mappos.z.val is briefly 65534 (for 1 turn) in certain situations, which can mess up the interpolation and cause things to fall from the sky.
            thing->interp_mappos.z.val = thing->interp_floor_height;
        }
    } else {
        // Interpolate position every frame
        thing->interp_mappos.x.val = interpolate(thing->interp_mappos.x.val, thing->previous_mappos.x.val, thing->mappos.x.val);
        thing->interp_mappos.z.val = interpolate(thing->interp_mappos.z.val, thing->previous_mappos.z.val, thing->mappos.z.val);
        thing->interp_mappos.y.val = interpolate(thing->interp_mappos.y.val, thing->previous_mappos.y.val, thing->mappos.y.val);
        thing->interp_floor_height = interpolate(thing->interp_floor_height, thing->previous_floor_height, thing->floor_height);

        // Cancel interpolation if distance to interpolate is too far. This is a catch-all to solve any remaining interpolation bugs.
        if ((abs(thing->interp_mappos.x.val-thing->mappos.x.val) >= 10000) ||
            (abs(thing->interp_mappos.y.val-thing->mappos.y.val) >= 10000) ||
            (abs(thing->interp_mappos.z.val-thing->mappos.z.val) >= 10000))
        {
            ERRORLOG("The %s index %d owned by player %d moved an unrealistic distance((%d,%d,%d) to (%d,%d,%d)), refusing interpolation."
                ,thing_model_name(thing), (int)thing->index, (int)thing->owner, thing->interp_mappos.x.stl.num, thing->interp_mappos.y.stl.num, thing->interp_mappos.z.stl.num, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing->mappos.z.stl.num);
            thing->interp_mappos = thing->mappos;
            thing->interp_floor_height = thing->floor_height;
        }
    }
}

static void get_floor_pointed_at(long x, long y, int32_t *floor_x, int32_t *floor_y)
{
    long long ofs_x;
    long long ofs_y;
    long long sor_hp;
    long long sor_hn;
    long long sor_vp;
    long long sor_vn;
    long long der_hp;
    long long der_hn;
    long long der_vp;
    long long der_vn;
    long long div_v;
    long long div_h;
    if ( (vert_offset[1] == 0) && (hori_offset[1] == 0) )
    {
        *floor_x = 0;
        *floor_y = 0;
        return;
    }
    ofs_x = (long long)x - (long long)x_init_off;
    ofs_y = (long long)y - (long long)y_init_off;
    sor_vp = (((long long)vert_offset[1] * ofs_x) / 2LL);
    sor_vn = (((long long)vert_offset[0] * ofs_y) / 2LL);
    der_vp = ((long long)hori_offset[0] * (long long)vert_offset[1]) / 8LL;
    der_vn = ((long long)vert_offset[0] * (long long)hori_offset[1]) / 8LL;
    sor_hp = (((long long)hori_offset[1] * ofs_x) / 2LL);
    sor_hn = (((long long)hori_offset[0] * ofs_y) / 2LL);
    der_hp = ((long long)vert_offset[0] * (long long)hori_offset[1]) / 8LL;
    der_hn = ((long long)hori_offset[0] * (long long)vert_offset[1]) / 8LL;
    div_v = (der_vp - der_vn) >> 8;
    div_h = (der_hp - der_hn) >> 8;
    if (div_v == 0 || div_h == 0)
    {
        ERRORLOG("Invalid floor value from %ld,%ld", x, y);
        *floor_x = 0;
        *floor_y = 0;
        return;
    }
    *floor_y = ((sor_vp - sor_vn) / div_v) >> 2;
    *floor_x = ((sor_hp - sor_hn) / div_h) >> 2;
}

static long compute_cells_away(void) // For overhead view, not for 1st person view
{
    long half_width;
    long half_height;
    int32_t xmin;
    int32_t ymin;
    int32_t xmax;
    int32_t ymax;
    int32_t xcell;
    int32_t ycell;
    struct PlayerInfo *player;
    long ncells_a;
    player = get_my_player();
    half_width = (player->engine_window_width >> 1);
    half_height = (player->engine_window_height >> 1);
    xcell = ((half_width<<1) + (half_width>>4))/pixel_size - player->engine_window_x/pixel_size;
    ycell = ((8 * high_offset[1]) >> 8) - (half_width>>4)/pixel_size - player->engine_window_y/pixel_size;
    get_floor_pointed_at(xcell, ycell, &xmax, &ymax);
    xcell = (half_width)/pixel_size - player->engine_window_x/pixel_size;
    ycell = (half_height)/pixel_size - player->engine_window_y/pixel_size;
    get_floor_pointed_at(xcell, ycell, &xmin, &ymin);
    xcell = abs(ymax - ymin);
    ycell = abs(xmax - xmin);
    if (ycell >= xcell) {
        ncells_a = ycell + (xcell >> 1);
    } else {
        ncells_a = xcell + (ycell >> 1);
    }
    ncells_a += 2;
    if (ncells_a > MAX_I_CAN_SEE_OVERHEAD) {
        ncells_a = MAX_I_CAN_SEE_OVERHEAD;
    }
    return ncells_a;
}

static void init_coords_and_rotation(struct EngineCoord *origin,struct M33 *matx)
{
    origin->x = 0;
    origin->y = 0;
    origin->z = 0;
    matx->r[0].v[0] = 0x4000u;
    matx->r[0].v[1] = 0;
    matx->r[0].v[2] = 0;
    matx->r[1].v[0] = 0;
    matx->r[1].v[1] = 0x4000u;
    matx->r[1].v[2] = 0;
    matx->r[2].v[0] = 0;
    matx->r[2].v[1] = 0;
    matx->r[2].v[2] = 0x4000u;
}

static void update_fade_limits(long ncells_a)
{
    fade_max = (ncells_a << 8);
    fade_scaler = (ncells_a << 8);
    fade_way_out = (ncells_a + 1) << 8;
    fade_min = (768 * ncells_a) / 4;
    z_threshold_near = (split1at << 8);
    split_2 = (split2at << 8);
}

static void update_normal_shade(struct M33 *matx)
{
    normal_shade_left = matx->r[2].v[0];
    normal_shade_right = -matx->r[2].v[0];
    normal_shade_back = -matx->r[2].v[2];
    normal_shade_front = matx->r[2].v[2];
    if (normal_shade_front < 0)
      normal_shade_front = 0;
    if (normal_shade_back < 0)
      normal_shade_back = 0;
    if (normal_shade_left < 0)
      normal_shade_left = 0;
    if (normal_shade_right < 0)
      normal_shade_right = 0;
}

void update_engine_settings(struct PlayerInfo *player)
{
    switch (settings.video_detail_level)
    {
    case 0:
        split1at = 4;
        split2at = 3;
        break;
    case 1:
        split1at = 3;
        split2at = 2;
        break;
    case 2:
        split1at = 2;
        split2at = 1;
        break;
    case 3:
    default:
        split1at = 0;
        split2at = 0;
        break;
    }
    me_pointed_at = NULL;
    me_distance = 100000000;
    max_i_can_see = get_max_i_can_see_from_settings();
    if (lens_mode != 0)
      temp_cluedo_mode = 0;
    else
      temp_cluedo_mode = settings.video_cluedo_mode;
    thing_pointed_at = NULL;
}

/**
 * Sets the reserved amount of poly pool entries.
 * Entries which are reserved won't be filled by standard rendering items, even if the queue is full.
 * @param nitems
 */
static void poly_pool_end_reserve(int nitems)
{
    poly_pool_end = &poly_pool[sizeof(poly_pool)-(nitems*sizeof(struct BucketKindSlabSelector))];
}

static TbBool is_free_space_in_poly_pool(int nitems)
{
    return (getpoly+(nitems*sizeof(struct BucketKindSlabSelector)) <= poly_pool_end);
}

static void rotpers_parallel_3(struct EngineCoord *epos, struct M33 *matx, long zoom)
{
    long factor_w;
    long factor_h;
    long inp_x;
    long inp_y;
    long inp_z;
    long long out_x;
    long long out_y;
    inp_x = epos->x;
    inp_y = epos->y;
    inp_z = epos->z;
    out_x = ((long long)(inp_z * matx->r[0].v[2]) + ((long long)(inp_y + matx->r[0].v[0]) * (long long)(inp_x + matx->r[0].v[1])) - (long long)matx->r[0].v[3] - (long long)(inp_x * inp_y)) >> 14;
    epos->x = out_x;
    out_y = ((long long)(inp_z * matx->r[1].v[2]) + ((long long)(inp_y + matx->r[1].v[0]) * (long long)(inp_x + matx->r[1].v[1])) - (long long)matx->r[1].v[3] - (long long)(inp_x * inp_y)) >> 14;
    epos->y = out_y;
    epos->z = ((long long)(inp_z * matx->r[2].v[2]) + ((long long)(inp_y + matx->r[2].v[0]) * (long long)(inp_x + matx->r[2].v[1])) - (long long)matx->r[2].v[3] - (long long)(inp_x * inp_y)) >> 14;
    factor_w = (long)view_width_over_2 + (zoom * out_x >> 16);
    epos->view_width = factor_w;
    factor_h = (long)view_height_over_2 - (zoom * out_y >> 16);
    epos->view_height = factor_h;
    if (factor_w < 0)
    {
        epos->clip_flags |= 0x0008;
    } else
    if (vec_window_width <= factor_w)
    {
        epos->clip_flags |= 0x0010;
    }
    if (factor_h < 0)
    {
        epos->clip_flags |= 0x0020;
    } else
    if (factor_h >= vec_window_height)
    {
        epos->clip_flags |= 0x0040;
    }
    epos->clip_flags |= 0x0400;
}

static void base_vec_normalisation(struct M33 *matx, unsigned char a2)
{
    struct M31 *vec;
    vec = &matx->r[a2];
    long rv0;
    long rv1;
    long rv2;
    long rvlen;
    rv0 = vec->v[0];
    rv1 = vec->v[1];
    rv2 = vec->v[2];
    rvlen = LbSqrL(rv0 * rv0 + rv1 * rv1 + rv2 * rv2);
    vec->v[0] = (rv0 << 14) / rvlen;
    vec->v[1] = (rv1 << 14) / rvlen;
    vec->v[2] = (rv2 << 14) / rvlen;
}

static void vec_cross_prod(struct M31 *outvec, const struct M31 *vec2, const struct M31 *vec3)
{
    outvec->v[0] = vec3->v[2] * vec2->v[1] - vec3->v[1] * vec2->v[2];
    outvec->v[1] = vec3->v[0] * vec2->v[2] - vec3->v[2] * vec2->v[0];
    outvec->v[2] = vec3->v[1] * vec2->v[0] - vec3->v[0] * vec2->v[1];
}

static void matrix_transform(struct M31 *outvec, const struct M33 *matx, const struct M31 *vec2)
{
    outvec->v[0] = matx->r[0].v[2] * vec2->v[2] + matx->r[0].v[0] * vec2->v[0] + matx->r[0].v[1] * vec2->v[1];
    outvec->v[1] = matx->r[1].v[2] * vec2->v[2] + matx->r[1].v[0] * vec2->v[0] + matx->r[1].v[1] * vec2->v[1];
    outvec->v[2] = matx->r[2].v[2] * vec2->v[2] + matx->r[2].v[1] * vec2->v[1] + matx->r[2].v[0] * vec2->v[0];
}

static void rotate_base_axis(struct M33 *matx, short angle, unsigned char axis)
{
    unsigned char scor0;
    unsigned char scor1;
    unsigned char scor2;
    switch (axis)
    {
    case 1:
        scor0 = 1;
        scor1 = 2;
        scor2 = 0;
        break;
    case 2:
        scor0 = 0;
        scor1 = 2;
        scor2 = 1;
        break;
    case 3:
        scor0 = 0;
        scor1 = 1;
        scor2 = 2;
        break;
    default:
        ERRORLOG("Bad axis");
        scor0 = 0;
        scor1 = 1;
        scor2 = 2;
        break;
    }

    struct M33 matt;
    {
#define TRIG_LIMIT (1 << (LbFPMath_TrigmBits - 2))
        int angle_sin;
        int angle_cos;
        angle_sin = LbSinL(angle) >> 2;
        angle_cos = LbCosL(angle) >> 2;
        long matrix_x_component;
        long matrix_y_component;
        long matrix_z_component;
        matrix_x_component = matx->r[scor2].v[0];
        matrix_z_component = matx->r[scor2].v[2];
        matrix_y_component = matx->r[scor2].v[1];
        long shf0;
        long shf1;
        long shf2;
        long mag0;
        long mag1;
        long mag2;
        matt.r[0].v[0] = (matrix_x_component * matrix_x_component >> 14) + (angle_cos * (TRIG_LIMIT - (matrix_x_component * matrix_x_component >> 14)) >> 14);
        matt.r[1].v[1] = (matrix_y_component * matrix_y_component >> 14) + (angle_cos * (TRIG_LIMIT - (matrix_y_component * matrix_y_component >> 14)) >> 14);
        matt.r[2].v[2] = (matrix_z_component * matrix_z_component >> 14) + (angle_cos * (TRIG_LIMIT - (matrix_z_component * matrix_z_component >> 14)) >> 14);
        mag2 = (TRIG_LIMIT - angle_cos) * (matrix_y_component * matrix_x_component >> 14) >> 14;
        shf2 = angle_sin * matrix_z_component >> 14;
        mag1 = (TRIG_LIMIT - angle_cos) * (matrix_x_component * matrix_z_component >> 14) >> 14;
        shf1 = angle_sin * matrix_y_component >> 14;
        mag0 = (TRIG_LIMIT - angle_cos) * (matrix_y_component * matrix_z_component >> 14) >> 14;
        shf0 = angle_sin * matrix_x_component >> 14;
        matt.r[0].v[1] = mag2 - shf2;
        matt.r[0].v[2] = mag1 + shf1;
        matt.r[1].v[2] = mag0 - shf0;
        matt.r[1].v[0] = mag2 + shf2;
        matt.r[2].v[0] = mag1 - shf1;
        matt.r[2].v[1] = mag0 + shf0;
#undef TRIG_LIMIT
    }

    struct M31 locvec;
    matrix_transform(&locvec, &matt, &matx->r[scor0]);
    matx->r[scor0].v[0] = locvec.v[0] >> 14;
    matx->r[scor0].v[1] = locvec.v[1] >> 14;
    matx->r[scor0].v[2] = locvec.v[2] >> 14;
    matrix_transform(&locvec, &matt, &matx->r[scor1]);
    matx->r[scor1].v[0] = locvec.v[0] >> 14;
    matx->r[scor1].v[1] = locvec.v[1] >> 14;
    matx->r[scor1].v[2] = locvec.v[2] >> 14;
    base_vec_normalisation(matx, 2);

    vec_cross_prod(&locvec, &matx->r[2], &matx->r[0]);
    matx->r[1].v[0] = locvec.v[0] >> 14;
    matx->r[1].v[1] = locvec.v[1] >> 14;
    matx->r[1].v[2] = locvec.v[2] >> 14;
    base_vec_normalisation(matx, 1);

    vec_cross_prod(&locvec, &matx->r[1], &matx->r[2]);
    matx->r[0].v[0] = locvec.v[0] >> 14;
    matx->r[0].v[1] = locvec.v[1] >> 14;
    matx->r[0].v[2] = locvec.v[2] >> 14;
    base_vec_normalisation(matx, 0);

    matx->r[0].v[3] = matx->r[0].v[0] * matx->r[0].v[1];
    matx->r[1].v[3] = matx->r[1].v[0] * matx->r[1].v[1];
    matx->r[2].v[3] = matx->r[2].v[0] * matx->r[2].v[1];
}

struct WibbleTable *get_wibble_from_table(struct Camera *cam, long table_index, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    if (table_index < 0 || table_index >= WIBBLE_TABLE_SIZE) {
        ERRORLOG("Invalid wibble table index %ld", table_index);
        return &blank_wibble_table[0];
    }
    if (cam->view_mode == PVM_IsoWibbleView || cam->view_mode == PVM_CreatureView)
    {
        return &wibble_table[table_index];
    }
    else if (cam->view_mode == PVM_IsoStraightView)
    {
        struct SlabMap *slb = get_slabmap_for_subtile(stl_slab_center_subtile(stl_x), stl_slab_center_subtile(stl_y));
         // additional checks needed to keep straight edges around liquid with liquid wibble mode
        struct SlabMap *slb2 = get_slabmap_for_subtile(stl_slab_center_subtile(stl_x), stl_slab_center_subtile(stl_y+1));
        struct SlabMap *slb3 = get_slabmap_for_subtile(stl_slab_center_subtile(stl_x-1), stl_slab_center_subtile(stl_y));
        if (slab_kind_is_liquid(slb3->kind) && slab_kind_is_liquid(slb2->kind) && slab_kind_is_liquid(slb->kind))
        {
            return &wibble_table[table_index];
        }
    }
    return &blank_wibble_table[table_index];
}

static struct BasicQ *get_bucket_item(int min_cor_z, enum QKinds kind, size_t size)
{
    if (getpoly >= poly_pool_end)
    {
        return NULL;
    }

    int bckt_idx = min_cor_z / BUCKETS_STEP;
    if (bckt_idx < 0)
    {
        bckt_idx = 0;
    }
    else if (bckt_idx > BUCKETS_COUNT-2)
    {
        bckt_idx = BUCKETS_COUNT-2;
    }
    struct BasicQ * kspr;
    kspr = (struct BasicQ *)getpoly;
    getpoly += size;
    kspr->next = buckets[bckt_idx];
    kspr->kind = kind;
    buckets[bckt_idx] = (struct BasicQ *)kspr;
    return kspr;
}

static void fill_in_points_perspective(struct Camera *cam, long bstl_x, long bstl_y, struct MinMax *mm)
{
    if ((bstl_y < 0) || (bstl_y > game.map_subtiles_y-1)) {
        return;
    }
    long mmin;
    long mmax;
    mmin = min(mm[0].min,mm[1].min);
    mmax = max(mm[0].max,mm[1].max);
    if (mmin + bstl_x < 1)
      mmin = 1 - bstl_x;
    if (mmax + bstl_x > game.map_subtiles_x)
      mmax = game.map_subtiles_x - bstl_x;
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    stl_y = bstl_y;
    stl_x = mmin + bstl_x;
    apos += subtile_coord(mmin,0);
    struct EngineCol *ecol;
    ecol = &front_ec[mmin + MINMAX_ALMOST_HALF];
    unsigned long mask_unrev;
    {
        struct Column *col;
        col = get_column(game.unrevealed_column_idx);
        mask_unrev = col->solidmask + 65536;
    }
    struct Map *mapblk;
    struct Column *col;
    unsigned long pfulmask_or;
    unsigned long pfulmask_and;
    {
        unsigned long mask_cur;
        unsigned long mask_yp;
        mask_cur = mask_unrev;
        mask_yp = mask_unrev;
        mapblk = get_map_block_at(stl_x-1, stl_y+1);
        if (map_block_revealed(mapblk, my_player_number)) {
            col = get_map_column(mapblk);
            mask_cur = col->solidmask;
        }
        mapblk = get_map_block_at(stl_x-1, stl_y);
        if (map_block_revealed(mapblk, my_player_number)) {
            col = get_map_column(mapblk);
            mask_yp = col->solidmask;
        }
        pfulmask_or = mask_cur | mask_yp;
        pfulmask_and = mask_cur & mask_yp;
    }

    int wib_x;
    int wib_y;
    int wib_v;
    wib_y = (stl_y + 1) & 3;
    int idxx;
    for (idxx=mmax-mmin+1; idxx > 0; idxx--)
    {
        unsigned long mask_cur;
        unsigned long mask_yp;
        mask_cur = mask_unrev;
        mask_yp = mask_unrev;
        mapblk = get_map_block_at(stl_x, stl_y+1);
        wib_v = get_mapblk_wibble_value(mapblk);
        if (map_block_revealed(mapblk, my_player_number)) {
            col = get_map_column(mapblk);
            mask_cur = col->solidmask;
        }
        mapblk = get_map_block_at(stl_x, stl_y);
        if (map_block_revealed(mapblk, my_player_number)) {
            col = get_map_column(mapblk);
            mask_yp = col->solidmask;
        }
        unsigned long nfulmask_or;
        unsigned long nfulmask_and;
        nfulmask_or = mask_cur | mask_yp;
        nfulmask_and = mask_cur & mask_yp;
        unsigned long fulmask_or;
        unsigned long fulmask_and;
        fulmask_or = nfulmask_or | pfulmask_or;
        fulmask_and = nfulmask_and & pfulmask_and;
        pfulmask_or = nfulmask_or;
        pfulmask_and = nfulmask_and;
        int lightness;
        lightness = 0;
        if ((fulmask_or & 0x10000) == 0)
            lightness = get_subtile_lightness(&game.lish,stl_x,stl_y+1);
        long hmin;
        long hmax;
        hmax = height_masks[fulmask_or & 0xff];
        hmin = floor_height_table[fulmask_and & 0xff];
        struct EngineCoord *ecord;
        ecord = &ecol->cors[hmin];
        long hpos;
        hpos = subtile_coord(hmin,0) - view_alt;
        wib_x = stl_x & 3;
        struct WibbleTable *wibl;
        wibl = get_wibble_from_table(cam, 32 * wib_v + wib_x + (wib_y << 2), stl_x, stl_y);
        int idxh;
        for (idxh = hmax-hmin+1; idxh > 0; idxh--)
        {
            ecord->x = apos + wibl->offset_x;
            ecord->y = hpos + wibl->offset_y;
            ecord->z = bpos + wibl->offset_z;
            ecord->clip_flags = 0;
            lightness += wibl->lightness_offset;
            if (lightness < 0)
                lightness = 0;
            if (lightness > 16128)
                lightness = 16128;
            ecord->shade_intensity = lightness;
            wibl += 2;
            hpos += COORD_PER_STL;
            rotpers(ecord, &camera_matrix);
            ecord++;
        }
        wibl -= 2;
        // Set ceiling
        mapblk = get_map_block_at(stl_x, stl_y+1);
        wib_v = get_mapblk_wibble_value(mapblk);
        hpos = subtile_coord(get_mapblk_filled_subtiles(mapblk),0) - view_alt;
        if (wib_v == 2)
        {
            wibl = get_wibble_from_table(cam, wib_x + 2 * (hmax + 2 * wib_y - hmin) + 32, stl_x, stl_y);
        }
        ecord = &ecol->cors[8];
        {
            ecord->x = apos + wibl->offset_x;
            ecord->y = hpos + wibl->offset_y;
            ecord->z = bpos + wibl->offset_z;
            ecord->clip_flags = 0;
            // Use lightness from last cube
            ecord->shade_intensity = lightness;
            rotpers(ecord, &camera_matrix);
        }
        stl_x++;
        ecol++;
        apos += COORD_PER_STL;
    }
}

static void fill_in_points_cluedo(struct Camera *cam, long bstl_x, long bstl_y, struct MinMax *mm)
{
    if ((bstl_y < 0) || (bstl_y > game.map_subtiles_y-1)) {
        return;
    }
    long mmin;
    long mmax;
    mmin = min(mm[0].min,mm[1].min);
    mmax = max(mm[0].max,mm[1].max);
    if (mmin + bstl_x < 1) {
        mmin = 1 - bstl_x;
    }
    if (mmax + bstl_x > game.map_subtiles_x) {
        mmax = game.map_subtiles_x - bstl_x;
    }
    if (mmax < mmin) {
        return;
    }
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    stl_y = bstl_y;
    stl_x = mmin + bstl_x;
    apos += (mmin << 8);
    struct EngineCol *ecol;
    ecol = &front_ec[mmin + MINMAX_ALMOST_HALF];
    unsigned long mask_unrev;
    {
        struct Column *col;
        col = get_column(game.unrevealed_column_idx);
        mask_unrev = (col->solidmask & 3) + 65536;
    }
    struct Map *mapblk;
    struct Column *col;
    unsigned long pfulmask_or;
    unsigned long pfulmask_and;
    {
        unsigned long mask_cur;
        unsigned long mask_yp;
        mask_cur = mask_unrev;
        mask_yp = mask_unrev;
        mapblk = get_map_block_at(stl_x-1, stl_y+1);
        if (map_block_revealed(mapblk, my_player_number)) {
            col = get_map_column(mapblk);
            mask_cur = col->solidmask;
            if ((mask_cur >= 8) && ((mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom)) == 0) && ((col->bitfields & 0xE) == 0)) {
                mask_cur &= 3;
            }
        }
        mapblk = get_map_block_at(stl_x-1, stl_y);
        if (map_block_revealed(mapblk, my_player_number)) {
            col = get_map_column(mapblk);
            mask_yp = col->solidmask;
            if ((mask_yp >= 8) && ((mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom)) == 0) && ((col->bitfields & 0xE) == 0)) {
                mask_yp &= 3;
            }
        }
        pfulmask_or = mask_cur | mask_yp;
        pfulmask_and = mask_cur & mask_yp;
    }
    long view_z;
    int zoom;
    long eview_w;
    long eview_h;
    long eview_z;
    int hview_y;
    int hview_z;
    zoom = camera_zoom / pixel_size;
    view_z = object_origin.z + (cells_away << 8)
        + ((bpos * camera_matrix.r[2].v[2]
         + (apos + camera_matrix.r[2].v[1]) * (camera_matrix.r[2].v[0] - view_alt)
          - camera_matrix.r[2].v[3]
          - apos * -view_alt) >> 14);
    eview_w = (view_width_over_2 + (zoom
          * (object_origin.x
           + ((bpos * camera_matrix.r[0].v[2]
            + (apos + camera_matrix.r[0].v[1]) * (camera_matrix.r[0].v[0] - view_alt)
             - camera_matrix.r[0].v[3]
             - apos * -view_alt) >> 14)) >> 16)) << 8;
    hview_y = (view_height_over_2 - (zoom
          * (object_origin.y
           + ((bpos * camera_matrix.r[1].v[2]
            + (apos + camera_matrix.r[1].v[1]) * (camera_matrix.r[1].v[0] - view_alt)
             - camera_matrix.r[1].v[3]
             - apos * -view_alt) >> 14)) >> 16)) << 8;
    hview_z = (abs(view_z) >> 1);
    if (hview_z < 32) {
        hview_z = 0;
    } else
    if (hview_z >= Z_DRAW_DISTANCE_MAX) {
        hview_z = Z_DRAW_DISTANCE_MAX;
    }
    int dview_w;
    int dview_h;
    int dview_z;
    int dhview_y;
    int dhview_z;

    dview_w = zoom * camera_matrix.r[0].v[0] >> 14;
    dhview_y = -(zoom * camera_matrix.r[1].v[0]) >> 14;
    dhview_z = camera_matrix.r[2].v[0] >> 7;
    dview_h = -(zoom * camera_matrix.r[1].v[1]) >> 14;
    dview_z = camera_matrix.r[2].v[1] >> 7;
    int wib_x;
    int wib_y;
    int wib_v;
    wib_y = (stl_y + 1) & 3;
    int idxx;
    for (idxx=mmax-mmin+1; idxx > 0; idxx--)
    {
        unsigned long mask_cur;
        unsigned long mask_yp;
        mask_cur = mask_unrev;
        mask_yp = mask_unrev;
        mapblk = get_map_block_at(stl_x, stl_y+1);
        wib_v = get_mapblk_wibble_value(mapblk);
        if (map_block_revealed(mapblk, my_player_number)) {
            col = get_map_column(mapblk);
            mask_cur = col->solidmask;
            if ((mask_cur >= 8) && ((mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom)) == 0) && ((col->bitfields & 0xE) == 0)) {
                mask_cur &= 3;
            }
        }
        mapblk = get_map_block_at(stl_x, stl_y);
        if (map_block_revealed(mapblk, my_player_number)) {
            col = get_map_column(mapblk);
            mask_yp = col->solidmask;
            if ((mask_yp >= 8) && ((mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom)) == 0) && ((col->bitfields & 0xE) == 0)) {
                mask_yp &= 3;
            }
        }
        unsigned long nfulmask_or;
        unsigned long nfulmask_and;
        nfulmask_or = mask_cur | mask_yp;
        nfulmask_and = mask_cur & mask_yp;
        unsigned long fulmask_or;
        unsigned long fulmask_and;
        fulmask_or = nfulmask_or | pfulmask_or;
        fulmask_and = nfulmask_and & pfulmask_and;
        pfulmask_or = nfulmask_or;
        pfulmask_and = nfulmask_and;
        int lightness;
        lightness = 0;
        if ((fulmask_or & 0x10000) == 0)
            lightness = get_subtile_lightness(&game.lish,stl_x,stl_y+1);

        long hmin;
        long hmax;
        hmax = height_masks[fulmask_or & 0xff];
        hmin = floor_height_table[fulmask_and & 0xff];
        struct EngineCoord *ecord;
        ecord = &ecol->cors[hmin];
        wib_x = stl_x & 3;
        struct WibbleTable *wibl;
        wibl = get_wibble_from_table(cam, 32 * wib_v + wib_x + (wib_y << 2), stl_x, stl_y);
        int32_t *randmis;
        randmis = &randomisors[(stl_x + 17 * (stl_y + 1)) & 0xff];
        eview_h = dview_h * hmin + hview_y;
        eview_z = dview_z * hmin + hview_z;
        int idxh;
        for (idxh = hmax-hmin+1; idxh > 0; idxh--)
        {
            ecord->view_width = (eview_w + wibl->view_width_offset) >> 8;
            ecord->view_height = (eview_h + wibl->view_height_offset) >> 8;
            ecord->z = eview_z;
            ecord->clip_flags = 0;
            lightness += *randmis;
            if (lightness < 0)
                lightness = 0;
            if (lightness > 16128)
                lightness = 16128;
            ecord->shade_intensity = lightness;
            if (ecord->z < 32) {
                ecord->z = 0;
            } else
            if (ecord->z >= Z_DRAW_DISTANCE_MAX) {
                ecord->z = Z_DRAW_DISTANCE_MAX;
            }
            if (ecord->view_width < 0) {
                ecord->clip_flags |= 0x08;
            } else
            if (ecord->view_width >= vec_window_width) {
                ecord->clip_flags |= 0x10;
            }
            if (ecord->view_height < 0) {
                ecord->clip_flags |= 0x20;
            } else
            if (ecord->view_height >= vec_window_height) {
                ecord->clip_flags |= 0x40;
            }

            wibl += 2;
            ecord++;
            randmis++;
            eview_h += dview_h;
            eview_z += dview_z;
        }
        stl_x++;
        ecol++;
        apos += 256;
        eview_w += dview_w;
        hview_y += dhview_y;
        hview_z += dhview_z;
    }
}

static void fill_in_points_isometric(struct Camera *cam, long bstl_x, long bstl_y, struct MinMax *mm)
{
    if ((bstl_y < 0) || (bstl_y > game.map_subtiles_y-1)) {
        return;
    }
    long mmin;
    long mmax;
    TbBool clip_min;
    TbBool clip_max;
    mmin = min(mm[0].min,mm[1].min);
    mmax = max(mm[0].max,mm[1].max);
    clip_min = false;
    clip_max = false;
    if (mmin + bstl_x < 1) {
        clip_min = true;
        mmin = 1 - bstl_x;
    }
    if (mmax + bstl_x > game.map_subtiles_x) {
        clip_max = true;
        mmax = game.map_subtiles_x - bstl_x;
    }
    if (mmax < mmin) {
        return;
    }
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    stl_y = bstl_y;
    stl_x = mmin + bstl_x;
    TbBool lim_min;
    TbBool lim_max;
    lim_min = (stl_y <= 0);
    lim_max = (stl_y >= game.map_subtiles_y-1);
    TbBool clip;
    clip = clip_min | clip_max | lim_max | lim_min;
    apos += (mmin << 8);
    struct EngineCol *ecol;
    ecol = &front_ec[mmin + MINMAX_ALMOST_HALF];
    unsigned long mask_unrev;
    {
        struct Column *col;
        col = get_column(game.unrevealed_column_idx);
        mask_unrev = col->solidmask + 65536;
    }
    struct Map *mapblk;
    struct Column *col;
    unsigned long pfulmask_or;
    unsigned long pfulmask_and;
    {
        unsigned long mask_cur;
        unsigned long mask_yp;
        mask_cur = mask_unrev;
        mask_yp = mask_unrev;
        mapblk = get_map_block_at(stl_x-1, stl_y+1);
        if (map_block_revealed(mapblk, my_player_number)) {
            col = get_map_column(mapblk);
            mask_cur = col->solidmask;
        }
        mapblk = get_map_block_at(stl_x-1, stl_y);
        if (map_block_revealed(mapblk, my_player_number)) {
            col = get_map_column(mapblk);
            mask_yp = col->solidmask;
        }
        if (clip)
        {
            if (clip_min || lim_min)
                mask_cur = 0;
            if (clip_min || lim_max)
                mask_yp = 0;
        }
        pfulmask_or = mask_cur | mask_yp;
        pfulmask_and = mask_cur & mask_yp;
    }

    long hpos;
    long view_x;
    long view_y;
    long view_z;
    int zoom;
    int hview_z;

    zoom = camera_zoom / pixel_size;
    hpos = -view_alt * apos;
    view_x = view_width_over_2 + (zoom
         * (object_origin.x
          + ((bpos * camera_matrix.r[0].v[2]
           + (apos + camera_matrix.r[0].v[1]) * (camera_matrix.r[0].v[0] - view_alt)
            - hpos - camera_matrix.r[0].v[3]) >> 14)) >> 16);
    view_y = view_height_over_2 - (zoom
         * (object_origin.y
          + ((bpos * camera_matrix.r[1].v[2]
           + (apos + camera_matrix.r[1].v[1]) * (camera_matrix.r[1].v[0] - view_alt)
            - hpos - camera_matrix.r[1].v[3]) >> 14)) >> 16);
    view_z = object_origin.z + (cells_away << 8)
        + ((bpos * camera_matrix.r[2].v[2]
         + (apos + camera_matrix.r[2].v[1]) * (camera_matrix.r[2].v[0] - view_alt)
          - hpos - camera_matrix.r[2].v[3]) >> 14);
    hview_z = (abs(view_z) >> 1);
    if (hview_z < 32) {
        hview_z = 0;
    } else
    if (hview_z >= Z_DRAW_DISTANCE_MAX) {
        hview_z = Z_DRAW_DISTANCE_MAX;
    }
    long eview_w;
    long eview_h;
    long eview_z;
    long hview_y;
    int32_t *randmis;
    int dview_w;
    int dview_h;
    int dview_z;
    int dhview_y;
    int dhview_z;

    eview_w = view_x << 8;
    hview_y = view_y << 8;
    dview_w = zoom * camera_matrix.r[0].v[0] >> 14;
    dhview_y = -(zoom * camera_matrix.r[1].v[0]) >> 14;
    dhview_z = camera_matrix.r[2].v[0] >> 7;
    dview_h = -(zoom * camera_matrix.r[1].v[1]) >> 14;
    dview_z = camera_matrix.r[2].v[1] >> 7;
    int wib_x;
    int wib_y;
    int wib_v;
    wib_y = (stl_y + 1) & 3;
    int idxx;
    for (idxx=mmax-mmin+1; idxx > 0; idxx--)
    {
        unsigned long mask_cur;
        unsigned long mask_yp;
        mask_cur = mask_unrev;
        mask_yp = mask_unrev;
        mapblk = get_map_block_at(stl_x, stl_y+1);
        wib_v = get_mapblk_wibble_value(mapblk);
        if (map_block_revealed(mapblk, my_player_number)) {
            col = get_map_column(mapblk);
            mask_cur = col->solidmask;
        }
        mapblk = get_map_block_at(stl_x, stl_y);
        if (map_block_revealed(mapblk, my_player_number)) {
            col = get_map_column(mapblk);
            mask_yp = col->solidmask;
        }
        if (clip)
        {
            if (clip_max && (idxx == 1)) {
                mask_cur = 0;
                mask_yp = 0;
            }
            if (lim_min)
                mask_cur = 0;
            if (lim_max)
                mask_yp = 0;
        }
        unsigned long nfulmask_or;
        unsigned long nfulmask_and;
        nfulmask_or = mask_cur | mask_yp;
        nfulmask_and = mask_cur & mask_yp;
        unsigned long fulmask_or;
        unsigned long fulmask_and;
        fulmask_or = nfulmask_or | pfulmask_or;
        fulmask_and = nfulmask_and & pfulmask_and;
        pfulmask_or = nfulmask_or;
        pfulmask_and = nfulmask_and;
        int lightness;
        lightness = 0;
        if ((fulmask_or & 0x10000) == 0)
            lightness = get_subtile_lightness(&game.lish,stl_x,stl_y+1);
        long hmin;
        long hmax;
        hmax = height_masks[fulmask_or & 0xff];
        hmin = floor_height_table[fulmask_and & 0xff];
        struct EngineCoord *ecord;
        ecord = &ecol->cors[hmin];
        wib_x = stl_x & 3;
        struct WibbleTable *wibl;
        wibl = get_wibble_from_table(cam, 32 * wib_v + wib_x + (wib_y << 2), stl_x, stl_y);
        eview_h = dview_h * hmin + hview_y;
        eview_z = dview_z * hmin + hview_z;
        randmis = &randomisors[(stl_x + 17 * (stl_y+1)) & 0xff] + hmin;
        int idxh;
        for (idxh = hmax-hmin+1; idxh > 0; idxh--)
        {
            ecord->view_width = (eview_w + wibl->view_width_offset) >> 8;
            ecord->view_height = (eview_h + wibl->view_height_offset) >> 8;
            ecord->z = eview_z;
            ecord->clip_flags = 0;
            lightness += 4 * (*randmis & 0xff) - 512;
            if (lightness < 0)
                lightness = 0;
            if (lightness > 15872)
                lightness = 15872;
            ecord->shade_intensity = lightness;
            if (ecord->z < 32) {
                ecord->z = 0;
            } else
            if (ecord->z >= Z_DRAW_DISTANCE_MAX) {
                ecord->z = Z_DRAW_DISTANCE_MAX;
            }
            if (ecord->view_width < 0) {
                ecord->clip_flags |= 0x08;
            } else
            if (ecord->view_width >= vec_window_width) {
                ecord->clip_flags |= 0x10;
            }
            if (ecord->view_height < 0) {
                ecord->clip_flags |= 0x20;
            } else
            if (ecord->view_height >= vec_window_height) {
                ecord->clip_flags |= 0x40;
            }
            wibl += 2;
            ecord++;
            randmis++;
            eview_h += dview_h;
            eview_z += dview_z;
        }
        stl_x++;
        ecol++;
        apos += 256;
        hview_z += dhview_z;
        eview_w += dview_w;
        hview_y += dhview_y;
    }
}

void frame_wibble_generate(void)
{
    int i;
    struct WibbleTable *wibl;
    wibl = &wibble_table[64];
    for (i = 0; i < 16; i++)
    {
        unsigned short angle;
        int osc;
        angle = water_wibble_angle + ((i & 0xFFFC) * ((i & 3) + 1) << 7);
        osc = LbSinL(angle);
        wibl->offset_y = osc >> 11;
        wibl->lightness_offset = osc >> 6;
        wibl++;
    }
    render_water_wibble += DEGREES_8_18 * game.delta_time;
    water_wibble_angle = (int)render_water_wibble & ANGLE_MASK;
    int zoom;
    {
        zoom = camera_zoom / pixel_size;
    }

    int zm00;
    int zm02;
    int zm10;
    int zm11;
    int zm12;
    zm00 = zoom * camera_matrix.r[0].v[0] >> 14;
    zm02 = zoom * camera_matrix.r[0].v[2] >> 14;
    zm10 = zoom * camera_matrix.r[1].v[0] >> 14;
    zm12 = zoom * camera_matrix.r[1].v[2] >> 14;
    zm11 = zoom * camera_matrix.r[1].v[1] >> 14;

    wibl = &wibble_table[32];
    for (i=64; i > 0; i--)
    {
        wibl->view_width_offset =   ((zm00 * wibl->offset_x) >> 8)
                         + ((zm02 * wibl->offset_z) >> 8);
        wibl->view_height_offset = -(((zm12 * wibl->offset_z) >> 8)
                         + ((zm10 * wibl->offset_x) >> 8)
                         + ((zm11 * wibl->offset_y) >> 8));
        wibl++;
    }
}

void setup_rotate_stuff(long x, long y, long z, long rotate_fade_max, long rotate_fade_min, long zoom, long map_angle, long rotate_map_roll)
{
    view_width_over_2 = vec_window_width / 2;
    view_height_over_2 = vec_window_height / 2;
    map_x_pos = x;
    map_y_pos = y;
    map_z_pos = z;
    thelens = zoom;
    spr_map_angle = map_angle;
    lfade_min = rotate_fade_min;
    lfade_max = rotate_fade_max;
    fade_mmm = rotate_fade_max - rotate_fade_min;
}

static void create_box_coords(struct EngineCoord *coord, long x, long z, long y)
{
    coord->x = x;
    coord->z = z;
    coord->clip_flags = 0;
    coord->y = y;
    rotpers(coord, &camera_matrix);
}

static void do_perspective_rotation(long x, long y, long z)
{
    struct PlayerInfo *player = get_my_player();
    struct EngineCoord epos;
    long zoom;
    long engine_w;
    long engine_h;
    zoom = camera_zoom / pixel_size;
    engine_w = player->engine_window_width/pixel_size;
    engine_h = player->engine_window_height/pixel_size;
    epos.x = -x;
    epos.y = 0;
    epos.z = y;
    rotpers_parallel_3(&epos, &camera_matrix, zoom);
    x_init_off = epos.view_width;
    y_init_off = epos.view_height;
    depth_init_off = epos.z;
    epos.x = 65536;
    epos.y = 0;
    epos.z = 0;
    rotpers_parallel_3(&epos, &camera_matrix, zoom);
    hori_offset[0] = epos.view_width - (engine_w >> 1);
    hori_offset[1] = epos.view_height - (engine_h >> 1);
    hori_offset[2] = epos.z;
    epos.x = 0;
    epos.y = 0;
    epos.z = -65536;
    rotpers_parallel_3(&epos, &camera_matrix, zoom);
    vert_offset[0] = epos.view_width - (engine_w >> 1);
    vert_offset[1] = epos.view_height - (engine_h >> 1);
    vert_offset[2] = epos.z;
    epos.x = 0;
    epos.y = 65536;
    epos.z = 0;
    rotpers_parallel_3(&epos, &camera_matrix, zoom);
    high_offset[0] = epos.view_width - (engine_w >> 1);
    high_offset[1] = epos.view_height - (engine_h >> 1);
    high_offset[2] = epos.z;
}

static void find_gamut(void)
{
    SYNCDBG(19,"Starting");
    {
        long cell_cur;
        long cell_lim;
        struct MinMax *mml;
        struct MinMax *mmr;
        cell_lim = cells_away + 1;
        mml = &minmaxs[MINMAX_ALMOST_HALF];
        mmr = &minmaxs[MINMAX_ALMOST_HALF];
        for (cell_cur = 0; cell_cur < cell_lim; cell_cur++)
        {
            long dist;
            dist = LbSqrL(cell_lim * cell_lim - cell_cur * cell_cur);
            mmr->max = dist;
            mml->max = dist;
            dist = -mmr->max;
            mmr->min = dist;
            mml->min = dist;
            mmr++;
            mml--;
        }
    }
    if (lens_mode == 0) {
        return;
    }

    int angle_sin;
    int angle_cos;
    angle_sin = LbSinL(cam_map_angle);
    angle_cos = LbCosL(cam_map_angle);
    int cells_w;
    int cells_h;
    cells_h = 6 * angle_cos >> 16;
    cells_w = -6 * angle_sin >> 16;
    int scr_w1;
    int scr_h1;
    int scr_w2;
    int scr_h2;
    long screen_dist;
    screen_dist = (lbDisplay.PhysicalScreenWidth << 7) / lens;
    scr_w1 = cells_w + ((screen_dist * angle_cos - (angle_sin << 8)) >> 16);
    scr_h1 = cells_h + (((angle_cos << 8) + screen_dist * angle_sin) >> 16);
    scr_w2 = cells_w + ((-screen_dist * angle_cos - (angle_sin << 8)) >> 16);
    scr_h2 = cells_h + (((angle_cos << 8) - screen_dist * angle_sin) >> 16);
    int mbase;
    int delta;
    struct MinMax *mm;
    int cell_curr;
    if (scr_h1 < cells_h)
    {
        delta = ((scr_w1 - cells_w) << 8) / (scr_h1 - cells_h);
        mm = &minmaxs[-cells_away + MINMAX_ALMOST_HALF];
        mbase = delta * (-cells_away - cells_h);
        for (cell_curr = -cells_away; cell_curr <= cells_away; cell_curr++)
        {
            int nlimit;
            nlimit = cells_w + (mbase >> 8);
            if (mm->max > nlimit)
                mm->max = nlimit;
            mm++;
            mbase += delta;
        }
    } else
    if (scr_h1 > cells_h)
    {
        delta = ((scr_w1 - cells_w) << 8) / (scr_h1 - cells_h);
        mm = &minmaxs[-cells_away + MINMAX_ALMOST_HALF];
        mbase = delta * (-cells_away - cells_h);
        for (cell_curr = -cells_away; cell_curr <= cells_away; cell_curr++)
        {
            int nlimit;
            nlimit = cells_w + (mbase >> 8);
            if (mm->min < nlimit)
                mm->min = nlimit;
            mm++;
            mbase += delta;
        }
    } else
    {
        if (scr_w1 <= cells_w)
        {
            mm = &minmaxs[cells_h + MINMAX_ALMOST_HALF];
            for (cell_curr = cells_h; cell_curr >= -cells_away; cell_curr--)
            {
                mm->max = 0;
                mm->min = 0;
                mm--;
            }
        } else
        {
            mm = &minmaxs[cells_h + MINMAX_ALMOST_HALF];
            for (cell_curr = cells_h; cell_curr <= cells_away; cell_curr++)
            {
                mm->max = 0;
                mm->min = 0;
                mm++;
            }
        }
    }

    if (scr_h2 < cells_h)
    {
        delta = ((scr_w2 - cells_w) << 8) / (scr_h2 - cells_h);
        mm = &minmaxs[-cells_away + MINMAX_ALMOST_HALF];
        mbase = delta * (-cells_away - cells_h);
        for (cell_curr = -cells_away; cell_curr <= cells_away; cell_curr++)
        {
            int nlimit;
            nlimit = cells_w + (mbase >> 8);
            if ( mm->min < nlimit )
              mm->min = nlimit;
            mm++;
            mbase += delta;
        }
    } else
    if (scr_h2 > cells_h)
    {
        delta = ((scr_w2 - cells_w) << 8) / (scr_h2 - cells_h);
        mm = &minmaxs[-cells_away + MINMAX_ALMOST_HALF];
        mbase = delta * (-cells_away - cells_h);
        for (cell_curr = -cells_away; cell_curr <= cells_away; cell_curr++)
        {
            int nlimit;
            nlimit = cells_w + (mbase >> 8);
            if (mm->max > nlimit)
              mm->max = nlimit;
            mm++;
            mbase += delta;
        }
    } else
    {
        if (cells_w <= scr_w2)
        {
            mm = &minmaxs[cells_h + MINMAX_ALMOST_HALF];
            for ( ; cells_h >= -cells_away; cells_h--)
            {
                mm->max = 0;
                mm->min = 0;
                mm--;
            }
        } else
        {
            mm = &minmaxs[cells_h + MINMAX_ALMOST_HALF];
            for ( ; cells_away >= cells_h; cells_h++)
            {
                mm->max = 0;
                mm->min = 0;
                mm++;
            }
        }
    }
}

static void fiddle_half_gamut(long start_stl_x, long start_stl_y, long step, long a4)
{
    long end_stl_x;
    long stl_xc;
    long stl_xp;
    long stl_xn;

    end_stl_x = start_stl_x + minmaxs[(MINMAX_LENGTH/2)].min;
    for (stl_xc=start_stl_x; 1; stl_xc--)
    {
        if (stl_xc < end_stl_x) {
            stl_xc = -4000;
            break;
        }
        struct Map *mapblk;
        mapblk = get_map_block_at(stl_xc, start_stl_y);
        if  ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
            break;
        }
    }
    for (stl_xp=start_stl_x; 1; stl_xp--)
    {
        if (stl_xp < end_stl_x) {
            stl_xp = -4000;
            break;
        }
        struct Map *mapblk;
        mapblk = get_map_block_at(stl_xp, start_stl_y-1);
        if  ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
            break;
        }
    }
    for (stl_xn=start_stl_x; 1; stl_xn--)
    {
        if (stl_xn < end_stl_x) {
            stl_xn = -4000;
            break;
        }
        struct Map *mapblk;
        mapblk = get_map_block_at(stl_xn, start_stl_y-1);
        if  ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
            break;
        }
    }
    long stl_x_min;
    stl_x_min = 0;
    TbBool set_x_min;
    set_x_min = false;
    if ((stl_xc != -4000) && (stl_xp != -4000) && (stl_xn != -4000))
    {
        stl_x_min = min(min(stl_xn, stl_xp), stl_xc);
        set_x_min = true;
        minmaxs[(MINMAX_LENGTH/2)].min = stl_x_min - start_stl_x;
    }

    end_stl_x = start_stl_x + minmaxs[(MINMAX_LENGTH/2)].max;
    for (stl_xc=start_stl_x; 1; stl_xc++)
    {
        if (stl_xc > end_stl_x) {
            stl_xc = -4000;
            break;
        }
        struct Map *mapblk;
        mapblk = get_map_block_at(stl_xc, start_stl_y);
        if  ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
            break;
        }
    }
    for (stl_xp=start_stl_x; 1; stl_xp++)
    {
        if (stl_xp > end_stl_x) {
            stl_xp = -4000;
            break;
        }
        struct Map *mapblk;
        mapblk = get_map_block_at(stl_xp, start_stl_y-1);
        if  ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
            break;
        }
    }
    for (stl_xn=start_stl_x; 1; stl_xn++)
    {
        if (stl_xn > end_stl_x) {
            stl_xn = -4000;
            break;
        }
        struct Map *mapblk;
        mapblk = get_map_block_at(stl_xn, start_stl_y-1);
        if  ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
            break;
        }
    }
    long stl_x_max;
    stl_x_max = 0;
    TbBool set_x_max;
    set_x_max = false;
    if ((stl_xc != -4000) && (stl_xp != -4000) && (stl_xn != -4000))
    {
        stl_x_max = max(max(stl_xn, stl_xp), stl_xc);
        set_x_max = true;
        minmaxs[(MINMAX_LENGTH/2)].max = stl_x_max - start_stl_x + 1;
    }

    struct MinMax *mm;
    long stl_y;
    stl_y = start_stl_y + step;
    mm = &minmaxs[step + (MINMAX_LENGTH/2)];
    long n;
    for (n=1; n < a4; n++)
    {
        if (mm->max <= mm->min)
        {
            long i;
            for (i=a4-n; i > 0; i--)
            {
                mm->min = 0;
                mm->max = 0;
                mm += step;
            }
            break;
        }
        long stl_x_min_limit;
        stl_x_min_limit = start_stl_x + mm->min;
        if (!set_x_min || (stl_x_min < stl_x_min_limit)) {
            stl_x_min = stl_x_min_limit;
        }
        long stl_x_max_limit;
        stl_x_max_limit = start_stl_x + mm->max;
        if (!set_x_max || (stl_x_max > stl_x_max_limit)) {
            stl_x_max = stl_x_max_limit;
        }

        /* The variable needs to be volatile to disallow changing it to float during optimisations.
         * Changing it to float would lead to conditions like "if (delta_y != 1)" not working.
         */
        volatile long delta_y;
        delta_y = abs(stl_y - start_stl_y);
        long rect_factor;

        TbBool set_x_min_rect;
        if (delta_y != 1) {
            rect_factor = (stl_x_min - start_stl_x) / (delta_y - 1);
        } else {
            rect_factor = 1;
        }
        if (rect_factor - 1 <= 0) {
            set_x_min_rect = false;
        } else {
            set_x_min_rect = true;
            stl_x_min = rect_factor + stl_x_min - 1;
        }

        long stl_x;
        long stl_x_lc_min;

        for (stl_x=stl_x_min-1; stl_x < stl_x_max_limit; stl_x++)
        {
            struct Map *mapblk;
            mapblk = get_map_block_at(stl_x+1, stl_y);
            if ((mapblk->flags & SlbAtFlg_Blocking) == 0)
              break;
        }
        stl_x_lc_min = stl_x;

        if ( set_x_min_rect
          || stl_x_lc_min > stl_x_min
          || (get_map_block_at(stl_x_lc_min, stl_y)->flags & SlbAtFlg_Blocking) )
        {
            long relative_x_offset;
            relative_x_offset = stl_x_min - start_stl_x;
            stl_x_min = stl_x_lc_min;
            set_x_min = true;
            mm->min = relative_x_offset - 1;
        }
        else
        {
          if (delta_y != 1) {
              rect_factor = (stl_x_lc_min - start_stl_x) / (delta_y - 1);
          } else {
              rect_factor = 1;
          }
          long stl_x_min_sublim;
          if ((delta_y == 1) || (stl_x_min + rect_factor - 1 < stl_x_min_limit))
          {
              set_x_min = false;
              stl_x_min_sublim = stl_x_min_limit;
          } else
          {
              stl_x_min += rect_factor - 1;
              set_x_min = true;
              stl_x_min_sublim = stl_x_min;
              mm->min = stl_x_min - start_stl_x - 1;
          }
          for (stl_x=stl_x_min; stl_x >= stl_x_min_sublim; stl_x--)
          {
              struct Map *mapblk;
              mapblk = get_map_block_at(stl_x, stl_y);
              if ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
                  stl_x_min = stl_x;
                  set_x_min = true;
                  mm->min = stl_x - start_stl_x - 1;
                  break;
              }
          }
        }

        TbBool set_x_max_rect;
        if (delta_y != 1) {
            rect_factor = (stl_x_max - start_stl_x) / (delta_y - 1);
        } else {
            rect_factor = 1;
        }
        if (rect_factor + 1 >= 0) {
            set_x_max_rect = false;
        } else {
            set_x_max_rect = true;
            stl_x_max += rect_factor + 1;
        }

        for (stl_x=stl_x_max+1; stl_x > stl_x_min_limit; stl_x--)
        {
            struct Map *mapblk;
            mapblk = get_map_block_at(stl_x-1, stl_y);
            if ((mapblk->flags & SlbAtFlg_Blocking) == 0)
              break;
        }
        stl_x_lc_min = stl_x;

        if ( set_x_max_rect
          || stl_x_lc_min < stl_x_max
          || (get_map_block_at(stl_x_lc_min, stl_y)->flags & SlbAtFlg_Blocking) )
        {
            long stl_tmp;
            stl_tmp = stl_x_max - start_stl_x;
            stl_x_max = stl_x_lc_min;
            mm->max = stl_tmp + 2;
            set_x_max = true;
        }
        else
        {
          set_x_max = 0;
          if (delta_y != 1) {
              rect_factor = (stl_x_lc_min - start_stl_x) / (delta_y - 1);
          } else {
              rect_factor = 1;
          }

          long stl_x_max_sublim;
          if ((delta_y == 1) || (stl_x_max + rect_factor + 1 > start_stl_x + mm->max))
          {
              stl_x_max_sublim = start_stl_x + mm->max;
          } else
          {
              stl_x_max += rect_factor + 1;
              set_x_max = true;
              stl_x_max_sublim = stl_x_max;
              mm->max = stl_x_max - start_stl_x + 2;
          }
          for (stl_x=stl_x_max; stl_x <= stl_x_max_sublim; stl_x++)
          {
              struct Map *mapblk;
              mapblk = get_map_block_at(stl_x, stl_y);
              if ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
                  set_x_max = true;
                  stl_x_max = stl_x;
                  mm->max = stl_x - start_stl_x + 2;
                  break;
              }
          }
        }

        if (mm->min < -cells_away)
            mm->min = -cells_away;
        if (mm->max > cells_away)
            mm->max = cells_away;
        if (mm->min >= mm->max)
        {
            long i;
            for (i=a4-n; i > 0; i--)
            {
                mm->min = 0;
                mm->max = 0;
                mm += step;
            }
            break;
        }
        mm += step;
        stl_y += step;
    }
}

static void fiddle_gamut_find_limits(int32_t *floor_x, int32_t *floor_y, long ewwidth, long ewheight, long ewzoom)
{
    long edge_length_01;
    long edge_length_02;
    long edge_length_13;
    long edge_length_23;
    long tmp_y;
    long tmp_x;
    long i;
    get_floor_pointed_at(ewwidth + ewzoom, -ewzoom, &floor_y[2], &floor_x[2]);
    get_floor_pointed_at(ewwidth + ewzoom, ewheight + ewzoom, &floor_y[1], &floor_x[1]);
    get_floor_pointed_at(-ewzoom, ewheight + ewzoom, &floor_y[0], &floor_x[0]);
    get_floor_pointed_at(-ewzoom, -ewzoom, &floor_y[3], &floor_x[3]);
    // Get the value with lowest X coord into [0]
    for (i=1; i < 4; i++)
    {
        tmp_y = floor_y[i];
        if (floor_y[0] > tmp_y)
        {
          tmp_x = floor_x[i];
          floor_x[i] = floor_x[0];
          floor_x[0] = tmp_x;
          floor_y[i] = floor_y[0];
          floor_y[0] = tmp_y;
        }
    }
    // Get the value with highest X coord into [3]
    for (i=0; i < 3; i++)
    {
        tmp_y = floor_y[i];
        if (floor_y[3] < tmp_y)
        {
          tmp_x = floor_x[i];
          floor_x[i] = floor_x[3];
          floor_x[3] = tmp_x;
          floor_y[i] = floor_y[3];
          floor_y[3] = tmp_y;
        }
    }
    // Between values with medicore X, place the lowest Y first
    if (floor_x[1] > floor_x[2])
    {
        tmp_x = floor_x[1];
        tmp_y = floor_y[1];
        floor_x[1] = floor_x[2];
        floor_x[2] = tmp_x;
        floor_y[1] = floor_y[2];
        floor_y[2] = tmp_y;
    }

    // Lengths of X vectors
    edge_length_01 = abs(floor_y[1] - floor_y[0]);
    edge_length_13 = abs(floor_y[3] - floor_y[1]);
    edge_length_02 = abs(floor_y[2] - floor_y[0]);
    edge_length_23 = abs(floor_y[3] - floor_y[2]);
    // Update points according to both coordinates
    if ( (floor_x[1] > floor_x[0]) && (edge_length_01 < edge_length_13) )
    {
        tmp_x = floor_x[1];
        floor_y[1] = floor_y[0];
        floor_x[1] = floor_x[0];
        floor_x[0] = tmp_x;
    }
    if ( (floor_x[1] > floor_x[3]) && (edge_length_13 < edge_length_01) )
    {
        tmp_x = floor_x[1];
        floor_y[1] = floor_y[3];
        floor_x[1] = floor_x[3];
        floor_x[3] = tmp_x;
    }
    if ( (floor_x[2] < floor_x[0]) && (edge_length_02 < edge_length_23) )
    {
        tmp_x = floor_x[2];
        floor_y[2] = floor_y[0];
        floor_x[2] = floor_x[0];
        floor_x[0] = tmp_x;
    }
    if ( (floor_x[2] < floor_x[3]) && (edge_length_23 < edge_length_02) )
    {
        tmp_x = floor_x[2];
        floor_x[2] = floor_x[3];
        floor_x[3] = tmp_x;
        floor_y[2] = floor_y[3];
    }
}

static void fiddle_gamut_set_base(int32_t *floor_x, int32_t *floor_y, long pos_x, long pos_y)
{
    floor_x[0] -= pos_x;
    floor_x[1] -= pos_x;
    floor_y[0] += (MINMAX_LENGTH/2) - pos_y;
    floor_x[2] -= pos_x;
    floor_y[1] += (MINMAX_LENGTH/2) - pos_y;
    floor_y[2] += (MINMAX_LENGTH/2) - pos_y;
    floor_x[3] -= pos_x;
    floor_y[3] += (MINMAX_LENGTH/2) - pos_y;
}

static void fiddle_gamut_set_minmaxes(int32_t *floor_x, int32_t *floor_y, long max_tiles)
{
    struct MinMax *mm;
    long mlimit;
    long bormul;
    long bormuh;
    long borinc;
    short bordec;
    long midx;
    midx = 0;
    mlimit = floor_y[0];
    if (mlimit > MINMAX_LENGTH-1)
      mlimit = MINMAX_LENGTH-1;
    for (; midx < mlimit; midx++)
    {
        mm = &minmaxs[midx];
        mm->min = 0;
        mm->max = 0;
    }
    if (floor_y[1] <= floor_y[0])
        borinc = floor_x[0];
    else
        borinc = ((floor_x[1] - floor_x[0]) << 16) / (floor_y[1] - floor_y[0]);

    bormul = (floor_x[0] << 16);
    if (floor_y[0] < 0)
        bormul -= floor_y[0] * borinc;

    mlimit = floor_y[1];
    if (mlimit > MINMAX_LENGTH-1)
      mlimit = MINMAX_LENGTH-1;
    for (; midx < mlimit; midx++)
    {
        mm = &minmaxs[midx];
        bordec = (bormul >> 16);
        if (bordec < -max_tiles)
            mm->min = -max_tiles;
        else
            mm->min = bordec;
        bormul += borinc;
    }

    bormul = floor_x[1] << 16;
    if (floor_y[1] < floor_y[3])
      borinc = ((floor_x[3] - floor_x[1]) << 16) / (floor_y[3] - floor_y[1]);

    mlimit = floor_y[3];
    if (mlimit > MINMAX_LENGTH-1)
      mlimit = MINMAX_LENGTH-1;
    if (midx < 0) {
        bormul -= midx * borinc;
        midx = 0;
    }

    for (; midx < mlimit; midx++)
    {
        mm = &minmaxs[midx];
        bordec = (bormul >> 16);
        if (bordec < -max_tiles)
            mm->min = -max_tiles;
        else
            mm->min = bordec;
        bormul += borinc;
    }
    midx = floor_y[0];
    if (floor_y[2] > floor_y[0])
        borinc = ((floor_x[2] - floor_x[0]) << 16) / (floor_y[2] - floor_y[0]);
    mlimit = floor_y[2];
    if (mlimit > MINMAX_LENGTH-1)
        mlimit = MINMAX_LENGTH-1;
    bormuh = (floor_x[0] << 16);
    if (midx < 0) {
        bormuh -= floor_y[0] * borinc;
        midx = 0;
    }

    for (; midx < mlimit; midx++)
    {
        mm = &minmaxs[midx];
        bordec = (bormuh >> 16) + 1;
        if (bordec > max_tiles)
            mm->max = max_tiles;
        else
            mm->max = bordec;
        bormuh += borinc;
    }

    bormul = floor_x[2] << 16;
    if (floor_y[2] < floor_y[3])
      borinc = ((floor_x[3] - floor_x[2]) << 16) / (floor_y[3] - floor_y[2]);
    mlimit = floor_y[3];
    if (mlimit > MINMAX_LENGTH-1)
      mlimit = MINMAX_LENGTH-1;
    if ( midx < 0 ) {
        bormul -= midx * borinc;
        midx = 0;
    }

    for (; midx < mlimit; midx++)
    {
        mm = &minmaxs[midx];
        bordec = (bormul >> 16) + 1;
        if (bordec > max_tiles)
            mm->max = max_tiles;
        else
            mm->max = bordec;
        bormul += borinc;
    }
    for (; midx <= MINMAX_LENGTH-1; midx++)
    {
        mm = &minmaxs[midx];
        mm->min = 0;
        mm->max = 0;
    }
}

/** Prepares limits for tiles to be rendered.
 *
 * @param pos_x
 * @param pos_y
 */
static void fiddle_gamut(long pos_x, long pos_y)
{
    struct PlayerInfo *player = get_my_player();
    long ewwidth;
    long ewheight;
    long ewzoom;
    int32_t floor_x[4];
    int32_t floor_y[4];
    switch (player->view_mode)
    {
    case PVM_CreatureView:
        fiddle_half_gamut(pos_x, pos_y, 1, cells_away);
        fiddle_half_gamut(pos_x, pos_y, -1, cells_away + 2);
        break;
    case PVM_IsoWibbleView:
    case PVM_IsoStraightView:
        // Retrieve coordinates on limiting map points
        ewwidth = player->engine_window_width / pixel_size;
        ewheight = player->engine_window_height / pixel_size - ((8 * high_offset[1]) >> 8);
        ewzoom = (768 * (camera_zoom/pixel_size)) >> 17;
        fiddle_gamut_find_limits(floor_x, floor_y, ewwidth, ewheight, ewzoom);
        // Place the area at proper base coords
        fiddle_gamut_set_base(floor_x, floor_y, pos_x, pos_y);
        fiddle_gamut_set_minmaxes(floor_x, floor_y, MAX_I_CAN_SEE_OVERHEAD);
        break;
    }
}

int floor_height_for_volume_box(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    struct SlabConfigStats* slabst = get_slab_stats(slb);
    if (!subtile_revealed(slab_subtile_center(slb_x), slab_subtile_center(slb_y), plyr_idx) || ((slabst->block_flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0))
    {
        return (temp_cluedo_mode == 0) ? 5 : 2; // return a height of 5 for a wall, or if cluedo mode (low walls mode) is enabled, return a wall height of 2.
    }
    if (slab_kind_is_liquid(slb->kind))
    {
        return 0; // Water/Lava is at height 0
    }
    return 1; // Floor is at height 1
}

static void create_line_element(long a1, long a2, long a3, long a4, long bckt_idx, TbPixel color)
{
    struct BucketKindSlabSelector *poly;
    if (!is_free_space_in_poly_pool(1))
    {
        return;
    }
    if (bckt_idx >= BUCKETS_COUNT)
        bckt_idx = BUCKETS_COUNT-1;
    else
    if (bckt_idx < 0)
        bckt_idx = 0;
    poly = (struct BucketKindSlabSelector *)getpoly;
    getpoly += sizeof(struct BucketKindSlabSelector);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_SlabSelector;
    buckets[bckt_idx] = (struct BasicQ *)poly;
    if (pixel_size > 0)
    {
        poly->p.X = a1 / pixel_size;
        poly->p.Y = a2 / pixel_size;
        poly->p.U = a3 / pixel_size;
        poly->p.V = a4 / pixel_size;
    }
    poly->p.S = color;
}

static void create_line_segment(struct EngineCoord *start, struct EngineCoord *end, TbPixel color)
{
    struct BucketKindSlabSelector *poly;
    long bckt_idx;
    if (!is_free_space_in_poly_pool(1))
        return;

    // Reducing line_z will make the lines look cleaner, but the "fancy_map_volume_box" vertical lines become more visible.
    float line_z = 0.994;
    bckt_idx = (( (start->z*line_z) + (end->z*line_z) ) / 32) - 2;
    // Original calculation:  bckt_idx = (start->z+end->z)/2 / 16 - 2;

    if (bckt_idx >= BUCKETS_COUNT)
        bckt_idx = BUCKETS_COUNT-1;
    else
    if (bckt_idx < 0)
        bckt_idx = 0;
    // Add to bucket
    poly = (struct BucketKindSlabSelector *)getpoly;
    getpoly += sizeof(struct BucketKindSlabSelector);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_SlabSelector;
    buckets[bckt_idx] = (struct BasicQ *)poly;
    // Fill parameters
    if (pixel_size > 0)
    {
        poly->p.X = start->view_width;
        poly->p.Y = start->view_height;
        poly->p.U = end->view_width;
        poly->p.V = end->view_height;
    }
    poly->p.S = color;
}

/**
* Adds a line with constant Z coord to the drawlist of perspective view.
* @param color Color index.
* @param pos_z The Z coord, constant for whole line.
* @param beg_x The X coord of start of the line.
* @param end_x The X coord of end of the line.
* @param beg_y The Y coord of start of the line.
* @param end_y The Y coord of end of the line.
*/
static void create_line_const_z(unsigned char color, long pos_z, long beg_x, long end_x, long beg_y, long end_y)
{
    struct EngineCoord end;
    struct EngineCoord start;
    long vec_x;
    long vec_y;
    long pos_x;
    long pos_y;
    vec_x = end_x - beg_x;
    vec_y = end_y - beg_y;
    create_box_coords(&start, beg_x, beg_y, pos_z);

    if (abs(vec_y) > abs(vec_x))
    {
        if (vec_y < 0)
        {
            long vec_tmp;
            vec_tmp = beg_x;
            beg_x = end_x;
            end_x = vec_tmp;
            vec_tmp = beg_y;
            beg_y = end_y;
            end_y = vec_tmp;
            vec_x = -vec_x;
            vec_y = -vec_y;
        }
        for (pos_y = beg_y + COORD_PER_STL; pos_y <= end_y; pos_y += COORD_PER_STL)
        {
            pos_x = beg_x + vec_x * abs(pos_y - beg_y) / abs(vec_y);
            create_box_coords(&end, pos_x, pos_y, pos_z);
            create_line_segment(&start, &end, color);
            memcpy(&start, &end, sizeof(struct EngineCoord));
        }
    }
    else
    {
        if (vec_x < 0)
        {
            long vec_tmp;
            vec_tmp = beg_x;
            beg_x = end_x;
            end_x = vec_tmp;
            vec_tmp = beg_y;
            beg_y = end_y;
            end_y = vec_tmp;
            vec_x = -vec_x;
            vec_y = -vec_y;
        }
        for (pos_x = beg_x + COORD_PER_STL; pos_x <= end_x; pos_x += COORD_PER_STL)
        {
            pos_y = beg_y + vec_y * abs(pos_x - beg_x) / abs(vec_x);
            create_box_coords(&end, pos_x, pos_y, pos_z);
            create_line_segment(&start, &end, color);
            memcpy(&start, &end, sizeof(struct EngineCoord));
        }
    }
}

/**
* Adds a line with constant XZ coords to the drawlist of perspective view.
* @param pos_x The X coord, constant for whole line.
* @param pos_z The Z coord, constant for whole line.
* @param start_y The Y coord of start of the line.
* @param end_y The Y coord of end of the line.
*/
static void create_line_const_xz(long pos_x, long pos_z, long start_y, long end_y)
{
    struct EngineCoord end;
    struct EngineCoord start;
    long pos_y;
    create_box_coords(&start, pos_x, start_y, pos_z);
    for (pos_y = start_y+256; pos_y <= end_y; pos_y+=256)
    {
        create_box_coords(&end, pos_x, pos_y, pos_z);
        create_line_segment(&start, &end, map_volume_box.color);
        memcpy(&start, &end, sizeof(struct EngineCoord));
    }
}

/**
* Adds a line with constant XY coords to the drawlist of perspective view.
* @param pos_x The X coord, constant for whole line.
* @param pos_y The Y coord, constant for whole line.
* @param start_z The Z coord of start of the line.
* @param end_z The Z coord of end of the line.
*/
static void create_line_const_xy(long pos_x, long pos_y, long start_z, long end_z)
{
    struct EngineCoord end;
    struct EngineCoord start;
    long pos_z;
    create_box_coords(&start, pos_x, pos_y, start_z);
    for (pos_z = start_z+256; pos_z <= end_z; pos_z+=256)
    {
        create_box_coords(&end, pos_x, pos_y, pos_z);
        create_line_segment(&start, &end, map_volume_box.color);
        memcpy(&start, &end, sizeof(struct EngineCoord));
    }
}

/**
* Adds a line with constant YZ coords to the drawlist of perspective view.
* @param pos_y The Y coord, constant for whole line.
* @param pos_z The Z coord, constant for whole line.
* @param start_x The X coord of start of the line.
* @param end_x The X coord of end of the line.
*/
static void create_line_const_yz(long pos_y, long pos_z, long start_x, long end_x)
{
    struct EngineCoord end;
    struct EngineCoord start;
    long pos_x;
    create_box_coords(&start, start_x, pos_y, pos_z);
    for (pos_x = start_x+256; pos_x <= end_x; pos_x+=256)
    {
        create_box_coords(&end, pos_x, pos_y, pos_z);
        create_line_segment(&start, &end, map_volume_box.color);
        memcpy(&start, &end, sizeof(struct EngineCoord));
    }
}

void create_map_volume_box(long x, long y, long z, long line_color)
{
    long box_xs;
    long box_xe;
    long box_ys;
    long box_ye;
    long box_zs;
    long box_ze;
    long i;
    long box_color = map_volume_box.color;
    map_volume_box.color = line_color;

    box_xs = map_volume_box.beg_x - x;
    box_ys = y - map_volume_box.beg_y;
    box_ye = y - map_volume_box.end_y;
    box_xe = map_volume_box.end_x - x;

    if ( temp_cluedo_mode )
    {
        box_ze = 2*COORD_PER_STL - z;
    }
    else
    {
        box_ze = 5*COORD_PER_STL - z;
    }

    box_zs = (map_volume_box.floor_height_z << 8) - z;
    if ( box_zs >= box_ze )
    {
      box_zs = box_ze;
    }

    if ( box_xe < box_xs )
    {
        i = map_volume_box.beg_x;
        box_xs = map_volume_box.end_x - x;
        box_xe = map_volume_box.beg_x - x;
        map_volume_box.beg_x = map_volume_box.end_x;
        map_volume_box.end_x = i;
    }

    if ( box_ye < box_ys )
    {
        i = map_volume_box.beg_y;
        box_ys = y - map_volume_box.end_y;
        box_ye = y - map_volume_box.beg_y;
        map_volume_box.beg_y = map_volume_box.end_y;
        map_volume_box.end_y = i;
    }

    // Draw top rectangle
    create_line_const_yz(box_ye, box_zs, box_xs, box_xe);
    create_line_const_yz(box_ys, box_zs, box_xs, box_xe);
    create_line_const_xz(box_xs, box_zs, box_ys, box_ye);
    create_line_const_xz(box_xe, box_zs, box_ys, box_ye);
    // Vertical lines which connect the rectangles
    create_line_const_xy(box_xs, box_ys, box_zs, box_ze);
    create_line_const_xy(box_xe, box_ys, box_zs, box_ze);
    create_line_const_xy(box_xe, box_ye, box_zs, box_ze);
    create_line_const_xy(box_xs, box_ye, box_zs, box_ze);
    // Bottom rectangle
    create_line_const_yz(box_ye, box_ze, box_xs, box_xe);
    create_line_const_yz(box_ys, box_ze, box_xs, box_xe);
    create_line_const_xz(box_xs, box_ze, box_ys, box_ye);
    create_line_const_xz(box_xe, box_ze, box_ys, box_ye);

    map_volume_box.color = box_color;
}

void create_fancy_map_volume_box(struct RoomSpace roomspace, long x, long y, long z, long color, TbBool show_outer_box)
{
    long line_color = color;
    if (show_outer_box)
    {
        line_color = map_volume_box.color; //  set the "inner" box color to the default colour (usually red/green)
    }
    long box_xs;
    long box_xe;
    long box_ys;
    long box_ye;
    long box_zs;
    long box_ze;
    long i;
    long box_color = map_volume_box.color;
    map_volume_box.color = line_color;
    struct MapVolumeBox valid_slabs = map_volume_box;
    // get the 'accurate' roomspace shape instead of the outer box
    valid_slabs.beg_x = subtile_coord((roomspace.left * 3), 0);
    valid_slabs.beg_y = subtile_coord((roomspace.top * 3), 0);
    valid_slabs.end_x = subtile_coord((3*1) + (roomspace.right * 3), 0);
    valid_slabs.end_y = subtile_coord(((3*1) + roomspace.bottom * 3), 0);

    box_xs = valid_slabs.beg_x - x;
    box_ys = y - valid_slabs.beg_y;
    box_ye = y - valid_slabs.end_y;
    box_xe = valid_slabs.end_x - x;

    if ( temp_cluedo_mode )
    {
        box_ze = 2*COORD_PER_STL - z;
    }
    else
    {
        box_ze = 5*COORD_PER_STL - z;
    }

    box_zs = 256 - z;
    if ( box_zs >= box_ze )
    {
      box_zs = box_ze;
    }

    if ( box_xe < box_xs )
    {
        i = valid_slabs.beg_x;
        box_xs = valid_slabs.end_x - x;
        box_xe = valid_slabs.beg_x - x;
        valid_slabs.beg_x = valid_slabs.end_x;
        valid_slabs.end_x = i;
    }

    if ( box_ye > box_ys )
    {
        i = valid_slabs.beg_y;
        box_ys = y - valid_slabs.end_y;
        box_ye = y - valid_slabs.beg_y;
        valid_slabs.beg_y = valid_slabs.end_y;
        valid_slabs.end_y = i;
    }
    for (int roomspace_y = 0; roomspace_y < roomspace.height; roomspace_y++)
    {
        for (int roomspace_x = 0; roomspace_x < roomspace.width; roomspace_x++)
        {
            TbBool is_in_roomspace = roomspace.slab_grid[roomspace_x][roomspace_y];
            int slab_xstart = box_xs + (roomspace_x * 3 * COORD_PER_STL);
            int slab_ystart = box_ys - (roomspace_y * 3 * COORD_PER_STL);
            int slab_xend = box_xs + ((roomspace_x + 1) * 3 * COORD_PER_STL);
            int slab_yend = box_ys - ((roomspace_y + 1) * 3 * COORD_PER_STL);
            if (is_in_roomspace)
            {
                TbBool air_left = (roomspace_x == 0) ? true : (roomspace.slab_grid[roomspace_x-1][roomspace_y] == false);
                TbBool air_right = (roomspace_x == roomspace.width) ? true : (roomspace.slab_grid[roomspace_x+1][roomspace_y] == false);
                TbBool air_above = (roomspace_y == 0) ? true : (roomspace.slab_grid[roomspace_x][roomspace_y-1] == false);
                TbBool air_below = (roomspace_y == roomspace.height) ? true : (roomspace.slab_grid[roomspace_x][roomspace_y+1] == false);
                if (air_left)
                {
                    // Draw top rectangle
                    create_line_const_xz(slab_xstart, box_zs, slab_yend, slab_ystart);
                    // Bottom rectangle
                    create_line_const_xz(slab_xstart, box_ze, slab_yend, slab_ystart);
                    // Vertical lines which connect the rectangles
                    if (air_above)
                    {
                        create_line_const_xy(slab_xstart, slab_ystart, box_zs, box_ze);
                    }
                    if (air_below)
                    {
                        create_line_const_xy(slab_xstart, slab_yend, box_zs, box_ze);
                    }
                }
                if (air_right)
                {
                    // Draw top rectangle
                    create_line_const_xz(slab_xend, box_zs, slab_yend, slab_ystart);
                    // Bottom rectangle
                    create_line_const_xz(slab_xend, box_ze, slab_yend, slab_ystart);
                    // Vertical lines which connect the rectangles
                    if (air_above)
                    {
                        create_line_const_xy(slab_xend, slab_ystart, box_zs, box_ze);
                    }
                    if (air_below)
                    {
                        create_line_const_xy(slab_xend, slab_yend, box_zs, box_ze);
                    }
                }
                if (air_above)
                {
                    // Draw top rectangle
                    create_line_const_yz(slab_ystart, box_zs, slab_xstart, slab_xend);
                    // Bottom rectangle
                    create_line_const_yz(slab_ystart, box_ze, slab_xstart, slab_xend);
                }
                if (air_below)
                {
                    // Draw top rectangle
                    create_line_const_yz(slab_yend, box_zs, slab_xstart, slab_xend);
                    // Bottom rectangle
                    create_line_const_yz(slab_yend, box_ze, slab_xstart, slab_xend);
                }
            }
            else if (!is_in_roomspace) //this handles "inside corners"
            {
                TbBool room_left = (roomspace_x == 0) ? false : roomspace.slab_grid[roomspace_x-1][roomspace_y];
                TbBool room_right = (roomspace_x == roomspace.width) ? false : roomspace.slab_grid[roomspace_x+1][roomspace_y];
                TbBool room_above = (roomspace_y == 0) ? false : roomspace.slab_grid[roomspace_x][roomspace_y-1];
                TbBool room_below = (roomspace_y == roomspace.height) ? false : roomspace.slab_grid[roomspace_x][roomspace_y+1];
                if (room_left)
                {
                    // Vertical lines which connect the rectangles
                    if (room_above)
                    {
                        create_line_const_xy(slab_xstart, slab_ystart, box_zs, box_ze);
                    }
                    if (room_below)
                    {
                        create_line_const_xy(slab_xstart, slab_yend, box_zs, box_ze);
                    }
                }
                if (room_right)
                {
                    // Vertical lines which connect the rectangles
                    if (room_above)
                    {
                        create_line_const_xy(slab_xend, slab_ystart, box_zs, box_ze);
                    }
                    if (room_below)
                    {
                        create_line_const_xy(slab_xend, slab_yend, box_zs, box_ze);
                    }
                }
                if (show_outer_box) // this handles the "outer line" (only when it is not in the roomspace)
                {
                    //draw 2nd line, i.e. the outer line - the one around the edge of the 5x5 cursor, not the valid slabs within the cursor
                    map_volume_box.color = color; // switch to the "secondary colour" (the one passed as a variable if show_outer_box is true)
                    TbBool left_edge   = (roomspace_x == 0)                    ? true : false;
                    TbBool right_edge  = (roomspace_x == roomspace.width - 1)  ? true : false;
                    TbBool top_edge    = (roomspace_y == 0)                    ? true : false;
                    TbBool bottom_edge = (roomspace_y == roomspace.height - 1) ? true : false;
                    if (left_edge)
                    {
                        create_line_const_xz(slab_xstart, box_zs, slab_yend, slab_ystart);
                        create_line_const_xz(slab_xstart, box_ze, slab_yend, slab_ystart);
                        if (top_edge)
                        {
                            create_line_const_xy(slab_xstart, slab_ystart, box_zs, box_ze);
                        }
                        if (bottom_edge)
                        {
                            create_line_const_xy(slab_xstart, slab_yend, box_zs, box_ze);
                        }
                    }
                    if (right_edge)
                    {
                        create_line_const_xz(slab_xend, box_zs, slab_yend, slab_ystart);
                        create_line_const_xz(slab_xend, box_ze, slab_yend, slab_ystart);
                        if (top_edge)
                        {
                            create_line_const_xy(slab_xend, slab_ystart, box_zs, box_ze);
                        }
                        if (bottom_edge)
                        {
                            create_line_const_xy(slab_xend, slab_yend, box_zs, box_ze);
                        }
                    }
                    if (top_edge)
                    {
                        create_line_const_yz(slab_ystart, box_zs, slab_xstart, slab_xend);
                        create_line_const_yz(slab_ystart, box_ze, slab_xstart, slab_xend);
                    }
                    if (bottom_edge)
                    {
                        create_line_const_yz(slab_yend, box_zs, slab_xstart, slab_xend);
                        create_line_const_yz(slab_yend, box_ze, slab_xstart, slab_xend);
                    }
                    map_volume_box.color = line_color; // switch back to default color (red/green) for the inner line
                }
            }
        }
    }

    map_volume_box.color = box_color;
}

static void process_isometric_map_volume_box(long x, long y, long z, PlayerNumber plyr_idx)
{
    unsigned char default_color = map_volume_box.color;
    unsigned char line_color = default_color;
    struct PlayerInfo* current_player = get_player(plyr_idx);
    // Check if a roomspace is currently being built
    // and if so feed this back to the user
    if ((current_player->roomspace.is_active) && ((current_player->work_state == PSt_Sell) || (current_player->work_state == PSt_BuildRoom)))
    {
        line_color = SLC_REDYELLOW; // change the cursor color to indicate to the user that nothing else can be built or sold at the moment
    }
    if (current_player->render_roomspace.render_roomspace_as_box)
    {
        if (current_player->render_roomspace.is_roomspace_a_box)
        {
            // This is a basic square box
            create_map_volume_box(x + box_lag_compensation_x, y + box_lag_compensation_y, z, line_color);
        }
        else
        {
            // This is a "2-line" square box
            // i.e. an "accurate" box with an outer square box
            map_volume_box.color = line_color;
            create_fancy_map_volume_box(current_player->render_roomspace, x + box_lag_compensation_x, y + box_lag_compensation_y, z, (current_player->render_roomspace.slab_count == 0) ? SLC_RED : SLC_BROWN, true);
        }
    }
    else
    {
        // This is an "accurate"/"automagic" box
        create_fancy_map_volume_box(current_player->render_roomspace, x + box_lag_compensation_x, y + box_lag_compensation_y, z, line_color, false);
    }
    map_volume_box.color = default_color;
}

static void do_a_trig_gourad_tr(struct EngineCoord *engine_coordinate_1, struct EngineCoord *engine_coordinate_2, struct EngineCoord *engine_coordinate_3, short textr_idx, long argument5)
{
    struct BucketKindPolygonNearFP *triangle_bucket_near_1;
    struct BucketKindPolygonNearFP *triangle_bucket_near_2;
    struct BucketKindPolygonNearFP *triangle_bucket_near_3;
    struct BucketKindPolygonNearFP *triangle_bucket_near_4;
    struct BucketKindPolygonStandard *triangle_bucket_far;
    struct PolyPoint *polypoint1;
    struct PolyPoint *polypoint2;
    struct PolyPoint *polypoint3;
    struct XYZ *xyz1;
    struct XYZ *xyz2;
    struct XYZ *xyz3;
    struct XYZ *xyz4;
    struct XYZ *xyz5;
    struct XYZ *xyz6;
    short coordinate_1_frustum = engine_coordinate_1->clip_flags;
    short coordinate_2_frustum = engine_coordinate_2->clip_flags;
    short coordinate_3_frustum = engine_coordinate_3->clip_flags;

    if (((unsigned short)coordinate_1_frustum & (unsigned short)(coordinate_2_frustum & coordinate_3_frustum) & 0x1F8) == 0 && (engine_coordinate_1->view_height - engine_coordinate_2->view_height) * (engine_coordinate_3->view_width - engine_coordinate_2->view_width) + (engine_coordinate_3->view_height - engine_coordinate_2->view_height) * (engine_coordinate_2->view_width - engine_coordinate_1->view_width) > 0)
    {
        int choose_largest_z = engine_coordinate_1->z;
        if (engine_coordinate_2->z > choose_largest_z)
            choose_largest_z = engine_coordinate_2->z;
        if (engine_coordinate_3->z > choose_largest_z)
            choose_largest_z = engine_coordinate_3->z;
        int divided_z = choose_largest_z / 16;
        if (getpoly < poly_pool_end)
        {
            if ((((uint8_t)coordinate_3_frustum | (uint8_t)(coordinate_2_frustum | coordinate_1_frustum)) & 3) != 0)
            {
                triangle_bucket_near_1 = (struct BucketKindPolygonNearFP *)getpoly;
                getpoly += sizeof(struct BucketKindPolygonNearFP);
                triangle_bucket_near_1->subtype = splittypes[16 * (engine_coordinate_3->clip_flags & 3) + 4 * (engine_coordinate_1->clip_flags & 3) + (engine_coordinate_2->clip_flags & 3)];
                triangle_bucket_near_1->b.next = buckets[divided_z];
                triangle_bucket_near_1->b.kind = QK_PolygonNearFP;
                buckets[divided_z] = &triangle_bucket_near_1->b;
                triangle_bucket_near_1->block = textr_idx;
                triangle_bucket_near_1->vertex_first.X = engine_coordinate_1->view_width;
                triangle_bucket_near_1->vertex_first.Y = engine_coordinate_1->view_height;
                triangle_bucket_near_1->vertex_first.U = 0;
                triangle_bucket_near_1->vertex_first.V = 0;

                int coordinate_1_lightness = engine_coordinate_1->shade_intensity;
                int coordinate_1_distance = engine_coordinate_1->render_distance;

                if (argument5 >= 0)
                    coordinate_1_lightness = (coordinate_1_lightness * (3 * argument5 + 81920)) >> 17;

                int apply_lighting_to_triangle_nearby_1;
                if (fade_min >= coordinate_1_distance)
                {
                    apply_lighting_to_triangle_nearby_1 = coordinate_1_lightness << 8;
                }
                else if (fade_max > coordinate_1_distance)
                {
                    apply_lighting_to_triangle_nearby_1 = coordinate_1_lightness * (fade_scaler - coordinate_1_distance) / fade_range + 0x8000;
                }
                else
                {
                    apply_lighting_to_triangle_nearby_1 = 0x8000;
                }

                triangle_bucket_near_1->vertex_first.S = apply_lighting_to_triangle_nearby_1;
                triangle_bucket_near_1->vertex_second.X = engine_coordinate_2->view_width;
                triangle_bucket_near_1->vertex_second.Y = engine_coordinate_2->view_height;
                triangle_bucket_near_1->vertex_second.U = 0x1FFFFF;
                triangle_bucket_near_1->vertex_second.V = 0;

                int coordinate_2_lightness = engine_coordinate_2->shade_intensity;
                int coordinate_2_distance = engine_coordinate_2->render_distance;

                if (argument5 >= 0)
                    coordinate_2_lightness = (coordinate_2_lightness * (3 * argument5 + 81920)) >> 17;

                int apply_lighting_to_triangle_nearby_2;
                if (coordinate_2_distance <= fade_min)
                {
                    apply_lighting_to_triangle_nearby_2 = coordinate_2_lightness << 8;
                }
                else if (coordinate_2_distance < fade_max)
                {
                    apply_lighting_to_triangle_nearby_2 = coordinate_2_lightness * (fade_scaler - coordinate_2_distance) / fade_range + 0x8000;
                }
                else
                {
                    apply_lighting_to_triangle_nearby_2 = 0x8000;
                }

                triangle_bucket_near_1->vertex_second.S = apply_lighting_to_triangle_nearby_2;
                triangle_bucket_near_1->vertex_third.X = engine_coordinate_3->view_width;
                triangle_bucket_near_1->vertex_third.Y = engine_coordinate_3->view_height;
                triangle_bucket_near_1->vertex_third.U = 0x1FFFFF;
                triangle_bucket_near_1->vertex_third.V = 0x1FFFFF;

                int coordinate_3_lightness = engine_coordinate_3->shade_intensity;
                int coordinate_3_distance = engine_coordinate_3->render_distance;

                if (argument5 >= 0)
                    coordinate_3_lightness = (coordinate_3_lightness * (3 * argument5 + 81920)) >> 17;

                int apply_lighting_to_triangle_nearby_3;
                if (fade_min >= coordinate_3_distance)
                {
                    apply_lighting_to_triangle_nearby_3 = coordinate_3_lightness << 8;
                }
                else if (fade_max > coordinate_3_distance)
                {
                    apply_lighting_to_triangle_nearby_3 = coordinate_3_lightness * (fade_scaler - coordinate_3_distance) / fade_range + 0x8000;
                }
                else
                {
                    apply_lighting_to_triangle_nearby_3 = 0x8000;
                }

                triangle_bucket_near_1->vertex_third.S = apply_lighting_to_triangle_nearby_3;

                int coordinate_1_z = engine_coordinate_1->z;
                if (coordinate_1_z >= 32)
                {
                    int coordinate_2_z = engine_coordinate_2->z;
                    int coordinate_3_z = engine_coordinate_3->z;
                    if (coordinate_2_z >= 32)
                    {
                        if (coordinate_3_z >= 32)
                        {
                            triangle_bucket_near_1->coordinate_first.x = engine_coordinate_1->x;
                            triangle_bucket_near_1->coordinate_first.y = engine_coordinate_1->y;
                            triangle_bucket_near_1->coordinate_first.z = engine_coordinate_1->z;
                            xyz5 = &triangle_bucket_near_1->coordinate_second;
                            triangle_bucket_near_1->coordinate_second.x = engine_coordinate_2->x;
                            xyz6 = &triangle_bucket_near_1->coordinate_third;
                            xyz5->y = engine_coordinate_2->y;
                            xyz5->z = engine_coordinate_2->z;
                            xyz6->x = engine_coordinate_3->x;
                            xyz6->y = engine_coordinate_3->y;
                            xyz6->z = engine_coordinate_3->z;
                        }
                        else
                        {
                            triangle_bucket_near_4 = (struct BucketKindPolygonNearFP *)getpoly;
                            getpoly += sizeof(struct BucketKindPolygonNearFP);
                            triangle_bucket_near_4->subtype = splittypes[16 * (engine_coordinate_3->clip_flags & 3) + 4 * (engine_coordinate_1->clip_flags & 3) + (engine_coordinate_2->clip_flags & 3)];
                            triangle_bucket_near_4->b.next = buckets[divided_z];
                            triangle_bucket_near_4->b.kind = QK_PolygonNearFP;
                            buckets[divided_z] = &triangle_bucket_near_4->b;
                            triangle_bucket_near_4->block = textr_idx;
                            triangle_bucket_near_1->coordinate_first.x = engine_coordinate_1->x;
                            triangle_bucket_near_1->coordinate_first.y = engine_coordinate_1->y;
                            triangle_bucket_near_1->coordinate_first.z = engine_coordinate_1->z;
                            triangle_bucket_near_1->coordinate_second.x = engine_coordinate_2->x;
                            triangle_bucket_near_1->coordinate_second.y = engine_coordinate_2->y;
                            triangle_bucket_near_1->coordinate_second.z = engine_coordinate_2->z;
                            memcpy(&triangle_bucket_near_4->vertex_third, &triangle_bucket_near_1->vertex_third, sizeof(triangle_bucket_near_4->vertex_third));
                            memcpy(&triangle_bucket_near_4->vertex_second, &triangle_bucket_near_1->vertex_second, sizeof(triangle_bucket_near_4->vertex_second));

                            int z_ratio_1 = ((32 - engine_coordinate_3->z) << 8) / (engine_coordinate_1->z - engine_coordinate_3->z);

                            triangle_bucket_near_1->coordinate_third.x = engine_coordinate_3->x + ((z_ratio_1 * (engine_coordinate_1->x - engine_coordinate_3->x)) >> 8);
                            triangle_bucket_near_1->coordinate_third.y = engine_coordinate_3->y + ((z_ratio_1 * (engine_coordinate_1->y - engine_coordinate_3->y)) >> 8);
                            triangle_bucket_near_1->coordinate_third.z = 32;
                            perspective(&triangle_bucket_near_1->coordinate_third, &triangle_bucket_near_1->vertex_third);
                            triangle_bucket_near_1->vertex_third.U += (z_ratio_1 * (triangle_bucket_near_1->vertex_first.U - triangle_bucket_near_1->vertex_third.U)) >> 8;
                            triangle_bucket_near_1->vertex_third.V += (z_ratio_1 * (triangle_bucket_near_1->vertex_first.V - triangle_bucket_near_1->vertex_third.V)) >> 8;

                            int light_factor_1 = triangle_bucket_near_1->vertex_third.S;
                            int light_delta_1 = (z_ratio_1 * (triangle_bucket_near_1->vertex_first.S - light_factor_1)) >> 8;

                            polypoint3 = &triangle_bucket_near_1->vertex_third;
                            xyz4 = &triangle_bucket_near_1->coordinate_third;
                            xyz4[-3].z = light_factor_1 + light_delta_1;
                            memcpy(&triangle_bucket_near_4->vertex_first, polypoint3, sizeof(triangle_bucket_near_4->vertex_first));
                            triangle_bucket_near_4->coordinate_first.x = xyz4->x;
                            triangle_bucket_near_4->coordinate_first.y = xyz4->y;
                            triangle_bucket_near_4->coordinate_first.z = xyz4->z;
                            triangle_bucket_near_4->coordinate_second.x = engine_coordinate_2->x;
                            triangle_bucket_near_4->coordinate_second.y = engine_coordinate_2->y;
                            triangle_bucket_near_4->coordinate_second.z = engine_coordinate_2->z;

                            int z_ratio_2 = ((32 - engine_coordinate_3->z) << 8) / (engine_coordinate_2->z - engine_coordinate_3->z);

                            triangle_bucket_near_4->coordinate_third.x = engine_coordinate_3->x + ((z_ratio_2 * (engine_coordinate_2->x - engine_coordinate_3->x)) >> 8);
                            triangle_bucket_near_4->coordinate_third.y = engine_coordinate_3->y + ((z_ratio_2 * (engine_coordinate_2->y - engine_coordinate_3->y)) >> 8);
                            triangle_bucket_near_4->coordinate_third.z = 32;
                            perspective(&triangle_bucket_near_4->coordinate_third, &triangle_bucket_near_4->vertex_third);
                            triangle_bucket_near_4->vertex_third.U += (z_ratio_2 * (triangle_bucket_near_4->vertex_second.U - triangle_bucket_near_4->vertex_third.U)) >> 8;
                            triangle_bucket_near_4->vertex_third.V += (z_ratio_2 * (triangle_bucket_near_4->vertex_second.V - triangle_bucket_near_4->vertex_third.V)) >> 8;
                            triangle_bucket_near_4->vertex_third.S += (z_ratio_2 * (triangle_bucket_near_4->vertex_second.S - triangle_bucket_near_4->vertex_third.S)) >> 8;
                        }
                    }
                    else if (coordinate_3_z >= 32)
                    {
                        triangle_bucket_near_3 = (struct BucketKindPolygonNearFP *)getpoly;
                        getpoly += sizeof(struct BucketKindPolygonNearFP);
                        triangle_bucket_near_3->subtype = splittypes[16 * (engine_coordinate_3->clip_flags & 3) + 4 * (engine_coordinate_1->clip_flags & 3) + (engine_coordinate_2->clip_flags & 3)];
                        triangle_bucket_near_3->b.next = buckets[divided_z];
                        triangle_bucket_near_3->b.kind = QK_PolygonNearFP;
                        buckets[divided_z] = &triangle_bucket_near_3->b;
                        triangle_bucket_near_3->block = textr_idx;
                        triangle_bucket_near_1->coordinate_first.x = engine_coordinate_1->x;
                        triangle_bucket_near_1->coordinate_first.y = engine_coordinate_1->y;
                        triangle_bucket_near_1->coordinate_first.z = engine_coordinate_1->z;
                        triangle_bucket_near_1->coordinate_third.x = engine_coordinate_3->x;
                        triangle_bucket_near_1->coordinate_third.y = engine_coordinate_3->y;
                        triangle_bucket_near_1->coordinate_third.z = engine_coordinate_3->z;
                        memcpy(&triangle_bucket_near_3->vertex_second, &triangle_bucket_near_1->vertex_second, sizeof(triangle_bucket_near_3->vertex_second));
                        memcpy(&triangle_bucket_near_3->vertex_third, &triangle_bucket_near_1->vertex_third, sizeof(triangle_bucket_near_3->vertex_third));

                        int z_split_1 = ((32 - engine_coordinate_2->z) << 8) / (engine_coordinate_1->z - engine_coordinate_2->z);

                        triangle_bucket_near_1->coordinate_second.x = engine_coordinate_2->x + ((z_split_1 * (engine_coordinate_1->x - engine_coordinate_2->x)) >> 8);
                        triangle_bucket_near_1->coordinate_second.y = engine_coordinate_2->y + ((z_split_1 * (engine_coordinate_1->y - engine_coordinate_2->y)) >> 8);
                        triangle_bucket_near_1->coordinate_second.z = 32;
                        perspective(&triangle_bucket_near_1->coordinate_second, &triangle_bucket_near_1->vertex_second);
                        triangle_bucket_near_1->vertex_second.U += (z_split_1 * (triangle_bucket_near_1->vertex_first.U - triangle_bucket_near_1->vertex_second.U)) >> 8;
                        triangle_bucket_near_1->vertex_second.V += (z_split_1 * (triangle_bucket_near_1->vertex_first.V - triangle_bucket_near_1->vertex_second.V)) >> 8;

                        int light_base_1 = triangle_bucket_near_1->vertex_second.S;
                        int light_delta_2 = (z_split_1 * (triangle_bucket_near_1->vertex_first.S - light_base_1)) >> 8;

                        polypoint2 = &triangle_bucket_near_1->vertex_second;
                        xyz3 = &triangle_bucket_near_1->coordinate_second;
                        xyz3[-3].x = light_base_1 + light_delta_2;
                        memcpy(&triangle_bucket_near_3->vertex_first, polypoint2, sizeof(triangle_bucket_near_3->vertex_first));
                        triangle_bucket_near_3->coordinate_first.x = xyz3->x;
                        triangle_bucket_near_3->coordinate_first.y = xyz3->y;
                        triangle_bucket_near_3->coordinate_first.z = xyz3->z;
                        triangle_bucket_near_3->coordinate_third.x = engine_coordinate_3->x;
                        triangle_bucket_near_3->coordinate_third.y = engine_coordinate_3->y;
                        triangle_bucket_near_3->coordinate_third.z = engine_coordinate_3->z;

                        int z_ratio_3 = ((32 - engine_coordinate_2->z) << 8) / (engine_coordinate_3->z - engine_coordinate_2->z);

                        triangle_bucket_near_3->coordinate_second.x = engine_coordinate_2->x + ((z_ratio_3 * (engine_coordinate_3->x - engine_coordinate_2->x)) >> 8);
                        triangle_bucket_near_3->coordinate_second.y = engine_coordinate_2->y + ((z_ratio_3 * (engine_coordinate_3->y - engine_coordinate_2->y)) >> 8);
                        triangle_bucket_near_3->coordinate_second.z = 32;
                        perspective(&triangle_bucket_near_3->coordinate_second, &triangle_bucket_near_3->vertex_second);
                        triangle_bucket_near_3->vertex_second.U += (z_ratio_3 * (triangle_bucket_near_3->vertex_third.U - triangle_bucket_near_3->vertex_second.U)) >> 8;
                        triangle_bucket_near_3->vertex_second.V += (z_ratio_3 * (triangle_bucket_near_3->vertex_third.V - triangle_bucket_near_3->vertex_second.V)) >> 8;
                        triangle_bucket_near_3->vertex_second.S += (z_ratio_3 * (triangle_bucket_near_3->vertex_third.S - triangle_bucket_near_3->vertex_second.S)) >> 8;
                    }
                    else
                    {
                        int z_split_2 = ((32 - coordinate_2_z) << 8) / (coordinate_1_z - coordinate_2_z);

                        triangle_bucket_near_1->coordinate_second.x = engine_coordinate_2->x + ((z_split_2 * (engine_coordinate_1->x - engine_coordinate_2->x)) >> 8);
                        triangle_bucket_near_1->coordinate_second.y = engine_coordinate_2->y + ((z_split_2 * (engine_coordinate_1->y - engine_coordinate_2->y)) >> 8);
                        triangle_bucket_near_1->coordinate_second.z = 32;
                        perspective(&triangle_bucket_near_1->coordinate_second, &triangle_bucket_near_1->vertex_second);
                        triangle_bucket_near_1->vertex_second.U += (z_split_2 * (triangle_bucket_near_1->vertex_first.U - triangle_bucket_near_1->vertex_second.U)) >> 8;
                        triangle_bucket_near_1->vertex_second.V += (z_split_2 * (triangle_bucket_near_1->vertex_first.V - triangle_bucket_near_1->vertex_second.V)) >> 8;
                        triangle_bucket_near_1->vertex_second.S += (z_split_2 * (triangle_bucket_near_1->vertex_first.S - triangle_bucket_near_1->vertex_second.S)) >> 8;

                        int z_ratio_4 = ((32 - engine_coordinate_3->z) << 8) / (engine_coordinate_1->z - engine_coordinate_3->z);

                        triangle_bucket_near_1->coordinate_third.x = engine_coordinate_3->x + ((z_ratio_4 * (engine_coordinate_1->x - engine_coordinate_3->x)) >> 8);
                        triangle_bucket_near_1->coordinate_third.y = engine_coordinate_3->y + ((z_ratio_4 * (engine_coordinate_1->y - engine_coordinate_3->y)) >> 8);
                        triangle_bucket_near_1->coordinate_third.z = 32;
                        perspective(&triangle_bucket_near_1->coordinate_third, &triangle_bucket_near_1->vertex_third);
                        triangle_bucket_near_1->vertex_third.U += (z_ratio_4 * (triangle_bucket_near_1->vertex_first.U - triangle_bucket_near_1->vertex_third.U)) >> 8;
                        triangle_bucket_near_1->vertex_third.V += (z_ratio_4 * (triangle_bucket_near_1->vertex_first.V - triangle_bucket_near_1->vertex_third.V)) >> 8;

                        int light_base_2 = triangle_bucket_near_1->vertex_first.S;
                        int light_base_3 = triangle_bucket_near_1->vertex_third.S;

                        xyz2 = &triangle_bucket_near_1->coordinate_first;
                        xyz2[-1].z = light_base_3 + ((z_ratio_4 * (light_base_2 - light_base_3)) >> 8);
                        xyz2->x = engine_coordinate_1->x;
                        xyz2->y = engine_coordinate_1->y;
                        xyz2->z = engine_coordinate_1->z;
                    }
                }
                else if (engine_coordinate_2->z >= 32)
                {
                    if (engine_coordinate_3->z >= 32)
                    {
                        triangle_bucket_near_2 = (struct BucketKindPolygonNearFP *)getpoly;
                        getpoly += sizeof(struct BucketKindPolygonNearFP);
                        triangle_bucket_near_2->subtype = splittypes[16 * (engine_coordinate_3->clip_flags & 3) + 4 * (engine_coordinate_1->clip_flags & 3) + (engine_coordinate_2->clip_flags & 3)];
                        triangle_bucket_near_2->b.next = buckets[divided_z];
                        triangle_bucket_near_2->b.kind = QK_PolygonNearFP;
                        buckets[divided_z] = &triangle_bucket_near_2->b;
                        triangle_bucket_near_2->block = textr_idx;
                        triangle_bucket_near_1->coordinate_second.x = engine_coordinate_2->x;
                        triangle_bucket_near_1->coordinate_second.y = engine_coordinate_2->y;
                        triangle_bucket_near_1->coordinate_second.z = engine_coordinate_2->z;
                        triangle_bucket_near_1->coordinate_third.x = engine_coordinate_3->x;
                        triangle_bucket_near_1->coordinate_third.y = engine_coordinate_3->y;
                        triangle_bucket_near_1->coordinate_third.z = engine_coordinate_3->z;
                        memcpy(&triangle_bucket_near_2->vertex_first, &triangle_bucket_near_1->vertex_first, sizeof(triangle_bucket_near_2->vertex_first));
                        memcpy(&triangle_bucket_near_2->vertex_third, &triangle_bucket_near_1->vertex_third, sizeof(triangle_bucket_near_2->vertex_third));

                        int z_split_3 = ((32 - engine_coordinate_1->z) << 8) / (engine_coordinate_2->z - engine_coordinate_1->z);

                        triangle_bucket_near_1->coordinate_first.x = engine_coordinate_1->x + ((z_split_3 * (engine_coordinate_2->x - engine_coordinate_1->x)) >> 8);
                        triangle_bucket_near_1->coordinate_first.y = engine_coordinate_1->y + ((z_split_3 * (engine_coordinate_2->y - engine_coordinate_1->y)) >> 8);
                        triangle_bucket_near_1->coordinate_first.z = 32;
                        perspective(&triangle_bucket_near_1->coordinate_first, &triangle_bucket_near_1->vertex_first);
                        triangle_bucket_near_1->vertex_first.U += (z_split_3 * (triangle_bucket_near_1->vertex_second.U - triangle_bucket_near_1->vertex_first.U)) >> 8;
                        triangle_bucket_near_1->vertex_first.V += (z_split_3 * (triangle_bucket_near_1->vertex_second.V - triangle_bucket_near_1->vertex_first.V)) >> 8;

                        int light_base_4 = triangle_bucket_near_1->vertex_first.S;
                        int light_delta_3 = (z_split_3 * (triangle_bucket_near_1->vertex_second.S - light_base_4)) >> 8;

                        polypoint1 = &triangle_bucket_near_1->vertex_first;
                        xyz1 = &triangle_bucket_near_1->coordinate_first;
                        xyz1[-4].y = light_base_4 + light_delta_3;
                        memcpy(&triangle_bucket_near_2->vertex_second, polypoint1, sizeof(triangle_bucket_near_2->vertex_second));
                        triangle_bucket_near_2->coordinate_second.x = xyz1->x;
                        triangle_bucket_near_2->coordinate_second.y = xyz1->y;
                        triangle_bucket_near_2->coordinate_second.z = xyz1->z;
                        triangle_bucket_near_2->coordinate_third.x = engine_coordinate_3->x;
                        triangle_bucket_near_2->coordinate_third.y = engine_coordinate_3->y;
                        triangle_bucket_near_2->coordinate_third.z = engine_coordinate_3->z;

                        int z_ratio_5 = ((32 - engine_coordinate_1->z) << 8) / (engine_coordinate_3->z - engine_coordinate_1->z);

                        triangle_bucket_near_2->coordinate_first.x = engine_coordinate_1->x + ((z_ratio_5 * (engine_coordinate_3->x - engine_coordinate_1->x)) >> 8);
                        triangle_bucket_near_2->coordinate_first.y = engine_coordinate_1->y + ((z_ratio_5 * (engine_coordinate_3->y - engine_coordinate_1->y)) >> 8);
                        triangle_bucket_near_2->coordinate_first.z = 32;
                        perspective(&triangle_bucket_near_2->coordinate_first, &triangle_bucket_near_2->vertex_first);
                        triangle_bucket_near_2->vertex_first.U += (z_ratio_5 * (triangle_bucket_near_2->vertex_third.U - triangle_bucket_near_2->vertex_first.U)) >> 8;
                        triangle_bucket_near_2->vertex_first.V += (z_ratio_5 * (triangle_bucket_near_2->vertex_third.V - triangle_bucket_near_2->vertex_first.V)) >> 8;
                        triangle_bucket_near_2->vertex_first.S += (z_ratio_5 * (triangle_bucket_near_2->vertex_third.S - triangle_bucket_near_2->vertex_first.S)) >> 8;
                    }
                    else
                    {
                        triangle_bucket_near_1->coordinate_second.x = engine_coordinate_2->x;
                        triangle_bucket_near_1->coordinate_second.y = engine_coordinate_2->y;
                        triangle_bucket_near_1->coordinate_second.z = engine_coordinate_2->z;

                        int z_split_4 = ((32 - engine_coordinate_1->z) << 8) / (engine_coordinate_2->z - engine_coordinate_1->z);

                        triangle_bucket_near_1->coordinate_first.x = engine_coordinate_1->x + ((z_split_4 * (engine_coordinate_2->x - engine_coordinate_1->x)) >> 8);
                        triangle_bucket_near_1->coordinate_first.y = engine_coordinate_1->y + ((z_split_4 * (engine_coordinate_2->y - engine_coordinate_1->y)) >> 8);
                        triangle_bucket_near_1->coordinate_first.z = 32;
                        perspective(&triangle_bucket_near_1->coordinate_first, &triangle_bucket_near_1->vertex_first);
                        triangle_bucket_near_1->vertex_first.U += (z_split_4 * (triangle_bucket_near_1->vertex_second.U - triangle_bucket_near_1->vertex_first.U)) >> 8;
                        triangle_bucket_near_1->vertex_first.V += (z_split_4 * (triangle_bucket_near_1->vertex_second.V - triangle_bucket_near_1->vertex_first.V)) >> 8;
                        triangle_bucket_near_1->vertex_first.S += (z_split_4 * (triangle_bucket_near_1->vertex_second.S - triangle_bucket_near_1->vertex_first.S)) >> 8;

                        int z_ratio_6 = ((32 - engine_coordinate_3->z) << 8) / (engine_coordinate_2->z - engine_coordinate_3->z);

                        triangle_bucket_near_1->coordinate_third.x = engine_coordinate_3->x + ((z_ratio_6 * (engine_coordinate_2->x - engine_coordinate_3->x)) >> 8);
                        triangle_bucket_near_1->coordinate_third.y = engine_coordinate_3->y + ((z_ratio_6 * (engine_coordinate_2->y - engine_coordinate_3->y)) >> 8);
                        triangle_bucket_near_1->coordinate_third.z = 32;
                        perspective(&triangle_bucket_near_1->coordinate_third, &triangle_bucket_near_1->vertex_third);
                        triangle_bucket_near_1->vertex_third.U += (z_ratio_6 * (triangle_bucket_near_1->vertex_second.U - triangle_bucket_near_1->vertex_third.U)) >> 8;
                        triangle_bucket_near_1->vertex_third.V += (z_ratio_6 * (triangle_bucket_near_1->vertex_second.V - triangle_bucket_near_1->vertex_third.V)) >> 8;
                        triangle_bucket_near_1->vertex_third.S += (z_ratio_6 * (triangle_bucket_near_1->vertex_second.S - triangle_bucket_near_1->vertex_third.S)) >> 8;
                    }
                }
                else
                {
                    triangle_bucket_near_1->coordinate_third.x = engine_coordinate_3->x;
                    triangle_bucket_near_1->coordinate_third.y = engine_coordinate_3->y;
                    triangle_bucket_near_1->coordinate_third.z = engine_coordinate_3->z;

                    int z_ratio_7 = ((32 - engine_coordinate_1->z) << 8) / (engine_coordinate_3->z - engine_coordinate_1->z);

                    triangle_bucket_near_1->coordinate_first.x = engine_coordinate_1->x + ((z_ratio_7 * (engine_coordinate_3->x - engine_coordinate_1->x)) >> 8);
                    triangle_bucket_near_1->coordinate_first.y = engine_coordinate_1->y + ((z_ratio_7 * (engine_coordinate_3->y - engine_coordinate_1->y)) >> 8);
                    triangle_bucket_near_1->coordinate_first.z = 32;
                    perspective(&triangle_bucket_near_1->coordinate_first, &triangle_bucket_near_1->vertex_first);
                    triangle_bucket_near_1->vertex_first.U += (z_ratio_7 * (triangle_bucket_near_1->vertex_third.U - triangle_bucket_near_1->vertex_first.U)) >> 8;
                    triangle_bucket_near_1->vertex_first.V += (z_ratio_7 * (triangle_bucket_near_1->vertex_third.V - triangle_bucket_near_1->vertex_first.V)) >> 8;
                    triangle_bucket_near_1->vertex_first.S += (z_ratio_7 * (triangle_bucket_near_1->vertex_third.S - triangle_bucket_near_1->vertex_first.S)) >> 8;

                    int z_ratio_8 = ((32 - engine_coordinate_2->z) << 8) / (engine_coordinate_3->z - engine_coordinate_2->z);

                    triangle_bucket_near_1->coordinate_second.x = engine_coordinate_2->x + ((z_ratio_8 * (engine_coordinate_3->x - engine_coordinate_2->x)) >> 8);
                    triangle_bucket_near_1->coordinate_second.y = engine_coordinate_2->y + ((z_ratio_8 * (engine_coordinate_3->y - engine_coordinate_2->y)) >> 8);
                    triangle_bucket_near_1->coordinate_second.z = 32;
                    perspective(&triangle_bucket_near_1->coordinate_second, &triangle_bucket_near_1->vertex_second);
                    triangle_bucket_near_1->vertex_second.U += (z_ratio_8 * (triangle_bucket_near_1->vertex_third.U - triangle_bucket_near_1->vertex_second.U)) >> 8;
                    triangle_bucket_near_1->vertex_second.V += (z_ratio_8 * (triangle_bucket_near_1->vertex_third.V - triangle_bucket_near_1->vertex_second.V)) >> 8;
                    triangle_bucket_near_1->vertex_second.S += (z_ratio_8 * (triangle_bucket_near_1->vertex_third.S - triangle_bucket_near_1->vertex_second.S)) >> 8;
                }
            }
            else
            {
                triangle_bucket_far = (struct BucketKindPolygonStandard *)getpoly;
                getpoly += sizeof(struct BucketKindPolygonStandard);
                triangle_bucket_far->b.next = buckets[divided_z];
                triangle_bucket_far->b.kind = QK_PolygonStandard;
                buckets[divided_z] = &triangle_bucket_far->b;

                triangle_bucket_far->block = textr_idx;
                triangle_bucket_far->vertex_first.X = engine_coordinate_1->view_width;
                triangle_bucket_far->vertex_first.Y = engine_coordinate_1->view_height;
                triangle_bucket_far->vertex_first.U = 0;
                triangle_bucket_far->vertex_first.V = 0;

                int coordinate_1_lightness = engine_coordinate_1->shade_intensity;
                int coordinate_1_distance = engine_coordinate_1->render_distance;

                if (argument5 >= 0)
                    coordinate_1_lightness = (coordinate_1_lightness * (3 * argument5 + 81920)) >> 17;

                int apply_lighting_to_triangle_far_1;
                if (coordinate_1_distance <= fade_min)
                {
                    apply_lighting_to_triangle_far_1 = coordinate_1_lightness << 8;
                }
                else if (coordinate_1_distance < fade_max)
                {
                    apply_lighting_to_triangle_far_1 = coordinate_1_lightness * (fade_scaler - coordinate_1_distance) / fade_range + 0x8000;
                }
                else
                {
                    apply_lighting_to_triangle_far_1 = 0x8000;
                }

                triangle_bucket_far->vertex_first.S = apply_lighting_to_triangle_far_1;
                triangle_bucket_far->vertex_second.X = engine_coordinate_2->view_width;
                triangle_bucket_far->vertex_second.Y = engine_coordinate_2->view_height;
                triangle_bucket_far->vertex_second.U = 0x1FFFFF;
                triangle_bucket_far->vertex_second.V = 0;

                int coordinate_2_lightness = engine_coordinate_2->shade_intensity;
                int coordinate_2_distance = engine_coordinate_2->render_distance;

                if (argument5 >= 0)
                    coordinate_2_lightness = (coordinate_2_lightness * (3 * argument5 + 81920)) >> 17;

                int apply_lighting_to_triangle_far_2;
                if (coordinate_2_distance <= fade_min)
                {
                    apply_lighting_to_triangle_far_2 = coordinate_2_lightness << 8;
                }
                else if (coordinate_2_distance < fade_max)
                {
                    apply_lighting_to_triangle_far_2 = coordinate_2_lightness * (fade_scaler - coordinate_2_distance) / fade_range + 0x8000;
                }
                else
                {
                    apply_lighting_to_triangle_far_2 = 0x8000;
                }

                triangle_bucket_far->vertex_second.S = apply_lighting_to_triangle_far_2;
                triangle_bucket_far->vertex_third.X = engine_coordinate_3->view_width;
                triangle_bucket_far->vertex_third.Y = engine_coordinate_3->view_height;
                triangle_bucket_far->vertex_third.U = 0x1FFFFF;
                triangle_bucket_far->vertex_third.V = 0x1FFFFF;

                int coordinate_3_lightness = engine_coordinate_3->shade_intensity;
                int coordinate_3_distance = engine_coordinate_3->render_distance;

                if (argument5 >= 0)
                    coordinate_3_lightness = (coordinate_3_lightness * (3 * argument5 + 81920)) >> 17;

                if (coordinate_3_distance <= fade_min)
                {
                    triangle_bucket_far->vertex_third.S = coordinate_3_lightness << 8;
                }
                else if (coordinate_3_distance < fade_max)
                {
                    triangle_bucket_far->vertex_third.S = coordinate_3_lightness * (fade_scaler - coordinate_3_distance) / fade_range + 0x8000;
                }
                else
                {
                    triangle_bucket_far->vertex_third.S = 0x8000;
                }
            }
        }
    }
}

static void do_a_trig_gourad_bl(struct EngineCoord *engine_coordinate_1, struct EngineCoord *engine_coordinate_2, struct EngineCoord *engine_coordinate_3, short argument4, long argument5)
{
    struct BucketKindPolygonNearFP *triangle_bucket_near_1;
    struct BucketKindPolygonNearFP *triangle_bucket_near_2;
    struct BucketKindPolygonNearFP *triangle_bucket_near_3;
    struct BucketKindPolygonNearFP *triangle_bucket_near_4;
    struct BucketKindPolygonStandard *triangle_bucket_far;
    struct PolyPoint *polypoint1;
    struct PolyPoint *polypoint2;
    struct PolyPoint *polypoint3;
    struct XYZ *xyz1;
    struct XYZ *xyz2;
    struct XYZ *xyz3;
    struct XYZ *xyz4;
    struct XYZ *xyz5;
    struct XYZ *xyz6;
    short coordinate_1_frustum = engine_coordinate_1->clip_flags;
    short coordinate_2_frustum = engine_coordinate_2->clip_flags;
    short coordinate_3_frustum = engine_coordinate_3->clip_flags;

    if (((unsigned short)coordinate_2_frustum & (unsigned short)(coordinate_3_frustum & coordinate_1_frustum) & 0x1F8) == 0 && (engine_coordinate_2->view_width - engine_coordinate_1->view_width) * (engine_coordinate_3->view_height - engine_coordinate_2->view_height) + (engine_coordinate_3->view_width - engine_coordinate_2->view_width) * (engine_coordinate_1->view_height - engine_coordinate_2->view_height) > 0)
    {
        int choose_smallest_z = engine_coordinate_1->z;
        if (choose_smallest_z < engine_coordinate_2->z)
            choose_smallest_z = engine_coordinate_2->z;
        if (choose_smallest_z < engine_coordinate_3->z)
            choose_smallest_z = engine_coordinate_3->z;
        int divided_z = choose_smallest_z / 16;
        if (getpoly < poly_pool_end)
        {
            if ((((uint8_t)coordinate_1_frustum | (uint8_t)(coordinate_3_frustum | coordinate_2_frustum)) & 3) != 0)
            {
                triangle_bucket_near_1 = (struct BucketKindPolygonNearFP *)getpoly;
                getpoly += sizeof(struct BucketKindPolygonNearFP);
                triangle_bucket_near_1->subtype = splittypes[16 * (engine_coordinate_3->clip_flags & 3) + 4 * (engine_coordinate_1->clip_flags & 3) + (engine_coordinate_2->clip_flags & 3)];
                triangle_bucket_near_1->b.next = buckets[divided_z];
                triangle_bucket_near_1->b.kind = QK_PolygonNearFP;
                buckets[divided_z] = &triangle_bucket_near_1->b;
                triangle_bucket_near_1->block = argument4;

                triangle_bucket_near_1->vertex_first.X = engine_coordinate_1->view_width;
                triangle_bucket_near_1->vertex_first.Y = engine_coordinate_1->view_height;
                triangle_bucket_near_1->vertex_first.U = 0x1FFFFF;
                triangle_bucket_near_1->vertex_first.V = 0x1FFFFF;

                int coordinate_1_lightness = engine_coordinate_1->shade_intensity;
                int coordinate_1_distance = engine_coordinate_1->render_distance;

                if (argument5 >= 0)
                    coordinate_1_lightness = (coordinate_1_lightness * (3 * argument5 + 81920)) >> 17;

                int apply_lighting_to_triangle_nearby_1;
                if (coordinate_1_distance <= fade_min)
                {
                    apply_lighting_to_triangle_nearby_1 = coordinate_1_lightness << 8;
                }
                else if (coordinate_1_distance < fade_max)
                {
                    apply_lighting_to_triangle_nearby_1 = coordinate_1_lightness * (fade_scaler - coordinate_1_distance) / fade_range + 0x8000;
                }
                else
                {
                    apply_lighting_to_triangle_nearby_1 = 0x8000;
                }

                triangle_bucket_near_1->vertex_first.S = apply_lighting_to_triangle_nearby_1;
                triangle_bucket_near_1->vertex_second.X = engine_coordinate_2->view_width;
                triangle_bucket_near_1->vertex_second.Y = engine_coordinate_2->view_height;
                triangle_bucket_near_1->vertex_second.U = 0;
                triangle_bucket_near_1->vertex_second.V = 0x1FFFFF;

                int coordinate_2_lightness = engine_coordinate_2->shade_intensity;
                int coordinate_2_distance = engine_coordinate_2->render_distance;

                if (argument5 >= 0)
                    coordinate_2_lightness = (coordinate_2_lightness * (3 * argument5 + 81920)) >> 17;

                int apply_lighting_to_triangle_nearby_2;
                if (coordinate_2_distance <= fade_min)
                {
                    apply_lighting_to_triangle_nearby_2 = coordinate_2_lightness << 8;
                }
                else if (coordinate_2_distance < fade_max)
                {
                    apply_lighting_to_triangle_nearby_2 = coordinate_2_lightness * (fade_scaler - coordinate_2_distance) / fade_range + 0x8000;
                }
                else
                {
                    apply_lighting_to_triangle_nearby_2 = 0x8000;
                }

                triangle_bucket_near_1->vertex_second.S = apply_lighting_to_triangle_nearby_2;
                triangle_bucket_near_1->vertex_third.X = engine_coordinate_3->view_width;
                triangle_bucket_near_1->vertex_third.Y = engine_coordinate_3->view_height;
                triangle_bucket_near_1->vertex_third.U = 0;
                triangle_bucket_near_1->vertex_third.V = 0;

                int coordinate_3_lightness = engine_coordinate_3->shade_intensity;
                int coordinate_3_distance = engine_coordinate_3->render_distance;

                if (argument5 >= 0)
                    coordinate_3_lightness = (coordinate_3_lightness * (3 * argument5 + 81920)) >> 17;

                int apply_lighting_to_triangle_nearby_3;
                if (coordinate_3_distance <= fade_min)
                {
                    apply_lighting_to_triangle_nearby_3 = coordinate_3_lightness << 8;
                }
                else if (coordinate_3_distance < fade_max)
                {
                    apply_lighting_to_triangle_nearby_3 = coordinate_3_lightness * (fade_scaler - coordinate_3_distance) / fade_range + 0x8000;
                }
                else
                {
                    apply_lighting_to_triangle_nearby_3 = 0x8000;
                }

                triangle_bucket_near_1->vertex_third.S = apply_lighting_to_triangle_nearby_3;

                int coordinate_1_z = engine_coordinate_1->z;
                if (coordinate_1_z >= 32)
                {
                    int coordinate_2_z = engine_coordinate_2->z;
                    int coordinate_3_z = engine_coordinate_3->z;
                    if (coordinate_2_z >= 32)
                    {
                        if (coordinate_3_z >= 32)
                        {
                            triangle_bucket_near_1->coordinate_first.x = engine_coordinate_1->x;
                            triangle_bucket_near_1->coordinate_first.y = engine_coordinate_1->y;
                            triangle_bucket_near_1->coordinate_first.z = engine_coordinate_1->z;
                            xyz5 = &triangle_bucket_near_1->coordinate_second;
                            triangle_bucket_near_1->coordinate_second.x = engine_coordinate_2->x;
                            xyz6 = &triangle_bucket_near_1->coordinate_third;
                            xyz5->y = engine_coordinate_2->y;
                            xyz5->z = engine_coordinate_2->z;
                            xyz6->x = engine_coordinate_3->x;
                            xyz6->y = engine_coordinate_3->y;
                            xyz6->z = engine_coordinate_3->z;
                        }
                        else
                        {
                            triangle_bucket_near_4 = (struct BucketKindPolygonNearFP *)getpoly;
                            getpoly += sizeof(struct BucketKindPolygonNearFP);
                            triangle_bucket_near_4->subtype = splittypes[16 * (engine_coordinate_3->clip_flags & 3) + 4 * (engine_coordinate_1->clip_flags & 3) + (engine_coordinate_2->clip_flags & 3)];
                            triangle_bucket_near_4->b.next = buckets[divided_z];
                            triangle_bucket_near_4->b.kind = QK_PolygonNearFP;
                            buckets[divided_z] = &triangle_bucket_near_4->b;
                            triangle_bucket_near_4->block = argument4;
                            triangle_bucket_near_1->coordinate_first.x = engine_coordinate_1->x;
                            triangle_bucket_near_1->coordinate_first.y = engine_coordinate_1->y;
                            triangle_bucket_near_1->coordinate_first.z = engine_coordinate_1->z;
                            triangle_bucket_near_1->coordinate_second.x = engine_coordinate_2->x;
                            triangle_bucket_near_1->coordinate_second.y = engine_coordinate_2->y;
                            triangle_bucket_near_1->coordinate_second.z = engine_coordinate_2->z;
                            memcpy(&triangle_bucket_near_4->vertex_third, &triangle_bucket_near_1->vertex_third, sizeof(triangle_bucket_near_4->vertex_third));
                            memcpy(&triangle_bucket_near_4->vertex_second, &triangle_bucket_near_1->vertex_second, sizeof(triangle_bucket_near_4->vertex_second));

                            int z_ratio_1 = ((32 - engine_coordinate_3->z) << 8) / (engine_coordinate_1->z - engine_coordinate_3->z);

                            triangle_bucket_near_1->coordinate_third.x = engine_coordinate_3->x + ((z_ratio_1 * (engine_coordinate_1->x - engine_coordinate_3->x)) >> 8);
                            triangle_bucket_near_1->coordinate_third.y = engine_coordinate_3->y + ((z_ratio_1 * (engine_coordinate_1->y - engine_coordinate_3->y)) >> 8);
                            triangle_bucket_near_1->coordinate_third.z = 32;
                            perspective(&triangle_bucket_near_1->coordinate_third, &triangle_bucket_near_1->vertex_third);
                            triangle_bucket_near_1->vertex_third.U += (z_ratio_1 * (triangle_bucket_near_1->vertex_first.U - triangle_bucket_near_1->vertex_third.U)) >> 8;
                            triangle_bucket_near_1->vertex_third.V += (z_ratio_1 * (triangle_bucket_near_1->vertex_first.V - triangle_bucket_near_1->vertex_third.V)) >> 8;

                            int light_factor_1 = triangle_bucket_near_1->vertex_third.S;
                            int light_delta_1 = (z_ratio_1 * (triangle_bucket_near_1->vertex_first.S - light_factor_1)) >> 8;

                            polypoint3 = &triangle_bucket_near_1->vertex_third;
                            xyz4 = &triangle_bucket_near_1->coordinate_third;
                            xyz4[-3].z = light_factor_1 + light_delta_1;
                            memcpy(&triangle_bucket_near_4->vertex_first, polypoint3, sizeof(triangle_bucket_near_4->vertex_first));
                            triangle_bucket_near_4->coordinate_first.x = xyz4->x;
                            triangle_bucket_near_4->coordinate_first.y = xyz4->y;
                            triangle_bucket_near_4->coordinate_first.z = xyz4->z;
                            triangle_bucket_near_4->coordinate_second.x = engine_coordinate_2->x;
                            triangle_bucket_near_4->coordinate_second.y = engine_coordinate_2->y;
                            triangle_bucket_near_4->coordinate_second.z = engine_coordinate_2->z;

                            int z_ratio_2 = ((32 - engine_coordinate_3->z) << 8) / (engine_coordinate_2->z - engine_coordinate_3->z);

                            triangle_bucket_near_4->coordinate_third.x = engine_coordinate_3->x + ((z_ratio_2 * (engine_coordinate_2->x - engine_coordinate_3->x)) >> 8);
                            triangle_bucket_near_4->coordinate_third.y = engine_coordinate_3->y + ((z_ratio_2 * (engine_coordinate_2->y - engine_coordinate_3->y)) >> 8);
                            triangle_bucket_near_4->coordinate_third.z = 32;
                            perspective(&triangle_bucket_near_4->coordinate_third, &triangle_bucket_near_4->vertex_third);
                            triangle_bucket_near_4->vertex_third.U += (z_ratio_2 * (triangle_bucket_near_4->vertex_second.U - triangle_bucket_near_4->vertex_third.U)) >> 8;
                            triangle_bucket_near_4->vertex_third.V += (z_ratio_2 * (triangle_bucket_near_4->vertex_second.V - triangle_bucket_near_4->vertex_third.V)) >> 8;
                            triangle_bucket_near_4->vertex_third.S += (z_ratio_2 * (triangle_bucket_near_4->vertex_second.S - triangle_bucket_near_4->vertex_third.S)) >> 8;
                        }
                    }
                    else if (coordinate_3_z >= 32)
                    {
                        triangle_bucket_near_3 = (struct BucketKindPolygonNearFP *)getpoly;
                        getpoly += sizeof(struct BucketKindPolygonNearFP);
                        triangle_bucket_near_3->subtype = splittypes[16 * (engine_coordinate_3->clip_flags & 3) + 4 * (engine_coordinate_1->clip_flags & 3) + (engine_coordinate_2->clip_flags & 3)];
                        triangle_bucket_near_3->b.next = buckets[divided_z];
                        triangle_bucket_near_3->b.kind = QK_PolygonNearFP;
                        buckets[divided_z] = &triangle_bucket_near_3->b;
                        triangle_bucket_near_3->block = argument4;
                        triangle_bucket_near_1->coordinate_first.x = engine_coordinate_1->x;
                        triangle_bucket_near_1->coordinate_first.y = engine_coordinate_1->y;
                        triangle_bucket_near_1->coordinate_first.z = engine_coordinate_1->z;
                        triangle_bucket_near_1->coordinate_third.x = engine_coordinate_3->x;
                        triangle_bucket_near_1->coordinate_third.y = engine_coordinate_3->y;
                        triangle_bucket_near_1->coordinate_third.z = engine_coordinate_3->z;
                        memcpy(&triangle_bucket_near_3->vertex_second, &triangle_bucket_near_1->vertex_second, sizeof(triangle_bucket_near_3->vertex_second));
                        memcpy(&triangle_bucket_near_3->vertex_third, &triangle_bucket_near_1->vertex_third, sizeof(triangle_bucket_near_3->vertex_third));

                        int z_split_1 = ((32 - engine_coordinate_2->z) << 8) / (engine_coordinate_1->z - engine_coordinate_2->z);

                        triangle_bucket_near_1->coordinate_second.x = engine_coordinate_2->x + ((z_split_1 * (engine_coordinate_1->x - engine_coordinate_2->x)) >> 8);
                        triangle_bucket_near_1->coordinate_second.y = engine_coordinate_2->y + ((z_split_1 * (engine_coordinate_1->y - engine_coordinate_2->y)) >> 8);
                        triangle_bucket_near_1->coordinate_second.z = 32;
                        perspective(&triangle_bucket_near_1->coordinate_second, &triangle_bucket_near_1->vertex_second);
                        triangle_bucket_near_1->vertex_second.U += (z_split_1 * (triangle_bucket_near_1->vertex_first.U - triangle_bucket_near_1->vertex_second.U)) >> 8;
                        triangle_bucket_near_1->vertex_second.V += (z_split_1 * (triangle_bucket_near_1->vertex_first.V - triangle_bucket_near_1->vertex_second.V)) >> 8;

                        int light_base_1 = triangle_bucket_near_1->vertex_second.S;
                        int light_delta_2 = (z_split_1 * (triangle_bucket_near_1->vertex_first.S - light_base_1)) >> 8;

                        polypoint2 = &triangle_bucket_near_1->vertex_second;
                        xyz3 = &triangle_bucket_near_1->coordinate_second;
                        xyz3[-3].x = light_base_1 + light_delta_2;
                        memcpy(&triangle_bucket_near_3->vertex_first, polypoint2, sizeof(triangle_bucket_near_3->vertex_first));
                        triangle_bucket_near_3->coordinate_first.x = xyz3->x;
                        triangle_bucket_near_3->coordinate_first.y = xyz3->y;
                        triangle_bucket_near_3->coordinate_first.z = xyz3->z;
                        triangle_bucket_near_3->coordinate_third.x = engine_coordinate_3->x;
                        triangle_bucket_near_3->coordinate_third.y = engine_coordinate_3->y;
                        triangle_bucket_near_3->coordinate_third.z = engine_coordinate_3->z;

                        int z_ratio_3 = ((32 - engine_coordinate_2->z) << 8) / (engine_coordinate_3->z - engine_coordinate_2->z);

                        triangle_bucket_near_3->coordinate_second.x = engine_coordinate_2->x + ((z_ratio_3 * (engine_coordinate_3->x - engine_coordinate_2->x)) >> 8);
                        triangle_bucket_near_3->coordinate_second.y = engine_coordinate_2->y + ((z_ratio_3 * (engine_coordinate_3->y - engine_coordinate_2->y)) >> 8);
                        triangle_bucket_near_3->coordinate_second.z = 32;
                        perspective(&triangle_bucket_near_3->coordinate_second, &triangle_bucket_near_3->vertex_second);
                        triangle_bucket_near_3->vertex_second.U += (z_ratio_3 * (triangle_bucket_near_3->vertex_third.U - triangle_bucket_near_3->vertex_second.U)) >> 8;
                        triangle_bucket_near_3->vertex_second.V += (z_ratio_3 * (triangle_bucket_near_3->vertex_third.V - triangle_bucket_near_3->vertex_second.V)) >> 8;
                        triangle_bucket_near_3->vertex_second.S += (z_ratio_3 * (triangle_bucket_near_3->vertex_third.S - triangle_bucket_near_3->vertex_second.S)) >> 8;
                    }
                    else
                    {
                        int z_split_2 = ((32 - coordinate_2_z) << 8) / (coordinate_1_z - coordinate_2_z);

                        triangle_bucket_near_1->coordinate_second.x = engine_coordinate_2->x + ((z_split_2 * (engine_coordinate_1->x - engine_coordinate_2->x)) >> 8);
                        triangle_bucket_near_1->coordinate_second.y = engine_coordinate_2->y + ((z_split_2 * (engine_coordinate_1->y - engine_coordinate_2->y)) >> 8);
                        triangle_bucket_near_1->coordinate_second.z = 32;
                        perspective(&triangle_bucket_near_1->coordinate_second, &triangle_bucket_near_1->vertex_second);
                        triangle_bucket_near_1->vertex_second.U += (z_split_2 * (triangle_bucket_near_1->vertex_first.U - triangle_bucket_near_1->vertex_second.U)) >> 8;
                        triangle_bucket_near_1->vertex_second.V += (z_split_2 * (triangle_bucket_near_1->vertex_first.V - triangle_bucket_near_1->vertex_second.V)) >> 8;
                        triangle_bucket_near_1->vertex_second.S += (z_split_2 * (triangle_bucket_near_1->vertex_first.S - triangle_bucket_near_1->vertex_second.S)) >> 8;

                        int z_ratio_4 = ((32 - engine_coordinate_3->z) << 8) / (engine_coordinate_1->z - engine_coordinate_3->z);

                        triangle_bucket_near_1->coordinate_third.x = engine_coordinate_3->x + ((z_ratio_4 * (engine_coordinate_1->x - engine_coordinate_3->x)) >> 8);
                        triangle_bucket_near_1->coordinate_third.y = engine_coordinate_3->y + ((z_ratio_4 * (engine_coordinate_1->y - engine_coordinate_3->y)) >> 8);
                        triangle_bucket_near_1->coordinate_third.z = 32;
                        perspective(&triangle_bucket_near_1->coordinate_third, &triangle_bucket_near_1->vertex_third);
                        triangle_bucket_near_1->vertex_third.U += (z_ratio_4 * (triangle_bucket_near_1->vertex_first.U - triangle_bucket_near_1->vertex_third.U)) >> 8;
                        triangle_bucket_near_1->vertex_third.V += (z_ratio_4 * (triangle_bucket_near_1->vertex_first.V - triangle_bucket_near_1->vertex_third.V)) >> 8;

                        int light_base_2 = triangle_bucket_near_1->vertex_first.S;
                        int light_base_3 = triangle_bucket_near_1->vertex_third.S;

                        xyz2 = &triangle_bucket_near_1->coordinate_first;
                        xyz2[-1].z = light_base_3 + ((z_ratio_4 * (light_base_2 - light_base_3)) >> 8);
                        xyz2->x = engine_coordinate_1->x;
                        xyz2->y = engine_coordinate_1->y;
                        xyz2->z = engine_coordinate_1->z;
                    }
                }
                else if (engine_coordinate_2->z >= 32)
                {
                    if (engine_coordinate_3->z >= 32)
                    {
                        triangle_bucket_near_2 = (struct BucketKindPolygonNearFP *)getpoly;
                        getpoly += sizeof(struct BucketKindPolygonNearFP);
                        triangle_bucket_near_2->subtype = splittypes[16 * (engine_coordinate_3->clip_flags & 3) + 4 * (engine_coordinate_1->clip_flags & 3) + (engine_coordinate_2->clip_flags & 3)];
                        triangle_bucket_near_2->b.next = buckets[divided_z];
                        triangle_bucket_near_2->b.kind = QK_PolygonNearFP;
                        buckets[divided_z] = &triangle_bucket_near_2->b;
                        triangle_bucket_near_2->block = argument4;
                        triangle_bucket_near_1->coordinate_second.x = engine_coordinate_2->x;
                        triangle_bucket_near_1->coordinate_second.y = engine_coordinate_2->y;
                        triangle_bucket_near_1->coordinate_second.z = engine_coordinate_2->z;
                        triangle_bucket_near_1->coordinate_third.x = engine_coordinate_3->x;
                        triangle_bucket_near_1->coordinate_third.y = engine_coordinate_3->y;
                        triangle_bucket_near_1->coordinate_third.z = engine_coordinate_3->z;
                        memcpy(&triangle_bucket_near_2->vertex_first, &triangle_bucket_near_1->vertex_first, sizeof(triangle_bucket_near_2->vertex_first));
                        memcpy(&triangle_bucket_near_2->vertex_third, &triangle_bucket_near_1->vertex_third, sizeof(triangle_bucket_near_2->vertex_third));

                        int z_split_3 = ((32 - engine_coordinate_1->z) << 8) / (engine_coordinate_2->z - engine_coordinate_1->z);

                        triangle_bucket_near_1->coordinate_first.x = engine_coordinate_1->x + ((z_split_3 * (engine_coordinate_2->x - engine_coordinate_1->x)) >> 8);
                        triangle_bucket_near_1->coordinate_first.y = engine_coordinate_1->y + ((z_split_3 * (engine_coordinate_2->y - engine_coordinate_1->y)) >> 8);
                        triangle_bucket_near_1->coordinate_first.z = 32;
                        perspective(&triangle_bucket_near_1->coordinate_first, &triangle_bucket_near_1->vertex_first);
                        triangle_bucket_near_1->vertex_first.U += (z_split_3 * (triangle_bucket_near_1->vertex_second.U - triangle_bucket_near_1->vertex_first.U)) >> 8;
                        triangle_bucket_near_1->vertex_first.V += (z_split_3 * (triangle_bucket_near_1->vertex_second.V - triangle_bucket_near_1->vertex_first.V)) >> 8;

                        int light_base_4 = triangle_bucket_near_1->vertex_first.S;
                        int light_delta_3 = (z_split_3 * (triangle_bucket_near_1->vertex_second.S - light_base_4)) >> 8;

                        polypoint1 = &triangle_bucket_near_1->vertex_first;
                        xyz1 = &triangle_bucket_near_1->coordinate_first;
                        xyz1[-4].y = light_base_4 + light_delta_3;
                        memcpy(&triangle_bucket_near_2->vertex_second, polypoint1, sizeof(triangle_bucket_near_2->vertex_second));
                        triangle_bucket_near_2->coordinate_second.x = xyz1->x;
                        triangle_bucket_near_2->coordinate_second.y = xyz1->y;
                        triangle_bucket_near_2->coordinate_second.z = xyz1->z;
                        triangle_bucket_near_2->coordinate_third.x = engine_coordinate_3->x;
                        triangle_bucket_near_2->coordinate_third.y = engine_coordinate_3->y;
                        triangle_bucket_near_2->coordinate_third.z = engine_coordinate_3->z;

                        int z_ratio_5 = ((32 - engine_coordinate_1->z) << 8) / (engine_coordinate_3->z - engine_coordinate_1->z);

                        triangle_bucket_near_2->coordinate_first.x = engine_coordinate_1->x + ((z_ratio_5 * (engine_coordinate_3->x - engine_coordinate_1->x)) >> 8);
                        triangle_bucket_near_2->coordinate_first.y = engine_coordinate_1->y + ((z_ratio_5 * (engine_coordinate_3->y - engine_coordinate_1->y)) >> 8);
                        triangle_bucket_near_2->coordinate_first.z = 32;
                        perspective(&triangle_bucket_near_2->coordinate_first, &triangle_bucket_near_2->vertex_first);
                        triangle_bucket_near_2->vertex_first.U += (z_ratio_5 * (triangle_bucket_near_2->vertex_third.U - triangle_bucket_near_2->vertex_first.U)) >> 8;
                        triangle_bucket_near_2->vertex_first.V += (z_ratio_5 * (triangle_bucket_near_2->vertex_third.V - triangle_bucket_near_2->vertex_first.V)) >> 8;
                        triangle_bucket_near_2->vertex_first.S += (z_ratio_5 * (triangle_bucket_near_2->vertex_third.S - triangle_bucket_near_2->vertex_first.S)) >> 8;
                    }
                    else
                    {
                        triangle_bucket_near_1->coordinate_second.x = engine_coordinate_2->x;
                        triangle_bucket_near_1->coordinate_second.y = engine_coordinate_2->y;
                        triangle_bucket_near_1->coordinate_second.z = engine_coordinate_2->z;

                        int z_split_4 = ((32 - engine_coordinate_1->z) << 8) / (engine_coordinate_2->z - engine_coordinate_1->z);

                        triangle_bucket_near_1->coordinate_first.x = engine_coordinate_1->x + ((z_split_4 * (engine_coordinate_2->x - engine_coordinate_1->x)) >> 8);
                        triangle_bucket_near_1->coordinate_first.y = engine_coordinate_1->y + ((z_split_4 * (engine_coordinate_2->y - engine_coordinate_1->y)) >> 8);
                        triangle_bucket_near_1->coordinate_first.z = 32;
                        perspective(&triangle_bucket_near_1->coordinate_first, &triangle_bucket_near_1->vertex_first);
                        triangle_bucket_near_1->vertex_first.U += (z_split_4 * (triangle_bucket_near_1->vertex_second.U - triangle_bucket_near_1->vertex_first.U)) >> 8;
                        triangle_bucket_near_1->vertex_first.V += (z_split_4 * (triangle_bucket_near_1->vertex_second.V - triangle_bucket_near_1->vertex_first.V)) >> 8;
                        triangle_bucket_near_1->vertex_first.S += (z_split_4 * (triangle_bucket_near_1->vertex_second.S - triangle_bucket_near_1->vertex_first.S)) >> 8;

                        int z_ratio_6 = ((32 - engine_coordinate_3->z) << 8) / (engine_coordinate_2->z - engine_coordinate_3->z);

                        triangle_bucket_near_1->coordinate_third.x = engine_coordinate_3->x + ((z_ratio_6 * (engine_coordinate_2->x - engine_coordinate_3->x)) >> 8);
                        triangle_bucket_near_1->coordinate_third.y = engine_coordinate_3->y + ((z_ratio_6 * (engine_coordinate_2->y - engine_coordinate_3->y)) >> 8);
                        triangle_bucket_near_1->coordinate_third.z = 32;
                        perspective(&triangle_bucket_near_1->coordinate_third, &triangle_bucket_near_1->vertex_third);
                        triangle_bucket_near_1->vertex_third.U += (z_ratio_6 * (triangle_bucket_near_1->vertex_second.U - triangle_bucket_near_1->vertex_third.U)) >> 8;
                        triangle_bucket_near_1->vertex_third.V += (z_ratio_6 * (triangle_bucket_near_1->vertex_second.V - triangle_bucket_near_1->vertex_third.V)) >> 8;
                        triangle_bucket_near_1->vertex_third.S += (z_ratio_6 * (triangle_bucket_near_1->vertex_second.S - triangle_bucket_near_1->vertex_third.S)) >> 8;
                    }
                }
                else
                {
                    triangle_bucket_near_1->coordinate_third.x = engine_coordinate_3->x;
                    triangle_bucket_near_1->coordinate_third.y = engine_coordinate_3->y;
                    triangle_bucket_near_1->coordinate_third.z = engine_coordinate_3->z;

                    int z_ratio_7 = ((32 - engine_coordinate_1->z) << 8) / (engine_coordinate_3->z - engine_coordinate_1->z);

                    triangle_bucket_near_1->coordinate_first.x = engine_coordinate_1->x + ((z_ratio_7 * (engine_coordinate_3->x - engine_coordinate_1->x)) >> 8);
                    triangle_bucket_near_1->coordinate_first.y = engine_coordinate_1->y + ((z_ratio_7 * (engine_coordinate_3->y - engine_coordinate_1->y)) >> 8);
                    triangle_bucket_near_1->coordinate_first.z = 32;
                    perspective(&triangle_bucket_near_1->coordinate_first, &triangle_bucket_near_1->vertex_first);
                    triangle_bucket_near_1->vertex_first.U += (z_ratio_7 * (triangle_bucket_near_1->vertex_third.U - triangle_bucket_near_1->vertex_first.U)) >> 8;
                    triangle_bucket_near_1->vertex_first.V += (z_ratio_7 * (triangle_bucket_near_1->vertex_third.V - triangle_bucket_near_1->vertex_first.V)) >> 8;
                    triangle_bucket_near_1->vertex_first.S += (z_ratio_7 * (triangle_bucket_near_1->vertex_third.S - triangle_bucket_near_1->vertex_first.S)) >> 8;

                    int z_ratio_8 = ((32 - engine_coordinate_2->z) << 8) / (engine_coordinate_3->z - engine_coordinate_2->z);

                    triangle_bucket_near_1->coordinate_second.x = engine_coordinate_2->x + ((z_ratio_8 * (engine_coordinate_3->x - engine_coordinate_2->x)) >> 8);
                    triangle_bucket_near_1->coordinate_second.y = engine_coordinate_2->y + ((z_ratio_8 * (engine_coordinate_3->y - engine_coordinate_2->y)) >> 8);
                    triangle_bucket_near_1->coordinate_second.z = 32;
                    perspective(&triangle_bucket_near_1->coordinate_second, &triangle_bucket_near_1->vertex_second);
                    triangle_bucket_near_1->vertex_second.U += (z_ratio_8 * (triangle_bucket_near_1->vertex_third.U - triangle_bucket_near_1->vertex_second.U)) >> 8;
                    triangle_bucket_near_1->vertex_second.V += (z_ratio_8 * (triangle_bucket_near_1->vertex_third.V - triangle_bucket_near_1->vertex_second.V)) >> 8;
                    triangle_bucket_near_1->vertex_second.S += (z_ratio_8 * (triangle_bucket_near_1->vertex_third.S - triangle_bucket_near_1->vertex_second.S)) >> 8;
                }
            }
            else
            {
                triangle_bucket_far = (struct BucketKindPolygonStandard *)getpoly;
                getpoly += sizeof(struct BucketKindPolygonStandard);
                triangle_bucket_far->b.next = buckets[divided_z];
                triangle_bucket_far->b.kind = QK_PolygonStandard;
                buckets[divided_z] = &triangle_bucket_far->b;
                triangle_bucket_far->block = argument4;

                triangle_bucket_far->vertex_first.X = engine_coordinate_1->view_width;
                triangle_bucket_far->vertex_first.Y = engine_coordinate_1->view_height;
                triangle_bucket_far->vertex_first.U = 0x1FFFFF;
                triangle_bucket_far->vertex_first.V = 0x1FFFFF;

                int coordinate_1_lightness = engine_coordinate_1->shade_intensity;
                int coordinate_1_distance = engine_coordinate_1->render_distance;

                if (argument5 >= 0)
                    coordinate_1_lightness = (coordinate_1_lightness * (3 * argument5 + 81920)) >> 17;

                int apply_lighting_to_triangle_far_1;
                if (coordinate_1_distance <= fade_min)
                {
                    apply_lighting_to_triangle_far_1 = coordinate_1_lightness << 8;
                }
                else if (coordinate_1_distance < fade_max)
                {
                    apply_lighting_to_triangle_far_1 = coordinate_1_lightness * (fade_scaler - coordinate_1_distance) / fade_range + 0x8000;
                }
                else
                {
                    apply_lighting_to_triangle_far_1 = 0x8000;
                }

                triangle_bucket_far->vertex_first.S = apply_lighting_to_triangle_far_1;
                triangle_bucket_far->vertex_second.X = engine_coordinate_2->view_width;
                triangle_bucket_far->vertex_second.Y = engine_coordinate_2->view_height;
                triangle_bucket_far->vertex_second.U = 0;
                triangle_bucket_far->vertex_second.V = 0x1FFFFF;

                int coordinate_2_lightness = engine_coordinate_2->shade_intensity;
                int coordinate_2_distance = engine_coordinate_2->render_distance;

                if (argument5 >= 0)
                    coordinate_2_lightness = (coordinate_2_lightness * (3 * argument5 + 81920)) >> 17;

                int apply_lighting_to_triangle_far_2;
                if (coordinate_2_distance <= fade_min)
                {
                    apply_lighting_to_triangle_far_2 = coordinate_2_lightness << 8;
                }
                else if (coordinate_2_distance < fade_max)
                {
                    apply_lighting_to_triangle_far_2 = coordinate_2_lightness * (fade_scaler - coordinate_2_distance) / fade_range + 0x8000;
                }
                else
                {
                    apply_lighting_to_triangle_far_2 = 0x8000;
                }

                triangle_bucket_far->vertex_second.S = apply_lighting_to_triangle_far_2;
                triangle_bucket_far->vertex_third.X = engine_coordinate_3->view_width;
                triangle_bucket_far->vertex_third.Y = engine_coordinate_3->view_height;
                triangle_bucket_far->vertex_third.U = 0;
                triangle_bucket_far->vertex_third.V = 0;

                int coordinate_3_lightness = engine_coordinate_3->shade_intensity;
                int coordinate_3_distance = engine_coordinate_3->render_distance;

                if (argument5 >= 0)
                    coordinate_3_lightness = (coordinate_3_lightness * (3 * argument5 + 81920)) >> 17;

                if (coordinate_3_distance <= fade_min)
                {
                    triangle_bucket_far->vertex_third.S = coordinate_3_lightness << 8;
                }
                else if (coordinate_3_distance < fade_max)
                {
                    triangle_bucket_far->vertex_third.S = coordinate_3_lightness * (fade_scaler - coordinate_3_distance) / fade_range + 0x8000;
                }
                else
                {
                    triangle_bucket_far->vertex_third.S = 0x8000;
                }
            }
        }
    }
}

static TbBool add_light_to_nearest_list(struct NearestLights* nlgt, int32_t * nlgt_dist, const struct Light* lgt, long dist)
{
    int i;
    for (i = settings.video_shadows-1; i > 0; i--)
    {
        nlgt_dist[i] = nlgt_dist[i-1];
        nlgt->coord[i] = nlgt->coord[i-1];
    }
    nlgt_dist[0] = dist;
    nlgt->coord[0] = lgt->mappos;
    return true;
}

static void find_closest_lights_on_list(struct NearestLights *nlgt, int32_t *nlgt_dist, const struct Coord3d *pos, ThingIndex list_start_idx)
{
    long i;
    unsigned long k;
    if (settings.video_shadows < 1)
        return;
    i = list_start_idx;
    k = 0;
    while (i > 0)
    {
        struct Light *lgt;
        lgt = &game.lish.lights[i];
        i = lgt->next_in_list;
        // Per-light code
        if ((lgt->flags & LgtF_Allocated) != 0)
        {
            long dist;
            dist = get_chessboard_distance(pos, &lgt->mappos);
            if ((dist < 2560) && (nlgt_dist[settings.video_shadows-1] > dist)
                && (pos->x.val != lgt->mappos.x.val) && (pos->y.val != lgt->mappos.y.val))
            {
                add_light_to_nearest_list(nlgt, nlgt_dist, lgt, dist);
            }
        }
        // Per-light code ends
        k++;
        if (k > LIGHTS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping lights list");
            break;
        }
    }
}

static long find_closest_lights(const struct Coord3d* pos, struct NearestLights* nlgt)
{
    long count;
    int32_t nlgt_dist[SHADOW_SOURCES_MAX_COUNT];
    long i;
    for (i = 0; i < SHADOW_SOURCES_MAX_COUNT; i++) {
        nlgt_dist[i] = INT32_MAX;
    }
    i = game.thing_lists[TngList_StaticLights].index;
    find_closest_lights_on_list(nlgt, nlgt_dist, pos, i);
    i = game.thing_lists[TngList_DynamLights].index;
    find_closest_lights_on_list(nlgt, nlgt_dist, pos, i);
    count = 0;
    for (i = 0; i < SHADOW_SOURCES_MAX_COUNT; i++) {
        if (nlgt_dist[i] == INT32_MAX)
            break;
        count++;
    }
    return count;
}

static long find_fade_S(struct EngineCoord *ecor)
{
    if (ecor->render_distance <= fade_min) {
        return ecor->shade_intensity << 8;
    }
    else if (ecor->render_distance >= fade_max) {
        return 32768;
    } else {
        return ecor->shade_intensity * (fade_scaler - ecor->render_distance) / fade_range + 32768;
    }
}

static void create_shadows(struct Thing *thing, struct EngineCoord *ecor, struct Coord3d *pos)
{
    short mv_angle;
    short sh_angle;
    short sprite_angle;
    long dist_sq;
    struct EngineCoord ecor1;
    struct EngineCoord ecor2;
    struct EngineCoord ecor3;
    struct EngineCoord ecor4;

    struct KeeperSprite *spr = keepersprite_array(thing->anim_sprite);

    mv_angle = thing->move_angle_xy;
    sh_angle = get_angle_xy_to(pos, &thing->mappos);
    sprite_angle = (mv_angle - sh_angle) & ANGLE_MASK;
    dist_sq = (get_2d_distance_squared(&thing->mappos, pos) >> 17) + 16;
    if (dist_sq < 16) {
        dist_sq = 16;
    }
    else if (dist_sq > 31) {
        dist_sq = 31;
    }
    short dim_ow;
    short dim_oh;
    short dim_th;
    short dim_tw;
    get_keepsprite_unscaled_dimensions(thing->anim_sprite, sprite_angle, thing->current_frame, &dim_ow, &dim_oh, &dim_tw, &dim_th);
    {
        int sh_angle_sin = LbSinL(sh_angle);
        int sh_angle_cos = LbCosL(sh_angle);

        int base_y2 = 8 * (6 - dim_oh - dim_th + spr->shadow_offset);
        int base_z2 = 8 * dim_tw;
        int base_th = 8 * (dim_th - 4 * dist_sq) + 560;
        int base_tw = 8 * (dim_tw + dim_ow);

        int base_x = ecor->x;
        int base_y = ecor->y;
        int base_z = ecor->z;

        // near and far are measured from origin of thing
        int near_x = base_y2 * sh_angle_sin;
        int near_y = base_y2 * sh_angle_cos;

        int left_x = base_z2 * sh_angle_cos;
        int left_y = base_z2 * sh_angle_sin;
        int far_x = base_th * sh_angle_sin;
        int far_y = base_th * sh_angle_cos;
        int right_x = base_tw * sh_angle_cos;
        int right_y = base_tw * sh_angle_sin;

        // near/left
        ecor1.x = base_x + FROM_FIXED(left_x - near_x);
        ecor1.y = base_y;
        ecor1.z = base_z - FROM_FIXED(left_y + near_y);

        // far/left
        ecor2.x = base_x + FROM_FIXED(left_x - far_x);
        ecor2.y = base_y;
        ecor2.z = base_z - FROM_FIXED(left_y + far_y);

        // far/right
        ecor3.x = base_x + FROM_FIXED(right_x - far_x);
        ecor3.y = base_y;
        ecor3.z = base_z - FROM_FIXED(right_y + far_y);

        // near/right
        ecor4.x = base_x + FROM_FIXED(right_x - near_x);
        ecor4.y = base_y;
        ecor4.z = base_z - FROM_FIXED(right_y + near_y);
    }

    rotpers(&ecor1, &camera_matrix);
    rotpers(&ecor2, &camera_matrix);
    rotpers(&ecor3, &camera_matrix);
    rotpers(&ecor4, &camera_matrix);

    int min_cor_z = min(min(ecor1.z,ecor2.z),min(ecor3.z,ecor4.z));
    struct BucketKindCreatureShadow *kspr = (struct BucketKindCreatureShadow *)get_bucket_item(min_cor_z, QK_CreatureShadow, sizeof(struct BucketKindCreatureShadow));
    if (kspr == NULL)
        return;

    // P1
    kspr->vertex_first.X = ecor1.view_width;
    kspr->vertex_first.Y = ecor1.view_height;
    kspr->vertex_first.U = 0;
    kspr->vertex_first.V = TO_FIXED(dim_oh - 1);
    kspr->vertex_first.S = find_fade_S(&ecor1);

    // P2
    kspr->vertex_second.X = ecor2.view_width;
    kspr->vertex_second.Y = ecor2.view_height;
    kspr->vertex_second.U = 0;
    kspr->vertex_second.V = 0;
    kspr->vertex_second.S = find_fade_S(&ecor2);

    // P3
    kspr->vertex_third.X = ecor3.view_width;
    kspr->vertex_third.Y = ecor3.view_height;
    kspr->vertex_third.U = TO_FIXED(dim_ow - 1);
    kspr->vertex_third.V = 0;
    kspr->vertex_third.S = find_fade_S(&ecor3);

    // P4
    kspr->vertex_fourth.X = ecor4.view_width;
    kspr->vertex_fourth.Y = ecor4.view_height;
    kspr->vertex_fourth.U = TO_FIXED(dim_ow - 1);
    kspr->vertex_fourth.V = TO_FIXED(dim_oh - 1);
    kspr->vertex_fourth.S = find_fade_S(&ecor4);

    // overall
    kspr->vertex_first.S = dist_sq;
    kspr->angle = sprite_angle;
    kspr->anim_sprite = thing->anim_sprite;
    kspr->current_frame = thing->current_frame;
}

// Creature status flower above head in isometric view
static void add_draw_status_box(struct Thing *thing, struct EngineCoord *ecor)
{
    struct EngineCoord coord = *ecor;
    const struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    short offset = thing->clipbox_size_z + crconf->status_offset;
    offset += (offset * game.conf.crtr_conf.exp.size_increase_on_exp * cctrl->exp_level) / 100;
    coord.y += offset;
    rotpers(&coord, &camera_matrix);

    int z_val = coord.z;
    if (!lens_mode)
        z_val = BUCKETS_STEP; // should get into bucket 1

    struct BucketKindCreatureStatus* poly = (struct BucketKindCreatureStatus*)get_bucket_item(z_val, QK_CreatureStatus, sizeof(struct BucketKindCreatureStatus));
    if (poly == NULL)
        return;

    poly->thing = thing;
    poly->x = coord.view_width;
    poly->y = coord.view_height;
    poly->z = coord.z;
}

unsigned short engine_remap_texture_blocks(long stl_x, long stl_y, unsigned short tex_id)
{
    long slb_x = subtile_slab(stl_x);
    long slb_y = subtile_slab(stl_y);
    return tex_id + (game.slab_ext_data[get_slab_number(slb_x,slb_y)] & 0x1F) * TEXTURE_BLOCKS_COUNT;
}

static void do_a_plane_of_engine_columns_perspective(long stl_x, long stl_y, long plane_start, long plane_end)
{
    struct Column *blank_colmn;
    struct Column *colmn;
    struct Map *mapblk;
    struct Map *sib_mapblk;
    struct Column *sib_colmn;
    unsigned short textr_idx;
    unsigned short height_bit;
    SubtlCodedCoords center_block_idx;
    long fepos;
    long bepos;
    long ecpos;
    long clip_start;
    long clip_end;
    struct CubeConfigStats *texturing;
    unsigned short *cubenum_ptr;
    long i;
    long n;
    if ((stl_y <= 0) || (stl_y >= game.map_subtiles_y))
        return;
    clip_start = plane_start;
    if (stl_x + plane_start < 1)
        clip_start = 1 - stl_x;
    clip_end = plane_end;
    if (stl_x + plane_end > game.map_subtiles_x)
        clip_end = game.map_subtiles_x - stl_x;
    struct EngineCol *bec;
    struct EngineCol *fec;
    bec = &back_ec[clip_start + MINMAX_ALMOST_HALF];
    fec = &front_ec[clip_start + MINMAX_ALMOST_HALF];
    blank_colmn = get_column(game.unrevealed_column_idx);
    center_block_idx = clip_start + stl_x + (stl_y * (game.map_subtiles_x+1));
    for (i = clip_end-clip_start; i > 0; i--)
    {
        mapblk = get_map_block_at_pos(center_block_idx);
        colmn = blank_colmn;
        if (map_block_revealed(mapblk, my_player_number))
        {
            n = get_mapwho_thing_index(mapblk);
            if (n != 0)
                do_map_who(n);
            colmn = get_map_column(mapblk);
        }
        // Retrieve solidmasks for surrounding area
        unsigned short solidmsk_center;
        unsigned short solidmsk_top;
        unsigned short solidmsk_bottom;
        unsigned short solidmsk_left;
        unsigned short solidmsk_right;
        solidmsk_center = colmn->solidmask;
        solidmsk_top = blank_colmn->solidmask;
        solidmsk_right = blank_colmn->solidmask;
        solidmsk_bottom = blank_colmn->solidmask;
        solidmsk_left = blank_colmn->solidmask;
        sib_mapblk = get_map_block_at_pos(center_block_idx-game.map_subtiles_x-1);
        if (map_block_revealed(sib_mapblk, my_player_number)) {
            sib_colmn = get_map_column(sib_mapblk);
            solidmsk_top = sib_colmn->solidmask;
        }
        sib_mapblk = get_map_block_at_pos(center_block_idx+game.map_subtiles_x+1);
        if (map_block_revealed(sib_mapblk, my_player_number)) {
            sib_colmn = get_map_column(sib_mapblk);
            solidmsk_bottom = sib_colmn->solidmask;
        }
        sib_mapblk = get_map_block_at_pos(center_block_idx-1);
        if (map_block_revealed(sib_mapblk, my_player_number)) {
            sib_colmn = get_map_column(sib_mapblk);
            solidmsk_left = sib_colmn->solidmask;
        }
        sib_mapblk = get_map_block_at_pos(center_block_idx+1);
        if (map_block_revealed(sib_mapblk, my_player_number)) {
            sib_colmn = get_map_column(sib_mapblk);
            solidmsk_right = sib_colmn->solidmask;
        }
        bepos = 0;
        fepos = 0;
        cubenum_ptr = &colmn->cubes[0];
        height_bit = 1;
        while (height_bit <= solidmsk_center)
        {
            texturing = get_cube_model_stats(*cubenum_ptr);
            if ((solidmsk_center & height_bit) != 0)
            {
              if ((solidmsk_top & height_bit) == 0)
              {

                  textr_idx = engine_remap_texture_blocks(stl_num_decode_x(center_block_idx), stl_num_decode_y(center_block_idx), texturing->texture_id[sideoris[0].back_texture_index]);
                  do_a_trig_gourad_tr(&bec[1].cors[bepos+1], &bec[0].cors[bepos+1], &bec[0].cors[bepos],   textr_idx, normal_shade_back);
                  do_a_trig_gourad_bl(&bec[0].cors[bepos],   &bec[1].cors[bepos],   &bec[1].cors[bepos+1], textr_idx, normal_shade_back);
              }
              if ((solidmsk_bottom & height_bit) == 0)
              {
                  textr_idx = engine_remap_texture_blocks(stl_num_decode_x(center_block_idx), stl_num_decode_y(center_block_idx), texturing->texture_id[sideoris[0].front_texture_index]);
                  do_a_trig_gourad_tr(&fec[0].cors[fepos+1], &fec[1].cors[fepos+1], &fec[1].cors[fepos],   textr_idx, normal_shade_front);
                  do_a_trig_gourad_bl(&fec[1].cors[fepos],   &fec[0].cors[fepos],   &fec[0].cors[fepos+1], textr_idx, normal_shade_front);
              }
              if ((solidmsk_left & height_bit) == 0)
              {
                  textr_idx = engine_remap_texture_blocks(stl_num_decode_x(center_block_idx), stl_num_decode_y(center_block_idx), texturing->texture_id[sideoris[0].bottom_texture_index]);
                  do_a_trig_gourad_tr(&bec[0].cors[bepos+1], &fec[0].cors[fepos+1], &fec[0].cors[fepos],   textr_idx, normal_shade_left);
                  do_a_trig_gourad_bl(&fec[0].cors[fepos],   &bec[0].cors[bepos],   &bec[0].cors[bepos+1], textr_idx, normal_shade_left);
              }
              if ((solidmsk_right & height_bit) == 0)
              {
                  textr_idx = engine_remap_texture_blocks(stl_num_decode_x(center_block_idx), stl_num_decode_y(center_block_idx), texturing->texture_id[sideoris[0].top_texture_index]);
                  do_a_trig_gourad_tr(&fec[1].cors[fepos+1], &bec[1].cors[bepos+1], &bec[1].cors[bepos],   textr_idx, normal_shade_right);
                  do_a_trig_gourad_bl(&bec[1].cors[bepos],   &fec[1].cors[fepos],   &fec[1].cors[fepos+1], textr_idx, normal_shade_right);
              }
            }
            bepos++; fepos++;
            cubenum_ptr++;
            height_bit = height_bit << 1;
        }

        ecpos = floor_height_table[solidmsk_center];
        if (ecpos > 0)
        {
            cubenum_ptr = &colmn->cubes[ecpos-1];
            texturing = get_cube_model_stats(*cubenum_ptr);
            textr_idx = engine_remap_texture_blocks(stl_num_decode_x(center_block_idx), stl_num_decode_y(center_block_idx), texturing->texture_id[4]);
            do_a_trig_gourad_tr(&bec[0].cors[ecpos], &bec[1].cors[ecpos], &fec[1].cors[ecpos], textr_idx, -1);
            do_a_trig_gourad_bl(&fec[1].cors[ecpos], &fec[0].cors[ecpos], &bec[0].cors[ecpos], textr_idx, -1);
        } else
        {
            ecpos = 0;
            textr_idx = engine_remap_texture_blocks(stl_num_decode_x(center_block_idx), stl_num_decode_y(center_block_idx), colmn->floor_texture);
            do_a_trig_gourad_tr(&bec[0].cors[ecpos], &bec[1].cors[ecpos], &fec[1].cors[ecpos], textr_idx, -1);
            do_a_trig_gourad_bl(&fec[1].cors[ecpos], &fec[0].cors[ecpos], &bec[0].cors[ecpos], textr_idx, -1);
        }
        // For tiles which have solid columns at top, draw them
        ecpos = lintel_top_height[solidmsk_center];
        if (ecpos > 0)
        {
            cubenum_ptr = &colmn->cubes[ecpos-1];
            texturing = get_cube_model_stats(*cubenum_ptr);
            textr_idx = engine_remap_texture_blocks(stl_num_decode_x(center_block_idx), stl_num_decode_y(center_block_idx), texturing->texture_id[4]);
            do_a_trig_gourad_tr(&bec[0].cors[ecpos], &bec[1].cors[ecpos], &fec[1].cors[ecpos], textr_idx, -1);
            do_a_trig_gourad_bl(&fec[1].cors[ecpos], &fec[0].cors[ecpos], &bec[0].cors[ecpos], textr_idx, -1);

            ecpos =  lintel_bottom_height[solidmsk_center];
            textr_idx = engine_remap_texture_blocks(stl_num_decode_x(center_block_idx), stl_num_decode_y(center_block_idx), texturing->texture_id[5]);
            do_a_trig_gourad_tr(&fec[0].cors[ecpos], &fec[1].cors[ecpos], &bec[1].cors[ecpos], textr_idx, -1);
            do_a_trig_gourad_bl(&bec[1].cors[ecpos], &bec[0].cors[ecpos], &fec[0].cors[ecpos], textr_idx, -1);
        }
        // Draw the universal ceiling on top of the columns
        ecpos = 8;
        {
            textr_idx = engine_remap_texture_blocks(stl_num_decode_x(center_block_idx), stl_num_decode_y(center_block_idx), floor_to_ceiling_map[colmn->floor_texture]);
            do_a_trig_gourad_tr(&fec[0].cors[ecpos], &fec[1].cors[ecpos], &bec[1].cors[ecpos], textr_idx, -1);
            do_a_trig_gourad_bl(&bec[1].cors[ecpos], &bec[0].cors[ecpos], &fec[0].cors[ecpos], textr_idx, -1);
        }
        bec++;
        fec++;
        center_block_idx++;
    }
}

static void do_a_gpoly_gourad_tr(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short textr_id, int a5)
{
    //BucketKindPolygonStandard in this function could also be BucketKindPolygonSimple or BucketKindBasicUnk10 idk all 3 pretty similar
    int z;
    struct BucketKindPolygonStandard *current_polygon_bucket;
    int bucket_index;
    struct BucketKindPolygonStandard *polygon_bucket_ptr;
    struct BasicQ *previous_bucket_item;
    struct PolyPoint *polypoint1;
    struct PolyPoint *polypoint2;
    struct PolyPoint *polypoint3;
    int ec1_fieldA;
    int ec2_fieldA;
    int ec3_fieldA;

    if ( (ec1->clip_flags & (uint16_t)(ec2->clip_flags & ec3->clip_flags) & 0x1F8) == 0
        && (ec2->view_width - ec1->view_width) * (ec3->view_height - ec2->view_height)
        + (ec1->view_height - ec2->view_height) * (ec3->view_width - ec2->view_width) > 0 )
    {
        z = ec1->z;
        if ( z < ec2->z )
        z = ec2->z;
        if ( z < ec3->z )
        z = ec3->z;
        current_polygon_bucket = (struct BucketKindPolygonStandard *)getpoly;
        bucket_index = z / 16;
        if ( getpoly < poly_pool_end )
        {
            polygon_bucket_ptr = (struct BucketKindPolygonStandard *)getpoly;
            previous_bucket_item = buckets[bucket_index];
            getpoly += sizeof(struct BucketKindPolygonStandard);
            current_polygon_bucket->b.next = previous_bucket_item;
            polypoint1 = &current_polygon_bucket->vertex_first;
            current_polygon_bucket->b.kind = 0;
            buckets[bucket_index] = &current_polygon_bucket->b;
            current_polygon_bucket->block = textr_id;
            ec1_fieldA = ec1->shade_intensity;
            ec2_fieldA = ec2->shade_intensity;
            ec3_fieldA = ec3->shade_intensity;
            if ( a5 >= 0 )
            {
                ec1_fieldA = (4 * ec1_fieldA * (a5 + 0x4000)) >> 17;
                ec2_fieldA = (4 * ec2_fieldA * (a5 + 0x4000)) >> 17;
                ec3_fieldA = (4 * (a5 + 0x4000) * ec3_fieldA) >> 17;
            }
            polypoint1->X = ec1->view_width;
            polypoint1->Y = ec1->view_height;
            polypoint1->U = 0;
            polypoint1->V = 0;
            polypoint1->S = ec1_fieldA << 8;
            polypoint2 = &polygon_bucket_ptr->vertex_second;
            polygon_bucket_ptr->vertex_second.X = ec2->view_width;
            polypoint3 = &polygon_bucket_ptr->vertex_third;
            polypoint2->Y = ec2->view_height;
            polypoint2->U = 0x1FFFFF;
            polypoint2->V = 0;
            polypoint2->S = ec2_fieldA << 8;
            polypoint3->X = ec3->view_width;
            polypoint3->Y = ec3->view_height;
            polypoint3->U = 0x1FFFFF;
            polypoint3->V = 0x1FFFFF;
            polypoint3->S = ec3_fieldA << 8;
        }
    }
}

static void do_a_gpoly_unlit_tr(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short textr_id)
{
    //BucketKindPolygonStandard in this function could also be BucketKindPolygonSimple or BucketKindBasicUnk10 idk all 3 pretty similar
    int z;
    struct BucketKindPolygonStandard *current_polygon_bucket;
    int bucket_index;
    struct BucketKindPolygonStandard *polygon_bucket;
    struct BasicQ *previous_bucket_item;

    if ( (ec1->clip_flags & (uint16_t)(ec2->clip_flags & ec3->clip_flags) & 0x1F8) == 0
        && (ec3->view_width - ec2->view_width) * (ec1->view_height - ec2->view_height)
        + (ec3->view_height - ec2->view_height) * (ec2->view_width - ec1->view_width) > 0 )
    {
        z = ec1->z;
        if ( z < ec2->z )
        z = ec2->z;
        if ( z < ec3->z )
        z = ec3->z;
        current_polygon_bucket = (struct BucketKindPolygonStandard *)getpoly;
        bucket_index = z / 16;
        if ( getpoly < poly_pool_end )
        {
            polygon_bucket = (struct BucketKindPolygonStandard *)getpoly;
            previous_bucket_item = buckets[bucket_index];
            getpoly += sizeof(struct BucketKindPolygonStandard);
            current_polygon_bucket->b.next = previous_bucket_item;
            current_polygon_bucket->b.kind = 0;
            buckets[bucket_index] = &current_polygon_bucket->b;
            current_polygon_bucket->block = textr_id;
            current_polygon_bucket->vertex_first.X = ec1->view_width;
            current_polygon_bucket->vertex_first.Y = ec1->view_height;
            current_polygon_bucket->vertex_first.U = 0;
            current_polygon_bucket->vertex_first.V = 0;
            current_polygon_bucket->vertex_first.S = (ec1->shade_intensity + 3072) << 8;
            current_polygon_bucket->vertex_second.X = ec2->view_width;
            current_polygon_bucket->vertex_second.Y = ec2->view_height;
            current_polygon_bucket->vertex_second.U = 0x1FFFFF;
            current_polygon_bucket->vertex_second.V = 0;
            current_polygon_bucket->vertex_second.S = (ec2->shade_intensity + 3072) << 8;
            polygon_bucket->vertex_third.X = ec3->view_width;
            polygon_bucket->vertex_third.Y = ec3->view_height;
            polygon_bucket->vertex_third.U = 0x1FFFFF;
            polygon_bucket->vertex_third.V = 0x1FFFFF;
            polygon_bucket->vertex_third.S = (ec3->shade_intensity + 3072) << 8;
        }
    }
}

static void do_a_gpoly_unlit_bl(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short textr_id)
{
    //BucketKindPolygonStandard in this function could also be BucketKindPolygonSimple or BucketKindBasicUnk10 idk all 3 pretty similar
    int z;
    struct BucketKindPolygonStandard *current_polygon_bucket;
    int bucket_index;
    struct BasicQ *next_bucket_item;

    if ( (ec1->clip_flags & (uint16_t)(ec2->clip_flags & ec3->clip_flags) & 0x1F8) == 0
        && (ec3->view_width - ec2->view_width) * (ec1->view_height - ec2->view_height)
        + (ec3->view_height - ec2->view_height) * (ec2->view_width - ec1->view_width) > 0 )
    {
        z = ec1->z;
        if ( z < ec2->z )
        z = ec2->z;
        if ( z < ec3->z )
        z = ec3->z;
        current_polygon_bucket = (struct BucketKindPolygonStandard *)getpoly;
        bucket_index = z / 16;
        if ( getpoly < poly_pool_end )
        {
        next_bucket_item = buckets[bucket_index];
        getpoly += sizeof(struct BucketKindPolygonStandard);
        current_polygon_bucket->b.next = next_bucket_item;
        current_polygon_bucket->b.kind = 0;
        buckets[bucket_index] = &current_polygon_bucket->b;
        current_polygon_bucket->block = textr_id;
        current_polygon_bucket->vertex_first.X = ec1->view_width;
        current_polygon_bucket->vertex_first.Y = ec1->view_height;
        current_polygon_bucket->vertex_first.U = 0x1FFFFF;
        current_polygon_bucket->vertex_first.V = 0x1FFFFF;
        current_polygon_bucket->vertex_first.S = (ec1->shade_intensity + 3072) << 8;
        current_polygon_bucket->vertex_second.X = ec2->view_width;
        current_polygon_bucket->vertex_second.Y = ec2->view_height;
        current_polygon_bucket->vertex_second.U = 0;
        current_polygon_bucket->vertex_second.V = 0x1FFFFF;
        current_polygon_bucket->vertex_second.S = (ec2->shade_intensity + 3072) << 8;
        current_polygon_bucket->vertex_third.X = ec3->view_width;
        current_polygon_bucket->vertex_third.Y = ec3->view_height;
        current_polygon_bucket->vertex_third.U = 0;
        current_polygon_bucket->vertex_third.V = 0;
        current_polygon_bucket->vertex_third.S = (ec3->shade_intensity + 3072) << 8;
        }
    }
}

static void do_a_gpoly_gourad_bl(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short textr_id, int a5)
{
    //BucketKindPolygonStandard in this function could also be BucketKindPolygonSimple or BucketKindBasicUnk10 idk all 3 pretty similar
    int z;
    struct BucketKindPolygonStandard *current_polygon_bucket;
    int zdiv16;
    struct BucketKindPolygonStandard *poly_ptr;
    struct BasicQ *previous_bucket_item;
    int ec1_fieldA;
    int ec2_fieldA;
    int ec3_fieldA;
    struct PolyPoint *polypoint2;
    struct PolyPoint *polypoint3;
    struct PolyPoint *polypoint1;

    if ( (ec1->clip_flags & (uint16_t)(ec2->clip_flags & ec3->clip_flags) & 0x1F8) == 0
        && (ec3->view_height - ec2->view_height) * (ec2->view_width - ec1->view_width)
        + (ec1->view_height - ec2->view_height) * (ec3->view_width - ec2->view_width) > 0 )
    {
        z = ec1->z;
        if ( z < ec2->z )
        z = ec2->z;
        if ( z < ec3->z )
        z = ec3->z;
        current_polygon_bucket = (struct BucketKindPolygonStandard *)getpoly;
        zdiv16 = z / 16;
        if ( getpoly < poly_pool_end )
        {
            poly_ptr = (struct BucketKindPolygonStandard *)getpoly;
            previous_bucket_item = buckets[zdiv16];
            getpoly += sizeof(struct BucketKindPolygonStandard);
            current_polygon_bucket->b.next = previous_bucket_item;
            polypoint1 = &current_polygon_bucket->vertex_first;
            current_polygon_bucket->b.kind = 0;
            buckets[zdiv16] = &current_polygon_bucket->b;
            current_polygon_bucket->block = textr_id;
            ec1_fieldA = ec1->shade_intensity;
            ec2_fieldA = ec2->shade_intensity;
            ec3_fieldA = ec3->shade_intensity;
            if ( a5 >= 0 )
            {
                ec1_fieldA = (4 * (a5 + 0x4000) * ec1_fieldA) >> 17;
                ec2_fieldA = (4 * (a5 + 0x4000) * ec2_fieldA) >> 17;
                ec3_fieldA = (4 * (a5 + 0x4000) * ec3_fieldA) >> 17;
            }
            polypoint1->X = ec1->view_width;
            polypoint2 = &poly_ptr->vertex_second;
            polypoint1->Y = ec1->view_height;
            polypoint1->U = 0x1FFFFF;
            polypoint1->V = 0x1FFFFF;
            polypoint1->S = ec1_fieldA << 8;
            poly_ptr->vertex_second.X = ec2->view_width;
            polypoint3 = &poly_ptr->vertex_third;
            polypoint2->Y = ec2->view_height;
            polypoint2->U = 0;
            polypoint2->V = 0x1FFFFF;
            polypoint2->S = ec2_fieldA << 8;
            polypoint3->X = ec3->view_width;
            polypoint3->Y = ec3->view_height;
            polypoint3->U = 0;
            polypoint3->V = 0;
            polypoint3->S = ec3_fieldA << 8;
        }
    }
}

static void do_a_plane_of_engine_columns_cluedo(long stl_x, long stl_y, long plane_start, long plane_end)
{
    if ((stl_y < 1) || (stl_y > (game.map_subtiles_y - 1))) {
        return;
    }
    long xaval;
    long xbval;
    xaval = plane_start;
    if (stl_x + plane_start < 1) {
        xaval = 1 - stl_x;
    }
    xbval = plane_end;
    if (stl_x + plane_end > game.map_subtiles_x) {
        xbval = game.map_subtiles_x - stl_x;
    }
    int xidx;
    int xdelta;
    xdelta = xbval - xaval;
    const struct Column *unrev_colmn;
    unrev_colmn = get_column(game.unrevealed_column_idx);
    for (xidx=0; xidx < xdelta; xidx++)
    {
        struct Map *cur_mapblk;
        cur_mapblk = get_map_block_at(stl_x + xaval + xidx, stl_y);
        // Get solidmasks of sibling columns
        unsigned short solidmsk_cur_raw;
        unsigned short solidmsk_cur;
        unsigned short solidmsk_back;
        unsigned short solidmsk_front;
        unsigned short solidmsk_left;
        unsigned short solidmsk_right;
        solidmsk_cur_raw = unrev_colmn->solidmask;
        solidmsk_cur = unrev_colmn->solidmask & 3;
        solidmsk_back = unrev_colmn->solidmask & 3;
        solidmsk_right = unrev_colmn->solidmask & 3;
        solidmsk_front = unrev_colmn->solidmask & 3;
        solidmsk_left = unrev_colmn->solidmask & 3;
        // Get column to be drawn
        const struct Column *cur_colmn;
        cur_colmn = unrev_colmn;
        if (map_block_revealed(cur_mapblk, my_player_number))
        {
            long i;
            i = get_mapwho_thing_index(cur_mapblk);
            if (i > 0) {
              do_map_who(i);
            }
            cur_colmn = get_map_column(cur_mapblk);
            solidmsk_cur_raw = cur_colmn->solidmask;
            solidmsk_cur = solidmsk_cur_raw;
            if (solidmsk_cur >= (1<<3))
            {
                if (((cur_mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom)) == 0) && ((cur_colmn->bitfields & 0xE) == 0)) {
                    solidmsk_cur &= 3;
                }
            }
        }
        struct Map *sib_mapblk;
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx, stl_y - 1);
        if (map_block_revealed(sib_mapblk, my_player_number)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_back = colmn->solidmask;
            if (solidmsk_back >= (1<<3))
            {
                if (((sib_mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom)) == 0) && ((colmn->bitfields & 0xE) == 0)) {
                    solidmsk_back &= 3;
                }
            }
        }
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx, stl_y + 1);
        if (map_block_revealed(sib_mapblk, my_player_number)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_front = colmn->solidmask;
            if (solidmsk_front >= (1<<3))
            {
                if (((sib_mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom)) == 0) && ((colmn->bitfields & 0xE) == 0)) {
                    solidmsk_front &= 3;
                }
            }
        }
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx - 1, stl_y);
        if (map_block_revealed(sib_mapblk, my_player_number)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_left = colmn->solidmask;
            if (solidmsk_left >= (1<<3))
            {
                if (((sib_mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom)) == 0) && ((colmn->bitfields & 0xE) == 0)) {
                    solidmsk_left &= 3;
                }
            }
        }
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx + 1, stl_y);
        if (map_block_revealed(sib_mapblk, my_player_number)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_right = colmn->solidmask;
            if (solidmsk_right >= (1<<3))
            {
                if (((sib_mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom)) == 0) && ((colmn->bitfields & 0xE) == 0)) {
                    solidmsk_right &= 3;
                }
            }
        }

        struct EngineCol *bec;
        struct EngineCol *fec;
        bec = &back_ec[xaval + MINMAX_ALMOST_HALF + xidx];
        fec = &front_ec[xaval + MINMAX_ALMOST_HALF + xidx];
        unsigned short mask;
        int ncor;
        for (mask=1,ncor=0; mask <= solidmsk_cur; mask*=2,ncor++)
        {
            unsigned short textr_id;
            struct CubeConfigStats *cubed;
            cubed = get_cube_model_stats(cur_colmn->cubes[ncor]);
            if ((mask & solidmsk_cur) == 0)
            {
                continue;
            }
            if ((mask & solidmsk_back) == 0)
            {
                textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, cubed->texture_id[sideoris[0].back_texture_index]);
                do_a_gpoly_gourad_tr(&bec[1].cors[ncor+1], &bec[0].cors[ncor+1], &bec[0].cors[ncor],   textr_id, normal_shade_back);
                do_a_gpoly_gourad_bl(&bec[0].cors[ncor],   &bec[1].cors[ncor],   &bec[1].cors[ncor+1], textr_id, normal_shade_back);
            }
            if ((solidmsk_front & mask) == 0)
            {
                textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, cubed->texture_id[sideoris[0].front_texture_index]);
                do_a_gpoly_gourad_tr(&fec[0].cors[ncor+1], &fec[1].cors[ncor+1], &fec[1].cors[ncor],   textr_id, normal_shade_front);
                do_a_gpoly_gourad_bl(&fec[1].cors[ncor],   &fec[0].cors[ncor],   &fec[0].cors[ncor+1], textr_id, normal_shade_front);
            }
            if ((solidmsk_left & mask) == 0)
            {
                textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, cubed->texture_id[sideoris[0].bottom_texture_index]);
                do_a_gpoly_gourad_tr(&bec[0].cors[ncor+1], &fec[0].cors[ncor+1], &fec[0].cors[ncor],   textr_id, normal_shade_left);
                do_a_gpoly_gourad_bl(&fec[0].cors[ncor],   &bec[0].cors[ncor],   &bec[0].cors[ncor+1], textr_id, normal_shade_left);
            }
            if ((solidmsk_right & mask) == 0)
            {
                textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, cubed->texture_id[sideoris[0].top_texture_index]);
                do_a_gpoly_gourad_tr(&fec[1].cors[ncor+1], &bec[1].cors[ncor+1], &bec[1].cors[ncor],   textr_id, normal_shade_right);
                do_a_gpoly_gourad_bl(&bec[1].cors[ncor],   &fec[1].cors[ncor],   &fec[1].cors[ncor+1], textr_id, normal_shade_right);
            }
        }

        ncor = floor_height_table[solidmsk_cur];
        if ((ncor > 0) && (ncor <= COLUMN_STACK_HEIGHT))
        {
            int ncor_raw;
            ncor_raw = floor_height_table[solidmsk_cur_raw];
            if ( (cur_mapblk->flags & SlbAtFlg_Unexplored) != 0 )
            {
                unsigned short textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, TEXTURE_LAND_MARKED_LAND);
                do_a_gpoly_unlit_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], textr_id);
                do_a_gpoly_unlit_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], textr_id);
            } else
            if ((cur_mapblk->flags & SlbAtFlg_TaggedValuable) != 0)
            {
                unsigned short textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, TEXTURE_LAND_MARKED_GOLD);
                do_a_gpoly_unlit_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], textr_id);
                do_a_gpoly_unlit_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], textr_id);
            } else
             {
                if ((ncor_raw > 0) && (ncor_raw <= COLUMN_STACK_HEIGHT))
                {
                    struct CubeConfigStats * cubed = get_cube_model_stats(cur_colmn->cubes[ncor_raw-1]);
                    unsigned short textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, cubed->texture_id[4]);
                    // Top surface in cluedo mode
                    do_a_gpoly_gourad_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], textr_id, -1);
                    do_a_gpoly_gourad_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], textr_id, -1);
                }
            }
        } else
        {
            if ((cur_mapblk->flags & SlbAtFlg_Unexplored) == 0)
            {
                unsigned short textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, cur_colmn->floor_texture);
                do_a_gpoly_gourad_tr(&bec[0].cors[0], &bec[1].cors[0], &fec[1].cors[0], textr_id, -1);
                do_a_gpoly_gourad_bl(&fec[1].cors[0], &fec[0].cors[0], &bec[0].cors[0], textr_id, -1);
            } else
            {
                unsigned short textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, TEXTURE_LAND_MARKED_LAND);
                do_a_gpoly_unlit_tr(&bec[0].cors[0], &bec[1].cors[0], &fec[1].cors[0], textr_id);
                do_a_gpoly_unlit_bl(&fec[1].cors[0], &fec[0].cors[0], &bec[0].cors[0], textr_id);
            }
        }
        ncor = lintel_top_height[solidmsk_cur];
        if ((ncor > 0) && (ncor <= COLUMN_STACK_HEIGHT))
        {
            struct CubeConfigStats * cubed;
            cubed = get_cube_model_stats(cur_colmn->cubes[ncor-1]);
            unsigned short textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, cubed->texture_id[4]);
            do_a_gpoly_gourad_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], textr_id, -1);
            do_a_gpoly_gourad_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], textr_id, -1);
        }
    }
}

static void do_a_plane_of_engine_columns_isometric(long stl_x, long stl_y, long plane_start, long plane_end)
{
    if ((stl_y < 1) || (stl_y > game.map_subtiles_y - 1)) {
        return;
    }

    long xaval;
    long xbval;
    TbBool xaclip;
    TbBool xbclip;
    xaval = plane_start;
    xaclip = 0;
    xbclip = 0;
    if (stl_x + plane_start < 1) {
        xaclip = 1;
        xaval = 1 - stl_x;
    }
    xbval = plane_end;
    if (stl_x + plane_end > game.map_subtiles_x) {
        xbclip = 1;
        xbval = game.map_subtiles_x - stl_x;
    }
    int xidx;
    int xdelta;
    xdelta = xbval - xaval;
    const struct Column *unrev_colmn;
    unrev_colmn = get_column(game.unrevealed_column_idx);
    for (xidx=0; xidx < xdelta; xidx++)
    {
        struct Map *cur_mapblk;
        cur_mapblk = get_map_block_at(stl_x + xaval + xidx, stl_y);
        // Get column to be drawn
        const struct Column *cur_colmn;
        cur_colmn = unrev_colmn;
        if (map_block_revealed(cur_mapblk, my_player_number))
        {
            long i;
            i = get_mapwho_thing_index(cur_mapblk);
            if (i > 0) {
              do_map_who(i);
            }
            cur_colmn = get_map_column(cur_mapblk);
        }
        // Get solidmasks of sibling columns
        unsigned short solidmsk_cur;
        unsigned short solidmsk_back;
        unsigned short solidmsk_front;
        unsigned short solidmsk_left;
        unsigned short solidmsk_right;
        solidmsk_cur = cur_colmn->solidmask;
        solidmsk_back = unrev_colmn->solidmask;
        solidmsk_right = unrev_colmn->solidmask;
        solidmsk_front = unrev_colmn->solidmask;
        solidmsk_left = unrev_colmn->solidmask;
        struct Map *sib_mapblk;
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx, stl_y - 1);
        if (map_block_revealed(sib_mapblk, my_player_number)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_back = colmn->solidmask;
        }
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx, stl_y + 1);
        if (map_block_revealed(sib_mapblk, my_player_number)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_front = colmn->solidmask;
        }
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx - 1, stl_y);
        if (map_block_revealed(sib_mapblk, my_player_number)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_left = colmn->solidmask;
        }
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx + 1, stl_y);
        if (map_block_revealed(sib_mapblk, my_player_number)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_right = colmn->solidmask;
        }
        if ( xaclip || xbclip || (stl_y <= 1) || (stl_y >= game.map_subtiles_y - 1))
        {
            if (xaclip && (xidx == 0)) {
                solidmsk_left = 0;
            }
            if (xbclip && (xdelta - xidx == 1)) {
                solidmsk_right = 0;
            }
            if (stl_y <= 1) {
                solidmsk_back = 0;
            }
            if (stl_y >= game.map_subtiles_y - 1) {
                solidmsk_front = 0;
            }
        }

        struct EngineCol *bec;
        struct EngineCol *fec;
        bec = &back_ec[xaval + MINMAX_ALMOST_HALF + xidx];
        fec = &front_ec[xaval + MINMAX_ALMOST_HALF + xidx];
        unsigned short mask;
        int ncor;
        for (mask=1,ncor=0; mask <= solidmsk_cur; mask*=2,ncor++)
        {
            unsigned short textr_id;
            struct CubeConfigStats *cubed;
            cubed = get_cube_model_stats(cur_colmn->cubes[ncor]);
            if ((mask & solidmsk_cur) == 0)
            {
                continue;
            }
            if ((mask & solidmsk_back) == 0)
            {
                textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, cubed->texture_id[sideoris[0].back_texture_index]);
                do_a_gpoly_gourad_tr(&bec[1].cors[ncor+1], &bec[0].cors[ncor+1], &bec[0].cors[ncor],   textr_id, normal_shade_back);
                do_a_gpoly_gourad_bl(&bec[0].cors[ncor],   &bec[1].cors[ncor],   &bec[1].cors[ncor+1], textr_id, normal_shade_back);
            }
            if ((solidmsk_front & mask) == 0)
            {
                textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, cubed->texture_id[sideoris[0].front_texture_index]);
                do_a_gpoly_gourad_tr(&fec[0].cors[ncor+1], &fec[1].cors[ncor+1], &fec[1].cors[ncor],   textr_id, normal_shade_front);
                do_a_gpoly_gourad_bl(&fec[1].cors[ncor],   &fec[0].cors[ncor],   &fec[0].cors[ncor+1], textr_id, normal_shade_front);
            }
            if ((solidmsk_left & mask) == 0)
            {
                textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, cubed->texture_id[sideoris[0].bottom_texture_index]);
                do_a_gpoly_gourad_tr(&bec[0].cors[ncor+1], &fec[0].cors[ncor+1], &fec[0].cors[ncor],   textr_id, normal_shade_left);
                do_a_gpoly_gourad_bl(&fec[0].cors[ncor],   &bec[0].cors[ncor],   &bec[0].cors[ncor+1], textr_id, normal_shade_left);
            }
            if ((solidmsk_right & mask) == 0)
            {
                textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, cubed->texture_id[sideoris[0].top_texture_index]);
                do_a_gpoly_gourad_tr(&fec[1].cors[ncor+1], &bec[1].cors[ncor+1], &bec[1].cors[ncor],   textr_id, normal_shade_right);
                do_a_gpoly_gourad_bl(&bec[1].cors[ncor],   &fec[1].cors[ncor],   &fec[1].cors[ncor+1], textr_id, normal_shade_right);
            }
        }

        ncor = floor_height_table[solidmsk_cur];
        if (ncor > 0)
        {
            if (cur_mapblk->flags & SlbAtFlg_Unexplored)
            {
                unsigned short textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, TEXTURE_LAND_MARKED_LAND);
                do_a_gpoly_unlit_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], textr_id);
                do_a_gpoly_unlit_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], textr_id);
            }
            else if ((cur_mapblk->flags & (SlbAtFlg_TaggedValuable|SlbAtFlg_Unexplored)) == 0)
            {
                struct CubeConfigStats * cubed;
                cubed = get_cube_model_stats(*(short *)((char *)&cur_colmn->floor_texture + 2 * ncor + 1));
                unsigned short textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, cubed->texture_id[4]);
                // Top surface on full iso mode
                do_a_gpoly_gourad_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], textr_id, -1);
                do_a_gpoly_gourad_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], textr_id, -1);
            } else
            if ((cur_mapblk->flags & SlbAtFlg_Valuable) != 0)
            {
                unsigned short textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, TEXTURE_LAND_MARKED_GOLD);
                do_a_gpoly_unlit_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], textr_id);
                do_a_gpoly_unlit_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], textr_id);
            }
        } else
        {
            if ((cur_mapblk->flags & SlbAtFlg_Unexplored) == 0)
            {
                unsigned short textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, cur_colmn->floor_texture);
                do_a_gpoly_gourad_tr(&bec[0].cors[0], &bec[1].cors[0], &fec[1].cors[0], textr_id, -1);
                do_a_gpoly_gourad_bl(&fec[1].cors[0], &fec[0].cors[0], &bec[0].cors[0], textr_id, -1);
            } else
            {
                unsigned short textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, TEXTURE_LAND_MARKED_LAND);
                do_a_gpoly_unlit_tr(&bec[0].cors[0], &bec[1].cors[0], &fec[1].cors[0], textr_id);
                do_a_gpoly_unlit_bl(&fec[1].cors[0], &fec[0].cors[0], &bec[0].cors[0], textr_id);
            }
        }
        ncor = lintel_top_height[solidmsk_cur];
        if (ncor > 0)
        {
            struct CubeConfigStats * cubed;
            cubed = get_cube_model_stats(*(short *)((char *)&cur_colmn->floor_texture + 2 * ncor + 1));
            unsigned short textr_id = engine_remap_texture_blocks(stl_x + xaval + xidx, stl_y, cubed->texture_id[4]);
            do_a_gpoly_gourad_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], textr_id, -1);
            do_a_gpoly_gourad_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], textr_id, -1);
        }
    }
}

void draw_map_volume_box(long cor1_x, long cor1_y, long cor2_x, long cor2_y, long floor_height_z, unsigned char color)
{
    map_volume_box.visible = 1;
    map_volume_box.beg_x = cor1_x & 0xFFFFFF00;
    map_volume_box.beg_y = cor1_y & 0xFFFF00;
    map_volume_box.end_x = cor2_x & 0xFFFFFF00;
    map_volume_box.end_y = cor2_y & 0xFFFFFF00;
    map_volume_box.floor_height_z = floor_height_z;
    map_volume_box.color = color;
}

/**
 * Some objects have a secondary sprite drawn on top, that is positioned relative to the original sprite and scaled differently.
 * @param jspr the base sprite
 * @param angle the camera angle at which sprite is shown
 * @param base_sprite_size the size of the sprite on the screen after camera zoom
 * @note Renders both the primary and secondary sprite. So both the torch and the flame.
  */
static void process_keeper_flame_on_sprite(struct BucketKindJontySprite* jspr, long angle, long base_sprite_size)
{
    struct PlayerInfo* player = get_my_player();
    struct Thing* thing = jspr->thing;
    struct ObjectConfigStats* objst;
    struct TrapConfigStats* trapst;
    struct FlameProperties flame;
    unsigned long nframe;
    long add_x, add_y;
    long scale = 0;
    if (thing_is_object(thing))
    {
        objst = get_object_model_stats(thing->model);
        flame = objst->flame;
    } else
    if (thing_is_deployed_trap(thing))
    {
        trapst = get_trap_model_stats(thing->model);
        flame = trapst->flame;
    }
    else
    {
        ERRORLOG("Thing %s is neither an object nor a flame.", thing_model_name(thing));
        return;
    }
    if (thing->sprite_size != 0)
    {
        scale = (flame.sprite_size * base_sprite_size / thing->sprite_size);
    }

    if (player->view_type == PVT_DungeonTop)
    {
        add_x = (base_sprite_size * flame.td_add_x) >> 5;
        add_y = (base_sprite_size * flame.td_add_y) >> 5;
    }
    else
    {
        add_x = (base_sprite_size * flame.fp_add_x) >> 5;
        add_y = (base_sprite_size * flame.fp_add_y) >> 5;
    }

    //Object/Trap itself
    clear_flag(lbDisplay.DrawFlags, TRF_Transpar_Flags);
    EngineSpriteDrawUsingAlpha = 0;
    if (flag_is_set(thing->rendering_flags,TRF_Transpar_8))
        lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR8;
    if (flag_is_set(thing->rendering_flags, TRF_Transpar_4))
        lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
    if (flag_is_set(thing->rendering_flags, TRF_Transpar_Alpha))
        EngineSpriteDrawUsingAlpha = 1;
    process_keeper_sprite(jspr->scr_x, jspr->scr_y, thing->anim_sprite, angle, thing->current_frame, base_sprite_size);

    //Flame
    lbDisplay.DrawFlags = 0;
    EngineSpriteDrawUsingAlpha = 0;
    if (flame.transparency_flags == TRF_Transpar_8)
    {
        lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR8;
    }
    else if (flame.transparency_flags == TRF_Transpar_4)
    {
        lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
    }
    else if (flame.transparency_flags == TRF_Transpar_Alpha)
    {
        EngineSpriteDrawUsingAlpha = 1;
    }
    nframe = (thing->index + game.play_gameturn * flame.anim_speed / 256) % keepersprite_frames(flame.animation_id);
    process_keeper_sprite(jspr->scr_x + add_x, jspr->scr_y + add_y, flame.animation_id, angle, nframe, scale);
}

static unsigned short get_thing_shade(struct Thing* thing);
static void draw_fastview_mapwho(struct Camera *cam, struct BucketKindJontySprite *jspr)
{
    unsigned short flg_mem;
    unsigned char alpha_mem;
    struct PlayerInfo *player = get_my_player();
    struct ObjectConfigStats* objst;
    struct Thing *thing = jspr->thing;
    short angle;
    flg_mem = lbDisplay.DrawFlags;
    alpha_mem = EngineSpriteDrawUsingAlpha;
    if (keepersprite_rotable(thing->anim_sprite))
    {
        angle = thing->move_angle_xy - cam->rotation_angle_x; // rotation_angle_x maybe short
    }
    else
    {
        angle = thing->move_angle_xy;
    }

    switch(thing->rendering_flags & TRF_Transpar_Alpha)
    {
        case TRF_Transpar_8:
            lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR8;
            break;
        case TRF_Transpar_4:
            lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
            break;
        default:
            break;
    }
    unsigned short shade_intensity = 0x2000;
    if ( !(thing->rendering_flags & TRF_Unshaded) )
        shade_intensity = get_thing_shade(thing);
    shade_intensity >>= 8;

    int size_on_screen = thing->sprite_size * ((camera_zoom << 13) / 0x10000 / pixel_size) / 0x10000;
    if ( thing->rendering_flags & TRF_Tint_Flags )
    {
        lbDisplay.DrawFlags |= Lb_TEXT_UNDERLNSHADOW;
        lbSpriteReMapPtr = &pixmap.ghost[256 * thing->tint_colour];
    }
    else if ( shade_intensity == 0x2000 )
    {
        lbDisplay.DrawFlags &= ~Lb_TEXT_UNDERLNSHADOW;
    }
    else
    {
        lbDisplay.DrawFlags |= Lb_TEXT_UNDERLNSHADOW;
        lbSpriteReMapPtr = &pixmap.fade_tables[shade_intensity << 8];
    }

    EngineSpriteDrawUsingAlpha = 0;
    switch (thing->rendering_flags & (TRF_Transpar_Flags))
    {
        case TRF_Transpar_8:
            lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR8;
            lbDisplay.DrawFlags &= ~Lb_TEXT_UNDERLNSHADOW;
            break;
        case TRF_Transpar_4:
            lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
            lbDisplay.DrawFlags &= ~Lb_TEXT_UNDERLNSHADOW;
            break;
        case TRF_Transpar_Alpha:
            EngineSpriteDrawUsingAlpha = 1;
            break;
    }

    if ((thing->class_id == TCls_Creature)
        || (thing->class_id == TCls_Object)
        || (thing->class_id == TCls_DeadCreature)
        || (player->work_state == PSt_QueryAll))
    {
        if ((player->thing_under_hand == thing->index) && ((game.play_gameturn % (4 * gui_blink_rate)) >= 2 * gui_blink_rate))
        {
            lbDisplay.DrawFlags |= Lb_TEXT_UNDERLNSHADOW;
            lbSpriteReMapPtr = white_pal;
        } else {
            if ((thing->rendering_flags & TRF_BeingHit) != 0)
            {
                lbDisplay.DrawFlags |= Lb_TEXT_UNDERLNSHADOW;
                lbSpriteReMapPtr = red_pal;
                thing->time_spent_displaying_hurt_colour += game.delta_time;
                if (thing->time_spent_displaying_hurt_colour >= 1.0 || game.frame_skip > 0)
                {
                    thing->time_spent_displaying_hurt_colour = 0;
                    thing->rendering_flags &= ~TRF_BeingHit; // Turns off red damage colour tint
                }
            }
        }
        thing_being_displayed_is_creature = 1;
        thing_being_displayed = thing;
    } else
    {
        thing_being_displayed_is_creature = 0;
        thing_being_displayed = NULL;
    }

    if (
            ((thing->anim_sprite >= CREATURE_FRAMELIST_LENGTH) && (thing->anim_sprite < KEEPERSPRITE_ADD_OFFSET))
            || (thing->anim_sprite >= KEEPERSPRITE_ADD_OFFSET + KEEPERSPRITE_ADD_NUM)
            )
    {
        ERRORLOG("Invalid graphic Id %d from model %d, class %d", (int)thing->anim_sprite, (int)thing->model, (int)thing->class_id);
        lbDisplay.DrawFlags = flg_mem;
        EngineSpriteDrawUsingAlpha = alpha_mem;
        return;
    }
    TbBool flame_on_sprite = false;
    TbBool is_shown = true;
    if (thing_is_object(thing))
    {
        objst = get_object_model_stats(thing->model);
        if (objst->flame.animation_id != 0)
        {
            flame_on_sprite = true;
            process_keeper_flame_on_sprite(jspr, angle, size_on_screen);
        }
    }
    else
    {
        if (thing->class_id == TCls_Trap)
        {
            is_shown = !game.conf.trapdoor_conf.trap_cfgstats[thing->model].hidden;
            if (is_shown || thing->trap.revealed)
            {
                struct TrapConfigStats* trapst = get_trap_model_stats(thing->model);
                if ((trapst->flame.animation_id != 0) && (thing->trap.num_shots != 0))
                {
                    flame_on_sprite = true;
                    process_keeper_flame_on_sprite(jspr, angle, size_on_screen);
                }
            }
        }
        else
        {
            is_shown = ((thing->rendering_flags & TRF_Invisible) == 0);
        }
    }
    if (!flame_on_sprite)
    {
        if (is_shown || get_my_player()->id_number == thing->owner || thing->trap.revealed)
        {
            process_keeper_sprite(jspr->scr_x, jspr->scr_y, thing->anim_sprite, angle, thing->current_frame, size_on_screen);
        }
    }
    lbDisplay.DrawFlags = flg_mem;
    EngineSpriteDrawUsingAlpha = alpha_mem;
}

static void draw_engine_number(struct BucketKindFloatingGoldText *num)
{
    struct PlayerInfo *player;
    unsigned short flg_mem;
    const struct TbSprite *spr;
    long remaining_digits;
    long ndigits;
    long w;
    long h;
    long pos_x;

    // 1st argument: the scale when fully zoomed out. 2nd argument: the scale at base level zoom
    float scale_by_zoom = LbLerp(0.15, 1.00, hud_scale);

    flg_mem = lbDisplay.DrawFlags;
    player = get_my_player();
    lbDisplay.DrawFlags &= ~Lb_SPRITE_FLIP_HORIZ;
    spr = get_button_sprite(GBS_fontchars_number_dig0);
    w = scale_ui_value(spr->SWidth) * scale_by_zoom;
    h = scale_ui_value(spr->SHeight) * scale_by_zoom;
    if (
        player->acamera->view_mode == PVM_IsoWibbleView ||
        player->acamera->view_mode == PVM_FrontView ||
        player->acamera->view_mode == PVM_IsoStraightView
    ) {
        // Count digits to be displayed
        ndigits=0;
        for (remaining_digits = num->lvl; remaining_digits > 0; remaining_digits /= 10)
            ndigits++;
        if (ndigits > 0)
        {
            // Show the digits
            pos_x = w*(ndigits-1)/2 + num->x;
            for (remaining_digits = num->lvl; remaining_digits > 0; remaining_digits /= 10)
            {
                spr = get_button_sprite((remaining_digits%10) + GBS_fontchars_number_dig0);
                LbSpriteDrawScaled(pos_x, num->y - h, spr, w, h);

                pos_x -= w;
            }
        }
    }
    lbDisplay.DrawFlags = flg_mem;
}

static void draw_engine_room_flagpole(struct BucketKindRoomFlag *rflg)
{
    lbDisplay.DrawFlags &= ~Lb_SPRITE_FLIP_HORIZ;

    struct Room *room = room_get(rflg->lvl);
    if (!room_exists(room) || !room_can_have_ensign(room->kind)) {
        return;
    }
    struct PlayerInfo *player = get_my_player();
    const struct Camera *cam = get_local_camera(player->acamera);

    if (
        cam->view_mode == PVM_IsoWibbleView ||
        cam->view_mode == PVM_FrontView ||
        cam->view_mode == PVM_IsoStraightView
    ) {
        if (settings.roomflags_on)
        {
            int deltay, height, zoom_factor;
            // 1st argument: the scale when fully zoomed out. 2nd argument: the scale at base level zoom
            float scale_by_zoom = LbLerp(0.15, 1.00, hud_scale);

            if (cam->view_mode == PVM_FrontView) {
                zoom_factor = 4094*scale_by_zoom;
                deltay = (zoom_factor << 7 >> 13) * units_per_pixel_ui / 16;
                height = ((2 * (71 * zoom_factor) >> 13) * units_per_pixel_ui + 8) / 16;
            } else {
                zoom_factor = camera_zoom;
                deltay = (zoom_factor << 7 >> 13);
                height = (2 * (71 * zoom_factor) >> 13) + 8;
            }

            LbDrawBox(rflg->x,
                      rflg->y - deltay,
                      ((4*scale_by_zoom) * units_per_pixel_ui + 8) / 16,
                      height,
                      colours[3][1][0]);
            LbDrawBox(rflg->x + (2*scale_by_zoom) * (units_per_pixel_ui) / 16,
                      rflg->y - deltay,
                      ((2*scale_by_zoom) * units_per_pixel_ui + 8) / 16,
                      height,
                      colours[1][0][0]);
        }
    }
}

/**
 * Selects index of a sprite used to show creature health flower.
 * @param thing
 */
static unsigned short choose_health_sprite(struct Thing* thing)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    HitPoints health;
    HitPoints maxhealth;
    health = thing->health;
    maxhealth = cctrl->max_health;

    if ((maxhealth <= 0) || (health <= 0))
    {
        return get_player_colored_button_sprite_idx(GBS_creature_flower_health_r1,thing->owner);
    } else
    if (health >= maxhealth)
    {
        return get_player_colored_button_sprite_idx(GBS_creature_flower_health_r1,thing->owner) - 7;
    } else
    {
        return get_player_colored_button_sprite_idx(GBS_creature_flower_health_r1,thing->owner) - (8 * health / maxhealth);
    }
}

void fill_status_sprite_indexes(struct Thing *thing, struct CreatureControl *cctrl, short *health_spridx,
                                short *state_spridx, short *anger_spridx)
{
    (*health_spridx) = choose_health_sprite(thing);
    if (is_my_player_number(thing->owner))
    {
        lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
        if (game.play_gameturn - cctrl->thought_bubble_last_turn_drawn == 1)
        {
            if (cctrl->thought_bubble_display_timer < 40) {
                cctrl->thought_bubble_display_timer++;
            }
        } else {
            if (game.play_gameturn - cctrl->thought_bubble_last_turn_drawn > 1) {
                cctrl->thought_bubble_display_timer = 0;
            }
        }
        cctrl->thought_bubble_last_turn_drawn = game.play_gameturn;
        if (cctrl->thought_bubble_display_timer >= 40)
        {
            struct CreatureStateConfig *stati;
            stati = get_creature_state_with_task_completion(thing);
            if (!stati->blocks_all_state_changes)
            {
                if (creature_under_spell_effect(thing, CSAfF_MadKilling))
                {
                    stati = &game.conf.crtr_conf.states[CrSt_MadKillingPsycho];
                }
                else if (anger_is_creature_livid(thing))
                {
                    stati = &game.conf.crtr_conf.states[CrSt_CreatureLeavingDungeon];
                }
                else if (creature_is_called_to_arms(thing))
                {
                    stati = &game.conf.crtr_conf.states[CrSt_ArriveAtCallToArms];
                }
                else if (creature_is_at_alarm(thing))
                {
                    stati = &game.conf.crtr_conf.states[CrSt_ArriveAtAlarm];
                }
                else if (anger_is_creature_angry(thing))
                {
                    stati = &game.conf.crtr_conf.states[CrSt_PersonSulkAtLair];
                }
                else if (hunger_is_creature_hungry(thing))
                {
                    stati = &game.conf.crtr_conf.states[CrSt_CreatureArrivedAtGarden];
                }
                else if (creature_requires_healing(thing))
                {
                    stati = &game.conf.crtr_conf.states[CrSt_CreatureSleep];
                }
                else if (cctrl->paydays_owed)
                {
                    stati = &game.conf.crtr_conf.states[CrSt_CreatureWantsSalary];
                }
                else
                {
                    stati = get_creature_state_with_task_completion(thing);
                }
                if ((*(short *)&stati->display_thought_bubble == 1) || (thing_pointed_at == thing))
                {
                    (*state_spridx) = stati->sprite_idx;
                }
                switch (anger_get_creature_anger_type(thing))
                {
                case AngR_NotPaid:
                    if ((cctrl->paydays_owed <= 0) && (cctrl->paydays_advanced >= 0))
                    {
                        (*anger_spridx) = GBS_creature_states_angry;
                    }
                    else
                    {
                        (*anger_spridx) = GBS_creature_states_getgold;
                    }
                    break;
                case AngR_Hungry:
                    (*anger_spridx) = GBS_creature_states_hungry;
                    break;
                case AngR_NoLair:
                    (*anger_spridx) = GBS_creature_states_sleep;
                    break;
                case AngR_Other:
                    (*anger_spridx) = GBS_creature_states_angry;
                    break;
                default:
                    break;
                }
            }
        }
    }
}

void draw_status_sprites(long scrpos_x, long scrpos_y, struct Thing *thing)
{
    struct PlayerInfo *player = get_my_player();
    const struct Camera *cam = get_local_camera(player->acamera);
    if (cam == NULL)
    {
        return;
    }

    float scale_by_zoom;
    int base_size = creature_status_size * 256;
    switch (cam->view_mode)
    {
    case PVM_IsoWibbleView:
    case PVM_IsoStraightView:
        // 1st argument: the scale when fully zoomed out. 2nd argument: the scale at base level zoom.
        scale_by_zoom = LbLerp(0.15, 1.00, hud_scale);
        break;
    case PVM_FrontView:
        scale_by_zoom = LbLerp(0.15, 1.00, hud_scale);
        break;
    case PVM_ParchmentView:
        scale_by_zoom = 1;
        break;
    default:
        return; // Do not draw if camera is 1st person.
    }

    unsigned short flg_mem;

    flg_mem = lbDisplay.DrawFlags;
    lbDisplay.DrawFlags = 0;

    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    if (cctrl->force_health_flower_hidden == true)
        return;
    if (flag_is_set(game.mode_flags,MFlg_NoHeroHealthFlower))
    {
        if (player->thing_under_hand != thing->index)
        {
            cctrl->thought_bubble_last_turn_drawn = game.play_gameturn;
            if (cctrl->force_health_flower_displayed == false)
            {
                return;
            }
        }
        cctrl->thought_bubble_display_timer = 40;
    }

    short health_spridx;
    short state_spridx;
    signed short anger_spridx;

    anger_spridx = 0;
    health_spridx = 0;
    state_spridx = 0;

    CrtrExpLevel exp_level = min(cctrl->exp_level, 9);
    if (cam->view_mode != PVM_ParchmentView)
    {
        fill_status_sprite_indexes(thing, cctrl, &health_spridx, &state_spridx, &anger_spridx);
    }

    int h_add;
    h_add = 0;
    int w;
    int h;
    const struct TbSprite *spr;
    int bs_units_per_px;
    spr = get_button_sprite(GBS_creature_states_cloud);
    bs_units_per_px = units_per_pixel_ui * 2 * scale_by_zoom;

    if (cam->view_mode == PVM_FrontView)
    {
        float flower_distance = 1280; // Higher number means flower is further away from creature.
        scrpos_y -= (int)((flower_distance / spr->SHeight) * ((float)camera_zoom / FRONTVIEW_CAMERA_ZOOM_MAX));
    }

    if (state_spridx || anger_spridx)
    {
        spr = get_button_sprite(GBS_creature_states_cloud);
        w = (base_size * spr->SWidth * bs_units_per_px / 16) >> 13;
        h = (base_size * spr->SHeight * bs_units_per_px / 16) >> 13;
        LbSpriteDrawScaled(scrpos_x - w / 2, scrpos_y - h, spr, w, h);
    }

    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR8;
    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    if (((game.play_gameturn % (8 * gui_blink_rate)) < 4 * gui_blink_rate) && (anger_spridx > 0))
    {
        spr = get_button_sprite(anger_spridx);
        w = (base_size * spr->SWidth * bs_units_per_px / 16) >> 13;
        h = (base_size * spr->SHeight * bs_units_per_px / 16) >> 13;
        LbSpriteDrawScaled(scrpos_x - w / 2, scrpos_y - h, spr, w, h);
        spr = get_button_sprite_for_player(state_spridx, thing->owner);
        h_add += spr->SHeight * bs_units_per_px / 16;
    }
    else if (state_spridx)
    {
        spr = get_button_sprite_for_player(state_spridx, thing->owner);
        w = (base_size * spr->SWidth * bs_units_per_px / 16) >> 13;
        h = (base_size * spr->SHeight * bs_units_per_px / 16) >> 13;
        LbSpriteDrawScaled(scrpos_x - w / 2, scrpos_y - h, spr, w, h);
        h_add += h;
    }

    if ((thing->lair.spr_size > 0) && (health_spridx > 0) && ((game.play_gameturn % (2 * gui_blink_rate)) >= gui_blink_rate))
    {
        int flash_color = get_player_color_idx(thing->owner);
        if (flash_color == PLAYER_NEUTRAL)
        {
            flash_color = (game.play_gameturn % (4 * neutral_flash_rate)) / neutral_flash_rate;
        }
        spr = get_button_sprite_for_player(health_spridx, thing->owner);
        w = (base_size * spr->SWidth * bs_units_per_px / 16) >> 13;
        h = (base_size * spr->SHeight * bs_units_per_px / 16) >> 13;
        LbSpriteDrawScaledOneColour(scrpos_x - w / 2, scrpos_y - h - h_add, spr, w, h, player_flash_colours[flash_color]);
    }
    else
    {
        // Determine if the creature is under the player's hand (being hovered over).
        TbBool is_thing_under_hand = (player->thing_under_hand == thing->index);
        // Check if the creature is an enemy and is visible.
        TbBool is_enemy_and_visible = players_are_enemies(player->id_number, thing->owner) && !creature_is_invisible(thing);
        // Check if the creature belongs to the player, is hurt but not unconscious.
        TbBool is_owned_and_hurt = false;
        // Check if the creature belongs to an ally.
        TbBool is_allied = false;
        TbBool should_drag_to_lair = false;
        TbBool is_zombie_player = !flag_is_set(get_player(thing->owner)->allocflags, PlaF_Allocated);
        TbBool forced_visible = cctrl->force_health_flower_displayed;
        if (!is_enemy_and_visible)
        {
            is_owned_and_hurt = creature_would_benefit_from_healing(thing) && !creature_is_being_unconscious(thing) && (player->id_number == thing->owner);
            is_allied = players_are_mutual_allies(player->id_number, thing->owner) && (player->id_number != thing->owner);
            should_drag_to_lair = creature_is_being_unconscious(thing) && (player->id_number == thing->owner)
            // Check if the creature has a lair room or can heal in a lair.
            && ((game.conf.rules[thing->owner].workers.drag_to_lair == 1 && !room_is_invalid(get_creature_lair_room(thing)))
            // Or check if the creature can have lair and heal in it.
            || (game.conf.rules[thing->owner].workers.drag_to_lair == 2 && creature_can_do_healing_sleep(thing)));
        }
        // Check if the creature is in combat.
        TbBool is_in_combat = (cctrl->combat_flags != 0);
        // Check if the creature has a lair.
        TbBool has_lair = (thing->lair.spr_size > 0);
        // Determine if the current view is the schematic top-down map view.
        TbBool is_parchment_map_view = (cam->view_mode == PVM_ParchmentView);
        if ((forced_visible)
        || (is_thing_under_hand)
        || (is_enemy_and_visible)
        || (is_owned_and_hurt)
        || (is_allied)
        || (is_zombie_player)
        || (thing->owner == PLAYER_NEUTRAL)
        // If drag_to_lair rule is active.
        || (should_drag_to_lair)
        || (is_in_combat)
        || (has_lair)
        || (is_parchment_map_view))
        {
            if (health_spridx > 0)
            {
                spr = get_button_sprite_for_player(health_spridx, thing->owner);
                w = (base_size * spr->SWidth * bs_units_per_px / 16) >> 13;
                h = (base_size * spr->SHeight * bs_units_per_px / 16) >> 13;
                LbSpriteDrawScaled(scrpos_x - w / 2, scrpos_y - h - h_add, spr, w, h);
            }
            spr = get_button_sprite(GBS_creature_flower_level_01 + exp_level);
            w = (base_size * spr->SWidth * bs_units_per_px / 16) >> 13;
            h = (base_size * spr->SHeight * bs_units_per_px / 16) >> 13;
            LbSpriteDrawScaled(scrpos_x - w / 2, scrpos_y - h - h_add, spr, w, h);
        }
    }
    lbDisplay.DrawFlags = flg_mem;
}

static void draw_iso_only_fastview_mapwho(struct Camera *cam, struct BucketKindJontySprite *spr)
{
    if (cam->view_mode == PVM_FrontView)
      draw_fastview_mapwho(cam, spr);
}

#define ROOM_FLAG_PROGRESS_BAR_WIDTH 10
static void draw_room_flag_top(long x, long y, int units_per_px, const struct Room *room)
{
    unsigned long flg_mem;
    flg_mem = lbDisplay.DrawFlags;
    int bar_fill;
    int bar_empty;
    const struct TbSprite *spr;
    int ps_units_per_px;
    spr = get_panel_sprite(GPS_rpanel_room_ensign_filled);
    ps_units_per_px = 36*units_per_px/spr->SHeight;
    LbSpriteDrawScaled(x, y, spr, spr->SWidth * ps_units_per_px / 16, spr->SHeight * ps_units_per_px / 16);
    struct RoomConfigStats *roomst;
    roomst = &game.conf.slab_conf.room_cfgstats[room->kind];
    int barpos_x;
    barpos_x = x + spr->SWidth * ps_units_per_px / 16 - (8 * units_per_px - 8) / 16;
    spr = get_panel_sprite(roomst->medsym_sprite_idx);
    LbSpriteDrawResized(x - 2*units_per_px/16, y - 4*units_per_px/16, ps_units_per_px, spr);
    bar_fill = ROOM_FLAG_PROGRESS_BAR_WIDTH;
    bar_empty = 0;
    if (room->slabs_count > 0)
    {
        bar_fill = ROOM_FLAG_PROGRESS_BAR_WIDTH * room->health / compute_room_max_health(room->slabs_count, room->efficiency);
        bar_empty = ROOM_FLAG_PROGRESS_BAR_WIDTH - bar_fill;
    }
    int bar_width;
    int bar_height;
    bar_width = (2 * bar_empty * units_per_px + 8) / 16;
    // Compute height in a way which will assure covering whole bar area
    bar_height = (5 * units_per_px - 8) / 16;
    LbDrawBox(barpos_x - bar_width, y +  (8 * units_per_px + 8) / 16, bar_width, bar_height, colours[0][0][0]);
    bar_empty = 0;
    if (room->total_capacity > 0)
    {
        bar_fill = ROOM_FLAG_PROGRESS_BAR_WIDTH * room->used_capacity / room->total_capacity;
        bar_empty = ROOM_FLAG_PROGRESS_BAR_WIDTH - bar_fill;
    }
    bar_width = (2 * bar_empty * units_per_px + 8) / 16;
    LbDrawBox(barpos_x - bar_width, y + (16 * units_per_px + 8) / 16, bar_width, bar_height, colours[0][0][0]);
    bar_empty = 0;
    {
        bar_fill = ROOM_FLAG_PROGRESS_BAR_WIDTH * room->efficiency / ROOM_EFFICIENCY_MAX;
        bar_empty = ROOM_FLAG_PROGRESS_BAR_WIDTH - bar_fill;
    }
    bar_width = (2 * bar_empty * units_per_px + 8) / 16;
    LbDrawBox(barpos_x - bar_width, y + (24 * units_per_px + 8) / 16, bar_width, bar_height, colours[0][0][0]);
    lbDisplay.DrawFlags = flg_mem;
}
#undef ROOM_FLAG_PROGRESS_BAR_WIDTH

static void draw_engine_room_flag_top(struct BucketKindRoomFlag *rflg)
{
    lbDisplay.DrawFlags &= ~Lb_SPRITE_FLIP_HORIZ;

    struct Room *room = room_get(rflg->lvl);
    if (!room_exists(room) || !room_can_have_ensign(room->kind)) {
        return;
    }
    struct PlayerInfo *player = get_my_player();
    const struct Camera *cam = get_local_camera(player->acamera);

    if (
        cam->view_mode == PVM_IsoWibbleView ||
        cam->view_mode == PVM_FrontView ||
        cam->view_mode == PVM_IsoStraightView
    ) {
        if (settings.roomflags_on)
        {
            int top_of_pole_offset, zoom_factor;
            // 1st argument: the scale when fully zoomed out. 2nd argument: the scale at base level zoom
            float scale_by_zoom = LbLerp(0.15, 1.00, hud_scale);

            if (cam->view_mode == PVM_FrontView) {
                zoom_factor = (4094*scale_by_zoom);
                top_of_pole_offset = (zoom_factor << 7 >> 13) * (units_per_pixel_ui)/16;
            } else {
                zoom_factor = camera_zoom;
                top_of_pole_offset = (zoom_factor << 7 >> 13);
            }
            draw_room_flag_top(rflg->x, rflg->y - top_of_pole_offset, (units_per_pixel_ui *scale_by_zoom), room);
        }
    }
}

static void draw_stripey_line(long x1,long y1,long x2,long y2,unsigned char line_color)
{
    if ((x1 == x2) && (y1 == y2)) return; // todo if distance is 0, provide a red square

    // get the 4 least significant bits of game.play_gameturn, to loop through the starting index of the color array, using numbers 0-15.
    unsigned char color_index = game.play_gameturn & 0xf;

    // get engine window width and height
    struct PlayerInfo *player = get_my_player();
    long relative_window_width = ((player->engine_window_width * 256) / (pixel_size * 256)) - 1;
    long relative_window_height = ((player->engine_window_height * 256) / (pixel_size * 256)) - 1;

    // Bresenham’s Line Drawing Algorithm - handles all octants
    // A and B are relative, and are set to be either X (shallow curves) or Y (steep curves).
    // A1 and A2, and B1 and B2, are swapped when the line is directed towards -1 X/Y.
    // A and B are incremented, apart from when the slope of the lines goes from 0 to -1 in A, where B will decrement instead
    int32_t distance_a, distance_b, a, b, a1, b1, a2, b2, relative_window_a, relative_window_b, remainder, remainder_limit;
    int32_t *x_coord, *y_coord; // Maintain a reference to the actual X/Y coordinates, even after swapping A and B

    if (abs(y2 - y1) < abs(x2 - x1))
    {
        x_coord = &a;
        y_coord = &b;
        relative_window_a = relative_window_width;
        relative_window_b = relative_window_height;
        if (x1 < x2)
        {
            a1 = x1;
            b1 = y1;
            a2 = x2;
            b2 = y2;
        }
        else // Swap n1 with n2
        {
            a1 = x2;
            b1 = y2;
            a2 = x1;
            b2 = y1;
            color_index = 0xf - color_index; // invert the color index
        }
    }
    else // Reverse X and Y
    {
        x_coord = &b;
        y_coord = &a;
        relative_window_a = relative_window_height;
        relative_window_b = relative_window_width;
        if (y1 < y2)
        {
            a1 = y1;
            b1 = x1;
            a2 = y2;
            b2 = x2;
        }
        else // Swap n1 with n2
        {
            a1 = y2;
            b1 = x2;
            a2 = y1;
            b2 = x1;
            color_index = 0xf - color_index; // invert the color index
        }
    }

    distance_a = a2 - a1;
    distance_b = b2 - b1;

    if (distance_b == 0)
    {
        if ((b1 < 0) || (b1 > relative_window_b))
        {
            return; // line is off the screen
        }
    }
    if (distance_a == 0)
    {
        if ((a1 < 0) || (a1 > relative_window_a))
        {
            return; // line is off the screen
        }
    }

    long start_b_dist_from_window = 0 - b1; // For window clipping
    long end_b_dist_from_window = b2 - relative_window_b; // For window clipping

    // Handle going towards 0 in B (i.e. B counts down, not up)
    long b_increment = 1;
    if (distance_b < 0)
    {
        b_increment = -1;
        distance_b = -distance_b;
        start_b_dist_from_window = b1 - relative_window_b;
        end_b_dist_from_window = 0 - b2;
    }

    // Clip line within engine_window
    remainder_limit = (distance_b+1)/2;
    // Find starting A coord
    if (distance_b == 0)
    {
        remainder = 0;
    }
    else
    {
        remainder = start_b_dist_from_window * distance_a % distance_b;
    }
    long min_a_start = 0;
    if ((b1 < 0 || b1 > relative_window_b))
    {
        min_a_start = a1 + ( (start_b_dist_from_window) * distance_a / distance_b );
        if (remainder >= remainder_limit)
        {
            min_a_start++;
        }
        min_a_start = max(min_a_start, 0);
    }
    long a_start = max(a1, min_a_start);
    // Find ending A coord
    if (distance_b == 0)
    {
        remainder = 0;
    }
    else
    {
        remainder = end_b_dist_from_window * distance_a % distance_b;
    }
    long max_a_end = relative_window_a;
    if (b2 < 0 || b2 > relative_window_b)
    {
        max_a_end = a2 - ( (end_b_dist_from_window) * distance_a / distance_b );
        if (remainder >= remainder_limit)
        {
            max_a_end--;
        }
        max_a_end = min(max_a_end, relative_window_a);

    }
    long a_end = min(a2, max_a_end);
    // Find starting B coord
    remainder_limit = (distance_a+1)/2;
    if (distance_a == 0)
    {
        remainder = 0;
    }
    else
    {
        remainder = (a_start - a1) * distance_b % distance_a; // initialise remainder for loop
    }
    long b_start =  (distance_a == 0) ? b1 : b1 + ( b_increment * (a_start - a1) * distance_b / distance_a );
    if (remainder >= remainder_limit)
    {
        remainder -= distance_a;
        b_start += b_increment;
    }
    b = b_start;

    // A hack-fix to ensure that pixels are always drawn on screen. Otherwise when zoomed in, pixels have trouble being drawn in the bottom right corner
    relative_window_a = lbDisplay.GraphicsScreenWidth;
    relative_window_b = lbDisplay.GraphicsScreenHeight;

    // Set up parameters before starting the drawing loop
    float custom_line_box_size = line_box_size / 100.0;
    int line_thickness = max(1, (custom_line_box_size * units_per_pixel_best / 16.0) );

    // Make the line slightly thinner when zoomed out
    line_thickness = LbLerp(line_thickness, 1, 1.0-hud_scale);

    int put_pixels_left = line_thickness/2; // Allocate half of the thickness to the left
    int put_pixels_right = line_thickness-put_pixels_left; // Remaining thickness is placed to the right

    TbBool isHorizontal = abs(x2 - x1) >= abs(y2 - y1); // Check if line is more horizontal than vertical, helps with the "pixel-art look".
    int temp_x, temp_y;
    float color_animation_position = color_index;
    // Main loop to draw the line
    for (a = a_start; a <= a_end; a++) {

        //if ((a < 0) || (a > relative_window_a) || (b < 0) || (b > relative_window_b))
        //{
        //    Temporary Error message, this should never appear in the log, but if it does, then the line must have been clipped incorrectly
        //    WARNMSG("draw_stripey_line: Pixel rendered outside engine window. X: %d, Y: %d, window_width: %d, window_height %d, A1: %d, A2 %d, B1 %d, B2 %d, a_start: %d, a_end: %d, b_start: %d, rWA: %d", *x_coord, *y_coord, relative_window_width, relative_window_height, a1, a2, b1, b2, a_start, a_end, b_start, relative_window_a);
        //}
        color_animation_position += LbLerp(1.0, 4.0, 1.0-hud_scale) * (16.0/units_per_pixel_best);
        if (color_animation_position >= 16.0) {
            color_animation_position -= 16.0;
        }
        color_index = max(0, (int)color_animation_position);

        // Nested loops to draw square pixels around each point for the specified thickness
        for (int dx = -put_pixels_left; dx < put_pixels_right; dx++) {
            for (int dy = -put_pixels_left; dy < put_pixels_right; dy++) {
                // Determine pixel coordinates based on line orientation
                if (isHorizontal) {
                    temp_x = *x_coord;
                    temp_y = *y_coord + dy;
                } else {
                    temp_x = *x_coord + dx;
                    temp_y = *y_coord;
                }

                // Draw the pixel if it's within the bounds of the window
                if ((temp_x >= 0) && (temp_x < relative_window_a) && (temp_y >= 0) && (temp_y < relative_window_b)) {
                    LbDrawPixel(temp_x, temp_y, colored_stripey_lines[line_color].stripey_line_color_array[color_index]);
                }
            }
        }

        if (remainder >= remainder_limit) {
            b += b_increment;
            remainder -= distance_a;
        }
        remainder += distance_b;
    }
}

static void draw_clipped_line(long x1, long y1, long x2, long y2, TbPixel color)
{
    struct PlayerInfo *player;
    if ((x1 >= 0) || (x2 >= 0))
    {
      if ((y1 >= 0) || (y2 >= 0))
      {
        player = get_my_player();
        if ((x1 < player->engine_window_width) || (x2 < player->engine_window_width))
        {
          if ((y1 < player->engine_window_height) || (y2 < player->engine_window_height))
          {
            draw_stripey_line(x1, y1, x2, y2, color);
          }
        }
      }
    }
}

static void draw_subdivided_near_polygon(struct BucketKindPolygonNearFP *polygon_data)
{
    struct XYZ coord_a;
    struct XYZ coord_b;
    struct XYZ coord_c;
    struct XYZ coord_d;
    struct XYZ coord_e;
    struct PolyPoint point_a;
    struct PolyPoint point_b;
    struct PolyPoint point_c;
    struct PolyPoint point_d;
    struct PolyPoint point_e;
    struct PolyPoint point_f;
    struct PolyPoint point_g;
    struct PolyPoint point_h;
    struct PolyPoint point_i;
    struct PolyPoint point_j;
    struct PolyPoint point_k;
    struct PolyPoint point_l;
    vec_map = block_ptrs[polygon_data->block];
    switch (polygon_data->subtype)
    {
    case 0:
        vec_mode = VM_QuadTextured;
        draw_gpoly(&polygon_data->vertex_first,&polygon_data->vertex_second,&polygon_data->vertex_third);
        break;
    case 1:
        vec_mode = VM_QuadTextured;
        coord_a.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_first.x) >> 1;
        coord_a.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_first.y) >> 1;
        coord_a.z = (polygon_data->coordinate_first.z + polygon_data->coordinate_second.z) >> 1;
        point_a.U = (polygon_data->vertex_first.U + polygon_data->vertex_second.U) >> 1;
        point_a.V = (polygon_data->vertex_second.V + polygon_data->vertex_first.V) >> 1;
        point_a.S = (polygon_data->vertex_second.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_a, &point_a);
        draw_gpoly(&polygon_data->vertex_first, &point_a, &polygon_data->vertex_third);
        draw_gpoly(&point_a, &polygon_data->vertex_second, &polygon_data->vertex_third);
        break;
    case 2:
        vec_mode = VM_QuadTextured;
        coord_a.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_third.x) >> 1;
        coord_a.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_third.y) >> 1;
        coord_a.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_second.z) >> 1;
        point_a.U = (polygon_data->vertex_third.U + polygon_data->vertex_second.U) >> 1;
        point_a.V = (polygon_data->vertex_third.V + polygon_data->vertex_second.V) >> 1;
        point_a.S = (polygon_data->vertex_third.S + polygon_data->vertex_second.S) >> 1;
        perspective(&coord_a, &point_a);
        draw_gpoly(&polygon_data->vertex_first, &polygon_data->vertex_second, &point_a);
        draw_gpoly(&polygon_data->vertex_first, &point_a, &polygon_data->vertex_third);
        break;
    case 3:
        vec_mode = VM_QuadTextured;
        coord_a.x = (polygon_data->coordinate_first.x + polygon_data->coordinate_third.x) >> 1;
        coord_a.y = (polygon_data->coordinate_third.y + polygon_data->coordinate_first.y) >> 1;
        coord_a.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_first.z) >> 1;
        point_a.U = (polygon_data->vertex_first.U + polygon_data->vertex_third.U) >> 1;
        point_a.V = (polygon_data->vertex_third.V + polygon_data->vertex_first.V) >> 1;
        point_a.S = (polygon_data->vertex_third.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_a, &point_a);
        draw_gpoly(&polygon_data->vertex_first, &polygon_data->vertex_second, &point_a);
        draw_gpoly(&point_a, &polygon_data->vertex_second, &polygon_data->vertex_third);
        break;
    case 4:
        vec_mode = VM_QuadTextured;
        coord_a.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_first.x) >> 1;
        coord_a.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_first.y) >> 1;
        coord_a.z = (polygon_data->coordinate_first.z + polygon_data->coordinate_second.z) >> 1;
        point_a.U = (polygon_data->vertex_first.U + polygon_data->vertex_second.U) >> 1;
        point_a.V = (polygon_data->vertex_second.V + polygon_data->vertex_first.V) >> 1;
        point_a.S = (polygon_data->vertex_second.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_third.x) >> 1;
        coord_b.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_third.y) >> 1;
        coord_b.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_second.z) >> 1;
        point_b.U = (polygon_data->vertex_third.U + polygon_data->vertex_second.U) >> 1;
        point_b.V = (polygon_data->vertex_third.V + polygon_data->vertex_second.V) >> 1;
        point_b.S = (polygon_data->vertex_third.S + polygon_data->vertex_second.S) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (polygon_data->coordinate_first.x + polygon_data->coordinate_third.x) >> 1;
        coord_c.y = (polygon_data->coordinate_third.y + polygon_data->coordinate_first.y) >> 1;
        coord_c.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_first.z) >> 1;
        point_c.U = (polygon_data->vertex_first.U + polygon_data->vertex_third.U) >> 1;
        point_c.V = (polygon_data->vertex_third.V + polygon_data->vertex_first.V) >> 1;
        point_c.S = (polygon_data->vertex_third.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_c, &point_c);
        draw_gpoly(&polygon_data->vertex_first, &point_a, &point_c);
        draw_gpoly(&point_a, &polygon_data->vertex_second, &point_b);
        draw_gpoly(&point_a, &point_b, &point_c);
        draw_gpoly(&point_c, &point_b, &polygon_data->vertex_third);
        break;
    case 5:
        vec_mode = VM_QuadTextured;
        coord_a.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_first.x) >> 1;
        coord_a.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_first.y) >> 1;
        coord_a.z = (polygon_data->coordinate_first.z + polygon_data->coordinate_second.z) >> 1;
        point_a.U = (polygon_data->vertex_first.U + polygon_data->vertex_second.U) >> 1;
        point_a.V = (polygon_data->vertex_second.V + polygon_data->vertex_first.V) >> 1;
        point_a.S = (polygon_data->vertex_second.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (coord_a.x + polygon_data->coordinate_first.x) >> 1;
        coord_b.y = (coord_a.y + polygon_data->coordinate_first.y) >> 1;
        coord_b.z = (coord_a.z + polygon_data->coordinate_first.z) >> 1;
        point_b.U = (point_a.U + polygon_data->vertex_first.U) >> 1;
        point_b.V = (point_a.V + polygon_data->vertex_first.V) >> 1;
        point_b.S = (point_a.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (coord_a.x + polygon_data->coordinate_second.x) >> 1;
        coord_c.y = (coord_a.y + polygon_data->coordinate_second.y) >> 1;
        coord_c.z = (coord_a.z + polygon_data->coordinate_second.z) >> 1;
        point_c.U = (point_a.U + polygon_data->vertex_second.U) >> 1;
        point_c.V = (point_a.V + polygon_data->vertex_second.V) >> 1;
        point_c.S = (point_a.S + polygon_data->vertex_second.S) >> 1;
        perspective(&coord_c, &point_c);
        draw_gpoly(&polygon_data->vertex_first, &point_b, &polygon_data->vertex_third);
        draw_gpoly(&point_b, &point_a, &polygon_data->vertex_third);
        draw_gpoly(&point_a, &point_c, &polygon_data->vertex_third);
        draw_gpoly(&point_c, &polygon_data->vertex_second, &polygon_data->vertex_third);
        break;
    case 6:
        vec_mode = VM_QuadTextured;
        coord_a.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_third.x) >> 1;
        coord_a.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_third.y) >> 1;
        coord_a.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_second.z) >> 1;
        point_a.U = (polygon_data->vertex_third.U + polygon_data->vertex_second.U) >> 1;
        point_a.V = (polygon_data->vertex_third.V + polygon_data->vertex_second.V) >> 1;
        point_a.S = (polygon_data->vertex_third.S + polygon_data->vertex_second.S) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (coord_a.x + polygon_data->coordinate_second.x) >> 1;
        coord_b.y = (coord_a.y + polygon_data->coordinate_second.y) >> 1;
        coord_b.z = (coord_a.z + polygon_data->coordinate_second.z) >> 1;
        point_b.U = (point_a.U + polygon_data->vertex_second.U) >> 1;
        point_b.V = (point_a.V + polygon_data->vertex_second.V) >> 1;
        point_b.S = (point_a.S + polygon_data->vertex_second.S) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (coord_a.x + polygon_data->coordinate_third.x) >> 1;
        coord_c.y = (coord_a.y + polygon_data->coordinate_third.y) >> 1;
        coord_c.z = (coord_a.z + polygon_data->coordinate_third.z) >> 1;
        point_c.U = (point_a.U + polygon_data->vertex_third.U) >> 1;
        point_c.V = (point_a.V + polygon_data->vertex_third.V) >> 1;
        point_c.S = (point_a.S + polygon_data->vertex_third.S) >> 1;
        perspective(&coord_c, &point_c);
        draw_gpoly(&polygon_data->vertex_first, &polygon_data->vertex_second, &point_b);
        draw_gpoly(&polygon_data->vertex_first, &point_b, &point_a);
        draw_gpoly(&polygon_data->vertex_first, &point_a, &point_c);
        draw_gpoly(&polygon_data->vertex_first, &point_c, &polygon_data->vertex_third);
        break;
    case 7:
        vec_mode = VM_QuadTextured;
        coord_a.x = (polygon_data->coordinate_first.x + polygon_data->coordinate_third.x) >> 1;
        coord_a.y = (polygon_data->coordinate_third.y + polygon_data->coordinate_first.y) >> 1;
        coord_a.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_first.z) >> 1;
        point_a.U = (polygon_data->vertex_first.U + polygon_data->vertex_third.U) >> 1;
        point_a.V = (polygon_data->vertex_third.V + polygon_data->vertex_first.V) >> 1;
        point_a.S = (polygon_data->vertex_third.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (coord_a.x + polygon_data->coordinate_third.x) >> 1;
        coord_b.y = (coord_a.y + polygon_data->coordinate_third.y) >> 1;
        coord_b.z = (coord_a.z + polygon_data->coordinate_third.z) >> 1;
        point_b.U = (point_a.U + polygon_data->vertex_third.U) >> 1;
        point_b.V = (point_a.V + polygon_data->vertex_third.V) >> 1;
        point_b.S = (point_a.S + polygon_data->vertex_third.S) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (coord_a.x + polygon_data->coordinate_first.x) >> 1;
        coord_c.y = (coord_a.y + polygon_data->coordinate_first.y) >> 1;
        coord_c.z = (coord_a.z + polygon_data->coordinate_first.z) >> 1;
        point_c.U = (point_a.U + polygon_data->vertex_first.U) >> 1;
        point_c.V = (point_a.V + polygon_data->vertex_first.V) >> 1;
        point_c.S = (point_a.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_c, &point_c);
        draw_gpoly(&polygon_data->vertex_second, &polygon_data->vertex_third, &point_b);
        draw_gpoly(&polygon_data->vertex_second, &point_b, &point_a);
        draw_gpoly(&polygon_data->vertex_second, &point_a, &point_c);
        draw_gpoly(&polygon_data->vertex_second, &point_c, &polygon_data->vertex_first);
        break;
    case 8:
        vec_mode = VM_QuadTextured;
        coord_a.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_first.x) >> 1;
        coord_a.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_first.y) >> 1;
        coord_a.z = (polygon_data->coordinate_first.z + polygon_data->coordinate_second.z) >> 1;
        point_a.U = (polygon_data->vertex_first.U + polygon_data->vertex_second.U) >> 1;
        point_a.V = (polygon_data->vertex_second.V + polygon_data->vertex_first.V) >> 1;
        point_a.S = (polygon_data->vertex_second.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_third.x) >> 1;
        coord_b.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_third.y) >> 1;
        coord_b.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_second.z) >> 1;
        point_b.U = (polygon_data->vertex_third.U + polygon_data->vertex_second.U) >> 1;
        point_b.V = (polygon_data->vertex_third.V + polygon_data->vertex_second.V) >> 1;
        point_b.S = (polygon_data->vertex_third.S + polygon_data->vertex_second.S) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (polygon_data->coordinate_first.x + polygon_data->coordinate_third.x) >> 1;
        coord_c.y = (polygon_data->coordinate_third.y + polygon_data->coordinate_first.y) >> 1;
        coord_c.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_first.z) >> 1;
        point_c.U = (polygon_data->vertex_first.U + polygon_data->vertex_third.U) >> 1;
        point_c.V = (polygon_data->vertex_third.V + polygon_data->vertex_first.V) >> 1;
        point_c.S = (polygon_data->vertex_third.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_a.x + polygon_data->coordinate_first.x) >> 1;
        coord_d.y = (coord_a.y + polygon_data->coordinate_first.y) >> 1;
        coord_d.z = (coord_a.z + polygon_data->coordinate_first.z) >> 1;
        point_d.U = (point_a.U + polygon_data->vertex_first.U) >> 1;
        point_d.V = (point_a.V + polygon_data->vertex_first.V) >> 1;
        point_d.S = (point_a.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_a.x + polygon_data->coordinate_second.x) >> 1;
        coord_e.y = (coord_a.y + polygon_data->coordinate_second.y) >> 1;
        coord_e.z = (coord_a.z + polygon_data->coordinate_second.z) >> 1;
        point_e.U = (point_a.U + polygon_data->vertex_second.U) >> 1;
        point_e.V = (point_a.V + polygon_data->vertex_second.V) >> 1;
        point_e.S = (point_a.S + polygon_data->vertex_second.S) >> 1;
        perspective(&coord_e, &point_e);
        draw_gpoly(&polygon_data->vertex_first, &point_d, &point_c);
        draw_gpoly(&point_d, &point_a, &point_c);
        draw_gpoly(&point_a, &point_e, &point_b);
        draw_gpoly(&point_e, &polygon_data->vertex_second, &point_b);
        draw_gpoly(&point_a, &point_b, &point_c);
        draw_gpoly(&point_c, &point_b, &polygon_data->vertex_third);
        break;
    case 9:
        vec_mode = VM_QuadTextured;
        coord_a.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_first.x) >> 1;
        coord_a.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_first.y) >> 1;
        coord_a.z = (polygon_data->coordinate_first.z + polygon_data->coordinate_second.z) >> 1;
        point_a.U = (polygon_data->vertex_first.U + polygon_data->vertex_second.U) >> 1;
        point_a.V = (polygon_data->vertex_second.V + polygon_data->vertex_first.V) >> 1;
        point_a.S = (polygon_data->vertex_second.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_third.x) >> 1;
        coord_b.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_third.y) >> 1;
        coord_b.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_second.z) >> 1;
        point_b.U = (polygon_data->vertex_third.U + polygon_data->vertex_second.U) >> 1;
        point_b.V = (polygon_data->vertex_third.V + polygon_data->vertex_second.V) >> 1;
        point_b.S = (polygon_data->vertex_third.S + polygon_data->vertex_second.S) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (polygon_data->coordinate_first.x + polygon_data->coordinate_third.x) >> 1;
        coord_c.y = (polygon_data->coordinate_third.y + polygon_data->coordinate_first.y) >> 1;
        coord_c.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_first.z) >> 1;
        point_c.U = (polygon_data->vertex_first.U + polygon_data->vertex_third.U) >> 1;
        point_c.V = (polygon_data->vertex_third.V + polygon_data->vertex_first.V) >> 1;
        point_c.S = (polygon_data->vertex_third.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_b.x + polygon_data->coordinate_second.x) >> 1;
        coord_d.y = (coord_b.y + polygon_data->coordinate_second.y) >> 1;
        coord_d.z = (coord_b.z + polygon_data->coordinate_second.z) >> 1;
        point_d.U = (point_b.U + polygon_data->vertex_second.U) >> 1;
        point_d.V = (point_b.V + polygon_data->vertex_second.V) >> 1;
        point_d.S = (point_b.S + polygon_data->vertex_second.S) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_b.x + polygon_data->coordinate_third.x) >> 1;
        coord_e.y = (coord_b.y + polygon_data->coordinate_third.y) >> 1;
        coord_e.z = (coord_b.z + polygon_data->coordinate_third.z) >> 1;
        point_e.U = (point_b.U + polygon_data->vertex_third.U) >> 1;
        point_e.V = (point_b.V + polygon_data->vertex_third.V) >> 1;
        point_e.S = (point_b.S + polygon_data->vertex_third.S) >> 1;
        perspective(&coord_e, &point_e);
        draw_gpoly(&polygon_data->vertex_first, &point_a, &point_c);
        draw_gpoly(&point_a, &point_b, &point_c);
        draw_gpoly(&point_a, &polygon_data->vertex_second, &point_d);
        draw_gpoly(&point_a, &point_d, &point_b);
        draw_gpoly(&point_c, &point_b, &point_e);
        draw_gpoly(&point_c, &point_e, &polygon_data->vertex_third);
        break;
    case 10:
        vec_mode = VM_QuadTextured;
        coord_a.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_first.x) >> 1;
        coord_a.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_first.y) >> 1;
        coord_a.z = (polygon_data->coordinate_first.z + polygon_data->coordinate_second.z) >> 1;
        point_a.U = (polygon_data->vertex_first.U + polygon_data->vertex_second.U) >> 1;
        point_a.V = (polygon_data->vertex_second.V + polygon_data->vertex_first.V) >> 1;
        point_a.S = (polygon_data->vertex_second.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_third.x) >> 1;
        coord_b.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_third.y) >> 1;
        coord_b.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_second.z) >> 1;
        point_b.U = (polygon_data->vertex_third.U + polygon_data->vertex_second.U) >> 1;
        point_b.V = (polygon_data->vertex_third.V + polygon_data->vertex_second.V) >> 1;
        point_b.S = (polygon_data->vertex_third.S + polygon_data->vertex_second.S) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (polygon_data->coordinate_first.x + polygon_data->coordinate_third.x) >> 1;
        coord_c.y = (polygon_data->coordinate_third.y + polygon_data->coordinate_first.y) >> 1;
        coord_c.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_first.z) >> 1;
        point_c.U = (polygon_data->vertex_first.U + polygon_data->vertex_third.U) >> 1;
        point_c.V = (polygon_data->vertex_third.V + polygon_data->vertex_first.V) >> 1;
        point_c.S = (polygon_data->vertex_third.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_c.x + polygon_data->coordinate_third.x) >> 1;
        coord_d.y = (coord_c.y + polygon_data->coordinate_third.y) >> 1;
        coord_d.z = (coord_c.z + polygon_data->coordinate_third.z) >> 1;
        point_d.U = (point_c.U + polygon_data->vertex_third.U) >> 1;
        point_d.V = (point_c.V + polygon_data->vertex_third.V) >> 1;
        point_d.S = (point_c.S + polygon_data->vertex_third.S) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_c.x + polygon_data->coordinate_first.x) >> 1;
        coord_e.y = (coord_c.y + polygon_data->coordinate_first.y) >> 1;
        coord_e.z = (coord_c.z + polygon_data->coordinate_first.z) >> 1;
        point_e.U = (point_c.U + polygon_data->vertex_first.U) >> 1;
        point_e.V = (point_c.V + polygon_data->vertex_first.V) >> 1;
        point_e.S = (point_c.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_e, &point_e);
        draw_gpoly(&point_a, &polygon_data->vertex_second, &point_b);
        draw_gpoly(&point_a, &point_b, &point_c);
        draw_gpoly(&polygon_data->vertex_first, &point_a, &point_e);
        draw_gpoly(&point_e, &point_a, &point_c);
        draw_gpoly(&point_c, &point_b, &point_d);
        draw_gpoly(&point_d, &point_b, &polygon_data->vertex_third);
        break;
    case 11: // Flickers in 1st person (before flicker_fix() was applied)
        vec_mode = VM_QuadTextured;
        coord_a.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_first.x) >> 1;
        coord_a.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_first.y) >> 1;
        coord_a.z = (polygon_data->coordinate_first.z + polygon_data->coordinate_second.z) >> 1;
        point_a.U = (polygon_data->vertex_first.U + polygon_data->vertex_second.U) >> 1;
        point_a.V = (polygon_data->vertex_second.V + polygon_data->vertex_first.V) >> 1;
        point_a.S = (polygon_data->vertex_second.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_third.x) >> 1;
        coord_b.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_third.y) >> 1;
        coord_b.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_second.z) >> 1;
        point_b.U = (polygon_data->vertex_third.U + polygon_data->vertex_second.U) >> 1;
        point_b.V = (polygon_data->vertex_third.V + polygon_data->vertex_second.V) >> 1;
        point_b.S = (polygon_data->vertex_third.S + polygon_data->vertex_second.S) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (polygon_data->coordinate_first.x + polygon_data->coordinate_third.x) >> 1;
        coord_c.y = (polygon_data->coordinate_third.y + polygon_data->coordinate_first.y) >> 1;
        coord_c.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_first.z) >> 1;
        point_c.U = (polygon_data->vertex_first.U + polygon_data->vertex_third.U) >> 1;
        point_c.V = (polygon_data->vertex_third.V + polygon_data->vertex_first.V) >> 1;
        point_c.S = (polygon_data->vertex_third.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_a.x + polygon_data->coordinate_first.x) >> 1;
        coord_d.y = (coord_a.y + polygon_data->coordinate_first.y) >> 1;
        coord_d.z = (coord_a.z + polygon_data->coordinate_first.z) >> 1;
        point_d.U = (point_a.U + polygon_data->vertex_first.U) >> 1;
        point_d.V = (point_a.V + polygon_data->vertex_first.V) >> 1;
        point_d.S = (point_a.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_a.x + polygon_data->coordinate_second.x) >> 1;
        coord_e.y = (coord_a.y + polygon_data->coordinate_second.y) >> 1;
        coord_e.z = (coord_a.z + polygon_data->coordinate_second.z) >> 1;
        point_e.U = (point_a.U + polygon_data->vertex_second.U) >> 1;
        point_e.V = (point_a.V + polygon_data->vertex_second.V) >> 1;
        point_e.S = (point_a.S + polygon_data->vertex_second.S) >> 1;
        perspective(&coord_e, &point_e);
        coord_d.x = (coord_b.x + polygon_data->coordinate_second.x) >> 1;
        coord_d.y = (coord_b.y + polygon_data->coordinate_second.y) >> 1;
        coord_d.z = (coord_b.z + polygon_data->coordinate_second.z) >> 1;
        point_f.U = (point_b.U + polygon_data->vertex_second.U) >> 1;
        point_f.V = (point_b.V + polygon_data->vertex_second.V) >> 1;
        point_f.S = (point_b.S + polygon_data->vertex_second.S) >> 1;
        perspective(&coord_d, &point_f);
        coord_e.x = (coord_b.x + polygon_data->coordinate_third.x) >> 1;
        coord_e.y = (coord_b.y + polygon_data->coordinate_third.y) >> 1;
        coord_e.z = (coord_b.z + polygon_data->coordinate_third.z) >> 1;
        point_g.U = (point_b.U + polygon_data->vertex_third.U) >> 1;
        point_g.V = (point_b.V + polygon_data->vertex_third.V) >> 1;
        point_g.S = (point_b.S + polygon_data->vertex_third.S) >> 1;
        perspective(&coord_e, &point_g);
        coord_d.x = (coord_c.x + polygon_data->coordinate_third.x) >> 1;
        coord_d.y = (coord_c.y + polygon_data->coordinate_third.y) >> 1;
        coord_d.z = (coord_c.z + polygon_data->coordinate_third.z) >> 1;
        point_h.U = (point_c.U + polygon_data->vertex_third.U) >> 1;
        point_h.V = (point_c.V + polygon_data->vertex_third.V) >> 1;
        point_h.S = (point_c.S + polygon_data->vertex_third.S) >> 1;
        perspective(&coord_d, &point_h);
        coord_e.x = (coord_c.x + polygon_data->coordinate_first.x) >> 1;
        coord_e.y = (coord_c.y + polygon_data->coordinate_first.y) >> 1;
        coord_e.z = (coord_c.z + polygon_data->coordinate_first.z) >> 1;
        point_i.U = (point_c.U + polygon_data->vertex_first.U) >> 1;
        point_i.V = (point_c.V + polygon_data->vertex_first.V) >> 1;
        point_i.S = (point_c.S + polygon_data->vertex_first.S) >> 1;
        perspective(&coord_e, &point_i);
        coord_d.x = (coord_a.x + coord_c.x) >> 1;
        coord_d.y = (coord_a.y + coord_c.y) >> 1;
        coord_d.z = (coord_a.z + coord_c.z) >> 1;
        point_j.U = (point_a.U + point_c.U) >> 1;
        point_j.V = (point_a.V + point_c.V) >> 1;
        point_j.S = (point_a.S + point_c.S) >> 1;
        perspective(&coord_d, &point_j);
        coord_e.x = (coord_a.x + coord_b.x) >> 1;
        coord_e.y = (coord_a.y + coord_b.y) >> 1;
        coord_e.z = (coord_a.z + coord_b.z) >> 1;
        point_k.U = (point_a.U + point_b.U) >> 1;
        point_k.V = (point_a.V + point_b.V) >> 1;
        point_k.S = (point_a.S + point_b.S) >> 1;
        perspective(&coord_e, &point_k);
        coord_d.x = (coord_b.x + coord_c.x) >> 1;
        coord_d.y = (coord_b.y + coord_c.y) >> 1;
        coord_d.z = (coord_b.z + coord_c.z) >> 1;
        point_l.U = (point_b.U + point_c.U) >> 1;
        point_l.V = (point_b.V + point_c.V) >> 1;
        point_l.S = (point_b.S + point_c.S) >> 1;
        perspective(&coord_d, &point_l);
        draw_gpoly(&polygon_data->vertex_first, &point_d, &point_i);
        draw_gpoly(&point_d, &point_a, &point_j);
        draw_gpoly(&point_a, &point_e, &point_k);
        draw_gpoly(&point_e, &polygon_data->vertex_second, &point_f);
        draw_gpoly(&point_d, &point_j, &point_i);
        draw_gpoly(&point_a, &point_k, &point_j);
        draw_gpoly(&point_e, &point_f, &point_k);
        draw_gpoly(&point_i, &point_j, &point_c);
        draw_gpoly(&point_j, &point_k, &point_l);
        draw_gpoly(&point_k, &point_f, &point_b);
        draw_gpoly(&point_j, &point_l, &point_c);
        draw_gpoly(&point_k, &point_b, &point_l);
        draw_gpoly(&point_c, &point_l, &point_h);
        draw_gpoly(&point_l, &point_b, &point_g);
        draw_gpoly(&point_l, &point_g, &point_h);
        draw_gpoly(&point_h, &point_g, &polygon_data->vertex_third);
        break;
    case 12:
        vec_mode = VM_SolidColor;
        vec_colour = (polygon_data->vertex_third.S + polygon_data->vertex_second.S + polygon_data->vertex_first.S) / 3 >> 16;
        trig(&polygon_data->vertex_first, &polygon_data->vertex_second, &polygon_data->vertex_third);
        break;
    case 13:
        vec_mode = VM_SolidColor;
        vec_colour = (polygon_data->vertex_third.S + polygon_data->vertex_second.S + polygon_data->vertex_first.S) / 3 >> 16;
        coord_a.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_first.x) >> 1;
        coord_a.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_first.y) >> 1;
        coord_a.z = (polygon_data->coordinate_first.z + polygon_data->coordinate_second.z) >> 1;
        point_a.U = (polygon_data->vertex_first.U + polygon_data->vertex_second.U) >> 1;
        point_a.V = (polygon_data->vertex_second.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_a, &point_a);
        trig(&polygon_data->vertex_first, &point_a, &polygon_data->vertex_third);
        trig(&point_a, &polygon_data->vertex_second, &polygon_data->vertex_third);
        break;
    case 14:
        vec_mode = VM_SolidColor;
        vec_colour = (polygon_data->vertex_third.S + polygon_data->vertex_second.S + polygon_data->vertex_first.S) / 3 >> 16;
        coord_a.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_third.x) >> 1;
        coord_a.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_third.y) >> 1;
        coord_a.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_second.z) >> 1;
        point_a.U = (polygon_data->vertex_third.U + polygon_data->vertex_second.U) >> 1;
        point_a.V = (polygon_data->vertex_third.V + polygon_data->vertex_second.V) >> 1;
        perspective(&coord_a, &point_a);
        trig(&polygon_data->vertex_first, &polygon_data->vertex_second, &point_a);
        trig(&polygon_data->vertex_first, &point_a, &polygon_data->vertex_third);
        break;
    case 15:
        vec_mode = VM_SolidColor;
        vec_colour = (polygon_data->vertex_third.S + polygon_data->vertex_second.S + polygon_data->vertex_first.S) / 3 >> 16;
        coord_a.x = (polygon_data->coordinate_first.x + polygon_data->coordinate_third.x) >> 1;
        coord_a.y = (polygon_data->coordinate_third.y + polygon_data->coordinate_first.y) >> 1;
        coord_a.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_first.z) >> 1;
        point_a.U = (polygon_data->vertex_first.U + polygon_data->vertex_third.U) >> 1;
        point_a.V = (polygon_data->vertex_third.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_a, &point_a);
        trig(&polygon_data->vertex_first, &polygon_data->vertex_second, &point_a);
        trig(&point_a, &polygon_data->vertex_second, &polygon_data->vertex_third);
        break;
    case 16:
        vec_mode = VM_SolidColor;
        vec_colour = (polygon_data->vertex_third.S + polygon_data->vertex_second.S + polygon_data->vertex_first.S) / 3 >> 16;
        coord_a.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_first.x) >> 1;
        coord_a.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_first.y) >> 1;
        coord_a.z = (polygon_data->coordinate_first.z + polygon_data->coordinate_second.z) >> 1;
        point_a.U = (polygon_data->vertex_first.U + polygon_data->vertex_second.U) >> 1;
        point_a.V = (polygon_data->vertex_second.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_third.x) >> 1;
        coord_b.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_third.y) >> 1;
        coord_b.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_second.z) >> 1;
        point_b.U = (polygon_data->vertex_third.U + polygon_data->vertex_second.U) >> 1;
        point_b.V = (polygon_data->vertex_third.V + polygon_data->vertex_second.V) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (polygon_data->coordinate_first.x + polygon_data->coordinate_third.x) >> 1;
        coord_c.y = (polygon_data->coordinate_third.y + polygon_data->coordinate_first.y) >> 1;
        coord_c.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_first.z) >> 1;
        point_c.U = (polygon_data->vertex_first.U + polygon_data->vertex_third.U) >> 1;
        point_c.V = (polygon_data->vertex_third.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_c, &point_c);
        trig(&polygon_data->vertex_first, &point_a, &point_c);
        trig(&point_a, &polygon_data->vertex_second, &point_b);
        trig(&point_a, &point_b, &point_c);
        trig(&point_c, &point_b, &polygon_data->vertex_third);
        break;
    case 17:
        vec_mode = VM_SolidColor;
        vec_colour = (polygon_data->vertex_third.S + polygon_data->vertex_second.S + polygon_data->vertex_first.S) / 3 >> 16;
        coord_a.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_first.x) >> 1;
        coord_a.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_first.y) >> 1;
        coord_a.z = (polygon_data->coordinate_first.z + polygon_data->coordinate_second.z) >> 1;
        point_a.U = (polygon_data->vertex_first.U + polygon_data->vertex_second.U) >> 1;
        point_a.V = (polygon_data->vertex_second.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (coord_a.x + polygon_data->coordinate_first.x) >> 1;
        coord_b.y = (coord_a.y + polygon_data->coordinate_first.y) >> 1;
        coord_b.z = (coord_a.z + polygon_data->coordinate_first.z) >> 1;
        point_b.U = (point_a.U + polygon_data->vertex_first.U) >> 1;
        point_b.V = (point_a.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (coord_a.x + polygon_data->coordinate_second.x) >> 1;
        coord_c.y = (coord_a.y + polygon_data->coordinate_second.y) >> 1;
        coord_c.z = (coord_a.z + polygon_data->coordinate_second.z) >> 1;
        point_c.U = (point_a.U + polygon_data->vertex_second.U) >> 1;
        point_c.V = (point_a.V + polygon_data->vertex_second.V) >> 1;
        perspective(&coord_c, &point_c);
        trig(&polygon_data->vertex_first, &point_b, &polygon_data->vertex_third);
        trig(&point_b, &point_a, &polygon_data->vertex_third);
        trig(&point_a, &point_c, &polygon_data->vertex_third);
        trig(&point_c, &polygon_data->vertex_second, &polygon_data->vertex_third);
        break;
    case 18:
        vec_mode = VM_SolidColor;
        vec_colour = (polygon_data->vertex_third.S + polygon_data->vertex_second.S + polygon_data->vertex_first.S) / 3 >> 16;
        coord_a.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_third.x) >> 1;
        coord_a.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_third.y) >> 1;
        coord_a.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_second.z) >> 1;
        point_a.U = (polygon_data->vertex_third.U + polygon_data->vertex_second.U) >> 1;
        point_a.V = (polygon_data->vertex_third.V + polygon_data->vertex_second.V) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (coord_a.x + polygon_data->coordinate_second.x) >> 1;
        coord_b.y = (coord_a.y + polygon_data->coordinate_second.y) >> 1;
        coord_b.z = (coord_a.z + polygon_data->coordinate_second.z) >> 1;
        point_b.U = (point_a.U + polygon_data->vertex_second.U) >> 1;
        point_b.V = (point_a.V + polygon_data->vertex_second.V) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (coord_a.x + polygon_data->coordinate_third.x) >> 1;
        coord_c.y = (coord_a.y + polygon_data->coordinate_third.y) >> 1;
        coord_c.z = (coord_a.z + polygon_data->coordinate_third.z) >> 1;
        point_c.U = (point_a.U + polygon_data->vertex_third.U) >> 1;
        point_c.V = (point_a.V + polygon_data->vertex_third.V) >> 1;
        perspective(&coord_c, &point_c);
        trig(&polygon_data->vertex_first, &polygon_data->vertex_second, &point_b);
        trig(&polygon_data->vertex_first, &point_b, &point_a);
        trig(&polygon_data->vertex_first, &point_a, &point_c);
        trig(&polygon_data->vertex_first, &point_c, &polygon_data->vertex_third);
        break;
    case 19:
        vec_mode = VM_SolidColor;
        vec_colour = (polygon_data->vertex_third.S + polygon_data->vertex_second.S + polygon_data->vertex_first.S) / 3 >> 16;
        coord_a.x = (polygon_data->coordinate_first.x + polygon_data->coordinate_third.x) >> 1;
        coord_a.y = (polygon_data->coordinate_third.y + polygon_data->coordinate_first.y) >> 1;
        coord_a.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_first.z) >> 1;
        point_a.U = (polygon_data->vertex_first.U + polygon_data->vertex_third.U) >> 1;
        point_a.V = (polygon_data->vertex_third.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (coord_a.x + polygon_data->coordinate_third.x) >> 1;
        coord_b.y = (coord_a.y + polygon_data->coordinate_third.y) >> 1;
        coord_b.z = (coord_a.z + polygon_data->coordinate_third.z) >> 1;
        point_b.U = (point_a.U + polygon_data->vertex_third.U) >> 1;
        point_b.V = (point_a.V + polygon_data->vertex_third.V) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (coord_a.x + polygon_data->coordinate_first.x) >> 1;
        coord_c.y = (coord_a.y + polygon_data->coordinate_first.y) >> 1;
        coord_c.z = (coord_a.z + polygon_data->coordinate_first.z) >> 1;
        point_c.U = (point_a.U + polygon_data->vertex_first.U) >> 1;
        point_c.V = (point_a.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_c, &point_c);
        trig(&polygon_data->vertex_second, &polygon_data->vertex_third, &point_b);
        trig(&polygon_data->vertex_second, &point_b, &point_a);
        trig(&polygon_data->vertex_second, &point_a, &point_c);
        trig(&polygon_data->vertex_second, &point_c, &polygon_data->vertex_first);
        break;
    case 20:
        vec_mode = VM_SolidColor;
        vec_colour = (polygon_data->vertex_third.S + polygon_data->vertex_second.S + polygon_data->vertex_first.S) / 3 >> 16;
        coord_a.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_first.x) >> 1;
        coord_a.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_first.y) >> 1;
        coord_a.z = (polygon_data->coordinate_first.z + polygon_data->coordinate_second.z) >> 1;
        point_a.U = (polygon_data->vertex_first.U + polygon_data->vertex_second.U) >> 1;
        point_a.V = (polygon_data->vertex_second.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_third.x) >> 1;
        coord_b.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_third.y) >> 1;
        coord_b.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_second.z) >> 1;
        point_b.U = (polygon_data->vertex_third.U + polygon_data->vertex_second.U) >> 1;
        point_b.V = (polygon_data->vertex_third.V + polygon_data->vertex_second.V) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (polygon_data->coordinate_first.x + polygon_data->coordinate_third.x) >> 1;
        coord_c.y = (polygon_data->coordinate_third.y + polygon_data->coordinate_first.y) >> 1;
        coord_c.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_first.z) >> 1;
        point_c.U = (polygon_data->vertex_first.U + polygon_data->vertex_third.U) >> 1;
        point_c.V = (polygon_data->vertex_third.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_a.x + polygon_data->coordinate_first.x) >> 1;
        coord_d.y = (coord_a.y + polygon_data->coordinate_first.y) >> 1;
        coord_d.z = (coord_a.z + polygon_data->coordinate_first.z) >> 1;
        point_d.U = (point_a.U + polygon_data->vertex_first.U) >> 1;
        point_d.V = (point_a.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_a.x + polygon_data->coordinate_second.x) >> 1;
        coord_e.y = (coord_a.y + polygon_data->coordinate_second.y) >> 1;
        coord_e.z = (coord_a.z + polygon_data->coordinate_second.z) >> 1;
        point_e.U = (point_a.U + polygon_data->vertex_second.U) >> 1;
        point_e.V = (point_a.V + polygon_data->vertex_second.V) >> 1;
        perspective(&coord_e, &point_e);
        trig(&polygon_data->vertex_first, &point_d, &point_c);
        trig(&point_d, &point_a, &point_c);
        trig(&point_a, &point_e, &point_b);
        trig(&point_e, &polygon_data->vertex_second, &point_b);
        trig(&point_a, &point_b, &point_c);
        trig(&point_c, &point_b, &polygon_data->vertex_third);
        break;
    case 21:
        vec_mode = VM_SolidColor;
        vec_colour = (polygon_data->vertex_third.S + polygon_data->vertex_second.S + polygon_data->vertex_first.S) / 3 >> 16;
        coord_a.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_first.x) >> 1;
        coord_a.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_first.y) >> 1;
        coord_a.z = (polygon_data->coordinate_first.z + polygon_data->coordinate_second.z) >> 1;
        point_a.U = (polygon_data->vertex_first.U + polygon_data->vertex_second.U) >> 1;
        point_a.V = (polygon_data->vertex_second.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_third.x) >> 1;
        coord_b.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_third.y) >> 1;
        coord_b.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_second.z) >> 1;
        point_b.U = (polygon_data->vertex_third.U + polygon_data->vertex_second.U) >> 1;
        point_b.V = (polygon_data->vertex_third.V + polygon_data->vertex_second.V) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (polygon_data->coordinate_first.x + polygon_data->coordinate_third.x) >> 1;
        coord_c.y = (polygon_data->coordinate_third.y + polygon_data->coordinate_first.y) >> 1;
        coord_c.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_first.z) >> 1;
        point_c.U = (polygon_data->vertex_first.U + polygon_data->vertex_third.U) >> 1;
        point_c.V = (polygon_data->vertex_third.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_b.x + polygon_data->coordinate_second.x) >> 1;
        coord_d.y = (coord_b.y + polygon_data->coordinate_second.y) >> 1;
        coord_d.z = (coord_b.z + polygon_data->coordinate_second.z) >> 1;
        point_d.U = (point_b.U + polygon_data->vertex_second.U) >> 1;
        point_d.V = (point_b.V + polygon_data->vertex_second.V) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_b.x + polygon_data->coordinate_third.x) >> 1;
        coord_e.y = (coord_b.y + polygon_data->coordinate_third.y) >> 1;
        coord_e.z = (coord_b.z + polygon_data->coordinate_third.z) >> 1;
        point_e.U = (point_b.U + polygon_data->vertex_third.U) >> 1;
        point_e.V = (point_b.V + polygon_data->vertex_third.V) >> 1;
        perspective(&coord_e, &point_e);
        trig(&polygon_data->vertex_first, &point_a, &point_c);
        trig(&point_a, &point_b, &point_c);
        trig(&point_a, &polygon_data->vertex_second, &point_d);
        trig(&point_a, &point_d, &point_b);
        trig(&point_c, &point_b, &point_e);
        trig(&point_c, &point_e, &polygon_data->vertex_third);
        break;
    case 22:
        vec_mode = VM_SolidColor;
        vec_colour = (polygon_data->vertex_third.S + polygon_data->vertex_second.S + polygon_data->vertex_first.S) / 3 >> 16;
        coord_a.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_first.x) >> 1;
        coord_a.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_first.y) >> 1;
        coord_a.z = (polygon_data->coordinate_first.z + polygon_data->coordinate_second.z) >> 1;
        point_a.U = (polygon_data->vertex_first.U + polygon_data->vertex_second.U) >> 1;
        point_a.V = (polygon_data->vertex_second.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_third.x) >> 1;
        coord_b.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_third.y) >> 1;
        coord_b.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_second.z) >> 1;
        point_b.U = (polygon_data->vertex_third.U + polygon_data->vertex_second.U) >> 1;
        point_b.V = (polygon_data->vertex_third.V + polygon_data->vertex_second.V) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (polygon_data->coordinate_first.x + polygon_data->coordinate_third.x) >> 1;
        coord_c.y = (polygon_data->coordinate_third.y + polygon_data->coordinate_first.y) >> 1;
        coord_c.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_first.z) >> 1;
        point_c.U = (polygon_data->vertex_first.U + polygon_data->vertex_third.U) >> 1;
        point_c.V = (polygon_data->vertex_third.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_c.x + polygon_data->coordinate_third.x) >> 1;
        coord_d.y = (coord_c.y + polygon_data->coordinate_third.y) >> 1;
        coord_d.z = (coord_c.z + polygon_data->coordinate_third.z) >> 1;
        point_d.U = (point_c.U + polygon_data->vertex_third.U) >> 1;
        point_d.V = (point_c.V + polygon_data->vertex_third.V) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_c.x + polygon_data->coordinate_first.x) >> 1;
        coord_e.y = (coord_c.y + polygon_data->coordinate_first.y) >> 1;
        coord_e.z = (coord_c.z + polygon_data->coordinate_first.z) >> 1;
        point_e.U = (point_c.U + polygon_data->vertex_first.U) >> 1;
        point_e.V = (point_c.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_e, &point_e);
        trig(&point_a, &polygon_data->vertex_second, &point_b);
        trig(&point_a, &point_b, &point_c);
        trig(&polygon_data->vertex_first, &point_a, &point_e);
        trig(&point_e, &point_a, &point_c);
        trig(&point_c, &point_b, &point_d);
        trig(&point_d, &point_b, &polygon_data->vertex_third);
        break;
    case 23:
        vec_mode = VM_SolidColor;
        vec_colour = (polygon_data->vertex_third.S + polygon_data->vertex_second.S + polygon_data->vertex_first.S) / 3 >> 16;
        coord_a.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_first.x) >> 1;
        coord_a.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_first.y) >> 1;
        coord_a.z = (polygon_data->coordinate_first.z + polygon_data->coordinate_second.z) >> 1;
        point_a.U = (polygon_data->vertex_first.U + polygon_data->vertex_second.U) >> 1;
        point_a.V = (polygon_data->vertex_second.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (polygon_data->coordinate_second.x + polygon_data->coordinate_third.x) >> 1;
        coord_b.y = (polygon_data->coordinate_second.y + polygon_data->coordinate_third.y) >> 1;
        coord_b.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_second.z) >> 1;
        point_b.U = (polygon_data->vertex_third.U + polygon_data->vertex_second.U) >> 1;
        point_b.V = (polygon_data->vertex_third.V + polygon_data->vertex_second.V) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (polygon_data->coordinate_first.x + polygon_data->coordinate_third.x) >> 1;
        coord_c.y = (polygon_data->coordinate_third.y + polygon_data->coordinate_first.y) >> 1;
        coord_c.z = (polygon_data->coordinate_third.z + polygon_data->coordinate_first.z) >> 1;
        point_c.U = (polygon_data->vertex_first.U + polygon_data->vertex_third.U) >> 1;
        point_c.V = (polygon_data->vertex_third.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_a.x + polygon_data->coordinate_first.x) >> 1;
        coord_d.y = (coord_a.y + polygon_data->coordinate_first.y) >> 1;
        coord_d.z = (coord_a.z + polygon_data->coordinate_first.z) >> 1;
        point_d.U = (point_a.U + polygon_data->vertex_first.U) >> 1;
        point_d.V = (point_a.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_a.x + polygon_data->coordinate_second.x) >> 1;
        coord_e.y = (coord_a.y + polygon_data->coordinate_second.y) >> 1;
        coord_e.z = (coord_a.z + polygon_data->coordinate_second.z) >> 1;
        point_e.U = (point_a.U + polygon_data->vertex_second.U) >> 1;
        point_e.V = (point_a.V + polygon_data->vertex_second.V) >> 1;
        perspective(&coord_e, &point_e);
        coord_d.x = (coord_b.x + polygon_data->coordinate_second.x) >> 1;
        coord_d.y = (coord_b.y + polygon_data->coordinate_second.y) >> 1;
        coord_d.z = (coord_b.z + polygon_data->coordinate_second.z) >> 1;
        point_f.U = (point_b.U + polygon_data->vertex_second.U) >> 1;
        point_f.V = (point_b.V + polygon_data->vertex_second.V) >> 1;
        perspective(&coord_d, &point_f);
        coord_e.x = (coord_b.x + polygon_data->coordinate_third.x) >> 1;
        coord_e.y = (coord_b.y + polygon_data->coordinate_third.y) >> 1;
        coord_e.z = (coord_b.z + polygon_data->coordinate_third.z) >> 1;
        point_g.U = (point_b.U + polygon_data->vertex_third.U) >> 1;
        point_g.V = (point_b.V + polygon_data->vertex_third.V) >> 1;
        perspective(&coord_e, &point_g);
        coord_d.x = (coord_c.x + polygon_data->coordinate_third.x) >> 1;
        coord_d.y = (coord_c.y + polygon_data->coordinate_third.y) >> 1;
        coord_d.z = (coord_c.z + polygon_data->coordinate_third.z) >> 1;
        point_h.U = (point_c.U + polygon_data->vertex_third.U) >> 1;
        point_h.V = (point_c.V + polygon_data->vertex_third.V) >> 1;
        perspective(&coord_d, &point_h);
        coord_e.x = (coord_c.x + polygon_data->coordinate_first.x) >> 1;
        coord_e.y = (coord_c.y + polygon_data->coordinate_first.y) >> 1;
        coord_e.z = (coord_c.z + polygon_data->coordinate_first.z) >> 1;
        point_i.U = (point_c.U + polygon_data->vertex_first.U) >> 1;
        point_i.V = (point_c.V + polygon_data->vertex_first.V) >> 1;
        perspective(&coord_e, &point_i);
        coord_d.x = (coord_a.x + coord_c.x) >> 1;
        coord_d.y = (coord_a.y + coord_c.y) >> 1;
        coord_d.z = (coord_a.z + coord_c.z) >> 1;
        point_j.U = (point_a.U + point_c.U) >> 1;
        point_j.V = (point_a.V + point_c.V) >> 1;
        perspective(&coord_d, &point_j);
        coord_e.x = (coord_a.x + coord_b.x) >> 1;
        coord_e.y = (coord_a.y + coord_b.y) >> 1;
        coord_e.z = (coord_a.z + coord_b.z) >> 1;
        point_k.U = (point_a.U + point_b.U) >> 1;
        point_k.V = (point_a.V + point_b.V) >> 1;
        perspective(&coord_e, &point_k);
        coord_d.x = (coord_b.x + coord_c.x) >> 1;
        coord_d.y = (coord_b.y + coord_c.y) >> 1;
        coord_d.z = (coord_b.z + coord_c.z) >> 1;
        point_l.U = (point_b.U + point_c.U) >> 1;
        point_l.V = (point_b.V + point_c.V) >> 1;
        perspective(&coord_d, &point_l);
        trig(&polygon_data->vertex_first, &point_d, &point_i);
        trig(&point_d, &point_a, &point_j);
        trig(&point_a, &point_e, &point_k);
        trig(&point_e, &polygon_data->vertex_second, &point_f);
        trig(&point_d, &point_j, &point_i);
        trig(&point_a, &point_k, &point_j);
        trig(&point_e, &point_f, &point_k);
        trig(&point_i, &point_j, &point_c);
        trig(&point_j, &point_k, &point_l);
        trig(&point_k, &point_f, &point_b);
        trig(&point_j, &point_l, &point_c);
        trig(&point_k, &point_b, &point_l);
        trig(&point_c, &point_l, &point_h);
        trig(&point_l, &point_b, &point_g);
        trig(&point_l, &point_g, &point_h);
        trig(&point_h, &point_g, &polygon_data->vertex_third);
        break;
    default:
        render_problems++;
        render_prob_kind = polygon_data->b.kind;
        break;
    }

}
static void display_drawlist(void) // Draws isometric and 1st person view. Not frontview.
{
    struct PlayerInfo *player;
    const struct Camera *cam;
    union {
        struct BasicQ *b;
        struct BucketKindPolygonStandard *polygonStandard;
        struct BucketKindPolygonSimple *polygonSimple;
        struct BucketKindPolyMode0 *polyMode0;
        struct BucketKindPolyMode4 *polyMode4;
        struct BucketKindTrigMode2 *trigMode2;
        struct BucketKindPolyMode5 *polyMode5;
        struct BucketKindTrigMode3 *trigMode3;
        struct BucketKindTrigMode6 *trigMode6;
        struct BucketKindRotableSprite *rotableSprite;
        struct BucketKindPolygonNearFP *polygonNearFP;
        struct BucketKindBasicUnk10 *basicUnk10;
        struct BucketKindJontySprite *jontySprite;
        struct BucketKindCreatureShadow *creatureShadow;
        struct BucketKindSlabSelector *slabSelector;
        struct BucketKindCreatureStatus *creatureStatus;
        struct BucketKindTexturedQuad *texturedQuad;
        struct BucketKindFloatingGoldText *floatingGoldText;
        struct BucketKindRoomFlag *roomFlag;
    } item;
    long bucket_num;
    struct PolyPoint point_a;
    struct PolyPoint point_b;
    struct PolyPoint point_c;
    SYNCDBG(9,"Starting");
    // Color rendering array pointers used by draw_keepersprite()
    render_fade_tables = pixmap.fade_tables;
    render_ghost = pixmap.ghost;
    render_alpha = (unsigned char *)&alpha_sprite_table;
    render_problems = 0;
    thing_pointed_at = 0;

    // The bucket list is the final step in drawing something to the screen. Visuals are added to the bucket list in previous functions.
    for (bucket_num = BUCKETS_COUNT-1; bucket_num > 0; bucket_num--)
    {
        for (item.b = buckets[bucket_num]; item.b != NULL; item.b = item.b->next)
        {
            //JUSTLOG("%d",(int)item.b->kind);
            switch ( item.b->kind )
            {
            case QK_PolygonStandard: // All textured polygons for isometric and 'far' textures in 1st person view
                vec_mode = VM_QuadTextured;
                vec_map = block_ptrs[item.polygonStandard->block];
                draw_gpoly(&item.polygonStandard->vertex_first, &item.polygonStandard->vertex_second, &item.polygonStandard->vertex_third);
                break;
            case QK_PolygonSimple: // Possibly unused
                vec_mode = VM_SolidColor;
                vec_colour = ((item.polygonSimple->vertex_third.S + item.polygonSimple->vertex_second.S + item.polygonSimple->vertex_first.S)/3) >> 16;
                vec_map = block_ptrs[item.polygonSimple->block];
                trig(&item.polygonSimple->vertex_first, &item.polygonSimple->vertex_second, &item.polygonSimple->vertex_third);
                break;
            case QK_PolyMode0: // Possibly unused
                vec_mode = VM_FlatColor;
                vec_colour = item.polyMode0->colour;
                point_a.X = item.polyMode0->vertex_first_x;
                point_a.Y = item.polyMode0->vertex_first_y;
                point_b.X = item.polyMode0->vertex_second_x;
                point_b.Y = item.polyMode0->vertex_second_y;
                point_c.X = item.polyMode0->vertex_third_x;
                point_c.Y = item.polyMode0->vertex_third_y;
                draw_gpoly(&point_a, &point_b, &point_c);
                break;
            case QK_PolyMode4: // Possibly unused
                vec_mode = VM_QuadFlatColor;
                vec_colour = item.polyMode4->colour;
                point_a.X = item.polyMode4->vertex_first_x;
                point_a.Y = item.polyMode4->vertex_first_y;
                point_b.X = item.polyMode4->vertex_second_x;
                point_b.Y = item.polyMode4->vertex_second_y;
                point_c.X = item.polyMode4->vertex_third_x;
                point_c.Y = item.polyMode4->vertex_third_y;
                point_a.S = item.polyMode4->texture_vertex_first << 16;
                point_b.S = item.polyMode4->texture_vertex_second << 16;
                point_c.S = item.polyMode4->texture_vertex_third << 16;
                draw_gpoly(&point_a, &point_b, &point_c);
                break;
            case QK_TrigMode2: // Possibly unused
                vec_mode = VM_TriangularGouraud;
                point_a.X = item.trigMode2->vertex_first_x;
                point_a.Y = item.trigMode2->vertex_first_y;
                point_b.X = item.trigMode2->vertex_second_x;
                point_b.Y = item.trigMode2->vertex_second_y;
                point_c.X = item.trigMode2->vertex_third_x;
                point_c.Y = item.trigMode2->vertex_third_y;
                point_a.U = item.trigMode2->texture_u_first << 16;
                point_a.V = item.trigMode2->texture_v_first << 16;
                point_b.U = item.trigMode2->texture_u_second << 16;
                point_b.V = item.trigMode2->texture_v_second << 16;
                point_c.U = item.trigMode2->texture_u_third << 16;
                point_c.V = item.trigMode2->texture_v_third << 16;
                trig(&point_a, &point_b, &point_c);
                break;
            case QK_PolyMode5: // Possibly unused
                vec_mode = VM_QuadTextured;
                point_a.X = item.polyMode5->vertex_first_x;
                point_a.Y = item.polyMode5->vertex_first_y;
                point_b.X = item.polyMode5->vertex_second_x;
                point_b.Y = item.polyMode5->vertex_second_y;
                point_c.X = item.polyMode5->vertex_third_x;
                point_c.Y = item.polyMode5->vertex_third_y;
                point_a.U = item.polyMode5->texture_u_first << 16;
                point_a.V = item.polyMode5->texture_v_first << 16;
                point_b.U = item.polyMode5->texture_u_second << 16;
                point_b.V = item.polyMode5->texture_v_second << 16;
                point_c.U = item.polyMode5->texture_u_third << 16;
                point_c.V = item.polyMode5->texture_v_third << 16;
                point_a.S = item.polyMode5->texture_w_first << 16;
                point_b.S = item.polyMode5->texture_w_second << 16;
                point_c.S = item.polyMode5->texture_w_third << 16;
                draw_gpoly(&point_a, &point_b, &point_c);
                break;
            case QK_TrigMode3: // Possibly unused
                vec_mode = VM_TriangularTexture;
                point_a.X = item.trigMode3->vertex_first_x;
                point_a.Y = item.trigMode3->vertex_first_y;
                point_b.X = item.trigMode3->vertex_second_x;
                point_b.Y = item.trigMode3->vertex_second_y;
                point_c.X = item.trigMode3->vertex_third_x;
                point_c.Y = item.trigMode3->vertex_third_y;
                point_a.U = item.trigMode3->texture_u_first << 16;
                point_a.V = item.trigMode3->texture_v_first << 16;
                point_b.U = item.trigMode3->texture_u_second << 16;
                point_b.V = item.trigMode3->texture_v_second << 16;
                point_c.U = item.trigMode3->texture_u_third << 16;
                point_c.V = item.trigMode3->texture_v_third << 16;
                trig(&point_a, &point_b, &point_c);
                break;
            case QK_TrigMode6: // Possibly unused
                vec_mode = VM_TriangularTextured;
                point_a.X = item.trigMode6->vertex_first_x;
                point_a.Y = item.trigMode6->vertex_first_y;
                point_b.X = item.trigMode6->vertex_second_x;
                point_b.Y = item.trigMode6->vertex_second_y;
                point_c.X = item.trigMode6->vertex_third_x;
                point_c.Y = item.trigMode6->vertex_third_y;
                point_a.U = item.trigMode6->texture_u_first << 16;
                point_a.V = item.trigMode6->texture_v_first << 16;
                point_b.U = item.trigMode6->texture_u_second << 16;
                point_b.V = item.trigMode6->texture_v_second << 16;
                point_c.U = item.trigMode6->texture_u_third << 16;
                point_c.V = item.trigMode6->texture_v_third << 16;
                point_a.S = item.trigMode6->texture_w_first << 16;
                point_b.S = item.trigMode6->texture_w_second << 16;
                point_c.S = item.trigMode6->texture_w_third << 16;
                trig(&point_a, &point_b, &point_c);
                break;
            case QK_RotableSprite: // Possibly unused
                // draw_map_who did nothing
                break;
            case QK_PolygonNearFP: // 'Near' textured polygons (closer to camera) in 1st person view
                draw_subdivided_near_polygon(item.polygonNearFP);
                break;
            case QK_BasicPolygon:
                vec_mode = VM_FlatColor;
                vec_colour = item.basicUnk10->color_value;
                draw_gpoly(&item.basicUnk10->vertex_first, &item.basicUnk10->vertex_second, &item.basicUnk10->vertex_third);
                break;
            case QK_JontySprite: // All creatures and things in isometric and 1st person view
                draw_jonty_mapwho(item.jontySprite);
                break;
            case QK_CreatureShadow: // Shadows of creatures in isometric and 1st person view
                // TODO: this could be cached
                draw_keepsprite_unscaled_in_buffer(item.creatureShadow->anim_sprite, item.creatureShadow->angle, item.creatureShadow->current_frame, big_scratch);
                vec_map = big_scratch;
                vec_mode = VM_SpriteTranslucent;
                vec_colour = item.creatureShadow->vertex_first.S;
                trig(&item.creatureShadow->vertex_first, &item.creatureShadow->vertex_second, &item.creatureShadow->vertex_third);
                trig(&item.creatureShadow->vertex_first, &item.creatureShadow->vertex_third, &item.creatureShadow->vertex_fourth);
                break;
            case QK_SlabSelector: // Selection outline box for placing/digging slabs
                draw_clipped_line(
                    item.slabSelector->p.X,
                    item.slabSelector->p.Y,
                    item.slabSelector->p.U,
                    item.slabSelector->p.V,
                    item.slabSelector->p.S);
                break;
            case QK_CreatureStatus: // Status flower above creature heads
                draw_status_sprites(item.creatureStatus->x, item.creatureStatus->y, item.creatureStatus->thing);
                break;
            case QK_FloatingGoldText: // Floating gold text when placing or selling a slab
                draw_engine_number(item.floatingGoldText);
                break;
            case QK_RoomFlagBottomPole: // The bottom pole part, doesn't affect the status sitting on top of the pole
                draw_engine_room_flagpole(item.roomFlag);
                break;
            case QK_JontyISOSprite: // Spinning key
                player = get_my_player();
                cam = get_local_camera(player->acamera);
                if (cam != NULL)
                {
                    if (cam->view_mode == PVM_IsoWibbleView || cam->view_mode == PVM_IsoStraightView) {
                        draw_jonty_mapwho(item.jontySprite);
                    }
                }
                break;
            case QK_RoomFlagStatusBox: // The status sitting on top of the pole
                draw_engine_room_flag_top(item.roomFlag);
                break;
            default:
                render_problems++;
                render_prob_kind = item.b->kind;
                break;
            }
        }
    }
    if (render_problems > 0)
      WARNLOG("Incurred %lu rendering problems; last was with poly kind %ld",render_problems,render_prob_kind);
}

static void prepare_draw_plane_of_engine_columns(struct Camera *cam, long aposc, long bposc, long xcell, long ycell, struct MinMax *mm)
{
    apos = aposc;
    bpos = bposc;
    back_ec = &ecs1[0];
    front_ec = &ecs2[0];
    if (lens_mode != 0)
    {
        fill_in_points_perspective(cam, xcell, ycell, mm);
    } else
    if (settings.video_cluedo_mode)
    {
        fill_in_points_cluedo(cam, xcell, ycell, mm);
    } else
    {
        fill_in_points_isometric(cam, xcell, ycell, mm);
    }
}

/**
 * Draws single plane of engine columns.
 *
 * @param aposc
 * @param bposc
 * @param xcell
 * @param ycell
 */
static void draw_plane_of_engine_columns(struct Camera *cam, long aposc, long bposc, long xcell, long ycell, struct MinMax *mm)
{
    struct EngineCol *ec;
    ec = front_ec;
    front_ec = back_ec;
    back_ec = ec;
    apos = aposc;
    bpos = bposc;
    if (lens_mode != 0)
    {
        fill_in_points_perspective(cam, xcell, ycell, mm);
        if (mm->min < mm->max)
        {
          apos = aposc;
          bpos = bposc;
          do_a_plane_of_engine_columns_perspective(xcell, ycell, mm->min, mm->max);
        }
    } else
    if ( settings.video_cluedo_mode )
    {
        fill_in_points_cluedo(cam, xcell, ycell, mm);
        if (mm->min < mm->max)
        {
          apos = aposc;
          bpos = bposc;
          do_a_plane_of_engine_columns_cluedo(xcell, ycell, mm->min, mm->max);
        }
    } else
    {
        fill_in_points_isometric(cam, xcell, ycell, mm);
        if (mm->min < mm->max)
        {
          apos = aposc;
          bpos = bposc;
          do_a_plane_of_engine_columns_isometric(xcell, ycell, mm->min, mm->max);
        }
    }
}

/**
 * Draws rectangular area of engine columns.
 * @param aposc
 * @param bposc
 * @param xcell
 * @param ycell
 */
static void draw_view_map_plane(struct Camera *cam, long aposc, long bposc, long xcell, long ycell)
{
    struct MinMax *mm;
    long i;
    i = MINMAX_ALMOST_HALF-cells_away;
    if (i < 0)
        i = 0;
    mm = &minmaxs[i];
    prepare_draw_plane_of_engine_columns(cam, aposc, bposc, xcell, ycell, mm);

    for (i = 2*cells_away-1; i > 0; i--)
    {
        ycell++;
        bposc -= 256;
        mm++;
        draw_plane_of_engine_columns(cam, aposc, bposc, xcell, ycell, mm);
    }
}

void draw_view(struct Camera *cam, unsigned char a2)
{
    long zoom_mem;
    long xcell;
    long ycell;
    long i;
    long aposc;
    long bposc;
    SYNCDBG(9,"Starting");
    calculate_hud_scale(cam);
    camera_zoom = scale_camera_zoom_to_screen(cam->zoom);
    zoom_mem = cam->zoom;//TODO [zoom] remove when all cam->zoom will be changed to camera_zoom
    cam->zoom = camera_zoom;//TODO [zoom] remove when all cam->zoom will be changed to camera_zoom
    long x = cam->mappos.x.val;
    long y = cam->mappos.y.val;
    long z = cam->mappos.z.val;

    getpoly = poly_pool;
    memset(buckets, 0, sizeof(buckets));
    memset(poly_pool, 0, sizeof(poly_pool));
    if (map_volume_box.visible)
    {
        poly_pool_end_reserve(14);
    }
    else
    {
        poly_pool_end_reserve(4);
    }
    i = lens_mode;
    if ((i < 0) || (i >= PERS_ROUTINES_COUNT))
    {
        i = 0;
    }

    perspective = perspective_routines[i];
    rotpers = rotpers_routines[i];
    update_fade_limits(cells_away);
    init_coords_and_rotation(&object_origin,&camera_matrix);
    rotate_base_axis(&camera_matrix, cam->rotation_angle_x, 2);
    update_normal_shade(&camera_matrix);
    rotate_base_axis(&camera_matrix, -cam->rotation_angle_y, 1);
    rotate_base_axis(&camera_matrix, -cam->rotation_angle_z, 3);
    cam_map_angle = cam->rotation_angle_x;
    map_roll = cam->rotation_angle_z;
    map_tilt = -cam->rotation_angle_y;

    frame_wibble_generate();
    view_alt = z;
    if (lens_mode != 0)
    { // 1st person
        cells_away = max_i_can_see;
        update_fade_limits(cells_away);
        fade_range = (fade_max - fade_min) >> 8;
        setup_rotate_stuff(x, y, z, fade_max, fade_min, lens, cam_map_angle, map_roll);
    }
    else
    { // isometric and straight view
        fade_min = 1000000;
        setup_rotate_stuff(x, y, z, fade_max, fade_min, camera_zoom/pixel_size, cam_map_angle, map_roll);
        do_perspective_rotation(x, y, z);
        cells_away = compute_cells_away();
    }

    xcell = (x >> 8);
    aposc = -(x & 0xFF);
    bposc = (cells_away << 8) + (y & 0xFF);
    ycell = (y >> 8) - (cells_away+1);
    find_gamut();
    fiddle_gamut(xcell, ycell + (cells_away+1));

    draw_view_map_plane(cam, aposc, bposc, xcell, ycell);

    if ( (map_volume_box.visible) && (!game_is_busy_doing_gui()) )
    {
        poly_pool_end_reserve(0);
        process_isometric_map_volume_box(x, y, z, my_player_number);
    }

    display_drawlist();
    cam->zoom = zoom_mem;//TODO [zoom] remove when all cam->zoom will be changed to camera_zoom
    SYNCDBG(9,"Finished");
}

static void clear_fast_bucket_list(void)
{
    getpoly = poly_pool;
    memset(buckets, 0, sizeof(buckets));
}

static void draw_texturedquad_block(struct BucketKindTexturedQuad *txquad)
{
    struct PolyPoint point_a;
    struct PolyPoint point_b;
    struct PolyPoint point_c;
    struct PolyPoint point_d;
    vec_mode = VM_QuadTextured;
    switch (txquad->marked_mode) // Is visible/selected
    {
    case 0:
        vec_map = block_ptrs[TEXTURE_LAND_MARKED_LAND];
        break;
    case 1:
        vec_map = block_ptrs[TEXTURE_LAND_MARKED_GOLD];
        break;
    case 3:
    default:
        vec_map = block_ptrs[txquad->texture_idx];
        break;
    }
    point_a.X = (txquad->texture_x >> 8) / pixel_size;
    point_a.Y = (txquad->texture_y >> 8) / pixel_size;
    point_a.U = orient_to_mapU1[txquad->orient];
    point_a.V = orient_to_mapV1[txquad->orient];
    point_a.S = txquad->shade_intensity0;
    point_d.X = ((txquad->zoom_x + txquad->texture_x) >> 8) / pixel_size;
    point_d.Y = (txquad->texture_y >> 8) / pixel_size;
    point_d.U = orient_to_mapU2[txquad->orient];
    point_d.V = orient_to_mapV2[txquad->orient];
    point_d.S = txquad->shade_intensity1;
    point_b.X = ((txquad->zoom_x + txquad->texture_x) >> 8) / pixel_size;
    point_b.Y = ((txquad->zoom_y + txquad->texture_y) >> 8) / pixel_size;
    point_b.U = orient_to_mapU3[txquad->orient];
    point_b.V = orient_to_mapV3[txquad->orient];
    point_b.S = txquad->shade_intensity2;
    point_c.X = (txquad->texture_x >> 8) / pixel_size;
    point_c.Y = ((txquad->zoom_y + txquad->texture_y) >> 8) / pixel_size;
    point_c.U = orient_to_mapU4[txquad->orient];
    point_c.V = orient_to_mapV4[txquad->orient];
    point_c.S = txquad->shade_intensity3;
    draw_gpoly(&point_a, &point_d, &point_b);
    draw_gpoly(&point_a, &point_b, &point_c);
}

static void display_fast_drawlist(struct Camera *cam) // Draws frontview only. Not isometric or 1st person view.
{
    int bucket_num;
    union {
        struct BasicQ *b;
        // Unused in display_fast_drawlist()
        struct BucketKindPolygonStandard *polygonStandard;
        struct BucketKindPolygonSimple *polygonSimple;
        struct BucketKindPolyMode0 *polyMode0;
        struct BucketKindPolyMode4 *polyMode4;
        struct BucketKindTrigMode2 *trigMode2;
        struct BucketKindPolyMode5 *polyMode5;
        struct BucketKindTrigMode3 *trigMode3;
        struct BucketKindTrigMode6 *trigMode6;
        struct BucketKindRotableSprite *rotableSprite;
        struct BucketKindPolygonNearFP *polygonNearFP;
        struct BucketKindBasicUnk10 *basicUnk10;
        struct BucketKindCreatureShadow *creatureShadow;
        // Used
        struct BucketKindJontySprite *jontySprite;
        struct BucketKindSlabSelector *slabSelector;
        struct BucketKindCreatureStatus *creatureStatus;
        struct BucketKindTexturedQuad *texturedQuad;
        struct BucketKindFloatingGoldText *floatingGoldText;
        struct BucketKindRoomFlag *roomFlag;
    } item;
    // Color rendering array pointers used by draw_keepersprite()
    render_fade_tables = pixmap.fade_tables;
    render_ghost = pixmap.ghost;
    render_alpha = (unsigned char *)&alpha_sprite_table;
    render_problems = 0;
    thing_pointed_at = 0;

    for (bucket_num = BUCKETS_COUNT-1; bucket_num >= 0; bucket_num--)
    {
        for (item.b = buckets[bucket_num]; item.b != NULL; item.b = item.b->next)
        {
            switch (item.b->kind)
            {
            case QK_JontySprite: // Creatures and things
                draw_fastview_mapwho(cam, item.jontySprite);
                break;
            case QK_SlabSelector: // Selection outline box for placing/digging slabs
                draw_clipped_line(
                    item.slabSelector->p.X,
                    item.slabSelector->p.Y,
                    item.slabSelector->p.U,
                    item.slabSelector->p.V,
                    item.slabSelector->p.S);
                break;
            case QK_CreatureStatus: // Status flower above creature heads
                draw_status_sprites(item.creatureStatus->x, item.creatureStatus->y, item.creatureStatus->thing);
                break;
            case QK_TextureQuad: // Textured polygons
                draw_texturedquad_block(item.texturedQuad);
                break;
            case QK_FloatingGoldText: // Floating gold text when placing or selling a slab
                draw_engine_number(item.floatingGoldText);
                break;
            case QK_RoomFlagBottomPole: // The bottom pole part, doesn't affect the status sitting on top of the pole
                draw_engine_room_flagpole(item.roomFlag);
                break;
            case QK_JontyISOSprite: // Spinning Key
                draw_iso_only_fastview_mapwho(cam, item.jontySprite);
                break;
            case QK_RoomFlagStatusBox: // The status sitting on top of the pole
                draw_engine_room_flag_top(item.roomFlag);
                break;
            default:
                render_problems++;
                render_prob_kind = item.b->kind;
                break;
            }
        }
    } // end for(bucket_num...
    if (render_problems > 0) {
        WARNLOG("Incurred %lu rendering problems; last was with poly kind %ld",render_problems,render_prob_kind);
    }
}

/**
 * sub of convert_world_coord_to_front_view_screen_coord for a single point
 *
 * @param player The player determine the point for
 * @param zoom The zoom level of the camera
 * @param vertical_delta The vertical difference between the camera and the pos
 * @param horizontal_delta The horizontal difference between the camera and the pos
 * @return true if projected point is withing player's window, false otherwise
 */

#define PPH_EVEN_ALIGN_MASK 0xFFFE
static TbBool project_point_helper(struct PlayerInfo *player, int zoom, MapCoordDelta vertical_delta, MapCoordDelta horizontal_delta, MapCoord pos_z, int32_t *x_out, int32_t *y_out, int32_t *z_out)
{
    int vertical_shift;
    int64_t new_zoom;
    uint8_t offset;
    short window_width = player->engine_window_width;
    short window_height = player->engine_window_height;

    *x_out = (zoom * horizontal_delta >> 16) + (*(uint16_t *)&window_width / 2);
    vertical_shift = zoom * vertical_delta >> 8;
    *z_out = window_height - ((vertical_shift + ((uint16_t)(window_height & PPH_EVEN_ALIGN_MASK) << 7)) >> 8) + 64;
    new_zoom = (zoom * ((int16_t) pos_z)) << 7;
    offset = *((uint8_t *)&new_zoom + 4);
    *y_out = (vertical_shift + ((uint16_t)(window_height & PPH_EVEN_ALIGN_MASK) << 7) - ((offset + (signed int)new_zoom) >> 16)) >> 8;

    return (*x_out >= 0 && *x_out < window_width && *y_out >= 0 && *y_out < window_height);
}

/**
 * determines where on the screen an object should be drawn
 *
 * @param player The player determine the point for
 * @param cam The camera to use for the point
 * @param x_out The x position of the object relative to the camera
 * @param y_out The y position of the object relative to the camera
 * @param z_out The z position of the object relative to the camera
 * @return true if projected point is withing player's window, false otherwise
 */
static TbBool convert_world_coord_to_front_view_screen_coord(struct Coord3d* pos, struct Camera* cam, int32_t * x_out, int32_t * y_out, int32_t * z_out)
{
    int zoom;
    unsigned int orientation;
    int vertical_delta, horizontal_delta;
    long result = 0;
    struct PlayerInfo* player = get_my_player();

    zoom = 32 * camera_zoom / 256;
    orientation = ((unsigned int)(cam->rotation_angle_x + DEGREES_45) / DEGREES_90) & 3;

    switch ( orientation )
    {
        case 0:
            vertical_delta = pos->y.val - cam->mappos.y.val;
            horizontal_delta = pos->x.val - cam->mappos.x.val;
            result = project_point_helper(player, zoom, vertical_delta, horizontal_delta, pos->z.val, x_out, y_out, z_out);
            break;

        case 1:
            vertical_delta = cam->mappos.x.val - pos->x.val;
            horizontal_delta = pos->y.val - cam->mappos.y.val;
            result = project_point_helper(player, zoom, vertical_delta, horizontal_delta, pos->z.val, x_out, y_out, z_out);
            break;

        case 2:
            vertical_delta = cam->mappos.y.val - pos->y.val;
            horizontal_delta = cam->mappos.x.val - pos->x.val;
            result = project_point_helper(player, zoom, vertical_delta, horizontal_delta, pos->z.val, x_out, y_out, z_out);
            break;

        case 3:
            vertical_delta = pos->x.val - cam->mappos.x.val;
            horizontal_delta = cam->mappos.y.val - pos->y.val;
            result = project_point_helper(player, zoom, vertical_delta, horizontal_delta, pos->z.val, x_out, y_out, z_out);
            break;
    }

    return result;
}

static void add_thing_sprite_to_polypool(struct Thing *thing, long scr_x, long scr_y, long a4, long bckt_idx)
{
    struct BucketKindJontySprite *poly;
    if (bckt_idx >= BUCKETS_COUNT)
        bckt_idx = BUCKETS_COUNT-1;
    else
    if (bckt_idx < 0)
        bckt_idx = 0;
    poly = (struct BucketKindJontySprite *)getpoly;
    getpoly += sizeof(struct BucketKindJontySprite);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_JontySprite;
    buckets[bckt_idx] = (struct BasicQ *)poly;
    poly->thing = thing;
    if (pixel_size > 0)
    {
        poly->scr_x = scr_x / pixel_size;
        poly->scr_y = scr_y / pixel_size;
    }
    poly->depth_fade = a4;
}

static void add_spinning_key_to_polypool(struct Thing *thing, long scr_x, long scr_y, long a4, long bckt_idx)
{
    struct BucketKindJontySprite *poly;
    if (bckt_idx >= BUCKETS_COUNT)
      bckt_idx = BUCKETS_COUNT-1;
    else
    if (bckt_idx < 0)
      bckt_idx = 0;
    poly = (struct BucketKindJontySprite *)getpoly;
    getpoly += sizeof(struct BucketKindJontySprite);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_JontyISOSprite;
    buckets[bckt_idx] = (struct BasicQ *)poly;
    poly->thing = thing;
    if (pixel_size > 0)
    {
      poly->scr_x = scr_x / pixel_size;
      poly->scr_y = scr_y / pixel_size;
    }
    poly->depth_fade = a4;
}

// Creature status flower above head in FrontView
static void create_status_box_element(struct Thing *thing, long a2, long a3, long a4, long bckt_idx) //
{
    struct BucketKindCreatureStatus *poly;
    if (bckt_idx >= BUCKETS_COUNT) {
      bckt_idx = BUCKETS_COUNT-1;
    } else
    if (bckt_idx < 0) {
      bckt_idx = 0;
    }
    poly = (struct BucketKindCreatureStatus *)getpoly;
    getpoly += sizeof(struct BucketKindCreatureStatus);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_CreatureStatus;
    buckets[bckt_idx] = (struct BasicQ *)poly;
    poly->thing = thing;
    if (pixel_size > 0)
    {
        poly->x = a2 / pixel_size;
        poly->y = a3 / pixel_size;
    }
    poly->z = a4;
}

static void add_textruredquad_to_polypool(long x, long y, long texture_idx, long zoom, long orient, long lightness, long marked_mode, long bckt_idx)
{
    struct BucketKindTexturedQuad *poly;
    if (bckt_idx >= BUCKETS_COUNT) {
      bckt_idx = BUCKETS_COUNT-1;
    } else if (bckt_idx < 0) {
      bckt_idx = 0;
    }
    poly = (struct BucketKindTexturedQuad *)getpoly;
    getpoly += sizeof(struct BucketKindTexturedQuad);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_TextureQuad;
    buckets[bckt_idx] = (struct BasicQ *)poly;

    poly->texture_idx = texture_idx;
    poly->texture_x = x;
    poly->texture_y = y;
    poly->zoom_x = zoom;
    poly->zoom_y = zoom;
    poly->orient = orient;
    poly->shade_intensity0 = lightness;
    poly->shade_intensity1 = lightness;
    poly->shade_intensity2 = lightness;
    poly->shade_intensity3 = lightness;
    poly->marked_mode = marked_mode;
}

static void add_lgttextrdquad_to_polypool(long x, long y, long texture_idx, long zoom_x, long zoom_y, long orient, long lg0, long lg1, long lg2, long lg3, long bckt_idx)
{
    struct BucketKindTexturedQuad *poly;
    if (bckt_idx >= BUCKETS_COUNT) {
      bckt_idx = BUCKETS_COUNT-1;
    } else if (bckt_idx < 0) {
      bckt_idx = 0;
    }
    poly = (struct BucketKindTexturedQuad *)getpoly;
    getpoly += sizeof(struct BucketKindTexturedQuad);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_TextureQuad;
    buckets[bckt_idx] = (struct BasicQ *)poly;

    poly->texture_idx = texture_idx;
    poly->texture_x = x;
    poly->texture_y = y;
    poly->zoom_x = zoom_x;
    poly->zoom_y = zoom_y;
    poly->orient = orient;
    poly->shade_intensity0 = lg0;
    poly->shade_intensity1 = lg1;
    poly->shade_intensity2 = lg2;
    poly->shade_intensity3 = lg3;
    poly->marked_mode = 3;
}

static void add_number_to_polypool(long x, long y, long number, long bckt_idx)
{
    struct BucketKindFloatingGoldText *poly;
    if (bckt_idx >= BUCKETS_COUNT) {
      bckt_idx = BUCKETS_COUNT-1;
    } else if (bckt_idx < 0) {
      bckt_idx = 0;
    }
    poly = (struct BucketKindFloatingGoldText *)getpoly;
    getpoly += sizeof(struct BucketKindFloatingGoldText);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_FloatingGoldText;
    buckets[bckt_idx] = (struct BasicQ *)poly;
    if (pixel_size > 0)
    {
      poly->x = x / pixel_size;
      poly->y = y / pixel_size;
    }
    poly->lvl = number;
}

static void add_room_flag_pole_to_polypool(long x, long y, long room_idx, long bckt_idx)
{
    struct BucketKindRoomFlag *poly;
    if (bckt_idx >= BUCKETS_COUNT) {
      bckt_idx = BUCKETS_COUNT-1;
    } else if (bckt_idx < 0) {
      bckt_idx = 0;
    }
    poly = (struct BucketKindRoomFlag *)getpoly;
    getpoly += sizeof(struct BucketKindRoomFlag);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_RoomFlagBottomPole;
    buckets[bckt_idx] = (struct BasicQ *)poly;
    if (pixel_size > 0)
    {
      poly->x = x / pixel_size;
      poly->y = y / pixel_size;
    }
    poly->lvl = room_idx;
}

static void add_room_flag_top_to_polypool(long x, long y, long room_idx, long bckt_idx)
{
    struct BucketKindRoomFlag *poly;
    if (bckt_idx >= BUCKETS_COUNT) {
      bckt_idx = BUCKETS_COUNT-1;
    } else if (bckt_idx < 0) {
      bckt_idx = 0;
    }
    poly = (struct BucketKindRoomFlag *)getpoly;
    getpoly += sizeof(struct BucketKindRoomFlag);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_RoomFlagStatusBox;
    buckets[bckt_idx] = (struct BasicQ *)poly;
    if (pixel_size > 0)
    {
      poly->x = x / pixel_size;
      poly->y = y / pixel_size;
    }
    poly->lvl = room_idx;
}

static void prepare_lightness_intensity_array(long stl_x, long stl_y, int32_t *arrp, long base_lightness)
{
    long i;
    long n;
    n = 4 * stl_x + 17 * stl_y;
    for (i=0; i < 9; i++)
    {
        long rndi;
        long nval;
        if ((base_lightness <= 256) || (base_lightness > 15872))
        {
            nval = base_lightness;
        } else
        {
            rndi = randomisors[n&RANDOMISORS_MASK];
            n++;
            nval = 32 * (rndi & 0x3F) + base_lightness - 256;
        }
        *arrp = nval << 8;
        arrp++;
    }
}

static void draw_element(struct Map *map, long lightness, long stl_x, long stl_y, long pos_x, long pos_y, long zoom, unsigned char qdrant, int32_t *ymax)
{
    struct PlayerInfo *myplyr;
    TbBool sibrevealed[3][3];
    struct CubeConfigStats *cube_config_stats;
    struct Map *mapblk;
    int32_t lightness_arr[4][9];
    long bckt_idx;
    long cube_itm;
    long delta_y;
    long tc; // top cube index
    long x;
    long y;
    long i;
    myplyr = get_my_player();
    cube_itm = (qdrant + 2) & 3;
    delta_y = (zoom << 7) / 256;
    bckt_idx = myplyr->engine_window_height - (pos_y >> 8) + 64;
    // Check if there's enough place to draw
    if (!is_free_space_in_poly_pool(8))
      return;

    // Prepare light intensity array

    for (y=0; y < 3; y++)
        for (x=0; x < 3; x++)
        {
            sibrevealed[y][x] = subtile_revealed(stl_x+x-1, stl_y+y-1, myplyr->id_number);
        }

    i = 0;
    if (sibrevealed[0][1] && sibrevealed[1][0] && sibrevealed[1][1] && sibrevealed[0][0])
        i = lightness;
    prepare_lightness_intensity_array(stl_x,stl_y,lightness_arr[(-qdrant) & 3],i);

    i = 0;
    if (sibrevealed[0][1] && sibrevealed[0][2] && sibrevealed[1][2] && sibrevealed[1][1])
        i = get_subtile_lightness(&game.lish,stl_x+1,stl_y);
    prepare_lightness_intensity_array(stl_x+1,stl_y,lightness_arr[(1-qdrant) & 3],i);

    i = 0;
    if (sibrevealed[1][0] && sibrevealed[1][1] && sibrevealed[2][0] && sibrevealed[2][1])
        i = get_subtile_lightness(&game.lish,stl_x,stl_y+1);
    prepare_lightness_intensity_array(stl_x,stl_y+1,lightness_arr[(-1-qdrant) & 3],i);

    i = 0;
    if (sibrevealed[2][2] && sibrevealed[1][2] && sibrevealed[1][1] && sibrevealed[2][1])
        i = get_subtile_lightness(&game.lish,stl_x+1,stl_y+1);
    prepare_lightness_intensity_array(stl_x+1,stl_y+1,lightness_arr[(-2-qdrant) & 3],i);

    // Get column to be drawn on the current subtile

    struct Column *col;
    if (map_block_revealed(map, my_player_number))
      i = get_mapblk_column_index(map);
    else
      i = game.unrevealed_column_idx;
    col = get_column(i);
    mapblk = get_map_block_at(stl_x, stl_y);
    unsigned short textr_idx;
    // Draw the columns base block

    if (*ymax > pos_y)
    {
      if ((col->floor_texture != 0) && (col->cubes[0] == 0))
      {
          *ymax = pos_y;
          textr_idx = engine_remap_texture_blocks(stl_x, stl_y, col->floor_texture);
          if ((mapblk->flags & SlbAtFlg_Unexplored) != 0)
          {
              add_textruredquad_to_polypool(pos_x, pos_y, textr_idx, zoom, 0,
                  2097152, 0, bckt_idx);
          } else
          {
              add_lgttextrdquad_to_polypool(pos_x, pos_y, textr_idx, zoom, zoom, 0,
                  lightness_arr[0][0], lightness_arr[1][0], lightness_arr[2][0], lightness_arr[3][0], bckt_idx);
          }
      }
    }

    // Draw the columns cubes

    y = zoom + pos_y;
    cube_config_stats = NULL;
    for (tc=0; tc < COLUMN_STACK_HEIGHT; tc++)
    {
      if (col->cubes[tc] == 0)
        break;
      y -= delta_y;
      cube_config_stats = get_cube_model_stats(col->cubes[tc]);
      if (*ymax > y)
      {
        *ymax = y;
        textr_idx = engine_remap_texture_blocks(stl_x, stl_y, cube_config_stats->texture_id[cube_itm]);
        add_lgttextrdquad_to_polypool(pos_x, y, textr_idx, zoom, delta_y, 0,
            lightness_arr[3][tc+1], lightness_arr[2][tc+1], lightness_arr[2][tc], lightness_arr[3][tc], bckt_idx);
      }
    }

    if (cube_config_stats != NULL)
    {
      i = y - zoom;
      if (*ymax > i)
      {
        *ymax = i;
        textr_idx = engine_remap_texture_blocks(stl_x, stl_y, cube_config_stats->texture_id[4]);
        if ((mapblk->flags & SlbAtFlg_TaggedValuable) != 0)
        {
          add_textruredquad_to_polypool(pos_x, i, textr_idx, zoom, qdrant, 2097152, 1, bckt_idx);
        } else
        if ((mapblk->flags & SlbAtFlg_Unexplored) != 0)
        {
          add_textruredquad_to_polypool(pos_x, i, textr_idx, zoom, qdrant, 2097152, 0, bckt_idx);
        } else
        {
          add_lgttextrdquad_to_polypool(pos_x, i, textr_idx, zoom, zoom, qdrant,
              lightness_arr[0][tc], lightness_arr[1][tc], lightness_arr[2][tc], lightness_arr[3][tc], bckt_idx);
        }
      }
    }

    // If there are still some solid cubes higher than tc
    if ((get_column_ceiling_filled_subtiles(col) != 0) && (col->solidmask > (1 << tc)))
    {
        // Find any top cube separated by empty space
        for (;tc < COLUMN_STACK_HEIGHT; tc++)
        {
            if (col->cubes[tc] != 0)
              break;
            y -= delta_y;
        }

        for (;tc < COLUMN_STACK_HEIGHT; tc++)
        {
            if (col->cubes[tc] == 0)
              break;
            y -= delta_y;
            cube_config_stats = get_cube_model_stats(col->cubes[tc]);
            if (*ymax > y)
            {
              textr_idx = engine_remap_texture_blocks(stl_x, stl_y, cube_config_stats->texture_id[cube_itm]);
              add_lgttextrdquad_to_polypool(pos_x, y, textr_idx, zoom, delta_y, 0,
                  lightness_arr[3][tc+1], lightness_arr[2][tc+1], lightness_arr[2][tc], lightness_arr[3][tc], bckt_idx);
            }
        }
        if (cube_config_stats != NULL)
        {
          i = y - zoom;
          if (*ymax > i)
          {
              textr_idx = engine_remap_texture_blocks(stl_x, stl_y, cube_config_stats->texture_id[4]);
            add_lgttextrdquad_to_polypool(pos_x, i, textr_idx, zoom, zoom, qdrant,
                lightness_arr[0][tc], lightness_arr[1][tc], lightness_arr[2][tc], lightness_arr[3][tc], bckt_idx);
          }
        }
    }

}

static unsigned short get_thing_shade(struct Thing* thing)
{
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    long minimum_lightness = game.conf.rules[thing->owner].game.thing_minimum_illumination << 8;
    long lgh[2][2]; // the dimensions are lgh[y][x]
    long shval;
    long fract_x;
    long fract_y;
    stl_x = thing->mappos.x.stl.num;
    stl_y = thing->mappos.y.stl.num;
    fract_x = thing->mappos.x.stl.pos;
    fract_y = thing->mappos.y.stl.pos;
    lgh[0][0] = get_subtile_lightness(&game.lish,stl_x,  stl_y);
    lgh[0][1] = get_subtile_lightness(&game.lish,stl_x+1,stl_y);
    lgh[1][0] = get_subtile_lightness(&game.lish,stl_x,  stl_y+1);
    lgh[1][1] = get_subtile_lightness(&game.lish,stl_x+1,stl_y+1);
    shval = (fract_x
        * (lgh[0][1] + (fract_y * (lgh[1][1] - lgh[0][1]) >> 8)
        - (lgh[0][0] + (fract_y * (lgh[1][0] - lgh[0][0]) >> 8))) >> 8)
        + (lgh[0][0] + (fract_y * (lgh[1][0] - lgh[0][0]) >> 8));
    if (shval < minimum_lightness)
    {
        shval += (minimum_lightness >>2);
        if (shval > minimum_lightness)
            shval = minimum_lightness;
    } else
    {
        // Max lightness value - make sure it won't exceed our limits
        if (shval > 64*256+255)
            shval = 64*256+255;
    }
    return shval;
}

static long load_single_frame(TbSpriteData *data_ptr, unsigned short kspr_idx)
{
    long nlength;
    nlength = creature_table[kspr_idx+1].DataOffset - creature_table[kspr_idx].DataOffset;
    *data_ptr = he_alloc(nlength);

    LbFileSeek(jty_file_handle, creature_table[kspr_idx].DataOffset, 0);
    LbFileRead(jty_file_handle, *data_ptr, nlength);

    keepsprite[kspr_idx] = data_ptr;
    return 1;
}

static long load_keepersprite_if_needed(unsigned short kspr_idx)
{
    int frame_num;
    int frame_count;
    struct KeeperSprite *kspr_arr;
    kspr_arr = &creature_table[kspr_idx];
    if (kspr_arr->Rotable) {
        frame_count = 5 * kspr_arr->FramesCount;
    } else {
        frame_count = kspr_arr->FramesCount;
    }
    for (frame_num=0; frame_num < frame_count; frame_num++)
    {
        TbSpriteData *sprite_data_ptr = &sprite_heap_handle[kspr_idx+frame_num];
        if ((*sprite_data_ptr) == NULL)
        {
            if (!load_single_frame(sprite_data_ptr, kspr_idx+frame_num))
            {
                return 0;
            }
        }
    }
    return 1;
}

static long heap_manage_keepersprite(unsigned short kspr_idx)
{
    long result;
    if (kspr_idx >= KEEPERSPRITE_ADD_OFFSET)
        return 1;
    result = load_keepersprite_if_needed(kspr_idx);
    return result;
}

static void draw_keepersprite(long x, long y, const struct KeeperSprite * kspr, long kspr_idx)
{
    if ((kspr_idx < 0)
        || ((kspr_idx >= KEEPSPRITE_LENGTH) && (kspr_idx < KEEPERSPRITE_ADD_OFFSET))
        || (kspr_idx > (KEEPERSPRITE_ADD_NUM + KEEPERSPRITE_ADD_OFFSET))) {
        WARNDBG(9,"Invalid KeeperSprite %ld at (%ld,%ld) size (%u,%u) alpha %d",
            kspr_idx, x, y, kspr->SWidth, kspr->SHeight, (int)EngineSpriteDrawUsingAlpha);
        return;
    }
    SYNCDBG(17,"Drawing %ld at (%ld,%ld) size (%u,%u) alpha %d",
        kspr_idx, x, y, kspr->SWidth, kspr->SHeight, (int)EngineSpriteDrawUsingAlpha);
    const long clipped_height = kspr->SHeight - water_source_cutoff;
    if (clipped_height <= 0) {
        return;
    }
    const TbSpriteData * sprite_data_ptr = NULL;
    if (kspr_idx >= 0) {
        if (kspr_idx >= KEEPERSPRITE_ADD_OFFSET) {
            if (kspr_idx - KEEPERSPRITE_ADD_OFFSET < KEEPERSPRITE_ADD_NUM) {
                sprite_data_ptr = &keepersprite_add[kspr_idx - KEEPERSPRITE_ADD_OFFSET];
            }
        } else if (kspr_idx < KEEPSPRITE_LENGTH) {
            sprite_data_ptr = keepsprite[kspr_idx];
        }
    }
    if (sprite_data_ptr == NULL || *sprite_data_ptr == NULL) {
        WARNDBG(9,"Unallocated KeeperSprite %ld can't be drawn at (%ld,%ld)",kspr_idx,x,y);
        return;
    }
    const struct TbSourceBuffer buffer = {
        *sprite_data_ptr,
        kspr->SWidth,
        clipped_height,
        kspr->SWidth,
    };
    if ( EngineSpriteDrawUsingAlpha ) {
        DrawAlphaSpriteUsingScalingData(x, y, &buffer);
    } else {
        LbSpriteDrawUsingScalingData(x, y, &buffer);
    }
    SYNCDBG(18,"Finished");
}

static void set_thing_pointed_at(struct Thing *thing)
{
    if (thing_pointed_at == NULL) {
        thing_pointed_at = thing;
    }
}

static void draw_single_keepersprite_omni_xflip(long kspos_x, long kspos_y, struct KeeperSprite *kspr, long kspr_idx, long scale)
{
    long src_dy = (long)kspr->FrameHeight;
    long src_dx = (long)kspr->FrameWidth;
    long x = src_dx - (long)kspr->FrameOffsW - (long)kspr->SWidth;
    long y = kspr->FrameOffsH;
    long sp_dy = (src_dy * scale) >> 5;
    long sp_dx = (src_dx * scale) >> 5;
    LbSpriteSetScalingData(kspos_x, kspos_y, src_dx, src_dy, sp_dx, sp_dy);
    if ( thing_being_displayed_is_creature )
    {
      if ( (pointer_x >= kspos_x) && (pointer_x <= sp_dx + kspos_x) )
      {
          if ( (pointer_y >= kspos_y) && (pointer_y <= sp_dy + kspos_y) )
          {
              set_thing_pointed_at(thing_being_displayed);
          }
      }
    }
    draw_keepersprite(x, y, kspr, kspr_idx);
}

static void draw_single_keepersprite_omni(long kspos_x, long kspos_y, struct KeeperSprite *kspr, long kspr_idx, long scale)
{
    long src_dy = (long)kspr->FrameHeight;
    long src_dx = (long)kspr->FrameWidth;
    long x = kspr->FrameOffsW;
    long y = kspr->FrameOffsH;
    long sp_dy = (src_dy * scale) >> 5;
    long sp_dx = (src_dx * scale) >> 5;
    LbSpriteSetScalingData(kspos_x, kspos_y, src_dx, src_dy, sp_dx, sp_dy);
    if ( thing_being_displayed_is_creature )
    {
      if ( (pointer_x >= kspos_x) && (pointer_x <= sp_dx + kspos_x) )
      {
          if ( (pointer_y >= kspos_y) && (pointer_y <= sp_dy + kspos_y) )
          {
              set_thing_pointed_at(thing_being_displayed);
          }
      }
    }
    draw_keepersprite(x, y, kspr, kspr_idx);
}

static void draw_single_keepersprite_xflip(long kspos_x, long kspos_y, struct KeeperSprite *kspr, long kspr_idx, long scale)
{
    SYNCDBG(18,"Starting");
    long src_dy = (long)kspr->SHeight;
    long src_dx = (long)kspr->SWidth;
    long x = (long)kspr->FrameWidth - (long)kspr->FrameOffsW - src_dx;
    long y = kspr->FrameOffsH;
    long sp_x = kspos_x + ((scale * x) >> 5);
    long sp_y = kspos_y + ((scale * y) >> 5);
    long sp_dy = (src_dy * scale) >> 5;
    long sp_dx = (src_dx * scale) >> 5;
    LbSpriteSetScalingData(sp_x, sp_y, src_dx, src_dy, sp_dx, sp_dy);
    if ( thing_being_displayed_is_creature )
    {
      if ( (pointer_x >= sp_x) && (pointer_x <= sp_dx + sp_x) )
      {
          if ( (pointer_y >= sp_y) && (pointer_y <= sp_dy + sp_y) )
          {
              set_thing_pointed_at(thing_being_displayed);
          }
      }
    }
    draw_keepersprite(0, 0, kspr, kspr_idx);
    SYNCDBG(18,"Finished");
}

static void draw_single_keepersprite(long kspos_x, long kspos_y, struct KeeperSprite *kspr, long kspr_idx, long scale)
{
    SYNCDBG(18,"Starting");
    long src_dy = (long)kspr->SHeight;
    long src_dx = (long)kspr->SWidth;
    long x = kspr->FrameOffsW;
    long y = kspr->FrameOffsH;
    long sp_x = kspos_x + ((scale * x) >> 5);
    long sp_y = kspos_y + ((scale * y) >> 5);
    long sp_dy = (src_dy * scale) >> 5;
    long sp_dx = (src_dx * scale) >> 5;
    LbSpriteSetScalingData(sp_x, sp_y, src_dx, src_dy, sp_dx, sp_dy);
    if ( thing_being_displayed_is_creature )
    {
        if ( (pointer_x >= x) && (pointer_x <= sp_dx + x) )
        {
            if ( (pointer_y >= y) && (pointer_y <= sp_dy + y) )
            {
                set_thing_pointed_at(thing_being_displayed);
            }
        }
    }
    draw_keepersprite(0, 0, kspr, kspr_idx);
    SYNCDBG(18,"Finished");
}

void process_keeper_sprite(short x, short y, unsigned short kspr_base, short kspr_angle, unsigned char sprgroup, long scale)
{
    struct KeeperSprite *creature_sprites;
    struct PlayerInfo *player;
    struct CreatureControl *cctrl;
    struct KeeperSprite *kspr;
    long kspr_idx;
    long draw_idx;
    short dim_ow;
    short dim_oh;
    short dim_th;
    short dim_tw;
    long scaled_x;
    long scaled_y;
    TbBool needs_xflip;
    long long lltemp;
    long sprite_group;
    long sprite_rot;
    long cutoff;
    SYNCDBG(17, "At (%d,%d) opts %d %d %d %d", (int)x, (int)y, (int)kspr_base, (int)kspr_angle, (int)sprgroup, (int)scale);
    player = get_my_player();
    creature_sprites = keepersprite_array(kspr_base);

    if (((kspr_angle & ANGLE_MASK) <= 1151) || ((kspr_angle & ANGLE_MASK) >= 1919) || (creature_sprites->Rotable != 2) )
        needs_xflip = 0;
    else
        needs_xflip = 1;

    if ( needs_xflip )
      lbDisplay.DrawFlags |= Lb_SPRITE_FLIP_HORIZ;
    else
      lbDisplay.DrawFlags &= ~Lb_SPRITE_FLIP_HORIZ;
    sprite_group = sprgroup;
    lltemp = 4 - ((((long)kspr_angle + DEGREES_22_5) & ANGLE_MASK) >> 8);
    sprite_rot = llabs(lltemp);
    kspr_idx = keepersprite_index(kspr_base);
    global_scaler = scale;
    if (needs_xflip)
    {
        scaled_x = ((long)x - ((scale * (long)(creature_sprites->FrameWidth + creature_sprites->offset_x)) >> 5));
    }
    else
    {
        scaled_x = ((scale * (long)creature_sprites->offset_x) >> 5) + (long)x;
    }
    scaled_y = ((scale * (long)creature_sprites->offset_y) >> 5) + (long)y;
    SYNCDBG(17,"Scaled (%d,%d)",(int)scaled_x,(int)scaled_y);
    if (thing_is_invalid(thing_being_displayed))
    {
        water_y_offset = 0;
        water_source_cutoff = 0;
    } else
    if ( ((thing_being_displayed->movement_flags & (TMvF_IsOnWater|TMvF_IsOnLava|TMvF_BeingSacrificed)) == 0) || (object_is_buoyant(thing_being_displayed)))
    {
        water_y_offset = 0;
        water_source_cutoff = 0;
    } else
    {
        cutoff = 6;
        if (creature_sprites->shadow_offset > (2 * cutoff))
        {
            cutoff = creature_sprites->shadow_offset / 2;
        }
        if ( (thing_being_displayed->movement_flags & TMvF_BeingSacrificed) != 0 )
        {
            get_keepsprite_unscaled_dimensions(thing_being_displayed->anim_sprite, thing_being_displayed->move_angle_xy, thing_being_displayed->current_frame, &dim_ow, &dim_oh, &dim_tw, &dim_th);
            cctrl = creature_control_get_from_thing(thing_being_displayed);
            lltemp = dim_oh * (48 - (long)cctrl->sacrifice.animation_counter);
            cutoff = ((((lltemp >> 24) & 0x1F) + (long)lltemp) >> 5) / 2;
        }
        if (player->view_mode == PVM_CreatureView)
        {
            water_source_cutoff = cutoff;
            water_y_offset = (2 * scale * cutoff) >> 5;
        } else
        {
            water_source_cutoff = 2 * cutoff;
            water_y_offset = (scale * cutoff) >> 5;
        }
    }
    scaled_y += water_y_offset;
    if (creature_sprites->Rotable == 0)
    {
        if (!heap_manage_keepersprite(kspr_idx))
        {
            return;
        }
        kspr = &creature_sprites[sprite_group];
        draw_idx = sprite_group + kspr_idx;
        if ( needs_xflip )
        {
            draw_single_keepersprite_omni_xflip(scaled_x, scaled_y, kspr, draw_idx, scale);
        } else
        {
            draw_single_keepersprite_omni(scaled_x, scaled_y, kspr, draw_idx, scale);
        }
    } else
    if (creature_sprites->Rotable == 2)
    {
        if (!heap_manage_keepersprite(kspr_idx))
        {
            return;
        }
        kspr = &creature_sprites[sprite_group + sprite_rot * (long)creature_sprites->FramesCount];
        draw_idx = sprite_group + sprite_rot * (long)kspr->FramesCount + kspr_idx;
        if ( needs_xflip )
        {
            draw_single_keepersprite_xflip(scaled_x, scaled_y, kspr, draw_idx, scale);
        } else
        {
            draw_single_keepersprite(scaled_x, scaled_y, kspr, draw_idx, scale);
        }
    }
}

static void prepare_jonty_remap_and_scale(int32_t *scale, const struct BucketKindJontySprite *jspr)
{
    long i;
    struct Thing *thing;
    long shade;
    long shade_factor;
    long fade;
    thing = jspr->thing;
    long minimum_lightness = game.conf.rules[thing->owner].game.thing_minimum_illumination << 8;
    if (lens_mode == 0)
    {
        fade = 65536;
        if ((thing->rendering_flags & TRF_Unshaded) == 0)
            i = get_thing_shade(thing);
        else
            i = minimum_lightness;
        shade = i;
    } else
    if (jspr->depth_fade <= lfade_min)
    {
        fade = jspr->depth_fade;
        if ((thing->rendering_flags & TRF_Unshaded) == 0)
            i = get_thing_shade(thing);
        else
            i = minimum_lightness;
        shade = i;
    } else
    if (jspr->depth_fade < lfade_max)
    {
        fade = jspr->depth_fade;
        if ((thing->rendering_flags & TRF_Unshaded) == 0)
            i = get_thing_shade(thing);
        else
            i = minimum_lightness;
        shade = i * (long long)(lfade_max - fade) / fade_mmm;
    } else
    {
        fade = jspr->depth_fade;
        shade = 0;
    }
    shade_factor = shade >> 8;
    *scale = (thelens * (long)thing->sprite_size) / fade;
    if ((thing->rendering_flags & (TRF_Tint_1|TRF_Tint_2)) != 0)
    {
        lbDisplay.DrawFlags |= Lb_TEXT_UNDERLNSHADOW;
        shade_factor = thing->tint_colour;
        lbSpriteReMapPtr = &pixmap.ghost[256 * shade_factor];
    } else
    if (shade_factor == 32)
    {
        lbDisplay.DrawFlags &= ~Lb_TEXT_UNDERLNSHADOW;
    } else
    {
        lbDisplay.DrawFlags |= Lb_TEXT_UNDERLNSHADOW;
        lbSpriteReMapPtr = &pixmap.fade_tables[256 * shade_factor];
    }
}

static void draw_mapwho_ariadne_path(struct Thing *thing)
{
    // Don't draw debug pathfinding lines in Possession to avoid crash
    struct PlayerInfo *player = get_my_player();
    if (player->view_mode == PVM_CreatureView)
        return;

    struct Ariadne *arid;
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        arid = &cctrl->arid;
    }
    SYNCDBG(16, "Starting for (%d,%d) to (%d,%d)", (int)arid->startpos.x.val, (int)arid->startpos.y.val, (int)arid->endpos.x.val, (int)arid->endpos.y.val);
    int i;
    struct Coord2d *wp_next;
    struct Coord2d *wp_prev;
    wp_prev = (struct Coord2d *)&arid->startpos;
    for (i = 0; i < arid->stored_waypoints; i++)
    {
        wp_next = &arid->waypoints[i];

        long beg_x;
        long end_x;
        long beg_y;
        long end_y;
        beg_x = (long)wp_prev->x.val - map_x_pos;
        end_x = (long)wp_next->x.val - map_x_pos;
        beg_y = map_y_pos - (long)wp_prev->y.val;
        end_y = map_y_pos - (long)wp_next->y.val;
        create_line_const_z(1, (long)arid->startpos.z.val + COORD_PER_STL / 16 - map_z_pos, beg_x, end_x, beg_y, end_y);
        wp_prev = wp_next;
    }
}

static void draw_jonty_mapwho(struct BucketKindJontySprite *jspr)
{
    unsigned short flg_mem;
    unsigned char alpha_mem;
    struct PlayerInfo *player = get_my_player();
    struct Thing *thing = jspr->thing;
    long angle;
    int32_t scaled_size;
    struct ObjectConfigStats* objst;
    flg_mem = lbDisplay.DrawFlags;
    alpha_mem = EngineSpriteDrawUsingAlpha;
    if (keepersprite_rotable(thing->anim_sprite))
    {
      angle = thing->move_angle_xy - spr_map_angle;
      angle += DEGREES_45 * (long)((thing->flags & TAF_ROTATED_MASK) >> TAF_ROTATED_SHIFT);
    }
    else
      angle = thing->move_angle_xy;
    prepare_jonty_remap_and_scale(&scaled_size, jspr);
    EngineSpriteDrawUsingAlpha = 0;
    switch (thing->rendering_flags & (TRF_Transpar_Flags))
    {
    case TRF_Transpar_8:
        lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR8;
        lbDisplay.DrawFlags &= ~Lb_TEXT_UNDERLNSHADOW;
        break;
    case TRF_Transpar_4:
        lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
        lbDisplay.DrawFlags &= ~Lb_TEXT_UNDERLNSHADOW;
        break;
    case TRF_Transpar_Alpha:
        EngineSpriteDrawUsingAlpha = 1;
        break;
    }

    if (!thing_is_invalid(thing))
    {
        if ((player->thing_under_hand == thing->index) && ((game.play_gameturn % (4 * gui_blink_rate)) >= 2 * gui_blink_rate))
        {
          if (player->acamera->view_mode == PVM_IsoWibbleView || player->acamera->view_mode == PVM_IsoStraightView)
          {
              lbDisplay.DrawFlags |= Lb_TEXT_UNDERLNSHADOW;
              lbSpriteReMapPtr = white_pal;
          }
          else if (player->acamera->view_mode == PVM_CreatureView)
          {
              struct Thing *creatng = thing_get(player->influenced_thing_idx);
              if (thing_is_creature(creatng))
              {
                  struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
                  struct Thing *dragtng = thing_get(cctrl->dragtng_idx);
                  if (!thing_exists(dragtng))
                  {
                    lbDisplay.DrawFlags |= Lb_TEXT_UNDERLNSHADOW;
                    lbSpriteReMapPtr = white_pal;
                  }
                  else if (thing_is_trap_crate(dragtng))
                  {
                      struct Thing *handthing = thing_get(player->thing_under_hand);
                      if (thing_exists(handthing))
                      {
                          if (handthing->class_id == TCls_Trap)
                          {
                              lbDisplay.DrawFlags |= Lb_TEXT_UNDERLNSHADOW;
                              lbSpriteReMapPtr = white_pal;
                          }
                      }
                  }
              }
          }
        } else {
            if ((thing->rendering_flags & TRF_BeingHit) != 0)
            {
                lbDisplay.DrawFlags |= Lb_TEXT_UNDERLNSHADOW;
                lbSpriteReMapPtr = red_pal;
                thing->time_spent_displaying_hurt_colour += game.delta_time;
                if (thing->time_spent_displaying_hurt_colour >= 1.0 || game.frame_skip > 0)
                {
                    thing->time_spent_displaying_hurt_colour = 0;
                    thing->rendering_flags &= ~TRF_BeingHit; // Turns off red damage colour tint
                }
            }
        }
        thing_being_displayed_is_creature = 1;
        thing_being_displayed = thing;
    } else
    {
        thing_being_displayed_is_creature = 0;
        thing_being_displayed = NULL;
    }
    if (render_sprite_debug_fn)
    {
        render_sprite_debug_fn(thing, jspr->scr_x, jspr->scr_y);
    }

    if (
        ((thing->anim_sprite >= CREATURE_FRAMELIST_LENGTH) && (thing->anim_sprite < KEEPERSPRITE_ADD_OFFSET))
        || (thing->anim_sprite >= KEEPERSPRITE_ADD_OFFSET + KEEPERSPRITE_ADD_NUM)
    )
    {
        ERRORLOG("Invalid graphic Id %d from model %d, class %d", (int)thing->anim_sprite, (int)thing->model, (int)thing->class_id);
    } else
    {
        struct TrapConfigStats *trapst;
        switch (thing->class_id)
        {
        case TCls_Object:
            objst = get_object_model_stats(thing->model);
            if (objst->flame.animation_id > 0)
            {
                process_keeper_flame_on_sprite(jspr, angle, scaled_size);
                break;
            }
            process_keeper_sprite(jspr->scr_x, jspr->scr_y, thing->anim_sprite, angle, thing->current_frame, scaled_size);
            break;
        case TCls_Trap:
            trapst = get_trap_model_stats(thing->model);
            if ((trapst->hidden == 1) && (player->id_number != thing->owner) && (thing->trap.revealed == 0))
            {
                break;
            }
            if ((trapst->flame.animation_id > 0) && (thing->trap.num_shots != 0))
            {
                process_keeper_flame_on_sprite(jspr, angle, scaled_size);
                break;
            }
            process_keeper_sprite(jspr->scr_x, jspr->scr_y, thing->anim_sprite, angle, thing->current_frame, scaled_size);
            break;
        default:
            process_keeper_sprite(jspr->scr_x, jspr->scr_y, thing->anim_sprite, angle, thing->current_frame, scaled_size);
            break;
        }
    }
    lbDisplay.DrawFlags = flg_mem;
    EngineSpriteDrawUsingAlpha = alpha_mem;
}

/** Fills solid area of the sprite in target buffer with color 255.
 *
 * @param sprdata Sprite data.
 * @param outbuf Output buffer to be filled.
 * @param lines_max Max lines to be written into output buffer.
 * @param scanln Length of scanline (length of line in output buffer).
 */
static void sprite_to_sbuff(const TbSpriteData sprdata, unsigned char *outbuf, int lines_max, int scanln)
{
    unsigned char *out_lnstart;
    unsigned char *out;
    int cval;
    const unsigned char *sprd;
    int i;
    sprd = sprdata;
    out = outbuf;
    out_lnstart = outbuf;
    cval = 0;
    while (lines_max > 0)
    {
      while ( 1 )
      {
          // Skip transparent area
          while ( 1 )
          {
              cval = *(char *)sprd;
              sprd++;
              if (cval >= 0)
                break;
              cval = -cval;
              out += cval;
          }
          if (cval == 0)
            break;
          sprd += cval;
          // Fill area per-byte until we get 32bit-aligned position
          while ((ptrdiff_t)out & 3)
          {
              *out = 0xFF;
              out++;
              cval--;
              if (cval <= 0)
                  break;
          }
          // Now fill the area faster - by writing 32-bit values
          for (i = (cval >> 2); i > 0; i--)
          {
              *(uint32_t *)out = 0xFFFFFFFF;
              out += 4;
          }
          // Fill the last unaligned bytes
          cval &= 3;
          while (cval > 0)
          {
            *out = 0xFF;
            out++;
            cval--;
          }
      }
      out_lnstart += scanln;
      out = out_lnstart;
      lines_max--;
    }
}

/** Fills solid area of the sprite in target buffer with color 255, x-flipping the sprite.
 *
 * @param sprdata Sprite data.
 * @param outbuf Output buffer to be filled.
 * @param lines_max Max lines to be written into output buffer.
 * @param scanln Length of scanline (length of line in output buffer).
 */
static void sprite_to_sbuff_xflip(const TbSpriteData sprdata, unsigned char *outbuf, int lines_max, int scanln)
{
    unsigned char *out_lnstart;
    unsigned char *out;
    int cval;
    const unsigned char *sprd;
    int i;
    sprd = sprdata;
    out = outbuf;
    out_lnstart = outbuf;
    cval = 0;
    while (lines_max > 0)
    {
      while ( 1 )
      {
          // Skip transparent area
          while ( 1 )
          {
              cval = *(char *)sprd;
              sprd++;
              if (cval >= 0)
                  break;
              cval = -cval;
              out -= cval;
          }
          if (cval == 0)
            break;
          sprd += cval;
          out++;
          // Fill area per-byte until we get 32bit-aligned position
          while ((uintptr_t)out & 3)
          {
              out--;
              *out = 0xFF;
              cval--;
              if (cval <= 0)
                  break;
          }
          // Now fill the area faster - by writing 32-bit values
          for (i = (cval >> 2); i > 0; i--)
          {
              out -= 4;
              *(uint32_t *)out = 0xFFFFFFFF;
          }
          out--;
          // Fill the last unaligned bytes
          cval &= 3;
          while (cval > 0)
          {
              *out = 0xFF;
              out--;
              cval--;
          }
      }
      out_lnstart += scanln;
      out = out_lnstart;
      lines_max--;
    }
}

static void draw_keepsprite_unscaled_in_buffer(unsigned short kspr_n, short angle, unsigned char current_frame, unsigned char *outbuf)
{
    struct KeeperSprite *kspr_arr;
    unsigned long kspr_idx;
    struct KeeperSprite *kspr;
    TbSpriteData sprite_data;
    unsigned int keepsprite_id;
    unsigned char *tmpbuf;
    int skip_w;
    int skip_h;
    int fill_w;
    int fill_h;
    TbBool flip_range;
    short quarter;
    int i;
    if ( ((angle & ANGLE_MASK) <= 1151) || ((angle & ANGLE_MASK) >= 1919) )
        flip_range = false;
    else
        flip_range = true;
    i = ((angle + DEGREES_22_5) & ANGLE_MASK);
    quarter = abs(4 - (i >> 8)); // i is restricted by "&" so (i>>8) is 0..7
    kspr_idx = keepersprite_index(kspr_n);
    kspr_arr = keepersprite_array(kspr_n);

    if (kspr_arr->Rotable == 0)
    {
        if (!heap_manage_keepersprite(kspr_idx))
        {
            return;
        }
        keepsprite_id = current_frame + kspr_idx;
        if (keepsprite_id >= KEEPERSPRITE_ADD_OFFSET)
        {
            sprite_data = keepersprite_add[keepsprite_id - KEEPERSPRITE_ADD_OFFSET];
        }
        else if (keepsprite_id >= KEEPSPRITE_LENGTH)
        {
            ERRORLOG("Sprite %d outside of valid range.", keepsprite_id);
            return;
        }
        else
        {
            sprite_data = *keepsprite[keepsprite_id];
        }
        kspr = &kspr_arr[current_frame];
        fill_w = kspr->FrameWidth;
        fill_h = kspr->FrameHeight;
        if ( flip_range )
        {
            tmpbuf = outbuf;
            skip_w = kspr->FrameWidth - kspr->FrameOffsW;
            skip_h = kspr->FrameOffsH;
            for (i = fill_h; i > 0; i--)
            {
                memset(tmpbuf, 0, fill_w);
                tmpbuf += 256;
            }
            sprite_to_sbuff_xflip(sprite_data, &outbuf[256 * skip_h + skip_w], kspr->SHeight, 256);
        }
        else
        {

            tmpbuf = outbuf;
            skip_w = kspr->FrameOffsW;
            skip_h = kspr->FrameOffsH;
            for (i = fill_h; i > 0; i--)
            {
                memset(tmpbuf, 0, fill_w);
                tmpbuf += 256;
            }
            sprite_to_sbuff(sprite_data, &outbuf[256 * skip_h + skip_w], kspr->SHeight, 256);
        }
    }
    else if (kspr_arr->Rotable == 2)
    {
        if (!heap_manage_keepersprite(kspr_idx))
        {
            return;
        }
        kspr = &kspr_arr[current_frame + quarter * kspr_arr->FramesCount];
        fill_w = kspr->SWidth;
        fill_h = kspr->SHeight;
        keepsprite_id = current_frame + quarter * kspr->FramesCount + kspr_idx;
        if (keepsprite_id >= KEEPERSPRITE_ADD_OFFSET)
        {
            sprite_data = keepersprite_add[keepsprite_id - KEEPERSPRITE_ADD_OFFSET];
        }
        else if (keepsprite_id >= KEEPSPRITE_LENGTH)
        {
            return; // WTF?!!
        }
        else
        {
            sprite_data = *keepsprite[keepsprite_id];
        }
        if ( flip_range )
        {
            tmpbuf = outbuf;
            for (i = fill_h; i > 0; i--)
            {
                memset(tmpbuf, 0, fill_w);
                tmpbuf += 256;
            }
            sprite_to_sbuff_xflip(sprite_data, &outbuf[kspr->SWidth], kspr->SHeight, 256);
        }
        else
        {
            tmpbuf = outbuf;
            for (i = fill_h; i > 0; i--)
            {
                memset(tmpbuf, 0, fill_w);
                tmpbuf += 256;
            }
            sprite_to_sbuff(sprite_data, &outbuf[0], kspr->SHeight, 256);
        }
    }
}

static void update_frontview_pointed_block(unsigned long laaa, unsigned char qdrant, long w, long h, long qx, long qy)
{
    TbGraphicsWindow ewnd;
    struct Column *colmn;
    unsigned long mask;
    struct Map *mapblk;
    long pos_x;
    long pos_y;
    long stl_x;
    long stl_y;
    long point_a;
    long point_b;
    long delta;
    long i;
    SYNCDBG(16,"Starting");
    store_engine_window(&ewnd,1);
    point_a = (((GetMouseX() - ewnd.x) << 8) - qx) << 8;
    point_b = (((GetMouseY() - ewnd.y) << 8) - qy) << 8;
    delta = (laaa << 7) / 256 << 8;
    for (i=0; i < 8; i++)
    {
        pos_x = (point_a / laaa) * x_step2[qdrant] + (point_b / laaa) * x_step1[qdrant] + (w << 8);
        pos_y = (point_a / laaa) * y_step2[qdrant] + (point_b / laaa) * y_step1[qdrant] + (h << 8);
        stl_x = (pos_x >> 8) + x_offs[qdrant];
        stl_y = (pos_y >> 8) + y_offs[qdrant];

        mapblk = get_map_block_at(stl_x, stl_y);
        if (!map_block_invalid(mapblk))
        {
          if (i == 0)
          {
            floor_pointed_at_x = stl_x;
            floor_pointed_at_y = stl_y;
            block_pointed_at_x = stl_x;
            block_pointed_at_y = stl_y;
            pointed_at_frac_x = pos_x & 0xFF;
            pointed_at_frac_y = pos_y & 0xFF;
            me_pointed_at = mapblk;
          } else
          {
            colmn = get_map_column(mapblk);
            mask = colmn->solidmask;
            if ( (1 << (i-1)) & mask )
            {
              pointed_at_frac_x = pos_x & 0xFF;
              pointed_at_frac_y = pos_y & 0xFF;
              block_pointed_at_x = stl_x;
              block_pointed_at_y = stl_y;
              me_pointed_at = mapblk;
            }
            if (((temp_cluedo_mode)  && (i == 2))
             || ((!temp_cluedo_mode) && (i == 5)))
            {
              top_pointed_at_frac_x = pos_x & 0xFF;
              top_pointed_at_frac_y = pos_y & 0xFF;
              top_pointed_at_x = stl_x;
              top_pointed_at_y = stl_y;
            }
          }
        }
        point_b += delta;
    }
}

void create_frontview_map_volume_box(struct Camera *cam, unsigned char stl_width, TbBool single_subtile, long line_color)
{
    unsigned char orient = ((unsigned int)(cam->rotation_angle_x + DEGREES_45) / DEGREES_90) & 0x03;
    // _depth_ is "how far in to the screen" the box goes - it will be the width/height of a slab
    // _breadth_ is usually the same as the depth (a single slab), but for single subtile selection, this will be the width/height of a subtile
    // (if we are dealing with a single subtile, breadth will be a third of the depth.)
    long depth = ((5 - map_volume_box.floor_height_z) * ((long)stl_width << 7) / 256);
    long breadth = depth / (single_subtile ? STL_PER_SLB : 1);
    struct Coord3d pos;
    int32_t coord_x;
    int32_t coord_y;
    int32_t coord_z;
    long box_width, box_height;
    pos.y.val = map_volume_box.end_y - box_lag_compensation_y;
    pos.x.val = map_volume_box.end_x - box_lag_compensation_x;
    pos.z.val = subtile_coord(5,0);
    convert_world_coord_to_front_view_screen_coord(&pos, cam, &coord_x, &coord_y, &coord_z);
    box_width = coord_x;
    box_height = coord_y;
    pos.y.val = map_volume_box.beg_y - box_lag_compensation_y;
    pos.x.val = map_volume_box.beg_x - box_lag_compensation_x;
    convert_world_coord_to_front_view_screen_coord(&pos, cam, &coord_x, &coord_y, &coord_z);
    box_width -= coord_x;
    box_height -= coord_y;
    box_width = abs(box_width);
    box_height = abs(box_height);
    switch ( orient )
    {
    //case 0: // North
    case 1: // East
        coord_y -= box_height;
        coord_z += box_height;
        break;
    case 2: // South
        coord_x -= box_width;
        coord_y -= box_height;
        coord_z += box_height;
        break;
    case 3: // West
        coord_x -= box_width;
        break;
    }
    coord_z -= (stl_width >> 1);
    // Draw 4 horizonal line elements
    create_line_element(coord_x,             coord_y,                      coord_x + box_width, coord_y,                      coord_z,                          line_color);
    create_line_element(coord_x,             coord_y + box_height,         coord_x + box_width, coord_y + box_height,         coord_z - box_height,             line_color);
    create_line_element(coord_x,             coord_y + depth,              coord_x + box_width, coord_y + depth,              coord_z,                          line_color);
    create_line_element(coord_x,             coord_y + box_height + depth, coord_x + box_width, coord_y + box_height + depth, coord_z - box_height,             line_color);
    // Now the lines at left and right
    create_line_element(coord_x,             coord_y,                      coord_x,             coord_y + box_height,         coord_z - box_height,             line_color);
    create_line_element(coord_x + box_width, coord_y,                      coord_x + box_width, coord_y + box_height,         coord_z - box_height,             line_color);
    create_line_element(coord_x,             coord_y + breadth,            coord_x,             coord_y + box_height + depth, coord_z - box_height + stl_width, line_color);
    create_line_element(coord_x + box_width, coord_y + breadth,            coord_x + box_width, coord_y + box_height + depth, coord_z - box_height + stl_width, line_color);
}
void create_fancy_frontview_map_volume_box(struct RoomSpace roomspace, struct Camera *cam, unsigned char stl_width, long color, TbBool show_outer_box)
{
    long line_color = color;
    if (show_outer_box)
    {
        line_color = map_volume_box.color; //  set the "inner" box color to the default colour (usually red/green)
    }
    unsigned char orient = ((unsigned int)(cam->rotation_angle_x + DEGREES_45) / DEGREES_90) & 0x03;
    int floor_height_z = (map_volume_box.floor_height_z == 0) ? 1 : map_volume_box.floor_height_z; // ignore "liquid height", and force it to "floor height". All fancy rooms are on the ground, and this ensures the boundboxes are drawn correctly. A different solution will be required if this function is used to draw fancy rooms over "liquid".
    long depth = ((5 - floor_height_z) * ((long)stl_width << 7) / 256);
    struct Coord3d pos;
    int32_t coord_x;
    int32_t coord_y;
    int32_t coord_z;
    long box_width, box_height;
    struct MapVolumeBox valid_slabs = map_volume_box;
    // get the 'accurate' roomspace shape instead of the outer box
    valid_slabs.beg_x = subtile_coord((roomspace.left * 3), 0);
    valid_slabs.beg_y = subtile_coord((roomspace.top * 3), 0);
    valid_slabs.end_x = subtile_coord((3*1) + (roomspace.right * 3), 0);
    valid_slabs.end_y = subtile_coord(((3*1) + roomspace.bottom * 3), 0);
    pos.y.val = valid_slabs.end_y - box_lag_compensation_y;
    pos.x.val = valid_slabs.end_x - box_lag_compensation_x;
    pos.z.val = subtile_coord(5,0);
    convert_world_coord_to_front_view_screen_coord(&pos, cam, &coord_x, &coord_y, &coord_z);
    box_width = coord_x;
    box_height = coord_y;
    pos.y.val = valid_slabs.beg_y - box_lag_compensation_y;
    pos.x.val = valid_slabs.beg_x - box_lag_compensation_x;
    convert_world_coord_to_front_view_screen_coord(&pos, cam, &coord_x, &coord_y, &coord_z);
    box_width -= coord_x;
    box_height -= coord_y;
    box_width = abs(box_width);
    box_height = abs(box_height);
    int room_slab_width = roomspace.width;
    int room_slab_height = roomspace.height;
    if (orient % 2 == 1)
    {
        room_slab_width = roomspace.height;
        room_slab_height = roomspace.width;
    }
    TbBool rotated_roomspace[MAX_ROOMSPACE_WIDTH][MAX_ROOMSPACE_WIDTH];
    memcpy(rotated_roomspace,roomspace.slab_grid, sizeof(rotated_roomspace));
    int i, j;
    switch ( orient )
    {
    //case 0: // North
    case 1: // East
        coord_y -= box_height;
        coord_z += box_height;
        for (i = 0; i < roomspace.width; i++)
        {
            for (j = 0; j < roomspace.height; j++)
            {
                rotated_roomspace[j][i] = roomspace.slab_grid[roomspace.width - 1 - i][j];
            }
        }
        break;
    case 2: // South
        coord_x -= box_width;
        coord_y -= box_height;
        coord_z += box_height;
        for (i = 0; i < roomspace.width; i++)
        {
            for (j = 0; j < roomspace.height; j++)
            {
                rotated_roomspace[i][j]  = roomspace.slab_grid[roomspace.width - 1 - i][roomspace.height - 1 - j];
            }
        }
        break;
    case 3: // West
        coord_x -= box_width;
        for (i = 0; i < roomspace.width; i++)
        {
            for (j = 0; j < roomspace.height; j++)
            {
                rotated_roomspace[j][i] = roomspace.slab_grid[i][roomspace.height - 1 - j];
            }
        }
        break;
    }
    coord_z -= (stl_width >> 1);
    for (int roomspace_y = 0; roomspace_y < room_slab_height; roomspace_y += 1)
    {
        int y_start = (box_height * roomspace_y       / room_slab_height) + ((((box_height * roomspace_y)       % room_slab_height) >= room_slab_height) ? 1 : 0);
        int y_end =   (box_height * (roomspace_y + 1) / room_slab_height) + ((((box_height * (roomspace_y + 1)) % room_slab_height) >= room_slab_height) ? 1 : 0);
        int bckt_idx = coord_z - y_end;
        for (int roomspace_x = 0; roomspace_x < room_slab_width; roomspace_x += 1)
        {
            int x_start = (box_width * roomspace_x       / room_slab_width) + ((((box_width * roomspace_x)       % room_slab_width) >= room_slab_width) ? 1 : 0);
            int x_end =   (box_width * (roomspace_x + 1) / room_slab_width) + ((((box_width * (roomspace_x + 1)) % room_slab_width) >= room_slab_width) ? 1 : 0);
            TbBool is_in_roomspace = rotated_roomspace[roomspace_x][roomspace_y];
            if (is_in_roomspace)
            {
                TbBool air_left =  (roomspace_x == 0)                    ? true : (rotated_roomspace[roomspace_x-1][roomspace_y] == false);
                TbBool air_right = (roomspace_x == room_slab_width - 1)  ? true : (rotated_roomspace[roomspace_x+1][roomspace_y] == false);
                TbBool air_above = (roomspace_y == 0)                    ? true : (rotated_roomspace[roomspace_x][roomspace_y-1] == false);
                TbBool air_below = (roomspace_y == room_slab_height - 1) ? true : (rotated_roomspace[roomspace_x][roomspace_y+1] == false);
                if (air_left)
                {
                    create_line_element(    coord_x + x_start, coord_y + y_start,         coord_x + x_start, coord_y + y_end,           bckt_idx,             line_color);
                    if (air_below)
                    {
                        create_line_element(coord_x + x_start, coord_y + y_end,           coord_x + x_start, coord_y + y_end + depth,   bckt_idx + stl_width, line_color);
                    }
                }
                if (air_right)
                {
                    create_line_element(    coord_x + x_end,   coord_y + y_start,         coord_x + x_end,   coord_y + y_end,           bckt_idx,             line_color);
                    if (air_below)
                    {
                        create_line_element(coord_x + x_end,   coord_y + y_end,           coord_x + x_end,   coord_y + y_end + depth,   bckt_idx + stl_width, line_color);
                    }
                }
                if (air_above)
                {
                    create_line_element(    coord_x + x_start, coord_y + y_start,         coord_x + x_end,   coord_y + y_start,         bckt_idx,             line_color);
                    create_line_element(    coord_x + x_start, coord_y + y_start + depth, coord_x + x_end,   coord_y + y_start + depth, bckt_idx,             line_color);
                }
                if (air_below)
                {
                    create_line_element(    coord_x + x_start, coord_y + y_end,           coord_x + x_end,   coord_y + y_end,           bckt_idx,             line_color);
                    create_line_element(    coord_x + x_start, coord_y + y_end + depth,   coord_x + x_end,   coord_y + y_end + depth,   bckt_idx,             line_color);
                }
            }
            else if (!is_in_roomspace) //this handles "inside corners"
            {
                TbBool room_left =  (roomspace_x == 0)                    ? false : rotated_roomspace[roomspace_x-1][roomspace_y];
                TbBool room_right = (roomspace_x == room_slab_width - 1)  ? false : rotated_roomspace[roomspace_x+1][roomspace_y];
                TbBool room_below = (roomspace_y == room_slab_height - 1) ? false : rotated_roomspace[roomspace_x][roomspace_y+1];
                if (room_left)
                {
                    if (room_below)
                    {
                        create_line_element(coord_x + x_start,  coord_y + y_end,          coord_x + x_start, coord_y + y_end + depth,   bckt_idx,             line_color);
                    }
                }
                if (room_right)
                {
                    if (room_below)
                    {
                        create_line_element(coord_x + x_end,   coord_y + y_end,           coord_x + x_end,   coord_y + y_end + depth,   bckt_idx,             line_color);
                    }
                }
                if (show_outer_box) // this handles the "outer line" (only when it is not in the roomspace)
                {
                    //draw 2nd line, i.e. the outer line - the one around the edge of the 5x5 cursor, not the valid slabs within the cursor
                    line_color = color; // switch to the "secondary colour" (the one passed as a variable if show_outer_box is true)
                    TbBool left_edge   = (roomspace_x == 0)                    ? true : false;
                    TbBool right_edge  = (roomspace_x == room_slab_width - 1)  ? true : false;
                    TbBool top_edge    = (roomspace_y == 0)                    ? true : false;
                    TbBool bottom_edge = (roomspace_y == room_slab_height - 1) ? true : false;
                    if (left_edge)
                    {
                        create_line_element(    coord_x + x_start, coord_y + y_start,         coord_x + x_start, coord_y + y_end,           bckt_idx,             line_color);
                        if (bottom_edge)
                        {
                            create_line_element(coord_x + x_start, coord_y + y_end,           coord_x + x_start, coord_y + y_end + depth,   bckt_idx + stl_width, line_color);
                        }
                    }
                    if (right_edge)
                    {
                        create_line_element(    coord_x + x_end,   coord_y + y_start,         coord_x + x_end,   coord_y + y_end,           bckt_idx,             line_color);
                        if (bottom_edge)
                        {
                            create_line_element(coord_x + x_end,   coord_y + y_end,           coord_x + x_end,   coord_y + y_end + depth,   bckt_idx + stl_width, line_color);
                        }
                    }
                    if (top_edge)
                    {
                        create_line_element(    coord_x + x_start, coord_y + y_start,         coord_x + x_end,   coord_y + y_start,         bckt_idx,             line_color);
                        create_line_element(    coord_x + x_start, coord_y + y_start + depth, coord_x + x_end,   coord_y + y_start + depth, bckt_idx,             line_color);
                    }
                    if (bottom_edge)
                    {
                        create_line_element(    coord_x + x_start, coord_y + y_end,           coord_x + x_end,   coord_y + y_end,           bckt_idx,             line_color);
                        create_line_element(    coord_x + x_start, coord_y + y_end + depth,   coord_x + x_end,   coord_y + y_end + depth,   bckt_idx,             line_color);
                    }
                    line_color = map_volume_box.color; // switch back to default color (red/green) for the inner line
                }
            }
        }
    }
}

static void process_frontview_map_volume_box(struct Camera *cam, unsigned char stl_width, PlayerNumber plyr_idx)
{
    unsigned char default_color = map_volume_box.color;
    unsigned char line_color = default_color;
    struct PlayerInfo* current_player = get_player(plyr_idx);
    // Check if a roomspace is currently being built
    // and if so feed this back to the user
    if ((current_player->roomspace.is_active) && ((current_player->work_state == PSt_Sell) || (current_player->work_state == PSt_BuildRoom)))
    {
        line_color = SLC_REDYELLOW; // change the cursor color to indicate to the user that nothing else can be built or sold at the moment
    }
    if (current_player->render_roomspace.render_roomspace_as_box)
    {
        if (current_player->render_roomspace.is_roomspace_a_box)
        {
            // This is a basic square box
             create_frontview_map_volume_box(cam, stl_width, current_player->render_roomspace.is_roomspace_a_single_subtile, line_color);
        }
        else
        {
            // This is a "2-line" square box
            // i.e. an "accurate" box with an outer square box
            map_volume_box.color = line_color;
            create_fancy_frontview_map_volume_box(current_player->render_roomspace, cam, stl_width, (current_player->render_roomspace.slab_count == 0) ? SLC_RED : SLC_BROWN, true);
        }
    }
    else
    {
        // This is an "accurate"/"automagic" box
        create_fancy_frontview_map_volume_box(current_player->render_roomspace, cam, stl_width, line_color, false);
    }
    map_volume_box.color = default_color;
}

TbBool cursor_on_room(RoomIndex room_index)
{
    struct PlayerInfo* player = get_my_player();
    struct SlabMap* slb = get_slabmap_for_subtile(player->cursor_subtile_x, player->cursor_subtile_y);
    if (slabmap_block_invalid(slb)) {
        return false;
    }
    if (slb->room_index != room_index) {
        return false;
    }
    return true;
}
TbBool room_is_damaged(RoomIndex room_index)
{
    struct Room* room = room_get(room_index);
    if (room->health == compute_room_max_health(room->slabs_count, room->efficiency)) {
        return false;
    }
    return true;
}
TbBool placing_same_room_type(RoomIndex room_index)
{
    struct PlayerInfo* player = get_my_player();
    if (map_volume_box.visible == 0) {
        return false;
    }
    struct Room* room = room_get(room_index);
    if (player->chosen_room_kind != room->kind) {
        return false;
    }
    return true;
}

static void do_map_who_for_thing(struct Thing *thing)
{
    int bckt_idx;
    struct EngineCoord ecor;
    struct NearestLights nearlgt;

    interpolate_thing(thing);
    int render_pos_x, render_floorpos, render_pos_y, render_pos_z;
    render_pos_x = thing->interp_mappos.x.val;
    render_pos_y = thing->interp_mappos.z.val;
    render_pos_z = thing->interp_mappos.y.val;
    render_floorpos = thing->interp_floor_height;

    switch (thing->draw_class)
    {
    case ODC_Default:
        ecor.clip_flags = 0;
        ecor.x = (render_pos_x - map_x_pos);
        ecor.z = (map_y_pos - render_pos_z);
        ecor.y = (render_floorpos - map_z_pos); // For shadows

        // Shadows
        if (thing_is_creature(thing) && ((thing->movement_flags & TMvF_BeingSacrificed) == 0))
        {
            int count;
            int i;

            struct KeeperSprite *spr = keepersprite_array(thing->anim_sprite);
            if ((spr->frame_flags & FFL_NoShadows) == 0)
            {
                count = find_closest_lights(&thing->mappos, &nearlgt);
                for (i = 0; i < count; i++)
                {
                    create_shadows(thing, &ecor, &nearlgt.coord[i]);
                }
            }
        }
        // Height movement, falling or going up steps. This is applied after shadows, because shadows are always drawn at the floor height.
        ecor.y = (render_pos_y - map_z_pos);

        if (thing->class_id == TCls_Creature)
        {
            add_draw_status_box(thing, &ecor);
            // Draw path the creature is following
            if ((start_params.debug_flags & DFlg_CreatrPaths) != 0) {
                draw_mapwho_ariadne_path(thing);
            }
        }
        rotpers(&ecor, &camera_matrix);
        if (getpoly < poly_pool_end)
        {
            if ( lens_mode )
              bckt_idx = (ecor.z - 64) / 16;
            else
              bckt_idx = (ecor.z - 64) / 16 - 6;
            add_thing_sprite_to_polypool(thing, ecor.view_width, ecor.view_height, ecor.z, bckt_idx);
        }
        break;
    case ODC_DrawAtOrigin:
        ecor.clip_flags = 0;
        ecor.x = (render_pos_x - map_x_pos);
        ecor.z = (map_y_pos - render_pos_z);
        ecor.y = (render_pos_y - map_z_pos);
        memcpy(&object_origin, &ecor, sizeof(struct EngineCoord));
        object_origin.x = 0;
        object_origin.y = 0;
        object_origin.z = 0;
        break;
    case ODC_RoomPrice:
        ecor.x = (render_pos_x - map_x_pos);
        ecor.z = (map_y_pos - render_pos_z);
        ecor.y = (render_pos_y - map_z_pos);
        rotpers(&ecor, &camera_matrix);
        if (getpoly < poly_pool_end)
        {
            add_number_to_polypool(ecor.view_width, ecor.view_height, thing->price_effect.number, 1);
        }
        break;
    case ODC_RoomStatusFlag:
        // Hide status flags when full zoomed out, for atmospheric overview
        if (hud_scale == 0) {
            break;
        }

        RoomIndex flag_room_index = thing->lair.belongs_to;
        if (cursor_on_room(flag_room_index) == false && room_is_damaged(flag_room_index) == false && placing_same_room_type(flag_room_index) == false) {
            break;
        }

        ecor.x = (render_pos_x - map_x_pos);
        ecor.z = (map_y_pos - render_pos_z);
        ecor.y = (render_pos_y - map_z_pos);
        rotpers(&ecor, &camera_matrix);
        if (getpoly < poly_pool_end)
        {
            if (game.play_gameturn - thing->roomflag2.last_turn_drawn == 1)
            {
                if (thing->roomflag2.display_timer < 10) {
                    thing->roomflag2.display_timer++;
                }
            } else {
                if (game.play_gameturn - thing->roomflag2.last_turn_drawn > 1) {
                    thing->roomflag2.display_timer = 0;
                }
            }
            thing->roomflag2.last_turn_drawn = game.play_gameturn;
            if (thing->roomflag2.display_timer == 10)
            {
                bckt_idx = (ecor.z - 64) / 16 - 6;
                add_room_flag_pole_to_polypool(ecor.view_width, ecor.view_height, thing->roomflag.room_idx, bckt_idx);
                if (getpoly < poly_pool_end)
                {
                    add_room_flag_top_to_polypool(ecor.view_width, ecor.view_height, thing->roomflag.room_idx, 1);
                }
            }
        }
        break;
    case ODC_SpinningKey:
        ecor.x = (render_pos_x - map_x_pos);
        ecor.z = (map_y_pos - render_pos_z);
        ecor.y = (render_pos_y - map_z_pos);
        rotpers(&ecor, &camera_matrix);
        if (getpoly < poly_pool_end) {
            add_spinning_key_to_polypool(thing, ecor.view_width, ecor.view_height, ecor.z, 1);
        }
        break;
    default:
        break;
    }
    thing->last_turn_drawn = game.play_gameturn;
}

static void do_map_who(short tnglist_idx)
{
    long i;
    unsigned long k;
    k = 0;
    i = tnglist_idx;
    while (i != 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code start
        if ((thing->rendering_flags & TRF_Invisible) == 0)
        {
            do_map_who_for_thing(thing);
        }
        // Per thing code end
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            struct Map* mapblk = get_map_block_at(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
}

static void draw_frontview_thing_on_element(struct Thing *thing, struct Map *map, struct Camera *cam)
{
    // The draw_frontview_thing_on_element() function is the FrontView equivalent of do_map_who_for_thing()
    interpolate_thing(thing);

    int32_t cx;
    int32_t cy;
    int32_t cz;
    if ((thing->rendering_flags & TRF_Invisible) != 0)
        return;
    switch (thing->draw_class)
    {
    case ODC_Default: // Things
        convert_world_coord_to_front_view_screen_coord(&thing->interp_mappos,cam,&cx,&cy,&cz);
        if (is_free_space_in_poly_pool(1))
        {
            add_thing_sprite_to_polypool(thing, cx, cy, cy, cz-3);
            if ((thing->class_id == TCls_Creature) && is_free_space_in_poly_pool(1))
            {
                create_status_box_element(thing, cx, cy, cy, 1);
            }
        }
        break;
    case ODC_RoomPrice: // Floating gold text when buying and selling
        convert_world_coord_to_front_view_screen_coord(&thing->interp_mappos,cam,&cx,&cy,&cz);
        if (is_free_space_in_poly_pool(1))
        {
            add_number_to_polypool(cx, cy, thing->creature.gold_carried, 1);
        }
        break;
    case ODC_RoomStatusFlag: // Room Status flags
        // Hide status flags when full zoomed out, for atmospheric overview
        if (hud_scale == 0) {
            break;
        }

        RoomIndex flag_room_index = thing->lair.belongs_to;
        if (cursor_on_room(flag_room_index) == false && room_is_damaged(flag_room_index) == false && placing_same_room_type(flag_room_index) == false) {
            break;
        }

        convert_world_coord_to_front_view_screen_coord(&thing->interp_mappos,cam,&cx,&cy,&cz);
        if (is_free_space_in_poly_pool(1))
        {
            if (game.play_gameturn - thing->roomflag2.last_turn_drawn == 1)
            {
                if (thing->roomflag2.display_timer < 10) {
                    thing->roomflag2.display_timer++;
                }
            } else {
                if (game.play_gameturn - thing->roomflag2.last_turn_drawn > 1) {
                    thing->roomflag2.display_timer = 0;
                }
            }
            thing->roomflag2.last_turn_drawn = game.play_gameturn;
            if (thing->roomflag2.display_timer == 10)
            {
                add_room_flag_pole_to_polypool(cx, cy, thing->roomflag.room_idx, cz-3);
                if (is_free_space_in_poly_pool(1))
                {
                    add_room_flag_top_to_polypool(cx, cy, thing->roomflag.room_idx, 1);
                }
            }
        }
        break;
    case ODC_SpinningKey:
        convert_world_coord_to_front_view_screen_coord(&thing->interp_mappos,cam,&cx,&cy,&cz);
        if (is_free_space_in_poly_pool(1))
        {
            add_spinning_key_to_polypool(thing, cx, cy, cy, cz-3);
        }
        break;
    default:
        break;
    }
    thing->last_turn_drawn = game.play_gameturn;
}

static void draw_frontview_things_on_element(struct Map *mapblk, struct Camera *cam)
{
    struct Thing *thing;
    long i;
    unsigned long k;
    k = 0;
    i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        draw_frontview_thing_on_element(thing, mapblk, cam);
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
}

void draw_frontview_engine(struct Camera *cam)
{
    long zoom_mem;
    struct PlayerInfo *player;
    TbGraphicsWindow grwnd;
    TbGraphicsWindow ewnd;
    unsigned char qdrant;
    long px;
    long py;
    long qx;
    long qy;
    long w;
    long h;
    long pos_x;
    long pos_y;
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    long lim_x;
    long lim_y;
    long cam_x;
    long cam_y;
    long long zoom;
    long long lbbb;
    int32_t i;
    SYNCDBG(9,"Starting");
    player = get_my_player();
    if (cam->zoom > FRONTVIEW_CAMERA_ZOOM_MAX)
        cam->zoom = FRONTVIEW_CAMERA_ZOOM_MAX;
    calculate_hud_scale(cam);
    camera_zoom = scale_camera_zoom_to_screen(cam->zoom);
    zoom_mem = cam->zoom;//TODO [zoom] remove when all cam->zoom will be changed to camera_zoom
    cam->zoom = camera_zoom;//TODO [zoom] remove when all cam->zoom will be changed to camera_zoom
    cam_x = cam->mappos.x.val;
    cam_y = cam->mappos.y.val;
    pointer_x = (GetMouseX() - player->engine_window_x) / pixel_size;
    pointer_y = (GetMouseY() - player->engine_window_y) / pixel_size;
    LbScreenStoreGraphicsWindow(&grwnd);
    store_engine_window(&ewnd,pixel_size);
    LbScreenSetGraphicsWindow(ewnd.x, ewnd.y, ewnd.width, ewnd.height);
    setup_vecs(lbDisplay.GraphicsWindowPtr, NULL, lbDisplay.GraphicsScreenWidth, ewnd.width, ewnd.height);
    clear_fast_bucket_list();
    store_engine_window(&ewnd,1);
    setup_engine_window(ewnd.x, ewnd.y, ewnd.width, ewnd.height);
    qdrant = ((unsigned int)(cam->rotation_angle_x + DEGREES_45) / DEGREES_90) & 0x03;
    zoom = camera_zoom >> 3;
    w = (ewnd.width << 16) / zoom >> 1;
    h = (ewnd.height << 16) / zoom >> 1;
    switch (qdrant)
    {
    case 0:
        px = ((cam_x - w) >> 8);
        py = ((cam_y - h) >> 8);
        lbbb = cam_x - (px << 8);
        qx = (ewnd.width << 7)  - ((zoom * lbbb) >> 8);
        lbbb = cam_y - (py << 8);
        qy = (ewnd.height << 7) - ((zoom * lbbb) >> 8);
        break;
    case 1:
        px = ((cam_x + h) >> 8);
        py = ((cam_y - w) >> 8);
        lbbb = cam_y - (py << 8);
        qx = (ewnd.width << 7)  - ((zoom * lbbb) >> 8);
        lbbb = (px << 8) - cam_x;
        qy = (ewnd.height << 7) - ((zoom * lbbb) >> 8);
        px--;
        break;
    case 2:
        px = ((cam_x + w) >> 8) + 1;
        py = ((cam_y + h) >> 8);
        lbbb = (px << 8) - cam_x;
        qx = (ewnd.width << 7)  - ((zoom * lbbb) >> 8);
        lbbb = (py << 8) - cam_y;
        qy = (ewnd.height << 7) - ((zoom * lbbb) >> 8);
        px--;
        py--;
        break;
    case 3:
        px = ((cam_x - h) >> 8);
        py = ((cam_y + w) >> 8) + 1;
        lbbb = (py << 8) - cam_y;
        qx = (ewnd.width << 7)  - ((zoom * lbbb) >> 8);
        lbbb = cam_x - (px << 8);
        qy = (ewnd.height << 7) - ((zoom * lbbb) >> 8);
        py--;
        break;
    default:
        ERRORLOG("Illegal quadrant, %d.",qdrant);
        LbScreenLoadGraphicsWindow(&grwnd);
        return;
    }

    update_frontview_pointed_block(zoom, qdrant, px, py, qx, qy);
    if ( (map_volume_box.visible) && (!game_is_busy_doing_gui()) )
    {
        process_frontview_map_volume_box(cam, ((zoom >> 8) & 0xFF), player->id_number);
    }


    h = (8 * (zoom + 32 * ewnd.height) - qy) / zoom;
    w = (8 * (zoom + 32 * ewnd.height) - qy) / zoom;
    qy += zoom * h;
    px += x_step1[qdrant] * w;
    stl_x = x_step1[qdrant] * w + px;
    stl_y = y_step1[qdrant] * h + py;
    py += y_step1[qdrant] * h;
    lim_x = ewnd.width << 8;
    lim_y = -zoom;
    SYNCDBG(19,"Range (%ld,%ld) to (%ld,%ld), quadrant %d",px,py,qx,qy,(int)qdrant);
    for (pos_x=qx; pos_x < lim_x; pos_x += zoom)
    {
        i = (ewnd.height << 8);
        // Initialize the stl_? which will be swept by second loop
        if (x_step1[qdrant] != 0)
          stl_x = px;
        else
          stl_y = py;
        for (pos_y=qy; pos_y > lim_y; pos_y -= zoom)
        {
            struct Map *mapblk;
            mapblk = get_map_block_at(stl_x, stl_y);
            if (!map_block_invalid(mapblk))
            {
                if (get_mapblk_column_index(mapblk) > 0)
                {
                    draw_element(mapblk, get_subtile_lightness(&game.lish,stl_x,stl_y), stl_x, stl_y, pos_x, pos_y, zoom, qdrant, &i);
                }
                if ( subtile_revealed(stl_x, stl_y, player->id_number) )
                {
                    draw_frontview_things_on_element(mapblk, cam);
                }
            }
            stl_x -= x_step1[qdrant];
            stl_y -= y_step1[qdrant];
        }
        stl_x += x_step2[qdrant];
        stl_y += y_step2[qdrant];
    }

    display_fast_drawlist(cam);
    LbScreenLoadGraphicsWindow(&grwnd);
    cam->zoom = zoom_mem;//TODO [zoom] remove when all cam->zoom will be changed to camera_zoom
    SYNCDBG(9,"Finished");
}

static void render_sprite_debug_id(struct Thing* thing, long scr_x, long scr_y)
{
    if (render_sprite_debug_level < 2)
    {
        if (thing->class_id != TCls_Creature)
            return;
    }
    ushort flg_mem = lbDisplay.DrawFlags;
    lbDisplay.DrawFlags = Lb_TEXT_ONE_COLOR;
    const struct TbSprite *spr = get_button_sprite(GBS_fontchars_number_dig0);
    long w = scale_ui_value(spr->SWidth);
    long h = scale_ui_value(spr->SHeight);

    long digit_counter, value = thing->index;

    // Count digits to be displayed
    int ndigits=0;
    for (digit_counter = value; digit_counter > 0; digit_counter /= 10)
        ndigits++;
    // Show the digits
    scr_y -= h;
    long pos_x = w * (ndigits - 1) / 2 + scr_x;
    for (digit_counter = value; digit_counter > 0; digit_counter /= 10)
    {
        spr = get_button_sprite((digit_counter%10) + GBS_fontchars_number_dig0);
        LbSpriteDrawScaled(pos_x, scr_y - h, spr, w, h);

        pos_x -= w;
    }
    lbDisplay.DrawFlags = flg_mem;
}

void render_set_sprite_debug(int level)
{
    render_sprite_debug_level = level;
    switch (level)
    {
        case 0:
            render_sprite_debug_fn = NULL;
            break;
        default:
            render_sprite_debug_fn = &render_sprite_debug_id;
    }
}
/******************************************************************************/
