/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_buckets.h
 *     Depth-bucket draw-list item types, shared between engine_render.c's
 *     own bucket walk (display_drawlist()/display_fast_drawlist()) and any
 *     renderer backend that needs to walk the same list itself 
 *     from these same bucket kinds instead of rasterizing them immediately.
 * @par Comment:
 *     engine_render should be the only producer of these items, shoot me if it's not.
 */
/******************************************************************************/
#ifndef DK_ENGNBUCKETS_H
#define DK_ENGNBUCKETS_H

#include "engine_render.h"  // BUCKETS_COUNT, struct PolyPoint, struct XYZ, struct Coord3d

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

struct Thing;

typedef unsigned char QKind;

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

struct BucketKindJontySprite { 
    struct BasicQ b;
    struct Thing *thing;
    long scr_x;
    long scr_y;
    long depth_fade;
    long bucket_idx;
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
    long bucket_idx;
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

extern struct BasicQ *buckets[BUCKETS_COUNT];

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // DK_ENGNBUCKETS_H
