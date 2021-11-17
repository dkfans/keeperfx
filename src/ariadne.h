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
// Originally was 3000, but we're not using bak_path from DLL which gives us 517 extra
#define TREE_ROUTE_LEN 3517
#define BORDER_LENGTH 100
#define ROUTE_LENGTH 12000
#define ARID_WAYPOINTS_COUNT 10
#define ARID_PATH_WAYPOINTS_COUNT 256

/******************************************************************************/
#pragma pack(1)

struct Thing;

typedef unsigned char AriadneReturn;
typedef unsigned char AriadneRouteFlags;

enum AriadneReturnValues {
    AridRet_OK    = 0,
    AridRet_FinalOK,
    AridRet_Val2,
    AridRet_PartOK,
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
    AridUpSStM_Unkn1,
    AridUpSStM_Unkn2,
};

#define NAVMAP_FLOORHEIGHT_BIT  0
#define NAVMAP_FLOORHEIGHT_MAX  0x0f
#define NAVMAP_FLOORHEIGHT_MASK 0x0f
#define NAVMAP_UNSAFE_SURFACE   0x10
#define NAVMAP_OWNERSELECT_BIT  5
#define NAVMAP_OWNERSELECT_MAX  0x06
#define NAVMAP_OWNERSELECT_MASK 0x07
#define NAVMAP_OWNER_HERO    5
#define NAVMAP_OWNER_NEUTRAL 6

struct Ariadne { // sizeof = 102
    /** Position where the journey stated. */
    struct Coord3d startpos;
    /** Final position where we're heading. */
    struct Coord3d endpos;
    /** Position of the last reached waypoint. */
    struct Coord3d current_waypoint_pos;
  struct Coord3d pos_12;
  struct Coord3d pos_18;
  unsigned char route_flags;
  unsigned char field_1F;
  unsigned char field_20;
  unsigned char update_state;
  unsigned char field_22;
  unsigned char field_23;
  short field_24;
  unsigned short move_speed;
    /** Index of the current waypoint in list of nearest waypoints stored. */
    unsigned char current_waypoint;
    /** List of nearest waypoints in the way towards destination, stored in an array. */
    struct Coord2d waypoints[ARID_WAYPOINTS_COUNT];
    /** Amount of nearest waypoints stored in the array. */
    unsigned char stored_waypoints; // offs = 0x51
    /** Total amount of waypoints planned on the way towards endpos. */
    unsigned char total_waypoints;
  struct Coord3d pos_53;
  struct Coord3d pos_59;
  unsigned char manoeuvre_state;
  short wallhug_angle;
  long field_62;
};

struct PathWayPoint { // sizeof = 8
    long x;
    long y;
};

struct Path { // sizeof = 2068
    struct PathWayPoint start;
    struct PathWayPoint finish;
    long waypoints_num;
    struct PathWayPoint waypoints[ARID_PATH_WAYPOINTS_COUNT];
};

struct Gate { // sizeof = 28
  long field_0;
  long field_4;
  long field_8;
  long field_C;
  long field_10;
  long field_14;
  long field_18;
};

struct Pathway { // sizeof = 7192
  long field_0;
  long field_4;
  long field_8;
  long field_C;
  struct Gate points[256];
  long points_num;
  long field_1C14;
};

struct WayPoints { // sizeof = 1040
  long wpfield_0;
  long wpfield_4;
  long wpfield_8;
  long wpfield_C;
  long wpfield_10[256];
};

struct Navigation { // sizeof = 0x27
  unsigned char navstate;
  unsigned char field_1[3];
  unsigned char field_4;
  long field_5;
  long field_9;
  long field_D;
  unsigned char field_11[4];
  unsigned short field_15;
  unsigned short field_17;
  unsigned char field_19[2];
  struct Coord3d pos_next;
  struct Coord3d pos_final;
};

struct FOV { // sizeof=0x18
    struct PathWayPoint tipA;
    struct PathWayPoint tipB;
    struct PathWayPoint tipC;
};

/******************************************************************************/
DLLIMPORT unsigned long *_DK_EdgeFit;
#define EdgeFit _DK_EdgeFit
DLLIMPORT struct Pathway _DK_ap_GPathway;
#define ap_GPathway _DK_ap_GPathway
DLLIMPORT long _DK_tree_routelen;
#define tree_routelen _DK_tree_routelen
DLLIMPORT long _DK_tree_route[TREE_ROUTE_LEN];
#define tree_route _DK_tree_route
DLLIMPORT long _DK_tree_routecost;
#define tree_routecost _DK_tree_routecost
DLLIMPORT long _DK_tree_triA;
#define tree_triA _DK_tree_triA
DLLIMPORT long _DK_tree_triB;
#define tree_triB _DK_tree_triB
DLLIMPORT long _DK_tree_altA;
#define tree_altA _DK_tree_altA
DLLIMPORT long _DK_tree_altB;
#define tree_altB _DK_tree_altB
DLLIMPORT long _DK_tree_Ax8;
#define tree_Ax8 _DK_tree_Ax8
DLLIMPORT long _DK_tree_Ay8;
#define tree_Ay8 _DK_tree_Ay8
DLLIMPORT long _DK_tree_Bx8;
#define tree_Bx8 _DK_tree_Bx8
DLLIMPORT long _DK_tree_By8;
#define tree_By8 _DK_tree_By8
DLLIMPORT unsigned char *_DK_LastTriangulatedMap;
#define LastTriangulatedMap _DK_LastTriangulatedMap
DLLIMPORT unsigned char *_DK_fringe_map;
#define fringe_map _DK_fringe_map
DLLIMPORT long _DK_fringe_y1;
#define fringe_y1 _DK_fringe_y1
DLLIMPORT long _DK_fringe_y2;
#define fringe_y2 _DK_fringe_y2
DLLIMPORT long _DK_fringe_x1;
#define fringe_x1 _DK_fringe_x1
DLLIMPORT long _DK_fringe_x2;
#define fringe_x2 _DK_fringe_x2
DLLIMPORT long _DK_fringe_y[256];
#define fringe_y _DK_fringe_y
DLLIMPORT long _DK_ix_Border;
#define ix_Border _DK_ix_Border
DLLIMPORT long _DK_Border[BORDER_LENGTH];
#define Border _DK_Border
DLLIMPORT long _DK_route_fwd[ROUTE_LENGTH];
#define route_fwd _DK_route_fwd
DLLIMPORT long _DK_route_bak[ROUTE_LENGTH];
#define route_bak _DK_route_bak
DLLIMPORT struct Path _DK_fwd_path;
//#define fwd_path _DK_fwd_path
DLLIMPORT struct Path _DK_bak_path;
//#define bak_path _DK_bak_path

#pragma pack()
/******************************************************************************/
extern unsigned char const actual_sizexy_to_nav_block_sizexy_table[];
extern unsigned long const actual_sizexy_to_nav_sizexy_table[];
extern struct Path fwd_path;
extern struct Path bak_path;
/******************************************************************************/
long init_navigation(void);
long update_navigation_triangulation(long start_x, long start_y, long end_x, long end_y);
TbBool triangulate_area(unsigned char *imap, long sx, long sy, long ex, long ey);

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
void path_init8_wide_f(struct Path *path, long start_x, long start_y, long end_x, long end_y, long a6, unsigned char nav_size, const char *func_name);
void nearest_search_f(long sizexy, long srcx, long srcy, long dstx, long dsty, long *px, long *py, const char *func_name);
#define nearest_search(sizexy, srcx, srcy, dstx, dsty, px, py) nearest_search_f(sizexy, srcx, srcy, dstx, dsty, px, py, __func__)
long get_navigation_colour(long stl_x, long stl_y);
TbBool border_clip_horizontal(const unsigned char *imap, long a1, long a2, long a3, long a4);
TbBool border_clip_vertical(const unsigned char *imap, long a1, long a2, long a3, long a4);
#define edge_lock(fin_x, fin_y, bgn_x, bgn_y) edge_lock_f(fin_x, fin_y, bgn_x, bgn_y, __func__)
TbBool edge_lock_f(long ptend_x, long ptend_y, long ptstart_x, long ptstart_y, const char *func_name);
#define edge_unlock_record_and_regions(fin_x, fin_y, bgn_x, bgn_y) edge_unlock_record_and_regions_f(fin_x, fin_y, bgn_x, bgn_y, __func__)
TbBool edge_unlock_record_and_regions_f(long ptend_x, long ptend_y, long ptstart_x, long ptstart_y, const char *func_name);
void border_internal_points_delete(long a1, long a2, long a3, long a4);
TbBool tri_set_rectangle(long start_x, long start_y, long end_x, long end_y, unsigned char nav_colour);
long fringe_get_rectangle(long *a1, long *a2, long *a3, long *a4, unsigned char *a5);
long delaunay_seeded(long a1, long a2, long a3, long a4);
void border_unlock(long a1, long a2, long a3, long a4);
TbBool triangulation_border_start(long *a1, long *a2);
void triangulation_init(void);
void triangulation_initxy(long sx, long sy, long ex, long ey);
long pointed_at8(long pos_x, long pos_y, long *ret_tri, long *ret_pt);
long angle_to_quadrant(long angle);

long thing_nav_block_sizexy(const struct Thing *thing);
long thing_nav_sizexy(const struct Thing *thing);

void clear_wallhugging_path(struct Navigation *navi);
void initialise_wallhugging_path_from_to(struct Navigation *navi, struct Coord3d *mvstart, struct Coord3d *mvend);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
