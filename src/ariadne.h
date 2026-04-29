/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne.h
 *     Header file for ariadne.c.
 * @par Purpose:
 *     Dungeon routing and path finding system.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Jan 2010 - 20 Feb 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ARIADNE_H
#define DK_ARIADNE_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
// TREE_ROUTE_LEN <= 5000 results in crash on big maze map
// ARID_PATH_WAYPOINTS_COUNT <= 1300 results in crash on big maze map
#define TREE_ROUTE_LEN 6000
#define BORDER_LENGTH 100
#define ROUTE_LENGTH 12000
#define ARID_WAYPOINTS_COUNT 10
#define ARID_PATH_WAYPOINTS_COUNT 1400

/******************************************************************************/
#pragma pack(1)

struct Thing;

typedef unsigned char AriadneReturn;
typedef unsigned char AriadneRouteFlags;

enum AriadneReturnValues {
    AridRet_OK    = 0,
    AridRet_FinalOK,
    AridRet_Failed,
    AridRet_PartOK,
};

// Current wall-hugging activity state - "What wall-hugging am I currently doing?"
enum WallhugCurrentState {
    WallhugCurrentState_None = 0,        // Not wall-hugging
    WallhugCurrentState_Right = 1,       // Keep wall on creature's right side
    WallhugCurrentState_Left = 2         // Keep wall on creature's left side
};

// Wall-hugging side preference configuration - "Which side should I prefer to hug?"
enum WallhugPreference {
    WallhugPreference_None = 0,       // No wall-hugging preference
    WallhugPreference_Right = 1,      // Prefer to keep wall on right side
    WallhugPreference_Left = 2        // Prefer to keep wall on left side
};

// Pathfinding direction states for gates
enum PathfindingDirection {
    PathDir_Reverse = -1,           // Direction from end to start
    PathDir_StartToEnd = 0,         // Direction from start to end
    PathDir_EndToStart = 1,         // Direction from end to start
    PathDir_BestPoint = 2           // Direction to best point
};

// Wall-hugging activity state
enum WallhugActive {
    WallhugActive_Off = 0,          // Wall-hugging disabled
    WallhugActive_On = 1            // Wall-hugging enabled
};

// Triangle navigation corner flags for pathfinding
enum TriangleNavigationFlags {
    TriangleFlag_TopLeft = 0x01,         // Top-left corner flag
    TriangleFlag_TopRight = 0x02,        // Top-right corner flag
    TriangleFlag_BottomLeft = 0x04,      // Bottom-left corner flag
    TriangleFlag_BottomRight = 0x08,     // Bottom-right corner flag
    TriangleFlag_All = 0x0F              // All corners flag
};

// Field of view region test results
enum FieldOfViewRegion {
    FieldOfViewRegion_OutsideLeft = -1,  // Point is outside FOV on left side
    FieldOfViewRegion_WithinBounds = 0,  // Point is within FOV bounds
    FieldOfViewRegion_OutsideRight = 1   // Point is outside FOV on right side
};

// Navigation rule results for pathfinding
enum NavigationRule {
    NavigationRule_Blocked = 0,          // Cannot navigate through this area
    NavigationRule_Normal = 1,           // Normal navigation allowed
    NavigationRule_Special = 2           // Special navigation (higher cost)
};

// Creature navigation radius sizes for pathfinding
enum CreatureNavigationRadius {
    CreatureRadius_Small = 1,            // Small creature navigation radius
    CreatureRadius_Medium = 2,           // Medium creature navigation radius
    CreatureRadius_Large = 3             // Large creature navigation radius
};

enum AriadneRouteFlagValues {
    AridRtF_Default   = 0x00,
    AridRtF_NoOwner   = 0x01,
};

enum AriadneUpdateStateValues {
    AridUpSt_Unset   = 0,
    AridUpSt_OnLine,
    AridUpSt_Wallhug,
    AridUpSt_Manoeuvre,
};

enum AriadneUpdateSubStateManoeuvreValues {
    AridUpSStM_Unset   = 0,
    AridUpSStM_StartWallhug,
    AridUpSStM_ContinueWallhug,
};

enum NavigationStateValues {
    NavS_NavigationDisabled   = 0,
    NavS_WallhugInProgress,
    NavS_InitialWallhugSetup,
    NavS_WallhugDirectionCheck,
    NavS_WallhugPositionAdjust,
    NavS_WallhugAngleCorrection,
    NavS_WallhugGapDetected,
    NavS_WallhugRestartSetup,
};

#define NAVMAP_FLOORHEIGHT_BIT  0
#define NAVMAP_FLOORHEIGHT_MAX  0x0f
#define NAVMAP_FLOORHEIGHT_MASK 0x0f
#define NAVMAP_UNSAFE_SURFACE   0x10
#define NAVMAP_OWNERSELECT_BIT  5
#define NAVMAP_OWNERSELECT_MASK 0x3FE0

struct Ariadne { // sizeof = 102
    /** Position where the journey stated. */
    struct Coord3d startpos;
    /** Final position where we're heading. */
    struct Coord3d endpos;
    /** Position of the last reached waypoint. */
    struct Coord3d current_waypoint_pos;
  struct Coord3d next_position;
  struct Coord3d previous_position;
  unsigned char route_flags;
  unsigned char hug_side;
  unsigned char update_state;
  unsigned char wallhug_active;
  unsigned char may_need_reroute;
  short wallhug_stored_angle;
  unsigned short move_speed;
    /** Index of the current waypoint in list of nearest waypoints stored. */
    unsigned char current_waypoint;
    /** List of nearest waypoints in the way towards destination, stored in an array. */
    struct Coord2d waypoints[ARID_WAYPOINTS_COUNT];
    /** Amount of nearest waypoints stored in the array. */
    unsigned char stored_waypoints; // offs = 0x51
    /** Total amount of waypoints planned on the way towards endpos. */
    unsigned int total_waypoints;
  struct Coord3d manoeuvre_fixed_position;
  struct Coord3d manoeuvre_requested_position;
  unsigned char manoeuvre_state;
  short wallhug_angle;
  long straight_dist_to_next_waypoint;
};

struct PathWayPoint { // sizeof = 8
    int32_t x;
    int32_t y;
};

struct Path { // sizeof = 2068
    struct PathWayPoint start;
    struct PathWayPoint finish;
    long waypoints_num;
    struct PathWayPoint waypoints[ARID_PATH_WAYPOINTS_COUNT];
};

struct Gate { // sizeof = 28
  long start_coordinate_x;
  long start_coordinate_y;
  long end_coordinate_x;
  long end_coordinate_y;
  long intersection_coordinate_x;
  long intersection_coordinate_y;
  long pathfinding_direction;
};

struct Pathway {
  long start_coordinate_x;
  long start_coordinate_y;
  long finish_coordinate_x;
  long finish_coordinate_y;
  struct Gate points[256];
  long points_num;
};

struct WayPoints {
  long edge1_start_index;
  long edge2_start_index;
  long edge1_current_index;
  long edge2_current_index;
  int32_t waypoint_index_array[ARID_PATH_WAYPOINTS_COUNT];
};

struct Navigation {
  unsigned char navstate;
  unsigned char side;
  unsigned char wallhug_retry_counter;
  unsigned char wallhug_state;
  unsigned char push_counter;
  long dist_to_final_pos;
  long distance_to_next_pos;
  int32_t angle;
  SubtlCodedCoords first_colliding_block;
  SubtlCodedCoords second_colliding_block;
  PlayerBitFlags owner_flags[2];
  struct Coord3d pos_next;
  struct Coord3d pos_final;
};

struct FOV { // sizeof=0x18
    struct PathWayPoint tipA;
    struct PathWayPoint tipB;
    struct PathWayPoint tipC;
};

struct HugStart {
    short wh_angle;
    unsigned char wh_side;
};

/******************************************************************************/

extern const struct HugStart blocked_x_hug_start[][2];
extern const struct HugStart blocked_y_hug_start[][2];
extern const struct HugStart blocked_xy_hug_start[][2][2];

/******************************************************************************/


#pragma pack()
/******************************************************************************/
long init_navigation(void);
long update_navigation_triangulation(long start_x, long start_y, long end_x, long end_y);
TbBool triangulate_area(NavColour *imap, long sx, long sy, long ex, long ey);

AriadneReturn ariadne_initialise_creature_route_f(struct Thing *thing, const struct Coord3d *pos, long speed, AriadneRouteFlags flags, const char *func_name);
#define ariadne_initialise_creature_route(thing, pos, speed, flags) ariadne_initialise_creature_route_f(thing, pos, speed, flags, __func__)
AriadneReturn creature_follow_route_to_using_gates(struct Thing *thing, struct Coord3d *finalpos, struct Coord3d *nextpos, long speed, AriadneRouteFlags flags);
#define ariadne_prepare_creature_route_to_target(thing, arid, srcpos, dstpos, speed, flags) ariadne_prepare_creature_route_to_target_f(thing, arid, srcpos, dstpos, speed, flags, __func__)
AriadneReturn ariadne_prepare_creature_route_to_target_f(const struct Thing *thing, struct Ariadne *arid,
    const struct Coord3d *srcpos, const struct Coord3d *dstpos, long speed, AriadneRouteFlags flags, const char *func_name);
long ariadne_count_waypoints_on_creature_route_to_target_f(const struct Thing *thing,
    const struct Coord3d *srcpos, const struct Coord3d *dstpos, AriadneRouteFlags flags, const char *func_name);
AriadneReturn ariadne_invalidate_creature_route(struct Thing *thing);

TbBool navigation_points_connected(struct Coord3d *pt1, struct Coord3d *pt2);
void path_init8_wide_f(struct Path *path, long start_x, long start_y, long end_x, long end_y, long subroute, unsigned char nav_size, const char *func_name);
void nearest_search_f(long sizexy, long srcx, long srcy, long dstx, long dsty, int32_t *px, int32_t *py, const char *func_name);
#define nearest_search(sizexy, srcx, srcy, dstx, dsty, px, py) nearest_search_f(sizexy, srcx, srcy, dstx, dsty, px, py, __func__)
NavColour get_navigation_colour(long stl_x, long stl_y);
TbBool border_clip_horizontal(const NavColour *imap, long start_x, long end_x, long start_y, long end_y);
TbBool border_clip_vertical(const NavColour *imap, long start_x, long end_x, long start_y, long end_y);
#define edge_lock(fin_x, fin_y, bgn_x, bgn_y) edge_lock_f(fin_x, fin_y, bgn_x, bgn_y, __func__)
TbBool edge_lock_f(long ptend_x, long ptend_y, long ptstart_x, long ptstart_y, const char *func_name);
#define edge_unlock_record_and_regions(fin_x, fin_y, bgn_x, bgn_y) edge_unlock_record_and_regions_f(fin_x, fin_y, bgn_x, bgn_y, __func__)
TbBool edge_unlock_record_and_regions_f(long ptend_x, long ptend_y, long ptstart_x, long ptstart_y, const char *func_name);
void border_internal_points_delete(long start_x, long start_y, long end_x, long end_y);
TbBool tri_set_rectangle(long start_x, long start_y, long end_x, long end_y, NavColour nav_colour);
long fringe_get_rectangle(int32_t *outfri_x1, int32_t *outfri_y1, int32_t *outfri_x2, int32_t *outfri_y2, NavColour *oval);
long delaunay_seeded(long start_x, long start_y, long end_x, long end_y);
void border_unlock(long start_x, long start_y, long end_x, long end_y);
TbBool triangulation_border_start(int32_t *border_a, int32_t *border_b);
void triangulation_init(void);
void triangulation_initxy(long sx, long sy, long ex, long ey);
long pointed_at8(long pos_x, long pos_y, int32_t *ret_tri, int32_t *ret_pt);
long angle_to_quadrant(long angle);

long thing_nav_block_sizexy(const struct Thing *thing);
long thing_nav_sizexy(const struct Thing *thing);

void initialise_wallhugging_path_from_to(struct Navigation *navi, struct Coord3d *mvstart, struct Coord3d *mvend);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
